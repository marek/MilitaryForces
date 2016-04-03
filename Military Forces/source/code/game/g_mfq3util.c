/*
 * $Id: g_mfq3util.c,v 1.7 2006-01-29 14:03:41 thebjoern Exp $
*/



#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"



int canLandOnIt( GameEntity *ent )
{
	if( strcmp( ent->classname_, "func_runway" ) == 0 ||
		strcmp( ent->classname_, "func_plat" ) == 0 ||
		strcmp( ent->classname_, "func_train" ) == 0 ||
		strcmp( ent->classname_, "func_door" ) == 0 ) {
		return 1;
	}
	return 0;
}

void unlock( GameEntity* ent )
{
	// update target
	if( ent->tracktarget_ && ent->tracktarget_->client_ ) 
		ent->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LOCKED;

	ent->locktime_ = theLevel.time_;
	ent->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_LOCKING;
}

void untrack( GameEntity* ent )
{
	// update target
	if( ent->tracktarget_ && ent->tracktarget_->client_ ) 
		ent->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LOCKED;

	ent->locktime_ = theLevel.time_;
	ent->tracktarget_ = 0;
	ent->s.tracktarget = ENTITYNUM_NONE;
	ent->client_->ps_.stats[STAT_LOCKINFO] &= ~(LI_TRACKING|LI_LOCKING);
	ent->client_->ps_.tracktarget = ENTITYNUM_NONE;
}

void track( GameEntity* ent, GameEntity* target )
{
	ent->tracktarget_ = target;
	ent->locktime_ = theLevel.time_;
	ent->s.tracktarget = ent->tracktarget_->s.number;
	ent->client_->ps_.stats[STAT_LOCKINFO] |= LI_TRACKING;
	ent->client_->ps_.tracktarget = ent->tracktarget_->s.number;
}

