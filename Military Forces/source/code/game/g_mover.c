/*
 * $Id: g_mover.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"



/*
===============================================================================

PUSHMOVE

===============================================================================
*/

void MatchTeam( GameEntity *teamLeader, int moverState, int time );

typedef struct 
{
	GameEntity	*ent;
	vec3_t	origin;
	vec3_t	angles;
	float	deltayaw;
} pushed_t;
pushed_t	pushed[MAX_GENTITIES], *pushed_p;


/*
============
G_TestEntityPosition

============
*/
GameEntity* G_TestEntityPosition( GameEntity *ent ) 
{
	trace_t	tr;
	int		mask;

	if ( ent->clipmask_ ) 
		mask = ent->clipmask_;
	else 
		mask = MASK_SOLID;

	if ( ent->client_ ) 
		SV_Trace( &tr, ent->client_->ps_.origin, ent->r.mins, ent->r.maxs, ent->client_->ps_.origin, ent->s.number, mask, false );
	else 
		SV_Trace( &tr, ent->s.pos.trBase, ent->r.mins, ent->r.maxs, ent->s.pos.trBase, ent->s.number, mask, false );
	
	if (tr.startsolid)
		return theLevel.getEntity(tr.entityNum);// &g_entities[ tr.entityNum ];
		
	return NULL;
}


/*
==================
G_TryPushingEntity

Returns false if the move is blocked
==================
*/
bool G_TryPushingEntity( GameEntity *check, GameEntity *pusher, vec3_t move, vec3_t amove ) 
{
	vec3_t		forward, right, up;
	vec3_t		org, org2, move2;
	GameEntity	*block;

	// EF_MOVER_STOP will just stop when contacting another entity
	// instead of pushing it, but entities can still ride on top of it
	if( ( pusher->s.eFlags & EF_MOVER_STOP ) && 
		check->s.groundEntityNum != pusher->s.number ) 
		return false;

	// save off the old position
	if( pushed_p > &pushed[MAX_GENTITIES] ) 
		Com_Error( ERR_DROP, "pushed_p > &pushed[MAX_GENTITIES]" );

	pushed_p->ent = check;
	VectorCopy (check->s.pos.trBase, pushed_p->origin);
	VectorCopy (check->s.apos.trBase, pushed_p->angles);
	if( check->client_ ) 
	{
		pushed_p->deltayaw = check->client_->ps_.delta_angles[YAW];
		VectorCopy (check->client_->ps_.origin, pushed_p->origin);
	}
	pushed_p++;

	// we need this for pushing things later
	VectorSubtract (vec3_origin, amove, org);
	AngleVectors (org, forward, right, up);

	// try moving the contacted entity 
	VectorAdd (check->s.pos.trBase, move, check->s.pos.trBase);
	if( check->client_ ) 
	{
		// make sure the client's view rotates when on a rotating mover
		check->client_->ps_.delta_angles[YAW] += ANGLE2SHORT(amove[YAW]);
	}

	// figure movement due to the pusher's amove
	VectorSubtract (check->s.pos.trBase, pusher->r.currentOrigin, org);
	org2[0] = DotProduct (org, forward);
	org2[1] = -DotProduct (org, right);
	org2[2] = DotProduct (org, up);
	VectorSubtract (org2, org, move2);
	VectorAdd (check->s.pos.trBase, move2, check->s.pos.trBase);
	if( check->client_ ) 
	{
		VectorAdd (check->client_->ps_.origin, move, check->client_->ps_.origin);
		VectorAdd (check->client_->ps_.origin, move2, check->client_->ps_.origin);
	}

	// may have pushed them off an edge
	if( check->s.groundEntityNum != pusher->s.number ) 
		check->s.groundEntityNum = -1;

	block = G_TestEntityPosition( check );
	if( !block ) 
	{
		// pushed ok
		if( check->client_ ) 
			VectorCopy( check->client_->ps_.origin, check->r.currentOrigin );
		else 
			VectorCopy( check->s.pos.trBase, check->r.currentOrigin );
		SV_LinkEntity( check );
		return true;
	}

	// if it is ok to leave in the old position, do it
	// this is only relevent for riding entities, not pushed
	// Sliding trapdoors can cause this.
	VectorCopy( (pushed_p-1)->origin, check->s.pos.trBase);
	if ( check->client_ ) 
		VectorCopy( (pushed_p-1)->origin, check->client_->ps_.origin);

	VectorCopy( (pushed_p-1)->angles, check->s.apos.trBase );
	block = G_TestEntityPosition (check);
	if ( !block ) 
	{
		check->s.groundEntityNum = -1;
		pushed_p--;
		return true;
	}

	// blocked
	return false;
}

