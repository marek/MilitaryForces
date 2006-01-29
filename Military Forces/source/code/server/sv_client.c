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
// sv_client.c -- server code for dealing with clients

#include "server.h"
#include "sv_server.h"
#include "sv_client.h"
#include "../game/game.h"



static void SV_CloseDownload( SV_Client *cl );

/*
=================
SV_GetChallenge

A "getchallenge" OOB command has been received
Returns a challenge number that can be used
in a subsequent connectResponse command.
We do this to prevent denial of service attacks that
flood the server with invalid connection IPs.  With a
challenge, they must give a valid IP address.

If we are authorizing, a challenge request will cause a packet
to be sent to the authorize server.

When an authorizeip is returned, a challenge response will be
sent to that ip.
=================
*/
void SV_GetChallenge( netadr_t from ) 
{
	int		i;
	int		oldest;
	int		oldestTime;
	ServerStatic::Challenge	*challenge;

	// ignore if we are in single player
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER || Cvar_VariableValue("ui_singlePlayerActive")) 
		return;

	oldest = 0;
	oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	challenge = &theSVS.challenges_[0];
	for (i = 0 ; i < ServerStatic::k_MAX_CHALLENGES ; i++, challenge++) 
	{
		if ( !challenge->connected_ && NET_CompareAdr( from, challenge->adr_ ) ) 
			break;
	
		if ( challenge->time_ < oldestTime ) 
		{
			oldestTime = challenge->time_;
			oldest = i;
		}
	}

	if (i == ServerStatic::k_MAX_CHALLENGES) 
	{
		// this is the first time this client has asked for a challenge
		challenge = &theSVS.challenges_[oldest];

		challenge->challenge_ = ( (rand() << 16) ^ rand() ) ^ theSVS.time_;
		challenge->adr_ = from;
		challenge->firstTime_ = theSVS.time_;
		challenge->time_ = theSVS.time_;
		challenge->connected_ = false;
		i = oldest;
	}

	// if they are on a lan address, send the challengeResponse immediately
	//if ( Sys_IsLANAddress( from ) ) {
		challenge->pingTime_ = theSVS.time_;
		NET_OutOfBandPrint( NS_SERVER, from, "challengeResponse %i", challenge->challenge_ );
		return;
	//}

	//// look up the authorize server's IP
	//if ( !svs.authorizeAddress.ip[0] && svs.authorizeAddress.type != NA_BAD ) {
	//	Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
	//	if ( !NET_StringToAdr( AUTHORIZE_SERVER_NAME, &svs.authorizeAddress ) ) {
	//		Com_Printf( "Couldn't resolve address\n" );
	//		return;
	//	}
	//	svs.authorizeAddress.port = BigShort( PORT_AUTHORIZE );
	//	Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
	//		svs.authorizeAddress.ip[0], svs.authorizeAddress.ip[1],
	//		svs.authorizeAddress.ip[2], svs.authorizeAddress.ip[3],
	//		BigShort( svs.authorizeAddress.port ) );
	//}

	// if they have been challenging for a long time and we
	// haven't heard anything from the authorize server, go ahead and
	// let them in, assuming the id server is down
	//if ( svs.time - challenge->firstTime > AUTHORIZE_TIMEOUT ) {
	//	Com_DPrintf( "authorize server timed out\n" );

	//	challenge->pingTime = svs.time;
	//	NET_OutOfBandPrint( NS_SERVER, challenge->adr, 
	//		"challengeResponse %i", challenge->challenge );
	//	return;
	//}

	//// otherwise send their ip to the authorize server
	//if ( svs.authorizeAddress.type != NA_BAD ) {
	//	cvar_t	*fs;
	//	char	game[1024];

	//	Com_DPrintf( "sending getIpAuthorize for %s\n", NET_AdrToString( from ));
	//	
	//	strcpy(game, BASEGAME);
	//	fs = Cvar_Get ("fs_game", "", CVAR_INIT|CVAR_SYSTEMINFO );
	//	if (fs && fs->string[0] != 0) {
	//		strcpy(game, fs->string);
	//	}
	//	
	//	// the 0 is for backwards compatibility with obsolete sv_allowanonymous flags
	//	// getIpAuthorize <challenge> <IP> <game> 0 <auth-flag>
	//	NET_OutOfBandPrint( NS_SERVER, svs.authorizeAddress,
	//		"getIpAuthorize %i %i.%i.%i.%i %s 0 %s",  svs.challenges[i].challenge,
	//		from.ip[0], from.ip[1], from.ip[2], from.ip[3], game, sv_strictAuth->string );
	//}
}

///*
//====================
//SV_AuthorizeIpPacket
//
//A packet has been returned from the authorize server.
//If we have a challenge adr for that ip, send the
//challengeResponse to it
//====================
//*/
//void SV_AuthorizeIpPacket( netadr_t from ) 
//{
//	int		challenge;
//	int		i;
//	char	*s;
//	char	*r;
//	char	ret[1024];
//
//	if ( !NET_CompareBaseAdr( from, theSVS.authorizeAddress_ ) ) 
//	{
//		Com_Printf( "SV_AuthorizeIpPacket: not from authorize server\n" );
//		return;
//	}
//
//	challenge = atoi( Cmd_Argv( 1 ) );
//
//	for (i = 0 ; i < MAX_CHALLENGES ; i++) {
//		if ( svs.challenges[i].challenge == challenge ) {
//			break;
//		}
//	}
//	if ( i == MAX_CHALLENGES ) {
//		Com_Printf( "SV_AuthorizeIpPacket: challenge not found\n" );
//		return;
//	}
//
//	// send a packet back to the original client
//	svs.challenges[i].pingTime = svs.time;
//	s = Cmd_Argv( 2 );
//	r = Cmd_Argv( 3 );			// reason
//
//	//if ( !Q_stricmp( s, "demo" ) ) {
//	//	if ( Cvar_VariableValue( "fs_restrict" ) ) {
//	//		// a demo client connecting to a demo server
//	//		NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, 
//	//			"challengeResponse %i", svs.challenges[i].challenge );
//	//		return;
//	//	}
//	//	// they are a demo client trying to connect to a real server
//	//	NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nServer is not a demo server\n" );
//	//	// clear the challenge record so it won't timeout and let them through
//	//	Com_Memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
//	//	return;
//	//}
//	if ( !Q_stricmp( s, "accept" ) ) {
//		NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, 
//			"challengeResponse %i", svs.challenges[i].challenge );
//		return;
//	}
//	if ( !Q_stricmp( s, "unknown" ) ) {
//		if (!r) {
//			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nAwaiting CD key authorization\n" );
//		} else {
//			sprintf(ret, "print\n%s\n", r);
//			NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, ret );
//		}
//		// clear the challenge record so it won't timeout and let them through
//		Com_Memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
//		return;
//	}
//
//	// authorization failed
//	if (!r) {
//		NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nSomeone is using this CD Key\n" );
//	} else {
//		sprintf(ret, "print\n%s\n", r);
//		NET_OutOfBandPrint( NS_SERVER, svs.challenges[i].adr, ret );
//	}
//
//	// clear the challenge record so it won't timeout and let them through
//	Com_Memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
//}

