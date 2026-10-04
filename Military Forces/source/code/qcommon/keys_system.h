#ifndef KEYS_SYSTEM_H
#define KEYS_SYSTEM_H

class Keys
{
public:
	static void writeBindings( fileHandle_t f );
	static void setBinding( int keynum, const char *binding );
	static char *getBinding( int keynum );
	static bool isDown( int keynum );
	static bool getOverstrikeMode( );
	static void setOverstrikeMode( bool state );
	static void clearStates( );
	static int getKey(const char *binding);
};

#endif
