/*
 * $Id: ui_util.c,v 1.3 2002-01-23 18:47:22 sparky909_uk Exp $
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
