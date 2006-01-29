/*
 * $Id: g_target.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"


//==========================================================

/*QUAKED target_delay (1 0 0) (-8 -8 -8) (8 8 8)
"wait" seconds to pause before firing targets.
"random" delay variance, total delay = delay +/- random seconds
*/
struct Think_TargetDelay : public GameEntity::EntityFunc_Think
{
	virtual void execute() 
	{
		G_UseTargets( self_, self_->activator_ );
	}
};

struct Use_TargetDelay : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator )
	{
		self_->nextthink_ = theLevel.time_ + ( self_->wait_ + self_->random_ * crandom() ) * 1000;
		self_->setThink(new Think_TargetDelay);
		self_->activator_ = activator;
	}
};

void SP_target_delay( GameEntity *ent ) 
{
	// check delay for backwards compatability
	if( !G_SpawnFloat( "delay", "0", &ent->wait_ ) ) 
		G_SpawnFloat( "wait", "1", &ent->wait_ );

	if ( !ent->wait_ ) 
		ent->wait_ = 1;

	ent->setUse(new Use_TargetDelay);
}


//==========================================================

/*QUAKED target_score (1 0 0) (-8 -8 -8) (8 8 8)
"count" number of points to add, default 1

The activator is given this many points.
*/
struct Use_TargetScore : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator) 
	{
		AddScore( activator, self_->r.currentOrigin, self_->count_ );
	}
};

void SP_target_score( GameEntity *ent ) 
{
	if( !ent->count_ ) 
		ent->count_ = 1;

	ent->setUse(new Use_TargetScore);
}


//==========================================================

/*QUAKED target_print (1 0 0) (-8 -8 -8) (8 8 8) redteam blueteam private
"message"	text to print
If "private", only the activator gets the message.  If no checks, all clients get the message.
*/
struct Use_TargetPrint : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator ) 
	{
		if( activator->client_ && ( self_->spawnflags_ & 4 ) ) 
		{
			SV_GameSendServerCommand( activator->s.number, va("cp \"%s\"", self_->message_ ));
			return;
		}

		if( self_->spawnflags_ & 3 ) 
		{
			if ( self_->spawnflags_ & 1 ) 
				G_TeamCommand( ClientBase::TEAM_RED, va("cp \"%s\"", self_->message_) );
			if ( self_->spawnflags_ & 2 ) 
				G_TeamCommand( ClientBase::TEAM_BLUE, va("cp \"%s\"", self_->message_) );
			return;
		}

		SV_GameSendServerCommand( -1, va("cp \"%s\"", self_->message_ ));
	}
};

void SP_target_print( GameEntity *ent ) 
{
	ent->setUse(new Use_TargetPrint);
}


//==========================================================


/*QUAKED target_speaker (1 0 0) (-8 -8 -8) (8 8 8) looped-on looped-off global activator
"noise"		wav file to play

A global sound will play full volume throughout the level.
Activator sounds will play on the player that activated the target.
Global and activator sounds can't be combined with looping.
Normal sounds play each time the target is used.
Looped sounds will be toggled by use functions.
Multiple identical looping sounds will just increase volume without any speed cost.
"wait" : Seconds between auto triggerings, 0 = don't auto trigger
"random"	wait variance, default is 0
*/
struct Use_TargetSpeaker : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator ) 
	{
		if( self_->spawnflags_ & 3 ) 
		{	
			// looping sound toggles
			if( self_->s.loopSound )
				self_->s.loopSound = 0;	// turn it off
			else
				self_->s.loopSound = self_->noise_index_;	// start it
		}
		else 
		{	
			// normal sound
			if ( self_->spawnflags_ & 8 ) 
				G_AddEvent( activator, EV_GENERAL_SOUND, self_->noise_index_, true );
			else if (self_->spawnflags_ & 4) 
				G_AddEvent( self_, EV_GLOBAL_SOUND, self_->noise_index_, true );
			else 
				G_AddEvent( self_, EV_GENERAL_SOUND, self_->noise_index_, true );
		}
	}
};

