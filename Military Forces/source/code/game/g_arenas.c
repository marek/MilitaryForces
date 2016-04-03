/*
 * $Id: g_arenas.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// g_arenas.c
//

#include "g_local.h"
#include "g_level.h"



/*
==================
UpdateTournamentInfo
==================
*/
void UpdateTournamentInfo() 
{
	int			i;
	GameEntity* player = 0;
	int			playerClientNum;
	int			n, accuracy, perfect,	msglen;
	int			buflen;//, score1, score2;
//	bool won;
	char		buf[32];
	char		msg[MAX_STRING_CHARS];

	// find the real player
	for (i = 1; i <= theLevel.maxclients_; i++ ) 
	{
		player = theLevel.getEntity(i);// g_entities[i];
		if ( !player || !player->inuse_ ) 
			continue;
		
		if ( !( player->r.svFlags & SVF_BOT ) ) 
			break;
	}
	// this should never happen!
	if ( !player || i > theLevel.maxclients_ ) 
		return;

	playerClientNum = i;

	CalculateRanks();

	if ( theLevel.getClient(playerClientNum)->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		Com_sprintf( msg, sizeof(msg), "postgame %i %i 0 0 0 0 0 0", theLevel.numNonSpectatorClients_, playerClientNum );
	}
	else 
	{
		if( player->client_->accuracy_shots_ ) 
			accuracy = player->client_->accuracy_hits_ * 100 / player->client_->accuracy_shots_;
		else 
			accuracy = 0;

		perfect = ( theLevel.getClient(playerClientNum)->ps_.persistant[PERS_RANK] == 0 && player->client_->ps_.persistant[PERS_KILLED] == 0 ) ? 1 : 0;
		Com_sprintf( msg, sizeof(msg), "postgame %i %i %i %i %i %i %i", theLevel.numNonSpectatorClients_, playerClientNum, accuracy,
			0, 0,
			player->client_->ps_.persistant[PERS_SCORE], perfect );
	}

	msglen = strlen( msg );
	for( i = 0; i < theLevel.numNonSpectatorClients_; i++ )
	{
		n = theLevel.sortedClients_[i];
		Com_sprintf( buf, sizeof(buf), " %i %i %i", n, theLevel.getClient(n)->ps_.persistant[PERS_RANK], 
							theLevel.getClient(n)->ps_.persistant[PERS_SCORE] );
		buflen = strlen( buf );
		if( msglen + buflen + 1 >= sizeof(msg) ) 
			break;

		strcat( msg, buf );
	}
	Cbuf_ExecuteText( EXEC_APPEND, msg );
}