/*
==================
SV_DirectConnect

A "connect" OOB command has been received
==================
*/

//#define PB_MESSAGE "PunkBuster Anti-Cheat software must be installed " \
//				"and Enabled in order to join this server. An updated game patch can be downloaded from " \
//				"www.idsoftware.com"

void SV_DirectConnect( netadr_t from ) 
{
	Com_DPrintf ("SVC_DirectConnect ()\n");

	char		userinfo[MAX_INFO_STRING];
	Q_strncpyz( userinfo, Cmd_Argv(1), sizeof(userinfo) );

	int version = atoi( Info_ValueForKey( userinfo, "protocol" ) );
	if ( version != PROTOCOL_VERSION ) 
	{
		NET_OutOfBandPrint( NS_SERVER, from, "print\nServer uses protocol version %i.\n", PROTOCOL_VERSION );
		Com_DPrintf ("    rejected connect from version %i\n", version);
		return;
	}

	int challenge = atoi( Info_ValueForKey( userinfo, "challenge" ) );
	int qport = atoi( Info_ValueForKey( userinfo, "qport" ) );

	// quick reject
	SV_Client* cl;
	for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
	{
		cl = theSVS.svClients_.at(i);

		if( !cl || cl->state_ == SV_Client::CS_FREE ) 
			continue;

		if( NET_CompareBaseAdr( from, cl->netchan_.remoteAddress )
			&& ( cl->netchan_.qport == qport 
			|| from.port == cl->netchan_.remoteAddress.port ) ) 
		{
			if (( theSVS.time_ - cl->lastConnectTime_) 
				< (sv_reconnectlimit->integer * 1000)) 
			{
				Com_DPrintf ("%s:reconnect rejected : too soon\n", NET_AdrToString (from));
				return;
			}
			break;
		}
	}

	// see if the challenge is valid (LAN clients don't need to challenge)
	if( !NET_IsLocalAddress (from) ) 
	{
		int		ping;

		for( int i = 0; i < ServerStatic::k_MAX_CHALLENGES ; i++ ) 
		{
			if( NET_CompareAdr(from, theSVS.challenges_[i].adr_) ) 
			{
				if ( challenge == theSVS.challenges_[i].challenge_ ) 
					break;		// good
			}
		}
		if( i == ServerStatic::k_MAX_CHALLENGES ) 
		{
			NET_OutOfBandPrint( NS_SERVER, from, "print\nNo or bad challenge for address.\n" );
			return;
		}
		// force the IP key/value pair so the game can filter based on ip
		Info_SetValueForKey( userinfo, "ip", NET_AdrToString( from ) );

		ping = theSVS.time_ - theSVS.challenges_[i].pingTime_;
		Com_Printf( "Client %i connecting with %i challenge ping\n", i, ping );
		theSVS.challenges_[i].connected_ = true;

		// never reject a LAN client based on ping
		if ( !Sys_IsLANAddress( from ) ) 
		{
			if ( sv_minPing->value && ping < sv_minPing->value ) 
			{
				// don't let them keep trying until they get a big delay
				NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is for high pings only\n" );
				Com_DPrintf ("Client %i rejected on a too low ping\n", i);
				// reset the address otherwise their ping will keep increasing
				// with each connect message and they'd eventually be able to connect
				theSVS.challenges_[i].adr_.port = 0;
				return;
			}
			if ( sv_maxPing->value && ping > sv_maxPing->value ) 
			{
				NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is for low pings only\n" );
				Com_DPrintf ("Client %i rejected on a too high ping\n", i);
				return;
			}
		}
	} 
	else 
	{
		// force the "ip" info key to "localhost"
		Info_SetValueForKey( userinfo, "ip", "localhost" );
	}

	//MAC_STATIC SV_Client temp;
	SV_Client	*newcl = 0;//&temp;
	//Com_Memset( newcl, 0, sizeof(SV_Client) );

	// if there is already a slot for this ip, reuse it
	for( int i = 1; i <= sv_maxclients->integer; i++ )
	{
		cl = theSVS.svClients_.at(i); 

		if( !cl || cl->state_ == SV_Client::CS_FREE ) 
			continue;

		if( NET_CompareBaseAdr( from, cl->netchan_.remoteAddress )
			&& ( cl->netchan_.qport == qport 
			|| from.port == cl->netchan_.remoteAddress.port ) ) 
		{
			Com_Printf ("%s:reconnect\n", NET_AdrToString (from));
			newcl = cl;
			newcl->clientNum_ = i;

			// this doesn't work because it nukes the players userinfo

//			// disconnect the client from the game first so any flags the
//			// player might have are dropped
//			VM_Call( gvm, GAME_CLIENT_DISCONNECT, newcl - svs.clients );
			//
			goto gotnewcl;
		}
	}

	// find a client slot
	// if "sv_privateClients" is set > 0, then that number
	// of client slots will be reserved for connections that
	// have "password" set to the value of "sv_privatePassword"
	// Info requests will report the maxclients as if the private
	// slots didn't exist, to prevent people from trying to connect
	// to a full server.
	// This is to allow us to reserve a couple slots here on our
	// servers so we can play without having to kick people.

	// check for privateClient password
	int	startIndex;
	char* password = Info_ValueForKey( userinfo, "password" );
	if ( !strcmp( password, sv_privatePassword->string ) ) 
	{
		startIndex = 1;
	} 
	else
	{
		// skip past the reserved slots
		startIndex = sv_privateClients->integer;
	}

	newcl = 0;
	for( int i = startIndex; i <= sv_maxclients->integer ; i++ ) 
	{
		cl = theSVS.svClients_.at(i);
		if( !cl )
		{
			newcl = new SV_Client;
			newcl->clientNum_ = i;
			break;
		}
		if( cl->state_ == SV_Client::CS_FREE ) 
		{
			newcl = cl;
			newcl->clientNum_ = i;
			break;
		}
	}

	int	count;
	if( !newcl )
	{
		if( NET_IsLocalAddress( from ) ) 
		{
			count = 0;
			for( i = startIndex; i <= sv_maxclients->integer ; i++ ) 
			{
				cl = theSVS.svClients_.at(i);
				if( cl->netchan_.remoteAddress.type == NA_BOT ) 
					count++;
			}
			// if they're all bots
			if (count >= sv_maxclients->integer - startIndex) 
			{
				SV_DropClient(theSVS.svClients_.at(sv_maxclients->integer), "only bots on server");
				newcl = theSVS.svClients_.at(sv_maxclients->integer);
				newcl->clientNum_ = i;
			}
			else 
			{
				Com_Error( ERR_FATAL, "server is full on local connect\n" );
				return;
			}
		}
		else 
		{
			NET_OutOfBandPrint( NS_SERVER, from, "print\nServer is full.\n" );
			Com_DPrintf ("Rejected a connection.\n");
			return;
		}
	}

	// we got a newcl, so reset the reliableSequence and reliableAcknowledge
	newcl->reliableAcknowledge_ = 0;
	newcl->reliableSequence_ = 0;

gotnewcl:	
	// build a New connection
	// accept the New client
	// this is the only place a client_t is ever initialized
	//*newcl = temp;
	int clientNum = newcl->clientNum_;
	newcl->clear();
	newcl->clientNum_ = clientNum;
	// add it to the list
	theSVS.svClients_.at( clientNum ) = newcl;
	// as we dont have entities yet at this time we set to 0
	//EntityBase* ent = theSV.getEntityByNum( clientNum );
	//newcl->gentity_ = ent;

	// save the challenge
	newcl->challenge_ = challenge;

	// save the address
	Netchan_Setup (NS_SERVER, &newcl->netchan_ , from, qport);
	// init the netchan queue
	newcl->netchan_end_queue_ = &newcl->netchan_start_queue_;

	// save the userinfo
	Q_strncpyz( newcl->userinfo_, userinfo, sizeof(newcl->userinfo_) );

	// get the game a chance to reject this connection or modify the userinfo
	//denied = (char *)VM_Call( gvm, GAME_CLIENT_CONNECT, clientNum, true, false ); // firstTime = true
	char* denied = theSG.clientConnect( clientNum, true, false );
	if ( denied ) 
	{
		// we can't just use VM_ArgPtr, because that is only valid inside a VM_Call
		//denied = reinterpret_cast<char*>(VM_ExplicitArgPtr( gvm, (int)denied ));

		NET_OutOfBandPrint( NS_SERVER, from, "print\n%s\n", denied );
		Com_DPrintf ("Game rejected a connection: %s.\n", denied);
		return;
	}

	SV_UserinfoChanged( newcl );

	// send the connect packet to the client
	NET_OutOfBandPrint( NS_SERVER, from, "connectResponse" );

	Com_DPrintf( "Going from CS_FREE to CS_CONNECTED for %s\n", newcl->name_ );

	newcl->state_ = SV_Client::CS_CONNECTED;
	newcl->nextSnapshotTime_ = theSVS.time_;
	newcl->lastPacketTime_ = theSVS.time_;
	newcl->lastConnectTime_ = theSVS.time_;
	
	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	newcl->gamestateMessageNum_ = -1;

	// if this was the first client on the server, or the last client
	// the server can hold, send a heartbeat to the master.
	count = 0;
	for( int i = 1; i <= sv_maxclients->integer; i++ ) 
	{
		SV_Client* client = theSVS.svClients_.at(i);
		if( !client )
			continue;
		if( client->state_ >= SV_Client::CS_CONNECTED )
			count++;
	}
	if( count == 1 || count == sv_maxclients->integer )
		SV_Heartbeat_f();
}


