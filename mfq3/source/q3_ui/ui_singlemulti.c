/*
 * $Id: ui_singlemulti.c,v 1.2 2002-01-07 00:06:02 thebjoern Exp $
*/

/*
=======================================================================

SINGLE/MULTIPLAYER MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK					"menu/art/back"
#define ART_BACK_A					"menu/art/back_a"
#define ART_UP						"menu/art/arrow_up"
#define ART_UP_A					"menu/art/arrow_up_a"
#define ART_DOWN					"menu/art/arrow_down"
#define ART_DOWN_A					"menu/art/arrow_down_a"
#define ART_BANNER					"menu/art/startgame"
#define ART_LOGO					"menu/art/background_fade"
#define ART_GAMESET_WW2				"menu/art/wwii/logo"
#define ART_GAMESET_WW1				"menu/art/wwi/logo"
#define ART_GAMESET_MODERN			"menu/art/modern/logo"
	
#define ID_BACK				10
#define ID_TRAINING			11
#define ID_MISSION			12
#define ID_CAMPAIGN			13
#define ID_MULTI			14
#define ID_UP				15
#define ID_DOWN				16


typedef struct {
	menuframework_s	menu;

	menubitmap_s	banner;

	menubitmap_s	up;
	menubitmap_s	down;

	menubitmap_s	back;
	menutext_s		training;
	menutext_s		mission;
	menutext_s		campaign;
	menutext_s		multi;

	const char		*gameset_name;

	unsigned long	gameset_value;
} singlemulti_t;

static singlemulti_t	s_singlemulti;


/*
=================
UI_Set_Gameset
=================
*/
static void UI_Set_Gameset()
{
	MF_SetGameset(s_singlemulti.gameset_value);
	switch( s_singlemulti.gameset_value )
	{
	case MF_GAMESET_WW2:
		trap_Cvar_Set("ui_gameset", "ww2");
		break;
	case MF_GAMESET_WW1:
		trap_Cvar_Set("ui_gameset", "ww1");
		break;
	case MF_GAMESET_MODERN:
	default:
		trap_Cvar_Set("ui_gameset", "modern");
		break;
	}
	s_singlemulti.gameset_name = gameset_items[MF_getItemIndexFromHex((s_singlemulti.gameset_value>>24))];
}

/*
=================
GamesetMenu_Prev
=================
*/
static void Gameset_Prev( void ) {
	s_singlemulti.gameset_value>>=1;
	if( s_singlemulti.gameset_value < MF_GAMESET_MIN ) s_singlemulti.gameset_value = MF_GAMESET_MAX;
	UI_Set_Gameset();
}


/*
=================
GamesetMenu_Next
=================
*/
static void Gameset_Next( void ) {
	s_singlemulti.gameset_value<<=1;
	if( s_singlemulti.gameset_value > MF_GAMESET_MAX ) s_singlemulti.gameset_value = MF_GAMESET_MIN;
	UI_Set_Gameset();
}

/*
===============
SingleMulti_MenuEvent
===============
*/
static void SingleMulti_MenuEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {

	case ID_BACK:
		UI_PopMenu();
		break;

	case ID_TRAINING:
		UI_Set_Gameset();
		break;

	case ID_MISSION:
		UI_Set_Gameset();
		break;

	case ID_CAMPAIGN:
		UI_Set_Gameset();
		break;
	
	case ID_UP:
		Gameset_Prev();
		break;

	case ID_DOWN:
		Gameset_Next();
		break;

	case ID_MULTI:
		UI_Set_Gameset();
		UI_ArenaServersMenu();
		break;
	}
}


/*
===============
SingleMultiMenu_Draw
===============
*/
static void SingleMultiMenu_Draw( void ) {
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	switch( s_singlemulti.gameset_value )
	{
	case MF_GAMESET_MODERN:
		UI_DrawNamedPic( 221, 136, 360, 280, ART_GAMESET_MODERN );
		break;
	case MF_GAMESET_WW2:
		UI_DrawNamedPic( 221, 136, 360, 280, ART_GAMESET_WW2 );
		break;
	case MF_GAMESET_WW1:
		UI_DrawNamedPic( 221, 136, 360, 280, ART_GAMESET_WW1 );
		break;
	default:
		break;
	}
	UI_DrawString( 580, 140, s_singlemulti.gameset_name, UI_RIGHT, color_grey );

	// standard menu drawing
	Menu_Draw( &s_singlemulti.menu );
}

