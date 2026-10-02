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

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

typedef u_long ioctlarg_t;

#define socketError		WSAGetLastError()

#define EAGAIN			WSAEWOULDBLOCK
#define ECONNRESET		WSAECONNRESET
#define EADDRNOTAVAIL	WSAEADDRNOTAVAIL
#define EAFNOSUPPORT	WSAEAFNOSUPPORT

static WSADATA	winsockdata;

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

typedef int SOCKET;
typedef int ioctlarg_t;

#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#define closesocket		close
#define ioctlsocket		ioctl
#define socketError		errno

#endif

static bool	winsockInitialized = false;
static bool usingSocks = false;
static bool networkingEnabled = false;

static cvar_t	*net_noudp;

static cvar_t	*net_socksEnabled;
static cvar_t	*net_socksServer;
static cvar_t	*net_socksPort;
static cvar_t	*net_socksUsername;
static cvar_t	*net_socksPassword;
static struct sockaddr	socksRelayAddr;

static SOCKET	ip_socket = INVALID_SOCKET;
static SOCKET	socks_socket = INVALID_SOCKET;

#define	MAX_IPS		16
static	int		numIP;
static	byte	localIP[MAX_IPS][4];

//=============================================================================


/*
====================
NET_ErrorString
====================
*/
const char *NET_ErrorString( void ) {
#ifdef _WIN32
	int		code;

	code = WSAGetLastError();
	switch( code ) {
	case WSAEINTR: return "WSAEINTR";
	case WSAEBADF: return "WSAEBADF";
	case WSAEACCES: return "WSAEACCES";
	case WSAEDISCON: return "WSAEDISCON";
	case WSAEFAULT: return "WSAEFAULT";
	case WSAEINVAL: return "WSAEINVAL";
	case WSAEMFILE: return "WSAEMFILE";
	case WSAEWOULDBLOCK: return "WSAEWOULDBLOCK";
	case WSAEINPROGRESS: return "WSAEINPROGRESS";
	case WSAEALREADY: return "WSAEALREADY";
	case WSAENOTSOCK: return "WSAENOTSOCK";
	case WSAEDESTADDRREQ: return "WSAEDESTADDRREQ";
	case WSAEMSGSIZE: return "WSAEMSGSIZE";
	case WSAEPROTOTYPE: return "WSAEPROTOTYPE";
	case WSAENOPROTOOPT: return "WSAENOPROTOOPT";
	case WSAEPROTONOSUPPORT: return "WSAEPROTONOSUPPORT";
	case WSAESOCKTNOSUPPORT: return "WSAESOCKTNOSUPPORT";
	case WSAEOPNOTSUPP: return "WSAEOPNOTSUPP";
	case WSAEPFNOSUPPORT: return "WSAEPFNOSUPPORT";
	case WSAEAFNOSUPPORT: return "WSAEAFNOSUPPORT";
	case WSAEADDRINUSE: return "WSAEADDRINUSE";
	case WSAEADDRNOTAVAIL: return "WSAEADDRNOTAVAIL";
	case WSAENETDOWN: return "WSAENETDOWN";
	case WSAENETUNREACH: return "WSAENETUNREACH";
	case WSAENETRESET: return "WSAENETRESET";
	case WSAECONNABORTED: return "WSAECONNABORTED";
	case WSAECONNRESET: return "WSAECONNRESET";
	case WSAENOBUFS: return "WSAENOBUFS";
	case WSAEISCONN: return "WSAEISCONN";
	case WSAENOTCONN: return "WSAENOTCONN";
	case WSAESHUTDOWN: return "WSAESHUTDOWN";
	case WSAETOOMANYREFS: return "WSAETOOMANYREFS";
	case WSAETIMEDOUT: return "WSAETIMEDOUT";
	case WSAECONNREFUSED: return "WSAECONNREFUSED";
	case WSAELOOP: return "WSAELOOP";
	case WSAENAMETOOLONG: return "WSAENAMETOOLONG";
	case WSAEHOSTDOWN: return "WSAEHOSTDOWN";
	case WSASYSNOTREADY: return "WSASYSNOTREADY";
	case WSAVERNOTSUPPORTED: return "WSAVERNOTSUPPORTED";
	case WSANOTINITIALISED: return "WSANOTINITIALISED";
	case WSAHOST_NOT_FOUND: return "WSAHOST_NOT_FOUND";
	case WSATRY_AGAIN: return "WSATRY_AGAIN";
	case WSANO_RECOVERY: return "WSANO_RECOVERY";
	case WSANO_DATA: return "WSANO_DATA";
	default: return "NO ERROR";
	}
#else
	return strerror( errno );
#endif
}

