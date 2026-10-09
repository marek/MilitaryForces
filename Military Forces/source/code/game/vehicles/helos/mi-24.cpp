#include "../vehicle_table.h"

// Mi-24V Hind, Mi-24V Hind (CAS)
static const VehicleEntry entries[] =
{
	{ 58,
	{
		.descriptiveName = "Mi-24V Hind",
		.tinyName = "Mi-24",
		.modelName = "mi-24",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_ATTACK,
		.caps = HC_GEAR,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 80},
		.cam_height = {0, 100, 25},
		.maxspeed = 270,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 300,
		.gunoffset = {23, 0, -6},
		.maxfuel = 160,
		.weapons = { 0, WI_MGT_YAKB, WI_AT6, WI_S8, 0, 0, 0, WI_FLARE },
		.ammo = { 0, 1470, 2, 4, 0, 0, 0, 30 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1200,
	} },

	{ 59,
	{
		.descriptiveName = "Mi-24V Hind (CAS)",
		.tinyName = "Mi-24",
		.modelName = "mi-24",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_ATTACK,
		.caps = HC_GEAR,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 80},
		.cam_height = {0, 100, 25},
		.maxspeed = 270,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 300,
		.gunoffset = {23, 0, -6},
		.maxfuel = 160,
		.weapons = { 0, WI_MGT_YAKB, WI_S5, WI_S8, 0, 0, 0, WI_FLARE },
		.ammo = { 0, 1470, 2, 2, 0, 0, 0, 30 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1200,
	} },
};

VehicleFamily VehicleFamilies::mi_24()
{
	return entries;
}
