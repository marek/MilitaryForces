/*
 * $Id: g_groundinstallation.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
*/

#include "g_local.h"

//
//static void loselock( gentity_t* ent )
//{
//	// update target
//	if( ent->tracktarget && ent->tracktarget->client ) {
//		ent->tracktarget->client->ps.stats[STAT_LOCKINFO] &= ~LI_BEING_LOCKED;
//	}
//	ent->locktime = 0;
//	ent->tracktarget = 0;
//	ent->s.tracktarget = ENTITYNUM_NONE;
//	ent->gi_nextScanTime = level.time + 1500;
//	ent->gi_reloadTime = level.time + availableGroundInstallations[ent->s.modelindex2].reloadTime;
//}


//void groundinstallation_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
//{
//	loselock(self);
//
//	if( self->score && attacker && attacker->client ) {
//		vec3_t	pos;
//		int score;
//		VectorAdd( self->r.absmin, self->r.absmax, pos );
//		VectorScale( pos, 0.5f, pos );
//		if( g_gametype.integer >= GT_TEAM ) {
//			if( self->s.generic1 == attacker->client->ps.persistant[PERS_TEAM] ) 
//				score = -self->score;
//			else 
//				score = self->score;
//		} else {
//			score = self->score;
//		}
//
//		AddScore( attacker, pos, score );
//	}
//
//	ExplodeVehicle(self);
//	self->freeAfterEvent = true;
//
//	G_RadiusDamage( self->r.currentOrigin, self, 150, 150, self, MOD_VEHICLEEXPLOSION, CAT_ANY );
//
//	SV_LinkEntity( &self->s, &self->r );
//	
//}

//static void Update_GI_Targets( gentity_t* ent )
//{
//	vec3_t			mins, maxs, rangevec, dir;
//	float			range = availableGroundInstallations[ent->s.modelindex2].radarRange;
//	int				i, num, touch[MAX_GENTITIES];
//	gentity_t		*hit, *best = 0;
//	float			dist, closest = 999999;
//	unsigned long	cat;
//	trace_t			tr;
//
//	// anti air missiles
//	if( ent->count > 0 && 
//		ent->s.weaponIndex > -1 &&
//			(	(availableWeapons[ent->s.weaponIndex].type == WT_ANTIAIRMISSILE) 
//				|| (availableWeapons[ent->s.weaponIndex].type == WT_FLAK)
//				|| (availableWeapons[ent->s.weaponIndex].type == WT_MACHINEGUN)
//			) 
//		)
//	{
//		// no target yet
//		if( !ent->tracktarget )
//		{
//			if( level.time > ent->gi_nextScanTime )
//			{
//				ent->gi_nextScanTime = level.time + 1500;
//				VectorSet( rangevec, range, range, range );
//				VectorSubtract( ent->r.currentOrigin, rangevec, mins );
//				VectorAdd( ent->r.currentOrigin, rangevec, maxs );
//				num = SV_AreaEntities( mins, maxs, touch, MAX_GENTITIES );
//
//				//Com_Printf("GI scanning...%d\n", num);
//				for ( i=0 ; i<num ; i++ ) 
//				{
//					hit = &g_entities[touch[i]];
//					// alive ?
//					if( !hit->inuse || hit->health <= 0 ) continue;
//					// determine category
//					if( hit->s.eType == ET_VEHICLE && hit->client )
//						cat = availableVehicles[hit->client->vehicle].cat;
//					else if( hit->s.eType == ET_MISC_VEHICLE &&
//							 hit->s.modelindex < 255 )	// not other sams
//						cat = availableVehicles[hit->s.modelindex].cat;
//					else continue;
//					//Com_Printf("%d: %s %d", i, hit->classname, cat );
//					// wrong category
//					if( !(cat & CAT_PLANE) && !(cat & CAT_HELO) ) continue;
//					// check LOS
//					SV_Trace( &tr, ent->r.currentOrigin, 0, 0, hit->r.currentOrigin, ent->s.number, MASK_PLAYERSOLID, false );
//					if( tr.fraction < 1 && tr.entityNum != hit->s.number ) continue;
//					// check dist
//					VectorSubtract(hit->r.currentOrigin, ent->r.currentOrigin, dir);
//					dist = VectorNormalize(dir);
//					if( !best ||
//						(best && dist < closest) )
//					{
//						best = hit;
//						closest = dist;
//					}
//				}
//				if( best )
//				{
//					ent->gi_lockangle = acos((double)availableWeapons[ent->s.weaponIndex].lockcone);
//					ent->gi_lockangle = RAD2DEG(ent->gi_lockangle);
//					ent->tracktarget = best;
//					ent->locktime = level.time;
//					ent->s.tracktarget = best->s.number;
//					if( ent->tracktarget->client ) 
//						ent->tracktarget->client->ps.stats[STAT_LOCKINFO] |= LI_BEING_LOCKED;
//				}
//				else
//					ent->tracktarget = 0;
//			}
//		}
//		// fight current target
//		else
//		{
//			// check still exists
//			if( !ent->tracktarget->inuse ||
//				ent->tracktarget->health <= 0 )
//			{
//				loselock(ent);
//				return;
//			}
//			// armed ?
//			if( ent->count <= 0 )
//			{
//				loselock(ent);
//				return;
//			}
//			// check distance
//			VectorSubtract(ent->tracktarget->r.currentOrigin, ent->r.currentOrigin, dir);
//			if( VectorLength(dir) > range )
//			{
//				loselock(ent);
//				return;
//			}
//			// check LOS
//			SV_Trace( &tr, ent->r.currentOrigin, 0, 0, ent->tracktarget->r.currentOrigin, ent->s.number, MASK_PLAYERSOLID, false );
//			if( tr.fraction < 1 && tr.entityNum != ent->tracktarget->s.number ) 
//			{
//				loselock(ent);
//				return;
//			}
//		}
//	}
//	else if( ent->tracktarget )
//	{
//		loselock(ent);
//	}
//}

