/*
 * $Id: bg_vehicledata.c,v 1.4 2001-12-23 02:02:14 thebjoern Exp $
*/

#include "q_shared.h"
#include "bg_public.h"


// just contains data of all the available vehicles
completeVehicleData_t availableVehicles[] = 
{
	// ---------------------------------------------
	// Planes
	// ---------------------------------------------
    {	"F-16 Falcon",		    // descriptiveName
	"f-16",						// modelName
	MF_GAMESET_MODERN|			// id
	MF_TEAM_2|
	CAT_PLANE|
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-26, -21, -4},				// mins (bounding box)
	{36, 21, 15},				// max (bounding box)
	{80, 80, 260},				// turnspeed around the three axis
	80, 20,						// camera distance and height
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	260,						// acceleration
	130,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	3,							// gearheight
	0,							// tailangle
	WI_MG_20MM, WI_FFAR, WI_MK82, WI_SIDEWINDER, WI_AMRAAM, WI_PHOENIX, WI_HELLFIRE, WI_FLARE,	// weapons
	450, 24, 8, 4, 4, 4, 8, 30,		// ammo
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	8000,						// radar range
	4000,						// radar range ground
	0							// renderflags
    },

    {	"Saab JAS 39 Gripen",    // descriptiveName
	"jas-39",					// modelName
	MF_GAMESET_MODERN|			// id
	MF_TEAM_1|
	CAT_PLANE|
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-26, -19, -4},				// mins (bounding box)
	{38, 19, 12},				// max (bounding box)
	{70, 80, 340},				// turnspeed around the three axis
	80, 20,						// camera distance and height
	200,						// stallspeed
	720,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	250,						// acceleration
	130,						// health
	{19.698f, -1.328f, -3.016f},// gun tag
	60,							// max fuel
	3,							// gearheight
	0,							// tailangle
	WI_MG_20MM, WI_FFAR, WI_MK82, 0, 0, 0, 0, WI_FLARE,		// weapons
	450, 24, 8, 0, 0, 0, 0, 28,	// ammo
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	10000,						// radar range
	3000,						// radar range ground
	0							// renderflags
    },

    {	"F-5 Tiger",		    // descriptiveName
	"f-5",						// modelName
	MF_GAMESET_MODERN|			// id
	MF_TEAM_2|
	CAT_PLANE|
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALENGINE|HC_DUALGUNS,	// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-24, -16, -1.5},				// mins (bounding box)
	{36, 16, 11},				// max (bounding box)
	{70, 80, 380},				// turnspeed around the three axis
	76, 18,						// camera distance and height
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	270,						// acceleration
	100,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	3,							// gearheight
	0,							// tailangle
	WI_MG_2X20MM, WI_FFAR, WI_MK82, 0, 0, 0, 0, WI_FLARE,	// weapons
	500, 12, 4, 0, 0, 0, 0, 32,		// ammo
	{11, 0, 5},					// cameraposition for cockpit view
	AB_RED_SMALL,				// effect model
	6000,						// radar range
	2000,						// radar range ground
	0							// renderflags
    },

    {	"F-15 Eagle",		    // descriptiveName
	"f-15",						// modelName
	MF_GAMESET_MODERN|			// id
	MF_TEAM_1|
	CAT_PLANE|
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALENGINE,		// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-31, -28, -5},				// mins (bounding box)
	{54, 28, 14},				// max (bounding box)
	{60, 80, 200},				// turnspeed around the three axis
	90, 18,						// camera distance and height
	220,						// stallspeed
	880,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	260,						// acceleration
	150,						// health
	{19.1f,7.87f,-0.112f},		// gun tag
	80,							// max fuel
	3,							// gearheight
	0,							// tailangle
	WI_MG_20MM, WI_FFAR, WI_MK82, 0, 0, 0, 0, WI_FLARE,	// weapons
	450, 12, 12, 0, 0, 0, 0, 35,		// ammo
	{28, 0, 6},					// cameraposition for cockpit view
	AB_RED,						// effect model
	12000,						// radar range
	6000,						// radar range ground
	MFR_DUALPILOT|MFR_BIGVAPOR	// renderflags
    },

    {	"P-51d Mustang",		// descriptiveName
	"p-51d",					// modelName
	MF_GAMESET_WW2|				// id
	MF_TEAM_1|
	CAT_PLANE|
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_PROP|
	HC_TAILDRAGGER|
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-28, -26, -3},				// mins (bounding box)
	{16, 26, 10},				// max (bounding box)
	{60, 80, 240},				// turnspeed around the three axis
	60, 16,						// camera distance and height
	130,					    // stallspeed
	600,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	220,						// acceleration
	100,						// health
	{3.5f, -9.2f, 0.136f},		// gun tag
	60,							// max fuel
	3,							// gearheight
	-14,						// tailangle
	WI_MG_6XCAL50, 0, 0, 0, 0, 0, 0, 0,		// weapons
	450, 0, 0, 0, 0, 0, 0, 0,	// ammo
	{-4, 0, 8},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0							// renderflags
    },

    {	"Spitfire Mk 5b",		// descriptiveName
	"spitfire_mk5b",			// modelName
	MF_GAMESET_WW2|				// id
	MF_TEAM_2|
	CAT_PLANE|
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_PROP|
	HC_TAILDRAGGER|
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-21, -20, -1},				// mins (bounding box)
	{10, 20, 6},				// max (bounding box)
	{70, 80, 300},				// turnspeed around the three axis
	60, 16,						// camera distance and height
	120,					    // stallspeed
	500,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	220,						// acceleration
	100,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	60,							// max fuel
	3,							// gearheight
	-14,							// tailangle
	WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0,		// weapons
	450, 0, 0, 0, 0, 0, 0, 0,	// ammo
	{-4, 0, 6},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0							// renderflags
    },

	// ---------------------------------------------
	// Ground Vehicles
	// ---------------------------------------------

    {	"M1A1i Abrams",			// descriptiveName
	"m1",						// modelName
	MF_GAMESET_MODERN|			// id
	MF_TEAM_1|
	CAT_GROUND|
	CLASS_GROUND_MBT,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-18, -9, -1},				// mins (bounding box)
	{18, 9, 9},					// max (bounding box)
	{50, 100, 100},				// turnspeed around the three axis
	44, 14,						// camera distance and height
	0,							// stallspeed
	60,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	15,							// acceleration
	400,						// health
	{5, 0, 8},					// gun tag
	150,						// max fuel
	310,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_12_7MM, WI_125MM_GUN, WI_STINGER, 0, 0, 0, 0, WI_FLARE,// weapons
	500, 40, 10, 0, 0, 0, 0, 20,	// ammo
	{0, 0, 14},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
	0							// renderflags
    },

    {	"T-90",					// descriptiveName
	"t-90",						// modelName
	MF_GAMESET_MODERN|			// id
	MF_TEAM_2|
	CAT_GROUND|
	CLASS_GROUND_MBT,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-17, -9, -1},				// mins (bounding box)
	{17, 9, 7},					// max (bounding box)
	{50, 100, 100},				// turnspeed around the three axis
	44, 14,						// camera distance and height
	0,							// stallspeed
	65,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	15,							// acceleration
	450,						// health
	{25, 0, 2},					// gun tag
	150,						// max fuel
	310,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_12_7MM, WI_125MM_GUN, 0, 0, 0, 0, 0, WI_FLARE,// weapons
	500, 40, 0, 0, 0, 0, 0, 20,	// ammo
	{0, 0, 14},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
	0							// renderflags
    },

    {	"BMP 3",				// descriptiveName
	"bmp-3",					// modelName
	MF_GAMESET_MODERN|			// id
	MF_TEAM_1|
	CAT_GROUND|
	CLASS_GROUND_RECON,
	0,							// flags
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-10, -5, -1},				// mins (bounding box)
	{11, 5, 4.5},					// max (bounding box)
	{50, 100, 100},				// turnspeed around the three axis
	38, 14,						// camera distance and height
	0,							// stallspeed
	75,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	18,							// acceleration
	240,						// health
	{8, 1, 2},					// gun tag
	160,						// max fuel
	300,						// max gun pitch (upwards = negative) <- gearheight
	6,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_14_5MM, WI_100MM_GUN, 0, 0, 0, 0, 0, WI_FLARE,// weapons
	500, 35, 0, 0, 0, 0, 0, 20,	// ammo
	{0, 0, 10},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
	0							// renderflags
    },

    {	"BRDM 2",				// descriptiveName
	"brdm2",					// modelName
	MF_GAMESET_MODERN|			// id
	MF_TEAM_2|
	CAT_GROUND|
	CLASS_GROUND_RECON,
	0,							// flags
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0},			// handles MUST BE NULL!
	{-10, -5, -2},				// mins (bounding box)
	{10, 5, 7},					// max (bounding box)
	{50, 100, 100},				// turnspeed around the three axis
	38, 14,						// camera distance and height
	0,							// stallspeed
	90,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	18,							// acceleration
	180,						// health
	{3, 1, 6},					// gun tag
	160,						// max fuel
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_14_5MM, 0, 0, 0, 0, 0, 0, WI_FLARE,// weapons
	600, 0, 0, 0, 0, 0, 0, 20,	// ammo
	{0, 0, 10},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
	0							// renderflags
    }
};

int bg_numberOfVehicles = sizeof(availableVehicles) / sizeof(availableVehicles[0]);

const char *gameset_items[MF_MAX_GAMESETS+1] =
{
	"Modern",
	"World War II",
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

const char *team_items[MF_MAX_GAMESETS][MF_MAX_TEAMS+1] =
{
	// modern
	{
		"NATO",
		"Rebels",
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// world war 2
	{
		"U.S.Air Force",
		"Royal Air Force",
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	}
};

const char *cat_items[MF_MAX_CATEGORIES+1] =
{
	// categories
	"Planes",
	"Ground Vehicles",
	0,
	0,
	0,
	0,
	0,
	0,
	0
};

const char *class_items[MF_MAX_CATEGORIES][MF_MAX_CLASSES+1] =
{	
	// planes
	{
		"Fighter",
		"Bomber",
		0,
		0,
		0,
		0,
		0,
		0,
		0
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
		0,
		0
	},

	// helicopters
	{
		"Attack",
		"Recon",
		"Transport",
		0,
		0,
		0,
		0,
		0,
		0
	},

	// infantry
	{
		"Special Unit",
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// free
	{
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	}
};

