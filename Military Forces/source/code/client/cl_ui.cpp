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

#include "../ui/ui.h"
#include "client.h"

//#include "../botlib/botlib.h"

//extern	botlib_export_t	*botlib_export;

//vm_t *uivm;
UserInterface& theUI = UserInterface::getInstance();

/*
====================
GetClientState
====================
*/
void GetClientState( uiClientState_t *state ) 
{
	state->connectPacketCount = clc.connectPacketCount;
	state->connState = clc.state;
	Q_strncpyz( state->servername, clc.servername, sizeof( state->servername ) );
	Q_strncpyz( state->updateInfoString, cls.updateInfoString, sizeof( state->updateInfoString ) );
	Q_strncpyz( state->messageString, clc.serverMessage, sizeof( state->messageString ) );
	state->clientNum = cl.snap.ps.clientNum;
}

/*
====================
LAN_LoadCachedServers
====================
*/
void LAN_LoadCachedServers( ) {
	int size;
	fileHandle_t fileIn;
	cls.numglobalservers = cls.numfavoriteservers = 0;
	cls.numGlobalServerAddresses = 0;
	if (FileSystem::baseDir_FOpenFileRead("servercache.dat", &fileIn)) {
		FileSystem::read(&cls.numglobalservers, sizeof(int), fileIn);
		FileSystem::read(&cls.numfavoriteservers, sizeof(int), fileIn);
		FileSystem::read(&size, sizeof(int), fileIn);
		if (size == sizeof(cls.globalServers) + sizeof(cls.favoriteServers)) {
			FileSystem::read(&cls.globalServers, sizeof(cls.globalServers), fileIn);
			FileSystem::read(&cls.favoriteServers, sizeof(cls.favoriteServers), fileIn);
		} else {
			cls.numglobalservers = cls.numfavoriteservers = 0;
			cls.numGlobalServerAddresses = 0;
		}
		FileSystem::fCloseFile(fileIn);
	}
}

/*
====================
LAN_SaveServersToCache
====================
*/
void LAN_SaveServersToCache( ) {
	int size;
	fileHandle_t fileOut = FileSystem::baseDir_FOpenFileWrite_HomeState("servercache.dat");
	FileSystem::write(&cls.numglobalservers, sizeof(int), fileOut);
	FileSystem::write(&cls.numfavoriteservers, sizeof(int), fileOut);
	size = sizeof(cls.globalServers) + sizeof(cls.favoriteServers);
	FileSystem::write(&size, sizeof(int), fileOut);
	FileSystem::write(&cls.globalServers, sizeof(cls.globalServers), fileOut);
	FileSystem::write(&cls.favoriteServers, sizeof(cls.favoriteServers), fileOut);
	FileSystem::fCloseFile(fileOut);
}


/*
====================
LAN_ResetPings
====================
*/
void LAN_ResetPings(int source) {
	int count,i;
	serverInfo_t *servers = nullptr;
	count = 0;

	switch (source) {
		case AS_LOCAL :
			servers = &cls.localServers[0];
			count = MAX_OTHER_SERVERS;
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			servers = &cls.globalServers[0];
			count = MAX_GLOBAL_SERVERS;
			break;
		case AS_FAVORITES :
			servers = &cls.favoriteServers[0];
			count = MAX_OTHER_SERVERS;
			break;
	}
	if (servers) {
		for (i = 0; i < count; i++) {
			servers[i].ping = -1;
		}
	}
}

/*
====================
LAN_AddServer
====================
*/
int LAN_AddServer(int source, const char *name, const char *address) {
	int max, *count, i;
	netadr_t adr;
	serverInfo_t *servers = nullptr;
	max = MAX_OTHER_SERVERS;
	count = nullptr;

	switch (source) {
		case AS_LOCAL :
			count = &cls.numlocalservers;
			servers = &cls.localServers[0];
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			max = MAX_GLOBAL_SERVERS;
			count = &cls.numglobalservers;
			servers = &cls.globalServers[0];
			break;
		case AS_FAVORITES :
			count = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}
	if (servers && *count < max) {
		Network::stringToAdr( address, &adr, NA_UNSPEC );
		for ( i = 0; i < *count; i++ ) {
			if (Network::compareAdr(servers[i].adr, adr)) {
				break;
			}
		}
		if (i >= *count) {
			servers[*count].adr = adr;
			Q_strncpyz(servers[*count].hostName, name, sizeof(servers[*count].hostName));
			servers[*count].visible = true;
			(*count)++;
			return 1;
		}
		return 0;
	}
	return -1;
}

