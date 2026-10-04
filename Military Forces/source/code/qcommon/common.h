#ifndef COMMON_H
#define COMMON_H

enum sysEventType_t {
	// SE_NONE must be zero
	SE_NONE = 0,		// evTime is still valid
	SE_KEY,			// evValue is a key code, evValue2 is the down flag
	SE_CHAR,		// evValue is an ascii char
	SE_MOUSE,		// evValue and evValue2 are relative signed x / y moves
	SE_JOYSTICK_AXIS,	// evValue is an axis number and evValue2 is the current state (-127 to 127)
	SE_CONSOLE		// evPtr is a char*
};

class Message;
struct netadr_t;
struct sysEvent_t;

class Common
{
public:
	static void queueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
	static int eventLoop( );
	static sysEvent_t getSystemEvent( );
	static void beginRedirect (char *buffer, int buffersize, void (*flush)(char *));
	static void endRedirect( );
	static void QDECL printf( const char *fmt, ... ) Q_PRINTF_FUNC(1, 2);
	static void QDECL dPrintf( const char *fmt, ... ) Q_PRINTF_FUNC(1, 2);
	static void QDECL error( int code, const char *fmt, ... ) Q_NO_RETURN Q_PRINTF_FUNC(2, 3);
	static void quit_f( ) Q_NO_RETURN;
	static void gameRestart(int checksumFeed, bool disconnect);
	static int milliseconds( );
	static int filter(char *filter, char *name, int casesensitive);
	static int filterPath(char *filter, char *name, int casesensitive);
	static int realTime(qtime_t *qtime);
	static bool safeMode( );
	static void runAndTimeServerPacket(netadr_t *evFrom, Message *buf);
	static bool isVoipTarget(uint8_t *voipTargets, int voipTargetsSize, int clientNum);
	static void startupVariable( const char *match );
	static bool playerNameToFieldString( char *str, int length, const char *name );
	static bool fieldStringToPlayerName( char *name, int length, const char *rawname );
	static int QDECL strCompare( const void *a, const void *b );
	static void touchMemory( );
	static void init( char *commandLine );
	static void frame( );
	static void shutdown( );
};

#endif
