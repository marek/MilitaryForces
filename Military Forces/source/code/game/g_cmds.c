/*
 * $Id: g_cmds.c,v 1.8 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "g_level.h"
#include "g_mfq3ents.h"

/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage( GameEntity *ent ) 
{
	char		entry[1024];
	char		string[1400];
	int			stringlength;
	int			i, j;
	GameClient	*cl;
	int			numSorted, scoreFlags, accuracy, perfect;

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;
	scoreFlags = 0;

	numSorted = theLevel.numConnectedClients_;
	
	for( i=0 ; i < numSorted ; i++ )
	{
		int		ping;

		cl = theLevel.getClient(theLevel.sortedClients_[i]);// &level.clients[level.sortedClients[i]];

		if( cl->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTING ) 
			ping = -1;
		else 
			ping = cl->ps_.ping < 999 ? cl->ps_.ping : 999;

		if( cl->accuracy_shots_ ) 
			accuracy = cl->accuracy_hits_ * 100 / cl->accuracy_shots_;
		else 
			accuracy = 0;

		perfect = ( cl->ps_.persistant[PERS_RANK] == 0 && cl->ps_.persistant[PERS_KILLED] == 0 ) ? 1 : 0;

		// SCORE_BLOCK = 15 parameters (make sure this ties with the value used in CG_ParseScores())
		Com_sprintf( entry, sizeof(entry),
			" %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i",
			theLevel.sortedClients_[i],
			cl->ps_.persistant[PERS_SCORE],
			ping,
			(theLevel.time_ - cl->pers_.enterTime_)/60000,
			scoreFlags,
			theLevel.getEntity(theLevel.sortedClients_[i])->s.objectives,// g_entities[theLevel.sortedClients_[i]].s.objectives,
			accuracy, 
			0,
			0,
			0,
			0, 
			0, 
			perfect,
			cl->ps_.persistant[PERS_CAPTURES],
			cl->ps_.persistant[PERS_DEATHS] );

		j = strlen(entry);
		if( stringlength + j > 1024 )
			break;
		strcpy( string + stringlength, entry );
		stringlength += j;
	}

	SV_GameSendServerCommand( ent->s.number,// ok ? before: ent-g_entities, 
							  va("scores %i %i %i%s", 
							  i, 
							  theLevel.teamScores_[ClientBase::TEAM_RED], 
							  theLevel.teamScores_[ClientBase::TEAM_BLUE],
							  string ) );
}


/*
==================
Cmd_Score_f

Request current scoreboard information
==================
*/
void Cmd_Score_f( GameEntity *ent ) 
{
	DeathmatchScoreboardMessage( ent );
}



/*
==================
CheatsOk
==================
*/
bool CheatsOk( GameEntity *ent ) 
{
	if ( !g_cheats.integer ) 
	{
		SV_GameSendServerCommand( ent->s.number,//ent-g_entities, 
								  va("print \"Cheats are not enabled on this server.\n\""));
		return false;
	}
	if ( ent->health_ <= 0 ) 
	{
		SV_GameSendServerCommand( ent->s.number,//ent-g_entities, 
								  va("print \"You must be alive to use this command.\n\""));
		return false;
	}
	// ensiform fix
	if ( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		SV_GameSendServerCommand( ent->s.number,//ent-g_entities, 
								  va("print \"Spectators can't use this command.\n\""));
		return false;
	}
	return true;
}


/*
==================
ConcatArgs
==================
*/
char	*ConcatArgs( int start ) 
{
	int		i, c, tlen;
	static char	line[MAX_STRING_CHARS];
	int		len;
	char	arg[MAX_STRING_CHARS];

	len = 0;
	c = Cmd_Argc();
	for ( i = start ; i < c ; i++ ) 
	{
		Cmd_ArgvBuffer( i, arg, sizeof( arg ) );
		tlen = strlen( arg );
		if ( len + tlen >= MAX_STRING_CHARS - 1 ) 
			break;

		memcpy( line + len, arg, tlen );
		len += tlen;
		if ( i != c - 1 ) 
		{
			line[len] = ' ';
			len++;
		}
	}

	line[len] = 0;

	return line;
}

/*
==================
SanitizeString

Remove case and control characters
==================
*/
void SanitizeString( char *in, char *out ) 
{
	while ( *in )
	{
		if ( *in == 27 )
		{
			in += 2;		// skip color code
			continue;
		}
		if ( *in < 32 )
		{
			in++;
			continue;
		}
		*out++ = tolower( *in++ );
	}

	*out = 0;
}

