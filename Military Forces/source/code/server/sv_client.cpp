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
#include "sv_entity.h"
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
void SV_GetChallenge(netadr_t from)
{
	int		i;
	int		oldest;
	int		oldestTime;
	int		oldestClientTime;
	int		clientChallenge;
	ServerStatic::Challenge	*challenge;
	bool wasfound = false;
	char *gameName;
	bool gameMismatch;

	// ignore if we are in single player
	if ( CvarSystem::variableValue( "g_gametype" ) == GT_SINGLE_PLAYER || CvarSystem::variableValue("ui_singlePlayerActive")) {
		return;
	}

	// Prevent using getchallenge as an amplifier
	if ( SVC_RateLimitAddress( from, 10, 1000 ) ) {
		Common::dPrintf( "SV_GetChallenge: rate limit from %s exceeded, dropping request\n",
			Network::adrToString( from ) );
		return;
	}

	// Allow getchallenge to be DoSed relatively easily, but prevent
	// excess outbound bandwidth usage when being flooded inbound
	if ( SVC_RateLimit( &outboundLeakyBucket, 10, 100 ) ) {
		Common::dPrintf( "SV_GetChallenge: rate limit exceeded, dropping request\n" );
		return;
	}

	gameName = CommandSystem::argv(2);

	gameMismatch = !*gameName || strcmp(gameName, com_gamename->string) != 0;

	// reject client if the gamename string sent by the client doesn't match ours
	if (gameMismatch)
	{
		Network::outOfBandPrint(NS_SERVER, from, "print\nGame mismatch: This is a %s server\n",
			com_gamename->string);
		return;
	}

	oldest = 0;
	oldestClientTime = oldestTime = 0x7fffffff;

	// see if we already have a challenge for this ip
	challenge = &theSVS.challenges_[0];
	clientChallenge = atoi(CommandSystem::argv(1));

	for(i = 0 ; i < ServerStatic::k_MAX_CHALLENGES ; i++, challenge++)
	{
		if(!challenge->connected_ && Network::compareAdr(from, challenge->adr_))
		{
			wasfound = true;
			
			if(challenge->time_ < oldestClientTime)
				oldestClientTime = challenge->time_;
		}
		
		if(wasfound && i >= ServerStatic::k_MAX_CHALLENGES_MULTI)
		{
			i = ServerStatic::k_MAX_CHALLENGES;
			break;
		}
		
		if(challenge->time_ < oldestTime)
		{
			oldestTime = challenge->time_;
			oldest = i;
		}
	}

	if (i == ServerStatic::k_MAX_CHALLENGES)
	{
		// this is the first time this client has asked for a challenge
		challenge = &theSVS.challenges_[oldest];
		challenge->clientChallenge_ = clientChallenge;
		challenge->adr_ = from;
		challenge->firstTime_ = theSVS.time_;
		challenge->connected_ = false;
	}

	// always generate a new challenge number, so the client cannot circumvent sv_maxping
	challenge->challenge_ = ( (static_cast<unsigned int>(rand()) << 16) ^ static_cast<unsigned int>(rand()) ) ^ theSVS.time_;
	challenge->wasrefused_ = false;
	challenge->time_ = theSVS.time_;

	challenge->pingTime_ = theSVS.time_;
	Network::outOfBandPrint(NS_SERVER, challenge->adr_, "challengeResponse %d %d %d",
			   challenge->challenge_, clientChallenge, com_protocol->integer);
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
//	if ( !Network::compareBaseAdr( from, theSVS.authorizeAddress_ ) ) 
//	{
//		Common::printf( "SV_AuthorizeIpPacket: not from authorize server\n" );
//		return;
//	}
//
//	challenge = atoi( CommandSystem::argv( 1 ) );
//
//	for (i = 0 ; i < MAX_CHALLENGES ; i++) {
//		if ( svs.challenges[i].challenge == challenge ) {
//			break;
//		}
//	}
//	if ( i == MAX_CHALLENGES ) {
//		Common::printf( "SV_AuthorizeIpPacket: challenge not found\n" );
//		return;
//	}
//
//	// send a packet back to the original client
//	svs.challenges[i].pingTime = svs.time;
//	s = CommandSystem::argv( 2 );
//	r = CommandSystem::argv( 3 );			// reason
//
//	//if ( !Q_stricmp( s, "demo" ) ) {
//	//	if ( CvarSystem::variableValue( "fs_restrict" ) ) {
//	//		// a demo client connecting to a demo server
//	//		Network::outOfBandPrint( NS_SERVER, svs.challenges[i].adr, 
//	//			"challengeResponse %i", svs.challenges[i].challenge );
//	//		return;
//	//	}
//	//	// they are a demo client trying to connect to a real server
//	//	Network::outOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nServer is not a demo server\n" );
//	//	// clear the challenge record so it won't timeout and let them through
//	//	Com_Memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
//	//	return;
//	//}
//	if ( !Q_stricmp( s, "accept" ) ) {
//		Network::outOfBandPrint( NS_SERVER, svs.challenges[i].adr, 
//			"challengeResponse %i", svs.challenges[i].challenge );
//		return;
//	}
//	if ( !Q_stricmp( s, "unknown" ) ) {
//		if (!r) {
//			Network::outOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nAwaiting CD key authorization\n" );
//		} else {
//			sprintf(ret, "print\n%s\n", r);
//			Network::outOfBandPrint( NS_SERVER, svs.challenges[i].adr, ret );
//		}
//		// clear the challenge record so it won't timeout and let them through
//		Com_Memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
//		return;
//	}
//
//	// authorization failed
//	if (!r) {
//		Network::outOfBandPrint( NS_SERVER, svs.challenges[i].adr, "print\nSomeone is using this CD Key\n" );
//	} else {
//		sprintf(ret, "print\n%s\n", r);
//		Network::outOfBandPrint( NS_SERVER, svs.challenges[i].adr, ret );
//	}
//
//	// clear the challenge record so it won't timeout and let them through
//	Com_Memset( &svs.challenges[i], 0, sizeof( svs.challenges[i] ) );
//}

/*
==================
SV_IsBanned

Check whether a certain address is banned
==================
*/

static bool SV_IsBanned(netadr_t *from, bool isexception)
{
	int index;
	serverBan_t *curban;
	
	if(!isexception)
	{
		// If this is a query for a ban, first check whether the client is excepted
		if(SV_IsBanned(from, true))
			return false;
	}
	
	for(index = 0; index < serverBansCount; index++)
	{
		curban = &serverBans[index];
		
		if(curban->isexception == isexception)
		{
			if(Network::compareBaseAdrMask(curban->ip, *from, curban->subnet))
				return true;
		}
	}
	
	return false;
}

/*
==================
SV_DirectConnect

A "connect" OOB command has been received
==================
*/

void SV_DirectConnect( netadr_t from ) 
{
	Common::dPrintf ("SVC_DirectConnect ()\n");

	// Check whether this client is banned.
	if(SV_IsBanned(&from, false))
	{
		Network::outOfBandPrint(NS_SERVER, from, "print\nYou are banned from this server.\n");
		return;
	}

	char		userinfo[MAX_INFO_STRING];
	Q_strncpyz( userinfo, CommandSystem::argv(1), sizeof(userinfo) );

	int version = atoi( Info::valueForKey( userinfo, "protocol" ) );
	if(version != com_protocol->integer)
	{
		Network::outOfBandPrint(NS_SERVER, from, "print\nServer uses protocol version %i "
				   "(yours is %i).\n", com_protocol->integer, version);
		Common::dPrintf("    rejected connect from version %i\n", version);
		return;
	}

	int challenge = atoi( Info::valueForKey( userinfo, "challenge" ) );
	int qport = atoi( Info::valueForKey( userinfo, "qport" ) );

	// quick reject
	SV_Client* cl;
	for( int i = 1; i <= sv_maxclients->integer ; i++ ) 
	{
		cl = theSVS.svClients_.at(i);

		if( !cl || cl->state_ == SV_Client::CS_FREE ) 
			continue;

		if( Network::compareBaseAdr( from, cl->netchan_.remoteAddress )
			&& ( cl->netchan_.qport == qport 
			|| from.port == cl->netchan_.remoteAddress.port ) ) 
		{
			if (( theSVS.time_ - cl->lastConnectTime_) 
				< (sv_reconnectlimit->integer * 1000)) 
			{
				Common::dPrintf ("%s:reconnect rejected : too soon\n", Network::adrToString (from));
				return;
			}
			break;
		}
	}
	
	// don't let "ip" overflow userinfo string
	const char *ip;
	if ( Network::isLocalAddress (from) )
		ip = "localhost";
	else
		ip = Network::adrToString( from );
	if( ( strlen( ip ) + strlen( userinfo ) + 4 ) >= MAX_INFO_STRING ) {
		Network::outOfBandPrint( NS_SERVER, from,
			"print\nUserinfo string length exceeded.  "
			"Try removing setu cvars from your config.\n" );
		return;
	}
	Info::setValueForKey( userinfo, "ip", ip );

	// see if the challenge is valid (LAN clients don't need to challenge)
	if( !Network::isLocalAddress (from) ) 
	{
		int		ping;
		int		i;

		for(i = 0; i < ServerStatic::k_MAX_CHALLENGES ; i++ ) 
		{
			if( Network::compareAdr(from, theSVS.challenges_[i].adr_) ) 
			{
				if ( challenge == theSVS.challenges_[i].challenge_ ) 
					break;		// good
			}
		}
		if( i == ServerStatic::k_MAX_CHALLENGES ) 
		{
			Network::outOfBandPrint( NS_SERVER, from, "print\nNo or bad challenge for your address.\n" );
			return;
		}

		ServerStatic::Challenge *challengeptr = &theSVS.challenges_[i];

		if(challengeptr->wasrefused_)
		{
			// Return silently, so that error messages written by the server keep being displayed.
			return;
		}

		ping = theSVS.time_ - challengeptr->pingTime_;

		// never reject a LAN client based on ping
		if ( !Sys_IsLANAddress( from ) ) 
		{
			if ( sv_minPing->value && ping < sv_minPing->value ) 
			{
				Network::outOfBandPrint( NS_SERVER, from, "print\nServer is for high pings only\n" );
				Common::dPrintf ("Client %i rejected on a too low ping\n", i);
				challengeptr->wasrefused_ = true;
				return;
			}
			if ( sv_maxPing->value && ping > sv_maxPing->value ) 
			{
				Network::outOfBandPrint( NS_SERVER, from, "print\nServer is for low pings only\n" );
				Common::dPrintf ("Client %i rejected on a too high ping\n", i);
				challengeptr->wasrefused_ = true;
				return;
			}
		}

		Common::printf("Client %i connecting with %i challenge ping\n", i, ping);
		challengeptr->connected_ = true;
	}

	//MAC_STATIC SV_Client temp;
	SV_Client	*newcl = nullptr;//&temp;
	//Com_Memset( newcl, 0, sizeof(SV_Client) );

	// if there is already a slot for this ip, reuse it
	for( int i = 1; i <= sv_maxclients->integer; i++ )
	{
		cl = theSVS.svClients_.at(i); 

		if( !cl || cl->state_ == SV_Client::CS_FREE ) 
			continue;

		if( Network::compareBaseAdr( from, cl->netchan_.remoteAddress )
			&& ( cl->netchan_.qport == qport 
			|| from.port == cl->netchan_.remoteAddress.port ) ) 
		{
			Common::printf ("%s:reconnect\n", Network::adrToString (from));
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
	char* password;
	password = Info::valueForKey( userinfo, "password" );
	if ( *password && !strcmp( password, sv_privatePassword->string ) ) 
	{
		startIndex = 1;
	} 
	else
	{
		// skip past the reserved slots
		startIndex = sv_privateClients->integer + 1;
	}

	newcl = nullptr;
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
		if( Network::isLocalAddress( from ) ) 
		{
			count = 0;
			int i;
			for( i = startIndex; i <= sv_maxclients->integer ; i++ ) 
			{
				cl = theSVS.svClients_.at(i);
				if( cl && cl->netchan_.remoteAddress.type == NA_BOT ) 
					count++;
			}
			// if they're all bots
			if (count >= sv_maxclients->integer - startIndex) 
			{
				(theSVS.svClients_.at(sv_maxclients->integer))->drop( "only bots on server" );
				newcl = theSVS.svClients_.at(sv_maxclients->integer);
				newcl->clientNum_ = i;
			}
			else 
			{
				Common::error( ERR_FATAL, "server is full on local connect\n" );
				return;
			}
		}
		else 
		{
			Network::outOfBandPrint( NS_SERVER, from, "print\nServer is full.\n" );
			Common::dPrintf ("Rejected a connection.\n");
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
	newcl->netchan_.setup( NS_SERVER, from, qport, challenge, false );
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

		Network::outOfBandPrint( NS_SERVER, from, "print\n%s\n", denied );
		Common::dPrintf ("Game rejected a connection: %s.\n", denied);
		return;
	}

	newcl->userinfoChanged();

	// send the connect packet to the client
	Network::outOfBandPrint(NS_SERVER, from, "connectResponse %d", challenge);

	Common::dPrintf( "Going from CS_FREE to CS_CONNECTED for %s\n", newcl->name_ );

	newcl->state_ = SV_Client::CS_CONNECTED;
	newcl->lastSnapshotTime_ = 0;
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
SV_FreeClient

Destructor for data allocated in a client structure
=====================
*/
void SV_Client::free()
{
#ifdef USE_VOIP
	int index;
	
	for(index = this->queuedVoipIndex_; index < this->queuedVoipPackets_; index++)
	{
		index %= ARRAY_LEN(this->voipPacket_);
		
		Z_Free(this->voipPacket_[index]);
	}
	
	this->queuedVoipPackets_ = 0;
#endif

	this->netchanFreeQueue();
	this->closeDownload();
}

/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing -- SV_FinalMessage() will handle that
=====================
*/
void SV_Client::drop( const char *reason ) 
{
	int		i;
	ServerStatic::Challenge	*challenge;
	const bool isBot = this->netchan_.remoteAddress.type == NA_BOT;

	if ( this->state_ == SV_Client::CS_ZOMBIE ) {
		return;		// already dropped
	}

	if ( !isBot ) {
		// see if we already have a challenge for this ip
		challenge = &theSVS.challenges_[0];

		for (i = 0 ; i < ServerStatic::k_MAX_CHALLENGES ; i++, challenge++)
		{
			if(Network::compareAdr(this->netchan_.remoteAddress, challenge->adr_))
			{
				Com_Memset(challenge, 0, sizeof(*challenge));
				break;
			}
		}
	}

	// Free all allocated data on the client structure
	this->free();

	// Reset the reliable sequence to the currently acknowledged command
	// This prevents ()->addServerCommand() from making another recursive call to ()->drop()
	// if the client lacks sufficient space for another reliable command
	// it also guarantees that the client receives both the print and disconnect commands
	this->reliableSequence_ = this->reliableAcknowledge_;
	// Setting the gamestate message number to -1 ensures that ()->addServerCommand()
	// will not call ()->drop() again, even though it is unlikely the client
	// will receive many server commands during the this
	this->gamestateMessageNum_ = -1;

	// tell everyone why they got dropped
	SV_SendServerCommand( nullptr, "print \"%s" S_COLOR_WHITE " %s\n\"", this->name_, reason );

	// call the prog function for removing a client
	// this will remove the body, among other things
	//VM_Call( gvm, GAME_CLIENT_DISCONNECT, this - svs.clients );
	theSG.clientDisconnect( this->clientNum_ );

	// add the disconnect command
	SV_SendServerCommand( this, "disconnect \"%s\"", reason);

	if ( isBot ) {
		// bots shouldn't go zombie, as there's no real net connection.
		this->state_ = SV_Client::CS_FREE;
	} else {
		Common::dPrintf( "Going to CS_ZOMBIE for %s\n", this->name_ );
		this->state_ = SV_Client::CS_ZOMBIE;		// become free in a few seconds
	}

	// nuke user info
	SV_SetUserinfo( this->clientNum_, "" );

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
void SV_Client::sendClientGameState() 
{
	int				start;
	entityState_t	*base, nullstate;
	Message			msg;
	byte			msgBuffer[MAX_MSGLEN];

 	Common::dPrintf ("()->sendClientGameState() for %s\n", this->name_);
	Common::dPrintf( "Going from CS_CONNECTED to CS_PRIMED for %s\n", this->name_ );
	this->state_ = SV_Client::CS_PRIMED;
	this->pureAuthentic_ = 0;
	this->gotCP_ = false;

	// when we receive the first packet from the this, we will
	// notice that it is from a different serverid and that the
	// gamestate message was not just sent, forcing a retransmit
	this->gamestateMessageNum_ = this->netchan_.outgoingSequence;
	this->entityReset_ = true;
	this->entityResetDone_ = false;

	msg.init( msgBuffer, sizeof( msgBuffer ) );

	// NOTE, MRE: all server->client messages now acknowledge
	// let the this know which reliable clientCommands we have received
	msg.writeLong( this->lastClientCommand_ );

	// send any server commands waiting to be sent first.
	// we have to do this cause we send the this->reliableSequence
	// with a gamestate and it sets the clc.serverCommandSequence at
	// the this side
	this->updateServerCommandsToClient( &msg );

	// send the gamestate
	msg.writeByte( svc_gamestate );
	msg.writeLong( this->reliableSequence_ );

	// write the configstrings
	for ( start = 0 ; start < MAX_CONFIGSTRINGS ; start++ ) 
	{
		if (theSV.configstrings_[start][0]) 
		{
			msg.writeByte( svc_configstring );
			msg.writeShort( start );
			msg.writeBigString( theSV.configstrings_[start] );
		}
	}

	// write the baselines
	Com_Memset( &nullstate, 0, sizeof( nullstate ) );
	for ( start = 0 ; start < MAX_GENTITIES; start++ ) 
	{
		base = &theSV.serverEntities_[start].baseline_;
		if ( !base->number ) 
			continue;

		msg.writeByte( svc_baseline );
		msg.writeDeltaEntity( &nullstate, base, true );
	}

	msg.writeByte( svc_EOF );

	msg.writeLong( this->clientNum_ );

	// write the checksum feed
	msg.writeLong( theSV.checksumFeed_);

	// deliver this to the this
	this->sendMessageToClient( &msg );
}


/*
==================
SV_ClientEnterWorld
==================
*/
void SV_Client::enterWorld( usercmd_t *cmd )
{
	int			clientNum;
	EntityBase *ent;

	Common::dPrintf( "Going from CS_PRIMED to CS_ACTIVE for %s\n", this->name_ );
	this->state_ = SV_Client::CS_ACTIVE;

	// set up the entity for the this
	clientNum = this->clientNum_;
	ent = theSV.getEntityByNum( clientNum );
	ent->s.number = clientNum;
	//this->gentity_ = ent;

	this->deltaMessage_ = -1;
	this->lastSnapshotTime_ = 0;	// generate a snapshot immediately
	if(cmd)
		memcpy(&this->lastUsercmd_, cmd, sizeof(this->lastUsercmd_));
	else
		memset(&this->lastUsercmd_, '\0', sizeof(this->lastUsercmd_));

	// resend all configstrings using the cs commands since these are
	// no longer sent when the this is CS_PRIMED
	this->updateConfigstrings();

	// call the game begin function
	//VM_Call( gvm, GAME_CLIENT_BEGIN, this - svs.clients );
	theSG.clientBegin( this->clientNum_ );
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
void SV_Client::closeDownload() 
{
	int i;

	// EOF
	if (this->download_) 
		FileSystem::fCloseFile( this->download_ );

	this->download_ = 0;
	*this->downloadName_ = 0;

	// Free the temporary buffer space
	for (i = 0; i < MAX_DOWNLOAD_WINDOW; i++) 
	{
		if (this->downloadBlocks_[i]) 
		{
			Z_Free( this->downloadBlocks_[i] );
			this->downloadBlocks_[i] = nullptr;
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
		Common::dPrintf( "clientDownload: %d : file \"%s\" aborted\n", cl->clientNum_, cl->downloadName_ );

	cl->closeDownload();
}

/*
==================
SV_DoneDownload_f

Downloads are finished
==================
*/
void SV_DoneDownload_f( SV_Client *cl ) 
{
	if ( cl->state_ == SV_Client::CS_ACTIVE )
		return;

	Common::dPrintf( "clientDownload: %s Done\n", cl->name_);
	// resend the game state to update any clients that entered during the download
	cl->sendClientGameState();
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
	int block = atoi( CommandSystem::argv(1) );

	if (block == cl->downloadClientBlock_) 
	{
		Common::dPrintf( "clientDownload: %d : client acknowledge of block %d\n", cl->clientNum_, block );

		// Find out if we are done.  A zero-length block indicates EOF
		if (cl->downloadBlockSize_[cl->downloadClientBlock_ % MAX_DOWNLOAD_WINDOW] == 0)
		{
			Common::printf( "clientDownload: %d : file \"%s\" completed\n", cl->clientNum_, cl->downloadName_ );
			cl->closeDownload();
			return;
		}

		cl->downloadSendTime_ = theSVS.time_;
		cl->downloadClientBlock_++;
		return;
	}
	// We aren't getting an acknowledge for the correct block, drop the client
	// FIXME: this is bad... the client will never parse the disconnect message
	//			because the cgame isn't loaded yet
	cl->drop( "broken download" );
}

/*
==================
SV_BeginDownload_f
==================
*/
void SV_BeginDownload_f( SV_Client *cl ) 
{

	// Kill any existing download
	cl->closeDownload();

	// cl->downloadName is non-zero now, SV_WriteDownloadToClient will see this and open
	// the file itself
	Q_strncpyz( cl->downloadName_, CommandSystem::argv(1), sizeof(cl->downloadName_) );
}

/*
==================
SV_WriteDownloadToClient

Check to see if the client wants a file, open it if needed and start pumping the client
Fill up msg with data, return number of download blocks added
==================
*/
int SV_Client::writeDownloadToClient( Message *msg )
{
	int curindex;
	int unreferenced = 1;
	char errorMessage[1024];
	char pakbuf[MAX_QPATH], *pakptr;
	int numRefPaks;

	if (!*this->downloadName_)
		return 0;	// Nothing being downloaded

	if(!this->download_)
	{
		bool idPack = false;
		#ifndef STANDALONE
		bool missionPack = false;
		#endif
	
 		// Chop off filename extension.
		Com_sprintf(pakbuf, sizeof(pakbuf), "%s", this->downloadName_);
		pakptr = strrchr(pakbuf, '.');
		
		if(pakptr)
		{
			*pakptr = '\0';

			// Check for pk3 filename extension
			if(!Q_stricmp(pakptr + 1, "pk3"))
			{
				const char *referencedPaks = FileSystem::referencedPakNames();

				// Check whether the file appears in the list of referenced
				// paks to prevent downloading of arbitrary files.
				CommandSystem::tokenizeStringIgnoreQuotes(referencedPaks);
				numRefPaks = CommandSystem::argc();

				for(curindex = 0; curindex < numRefPaks; curindex++)
				{
					if(!FileSystem::filenameCompare(CommandSystem::argv(curindex), pakbuf))
					{
						unreferenced = 0;

						// now that we know the file is referenced,
						// check whether it's legal to download it.
#ifndef STANDALONE
						missionPack = FileSystem::idPak(pakbuf, BASETA, NUM_TA_PAKS);
						idPack = missionPack;
#endif
						idPack = idPack || FileSystem::idPak(pakbuf, (char *)BASEGAME, NUM_ID_PAKS);

						break;
					}
				}
			}
		}

		this->download_ = 0;

		// We open the file here
		if ( !(sv_allowDownload->integer & DLF_ENABLE) ||
			(sv_allowDownload->integer & DLF_NO_UDP) ||
			idPack || unreferenced ||
			( this->downloadSize_ = FileSystem::baseDir_FOpenFileRead( this->downloadName_, &this->download_ ) ) < 0 ) {
			// cannot auto-download file
			if(unreferenced)
			{
				Common::printf("clientDownload: %d : \"%s\" is not referenced and cannot be downloaded.\n", this->clientNum_, this->downloadName_);
				Com_sprintf(errorMessage, sizeof(errorMessage), "File \"%s\" is not referenced and cannot be downloaded.", this->downloadName_);
			}
			else if (idPack) {
				Common::printf("clientDownload: %d : \"%s\" cannot download id pk3 files\n", this->clientNum_, this->downloadName_);
#ifndef STANDALONE
				if(missionPack)
				{
					Com_sprintf(errorMessage, sizeof(errorMessage), "Cannot autodownload Team Arena file \"%s\"\n"
									"The Team Arena mission pack can be found in your local game store.", this->downloadName_);
				}
				else
#endif
				{
					Com_sprintf(errorMessage, sizeof(errorMessage), "Cannot autodownload id pk3 file \"%s\"", this->downloadName_);
				}
			}
			else if ( !(sv_allowDownload->integer & DLF_ENABLE) ||
				(sv_allowDownload->integer & DLF_NO_UDP) ) {

				Common::printf("clientDownload: %d : \"%s\" download disabled\n", this->clientNum_, this->downloadName_);
				if (sv_pure->integer) {
					Com_sprintf(errorMessage, sizeof(errorMessage), "Could not download \"%s\" because autodownloading is disabled on the server.\n\n"
										"You will need to get this file elsewhere before you "
										"can connect to this pure server.\n", this->downloadName_);
				} else {
					Com_sprintf(errorMessage, sizeof(errorMessage), "Could not download \"%s\" because autodownloading is disabled on the server.\n\n"
                    "The server you are connecting to is not a pure server, "
                    "set autodownload to No in your settings and you might be "
                    "able to join the game anyway.\n", this->downloadName_);
				}
			} else {
        // NOTE TTimo this is NOT supposed to happen unless bug in our filesystem scheme?
        //   if the pk3 is referenced, it must have been found somewhere in the filesystem
				Common::printf("clientDownload: %d : \"%s\" file not found on server\n", this->clientNum_, this->downloadName_);
				Com_sprintf(errorMessage, sizeof(errorMessage), "File \"%s\" not found on server for autodownloading.\n", this->downloadName_);
			}
			msg->writeByte( svc_download );
			msg->writeShort( 0 ); // client is expecting block zero
			msg->writeLong( -1 ); // illegal file size
			msg->writeString( errorMessage );

			*this->downloadName_ = 0;
			
			if(this->download_)
				FileSystem::fCloseFile(this->download_);
			
			return 1;
		}
 
		Common::printf( "clientDownload: %d : beginning \"%s\"\n", this->clientNum_, this->downloadName_ );
		
		// Init
		this->downloadCurrentBlock_ = this->downloadClientBlock_ = this->downloadXmitBlock_ = 0;
		this->downloadCount_ = 0;
		this->downloadEOF_ = false;
	}

	// Perform any reads that we need to
	while (this->downloadCurrentBlock_ - this->downloadClientBlock_ < MAX_DOWNLOAD_WINDOW &&
		this->downloadSize_ != this->downloadCount_) {

		curindex = (this->downloadCurrentBlock_ % MAX_DOWNLOAD_WINDOW);

		if (!this->downloadBlocks_[curindex])
			this->downloadBlocks_[curindex] = static_cast<unsigned char *>(Z_Malloc(MAX_DOWNLOAD_BLKSIZE));

		this->downloadBlockSize_[curindex] = FileSystem::read( this->downloadBlocks_[curindex], MAX_DOWNLOAD_BLKSIZE, this->download_ );

		if (this->downloadBlockSize_[curindex] < 0) {
			// EOF right now
			this->downloadCount_ = this->downloadSize_;
			break;
		}

		this->downloadCount_ += this->downloadBlockSize_[curindex];

		// Load in next block
		this->downloadCurrentBlock_++;
	}

	// Check to see if we have eof condition and add the EOF block
	if (this->downloadCount_ == this->downloadSize_ &&
		!this->downloadEOF_ &&
		this->downloadCurrentBlock_ - this->downloadClientBlock_ < MAX_DOWNLOAD_WINDOW) {

		this->downloadBlockSize_[this->downloadCurrentBlock_ % MAX_DOWNLOAD_WINDOW] = 0;
		this->downloadCurrentBlock_++;

		this->downloadEOF_ = true;  // We have added the EOF block
	}

	if (this->downloadClientBlock_ == this->downloadCurrentBlock_)
		return 0; // Nothing to transmit

	// Write out the next section of the file, if we have already reached our window,
	// automatically start retransmitting
	if (this->downloadXmitBlock_ == this->downloadCurrentBlock_)
	{
		// We have transmitted the complete window, should we start resending?
		if (theSVS.time_ - this->downloadSendTime_ > 1000)
			this->downloadXmitBlock_ = this->downloadClientBlock_;
		else
			return 0;
	}

	// Send current block
	curindex = (this->downloadXmitBlock_ % MAX_DOWNLOAD_WINDOW);

	msg->writeByte( svc_download );
	msg->writeShort( this->downloadXmitBlock_ );

	// block zero is special, contains file size
	if ( this->downloadXmitBlock_ == 0 )
		msg->writeLong( this->downloadSize_ );

	msg->writeShort( this->downloadBlockSize_[curindex] );

	// Write the block
	if(this->downloadBlockSize_[curindex])
		msg->writeData( this->downloadBlocks_[curindex], this->downloadBlockSize_[curindex]);

	Common::dPrintf( "clientDownload: %d : writing block %d\n", this->clientNum_, this->downloadXmitBlock_ );

	// Move on to the next block
	// It will get sent with next snap shot.  The rate will keep us in line.
	this->downloadXmitBlock_++;
	this->downloadSendTime_ = theSVS.time_;

	return 1;
}

/*
==================
SV_SendQueuedMessages

Send one round of fragments, or queued messages to all clients that have data pending.
Return the shortest time interval for sending next packet to client
==================
*/

int SV_SendQueuedMessages()
{
	int i, retval = -1, nextFragT;
	SV_Client *cl;
	
	for(i=1; i <= sv_maxclients->integer; i++)
	{
		cl = theSVS.svClients_.at(i);
		
		if(cl && cl->state_)
		{
			nextFragT = cl->rateMsec();

			if(!nextFragT)
				nextFragT = cl->netchanTransmitNextFragment();

			if(nextFragT >= 0 && (retval == -1 || retval > nextFragT))
				retval = nextFragT;
		}
	}

	return retval;
}


/*
==================
SV_SendDownloadMessages

Send one round of download messages to all clients
==================
*/

int SV_SendDownloadMessages()
{
	int i, numDLs = 0, retval;
	SV_Client *cl;
	Message msg;
	byte msgBuffer[MAX_MSGLEN];
	
	for(i=1; i <= sv_maxclients->integer; i++)
	{
		cl = theSVS.svClients_.at(i);
		
		if(cl && cl->state_ && *cl->downloadName_)
		{
			msg.init( msgBuffer, sizeof(msgBuffer));
			msg.writeLong( cl->lastClientCommand_);
			
			retval = cl->writeDownloadToClient( &msg );
				
			if(retval)
			{
				msg.writeByte( svc_EOF);
				cl->netchanTransmit( &msg );
				numDLs += retval;
			}
		}
	}

	return numDLs;
}


/*
=================
SV_Disconnect_f

The client is going to disconnect, so remove the connection immediately  FIXME: move to game?
=================
*/
static void SV_Disconnect_f( SV_Client *cl ) 
{
	cl->drop( "disconnected" );
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
		//bGood = (FileSystem::fileIsInPAK("vm/cgame.qvm", &nChkSum1) == 1);
		//if (bGood)
		//	bGood = (FileSystem::fileIsInPAK("vm/ui.qvm", &nChkSum2) == 1);

		nClientPaks = CommandSystem::argc();

		// start at arg 2 ( skip serverId cl_paks )
		nCurArg = 1;

		pArg = CommandSystem::argv(nCurArg++);
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
				Common::dPrintf("ignoring outdated cp command from client %s\n", cl->name_);
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
			pArg = CommandSystem::argv(nCurArg++);
			if (!pArg || *pArg == '@' || atoi(pArg) != nChkSum1 ) 
			{
				bGood = false;
				break;
			}
			// verify the second to be the ui checksum
			pArg = CommandSystem::argv(nCurArg++);
			if (!pArg || *pArg == '@' || atoi(pArg) != nChkSum2 ) 
			{
				bGood = false;
				break;
			}
			// should be sitting at the delimeter now
			pArg = CommandSystem::argv(nCurArg++);
			if (*pArg != '@')
			{
				bGood = false;
				break;
			}
			// store checksums since tokenization is not re-entrant
			for (i = 0; nCurArg < nClientPaks; i++) 
			{
				nClientChkSum[i] = atoi(CommandSystem::argv(nCurArg++));
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
			pPaks = FileSystem::loadedPakPureChecksums();
			CommandSystem::tokenizeString( pPaks );
			nServerPaks = CommandSystem::argc();
			if (nServerPaks > 1024)
				nServerPaks = 1024;

			for (i = 0; i < nServerPaks; i++) 
				nServerChkSum[i] = atoi(CommandSystem::argv(i));

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
			cl->lastSnapshotTime_ = 0;
			cl->state_ = SV_Client::CS_ACTIVE;
			cl->sendClientSnapshot();
			cl->drop( "Unpure Client. "
				"You may need to enable in-game downloads "
				"to connect to this server (set "
				"cl_allowDownload 1)" );
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
void SV_Client::userinfoChanged() {
	char	*val;
	char	*ip;
	int		i;
	int	len;

	// name for C code
	Q_strncpyz( this->name_, Info::valueForKey (this->userinfo_, "name"), sizeof(this->name_) );

	// rate command

	// if the client is on the same subnet as the server and we aren't running an
	// internet public server, assume they don't need a rate choke
	if ( Sys_IsLANAddress( this->netchan_.remoteAddress ) && com_dedicated->integer != 2 && sv_lanForceRate->integer == 1) {
		this->rate_ = 99999;	// lans should not rate limit
	} else {
		val = Info::valueForKey (this->userinfo_, "rate");
		if (strlen(val)) {
			i = atoi(val);
			this->rate_ = i;
			if (this->rate_ < 1000) {
				this->rate_ = 1000;
			} else if (this->rate_ > 1000000) {
				this->rate_ = 1000000;
			}
		} else {
			this->rate_ = 3000;
		}
	}
	val = Info::valueForKey (this->userinfo_, "handicap");
	if (strlen(val)) {
		i = atoi(val);
		if (i<=0 || i>100 || strlen(val) > 4) {
			Info::setValueForKey( this->userinfo_, "handicap", "100" );
		}
	}

	// snaps command
	val = Info::valueForKey (this->userinfo_, "snaps");
	
	if(strlen(val))
	{
		i = atoi(val);
		
		if(i < 1)
			i = 1;
		else if(i > sv_fps->integer)
			i = sv_fps->integer;

		i = 1000 / i;
	}
	else
		i = 50;

	if(i != this->snapshotMsec_)
	{
		// Reset last sent snapshot so we avoid desync between server frame time and snapshot send time
		this->lastSnapshotTime_ = 0;
		this->snapshotMsec_ = i;		
	}
	
#ifdef USE_VOIP
#ifdef LEGACY_PROTOCOL
	if(this->compat)
		this->hasVoip_ = false;
	else
#endif
	{
		val = Info::valueForKey(this->userinfo_, "cl_voipProtocol");
		this->hasVoip_ = !Q_stricmp( val, "opus" );
	}
#endif

	// TTimo
	// maintain the IP information
	// the banning code relies on this being consistently present
	if( Network::isLocalAddress(this->netchan_.remoteAddress) )
		ip = "localhost";
	else
		ip = const_cast<char*>(Network::adrToString( this->netchan_.remoteAddress ));

	val = Info::valueForKey( this->userinfo_, "ip" );
	if( val[0] )
		len = strlen( ip ) - strlen( val ) + strlen( this->userinfo_ );
	else
		len = strlen( ip ) + 4 + strlen( this->userinfo_ );

	if( len >= MAX_INFO_STRING )
		this->drop( "userinfo string length exceeded" );
	else
		Info::setValueForKey( this->userinfo_, "ip", ip );

}


/*
==================
SV_UpdateUserinfo_f
==================
*/
static void SV_UpdateUserinfo_f( SV_Client *cl ) 
{
	Q_strncpyz( cl->userinfo_, CommandSystem::argv(1), sizeof(cl->userinfo_) );

	cl->userinfoChanged();
	// call prog code to allow overrides
	//VM_Call( gvm, GAME_CLIENT_USERINFO_CHANGED, cl - svs.clients );
	theSG.clientUserInfoChanged( cl->clientNum_ );
}

#ifdef USE_VOIP
void SV_Client::updateVoipIgnore( const char *idstr, bool ignore )
{
	if ((*idstr >= '0') && (*idstr <= '9')) {
		const int id = atoi(idstr);
		if ((id >= 0) && (id < MAX_CLIENTS)) {
			this->ignoreVoipFromClient_[id] = ignore;
		}
	}
}

/*
==================
SV_Voip_f
==================
*/
static void SV_Voip_f( SV_Client *cl ) {
	const char *cmd = CommandSystem::argv(1);
	if (strcmp(cmd, "ignore") == 0) {
		cl->updateVoipIgnore( CommandSystem::argv(2), true );
	} else if (strcmp(cmd, "unignore") == 0) {
		cl->updateVoipIgnore( CommandSystem::argv(2), false );
	} else if (strcmp(cmd, "muteall") == 0) {
		cl->muteAllVoip_ = true;
	} else if (strcmp(cmd, "unmuteall") == 0) {
		cl->muteAllVoip_ = false;
	}
}
#endif


struct ucmd_t 
{
	char	*name;
	void	(*func)( SV_Client *cl );
};

static ucmd_t ucmds[] = {
	{"userinfo", SV_UpdateUserinfo_f},
	{"disconnect", SV_Disconnect_f},
	{"cp", SV_VerifyPaks_f},
	{"vdr", SV_ResetPureClient_f},
	{"download", SV_BeginDownload_f},
	{"nextdl", SV_NextDownload_f},
	{"stopdl", SV_StopDownload_f},
	{"donedl", SV_DoneDownload_f},

#ifdef USE_VOIP
	{"voip", SV_Voip_f},
#endif

	{nullptr, nullptr}
};

/*
==================
SV_ExecuteClientCommand

Also called by bot code
==================
*/
void SV_Client::executeClientCommand( const char *s, bool clientOK ) 
{
	ucmd_t	*u;
	bool bProcessed = false;
	
	CommandSystem::tokenizeString( s );

	// see if it is a server level command
	for (u=ucmds ; u->name ; u++) 
	{
		if (!strcmp (CommandSystem::argv(0), u->name) ) 
		{
			u->func( this );
			bProcessed = true;
			break;
		}
	}

	if (clientOK) 
	{
		// pass unknown strings to the game
		if (!u->name && theSV.state_ == Server::SS_GAME && (this->state_ == SV_Client::CS_ACTIVE || this->state_ == SV_Client::CS_PRIMED)) 
		{
			CommandSystem::args_Sanitize();
			//VM_Call( gvm, GAME_CLIENT_COMMAND, this - svs.clients );
			theSG.clientCommand( this->clientNum_ );
		}
	}
	else if (!bProcessed)
		Common::dPrintf( "client text ignored for %s: %s\n", this->name_, CommandSystem::argv(0) );
}

/*
===============
SV_ClientCommand
===============
*/
bool SV_Client::clientCommand( Message *msg ) 
{
	int		seq;
	const char	*s;
	bool clientOk = true;

	seq = msg->readLong();
	s = msg->readString();

	// see if we have already executed it
	if ( this->lastClientCommand_ >= seq ) 
		return true;

	Common::dPrintf( "clientCommand: %s : %i : %s\n", this->name_, seq, s );

	// drop the connection if we have somehow lost commands
	if ( seq > this->lastClientCommand_ + 1 ) 
	{
		Common::printf( "Client %s lost %i clientCommands\n", this->name_, 
			seq - this->lastClientCommand_ + 1 );
		this->drop( "Lost reliable commands" );
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
		this->state_ >= SV_Client::CS_ACTIVE &&
		sv_floodProtect->integer && 
		theSVS.time_ < this->nextReliableTime_ ) 
	{
		// ignore any other text messages from this client but let them keep playing
		// TTimo - moved the ignored verbose to the actual processing in SV_ExecuteClientCommand, only printing if the core doesn't intercept
		clientOk = false;
	} 

	// don't allow another command for one second
	this->nextReliableTime_ = theSVS.time_ + 1000;

	this->executeClientCommand( s, clientOk );

	this->lastClientCommand_ = seq;
	Com_sprintf(this->lastClientCommandString_, sizeof(this->lastClientCommandString_), "%s", s);

	return true;		// continue procesing
}


//==================================================================================


/*
==================
SV_ClientThink

Also called by bot code
==================
*/
void SV_Client::think( usercmd_t *cmd ) 
{
	this->lastUsercmd_ = *cmd;

	if ( this->state_ != SV_Client::CS_ACTIVE ) 
		return;		// may have been kicked during the last usercmd

	//VM_Call( gvm, GAME_CLIENT_THINK, this - svs.clients );
	theSG.clientThink( this->clientNum_ );
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
void SV_Client::userMove( Message *msg, bool delta ) 
{
	int			i, key;
	int			cmdCount;
	usercmd_t	nullcmd;
	usercmd_t	cmds[MAX_PACKET_USERCMDS];
	usercmd_t	*cmd, *oldcmd;

	if ( delta ) 
		this->deltaMessage_ = this->messageAcknowledge_;
	else 
		this->deltaMessage_ = -1;

	cmdCount = msg->readByte();

	if ( cmdCount < 1 )
	{
		Common::printf( "cmdCount < 1\n" );
		return;
	}

	if ( cmdCount > MAX_PACKET_USERCMDS ) 
	{
		Common::printf( "cmdCount > MAX_PACKET_USERCMDS\n" );
		return;
	}

	// use the checksum feed in the key
	key = theSV.checksumFeed_;
	// also use the message acknowledge
	key ^= this->messageAcknowledge_;
	// also use the last acknowledged server command in the key
	key ^= Message::hashKey(this->reliableCommands_[ this->reliableAcknowledge_ & (MAX_RELIABLE_COMMANDS-1) ], 32);

	Com_Memset( &nullcmd, 0, sizeof(nullcmd) );
	oldcmd = &nullcmd;
	for ( i = 0 ; i < cmdCount ; i++ ) 
	{
		cmd = &cmds[i];
		msg->readDeltaUsercmdKey( key, oldcmd, cmd );
		oldcmd = cmd;
	}

	// save time for ping calculation
	this->frames_[ this->messageAcknowledge_ & PACKET_MASK ].messageAcked_ = theSVS.time_;

	// TTimo
	// catch the no-cp-yet situation before SV_ClientEnterWorld
	// if CS_ACTIVE, then it's time to trigger a New gamestate emission
	// if not, then we are getting remaining parasite usermove commands, which we should ignore
	if (sv_pure->integer != 0 && this->pureAuthentic_ == 0 && !this->gotCP_) 
	{
		if (this->state_ == SV_Client::CS_ACTIVE)
		{
			// we didn't get a cp yet, don't assume anything and just send the gamestate all over again
			Common::dPrintf( "%s: didn't get cp command, resending gamestate\n", this->name_, this->state_ );
			this->sendClientGameState();
		}
		return;
	}			
	
	// if this is the first usercmd we have received
	// this gamestate, put the client into the world
	if ( this->state_ == SV_Client::CS_PRIMED ) 
	{
		this->enterWorld( &cmds[0] );
		// the moves can be processed normaly
	}
	
	// a bad cp command was sent, drop the client
	if (sv_pure->integer != 0 && this->pureAuthentic_ == 0) 
	{		
		this->drop( "Cannot validate pure client!" );
		return;
	}

	if ( this->state_ != SV_Client::CS_ACTIVE ) 
	{
		this->deltaMessage_ = -1;
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
		if ( cmds[i].serverTime <= this->lastUsercmd_.serverTime ) {
			continue;
		}
		this->think( &cmds[ i ] );
	}
}


/*
===========================================================================

USER CMD EXECUTION

===========================================================================
*/

#ifdef USE_VOIP
/*
==================
SV_ShouldIgnoreVoipSender

Blocking of voip packets based on source client
==================
*/

bool SV_Client::shouldIgnoreVoipSender()
{
	if (!sv_voip->integer)
		return true;  // VoIP disabled on this server.
	else if (!this->hasVoip_)  // client doesn't have VoIP support?!
		return true;
    
	// !!! FIXME: implement player blacklist.

	return false;  // don't ignore.
}

void SV_Client::userVoip( Message *msg, bool ignoreData )
{
	int sender, generation, sequence, frames, packetsize;
	uint8_t recips[(MAX_CLIENTS + 7) / 8];
	int flags;
	byte encoded[sizeof(this->voipPacket_[0]->data)];
	SV_Client *client = nullptr;
	voipServerPacket_t *packet = nullptr;
	int i;

	sender = this->clientNum_;
	generation = msg->readByte();
	sequence = msg->readLong();
	frames = msg->readByte();
	msg->readData( recips, sizeof(recips));
	flags = msg->readByte();
	packetsize = msg->readShort();

	if (msg->readcount > msg->cursize)
		return;   // short/invalid packet, bail.

	if (packetsize > sizeof (encoded)) {  // overlarge packet?
		int bytesleft = packetsize;
		while (bytesleft) {
			int br = bytesleft;
			if (br > sizeof (encoded))
				br = sizeof (encoded);
			msg->readData( encoded, br);
			bytesleft -= br;
		}
		return;   // overlarge packet, bail.
	}

	msg->readData( encoded, packetsize);

	if (ignoreData || this->shouldIgnoreVoipSender())
		return;   // Blacklisted, disabled, etc.

	// !!! FIXME: see if we read past end of msg...

	// !!! FIXME: reject if not opus data.
	// !!! FIXME: decide if this is bogus data?

	// decide who needs this VoIP packet sent to them...
	for (i = 1; i <= sv_maxclients->integer ; i++) {
		client = theSVS.svClients_.at(i);
		if (!client)
			continue;
		if (client->state_ != SV_Client::CS_ACTIVE)
			continue;  // not in the game yet, don't send to this guy.
		else if (i == sender)
			continue;  // don't send voice packet back to original author.
		else if (!client->hasVoip_)
			continue;  // no VoIP support, or unsupported protocol
		else if (client->muteAllVoip_)
			continue;  // client is ignoring everyone.
		else if (client->ignoreVoipFromClient_[sender])
			continue;  // client is ignoring this talker.
		else if (*this->downloadName_)   // !!! FIXME: possible to DoS?
			continue;  // no VoIP allowed if downloading, to save bandwidth.

		if(Common::isVoipTarget(recips, sizeof(recips), i))
			flags |= VOIP_DIRECT;
		else
			flags &= ~VOIP_DIRECT;

		if (!(flags & (VOIP_SPATIAL | VOIP_DIRECT)))
			continue;  // not addressed to this player.

		// Transmit this packet to the client.
		if (client->queuedVoipPackets_ >= ARRAY_LEN(client->voipPacket_)) {
			Common::printf("Too many VoIP packets queued for client #%d\n", i);
			continue;  // no room for another packet right now.
		}

		packet = static_cast<voipServerPacket_t *>(Z_Malloc(sizeof(*packet)));
		packet->sender = sender;
		packet->frames = frames;
		packet->len = packetsize;
		packet->generation = generation;
		packet->sequence = sequence;
		packet->flags = flags;
		memcpy(packet->data, encoded, packetsize);

		client->voipPacket_[(client->queuedVoipIndex_ + client->queuedVoipPackets_) % ARRAY_LEN(client->voipPacket_)] = packet;
		client->queuedVoipPackets_++;
	}
}
#endif

/*
===================
SV_ExecuteClientMessage

Parse a client packet
===================
*/
void SV_Client::executeClientMessage( Message *msg ) 
{
	int			c;
	int			serverId;

	msg->bitstream();

	serverId = msg->readLong();
	this->messageAcknowledge_ = msg->readLong();

	if (this->messageAcknowledge_ < 0) 
	{
		// usually only hackers create messages like this
		// it is more annoying for them to let them hanging
#ifndef NDEBUG
		this->drop( "DEBUG: illegible client message" );
#endif
		return;
	}

	this->reliableAcknowledge_ = msg->readLong();
	this->entityAck_ = msg->readLong();
	this->entityAckBits_ = msg->readLong();

	// NOTE: when the client message is fux0red the acknowledgement numbers
	// can be out of range, this could cause the server to send thousands of server
	// commands which the server thinks are not yet acknowledged in SV_UpdateServerCommandsToClient
	if ((this->reliableSequence_ - this->reliableAcknowledge_ >= MAX_RELIABLE_COMMANDS) || (this->reliableSequence_ - this->reliableAcknowledge_ < 0))
	{
		// usually only hackers create messages like this
		// it is more annoying for them to let them hanging
#ifndef NDEBUG
		this->drop( "DEBUG: illegible client message" );
#endif
		this->reliableAcknowledge_ = this->reliableSequence_;
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
	if ( serverId != theSV.serverId_ && !*this->downloadName_ && !strstr(this->lastClientCommandString_, "nextdl") ) 
	{
		if ( serverId >= theSV.restartedServerId_ && serverId < theSV.serverId_ ) 
		{	// TTimo - use a comparison here to catch multiple map_restart
			// they just haven't caught the map_restart yet
			Common::dPrintf("%s : ignoring pre map_restart / outdated client message\n", this->name_);
			return;
		}
		// if we can tell that the client has dropped the last
		// gamestate we sent them, resend it
		if ( this->state_ != SV_Client::CS_ACTIVE && this->messageAcknowledge_ > this->gamestateMessageNum_ ) 
		{
			Common::dPrintf( "%s : dropped gamestate, resending\n", this->name_ );
			this->sendClientGameState();
		}
		return;
	}

	// this client has acknowledged the new gamestate so it's
	// safe to start sending it the real time again
	if( this->oldServerTime_ && serverId == theSV.serverId_ ){
		Common::dPrintf( "%s acknowledged gamestate\n", this->name_ );
		this->oldServerTime_ = 0;
	}

	// read optional clientCommand strings
	do 
	{
		c = msg->readByte();
		if ( c == clc_EOF ) 
			break;

		if ( c != clc_clientCommand ) 
			break;

		if ( !this->clientCommand( msg ) ) 
			return;	// we couldn't execute it because of the flood protection

		if (this->state_ == SV_Client::CS_ZOMBIE) 
			return;	// disconnect command

	} while ( true );

	// skip legacy speex voip data
	if ( c == clc_voipSpeex ) {
#ifdef USE_VOIP
		this->userVoip( msg, true );
		c = msg->readByte();
#endif
	}

	// read optional voip data
	if ( c == clc_voipOpus ) {
#ifdef USE_VOIP
		this->userVoip( msg, false );
		c = msg->readByte();
#endif
	}

	// read the usercmd_t
	if ( c == clc_move ) 
		this->userMove( msg, true );
	else if ( c == clc_moveNoDelta ) 
		this->userMove( msg, false );
	else if ( c != clc_EOF ) 
		Common::printf( "WARNING: bad command byte for client %i\n", this->clientNum_ );

//	if ( msg->readcount != msg->cursize ) {
//		Common::printf( "WARNING: Junk at end of packet for client %i\n", this - svs.clients );
//	}
}

SV_Client::ClientSnapshot::ClientSnapshot() :
	areabytes_(0),
	num_entities_(0),
	first_entity_(0),
	messageSent_(0),
	messageAcked_(0),
	messageSize_(0)
{
	memset( areabits_, 0, sizeof(areabits_) );
	memset( &ps_, 0, sizeof(ps_) );
}

SV_Client::Netchan_Buffer::Netchan_Buffer() :
	next_(nullptr)
{
	memset( &msg_, 0, sizeof(msg_) );
	memset( msgBuffer_, 0, sizeof(msgBuffer_) );
}


SV_Client::SV_Client() :
	clientNum_(-1),
	state_(CS_FREE),
	reliableSequence_(0),
	reliableAcknowledge_(0),
	reliableSent_(0),
	messageAcknowledge_(0),
	gamestateMessageNum_(0),
	challenge_(0),
	lastMessageNum_(0),
	lastClientCommand_(0),
	//gentity_(0),
	download_(0),
 	downloadSize_(0),
 	downloadCount_(0),
	downloadClientBlock_(0),
	downloadCurrentBlock_(0),
	downloadXmitBlock_(0),
	downloadEOF_(false),
	downloadSendTime_(0),
	deltaMessage_(0),
	nextReliableTime_(0),
	lastPacketTime_(0),
	lastConnectTime_(0),
	lastSnapshotTime_(0),
	rateDelayed_(false),
	timeoutCount_(0),
	ping_(0),
	rate_(0),
	snapshotMsec_(0),
	pureAuthentic_(0),
	gotCP_(false),
	netchan_start_queue_(nullptr),
	netchan_end_queue_(nullptr),
	oldServerTime_(0)
{
	memset( csUpdated_, 0, sizeof( csUpdated_ ) );
	memset( userinfo_, 0, sizeof( userinfo_ ) );
	memset( reliableCommands_, 0, sizeof(reliableCommands_) );
	memset( &lastUsercmd_, 0, sizeof(lastUsercmd_) );
	memset( lastClientCommandString_, 0, sizeof(lastClientCommandString_) );
	memset( name_, 0, sizeof(name_) );
	memset( downloadName_, 0, sizeof(downloadName_) );
	memset( downloadBlocks_, 0, sizeof(downloadBlocks_) );
	memset( downloadBlockSize_, 0, sizeof(downloadBlockSize_) );
	memset( frames_, 0, sizeof(frames_) );
	entityTable_ = nullptr;
	entityReset_ = true;
	entityResetDone_ = false;
	entityAck_ = 0;
	entityAckBits_ = 0;
	memset( &netchan_, 0, sizeof(netchan_ ) );
#ifdef USE_VOIP
	hasVoip_ = false;
	muteAllVoip_ = false;
	memset( ignoreVoipFromClient_, 0, sizeof(ignoreVoipFromClient_) );
	memset( voipPacket_, 0, sizeof(voipPacket_) );
	queuedVoipPackets_ = 0;
	queuedVoipIndex_ = 0;
#endif
}


SV_Client::~SV_Client()
{
	delete entityTable_;
}

void
SV_Client::clear()
{
	clientNum_ = -1;
	state_ = CS_FREE;
	reliableSequence_ = 0;
	reliableAcknowledge_ = 0;
	reliableSent_ = 0;
	messageAcknowledge_ = 0;
	gamestateMessageNum_ = 0;
	challenge_ = 0;
	lastMessageNum_ = 0;
	lastClientCommand_ = 0;
	//gentity_ = 0;
	download_ = 0;
 	downloadSize_ = 0;
 	downloadCount_ = 0;
	downloadClientBlock_ = 0;
	downloadCurrentBlock_ = 0;
	downloadXmitBlock_ = 0;
	downloadEOF_ = false;
	downloadSendTime_ = 0;
	deltaMessage_ = 0;
	nextReliableTime_ = 0;
	lastPacketTime_ = 0;
	lastConnectTime_ = 0;
	lastSnapshotTime_ = 0;
	rateDelayed_ = false;
	timeoutCount_ = 0;
	ping_ = 0;
	rate_ = 0;
	snapshotMsec_ = 0;
	pureAuthentic_ = 0;
	gotCP_ = false;
	netchan_start_queue_ = nullptr;
	netchan_end_queue_ = nullptr;
	oldServerTime_ = 0;

	memset( csUpdated_, 0, sizeof( csUpdated_ ) );
	memset( userinfo_, 0, sizeof( userinfo_ ) );
	memset( reliableCommands_, 0, sizeof(reliableCommands_) );
	memset( &lastUsercmd_, 0, sizeof(lastUsercmd_) );
	memset( lastClientCommandString_, 0, sizeof(lastClientCommandString_) );
	memset( name_, 0, sizeof(name_) );
	memset( downloadName_, 0, sizeof(downloadName_) );
	memset( downloadBlocks_, 0, sizeof(downloadBlocks_) );
	memset( downloadBlockSize_, 0, sizeof(downloadBlockSize_) );
	memset( frames_, 0, sizeof(frames_) );
	entityReset_ = true;
	entityResetDone_ = false;
	entityAck_ = 0;
	entityAckBits_ = 0;
	memset( &netchan_, 0, sizeof(netchan_ ) );
#ifdef USE_VOIP
	hasVoip_ = false;
	muteAllVoip_ = false;
	memset( ignoreVoipFromClient_, 0, sizeof(ignoreVoipFromClient_) );
	memset( voipPacket_, 0, sizeof(voipPacket_) );
	queuedVoipPackets_ = 0;
	queuedVoipIndex_ = 0;
#endif
}

