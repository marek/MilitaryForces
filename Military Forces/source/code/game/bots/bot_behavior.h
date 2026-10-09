#ifndef __BOT_BEHAVIOR_H__
#define __BOT_BEHAVIOR_H__

#include <array>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <vector>

namespace behavior
{
	enum class Status { Running, Success, Failure };

	constexpr std::size_t maxNodes = 128;

	struct Memory
	{
		std::array<short, maxNodes>	cursor = {};
		int							leaf = -1;
		int							previousLeaf = -1;
	};

	template<typename Context>
	class Node
	{
	public:
		explicit Node( const char* name ) : name_( name ) {}
		virtual ~Node() = default;

		virtual Status tick( Context& context, Memory& memory ) const = 0;
		virtual void reset( Memory& memory ) const { memory.cursor[id_] = 0; }

		const char*	name_;
		int			id_ = 0;
	};

	template<typename Context>
	using Children = std::vector<const Node<Context>*>;

	template<typename Context>
	class Action : public Node<Context>
	{
	public:
		using Function = Status (*)( Context& );
		Action( const char* name, Function function ) : Node<Context>( name ), function_( function ) {}

		Status tick( Context& context, Memory& memory ) const override
		{
			memory.leaf = this->id_;
			return function_( context );
		}

	private:
		Function function_;
	};

	template<typename Context>
	class Sequence : public Node<Context>
	{
	public:
		Sequence( const char* name, Children<Context> children ) : Node<Context>( name ), children_( std::move( children ) ) {}

		Status tick( Context& context, Memory& memory ) const override
		{
			std::size_t index = static_cast<std::size_t>( memory.cursor[this->id_] );
			while( index < children_.size() )
			{
				const Status status = children_[index]->tick( context, memory );
				if( status == Status::Running )
				{
					memory.cursor[this->id_] = static_cast<short>( index );
					return status;
				}
				if( status == Status::Failure )
				{
					reset( memory );
					return status;
				}
				index++;
			}
			reset( memory );
			return Status::Success;
		}

		void reset( Memory& memory ) const override
		{
			memory.cursor[this->id_] = 0;
			for( const Node<Context>* child : children_ )
				child->reset( memory );
		}

	private:
		Children<Context> children_;
	};

	template<typename Context>
	class Selector : public Node<Context>
	{
	public:
		Selector( const char* name, Children<Context> children ) : Node<Context>( name ), children_( std::move( children ) ) {}

		Status tick( Context& context, Memory& memory ) const override
		{
			for( std::size_t index = 0; index < children_.size(); index++ )
			{
				const Status status = children_[index]->tick( context, memory );
				if( status == Status::Failure )
					continue;
				const std::size_t previous = static_cast<std::size_t>( memory.cursor[this->id_] );
				if( previous != index + 1 && previous > 0 && previous <= children_.size() )
					children_[previous - 1]->reset( memory );
				memory.cursor[this->id_] = static_cast<short>( index + 1 );
				return status;
			}
			memory.cursor[this->id_] = 0;
			return Status::Failure;
		}

		void reset( Memory& memory ) const override
		{
			memory.cursor[this->id_] = 0;
			for( const Node<Context>* child : children_ )
				child->reset( memory );
		}

	private:
		Children<Context> children_;
	};

	template<typename Context>
	class Chooser : public Node<Context>
	{
	public:
		using Choice = int (*)( Context& );
		Chooser( const char* name, Choice choice, Children<Context> children ) :
			Node<Context>( name ), choice_( choice ), children_( std::move( children ) ) {}

		Status tick( Context& context, Memory& memory ) const override
		{
			const int wanted = choice_( context );
			if( wanted < 0 || static_cast<std::size_t>( wanted ) >= children_.size() )
				return Status::Failure;
			const int previous = memory.cursor[this->id_] - 1;
			if( previous >= 0 && previous != wanted )
				children_[previous]->reset( memory );
			memory.cursor[this->id_] = static_cast<short>( wanted + 1 );
			return children_[wanted]->tick( context, memory );
		}

		void reset( Memory& memory ) const override
		{
			memory.cursor[this->id_] = 0;
			for( const Node<Context>* child : children_ )
				child->reset( memory );
		}

	private:
		Choice				choice_;
		Children<Context>	children_;
	};

	template<typename Context>
	class When : public Node<Context>
	{
	public:
		using Predicate = bool (*)( Context& );
		When( const char* name, Predicate predicate, const Node<Context>* child ) :
			Node<Context>( name ), predicate_( predicate ), child_( child ) {}

		Status tick( Context& context, Memory& memory ) const override
		{
			if( !predicate_( context ) )
			{
				if( memory.cursor[this->id_] )
					reset( memory );
				return Status::Failure;
			}
			memory.cursor[this->id_] = 1;
			return child_->tick( context, memory );
		}

		void reset( Memory& memory ) const override
		{
			memory.cursor[this->id_] = 0;
			child_->reset( memory );
		}

	private:
		Predicate			predicate_;
		const Node<Context>*	child_;
	};

	template<typename Context>
	class RetryUntilSuccess : public Node<Context>
	{
	public:
		RetryUntilSuccess( const char* name, const Node<Context>* child ) : Node<Context>( name ), child_( child ) {}

		Status tick( Context& context, Memory& memory ) const override
		{
			const Status status = child_->tick( context, memory );
			if( status == Status::Failure )
			{
				child_->reset( memory );
				return Status::Running;
			}
			return status;
		}

		void reset( Memory& memory ) const override
		{
			memory.cursor[this->id_] = 0;
			child_->reset( memory );
		}

	private:
		const Node<Context>* child_;
	};

	template<typename Context>
	class Always : public Node<Context>
	{
	public:
		Always( const char* name, const Node<Context>* child, Children<Context> after ) :
			Node<Context>( name ), child_( child ), after_( std::move( after ) ) {}

		Status tick( Context& context, Memory& memory ) const override
		{
			const Status status = child_->tick( context, memory );
			const int leaf = memory.leaf;
			for( const Node<Context>* node : after_ )
				node->tick( context, memory );
			memory.leaf = leaf;
			return status;
		}

		void reset( Memory& memory ) const override
		{
			memory.cursor[this->id_] = 0;
			child_->reset( memory );
			for( const Node<Context>* node : after_ )
				node->reset( memory );
		}

	private:
		const Node<Context>*	child_;
		Children<Context>	after_;
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

		void setRoot( const Node<Context>* root ) { root_ = root; }
		std::size_t size() const { return nodes_.size(); }
		const char* nameOf( int id ) const { return id >= 0 && static_cast<std::size_t>( id ) < nodes_.size() ? nodes_[id]->name_ : "-"; }

		Status tick( Context& context, Memory& memory ) const
		{
			memory.previousLeaf = memory.leaf;
			memory.leaf = -1;
			return root_->tick( context, memory );
		}

	private:
		std::vector<std::unique_ptr<Node<Context>>>	nodes_;
		const Node<Context>*							root_ = nullptr;
	};
}

#endif