//void GroundInstallation_Think( gentity_t* ent ) 
//{
//	float	diff, turnspeed;
//	float	timediff = level.time - ent->s.pos.trTime;
//	vec3_t	targdir, targangles;
//	int		locktime;
//
////	Com_Printf("Ground installation think..\n");
//
//	Update_GI_Targets(ent);
//
////	ent->s.angles2[ROLL] += 10;
////	if( ent->s.angles2[ROLL] >= 360 )
////		ent->s.angles2[ROLL] -= 360;
//
//	if( ent->tracktarget )
//	{
//		//Com_Printf("Ground installation tracking...%s\n", ent->tracktarget->classname);
//
//		VectorSubtract(ent->tracktarget->r.currentOrigin, ent->r.currentOrigin, targdir);
//		vectoangles(targdir, targangles);
//		targangles[1] -= 90;
//
//		// clamp target
//		if( targangles[1] >= 360 ) targangles[1] -= 360;
//		else if( targangles[1] < 0 ) targangles[1] += 360;
//		if( targangles[0] < 90 && 
//			targangles[0] > availableGroundInstallations[ent->s.modelindex2].minpitch ) 
//			targangles[0] = availableGroundInstallations[ent->s.modelindex2].minpitch;
//		else if( targangles[0] > 270 &&
//			targangles[0] < availableGroundInstallations[ent->s.modelindex2].maxpitch )
//			targangles[0] = availableGroundInstallations[ent->s.modelindex2].maxpitch;
//		
//		// turn
//		diff = targangles[1] - ent->s.angles2[ROLL];
//		if( diff > 180 ) diff -= 360;
//		else if( diff < -180 ) diff += 360;
//		turnspeed = availableGroundInstallations[ent->s.modelindex2].turnspeed[1] * timediff / 1000;
//		if( turnspeed < abs(diff) )
//		{
//			if( diff > 0 )
//				ent->s.angles2[ROLL] += turnspeed;
//			else
//				ent->s.angles2[ROLL] -= turnspeed;
//		}
//		else
//		{
//			ent->s.angles2[ROLL] = targangles[1];
//		}
//		if( diff <= ent->gi_lockangle )
//		{
//			if( ent->tracktarget->client )
//			{
//				if( ent->tracktarget->client->ps.ONOFF & OO_RADAR )
//					locktime = availableWeapons[ent->s.weaponIndex].lockdelay/2;
//				else
//					locktime = availableWeapons[ent->s.weaponIndex].lockdelay;
//			}
//			else
//			{
//				if( ent->s.ONOFF & OO_RADAR )
//					locktime = availableWeapons[ent->s.weaponIndex].lockdelay/2;
//				else
//					locktime = availableWeapons[ent->s.weaponIndex].lockdelay;
//			}
//			if( level.time > ent->locktime + locktime && Distance(ent->r.currentOrigin,ent->tracktarget->r.currentOrigin) <= availableWeapons[ent->s.weaponIndex].range)
//			{
//				//LaunchMissile_GI(ent);
//				FireWeapon_GI(ent);
//				ent->locktime = level.time + availableWeapons[ent->s.weaponIndex].fireInterval;	// so it doesnt launch all of them instantly
//			}
//		}
//		else
//			ent->locktime = level.time;
//	}
//	else if( level.time > ent->gi_reloadTime && 
//			 ent->count < availableGroundInstallations[ent->s.modelindex2].ammo ) 
//	{
//		ent->count++;
////		Com_Printf("Reloading: %d\n", ent->count);
//		ent->gi_reloadTime = level.time + availableGroundInstallations[ent->s.modelindex2].reloadTime;
//	}
//
//	ent->nextthink = level.time + 50;
//	ent->s.pos.trTime = level.time;
//	SV_LinkEntity (&ent->s, &ent->r);
//}

