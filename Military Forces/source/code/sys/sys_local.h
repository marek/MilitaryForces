/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"

#include <SDL_version.h>

#define MINSDL_MAJOR 2
#define MINSDL_MINOR 0
#define MINSDL_PATCH 5

void CON_Shutdown( void );
void CON_Init( void );
char *CON_Input( void );
void CON_Print( const char *message );

unsigned int CON_LogSize( void );
unsigned int CON_LogWrite( const char *in );
unsigned int CON_LogRead( char *out, unsigned int outSize );

char *Sys_BinaryPath( void );
const char *Sys_Basename( char *path );
const char *Sys_Dirname( char *path );

#ifdef __APPLE__
char *Sys_StripAppBundle( char *pwd );
#endif

void Sys_PlatformInit( void );
void Sys_PlatformExit( void );
void Sys_SigHandler( int signal );
void Sys_ErrorDialog( const char *error );
void Sys_AnsiColorPrint( const char *msg );

bool Sys_SetMaxFileLimit( void );

void Sys_QueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr );
bool Sys_GetPacket( netadr_t *net_from, msg_t *net_message );
bool Sys_RandomBytes( byte *string, int len );
