/*
 * $Id: ui_menu.c,v 1.1 2001-11-15 15:47:38 moorman Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

MAIN MENU

=======================================================================
*/


#include "ui_local.h"


#define ID_SETUP				14
#define ID_ENCYCLOPEDIA			15
#define ID_DEMOS				16
#define ID_CINEMATICS			17
#define ID_EXIT					18
#define ID_STARTGAME			19


#define MAIN_MENU_VERTICAL_SPACING	40

#define ART_LOGO				"menu/art/background"
#define ART_STARTGAME			"menu/art/startgame"
#define ART_SETUP				"menu/art/setup"
#define ART_ENCYCLOPEDIA		"menu/art/encyclopedia"
#define ART_DEMOS				"menu/art/demos"
#define ART_CINEMATICS			"menu/art/cinematics"
#define ART_EXIT				"menu/art/exit"

#define MAIN_BANNER_MODEL		"models/mapobjects/banner/mfq3logo.md3"
#define MAIN_PLANE_MODEL		"models/mapobjects/f-14/f-14.md3"

typedef struct {
	menuframework_s	menu;

	menubitmap_s		startgame;
	menubitmap_s		setup;
	menubitmap_s		encyclopedia;
	menubitmap_s		demos;
	menubitmap_s		cinematics;
	menubitmap_s		exit;

	qhandle_t			bannerModel;

	qhandle_t			planeModel;

} mainmenu_t;


static mainmenu_t s_main;

/*
=================
MainMenu_ExitAction
=================
*/
static void MainMenu_ExitAction( qboolean result ) {
	if( !result ) {
		return;
	}
	UI_PopMenu();
	UI_CreditMenu();
}



/*
=================
Main_MenuEvent
=================
*/
void Main_MenuEvent (void* ptr, int event) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
//	case ID_TRAINING:
//		UI_TrainingMenu();
//		break;

//	case ID_SINGLEPLAYER:
//		UI_SPLevelMenu();
//		break;

//	case ID_MULTIPLAYER:
//		UI_ArenaServersMenu();
//		break;

	case ID_STARTGAME:
		UI_SingleMultiMenu();
		break;

	case ID_SETUP:
		UI_SetupMenu();
		break;

	case ID_ENCYCLOPEDIA:
		UI_EncyclopediaMenu();
		break;

	case ID_DEMOS:
		UI_DemosMenu();
		break;

	case ID_CINEMATICS:
		UI_CinematicsMenu();
		break;

	case ID_EXIT:
		UI_ConfirmMenu( "EXIT GAME?", NULL, MainMenu_ExitAction );
		break;
	}
}


/*
===============
MainMenu_Cache
===============
*/
void MainMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_LOGO );
	trap_R_RegisterShaderNoMip( ART_STARTGAME );
	trap_R_RegisterShaderNoMip( ART_SETUP );
	trap_R_RegisterShaderNoMip( ART_ENCYCLOPEDIA );
	trap_R_RegisterShaderNoMip( ART_DEMOS );
	trap_R_RegisterShaderNoMip( ART_CINEMATICS );
	trap_R_RegisterShaderNoMip( ART_EXIT );
	s_main.bannerModel = trap_R_RegisterModel( MAIN_BANNER_MODEL );
	s_main.planeModel = trap_R_RegisterModel( MAIN_PLANE_MODEL );
}


/*
===============
Main_MenuDraw
===============
*/
static void Main_MenuDraw( void ) {

	refdef_t		refdef;
	refEntity_t		ent;
	refEntity_t		plane;
	vec3_t			origin;
	vec3_t			angles;
	float			x, y, w, h;
	static int		old = 0;
	static float	angle = 180;
	vec4_t			color = {0.5, 0, 0, 1};

	// background
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );

	x = 0;
	y = 0;
	w = 640;
	h = 120;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 58;
	refdef.fov_y = 8;

	refdef.time = uis.realtime;

	trap_R_ClearScene();

	// add the model
	origin[0] = 520;
	origin[1] = -50;
	origin[2] = -20;
	memset( &ent, 0, sizeof(ent) );
	VectorSet( angles, -30, 180, 0 );
	AnglesToAxis( angles, ent.axis );
	ent.hModel = s_main.bannerModel;
	VectorCopy( origin, ent.origin );
	VectorCopy( origin, ent.lightingOrigin );
	ent.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent.origin, ent.oldorigin );

	trap_R_AddRefEntityToScene( &ent );

	// add the plane
	if( uis.realtime > old + 20 ) { 
		angle++;
		old = uis.realtime;
	}
	if( angle >= 360 ) angle -= 360;
	origin[0] = 600;
	origin[1] = 258;
	origin[2] = 4;
	memset( &plane, 0, sizeof(plane) );
	VectorSet( angles, 10, angle, 0 );
	AnglesToAxis( angles, plane.axis );
	plane.hModel = s_main.planeModel;
	VectorCopy( origin, plane.origin );
	VectorCopy( origin, plane.lightingOrigin );
	plane.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( plane.origin, plane.oldorigin );

	trap_R_AddRefEntityToScene( &plane );

	trap_R_RenderScene( &refdef );

	// standard menu drawing
	Menu_Draw( &s_main.menu );

}

