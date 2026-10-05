/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "server.h"
#include "sv_client.h"
#include "sv_server.h"
#include "../game/game.h"

/*
===============================================================================

OPERATOR CONSOLE ONLY COMMANDS

These commands can only be entered from stdin or by a remote operator datagram
===============================================================================
*/


/*
==================
ServerMain::getPlayerByHandle

Returns the player with player id or name from CommandSystem::argv(1)
==================
*/
SV_Client * ServerMain::getPlayerByHandle( ) {
	SV_Client	*cl;
	int			i;
	char		*s;
	char		cleanName[64];

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		return nullptr;
	}

	if ( CommandSystem::argc() < 2 ) {
		Common::printf( "No player specified.\n" );
		return nullptr;
	}

	s = CommandSystem::argv(1);

	// Check whether this is a numeric player handle
	for(i = 0; s[i] >= '0' && s[i] <= '9'; i++);
	
	if(!s[i])
	{
		int plid = atoi(s);

		// Check for numeric playerid match
		if(plid >= 1 && plid <= sv_maxclients->asInt())
		{
			cl = theSVS.svClients_.at(plid);
			
			if(cl && cl->state_)
				return cl;
		}
	}

	// check for a name match
	for ( i=1 ; i <= sv_maxclients->asInt() ; i++ ) {
		cl = theSVS.svClients_.at(i);
		if ( !cl )
			continue;
		if ( !cl->state_ ) {
			continue;
		}
		if ( !Q_stricmp( cl->name_, s ) ) {
			return cl;
		}

		Q_strncpyz( cleanName, cl->name_, sizeof(cleanName) );
		Q_CleanStr( cleanName );
		if ( !Q_stricmp( cleanName, s ) ) {
			return cl;
		}
	}

	Common::printf( "Player %s is not on the server\n", s );

	return nullptr;
}

/*
==================
ServerMain::getPlayerByNum

Returns the player with idnum from CommandSystem::argv(1)
==================
*/
SV_Client * ServerMain::getPlayerByNum( ) {
	SV_Client	*cl;
	int			i;
	int			idnum;
	char		*s;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		return nullptr;
	}

	if ( CommandSystem::argc() < 2 ) {
		Common::printf( "No player specified.\n" );
		return nullptr;
	}

	s = CommandSystem::argv(1);

	for (i = 0; s[i]; i++) {
		if (s[i] < '0' || s[i] > '9') {
			Common::printf( "Bad slot number: %s\n", s);
			return nullptr;
		}
	}
	idnum = atoi( s );
	if ( idnum < 1 || idnum > sv_maxclients->asInt() ) {
		Common::printf( "Bad client slot: %i\n", idnum );
		return nullptr;
	}

	cl = theSVS.svClients_.at(idnum);
	if ( !cl || !cl->state_ ) {
		Common::printf( "Client %i is not active\n", idnum );
		return nullptr;
	}
	return cl;
}

//=========================================================


/*
==================
ServerMain::map_f

Restart the server on a different map
==================
*/
void ServerMain::map_f() 
{
	char		*cmd;
	char		*map;
	bool		killBots, cheat;
	char		expanded[MAX_QPATH];
	char		mapname[MAX_QPATH];

	map = CommandSystem::argv(1);
	if ( !map ) 
		return;

	// make sure the level exists before trying to change, so that
	// a typo at the server console won't end the game
	Com_sprintf (expanded, sizeof(expanded), "maps/%s.bsp", map);
	if ( FileSystem::readFile (expanded, nullptr) == -1 ) 
	{
		Common::printf ("Can't find map %s\n", expanded);
		return;
	}

	// force latched values to get set
	CvarSystem::get ("g_gametype", "0", CVAR_SERVERINFO | CVAR_USERINFO | CVAR_LATCH );

	cmd = CommandSystem::argv(0);
	if( Q_stricmpn( cmd, "sp", 2 ) == 0 ) 
	{
		CvarSystem::setValue( "g_gametype", GT_SINGLE_PLAYER );
		CvarSystem::setValue( "g_doWarmup", 0 );
		// may not set sv_maxclients directly, always set latched
		CvarSystem::setLatched( "sv_maxclients", "8" );
		cmd += 2;
		if (!Q_stricmp( cmd, "devmap" ) ) {
			cheat = true;
		} else {
			cheat = false;
		}
		killBots = true;
	}
	else 
	{
		if ( !Q_stricmp( cmd, "devmap" ) ) 
		{
			cheat = true;
			killBots = true;
		} 
		else 
		{
			cheat = false;
			killBots = false;
		}
		if( sv_gametype->asInt() == GT_SINGLE_PLAYER )
			CvarSystem::setValue( "g_gametype", GT_FFA );
	}

	// save the map name here cause on a map restart we reload the mfconfig.cfg
	// and thus nuke the arguments of the map command
	Q_strncpyz(mapname, map, sizeof(mapname));

	// start up the map
	ServerMain::spawnServer( mapname, killBots );

	// set the cheat value
	// if the level was started with "map <levelname>", then
	// cheats will not be allowed.  If started with "devmap <levelname>"
	// then cheats will be allowed
	if ( cheat ) 
		CvarSystem::set( "sv_cheats", "1" );
	else
		CvarSystem::set( "sv_cheats", "0" );
}

