#ifndef __G_ENTITY_H__
#define __G_ENTITY_H__

#include "q_shared.h"
#include "g_client.h"
#include "g_public.h"
#include "bg_public.h"


struct GameEntity : public EntityBase
{
	struct Event
	{
				Event();

		int		event_;
		int		eventParm_;
		bool	used_;
	};	

	enum eMoverState
	{
		MOVER_POS1,
		MOVER_POS2,
		MOVER_1TO2,
		MOVER_2TO1
	};
	
	struct EntityFunc_Think;
	struct EntityFunc_Reached;
	struct EntityFunc_Blocked;
	struct EntityFunc_Touch;
	struct EntityFunc_Use;
	struct EntityFunc_Pain;
	struct EntityFunc_Die;

	struct EntityFunc_Free;	// derived from think to call free()

					GameEntity();
	virtual			~GameEntity() {}

	void			cleanAll();				// cleans up and sets to 0
	void			init(int num);			// initialize this ent
	void			freeUp();				// frees this entity

	// command setters (could be made into template)
	void			setThink( EntityFunc_Think* newThink );
	void			setReached( EntityFunc_Reached* newReached );
	void			setBlocked( EntityFunc_Blocked* newBlocked );
	void			setTouch( EntityFunc_Touch* newTouch );
	void			setUse( EntityFunc_Use* newUse );
	void			setPain( EntityFunc_Pain* newPain );
	void			setDie( EntityFunc_Die* newDie );

	// data
	const static int k_MAX_EVENT_QUEUE_SIZE	= 20;

	GameClient*		client_;			// NULL if not a client

	bool			inuse_;

	char*			classname_;			// set in QuakeEd
	int				spawnflags_;		// set in QuakeEd

	//bool			neverFree_;			// if true, FreeEntity will only unlink
	//									// bodyque uses this

	int				flags_;				// FL_* variables

	char*			model_;
	char*			model2_;
	int				freetime_;			// level.time when the object was freed
	
	int				eventTime_;			// events will be cleared EVENT_VALID_MSEC after set
	bool			freeAfterEvent_;
	bool			unlinkAfterEvent_;

	bool			physicsObject_;		// if true, it can be pushed by movers and fall off edges
										// all game items are physicsObjects, 
	float			physicsBounce_;		// 1.0 = continuous bounce, 0.0 = no bounce
	int				clipmask_;			// brushes with this content value will be collided against
										// when moving.  items and corpses do not collide against
										// players, for instance

	// movers
	eMoverState		moverState_;
	int				soundPos1_;
	int				sound1to2_;
	int				sound2to1_;
	int				soundPos2_;
	int				soundLoop_;
	GameEntity*		parent_;
	GameEntity*		nextTrain_;
	GameEntity*		prevTrain_;
	vec3_t			pos1_;
	vec3_t			pos2_;

	char*			message_;

	int				timestamp_;			// body queue sinking, etc

	float			angle_;				// set in editor, -1 = up, -2 = down
	char*			target_;
	char*			targetname_;
	char*			team_;
	char*			targetShaderName_;
	char*			targetShaderNewName_;
	GameEntity*		target_ent_;

	float			speed_;
	vec3_t			movedir_;

	int				nextthink_;

	int				pain_debounce_time_;
	int				fly_sound_debounce_time_;	// wind tunnel
	int				last_move_time_;

	int				health_;

	bool			takedamage_;

	int				damage_;
	int				splashDamage_;	
	int				splashRadius_;
	int				methodOfDeath_;
	int				splashMethodOfDeath_;

	int				count_;

	GameEntity*		chain_;
	GameEntity*		enemy_;
	GameEntity*		activator_;
	GameEntity*		teamchain_;			// next entity in team
	GameEntity*		teammaster_;		// master of the team

	int				watertype_;
	int				waterlevel_;

	int				noise_index_;

	// timing variables
	float			wait_;
	float			random_;

	gitem_t*		item_;				// for bonus items

