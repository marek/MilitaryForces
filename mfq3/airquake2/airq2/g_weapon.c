#include "g_local.h"


/*
=================
check_dodge

This is a support routine used when a client is firing
a non-instant attack weapon.  It checks to see if a
monster's dodge function should be called.
=================
*/
/*
static void check_dodge (edict_t *self, vec3_t start, vec3_t dir, int speed)
{
	vec3_t	end;
	vec3_t	v;
	trace_t	tr;
	float	eta;

	// easy mode only ducks one quarter the time
	if (skill->value == 0)
	{
		if (random() > 0.25)
			return;
	}
	VectorMA (start, 8192, dir, end);
	tr = gi.trace (start, NULL, NULL, end, self, MASK_SHOT);
	if ((tr.ent) && (tr.ent->svflags & SVF_MONSTER) && (tr.ent->health > 0) && (tr.ent->monsterinfo.dodge) && infront(tr.ent, self))
	{
		VectorSubtract (tr.endpos, start, v);
		eta = (VectorLength(v) - tr.ent->maxs[0]) / speed;
		tr.ent->monsterinfo.dodge (tr.ent, self, eta);
	}
}
*/

/*
=================
fire_hit

Used for all impact (hit/punch/slash) attacks
=================
*/
qboolean fire_hit (edict_t *self, vec3_t aim, int damage, int kick)
{
	trace_t		tr;
	vec3_t		forward, right, up;
	vec3_t		v;
	vec3_t		point;
	float		range;
	vec3_t		dir;

	//see if enemy is in range
	VectorSubtract (self->enemy->s.origin, self->s.origin, dir);
	range = VectorLength(dir);
	if (range > aim[0])
		return false;

	if (aim[1] > self->mins[0] && aim[1] < self->maxs[0])
	{
		// the hit is straight on so back the range up to the edge of their bbox
		range -= self->enemy->maxs[0];
	}
	else
	{
		// this is a side hit so adjust the "right" value out to the edge of their bbox
		if (aim[1] < 0)
			aim[1] = self->enemy->mins[0];
		else
			aim[1] = self->enemy->maxs[0];
	}

	VectorMA (self->s.origin, range, dir, point);

	tr = gi.trace (self->s.origin, NULL, NULL, point, self, MASK_SHOT);
	if (tr.fraction < 1)
	{
		if (!tr.ent->takedamage)
			return false;
		// if it will hit any client/monster then hit the one we wanted to hit
		if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
			tr.ent = self->enemy;
	}

	AngleVectors(self->s.angles, forward, right, up);
	VectorMA (self->s.origin, range, forward, point);
	VectorMA (point, aim[1], right, point);
	VectorMA (point, aim[2], up, point);
	VectorSubtract (point, self->enemy->s.origin, dir);

	// do the damage
	T_Damage (tr.ent, self, self, dir, point, vec3_origin, damage, kick/2, DAMAGE_NO_KNOCKBACK, MOD_HIT);

	if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
		return false;

	// do our special form of knockback here
	VectorMA (self->enemy->absmin, 0.5, self->enemy->size, v);
	VectorSubtract (v, point, v);
	VectorNormalize (v);
	VectorMA (self->enemy->velocity, kick, v, self->enemy->velocity);
	if (self->enemy->velocity[2] > 0)
		self->enemy->groundentity = NULL;
	return true;
}


/*
=================
fire_lead

This is an internal support routine used for bullet/pellet based weapons.
=================
*/
static void fire_lead (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int te_impact, int hspread, int vspread, int mod)
{
	trace_t		tr;
	vec3_t		dir;
	vec3_t		forward, right, up;
	vec3_t		end;
	float		r;
	float		u;
	vec3_t		water_start;
	qboolean	water = false;
	int			content_mask = MASK_SHOT | MASK_WATER;

	tr = gi.trace (self->s.origin, NULL, NULL, start, self, MASK_SHOT);
	if (!(tr.fraction < 1.0))
	{
		vectoangles (aimdir, dir);
		AngleVectors (dir, forward, right, up);

		r = crandom()*hspread;
		u = crandom()*vspread;
		VectorMA (start, 8192, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		if (gi.pointcontents (start) & MASK_WATER)
		{
			water = true;
			VectorCopy (start, water_start);
			content_mask &= ~MASK_WATER;
		}

		tr = gi.trace (start, NULL, NULL, end, self, content_mask);

		// see if we hit water
		if (tr.contents & MASK_WATER)
		{
			int		color;

			water = true;
			VectorCopy (tr.endpos, water_start);

			if (!VectorCompare (start, tr.endpos))
			{
				if (tr.contents & CONTENTS_WATER)
				{
					if (strcmp(tr.surface->name, "*brwater") == 0)
						color = SPLASH_BROWN_WATER;
					else
						color = SPLASH_BLUE_WATER;
				}
				else if (tr.contents & CONTENTS_SLIME)
					color = SPLASH_SLIME;
				else if (tr.contents & CONTENTS_LAVA)
					color = SPLASH_LAVA;
				else
					color = SPLASH_UNKNOWN;

				if (color != SPLASH_UNKNOWN)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_SPLASH);
					gi.WriteByte (8);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.WriteByte (color);
					gi.multicast (tr.endpos, MULTICAST_PVS);
				}

				// change bullet's course when it enters water
				VectorSubtract (end, start, dir);
				vectoangles (dir, dir);
				AngleVectors (dir, forward, right, up);
				r = crandom()*hspread*2;
				u = crandom()*vspread*2;
				VectorMA (water_start, 8192, forward, end);
				VectorMA (end, r, right, end);
				VectorMA (end, u, up, end);
			}

			// re-trace ignoring water this time
			tr = gi.trace (water_start, NULL, NULL, end, self, MASK_SHOT);
		}
	}

	// send gun puff / flash
	if (!((tr.surface) && (tr.surface->flags & SURF_SKY)))
	{
		if (tr.fraction < 1.0)
		{
			if (tr.ent->takedamage && strcmp( tr.ent->classname, "func_explosive") != 0 )
			{
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, DAMAGE_BULLET, mod);
			}
			else
			{
				if (strncmp (tr.surface->name, "sky", 3) != 0)
				{
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (te_impact);
					gi.WritePosition (tr.endpos);
					gi.WriteDir (tr.plane.normal);
					gi.multicast (tr.endpos, MULTICAST_PVS);

					if (self->client)
						PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
				}
			}
		}
	}

	// if went through water, determine where the end and make a bubble trail
	if (water)
	{
		vec3_t	pos;

		VectorSubtract (tr.endpos, water_start, dir);
		VectorNormalize (dir);
		VectorMA (tr.endpos, -2, dir, pos);
		if (gi.pointcontents (pos) & MASK_WATER)
			VectorCopy (pos, tr.endpos);
		else
			tr = gi.trace (pos, NULL, NULL, water_start, tr.ent, MASK_WATER);

		VectorAdd (water_start, tr.endpos, pos);
		VectorScale (pos, 0.5, pos);

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BUBBLETRAIL);
		gi.WritePosition (water_start);
		gi.WritePosition (tr.endpos);
		gi.multicast (pos, MULTICAST_PVS);
	}
}


/*
=================
fire_bullet

Fires a single round.  Used for machinegun and chaingun.  Would be fine for
pistols, rifles, etc....
=================
*/
void fire_bullet (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod)
{
	fire_lead (self, start, aimdir, damage, kick, TE_GUNSHOT, hspread, vspread, mod);
}


/*
=================
fire_shotgun

Shoots shotgun pellets.  Used by shotgun and super shotgun.
=================
*/
void fire_shotgun (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int count, int mod)
{
	int		i;

	for (i = 0; i < count; i++)
		fire_lead (self, start, aimdir, damage, kick, TE_SHOTGUN, hspread, vspread, mod);
}