/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing -- SV_FinalMessage() will handle that
=====================
*/
void SV_DropClient( SV_Client *drop, const char *reason ) 
{
	if( drop->state_ == SV_Client::CS_ZOMBIE ) 
		return;		// already dropped

	int i;
	EntityBase* gentity = theSV.getEntityByNum( drop->clientNum_ );
	if( !gentity || !(gentity->r.svFlags & SVF_BOT) ) 
	{
		// see if we already have a challenge for this ip
		ServerStatic::Challenge	*challenge = &theSVS.challenges_[0];

		for( i = 0; i < ServerStatic::k_MAX_CHALLENGES ; i++, challenge++ ) 
		{
			if( NET_CompareAdr( drop->netchan_.remoteAddress, challenge->adr_ ) ) 
			{
				challenge->connected_ = false;
				break;
			}
		}
	}

	// Kill any download
	SV_CloseDownload( drop );

	// tell everyone why they got dropped
	SV_SendServerCommand( NULL, "print \"%s" S_COLOR_WHITE " %s\n\"", drop->name_, reason );

	Com_DPrintf( "Going to CS_ZOMBIE for %s\n", drop->name_ );
	drop->state_ = SV_Client::CS_ZOMBIE;		// become free in a few seconds

	if( drop->download_ )	
	{
		FS_FCloseFile( drop->download_ );
		drop->download_ = 0;
	}

	// call the prog function for removing a client
	// this will remove the body, among other things
	//VM_Call( gvm, GAME_CLIENT_DISCONNECT, drop - svs.clients );
	theSG.clientDisconnect( drop->clientNum_ );

	// add the disconnect command
	SV_SendServerCommand( drop, "disconnect \"%s\"", reason);

	// nuke user info
	SV_SetUserinfo( drop->clientNum_, "" );

	// if this was the last client on the server, send a heartbeat
	// to the master so it is known the server is empty
	// send a heartbeat now so the master will get up to date info
	// if there is already a slot for this ip, reuse it
	for( i = 1; i <= sv_maxclients->integer; i++ ) 
	{
		SV_Client* cl = theSVS.svClients_.at(i);
		if( !cl )
			continue;
		if( cl->state_ >= SV_Client::CS_CONNECTED ) 
			break;
	}
	if( i > sv_maxclients->integer ) 
		SV_Heartbeat_f();
}

