/*
 * $Id: cg_servercmds.c,v 1.6 2006-09-21 01:18:59 minkis Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"

struct orderTask_t {
	const char *order;
	int taskNum;
};

/*
=================
ClientGame::parseScores

=================
*/

constexpr int SCORE_BLOCK = 15; // this value must be exactly how many parameters we expect per client score

void ClientGame::parseScores( )
{
	int	i, objectives;

	cg.numScores = atoi( ClientGame::argv( 1 ) );
	if ( cg.numScores > MAX_CLIENTS )
	{
		cg.numScores = MAX_CLIENTS;
	}

	cg.teamScores[0] = atoi( ClientGame::argv( 2 ) );
	cg.teamScores[1] = atoi( ClientGame::argv( 3 ) );

	memset( cg.scores, 0, sizeof( cg.scores ) );
	for ( i = 0 ; i < cg.numScores ; i++ )
	{
		//
		cg.scores[i].client = atoi( ClientGame::argv( i * SCORE_BLOCK + 4 ) );
		cg.scores[i].score = atoi( ClientGame::argv( i * SCORE_BLOCK + 5 ) );
		cg.scores[i].ping = atoi( ClientGame::argv( i * SCORE_BLOCK + 6 ) );
		cg.scores[i].time = atoi( ClientGame::argv( i * SCORE_BLOCK + 7 ) );
		cg.scores[i].scoreFlags = atoi( ClientGame::argv( i * SCORE_BLOCK + 8 ) );
		objectives = atoi( ClientGame::argv( i * SCORE_BLOCK + 9 ) );
		cg.scores[i].accuracy = atoi(ClientGame::argv( i * SCORE_BLOCK + 10));
		cg.scores[i].impressiveCount = atoi(ClientGame::argv( i * SCORE_BLOCK + 11));
		cg.scores[i].excellentCount = atoi(ClientGame::argv (i * SCORE_BLOCK + 12));
		cg.scores[i].guantletCount = atoi(ClientGame::argv( i * SCORE_BLOCK + 13));
		cg.scores[i].defendCount = atoi(ClientGame::argv( i * SCORE_BLOCK + 14));
		cg.scores[i].assistCount = atoi(ClientGame::argv( i * 14 + 15));
		cg.scores[i].perfect = atoi(ClientGame::argv( i * SCORE_BLOCK + 16));
		cg.scores[i].captures = atoi(ClientGame::argv(i * SCORE_BLOCK + 17));
		cg.scores[i].deaths = atoi( ClientGame::argv( i * SCORE_BLOCK + 18 ) );

		if ( cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS )
		{
			cg.scores[i].client = 0;
		}
		cgs.clientinfo[ cg.scores[i].client ].score = cg.scores[i].score;
		cgs.clientinfo[ cg.scores[i].client ].objectives = objectives;
		cgs.clientinfo[ cg.scores[i].client ].deaths = cg.scores[i].deaths;

		cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;
	}
}

/*
=================
ClientGame::parseTeamInfo

=================
*/
void ClientGame::parseTeamInfo( ) {
	int		i;
	int		client;

	numSortedTeamPlayers = atoi( ClientGame::argv( 1 ) );
	if( numSortedTeamPlayers < 0 || numSortedTeamPlayers > TEAM_MAXOVERLAY )
	{
		Common::error( ERR_DROP, "CG_ParseTeamInfo: numSortedTeamPlayers out of range (%d)",
				numSortedTeamPlayers );
		return;
	}

	for ( i = 0 ; i < numSortedTeamPlayers ; i++ ) {
		client = atoi( ClientGame::argv( i * 6 + 2 ) );
		if( client < 0 || client >= MAX_CLIENTS )
		{
		  Common::error( ERR_DROP, "CG_ParseTeamInfo: bad client number: %d", client );
		  return;
		}

		sortedTeamPlayers[i] = client;

		cgs.clientinfo[ client ].location = atoi( ClientGame::argv( i * 6 + 3 ) );
		cgs.clientinfo[ client ].health = atoi( ClientGame::argv( i * 6 + 4 ) );
		cgs.clientinfo[ client ].curWeapon = atoi( ClientGame::argv( i * 6 + 5 ) );
		cgs.clientinfo[ client ].objectives = atoi( ClientGame::argv( i * 6 + 6 ) );
	}
}


