/*
 * $Id: ui_cdkey.c,v 1.1 2001-11-15 21:35:14 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

CD KEY MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_LOGO		"menu/art/background_sub"
#define ART_ACCEPT		"menu/art/accept"
#define ART_ACCEPT_A	"menu/art/accept_a"	
#define ART_BACK		"menu/art/back"
#define ART_BACK_A		"menu/art/back_a"	

#define ID_CDKEY		10
#define ID_ACCEPT		11
#define ID_BACK			12


typedef struct {
	menuframework_s	menu;

	menutext_s		banner;

	menufield_s		cdkey;

	menubitmap_s	accept;
	menubitmap_s	back;
} cdkeyMenuInfo_t;

static cdkeyMenuInfo_t	cdkeyMenuInfo;


/*
===============
UI_CDKeyMenu_Event
===============
*/
static void UI_CDKeyMenu_Event( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {
	case ID_ACCEPT:
		if( cdkeyMenuInfo.cdkey.field.buffer[0] ) {
			trap_SetCDKey( cdkeyMenuInfo.cdkey.field.buffer );
		}
		UI_PopMenu();
		break;

	case ID_BACK:
		UI_PopMenu();
		break;
	}
}


/*
=================
UI_CDKeyMenu_PreValidateKey
=================
*/
static int UI_CDKeyMenu_PreValidateKey( const char *key ) {
	char	ch;

	if( strlen( key ) != 16 ) {
		return 1;
	}

	while( ( ch = *key++ ) ) {
		switch( ch ) {
		case '2':
		case '3':
		case '7':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'g':
		case 'h':
		case 'j':
		case 'l':
		case 'p':
		case 'r':
		case 's':
		case 't':
		case 'w':
			continue;
		default:
			return -1;
		}
	}

	return 0;
}


/*
=================
UI_CDKeyMenu_DrawKey
=================
*/
static void UI_CDKeyMenu_DrawKey( void *self ) {
	menufield_s		*f;
	qboolean		focus;
	int				style;
	char			c;
//	float			*color;
	int				x, y;
	int				val;

	f = (menufield_s *)self;

	focus = (f->generic.parent->cursor == f->generic.menuPosition);

	style = UI_LEFT;
//	if( focus ) {
//		color = color_yellow;
//	}
//	else {
//		color = color_orange;
//	}

//	x = 320 - 8 * BIGCHAR_WIDTH;
//	y = 240 - BIGCHAR_HEIGHT / 2;
//	UI_FillRect( x, y, 16 * BIGCHAR_WIDTH, BIGCHAR_HEIGHT, listbar_color );
//	UI_DrawString( x, y, f->field.buffer, style, color );

	x = 320 - 8 * BIGCHAR_WIDTH;
	y = 240 - BIGCHAR_HEIGHT / 2;
	UI_FillRect( x, y, 16 * BIGCHAR_WIDTH, BIGCHAR_HEIGHT, color_grey );
	UI_DrawRect( x-1, y-1, 16 * BIGCHAR_WIDTH + 2, BIGCHAR_HEIGHT + 2, color_black );
	UI_DrawString( x, y, f->field.buffer, style, color_black );

	// draw cursor if we have focus
	if( focus ) {
		if ( trap_Key_GetOverstrikeMode() ) {
			c = 11;
		} else {
			c = 10;
		}

		style &= ~UI_PULSE;
		style |= UI_BLINK;

		UI_DrawChar( x + f->field.cursor * BIGCHAR_WIDTH, y, c, style, color_black );
	}

	val = UI_CDKeyMenu_PreValidateKey( f->field.buffer );
	if( val == 1 ) {
		UI_DrawProportionalString( 320, 306, "Please enter your CD Key", UI_CENTER|UI_SMALLFONT, color_yellow );
	}
	else if ( val == 0 ) {
		UI_DrawProportionalString( 320, 306, "Valid CD Key, thank you", UI_CENTER|UI_SMALLFONT, color_grey );
	}
	else {
		UI_DrawProportionalString( 320, 306, "Invalid CD Key, you bastard!", UI_CENTER|UI_SMALLFONT, color_red );
	}
}


/*
===============
UI_CDKeyMenu_Draw
===============
*/
static void UI_CDKeyMenu_Draw( void ) {
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	// standard menu drawing
	Menu_Draw( &cdkeyMenuInfo.menu );
}


