/*
 * $Id: g_groundinstallation.c,v 1.2 2003-02-24 01:24:10 thebjoern Exp $
*/

#include "g_local.h"

static void Find_GI_Targets( gentity_t* ent )
{
	vec3_t			mins, maxs, rangevec, dir, bestdir;
	float			range = availableGroundInstallations[ent->s.modelindex].radarRange;
	int				i, num, touch[MAX_GENTITIES];
	gentity_t		*hit, *best = 0;
	float			dist, closest;
	unsigned long	cat;
	trace_t			tr;

	// anti air missiles
	if( availableWeapons[ent->s.weaponIndex].type & WT_ANTIAIRMISSILE )
	{
		// no target yet
		if( !ent->tracktarget )
		{
			if( level.time > ent->gi_nextScanTime )
			{
				ent->gi_nextScanTime = level.time + 1500;
				VectorSet( rangevec, range, range, range );
				VectorSubtract( ent->r.currentOrigin, rangevec, mins );
				VectorAdd( ent->r.currentOrigin, rangevec, maxs );
				num = trap_EntitiesInBox( mins, maxs, touch, MAX_GENTITIES );

				for ( i=0 ; i<num ; i++ ) 
				{
					hit = &g_entities[touch[i]];
					// determine category
					if( hit->s.eType == ET_VEHICLE && hit->client )
					{
						cat = availableVehicles[hit->client->vehicle].cat;
					}
					else if( hit->s.eType == ET_MISC_VEHICLE &&
							 hit->s.modelindex < 255 )	// not other sams
					{
						cat = availableVehicles[hit->s.modelindex].cat;
					}
					else continue;
					// wrong category
					if( !(cat & CAT_PLANE) && !(cat & CAT_HELO) ) continue;
					// check LOS
					trap_Trace( &tr, ent->r.currentOrigin, 0, 0, hit->r.currentOrigin, ent->s.number, MASK_ALL );
					if( tr.fraction < 1 && tr.entityNum != hit->s.number ) continue;
					// check dist
					VectorSubtract(hit->r.currentOrigin, ent->r.currentOrigin, dir);
					dist = VectorNormalize(dir);
					if( !best ||
						(best && dist < closest) )
					{
						best = hit;
						closest = dist;
						VectorCopy( dir, bestdir );
					}
				}
				if( best )
				{
					ent->tracktarget = best;
					vectoangles(bestdir, ent->gi_targetHeading);
				}
				else
					ent->tracktarget = 0;
			}
		}
		// fight current target
		else
		{

		}
	}
}

void GroundInstallation_Think( gentity_t* ent ) 
{
	float	diff, turnspeed;
	float	timediff = level.time - ent->s.pos.trTime;

//	Com_Printf("Ground installation think..\n");

	if( ent->count > 0 && ent->s.weaponIndex > -1 ) // if it has weapons
		Find_GI_Targets(ent);

//	ent->s.angles2[ROLL] += 10;
//	if( ent->s.angles2[ROLL] >= 360 )
//		ent->s.angles2[ROLL] -= 360;

	// clamp target
	if( ent->gi_targetHeading[1] >= 360 ) ent->gi_targetHeading[1] -= 360;
	else if( ent->gi_targetHeading[1] < 0 ) ent->gi_targetHeading[1] += 360;
	if( ent->gi_targetHeading[0] < 90 && 
		ent->gi_targetHeading[0] > availableGroundInstallations[ent->s.modelindex2].minpitch ) 
		ent->gi_targetHeading[0] = availableGroundInstallations[ent->s.modelindex2].minpitch;
	else if( ent->gi_targetHeading[0] > 270 &&
		ent->gi_targetHeading[0] < availableGroundInstallations[ent->s.modelindex2].maxpitch )
		ent->gi_targetHeading[0] = availableGroundInstallations[ent->s.modelindex2].maxpitch;
	
	// turn
	diff = ent->gi_targetHeading[1] - ent->s.angles2[ROLL];
	if( diff > 180 ) diff -= 360;
	else if( diff < -180 ) diff += 360;
	turnspeed = availableGroundInstallations[ent->s.modelindex2].turnspeed[1] * timediff / 1000;
	if( turnspeed < abs(diff) )
	{
		if( diff > 0 )
			ent->s.angles2[ROLL] += turnspeed;
		else
			ent->s.angles2[ROLL] -= turnspeed;
	}
	else
	{
		ent->s.angles2[ROLL] = ent->gi_targetHeading[1];
	}

	ent->nextthink = level.time + 50;
	ent->s.pos.trTime = level.time;
	trap_LinkEntity (ent);
}