static void NetadrToSockadr( netadr_t *a, struct sockaddr *s ) {
	memset( s, 0, sizeof(*s) );

	if( a->type == NA_BROADCAST ) {
		((struct sockaddr_in *)s)->sin_family = AF_INET;
		((struct sockaddr_in *)s)->sin_port = a->port;
		((struct sockaddr_in *)s)->sin_addr.s_addr = INADDR_BROADCAST;
	}
	else if( a->type == NA_IP ) {
		((struct sockaddr_in *)s)->sin_family = AF_INET;
		memcpy( &((struct sockaddr_in *)s)->sin_addr, a->ip, 4 );
		((struct sockaddr_in *)s)->sin_port = a->port;
	}
}


static void SockadrToNetadr( struct sockaddr *s, netadr_t *a ) {
	if (s->sa_family == AF_INET) {
		a->type = NA_IP;
		memcpy( a->ip, &((struct sockaddr_in *)s)->sin_addr, 4 );
		a->port = ((struct sockaddr_in *)s)->sin_port;
	}
}


/*
=============
Sys_StringToSockaddr
=============
*/
static bool Sys_StringToSockaddr( const char *s, struct sockaddr *sadr ) {
	struct addrinfo	hints;
	struct addrinfo	*res = NULL;

	memset( sadr, 0, sizeof( *sadr ) );
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if( getaddrinfo( s, NULL, &hints, &res ) != 0 || !res ) {
		return false;
	}

	memcpy( sadr, res->ai_addr, sizeof( struct sockaddr_in ) );
	((struct sockaddr_in *)sadr)->sin_port = 0;
	freeaddrinfo( res );

	return true;
}

/*
=============
Sys_StringToAdr

idnewt
192.246.40.70
=============
*/
bool Sys_StringToAdr( const char *s, netadr_t *a ) {
	struct sockaddr sadr;

	if ( !Sys_StringToSockaddr( s, &sadr ) ) {
		return false;
	}

	SockadrToNetadr( &sadr, a );
	return true;
}

//=============================================================================

/*
==================
Sys_GetPacket

Never called by the game logic, just the system event queing
==================
*/
int	recvfromCount;

bool Sys_GetPacket( netadr_t *net_from, msg_t *net_message ) {
	int 	ret;
	struct sockaddr from;
	socklen_t	fromlen;
	int		err;

	if( ip_socket == INVALID_SOCKET ) {
		return false;
	}

	fromlen = sizeof(from);
	recvfromCount++;
	ret = recvfrom( ip_socket, reinterpret_cast<char*>(net_message->data), net_message->maxsize, 0, (struct sockaddr *)&from, &fromlen );
	if (ret == SOCKET_ERROR)
	{
		err = socketError;

		if( err == EAGAIN || err == ECONNRESET ) {
			return false;
		}
		Com_Printf( "NET_GetPacket: %s\n", NET_ErrorString() );
		return false;
	}

	memset( ((struct sockaddr_in *)&from)->sin_zero, 0, 8 );

	if ( usingSocks && memcmp( &from, &socksRelayAddr, fromlen ) == 0 ) {
		if ( ret < 10 || net_message->data[0] != 0 || net_message->data[1] != 0 || net_message->data[2] != 0 || net_message->data[3] != 1 ) {
			return false;
		}
		net_from->type = NA_IP;
		net_from->ip[0] = net_message->data[4];
		net_from->ip[1] = net_message->data[5];
		net_from->ip[2] = net_message->data[6];
		net_from->ip[3] = net_message->data[7];
		memcpy( &net_from->port, &net_message->data[8], 2 );
		net_message->readcount = 10;
	}
	else {
		SockadrToNetadr( &from, net_from );
		net_message->readcount = 0;
	}

	if( ret == net_message->maxsize ) {
		Com_Printf( "Oversize packet from %s\n", NET_AdrToString (*net_from) );
		return false;
	}

	net_message->cursize = ret;
	return true;
}

//=============================================================================

static char socksBuf[4096];