/*
=================
fire_blaster

Fires a single blaster bolt.  Used by the blaster and hyper blaster.
=================
*/
void blaster_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;

	if( !self->dmg )
		return;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		if( self->owner->owner )
		{
			if( self->owner->owner->client->cameramode == 3
				&& self->owner->owner->cam_target == self )
				self->owner->owner->cam_target = self->owner->owner->oldenemy;
			if( self->owner->owner->goalentity == self )
				self->owner->owner->goalentity = NULL;
		}
			G_FreeEdict (self);
			return;
	}

//	if (self->owner->owner->client)	// +BD +owner
//		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);// Achtung +owner ?

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_GUNSHOT);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	if (other->takedamage)
	{
		if (self->spawnflags & 1)
			mod = MOD_AUTOCANNON;
		else
			mod = MOD_BLASTER;

		if( strcmp( other->classname, "func_explosive" ) != 0 )
			T_Damage (other, self, self->owner, self->velocity, self->s.origin,
						plane->normal, self->dmg, 1, DAMAGE_BULLET, mod);
		else
			T_Damage( other, self, self->owner, self->velocity, self->s.origin,
						plane->normal, self->dmg-15, 1, DAMAGE_BULLET, mod );
		// +BD owner von other entfernt
	}

	if( self->owner->owner )
	{
		if( self->owner->owner->client->cameramode == 3
			&& self->owner->owner->cam_target == self )
			self->owner->owner->cam_target = self->owner->owner->oldenemy;
		if( self->owner->owner->goalentity == self )
			self->owner->owner->goalentity = NULL;
	}

	G_FreeEdict (self);
}

void fire_blaster (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*bolt;
//	trace_t	tr;

	VectorNormalize (dir);

	if( !(self->HASCAN & IS_DRONE ) )
	{
		self->owner->client->machinegun_shots++;
		if( self->owner->client->machinegun_shots > 4 )
			self->owner->client->machinegun_shots = 0;
	}

	bolt = G_Spawn();
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	bolt->s.effects |= effect;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	if( !(self->HASCAN & IS_DRONE) )
	{
		if( self->owner->client->machinegun_shots == 1 )
			bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	}
	else
		bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	bolt->owner = self;
	bolt->touch = blaster_touch;
	bolt->nextthink = level.time + 2.5;
	bolt->think = G_FreeEdict;
	bolt->dmg = damage;
	bolt->classname = "bolt";
	bolt->spawnflags = 1;
	gi.linkentity (bolt);

// camera DOES NOT follow shots now!
/*	if( !(self->owner->HASCAN & IS_DRONE) )// drone
	{
		if( self->owner->client->machinegun_shots == 1 )
		{
			if( self->owner->client->cameramode == 3 )
				self->owner->cam_target = bolt;
			self->owner->goalentity = bolt;
		}
	}
*/
}


/*
=================
fire_grenade
=================
*/
static void Grenade_Explode (edict_t *ent)
{
	vec3_t		origin;
	int			mod;

	if (ent->owner->client)
		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT);

	//FIXME: if we are onground then raise our Z just a bit since we are a point?
	if (ent->enemy)
	{
		float	points;
		vec3_t	v;
		vec3_t	dir;

		VectorAdd (ent->enemy->mins, ent->enemy->maxs, v);
		VectorMA (ent->enemy->s.origin, 0.5, v, v);
		VectorSubtract (ent->s.origin, v, v);
		points = ent->dmg - 0.5 * VectorLength (v);
		VectorSubtract (ent->enemy->s.origin, ent->s.origin, dir);
		if (ent->spawnflags & 1)
			mod = MOD_HANDGRENADE;
		else
			mod = MOD_GRENADE;
		T_Damage (ent->enemy, ent, ent->owner, dir, ent->s.origin, vec3_origin, (int)points, (int)points, DAMAGE_RADIUS, mod);
	}

	if (ent->spawnflags & 2)
		mod = MOD_HELD_GRENADE;
	else if (ent->spawnflags & 1)
		mod = MOD_HG_SPLASH;
	else
		mod = MOD_G_SPLASH;
	T_RadiusDamage(ent, ent->owner, ent->dmg, ent->enemy, ent->dmg_radius, mod);

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);
	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION_WATER);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	}
	else
	{
		if (ent->groundentity)
			gi.WriteByte (TE_GRENADE_EXPLOSION);
		else
			gi.WriteByte (TE_ROCKET_EXPLOSION);
	}
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

static void Grenade_Touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	if (!other->takedamage)
	{
		if (ent->spawnflags & 1)
		{
			if (random() > 0.5)
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb1a.wav"), 1, ATTN_NORM, 0);
			else
				gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/hgrenb2a.wav"), 1, ATTN_NORM, 0);
		}
		else
		{
			gi.sound (ent, CHAN_VOICE, gi.soundindex ("weapons/grenlb1b.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	ent->enemy = other;
	Grenade_Explode (ent);
}

void fire_grenade (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "grenade";

	gi.linkentity (grenade);
}

void fire_grenade2 (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int speed, float timer, float damage_radius, qboolean held)
{
	edict_t	*grenade;
	vec3_t	dir;
	vec3_t	forward, right, up;

	vectoangles (aimdir, dir);
	AngleVectors (dir, forward, right, up);

	grenade = G_Spawn();
	VectorCopy (start, grenade->s.origin);
	VectorScale (aimdir, speed, grenade->velocity);
	VectorMA (grenade->velocity, 200 + crandom() * 10.0, up, grenade->velocity);
	VectorMA (grenade->velocity, crandom() * 10.0, right, grenade->velocity);
	VectorSet (grenade->avelocity, 300, 300, 300);
	grenade->movetype = MOVETYPE_BOUNCE;
	grenade->clipmask = MASK_SHOT;
	grenade->solid = SOLID_BBOX;
	grenade->s.effects |= EF_GRENADE;
	VectorClear (grenade->mins);
	VectorClear (grenade->maxs);
	grenade->s.modelindex = gi.modelindex ("models/objects/grenade2/tris.md2");
	grenade->owner = self;
	grenade->touch = Grenade_Touch;
	grenade->nextthink = level.time + timer;
	grenade->think = Grenade_Explode;
	grenade->dmg = damage;
	grenade->dmg_radius = damage_radius;
	grenade->classname = "hgrenade";
	if (held)
		grenade->spawnflags = 3;
	else
		grenade->spawnflags = 1;
	grenade->s.sound = gi.soundindex("weapons/hgrenc1b.wav");

	if (timer <= 0.0)
		Grenade_Explode (grenade);
	else
	{
		gi.sound (self, CHAN_WEAPON, gi.soundindex ("weapons/hgrent1a.wav"), 1, ATTN_NORM, 0);
		gi.linkentity (grenade);
	}
}


/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

//	if (other == ent->owner)
//		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		if( ent->owner->owner )
		{
			if( ent->owner->owner->client->cameramode == 3
				&& ent->owner->owner->cam_target == ent )
				ent->owner->owner->cam_target = ent->owner->owner->oldenemy;
			if( ent->owner->owner->goalentity == ent )
				ent->owner->owner->goalentity = NULL;
		}

		G_FreeEdict (ent);
		return;
	}

//	gi.bprintf( PRINT_HIGH,"Einschlag\n" );

//	if (ent->owner->owner->client)		// +BD +owner
//		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT); //Achtung +owner ?

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, DAMAGE_NO_PROTECTION, MOD_ROCKET);
		// +BD owner von other entfernt
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_ROCKET);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if( ent->owner->owner )
	{
		if( ent->owner->owner->client->cameramode == 3
			&& ent->owner->owner->cam_target == ent )
			ent->owner->owner->cam_target = ent->owner->owner->oldenemy;
		if( ent->owner->owner->goalentity == ent )
			ent->owner->owner->goalentity = NULL;
	}

	G_FreeEdict (ent);
}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius )
{
	edict_t	*rocket;

//	gi.bprintf( PRINT_HIGH, "Feuer\n" );

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (start, rocket->s.old_origin);	// +BD
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	if( rockettrail->value )
		rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/objects/rocket/tris.md2");
	rocket->owner = self;
	rocket->touch = rocket_touch;
	rocket->nextthink = level.time + 8000/speed;
	rocket->think = G_FreeEdict;
	rocket->dmg = damage;
	rocket->radius_dmg = damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	gi.linkentity (rocket);

	if( self->owner->client->cameramode == 3 )
		self->owner->cam_target = rocket;
	self->owner->goalentity = rocket;
	if( weaponinfo->value != 0 )
	{
		gi.cprintf( self->owner, PRINT_HIGH, "Name: FFAR, Maneuverability: XX, Speed: %d\n",
			speed );
		gi.cprintf( self->owner, PRINT_HIGH, "Damage: %d, Damage-Radius: %.0f, Lock-Range: XX, Endurace: XX\n",
			damage, damage_radius );
	}
}


