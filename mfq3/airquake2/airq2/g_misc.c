// g_misc.c

#include "g_local.h"


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
gibs
=================
*/
void gib_think (edict_t *self)
{
	self->s.frame++;
	self->nextthink = level.time + FRAMETIME;

	if (self->s.frame == 10)
	{
		self->think = G_FreeEdict;
		self->nextthink = level.time + 8 + random()*10;
	}
}

void gib_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t	normal_angles, right;

	if (!self->groundentity)
		return;

	self->touch = NULL;

	if (plane)
	{
		gi.sound (self, CHAN_VOICE, gi.soundindex ("misc/fhit3.wav"), 1, ATTN_NORM, 0);

		vectoangles (plane->normal, normal_angles);
		AngleVectors (normal_angles, NULL, right, NULL);
		vectoangles (right, self->s.angles);

		if (self->s.modelindex == sm_meat_index)
		{
			self->s.frame++;
			self->think = gib_think;
			self->nextthink = level.time + FRAMETIME;
		}
	}
}

void gib_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_FreeEdict (self);
}

void ThrowGib (edict_t *self, char *gibname, int damage, int type)
{
	edict_t *gib;
	vec3_t	vd;
	vec3_t	origin;
	vec3_t	size;
	float	vscale;

	gib = G_Spawn();

	VectorScale (self->size, 0.5, size);
	VectorAdd (self->absmin, size, origin);
	gib->s.origin[0] = origin[0] + crandom() * size[0];
	gib->s.origin[1] = origin[1] + crandom() * size[1];
	gib->s.origin[2] = origin[2] + crandom() * size[2];

	gi.setmodel (gib, gibname);
	gib->solid = SOLID_NOT;
	gib->s.effects |= EF_GIB;
	gib->flags |= FL_NO_KNOCKBACK;
	gib->takedamage = DAMAGE_YES;
	gib->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		gib->movetype = MOVETYPE_TOSS;
		gib->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		gib->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, gib->velocity);
	ClipGibVelocity (gib);
	gib->avelocity[0] = random()*600;
	gib->avelocity[1] = random()*600;
	gib->avelocity[2] = random()*600;

	gib->think = G_FreeEdict;
	gib->nextthink = level.time + 10 + random()*10;

	gi.linkentity (gib);
}

void ThrowHead (edict_t *self, char *gibname, int damage, int type)
{
	vec3_t	vd;
	float	vscale;

	self->s.skinnum = 0;
	self->s.frame = 0;
	VectorClear (self->mins);
	VectorClear (self->maxs);

	self->s.modelindex2 = 0;
	gi.setmodel (self, gibname);
	self->solid = SOLID_NOT;
	self->s.effects |= EF_GIB;
	self->s.effects &= ~EF_FLIES;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;
	self->svflags &= ~SVF_MONSTER;
	self->takedamage = DAMAGE_YES;
	self->die = gib_die;

	if (type == GIB_ORGANIC)
	{
		self->movetype = MOVETYPE_TOSS;
		self->touch = gib_touch;
		vscale = 0.5;
	}
	else
	{
		self->movetype = MOVETYPE_BOUNCE;
		vscale = 1.0;
	}

	VelocityForDamage (damage, vd);
	VectorMA (self->velocity, vscale, vd, self->velocity);
	ClipGibVelocity (self);

	self->avelocity[YAW] = crandom()*600;

	self->think = G_FreeEdict;
	self->nextthink = level.time + 10 + random()*10;

	gi.linkentity (self);
}


void ThrowClientHead (edict_t *self, int damage)
{
	vec3_t	vd;
	char	*gibname;

	if (rand()&1)
	{
		gibname = "models/objects/gibs/head2/tris.md2";
		self->s.skinnum = 1;		// second skin is player
	}
	else
	{
		gibname = "models/objects/gibs/skull/tris.md2";
		self->s.skinnum = 0;
	}

	self->s.origin[2] += 32;
	self->s.frame = 0;
	gi.setmodel (self, gibname);
	VectorSet (self->mins, -16, -16, 0);
	VectorSet (self->maxs, 16, 16, 16);

	self->takedamage = DAMAGE_NO;
	self->solid = SOLID_NOT;
	self->s.effects = EF_GIB;
	self->s.sound = 0;
	self->flags |= FL_NO_KNOCKBACK;

	self->movetype = MOVETYPE_BOUNCE;
	VelocityForDamage (damage, vd);
	VectorAdd (self->velocity, vd, self->velocity);

	if (self->client)	// bodies in the queue don't have a client anymore
	{
		self->client->anim_priority = ANIM_DEATH;
		self->client->anim_end = self->s.frame;
	}

	gi.linkentity (self);
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
	chunk->nextthink = level.time + 2 + random()*2;
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
}


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
}


