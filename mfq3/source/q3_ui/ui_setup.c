/*
 * $Id: ui_setup.c,v 1.1 2001-11-15 21:35:14 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

SETUP MENU

=======================================================================
*/


#include "ui_local.h"


#define SETUP_MENU_VERTICAL_SPACING		40

#define ART_LOGO		"menu/art/background_fade"
#define ART_BANNER		"menu/art/setup"
#define ART_BACK		"menu/art/back"
#define ART_BACK_A		"menu/art/back_a"	

#define ID_CUSTOMIZEPLAYER		10
#define ID_CUSTOMIZECONTROLS	11
#define ID_SYSTEMCONFIG			12
#define ID_GAME					13
#define ID_CDKEY				14
#define ID_LOAD					15
#define ID_SAVE					16
#define ID_DEFAULTS				17
#define ID_BACK					18


typedef struct {
	menuframework_s	menu;

	menubitmap_s	banner;
	menutext_s		setupplayer;
	menutext_s		setupcontrols;
	menutext_s		setupsystem;
	menutext_s		game;
	menutext_s		cdkey;
//	menutext_s		load;
//	menutext_s		save;
	menutext_s		defaults;
	menubitmap_s	back;
} setupMenuInfo_t;

static setupMenuInfo_t	setupMenuInfo;


/*
=================
Setup_ResetDefaults_Action
=================
*/
static void Setup_ResetDefaults_Action( qboolean result ) {
	if( !result ) {
		return;
	}
	trap_Cmd_ExecuteText( EXEC_APPEND, "exec default.cfg\n");
	trap_Cmd_ExecuteText( EXEC_APPEND, "cvar_restart\n");
	trap_Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
}


/*
=================
Setup_ResetDefaults_Draw
=================
*/
static void Setup_ResetDefaults_Draw( void ) {
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 0, "WARNING: This will reset *ALL*", UI_CENTER|UI_SMALLFONT, color_yellow );
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 1, "options to their default values.", UI_CENTER|UI_SMALLFONT, color_yellow );
}


/*
===============
UI_SetupMenu_Event
===============
*/
static void UI_SetupMenu_Event( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_CUSTOMIZEPLAYER:
		UI_PlayerSettingsMenu();
		break;

	case ID_CUSTOMIZECONTROLS:
		UI_ControlsMenu();
		break;

	case ID_SYSTEMCONFIG:
		UI_GraphicsOptionsMenu();
		break;

	case ID_GAME:
		UI_PreferencesMenu();
		break;

	case ID_CDKEY:
		UI_CDKeyMenu();
		break;

//	case ID_LOAD:
//		UI_LoadConfigMenu();
//		break;

//	case ID_SAVE:
//		UI_SaveConfigMenu();
//		break;

	case ID_DEFAULTS:
		UI_ConfirmMenu( "SET TO DEFAULTS?", Setup_ResetDefaults_Draw, Setup_ResetDefaults_Action );
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}




/*
===============
SetupMenu_Draw
===============
*/
static void SetupMenu_Draw( void ) {
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	// standard menu drawing
	Menu_Draw( &setupMenuInfo.menu );
}


