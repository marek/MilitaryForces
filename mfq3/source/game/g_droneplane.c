/*
 * $Id: g_droneplane.c,v 1.3 2001-12-03 21:33:46 thebjoern Exp $
*/

#include "g_local.h"


void Drone_Plane_Think( gentity_t* ent ) {

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
		if( diff > 180 ) diff -= 360;
		else if( diff < -180 ) diff += 360;
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

		if( diff < -turnspeed ) {
			angles[2] -= turnspeed;
			ent->s.apos.trDelta[2] = -availableVehicles[ent->s.modelindex].turnspeed[2];
		}
		else if( diff > turnspeed ) {
			angles[2] += turnspeed;
			ent->s.apos.trDelta[2] = availableVehicles[ent->s.modelindex].turnspeed[2];
		}
		else {
			angles[2] = targroll;	
			ent->s.apos.trDelta[2] = 0;
		}


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