/*
================
ServerMain::mapRestart_f

Completely restarts a level, but doesn't send a new gamestate to the clients.
This allows fair starts with variable load times.
================
*/
void ServerMain::mapRestart_f() 
{
	// make sure we aren't restarting twice in the same frame
	if( com_frameTime == theSV.serverId_ ) 
		return;

	// make sure server is running
	if( !com_sv_running->asInt() )
	{
		Common::printf( "Server is not running.\n" );
		return;
	}

	if( theSV.restartTime_ ) 
		return;

	int	delay;
	if (CommandSystem::argc() > 1 ) 
		delay = atoi( CommandSystem::argv(1) );
	else 
		delay = 5;

	if( delay && !CvarSystem::variableValue("g_doWarmup") ) 
	{
		theSV.restartTime_ = theSV.time_ + delay * 1000;
		GameHost::setConfigstring( CS_WARMUP, va("%i", theSV.restartTime_) );
		return;
	}

	// check for changes in variables that can't just be restarted
	// check for maxclients change
	if ( sv_maxclients->modified || sv_gametype->modified ) 
	{
		char	mapname[MAX_QPATH];

		Common::printf( "variable change -- restarting.\n" );
		// restart the map the slow way
		Q_strncpyz( mapname, CvarSystem::variableString( "mapname" ), sizeof( mapname ) );

		ServerMain::spawnServer( mapname, false );
		return;
	}

	// toggle the server bit so clients can detect that a
	// map_restart has happened
	theSVS.snapFlagServerBit_ ^= SNAPFLAG_SERVERCOUNT;

	// generate a New serverid	
	// TTimo - don't update restartedserverId there, otherwise we won't deal correctly with multiple map_restart
	theSV.serverId_ = com_frameTime;
	CvarSystem::set( "sv_serverid", va("%i", theSV.serverId_ ) );

	// if a map_restart occurs while a client is changing maps, we need
	// to give them the correct time so that when they finish loading
	// they don't violate the backwards time check in cl_cgame.c
	for( int i = 1; i <= sv_maxclients->asInt() ; i++ ) 
	{
		SV_Client* client = theSVS.svClients_.at(i);
		if( client && client->state_ == SV_Client::CS_PRIMED ) 
			client->oldServerTime_ = theSV.restartTime_;
	}

	// reset all the vm data in place without changing memory allocation
	// note that we do NOT set sv.state = SS_LOADING, so configstrings that
	// had been changed from their default values will generate broadcast updates
	theSV.state_ = Server::SS_LOADING;
	theSV.restarting_ = true;

	ServerMain::restartGameProgs();

	// run a few frames to allow everything to settle
	for( int i = 0 ;i < 3 ; i++ ) 
	{
		//VM_Call( gvm, GAME_RUN_FRAME, svs.time );
		theSG.gameRunFrame( theSV.time_ );
		theSV.time_ += 100;
		theSVS.time_ += 100;
	}

	theSV.state_ = Server::SS_GAME;
	theSV.restarting_ = false;

	char *denied;
	bool isBot;
	// connect and begin all the clients
	for( int i = 1; i <= sv_maxclients->asInt() ; i++ ) 
	{
		SV_Client* client = theSVS.svClients_.at(i);

		// send the New gamestate to all connected clients
		if( !client || client->state_ < SV_Client::CS_CONNECTED)
			continue;

		if( client->netchan_.remoteAddress().type == NA_BOT ) 
			isBot = true;
		else 
			isBot = false;

		// add the map_restart command
		client->addServerCommand( "map_restart\n" );

		// connect the client again, without the firstTime flag
		//denied = reinterpret_cast<char*>(VM_ExplicitArgPtr( gvm, VM_Call( gvm, GAME_CLIENT_CONNECT, i, false, isBot ) ));
		denied = theSG.clientConnect( i, false, isBot );
		if ( denied )
		{
			// this generally shouldn't happen, because the client
			// was connected before the level change
			client->drop( denied );
			Common::printf( "SV_MapRestart_f(%d): dropped client %i - denied!\n", delay, i ); // bk010125
			continue;
		}

		if(client->state_ == SV_Client::CS_ACTIVE)
			client->enterWorld( &client->lastUsercmd_ );
		else
		{
			// If we don't reset client->lastUsercmd and are restarting during map load,
			// the client will hang because we'll use the last Usercmd from the previous map,
			// which is wrong obviously.
			client->enterWorld( nullptr );
		}
	}	

	// run another frame to allow things to look at all the players
	//VM_Call( gvm, GAME_RUN_FRAME, svs.time );
	theSG.gameRunFrame( theSV.time_ );
	theSV.time_ += 100;
	theSVS.time_ += 100;
}

//===============================================================

/*
==================
ServerMain::kick_f

Kick a user off of the server
==================
*/
void ServerMain::kick_f( ) {
	SV_Client	*cl;
	int			i;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: kick <player name>\nkick all = kick everyone\nkick allbots = kick all bots\n");
		return;
	}

	cl = ServerMain::getPlayerByHandle();
	if ( !cl ) {
		if ( !Q_stricmp(CommandSystem::argv(1), "all") ) {
			for ( i=1 ; i <= sv_maxclients->asInt() ; i++ ) {
				cl = theSVS.svClients_.at(i);
				if ( !cl )
					continue;
				if ( !cl->state_ ) {
					continue;
				}
				if( cl->netchan_.remoteAddress().type == NA_LOOPBACK ) {
					continue;
				}
				cl->drop( "was kicked" );
				cl->lastPacketTime_ = theSVS.time_;	// in case there is a funny zombie
			}
		}
		else if ( !Q_stricmp(CommandSystem::argv(1), "allbots") ) {
			for ( i=1 ; i <= sv_maxclients->asInt() ; i++ ) {
				cl = theSVS.svClients_.at(i);
				if ( !cl )
					continue;
				if ( !cl->state_ ) {
					continue;
				}
				if( cl->netchan_.remoteAddress().type != NA_BOT ) {
					continue;
				}
				cl->drop( "was kicked" );
				cl->lastPacketTime_ = theSVS.time_;	// in case there is a funny zombie
			}
		}
		return;
	}
	if( cl->netchan_.remoteAddress().type == NA_LOOPBACK ) {
		Common::printf("Cannot kick host player\n");
		return;
	}

	cl->drop( "was kicked" );
	cl->lastPacketTime_ = theSVS.time_;	// in case there is a funny zombie
}

