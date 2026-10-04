#include "g_entity.h"
#include "g_local.h"
#include "g_level.h"



GameEntity::Event::Event() :
	event_(0),
	eventParm_(0),
	used_(false)
{
}

GameEntity::GameEntity() :
	client_(nullptr),
	inuse_(false),
	classname_(nullptr),
	spawnflags_(0),
	//neverFree_(false),
	flags_(0),
	model_(nullptr),
	model2_(nullptr),
	freetime_(0),
	eventTime_(0),
	freeAfterEvent_(false),
	unlinkAfterEvent_(false),
	physicsObject_(false),
	physicsBounce_(0.0f),
	clipmask_(0),
	moverState_(MOVER_POS1),
	soundPos1_(0),
	sound1to2_(0),
	sound2to1_(0),
	soundPos2_(0),
	soundLoop_(0),
	parent_(nullptr),
	nextTrain_(nullptr),
	prevTrain_(nullptr),
	message_(nullptr),
	timestamp_(0),
	angle_(0.0f),
	target_(nullptr),
	targetname_(nullptr),
	team_(nullptr),
	targetShaderName_(nullptr),
	targetShaderNewName_(nullptr),
	target_ent_(nullptr),
	speed_(0.0f),
	nextthink_(0),
	pain_debounce_time_(0),
	fly_sound_debounce_time_(0),
	last_move_time_(0),
	health_(0),
	takedamage_(false),
	damage_(0),
	splashDamage_(0),
	splashRadius_(0),
	methodOfDeath_(0),
	splashMethodOfDeath_(0),
	count_(0),
	chain_(nullptr),
	enemy_(nullptr),
	activator_(nullptr),
	teamchain_(nullptr),
	teammaster_(nullptr),
	watertype_(0),
	waterlevel_(0),
	noise_index_(0),
	wait_(0.0f),
	random_(0.0f),
	item_(nullptr),
	ONOFF_(0),
	left_(0),
	bulletpos_(0),
	gearheight_(0.0f),
	rechargetime_(0),
	radio_target_(nullptr),
	ent_category_(0),
	radartime_(0),
	score_(0),
	tracerindex_(0),
	idxScriptBegin_(0),
	idxScriptEnd_(0),
	tracktarget_(nullptr),
	locktime_(0),
	targetcat_(0.0f),
	catmodifier_(0.0f),
	range_(0.0f),
	followcone_(0.0f),
	lastDist_(0),
	updateGear_(false),
	updateBay_(false),
	eventSent_(false),
	currentEventToSend_(0),
	currentEventToAdd_(0),
	gi_nextScanTime_(0),
	gi_lockangle_(0.0f),
	gi_reloadTime_(0),
	basicECMVulnerability_(0),
	nukeTime_(0),
	nukeShockTime_(0),
	thinkFunc_(nullptr),
	reachedFunc_(nullptr),
	blockedFunc_(nullptr),
	touchFunc_(nullptr),
	useFunc_(nullptr),
	painFunc_(nullptr),
	dieFunc_(nullptr)
{
	VectorClear(pos1_);
	VectorClear(pos2_);
	VectorClear(movedir_);

}

void
GameEntity::cleanAll()
{
	client_ = nullptr;
	inuse_ = false;
	classname_ = nullptr;
	spawnflags_ = 0;
	//neverFree_ = false;
	flags_ = 0;
	model_ = nullptr;
	model2_ = nullptr;
	freetime_ = 0;
	eventTime_ = 0;
	freeAfterEvent_ = false;
	unlinkAfterEvent_ = false;
	physicsObject_ = false;
	physicsBounce_ = 0.0f;
	clipmask_ = 0;
	moverState_ = MOVER_POS1;
	soundPos1_ = 0;
	sound1to2_ = 0;
	sound2to1_ = 0;
	soundPos2_ = 0;
	soundLoop_ = 0;
	parent_ = nullptr;
	nextTrain_ = nullptr;
	prevTrain_ = nullptr;
	message_ = nullptr;
	timestamp_ = 0;
	angle_ = 0.0f;
	target_ = nullptr;
	targetname_ = nullptr;
	team_ = nullptr;
	targetShaderName_ = nullptr;
	targetShaderNewName_ = nullptr;
	target_ent_ = nullptr;
	speed_ = 0.0f;
	nextthink_ = 0;
	pain_debounce_time_ = 0;
	fly_sound_debounce_time_ = 0;
	last_move_time_ = 0;
	health_ = 0;
	takedamage_ = false;
	damage_ = 0;
	splashDamage_ = 0;
	splashRadius_ = 0;
	methodOfDeath_ = 0;
	splashMethodOfDeath_ = 0;
	count_ = 0;
	chain_ = nullptr;
	enemy_ = nullptr;
	activator_ = nullptr;
	teamchain_ = nullptr;
	teammaster_ = nullptr;
	watertype_ = 0;
	waterlevel_ = 0;
	noise_index_ = 0;
	wait_ = 0.0f;
	random_ = 0.0f;
	item_ = nullptr;
	ONOFF_ = 0;
	left_ = 0;
	bulletpos_ = 0;
	gearheight_ = 0.0f;
	rechargetime_ = 0;
	radio_target_ = nullptr;
	ent_category_ = 0;
	radartime_ = 0;
	score_ = 0;
	tracerindex_ = 0;
	idxScriptBegin_ = 0;
	idxScriptEnd_ = 0;
	tracktarget_ = nullptr;
	locktime_ = 0;
	targetcat_ = 0.0f;
	catmodifier_ = 0.0f;
	range_ = 0.0f;
	followcone_ = 0.0f;
	lastDist_ = 0;
	updateGear_ = false;
	updateBay_ = false;
	eventSent_ = false;
	currentEventToSend_ = 0;
	currentEventToAdd_ = 0;
	gi_nextScanTime_ = 0;
	gi_lockangle_ = 0.0f;
	gi_reloadTime_ = 0;
	basicECMVulnerability_ = 0;
	nukeTime_ = 0;
	nukeShockTime_ = 0;
	VectorClear(pos1_);
	VectorClear(pos2_);
	VectorClear(movedir_);
	retireAll();
}

