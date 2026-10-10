#include "../vehicle_table.h"

// ZSU-23
static const VehicleEntry entries[] =
{
	{ 49,
	{
		.descriptiveName = "ZSU-23",
		.tinyName = "zsu23",
		.modelName = "ZSU-23",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_SAM,
		.caps = HC_DUALGUNS,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 35.7},
		.cam_height = {0, 100, 11.9},
		.mass = 19.0f,
		.maxspeed = 70,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 160,
		.gunoffset = {3.6, 1.2, 7.1},
		.maxfuel = 160,
		.gearheight = 285,
		.tailangle = 5,
		.weapons = { WI_MG_4X14_5MM, 0, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 1200, 0, 0, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 11.9},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
	} },
};

VehicleFamily VehicleFamilies::ZSU_23()
{
	return entries;
}
