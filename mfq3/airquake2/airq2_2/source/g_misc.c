// g_misc.c

#include "g_local.h"


void DoAQ2Respawn (edict_t *ent)
{
	gi.unlinkentity (ent);

	ent->s.origin[2] += 3;	// ugly hack

	KillBox( ent );

	ent->s.origin[2] -= 3;	// ugly hack

	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			;
	}

//	gi.bprintf( PRINT_HIGH, "%.0f %.0f %.0f\n", ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] );

	if( strcmp( ent->classname, "func_explosive" ) == 0 )
	{
		VectorAdd( ent->mins, ent->s.origin, ent->mins );
		VectorAdd( ent->maxs, ent->s.origin, ent->maxs );
		VectorClear( ent->s.origin );
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = ent->thrust;
	ent->health = ent->max_health;
	ent->takedamage = DAMAGE_YES;
	gi.linkentity (ent);

	// send an effect
//	ent->s.event = EV_ITEM_RESPAWN;
}

void SetAQ2Respawn (edict_t *ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoAQ2Respawn;
	gi.linkentity (ent);
}

/*QUAKED func_group (0 0 0) ?
Used to group brushes together just for editor convenience.
*/

//=====================================================

void Use_Areaportal (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->count ^= 1;		// toggle state
//	gi.dprintf ("portalstate: %i = %i\n", ent->style, ent->count);
	gi.SetAreaPortalState (ent->style, ent->count);
}

/*QUAKED func_areaportal (0 0 0) ?

This is a non-visible object that divides the world into
areas that are seperated when this portal is not activated.
Usually enclosed in the middle of a door.
*/
void SP_func_areaportal (edict_t *ent)
{
	ent->use = Use_Areaportal;
	ent->count = 0;		// always start closed;
}

//=====================================================


/*
=================
Misc functions
=================
*/
void VelocityForDamage (int damage, vec3_t v)
{
	v[0] = 100.0 * crandom();
	v[1] = 100.0 * crandom();
	v[2] = 200.0 + 100.0 * random();

	if (damage < 50)
		VectorScale (v, 0.7, v);
	else 
		VectorScale (v, 1.2, v);
}

void ClipGibVelocity (edict_t *ent)
{
	if (ent->velocity[0] < -300)
		ent->velocity[0] = -300;
	else if (ent->velocity[0] > 300)
		ent->velocity[0] = 300;
	if (ent->velocity[1] < -300)
		ent->velocity[1] = -300;
	else if (ent->velocity[1] > 300)
		ent->velocity[1] = 300;
	if (ent->velocity[2] < 200)
		ent->velocity[2] = 200;	// always some upwards
	else if (ent->velocity[2] > 500)
		ent->velocity[2] = 500;
}


/*
=================
debris
=================
*/
void debris_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void ThrowDebris (edict_t *self, char *modelname, float speed, vec3_t origin)
{
	edict_t	*chunk;
	vec3_t	v;

	chunk = G_Spawn();
	VectorCopy (origin, chunk->s.origin);
	gi.setmodel (chunk, modelname);
	v[0] = 100 * crandom();
	v[1] = 100 * crandom();
	v[2] = 100 + 100 * crandom();
	VectorMA (self->velocity, speed, v, chunk->velocity);
	chunk->movetype = MOVETYPE_BOUNCE;
	chunk->solid = SOLID_NOT;
	chunk->avelocity[0] = random()*600;
	chunk->avelocity[1] = random()*600;
	chunk->avelocity[2] = random()*600;
	chunk->think = G_FreeEdict;
	chunk->nextthink = level.time + 5 + random()*5;
	chunk->s.frame = 0;
	chunk->flags = 0;
	chunk->classname = "debris";
	chunk->takedamage = DAMAGE_YES;
	chunk->die = debris_die;
	gi.linkentity (chunk);
}


void BecomeExplosion1 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}


void BecomeExplosion2 (edict_t *self)
{
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION2);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}


/*QUAKED path_corner (.5 .3 0) (-8 -8 -8) (8 8 8) TELEPORT
Target: next path corner
Pathtarget: gets used when an entity that has
	this path_corner targeted touches it
*/

void path_corner_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		v;
	edict_t		*next;

	if (other->movetarget != self)
		return;
	
	if (other->enemy)
		return;

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		G_UseTargets (self, other);
		self->target = savetarget;
	}

	if (self->target)
		next = G_PickTarget(self->target);
	else
		next = NULL;

	if ((next) && (next->spawnflags & 1))
	{
		VectorCopy (next->s.origin, v);
		v[2] += next->mins[2];
		v[2] -= other->mins[2];
		VectorCopy (v, other->s.origin);
		next = G_PickTarget(next->target);
		other->s.event = EV_OTHER_TELEPORT;
	}

	other->goalentity = other->movetarget = next;

	if (self->wait)
	{
		other->monsterinfo.pausetime = level.time + self->wait;
		other->monsterinfo.stand (other);
		return;
	}

	if (!other->movetarget)
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.stand (other);
	}
	else
	{
		VectorSubtract (other->goalentity->s.origin, other->s.origin, v);
		other->ideal_yaw = vectoyaw (v);
	}
}