/*
============
G_MoverPush

Objects need to be moved back on a failed push,
otherwise riders would continue to slide.
If false is returned, *obstacle will be the blocking entity
============
*/
bool G_MoverPush( GameEntity *pusher, vec3_t move, vec3_t amove, GameEntity **obstacle ) 
{
	int			i, e;
	GameEntity	*check;
	vec3_t		mins, maxs;
	pushed_t	*p;
	int			entityList[MAX_GENTITIES];
	int			listedEntities;
	vec3_t		totalMins, totalMaxs;

	*obstacle = NULL;


	// mins/maxs are the bounds at the destination
	// totalMins / totalMaxs are the bounds for the entire move
	if ( pusher->r.currentAngles[0] || pusher->r.currentAngles[1] || pusher->r.currentAngles[2]
		|| amove[0] || amove[1] || amove[2] ) 
	{
		float		radius;

		radius = RadiusFromBounds( pusher->r.mins, pusher->r.maxs );
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			mins[i] = pusher->r.currentOrigin[i] + move[i] - radius;
			maxs[i] = pusher->r.currentOrigin[i] + move[i] + radius;
			totalMins[i] = mins[i] - move[i];
			totalMaxs[i] = maxs[i] - move[i];
		}
	} 
	else 
	{
		for (i=0 ; i<3 ; i++) 
		{
			mins[i] = pusher->r.absmin[i] + move[i];
			maxs[i] = pusher->r.absmax[i] + move[i];
		}

		VectorCopy( pusher->r.absmin, totalMins );
		VectorCopy( pusher->r.absmax, totalMaxs );
		for (i=0 ; i<3 ; i++) 
		{
			if ( move[i] > 0 ) 
				totalMaxs[i] += move[i];
			else 
				totalMins[i] += move[i];
		}
	}

	// unlink the pusher so we don't get it in the entityList
	SV_UnlinkEntity( pusher );

	listedEntities = SV_AreaEntities( totalMins, totalMaxs, entityList, MAX_GENTITIES );

	// move the pusher to it's final position
	VectorAdd( pusher->r.currentOrigin, move, pusher->r.currentOrigin );
	VectorAdd( pusher->r.currentAngles, amove, pusher->r.currentAngles );
	SV_LinkEntity( pusher );

	// see if any solid entities are inside the final position
	for ( e = 0 ; e < listedEntities ; e++ ) 
	{
		check = theLevel.getEntity(entityList[e]);// &g_entities[ entityList[ e ] ];

		// only push items and players
		if ( !check || check->s.eType != ET_ITEM && check->s.eType != ET_PLAYER && !check->physicsObject_ ) 
			continue;

		// if the entity is standing on the pusher, it will definitely be moved
		if ( check->s.groundEntityNum != pusher->s.number ) 
		{
			// see if the ent needs to be tested
			if ( check->r.absmin[0] >= maxs[0]
			|| check->r.absmin[1] >= maxs[1]
			|| check->r.absmin[2] >= maxs[2]
			|| check->r.absmax[0] <= mins[0]
			|| check->r.absmax[1] <= mins[1]
			|| check->r.absmax[2] <= mins[2] ) 
				continue;

			// see if the ent's bbox is inside the pusher's final position
			// this does allow a fast moving object to pass through a thin entity...
			if (!G_TestEntityPosition (check)) 
				continue;
		}

		// the entity needs to be pushed
		if ( G_TryPushingEntity( check, pusher, move, amove ) ) 
			continue;

		// the move was blocked an entity

		// bobbing entities are instant-kill and never get blocked
		if ( pusher->s.pos.trType == TR_SINE || pusher->s.apos.trType == TR_SINE )
		{
			G_Damage( check, pusher, pusher, NULL, NULL, 99999, 0, MOD_CRUSH, CAT_ANY );
			continue;
		}

		
		// save off the obstacle so we can call the block function (crush, etc)
		*obstacle = check;

		// move back any entities we already moved
		// go backwards, so if the same entity was pushed
		// twice, it goes back to the original position
		for ( p=pushed_p-1 ; p>=pushed ; p-- ) 
		{
			VectorCopy (p->origin, p->ent->s.pos.trBase);
			VectorCopy (p->angles, p->ent->s.apos.trBase);
			if ( p->ent->client_ ) 
			{
				p->ent->client_->ps_.delta_angles[YAW] = p->deltayaw;
				VectorCopy (p->origin, p->ent->client_->ps_.origin);
			}
			SV_LinkEntity( p->ent );
		}
		return false;
	}

	return true;
}


/*
=================
G_MoverTeam
=================
*/
void G_MoverTeam( GameEntity *ent ) 
{
	vec3_t		move, amove;
	GameEntity	*part, *obstacle;
	vec3_t		origin, angles;

	obstacle = NULL;

	// make sure all team slaves can move before commiting
	// any moves or calling any think functions
	// if the move is blocked, all moved objects will be backed out
	pushed_p = pushed;
	for (part = ent ; part ; part=part->teamchain_) 
	{
		// get current position
		BG_EvaluateTrajectory( &part->s.pos, theLevel.time_, origin );
		BG_EvaluateTrajectory( &part->s.apos, theLevel.time_, angles );
		VectorSubtract( origin, part->r.currentOrigin, move );
		VectorSubtract( angles, part->r.currentAngles, amove );
		if ( !G_MoverPush( part, move, amove, &obstacle ) ) 
			break;	// move was blocked
	}

	if (part) 
	{
		// go back to the previous position
		for ( part = ent ; part ; part = part->teamchain_ ) 
		{
			part->s.pos.trTime += theLevel.time_ - theLevel.previousTime_;
			part->s.apos.trTime += theLevel.time_ - theLevel.previousTime_;
			BG_EvaluateTrajectory( &part->s.pos, theLevel.time_, part->r.currentOrigin );
			BG_EvaluateTrajectory( &part->s.apos, theLevel.time_, part->r.currentAngles );
			SV_LinkEntity( part );
		}

		// if the pusher has a "blocked" function, call it
		if( ent->blockedFunc_ ) 
			ent->blockedFunc_->execute( obstacle );

		return;
	}

	// the move succeeded
	for( part = ent ; part ; part = part->teamchain_ ) 
	{
		// call the reached function if time is at or past end point
		if( part->s.pos.trType == TR_LINEAR_STOP ) {
			if( theLevel.time_ >= part->s.pos.trTime + part->s.pos.trDuration ) {
				if( part->reachedFunc_ ) 
					part->reachedFunc_->execute();
			}
		}
	}
}

