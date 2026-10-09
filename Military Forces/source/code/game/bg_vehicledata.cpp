/*
 * $Id: bg_vehicledata.c,v 1.4 2007-06-17 17:11:12 minkis Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"


completeLoadout_t availableLoadouts[MAX_LOADOUTS];


// The vehicle table is filled from the family files under vehicles/ (see vehicles/registry.cpp)



groundInstallationData_t availableGroundInstallations[] = 
{
    {
		.descriptiveName = "SAM Mk1",
		.tinyName = "SAM",
		.modelName = "turret_samup",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {90, 90, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 100,
		.weapon = WI_AASAM,
		.ammo = 4,
		.radarRange = 4500,
		.trackCone2 = 1,
		.reloadTime = 6000,
	},

    {
		.descriptiveName = "SAM Mk2",
		.tinyName = "SAM",
		.modelName = "turret_samup",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {75, 75, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 110,
		.weapon = WI_AASAM,
		.ammo = 8,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 1,
		.reloadTime = 6000,
	},

    {
		.descriptiveName = "SAM Mk3",
		.tinyName = "SAM",
		.modelName = "turret_samup",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {60, 60, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 120,
		.weapon = WI_AASAM,
		.ammo = 12,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 2,
		.reloadTime = 6000,
	},

    {
		.descriptiveName = "SAM Mk4",
		.tinyName = "SAM4",
		.modelName = "turret_samup",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {45, 45, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 130,
		.weapon = WI_AASAM,
		.ammo = 16,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 3,
		.reloadTime = 6000,
	},  

	{
		.descriptiveName = "Flak (modern)",
		.tinyName = "Flak",
		.modelName = "turret_aaa",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {45, 45, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 130,
		.weapon = WI_FLAK,
		.ammo = 16,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 3,
		.reloadTime = 6000,
	},	

	{
		.descriptiveName = "Flak (ww2)",
		.tinyName = "Flak",
		.modelName = "turret_aaa",
		.gameset = MF_GAMESET_WW2,
		.turnspeed = {45, 45, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 130,
		.weapon = WI_FLAK,
		.ammo = 16,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 3,
		.reloadTime = 6000,
	},

	{
		.descriptiveName = "AA",
		.tinyName = "AA",
		.modelName = "turret_aa",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {45, 45, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 130,
		.weapon = WI_MG_20MM,
		.ammo = 5000,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 3,
		.reloadTime = 6000,
	},
};

int bg_numberOfGroundInstallations = std::size( availableGroundInstallations );


const char *gameset_items[MF_MAX_GAMESETS+1] =
{
	"Modern",
	"World War II",
	"World War I",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

const char *gameset_codes[MF_MAX_GAMESETS+1] =
{
	"modern",
	"ww2",
	"ww1",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

const char *team_items[MF_MAX_GAMESETS][MF_MAX_TEAMS+1] =
{
	// modern
	{
		"NATO",
		"Rebels",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// world war 2
	{
		"U.S.Air Force",
		"Royal Air Force",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// world war 1
	{
		"Germany",
		"England",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	}
};

const char *cat_fileRef[MF_MAX_CATEGORIES+1] =
{
	// categories
	"Plane",
	"GroundVehicle",
	"Helicopter",	// ok that way, Mark ?
	"Infantry",
	"Boat",
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

const char *cat_items[MF_MAX_CATEGORIES+1] =
{
	// categories
	"Planes",
	"Ground Vehicles",
	"Helicopters",
	"Infantry",
	"Boats",
	"Ground Installations",
	nullptr,
	nullptr,
	nullptr
};

const char *class_items[MF_MAX_CATEGORIES][MF_MAX_CLASSES+1] =
{	
	// planes
	{
		"Fighter",
		"Bomber",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// ground vehicles
	{
		"MBT",
		"Recon",
		"APC",
		"SAM",
		"AAG",
		"Transport",
		"Light",
		nullptr,
		nullptr
	},

	// helicopters
	{
		"Attack",
		"Recon",
		"Transport",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// infantry
	{
		"Special Unit",
		"Infantry",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// boats
	{
		"Patrol Boat",
		"Transport Boat",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	}
};

