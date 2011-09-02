// p_weapon.c

#include "g_local.h"



/*
======================================================================

IronBombs

======================================================================
*/

void Weapon_Bombs_Fire (edict_t *vehicle)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		VM = (strcmp(vid_ref->string, "gl") == 0 ? -1 : 1);

	if( vehicle->attenuation > level.time || vehicle->ammo[vehicle->active_weapon] < 1 )
		return;
	if( !(vehicle->ONOFF & ONOFF_AIRBORNE) )
		return;
	if( ( vehicle->s.angles[0] > 270 || vehicle->s.angles[0] <= 90 ) &&
		fabs(vehicle->s.angles[2]) > 90 )
		return;
	if( ( vehicle->s.angles[0] <= 270 && vehicle->s.angles[0] > 90 ) &&
		fabs(vehicle->s.angles[2]) < 90 )
		return;

	vehicle->dmg_radius = vehicle->dmg_radius ? 0 : 1;

	VectorCopy( vehicle->s.angles, angles );
	if( vehicle->HASCAN & HASCAN_DRONE )
		angles[2] *= VM;
	else if( vehicle->owner )
		angles[2] *= ((vehicle->owner->client->FLAGS & CLIENT_NONGL_MODE) ? 1 : -1 );

	AngleVectors( angles, forward, right, up );

	// +BD this block will change the side where the bombs start
	VectorCopy( vehicle->s.origin, start );
	VectorScale( up, -3, up );
	VectorScale( right, 12, right );

	if( vehicle->dmg_radius == 1 )
		VectorInverse(right);
	if( !(vehicle->HASCAN & HASCAN_WEAPONBAY) ||
			strcmp( vehicle->classname, "helo" ) == 0)
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_bombs( vehicle, start, forward ); 

	vehicle->ammo[vehicle->active_weapon]--;

	vehicle->attenuation = level.time + 0.3;
}


/*
======================================================================

FFAR

======================================================================
*/

void Weapon_FFAR_Fire (edict_t *vehicle)
{
	vec3_t	start, right, up;
	vec3_t	forward, angles;
	int		VM = (strcmp(vid_ref->string, "gl") == 0 ? -1 : 1);

	if( vehicle->attenuation > level.time || vehicle->ammo[vehicle->active_weapon] < 1 )
		return;

	VectorCopy( vehicle->s.angles, angles );
	if( vehicle->HASCAN & HASCAN_DRONE )
		angles[2] *= VM;
	else if( vehicle->owner )
		angles[2] *= ((vehicle->owner->client->FLAGS & CLIENT_NONGL_MODE) ? 1 : -1 );
		
	AngleVectors( angles, forward, right, up );// +BD new

	// +BD this block will change the side where the rocket starts
	VectorCopy( vehicle->s.origin, start );
	VectorScale( up, -3, up );
	if( strcmp( vehicle->classname, "plane" ) == 0 )
		VectorScale( right, 12, right );
	else
		VectorScale( right, 4, right );

	if( strcmp( vehicle->classname, "plane" ) == 0 ||
		strcmp( vehicle->classname, "helo" ) == 0 )
	{
		vehicle->dmg_radius = vehicle->dmg_radius ? 0 : 1;
		if( vehicle->dmg_radius == 1 )
			VectorInverse(right);
		if( !(vehicle->HASCAN & HASCAN_WEAPONBAY) ||
			strcmp( vehicle->classname, "helo" ) == 0 )
			VectorAdd( start, right, start );
		VectorAdd( start, up, start );
	}

	fire_rocket (vehicle, start, forward );

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (vehicle-g_edicts);
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (vehicle->s.origin, MULTICAST_PVS);

	vehicle->ammo[vehicle->active_weapon]--;

	vehicle->attenuation = level.time + 0.3;

}





/*
======================================================================

AAM

======================================================================
*/