void SP_path_corner (edict_t *self)
{
	if (!self->targetname)
	{
		gi.dprintf ("path_corner with no targetname at %s\n", vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	self->solid = SOLID_TRIGGER;
	self->touch = path_corner_touch;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);
	self->svflags |= SVF_NOCLIENT;
	gi.linkentity (self);
}


/*QUAKED point_combat (0.5 0.3 0) (-8 -8 -8) (8 8 8) Hold
Makes this the target of a monster and it will head here
when first activated before going after the activator.  If
hold is selected, it will stay here.
*/
void point_combat_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t	*activator;

	if (other->movetarget != self)
		return;

	if (self->target)
	{
		other->target = self->target;
		other->goalentity = other->movetarget = G_PickTarget(other->target);
		if (!other->goalentity)
		{
			gi.dprintf("%s at %s target %s does not exist\n", self->classname, vtos(self->s.origin), self->target);
			other->movetarget = self;
		}
		self->target = NULL;
	}
	else if ((self->spawnflags & 1) && !(other->flags & (FL_SWIM|FL_FLY)))
	{
		other->monsterinfo.pausetime = level.time + 100000000;
		other->monsterinfo.aiflags |= AI_STAND_GROUND;
		other->monsterinfo.stand (other);
	}

	if (other->movetarget == self)
	{
		other->target = NULL;
		other->movetarget = NULL;
		other->goalentity = other->enemy;
		other->monsterinfo.aiflags &= ~AI_COMBAT_POINT;
	}

	if (self->pathtarget)
	{
		char *savetarget;

		savetarget = self->target;
		self->target = self->pathtarget;
		if (other->enemy && other->enemy->client)
			activator = other->enemy;
		else if (other->oldenemy && other->oldenemy->client)
			activator = other->oldenemy;
		else if (other->activator && other->activator->client)
			activator = other->activator;
		else
			activator = other;
		G_UseTargets (self, activator);
		self->target = savetarget;
	}
}

void SP_point_combat (edict_t *self)
{
	if (deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}
	self->solid = SOLID_TRIGGER;
	self->touch = point_combat_touch;
	VectorSet (self->mins, -8, -8, -16);
	VectorSet (self->maxs, 8, 8, 16);
	self->svflags = SVF_NOCLIENT;
	gi.linkentity (self);
};


/*QUAKED viewthing (0 .5 .8) (-8 -8 -8) (8 8 8)
Just for the debugging level.  Don't use
*/
void TH_viewthing(edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 7;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_viewthing(edict_t *ent)
{
	gi.dprintf ("viewthing spawned\n");

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.renderfx = RF_FRAMELERP;
	VectorSet (ent->mins, -16, -16, -24);
	VectorSet (ent->maxs, 16, 16, 32);
	ent->s.modelindex = gi.modelindex ("models/objects/banner/tris.md2");
	gi.linkentity (ent);
	ent->nextthink = level.time + 0.5;
	ent->think = TH_viewthing;
	return;
}


/*QUAKED info_null (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for spotlights, etc.
*/
void SP_info_null (edict_t *self)
{
	G_FreeEdict (self);
};


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
void SP_info_notnull (edict_t *self)
{
	VectorCopy (self->s.origin, self->absmin);
	VectorCopy (self->s.origin, self->absmax);
};


/*QUAKED light (0 1 0) (-8 -8 -8) (8 8 8) START_OFF
Non-displayed light.
Default light value is 300.
Default style is 0.
If targeted, will toggle between on and off.
Default _cone value is 10 (used to set size of light for spotlights)
*/

#define START_OFF	1

static void light_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->spawnflags & START_OFF)
	{
		gi.configstring (CS_LIGHTS+self->style, "m");
		self->spawnflags &= ~START_OFF;
	}
	else
	{
		gi.configstring (CS_LIGHTS+self->style, "a");
		self->spawnflags |= START_OFF;
	}
}

void SP_light (edict_t *self)
{
	// no targeted lights in deathmatch, because they cause global messages
	if (!self->targetname || deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	if (self->style >= 32)
	{
		self->use = light_use;
		if (self->spawnflags & START_OFF)
			gi.configstring (CS_LIGHTS+self->style, "a");
		else
			gi.configstring (CS_LIGHTS+self->style, "m");
	}
}


/*QUAKED func_wall (0 .5 .8) ? TRIGGER_SPAWN TOGGLE START_ON ANIMATED ANIMATED_FAST
This is just a solid wall if not inhibited

TRIGGER_SPAWN	the wall will not be present until triggered
				it will then blink in to existance; it will
				kill anything that was in it's way

TOGGLE			only valid for TRIGGER_SPAWN walls
				this allows the wall to be turned on and off

START_ON		only valid for TRIGGER_SPAWN walls
				the wall will initially be present
*/

void func_wall_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
	{
		self->solid = SOLID_BSP;
		self->svflags &= ~SVF_NOCLIENT;
		KillBox (self);
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}

void SP_func_wall (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);

	if (self->spawnflags & 8)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 16)
		self->s.effects |= EF_ANIM_ALLFAST;

	// just a wall
	if ((self->spawnflags & 7) == 0)
	{
		self->solid = SOLID_BSP;
		gi.linkentity (self);
		return;
	}

	// it must be TRIGGER_SPAWN
	if (!(self->spawnflags & 1))
	{
//		gi.dprintf("func_wall missing TRIGGER_SPAWN\n");
		self->spawnflags |= 1;
	}

	// yell if the spawnflags are odd
	if (self->spawnflags & 4)
	{
		if (!(self->spawnflags & 2))
		{
			gi.dprintf("func_wall START_ON without TOGGLE\n");
			self->spawnflags |= 2;
		}
	}

	self->use = func_wall_use;
	if (self->spawnflags & 4)
	{
		self->solid = SOLID_BSP;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->svflags |= SVF_NOCLIENT;
	}
	gi.linkentity (self);
}


/*QUAKED func_object (0 .5 .8) ? TRIGGER_SPAWN ANIMATED ANIMATED_FAST
This is solid bmodel that will fall if it's support it removed.
*/

