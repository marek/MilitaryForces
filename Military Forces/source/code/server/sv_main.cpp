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

//serverStatic_t	svs;				// persistant server info
//server_t		sv;					// local server

ServerGame& theSG = ServerGame::getInstance();

Server& theSV = Server::getInstance();
ServerStatic& theSVS = ServerStatic::getInstance(); 

cvar_t	*sv_fps;				// time rate for running non-clients
cvar_t	*sv_timeout;			// seconds without any message
cvar_t	*sv_zombietime;			// seconds to sink messages after disconnect
cvar_t	*sv_rconPassword;		// password for remote server commands
cvar_t	*sv_privatePassword;	// password for the privateClient slots
cvar_t	*sv_allowDownload;
cvar_t	*sv_maxclients;

cvar_t	*sv_privateClients;		// number of clients reserved for password
cvar_t	*sv_hostname;
//cvar_t	*sv_master[MAX_MASTER_SERVERS];		// master server ip address
cvar_t	*sv_reconnectlimit;		// minimum seconds between connect messages
cvar_t	*sv_showloss;			// report when usercmds are lost
cvar_t	*sv_padPackets;			// add nop bytes to messages
cvar_t	*sv_snapshotBudget;
cvar_t	*sv_killserver;			// menu system can set to 1 to shut server down
cvar_t	*sv_mapname;
cvar_t	*sv_mapChecksum;
cvar_t	*sv_serverid;
cvar_t	*sv_minRate;
cvar_t	*sv_maxRate;
cvar_t	*sv_dlRate;
cvar_t	*sv_minPing;
cvar_t	*sv_maxPing;
cvar_t	*sv_gametype;
cvar_t	*sv_gameset;
cvar_t	*sv_pure;
cvar_t	*sv_floodProtect;
cvar_t	*sv_lanForceRate; // dedicated 1 (LAN) server forces local client rates to 99999 (bug #491)
cvar_t	*sv_strictAuth;
cvar_t	*sv_banFile;
cvar_t	*sv_master[MAX_MASTER_SERVERS];	// master server ip address

serverBan_t serverBans[SERVER_MAXBANS];
int serverBansCount = 0;

#ifdef USE_VOIP
cvar_t *sv_voip;
cvar_t *sv_voipProtocol;
#endif

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*
===============
SV_ExpandNewlines

Converts newlines to "\n" so a line prints nicer
===============
*/
char	*SV_ExpandNewlines( char *in ) {
	static	char	string[1024];
	int		l;

	l = 0;
	while ( *in && l < sizeof(string) - 3 ) {
		if ( *in == '\n' ) {
			string[l++] = '\\';
			string[l++] = 'n';
		} else {
			string[l++] = *in;
		}
		in++;
	}
	string[l] = 0;

	return string;
}

/*
======================
SV_ReplacePendingServerCommands

  This is ugly
======================
*/
int SV_ReplacePendingServerCommands( SV_Client *client, const char *cmd ) {
	int i, index, csnum1, csnum2;

	for ( i = client->reliableSent_+1; i <= client->reliableSequence_; i++ ) {
		index = i & ( MAX_RELIABLE_COMMANDS - 1 );
		//
		if ( !Q_strncmp(cmd, client->reliableCommands_[ index ], strlen("cs")) ) {
			sscanf(cmd, "cs %i", &csnum1);
			sscanf(client->reliableCommands_[ index ], "cs %i", &csnum2);
			if ( csnum1 == csnum2 ) {
				Q_strncpyz( client->reliableCommands_[ index ], cmd, sizeof( client->reliableCommands_[ index ] ) );
				/*
				if ( client->netchan_.remoteAddress.type != NA_BOT ) {
					Com_Printf( "WARNING: client %i removed double pending config string %i: %s\n", client->clientNum_, csnum1, cmd );
				}
				*/
				return true;
			}
		}
	}
	return false;
}

/*
======================
SV_AddServerCommand

The given command will be transmitted to the client, and is guaranteed to
not have future snapshot_t executed before it is executed
======================
*/
void SV_AddServerCommand( SV_Client *client, const char *cmd ) {
	int		index, i;

	// this is very ugly but it's also a waste to for instance send multiple config string updates
	// for the same config string index in one snapshot
//	if ( SV_ReplacePendingServerCommands( client, cmd ) ) {
//		return;
//	}

	// do not send commands until the gamestate has been sent
	if( client->state_ < SV_Client::CS_PRIMED )
		return;

	client->reliableSequence_++;
	// if we would be losing an old command that hasn't been acknowledged,
	// we must drop the connection
	// we check == instead of >= so a broadcast print added by SV_DropClient()
	// doesn't cause a recursive drop client
	if ( client->reliableSequence_ - client->reliableAcknowledge_ == MAX_RELIABLE_COMMANDS + 1 ) {
		if ( client->gamestateMessageNum_ == -1 )  {
			// invalid game state message 
			// this can occur in SV_DropClient() to avoid calling it more than once
			return;
		}

		Com_Printf( "===== pending server commands =====\n" );
		for ( i = client->reliableAcknowledge_ + 1 ; i <= client->reliableSequence_ ; i++ ) {
			Com_Printf( "cmd %5d: %s\n", i, client->reliableCommands_[ i & (MAX_RELIABLE_COMMANDS-1) ] );
		}
		Com_Printf( "cmd %5d: %s\n", i, cmd );
		SV_DropClient( client, "Server command overflow" );
		return;
	}
	index = client->reliableSequence_ & ( MAX_RELIABLE_COMMANDS - 1 );
	Q_strncpyz( client->reliableCommands_[ index ], cmd, sizeof( client->reliableCommands_[ index ] ) );
}


