#include "../vehicle_table.h"

// P-51d Mustang
static const VehicleEntry entries[] =
{
	{ 36,
	{
		.descriptiveName = "P-51d Mustang",
		.tinyName = "P-51d",
		.modelName = "p-51d",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_1,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {60, 60, 200},
		.cam_dist = {0, 100, 50},
		.cam_height = {0, 100, 10},
		.stallspeed = 90,
		.maxspeed = 420,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.5f, -9.2f, 0.136f},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_6XCAL50, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-4, 0, 8},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {2,0,30,26},
	} },
};

VehicleFamily VehicleFamilies::p_51d()
{
	return entries;
}
