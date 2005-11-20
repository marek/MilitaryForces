/*
 * $Id: g_mfq3util.c,v 1.5 2005-11-20 11:21:38 thebjoern Exp $
*/



#include "g_local.h"




int canLandOnIt( gentity_t *ent )
{
	if( strcmp( ent->classname, "func_runway" ) == 0 ||
		strcmp( ent->classname, "func_plat" ) == 0 ||
		strcmp( ent->classname, "func_train" ) == 0 ||
		strcmp( ent->classname, "func_door" ) == 0 ) {
		return 1;
	}
	return 0;
}

void unlock( gentity_t* ent )
{
	// update target
	if( ent->tracktarget && ent->tracktarget->client ) {
		ent->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LOCKED;
	}
	ent->locktime = level.time;
	ent->client->ps.stats[STAT_LOCKINFO] &= ~LI_LOCKING;
}

void untrack( gentity_t* ent )
{
	// update target
	if( ent->tracktarget && ent->tracktarget->client ) {
		ent->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LOCKED;
	}
	ent->locktime = level.time;
	ent->tracktarget = 0;
	ent->s.tracktarget = ENTITYNUM_NONE;
	ent->client->ps.stats[STAT_LOCKINFO] &= ~(LI_TRACKING|LI_LOCKING);
	ent->client->ps.tracktarget = ENTITYNUM_NONE;
}

void track( gentity_t* ent, gentity_t* target )
{
	ent->tracktarget = target;
	ent->locktime = level.time;
	ent->s.tracktarget = ent->tracktarget->s.number;
	ent->client->ps.stats[STAT_LOCKINFO] |= LI_TRACKING;
	ent->client->ps.tracktarget = ent->tracktarget->s.number;
}

