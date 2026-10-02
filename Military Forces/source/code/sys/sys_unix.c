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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <libgen.h>
#include <signal.h>

bool stdinIsATTY;

/*
==================
Sys_PlatformHomePath
==================
*/
char *Sys_PlatformHomePath(void)
{
	static char homePath[ MAX_OSPATH ] = { 0 };
	char *p;

	if( !*homePath )
	{
		if( ( p = getenv( "HOME" ) ) != NULL )
		{
#ifdef __APPLE__
			Com_sprintf( homePath, sizeof( homePath ), "%s%cLibrary%cApplication Support%cMilitaryForces",
					p, PATH_SEP, PATH_SEP, PATH_SEP );
#else
			Com_sprintf( homePath, sizeof( homePath ), "%s%c.militaryforces", p, PATH_SEP );
#endif
		}
	}

	return homePath;
}

/*
================
Sys_Milliseconds
================
*/
/* base time in seconds, that's our origin
   timeval:tv_sec is an int:
   assuming this wraps every 0x7fffffff - ~68 years since the Epoch (1970) - we're safe till 2038 */
unsigned long sys_timeBase = 0;
/* current time in ms, using sys_timeBase as origin
   NOTE: sys_timeBase*1000 + curtime -> ms since the Epoch
     0x7fffffff ms - ~24 days
   although timeval:tv_usec is an int, I'm not sure wether it is actually used as an unsigned int
     (which would affect the wrap period) */
int curtime;
int Sys_Milliseconds (void)
{
	struct timeval tp;

	gettimeofday(&tp, NULL);

	if (!sys_timeBase)
	{
		sys_timeBase = tp.tv_sec;
		return tp.tv_usec/1000;
	}

	curtime = (tp.tv_sec - sys_timeBase)*1000 + tp.tv_usec/1000;

	return curtime;
}

/*
==================
Sys_RandomBytes
==================
*/
bool Sys_RandomBytes( byte *string, int len )
{
	FILE *fp;

	fp = fopen( "/dev/urandom", "r" );
	if( !fp )
		return false;

	setvbuf( fp, NULL, _IONBF, 0 ); // don't buffer reads from /dev/urandom

	if( fread( string, sizeof( byte ), len, fp ) != len )
	{
		fclose( fp );
		return false;
	}

	fclose( fp );
	return true;
}

/*
==================
Sys_GetCurrentUser
==================
*/
char *Sys_GetCurrentUser( void )
{
	static char username[] = "player";
	struct passwd *p;

	if ( (p = getpwuid( getuid() )) == NULL ) {
		return username;
	}
	return p->pw_name;
}

/*
==================
Sys_LowPhysicalMemory
==================
*/
bool Sys_LowPhysicalMemory( void )
{
	return false;
}

/*
==================
Sys_Basename
==================
*/
const char *Sys_Basename( char *path )
{
	return basename( path );
}

/*
==================
Sys_Dirname
==================
*/
const char *Sys_Dirname( char *path )
{
	return dirname( path );
}

