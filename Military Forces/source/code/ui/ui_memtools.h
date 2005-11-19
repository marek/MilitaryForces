#ifndef __UI_MEMTOOLS_H__
#define __UI_MEMTOOLS_H__


// pure utils class
struct UI_Memtools
{
	//allocate a memory block of the given size
	static void*	getMemory( unsigned long size );
	//allocate a memory block of the given size and clear it
	static void*	getClearedMemory( unsigned long size );

	//allocate a memory block of the given size
	static void*	getHunkMemory( unsigned long size );
	//allocate a memory block of the given size and clear it
	static void*	getClearedHunkMemory( unsigned long size );

	//free the given memory block
	static void		freeMemory( void *ptr );
	//returns the amount available memory
	static int		availableMemory();

private:
	UI_Memtools();
	~UI_Memtools();

	static const unsigned long MEM_ID =	0x12345678l;
	static const unsigned long HUNK_ID = 0x87654321l;

};



#endif // __UI_MEMTOOLS_H__

