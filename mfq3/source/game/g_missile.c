/*
 * $Id: g_missile.c,v 1.34 2005-08-19 00:09:36 minkis Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"

#define	MISSILE_PRESTEP_TIME	50

/*
================
G_BounceMissile

================
*/
void G_BounceMissile( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
		VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );
		// check for stop
		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) {
			G_SetOrigin( ent, trace->endpos );
			return;
		}
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
void G_ExplodeMissile( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;


	// update target
	if( ent->tracktarget && ent->tracktarget->client ) {
		ent->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;
	}

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ), qtrue );

	ent->freeAfterEvent = qtrue;

	// splash damage
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
			, ent->splashMethodOfDeath, ent->targetcat ) ) {
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

	trap_LinkEntity( ent );
}




/*
================
G_ExplodeFlak

Explode a flak at destination
================
*/
void G_ExplodeFlak( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;


	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_FLAK, DirToByte( dir ), qtrue );

	ent->freeAfterEvent = qtrue;

	// splash damage
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
			, ent->splashMethodOfDeath, ent->targetcat ) ) {
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

	trap_LinkEntity( ent );
}


/*
================
G_ExplodeCFlare
spawns other flares
================
*/
void G_ExplodeCFlare( gentity_t *ent ) {
	vec3_t		dir;
	vec3_t		origin;
	static int	seed = 0x92;

	// update target
	if( ent->tracktarget && ent->tracktarget->client ) {
		ent->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;
	}

	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	SnapVector( origin );
	G_SetOrigin( ent, origin );

	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;

	ent->s.eType = ET_GENERAL;
	G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( dir ), qtrue );

	ent->freeAfterEvent = qtrue;

	// splash damage
	if ( ent->splashDamage ) {
		if( G_RadiusDamage( ent->r.currentOrigin, ent->parent, ent->splashDamage, ent->splashRadius, ent
			, ent->splashMethodOfDeath, ent->targetcat ) ) {
			g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}
	
	// Launch flares
	VectorSet(dir, 70 * Q_random(&seed),	70 * Q_random(&seed),		80 * Q_random(&seed));
	fire_flare2(ent->parent, ent->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );
	VectorSet(dir, -70  * Q_random(&seed),	70 * Q_random(&seed),		80 * Q_random(&seed));
	fire_flare2(ent->parent, ent->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );
	VectorSet(dir, 70 * Q_random(&seed),	-70 * Q_random(&seed),		80 * Q_random(&seed));
	fire_flare2(ent->parent, ent->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );
	VectorSet(dir, -70 * Q_random(&seed),	-70  * Q_random(&seed),		80 * Q_random(&seed));
	fire_flare2(ent->parent, ent->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );
	VectorSet(dir, 0 + 5 * Q_random(&seed),	0 + 5 * Q_random(&seed),	120 * Q_random(&seed));
	fire_flare2(ent->parent, ent->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );

	trap_LinkEntity( ent );
}

/*
===============
NukeRadiusDamage
===============
*/
static void NukeRadiusDamage( vec3_t origin, gentity_t *attacker, float damage, float radius ) {
	float		dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	trace_t		trace, trace2;
	vec3_t		temp;

	if ( radius < 1 ) {
		radius = 1;
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (!ent->takedamage) {
			continue;
		}

		// dont hit things we have already hit
		if( ent->nukeTime > level.time ) {
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}
		
		// Do trace only for players only, save cpu
		if(strcmp(ent->classname, "player") == 0)
		{
			// Try a trace
			// trap_Trace (&trace, origin, ent->r.absmin, ent->r.absmax,  ent->r.currentOrigin, ENTITYNUM_NONE, MASK_SOLID);
			trap_Trace (&trace, origin, NULL, NULL,  ent->r.currentOrigin, ent->s.number, MASK_SOLID);
		
			// Also don't let them get away to easily so check slightly above the origin, and see if it reaches) It may have to be a big hill.
			VectorCopy(origin, temp);
			temp[2] += radius / 2;
			trap_Trace (&trace2, temp, NULL, NULL, ent->r.currentOrigin, ent->s.number, MASK_SOLID);
		}
		else
		{
			trace.fraction = trace2.fraction = 1.0;
		}
		
		// If it didn't hit anything, do damage, cause nothings in the way, but only players get off so easily >:D
		if(trace.fraction == 1.0 || trace2.fraction == 1.0)
		{
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			//G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_NUKE, CAT_ANY );
			
			G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS, MOD_NUKE, CAT_ANY );
			ent->nukeTime = level.time + 3000;
		}
	}
}

/*
===============
NukeShockWave
===============
*/
static void NukeShockWave( vec3_t origin, gentity_t *attacker, float damage, float push, float radius ) {
	float		dist;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;

	if ( radius < 1 )
		radius = 1;

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		// dont hit things we have already hit
		if( ent->nukeShockTime > level.time ) {
			continue;
		}

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) {
			if ( origin[i] < ent->r.absmin[i] ) {
				v[i] = ent->r.absmin[i] - origin[i];
			} else if ( origin[i] > ent->r.absmax[i] ) {
				v[i] = origin[i] - ent->r.absmax[i];
			} else {
				v[i] = 0;
			}
		}

		dist = VectorLength( v );
		if ( dist >= radius ) {
			continue;
		}

//		if( CanDamage (ent, origin) ) {
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			dir[2] += 24;
		//	G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_NUKE, CAT_ANY );
			G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS, MOD_NUKE, CAT_ANY );
			//
			dir[2] = 0;
			VectorNormalize(dir);
			if ( ent->client ) {
				ent->client->ps.velocity[0] = dir[0] * push;
				ent->client->ps.velocity[1] = dir[1] * push;
				ent->client->ps.velocity[2] = 100;
			}
			ent->nukeShockTime = level.time + 3000;
//		}
	}
}

