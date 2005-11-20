/*
 * $Id: g_waypoint.c,v 1.2 2005-11-20 11:21:38 thebjoern Exp $
*/

#include "g_local.h"


int findWaypoint( const char* name ) {
	unsigned int i = 0;
	for( i = 0; i < level.waypointList.usedWPs; ++i ) {
		if( strcmp( (&level.waypointList.waypoints[i])->name, name ) == 0 ) return i;
	}
	return -1;
}

void AddToWaypointManager( gentity_t *wp )
{
	unsigned int idx = level.waypointList.usedWPs;
	waypoint_t* w = &level.waypointList.waypoints[idx];

	// set wp
	w->index = idx;
	Q_strncpyz( w->name, wp->targetname, MAX_NAME_LENGTH );
	VectorCopy( wp->s.origin, w->pos );
	
	Com_Printf( "Found waypoint %s (%d)\n", w->name, idx );

	// increment
	level.waypointList.usedWPs++;
}

/*QUAKED misc_waypoint (0 .5 .8) (-32 -32 0) (32 32 32)
*/
/*void SP_misc_waypoint( gentity_t *ent ) 
{
	if( ent->targetname ) {
		AddToWaypointManager(ent);
	}

	G_FreeEntity(ent);
}
*/