void Weapon_AAM_Fire (edict_t *vehicle)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		VM = (strcmp(vid_ref->string, "gl") == 0 ? -1 : 1);


	if( vehicle->attenuation > level.time || vehicle->ammo[vehicle->active_weapon] < 1 )
		return;

	if( strcmp( vehicle->classname, "ground" ) == 0 )
		VectorCopy( vehicle->movetarget->s.angles, angles );
	else if( strcmp( vehicle->classname, "LQM" ) == 0 )
	{
		VectorCopy( vehicle->s.angles, angles );
		angles[0] = vehicle->ideal_yaw;
	}
	else
		VectorCopy( vehicle->s.angles, angles );

	if( vehicle->HASCAN & HASCAN_DRONE )
		angles[2] *= VM;
	else if( vehicle->owner )
		angles[2] *= ((vehicle->owner->client->FLAGS & CLIENT_NONGL_MODE) ? 1 : -1 );

	AngleVectors( angles, forward, right, up );// +BD new

	// +BD this block will change the side where the rocket starts
	if( strcmp( vehicle->classname, "ground" ) == 0 )
		VectorCopy( vehicle->movetarget->s.origin, start );
	else
		VectorCopy( vehicle->s.origin, start );
	if( strcmp( vehicle->classname, "plane" ) == 0 )
	{
		VectorScale( right, 12, right );
		VectorScale( up, -3, up );
	}
	else if( strcmp( vehicle->classname, "helo" ) == 0 )
	{
		VectorScale( right, 6, right );
		VectorScale( up, -3, up );
	}
	else if( strcmp( vehicle->classname, "ground" ) == 0 )
	{
		VectorScale( right, 6, right );
		VectorScale( up, 2, up );
	}

	if( strcmp( vehicle->classname, "plane" ) == 0 ||
		strcmp( vehicle->classname, "helo" ) == 0 )
	{
		vehicle->dmg_radius = vehicle->dmg_radius ? 0 : 1;
		if( vehicle->dmg_radius == 1 )
			VectorInverse(right);
		if( !(vehicle->HASCAN & HASCAN_WEAPONBAY) ||
			strcmp( vehicle->classname, "helo" ) == 0 )
			VectorAdd( start, right, start );
		VectorAdd( start, up, start );
	}

	fire_AAM( vehicle, start, forward );

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle

	vehicle->ammo[vehicle->active_weapon]--;

	if( vehicle->weapon[vehicle->active_weapon] == WEAPON_SIDEWINDER )
		vehicle->attenuation = level.time + 1.2;
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_STINGER )
		vehicle->attenuation = level.time + 0.8;
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_AMRAAM || 
			vehicle->weapon[vehicle->active_weapon] == WEAPON_PHOENIX )
		vehicle->attenuation = level.time + 1.4;

}






/*
======================================================================

LGB + Runway

======================================================================
*/

void Weapon_LGB_Fire (edict_t *vehicle)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		VM = (strcmp(vid_ref->string, "gl") == 0 ? -1 : 1);

	if( vehicle->attenuation > level.time || vehicle->ammo[vehicle->active_weapon] < 1 )
		return;
	if( !(vehicle->ONOFF & ONOFF_AIRBORNE) )
		return;
	if( ( vehicle->s.angles[0] > 270 || vehicle->s.angles[0] <= 90 ) &&
		fabs(vehicle->s.angles[2]) > 90 )
		return;
	if( ( vehicle->s.angles[0] <= 270 && vehicle->s.angles[0] > 90 ) &&
		fabs(vehicle->s.angles[2]) < 90 )
		return;

	vehicle->dmg_radius = vehicle->dmg_radius ? 0 : 1;

	VectorCopy( vehicle->s.angles, angles );
	if( vehicle->HASCAN & HASCAN_DRONE )
		angles[2] *= VM;
	else if( vehicle->owner )
		angles[2] *= ((vehicle->owner->client->FLAGS & CLIENT_NONGL_MODE) ? 1 : -1 );

	AngleVectors( angles, forward, right, up );

	// +BD this block will change the side where the bombs start
	VectorCopy( vehicle->s.origin, start );
	VectorScale( up, -3, up );
	VectorScale( right, 12, right );

	if( vehicle->dmg_radius == 1 )
		VectorInverse(right);
	if( !(vehicle->HASCAN & HASCAN_WEAPONBAY) ||
			strcmp( vehicle->classname, "helo" ) == 0)
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_LGB( vehicle, start, forward ); 

	vehicle->ammo[vehicle->active_weapon]--;

	if( vehicle->weapon[vehicle->active_weapon] == WEAPON_LASERBOMBS )
		vehicle->attenuation = level.time + 1.0;
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_RUNWAYBOMBS )
		vehicle->attenuation = level.time + 0.8;
}