/*
===============
NukeDamage
===============
*/
static void NukeDamage( gentity_t *self ) {
	int i;
	float t;
	gentity_t *ent;
	vec3_t newangles;

	self->count += 100;

	if (self->count >= NUKE_SHOCKWAVE_STARTTIME) {
		// shockwave push back
		t = self->count - NUKE_SHOCKWAVE_STARTTIME;
		NukeShockWave(self->s.pos.trBase, self->activator, 25, 50,	(int) (float) t * self->splashRadius / (NUKE_SHOCKWAVE_ENDTIME - NUKE_SHOCKWAVE_STARTTIME) );
	}
	//
	if (self->count >= NUKE_EXPLODE_STARTTIME) {
		// do our damage
		t = self->count - NUKE_EXPLODE_STARTTIME;
		NukeRadiusDamage( self->s.pos.trBase, self->activator, 700,	(int) (float) t * self->splashRadius / (NUKE_IMPLODE_STARTTIME - NUKE_EXPLODE_STARTTIME) );
	}

	// either cycle or kill self
	if( self->count >= NUKE_SHOCKWAVE_ENDTIME ) {
		G_FreeEntity( self );
		return;
	}
	self->nextthink = level.time + 100;
	
	// add earth quake effect
	newangles[0] = crandom() * 2;
	newangles[1] = crandom() * 2;
	newangles[2] = 0;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		ent = &g_entities[i];
		if (!ent->inuse)
			continue;
		if (!ent->client)
			continue;

		if (ent->client->ps.groundEntityNum != ENTITYNUM_NONE) {
			ent->client->ps.velocity[0] += crandom() * 120;
			ent->client->ps.velocity[1] += crandom() * 120;
			ent->client->ps.velocity[2] = 30 + random() * 25;
		}

		ent->client->ps.delta_angles[0] += ANGLE2SHORT(newangles[0] - self->movedir[0]);
		ent->client->ps.delta_angles[1] += ANGLE2SHORT(newangles[1] - self->movedir[1]);
		ent->client->ps.delta_angles[2] += ANGLE2SHORT(newangles[2] - self->movedir[2]);

	}
	VectorCopy(newangles, self->movedir);
}

/*
================
G_burningManExplode
================
*/
void G_burningManExplode( gentity_t *ent ) {
	vec3_t		start, dir;
	int i;
	gentity_t	*bolt;
	static int	seed = 0x92;


	VectorCopy(ent->r.currentOrigin, start);
	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;


	for(i = 0; i < 75; i++)
	{
		VectorSet(dir, (30 - (-30) + 1) * Q_random(&seed) + -30,	(30 - (-30) + 1) * Q_random(&seed) + -30,		40 * Q_random(&seed));
	
		bolt = G_Spawn();
		bolt->classname = "burningman";
		bolt->nextthink = level.time + 10000 + 4000 * Q_random(&seed);
		bolt->think = G_ExplodeMissile;
		bolt->s.eType = ET_MISSILE;
		bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
		bolt->s.weaponIndex = WI_BURNINGMAN;
		
		// bolt->parent = self;	// No parent
	//	bolt->r.ownerNum = self->s.number;

		bolt->damage = bolt->splashDamage = 0;
		bolt->splashRadius = 0;
		bolt->clipmask = MASK_SHOT;
		bolt->ONOFF = 1;// not used

		bolt->s.pos.trType = TR_GRAVITY_10;
		bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
		VectorCopy( start, bolt->s.pos.trBase );
		VectorScale( dir, 15, bolt->s.pos.trDelta );
		SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
		VectorCopy (start, bolt->r.currentOrigin);
	}
}


/*
================
G_CrateDropItems
================
*/
void G_CrateDropItems(gentity_t *ent)
{
	gitem_t		*item;
	qboolean	found = qfalse;
	char		*classname[5];
	int			items,i;

	switch(availableWeapons[ent->s.weaponIndex].type)
	{
		case WT_FUELCRATE:
			switch(availableWeapons[ent->s.weaponIndex].damage)
			{
				case 1:
				default:
					classname[0] = "Some Fuel";
					break;
				case 2:
					classname[0] = "More Fuel";
					break;
			}
			items = 1;
			break;
		case WT_AMMOCRATE:
			classname[0] = "Shells";
			classname[1] = "Bullets";
			classname[2] = "Rockets";
			classname[3] = "Slugs";
			items = 4;
			break;
		case WT_HEALTHCRATE:
			switch(availableWeapons[ent->s.weaponIndex].damage)
			{
				case 5:
				default:
					classname[0] = "5 Health";
					break;
				case 25:
					classname[0] = "25 Health";
					break;
				case 50:
					classname[0] = "50 Health";
					break;
			}
			items = 1;
			break;
	}

	for(i = 0; i < items; i++)
	{
		found = qfalse;
		// find the item 
		for ( item = bg_itemlist + 1; item->classname; item++) {
			if ( strcmp(item->pickup_name,classname[i]) == 0  )  {
				found = qtrue;
				break;
			}
		}
		// spawn the item
		if(found) 
		{
			ent->s.pos.trBase[2] += 25;
			Drop_Item( ent, item, 180*sin(crandom()));
			ent->s.pos.trBase[2] -= 25;

		}
	}



}

/*
================
G_ExplodeNuke
================
*/
void G_ExplodeNuke( gentity_t *ent ) {
	gentity_t	*explosion;
	gentity_t	*te;
	vec3_t		snapped;
	static int	seed = 0x94;
	int rndnum;



	// start up the explosion logic
	explosion = G_Spawn();

	explosion->s.eType = ET_EVENTS + EV_NUKE;
	explosion->eventTime = level.time;

	VectorCopy( ent->r.currentOrigin, snapped );

	SnapVector( snapped );		// save network bandwidth
	G_SetOrigin( explosion, snapped );

	explosion->classname = "nuke";
	explosion->s.pos.trType = TR_STATIONARY;

	explosion->nukeTime = level.time;

	explosion->think = NukeDamage;
	explosion->nextthink = level.time + 100;
	explosion->count = 0;
	explosion->s.weaponIndex = ent->s.weaponIndex;
	explosion->splashRadius = ent->splashRadius;

	VectorClear(explosion->movedir);

	trap_LinkEntity( explosion );

	explosion->activator = ent->parent;		// Activator is the owner of the bomb/missile 

	ent->freeAfterEvent = qtrue;
	

	// Easter egg
	rndnum = (35 - (1) + 1) * Q_random(&seed) + 1;
	if(rndnum == 2)	// Magic number supplied by Cannon ¬_¬
	{
		G_burningManExplode(ent);
	}


	// play global sound at all clients
	te = G_TempEntity(snapped, EV_GLOBAL_TEAM_SOUND );
	te->r.svFlags |= SVF_BROADCAST;
	te->s.eventParm = GTS_NUKE;

}


