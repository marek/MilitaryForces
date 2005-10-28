/*
 * $Id: g_missions.c,v 1.3 2005-10-28 13:06:54 thebjoern Exp $
 */

 
#include "g_local.h"


static void G_SpawnMissionVehicles(mission_vehicle_t* vehs)
{
	int			i;
	gentity_t*	ent;

	for( i = 0; i < IGME_MAX_VEHICLES/4; ++i )
	{
		if( !vehs[i].used ) break;
		ent = G_Spawn();
		ent->classname = "misc_vehicle";
		VectorCopy(vehs[i].origin, ent->s.origin);
		VectorCopy(vehs[i].angles, ent->s.angles);
		ent->model = G_NewString(vehs[i].objectname);
		ent->s.modelindex = vehs[i].index;
		SP_misc_vehicle(ent);
	}

/*
		for( j = 0; j < IGME_MAX_WAYPOINTS; ++j )
		{
			if( vehs[i].waypoints[j].used )
			{
				veh->waypoints[k].active = true;
				veh->waypoints[k].selected = false;
				VectorCopy( vehs[i].waypoints[j].origin, veh->waypoints[k].origin );
				k++;
			}
		}
	}
*/
}

static void G_SpawnMissionGroundInstallations(mission_groundInstallation_t* gis)
{
	int			i;
	gentity_t*	ent;


	for( i = 0; i < IGME_MAX_VEHICLES/4; ++i )
	{
		if( !gis[i].used ) break;
		ent = G_Spawn();
		ent->classname = "misc_vehicle";
		VectorCopy(gis[i].origin, ent->s.origin);
		VectorCopy(gis[i].angles, ent->s.angles);
		ent->s.modelindex = -1;
		ent->s.modelindex2 = gis[i].index;
		SP_misc_groundinstallation(ent);
	}
}


static bool G_LoadOverviewAndEntities( char *filename,
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
		trap_Print( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return false;
	}
	if ( len >= MAX_MISSION_TEXT ) 
	{
		trap_Print( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_MISSION_TEXT ) );
		trap_FS_FCloseFile( f );
		return false;
	}

	trap_FS_Read( inbuffer, len, f );
	inbuffer[len] = 0;
	trap_FS_FCloseFile( f );

	trap_Print( va(S_COLOR_GREEN "Successfully opened mission script: %s\n", filename) );

	MF_ParseMissionScripts(inbuffer, overview, vehs, gis);

	return true;
}

void G_LoadMissionScripts()
{
	char				filename[MAX_FILEPATH];
	char				missionname[32];
	char				buffer[33];
	mission_overview_t	overview;
	mission_vehicle_t	vehicles[IGME_MAX_VEHICLES/4];
	mission_groundInstallation_t installations[IGME_MAX_VEHICLES/4];

	memset(&overview, 0, sizeof(overview));
	memset(&vehicles[0], 0, sizeof(vehicles));
	memset(&installations[0], 0, sizeof(installations));

	if( strlen(mf_mission.string) > 31 ) 
	{
		Com_Printf(S_COLOR_RED "Missionname in mf_mission too long -> no mission loaded\n");
		return;
	}
	Q_strncpyz(missionname, mf_mission.string, 32);
	if( !Q_stricmp(missionname, "") ||
		!Q_stricmp(missionname, "none") )
	{
		Com_Printf("Not loading mission, as requested.\n");
		return;
	}

	// for now just load the default mission
	trap_Cvar_VariableStringBuffer("mapname", buffer, 32);
	Com_sprintf( filename, MAX_FILEPATH, "missions/%s/%s.mis", buffer, missionname );

	if( !G_LoadOverviewAndEntities(filename, &overview, vehicles, installations) )
		return;

	G_SpawnMissionVehicles(vehicles);

	G_SpawnMissionGroundInstallations(installations);

//	trap_Print( va("Loaded: %s\n", inbuffer) );
}