/*
==================
Sys_SendPacket
==================
*/
void Sys_SendPacket( int length, const void *data, netadr_t to ) {
	int				ret;
	struct sockaddr	addr;

	if( to.type != NA_BROADCAST && to.type != NA_IP ) {
		Com_Error( ERR_FATAL, "Sys_SendPacket: bad address type" );
		return;
	}

	if( ip_socket == INVALID_SOCKET ) {
		return;
	}

	NetadrToSockadr( &to, &addr );

	if( usingSocks && to.type == NA_IP ) {
		socksBuf[0] = 0;
		socksBuf[1] = 0;
		socksBuf[2] = 0;
		socksBuf[3] = 1;
		memcpy( &socksBuf[4], &((struct sockaddr_in *)&addr)->sin_addr, 4 );
		memcpy( &socksBuf[8], &((struct sockaddr_in *)&addr)->sin_port, 2 );
		memcpy( &socksBuf[10], data, length );
		ret = sendto( ip_socket, socksBuf, length+10, 0, &socksRelayAddr, sizeof(socksRelayAddr) );
	}
	else {
		ret = sendto( ip_socket, reinterpret_cast<const char*>(data), length, 0, &addr, sizeof(struct sockaddr_in) );
	}
	if( ret == SOCKET_ERROR ) {
		int err = socketError;

		// wouldblock is silent
		if( err == EAGAIN ) {
			return;
		}

		// some PPP links do not allow broadcasts and return an error
		if( ( err == EADDRNOTAVAIL ) && ( to.type == NA_BROADCAST ) ) {
			return;
		}

		Com_Printf( "NET_SendPacket: %s\n", NET_ErrorString() );
	}
}


//=============================================================================

/*
==================
Sys_IsLANAddress

LAN clients will have their rate var ignored
==================
*/
bool Sys_IsLANAddress( netadr_t adr ) {
	int		i;

	if( adr.type == NA_LOOPBACK ) {
		return true;
	}

	if( adr.type != NA_IP ) {
		return false;
	}

	// choose which comparison to use based on the class of the address being tested
	// any local adresses of a different class than the address being tested will fail based on the first byte

	if( adr.ip[0] == 127 && adr.ip[1] == 0 && adr.ip[2] == 0 && adr.ip[3] == 1 ) {
		return true;
	}

	// Class A
	if( (adr.ip[0] & 0x80) == 0x00 ) {
		for ( i = 0 ; i < numIP ; i++ ) {
			if( adr.ip[0] == localIP[i][0] ) {
				return true;
			}
		}
		// the RFC1918 class a block will pass the above test
		return false;
	}

	// Class B
	if( (adr.ip[0] & 0xc0) == 0x80 ) {
		for ( i = 0 ; i < numIP ; i++ ) {
			if( adr.ip[0] == localIP[i][0] && adr.ip[1] == localIP[i][1] ) {
				return true;
			}
			// also check against the RFC1918 class b blocks
			if( adr.ip[0] == 172 && localIP[i][0] == 172 && (adr.ip[1] & 0xf0) == 16 && (localIP[i][1] & 0xf0) == 16 ) {
				return true;
			}
		}
		return false;
	}

	// Class C
	for ( i = 0 ; i < numIP ; i++ ) {
		if( adr.ip[0] == localIP[i][0] && adr.ip[1] == localIP[i][1] && adr.ip[2] == localIP[i][2] ) {
			return true;
		}
		// also check against the RFC1918 class c blocks
		if( adr.ip[0] == 192 && localIP[i][0] == 192 && adr.ip[1] == 168 && localIP[i][1] == 168 ) {
			return true;
		}
	}
	return false;
}

/*
==================
Sys_ShowIP
==================
*/
void Sys_ShowIP(void) {
	int i;

	for (i = 0; i < numIP; i++) {
		Com_Printf( "IP: %i.%i.%i.%i\n", localIP[i][0], localIP[i][1], localIP[i][2], localIP[i][3] );
	}
}


//=============================================================================


