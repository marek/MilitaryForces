#include "../vehicle_table.h"

// M4 Sherman
static const VehicleEntry entries[] =
{
	{ 51,
	{
		.descriptiveName = "M4 Sherman",
		.tinyName = "M4",
		.modelName = "m4",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_2,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_MBT,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 21.9},
		.cam_height = {0, 100, 7.3},
		.mass = 30.0f,
		.maxspeed = 60,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 20,
		.maxhealth = 350,
		.gunoffset = {3.6, 0, 5.8},
		.maxfuel = 150,
		.gearheight = 335,
		.tailangle = 3,
		.weapons = { WI_MG_12_7MM, WI_125MM_GUN, 0, 0, 0, 0, 0, 0 },
		.ammo = { 500, 40, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10.2},
		.radarRange = 8000,
		.radarRange2 = 4000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::m4()
{
	return entries;
}
