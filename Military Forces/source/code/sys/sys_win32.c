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
#include "sys_local.h"

#include <windows.h>
#include <mmsystem.h>
#include <wincrypt.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <signal.h>

static UINT timerResolution = 0;

/*
==================
Sys_PlatformHomePath
==================
*/
char *Sys_PlatformHomePath( void )
{
	static char homePath[] = "";

	return homePath;
}

/*
================
Sys_Milliseconds
================
*/
int sys_timeBase;
int Sys_Milliseconds (void)
{
	int             sys_curtime;
	static bool initialized = false;

	if (!initialized) {
		sys_timeBase = timeGetTime();
		initialized = true;
	}
	sys_curtime = timeGetTime() - sys_timeBase;

	return sys_curtime;
}

/*
================
Sys_RandomBytes
================
*/
bool Sys_RandomBytes( byte *string, int len )
{
	HCRYPTPROV  prov;

	if( !CryptAcquireContext( &prov, NULL, NULL,
		PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )  {

		return false;
	}

	if( !CryptGenRandom( prov, len, (BYTE *)string ) )  {
		CryptReleaseContext( prov, 0 );
		return false;
	}
	CryptReleaseContext( prov, 0 );
	return true;
}

/*
================
Sys_GetCurrentUser
================
*/
char *Sys_GetCurrentUser( void )
{
	static char s_userName[1024];
	DWORD size = sizeof( s_userName );

	if( !GetUserNameA( s_userName, &size ) )
		strcpy( s_userName, "player" );

	if( !s_userName[0] )
	{
		strcpy( s_userName, "player" );
	}

	return s_userName;
}

#define MEM_THRESHOLD 96*1024*1024

/*
==================
Sys_LowPhysicalMemory
==================
*/
bool Sys_LowPhysicalMemory( void )
{
	MEMORYSTATUSEX stat;
	stat.dwLength = sizeof( stat );
	GlobalMemoryStatusEx (&stat);
	return (stat.ullTotalPhys <= MEM_THRESHOLD) ? true : false;
}

/*
==============
Sys_Basename
==============
*/
const char *Sys_Basename( char *path )
{
	static char base[ MAX_OSPATH ] = { 0 };
	int length;

	length = strlen( path ) - 1;

	// Skip trailing slashes
	while( length > 0 && path[ length ] == '\\' )
		length--;

	while( length > 0 && path[ length - 1 ] != '\\' )
		length--;

	Q_strncpyz( base, &path[ length ], sizeof( base ) );

	length = strlen( base ) - 1;

	// Strip trailing slashes
	while( length > 0 && base[ length ] == '\\' )
    base[ length-- ] = '\0';

	return base;
}

/*
==============
Sys_Dirname
==============
*/
const char *Sys_Dirname( char *path )
{
	static char dir[ MAX_OSPATH ] = { 0 };
	int length;

	Q_strncpyz( dir, path, sizeof( dir ) );
	length = strlen( dir ) - 1;

	while( length > 0 && dir[ length ] != '\\' && dir[ length ] != '/' )
		length--;

	dir[ length ] = '\0';

	return dir;
}

/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir( const char *path )
{
	_mkdir( path );
}

/*
==============
Sys_Cwd
==============
*/
char *Sys_Cwd( void ) {
	static char cwd[MAX_OSPATH];

	_getcwd( cwd, sizeof( cwd ) - 1 );
	cwd[MAX_OSPATH-1] = 0;

	return cwd;
}

/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

#define MAX_FOUND_FILES 0x1000

/*
==============
Sys_ListFilteredFiles
==============
*/
void Sys_ListFilteredFiles( const char *basedir, char *subdirs, char *filter, char **list, int *numfiles )
{
	char		search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char		filename[MAX_OSPATH];
	intptr_t	findhandle;
	struct _finddata_t findinfo;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if ( basedir[0] == '\0' ) {
		return;
	}

	if (strlen(subdirs)) {
		Com_sprintf( search, sizeof(search), "%s\\%s\\*", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof(search), "%s\\*", basedir );
	}

	findhandle = _findfirst (search, &findinfo);
	if (findhandle == -1) {
		return;
	}

	do {
		if (findinfo.attrib & _A_SUBDIR) {
			if (Q_stricmp(findinfo.name, ".") && Q_stricmp(findinfo.name, "..")) {
				if (strlen(subdirs)) {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s\\%s", subdirs, findinfo.name);
				}
				else {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", findinfo.name);
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof(filename), "%s\\%s", subdirs, findinfo.name );
		if (!Com_FilterPath( filter, filename, false ))
			continue;
		list[ *numfiles ] = CopyString( filename );
		(*numfiles)++;
	} while ( _findnext (findhandle, &findinfo) != -1 );

	_findclose (findhandle);
}

/*
==============
strgtr
==============
*/
static bool strgtr(const char *s0, const char *s1)
{
	int l0, l1, i;

	l0 = strlen(s0);
	l1 = strlen(s1);

	if (l1<l0) {
		l0 = l1;
	}

	for(i=0;i<l0;i++) {
		if (s1[i] > s0[i]) {
			return true;
		}
		if (s1[i] < s0[i]) {
			return false;
		}
	}
	return false;
}

/*
==============
Sys_ListFiles
==============
*/
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, bool wantsubs )
{
	char		search[MAX_OSPATH];
	int			nfiles;
	char		**listCopy;
	char		*list[MAX_FOUND_FILES];
	struct _finddata_t findinfo;
	intptr_t		findhandle;
	int			flag;
	int			i;
	int			extLen;
	char		emptyString[] = "";

	if (filter) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, emptyString, filter, list, &nfiles );

		list[ nfiles ] = 0;
		*numfiles = nfiles;

		if (!nfiles)
			return NULL;

		listCopy = reinterpret_cast<char**>(Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) ));
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( directory[0] == '\0' ) {
		*numfiles = 0;
		return NULL;
	}

	if ( !extension) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	extLen = strlen( extension );

	Com_sprintf( search, sizeof(search), "%s\\*%s", directory, extension );

	// search
	nfiles = 0;

	findhandle = _findfirst (search, &findinfo);
	if (findhandle == -1) {
		*numfiles = 0;
		return NULL;
	}

	do {
		if ( (!wantsubs && flag ^ ( findinfo.attrib & _A_SUBDIR )) || (wantsubs && findinfo.attrib & _A_SUBDIR) ) {
			if (*extension) {
				if ( strlen( findinfo.name ) < extLen ||
					Q_stricmp(
						findinfo.name + strlen( findinfo.name ) - extLen,
						extension ) ) {
					continue; // didn't match
				}
			}
			if ( nfiles == MAX_FOUND_FILES - 1 ) {
				break;
			}
			list[ nfiles ] = CopyString( findinfo.name );
			nfiles++;
		}
	} while ( _findnext (findhandle, &findinfo) != -1 );

	list[ nfiles ] = 0;

	_findclose (findhandle);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = reinterpret_cast<char**>(Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) ));
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	do {
		flag = 0;
		for(i=1; i<nfiles; i++) {
			if (strgtr(listCopy[i-1], listCopy[i])) {
				char *temp = listCopy[i];
				listCopy[i] = listCopy[i-1];
				listCopy[i-1] = temp;
				flag = 1;
			}
		}
	} while(flag);

	return listCopy;
}

/*
==============
Sys_FreeFileList
==============
*/
void Sys_FreeFileList( char **list )
{
	int i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}

/*
==============
Sys_ErrorDialog

Display an error message
==============
*/
void Sys_ErrorDialog( const char *error )
{
	Sys_Print( va( "%s\n", error ) );

	MessageBoxA( NULL, error, "Error", MB_ICONERROR|MB_OK );
}

/*
==============
Sys_PlatformInit

Windows specific initialisation
==============
*/
void Sys_PlatformInit( void )
{
	TIMECAPS ptc;

	if(timeGetDevCaps(&ptc, sizeof(ptc)) == MMSYSERR_NOERROR)
	{
		timerResolution = ptc.wPeriodMin;

		timeBeginPeriod(timerResolution);
	}
	else
		timerResolution = 0;
}

/*
==============
Sys_PlatformExit

Windows specific initialisation
==============
*/
void Sys_PlatformExit( void )
{
	if(timerResolution)
		timeEndPeriod(timerResolution);
}

/*
=================
Sys_SetMaxFileLimit
=================
*/
bool Sys_SetMaxFileLimit( void )
{
	return true;
}
