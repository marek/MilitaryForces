#ifndef __BOT_BEHAVIOR_H__
#define __BOT_BEHAVIOR_H__

#include <array>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

namespace behavior
{
	enum class Status : unsigned char { Running, Success, Failure };

	constexpr std::size_t maxNodes = 256;

	// what one bot's tree remembers between frames: where each composite is, each decorator's clock, and what every
	// node returned the last time it was ticked, for the tree dump
	struct Memory
	{
		std::array<short, maxNodes>		cursor = {};
		std::array<int, maxNodes>		clock = {};	// a decorator's time: when it started, or until when it holds
		std::array<int, maxNodes>		stamp = {};	// the frame each node was last ticked in
		std::array<Status, maxNodes>	status = {};	// and what it returned then
		int								frame = 0;
		int								now = 0;	// the game time of this frame, in milliseconds
		int								leaf = -1;
		int								previousLeaf = -1;

		bool tickedThisFrame( int id ) const { return stamp[id] == frame; }
	};

	template<typename Context>
	class Node
	{
	public:
		explicit Node( const char* name ) : name_( name ) {}
		virtual ~Node() = default;

		Status tick( Context& context, Memory& memory ) const
		{
			memory.stamp[id_] = memory.frame;
			const Status status = run( context, memory );
			memory.status[id_] = status;
			return status;
		}
		virtual void reset( Memory& memory ) const { memory.cursor[id_] = 0; }
		// the nodes under this one, for the tree dump
		virtual void children( std::vector<const Node*>& out ) const { static_cast<void>( out ); }
		// what kind of node, for the tree dump
		virtual const char* kind() const = 0;

		const char*	name_;
		int			id_ = 0;

	protected:
		virtual Status run( Context& context, Memory& memory ) const = 0;
	};

	template<typename Context>
	using Children = std::vector<const Node<Context>*>;

	// a length of time, fixed or worked out from the context (a patient character waits longer)
	template<typename Context>
	struct Duration
	{
		using Function = int (*)( const Context& );
		Duration( int milliseconds ) : fixed( milliseconds ) {}
		Duration( Function function ) : function( function ) {}
		int of( const Context& context ) const { return function ? function( context ) : fixed; }

		int			fixed = 0;
		Function	function = nullptr;
	};

	template<typename Context>
	class Action : public Node<Context>
	{
	public:
		using Function = Status (*)( Context& );
		Action( const char* name, Function function ) : Node<Context>( name ), function_( function ) {}
		const char* kind() const override { return "action"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			memory.leaf = this->id_;
			return function_( context );
		}

	private:
		Function function_;
	};

	// a composite: holds children and resets them with itself
	template<typename Context>
	class Composite : public Node<Context>
	{
	public:
		Composite( const char* name, Children<Context> children ) : Node<Context>( name ), children_( std::move( children ) ) {}

		void reset( Memory& memory ) const override
		{
			memory.cursor[this->id_] = 0;
			for( const Node<Context>* child : children_ )
				child->reset( memory );
		}
		void children( std::vector<const Node<Context>*>& out ) const override { out.insert( out.end(), children_.begin(), children_.end() ); }

	protected:
		Children<Context> children_;
	};

	// the children in order until one fails; running resumes where it left off
	template<typename Context>
	class Sequence : public Composite<Context>
	{
	public:
		using Composite<Context>::Composite;
		const char* kind() const override { return "sequence"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			const auto& children = this->children_;
			std::size_t index = static_cast<std::size_t>( memory.cursor[this->id_] );
			while( index < children.size() )
			{
				const Status status = children[index]->tick( context, memory );
				if( status == Status::Running )
				{
					memory.cursor[this->id_] = static_cast<short>( index );
					return status;
				}
				if( status == Status::Failure )
				{
					this->reset( memory );
					return status;
				}
				index++;
			}
			this->reset( memory );
			return Status::Success;
		}
	};

	// the first child that does not fail, tried in order every frame
	template<typename Context>
	class Selector : public Composite<Context>
	{
	public:
		using Composite<Context>::Composite;
		const char* kind() const override { return "selector"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			const auto& children = this->children_;
			for( std::size_t index = 0; index < children.size(); index++ )
			{
				const Status status = children[index]->tick( context, memory );
				if( status == Status::Failure )
					continue;
				const std::size_t previous = static_cast<std::size_t>( memory.cursor[this->id_] );
				if( previous != index + 1 && previous > 0 && previous <= children.size() )
					children[previous - 1]->reset( memory );
				memory.cursor[this->id_] = static_cast<short>( index + 1 );
				return status;
			}
			memory.cursor[this->id_] = 0;
			return Status::Failure;
		}
	};

	// the child a function names
	template<typename Context>
	class Chooser : public Composite<Context>
	{
	public:
		using Choice = int (*)( Context& );
		Chooser( const char* name, Choice choice, Children<Context> children ) :
			Composite<Context>( name, std::move( children ) ), choice_( choice ) {}
		const char* kind() const override { return "chooser"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			const auto& children = this->children_;
			const int wanted = choice_( context );
			if( wanted < 0 || static_cast<std::size_t>( wanted ) >= children.size() )
				return Status::Failure;
			const int previous = memory.cursor[this->id_] - 1;
			if( previous >= 0 && previous != wanted )
				children[previous]->reset( memory );
			memory.cursor[this->id_] = static_cast<short>( wanted + 1 );
			return children[wanted]->tick( context, memory );
		}

