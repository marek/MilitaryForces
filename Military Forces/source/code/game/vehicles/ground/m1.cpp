#include "../vehicle_table.h"

// M1A1i Abrams
static const VehicleEntry entries[] =
{
	{ 43,
	{
		.descriptiveName = "M1A1i Abrams",
		.tinyName = "M1",
		.modelName = "m1",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_MBT,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 35.6},
		.cam_height = {0, 100, 17.8},
		.maxspeed = 60,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 25,
		.maxhealth = 400,
		.gunoffset = {4.5, 0, 7.1},
		.maxfuel = 150,
		.gearheight = 310,
		.tailangle = 3,
		.weapons = { WI_MG_12_7MM, WI_125MM_GUN, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE },
		.ammo = { 500, 60, 4, 2, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 12.5},
		.radarRange = 9000,
		.radarRange2 = 6000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::m1()
{
	return entries;
}
