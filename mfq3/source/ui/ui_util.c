/*
 * $Id: ui_util.c,v 1.5 2002-02-21 09:46:37 sparky909_uk Exp $
*/
//
// origin: rad
// new ui support stuff
// 
// memory, string alloc

#include "ui_local.h"

/*
===============
MF_UI_GetTeam
===============
*/
// uses the client info data to extract a MFQ3 team type
unsigned int MF_UI_GetTeam( void )
{
	char info[ MAX_INFO_STRING ];	// info text buffer
	uiClientState_t	cs;				// client state struct
	int teamIdx;

	// get the current client state & info text
	trap_GetClientState( &cs );
	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );

	// find out the team index
	teamIdx = atoi( Info_ValueForKey( info, "t" ) );

	// find return
	switch( teamIdx )
	{
	// Red Team
	case 1:
		return MF_TEAM_1;

	// Blue team
	case 2:
		return MF_TEAM_2;
	}

	// Spectator/Any
	return MF_TEAM_ANY;
}

/*
=================
MF_UI_GetGameset
=================
*/

// gets the mf_gameset variable into the correct a MF_GAMESET_x return
unsigned long MF_UI_GetGameset( qboolean asEnum )
{
	char * pGameSet = NULL;
	char info[MAX_INFO_STRING];
	char * pInfo = NULL;
	unsigned long returnValue = MF_GAMESET_MODERN;

	// get info
	trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	pGameSet = Info_ValueForKey( info, "mf_gameset" );

	// compare
	if( Q_stricmp( pGameSet, "ww1" ) == 0 )
	{
		returnValue = MF_GAMESET_WW1;
	}
	else if( Q_stricmp( pGameSet, "ww2" ) == 0 )
	{
		returnValue = MF_GAMESET_WW2;
	}
	else if( Q_stricmp( pGameSet, "modern" ) == 0 )
	{
		returnValue = MF_GAMESET_MODERN;
	}

	// convert to enum?
	if( asEnum )
	{
		switch( returnValue )
		{
		case MF_GAMESET_WW1:
			returnValue = 2;
			break;
		case MF_GAMESET_WW2:
			returnValue = 1;
			break;
		case MF_GAMESET_MODERN:
			returnValue = 0;
			break;
		}
	}

	// return the result (as either flag mask or enum)
	return returnValue;
}