namespace
{
	constexpr std::size_t funcBlockSize = 96;
	constexpr std::size_t funcSlabBlocks = 256;
	std::vector<void*> funcFree;

	void* takeFuncBlock()
	{
		if( funcFree.empty() )
		{
			char* slab = static_cast<char*>( ::operator new( funcBlockSize * funcSlabBlocks ) );
			funcFree.reserve( funcFree.capacity() + funcSlabBlocks );
			for( std::size_t i = 0; i < funcSlabBlocks; i++ )
				funcFree.push_back( slab + i * funcBlockSize );
		}
		void* block = funcFree.back();
		funcFree.pop_back();
		return block;
	}
}

void* GameEntity::EntityFunc_Base::operator new( std::size_t size )
{
	return size <= funcBlockSize ? takeFuncBlock() : ::operator new( size );
}

void GameEntity::EntityFunc_Base::operator delete( void* block, std::size_t size )
{
	if( !block )
		return;
	if( size <= funcBlockSize )
		funcFree.push_back( block );
	else
		::operator delete( block );
}

static std::vector<std::unique_ptr<GameEntity::EntityFunc_Base>> retiredFuncs;

template<class T>
static void retire( std::unique_ptr<T>& func )
{
	if( func )
		retiredFuncs.push_back( std::move( func ) );
}

void
GameEntity::releaseRetired()
{
	retiredFuncs.clear();
}

void
GameEntity::retireAll()
{
	retire( thinkFunc_ );
	retire( reachedFunc_ );
	retire( blockedFunc_ );
	retire( touchFunc_ );
	retire( useFunc_ );
	retire( painFunc_ );
	retire( dieFunc_ );
}

GameEntity::~GameEntity()
{
	retireAll();
}

void
GameEntity::setThink( std::unique_ptr<EntityFunc_Think> newThink )
{
	retire( thinkFunc_ );
	thinkFunc_ = std::move( newThink );
	if( thinkFunc_ )
		thinkFunc_->self_ = this;
}

void
GameEntity::setReached( std::unique_ptr<EntityFunc_Reached> newReached )
{
	retire( reachedFunc_ );
	reachedFunc_ = std::move( newReached );
	if( reachedFunc_ )
		reachedFunc_->self_ = this;
}

void
GameEntity::setBlocked( std::unique_ptr<EntityFunc_Blocked> newBlocked )
{
	retire( blockedFunc_ );
	blockedFunc_ = std::move( newBlocked );
	if( blockedFunc_ )
		blockedFunc_->self_ = this;
}

void
GameEntity::setTouch( std::unique_ptr<EntityFunc_Touch> newTouch )
{
	retire( touchFunc_ );
	touchFunc_ = std::move( newTouch );
	if( touchFunc_ )
		touchFunc_->self_ = this;
}

void
GameEntity::setUse( std::unique_ptr<EntityFunc_Use> newUse )
{
	retire( useFunc_ );
	useFunc_ = std::move( newUse );
	if( useFunc_ )
		useFunc_->self_ = this;
}

void
GameEntity::setPain( std::unique_ptr<EntityFunc_Pain> newPain )
{
	retire( painFunc_ );
	painFunc_ = std::move( newPain );
	if( painFunc_ )
		painFunc_->self_ = this;
}

void
GameEntity::setDie( std::unique_ptr<EntityFunc_Die> newDie )
{
	retire( dieFunc_ );
	dieFunc_ = std::move( newDie );
	if( dieFunc_ )
		dieFunc_->self_ = this;
}


void
GameEntity::init(int num)
{
	inuse_ = true;
	classname_ = "noclass";
	s.number = num;
	r.ownerNum = ENTITYNUM_NONE;
}

void
GameEntity::freeUp()
{
	GameHost::unlinkEntity( this );		// unlink from world

	//if( neverFree_ ) 
	//	return;

	//memset( ed, 0, sizeof(*ed) );
	cleanAll();
	classname_ = "freed";
	freetime_ = theLevel.time_;
	inuse_ = false;	
}

