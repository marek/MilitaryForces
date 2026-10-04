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
void SV_Client::netchanFreeQueue()
{
	SV_Client::Netchan_Buffer *netbuf, *next;
	
	for(netbuf = this->netchan_start_queue_; netbuf; netbuf = next)
	{
		next = netbuf->next_;
		Z_Free(netbuf);
	}
	
	this->netchan_start_queue_ = nullptr;
	this->netchan_end_queue_ = &this->netchan_start_queue_;
}

/*
=================
SV_Netchan_TransmitNextInQueue
=================
*/
void SV_Client::netchanTransmitNextInQueue()
{
	SV_Client::Netchan_Buffer *netbuf;
		
	Common::dPrintf("#462 Netchan_TransmitNextFragment: popping a queued message for transmit\n");
	netbuf = this->netchan_start_queue_;

	this->netchan_.transmit( netbuf->msg_.size(), netbuf->msg_.data() );

	// pop from queue
	this->netchan_start_queue_ = netbuf->next_;
	if(!this->netchan_start_queue_)
	{
		Common::dPrintf("#462 Netchan_TransmitNextFragment: emptied queue\n");
		this->netchan_end_queue_ = &this->netchan_start_queue_;
	}
	else
		Common::dPrintf("#462 Netchan_TransmitNextFragment: remaining queued message\n");

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

int SV_Client::netchanTransmitNextFragment()
{
	if(this->netchan_.unsentFragments())
	{
		this->netchan_.transmitNextFragment();
		return this->rateMsec();
	}
	else if(this->netchan_start_queue_)
	{
		this->netchanTransmitNextInQueue();
		return this->rateMsec();
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

void SV_Client::netchanTransmit( Message *msg )
{
	msg->writeByte( svc_EOF );

	if(this->netchan_.unsentFragments() || this->netchan_start_queue_)
	{
		SV_Client::Netchan_Buffer *netbuf;
		Common::dPrintf("#462 SV_Netchan_Transmit: unsent fragments, stacked\n");
		netbuf = static_cast<SV_Client::Netchan_Buffer *>(Z_Malloc(sizeof(SV_Client::Netchan_Buffer)));
		// store the msg, we can't store it encoded, as the encoding depends on stuff we still have to finish sending
		netbuf->msg_.copy( netbuf->msgBuffer_, sizeof( netbuf->msgBuffer_ ), msg);
		netbuf->next_ = nullptr;
		// insert it in the queue, the message will be encoded and sent later
		*this->netchan_end_queue_ = netbuf;
		this->netchan_end_queue_ = &(*this->netchan_end_queue_)->next_;
	}
	else
	{
		this->netchan_.transmit( msg->size(), msg->data() );
	}
}

/*
=================
Netchan_SV_Process
=================
*/
bool SV_Client::netchanProcess( Message *msg ) 
{
	int ret;
	ret = this->netchan_.process( msg );
	if (!ret)
		return false;

	return true;
}

