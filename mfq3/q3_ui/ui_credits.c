/*
 * $Id: ui_credits.c,v 1.1 2001-11-15 15:47:38 moorman Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

CREDITS

=======================================================================
*/


#include "ui_local.h"

#define ART_LOGO					"menu/art/background_sub"



typedef struct {
	menuframework_s	menu;

	menutext_s		banner;

	qhandle_t		body;
	qhandle_t		turret;
	qhandle_t		gun;
	qhandle_t		cockpit;
	qhandle_t		controls;
	qhandle_t		gear;
	qhandle_t		brakes;
	qhandle_t		prop;

} creditsmenu_t;

static creditsmenu_t	s_credits;

static int vehnum;
static int vehdir;
static float vehpos;
static int movetime;
static int drawveh;

/*
=================
UI_CreditMenu_Key
=================
*/
static sfxHandle_t UI_CreditMenu_Key( int key ) {
	if( key & K_CHAR_FLAG ) {
		return 0;
	}

	trap_Cmd_ExecuteText( EXEC_APPEND, "quit\n" );
	return 0;
}



/*
===============
Credits_DrawPlane
===============
*/
static void Credits_DrawPlane( void ) {

	refdef_t		refdef;
	refEntity_t		body;
	refEntity_t		cockpit;
	refEntity_t		controls;
	refEntity_t		breaks;
	refEntity_t		gear;
	refEntity_t		prop;
	vec3_t			origin;
	vec3_t			angles;
	float			x, y, w, h;
	vec4_t			color = {0.5, 0, 0, 1};
	static float	angle = 0;
	float			timediff = (float)(uis.realtime - movetime)/1000;

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );

	x = 0;
	y = 0;
	w = 640;
	h = 480;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 64;// 60
	refdef.fov_y = 48;//19.6875;

	refdef.time = uis.realtime;

	origin[0] = 200;
	if( vehdir == 90 ) {
		vehpos += (80 * timediff);
		if( vehpos > 160 ) drawveh = 0;
	} else {
		vehpos -= (80 * timediff);
		if( vehpos < -160 ) drawveh = 0;
	}
	movetime = uis.realtime;
	origin[1] = vehpos;
	origin[2] = -45;

	trap_R_ClearScene();

	// add the models
	angle = 70.0 * sin( (float)uis.realtime / 1000 );
	memset( &body, 0, sizeof(body) );
	VectorSet( angles, 0, vehdir, angle );
	AnglesToAxis( angles, body.axis );
	body.hModel = s_credits.body;
	VectorCopy( origin, body.origin );
	VectorCopy( origin, body.lightingOrigin );
	body.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( body.origin, body.oldorigin );
	trap_R_AddRefEntityToScene( &body );
	
	// cockpit
	memset( &cockpit, 0, sizeof(cockpit) );
	cockpit.hModel = s_credits.cockpit;
	VectorCopy( origin, cockpit.lightingOrigin );
	AxisCopy( axisDefault, cockpit.axis );
	cockpit.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	UI_PositionRotatedEntityOnTag( &cockpit, &body, s_credits.body, "tag_cockpit");
	trap_R_AddRefEntityToScene( &cockpit );	

	// controls
	memset( &controls, 0, sizeof(controls) );
	controls.hModel = s_credits.controls;
	VectorCopy( origin, controls.lightingOrigin );
	AxisCopy( axisDefault, controls.axis );
	controls.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	UI_PositionRotatedEntityOnTag( &controls, &body, s_credits.body, "tag_controls");
	trap_R_AddRefEntityToScene( &controls );

	// breaks
	if( availableVehicles[vehnum].caps & HC_SPEEDBRAKE ) {
		memset( &breaks, 0, sizeof(breaks) );
		breaks.hModel = s_credits.brakes;
		VectorCopy( origin, breaks.lightingOrigin );
		AxisCopy( axisDefault, breaks.axis );
		breaks.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		UI_PositionRotatedEntityOnTag( &breaks, &body, s_credits.body, "tag_breaks");
		trap_R_AddRefEntityToScene( &breaks );
	}

	// gear
	if( availableVehicles[vehnum].caps & HC_GEAR ) {
		memset( &gear, 0, sizeof(gear) );
		gear.hModel = s_credits.gear;
		gear.frame = 2;
		VectorCopy( origin, gear.lightingOrigin );
		AxisCopy( axisDefault, gear.axis );
		gear.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		UI_PositionRotatedEntityOnTag( &gear, &body, s_credits.body, "tag_gear");
		trap_R_AddRefEntityToScene( &gear );
	}

	// prop
	if( availableVehicles[vehnum].id&MF_GAMESET_ANY&MF_GAMESET_WW2 ) {
		memset( &prop, 0, sizeof(prop) );
		prop.hModel = s_credits.prop;
		AxisCopy( axisDefault, prop.axis );
		RotateAroundDirection( prop.axis, uis.realtime );
		VectorCopy( origin, prop.lightingOrigin );
		prop.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		UI_PositionRotatedEntityOnTag( &prop, &body, s_credits.body, "tag_prop1");
		trap_R_AddRefEntityToScene( &prop );
	}

	trap_R_RenderScene( &refdef );
}

