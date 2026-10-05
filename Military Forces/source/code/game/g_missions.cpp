/*
 * $Id: g_missions.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
 */


#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"
#include "g_miscvehicle.h"


void Game::spawnMissionVehicles(mission_vehicle_t* vehs)
{
	for( int i = 0; i < IGME_MAX_VEHICLES; ++i )
	{
		if( !vehs[i].used ) break;
		GameEntity* ent = theLevel.spawnEntity();
		ent->classname_ = "misc_vehicle";
		VectorCopy(vehs[i].origin, ent->s.origin);
		VectorCopy(vehs[i].angles, ent->s.angles);
		ent->model_ = Game::newString(vehs[i].objectname);
		ent->s.modelindex = vehs[i].index;
		if( vehs[i].side[0] )
			ent->team_ = Game::newString(vehs[i].side);
		GameEntity* veh = SP_misc_vehicle(ent);
		if( veh )
			MiscVehicle_SetRoute(veh, vehs[i].waypoints);
	}
}

void Game::spawnMissionGroundInstallations(mission_groundInstallation_t* gis)
{
	for( int i = 0; i < IGME_MAX_VEHICLES; ++i )
	{
		if( !gis[i].used ) break;
		GameEntity* ent = theLevel.spawnEntity();
		ent->classname_ = "misc_vehicle";
		VectorCopy(gis[i].origin, ent->s.origin);
		VectorCopy(gis[i].angles, ent->s.angles);
		ent->model_ = Game::newString(gis[i].objectname);
		if( gis[i].teamname[0] )
			ent->team_ = Game::newString(gis[i].teamname);
		ent->s.modelindex = -1;
		ent->s.modelindex2 = gis[i].index;
		SP_misc_groundinstallation(ent);
	}
}


bool Game::loadOverviewAndEntities( char *filename,
										mission_overview_t* overview,
										mission_vehicle_t* vehs,
										mission_groundInstallation_t* gis)
{
	int					len;
	fileHandle_t		f;
	static char			inbuffer[MAX_MISSION_TEXT];

	// open the file, fill it into buffer and close it, afterwards parse it
	const OpenFile file( filename, FS_READ );
	f = file.handle();
	len = file.length();
	if ( !f )
	{
		Common::printf( S_COLOR_RED "file not found: %s\n", filename );
		return false;
	}
	if ( len >= MAX_MISSION_TEXT )
	{
		Common::printf( S_COLOR_RED "file too large: %s is %i, max allowed is %i\n", filename, len, MAX_MISSION_TEXT );
		return false;
	}

	FileSystem::read( inbuffer, len, f );
	inbuffer[len] = 0;

	Common::printf( S_COLOR_GREEN "Successfully opened mission script: %s\n", filename );

	VehicleData::parseMissionScripts(inbuffer, overview, vehs, gis);

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

	overview = {};
	memset(&vehicles[0], 0, sizeof(vehicles));
	memset(&installations[0], 0, sizeof(installations));

	if( strlen(mf_mission.string) > 31 )
	{
		Common::printf(S_COLOR_RED "Missionname in mf_mission too long -> no mission loaded\n");
		return;
	}
	Q_strncpyz(missionname, mf_mission.string, 32);
	if( !Q_stricmp(missionname, "") ||
		!Q_stricmp(missionname, "none") )
	{
		Common::printf("Not loading mission, as requested.\n");
		return;
	}

	CvarSystem::variableStringBuffer("mapname", buffer, 32);
	Com_sprintf( filename, MAX_FILEPATH, "missions/%s/%s.mis", buffer, missionname );

	if( !Game::loadOverviewAndEntities(filename, &overview, vehicles, installations) )
		return;

	Game::spawnMissionVehicles(vehicles);

	Game::spawnMissionGroundInstallations(installations);
}
