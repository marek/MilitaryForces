#include "g_local.h"


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
			if (tr.ent->takedamage)
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
fire_round

Fires a single autocannon round. 
=================
*/
void round_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == self->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (self);
		return;
	}

	if (other->takedamage)
	{
		if( strcmp( other->classname, "func_explosive" ) != 0 )
			T_Damage (other, self, self->owner, self->velocity, self->s.origin,
						plane->normal, self->dmg, 1, DAMAGE_BULLET, MOD_AUTOCANNON);
		else
			T_Damage( other, self, self->owner, self->velocity, self->s.origin,
						plane->normal, self->dmg-15, 1, DAMAGE_BULLET, MOD_AUTOCANNON );

//		T_Damage (other, self, self->owner, self->velocity, self->s.origin, plane->normal, self->dmg, 1, DAMAGE_ENERGY, mod);
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_GUNSHOT);
	gi.WritePosition (self->s.origin);
	if (!plane)
		gi.WriteDir (vec3_origin);
	else
		gi.WriteDir (plane->normal);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	G_FreeEdict (self);
}

void fire_round (edict_t *self, vec3_t start, vec3_t dir)
{
	edict_t	*bolt;

	VectorNormalize (dir);

	if( !(self->HASCAN & HASCAN_DRONE ) )
	{
		self->owner->client->machinegun_shots++;
		if( self->owner->client->machinegun_shots > num_tracers->value )
			self->owner->client->machinegun_shots = 0;
	}
	else
	{
		self->noise_index++;
		if( self->noise_index > num_tracers->value )
			self->noise_index = 0;
	}

	bolt = G_Spawn();
	bolt->svflags = SVF_DEADMONSTER;
	VectorCopy (start, bolt->s.origin);
	VectorCopy (start, bolt->s.old_origin);
	vectoangles (dir, bolt->s.angles);
	VectorScale (dir, self->guntype->bullet_speed, bolt->velocity);
	bolt->movetype = MOVETYPE_FLYMISSILE;
	bolt->clipmask = MASK_SHOT;
	bolt->solid = SOLID_BBOX;
	VectorClear (bolt->mins);
	VectorClear (bolt->maxs);
	bolt->s.modelindex = 0;
	if( !(self->HASCAN & HASCAN_DRONE) )
	{
		if( self->owner->client->machinegun_shots == 1 )
			bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	}
	else
	{
		if( self->noise_index == 1 )
			bolt->s.modelindex = gi.modelindex ("models/objects/laser/tris.md2");
	}
	bolt->owner = self;
	bolt->touch = round_touch;
	bolt->nextthink = level.time + 2.5;
	bolt->think = G_FreeEdict;
	bolt->dmg = self->guntype->damage;
	bolt->classname = "bolt";
	bolt->spawnflags = 1;
	gi.linkentity (bolt);
}	



/*
=================
fire_rocket
=================
*/
void rocket_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;

	if (other == ent->owner)
		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		G_FreeEdict (ent);
		return;
	}

	// calculate position for the explosion entity
	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	if (other->takedamage)
	{
		if( strcmp( ent->classname, "rocket" ) == 0 )
			T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_ROCKET);
		if( strcmp( ent->classname, "bomb" ) == 0 )
			T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin, plane->normal, ent->dmg, 0, 0, MOD_BOMBS);
	}

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, MOD_R_SPLASH);

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	G_FreeEdict (ent);
}

void fire_rocket (edict_t *self, vec3_t start, vec3_t dir)
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	VectorScale (dir, 1700, rocket->velocity);
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
	rocket->nextthink = level.time + 8000/1700;
	rocket->think = G_FreeEdict;
	rocket->radius_dmg = rocket->dmg = 45 + (int)(random() * 20.0)-10;;
	rocket->dmg_radius = 100;
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "rocket";

	gi.linkentity (rocket);

	if( !(self->HASCAN & HASCAN_DRONE) )
	{
		if( self->owner->client->cameramode >= CM_MISSILE_FORWARD &&
			self->owner->client->cameramode <= CM_MISSILE_DOWN )
		{
			SetCamTarget( self->owner, rocket );
		}
	}
}



/*
=================
fire_flare
=================
*/


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