void func_object_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	// only squash thing we fall on top of
	if (!plane)
		return;
	if (plane->normal[2] < 1.0)
		return;
	if (other->takedamage == DAMAGE_NO)
		return;
	T_Damage (other, self, self, vec3_origin, self->s.origin, vec3_origin, self->dmg, 1, 0, MOD_CRUSH);
}

void func_object_release (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->touch = func_object_touch;
}

void func_object_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox (self);
	func_object_release (self);
}

void SP_func_object (edict_t *self)
{
	gi.setmodel (self, self->model);

	self->mins[0] += 1;
	self->mins[1] += 1;
	self->mins[2] += 1;
	self->maxs[0] -= 1;
	self->maxs[1] -= 1;
	self->maxs[2] -= 1;

	if (!self->dmg)
		self->dmg = 100;

	if (self->spawnflags == 0)
	{
		self->solid = SOLID_BSP;
		self->movetype = MOVETYPE_PUSH;
		self->think = func_object_release;
		self->nextthink = level.time + 2 * FRAMETIME;
	}
	else
	{
		self->solid = SOLID_NOT;
		self->movetype = MOVETYPE_PUSH;
		self->use = func_object_use;
		self->svflags |= SVF_NOCLIENT;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	self->clipmask = MASK_MONSTERSOLID;

	gi.linkentity (self);
}


/*QUAKED func_explosive (0 .5 .8) ? Trigger_Spawn ANIMATED ANIMATED_FAST
Any brush that you want to explode or break apart.  If you want an
ex0plosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void func_explosive_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass, delay;

	// new for AirQuake2
	VectorCopy( self->size, self->move_origin );
	VectorCopy( self->absmin, self->move_angles );
//	gi.bprintf( PRINT_HIGH, "min: %.0f %.0f %.0f maxs: %.0f %.0f %.0f\n", self->mins[0], self->mins[1],
//		self->mins[2], self->maxs[0], self->maxs[1], self->maxs[2] );
	self->thrust = self->solid;

	// bmodel origins are (0 0 0), we need to adjust that here
	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	VectorCopy (origin, self->s.origin);

	self->takedamage = DAMAGE_NO;

	if (self->dmg)
		T_RadiusDamage (self, attacker, self->dmg, NULL, self->dmg+40, MOD_EXPLOSIVE);

	VectorSubtract (self->s.origin, inflictor->s.origin, self->velocity);
	VectorNormalize (self->velocity);
	VectorScale (self->velocity, 150, self->velocity);

	// start chunks towards the center
	VectorScale (size, 0.5, size);

	mass = self->mass;
	if (!mass)
		mass = 75;

	// big chunks
	if (mass >= 100)
	{
		count = mass / 100;
		if (count > 8)
			count = 8;
		while(count--)
		{
			chunkorigin[0] = origin[0] + crandom() * size[0];
			chunkorigin[1] = origin[1] + crandom() * size[1];
			chunkorigin[2] = origin[2] + crandom() * size[2];
			ThrowDebris (self, "models/objects/debris1/tris.md2", 1, chunkorigin);
		}
	}

	// small chunks
	count = mass / 25;
	if (count > 8)
		count = 8;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris2/tris.md2", 2, chunkorigin);
	}

	G_UseTargets (self, attacker);

	// new for AirQuake2
	VectorCopy( self->move_origin, self->size );
	VectorCopy( self->move_angles, self->absmin );
	VectorClear( self->velocity );
	VectorSubtract( self->mins, self->s.origin, self->mins );
	VectorSubtract( self->maxs, self->s.origin, self->maxs );

	delay = rand()%60 + 60;

	SetAQ2Respawn( self, delay );

	gi.unlinkentity( self );

/*	if (self->dmg)
		BecomeExplosion1 (self);
	else
		G_FreeEdict (self);*/
}

void func_explosive_use(edict_t *self, edict_t *other, edict_t *activator)
{
	func_explosive_explode (self, self, other, self->health, vec3_origin);
}

void func_explosive_spawn (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_BSP;
	self->svflags &= ~SVF_NOCLIENT;
	self->use = NULL;
	KillBox (self);
	gi.linkentity (self);
}

void SP_func_explosive (edict_t *self)
{
/*	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (self);
		return;
	}*/	// not in NewAirQuake2

	self->movetype = MOVETYPE_NONE;

	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");

	gi.setmodel (self, self->model);

	if (self->spawnflags & 1)
	{
		self->svflags |= SVF_NOCLIENT;
		self->solid = SOLID_NOT;
		self->use = func_explosive_spawn;
	}
	else
	{
		self->solid = SOLID_BSP;
		if (self->targetname)
			self->use = func_explosive_use;
	}

	if (self->spawnflags & 2)
		self->s.effects |= EF_ANIM_ALL;
	if (self->spawnflags & 4)
		self->s.effects |= EF_ANIM_ALLFAST;

	if (self->use != func_explosive_use)
	{
		if (!self->health)
			self->health = 100;
		self->die = func_explosive_explode;
		self->takedamage = DAMAGE_YES;
	}

	self->max_health = self->health;

	gi.linkentity (self);
}


/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/

void barrel_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)

{
	vec3_t	v;

	if ((!other->groundentity) || (other->groundentity == self))
		return;

	VectorSubtract (self->s.origin, other->s.origin, v);
}

void barrel_explode (edict_t *self)
{
	vec3_t	org;
	float	spd;
	vec3_t	save;
	int		delay;

	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_BARREL);

	VectorCopy (self->s.origin, save);
	VectorMA (self->absmin, 0.5, self->size, self->s.origin);

	// a few big chunks
	spd = 1.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);