void updateTargetTracking( GameEntity *ent )
{
	unsigned long	targetcat = 0;
	trace_t			tr;
	vec3_t			forward, endpos, dir;
	float			radarrange = 0;
	bool			buildings = false, groundinstallations = false;
	GameEntity		*test;
	float			cone = 0.0f;
	
//	Com_Printf( "updateTargetTracking for %s\n", ent->client->pers.netname );

	// what can we lock on ?
	if(	ent->client_->ps_.ONOFF & OO_RADAR_AIR )
	{
		targetcat = CAT_PLANE|CAT_HELO;
		radarrange = availableVehicles[ent->client_->vehicle_].radarRange;
		cone = availableVehicles[ent->client_->vehicle_].trackCone;
	} 
	else if( ent->client_->ps_.ONOFF & OO_RADAR_GROUND ) 
	{
		targetcat = CAT_GROUND|CAT_BOAT;
		buildings = true;
		groundinstallations = true;
		radarrange = availableVehicles[ent->client_->vehicle_].radarRange2;
		cone = availableVehicles[ent->client_->vehicle_].trackCone2;
	}
	
	if( !targetcat ) 
	{
		if( ent->client_->ps_.stats[STAT_LOCKINFO] ) 
			untrack(ent);
		return;
	}

	// weapon range and vehicle direction
	if( (availableVehicles[ent->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[ent->client_->vehicle_].cat & CAT_BOAT)) 
	{
		vec3_t	right, up, temp;
		AngleVectors( ent->client_->ps_.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ent->client_->ps_.turretAngle );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ent->client_->ps_.gunAngle );
		VectorCopy( dir, forward );
	} 
	else
	{
		VectorCopy( ent->s.angles, dir );
		AngleVectors( dir, forward, 0, 0 );
	}

	// no target yet
	if( !ent->tracktarget_ ) 
	{
		VectorMA( ent->r.currentOrigin, radarrange, forward, endpos );
		SV_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, endpos, ent->s.number, MASK_ALL, false );
		// nothing found
		if( tr.entityNum == ENTITYNUM_NONE ) 
			return;

		// found something
		test = theLevel.getEntity(tr.entityNum);// &g_entities[tr.entityNum];
		if( test->s.eType == ET_VEHICLE && test->client_ && 
			(availableVehicles[test->client_->vehicle_].cat & targetcat) ) 
		{
			track(ent, test);
		} 
		else if( test->s.eType == ET_MISC_VEHICLE )
		{
			if( (test->s.modelindex == 255 && groundinstallations) || // ground installations
			    (availableVehicles[test->s.modelindex].cat & targetcat) ) 
				track(ent, test);
		} 
		else
		{
			if( buildings ) 
			{
				if( test->s.eType == ET_EXPLOSIVE && test->takedamage_ ) 
					track(ent, test);
			}
			else 
			{
				return;
			}
		}
	} 
	else 
	{ 
		// update existing target
		vec3_t diff;
		float dot, dist;
		int actualcat = 0;

		if( ent->tracktarget_->s.eType == ET_EXPLOSIVE ) 
		{
			vec3_t	mid;
			actualcat = CAT_GROUND;
			VectorAdd( ent->tracktarget_->r.absmax, ent->tracktarget_->r.absmin, mid );
			VectorScale( mid, 0.5f, mid );
			VectorSubtract( mid, ent->r.currentOrigin, diff );
		} 
		else 
		{
			if( ent->tracktarget_->s.eType == ET_MISC_VEHICLE )
			{
				if( ent->tracktarget_->s.modelindex == 255 )// groundinstallation
				{
					actualcat = CAT_GROUND;
					groundinstallations = true;
				}
				else
					actualcat = availableVehicles[ent->tracktarget_->s.modelindex].cat;
			} 
			else if( ent->tracktarget_->s.eType == ET_VEHICLE && ent->tracktarget_->client_ ) 
			{
				actualcat =	availableVehicles[ent->tracktarget_->client_->vehicle_].cat;
			}
			VectorSubtract( ent->tracktarget_->r.currentOrigin, ent->r.currentOrigin, diff );
		}
		if( !actualcat ) 
		{
			untrack(ent);
			return;
		}
			
		// check within range
		dist = VectorNormalize( diff );
		if( dist > radarrange )
		{
			untrack(ent);
			return;
		}
		// check within cone
		dot = DotProduct( forward, diff );
		if( dot < cone ) 
		{
			untrack(ent);
			return;
		} 
		else if( dot < availableWeapons[ent->s.weaponIndex].lockcone ) 
		{
			if( ent->client_->ps_.stats[STAT_LOCKINFO] & LI_LOCKING )	
				unlock(ent);
			ent->locktime_ = theLevel.time_;
			return;
		}
			
		// check LOS
		VectorMA( ent->r.currentOrigin, dist, diff, endpos );
		SV_Trace( &tr, ent->r.currentOrigin, 0, 0, endpos, ent->s.number, MASK_PLAYERSOLID, false );
		if( tr.fraction < 1 && tr.entityNum != ent->s.tracktarget ) 
		{
			untrack(ent);
			return;
		}

		// only weapons that can lock on
		if( availableWeapons[ent->s.weaponIndex].type != WT_GUIDEDBOMB &&
			availableWeapons[ent->s.weaponIndex].type != WT_ANTIAIRMISSILE &&
			availableWeapons[ent->s.weaponIndex].type != WT_ANTIGROUNDMISSILE &&
			availableWeapons[ent->s.weaponIndex].type != WT_ANTIRADARMISSILE )
		{
			//untrack(ent);
			if( ent->client_->ps_.stats[STAT_LOCKINFO] & LI_LOCKING )
				unlock(ent);
			return;
		}

		// what can we lock on ?
		targetcat = 0;
		if(	availableWeapons[ent->s.weaponIndex].type == WT_ANTIAIRMISSILE &&
			(ent->client_->ps_.ONOFF & OO_RADAR_AIR) ) 
		{
			targetcat = CAT_PLANE|CAT_HELO;
		} 
		else if( availableWeapons[ent->s.weaponIndex].type != WT_ANTIAIRMISSILE &&
			(ent->client_->ps_.ONOFF & OO_RADAR_GROUND) )
		{
			targetcat = CAT_GROUND|CAT_BOAT;
			buildings = true;
		}
		if( !targetcat ) 
		{
			unlock(ent);
			//untrack(ent);
			return;
		} 
		else if( !(targetcat & actualcat) ) 
		{
			unlock(ent);
			return;			
		}
		// check time
		if( theLevel.time_ > ent->locktime_ + availableWeapons[ent->s.weaponIndex].lockdelay )
		{
			ent->client_->ps_.stats[STAT_LOCKINFO] |= LI_LOCKING;
		}
		// update target
		if( ent->tracktarget_->client_ && (ent->client_->ps_.stats[STAT_LOCKINFO] & LI_LOCKING) )
		{
			ent->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] |= LI_BEING_LOCKED;
		}
	}
}



