//
// cg_util.c -- utility functions for use with CGame
#include "cg_local.h"

#ifdef _MENU_SCOREBOARD
	#include "../ui/ui_shared.h"
#endif

/*
=================
CG_Cvar_Get

Generic get value from Cvar
=================
*/
float CG_Cvar_Get( const char * cvar )
{
	// allocate temp string buffer
	char buff[128];
	memset( buff, 0, sizeof(buff) );

	// get variable as string
	trap_Cvar_VariableStringBuffer( cvar, buff, sizeof(buff) );

	// return converted value as float
	return atof( buff );
}

/*
=================
MF_CG_GetGameset
=================
*/

// gets the mf_gameset variable into the correct a MF_GAMESET_x return
unsigned long MF_CG_GetGameset( qboolean asEnum )
{
	char * pGameSet = NULL;
	const char * pInfo = NULL;
	unsigned long returnValue = MF_GAMESET_MODERN;

	// get info
	pInfo = CG_ConfigString( CS_SERVERINFO );
	pGameSet = Info_ValueForKey( pInfo, "mf_gameset" );

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