//	org[0] = self->s.origin[0] + crandom() * self->size[0];
//	org[1] = self->s.origin[1] + crandom() * self->size[1];
//	org[2] = self->s.origin[2] + crandom() * self->size[2];
//	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);

	// bottom corners
	spd = 1.75 * (float)self->dmg / 200.0;
	VectorCopy (self->absmin, org);
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
//	VectorCopy (self->absmin, org);
//	org[0] += self->size[0];
//	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
//	VectorCopy (self->absmin, org);
//	org[1] += self->size[1];
//	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
//	VectorCopy (self->absmin, org);
//	org[0] += self->size[0];
//	org[1] += self->size[1];
//	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);

	// a bunch of little chunks
	spd = 2 * self->dmg / 200;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
//	org[0] = self->s.origin[0] + crandom() * self->size[0];
//	org[1] = self->s.origin[1] + crandom() * self->size[1];
//	org[2] = self->s.origin[2] + crandom() * self->size[2];
//	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
//	org[0] = self->s.origin[0] + crandom() * self->size[0];
//	org[1] = self->s.origin[1] + crandom() * self->size[1];
//	org[2] = self->s.origin[2] + crandom() * self->size[2];
//	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
//	org[0] = self->s.origin[0] + crandom() * self->size[0];
//	org[1] = self->s.origin[1] + crandom() * self->size[1];
//	org[2] = self->s.origin[2] + crandom() * self->size[2];
//	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
//	org[0] = self->s.origin[0] + crandom() * self->size[0];
//	org[1] = self->s.origin[1] + crandom() * self->size[1];
//	org[2] = self->s.origin[2] + crandom() * self->size[2];
//	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
//	org[0] = self->s.origin[0] + crandom() * self->size[0];
//	org[1] = self->s.origin[1] + crandom() * self->size[1];
//	org[2] = self->s.origin[2] + crandom() * self->size[2];
//	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);

	VectorCopy (save, self->s.origin);

	if (self->groundentity)
	{
//		BecomeExplosion2 (self);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION2);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}
	else
	{
//		BecomeExplosion1 (self);
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
	}

	delay = rand()%60 + 60;

	SetAQ2Respawn( self, delay );

	gi.unlinkentity( self );
}

void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->think = barrel_explode;
	self->activator = attacker;
}

void SP_misc_explobox (edict_t *self)
{
/*	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (self);
		return;
	}*/

//	gi.modelindex ("models/objects/debris1/tris.md2");
//	gi.modelindex ("models/objects/debris2/tris.md2");
//	gi.modelindex ("models/objects/debris3/tris.md2");

	self->thrust = self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;

	self->model = "models/objects/oiltank/tris.md2";;
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -18, -18, 0);
	VectorSet (self->maxs, 18, 18, 21);

	if (!self->mass)
		self->mass = 400;
	if (!self->health)
		self->health = 100;
	self->max_health = self->health;
	if (!self->dmg)
		self->dmg = 150;

	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
//	self->monsterinfo.aiflags = AI_NOSTEP;

	self->touch = barrel_touch;

//	self->nextthink = level.time + 2 * FRAMETIME;

	gi.linkentity (self);
}


//
// miscellaneous specialty items
//

/*QUAKED misc_blackhole (1 .5 0) (-8 -8 -8) (8 8 8)
*/

void misc_blackhole_use (edict_t *ent, edict_t *other, edict_t *activator)
{
	/*
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BOSSTPORT);
	gi.WritePosition (ent->s.origin);
	gi.multicast (ent->s.origin, MULTICAST_PVS);
	*/
	G_FreeEdict (ent);
}