/*
=================
fire_rail
=================
*/
void fire_rail (edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick)
{
	vec3_t		from;
	vec3_t		end;
	trace_t		tr;
	edict_t		*ignore;
	int			mask;
	qboolean	water;

	VectorMA (start, 8192, aimdir, end);
	VectorCopy (start, from);
	ignore = self;
	water = false;
	mask = MASK_SHOT|CONTENTS_SLIME|CONTENTS_LAVA;
	while (ignore)
	{
		tr = gi.trace (from, NULL, NULL, end, ignore, mask);

		if (tr.contents & (CONTENTS_SLIME|CONTENTS_LAVA))
		{
			mask &= ~(CONTENTS_SLIME|CONTENTS_LAVA);
			water = true;
		}
		else
		{
			if ((tr.ent->svflags & SVF_MONSTER) || (tr.ent->client))
				ignore = tr.ent;
			else
				ignore = NULL;

			if ((tr.ent != self) && (tr.ent->takedamage))
				T_Damage (tr.ent, self, self, aimdir, tr.endpos, tr.plane.normal, damage, kick, 0, MOD_RAILGUN);
		}

		VectorCopy (tr.endpos, from);
	}

	// send gun puff / flash
	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_RAILTRAIL);
	gi.WritePosition (start);
	gi.WritePosition (tr.endpos);
	gi.multicast (self->s.origin, MULTICAST_PHS);
//	gi.multicast (start, MULTICAST_PHS);
	if (water)
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_RAILTRAIL);
		gi.WritePosition (start);
		gi.WritePosition (tr.endpos);
		gi.multicast (tr.endpos, MULTICAST_PHS);
	}

	if (self->client)
		PlayerNoise(self, tr.endpos, PNOISE_IMPACT);
}


/*
=================
fire_bfg
=================
*/
void bfg_explode (edict_t *self)
{
	edict_t	*ent;
	float	points;
	vec3_t	v;
	float	dist;

	if (self->s.frame == 0)
	{
		// the BFG effect
		ent = NULL;
		while ((ent = findradius(ent, self->s.origin, self->dmg_radius)) != NULL)
		{
			if (!ent->takedamage)
				continue;
			if (ent == self->owner)
				continue;
			if (!CanDamage (ent, self))
				continue;
			if (!CanDamage (ent, self->owner))
				continue;

			VectorAdd (ent->mins, ent->maxs, v);
			VectorMA (ent->s.origin, 0.5, v, v);
			VectorSubtract (self->s.origin, v, v);
			dist = VectorLength(v);
			points = self->radius_dmg * (1.0 - sqrt(dist/self->dmg_radius));
			if (ent == self->owner)
				points = points * 0.5;

			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_BFG_EXPLOSION);
			gi.WritePosition (ent->s.origin);
			gi.multicast (ent->s.origin, MULTICAST_PHS);
			T_Damage (ent, self, self->owner, self->velocity, ent->s.origin, vec3_origin, (int)points, 0, DAMAGE_ENERGY, MOD_BFG_EFFECT);
		}
	}

	self->nextthink = level.time + FRAMETIME;
	self->s.frame++;
	if (self->s.frame == 5)
		self->think = G_FreeEdict;
}

void bfg_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (self->owner->client)
		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);

	// core explosion - prevents firing it into the wall/floor
	if (other->takedamage)
		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, 200, 0, 0, MOD_BFG_BLAST);
	T_RadiusDamage(self, self->owner, 200, other, 100, MOD_BFG_BLAST);

	gi.sound (self, CHAN_VOICE, gi.soundindex ("weapons/bfg__x1b.wav"), 1, ATTN_NORM, 0);
	self->solid = SOLID_NOT;
	self->touch = NULL;
	VectorMA (self->s.origin, -1 * FRAMETIME, self->velocity, self->s.origin);
	VectorClear (self->velocity);
	self->s.modelindex = gi.modelindex ("sprites/s_bfg3.sp2");
	self->s.frame = 0;
	self->s.sound = 0;
	self->s.effects &= ~EF_ANIM_ALLFAST;
	self->think = bfg_explode;
	self->nextthink = level.time + FRAMETIME;
	self->enemy = other;

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_BFG_BIGEXPLOSION);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
}


void bfg_think (edict_t *self)
{
	edict_t	*ent;
	edict_t	*ignore;
	vec3_t	point;
	vec3_t	dir;
	vec3_t	start;
	vec3_t	end;
	int		dmg;
	trace_t	tr;

	if (deathmatch->value)
		dmg = 5;
	else
		dmg = 10;

	ent = NULL;
	while ((ent = findradius(ent, self->s.origin, 256)) != NULL)
	{
		if (ent == self)
			continue;

		if (ent == self->owner)
			continue;

		if (!ent->takedamage)
			continue;

		if (!(ent->svflags & SVF_MONSTER) && (!ent->client) && (strcmp(ent->classname, "misc_explobox") != 0))
			continue;

		VectorMA (ent->absmin, 0.5, ent->size, point);

		VectorSubtract (point, self->s.origin, dir);
		VectorNormalize (dir);

		ignore = self;
		VectorCopy (self->s.origin, start);
		VectorMA (start, 2048, dir, end);
		while(1)
		{
			tr = gi.trace (start, NULL, NULL, end, ignore, CONTENTS_SOLID|CONTENTS_MONSTER|CONTENTS_DEADMONSTER);

			if (!tr.ent)
				break;

			// hurt it if we can
			if ((tr.ent->takedamage) && !(tr.ent->flags & FL_IMMUNE_LASER) && (tr.ent != self->owner))
				T_Damage (tr.ent, self, self->owner, dir, tr.endpos, vec3_origin, dmg, 1, DAMAGE_ENERGY, MOD_BFG_LASER);

			// if we hit something that's not a monster or player we're done
			if (!(tr.ent->svflags & SVF_MONSTER) && (!tr.ent->client))
			{
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_LASER_SPARKS);
				gi.WriteByte (4);
				gi.WritePosition (tr.endpos);
				gi.WriteDir (tr.plane.normal);
				gi.WriteByte (self->s.skinnum);
				gi.multicast (tr.endpos, MULTICAST_PVS);
				break;
			}

			ignore = tr.ent;
			VectorCopy (tr.endpos, start);
		}

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_BFG_LASER);
		gi.WritePosition (self->s.origin);
		gi.WritePosition (tr.endpos);
		gi.multicast (self->s.origin, MULTICAST_PHS);
	}

	self->nextthink = level.time + FRAMETIME;
}