/*
====================
LAN_RemoveServer
====================
*/
void LAN_RemoveServer(int source, const char *addr) {
	int *count, i;
	serverInfo_t *servers = nullptr;
	count = nullptr;
	switch (source) {
		case AS_LOCAL :
			count = &cls.numlocalservers;
			servers = &cls.localServers[0];
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			count = &cls.numglobalservers;
			servers = &cls.globalServers[0];
			break;
		case AS_FAVORITES :
			count = &cls.numfavoriteservers;
			servers = &cls.favoriteServers[0];
			break;
	}
	if (servers) {
		netadr_t comp;
		Network::stringToAdr( addr, &comp, NA_UNSPEC );
		for (i = 0; i < *count; i++) {
			if (Network::compareAdr( comp, servers[i].adr)) {
				int j = i;
				while (j < *count - 1) {
					Com_Memcpy(&servers[j], &servers[j+1], sizeof(servers[j]));
					j++;
				}
				(*count)--;
				break;
			}
		}
	}
}


/*
====================
LAN_GetServerCount
====================
*/
int LAN_GetServerCount( int source ) {
	switch (source) {
		case AS_LOCAL :
			return cls.numlocalservers;
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			return cls.numglobalservers;
			break;
		case AS_FAVORITES :
			return cls.numfavoriteservers;
			break;
	}
	return 0;
}

/*
====================
LAN_GetLocalServerAddressString
====================
*/
void LAN_GetServerAddressString( int source, int n, char *buf, int buflen ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				Q_strncpyz(buf, Network::adrToStringwPort( cls.localServers[n].adr) , buflen );
				return;
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				Q_strncpyz(buf, Network::adrToStringwPort( cls.globalServers[n].adr) , buflen );
				return;
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				Q_strncpyz(buf, Network::adrToStringwPort( cls.favoriteServers[n].adr) , buflen );
				return;
			}
			break;
	}
	buf[0] = '\0';
}

/*
====================
LAN_GetServerInfo
====================
*/
void LAN_GetServerInfo( int source, int n, char *buf, int buflen ) {
	char info[MAX_STRING_CHARS];
	serverInfo_t *server = nullptr;
	info[0] = '\0';
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				server = &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.favoriteServers[n];
			}
			break;
	}
	if (server && buf) {
		buf[0] = '\0';
		Info::setValueForKey( info, "hostname", server->hostName);
		Info::setValueForKey( info, "mapname", server->mapName);
		Info::setValueForKey( info, "clients", va("%i",server->clients));
		Info::setValueForKey( info, "sv_maxclients", va("%i",server->maxClients));
		Info::setValueForKey( info, "ping", va("%i",server->ping));
		Info::setValueForKey( info, "minping", va("%i",server->minPing));
		Info::setValueForKey( info, "maxping", va("%i",server->maxPing));
		Info::setValueForKey( info, "game", server->game);
		Info::setValueForKey( info, "gametype", va("%i",server->gameType));
		Info::setValueForKey( info, "nettype", va("%i",server->netType));
		Info::setValueForKey( info, "addr", Network::adrToStringwPort(server->adr));
		Info::setValueForKey( info, "punkbuster", va("%i", server->punkbuster));
		Info::setValueForKey( info, "g_needpass", va("%i", server->g_needpass));
		Info::setValueForKey( info, "g_humanplayers", va("%i", server->g_humanplayers));
		Q_strncpyz(buf, info, buflen);
	} else {
		if (buf) {
			buf[0] = '\0';
		}
	}
}

/*
====================
LAN_GetServerPing
====================
*/
int LAN_GetServerPing( int source, int n ) {
	serverInfo_t *server = nullptr;
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				server = &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				server = &cls.favoriteServers[n];
			}
			break;
	}
	if (server) {
		return server->ping;
	}
	return -1;
}

/*
====================
LAN_GetServerPtr
====================
*/
serverInfo_t *LAN_GetServerPtr( int source, int n ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return &cls.localServers[n];
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				return &cls.globalServers[n];
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return &cls.favoriteServers[n];
			}
			break;
	}
	return nullptr;
}

