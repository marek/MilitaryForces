#include "../vehicle_table.h"

// Sopwith camel
static const VehicleEntry entries[] =
{
	{ 42,
	{
		.descriptiveName = "Sopwith camel",
		.tinyName = "Camel",
		.modelName = "camel",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW1,
		.team = MF_TEAM_2,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {60, 70, 260},
		.cam_dist = {0, 100, 42.3},
		.cam_height = {0, 100, 14.1},
		.stallspeed = 55,
		.maxspeed = 170,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.6, -1.9, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_2XCAL303, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-5.6, 0, 8.5},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-1.4, 0, 21.1, 19.7},
	} },
};

VehicleFamily VehicleFamilies::camel()
{
	return entries;
}