	private:
		Choice choice_;
	};

	// the first child, then the others every frame whatever the first returned: things a pilot does alongside
	template<typename Context>
	class Always : public Composite<Context>
	{
	public:
		Always( const char* name, const Node<Context>* child, Children<Context> after ) :
			Composite<Context>( name, withFirst( child, std::move( after ) ) ) {}
		const char* kind() const override { return "always"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			const auto& children = this->children_;
			const Status status = children[0]->tick( context, memory );
			const int leaf = memory.leaf;
			for( std::size_t index = 1; index < children.size(); index++ )
				children[index]->tick( context, memory );
			memory.leaf = leaf;
			return status;
		}

	private:
		static Children<Context> withFirst( const Node<Context>* child, Children<Context> after )
		{
			after.insert( after.begin(), child );
			return after;
		}
	};

	// a decorator: one child
	template<typename Context>
	class Decorator : public Node<Context>
	{
	public:
		Decorator( const char* name, const Node<Context>* child ) : Node<Context>( name ), child_( child ) {}

		void reset( Memory& memory ) const override
		{
			memory.cursor[this->id_] = 0;
			child_->reset( memory );
		}
		void children( std::vector<const Node<Context>*>& out ) const override { out.push_back( child_ ); }

	protected:
		const Node<Context>* child_;
	};

	// the child while a condition holds
	template<typename Context>
	class When : public Decorator<Context>
	{
	public:
		using Predicate = bool (*)( Context& );
		When( const char* name, Predicate predicate, const Node<Context>* child ) : Decorator<Context>( name, child ), predicate_( predicate ) {}
		const char* kind() const override { return "when"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			if( !predicate_( context ) )
			{
				if( memory.cursor[this->id_] )
					this->reset( memory );
				return Status::Failure;
			}
			memory.cursor[this->id_] = 1;
			return this->child_->tick( context, memory );
		}

	private:
		Predicate predicate_;
	};

	// the child again whenever it fails
	template<typename Context>
	class RetryUntilSuccess : public Decorator<Context>
	{
	public:
		using Decorator<Context>::Decorator;
		const char* kind() const override { return "retry"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			const Status status = this->child_->tick( context, memory );
			if( status == Status::Failure )
			{
				this->child_->reset( memory );
				return Status::Running;
			}
			return status;
		}
	};

	// once the child has finished, it is not tried again until some time has passed. The wait outlives a reset of the
	// branch: a cooldown that a reset cleared would never cool anything
	template<typename Context>
	class Cooldown : public Decorator<Context>
	{
	public:
		Cooldown( const char* name, Duration<Context> wait, const Node<Context>* child ) : Decorator<Context>( name, child ), wait_( wait ) {}
		const char* kind() const override { return "cooldown"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			if( memory.now < memory.clock[this->id_] )
				return Status::Failure;
			const Status status = this->child_->tick( context, memory );
			if( status != Status::Running )
				memory.clock[this->id_] = memory.now + wait_.of( context );
			return status;
		}

	private:
		Duration<Context> wait_;
	};

	// the child may run this long before it is given up on
	template<typename Context>
	class TimeLimit : public Decorator<Context>
	{
	public:
		TimeLimit( const char* name, Duration<Context> limit, const Node<Context>* child ) : Decorator<Context>( name, child ), limit_( limit ) {}
		const char* kind() const override { return "time limit"; }

		void reset( Memory& memory ) const override
		{
			memory.clock[this->id_] = 0;
			Decorator<Context>::reset( memory );
		}

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			if( !memory.clock[this->id_] )
				memory.clock[this->id_] = memory.now;
			else if( memory.now - memory.clock[this->id_] > limit_.of( context ) )
			{
				reset( memory );
				return Status::Failure;
			}
			const Status status = this->child_->tick( context, memory );
			if( status != Status::Running )
				memory.clock[this->id_] = 0;
			return status;
		}

