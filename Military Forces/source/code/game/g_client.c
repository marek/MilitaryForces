/*
 * $Id: g_client.c,v 1.7 2006-01-29 14:03:41 thebjoern Exp $
*/

//null cvs upload test, comment placed for a difference.

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"

// g_client.c -- client functions that don't happen every frame

static vec3_t	playerMins = {-15, -15, -24};
static vec3_t	playerMaxs = {15, 15, 32};

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32) initial
potential spawning position for deathmatch games.
The first time a player enters the game, they will be at an 'initial' spot.
Targets will be fired when someone spawns in on them.
"nobots" will prevent bots from using this spot.
"nohumans" will prevent non-bots from using this spot.
*/
void SP_info_player_deathmatch( GameEntity *ent ) 
{
	int		i;

	if( !ent->ent_category_ ) 
		ent->ent_category_ = CAT_ANY;
//	else ent->ent_category <<= 8;
	
	// update level information
	theLevel.ent_category_ |= ent->ent_category_;

	Cvar_Set( "mf_lvcat", va("%i", theLevel.ent_category_) );

	G_SpawnInt( "nobots", "0", &i);
	if( i ) 
		ent->flags_ |= FL_NO_BOTS;

	G_SpawnInt( "nohumans", "0", &i );
	if( i ) 
		ent->flags_ |= FL_NO_HUMANS;
}

/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
equivelant to info_player_deathmatch
*/
void SP_info_player_start( GameEntity *ent ) 
{
	ent->classname_ = "info_player_deathmatch";
	SP_info_player_deathmatch( ent );
}