	// MFQ3
	int				ONOFF_;				// to remember status after leaving vehicle
	int				left_;				// for projectiles
	int				bulletpos_;			// for dual guns (or more)
	float			gearheight_;	
	int				rechargetime_;		// when to recharge next
	GameEntity*		radio_target_;
	int				ent_category_;
	int				radartime_;
	int				score_;				// for func_explosive teamplay count
	int				tracerindex_;		// for tracers
	int				idxScriptBegin_;	// first script for this ent
	int				idxScriptEnd_;		// last script for this ent
	//waypoint_t*		nextWaypoint_;
	GameEntity*		tracktarget_;
	int				locktime_;
	float			targetcat_;			// for weapons mfq3
	float			catmodifier_;		// for weapons mfq3
	float			range_;				// for weapons mfq3
	float			followcone_;		// for weapons mfq3
	int				lastDist_;
	bool			updateGear_;
	bool			updateBay_;
	completeLoadout_t loadout_;
	bool			eventSent_;
	int				currentEventToSend_;
	int				currentEventToAdd_;
	Event			eventQueue_[k_MAX_EVENT_QUEUE_SIZE];
	int				gi_nextScanTime_;
	float			gi_lockangle_;
	int				gi_reloadTime_;
	int				basicECMVulnerability_;
	int				nukeTime_;			// For nuke effect/damage
	int				nukeShockTime_;		// For nuke effect/damage

	//virtual void	think() = 0;
	//virtual void	reached() = 0;
	//virtual void	blocked( GameEntity* other ) = 0;
	//virtual void	touch( GameEntity* other, trace_t *trace ) = 0;
	//virtual void	use( GameEntity* other, GameEntity* activator ) = 0;
	//virtual void	pain( GameEntity* attacker, int damage ) = 0;
	//virtual void	die( GameEntity* inflictor, GameEntity* attacker, int damage, int mod ) = 0;

	EntityFunc_Think* thinkFunc_;
	EntityFunc_Reached* reachedFunc_;
	EntityFunc_Blocked* blockedFunc_;
	EntityFunc_Touch* touchFunc_;
	EntityFunc_Use* useFunc_;
	EntityFunc_Pain* painFunc_;
	EntityFunc_Die* dieFunc_;
};

struct GameEntity::EntityFunc_Think
{
	EntityFunc_Think() : self_(0) {}

	virtual void	execute() = 0;

	GameEntity*	self_;
};

struct GameEntity::EntityFunc_Free : public GameEntity::EntityFunc_Think
{
	virtual void	execute() 
	{ 
		self_->freeUp(); 
	}
};

struct GameEntity::EntityFunc_Reached
{
	EntityFunc_Reached() : self_(0) {}

	virtual void	execute() = 0;

	GameEntity*	self_;
};

struct GameEntity::EntityFunc_Blocked
{
	EntityFunc_Blocked() : self_(0) {}

	virtual void	execute( GameEntity* other ) = 0;

	GameEntity*	self_;
};

struct GameEntity::EntityFunc_Touch
{
	EntityFunc_Touch() : self_(0) {}

	virtual void	execute( GameEntity* other, trace_t *trace ) = 0;

	GameEntity*	self_;
};

struct GameEntity::EntityFunc_Use
{
	EntityFunc_Use() : self_(0) {}

	virtual void	execute( GameEntity* other, GameEntity* activator ) = 0;

	GameEntity*	self_;
};

struct GameEntity::EntityFunc_Pain
{
	EntityFunc_Pain() : self_(0) {}

	virtual void	execute( GameEntity* attacker, int damage ) = 0;

	GameEntity*	self_;
};

struct GameEntity::EntityFunc_Die
{
	EntityFunc_Die() : self_(0) {}

	virtual void	execute( GameEntity* inflictor, GameEntity* attacker, int damage, int mod ) = 0;

	GameEntity*	self_;
};

#endif // __G_ENTITY_H__