/*
=================
SV_SendServerCommand

Sends a reliable command string to be interpreted by 
the client game module: "cp", "print", "chat", etc
A NULL client will broadcast to all clients
=================
*/
void QDECL SV_SendServerCommand( SV_Client *cl, const char *fmt, ... ) 
{
	va_list		argptr;
	byte		message[MAX_MSGLEN];
	
	va_start (argptr,fmt);
	Q_vsnprintf ((char *)message, sizeof(message), fmt,argptr);
	va_end (argptr);

	// Fix to http://aluigi.altervista.org/adv/q3msgboom-adv.txt
	// The actual cause of the bug is probably further downstream
	// and should maybe be addressed later, but this certainly
	// fixes the problem for now
	if ( strlen ((char *)message) > 1022 ) {
		return;
	}

	if( cl != nullptr ) 
	{
		SV_AddServerCommand( cl, (char *)message );
		return;
	}

	// hack to echo broadcast prints to console
	if( com_dedicated->integer && !strncmp( (char *)message, "print", 5) ) 
	{
		Com_Printf ("broadcast: %s\n", SV_ExpandNewlines((char *)message) );
	}

	// send the data to all relevent clients
	for( int j = 1; j <= sv_maxclients->integer; j++ )
	{
		SV_Client* client = theSVS.svClients_.at(j);

		if( !client ) 
			continue;

		SV_AddServerCommand( client, (char *)message );
	}
}


/*
==============================================================================

MASTER SERVER FUNCTIONS

==============================================================================
*/

/*
================
SV_MasterHeartbeat

Send a message to the masters every few minutes to
let it know we are alive, and log information.
We will also have a heartbeat sent when a server
changes from empty to non-empty, and full to non-full,
but not on every player enter or exit.
================
*/
constexpr int HEARTBEAT_MSEC = 300*1000;
constexpr int MASTERDNS_MSEC = 24*60*60*1000;
void SV_MasterHeartbeat(const char *message)
{
	static netadr_t	adr[MAX_MASTER_SERVERS][2]; // [2] for v4 and v6 address for the same address string.
	int			i;
	int			res;
	int			netenabled;

	netenabled = Cvar_VariableIntegerValue("net_enabled");

	// "dedicated 1" is for lan play, "dedicated 2" is for inet public play
	if (!com_dedicated || com_dedicated->integer != 2 || !(netenabled & (NET_ENABLEV4 | NET_ENABLEV6)))
		return;		// only dedicated servers send heartbeats

	// if not time yet, don't send anything
	if ( theSVS.time_ < theSVS.nextHeartbeatTime_ )
		return;

	if ( !Q_stricmp( com_gamename->string, LEGACY_MASTER_GAMENAME ) )
		message = LEGACY_HEARTBEAT_FOR_MASTER;

	theSVS.nextHeartbeatTime_ = theSVS.time_ + HEARTBEAT_MSEC;

	// send to group masters
	for (i = 0; i < MAX_MASTER_SERVERS; i++)
	{
		if(!sv_master[i]->string[0])
			continue;

		// see if we haven't already resolved the name or if it's been over 24 hours
		// resolving usually causes hitches on win95, so only do it when needed
		if (sv_master[i]->modified || theSVS.time_ > theSVS.masterResolveTime_[i])
		{
			sv_master[i]->modified = false;
			theSVS.masterResolveTime_[i] = theSVS.time_ + MASTERDNS_MSEC;
			
			if(netenabled & NET_ENABLEV4)
			{
				Com_Printf("Resolving %s (IPv4)\n", sv_master[i]->string);
				res = NET_StringToAdr(sv_master[i]->string, &adr[i][0], NA_IP);

				if(res == 2)
				{
					// if no port was specified, use the default master port
					adr[i][0].port = BigShort(PORT_MASTER);
				}
				
				if(res)
					Com_Printf( "%s resolved to %s\n", sv_master[i]->string, NET_AdrToStringwPort(adr[i][0]));
				else
					Com_Printf( "%s has no IPv4 address.\n", sv_master[i]->string);
			}
			
			if(netenabled & NET_ENABLEV6)
			{
				Com_Printf("Resolving %s (IPv6)\n", sv_master[i]->string);
				res = NET_StringToAdr(sv_master[i]->string, &adr[i][1], NA_IP6);

				if(res == 2)
				{
					// if no port was specified, use the default master port
					adr[i][1].port = BigShort(PORT_MASTER);
				}
				
				if(res)
					Com_Printf( "%s resolved to %s\n", sv_master[i]->string, NET_AdrToStringwPort(adr[i][1]));
				else
					Com_Printf( "%s has no IPv6 address.\n", sv_master[i]->string);
			}
		}

		if(adr[i][0].type == NA_BAD && adr[i][1].type == NA_BAD)
		{
			continue;
		}


		Com_Printf ("Sending heartbeat to %s\n", sv_master[i]->string );

		// this command should be changed if the server info / status format
		// ever incompatably changes

		if(adr[i][0].type != NA_BAD)
			NET_OutOfBandPrint( NS_SERVER, adr[i][0], "heartbeat %s\n", message);
		if(adr[i][1].type != NA_BAD)
			NET_OutOfBandPrint( NS_SERVER, adr[i][1], "heartbeat %s\n", message);
	}
}

