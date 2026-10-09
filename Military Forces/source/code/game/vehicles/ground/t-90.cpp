#include "../vehicle_table.h"

// T-90
static const VehicleEntry entries[] =
{
	{ 44,
	{
		.descriptiveName = "T-90",
		.tinyName = "T-90",
		.modelName = "t-90",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_MBT,
		.flags = VF_COOKOFF,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 36},
		.cam_height = {0, 100, 18},
		.maxspeed = 65,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 25,
		.maxhealth = 380,
		.gunoffset = {22.5, 0, 1.8},
		.maxfuel = 150,
		.gearheight = 310,
		.tailangle = 3,
		.weapons = { WI_MG_12_7MM, WI_125MM_GUN, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE },
		.ammo = { 500, 60, 4, 2, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 12.6},
		.radarRange = 9000,
		.radarRange2 = 6000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::t_90()
{
	return entries;
}