/*
====================
LAN_CompareServers
====================
*/
int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 ) {
	int res;
	serverInfo_t *server1, *server2;
	int clients1, clients2;

	server1 = LAN_GetServerPtr(source, s1);
	server2 = LAN_GetServerPtr(source, s2);
	if (!server1 || !server2) {
		return 0;
	}

	res = 0;
	switch( sortKey ) {
		case SORT_HOST:
			res = Q_stricmp( server1->hostName, server2->hostName );
			break;

		case SORT_MAP:
			res = Q_stricmp( server1->mapName, server2->mapName );
			break;
		case SORT_CLIENTS:
			// sub sort by max clients
			if ( server1->clients == server2->clients ) {
				clients1 = server1->maxClients;
				clients2 = server2->maxClients;
			} else {
				clients1 = server1->clients;
				clients2 = server2->clients;
			}

			if (clients1 < clients2) {
				res = -1;
			}
			else if (clients1 > clients2) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
		case SORT_GAME:
			if (server1->gameType < server2->gameType) {
				res = -1;
			}
			else if (server1->gameType > server2->gameType) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
		case SORT_PING:
			if (server1->ping < server2->ping) {
				res = -1;
			}
			else if (server1->ping > server2->ping) {
				res = 1;
			}
			else {
				res = 0;
			}
			break;
	}

	if (sortDir) {
		if (res < 0)
			return 1;
		if (res > 0)
			return -1;
		return 0;
	}
	return res;
}

/*
====================
LAN_GetPingQueueCount
====================
*/
int LAN_GetPingQueueCount( ) {
	return (CL_GetPingQueueCount());
}

/*
====================
LAN_ClearPing
====================
*/
void LAN_ClearPing( int n ) {
	CL_ClearPing( n );
}

/*
====================
LAN_GetPing
====================
*/
void LAN_GetPing( int n, char *buf, int buflen, int *pingtime ) {
	CL_GetPing( n, buf, buflen, pingtime );
}

/*
====================
LAN_GetPingInfo
====================
*/
void LAN_GetPingInfo( int n, char *buf, int buflen ) {
	CL_GetPingInfo( n, buf, buflen );
}

/*
====================
LAN_MarkServerVisible
====================
*/
void LAN_MarkServerVisible(int source, int n, bool visible ) {
	if (n == -1) {
		int count = MAX_OTHER_SERVERS;
		serverInfo_t *server = nullptr;
		switch (source) {
			case AS_LOCAL :
				server = &cls.localServers[0];
				break;
			case AS_MPLAYER:
			case AS_GLOBAL :
				server = &cls.globalServers[0];
				count = MAX_GLOBAL_SERVERS;
				break;
			case AS_FAVORITES :
				server = &cls.favoriteServers[0];
				break;
		}
		if (server) {
			for (n = 0; n < count; n++) {
				server[n].visible = visible;
			}
		}

	} else {
		switch (source) {
			case AS_LOCAL :
				if (n >= 0 && n < MAX_OTHER_SERVERS) {
					cls.localServers[n].visible = visible;
				}
				break;
			case AS_MPLAYER:
			case AS_GLOBAL :
				if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
					cls.globalServers[n].visible = visible;
				}
				break;
			case AS_FAVORITES :
				if (n >= 0 && n < MAX_OTHER_SERVERS) {
					cls.favoriteServers[n].visible = visible;
				}
				break;
		}
	}
}


/*
=======================
LAN_ServerIsVisible
=======================
*/
int LAN_ServerIsVisible(int source, int n ) {
	switch (source) {
		case AS_LOCAL :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return cls.localServers[n].visible;
			}
			break;
		case AS_MPLAYER:
		case AS_GLOBAL :
			if (n >= 0 && n < MAX_GLOBAL_SERVERS) {
				return cls.globalServers[n].visible;
			}
			break;
		case AS_FAVORITES :
			if (n >= 0 && n < MAX_OTHER_SERVERS) {
				return cls.favoriteServers[n].visible;
			}
			break;
	}
	return false;
}

/*
=======================
LAN_UpdateVisiblePings
=======================
*/
bool LAN_UpdateVisiblePings(int source ) {
	return CL_UpdateVisiblePings_f(source);
}