void fire_bfg (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bfg;

	bfg = G_Spawn();
	VectorCopy (start, bfg->s.origin);
	VectorCopy (dir, bfg->movedir);
	vectoangles (dir, bfg->s.angles);
	VectorScale (dir, speed, bfg->velocity);
	bfg->movetype = MOVETYPE_FLYMISSILE;
	bfg->clipmask = MASK_SHOT;
	bfg->solid = SOLID_BBOX;
	bfg->s.effects |= EF_BFG | EF_ANIM_ALLFAST;
	VectorClear (bfg->mins);
	VectorClear (bfg->maxs);
	bfg->s.modelindex = gi.modelindex ("sprites/s_bfg1.sp2");
	bfg->owner = self;
	bfg->touch = bfg_touch;
	bfg->nextthink = level.time + 8000/speed;
	bfg->think = G_FreeEdict;
	bfg->radius_dmg = damage;
	bfg->dmg_radius = damage_radius;
	bfg->classname = "bfg blast";
	bfg->s.sound = gi.soundindex ("weapons/bfg__l1a.wav");

	bfg->think = bfg_think;
	bfg->nextthink = level.time + FRAMETIME;
	bfg->teammaster = bfg;
	bfg->teamchain = NULL;

//	if (self->client)
//		check_dodge (self, bfg->s.origin, dir, speed);

	gi.linkentity (bfg);
}


/*
=================
fire_sidewinder
=================
*/
void nofuelthink( edict_t *ent )
{
		ent->movetype = MOVETYPE_TOSS;
		ent->s.effects = 0;
		ent->s.skinnum = 0;
		VectorClear( ent->avelocity );

		gi.linkentity( ent );
}

void nolockthink( edict_t *ent );

void explodeside( edict_t *ent )
{
	vec3_t origin;

	ent->s.skinnum = 0;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, ent->stealth );

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if( ent->enemy )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
	}

	G_FreeEdict (ent);
}

void lockthink(edict_t *ent)
{
	vec3_t	targetdir;
	vec3_t  aimatthis;
	vec3_t  owner_dist;
	edict_t *lock = NULL;
	float speed, distance;// time to target in frames

	if( strcmp( ent->enemy->classname, "freed" ) == 0 )
	{
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy->deadflag )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !visible( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !infront( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy->stealth > 0 )//stealth
	{
		if( !( ent->enemy->ONOFF & BAY_OPEN) && !(ent->enemy->ONOFF & GEARDOWN) )
		{
			if( (int)(random()*100) <= ent->enemy->stealth/10 )
			{
				ent->enemy->lockstatus = 0;
				ent->enemy = NULL;
				ent->think = nolockthink;
				ent->nextthink = level.time + 0.1;
				return;
			}
		}
	}
	if( level.time < ent->enemy->jam_time )//jammer
	{
		if( (int)(random()*100) <= JAMMER_EFFECT/10 )
		{
			ent->enemy->lockstatus = 0;
			ent->enemy = NULL;
			ent->think = nolockthink;
			ent->nextthink = level.time + 0.1;
			return;
		}
	}
	if( !ent->enemy )
	{
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy )	// targetchange ?
	{
		while( (lock = findradius( lock, ent->enemy->s.origin, 300 ) ) != NULL )
		{
			if( ( strcmp( lock->classname, "plane" ) == 0 ||
				strcmp( lock->classname, "helo" ) == 0 ) &&
				lock != ent->owner )
			{
				if( ((int)(random()*100)) < 3 )
				{
					ent->enemy->lockstatus = 0;
					ent->enemy = lock;
					ent->enemy->lockstatus = 2;
				}
			}
			else if( strcmp( lock->classname, "bolt" ) == 0 )
			{
				// lock->ONOFF is used to indicate that this flare was already used
				// and cannot fool our missiles anymore
				if( !lock->ONOFF )
				{
					lock->ONOFF = 1;
					if( (random()*100) < 40 )
					{
						ent->enemy->lockstatus = 0;
						ent->enemy = lock;
						ent->enemy->lockstatus = 2;
					}
				}
			}
		}
	}
	// overturning
	ent->s.angles[0] = anglemod( ent->s.angles[0] );
	ent->s.angles[1] = anglemod( ent->s.angles[1] );
	// and now the tracking
	// first build a direction-vector
	VectorCopy( ent->enemy->s.origin, aimatthis );
//	AngleVectors( ent->enemy->s.angles, forward, NULL, NULL );
	VectorSubtract( aimatthis, ent->s.origin, targetdir );
	// get the distance to the target
	distance = VectorLength( targetdir );
	// if missile is getting farther away but within explosionradius blow it up
	// to inflict at least radius damage!
	if( distance > ent->ideal_yaw && distance < ent->dmg_radius )
	{
		VectorSubtract( ent->owner->s.origin, ent->s.origin, owner_dist );
		if( VectorLength( owner_dist ) > 130 )
		{
			explodeside( ent );
			return;
		}
	}
	VectorClear( ent->avelocity );
	vectoangles( targetdir, aimatthis );

	if( aimatthis[0] < 0 )
		aimatthis[0] += 360;
	if( aimatthis[1] < 0 )
		aimatthis[1] += 360;

	targetdir[0] = aimatthis[0] - ent->s.angles[0];
	if( targetdir[0] > 180 )
		targetdir[0] -= 360;
	else if( targetdir[0] < -180 )
		targetdir[0] += 360;

	targetdir[1] = aimatthis[1] - ent->s.angles[1];
	if( targetdir[1] > 180 )
		targetdir[1] -= 360;
	else if( targetdir[1] < -180 )
		targetdir[1] += 360;

	if( targetdir[0] < -10 )
		ent->avelocity[0] = -ent->turnspeed[0];
//	else if( targetdir[0] < -3 )
//		ent->avelocity[0] = -70;
	else if( targetdir[0] > 10 )
		ent->avelocity[0] = ent->turnspeed[0];
//	else if( targetdir[0] > 3 )
//		ent->avelocity[0] = 70;
	else
		ent->s.angles[0] = aimatthis[0];

	if( targetdir[1] < -10 )
		ent->avelocity[1] = -ent->turnspeed[1];
//	else if( targetdir[1] < -3 )
//		ent->avelocity[1] = -70;
	else if( targetdir[1] > 10 )
		ent->avelocity[1] = ent->turnspeed[1];
//	else if( targetdir[1] > 3 )
//		ent->avelocity[1] = 70;
	else
		ent->s.angles[1] = aimatthis[1];

//	gi.bprintf( PRINT_HIGH, "%.1f %.1f\n", aimatthis[1], ent->s.angles[1] );

	AngleVectors( ent->s.angles, targetdir, NULL, NULL );
	speed = VectorLength( ent->velocity );
	VectorScale( targetdir, speed, ent->velocity );
/*
	// time to target in frames
	timetotarg = (distance/VectorLength( ent->velocity ))/FRAMETIME;
	// if enemy is moving aim ahead
	if( VectorLength(ent->enemy->velocity) > 1 )
	{
		VectorScale( forward, VectorLength(ent->enemy->velocity)*FRAMETIME*timetotarg, forward );
		VectorAdd( aimatthis, forward, aimatthis );
		VectorSubtract( aimatthis, ent->s.origin, targetdir );
	}
	VectorNormalize( targetdir );
	VectorNormalize( ent->movedir );
	VectorScale( targetdir, 0.35, targetdir );
	VectorScale( ent->movedir, 0.65, ent->movedir );
	VectorAdd( targetdir, ent->movedir, targetdir );
	VectorNormalize( targetdir );
	VectorCopy( targetdir, ent->movedir );
	vectoangles( targetdir, ent->s.angles );
	speed = VectorLength( ent->velocity );
	VectorScale( targetdir, speed, ent->velocity );
*/



	if( level.time > ent->wait )
	{
		ent->think = nofuelthink;
	}
	ent->enemy->lockstatus = 2;

	ent->ideal_yaw = distance;

	ent->nextthink = level.time +0.1;

	gi.linkentity( ent );
}

void ATGM_lockthink(edict_t *ent);

void nolockthink(edict_t *ent)
{
	vec3_t 		forward;
	float 		speed;

	if( level.time > ent->wait )
	{
		ent->think = nofuelthink;
	}
	AngleVectors( ent->s.angles, forward, NULL, NULL );
	VectorCopy (forward, ent->movedir);
	vectoangles (forward, ent->s.angles);
	speed = VectorLength( ent->velocity );
	VectorScale (forward, speed, ent->velocity);
	ent->nextthink = level.time + 0.1;

	gi.linkentity( ent );
}

void sidewinder_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

//	if (other == ent->owner)
//		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		// set old cam_target
		if( ent->owner->owner )
		{
			if( !(ent->owner->HASCAN & IS_DRONE) )
			{
				if( ent->owner->owner->client->cameramode == 3
					&& ent->owner->owner->cam_target == ent )
						ent->owner->owner->cam_target = ent->owner->owner->oldenemy;
				if( ent->owner->owner->goalentity == ent )
					ent->owner->owner->goalentity = NULL;
			}
		}
		if( ent->enemy )
		{
			ent->enemy->lockstatus = 0;
			ent->enemy = NULL;
		}
		G_FreeEdict (ent);
		return;
	}