/*
====================
NET_IPSocket
====================
*/
static SOCKET NET_IPSocket( char *net_interface, int port ) {
	SOCKET				newsocket;
	struct sockaddr_in	address;
	ioctlarg_t			_true = 1;
	int					i = 1;
	int					err;

	if( net_interface ) {
		Com_Printf( "Opening IP socket: %s:%i\n", net_interface, port );
	}
	else {
		Com_Printf( "Opening IP socket: localhost:%i\n", port );
	}

	if( ( newsocket = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) == INVALID_SOCKET ) {
		err = socketError;
		if( err != EAFNOSUPPORT ) {
			Com_Printf( "WARNING: UDP_OpenSocket: socket: %s\n", NET_ErrorString() );
		}
		return INVALID_SOCKET;
	}

	// make it non-blocking
	if( ioctlsocket( newsocket, FIONBIO, &_true ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: UDP_OpenSocket: ioctl FIONBIO: %s\n", NET_ErrorString() );
		closesocket( newsocket );
		return INVALID_SOCKET;
	}

	// make it broadcast capable
	if( setsockopt( newsocket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i) ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: UDP_OpenSocket: setsockopt SO_BROADCAST: %s\n", NET_ErrorString() );
		closesocket( newsocket );
		return INVALID_SOCKET;
	}

	memset( &address, 0, sizeof( address ) );

	if( !net_interface || !net_interface[0] || !Q_stricmp(net_interface, "localhost") ) {
		address.sin_addr.s_addr = INADDR_ANY;
	}
	else {
		Sys_StringToSockaddr( net_interface, (struct sockaddr *)&address );
	}

	if( port == PORT_ANY ) {
		address.sin_port = 0;
	}
	else {
		address.sin_port = htons( (short)port );
	}

	address.sin_family = AF_INET;

	if( bind( newsocket, reinterpret_cast<const sockaddr*>(&address), sizeof(address) ) == SOCKET_ERROR ) {
		Com_Printf( "WARNING: UDP_OpenSocket: bind: %s\n", NET_ErrorString() );
		closesocket( newsocket );
		return INVALID_SOCKET;
	}

	return newsocket;
}


/*
====================
NET_OpenSocks
====================
*/
static void NET_OpenSocks( int port ) {
	struct sockaddr_in	address;
	int					len;
	bool			rfc1929;
	unsigned char		buf[64];
	unsigned short		nport;

	usingSocks = false;

	Com_Printf( "Opening connection to SOCKS server.\n" );

	if ( ( socks_socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP ) ) == INVALID_SOCKET ) {
		Com_Printf( "WARNING: NET_OpenSocks: socket: %s\n", NET_ErrorString() );
		return;
	}

	if ( !Sys_StringToSockaddr( net_socksServer->string, (struct sockaddr *)&address ) ) {
		Com_Printf( "WARNING: NET_OpenSocks: could not resolve %s\n", net_socksServer->string );
		return;
	}
	address.sin_family = AF_INET;
	address.sin_port = htons( (short)net_socksPort->integer );

	if ( connect( socks_socket, (struct sockaddr *)&address, sizeof( address ) ) == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: connect: %s\n", NET_ErrorString() );
		return;
	}

	// send socks authentication handshake
	if ( *net_socksUsername->string || *net_socksPassword->string ) {
		rfc1929 = true;
	}
	else {
		rfc1929 = false;
	}

	buf[0] = 5;		// SOCKS version
	// method count
	if ( rfc1929 ) {
		buf[1] = 2;
		len = 4;
	}
	else {
		buf[1] = 1;
		len = 3;
	}
	buf[2] = 0;		// method #1 - method id #00: no authentication
	if ( rfc1929 ) {
		buf[2] = 2;		// method #2 - method id #02: username/password
	}
	if ( send( socks_socket, reinterpret_cast<const char*>(buf), len, 0 ) == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
		return;
	}

	// get the response
	len = recv( socks_socket, reinterpret_cast<char*>(buf), 64, 0 );
	if ( len == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
		return;
	}
	if ( len != 2 || buf[0] != 5 ) {
		Com_Printf( "NET_OpenSocks: bad response\n" );
		return;
	}
	switch( buf[1] ) {
	case 0:	// no authentication
		break;
	case 2: // username/password authentication
		break;
	default:
		Com_Printf( "NET_OpenSocks: request denied\n" );
		return;
	}

	// do username/password authentication if needed
	if ( buf[1] == 2 ) {
		int		ulen;
		int		plen;

		// build the request
		ulen = strlen( net_socksUsername->string );
		plen = strlen( net_socksPassword->string );

		buf[0] = 1;		// username/password authentication version
		buf[1] = ulen;
		if ( ulen ) {
			memcpy( &buf[2], net_socksUsername->string, ulen );
		}
		buf[2 + ulen] = plen;
		if ( plen ) {
			memcpy( &buf[3 + ulen], net_socksPassword->string, plen );
		}

		// send it
		if ( send( socks_socket, reinterpret_cast<const char*>(buf), 3 + ulen + plen, 0 ) == SOCKET_ERROR ) {
			Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
			return;
		}

		// get the response
		len = recv( socks_socket, reinterpret_cast<char*>(buf), 64, 0 );
		if ( len == SOCKET_ERROR ) {
			Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
			return;
		}
		if ( len != 2 || buf[0] != 1 ) {
			Com_Printf( "NET_OpenSocks: bad response\n" );
			return;
		}
		if ( buf[1] != 0 ) {
			Com_Printf( "NET_OpenSocks: authentication failed\n" );
			return;
		}
	}

	// send the UDP associate request
	buf[0] = 5;		// SOCKS version
	buf[1] = 3;		// command: UDP associate
	buf[2] = 0;		// reserved
	buf[3] = 1;		// address type: IPV4
	memset( &buf[4], 0, 4 );
	nport = htons( (short)port );
	memcpy( &buf[8], &nport, 2 );
	if ( send( socks_socket, reinterpret_cast<const char*>(buf), 10, 0 ) == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: send: %s\n", NET_ErrorString() );
		return;
	}

	// get the response
	len = recv( socks_socket, reinterpret_cast<char*>(buf), 64, 0 );
	if( len == SOCKET_ERROR ) {
		Com_Printf( "NET_OpenSocks: recv: %s\n", NET_ErrorString() );
		return;
	}
	if( len < 2 || buf[0] != 5 ) {
		Com_Printf( "NET_OpenSocks: bad response\n" );
		return;
	}
	// check completion code
	if( buf[1] != 0 ) {
		Com_Printf( "NET_OpenSocks: request denied: %i\n", buf[1] );
		return;
	}
	if( buf[3] != 1 ) {
		Com_Printf( "NET_OpenSocks: relay address is not IPV4: %i\n", buf[3] );
		return;
	}
	memset( &socksRelayAddr, 0, sizeof( socksRelayAddr ) );
	((struct sockaddr_in *)&socksRelayAddr)->sin_family = AF_INET;
	memcpy( &((struct sockaddr_in *)&socksRelayAddr)->sin_addr, &buf[4], 4 );
	memcpy( &((struct sockaddr_in *)&socksRelayAddr)->sin_port, &buf[8], 2 );

	usingSocks = true;
}