void SP_target_speaker( GameEntity *ent ) 
{
	char	buffer[MAX_QPATH];
	char	*s;

	G_SpawnFloat( "wait", "0", &ent->wait_ );
	G_SpawnFloat( "random", "0", &ent->random_ );

	if ( !G_SpawnString( "noise", "NOSOUND", &s ) ) 
		Com_Error( ERR_DROP, "target_speaker without a noise key at %s", vtos( ent->s.origin ) );

	// force all client reletive sounds to be "activator" speakers that
	// play on the entity that activates it
	if ( s[0] == '*' ) 
		ent->spawnflags_ |= 8;

	if (!strstr( s, ".wav" )) 
		Com_sprintf (buffer, sizeof(buffer), "%s.wav", s );
	else 
		Q_strncpyz( buffer, s, sizeof(buffer) );

	ent->noise_index_ = G_SoundIndex(buffer);

	// a repeating speaker can be done completely client side
	ent->s.eType = ET_SPEAKER;
	ent->s.eventParm = ent->noise_index_;
	ent->s.frame = ent->wait_ * 10;
	ent->s.clientNum = ent->random_ * 10;


	// check for prestarted looping sound
	if( ent->spawnflags_ & 1 ) 
		ent->s.loopSound = ent->noise_index_;

	ent->setUse(new Use_TargetSpeaker);

	if( ent->spawnflags_ & 4 ) 
		ent->r.svFlags |= SVF_BROADCAST;

	VectorCopy( ent->s.origin, ent->s.pos.trBase );

	// must link the entity so we get areas and clusters so
	// the server can determine who to send updates to
	SV_LinkEntity( ent );
}


//==========================================================


/*QUAKED target_relay (.5 .5 .5) (-8 -8 -8) (8 8 8) RED_ONLY BLUE_ONLY RANDOM
This doesn't perform any actions except fire its targets.
The activator can be forced to be from a certain team.
if RANDOM is checked, only one of the targets will be fired, not all of them
*/

struct Use_TargetRelay : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator ) 
	{
		if( ( self_->spawnflags_ & 1 ) && activator->client_ 
			&& activator->client_->sess_.sessionTeam_ != ClientBase::TEAM_RED ) 
		{
			return;
		}
		if( ( self_->spawnflags_ & 2 ) && activator->client_ 
			&& activator->client_->sess_.sessionTeam_ != ClientBase::TEAM_BLUE )
		{
			return;
		}
		if( self_->spawnflags_ & 4 ) 
		{
			GameEntity	*ent = G_PickTarget( self_->target_ );
			if( ent && ent->useFunc_ ) 
				ent->useFunc_->execute( self_, activator );
			return;
		}
		G_UseTargets (self_, activator);
	}
};

void SP_target_relay (GameEntity *self)
{
	self->setUse(new Use_TargetRelay);
}


//==========================================================

/*QUAKED target_kill (.5 .5 .5) (-8 -8 -8) (8 8 8)
Kills the activator.
*/
struct Use_TargetKill : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator ) 
	{
		G_Damage ( activator, NULL, NULL, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG, CAT_ANY);
	}
};

void SP_target_kill( GameEntity *self ) 
{
	self->setUse(new Use_TargetKill);
}

/*QUAKED target_position (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
*/
void SP_target_position( GameEntity *self )
{
	G_SetOrigin( self, self->s.origin );
}

struct Think_TargetLocationLinkup : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		if (theLevel.locationLinked_) 
			return;

		theLevel.locationLinked_ = true;

		theLevel.locationHead_ = 0;

		SV_SetConfigstring( CS_LOCATIONS, "unknown" );

		for( int i = 1, n = 1; i <= theLevel.num_entities_; i++ ) 
		{
			self_ = theLevel.getEntity(i);
			if( self_ && self_->classname_ && !Q_stricmp(self_->classname_, "target_location") ) 
			{
				// lets overload some variables!
				self_->health_ = n; // use for location marking
				SV_SetConfigstring( CS_LOCATIONS + n, self_->message_ );
				n++;
				self_->nextTrain_ = theLevel.locationHead_;
				theLevel.locationHead_ = self_;
			}
		}

		// All linked together now
	}
};

/*QUAKED target_location (0 0.5 0) (-8 -8 -8) (8 8 8)
Set "message" to the name of this location.
Set "count" to 0-7 for color.
0:white 1:red 2:green 3:yellow 4:blue 5:cyan 6:magenta 7:white

Closest target_location in sight used for the location, if none
in site, closest in distance
*/
void SP_target_location( GameEntity *self )
{
	self->setThink(new Think_TargetLocationLinkup);
	self->nextthink_ = theLevel.time_ + 200;  // Let them all spawn first

	G_SetOrigin( self, self->s.origin );
}

