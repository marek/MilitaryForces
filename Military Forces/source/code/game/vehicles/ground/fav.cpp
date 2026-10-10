#include "../vehicle_table.h"

// Fast Attack Vehicle
static const VehicleEntry entries[] =
{
	{ 45,
	{
		.descriptiveName = "Fast Attack Vehicle",
		.tinyName = "FAV",
		.modelName = "fav",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_RECON,
		.caps = HC_WHEELS,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 38.4},
		.cam_height = {0, 100, 12.8},
		.mass = 1.0f,
		.maxspeed = 120,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.wheels = 4,
		.wheelCF = 10.2,
		.accel = 50,
		.maxhealth = 130,
		.gunoffset = {10.2, 0, 2.6},
		.maxfuel = 160,
		.gearheight = 300,
		.tailangle = 6,
		.weapons = { WI_MG_12_7MM, 0, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 800, 0, 0, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 12.8},
		.radarRange = 8000,
		.radarRange2 = 4000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::fav()
{
	return entries;
}
