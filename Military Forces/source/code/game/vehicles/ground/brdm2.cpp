#include "../vehicle_table.h"

// BRDM 2
static const VehicleEntry entries[] =
{
	{ 48,
	{
		.descriptiveName = "BRDM 2",
		.tinyName = "BRDM2",
		.modelName = "brdm2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_RECON,
		.caps = HC_DUALGUNS|HC_AMPHIBIOUS|HC_WHEELS,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 30},
		.cam_height = {0, 100, 10},
		.maxspeed = 90,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.wheels = 4,
		.wheelCF = 9,
		.accel = 33,
		.maxhealth = 170,
		.gunoffset = {3, 1, 6},
		.maxfuel = 160,
		.gearheight = 285,
		.tailangle = 5,
		.weapons = { WI_MG_14_5MM, WI_STINGER, WI_HELLFIRE, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 600, 2, 4, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::brdm2()
{
	return entries;
}
