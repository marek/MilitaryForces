/*
 * $Id: ui_team.c,v 1.1 2001-11-15 15:47:38 moorman Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// ui_team.c
//

#include "ui_local.h"


#define TEAMMAIN_BACKGROUND	"menu/art/ingame"

#define ID_JOINRED		100
#define ID_JOINBLUE		101
#define ID_JOINGAME		102
#define ID_SPECTATE		103


typedef struct
{
	menuframework_s	menu;
	menutext_s		banner;
	menutext_s		joinred;
	menutext_s		joinblue;
	menutext_s		joingame;
	menutext_s		spectate;
} teammain_t;

static teammain_t	s_teammain;

static menuframework_s	s_teammain_menu;
static menuaction_s		s_teammain_orders;
static menuaction_s		s_teammain_joinred;
static menuaction_s		s_teammain_joinblue;
static menuaction_s		s_teammain_joingame;
static menuaction_s		s_teammain_spectate;


/*
===============
TeamMain_MenuEvent
===============
*/
static void TeamMain_MenuEvent( void* ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_JOINRED:
		trap_Cmd_ExecuteText( EXEC_APPEND, "cmd team red\n" );
		UI_ForceMenuOff();
		break;

	case ID_JOINBLUE:
		trap_Cmd_ExecuteText( EXEC_APPEND, "cmd team blue\n" );
		UI_ForceMenuOff();
		break;

	case ID_JOINGAME:
		trap_Cmd_ExecuteText( EXEC_APPEND, "cmd team free\n" );
		UI_ForceMenuOff();
		break;

	case ID_SPECTATE:
		trap_Cmd_ExecuteText( EXEC_APPEND, "cmd team spectator\n" );
		UI_ForceMenuOff();
		break;
	}
}

/*
===============
TeamMain_Draw
===============
*/
void TeamMain_Draw( void ) {
	UI_DrawNamedPic( 0, 0, 640, 480, TEAMMAIN_BACKGROUND );

	// standard menu drawing
	Menu_Draw( &s_teammain.menu );
}

/*
===============
TeamMain_MenuInit
===============
*/
void TeamMain_MenuInit( void ) {
	int		y;
	int		gametype;
	char	info[MAX_INFO_STRING];

	memset( &s_teammain, 0, sizeof(s_teammain) );

	TeamMain_Cache();

	s_teammain.menu.wrapAround = qtrue;
	s_teammain.menu.fullscreen = qfalse;
	s_teammain.menu.draw = TeamMain_Draw;

	s_teammain.banner.generic.type			= MTYPE_BTEXT;
	s_teammain.banner.generic.x				= 320;
	s_teammain.banner.generic.y				= 105;
	s_teammain.banner.string				= "JOIN TEAM";
	s_teammain.banner.color					= color_black;
	s_teammain.banner.style					= UI_CENTER;

	y = 194;

	s_teammain.joinred.generic.type     = MTYPE_PTEXT;
	s_teammain.joinred.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_teammain.joinred.generic.id       = ID_JOINRED;
	s_teammain.joinred.generic.callback = TeamMain_MenuEvent;
	s_teammain.joinred.generic.x        = 320;
	s_teammain.joinred.generic.y        = y;
	s_teammain.joinred.string           = "JOIN RED";
	s_teammain.joinred.style            = UI_CENTER|UI_SMALLFONT;
	s_teammain.joinred.color            = color_red;
	y += 20;

	s_teammain.joinblue.generic.type     = MTYPE_PTEXT;
	s_teammain.joinblue.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_teammain.joinblue.generic.id       = ID_JOINBLUE;
	s_teammain.joinblue.generic.callback = TeamMain_MenuEvent;
	s_teammain.joinblue.generic.x        = 320;
	s_teammain.joinblue.generic.y        = y;
	s_teammain.joinblue.string           = "JOIN BLUE";
	s_teammain.joinblue.style            = UI_CENTER|UI_SMALLFONT;
	s_teammain.joinblue.color            = color_red;
	y += 20;

	s_teammain.joingame.generic.type     = MTYPE_PTEXT;
	s_teammain.joingame.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_teammain.joingame.generic.id       = ID_JOINGAME;
	s_teammain.joingame.generic.callback = TeamMain_MenuEvent;
	s_teammain.joingame.generic.x        = 320;
	s_teammain.joingame.generic.y        = y;
	s_teammain.joingame.string           = "JOIN GAME";
	s_teammain.joingame.style            = UI_CENTER|UI_SMALLFONT;
	s_teammain.joingame.color            = color_red;
	y += 20;

	s_teammain.spectate.generic.type     = MTYPE_PTEXT;
	s_teammain.spectate.generic.flags    = QMF_CENTER_JUSTIFY|QMF_PULSEIFFOCUS;
	s_teammain.spectate.generic.id       = ID_SPECTATE;
	s_teammain.spectate.generic.callback = TeamMain_MenuEvent;
	s_teammain.spectate.generic.x        = 320;
	s_teammain.spectate.generic.y        = y;
	s_teammain.spectate.string           = "SPECTATE";
	s_teammain.spectate.style            = UI_CENTER|UI_SMALLFONT;
	s_teammain.spectate.color            = color_red;
	y += 20;

	trap_GetConfigString(CS_SERVERINFO, info, MAX_INFO_STRING);   
	gametype = atoi( Info_ValueForKey( info,"g_gametype" ) );
			      
	// set initial states
	switch( gametype ) {
	case GT_SINGLE_PLAYER:
	case GT_FFA:
	case GT_TOURNAMENT:
		s_teammain.joinred.generic.flags  |= QMF_GRAYED;
		s_teammain.joinblue.generic.flags |= QMF_GRAYED;
		break;

	default:
	case GT_TEAM:
	case GT_CTF:
		s_teammain.joingame.generic.flags |= QMF_GRAYED;
		break;
	}

	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.banner );
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.joinred );
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.joinblue );
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.joingame );
	Menu_AddItem( &s_teammain.menu, (void*) &s_teammain.spectate );
}


/*
===============
TeamMain_Cache
===============
*/
void TeamMain_Cache( void ) {
	trap_R_RegisterShaderNoMip( TEAMMAIN_BACKGROUND );
}


/*
===============
UI_TeamMainMenu
===============
*/
void UI_TeamMainMenu( void ) {
	TeamMain_MenuInit();
	UI_PushMenu ( &s_teammain.menu );
}