/*
================
ClientGame::parseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/
void ClientGame::parseServerinfo( ) {
	const char	*info;
	char	*mapname;
	char	gs[32];
	unsigned int newset = cgs.gameset;

	info = ClientGame::configString( CS_SERVERINFO );
	cgs.gametype = static_cast<gametype_t>(atoi( Info::valueForKey( info, "g_gametype" ) ));
	CvarSystem::set("g_gametype", va("%i", cgs.gametype));
	cgs.dmflags = atoi( Info::valueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info::valueForKey( info, "teamflags" ) );
	cgs.fraglimit = atoi( Info::valueForKey( info, "fraglimit" ) );
	cgs.capturelimit = atoi( Info::valueForKey( info, "capturelimit" ) );
	cgs.timelimit = atoi( Info::valueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info::valueForKey( info, "sv_maxclients" ) );
	mapname = Info::valueForKey( info, "mapname" );
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );
	Q_strncpyz( cgs.redTeam, Info::valueForKey( info, "g_redTeam" ), sizeof(cgs.redTeam) );
	CvarSystem::set("g_redTeam", cgs.redTeam);
	Q_strncpyz( cgs.blueTeam, Info::valueForKey( info, "g_blueTeam" ), sizeof(cgs.blueTeam) );
	CvarSystem::set("g_blueTeam", cgs.blueTeam);
	
	// mfq3
	Q_strncpyz( gs, Info::valueForKey( info, "mf_gameset" ), sizeof(gs) );
	if( strcmp( gs, "modern" ) == 0 ) newset = MF_GAMESET_MODERN;
	else if( strcmp( gs, "ww2" ) == 0 ) newset = MF_GAMESET_WW2;
	else if( strcmp( gs, "ww1" ) == 0 ) newset = MF_GAMESET_WW1;
	else newset = MF_GAMESET_MODERN;
	Common::printf("The gameset is '%s'\n", gs);

	// gameset changed>
	if( cgs.gameset && newset != cgs.gameset )
	{	
		CommandBuffer::addText(";vid_restart\n" );  // Leading ' ; ' Previous command getting stuck?
		Common::printf("The gameset has changed to %s\n", gs );
	}
	cgs.gameset = newset;
}

/*
==================
ClientGame::parseWarmup
==================
*/
void ClientGame::parseWarmup( ) {
	const char	*info;
	int			warmup;

	info = ClientGame::configString( CS_WARMUP );

	warmup = atoi( info );
	cg.warmupCount = -1;

	if ( warmup == 0 && cg.warmup ) {

	} else if ( warmup > 0 && cg.warmup <= 0 && cgs.gametype != GT_MISSION_EDITOR ) {
		{
			SoundSystem::startLocalSound( cgs.media.countPrepareSound, CHAN_ANNOUNCER );
		}
	}

	cg.warmup = warmup;
}

/*
================
ClientGame::setConfigValues

Called on load to set the initial values from configure strings
================
*/
void ClientGame::setConfigValues( ) {
	const char *s;

	cgs.scores1 = atoi( ClientGame::configString( CS_SCORES1 ) );
	cgs.scores2 = atoi( ClientGame::configString( CS_SCORES2 ) );
	cgs.levelStartTime = atoi( ClientGame::configString( CS_LEVEL_START_TIME ) );
	if( cgs.gametype == GT_CTF ) {
		s = ClientGame::configString( CS_FLAGSTATUS );
		cgs.redflag = s[0] - '0';
		cgs.blueflag = s[1] - '0';
	}
	cg.warmup = atoi( ClientGame::configString( CS_WARMUP ) );
}

