// Handles flare firing and other stuff (gps, autopilot)
// by Bjoern Drabeck

#include "g_local.h"
#include "v_flare.h"

void Flare_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward;
	vec3_t	start;

	AngleVectors ( ent->vehicle->s.angles, forward, NULL, NULL);
	VectorCopy( ent->vehicle->s.origin, start );
	start[2] += 15;

	fire_flare (ent->vehicle, start, forward,
			damage, 180, effect, hyper );

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);
	gi.WriteByte (MZ_SILENCED);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);

}



void flare_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BLASTER);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}


void fire_flare (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;

	VectorNormalize (dir);

	bolt = G_Spawn();
	if( strcmp( self->classname, "ground" ) == 0 )
	{
		speed *= 1.7;
		start[2] += 10;
	}
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorSet( dir, 0, 0, 1 );
	VectorScale (dir, speed, bolt->velocity);
	bolt->gravity = 0.3;
	bolt->movetype = MOVETYPE_TOSS;
	bolt->avelocity[0] = 200;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->s.sound = gi.soundindex ("misc/lasfly.wav");
	bolt->owner = self;
	bolt->touch = flare_touch;
	if( strcmp( self->classname, "ground" ) == 0 )
		bolt->nextthink = level.time + 3.0;
	else
		bolt->nextthink = level.time + 2.5;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";

	gi.linkentity (bolt);

}


void LightThink( edict_t *light )
{
	vec3_t forward;
	trace_t tr;

	AngleVectors( light->owner->s.angles, forward, NULL, NULL );
	VectorScale (forward, 300, forward);
	forward[2] -= 20;

	tr = gi.trace( light->owner->s.origin, NULL, NULL, forward, light->owner, MASK_ALL );

	if( tr.fraction < 1.0 )
		VectorCopy( tr.endpos, light->s.origin );
	else
	{
		VectorAdd( light->owner->s.origin, forward, light->s.origin );
	}

	if( !(light->owner->ONOFF & LIGHT) )
		G_FreeEdict( light );

	light->nextthink = level.time + 0.1;
}

void SpawnLight( edict_t *ent )
{

	edict_t	*light;
	trace_t	tr;
	vec3_t	forward;

	light = G_Spawn();

	AngleVectors( ent->vehicle->s.angles, forward, NULL, NULL );
	VectorScale (forward, 300, forward);
	forward[2] -= 20;

	tr = gi.trace( ent->vehicle->s.origin, NULL, NULL, forward, ent->vehicle, MASK_ALL );

	if( tr.fraction < 1.0 )
		VectorCopy( tr.endpos, light->s.origin );
	else
	{
		VectorAdd( ent->vehicle->s.origin, forward, light->s.origin );
	}

	light->s.modelindex = gi.modelindex ("models/weapons/shell/shell.md2");
	light->movetype = MOVETYPE_NONE;
	light->solid = SOLID_BBOX;

	VectorSet( light->s.angles, 0,0,0 );

	light->s.effects |= EF_HYPERBLASTER;

	VectorClear (light->mins);
	VectorClear (light->maxs);
	light->owner = ent->vehicle;

	light->nextthink = level.time + 0.1;
	light->think = LightThink;

	light->classname = "light";

	ent->vehicle->target_ent = light;

	gi.linkentity (light);

}


