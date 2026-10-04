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
SV_GetPlayerByHandle

Returns the player with player id or name from CommandSystem::argv(1)
==================
*/
static SV_Client *SV_GetPlayerByHandle( ) {
	SV_Client	*cl;
	int			i;
	char		*s;
	char		cleanName[64];

	// make sure server is running
	if ( !com_sv_running->integer ) {
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
		if(plid >= 1 && plid <= sv_maxclients->integer)
		{
			cl = theSVS.svClients_.at(plid);
			
			if(cl && cl->state_)
				return cl;
		}
	}

	// check for a name match
	for ( i=1 ; i <= sv_maxclients->integer ; i++ ) {
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
SV_GetPlayerByNum

Returns the player with idnum from CommandSystem::argv(1)
==================
*/
static SV_Client *SV_GetPlayerByNum( ) {
	SV_Client	*cl;
	int			i;
	int			idnum;
	char		*s;

	// make sure server is running
	if ( !com_sv_running->integer ) {
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
	if ( idnum < 1 || idnum > sv_maxclients->integer ) {
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
SV_Map_f

Restart the server on a different map
==================
*/
static void SV_Map_f() 
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
		if( sv_gametype->integer == GT_SINGLE_PLAYER )
			CvarSystem::setValue( "g_gametype", GT_FFA );
	}

	// save the map name here cause on a map restart we reload the mfconfig.cfg
	// and thus nuke the arguments of the map command
	Q_strncpyz(mapname, map, sizeof(mapname));

	// start up the map
	SV_SpawnServer( mapname, killBots );

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
SV_MapRestart_f

Completely restarts a level, but doesn't send a new gamestate to the clients.
This allows fair starts with variable load times.
================
*/
static void SV_MapRestart_f() 
{
	// make sure we aren't restarting twice in the same frame
	if( com_frameTime == theSV.serverId_ ) 
		return;

	// make sure server is running
	if( !com_sv_running->integer )
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
		SV_SetConfigstring( CS_WARMUP, va("%i", theSV.restartTime_) );
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

		SV_SpawnServer( mapname, false );
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
	for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
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

	SV_RestartGameProgs();

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
	for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
	{
		SV_Client* client = theSVS.svClients_.at(i);

		// send the New gamestate to all connected clients
		if( !client || client->state_ < SV_Client::CS_CONNECTED)
			continue;

		if( client->netchan_.remoteAddress.type == NA_BOT ) 
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
SV_Kick_f

Kick a user off of the server
==================
*/
static void SV_Kick_f( ) {
	SV_Client	*cl;
	int			i;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: kick <player name>\nkick all = kick everyone\nkick allbots = kick all bots\n");
		return;
	}

	cl = SV_GetPlayerByHandle();
	if ( !cl ) {
		if ( !Q_stricmp(CommandSystem::argv(1), "all") ) {
			for ( i=1 ; i <= sv_maxclients->integer ; i++ ) {
				cl = theSVS.svClients_.at(i);
				if ( !cl )
					continue;
				if ( !cl->state_ ) {
					continue;
				}
				if( cl->netchan_.remoteAddress.type == NA_LOOPBACK ) {
					continue;
				}
				cl->drop( "was kicked" );
				cl->lastPacketTime_ = theSVS.time_;	// in case there is a funny zombie
			}
		}
		else if ( !Q_stricmp(CommandSystem::argv(1), "allbots") ) {
			for ( i=1 ; i <= sv_maxclients->integer ; i++ ) {
				cl = theSVS.svClients_.at(i);
				if ( !cl )
					continue;
				if ( !cl->state_ ) {
					continue;
				}
				if( cl->netchan_.remoteAddress.type != NA_BOT ) {
					continue;
				}
				cl->drop( "was kicked" );
				cl->lastPacketTime_ = theSVS.time_;	// in case there is a funny zombie
			}
		}
		return;
	}
	if( cl->netchan_.remoteAddress.type == NA_LOOPBACK ) {
		Common::printf("Cannot kick host player\n");
		return;
	}

	cl->drop( "was kicked" );
	cl->lastPacketTime_ = theSVS.time_;	// in case there is a funny zombie
}

/*
==================
SV_KickBots_f

Kick all bots off of the server
==================
*/
static void SV_KickBots_f( ) {
	SV_Client	*cl;
	int			i;

	// make sure server is running
	if( !com_sv_running->integer ) {
		Common::printf("Server is not running.\n");
		return;
	}

	for ( i=1 ; i <= sv_maxclients->integer ; i++ ) {
		cl = theSVS.svClients_.at(i);
		if ( !cl )
			continue;
		if( !cl->state_ ) {
			continue;
		}

		if( cl->netchan_.remoteAddress.type != NA_BOT ) {
			continue;
		}

		cl->drop( "was kicked" );
		cl->lastPacketTime_ = theSVS.time_; // in case there is a funny zombie
	}
}
/*
==================
SV_KickAll_f

Kick all users off of the server
==================
*/
static void SV_KickAll_f( ) {
	SV_Client *cl;
	int i;

	// make sure server is running
	if( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	for ( i=1 ; i <= sv_maxclients->integer ; i++ ) {
		cl = theSVS.svClients_.at(i);
		if ( !cl )
			continue;
		if( !cl->state_ ) {
			continue;
		}

		if( cl->netchan_.remoteAddress.type == NA_LOOPBACK ) {
			continue;
		}

		cl->drop( "was kicked" );
		cl->lastPacketTime_ = theSVS.time_; // in case there is a funny zombie
	}
}

/*
==================
SV_KickNum_f

Kick a user off of the server
==================
*/
static void SV_KickNum_f( ) {
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: %s <client number>\n", CommandSystem::argv(0));
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}
	if( cl->netchan_.remoteAddress.type == NA_LOOPBACK ) {
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
SV_Ban_f

Ban a user from being able to play on this server through the auth
server
==================
*/
static void SV_Ban_f( void ) {
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: banUser <player name>\n");
		return;
	}

	cl = SV_GetPlayerByHandle();

	if (!cl) {
		return;
	}

	if( cl->netchan_.remoteAddress.type == NA_LOOPBACK ) {
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
			"banUser %i.%i.%i.%i", cl->netchan_.remoteAddress.ip[0], cl->netchan_.remoteAddress.ip[1], 
								   cl->netchan_.remoteAddress.ip[2], cl->netchan_.remoteAddress.ip[3] );
		Common::printf("%s was banned from coming back\n", cl->name_);
	}
}

/*
==================
SV_BanNum_f

Ban a user from being able to play on this server through the auth
server
==================
*/
static void SV_BanNum_f( void ) {
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: banClient <client number>\n");
		return;
	}

	cl = SV_GetPlayerByNum();
	if ( !cl ) {
		return;
	}
	if( cl->netchan_.remoteAddress.type == NA_LOOPBACK ) {
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
			"banUser %i.%i.%i.%i", cl->netchan_.remoteAddress.ip[0], cl->netchan_.remoteAddress.ip[1], 
								   cl->netchan_.remoteAddress.ip[2], cl->netchan_.remoteAddress.ip[3] );
		Common::printf("%s was banned from coming back\n", cl->name_);
	}
}
#endif

/*
==================
SV_RehashBans_f

Load saved bans from file.
==================
*/
static void SV_RehashBans_f()
{
	int index, filelen;
	fileHandle_t readfrom;
	char *textbuf, *curpos, *maskpos, *newlinepos, *endpos;
	char filepath[MAX_QPATH];
	
	serverBansCount = 0;
	
	if(!sv_banFile->string || !*sv_banFile->string)
		return;

	Com_sprintf(filepath, sizeof(filepath), "%s/%s", FileSystem::getCurrentGameDir(), sv_banFile->string);

	if((filelen = FileSystem::baseDir_FOpenFileRead(filepath, &readfrom)) >= 0)
	{
		if(filelen < 2)
		{
			// Don't bother if file is too short.
			FileSystem::fCloseFile(readfrom);
			return;
		}

		curpos = textbuf = static_cast<char *>(Z_Malloc(filelen));
		
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
		
		Z_Free(textbuf);
	}
}

/*
==================
SV_WriteBans

Save bans to file.
==================
*/
static void SV_WriteBans()
{
	int index;
	fileHandle_t writeto;
	char filepath[MAX_QPATH];
	
	if(!sv_banFile->string || !*sv_banFile->string)
		return;
	
	Com_sprintf(filepath, sizeof(filepath), "%s/%s", FileSystem::getCurrentGameDir(), sv_banFile->string);

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
SV_DelBanEntryFromList

Remove a ban or an exception from the list.
==================
*/

static bool SV_DelBanEntryFromList(int index)
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
SV_ParseCIDRNotation

Parse a CIDR notation type string and return a netadr_t and suffix by reference
==================
*/

static bool SV_ParseCIDRNotation(netadr_t *dest, int *mask, char *adrstr)
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
SV_AddBanToList

Ban a user from being able to play on this server based on his ip address.
==================
*/

static void SV_AddBanToList(bool isexception)
{
	char *banstring;
	char addy2[NET_ADDRSTRMAXLEN];
	netadr_t ip;
	int index, argc, mask;
	serverBan_t *curban;

	// make sure server is running
	if ( !com_sv_running->integer ) {
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
		
		if(SV_ParseCIDRNotation(&ip, &mask, banstring))
		{
			Common::printf("Error: Invalid address %s\n", banstring);
			return;
		}
	}
	else
	{
		SV_Client *cl;
		
		// client num.
		
		cl = SV_GetPlayerByNum();

		if(!cl)
		{
			Common::printf("Error: Playernum %s does not exist.\n", CommandSystem::argv(1));
			return;
		}
		
		ip = cl->netchan_.remoteAddress;
		
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
			SV_DelBanEntryFromList(index);
		else
			index++;
	}

	serverBans[serverBansCount].ip = ip;
	serverBans[serverBansCount].subnet = mask;
	serverBans[serverBansCount].isexception = isexception;
	
	serverBansCount++;
	
	SV_WriteBans();

	Common::printf("Added %s: %s/%d\n", isexception ? "ban exception" : "ban",
		   Network::adrToString(ip), mask);
}

/*
==================
SV_DelBanFromList

Remove a ban or an exception from the list.
==================
*/

static void SV_DelBanFromList(bool isexception)
{
	int index, count = 0, todel, mask;
	netadr_t ip;
	char *banstring;
	
	// make sure server is running
	if ( !com_sv_running->integer ) {
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
		
		if(SV_ParseCIDRNotation(&ip, &mask, banstring))
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
					   
				SV_DelBanEntryFromList(index);
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

					SV_DelBanEntryFromList(index);

					break;
				}
			}
		}
	}
	
	SV_WriteBans();
}


/*
==================
SV_ListBans_f

List all bans and exceptions on console
==================
*/

static void SV_ListBans_f()
{
	int index, count;
	serverBan_t *ban;

	// make sure server is running
	if ( !com_sv_running->integer ) {
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
SV_FlushBans_f

Delete all bans and exceptions.
==================
*/

static void SV_FlushBans_f()
{
	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	serverBansCount = 0;
	
	// empty the ban file.
	SV_WriteBans();
	
	Common::printf("All bans and exceptions have been deleted.\n");
}

static void SV_BanAddr_f()
{
	SV_AddBanToList(false);
}

static void SV_ExceptAddr_f()
{
	SV_AddBanToList(true);
}

static void SV_BanDel_f()
{
	SV_DelBanFromList(false);
}

static void SV_ExceptDel_f()
{
	SV_DelBanFromList(true);
}

/*
** SV_Strlen -- skips color escape codes
*/
static int SV_Strlen( const char *str ) {
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
SV_Status_f
================
*/
static void SV_Status_f( ) {
	int			i, j, l;
	SV_Client	*cl;
	playerState_t	*ps;
	const char		*s;
	int			ping;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	Common::printf ("map: %s\n", sv_mapname->string );

	Common::printf ("cl score ping name            address                                 rate \n");
	Common::printf ("-- ----- ---- --------------- --------------------------------------- -----\n");
	for (i=1 ; i <= sv_maxclients->integer ; i++)
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
		
		l = 16 - SV_Strlen(cl->name_);
		j = 0;
		
		do
		{
			Common::printf (" ");
			j++;
		} while(j < l);


		// TTimo adding a ^7 to reset the color
		s = Network::adrToString( cl->netchan_.remoteAddress );
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
SV_ConSay_f
==================
*/
static void SV_ConSay_f() {
	char	*p;
	char	text[1024];

	// make sure server is running
	if ( !com_sv_running->integer ) {
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
	SV_SendServerCommand(nullptr, "chat \"%s\"", text);
}

/*
==================
SV_ConTell_f
==================
*/
static void SV_ConTell_f() {
	char	*p;
	char	text[1024];
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() < 3 ) {
		Common::printf ("Usage: tell <client number> <text>\n");
		return;
	}

	cl = SV_GetPlayerByNum();
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
	SV_SendServerCommand(cl, "chat \"%s\"", text);
}


/*
==================
SV_ConSayto_f
==================
*/
static void SV_ConSayto_f() {
	char		*p;
	char		text[1024];
	SV_Client	*cl;
	char		*rawname;
	char		name[MAX_NAME_LENGTH];
	char		cleanName[MAX_NAME_LENGTH];
	SV_Client	*saytocl;
	int			i;

	// make sure server is running
	if ( !com_sv_running->integer ) {
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
	for ( i=1 ; i <= sv_maxclients->integer ; i++ ) {
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
	SV_SendServerCommand(saytocl, "chat \"%s\"", text);
}


/*
==================
SV_Heartbeat_f

Also called by SV_DropClient, SV_DirectConnect, and SV_SpawnServer
==================
*/
void SV_Heartbeat_f( ) {
	theSVS.nextHeartbeatTime_ = -9999999;
}


/*
===========
SV_Serverinfo_f

Examine the serverinfo string
===========
*/
static void SV_Serverinfo_f( ) {
	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	Common::printf ("Server info settings:\n");
	Info_Print ( CvarSystem::infoString( CVAR_SERVERINFO ) );
}


/*
===========
SV_Systeminfo_f

Examine the systeminfo string
===========
*/
static void SV_Systeminfo_f( ) {
	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	Common::printf ("System info settings:\n");
	Info_Print ( CvarSystem::infoString_Big( CVAR_SYSTEMINFO ) );
}


/*
===========
SV_DumpUser_f

Examine all a users info strings
===========
*/
static void SV_DumpUser_f( ) {
	SV_Client	*cl;

	// make sure server is running
	if ( !com_sv_running->integer ) {
		Common::printf( "Server is not running.\n" );
		return;
	}

	if ( CommandSystem::argc() != 2 ) {
		Common::printf ("Usage: dumpuser <userid>\n");
		return;
	}

	cl = SV_GetPlayerByHandle();
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
static void SV_KillServer_f( ) {
	SV_Shutdown( "killserver" );
}

//===========================================================

/*
==================
SV_CompleteMapName
==================
*/
static void SV_CompleteMapName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "maps", "bsp", nullptr, true, false );
	}
}

/*
==================
SV_CompletePlayerName
==================
*/
static void SV_CompletePlayerName( char *args, int argNum ) {
	if( argNum == 2 ) {
		char		names[MAX_CLIENTS][MAX_NAME_LENGTH];
		const char	*namesPtr[MAX_CLIENTS];
		SV_Client	*cl;
		int			i;
		int			nameCount;
		int			clientCount;

		nameCount = 0;
		clientCount = sv_maxclients->integer;

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
		qsort( (void*)namesPtr, nameCount, sizeof( namesPtr[0] ), Common::strCompare );

		Field_CompletePlayerName( namesPtr, nameCount );
	}
}

/*
==================
SV_AddOperatorCommands
==================
*/
void SV_AddOperatorCommands( ) {
	static bool	initialized;

	if ( initialized ) {
		return;
	}
	initialized = true;

	CommandSystem::addCommand ("heartbeat", SV_Heartbeat_f);
	CommandSystem::addCommand ("kick", SV_Kick_f);
#ifndef STANDALONE
	if(!com_standalone->integer)
	{
		CommandSystem::addCommand ("banUser", SV_Ban_f);
		CommandSystem::addCommand ("banClient", SV_BanNum_f);
	}
#endif
	CommandSystem::addCommand ("kickbots", SV_KickBots_f);
	CommandSystem::addCommand ("kickall", SV_KickAll_f);
	CommandSystem::addCommand ("kicknum", SV_KickNum_f);
	CommandSystem::addCommand ("clientkick", SV_KickNum_f); // Legacy command
	CommandSystem::addCommand ("status", SV_Status_f);
	CommandSystem::addCommand ("serverinfo", SV_Serverinfo_f);
	CommandSystem::addCommand ("systeminfo", SV_Systeminfo_f);
	CommandSystem::addCommand ("dumpuser", SV_DumpUser_f);
	CommandSystem::addCommand ("map_restart", SV_MapRestart_f);
	CommandSystem::addCommand ("sectorlist", SV_SectorList_f);
	CommandSystem::addCommand ("map", SV_Map_f);
	CommandSystem::setCommandCompletionFunc( "map", SV_CompleteMapName );
#ifndef PRE_RELEASE_DEMO
	CommandSystem::addCommand ("devmap", SV_Map_f);
	CommandSystem::setCommandCompletionFunc( "devmap", SV_CompleteMapName );
	CommandSystem::addCommand ("spmap", SV_Map_f);
	CommandSystem::setCommandCompletionFunc( "spmap", SV_CompleteMapName );
	CommandSystem::addCommand ("spdevmap", SV_Map_f);
	CommandSystem::setCommandCompletionFunc( "spdevmap", SV_CompleteMapName );
#endif
	CommandSystem::addCommand ("killserver", SV_KillServer_f);
	if( com_dedicated->integer ) {
		CommandSystem::addCommand ("say", SV_ConSay_f);
		CommandSystem::addCommand ("tell", SV_ConTell_f);
		CommandSystem::addCommand ("sayto", SV_ConSayto_f);
		CommandSystem::setCommandCompletionFunc( "sayto", SV_CompletePlayerName );
	}
	
	CommandSystem::addCommand("rehashbans", SV_RehashBans_f);
	CommandSystem::addCommand("listbans", SV_ListBans_f);
	CommandSystem::addCommand("banaddr", SV_BanAddr_f);
	CommandSystem::addCommand("exceptaddr", SV_ExceptAddr_f);
	CommandSystem::addCommand("bandel", SV_BanDel_f);
	CommandSystem::addCommand("exceptdel", SV_ExceptDel_f);
	CommandSystem::addCommand("flushbans", SV_FlushBans_f);
}

/*
==================
SV_RemoveOperatorCommands
==================
*/
void SV_RemoveOperatorCommands( ) {
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