/*
================
SV_SendClientGameState

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each New map load.

It will be resent if the client acknowledges a later message but has
the wrong gamestate.
================
*/
void SV_SendClientGameState( SV_Client *client ) 
{
	int				start;
	entityState_t	*base, nullstate;
	msg_t			msg;
	byte			msgBuffer[MAX_MSGLEN];

 	Com_DPrintf ("SV_SendClientGameState() for %s\n", client->name_);
	Com_DPrintf( "Going from CS_CONNECTED to CS_PRIMED for %s\n", client->name_ );
	client->state_ = SV_Client::CS_PRIMED;
	client->pureAuthentic_ = 0;
	client->gotCP_ = false;

	// when we receive the first packet from the client, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	client->gamestateMessageNum_ = client->netchan_.outgoingSequence;

	MSG_Init( &msg, msgBuffer, sizeof( msgBuffer ) );

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand_ );

	// send any server commands waiting to be sent first.
	// we have to do this cause we send the client->reliableSequence
	// with a gamestate and it sets the clc.serverCommandSequence at
	// the client side
	SV_UpdateServerCommandsToClient( client, &msg );

	// send the gamestate
	MSG_WriteByte( &msg, svc_gamestate );
	MSG_WriteLong( &msg, client->reliableSequence_ );

	// write the configstrings
	for ( start = 0 ; start < MAX_CONFIGSTRINGS ; start++ ) 
	{
		if (theSV.configstrings_[start][0]) 
		{
			MSG_WriteByte( &msg, svc_configstring );
			MSG_WriteShort( &msg, start );
			MSG_WriteBigString( &msg, theSV.configstrings_[start] );
		}
	}

	// write the baselines
	Com_Memset( &nullstate, 0, sizeof( nullstate ) );
	for ( start = 0 ; start < MAX_GENTITIES; start++ ) 
	{
		base = &theSV.serverEntities_[start].baseline_;
		if ( !base->number ) 
			continue;

		MSG_WriteByte( &msg, svc_baseline );
		MSG_WriteDeltaEntity( &msg, &nullstate, base, true );
	}

	MSG_WriteByte( &msg, svc_EOF );

	MSG_WriteLong( &msg, client->clientNum_ );

	// write the checksum feed
	MSG_WriteLong( &msg, theSV.checksumFeed_);

	// deliver this to the client
	SV_SendMessageToClient( &msg, client );
}


/*
==================
SV_ClientEnterWorld
==================
*/
void SV_ClientEnterWorld( SV_Client *client, usercmd_t *cmd )
{
	int			clientNum;
	EntityBase *ent;

	Com_DPrintf( "Going from CS_PRIMED to CS_ACTIVE for %s\n", client->name_ );
	client->state_ = SV_Client::CS_ACTIVE;

	// set up the entity for the client
	clientNum = client->clientNum_;
	ent = theSV.getEntityByNum( clientNum );
	ent->s.number = clientNum;
	//client->gentity_ = ent;

	client->deltaMessage_ = -1;
	client->nextSnapshotTime_ = theSVS.time_;	// generate a snapshot immediately
	client->lastUsercmd_ = *cmd;

	// call the game begin function
	//VM_Call( gvm, GAME_CLIENT_BEGIN, client - svs.clients );
	theSG.clientBegin( client->clientNum_ );
}

/*
============================================================

CLIENT COMMAND EXECUTION

============================================================
*/

/*
==================
SV_CloseDownload

clear/free any download vars
==================
*/
static void SV_CloseDownload( SV_Client *cl ) 
{
	int i;

	// EOF
	if (cl->download_) 
		FS_FCloseFile( cl->download_ );

	cl->download_ = 0;
	*cl->downloadName_ = 0;

	// Free the temporary buffer space
	for (i = 0; i < MAX_DOWNLOAD_WINDOW; i++) 
	{
		if (cl->downloadBlocks_[i]) 
		{
			Z_Free( cl->downloadBlocks_[i] );
			cl->downloadBlocks_[i] = NULL;
		}
	}

}

/*
==================
SV_StopDownload_f

Abort a download if in progress
==================
*/
void SV_StopDownload_f( SV_Client *cl ) 
{
	if( *cl->downloadName_ )
		Com_DPrintf( "clientDownload: %d : file \"%s\" aborted\n", cl->clientNum_, cl->downloadName_ );

	SV_CloseDownload( cl );
}

/*
==================
SV_DoneDownload_f

Downloads are finished
==================
*/
void SV_DoneDownload_f( SV_Client *cl ) 
{
	Com_DPrintf( "clientDownload: %s Done\n", cl->name_);
	// resend the game state to update any clients that entered during the download
	SV_SendClientGameState(cl);
}

/*
==================
SV_NextDownload_f

The argument will be the last acknowledged block from the client, it should be
the same as cl->downloadClientBlock
==================
*/
void SV_NextDownload_f( SV_Client *cl )
{
	int block = atoi( Cmd_Argv(1) );

	if (block == cl->downloadClientBlock_) 
	{
		Com_DPrintf( "clientDownload: %d : client acknowledge of block %d\n", cl->clientNum_, block );

		// Find out if we are done.  A zero-length block indicates EOF
		if (cl->downloadBlockSize_[cl->downloadClientBlock_ % MAX_DOWNLOAD_WINDOW] == 0)
		{
			Com_Printf( "clientDownload: %d : file \"%s\" completed\n", cl->clientNum_, cl->downloadName_ );
			SV_CloseDownload( cl );
			return;
		}

		cl->downloadSendTime_ = theSVS.time_;
		cl->downloadClientBlock_++;
		return;
	}
	// We aren't getting an acknowledge for the correct block, drop the client
	// FIXME: this is bad... the client will never parse the disconnect message
	//			because the cgame isn't loaded yet
	SV_DropClient( cl, "broken download" );
}

/*
==================
SV_BeginDownload_f
==================
*/
void SV_BeginDownload_f( SV_Client *cl ) 
{

	// Kill any existing download
	SV_CloseDownload( cl );

	// cl->downloadName is non-zero now, SV_WriteDownloadToClient will see this and open
	// the file itself
	Q_strncpyz( cl->downloadName_, Cmd_Argv(1), sizeof(cl->downloadName_) );
}

