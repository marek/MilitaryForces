#include "../vehicle_table.h"

// F-15E Eagle
static const VehicleEntry entries[] =
{
	{ 22,
	{
		.descriptiveName = "F-15E Eagle",
		.tinyName = "F-15",
		.modelName = "f-15",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {65, 50, 240},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 220,
		.maxspeed = 880,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 260,
		.maxhealth = 150,
		.gunoffset = {19.1f,7.87f,-0.112f},
		.maxfuel = 80,
		.gearheight = 4,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_MK83, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 4, 17, 0, 0, 0, 0, 35 },
		.cockpitview = {28, 0, 6},
		.effectModel = AB_RED,
		.radarRange = 14000,
		.radarRange2 = 7000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.renderFlags = MFR_DUALPILOT|MFR_BIGVAPOR,
		.shadowCoords = {6,0,48,34},
	} },
};

VehicleFamily VehicleFamilies::f_15()
{
	return entries;
}
