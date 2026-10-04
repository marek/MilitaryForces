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

#include <csignal>
#include <cstdlib>
#include <climits>
#include <sys/types.h>
#include <cstdarg>
#include <cstdio>
#include <sys/stat.h>
#include <cstring>
#include <cctype>
#include <cerrno>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#ifndef DEDICATED
#ifdef USE_INTERNAL_SDL_HEADERS
#	include "SDL.h"
#	include "SDL_cpuinfo.h"
#else
#	include <SDL.h>
#	include <SDL_cpuinfo.h>
#endif
#endif

#include "sys_local.h"
#include "sys_loadlib.h"

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"

static char binaryPath[ MAX_OSPATH ] = { 0 };
static char installPath[ MAX_OSPATH ] = { 0 };

/*
=================
Sys_SetBinaryPath
=================
*/
void Sys_SetBinaryPath(const char *path)
{
	Q_strncpyz(binaryPath, path, sizeof(binaryPath));
}

/*
=================
Sys_BinaryPath
=================
*/
char *Sys_BinaryPath()
{
	return binaryPath;
}

/*
=================
System::setDefaultInstallPath
=================
*/
void System::setDefaultInstallPath(const char *path)
{
	Q_strncpyz(installPath, path, sizeof(installPath));
}

/*
=================
System::defaultInstallPath
=================
*/
char *System::defaultInstallPath()
{
	if (*installPath)
		return installPath;
	else
		return System::cwd();
}

/*
=================
System::defaultAppPath
=================
*/
char *System::defaultAppPath()
{
	return Sys_BinaryPath();
}

/*
=================
Sys_In_Restart_f

Restart the input subsystem
=================
*/
void Sys_In_Restart_f( )
{
#ifndef DEDICATED
	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		Common::printf( "in_restart: Cannot restart input while video is shutdown\n" );
		return;
	}
#endif

	IN_Restart( );
}

/*
=================
System::consoleInput

Handle new console input
=================
*/
char *System::consoleInput()
{
	return CON_Input( );
}

/*
==================
System::getClipboardData
==================
*/
char *System::getClipboardData()
{
#ifdef DEDICATED
	return NULL;
#else
	char *data = nullptr;
	char *cliptext;

	if ( ( cliptext = SDL_GetClipboardText() ) != nullptr ) {
		if ( cliptext[0] != '\0' ) {
			size_t bufsize = strlen( cliptext ) + 1;

			data = static_cast<char *>(Z_Malloc( bufsize ));
			Q_strncpyz( data, cliptext, bufsize );

			// find first listed char and set to '\0'
			strtok( data, "\n\r\b" );
		}
		SDL_free( cliptext );
	}
	return data;
#endif
}

#ifdef DEDICATED
#	define PID_FILENAME PRODUCT_NAME "_server.pid"
#else
#	define PID_FILENAME PRODUCT_NAME ".pid"
#endif

/*
=================
Sys_PIDFileName
=================
*/
static char *Sys_PIDFileName( const char *gamedir )
{
	const char *homeStatePath = CvarSystem::variableString( "fs_homestatepath" );

	if( *homeStatePath != '\0' )
		return va( "%s/%s/%s", homeStatePath, gamedir, PID_FILENAME );

	return nullptr;
}

/*
=================
System::removePIDFile
=================
*/
void System::removePIDFile( const char *gamedir )
{
	char *pidFile = Sys_PIDFileName( gamedir );

	if( pidFile != nullptr )
		remove( pidFile );
}