/*
===============
UI_CDKeyMenu_Init
===============
*/
static void UI_CDKeyMenu_Init( void ) {
	trap_Cvar_Set( "ui_cdkeychecked", "1" );

	UI_CDKeyMenu_Cache();

	memset( &cdkeyMenuInfo, 0, sizeof(cdkeyMenuInfo) );
	cdkeyMenuInfo.menu.wrapAround = qtrue;
	cdkeyMenuInfo.menu.fullscreen = qtrue;
	cdkeyMenuInfo.menu.draw = UI_CDKeyMenu_Draw;

	cdkeyMenuInfo.banner.generic.type				= MTYPE_BTEXT;
	cdkeyMenuInfo.banner.generic.x					= 320;
	cdkeyMenuInfo.banner.generic.y					= 80;
	cdkeyMenuInfo.banner.string						= "CD KEY";
	cdkeyMenuInfo.banner.color						= color_white;
	cdkeyMenuInfo.banner.style						= UI_CENTER;

	cdkeyMenuInfo.cdkey.generic.type				= MTYPE_FIELD;
	cdkeyMenuInfo.cdkey.generic.name				= "CD Key:";
	cdkeyMenuInfo.cdkey.generic.flags				= QMF_LOWERCASE;
	cdkeyMenuInfo.cdkey.generic.x					= 320 - BIGCHAR_WIDTH * 2.5;
	cdkeyMenuInfo.cdkey.generic.y					= 240 - BIGCHAR_HEIGHT / 2;
	cdkeyMenuInfo.cdkey.field.widthInChars			= 16;
	cdkeyMenuInfo.cdkey.field.maxchars				= 16;
	cdkeyMenuInfo.cdkey.generic.ownerdraw			= UI_CDKeyMenu_DrawKey;

	cdkeyMenuInfo.accept.generic.type				= MTYPE_BITMAP;
	cdkeyMenuInfo.accept.generic.name				= ART_ACCEPT;
	cdkeyMenuInfo.accept.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	cdkeyMenuInfo.accept.generic.id					= ID_ACCEPT;
	cdkeyMenuInfo.accept.generic.callback			= UI_CDKeyMenu_Event;
	cdkeyMenuInfo.accept.generic.x					= 522;
	cdkeyMenuInfo.accept.generic.y					= 425;
	cdkeyMenuInfo.accept.width						= 96;
	cdkeyMenuInfo.accept.height						= 32;
	cdkeyMenuInfo.accept.focuspic					= ART_ACCEPT_A;

	cdkeyMenuInfo.back.generic.type					= MTYPE_BITMAP;
	cdkeyMenuInfo.back.generic.name					= ART_BACK;
	cdkeyMenuInfo.back.generic.flags				= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	cdkeyMenuInfo.back.generic.id					= ID_BACK;
	cdkeyMenuInfo.back.generic.callback				= UI_CDKeyMenu_Event;
	cdkeyMenuInfo.back.generic.x					= 20;
	cdkeyMenuInfo.back.generic.y					= 425;
	cdkeyMenuInfo.back.width						= 96;
	cdkeyMenuInfo.back.height						= 32;
	cdkeyMenuInfo.back.focuspic						= ART_BACK_A;

	Menu_AddItem( &cdkeyMenuInfo.menu, &cdkeyMenuInfo.banner );
	Menu_AddItem( &cdkeyMenuInfo.menu, &cdkeyMenuInfo.cdkey );
	Menu_AddItem( &cdkeyMenuInfo.menu, &cdkeyMenuInfo.accept );
	if( uis.menusp ) {
		Menu_AddItem( &cdkeyMenuInfo.menu, &cdkeyMenuInfo.back );
	}

	trap_GetCDKey( cdkeyMenuInfo.cdkey.field.buffer, cdkeyMenuInfo.cdkey.field.maxchars + 1 );
	if( trap_VerifyCDKey( cdkeyMenuInfo.cdkey.field.buffer, NULL ) == qfalse ) {
		cdkeyMenuInfo.cdkey.field.buffer[0] = 0;
	}
}


/*
=================
UI_CDKeyMenu_Cache
=================
*/
void UI_CDKeyMenu_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_ACCEPT );
	trap_R_RegisterShaderNoMip( ART_ACCEPT_A );
	trap_R_RegisterShaderNoMip( ART_BACK );
	trap_R_RegisterShaderNoMip( ART_BACK_A );
	trap_R_RegisterShaderNoMip( ART_LOGO );
}


/*
===============
UI_CDKeyMenu
===============
*/
void UI_CDKeyMenu( void ) {
	UI_CDKeyMenu_Init();
	UI_PushMenu( &cdkeyMenuInfo.menu );
}


/*
===============
UI_CDKeyMenu_f
===============
*/
void UI_CDKeyMenu_f( void ) {
	UI_CDKeyMenu();
}