/*
==================
ClientNumberFromString

Returns a player number for either a number or name string
Returns -1 if invalid
==================
*/
int ClientNumberFromString( GameEntity *to, char *s ) 
{
	GameClient	*cl;
	int			idnum;
	char		s2[MAX_STRING_CHARS];
	char		n2[MAX_STRING_CHARS];

	// numeric values are just slot numbers
	if( s[0] >= '0' && s[0] <= '9' )
	{
		idnum = atoi( s );
		if( idnum < 1 || idnum > theLevel.maxclients_ ) 
		{
			SV_GameSendServerCommand( to->s.number,//to-g_entities, 
									  va("print \"Bad client slot: %i\n\"", 
									  idnum));
			return -1;
		}

		cl = theLevel.getClient(idnum);//&level.clients[idnum];
		if( cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
		{
			SV_GameSendServerCommand( to->s.number,//to-g_entities, 
									  va("print \"Client %i is not active\n\"", 
									  idnum));
			return -1;
		}
		return idnum;
	}

	// check for a name match
	SanitizeString( s, s2 );
	for( idnum=1; idnum <= theLevel.maxclients_ ; idnum++ ) 
	{
		cl = theLevel.getClient(idnum);
		if( !cl || cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
			continue;

		SanitizeString( cl->pers_.netname_, n2 );
		if( !strcmp( n2, s2 ) ) {
			return idnum;
		}
	}

	SV_GameSendServerCommand( to->s.number,//to-g_entities, 
							  va("print \"User %s is not on the server\n\"", 
							  s));
	return -1;
}


/*
==================
Cmd_Noclip_f

argv(0) noclip
==================
*/
void Cmd_Noclip_f( GameEntity *ent ) 
{
	char	*msg;

	if( !CheatsOk( ent ) )
		return;

	if( ent->client_->noclip_ ) 
		msg = "noclip OFF\n";
	else 
		msg = "noclip ON\n";
	
	ent->client_->noclip_ = !ent->client_->noclip_;

	SV_GameSendServerCommand( ent->s.number,//ent-g_entities, 
							  va("print \"%s\"", 
							  msg));
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_LevelShot_f( GameEntity *ent )
{
	if( !CheatsOk( ent ) ) 
		return;

	// doesn't work in single player
	if( g_gametype.integer != 0 ) 
	{
		SV_GameSendServerCommand( ent->s.number,// ent-g_entities, 
			"print \"Must be in g_gametype 0 for levelshot\n\"" );
		return;
	}

	BeginIntermission();
	SV_GameSendServerCommand( ent->s.number,//ent-g_entities, 
							  "clientLevelShot" );
}


/*
==================
Cmd_LevelShot_f

This is just to help generate the level pictures
for the menus.  It goes to the intermission immediately
and sends over a command to the client to resize the view,
hide the scoreboard, and take a special screenshot
==================
*/
void Cmd_TeamTask_f( GameEntity *ent )
{
	char userinfo[MAX_INFO_STRING];
	char		arg[MAX_TOKEN_CHARS];
	int task;
	int client = ent->s.number;// ok ? before: ent->client_ - theLevel.clients_;

	if ( Cmd_Argc() != 2 ) 
		return;

	Cmd_ArgvBuffer( 1, arg, sizeof( arg ) );
	task = atoi( arg );

	SV_GetUserinfo(client, userinfo, sizeof(userinfo));
	Info_SetValueForKey(userinfo, "teamtask", va("%d", task));
	SV_SetUserinfo(client, userinfo);
	ClientUserinfoChanged(client);
}



/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f( GameEntity *ent ) 
{
	if( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
		return;

	if( ent->health_ <= 0 ) 
		return;

	ent->flags_ &= ~FL_GODMODE;
	if( (availableVehicles[ent->client_->vehicle_].cat & CAT_PLANE) ||
		(availableVehicles[ent->client_->vehicle_].cat & CAT_HELO) ) 
	{
		ent->client_->ps_.stats[STAT_HEALTH] = ent->health_ = -20;
	}
	else 
	{
		ent->client_->ps_.stats[STAT_HEALTH] = ent->health_ = -999;
	}
	VehicleDeathImpl( ent, ent, ent, 100000, MOD_SUICIDE );
}


/*
=================
Cmd_Tower_f
=================
*/
void Cmd_Tower_f( GameEntity *ent ) 
{
	if( !ent->radio_target_ ) 
		return;

	if( ent->health_ <= 0 ) 
		return;

	if( ent->radio_target_->wait_ > 0 )
	{
		ent->radio_target_->setThink(new Think_RadioComply);
		ent->radio_target_->nextthink_ = theLevel.time_ + 1000 * ent->radio_target_->wait_;
	} 
	else 
		G_UseTargets( ent->radio_target_, ent );

	if( !ent->radio_target_->message_ ) 
	{
		SV_GameSendServerCommand( ent->s.number,//ent-g_entities, 
								  va("print \"Tower, request operation of %s\n\"",
								  ent->radio_target_->classname_) );
	} 
	else 
	{
		SV_GameSendServerCommand( ent->s.number,//ent-g_entities, 
								  va("print \"Tower, %s\n\"", 
								  ent->radio_target_->message_) );
	}
}



/*
=================
Cmd_Radar_f
=================
*/
void Cmd_Radar_f( GameEntity *ent ) 
{
	if( ent->health_ <= 0 ) 
		return;

	if( theLevel.time_ < ent->radartime_ ) 
		return;

	if( !(ent->client_->ps_.ONOFF & OO_RADAR) ) 
	{
		if( availableVehicles[ent->client_->vehicle_].radarRange > 0 ) 
			ent->client_->ps_.ONOFF |= OO_RADAR_AIR;
		else if( availableVehicles[ent->client_->vehicle_].radarRange2 > 0 ) 
			ent->client_->ps_.ONOFF |= OO_RADAR_GROUND;
	} 
	else if( ent->client_->ps_.ONOFF & OO_RADAR_AIR ) 
	{
		if( availableVehicles[ent->client_->vehicle_].radarRange2 > 0 ) 
		{
			ent->client_->ps_.ONOFF |= OO_RADAR_GROUND;
			ent->client_->ps_.ONOFF &= ~OO_RADAR_AIR;
		} 
		else 
			ent->client_->ps_.ONOFF &= ~OO_RADAR;
	} 
	else if( ent->client_->ps_.ONOFF & OO_RADAR_GROUND ) 
		ent->client_->ps_.ONOFF &= ~OO_RADAR;

	untrack(ent);
	ent->radartime_ = theLevel.time_ + 200;

	ent->client_->pers_.lastRadar_ = (ent->client_->ps_.ONOFF & OO_RADAR);
}

/*
=================
Cmd_Unlock_f
=================
*/
void Cmd_Unlock_f( GameEntity *ent ) 
{
	if( ent->health_ <= 0 ) {
		return;
	}
	untrack(ent);
}

/*
=================
BroadCastTeamChange

Let everyone know about a team change
=================
*/
void BroadcastTeamChange( GameClient *client, int oldTeam )
{
	const char * pTeam = NULL;

	// work out what to shout
	switch( client->sess_.sessionTeam_ )
	{
	case ClientBase::TEAM_RED:
		pTeam = "red team";
		break;

	case ClientBase::TEAM_BLUE:
		pTeam = "blue team";
		break;

	case ClientBase::TEAM_SPECTATOR:
		// make sure we are changing TO spectator mode
		if( oldTeam != ClientBase::TEAM_SPECTATOR )
		{
			pTeam = "spectators";
		}
		break;
	case ClientBase::TEAM_FREE:
		pTeam = "the battle";
		break;
	}

	// send centre print and console trace
	if( pTeam )
	{
		SV_GameSendServerCommand( -1, va("cp \"%s" S_COLOR_WHITE " joined the %s\n\"", client->pers_.netname_, pTeam) );
		SV_GameSendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " joined the %s\n\"", client->pers_.netname_, pTeam) );
	}
}

/*
=================
SetTeam
=================
*/
void SetTeam( GameEntity *ent, char *s ) 
{
	ClientBase::eTeam	team, oldTeam;
	GameClient			*client;
	int					clientNum;
	GameClient::ClientSession::eSpectatorState specState;
	int					specClient;
	int					teamLeader;
	char				userinfo[MAX_INFO_STRING];
	bool				dontBalance = false;
	int					balanceNum = 0;
	const char *		pTeam = NULL;

	// only spect int mission editor
	if( g_gametype.integer == GT_MISSION_EDITOR )
		return;

	//
	// see what change is requested
	//
	client = ent->client_;
	oldTeam = client->sess_.sessionTeam_;

	clientNum = ent->s.number;// ok ? before: client - level.clients_;
	specClient = 0;
	specState = GameClient::ClientSession::SPECTATOR_NOT;

	if ( !Q_stricmp( s, "scoreboard" ) || !Q_stricmp( s, "score" )  )
	{
		team = ClientBase::TEAM_SPECTATOR;
		specState = GameClient::ClientSession::SPECTATOR_SCOREBOARD;
	}
	else if ( !Q_stricmp( s, "follow1" ) )
	{
		team = ClientBase::TEAM_SPECTATOR;
		specState = GameClient::ClientSession::SPECTATOR_FOLLOW;
		specClient = -1;
	}
	else if ( !Q_stricmp( s, "follow2" ) )
	{
		team = ClientBase::TEAM_SPECTATOR;
		specState = GameClient::ClientSession::SPECTATOR_FOLLOW;
		specClient = -2;
	}
	else if ( !Q_stricmp( s, "spectator" ) || !Q_stricmp( s, "s" ) )
	{
		team = ClientBase::TEAM_SPECTATOR;
		specState = GameClient::ClientSession::SPECTATOR_FREE;
	}
	else if ( g_gametype.integer >= GT_TEAM )
	{
		// if running a team game, assign player to one of the teams
		specState = GameClient::ClientSession::SPECTATOR_NOT;
		
		// request to join red?
		if ( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) )
		{
			team = ClientBase::TEAM_RED;
		}
		// request to join blue?
		else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) )
		{
			team = ClientBase::TEAM_BLUE;
		}
		else
		{
			// AUTOJOIN: pick a best team for the player

			// have we already joined the game? (i.e. are on a playable team?)
			if( oldTeam == ClientBase::TEAM_RED || 
				oldTeam == ClientBase::TEAM_BLUE )
			{
				SV_GameSendServerCommand( ent->client_->ps_.clientNum, "cp \"Can't auto-join, you are already on a team!\n\"" );
				
				return; // ignore the request
			}

			team = PickTeam( clientNum );

			// since we asked the server to 'auto-join' us, we should not do a balance check
			dontBalance = true;
		}

		// force team balance? (stop very uneven teams for being allowed)
		if( g_teamForceBalance.integer && !dontBalance )
		{
			int counts[ClientBase::TEAM_NUM_TEAMS];

			counts[ClientBase::TEAM_BLUE] = TeamCount( ent->client_->ps_.clientNum, ClientBase::TEAM_BLUE );
			counts[ClientBase::TEAM_RED] = TeamCount( ent->client_->ps_.clientNum, ClientBase::TEAM_RED );

			// work out the balanceNum
			balanceNum = g_teamForceBalanceNum.integer - 1;
			if( balanceNum < 0 ) 
				balanceNum = 0;

			// we allow a spread of g_teamForceBalanceNum, so if the value is 1 then there can never be
			// a spread of more than 1, this would allow 3-2 - but if another player joined then that player
			// would be forced to join the team with 2.
			if( team == ClientBase::TEAM_RED && counts[ClientBase::TEAM_RED] - 
					counts[ClientBase::TEAM_BLUE] > balanceNum )
			{
				SV_GameSendServerCommand( ent->client_->ps_.clientNum,"cp \"Red team has too many players.\n\"" );

				return; // ignore the request
			}

			if( team == ClientBase::TEAM_BLUE && 
				counts[ClientBase::TEAM_BLUE] - 
					counts[ClientBase::TEAM_RED] > balanceNum )
			{
				SV_GameSendServerCommand( ent->client_->ps_.clientNum, "cp \"Red team has too many players.\n\"" );
				return; // ignore the request
			}

			// if we get here it's ok, the team we are switching to has less or same number of players,
			// permit the joining of requested team
		}

	}
	else
	{
		// force them to spectators if there aren't any spots free
		team = ClientBase::TEAM_FREE;
	}

	// override decision if limiting the players
	if( (g_gametype.integer == GT_TOURNAMENT) && theLevel.numNonSpectatorClients_ >= 2 )
	{
		team = ClientBase::TEAM_SPECTATOR;
	}
	else if ( g_maxGameClients.integer > 0 && theLevel.numNonSpectatorClients_ >= g_maxGameClients.integer )
	{
		team = ClientBase::TEAM_SPECTATOR;
	}

	//
	// decide if we will allow the change
	//
	if( team == oldTeam )
	{
		// decided what to say
		switch( team )
		{
		case ClientBase::TEAM_FREE:
			pTeam = " in the game.";
			break;
		case ClientBase::TEAM_SPECTATOR:
			pTeam = " a spectator.";
			break;
		case ClientBase::TEAM_RED:
			pTeam = " on red.";
			break;
		case ClientBase::TEAM_BLUE:
			pTeam = " on blue.";
			break;
		}

		// say something?
		if( pTeam )
		{
			SV_GameSendServerCommand( ent->client_->ps_.clientNum, va( "cp \"You are already %s\n\"", pTeam ) );
		}
		return; // ignore the request
	}


	// 
	if( team == ClientBase::TEAM_SPECTATOR )
	{
		team = ClientBase::TEAM_FREE;
	}
	//
	// execute the team change
	//

	// he starts at 'base'
	client->pers_.teamState_.state_ = GameClient::ClientPersistant::PlayerTeamState::TEAM_BEGIN;
	if( oldTeam != ClientBase::TEAM_SPECTATOR ) 
	{
		// Kill him (makes sure he loses flags, etc)
		ent->flags_ &= ~FL_GODMODE;
		ent->client_->ps_.stats[STAT_HEALTH] = ent->health_ = 0;
		VehicleDeathImpl( ent, ent, ent, 100000, MOD_SUICIDE );

	}
	// they go to the end of the line for tournements
	if( team == ClientBase::TEAM_SPECTATOR ) 
	{
		client->sess_.spectatorTime_ = theLevel.time_;
		client->ps_.pm_flags &= ~(PMF_VEHICLESELECT|PMF_VEHICLESPAWN);// MFQ3
	}
	else
	{
		if( !(ent->r.svFlags & SVF_BOT) )// MFQ3
			client->ps_.pm_flags |= PMF_VEHICLESELECT;// MFQ3
	}
	// init vehicle
	SV_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	Info_SetValueForKey( userinfo, "cg_vehicle", "-1" );
	Info_SetValueForKey( userinfo, "cg_nextVehicle", "-1" );
	SV_SetUserinfo( clientNum, userinfo );

	client->sess_.sessionTeam_ = team; //static_cast<team_t>(team);
	client->sess_.spectatorState_ = specState;
	client->sess_.spectatorClient_ = specClient;

	client->sess_.teamLeader_ = false;
	if( team == ClientBase::TEAM_RED ||
		team == ClientBase::TEAM_BLUE )
	{
		teamLeader = TeamLeader( team );
		// if there is no team leader or the team leader is a bot and this client is not a bot
		if ( teamLeader == -1 || ( !(theLevel.getEntity(clientNum)->r.svFlags & SVF_BOT) && 
			(theLevel.getEntity(teamLeader)->r.svFlags & SVF_BOT) ) )
		{
			SetLeader( team, clientNum );
		}
	}
	// make sure there is a team leader on the team the player came from
	if( oldTeam == ClientBase::TEAM_RED || 
		oldTeam == ClientBase::TEAM_BLUE ) 
		CheckTeamLeader( oldTeam );

	// tell everyone about the team change for this client
	BroadcastTeamChange( client, oldTeam );

	// get and distribute relevent paramters
	ClientUserinfoChanged( clientNum );

	MF_ClientBegin( clientNum );
}

