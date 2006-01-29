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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "server.h"
#include "../game/game.h"
#include "sv_server.h"
#include "sv_client.h"

void SV_InitGame();

/*
===============
SV_SetConfigstring

===============
*/
void SV_SetConfigstring( int index, const char *val ) 
{
	if( index < 0 || index >= MAX_CONFIGSTRINGS ) 
		Com_Error (ERR_DROP, "SV_SetConfigstring: bad index %i\n", index);

	if( !val ) 
		val = "";

	// don't bother broadcasting an update if no change
	if( !strcmp( val, theSV.configstrings_[ index ] ) ) 
		return;

	// change the string in sv
	Z_Free( theSV.configstrings_[index] );
	theSV.configstrings_[index] = CopyString( val );

	// send it to all the clients if we aren't
	// spawning a New server
	if( theSV.state_ == Server::SS_GAME || theSV.restarting_ ) 
	{
		int	maxChunkSize = MAX_STRING_CHARS - 24;

		// send the data to all relevent clients
		for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
		{
			SV_Client* client = theSVS.svClients_.at(i);

			if( !client || client->state_ < SV_Client::CS_PRIMED ) 
				continue;

			// do not always send server info to all clients
			EntityBase* gentity = theSV.getEntityByNum(client->clientNum_);
			if( index == CS_SERVERINFO && gentity && (gentity->r.svFlags & SVF_NOSERVERINFO) ) 
				continue;

			int len = strlen( val );
			if( len >= maxChunkSize ) 
			{
				int		sent = 0;
				int		remaining = len;
				char	*cmd;
				char	buf[MAX_STRING_CHARS];

				while (remaining > 0 ) 
				{
					if ( sent == 0 ) 
						cmd = "bcs0";
					else if( remaining < maxChunkSize ) 
						cmd = "bcs2";
					else 
						cmd = "bcs1";
	
					Q_strncpyz( buf, &val[sent], maxChunkSize );

					SV_SendServerCommand( client, "%s %i \"%s\"\n", cmd, index, buf );

					sent += (maxChunkSize - 1);
					remaining -= (maxChunkSize - 1);
				}
			} 
			else 
			{
				// standard cs, just send it
				SV_SendServerCommand( client, "cs %i \"%s\"\n", index, val );
			}
		}
	}
}



/*
===============
SV_GetConfigstring

===============
*/
void SV_GetConfigstring( int index, char *buffer, int bufferSize ) 
{
	if ( bufferSize < 1 )
	{
		Com_Error( ERR_DROP, "SV_GetConfigstring: bufferSize == %i", bufferSize );
	}
	if ( index < 0 || index >= MAX_CONFIGSTRINGS )
	{
		Com_Error (ERR_DROP, "SV_GetConfigstring: bad index %i\n", index);
	}
	if ( !theSV.configstrings_[index] )
	{
		buffer[0] = 0;
		return;
	}

	Q_strncpyz( buffer, theSV.configstrings_[index], bufferSize );
}


/*
===============
SV_SetUserinfo

===============
*/
void SV_SetUserinfo( int index, const char *val ) 
{
	if( index < 1 || index > sv_maxclients->integer ) 
	{
		Com_Error (ERR_DROP, "SV_SetUserinfo: bad index %i\n", index);
	}

	if( !val ) 
		val = "";

	SV_Client* cl = theSVS.svClients_.at(index);

	Q_strncpyz( cl->userinfo_, val, sizeof( cl->userinfo_ ) );
	Q_strncpyz( cl->name_, Info_ValueForKey( val, "name" ), sizeof(cl->name_) );
}



/*
===============
SV_GetUserinfo

===============
*/
void SV_GetUserinfo( int index, char *buffer, int bufferSize ) 
{
	if( bufferSize < 1 ) 
		Com_Error( ERR_DROP, "SV_GetUserinfo: bufferSize == %i", bufferSize );

	if( index < 1 || index > sv_maxclients->integer ) 
		Com_Error (ERR_DROP, "SV_GetUserinfo: bad index %i\n", index);

	SV_Client* cl = theSVS.svClients_.at(index);
	Q_strncpyz( buffer, cl->userinfo_, bufferSize );
}