/*
==================
ServerMain::traceBench_f

times a fixed set of world traces and prints a checksum of the results,
so trace optimisations can be measured and checked for identical output
==================
*/
void ServerMain::traceBench_f()
{
	if( !com_sv_running->asInt() )
	{
		Common::printf( "No map loaded.\n" );
		return;
	}
	const int count = CommandSystem::argc() > 1 ? atoi( CommandSystem::argv( 1 ) ) : 20000;
	vec3_t worldMins, worldMaxs;
	CollisionModel::modelBounds( 0, worldMins, worldMaxs );
	unsigned int seed = 12345;
	const auto next = [&seed]() { seed = seed * 1664525u + 1013904223u; return (seed >> 8) / 16777216.0f; };
	struct Kind { const char* name; float halfWidth; float length; };
	const Kind kinds[] = { { "point 900", 0.0f, 900.0f }, { "box60 900", 60.0f, 900.0f }, { "box28 160", 28.0f, 160.0f }, { "box60 place", 60.0f, 0.0f } };
	for( const Kind& kind : kinds )
	{
		double checksum = 0, distances = 0;
		int solid = 0;
		int hits = 0;
		const int startTime = System::milliseconds();
		for( int i = 0; i < count; i++ )
		{
			vec3_t start, end, dir;
			for( int k = 0; k < 3; k++ )
			{
				start[k] = worldMins[k] + next() * (worldMaxs[k] - worldMins[k]);
				dir[k] = next() * 2.0f - 1.0f;
			}
			VectorNormalize( dir );
			VectorMA( start, kind.length, dir, end );
			vec3_t mins = { -kind.halfWidth, -kind.halfWidth, -kind.halfWidth * 0.4f }, maxs = { kind.halfWidth, kind.halfWidth, kind.halfWidth * 0.4f };
			trace_t tr;
			CollisionModel::boxTrace( &tr, start, end, mins, maxs, 0, MASK_PLAYERSOLID, false );
			checksum += tr.fraction + (tr.startsolid ? 2.0 : 0.0) + tr.plane.normal[2] * 0.25;
			distances += tr.fraction;
			if( tr.startsolid )
				solid++;
			if( tr.fraction < 1.0f )
				hits++;
		}
		const int elapsed = System::milliseconds() - startTime;
		Common::printf( "TRACEBENCH %s: %d traces %d ms (%.2f us each) hits %d startsolid %d fractions %.6f checksum %.6f\n", kind.name, count, elapsed, elapsed * 1000.0 / count, hits, solid, distances, checksum );
	}
}

/*
==================
ServerMain::kickBots_f

Kick all bots off of the server
==================
*/
void ServerMain::kickBots_f( ) {
	SV_Client	*cl;
	int			i;

	// make sure server is running
	if( !com_sv_running->asInt() ) {
		Common::printf("Server is not running.\n");
		return;
	}

	for ( i=1 ; i <= sv_maxclients->asInt() ; i++ ) {
		cl = theSVS.svClients_.at(i);
		if ( !cl )
			continue;
		if( !cl->state_ ) {
			continue;
		}

		if( cl->netchan_.remoteAddress().type != NA_BOT ) {
			continue;
		}

		cl->drop( "was kicked" );
		cl->lastPacketTime_ = theSVS.time_; // in case there is a funny zombie
	}
}
/*
==================
ServerMain::kickAll_f

Kick all users off of the server
==================
*/
void ServerMain::kickAll_f( ) {
	SV_Client *cl;
	int i;

	// make sure server is running
	if( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	for ( i=1 ; i <= sv_maxclients->asInt() ; i++ ) {
		cl = theSVS.svClients_.at(i);
		if ( !cl )
			continue;
		if( !cl->state_ ) {
			continue;
		}

		if( cl->netchan_.remoteAddress().type == NA_LOOPBACK ) {
			continue;
		}

		cl->drop( "was kicked" );
		cl->lastPacketTime_ = theSVS.time_; // in case there is a funny zombie
	}
}

/*
==================
ServerMain::kickNum_f

Kick a user off of the server
==================
*/
void ServerMain::kickNum_f( ) {
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: %s <client number>\n", CommandSystem::argv(0));
		return;
	}

	cl = ServerMain::getPlayerByNum();
	if ( !cl ) {
		return;
	}
	if( cl->netchan_.remoteAddress().type == NA_LOOPBACK ) {
		Common::printf("Cannot kick host player\n");
		return;
	}

	cl->drop( "was kicked" );
	cl->lastPacketTime_ = theSVS.time_;	// in case there is a funny zombie
}

#ifndef STANDALONE
// these functions require the auth server which of course is not available anymore for stand-alone games.