void updateTargetTracking( gentity_t *ent )
{
	unsigned long	targetcat = 0;
	trace_t			tr;
	vec3_t			forward, endpos, dir;
	float			radarrange = 0;
	bool		buildings = false, groundinstallations = false;
	gentity_t		*test;
	float			cone = 0.0f;
	
//	Com_Printf( "updateTargetTracking for %s\n", ent->client->pers.netname );

	// what can we lock on ?
	if(	ent->client->ps.ONOFF & OO_RADAR_AIR ) {
		targetcat = CAT_PLANE|CAT_HELO;
		radarrange = availableVehicles[ent->client->vehicle].radarRange;
		cone = availableVehicles[ent->client->vehicle].trackCone;
	} else if( ent->client->ps.ONOFF & OO_RADAR_GROUND ) {
		targetcat = CAT_GROUND|CAT_BOAT;
		buildings = true;
		groundinstallations = true;
		radarrange = availableVehicles[ent->client->vehicle].radarRange2;
		cone = availableVehicles[ent->client->vehicle].trackCone2;
	}
	
	if( !targetcat ) {
		if( ent->client->ps.stats[STAT_LOCKINFO] ) untrack(ent);
		return;
	}

	// weapon range and vehicle direction
	if( (availableVehicles[ent->client->vehicle].cat & CAT_GROUND) ||
		(availableVehicles[ent->client->vehicle].cat & CAT_BOAT)) {
		vec3_t	right, up, temp;
		AngleVectors( ent->client->ps.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ent->client->ps.turretAngle );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ent->client->ps.gunAngle );
		VectorCopy( dir, forward );
	} else {
		VectorCopy( ent->s.angles, dir );
		AngleVectors( dir, forward, 0, 0 );
	}


	// no target yet
	if( !ent->tracktarget ) {
		VectorMA( ent->r.currentOrigin, radarrange, forward, endpos );
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, endpos, ent->s.number, MASK_ALL );
		// nothing found
		if( tr.entityNum == ENTITYNUM_NONE ) {
			return;
		}
		// found something
		test = &g_entities[tr.entityNum];
		if( test->s.eType == ET_VEHICLE && test->client && 
			(availableVehicles[test->client->vehicle].cat & targetcat) ) {
			track(ent, test);
		} else if( test->s.eType == ET_MISC_VEHICLE ) {
			if( (test->s.modelindex == 255 && groundinstallations) || // ground installations
			    (availableVehicles[test->s.modelindex].cat & targetcat) ) 
				track(ent, test);
		} else {
			if( buildings ) {
				if( test->s.eType == ET_EXPLOSIVE && test->takedamage ) {
					track(ent, test);
				}
			} else {
				return;
			}
		}
	} else { // update existing target
		vec3_t diff;
		float dot, dist;
		int actualcat = 0;

		if( ent->tracktarget->s.eType == ET_EXPLOSIVE ) {
			vec3_t	mid;
			actualcat = CAT_GROUND;
			VectorAdd( ent->tracktarget->r.absmax, ent->tracktarget->r.absmin, mid );
			VectorScale( mid, 0.5f, mid );
			VectorSubtract( mid, ent->r.currentOrigin, diff );
		} else {
			if( ent->tracktarget->s.eType == ET_MISC_VEHICLE ) {
				if( ent->tracktarget->s.modelindex == 255 )// groundinstallation
				{
					actualcat = CAT_GROUND;
					groundinstallations = true;
				}
				else
					actualcat = availableVehicles[ent->tracktarget->s.modelindex].cat;
			} else if( ent->tracktarget->s.eType == ET_VEHICLE && ent->tracktarget->client ) {
				actualcat =	availableVehicles[ent->tracktarget->client->vehicle].cat;
			}
			VectorSubtract( ent->tracktarget->r.currentOrigin, ent->r.currentOrigin, diff );
		}
		if( !actualcat ) {
			untrack(ent);
			return;
		}
			
		// check within range
		dist = VectorNormalize( diff );
		if( dist > radarrange ) {
			untrack(ent);
			return;
		}
		// check within cone
		dot = DotProduct( forward, diff );
		if( dot < cone ) {
			untrack(ent);
			return;
		} else if( dot < availableWeapons[ent->s.weaponIndex].lockcone ) {
			if( ent->client->ps.stats[STAT_LOCKINFO] & LI_LOCKING )	
				unlock(ent);
			ent->locktime = level.time;
			return;
		}
			
		// check LOS
		VectorMA( ent->r.currentOrigin, dist, diff, endpos );
		trap_Trace( &tr, ent->r.currentOrigin, 0, 0, endpos, ent->s.number, MASK_PLAYERSOLID  );
		if( tr.fraction < 1 && tr.entityNum != ent->s.tracktarget ) {
			untrack(ent);
			return;
		}

		// only weapons that can lock on
		if( availableWeapons[ent->s.weaponIndex].type != WT_GUIDEDBOMB &&
			availableWeapons[ent->s.weaponIndex].type != WT_ANTIAIRMISSILE &&
			availableWeapons[ent->s.weaponIndex].type != WT_ANTIGROUNDMISSILE &&
			availableWeapons[ent->s.weaponIndex].type != WT_ANTIRADARMISSILE ) {
			//untrack(ent);
			if( ent->client->ps.stats[STAT_LOCKINFO] & LI_LOCKING ) unlock(ent);
			return;
		}

		// what can we lock on ?
		targetcat = 0;
		if(	availableWeapons[ent->s.weaponIndex].type == WT_ANTIAIRMISSILE &&
			(ent->client->ps.ONOFF & OO_RADAR_AIR) ) {
			targetcat = CAT_PLANE|CAT_HELO;
		} else if( availableWeapons[ent->s.weaponIndex].type != WT_ANTIAIRMISSILE &&
			(ent->client->ps.ONOFF & OO_RADAR_GROUND) ) {
			targetcat = CAT_GROUND|CAT_BOAT;
			buildings = true;
		}
		if( !targetcat ) {
			unlock(ent);
			//untrack(ent);
			return;
		} else if( !(targetcat & actualcat) ) {
			unlock(ent);
			return;			
		}
		// check time
		if( level.time > ent->locktime + availableWeapons[ent->s.weaponIndex].lockdelay ) {
			ent->client->ps.stats[STAT_LOCKINFO] |= LI_LOCKING;
		}
		// update target
		if( ent->tracktarget->client && (ent->client->ps.stats[STAT_LOCKINFO] & LI_LOCKING) ) {
			ent->tracktarget->client->ps.stats[STAT_LOCKINFO] |= LI_BEING_LOCKED;
		}
	}
}