/*
==================
SV_WriteDownloadToClient

Check to see if the client wants a file, open it if needed and start pumping the client
Fill up msg with data 
==================
*/
void SV_WriteDownloadToClient( SV_Client *cl , msg_t *msg )
{
	int curindex;
	int rate;
	int blockspersnap;
	int idPack, missionPack;
	char errorMessage[1024];

	if (!*cl->downloadName_)
		return;	// Nothing being downloaded

	if (!cl->download_) 
	{
		// We open the file here

		Com_Printf( "clientDownload: %d : begining \"%s\"\n", cl->clientNum_, cl->downloadName_ );

		missionPack = FS_idPak(cl->downloadName_, "missionpack");
		idPack = missionPack || FS_idPak(cl->downloadName_, "mfdata");

		if ( !sv_allowDownload->integer || idPack ||
			( cl->downloadSize_ = FS_SV_FOpenFileRead( cl->downloadName_, &cl->download_ ) ) <= 0 )
		{
			// cannot auto-download file
			if (idPack) 
			{
				Com_Printf("clientDownload: %d : \"%s\" cannot download id pk3 files\n", cl->clientNum_, 
					cl->downloadName_);
				if (missionPack) 
				{
					Com_sprintf(errorMessage, sizeof(errorMessage), "Cannot autodownload Team Arena file \"%s\"\n"
									"The Team Arena mission pack can be found in your local game store.", 
									cl->downloadName_);
				}
				else 
				{
					Com_sprintf(errorMessage, sizeof(errorMessage), "Cannot autodownload id pk3 file \"%s\"", 
						cl->downloadName_);
				}
			} 
			else if ( !sv_allowDownload->integer ) 
			{
				Com_Printf("clientDownload: %d : \"%s\" download disabled", cl->clientNum_, cl->downloadName_);
				if (sv_pure->integer) 
				{
					Com_sprintf(errorMessage, sizeof(errorMessage), 
						"Could not download \"%s\" because autodownloading is disabled on the server.\n\n"
										"You will need to get this file elsewhere before you "
										"can connect to this pure server.\n", cl->downloadName_);
				} 
				else 
				{
					Com_sprintf(errorMessage, sizeof(errorMessage), 
						"Could not download \"%s\" because autodownloading is disabled on the server.\n\n"
                    "The server you are connecting to is not a pure server, "
                    "set autodownload to No in your settings and you might be "
                    "able to join the game anyway.\n", cl->downloadName_);
				}
			} 
			else 
			{
        // NOTE TTimo this is NOT supposed to happen unless bug in our filesystem scheme?
        //   if the pk3 is referenced, it must have been found somewhere in the filesystem
				Com_Printf("clientDownload: %d : \"%s\" file not found on server\n", cl->clientNum_, 
					cl->downloadName_);
				Com_sprintf(errorMessage, sizeof(errorMessage), "File \"%s\" not found on server for autodownloading.\n", 
					cl->downloadName_);
			}
			MSG_WriteByte( msg, svc_download );
			MSG_WriteShort( msg, 0 ); // client is expecting block zero
			MSG_WriteLong( msg, -1 ); // illegal file size
			MSG_WriteString( msg, errorMessage );

			*cl->downloadName_ = 0;
			return;
		}
 
		// Init
		cl->downloadCurrentBlock_ = cl->downloadClientBlock_ = cl->downloadXmitBlock_ = 0;
		cl->downloadCount_ = 0;
		cl->downloadEOF_ = false;
	}

	// Perform any reads that we need to
	while (cl->downloadCurrentBlock_ - cl->downloadClientBlock_ < MAX_DOWNLOAD_WINDOW &&
		cl->downloadSize_ != cl->downloadCount_) 
	{

		curindex = (cl->downloadCurrentBlock_ % MAX_DOWNLOAD_WINDOW);

		if (!cl->downloadBlocks_[curindex])
			cl->downloadBlocks_[curindex] = reinterpret_cast<unsigned char*>(Z_Malloc( MAX_DOWNLOAD_BLKSIZE ));

		cl->downloadBlockSize_[curindex] = FS_Read( cl->downloadBlocks_[curindex], MAX_DOWNLOAD_BLKSIZE, cl->download_ );

		if (cl->downloadBlockSize_[curindex] < 0) 
		{
			// EOF right now
			cl->downloadCount_ = cl->downloadSize_;
			break;
		}

		cl->downloadCount_ += cl->downloadBlockSize_[curindex];

		// Load in next block
		cl->downloadCurrentBlock_++;
	}

	// Check to see if we have eof condition and add the EOF block
	if (cl->downloadCount_ == cl->downloadSize_ &&
		!cl->downloadEOF_ &&
		cl->downloadCurrentBlock_ - cl->downloadClientBlock_ < MAX_DOWNLOAD_WINDOW)
	{

		cl->downloadBlockSize_[cl->downloadCurrentBlock_ % MAX_DOWNLOAD_WINDOW] = 0;
		cl->downloadCurrentBlock_++;

		cl->downloadEOF_ = true;  // We have added the EOF block
	}

	// Loop up to window size times based on how many blocks we can fit in the
	// client snapMsec and rate

	// based on the rate, how many bytes can we fit in the snapMsec time of the client
	// normal rate / snapshotMsec calculation
	rate = cl->rate_;
	if ( sv_maxRate->integer ) 
	{
		if ( sv_maxRate->integer < 1000 ) 
			Cvar_Set( "sv_MaxRate", "1000" );

		if ( sv_maxRate->integer < rate ) 
			rate = sv_maxRate->integer;
	}

	if (!rate) 
	{
		blockspersnap = 1;
	} 
	else 
	{
		blockspersnap = ( (rate * cl->snapshotMsec_) / 1000 + MAX_DOWNLOAD_BLKSIZE ) /
			MAX_DOWNLOAD_BLKSIZE;
	}

	if (blockspersnap < 0)
		blockspersnap = 1;

	while (blockspersnap--)
	{

		// Write out the next section of the file, if we have already reached our window,
		// automatically start retransmitting

		if (cl->downloadClientBlock_ == cl->downloadCurrentBlock_)
			return; // Nothing to transmit

		if (cl->downloadXmitBlock_ == cl->downloadCurrentBlock_)
		{
			// We have transmitted the complete window, should we start resending?

			//FIXME:  This uses a hardcoded one second timeout for lost blocks
			//the timeout should be based on client rate somehow
			if (theSVS.time_ - cl->downloadSendTime_ > 1000)
				cl->downloadXmitBlock_ = cl->downloadClientBlock_;
			else
				return;
		}

		// Send current block
		curindex = (cl->downloadXmitBlock_ % MAX_DOWNLOAD_WINDOW);

		MSG_WriteByte( msg, svc_download );
		MSG_WriteShort( msg, cl->downloadXmitBlock_ );

		// block zero is special, contains file size
		if ( cl->downloadXmitBlock_ == 0 )
			MSG_WriteLong( msg, cl->downloadSize_ );
 
		MSG_WriteShort( msg, cl->downloadBlockSize_[curindex] );

		// Write the block
		if ( cl->downloadBlockSize_[curindex] ) {
			MSG_WriteData( msg, cl->downloadBlocks_[curindex], cl->downloadBlockSize_[curindex] );
		}

		Com_DPrintf( "clientDownload: %d : writing block %d\n", cl->clientNum_, cl->downloadXmitBlock_ );

		// Move on to the next block
		// It will get sent with next snap shot.  The rate will keep us in line.
		cl->downloadXmitBlock_++;

		cl->downloadSendTime_ = theSVS.time_;
	}
}

/*
=================
SV_Disconnect_f

The client is going to disconnect, so remove the connection immediately  FIXME: move to game?
=================
*/
static void SV_Disconnect_f( SV_Client *cl ) 
{
	SV_DropClient( cl, "disconnected" );
}

