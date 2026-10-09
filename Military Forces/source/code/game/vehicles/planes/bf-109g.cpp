#include "../vehicle_table.h"

// Messerschmitt Bf-109g
static const VehicleEntry entries[] =
{
	{ 38,
	{
		.descriptiveName = "Messerschmitt Bf-109g",
		.tinyName = "Bf-109",
		.modelName = "bf-109g",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_2,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {70, 65, 240},
		.cam_dist = {0, 100, 68},
		.cam_height = {0, 100, 13.6},
		.stallspeed = 90,
		.maxspeed = 410,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.4, -10, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-5.4, 0, 8.2},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-4.1, 0, 24.5, 21.8},
	} },
};

VehicleFamily VehicleFamilies::bf_109g()
{
	return entries;
}
