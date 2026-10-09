#include "../vehicle_table.h"

// UH-1 "Huey", UH-1 "Huey" (Gunship)
static const VehicleEntry entries[] =
{
	{ 54,
	{
		.descriptiveName = "UH-1 \"Huey\"",
		.tinyName = "UH-1",
		.modelName = "UH-1",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_RECON,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 46.8},
		.cam_height = {0, 100, 14.6},
		.maxspeed = 130,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 170,
		.gunoffset = {1.8, 0.6, 3.5},
		.maxfuel = 160,
		.gearheight = 285,
		.tailangle = 5,
		.flags = VF_DOORGUNS,
		.weapons = { 0, WI_MGT_7_62MM, WI_FUELCRATE, WI_HEALTHCRATE, WI_AMMOCRATE, 0, 0, WI_FLARE },
		.ammo = { 0, 1500, 3, 3, 3, 0, 0, 20 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.renderFlags = MFR_DUALPILOT,
		.cockpitview = {0, 0, 5.8},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
	} },

	// the Huey without supplies: twice the door gun ammunition, for games without team mates to supply.
	// Added at the end so the indices of the vehicles above stay as they were
	{ 64,
	{
		.descriptiveName = "UH-1 \"Huey\" (Gunship)",
		.tinyName = "UH-1G",
		.modelName = "UH-1",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_RECON,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 46.8},
		.cam_height = {0, 100, 14.6},
		.maxspeed = 143,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 170,
		.gunoffset = {1.8, 0.6, 3.5},
		.maxfuel = 160,
		.gearheight = 285,
		.tailangle = 5,
		.flags = VF_DOORGUNS,
		.weapons = { 0, WI_MGT_7_62MM, 0, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 0, 3000, 0, 0, 0, 0, 0, 40 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.renderFlags = MFR_DUALPILOT,
		.cockpitview = {0, 0, 5.8},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
	} },
};

VehicleFamily VehicleFamilies::UH_1()
{
	return entries;
}
