/*
 * $Id: g_missions.c,v 1.2 2003-02-02 02:52:03 thebjoern Exp $
 */

 
#include "g_local.h"

static void ParseOverview( char *buf, mission_overview_t* overview ) 
{
	char	*token;
//	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];
	int     openbraces = 0;

	// TODO: count number of { and } and if < 0 (ie more closed than opened
	// then we know overview is finished and return

	token = COM_Parse( &buf );
	if( !token[0] ) 
	{
		//trap_Printf(va("Token: '%s' - END\n", token));
		return;
	}
	trap_Printf(va("O-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Com_Printf( "Missing { in overview\n" );
		return;
	} 

	info[0] = '\0';
	while( 1 ) 
	{
		token = COM_ParseExt( &buf, qtrue );
		if( !token[0] ) 
		{
//				Com_Printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Com_Printf( "Unexpected { found\n" );
			break;
		}
		if( !strcmp( token, "}" ) ) 
		{
			break;
		}
		if( !strcmp( token, "map" ) )
		{
			token = COM_ParseExt( &buf, qfalse );
			if( token[0] )
			{
				Q_strncpyz(overview->mapname, token, MAX_NAME_LENGTH );
				continue;
			}
		}
		else if( !strcmp( token, "gameset" ) )
		{
			token = COM_ParseExt( &buf, qfalse );
			if( token[0] )
			{
				overview->gameset = atoi(token);
				continue;
			}
		}
		else if( !strcmp( token, "gametype" ) )
		{
			token = COM_ParseExt( &buf, qfalse );
			if( token[0] )
			{
				overview->gametype = atoi(token);
				continue;
			}
		}
		else if( !strcmp( token, "mission" ) )
		{
			token = COM_ParseExt( &buf, qfalse );
			if( token[0] )
			{
				Q_strncpyz(overview->missionname, token, MAX_NAME_LENGTH );
				continue;
			}
		}
		else if( !strcmp( token, "goal" ) )
		{
			token = COM_ParseExt( &buf, qfalse );
			if( token[0] )
			{
				Q_strncpyz(overview->objective, token, MAX_NAME_LENGTH );
				continue;
			}
		}
	}
	MF_CheckMissionScriptOverviewValid(overview, qfalse);// set to true after format change
}

static void ParseEntities( char *buf ) 
{
	char	*token;

	while( 1 )
	{
		token = COM_Parse( &buf );
		if( !token[0] ) 
		{
			trap_Printf(va("Token: '%s' - END\n", token));
			break;
		}
		trap_Printf(va("E-Token: '%s' - VALID\n", token));
	}

}

static void ParseMissionScripts( char *buf ) 
{
	char				*token;
	mission_overview_t	overview;

	MF_SetMissionScriptOverviewDefaults(&overview);

	while( 1 )
	{
		token = COM_Parse( &buf );
		if( !token[0] ) 
		{
			trap_Printf(va("Token: '%s' - END\n", token));
			break;
		}
		else if( !strcmp(token, "Overview" ) )
		{
			ParseOverview(buf, &overview);
			continue;
		}
		else if( !strcmp(token, "Entities" ) )
		{
			ParseEntities(buf);
			continue;
		}
		else
		{
			trap_Printf(va(S_COLOR_RED "Invalid Token - parsing cancelled", token));
			break;
		}
	}
	
	// TODO:
	// First get the task (ie Overview or Entities)
	// Then depending on that parse info (if no overview then stop it)
	// and go through entities and ground installations to spawn them
	// Also make sure we are using the right gameset

	
/*	
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];
	qboolean mfq3map = qfalse;
	int     openbraces = 0;
	scripttask_t* currentTask = 0;

	ent->idxScriptBegin = ent->idxScriptEnd = level.scriptList.usedSTs;

	count = 0;

	while ( 1 ) {
		token = COM_Parse( &buf );
		if( !token[0] ) {
			break;
		}
		if( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		} else {
			openbraces++;
		}

		info[0] = '\0';
		while( 1 ) {
			token = COM_ParseExt( &buf, qtrue );
			if( !token[0] ) {
//				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if( !strcmp( token, "{" ) ) {
				openbraces++;
				continue;
			}
			if( !strcmp( token, "}" ) ) {
				openbraces--;
				currentTask = 0;
				if( !openbraces ) break;
				continue;
			}
			if( level.scriptList.usedSTs >= MAX_SCRIPT_TASKS ) {
				Com_Printf( "Exceeding maximum number of scripts - parsing cancelled\n" );
				return -1;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, qfalse );
			if( !token[0] ) {
				strcpy( token, "<NULL>" );
			}
			Info_SetValueForKey( info, key, token );
			// no token -> no action
			if( !token ) continue;
			// parse all possible keys
				// keys which change the tasks
			if( strcmp( key, "on_Waypoint" ) == 0 ) {
				currentTask = addWaypointEvent( ent, token );
				if( !currentTask ) return -1;				
				continue;
			}
				// keys which add to tasks
			if( strcmp( key, "next_Waypoint" ) == 0 ) {
				if( !addToTask( currentTask, AT_NEXT_WAYPOINT, token ) ) {
					if( !addToGeneralBehaviour(ent, AT_NEXT_WAYPOINT, token) ) {
						Com_Printf( "Error in script %s %s\n", key, token );
						return -1;
					}
				}
				continue;
			}
				// keys which change the state
			if( strcmp( key, "set_State" ) == 0 ) {
				if( !addToTask( currentTask, AT_SET_STATE, token ) ) {
					if( !addToGeneralBehaviour(ent, AT_SET_STATE, token) ) {
						Com_Printf( "Error in script %s %s\n", key, token );
						return -1;
					}

				}
			}
		}
	}
	return count;

*/
}


void MF_LoadMissionScripts()
{
	int				len;
	fileHandle_t	f;
	char			inbuffer[MAX_MISSION_TEXT];
	char			filename[MAX_FILEPATH];
	char			buffer[33];

	// for now just load the default mission
	trap_Cvar_VariableStringBuffer("mapname", buffer, 32);
	Com_sprintf( filename, MAX_FILEPATH, "missions/%s/default.mis", buffer );

	// open the file, fill it into buffer and close it, afterwards parse it
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) 
	{
		trap_Printf( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return;
	}
	if ( len >= MAX_MISSION_TEXT ) 
	{
		trap_Printf( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_MISSION_TEXT ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( inbuffer, len, f );
	inbuffer[len] = 0;
	trap_FS_FCloseFile( f );

	trap_Printf( va(S_COLOR_GREEN "Successfully opened mission script: %s\n", filename) );

	ParseMissionScripts(inbuffer);

//	trap_Printf( va("Loaded: %s\n", inbuffer) );
}