/*
==================
ServerMain::ban_f

Ban a user from being able to play on this server through the auth
server
==================
*/
void ServerMain::ban_f( void ) {
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: banUser <player name>\n");
		return;
	}

	cl = ServerMain::getPlayerByHandle();

	if (!cl) {
		return;
	}

	if( cl->netchan_.remoteAddress().type == NA_LOOPBACK ) {
		Common::printf("Cannot kick host player\n");
		return;
	}

	// look up the authorize server's IP
	if ( !theSVS.authorizeAddress_.ip[0] && theSVS.authorizeAddress_.type != NA_BAD ) {
		Common::printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !Network::stringToAdr( AUTHORIZE_SERVER_NAME, &theSVS.authorizeAddress_, NA_IP ) ) {
			Common::printf( "Couldn't resolve address\n" );
			return;
		}
		theSVS.authorizeAddress_.port = BigShort( PORT_AUTHORIZE );
		Common::printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
			theSVS.authorizeAddress_.ip[0], theSVS.authorizeAddress_.ip[1],
			theSVS.authorizeAddress_.ip[2], theSVS.authorizeAddress_.ip[3],
			BigShort( theSVS.authorizeAddress_.port ) );
	}

	// otherwise send their ip to the authorize server
	if ( theSVS.authorizeAddress_.type != NA_BAD ) {
		Network::outOfBandPrint( NS_SERVER, theSVS.authorizeAddress_,
			"banUser %i.%i.%i.%i", cl->netchan_.remoteAddress().ip[0], cl->netchan_.remoteAddress().ip[1], 
								   cl->netchan_.remoteAddress().ip[2], cl->netchan_.remoteAddress().ip[3] );
		Common::printf("%s was banned from coming back\n", cl->name_);
	}
}

/*
==================
ServerMain::banNum_f

Ban a user from being able to play on this server through the auth
server
==================
*/
void ServerMain::banNum_f( void ) {
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: banClient <client number>\n");
		return;
	}

	cl = ServerMain::getPlayerByNum();
	if ( !cl ) {
		return;
	}
	if( cl->netchan_.remoteAddress().type == NA_LOOPBACK ) {
		Common::printf("Cannot kick host player\n");
		return;
	}

	// look up the authorize server's IP
	if ( !theSVS.authorizeAddress_.ip[0] && theSVS.authorizeAddress_.type != NA_BAD ) {
		Common::printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !Network::stringToAdr( AUTHORIZE_SERVER_NAME, &theSVS.authorizeAddress_, NA_IP ) ) {
			Common::printf( "Couldn't resolve address\n" );
			return;
		}
		theSVS.authorizeAddress_.port = BigShort( PORT_AUTHORIZE );
		Common::printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
			theSVS.authorizeAddress_.ip[0], theSVS.authorizeAddress_.ip[1],
			theSVS.authorizeAddress_.ip[2], theSVS.authorizeAddress_.ip[3],
			BigShort( theSVS.authorizeAddress_.port ) );
	}

	// otherwise send their ip to the authorize server
	if ( theSVS.authorizeAddress_.type != NA_BAD ) {
		Network::outOfBandPrint( NS_SERVER, theSVS.authorizeAddress_,
			"banUser %i.%i.%i.%i", cl->netchan_.remoteAddress().ip[0], cl->netchan_.remoteAddress().ip[1], 
								   cl->netchan_.remoteAddress().ip[2], cl->netchan_.remoteAddress().ip[3] );
		Common::printf("%s was banned from coming back\n", cl->name_);
	}
}
#endif

/*
==================
ServerMain::rehashBans_f

Load saved bans from file.
==================
*/
void ServerMain::rehashBans_f()
{
	int index, filelen;
	fileHandle_t readfrom;
	char *textbuf, *curpos, *maskpos, *newlinepos, *endpos;
	char filepath[MAX_QPATH];
	
	serverBansCount = 0;
	
	if(!sv_banFile->asString() || !*sv_banFile->asString())
		return;

	Com_sprintf(filepath, sizeof(filepath), "%s/%s", FileSystem::getCurrentGameDir(), sv_banFile->asString());

	if((filelen = FileSystem::baseDir_FOpenFileRead(filepath, &readfrom)) >= 0)
	{
		if(filelen < 2)
		{
			// Don't bother if file is too short.
			FileSystem::fCloseFile(readfrom);
			return;
		}

		curpos = textbuf = static_cast<char *>(Zone::malloc(filelen));
		
		filelen = FileSystem::read(textbuf, filelen, readfrom);
		FileSystem::fCloseFile(readfrom);
		
		endpos = textbuf + filelen;
		
		for(index = 0; index < SERVER_MAXBANS && curpos + 2 < endpos; index++)
		{
			// find the end of the address string
			for(maskpos = curpos + 2; maskpos < endpos && *maskpos != ' '; maskpos++);
			
			if(maskpos + 1 >= endpos)
				break;

			*maskpos = '\0';
			maskpos++;
			
			// find the end of the subnet specifier
			for(newlinepos = maskpos; newlinepos < endpos && *newlinepos != '\n'; newlinepos++);
			
			if(newlinepos >= endpos)
				break;
			
			*newlinepos = '\0';
			
			if(Network::stringToAdr(curpos + 2, &serverBans[index].ip, NA_UNSPEC))
			{
				serverBans[index].isexception = (curpos[0] != '0');
				serverBans[index].subnet = atoi(maskpos);
				
				if(serverBans[index].ip.type == NA_IP &&
				   (serverBans[index].subnet < 1 || serverBans[index].subnet > 32))
				{
					serverBans[index].subnet = 32;
				}
				else if(serverBans[index].ip.type == NA_IP6 &&
					(serverBans[index].subnet < 1 || serverBans[index].subnet > 128))
				{
					serverBans[index].subnet = 128;
				}
			}
			
			curpos = newlinepos + 1;
		}
			
		serverBansCount = index;
		
		Zone::free(textbuf);
	}
}

