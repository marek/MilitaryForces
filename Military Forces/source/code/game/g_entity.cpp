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
	client_(0),
	inuse_(false),
	classname_(0),
	spawnflags_(0),
	//neverFree_(false),
	flags_(0),
	model_(0),
	model2_(0),
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
	parent_(0),
	nextTrain_(0),
	prevTrain_(0),
	message_(0),
	timestamp_(0),
	angle_(0.0f),
	target_(0),
	targetname_(0),
	team_(0),
	targetShaderName_(0),
	targetShaderNewName_(0),
	target_ent_(0),
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
	chain_(0),
	enemy_(0),
	activator_(0),
	teamchain_(0),
	teammaster_(0),
	watertype_(0),
	waterlevel_(0),
	noise_index_(0),
	wait_(0.0f),
	random_(0.0f),
	item_(0),
	ONOFF_(0),
	left_(0),
	bulletpos_(0),
	gearheight_(0.0f),
	rechargetime_(0),
	radio_target_(0),
	ent_category_(0),
	radartime_(0),
	score_(0),
	tracerindex_(0),
	idxScriptBegin_(0),
	idxScriptEnd_(0),
	tracktarget_(0),
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
	thinkFunc_(0),
	reachedFunc_(0),
	blockedFunc_(0),
	touchFunc_(0),
	useFunc_(0),
	painFunc_(0),
	dieFunc_(0)
{
	VectorClear(pos1_);
	VectorClear(pos2_);
	VectorClear(movedir_);

}

void
GameEntity::cleanAll()
{
	client_ = 0;
	inuse_ = false;
	classname_ = 0;
	spawnflags_ = 0;
	//neverFree_ = false;
	flags_ = 0;
	model_ = 0;
	model2_ = 0;
	freetime_ = 0;
	eventTime_ = 0;
	freeAfterEvent_ = 0;
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
	parent_ = 0;
	nextTrain_ = 0;
	prevTrain_ = 0;
	message_ = 0;
	timestamp_ = 0;
	angle_ = 0.0f;
	target_ = 0;
	targetname_ = 0;
	team_ = 0;
	targetShaderName_ = 0;
	targetShaderNewName_ = 0;
	target_ent_ = 0;
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
	chain_ = 0;
	enemy_ = 0;
	activator_ = 0;
	teamchain_ = 0;
	teammaster_ = 0;
	watertype_ = 0;
	waterlevel_ = 0;
	noise_index_ = 0;
	wait_ = 0.0f;
	random_ = 0.0f;
	item_ = 0;
	ONOFF_ = 0;
	left_ = 0;
	bulletpos_ = 0;
	gearheight_ = 0.0f;
	rechargetime_ = 0;
	radio_target_ = 0;
	ent_category_ = 0;
	radartime_ = 0;
	score_ = 0;
	tracerindex_ = 0;
	idxScriptBegin_ = 0;
	idxScriptEnd_ = 0;
	tracktarget_ = 0;
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
	thinkFunc_ = 0;
	reachedFunc_ = 0;
	blockedFunc_ = 0;
	touchFunc_ = 0;
	useFunc_ = 0;
	painFunc_ = 0;
	dieFunc_ = 0;
}

void
GameEntity::setThink( EntityFunc_Think* newThink )
{
	if( thinkFunc_ )
		delete thinkFunc_;
	thinkFunc_ = newThink;
	if( newThink )
		thinkFunc_->self_ = this;
}

void
GameEntity::setReached( EntityFunc_Reached* newReached )
{
	if( reachedFunc_ )
		delete reachedFunc_;
	reachedFunc_ = newReached;
	if( newReached )
		reachedFunc_->self_ = this;
}

void
GameEntity::setBlocked( EntityFunc_Blocked* newBlocked )
{
	if( blockedFunc_ )
		delete blockedFunc_;
	blockedFunc_ = newBlocked;
	if( newBlocked )
		blockedFunc_->self_ = this;
}

void
GameEntity::setTouch( EntityFunc_Touch* newTouch )
{
	if( touchFunc_ )
		delete touchFunc_;
	touchFunc_ = newTouch;
	if( newTouch )
		touchFunc_->self_ = this;
}

void
GameEntity::setUse( EntityFunc_Use* newUse )
{
	if( useFunc_ )
		delete useFunc_;
	useFunc_ = newUse;
	if( newUse )
		useFunc_->self_ = this;
}

void
GameEntity::setPain( EntityFunc_Pain* newPain )
{
	if( painFunc_ )
		delete painFunc_;
	painFunc_ = newPain;
	if( newPain )
		painFunc_->self_ = this;
}

void
GameEntity::setDie( EntityFunc_Die* newDie )
{
	if( dieFunc_ )
		delete dieFunc_;
	dieFunc_ = newDie;
	if( newDie )
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
	SV_UnlinkEntity( this );		// unlink from world

	//if( neverFree_ ) 
	//	return;

	//memset( ed, 0, sizeof(*ed) );
	cleanAll();
	classname_ = "freed";
	freetime_ = theLevel.time_;
	inuse_ = false;	
}

