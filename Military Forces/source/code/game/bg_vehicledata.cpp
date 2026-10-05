/*
 * $Id: bg_vehicledata.c,v 1.4 2007-06-17 17:11:12 minkis Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"


completeLoadout_t availableLoadouts[MAX_LOADOUTS];

// just contains data of all the available vehicles
completeVehicleData_t availableVehicles[] = 
{
	// ---------------------------------------------
	// Planes
	// ---------------------------------------------
    {
		.descriptiveName = "F-16 Falcon",
		.tinyName = "F-16",
		.modelName = "f-16",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {85, 60, 280},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 700,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 260,
		.maxhealth = 120,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 4, 4, 0, 0, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 12000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {4,0,34,24},
	},

    {
		.descriptiveName = "F-16 Falcon (AG - MK82)",
		.tinyName = "F-16",
		.modelName = "f-16",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {85, 60, 280},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 700,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 260,
		.maxhealth = 120,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_AMRAAM, WI_SIDEWINDER, WI_MK82, WI_CM, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 2, 12, 1, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 12000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {4,0,34,24},
	},



    {
		.descriptiveName = "F-16 Falcon (AG - Maverick)",
		.tinyName = "F-16",
		.modelName = "f-16",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {85, 60, 280},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 700,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 260,
		.maxhealth = 120,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_MAVERICK, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 2, 12, 0, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 12000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {4,0,34,24},
	},


    {
		.descriptiveName = "F-16 Falcon (AG - FFAR)",
		.tinyName = "F-16",
		.modelName = "f-16",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {85, 60, 280},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 700,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 260,
		.maxhealth = 120,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_FFAR, WI_CM, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 2, 8, 1, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 12000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {4,0,34,24},
	},


    {
		.descriptiveName = "F-16 Falcon (AG - Weasel)",
		.tinyName = "F-16",
		.modelName = "f-16",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {85, 60, 280},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 700,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 260,
		.maxhealth = 120,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_HARM, WI_ECM, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 2, 4, 1, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 12000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {4,0,34,24},
	},


    {
		.descriptiveName = "F-16 Falcon (ER)",
		.tinyName = "F-16",
		.modelName = "f-16",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {85, 60, 280},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 700,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 260,
		.maxhealth = 120,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_AMRAAM, WI_SPARROW, WI_DROPTANK_PAIR, WI_DROPTANK_SMALL, 0, 0, WI_FLARE },
		.ammo = { 450, 4, 2, 2, 1, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 12000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {4,0,34,24},
	},


    {
		.descriptiveName = "F-16 Falcon (DF)",
		.tinyName = "F-16",
		.modelName = "f-16",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {85, 60, 280},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 700,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 260,
		.maxhealth = 120,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_CM, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 8, 1, 0, 0, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 12000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {4,0,34,24},
	},



    {
		.descriptiveName = "Saab JAS 39 Gripen",
		.tinyName = "Jas-39",
		.modelName = "jas-39",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {90, 70, 320},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 250,
		.maxhealth = 110,
		.gunoffset = {19.698f, -1.328f, -3.016f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_CM, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 4, 1, 0, 0, 0, 28 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 10000,
		.radarRange2 = 8000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {6,0,36,20},
	},

    {
		.descriptiveName = "Saab JAS 39 Gripen (AG - FFAR+)",
		.tinyName = "Jas-39",
		.modelName = "jas-39",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {90, 70, 320},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 250,
		.maxhealth = 110,
		.gunoffset = {19.698f, -1.328f, -3.016f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_FFAR_LARGE, WI_MAVERICK, WI_CM, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 4, 1, 1, 0, 0, 28 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 10000,
		.radarRange2 = 8000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {6,0,36,20},
	},


    {
		.descriptiveName = "Saab JAS 39 Gripen (AG - Weasel)",
		.tinyName = "Jas-39",
		.modelName = "jas-39",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {90, 70, 320},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 250,
		.maxhealth = 110,
		.gunoffset = {19.698f, -1.328f, -3.016f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_HARM, WI_MAVERICK, WI_ECM, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 4, 1, 1, 0, 0, 28 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 10000,
		.radarRange2 = 8000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {6,0,36,20},
	},


    {
		.descriptiveName = "Saab JAS 39 Gripen (ER)",
		.tinyName = "Jas-39",
		.modelName = "jas-39",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {90, 70, 320},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 250,
		.maxhealth = 110,
		.gunoffset = {19.698f, -1.328f, -3.016f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_SPARROW, WI_DROPTANK_SMALL_PAIR, WI_DROPTANK_SMALL, WI_CM, 0, WI_FLARE },
		.ammo = { 450, 2, 2, 2, 1, 1, 0, 28 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 10000,
		.radarRange2 = 8000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {6,0,36,20},
	},


    {
		.descriptiveName = "Saab JAS 39 Gripen (DF)",
		.tinyName = "Jas-39",
		.modelName = "jas-39",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {90, 70, 320},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 1,
		.accel = 250,
		.maxhealth = 110,
		.gunoffset = {19.698f, -1.328f, -3.016f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_CM, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 6, 1, 0, 0, 0, 0, 28 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 10000,
		.radarRange2 = 8000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {6,0,36,20},
	},


/*
    {	"Harrier GR-7",		    // descriptiveName
	"GR-7",						// tinyName
	"gr7",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_ANY,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{100, 70, 240},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	720,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	250,						// acceleration
	100,							// health
	{19.698f, -1.328f, -3.016f},// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_FFAR, WI_MK82, 0, 0, WI_FLARE,		// weapons
	450, 2, 2, 2, 6, 0, 0, 28,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	10000,						// radar range
	8000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },
*/

    {
		.descriptiveName = "A-10 Thunderbolt II",
		.tinyName = "A-10",
		.modelName = "a10",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 60, 200},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 270,
		.maxhealth = 280,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_30MM, WI_SIDEWINDER, WI_MK84, WI_MK83, 0, 0, 0, WI_FLARE },
		.ammo = { 500, 3, 2, 10, 0, 0, 0, 64 },
		.cockpitview = {11, 0, 5},
		.radarRange = 8000,
		.radarRange2 = 10000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},



    {
		.descriptiveName = "A-10 Thunderbolt II (Runway Killer)",
		.tinyName = "A-10",
		.modelName = "a10",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 60, 200},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 270,
		.maxhealth = 280,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_30MM, WI_SIDEWINDER, WI_MAVERICK, WI_GBU15, WI_DURANDAL, 0, 0, WI_FLARE },
		.ammo = { 500, 3, 2, 2, 8, 0, 0, 64 },
		.cockpitview = {11, 0, 5},
		.radarRange = 8000,
		.radarRange2 = 10000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},


    {
		.descriptiveName = "A-10 Thunderbolt II (FFAR)",
		.tinyName = "A-10",
		.modelName = "a10",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 60, 200},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 270,
		.maxhealth = 280,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_30MM, WI_CM, WI_SIDEWINDER, WI_FFAR, WI_FFAR_LARGE, 0, 0, WI_FLARE },
		.ammo = { 500, 1, 3, 3, 2, 0, 0, 64 },
		.cockpitview = {11, 0, 5},
		.radarRange = 8000,
		.radarRange2 = 10000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},


    {
		.descriptiveName = "A-10 Thunderbolt II (CAS)",
		.tinyName = "A-10",
		.modelName = "a10",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 60, 200},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 270,
		.maxhealth = 280,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_30MM, WI_ECM, WI_CM, WI_SIDEWINDER, WI_FFAR, WI_MAVERICK, WI_MK82, WI_FLARE },
		.ammo = { 500, 1, 1, 2, 1, 2, 13, 64 },
		.cockpitview = {11, 0, 5},
		.radarRange = 8000,
		.radarRange2 = 10000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},


    {
		.descriptiveName = "A-10 Thunderbolt II (ER)",
		.tinyName = "A-10",
		.modelName = "a10",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 60, 200},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 270,
		.maxhealth = 280,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_30MM, WI_DROPTANK_PAIR, WI_MAVERICK, WI_SIDEWINDER, 0, 0, 0, WI_FLARE },
		.ammo = { 500, 2, 8, 2, 0, 0, 0, 64 },
		.cockpitview = {11, 0, 5},
		.radarRange = 8000,
		.radarRange2 = 10000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},

	/*
    {	"F-105 TEST",			// descriptiveName
	"F-105",					// tinyName
	"f-105",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_ANY,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{55, 45, 240},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	220,						// stallspeed
	800,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	180,						// health
	{19.1f,7.87f,-0.112f},		// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_MK83R, 0, 0, 0, 0, WI_FLARE,	// weapons
	450, 2, 11, 0, 0, 0, 0, 50,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{28, 0, 6},					// cameraposition for cockpit view
	AB_RED,						// effect model
	8000,						// radar range
	10000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{6,0,48,34},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },
	*/


    {
		.descriptiveName = "F-5 Tiger",
		.tinyName = "F-5",
		.modelName = "f-5",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 80, 380},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 270,
		.maxhealth = 100,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_2X20MM, WI_SIDEWINDER, WI_SPARROW, WI_FFAR_LARGE, 0, 0, 0, WI_FLARE },
		.ammo = { 500, 4, 2, 1, 0, 0, 0, 32 },
		.cockpitview = {11, 0, 5},
		.effectModel = AB_RED_SMALL,
		.radarRange = 6000,
		.radarRange2 = 4000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},


    {
		.descriptiveName = "F-5 Tiger (AG)",
		.tinyName = "F-5",
		.modelName = "f-5",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 80, 380},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 270,
		.maxhealth = 100,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_2X20MM, WI_SIDEWINDER, WI_FFAR, WI_MK83, WI_MK84, 0, 0, WI_FLARE },
		.ammo = { 500, 2, 2, 2, 1, 0, 0, 32 },
		.cockpitview = {11, 0, 5},
		.effectModel = AB_RED_SMALL,
		.radarRange = 6000,
		.radarRange2 = 4000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},


    {
		.descriptiveName = "F-5 Tiger (AG - FFAR)",
		.tinyName = "F-5",
		.modelName = "f-5",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 80, 380},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 270,
		.maxhealth = 100,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_2X20MM, WI_SIDEWINDER, WI_FFAR, WI_FFAR_LARGE, WI_CM, 0, 0, WI_FLARE },
		.ammo = { 500, 1, 4, 1, 1, 0, 0, 32 },
		.cockpitview = {11, 0, 5},
		.effectModel = AB_RED_SMALL,
		.radarRange = 6000,
		.radarRange2 = 4000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},


   {
		.descriptiveName = "F-5 Tiger (ER)",
		.tinyName = "F-5",
		.modelName = "f-5",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 80, 380},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 270,
		.maxhealth = 100,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_2X20MM, WI_SIDEWINDER, WI_SPARROW, WI_DROPTANK_SMALL_PAIR, WI_DROPTANK_SMALL, 0, 0, WI_FLARE },
		.ammo = { 500, 2, 2, 2, 1, 0, 0, 32 },
		.cockpitview = {11, 0, 5},
		.effectModel = AB_RED_SMALL,
		.radarRange = 6000,
		.radarRange2 = 4000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},


   {
		.descriptiveName = "F-5 Tiger (DF)",
		.tinyName = "F-5",
		.modelName = "f-5",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_DUALGUNS,
		.turnspeed = {80, 80, 380},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 170,
		.maxspeed = 620,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 270,
		.maxhealth = 100,
		.gunoffset = {26.25f, -0.94f, 1.811f},
		.maxfuel = 80,
		.weapons = { WI_MG_2X20MM, WI_CM, WI_SIDEWINDER, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 500, 1, 5, 0, 0, 0, 0, 32 },
		.cockpitview = {11, 0, 5},
		.effectModel = AB_RED_SMALL,
		.radarRange = 6000,
		.radarRange2 = 4000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {5,0,34,26},
	},


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
	},

    {
		.descriptiveName = "F-14 Tomcat",
		.tinyName = "F-14",
		.modelName = "f-14",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_SWINGWING,
		.turnspeed = {60, 55, 220},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 220,
		.maxspeed = 840,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 260,
		.maxhealth = 150,
		.gunoffset = {37.8f,-2.4f,-1.3f},
		.maxfuel = 70,
		.gearheight = 1.5,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_SPARROW, WI_PHOENIX, WI_CM, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 2, 4, 1, 0, 0, 35 },
		.cockpitview = {28, 0, 6},
		.effectModel = AB_RED,
		.radarRange = 20000,
		.radarRange2 = 5500,
		.trackCone2 = -1,
		.swingangle = 50,
		.gearTime = 1400,
		.bayTime = 1000,
		.renderFlags = MFR_DUALPILOT|MFR_BIGVAPOR,
	},


    {
		.descriptiveName = "F-14 Tomcat (ER)",
		.tinyName = "F-14",
		.modelName = "f-14",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_SWINGWING,
		.turnspeed = {60, 55, 220},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 220,
		.maxspeed = 840,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 260,
		.maxhealth = 150,
		.gunoffset = {37.8f,-2.4f,-1.3f},
		.maxfuel = 70,
		.weapons = { WI_MG_20MM, WI_PHOENIX, WI_SIDEWINDER, WI_DROPTANK_SMALL_PAIR, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 6, 2, 2, 0, 0, 0, 35 },
		.cockpitview = {28, 0, 6},
		.effectModel = AB_RED,
		.radarRange = 20000,
		.radarRange2 = 5500,
		.trackCone2 = -1,
		.swingangle = 50,
		.gearTime = 1400,
		.bayTime = 1000,
		.renderFlags = MFR_DUALPILOT|MFR_BIGVAPOR,
	},


    {
		.descriptiveName = "F-18 Hornet",
		.tinyName = "F-18",
		.modelName = "f-18",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {70, 60, 240},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 260,
		.maxhealth = 130,
		.gunoffset = {40.3f, 0, 0.478f},
		.maxfuel = 60,
		.gearheight = 4,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_SPARROW, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 4, 4, 4, 0, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 11000,
		.radarRange2 = 8500,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {8,0,42,42},
	},



    {
		.descriptiveName = "F-18 Hornet (AG)",
		.tinyName = "F-18",
		.modelName = "f-18",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {70, 60, 240},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 260,
		.maxhealth = 130,
		.gunoffset = {40.3f, 0, 0.478f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_MK82, WI_MK83, WI_CM, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 4, 8, 1, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 11000,
		.radarRange2 = 8500,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {8,0,42,42},
	},



    {
		.descriptiveName = "F-18 Hornet (AG - Maverick)",
		.tinyName = "F-18",
		.modelName = "f-18",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {70, 60, 240},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 260,
		.maxhealth = 130,
		.gunoffset = {40.3f, 0, 0.478f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_MAVERICK, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 13, 0, 0, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 11000,
		.radarRange2 = 8500,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {8,0,42,42},
	},


    {
		.descriptiveName = "F-18 Hornet (AG - Weasel)",
		.tinyName = "F-18",
		.modelName = "f-18",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {70, 60, 240},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 260,
		.maxhealth = 130,
		.gunoffset = {40.3f, 0, 0.478f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_HARM, WI_ECM, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 4, 4, 1, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 11000,
		.radarRange2 = 8500,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {8,0,42,42},
	},


    {
		.descriptiveName = "F-18 Hornet (ER)",
		.tinyName = "F-18",
		.modelName = "f-18",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR,
		.turnspeed = {70, 60, 240},
		.cam_dist = {0, 100, 70},
		.cam_height = {0, 100, 20},
		.stallspeed = 200,
		.maxspeed = 720,
		.maxthrottle = 15,
		.engines = 2,
		.accel = 260,
		.maxhealth = 130,
		.gunoffset = {40.3f, 0, 0.478f},
		.maxfuel = 60,
		.weapons = { WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_DROPTANK_PAIR, WI_SPARROW, 0, 0, WI_FLARE },
		.ammo = { 450, 2, 2, 2, 6, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 11000,
		.radarRange2 = 8500,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {8,0,42,42},
	},

/*
    {	"F-22 Raptor",		    // descriptiveName
	"F-22",						// tinyName
	"f-22",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_ANY,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_WEAPONBAY,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 50, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	720,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	130,						// health
	{40.3f, 0, 0.478f},			// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_MAVERICK, 0, 0, 0, WI_FLARE,	// weapons
	450, 2, 4, 2, 0, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	7500,						// radar range
	6500,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"YF-23",			    // descriptiveName
	"YF-23",					// tinyName
	"yf-23",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_ANY,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_WEAPONBAY,			// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 40, 260},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	220,						// stallspeed
	880,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	150,						// health
	{19.1f,7.87f,-0.112f},		// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_AMRAAM, WI_SIDEWINDER, 0, 0, 0, 0, WI_FLARE,	// weapons
	450, 4, 4, 0, 0, 0, 0, 35,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{28, 0, 6},					// cameraposition for cockpit view
	AB_RED,						// effect model
	14000,						// radar range
	7000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{6,0,48,34},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },
*/
	/*
    {	"SU-27 TEST",			    // descriptiveName
	"SU-27",					// tinyName
	"su-27",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_ANY,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,			// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 50, 260},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	220,						// stallspeed
	880,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	150,						// health
	{19.1f,7.87f,-0.112f},		// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_AMRAAM, WI_SIDEWINDER, 0, 0, 0, 0, WI_FLARE,	// weapons
	450, 4, 4, 0, 0, 0, 0, 35,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{28, 0, 6},					// cameraposition for cockpit view
	AB_RED,						// effect model
	11000,						// radar range
	6000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{6,0,48,34},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },
	*/

    {
		.descriptiveName = "B-2 Spirit",
		.tinyName = "B-2",
		.modelName = "b-2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.flags = VF_STEALTH,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_WEAPONBAY,
		.turnspeed = {50, 30, 150},
		.cam_dist = {0, 120, 100},
		.cam_height = {0, 100, 35},
		.stallspeed = 200,
		.maxspeed = 700,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 260,
		.maxhealth = 450,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { 0, WI_MK83, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 0, 32, 0, 0, 0, 0, 0, 60 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 5000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 100,
		.shadowCoords = {3,0,76,76},
	},

	{
		.descriptiveName = "B-2 Doomsday 10MT NB",
		.tinyName = "B-2",
		.modelName = "b-2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.flags = VF_STEALTH|VF_TESTONLY,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_WEAPONBAY,
		.turnspeed = {50, 30, 150},
		.cam_dist = {0, 120, 100},
		.cam_height = {0, 100, 35},
		.stallspeed = 200,
		.maxspeed = 500,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 260,
		.maxhealth = 450,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { 0, WI_NB10MT, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 0, 1, 0, 0, 0, 0, 0, 35 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 5000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 100,
		.shadowCoords = {3,0,76,76},
	},

	{
		.descriptiveName = "B-2 Doomsday 5MT NB",
		.tinyName = "B-2",
		.modelName = "b-2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.flags = VF_STEALTH|VF_TESTONLY,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_WEAPONBAY,
		.turnspeed = {50, 30, 150},
		.cam_dist = {0, 120, 100},
		.cam_height = {0, 100, 35},
		.stallspeed = 200,
		.maxspeed = 500,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 260,
		.maxhealth = 450,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { 0, WI_NB5MT, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 0, 2, 0, 0, 0, 0, 0, 40 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 5000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 100,
		.shadowCoords = {3,0,76,76},
	},

	{
		.descriptiveName = "B-2 Doomsday 1MT NB",
		.tinyName = "B-2",
		.modelName = "b-2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.flags = VF_STEALTH|VF_TESTONLY,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_WEAPONBAY,
		.turnspeed = {50, 30, 150},
		.cam_dist = {0, 120, 100},
		.cam_height = {0, 100, 35},
		.stallspeed = 200,
		.maxspeed = 500,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 260,
		.maxhealth = 450,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { 0, WI_NB1MT, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 0, 4, 0, 0, 0, 0, 0, 40 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 5000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 100,
		.shadowCoords = {3,0,76,76},
	},

	{
		.descriptiveName = "B-2 Doomsday 5MT NM",
		.tinyName = "B-2",
		.modelName = "b-2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.flags = VF_STEALTH|VF_TESTONLY,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_WEAPONBAY,
		.turnspeed = {50, 30, 150},
		.cam_dist = {0, 120, 100},
		.cam_height = {0, 100, 35},
		.stallspeed = 200,
		.maxspeed = 500,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 260,
		.maxhealth = 450,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { 0, WI_NM5MT, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 0, 1, 0, 0, 0, 0, 0, 30 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 5000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 100,
		.shadowCoords = {3,0,76,76},
	},

	{
		.descriptiveName = "B-2 Doomsday 1MT NM",
		.tinyName = "B-2",
		.modelName = "b-2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
		.flags = VF_STEALTH|VF_TESTONLY,
		.caps = HC_GEAR|HC_SPEEDBRAKE| HC_VAPOR|HC_WEAPONBAY,
		.turnspeed = {50, 30, 150},
		.cam_dist = {0, 120, 100},
		.cam_height = {0, 100, 35},
		.stallspeed = 200,
		.maxspeed = 500,
		.maxthrottle = 10,
		.engines = 2,
		.accel = 260,
		.maxhealth = 450,
		.gunoffset = {16.65f, -3.391f, 0.378f},
		.maxfuel = 60,
		.weapons = { 0, WI_NM1MT, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 0, 2, 0, 0, 0, 0, 0, 35 },
		.cockpitview = {19, 0, 5},
		.effectModel = AB_BALL,
		.radarRange = 5000,
		.radarRange2 = 9000,
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 100,
		.shadowCoords = {3,0,76,76},
	},

    {
		.descriptiveName = "P-51d Mustang",
		.tinyName = "P-51d",
		.modelName = "p-51d",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_1,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {60, 60, 200},
		.cam_dist = {0, 100, 50},
		.cam_height = {0, 100, 10},
		.stallspeed = 90,
		.maxspeed = 420,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.5f, -9.2f, 0.136f},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_6XCAL50, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-4, 0, 8},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {2,0,30,26},
	},

    {
		.descriptiveName = "Spitfire Mk 5b",
		.tinyName = "Spitfire",
		.modelName = "spitfire_mk5b",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_1,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {80, 80, 240},
		.cam_dist = {0, 100, 65},
		.cam_height = {0, 100, 13},
		.stallspeed = 85,
		.maxspeed = 400,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.3, -9.6, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-5.2, 0, 7.8},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-5.2, 0, 28.6, 26},
	},

    {
		.descriptiveName = "Messerschmitt Bf-109g",
		.tinyName = "Bf-109",
		.modelName = "bf-109g",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_2,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_GEAR|HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {70, 65, 240},
		.cam_dist = {0, 100, 68},
		.cam_height = {0, 100, 13.6},
		.stallspeed = 90,
		.maxspeed = 410,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.4, -10, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-5.4, 0, 8.2},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-4.1, 0, 24.5, 21.8},
	},

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
	},


    {
		.descriptiveName = "B-17g",
		.tinyName = "B-17",
		.modelName = "b17g",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_1,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_BOMBER,
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
	},

    {
		.descriptiveName = "Fokker Dr.1",
		.tinyName = "Fokker",
		.modelName = "dr1",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW1,
		.team = MF_TEAM_1,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {70, 80, 320},
		.cam_dist = {0, 100, 43.2},
		.cam_height = {0, 100, 14.4},
		.stallspeed = 50,
		.maxspeed = 150,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.6, -2, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_2XCAL312, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-5.8, 0, 8.6},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-4.3, 0, 18.7, 17.3},
	},

    {
		.descriptiveName = "Sopwith camel",
		.tinyName = "Camel",
		.modelName = "camel",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW1,
		.team = MF_TEAM_2,
		.cat = CAT_PLANE,
		.cls = CLASS_PLANE_FIGHTER,
		.caps = HC_PROP| HC_TAILDRAGGER| HC_DUALGUNS,
		.turnspeed = {60, 70, 260},
		.cam_dist = {0, 100, 42.3},
		.cam_height = {0, 100, 14.1},
		.stallspeed = 55,
		.maxspeed = 170,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 220,
		.maxhealth = 100,
		.gunoffset = {3.6, -1.9, -0.2},
		.maxfuel = 60,
		.tailangle = -14,
		.weapons = { WI_MG_2XCAL303, 0, 0, 0, 0, 0, 0, 0 },
		.ammo = { 450, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {-5.6, 0, 8.5},
		.trackCone2 = -1,
		.gearTime = 1400,
		.bayTime = 1000,
		.shadowCoords = {-1.4, 0, 21.1, 19.7},
	},

	// ---------------------------------------------
	// Ground Vehicles
	// ---------------------------------------------

    {
		.descriptiveName = "M1A1i Abrams",
		.tinyName = "M1",
		.modelName = "m1",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_MBT,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 35.6},
		.cam_height = {0, 100, 17.8},
		.maxspeed = 60,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 25,
		.maxhealth = 400,
		.gunoffset = {4.5, 0, 7.1},
		.maxfuel = 150,
		.gearheight = 310,
		.tailangle = 3,
		.weapons = { WI_MG_12_7MM, WI_125MM_GUN, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE },
		.ammo = { 500, 60, 4, 2, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 12.5},
		.radarRange = 9000,
		.radarRange2 = 6000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	},

    {
		.descriptiveName = "T-90",
		.tinyName = "T-90",
		.modelName = "t-90",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_MBT,
		.flags = VF_COOKOFF,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 36},
		.cam_height = {0, 100, 18},
		.maxspeed = 65,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 25,
		.maxhealth = 380,
		.gunoffset = {22.5, 0, 1.8},
		.maxfuel = 150,
		.gearheight = 310,
		.tailangle = 3,
		.weapons = { WI_MG_12_7MM, WI_125MM_GUN, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE },
		.ammo = { 500, 60, 4, 2, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 12.6},
		.radarRange = 9000,
		.radarRange2 = 6000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	},