/*
===============
UI_SetupMenu_Init
===============
*/
static void UI_SetupMenu_Init( void ) {
	int				y;

	UI_SetupMenu_Cache();

	memset( &setupMenuInfo, 0, sizeof(setupMenuInfo) );
	setupMenuInfo.menu.wrapAround = qtrue;
	setupMenuInfo.menu.fullscreen = qtrue;
	setupMenuInfo.menu.draw = SetupMenu_Draw;

	setupMenuInfo.banner.generic.type				= MTYPE_BITMAP;
	setupMenuInfo.banner.generic.name				= ART_BANNER;
	setupMenuInfo.banner.generic.flags				= QMF_LEFT_JUSTIFY;
	setupMenuInfo.banner.generic.x					= 480;
	setupMenuInfo.banner.generic.y					= 115;
	setupMenuInfo.banner.width						= 180;
	setupMenuInfo.banner.height						= 23;

	y = 160;
	setupMenuInfo.setupplayer.generic.type			= MTYPE_PTEXT;
	setupMenuInfo.setupplayer.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.setupplayer.generic.x				= 240;
	setupMenuInfo.setupplayer.generic.y				= y;
	setupMenuInfo.setupplayer.generic.id			= ID_CUSTOMIZEPLAYER;
	setupMenuInfo.setupplayer.generic.callback		= UI_SetupMenu_Event; 
	setupMenuInfo.setupplayer.string				= "PLAYER";
	setupMenuInfo.setupplayer.color					= color_red;
	setupMenuInfo.setupplayer.style					= UI_LEFT;

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.setupcontrols.generic.type		= MTYPE_PTEXT;
	setupMenuInfo.setupcontrols.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.setupcontrols.generic.x			= 240;
	setupMenuInfo.setupcontrols.generic.y			= y;
	setupMenuInfo.setupcontrols.generic.id			= ID_CUSTOMIZECONTROLS;
	setupMenuInfo.setupcontrols.generic.callback	= UI_SetupMenu_Event; 
	setupMenuInfo.setupcontrols.string				= "CONTROLS";
	setupMenuInfo.setupcontrols.color				= color_red;
	setupMenuInfo.setupcontrols.style				= UI_LEFT;

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.setupsystem.generic.type			= MTYPE_PTEXT;
	setupMenuInfo.setupsystem.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.setupsystem.generic.x				= 240;
	setupMenuInfo.setupsystem.generic.y				= y;
	setupMenuInfo.setupsystem.generic.id			= ID_SYSTEMCONFIG;
	setupMenuInfo.setupsystem.generic.callback		= UI_SetupMenu_Event; 
	setupMenuInfo.setupsystem.string				= "SYSTEM";
	setupMenuInfo.setupsystem.color					= color_red;
	setupMenuInfo.setupsystem.style					= UI_LEFT;

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.game.generic.type					= MTYPE_PTEXT;
	setupMenuInfo.game.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.game.generic.x					= 240;
	setupMenuInfo.game.generic.y					= y;
	setupMenuInfo.game.generic.id					= ID_GAME;
	setupMenuInfo.game.generic.callback				= UI_SetupMenu_Event; 
	setupMenuInfo.game.string						= "GAME OPTIONS";
	setupMenuInfo.game.color						= color_red;
	setupMenuInfo.game.style						= UI_LEFT;

	y += SETUP_MENU_VERTICAL_SPACING;
	setupMenuInfo.cdkey.generic.type				= MTYPE_PTEXT;
	setupMenuInfo.cdkey.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.cdkey.generic.x					= 240;
	setupMenuInfo.cdkey.generic.y					= y;
	setupMenuInfo.cdkey.generic.id					= ID_CDKEY;
	setupMenuInfo.cdkey.generic.callback			= UI_SetupMenu_Event; 
	setupMenuInfo.cdkey.string						= "CD Key";
	setupMenuInfo.cdkey.color						= color_red;
	setupMenuInfo.cdkey.style						= UI_LEFT;

	if( !trap_Cvar_VariableValue( "cl_paused" ) ) {
#if 0
		y += SETUP_MENU_VERTICAL_SPACING;
		setupMenuInfo.load.generic.type					= MTYPE_PTEXT;
		setupMenuInfo.load.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
		setupMenuInfo.load.generic.x					= 240;
		setupMenuInfo.load.generic.y					= y;
		setupMenuInfo.load.generic.id					= ID_LOAD;
		setupMenuInfo.load.generic.callback				= UI_SetupMenu_Event; 
		setupMenuInfo.load.string						= "LOAD";
		setupMenuInfo.load.color						= color_red;
		setupMenuInfo.load.style						= UI_LEFT;

		y += SETUP_MENU_VERTICAL_SPACING;
		setupMenuInfo.save.generic.type					= MTYPE_PTEXT;
		setupMenuInfo.save.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
		setupMenuInfo.save.generic.x					= 240;
		setupMenuInfo.save.generic.y					= y;
		setupMenuInfo.save.generic.id					= ID_SAVE;
		setupMenuInfo.save.generic.callback				= UI_SetupMenu_Event; 
		setupMenuInfo.save.string						= "SAVE";
		setupMenuInfo.save.color						= color_red;
		setupMenuInfo.save.style						= UI_LEFT;
#endif

		y += SETUP_MENU_VERTICAL_SPACING;
		setupMenuInfo.defaults.generic.type				= MTYPE_PTEXT;
		setupMenuInfo.defaults.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
		setupMenuInfo.defaults.generic.x				= 240;
		setupMenuInfo.defaults.generic.y				= y;
		setupMenuInfo.defaults.generic.id				= ID_DEFAULTS;
		setupMenuInfo.defaults.generic.callback			= UI_SetupMenu_Event; 
		setupMenuInfo.defaults.string					= "DEFAULTS";
		setupMenuInfo.defaults.color					= color_red;
		setupMenuInfo.defaults.style					= UI_LEFT;
	}

	setupMenuInfo.back.generic.type					= MTYPE_BITMAP;
	setupMenuInfo.back.generic.name					= ART_BACK;
	setupMenuInfo.back.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	setupMenuInfo.back.generic.id					= ID_BACK;
	setupMenuInfo.back.generic.callback				= UI_SetupMenu_Event;
	setupMenuInfo.back.generic.x					= 20;
	setupMenuInfo.back.generic.y					= 425;
	setupMenuInfo.back.width						= 96;
	setupMenuInfo.back.height						= 32;
	setupMenuInfo.back.focuspic						= ART_BACK_A;

	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.banner );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.setupplayer );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.setupcontrols );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.setupsystem );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.game );
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.cdkey );
//	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.load );
//	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.save );
	if( !trap_Cvar_VariableValue( "cl_paused" ) ) {
		Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.defaults );
	}
	Menu_AddItem( &setupMenuInfo.menu, &setupMenuInfo.back );
}


/*
=================
UI_SetupMenu_Cache
=================
*/
void UI_SetupMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_LOGO );
	trap_R_RegisterShaderNoMip( ART_BANNER );
	trap_R_RegisterShaderNoMip( ART_BACK );
	trap_R_RegisterShaderNoMip( ART_BACK_A );
}


/*
===============
UI_SetupMenu
===============
*/
void UI_SetupMenu( void ) {
	UI_SetupMenu_Init();
	UI_PushMenu( &setupMenuInfo.menu );
}