/*
=================
Sys_WritePIDFile

Return true if there is an existing stale PID file
=================
*/
static bool Sys_WritePIDFile( const char *gamedir )
{
	char      *pidFile = Sys_PIDFileName( gamedir );
	FILE      *f;
	bool  stale = false;

	if( pidFile == nullptr )
		return false;

	// First, check if the pid file is already there
	if( ( f = fopen( pidFile, "r" ) ) != nullptr )
	{
		char  pidBuffer[ 64 ] = { 0 };
		int   pid;

		pid = fread( pidBuffer, sizeof( char ), sizeof( pidBuffer ) - 1, f );
		fclose( f );

		if(pid > 0)
		{
			pid = atoi( pidBuffer );
			if( !Sys_PIDIsRunning( pid ) )
				stale = true;
		}
		else
			stale = true;
	}

	if( FileSystem::createPath( pidFile ) ) {
		return false;
	}

	if( ( f = fopen( pidFile, "w" ) ) != nullptr )
	{
		fprintf( f, "%d", Sys_PID( ) );
		fclose( f );
	}
	else
		Common::printf( S_COLOR_YELLOW "Couldn't write %s.\n", pidFile );

	return stale;
}

/*
=================
System::initPIDFile
=================
*/
void System::initPIDFile( const char *gamedir ) {
	if( Sys_WritePIDFile( gamedir ) ) {
#ifndef DEDICATED
		char message[1024];
		char modName[MAX_OSPATH];

		FileSystem::getModDescription( gamedir, modName, sizeof ( modName ) );
		Q_CleanStr( modName );

		Com_sprintf( message, sizeof (message), "The last time %s ran, "
			"it didn't exit properly. This may be due to inappropriate video "
			"settings. Would you like to start with \"safe\" video settings?", modName );

		if( System::dialog( DT_YES_NO, message, "Abnormal Exit" ) == DR_YES ) {
			CvarSystem::set( "com_abnormalExit", "1" );
		}
#endif
	}
}

/*
=================
System::openFolderInFileManager
=================
*/
bool System::openFolderInFileManager( const char *path, bool create )
{
	if( create )
	{
		if( FileSystem::createPath( path ) )
			return false;
	}

	return Sys_OpenFolderInPlatformFileManager( path );
}

/*
=================
Sys_Exit

Single exit point (regular exit or in case of error)
=================
*/
static Q_NO_RETURN void Sys_Exit( int exitCode )
{
	CON_Shutdown( );

#ifndef DEDICATED
	SDL_Quit( );
#endif

	if( exitCode < 2 && com_fullyInitialized )
	{
		// Normal exit
		System::removePIDFile( FileSystem::getCurrentGameDir() );
	}

	Network::shutdown( );

	Sys_PlatformExit( );

	exit( exitCode );
}

/*
=================
System::quit
=================
*/
void System::quit( )
{
	Sys_Exit( 0 );
}

/*
=================
System::getProcessorFeatures
=================
*/
cpuFeatures_t System::getProcessorFeatures( )
{
	cpuFeatures_t features = static_cast<cpuFeatures_t>(0);

#ifndef DEDICATED
	if( SDL_HasRDTSC( ) )      features |= CF_RDTSC;
	if( SDL_Has3DNow( ) )      features |= CF_3DNOW;
	if( SDL_HasMMX( ) )        features |= CF_MMX;
	if( SDL_HasSSE( ) )        features |= CF_SSE;
	if( SDL_HasSSE2( ) )       features |= CF_SSE2;
	if( SDL_HasAltiVec( ) )    features |= CF_ALTIVEC;
#endif

	return features;
}

/*
=================
System::init
=================
*/
void System::init()
{
	CommandSystem::addCommand( "in_restart", Sys_In_Restart_f );
	CvarSystem::set( "arch", OS_STRING " " ARCH_STRING );
	CvarSystem::set( "username", System::getCurrentUser( ) );
}