/*
void Drone_Plane_Think( gentity_t* ent ) {

	vec3_t		origin, angles, forward;
	trace_t		tr;
	vec3_t		bearing;
	float		diff, turnspeed;
	float		targroll;

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
	BG_EvaluateTrajectory( &ent->s.apos, level.time, angles );

	// trace a line from the previous position to the current position
	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->s.number, ent->clipmask );
	VectorCopy( tr.endpos, ent->r.currentOrigin );
	VectorCopy( angles, ent->r.currentAngles );

	// check position relative to target
	getTargetDirAndDist(ent);

	if( ent->physicsBounce < 50 ) {
		onWaypointEvent( ent );		
		getTargetDirAndDist( ent );		
	}

	// check whether to change course (yaw/roll)
	vectoangles( ent->pos1, bearing );
	diff = bearing[1] - angles[1];
	diff = AngleMod(diff);
	if( diff ) {
		// calc change
		turnspeed = availableVehicles[ent->s.modelindex].turnspeed[1];
		if( diff < -turnspeed ) ent->s.apos.trDelta[1] = -turnspeed;
		else if( diff > turnspeed ) ent->s.apos.trDelta[1] = turnspeed;
		else {
			ent->s.apos.trBase[1] = bearing[1];
			ent->s.apos.trDelta[1] = 0;
			ent->s.apos.trTime = level.time;
		}
	}

	// roll
	turnspeed = availableVehicles[ent->s.modelindex].turnspeed[2];
	if( diff > 1 ) targroll = -90;
	else if( diff < -1 ) targroll = 90;
	else targroll = 0;

	diff = targroll - angles[2];

	if( diff < -turnspeed ) ent->s.apos.trDelta[2] = -turnspeed;
	else if( diff > turnspeed ) ent->s.apos.trDelta[2] = turnspeed;
	else {
		ent->s.apos.trBase[2] = targroll;
		ent->s.apos.trDelta[2] = 0;	
		ent->s.apos.trTime = level.time;
	}


	// check whether to change course (pitch)
	diff = bearing[0] - angles[0];
	diff = AngleMod(diff);
	if( diff ) {
		// calc change
		turnspeed = availableVehicles[ent->s.modelindex].turnspeed[0];
		if( diff > 270 && diff < 360 - turnspeed ) ent->s.apos.trDelta[0] = turnspeed;
		else if( diff < 90 && diff > turnspeed ) ent->s.apos.trDelta[0] = turnspeed;
		else {
			ent->s.apos.trBase[0] = bearing[0];
			ent->s.apos.trDelta[0] = 0;
			ent->s.apos.trTime = level.time;
		}
	}

	ent->nextthink = level.time + 100;
	trap_LinkEntity (ent);

	AngleVectors( angles, forward, 0, 0 );
	VectorScale( forward, ent->speed, ent->s.pos.trDelta );
	VectorCopy( tr.endpos, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


*/
/*
	if( ent->s.ONOFF & OO_LANDED ) {

	} else {
		vec3_t		origin, angles, forward;
		trace_t		tr;
		vec3_t		bearing;
		float		diff, turnspeed;
		float		targroll;
		float		timediff = level.time - ent->s.pos.trTime;

		// get current position
		BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );
		BG_EvaluateTrajectory( &ent->s.apos, level.time, angles );

		// trace a line from the previous position to the current position
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->s.number, ent->clipmask );
		VectorCopy( tr.endpos, ent->r.currentOrigin );

		// check position relative to target
		getTargetDirAndDist(ent);

		if( ent->physicsBounce < 50 ) {
			onWaypointEvent( ent );		
			getTargetDirAndDist( ent );		
		}

		// check whether to change course (yaw/roll)
		vectoangles( ent->pos1, bearing );
		diff = bearing[1] - angles[1];
		diff = AngleMod(diff);
		if( diff ) {
			// calc change
			turnspeed = availableVehicles[ent->s.modelindex].turnspeed[1] * timediff / 1000;
			if( diff < -turnspeed ) angles[1] -= turnspeed;
			else if( diff > turnspeed ) angles[1] += turnspeed;
			else angles[1] = bearing[1];
		}

		// roll
		turnspeed = availableVehicles[ent->s.modelindex].turnspeed[2] * timediff / 1000;
		if( diff > 1 ) targroll = -90;
		else if( diff < -1 ) targroll = 90;
		else targroll = 0;

		diff = targroll - angles[2];

		if( diff < -turnspeed ) angles[2] -= turnspeed;
		else if( diff > turnspeed ) angles[2] += turnspeed;
		else angles[2] = targroll;	


		// check whether to change course (pitch)
		diff = bearing[0] - angles[0];
		diff = AngleMod(diff);
		if( diff ) {
			// calc change
			turnspeed = availableVehicles[ent->s.modelindex].turnspeed[0] * timediff / 1000;
			if( diff > 270 && diff < 360 - turnspeed ) angles[0] -= turnspeed;
			else if( diff < 90 && diff > turnspeed ) angles[0] += turnspeed;
			else angles[0] = bearing[0];
		}

		ent->nextthink = level.time + 50;
		trap_LinkEntity (ent);

		VectorCopy( angles, ent->s.angles );
		VectorCopy( ent->s.angles, ent->s.apos.trBase );
		VectorCopy( ent->s.angles, ent->r.currentAngles );
		ent->s.apos.trTime = level.time;

		AngleVectors( angles, forward, 0, 0 );
		VectorScale( forward, ent->speed, ent->s.pos.trDelta );
		VectorCopy( tr.endpos, ent->s.pos.trBase );
		ent->s.pos.trTime = level.time;

	}

	*/