/*
=====================
ClientGame::shaderStateChanged
=====================
*/
void ClientGame::shaderStateChanged() {
	char originalShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	char timeOffset[16];
	const char *o;
	char *n,*t;
	int length;

	o = ClientGame::configString( CS_SHADERSTATE );
	while (o && *o) {
		n = const_cast<char *>(strstr(o, "="));
		if (n && *n) {
			length = n-o+1;
			if (length > sizeof(originalShader)) {
				length = sizeof(originalShader);
			}
			Q_strncpyz(originalShader, o, length);
			n++;
			t = strstr(n, ":");
			if (t && *t) {
				length = t-n+1;
				if (length > sizeof(newShader)) {
					length = sizeof(newShader);
				}
				Q_strncpyz(newShader, n, length);
			} else {
				break;
			}
			t++;
			o = strstr(t, "@");
			if (o) {
				length = o-t+1;
				if (length > sizeof(timeOffset)) {
					length = sizeof(timeOffset);
				}
				Q_strncpyz(timeOffset, t, length);
				o++;
				refExport.RemapShader( originalShader, newShader, timeOffset );
			}
		} else {
			break;
		}
	}
}

/*
================
ClientGame::configStringModified

================
*/
void ClientGame::configStringModified( ) {
	const char	*str;
	int		num;

	num = atoi( ClientGame::argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// New configstring already integrated
	CL_GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = ClientGame::configString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC ) {
		ClientGame::startMusic();
	} else if ( num == CS_SERVERINFO ) {
		ClientGame::parseServerinfo();
	} else if ( num == CS_WARMUP ) {
		ClientGame::parseWarmup();
	} else if ( num == CS_SCORES1 ) {
		cgs.scores1 = atoi( str );
	} else if ( num == CS_SCORES2 ) {
		cgs.scores2 = atoi( str );
	} else if ( num == CS_LEVEL_START_TIME ) {
		cgs.levelStartTime = atoi( str );
	} else if ( num == CS_VOTE_TIME ) {
		cgs.voteTime = atoi( str );
		cgs.voteModified = true;
	} else if ( num == CS_VOTE_YES ) {
		cgs.voteYes = atoi( str );
		cgs.voteModified = true;
	} else if ( num == CS_VOTE_NO ) {
		cgs.voteNo = atoi( str );
		cgs.voteModified = true;
	} else if ( num == CS_VOTE_STRING ) {
		Q_strncpyz( cgs.voteString, str, sizeof( cgs.voteString ) );
	} else if ( num >= CS_TEAMVOTE_TIME && num <= CS_TEAMVOTE_TIME + 1) {
		cgs.teamVoteTime[num-CS_TEAMVOTE_TIME] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_TIME] = true;
	} else if ( num >= CS_TEAMVOTE_YES && num <= CS_TEAMVOTE_YES + 1) {
		cgs.teamVoteYes[num-CS_TEAMVOTE_YES] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_YES] = true;
	} else if ( num >= CS_TEAMVOTE_NO && num <= CS_TEAMVOTE_NO + 1) {
		cgs.teamVoteNo[num-CS_TEAMVOTE_NO] = atoi( str );
		cgs.teamVoteModified[num-CS_TEAMVOTE_NO] = true;
	} else if ( num >= CS_TEAMVOTE_STRING && num <= CS_TEAMVOTE_STRING + 1) {
		Q_strncpyz( cgs.teamVoteString[num-CS_TEAMVOTE_STRING], str, sizeof( cgs.teamVoteString[0] ) );
	} else if ( num == CS_INTERMISSION ) {
		cg.intermissionStarted = atoi( str );
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.gameModels[ num-CS_MODELS ] = refExport.RegisterModel( str );
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS ) {
		if ( str[0] != '*' ) {	// player specific sounds don't register here
			cgs.gameSounds[ num-CS_SOUNDS] = SoundSystem::registerSound( str, false );
		}
	} else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS ) {
		ClientGame::newClientInfo( num - CS_PLAYERS );
		ClientGame::buildSpectatorString();
	} else if ( num == CS_FLAGSTATUS ) {
		if( cgs.gametype == GT_CTF ) {
			// format is rb where its red/blue, 0 is at base, 1 is taken, 2 is dropped
			cgs.redflag = str[0] - '0';
			cgs.blueflag = str[1] - '0';
		}
	}
	else if ( num == CS_SHADERSTATE ) {
		ClientGame::shaderStateChanged();
	}
		
}