/*
================
G_MissileImpact
================
*/
void G_MissileImpact( gentity_t *ent, trace_t *trace ) {
	gentity_t		*other;
	qboolean		hitClient = qfalse;

	other = &g_entities[trace->entityNum];

	// check for bounce
//	if ( !other->takedamage &&
//		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
//		G_BounceMissile( ent, trace );
//		return;
//	}

	// update target
	if( ent->tracktarget && ent->tracktarget->client ) {
		ent->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;
	}

	// impact damage
	if (other->takedamage) {
		// FIXME: wrong damage direction?
		if ( ent->damage ) {
			vec3_t	velocity;
			int		actualdamage;

			if( LogAccuracyHit( other, &g_entities[ent->r.ownerNum] ) ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = qtrue;
			}
			BG_EvaluateTrajectoryDelta( &ent->s.pos, level.time, velocity );
			if ( VectorLength( velocity ) == 0 ) {
				velocity[2] = 1;
			}
			actualdamage = ent->damage + ((rand() % (ent->damage/2)) - (ent->damage/4));
			G_Damage (other, ent, &g_entities[ent->r.ownerNum], velocity,
				ent->s.origin, actualdamage, 
				0, ent->methodOfDeath, ent->targetcat);
		}
	}

	// is it cheaper in bandwidth to just remove this ent and create a new
	// one, rather than changing the missile into the explosion?

	if (availableWeapons[ent->s.weaponIndex].type != WT_FUELCRATE && 
			availableWeapons[ent->s.weaponIndex].type != WT_AMMOCRATE && 
			availableWeapons[ent->s.weaponIndex].type != WT_HEALTHCRATE) 
	{
		if ( other->takedamage && other->client ) {
			G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ), qtrue );
			ent->s.otherEntityNum = other->s.number;
		} else if( trace->surfaceFlags & SURF_METALSTEPS ) {
			G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ), qtrue );
		} else {
			G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ), qtrue );
		}
	}

	ent->freeAfterEvent = qtrue;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	// Nuke specifics to execute its detonation anyways
	if(availableWeapons[ent->s.weaponIndex].type == WT_NUKEBOMB || availableWeapons[ent->s.weaponIndex].type == WT_NUKEMISSILE)
	{
		if (!ent->think)
			G_Error ( "NULL ent->think");
		else
			ent->think (ent);
	}
	else if (availableWeapons[ent->s.weaponIndex].type == WT_FUELCRATE || 
		availableWeapons[ent->s.weaponIndex].type == WT_AMMOCRATE || 
		availableWeapons[ent->s.weaponIndex].type == WT_HEALTHCRATE)
	{
		char * classname = g_entities[trace->entityNum].classname;
		if( strcmp( classname, "func_runway" ) == 0 ||
			strcmp( classname, "func_plat" ) == 0 ||
			strcmp( classname, "func_train" ) == 0 ||
			strcmp( classname, "func_door" ) == 0 ||
			strcmp( classname, "worldspawn" ) == 0) {
			G_CrateDropItems(ent);
		}
	}
	// splash damage (doesn't apply to person directly hit)
	else if ( ent->splashDamage ) {
		if( G_RadiusDamage( trace->endpos, ent->parent, ent->splashDamage, ent->splashRadius, 
			other, ent->splashMethodOfDeath, ent->targetcat ) ) {
			if( !hitClient ) {
				g_entities[ent->r.ownerNum].client->accuracy_hits++;
			}
		}
	}

	trap_LinkEntity( ent );
}

/*
================
G_RunMissile
================
*/
void G_RunMissile( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			passent;

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// if this missile bounced off an invulnerability sphere
	if ( ent->target_ent ) {
		passent = ent->target_ent->s.number;
	}
	else {
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;
	}
	// trace a line from the previous position to the current position
	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask );

	if ( tr.startsolid || tr.allsolid ) {
		// make sure the tr.entityNum is set to the entity we're stuck in
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask );
		tr.fraction = 0;
	}
	else {
		VectorCopy( tr.endpos, ent->r.currentOrigin );
	}

	trap_LinkEntity( ent );

	if ( tr.fraction != 1) {
		// never explode or bounce on sky
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			G_FreeEntity( ent );
			return;
		}

		G_MissileImpact( ent, &tr );

		if ( ent->s.eType != ET_MISSILE &&
			 ent->s.eType != ET_BULLET) {
			return;		// exploded
		}
	}

	// check think function after bouncing
	G_RunThink( ent );
}

/*
================
G_CrateThink
================
*/
void G_CrateThink( gentity_t *ent ) {
	vec3_t angles, pos, dir;
	int i;

	// Get values
	VectorCopy(ent->r.currentOrigin, pos);

	// Set angles
	for(i = 0; i < 3; i++)
	{
		angles[i] = 45*sin((pos[2]/25)+(i*10));
	}

	// Create forces based on angles
	VectorCopy(angles,  dir);
	AngleVectors( dir, dir, NULL, NULL );
	VectorScale( dir, (float)ent->speed/5, dir );
	dir[2] -= ent->speed;
		
	// Save angles
	VectorCopy( ent->r.currentAngles, ent->s.apos.trBase);
	VectorCopy( angles, ent->s.angles );
	VectorCopy( ent->s.angles, ent->s.apos.trDelta );
	VectorCopy( ent->s.angles, ent->r.currentAngles );
	ent->s.apos.trTime = level.time;

	// Save Origin
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	VectorCopy( dir, ent->s.pos.trDelta );
	ent->s.pos.trTime = level.time;

	// Set next think
	ent->nextthink = level.time + 5;
	ent->lastDist = VectorNormalize(dir);

	trap_LinkEntity( ent );
}

//=============================================================================

static void no_fuel_flight( gentity_t *missile ) {
	// update target
	if( missile->tracktarget && missile->tracktarget->client ) {
		missile->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;
	}
	missile->s.pos.trType = TR_GRAVITY;
	missile->think = G_ExplodeMissile;
	missile->nextthink = level.time + 5000;
}

