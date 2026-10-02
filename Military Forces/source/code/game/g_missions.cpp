/*
 * $Id: g_missions.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
 */


#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"
#include "g_miscvehicle.h"


static void G_SpawnMissionVehicles(mission_vehicle_t* vehs)
{
	for( int i = 0; i < IGME_MAX_VEHICLES; ++i )
	{
		if( !vehs[i].used ) break;
		GameEntity* ent = theLevel.spawnEntity();
		ent->classname_ = "misc_vehicle";
		VectorCopy(vehs[i].origin, ent->s.origin);
		VectorCopy(vehs[i].angles, ent->s.angles);
		ent->model_ = G_NewString(vehs[i].objectname);
		ent->s.modelindex = vehs[i].index;
		SP_misc_vehicle(ent);
	}
}

static void G_SpawnMissionGroundInstallations(mission_groundInstallation_t* gis)
{
	for( int i = 0; i < IGME_MAX_VEHICLES; ++i )
	{
		if( !gis[i].used ) break;
		GameEntity* ent = theLevel.spawnEntity();
		ent->classname_ = "misc_vehicle";
		VectorCopy(gis[i].origin, ent->s.origin);
		VectorCopy(gis[i].angles, ent->s.angles);
		ent->model_ = G_NewString(gis[i].objectname);
		if( gis[i].teamname[0] )
			ent->team_ = G_NewString(gis[i].teamname);
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
	static char			inbuffer[MAX_MISSION_TEXT];

	// open the file, fill it into buffer and close it, afterwards parse it
	len = FS_FOpenFileByMode( filename, &f, FS_READ );
	if ( !f )
	{
		Com_Printf( S_COLOR_RED "file not found: %s\n", filename );
		return false;
	}
	if ( len >= MAX_MISSION_TEXT )
	{
		Com_Printf( S_COLOR_RED "file too large: %s is %i, max allowed is %i\n", filename, len, MAX_MISSION_TEXT );
		FS_FCloseFile( f );
		return false;
	}

	FS_Read2( inbuffer, len, f );
	inbuffer[len] = 0;
	FS_FCloseFile( f );

	Com_Printf( S_COLOR_GREEN "Successfully opened mission script: %s\n", filename );

	MF_ParseMissionScripts(inbuffer, overview, vehs, gis);

	return true;
}

void G_LoadMissionScripts()
{
	char				filename[MAX_FILEPATH];
	char				missionname[32];
	char				buffer[33];
	mission_overview_t	overview;
	static mission_vehicle_t	vehicles[IGME_MAX_VEHICLES];
	static mission_groundInstallation_t installations[IGME_MAX_VEHICLES];

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

	Cvar_VariableStringBuffer("mapname", buffer, 32);
	Com_sprintf( filename, MAX_FILEPATH, "missions/%s/%s.mis", buffer, missionname );

	if( !G_LoadOverviewAndEntities(filename, &overview, vehicles, installations) )
		return;

	G_SpawnMissionVehicles(vehicles);

	G_SpawnMissionGroundInstallations(installations);
}