/*
======================================================================

ATGM

======================================================================
*/

void Weapon_ATGM_Fire (edict_t *vehicle)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		VM = (strcmp(vid_ref->string, "gl") == 0 ? -1 : 1);


	if( vehicle->attenuation > level.time || vehicle->ammo[vehicle->active_weapon] < 1 )
		return;

	if( strcmp( vehicle->classname, "ground" ) == 0 )
		VectorCopy( vehicle->movetarget->s.angles, angles );
	else if( strcmp( vehicle->classname, "LQM" ) == 0 )
	{
		VectorCopy( vehicle->s.angles, angles );
		angles[0] = vehicle->ideal_yaw;
	}
	else
		VectorCopy( vehicle->s.angles, angles );

	if( vehicle->HASCAN & HASCAN_DRONE )
		angles[2] *= VM;
	else if( vehicle->owner )
		angles[2] *= ((vehicle->owner->client->FLAGS & CLIENT_NONGL_MODE) ? 1 : -1 );

	AngleVectors( angles, forward, right, up );// +BD new

	// +BD this block will change the side where the rocket starts
	if( strcmp( vehicle->classname, "ground" ) == 0 )
		VectorCopy( vehicle->movetarget->s.origin, start );
	else
		VectorCopy( vehicle->s.origin, start );
	if( strcmp( vehicle->classname, "plane" ) == 0 )
	{
		VectorScale( right, 12, right );
		VectorScale( up, -3, up );
	}
	else if( strcmp( vehicle->classname, "helo" ) == 0 )
	{
		VectorScale( right, 6, right );
		VectorScale( up, -3, up );
	}
	else if( strcmp( vehicle->classname, "ground" ) == 0 )
	{
		VectorScale( right, 6, right );
		VectorScale( up, 2, up );
	}

	if( strcmp( vehicle->classname, "plane" ) == 0 ||
		strcmp( vehicle->classname, "helo" ) == 0 )
	{
		vehicle->dmg_radius = vehicle->dmg_radius ? 0 : 1;
		if( vehicle->dmg_radius == 1 )
			VectorInverse(right);
		if( !(vehicle->HASCAN & HASCAN_WEAPONBAY) ||
			strcmp( vehicle->classname, "helo" ) == 0 )
			VectorAdd( start, right, start );
		VectorAdd( start, up, start );
	}

	fire_ATGM( vehicle, start, forward );

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle

	vehicle->ammo[vehicle->active_weapon]--;

	if( vehicle->weapon[vehicle->active_weapon] == WEAPON_ANTIRADAR )
		vehicle->attenuation = level.time + 1.2;
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_HELLFIRE || 
			vehicle->weapon[vehicle->active_weapon] == WEAPON_MAVERICK )
		vehicle->attenuation = level.time + 1.0;

}








// =======================================

// AutoCannon

// =======================================


void Weapon_Autocannon_Fire (edict_t *vehicle)
{

	vec3_t	forward, right;
	vec3_t	start;
	vec3_t	angles;		// used for spread
	vec3_t	direction;
	vec3_t  forward2;
	vec3_t	aimatthis;
	float	targetspeed;
	float	timetotarget;
	float	spread;

	if( vehicle->autocannon_time > level.time || vehicle->ammo[0] < vehicle->guntype->barrels )
		return;

	if( (vehicle->HASCAN & HASCAN_AUTOAIM) && vehicle->weapon[vehicle->active_weapon] == WEAPON_AUTOCANNON 
		&& vehicle->enemy )
	{
		AngleVectors( vehicle->enemy->s.angles, forward, NULL, NULL );
		VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, angles );
		timetotarget = VectorLength( angles )/(vehicle->guntype->bullet_speed*FRAMETIME);
		targetspeed = (VectorLength( vehicle->enemy->velocity )*FRAMETIME)*timetotarget;
		VectorMA( vehicle->enemy->s.origin, targetspeed, forward, aimatthis );
		VectorSubtract( aimatthis, vehicle->s.origin, direction );
		vectoangles( direction, angles );
	}
	else
	{
		if( strcmp( vehicle->classname, "ground" ) == 0 )
			VectorCopy( vehicle->movetarget->s.angles, angles );
		else if( strcmp( vehicle->classname, "LQM" ) == 0 )
		{
			VectorCopy( vehicle->s.angles, angles );
			angles[0] = vehicle->ideal_yaw;
		}
		else
			VectorCopy( vehicle->s.angles, angles );
	}

	// add spread
	spread = vehicle->guntype->spread;
	if( (vehicle->HASCAN & HASCAN_AUTOAIM) )
		spread /= 2;
	angles[0] += (random()*spread)-(spread/2);
	angles[1] += (random()*spread)-(spread/2);
	AngleVectors( angles, forward, right, NULL );

