#include "../vehicle_table.h"

// Panzer III
static const VehicleEntry entries[] =
{
	{ 50,
	{
		.descriptiveName = "Panzer III",
		.tinyName = "Pz 3",
		.modelName = "panzeriii",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_1,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_MBT,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 40.5},
		.cam_height = {0, 100, 13.5},
		.mass = 23.0f,
		.maxspeed = 60,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 20,
		.maxhealth = 400,
		.gunoffset = {6.8, 0, 10.8},
		.maxfuel = 150,
		.gearheight = 335,
		.tailangle = 3,
		.weapons = { WI_MG_12_7MM, WI_125MM_GUN, 0, 0, 0, 0, 0, 0 },
		.ammo = { 500, 40, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 18.9},
		.radarRange = 8000,
		.radarRange2 = 4000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::panzeriii()
{
	return entries;
}