/*
=================
StopFollowing

If the client being followed leaves the game, or you just want to drop
to free floating spectator mode
=================
*/
void StopFollowing( GameEntity *ent ) 
{
	ent->client_->ps_.persistant[ PERS_TEAM ] = ClientBase::TEAM_SPECTATOR;	
	ent->client_->sess_.sessionTeam_ = ClientBase::TEAM_SPECTATOR;	
	ent->client_->sess_.spectatorState_ = GameClient::ClientSession::SPECTATOR_FREE;
	ent->client_->ps_.pm_flags &= ~PMF_FOLLOW;
	ent->r.svFlags &= ~SVF_BOT;
	ent->client_->ps_.clientNum = ent->s.number;// ent - g_entities;
}

/*
=================
Cmd_Team_f
=================
*/
void Cmd_Team_f( GameEntity *ent )
{
	ClientBase::eTeam oldTeam;
	char		s[MAX_TOKEN_CHARS];

	if ( Cmd_Argc() != 2 )
	{
		oldTeam = ent->client_->sess_.sessionTeam_;
		switch ( oldTeam )
		{
		case ClientBase::TEAM_BLUE:
			SV_GameSendServerCommand( ent->s.number, "print \"Blue team\n\"" );
			break;
		case ClientBase::TEAM_RED:
			SV_GameSendServerCommand( ent->s.number, "print \"Red team\n\"" );
			break;
		case ClientBase::TEAM_FREE:
			SV_GameSendServerCommand( ent->s.number, "print \"Free team\n\"" );
			break;
		case ClientBase::TEAM_SPECTATOR:
			SV_GameSendServerCommand( ent->s.number, "print \"Spectator team\n\"" );
			break;
		}		
		
		return;
	}

	// stop people changing teams too much
	if( ent->client_->switchTeamTime_ > theLevel.time_ )
	{
		SV_GameSendServerCommand( ent->s.number, "print \"May not request to switch teams more than once per 5 seconds.\n\"" );
		return;
	}

	// if they are playing a tournement game, count as a loss
	if( (g_gametype.integer == GT_TOURNAMENT ) && 
		ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_FREE )
	{
		ent->client_->sess_.losses_++;
	}

	Cmd_ArgvBuffer( 1, s, sizeof( s ) );

	// set team
	SetTeam( ent, s );

	// set the time when the next request will be allowed (level time plus 5 seconds)
	ent->client_->switchTeamTime_ = theLevel.time_ + 5000;
}