void fire_flare (edict_t *self, vec3_t start, vec3_t dir)
{
	edict_t	*bolt;
	int	    speed = 180;

	VectorNormalize (dir);

	bolt = G_Spawn();
	if( strcmp( self->classname, "ground" ) == 0 ||
		strcmp( self->classname, "LQM" ) == 0 )
	{
		speed = 300;
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
	bolt->s.effects |= EF_HYPERBLASTER;
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
	bolt->dmg = 0;
	bolt->classname = "bolt";
	bolt->ONOFF = 0;

	gi.linkentity (bolt);

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


void fire_bombs(edict_t *self, vec3_t start, vec3_t dir )
{
	edict_t	*bomb;

	bomb = G_Spawn();
	VectorCopy (start, bomb->s.origin);
	VectorCopy (start, bomb->s.old_origin);	// +BD
	VectorCopy (dir, bomb->movedir);
	vectoangles (dir, bomb->s.angles);
	VectorScale (dir, self->speed, bomb->velocity);
	bomb->movetype = MOVETYPE_TOSS;
	bomb->clipmask = MASK_SHOT;
	bomb->solid = SOLID_BBOX;
	VectorClear (bomb->mins);
	VectorClear (bomb->maxs);
	bomb->s.modelindex = gi.modelindex ("models/weapons/bomb/tris.md2");
	bomb->owner = self;
	bomb->touch = rocket_touch;
	bomb->radius_dmg = bomb->dmg = 180 + (int)(random() * 60.0)-30;;
	bomb->dmg_radius = 130;
	bomb->s.sound = gi.soundindex ("weapons/rockfly.wav");
	bomb->classname = "bomb";
	bomb->gravity = 0.4;
	bomb->think = bomb_think;
	bomb->nextthink = level.time + 0.1;

	gi.linkentity (bomb);

	if( !(self->HASCAN & HASCAN_DRONE) )
	{
		if( self->owner->client->cameramode >= CM_MISSILE_FORWARD &&
			self->owner->client->cameramode <= CM_MISSILE_DOWN )
		{
			SetCamTarget( self->owner, bomb );
		}
	}
}








/*
=================
fire_AAM
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

void projectile_explode( edict_t *ent )
{
	vec3_t origin;

	ent->s.skinnum = 0;

	VectorMA (ent->s.origin, -0.02, ent->velocity, origin);

	T_RadiusDamage(ent, ent->owner, ent->radius_dmg, NULL, ent->dmg_radius, ent->flares );

	gi.WriteByte (svc_temp_entity);
	if (ent->waterlevel)
		gi.WriteByte (TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_ROCKET_EXPLOSION);
	gi.WritePosition (origin);
	gi.multicast (ent->s.origin, MULTICAST_PHS);

	if( ent->enemy )
	{
		ent->enemy->lockstatus = LS_NOLOCK;
		ent->enemy = NULL;
	}

	G_FreeEdict (ent);
}

void AAM_lockthink(edict_t *ent)
{
	vec3_t	targetdir;
	vec3_t  aimatthis;
	vec3_t  owner_dist;
	edict_t *lock = NULL;
	float speed, distance;// time to target in frames (roughly approximated)

	if( strcmp( ent->enemy->classname, "freed" ) == 0 )
	{
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy->deadflag )
	{
		ent->enemy->lockstatus = LS_NOLOCK;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !visible( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = LS_NOLOCK;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !infront( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = LS_NOLOCK;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy->HASCAN & HASCAN_STEALTH )//stealth
	{
		if( ent->enemy->ONOFF & ONOFF_STEALTH )
		{
			if( (int)(random()*100) <= 20 )
			{
				ent->enemy->lockstatus = LS_NOLOCK;
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
			ent->enemy->lockstatus = LS_NOLOCK;
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
					ent->enemy->lockstatus = LS_NOLOCK;
					ent->enemy = lock;
					ent->enemy->lockstatus = LS_LAUNCH;
				}
			}
			else if( strcmp( lock->classname, "bolt" ) == 0 )
			{
				// lock->ONOFF is used to indicate that this flare was already used
				// and cannot fool our missiles anymore
				if( !lock->ONOFF )
				{
					lock->ONOFF = 1;
					if( (random()*100) < 15 )
					{
						ent->enemy->lockstatus = LS_NOLOCK;
						ent->enemy = lock;
						ent->enemy->lockstatus = LS_LAUNCH;
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
	// if missile is getting farther away but within explosionradius blow it up
	// to inflict at least radius damage!
	if( distance > ent->ideal_yaw && distance < ent->dmg_radius )
	{
		VectorSubtract( ent->owner->s.origin, ent->s.origin, owner_dist );
		if( VectorLength( owner_dist ) > 130 )
		{
			projectile_explode( ent );
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

	if( level.time > ent->wait )
	{
		ent->think = nofuelthink;
	}
	ent->enemy->lockstatus = LS_LAUNCH;

	ent->ideal_yaw = distance;

	ent->nextthink = level.time +0.1;

	gi.linkentity( ent );
}

void ATGM_lockthink(edict_t *ent)
{
	vec3_t	targetdir;
	vec3_t  aimatthis;
	vec3_t  owner_dist;
	edict_t *lock = NULL;
	float speed, distance;// time to target in frames (roughly approximated)

	if( strcmp( ent->enemy->classname, "freed" ) == 0 )
	{
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy->deadflag )
	{
		ent->enemy->lockstatus = LS_NOLOCK;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !visible( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = LS_NOLOCK;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( !infront( ent, ent->enemy ) )
	{
		ent->enemy->lockstatus = LS_NOLOCK;
		ent->enemy = NULL;
		ent->think = nolockthink;
		ent->nextthink = level.time + 0.1;
		return;
	}
	if( ent->enemy->HASCAN & HASCAN_STEALTH )//stealth
	{
		if( ent->enemy->ONOFF & ONOFF_STEALTH )
		{
			if( (int)(random()*100) <= 20 )
			{
				ent->enemy->lockstatus = LS_NOLOCK;
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
			ent->enemy->lockstatus = LS_NOLOCK;
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
			if( strcmp( lock->classname, "ground" ) == 0 &&
				lock != ent->owner ) 
			{
				if( ((int)(random()*100)) < 3 )
				{
					ent->enemy->lockstatus = LS_NOLOCK;
					ent->enemy = lock;
					ent->enemy->lockstatus = LS_LAUNCH;
				}
			}
			else if( strcmp( lock->classname, "bolt" ) == 0 )
			{
				// lock->ONOFF is used to indicate that this flare was already used
				// and cannot fool our missiles anymore
				if( !lock->ONOFF )
				{
					lock->ONOFF = 1;
					if( (random()*100) < 15 )
					{
						ent->enemy->lockstatus = LS_NOLOCK;
						ent->enemy = lock;
						ent->enemy->lockstatus = LS_LAUNCH;
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
	// if missile is getting farther away but within explosionradius blow it up
	// to inflict at least radius damage!
	if( distance > ent->ideal_yaw && distance < ent->dmg_radius )
	{
		VectorSubtract( ent->owner->s.origin, ent->s.origin, owner_dist );
		if( VectorLength( owner_dist ) > 130 )
		{
			projectile_explode( ent );
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

	if( level.time > ent->wait )
	{
		ent->think = nofuelthink;
	}
	ent->enemy->lockstatus = LS_LAUNCH;

	ent->ideal_yaw = distance;

	ent->nextthink = level.time +0.1;

	gi.linkentity( ent );
}

void LGB_lockthink(edict_t *ent)
{
	vec3_t	targetdir;
	vec3_t  aimatthis;
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
					if( (random()*100) < 20 )
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


void projectile_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	vec3_t		origin;
	int			what = ent->flares;	// MOD

//	if (other == ent->owner)
//		return;

	if (surf && (surf->flags & SURF_SKY))
	{
		if( ent->enemy )
		{
			ent->enemy->lockstatus = LS_NOLOCK;
			ent->enemy = NULL;
		}
		G_FreeEdict (ent);
		return;
	}


//	gi.bprintf( PRINT_HIGH,"SW-Einschlag\n" );

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
			   			 plane->normal, ent->dmg, 0, DAMAGE_NO_PROTECTION, what );
			}
			else
			{
				T_Damage (other, ent, NULL, ent->velocity, ent->s.origin,
			   			 plane->normal, ent->dmg, 0, DAMAGE_NO_PROTECTION, what);
			}
		}
		else if( strcmp( other->classname, "ground" ) == 0 )
		{
			if( ent->owner )
			{
				T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin,
			   			 plane->normal, ent->dmg/3, 0, DAMAGE_NO_PROTECTION, what );
			}
			else
			{
				T_Damage (other, ent, NULL, ent->velocity, ent->s.origin,
			   			 plane->normal, ent->dmg/3, 0, DAMAGE_NO_PROTECTION, what );
			}
		}
		else if ( strcmp( other->classname, "func_explosive") == 0 )
		{
			if( ent->owner )
			{
				T_Damage (other, ent, ent->owner, ent->velocity, ent->s.origin,plane->normal,
					 ent->dmg/3, 0, DAMAGE_NO_PROTECTION, what );
			}
			else
			{
				T_Damage (other, ent, NULL, ent->velocity, ent->s.origin,plane->normal,
					 ent->dmg/3, 0, DAMAGE_NO_PROTECTION, what );
			}

		}
	}

	if( ent->owner )
	{
		T_RadiusDamage(ent, ent->owner, ent->radius_dmg, other, ent->dmg_radius, what );
	}
	else
	{
		T_RadiusDamage(ent, NULL, ent->radius_dmg, other, ent->dmg_radius, what );
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
		ent->enemy->lockstatus = LS_NOLOCK;
		ent->enemy = NULL;
	}

	G_FreeEdict (ent);
}

void fire_AAM( edict_t *self, vec3_t start, vec3_t dir )
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (start, rocket->s.old_origin);	// +BD
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	if( rockettrail->value )
		rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->owner = self;

	rocket->touch = projectile_touch;
	if( self->enemy )
	{
		rocket->enemy = self->enemy;
		rocket->think = AAM_lockthink;
		rocket->enemy->lockstatus = LS_LAUNCH;
	}
	else
	{
		rocket->think = nolockthink;
	}
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "sidewinder";
	rocket->wait = level.time + 6;
	rocket->gravity = 0.1;
	rocket->nextthink = level.time + 0.1;
	rocket->ideal_yaw = 0;			// olddistance to target
	rocket->active_weapon = self->weapon[self->active_weapon];

	switch( self->weapon[self->active_weapon] )
	{
	case WEAPON_SIDEWINDER:
		rocket->s.modelindex = gi.modelindex ("models/weapons/aam/aim9.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 100;
		rocket->dmg = rocket->radius_dmg = 60 + (int)(random() * 80.0)-20;
		rocket->dmg_radius = 150;
		VectorScale (dir, 800, rocket->velocity);
		rocket->flares = MOD_SIDEWINDER;
		break;
	case WEAPON_STINGER:
		rocket->s.modelindex = gi.modelindex ("models/weapons/aam/stinger.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 100;
		rocket->dmg = rocket->radius_dmg = 40 + (int)(random() * 50.0)-20;
		rocket->dmg_radius = 150;
		VectorScale (dir, 800, rocket->velocity);
		rocket->flares = MOD_STINGER;
		break;
	case WEAPON_AMRAAM:
		rocket->s.modelindex = gi.modelindex ("models/weapons/aam/amraam.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 50;
		rocket->dmg = rocket->radius_dmg = 70 + (int)(random() * 80.0)-20;
		rocket->dmg_radius = 150;
		VectorScale (dir, 1300, rocket->velocity);
		rocket->flares = MOD_AMRAAM;
		break;
	case WEAPON_PHOENIX:
		rocket->s.modelindex = gi.modelindex ("models/weapons/aam/phoenix.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 30;
		rocket->dmg = rocket->radius_dmg = 100 + (int)(random() * 90.0)-30;
		rocket->dmg_radius = 150;
		VectorScale (dir, 1800, rocket->velocity);
		rocket->flares = MOD_PHOENIX;
		break;
	}
	rocket->s.skinnum = 1;

	gi.linkentity (rocket);

	if( !(self->HASCAN & HASCAN_DRONE) )
	{
		if( self->owner->client->cameramode >= CM_MISSILE_FORWARD &&
			self->owner->client->cameramode <= CM_MISSILE_DOWN )
		{
			SetCamTarget( self->owner, rocket );
		}
	}
}



void fire_ATGM( edict_t *self, vec3_t start, vec3_t dir )
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (start, rocket->s.old_origin);	// +BD
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	rocket->movetype = MOVETYPE_FLYMISSILE;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	if( rockettrail->value )
		rocket->s.effects |= EF_ROCKET;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->owner = self;

	rocket->touch = projectile_touch;
	if( self->enemy )
	{
		rocket->enemy = self->enemy;
		rocket->think = ATGM_lockthink;
		rocket->enemy->lockstatus = LS_LAUNCH;
	}
	else
	{
		rocket->think = nolockthink;
	}
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "ATGM";
	rocket->wait = level.time + 6;
	rocket->gravity = 0.1;
	rocket->nextthink = level.time + 0.1;
	rocket->ideal_yaw = 0;			// olddistance to target
	rocket->active_weapon = self->weapon[self->active_weapon];

	switch( self->weapon[self->active_weapon] )
	{
	case WEAPON_HELLFIRE:
		rocket->s.modelindex = gi.modelindex ("models/weapons/hellfire/hellfire.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 60;
		rocket->dmg = rocket->radius_dmg = 90 + (int)(random() * 40.0)-20;
		rocket->dmg_radius = 120;
		VectorScale (dir, 550, rocket->velocity);
		rocket->flares = MOD_HELLFIRE;
		break;
	case WEAPON_MAVERICK:
		rocket->s.modelindex = gi.modelindex ("models/weapons/aam/maverick.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 60;
		rocket->dmg = rocket->radius_dmg = 140 + (int)(random() * 60.0)-30;
		rocket->dmg_radius = 120;
		VectorScale (dir, 600, rocket->velocity);
		rocket->flares = MOD_MAVERICK;
		break;
	case WEAPON_ANTIRADAR:
		rocket->s.modelindex = gi.modelindex ("models/weapons/aam/maverick.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 60;
		rocket->dmg = rocket->radius_dmg = 80 + (int)(random() * 40.0)-20;
		rocket->dmg_radius = 100;
		VectorScale (dir, 600, rocket->velocity);
		rocket->flares = MOD_ANTIRADAR;
		break;
	}
	rocket->s.skinnum = 1;

	gi.linkentity (rocket);

	if( !(self->HASCAN & HASCAN_DRONE) )
	{
		if( self->owner->client->cameramode >= CM_MISSILE_FORWARD &&
			self->owner->client->cameramode <= CM_MISSILE_DOWN )
		{
			SetCamTarget( self->owner, rocket );
		}
	}
}




void fire_LGB( edict_t *self, vec3_t start, vec3_t dir )
{
	edict_t	*rocket;

	rocket = G_Spawn();
	VectorCopy (start, rocket->s.origin);
	VectorCopy (start, rocket->s.old_origin);	// +BD
	VectorCopy (dir, rocket->movedir);
	vectoangles (dir, rocket->s.angles);
	rocket->movetype = MOVETYPE_TOSS;
	rocket->clipmask = MASK_SHOT;
	rocket->solid = SOLID_BBOX;
	VectorClear (rocket->mins);
	VectorClear (rocket->maxs);
	rocket->owner = self;

	rocket->touch = projectile_touch;
	if( self->enemy )
	{
		rocket->enemy = self->enemy;
		rocket->think = LGB_lockthink;
		rocket->enemy->lockstatus = LS_LAUNCH;
	}
	else
	{
		rocket->think = bomb_think;
	}
	rocket->s.sound = gi.soundindex ("weapons/rockfly.wav");
	rocket->classname = "LGB";
	rocket->wait = level.time + 6;
	rocket->gravity = 0.1;
	rocket->nextthink = level.time + 0.1;
	rocket->ideal_yaw = 0;			// olddistance to target
	rocket->active_weapon = self->weapon[self->active_weapon];

	switch( self->weapon[self->active_weapon] )
	{
	case WEAPON_LASERBOMBS:
		rocket->s.modelindex = gi.modelindex ("models/weapons/lgb/tris.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 50;
		rocket->dmg = rocket->radius_dmg = 280 + (int)(random() * 60.0)-30;
		rocket->dmg_radius = 160;
		VectorScale (dir, self->speed, rocket->velocity);
		rocket->flares = MOD_LASERBOMBS;
		break;
	case WEAPON_RUNWAYBOMBS:
		rocket->s.modelindex = gi.modelindex ("models/weapons/lgb/tris.md2");
		rocket->turnspeed[0] = rocket->turnspeed[1] = rocket->turnspeed[2] = 50;
		rocket->dmg = rocket->radius_dmg = 180 + (int)(random() * 60.0)-30;
		rocket->dmg_radius = 120;
		VectorScale (dir, self->speed, rocket->velocity);
		rocket->flares = MOD_RUNWAYBOMBS;
		break;
	}
	rocket->s.skinnum = 1;

	gi.linkentity (rocket);

	if( !(self->HASCAN & HASCAN_DRONE) )
	{
		if( self->owner->client->cameramode >= CM_MISSILE_FORWARD &&
			self->owner->client->cameramode <= CM_MISSILE_DOWN )
		{
			SetCamTarget( self->owner, rocket );
		}
	}
}


