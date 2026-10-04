#include "ui_memtools.h"
#include "../game/q_shared.h"
#include <memory>
#include <cstring>








void *BotImport_GetMemory(int size);
void *BotImport_HunkAlloc( int size );
void BotImport_FreeMemory(void *ptr);



void*
UI_Memtools::getMemory( unsigned long size )
{
	void *ptr;
	intptr_t *memid;

	ptr = BotImport_GetMemory(size + sizeof(intptr_t));
	if (!ptr) 
		return nullptr;
	memid = static_cast<intptr_t *>(ptr);
	*memid = MEM_ID;
	return reinterpret_cast<intptr_t *>(static_cast<char *>(ptr) + sizeof(intptr_t));
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
	intptr_t *memid;

	ptr = BotImport_HunkAlloc(size + sizeof(intptr_t));
	if (!ptr) return nullptr;
	memid = static_cast<intptr_t *>(ptr);
	*memid = HUNK_ID;
	return reinterpret_cast<intptr_t *>(static_cast<char *>(ptr) + sizeof(intptr_t));
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
	intptr_t *memid;

	memid = reinterpret_cast<intptr_t *>(static_cast<char *>(ptr) - sizeof(intptr_t));

	if (*memid == MEM_ID)
		BotImport_FreeMemory(memid);
}

int 
UI_Memtools::availableMemory()
{
	return Zone::availableMemory();
}