void misc_blackhole_think (edict_t *self)
{
	if (++self->s.frame < 19)
		self->nextthink = level.time + FRAMETIME;
	else
	{		
		self->s.frame = 0;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_blackhole (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	VectorSet (ent->mins, -64, -64, 0);
	VectorSet (ent->maxs, 64, 64, 8);
	ent->s.modelindex = gi.modelindex ("models/objects/black/tris.md2");
	ent->s.renderfx = RF_TRANSLUCENT;
	ent->use = misc_blackhole_use;
	ent->think = misc_blackhole_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED misc_banner (1 .5 0) (-4 -4 -4) (4 4 4)
The origin is the bottom of the banner.
The banner is 128 tall.
*/
void misc_banner_think (edict_t *ent)
{
	ent->s.frame = (ent->s.frame + 1) % 16;
	ent->nextthink = level.time + FRAMETIME;
}

void SP_misc_banner (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/objects/banner/tris.md2");
	ent->s.frame = rand() % 16;
	gi.linkentity (ent);

	ent->think = misc_banner_think;
	ent->nextthink = level.time + FRAMETIME;
}


//=====================================================

/*QUAKED target_character (0 0 1) ?
used with target_string (must be on same "team")
"count" is position in the string (starts at 1)
*/

void SP_target_character (edict_t *self)
{
	self->movetype = MOVETYPE_PUSH;
	gi.setmodel (self, self->model);
	self->solid = SOLID_BSP;
	self->s.frame = 12;
	gi.linkentity (self);
	return;
}


/*QUAKED target_string (0 0 1) (-8 -8 -8) (8 8 8)
*/

void target_string_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t *e;
	int		n, l;
	char	c;

	l = strlen(self->message);
	for (e = self->teammaster; e; e = e->teamchain)
	{
		if (!e->count)
			continue;
		n = e->count - 1;
		if (n > l)
		{
			e->s.frame = 12;
			continue;
		}

		c = self->message[n];
		if (c >= '0' && c <= '9')
			e->s.frame = c - '0';
		else if (c == '-')
			e->s.frame = 10;
		else if (c == ':')
			e->s.frame = 11;
		else
			e->s.frame = 12;
	}
}

void SP_target_string (edict_t *self)
{
	if (!self->message)
		self->message = "";
	self->use = target_string_use;
}


/*QUAKED func_clock (0 0 1) (-8 -8 -8) (8 8 8) TIMER_UP TIMER_DOWN START_OFF MULTI_USE
target a target_string with this

The default is to be a time of day clock

TIMER_UP and TIMER_DOWN run for "count" seconds and the fire "pathtarget"
If START_OFF, this entity must be used before it starts

"style"		0 "xx"
			1 "xx:xx"
			2 "xx:xx:xx"
*/

#define	CLOCK_MESSAGE_SIZE	16

// don't let field width of any clock messages change, or it
// could cause an overwrite after a game load

static void func_clock_reset (edict_t *self)
{
	self->activator = NULL;
	if (self->spawnflags & 1)
	{
		self->health = 0;
		self->wait = self->count;
	}
	else if (self->spawnflags & 2)
	{
		self->health = self->count;
		self->wait = 0;
	}
}

static void func_clock_format_countdown (edict_t *self)
{
	if (self->style == 0)
	{
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i", self->health);
		return;
	}

	if (self->style == 1)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i", self->health / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		return;
	}

	if (self->style == 2)
	{
		Com_sprintf(self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", self->health / 3600, (self->health - (self->health / 3600) * 3600) / 60, self->health % 60);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
		return;
	}
}

void func_clock_think (edict_t *self)
{
	if (!self->enemy)
	{
		self->enemy = G_Find (NULL, FOFS(targetname), self->target);
		if (!self->enemy)
			return;
	}

	if (self->spawnflags & 1)
	{
		func_clock_format_countdown (self);
		self->health++;
	}
	else if (self->spawnflags & 2)
	{
		func_clock_format_countdown (self);
		self->health--;
	}
	else
	{
		struct tm	*ltime;
		time_t		gmtime;

		time(&gmtime);
		ltime = localtime(&gmtime);
		Com_sprintf (self->message, CLOCK_MESSAGE_SIZE, "%2i:%2i:%2i", ltime->tm_hour, ltime->tm_min, ltime->tm_sec);
		if (self->message[3] == ' ')
			self->message[3] = '0';
		if (self->message[6] == ' ')
			self->message[6] = '0';
	}

	self->enemy->message = self->message;
	self->enemy->use (self->enemy, self, self);

	if (((self->spawnflags & 1) && (self->health > self->wait)) ||
		((self->spawnflags & 2) && (self->health < self->wait)))
	{
		if (self->pathtarget)
		{
			char *savetarget;
			char *savemessage;

			savetarget = self->target;
			savemessage = self->message;
			self->target = self->pathtarget;
			self->message = NULL;
			G_UseTargets (self, self->activator);
			self->target = savetarget;
			self->message = savemessage;
		}

		if (!(self->spawnflags & 8))
			return;

		func_clock_reset (self);

		if (self->spawnflags & 4)
			return;
	}

	self->nextthink = level.time + 1;
}

void func_clock_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (!(self->spawnflags & 8))
		self->use = NULL;
	if (self->activator)
		return;
	self->activator = activator;
	self->think (self);
}

void SP_func_clock (edict_t *self)
{
	if (!self->target)
	{
		gi.dprintf("%s with no target at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 2) && (!self->count))
	{
		gi.dprintf("%s with no count at %s\n", self->classname, vtos(self->s.origin));
		G_FreeEdict (self);
		return;
	}

	if ((self->spawnflags & 1) && (!self->count))
		self->count = 60*60;;

	func_clock_reset (self);

	self->message = gi.TagMalloc (CLOCK_MESSAGE_SIZE, TAG_LEVEL);

	self->think = func_clock_think;

	if (self->spawnflags & 4)
		self->use = func_clock_use;
	else
		self->nextthink = level.time + 1;
}



/*
==============================================================================

object_tree
(AQ2 only)
==============================================================================
*/

/*QUAKED object_tree (1 1 1) (-5 -5 0) (5 5 30)
Spawns a tree
*/
/*QUAKED object_tree_multiple (1 1 1) (-20 -20 0) (20 20 30)
Spawns a tree
*/

void treetofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
//		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
//		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	gi.linkentity (ent);
}



void tree_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{

}

void tree_pain (edict_t *self, edict_t *other, float kick, int damage)
{

}


void tree_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t up;

	AngleVectors( self->s.angles, NULL, NULL, up );

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LASER_SPARKS);
	gi.WriteByte ( 50 );
	gi.WritePosition (self->s.origin);
	gi.WriteDir ( up );
	gi.WriteByte ( 2 );
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->deadflag = DEAD_DEAD;
	G_FreeEdict( self );
}

void SP_object_tree (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_AIM;
	VectorSet (self->mins, -2, -2, 0);
	VectorSet (self->maxs, 2, 2, 30);
	self->s.angles[1] = random()*360;
	self->s.modelindex = gi.modelindex ("models/objects/trees/palm/single.md2");

	if( !self->mass )
		self->mass = (int)(random()*3+1);

	switch( self->mass )
	{
		case 1:
				self->s.frame = 0;
				self->maxs[2] = 40;
				break;
		case 2:
				self->s.frame = 1;
				self->maxs[2] = 24;
				break;
		case 3:
		default:
				self->s.frame = 2;
				self->maxs[2] = 28;
				break;
	}
	if( !self->health )
		self->health = 50;
	self->touch = tree_touch;
	self->pain = tree_pain;
	self->die = tree_die;
	self->think = treetofloor;
	self->nextthink =level.time + 0.2;

	gi.linkentity (self);
}

