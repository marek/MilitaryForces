#ifndef __G_ENTITY_H__
#define __G_ENTITY_H__

#include "q_shared.h"
#include "g_client.h"
#include "g_public.h"
#include "bg_public.h"

#include <memory>
#include <vector>


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

	struct EntityFunc_Base;
	struct EntityFunc_Free;	// derived from think to call free()

					GameEntity();
	~GameEntity() override;

	static void		releaseRetired();

	void			cleanAll();				// cleans up and sets to 0
	void			init(int num);			// initialize this ent
	void			freeUp();				// frees this entity

	// command setters (could be made into template)
	void			setThink( std::unique_ptr<EntityFunc_Think> newThink );
	void			setReached( std::unique_ptr<EntityFunc_Reached> newReached );
	void			setBlocked( std::unique_ptr<EntityFunc_Blocked> newBlocked );
	void			setTouch( std::unique_ptr<EntityFunc_Touch> newTouch );
	void			setUse( std::unique_ptr<EntityFunc_Use> newUse );
	void			setPain( std::unique_ptr<EntityFunc_Pain> newPain );
	void			setDie( std::unique_ptr<EntityFunc_Die> newDie );

	// data
	const static int k_MAX_EVENT_QUEUE_SIZE	= 20;

	GameClient*		client_;			// NULL if not a client

	bool			inuse_;

	const char*			classname_;			// set in QuakeEd
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
	Vec3			pos1_;
	Vec3			pos2_;

	char*			message_;


	char*			target_;
	char*			targetname_;
	char*			team_;
	char*			targetShaderName_;
	char*			targetShaderNewName_;
	GameEntity*		target_ent_;

	float			speed_;
	Vec3			movedir_;

	int				nextthink_;

	int				pain_debounce_time_;

	int				health_;

	bool			takedamage_;

	int				damage_;
	int				splashDamage_;	
	int				splashRadius_;
	int				methodOfDeath_;
	int				splashMethodOfDeath_;

	int				count_;

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

	void			retireAll();

	std::unique_ptr<EntityFunc_Think> thinkFunc_;
	std::unique_ptr<EntityFunc_Reached> reachedFunc_;
	std::unique_ptr<EntityFunc_Blocked> blockedFunc_;
	std::unique_ptr<EntityFunc_Touch> touchFunc_;
	std::unique_ptr<EntityFunc_Use> useFunc_;
	std::unique_ptr<EntityFunc_Pain> painFunc_;
	std::unique_ptr<EntityFunc_Die> dieFunc_;
};

struct GameEntity::EntityFunc_Base
{
	virtual ~EntityFunc_Base() {}

	static void*	operator new( std::size_t size );
	static void		operator delete( void* block, std::size_t size );

	GameEntity*	self_ = nullptr;
};

struct GameEntity::EntityFunc_Think : public GameEntity::EntityFunc_Base
{
	virtual void	execute() = 0;
};

struct GameEntity::EntityFunc_Free : public GameEntity::EntityFunc_Think
{
	void	execute() override 
	{ 
		self_->freeUp(); 
	}
};

struct GameEntity::EntityFunc_Reached : public GameEntity::EntityFunc_Base
{
	virtual void	execute() = 0;
};

struct GameEntity::EntityFunc_Blocked : public GameEntity::EntityFunc_Base
{
	virtual void	execute( GameEntity* other ) = 0;
};

struct GameEntity::EntityFunc_Touch : public GameEntity::EntityFunc_Base
{
	virtual void	execute( GameEntity* other, trace_t *trace ) = 0;
};

struct GameEntity::EntityFunc_Use : public GameEntity::EntityFunc_Base
{
	virtual void	execute( GameEntity* other, GameEntity* activator ) = 0;
};

struct GameEntity::EntityFunc_Pain : public GameEntity::EntityFunc_Base
{
	virtual void	execute( GameEntity* attacker, int damage ) = 0;
};

struct GameEntity::EntityFunc_Die : public GameEntity::EntityFunc_Base
{
	virtual void	execute( GameEntity* inflictor, GameEntity* attacker, int damage, int mod ) = 0;
};

#endif // __G_ENTITY_H__


