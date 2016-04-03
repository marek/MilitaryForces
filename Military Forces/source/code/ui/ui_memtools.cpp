#include "ui_memtools.h"
#include <memory>








void *BotImport_GetMemory(int size);
void *BotImport_HunkAlloc( int size );
void BotImport_FreeMemory(void *ptr);
int Z_AvailableMemory( void );



void*
UI_Memtools::getMemory( unsigned long size )
{
	void *ptr;
	unsigned long int *memid;

	ptr = BotImport_GetMemory(size + sizeof(unsigned long int));
	if (!ptr) 
		return 0;
	memid = (unsigned long int *) ptr;
	*memid = MEM_ID;
	return (unsigned long int *) ((char *) ptr + sizeof(unsigned long int));
} 

void*
UI_Memtools::getClearedMemory( unsigned long size )
{
	void *ptr;
	ptr = getMemory(size);
	memset(ptr, 0, size);
	return ptr;
}

void*
UI_Memtools::getHunkMemory( unsigned long size )
{
	void *ptr;
	unsigned long int *memid;

	ptr = BotImport_HunkAlloc(size + sizeof(unsigned long int));
	if (!ptr) return NULL;
	memid = (unsigned long int *) ptr;
	*memid = HUNK_ID;
	return (unsigned long int *) ((char *) ptr + sizeof(unsigned long int));
} 

void*
UI_Memtools::getClearedHunkMemory( unsigned long size )
{
	void *ptr;
	ptr = getHunkMemory(size);
	memset(ptr, 0, size);
	return ptr;
}

void 
UI_Memtools::freeMemory( void *ptr )
{
	unsigned long int *memid;

	memid = (unsigned long int *) ((char *) ptr - sizeof(unsigned long int));

	if (*memid == MEM_ID)
		BotImport_FreeMemory(memid);
}

int 
UI_Memtools::availableMemory()
{
	return Z_AvailableMemory();
}

