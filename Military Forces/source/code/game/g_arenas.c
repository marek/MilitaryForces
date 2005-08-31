/*
 * $Id: g_arenas.c,v 1.2 2005-08-31 19:20:06 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// g_arenas.c
//

#include "g_local.h"


/*
==================
UpdateTournamentInfo
==================
*/
void UpdateTournamentInfo( void ) {
	int			i;
	gentity_t	*player;
	int			playerClientNum;
	int			n, accuracy, perfect,	msglen;
	int			buflen;//, score1, score2;
//	bool won;
	char		buf[32];
	char		msg[MAX_STRING_CHARS];

	// find the real player
	player = NULL;
	for (i = 0; i < level.maxclients; i++ ) {
		player = &g_entities[i];
		if ( !player->inuse ) {
			continue;
		}
		if ( !( player->r.svFlags & SVF_BOT ) ) {
			break;
		}
	}
	// this should never happen!
	if ( !player || i == level.maxclients ) {
		return;
	}
	playerClientNum = i;

	CalculateRanks();

	if ( level.clients[playerClientNum].sess.sessionTeam == TEAM_SPECTATOR ) {
		Com_sprintf( msg, sizeof(msg), "postgame %i %i 0 0 0 0 0 0", level.numNonSpectatorClients, playerClientNum );
	}
	else {
		if( player->client->accuracy_shots ) {
			accuracy = player->client->accuracy_hits * 100 / player->client->accuracy_shots;
		}
		else {
			accuracy = 0;
		}
		perfect = ( level.clients[playerClientNum].ps.persistant[PERS_RANK] == 0 && player->client->ps.persistant[PERS_KILLED] == 0 ) ? 1 : 0;
		Com_sprintf( msg, sizeof(msg), "postgame %i %i %i %i %i %i %i", level.numNonSpectatorClients, playerClientNum, accuracy,
			player->client->ps.persistant[PERS_IMPRESSIVE_COUNT], player->client->ps.persistant[PERS_EXCELLENT_COUNT],
			player->client->ps.persistant[PERS_SCORE], perfect );
	}

	msglen = strlen( msg );
	for( i = 0; i < level.numNonSpectatorClients; i++ ) {
		n = level.sortedClients[i];
		Com_sprintf( buf, sizeof(buf), " %i %i %i", n, level.clients[n].ps.persistant[PERS_RANK], level.clients[n].ps.persistant[PERS_SCORE] );
		buflen = strlen( buf );
		if( msglen + buflen + 1 >= sizeof(msg) ) {
			break;
		}
		strcat( msg, buf );
	}
	trap_SendConsoleCommand( EXEC_APPEND, msg );
}


