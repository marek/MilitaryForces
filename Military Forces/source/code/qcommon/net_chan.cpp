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
#include "qcommon.h"

/*

packet header
-------------
4	outgoing sequence.  high bit will be set if this is a fragmented message
[2	qport (only for client to server)]
[4	fragment start byte]
[2	fragment length. if < FRAGMENT_SIZE, this is the last fragment]

if the sequence number is -1, the packet should be handled as an out-of-band
message instead of as part of a netcon.

All fragments will have the same sequence numbers.

The qport field is a workaround for bad address translating routers that
sometimes remap the client's source port on a packet during gameplay.

If the base part of the net address matches and the qport matches, then the
channel matches even if the IP port differs.  The IP port should be updated
to the new value before sending out any replies.

*/


constexpr int MAX_PACKETLEN = 1400; // max size of a network packet

#define	FRAGMENT_SIZE			(MAX_PACKETLEN - 100)
constexpr int PACKET_HEADER = 10; // two ints and a short

constexpr unsigned int FRAGMENT_BIT = (1U<<31);

static cvar_t		*showpackets;
static cvar_t		*showdrop;
cvar_t		*net_qport;

char *NetChannel::netsrcString[2] = {
	"client",
	"server"
};

/*
===============
Netchan_Init

===============
*/
void Netchan_Init( int port ) {
	port &= 0xffff;
	showpackets = CvarSystem::get ("showpackets", "0", CVAR_TEMP );
	showdrop = CvarSystem::get ("showdrop", "0", CVAR_TEMP );
	net_qport = CvarSystem::get ("net_qport", va("%i", port), CVAR_INIT );
	MSG_InitHuffStats();
}

/*
==============
Netchan_Setup

called to open a channel to a remote system
==============
*/
void NetChannel::setup( netsrc_t sock, netadr_t adr, int qport, int challenge, bool compat )
{
	Com_Memset (this, 0, sizeof(*this));
	
	this->sock_ = sock;
	this->remoteAddress_ = adr;
	this->qport_ = qport;
	this->incomingSequence_ = 0;
	this->outgoingSequence_ = 1;
	this->challenge_ = challenge;

#ifdef LEGACY_PROTOCOL
	this->compat_ = compat;
#endif
}

/*
=================
Netchan_TransmitNextFragment

Send one fragment of the current message
=================
*/
void NetChannel::transmitNextFragment() {
	Message		send;
	byte		send_buf[MAX_PACKETLEN];
	int			fragmentLength;
	int			outgoingSequence;

	// write the packet header
	send = Message::outOfBand( send_buf, sizeof(send_buf) );				// <-- only do the oob here

	outgoingSequence = this->outgoingSequence_ | FRAGMENT_BIT;
	send.writeLong( outgoingSequence);

	// send the qport if we are a client
	if ( this->sock_ == NS_CLIENT ) {
		send.writeShort( net_qport->integer );
	}

#ifdef LEGACY_PROTOCOL
	if(!this->compat_)
#endif
		send.writeLong( NETCHAN_GENCHECKSUM(this->challenge_, this->outgoingSequence_));

	// copy the reliable message to the packet first
	fragmentLength = FRAGMENT_SIZE;
	if ( this->unsentFragmentStart_  + fragmentLength > this->unsentLength_ ) {
		fragmentLength = this->unsentLength_ - this->unsentFragmentStart_;
	}

	send.writeLong( this->unsentFragmentStart_ );
	send.writeShort( fragmentLength );
	send.writeData( this->unsentBuffer_ + this->unsentFragmentStart_, fragmentLength );

	// send the datagram
	Network::sendPacket(this->sock_, send.size(), send.data(), this->remoteAddress_);
	
	// Store send time and size of this packet for rate control
	this->lastSentTime_ = System::milliseconds();
	this->lastSentSize_ = send.size();

	if ( showpackets->integer ) {
		Common::printf ("%s send %4i : s=%i fragment=%i,%i\n"
			, netsrcString[ this->sock_ ]
			, send.size()
			, this->outgoingSequence_
			, this->unsentFragmentStart_, fragmentLength);
	}

	this->unsentFragmentStart_ += fragmentLength;

	// this exit condition is a little tricky, because a packet
	// that is exactly the fragment length still needs to send
	// a second packet of zero length so that the other side
	// can tell there aren't more to follow
	if ( this->unsentFragmentStart_ == this->unsentLength_ && fragmentLength != FRAGMENT_SIZE ) {
		this->outgoingSequence_++;
		this->unsentFragments_ = false;
	}
}