/*
================
SV_CreateBaseline

Entity baselines are used to compress non-delta messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
void SV_CreateBaseline() 
{
	EntityBase *svent;
	size_t		entnum;	
	size_t		numEnts = theSV.gameEntities_->size();

	for ( entnum = 1; entnum < numEnts ; entnum++ )
	{
		svent = theSV.getEntityByNum(entnum);
		if( !svent || !svent->r.linked ) 
			continue;

		svent->s.number = entnum;

		//
		// take current state as baseline
		//
		theSV.serverEntities_[entnum].baseline_ = svent->s;
	}
}


/*
===============
SV_BoundMaxClients

===============
*/
void SV_BoundMaxClients( int minimum ) 
{
	// get the current maxclients value
	Cvar_Get( "sv_maxclients", "8", 0 );

	sv_maxclients->modified = false;

	if( sv_maxclients->integer < minimum ) 
	{
		Cvar_Set( "sv_maxclients", va("%i", minimum) );
	} 
	else if( sv_maxclients->integer > MAX_CLIENTS )
	{
		Cvar_Set( "sv_maxclients", va("%i", MAX_CLIENTS) );
	}
}


/*
===============
SV_Startup

Called when a host starts a map when it wasn't running
one before.  Successive map or map_restart commands will
NOT cause this to be called, unless the game is exited to
the menu system first.
===============
*/
void SV_Startup() 
{
	if( theSVS.initialized_ ) 
		Com_Error( ERR_FATAL, "SV_Startup: svs.initialized" );

	SV_BoundMaxClients( 1 );

	//theSVS.clients_ = reinterpret_cast<SV_Client*>(Z_Malloc (sizeof(SV_Client) * sv_maxclients->integer ));
	theSVS.initClients(sv_maxclients->integer);	// initializes clients to 0 (will be allocated once they join)
	if( com_dedicated->integer ) 
	{
		theSVS.numSnapshotEntities_ = sv_maxclients->integer * PACKET_BACKUP * 64;
	} 
	else 
	{
		// we don't need nearly as many when playing locally
		theSVS.numSnapshotEntities_ = sv_maxclients->integer * 4 * 64;
	}
	theSVS.initialized_ = true;

	Cvar_Set( "sv_running", "1" );
}


/*
==================
SV_ChangeMaxClients
==================
*/
void SV_ChangeMaxClients() 
{
	// get the highest client number in use
	int count = 0;
	for( int i = 1 ; i <= sv_maxclients->integer ; i++ ) 
	{
		SV_Client* cl = theSVS.svClients_.at(i);
		if( !cl )
			continue;
		if( cl->state_ >= SV_Client::CS_CONNECTED ) 
		{
			if( i > count )
				count = i;
		}
	}
	count++;

	int oldMaxClients = sv_maxclients->integer;
	// never go below the highest client number in use
	SV_BoundMaxClients( count );
	// if still the same
	if( sv_maxclients->integer == oldMaxClients ) 
		return;

	theSVS.changeMaxNumClients( sv_maxclients->integer, count );
	
	// allocate New snapshot entities
	if( com_dedicated->integer )
	{
		theSVS.numSnapshotEntities_ = sv_maxclients->integer * PACKET_BACKUP * 64;
	} 
	else 
	{
		// we don't need nearly as many when playing locally
		theSVS.numSnapshotEntities_ = sv_maxclients->integer * 4 * 64;
	}
}

/*
================
SV_ClearServer
================
*/
void SV_ClearServer() 
{
	int i;

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ )
	{
		if ( theSV.configstrings_[i] )
		{
			Z_Free( theSV.configstrings_[i] );
		}
	}
	theSV.clear();
}

///*
//================
//SV_TouchCGame
//
//  touch the cgame.vm so that a pure client can load it if it's in a seperate pk3
//================
//*/
//void SV_TouchCGame(void) {
//	fileHandle_t	f;
//	char filename[MAX_QPATH];
//
//	Com_sprintf( filename, sizeof(filename), "vm/%s.qvm", "cgame" );
//	FS_FOpenFileRead( filename, &f, false );
//	if ( f ) {
//		FS_FCloseFile( f );
//	}
//}

