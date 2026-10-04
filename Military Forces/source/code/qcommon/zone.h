#ifndef ZONE_H
#define ZONE_H

class Zone
{
public:
	static void *tagMalloc( int size, int tag, const std::source_location& at = std::source_location::current() );
	static void *malloc( int size, const std::source_location& at = std::source_location::current() );
	static void *smallMalloc( int size, const std::source_location& at = std::source_location::current() );	// NOT 0 filled, only for small allocations
	static void free( void *ptr );
	static void freeTags( int tag );
	static int availableMemory( );
	static void logHeap( );
};

#endif