/*
=====================
NET_GetLocalAddress
=====================
*/
static void NET_GetLocalAddress( void ) {
	char				hostname[256];
	struct addrinfo		hints;
	struct addrinfo		*res = NULL;
	struct addrinfo		*search;

	if( gethostname( hostname, sizeof( hostname ) ) == SOCKET_ERROR ) {
		return;
	}

	Com_Printf( "Hostname: %s\n", hostname );

	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	if( getaddrinfo( hostname, NULL, &hints, &res ) != 0 ) {
		return;
	}

	numIP = 0;
	for( search = res; search && numIP < MAX_IPS; search = search->ai_next ) {
		byte *p = (byte *)&((struct sockaddr_in *)search->ai_addr)->sin_addr;

		localIP[ numIP ][0] = p[0];
		localIP[ numIP ][1] = p[1];
		localIP[ numIP ][2] = p[2];
		localIP[ numIP ][3] = p[3];
		Com_Printf( "IP: %i.%i.%i.%i\n", p[0], p[1], p[2], p[3] );
		numIP++;
	}

	freeaddrinfo( res );
}

/*
====================
NET_OpenIP
====================
*/
static void NET_OpenIP( void ) {
	cvar_t	*ip;
	int		port;
	int		i;

	ip = Cvar_Get( "net_ip", "localhost", CVAR_LATCH );
	port = Cvar_Get( "net_port", va( "%i", PORT_SERVER ), CVAR_LATCH )->integer;

	// automatically scan for a valid port, so multiple
	// dedicated servers can be started without requiring
	// a different net_port for each one
	for( i = 0 ; i < 10 ; i++ ) {
		ip_socket = NET_IPSocket( ip->string, port + i );
		if ( ip_socket != INVALID_SOCKET ) {
			Cvar_SetValue( "net_port", port + i );
			if ( net_socksEnabled->integer ) {
				NET_OpenSocks( port + i );
			}
			NET_GetLocalAddress();
			return;
		}
	}
	Com_Printf( "WARNING: Couldn't allocate IP port\n");
}


