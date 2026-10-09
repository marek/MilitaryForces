#include "../vehicle_table.h"

// MLRS
static const VehicleEntry entries[] =
{
	{ 52,
	{
		.descriptiveName = "MLRS",
		.tinyName = "MLRS",
		.modelName = "mlrs",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_SAM,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 30},
		.cam_height = {0, 100, 10},
		.maxspeed = 60,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 20,
		.maxhealth = 400,
		.gunoffset = {5, 0, 8},
		.maxfuel = 150,
		.gearheight = 335,
		.tailangle = 3,
		.weapons = { 0, WI_HELLFIRE, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 0, 12, 0, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 14},
		.radarRange = 8000,
		.radarRange2 = 4000,
	} },
};

VehicleFamily VehicleFamilies::mlrs()
{
	return entries;
}
