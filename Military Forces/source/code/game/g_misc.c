/*
 * $Id: g_misc.c,v 1.5 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_misc.c

#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.  They are turned into normal brushes by the utilities.
*/


/*QUAKED info_camp (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_camp( GameEntity *self )
{
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for calculations in the utilities (spotlights, etc), but removed during gameplay.
*/
void SP_info_null( GameEntity *self ) 
{
	self->freeUp();
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for in-game calculation, like jumppad targets.
target_position does the same thing
*/
void SP_info_notnull( GameEntity *self )
{
	G_SetOrigin( self, self->s.origin );
}


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) linear
Non-displayed light.
"light" overrides the default 300 intensity.
Linear checbox gives linear falloff instead of inverse square
Lights pointed at a target will be spotlights.
"radius" overrides the default 64 unit radius of a spotlight at the target point.
*/
void SP_light( GameEntity *self ) 
{
	self->freeUp();
}



//===========================================================

/*QUAKED misc_model (1 0 0) (-16 -16 -16) (16 16 16)
"model"		arbitrary .md3 file to display
*/
void SP_misc_model( GameEntity *ent ) 
{
	ent->freeUp();
}

//===========================================================

struct Think_LocateCamera : public GameEntity::EntityFunc_Think
{
	virtual void execute() 
	{
		vec3_t		dir;
		GameEntity*	target;
		GameEntity* owner;

		owner = G_PickTarget( self_->target_ );
		if ( !owner ) 
		{
			Com_Printf( "Couldn't find target for misc_partal_surface\n" );
			self_->freeUp();
			return;
		}
		self_->r.ownerNum = owner->s.number;

		// frame holds the rotate speed
		if ( owner->spawnflags_ & 1 ) 
			self_->s.frame = 25;
		else if ( owner->spawnflags_ & 2 ) 
			self_->s.frame = 75;

		// set to 0 for no rotation at all
		self_->s.objectives = 1;

		// clientNum holds the rotate offset
		self_->s.clientNum = owner->s.clientNum;

		VectorCopy( owner->s.origin, self_->s.origin2 );

		// see if the portal_camera has a target
		target = G_PickTarget( owner->target_ );
		if ( target ) 
		{
			VectorSubtract( target->s.origin, owner->s.origin, dir );
			VectorNormalize( dir );
		} 
		else 
		{
			G_SetMovedir( owner->s.angles, dir );
		}

		self_->s.eventParm = DirToByte( dir );
	}
};

/*QUAKED misc_portal_surface (0 0 1) (-8 -8 -8) (8 8 8)
The portal surface nearest this entity will show a view from the targeted misc_portal_camera, or a mirror view if untargeted.
This must be within 64 world units of the surface!
*/
void SP_misc_portal_surface( GameEntity *ent ) 
{
	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	SV_LinkEntity( ent );

	ent->r.svFlags = SVF_PORTAL;
	ent->s.eType = ET_PORTAL;

	if ( !ent->target_ ) 
	{
		VectorCopy( ent->s.origin, ent->s.origin2 );
	} 
	else 
	{
		ent->setThink(new Think_LocateCamera);
		ent->nextthink_ = theLevel.time_ + 100;
	}
}

/*QUAKED misc_portal_camera (0 0 1) (-8 -8 -8) (8 8 8) slowrotate fastrotate
The target for a misc_portal_director.  You can set either angles or target another entity to determine the direction of view.
"roll" an angle modifier to orient the camera around the target vector;
*/
void SP_misc_portal_camera( GameEntity* ent ) 
{
	float	roll;

	VectorClear( ent->r.mins );
	VectorClear( ent->r.maxs );
	SV_LinkEntity( ent );

	G_SpawnFloat( "roll", "0", &roll );

	ent->s.clientNum = roll/360.0 * 256;
}



