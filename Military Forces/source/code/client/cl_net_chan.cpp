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

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "client.h"

#ifdef LEGACY_PROTOCOL
/*
==============
Client::netchan_Encode

	// first 12 bytes of the data are always:
	long serverId;
	long messageAcknowledge;
	long reliableAcknowledge;

==============
*/
void Client::netchan_Encode( Message *msg ) {
	int serverId, messageAcknowledge, reliableAcknowledge;
	int i, index, srdc, sbit, soob;
	byte key, *string;

	if ( msg->cursize <= CL_ENCODE_START ) {
		return;
	}

        srdc = msg->readcount;
        sbit = msg->bit;
        soob = msg->oob;
        
        msg->bit = 0;
        msg->readcount = 0;
        msg->oob = 0;
        
        serverId = msg->readLong();
	messageAcknowledge = msg->readLong();
	reliableAcknowledge = msg->readLong();

        msg->oob = soob;
        msg->bit = sbit;
        msg->readcount = srdc;
        
	string = (byte *)clc.serverCommands[ reliableAcknowledge & (MAX_RELIABLE_COMMANDS-1) ];
	index = 0;
	//
	key = clc.challenge ^ serverId ^ messageAcknowledge;
	for (i = CL_ENCODE_START; i < msg->cursize; i++) {
		// modify the key with the last received now acknowledged server command
		if (!string[index])
			index = 0;
		if (string[index] > 127 || string[index] == '%') {
			key ^= '.' << (i & 1);
		}
		else {
			key ^= string[index] << (i & 1);
		}
		index++;
		// encode the data with this key
		*(msg->data + i) = (*(msg->data + i)) ^ key;
	}
}

/*
==============
Client::netchan_Decode

	// first four bytes of the data are always:
	long reliableAcknowledge;

==============
*/
void Client::netchan_Decode( Message *msg ) {
	long reliableAcknowledge, i, index;
	byte key, *string;
        int	srdc, sbit, soob;

        srdc = msg->readcount;
        sbit = msg->bit;
        soob = msg->oob;
        
        msg->oob = 0;
        
	reliableAcknowledge = msg->readLong();

        msg->oob = soob;
        msg->bit = sbit;
        msg->readcount = srdc;

	string = (byte *) clc.reliableCommands[ reliableAcknowledge & (MAX_RELIABLE_COMMANDS-1) ];
	index = 0;
	// xor the client challenge with the netchan sequence number (need something that changes every message)
	key = clc.challenge ^ LittleLong( *(unsigned *)msg->data );
	for (i = msg->readcount + CL_DECODE_START; i < msg->cursize; i++) {
		// modify the key with the last sent and with this message acknowledged client command
		if (!string[index])
			index = 0;
		if (string[index] > 127 || string[index] == '%') {
			key ^= '.' << (i & 1);
		}
		else {
			key ^= string[index] << (i & 1);
		}
		index++;
		// decode the data with this key
		*(msg->data + i) = *(msg->data + i) ^ key;
	}
}
#endif

/*
=================
CL_Netchan_TransmitNextFragment
=================
*/
bool CL_Netchan_TransmitNextFragment(NetChannel *chan)
{
	if(chan->unsentFragments())
	{
		chan->transmitNextFragment();
		return true;
	}
	
	return false;
}

/*
===============
Client::netchan_Transmit
================
*/
void Client::netchan_Transmit( NetChannel *chan, Message* msg ) {
	msg->writeByte( clc_EOF );

#ifdef LEGACY_PROTOCOL
	if(chan->compat())
		Client::netchan_Encode(msg);
#endif

	chan->transmit( msg->size(), msg->data() );
	
	// Transmit all fragments without delay
	while(CL_Netchan_TransmitNextFragment(chan))
	{
		Common::dPrintf("WARNING: #462 unsent fragments (not supposed to happen!)\n");
	}
}

/*
=================
Client::netchan_Process
=================
*/
bool Client::netchan_Process( NetChannel *chan, Message *msg ) {
	int ret;

	ret = chan->process( msg );
	if (!ret)
		return false;

#ifdef LEGACY_PROTOCOL
	if(chan->compat())
		Client::netchan_Decode(msg);
#endif

	return true;
}
