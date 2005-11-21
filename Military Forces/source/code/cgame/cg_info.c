/*
 * $Id: cg_info.c,v 1.4 2005-11-21 17:28:20 thebjoern Exp $
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

	SCR_UpdateScreen();
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
		loadingItemIcons[loadingItemIconCount++] = refExport.RegisterShaderNoMip( item->icon );
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
	const char	* s = NULL, * pGameset = NULL;
	const char	* info = NULL;
	const char	* sysInfo = NULL;
	int			x, y;
	int			value;
	qhandle_t	background = 0, loadingshot = 0;
	char		buf[1024] = { 0 } ;
	char		flagBuf[64] = { 0 };

	info = CG_ConfigString( CS_SERVERINFO );
	sysInfo = CG_ConfigString( CS_SYSTEMINFO );

	// get the map name
	s = Info_ValueForKey( info, "mapname" );

	// try to load the loading shot for this map
	// NOTE: changed from 'levelshots', because only use now use the levelshots for small map previews (MM)
	loadingshot = refExport.RegisterShaderNoMip( va( "loadingshots/%s.jpg", s ) );	

	// load background
	background = refExport.RegisterShaderNoMip( "ui/assets/backscreen.jpg" );	
	if( !background )
	{
		// use default
		background = refExport.RegisterShaderNoMip( "textures/sfx/logo512" );
	}

	// full screen render
	refExport.SetColor( NULL );
	CG_DrawPic( 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, background );

	// alpha blend the 1024x256 loading shot (or gameset artwork) over the picture frame
	if( !loadingshot )
	{
		// if no loading shot, display the correct gameset artwork
		pGameset = Info_ValueForKey( info, "mf_gameset" );
		if( pGameset[0] )
		{
			loadingshot = refExport.RegisterShaderNoMip( va( "ui\\assets\\mid-%s", pGameset ) );
		}
	}

	// something to display?
	if( loadingshot )
	{
		CG_DrawPic( 0, 160, 640, 160, loadingshot );
	}
/*
	// draw the icons of things as they are loaded
	CG_DrawLoadingIcons();
*/
	// map-specific message (long map name)
	s = CG_ConfigString( CS_MESSAGE );
	if ( s[0] )
	{
		UI_DrawProportionalString( 632, 300, s, UI_RIGHT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	}

	// draw loading progress text
	if ( cg.infoScreenText[0] )
	{
		UI_DrawProportionalString( 320, 170, va("Loading... %s", cg.infoScreenText), UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	}
	else
	{
		UI_DrawProportionalString( 320, 170, "Awaiting snapshot...", UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	}
	//CG_DrawStringExt(0, 150, cg.infoCurrentLoadItemText, colorLtGrey, true, false, 6,6, strlen(cg.infoCurrentLoadItemText));
	//p.x = 0;
	//p.y = 323;
	//p.h = 15;
	//p.w = 640;
	//CG_DrawProgressBar(&p, colorDkGrey, colorWhite, 12.0f, UI_SMALLFONT|UI_INVERSE|UI_CENTER, cg.infoLoadFraction);

	// draw info string information
	x = 32;
	y = 348;

	// server information (OPTIONAL - if not a local game)
	Cvar_VariableStringBuffer( "sv_running", buf, sizeof( buf ) );
	if( !atoi( buf ) )
	{		
		// pure?
		s = Info_ValueForKey( sysInfo, "sv_pure" );
		if ( s[0] == '1' )
		{
			strcat( flagBuf, "+Pure " );
		}

		// cheats?
		s = Info_ValueForKey( sysInfo, "sv_cheats" );
		if ( s[0] == '1' )
		{
			strcat( flagBuf, "+Cheats " );
		}

		// server hostname
		Q_strncpyz(buf, Info_ValueForKey( info, "sv_hostname" ), 1024);
		Q_CleanStr(buf);
		UI_DrawProportionalString( x, y, va( "server: %s %s", buf, flagBuf ), UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;

		// server-specific message of the day
		s = CG_ConfigString( CS_MOTD );
		if ( s[0] )
		{
			UI_DrawProportionalString( x, y, s, UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}

		// some extra space after hostname and motd
		y += 10;
	}

	// game type
	switch ( cgs.gametype ) {
	case GT_FFA:
		s = "Free For All";
		break;
	case GT_SINGLE_PLAYER:
		s = "Single Player";
		break;
	case GT_MISSION_EDITOR:
		s = "MFQ3 Mission Editor";
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
	UI_DrawProportionalString( x, y, s, UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
	y += PROP_HEIGHT;

	// timelimit (OPTIONAL)
	value = atoi( Info_ValueForKey( info, "timelimit" ) );
	if ( value )
	{
		UI_DrawProportionalString( x, y, va( "timelimit: %i", value ), UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
		y += PROP_HEIGHT;
	}

	// fraglimit OR ...
	if (cgs.gametype < GT_CTF ) 
	{
		value = atoi( Info_ValueForKey( info, "fraglimit" ) );
		if ( value ) {
			UI_DrawProportionalString( x, y, va( "fraglimit: %i", value ), UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}
	}
	// ... capturelimit
	if (cgs.gametype >= GT_CTF)
	{
		value = atoi( Info_ValueForKey( info, "capturelimit" ) );
		if ( value ) {
			UI_DrawProportionalString( x, y, va( "capturelimit: %i", value ), UI_LEFT|UI_SMALLFONT|UI_DROPSHADOW, colorWhite );
			y += PROP_HEIGHT;
		}
	}
}

