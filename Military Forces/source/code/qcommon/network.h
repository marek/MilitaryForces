#ifndef NETWORK_H
#define NETWORK_H

class Network
{
public:
	static void init( );
	static void shutdown( );
	static void restart_f( );
	static void config( bool enableNetworking );
	static void flushPacketQueue();
	static void sendPacket (netsrc_t sock, int length, const void *data, netadr_t to);
	static void QDECL outOfBandPrint( netsrc_t net_socket, netadr_t adr, const char *format, ...) Q_PRINTF_FUNC(3, 4);
	static void QDECL outOfBandData( netsrc_t sock, netadr_t adr, byte *format, int len );
	static bool compareAdr (netadr_t a, netadr_t b);
	static bool compareBaseAdrMask(netadr_t a, netadr_t b, int netmask);
	static bool compareBaseAdr (netadr_t a, netadr_t b);
	static bool isLocalAddress (netadr_t adr);
	static const char *adrToString (netadr_t a);
	static const char *adrToStringwPort (netadr_t a);
	static int stringToAdr ( const char *s, netadr_t *a, netadrtype_t family);
	static bool getLoopPacket (netsrc_t sock, netadr_t *net_from, Message *net_message);
	static void joinMulticast6();
	static void leaveMulticast6();
	static void sleep(int msec);
};

#endif