/*
===============
Netchan_Transmit

Sends a message to a connection, fragmenting if necessary
A 0 length will still generate a packet.
================
*/
void NetChannel::transmit( int length, const byte *data ) {
	Message		send;
	byte		send_buf[MAX_PACKETLEN];

	if ( length > MAX_MSGLEN ) {
		Common::error( ERR_DROP, "Netchan_Transmit: length = %i", length );
	}
	this->unsentFragmentStart_ = 0;

	// fragment large reliable messages
	if ( length >= FRAGMENT_SIZE ) {
		this->unsentFragments_ = true;
		this->unsentLength_ = length;
		Com_Memcpy( this->unsentBuffer_, data, length );

		// only send the first fragment now
		this->transmitNextFragment();

		return;
	}

	// write the packet header
	send = Message::outOfBand( send_buf, sizeof(send_buf) );

	send.writeLong( this->outgoingSequence_ );

	// send the qport if we are a client
	if(this->sock_ == NS_CLIENT)
		send.writeShort( net_qport->integer);

#ifdef LEGACY_PROTOCOL
	if(!this->compat_)
#endif
		send.writeLong( NETCHAN_GENCHECKSUM(this->challenge_, this->outgoingSequence_));

	this->outgoingSequence_++;

	send.writeData( data, length );

	// send the datagram
	Network::sendPacket( this->sock_, send.size(), send.data(), this->remoteAddress_ );

	// Store send time and size of this packet for rate control
	this->lastSentTime_ = System::milliseconds();
	this->lastSentSize_ = send.size();

	if ( showpackets->integer ) {
		Common::printf( "%s send %4i : s=%i ack=%i\n"
			, netsrcString[ this->sock_ ]
			, send.size()
			, this->outgoingSequence_ - 1
			, this->incomingSequence_ );
	}
}

/*
=================
Netchan_Process

Returns false if the message should not be processed due to being
out of order or a fragment.

Msg must be large enough to hold MAX_MSGLEN, because if this is the
final fragment of a multi-part message, the entire thing will be
copied out.
=================
*/
bool NetChannel::process( Message *msg ) {
	int			sequence;
	int			fragmentStart, fragmentLength;
	bool	fragmented;

	// XOR unscramble all data in the packet after the header
//	Netchan_UnScramblePacket( msg );

	// get sequence numbers		
	msg->beginReadingOOB();
	sequence = msg->readLong();

	// check for fragment information
	if ( sequence & FRAGMENT_BIT ) {
		sequence &= ~FRAGMENT_BIT;
		fragmented = true;
	} else {
		fragmented = false;
	}

	// read the qport if we are a server
	if ( this->sock_ == NS_SERVER ) {
		msg->readShort();
	}

#ifdef LEGACY_PROTOCOL
	if(!this->compat_)
#endif
	{
		int checksum = msg->readLong();

		// UDP spoofing protection
		if(NETCHAN_GENCHECKSUM(this->challenge_, sequence) != checksum)
			return false;
	}

	// read the fragment information
	if ( fragmented ) {
		fragmentStart = msg->readLong();
		fragmentLength = msg->readShort();
	} else {
		fragmentStart = 0;		// stop warning message
		fragmentLength = 0;
	}

	if ( showpackets->integer ) {
		if ( fragmented ) {
			Common::printf( "%s recv %4i : s=%i fragment=%i,%i\n"
				, netsrcString[ this->sock_ ]
				, msg->size()
				, sequence
				, fragmentStart, fragmentLength );
		} else {
			Common::printf( "%s recv %4i : s=%i\n"
				, netsrcString[ this->sock_ ]
				, msg->size()
				, sequence );
		}
	}

	//
	// discard out of order or duplicated packets
	//
	if ( sequence <= this->incomingSequence_ ) {
		if ( showdrop->integer || showpackets->integer ) {
			Common::printf( "%s:Out of order packet %i at %i\n"
				, Network::adrToString( this->remoteAddress_ )
				,  sequence
				, this->incomingSequence_ );
		}
		return false;
	}

	//
	// dropped packets don't keep the message from being used
	//
	this->dropped_ = sequence - (this->incomingSequence_+1);
	if ( this->dropped_ > 0 ) {
		if ( showdrop->integer || showpackets->integer ) {
			Common::printf( "%s:Dropped %i packets at %i\n"
			, Network::adrToString( this->remoteAddress_ )
			, this->dropped_
			, sequence );
		}
	}
	

	//
	// if this is the final framgent of a reliable message,
	// bump incoming_reliable_sequence 
	//
	if ( fragmented ) {
		// TTimo
		// make sure we add the fragments in correct order
		// either a packet was dropped, or we received this one too soon
		// we don't reconstruct the fragments. we will wait till this fragment gets to us again
		// (NOTE: we could probably try to rebuild by out of order chunks if needed)
		if ( sequence != this->fragmentSequence_ ) {
			this->fragmentSequence_ = sequence;
			this->fragmentLength_ = 0;
		}

		// if we missed a fragment, dump the message
		if ( fragmentStart != this->fragmentLength_ ) {
			if ( showdrop->integer || showpackets->integer ) {
				Common::printf( "%s:Dropped a message fragment\n"
				, Network::adrToString( this->remoteAddress_ ));
			}
			// we can still keep the part that we have so far,
			// so we don't need to clear this->fragmentLength_
			return false;
		}

		// copy the fragment to the fragment buffer
		if ( fragmentLength < 0 || msg->readCount() + fragmentLength > msg->size() ||
			this->fragmentLength_ + fragmentLength > sizeof( this->fragmentBuffer_ ) ) {
			if ( showdrop->integer || showpackets->integer ) {
				Common::printf ("%s:illegal fragment length\n"
				, Network::adrToString (this->remoteAddress_ ) );
			}
			return false;
		}

		Com_Memcpy( this->fragmentBuffer_ + this->fragmentLength_, 
			msg->data() + msg->readCount(), fragmentLength );

		this->fragmentLength_ += fragmentLength;

		// if this wasn't the last fragment, don't process anything
		if ( fragmentLength == FRAGMENT_SIZE ) {
			return false;
		}

		if ( this->fragmentLength_ > msg->maxSize() ) {
			Common::printf( "%s:fragmentLength %i > msg->maxsize\n"
				, Network::adrToString (this->remoteAddress_ ),
				this->fragmentLength_ );
			return false;
		}

		// copy the full message over the partial fragment

		// make sure the sequence number is still there
		*reinterpret_cast<int *>(msg->data()) = LittleLong( sequence );

		Com_Memcpy( msg->data() + 4, this->fragmentBuffer_, this->fragmentLength_ );
		msg->setSize( this->fragmentLength_ + 4 );
		this->fragmentLength_ = 0;
		msg->setReadCount( 4 );	// past the sequence number
		msg->setBit( 32 );	// past the sequence number

		// TTimo
		// clients were not acking fragmented messages
		this->incomingSequence_ = sequence;
		
		return true;
	}

	//
	// the message can now be read from the current message pointer
	//
	this->incomingSequence_ = sequence;

	return true;
}