//===================================================================


/*
====================
NET_GetCvars
====================
*/
static bool NET_GetCvars( void ) {
	bool	modified;

	modified = false;

	if( net_noudp && net_noudp->modified ) {
		modified = true;
	}
	net_noudp = Cvar_Get( "net_noudp", "0", CVAR_LATCH | CVAR_ARCHIVE );

	if( net_socksEnabled && net_socksEnabled->modified ) {
		modified = true;
	}
	net_socksEnabled = Cvar_Get( "net_socksEnabled", "0", CVAR_LATCH | CVAR_ARCHIVE );

	if( net_socksServer && net_socksServer->modified ) {
		modified = true;
	}
	net_socksServer = Cvar_Get( "net_socksServer", "", CVAR_LATCH | CVAR_ARCHIVE );

	if( net_socksPort && net_socksPort->modified ) {
		modified = true;
	}
	net_socksPort = Cvar_Get( "net_socksPort", "1080", CVAR_LATCH | CVAR_ARCHIVE );

	if( net_socksUsername && net_socksUsername->modified ) {
		modified = true;
	}
	net_socksUsername = Cvar_Get( "net_socksUsername", "", CVAR_LATCH | CVAR_ARCHIVE );

	if( net_socksPassword && net_socksPassword->modified ) {
		modified = true;
	}
	net_socksPassword = Cvar_Get( "net_socksPassword", "", CVAR_LATCH | CVAR_ARCHIVE );


	return modified;
}


/*
====================
NET_Config
====================
*/
void NET_Config( bool enableNetworking ) {
	bool	modified;
	bool	stop;
	bool	start;

	// get any latched changes to cvars
	modified = NET_GetCvars();

	if( net_noudp->integer ) {
		enableNetworking = false;
	}

	// if enable state is the same and no cvars were modified, we have nothing to do
	if( enableNetworking == networkingEnabled && !modified ) {
		return;
	}

	if( enableNetworking == networkingEnabled ) {
		if( enableNetworking ) {
			stop = true;
			start = true;
		}
		else {
			stop = false;
			start = false;
		}
	}
	else {
		if( enableNetworking ) {
			stop = false;
			start = true;
		}
		else {
			stop = true;
			start = false;
		}
		networkingEnabled = enableNetworking;
	}

	if( stop ) {
		if ( ip_socket != INVALID_SOCKET ) {
			closesocket( ip_socket );
			ip_socket = INVALID_SOCKET;
		}

		if ( socks_socket != INVALID_SOCKET ) {
			closesocket( socks_socket );
			socks_socket = INVALID_SOCKET;
		}
	}

	if( start ) {
		if (! net_noudp->integer ) {
			NET_OpenIP();
		}
	}
}


/*
====================
NET_Init
====================
*/
void NET_Init( void ) {
#ifdef _WIN32
	int		r;

	r = WSAStartup( MAKEWORD( 1, 1 ), &winsockdata );
	if( r ) {
		Com_Printf( "WARNING: Winsock initialization failed, returned %d\n", r );
		return;
	}

	Com_Printf( "Winsock Initialized\n" );
#endif

	winsockInitialized = true;

	// this is really just to get the cvars registered
	NET_GetCvars();

	NET_Config( true );
}


/*
====================
NET_Shutdown
====================
*/
void NET_Shutdown( void ) {
	if ( !winsockInitialized ) {
		return;
	}
	NET_Config( false );
#ifdef _WIN32
	WSACleanup();
#endif
	winsockInitialized = false;
}


/*
====================
NET_Sleep

sleeps msec or until net socket is ready
====================
*/
void NET_Sleep( int msec ) {
	struct timeval	timeout;
	fd_set			fdset;

	if( ip_socket == INVALID_SOCKET || !com_dedicated || !com_dedicated->integer ) {
		return;
	}

	if( msec < 0 ) {
		return;
	}

	FD_ZERO( &fdset );
	FD_SET( ip_socket, &fdset );
	timeout.tv_sec = msec / 1000;
	timeout.tv_usec = ( msec % 1000 ) * 1000;
	select( (int)ip_socket + 1, &fdset, NULL, NULL, &timeout );
}


/*
====================
NET_Restart_f
====================
*/
void NET_Restart( void ) {
	NET_Config( networkingEnabled );
}
