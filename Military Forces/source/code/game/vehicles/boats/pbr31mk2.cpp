#include "../vehicle_table.h"

// Patrolboat
static const VehicleEntry entries[] =
{
	{ 63,
	{
		.descriptiveName = "Patrolboat",
		.tinyName = "PBR31",
		.modelName = "pbr31mk2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_BOAT,
		.cls = CLASS_BOAT_PATROL,
		.caps = HC_DUALGUNS,
		.turnspeed = {50, 100, 20},
		.cam_dist = {0, 100, 37.8},
		.cam_height = {0, 100, 12.6},
		.maxspeed = 50,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 15,
		.maxhealth = 350,
		.gunoffset = {6.3, 0, 10.1},
		.maxfuel = 150,
		.gearheight = 8,
		.tailangle = -5,
		.weapons = { 0, WI_MGT_2X30MM, WI_MGT_12_7MM, WI_HELLFIRE, WI_STINGER, 0, 0, WI_CFLARE },
		.ammo = { 0, 500, 300, 2, 2, 0, 0, 20 },
		.turret = { 1, 2, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 7.6},
		.radarRange = 8000,
		.radarRange2 = 7000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	} },
};

VehicleFamily VehicleFamilies::pbr31mk2()
{
	return entries;
}
