/*
 * $Id: cg_info.c,v 1.3 2002-01-16 19:29:36 sparky909_uk Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_info.c -- display information while data is being loading

#include "cg_local.h"

//#define MAX_LOADING_PLAYER_ICONS	16
//#define MAX_LOADING_ITEM_ICONS		26

//static int			loadingItemIconCount;
//static qhandle_t	loadingItemIcons[MAX_LOADING_ITEM_ICONS];


/*
===================
CG_DrawLoadingIcons
===================
*//*
static void CG_DrawLoadingIcons( void ) {
	int		n;
	int		x, y;

	for( n = 0; n < loadingItemIconCount; n++ ) {
		y = 400-40;
		if( n >= 13 ) {
			y += 40;
		}
		x = 16 + n % 13 * 48;
		CG_DrawPic( x, y, 32, 32, loadingItemIcons[n] );
	}
}
*/

/*
======================
CG_LoadingString

======================
*/
void CG_LoadingString( const char *s ) {
	Q_strncpyz( cg.infoScreenText, s, sizeof( cg.infoScreenText ) );

	trap_UpdateScreen();
}

/*
===================
CG_LoadingItem
===================
*//*
void CG_LoadingItem( int itemNum ) {
	gitem_t		*item;

	item = &bg_itemlist[itemNum];
	
	if ( item->icon && loadingItemIconCount < MAX_LOADING_ITEM_ICONS ) {
		loadingItemIcons[loadingItemIconCount++] = trap_R_RegisterShaderNoMip( item->icon );
	}

	CG_LoadingString( item->pickup_name );
}
*/
/*
====================
CG_DrawInformation

Draw all the status / pacifier stuff during level loading
====================
*/
void CG_DrawInformation( void ) {
	const char	*s;
	const char	*info;
	const char	*sysInfo;
	int			x, y;
	int			value;
	qhandle_t	background, loadingshot;
	char		buf[1024];

	info = CG_ConfigString( CS_SERVERINFO );
	sysInfo = CG_ConfigString( CS_SYSTEMINFO );

	// get the map name
	s = Info_ValueForKey( info, "mapname" );

	// try to load the loading shot for this map
	// NOTE: changed from 'levelshots', because only use now use the levelshots for small map previews (MM)
	loadingshot = trap_R_RegisterShaderNoMip( va( "loadingshots/%s.jpg", s ) );	
	if( !loadingshot )
	{
		// use default
		loadingshot = trap_R_RegisterShaderNoMip( "ui/assets/noLoadingPicture.jpg" );
	}

	// load background
	background = trap_R_RegisterShaderNoMip( "ui/assets/backscreen.jpg" );	
	if( !background )
	{
		// use default
		background = trap_R_RegisterShaderNoMip( "textures/sfx/logo512" );
	}

	// full screen render
	trap_R_SetColor( NULL );
	CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, background );

	// alpha blend the 1024x256 loading shot over the picture frame
	if( loadingshot )
	{
		CG_DrawPic( 0, 160, 640, 160, loadingshot );
	}
/*
	// draw the icons of things as they are loaded
	CG_DrawLoadingIcons();
*/
	// draw info string information
	y = 320 + 16;
	x = 32;

	// the first 150 rows are reserved for the client connection
	// screen to write into
	if ( cg.infoScreenText[0] ) {
		UI_DrawProportionalString( x, y, va("Loading... %s", cg.infoScreenText),
			UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	} else {
		UI_DrawProportionalString( x, y, "Awaiting snapshot...",
			UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	}
	y += (PROP_HEIGHT * 1.5);

	// don't print server lines if playing a local game
	trap_Cvar_VariableStringBuffer( "sv_running", buf, sizeof( buf ) );
	if ( !atoi( buf ) ) {
		// server hostname
		Q_strncpyz(buf, Info_ValueForKey( info, "sv_hostname" ), 1024);
		Q_CleanStr(buf);
		UI_DrawProportionalString( x, y, buf,
			UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;

		// pure server
		s = Info_ValueForKey( sysInfo, "sv_pure" );
		if ( s[0] == '1' ) {
			UI_DrawProportionalString( x, y, "Pure Server",
				UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}

		// server-specific message of the day
		s = CG_ConfigString( CS_MOTD );
		if ( s[0] ) {
			UI_DrawProportionalString( x, y, s,
				UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}

		// some extra space after hostname and motd
		y += 10;
	}

	// map-specific message (long map name)
	s = CG_ConfigString( CS_MESSAGE );
	if ( s[0] ) {
		UI_DrawProportionalString( x, y, s,
			UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;
	}

	// cheats warning
	s = Info_ValueForKey( sysInfo, "sv_cheats" );
	if ( s[0] == '1' ) {
		UI_DrawProportionalString( x, y, "CHEATS ARE ENABLED",
			UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;
	}

	// game type
	switch ( cgs.gametype ) {
	case GT_FFA:
		s = "Free For All";
		break;
	case GT_SINGLE_PLAYER:
		s = "Single Player";
		break;
	case GT_TOURNAMENT:
		s = "Tournament";
		break;
	case GT_TEAM:
		s = "Team Deathmatch";
		break;
	case GT_CTF:
		s = "Capture The Flag";
		break;
	default:
		s = "Unknown Gametype";
		break;
	}
	UI_DrawProportionalString( x, y, s,
		UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	y += PROP_HEIGHT;
		
	value = atoi( Info_ValueForKey( info, "timelimit" ) );
	if ( value ) {
		UI_DrawProportionalString( x, y, va( "timelimit %i", value ),
			UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;
	}

	if (cgs.gametype < GT_CTF ) {
		value = atoi( Info_ValueForKey( info, "fraglimit" ) );
		if ( value ) {
			UI_DrawProportionalString( x, y, va( "fraglimit %i", value ),
				UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}
	}

	if (cgs.gametype >= GT_CTF) {
		value = atoi( Info_ValueForKey( info, "capturelimit" ) );
		if ( value ) {
			UI_DrawProportionalString( x, y, va( "capturelimit %i", value ),
				UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}
	}
}

