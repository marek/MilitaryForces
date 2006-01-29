#ifndef __G_LEVEL_H__
#define __G_LEVEL_H__

#include "q_shared.h"
#include "g_client.h"
#include "g_entity.h"
#include <vector>


struct SpawnFieldHolder;
struct spawn_t;


struct LevelLocals
{
	typedef std::vector<ClientBase*> ClientList;
	typedef std::vector<EntityBase*> EntityList;

	static const int k_MAX_SPAWN_VARS =	64;
	static const int k_MAX_SPAWN_VARS_CHARS = 2048;

					LevelLocals();
					~LevelLocals();

	static LevelLocals& getInstance();

	void			resetAll();					// reset all variables
	void			resetClientsAndEntities();// reset client and entity lists (prepare for use)

	void			addEntity(GameEntity* newEnt);	// add a new ent to the list, isClient = true for players only
	void			removeEntity(GameEntity* entToRemove);
	void			addWorld(GameEntity* world);

	GameEntity*		spawnEntity();

	GameEntity*		getEntity(int entNum);
	GameClient*		getClient(int clientNum);

	GameClient*		createNewClient(int clientNum);
	void			setPlayerEntity(GameEntity* playerEnt, int clientNum);

	void			createEntityFromSpawnFields(SpawnFieldHolder* sfh, spawn_t* s);
	void			createItemFromSpawnFields(SpawnFieldHolder* sfh, gitem_t* item);
	void			setEntityFromSpawnFields(GameEntity* ent, SpawnFieldHolder* sfh);

	void			setServerGameData();

	// data

	int				num_entities_;				// is the size() of gameEntities_ // before:current number, <= MAX_GENTITIES

	int				warmupTime_;				// restart match at this time

	fileHandle_t	logFile_;

	// store latched cvars here that we want to get at often
	int				maxclients_;

	int				framenum_;
	int				time_;						// in msec
	int				previousTime_;				// so movers can back up when blocked

	int				startTime_;					// level.time the map was started

	int				teamScores_[ClientBase::TEAM_NUM_TEAMS];
	int				lastTeamLocationTime_;		// last time of client team location update

	bool			newSession_;				// don't use any old session data, because
												// we changed gametype

	bool			restarted_;					// waiting for a map_restart to fire

	int				numConnectedClients_;
	int				numNonSpectatorClients_;	// includes connecting clients
	int				numPlayingClients_;			// connected, non-spectators
	int				sortedClients_[MAX_CLIENTS];// sorted by score
	int				follow1_; 
	int				follow2_;					// clientNums for auto-follow spectators

	int				warmupModificationCount_;	// for detecting if g_warmup is changed

	// voting state
	char			voteString_[MAX_STRING_CHARS];
	char			voteDisplayString_[MAX_STRING_CHARS];
	int				voteTime_;					// level.time vote was called
	int				voteExecuteTime_;			// time the vote is executed
	int				voteYes_;
	int				voteNo_;
	int				numVotingClients_;			// set by CalculateRanks

	// team voting state
	char			teamVoteString_[2][MAX_STRING_CHARS];
	int				teamVoteTime_[2];			// level.time vote was called
	int				teamVoteYes_[2];
	int				teamVoteNo_[2];
	int				numteamVotingClients_[2];	// set by CalculateRanks

	// spawn variables
	bool			spawning_;					// the G_Spawn*() functions are valid
	int				numSpawnVars_;
	char*			spawnVars_[k_MAX_SPAWN_VARS][2];	// key / value pairs
	int				numSpawnVarChars_;
	char			spawnVarChars_[k_MAX_SPAWN_VARS_CHARS];
	// MFQ3 spawning
	int				ent_category_;

	// intermission state
	int				intermissionQueued_;		// intermission was qualified, but
												// wait INTERMISSION_DELAY_TIME before
												// actually going there so the last
												// frag can be watched.  Disable future
												// kills during this delay
	int				intermissiontime_;		// time the intermission was started
	char*			changemap_;
	bool			readyToExit_;			// at least one client wants to exit
	int				exitTime_;
	vec3_t			intermission_origin_;	// also used for spectator spawns
	vec3_t			intermission_angle_;

	bool			locationLinked_;			// target_locations get linked
	GameEntity*		locationHead_;			// head of the location list

private:
	ClientList		clients_;					// [maxclients]

	EntityList		gameEntities_;				// any game entity
	//waypointList_t	waypointList_;
	//scripttaskList_t scriptList_; 
};











#endif // __G_LEVEL_H__

