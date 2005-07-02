/*
 * $Id: bg_vehicledata.c,v 1.63 2005-07-02 07:45:05 minkis Exp $
*/

#include "q_shared.h"
#include "bg_public.h"


completeLoadout_t availableLoadouts[MAX_LOADOUTS];

// just contains data of all the available vehicles
completeVehicleData_t availableVehicles[] = 
{
	// ---------------------------------------------
	// Planes
	// ---------------------------------------------
    {	"F-16 Falcon",			// descriptiveName
	"F-16",						// tinyName
	"f-16",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{85, 60, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	120,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, 0, 0, 0, 0, WI_FLARE,	// weapons  WI_MG_20MM
	450, 4, 4, 0, 0, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret - useful ??
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	12000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{4,0,34,24},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"F-16 Falcon (AG - MK82)",	    // descriptiveName
	"F-16",						// tinyName
	"f-16",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{85, 60, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	120,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_AMRAAM, WI_SIDEWINDER, WI_MK82, WI_CM, 0, 0, WI_FLARE,	// weapons  WI_MG_20MM
	450, 2, 2, 12, 1, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret - useful ??
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	12000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{4,0,34,24},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },



    {	"F-16 Falcon (AG - Maverick)",    // descriptiveName
	"F-16",						// tinyName
	"f-16",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{85, 60, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	120,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_MAVERICK, 0, 0, 0, WI_FLARE,	// weapons  WI_MG_20MM
	450, 2, 2, 12, 0, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret - useful ??
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	12000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{4,0,34,24},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-16 Falcon (AG - FFAR)",    // descriptiveName
	"F-16",						// tinyName
	"f-16",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{85, 60, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	120,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_FFAR, WI_CM, 0, 0, WI_FLARE,	// weapons  WI_MG_20MM
	450, 2, 2, 8, 1, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret - useful ??
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	12000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{4,0,34,24},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-16 Falcon (AG - Weasel)",    // descriptiveName
	"F-16",						// tinyName
	"f-16",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{85, 60, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	120,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_HARM, WI_ECM, 0, 0, WI_FLARE,	// weapons  WI_MG_20MM
	450, 2, 2, 4, 1, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret - useful ??
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	12000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{4,0,34,24},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-16 Falcon (ER)",    // descriptiveName
	"F-16",						// tinyName
	"f-16",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{85, 60, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	120,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_AMRAAM, WI_SPARROW, WI_DROPTANK_PAIR, WI_DROPTANK_SMALL, 0, 0, WI_FLARE,	// weapons  WI_MG_20MM
	450, 4, 2, 2, 1, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret - useful ??
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	12000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{4,0,34,24},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-16 Falcon (DF)",    // descriptiveName
	"F-16",						// tinyName
	"f-16",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{85, 60, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	120,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_CM, 0, 0, 0, 0, WI_FLARE,	// weapons  WI_MG_20MM
	450, 8, 1, 0, 0, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret - useful ??
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	12000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{4,0,34,24},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },



    {	"Saab JAS 39 Gripen",    // descriptiveName
	"Jas-39",					// tinyName
	"jas-39",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{90, 70, 320},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	720,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	250,						// acceleration
	110,						// health
	{19.698f, -1.328f, -3.016f},// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_CM, 0, 0, 0, WI_FLARE,		// weapons
	450, 2, 4, 1, 0, 0, 0, 28,	// ammo
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
	{6,0,36,20},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"Saab JAS 39 Gripen (AG - FFAR+)",    // descriptiveName
	"Jas-39",					// tinyName
	"jas-39",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{90, 70, 320},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	720,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	250,						// acceleration
	110,						// health
	{19.698f, -1.328f, -3.016f},// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_FFAR_LARGE, WI_MAVERICK, WI_CM, 0, 0, WI_FLARE,		// weapons
	450, 2, 4, 1, 1, 0, 0, 28,	// ammo
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
	{6,0,36,20},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"Saab JAS 39 Gripen (AG - Weasel)",    // descriptiveName
	"Jas-39",					// tinyName
	"jas-39",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{90, 70, 320},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	720,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	250,						// acceleration
	110,						// health
	{19.698f, -1.328f, -3.016f},// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_HARM, WI_MAVERICK, WI_ECM, 0, 0, WI_FLARE,		// weapons
	450, 2, 4, 1, 1, 0, 0, 28,	// ammo
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
	{6,0,36,20},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"Saab JAS 39 Gripen (ER)",    // descriptiveName
	"Jas-39",					// tinyName
	"jas-39",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{90, 70, 320},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	720,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	250,						// acceleration
	110,						// health
	{19.698f, -1.328f, -3.016f},// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_SPARROW, WI_DROPTANK_SMALL_PAIR, WI_DROPTANK_SMALL, WI_CM, 0, WI_FLARE,		// weapons
	450, 2, 2, 2, 1, 1, 0, 28,	// ammo
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
	{6,0,36,20},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"Saab JAS 39 Gripen (DF)",    // descriptiveName
	"Jas-39",					// tinyName
	"jas-39",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{90, 70, 320},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	720,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	250,						// acceleration
	110,						// health
	{19.698f, -1.328f, -3.016f},// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_CM, 0, 0, 0, 0, WI_FLARE,		// weapons
	450, 6, 1, 0, 0, 0, 0, 28,	// ammo
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
	{6,0,36,20},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


/*
    {	"Harrier GR-7",		    // descriptiveName
	"GR-7",						// tinyName
	"gr7",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
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

    {	"A-10 Thunderbolt II",  // descriptiveName
	"A-10",						// tinyName
	"a10",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 60, 200},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	280,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_30MM, WI_SIDEWINDER, WI_MK84, WI_MK83, 0, 0, 0, WI_FLARE,	// weapons
	500, 3, 2, 10, 0, 0, 0, 64,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	0,							// effect model
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
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },



    {	"A-10 Thunderbolt II (Runway Killer)",  // descriptiveName
	"A-10",						// tinyName
	"a10",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 60, 200},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	280,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_30MM, WI_SIDEWINDER, WI_MAVERICK, WI_GBU15, WI_DURANDAL, 0, 0, WI_FLARE,	// weapons
	500, 3, 2, 2, 8, 0, 0, 64,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	0,							// effect model
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
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"A-10 Thunderbolt II (FFAR)",  // descriptiveName
	"A-10",						// tinyName
	"a10",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 60, 200},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	280,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_30MM, WI_CM, WI_SIDEWINDER, WI_FFAR, WI_FFAR_LARGE, 0, 0, WI_FLARE,	// weapons
	500, 1, 3, 3, 2, 0, 0, 64,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	0,							// effect model
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
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"A-10 Thunderbolt II (CAS)",  // descriptiveName
	"A-10",						// tinyName
	"a10",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 60, 200},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	280,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_30MM, WI_ECM, WI_CM, WI_SIDEWINDER, WI_FFAR, WI_MAVERICK, WI_MK82, WI_FLARE,	// weapons
	500, 1, 1, 2, 1, 2, 13, 64,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	0,							// effect model
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
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"A-10 Thunderbolt II (ER)",  // descriptiveName
	"A-10",						// tinyName
	"a10",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 60, 200},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	280,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_30MM, WI_DROPTANK_PAIR, WI_MAVERICK, WI_SIDEWINDER, 0, 0, 0, WI_FLARE,	// weapons
	500, 2, 8, 2, 0, 0, 0, 64,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	0,							// effect model
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
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	/*
    {	"F-105 TEST",			// descriptiveName
	"F-105",					// tinyName
	"f-105",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
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


    {	"F-5 Tiger",		    // descriptiveName
	"F-5",						// tinyName
	"f-5",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 80, 380},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	100,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_2X20MM, WI_SIDEWINDER, WI_SPARROW, WI_FFAR_LARGE, 0, 0, 0, WI_FLARE,	// weapons
	500, 4, 2, 1, 0, 0, 0, 32,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	AB_RED_SMALL,				// effect model
	6000,						// radar range
	4000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-5 Tiger (AG)",		    // descriptiveName
	"F-5",						// tinyName
	"f-5",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 80, 380},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	100,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_2X20MM, WI_SIDEWINDER, WI_FFAR, WI_MK83, WI_MK84, 0, 0, WI_FLARE,	// weapons
	500, 2, 2, 2, 1, 0, 0, 32,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	AB_RED_SMALL,				// effect model
	6000,						// radar range
	4000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-5 Tiger (AG - FFAR)",		    // descriptiveName
	"F-5",						// tinyName
	"f-5",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 80, 380},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	100,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_2X20MM, WI_SIDEWINDER, WI_FFAR, WI_FFAR_LARGE, WI_CM, 0, 0, WI_FLARE,	// weapons
	500, 1, 4, 1, 1, 0, 0, 32,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	AB_RED_SMALL,				// effect model
	6000,						// radar range
	4000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


   {	"F-5 Tiger (ER)",		    // descriptiveName
	"F-5",						// tinyName
	"f-5",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 80, 380},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	100,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_2X20MM, WI_SIDEWINDER, WI_SPARROW, WI_DROPTANK_SMALL_PAIR, WI_DROPTANK_SMALL, 0, 0, WI_FLARE,	// weapons
	500, 2, 2, 2, 1, 0, 0, 32,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	AB_RED_SMALL,				// effect model
	6000,						// radar range
	4000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


   {	"F-5 Tiger (DF)",		    // descriptiveName
	"F-5",						// tinyName
	"f-5",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_DUALGUNS,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 80, 380},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	620,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	100,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	80,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_2X20MM, WI_CM, WI_SIDEWINDER, 0, 0, 0, 0, WI_FLARE,	// weapons
	500, 1, 5, 0, 0, 0, 0, 32,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{11, 0, 5},					// cameraposition for cockpit view
	AB_RED_SMALL,				// effect model
	6000,						// radar range
	4000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{5,0,34,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-15E Eagle",		    // descriptiveName
	"F-15",						// tinyName
	"f-15",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{65, 50, 240},				// turnspeed around the three axis
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
	4,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_MK83, 0, 0, 0, 0, WI_FLARE,	// weapons
	450, 4, 17, 0, 0, 0, 0, 35,		// ammo
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
	MFR_DUALPILOT|MFR_BIGVAPOR,	// renderflags
	{6,0,48,34},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"F-14 Tomcat",		    // descriptiveName
	"F-14",						// tinyName
	"f-14",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_SWINGWING,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{60, 55, 220},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	220,						// stallspeed
	840,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	150,						// health
	{37.8f,-2.4f,-1.3f},		// gun tag
	70,							// max fuel
	1.5,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_SPARROW, WI_PHOENIX, WI_CM, 0, 0, WI_FLARE,	// weapons
	450, 2, 2, 4, 1, 0, 0, 35,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{28, 0, 6},					// cameraposition for cockpit view
	AB_RED,						// effect model
	20000,						// radar range
	5500,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	50,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	MFR_DUALPILOT|MFR_BIGVAPOR,	// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-14 Tomcat (ER)",		    // descriptiveName
	"F-14",						// tinyName
	"f-14",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_SWINGWING,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{60, 55, 220},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	220,						// stallspeed
	840,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	150,						// health
	{37.8f,-2.4f,-1.3f},		// gun tag
	70,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_PHOENIX, WI_SIDEWINDER, WI_DROPTANK_SMALL_PAIR, 0, 0, 0, WI_FLARE,	// weapons
	450, 6, 2, 2, 0, 0, 0, 35,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{28, 0, 6},					// cameraposition for cockpit view
	AB_RED,						// effect model
	20000,						// radar range
	5500,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	50,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	MFR_DUALPILOT|MFR_BIGVAPOR,	// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-18 Hornet",		    // descriptiveName
	"F-18",						// tinyName
	"f-18",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 60, 240},				// turnspeed around the three axis
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
	4,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_SPARROW, 0, 0, 0, WI_FLARE,	// weapons
	450, 4, 4, 4, 0, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	11000,						// radar range
	8500,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{8,0,42,42},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },



    {	"F-18 Hornet (AG)",		    // descriptiveName
	"F-18",						// tinyName
	"f-18",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 60, 240},				// turnspeed around the three axis
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
	WI_MG_20MM, WI_SIDEWINDER, WI_MK82, WI_MK83, WI_CM, 0, 0, WI_FLARE,	// weapons
	450, 2, 4, 8, 1, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	11000,						// radar range
	8500,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{8,0,42,42},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },



    {	"F-18 Hornet (AG - Maverick)",		    // descriptiveName
	"F-18",						// tinyName
	"f-18",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 60, 240},				// turnspeed around the three axis
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
	WI_MG_20MM, WI_SIDEWINDER, WI_MAVERICK, 0, 0, 0, 0, WI_FLARE,	// weapons
	450, 2, 13, 0, 0, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	11000,						// radar range
	8500,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{8,0,42,42},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-18 Hornet (AG - Weasel)",		    // descriptiveName
	"F-18",						// tinyName
	"f-18",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 60, 240},				// turnspeed around the three axis
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
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_HARM, WI_ECM, 0, 0, WI_FLARE,	// weapons
	450, 2, 4, 4, 1, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	11000,						// radar range
	8500,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{8,0,42,42},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"F-18 Hornet (ER)",		    // descriptiveName
	"F-18",						// tinyName
	"f-18",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 60, 240},				// turnspeed around the three axis
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
	WI_MG_20MM, WI_SIDEWINDER, WI_AMRAAM, WI_DROPTANK_PAIR, WI_SPARROW, 0, 0, WI_FLARE,	// weapons
	450, 2, 2, 2, 6, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	11000,						// radar range
	8500,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{8,0,42,42},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

/*
    {	"F-22 Raptor",		    // descriptiveName
	"F-22",						// tinyName
	"f-22",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
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
	MF_TEAM_2,
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
	MF_TEAM_2,
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

    {	"B-2 Spirit",		    // descriptiveName
	"B-2",						// tinyName
	"b-2",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_WEAPONBAY,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 30, 150},				// turnspeed around the three axis
	{0, 120, 100},				// camera distance {min,max,default}
	{0, 100, 35},				// camera height {min,max,default}
	200,						// stallspeed
	700,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	300,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	0, WI_MK83, 0, 0, 0, 0, 0, WI_CFLARE,	// weapons
	0, 32, 0, 0, 0, 0, 0, 60,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	5000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	100,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{3,0,76,76},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	{	"B-2 Doomsday 10MT NB",	// descriptiveName
	"B-2",						// tinyName
	"b-2",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_WEAPONBAY,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 30, 150},				// turnspeed around the three axis
	{0, 120, 100},				// camera distance {min,max,default}
	{0, 100, 35},				// camera height {min,max,default}
	200,						// stallspeed
	500,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	300,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	0, WI_NB10MT, 0, 0, 0, 0, 0, WI_CFLARE,	// weapons
	0, 1, 0, 0, 0, 0, 0, 35,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	5000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	100,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{3,0,76,76},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	{	"B-2 Doomsday 5MT NB",	// descriptiveName
	"B-2",						// tinyName
	"b-2",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_WEAPONBAY,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 30, 150},				// turnspeed around the three axis
	{0, 120, 100},				// camera distance {min,max,default}
	{0, 100, 35},				// camera height {min,max,default}
	200,						// stallspeed
	500,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	300,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	0, WI_NB5MT, 0, 0, 0, 0, 0, WI_CFLARE,	// weapons
	0, 2, 0, 0, 0, 0, 0, 40,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	5000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	100,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{3,0,76,76},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	{	"B-2 Doomsday 1MT NB",	// descriptiveName
	"B-2",						// tinyName
	"b-2",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_WEAPONBAY,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 30, 150},				// turnspeed around the three axis
	{0, 120, 100},				// camera distance {min,max,default}
	{0, 100, 35},				// camera height {min,max,default}
	200,						// stallspeed
	500,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	300,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	0, WI_NB1MT, 0, 0, 0, 0, 0, WI_CFLARE,	// weapons
	0, 4, 0, 0, 0, 0, 0, 40,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	5000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	100,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{3,0,76,76},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	{	"B-2 Doomsday 5MT NM",	// descriptiveName
	"B-2",						// tinyName
	"b-2",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_WEAPONBAY,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 30, 150},				// turnspeed around the three axis
	{0, 120, 100},				// camera distance {min,max,default}
	{0, 100, 35},				// camera height {min,max,default}
	200,						// stallspeed
	500,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	300,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	0, WI_NM5MT, 0, 0, 0, 0, 0, WI_CFLARE,	// weapons
	0, 1, 0, 0, 0, 0, 0, 30,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	5000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	100,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{3,0,76,76},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	{	"B-2 Doomsday 1MT NM",	// descriptiveName
	"B-2",						// tinyName
	"b-2",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_SPEEDBRAKE|
	HC_VAPOR|HC_WEAPONBAY,		// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 30, 150},				// turnspeed around the three axis
	{0, 120, 100},				// camera distance {min,max,default}
	{0, 100, 35},				// camera height {min,max,default}
	200,						// stallspeed
	500,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	2,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	300,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	0, WI_NM1MT, 0, 0, 0, 0, 0, WI_CFLARE,	// weapons
	0, 2, 0, 0, 0, 0, 0, 35,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{19, 0, 5},					// cameraposition for cockpit view
	AB_BALL,					// effect model
	5000,						// radar range
	9000,						// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	100,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{3,0,76,76},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"P-51d Mustang",		// descriptiveName
	"P-51d",					// tinyName
	"p-51d",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW2,				// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_PROP|
	HC_TAILDRAGGER|
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{60, 60, 200},				// turnspeed around the three axis
	{0, 100, 50},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	90,						    // stallspeed
	420,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	100,						// health
	{3.5f, -9.2f, 0.136f},		// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	-14,						// tailangle
	WI_MG_6XCAL50, 0, 0, 0, 0, 0, 0, 0,		// weapons
	450, 0, 0, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{-4, 0, 8},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{2,0,30,26},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"Spitfire Mk 5b",		// descriptiveName
	"Spitfire",					// tinyName
	"spitfire_mk5b",			// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW2,				// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_PROP|
	HC_TAILDRAGGER|
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 80, 240},				// turnspeed around the three axis
	{0, 100, 50},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	85,						    // stallspeed
	400,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	100,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	-14,							// tailangle
	WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0,		// weapons
	450, 0, 0, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{-4, 0, 6},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{-4,0,22,20},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"Messerschmitt Bf-109g",// descriptiveName
	"Bf-109",					// tinyName
	"bf-109g",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW2,				// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_PROP|
	HC_TAILDRAGGER|
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 65, 240},				// turnspeed around the three axis
	{0, 100, 50},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	90,						    // stallspeed
	410,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	100,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	-14,							// tailangle
	WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0,		// weapons
	450, 0, 0, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{-4, 0, 6},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{-3,0,18,16},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"Focke-Wulf Fw190 A8",// descriptiveName
	"Fw-190",					// tinyName
	"fw190a8",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW2,				// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_GEAR|HC_PROP|
	HC_TAILDRAGGER|
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{60, 60, 220},				// turnspeed around the three axis
	{0, 100, 50},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	90,						    // stallspeed
	420,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	100,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	-14,							// tailangle
	WI_MG_8XCAL50, 0, 0, 0, 0, 0, 0, 0,		// weapons
	450, 0, 0, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{-4, 0, 6},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{-2,0,18,18},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },


    {	"B-17g",				// descriptiveName
	"B-17",						// tinyName
	"b17g",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW2,				// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_BOMBER,
	0,							// flags
	HC_GEAR|HC_PROP|
	HC_TAILDRAGGER|
	HC_WEAPONBAY,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{40, 30, 100},				// turnspeed around the three axis
	{0, 100, 80},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	100,					    // stallspeed
	320,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	4,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	1500,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	-8,							// tailangle
	WI_MG_8XCAL50, WI_MK83, WI_MK83, 0, 0, 0, 0, 0,		// weapons
	450, 16, 16, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{-4, 0, 6},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	500,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{-12,0,60,60},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
	},

    {	"Fokker Dr.1",			// descriptiveName
	"Fokker",					// tinyName
	"dr1",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW1,				// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_PROP|
	HC_TAILDRAGGER|
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 80, 320},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	50,						    // stallspeed
	150,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	100,						// health
	{2.518f, -1.371f, -0.155f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	-14,							// tailangle
	WI_MG_2XCAL312, 0, 0, 0, 0, 0, 0, 0,		// weapons
	450, 0, 0, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{-4, 0, 6},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{-3,0,13,12},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"Sopwith camel",		// descriptiveName
	"Camel",					// tinyName
	"camel",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW1,				// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_PROP|
	HC_TAILDRAGGER|
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{60, 70, 260},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	55,						    // stallspeed
	170,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	100,						// health
	{2.518f, -1.371f, -0.155f},	// gun tag
	60,							// max fuel
	0,							// gearheight - SET AUTOMATICALLY
	-14,							// tailangle
	WI_MG_2XCAL303, 0, 0, 0, 0, 0, 0, 0,		// weapons
	450, 0, 0, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{-4, 0, 6},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	0,							// trackcone
	-1,							// trackcone ground
	0,							// swingangle
	1400,						// geartime
	0,							// max gear frame
	1000,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{-1,0,15,14},				// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	// ---------------------------------------------
	// Ground Vehicles
	// ---------------------------------------------

    {	"M1A1i Abrams",			// descriptiveName
	"M1",						// tinyName
	"m1",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_GROUND,
	CLASS_GROUND_MBT,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 40},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	0,							// stallspeed
	60,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	25,							// acceleration
	400,						// health
	{5, 0, 8},					// gun tag
	150,						// max fuel
	310,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_12_7MM, WI_125MM_GUN, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE,// weapons
	500, 60, 4, 2, 0, 0, 0, 20,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 14},					// cameraposition for cockpit view
	0,							// effect model
	9000,						// radar range
	6000,						// radar range ground
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
    },

    {	"T-90",					// descriptiveName
	"T-90",						// tinyName
	"t-90",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_GROUND,
	CLASS_GROUND_MBT,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 40},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	0,							// stallspeed
	65,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	25,							// acceleration
	380,						// health
	{25, 0, 2},					// gun tag
	150,						// max fuel
	310,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_12_7MM, WI_125MM_GUN, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE,// weapons
	500, 60, 4, 2, 0, 0, 0, 20,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 14},					// cameraposition for cockpit view
	0,							// effect model
	9000,						// radar range
	6000,						// radar range ground
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
    },
/*
    {	"BMP 3",				// descriptiveName
	"BMP-3",					// tinyName
	"bmp-3",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
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

    {	"Hummer",				// descriptiveName
	"HMV",						// tinyName
	"humvee_50cal",				// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_GROUND,
	CLASS_GROUND_RECON,
	0,							// flags
	HC_WHEELS,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	85,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	8,							// wheel circumference
	35,							// acceleration
	130,						// health
	{8, 0, 2},					// gun tag
	160,						// max fuel
	300,						// max gun pitch (upwards = negative) <- gearheight
	6,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_12_7MM, WI_STINGER, 0, 0, 0, 0, 0, WI_CFLARE,// weapons
	500, 4, 0, 0, 0, 0, 0, 20,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 10},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
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
    },


    {	"Hummer (AA)",			// descriptiveName
	"HMVAA",						// tinyName
	"humvee_avng",				// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_GROUND,
	CLASS_GROUND_SAM,
	0,							// flags
	HC_WHEELS,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	85,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	8,							// wheel circumference
	35,							// acceleration
	130,						// health
	{8, 0, 2},					// gun tag
	160,						// max fuel
	300,						// max gun pitch (upwards = negative) <- gearheight
	6,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_12_7MM, WI_STINGER, 0, 0, 0, 0, 0, WI_CFLARE,// weapons
	300, 8, 0, 0, 0, 0, 0, 20,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 10},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
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
    },

    {	"BRDM 2",				// descriptiveName
	"BRDM2",					// tinyName
	"brdm2",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_GROUND,
	CLASS_GROUND_RECON,
	0,							// flags
	HC_DUALGUNS|HC_AMPHIBIOUS|HC_WHEELS,	// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	90,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	9,							// wheel circumference
	33,							// acceleration
	170,						// health
	{3, 1, 6},					// gun tag
	160,						// max fuel
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_14_5MM, WI_STINGER, WI_HELLFIRE, 0, 0, 0, 0, WI_CFLARE,// weapons
	600, 2, 4, 0, 0, 0, 0, 20,	// ammo
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
    },

    {	"ZSU-23",				// descriptiveName
	"zsu23",					// tinyName
	"ZSU-23",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_GROUND,
	CLASS_GROUND_SAM,
	0,							// flags
	HC_DUALGUNS,	// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	70,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	33,							// acceleration
	160,						// health
	{3, 1, 6},					// gun tag
	160,						// max fuel
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_4X14_5MM, 0, 0, 0, 0, 0, 0, WI_CFLARE,// weapons
	1200, 0, 0, 0, 0, 0, 0, 20,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 10},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	5000,						// radar range ground
	-1,							// trackcone
	0,							// trackcone ground
	0,							// swingangle
	0,						// geartime
	0,							// max gear frame
	0,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

    {	"Panzer III",			// descriptiveName
	"Pz 3",						// tinyName
	"panzeriii",				// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW2,				// id
	MF_TEAM_1,
	CAT_GROUND,
	CLASS_GROUND_MBT,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	60,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	20,							// acceleration
	400,						// health
	{5, 0, 8},					// gun tag
	150,						// max fuel
	335,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_12_7MM, WI_125MM_GUN, 0, 0, 0, 0, 0, 0,// weapons
	500, 40, 0, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 14},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
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
    },


    {	"M4 Sherman",			// descriptiveName
	"M4",						// tinyName
	"m4",				// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW2,				// id
	MF_TEAM_2,
	CAT_GROUND,
	CLASS_GROUND_MBT,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	60,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	20,							// acceleration
	350,						// health
	{5, 0, 8},					// gun tag
	150,						// max fuel
	335,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_12_7MM, WI_125MM_GUN, 0, 0, 0, 0, 0, 0,// weapons
	500, 40, 0, 0, 0, 0, 0, 0,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 14},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
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
    },

	{	"MLRS",						// descriptiveName
	"MLRS",						// tinyName
	"mlrs",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,				// id
	MF_TEAM_1,
	CAT_GROUND,
	CLASS_GROUND_SAM,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	60,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	20,							// acceleration
	400,						// health
	{5, 0, 8},					// gun tag
	150,						// max fuel
	335,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	0, WI_HELLFIRE, 0, 0, 0, 0, 0, WI_CFLARE,// weapons
	0, 12, 0, 0, 0, 0, 0, 20,	// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 14},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	4000,						// radar range ground
	0,							// trackcone
	0,							// trackcone ground
	0,							// swingangle
	0,						// geartime
	0,							// max gear frame
	0,						// baytime
	0,							// max bay frame
	0,							// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	// ---------------------------------------------
	// Helo
	// ---------------------------------------------

	{	"helo",				// descriptiveName
	"helo",					// tinyName
	"helo",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_HELO,
	CLASS_HELO_RECON,
	0,							// flags
	0,	// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	300,						// maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	9,							// wheel circumference
	33,							// acceleration
	170,						// health
	{3, 1, 6},					// gun tag
	160,						// max fuel
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	0, WI_MGT_2X30MM, WI_STINGER, WI_HELLFIRE, 0, 0, 0, WI_CFLARE,// weapons
	0, 2, 4, 0, 0, 0, 0, 20,	// ammo
	1,2,0,0,0,0,0,0,			// turret
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
    },

	{	"UH-1 \"Huey\"",			// descriptiveName
	"UH-1",						// tinyName
	"UH-1",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_HELO,
	CLASS_HELO_RECON,
	0,							// flags
	0,	// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 125, 70},				// turnspeed around the three axis
	{0, 100, 80},				// camera distance {min,max,default}
	{0, 100, 25},				// camera height {min,max,default}
	0,							// stallspeed
	300,						// maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	33,							// acceleration
	170,						// health
	{3, 1, 6},					// gun tag
	160,						// max fuel
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	0, WI_MGT_7_62MM, WI_FUELCRATE, WI_HEALTHCRATE, WI_AMMOCRATE, 0, 0, WI_FLARE,// weapons
	0, 1500, 3, 3, 3, 0, 0, 20,	// ammo
	1,0,0,0,0,0,0,0,			// turret
	{0, 0, 10},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	5000,						// radar range ground
	-1,							// trackcone
	0,							// trackcone ground
	0,							// swingangle
	0,							// geartime
	0,							// max gear frame
	0,							// baytime
	0,							// max bay frame
	0,							// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	// ---------------------------------------------
	// Infantry
	// ---------------------------------------------

	{	"Sarge - Generic Fodder",	// descriptiveName
	"sarge",					// tinyName
	"sarge",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_LQM,
	CLASS_LQM_INFANTRY,
	0,							// flags
	0,	// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 125, 70},				// turnspeed around the three axis
	{0, 100, 80},				// camera distance {min,max,default}
	{0, 100, 25},				// camera height {min,max,default}
	0,							// stallspeed
	300,						// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	20,							// health
	{0, 0, 0},					// gun tag
	0,							// max fuel
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	0, 0, 0, 0, 0, 0, 0, 0,		// weapons
	0, 0, 0, 0, 0, 0, 0, 0,		// ammo
	0,0,0,0,0,0,0,0,			// turret
	{0, 0, 10},					// cameraposition for cockpit view
	0,							// effect model
	0,							// radar range
	0,							// radar range ground
	-1,							// trackcone
	0,							// trackcone ground
	0,							// swingangle
	0,							// geartime
	0,							// max gear frame
	0,							// baytime
	0,							// max bay frame
	0,							// renderflags
	{0,0,0,0},					// shadow coordinates
	{0,0,0,0}					// shadow orientation adjusters
    },

	// ---------------------------------------------
	// Boats
	// ---------------------------------------------

    {	"Patrolboat",			// descriptiveName
	"PBR31",					// tinyName
	"pbr31mk2",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_BOAT,
	CLASS_BOAT_PATROL,
	0,							// flags
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 20},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	50,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	15,							// acceleration
	350,						// health
	{5, 0, 8},					// gun tag
	150,						// max fuel
	8,							// max angle to lean when turning <- gearheight
	-5,							// bowangle (depends on speeed) <-tailangle 
	0, WI_MGT_2X30MM, WI_MGT_12_7MM, WI_HELLFIRE, WI_STINGER, 0, 0, WI_CFLARE,// weapons
	0, 500, 300, 2, 2, 0, 0, 20,	// ammo
	1,2,0,0,0,0,0,0,			// turret
	{0, 0, 6},					// cameraposition for cockpit view
	0,							// effect model
	8000,						// radar range
	7000,						// radar range ground
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
    }
};

int bg_numberOfVehicles = sizeof(availableVehicles) / sizeof(availableVehicles[0]);

// just contains data of all the available vehicles
groundInstallationData_t availableGroundInstallations[] = 
{
    {	"SAM Mk1",				// descriptiveName
	"SAM",						// tinyName
	"turret_samup",				// modelName
	MF_GAMESET_MODERN,			// gameset
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box)
	{0, 0, 0},					// max (bounding box)
	{90, 90, 0},				// turnspeed around the three axis
	290,						// maxgunpitch
	10,							// mingunpitch
	100,						// health
	WI_AASAM,					// weapon
	4,							// ammo
	4500,						// radar range
	0,							// radar range ground
	0,							// trackcone
	1,							// trackcone ground
	0,							// upgrades
	6000						// reloadtime
    },

    {	"SAM Mk2",				// descriptiveName
	"SAM",						// tinyName
	"turret_samup",				// modelName
	MF_GAMESET_MODERN,			// gameset
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box)
	{0, 0, 0},					// max (bounding box)
	{75, 75, 0},				// turnspeed around the three axis
	290,						// maxgunpitch
	10,							// mingunpitch
	110,						// health
	WI_AASAM,					// weapon
	8,							// ammo
	4500,						// radar range
	0,							// radar range ground
	0,							// trackcone
	1,							// trackcone ground
	1,							// upgrades
	6000						// reloadtime
    },

    {	"SAM Mk3",				// descriptiveName
	"SAM",						// tinyName
	"turret_samup",				// modelName
	MF_GAMESET_MODERN,			// gameset
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box)
	{0, 0, 0},					// max (bounding box)
	{60, 60, 0},				// turnspeed around the three axis
	290,						// maxgunpitch
	10,							// mingunpitch
	120,						// health
	WI_AASAM,					// weapon
	12,							// ammo
	4500,						// radar range
	0,							// radar range ground
	0,							// trackcone
	1,							// trackcone ground
	2,							// upgrades
	6000						// reloadtime
    },

    {	"SAM Mk4",				// descriptiveName
	"SAM4",						// tinyName
	"turret_samup",				// modelName
	MF_GAMESET_MODERN,			// gameset
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box)
	{0, 0, 0},					// max (bounding box)
	{45, 45, 0},				// turnspeed around the three axis
	290,						// maxgunpitch
	10,							// mingunpitch
	130,						// health
	WI_AASAM,					// weapon
	16,							// ammo
	4500,						// radar range
	0,							// radar range ground
	0,							// trackcone
	1,							// trackcone ground
	3,							// upgrades
	6000						// reloadtime
    },  

	{	"Flak (modern)",		// descriptiveName
	"Flak",						// tinyName
	"turret_samup",				// modelName
	MF_GAMESET_MODERN,			// gameset
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box)
	{0, 0, 0},					// max (bounding box)
	{45, 45, 0},				// turnspeed around the three axis
	290,						// maxgunpitch
	10,							// mingunpitch
	130,						// health
	WI_FLAK,					// weapon
	16,							// ammo
	4500,						// radar range
	0,							// radar range ground
	0,							// trackcone
	1,							// trackcone ground
	3,							// upgrades
	6000						// reloadtime
    },	

	{	"Flak (ww2)",			// descriptiveName
	"Flak",						// tinyName
	"turret_samup",				// modelName
	MF_GAMESET_WW2,				// gameset
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box)
	{0, 0, 0},					// max (bounding box)
	{45, 45, 0},				// turnspeed around the three axis
	290,						// maxgunpitch
	10,							// mingunpitch
	130,						// health
	WI_FLAK,					// weapon
	16,							// ammo
	4500,						// radar range
	0,							// radar range ground
	0,							// trackcone
	1,							// trackcone ground
	3,							// upgrades
	6000						// reloadtime
    },

	{	"AAA (modern)",			// descriptiveName
	"AAA",						// tinyName
	"turret_samup",				// modelName
	MF_GAMESET_MODERN,			// gameset
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box)
	{0, 0, 0},					// max (bounding box)
	{45, 45, 0},				// turnspeed around the three axis
	290,						// maxgunpitch
	10,							// mingunpitch
	130,						// health
	WI_MG_20MM,					// weapon
	5000,						// ammo
	4500,						// radar range
	0,							// radar range ground
	0,							// trackcone
	1,							// trackcone ground
	3,							// upgrades
	6000						// reloadtime
    },
};

int bg_numberOfGroundInstallations = sizeof(availableGroundInstallations) / 
								sizeof(availableGroundInstallations[0]);


const char *gameset_items[MF_MAX_GAMESETS+1] =
{
	"Modern",
	"World War II",
	"World War I",
	0,
	0,
	0,
	0,
	0,
	0
};

const char *gameset_codes[MF_MAX_GAMESETS+1] =
{
	"modern",
	"ww2",
	"ww1",
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

	// world war 1
	{
		"Germany",
		"England",
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

const char *cat_fileRef[MF_MAX_CATEGORIES+1] =
{
	// categories
	"Plane",
	"GroundVehicle",
	"Helicopter",	// ok that way, Mark ?
	"Infantry",
	"Boat",
	0,
	0,
	0,
	0
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
		"Infantry",
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	// boats
	{
		"Patrol Boat",
		"Transport Boat",
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