/*
=================
SV_VerifyPaks_f

If we are pure, disconnect the client if they do no meet the following conditions:

1. the first two checksums match our view of cgame and ui
2. there are no any additional checksums that we do not have

This routine would be a bit simpler with a goto but i abstained

=================
*/
static void SV_VerifyPaks_f( SV_Client *cl ) 
{
	int nChkSum1, nChkSum2, nClientPaks, nServerPaks, i, j, nCurArg;
	int nClientChkSum[1024];
	int nServerChkSum[1024];
	const char *pPaks, *pArg;
	bool bGood = true;

	// if we are pure, we "expect" the client to load certain things from 
	// certain pk3 files, namely we want the client to have loaded the
	// ui and cgame that we think should be loaded based on the pure setting
	//
	if ( sv_pure->integer != 0 ) 
	{

		bGood = true;
		nChkSum1 = nChkSum2 = 0;
		// we run the game, so determine which cgame and ui the client "should" be running
		//bGood = (FS_FileIsInPAK("vm/cgame.qvm", &nChkSum1) == 1);
		//if (bGood)
		//	bGood = (FS_FileIsInPAK("vm/ui.qvm", &nChkSum2) == 1);

		nClientPaks = Cmd_Argc();

		// start at arg 2 ( skip serverId cl_paks )
		nCurArg = 1;

		pArg = Cmd_Argv(nCurArg++);
		if(!pArg) 
		{
			bGood = false;
		}
		else
		{
			// we may get incoming cp sequences from a previous checksumFeed, which we need to ignore
			// since serverId is a frame count, it always goes up
			if (atoi(pArg) < theSV.checksumFeedServerId_)
			{
				Com_DPrintf("ignoring outdated cp command from client %s\n", cl->name_);
				return;
			}
		}
	
		// we basically use this while loop to avoid using 'goto' :)
		while (bGood) 
		{

			// must be at least 6: "cl_paks cgame ui @ firstref ... numChecksums"
			// numChecksums is encoded
			if (nClientPaks < 6)
			{
				bGood = false;
				break;
			}
			// verify first to be the cgame checksum
			pArg = Cmd_Argv(nCurArg++);
			if (!pArg || *pArg == '@' || atoi(pArg) != nChkSum1 ) 
			{
				bGood = false;
				break;
			}
			// verify the second to be the ui checksum
			pArg = Cmd_Argv(nCurArg++);
			if (!pArg || *pArg == '@' || atoi(pArg) != nChkSum2 ) 
			{
				bGood = false;
				break;
			}
			// should be sitting at the delimeter now
			pArg = Cmd_Argv(nCurArg++);
			if (*pArg != '@')
			{
				bGood = false;
				break;
			}
			// store checksums since tokenization is not re-entrant
			for (i = 0; nCurArg < nClientPaks; i++) 
			{
				nClientChkSum[i] = atoi(Cmd_Argv(nCurArg++));
			}

			// store number to compare against (minus one cause the last is the number of checksums)
			nClientPaks = i - 1;

			// make sure none of the client check sums are the same
			// so the client can't send 5 the same checksums
			for (i = 0; i < nClientPaks; i++) 
			{
				for (j = 0; j < nClientPaks; j++)
				{
					if (i == j)
						continue;
					if (nClientChkSum[i] == nClientChkSum[j]) 
					{
						bGood = false;
						break;
					}
				}
				if (bGood == false)
					break;
			}
			if (bGood == false)
				break;

			// get the pure checksums of the pk3 files loaded by the server
			pPaks = FS_LoadedPakPureChecksums();
			Cmd_TokenizeString( pPaks );
			nServerPaks = Cmd_Argc();
			if (nServerPaks > 1024)
				nServerPaks = 1024;

			for (i = 0; i < nServerPaks; i++) 
				nServerChkSum[i] = atoi(Cmd_Argv(i));

			// check if the client has provided any pure checksums of pk3 files not loaded by the server
			for (i = 0; i < nClientPaks; i++) 
			{
				for (j = 0; j < nServerPaks; j++)
				{
					if (nClientChkSum[i] == nServerChkSum[j]) 
						break;
				}
				if (j >= nServerPaks) 
				{
					bGood = false;
					break;
				}
			}
			if ( bGood == false )
				break;

			// check if the number of checksums was correct
			nChkSum1 = theSV.checksumFeed_;
			for (i = 0; i < nClientPaks; i++) 
				nChkSum1 ^= nClientChkSum[i];

			nChkSum1 ^= nClientPaks;
			if (nChkSum1 != nClientChkSum[nClientPaks]) 
			{
				bGood = false;
				break;
			}

			// break out
			break;
		}

		cl->gotCP_ = true;

		if (bGood) 
		{
			cl->pureAuthentic_ = 1;
		} 
		else 
		{
			cl->pureAuthentic_ = 0;
			cl->nextSnapshotTime_ = -1;
			cl->state_ = SV_Client::CS_ACTIVE;
			SV_SendClientSnapshot( cl );
			SV_DropClient( cl, "Unpure client detected. Invalid .PK3 files referenced!" );
		}
	}
}

/*
=================
SV_ResetPureClient_f
=================
*/
static void SV_ResetPureClient_f( SV_Client *cl ) 
{
	cl->pureAuthentic_ = 0;
	cl->gotCP_ = false;
}

/*
=================
SV_UserinfoChanged

Pull specific info from a newly changed userinfo string
into a more C friendly form.
=================
*/
void SV_UserinfoChanged( SV_Client *cl ) 
{
	char	*val;
	int		i;

	// name for C code
	Q_strncpyz( cl->name_, Info_ValueForKey (cl->userinfo_, "name"), sizeof(cl->name_) );

	// rate command

	// if the client is on the same subnet as the server and we aren't running an
	// internet public server, assume they don't need a rate choke
	if ( Sys_IsLANAddress( cl->netchan_.remoteAddress ) && com_dedicated->integer != 2 && sv_lanForceRate->integer == 1) 
	{
		cl->rate_ = 99999;	// lans should not rate limit
	} 
	else 
	{
		val = Info_ValueForKey (cl->userinfo_, "rate");
		if (strlen(val))
		{
			i = atoi(val);
			cl->rate_ = i;
			if (cl->rate_ < 1000)
			{
				cl->rate_ = 1000;
			} else if (cl->rate_ > 90000)
			{
				cl->rate_ = 90000;
			}
		} 
		else 
		{
			cl->rate_ = 3000;
		}
	}
	val = Info_ValueForKey (cl->userinfo_, "handicap");
	if (strlen(val)) 
	{
		i = atoi(val);
		if (i<=0 || i>100 || strlen(val) > 4)
		{
			Info_SetValueForKey( cl->userinfo_, "handicap", "100" );
		}
	}

	// snaps command
	val = Info_ValueForKey (cl->userinfo_, "snaps");
	if (strlen(val)) 
	{
		i = atoi(val);
		if ( i < 1 ) 
		{
			i = 1;
		} 
		else if ( i > 30 ) 
		{
			i = 30;
		}
		cl->snapshotMsec_ = 1000/i;
	} 
	else 
	{
		cl->snapshotMsec_ = 50;
	}
	
	// TTimo
	// maintain the IP information
	// this is set in SV_DirectConnect (directly on the server, not transmitted), may be lost when client updates it's userinfo
	// the banning code relies on this being consistently present
	val = Info_ValueForKey (cl->userinfo_, "ip");
	if (!val[0])
	{
		//Com_DPrintf("Maintain IP in userinfo for '%s'\n", cl->name);
		if ( !NET_IsLocalAddress(cl->netchan_.remoteAddress) )
			Info_SetValueForKey( cl->userinfo_, "ip", NET_AdrToString( cl->netchan_.remoteAddress ) );
		else
			// force the "ip" info key to "localhost" for local clients
			Info_SetValueForKey( cl->userinfo_, "ip", "localhost" );
	}
}


