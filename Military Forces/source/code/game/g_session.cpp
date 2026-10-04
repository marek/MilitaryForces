/*
 * $Id: g_session.c,v 1.5 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "g_client.h"
#include "g_level.h"

/*
=======================================================================

  SESSION DATA

Session data is the only data that stays persistant across level loads
and tournament restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData( GameClient* client ) 
{
	const char	*s;
	const char	*var;

	s = va("%i %i %i %i %i %i %i", 
		client->sess_.sessionTeam_,
		client->sess_.spectatorTime_,
		client->sess_.spectatorState_,
		client->sess_.spectatorClient_,
		client->sess_.wins_,
		client->sess_.losses_,
		client->sess_.teamLeader_
		);

	var = va( "session%i", client->ps_.clientNum );// client - level.clients );

	CvarSystem::set( var, s );
}

/*
================
Game::readSessionData

Called on a reconnect
================
*/
void Game::readSessionData( GameClient* client ) 
{
	char	s[MAX_STRING_CHARS];
	const char	*var;

	var = va( "session%i", client->ps_.clientNum );//client - level.clients );
	CvarSystem::variableStringBuffer( var, s, sizeof(s) );

	sscanf( s, "%i %i %i %i %i %i %i",
		&client->sess_.sessionTeam_,
		&client->sess_.spectatorTime_,
		&client->sess_.spectatorState_,
		&client->sess_.spectatorClient_,
		&client->sess_.wins_,
		&client->sess_.losses_,
		&client->sess_.teamLeader_
		);
}


/*
================
Game::initSessionData

Called on a first-time connect
================
*/
void Game::initSessionData( GameClient* client, char *userinfo ) 
{
	GameClient::ClientSession	*sess;
	const char					*value;

	sess = &client->sess_;

	// initial team determination
	if ( g_gametype.integer >= GT_TEAM ) 
	{
		if ( g_teamAutoJoin.integer ) 
		{
			sess->sessionTeam_ = PickTeam( -1 );
			BroadcastTeamChange( client, -1 );
		} 
		else 
		{
			// always spawn as spectator in team games
			sess->sessionTeam_ = ClientBase::TEAM_SPECTATOR;	
		}
	} 
	else 
	{
		value = Info::valueForKey( userinfo, "team" );
		if ( value[0] == 's' ) 
		{
			// a willing spectator, not a waiting-in-line
			sess->sessionTeam_ = ClientBase::TEAM_SPECTATOR;
		}
		else 
		{
			switch ( g_gametype.integer ) 
			{
			default:
			case GT_FFA:
			case GT_SINGLE_PLAYER:
				if ( g_maxGameClients.integer > 0 && 
					theLevel.numNonSpectatorClients_ >= g_maxGameClients.integer )
				{
					sess->sessionTeam_ = ClientBase::TEAM_SPECTATOR;
				} 
				else
				{
					sess->sessionTeam_ = ClientBase::TEAM_FREE;
				}
				break;
			case GT_MISSION_EDITOR:
				sess->sessionTeam_ = ClientBase::TEAM_SPECTATOR;
				break;
			case GT_TOURNAMENT:
				// if the game is full, go into a waiting mode
				if ( theLevel.numNonSpectatorClients_ >= 2 )
				{
					sess->sessionTeam_ = ClientBase::TEAM_SPECTATOR;
				} 
				else 
				{
					sess->sessionTeam_ = ClientBase::TEAM_FREE;
				}
				break;
			}
		}
	}

	sess->spectatorState_ = GameClient::ClientSession::SPECTATOR_FREE;
	sess->spectatorTime_ = theLevel.time_;

	G_WriteClientSessionData( client );
}


/*
==================
Game::initWorldSession

==================
*/
void Game::initWorldSession()
{
	char	s[MAX_STRING_CHARS];
	int			gt;

	CvarSystem::variableStringBuffer( "session", s, sizeof(s) );
	gt = atoi( s );
	
	// if the gametype changed since the last session, don't use any
	// client sessions
	if ( g_gametype.integer != gt) 
	{
		theLevel.newSession_ = true;
		Common::printf( "Gametype changed, clearing session data.\n" );
	} 
}

/*
==================
Game::writeSessionData

==================
*/
void Game::writeSessionData() 
{
	CvarSystem::set( "session", va("%i", g_gametype.integer) );

	for( int i = 1 ; i <= theLevel.maxclients_ ; i++ )
	{
		GameClient* cl = theLevel.getClient(i);
		if( cl && cl->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED ) 
		{
			G_WriteClientSessionData( cl );
		}
	}
}