//	gi.bprintf( PRINT_HIGH,"SW-Einschlag\n" );

//	if (ent->owner->owner->client)		// +BD +owner
//		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT); //Achtung +owner ?

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage )
	{
		if( strcmp( other->classname, "plane" ) == 0 ||
			strcmp( other->classname, "helo" ) == 0 )
		{
			if( ent->owner )
			{
				T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin,
			   			 plane->normal, ent->dmg, 0, DAMAGE_NO_PROTECTION, ent->stealth );
			}
			else
			{
				T_Damage (other, ent, NULL, ent->velocity, ent->s.origin,
			   			 plane->normal, ent->dmg, 0, DAMAGE_NO_PROTECTION, ent->stealth);
			}
		}
		else if( strcmp( other->classname, "ground" ) == 0 )
		{
			if( ent->owner )
			{
				T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin,
			   			 plane->normal, ent->dmg/3, 0, DAMAGE_NO_PROTECTION, ent->stealth);
			}
			else
			{
				T_Damage (other, ent, NULL, ent->velocity, ent->s.origin,
			   			 plane->normal, ent->dmg/3, 0, DAMAGE_NO_PROTECTION, ent->stealth);
			}
		}
		else if ( strcmp( other->classname, "func_explosive") == 0 )
		{
			if( ent->owner )
			{
				T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin,plane->normal,
					 ent->dmg/3, 0, DAMAGE_NO_PROTECTION, ent->stealth);
			}
			else
			{
				T_Damage (other, ent, NULL, ent->velocity, ent->s.origin,plane->normal,
					 ent->dmg/3, 0, DAMAGE_NO_PROTECTION, ent->stealth);
			}

		}
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	if( ent->owner )
	{
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, ent->stealth);
	}
	else
	{
		T_RadiusDamage(ent, NULL, ent->radius_dmg, other, ent->dmg_radius, ent->stealth);
	}

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if( ent->enemy )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
	}

	if( ent->owner->owner )
	{
		if( !(ent->owner->HASCAN & IS_DRONE) )
		{
			if( ent->owner->owner->client->cameramode == 3
					&& ent->owner->owner->cam_target == ent )
				ent->owner->owner->cam_target = ent->owner->owner->oldenemy;
			if( ent->owner->owner->goalentity == ent )
					ent->owner->owner->goalentity = NULL;
		}
	}

	G_FreeEdict (ent);
}



void fire_sidewinder(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed,
	float damage_radius, int man, int type)// man = maneuverability
{
	edict_t	*rocket;

	rocket = G_Spawn();
	rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = man;
	VectorCopy (start, rocket->s.origin);
	VectorCopy (start, rocket->s.old_origin);	// +BD
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	if( rockettrail->value )
		rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->owner = self;
	rocket->touch = sidewinder_touch;
	if( self->enemy )
	{
		rocket->enemy = self->enemy;
		rocket->think = lockthink;
		rocket->enemy->lockstatus = 2;
	}
	else
	{
		rocket->think = nolockthink;
	}
	rocket->dmg = damage;
	rocket->radius_dmg = damage;
	rocket->dmg_radius = damage_radius;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "sidewinder";
	rocket->wait = level.time + 6;
	rocket->gravity = 0.1;
	rocket->nextthink = level.time + 0.1;
	rocket->ideal_yaw = 0;			// olddistance to target
	switch( type )
	{
	case 0:
		rocket->stealth = MOD_SIDEWINDER;
		if( self->HASCAN & IS_EASTERN )
			rocket->s.modelindex = gi.modelindex ("models/weapons/aam/archer.md2");
		else
			rocket->s.modelindex = gi.modelindex ("models/weapons/aam/aim9.md2");
		break;
	case 1:
		rocket->stealth = MOD_STINGER;
		rocket->s.modelindex = gi.modelindex ("models/weapons/aam/stinger.md2");
		break;
	case 2:
		rocket->stealth = MOD_AMRAAM;
		if( self->HASCAN & IS_EASTERN )
			rocket->s.modelindex = gi.modelindex ("models/weapons/aam/alamo.md2");
		else
			rocket->s.modelindex = gi.modelindex ("models/weapons/aam/amraam.md2");
		break;
	case 3:
		rocket->stealth = MOD_PHOENIX;
		rocket->s.modelindex = gi.modelindex ("models/weapons/aam/phoenix.md2");
		break;
	}
	rocket->s.skinnum = 1;

	gi.linkentity (rocket);

	if( !(self->HASCAN & IS_DRONE ) )
	{
		if( self->owner->client->cameramode == 3 )
			self->owner->cam_target = rocket;
		self->owner->goalentity = rocket;
	}
	if( weaponinfo->value != 0 )
	{
		char	name[12];
		float	range;

		switch( type )
		{
			case 0:
					strcpy( name, "Sidewinder" );
					range = STANDARD_RANGE * SIDEWINDER_MODIFIER;
					break;
			case 1:
					strcpy( name, "Stinger" );
					range = STANDARD_RANGE * STINGER_MODIFIER;
					break;
			case 2:
					strcpy( name, "AMRAAM" );
					range = STANDARD_RANGE * AMRAAM_MODIFIER;
					break;
			case 3:
					strcpy( name, "Phoenix" );
					range = STANDARD_RANGE * PHOENIX_MODIFIER;
					break;
		}
		gi.cprintf( self->owner, PRINT_HIGH, "Name: %s, Maneuverability: %d, Speed: %d\n",
			name, man, speed );
		gi.cprintf( self->owner, PRINT_HIGH, "Damage: %d, Damage-Radius: %.0f, Lock-Range: %.0f, Endurace: 6 sec\n",
			damage, damage_radius, range );
	}

}


/*
=================
fire_bombs
=================
*/


void bomb_think(edict_t *ent)
{
	vec3_t forward;

	vectoangles( ent->velocity, forward );
	ent->s.angles[0] = forward[0];
	ent->nextthink = level.time + 0.1;

	gi.linkentity( ent );
}


void bomb_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