//==============================================================================


/*
=============================================================================

LOOPBACK BUFFERS FOR LOCAL PLAYER

=============================================================================
*/

// there needs to be enough loopback messages to hold a complete
// gamestate of maximum size
constexpr int MAX_LOOPBACK = 16;

struct loopmsg_t {
	byte	data[MAX_PACKETLEN];
	int		datalen;
};

struct loopback_t {
	loopmsg_t	msgs[MAX_LOOPBACK];
	int			get, send;
};

loopback_t	loopbacks[2];


bool	Network::getLoopPacket (netsrc_t sock, netadr_t *net_from, Message *net_message)
{
	int		i;
	loopback_t	*loop;

	loop = &loopbacks[sock];

	if (loop->send - loop->get > MAX_LOOPBACK)
		loop->get = loop->send - MAX_LOOPBACK;

	if (loop->get >= loop->send)
		return false;

	i = loop->get & (MAX_LOOPBACK-1);
	loop->get++;

	Com_Memcpy (net_message->data(), loop->msgs[i].data, loop->msgs[i].datalen);
	net_message->setSize( loop->msgs[i].datalen );
	Com_Memset (net_from, 0, sizeof(*net_from));
	net_from->type = NA_LOOPBACK;
	return true;

}


static void NET_SendLoopPacket (netsrc_t sock, int length, const void *data, netadr_t to)
{
	int		i;
	loopback_t	*loop;

	loop = &loopbacks[sock^1];

	i = loop->send & (MAX_LOOPBACK-1);
	loop->send++;

	Com_Memcpy (loop->msgs[i].data, data, length);
	loop->msgs[i].datalen = length;
}

//=============================================================================

struct packetQueue_s {
        struct packetQueue_s *next;
        int length;
        byte *data;
        netadr_t to;
        int release;
};
using packetQueue_t = packetQueue_s;

packetQueue_t *packetQueue = nullptr;

