#include "../vehicle_table.h"

// Major
static const VehicleEntry entries[] =
{
	{ 62,
	{
		.descriptiveName = "Major",
		.tinyName = "Soldier",
		.modelName = "major",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_LQM,
		.cls = CLASS_LQM_INFANTRY,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 10, 0},
		.cam_height = {0, 10, 3.5},
		.maxspeed = 80,
		.maxhealth = 20,
		.weapons = { WI_MG_M4A1, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 5, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10},
		.trackCone = -1,
	} },
};

VehicleFamily VehicleFamilies::major()
{
	return entries;
}