/*
=================
SV_MasterShutdown

Informs all masters that this server is going down
=================
*/
void SV_MasterShutdown( ) {
	// send a heartbeat right now
	theSVS.nextHeartbeatTime_ = -9999;
	SV_MasterHeartbeat(HEARTBEAT_FOR_MASTER);

	// send it again to minimize chance of drops
	theSVS.nextHeartbeatTime_ = -9999;
	SV_MasterHeartbeat(HEARTBEAT_FOR_MASTER);

	// when the master tries to poll the server, it won't respond, so
	// it will be removed from the list
}


/*
==============================================================================

CONNECTIONLESS COMMANDS

==============================================================================
*/

// This is deliberately quite large to make it more of an effort to DoS
constexpr int MAX_BUCKETS = 16384;
constexpr int MAX_HASHES = 1024;

static leakyBucket_t buckets[ MAX_BUCKETS ];
static leakyBucket_t *bucketHashes[ MAX_HASHES ];
leakyBucket_t outboundLeakyBucket;

/*
================
SVC_HashForAddress
================
*/
static long SVC_HashForAddress( netadr_t address ) {
	byte 		*ip = nullptr;
	size_t	size = 0;
	int			i;
	long		hash = 0;

	switch ( address.type ) {
		case NA_IP:  ip = address.ip;  size = 4; break;
		case NA_IP6: ip = address.ip6; size = 16; break;
		default: break;
	}

	for ( i = 0; i < size; i++ ) {
		hash += (long)( ip[ i ] ) * ( i + 119 );
	}

	hash = ( hash ^ ( hash >> 10 ) ^ ( hash >> 20 ) );
	hash &= ( MAX_HASHES - 1 );

	return hash;
}

/*
================
SVC_BucketForAddress

Find or allocate a bucket for an address
================
*/
static leakyBucket_t *SVC_BucketForAddress( netadr_t address, int burst, int period ) {
	leakyBucket_t	*bucket = nullptr;
	int						i;
	long					hash = SVC_HashForAddress( address );
	int						now = Sys_Milliseconds();

	for ( bucket = bucketHashes[ hash ]; bucket; bucket = bucket->next ) {
		switch ( bucket->type ) {
			case NA_IP:
				if ( memcmp( bucket->ipv._4, address.ip, 4 ) == 0 ) {
					return bucket;
				}
				break;

			case NA_IP6:
				if ( memcmp( bucket->ipv._6, address.ip6, 16 ) == 0 ) {
					return bucket;
				}
				break;

			default:
				break;
		}
	}

	for ( i = 0; i < MAX_BUCKETS; i++ ) {
		int interval;

		bucket = &buckets[ i ];
		interval = now - bucket->lastTime;

		// Reclaim expired buckets
		if ( bucket->lastTime > 0 && ( interval > ( burst * period ) ||
					interval < 0 ) ) {
			if ( bucket->prev != nullptr ) {
				bucket->prev->next = bucket->next;
			} else {
				bucketHashes[ bucket->hash ] = bucket->next;
			}
			
			if ( bucket->next != nullptr ) {
				bucket->next->prev = bucket->prev;
			}

			Com_Memset( bucket, 0, sizeof( leakyBucket_t ) );
		}

		if ( bucket->type == NA_BAD ) {
			bucket->type = address.type;
			switch ( address.type ) {
				case NA_IP:  Com_Memcpy( bucket->ipv._4, address.ip, 4 );   break;
				case NA_IP6: Com_Memcpy( bucket->ipv._6, address.ip6, 16 ); break;
				default: break;
			}

			bucket->lastTime = now;
			bucket->burst = 0;
			bucket->hash = hash;

			// Add to the head of the relevant hash chain
			bucket->next = bucketHashes[ hash ];
			if ( bucketHashes[ hash ] != nullptr ) {
				bucketHashes[ hash ]->prev = bucket;
			}

			bucket->prev = nullptr;
			bucketHashes[ hash ] = bucket;

			return bucket;
		}
	}

	// Couldn't allocate a bucket for this address
	return nullptr;
}

