/*
 * $Id: g_droneutil.c,v 1.1 2001-12-03 21:33:46 thebjoern Exp $
*/

#include "g_local.h"



void onWaypointEvent( gentity_t* ent ) {

	int i;

	for( i = ent->idxScriptBegin; i < ent->idxScriptEnd; ++i ) {
		if( level.scriptList.scripts[i].type == ST_ON_WAYPOINT &&
			strcmp( level.scriptList.scripts[i].name, ent->nextWaypoint->name) == 0 ) {
			ent->nextWaypoint = level.scriptList.scripts[i].nextWp;
			return;
		}
	}
	ent->nextWaypoint = 0;
}

void getTargetDirAndDist( gentity_t* ent ) {

	vec3_t		target;
	vec3_t		forward;

	// check position relative to target
	AngleVectors( ent->s.apos.trBase, forward, 0, 0 );
	if( ent->nextWaypoint ) {
		VectorCopy( ent->nextWaypoint->pos, target );
	} else {	// if no target then circle
		VectorMA( ent->r.currentOrigin, -100, forward, target );
	}
	VectorSubtract( target, ent->r.currentOrigin, ent->pos1 );
	ent->physicsBounce = VectorNormalize( ent->pos1 );// physicsbounce = dist
}