/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The intermission will be viewed from this point.  Target an info_notnull for the view direction.
*/
void SP_info_player_intermission( GameEntity *ent ) 
{
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
SpotWouldTelefrag

================
*/
bool SpotWouldTelefrag( GameEntity *spot )
{
	int			touch[MAX_GENTITIES];
	vec3_t		mins, maxs;

	VectorAdd( spot->s.origin, playerMins, mins );
	VectorAdd( spot->s.origin, playerMaxs, maxs );
	int num = SV_AreaEntities( mins, maxs, touch, MAX_GENTITIES );

	for( int i=0 ; i<num ; i++ )
	{
		GameEntity* hit = theLevel.getEntity(touch[i]);
		//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
		if ( hit && hit->client_ ) 
			return true;
	}
	return false;
}

/*
================
SelectNearestDeathmatchSpawnPoint

Find the spot that we DON'T want to use
================
*/
#define	MAX_SPAWN_POINTS	128
GameEntity *SelectNearestDeathmatchSpawnPoint( vec3_t from ) 
{
	GameEntity	*spot;
	vec3_t		delta;
	float		dist, nearestDist;
	GameEntity	*nearestSpot;

	nearestDist = 999999;
	nearestSpot = NULL;
	spot = NULL;

	while( (spot = G_Find (spot, FOFS(classname_), "info_player_deathmatch") ) != NULL )
	{
		VectorSubtract( spot->s.origin, from, delta );
		dist = VectorLength( delta );
		if( dist < nearestDist ) 
		{
			nearestDist = dist;
			nearestSpot = spot;
		}
	}
	return nearestSpot;
}


/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_SPAWN_POINTS	128
GameEntity *SelectRandomDeathmatchSpawnPoint()
{
	GameEntity	*spot;
	int			count;
	int			selection;
	GameEntity	*spots[MAX_SPAWN_POINTS];

	count = 0;
	spot = NULL;

	while( (spot = G_Find (spot, FOFS(classname_), "info_player_deathmatch")) != NULL )
	{
		if( SpotWouldTelefrag( spot ) ) 
			continue;
		spots[ count ] = spot;
		count++;
	}

	if( !count ) 
	{
		// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname_), "info_player_deathmatch");
	}

	selection = rand() % count;
	return spots[ selection ];
}

/*
===========
SelectRandomFurthestSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
GameEntity *SelectRandomFurthestSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles, int category ) 
{
	GameEntity	*spot;
	vec3_t		delta;
	float		dist;
	float		list_dist[64];
	GameEntity	*list_spot[64];
	int			numSpots, rnd, i, j;

	numSpots = 0;
	spot = NULL;

	while( (spot = G_Find (spot, FOFS(classname_), "info_player_deathmatch")) != NULL )
	{
		if( SpotWouldTelefrag( spot ) ) 
			continue;

		if( category < 0 || !(spot->ent_category_ & category) )
			continue;
		VectorSubtract( spot->s.origin, avoidPoint, delta );
		dist = VectorLength( delta );
		for( i = 0; i < numSpots; i++ )
		{
			if( dist > list_dist[i] ) 
			{
				if( numSpots >= 64 )
					numSpots = 64-1;
				for( j = numSpots; j > i; j-- )
				{
					list_dist[j] = list_dist[j-1];
					list_spot[j] = list_spot[j-1];
				}
				list_dist[i] = dist;
				list_spot[i] = spot;
				numSpots++;
				if( numSpots > 64 )
					numSpots = 64;
				break;
			}
		}
		if( i >= numSpots && numSpots < 64 )
		{
			list_dist[numSpots] = dist;
			list_spot[numSpots] = spot;
			numSpots++;
		}
	}
	if( !numSpots )
	{
		spot = G_Find( NULL, FOFS(classname_), "info_player_deathmatch");
		if( !spot )
			Com_Error( ERR_DROP, "Couldn't find a spawn point" );
		VectorCopy( spot->s.origin, origin );
		origin[2] += 9;
		VectorCopy( spot->s.angles, angles );
		return spot;
	}

	// select a random spot from the spawn points furthest away
	rnd = random() * (numSpots / 2);

	VectorCopy( list_spot[rnd]->s.origin, origin );
	origin[2] += 9;
	VectorCopy( list_spot[rnd]->s.angles, angles );

	return list_spot[rnd];
}

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, etc
============
*/
GameEntity *SelectSpawnPoint( vec3_t avoidPoint, vec3_t origin, vec3_t angles, int category ) 
{
	return SelectRandomFurthestSpawnPoint( avoidPoint, origin, angles, category );

	/*
	gentity_t	*spot;
	gentity_t	*nearestSpot;

	nearestSpot = SelectNearestDeathmatchSpawnPoint( avoidPoint );

	spot = SelectRandomDeathmatchSpawnPoint ( );
	if ( spot == nearestSpot ) {
		// roll again if it would be real close to point of death
		spot = SelectRandomDeathmatchSpawnPoint ( );
		if ( spot == nearestSpot ) {
			// last try
			spot = SelectRandomDeathmatchSpawnPoint ( );
		}		
	}

	// find a single player start spot
	if (!spot) {
		G_Error( "Couldn't find a spawn point" );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
	*/
}

/*
===========
SelectInitialSpawnPoint

Try to find a spawn point marked 'initial', otherwise
use normal spawn selection.
============
*/
GameEntity *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles ) 
{
	GameEntity	*spot;

	spot = NULL;
	while( (spot = G_Find (spot, FOFS(classname_), "info_player_deathmatch")) != NULL )
	{
		if( spot->spawnflags_ & 1 ) 
			break;
	}

	if( !spot || SpotWouldTelefrag( spot ) ) 
		return SelectSpawnPoint( vec3_origin, origin, angles, -1 );

	VectorCopy( spot->s.origin, origin );
	origin[2] += 9;
	VectorCopy( spot->s.angles, angles );

	return spot;
}

/*
===========
SelectSpectatorSpawnPoint

============
*/
GameEntity *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles ) 
{
	FindIntermissionPoint();

	VectorCopy( theLevel.intermission_origin_, origin );
	VectorCopy( theLevel.intermission_angle_, angles );

	return NULL;
}

//======================================================================


/*
==================
SetClientViewAngle

==================
*/
void SetClientViewAngle( GameEntity *ent, vec3_t angle ) 
{
	int			i;

	// set the delta angle
	for( i=0 ; i<3 ; i++ )
	{
		int		cmdAngle;

		cmdAngle = ANGLE2SHORT(angle[i]);
		ent->client_->ps_.delta_angles[i] = cmdAngle - ent->client_->pers_.cmd_.angles[i];
	}
	VectorCopy( angle, ent->s.angles );
	VectorCopy( ent->s.angles, ent->client_->ps_.viewangles );
}

/*
================
respawn
================
*/
void respawn( GameEntity *ent ) 
{
	int clientNum = ent->s.number;

	// delete old entity
	theLevel.removeEntity( ent );

	// spawn new one
	MF_ClientSpawn( clientNum, 0 );
}

/*
================
switch_vehicle
================
*/
void switch_vehicle( GameEntity *ent ) 
{
	int clientNum = ent->s.number;

	// delete old entity
	theLevel.removeEntity( ent );

	// spawn new one
	MF_ClientSpawn( clientNum, CS_NOKILL | CS_LASTPOS );
}

