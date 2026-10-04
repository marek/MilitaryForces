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
// sv_bot.c

#include "server.h"
#include "../game/game.h"
#include "sv_server.h"
#include "sv_client.h"
//#include "../botlib/botlib.h"


//extern botlib_export_t	*botlib_export;
int	bot_enable;




/*
==================
BotImport_GetMemory
==================
*/
void *BotImport_GetMemory(int size) 
{
	void *ptr;

	ptr = Zone::tagMalloc( size, TAG_BOTLIB );
	return ptr;
}

/*
==================
BotImport_FreeMemory
==================
*/
void BotImport_FreeMemory(void *ptr) 
{
	Zone::free(ptr);
}

/*
=================
BotImport_HunkAlloc
=================
*/
void *BotImport_HunkAlloc( int size ) 
{
	if( Hunk_CheckMark() ) {
		Common::error( ERR_DROP, "SV_Bot_HunkAlloc: Alloc with marks already set\n" );
	}
	return Hunk_Alloc( size, h_high );
}



/*
==================
SV_BotAllocateClient
==================
*/
int SV_BotAllocateClient()
{
	int			i;
	SV_Client	*cl = nullptr;

	// find a client slot
	for( i = sv_privateClients->integer + 1; i <= sv_maxclients->integer; i++ ) 
	{
		cl = theSVS.svClients_.at(i);
		if( !cl )
		{
			cl = new SV_Client;
			break;
		}
		if( cl->state_ == SV_Client::CS_FREE ) 
			break;
	}
	if( i > sv_maxclients->integer ) 
		return -1;

	cl->clear();
	cl->clientNum_ = i;
	theSVS.svClients_.at(i) = cl;
	cl->state_ = SV_Client::CS_ACTIVE;
	cl->lastPacketTime_ = theSVS.time_;
	cl->netchan_.setRemoteType( NA_BOT );
	cl->rate_ = 16384;

	return i;
}

/*
==================
SV_BotFreeClient
==================
*/
void SV_BotFreeClient( int clientNum ) 
{
	if( clientNum < 1 || clientNum > sv_maxclients->integer || !theSVS.svClients_.at(clientNum) ) 
		Common::error( ERR_DROP, "SV_BotFreeClient: bad clientNum: %i", clientNum );

	SV_Client* cl = theSVS.svClients_.at(clientNum);
	cl->state_ = SV_Client::CS_FREE;
	cl->name_[0] = 0;
}

/*
==================
SV_BotUserCommand
==================
*/
void SV_BotUserCommand( int clientNum, usercmd_t *cmd ) 
{
	if( clientNum < 1 || clientNum > sv_maxclients->integer || !theSVS.svClients_.at(clientNum) ) 
		return;

	SV_Client* cl = theSVS.svClients_.at(clientNum);
	cl->lastPacketTime_ = theSVS.time_;
	cl->reliableAcknowledge_ = cl->reliableSequence_;
	cl->think( cmd );
}