/*
===============
SingleMulti_MenuInit
===============
*/
static void SingleMulti_MenuInit( void ) {

	memset( &s_singlemulti, 0 ,sizeof(singlemulti_t) );

	SingleMulti_Cache();

	s_singlemulti.menu.fullscreen = qtrue;
	s_singlemulti.menu.wrapAround = qtrue;
	s_singlemulti.menu.draw = SingleMultiMenu_Draw;

	s_singlemulti.gameset_value = UI_GetGameset();
	s_singlemulti.gameset_name = gameset_items[MF_getItemIndexFromHex((s_singlemulti.gameset_value>>24))];

	s_singlemulti.banner.generic.type			= MTYPE_BITMAP;
	s_singlemulti.banner.generic.name			= ART_BANNER;
	s_singlemulti.banner.generic.flags			= QMF_LEFT_JUSTIFY;
	s_singlemulti.banner.generic.x				= 420;
	s_singlemulti.banner.generic.y				= 115;
	s_singlemulti.banner.width					= 180;
	s_singlemulti.banner.height					= 23;

	s_singlemulti.back.generic.type				= MTYPE_BITMAP;
	s_singlemulti.back.generic.name				= ART_BACK;
	s_singlemulti.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_singlemulti.back.generic.id				= ID_BACK;
	s_singlemulti.back.generic.callback			= SingleMulti_MenuEvent;
	s_singlemulti.back.generic.x				= 20;
	s_singlemulti.back.generic.y				= 425;
	s_singlemulti.back.width					= 96;
	s_singlemulti.back.height					= 32;
	s_singlemulti.back.focuspic					= ART_BACK_A;

	s_singlemulti.training.generic.type			= MTYPE_PTEXT;      
	s_singlemulti.training.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_INACTIVE|QMF_GRAYED; 
	s_singlemulti.training.generic.callback		= SingleMulti_MenuEvent;
	s_singlemulti.training.generic.id			= ID_TRAINING;
	s_singlemulti.training.generic.x			= 240;
	s_singlemulti.training.generic.y			= 190;
	s_singlemulti.training.string				= "TRAINING";
	s_singlemulti.training.color				= color_red;
	s_singlemulti.training.style				= UI_LEFT;

	s_singlemulti.mission.generic.type			= MTYPE_PTEXT;      
	s_singlemulti.mission.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_INACTIVE|QMF_GRAYED; 
	s_singlemulti.mission.generic.callback		= SingleMulti_MenuEvent;
	s_singlemulti.mission.generic.id			= ID_MISSION;
	s_singlemulti.mission.generic.x				= 240;
	s_singlemulti.mission.generic.y				= 240;
	s_singlemulti.mission.string				= "SINGLE MISSION";
	s_singlemulti.mission.color					= color_red;
	s_singlemulti.mission.style					= UI_LEFT;

	s_singlemulti.campaign.generic.type			= MTYPE_PTEXT;      
	s_singlemulti.campaign.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_INACTIVE|QMF_GRAYED; 
	s_singlemulti.campaign.generic.callback		= SingleMulti_MenuEvent;
	s_singlemulti.campaign.generic.id			= ID_CAMPAIGN;
	s_singlemulti.campaign.generic.x			= 240;
	s_singlemulti.campaign.generic.y			= 290;
	s_singlemulti.campaign.string				= "CAMPAIGN";
	s_singlemulti.campaign.color				= color_red;
	s_singlemulti.campaign.style				= UI_LEFT;

	s_singlemulti.multi.generic.type			= MTYPE_PTEXT;      
	s_singlemulti.multi.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS; 
	s_singlemulti.multi.generic.callback		= SingleMulti_MenuEvent;
	s_singlemulti.multi.generic.id				= ID_MULTI;
	s_singlemulti.multi.generic.x				= 240;
	s_singlemulti.multi.generic.y				= 340;
	s_singlemulti.multi.string					= "MULTIPLAYER";
	s_singlemulti.multi.color					= color_red;
	s_singlemulti.multi.style					= UI_LEFT;

	s_singlemulti.up.generic.type				= MTYPE_BITMAP;
	s_singlemulti.up.generic.name				= ART_UP;
	s_singlemulti.up.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_singlemulti.up.generic.id					= ID_UP;
	s_singlemulti.up.generic.callback			= SingleMulti_MenuEvent;
	s_singlemulti.up.generic.x					= 600;
	s_singlemulti.up.generic.y					= 200;
	s_singlemulti.up.width						= 25;
	s_singlemulti.up.height						= 66;
	s_singlemulti.up.focuspic					= ART_UP_A;

	s_singlemulti.down.generic.type				= MTYPE_BITMAP;
	s_singlemulti.down.generic.name				= ART_DOWN;
	s_singlemulti.down.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_singlemulti.down.generic.id				= ID_DOWN;
	s_singlemulti.down.generic.callback			= SingleMulti_MenuEvent;
	s_singlemulti.down.generic.x				= 600;
	s_singlemulti.down.generic.y				= 300;
	s_singlemulti.down.width					= 25;
	s_singlemulti.down.height					= 66;
	s_singlemulti.down.focuspic					= ART_DOWN_A;

	Menu_AddItem( &s_singlemulti.menu, &s_singlemulti.banner );
	Menu_AddItem( &s_singlemulti.menu, &s_singlemulti.back );
	Menu_AddItem( &s_singlemulti.menu, &s_singlemulti.training );
	Menu_AddItem( &s_singlemulti.menu, &s_singlemulti.mission );
	Menu_AddItem( &s_singlemulti.menu, &s_singlemulti.campaign );
	Menu_AddItem( &s_singlemulti.menu, &s_singlemulti.multi );
	Menu_AddItem( &s_singlemulti.menu, &s_singlemulti.up );
	Menu_AddItem( &s_singlemulti.menu, &s_singlemulti.down );
}

/*
=================
SingleMulti_Cache
=================
*/
void SingleMulti_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_LOGO );
	trap_R_RegisterShaderNoMip( ART_BANNER );
	trap_R_RegisterShaderNoMip( ART_BACK );
	trap_R_RegisterShaderNoMip( ART_BACK_A );
	trap_R_RegisterShaderNoMip( ART_UP );
	trap_R_RegisterShaderNoMip( ART_UP_A );
	trap_R_RegisterShaderNoMip( ART_DOWN );
	trap_R_RegisterShaderNoMip( ART_DOWN_A );
	trap_R_RegisterShaderNoMip( ART_GAMESET_WW2 );
	trap_R_RegisterShaderNoMip( ART_GAMESET_WW1 );
	trap_R_RegisterShaderNoMip( ART_GAMESET_MODERN );
}

/*
===============
UI_DemosMenu
===============
*/
void UI_SingleMultiMenu( void ) {
	SingleMulti_MenuInit();
	UI_PushMenu( &s_singlemulti.menu );
}