void SP_object_tree_multiple (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_AIM;
	VectorSet (self->mins, -18, -18, 0);
	VectorSet (self->maxs, 18, 18, 34);
	self->s.angles[1] = random()*360;
	self->s.modelindex = gi.modelindex ("models/objects/trees/palm/multiple.md2");

	if( !self->mass )
		self->mass = (int)(random()*3+1);

	switch( self->mass )
	{
		case 1:
				self->s.frame = 0;
				break;
		case 2:
				self->s.frame = 1;
				break;
		case 3:
		default:
				self->s.frame = 2;
				break;
	}
	if( !self->health )
		self->health = 80;
	self->touch = tree_touch;
	self->pain = tree_pain;
	self->die = tree_die;
	self->think = treetofloor;
	self->nextthink =level.time + 0.2;

	gi.linkentity (self);
}








/*
==============================================================================

object_tent
(AQ2 only)
==============================================================================
*/

/*QUAKED object_tent (1 1 1) (-17 -11 0) (17 11 15)
Spawns a tent
*/


void tent_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{

}

void tent_pain (edict_t *self, edict_t *other, float kick, int damage)
{

}

void tent_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t up;

/*	if( attacker->owner && teamplay->value == 1 )
	{
		if( attacker->owner->client )
		{
			gi.bprintf( PRINT_MEDIUM, "[%s]%s destroyed %s's %s.\n",
				team[attacker->owner->client->pers.team-1].name,
				attacker->owner->client->pers.netname,
				(self->aqteam > 0 ? team[self->aqteam-1].name : "neutral"),
				( self->message != NULL ? self->message : "tent" ) );
		}
	}
	if( self->count > 0 && teamplay->value == 1 )
	{
		if( attacker->aqteam != self->aqteam )
		{
			attacker->owner->client->resp.score += self->count;
			team[attacker->aqteam-1].score += self->count;
		}
		else
		{
			attacker->owner->client->resp.score -= self->count;
			team[attacker->aqteam-1].score -= self->count;
		}
	}
*/
	AngleVectors( self->s.angles, NULL, NULL, up );

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_LASER_SPARKS);
	gi.WriteByte ( 50 );
	gi.WritePosition (self->s.origin);
	gi.WriteDir ( up );
	gi.WriteByte ( 2 );
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->deadflag = DEAD_DEAD;
	G_FreeEdict( self );
}




void SP_object_tent (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_AIM;
	self->s.modelindex = gi.modelindex ("models/objects/tent/tent_small.md2");
	VectorSet( self->mins, -17, -11, 0 );
	VectorSet( self->maxs, 17, 11, 15 );

	if( !self->sounds )
		self->sounds = (int)(random()*3+1);

	switch( self->sounds )
	{
		case 1:
				self->s.skinnum = 0;
				break;
		case 2:
				self->s.skinnum = 1;
				break;
		case 3:
		default:
				self->s.skinnum = 2;
				break;
	}
	if( !self->health )
		self->health = 50;
	if( self->aqteam < 0 || self->aqteam > 2 )
		self->aqteam = 0;
	self->touch = tent_touch;
	self->pain = tent_pain;
	self->die = tent_die;
	self->think = treetofloor;
	self->nextthink =level.time + 0.2;

	gi.linkentity (self);
}




/*
==============================================================================

misc_vehicle
(AQ2 only)
==============================================================================
*/

/*QUAKED misc_vehicle (1 1 1) (-16 -16 -8) (16 16 16)
Spawns an empty vehicle
sounds spawns a specific vehicle,
adding no sounds key or only a
partial sounds key generates a
random vehicle.
Examples: 	sounds - 121
			sounds - 41
			sounds - 2
*/

void misc_vehicle_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->s.effects = EF_ROCKET;
}



void misc_vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->deadflag = DEAD_DEAD;
	SetVehicleModel( self );
	self->think = G_FreeEdict;
	self->nextthink = level.time + 3;

}



void misc_vehicle_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( self->deadflag == DEAD_DEAD || strcmp( other->classname, "freed" ) == 0
		|| strcmp( other->classname, "debris" ) == 0 )
		return;
	if( strcmp( other->classname, "rocket" ) == 0 ||
		strcmp( other->classname, "AGM" ) == 0 ||
   		strcmp( other->classname, "sidewinder" ) == 0 ||
		strcmp( other->classname, "bolt" ) == 0 ||
		strcmp( other->classname, "shell" ) == 0 ||
		strcmp( other->classname, "bomb" ) == 0 )
		return;
	if( self->ONOFF & ONOFF_SINKING )
	{
		self->ONOFF &= ~ONOFF_SINKING;
		self->think = EmptyPlaneDie;
		self->nextthink = level.time + 10;
		return;
	}
	else if( strcmp( other->classname, "func_runway") == 0
   		 && ( !(self->ONOFF & ONOFF_AIRBORNE) ) )
		return;
	if( strcmp( other->classname, "camera" ) == 0 )
		return;
	if( surf && (surf->flags & SURF_SKY) )
	{
		if( self->health > 0 )
			return;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		VehicleExplosion( self );
		SetVehicleModel( self );
		self->deadflag = DEAD_DEAD;
		self->think = G_FreeEdict;
		self->nextthink = level.time + 3;
	}
	else
	{
		if( !(self->ONOFF & ONOFF_AIRBORNE) )
		{
			if( VectorLength(self->velocity) >= 5 )
			{
				self->health -= (VectorLength(self->velocity)/2);
				MakeDamage( self );
				gi.sound( self, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
			}
			self->speed = 0;
			VectorClear( self->avelocity );
			VectorClear( self->velocity );
		}
		else
		{
			self->health = 0;
		}
		if( self->health <= 0 )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			VehicleExplosion( self );
			SetVehicleModel( self );
			self->deadflag = DEAD_DEAD;
			gi.bprintf( PRINT_MEDIUM, "An empty vehicle crashed.\n" );
			self->think = G_FreeEdict;
			self->nextthink = level.time + 3;
//			gi.bprintf( PRINT_HIGH, "%s\n", other->classname );
		}
	}
}