/*
==================
ServerMain::writeBans

Save bans to file.
==================
*/
void ServerMain::writeBans()
{
	int index;
	fileHandle_t writeto;
	char filepath[MAX_QPATH];
	
	if(!sv_banFile->asString() || !*sv_banFile->asString())
		return;
	
	Com_sprintf(filepath, sizeof(filepath), "%s/%s", FileSystem::getCurrentGameDir(), sv_banFile->asString());

	if((writeto = FileSystem::baseDir_FOpenFileWrite_HomeState(filepath)))
	{
		char writebuf[128];
		serverBan_t *curban;
		
		for(index = 0; index < serverBansCount; index++)
		{
			curban = &serverBans[index];
			
			Com_sprintf(writebuf, sizeof(writebuf), "%d %s %d\n",
				    curban->isexception, Network::adrToString(curban->ip), curban->subnet);
			FileSystem::write(writebuf, strlen(writebuf), writeto);
		}

		FileSystem::fCloseFile(writeto);
	}
}

/*
==================
ServerMain::delBanEntryFromList

Remove a ban or an exception from the list.
==================
*/

bool ServerMain::delBanEntryFromList(int index)
{
	if(index == serverBansCount - 1)
		serverBansCount--;
	else if(index < ARRAY_LEN(serverBans) - 1)
	{
		memmove(serverBans + index, serverBans + index + 1, (serverBansCount - index - 1) * sizeof(*serverBans));
		serverBansCount--;
	}
	else
		return true;

	return false;
}

/*
==================
ServerMain::parseCIDRNotation

Parse a CIDR notation type string and return a netadr_t and suffix by reference
==================
*/

bool ServerMain::parseCIDRNotation(netadr_t *dest, int *mask, char *adrstr)
{
	char *suffix;
	
	suffix = strchr(adrstr, '/');
	if(suffix)
	{
		*suffix = '\0';
		suffix++;
	}

	if(!Network::stringToAdr(adrstr, dest, NA_UNSPEC))
		return true;

	if(suffix)
	{
		*mask = atoi(suffix);
		
		if(dest->type == NA_IP)
		{
			if(*mask < 1 || *mask > 32)
				*mask = 32;
		}
		else
		{
			if(*mask < 1 || *mask > 128)
				*mask = 128;
		}
	}
	else if(dest->type == NA_IP)
		*mask = 32;
	else
		*mask = 128;
	
	return false;
}

/*
==================
ServerMain::addBanToList

Ban a user from being able to play on this server based on his ip address.
==================
*/

void ServerMain::addBanToList(bool isexception)
{
	char *banstring;
	char addy2[NET_ADDRSTRMAXLEN];
	netadr_t ip;
	int index, argc, mask;
	serverBan_t *curban;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	argc = CommandSystem::argc();
	
	if(argc < 2 || argc > 3)
	{
		Common::printf ("Usage: %s (ip[/subnet] | clientnum [subnet])\n", CommandSystem::argv(0));
		return;
	}

	if(serverBansCount >= ARRAY_LEN(serverBans))
	{
		Common::printf ("Error: Maximum number of bans/exceptions exceeded.\n");
		return;
	}

	banstring = CommandSystem::argv(1);
	
	if(strchr(banstring, '.') || strchr(banstring, ':'))
	{
		// This is an ip address, not a client num.
		
		if(ServerMain::parseCIDRNotation(&ip, &mask, banstring))
		{
			Common::printf("Error: Invalid address %s\n", banstring);
			return;
		}
	}
	else
	{
		SV_Client *cl;
		
		// client num.
		
		cl = ServerMain::getPlayerByNum();

		if(!cl)
		{
			Common::printf("Error: Playernum %s does not exist.\n", CommandSystem::argv(1));
			return;
		}
		
		ip = cl->netchan_.remoteAddress();
		
		if(argc == 3)
		{
			mask = atoi(CommandSystem::argv(2));
			
			if(ip.type == NA_IP)
			{
				if(mask < 1 || mask > 32)
					mask = 32;
			}
			else
			{
				if(mask < 1 || mask > 128)
					mask = 128;
			}
		}
		else
			mask = (ip.type == NA_IP6) ? 128 : 32;
	}

	if(ip.type != NA_IP && ip.type != NA_IP6)
	{
		Common::printf("Error: Can ban players connected via the internet only.\n");
		return;
	}

	// first check whether a conflicting ban exists that would supersede the new one.
	for(index = 0; index < serverBansCount; index++)
	{
		curban = &serverBans[index];
		
		if(curban->subnet <= mask)
		{
			if((curban->isexception || !isexception) && Network::compareBaseAdrMask(curban->ip, ip, curban->subnet))
			{
				Q_strncpyz(addy2, Network::adrToString(ip), sizeof(addy2));
				
				Common::printf("Error: %s %s/%d supersedes %s %s/%d\n", curban->isexception ? "Exception" : "Ban",
					   Network::adrToString(curban->ip), curban->subnet,
					   isexception ? "exception" : "ban", addy2, mask);
				return;
			}
		}
		if(curban->subnet >= mask)
		{
			if(!curban->isexception && isexception && Network::compareBaseAdrMask(curban->ip, ip, mask))
			{
				Q_strncpyz(addy2, Network::adrToString(curban->ip), sizeof(addy2));
			
				Common::printf("Error: %s %s/%d supersedes already existing %s %s/%d\n", isexception ? "Exception" : "Ban",
					   Network::adrToString(ip), mask,
					   curban->isexception ? "exception" : "ban", addy2, curban->subnet);
				return;
			}
		}
	}

	// now delete bans that are superseded by the new one
	index = 0;
	while(index < serverBansCount)
	{
		curban = &serverBans[index];
		
		if(curban->subnet > mask && (!curban->isexception || isexception) && Network::compareBaseAdrMask(curban->ip, ip, mask))
			ServerMain::delBanEntryFromList(index);
		else
			index++;
	}

	serverBans[serverBansCount].ip = ip;
	serverBans[serverBansCount].subnet = mask;
	serverBans[serverBansCount].isexception = isexception;
	
	serverBansCount++;
	
	ServerMain::writeBans();

	Common::printf("Added %s: %s/%d\n", isexception ? "ban exception" : "ban",
		   Network::adrToString(ip), mask);
}