/*
================
G_RunMover

================
*/
void G_RunMover( GameEntity *ent ) 
{
	// if not a team captain, don't do anything, because
	// the captain will handle everything
	if ( ent->flags_ & FL_TEAMSLAVE ) 
		return;

	// if stationary at one of the positions, don't move anything
	if ( ent->s.pos.trType != TR_STATIONARY || ent->s.apos.trType != TR_STATIONARY ) 
		G_MoverTeam( ent );

	// check think function
	G_RunThink( ent );
}

/*
============================================================================

GENERAL MOVERS

Doors, plats, and buttons are all binary (two position) movers
Pos1 is "at rest", pos2 is "activated"
============================================================================
*/

/*
===============
SetMoverState
===============
*/
void SetMoverState( GameEntity *ent, GameEntity::eMoverState moverState, int time )
{
	vec3_t			delta;
	float			f;

	ent->moverState_ = moverState;

	ent->s.pos.trTime = time;
	switch( moverState ) 
	{
	case GameEntity::MOVER_POS1:
		VectorCopy( ent->pos1_, ent->s.pos.trBase );
		ent->s.pos.trType = TR_STATIONARY;
		break;
	case GameEntity::MOVER_POS2:
		VectorCopy( ent->pos2_, ent->s.pos.trBase );
		ent->s.pos.trType = TR_STATIONARY;
		break;
	case GameEntity::MOVER_1TO2:
		VectorCopy( ent->pos1_, ent->s.pos.trBase );
		VectorSubtract( ent->pos2_, ent->pos1_, delta );
		f = 1000.0 / ent->s.pos.trDuration;
		VectorScale( delta, f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_LINEAR_STOP;
		break;
	case GameEntity::MOVER_2TO1:
		VectorCopy( ent->pos2_, ent->s.pos.trBase );
		VectorSubtract( ent->pos1_, ent->pos2_, delta );
		f = 1000.0 / ent->s.pos.trDuration;
		VectorScale( delta, f, ent->s.pos.trDelta );
		ent->s.pos.trType = TR_LINEAR_STOP;
		break;
	}
	BG_EvaluateTrajectory( &ent->s.pos, theLevel.time_, ent->r.currentOrigin );	
	SV_LinkEntity( ent );
}

/*
================
MatchTeam

All entities in a mover team will move from pos1 to pos2
in the same amount of time
================
*/
void MatchTeam( GameEntity *teamLeader, int moverState, int time ) 
{
	GameEntity		*slave;

	for ( slave = teamLeader ; slave ; slave = slave->teamchain_ ) 
	{
		SetMoverState( slave, static_cast<GameEntity::eMoverState>(moverState), time );
	}
}



/*
================
ReturnToPos1
================
*/
struct Think_ReturnToPos1 : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		MatchTeam( self_, GameEntity::MOVER_2TO1, theLevel.time_ );

		// looping sound
		self_->s.loopSound = self_->soundLoop_;

		// starting sound
		if ( self_->sound2to1_ ) 
			G_AddEvent( self_, EV_GENERAL_SOUND, self_->sound2to1_, true );
	}
};

/*
================
Reached_BinaryMover
================
*/
struct Reached_BinaryMover : public GameEntity::EntityFunc_Reached
{
	virtual void execute()
	{
		// stop the looping sound
		self_->s.loopSound = self_->soundLoop_;

		if ( self_->moverState_ == GameEntity::MOVER_1TO2 ) 
		{
			// reached pos2
			SetMoverState( self_, GameEntity::MOVER_POS2, theLevel.time_ );

			// play sound
			if ( self_->soundPos2_ ) 
				G_AddEvent( self_, EV_GENERAL_SOUND, self_->soundPos2_, true );

			// return to pos1 after a delay
			self_->setThink(new Think_ReturnToPos1);
			self_->nextthink_ = theLevel.time_ + self_->wait_;

			// fire targets
			if ( !self_->activator_ ) 
				self_->activator_ = self_;

			G_UseTargets( self_, self_->activator_ );
		} 
		else if ( self_->moverState_ == GameEntity::MOVER_2TO1 ) 
		{
			// reached pos1
			SetMoverState( self_, GameEntity::MOVER_POS1, theLevel.time_ );

			// play sound
			if ( self_->soundPos1_ ) 
				G_AddEvent( self_, EV_GENERAL_SOUND, self_->soundPos1_, true );

			// close areaportals
			if ( self_->teammaster_ == self_ || !self_->teammaster_ ) 
				SV_AdjustAreaPortalState( self_, false );

		} 
		else 
			Com_Error( ERR_DROP, "Reached_BinaryMover: bad moverState" );
	}
};