/*QUAKED info_notnull (0 0.5 0) (-4 -4 -4) (4 4 4)
Used as a positional target for lightning.
*/
void SP_info_notnull (edict_t *self)
{
	VectorCopy (self->s.origin, self->absmin);
	VectorCopy (self->s.origin, self->absmax);
}


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
explosion, set dmg and it will do a radius explosion of that amount
at the center of the bursh.

If targeted it will not be shootable.

health defaults to 100.

mass defaults to 75.  This determines how much debris is emitted when
it explodes.  You get one large chunk per 100 of mass (up to 8) and
one small chunk per 25 of mass (up to 16).  So 800 gives the most.
*/
void SP_func_explosive (edict_t *self);

void func_explosive_explode (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	vec3_t	origin;
	vec3_t	chunkorigin;
	vec3_t	size;
	int		count;
	int		mass;

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
		if (count > 6)
			count = 6;
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
	if (count > 10)
		count = 10;
	while(count--)
	{
		chunkorigin[0] = origin[0] + crandom() * size[0];
		chunkorigin[1] = origin[1] + crandom() * size[1];
		chunkorigin[2] = origin[2] + crandom() * size[2];
		ThrowDebris (self, "models/objects/debris3/tris.md2", 2, chunkorigin);
	}

	G_UseTargets (self, attacker);

	if (self->dmg)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
//		BecomeExplosion1 (self);
	}
	else
	{
//		G_FreeEdict (self);
	}

	if( attacker->owner && teamplay->value == 1 )
	{
		if( attacker->owner->client )
		{
			gi.bprintf( PRINT_MEDIUM, "[%s]%s destroyed %s's %s.\n",
				team[attacker->owner->client->pers.team-1].name,
				attacker->owner->client->pers.netname,
				(self->aqteam > 0 ? team[self->aqteam-1].name : "neutral"),
				( self->message != NULL ? self->message : "object" ) );
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


	self->flags |= FL_RESPAWN;
	self->svflags |= SVF_NOCLIENT;
	self->solid = SOLID_NOT;
//	VectorSet( self->s.origin, 0, 0, 0 );
	self->nextthink = level.time + 3;
	self->think = SP_func_explosive;
	self = G_Spawn();
	gi.linkentity (self);

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
	}*/ // in AQ2 leave them in the game!

//	gi.bprintf( PRINT_HIGH, "3 seconds!\n" );

	self->movetype = MOVETYPE_NONE;

	gi.modelindex ("models/objects/debris1/tris.md2");
	gi.modelindex ("models/objects/debris2/tris.md2");

	gi.setmodel (self,self->model );

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
	self->classname = "func_explosive";

	gi.linkentity (self);
}


/*QUAKED misc_explobox (0 .5 .8) (-16 -16 0) (16 16 40)
Large exploding box.  You can override its mass (100),
health (80), and dmg (150).
*/
void SP_misc_explobox (edict_t *self);

void barrel_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)

{
	float	ratio;
	vec3_t	v;

	if ((!other->groundentity) || (other->groundentity == self))
		return;

	ratio = (float)other->mass / (float)self->mass;
	VectorSubtract (self->s.origin, other->s.origin, v);
//	M_walkmove (self, vectoyaw(v), 20 * ratio * FRAMETIME);// +BD removed
}

void barrel_explode (edict_t *self)
{
	vec3_t	org;
	float	spd;
	vec3_t	save;

//	gi.bprintf( PRINT_HIGH, "%d %s\n", self->aqteam, self->message );

	T_RadiusDamage (self, self->activator, self->dmg, NULL, self->dmg+40, MOD_BARREL);

	VectorCopy (self->s.origin, save);
	VectorMA (self->absmin, 0.5, self->size, self->s.origin);

	// a few big chunks
	spd = 1.5 * (float)self->dmg / 200.0;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris1/tris.md2", spd, org);
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);

	// bottom corners
	spd = 1.75 * (float)self->dmg / 200.0;
	VectorCopy (self->absmin, org);
	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