/*	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
		AngleVectors( ent->vehicle->movetarget->s.angles, forward, right, NULL );
	else
		AngleVectors ( ent->vehicle->s.angles, forward, right, NULL); // +BD new
*/
	VectorCopy( vehicle->s.origin, start );
	VectorCopy( forward, forward2 );
	VectorScale( forward2, vehicle->maxs[0], forward2 );
	VectorAdd( vehicle->s.origin, forward2, start );
	start[2] += 2;

	// set the start position
	if( vehicle->guntype->barrels > 1 )
	{
		VectorMA( start, vehicle->guntype->barrel_dist, right, start );
		fire_round (vehicle, start, forward );
		VectorMA( start, -2*vehicle->guntype->barrel_dist, right, start );
		fire_round (vehicle, start, forward );
	}
	else
		fire_round (vehicle, start, forward );

	// send muzzle flash
/*	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (vehicle-g_edicts);
	gi.WriteByte (MZ_CHAINGUN3);
	gi.multicast (vehicle->s.origin, MULTICAST_PVS);*/

	gi.sound( vehicle, CHAN_WEAPON, gi.soundindex( "weapons/machgf1b.wav" ), 1.0, ATTN_NORM, 0 );

	vehicle->ammo[0] -= vehicle->guntype->barrels;
	vehicle->autocannon_time = level.time + vehicle->guntype->firerate;
}




// =======================================

// Jammer

// =======================================


void Weapon_Jammer_Fire (edict_t *vehicle)
{
	if( vehicle->attenuation > level.time || vehicle->ammo[vehicle->active_weapon] < 0 )
		return;	
	
	vehicle->ONOFF ^= ONOFF_JAMMER;

	vehicle->attenuation = level.time + 0.3;
}

void Reload_Jammer( edict_t *vehicle )
{
	if( vehicle->ammo[vehicle->active_weapon] < vehicle->maxammo[vehicle->active_weapon] )
		vehicle->ammo[vehicle->active_weapon]++;
}

void Jammer_Active( edict_t *vehicle )
{
	edict_t *lock = NULL;

	if( vehicle->ammo[vehicle->active_weapon] > 0 )
	{
		vehicle->ammo[vehicle->active_weapon]--;

//		gi.bprintf( PRINT_HIGH, "jam!\n");
		while( (lock = findradius( lock, vehicle->s.origin, STANDARD_RANGE*JAMMER_MODIFIER ) ) != NULL )
		{
			if( strcmp( lock->classname, "plane" ) != 0 &&
				strcmp( lock->classname, "helo" ) != 0 &&
				strcmp( lock->classname, "ground" )!= 0 )
				continue;
			if( vehicle->owner )
			{
				if( lock == vehicle->owner )
					continue;
			}
			if( !lock->takedamage )
				continue;
			if( lock->deadflag )
				continue;
			lock->jam_time = level.time + 0.3;
		}
	}
}



// =======================================

// Flares

// =======================================

void Fire_Flare( edict_t *vehicle )
{
	vec3_t	forward;
	vec3_t	start;

	if( level.time < vehicle->volume || vehicle->flares < 1 )
		return;

	vehicle->flares--;

	AngleVectors ( vehicle->s.angles, forward, NULL, NULL);
	VectorCopy( vehicle->s.origin, start );
	start[2] += 15;

	fire_flare (vehicle, start, forward );

	vehicle->volume = level.time + 0.8;

	// send muzzle flash
/*	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);
	gi.WriteByte (MZ_SILENCED);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);*/
}