void Network::queuePacket( int length, const void *data, netadr_t to,
	int offset )
{
	packetQueue_t *newPacket, *next = packetQueue;

	if(offset > 999)
		offset = 999;

	newPacket = static_cast<packetQueue_t *>(Zone::smallMalloc(sizeof(packetQueue_t)));
	newPacket->data = static_cast<byte *>(Zone::smallMalloc(length));
	Com_Memcpy(newPacket->data, data, length);
	newPacket->length = length;
	newPacket->to = to;
	newPacket->release = System::milliseconds() + static_cast<int>(static_cast<float>(offset) / com_timescale->value);	
	newPacket->next = nullptr;

	if(!packetQueue) {
		packetQueue = newPacket;
		return;
	}
	while(next) {
		if(!next->next) {
			next->next = newPacket;
			return;
		}
		next = next->next;
	}
}

void Network::flushPacketQueue()
{
	packetQueue_t *last;
	int now;

	while(packetQueue) {
		now = System::milliseconds();
		if(packetQueue->release >= now)
			break;
		Sys_SendPacket(packetQueue->length, packetQueue->data,
			packetQueue->to);
		last = packetQueue;
		packetQueue = packetQueue->next;
		Zone::free(last->data);
		Zone::free(last);
	}
}

void Network::sendPacket( netsrc_t sock, int length, const void *data, netadr_t to ) {

	// sequenced packets are shown in netchan, so just show oob
	if ( showpackets->integer && *(int *)data == -1 )	{
		Common::printf ("send packet %4i\n", length);
	}

	if ( to.type == NA_LOOPBACK ) {
		NET_SendLoopPacket (sock, length, data, to);
		return;
	}
	if ( to.type == NA_BOT ) {
		return;
	}
	if ( to.type == NA_BAD ) {
		return;
	}

	if ( sock == NS_CLIENT && cl_packetdelay->integer > 0 ) {
		Network::queuePacket( length, data, to, cl_packetdelay->integer );
	}
	else if ( sock == NS_SERVER && sv_packetdelay->integer > 0 ) {
		Network::queuePacket( length, data, to, sv_packetdelay->integer );
	}
	else {
		Sys_SendPacket( length, data, to );
	}
}

/*
===============
Network::outOfBandPrint

Sends a text message in an out-of-band datagram
================
*/
void QDECL Network::outOfBandPrint( netsrc_t sock, netadr_t adr, const char *format, ... ) {
	va_list		argptr;
	static char	string[MAX_MSGLEN];


	// set the header
	string[0] = -1;
	string[1] = -1;
	string[2] = -1;
	string[3] = -1;

	va_start( argptr, format );
	Q_vsnprintf( string+4, sizeof(string)-4, format, argptr );
	va_end( argptr );

	// send the datagram
	Network::sendPacket( sock, strlen( string ), string, adr );
}

/*
===============
Network::outOfBandPrint

Sends a data message in an out-of-band datagram (only used for "connect")
================
*/
void QDECL Network::outOfBandData( netsrc_t sock, netadr_t adr, byte *format, int len ) {
	static byte	string[MAX_MSGLEN*2];
	int			i;
	Message		mbuf;

	// set the header
	string[0] = 0xff;
	string[1] = 0xff;
	string[2] = 0xff;
	string[3] = 0xff;

	for(i=0;i<len;i++) {
		string[i+4] = format[i];
	}

	mbuf.setData( string );
	mbuf.setSize( len+4 );
	Huff_Compress( &mbuf, 12);
	// send the datagram
	Network::sendPacket( sock, mbuf.size(), mbuf.data(), adr );
}

/*
=============
Network::stringToAdr

Traps "localhost" for loopback, passes everything else to system
return 0 on address not found, 1 on address found with port, 2 on address found without port.
=============
*/
int Network::stringToAdr( const char *s, netadr_t *a, netadrtype_t family )
{
	char	base[MAX_STRING_CHARS], *search;
	char	*port = nullptr;

	if (!strcmp (s, "localhost")) {
		Com_Memset (a, 0, sizeof(*a));
		a->type = NA_LOOPBACK;
// as NA_LOOPBACK doesn't require ports report port was given.
		return 1;
	}

	Q_strncpyz( base, s, sizeof( base ) );
	
	if(*base == '[' || Q_CountChar(base, ':') > 1)
	{
		// This is an ipv6 address, handle it specially.
		search = strchr(base, ']');
		if(search)
		{
			*search = '\0';
			search++;

			if(*search == ':')
				port = search + 1;
		}
		
		if(*base == '[')
			search = base + 1;
		else
			search = base;
	}
	else
	{
		// look for a port number
		port = strchr( base, ':' );
		
		if ( port ) {
			*port = '\0';
			port++;
		}
		
		search = base;
	}

	if(!Sys_StringToAdr(search, a, family))
	{
		a->type = NA_BAD;
		return 0;
	}

	if(port)
	{
		a->port = BigShort((short) atoi(port));
		return 1;
	}
	else
	{
		a->port = BigShort(PORT_SERVER);
		return 2;
	}
}