/*
==================
SV_UpdateUserinfo_f
==================
*/
static void SV_UpdateUserinfo_f( SV_Client *cl ) 
{
	Q_strncpyz( cl->userinfo_, Cmd_Argv(1), sizeof(cl->userinfo_) );

	SV_UserinfoChanged( cl );
	// call prog code to allow overrides
	//VM_Call( gvm, GAME_CLIENT_USERINFO_CHANGED, cl - svs.clients );
	theSG.clientUserInfoChanged( cl->clientNum_ );
}

typedef struct 
{
	char	*name;
	void	(*func)( SV_Client *cl );
} ucmd_t;

static ucmd_t ucmds[] = {
	{"userinfo", SV_UpdateUserinfo_f},
	{"disconnect", SV_Disconnect_f},
	{"cp", SV_VerifyPaks_f},
	{"vdr", SV_ResetPureClient_f},
	{"download", SV_BeginDownload_f},
	{"nextdl", SV_NextDownload_f},
	{"stopdl", SV_StopDownload_f},
	{"donedl", SV_DoneDownload_f},

	{NULL, NULL}
};

/*
==================
SV_ExecuteClientCommand

Also called by bot code
==================
*/
void SV_ExecuteClientCommand( SV_Client *cl, const char *s, bool clientOK ) 
{
	ucmd_t	*u;
	bool bProcessed = false;
	
	Cmd_TokenizeString( s );

	// see if it is a server level command
	for (u=ucmds ; u->name ; u++) 
	{
		if (!strcmp (Cmd_Argv(0), u->name) ) 
		{
			u->func( cl );
			bProcessed = true;
			break;
		}
	}

	if (clientOK) 
	{
		// pass unknown strings to the game
		if (!u->name && theSV.state_ == Server::SS_GAME) 
		{
			//VM_Call( gvm, GAME_CLIENT_COMMAND, cl - svs.clients );
			theSG.clientCommand( cl->clientNum_ );
		}
	}
	else if (!bProcessed)
		Com_DPrintf( "client text ignored for %s: %s\n", cl->name_, Cmd_Argv(0) );
}

/*
===============
SV_ClientCommand
===============
*/
static bool SV_ClientCommand( SV_Client *cl, msg_t *msg ) 
{
	int		seq;
	const char	*s;
	bool clientOk = true;

	seq = MSG_ReadLong( msg );
	s = MSG_ReadString( msg );

	// see if we have already executed it
	if ( cl->lastClientCommand_ >= seq ) 
		return true;

	Com_DPrintf( "clientCommand: %s : %i : %s\n", cl->name_, seq, s );

	// drop the connection if we have somehow lost commands
	if ( seq > cl->lastClientCommand_ + 1 ) 
	{
		Com_Printf( "Client %s lost %i clientCommands\n", cl->name_, 
			seq - cl->lastClientCommand_ + 1 );
		SV_DropClient( cl, "Lost reliable commands" );
		return false;
	}

	// malicious users may try using too many string commands
	// to lag other players.  If we decide that we want to stall
	// the command, we will stop processing the rest of the packet,
	// including the usercmd.  This causes flooders to lag themselves
	// but not other people
	// We don't do this when the client hasn't been active yet since its
	// normal to spam a lot of commands when downloading
	if ( !com_cl_running->integer && 
		cl->state_ >= SV_Client::CS_ACTIVE &&
		sv_floodProtect->integer && 
		theSVS.time_ < cl->nextReliableTime_ ) 
	{
		// ignore any other text messages from this client but let them keep playing
		// TTimo - moved the ignored verbose to the actual processing in SV_ExecuteClientCommand, only printing if the core doesn't intercept
		clientOk = false;
	} 

	// don't allow another command for one second
	cl->nextReliableTime_ = theSVS.time_ + 1000;

	SV_ExecuteClientCommand( cl, s, clientOk );

	cl->lastClientCommand_ = seq;
	Com_sprintf(cl->lastClientCommandString_, sizeof(cl->lastClientCommandString_), "%s", s);

	return true;		// continue procesing
}


//==================================================================================


/*
==================
SV_ClientThink

Also called by bot code
==================
*/
void SV_ClientThink (SV_Client *cl, usercmd_t *cmd) 
{
	cl->lastUsercmd_ = *cmd;

	if ( cl->state_ != SV_Client::CS_ACTIVE ) 
		return;		// may have been kicked during the last usercmd

	//VM_Call( gvm, GAME_CLIENT_THINK, cl - svs.clients );
	theSG.clientThink( cl->clientNum_ );
}