void Fire_Weapon( edict_t *vehicle )
{
	if( vehicle->weapon[vehicle->active_weapon] != WEAPON_AUTOCANNON &&
		(vehicle->HASCAN & HASCAN_WEAPONBAY) && !(vehicle->ONOFF & ONOFF_WEAPONBAY) )
	{
		if( !(vehicle->DAMAGE & DAMAGE_WEAPONBAY) && (vehicle->ONOFF & ONOFF_AIRBORNE) &&
			level.time > vehicle->attenuation )
		{
			vehicle->ONOFF |= ONOFF_WEAPONBAY;
			if( vehicle->HASCAN & HASCAN_STEALTH )
				vehicle->ONOFF &= ~ONOFF_STEALTH;
			vehicle->wait = level.time + 3.0;
			vehicle->attenuation = level.time + 1.0;			
			return;
		}
		else
			return;
	}
	if( strcmp( vehicle->classname, "LQM" ) == 0 && (vehicle->ONOFF & ONOFF_AIRBORNE) )
		return;
	if( (strcmp( vehicle->classname, "plane" ) == 0 || strcmp( vehicle->classname, "helo" ) == 0 ) &&
		!(vehicle->ONOFF & ONOFF_AIRBORNE) && !(vehicle->ONOFF & ONOFF_LANDED) )
		return;

	if( vehicle->weapon[vehicle->active_weapon] == WEAPON_AUTOCANNON )
		Weapon_Autocannon_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_FFAR )
		Weapon_FFAR_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_IRONBOMBS )
		Weapon_Bombs_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_SIDEWINDER )
		Weapon_AAM_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_STINGER )
		Weapon_AAM_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_AMRAAM )
		Weapon_AAM_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_PHOENIX )
		Weapon_AAM_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_JAMMER )
		Weapon_Jammer_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_HELLFIRE )
		Weapon_ATGM_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_MAVERICK )
		Weapon_ATGM_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_ANTIRADAR )
		Weapon_ATGM_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_LASERBOMBS )
		Weapon_LGB_Fire( vehicle );
	else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_RUNWAYBOMBS )
		Weapon_LGB_Fire( vehicle );

	vehicle->wait = level.time + 3.0;
}

void AAM_Lock( edict_t *vehicle )
{
	vec3_t		forward, testvec;
	float		test;
	edict_t		*lock = NULL;
	float		range;
	vec3_t angle;
	
	VectorCopy( vehicle->s.angles, angle );
	if( strcmp( vehicle->classname, "LQM" ) == 0 )
		angle[0] = vehicle->ideal_yaw;

//	gi.bprintf( PRINT_HIGH, "Side\n" );

	if( vehicle->enemy && ( vehicle->enemy->deadflag || strcmp( vehicle->enemy->classname, "freed" ) == 0 ))
	{
		vehicle->enemy->lockstatus = LS_NOLOCK;
		vehicle->enemy = NULL;
	}

	// is jammed
	if( vehicle->jam_time > level.time )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
		}
		return;
	}

