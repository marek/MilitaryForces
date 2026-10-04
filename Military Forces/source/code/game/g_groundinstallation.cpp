/*
 * $Id: g_groundinstallation.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
*/

#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"
#include "g_miscvehicle.h"


static void loselock( GameEntity* ent )
{
	// update target
	if( ent->tracktarget_ && ent->tracktarget_->client_ ) {
		ent->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LOCKED;
	}
	ent->locktime_ = 0;
	ent->tracktarget_ = nullptr;
	ent->s.tracktarget = ENTITYNUM_NONE;
	ent->gi_nextScanTime_ = theLevel.time_ + 1500;
	ent->gi_reloadTime_ = theLevel.time_ + availableGroundInstallations[ent->s.modelindex2].reloadTime;
}


void GroundInstallation_Die::execute( GameEntity *inflictor, GameEntity *attacker, int damage, int meansOfDeath )
{
	loselock(self_);

	if( self_->score_ && attacker && attacker->client_ ) {
		vec3_t	pos;
		int score;
		VectorAdd( self_->r.absmin, self_->r.absmax, pos );
		VectorScale( pos, 0.5f, pos );
		if( g_gametype.integer >= GT_TEAM && self_->s.generic1 == attacker->client_->ps_.persistant[PERS_TEAM] ) {
			score = -self_->score_;
		} else {
			score = self_->score_;
		}

		AddScore( attacker, pos, score );
	}

	ExplodeVehicle(self_);
	self_->freeAfterEvent_ = true;

	G_RadiusDamage( self_->r.currentOrigin, self_, 150, 150, self_, MOD_VEHICLEEXPLOSION, CAT_ANY );

	SV_LinkEntity( self_ );
}

static void Update_GI_Targets( GameEntity* ent )
{
	vec3_t			mins, maxs, rangevec, dir;
	float			range = availableGroundInstallations[ent->s.modelindex2].radarRange;
	int				i, num, touch[MAX_GENTITIES];
	GameEntity		*hit, *best = nullptr;
	float			dist, closest = 999999;
	unsigned long	cat;
	trace_t			tr;

	// anti air missiles
	if( ent->count_ > 0 &&
		ent->s.weaponIndex > -1 &&
			(	(availableWeapons[ent->s.weaponIndex].type == WT_ANTIAIRMISSILE)
				|| (availableWeapons[ent->s.weaponIndex].type == WT_FLAK)
				|| (availableWeapons[ent->s.weaponIndex].type == WT_MACHINEGUN)
			)
		)
	{
		// no target yet
		if( !ent->tracktarget_ )
		{
			if( theLevel.time_ > ent->gi_nextScanTime_ )
			{
				ent->gi_nextScanTime_ = theLevel.time_ + 1500;
				VectorSet( rangevec, range, range, range );
				VectorSubtract( ent->r.currentOrigin, rangevec, mins );
				VectorAdd( ent->r.currentOrigin, rangevec, maxs );
				num = SV_AreaEntities( mins, maxs, touch, MAX_GENTITIES );

				for ( i=0 ; i<num ; i++ )
				{
					hit = theLevel.getEntity(touch[i]);
					// alive ?
					if( !hit || !hit->inuse_ || hit->health_ <= 0 ) continue;
					// determine category
					if( hit->s.eType == ET_VEHICLE && hit->client_ )
						cat = availableVehicles[hit->client_->vehicle_].cat;
					else if( hit->s.eType == ET_MISC_VEHICLE &&
							 hit->s.modelindex < 255 )	// not other sams
						cat = availableVehicles[hit->s.modelindex].cat;
					else continue;
					// wrong category
					if( !(cat & CAT_PLANE) && !(cat & CAT_HELO) ) continue;
					// parked
					if( !hit->client_ && (hit->s.ONOFF & OO_LANDED) ) continue;
					// same team
					if( g_gametype.integer >= GT_TEAM )
					{
						int team = hit->client_ ? hit->client_->sess_.sessionTeam_ : hit->s.generic1;
						if( team == ent->s.generic1 ) continue;
					}
					// check LOS
					SV_Trace( &tr, ent->r.currentOrigin, nullptr, nullptr, hit->r.currentOrigin, ent->s.number, MASK_PLAYERSOLID, false );
					if( tr.fraction < 1 && tr.entityNum != hit->s.number ) continue;
					// check dist
					VectorSubtract(hit->r.currentOrigin, ent->r.currentOrigin, dir);
					dist = VectorNormalize(dir);
					if( !best || dist < closest )
					{
						best = hit;
						closest = dist;
					}
				}
				if( best )
				{
					ent->gi_lockangle_ = acos(static_cast<double>(availableWeapons[ent->s.weaponIndex].lockcone));
					ent->gi_lockangle_ = RAD2DEG(ent->gi_lockangle_);
					ent->tracktarget_ = best;
					ent->locktime_ = theLevel.time_;
					ent->s.tracktarget = best->s.number;
					if( ent->tracktarget_->client_ )
						ent->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] |= LI_BEING_LOCKED;
				}
				else
					ent->tracktarget_ = nullptr;
			}
		}
		// fight current target
		else
		{
			// check still exists
			if( !ent->tracktarget_->inuse_ ||
				ent->tracktarget_->health_ <= 0 )
			{
				loselock(ent);
				return;
			}
			// armed ?
			if( ent->count_ <= 0 )
			{
				loselock(ent);
				return;
			}
			// check distance
			VectorSubtract(ent->tracktarget_->r.currentOrigin, ent->r.currentOrigin, dir);
			if( VectorLength(dir) > range )
			{
				loselock(ent);
				return;
			}
			// check LOS
			SV_Trace( &tr, ent->r.currentOrigin, nullptr, nullptr, ent->tracktarget_->r.currentOrigin, ent->s.number, MASK_PLAYERSOLID, false );
			if( tr.fraction < 1 && tr.entityNum != ent->tracktarget_->s.number )
			{
				loselock(ent);
				return;
			}
		}
	}
	else if( ent->tracktarget_ )
	{
		loselock(ent);
	}
}