/*
================
SVC_RateLimit
================
*/
bool SVC_RateLimit( leakyBucket_t *bucket, int burst, int period ) {
	if ( bucket != nullptr ) {
		int now = Sys_Milliseconds();
		int interval = now - bucket->lastTime;
		int expired = interval / period;
		int expiredRemainder = interval % period;

		if ( expired > bucket->burst || interval < 0 ) {
			bucket->burst = 0;
			bucket->lastTime = now;
		} else {
			bucket->burst -= expired;
			bucket->lastTime = now - expiredRemainder;
		}

		if ( bucket->burst < burst ) {
			bucket->burst++;

			return false;
		}
	}

	return true;
}

/*
================
SVC_RateLimitAddress

Rate limit for a particular address
================
*/
bool SVC_RateLimitAddress( netadr_t from, int burst, int period ) {
	leakyBucket_t *bucket = SVC_BucketForAddress( from, burst, period );

	return SVC_RateLimit( bucket, burst, period );
}

/*
================
SVC_Status

Responds with all the info that qplug or qspy can see about the server
and all connected players.  Used for getting detailed information after
the simple info query.
================
*/
void SVC_Status( netadr_t from ) 
{
	// ignore if we are in single player
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER || Cvar_VariableValue("ui_singlePlayerActive")) 
		return;

	// Prevent using getstatus as an amplifier
	if ( SVC_RateLimitAddress( from, 10, 1000 ) ) {
		Com_DPrintf( "SVC_Status: rate limit from %s exceeded, dropping request\n",
			NET_AdrToString( from ) );
		return;
	}

	// Allow getstatus to be DoSed relatively easily, but prevent
	// excess outbound bandwidth usage when being flooded inbound
	if ( SVC_RateLimit( &outboundLeakyBucket, 10, 100 ) ) {
		Com_DPrintf( "SVC_Status: rate limit exceeded, dropping request\n" );
		return;
	}

	// A maximum challenge length of 128 should be more than plenty.
	if(strlen(Cmd_Argv(1)) > 128)
		return;

	char	infostring[MAX_INFO_STRING];
	strcpy( infostring, Cvar_InfoString( CVAR_SERVERINFO ) );

	// echo back the parameter to status. so master servers can use it as a challenge
	// to prevent timed spoofed reply packets that add ghost servers
	Info_SetValueForKey( infostring, "challenge", Cmd_Argv(1) );

	// add "demo" to the sv_keywords if restricted
	//if ( Cvar_VariableValue( "fs_restrict" ) ) {
	//	char	keywords[MAX_INFO_STRING];

	//	Com_sprintf( keywords, sizeof( keywords ), "demo %s",
	//		Info_ValueForKey( infostring, "sv_keywords" ) );
	//	Info_SetValueForKey( infostring, "sv_keywords", keywords );
	//}

	char	status[MAX_MSGLEN];
	status[0] = 0;
	int statusLength = 0;

	char	player[1024];
//	playerState_t	*ps;

	for( int i = 1; i <= sv_maxclients->integer; i++ )
	{
		SV_Client* cl = theSVS.svClients_.at(i);
		if( cl && cl->state_ >= SV_Client::CS_CONNECTED ) 
		{
			ClientBase* clientBase = theSV.getClientByNum( i );
			Com_sprintf (player, sizeof(player), "%i %i \"%s\"\n", 
				clientBase->ps_.persistant[PERS_SCORE], cl->ping_, cl->name_);
			int playerLength = strlen(player);
			if (statusLength + playerLength >= sizeof(status) ) 
				break;		// can't hold any more

			strcpy (status + statusLength, player);
			statusLength += playerLength;
		}
	}

	NET_OutOfBandPrint( NS_SERVER, from, "statusResponse\n%s\n%s", infostring, status );
}