//	if( level.time < vehicle->drone_decision_time )	// used when forcing unlock
//		return;

	switch( vehicle->weapon[vehicle->active_weapon] )
	{
	case WEAPON_SIDEWINDER:
		range = STANDARD_RANGE * SIDEWINDER_MODIFIER;
		break;
	case WEAPON_STINGER:
		range = STANDARD_RANGE * STINGER_MODIFIER;
		break;
	case WEAPON_AMRAAM:
		range = STANDARD_RANGE * AMRAAM_MODIFIER;
		break;
	case WEAPON_PHOENIX:
		range = STANDARD_RANGE * PHOENIX_MODIFIER;
		break;
	}

	if( vehicle->enemy )
	{
		if( !visible(vehicle,vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( !infront(vehicle,vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
		if( VectorLength( testvec ) > range )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( vehicle->enemy->HASCAN & HASCAN_STEALTH )
		{
			if( vehicle->enemy->ONOFF & ONOFF_STEALTH )
			{
				vehicle->enemy->lockstatus = LS_NOLOCK;
				vehicle->enemy = NULL;
				return;
			}
		}
		if( level.time < vehicle->enemy->jam_time )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		// test if within the target-cone
		AngleVectors( angle, forward, NULL, NULL );
		VectorNormalize( forward );
		VectorNormalize( testvec );
		test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
				testvec[2]*forward[2] );
		test *= (180/M_PI);
		if( test > 15 )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( teamplay->value != 1 )
		{
			if( !vehicle->enemy->lockstatus )
				vehicle->enemy->lockstatus = LS_LOCK;
		}
		else
		{
			if( vehicle->enemy->HASCAN & HASCAN_DRONE )	// don't screw up drones 
			{
				if( vehicle->enemy->aqteam != vehicle->aqteam )
				{
					if( !vehicle->enemy->lockstatus )
						vehicle->enemy->lockstatus = LS_LOCK;
				}
			}
			else
			{
				if( !vehicle->enemy->lockstatus )
					vehicle->enemy->lockstatus = LS_LOCK;
			}
		}
		return;
	}

	if( !vehicle->enemy )
	{
		while( (lock = findradius( lock, vehicle->s.origin, range ) ) != NULL )
		{
			if( lock == vehicle )
				continue;
			if( strcmp( lock->classname, "plane" ) != 0 &&
				strcmp( lock->classname, "helo" ) != 0 )
				continue;
			if( vehicle->owner )
				if( lock == vehicle->owner )
					continue;
			if( !lock->takedamage )
				continue;
			if( lock->deadflag )
				continue;
			if( strcmp(vehicle->classname, "plane") == 0 ||
				strcmp(vehicle->classname, "helo") == 0 ||
				strcmp(vehicle->classname, "LQM") == 0 )
			{
				if( !visible(vehicle,lock) )
					continue;
				if( !infront(vehicle, lock) )
					continue;
			}
			else if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				if( !visible(vehicle->movetarget,lock) )
					continue;
				if( !infront(vehicle->movetarget,lock) )
					continue;
			}
			// stealth
			if( lock->HASCAN & HASCAN_STEALTH )
			{
				if( lock->ONOFF & ONOFF_STEALTH )
					continue;
			}
			// jammer
			if( level.time < lock->jam_time )
				continue;
			// test if within the target-cone
			if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				AngleVectors( vehicle->movetarget->s.angles, forward, NULL, NULL );
			}
			else
			{
				AngleVectors( angle, forward, NULL, NULL );
			}
			VectorSubtract( lock->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > 8 )
				continue;
			vehicle->enemy = lock;
			if( teamplay->value != 1 )
				lock->lockstatus = LS_LOCK;
			else
			{
				if( vehicle->enemy->HASCAN & HASCAN_DRONE )
				{
					if( vehicle->enemy->aqteam != vehicle->aqteam )
						lock->lockstatus = LS_LOCK;
				}
				else
					lock->lockstatus = LS_LOCK;
			}
			break;
		}
	}
}




void ATGM_Lock( edict_t *vehicle )
{
	vec3_t		forward, testvec;
	float		test;
	edict_t		*lock = NULL;
	float		range;
	vec3_t angle;
	
	VectorCopy( vehicle->s.angles, angle );
	if( strcmp( vehicle->classname, "LQM" ) == 0 )
		angle[0] = vehicle->ideal_yaw;

//	gi.bprintf( PRINT_HIGH, "Side\n" );

	if( vehicle->enemy && ( vehicle->enemy->deadflag || strcmp( vehicle->enemy->classname, "freed" ) == 0 ))
	{
		vehicle->enemy->lockstatus = LS_NOLOCK;
		vehicle->enemy = NULL;
	}

	// is jammed
	if( vehicle->jam_time > level.time )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
		}
		return;
	}