#ifdef __APPLE__
/*
=================
Sys_StripAppBundle

Discovers if passed dir is suffixed with the directory structure of a Mac OS X
.app bundle. If it is, the .app directory structure is stripped off the end and
the result is returned. If not, dir is returned untouched.
=================
*/
char *Sys_StripAppBundle( char *dir )
{
	static char cwd[MAX_OSPATH];

	Q_strncpyz(cwd, dir, sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "MacOS"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(strcmp(Sys_Basename(cwd), "Contents"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	if(!strstr(Sys_Basename(cwd), ".app"))
		return dir;
	Q_strncpyz(cwd, Sys_Dirname(cwd), sizeof(cwd));
	return cwd;
}
#endif

/*
==================
Sys_Mkdir
==================
*/
void Sys_Mkdir( const char *path )
{
	mkdir( path, 0750 );
}

/*
==================
Sys_Cwd
==================
*/
char *Sys_Cwd( void )
{
	static char cwd[MAX_OSPATH];

	char *result = getcwd( cwd, sizeof( cwd ) - 1 );
	if( result != cwd )
		return NULL;

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
==================
Sys_ListFilteredFiles
==================
*/
void Sys_ListFilteredFiles( const char *basedir, char *subdirs, char *filter, char **list, int *numfiles )
{
	char          search[MAX_OSPATH], newsubdirs[MAX_OSPATH];
	char          filename[MAX_OSPATH];
	DIR           *fdir;
	struct dirent *d;
	struct stat   st;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if ( basedir[0] == '\0' ) {
		return;
	}

	if (strlen(subdirs)) {
		Com_sprintf( search, sizeof(search), "%s/%s", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof(search), "%s", basedir );
	}

	if ((fdir = opendir(search)) == NULL) {
		return;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(filename, sizeof(filename), "%s/%s", search, d->d_name);
		if (stat(filename, &st) == -1)
			continue;

		if (st.st_mode & S_IFDIR) {
			if (Q_stricmp(d->d_name, ".") && Q_stricmp(d->d_name, "..")) {
				if (strlen(subdirs)) {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s/%s", subdirs, d->d_name);
				}
				else {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", d->d_name);
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof(filename), "%s/%s", subdirs, d->d_name );
		if (!Com_FilterPath( filter, filename, false ))
			continue;
		list[ *numfiles ] = CopyString( filename );
		(*numfiles)++;
	}

	closedir(fdir);
}

/*
==================
Sys_ListFiles
==================
*/
char **Sys_ListFiles( const char *directory, const char *extension, char *filter, int *numfiles, bool wantsubs )
{
	struct dirent *d;
	DIR           *fdir;
	bool      dironly = wantsubs;
	char          search[MAX_OSPATH];
	int           nfiles;
	char          **listCopy;
	char          *list[MAX_FOUND_FILES];
	int           i;
	struct stat   st;
	char          emptyString[] = "";

	int           extLen;

	if (filter) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, emptyString, filter, list, &nfiles );

		list[ nfiles ] = NULL;
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

	if ( !extension)
		extension = "";

	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		dironly = true;
	}

	extLen = strlen( extension );

	// search
	nfiles = 0;

	if ((fdir = opendir(directory)) == NULL) {
		*numfiles = 0;
		return NULL;
	}

	while ((d = readdir(fdir)) != NULL) {
		Com_sprintf(search, sizeof(search), "%s/%s", directory, d->d_name);
		if (stat(search, &st) == -1)
			continue;
		if ((dironly && !(st.st_mode & S_IFDIR)) ||
			(!dironly && (st.st_mode & S_IFDIR)))
			continue;

		if (*extension) {
			if ( strlen( d->d_name ) < extLen ||
				Q_stricmp(
					d->d_name + strlen( d->d_name ) - extLen,
					extension ) ) {
				continue; // didn't match
			}
		}

		if ( nfiles == MAX_FOUND_FILES - 1 )
			break;
		list[ nfiles ] = CopyString( d->d_name );
		nfiles++;
	}

	list[ nfiles ] = NULL;

	closedir(fdir);

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

	return listCopy;
}

/*
==================
Sys_FreeFileList
==================
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
=================
Sys_DllExtension

Check if filename should be allowed to be loaded as a DLL.
=================
*/
bool Sys_DllExtension( const char *name ) {
	const char *p;
	char c = 0;

	if ( COM_CompareExtension( name, DLL_EXT ) ) {
		return true;
	}

#ifdef __APPLE__
	// Allow system frameworks without dylib extensions
	// i.e., /System/Library/Frameworks/OpenAL.framework/OpenAL
	if ( strncmp( name, "/System/Library/Frameworks/", 27 ) == 0 ) {
		return true;
	}
#endif

	// Check for format of filename.so.1.2.3
	p = strstr( name, DLL_EXT "." );

	if ( p ) {
		p += strlen( DLL_EXT );

		// Check if .so is only followed for periods and numbers.
		while ( *p ) {
			c = *p;

			if ( !isdigit( c ) && c != '.' ) {
				return false;
			}

			p++;
		}

		// Don't allow filename to end in a period. file.so., file.so.0., etc
		if ( c != '.' ) {
			return true;
		}
	}

	return false;
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
}

/*
=================
Sys_SetMaxFileLimit
=================
*/
bool Sys_SetMaxFileLimit( void )
{
#ifdef RLIMIT_NOFILE
	struct rlimit limit;

	// Get the current open file limit
	if( getrlimit( RLIMIT_NOFILE, &limit ) == 0 )
	{
		// Set the file limit to the maximum
		limit.rlim_cur = limit.rlim_max;
		if( setrlimit( RLIMIT_NOFILE, &limit ) == 0 )
			return true;

#ifdef OPEN_MAX
		limit.rlim_cur = OPEN_MAX;
		if( setrlimit( RLIMIT_NOFILE, &limit ) == 0 )
			return true;
#endif
	}
#endif

	return false;
}

/*
==============
Sys_PlatformInit

Unix specific initialisation
==============
*/
void Sys_PlatformInit( void )
{
	const char* term = getenv( "TERM" );

	signal( SIGHUP, Sys_SigHandler );
	signal( SIGQUIT, Sys_SigHandler );
	signal( SIGTRAP, Sys_SigHandler );
	signal( SIGABRT, Sys_SigHandler );
	signal( SIGBUS, Sys_SigHandler );

	Sys_SetMaxFileLimit( );

	stdinIsATTY = isatty( STDIN_FILENO ) &&
		!( term && ( !strcmp( term, "raw" ) || !strcmp( term, "dumb" ) ) );
}

/*
==============
Sys_PlatformExit

Unix specific deinitialisation
==============
*/
void Sys_PlatformExit( void )
{
}