/*
================
SVC_Info

Responds with a short info message that should be enough to determine
if a user is interested in a server to do a full status
================
*/
void SVC_Info( netadr_t from ) 
{
	// ignore if we are in single player
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER || Cvar_VariableValue("ui_singlePlayerActive")) 
		return;

	// Prevent using getinfo as an amplifier
	if ( SVC_RateLimitAddress( from, 10, 1000 ) ) {
		Com_DPrintf( "SVC_Info: rate limit from %s exceeded, dropping request\n",
			NET_AdrToString( from ) );
		return;
	}

	// Allow getinfo to be DoSed relatively easily, but prevent
	// excess outbound bandwidth usage when being flooded inbound
	if ( SVC_RateLimit( &outboundLeakyBucket, 10, 100 ) ) {
		Com_DPrintf( "SVC_Info: rate limit exceeded, dropping request\n" );
		return;
	}

	/*
	 * Check whether Cmd_Argv(1) has a sane length. This was not done in the original Quake3 version which led
	 * to the Infostring bug discovered by Luigi Auriemma. See http://aluigi.altervista.org/ for the advisory.
	 */

	// A maximum challenge length of 128 should be more than plenty.
	if(strlen(Cmd_Argv(1)) > 128)
		return;

	// don't count privateclients
	int count = 0, humans = 0;
	for( int i = sv_privateClients->integer + 1 ; i <= sv_maxclients->integer ; i++ ) 
	{
		SV_Client* cl = theSVS.svClients_.at(i);
		if( !cl )
			continue;
		if( cl->state_ >= SV_Client::CS_CONNECTED ) {
			count++;
			if (cl->netchan_.remoteAddress.type != NA_BOT) {
				humans++;
			}
		}
	}

	char	infostring[MAX_INFO_STRING];
	infostring[0] = 0;

	// echo back the parameter to status. so servers can use it as a challenge
	// to prevent timed spoofed reply packets that add ghost servers
	Info_SetValueForKey( infostring, "challenge", Cmd_Argv(1) );

	Info_SetValueForKey( infostring, "gamename", com_gamename->string );
	Info_SetValueForKey(infostring, "protocol", va("%i", com_protocol->integer));
	Info_SetValueForKey( infostring, "hostname", sv_hostname->string );
	Info_SetValueForKey( infostring, "mapname", sv_mapname->string );
	Info_SetValueForKey( infostring, "clients", va("%i", count) );
	Info_SetValueForKey(infostring, "g_humanplayers", va("%i", humans));
	Info_SetValueForKey( infostring, "sv_maxclients", 
		va("%i", sv_maxclients->integer - sv_privateClients->integer ) );
	Info_SetValueForKey( infostring, "gametype", va("%i", sv_gametype->integer ) );
	Info_SetValueForKey( infostring, "pure", va("%i", sv_pure->integer ) );
	Info_SetValueForKey(infostring, "g_needpass", va("%d", Cvar_VariableIntegerValue("g_needpass")));

	if( sv_minPing->integer ) 
		Info_SetValueForKey( infostring, "minPing", va("%i", sv_minPing->integer) );

	if( sv_maxPing->integer ) 
		Info_SetValueForKey( infostring, "maxPing", va("%i", sv_maxPing->integer) );

	char* gamedir = Cvar_VariableString( "fs_game" );
	if( *gamedir ) 
		Info_SetValueForKey( infostring, "game", gamedir );

	NET_OutOfBandPrint( NS_SERVER, from, "infoResponse\n%s", infostring );
}

/*
================
SVC_FlushRedirect

================
*/
void SV_FlushRedirect( char *outputbuf ) 
{
	NET_OutOfBandPrint( NS_SERVER, theSVS.redirectAddress_, "print\n%s", outputbuf );
}

/*
===============
SVC_RemoteCommand

An rcon packet arrived from the network.
Shift down the remaining args
Redirect all printfs
===============
*/
void SVC_RemoteCommand( netadr_t from, Message *msg ) 
{
	bool	valid;
	char		remaining[1024];
	// TTimo - scaled down to accumulate, but not overflow anything network wise, print wise etc.
	// (OOB messages are the bottleneck here)
#define SV_OUTPUTBUF_LENGTH (1024 - 16)
	char		sv_outputbuf[SV_OUTPUTBUF_LENGTH];
	char *cmd_aux;

	// Prevent using rcon as an amplifier and make dictionary attacks impractical
	if ( SVC_RateLimitAddress( from, 10, 1000 ) ) {
		Com_DPrintf( "SVC_RemoteCommand: rate limit from %s exceeded, dropping request\n",
			NET_AdrToString( from ) );
		return;
	}

	if ( !strlen( sv_rconPassword->string ) ||
		strcmp (Cmd_Argv(1), sv_rconPassword->string) ) {
		static leakyBucket_t bucket;

		// Make DoS via rcon impractical
		if ( SVC_RateLimit( &bucket, 10, 1000 ) ) {
			Com_DPrintf( "SVC_RemoteCommand: rate limit exceeded, dropping request\n" );
			return;
		}

		valid = false;
		Com_Printf ("Bad rcon from %s: %s\n", NET_AdrToString (from), Cmd_ArgsFrom(2) );
	} else {
		valid = true;
		Com_Printf ("Rcon from %s: %s\n", NET_AdrToString (from), Cmd_ArgsFrom(2) );
	}

	// start redirecting all print outputs to the packet
	theSVS.redirectAddress_ = from;
	Com_BeginRedirect (sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);

	if ( !strlen( sv_rconPassword->string ) ) 
	{
		Com_Printf ("No rconpassword set on the server.\n");
	} 
	else if ( !valid ) 
	{
		Com_Printf ("Bad rconpassword.\n");
	} 
	else 
	{
		remaining[0] = 0;
		
		// get the command directly, "rcon <pass> <command>" to avoid quoting issues
		// extract the command by walking
		// since the cmd formatting can fuckup (amount of spaces), using a dumb step by step parsing
		cmd_aux = Cmd_Cmd();
		cmd_aux+=4;
		while(cmd_aux[0]==' ')
			cmd_aux++;
		while(cmd_aux[0] && cmd_aux[0]!=' ') // password
			cmd_aux++;
		while(cmd_aux[0]==' ')
			cmd_aux++;
		
		Q_strcat( remaining, sizeof(remaining), cmd_aux);
		
		Cmd_ExecuteString (remaining);

	}

	Com_EndRedirect ();
}