//	if( level.time < vehicle->drone_decision_time )	// used when forcing unlock
//		return;

	switch( vehicle->weapon[vehicle->active_weapon] )
	{
	case WEAPON_MAVERICK:
		range = STANDARD_RANGE * MAVERICK_MODIFIER;
		break;
	case WEAPON_HELLFIRE:
		range = STANDARD_RANGE * HELLFIRE_MODIFIER;
		break;
	case WEAPON_LASERBOMBS:
		range = STANDARD_RANGE * LGB_MODIFIER;
		break;
	case WEAPON_ANTIRADAR:
		range = STANDARD_RANGE * ANTIRADAR_MODIFIER;
		break;
	case WEAPON_RUNWAYBOMBS:
		range = STANDARD_RANGE * RUNWAYBOMB_MODIFIER;
		break;
	}

	if( vehicle->enemy )
	{
		if( !visible(vehicle,vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( !infront(vehicle,vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
		if( VectorLength( testvec ) > range )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( vehicle->enemy->HASCAN & HASCAN_STEALTH )
		{
			if( vehicle->enemy->ONOFF & ONOFF_STEALTH )
			{
				vehicle->enemy->lockstatus = LS_NOLOCK;
				vehicle->enemy = NULL;
				return;
			}
		}
		if( level.time < vehicle->enemy->jam_time )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		// test if within the target-cone
		AngleVectors( angle, forward, NULL, NULL );
		VectorNormalize( forward );
		VectorNormalize( testvec );
		test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
				testvec[2]*forward[2] );
		test *= (180/M_PI);
		if( test > 15 )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( teamplay->value != 1 )
		{
			if( !vehicle->enemy->lockstatus )
				vehicle->enemy->lockstatus = LS_LOCK;
		}
		else
		{
			if( vehicle->enemy->HASCAN & HASCAN_DRONE )	// don't screw up drones 
			{
				if( vehicle->enemy->aqteam != vehicle->aqteam )
				{
					if( !vehicle->enemy->lockstatus )
						vehicle->enemy->lockstatus = LS_LOCK;
				}
			}
			else
			{
				if( !vehicle->enemy->lockstatus )
					vehicle->enemy->lockstatus = LS_LOCK;
			}
		}
		return;
	}
	if( !vehicle->enemy )
	{
		while( (lock = findradius( lock, vehicle->s.origin, range ) ) != NULL )
		{
			if( lock == vehicle )
				continue;
			if( strcmp( lock->classname, "ground" ) != 0 )
				continue;
			if( vehicle->owner )
				if( lock == vehicle->owner )
					continue;
			if( !lock->takedamage )
				continue;
			if( lock->deadflag )
				continue;
			if( strcmp(vehicle->classname, "plane") == 0 ||
				strcmp(vehicle->classname, "helo") == 0 ||
				strcmp(vehicle->classname, "LQM") == 0 )
			{
				if( !visible(vehicle,lock) )
					continue;
				if( !infront(vehicle, lock) )
					continue;
			}
			else if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				if( !visible(vehicle->movetarget,lock) )
					continue;
				if( !infront(vehicle->movetarget,lock) )
					continue;
			}
			// stealth
			if( lock->HASCAN & HASCAN_STEALTH )
			{
				if( lock->ONOFF & ONOFF_STEALTH )
					continue;
			}
			// jammer
			if( level.time < lock->jam_time )
				continue;
			// test if within the target-cone
			if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				AngleVectors( vehicle->movetarget->s.angles, forward, NULL, NULL );
			}
			else
			{
				AngleVectors( angle, forward, NULL, NULL );
			}
			VectorSubtract( lock->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > 8 )
				continue;
			vehicle->enemy = lock;
			if( teamplay->value != 1 )
				lock->lockstatus = LS_LOCK;
			else
			{
				if( vehicle->enemy->HASCAN & HASCAN_DRONE )
				{
					if( vehicle->enemy->aqteam != vehicle->aqteam )
						lock->lockstatus = LS_LOCK;
				}
				else
					lock->lockstatus = LS_LOCK;
			}
			break;
		}
	}
}

void Autocannon_Lock( edict_t *vehicle )
{
	vec3_t		forward, testvec;
	float		test;
	edict_t		*lock = NULL;
	float		range = STANDARD_RANGE*AUTOCANNON_MODIFIER;
	vec3_t angle;
	
	VectorCopy( vehicle->s.angles, angle );
	if( strcmp( vehicle->classname, "LQM" ) == 0 )
		angle[0] = vehicle->ideal_yaw;

	if( vehicle->enemy && ( vehicle->enemy->deadflag || strcmp( vehicle->enemy->classname, "freed" ) == 0 ))
	{
		vehicle->enemy->lockstatus = LS_NOLOCK;
		vehicle->enemy = NULL;
	}

	// is jammed
	if( vehicle->jam_time > level.time )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
		}
		return;
	}

