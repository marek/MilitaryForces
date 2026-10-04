#ifndef HUNK_H
#define HUNK_H

class Hunk
{
public:
	static void *alloc( int size, ha_pref preference, const std::source_location& at = std::source_location::current() );
	static void clear( );
	static void clearToMark( );
	static void setMark( );
	static bool checkMark( );
	static void clearTempMemory( );
	static void *allocateTempMemory( int size );
	static void freeTempMemory( void *buf );
	static int memoryRemaining( );
	static void log( );
	static void smallLog( );
};

#endif