/*
================
Use_BinaryMover
================
*/
void Use_BinaryMoverImpl( GameEntity *ent, GameEntity *other, GameEntity *activator )
{
	int		total;
	int		partial;

	// only the master should be used
	if ( ent->flags_ & FL_TEAMSLAVE ) 
	{
		Use_BinaryMoverImpl( ent->teammaster_, other, activator );
		return;
	}

	ent->activator_ = activator;

	if ( ent->moverState_ == GameEntity::MOVER_POS1 ) {
		// start moving 50 msec later, becase if this was player
		// triggered, level.time hasn't been advanced yet
		MatchTeam( ent, GameEntity::MOVER_1TO2, theLevel.time_ + 50 );

		// starting sound
		if ( ent->sound1to2_ ) 
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound1to2_, true );

		// looping sound
		ent->s.loopSound = ent->soundLoop_;

		// open areaportal
		if ( ent->teammaster_ == ent || !ent->teammaster_ ) 
			SV_AdjustAreaPortalState( ent, true );
		return;
	}

	// if all the way up, just delay before coming down
	if ( ent->moverState_ == GameEntity::MOVER_POS2 ) 
	{
		ent->nextthink_ = theLevel.time_ + ent->wait_;
		return;
	}

	// only partway down before reversing
	if ( ent->moverState_ == GameEntity::MOVER_2TO1 ) 
	{
		total = ent->s.pos.trDuration;
		partial = theLevel.time_ - ent->s.pos.trTime;
		if ( partial > total ) 
			partial = total;

		MatchTeam( ent, GameEntity::MOVER_1TO2, theLevel.time_ - ( total - partial ) );

		if ( ent->sound1to2_ ) 
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound1to2_, true );
		return;
	}

	// only partway up before reversing
	if ( ent->moverState_ == GameEntity::MOVER_1TO2 ) 
	{
		total = ent->s.pos.trDuration;
		partial = theLevel.time_ - ent->s.pos.trTime;
		if ( partial > total ) 
			partial = total;

		MatchTeam( ent, GameEntity::MOVER_2TO1, theLevel.time_ - ( total - partial ) );

		if ( ent->sound2to1_ ) 
			G_AddEvent( ent, EV_GENERAL_SOUND, ent->sound2to1_, true );

		return;
	}
}

struct Use_BinaryMover : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator )
	{
		Use_BinaryMoverImpl( self_, other, activator );
	}
};


/*
================
InitMover

"pos1", "pos2", and "speed" should be set before calling,
so the movement delta can be calculated
================
*/
void InitMover( GameEntity *ent ) 
{
	vec3_t		move;
	float		distance;
	float		light;
	vec3_t		color;
	bool		lightSet, colorSet;
	char		*sound;

	// if the "model2" key is set, use a seperate model
	// for drawing, but clip against the brushes
	if ( ent->model2_ ) {
		ent->s.modelindex2 = G_ModelIndex( ent->model2_ );
	}

	// if the "loopsound" key is set, use a constant looping sound when moving
	if ( G_SpawnString( "noise", "100", &sound ) ) 
		ent->s.loopSound = G_SoundIndex( sound );

	// if the "color" or "light" keys are set, setup constantLight
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnVector( "color", "1 1 1", color );
	if ( lightSet || colorSet ) {
		int		r, g, b, i;

		r = color[0] * 255;
		if ( r > 255 ) {
			r = 255;
		}
		g = color[1] * 255;
		if ( g > 255 ) {
			g = 255;
		}
		b = color[2] * 255;
		if ( b > 255 ) {
			b = 255;
		}
		i = light / 4;
		if ( i > 255 ) {
			i = 255;
		}
//		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
	}


	ent->setUse(new Use_BinaryMover);
	ent->setReached(new Reached_BinaryMover);

	ent->moverState_ = GameEntity::MOVER_POS1;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_MOVER;
	VectorCopy (ent->pos1_, ent->r.currentOrigin);
	SV_LinkEntity( ent );

	ent->s.pos.trType = TR_STATIONARY;
	VectorCopy( ent->pos1_, ent->s.pos.trBase );

	// calculate time to reach second position from speed
	VectorSubtract( ent->pos2_, ent->pos1_, move );
	distance = VectorLength( move );
	if ( !ent->speed_ ) 
		ent->speed_ = 100;

	VectorScale( move, ent->speed_, ent->s.pos.trDelta );
	ent->s.pos.trDuration = distance * 1000 / ent->speed_;
	if ( ent->s.pos.trDuration <= 0 ) 
		ent->s.pos.trDuration = 1;
}


/*
===============================================================================

DOOR

A use can be triggered either by a touch function, by being shot, or by being
targeted by another entity.

===============================================================================
*/

/*
================
Blocked_Door
================
*/
struct Blocked_DoorBlocked : public GameEntity::EntityFunc_Blocked
{
	void execute( GameEntity *other )
	{
		// remove anything other than a client
		if ( !other->client_ ) 
		{
			// except CTF flags!!!!
			if( other->s.eType == ET_ITEM && other->item_->giType == IT_TEAM )
			{
				Team_DroppedFlagThink( other );
				return;
			}
			G_TempEntity( other->s.origin, EV_ITEM_POP );
			other->freeUp();
			return;
		}

		if ( self_->damage_ ) 
			G_Damage( other, self_, self_, NULL, NULL, self_->damage_
			, 0, MOD_CRUSH, CAT_ANY );

		if ( self_->spawnflags_ & 4 ) 
			return;		// crushers don't reverse

		// reverse direction
		Use_BinaryMoverImpl( self_, self_, other );
	}
};