/*
=================
Cmd_Follow_f
=================
*/
void Cmd_Follow_f( GameEntity *ent ) 
{
	int		i;
	char	arg[MAX_TOKEN_CHARS];

	if( Cmd_Argc() != 2 ) 
	{
		if( ent->client_->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_FOLLOW ) 
			StopFollowing( ent );
		return;
	}

	Cmd_ArgvBuffer( 1, arg, sizeof( arg ) );
	i = ClientNumberFromString( ent, arg );
	if( i == -1 ) 
		return;

	// can't follow self
	if( theLevel.getClient(i) == ent->client_ ) 
		return;

	// can't follow another spectator
	if( theLevel.getClient(i)->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
		return;

	// if they are playing a tournement game, count as a loss
	if( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_FREE ) 
	{
		ent->client_->sess_.losses_++;
	}

	// first set them to spectator
	if( ent->client_->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
		SetTeam( ent, "spectator" );

	ent->client_->sess_.spectatorState_ = GameClient::ClientSession::SPECTATOR_FOLLOW;
	ent->client_->sess_.spectatorClient_ = i;
}

/*
=================
Cmd_FollowCycle_f
=================
*/
void Cmd_FollowCycle_f( GameEntity *ent, int dir )
{
	// if they are playing a tournement game, count as a loss
	if( (g_gametype.integer == GT_TOURNAMENT )
		&& ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_FREE ) 
	{
		ent->client_->sess_.losses_++;
	}
	// first set them to spectator
	if( ent->client_->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_NOT ) 
		SetTeam( ent, "spectator" );

	if( dir != 1 && dir != -1 ) 
		Com_Error( ERR_DROP, "Cmd_FollowCycle_f: bad dir %i", dir );

	int clientnum = ent->client_->sess_.spectatorClient_;
	int original = clientnum;
	do {
		clientnum += dir;
		if( clientnum > theLevel.maxclients_ ) 
			clientnum = 1;

		if( clientnum < 1 ) 
			clientnum = theLevel.maxclients_;

		GameClient* currCl = theLevel.getClient(clientnum);

		// can only follow connected clients
		if( !currCl || currCl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
			continue;

		// can't follow another spectator
		if( currCl->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
			continue;

		// this is good, we can use it
		ent->client_->sess_.spectatorClient_ = clientnum;
		ent->client_->sess_.spectatorState_ = GameClient::ClientSession::SPECTATOR_FOLLOW;
		return;
	} while( clientnum != original );

	// leave it where it was
}


/*
==================
G_Say
==================
*/

static void G_SayTo( GameEntity *ent, GameEntity *other, int mode, int color, const char *name, const char *message )
{
	if( !other ) 
		return;
	if( !other->inuse_ ) 
		return;
	if( !other->client_ )
		return;
	if( mode == SAY_TEAM  && !OnSameTeam(ent, other) ) 
		return;

	// no chatting to players in tournements
	if( (g_gametype.integer == GT_TOURNAMENT )
		&& other->client_->sess_.sessionTeam_ == ClientBase::TEAM_FREE
		&& ent->client_->sess_.sessionTeam_ != ClientBase::TEAM_FREE ) 
	{
		return;
	}

	SV_GameSendServerCommand( other->s.number,// other-g_entities, 
							  va("%s \"%s%c%c%s\"", 
							  mode == SAY_TEAM ? "tchat" : "chat",
							  name, Q_COLOR_ESCAPE, color, message));
}

#define EC		"\x19"

void G_Say( GameEntity *ent, GameEntity *target, int mode, const char *chatText ) 
{
	int			color;
	char		name[64];
	// don't let text be too long for malicious reasons
	char		text[MAX_SAY_TEXT];
	char		location[64];

	if ( g_gametype.integer < GT_TEAM && mode == SAY_TEAM ) 
		mode = SAY_ALL;

	switch( mode ) 
	{
	default:
	case SAY_ALL:
		G_LogPrintf( "say: %s: %s\n", ent->client_->pers_.netname_, chatText );
		Com_sprintf (name, sizeof(name), "%s%c%c"EC": ", ent->client_->pers_.netname_, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_GREEN;
		break;
	case SAY_TEAM:
		G_LogPrintf( "sayteam: %s: %s\n", ent->client_->pers_.netname_, chatText );
		if( Team_GetLocationMsg(ent, location, sizeof(location)) )
			Com_sprintf (name, sizeof(name), EC"(%s%c%c"EC") (%s)"EC": ", 
				ent->client_->pers_.netname_, Q_COLOR_ESCAPE, COLOR_WHITE, location);
		else
			Com_sprintf( name, sizeof(name), EC"(%s%c%c"EC")"EC": ", 
				ent->client_->pers_.netname_, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_CYAN;
		break;
	case SAY_TELL:
		if( target && g_gametype.integer >= GT_TEAM &&
			target->client_->sess_.sessionTeam_ == ent->client_->sess_.sessionTeam_ &&
			Team_GetLocationMsg(ent, location, sizeof(location)) )
			Com_sprintf( name, sizeof(name), EC"[%s%c%c"EC"] (%s)"EC": ", ent->client_->pers_.netname_, Q_COLOR_ESCAPE, COLOR_WHITE, location );
		else
			Com_sprintf( name, sizeof(name), EC"[%s%c%c"EC"]"EC": ", ent->client_->pers_.netname_, Q_COLOR_ESCAPE, COLOR_WHITE );
		color = COLOR_MAGENTA;
		break;
	}

	Q_strncpyz( text, chatText, sizeof(text) );

	if( target ) 
	{
		G_SayTo( ent, target, mode, color, name, text );
		return;
	}

	// echo the text to the console
	if( g_dedicated.integer ) 
		Com_Printf( "%s%s\n", name, text);

	// send it to all the apropriate clients
	for( int j = 1; j <= theLevel.maxclients_; j++ ) 
	{
		GameEntity* other = theLevel.getEntity(j);
		if( !other )
			continue;
		G_SayTo( ent, other, mode, color, name, text );
	}
}


/*
==================
Cmd_Say_f
==================
*/
static void Cmd_Say_f( GameEntity *ent, int mode, bool arg0 ) 
{
	char		*p;

	if( Cmd_Argc () < 2 && !arg0 ) 
		return;

	if( arg0 )
		p = ConcatArgs( 0 );
	else
		p = ConcatArgs( 1 );

	G_Say( ent, NULL, mode, p );
}

/*
==================
Cmd_Tell_f
==================
*/
static void Cmd_Tell_f( GameEntity* ent ) 
{
	if( Cmd_Argc () < 2 ) 
		return;

	char arg[MAX_TOKEN_CHARS];

	Cmd_ArgvBuffer( 1, arg, sizeof( arg ) );
	int targetNum = atoi( arg );
	if( targetNum < 1 || targetNum > theLevel.maxclients_ ) 
		return;

	GameEntity* target = theLevel.getEntity(targetNum);
	if( !target || !target->inuse_ || !target->client_ ) 
		return;

	char* p = ConcatArgs( 2 );

	G_LogPrintf( "tell: %s to %s: %s\n", ent->client_->pers_.netname_, target->client_->pers_.netname_, p );
	G_Say( ent, target, SAY_TELL, p );
	// don't tell to the player self if it was already directed to this player
	// also don't send the chat back to a bot
	if ( ent != target && !(ent->r.svFlags & SVF_BOT)) 
		G_Say( ent, ent, SAY_TELL, p );
}


static char	*gc_orders[] = {
	"hold your position",
	"hold this position",
	"come here",
	"cover me",
	"guard location",
	"search and destroy",
	"report"
};

void Cmd_GameCommand_f( GameEntity *ent ) 
{
	char	str[MAX_TOKEN_CHARS];

	Cmd_ArgvBuffer( 1, str, sizeof( str ) );
	int player = atoi( str );
	Cmd_ArgvBuffer( 2, str, sizeof( str ) );
	int order = atoi( str );

	if( player < 1 || player > MAX_CLIENTS ) 
		return;
	if( order < 0 || order > sizeof(gc_orders)/sizeof(char *) ) 
		return;
	G_Say( ent, theLevel.getEntity(player), SAY_TELL, gc_orders[order] );
	G_Say( ent, ent, SAY_TELL, gc_orders[order] );
}

/*
==================
Cmd_Where_f
==================
*/
void Cmd_Where_f( GameEntity *ent )
{
	SV_GameSendServerCommand( ent->s.number, va("print \"%s\n\"", vtos( ent->s.origin ) ) );
}

static const char *gameNames[] = {
	"Free For All",
	"Tournament",
	"Single Player",
	"MFQ3 Mission Editor",
	"Team Deathmatch",
	"Capture the Flag",
	"One Flag CTF",
	"Overload",
	"Harvester"
};

/*
==================
Cmd_CallVote_f
==================
*/
void Cmd_CallVote_f( GameEntity *ent ) 
{
	if( !g_allowVote.integer )
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if( theLevel.voteTime_ ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"A vote is already in progress.\n\"" );
		return;
	}
	if( ent->client_->pers_.voteCount_ >= MAX_VOTE_COUNT ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"You have called the maximum number of votes.\n\"" );
		return;
	}
	if( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	// make sure it is a valid command to vote on
	Cmd_ArgvBuffer( 1, arg1, sizeof( arg1 ) );
	Cmd_ArgvBuffer( 2, arg2, sizeof( arg2 ) );

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Invalid vote string.\n\"" );
		return;
	}

	if( !Q_stricmp( arg1, "map_restart" ) ) 
	{
	} 
	else if( !Q_stricmp( arg1, "nextmap" ) )
	{
	} 
	else if( !Q_stricmp( arg1, "map" ) ) 
	{
	} 
	else if( !Q_stricmp( arg1, "g_gametype" ) ) 
	{
	} 
	else if( !Q_stricmp( arg1, "kick" ) ) 
	{
	} 
	else if( !Q_stricmp( arg1, "clientkick" ) ) 
	{
	} 
	else if( !Q_stricmp( arg1, "g_doWarmup" ) ) 
	{
	} 
	else if( !Q_stricmp( arg1, "timelimit" ) ) 
	{
	} 
	else if( !Q_stricmp( arg1, "fraglimit" ) ) 
	{
	} 
	else if( !Q_stricmp( arg1, "mf_gameset" ) ) 
	{
	} 
	else 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Invalid vote string.\n\"" );
		SV_GameSendServerCommand( ent->s.number, "print \"Vote commands are: map_restart, nextmap, map <mapname>, g_gametype <n>, kick <player>, clientkick <clientnum>, g_doWarmup, timelimit <time>, fraglimit <frags>, mf_gameset <set>.\n\"" );
		return;
	}

	// if there is still a vote to be executed
	if( theLevel.voteExecuteTime_ ) 
	{
		theLevel.voteExecuteTime_ = 0;
		Cbuf_ExecuteText( EXEC_APPEND, va("%s\n", theLevel.voteString_ ) );
	}

	int i = 0;
	// special case for g_gametype, check for bad values
	if( !Q_stricmp( arg1, "g_gametype" ) )
	{
		i = atoi( arg2 );
		if( i == GT_SINGLE_PLAYER || i == GT_MISSION_EDITOR || i < GT_FFA || i >= GT_MAX_GAME_TYPE ) 
		{
			SV_GameSendServerCommand( ent->s.number, "print \"Invalid gametype.\n\"" );
			return;
		}

		Com_sprintf( theLevel.voteString_, sizeof( theLevel.voteString_ ), "%s %d", arg1, i );
		Com_sprintf( theLevel.voteDisplayString_, sizeof( theLevel.voteDisplayString_ ), "%s %s", arg1, gameNames[i] );
	}
	else if( !(Q_stricmp( arg1, "mf_gameset") ) ) 
	{
		if( Q_stricmp( arg2, "modern" ) != 0 && Q_stricmp( arg2, "ww2" ) != 0 &&
			Q_stricmp( arg2, "ww1" ) != 0 ) {
			return;
		}
		Com_sprintf( theLevel.voteString_, sizeof( theLevel.voteString_ ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( theLevel.voteDisplayString_, sizeof( theLevel.voteDisplayString_ ), "%s", theLevel.voteString_ );
	}
	else if( !Q_stricmp( arg1, "map" ) ) 
	{
		// special case for map changes, we want to reset the nextmap setting
		// this allows a player to change maps, but not upset the map rotation
		char	s[MAX_STRING_CHARS];

		Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if( *s )
			Com_sprintf( theLevel.voteString_, sizeof( theLevel.voteString_ ), "%s %s; set nextmap \"%s\"", arg1, arg2, s );
		else
			Com_sprintf( theLevel.voteString_, sizeof( theLevel.voteString_ ), "%s %s", arg1, arg2 );
		Com_sprintf( theLevel.voteDisplayString_, sizeof( theLevel.voteDisplayString_ ), "%s", theLevel.voteString_ );
	} 
	else if( !Q_stricmp( arg1, "nextmap" ) )
	{
		char	s[MAX_STRING_CHARS];

		Cvar_VariableStringBuffer( "nextmap", s, sizeof(s) );
		if( !*s )
		{
			SV_GameSendServerCommand( ent->s.number, "print \"nextmap not set.\n\"" );
			return;
		}
		Com_sprintf( theLevel.voteString_, sizeof( theLevel.voteString_ ), "vstr nextmap");
		Com_sprintf( theLevel.voteDisplayString_, sizeof( theLevel.voteDisplayString_ ), "%s", theLevel.voteString_ );
	} 
	else 
	{
		Com_sprintf( theLevel.voteString_, sizeof( theLevel.voteString_ ), "%s \"%s\"", arg1, arg2 );
		Com_sprintf( theLevel.voteDisplayString_, sizeof( theLevel.voteDisplayString_ ), "%s", theLevel.voteString_ );
	}

	SV_GameSendServerCommand( -1, va("print \"%s called a vote.\n\"", ent->client_->pers_.netname_ ) );

	// start the voting, the caller autoamtically votes yes
	theLevel.voteTime_ = theLevel.time_;
	theLevel.voteYes_ = 1;
	theLevel.voteNo_ = 0;

	for( i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		theLevel.getClient(i)->ps_.eFlags &= ~EF_VOTED;
	}
	ent->client_->ps_.eFlags |= EF_VOTED;

	SV_SetConfigstring( CS_VOTE_TIME, va("%i", theLevel.voteTime_ ) );
	SV_SetConfigstring( CS_VOTE_STRING, theLevel.voteDisplayString_ );	
	SV_SetConfigstring( CS_VOTE_YES, va("%i", theLevel.voteYes_ ) );
	SV_SetConfigstring( CS_VOTE_NO, va("%i", theLevel.voteNo_ ) );	
}

/*
==================
Cmd_Vote_f
==================
*/
void Cmd_Vote_f( GameEntity *ent ) 
{
	char msg[64];

	if( !theLevel.voteTime_ ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"No vote in progress.\n\"" );
		return;
	}
	if( ent->client_->ps_.eFlags & EF_VOTED ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Vote already cast.\n\"" );
		return;
	}
	if( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	SV_GameSendServerCommand( ent->s.number, "print \"Vote cast.\n\"" );

	ent->client_->ps_.eFlags |= EF_VOTED;

	Cmd_ArgvBuffer( 1, msg, sizeof( msg ) );

	if( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' )
	{
		theLevel.voteYes_++;
		SV_SetConfigstring( CS_VOTE_YES, va("%i", theLevel.voteYes_ ) );
	} 
	else 
	{
		theLevel.voteNo_++;
		SV_SetConfigstring( CS_VOTE_NO, va("%i", theLevel.voteNo_ ) );	
	}

	// a majority will be determined in CheckVote, which will also account
	// for players entering or leaving
}

/*
==================
Cmd_CallTeamVote_f
==================
*/
void Cmd_CallTeamVote_f( GameEntity *ent ) 
{
	int		cs_offset;
	ClientBase::eTeam team = ent->client_->sess_.sessionTeam_;

	if( team == ClientBase::TEAM_RED )
		cs_offset = 0;
	else if( team == ClientBase::TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if( !g_allowVote.integer ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Voting not allowed here.\n\"" );
		return;
	}

	if( theLevel.teamVoteTime_[cs_offset] ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"A team vote is already in progress.\n\"" );
		return;
	}
	if( ent->client_->pers_.teamVoteCount_ >= MAX_VOTE_COUNT ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"You have called the maximum number of team votes.\n\"" );
		return;
	}
	if( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Not allowed to call a vote as spectator.\n\"" );
		return;
	}

	char	arg1[MAX_STRING_TOKENS];
	char	arg2[MAX_STRING_TOKENS];

	// make sure it is a valid command to vote on
	Cmd_ArgvBuffer( 1, arg1, sizeof( arg1 ) );
	arg2[0] = '\0';
	int i;
	for( i = 2; i < Cmd_Argc(); i++ ) 
	{
		if( i > 2 )
			strcat(arg2, " ");
		Cmd_ArgvBuffer( i, &arg2[strlen(arg2)], sizeof( arg2 ) - strlen(arg2) );
	}

	if( strchr( arg1, ';' ) || strchr( arg2, ';' ) ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Invalid vote string.\n\"" );
		return;
	}

	if( !Q_stricmp( arg1, "leader" ) ) 
	{
		char netname[MAX_NETNAME], leader[MAX_NETNAME];

		if( !arg2[0] ) 
			i = ent->client_->ps_.clientNum;
		else 
		{
			// numeric values are just slot numbers
			for( i = 0; i < 3; i++ )
			{
				if ( !arg2[i] || arg2[i] < '0' || arg2[i] > '9' )
					break;
			}
			if( i >= 3 || !arg2[i]) 
			{
				i = atoi( arg2 );
				if( i < 1 || i > theLevel.maxclients_ ) 
				{
					SV_GameSendServerCommand( ent->s.number, va("print \"Bad client slot: %i\n\"", i) );
					return;
				}

				if( !theLevel.getEntity(i) || !theLevel.getEntity(i)->inuse_ ) 
				{
					SV_GameSendServerCommand( ent->s.number, va("print \"Client %i is not active\n\"", i) );
					return;
				}
			}
			else 
			{
				Q_strncpyz(leader, arg2, sizeof(leader));
				Q_CleanStr(leader);
				for( i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
				{
					GameClient* currCl = theLevel.getClient(i);
					if( !currCl || currCl->pers_.connected_ == GameClient::ClientPersistant::CON_DISCONNECTED )
						continue;
					if( currCl->sess_.sessionTeam_ != team)
						continue;
					Q_strncpyz(netname, currCl->pers_.netname_, sizeof(netname));
					Q_CleanStr(netname);
					if( !Q_stricmp(netname, leader) ) 
						break;
				}
				if ( i > theLevel.maxclients_ ) 
				{
					SV_GameSendServerCommand( ent->s.number, va("print \"%s is not a valid player on your team.\n\"", arg2) );
					return;
				}
			}
		}
		Com_sprintf(arg2, sizeof(arg2), "%d", i);
	} 
	else 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Invalid vote string.\n\"" );
		SV_GameSendServerCommand( ent->s.number, "print \"Team vote commands are: leader <player>.\n\"" );
		return;
	}

	Com_sprintf( theLevel.teamVoteString_[cs_offset], sizeof( theLevel.teamVoteString_[cs_offset] ), "%s %s", arg1, arg2 );

	for( i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		GameClient* currCl = theLevel.getClient(i);
		if( !currCl || currCl->pers_.connected_ == GameClient::ClientPersistant::CON_DISCONNECTED )
			continue;
		if( currCl->sess_.sessionTeam_ == team )
			SV_GameSendServerCommand( i, va("print \"%s called a team vote.\n\"", ent->client_->pers_.netname_ ) );
	}

	// start the voting, the caller autoamtically votes yes
	theLevel.teamVoteTime_[cs_offset] = theLevel.time_;
	theLevel.teamVoteYes_[cs_offset] = 1;
	theLevel.teamVoteNo_[cs_offset] = 0;

	for( i = 1 ; i <= theLevel.maxclients_; i++ ) 
	{
		GameClient* currCl = theLevel.getClient(i);
		if( currCl && currCl->sess_.sessionTeam_ == team )
			currCl->ps_.eFlags &= ~EF_TEAMVOTED;
	}
	ent->client_->ps_.eFlags |= EF_TEAMVOTED;

	SV_SetConfigstring( CS_TEAMVOTE_TIME + cs_offset, va("%i", theLevel.teamVoteTime_[cs_offset] ) );
	SV_SetConfigstring( CS_TEAMVOTE_STRING + cs_offset, theLevel.teamVoteString_[cs_offset] );
	SV_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", theLevel.teamVoteYes_[cs_offset] ) );
	SV_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", theLevel.teamVoteNo_[cs_offset] ) );
}