/*
==================
SV_UserMove

The message usually contains all the movement commands 
that were in the last three packets, so that the information
in dropped packets can be recovered.

On very fast clients, there may be multiple usercmd packed into
each of the backup packets.
==================
*/
static void SV_UserMove( SV_Client *cl, msg_t *msg, bool delta ) 
{
	int			i, key;
	int			cmdCount;
	usercmd_t	nullcmd;
	usercmd_t	cmds[MAX_PACKET_USERCMDS];
	usercmd_t	*cmd, *oldcmd;

	if ( delta ) 
		cl->deltaMessage_ = cl->messageAcknowledge_;
	else 
		cl->deltaMessage_ = -1;

	cmdCount = MSG_ReadByte( msg );

	if ( cmdCount < 1 )
	{
		Com_Printf( "cmdCount < 1\n" );
		return;
	}

	if ( cmdCount > MAX_PACKET_USERCMDS ) 
	{
		Com_Printf( "cmdCount > MAX_PACKET_USERCMDS\n" );
		return;
	}

	// use the checksum feed in the key
	key = theSV.checksumFeed_;
	// also use the message acknowledge
	key ^= cl->messageAcknowledge_;
	// also use the last acknowledged server command in the key
	key ^= Com_HashKey(cl->reliableCommands_[ cl->reliableAcknowledge_ & (MAX_RELIABLE_COMMANDS-1) ], 32);

	Com_Memset( &nullcmd, 0, sizeof(nullcmd) );
	oldcmd = &nullcmd;
	for ( i = 0 ; i < cmdCount ; i++ ) 
	{
		cmd = &cmds[i];
		MSG_ReadDeltaUsercmdKey( msg, key, oldcmd, cmd );
		oldcmd = cmd;
	}

	// save time for ping calculation
	cl->frames_[ cl->messageAcknowledge_ & PACKET_MASK ].messageAcked_ = theSVS.time_;

	// TTimo
	// catch the no-cp-yet situation before SV_ClientEnterWorld
	// if CS_ACTIVE, then it's time to trigger a New gamestate emission
	// if not, then we are getting remaining parasite usermove commands, which we should ignore
	if (sv_pure->integer != 0 && cl->pureAuthentic_ == 0 && !cl->gotCP_) 
	{
		if (cl->state_ == SV_Client::CS_ACTIVE)
		{
			// we didn't get a cp yet, don't assume anything and just send the gamestate all over again
			Com_DPrintf( "%s: didn't get cp command, resending gamestate\n", cl->name_, cl->state_ );
			SV_SendClientGameState( cl );
		}
		return;
	}			
	
	// if this is the first usercmd we have received
	// this gamestate, put the client into the world
	if ( cl->state_ == SV_Client::CS_PRIMED ) 
	{
		SV_ClientEnterWorld( cl, &cmds[0] );
		// the moves can be processed normaly
	}
	
	// a bad cp command was sent, drop the client
	if (sv_pure->integer != 0 && cl->pureAuthentic_ == 0) 
	{		
		SV_DropClient( cl, "Cannot validate pure client!");
		return;
	}

	if ( cl->state_ != SV_Client::CS_ACTIVE ) 
	{
		cl->deltaMessage_ = -1;
		return;
	}

	// usually, the first couple commands will be duplicates
	// of ones we have previously received, but the servertimes
	// in the commands will cause them to be immediately discarded
	for ( i =  0 ; i < cmdCount ; i++ ) 
	{
		// if this is a cmd from before a map_restart ignore it
		if ( cmds[i].serverTime > cmds[cmdCount-1].serverTime ) {
			continue;
		}
		// extremely lagged or cmd from before a map_restart
		//if ( cmds[i].serverTime > svs.time + 3000 ) {
		//	continue;
		//}
		// don't execute if this is an old cmd which is already executed
		// these old cmds are included when cl_packetdup > 0
		if ( cmds[i].serverTime <= cl->lastUsercmd_.serverTime ) {
			continue;
		}
		SV_ClientThink (cl, &cmds[ i ]);
	}
}


/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/

/*
===================
SV_ExecuteClientMessage

Parse a client packet
===================
*/
void SV_ExecuteClientMessage( SV_Client *cl, msg_t *msg ) 
{
	int			c;
	int			serverId;

	MSG_Bitstream(msg);

	serverId = MSG_ReadLong( msg );
	cl->messageAcknowledge_ = MSG_ReadLong( msg );

	if (cl->messageAcknowledge_ < 0) 
	{
		// usually only hackers create messages like this
		// it is more annoying for them to let them hanging
#ifndef NDEBUG
		SV_DropClient( cl, "DEBUG: illegible client message" );
#endif
		return;
	}

	cl->reliableAcknowledge_ = MSG_ReadLong( msg );

	// NOTE: when the client message is fux0red the acknowledgement numbers
	// can be out of range, this could cause the server to send thousands of server
	// commands which the server thinks are not yet acknowledged in SV_UpdateServerCommandsToClient
	if (cl->reliableAcknowledge_ < cl->reliableSequence_ - MAX_RELIABLE_COMMANDS)
	{
		// usually only hackers create messages like this
		// it is more annoying for them to let them hanging
#ifndef NDEBUG
		SV_DropClient( cl, "DEBUG: illegible client message" );
#endif
		cl->reliableAcknowledge_ = cl->reliableSequence_;
		return;
	}
	// if this is a usercmd from a previous gamestate,
	// ignore it or retransmit the current gamestate
	// 
	// if the client was downloading, let it stay at whatever serverId and
	// gamestate it was at.  This allows it to keep downloading even when
	// the gamestate changes.  After the download is finished, we'll
	// notice and send it a New game state
	//
	// don't drop as long as previous command was a nextdl, after a dl is done, downloadName is set back to ""
	// but we still need to read the next message to move to next download or send gamestate
	// I don't like this hack though, it must have been working fine at some point, suspecting the fix is somewhere else
	if ( serverId != theSV.serverId_ && !*cl->downloadName_ && !strstr(cl->lastClientCommandString_, "nextdl") ) 
	{
		if ( serverId >= theSV.restartedServerId_ && serverId < theSV.serverId_ ) 
		{	// TTimo - use a comparison here to catch multiple map_restart
			// they just haven't caught the map_restart yet
			Com_DPrintf("%s : ignoring pre map_restart / outdated client message\n", cl->name_);
			return;
		}
		// if we can tell that the client has dropped the last
		// gamestate we sent them, resend it
		if ( cl->messageAcknowledge_ > cl->gamestateMessageNum_ ) 
		{
			Com_DPrintf( "%s : dropped gamestate, resending\n", cl->name_ );
			SV_SendClientGameState( cl );
		}
		return;
	}

	// read optional clientCommand strings
	do 
	{
		c = MSG_ReadByte( msg );
		if ( c == clc_EOF ) 
			break;

		if ( c != clc_clientCommand ) 
			break;

		if ( !SV_ClientCommand( cl, msg ) ) 
			return;	// we couldn't execute it because of the flood protection

		if (cl->state_ == SV_Client::CS_ZOMBIE) 
			return;	// disconnect command

	} while ( 1 );

	// read the usercmd_t
	if ( c == clc_move ) 
		SV_UserMove( cl, msg, true );
	else if ( c == clc_moveNoDelta ) 
		SV_UserMove( cl, msg, false );
	else if ( c != clc_EOF ) 
		Com_Printf( "WARNING: bad command byte for client %i\n", cl->clientNum_ );

//	if ( msg->readcount != msg->cursize ) {
//		Com_Printf( "WARNING: Junk at end of packet for client %i\n", cl - svs.clients );
//	}
}