/*
================
TeamCount

Returns number of players on a team
================
*/
int TeamCount( int ignoreClientNum, int team ) 
{
	int		count = 0;
	for( int i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		if( i == ignoreClientNum ) 
			continue;
	
		GameClient* cl = theLevel.getClient(i);
		if( !cl || cl->pers_.connected_ == GameClient::ClientPersistant::CON_DISCONNECTED ) 
			continue;
		
		if( cl->sess_.sessionTeam_ == team ) 
			count++;
	}

	//return static_cast<team_t>(count);
	return count;
}

/*
================
TeamLeader

Returns the client number of the team leader
================
*/
int TeamLeader( int team ) 
{
	int		i;

	for ( i = 0 ; i < theLevel.maxclients_ ; i++ ) 
	{
		GameClient* cl = theLevel.getClient(i);
		if( !cl || cl->pers_.connected_ == GameClient::ClientPersistant::CON_DISCONNECTED ) 
			continue;
		if( cl->sess_.sessionTeam_ == team ) 
		{
			if( cl->sess_.teamLeader_ )
				return i;
		}
	}

	return -1;
}


/*
================
PickTeam

================
*/
ClientBase::eTeam PickTeam( int ignoreClientNum )
{
	int	counts[ClientBase::TEAM_NUM_TEAMS];

	// find out how many players are on each team
	counts[ClientBase::TEAM_BLUE] = TeamCount( ignoreClientNum, ClientBase::TEAM_BLUE );
	counts[ClientBase::TEAM_RED] = TeamCount( ignoreClientNum, ClientBase::TEAM_RED );

	// more on blue team?
	if( counts[ClientBase::TEAM_BLUE] > counts[ClientBase::TEAM_RED] )
	{
		// join red
		return ClientBase::TEAM_RED;
	}
	
	// more on red team?
	if( counts[ClientBase::TEAM_RED] > counts[ClientBase::TEAM_BLUE] )
	{
		// join blue
		return ClientBase::TEAM_BLUE;
	}
	
	// equal team count, so equal score?
	if( theLevel.teamScores_[ClientBase::TEAM_BLUE] == theLevel.teamScores_[ClientBase::TEAM_RED] )
	{
		// scores equal, just pick a random team (change random number generation if ever more than two teams)
		if( rand() & 0x01 )
		{
			// join red
			return ClientBase::TEAM_RED;
		}
		else
		{
			// join blue
			return ClientBase::TEAM_BLUE;
		}
	}
	// unequal scores, so join the team with the lowest score
	else if( theLevel.teamScores_[ClientBase::TEAM_BLUE] > theLevel.teamScores_[ClientBase::TEAM_RED] )
	{
		// join red, blue in the lead
		return ClientBase::TEAM_RED;
	}

	// join blue, red in the lead
	return ClientBase::TEAM_BLUE;
}

/*
===========
ClientCheckName
============
*/
static void ClientCleanName( const char *in, char *out, int outSize ) {
	int		len, colorlessLen;
	char	ch;
	char	*p;
	int		spaces;

	//save room for trailing null byte
	outSize--;

	len = 0;
	colorlessLen = 0;
	p = out;
	*p = 0;
	spaces = 0;

	while( 1 ) {
		ch = *in++;
		if( !ch ) {
			break;
		}

		// don't allow leading spaces
		if( !*p && ch == ' ' ) {
			continue;
		}

		// check colors
		if( ch == Q_COLOR_ESCAPE ) {
			// solo trailing carat is not a color prefix
			if( !*in ) {
				break;
			}

			// don't allow black in a name, period
			if( ColorIndex(*in) == 0 ) {
				in++;
				continue;
			}

			// make sure room in dest for both chars
			if( len > outSize - 2 ) {
				break;
			}

			*out++ = ch;
			*out++ = *in++;
			len += 2;
			continue;
		}

		// don't allow too many consecutive spaces
		if( ch == ' ' ) {
			spaces++;
			if( spaces > 3 ) {
				continue;
			}
		}
		else {
			spaces = 0;
		}

		if( len > outSize - 1 ) {
			break;
		}

		*out++ = ch;
		colorlessLen++;
		len++;
	}
	*out = 0;

	// don't allow empty names
	if( *p == 0 || colorlessLen == 0 ) {
		Q_strncpyz( p, "UnnamedPlayer", outSize );
	}
}