void GroundInstallation_Think::execute()
{
	GameEntity* ent = self_;
	float	diff, turnspeed;
	float	timediff = theLevel.time_ - ent->s.pos.trTime;
	vec3_t	targdir, targangles;
	int		locktime;

	Update_GI_Targets(ent);

	if( ent->tracktarget_ )
	{
		VectorSubtract(ent->tracktarget_->r.currentOrigin, ent->r.currentOrigin, targdir);
		vectoangles(targdir, targangles);
		targangles[1] -= 90;

		// clamp target
		if( targangles[1] >= 360 ) targangles[1] -= 360;
		else if( targangles[1] < 0 ) targangles[1] += 360;
		if( targangles[0] < 90 &&
			targangles[0] > availableGroundInstallations[ent->s.modelindex2].minpitch )
			targangles[0] = availableGroundInstallations[ent->s.modelindex2].minpitch;
		else if( targangles[0] > 270 &&
			targangles[0] < availableGroundInstallations[ent->s.modelindex2].maxpitch )
			targangles[0] = availableGroundInstallations[ent->s.modelindex2].maxpitch;

		// turn
		diff = targangles[1] - ent->s.angles2[ROLL];
		if( diff > 180 ) diff -= 360;
		else if( diff < -180 ) diff += 360;
		turnspeed = availableGroundInstallations[ent->s.modelindex2].turnspeed[1] * timediff / 1000;
		if( turnspeed < fabs(diff) )
		{
			if( diff > 0 )
				ent->s.angles2[ROLL] += turnspeed;
			else
				ent->s.angles2[ROLL] -= turnspeed;
		}
		else
		{
			ent->s.angles2[ROLL] = targangles[1];
		}
		// only fire when the target is inside the lock cone on either side
		if( fabs(diff) <= ent->gi_lockangle_ )
		{
			if( ent->tracktarget_->client_ )
			{
				if( ent->tracktarget_->client_->ps_.ONOFF & OO_RADAR )
					locktime = availableWeapons[ent->s.weaponIndex].lockdelay/2;
				else
					locktime = availableWeapons[ent->s.weaponIndex].lockdelay;
			}
			else
			{
				if( ent->s.ONOFF & OO_RADAR )
					locktime = availableWeapons[ent->s.weaponIndex].lockdelay/2;
				else
					locktime = availableWeapons[ent->s.weaponIndex].lockdelay;
			}
			if( theLevel.time_ > ent->locktime_ + locktime && Distance(ent->r.currentOrigin,ent->tracktarget_->r.currentOrigin) <= availableWeapons[ent->s.weaponIndex].range)
			{
				FireWeapon_GI(ent);
				ent->locktime_ = theLevel.time_ + availableWeapons[ent->s.weaponIndex].fireInterval;	// so it doesnt launch all of them instantly
			}
		}
		else
			ent->locktime_ = theLevel.time_;
	}
	else if( theLevel.time_ > ent->gi_reloadTime_ &&
			 ent->count_ < availableGroundInstallations[ent->s.modelindex2].ammo )
	{
		ent->count_++;
		ent->gi_reloadTime_ = theLevel.time_ + availableGroundInstallations[ent->s.modelindex2].reloadTime;
	}

	ent->nextthink_ = theLevel.time_ + 50;
	ent->s.pos.trTime = theLevel.time_;
	SV_LinkEntity (ent);
}
