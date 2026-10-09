#include "../vehicle_table.h"

// Spitfire Mk 5b
static const VehicleEntry entries[] =
{
	{ 37,
	{
		.descriptiveName = "Spitfire Mk 5b",
		.tinyName = "Spitfire",
		.modelName = "spitfire_mk5b",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_1,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {80, 80, 240},
		.cam_dist = {0, 100, 65},
		.cam_height = {0, 100, 13},
		.stallspeed = 85,
		.maxspeed = 400,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.3, -9.6, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-5.2, 0, 7.8},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-5.2, 0, 28.6, 26},
	} },
};

VehicleFamily VehicleFamilies::spitfire_mk5b()
{
	return entries;
}
