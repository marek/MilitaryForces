/*
 * $Id: g_trigger.c,v 1.5 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"

void InitTrigger( GameEntity *self ) 
{
	if( !VectorCompare(self->s.angles, vec3_origin) )
		G_SetMovedir (self->s.angles, self->movedir_);

	SV_SetBrushModel( self, self->model_ );
	self->r.contents = CONTENTS_TRIGGER;		// replaces the -1 from SV_SetBrushModel
	self->r.svFlags = SVF_NOCLIENT;
}


// the wait time has passed, so set back up for another activation
struct Think_MultiWait : public GameEntity::EntityFunc_Think
{
	virtual void execute() 
	{
		self_->nextthink_ = 0;
	}
};

// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger( GameEntity *ent, GameEntity *activator ) 
{
	ent->activator_ = activator;
	if( ent->nextthink_ ) 
	{
		return;		// can't retrigger until the wait is over
	}

	if( activator->client_ ) 
	{
		if( ( ent->spawnflags_ & 1 ) &&
			activator->client_->sess_.sessionTeam_ != ClientBase::TEAM_RED ) {
			return;
		}
		if( ( ent->spawnflags_ & 2 ) &&
			activator->client_->sess_.sessionTeam_ != ClientBase::TEAM_BLUE ) {
			return;
		}
	}

	G_UseTargets(ent, ent->activator_);

	if( ent->wait_ > 0 ) 
	{
		ent->setThink(new Think_MultiWait);
		ent->nextthink_ = theLevel.time_ + ( ent->wait_ + ent->random_ * crandom() ) * 1000;
	} 
	else 
	{
		// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->setTouch(0);
		ent->nextthink_ = theLevel.time_ + FRAMETIME;
		ent->setThink(new GameEntity::EntityFunc_Free);
	}
}

struct Use_MultiUse : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator ) 
	{
		multi_trigger( self_, activator );
	}
};

struct Touch_MultiTouch : public GameEntity::EntityFunc_Touch
{
	virtual void execute( GameEntity *other, trace_t *trace ) 
	{
		if( !other->client_ ) 
			return;

		multi_trigger( self_, other );
	}
};

/*QUAKED trigger_multiple (.5 .5 .5) ?
"wait" : Seconds between triggerings, 0.5 default, -1 = one time only.
"random"	wait variance, default is 0
Variable sized repeatable trigger.  Must be targeted at one or more entities.
so, the basic time between firing is a random time between
(wait - random) and (wait + random)
*/
void SP_trigger_multiple( GameEntity *ent )
{
	G_SpawnFloat( "wait", "0.5", &ent->wait_ );
	G_SpawnFloat( "random", "0", &ent->random_ );

	if( ent->random_ >= ent->wait_ && ent->wait_ >= 0 ) 
	{
		ent->random_ = ent->wait_ - FRAMETIME;
		Com_Printf( "trigger_multiple has random >= wait\n" );
	}

	ent->setTouch(new Touch_MultiTouch);
	ent->setUse(new Use_MultiUse);

	InitTrigger( ent );
	SV_LinkEntity( ent );
}



/*
==============================================================================

trigger_always

==============================================================================
*/

struct Think_TriggerAlwaysThink : public GameEntity::EntityFunc_Think
{
	virtual void execute() 
	{
		G_UseTargets(self_, self_);
		self_->freeUp();
	}
};

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always( GameEntity *ent )
{
	// we must have some delay to make sure our use targets are present
	ent->nextthink_ = theLevel.time_ + 300;
	ent->setThink(new Think_TriggerAlwaysThink);
}


/*
==============================================================================

timer

==============================================================================
*/


/*QUAKED func_timer (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
This should be renamed trigger_timer...
Repeatedly fires its targets.
Can be turned on or off by using.

"wait"			base time between triggering all targets, default is 1
"random"		wait variance, default is 0
so, the basic time between firing is a random time between
(wait - random) and (wait + random)

*/
struct Think_FuncTimer : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		G_UseTargets (self_, self_->activator_);
		// set time before next firing
		self_->nextthink_ = theLevel.time_ + 1000 * ( self_->wait_ + crandom() * self_->random_ );
	}
};

struct Use_FuncTimer : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator ) 
	{
		self_->activator_ = activator;

		// if on, turn it off
		if( self_->nextthink_ )
		{
			self_->nextthink_ = 0;
			return;
		}

		// turn it on
        // old: func_timer_think(self_);
		if( self_->thinkFunc_ )
			self_->thinkFunc_->execute();
	}
};

void SP_func_timer( GameEntity *self ) 
{
	G_SpawnFloat( "random", "1", &self->random_);
	G_SpawnFloat( "wait", "1", &self->wait_ );

	self->setUse(new Use_FuncTimer);
	self->setThink(new Think_FuncTimer);

	if( self->random_ >= self->wait_ ) 
	{
		self->random_ = self->wait_ - FRAMETIME;
		Com_Printf( "func_timer at %s has random >= wait\n", vtos( self->s.origin ) );
	}

	if( self->spawnflags_ & 1 )
	{
		self->nextthink_ = theLevel.time_ + FRAMETIME;
		self->activator_ = self;
	}

	self->r.svFlags = SVF_NOCLIENT;
}


