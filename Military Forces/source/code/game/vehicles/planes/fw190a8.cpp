#include "../vehicle_table.h"

// Focke-Wulf Fw190 A8
static const VehicleEntry entries[] =
{
	{ 39,
	{
		.descriptiveName = "Focke-Wulf Fw190 A8",
		.tinyName = "Fw-190",
		.modelName = "fw190a8",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_2,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {60, 60, 220},
		.cam_dist = {0, 100, 74.5},
		.cam_height = {0, 100, 14.9},
		.stallspeed = 90,
		.maxspeed = 420,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.8, -11, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-6, 0, 8.9},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-3, 0, 26.8, 26.8},
	} },
};

VehicleFamily VehicleFamilies::fw190a8()
{
	return entries;
}