/*
==================
Cmd_TeamVote_f
==================
*/
void Cmd_TeamVote_f( GameEntity *ent )
{
	int			team, cs_offset;
	char		msg[64];

	team = ent->client_->sess_.sessionTeam_;
	if( team == ClientBase::TEAM_RED )
		cs_offset = 0;
	else if( team == ClientBase::TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if( !theLevel.teamVoteTime_[cs_offset] ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"No team vote in progress.\n\"" );
		return;
	}
	if( ent->client_->ps_.eFlags & EF_TEAMVOTED ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Team vote already cast.\n\"" );
		return;
	}
	if( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		SV_GameSendServerCommand( ent->s.number, "print \"Not allowed to vote as spectator.\n\"" );
		return;
	}

	SV_GameSendServerCommand( ent->s.number, "print \"Team vote cast.\n\"" );

	ent->client_->ps_.eFlags |= EF_TEAMVOTED;

	Cmd_ArgvBuffer( 1, msg, sizeof( msg ) );

	if( msg[0] == 'y' || msg[1] == 'Y' || msg[1] == '1' ) 
	{
		theLevel.teamVoteYes_[cs_offset]++;
		SV_SetConfigstring( CS_TEAMVOTE_YES + cs_offset, va("%i", theLevel.teamVoteYes_[cs_offset] ) );
	} 
	else 
	{
		theLevel.teamVoteNo_[cs_offset]++;
		SV_SetConfigstring( CS_TEAMVOTE_NO + cs_offset, va("%i", theLevel.teamVoteNo_[cs_offset] ) );	
	}

	// a majority will be determined in TeamCheckVote, which will also account
	// for players entering or leaving
}