	private:
		Duration<Context> limit_;
	};

	// a condition that, once true, is held true for some time after it last was: a choice is not dropped the moment
	// its reason flickers
	template<typename Context>
	class HoldFor : public Decorator<Context>
	{
	public:
		using Predicate = bool (*)( Context& );
		HoldFor( const char* name, Predicate predicate, Duration<Context> hold, const Node<Context>* child ) :
			Decorator<Context>( name, child ), predicate_( predicate ), hold_( hold ) {}
		const char* kind() const override { return "hold for"; }

		void reset( Memory& memory ) const override
		{
			memory.clock[this->id_] = 0;
			Decorator<Context>::reset( memory );
		}

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			if( predicate_( context ) )
				memory.clock[this->id_] = memory.now + hold_.of( context );
			else if( memory.now >= memory.clock[this->id_] )
			{
				if( memory.cursor[this->id_] )
					reset( memory );
				return Status::Failure;
			}
			memory.cursor[this->id_] = 1;
			return this->child_->tick( context, memory );
		}

	private:
		Predicate			predicate_;
		Duration<Context>	hold_;
	};

	// entered when a condition holds, then kept until the child finishes whatever the condition does meanwhile
	template<typename Context>
	class Latch : public Decorator<Context>
	{
	public:
		using Predicate = bool (*)( Context& );
		Latch( const char* name, Predicate predicate, const Node<Context>* child ) : Decorator<Context>( name, child ), predicate_( predicate ) {}
		const char* kind() const override { return "latch"; }

	protected:
		Status run( Context& context, Memory& memory ) const override
		{
			if( !memory.cursor[this->id_] && !predicate_( context ) )
				return Status::Failure;
			memory.cursor[this->id_] = 1;
			const Status status = this->child_->tick( context, memory );
			if( status != Status::Running )
				memory.cursor[this->id_] = 0;
			return status;
		}

	private:
		Predicate predicate_;
	};

	template<typename Context>
	class Tree
	{
	public:
		template<typename NodeType, typename... Arguments>
		const Node<Context>* add( Arguments&&... arguments )
		{
			auto node = std::make_unique<NodeType>( std::forward<Arguments>( arguments )... );
			node->id_ = static_cast<int>( nodes_.size() );
			nodes_.push_back( std::move( node ) );
			return nodes_.back().get();
		}

		const Node<Context>* action( const char* name, typename Action<Context>::Function function )
		{
			return add<Action<Context>>( name, function );
		}
		const Node<Context>* sequence( const char* name, std::initializer_list<const Node<Context>*> children )
		{
			return add<Sequence<Context>>( name, Children<Context>( children ) );
		}
		const Node<Context>* selector( const char* name, std::initializer_list<const Node<Context>*> children )
		{
			return add<Selector<Context>>( name, Children<Context>( children ) );
		}
		const Node<Context>* chooser( const char* name, typename Chooser<Context>::Choice choice, std::initializer_list<const Node<Context>*> children )
		{
			return add<Chooser<Context>>( name, choice, Children<Context>( children ) );
		}
		const Node<Context>* when( const char* name, typename When<Context>::Predicate predicate, const Node<Context>* child )
		{
			return add<When<Context>>( name, predicate, child );
		}
		const Node<Context>* retry( const char* name, const Node<Context>* child )
		{
			return add<RetryUntilSuccess<Context>>( name, child );
		}
		const Node<Context>* always( const char* name, const Node<Context>* child, std::initializer_list<const Node<Context>*> after )
		{
			return add<Always<Context>>( name, child, Children<Context>( after ) );
		}
		const Node<Context>* cooldown( const char* name, Duration<Context> wait, const Node<Context>* child )
		{
			return add<Cooldown<Context>>( name, wait, child );
		}
		const Node<Context>* timeLimit( const char* name, Duration<Context> limit, const Node<Context>* child )
		{
			return add<TimeLimit<Context>>( name, limit, child );
		}
		const Node<Context>* holdFor( const char* name, typename HoldFor<Context>::Predicate predicate, Duration<Context> hold, const Node<Context>* child )
		{
			return add<HoldFor<Context>>( name, predicate, hold, child );
		}
		const Node<Context>* latch( const char* name, typename Latch<Context>::Predicate predicate, const Node<Context>* child )
		{
			return add<Latch<Context>>( name, predicate, child );
		}

		void setRoot( const Node<Context>* root ) { root_ = root; }
		std::size_t size() const { return nodes_.size(); }
		const char* nameOf( int id ) const { return id >= 0 && static_cast<std::size_t>( id ) < nodes_.size() ? nodes_[id]->name_ : "-"; }

		Status tick( Context& context, Memory& memory, int now ) const
		{
			memory.frame++;
			memory.now = now;
			memory.previousLeaf = memory.leaf;
			memory.leaf = -1;
			return root_->tick( context, memory );
		}

		// the tree as indented lines, each node marked with what it returned in the last frame: '>' running,
		// '+' succeeded, '-' failed, blank when not reached. The failed nodes ahead of the running one say why a
		// selector went past them
		template<typename Print>
		void dump( const Memory& memory, Print print ) const
		{
			if( root_ )
				dumpNode( root_, memory, 0, print );
		}

	private:
		template<typename Print>
		void dumpNode( const Node<Context>* node, const Memory& memory, int depth, Print& print ) const
		{
			char mark = ' ';
			if( memory.tickedThisFrame( node->id_ ) )
				mark = memory.status[node->id_] == Status::Running ? '>' : memory.status[node->id_] == Status::Success ? '+' : '-';
			print( std::string( static_cast<std::size_t>( depth ) * 2, ' ' ) + mark + ' ' + node->name_ + " (" + node->kind() + ")" );
			std::vector<const Node<Context>*> children;
			node->children( children );
			for( const Node<Context>* child : children )
				dumpNode( child, memory, depth + 1, print );
		}

		std::vector<std::unique_ptr<Node<Context>>>	nodes_;
		const Node<Context>*							root_ = nullptr;
	};
}

#endif