//	VectorCopy (self->absmin, org);
//	org[1] += self->size[1];
//	ThrowDebris (self, "models/objects/debris3/tris.md2", spd, org);
	VectorCopy (self->absmin, org);
	org[0] += self->size[0];
	org[1] += self->size[1];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);

	// a bunch of little chunks
	spd = 2 * self->dmg / 200;
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);
/*	org[0] = self->s.origin[0] + crandom() * self->size[0];
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
	org[0] = self->s.origin[0] + crandom() * self->size[0];
	org[1] = self->s.origin[1] + crandom() * self->size[1];
	org[2] = self->s.origin[2] + crandom() * self->size[2];
	ThrowDebris (self, "models/objects/debris2/tris.md2", spd, org);*/

	VectorCopy (save, self->s.origin);
	if (self->groundentity)
		BecomeExplosion2 (self);
	else
		BecomeExplosion1 (self);

}

void barrel_delay (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if( attacker->owner && teamplay->value == 1 )
	{
		if( attacker->owner->client )
		{
			gi.bprintf( PRINT_MEDIUM, "[%s]%s destroyed %s's %s.\n",
				team[attacker->owner->client->pers.team-1].name,
				attacker->owner->client->pers.netname,
				(self->aqteam > 0 ? team[self->aqteam-1].name : "neutral"),
				( self->message != NULL ? self->message : "oiltank" ) );
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

	self->takedamage = DAMAGE_NO;
	self->nextthink = level.time + 2 * FRAMETIME;
	self->think = barrel_explode;
	self->activator = attacker;
}

void SP_misc_explobox (edict_t *self) // these are the oiltanks!
{
/*	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (self);
		return;
	}*/ // leave it in AQ2 +BD

//	gi.modelindex ("models/objects/debris1/tris.md2");
//	gi.modelindex ("models/objects/debris2/tris.md2");
//	gi.modelindex ("models/objects/debris3/tris.md2");

	self->solid = SOLID_BBOX;
	self->movetype = MOVETYPE_NONE;// +BD changed from STEP

//	self->model = "models/objects/barrels/tris.md2";
	self->model = "models/objects/oiltank/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -19, -19, 0);
	VectorSet (self->maxs, 19, 19, 19);

	if (!self->mass)
		self->mass = 400;
	if (!self->health)
		self->health = 200;
	if (!self->dmg)
		self->dmg = 150;

	self->die = barrel_delay;
	self->takedamage = DAMAGE_YES;
	self->monsterinfo.aiflags = AI_NOSTEP;

	self->touch = barrel_touch;

	self->think = M_droptofloor;
	self->nextthink = level.time + 2 * FRAMETIME;

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

/*QUAKED misc_eastertank (1 .5 0) (-32 -32 -16) (32 32 32)
*/

void misc_eastertank_think (edict_t *self)
{
	if (++self->s.frame < 293)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 254;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_eastertank (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -32, -32, -16);
	VectorSet (ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex ("models/monsters/tank/tris.md2");
	ent->s.frame = 254;
	ent->think = misc_eastertank_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED misc_easterchick (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick_think (edict_t *self)
{
	if (++self->s.frame < 247)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 208;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_easterchick (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -32, -32, 0);
	VectorSet (ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
	ent->s.frame = 208;
	ent->think = misc_easterchick_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}

/*QUAKED misc_easterchick2 (1 .5 0) (-32 -32 0) (32 32 32)
*/


void misc_easterchick2_think (edict_t *self)
{
	if (++self->s.frame < 287)
		self->nextthink = level.time + FRAMETIME;
	else
	{
		self->s.frame = 248;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_misc_easterchick2 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -32, -32, 0);
	VectorSet (ent->maxs, 32, 32, 32);
	ent->s.modelindex = gi.modelindex ("models/monsters/bitch/tris.md2");
	ent->s.frame = 248;
	ent->think = misc_easterchick2_think;
	ent->nextthink = level.time + 2 * FRAMETIME;
	gi.linkentity (ent);
}


/*QUAKED monster_commander_body (1 .5 0) (-32 -32 0) (32 32 48)
Not really a monster, this is the Tank Commander's decapitated body.
There should be a item_commander_head that has this as it's target.
*/

void commander_body_think (edict_t *self)
{
	if (++self->s.frame < 24)
		self->nextthink = level.time + FRAMETIME;
	else
		self->nextthink = 0;

	if (self->s.frame == 22)
		gi.sound (self, CHAN_BODY, gi.soundindex ("tank/thud.wav"), 1, ATTN_NORM, 0);
}

void commander_body_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->think = commander_body_think;
	self->nextthink = level.time + FRAMETIME;
	gi.sound (self, CHAN_BODY, gi.soundindex ("tank/pain.wav"), 1, ATTN_NORM, 0);
}

void commander_body_drop (edict_t *self)
{
	self->movetype = MOVETYPE_TOSS;
	self->s.origin[2] += 2;
}

void SP_monster_commander_body (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_BBOX;
	self->model = "models/monsters/commandr/tris.md2";
	self->s.modelindex = gi.modelindex (self->model);
	VectorSet (self->mins, -32, -32, 0);
	VectorSet (self->maxs, 32, 32, 48);
	self->use = commander_body_use;
	self->takedamage = DAMAGE_YES;
	self->flags = FL_GODMODE;
	self->s.renderfx |= RF_FRAMELERP;
	gi.linkentity (self);

	gi.soundindex ("tank/thud.wav");
	gi.soundindex ("tank/pain.wav");

	self->think = commander_body_drop;
	self->nextthink = level.time + 5 * FRAMETIME;
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

/*QUAKED misc_deadsoldier (1 .5 0) (-16 -16 0) (16 16 16) ON_BACK ON_STOMACH BACK_DECAP FETAL_POS SIT_DECAP IMPALED
This is the dead player model. Comes in 6 exciting different poses!
*/
void misc_deadsoldier_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	int		n;

	if (self->health > -80)
		return;

	gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
	for (n= 0; n < 4; n++)
		ThrowGib (self, "models/objects/gibs/sm_meat/tris.md2", damage, GIB_ORGANIC);
	ThrowHead (self, "models/objects/gibs/head2/tris.md2", damage, GIB_ORGANIC);
}

void SP_misc_deadsoldier (edict_t *ent)
{
	if (deathmatch->value)
	{	// auto-remove for deathmatch
		G_FreeEdict (ent);
		return;
	}

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex=gi.modelindex ("models/deadbods/dude/tris.md2");

	// Defaults to frame 0
	if (ent->spawnflags & 2)
		ent->s.frame = 1;
	else if (ent->spawnflags & 4)
		ent->s.frame = 2;
	else if (ent->spawnflags & 8)
		ent->s.frame = 3;
	else if (ent->spawnflags & 16)
		ent->s.frame = 4;
	else if (ent->spawnflags & 32)
		ent->s.frame = 5;
	else
		ent->s.frame = 0;

	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 16);
	ent->deadflag = DEAD_DEAD;
	ent->takedamage = DAMAGE_YES;
	ent->svflags |= SVF_MONSTER|SVF_DEADMONSTER;
	ent->die = misc_deadsoldier_die;
	ent->monsterinfo.aiflags |= AI_GOOD_GUY;

	gi.linkentity (ent);
}

/*QUAKED misc_viper (1 .5 0) (-16 -16 0) (16 16 32)
This is the Viper for the flyby bombing.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast the Viper should fly
*/

extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);

void misc_viper_use  (edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use (self, other, activator);
}

void SP_misc_viper (edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf ("misc_viper without a target at %s\n", vtos(ent->absmin));
		G_FreeEdict (ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ships/viper/tris.md2");
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 32);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_viper_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity (ent);
}


/*QUAKED misc_bigviper (1 .5 0) (-176 -120 -24) (176 120 72)
This is a large stationary viper as seen in Paul's intro
*/
void SP_misc_bigviper (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -176, -120, -24);
	VectorSet (ent->maxs, 176, 120, 72);
	ent->s.modelindex = gi.modelindex ("models/ships/bigviper/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED misc_viper_bomb (1 0 0) (-8 -8 -8) (8 8 8)
"dmg"	how much boom should the bomb make?
*/
void misc_viper_bomb_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	G_UseTargets (self, self->activator);

	self->s.origin[2] = self->absmin[2] + 1;
	T_RadiusDamage (self, self, self->dmg, NULL, self->dmg+40, MOD_BOMB);
	BecomeExplosion2 (self);
}

void misc_viper_bomb_prethink (edict_t *self)
{
	vec3_t	v;
	float	diff;

	self->groundentity = NULL;

	diff = self->timestamp - level.time;
	if (diff < -1.0)
		diff = -1.0;

	VectorScale (self->moveinfo.dir, 1.0 + diff, v);
	v[2] = diff;

	diff = self->s.angles[2];
	vectoangles (v, self->s.angles);
	self->s.angles[2] = diff + 10;
}

void misc_viper_bomb_use (edict_t *self, edict_t *other, edict_t *activator)
{
	edict_t	*viper;

	self->solid = SOLID_BBOX;
	self->svflags &= ~SVF_NOCLIENT;
	self->s.effects |= EF_ROCKET;
	self->use = NULL;
	self->movetype = MOVETYPE_TOSS;
	self->prethink = misc_viper_bomb_prethink;
	self->touch = misc_viper_bomb_touch;
	self->activator = activator;

	viper = G_Find (NULL, FOFS(classname), "misc_viper");
	VectorScale (viper->moveinfo.dir, viper->moveinfo.speed, self->velocity);

	self->timestamp = level.time;
	VectorCopy (viper->moveinfo.dir, self->moveinfo.dir);
}

void SP_misc_viper_bomb (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid = SOLID_NOT;
	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	self->s.modelindex = gi.modelindex ("models/objects/bomb/tris.md2");

	if (!self->dmg)
		self->dmg = 1000;

	self->use = misc_viper_bomb_use;
	self->svflags |= SVF_NOCLIENT;

	gi.linkentity (self);
}


/*QUAKED misc_strogg_ship (1 .5 0) (-16 -16 0) (16 16 32)
This is a Storgg ship for the flybys.
It is trigger_spawned, so you must have something use it for it to show up.
There must be a path for it to follow once it is activated.

"speed"		How fast it should fly
*/

extern void train_use (edict_t *self, edict_t *other, edict_t *activator);
extern void func_train_find (edict_t *self);

void misc_strogg_ship_use  (edict_t *self, edict_t *other, edict_t *activator)
{
	self->svflags &= ~SVF_NOCLIENT;
	self->use = train_use;
	train_use (self, other, activator);
}

void SP_misc_strogg_ship (edict_t *ent)
{
	if (!ent->target)
	{
		gi.dprintf ("%s without a target at %s\n", ent->classname, vtos(ent->absmin));
		G_FreeEdict (ent);
		return;
	}

	if (!ent->speed)
		ent->speed = 300;

	ent->movetype = MOVETYPE_PUSH;
	ent->solid = SOLID_NOT;
	ent->s.modelindex = gi.modelindex ("models/ships/strogg1/tris.md2");
	VectorSet (ent->mins, -16, -16, 0);
	VectorSet (ent->maxs, 16, 16, 32);

	ent->think = func_train_find;
	ent->nextthink = level.time + FRAMETIME;
	ent->use = misc_strogg_ship_use;
	ent->svflags |= SVF_NOCLIENT;
	ent->moveinfo.accel = ent->moveinfo.decel = ent->moveinfo.speed = ent->speed;

	gi.linkentity (ent);
}


/*QUAKED misc_satellite_dish (1 .5 0) (-64 -64 0) (64 64 128)
*/
void misc_satellite_dish_think (edict_t *self)
{
	self->s.frame++;
	if (self->s.frame < 38)
		self->nextthink = level.time + FRAMETIME;
}

void misc_satellite_dish_use (edict_t *self, edict_t *other, edict_t *activator)
{
	self->s.frame = 0;
	self->think = misc_satellite_dish_think;
	self->nextthink = level.time + FRAMETIME;
}

void SP_misc_satellite_dish (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	VectorSet (ent->mins, -64, -64, 0);
	VectorSet (ent->maxs, 64, 64, 128);
	ent->s.modelindex = gi.modelindex ("models/objects/satellite/tris.md2");
	ent->use = misc_satellite_dish_use;
	gi.linkentity (ent);
}


/*QUAKED light_mine1 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine1 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/minelite/light1/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED light_mine2 (0 1 0) (-2 -2 -12) (2 2 12)
*/
void SP_light_mine2 (edict_t *ent)
{
	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_BBOX;
	ent->s.modelindex = gi.modelindex ("models/objects/minelite/light2/tris.md2");
	gi.linkentity (ent);
}


/*QUAKED misc_gib_arm (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_arm (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/arm/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
}

/*QUAKED misc_gib_leg (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_leg (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/leg/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
}

/*QUAKED misc_gib_head (1 0 0) (-8 -8 -8) (8 8 8)
Intended for use with the target_spawner
*/
void SP_misc_gib_head (edict_t *ent)
{
	gi.setmodel (ent, "models/objects/gibs/head/tris.md2");
	ent->solid = SOLID_NOT;
	ent->s.effects |= EF_GIB;
	ent->takedamage = DAMAGE_YES;
	ent->die = gib_die;
	ent->movetype = MOVETYPE_TOSS;
	ent->svflags |= SVF_MONSTER;
	ent->deadflag = DEAD_DEAD;
	ent->avelocity[0] = random()*200;
	ent->avelocity[1] = random()*200;
	ent->avelocity[2] = random()*200;
	ent->think = G_FreeEdict;
	ent->nextthink = level.time + 30;
	gi.linkentity (ent);
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

//=================================================================================

void teleporter_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	edict_t		*dest;
	int			i;

	if (!other->client)
		return;
	dest = G_Find (NULL, FOFS(targetname), self->target);
	if (!dest)
	{
		gi.dprintf ("Couldn't find destination\n");
		return;
	}

	// unlink to make sure it can't possibly interfere with KillBox
	gi.unlinkentity (other);

	VectorCopy (dest->s.origin, other->s.origin);
	VectorCopy (dest->s.origin, other->s.old_origin);
	other->s.origin[2] += 10;

	// clear the velocity and hold them in place briefly
	VectorClear (other->velocity);
	other->client->ps.pmove.pm_time = 160>>3;		// hold time
	other->client->ps.pmove.pm_flags |= PMF_TIME_TELEPORT;

	// draw the teleport splash at source and on the player
	self->owner->s.event = EV_PLAYER_TELEPORT;
	other->s.event = EV_PLAYER_TELEPORT;

	// set angles
	for (i=0 ; i<3 ; i++)
		other->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(dest->s.angles[i] - other->client->resp.cmd_angles[i]);

	VectorClear (other->s.angles);
	VectorClear (other->client->ps.viewangles);
	VectorClear (other->client->v_angle);

	// kill anything at the destination
	KillBox (other);

	gi.linkentity (other);
}

/*QUAKED misc_teleporter (1 0 0) (-32 -32 -24) (32 32 -16)
Stepping onto this disc will teleport players to the targeted misc_teleporter_dest object.
*/
void SP_misc_teleporter (edict_t *ent)
{
	edict_t		*trig;

	if (!ent->target)
	{
		gi.dprintf ("teleporter without a target.\n");
		G_FreeEdict (ent);
		return;
	}

	gi.setmodel (ent, "models/objects/dmspot/tris.md2");
	ent->s.skinnum = 1;
	ent->s.effects = EF_TELEPORTER;
	ent->s.sound = gi.soundindex ("world/amb10.wav");
	ent->solid = SOLID_BBOX;

	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -16);
	gi.linkentity (ent);

	trig = G_Spawn ();
	trig->touch = teleporter_touch;
	trig->solid = SOLID_TRIGGER;
	trig->target = ent->target;
	trig->owner = ent;
	VectorCopy (ent->s.origin, trig->s.origin);
	VectorSet (trig->mins, -8, -8, 8);
	VectorSet (trig->maxs, 8, 8, 24);
	gi.linkentity (trig);

}

/*QUAKED misc_teleporter_dest (1 0 0) (-32 -32 -24) (32 32 -16)
Point teleporters at these.
*/
void SP_misc_teleporter_dest (edict_t *ent)
{
// +BD As the vehicles will not teleport we dont need that:
//	gi.setmodel (ent, "models/objects/dmspot/tris.md2");	// +BD removed
	ent->s.skinnum = 0;
	ent->solid = SOLID_NOT;		// +BD changed from BBOX
//	ent->s.effects |= EF_FLIES;
	VectorSet (ent->mins, -32, -32, -24);
	VectorSet (ent->maxs, 32, 32, -16);
	gi.linkentity (ent);
}

/*
=============
visible

returns 1 if the entity is visible to self, even if not infront ()
=============
*/
qboolean visible (edict_t *self, edict_t *other)
{
	vec3_t	spot1;
	vec3_t	spot2;
	trace_t	trace;

	VectorCopy (self->s.origin, spot1);
	spot1[2] += self->viewheight;
	VectorCopy (other->s.origin, spot2);
	spot2[2] += other->viewheight;
	trace = gi.trace (spot1, vec3_origin, vec3_origin, spot2, self, MASK_OPAQUE);

	if (trace.fraction == 1.0)
		return true;
	return false;
}

/*
=============
infront

returns 1 if the entity is in front (in sight) of self
=============
*/
qboolean infront (edict_t *self, edict_t *other)
{
	vec3_t	vec;
	float	dot;
	vec3_t	forward;

	AngleVectors (self->s.angles, forward, NULL, NULL);
	VectorSubtract (other->s.origin, self->s.origin, vec);
	VectorNormalize (vec);
	dot = DotProduct (vec, forward);

	if (dot > 0.3)
		return true;
	return false;
}

void M_droptofloor (edict_t *ent)
{
	vec3_t		end;
	trace_t		trace;

	ent->s.origin[2] += 1;
	VectorCopy (ent->s.origin, end);
	end[2] -= 256;

	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, end, ent, MASK_MONSTERSOLID);

	if (trace.fraction == 1 || trace.allsolid)
		return;

	VectorCopy (trace.endpos, ent->s.origin);

	gi.linkentity (ent);
	M_CheckGround (ent);
	M_CatagorizePosition (ent);
}

void M_CatagorizePosition (edict_t *ent)
{
	vec3_t		point;
	int			cont;

//
// get waterlevel
//
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] + ent->mins[2] + 1;
	cont = gi.pointcontents (point);

	if (!(cont & MASK_WATER))
	{
		ent->waterlevel = 0;
		ent->watertype = 0;
		return;
	}

	ent->watertype = cont;
	ent->waterlevel = 1;
	point[2] += 26;
	cont = gi.pointcontents (point);
	if (!(cont & MASK_WATER))
		return;

	ent->waterlevel = 2;
	point[2] += 22;
	cont = gi.pointcontents (point);
	if (cont & MASK_WATER)
		ent->waterlevel = 3;
}

void M_CheckGround (edict_t *ent)
{
	vec3_t		point;
	trace_t		trace;

	if (ent->flags & (FL_SWIM|FL_FLY))
		return;

	if (ent->velocity[2] > 100)
	{
		ent->groundentity = NULL;
		return;
	}

// if the hull point one-quarter unit down is solid the entity is on ground
	point[0] = ent->s.origin[0];
	point[1] = ent->s.origin[1];
	point[2] = ent->s.origin[2] - 0.25;

	trace = gi.trace (ent->s.origin, ent->mins, ent->maxs, point, ent, MASK_MONSTERSOLID);

	// check steepness
	if ( trace.plane.normal[2] < 0.7 && !trace.startsolid)
	{
		ent->groundentity = NULL;
		return;
	}

//	ent->groundentity = trace.ent;
//	ent->groundentity_linkcount = trace.ent->linkcount;
//	if (!trace.startsolid && !trace.allsolid)
//		VectorCopy (trace.endpos, ent->s.origin);
	if (!trace.startsolid && !trace.allsolid)
	{
		VectorCopy (trace.endpos, ent->s.origin);
		ent->groundentity = trace.ent;
		ent->groundentity_linkcount = trace.ent->linkcount;
		ent->velocity[2] = 0;
	}
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

	if( attacker )
	{
		if( attacker != self && !(attacker->HASCAN & IS_DRONE) && attacker->owner)// drone
		{
			if( attacker->owner->client )
			{

				if( attacker->enemy )
				{
					if( attacker->enemy == self )
						attacker->enemy = NULL;
				}
				if( !self->deadflag )
				{
					if( teamplay->value == 1 )
					{
						if( self->aqteam != attacker->aqteam )
						{
							attacker->owner->client->resp.score += 1;
							team[attacker->owner->client->pers.team-1].score += 1;
							if( !attacker->deadflag )
								gi.bprintf( PRINT_MEDIUM, "%s destroyed an enemy vehicle!\n",
								attacker->owner->client->pers.netname );
						}
						else
						{
							attacker->owner->client->resp.score -= 2;
							team[attacker->owner->client->pers.team-1].score -= 2;
							if( !attacker->deadflag )
								gi.bprintf( PRINT_MEDIUM, "%s destroyed a friendly vehicle!\n",
								attacker->owner->client->pers.netname );
						}
					}
					else
					{
						attacker->owner->client->resp.score += 1;
						team[attacker->owner->client->pers.team-1].score += 1;
						if( !attacker->deadflag )
							gi.bprintf( PRINT_MEDIUM, "%s killed a parked vehicle!\n",
							attacker->owner->client->pers.netname );
					}
				}
			}
		}
	}


	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->deadflag = DEAD_DEAD;
	SetVehicleModel( self );

}



void misc_vehicle_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( self->deadflag || strcmp( other->classname, "freed" ) == 0
		|| strcmp( other->classname, "debris" ) == 0 )
		return;
	if( strcmp( other->classname, "rocket" ) == 0 ||
		strcmp( other->classname, "ATGM" ) == 0 ||
   		strcmp( other->classname, "sidewinder" ) == 0 ||
		strcmp( other->classname, "bolt" ) == 0 ||
		strcmp( other->classname, "shell" ) == 0 ||
		strcmp( other->classname, "bomb" ) == 0 )
		return;
	else if( strcmp( other->classname, "func_runway") == 0
   		 && ( self->ONOFF & LANDED ) )
		return;

}