/*
===========
ClientUserInfoChanged

Called from ClientConnect when the player first connects and
directly by the server system when the player updates a userinfo variable.

The game can override any of the settings and call SetUserinfo
if desired.
============
*/
void ClientUserinfoChanged( int clientNum )
{
	//GameEntity* ent = theLevel.getEntity(clientNum);
	GameClient* client = theLevel.getClient(clientNum);

	char	userinfo[MAX_INFO_STRING];
	SV_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check for malformed or illegal info strings
	if( !Info_Validate(userinfo) ) 
		strcpy (userinfo, "\\name\\badinfo");

	// check for local client
	char* s = Info_ValueForKey( userinfo, "ip" );
	if( !strcmp( s, "localhost" ) ) 
		client->pers_.localClient_ = true;

	// check the item prediction
	s = Info_ValueForKey( userinfo, "cg_predictItems" );
	if( !atoi( s ) ) 
		client->pers_.predictItemPickup_ = false;
	else 
		client->pers_.predictItemPickup_ = true;

	// set name
	char	oldname[MAX_STRING_CHARS];
	Q_strncpyz ( oldname, client->pers_.netname_, sizeof( oldname ) );
	s = Info_ValueForKey( userinfo, "name" );
	ClientCleanName( s, client->pers_.netname_, sizeof(client->pers_.netname_) );

	if( client->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		if( client->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_SCOREBOARD ) 
			Q_strncpyz( client->pers_.netname_, "scoreboard", sizeof(client->pers_.netname_) );
	}

	if( client->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED ) 
	{
		if( strcmp( oldname, client->pers_.netname_ ) ) 
		{
			SV_GameSendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " renamed to %s\n\"", oldname, 
				client->pers_.netname_) );
		}
	}

	int		nextVehicle;

	// For changing vehicle
	if( (client->ps_.pm_flags & PMF_RECHARGING) && (client->ps_.objectives != OB_REDFLAG) && (client->ps_.objectives != OB_BLUEFLAG)) 
	{
		nextVehicle = atoi( Info_ValueForKey( userinfo, "cg_nextVehicle" ) );	
		if( nextVehicle != client->nextVehicle_ ) 
		{
			client->ps_.pm_flags &= ~PMF_VEHICLESELECT;
			client->ps_.pm_flags |= PMF_VEHICLESPAWN;
		}
	}
	// mfq3 - vehicle
	int		vehicle;
	//if( !(ent->r.svFlags & SVF_BOT) )
	//{
		// get the next-vehicle value
		nextVehicle = atoi( Info_ValueForKey( userinfo, "cg_nextVehicle" ) );	
		client->nextVehicle_ = nextVehicle;
		
		// if next-vehicle is invalid, set the current vehicle to invalid also
		if( nextVehicle == -1 )
		{
			// current vehicle no longer valid
			vehicle = -1;
			client->vehicle_ = vehicle;
		}
		else
		{
			// get current vehicle value
			vehicle = atoi( Info_ValueForKey( userinfo, "cg_vehicle" ) );
			client->vehicle_ = vehicle;
		}

		if( nextVehicle >= 0 && (client->ps_.pm_flags & PMF_VEHICLESELECT) )
		{
			client->ps_.pm_flags &= ~PMF_VEHICLESELECT;
			client->ps_.pm_flags |= PMF_VEHICLESPAWN;
		}
	//}
	//else
	//{
	//	// no vehicle info is valid
	//	client->vehicle_ = client->nextVehicle_ = vehicle = nextVehicle = -1;
	//}

	client->advancedControls_ = atoi( Info_ValueForKey( userinfo, "cg_advanced" ) );

	// set max health
	int health = atoi( Info_ValueForKey( userinfo, "handicap" ) );
	//if( (ent->r.svFlags & SVF_BOT) || vehicle < 0 ) 
	//{
	//	client->pers_.maxHealth_ = health;
	//	if( client->pers_.maxHealth_ < 1 || client->pers_.maxHealth_ > 100 ) 
	//		client->pers_.maxHealth_ = 100;
	//}
	//else 
	//{
		if ( health < 1 || health > 100 ) 
			health = 100;
		client->pers_.maxHealth_ = availableVehicles[vehicle].maxhealth * health / 100;
	//}
	client->ps_.stats[STAT_MAX_HEALTH] = client->pers_.maxHealth_;

	// bots set their team a few frames later
	int team;
	if( g_gametype.integer >= GT_TEAM && 
		(theLevel.getEntity(clientNum)->r.svFlags & SVF_BOT) )
	{
		s = Info_ValueForKey( userinfo, "team" );
		if( !Q_stricmp( s, "red" ) || !Q_stricmp( s, "r" ) ) 
			team = ClientBase::TEAM_RED;
		else if ( !Q_stricmp( s, "blue" ) || !Q_stricmp( s, "b" ) ) 
			team = ClientBase::TEAM_BLUE;
		else 
		{
			// pick the team with the least number of players
			team = PickTeam( clientNum );
		}
	}
	else 
		team = client->sess_.sessionTeam_;

	// teamInfo
	s = Info_ValueForKey( userinfo, "teamoverlay" );
	if( ! *s || atoi( s ) != 0 ) 
		client->pers_.teamInfo_ = true;
	else 
		client->pers_.teamInfo_ = false;

	// team task (0 = none, 1 = offence, 2 = defence)
	int teamTask = atoi(Info_ValueForKey(userinfo, "teamtask"));
	// team Leader (1 = leader, 0 is normal player)
	int teamLeader = client->sess_.teamLeader_;

	// colors
	char	c1[MAX_INFO_STRING];
	char	redTeam[MAX_INFO_STRING];
	char	blueTeam[MAX_INFO_STRING];
	strcpy(c1, Info_ValueForKey( userinfo, "color" ));
	strcpy(redTeam, Info_ValueForKey( userinfo, "g_redteam" ));
	strcpy(blueTeam, Info_ValueForKey( userinfo, "g_blueteam" ));

	// send over a subset of the userinfo keys so other clients can
	// print scoreboards, display models, and play custom sounds
	//if( ent->r.svFlags & SVF_BOT ) 
	//{
	//	s = va("n\\%s\\t\\%i\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\skill\\%s\\tt\\%d\\tl\\%d\\v\\%d\\x\\%d\\ac\\%d",
	//		client->pers_.netname_, client->sess_.sessionTeam_, c1,
	//		client->pers_.maxHealth_, client->sess_.wins_, client->sess_.losses_,
	//		Info_ValueForKey( userinfo, "skill" ), teamTask, teamLeader,
	//		-1, -1, 0 );	// MFQ3: for now no vehicle and simple controls
	//} 
	//else 
	//{
		s = va("n\\%s\\t\\%i\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d\\v\\%d\\x\\%d\\ac\\%d",
			client->pers_.netname_, client->sess_.sessionTeam_, redTeam, blueTeam, c1,
			client->pers_.maxHealth_, client->sess_.wins_, client->sess_.losses_, teamTask, teamLeader,
			vehicle, nextVehicle, client->advancedControls_ ); // MFQ3: send current/next vehicle indexes
	//}

	SV_SetConfigstring( CS_PLAYERS+clientNum, s );

	G_LogPrintf( "ClientUserinfoChanged: %i %s\n", clientNum, s );
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.