/*
===============
Credits_DrawGV
===============
*/
static void Credits_DrawGV( void ) {

	refdef_t		refdef;
	refEntity_t		body;
	refEntity_t		turret;
	refEntity_t		gun;
	vec3_t			origin;
	vec3_t			angles;
	float			x, y, w, h;
	vec4_t			color = {0.5, 0, 0, 1};
	static float	angle = 0;
	float			timediff = (float)(uis.realtime - movetime)/1000;

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );

	x = 0;
	y = 0;
	w = 640;
	h = 480;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 64;// 60
	refdef.fov_y = 48;//19.6875;

	refdef.time = uis.realtime;

	origin[0] = 120;
	if( vehdir == 90 ) {
		vehpos += (40 * timediff);
		if( vehpos > 160 ) drawveh = 0;
	} else {
		vehpos -= (40 * timediff);
		if( vehpos < -160 ) drawveh = 0;
	}
	movetime = uis.realtime;
	origin[1] = vehpos;
	origin[2] = -35;

	trap_R_ClearScene();

	// add the models
	angle = 70.0 * sin( (float)uis.realtime / 2000 );
	memset( &body, 0, sizeof(body) );
	VectorSet( angles, 0, vehdir, 0 );
	AnglesToAxis( angles, body.axis );
	body.hModel = s_credits.body;
	VectorCopy( origin, body.origin );
	VectorCopy( origin, body.lightingOrigin );
	body.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( body.origin, body.oldorigin );
	trap_R_AddRefEntityToScene( &body );
	
	// turret
	memset( &turret, 0, sizeof(turret) );
	turret.hModel = s_credits.turret;
	VectorCopy( origin, turret.lightingOrigin );
	AxisCopy( axisDefault, turret.axis );
	RotateAroundYaw( turret.axis, angle );
	turret.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	UI_PositionRotatedEntityOnTag( &turret, &body, s_credits.body, "tag_turret");
	trap_R_AddRefEntityToScene( &turret );	

	// gun
	memset( &gun, 0, sizeof(gun) );
	gun.hModel = s_credits.gun;
	VectorCopy( origin, gun.lightingOrigin );
	AxisCopy( axisDefault, gun.axis );
	RotateAroundPitch( gun.axis, -10 );
	gun.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	UI_PositionRotatedEntityOnTag( &gun, &turret, s_credits.turret, "tag_weap");
	trap_R_AddRefEntityToScene( &gun );

	trap_R_RenderScene( &refdef );
}


