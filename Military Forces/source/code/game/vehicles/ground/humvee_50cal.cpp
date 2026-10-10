#include "../vehicle_table.h"

// HMMWV
static const VehicleEntry entries[] =
{
	{ 46,
	{
		.descriptiveName = "HMMWV",
		.tinyName = "HMMWV",
		.modelName = "humvee_50cal",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_RECON,
		.caps = HC_WHEELS,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 30},
		.cam_height = {0, 100, 10},
		.mass = 2.5f,
		.maxspeed = 85,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.wheels = 4,
		.wheelCF = 8,
		.accel = 35,
		.maxhealth = 130,
		.gunoffset = {8, 0, 2},
		.maxfuel = 160,
		.gearheight = 300,
		.tailangle = 6,
		.weapons = { WI_MG_12_7MM, WI_STINGER, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 500, 4, 0, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 4000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::humvee_50cal()
{
	return entries;
}