/*
================
SV_SpawnServer

Change the server to a New map, taking all connected
clients along with it.
This is NOT called for map_restart
================
*/
void SV_SpawnServer( char *server, bool killBots ) 
{
	int			i;
	int			checksum;
	bool		isBot;
	char		systemInfo[16384];
	const char	*p;

	// shut down the existing game if it is running
	SV_ShutdownGameProgs();

	Com_Printf ("------ Server Initialization ------\n");
	Com_Printf ("Server: %s\n",server);

	// if not running a dedicated server CL_MapLoading will connect the client to the server
	// also print some status stuff
	CL_MapLoading();

	// make sure all the client stuff is unloaded
	CL_ShutdownAll();

	// clear the whole hunk because we're (re)loading the server
	Hunk_Clear();

	// clear collision map data
	CM_ClearMap();

	// init client structures and svs.numSnapshotEntities 
	if ( !Cvar_VariableValue("sv_running") ) {
		SV_Startup();
	} else {
		// check for maxclients change
		if ( sv_maxclients->modified ) {
			SV_ChangeMaxClients();
		}
	}

	// clear pak references
	FS_ClearPakReferences(0);

	// allocate the snapshot entities on the hunk
	theSVS.snapshotEntities_ = reinterpret_cast<entityState_t*>(
					Hunk_Alloc( sizeof(entityState_t)*theSVS.numSnapshotEntities_, h_high ));
	theSVS.nextSnapshotEntities_ = 0;

	// toggle the server bit so clients can detect that a
	// server has changed
	theSVS.snapFlagServerBit_ ^= SNAPFLAG_SERVERCOUNT;

	// set nextmap to the same map, but it may be overriden
	// by the game startup or another console command
	Cvar_Set( "nextmap", "map_restart 0");
//	Cvar_Set( "nextmap", va("map %s", server) );

	// wipe the entire per-level structure
	SV_ClearServer();
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		theSV.configstrings_[i] = CopyString("");
	}

	// make sure we are not paused
	Cvar_Set("cl_paused", "0");

	// get a New checksum feed and restart the file system
	srand(Com_Milliseconds());
	theSV.checksumFeed_ = ( ((int) rand() << 16) ^ rand() ) ^ Com_Milliseconds();
	FS_Restart( theSV.checksumFeed_ );

	CM_LoadMap( va("maps/%s.bsp", server), false, &checksum );

	// set serverinfo visible name
	Cvar_Set( "mapname", server );

	Cvar_Set( "sv_mapChecksum", va("%i",checksum) );

	// serverid should be different each time
	theSV.serverId_ = com_frameTime;
	theSV.restartedServerId_ = theSV.serverId_; // I suppose the init here is just to be safe
	theSV.checksumFeedServerId_ = theSV.serverId_;
	Cvar_Set( "sv_serverid", va("%i", theSV.serverId_ ) );

	// clear physics interaction links
	SV_ClearWorld ();
	
	// media configstring setting should be done during
	// the loading stage, so connected clients don't have
	// to load during actual gameplay
	theSV.state_ = Server::SS_LOADING;

	// load and spawn all other entities
	SV_InitGame();

	// don't allow a map_restart if game is modified
	sv_gametype->modified = false;

	// run a few frames to allow everything to settle
	for ( i = 0 ;i < 3 ; i++ ) 
	{
		//VM_Call( gvm, GAME_RUN_FRAME, svs.time );
		theSG.gameRunFrame( theSVS.time_ );
		theSVS.time_ += 100;
	}

	// create a baseline for more efficient communications
	SV_CreateBaseline ();

	for( i = 1 ; i <= sv_maxclients->integer ; i++ ) 
	{
		SV_Client* cl = theSVS.svClients_.at(i);

		if( !cl ) 
			continue;

		// send the New gamestate to all connected clients
		if( cl->state_ >= SV_Client::CS_CONNECTED )
		{
			char	*denied;

			if( cl->netchan_.remoteAddress.type == NA_BOT )
			{
				if( killBots ) 
				{
					SV_DropClient( cl, "" );
					continue;
				}
				isBot = true;
			}
			else 
			{
				isBot = false;
			}

			// connect the client again
			//denied = reinterpret_cast<char*>(VM_ExplicitArgPtr( gvm, VM_Call( gvm, GAME_CLIENT_CONNECT, i, false, isBot ) ));	// firstTime = false
			denied = theSG.clientConnect( i, false, isBot );
			if ( denied ) 
			{
				// this generally shouldn't happen, because the client
				// was connected before the level change
				SV_DropClient( cl, denied );
			} 
			else
			{
				if( !isBot ) 
				{
					// when we get the next packet from a connected client,
					// the New gamestate will be sent
					cl->state_ = SV_Client::CS_CONNECTED;
				}
				else 
				{
					EntityBase		*ent;

					cl->state_ = SV_Client::CS_ACTIVE;
//#pragma message("does the entity already exist there ??")
					ent = theSV.getEntityByNum( i );
					ent->s.number = i;
					//cl->gentity_ = ent;

					cl->deltaMessage_ = -1;
					cl->nextSnapshotTime_ = theSVS.time_;	// generate a snapshot immediately

					//VM_Call( gvm, GAME_CLIENT_BEGIN, i );
					theSG.clientBegin( i );
				}
			}
		}
	}	

	// run another frame to allow things to look at all the players
	//VM_Call( gvm, GAME_RUN_FRAME, svs.time );
	theSG.gameRunFrame( theSVS.time_ );
	theSVS.time_ += 100;

	if ( sv_pure->integer ) 
	{
		// the server sends these to the clients so they will only
		// load pk3s also loaded at the server
		p = FS_LoadedPakChecksums();
		Cvar_Set( "sv_paks", p );
		if (strlen(p) == 0) 
			Com_Printf( "WARNING: sv_pure set but no PK3 files loaded\n" );

		p = FS_LoadedPakNames();
		Cvar_Set( "sv_pakNames", p );

		// if a dedicated pure server we need to touch the cgame because it could be in a
		// seperate pk3 file and the client will need to load the latest cgame.qvm
		//if ( com_dedicated->integer ) {
		//	SV_TouchCGame();
		//}
	}
	else 
	{
		Cvar_Set( "sv_paks", "" );
		Cvar_Set( "sv_pakNames", "" );
	}
	// the server sends these to the clients so they can figure
	// out which pk3s should be auto-downloaded
	p = FS_ReferencedPakChecksums();
	Cvar_Set( "sv_referencedPaks", p );
	p = FS_ReferencedPakNames();
	Cvar_Set( "sv_referencedPakNames", p );

	// save systeminfo and serverinfo strings
	Q_strncpyz( systemInfo, Cvar_InfoString_Big( CVAR_SYSTEMINFO ), sizeof( systemInfo ) );
	cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;
	SV_SetConfigstring( CS_SYSTEMINFO, systemInfo );

	SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO ) );
	cvar_modifiedFlags &= ~CVAR_SERVERINFO;

	// any media configstring setting now should issue a warning
	// and any configstring changes should be reliably transmitted
	// to all clients
	theSV.state_ = Server::SS_GAME;

	// send a heartbeat now so the master will get up to date info
	SV_Heartbeat_f();

	Hunk_SetMark();

	Com_Printf ("-----------------------------------\n");
}