/*
================
Touch_DoorTrigger
================
*/
struct Touch_DoorTrigger : public GameEntity::EntityFunc_Touch
{
	virtual void execute( GameEntity *other, trace_t *trace )
	{
		if ( self_->parent_->moverState_ != GameEntity::MOVER_1TO2 ) 
			Use_BinaryMoverImpl( self_->parent_, self_, other );
	}
};

/*
======================
Think_SpawnNewDoorTrigger

All of the parts of a door have been spawned, so create
a trigger that encloses all of them
======================
*/
struct Think_SpawnNewDoorTrigger : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		GameEntity		*other;
		vec3_t		mins, maxs;
		int			i, best;

		// set all of the slaves as shootable
		for ( other = self_ ; other ; other = other->teamchain_ ) 
			other->takedamage_ = true;

		// find the bounds of everything on the team
		VectorCopy (self_->r.absmin, mins);
		VectorCopy (self_->r.absmax, maxs);

		for( other = self_->teamchain_ ; other ; other=other->teamchain_ ) 
		{
			AddPointToBounds (other->r.absmin, mins, maxs);
			AddPointToBounds (other->r.absmax, mins, maxs);
		}

		// find the thinnest axis, which will be the one we expand
		best = 0;
		for( i = 1 ; i < 3 ; i++ ) 
		{
			if ( maxs[i] - mins[i] < maxs[best] - mins[best] ) 
				best = i;
		}
		maxs[best] += 120;
		mins[best] -= 120;

		// create a trigger with this size
		other = theLevel.spawnEntity ();
		other->classname_ = "door_trigger";
		VectorCopy (mins, other->r.mins);
		VectorCopy (maxs, other->r.maxs);
		other->parent_ = self_;
		other->r.contents = CONTENTS_TRIGGER;
		other->setTouch(new Touch_DoorTrigger);
		// remember the thinnest axis
		other->count_ = best;
		SV_LinkEntity( other );

		MatchTeam( self_, self_->moverState_, theLevel.time_ );
	}
};

struct Think_MatchTeam : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		MatchTeam( self_, self_->moverState_, theLevel.time_ );
	}
};


/*QUAKED func_door (0 .5 .8) ? START_OPEN x CRUSHER
TOGGLE		wait in both the start and end states for a trigger event.
START_OPEN	the door to moves to its destination when spawned, and operate in reverse.  It is used to temporarily or permanently close off an area when triggered (not useful for touch or takedamage doors).
NOMONSTER	monsters will not trigger this door

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"targetname" if set, no touch field will be spawned and a remote button or trigger field activates the door.
"speed"		movement speed (100 default)
"wait"		wait before returning (3 default, -1 = never return)
"lip"		lip remaining at end of move (8 default)
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
"health"	if set, the door must be shot open
*/
void SP_func_door (GameEntity *ent) 
{
	vec3_t	abs_movedir;
	float	distance;
	vec3_t	size;
	float	lip;

	ent->sound1to2_ = ent->sound2to1_ = G_SoundIndex("sound/movers/doors/dr1_strt.wav");
	ent->soundPos1_ = ent->soundPos2_ = G_SoundIndex("sound/movers/doors/dr1_end.wav");

	ent->setBlocked(new Blocked_DoorBlocked);

	// default speed of 400
	if( !ent->speed_ || ent->speed_ > 20 ) 
		ent->speed_ = 20;

	// default wait of 2 seconds
	if( !ent->wait_ )
		ent->wait_ = 2;
	ent->wait_ *= 1000;

	// default lip of 8 units
	G_SpawnFloat( "lip", "8", &lip );

	// default damage of 2 points
	G_SpawnInt( "dmg", "2", &ent->damage_ );

	// first position at start
	VectorCopy( ent->s.origin, ent->pos1_ );

	// calculate second position
	SV_SetBrushModel( ent, ent->model_ );
	G_SetMovedir( ent->s.angles, ent->movedir_ );
	abs_movedir[0] = fabs(ent->movedir_[0]);
	abs_movedir[1] = fabs(ent->movedir_[1]);
	abs_movedir[2] = fabs(ent->movedir_[2]);
	VectorSubtract( ent->r.maxs, ent->r.mins, size );
	distance = DotProduct( abs_movedir, size ) - lip;
	VectorMA( ent->pos1_, distance, ent->movedir_, ent->pos2_ );

	// if "start_open", reverse position 1 and 2
	if( ent->spawnflags_ & 1 ) 
	{
		vec3_t	temp;

		VectorCopy( ent->pos2_, temp );
		VectorCopy( ent->s.origin, ent->pos2_ );
		VectorCopy( temp, ent->pos1_ );
	}

	InitMover( ent );

	ent->nextthink_ = theLevel.time_ + FRAMETIME;

	if( ! (ent->flags_ & FL_TEAMSLAVE ) ) 
	{
		int health;

		G_SpawnInt( "health", "0", &health );
		if ( health ) 
			ent->takedamage_ = true;
		if ( ent->targetname_ || health ) 
		{
			// non touch/shoot doors
			ent->setThink(new Think_MatchTeam);
		} 
		else 
		{
			ent->setThink(new Think_SpawnNewDoorTrigger);
		}
	}


}

/*
===============================================================================

PLAT

===============================================================================
*/

/*
==============
Touch_Plat

Don't allow decent if a living player is on it
===============
*/
struct Touch_Plat : public GameEntity::EntityFunc_Touch
{
	virtual void execute( GameEntity *other, trace_t *trace ) 
	{
		if( !other->client_ || other->client_->ps_.stats[STAT_HEALTH] <= 0 ) 
			return;

		// delay return-to-pos1 by one second
		if( self_->moverState_ == GameEntity::MOVER_POS2 ) 
			self_->nextthink_ = theLevel.time_ + 1000;
	}
};