/*
=================
Sys_AnsiColorPrint

Transform Q3 colour codes to ANSI escape sequences
=================
*/
void Sys_AnsiColorPrint( const char *msg )
{
	static char buffer[ MAXPRINTMSG ];
	int         length = 0;
	static int  q3ToAnsi[ 8 ] =
	{
		7, // COLOR_BLACK
		31, // COLOR_RED
		32, // COLOR_GREEN
		33, // COLOR_YELLOW
		34, // COLOR_BLUE
		36, // COLOR_CYAN
		35, // COLOR_MAGENTA
		0   // COLOR_WHITE
	};

	while( *msg )
	{
		if( Q_IsColorString( msg ) || *msg == '\n' )
		{
			// First empty the buffer
			if( length > 0 )
			{
				buffer[ length ] = '\0';
				fputs( buffer, stderr );
				length = 0;
			}

			if( *msg == '\n' )
			{
				// Issue a reset and then the newline
				fputs( "\033[0m\n", stderr );
				msg++;
			}
			else
			{
				// Print the color code (reset first to clear potential inverse (black))
				Com_sprintf( buffer, sizeof( buffer ), "\033[0m\033[%dm",
						q3ToAnsi[ ColorIndex( *( msg + 1 ) ) ] );
				fputs( buffer, stderr );
				msg += 2;
			}
		}
		else
		{
			if( length >= MAXPRINTMSG - 1 )
				break;

			buffer[ length ] = *msg;
			length++;
			msg++;
		}
	}

	// Empty anything still left in the buffer
	if( length > 0 )
	{
		buffer[ length ] = '\0';
		fputs( buffer, stderr );
	}
}

/*
=================
System::print
=================
*/
void System::print( const char *msg )
{
	CON_LogWrite( msg );
	CON_Print( msg );
}

/*
=================
System::error
=================
*/
void System::error( const char *error, ... )
{
	va_list argptr;
	char    string[1024];

	va_start (argptr,error);
	Q_vsnprintf (string, sizeof(string), error, argptr);
	va_end (argptr);

	Sys_ErrorDialog( string );

	Sys_Exit( 3 );
}

#if 0
/*
=================
Sys_Warn
=================
*/
static Q_PRINTF_FUNC(1, 2) void Sys_Warn( char *warning, ... )
{
	va_list argptr;
	char    string[1024];

	va_start (argptr,warning);
	Q_vsnprintf (string, sizeof(string), warning, argptr);
	va_end (argptr);

	CON_Print( va( "Warning: %s", string ) );
}
#endif

/*
============
Sys_FileTime

returns -1 if not present
============
*/
int Sys_FileTime( char *path )
{
	struct stat buf;

	if (stat (path,&buf) == -1)
		return -1;

	return buf.st_mtime;
}


/*
=================
Sys_LoadDll

First try to load library name from system library path,
from executable path, then fs_basepath.
=================
*/

void *Sys_LoadDll(const char *name, bool useSystemLib)
{
	void *dllhandle = nullptr;

	if(!System::dllExtension(name))
	{
		Common::printf("Refusing to attempt to load library \"%s\": Extension not allowed.\n", name);
		return nullptr;
	}

	if(useSystemLib)
	{
		Common::printf("Trying to load \"%s\"...\n", name);
		dllhandle = Sys_LoadLibrary(name);
	}
	
	if(!dllhandle)
	{
		const char *topDir;
		char libPath[MAX_OSPATH];
		int len;

		topDir = Sys_BinaryPath();

		if(!*topDir)
			topDir = ".";

		len = Com_sprintf(libPath, sizeof(libPath), "%s%c%s", topDir, PATH_SEP, name);
		if(len < sizeof(libPath))
		{
			Common::printf("Trying to load \"%s\" from \"%s\"...\n", name, topDir);
			dllhandle = Sys_LoadLibrary(libPath);
		}
		else
		{
			Common::printf("Skipping trying to load \"%s\" from \"%s\", file name is too long.\n", name, topDir);
		}

		if(!dllhandle)
		{
			const char *basePath = CvarSystem::variableString("fs_basepath");
			
			if(!basePath || !*basePath)
				basePath = ".";
			
			if(FileSystem::filenameCompare(topDir, basePath))
			{
				len = Com_sprintf(libPath, sizeof(libPath), "%s%c%s", basePath, PATH_SEP, name);
				if(len < sizeof(libPath))
				{
					Common::printf("Trying to load \"%s\" from \"%s\"...\n", name, basePath);
					dllhandle = Sys_LoadLibrary(libPath);
				}
				else
				{
					Common::printf("Skipping trying to load \"%s\" from \"%s\", file name is too long.\n", name, basePath);
				}
			}
			
			if(!dllhandle)
				Common::printf("Loading \"%s\" failed\n", name);
		}
	}
	
	return dllhandle;
}