/*
===============
UI_MainMenu

The main menu only comes up when not in a game,
so make sure that the attract loop server is down
and that local cinematics are killed
===============
*/
void UI_MainMenu( void ) {
	int		style = UI_CENTER | UI_DROPSHADOW;

	trap_Cvar_Set( "sv_killserver", "1" );

	if( !uis.demoversion && !ui_cdkeychecked.integer ) {
		char	key[17];

		trap_GetCDKey( key, sizeof(key) );
		if( trap_VerifyCDKey( key, NULL ) == qfalse ) {
			UI_CDKeyMenu();
			return;
		}
	}

	memset( &s_main, 0 ,sizeof(mainmenu_t) );

	MainMenu_Cache();

	s_main.menu.draw = Main_MenuDraw;
	s_main.menu.fullscreen = qtrue;
	s_main.menu.wrapAround = qtrue;

	s_main.startgame.generic.type				= MTYPE_BITMAP_MFQ3;
	s_main.startgame.generic.name				= ART_STARTGAME;
	s_main.startgame.generic.flags				= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_main.startgame.generic.id					= ID_STARTGAME;
	s_main.startgame.generic.callback			= Main_MenuEvent; 
	s_main.startgame.generic.x					= 0;
	s_main.startgame.generic.y					= 145;
	s_main.startgame.width						= 180;
	s_main.startgame.height						= 23;

	s_main.setup.generic.type					= MTYPE_BITMAP_MFQ3;
	s_main.setup.generic.name					= ART_SETUP;
	s_main.setup.generic.flags					= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_main.setup.generic.id						= ID_SETUP;
	s_main.setup.generic.callback				= Main_MenuEvent; 
	s_main.setup.generic.x						= 0;
	s_main.setup.generic.y						= 187;
	s_main.setup.width							= 180;
	s_main.setup.height							= 23;

	s_main.encyclopedia.generic.type			= MTYPE_BITMAP_MFQ3;
	s_main.encyclopedia.generic.name			= ART_ENCYCLOPEDIA;
	s_main.encyclopedia.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_main.encyclopedia.generic.id				= ID_ENCYCLOPEDIA;
	s_main.encyclopedia.generic.callback		= Main_MenuEvent; 
	s_main.encyclopedia.generic.x				= 0;
	s_main.encyclopedia.generic.y				= 229;
	s_main.encyclopedia.width					= 180;
	s_main.encyclopedia.height					= 23;

	s_main.demos.generic.type					= MTYPE_BITMAP_MFQ3;
	s_main.demos.generic.name					= ART_DEMOS;
	s_main.demos.generic.flags					= QMF_LEFT_JUSTIFY|QMF_INACTIVE;//QMF_HIGHLIGHT_IF_FOCUS;
	s_main.demos.generic.id						= ID_DEMOS;
	s_main.demos.generic.callback				= Main_MenuEvent; 
	s_main.demos.generic.x						= 0;
	s_main.demos.generic.y						= 271;
	s_main.demos.width							= 180;
	s_main.demos.height							= 23;

	s_main.cinematics.generic.type				= MTYPE_BITMAP_MFQ3;
	s_main.cinematics.generic.name				= ART_CINEMATICS;
	s_main.cinematics.generic.flags				= QMF_LEFT_JUSTIFY|QMF_INACTIVE;//QMF_HIGHLIGHT_IF_FOCUS;
	s_main.cinematics.generic.id				= ID_CINEMATICS;
	s_main.cinematics.generic.callback			= Main_MenuEvent; 
	s_main.cinematics.generic.x					= 0;
	s_main.cinematics.generic.y					= 313;
	s_main.cinematics.width						= 180;
	s_main.cinematics.height					= 23;

	s_main.exit.generic.type					= MTYPE_BITMAP_MFQ3;
	s_main.exit.generic.name					= ART_EXIT;
	s_main.exit.generic.flags					= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_main.exit.generic.id						= ID_EXIT;
	s_main.exit.generic.callback				= Main_MenuEvent; 
	s_main.exit.generic.x						= 0;
	s_main.exit.generic.y						= 355;
	s_main.exit.width							= 180;
	s_main.exit.height							= 23;

	Menu_AddItem( &s_main.menu,	&s_main.startgame );
	Menu_AddItem( &s_main.menu,	&s_main.setup );
	Menu_AddItem( &s_main.menu,	&s_main.encyclopedia );
	Menu_AddItem( &s_main.menu,	&s_main.demos );
	Menu_AddItem( &s_main.menu,	&s_main.cinematics );
	Menu_AddItem( &s_main.menu,	&s_main.exit );             

	trap_Key_SetCatcher( KEYCATCH_UI );
	uis.menusp = 0;
	UI_PushMenu ( &s_main.menu );
}