//	if( level.time < vehicle->drone_decision_time )	// used when forcing unlock
//		return;

	if( vehicle->enemy )
	{
		if( !visible(vehicle,vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( !infront(vehicle,vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
		if( VectorLength( testvec ) > range )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( vehicle->enemy->HASCAN & HASCAN_STEALTH )
		{
			if( vehicle->enemy->ONOFF & ONOFF_STEALTH )
			{
				vehicle->enemy->lockstatus = LS_NOLOCK;
				vehicle->enemy = NULL;
				return;
			}
		}
		if( level.time < vehicle->enemy->jam_time )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		// test if within the target-cone
		AngleVectors( angle, forward, NULL, NULL );
		VectorNormalize( forward );
		VectorNormalize( testvec );
		test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
				testvec[2]*forward[2] );
		test *= (180/M_PI);
		if( test > 15 )
		{
			vehicle->enemy->lockstatus = LS_NOLOCK;
			vehicle->enemy = NULL;
			return;
		}
		if( teamplay->value != 1 )
		{
			if( !vehicle->enemy->lockstatus )
				vehicle->enemy->lockstatus = LS_LOCK;
		}
		else
		{
			if( vehicle->enemy->HASCAN & HASCAN_DRONE )	// don't screw up drones 
			{
				if( vehicle->enemy->aqteam != vehicle->aqteam )
				{
					if( !vehicle->enemy->lockstatus )
						vehicle->enemy->lockstatus = LS_LOCK;
				}
			}
			else
			{
				if( !vehicle->enemy->lockstatus )
					vehicle->enemy->lockstatus = LS_LOCK;
			}
		}
		return;
	}

	if( !vehicle->enemy )
	{
		while( (lock = findradius( lock, vehicle->s.origin, range ) ) != NULL )
		{
			if( lock == vehicle )
				continue;
			if( strcmp( lock->classname, "ground" ) != 0 )
				continue;
			if( vehicle->owner )
				if( lock == vehicle->owner )
					continue;
			if( !lock->takedamage )
				continue;
			if( lock->deadflag )
				continue;
			if( strcmp(vehicle->classname, "plane") == 0 ||
				strcmp(vehicle->classname, "helo") == 0 ||
				strcmp(vehicle->classname, "LQM") == 0 )
			{
				if( !visible(vehicle,lock) )
					continue;
				if( !infront(vehicle, lock) )
					continue;
			}
			else if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				if( !visible(vehicle->movetarget,lock) )
					continue;
				if( !infront(vehicle->movetarget,lock) )
					continue;
			}
			// stealth
			if( lock->HASCAN & HASCAN_STEALTH )
			{
				if( lock->ONOFF & ONOFF_STEALTH )
					continue;
			}
			// jammer
			if( level.time < lock->jam_time )
				continue;
			// test if within the target-cone
			if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				AngleVectors( vehicle->movetarget->s.angles, forward, NULL, NULL );
			}
			else
			{
				AngleVectors( angle, forward, NULL, NULL );
			}
			VectorSubtract( lock->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > 8 )
				continue;
			vehicle->enemy = lock;
			if( teamplay->value != 1 )
				lock->lockstatus = LS_LOCK;
			else
			{
				if( vehicle->enemy->HASCAN & HASCAN_DRONE )
				{
					if( vehicle->enemy->aqteam != vehicle->aqteam )
						lock->lockstatus = LS_LOCK;
				}
				else
					lock->lockstatus = LS_LOCK;
			}
			break;
		}
	}
}

void UnlockEnemyOnDie( edict_t *vehicle )
{
	if( !vehicle->lockstatus )
		return;
	else
	{
		int i;
		edict_t *testent;

		testent = &g_edicts[0];
		for (i=0 ; i<globals.num_edicts ; i++, testent++)
		{
			if( testent->enemy )
			{
				if( testent->enemy == vehicle )
				{
					testent->enemy = NULL;
					if( strcmp( testent->classname, "Sidewinder" ) == 0 ||
						strcmp( testent->classname, "ATGM" ) == 0 )
						testent->think = nolockthink;
					else if( strcmp( testent->classname, "LGB" ) == 0 )
						testent->think = bomb_think;
				}
			}
		}
	}
}