/*
===============
UI_CreditMenu_Draw
===============
*/
static void UI_CreditMenu_Draw( void ) {
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	if( drawveh ) {
		if( availableVehicles[vehnum].id&CAT_ANY & CAT_PLANE ) {
			Credits_DrawPlane();
		} else if( availableVehicles[vehnum].id&CAT_ANY & CAT_GROUND ) {
			Credits_DrawGV();
		}
	}

	UI_DrawString( 320, 70, "Bjoern Drabeck - Project Lead & Programming", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 88, "Jeremy James - Website Developement", UI_CENTER|UI_SMALLFONT, color_white );

	UI_DrawString( 320, 120, "Michael 'Scheherazade' Banach - Modeller and Artist", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 138, "Fredrik 'oq' Haerenstam - Modeller, Artist and Skinner", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 156, "Stephen 'Abbo' Abbott - Map Designer", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 174, "Ziniewicz 'HaroldTheMadCow' Wojciech - Skinner", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 192, "Charles 'Hedhunta' MacTavish - Modeller and Skinner", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 210, "Marek 'Minkis' Kudlacz - Modeller and Skinner", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 228, "Aren 'ReAn' Blondahl - Modeller", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 246, "Brad 'LSMR' Anderson - Modeller", UI_CENTER|UI_SMALLFONT, color_white );
	UI_DrawString( 320, 268, "...and also thanks to all the other people helping us!", UI_CENTER|UI_SMALLFONT, color_white );


	UI_DrawString( 320, 440, "To order: 1-800-idgames     www.quake3arena.com     www.idsoftware.com", UI_CENTER|UI_SMALLFONT, color_red );
	UI_DrawString( 320, 460, "Quake III Arena(c) 1999-2000, Id Software, Inc.  All Rights Reserved", UI_CENTER|UI_SMALLFONT, color_red );

	// standard menu drawing
	Menu_Draw( &s_credits.menu );
}


static void Credits_Reg_Plane( void ) {
	s_credits.body = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s.md3", 
		availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
	s_credits.cockpit = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_cockpit.md3", 
		availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
	s_credits.controls = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_controls.md3", 
		availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
	if( availableVehicles[vehnum].caps & HC_GEAR ) {
		s_credits.gear = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_gear.md3", 
			availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
	}
	if( availableVehicles[vehnum].caps & HC_SPEEDBRAKE ) {
		s_credits.brakes = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_breaks.md3", 
			availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
	}
	if( availableVehicles[vehnum].id&MF_GAMESET_ANY&MF_GAMESET_WW2 ) {
		s_credits.prop = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_prop.md3", 
			availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
	}
}

static void Credits_Reg_GV( void ) {
	s_credits.body = trap_R_RegisterModel( va("models/vehicles/ground/%s/%s.md3", 
		availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
	s_credits.turret = trap_R_RegisterModel( va("models/vehicles/ground/%s/%s_tur.md3", 
		availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
	s_credits.gun = trap_R_RegisterModel( va("models/vehicles/ground/%s/%s_gun.md3", 
		availableVehicles[vehnum].modelName, availableVehicles[vehnum].modelName) );
}


/*
=================
UI_CreditMenu_Cache
=================
*/
void UI_CreditMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_LOGO );

	if( availableVehicles[vehnum].id&CAT_ANY & CAT_PLANE ) {
		Credits_Reg_Plane();
	} else if( availableVehicles[vehnum].id&CAT_ANY & CAT_GROUND ) {
		Credits_Reg_GV();
	}
}


/*
===============
UI_CreditMenuInit
===============
*/
static void UI_CreditMenuInit( void ) {

	memset( &s_credits, 0 ,sizeof(s_credits) );

	UI_CreditMenu_Cache();

	s_credits.menu.draw = UI_CreditMenu_Draw;
	s_credits.menu.key = UI_CreditMenu_Key;
	s_credits.menu.fullscreen = qtrue;

	s_credits.banner.generic.type		= MTYPE_BTEXT;
	s_credits.banner.generic.x			= 320;
	s_credits.banner.generic.y			= 10;
	s_credits.banner.string				= "MILITARY FORCES";
	s_credits.banner.color				= color_black;
	s_credits.banner.style				= UI_CENTER;

	Menu_AddItem( &s_credits.menu, &s_credits.banner );

	vehdir = (rand() % 2 ? 90 : 270);
	vehpos = (vehdir == 90 ? -160 : 160);
	movetime = uis.realtime;
	drawveh = 1;
}


/*
===============
UI_CreditMenu
===============
*/
void UI_CreditMenu( void ) {
	srand(uis.realtime);
	vehnum = rand() % bg_numberOfVehicles;

	UI_CreditMenuInit();

	UI_PushMenu ( &s_credits.menu );
}