//	if (other == ent->owner)
//		return;

	if( ent->enemy )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
	}

	if (surf && (surf->flags & SURF_SKY))
	{
		if( ent->owner->owner )
		{
			if( ent->owner->owner->client->cameramode == 3
				&& ent->owner->owner->cam_target == ent )
				ent->owner->owner->cam_target = ent->owner->owner->oldenemy;
			if( ent->owner->owner->goalentity == ent )
				ent->owner->owner->goalentity = NULL;
		}

		G_FreeEdict (ent);
		return;
	}

//	gi.bprintf( PRINT_HIGH,"SW-Einschlag\n" );

//	if (ent->owner->owner->client)		// +BD +owner
//		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT); //Achtung +owner ?

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, DAMAGE_NO_PROTECTION, ent->stealth);
		// +BD owner von other entfernt
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}
	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, ent->stealth);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if( ent->owner->owner )
	{
		if( ent->owner->owner->client->cameramode == 3
			&& ent->owner->owner->cam_target == ent )
			ent->owner->owner->cam_target = ent->owner->owner->oldenemy;
		if( ent->owner->owner->goalentity == ent )
			ent->owner->owner->goalentity = NULL;
	}

	G_FreeEdict (ent);
}



void fire_bombs(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius )
{
	edict_t	*bomb;

	bomb = G_Spawn();
	VectorCopy (start, bomb->s.origin);
	VectorCopy (start, bomb->s.old_origin);	// +BD
	VectorCopy (dir, bomb->movedir);
	vectoangles (dir, bomb->s.angles);
	VectorScale (dir, speed, bomb->velocity);
	bomb->movetype = MOVETYPE_TOSS;
	bomb->clipmask = MASK_SHOT;
	bomb->solid = SOLID_BBOX;
	VectorClear (bomb->mins);
	VectorClear (bomb->maxs);
	bomb->s.modelindex = gi.modelindex ("models/weapons/lgb/tris.md2");
	bomb->owner = self;
	bomb->touch = bomb_touch;
	bomb->dmg = damage;
	bomb->radius_dmg = damage;
	bomb->dmg_radius = damage_radius;
	bomb->s.sound = gi.soundindex ("weapons/rockfly.wav");
	bomb->classname = "bomb";
	bomb->gravity = 0.4;
	bomb->think = bomb_think;
	bomb->nextthink = level.time + 0.1;
	bomb->stealth = MOD_BOMBS;

	gi.linkentity (bomb);

	if( self->owner->client->cameramode == 3 )
		self->owner->cam_target = bomb;
	self->owner->goalentity = bomb;
	if( weaponinfo->value != 0 )
	{
		gi.cprintf( self->owner, PRINT_HIGH, "Name: Iron Bomb, Maneuverability: XX, Speed: XX\n");
		gi.cprintf( self->owner, PRINT_HIGH, "Damage: %d, Damage-Radius: %.0f, Lock-Range: XX, Endurace: XX\n",
			damage, damage_radius );
	}
}

// =======================
// Fire Laser Bombs
// =======================



void LGB_lockthink(edict_t *ent)
{
	vec3_t	targetdir;
	vec3_t  aimatthis;
	vec3_t  owner_dist;
	edict_t *lock = NULL;
	float speed, distance;// time to target in frames

	if( !ent->enemy )
	{
		ent->think = bomb_think;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( strcmp( ent->enemy->classname, "freed" ) == 0 )
	{
		ent->enemy = NULL;
		ent->think = bomb_think;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy->deadflag )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = bomb_think;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !visible( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = bomb_think;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !infront( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = bomb_think;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy )	// targetchange ?
	{
		while( (lock = findradius( lock, ent->enemy->s.origin, 300 ) ) != NULL )
		{
			if( ( strcmp( lock->classname, "ground" ) == 0 ) &&
				lock != ent->owner )
			{
				if( ((int)(random()*100)) < 3 )
				{
					ent->enemy->lockstatus = 0;
					ent->enemy = lock;
					ent->enemy->lockstatus = 2;
				}
			}
			else if( strcmp( lock->classname, "bolt" ) == 0 )
			{
				// lock->ONOFF is used to indicate that this flare was already used
				// and cannot fool our missiles anymore
				if( !lock->ONOFF )
				{
					lock->ONOFF = 1;
					if( (random()*100) < 40 )
					{
						ent->enemy->lockstatus = 0;
						ent->enemy = lock;
						ent->enemy->lockstatus = 2;
					}
				}
			}
		}
	}

	// overturning
	ent->s.angles[0] = anglemod( ent->s.angles[0] );
	ent->s.angles[1] = anglemod( ent->s.angles[1] );
	// and now the tracking
	// first build a direction-vector
	VectorCopy( ent->enemy->s.origin, aimatthis );
	VectorSubtract( aimatthis, ent->s.origin, targetdir );
	// get the distance to the target
	distance = VectorLength( targetdir );
	VectorClear( ent->avelocity );
	vectoangles( targetdir, aimatthis );

	if( aimatthis[0] < 0 )
		aimatthis[0] += 360;
	if( aimatthis[1] < 0 )
		aimatthis[1] += 360;

	targetdir[0] = aimatthis[0] - ent->s.angles[0];
	if( targetdir[0] > 180 )
		targetdir[0] -= 360;
	else if( targetdir[0] < -180 )
		targetdir[0] += 360;

	targetdir[1] = aimatthis[1] - ent->s.angles[1];
	if( targetdir[1] > 180 )
		targetdir[1] -= 360;
	else if( targetdir[1] < -180 )
		targetdir[1] += 360;

	if( targetdir[0] < -8 )
		ent->avelocity[0] = -ent->turnspeed[0];
	else if( targetdir[0] < -3 )
		ent->avelocity[0] = -70;
	else if( targetdir[0] < -1 )
		ent->avelocity[0] = -20;
	else if( targetdir[0] > 8 )
		ent->avelocity[0] = ent->turnspeed[0];
	else if( targetdir[0] > 3 )
		ent->avelocity[0] = 70;
	else if( targetdir[0] > 1 )
		ent->avelocity[0] = 20;
	else
		ent->s.angles[0] = aimatthis[0];

	if( targetdir[1] < -8 )
		ent->avelocity[1] = -ent->turnspeed[1];
	else if( targetdir[1] < -3 )
		ent->avelocity[1] = -70;
	else if( targetdir[1] < -1 )
		ent->avelocity[1] = -20;
	else if( targetdir[1] > 8 )
		ent->avelocity[1] = ent->turnspeed[1];
	else if( targetdir[1] > 3 )
		ent->avelocity[1] = 70;
	else if( targetdir[1] > 1 )
		ent->avelocity[1] = 20;
	else
		ent->s.angles[1] = aimatthis[1];

//	gi.bprintf( PRINT_HIGH, "%.1f %.1f\n", aimatthis[1], ent->s.angles[1] );

	AngleVectors( ent->s.angles, targetdir, NULL, NULL );
	speed = VectorLength( ent->velocity );
	VectorScale( targetdir, speed, ent->velocity );

	ent->enemy->lockstatus = 2;
	if( level.time > ent->wait )
	{
		ent->think = bomb_think;
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
	}
	ent->nextthink = level.time +0.1;

	gi.linkentity( ent );
}


void fire_lgbs(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius)
{
	edict_t	*bomb;

	bomb = G_Spawn();
	VectorCopy (start, bomb->s.origin);
	VectorCopy (start, bomb->s.old_origin);	// +BD
	VectorCopy (dir, bomb->movedir);
	vectoangles (dir, bomb->s.angles);
	VectorScale (dir, speed, bomb->velocity);
	bomb->turnspeed[0] = bomb->turnspeed[1] = bomb->turnspeed[2] = 80;
	bomb->movetype = MOVETYPE_TOSS;
	bomb->clipmask = MASK_SHOT;
	bomb->solid = SOLID_BBOX;
	VectorClear (bomb->mins);
	VectorClear (bomb->maxs);
	bomb->s.modelindex = gi.modelindex ("models/weapons/lgb/tris.md2");
	bomb->owner = self;
	bomb->touch = bomb_touch;
	bomb->dmg = damage;
	bomb->radius_dmg = damage;
	bomb->dmg_radius = damage_radius;
	bomb->s.sound = gi.soundindex ("weapons/rockfly.wav");
	bomb->classname = "bomb";
	bomb->nextthink = level.time + 0.1;
	bomb->wait = level.time + 6;
	bomb->stealth = MOD_LGB;

	if( self->enemy )
	{
		bomb->gravity = 0.6;
		bomb->enemy = self->enemy;
		bomb->think = LGB_lockthink;
		bomb->enemy->lockstatus = 2;
	}
	else
	{
		bomb->gravity = 0.1;
		bomb->think = bomb_think;
	}

	if( self->owner->client->cameramode == 3 )
		self->owner->cam_target = bomb;
	self->owner->goalentity = bomb;
	if( weaponinfo->value != 0 )
	{
		gi.cprintf( self->owner, PRINT_HIGH, "Name: LGB, Maneuverability: %d, Speed: variable\n",
			bomb->turnspeed[0] );
		gi.cprintf( self->owner, PRINT_HIGH, "Damage: %d, Damage-Radius: %.0f, Lock-Range: %.0f, Endurace: 6 sec\n",
			damage, damage_radius, STANDARD_RANGE*LGB_MODIFIER );
	}
	gi.linkentity (bomb);
}



// =======================
// Fire MainGun
// =======================

void shell_think(edict_t *ent)
{
	vec3_t forward;

	vectoangles( ent->velocity, forward );
	ent->s.angles[0] = forward[0];
	ent->nextthink = level.time + 0.1;

	gi.linkentity( ent );
}

void maingun_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		mod;
	vec3_t	origin;

	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		if( self->owner->owner )
		{
			if( self->owner->owner->client->cameramode == 3
				&& self->owner->owner->cam_target == self )
				self->owner->owner->cam_target = self->owner->owner->oldenemy;
			if( self->owner->owner->goalentity == self )
				self->owner->owner->goalentity = NULL;
		}

		G_FreeEdict (self);
		return;
	}

	// calculate position for the explosion entity
	VectorMA (self->s.origin, -0.02, self->velocity, origin);

//	if (self->owner->owner->client)	// +BD +owner
//		PlayerNoise(self->owner, self->s.origin, PNOISE_IMPACT);// Achtung +owner ?

	gi.WriteByte (svc_temp_entity);
	if (self->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (self->s.origin, MULTICAST_PHS);

	if (other->takedamage)
	{
		mod = MOD_MAINGUN;

		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_BULLET, mod);
		// +BD owner von other entfernt
	}

	T_RadiusDamage(self, self->owner, self->radius_dmg, other, self->dmg_radius, MOD_MAINGUN);

	if( self->owner->owner )
	{
		if( self->owner->owner->client->cameramode == 3
			&& self->owner->owner->cam_target == self )
			self->owner->owner->cam_target = self->owner->owner->oldenemy;
		if( self->owner->owner->goalentity == self )
			self->owner->owner->goalentity = NULL;
	}

	G_FreeEdict (self);
}