/*
==================
ServerMain::delBanFromList

Remove a ban or an exception from the list.
==================
*/

void ServerMain::delBanFromList(bool isexception)
{
	int index, count = 0, todel, mask;
	netadr_t ip;
	char *banstring;
	
	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}
	
	if(CommandSystem::argc() != 2)
	{
		Common::printf ("Usage: %s (ip[/subnet] | num)\n", CommandSystem::argv(0));
		return;
	}

	banstring = CommandSystem::argv(1);
	
	if(strchr(banstring, '.') || strchr(banstring, ':'))
	{
		serverBan_t *curban;
		
		if(ServerMain::parseCIDRNotation(&ip, &mask, banstring))
		{
			Common::printf("Error: Invalid address %s\n", banstring);
			return;
		}
		
		index = 0;
		
		while(index < serverBansCount)
		{
			curban = &serverBans[index];
			
			if(curban->isexception == isexception		&&
			   curban->subnet >= mask 			&&
			   Network::compareBaseAdrMask(curban->ip, ip, mask))
			{
				Common::printf("Deleting %s %s/%d\n",
					   isexception ? "exception" : "ban",
					   Network::adrToString(curban->ip), curban->subnet);
					   
				ServerMain::delBanEntryFromList(index);
			}
			else
				index++;
		}
	}
	else
	{
		todel = atoi(CommandSystem::argv(1));

		if(todel < 1 || todel > serverBansCount)
		{
			Common::printf("Error: Invalid ban number given\n");
			return;
		}
	
		for(index = 0; index < serverBansCount; index++)
		{
			if(serverBans[index].isexception == isexception)
			{
				count++;
			
				if(count == todel)
				{
					Common::printf("Deleting %s %s/%d\n",
					   isexception ? "exception" : "ban",
					   Network::adrToString(serverBans[index].ip), serverBans[index].subnet);

					ServerMain::delBanEntryFromList(index);

					break;
				}
			}
		}
	}
	
	ServerMain::writeBans();
}


/*
==================
ServerMain::listBans_f

List all bans and exceptions on console
==================
*/

void ServerMain::listBans_f()
{
	int index, count;
	serverBan_t *ban;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}
	
	// List all bans
	for(index = count = 0; index < serverBansCount; index++)
	{
		ban = &serverBans[index];
		if(!ban->isexception)
		{
			count++;

			Common::printf("Ban #%d: %s/%d\n", count,
				    Network::adrToString(ban->ip), ban->subnet);
		}
	}
	// List all exceptions
	for(index = count = 0; index < serverBansCount; index++)
	{
		ban = &serverBans[index];
		if(ban->isexception)
		{
			count++;

			Common::printf("Except #%d: %s/%d\n", count,
				    Network::adrToString(ban->ip), ban->subnet);
		}
	}
}

/*
==================
ServerMain::flushBans_f

Delete all bans and exceptions.
==================
*/

void ServerMain::flushBans_f()
{
	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	serverBansCount = 0;
	
	// empty the ban file.
	ServerMain::writeBans();
	
	Common::printf("All bans and exceptions have been deleted.\n");
}

void ServerMain::banAddr_f()
{
	ServerMain::addBanToList(false);
}

void ServerMain::exceptAddr_f()
{
	ServerMain::addBanToList(true);
}

void ServerMain::banDel_f()
{
	ServerMain::delBanFromList(false);
}

void ServerMain::exceptDel_f()
{
	ServerMain::delBanFromList(true);
}