/*
    {	"BMP 3",				// descriptiveName
	"BMP-3",					// tinyName
	"bmp-3",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_ANY,
	CAT_GROUND,
	CLASS_GROUND_RECON,
	0,							// flags
	HC_DUALGUNS|HC_AMPHIBIOUS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	75,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	30,							// acceleration
	200,						// health
	{8, 1, 2},					// gun tag
	160,						// max fuel
	300,						// max gun pitch (upwards = negative) <- gearheight
	6,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_14_5MM, WI_100MM_GUN, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE,// weapons
	500, 45, 2, 2, 0, 0, 0, 20,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 10},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	5000,						// radar range ground
	-1,							// trackcone
	0,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },*/
{
		.descriptiveName = "Fast Attack Vehicle",
		.tinyName = "FAV",
		.modelName = "fav",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_RECON,
		.caps = HC_WHEELS,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 38.4},
		.cam_height = {0, 100, 12.8},
		.maxspeed = 120,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.wheels = 4,
		.wheelCF = 10.2,
		.accel = 50,
		.maxhealth = 130,
		.gunoffset = {10.2, 0, 2.6},
		.maxfuel = 160,
		.gearheight = 300,
		.tailangle = 6,
		.weapons = { WI_MG_12_7MM, 0, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 800, 0, 0, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 12.8},
		.radarRange = 8000,
		.radarRange2 = 4000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	},

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
	},


    {
		.descriptiveName = "HMMWV (AA)",
		.tinyName = "HMVAA",
		.modelName = "humvee_avng",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_SAM,
		.caps = HC_WHEELS,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 30},
		.cam_height = {0, 100, 10},
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
		.ammo = { 300, 8, 0, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 4000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	},

    {
		.descriptiveName = "BRDM 2",
		.tinyName = "BRDM2",
		.modelName = "brdm2",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_RECON,
		.caps = HC_DUALGUNS|HC_AMPHIBIOUS|HC_WHEELS,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 30},
		.cam_height = {0, 100, 10},
		.maxspeed = 90,
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
		.weapons = { WI_MG_14_5MM, WI_STINGER, WI_HELLFIRE, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 600, 2, 4, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	},

    {
		.descriptiveName = "ZSU-23",
		.tinyName = "zsu23",
		.modelName = "ZSU-23",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_SAM,
		.caps = HC_DUALGUNS,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 35.7},
		.cam_height = {0, 100, 11.9},
		.maxspeed = 70,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 160,
		.gunoffset = {3.6, 1.2, 7.1},
		.maxfuel = 160,
		.gearheight = 285,
		.tailangle = 5,
		.weapons = { WI_MG_4X14_5MM, 0, 0, 0, 0, 0, 0, WI_CFLARE },
		.ammo = { 1200, 0, 0, 0, 0, 0, 0, 20 },
		.cockpitview = {0, 0, 11.9},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
	},

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
	},


    {
		.descriptiveName = "M4 Sherman",
		.tinyName = "M4",
		.modelName = "m4",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_WW2,
		.team = MF_TEAM_2,
		.cat = CAT_GROUND,
		.cls = CLASS_GROUND_MBT,
		.turnspeed = {50, 100, 100},
		.cam_dist = {0, 100, 21.9},
		.cam_height = {0, 100, 7.3},
		.maxspeed = 60,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 20,
		.maxhealth = 350,
		.gunoffset = {3.6, 0, 5.8},
		.maxfuel = 150,
		.gearheight = 335,
		.tailangle = 3,
		.weapons = { WI_MG_12_7MM, WI_125MM_GUN, 0, 0, 0, 0, 0, 0 },
		.ammo = { 500, 40, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10.2},
		.radarRange = 8000,
		.radarRange2 = 4000,
		.trackCone = -1,
		.gearTime = 1400,
		.bayTime = 1000,
	},

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
	},

	// ---------------------------------------------
	// Helo
	// ---------------------------------------------

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
		.maxspeed = 220,
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
	},

	{
		.descriptiveName = "UH-1 \"Huey\"",
		.tinyName = "UH-1",
		.modelName = "UH-1",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_RECON,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 46.8},
		.cam_height = {0, 100, 14.6},
		.maxspeed = 220,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 170,
		.gunoffset = {1.8, 0.6, 3.5},
		.maxfuel = 160,
		.gearheight = 285,
		.tailangle = 5,
		.flags = VF_DOORGUNS,
		.weapons = { 0, WI_MGT_7_62MM, WI_FUELCRATE, WI_HEALTHCRATE, WI_AMMOCRATE, 0, 0, WI_FLARE },
		.ammo = { 0, 1500, 3, 3, 3, 0, 0, 20 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.renderFlags = MFR_DUALPILOT,
		.cockpitview = {0, 0, 5.8},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
	},

	{
		.descriptiveName = "RAH-66 Comanche",
		.tinyName = "RAH-66",
		.modelName = "comanche",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_ATTACK,
		.flags = VF_STEALTH,
		.caps = HC_GEAR|HC_WEAPONBAY,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 80},
		.cam_height = {0, 100, 25},
		.maxspeed = 260,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 200,
		.gunoffset = {18, 0, -4},
		.maxfuel = 160,
		.weapons = { 0, WI_MGT_20MM, WI_HELLFIRE, WI_STINGER, 0, 0, 0, WI_FLARE },
		.ammo = { 0, 500, 4, 2, 0, 0, 0, 30 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1000,
		.bayTime = 500,
	},

	{
		.descriptiveName = "RAH-66 Comanche (AT)",
		.tinyName = "RAH-66",
		.modelName = "comanche",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_ATTACK,
		.flags = VF_STEALTH,
		.caps = HC_GEAR|HC_WEAPONBAY,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 80},
		.cam_height = {0, 100, 25},
		.maxspeed = 260,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 200,
		.gunoffset = {18, 0, -4},
		.maxfuel = 160,
		.weapons = { 0, WI_MGT_20MM, WI_HELLFIRE, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 0, 500, 6, 0, 0, 0, 0, 30 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1000,
		.bayTime = 500,
	},

	{
		.descriptiveName = "RAH-66 Comanche (AA)",
		.tinyName = "RAH-66",
		.modelName = "comanche",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_ATTACK,
		.flags = VF_STEALTH,
		.caps = HC_GEAR|HC_WEAPONBAY,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 80},
		.cam_height = {0, 100, 25},
		.maxspeed = 260,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 200,
		.gunoffset = {18, 0, -4},
		.maxfuel = 160,
		.weapons = { 0, WI_MGT_20MM, WI_STINGER_PAIR, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 0, 500, 6, 0, 0, 0, 0, 30 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1000,
		.bayTime = 500,
	},

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
	},

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
	},

	{
		.descriptiveName = "Mi-35 Hind",
		.tinyName = "Mi-35",
		.modelName = "mi-35",
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
		.gunoffset = {24, -2.6, -5.3},
		.maxfuel = 160,
		.weapons = { WI_MG_30MM, 0, WI_AT6, WI_GUNPOD_23MM, WI_S8, 0, 0, WI_FLARE },
		.ammo = { 750, 0, 2, 2, 2, 0, 0, 30 },
		.cockpitview = {0, 0, 10},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
		.gearTime = 1200,
	},

	// ---------------------------------------------
	// Infantry
	// ---------------------------------------------

	{
		.descriptiveName = "Sarge",
		.tinyName = "Soldier",
		.modelName = "sarge",
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
	},

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
	},

	// ---------------------------------------------
	// Boats
	// ---------------------------------------------

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
	},

	// the Huey without supplies: twice the door gun ammunition, for games without team mates to supply.
	// Added at the end so the indices of the vehicles above stay as they were
    {
		.descriptiveName = "UH-1 \"Huey\" (Gunship)",
		.tinyName = "UH-1G",
		.modelName = "UH-1",
		.shadowShader = SHADOW_DEFAULT,
		.gameset = MF_GAMESET_MODERN,
		.team = MF_TEAM_ANY,
		.cat = CAT_HELO,
		.cls = CLASS_HELO_RECON,
		.turnspeed = {50, 125, 70},
		.cam_dist = {0, 100, 46.8},
		.cam_height = {0, 100, 14.6},
		.maxspeed = 300,
		.minthrottle = -5,
		.maxthrottle = 10,
		.engines = 1,
		.accel = 33,
		.maxhealth = 170,
		.gunoffset = {1.8, 0.6, 3.5},
		.maxfuel = 160,
		.gearheight = 285,
		.tailangle = 5,
		.flags = VF_DOORGUNS,
		.weapons = { 0, WI_MGT_7_62MM, 0, 0, 0, 0, 0, WI_FLARE },
		.ammo = { 0, 3000, 0, 0, 0, 0, 0, 40 },
		.turret = { 1, 0, 0, 0, 0, 0, 0, 0 },
		.renderFlags = MFR_DUALPILOT,
		.cockpitview = {0, 0, 5.8},
		.radarRange = 8000,
		.radarRange2 = 5000,
		.trackCone = -1,
	}
};