void fire_maingun (edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, int effect, qboolean hyper)
{
	edict_t	*shell;
//	trace_t	tr;

	VectorNormalize (dir);

	speed += ((damage - 75)*3);

	shell = G_Spawn();
	VectorCopy (start, shell->s.origin);
	VectorCopy (start, shell->s.old_origin);
	vectoangles (dir, shell->s.angles);
	VectorScale (dir, speed, shell->velocity);
	shell->movetype = MOVETYPE_TOSS;
	shell->clipmask = MASK_SHOT;
	shell->solid = SOLID_BBOX;
	shell->s.effects |= effect;
	VectorClear (shell->mins);
	VectorClear (shell->maxs);
	shell->s.modelindex = gi.modelindex ("models/weapons/shell/shell.md2");
	shell->owner = self;
	shell->touch = maingun_touch;
 	shell->nextthink = level.time + 0.1;
	shell->think = shell_think;
	shell->dmg = damage;
	shell->classname = "shell";
	shell->gravity = 0.25;
	shell->radius_dmg = damage*2/3;
	shell->dmg_radius = damage/2;
	gi.linkentity (shell);

//	tr = gi.trace (self->s.origin, NULL, NULL, shell->s.origin, shell, MASK_SHOT);

	if( !(self->owner->HASCAN & IS_DRONE) )
	{
		if( self->owner->client->cameramode == 3 )
			self->owner->cam_target = shell;
		self->owner->goalentity = shell;
	}
	if( weaponinfo->value != 0 )
	{
		gi.cprintf( self->owner, PRINT_HIGH, "Name: Gunshell, Maneuverability: XX, Speed: %d\n",
			speed );
		gi.cprintf( self->owner, PRINT_HIGH, "Damage: %d, Damage-Radius: %.0f, Lock-Range: XX, Endurace: XX\n",
			damage, shell->dmg_radius );
	}
}


// ================================

// ATGM fire

// ================================


