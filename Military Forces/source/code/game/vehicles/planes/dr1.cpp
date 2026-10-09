#include "../vehicle_table.h"

// Fokker Dr.1
static const VehicleEntry entries[] =
{
	{ 41,
	{
		.descriptiveName = "Fokker Dr.1",
		.tinyName = "Fokker",
		.modelName = "dr1",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW1,
		.team = MF_TEAM_1,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {70, 80, 320},
		.cam_dist = {0, 100, 43.2},
		.cam_height = {0, 100, 14.4},
		.stallspeed = 50,
		.maxspeed = 150,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.6, -2, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_2XCAL312, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-5.8, 0, 8.6},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-4.3, 0, 18.7, 17.3},
	} },
};

VehicleFamily VehicleFamilies::dr1()
{
	return entries;
}
