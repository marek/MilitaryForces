#include "../vehicle_table.h"

// helo
static const VehicleEntry entries[] =
{
	// Helo
	{ 53,
	{
		.descriptiveName = "helo",
		.tinyName = "helo",
		.modelName = "helo",
		.flags = VF_TESTONLY,
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_RECON,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 30},
		.cam_height = {0, 100, 10},
		.maxspeed = 130,
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
		.weapons = { 0, WI_MGT_2X30MM, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE },
		.ammo = { 0, 2, 4, 0, 0, 0, 0, 20 },
		.turret = { 1, 2, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::helo()
{
	return entries;
}
