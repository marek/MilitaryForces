/*
 * $Id: mf_vehiclespawn.c,v 1.3 2006-01-29 14:03:41 thebjoern Exp $
*/

#include "g_local.h"
#include "g_entity.h"
#include "g_plane.h"
#include "g_groundvehicle.h"
#include "g_helo.h"
#include "g_lqm.h"
#include "g_boat.h"

// spawning functions for vehicles, that just set certain values, functions etc.

// planes
void MF_Spawn_Plane(GameEntity *ent, int idx, bool landed) 
{
	// set flags
	ent->client_->ps_.ONOFF = OO_NOTHING_ON;

	// speed, throttle depend on landed/airborne 
	if( landed ) 
	{
		ent->client_->ps_.speed = 0;
		ent->client_->ps_.fixed_throttle = 0;
		ent->client_->ps_.ONOFF |= OO_LANDED|OO_STALLED|OO_GEAR;
	}
	else 
	{
		ent->client_->ps_.speed = availableVehicles[idx].maxspeed*7;// 70% of topspeed
		ent->client_->ps_.fixed_throttle = MF_THROTTLE_MILITARY;// thrust
	}
	
	// gearheight
	if( availableVehicles[idx].caps & HC_GEAR ) 
		ent->gearheight_ = (float)availableVehicles[idx].gearheight;
	else 
		ent->client_->ps_.ONOFF |= OO_GEAR;

	// set functions
	ent->setTouch(new Entity_Plane::Entity_Plane_Touch);
	ent->setPain(new Entity_Plane::Entity_Plane_Pain);

	// update gear anim
	ent->updateGear_ = true;

	// update bay anim
	ent->updateBay_ = true;
}

// ground vehicles
void MF_Spawn_GroundVehicle(GameEntity *ent, int idx)
{

	// set flags
	ent->client_->ps_.ONOFF = OO_LANDED;	// only "aerial" when falling

	// speed, throttle
	ent->client_->ps_.speed = 0;
	ent->client_->ps_.fixed_throttle = 0;

	// set functions
	ent->setPain(new Entity_GroundVehicle::Entity_GroundVehicle_Pain);
}

// ground vehicles
void MF_Spawn_Helo(GameEntity *ent, int idx, bool landed) 
{

	// set flags
	ent->client_->ps_.ONOFF = OO_NOTHING_ON;

	// speed, throttle depend on landed/airborne 
	if( landed ) 
	{
		ent->client_->ps_.speed = 0;
		ent->client_->ps_.fixed_throttle = 0;
		ent->client_->ps_.ONOFF |= OO_LANDED|OO_STALLED|OO_GEAR;
	}

	// gearheight
	if( availableVehicles[idx].caps & HC_GEAR ) 
		ent->gearheight_ = (float)availableVehicles[idx].gearheight;
	else 
		ent->client_->ps_.ONOFF |= OO_GEAR;

	// update gear anim
	ent->updateGear_ = true;

	// set functions
	ent->setTouch(new Entity_Helicopter::Entity_Helicopter_Touch);
	ent->setPain(new Entity_Helicopter::Entity_Helicopter_Pain);

}

// LQM
void MF_Spawn_LQM(GameEntity *ent, int idx) 
{
	// set functions
	ent->setTouch(new Entity_Infantry::Entity_Infantry_Touch);
	ent->setPain(new Entity_Infantry::Entity_Infantry_Pain);

}

// Boat
void MF_Spawn_Boat(GameEntity *ent, int idx) 
{

	// set flags
	ent->client_->ps_.ONOFF = OO_LANDED;	// only "aerial" when falling

	// speed, throttle
	ent->client_->ps_.speed = 0;
	ent->client_->ps_.fixed_throttle = 0;

	// set functions
	ent->setPain(new Entity_Boat::Entity_Boat_Pain);

}


