#ifndef ZONE_H
#define ZONE_H

class Zone
{
public:
	static void *tagMalloc( int size, int tag, const std::source_location& at = std::source_location::current() );
	static void *malloc( int size, const std::source_location& at = std::source_location::current() );
	static void *smallMalloc( int size, const std::source_location& at = std::source_location::current() );	// NOT 0 filled, only for small allocations
	static void free( void *ptr );
	// room for count objects of one type, zero filled (make) or not (makeSmall)
	template<typename T> static T *make( int count = 1, const std::source_location& at = std::source_location::current() ) { return static_cast<T *>( malloc( count * static_cast<int>( sizeof( T ) ), at ) ); }
	template<typename T> static T *makeSmall( int count = 1, const std::source_location& at = std::source_location::current() ) { return static_cast<T *>( smallMalloc( count * static_cast<int>( sizeof( T ) ), at ) ); }
	static void freeTags( int tag );
	static int availableMemory( );
	static void logHeap( );
};

#endif