/*
** ServerMain::strlen -- skips color escape codes
*/
int ServerMain::strlen( const char *str ) {
	const char *s = str;
	int count = 0;

	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

/*
================
ServerMain::status_f
================
*/
void ServerMain::status_f( ) {
	int			i, j, l;
	SV_Client	*cl;
	playerState_t	*ps;
	const char		*s;
	int			ping;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	Common::printf ("map: %s\n", sv_mapname->asString() );

	Common::printf ("cl score ping name            address                                 rate \n");
	Common::printf ("-- ----- ---- --------------- --------------------------------------- -----\n");
	for (i=1 ; i <= sv_maxclients->asInt() ; i++)
	{
		cl = theSVS.svClients_.at(i);
		if (!cl || !cl->state_)
			continue;
		Common::printf ("%2i ", i);
		ps = &theSV.getClientByNum( i )->ps_;
		Common::printf ("%5i ", ps->persistant[PERS_SCORE]);

		if (cl->state_ == SV_Client::CS_CONNECTED)
			Common::printf ("CON ");
		else if (cl->state_ == SV_Client::CS_ZOMBIE)
			Common::printf ("ZMB ");
		else
		{
			ping = cl->ping_ < 9999 ? cl->ping_ : 9999;
			Common::printf ("%4i ", ping);
		}

		Common::printf ("%s", cl->name_);
		
		l = 16 - ServerMain::strlen(cl->name_);
		j = 0;
		
		do
		{
			Common::printf (" ");
			j++;
		} while(j < l);


		// TTimo adding a ^7 to reset the color
		s = Network::adrToString( cl->netchan_.remoteAddress() );
		Common::printf ("^7%s", s);
		l = 39 - strlen(s);
		j = 0;
		
		do
		{
			Common::printf(" ");
			j++;
		} while(j < l);
		
		Common::printf (" %5i", cl->rate_);

		Common::printf ("\n");
	}
	Common::printf ("\n");
}

/*
==================
ServerMain::conSay_f
==================
*/
void ServerMain::conSay_f() {
	char	*p;
	char	text[1024];

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc () < 2 ) {
		return;
	}

	strcpy (text, "console: ");
	p = CommandSystem::args();

	if ( *p == '"' ) {
		p++;
		p[strlen(p)-1] = 0;
	}

	strcat(text, p);

	Common::printf("%s\n", text);
	ServerMain::sendServerCommand(nullptr, "chat \"%s\"", text);
}

/*
==================
ServerMain::conTell_f
==================
*/
void ServerMain::conTell_f() {
	char	*p;
	char	text[1024];
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() < 3 ) {
		Common::printf ("Usage: tell <client number> <text>\n");
		return;
	}

	cl = ServerMain::getPlayerByNum();
	if ( !cl ) {
		return;
	}

	strcpy (text, "console_tell: ");
	p = CommandSystem::argsFrom(2);

	if ( *p == '"' ) {
		p++;
		p[strlen(p)-1] = 0;
	}

	strcat(text, p);

	Common::printf("%s\n", text);
	ServerMain::sendServerCommand(cl, "chat \"%s\"", text);
}


/*
==================
ServerMain::conSayto_f
==================
*/
void ServerMain::conSayto_f() {
	char		*p;
	char		text[1024];
	SV_Client	*cl;
	char		*rawname;
	char		name[MAX_NAME_LENGTH];
	char		cleanName[MAX_NAME_LENGTH];
	SV_Client	*saytocl;
	int			i;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() < 3 ) {
		Common::printf ("Usage: sayto <player name> <text>\n");
		return;
	}

	rawname = CommandSystem::argv(1);
	
	//allowing special characters in the console 
	//with hex strings for player names
	Common::fieldStringToPlayerName( name, MAX_NAME_LENGTH, rawname );

	saytocl = nullptr;
	for ( i=1 ; i <= sv_maxclients->asInt() ; i++ ) {
		cl = theSVS.svClients_.at(i);
		if ( !cl )
			continue;
		if ( !cl->state_ ) {
			continue;
		}
		Q_strncpyz( cleanName, cl->name_, sizeof(cleanName) );
		Q_CleanStr( cleanName );

		if ( !Q_stricmp( cleanName, name ) ) {
			saytocl = cl;
			break;
		}
	}
	if( !saytocl )
	{
		Common::printf ("No such player name: %s.\n", name);
		return;
	}

	strcpy (text, "console_sayto: ");
	p = CommandSystem::argsFrom(2);

	if ( *p == '"' ) {
		p++;
		p[strlen(p)-1] = 0;
	}

	strcat(text, p);

	Common::printf("%s\n", text);
	ServerMain::sendServerCommand(saytocl, "chat \"%s\"", text);
}


/*
==================
ServerMain::heartbeat_f

Also called by SV_DropClient, ServerMain::directConnect, and ServerMain::spawnServer
==================
*/
void ServerMain::heartbeat_f( ) {
	theSVS.nextHeartbeatTime_ = -9999999;
}


/*
===========
ServerMain::serverinfo_f

Examine the serverinfo string
===========
*/
void ServerMain::serverinfo_f( ) {
	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	Common::printf ("Server info settings:\n");
	Info_Print ( CvarSystem::infoString( CVAR_SERVERINFO ) );
}


/*
===========
ServerMain::systeminfo_f

Examine the systeminfo string
===========
*/
void ServerMain::systeminfo_f( ) {
	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	Common::printf ("System info settings:\n");
	Info_Print ( CvarSystem::infoString_Big( CVAR_SYSTEMINFO ) );
}


/*
===========
ServerMain::dumpUser_f

Examine all a users info strings
===========
*/
void ServerMain::dumpUser_f( ) {
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->asInt() ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: dumpuser <userid>\n");
		return;
	}

	cl = ServerMain::getPlayerByHandle();
	if ( !cl ) {
		return;
	}

	Common::printf( "userinfo\n" );
	Common::printf( "--------\n" );
	Info_Print( cl->userinfo_ );
}


/*
=================
SV_KillServer
=================
*/
void ServerMain::killServer_f( ) {
	ServerLoop::shutdown( "killserver" );
}

//===========================================================