void misc_vehicle_think( edict_t *vehicle )
{
	// health-effects
	if( vehicle->health >= 26 )
		vehicle->s.effects = 0;
	else if( vehicle->health < 26 && vehicle->health > 10 )
	{
		vehicle->s.effects = EF_GRENADE;
	}
	else if( vehicle->health <= 10 && vehicle->deadflag != DEAD_DEAD )
	{
		vehicle->s.effects = EF_ROCKET;
	}
	else
		vehicle->s.effects = 0;

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
		case 111:	// F14
					Start_F14( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case 112:	// F22
					Start_F22( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					self->s.origin[2] -= 1;
					self->s.frame = 4;
					break;
		case 113:	// F16
					Start_F16( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case 114:	// F4
					Start_F4( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case 115:	// SU37
					Start_SU37( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case 116:	// MiG21
					Start_MiG21( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case 117:	// MiG29
					Start_MiG29( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case 118:	// MiG29
					Start_MiGMFI( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					//self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case 121:	// Tornado
					Start_Tornado( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
		case 122:	// A10
					Start_A10( self );
					self->ONOFF |= (LANDED | GEARDOWN );
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case 123:	// F117
					Start_F117( self );
					self->ONOFF |= (LANDED | GEARDOWN );
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case 124:	// Su24
					Start_Su24( self );
					self->ONOFF |= (LANDED | GEARDOWN );
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case 125:	// Su25
					Start_SU25( self );
					self->ONOFF |= (LANDED | GEARDOWN );
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case 126:	// Su34
					Start_SU34( self );
					self->ONOFF |= (LANDED | GEARDOWN );
					self->s.origin[2] += 2;
					self->s.frame = 4;
					break;
		case 211:	// AH64
					Start_AH64( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					self->s.origin[2] -= 3;
					self->s.frame = 0;
					break;
		case 212:	// MI28
					Start_MI28( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					self->s.origin[2] += 5;
					self->s.frame = 0;
					break;
		case 221:	// RAH66
					Start_RAH66( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					self->s.origin[2] -= 1;
					self->s.frame = 0;
					break;
		case 222:	// Ka50
					Start_KA50( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					self->s.frame = 0;
					break;
		case 411:	// M1A2
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
					break;
		default:	// default gives you an A10
					gi.dprintf( "Non valid vehicle! Using default!\n" );
					Start_F16( self );
					self->ONOFF |= (LANDED | GEARDOWN);
					self->s.origin[2] += 1;
					self->s.frame = 4;
					break;
	}
	if( ( strcmp( self->classname,"helo" ) == 0 ) &&
		(self->HASCAN & IS_TAILDRAGGER) )
		self->s.angles[0] = -5;
	self->movetype = MOVETYPE_FLYMISSILE;
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_AIM;
	self->thrust = THRUST_0;
	if( self->HASCAN & IS_EASTERN )
		self->aqteam = 2;
	else
		self->aqteam = 1;

	self->touch = misc_vehicle_touch;
	self->pain = misc_vehicle_pain;
	self->die = misc_vehicle_die;
	self->think = misc_vehicle_think;
	self->nextthink = level.time + 0.1;
	gi.linkentity (self);
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
	VectorSet (self->mins, -5, -5, 0);
	VectorSet (self->maxs, 5, 5,30);
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
	VectorSet (self->mins, -20, -20, 0);
	VectorSet (self->maxs, 20, 20, 34);
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

	if( attacker->owner && teamplay->value == 1 )
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
	self->touch = tree_touch;
	self->pain = tree_pain;
	self->die = tree_die;
	self->think = treetofloor;
	self->nextthink =level.time + 0.2;

	gi.linkentity (self);
}