/*
=======================
ClientGame::addToTeamChat

=======================
*/
void ClientGame::addToTeamChat( const char *str ) {
	int len;
	char *p, *ls;
	int lastcolor;
	int chatHeight;

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT) {
		chatHeight = cg_teamChatHeight.integer;
	} else {
		chatHeight = TEAMCHAT_HEIGHT;
	}

	if (chatHeight <= 0 || cg_teamChatTime.integer <= 0) {
		// team chat disabled, dump into normal chat
		cgs.teamChatPos = cgs.teamLastChatPos = 0;
		return;
	}

	len = 0;

	p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
	*p = 0;

	lastcolor = '7';

	ls = nullptr;
	while (*str) {
		if (len > TEAMCHAT_WIDTH - 1) {
			if (ls) {
				str -= (p - ls);
				str++;
				p -= (p - ls);
			}
			*p = 0;

			cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;

			cgs.teamChatPos++;
			p = cgs.teamChatMsgs[cgs.teamChatPos % chatHeight];
			*p = 0;
			*p++ = Q_COLOR_ESCAPE;
			*p++ = lastcolor;
			len = 0;
			ls = nullptr;
		}

		if ( Q_IsColorString( str ) ) {
			*p++ = *str++;
			lastcolor = *str;
			*p++ = *str++;
			continue;
		}
		if (*str == ' ') {
			ls = p;
		}
		*p++ = *str++;
		len++;
	}
	*p = 0;

	cgs.teamChatMsgTimes[cgs.teamChatPos % chatHeight] = cg.time;
	cgs.teamChatPos++;

	if (cgs.teamChatPos - cgs.teamLastChatPos > chatHeight)
		cgs.teamLastChatPos = cgs.teamChatPos - chatHeight;
}



/*
===============
ClientGame::mapRestart

The server has issued a map_restart, so the next snapshot
is completely New and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
void ClientGame::mapRestart( ) {

	if ( cg_showmiss.integer ) {
		ClientGame::printf( "CG_MapRestart\n" );
	}

	ClientGame::initLocalEntities();
	ClientGame::initMarkPolys();

	// make sure the "3 frags left" warnings play again
	cg.fraglimitWarnings = 0;

	cg.timelimitWarnings = 0;
	cg.rewardTime = 0;
	cg.rewardStack = 0;

	cg.intermissionStarted = false;

	cgs.voteTime = 0;

	cg.mapRestart = true;

	ClientGame::startMusic();

	SoundSystem::clearLoopingSounds(true);

	// MFQ3 vehicle needs to be reset
	if( cg_vehicle.integer != -1 ) {
		CvarSystem::set( "cg_vehicle", "-1" );
	}
	if( cg_nextVehicle.integer != -1 ) {
		CvarSystem::set( "cg_nextVehicle", "-1" );
	}

	// we really should clear more parts of cg here and stop sounds

	// play the "fight" sound if this is a restart without warmup
	if ( cg.warmup == 0 && cgs.gametype != GT_MISSION_EDITOR /* && cgs.gametype == GT_TOURNAMENT */) {
		SoundSystem::startLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
		ClientGame::centerPrint( "FIGHT!", 120, GIANTCHAR_WIDTH*2 );
	} else if ( cgs.gametype == GT_MISSION_EDITOR ) {
		ClientGame::centerPrint( "EDIT!", 120, GIANTCHAR_WIDTH*2 );
	}
}