/*
=================
Cmd_Eject_f
=================
*/
void Cmd_Eject_f( GameEntity *ent ) 
{
	GameEntity	*veh;
//	gclient_t	*client;
//	char	userinfo[MAX_INFO_STRING];
//	int index = ent - g_entities;
	vec3_t start;
//	int i;

	Com_Printf ("eject!!\n");
	if( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
		return;

	if( ent->health_ <= 0) 
		return;

	VectorCopy( ent->s.pos.trBase, start );
	SnapVector( start );

	ent->flags_ &= ~FL_GODMODE;
	if( (availableVehicles[ent->client_->vehicle_].cat & CAT_PLANE) ||
		(availableVehicles[ent->client_->vehicle_].cat & CAT_HELO) ) 
	{
		veh = theLevel.spawnEntity();
		veh->init(-1);
		veh->classname_ = "misc_vehicle";
		veh->health_ = 0;
		veh->client_ = NULL;
		veh->setThink(new Think_ExplodeVehicle);
		veh->nextthink_ = theLevel.time_ + 10000;
		veh->tracktarget_ = NULL;
		veh->parent_ = ent;
		veh->r.ownerNum = veh->parent_->s.number;
		veh->s.eType = ET_MISC_VEHICLE;
		veh->r.svFlags = SVF_USE_CURRENT_ORIGIN;
		veh->s.modelindex2 = 255;
		veh->s.modelindex = ent->client_->vehicle_;
		VectorCopy( start, veh->s.pos.trBase );
		VectorCopy(start, veh->r.currentOrigin);
	}
}


/*
=================
Cmd_ME_Spawn_f
=================
*/
void Cmd_ME_Spawn_f( GameEntity *ent ) 
{

	int		idx;
	char	arg1[MAX_STRING_TOKENS];

	Cmd_ArgvBuffer( 1, arg1, sizeof( arg1 ) );

	idx = atoi(arg1);
	if( idx < 0 || idx >= bg_numberOfVehicles ) 
		return;

	SV_GameSendServerCommand( -1, va("me_spawnvehicle %d\n", idx) );
}

/*
=================
Cmd_ME_SpawnGI_f
=================
*/
void Cmd_ME_SpawnGI_f( GameEntity *ent ) 
{

	int		idx;
	char	arg1[MAX_STRING_TOKENS];

	Cmd_ArgvBuffer( 1, arg1, sizeof( arg1 ) );

	idx = atoi(arg1);
	if( idx < 0 || idx >= bg_numberOfGroundInstallations ) 
		return;

	SV_GameSendServerCommand( -1, va("me_spawnvehiclegi %d\n", idx) );
}

/*
=================
ClientCommand
=================
*/
void ClientCommand( int clientNum )
{
	GameEntity *ent;
	char	cmd[MAX_TOKEN_CHARS];

	ent = theLevel.getEntity(clientNum);
	if( !ent || !ent->client_ ) 
		return;		// not fully in game yet

	Cmd_ArgvBuffer( 0, cmd, sizeof( cmd ) );

	if( Q_stricmp( cmd, "say") == 0 ) 
	{
		Cmd_Say_f( ent, SAY_ALL, false );
		return;
	}
	if( Q_stricmp( cmd, "say_team" ) == 0 ) 
	{
		Cmd_Say_f( ent, SAY_TEAM, false );
		return;
	}
	if( Q_stricmp( cmd, "tell" ) == 0 )
	{
		Cmd_Tell_f( ent );
		return;
	}
	if( Q_stricmp( cmd, "score" ) == 0 )
	{
		Cmd_Score_f( ent );
		return;
	}

	// ignore all other commands when at intermission
	if( theLevel.intermissiontime_ )
	{
		Cmd_Say_f (ent, false, true);
		return;
	}

	if( Q_stricmp (cmd, "noclip") == 0 )
		Cmd_Noclip_f (ent);
	else if( Q_stricmp (cmd, "kill") == 0 )
		Cmd_Kill_f (ent);
	else if( Q_stricmp (cmd, "contact_tower") == 0 )
		Cmd_Tower_f (ent);
	else if( Q_stricmp (cmd, "radar") == 0 )
		Cmd_Radar_f (ent);
	else if( Q_stricmp (cmd, "unlock") == 0 )
		Cmd_Unlock_f (ent);
	else if( Q_stricmp (cmd, "teamtask") == 0 )
		Cmd_TeamTask_f (ent);
	else if( Q_stricmp (cmd, "levelshot") == 0 )
		Cmd_LevelShot_f (ent);
	else if( Q_stricmp (cmd, "follow") == 0 )
		Cmd_Follow_f (ent);
	else if( Q_stricmp (cmd, "follownext") == 0 )
		Cmd_FollowCycle_f (ent, 1);
	else if( Q_stricmp (cmd, "followprev") == 0 )
		Cmd_FollowCycle_f (ent, -1);
	else if( Q_stricmp (cmd, "team") == 0 )
		Cmd_Team_f (ent);
	else if( Q_stricmp (cmd, "where") == 0 )
		Cmd_Where_f (ent);
	else if( Q_stricmp (cmd, "callvote") == 0 )
		Cmd_CallVote_f (ent);
	else if( Q_stricmp (cmd, "vote") == 0 )
		Cmd_Vote_f (ent);
	else if( Q_stricmp (cmd, "callteamvote") == 0 )
		Cmd_CallTeamVote_f (ent);
	else if( Q_stricmp (cmd, "teamvote") == 0 )
		Cmd_TeamVote_f (ent);
	else if( Q_stricmp (cmd, "gc") == 0 )
		Cmd_GameCommand_f( ent );
	else if( Q_stricmp (cmd, "me_spawn") == 0 )
		Cmd_ME_Spawn_f( ent );
	else if( Q_stricmp (cmd, "me_spawngi") == 0 )
		Cmd_ME_SpawnGI_f( ent );
	else if( Q_stricmp (cmd, "eject") == 0 )
		Cmd_Eject_f( ent );

	else
		SV_GameSendServerCommand( clientNum, va("print \"unknown cmd %s\n\"", cmd ) );
}