This should NOT be called directly by any game logic,
call SV_GameDropClient(), which will call this and do
server system housekeeping.
============
*/
void ClientDisconnect( int clientNum ) 
{
	GameEntity* ent = theLevel.getEntity(clientNum);
	if( !ent || !ent->client_ ) 
		return;

	// stop any following clients
	for( int i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		GameClient* cl = theLevel.getClient(i);
		if( cl && cl->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR &&
			cl->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_FOLLOW &&
			cl->sess_.spectatorClient_ == clientNum ) 
		{
			StopFollowing( theLevel.getEntity(i) );
		}
	}

	// send effect if they were completely connected
	if( ent->client_->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED &&
		ent->client_->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
	{
		// They don't get to take powerups with them!
		// Especially important for stuff like CTF flags
		TossVehicleFlags( ent );
	}

	G_LogPrintf( "ClientDisconnect: %i\n", clientNum );

	// if we are playing in tourney mode and losing, give a win to the other player
	if( (g_gametype.integer == GT_TOURNAMENT ) &&
		!theLevel.intermissiontime_ &&
		!theLevel.warmupTime_ && 
		theLevel.sortedClients_[1] == clientNum ) 
	{
		theLevel.getClient(theLevel.sortedClients_[0])->sess_.wins_++;
		ClientUserinfoChanged( theLevel.sortedClients_[0] );
	}

	SV_UnlinkEntity( ent );
	ent->s.modelindex = 0;
	ent->inuse_ = false;
	ent->classname_ = "disconnected";
	ent->client_->pers_.connected_ = GameClient::ClientPersistant::CON_DISCONNECTED;
	ent->client_->ps_.persistant[PERS_TEAM] = ClientBase::TEAM_FREE;
	ent->client_->sess_.sessionTeam_ = ClientBase::TEAM_FREE;

	SV_SetConfigstring( CS_PLAYERS + clientNum, "");

	CalculateRanks();

}