/*
===============
SV_Init

Only called at main exe startup, not for each game
===============
*/

void SV_Init (void) {
	SV_AddOperatorCommands ();

	// serverinfo vars
	Cvar_Get ("dmflags", "0", CVAR_SERVERINFO);
	Cvar_Get ("fraglimit", "20", CVAR_SERVERINFO);
	Cvar_Get ("timelimit", "0", CVAR_SERVERINFO);
	sv_gametype = Cvar_Get ("g_gametype", "0", CVAR_SERVERINFO | CVAR_LATCH );
	sv_gameset = Cvar_Get ("mf_gameset", "modern", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_LATCH );
	Cvar_Get ("sv_keywords", "", CVAR_SERVERINFO);
	Cvar_Get ("protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_ROM);
	sv_mapname = Cvar_Get ("mapname", "nomap", CVAR_SERVERINFO | CVAR_ROM);
	sv_privateClients = Cvar_Get ("sv_privateClients", "0", CVAR_SERVERINFO);
	sv_hostname = Cvar_Get ("sv_hostname", "noname", CVAR_SERVERINFO | CVAR_ARCHIVE );
	sv_maxclients = Cvar_Get ("sv_maxclients", "8", CVAR_SERVERINFO | CVAR_LATCH);

	sv_maxRate = Cvar_Get ("sv_maxRate", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_minPing = Cvar_Get ("sv_minPing", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_maxPing = Cvar_Get ("sv_maxPing", "0", CVAR_ARCHIVE | CVAR_SERVERINFO );
	sv_floodProtect = Cvar_Get ("sv_floodProtect", "1", CVAR_ARCHIVE | CVAR_SERVERINFO );

	// systeminfo
	Cvar_Get ("sv_cheats", "1", CVAR_SYSTEMINFO | CVAR_ROM );
	sv_serverid = Cvar_Get ("sv_serverid", "0", CVAR_SYSTEMINFO | CVAR_ROM );
	sv_pure = Cvar_Get ("sv_pure", "0", CVAR_SYSTEMINFO | CVAR_INIT | CVAR_ROM );
	Cvar_Get ("sv_paks", "", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_Get ("sv_pakNames", "", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_Get ("sv_referencedPaks", "", CVAR_SYSTEMINFO | CVAR_ROM );
	Cvar_Get ("sv_referencedPakNames", "", CVAR_SYSTEMINFO | CVAR_ROM );

	// server vars
	sv_rconPassword = Cvar_Get ("rconPassword", "", CVAR_TEMP );
	sv_privatePassword = Cvar_Get ("sv_privatePassword", "", CVAR_TEMP );
	sv_fps = Cvar_Get ("sv_fps", "20", CVAR_TEMP );
	sv_timeout = Cvar_Get ("sv_timeout", "200", CVAR_TEMP );
	sv_zombietime = Cvar_Get ("sv_zombietime", "2", CVAR_TEMP );
	Cvar_Get ("nextmap", "", CVAR_TEMP );

	sv_allowDownload = Cvar_Get ("sv_allowDownload", "0", CVAR_SERVERINFO);
	//sv_master[0] = Cvar_Get ("sv_master1", MASTER_SERVER_NAME, 0 );
	//sv_master[1] = Cvar_Get ("sv_master2", "", CVAR_ARCHIVE );
	//sv_master[2] = Cvar_Get ("sv_master3", "", CVAR_ARCHIVE );
	//sv_master[3] = Cvar_Get ("sv_master4", "", CVAR_ARCHIVE );
	//sv_master[4] = Cvar_Get ("sv_master5", "", CVAR_ARCHIVE );
	sv_reconnectlimit = Cvar_Get ("sv_reconnectlimit", "3", 0);
	sv_showloss = Cvar_Get ("sv_showloss", "0", 0);
	sv_padPackets = Cvar_Get ("sv_padPackets", "0", 0);
	sv_killserver = Cvar_Get ("sv_killserver", "0", 0);
	sv_mapChecksum = Cvar_Get ("sv_mapChecksum", "", CVAR_ROM);
	sv_lanForceRate = Cvar_Get ("sv_lanForceRate", "1", CVAR_ARCHIVE );
	sv_strictAuth = Cvar_Get ("sv_strictAuth", "0", CVAR_ARCHIVE );

}


/*
==================
SV_FinalMessage

Used by SV_Shutdown to send a final message to all
connected clients before the server goes down.  The messages are sent immediately,
not just stuck on the outgoing message list, because the server is going
to totally exit after returning from this function.
==================
*/
void SV_FinalMessage( char *message ) 
{

	// send it twice, ignoring rate
	for( int j = 0 ; j < 2 ; j++ ) 
	{
		for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
		{
			SV_Client	*cl = theSVS.svClients_.at(i);
	
			if( !cl )
				continue;

			if( cl->state_ >= SV_Client::CS_CONNECTED ) 
			{
				// don't send a disconnect to a local client
				if( cl->netchan_.remoteAddress.type != NA_LOOPBACK ) 
				{
					SV_SendServerCommand( cl, "print \"%s\"", message );
					SV_SendServerCommand( cl, "disconnect" );
				}
				// force a snapshot to be sent
				cl->nextSnapshotTime_ = -1;
				SV_SendClientSnapshot( cl );
			}
		}
	}
}


/*
================
SV_Shutdown

Called when each game quits,
before Sys_Quit or Sys_Error
================
*/
void SV_Shutdown( char *finalmsg ) 
{
	if ( !com_sv_running || !com_sv_running->integer ) 
		return;

	Com_Printf( "----- Server Shutdown -----\n" );

	if( theSVS.svClients_.size() && !com_errorEntered ) {
		SV_FinalMessage( finalmsg );
	}

	//SV_RemoveOperatorCommands();
	//SV_MasterShutdown();
	SV_ShutdownGameProgs();

	// free current level
	SV_ClearServer();

	// free server static data
//	if( theSVS.clients_ ) 
//		Z_Free( theSVS.clients_ );
//	theSVS.clearClients();// done in theSVS.clear()

///	Com_Memset( &svs, 0, sizeof( svs ) );
	theSVS.clear();

	Cvar_Set( "sv_running", "0" );
	Cvar_Set("ui_singlePlayerActive", "0");

	Com_Printf( "---------------------------\n" );

	// disconnect any local clients
	CL_Disconnect( false );
}

