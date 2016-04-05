/*
 * $Id: bg_vehicledata.c,v 1.7 2016-04-04 osfpsproject Exp $
*/

#include "q_shared.h"
#include "bg_public.h"


completeLoadout_t availableLoadouts[MAX_LOADOUTS];

// just contains data of all the available vehicles
completeVehicleData_t availableVehicles[] = 
{
	
	// ---------------------------------------------
	// Infantry
	// ---------------------------------------------
	
	// Coalition soldier (5,56 mm assault rifle)
{	"Soldier (5,56 mm assault rifle)",	// descriptiveName
	"coalition_soldier",		// tinyName
	"coalition_soldier",		// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_LQM,
	CLASS_LQM_INFANTRY,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 125, 70},				// turnspeed around the three axis
	//{0, 0, 0},				// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},			// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},				// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_AR_1X5_56MM, WI_FLARE, 0, 0, 0, 0, 0, 0,	// weapons
	210, 5, 0, 0, 0, 0, 0, 0,	// ammo
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
	
	// Coalition medic (7,62 mm rifle)
{	"Coalition medic (7,62 mm rifle)",	// descriptiveName
	"coalition_medic",			// tinyName
	"coalition_medic",			// modelName
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
	//{0, 0, 0},				// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},			// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},				// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_R_1X7_62MM, WI_FLARE, 0, 0, 0, 0, 0, 0,	// weapons
	125, 5, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Coalition engineer (5,7 mm submachine gun)
{	"Engineer (5,7 mm submachine gun)",	// descriptiveName
	"coalition_engineer",		// tinyName
	"coalition_engineer",		// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_LQM,
	CLASS_LQM_INFANTRY,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 125, 70},				// turnspeed around the three axis
	//{0, 0, 0},				// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},			// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},				// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_SMG_1X5_7MM, WI_FLARE, 0, 0, 0, 0, 0, 0,	// weapons
	240, 5, 0, 0, 0, 0, 0, 0,	// ammo
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
    
	// Coalition fieldops (M72law)
{	"Coalition fieldops (M72law)",		// descriptiveName
	"coalition_fieldops",		// tinyName
	"coalition_fieldops",		// modelName
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
	//{0, 0, 0},				// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},			// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},				// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_SSM_0_6KG, WI_FLARE, 0, 0, 0, 0, 0, 0,	// weapons
	6, 5, 0, 0, 0, 0, 0, 0,		// ammo
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
    
	// Coalition covertops (7,62 mm rifle)
{	"Coalition covertops (7,62 mm rifle)",	// descriptiveName
	"coalition_covertops",		// tinyName
	"coalition_covertops",		// modelName
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
	//{0, 0, 0},				// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},			// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},				// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_R_1X7_62MM, WI_FLARE, 0, 0, 0, 0, 0, 0,	// weapons
	60, 5, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Rebels soldier (5,45 mm assault rifle)
{	"Soldier (5,45 mm assault rifle)",	// descriptiveName
	"rebels_soldier",					// tinyName
	"rebels_soldier",			// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_LQM,
	CLASS_LQM_INFANTRY,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 125, 70},				// turnspeed around the three axis
	//{0, 0, 0},				// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},			// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},					// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_AR_1X5_45MM, WI_FLARE, 0, 0, 0, 0, 0, 0,	// weapons
	210, 5, 0, 0, 0, 0, 0, 0,	// ammo
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
	
	// Rebels medic (7,62 mm rifle)
{	"Medic (7,62 mm rifle)",	// descriptiveName
	"rebels_medic",					// tinyName
	"rebels_medic",					// modelName
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
	//{0, 0, 0},					// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},					// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},					// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_R_1X7_62MM, WI_FLARE, 0, 0, 0, 0, 0, 0,		// weapons
	125, 5, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Rebels engineer (5,7 mm submachine gun)
{	"Engineer (5,7 mm submachine gun)",	// descriptiveName
	"rebels_engineer",					// tinyName
	"rebels_engineer",					// modelName
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
	//{0, 0, 0},				// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},				// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},					// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_SMG_1X5_7MM, WI_FLARE, 0, 0, 0, 0, 0, 0,	// weapons
	240, 5, 0, 0, 0, 0, 0, 0,		// ammo
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
    
	// Rebels fieldops (RPG-7V2)
{	"Rebels fieldops (RPG-7V2)",	// descriptiveName
	"rebels_fieldops",			// tinyName
	"rebels_fieldops",			// modelName
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
	//{0, 0, 0},					// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},					// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},					// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_SSM_0_7KG, WI_FLARE, 0, 0, 0, 0, 0, 0,		// weapons
	6, 5, 0, 0, 0, 0, 0, 0,		// ammo
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
    
	// Rebels covertops (7,62 mm rifle)
{	"Covertops (7,62 mm rifle)",	// descriptiveName
	"rebels_covertops",			// tinyName
	"rebels_covertops",			// modelName
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
	//{0, 0, 0},				// camera distance {min,max,default}
	//{2.5, 2.5, 2.5},			// camera height {min,max,default}
	{0, 10, 0},					// camera distance {min,max,default}
	{0, 10, 2.5},				// camera height {min,max,default}
	0,							// stallspeed
	24,							// maxspeed
	0,							// min throttle
	0,						    // max throttle
	0,							// engines
	0,							// wheels
	0,							// wheel circumference
	0,							// acceleration
	10,							// health
	{0, 0, 0},					// gun tag
	0,							// max gun pitch (upwards = negative) <- gearheight
	0,							// min gun pitch (downwards = positive) <- tailangle
	WI_R_1X7_62MM, WI_FLARE, 0, 0, 0, 0, 0, 0,		// weapons
	60, 5, 0, 0, 0, 0, 0, 0,		// ammo
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
	// Multipurpose aircraft
	// ---------------------------------------------
	
	// Coalition
	
	// F-16 Falcon
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
	2152,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	750,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X20MM, WI_AAM_11KG, WI_AAM_23KG, WI_DROPTANK_PAIR, 0, 0, 0, 0,	// weapons
	580, 2, 2, 1, 0, 0, 0, 0,	// ammo
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

	// Saab JAS 39 Gripen
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
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{90, 70, 320},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	2424,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	250,						// acceleration
	750,						// health
	{19.698f, -1.328f, -3.016f},// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X20MM, WI_AAM_11KG, WI_ASM_66KG, WI_CM, 0, 0, 0, 0,		// weapons
	500, 2, 4, 1, 0, 0, 0, 0,	// ammo
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

	// Harrier GR-7
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
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{100, 70, 240},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	1048,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	250,						// acceleration
	750,						// health
	{19.698f, -1.328f, -3.016f},// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X25MM, WI_AAM_11KG, WI_ASM_57KG, 0, 0, 0, 0, 0,		// weapons
	400, 4, 2, 0, 0, 0, 0, 0,	// ammo
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

	// F-14 Tomcat
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
	2672,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	750,						// health
	{37.8f,-2.4f,-1.3f},		// gun tag
	1.5,						// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X20MM, WI_AAM_40KG, WI_ASM_9KG, WI_DROPTANK_SMALL, WI_CM, 0, 0, 0,	// weapons
	580, 6, 4, 1, 0, 0, 0, 0,	// ammo
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

	// F-18 Hornet
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
	1896,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	750,						// health
	{40.3f, 0, 0.478f},			// gun tag
	4,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X20MM, WI_AAM_60KG, WI_ASM_66KG, WI_ASM_57KG, 0, 0, 0, 0,	// weapons
	580, 4, 1, 1, 0, 0, 0, 0,		// ammo
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

	// F-22 Raptor
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
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 50, 280},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	200,						// stallspeed
	2160,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	750,						// health
	{40.3f, 0, 0.478f},			// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X20MM, WI_AAM_23KG, WI_CM, 0, 0, 0, 0, 0,	// weapons
	580, 6, 1, 0, 0, 0, 0, 0,	// ammo
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
	
	// Rebels
	
	// F-105
    {	"F-105",			// descriptiveName
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
	2184,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	750,						// health
	{19.1f,7.87f,-0.112f},		// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X20MM, WI_AAM_11KG, 0, 0, 0, 0, 0, 0,	// weapons
	580, 2, 0, 0, 0, 0, 0, 0,	// ammo
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

	// F-5 Tiger
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
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 80, 380},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	170,						// stallspeed
	1680,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	750,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_2X20MM, WI_AAM_11KG, WI_ASM_57KG, 0, 0, 0, 0, 0,	// weapons
	400, 4, 2, 0, 0, 0, 0, 0,		// ammo
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

	// F-15 Eagle
    {	"F-15 Eagle",		    // descriptiveName
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
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{65, 50, 240},				// turnspeed around the three axis
	{0, 100, 70},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	220,						// stallspeed
	2632,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	750,						// health
	{19.1f,7.87f,-0.112f},		// gun tag
	4,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X20MM, WI_AAM_40KG, WI_ASM_57KG, WI_AAM_23KG, 0, 0, 0, 0,	// weapons
	580, 4, 2, 2, 0, 0, 0, 35,	// ammo
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

	// SU-27 Flanker
    {	"SU-27 Flanker",		// descriptiveName
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
	2480,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	750,						// health
	{19.1f,7.87f,-0.112f},		// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X30MM, WI_AAM_39KG, WI_AAM_7KG, 0, 0, 0, 0, 0,	// weapons
	150, 6, 2, 0, 0, 0, 0, 0,	// ammo
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

	// YF-23
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
	2312,						// maxspeed
	0,							// min throttle
	15,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	750,						// health
	{19.1f,7.87f,-0.112f},		// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X20MM, WI_AAM_11KG, WI_AAM_23KG, 0, 0, 0, 0, 0,	// weapons
	580, 2, 4, 0, 0, 0, 0, 0,		// ammo
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

	// A-10 Thunderbolt II
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
	696,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	270,						// acceleration
	6000,						// health
	{26.25f, -0.94f, 1.811f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	WI_ACN_1X30MM, WI_AAM_11KG, WI_ASM_57KG, 0, 0, 0, 0, 0,	// weapons
	1350, 2, 6, 0, 0, 0, 0, 0,	// ammo
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
	
	// ---------------------------------------------
	// Strategic bombers
	// ---------------------------------------------
	
	// Coalition
	
	// B-2 Spirit
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
	1000,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	4,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	1000,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	0, WI_BMB_940KG, WI_CFLARE, 0, 0, 0, 0, 0,	// weapons
	0, 16, 30, 0, 0, 0, 0, 0,		// ammo
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
	
	
	// B-2 Spirit (NB loadout)
	{	"B-2 Spirit (NB loadout)",	// descriptiveName
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
	1000,						// maxspeed
	0,							// min throttle
	10,							// max throttle
	4,							// engines
	0,							// wheels
	0,							// wheel circumference
	260,						// acceleration
	1000,						// health
	{16.65f, -3.391f, 0.378f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	0,							// tailangle
	0, WI_NB_B82, WI_CFLARE, 0, 0, 0, 0, 0,	// weapons
	0, 1, 30, 0, 0, 0, 0, 0,		// ammo
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
	
	// Rebels
	
	// B-17g
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
	456,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	4,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	1000,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	-8,							// tailangle
	0, WI_BMB_460KG, WI_CFLARE, 0, 0, 0, 0, 0,		// weapons
	0, 10, 30, 0, 0, 0, 0, 0,	// ammo
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

	// B-17g bomber (incendiary bomb loadout)
    {	"B-17g (incendiary bomb loadout)",	// descriptiveName
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
	456,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	4,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	1000,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	-8,							// tailangle
	0, WI_ICB_ANM50, WI_CFLARE, 0, 0, 0, 0, 0,		// weapons
	0, 100, 30, 0, 0, 0, 0, 0,	// ammo
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
	
	// ---------------------------------------------
	// Attack helicopters
	// ---------------------------------------------	
	
	//Coalition
	
	{	"UH-1 Iroquois",		// descriptiveName
	"UH-1",						// tinyName
	"UH-1",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_HELO,
	CLASS_HELO_RECON,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 125, 70},				// turnspeed around the three axis
	{0, 100, 80},				// camera distance {min,max,default}
	{0, 100, 25},				// camera height {min,max,default}
	0,							// stallspeed
	200,						// maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	33,							// acceleration
	750,						// health
	{3, 1, 6},					// gun tag
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	0, WI_ASM_8KG, 0, 0, 0, 0, 0, 0,	// weapons
	0, 2, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Rebels
	
	{	"Mil Mi-24 Hind",				// descriptiveName
	"Hind",						// tinyName
	"Hind",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_HELO,
	CLASS_HELO_RECON,
	0,							// flags
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	328,						// maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	9,							// wheel circumference
	33,							// acceleration
	3000,						// health
	{3, 1, 6},					// gun tag
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	0, WI_ACN_2X23MM, WI_MG_1X12_7MM, 0, 0, 0, 0, 0,// weapons
	0, 450, 1500, 0, 0, 0, 0, 20,	// ammo
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

	// ---------------------------------------------
	// Unmanned Aerial Vehicles
	// ---------------------------------------------
	
	// Coalition
	
	// Spitfire Mk VB fighter UAV
    {	"Spitfire Mk VB F UAV",		// descriptiveName
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
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{80, 80, 240},				// turnspeed around the three axis
	{0, 100, 50},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	85,						    // stallspeed
	584,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	500,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	-14,							// tailangle
	WI_MG_8X7_62MM, 0, 0, 0, 0, 0, 0, 0,	// weapons
	300, 0, 0, 0, 0, 0, 0, 0,	// ammo
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
	
	// P-51d Mustang ground-attack UAV
    {	"P-51d Mustang GA UAV",		// descriptiveName
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
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{60, 60, 200},				// turnspeed around the three axis
	{0, 100, 50},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	90,						    // stallspeed
	696,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	500,						// health
	{3.5f, -9.2f, 0.136f},		// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	-14,						// tailangle
	WI_MG_4X12_7MM, 0, 0, 0, 0, 0, 0, 0,	// weapons
	375, 0, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Rebels
	
	// Me Bf 109g fighter UAV
    {	"Me Bf 109g F UAV",// descriptiveName
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
	632,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	500,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	-14,						// tailangle
	WI_MG_2X7_92MM, 0, 0, 0, 0, 0, 0, 0,	// weapons
	1000, 0, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Focke-Wulf Fw190 ground-attack UAV
    {	"Focke-Wulf Fw190 GA UAV",	// descriptiveName
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
	648,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	500,						// health
	{2.518f, -7.371f, -0.155f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	-14,							// tailangle
	WI_MG_2X7_92MM, WI_MG_2X13MM, 0, 0, 0, 0, 0, 0,		// weapons
	600, 600, 0, 0, 0, 0, 0, 0,	// ammo
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

	// ---------------------------------------------
	// Flying bombs
	// ---------------------------------------------

	// Coalition
	
	// none
	
	//Rebels
	
	// Fokker Dr.1 flying bomb
    {	"Fokker Dr.1 flying bomb",	// descriptiveName
	"Fokker",					// tinyName
	"dr1",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW1,				// id
	MF_TEAM_1,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_PROP|
	HC_TAILDRAGGER|,			// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{70, 80, 320},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	50,						    // stallspeed
	176,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	200,						// health
	{2.518f, -1.371f, -0.155f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	-14,							// tailangle
	0, 0, 0, 0, 0, 0, 0, 0,		// weapons
	0, 0, 0, 0, 0, 0, 0, 0,		// ammo
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

	// Sopwith camel flying bomb
    {	"Sopwith camel flying bomb", // descriptiveName
	"Camel",					// tinyName
	"camel",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_WW1,				// id
	MF_TEAM_2,
	CAT_PLANE,
	CLASS_PLANE_FIGHTER,
	0,							// flags
	HC_PROP|
	HC_TAILDRAGGER|,			// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{60, 70, 260},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	55,						    // stallspeed
	176,					    // maxspeed
	0,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	220,						// acceleration
	200,						// health
	{2.518f, -1.371f, -0.155f},	// gun tag
	0,							// gearheight - SET AUTOMATICALLY
	-14,						// tailangle
	0, 0, 0, 0, 0, 0, 0, 0,		// weapons
	0, 0, 0, 0, 0, 0, 0, 0,		// ammo
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
	// Recon vehicles
	// ---------------------------------------------
	
	// Coalition
	
	// BRDM-2
	{	"BRDM-2",				// descriptiveName
	"BRDM2",					// tinyName
	"brdm2",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_GROUND,
	CLASS_GROUND_RECON,
	0,							// flags
	HC_AMPHIBIOUS|HC_WHEELS,	// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	96,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	9,							// wheel circumference
	33,							// acceleration
	2000,						// health
	{3, 1, 6},					// gun tag
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_1X14_5MM, 0, 0, 0, 0, 0, 0, 0,	// weapons
	500, 0, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Rebels
	
	// Fast Attack Vehicle
	{	"Fast Attack Vehicle",	// descriptiveName
	"FAV",						// tinyName
	"fav",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_GROUND,
	CLASS_GROUND_RECON,
	0,							// flags
	HC_WHEELS,					// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	136,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	9,							// wheel circumference
	33,							// acceleration
	2000,						// health
	{3, 1, 6},					// gun tag
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_1X7_62MM, 0, 0, 0, 0, 0, 0, 0,	// weapons
	800, 0, 0, 0, 0, 0, 0, 20,	// ammo
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
	
	// ---------------------------------------------
	// Armoured personnel carriers
	// ---------------------------------------------
	
	// Coalition
	
	// High Mobility Multipurpose Wheeled Vehicle (up-armoured)
	 {	"HMMWV (up-armoured)",	// descriptiveName
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
	104,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	8,							// wheel circumference
	35,							// acceleration
	2000,						// health
	{8, 0, 2},					// gun tag
	300,						// max gun pitch (upwards = negative) <- gearheight
	6,							// min gun pitch (downwards = positive) <- tailangle
	0, 0, 0, 0, 0, 0, 0, 0,		// weapons
	0, 0, 0, 0, 0, 0, 0, 20,	// ammo
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
	
	// Rebels
	
	// BMP-3
    {	"BMP-3",				// descriptiveName
	"BMP-3",					// tinyName
	"bmp-3",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_GROUND,
	CLASS_GROUND_RECON,
	0,							// flags
	HC_AMPHIBIOUS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	64,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	30,							// acceleration
	7000,						// health
	{8, 1, 2},					// gun tag
	300,						// max gun pitch (upwards = negative) <- gearheight
	6,							// min gun pitch (downwards = positive) <- tailangle
	WI_ACN_1X30MM, WI_CNN_1X100MM, WI_MG_1X7_62MM, 0, 0, 0, 0, 0,	// weapons
	500, 40, 2000, 0, 0, 0, 0, 0,	// ammo
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

	// ---------------------------------------------
	// Tanks
	// ---------------------------------------------

	// Coalition
	
	// M1A1 Abrams
    {	"M1A1 Abrams",			// descriptiveName
	"M1",						// tinyName
	"m1",						// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_GROUND,
	CLASS_GROUND_MBT,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 40},				// camera distance {min,max,default}
	{0, 100, 20},				// camera height {min,max,default}
	0,							// stallspeed
	64,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	25,							// acceleration
	120000,						// health
	{5, 0, 8},					// gun tag
	310,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_1X12_7MM, WI_CNN_1X120MM, 0, 0, 0, 0, 0, 0,	// weapons
	900, 40, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Rebels
	
	// Panzer III
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
	40,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	20,							// acceleration
	10000,						// health
	{5, 0, 8},					// gun tag
	335,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_2X7_92MM, WI_CNN_1X50MM, 0, 0, 0, 0, 0, 0,	// weapons
	2500, 90, 0, 0, 0, 0, 0, 0,	// ammo
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

	// M4 Sherman
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
	40,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	20,							// acceleration
	24000,						// health
	{5, 0, 8},					// gun tag
	335,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	WI_MG_2X7_62MM, WI_CNN_1X75MM, 0, 0, 0, 0, 0, 0,	// weapons
	4750, 90, 0, 0, 0, 0, 0, 0,	// ammo
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

	// ---------------------------------------------
	// Anti-aircraft
	// ---------------------------------------------
	
	// Coalition
	
	// AN/TWQ-1 Avenger
    {	"AN/TWQ-1 Avenger",			// descriptiveName
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
	88,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	4,							// wheels
	8,							// wheel circumference
	35,							// acceleration
	750,						// health
	{8, 0, 2},					// gun tag
	300,						// max gun pitch (upwards = negative) <- gearheight
	6,							// min gun pitch (downwards = positive) <- tailangle
	0, WI_SAM_3KG, 0, 0, 0, 0, 0, 0,	// weapons
	0, 8, 0, 0, 0, 0, 0, 0,	// ammo
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

	// Rebels
	
	// ZSU-23-4
    {	"ZSU-23",				// descriptiveName
	"zsu23",					// tinyName
	"ZSU-23",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_2,
	CAT_GROUND,
	CLASS_GROUND_SAM,
	0,							// flags
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 100},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	48,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	33,							// acceleration
	3000,						// health
	{3, 1, 6},					// gun tag
	285,						// max gun pitch (upwards = negative) <- gearheight
	5,							// min gun pitch (downwards = positive) <- tailangle
	WI_ACN_4X23MM, 0, 0, 0, 0, 0, 0, 0,	// weapons
	2000, 0, 0, 0, 0, 0, 0, 0,	// ammo
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

	// ---------------------------------------------
	// Artillery
	// ---------------------------------------------
	
	// Coalition
	
	// M110 Howitzer
    {	"M110 Howitzer",					// descriptiveName
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
	48,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	25,							// acceleration
	2600,						// health
	{25, 0, 2},					// gun tag
	310,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	0, WI_HOW_1X203MM, 0, 0, 0, 0, 0, 0,	// weapons
	0, 40, 0, 0, 0, 0, 0, 0,	// ammo
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
	
	// Rebels
	
	// M270 Multiple Launch Rocket System
	{	"M270 Multiple Launch Rocket System",	// descriptiveName
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
	56,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	20,							// acceleration
	1000,						// health
	{5, 0, 8},					// gun tag
	335,						// max gun pitch (upwards = negative) <- gearheight
	3,							// min gun pitch (downwards = positive) <- tailangle
	0, WI_SSM_90KG, 0, 0, 0, 0, 0, 0,	// weapons
	0, 12, 0, 0, 0, 0, 0, 0,	// ammo
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
	// Recon ships
	// ---------------------------------------------

	// Coalition
	
	// Patrolboat, River 31
    {	"Patrolboat, River 31",			// descriptiveName
	"PBR31",					// tinyName
	"pbr31mk2",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_BOAT,
	CLASS_BOAT_PATROL,
	0,							// flags
	HC_DUALGUNS,				// capabilities
	{0,0,0,0,0,0,0,0,0,0},		// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 20},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	48,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	15,							// acceleration
	2000,						// health
	{5, 0, 8},					// gun tag
	8,							// max angle to lean when turning <- gearheight
	-5,							// bowangle (depends on speeed) <-tailangle 
	WI_ACN_1X20MM, WI_MG_1X7_62MM, 0, 0, 0, 0, 0, 0,	// weapons
	400, 900, 0, 0, 0, 0, 0, 0,	// ammo
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
    },

	// Rebels
	
	// Skiff
	{	"Skiff",			// descriptiveName
	"PBR31",					// tinyName
	"pbr31mk2",					// modelName
	SHADOW_DEFAULT,				// alpha shadow
	MF_GAMESET_MODERN,			// id
	MF_TEAM_1,
	CAT_BOAT,
	CLASS_BOAT_PATROL,
	0,							// flags
	0,							// capabilities
	{0,0,0,0,0,0,0,0,0,0},			// handles - SET AUTOMATICALLY
	{0, 0, 0},					// mins (bounding box) - SET AUTOMATICALLY
	{0, 0, 0},					// max (bounding box) - SET AUTOMATICALLY
	{50, 100, 20},				// turnspeed around the three axis
	{0, 100, 30},				// camera distance {min,max,default}
	{0, 100, 10},				// camera height {min,max,default}
	0,							// stallspeed
	64,						    // maxspeed
	-5,							// min throttle
	10,						    // max throttle
	1,							// engines
	0,							// wheels
	0,							// wheel circumference
	15,							// acceleration
	750,						// health
	{5, 0, 8},					// gun tag
	8,							// max angle to lean when turning <- gearheight
	-5,							// bowangle (depends on speeed) <-tailangle 
	WI_MG_1X7_62MM, 0, 0, 0, 0, 0, 0, 0,	// weapons
	500, 0, 0, 0, 0, 0, 0, 0,	// ammo
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

const char *gameset_items[MF_MAX_GAMESETS+1] =
{
	"Team deathmatch",
	"Capture the flag",
	"Objective",
};

const char *gameset_codes[MF_MAX_GAMESETS+1] =
{
	"teamdeathmatch",
	"capturetheflag",
	"objective",
};

const char *team_items[MF_MAX_GAMESETS][MF_MAX_TEAMS+1] =
{
	// Team deathmatch
	{
		"Coalition",
		"Rebels",
	},

	// Capture the flag
	{
		"Coalition",
		"Rebels",
	},

	// Objective
	{
		"Coalition",
		"Rebels",
	},
};

const char *cat_fileRef[MF_MAX_CATEGORIES+1] =
{
	// Vehicle categories
	"Aircraft",
	"Ground vehicles",
	"Watercraft",
};

const char *cat_items[MF_MAX_CATEGORIES+1] =
{
	// categories
	"Aircraft",
	"Ground vehicles",
	"Helicopters",
	"Watercraft",
};

const char *class_items[MF_MAX_CATEGORIES][MF_MAX_CLASSES+1] =
{	
	// Aircraft
	{
		"Multipurpose aircraft",
		"Strategic bombers",
		"Attack helicopters",
	},

	// Ground vehicles
	{
		"Recon vehicles",
		"Armoured personnel carriers",
		"Tanks",
		"Anti-aircraft",
		"Artillery",
	},

	// Watercraft
	{
		"Recon ships",
	},
};
