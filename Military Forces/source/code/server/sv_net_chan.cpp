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

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "server.h"
#include "sv_client.h"

/*
=================
SV_Netchan_FreeQueue
=================
*/
void SV_Netchan_FreeQueue(SV_Client *client)
{
	SV_Client::Netchan_Buffer *netbuf, *next;
	
	for(netbuf = client->netchan_start_queue_; netbuf; netbuf = next)
	{
		next = netbuf->next_;
		Z_Free(netbuf);
	}
	
	client->netchan_start_queue_ = nullptr;
	client->netchan_end_queue_ = &client->netchan_start_queue_;
}

/*
=================
SV_Netchan_TransmitNextInQueue
=================
*/
void SV_Netchan_TransmitNextInQueue(SV_Client *client)
{
	SV_Client::Netchan_Buffer *netbuf;
		
	Com_DPrintf("#462 Netchan_TransmitNextFragment: popping a queued message for transmit\n");
	netbuf = client->netchan_start_queue_;

	Netchan_Transmit(&client->netchan_, netbuf->msg_.cursize, netbuf->msg_.data);

	// pop from queue
	client->netchan_start_queue_ = netbuf->next_;
	if(!client->netchan_start_queue_)
	{
		Com_DPrintf("#462 Netchan_TransmitNextFragment: emptied queue\n");
		client->netchan_end_queue_ = &client->netchan_start_queue_;
	}
	else
		Com_DPrintf("#462 Netchan_TransmitNextFragment: remaining queued message\n");

	Z_Free(netbuf);
}

/*
=================
SV_Netchan_TransmitNextFragment
Transmit the next fragment and the next queued packet
Return number of ms until next message can be sent based on throughput given by client rate,
-1 if no packet was sent.
=================
*/

int SV_Netchan_TransmitNextFragment(SV_Client *client)
{
	if(client->netchan_.unsentFragments)
	{
		Netchan_TransmitNextFragment(&client->netchan_);
		return SV_RateMsec(client);
	}
	else if(client->netchan_start_queue_)
	{
		SV_Netchan_TransmitNextInQueue(client);
		return SV_RateMsec(client);
	}
	
	return -1;
}


/*
===============
SV_Netchan_Transmit
TTimo
https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=462
if there are some unsent fragments (which may happen if the snapshots
and the gamestate are fragmenting, and collide on send for instance)
then buffer them and make sure they get sent in correct order
================
*/

void SV_Netchan_Transmit( SV_Client *client, msg_t *msg)
{
	MSG_WriteByte( msg, svc_EOF );

	if(client->netchan_.unsentFragments || client->netchan_start_queue_)
	{
		SV_Client::Netchan_Buffer *netbuf;
		Com_DPrintf("#462 SV_Netchan_Transmit: unsent fragments, stacked\n");
		netbuf = (SV_Client::Netchan_Buffer *) Z_Malloc(sizeof(SV_Client::Netchan_Buffer));
		// store the msg, we can't store it encoded, as the encoding depends on stuff we still have to finish sending
		MSG_Copy(&netbuf->msg_, netbuf->msgBuffer_, sizeof( netbuf->msgBuffer_ ), msg);
		netbuf->next_ = nullptr;
		// insert it in the queue, the message will be encoded and sent later
		*client->netchan_end_queue_ = netbuf;
		client->netchan_end_queue_ = &(*client->netchan_end_queue_)->next_;
	}
	else
	{
		Netchan_Transmit( &client->netchan_, msg->cursize, msg->data );
	}
}

/*
=================
Netchan_SV_Process
=================
*/
bool SV_Netchan_Process( SV_Client *client, msg_t *msg ) 
{
	int ret;
	ret = Netchan_Process( &client->netchan_, msg );
	if (!ret)
		return false;

	return true;
}

