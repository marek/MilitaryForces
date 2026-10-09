#include "../vehicle_table.h"

// Mi-35 Hind
static const VehicleEntry entries[] =
{
	{ 60,
	{
		.descriptiveName = "Mi-35 Hind",
		.tinyName = "Mi-35",
		.modelName = "mi-35",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_ATTACK,
		.caps = HC_GEAR,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 80},
		.cam_height = {0, 100, 25},
		.maxspeed = 186,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 300,
		.gunoffset = {24, -2.6, -5.3},
		.maxfuel = 160,
		.weapons = { WI_MG_30MM, 0, WI_AT6, WI_GUNPOD_23MM, WI_S8, 0, 0, WI_FLARE },
		.ammo = { 750, 0, 2, 2, 2, 0, 0, 30 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1200,
	} },
};

VehicleFamily VehicleFamilies::mi_35()
{
	return entries;
}
