#include "../vehicle_table.h"

// B-17g
static const VehicleEntry entries[] =
{
	{ 40,
	{
		.descriptiveName = "B-17g",
		.tinyName = "B-17",
		.modelName = "b17g",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_1,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.flags = VF_GUNSTATIONS,
		.caps = HC_GEAR|HC_PROP| HC_TAILDRAGGER| HC_WEAPONBAY,
		.turnspeed = {40, 30, 100},
		.cam_dist = {0, 100, 97.6},
		.cam_height = {0, 100, 12.2},
		.stallspeed = 100,
		.maxspeed = 320,
		.maxthrottle = 10,
		.engines = 4,
		.accel = 220,
		.maxhealth = 1500,
		.gunoffset = {3.1, -9, -0.2},
		.maxfuel = 60,
		.tailangle = -8,
		.weapons = { WI_MG_8XCAL50, WI_MK83, WI_MK83, 0, 0, 0, 0, 0 },
		.ammo = { 450, 16, 16, 0, 0, 0, 0, 0 },
		.cockpitview = {-4.9, 0, 7.3},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 500,
		.shadowCoords = {-14.6, 0, 73.2, 73.2},
	} },
};

VehicleFamily VehicleFamilies::b17g()
{
	return entries;
}