/*
=================
SV_ConnectionlessPacket

A connectionless packet has four leading 0xff
characters to distinguish it from a game channel.
Clients that are in the game can still send
connectionless packets.
=================
*/
void SV_ConnectionlessPacket( netadr_t from, Message *msg ) {
	char	*s;
	char	*c;

	msg->beginReadingOOB();
	msg->readLong();		// skip the -1 marker

	if (!Q_strncmp("connect", reinterpret_cast<const char*>(&msg->data[4]), 7)) {
		Huff_Decompress(msg, 12);
	}

	s = msg->readStringLine();
	Cmd_TokenizeString( s );

	c = Cmd_Argv(0);
	Com_DPrintf ("SV packet %s : %s\n", NET_AdrToString(from), c);

	if (!Q_stricmp(c, "getstatus")) {
		SVC_Status( from  );
  } else if (!Q_stricmp(c, "getinfo")) {
		SVC_Info( from );
	} else if (!Q_stricmp(c, "getchallenge")) {
		SV_GetChallenge( from );
	} else if (!Q_stricmp(c, "connect")) {
		SV_DirectConnect( from );
	//} else if (!Q_stricmp(c, "ipAuthorize")) {
	//	SV_AuthorizeIpPacket( from );
	} else if (!Q_stricmp(c, "rcon")) {
		SVC_RemoteCommand( from, msg );
	} else if (!Q_stricmp(c, "disconnect")) {
		// if a client starts up a local server, we may see some spurious
		// server disconnect messages when their New server sees our final
		// sequenced messages to the old client
	} else {
		Com_DPrintf ("bad connectionless packet from %s:\n%s\n"
		, NET_AdrToString (from), s);
	}
}

//============================================================================

/*
=================
SV_ReadPackets
=================
*/
void SV_PacketEvent( netadr_t from, Message *msg ) 
{
	// check for connectionless packet (0xffffffff) first
	if( msg->cursize >= 4 && *(int *)msg->data == -1) 
	{
		SV_ConnectionlessPacket( from, msg );
		return;
	}

	// read the qport out of the message so we can fix up
	// stupid address translating routers
	msg->beginReadingOOB();
	msg->readLong();				// sequence number
	int qport = msg->readShort() & 0xffff;

	// find which client the message is from
	for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
	{
		SV_Client *cl = theSVS.svClients_.at(i);

		if( !cl || cl->state_ == SV_Client::CS_FREE ) 
			continue;
		
		if( !NET_CompareBaseAdr( from, cl->netchan_.remoteAddress ) ) 
			continue;
		
		// it is possible to have multiple clients from a single IP
		// address, so they are differentiated by the qport variable
		if( cl->netchan_.qport != qport ) 
			continue;
		

		// the IP port can't be used to differentiate them, because
		// some address translating routers periodically change UDP
		// port assignments
		if( cl->netchan_.remoteAddress.port != from.port )
		{
			Com_Printf( "SV_PacketEvent: fixing up a translated port\n" );
			cl->netchan_.remoteAddress.port = from.port;
		}

		// make sure it is a valid, in sequence packet
		if( SV_Netchan_Process(cl, msg) ) 
		{
			// zombie clients still need to do the Netchan_Process
			// to make sure they don't need to retransmit the final
			// reliable message, but they don't do any other processing
			if( cl->state_ != SV_Client::CS_ZOMBIE )
			{
				cl->lastPacketTime_ = theSVS.time_;	// don't timeout
				SV_ExecuteClientMessage( cl, msg );
			}
		}
		return;
	}
}


/*
===================
SV_CalcPings

Updates the cl->ping variables
===================
*/
void SV_CalcPings() 
{
	int			total, count;
	int			delta;

	for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
	{
		SV_Client *cl = theSVS.svClients_.at(i);

		if( !cl )
			continue;

		if( cl->state_ != SV_Client::CS_ACTIVE ) 
		{
			cl->ping_ = 999;
			continue;
		}
		EntityBase* gentity = theSV.getEntityByNum(i);
		if( !gentity ) 
		{
			cl->ping_ = 999;
			continue;
		}
		if( gentity->r.svFlags & SVF_BOT ) 
		{
			cl->ping_ = 0;
			continue;
		}

		total = 0;
		count = 0;
		for( int j = 0 ; j < PACKET_BACKUP ; j++ ) 
		{
			if ( cl->frames_[j].messageAcked_ <= 0 ) 
				continue;

			delta = cl->frames_[j].messageAcked_ - cl->frames_[j].messageSent_;
			count++;
			total += delta;
		}
		if( !count ) 
		{
			cl->ping_ = 999;
		} 
		else 
		{
			cl->ping_ = total/count;
			if ( cl->ping_ > 999 )
			{
				cl->ping_ = 999;
			}
		}

		// let the game dll know about the ping
		ClientBase* cb = theSV.getClientByNum( i );
		cb->ps_.ping = cl->ping_;
	}
}

