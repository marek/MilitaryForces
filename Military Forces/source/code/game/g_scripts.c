/*
 * $Id: g_scripts.c,v 1.1 2005-08-22 16:07:10 thebjoern Exp $
*/

#include "g_local.h"


static int setState( gentity_t* ent, const char* token, qboolean taskaction ) {

	if( taskaction ) {

	} else {	// only to be performed on startup
		if( strcmp( token, "landed" ) == 0 ) {
			ent->s.ONOFF |= OO_LANDED;
			G_Printf( "Set %s's state to landed\n", ent->targetname );
			return 1;
		} else if( strcmp( token, "airborne" ) == 0 ) {
			ent->s.ONOFF &= ~OO_LANDED;
			G_Printf( "Set %s's state to airborne\n", ent->targetname );
			return 1;
		}
	}
	return 0;
}

static int addToGeneralBehaviour( gentity_t* ent, actionTypes_t at, const char* token ) {

	int i;

	switch( at ) {
	case AT_NEXT_WAYPOINT:
		i = findWaypoint(token);
		if( i >= 0 ) {
			ent->nextWaypoint = &level.waypointList.waypoints[i];
			G_Printf( "Set %s's next waypoint to %s\n", ent->targetname, ent->nextWaypoint->name );
			return 1;
		}
		break;
	case AT_SET_STATE:
		if( setState( ent, token, qfalse ) ) return 1;
		break;
	}

	return 0;
}

static int addToTask( scripttask_t* st, actionTypes_t at, const char* token ) {
	int i;
	
	if( !st ) return 0;

	switch( at ) {
	case AT_NEXT_WAYPOINT:
		i = findWaypoint(token);
		if( i >= 0 ) {
			st->nextWp = &level.waypointList.waypoints[i];
			G_Printf( "Added 'next_Waypoint' task (with wp %s) to current task\n", st->nextWp->name );
			return 1;
		}
		break;
	case AT_SET_STATE:
		if( setState( st->owner, token, qtrue ) ) return 1;
		return 1;
		break;
	}

	return 1;
}

static scripttask_t* addWaypointEvent( gentity_t* ent, const char* wp ) {
	scripttask_t*  st = &level.scriptList.scripts[level.scriptList.usedSTs];
	int idx;

	G_Printf( "Found waypoint event: %s\n", wp );

	idx = findWaypoint( wp );
	if( idx < 0 ) {
		G_Printf( "Unable to find waypoint %s - no scripts for this drone\n", wp );
		return 0;
	}

	// set it
	st->owner = ent;
	st->type = ST_ON_WAYPOINT;
	strncpy( st->name, wp, MAX_NAME_LENGTH );

	// increment counters
	ent->idxScriptEnd++;
	level.scriptList.usedSTs++;

	return st;
}

static int ParseScripts( gentity_t* ent, char *buf ) {
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];
	char	info[MAX_INFO_STRING];
//	qboolean mfq3map = qfalse;
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
}

int LoadVehicleScripts( gentity_t* ent, char *filename ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_BOTS_TEXT];
	unsigned int	stidx = level.scriptList.usedSTs;

	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) {
		trap_Printf( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return 0;
	}
	if ( len >= MAX_BOTS_TEXT ) {
		trap_Printf( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_BOTS_TEXT ) );
		trap_FS_FCloseFile( f );
		return 0;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );

	if( ParseScripts( ent, buf ) < 0 ) {
		ent->idxScriptBegin = ent->idxScriptEnd = -1;
		level.scriptList.usedSTs = stidx;
		return 0;
	}

	return 1;
}