/*
=================
ClientGame::removeChatEscapeChar
=================
*/
void ClientGame::removeChatEscapeChar( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (text[i] == '\x19')
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

/*
=================
ClientGame::printExtractExtra
=================
*/
void ClientGame::printExtractExtra( )
{
	const char	*extraCmd = ClientGame::argv(2);

	// intercepts 'print' commands and forwards the text onto other parts if need be

	// any second parameter?
	if( strlen( extraCmd ) > 0 )
	{
		// compare with known extra-commands
		if( strcmp( extraCmd, "<scoreboard>" ) == 0 )
		{
			// copy the first parameter into the information chars
			strcpy( cg.scoreboardMisc, ClientGame::argv(1) );
		}
	}
}

/*
=================
ClientGame::serverCommand

The string has been tokenized and can be retrieved with
CommandSystem::argc() / CommandSystem::argv()
=================
*/
void ClientGame::serverCommand( ) {
	const char	*cmd;
	char		text[MAX_SAY_TEXT];

	cmd = ClientGame::argv(0);

	if ( !cmd[0] ) {
		// server claimed the command
		return;
	}

	if ( !strcmp( cmd, "cp" ) ) {
		ClientGame::centerPrint( ClientGame::argv(1), SCREEN_HEIGHT * 0.30, BIGCHAR_WIDTH );
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		ClientGame::configStringModified();
		return;
	}

	if ( !strcmp( cmd, "print" ) ) {
		ClientGame::printExtractExtra( );
		ClientGame::printf( "%s", ClientGame::argv(1) );
		return;
	}

	if ( !strcmp( cmd, "chat" ) ) {
		if ( !cg_teamChatsOnly.integer ) {
			SoundSystem::startLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
			Q_strncpyz( text, ClientGame::argv(1), MAX_SAY_TEXT );
			ClientGame::removeChatEscapeChar( text );
			ClientGame::printf( "%s\n", text );
		}
		return;
	}

	if ( !strcmp( cmd, "tchat" ) ) {
		SoundSystem::startLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		Q_strncpyz( text, ClientGame::argv(1), MAX_SAY_TEXT );
		ClientGame::removeChatEscapeChar( text );
		ClientGame::addToTeamChat( text );
		ClientGame::printf( "%s\n", text );
		return;
	}

	if ( !strcmp( cmd, "scores" ) ) {
		ClientGame::parseScores();
		return;
	}

	if ( !strcmp( cmd, "tinfo" ) ) {
		ClientGame::parseTeamInfo();
		return;
	}

	if ( !strcmp( cmd, "map_restart" ) ) {
		ClientGame::mapRestart();
		return;
	}

  if ( Q_stricmp (cmd, "remapShader") == 0 ) {
		if (CommandSystem::argc() == 4) {
			refExport.RemapShader(ClientGame::argv(1), ClientGame::argv(2), ClientGame::argv(3));
		}
	}

	// clientLevelShot is sent before taking a special screenshot for
	// the menu system during development
	if ( !strcmp( cmd, "clientLevelShot" ) ) {
		cg.levelShot = true;
		return;
	}

	if( !strcmp( cmd, "me_spawnvehicle" ) ) {
		if( CommandSystem::argc() == 2 ) {
			int idx = atoi(ClientGame::argv(1));
			ME_SpawnVehicle(idx);
		}
		return;
	}

	if( !strcmp( cmd, "me_spawnvehiclegi" ) ) {
		if( CommandSystem::argc() == 2 ) {
			int idx = atoi(ClientGame::argv(1));
			ME_SpawnGroundInstallation(idx);
		}
		return;
	}

	ClientGame::printf( "Unknown client game command: %s\n", cmd );
}


/*
====================
ClientGame::executeNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void ClientGame::executeNewServerCommands( int latestSequence ) 
{
	while ( cgs.serverCommandSequence < latestSequence ) 
	{
		if ( CL_GetServerCommand( ++cgs.serverCommandSequence ) ) 
			ClientGame::serverCommand();
	}
}