/*
==============
Touch_PlatCenterTrigger

If the plat is at the bottom position, start it going up
===============
*/
struct Touch_PlatCenterTrigger : public GameEntity::EntityFunc_Touch
{
	virtual void execute( GameEntity *other, trace_t *trace ) 
	{
		if( !other->client_ ) 
			return;

		if( self_->parent_->moverState_ == GameEntity::MOVER_POS1 ) 
			Use_BinaryMoverImpl( self_->parent_, self_, other );
	}
};


/*
================
SpawnPlatTrigger

Spawn a trigger in the middle of the plat's low position
Elevator cars require that the trigger extend through the entire low position,
not just sit on top of it.
================
*/
void SpawnPlatTrigger( GameEntity *ent ) 
{
	// the middle trigger will be a thin trigger just
	// above the starting position
	GameEntity* trigger = theLevel.spawnEntity();
	trigger->classname_ = "plat_trigger";
	trigger->setTouch(new Touch_PlatCenterTrigger);
	trigger->r.contents = CONTENTS_TRIGGER;
	trigger->parent_ = ent;

	vec3_t tmin, tmax;
	tmin[0] = ent->pos1_[0] + ent->r.mins[0] + 33;
	tmin[1] = ent->pos1_[1] + ent->r.mins[1] + 33;
	tmin[2] = ent->pos1_[2] + ent->r.mins[2];

	tmax[0] = ent->pos1_[0] + ent->r.maxs[0] - 33;
	tmax[1] = ent->pos1_[1] + ent->r.maxs[1] - 33;
	tmax[2] = ent->pos1_[2] + ent->r.maxs[2] + 8;

	if( tmax[0] <= tmin[0] )
	{
		tmin[0] = ent->pos1_[0] + (ent->r.mins[0] + ent->r.maxs[0]) *0.5;
		tmax[0] = tmin[0] + 1;
	}
	if( tmax[1] <= tmin[1] ) 
	{
		tmin[1] = ent->pos1_[1] + (ent->r.mins[1] + ent->r.maxs[1]) *0.5;
		tmax[1] = tmin[1] + 1;
	}
	
	VectorCopy( tmin, trigger->r.mins );
	VectorCopy( tmax, trigger->r.maxs );

	SV_LinkEntity( trigger );
}


/*QUAKED func_plat (0 .5 .8) ?
Plats are always drawn in the extended position so they will light correctly.

"lip"		default 8, protrusion above rest position
"height"	total height of movement, defaults to model height
"speed"		overrides default 200.
"dmg"		overrides default 2
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_plat (GameEntity *ent) 
{
	ent->sound1to2_ = ent->sound2to1_ = G_SoundIndex("sound/movers/plats/pt1_strt.wav");
	ent->soundPos1_ = ent->soundPos2_ = G_SoundIndex("sound/movers/plats/pt1_end.wav");

	VectorClear(ent->s.angles);

	G_SpawnFloat( "speed", "20", &ent->speed_ );
	G_SpawnInt( "dmg", "2", &ent->damage_ );
	G_SpawnFloat( "wait", "1", &ent->wait_ );
	float lip;
	G_SpawnFloat( "lip", "8", &lip );

	ent->wait_ = 1000;

	if( ent->speed_ > 20 ) 
		ent->speed_ = 20;

	// create second position
	SV_SetBrushModel( ent, ent->model_ );

	float height;
	if( !G_SpawnFloat( "height", "0", &height ) ) 
		height = (ent->r.maxs[2] - ent->r.mins[2]) - lip;

	// pos1 is the rest (bottom) position, pos2 is the top
	VectorCopy( ent->s.origin, ent->pos2_ );
	VectorCopy( ent->pos2_, ent->pos1_ );
	ent->pos1_[2] -= height;

	InitMover( ent );

	// touch function keeps the plat from returning while
	// a live player is standing on it
	ent->setTouch(new Touch_Plat);

	ent->setBlocked(new Blocked_DoorBlocked);

	ent->parent_ = ent;	// so it can be treated as a door

	// spawn the trigger if one hasn't been custom made
	if( !ent->targetname_ ) 
		SpawnPlatTrigger(ent);
}


/*
===============================================================================

BUTTON

===============================================================================
*/

/*
==============
Touch_Button

===============
*/
struct Touch_Button : public GameEntity::EntityFunc_Touch
{
	virtual void execute( GameEntity *other, trace_t *trace )
	{
		if( !other->client_ ) 
			return;

		if( self_->moverState_ == GameEntity::MOVER_POS1 ) 
			Use_BinaryMoverImpl( self_, other, other );
	}
};

/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle, triggers all of it's targets, waits some time, then returns to it's original position where it can be triggered again.

