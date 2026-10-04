/*
 * $Id: g_mem.c,v 1.2 2005-11-20 11:21:38 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// g_mem.c
//


#include "g_local.h"


constexpr int POOLSIZE = (256 * 1024);

static char		memoryPool[POOLSIZE];
static int		allocPoint;

void *Game::alloc( int size ) {
	char	*p;

	if ( g_debugAlloc.integer ) {
		Common::printf( "Game::alloc of %i bytes (%i left)\n", size, POOLSIZE - allocPoint - ( ( size + 31 ) & ~31 ) );
	}

	if ( allocPoint + size > POOLSIZE ) {
		Common::error( ERR_DROP, "Game::alloc: failed on allocation of %u bytes\n", size );
		return nullptr;
	}

	p = &memoryPool[allocPoint];

	allocPoint += ( size + 31 ) & ~31;

	return p;
}

void Game::initMemory( ) {
	allocPoint = 0;
}

void Svcmd_GameMem_f( ) {
	Common::printf( "Game memory status: %i out of %i bytes allocated\n", allocPoint, POOLSIZE );
}