int bg_numberOfVehicles = std::size( availableVehicles );

// just contains data of all the available vehicles
groundInstallationData_t availableGroundInstallations[] = 
{
    {
		.descriptiveName = "SAM Mk1",
		.tinyName = "SAM",
		.modelName = "turret_samup",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {90, 90, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 100,
		.weapon = WI_AASAM,
		.ammo = 4,
		.radarRange = 4500,
		.trackCone2 = 1,
		.reloadTime = 6000,
	},

    {
		.descriptiveName = "SAM Mk2",
		.tinyName = "SAM",
		.modelName = "turret_samup",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {75, 75, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 110,
		.weapon = WI_AASAM,
		.ammo = 8,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 1,
		.reloadTime = 6000,
	},

    {
		.descriptiveName = "SAM Mk3",
		.tinyName = "SAM",
		.modelName = "turret_samup",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {60, 60, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 120,
		.weapon = WI_AASAM,
		.ammo = 12,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 2,
		.reloadTime = 6000,
	},

    {
		.descriptiveName = "SAM Mk4",
		.tinyName = "SAM4",
		.modelName = "turret_samup",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {45, 45, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 130,
		.weapon = WI_AASAM,
		.ammo = 16,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 3,
		.reloadTime = 6000,
	},  

	{
		.descriptiveName = "Flak (modern)",
		.tinyName = "Flak",
		.modelName = "turret_aaa",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {45, 45, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 130,
		.weapon = WI_FLAK,
		.ammo = 16,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 3,
		.reloadTime = 6000,
	},	

	{
		.descriptiveName = "Flak (ww2)",
		.tinyName = "Flak",
		.modelName = "turret_aaa",
		.gameset = MF_GAMESET_WW2,
		.turnspeed = {45, 45, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 130,
		.weapon = WI_FLAK,
		.ammo = 16,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 3,
		.reloadTime = 6000,
	},

	{
		.descriptiveName = "AA",
		.tinyName = "AA",
		.modelName = "turret_aa",
		.gameset = MF_GAMESET_MODERN,
		.turnspeed = {45, 45, 0},
		.maxpitch = 290,
		.minpitch = 10,
		.maxhealth = 130,
		.weapon = WI_MG_20MM,
		.ammo = 5000,
		.radarRange = 4500,
		.trackCone2 = 1,
		.upgrades = 3,
		.reloadTime = 6000,
	},
};

int bg_numberOfGroundInstallations = std::size( availableGroundInstallations );


const char *gameset_items[MF_MAX_GAMESETS+1] =
{
	"Modern",
	"World War II",
	"World War I",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

const char *gameset_codes[MF_MAX_GAMESETS+1] =
{
	"modern",
	"ww2",
	"ww1",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

const char *team_items[MF_MAX_GAMESETS][MF_MAX_TEAMS+1] =
{
	// modern
	{
		"NATO",
		"Rebels",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// world war 2
	{
		"U.S.Air Force",
		"Royal Air Force",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// world war 1
	{
		"Germany",
		"England",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	}
};

const char *cat_fileRef[MF_MAX_CATEGORIES+1] =
{
	// categories
	"Plane",
	"GroundVehicle",
	"Helicopter",	// ok that way, Mark ?
	"Infantry",
	"Boat",
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

const char *cat_items[MF_MAX_CATEGORIES+1] =
{
	// categories
	"Planes",
	"Ground Vehicles",
	"Helicopters",
	"Infantry",
	"Boats",
	"Ground Installations",
	nullptr,
	nullptr,
	nullptr
};

const char *class_items[MF_MAX_CATEGORIES][MF_MAX_CLASSES+1] =
{	
	// planes
	{
		"Fighter",
		"Bomber",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// ground vehicles
	{
		"MBT",
		"Recon",
		"APC",
		"SAM",
		"AAG",
		"Transport",
		"Light",
		nullptr,
		nullptr
	},

	// helicopters
	{
		"Attack",
		"Recon",
		"Transport",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// infantry
	{
		"Special Unit",
		"Infantry",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// boats
	{
		"Patrol Boat",
		"Transport Boat",
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	},

	// free
	{
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	}
};

