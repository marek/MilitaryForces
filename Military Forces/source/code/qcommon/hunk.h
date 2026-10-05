#ifndef HUNK_H
#define HUNK_H

class Hunk
{
public:
	static void *alloc( int size, ha_pref preference, const std::source_location& at = std::source_location::current() );
	// room for count objects of one type
	template<typename T> static T *make( int count, ha_pref preference, const std::source_location& at = std::source_location::current() ) { return static_cast<T *>( alloc( count * static_cast<int>( sizeof( T ) ), preference, at ) ); }
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

// temporary hunk memory, given back when the object goes out of scope. Temporary blocks are a stack: they must
// be released in the reverse order of their creation, which scopes do by themselves.
class TempMemory
{
public:
	explicit TempMemory( int size ) : data_( Hunk::allocateTempMemory( size ) ) {}
	~TempMemory() { Hunk::freeTempMemory( data_ ); }
	TempMemory( const TempMemory& ) = delete;
	TempMemory& operator=( const TempMemory& ) = delete;

	void *data() const { return data_; }
	template<typename T> T *as() const { return static_cast<T *>( data_ ); }

private:
	void	*data_;
};

#endif