/*
=================
Sys_ParseArgs
=================
*/
void Sys_ParseArgs( int argc, char **argv )
{
	if( argc == 2 )
	{
		if( !strcmp( argv[1], "--version" ) ||
				!strcmp( argv[1], "-v" ) )
		{
			const char* date = PRODUCT_DATE;
#ifdef DEDICATED
			fprintf( stdout, Q3_VERSION " dedicated server (%s)\n", date );
#else
			fprintf( stdout, Q3_VERSION " client (%s)\n", date );
#endif
			Sys_Exit( 0 );
		}
	}
}

#ifdef PROTOCOL_HANDLER
/*
=================
Sys_ParseProtocolUri

This parses a protocol URI, e.g. "quake3://connect/example.com:27950"
to a string that can be run in the console, or a null pointer if the
operation is invalid or unsupported.
At the moment only the "connect" command is supported.
=================
*/
char *Sys_ParseProtocolUri( const char *uri )
{
	// Both "quake3://" and "quake3:" can be used
	if ( Q_strncmp( uri, PROTOCOL_HANDLER ":", strlen( PROTOCOL_HANDLER ":" ) ) )
	{
		Common::printf( "Sys_ParseProtocolUri: unsupported protocol.\n" );
		return NULL;
	}
	uri += strlen( PROTOCOL_HANDLER ":" );
	if ( !Q_strncmp( uri, "//", strlen( "//" ) ) )
	{
		uri += strlen( "//" );
	}
	Common::printf( "Sys_ParseProtocolUri: %s\n", uri );

	// At the moment, only "connect/hostname:port" is supported
	if ( !Q_strncmp( uri, "connect/", strlen( "connect/" ) ) )
	{
		int i, bufsize;
		char *out;

		uri += strlen( "connect/" );
		if ( *uri == '\0' || *uri == '?' )
		{
			Common::printf( "Sys_ParseProtocolUri: missing argument.\n" );
			return NULL;
		}

		// Check for any unsupported characters
		// For safety reasons, the "hostname:port" part can only
		// contain characters from: a-zA-Z0-9.:-[]
		for ( i=0; uri[i] != '\0'; i++ )
		{
			if ( uri[i] == '?' )
			{
				// For forwards compatibility, any query string parameters are ignored (e.g. "?password=abcd")
				// However, these are not passed on macOS, so it may be a bad idea to add them.
				break;
			}

			if ( isalpha( uri[i] ) == 0 && isdigit( uri[i] ) == 0
				&& uri[i] != '.' && uri[i] != ':' && uri[i] != '-'
				&& uri[i] != '[' && uri[i] != ']' )
			{
				Common::printf( "Sys_ParseProtocolUri: hostname contains unsupported character.\n" );
				return NULL;
			}
		}

		bufsize = strlen( "connect " ) + i + 1;
		out = malloc( bufsize );
		strcpy( out, "connect " );
		strncat( out, uri, i );
		return out;
	}
	else
	{
		Common::printf( "Sys_ParseProtocolUri: unsupported command.\n" );
		return NULL;
	}
}
#endif

#ifndef DEFAULT_BASEDIR
#	if defined(DEFAULT_RELATIVE_BASEDIR)
#		define DEFAULT_BASEDIR Sys_BinaryPathRelative(DEFAULT_RELATIVE_BASEDIR)
#	elif defined(__APPLE__)
#		define DEFAULT_BASEDIR Sys_StripAppBundle(Sys_BinaryPath())
#	else
#		define DEFAULT_BASEDIR Sys_BinaryPath()
#	endif
#endif

/*
=================
Sys_SigHandler
=================
*/
void Sys_SigHandler( int signal )
{
	static bool signalcaught = false;

	if( signalcaught )
	{
		fprintf( stderr, "DOUBLE SIGNAL FAULT: Received signal %d, exiting...\n",
			signal );
	}
	else
	{
		signalcaught = true;
#ifndef DEDICATED
		CL_Shutdown(va("Received signal %d", signal), true, true);
#endif
		SV_Shutdown(va("Received signal %d", signal) );
	}

	if( signal == SIGTERM || signal == SIGINT )
		Sys_Exit( 1 );
	else
		Sys_Exit( 2 );
}