void misc_vehicle_think( edict_t *vehicle )
{
#pragma message( "add helos" )

	vehicle->s.effects = 0;

	if( strcmp(vehicle->classname,"plane" ) == 0 )
	{
		vec3_t	targetangle, forward, above;
		trace_t trace;

		// check for overturning
		vehicle->s.angles[0] = anglemod( vehicle->s.angles[0] );
		vehicle->s.angles[1] = anglemod( vehicle->s.angles[1] );
		if( vehicle->s.angles[2] > 180 )
			vehicle->s.angles[2] -= 360;
		else if( vehicle->s.angles[2] < -180 )
			vehicle->s.angles[2] += 360;

		if( vehicle->deadflag == DEAD_DEAD )
		{
			return;
		}
		else if( vehicle->deadflag == DEAD_DYING )
		{
			vehicle->avelocity[2] = (vehicle->avelocity[2]>=0) ? 60 : -60;
			vectoangles( vehicle->velocity, targetangle );
			vehicle->s.angles[0] = targetangle[0];
			vehicle->s.angles[1] = targetangle[1];
			vehicle->gravity = 0.1;
			vehicle->nextthink = level.time + 0.1;
			return;
		}
		else if( vehicle->ONOFF & ONOFF_SINKING )
		{
			vehicle->gravity = 0.01;
			vehicle->nextthink = level.time + 0.1;
			return;
		}

		if( !(vehicle->ONOFF & ONOFF_AIRBORNE) )	// moving on ground
		{
			vehicle->s.effects = 0;
			VectorClear( vehicle->avelocity );
			VectorSet( above, vehicle->s.origin[0], vehicle->s.origin[1],
							vehicle->s.origin[2] - 32 );

			trace = gi.trace( vehicle->s.origin, NULL, NULL, above, vehicle, MASK_ALL );

			// are you standing on a runway ?
			if( strcmp( trace.ent->classname, "func_runway" ) == 0
			|| strcmp( trace.ent->classname, "func_door" ) == 0
			|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0 )
			{
				if( vehicle->ONOFF & ONOFF_LANDED_TERRAIN )
				{
					vehicle->ONOFF |= ONOFF_LANDED;
					vehicle->ONOFF &= ~ONOFF_LANDED_TERRAIN;
				}

				vehicle->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2]
			   			 + abs(vehicle->mins[2])+2;

				// crashlanded planes
				if( (vehicle->ONOFF & ONOFF_LANDED_NOGEAR) && vehicle->speed < 2 )
				{
					if( vehicle->s.angles[2] < 15 && (vehicle->HASCAN & HASCAN_ROUNDBODY))
						vehicle->avelocity[2] = 10;
					else
					{
						VectorClear( vehicle->velocity );
						vehicle->avelocity[2] = 0;
						vehicle->think = EmptyPlaneDie;
						vehicle->nextthink = level.time + 10;
						return;
					}
				}
			}
			// moving on terrain
			else if( trace.contents & CONTENTS_SOLID )
			{
				if( vehicle->ONOFF & ONOFF_LANDED )
				{
					vehicle->ONOFF |= ONOFF_LANDED_TERRAIN;
					vehicle->ONOFF &= ~ONOFF_LANDED;
				}

				vehicle->s.origin[2] = trace.endpos[2] + abs(vehicle->mins[2])+2;

				// crashlanded planes
				if( (vehicle->ONOFF & ONOFF_LANDED_TERRAIN_NOGEAR) && vehicle->speed < 2 )
				{
					if( vehicle->s.angles[2] < 15 && (vehicle->HASCAN & HASCAN_ROUNDBODY) )
						vehicle->avelocity[2] = 10;
					else
					{
						VectorClear( vehicle->velocity );
						vehicle->avelocity[2] = 0;
						vehicle->think = EmptyPlaneDie;
						vehicle->nextthink = level.time + 10;
						return;
					}
				}
			}
			// moving on water
			else if( trace.contents & CONTENTS_LIQUID )
			{
				if( vehicle->ONOFF & ONOFF_LANDED )
				{
					vehicle->ONOFF |= ONOFF_LANDED_WATER;
					vehicle->ONOFF &= ~ONOFF_LANDED;
				}
				if( vehicle->speed > 5 )
				{
					vehicle->s.origin[2] = trace.endpos[2] + abs(vehicle->mins[2])+2;
				}
				else	// sinking
				{
					vehicle->ONOFF |= ONOFF_SINKING;
					vehicle->DAMAGE |= DAMAGE_ENGINE_DESTROYED | DAMAGE_ENGINE_20 | DAMAGE_ENGINE_40 |
							DAMAGE_ENGINE_60 | DAMAGE_ENGINE_80 | DAMAGE_AFTERBURNER;
					vehicle->thrust = THRUST_IDLE;
				}
			}
			// if not then you stall
			else
			{
				vehicle->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN | ONOFF_LANDED_WATER_NOGEAR
							| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR | ONOFF_LANDED_WATER);
				vehicle->ONOFF |= ONOFF_AIRBORNE;
				if( vehicle->speed < vehicle->stallspeed )
				{
					vehicle->ONOFF |= ONOFF_STALLED;
				}
			}

			if( vehicle->speed > 0 )
			{
				PlaneSpeed( vehicle );
				AngleVectors( vehicle->s.angles, forward, NULL, NULL );
				VectorScale( forward, vehicle->speed, vehicle->velocity );
			}
		}
		else	// airborne
		{
			vehicle->health = 0;
			vehicle->deadflag = DEAD_DYING;
		}
		if( vehicle->health <= vehicle->gib_health && vehicle->deadflag != DEAD_DEAD )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (vehicle->s.origin);
			gi.multicast (vehicle->s.origin, MULTICAST_PVS);
			SetVehicleModel(vehicle);
			vehicle->deadflag = DEAD_DEAD;
		}
		else if( vehicle->health <= 0 && !vehicle->deadflag )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION2);
			gi.WritePosition (vehicle->s.origin);
			gi.multicast (vehicle->s.origin, MULTICAST_PVS);
			if( vehicle->ONOFF & ONOFF_AIRBORNE )
			{
				vehicle->deadflag = DEAD_DYING;
			}
			else
			{
				SetVehicleModel(vehicle);
				vehicle->deadflag = DEAD_DEAD;
			}
		}
	}

	vehicle->nextthink = level.time + 0.1;
}