void ATGM_lockthink(edict_t *ent)
{
	vec3_t	targetdir;
	vec3_t  aimatthis;
	vec3_t  owner_dist;
	edict_t *lock = NULL;
	float speed, distance;// time to target in frames

	if( strcmp( ent->enemy->classname, "freed" ) == 0 )
	{
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy->deadflag )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !visible( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !infront( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy )	// targetchange ?
	{
		while( (lock = findradius( lock, ent->enemy->s.origin, 300 ) ) != NULL )
		{
			if( ( strcmp( lock->classname, "ground" ) == 0 ) &&
				lock != ent->owner )
			{
				if( ((int)(random()*100)) < 3 )
				{
					ent->enemy->lockstatus = 0;
					ent->enemy = lock;
					ent->enemy->lockstatus = 2;
				}
			}
			else if( strcmp( lock->classname, "bolt" ) == 0 )
			{
				// lock->ONOFF is used to indicate that this flare was already used
				// and cannot fool our missiles anymore
				if( !lock->ONOFF )
				{
					lock->ONOFF = 1;
					if( (random()*100) < 45 )
					{
						ent->enemy->lockstatus = 0;
						ent->enemy = lock;
						ent->enemy->lockstatus = 2;
					}
				}
			}
		}
	}

/*	VectorCopy( ent->enemy->s.origin, aimatthis );
//	AngleVectors( ent->enemy->s.angles, forward, NULL, NULL );
	VectorSubtract( aimatthis, ent->s.origin, targetdir );
	distance = VectorLength( targetdir );
	if( distance > olddist && distance < 130 )
	{
		VectorSubtract( ent->owner->s.origin, ent->s.origin, owner_dist );
		if( VectorLength( owner_dist ) > 130 )
		{
			explodeside( ent );
			return;
		}
	}*/
	// overturning
	ent->s.angles[0] = anglemod( ent->s.angles[0] );
	ent->s.angles[1] = anglemod( ent->s.angles[1] );
	// and now the tracking
	// first build a direction-vector
	VectorCopy( ent->enemy->s.origin, aimatthis );
//	AngleVectors( ent->enemy->s.angles, forward, NULL, NULL );
	VectorSubtract( aimatthis, ent->s.origin, targetdir );
	// get the distance to the target
	distance = VectorLength( targetdir );
	// if missile is getting farther away but within explosionradius blow it up
	// to inflict at least radius damage!
	if( distance > ent->ideal_yaw && distance < 130 )
	{
		VectorSubtract( ent->owner->s.origin, ent->s.origin, owner_dist );
		if( VectorLength( owner_dist ) > 130 )
		{
			explodeside( ent );
			return;
		}
	}
	VectorClear( ent->avelocity );
	vectoangles( targetdir, aimatthis );

	if( aimatthis[0] < 0 )
		aimatthis[0] += 360;
	if( aimatthis[1] < 0 )
		aimatthis[1] += 360;

	targetdir[0] = aimatthis[0] - ent->s.angles[0];
	if( targetdir[0] > 180 )
		targetdir[0] -= 360;
	else if( targetdir[0] < -180 )
		targetdir[0] += 360;

	targetdir[1] = aimatthis[1] - ent->s.angles[1];
	if( targetdir[1] > 180 )
		targetdir[1] -= 360;
	else if( targetdir[1] < -180 )
		targetdir[1] += 360;

	if( targetdir[0] < -8 )
		ent->avelocity[0] = -ent->turnspeed[0];
	else if( targetdir[0] < -3 )
		ent->avelocity[0] = -70;
	else if( targetdir[0] < -1 )
		ent->avelocity[0] = -20;
	else if( targetdir[0] > 8 )
		ent->avelocity[0] = ent->turnspeed[0];
	else if( targetdir[0] > 3 )
		ent->avelocity[0] = 70;
	else if( targetdir[0] > 1 )
		ent->avelocity[0] = 20;
	else
		ent->s.angles[0] = aimatthis[0];

	if( targetdir[1] < -8 )
		ent->avelocity[1] = -ent->turnspeed[1];
	else if( targetdir[1] < -3 )
		ent->avelocity[1] = -70;
	else if( targetdir[1] < -1 )
		ent->avelocity[1] = -20;
	else if( targetdir[1] > 8 )
		ent->avelocity[1] = ent->turnspeed[1];
	else if( targetdir[1] > 3 )
		ent->avelocity[1] = 70;
	else if( targetdir[1] > 1 )
		ent->avelocity[1] = 20;
	else
		ent->s.angles[1] = aimatthis[1];

//	gi.bprintf( PRINT_HIGH, "%.1f %.1f\n", aimatthis[1], ent->s.angles[1] );

	AngleVectors( ent->s.angles, targetdir, NULL, NULL );
	speed = VectorLength( ent->velocity );
	VectorScale( targetdir, speed, ent->velocity );


/*	timetotarg = (distance/VectorLength( ent->velocity ))/FRAMETIME;
	VectorScale( forward, VectorLength(ent->enemy->velocity)*FRAMETIME*timetotarg, forward );
	VectorAdd( aimatthis, forward, aimatthis );
	VectorSubtract( aimatthis, ent->s.origin, targetdir );
	VectorNormalize( targetdir );
	VectorScale( targetdir, 0.3, targetdir );
	VectorAdd( targetdir, ent->movedir, targetdir );
	VectorNormalize( targetdir );
	VectorCopy( targetdir, ent->movedir );
	vectoangles( targetdir, ent->s.angles );
	speed = VectorLength( ent->velocity );
	VectorScale( targetdir, speed, ent->velocity );
*/
	if( level.time > ent->wait )
	{
		ent->think = nofuelthink;
	}
	ent->enemy->lockstatus = 2;

	ent->ideal_yaw = distance;

	ent->nextthink = level.time +0.1;

	gi.linkentity( ent );
}


void ATGM_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			n;

//	if (other == ent->owner)
//		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		// set old cam_target
		if( !(ent->owner->HASCAN & IS_DRONE) )
		{
			if( ent->owner->owner )
			{
				if( ent->owner->owner->client->cameramode == 3
					&& ent->owner->owner->cam_target == ent )
					ent->owner->owner->cam_target = ent->owner->owner->oldenemy;
				if( ent->owner->owner->goalentity == ent )
					ent->owner->owner->goalentity = NULL;
			}
		}
		if( ent->enemy )
		{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
		}
		G_FreeEdict (ent);
		return;
	}

//	gi.bprintf( PRINT_HIGH,"SW-Einschlag\n" );

//	if (ent->owner->owner->client)		// +BD +owner
//		PlayerNoise(ent->owner, ent->s.origin, PNOISE_IMPACT); //Achtung +owner ?

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage  )
	{
		T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal,
				ent->dmg, 0, DAMAGE_NO_PROTECTION, ent->stealth);
		// +BD owner von other entfernt
	}
	else
	{
		// don't throw any debris in net games
		if (!deathmatch->value && !coop->value)
		{
			if ((surf) && !(surf->flags & (SURF_WARP|SURF_TRANS33|SURF_TRANS66|SURF_FLOWING)))
			{
				n = rand() % 5;
				while(n--)
					ThrowDebris (ent, "models/objects/debris2/tris.md2", 2, ent->s.origin);
			}
		}
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, ent->stealth);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if( ent->enemy )
	{
		ent->enemy->lockstatus = 0;
		ent->enemy = NULL;
	}

	if( !(ent->owner->HASCAN & IS_DRONE) )
	{
		if( ent->owner->owner )
		{
			if( ent->owner->owner->client->cameramode == 3
				&& ent->owner->owner->cam_target == ent )
				ent->owner->owner->cam_target = ent->owner->owner->oldenemy;
			if( ent->owner->owner->goalentity == ent )
				ent->owner->owner->goalentity = NULL;
		}
	}

	G_FreeEdict (ent);
}



void fire_ATGM(edict_t *self, vec3_t start, vec3_t dir, int damage, int speed, float damage_radius,
	 int man, int type )
{
	edict_t	*rocket;

	rocket = G_Spawn();
	rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 80;
	VectorCopy (start, rocket->s.origin);
	VectorCopy (start, rocket->s.old_origin);	// +BD
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, speed, rocket->velocity);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	if( rockettrail->value )
		rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->s.modelindex = gi.modelindex ("models/weapons/hellfire/hellfire.md2");
	rocket->owner = self;
	rocket->touch = ATGM_touch;
	if( self->enemy )
	{
		rocket->enemy = self->enemy;
		rocket->think = ATGM_lockthink;
		rocket->enemy->lockstatus = 2;
	}
	else
	{
		rocket->think = nolockthink;
	}
	rocket->dmg = damage;
	rocket->radius_dmg = damage;
	rocket->dmg_radius = 120;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "ATGM";
	rocket->wait = level.time + 6;
	rocket->gravity = 0.1;
	rocket->nextthink = level.time + 0.1;
	rocket->ideal_yaw = 0;

	switch( type )
	{
		case 0:
				rocket->stealth = MOD_ATGM;
				break;
		case 1:
				rocket->stealth = MOD_MAVERICK;
				break;
	}

	gi.linkentity (rocket);

	if( !(self->HASCAN & IS_DRONE ) )
	{
		if( self->owner->client->cameramode == 3 )
			self->owner->cam_target = rocket;
		self->owner->goalentity = rocket;
	}
	if( weaponinfo->value != 0 )
	{
		char	name[12];
		float	range;

		switch( type )
		{
			case 0:
					strcpy( name, "Hellfire" );
					range = STANDARD_RANGE * HELLFIRE_MODIFIER;
					break;
			case 1:
					strcpy( name, "Maverick" );
					range = STANDARD_RANGE * MAVERICK_MODIFIER;
					break;
		}
		gi.cprintf( self->owner, PRINT_HIGH, "Name: %s, Maneuverability: %d, Speed: %d\n",
			name, man, speed );
		gi.cprintf( self->owner, PRINT_HIGH, "Damage: %d, Damage-Radius: %.0f, Lock-Range: %.0f, Endurace: 6 sec\n",
			damage, damage_radius, range );
	}
}