/*
====================
LAN_GetServerStatus
====================
*/
int LAN_GetServerStatus( const char *serverAddress, char *serverStatus, int maxLen ) {
	return CL_ServerStatus( serverAddress, serverStatus, maxLen );
}

/*
====================
CL_GetGlConfig
====================
*/
//void CL_GetGlconfig( glconfig_t *config ) {
//	*config = cls.glconfig;
//}

/*
====================
GetClipboardData
====================
*/
void GetClipboardData( char *buf, int buflen ) {
	char	*cbd;

	cbd = System::getClipboardData();

	if ( !cbd ) {
		*buf = 0;
		return;
	}

	Q_strncpyz( buf, cbd, buflen );

	Z_Free( cbd );
}

/*
====================
Key_KeynumToStringBuf
====================
*/
void Key_KeynumToStringBuf( int keynum, char *buf, int buflen ) 
{
	Q_strncpyz( buf, Key_KeynumToString( keynum ), buflen );
}

/*
====================
Key_GetBindingBuf
====================
*/
void Key_GetBindingBuf( int keynum, char *buf, int buflen ) 
{
	char	*value;

	value = Keys::getBinding( keynum );
	if ( value ) {
		Q_strncpyz( buf, value, buflen );
	}
	else {
		*buf = 0;
	}
}



///*
//====================
//CLUI_GetCDKey
//====================
//*/
//void CLUI_GetCDKey( char *buf, int buflen ) 
//{
//	cvar_t	*fs;
//	fs = CvarSystem::get ("fs_game", "", CVAR_INIT|CVAR_SYSTEMINFO );
//	if (UI_usesUniqueCDKey() && fs && fs->string[0] != 0) {
//		Com_Memcpy( buf, &cl_cdkey[16], 16);
//		buf[16] = 0;
//	} else {
//		Com_Memcpy( buf, cl_cdkey, 16);
//		buf[16] = 0;
//	}
//}


///*
//====================
//CLUI_SetCDKey
//====================
//*/
//void CLUI_SetCDKey( char *buf ) 
//{
//	cvar_t	*fs;
//	fs = CvarSystem::get ("fs_game", "", CVAR_INIT|CVAR_SYSTEMINFO );
//	if (UI_usesUniqueCDKey() && fs && fs->string[0] != 0) {
//		Com_Memcpy( &cl_cdkey[16], buf, 16 );
//		cl_cdkey[32] = 0;
//		// set the flag so the fle will be written at the next opportunity
//		cvar_modifiedFlags |= CVAR_ARCHIVE;
//	} else {
//		Com_Memcpy( cl_cdkey, buf, 16 );
//		// set the flag so the fle will be written at the next opportunity
//		cvar_modifiedFlags |= CVAR_ARCHIVE;
//	}
//}

/*
====================
GetConfigString
====================
*/
int GetConfigString(int index, char *buf, int size)
{
	int		offset;

	if (index < 0 || index >= MAX_CONFIGSTRINGS)
		return false;

	offset = cl.gameState.stringOffsets[index];
	if (!offset) {
		if( size ) {
			buf[0] = 0;
		}
		return false;
	}

	Q_strncpyz( buf, cl.gameState.stringData+offset, size);
 
	return true;
}

/*
====================
CL_ShutdownUI
====================
*/
void CL_ShutdownUI( ) {
	Key_SetCatcher( Key_GetCatcher( ) & ~KEYCATCH_UI );
	cls.uiStarted = false;
	if ( !theUI.isInitialized() ) {
		return;
	}
	theUI.shutdown();
}

/*
====================
CL_InitUI
====================
*/
void CL_InitUI( ) {
	// init for this gamestate
	theUI.init( (clc.state >= CA_AUTHORIZING && clc.state < CA_ACTIVE) );
}

//bool UI_usesUniqueCDKey() 
//{
//	if( theUI.isInitialized() )
//		return theUI.hasUniqueCDKey();
//
//	return false;
//}

/*
====================
UI_GameCommand

See if the current console command is claimed by the ui
====================
*/
bool UI_GameCommand( ) 
{
	if( !theUI.isInitialized() )
		return false;

	return theUI.consoleCommand( cls.realtime );
}