void SP_misc_vehicle (edict_t *self)
{
	int vehclass;

	if( self->sounds < 100 ) // put something random in
	{
		if( self->sounds < 1 ) // nothing selected -> select category
		{
			vehclass = (int)(random() * 3 + 1);
			if( vehclass == 3 )
				vehclass = 4;
			vehclass *= 100;
		}
		else if( self->sounds > 0 && self->sounds < 11 ) // category selected -> make it valid
		{
			vehclass = self->sounds * 100;
		}
		if( self->sounds > 10 ) // class and category selected -> make them valid
		{
			vehclass = self->sounds * 10;
		}
		else	// category selected -> select class
		{
			switch( vehclass )
			{
				case 100:
							vehclass += ((int)(random()*2+1)*10);
							break;
				case 200:
							vehclass += ((int)(random()*2+1)*10);
							break;
				case 400:
							vehclass += ((int)(random()*4+1)*10);
							if( vehclass == 430 )
								vehclass = 440;
							break;
			}
		}
		switch( vehclass )
		{
			case 110:
						vehclass += (int)(random()*8+1);
						break;
			case 120:
						vehclass += (int)(random()*5+1);
						break;
			case 210:
						vehclass += (int)(random()*2+1);
						break;
			case 220:
						vehclass += (int)(random()*2+1);
						break;
			case 410:
						vehclass += (int)(random()*2+1);
						break;
			case 420:
						vehclass += (int)(random()*2+1);
						break;
			case 440:
						vehclass += (int)(random()*4+1);
		}
	}
	else
	{
		vehclass = self->sounds;
	}

	// Spawn the specific vehicle
	switch( vehclass )
	{
		case CODE_F14:	// F14
					Spawn_F14( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case CODE_F22:	// F22
					Spawn_F22( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] -= 1;
					self->s.frame = 4;
					break;
		case CODE_F16:	// F16
					Spawn_F16( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case CODE_F4:	// F4
					Spawn_F4( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case CODE_SU37:	// SU37
					Spawn_SU37( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case CODE_MIG21:	// MiG21
					Spawn_MIG21( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case CODE_MIG29:	// MiG29
					Spawn_MIG29( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case CODE_MIGMFI:	// MiG MFI
					Spawn_MIGMFI( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case CODE_TORNADO:	// Tornado
					Spawn_Tornado( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case CODE_A10:	// A10
					Spawn_A10( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case CODE_F117:	// F117
					Spawn_F117( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case CODE_SU24:	// Su24
					Spawn_SU24( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case CODE_SU25:	// Su25
					Spawn_SU25( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case CODE_SU34:	// Su34
					Spawn_SU34( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case CODE_AH64:	// AH64
					Spawn_AH64( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] -= 3;
					self->s.frame = 0;
					break;
		case CODE_MI28:	// MI28
					Spawn_MI28( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 5;
					self->s.frame = 0;
					break;
		case CODE_RAH66:	// RAH66
					Spawn_RAH66( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] -= 1;
					self->s.frame = 0;
					break;
		case CODE_KA50:	// Ka50
					Spawn_KA50( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.frame = 0;
					break;
/*		case 411:	// M1A2
					self->s.origin[2] -= 2;
					Start_M1( self );
					self->s.frame = 0;
					break;
		case 412:	// T72
					self->s.origin[2] -= 3;
					Start_T72( self );
					self->s.frame = 0;
					break;
		case 421:	// Scorpion
					self->s.origin[2] -= 3;
					Start_Scorpion( self );
					self->s.frame = 0;
					break;
		case 422:	// BRDM2
					self->s.origin[2] -= 3;
					Start_BRDM2( self );
					self->s.frame = 0;
					break;
		case 441:	// Lav25
					self->s.origin[2] -= 3;
					Start_LAV25( self );
					self->s.frame = 0;
					break;
		case 442:	// Rapier
					self->s.origin[2] -= 3;
					Start_Rapier( self );
					self->s.frame = 0;
					break;
		case 443:	// 2S6 Tunguska
					self->s.origin[2] -= 3;
					Start_2S6( self );
					self->s.frame = 0;
					break;
		case 444:	// Sa9
					self->s.origin[2] -= 3;
					Start_SA9( self );
					self->s.frame = 0;
					break;*/
		default:	// default gives you an F16
					gi.bprintf( PRINT_HIGH, "Non valid vehicle! Using default!\n" );
					Spawn_F16( self );
					self->ONOFF |= (ONOFF_LANDED | ONOFF_GEAR);
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
	}
	if( self->HASCAN & HASCAN_TAILDRAGGER )
		self->s.angles[0] = -5;

	// override defaults
	self->touch = misc_vehicle_touch;
	self->pain = misc_vehicle_pain;
	self->die = misc_vehicle_die;
	self->think = misc_vehicle_think;
	self->nextthink = level.time + 0.1;

	gi.linkentity (self);
}





