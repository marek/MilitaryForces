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

Vec3 lockPoint( const GameEntity* target )
{
	// the middle of the target, not its origin: a vehicle's origin is at its underside, so a line drawn to it from above
	// meets the ground the vehicle stands on
	return ( Vec3( target->r.absmin ) + Vec3( target->r.absmax ) ) * 0.5f;
}

// Whether the line from the seeker to the target's middle is clear. The line counts as clear once it is inside the
// target's own box: the ground it stands on reaches up into that box, and on a slope or a bump it meets the line there
static bool seesTarget( const GameEntity* ent, const GameEntity* target, int mask )
{
	constexpr float margin = 1.0f;
	trace_t tr;
	GameHost::trace( &tr, ent->r.currentOrigin, nullptr, nullptr, lockPoint( target ), ent->s.number, mask, false );
	if( tr.fraction >= 1.0f || tr.entityNum == target->s.number )
		return true;
	for( int i = 0; i < 3; i++ )
		if( tr.endpos[i] < target->r.absmin[i] - margin || tr.endpos[i] > target->r.absmax[i] + margin )
			return false;
	return true;
}

void untrack( GameEntity* ent )
{
	// update target
	if( ent->tracktarget_ && ent->tracktarget_->client_ ) 
		ent->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LOCKED;

	ent->locktime_ = theLevel.time_;
	ent->tracktarget_ = nullptr;
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
	if( target->client_ && target->client_->ruseBlown_ ) 
		target->client_->ruseSeenTime_ = theLevel.time_;
}