"model2"	.md3 model to also draw
"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_button( GameEntity *ent ) 
{
	ent->sound1to2_ = G_SoundIndex("sound/movers/switches/butn2.wav");
	
	if( !ent->speed_ ) 
		ent->speed_ = 40;

	if( !ent->wait_ ) 
		ent->wait_ = 1;
	ent->wait_ *= 1000;

	// first position
	VectorCopy( ent->s.origin, ent->pos1_ );

	// calculate second position
	SV_SetBrushModel( ent, ent->model_ );

	float lip;
	G_SpawnFloat( "lip", "4", &lip );

	G_SetMovedir( ent->s.angles, ent->movedir_ );
	vec3_t abs_movedir;
	abs_movedir[0] = fabs(ent->movedir_[0]);
	abs_movedir[1] = fabs(ent->movedir_[1]);
	abs_movedir[2] = fabs(ent->movedir_[2]);
	vec3_t size;
	VectorSubtract( ent->r.maxs, ent->r.mins, size );
	float distance = abs_movedir[0] * size[0] + abs_movedir[1] * size[1] + abs_movedir[2] * size[2] - lip;
	VectorMA (ent->pos1_, distance, ent->movedir_, ent->pos2_);

	if( ent->health_ ) 
	{
		// shootable button
		ent->takedamage_ = true;
	}
	else 
	{
		// touchable button
		ent->setTouch(new Touch_Button);
	}

	InitMover( ent );
}



/*
===============================================================================

TRAIN

===============================================================================
*/


#define TRAIN_START_ON		1
#define TRAIN_TOGGLE		2
#define TRAIN_BLOCK_STOPS	4

/*
===============
Think_BeginMoving

The wait time at a corner has completed, so start moving again
===============
*/
struct Think_BeginMoving : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		self_->s.pos.trTime = theLevel.time_;
		self_->s.pos.trType = TR_LINEAR_STOP;
	}
};

/*
===============
Reached_Train
===============
*/
void Reached_TrainImpl( GameEntity *ent ) 
{
	GameEntity		*next;
	float			speed;
	vec3_t			move;
	float			length;

	// copy the apropriate values
	next = ent->nextTrain_;
	if( !next || !next->nextTrain_ ) 
		return;		// just stop

	// fire all other targets
	G_UseTargets( next, NULL );

	// set the New trajectory
	ent->nextTrain_ = next->nextTrain_;
	VectorCopy( next->s.origin, ent->pos1_ );
	VectorCopy( next->nextTrain_->s.origin, ent->pos2_ );

	// if the path_corner has a speed, use that
	if ( next->speed_ ) 
		speed = next->speed_;
	else 
	{
		// otherwise use the train's speed
		speed = ent->speed_;
	}
	if( speed < 1 ) 
		speed = 1;

	// calculate duration
	VectorSubtract( ent->pos2_, ent->pos1_, move );
	length = VectorLength( move );

	ent->s.pos.trDuration = length * 1000 / speed;

	// looping sound
	ent->s.loopSound = next->soundLoop_;

	// start it going
	SetMoverState( ent, GameEntity::MOVER_1TO2, theLevel.time_ );

	// if there is a "wait" value on the target, don't start moving yet
	if( next->wait_ )
	{
		ent->nextthink_ = theLevel.time_ + next->wait_ * 1000;
		ent->setThink(new Think_BeginMoving);
		ent->s.pos.trType = TR_STATIONARY;
	}
}


/*
===============
Think_SetupTrainTargets

Link all the corners together
===============
*/
struct Think_SetupTrainTargets : public GameEntity::EntityFunc_Think
{
	virtual void execute() 
	{
		GameEntity		*path, *next;

		self_->nextTrain_ = G_Find( NULL, FOFS(targetname_), self_->target_ );
		if( !self_->nextTrain_ ) 
		{
			Com_Printf( "func_train at %s with an unfound target\n",
				vtos(self_->r.absmin) );
			return;
		}

		GameEntity* start = 0;
		for( path = self_->nextTrain_ ; path != start ; path = next ) 
		{
			if( !start ) 
				start = path;

			if( !path->target_ ) 
			{
				Com_Printf( "Train corner at %s without a target\n",
					vtos(path->s.origin) );
				return;
			}

			// find a path_corner among the targets
			// there may also be other targets that get fired when the corner
			// is reached
			next = 0;
			do {
				next = G_Find( next, FOFS(targetname_), path->target_ );
				if( !next ) 
				{
					Com_Printf( "Train corner at %s without a target path_corner\n",
						vtos(path->s.origin) );
					return;
				}
			} while ( strcmp( next->classname_, "path_corner" ) );

			path->nextTrain_ = next;
		}

		// start the train moving from the first corner
		Reached_TrainImpl( self_ );
	}
};

struct Reached_Train : public GameEntity::EntityFunc_Reached
{
	virtual void execute()
	{
		Reached_TrainImpl( self_ );
	}
};

/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8)
Train path corners.
Target: next path corner and other targets to fire
"speed" speed to move to the next corner
"wait" seconds to wait before behining move to next corner
*/
void SP_path_corner( GameEntity *self ) 
{
	if ( !self->targetname_ ) 
	{
		Com_Printf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
		self->freeUp();
		return;
	}
	// path corners don't need to be linked in
}



