/*
 * $Id: g_missions.c,v 1.4 2003-02-06 14:40:13 thebjoern Exp $
 */

 
#include "g_local.h"


static void G_SpawnMissionVehicles(mission_vehicle_t* vehs)
{
	// now that we have valid vehicle data make them spawn here
	// use SP_misc_plane etc and set the relevant values to spawn them
	// rest should work automatically
}


static void G_LoadOverviewAndEntities( char *filename,
										mission_overview_t* overview,
										mission_vehicle_t* vehs, 
										mission_groundInstallation_t* gis)
{
	int					len;
	fileHandle_t		f;
	char				inbuffer[MAX_MISSION_TEXT];

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

	MF_ParseMissionScripts(inbuffer, overview, vehs, gis);
}

void G_LoadMissionScripts()
{
	char				filename[MAX_FILEPATH];
	char				buffer[33];
	mission_overview_t	overview;
	mission_vehicle_t	vehicles[IGME_MAX_VEHICLES/4];
	mission_groundInstallation_t installations[IGME_MAX_VEHICLES/4];

	memset(&overview, 0, sizeof(overview));
	memset(&vehicles[0], 0, sizeof(vehicles));
	memset(&installations[0], 0, sizeof(installations));

	// for now just load the default mission
	trap_Cvar_VariableStringBuffer("mapname", buffer, 32);
	Com_sprintf( filename, MAX_FILEPATH, "missions/%s/default.mis", buffer );

	G_LoadOverviewAndEntities(filename, &overview, vehicles, installations);

	G_SpawnMissionVehicles(vehicles);

//	trap_Printf( va("Loaded: %s\n", inbuffer) );
}