void updateTargetTracking( GameEntity *ent )
{

	unsigned long	targetcat = 0;
	trace_t			tr;
	Vec3			forward, endpos, dir;
	float			radarrange = 0;
	bool			buildings = false, groundinstallations = false;
	GameEntity		*test;
	float			cone = 0.0f;
	
//	Common::printf( "updateTargetTracking for %s\n", ent->client->pers.netname );

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
		Vec3	right, up, temp;
		AngleVectors( ent->client_->ps_.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ent->client_->ps_.turretAngle );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ent->client_->ps_.gunAngle );
		forward = dir;
	} 
	else if( availableVehicles[ent->client_->vehicle_].cat & CAT_HELO )
	{
		// a helicopter does not point its nose at what it attacks the way an aeroplane does: its seeker looks where
		// the pilot looks
		VectorCopy( ent->client_->ps_.viewangles, dir );
		AngleVectors( dir, forward, nullptr, nullptr );
	}
	else
	{
		VectorCopy( ent->s.angles, dir );
		AngleVectors( dir, forward, nullptr, nullptr );
	}
	// An aircraft's pilot looks along that line from the camera, which sits above the aircraft, not from its middle.
	// Close to a target the gap between the two lines is several degrees, enough to miss a tank the pilot has in the
	// middle of the screen
	Vec3 eye = ent->r.currentOrigin;
	if( availableVehicles[ent->client_->vehicle_].cat & (CAT_HELO|CAT_PLANE) )
		eye[2] += availableVehicles[ent->client_->vehicle_].cam_height[CAMERA_V_DEFAULT];

	// no target yet
	if( !ent->tracktarget_ ) 
	{
		// Ground targets first, by the line of sight: the target it passes through, or the one nearest it within three
		// degrees. A sweep of the whole hull along that line, which is what follows, strikes the ground beside a target
		// on it long before it reaches the target, and close in a target is wider than any narrow cone about its middle
		if( groundinstallations )
		{
			const float acquireTan = tanf( DEG2RAD( 3.0f ) );
			GameEntity* found = nullptr;
			float best = 0;
			for( int i = 1; i < theLevel.num_entities_; i++ )
			{
				GameEntity* other = theLevel.getEntity( i );
				if( !other || other == ent || !other->inuse_ || other->health_ <= 0 || OnSameSide( ent, other ) )
					continue;
				if( other->s.eType == ET_VEHICLE && other->client_ )
				{
					if( other->client_->vehicle_ < 0 || !(availableVehicles[other->client_->vehicle_].cat & targetcat) || other->client_->sess_.ridingClient_ >= 0 )
						continue;
				}
				else if( other->s.eType == ET_MISC_VEHICLE )
				{
					if( other->s.modelindex != 255 && !(availableVehicles[other->s.modelindex].cat & targetcat) )
						continue;
				}
				else
					continue;
				const Vec3 aim = lockPoint( other );
				const float dist = Distance( ent->r.currentOrigin, aim );
				const Vec3 to = aim - eye;
				const float along = DotProduct( forward, to );
				if( dist > radarrange || along <= 0 )
					continue;
				const float miss = VectorLength( to - forward * along );
				const float size = 0.5f * Distance( other->r.absmin, other->r.absmax );
				if( miss > std::max( along * acquireTan, size ) || ( found && miss / along >= best ) )
					continue;
				if( other->client_ && VehicleData::stealthHidden( other->client_->vehicle_, other->client_->ps_.timers[TIMER_REVEAL], theLevel.time_, dist, radarrange ) )
					continue;
				if( !seesTarget( ent, other, MASK_SOLID ) )
					continue;
				found = other;
				best = miss / along;
			}
			if( found )
			{
				track( ent, found );
				return;
			}
		}
		endpos = ent->r.currentOrigin + forward * ( radarrange );
		GameHost::trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, endpos, ent->s.number, MASK_ALL, false );
		// nothing found
		if( tr.entityNum == ENTITYNUM_NONE ) 
			return;

		// found something; a friend is never a radar target, so it never stands between the player and a lock
		test = theLevel.getEntity(tr.entityNum);// &g_entities[tr.entityNum];
		if( OnSameSide( ent, test ) )
			return;
		if( test->s.eType == ET_VEHICLE && test->client_ && 
			(availableVehicles[test->client_->vehicle_].cat & targetcat) ) 
		{
			if( VehicleData::stealthHidden( test->client_->vehicle_, test->client_->ps_.timers[TIMER_REVEAL], theLevel.time_,
					Distance( ent->r.currentOrigin, test->r.currentOrigin ), radarrange ) ) 
				return;
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
		Vec3 diff;
		float dot, dist;
		int actualcat = 0;

		if( ent->tracktarget_->s.eType == ET_EXPLOSIVE ) 
		{
			actualcat = CAT_GROUND;
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
		}
		diff = lockPoint( ent->tracktarget_ ) - Vec3( ent->r.currentOrigin );
		if( !actualcat || OnSameSide( ent, ent->tracktarget_ ) ) 
		{
			untrack(ent);
			return;
		}
			
		if( ent->tracktarget_->client_ && ent->tracktarget_->client_->ruseBlown_ ) 
			ent->tracktarget_->client_->ruseSeenTime_ = theLevel.time_;

		// check within range
		dist = VectorNormalize( diff );
		if( dist > radarrange )
		{
			untrack(ent);
			return;
		}
		if( ent->tracktarget_->s.eType == ET_VEHICLE && ent->tracktarget_->client_ &&
			VehicleData::stealthHidden( ent->tracktarget_->client_->vehicle_, ent->tracktarget_->client_->ps_.timers[TIMER_REVEAL],
				theLevel.time_, dist, radarrange ) ) 
		{
			untrack(ent);
			return;
		}
		// check within cone, seen from where the pilot looks
		Vec3 look = lockPoint( ent->tracktarget_ ) - eye;
		VectorNormalize( look );
		dot = DotProduct( forward, look );
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
		if( !seesTarget( ent, ent->tracktarget_, MASK_PLAYERSOLID ) )
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
		// a lock needs the target inside the selected weapon's own reach, not only the radar's:
		// a missile launched at something beyond it loses the target at once
		const float weaponRange = static_cast<float>( availableWeapons[ent->s.weaponIndex].range );
		if( weaponRange > 0 && dist > weaponRange )
		{
			if( ent->client_->ps_.stats[STAT_LOCKINFO] & LI_LOCKING )
				unlock(ent);
			ent->locktime_ = theLevel.time_;
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