/*QUAKED func_train (0 .5 .8) ? START_ON TOGGLE BLOCK_STOPS
A train is a mover that moves between path_corner target points.
Trains MUST HAVE AN ORIGIN BRUSH.
The train spawns at the first target it is pointing at.
"model2"	.md3 model to also draw
"speed"		default 100
"dmg"		default	2
"noise"		looping sound to play when the train is in motion
"target"	next path corner
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_train (GameEntity *self) 
{
	VectorClear( self->s.angles );

	if( self->spawnflags_ & TRAIN_BLOCK_STOPS ) 
	{
		self->damage_ = 0;
	} 
	else 
	{
		if( !self->damage_ ) 
			self->damage_ = 2;
	}

	if( !self->speed_ || self->speed_ > 20 ) 
		self->speed_ = 20;

	if( !self->target_ ) 
	{
		Com_Printf ("func_train without a target at %s\n", vtos(self->r.absmin));
		self->freeUp();
		return;
	}

	SV_SetBrushModel( self, self->model_ );
	InitMover( self );

	self->setReached(new Reached_Train);

	// start trains on the second frame, to make sure their targets have had
	// a chance to spawn
	self->nextthink_ = theLevel.time_ + FRAMETIME;
	self->setThink(new Think_SetupTrainTargets);
}

/*
===============================================================================

STATIC

===============================================================================
*/


/*QUAKED func_static (0 .5 .8) ?
A bmodel that just sits there, doing nothing.  Can be used for conditional walls and models.
"model2"	.md3 model to also draw
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_static( GameEntity *ent )
{
	SV_SetBrushModel( ent, ent->model_ );
	InitMover( ent );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
}



/*
===============================================================================

ROTATING

===============================================================================
*/


/*QUAKED func_rotating (0 .5 .8) ? START_ON - X_AXIS Y_AXIS
You need to have an origin brush as part of this entity.  The center of that brush will be
the point around which it is rotated. It will rotate around the Z axis by default.  You can
check either the X_AXIS or Y_AXIS box to change that.

"model2"	.md3 model to also draw
"speed"		determines how fast it moves; default value is 100.
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_rotating (GameEntity *ent) 
{
	if( !ent->speed_ ) 
		ent->speed_ = 100;

	// set the axis of rotation
	ent->s.apos.trType = TR_LINEAR;
	if ( ent->spawnflags_ & 4 ) 
		ent->s.apos.trDelta[2] = ent->speed_;
	else if ( ent->spawnflags_ & 8 ) 
		ent->s.apos.trDelta[0] = ent->speed_;
	else 
		ent->s.apos.trDelta[1] = ent->speed_;

	if( !ent->damage_ ) 
		ent->damage_ = 2;

	SV_SetBrushModel( ent, ent->model_ );
	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );
	VectorCopy( ent->s.apos.trBase, ent->r.currentAngles );

	SV_LinkEntity( ent );
}


/*
===============================================================================

BOBBING

===============================================================================
*/


/*QUAKED func_bobbing (0 .5 .8) ? X_AXIS Y_AXIS
Normally bobs on the Z axis
"model2"	.md3 model to also draw
"height"	amplitude of bob (32 default)
"speed"		seconds to complete a bob cycle (4 default)
"phase"		the 0.0 to 1.0 offset in the cycle to start at
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
struct Touch_FuncBobbing : public GameEntity::EntityFunc_Touch
{
	virtual void execute( GameEntity *other, trace_t *trace )
	{
		if( !other->client_ ) 
			return;

		G_Damage( other, NULL, other, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );

	}
};

void SP_func_bobbing (GameEntity *ent) 
{
	float		height;
	float		phase;

	G_SpawnFloat( "speed", "4", &ent->speed_ );
	G_SpawnFloat( "height", "32", &height );
	G_SpawnInt( "dmg", "2", &ent->damage_ );
	G_SpawnFloat( "phase", "0", &phase );

	SV_SetBrushModel( ent, ent->model_ );
	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	ent->s.pos.trDuration = ent->speed_ * 1000;
	ent->s.pos.trTime = ent->s.pos.trDuration * phase;
	ent->s.pos.trType = TR_SINE;

	// set the axis of bobbing
	if ( ent->spawnflags_ & 1 ) 
		ent->s.pos.trDelta[0] = height;
	else if ( ent->spawnflags_ & 2 ) 
		ent->s.pos.trDelta[1] = height;
	else 
		ent->s.pos.trDelta[2] = height;

	// MFQ3
	ent->setTouch(new Touch_FuncBobbing);
}

/*
===============================================================================

PENDULUM

===============================================================================
*/


/*QUAKED func_pendulum (0 .5 .8) ?
You need to have an origin brush as part of this entity.
Pendulums always swing north / south on unrotated models.  Add an angles field to the model to allow rotation in other directions.
Pendulum frequency is a physical constant based on the length of the beam and gravity.
"model2"	.md3 model to also draw
"speed"		the number of degrees each way the pendulum swings, (30 default)
"phase"		the 0.0 to 1.0 offset in the cycle to start at
"dmg"		damage to inflict when blocked (2 default)
"color"		constantLight color
"light"		constantLight radius
*/
void SP_func_pendulum(GameEntity *ent) 
{
	float		phase;
	float		speed;

	G_SpawnFloat( "speed", "30", &speed );
	G_SpawnInt( "dmg", "2", &ent->damage_ );
	G_SpawnFloat( "phase", "0", &phase );

	SV_SetBrushModel( ent, ent->model_ );

	// find pendulum length
	float length = fabs( ent->r.mins[2] );
	if ( length < 8 ) 
		length = 8;

	float freq = 1 / ( M_PI * 2 ) * sqrt( g_gravity.value / ( 3 * length ) );

	ent->s.pos.trDuration = ( 1000 / freq );

	InitMover( ent );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	ent->s.apos.trDuration = 1000 / freq;
	ent->s.apos.trTime = ent->s.apos.trDuration * phase;
	ent->s.apos.trType = TR_SINE;
	ent->s.apos.trDelta[2] = speed;
}
