/*
 * $Id: mf_vehiclespawn.c,v 1.2 2002-01-30 19:26:02 thebjoern Exp $
*/

#include "g_local.h"


// spawning functions for vehicles, that just set certain values, functions etc.

// planes
void MF_Spawn_Plane(gentity_t *ent, int idx, qboolean landed) {

	// set flags
	ent->client->ps.ONOFF = OO_NOTHING_ON;

	// speed, throttle depend on landed/airborne 
	if( landed ) {
		ent->client->ps.speed = 0;
		ent->client->ps.fixed_throttle = 0;
		ent->client->ps.ONOFF |= OO_LANDED|OO_STALLED|OO_GEAR;
	}
	else {
		ent->client->ps.speed = availableVehicles[idx].maxspeed*7;// 70% of topspeed
		ent->client->ps.fixed_throttle = MF_THROTTLE_MILITARY;// thrust
	}
	
	// gearheight
	if( availableVehicles[idx].caps & HC_GEAR ) {
		ent->gearheight = (float)availableVehicles[idx].gearheight;
	} else {
		ent->client->ps.ONOFF |= OO_GEAR;
	}

	// set functions
	ent->touch = Touch_Plane;
	ent->pain = Plane_Pain;

	// update gear anim
	ent->updateGear = qtrue;
}

// ground vehicles
void MF_Spawn_GroundVehicle(gentity_t *ent, int idx) {

	// set flags
	ent->client->ps.ONOFF = OO_LANDED;	// only "aerial" when falling

	// speed, throttle
	ent->client->ps.speed = 0;
	ent->client->ps.fixed_throttle = 0;

	// set functions
	ent->touch = Touch_GroundVehicle;
	ent->pain = GroundVehicle_Pain;
}