/*
==================
SV_CheckTimeouts

If a packet has not been received from a client for timeout->integer 
seconds, drop the conneciton.  Server time is used instead of
realtime to avoid dropping the local client while debugging.

When a client is normally dropped, the client_t goes into a zombie state
for a few seconds to make sure any final reliable message gets resent
if necessary
==================
*/
void SV_CheckTimeouts() 
{
	int droppoint = theSVS.time_ - 1000 * sv_timeout->integer;
	int zombiepoint = theSVS.time_ - 1000 * sv_zombietime->integer;

	for( int i = 1; i <= sv_maxclients->integer ; i++) 
	{
		SV_Client* cl = theSVS.svClients_.at(i);
		if( !cl )
			continue;

		// message times may be wrong across a changelevel
		if( cl->lastPacketTime_ > theSVS.time_ ) 
			cl->lastPacketTime_ = theSVS.time_;

		if( cl->state_ == SV_Client::CS_ZOMBIE
			&& cl->lastPacketTime_ < zombiepoint ) 
		{
			// using the client id cause the cl->name is empty at this point
			Com_DPrintf( "Going from CS_ZOMBIE to CS_FREE for client %d\n", i );
			cl->state_ = SV_Client::CS_FREE;	// can now be reused
			continue;
		}
		if( cl->state_ >= SV_Client::CS_CONNECTED && cl->lastPacketTime_ < droppoint ) 
		{
			// wait several frames so a debugger session doesn't
			// cause a timeout
			if( ++cl->timeoutCount_ > 5 ) 
			{
				SV_DropClient (cl, "timed out"); 
				cl->state_ = SV_Client::CS_FREE;	// don't bother with zombie state
			}
		}
		else 
		{
			cl->timeoutCount_ = 0;
		}
	}
}


/*
==================
SV_CheckPaused
==================
*/
bool SV_CheckPaused() 
{
	if ( !cl_paused->integer ) 
		return false;

	// only pause if there is just a single client connected
	int count = 0;
	for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
	{
		SV_Client	*cl = theSVS.svClients_.at(i);
		if( !cl )
			continue;

		if( cl->state_ >= SV_Client::CS_CONNECTED && cl->netchan_.remoteAddress.type != NA_BOT ) 
			count++;
	}

	if( count > 1 ) 
	{
		// don't pause
		if (sv_paused->integer)
			Cvar_Set("sv_paused", "0");
		return false;
	}

	if (!sv_paused->integer)
		Cvar_Set("sv_paused", "1");
	return true;
}

/*
==================
SV_FrameMsec
Return time in millseconds until processing of the next server frame.
==================
*/
int SV_FrameMsec()
{
	if(sv_fps)
	{
		int frameMsec;
		
		frameMsec = 1000.0f / sv_fps->value;
		
		if(frameMsec < theSV.timeResidual_)
			return 0;
		else
			return frameMsec - theSV.timeResidual_;
	}
	else
		return 1;
}

/*
==================
SV_Frame

Player movement occurs as a result of packet events, which
happen before SV_Frame is called
==================
*/
void SV_Frame( int msec ) 
{
	int		frameMsec;
	int		startTime;

	// the menu kills the server with this cvar
	if ( sv_killserver->integer ) 
	{
		SV_Shutdown ("Server was killed");
		Cvar_Set( "sv_killserver", "0" );
		return;
	}

	if ( !com_sv_running->integer ) 
		return;

	// allow pause if only the local client is connected
	if ( SV_CheckPaused() ) 
		return;

	// if it isn't time for the next frame, do nothing
	if ( sv_fps->integer < 1 ) 
		Cvar_Set( "sv_fps", "10" );

	frameMsec = 1000 / sv_fps->integer * com_timescale->value;
	// don't let it scale below 1ms
	if(frameMsec < 1)
	{
		Cvar_Set("timescale", va("%f", sv_fps->integer / 1000.0f));
		frameMsec = 1;
	}

	theSV.timeResidual_ += msec;

	// if time is about to hit the 32nd bit, kick all clients
	// and clear sv.time, rather
	// than checking for negative time wraparound everywhere.
	// 2giga-milliseconds = 23 days, so it won't be too often
	if ( theSVS.time_ > 0x70000000 )
	{
		SV_Shutdown( "Restarting server due to time wrapping" );
		Cbuf_AddText( va( "map %s\n", Cvar_VariableString( "mapname" ) ) );
		return;
	}
	// this can happen considerably earlier when lots of clients play and the map doesn't change
	if ( theSVS.nextSnapshotEntities_ >= 0x7FFFFFFE - theSVS.numSnapshotEntities_ )
	{
		SV_Shutdown( "Restarting server due to numSnapshotEntities wrapping" );
		Cbuf_AddText( va( "map %s\n", Cvar_VariableString( "mapname" ) ) );
		return;
	}

	if( theSV.restartTime_ && theSV.time_ >= theSV.restartTime_ ) 
	{
		theSV.restartTime_ = 0;
		Cbuf_AddText( "map_restart 0\n" );
		return;
	}

	// update infostrings if anything has been changed
	if ( cvar_modifiedFlags & CVAR_SERVERINFO ) 
	{
		SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO ) );
		cvar_modifiedFlags &= ~CVAR_SERVERINFO;
	}
	if ( cvar_modifiedFlags & CVAR_SYSTEMINFO ) 
	{
		SV_SetConfigstring( CS_SYSTEMINFO, Cvar_InfoString_Big( CVAR_SYSTEMINFO ) );
		cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;
	}

	if ( com_speeds->integer ) 
	{
		startTime = Sys_Milliseconds ();
	}
	else 
	{
		startTime = 0;	// quite a compiler warning
	}

	// update ping based on the all received frames
	SV_CalcPings();

	// run the game simulation in chunks
	while ( theSV.timeResidual_ >= frameMsec ) 
	{
		theSV.timeResidual_ -= frameMsec;
		theSVS.time_ += frameMsec;
		theSV.time_ += frameMsec;

		// let everything in the world think and move
		//VM_Call( gvm, GAME_RUN_FRAME, svs.time );
		theSG.gameRunFrame( theSV.time_ );
	}

	if ( com_speeds->integer ) 
		time_game = Sys_Milliseconds () - startTime;

	// check timeouts
	SV_CheckTimeouts();

	// send messages back to the clients
	SV_SendClientMessages();

	// send a heartbeat to the master if needed
	SV_MasterHeartbeat(HEARTBEAT_FOR_MASTER);
}