/*
==================
ServerMain::completeMapName
==================
*/
void ServerMain::completeMapName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "maps", "bsp", nullptr, true, false );
	}
}

/*
==================
ServerMain::completePlayerName
==================
*/
void ServerMain::completePlayerName( char *args, int argNum ) {
	if( argNum == 2 ) {
		char		names[MAX_CLIENTS][MAX_NAME_LENGTH];
		const char	*namesPtr[MAX_CLIENTS];
		SV_Client	*cl;
		int			i;
		int			nameCount;
		int			clientCount;

		nameCount = 0;
		clientCount = sv_maxclients->asInt();

		for ( i=1 ; i <= clientCount ; i++ ) {
			cl = theSVS.svClients_.at(i);
			if ( !cl )
				continue;
			if ( !cl->state_ ) {
				continue;
			}
			if( i >= MAX_CLIENTS ) {
				break;
			}
			Q_strncpyz( names[nameCount], cl->name_, sizeof(names[nameCount]) );
			Q_CleanStr( names[nameCount] );

			namesPtr[nameCount] = names[nameCount];

			nameCount++;
		}
		SortArray( namesPtr, nameCount, Common::strCompare );

		Field_CompletePlayerName( namesPtr, nameCount );
	}
}

/*
==================
ServerMain::addOperatorCommands
==================
*/
void ServerMain::addOperatorCommands( ) {
	static bool	initialized;

	if ( initialized ) {
		return;
	}
	initialized = true;

	CommandSystem::addCommand ("heartbeat", ServerMain::heartbeat_f);
	CommandSystem::addCommand ("kick", ServerMain::kick_f);
#ifndef STANDALONE
	if(!com_standalone->asInt())
	{
		CommandSystem::addCommand ("banUser", ServerMain::ban_f);
		CommandSystem::addCommand ("banClient", ServerMain::banNum_f);
	}
#endif
	CommandSystem::addCommand ("kickbots", ServerMain::kickBots_f);
	CommandSystem::addCommand ("tracebench", ServerMain::traceBench_f);
	CommandSystem::addCommand ("kickall", ServerMain::kickAll_f);
	CommandSystem::addCommand ("kicknum", ServerMain::kickNum_f);
	CommandSystem::addCommand ("clientkick", ServerMain::kickNum_f); // Legacy command
	CommandSystem::addCommand ("status", ServerMain::status_f);
	CommandSystem::addCommand ("serverinfo", ServerMain::serverinfo_f);
	CommandSystem::addCommand ("systeminfo", ServerMain::systeminfo_f);
	CommandSystem::addCommand ("dumpuser", ServerMain::dumpUser_f);
	CommandSystem::addCommand ("map_restart", ServerMain::mapRestart_f);
	CommandSystem::addCommand ("sectorlist", ServerMain::sectorList_f);
	CommandSystem::addCommand ("map", ServerMain::map_f);
	CommandSystem::setCommandCompletionFunc( "map", ServerMain::completeMapName );
#ifndef PRE_RELEASE_DEMO
	CommandSystem::addCommand ("devmap", ServerMain::map_f);
	CommandSystem::setCommandCompletionFunc( "devmap", ServerMain::completeMapName );
	CommandSystem::addCommand ("spmap", ServerMain::map_f);
	CommandSystem::setCommandCompletionFunc( "spmap", ServerMain::completeMapName );
	CommandSystem::addCommand ("spdevmap", ServerMain::map_f);
	CommandSystem::setCommandCompletionFunc( "spdevmap", ServerMain::completeMapName );
#endif
	CommandSystem::addCommand ("killserver", ServerMain::killServer_f);
	if( com_dedicated->asInt() ) {
		CommandSystem::addCommand ("say", ServerMain::conSay_f);
		CommandSystem::addCommand ("tell", ServerMain::conTell_f);
		CommandSystem::addCommand ("sayto", ServerMain::conSayto_f);
		CommandSystem::setCommandCompletionFunc( "sayto", ServerMain::completePlayerName );
	}
	
	CommandSystem::addCommand("rehashbans", ServerMain::rehashBans_f);
	CommandSystem::addCommand("listbans", ServerMain::listBans_f);
	CommandSystem::addCommand("banaddr", ServerMain::banAddr_f);
	CommandSystem::addCommand("exceptaddr", ServerMain::exceptAddr_f);
	CommandSystem::addCommand("bandel", ServerMain::banDel_f);
	CommandSystem::addCommand("exceptdel", ServerMain::exceptDel_f);
	CommandSystem::addCommand("flushbans", ServerMain::flushBans_f);
}

/*
==================
ServerMain::removeOperatorCommands
==================
*/
void ServerMain::removeOperatorCommands( ) {
#if 0
	// removing these won't let the server start again
	CommandSystem::removeCommand ("heartbeat");
	CommandSystem::removeCommand ("kick");
	CommandSystem::removeCommand ("kicknum");
	CommandSystem::removeCommand ("clientkick");
	CommandSystem::removeCommand ("kickall");
	CommandSystem::removeCommand ("kickbots");
	CommandSystem::removeCommand ("banUser");
	CommandSystem::removeCommand ("banClient");
	CommandSystem::removeCommand ("status");
	CommandSystem::removeCommand ("serverinfo");
	CommandSystem::removeCommand ("systeminfo");
	CommandSystem::removeCommand ("dumpuser");
	CommandSystem::removeCommand ("map_restart");
	CommandSystem::removeCommand ("sectorlist");
	CommandSystem::removeCommand ("say");
#endif
}