static void on_target_lost( gentity_t *missile ) {
	// update target
	if( missile->tracktarget && missile->tracktarget->client ) {
		missile->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;
	}
	missile->tracktarget = 0;
	missile->think = no_fuel_flight;
	missile->nextthink = missile->wait + 50;
}

/*
================
follow your target
================
*/
static void follow_target( gentity_t *missile ) {
	vec3_t	dir, targdir, mid, mins, maxs;
	float	dist, dot;
	trace_t	tr;
	int		i, prob, num, touch[MAX_GENTITIES], vulner;
	static vec3_t range = { 500, 500, 500 };
	gentity_t* hit;

	// target invalid
	if( !missile->tracktarget ) {
		on_target_lost(missile);
		return;
	} else if( !missile->tracktarget->inuse ) {
		on_target_lost(missile);
		return;
	}

	// out of fuel
	if( level.time >= missile->wait ) {
		on_target_lost(missile);
		return;
	}

	// direction vector and range
	if( missile->tracktarget->s.eType == ET_EXPLOSIVE ) {
		VectorAdd( missile->tracktarget->r.absmin, missile->tracktarget->r.absmax, mid );
		VectorScale( mid, 0.5f, mid );
	} else {
		VectorCopy( missile->tracktarget->r.currentOrigin, mid );
	}
	VectorSubtract( mid, missile->r.currentOrigin, targdir );
	dist = VectorNormalize(dir);

	// out of range (if ever possible)
	if( dist > missile->range ) {
		on_target_lost(missile);
		return;
	}

	// check for flares
	VectorSubtract( missile->r.currentOrigin, range, mins );
	VectorAdd( missile->r.currentOrigin, range, maxs );
	num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

	for ( i=0 ; i<num ; i++ ) {
		hit = &g_entities[touch[i]];
		if( !hit->inuse ) continue;
		if( hit->s.eType != ET_MISSILE ) continue;
		if( strcmp( hit->classname, "flare" ) != 0 ) continue;
		if( !hit->ONOFF ) continue;
		if( hit->r.ownerNum == missile->r.ownerNum ) continue; 
		hit->ONOFF = 0; // disable the flare
		prob = rand() % 100;
		vulner = missile->basicECMVulnerability - missile->tracktarget->s.frame;
		//G_Printf("TT: %d, vulner: %d, prob: %d\n", missile->tracktarget->s.frame, vulner, prob);
		if( prob < vulner ) {
			if( missile->tracktarget->client )
				missile->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;
			missile->tracktarget = hit;
			missile->lastDist = 0;
			missile->nextthink = level.time + 10;
			return;
		}
	}

	// close miss
	if( missile->lastDist && dist > missile->lastDist && dist < missile->splashRadius ) {
		missile->nextthink = level.time + 10;
		missile->think = G_ExplodeMissile;
	}

	// out of seeker cone
	VectorCopy( missile->s.pos.trDelta, dir );
	VectorNormalize( dir );
	dot = DotProduct( dir, targdir );
	if( dot < missile->followcone ) { // roughly 5 degrees
		on_target_lost(missile);
		return;
	}

	// LOS
	trap_Trace( &tr, missile->r.currentOrigin, 0, 0, mid, missile->tracktarget->s.number, MASK_SOLID );
	if( tr.fraction < 1.0f ) {
		on_target_lost(missile);
		return;
	}

	// adjust course/speed
	VectorMA( dir, 0.05f, targdir, targdir );
	VectorNormalize( targdir );
	VectorScale( targdir, missile->speed, dir );
	
	vectoangles( dir, targdir );
	VectorCopy( targdir, missile->s.angles );
	VectorCopy( missile->s.angles, missile->s.apos.trBase );
	VectorCopy( missile->s.angles, missile->r.currentAngles );
	missile->s.apos.trTime = level.time;

	VectorCopy( missile->r.currentOrigin, missile->s.pos.trBase );
	VectorCopy( dir, missile->s.pos.trDelta );
	missile->s.pos.trTime = level.time;

	missile->nextthink = level.time + 100;
	missile->lastDist = dist;

	// update target
	if( missile->tracktarget->client ) {
		missile->tracktarget->client->ps.stats[STAT_LOCKINFO] |= LI_BEING_LAUNCHED;
	}


	trap_LinkEntity( missile );
}



static void drop_to_normal_transition( gentity_t* missile ) {
	missile->s.pos.trType = TR_ACCEL;//TR_LINEAR;
	missile->s.pos.trTime = level.time;
	missile->s.pos.trDuration = 500;

	VectorCopy( missile->r.currentOrigin, missile->s.pos.trBase );
	VectorNormalize( missile->s.pos.trDelta );
	VectorScale( missile->s.pos.trDelta, missile->speed, missile->s.pos.trDelta );
	if( missile->tracktarget ) {
		missile->wait = level.time + availableWeapons[missile->s.weaponIndex].fuelrange;
		missile->think = follow_target;
		missile->nextthink = level.time + 50;
	} else {
		missile->think = G_ExplodeMissile;
		missile->nextthink = level.time + availableWeapons[missile->s.weaponIndex].fuelrange;
	}
}


//=============================================================================

