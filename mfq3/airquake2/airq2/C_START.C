// file c_start.c by Bjoern Drabeck
// handles client variables for vehicle start

#include "g_local.h"
#include "c_start.h"
#include "v_lqm.h"

void Start_Client(edict_t *ent, int bullets, int shells, int sidewinders, int stingers,
		int amraams, int phoenix, int ATGMs, int mavericks, int rockets, int bombs,
	    int lgbs, int flares, int jammer )
{
	gitem_t	*it;

	// set new weapons
	ent->client->pers.max_bullets	 = bullets;
	ent->client->pers.max_shells	 = shells;
	ent->client->pers.max_sidewinder = sidewinders;
	ent->client->pers.max_ATGMs		 = ATGMs;
	ent->client->pers.max_mavericks  = mavericks;
	ent->client->pers.max_rockets	 = rockets;
	ent->client->pers.max_bombs		 = bombs;
	ent->client->pers.max_lgbs		 = lgbs;
	ent->client->pers.max_flares	 = flares;
	ent->client->pers.max_jammer     = jammer;
	ent->client->pers.max_stinger    = stingers;
	ent->client->pers.max_amraam	 = amraams;
	ent->client->pers.max_phoenix	 = phoenix;
	// set old weapons to 0
	ent->client->pers.max_grenades	 = 0;
	ent->client->pers.max_cells		 = 0;
	ent->client->pers.max_slugs		 = 0;

	// add new weapons
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] +=
			ent->client->pers.max_bullets;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] +=
			ent->client->pers.max_shells;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] +=
			ent->client->pers.max_sidewinder;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] +=
			ent->client->pers.max_ATGMs;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] +=
			ent->client->pers.max_mavericks;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] +=
			ent->client->pers.max_rockets;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] +=
			ent->client->pers.max_bombs;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] +=
			ent->client->pers.max_lgbs;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Jammer"))] +=
			ent->client->pers.max_jammer;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] +=
			ent->client->pers.max_stinger;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] +=
			ent->client->pers.max_amraam;
	ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] +=
			ent->client->pers.max_phoenix;
	ent->vehicle->flare = ent->client->pers.max_flares;

	// add launchers for weapons
	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("AutoCannon"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("MainGun"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("SW Launcher"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("ST Launcher"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("AM Launcher"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("PH Launcher"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM Launcher"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick Launcher"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Iron Bombs"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] )
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Laser Bombs"))] = 1;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Jammer"))] )
	{
		ent->client->pers.inventory[ITEM_INDEX(FindItem("ECM"))] = 1;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Jammer"))] = 0;
	}

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Blaster"))] = 0;


	// activate the autocannon
	it = FindItem ("AutoCannon");
	it->use (ent, it);

	ent->client->pers.weapon = it;


	// activate the secondary weapon
	if( ent->vehicle->HASCAN & HAS_AUTOAIM )
	{
		it = FindItem ("AutoCannon");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ECM"))] )
	{
		it = FindItem ("ECM");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("MainGun"))] )
	{
		it = FindItem ("MainGun");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("PH Launcher"))] )
	{
		it = FindItem ("PH Launcher");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("AM Launcher"))] )
	{
		it = FindItem ("AM Launcher");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("SW Launcher"))] )
	{
		it = FindItem ("SW Launcher");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ST Launcher"))] )
	{
		it = FindItem ("ST Launcher");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM Launcher"))] )
	{
		it = FindItem ("ATGM Launcher");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick Launcher"))] )
	{
		it = FindItem ("Maverick Launcher");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] )
	{
		it = FindItem ("Rocket Launcher");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Iron Bombs"))] )
	{
		it = FindItem ("Iron Bombs");
	}
	else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Laser Bombs"))] )
	{
		it = FindItem ("Laser Bombs");
	}
	ent->client->pers.secondary = it;
	ent->client->ammo_index2 = ITEM_INDEX(FindItem(ent->client->pers.secondary->ammo));

	// set the camera properly
	if( strcmp( ent->vehicle->classname, "plane" ) == 0 ||
		strcmp( ent->vehicle->classname, "helo" ) == 0 )
	{
		ent->cam_target = ent->vehicle;
		ent->client->cameramode = 1;
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		ent->cam_target = ent->vehicle->movetarget;
		ent->client->cameramode = 2;
	}
	else if( strcmp( ent->vehicle->classname, "LQM" ) == 0 )
	{
		ent->cam_target = ent->vehicle;
		ent->client->cameramode = 1;
	}

	// set functions properly
	if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
	{
		ent->vehicle->think = SteerPlane;
		ent->vehicle->touch = plane_touch;
		ent->vehicle->pain = vehicle_pain;
		ent->vehicle->die = vehicle_die;
	}
	else if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
	{
		ent->vehicle->think = SteerHelo;
		ent->vehicle->touch = helo_touch;
		ent->vehicle->pain = vehicle_pain;
		ent->vehicle->die = vehicle_die;
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		ent->vehicle->think = SteerGround;
		ent->vehicle->touch = ground_touch;
		ent->vehicle->pain = vehicle_pain;
		ent->vehicle->die = vehicle_die;
	}
	else if( strcmp( ent->vehicle->classname, "LQM" ) == 0 )
	{
		ent->vehicle->think = SteerLQM;
		ent->vehicle->touch = LQM_touch;
		ent->vehicle->pain = LQM_pain;
		ent->vehicle->die = LQM_die;
	}
	ent->vehicle->jam_time = level.time;
	ent->vehicle->flare_time = level.time + 1;
	ent->vehicle->delay = level.time;
	ent->client->pickle_time = ent->client->autocannon_time = level.time + 0.5;
	ent->client->side = 0;
	ent->client->death_msg_sent = 0;
}



