/*
 * $Id: ui_specifyserver.c,v 1.1 2001-11-15 15:47:38 moorman Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "ui_local.h"

/*********************************************************************************
	SPECIFY SERVER
*********************************************************************************/

#define ART_LOGO				"menu/art/background_sub"
#define SPECIFYSERVER_BACK		"menu/art/back"
#define SPECIFYSERVER_BACK_A	"menu/art/back_a"
#define SPECIFYSERVER_FIGHT		"menu/art/fight"
#define SPECIFYSERVER_FIGHT_A	"menu/art/fight_a"

#define ID_SPECIFYSERVERBACK	102
#define ID_SPECIFYSERVERGO		103

static char* specifyserver_artlist[] =
{
	ART_LOGO,
	SPECIFYSERVER_BACK,	
	SPECIFYSERVER_BACK_A,	
	SPECIFYSERVER_FIGHT,
	SPECIFYSERVER_FIGHT_A,
	NULL
};

typedef struct
{
	menuframework_s	menu;
	menutext_s		banner;
	menufield_s		domain;
	menufield_s		port;
	menubitmap_s	go;
	menubitmap_s	back;
} specifyserver_t;

static specifyserver_t	s_specifyserver;

/*
=================
SpecifyServer_Event
=================
*/
static void SpecifyServer_Event( void* ptr, int event )
{
	char	buff[256];

	switch (((menucommon_s*)ptr)->id)
	{
		case ID_SPECIFYSERVERGO:
			if (event != QM_ACTIVATED)
				break;

			if (s_specifyserver.domain.field.buffer[0])
			{
				strcpy(buff,s_specifyserver.domain.field.buffer);
				if (s_specifyserver.port.field.buffer[0])
					Com_sprintf( buff+strlen(buff), 128, ":%s", s_specifyserver.port.field.buffer );

				trap_Cmd_ExecuteText( EXEC_APPEND, va( "connect %s\n", buff ) );
			}
			break;

		case ID_SPECIFYSERVERBACK:
			if (event != QM_ACTIVATED)
				break;

			UI_PopMenu();
			break;
	}
}



/*
=================
SpecifyServer_MenuDraw
=================
*/
static void SpecifyServer_MenuDraw( void )
{
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	Menu_Draw( &s_specifyserver.menu );
}

/*
=================
SpecifyServer_MenuInit
=================
*/
void SpecifyServer_MenuInit( void )
{
	// zero set all our globals
	memset( &s_specifyserver, 0 ,sizeof(specifyserver_t) );

	SpecifyServer_Cache();

	s_specifyserver.menu.wrapAround = qtrue;
	s_specifyserver.menu.fullscreen = qtrue;
	s_specifyserver.menu.draw = SpecifyServer_MenuDraw;

	s_specifyserver.banner.generic.type			= MTYPE_BTEXT;
	s_specifyserver.banner.generic.x			= 320;
	s_specifyserver.banner.generic.y			= 160;
	s_specifyserver.banner.string				= "SPECIFY SERVER";
	s_specifyserver.banner.color  				= color_white;
	s_specifyserver.banner.style  				= UI_CENTER;

	s_specifyserver.domain.generic.type			= MTYPE_FIELD;
	s_specifyserver.domain.generic.name			= "Address:";
	s_specifyserver.domain.generic.flags		= QMF_PULSEIFFOCUS|QMF_SMALLFONT;
	s_specifyserver.domain.generic.x			= 206;
	s_specifyserver.domain.generic.y			= 220;
	s_specifyserver.domain.field.widthInChars	= 38;
	s_specifyserver.domain.field.maxchars		= 80;

	s_specifyserver.port.generic.type			= MTYPE_FIELD;
	s_specifyserver.port.generic.name			= "Port:";
	s_specifyserver.port.generic.flags			= QMF_PULSEIFFOCUS|QMF_SMALLFONT|QMF_NUMBERSONLY;
	s_specifyserver.port.generic.x				= 206;
	s_specifyserver.port.generic.y				= 250;
	s_specifyserver.port.field.widthInChars		= 6;
	s_specifyserver.port.field.maxchars			= 5;

	s_specifyserver.go.generic.type				= MTYPE_BITMAP;
	s_specifyserver.go.generic.name				= SPECIFYSERVER_FIGHT;
	s_specifyserver.go.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_specifyserver.go.generic.callback			= SpecifyServer_Event;
	s_specifyserver.go.generic.id				= ID_SPECIFYSERVERGO;
	s_specifyserver.go.generic.x				= 522;
	s_specifyserver.go.generic.y				= 425;
	s_specifyserver.go.width  					= 96;
	s_specifyserver.go.height  					= 32;
	s_specifyserver.go.focuspic					= SPECIFYSERVER_FIGHT_A;

	s_specifyserver.back.generic.type			= MTYPE_BITMAP;
	s_specifyserver.back.generic.name			= SPECIFYSERVER_BACK;
	s_specifyserver.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_specifyserver.back.generic.callback		= SpecifyServer_Event;
	s_specifyserver.back.generic.id				= ID_SPECIFYSERVERBACK;
	s_specifyserver.back.generic.x				= 20;
	s_specifyserver.back.generic.y				= 425;
	s_specifyserver.back.width  				= 96;
	s_specifyserver.back.height  				= 32;
	s_specifyserver.back.focuspic				= SPECIFYSERVER_BACK_A;

	Menu_AddItem( &s_specifyserver.menu, &s_specifyserver.banner );
	Menu_AddItem( &s_specifyserver.menu, &s_specifyserver.domain );
	Menu_AddItem( &s_specifyserver.menu, &s_specifyserver.port );
	Menu_AddItem( &s_specifyserver.menu, &s_specifyserver.go );
	Menu_AddItem( &s_specifyserver.menu, &s_specifyserver.back );

	Com_sprintf( s_specifyserver.port.field.buffer, 6, "%i", 27960 );
}

/*
=================
SpecifyServer_Cache
=================
*/
void SpecifyServer_Cache( void )
{
	int	i;

	// touch all our pics
	for (i=0; ;i++)
	{
		if (!specifyserver_artlist[i])
			break;
		trap_R_RegisterShaderNoMip(specifyserver_artlist[i]);
	}
}

/*
=================
UI_SpecifyServerMenu
=================
*/
void UI_SpecifyServerMenu( void )
{
	SpecifyServer_MenuInit();
	UI_PushMenu( &s_specifyserver.menu );
}