/*
=================
fire_antiair MFQ3
=================
*/
void fire_antiair (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		dir, right, up, temp, forward;
	vec3_t		start, offset;
	qboolean	active = ( self->client->ps.stats[STAT_LOCKINFO] & LI_LOCKING );
	qboolean	wingtip = qfalse;

	VectorCopy( self->s.pos.trBase, start );

	if( (availableVehicles[self->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[self->client->vehicle].cat & CAT_BOAT) ) {
		VectorCopy( availableVehicles[self->client->vehicle].gunoffset, offset );
		AngleVectors( self->client->ps.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ((float)self->client->ps.turretAngle)/10 );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ((float)self->client->ps.gunAngle)/10 );
		wingtip = qtrue; // dont drop
	} else {
		self->left = (self->left ? qfalse : qtrue);
		MF_removeWeaponFromLoadout(self->client->ps.weaponIndex, &self->loadout, &wingtip, offset, 0 );
		AngleVectors( self->client->ps.vehicleAngles, dir, right, up );
		VectorInverse( right );
	}
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "aam";
	if( active ) {
		bolt->tracktarget = self->tracktarget;
		// update target
		if( self->tracktarget->client ) {
			self->tracktarget->client->ps.stats[STAT_LOCKINFO] |= LI_BEING_LAUNCHED;
		}
		bolt->followcone = availableWeapons[self->client->ps.weaponIndex].followcone;
	}
	bolt->speed = availableWeapons[self->client->ps.weaponIndex].muzzleVelocity;
	if( wingtip ) {
		if( active ) {
			bolt->wait = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
			bolt->think = follow_target;
			bolt->nextthink = level.time + 50;
		} else {
			bolt->think = G_ExplodeMissile;
			bolt->nextthink = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
		}
		bolt->s.pos.trType = TR_LINEAR;
		VectorScale( dir, availableWeapons[self->client->ps.weaponIndex].muzzleVelocity, bolt->s.pos.trDelta );
		/*
		bolt->s.pos.trType = TR_ACCEL;
		bolt->s.pos.trDuration = availableWeapons[self->client->ps.weaponIndex].muzzleVelocity;
		VectorScale( dir, max(10,self->client->ps.speed/10), bolt->s.pos.trDelta );
		*/
	} else {
		bolt->think = drop_to_normal_transition;
		bolt->nextthink = level.time + 400;
		bolt->s.pos.trType = TR_GRAVITY;
		VectorScale( dir, self->client->ps.speed/10, bolt->s.pos.trDelta );
	}
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = bolt->splashDamage = availableWeapons[self->client->ps.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[self->client->ps.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[self->client->ps.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->range = availableWeapons[self->client->ps.weaponIndex].range;
	bolt->methodOfDeath = MOD_FFAR;
	bolt->splashMethodOfDeath = MOD_FFAR_SPLASH;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;
	bolt->basicECMVulnerability = availableWeapons[self->client->ps.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_antiground MFQ3
=================
*/
void fire_antiground (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		dir, right, up;
	vec3_t		start, offset, forward, temp;
	qboolean	active = ( self->client->ps.stats[STAT_LOCKINFO] & LI_LOCKING );
	qboolean	wingtip = qfalse;

	VectorCopy( self->s.pos.trBase, start );

	if( (availableVehicles[self->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[self->client->vehicle].cat & CAT_BOAT) ) {
		VectorCopy( availableVehicles[self->client->vehicle].gunoffset, offset );
		AngleVectors( self->client->ps.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ((float)self->client->ps.turretAngle)/10 );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ((float)self->client->ps.gunAngle)/10 );
		wingtip = qtrue; // dont drop
	} else {
		self->left = (self->left ? qfalse : qtrue);
		MF_removeWeaponFromLoadout(self->client->ps.weaponIndex, &self->loadout, 0, offset, 0 );
		AngleVectors( self->client->ps.vehicleAngles, dir, right, up );
		VectorInverse( right );
	}
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "agm";

	if( active ) {
//		bolt->wait = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
//		bolt->think = follow_target;
//		bolt->nextthink = level.time + 50;
		bolt->tracktarget = self->tracktarget;
		// update target
		if( self->tracktarget->client ) {
			self->tracktarget->client->ps.stats[STAT_LOCKINFO] |= LI_BEING_LAUNCHED;
		}
		bolt->followcone = availableWeapons[self->client->ps.weaponIndex].followcone;
//	} else {
//		bolt->think = G_ExplodeMissile;
//		bolt->nextthink = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
	}

	bolt->speed = availableWeapons[self->client->ps.weaponIndex].muzzleVelocity;
	if( wingtip ) {
		if( active ) {
			bolt->wait = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
			bolt->think = follow_target;
			bolt->nextthink = level.time + 50;
		} else {
			bolt->think = G_ExplodeMissile;
			bolt->nextthink = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
		}
		bolt->s.pos.trType = TR_LINEAR;
		VectorScale( dir, bolt->speed, bolt->s.pos.trDelta );
	} else {
		bolt->think = drop_to_normal_transition;
		bolt->nextthink = level.time + 400;
		bolt->s.pos.trType = TR_GRAVITY;
		VectorScale( dir, self->client->ps.speed/10, bolt->s.pos.trDelta );
	}


	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = bolt->splashDamage = availableWeapons[self->client->ps.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[self->client->ps.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[self->client->ps.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->range = availableWeapons[self->client->ps.weaponIndex].range;
	bolt->methodOfDeath = MOD_FFAR;
	bolt->splashMethodOfDeath = MOD_FFAR_SPLASH;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;
	bolt->basicECMVulnerability = availableWeapons[self->client->ps.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_ffar MFQ3
=================
*/
void fire_ffar (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		dir, right, up;
	vec3_t		start, offset;

	VectorCopy( self->s.pos.trBase, start );

	self->left++;
	if( self->left >= availableVehicles[self->client->vehicle].ammo[self->client->ps.weaponNum] ) {
		self->left = 0;
	}
	MF_removeWeaponFromLoadout(self->client->ps.weaponIndex, &self->loadout, 0, offset, self->left );
	AngleVectors( self->client->ps.vehicleAngles, dir, right, up );
	VectorInverse( right );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "ffar";
	bolt->nextthink = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = bolt->splashDamage = availableWeapons[self->client->ps.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[self->client->ps.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[self->client->ps.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->methodOfDeath = MOD_FFAR;
	bolt->splashMethodOfDeath = MOD_FFAR_SPLASH;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, availableWeapons[self->client->ps.weaponIndex].muzzleVelocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
fire_ironbomb MFQ3
=================
*/
void fire_ironbomb (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		dir, right, up, offset;
	vec3_t		start;

//	self->left = (self->left ? qfalse : qtrue);
	MF_removeWeaponFromLoadout(self->client->ps.weaponIndex, &self->loadout, 0, offset, 0 );
	AngleVectors( self->client->ps.vehicleAngles, dir, right, up );
	VectorInverse( right );
	VectorCopy( self->s.pos.trBase, start );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "ironbomb";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = bolt->splashDamage = availableWeapons[self->client->ps.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[self->client->ps.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[self->client->ps.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->methodOfDeath = MOD_IRONBOMB;
	bolt->splashMethodOfDeath = MOD_IRONBOMB_SPLASH;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, self->client->ps.speed/10, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_autocannon MFQ3
=================
*/
void fire_autocannon (gentity_t *self, qboolean main) {
	gentity_t	*bolt;
	vec3_t		dir, forward, right, up, temp;
	vec3_t		start, offset;
	vec3_t		spreadangle;
	int			weapIdx = (main ? self->client->ps.weaponIndex : availableVehicles[self->client->vehicle].weapons[0]);
//	float		spreadX = availableWeapons[availableVehicles[self->client->vehicle].weapons[0]].spread; 
	float		spreadX = availableWeapons[weapIdx].spread; 
	float		spreadY = spreadX;

	// used for spread
	VectorCopy( self->client->ps.vehicleAngles, spreadangle );
	spreadX = ((rand() % (unsigned int)spreadX) - spreadX/2)/10;
	spreadY = ((rand() % (unsigned int)spreadY) - spreadY/2)/10;
	spreadangle[0] += spreadX;
	spreadangle[1] += spreadY;
//	G_Printf( "spread %.1f %.1f\n", spreadX, spreadY );

	VectorCopy( availableVehicles[self->client->vehicle].gunoffset, offset );
	VectorCopy( self->s.pos.trBase, start );

	if( (availableVehicles[self->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[self->client->vehicle].cat & CAT_BOAT) ||
		(availableVehicles[self->client->vehicle].cat & CAT_HELO)) {
		// use this to make it shoot where the player looks
//		AngleVectors( self->client->ps.viewangles, dir, 0, 0 );
//		VectorCopy( self->s.pos.trBase, start );
//		start[2] += availableVehicles[self->client->vehicle].maxs[2];
		// otherwise use this
		AngleVectors( spreadangle, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ((float)self->client->ps.turretAngle)/10 );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ((float)self->client->ps.gunAngle)/10 );

	} else {
		// planes and helos for now just shoot along their direction of flight
		AngleVectors( spreadangle, dir, right, up );
	}

	if( availableVehicles[self->client->vehicle].caps & HC_DUALGUNS ) {
		self->bulletpos = ( self->bulletpos == 0 ? 1 : 0 );
		if( self->bulletpos ) {
			offset[1] *= -1;
		}
	}

	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "bullet";
	bolt->nextthink = level.time + 4000;
	bolt->think = G_FreeEntity;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = weapIdx;
	//availableVehicles[self->client->vehicle].weapons[WP_MACHINEGUN];
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = availableWeapons[weapIdx].damage;
	//availableWeapons[availableVehicles[self->client->vehicle].weapons[WP_MACHINEGUN]].damage;
	bolt->targetcat = availableWeapons[weapIdx].category;
	bolt->catmodifier = availableWeapons[weapIdx].noncatmod;
	bolt->methodOfDeath = MOD_AUTOCANNON;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	//tracer
	self->tracerindex --;
	if( self->tracerindex < 0 ) self->tracerindex = 255;
	bolt->s.eType = ET_BULLET;
	bolt->s.generic1 = self->tracerindex;
	VectorCopy( start, bolt->s.pos.trBase );
//	VectorScale( dir, availableWeapons[availableVehicles[self->client->vehicle].weapons[WP_MACHINEGUN]].muzzleVelocity, bolt->s.pos.trDelta );
	VectorScale( dir, availableWeapons[weapIdx].muzzleVelocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_autocannon_GI MFQ3
=================
*/
void fire_autocannon_GI (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		dir;
	vec3_t		start;
	int			weapIdx = self->s.weaponIndex;
	float		spreadX = availableWeapons[weapIdx].spread; 
	float		spreadY = spreadX;

	// used for spread
	VectorSubtract( self->tracktarget->r.currentOrigin, self->r.currentOrigin, dir );
	VectorNormalize(dir);
	VectorCopy( self->s.pos.trBase, start );
	start[2] += 10;
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "bullet";
	bolt->nextthink = level.time + 4000;
	bolt->think = G_FreeEntity;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = weapIdx;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = availableWeapons[weapIdx].damage;
	bolt->targetcat = availableWeapons[weapIdx].category;
	bolt->catmodifier = availableWeapons[weapIdx].noncatmod;
	bolt->methodOfDeath = MOD_AUTOCANNON;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;
	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = level.time - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	//tracer
	self->tracerindex --;
	if( self->tracerindex < 0 ) self->tracerindex = 255;
	bolt->s.eType = ET_BULLET;
	bolt->s.generic1 = self->tracerindex;
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, availableWeapons[weapIdx].muzzleVelocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
fire_maingun MFQ3
=================
*/
void fire_maingun( gentity_t *self ) {
	gentity_t	*bolt;
	vec3_t		dir, forward, right, up, temp;
	vec3_t		start, offset;

	AngleVectors( self->client->ps.vehicleAngles, forward, right, up );
	RotatePointAroundVector( temp, up, forward, ((float)self->client->ps.turretAngle)/10 );
	CrossProduct( up, temp, right );
	RotatePointAroundVector( dir, right, temp, ((float)self->client->ps.gunAngle)/10 );

#pragma message("we NEED access to the guntag in order to properly place the gunshell!!!!!!!!!!!!!")
	VectorCopy( self->s.pos.trBase, start );
	VectorCopy( availableVehicles[self->client->vehicle].gunoffset, offset );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "shell";
	bolt->nextthink = level.time + 15000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = bolt->splashDamage = availableWeapons[self->client->ps.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[self->client->ps.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[self->client->ps.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->methodOfDeath = MOD_MAINGUN;
	bolt->splashMethodOfDeath = MOD_MAINGUN;
	bolt->clipmask = MASK_SHOT;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, availableWeapons[self->client->ps.weaponIndex].muzzleVelocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}



/*
=================
fire_flare MFQ3
=================
*/
void fire_flare( gentity_t *self ) {
	gentity_t	*bolt;
	vec3_t		start, up;
	
	VectorSet( up, 0, 0, 1 );
	VectorCopy( self->s.pos.trBase, start );
	if( (availableVehicles[self->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[self->client->vehicle].cat & CAT_BOAT) ||
		(availableVehicles[self->client->vehicle].cat & CAT_HELO) ) 
		VectorMA( start, self->r.maxs[2]+3, up, start );
	else
		VectorMA( start, self->r.mins[2]-3, up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "flare";
	bolt->nextthink = level.time + 1600;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = WI_FLARE;
		
	bolt->parent = self;
	bolt->r.ownerNum = self->s.number;

	bolt->damage = bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->clipmask = MASK_SHOT;
	bolt->ONOFF = 1;// not used

	bolt->s.pos.trType = TR_GRAVITY_10;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	if( (availableVehicles[self->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[self->client->vehicle].cat & CAT_BOAT) ) 
		VectorScale( up, 300, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

}

/*
=================
fire_flare2 MFQ3
fire flare with direction
=================
*/
void fire_flare2( gentity_t *self, vec3_t start, vec3_t up, int age ) {
	gentity_t	*bolt;
	
	bolt = G_Spawn();
	bolt->classname = "flare";
	bolt->nextthink = level.time + age;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = WI_FLARE;
		
	bolt->parent = self;
	bolt->r.ownerNum = self->s.number;

	bolt->damage = bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->clipmask = MASK_SHOT;
	bolt->ONOFF = 1;// not used

	bolt->s.pos.trType = TR_GRAVITY_10;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( up, 1, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
fire_cflare
fire flare cluster
=================
*/
void fire_cflare( gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		start, up;
	int age;
	
	VectorSet( up, 0, 0, 1 );
	VectorCopy( self->s.pos.trBase, start );
	if( (availableVehicles[self->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[self->client->vehicle].cat & CAT_BOAT) ||
		(availableVehicles[self->client->vehicle].cat & CAT_HELO) )  {
		VectorMA( start, self->r.maxs[2]+3, up, start );
		age = 1600;
	}
	else {
		VectorMA( start, self->r.mins[2]-3, up, start );
		age = 800;
	}
	SnapVector( start );

	
	bolt = G_Spawn();
	bolt->classname = "cflare";
	bolt->nextthink = level.time + age;
	bolt->think = G_ExplodeCFlare;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = WI_CFLARE;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = bolt->splashDamage = 0;
	bolt->splashRadius = 0;
	bolt->clipmask = MASK_SHOT;
	bolt->ONOFF = 1;// not used

	bolt->s.pos.trType = TR_GRAVITY_10;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	if( (availableVehicles[self->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[self->client->vehicle].cat & CAT_BOAT) ) 
			VectorScale( up, 300, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
use_fueltank MFQ3
=================
*/
void drop_fueltank (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		dir, right, up, offset;
	vec3_t		start;

//	self->left = (self->left ? qfalse : qtrue);
	MF_removeWeaponFromLoadout(self->client->ps.weaponIndex, &self->loadout, 0, offset, 0 );
	AngleVectors( self->client->ps.vehicleAngles, dir, right, up );
	VectorInverse( right );
	VectorCopy( self->s.pos.trBase, start );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "droptank";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = bolt->splashDamage = availableWeapons[self->client->ps.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[self->client->ps.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[self->client->ps.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->methodOfDeath = MOD_IRONBOMB;
	bolt->splashMethodOfDeath = MOD_IRONBOMB_SPLASH;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, self->client->ps.speed/10, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
================
G_CrateDie
================
*/
void G_CrateDie( gentity_t *self, gentity_t *inflictor,
        gentity_t *attacker, int damage, int mod ) {
        if (inflictor == self)
                return;
        self->takedamage = qfalse;
        self->think = G_ExplodeMissile;
        self->nextthink = level.time + 10;
}


/*
=================
drop_crate
=================
*/
void drop_crate (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		start;

	MF_removeWeaponFromLoadout(self->client->ps.weaponIndex, &self->loadout, 0, 0, 0 );

	VectorCopy( self->s.pos.trBase, start );
	//start[2] -= availableVehicles[self->client->vehicle].maxs[2] - availableVehicles[self->client->vehicle].mins[2];
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "crate";
	bolt->nextthink = level.time + 100;
	//bolt->nextthink = level.time + 10000;
	bolt->think = G_CrateThink;
	//bolt->think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;
	bolt->speed = bolt->speed = availableWeapons[self->client->ps.weaponIndex].muzzleVelocity;

	bolt->s.pos.trType = TR_LINEAR;
	//bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time;

	bolt->health = 5;
	bolt->takedamage = qtrue;
	bolt->die = G_CrateDie;
	bolt->r.contents = CONTENTS_BODY;

	VectorSet(bolt->r.mins, -10, -3, 0);
	VectorCopy(bolt->r.mins, bolt->r.absmin);
	VectorSet(bolt->r.maxs, 10, 3, 6);
	VectorCopy(bolt->r.maxs, bolt->r.absmax);

	VectorCopy( start, bolt->s.pos.trBase );
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_nukebomb MFQ3
=================
*/
void fire_nukebomb (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		dir, right, up, offset;
	vec3_t		start;

//	self->left = (self->left ? qfalse : qtrue);
	MF_removeWeaponFromLoadout(self->client->ps.weaponIndex, &self->loadout, 0, offset, 0 );
	AngleVectors( self->client->ps.vehicleAngles, dir, right, up );
	VectorInverse( right );
	VectorCopy( self->s.pos.trBase, start );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "nukebomb";
	bolt->nextthink = level.time + 10000;
	bolt->think = G_ExplodeNuke;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = 1000;
	bolt->splashDamage = 0;
	bolt->damage = availableWeapons[self->client->ps.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[self->client->ps.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[self->client->ps.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->methodOfDeath = MOD_NUKE;
	bolt->splashMethodOfDeath = MOD_NUKE;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, self->client->ps.speed/10, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
fire_nukemissile MFQ3
=================
*/
void fire_nukemissile (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		dir, right, up;
	vec3_t		start, offset, forward, temp;
	qboolean	wingtip = qfalse;

	VectorCopy( self->s.pos.trBase, start );

	if( (availableVehicles[self->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[self->client->vehicle].cat & CAT_BOAT) ) {
		VectorCopy( availableVehicles[self->client->vehicle].gunoffset, offset );
		AngleVectors( self->client->ps.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ((float)self->client->ps.turretAngle)/10 );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ((float)self->client->ps.gunAngle)/10 );
		wingtip = qtrue; // dont drop
	} else {
		self->left = (self->left ? qfalse : qtrue);
		MF_removeWeaponFromLoadout(self->client->ps.weaponIndex, &self->loadout, 0, offset, 0 );
		AngleVectors( self->client->ps.vehicleAngles, dir, right, up );
		VectorInverse( right );
	}
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "nukemissile";

	bolt->speed = availableWeapons[self->client->ps.weaponIndex].muzzleVelocity;

	bolt->think = G_ExplodeNuke;
	bolt->nextthink = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;

	bolt->s.pos.trType = TR_LINEAR;
	VectorScale( dir, bolt->speed, bolt->s.pos.trDelta );

	// Give health? TODO...

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client->ps.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;

	bolt->damage = availableWeapons[self->client->ps.weaponIndex].damage;
	bolt->splashDamage = 0;
	bolt->splashRadius = availableWeapons[self->client->ps.weaponIndex].damageRadius;

	bolt->targetcat = availableWeapons[self->client->ps.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->client->ps.weaponIndex].noncatmod;
	bolt->range = availableWeapons[self->client->ps.weaponIndex].range;
	bolt->methodOfDeath = MOD_NUKE;
	bolt->splashMethodOfDeath = MOD_NUKE;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;
	bolt->basicECMVulnerability = availableWeapons[self->client->ps.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	
}


// =================================================================================================================
// ================================================== GI MISSILE STUFF =============================================
// =================================================================================================================




/*
=================
LaunchMissile_GI MFQ3
=================
*/
void LaunchMissile_GI( gentity_t* ent )
{
	gentity_t	*bolt;
	vec3_t		dir, start;

	ent->count--;
//	G_Printf("Launching GI missile....%d left\n", ent->count);

	VectorCopy( ent->s.pos.trBase, start );
	start[2] += 10;
	VectorSubtract( ent->tracktarget->r.currentOrigin, ent->r.currentOrigin, dir );
	VectorNormalize(dir);
	SnapVector( start );

	bolt = G_Spawn();
	bolt->classname = "aam";
	bolt->tracktarget = ent->tracktarget;
	// update target
	if( ent->tracktarget->client ) {
		ent->tracktarget->client->ps.stats[STAT_LOCKINFO] |= LI_BEING_LAUNCHED;
	}
	bolt->followcone = availableWeapons[ent->s.weaponIndex].followcone;
	bolt->speed = availableWeapons[ent->s.weaponIndex].muzzleVelocity;
	bolt->wait = level.time + availableWeapons[ent->s.weaponIndex].fuelrange;
	bolt->think = follow_target;
	bolt->nextthink = level.time + 50;
	bolt->s.pos.trType = TR_LINEAR;
	VectorScale( dir, bolt->speed, bolt->s.pos.trDelta );
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = ent->s.weaponIndex;
	bolt->r.ownerNum = ent->s.number;
	bolt->parent = ent;
	bolt->damage = bolt->splashDamage = availableWeapons[ent->s.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[ent->s.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[ent->s.weaponIndex].category;
	bolt->catmodifier = availableWeapons[ent->s.weaponIndex].noncatmod;
	bolt->range = availableWeapons[ent->s.weaponIndex].range;
	bolt->methodOfDeath = MOD_FFAR;
	bolt->splashMethodOfDeath = MOD_FFAR_SPLASH;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;
	bolt->basicECMVulnerability = availableWeapons[ent->s.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}



/*
=================
fire_flak_GI
=================
*/
#define FLAK_DEVIATION_XY 800
#define FLAK_DEVIATION_HEIGHT 200
void fire_flak_GI (gentity_t *self) {
	gentity_t	*bolt;
	vec3_t		start, offset;
	qboolean	wingtip = qfalse;
	int		fuse;
	static int	seed = 0x92;
	int dis;

	// Find fuse time
	dis = Distance(self->r.currentOrigin,self->tracktarget->r.currentOrigin);
	if(dis < 1000)
		return;	// Need to be above reasonably!
	fuse = (float)dis/availableWeapons[self->s.weaponIndex].muzzleVelocity;


	// Cheat using a random detonation position near player
	start[0] = self->tracktarget->r.currentOrigin[0] + (FLAK_DEVIATION_XY - (FLAK_DEVIATION_XY * -1) + 1) * Q_random( &seed ) + (FLAK_DEVIATION_XY * -1);
	start[1] = self->tracktarget->r.currentOrigin[1] + (FLAK_DEVIATION_XY - (FLAK_DEVIATION_XY * -1) + 1) * Q_random( &seed ) + (FLAK_DEVIATION_XY * -1);
	start[2] = self->tracktarget->r.currentOrigin[2] + (FLAK_DEVIATION_HEIGHT - (FLAK_DEVIATION_HEIGHT * -1) + 1) * Q_random( &seed ) + (FLAK_DEVIATION_HEIGHT * -1);

	// Remove one weapon to fire
	MF_removeWeaponFromLoadout(self->s.weaponIndex, &self->loadout, &wingtip, offset, 0 );
	


	bolt = G_Spawn();
	bolt->classname = "flak";

	bolt->speed = availableWeapons[self->s.weaponIndex].muzzleVelocity;
	bolt->think = G_ExplodeFlak;
	bolt->nextthink = level.time + fuse;
	bolt->s.pos.trType = TR_LINEAR;


	bolt->s.eType = ET_INVISIBLE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->s.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent = self;
	bolt->damage = bolt->splashDamage = availableWeapons[self->s.weaponIndex].damage;
	bolt->splashRadius = availableWeapons[self->s.weaponIndex].damageRadius;
	bolt->targetcat = availableWeapons[self->s.weaponIndex].category;
	bolt->catmodifier = availableWeapons[self->s.weaponIndex].noncatmod;
	bolt->range = availableWeapons[self->s.weaponIndex].range;
	bolt->methodOfDeath = MOD_FFAR;
	bolt->splashMethodOfDeath = MOD_FFAR_SPLASH;
	bolt->clipmask = MASK_SHOT|MASK_WATER;
	bolt->target_ent = NULL;
	bolt->basicECMVulnerability = availableWeapons[self->s.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = level.time;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	
}