/*
====================
SV_RateMsec

Return the number of msec until another message can be sent to
a client based on its rate settings
====================
*/

constexpr int UDPIP_HEADER_SIZE = 28;
constexpr int UDPIP6_HEADER_SIZE = 48;

int SV_RateMsec(SV_Client *client)
{
	int rate, rateMsec;
	int messageSize;
	
	messageSize = client->netchan_.lastSentSize;
	rate = client->rate_;

	if(sv_maxRate->integer)
	{
		if(sv_maxRate->integer < 1000)
			Cvar_Set( "sv_MaxRate", "1000" );
		if(sv_maxRate->integer < rate)
			rate = sv_maxRate->integer;
	}

	if(sv_minRate->integer)
	{
		if(sv_minRate->integer < 1000)
			Cvar_Set("sv_minRate", "1000");
		if(sv_minRate->integer > rate)
			rate = sv_minRate->integer;
	}

	if(client->netchan_.remoteAddress.type == NA_IP6)
		messageSize += UDPIP6_HEADER_SIZE;
	else
		messageSize += UDPIP_HEADER_SIZE;
		
	rateMsec = messageSize * 1000 / ((int) (rate * com_timescale->value));
	rate = Sys_Milliseconds() - client->netchan_.lastSentTime;
	
	if(rate > rateMsec)
		return 0;
	else
		return rateMsec - rate;
}

/*
====================
SV_SendQueuedPackets

Send download messages and queued packets in the time that we're idle, i.e.
not computing a server frame or sending client snapshots.
Return the time in msec until we expect to be called next
====================
*/

int SV_SendQueuedPackets()
{
	int numBlocks;
	int dlStart, deltaT, delayT;
	static int dlNextRound = 0;
	int timeVal = INT_MAX;

	// Send out fragmented packets now that we're idle
	delayT = SV_SendQueuedMessages();
	if(delayT >= 0)
		timeVal = delayT;

	if(sv_dlRate->integer)
	{
		// Rate limiting. This is very imprecise for high
		// download rates due to millisecond timedelta resolution
		dlStart = Sys_Milliseconds();
		deltaT = dlNextRound - dlStart;

		if(deltaT > 0)
		{
			if(deltaT < timeVal)
				timeVal = deltaT + 1;
		}
		else
		{
			numBlocks = SV_SendDownloadMessages();

			if(numBlocks)
			{
				// There are active downloads
				deltaT = Sys_Milliseconds() - dlStart;

				delayT = 1000 * numBlocks * MAX_DOWNLOAD_BLKSIZE;
				delayT /= sv_dlRate->integer * 1024;

				if(delayT <= deltaT + 1)
				{
					// Sending the last round of download messages
					// took too long for given rate, don't wait for
					// next round, but always enforce a 1ms delay
					// between DL message rounds so we don't hog
					// all of the bandwidth. This will result in an
					// effective maximum rate of 1MB/s per user, but the
					// low download window size limits this anyways.
					if(timeVal > 2)
						timeVal = 2;

					dlNextRound = dlStart + deltaT + 1;
				}
				else
				{
					dlNextRound = dlStart + delayT;
					delayT -= deltaT;

					if(delayT < timeVal)
						timeVal = delayT;
				}
			}
		}
	}
	else
	{
		if(SV_SendDownloadMessages())
			timeVal = 0;
	}

	return timeVal;
}

//============================================================================

bool SV_GetEntityToken( char *buffer, int bufferSize )
{
	const char	*s;

	s = COM_Parse( &theSV.entityParsePoint_ );
	Q_strncpyz( buffer, s, bufferSize );
	if ( !theSV.entityParsePoint_ && !s[0] ) 
	{
		return false;
	}
	else 
	{
		return true;
	}
}
