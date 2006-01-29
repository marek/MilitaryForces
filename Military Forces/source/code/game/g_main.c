#include "g_local.h"
#include "..\vehiclelib\bg_datamanager.h"
#include "g_level.h"

//level_locals_t	level;
LevelLocals& theLevel = LevelLocals::getInstance();

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
	int			modificationCount;  // for tracking changes
	bool	trackChange;	    // track this variable, and announce if changed
  bool teamShader;        // track and if changed, update shader state
} cvarTable_t;

//gentity_t		g_entities[MAX_GENTITIES];
//gclient_t		g_clients[MAX_CLIENTS];

vmCvar_t	g_gametype;
vmCvar_t	g_dmflags;
vmCvar_t	g_fraglimit;
vmCvar_t	g_timelimit;
vmCvar_t	g_capturelimit;
vmCvar_t	g_friendlyFire;
vmCvar_t	g_password;
vmCvar_t	g_needpass;
vmCvar_t	g_maxclients;
vmCvar_t	g_maxGameClients;
vmCvar_t	g_dedicated;
vmCvar_t	g_speed;
vmCvar_t	g_gravity;
vmCvar_t	g_cheats;
vmCvar_t	g_knockback;
vmCvar_t	g_forcerespawn;
vmCvar_t	g_inactivity;
vmCvar_t	g_debugMove;
vmCvar_t	g_debugDamage;
vmCvar_t	g_debugAlloc;
vmCvar_t	g_weaponRespawn;
vmCvar_t	g_weaponTeamRespawn;
vmCvar_t	g_motd;
vmCvar_t	g_synchronousClients;
vmCvar_t	g_warmup;
vmCvar_t	g_doWarmup;
vmCvar_t	g_restarted;
vmCvar_t	g_log;
vmCvar_t	g_logSync;
vmCvar_t	g_allowVote;
vmCvar_t	g_teamAutoJoin;
vmCvar_t	g_teamForceBalance;
vmCvar_t	g_teamForceBalanceNum;
vmCvar_t	g_banIPs;
vmCvar_t	g_filterBan;
vmCvar_t	g_smoothClients;
vmCvar_t	pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	g_rankings;
vmCvar_t	g_listEntity;
vmCvar_t	g_spectSpeed;

// MFQ3
vmCvar_t	mf_gameset;
vmCvar_t	mf_lvcat;
vmCvar_t	mf_version;
vmCvar_t	mf_mission;
vmCvar_t	mf_allowNukes;