/*
=================
main
=================
*/
int main( int argc, char **argv )
{
	int   i;
	char  commandLine[ MAX_STRING_CHARS ] = { 0 };
#ifdef PROTOCOL_HANDLER
	char *protocolCommand = NULL;
#endif

#ifdef USE_AUTOUPDATER
	Sys_LaunchAutoupdater(argc, argv);
#endif

#ifndef DEDICATED
	// SDL version check

	// Compile time
#	if !SDL_VERSION_ATLEAST(MINSDL_MAJOR,MINSDL_MINOR,MINSDL_PATCH)
#		error A more recent version of SDL is required
#	endif

	// Run time
	SDL_version ver;
	SDL_GetVersion( &ver );

#define MINSDL_VERSION \
	XSTRING(MINSDL_MAJOR) "." \
	XSTRING(MINSDL_MINOR) "." \
	XSTRING(MINSDL_PATCH)

	if( SDL_VERSIONNUM( ver.major, ver.minor, ver.patch ) <
			SDL_VERSIONNUM( MINSDL_MAJOR, MINSDL_MINOR, MINSDL_PATCH ) )
	{
		System::dialog( DT_ERROR, va( "SDL version " MINSDL_VERSION " or greater is required, "
			"but only version %d.%d.%d was found. You may be able to obtain a more recent copy "
			"from https://www.libsdl.org/.", ver.major, ver.minor, ver.patch ), "SDL Library Too Old" );

		Sys_Exit( 1 );
	}
#endif

	Sys_PlatformInit( );
	Sys_SetMaxFileLimit( );

	// Set the initial time base
	System::milliseconds( );

#ifdef __APPLE__
	// This is passed if we are launched by double-clicking
	if ( argc >= 2 && Q_strncmp ( argv[1], "-psn", 4 ) == 0 )
		argc = 1;
#endif

	Sys_ParseArgs( argc, argv );
	Sys_SetBinaryPath( System::dirName( argv[ 0 ] ) );
	System::setDefaultInstallPath( DEFAULT_BASEDIR );

	// Concatenate the command line for passing to Common::init
	for( i = 1; i < argc; i++ )
	{
		bool containsSpaces;

		// For security reasons we always detect --uri, even when PROTOCOL_HANDLER is undefined
		// Any arguments after "--uri quake3://..." is ignored
		if ( !strcmp( argv[i], "--uri" ) )
		{
#ifdef PROTOCOL_HANDLER
			if ( argc > i+1 )
			{
				protocolCommand = Sys_ParseProtocolUri( argv[i+1] );
			}
#endif
			break;
		}

		containsSpaces = strchr(argv[i], ' ') != nullptr;
		if (containsSpaces)
			Q_strcat( commandLine, sizeof( commandLine ), "\"" );

		Q_strcat( commandLine, sizeof( commandLine ), argv[ i ] );

		if (containsSpaces)
			Q_strcat( commandLine, sizeof( commandLine ), "\"" );

		Q_strcat( commandLine, sizeof( commandLine ), " " );
	}

#ifdef PROTOCOL_HANDLER
	if ( protocolCommand != NULL )
	{
		Q_strcat( commandLine, sizeof( commandLine ), "+" );
		Q_strcat( commandLine, sizeof( commandLine ), protocolCommand );
		free( protocolCommand );
	}
#endif

	CON_Init( );
	Common::init( commandLine );
	Network::init( );

	signal( SIGILL, Sys_SigHandler );
	signal( SIGFPE, Sys_SigHandler );
	signal( SIGSEGV, Sys_SigHandler );
	signal( SIGTERM, Sys_SigHandler );
	signal( SIGINT, Sys_SigHandler );

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop( Common::frame, 0, 1 );
#else
	while( true )
	{
		Common::frame( );
	}
#endif

	return 0;
}