cvarTable_t		gameCvarTable[] = {
	// don't override the cheat state set by the system
	{ &g_cheats, "sv_cheats", "", 0, 0, false },

	// noset vars
	{ NULL, "gamename", GAME_IDENTIFIER , CVAR_SERVERINFO | CVAR_ROM, 0, false  },
	{ NULL, "gamedate", __DATE__ , CVAR_ROM, 0, false  },
	{ &g_restarted, "g_restarted", "0", CVAR_ROM, 0, false  },
	{ NULL, "sv_mapname", "", CVAR_SERVERINFO | CVAR_ROM, 0, false  },

	// latched vars
	{ &g_gametype, "g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH, 0, false  },

	{ &g_maxclients, "sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, false  },
	{ &g_maxGameClients, "g_maxGameClients", "0", CVAR_SERVERINFO | CVAR_LATCH | CVAR_ARCHIVE, 0, false  },

	// MFQ3 vars
	{ &mf_gameset, "mf_gameset", "modern", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, false  },
	{ &mf_version, "mf_version", "unknown", CVAR_SERVERINFO | CVAR_ROM, 0, false  },
	{ &mf_lvcat, "mf_lvcat", "0", CVAR_SERVERINFO | CVAR_ROM, 0, false },
	{ &mf_mission, "mf_mission", "default", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, false  },
	{ &mf_allowNukes, "mf_allowNukes", "1", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH, 0, false  },

	// change anytime vars
	{ &g_dmflags, "dmflags", "0", CVAR_SERVERINFO | CVAR_ARCHIVE, 0, true  },
	{ &g_fraglimit, "fraglimit", "20", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, true },
	{ &g_timelimit, "timelimit", "0", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, true },
	{ &g_capturelimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART, 0, true },

	{ &g_synchronousClients, "g_synchronousClients", "0", CVAR_SYSTEMINFO, 0, false  },

	{ &g_friendlyFire, "g_friendlyFire", "0", CVAR_ARCHIVE, 0, true  },

	{ &g_teamAutoJoin, "g_teamAutoJoin", "0", CVAR_ARCHIVE  },
	{ &g_teamForceBalance, "g_teamForceBalance", "0", CVAR_ARCHIVE  },
	{ &g_teamForceBalanceNum, "g_teamForceBalanceNum", "1", CVAR_ARCHIVE  },	// allowed spread when g_teamForceBalance == 1

	{ &g_warmup, "g_warmup", "20", CVAR_ARCHIVE, 0, true  },
	{ &g_doWarmup, "g_doWarmup", "0", 0, 0, true  },
	{ &g_log, "g_log", "games.log", CVAR_ARCHIVE, 0, false  },
	{ &g_logSync, "g_logSync", "0", CVAR_ARCHIVE, 0, false  },

	{ &g_password, "g_password", "", CVAR_USERINFO, 0, false  },

	{ &g_banIPs, "g_banIPs", "", CVAR_ARCHIVE, 0, false  },
	{ &g_filterBan, "g_filterBan", "1", CVAR_ARCHIVE, 0, false  },

	{ &g_needpass, "g_needpass", "0", CVAR_SERVERINFO | CVAR_ROM, 0, false },

	{ &g_dedicated, "dedicated", "0", 0, 0, false  },

	{ &g_speed, "g_speed", "320", 0, 0, true  },
	{ &g_gravity, "g_gravity", "800", 0, 0, true  },
	{ &g_knockback, "g_knockback", "1000", 0, 0, true  },
	{ &g_weaponRespawn, "g_weaponrespawn", "5", 0, 0, true  },
	{ &g_weaponTeamRespawn, "g_weaponTeamRespawn", "30", 0, 0, true },
	{ &g_forcerespawn, "g_forcerespawn", "20", 0, 0, true },
	{ &g_inactivity, "g_inactivity", "0", 0, 0, true },
	{ &g_debugMove, "g_debugMove", "0", 0, 0, false },
	{ &g_debugDamage, "g_debugDamage", "0", 0, 0, false },
	{ &g_debugAlloc, "g_debugAlloc", "0", 0, 0, false },
	{ &g_motd, "g_motd", "", 0, 0, false },

	{ &g_allowVote, "g_allowVote", "1", CVAR_ARCHIVE, 0, false },
	{ &g_listEntity, "g_listEntity", "0", 0, 0, false },

	{ &g_smoothClients, "g_smoothClients", "1", 0, 0, false},
	{ &pmove_fixed, "pmove_fixed", "1", CVAR_SYSTEMINFO|CVAR_ROM, 0, false},
	{ &pmove_msec, "pmove_msec", "8", CVAR_SYSTEMINFO, 0, false},

	{ &g_rankings, "g_rankings", "0", 0, 0, false},
	{ &g_spectSpeed, "g_spectSpeed", "400", CVAR_ARCHIVE, 0, false}

};

int		gameCvarTableSize = sizeof( gameCvarTable ) / sizeof( gameCvarTable[0] );


void G_InitGame( int levelTime, int randomSeed );
void G_RunFrame( int levelTime );
void G_ShutdownGame();
void CheckExitRules();


/*
================
G_FindTeams

Chain together all entities with a matching team field.
Entity teams are used for item groups and multi-entity mover groups.

All but the first will have the FL_TEAMSLAVE flag set and teammaster field set
All but the last will have the teamchain field set to the next one
================
*/
void G_FindTeams() 
{
	GameEntity	*e, *e2;
	int			i, j;
	int			c, c2;

	c = 0;
	c2 = 0;
	//for ( i=1, e=g_entities+i ; i < level.num_entities ; i++,e++ )
	for( i=1; i < theLevel.num_entities_; i++ )
	{
		e = theLevel.getEntity(i);
		if( !e || !e->inuse_ )
			continue;
		if( !e->team_ )
			continue;
		if( e->flags_ & FL_TEAMSLAVE )
			continue;
		e->teammaster_ = e;
		c++;
		c2++;
		for( j=i+1; j < theLevel.num_entities_ ; j++ )
		{
			e2 = theLevel.getEntity(j);
			if (!e2 || !e2->inuse_)
				continue;
			if (!e2->team_)
				continue;
			if (e2->flags_ & FL_TEAMSLAVE)
				continue;
			if (!strcmp(e->team_, e2->team_))
			{
				c2++;
				e2->teamchain_ = e->teamchain_;
				e->teamchain_ = e2;
				e2->teammaster_ = e;
				e2->flags_ |= FL_TEAMSLAVE;

				// make sure that targets only point at the master
				if ( e2->targetname_ ) 
				{
					e->targetname_ = e2->targetname_;
					e2->targetname_ = NULL;
				}
			}
		}
	}

	Com_Printf ("%i teams with %i entities\n", c, c2);
}

void G_RemapTeamShaders() 
{
}


/*
=================
G_RegisterCvars
=================
*/
void G_RegisterCvars() 
{
	int			i;
	cvarTable_t	*cv;
	bool remapped = false;

	for ( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ ) 
	{
		Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
		if ( cv->vmCvar )
			cv->modificationCount = cv->vmCvar->modificationCount;

		if (cv->teamShader) 
			remapped = true;
	}

	if (remapped) 
		G_RemapTeamShaders();

	// check some things
	if ( g_gametype.integer < 0 || g_gametype.integer >= GT_MAX_GAME_TYPE ) 
	{
		Com_Printf( "g_gametype %i is out of range, defaulting to 0\n", g_gametype.integer );
		Cvar_Set( "g_gametype", "0" );
	}
	theLevel.warmupModificationCount_ = g_warmup.modificationCount;
}

/*
=================
G_UpdateCvars
=================
*/
void G_UpdateCvars() 
{
	int			i;
	cvarTable_t	*cv;
	bool remapped = false;

	for( i = 0, cv = gameCvarTable ; i < gameCvarTableSize ; i++, cv++ )
	{
		if( cv->vmCvar ) 
		{
			Cvar_Update( cv->vmCvar );

			if( cv->modificationCount != cv->vmCvar->modificationCount )
			{
				cv->modificationCount = cv->vmCvar->modificationCount;

				if( cv->trackChange ) 
				{
					SV_GameSendServerCommand( -1, va("print \"Server: %s changed to %s\n\"", 
						cv->cvarName, cv->vmCvar->string ) );
				}

				if( cv->teamShader ) 
					remapped = true;
			}
		}
	}
	if( remapped ) 
		G_RemapTeamShaders();
}

/*
============
G_InitGame

============
*/
void G_InitGame( int levelTime, int randomSeed ) 
{
	Com_Printf ("------- Game Initialization -------\n");
	Com_Printf ("gamename: %s\n", GAME_IDENTIFIER);
	Com_Printf ("gamedate: %s\n",  __DATE__ );
	Com_Printf ("mf version: %s\n", MF_VERSION );

	srand( randomSeed );

	G_RegisterCvars();

	G_ProcessIPBans();

	G_InitMemory();

	// MFQ3: set the game version into a server var (that can be viewed as server-info)
	Cvar_Set( "mf_version", MF_VERSION );

	// MFQ3: (for now) always set the 'g_synchronousClients' var to 0, as setting to 1
	// causes glitching/flickering with the models
#pragma message( "g_synchronousClients is always being set to 0 in G_InitGame()" )
	Cvar_Set( "g_synchronousClients", "0" );

	// set some level globals
	//memset( &level, 0, sizeof( level ) );
	theLevel.resetAll();
	theLevel.time_ = levelTime;
	theLevel.startTime_ = levelTime;

	//level.snd_fry = G_SoundIndex("sound/player/fry.wav");	// FIXME standing in lava / slime

	if ( g_gametype.integer != GT_SINGLE_PLAYER && g_gametype.integer != GT_MISSION_EDITOR &&
			g_log.string[0] ) 
	{
		if ( g_logSync.integer ) 
			FS_FOpenFileByMode( g_log.string, &theLevel.logFile_, FS_APPEND_SYNC );
		else
			FS_FOpenFileByMode( g_log.string, &theLevel.logFile_, FS_APPEND );

		if ( !theLevel.logFile_ )
			Com_Printf( "WARNING: Couldn't open logfile: %s\n", g_log.string );
		else 
		{
			char serverinfo[MAX_INFO_STRING];

			SV_GetServerinfo( serverinfo, sizeof( serverinfo ) );

			G_LogPrintf("------------------------------------------------------------\n" );
			G_LogPrintf("InitGame: %s\n", serverinfo );
		}
	} 
	else
		Com_Printf( "Not logging to disk.\n" );

	G_InitWorldSession();

	// initialize all entities for this game
	//memset( g_entities, 0, MAX_GENTITIES * sizeof(g_entities[0]) );
	//level.gentities = g_entities;

	// initialize all clients for this game
	theLevel.maxclients_ = g_maxclients.integer;
	//memset( g_clients, 0, MAX_CLIENTS * sizeof(g_clients[0]) );
	//level.clients = g_clients;

	// set client fields on player ents
#pragma message("cant do this with the new system.... find solution!!")
	//for( int i=1 ; i <= theLevel.maxclients_ ; i++ ) 
	//{
	//	//g_entities[i].client = level.clients + i;
	//	reinterpret_cast<GameEntity*>(theLevel.gameEntities_.at(i))->client_ = theLevel.clients_.at(i);
	//}

	// always leave room for the max number of clients,
	// even if they aren't all used, so numbers inside that
	// range are NEVER anything but clients
	//theLevel.num_entities_ = 0;//MAX_CLIENTS;

	// let the server system know where the entites are
//	SV_LocateGameData( (void*)level.gentities, level.num_entities, sizeof( gentity_t ), 
//		&level.clients[0].ps, sizeof( level.clients[0] ) );
	theLevel.setServerGameData();//	SV_LocateGameData( &theLevel.gameEntities_, &theLevel.clients_ );

	ClearRegisteredItems();

	// MFQ3 data
	MF_LoadAllVehicleData();
	//DataManager::getInstance().createAllData();

	// parse the key/value pairs and spawn gentities
	G_SpawnEntitiesFromString();

	// general initialization
	G_FindTeams();

	// make sure we have flags for CTF, etc
	if( g_gametype.integer >= GT_TEAM ) 
		G_CheckTeamItems();

	SaveRegisteredItems();

	Com_Printf ("-----------------------------------\n");

	if( g_gametype.integer == GT_SINGLE_PLAYER || Cvar_VariableIntegerValue( "com_buildScript" ) ) 
	{
		G_SoundIndex( "sound/player/gurp1.wav" );
		G_SoundIndex( "sound/player/gurp2.wav" );
	}

	G_RemapTeamShaders();

	if( g_gametype.integer != GT_MISSION_EDITOR )
	{
		// MFQ3 mission scripts
		//G_LoadMissionScripts();

		// init some MFQ3 stuff
		//DroneInit();
	}
}



/*
=================
G_ShutdownGame
=================
*/
void G_ShutdownGame() 
{
	Com_Printf ("==== ShutdownGame ====\n");

	if ( theLevel.logFile_ ) 
	{
		G_LogPrintf("ShutdownGame:\n" );
		G_LogPrintf("------------------------------------------------------------\n" );
		FS_FCloseFile( theLevel.logFile_ );
	}

	// write all the client session data so we can get it back
	G_WriteSessionData();
}



//===================================================================



/*
========================================================================

PLAYER COUNTING / SCORE SORTING

========================================================================
*/

/*
=============
AddTournamentPlayer

If there are less than two tournament players, put a
spectator in the game and restart
=============
*/
void AddTournamentPlayer()
{
	if ( theLevel.numPlayingClients_ >= 2 ) 
		return;

	// never change during intermission
	if ( theLevel.intermissiontime_ ) 
		return;

	GameClient *nextInLine = 0;
	GameClient	*client = 0;
	for ( int i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		client = theLevel.getClient(i);
		if( !client || client->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
			continue;
		if( client->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
			continue;
		// never select the dedicated follow or scoreboard clients
		if( client->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_SCOREBOARD || 
			client->sess_.spectatorClient_ < 0  ) 
			continue;

		if ( !nextInLine || client->sess_.spectatorTime_ < nextInLine->sess_.spectatorTime_ ) 
			nextInLine = client;
	}

	if ( !nextInLine ) 
		return;

	theLevel.warmupTime_ = -1;

	// set them to free-for-all team
	//SetTeam( &g_entities[ nextInLine - level.clients ], "f" );
	SetTeam( theLevel.getEntity(nextInLine->ps_.clientNum), "f" );
}

/*
=======================
RemoveTournamentLoser

Make the loser a spectator at the back of the line
=======================
*/
void RemoveTournamentLoser() 
{
	if( theLevel.numPlayingClients_ != 2 ) 
		return;

	int clientNum = theLevel.sortedClients_[1];

	if( theLevel.getClient(clientNum)->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
		return;

	// make them a spectator
	//SetTeam( &g_entities[ clientNum ], "s" );
	SetTeam( theLevel.getEntity(clientNum), "s" );
}

/*
=======================
RemoveTournamentWinner
=======================
*/
void RemoveTournamentWinner() 
{
	if( theLevel.numPlayingClients_ != 2 ) 
		return;

	int clientNum = theLevel.sortedClients_[0];

	if( theLevel.getClient(clientNum)->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
		return;

	// make them a spectator
	//SetTeam( &g_entities[ clientNum ], "s" );
	SetTeam( theLevel.getEntity(clientNum), "s" );
}

/*
=======================
AdjustTournamentScores
=======================
*/
void AdjustTournamentScores() 
{
	int clientNum = theLevel.sortedClients_[0];
	GameClient* client = theLevel.getClient(clientNum);

	if( client->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED ) 
	{
		client->sess_.wins_++;
		ClientUserinfoChanged( clientNum );
	}

	clientNum = theLevel.sortedClients_[1];
	client = theLevel.getClient(clientNum);

	if( client->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED ) 
	{
		client->sess_.losses_++;
		ClientUserinfoChanged( clientNum );
	}
}

/*
=============
SortRanks

=============
*/
int QDECL SortRanks( const void *a, const void *b ) 
{
	GameClient	*ca = theLevel.getClient(*(int *)a);
	GameClient	*cb = theLevel.getClient(*(int *)b);

	// sort special clients last
	if( ca->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_SCOREBOARD || ca->sess_.spectatorClient_ < 0 ) 
		return 1;

	if( cb->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_SCOREBOARD || cb->sess_.spectatorClient_ < 0  ) 
		return -1;

	// then connecting clients
	if( ca->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTING ) 
		return 1;

	if( cb->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTING ) 
		return -1;

	// then spectators
	if( ca->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR && 
		cb->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		if( ca->sess_.spectatorTime_ < cb->sess_.spectatorTime_ ) 
			return -1;

		if ( ca->sess_.spectatorTime_ > cb->sess_.spectatorTime_ )
			return 1;

		return 0;
	}
	if ( ca->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
		return 1;

	if ( cb->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
		return -1;

	// then sort by score
	if ( ca->ps_.persistant[PERS_SCORE]
		> cb->ps_.persistant[PERS_SCORE] ) 
		return -1;

	if ( ca->ps_.persistant[PERS_SCORE]
		< cb->ps_.persistant[PERS_SCORE] ) 
		return 1;

	return 0;
}

/*
============
CalculateRanks

Recalculates the score ranks of all players
This will be called on every client connect, begin, disconnect, death,
and team change.
============
*/
void CalculateRanks()
{
	theLevel.follow1_ = -1;
	theLevel.follow2_ = -1;
	theLevel.numConnectedClients_ = 0;
	theLevel.numNonSpectatorClients_ = 0;
	theLevel.numPlayingClients_ = 0;
	theLevel.numVotingClients_ = 0;		// don't count bots
	for( int i = 0; i < ClientBase::TEAM_NUM_TEAMS; i++ ) 
		theLevel.numteamVotingClients_[i] = 0;

	for( int i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		GameClient* client = theLevel.getClient(i);
		if( client && client->pers_.connected_ != GameClient::ClientPersistant::CON_DISCONNECTED ) 
		{
			theLevel.sortedClients_[theLevel.numConnectedClients_] = i;
			theLevel.numConnectedClients_++;

			if( client->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
			{
				theLevel.numNonSpectatorClients_++;
			
				// decide if this should be auto-followed
				if ( client->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED ) 
				{
					theLevel.numPlayingClients_++;
					if( !(theLevel.getEntity(i)->r.svFlags & SVF_BOT) ) 
					{
						theLevel.numVotingClients_++;
						if( client->sess_.sessionTeam_ == ClientBase::TEAM_RED )
							theLevel.numteamVotingClients_[0]++;
						else if( client->sess_.sessionTeam_ == ClientBase::TEAM_BLUE )
							theLevel.numteamVotingClients_[1]++;
					}
					if( theLevel.follow1_ == -1 ) 
						theLevel.follow1_ = i;
					else if( theLevel.follow2_ == -1 ) 
						theLevel.follow2_ = i;
				}
			}
		}
	}

	qsort( theLevel.sortedClients_, theLevel.numConnectedClients_, 
		sizeof(theLevel.sortedClients_[0]), SortRanks );

	// set the rank value for all clients that are connected and not spectators
	if( g_gametype.integer >= GT_TEAM ) 
	{
		// in team games, rank is just the order of the teams, 0=red, 1=blue, 2=tied
		for ( int i = 0;  i < theLevel.numConnectedClients_; i++ ) 
		{
			GameClient *cl =  theLevel.getClient(theLevel.sortedClients_[i]);
			if( theLevel.teamScores_[ClientBase::TEAM_RED] == theLevel.teamScores_[ClientBase::TEAM_BLUE] ) 
				cl->ps_.persistant[PERS_RANK] = 2;
			else if( theLevel.teamScores_[ClientBase::TEAM_RED] > theLevel.teamScores_[ClientBase::TEAM_BLUE] ) 
				cl->ps_.persistant[PERS_RANK] = 0;
			else
				cl->ps_.persistant[PERS_RANK] = 1;
		}
	} 
	else 
	{	
		int rank = -1;
		int score = 0;
		for( int i = 0;  i < theLevel.numPlayingClients_; i++ ) 
		{
			GameClient *cl = theLevel.getClient(theLevel.sortedClients_[i]);
			int newScore = cl->ps_.persistant[PERS_SCORE];
			if( i == 0 || newScore != score ) 
			{
				rank = i;
				// assume we aren't tied until the next client is checked
				theLevel.getClient(theLevel.sortedClients_[i])->ps_.persistant[PERS_RANK] = rank;
			} 
			else 
			{
				// we are tied with the previous client
				theLevel.getClient(theLevel.sortedClients_[i-1])->ps_.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
				theLevel.getClient(theLevel.sortedClients_[i])->ps_.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
			}
			score = newScore;
			if( g_gametype.integer == GT_SINGLE_PLAYER && theLevel.numPlayingClients_ == 1 ) 
				theLevel.getClient(theLevel.sortedClients_[i])->ps_.persistant[PERS_RANK] = rank | RANK_TIED_FLAG;
		}
	}

	// set the CS_SCORES1/2 configstrings, which will be visible to everyone
	if( g_gametype.integer >= GT_TEAM ) 
	{
		SV_SetConfigstring( CS_SCORES1, va("%i", theLevel.teamScores_[ClientBase::TEAM_RED] ) );
		SV_SetConfigstring( CS_SCORES2, va("%i", theLevel.teamScores_[ClientBase::TEAM_BLUE] ) );
	} 
	else
	{
		if( theLevel.numConnectedClients_ == 0 ) 
		{
			SV_SetConfigstring( CS_SCORES1, va("%i", SCORE_NOT_PRESENT) );
			SV_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		}
		else if( theLevel.numConnectedClients_ == 1 ) 
		{
			SV_SetConfigstring( CS_SCORES1, va("%i", theLevel.getClient(theLevel.sortedClients_[0])->ps_.persistant[PERS_SCORE] ) );
			SV_SetConfigstring( CS_SCORES2, va("%i", SCORE_NOT_PRESENT) );
		} 
		else
		{
			SV_SetConfigstring( CS_SCORES1, va("%i", theLevel.getClient(theLevel.sortedClients_[0])->ps_.persistant[PERS_SCORE] ) );
			SV_SetConfigstring( CS_SCORES2, va("%i", theLevel.getClient(theLevel.sortedClients_[1])->ps_.persistant[PERS_SCORE] ) );
		}
	}

	// see if it is time to end the level
	CheckExitRules();

	// if we are at the intermission, send the New info to everyone
	if( theLevel.intermissiontime_ ) 
		SendScoreboardMessageToAllClients();
}


/*
========================================================================

MAP CHANGING

========================================================================
*/

/*
========================
SendScoreboardMessageToAllClients

Do this at BeginIntermission time and whenever ranks are recalculated
due to enters/exits/forced team changes
========================
*/
void SendScoreboardMessageToAllClients() 
{
	int		i;

	for( i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		GameClient* client = theLevel.getClient(i);
		if( client && client->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED ) 
			DeathmatchScoreboardMessage( theLevel.getEntity(i) );
	}
}

/*
========================
MoveClientToIntermission

When the intermission starts, this will be called for all players.
If a New client connects, this will be called after the spawn function.
========================
*/
void MoveClientToIntermission( GameEntity *ent )
{
	// take out of follow mode if needed
	if ( ent->client_->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_FOLLOW ) 
		StopFollowing( ent );

	// move to the spot
	VectorCopy( theLevel.intermission_origin_, ent->s.origin );
	VectorCopy( theLevel.intermission_origin_, ent->client_->ps_.origin );
	VectorCopy( theLevel.intermission_angle_, ent->client_->ps_.viewangles);
	ent->client_->ps_.pm_type = PM_INTERMISSION;

	// clean up objective info
	ent->client_->ps_.objectives = 0;

	ent->client_->ps_.eFlags = 0;
	ent->s.eFlags = 0;
	ent->s.eType = ET_GENERAL;
	ent->s.modelindex = 0;
	ent->s.loopSound = 0;
	ent->s.event = 0;
	ent->r.contents = 0;
}

/*
==================
FindIntermissionPoint

This is also used for spectator spawns
==================
*/
void FindIntermissionPoint() 
{
	// find the intermission spot
	GameEntity *ent = G_Find (NULL, FOFS(classname_), "info_player_intermission");
	if ( !ent ) 
	{	
		// the map creator forgot to put in an intermission point...
		SelectSpawnPoint ( vec3_origin, theLevel.intermission_origin_, theLevel.intermission_angle_, -1 );
	} 
	else 
	{
		VectorCopy( ent->s.origin, theLevel.intermission_origin_ );
		VectorCopy( ent->s.angles, theLevel.intermission_angle_ );
		// if it has a target, look towards it
		if ( ent->target_ ) 
		{
			GameEntity *target = G_PickTarget( ent->target_ );
			if ( target ) 
			{
				vec3_t		dir;
				VectorSubtract( target->s.origin, theLevel.intermission_origin_, dir );
				vectoangles( dir, theLevel.intermission_angle_ );
			}
		}
	}
}

/*
==================
BeginIntermission
==================
*/
void BeginIntermission()
{
	if ( theLevel.intermissiontime_ ) 
		return;		// already active

	// if in tournement mode, change the wins / losses
	if ( g_gametype.integer == GT_TOURNAMENT ) 
		AdjustTournamentScores();

	theLevel.intermissiontime_ = theLevel.time_;
	FindIntermissionPoint();

	// if single player game
	if ( g_gametype.integer == GT_SINGLE_PLAYER ) 
		UpdateTournamentInfo();

	// move all clients to the intermission point
	for( int i=1 ; i<= theLevel.maxclients_ ; i++ ) 
	{
		GameEntity* client = theLevel.getEntity(i);
		if( !client || !client->inuse_ )
			continue;
		// respawn if dead
		if( client->health_ <= 0 ) 
			respawn(client);

		MoveClientToIntermission( client );
	}

	// send the current scoring to all clients
	SendScoreboardMessageToAllClients();

}


/*
=============
ExitLevel

When the intermission has been exited, the server is either killed
or moved to a New level based on the "nextmap" cvar 

=============
*/
void ExitLevel() 
{
	//bot interbreeding
//	BotInterbreedEndMatch();

	// if we are running a tournement map, kick the loser to spectator status,
	// which will automatically grab the next spectator and restart
	if ( g_gametype.integer == GT_TOURNAMENT  ) 
	{
		if ( !theLevel.restarted_ ) 
		{
			RemoveTournamentLoser();
			Cbuf_ExecuteText( EXEC_APPEND, "map_restart 0\n" );
			theLevel.restarted_ = true;
			theLevel.changemap_ = 0;
			theLevel.intermissiontime_ = 0;
		}
		return;	
	}

	Cbuf_ExecuteText( EXEC_APPEND, "vstr nextmap\n" );
	theLevel.changemap_ = 0;
	theLevel.intermissiontime_ = 0;

	// reset all the scores so we don't enter the intermission again
	theLevel.teamScores_[ClientBase::TEAM_RED] = 0;
	theLevel.teamScores_[ClientBase::TEAM_BLUE] = 0;
	for( int i=1 ; i<= g_maxclients.integer ; i++ )
	{
		GameClient *cl = theLevel.getClient(i);
		if ( !cl || cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
			continue;
		cl->ps_.persistant[PERS_SCORE] = 0;
	}

	// we need to do this here before chaning to CON_CONNECTING
	G_WriteSessionData();

	// change all client states to connecting, so the early players into the
	// next level will know the others aren't done reconnecting
	for( int i=1 ; i<= g_maxclients.integer ; i++ ) 
	{
		GameClient *cl = theLevel.getClient(i);
		if( cl->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED ) 
			cl->pers_.connected_ = GameClient::ClientPersistant::CON_CONNECTING;
	}

}

/*
=================
G_LogPrintf

Print to the logfile with a time stamp if it is open
=================
*/
void QDECL G_LogPrintf( const char *fmt, ... ) 
{
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;

	sec = theLevel.time_ / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );

	va_start( argptr, fmt );
	vsprintf( string +7 , fmt,argptr );
	va_end( argptr );

	if ( g_dedicated.integer )
		Com_Printf( "%s", string + 7 );

	if ( !theLevel.logFile_ ) 
		return;

	FS_Write( string, strlen( string ), theLevel.logFile_ );
}

/*
================
LogExit

Append information about this game to the log file
================
*/
void LogExit( const char *string )
{
//	bool won = true;

	G_LogPrintf( "Exit: %s\n", string );

	theLevel.intermissionQueued_ = theLevel.time_;

	// this will keep the clients from playing any voice sounds
	// that will get cut off when the queued intermission starts
	SV_SetConfigstring( CS_INTERMISSION, "1" );

	// don't send more than 32 scores (FIXME?)
	int numSorted = theLevel.numConnectedClients_;
	if( numSorted > 32 ) 
		numSorted = 32;

	if( g_gametype.integer >= GT_TEAM )
	{
		G_LogPrintf( "red:%i  blue:%i\n",
			theLevel.teamScores_[ClientBase::TEAM_RED], 
			theLevel.teamScores_[ClientBase::TEAM_BLUE] );
	}

	for( int i=0 ; i < numSorted ; i++ )
	{
		int		ping;

		GameClient *cl = theLevel.getClient(theLevel.sortedClients_[i]);

		if ( cl->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
			continue;

		if ( cl->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTING ) 
			continue;

		ping = cl->ps_.ping < 999 ? cl->ps_.ping : 999;

		G_LogPrintf( "score: %i  ping: %i  client: %i %s\n", cl->ps_.persistant[PERS_SCORE], ping, theLevel.sortedClients_[i],	cl->pers_.netname_ );
	}
}


/*
=================
CheckIntermissionExit

The level will stay at the intermission for a minimum of 5 seconds
If all players wish to continue, the level will then exit.
If one or more players have not acknowledged the continue, the game will
wait 10 seconds before going on.
=================
*/
void CheckIntermissionExit() 
{
	if ( g_gametype.integer == GT_SINGLE_PLAYER || g_gametype.integer == GT_MISSION_EDITOR ) 
		return;

	// see which players are ready
	int ready = 0;
	int notReady = 0;
	int readyMask = 0;
	for( int i=1 ; i<= g_maxclients.integer ; i++) 
	{
		GameClient *cl = theLevel.getClient(i);
		if ( !cl || cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
			continue;

		if ( theLevel.getEntity(cl->ps_.clientNum)->r.svFlags & SVF_BOT ) 
			continue;

		if ( cl->readyToExit_ ) 
		{
			ready++;
			if ( i < 16 ) 
				readyMask |= 1 << i;
		} 
		else 
		{
			notReady++;
		}
	}

	// copy the readyMask to each player's stats so
	// it can be displayed on the scoreboard
	for( i=1 ; i<= g_maxclients.integer ; i++ ) 
	{
		GameClient* cl = theLevel.getClient(i);
		if( !cl || cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
			continue;

		cl->ps_.stats[STAT_CLIENTS_READY] = readyMask;
	}

	// never exit in less than five seconds
	if ( theLevel.time_ < theLevel.intermissiontime_ + 5000 ) 
		return;

	// if nobody wants to go, clear timer
	if ( !ready ) 
	{
		theLevel.readyToExit_ = false;
		return;
	}

	// if everyone wants to go, go now
	if ( !notReady ) 
	{
		ExitLevel();
		return;
	}

	// the first person to ready starts the ten second timeout
	if ( !theLevel.readyToExit_ )
	{
		theLevel.readyToExit_ = true;
		theLevel.exitTime_ = theLevel.time_;
	}

	// if we have waited ten seconds since at least one player
	// wanted to exit, go ahead
	if ( theLevel.time_ < theLevel.exitTime_ + 10000 ) 
		return;

	ExitLevel();
}

/*
=============
ScoreIsTied
=============
*/
bool ScoreIsTied() 
{
	if( theLevel.numPlayingClients_ < 2 ) 
		return false;
	
	if ( g_gametype.integer >= GT_TEAM ) 
		return theLevel.teamScores_[ClientBase::TEAM_RED] == theLevel.teamScores_[ClientBase::TEAM_BLUE];

	int a = theLevel.getClient(theLevel.sortedClients_[0])->ps_.persistant[PERS_SCORE];
	int b = theLevel.getClient(theLevel.sortedClients_[1])->ps_.persistant[PERS_SCORE];

	return a == b;
}

/*
=================
CheckExitRules

There will be a delay between the time the exit is qualified for
and the time everyone is moved to the intermission spot, so you
can see the last frag.
=================
*/
void CheckExitRules() 
{
	// if at the intermission, wait for all non-bots to
	// signal ready, then go to next level
	if( theLevel.intermissiontime_ ) 
	{
		CheckIntermissionExit ();
		return;
	}

	if( theLevel.intermissionQueued_ ) 
	{
		if ( theLevel.time_ - theLevel.intermissionQueued_ >= INTERMISSION_DELAY_TIME ) 
		{
			theLevel.intermissionQueued_ = 0;
			BeginIntermission();
		}
		return;
	}

	// check for sudden death
	if( ScoreIsTied() )
	{
		// always wait for sudden death
		return;
	}

	if( g_timelimit.integer && !theLevel.warmupTime_ ) 
	{
		if( theLevel.time_ - theLevel.startTime_ >= g_timelimit.integer*60000 ) 
		{
			SV_GameSendServerCommand( -1, "print \"Timelimit hit.\n\" \"<scoreboard>\"");
			LogExit( "Timelimit hit." );
			return;
		}
	}

	if( theLevel.numPlayingClients_ < 2 ) 
		return;

	if( g_gametype.integer < GT_CTF && g_fraglimit.integer ) 
	{
		if( theLevel.teamScores_[ClientBase::TEAM_RED] >= g_fraglimit.integer ) 
		{
			SV_GameSendServerCommand( -1, "print \"Red hit the fraglimit.\n\" \"<scoreboard>\"" );
			LogExit( "Fraglimit hit." );
			return;
		}

		if( theLevel.teamScores_[ClientBase::TEAM_BLUE] >= g_fraglimit.integer ) 
		{
			SV_GameSendServerCommand( -1, "print \"Blue hit the fraglimit.\n\" \"<scoreboard>\"" );
			LogExit( "Fraglimit hit." );
			return;
		}

		for( int i=1 ; i<= g_maxclients.integer ; i++ ) 
		{
			GameClient *cl = theLevel.getClient(i);
			if( !cl || cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
				continue;

			if( cl->sess_.sessionTeam_ != ClientBase::TEAM_FREE ) 
				continue;

			if( cl->ps_.persistant[PERS_SCORE] >= g_fraglimit.integer )
			{
				LogExit( "Fraglimit hit." );
				SV_GameSendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " hit the fraglimit.\n\" \"<scoreboard>\"",
					cl->pers_.netname_ ) );
				return;
			}
		}
	}

	if( g_gametype.integer >= GT_CTF && g_capturelimit.integer )
	{

		if( theLevel.teamScores_[ClientBase::TEAM_RED] >= g_capturelimit.integer )
		{
			SV_GameSendServerCommand( -1, "print \"Red hit the capturelimit.\n\" \"<scoreboard>\"" );
			LogExit( "Capturelimit hit." );
			return;
		}

		if( theLevel.teamScores_[ClientBase::TEAM_BLUE] >= g_capturelimit.integer )
		{
			SV_GameSendServerCommand( -1, "print \"Blue hit the capturelimit.\n\" \"<scoreboard>\"" );
			LogExit( "Capturelimit hit." );
			return;
		}
	}
}



/*
========================================================================

FUNCTIONS CALLED EVERY FRAME

========================================================================
*/


/*
=============
CheckTournament

Once a frame, check for changes in tournement player state
=============
*/
void CheckTournament() 
{
	// check because we run 3 game frames before calling Connect and/or ClientBegin
	// for clients on a map_restart
	if( theLevel.numPlayingClients_ == 0 ) 
		return;

	if( g_gametype.integer == GT_TOURNAMENT ) 
	{
		// pull in a spectator if needed
		if( theLevel.numPlayingClients_ < 2 ) 
			AddTournamentPlayer();

		// if we don't have two players, go back to "waiting for players"
		if( theLevel.numPlayingClients_ != 2 )
		{
			if( theLevel.warmupTime_ != -1 ) 
			{
				theLevel.warmupTime_ = -1;
				SV_SetConfigstring( CS_WARMUP, va("%i", theLevel.warmupTime_) );
				G_LogPrintf( "Warmup:\n" );
			}
			return;
		}

		if( theLevel.warmupTime_ == 0 ) 
			return;

		// if the warmup is changed at the console, restart it
		if( g_warmup.modificationCount != theLevel.warmupModificationCount_ ) 
		{
			theLevel.warmupModificationCount_ = g_warmup.modificationCount;
			theLevel.warmupTime_ = -1;
		}

		// if all players have arrived, start the countdown
		if( theLevel.warmupTime_ < 0 ) 
		{
			if( theLevel.numPlayingClients_ == 2 ) 
			{
				// fudge by -1 to account for extra delays
				theLevel.warmupTime_ = theLevel.time_ + ( g_warmup.integer - 1 ) * 1000;
				SV_SetConfigstring( CS_WARMUP, va("%i", theLevel.warmupTime_) );
			}
			return;
		}

		// if the warmup time has counted down, restart
		if( theLevel.time_ > theLevel.warmupTime_ ) 
		{
			theLevel.warmupTime_ += 10000;
			Cvar_Set( "g_restarted", "1" );
			Cbuf_ExecuteText( EXEC_APPEND, "map_restart 0\n" );
			theLevel.restarted_ = true;
			return;
		}
	} 
	else if( g_gametype.integer != GT_SINGLE_PLAYER && g_gametype.integer != GT_MISSION_EDITOR &&
			 theLevel.warmupTime_ != 0 ) 
	{
		int	 counts[ClientBase::TEAM_NUM_TEAMS];
		bool notEnough = false;

		if( g_gametype.integer > GT_TEAM ) 
		{
			counts[ClientBase::TEAM_BLUE] = TeamCount( -1, ClientBase::TEAM_BLUE );
			counts[ClientBase::TEAM_RED] = TeamCount( -1, ClientBase::TEAM_RED );

			if (counts[ClientBase::TEAM_RED] < 1 || counts[ClientBase::TEAM_BLUE] < 1) 
				notEnough = true;
		}
		else if( theLevel.numPlayingClients_ < 2 ) 
			notEnough = true;

		if( notEnough ) 
		{
			if( theLevel.warmupTime_ != -1 ) 
			{
				theLevel.warmupTime_ = -1;
				SV_SetConfigstring( CS_WARMUP, va("%i", theLevel.warmupTime_) );
				G_LogPrintf( "Warmup:\n" );
			}
			return; // still waiting for team members
		}

		if( theLevel.warmupTime_ == 0 ) 
			return;

		// if the warmup is changed at the console, restart it
		if( g_warmup.modificationCount != theLevel.warmupModificationCount_ ) 
		{
			theLevel.warmupModificationCount_ = g_warmup.modificationCount;
			theLevel.warmupTime_ = -1;
		}

		// if all players have arrived, start the countdown
		if( theLevel.warmupTime_ < 0 ) 
		{
			// fudge by -1 to account for extra delays
			theLevel.warmupTime_ = theLevel.time_ + ( g_warmup.integer - 1 ) * 1000;
			SV_SetConfigstring( CS_WARMUP, va("%i", theLevel.warmupTime_) );
			return;
		}

		// if the warmup time has counted down, restart
		if( theLevel.time_ > theLevel.warmupTime_ ) 
		{
			theLevel.warmupTime_ += 10000;
			Cvar_Set( "g_restarted", "1" );
			Cbuf_ExecuteText( EXEC_APPEND, "map_restart 0\n" );
			theLevel.restarted_ = true;
			return;
		}
	}
}




/*
==================
CheckVote
==================
*/
void CheckVote() 
{
	if( theLevel.voteExecuteTime_ && theLevel.voteExecuteTime_ < theLevel.time_ ) 
	{
		theLevel.voteExecuteTime_ = 0;
		Cbuf_ExecuteText( EXEC_APPEND, va("%s\n", theLevel.voteString_ ) );
	}
	if( !theLevel.voteTime_ ) 
		return;

	if( theLevel.time_ - theLevel.voteTime_ >= VOTE_TIME ) 
	{
		SV_GameSendServerCommand( -1, "print \"Vote failed.\n\"" );
	}
	else 
	{
		if( theLevel.voteYes_ > theLevel.numVotingClients_/2 ) 
		{
			// execute the command, then remove the vote
			SV_GameSendServerCommand( -1, "print \"Vote passed.\n\"" );
			theLevel.voteExecuteTime_ = theLevel.time_ + 3000;
		}
		else if( theLevel.voteNo_ >= theLevel.numVotingClients_/2 ) 
		{
			// same behavior as a timeout
			SV_GameSendServerCommand( -1, "print \"Vote failed.\n\"" );
		}
		else 
		{
			// still waiting for a majority
			return;
		}
	}
	theLevel.voteTime_ = 0;
	SV_SetConfigstring( CS_VOTE_TIME, "" );

}

/*
==================
PrintTeam
==================
*/
void PrintTeam( int team, char *message ) 
{
	for( int i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		GameClient* cl = theLevel.getClient(i);
		if( !cl || cl->sess_.sessionTeam_ != team )
			continue;
		SV_GameSendServerCommand( i, message );
	}
}

/*
==================
SetLeader
==================
*/
void SetLeader( int team, int client )
{
	GameClient* cl = theLevel.getClient(client);

	if( !cl || cl->pers_.connected_ == GameClient::ClientPersistant::CON_DISCONNECTED ) 
	{
		PrintTeam(team, va("print \"%s is not connected\n\"", cl->pers_.netname_) );
		return;
	}
	if( cl->sess_.sessionTeam_ != team ) 
	{
		PrintTeam(team, va("print \"%s is not on the team anymore\n\"", cl->pers_.netname_) );
		return;
	}
	for( int i = 1 ; i <= theLevel.maxclients_ ; i++ )
	{
		GameClient* currCl = theLevel.getClient(i);
		if( !currCl || currCl->sess_.sessionTeam_ != team )
			continue;
		if( currCl->sess_.teamLeader_ )
		{
			currCl->sess_.teamLeader_ = false;
			ClientUserinfoChanged(i);
		}
	}
	cl->sess_.teamLeader_ = true;
	ClientUserinfoChanged( client );
	PrintTeam(team, va("print \"%s is the New team leader\n\"", cl->pers_.netname_) );
}

/*
==================
CheckTeamLeader
==================
*/
void CheckTeamLeader( int team ) 
{
	for( int i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		GameClient* cl = theLevel.getClient(i);
		if( cl->sess_.sessionTeam_ != team )
			continue;
		if( cl->sess_.teamLeader_ )
			break;
	}
	if( i > theLevel.maxclients_ ) 
	{
		for( i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
		{
			GameClient* cl = theLevel.getClient(i);
			if( !cl || cl->sess_.sessionTeam_ != team )
				continue;
			if( !(theLevel.getEntity(i)->r.svFlags & SVF_BOT) ) 
			{
				cl->sess_.teamLeader_ = true;
				break;
			}
		}
		for( i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
		{
			GameClient* cl = theLevel.getClient(i);
			if( !cl || cl->sess_.sessionTeam_ != team )
				continue;
			cl->sess_.teamLeader_ = true;
			break;
		}
	}
}

/*
==================
CheckTeamVote
==================
*/
void CheckTeamVote( int team ) 
{
	int cs_offset;

	if( team == ClientBase::TEAM_RED )
		cs_offset = 0;
	else if( team == ClientBase::TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if( !theLevel.teamVoteTime_[cs_offset] ) 
		return;

	if( theLevel.time_ - theLevel.teamVoteTime_[cs_offset] >= VOTE_TIME ) 
	{
		SV_GameSendServerCommand( -1, "print \"Team vote failed.\n\"" );
	}
	else
	{
		if( theLevel.teamVoteYes_[cs_offset] > theLevel.numteamVotingClients_[cs_offset]/2 ) 
		{
			// execute the command, then remove the vote
			SV_GameSendServerCommand( -1, "print \"Team vote passed.\n\"" );
			//
			if( !Q_strncmp( "leader", theLevel.teamVoteString_[cs_offset], 6) ) 
			{
				//set the team leader
				SetLeader(team, atoi(theLevel.teamVoteString_[cs_offset] + 7));
			}
			else
				Cbuf_ExecuteText( EXEC_APPEND, va("%s\n", theLevel.teamVoteString_[cs_offset] ) );
		}
		else if( theLevel.teamVoteNo_[cs_offset] >= theLevel.numteamVotingClients_[cs_offset]/2 ) 
		{
			// same behavior as a timeout
			SV_GameSendServerCommand( -1, "print \"Team vote failed.\n\"" );
		}
		else 
		{
			// still waiting for a majority
			return;
		}
	}
	theLevel.teamVoteTime_[cs_offset] = 0;
	SV_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, "" );
}


/*
==================
CheckCvars
==================
*/
void CheckCvars( void ) {
	static int lastMod = -1;

	if ( g_password.modificationCount != lastMod ) {
		lastMod = g_password.modificationCount;
		if ( *g_password.string && Q_stricmp( g_password.string, "none" ) ) {
			Cvar_Set( "g_needpass", "1" );
		} else {
			Cvar_Set( "g_needpass", "0" );
		}
	}
}

/*
=============
G_RunThink

Runs thinking code for this frame if necessary
=============
*/
void G_RunThink( GameEntity *ent )
{
	float	thinktime;

	thinktime = ent->nextthink_;
	if( thinktime <= 0 ) 
		return;

	if( thinktime > theLevel.time_ )
		return;
	
	ent->nextthink_ = 0;
	if( !ent->thinkFunc_ ) 
		Com_Error( ERR_DROP, "NULL ent->think");

	ent->thinkFunc_->execute();
}

/*
================
G_RunFrame

Advances the non-player objects in the world
================
*/
void G_RunFrame( int levelTime ) 
{
	// if we are waiting for the level to restart, do nothing
	if( theLevel.restarted_ )
		return;

	theLevel.framenum_++;
	theLevel.previousTime_ = theLevel.time_;
	theLevel.time_ = levelTime;
	int msec = theLevel.time_ - theLevel.previousTime_;

	// get any cvar changes
	G_UpdateCvars();

	//
	// go through all allocated objects
	//
	int start = Sys_Milliseconds();
	GameEntity* ent = 0;//&g_entities[0];
	for( int i=0 ; i<theLevel.num_entities_ ; i++ )
	{
		ent = theLevel.getEntity(i);
		if( !ent || !ent->inuse_ ) 
			continue;

		// clear events that are too old
		if( theLevel.time_ - ent->eventTime_ > EVENT_VALID_MSEC ) 
		{
			if( ent->s.event ) 
			{
				ent->s.event = 0;	// &= EV_EVENT_BITS;
				if( ent->client_ ) 
				{
					ent->client_->ps_.externalEvent = 0;
					// predicted events should never be set to zero
					//ent->client->ps.events[0] = 0;
					//ent->client->ps.events[1] = 0;
				}
			}
			if( ent->freeAfterEvent_ ) 
			{
				// tempEntities or dropped items completely go away after their event
				ent->freeUp();// former G_FreeEntity
				continue;
			} 
			else if( ent->unlinkAfterEvent_ ) 
			{
				// items that will respawn will hide themselves after their pickup event
				ent->unlinkAfterEvent_ = false;
				SV_UnlinkEntity( ent );
			}
		}
		// temporary entities don't think
		if( ent->freeAfterEvent_ ) 
			continue;
		if( !ent->r.linked ) //&& ent->neverFree_ ) 
			continue;
		if( ent->s.eType == ET_MISSILE ||
			 ent->s.eType == ET_BULLET ) 
		{
			G_RunMissile( ent );
			continue;
		}
		if( ent->s.eType == ET_ITEM || ent->physicsObject_ ) 
		{
			G_RunItem( ent );
			continue;
		}
		if( ent->s.eType == ET_MOVER )
		{
			G_RunMover( ent );
			continue;
		}
		if( ent->client_ )
		{
			G_RunClient( ent );
			continue;
		}
		G_RunThink( ent );
	}
	int end = Sys_Milliseconds();

	start = Sys_Milliseconds();
	// perform final fixups on the players
	//ent = &g_entities[0];
	for( i=1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		ent = theLevel.getEntity(i);
		if( ent && ent->inuse_ ) 
			ClientEndFrame( ent );
	}
	end = Sys_Milliseconds();

	// see if it is time to do a tournement restart
	CheckTournament();

	// see if it is time to end the level
	CheckExitRules();

	// update to team status?
	CheckTeamStatus();

	// cancel vote if timed out
	CheckVote();

	// check team votes
	CheckTeamVote( ClientBase::TEAM_RED );
	CheckTeamVote( ClientBase::TEAM_BLUE );

	// for tracking changes
	CheckCvars();

	if( g_listEntity.integer ) 
	{
		for( i = 0; i < MAX_GENTITIES; i++ )
		{
			ent = theLevel.getEntity(i);
			Com_Printf("%4i: %s\n", i, ent->classname_);
		}
		Cvar_Set("g_listEntity", "0");
	}
}
