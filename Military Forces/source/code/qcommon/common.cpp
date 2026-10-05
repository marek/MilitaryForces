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
// common.c -- misc functions used in client and server

#include "../game/q_shared.h"
#include "qcommon.h"
#include <csetjmp>
#ifndef _WIN32
#include <netinet/in.h>
#include <sys/stat.h> // umask
#else
#include <winsock.h>
#endif

int demo_protocols[] =
{ 5, 0 };

constexpr int MAX_NUM_ARGVS = 50;

constexpr int MIN_DEDICATED_COMHUNKMEGS = 1;
#define DEF_COMHUNKMEGS 	128
#define MIN_COMHUNKMEGS		DEF_COMHUNKMEGS
#define DEF_COMZONEMEGS		48
#define DEF_COMHUNKMEGS_S	XSTRING(DEF_COMHUNKMEGS)
#define DEF_COMZONEMEGS_S	XSTRING(DEF_COMZONEMEGS)


jmp_buf abortframe;		// an ERR_DROP occurred, exit the entire frame


FILE *debuglogfile;
static fileHandle_t pipefile;
static fileHandle_t logfile;
static fileHandle_t	com_journalFile;			// events are written here
fileHandle_t	com_journalDataFile;		// config files are written here

cvar_t	*com_speeds;
cvar_t	*com_developer;
cvar_t	*com_dedicated;
cvar_t	*com_timescale;
static cvar_t	*com_fixedtime;
cvar_t	*com_journal;
cvar_t	*com_maxfps;
cvar_t	*com_altivec;
cvar_t *Common::com_timedemo;
cvar_t	*com_sv_running;
cvar_t	*com_cl_running;
cvar_t *Common::com_logfile;		// 1 = buffer log, 2 = flush after each print
cvar_t	*com_pipefile;
cvar_t *Common::com_showtrace;
cvar_t	*com_version;
cvar_t *Common::com_blood;
cvar_t	*com_buildScript;	// for automated data building scripts
#ifdef CINEMATICS_INTRO
cvar_t	*com_introPlayed;
#endif
cvar_t	*cl_paused;
cvar_t	*sv_paused;
cvar_t  *cl_packetdelay;
cvar_t  *sv_packetdelay;
cvar_t	*com_cameraMode;
cvar_t	*com_ansiColor;
cvar_t	*com_unfocused;
cvar_t *Common::com_maxfpsUnfocused;
cvar_t	*com_minimized;
cvar_t *Common::com_maxfpsMinimized;
cvar_t	*com_abnormalExit;
cvar_t	*com_standalone;
cvar_t	*com_gamename;
cvar_t	*com_protocol;
#ifdef LEGACY_PROTOCOL
cvar_t	*com_legacyprotocol;
#endif
cvar_t	*com_basegame;
cvar_t  *com_homepath;
cvar_t *Common::com_busyWait;
#ifndef DEDICATED
cvar_t  *con_autochat;
#endif

#if id386
	long (QDECL *Q_ftol)(float f);
	void (QDECL *Q_SnapVector)(vec3_t vec);
#endif

// com_speeds times
int		time_game;
int		time_frontend;		// renderer frontend time
int		time_backend;		// renderer backend time

int			com_frameTime;
int Common::com_frameNumber;

bool	com_errorEntered = false;
bool	com_fullyInitialized = false;
bool Common::com_gameRestarting = false;
bool Common::com_gameClientRestarting = false;

char	com_errorMessage[MAXPRINTMSG];

static void Com_WriteConfig_f( );
void CIN_CloseAllVideos( );

//============================================================================

char *Common::rd_buffer;
int Common::rd_buffersize;
static void	(*rd_flush)( char *buffer );

void Common::beginRedirect (char *buffer, int buffersize, void (*flush)( char *) )
{
	if (!buffer || !buffersize || !flush)
		return;
	rd_buffer = buffer;
	rd_buffersize = buffersize;
	rd_flush = flush;

	*rd_buffer = 0;
}

void Common::endRedirect ()
{
	if ( rd_flush ) {
		rd_flush(rd_buffer);
	}

	rd_buffer = nullptr;
	rd_buffersize = 0;
	rd_flush = nullptr;
}

/*
=============
Common::printf

Both client and server can use this, and it will output
to the appropriate place.

A raw string should NEVER be passed as fmt, because of "%f" type crashers.
=============
*/
void QDECL Common::printf( const char *fmt, ... ) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];
  static bool opening_qconsole = false;


	va_start (argptr,fmt);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if ( rd_buffer ) {
		if ((strlen (msg) + strlen(rd_buffer)) > (rd_buffersize - 1)) {
			rd_flush(rd_buffer);
			*rd_buffer = 0;
		}
		Q_strcat(rd_buffer, rd_buffersize, msg);
    // TTimo nooo .. that would defeat the purpose
		//rd_flush(rd_buffer);			
		//*rd_buffer = 0;
		return;
	}

#ifndef DEDICATED
	ClientLoop::consolePrint( msg );
#endif

	// echo to dedicated console and early console
	System::print( msg );

	// logfile
	if ( com_logfile && com_logfile->asInt() ) {
    // TTimo: only open the qconsole.log if the filesystem is in an initialized state
    //   also, avoid recursing in the qconsole.log opening (i.e. if fs_debug is on)
		if ( !logfile && FileSystem::initialized() && !opening_qconsole) {
			struct tm *newtime;
			time_t aclock;

      opening_qconsole = true;

			time( &aclock );
			newtime = localtime( &aclock );

			logfile = FileSystem::fOpenFileWrite_HomeData( "qconsole.log" );
			
			if(logfile)
			{
				Common::printf( "logfile opened on %s\n", asctime( newtime ) );
			
				if ( com_logfile->asInt() > 1 )
				{
					// force it to not buffer so we get valid
					// data even if we are crashing
					FileSystem::forceFlush(logfile);
				}
			}
			else
			{
				Common::printf("Opening qconsole.log failed!\n");
				CvarSystem::setValue("logfile", 0);
			}

      opening_qconsole = false;
		}
		if ( logfile && FileSystem::initialized()) {
			FileSystem::write(msg, strlen(msg), logfile);
		}
	}
}


/*
================
Common::dPrintf

A Common::printf that only shows up if the "developer" cvar is set
================
*/
void QDECL Common::dPrintf( const char *fmt, ...) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];
		
	if ( !com_developer || !com_developer->asInt() ) {
		return;			// don't confuse non-developers with techie stuff...
	}

	va_start (argptr,fmt);	
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);
	
	Common::printf ("%s", msg);
}

/*
=============
Common::error

Both client and server can use this, and it will
do the appropriate thing.
=============
*/
void QDECL Common::error( int code, const char *fmt, ... ) {
	va_list		argptr;
	static int	lastErrorTime;
	static int	errorCount;
	int			currentTime;
	bool	restartClient;

	if(com_errorEntered)
		System::error("recursive error after: %s", com_errorMessage);

	com_errorEntered = true;

	CvarSystem::set("com_errorCode", va("%i", code));

	// when we are running automated scripts, make sure we
	// know if anything failed
	if ( com_buildScript && com_buildScript->asInt() ) {
		code = ERR_FATAL;
	}

	// if we are getting a solid stream of ERR_DROP, do an ERR_FATAL
	currentTime = System::milliseconds();
	if ( currentTime - lastErrorTime < 100 ) {
		if ( ++errorCount > 3 ) {
			code = ERR_FATAL;
		}
	} else {
		errorCount = 0;
	}
	lastErrorTime = currentTime;

	va_start (argptr,fmt);
	Q_vsnprintf (com_errorMessage, sizeof(com_errorMessage),fmt,argptr);
	va_end (argptr);

	if (code != ERR_DISCONNECT && code != ERR_NEED_CD)
		CvarSystem::set("com_errorMessage", com_errorMessage);

	restartClient = com_gameClientRestarting && !( com_cl_running && com_cl_running->asInt() );

	com_gameRestarting = false;
	com_gameClientRestarting = false;

	if (code == ERR_DISCONNECT || code == ERR_SERVERDISCONNECT) {
		ServerLoop::shutdown( "Server disconnected" );
		if ( restartClient ) {
			ClientLoop::init();
		}
		ClientLoop::disconnect( true );
		ClientLoop::flushMemory( );
		// make sure we can get at our local stuff
		FileSystem::pureServerSetLoadedPaks("", "");
		com_errorEntered = false;
		longjmp (abortframe, -1);
	} else if (code == ERR_DROP) {
		Common::printf ("********************\nERROR: %s\n********************\n", com_errorMessage);
		ServerLoop::shutdown (va("Server crashed: %s",  com_errorMessage));
		if ( restartClient ) {
			ClientLoop::init();
		}
		ClientLoop::disconnect( true );
		ClientLoop::flushMemory( );
		FileSystem::pureServerSetLoadedPaks("", "");
		com_errorEntered = false;
		longjmp (abortframe, -1);
	} else if ( code == ERR_NEED_CD ) {
		ServerLoop::shutdown( "Server didn't have CD" );
		if ( restartClient ) {
			ClientLoop::init();
		}
		if ( com_cl_running && com_cl_running->asInt() ) {
			ClientLoop::disconnect( true );
			ClientLoop::flushMemory( );
			ClientLoop::cdDialog();
		} else {
			Common::printf("Server didn't have CD\n" );
		}

		FileSystem::pureServerSetLoadedPaks("", "");

		com_errorEntered = false;
		longjmp (abortframe, -1);
	} else {
		ClientLoop::shutdown(va("Client fatal crashed: %s", com_errorMessage), true, true);
		ServerLoop::shutdown(va("Server fatal crashed: %s", com_errorMessage));
	}

	Common::shutdown ();

	System::error ("%s", com_errorMessage);
}


/*
=============
Common::quit_f

Both client and server can use this, and it will
do the appropriate things.
=============
*/
void Common::quit_f( ) {
	// don't try to shutdown if we are in a recursive error
	char *p = CommandSystem::args( );
	if ( !com_errorEntered ) {
		// Some VMs might execute "quit" command directly,
		// which would trigger an unload of active VM error.
		// System::quit will kill this process anyways, so
		// a corrupt call stack makes no difference
		ServerLoop::shutdown(p[0] ? p : "Server quit");
		ClientLoop::shutdown(p[0] ? p : "Client quit", true, true);
		Common::shutdown ();
		FileSystem::shutdown(true);
	}
	System::quit ();
}



/*
============================================================================

COMMAND LINE FUNCTIONS

+ characters separate the commandLine string into multiple console
command lines.

All of these are valid:

quake3 +set test blah +map test
quake3 set test blah+map test
quake3 set test blah + map test

============================================================================
*/

#define	MAX_CONSOLE_LINES	32
static int		com_numConsoleLines;
static char	*com_consoleLines[MAX_CONSOLE_LINES];

/*
==================
Com_ParseCommandLine

Break it up into multiple console lines
==================
*/
static void Com_ParseCommandLine( char *commandLine ) {
    int inq = 0;
    com_consoleLines[0] = commandLine;
    com_numConsoleLines = 1;

    while ( *commandLine ) {
        if (*commandLine == '"') {
            inq = !inq;
        }
        // look for a + separating character
        // if commandLine came from a file, we might have real line seperators
        if ( (*commandLine == '+' && !inq) || *commandLine == '\n'  || *commandLine == '\r' ) {
            if ( com_numConsoleLines == MAX_CONSOLE_LINES ) {
                return;
            }
            com_consoleLines[com_numConsoleLines] = commandLine + 1;
            com_numConsoleLines++;
            *commandLine = 0;
        }
        commandLine++;
    }
}


/*
===================
Common::safeMode

Check for "safe" on the command line, which will
skip loading of q3config.cfg
===================
*/
bool Common::safeMode( ) {
	int		i;

	for ( i = 0 ; i < com_numConsoleLines ; i++ ) {
		CommandSystem::tokenizeString( com_consoleLines[i] );
		if ( !Q_stricmp( CommandSystem::argv(0), "safe" )
			|| !Q_stricmp( CommandSystem::argv(0), "cvar_restart" ) ) {
			com_consoleLines[i][0] = 0;
			return true;
		}
	}
	return false;
}


/*
===============
Common::startupVariable

Searches for command line parameters that are set commands.
If match is not NULL, only that cvar will be looked for.
That is necessary because cddir and basedir need to be set
before the filesystem is started, but all other sets should
be after execing the config and default.
===============
*/
void Common::startupVariable( const char *match ) {
	int		i;
	char	*s;

	for (i=0 ; i < com_numConsoleLines ; i++) {
		CommandSystem::tokenizeString( com_consoleLines[i] );
		if ( strcmp( CommandSystem::argv(0), "set" ) ) {
			continue;
		}

		s = CommandSystem::argv(1);
		
		if(!match || !strcmp(s, match))
		{
			if(CvarSystem::flags(s) == CVAR_NONEXISTENT)
				CvarSystem::get(s, CommandSystem::argsFrom(2), CVAR_USER_CREATED);
			else
				CvarSystem::set2(s, CommandSystem::argsFrom(2), false);
		}
	}
}


/*
=================
Com_AddStartupCommands

Adds command line parameters as script statements
Commands are separated by + signs

Returns true if any late commands were added, which
will keep the demoloop from immediately starting
=================
*/
static bool Com_AddStartupCommands( ) {
	int		i;
	bool	added;

	added = false;
	// quote every token, so args with semicolons can work
	for (i=0 ; i < com_numConsoleLines ; i++) {
		if ( !com_consoleLines[i] || !com_consoleLines[i][0] ) {
			continue;
		}

		// set commands already added with Common::startupVariable
		if ( !Q_stricmpn( com_consoleLines[i], "set ", 4 ) ) {
			continue;
		}

		added = true;
		CommandBuffer::addText( com_consoleLines[i] );
		CommandBuffer::addText( "\n" );
	}

	return added;
}


//============================================================================

void Info_Print( const char *s ) {
	char	key[BIG_INFO_KEY];
	char	value[BIG_INFO_VALUE];
	char	*o;
	int		l;

	if (*s == '\\')
		s++;
	while (*s)
	{
		o = key;
		while (*s && *s != '\\')
			*o++ = *s++;

		l = o - key;
		if (l < 20)
		{
			Com_Memset (o, ' ', 20-l);
			key[20] = 0;
		}
		else
			*o = 0;
		Common::printf ("%s ", key);

		if (!*s)
		{
			Common::printf ("MISSING VALUE\n");
			return;
		}

		o = value;
		s++;
		while (*s && *s != '\\')
			*o++ = *s++;
		*o = 0;

		if (*s)
			s++;
		Common::printf ("%s\n", value);
	}
}

/*
============
Com_StringContains
============
*/
static char *Com_StringContains(char *str1, char *str2, int casesensitive) {
	int len, i, j;

	len = strlen(str1) - strlen(str2);
	for (i = 0; i <= len; i++, str1++) {
		for (j = 0; str2[j]; j++) {
			if (casesensitive) {
				if (str1[j] != str2[j]) {
					break;
				}
			}
			else {
				if (toupper(str1[j]) != toupper(str2[j])) {
					break;
				}
			}
		}
		if (!str2[j]) {
			return str1;
		}
	}
	return nullptr;
}

/*
============
Common::filter
============
*/
int Common::filter(char *filter, char *name, int casesensitive)
{
	char buf[MAX_TOKEN_CHARS];
	char *ptr;
	int i, found;

	while(*filter) {
		if (*filter == '*') {
			filter++;
			for (i = 0; *filter; i++) {
				if (*filter == '*' || *filter == '?' || *filter == '[') break;
				buf[i] = *filter;
				filter++;
			}
			buf[i] = '\0';
			if (strlen(buf)) {
				ptr = Com_StringContains(name, buf, casesensitive);
				if (!ptr) return false;
				name = ptr + strlen(buf);
			}
		}
		else if (*filter == '?') {
			filter++;
			name++;
		}
		else if (*filter == '[' && *(filter+1) == '[') {
			filter++;
		}
		else if (*filter == '[') {
			filter++;
			found = false;
			while(*filter && !found) {
				if (*filter == ']' && *(filter+1) != ']') break;
				if (*(filter+1) == '-' && *(filter+2) && (*(filter+2) != ']' || *(filter+3) == ']')) {
					if (casesensitive) {
						if (*name >= *filter && *name <= *(filter+2)) found = true;
					}
					else {
						if (toupper(*name) >= toupper(*filter) &&
							toupper(*name) <= toupper(*(filter+2))) found = true;
					}
					filter += 3;
				}
				else {
					if (casesensitive) {
						if (*filter == *name) found = true;
					}
					else {
						if (toupper(*filter) == toupper(*name)) found = true;
					}
					filter++;
				}
			}
			if (!found) return false;
			while(*filter) {
				if (*filter == ']' && *(filter+1) != ']') break;
				filter++;
			}
			filter++;
			name++;
		}
		else {
			if (casesensitive) {
				if (*filter != *name) return false;
			}
			else {
				if (toupper(*filter) != toupper(*name)) return false;
			}
			filter++;
			name++;
		}
	}
	return true;
}

/*
============
Common::filterPath
============
*/
int Common::filterPath(char *filter, char *name, int casesensitive)
{
	int i;
	char new_filter[MAX_QPATH];
	char new_name[MAX_QPATH];

	for (i = 0; i < MAX_QPATH-1 && filter[i]; i++) {
		if ( filter[i] == '\\' || filter[i] == ':' ) {
			new_filter[i] = '/';
		}
		else {
			new_filter[i] = filter[i];
		}
	}
	new_filter[i] = '\0';
	for (i = 0; i < MAX_QPATH-1 && name[i]; i++) {
		if ( name[i] == '\\' || name[i] == ':' ) {
			new_name[i] = '/';
		}
		else {
			new_name[i] = name[i];
		}
	}
	new_name[i] = '\0';
	return Common::filter(new_filter, new_name, casesensitive);
}

/*
================
Common::realTime
================
*/
int Common::realTime(qtime_t *qtime) {
	time_t t;
	struct tm *tms;

	t = time(nullptr);
	if (!qtime)
		return t;
	tms = localtime(&t);
	if (tms) {
		qtime->tm_sec = tms->tm_sec;
		qtime->tm_min = tms->tm_min;
		qtime->tm_hour = tms->tm_hour;
		qtime->tm_mday = tms->tm_mday;
		qtime->tm_mon = tms->tm_mon;
		qtime->tm_year = tms->tm_year;
		qtime->tm_wday = tms->tm_wday;
		qtime->tm_yday = tms->tm_yday;
		qtime->tm_isdst = tms->tm_isdst;
	}
	return t;
}


/*
==============================================================================

						ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

The zone calls are pretty much only used for small strings and structures,
all big things are allocated on the hunk.
==============================================================================
*/

#define	ZONEID	0x1d4a11
#define MINFRAGMENT	64

struct zonedebug_s {
	const char *label;
	const char *file;
	int line;
	int allocSize;
};
using zonedebug_t = zonedebug_s;

struct memblock_s {
	int		size;           // including the header and possibly tiny fragments
	int     tag;            // a tag of 0 is a free block
	struct memblock_s       *next, *prev;
	int     id;        		// should be ZONEID
#ifdef ZONE_DEBUG
	zonedebug_t d;
#endif
};
using memblock_t = memblock_s;

struct memzone_t {
	int		size;			// total bytes malloced, including header
	int		used;			// total bytes used
	memblock_t	blocklist;	// start / end cap for linked list
	memblock_t	*rover;
};

// main zone for all "dynamic" memory allocation
static memzone_t	*mainzone;
// we also have a small zone for small allocations that would only
// fragment the main zone (think of cvar and cmd strings)
static memzone_t	*smallzone;

static void Z_CheckHeap( );

/*
========================
Z_ClearZone
========================
*/
static void Z_ClearZone( memzone_t *zone, int size ) {
	memblock_t	*block;
	
	// set the entire zone to one free block

	zone->blocklist.next = zone->blocklist.prev = block =
		reinterpret_cast<memblock_t *>( reinterpret_cast<byte *>(zone) + sizeof(memzone_t) );
	zone->blocklist.tag = 1;	// in use block
	zone->blocklist.id = 0;
	zone->blocklist.size = 0;
	zone->rover = block;
	zone->size = size;
	zone->used = 0;
	
	block->prev = block->next = &zone->blocklist;
	block->tag = 0;			// free block
	block->id = ZONEID;
	block->size = size - sizeof(memzone_t);
}

/*
========================
Z_AvailableZoneMemory
========================
*/
static int Z_AvailableZoneMemory( memzone_t *zone ) {
	return zone->size - zone->used;
}

/*
========================
Zone::availableMemory
========================
*/
int Zone::availableMemory( ) {
	return Z_AvailableZoneMemory( mainzone );
}

/*
========================
Zone::free
========================
*/
void Zone::free( void *ptr ) {
	memblock_t	*block, *other;
	memzone_t *zone;
	
	if (!ptr) {
		Common::error( ERR_DROP, "Z_Free: NULL pointer" );
	}

	block = reinterpret_cast<memblock_t *>( static_cast<byte *>(ptr) - sizeof(memblock_t));
	if (block->id != ZONEID) {
		Common::error( ERR_FATAL, "Z_Free: freed a pointer without ZONEID" );
	}
	if (block->tag == 0) {
		Common::error( ERR_FATAL, "Z_Free: freed a freed pointer" );
	}
	// if static memory
	if (block->tag == TAG_STATIC) {
		return;
	}

	// check the memory trash tester
	if ( *reinterpret_cast<int *>(reinterpret_cast<byte *>(block) + block->size - 4 ) != ZONEID ) {
		Common::error( ERR_FATAL, "Z_Free: memory block wrote past end" );
	}

	if (block->tag == TAG_SMALL) {
		zone = smallzone;
	}
	else {
		zone = mainzone;
	}

	zone->used -= block->size;
	// set the block to something that should cause problems
	// if it is referenced...
	Com_Memset( ptr, 0xaa, block->size - sizeof( *block ) );

	block->tag = 0;		// mark as free
	
	other = block->prev;
	if (!other->tag) {
		// merge with previous free block
		other->size += block->size;
		other->next = block->next;
		other->next->prev = other;
		if (block == zone->rover) {
			zone->rover = other;
		}
		block = other;
	}

	zone->rover = block;

	other = block->next;
	if ( !other->tag ) {
		// merge the next free block onto the end
		block->size += other->size;
		block->next = other->next;
		block->next->prev = block;
	}
}


/*
================
Zone::freeTags
================
*/
void Zone::freeTags( int tag ) {
	memzone_t	*zone;

	if ( tag == TAG_SMALL ) {
		zone = smallzone;
	}
	else {
		zone = mainzone;
	}
	// use the rover as our pointer, because
	// Zone::free automatically adjusts it
	zone->rover = zone->blocklist.next;
	do {
		if ( zone->rover->tag == tag ) {
			Zone::free( reinterpret_cast<void *>(zone->rover + 1) );
			continue;
		}
		zone->rover = zone->rover->next;
	} while ( zone->rover != &zone->blocklist );
}


/*
================
Zone::tagMalloc
================
*/
void *Zone::tagMalloc( int size, int tag, const std::source_location& at ) {
#ifdef ZONE_DEBUG
	// debug builds record who asked for each block: the calling function, file and line
	const char *label = at.function_name();
	const char *file = at.file_name();
	const int line = static_cast<int>( at.line() );
	int		allocSize;
#endif
	int		extra;
	memblock_t	*start, *rover, *newblock, *base;
	memzone_t *zone;

	if (!tag) {
		Common::error( ERR_FATAL, "Z_TagMalloc: tried to use a 0 tag" );
	}

	if ( tag == TAG_SMALL ) {
		zone = smallzone;
	}
	else {
		zone = mainzone;
	}

#ifdef ZONE_DEBUG
	allocSize = size;
#endif
	//
	// scan through the block list looking for the first free block
	// of sufficient size
	//
	size += sizeof(memblock_t);	// account for size of block header
	size += 4;					// space for memory trash tester
	size = PAD(size, sizeof(intptr_t));		// align to 32/64 bit boundary
	
	base = rover = zone->rover;
	start = base->prev;
	
	do {
		if (rover == start)	{
			// scaned all the way around the list
#ifdef ZONE_DEBUG
			Zone::logHeap();

			Common::error(ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone: %s, line: %d (%s)",
								size, zone == smallzone ? "small" : "main", file, line, label);
#else
			Common::error(ERR_FATAL, "Z_Malloc: failed on allocation of %i bytes from the %s zone",
								size, zone == smallzone ? "small" : "main");
#endif
			return nullptr;
		}
		if (rover->tag) {
			base = rover = rover->next;
		} else {
			rover = rover->next;
		}
	} while (base->tag || base->size < size);
	
	//
	// found a block big enough
	//
	extra = base->size - size;
	if (extra > MINFRAGMENT) {
		// there will be a free fragment after the allocated block
		newblock = reinterpret_cast<memblock_t *>(reinterpret_cast<byte *>(base) + size );
		newblock->size = extra;
		newblock->tag = 0;			// free block
		newblock->prev = base;
		newblock->id = ZONEID;
		newblock->next = base->next;
		newblock->next->prev = newblock;
		base->next = newblock;
		base->size = size;
	}
	
	base->tag = tag;			// no longer a free block
	
	zone->rover = base->next;	// next allocation will start looking here
	zone->used += base->size;	//
	
	base->id = ZONEID;

#ifdef ZONE_DEBUG
	base->d.label = label;
	base->d.file = file;
	base->d.line = line;
	base->d.allocSize = allocSize;
#endif

	// marker for memory trash testing
	*reinterpret_cast<int *>(reinterpret_cast<byte *>(base) + base->size - 4) = ZONEID;

	return reinterpret_cast<void *>(reinterpret_cast<byte *>(base) + sizeof(memblock_t));
}

/*
========================
Zone::malloc
========================
*/
void *Zone::malloc( int size, const std::source_location& at ) {
	void	*buf;
	
  //Z_CheckHeap ();	// DEBUG

	buf = Zone::tagMalloc( size, TAG_GENERAL, at );
	Com_Memset( buf, 0, size );

	return buf;
}

void *Zone::smallMalloc( int size, const std::source_location& at ) {
	return Zone::tagMalloc( size, TAG_SMALL, at );
}

/*
========================
Z_CheckHeap
========================
*/
static void Z_CheckHeap( ) {
	memblock_t	*block;
	
	for (block = mainzone->blocklist.next ; ; block = block->next) {
		if (block->next == &mainzone->blocklist) {
			break;			// all blocks have been hit
		}
		if ( reinterpret_cast<byte *>(block) + block->size != reinterpret_cast<byte *>(block->next))
			Common::error( ERR_FATAL, "Z_CheckHeap: block size does not touch the next block" );
		if ( block->next->prev != block) {
			Common::error( ERR_FATAL, "Z_CheckHeap: next block doesn't have proper back link" );
		}
		if ( !block->tag && !block->next->tag ) {
			Common::error( ERR_FATAL, "Z_CheckHeap: two consecutive free blocks" );
		}
	}
}

/*
========================
Z_LogZoneHeap
========================
*/
static void Z_LogZoneHeap( memzone_t *zone, char *name ) {
#ifdef ZONE_DEBUG
	char dump[32], *ptr;
	int  i, j;
#endif
	memblock_t	*block;
	char		buf[4096];
	int size, allocSize, numBlocks;

	if (!logfile || !FileSystem::initialized())
		return;
	size = numBlocks = 0;
#ifdef ZONE_DEBUG
	allocSize = 0;
#endif
	Com_sprintf(buf, sizeof(buf), "\r\n================\r\n%s log\r\n================\r\n", name);
	FileSystem::write(buf, strlen(buf), logfile);
	for (block = zone->blocklist.next ; block->next != &zone->blocklist; block = block->next) {
		if (block->tag) {
#ifdef ZONE_DEBUG
			ptr = (reinterpret_cast<char *>(block)) + sizeof(memblock_t);
			j = 0;
			for (i = 0; i < 20 && i < block->d.allocSize; i++) {
				if (ptr[i] >= 32 && ptr[i] < 127) {
					dump[j++] = ptr[i];
				}
				else {
					dump[j++] = '_';
				}
			}
			dump[j] = '\0';
			Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s) [%s]\r\n", block->d.allocSize, block->d.file, block->d.line, block->d.label, dump);
			FileSystem::write(buf, strlen(buf), logfile);
			allocSize += block->d.allocSize;
#endif
			size += block->size;
			numBlocks++;
		}
	}
#ifdef ZONE_DEBUG
	// subtract debug memory
	size -= numBlocks * sizeof(zonedebug_t);
#else
	allocSize = numBlocks * sizeof(memblock_t); // + 32 bit alignment
#endif
	Com_sprintf(buf, sizeof(buf), "%d %s memory in %d blocks\r\n", size, name, numBlocks);
	FileSystem::write(buf, strlen(buf), logfile);
	Com_sprintf(buf, sizeof(buf), "%d %s memory overhead\r\n", size - allocSize, name);
	FileSystem::write(buf, strlen(buf), logfile);
}

/*
========================
Zone::logHeap
========================
*/
void Zone::logHeap( ) {
	Z_LogZoneHeap( mainzone, "MAIN" );
	Z_LogZoneHeap( smallzone, "SMALL" );
}

// static mem blocks to reduce a lot of small zone overhead
struct memstatic_s {
	memblock_t b;
	byte mem[2];
};
using memstatic_t = memstatic_s;

memstatic_t emptystring =
	{ {(sizeof(memblock_t)+2 + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'\0', '\0'} };
memstatic_t numberstring[] = {
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'0', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'1', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'2', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'3', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'4', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'5', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'6', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'7', '\0'} },
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'8', '\0'} }, 
	{ {(sizeof(memstatic_t) + 3) & ~3, TAG_STATIC, nullptr, nullptr, ZONEID}, {'9', '\0'} }
};

/*
========================
CopyString

 NOTE:	never write over the memory CopyString returns because
		memory from a memstatic_t might be returned
========================
*/
char *CopyString( const char *in ) {
	char	*out;

	if (!in[0]) {
		return (reinterpret_cast<char *>(&emptystring)) + sizeof(memblock_t);
	}
	else if (!in[1]) {
		if (in[0] >= '0' && in[0] <= '9') {
			return (reinterpret_cast<char *>(&numberstring[in[0]-'0'])) + sizeof(memblock_t);
		}
	}
	out = static_cast<char *>(Zone::smallMalloc(strlen(in)+1));
	strcpy (out, in);
	return out;
}

/*
==============================================================================

Goals:
	reproducible without history effects -- no out of memory errors on weird map to map changes
	allow restarting of the client without fragmentation
	minimize total pages in use at run time
	minimize total pages needed during load time

  Single block of memory with stack allocators coming from both ends towards the middle.

  One side is designated the temporary memory allocator.

  Temporary memory can be allocated and freed in any order.

  A highwater mark is kept of the most in use at any time.

  When there is no temporary memory allocated, the permanent and temp sides
  can be switched, allowing the already touched temp memory to be used for
  permanent storage.

  Temp memory must never be allocated on two ends at once, or fragmentation
  could occur.

  If we have any in-use temp memory, additional temp allocations must come from
  that side.

  If not, we can choose to make either side the new temp side and push future
  permanent allocations to the other side.  Permanent allocations should be
  kept on the side that has the current greatest wasted highwater mark.

==============================================================================
*/


#define	HUNK_MAGIC	0x89537892
#define	HUNK_FREE_MAGIC	0x89537893

struct hunkHeader_t {
	int		magic;
	int		size;
};

struct hunkUsed_t {
	int		mark;
	int		permanent;
	int		temp;
	int		tempHighwater;
};

struct hunkblock_s {
	int size;
	byte printed;
	struct hunkblock_s *next;
	const char *label;
	const char *file;
	int line;
};
using hunkblock_t = hunkblock_s;

static	hunkblock_t *hunkblocks;

static	hunkUsed_t	hunk_low, hunk_high;
static	hunkUsed_t	*hunk_permanent, *hunk_temp;

static	byte	*s_hunkData = nullptr;
static	int		s_hunkTotal;

static	int		s_zoneTotal;
static	int		s_smallZoneTotal;


/*
=================
Com_Meminfo_f
=================
*/
static void Com_Meminfo_f( ) {
	memblock_t	*block;
	int			zoneBytes, zoneBlocks;
	int			smallZoneBytes;
	int			botlibBytes, rendererBytes;
	int			unused;

	zoneBytes = 0;
	botlibBytes = 0;
	rendererBytes = 0;
	zoneBlocks = 0;
	for (block = mainzone->blocklist.next ; ; block = block->next) {
		if ( CommandSystem::argc() != 1 ) {
			Common::printf ("block:%p    size:%7i    tag:%3i\n",
				reinterpret_cast<void *>(block), block->size, block->tag);
		}
		if ( block->tag ) {
			zoneBytes += block->size;
			zoneBlocks++;
			if ( block->tag == TAG_BOTLIB ) {
				botlibBytes += block->size;
			} else if ( block->tag == TAG_RENDERER ) {
				rendererBytes += block->size;
			}
		}

		if (block->next == &mainzone->blocklist) {
			break;			// all blocks have been hit	
		}
		if ( reinterpret_cast<byte *>(block) + block->size != reinterpret_cast<byte *>(block->next)) {
			Common::printf ("ERROR: block size does not touch the next block\n");
		}
		if ( block->next->prev != block) {
			Common::printf ("ERROR: next block doesn't have proper back link\n");
		}
		if ( !block->tag && !block->next->tag ) {
			Common::printf ("ERROR: two consecutive free blocks\n");
		}
	}

	smallZoneBytes = 0;
	for (block = smallzone->blocklist.next ; ; block = block->next) {
		if ( block->tag ) {
			smallZoneBytes += block->size;
		}

		if (block->next == &smallzone->blocklist) {
			break;			// all blocks have been hit	
		}
	}

	Common::printf( "%8i bytes total hunk\n", s_hunkTotal );
	Common::printf( "%8i bytes total zone\n", s_zoneTotal );
	Common::printf( "\n" );
	Common::printf( "%8i low mark\n", hunk_low.mark );
	Common::printf( "%8i low permanent\n", hunk_low.permanent );
	if ( hunk_low.temp != hunk_low.permanent ) {
		Common::printf( "%8i low temp\n", hunk_low.temp );
	}
	Common::printf( "%8i low tempHighwater\n", hunk_low.tempHighwater );
	Common::printf( "\n" );
	Common::printf( "%8i high mark\n", hunk_high.mark );
	Common::printf( "%8i high permanent\n", hunk_high.permanent );
	if ( hunk_high.temp != hunk_high.permanent ) {
		Common::printf( "%8i high temp\n", hunk_high.temp );
	}
	Common::printf( "%8i high tempHighwater\n", hunk_high.tempHighwater );
	Common::printf( "\n" );
	Common::printf( "%8i total hunk in use\n", hunk_low.permanent + hunk_high.permanent );
	unused = 0;
	if ( hunk_low.tempHighwater > hunk_low.permanent ) {
		unused += hunk_low.tempHighwater - hunk_low.permanent;
	}
	if ( hunk_high.tempHighwater > hunk_high.permanent ) {
		unused += hunk_high.tempHighwater - hunk_high.permanent;
	}
	Common::printf( "%8i unused highwater\n", unused );
	Common::printf( "\n" );
	Common::printf( "%8i bytes in %i zone blocks\n", zoneBytes, zoneBlocks	);
	Common::printf( "        %8i bytes in dynamic botlib\n", botlibBytes );
	Common::printf( "        %8i bytes in dynamic renderer\n", rendererBytes );
	Common::printf( "        %8i bytes in dynamic other\n", zoneBytes - ( botlibBytes + rendererBytes ) );
	Common::printf( "        %8i bytes in small Zone memory\n", smallZoneBytes );
}

/*
===============
Common::touchMemory

Touch all known used data to make sure it is paged in
===============
*/
void Common::touchMemory( ) {
	int		start, end;
	int		i, j;
	unsigned	sum;
	memblock_t	*block;

	Z_CheckHeap();

	start = System::milliseconds();

	sum = 0;

	j = hunk_low.permanent >> 2;
	for ( i = 0 ; i < j ; i+=64 ) {			// only need to touch each page
		sum += (reinterpret_cast<int *>(s_hunkData))[i];
	}

	i = ( s_hunkTotal - hunk_high.permanent ) >> 2;
	j = hunk_high.permanent >> 2;
	for (  ; i < j ; i+=64 ) {			// only need to touch each page
		sum += (reinterpret_cast<int *>(s_hunkData))[i];
	}

	for (block = mainzone->blocklist.next ; ; block = block->next) {
		if ( block->tag ) {
			j = block->size >> 2;
			for ( i = 0 ; i < j ; i+=64 ) {				// only need to touch each page
				sum += (reinterpret_cast<int *>(block))[i];
			}
		}
		if ( block->next == &mainzone->blocklist ) {
			break;			// all blocks have been hit	
		}
	}

	end = System::milliseconds();

	(void)sum; // Suppress warning

	Common::printf( "Common::touchMemory: %i msec\n", end - start );
}



/*
=================
Com_InitZoneMemory
=================
*/
static void Com_InitSmallZoneMemory( ) {
	s_smallZoneTotal = 512 * 1024;
	smallzone = static_cast<memzone_t *>(calloc( s_smallZoneTotal, 1 ));
	if ( !smallzone ) {
		Common::error( ERR_FATAL, "Small zone data failed to allocate %1.1f megs", static_cast<float>(s_smallZoneTotal) / (1024*1024) );
	}
	Z_ClearZone( smallzone, s_smallZoneTotal );
}

static void Com_InitZoneMemory( ) {
	cvar_t	*cv;

	// Please note: com_zoneMegs can only be set on the command line, and
	// not in q3config.cfg or Common::startupVariable, as they haven't been
	// executed by this point. It's a chicken and egg problem. We need the
	// memory manager configured to handle those places where you would
	// configure the memory manager.

	// allocate the random block zone
	cv = CvarSystem::get( "com_zoneMegs", DEF_COMZONEMEGS_S, CVAR_LATCH | CVAR_ARCHIVE );

	if ( cv->asInt() < DEF_COMZONEMEGS ) {
		s_zoneTotal = 1024 * 1024 * DEF_COMZONEMEGS;
	} else {
		s_zoneTotal = cv->asInt() * 1024 * 1024;
	}

	mainzone = static_cast<memzone_t *>(calloc( s_zoneTotal, 1 ));
	if ( !mainzone ) {
		Common::error( ERR_FATAL, "Zone data failed to allocate %i megs", s_zoneTotal / (1024*1024) );
	}
	Z_ClearZone( mainzone, s_zoneTotal );

}

/*
=================
Hunk::log
=================
*/
void Hunk::log( ) {
	hunkblock_t	*block;
	char		buf[4096];
	int size, numBlocks;

	if (!logfile || !FileSystem::initialized())
		return;
	size = 0;
	numBlocks = 0;
	Com_sprintf(buf, sizeof(buf), "\r\n================\r\nHunk log\r\n================\r\n");
	FileSystem::write(buf, strlen(buf), logfile);
	for (block = hunkblocks ; block; block = block->next) {
#ifdef HUNK_DEBUG
		Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s)\r\n", block->size, block->file, block->line, block->label);
		FileSystem::write(buf, strlen(buf), logfile);
#endif
		size += block->size;
		numBlocks++;
	}
	Com_sprintf(buf, sizeof(buf), "%d Hunk memory\r\n", size);
	FileSystem::write(buf, strlen(buf), logfile);
	Com_sprintf(buf, sizeof(buf), "%d hunk blocks\r\n", numBlocks);
	FileSystem::write(buf, strlen(buf), logfile);
	FileSystem::flush(logfile);
}

/*
=================
Hunk::smallLog
=================
*/
void Hunk::smallLog( ) {
	hunkblock_t	*block, *block2;
	char		buf[4096];
	int size, locsize, numBlocks;

	if (!logfile || !FileSystem::initialized())
		return;
	for (block = hunkblocks ; block; block = block->next) {
		block->printed = false;
	}
	size = 0;
	numBlocks = 0;
	Com_sprintf(buf, sizeof(buf), "\r\n================\r\nHunk Small log\r\n================\r\n");
	FileSystem::write(buf, strlen(buf), logfile);
	for (block = hunkblocks; block; block = block->next) {
		if (block->printed) {
			continue;
		}
		locsize = block->size;
		for (block2 = block->next; block2; block2 = block2->next) {
			if (block->line != block2->line) {
				continue;
			}
			if (Q_stricmp(block->file, block2->file)) {
				continue;
			}
			size += block2->size;
			locsize += block2->size;
			block2->printed = true;
		}
#ifdef HUNK_DEBUG
		Com_sprintf(buf, sizeof(buf), "size = %8d: %s, line: %d (%s)\r\n", locsize, block->file, block->line, block->label);
		FileSystem::write(buf, strlen(buf), logfile);
#else
		(void)locsize; // Suppress warning
#endif
		size += block->size;
		numBlocks++;
	}
	Com_sprintf(buf, sizeof(buf), "%d Hunk memory\r\n", size);
	FileSystem::write(buf, strlen(buf), logfile);
	Com_sprintf(buf, sizeof(buf), "%d hunk blocks\r\n", numBlocks);
	FileSystem::write(buf, strlen(buf), logfile);
	FileSystem::flush(logfile);
}

/*
=================
Com_InitHunkZoneMemory
=================
*/
static void Com_InitHunkMemory( ) {
	cvar_t	*cv;
	int nMinAlloc;
	char *pMsg = nullptr;

	// make sure the file system has allocated and "not" freed any temp blocks
	// this allows the config and product id files ( journal files too ) to be loaded
	// by the file system without redunant routines in the file system utilizing different 
	// memory systems
	if (FileSystem::loadStack() != 0) {
		Common::error( ERR_FATAL, "Hunk initialization failed. File system load stack not zero");
	}

	// allocate the stack based hunk allocator
	cv = CvarSystem::get( "com_hunkMegs", DEF_COMHUNKMEGS_S, CVAR_LATCH | CVAR_ARCHIVE );
	CvarSystem::setDescription(cv, "The size of the hunk memory segment");

	// if we are not dedicated min allocation is 56, otherwise min is 1
	if (com_dedicated && com_dedicated->asInt()) {
		nMinAlloc = MIN_DEDICATED_COMHUNKMEGS;
		pMsg = "Minimum com_hunkMegs for a dedicated server is %i, allocating %i megs.\n";
	}
	else {
		nMinAlloc = MIN_COMHUNKMEGS;
		pMsg = "Minimum com_hunkMegs is %i, allocating %i megs.\n";
	}

	if ( cv->asInt() < nMinAlloc ) {
		s_hunkTotal = 1024 * 1024 * nMinAlloc;
	    Common::printf(pMsg, nMinAlloc, s_hunkTotal / (1024 * 1024));
	} else {
		s_hunkTotal = cv->asInt() * 1024 * 1024;
	}

	s_hunkData = static_cast<byte *>(calloc( s_hunkTotal + 31, 1 ));
	if ( !s_hunkData ) {
		Common::error( ERR_FATAL, "Hunk data failed to allocate %i megs", s_hunkTotal / (1024*1024) );
	}
	// cacheline align
	s_hunkData = (byte *) ( ( (intptr_t)s_hunkData + 31 ) & ~31 );
	Hunk::clear();

	CommandSystem::addCommand( "meminfo", Com_Meminfo_f );
#ifdef ZONE_DEBUG
	CommandSystem::addCommand( "zonelog", Zone::logHeap );
#endif
#ifdef HUNK_DEBUG
	CommandSystem::addCommand( "hunklog", Hunk::log );
	CommandSystem::addCommand( "hunksmalllog", Hunk::smallLog );
#endif
}

/*
====================
Hunk::memoryRemaining
====================
*/
int	Hunk::memoryRemaining( ) {
	int		low, high;

	low = hunk_low.permanent > hunk_low.temp ? hunk_low.permanent : hunk_low.temp;
	high = hunk_high.permanent > hunk_high.temp ? hunk_high.permanent : hunk_high.temp;

	return s_hunkTotal - ( low + high );
}

/*
===================
Hunk::setMark

The server calls this after the level and game VM have been loaded
===================
*/
void Hunk::setMark( ) {
	hunk_low.mark = hunk_low.permanent;
	hunk_high.mark = hunk_high.permanent;
}

/*
=================
Hunk::clearToMark

The client calls this before starting a vid_restart or snd_restart
=================
*/
void Hunk::clearToMark( ) {
	hunk_low.permanent = hunk_low.temp = hunk_low.mark;
	hunk_high.permanent = hunk_high.temp = hunk_high.mark;
}

/*
=================
Hunk::checkMark
=================
*/
bool Hunk::checkMark( ) {
	if( hunk_low.mark || hunk_high.mark ) {
		return true;
	}
	return false;
}

/*
=================
Hunk::clear

The server calls this before shutting down or loading a new map
=================
*/
void Hunk::clear( ) {

#ifndef DEDICATED
	ClientLoop::shutdownCGame();
	ClientLoop::shutdownUI();
#endif
	ServerLoop::shutdownGameProgs();
#ifndef DEDICATED
	CIN_CloseAllVideos();
#endif
	hunk_low.mark = 0;
	hunk_low.permanent = 0;
	hunk_low.temp = 0;
	hunk_low.tempHighwater = 0;

	hunk_high.mark = 0;
	hunk_high.permanent = 0;
	hunk_high.temp = 0;
	hunk_high.tempHighwater = 0;

	hunk_permanent = &hunk_low;
	hunk_temp = &hunk_high;

	Common::printf( "Hunk_Clear: reset the hunk ok\n" );
#ifdef HUNK_DEBUG
	hunkblocks = nullptr;
#endif
}

static void Hunk_SwapBanks( ) {
	hunkUsed_t	*swap;

	// can't swap banks if there is any temp already allocated
	if ( hunk_temp->temp != hunk_temp->permanent ) {
		return;
	}

	// if we have a larger highwater mark on this side, start making
	// our permanent allocations here and use the other side for temp
	if ( hunk_temp->tempHighwater - hunk_temp->permanent >
		hunk_permanent->tempHighwater - hunk_permanent->permanent ) {
		swap = hunk_temp;
		hunk_temp = hunk_permanent;
		hunk_permanent = swap;
	}
}

/*
=================
Hunk::alloc

Allocate permanent (until the hunk is cleared) memory
=================
*/
void *Hunk::alloc( int size, ha_pref preference, const std::source_location& at ) {
#ifdef HUNK_DEBUG
	const char *label = at.function_name();
	const char *file = at.file_name();
	const int line = static_cast<int>( at.line() );
#endif
	void	*buf;

	if ( s_hunkData == nullptr)
	{
		Common::error( ERR_FATAL, "Hunk_Alloc: Hunk memory system not initialized" );
	}

	// can't do preference if there is any temp allocated
	if (preference == h_dontcare || hunk_temp->temp != hunk_temp->permanent) {
		Hunk_SwapBanks();
	} else {
		if (preference == h_low && hunk_permanent != &hunk_low) {
			Hunk_SwapBanks();
		} else if (preference == h_high && hunk_permanent != &hunk_high) {
			Hunk_SwapBanks();
		}
	}

#ifdef HUNK_DEBUG
	size += sizeof(hunkblock_t);
#endif

	// round to cacheline
	size = (size+31)&~31;

	if ( hunk_low.temp + hunk_high.temp + size > s_hunkTotal ) {
#ifdef HUNK_DEBUG
		Hunk::log();
		Hunk::smallLog();

		Common::error(ERR_DROP, "Hunk_Alloc failed on %i: %s, line: %d (%s)", size, file, line, label);
#else
		Common::error(ERR_DROP, "Hunk_Alloc failed on %i", size);
#endif
	}

	if ( hunk_permanent == &hunk_low ) {
		buf = reinterpret_cast<void *>(s_hunkData + hunk_permanent->permanent);
		hunk_permanent->permanent += size;
	} else {
		hunk_permanent->permanent += size;
		buf = reinterpret_cast<void *>(s_hunkData + s_hunkTotal - hunk_permanent->permanent );
	}

	hunk_permanent->temp = hunk_permanent->permanent;

	Com_Memset( buf, 0, size );

#ifdef HUNK_DEBUG
	{
		hunkblock_t *block;

		block = static_cast<hunkblock_t *>(buf);
		block->size = size - sizeof(hunkblock_t);
		block->file = file;
		block->label = label;
		block->line = line;
		block->next = hunkblocks;
		hunkblocks = block;
		buf = (static_cast<byte *>(buf)) + sizeof(hunkblock_t);
	}
#endif
	return buf;
}

/*
=================
Hunk::allocateTempMemory

This is used by the file loading system.
Multiple files can be loaded in temporary memory.
When the files-in-use count reaches zero, all temp memory will be deleted
=================
*/
void *Hunk::allocateTempMemory( int size ) {
	void		*buf;
	hunkHeader_t	*hdr;

	// return a Zone::malloc'd block if the hunk has not been initialized
	// this allows the config and product id files ( journal files too ) to be loaded
	// by the file system without redunant routines in the file system utilizing different 
	// memory systems
	if ( s_hunkData == nullptr )
	{
		return Zone::malloc(size);
	}

	Hunk_SwapBanks();

	size = PAD(size, sizeof(intptr_t)) + sizeof( hunkHeader_t );

	if ( hunk_temp->temp + hunk_permanent->permanent + size > s_hunkTotal ) {
		Common::error( ERR_DROP, "Hunk_AllocateTempMemory: failed on %i", size );
	}

	if ( hunk_temp == &hunk_low ) {
		buf = reinterpret_cast<void *>(s_hunkData + hunk_temp->temp);
		hunk_temp->temp += size;
	} else {
		hunk_temp->temp += size;
		buf = reinterpret_cast<void *>(s_hunkData + s_hunkTotal - hunk_temp->temp );
	}

	if ( hunk_temp->temp > hunk_temp->tempHighwater ) {
		hunk_temp->tempHighwater = hunk_temp->temp;
	}

	hdr = static_cast<hunkHeader_t *>(buf);
	buf = reinterpret_cast<void *>(hdr+1);

	hdr->magic = HUNK_MAGIC;
	hdr->size = size;

	// don't bother clearing, because we are going to load a file over it
	return buf;
}


/*
==================
Hunk::freeTempMemory
==================
*/
void Hunk::freeTempMemory( void *buf ) {
	hunkHeader_t	*hdr;

	  // free with Zone::free if the hunk has not been initialized
	  // this allows the config and product id files ( journal files too ) to be loaded
	  // by the file system without redunant routines in the file system utilizing different 
	  // memory systems
	if ( s_hunkData == nullptr )
	{
		Zone::free(buf);
		return;
	}


	hdr = ( static_cast<hunkHeader_t *>(buf) ) - 1;
	if ( hdr->magic != HUNK_MAGIC ) {
		Common::error( ERR_FATAL, "Hunk_FreeTempMemory: bad magic" );
	}

	hdr->magic = HUNK_FREE_MAGIC;

	// this only works if the files are freed in stack order,
	// otherwise the memory will stay around until Hunk::clearTempMemory
	if ( hunk_temp == &hunk_low ) {
		if ( hdr == reinterpret_cast<void *>(s_hunkData + hunk_temp->temp - hdr->size ) ) {
			hunk_temp->temp -= hdr->size;
		} else {
			Common::printf( "Hunk_FreeTempMemory: not the final block\n" );
		}
	} else {
		if ( hdr == reinterpret_cast<void *>(s_hunkData + s_hunkTotal - hunk_temp->temp ) ) {
			hunk_temp->temp -= hdr->size;
		} else {
			Common::printf( "Hunk_FreeTempMemory: not the final block\n" );
		}
	}
}


/*
=================
Hunk::clearTempMemory

The temp space is no longer needed.  If we have left more
touched but unused memory on this side, have future
permanent allocs use this side.
=================
*/
void Hunk::clearTempMemory( ) {
	if ( s_hunkData != nullptr ) {
		hunk_temp->temp = hunk_temp->permanent;
	}
}

/*
===================================================================

EVENTS AND JOURNALING

In addition to these events, .cfg files are also copied to the
journaled file
===================================================================
*/

#define	MAX_PUSHED_EVENTS	            1024
static int com_pushedEventsHead = 0;
static int com_pushedEventsTail = 0;
static sysEvent_t	com_pushedEvents[MAX_PUSHED_EVENTS];

/*
=================
Com_InitJournaling
=================
*/
static void Com_InitJournaling( ) {
	Common::startupVariable( "journal" );
	com_journal = CvarSystem::get ("journal", "0", CVAR_INIT);
	if ( !com_journal->asInt() ) {
		return;
	}

	if ( com_journal->asInt() == 1 ) {
		Common::printf( "Journaling events\n");
		com_journalFile = FileSystem::fOpenFileWrite_HomeState( "journal.dat" );
		com_journalDataFile = FileSystem::fOpenFileWrite_HomeState( "journaldata.dat" );
	} else if ( com_journal->asInt() == 2 ) {
		Common::printf( "Replaying journaled events\n");
		FileSystem::fOpenFileRead( "journal.dat", &com_journalFile, true );
		FileSystem::fOpenFileRead( "journaldata.dat", &com_journalDataFile, true );
	}

	if ( !com_journalFile || !com_journalDataFile ) {
		CvarSystem::set( "com_journal", "0" );
		com_journalFile = 0;
		com_journalDataFile = 0;
		Common::printf( "Couldn't open journal files\n" );
	}
}

/*
========================================================================

EVENT LOOP

========================================================================
*/

#define MAX_QUEUED_EVENTS  256
#define MASK_QUEUED_EVENTS ( MAX_QUEUED_EVENTS - 1 )

static sysEvent_t  eventQueue[ MAX_QUEUED_EVENTS ];
int Common::eventHead = 0;
int Common::eventTail = 0;

/*
================
Common::queueEvent

A time of 0 will get the current time
Ptr should either be null, or point to a block of data that can
be freed by the game later.
================
*/
void Common::queueEvent( int time, sysEventType_t type, int value, int value2, int ptrLength, void *ptr )
{
	sysEvent_t  *ev;

	// combine mouse movement with previous mouse event
	if ( type == SE_MOUSE && eventHead != eventTail )
	{
		ev = &eventQueue[ ( eventHead + MAX_QUEUED_EVENTS - 1 ) & MASK_QUEUED_EVENTS ];

		if ( ev->evType == SE_MOUSE )
		{
			ev->evValue += value;
			ev->evValue2 += value2;
			return;
		}
	}

	ev = &eventQueue[ eventHead & MASK_QUEUED_EVENTS ];

	if ( eventHead - eventTail >= MAX_QUEUED_EVENTS )
	{
		Common::printf("Common::queueEvent: overflow\n");
		// we are discarding an event, but don't leak memory
		if ( ev->evPtr )
		{
			Zone::free( ev->evPtr );
		}
		eventTail++;
	}

	eventHead++;

	if ( time == 0 )
	{
		time = System::milliseconds();
	}

	ev->evTime = time;
	ev->evType = type;
	ev->evValue = value;
	ev->evValue2 = value2;
	ev->evPtrLength = ptrLength;
	ev->evPtr = ptr;
}

/*
================
Common::getSystemEvent

================
*/
sysEvent_t Common::getSystemEvent( )
{
	sysEvent_t  ev;
	char        *s;

	// return if we have data
	if ( eventHead > eventTail )
	{
		eventTail++;
		return eventQueue[ ( eventTail - 1 ) & MASK_QUEUED_EVENTS ];
	}

	// check for console commands
	s = System::consoleInput();
	if ( s )
	{
		char  *b;
		int   len;

		len = strlen( s ) + 1;
		b = static_cast<char *>(Zone::malloc( len ));
		strcpy( b, s );
		Common::queueEvent( 0, SE_CONSOLE, 0, 0, len, b );
	}

	// return if we have data
	if ( eventHead > eventTail )
	{
		eventTail++;
		return eventQueue[ ( eventTail - 1 ) & MASK_QUEUED_EVENTS ];
	}

	// create an empty event to return
	memset( &ev, 0, sizeof( ev ) );
	ev.evTime = System::milliseconds();

	return ev;
}

/*
=================
Com_GetRealEvent
=================
*/
static sysEvent_t	Com_GetRealEvent( ) {
	int			r;
	sysEvent_t	ev;

	// either get an event from the system or the journal file
	if ( com_journal->asInt() == 2 ) {
		r = FileSystem::read( &ev, sizeof(ev), com_journalFile );
		if ( r != sizeof(ev) ) {
			Common::error( ERR_FATAL, "Error reading from journal file" );
		}
		if ( ev.evPtrLength ) {
			ev.evPtr = Zone::malloc( ev.evPtrLength );
			r = FileSystem::read( ev.evPtr, ev.evPtrLength, com_journalFile );
			if ( r != ev.evPtrLength ) {
				Common::error( ERR_FATAL, "Error reading from journal file" );
			}
		}
	} else {
		ev = Common::getSystemEvent();

		// write the journal value out if needed
		if ( com_journal->asInt() == 1 ) {
			r = FileSystem::write( &ev, sizeof(ev), com_journalFile );
			if ( r != sizeof(ev) ) {
				Common::error( ERR_FATAL, "Error writing to journal file" );
			}
			if ( ev.evPtrLength ) {
				r = FileSystem::write( ev.evPtr, ev.evPtrLength, com_journalFile );
				if ( r != ev.evPtrLength ) {
					Common::error( ERR_FATAL, "Error writing to journal file" );
				}
			}
		}
	}

	return ev;
}


/*
=================
Com_InitPushEvent
=================
*/
static void Com_InitPushEvent( ) {
  // clear the static buffer array
  // this requires SE_NONE to be accepted as a valid but NOP event
  memset( com_pushedEvents, 0, sizeof(com_pushedEvents) );
  // reset counters while we are at it
  // beware: GetEvent might still return an SE_NONE from the buffer
  com_pushedEventsHead = 0;
  com_pushedEventsTail = 0;
}


/*
=================
Com_PushEvent
=================
*/
static void Com_PushEvent( sysEvent_t *event ) {
	sysEvent_t		*ev;
	static int printedWarning = 0;

	ev = &com_pushedEvents[ com_pushedEventsHead & (MAX_PUSHED_EVENTS-1) ];

	if ( com_pushedEventsHead - com_pushedEventsTail >= MAX_PUSHED_EVENTS ) {

		// don't print the warning constantly, or it can give time for more...
		if ( !printedWarning ) {
			printedWarning = true;
			Common::printf( "WARNING: Com_PushEvent overflow\n" );
		}

		if ( ev->evPtr ) {
			Zone::free( ev->evPtr );
		}
		com_pushedEventsTail++;
	} else {
		printedWarning = false;
	}

	*ev = *event;
	com_pushedEventsHead++;
}

/*
=================
Com_GetEvent
=================
*/
static sysEvent_t	Com_GetEvent( ) {
	if ( com_pushedEventsHead > com_pushedEventsTail ) {
		com_pushedEventsTail++;
		return com_pushedEvents[ (com_pushedEventsTail-1) & (MAX_PUSHED_EVENTS-1) ];
	}
	return Com_GetRealEvent();
}

/*
=================
Common::runAndTimeServerPacket
=================
*/
void Common::runAndTimeServerPacket( netadr_t *evFrom, Message *buf ) {
	int		t1, t2, msec;

	t1 = 0;

	if ( com_speeds->asInt() ) {
		t1 = System::milliseconds ();
	}

	ServerLoop::packetEvent( *evFrom, buf );

	if ( com_speeds->asInt() ) {
		t2 = System::milliseconds ();
		msec = t2 - t1;
		if ( com_speeds->asInt() == 3 ) {
			Common::printf( "SV_PacketEvent time: %i\n", msec );
		}
	}
}

/*
=================
Common::eventLoop

Returns last event time
=================
*/
int Common::eventLoop( ) {
	sysEvent_t	ev;
	netadr_t	evFrom;
	byte		bufData[MAX_MSGLEN];
	Message		buf;

	buf = Message( bufData, sizeof( bufData ) );

	while ( true ) {
		ev = Com_GetEvent();

		// if no more events are available
		if ( ev.evType == SE_NONE ) {
			// manually send packet events for the loopback channel
			while ( Network::getLoopPacket( NS_CLIENT, &evFrom, &buf ) ) {
				ClientLoop::packetEvent( evFrom, &buf );
			}

			while ( Network::getLoopPacket( NS_SERVER, &evFrom, &buf ) ) {
				// if the server just shut down, flush the events
				if ( com_sv_running->asInt() ) {
					Common::runAndTimeServerPacket( &evFrom, &buf );
				}
			}

			return ev.evTime;
		}


		switch(ev.evType)
		{
			case SE_KEY:
				ClientLoop::keyEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
			case SE_CHAR:
				ClientLoop::charEvent( ev.evValue );
			break;
			case SE_MOUSE:
				ClientLoop::mouseEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
			case SE_JOYSTICK_AXIS:
				ClientLoop::joystickEvent( ev.evValue, ev.evValue2, ev.evTime );
			break;
			case SE_CONSOLE:
				CommandBuffer::addText( static_cast<char *>(ev.evPtr) );
				CommandBuffer::addText( "\n" );
			break;
			default:
				Common::error( ERR_FATAL, "Common::eventLoop: bad event type %i", ev.evType );
			break;
		}

		// free any block data
		if ( ev.evPtr ) {
			Zone::free( ev.evPtr );
		}
	}

	return 0;	// never reached
}

/*
================
Common::milliseconds

Can be used for profiling, but will be journaled accurately
================
*/
int Common::milliseconds () {
	sysEvent_t	ev;

	// get events and push them until we get a null event with the current time
	do {

		ev = Com_GetRealEvent();
		if ( ev.evType != SE_NONE ) {
			Com_PushEvent( &ev );
		}
	} while ( ev.evType != SE_NONE );
	
	return ev.evTime;
}

//============================================================================

/*
=============
Com_Error_f

Just throw a fatal error to
test error shutdown procedures
=============
*/
static void Q_NO_RETURN Com_Error_f () {
	if ( CommandSystem::argc() > 1 ) {
		Common::error( ERR_DROP, "Testing drop error" );
	} else {
		Common::error( ERR_FATAL, "Testing fatal error" );
	}
}


/*
=============
Com_Freeze_f

Just freeze in place for a given number of seconds to test
error recovery
=============
*/
static void Com_Freeze_f () {
	float	s;
	int		start, now;

	if ( CommandSystem::argc() != 2 ) {
		Common::printf( "freeze <seconds>\n" );
		return;
	}
	s = atof( CommandSystem::argv(1) );

	start = Common::milliseconds();

	while ( true ) {
		now = Common::milliseconds();
		if ( ( now - start ) * 0.001 > s ) {
			break;
		}
	}
}

/*
=================
Com_Crash_f

A way to force a bus error for development reasons
=================
*/
static void Com_Crash_f( ) {
	* ( volatile int * ) nullptr = 0x12345678;
}

/*
==================
Com_Setenv_f

For controlling environment variables
==================
*/
static void Com_Setenv_f()
{
	int argc = CommandSystem::argc();
	char *arg1 = CommandSystem::argv(1);

	if(argc > 2)
	{
		char *arg2 = CommandSystem::argsFrom(2);
		
		System::setEnv(arg1, arg2);
	}
	else if(argc == 2)
	{
		char *env = getenv(arg1);
		
		if(env)
			Common::printf("%s=%s\n", arg1, env);
		else
			Common::printf("%s undefined\n", arg1);
        }
}

/*
==================
Com_ExecuteCfg

For controlling environment variables
==================
*/

static void Com_ExecuteCfg()
{
	CommandBuffer::executeText(EXEC_NOW, "exec default.cfg\n");
	CommandBuffer::execute(); // Always execute after exec to prevent text buffer overflowing

	if(!Common::safeMode())
	{
		// skip the q3config.cfg and autoexec.cfg if "safe" is on the command line
		CommandBuffer::executeText(EXEC_NOW, "exec " Q3CONFIG_CFG "\n");
		CommandBuffer::execute();
		CommandBuffer::executeText(EXEC_NOW, "exec autoexec.cfg\n");
		CommandBuffer::execute();
	}
}

/*
==================
Common::gameRestart

Change to a new mod properly with cleaning up cvars before switching.
==================
*/

void Common::gameRestart(int checksumFeed, bool disconnect)
{
	// make sure no recursion can be triggered
	if(!com_gameRestarting && com_fullyInitialized)
	{
		com_gameRestarting = true;
		com_gameClientRestarting = com_cl_running->asInt();

		// Kill server if we have one
		if(com_sv_running->asInt())
			ServerLoop::shutdown("Game directory changed");

		if(com_gameClientRestarting)
		{
			if(disconnect)
				ClientLoop::disconnect(false);
				
			ClientLoop::shutdown("Game directory changed", disconnect, false);
		}

		FileSystem::restart(checksumFeed);
	
		// Clean out any user and VM created cvars
		CvarSystem::restart(true);
		Com_ExecuteCfg();

		if(disconnect)
		{
			// We don't want to change any network settings if gamedir
			// change was triggered by a connect to server because the
			// new network settings might make the connection fail.
			Network::restart_f();
		}

		if(com_gameClientRestarting)
		{
			ClientLoop::init();
			ClientLoop::startHunkUsers(false);
		}
		
		com_gameRestarting = false;
		com_gameClientRestarting = false;
	}
}

/*
==================
Com_GameRestart_f

Expose possibility to change current running mod to the user
==================
*/

static void Com_GameRestart_f()
{
	CvarSystem::set("fs_game", CommandSystem::argv(1));

	Common::gameRestart(0, true);
}

#ifndef STANDALONE

// TTimo: centralizing the cl_cdkey stuff after I discovered a buffer overflow problem with the dedicated server version
//   not sure it's necessary to have different defaults for regular and dedicated, but I don't want to risk it
//   https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=470
#ifndef DEDICATED
char	cl_cdkey[34] = "                                ";
#else
char	cl_cdkey[34] = "123456789";
#endif

/*
=================
Com_ReadCDKey
=================
*/
void Com_ReadCDKey( const char *filename ) {
	fileHandle_t	f;
	char			buffer[33];
	char			fbuffer[MAX_OSPATH];

	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/q3key", filename);

	FileSystem::baseDir_FOpenFileRead( fbuffer, &f );
	if ( !f ) {
		Com_Memset( cl_cdkey, '\0', 17 );
		return;
	}

	Com_Memset( buffer, 0, sizeof(buffer) );

	FileSystem::read( buffer, 16, f );
	FileSystem::fCloseFile( f );

	Q_strlwr(buffer);

	if (ClientLoop::cdKeyValidate(buffer, NULL)) {
		Q_strncpyz( cl_cdkey, buffer, 17 );
	} else {
		Com_Memset( cl_cdkey, '\0', 17 );
	}
}

/*
=================
Com_AppendCDKey
=================
*/
void Com_AppendCDKey( const char *filename ) {
	fileHandle_t	f;
	char			buffer[33];
	char			fbuffer[MAX_OSPATH];

	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/q3key", filename);

	FileSystem::baseDir_FOpenFileRead( fbuffer, &f );
	if (!f) {
		Com_Memset( &cl_cdkey[16], '\0', 17 );
		return;
	}

	Com_Memset( buffer, 0, sizeof(buffer) );

	FileSystem::read( buffer, 16, f );
	FileSystem::fCloseFile( f );

	if (ClientLoop::cdKeyValidate(buffer, NULL)) {
		strcat( &cl_cdkey[16], buffer );
	} else {
		Com_Memset( &cl_cdkey[16], '\0', 17 );
	}
}

#ifndef DEDICATED
/*
=================
Com_WriteCDKey
=================
*/
static void Com_WriteCDKey( const char *filename, const char *ikey ) {
	fileHandle_t	f;
	char			fbuffer[MAX_OSPATH];
	char			key[17];
#ifndef _WIN32
	mode_t			savedumask;
#endif


	Com_sprintf(fbuffer, sizeof(fbuffer), "%s/q3key", filename);


	Q_strncpyz( key, ikey, 17 );

	if(!ClientLoop::cdKeyValidate(key, NULL) ) {
		return;
	}

#ifndef _WIN32
	savedumask = umask(0077);
#endif
	f = FileSystem::baseDir_FOpenFileWrite_HomeState( fbuffer );
	if ( !f ) {
		Common::printf ("Couldn't write CD key to %s.\n", fbuffer );
		goto out;
	}

	Q_strlwr(key);

	FileSystem::write( key, 16, f );

	FileSystem::printf( f, "\n// generated by " PRODUCT_NAME ", do not modify\r\n" );
	FileSystem::printf( f, "// Do not give this file to ANYONE.\r\n" );
	FileSystem::printf( f, "// id Software and Activision will NOT ask you to send this file to them.\r\n");

	FileSystem::fCloseFile( f );
out:
#ifndef _WIN32
	umask(savedumask);
#else
	;
#endif
}
#endif

#endif // STANDALONE

static void Com_DetectAltivec()
{
	// Only detect if user hasn't forcibly disabled it.
	if (com_altivec->asInt()) {
		static bool altivec = false;
		static bool detected = false;
		if (!detected) {
			altivec = !!( System::getProcessorFeatures( ) & CF_ALTIVEC );
			detected = true;
		}

		if (!altivec) {
			CvarSystem::set( "com_altivec", "0" );  // we don't have it! Disable support!
		}
	}
}

/*
=================
Com_DetectSSE
Find out whether we have SSE support for Q_ftol function
=================
*/

#if id386 || idx64

static void Com_DetectSSE()
{
#if !idx64
	cpuFeatures_t feat;
	
	feat = System::getProcessorFeatures();

	if(feat & CF_SSE)
	{
		if(feat & CF_SSE2)
			Q_SnapVector = qsnapvectorsse;
		else
			Q_SnapVector = qsnapvectorx87;

		Q_ftol = qftolsse;
#endif

		Common::printf("SSE instruction set enabled\n");
#if !idx64
	}
	else
	{
		Q_ftol = qftolx87;
		Q_SnapVector = qsnapvectorx87;

		Common::printf("SSE instruction set not available\n");
	}
#endif
}

#else

#define Com_DetectSSE()

#endif

/*
=================
Com_InitRand
Seed the random number generator, if possible with an OS supplied random seed.
=================
*/
static void Com_InitRand()
{
	unsigned int seed;

	if(System::randomBytes(reinterpret_cast<byte *>(&seed), sizeof(seed)))
		srand(seed);
	else
		srand(time(nullptr));
}

/*
=================
Common::init
=================
*/
void Common::init( char *commandLine ) {
	char	*s;
	int	qport;

	Common::printf( "%s %s %s\n", Q3_VERSION, PLATFORM_STRING, PRODUCT_DATE );

	if ( setjmp (abortframe) ) {
		System::error ("Error during initialization");
	}

	// Clear queues
	Com_Memset( &eventQueue[ 0 ], 0, MAX_QUEUED_EVENTS * sizeof( sysEvent_t ) );

	// initialize the weak pseudo-random number generator for use later.
	Com_InitRand();

	// do this before anything else decides to push events
	Com_InitPushEvent();

	Com_InitSmallZoneMemory();
	CvarSystem::init ();

	// prepare enough of the subsystems to handle
	// cvar and command buffer management
	Com_ParseCommandLine( commandLine );

//	Swap_Init ();
	CommandBuffer::init ();

	Com_DetectSSE();

	// override anything from the config files with command line args
	Common::startupVariable( nullptr );

	Com_InitZoneMemory();
	CommandSystem::init ();

	// get the developer cvar set as early as possible
	com_developer = CvarSystem::get("developer", "0", CVAR_TEMP);

	// done early so bind command exists
	ClientLoop::initKeyCommands();

	com_standalone = CvarSystem::get("com_standalone", "0", CVAR_ROM);
	com_basegame = CvarSystem::get("com_basegame", BASEGAME, CVAR_INIT);
	com_homepath = CvarSystem::get("com_homepath", "", CVAR_INIT|CVAR_PROTECTED);

	FileSystem::initFilesystem ();

	Com_InitJournaling();

	// Add some commands here already so users can use them from config files
	CommandSystem::addCommand ("setenv", Com_Setenv_f);
	if (com_developer && com_developer->asInt())
	{
		CommandSystem::addCommand ("error", Com_Error_f);
		CommandSystem::addCommand ("crash", Com_Crash_f);
		CommandSystem::addCommand ("freeze", Com_Freeze_f);
	}
	CommandSystem::addCommand ("quit", Common::quit_f);
	CommandSystem::addCommand ("changeVectors", MSG_ReportChangeVectors_f );
	CommandSystem::addCommand ("writeconfig", Com_WriteConfig_f );
	CommandSystem::setCommandCompletionFunc( "writeconfig", CommandSystem::completeCfgName );
	CommandSystem::addCommand("game_restart", Com_GameRestart_f);

	Com_ExecuteCfg();

	// override anything from the config files with command line args
	Common::startupVariable( nullptr );

  // get dedicated here for proper hunk megs initialization
#ifdef DEDICATED
	com_dedicated = CvarSystem::get ("dedicated", "1", CVAR_INIT);
	CvarSystem::checkRange( com_dedicated, 1, 2, true );
#else
	com_dedicated = CvarSystem::get ("dedicated", "0", CVAR_LATCH);
	CvarSystem::checkRange( com_dedicated, 0, 2, true );
#endif
	// allocate the stack based hunk allocator
	Com_InitHunkMemory();

	// if any archived cvars are modified after this, we will trigger a writing
	// of the config file
	CvarSystem::clearModified( CVAR_ARCHIVE );

	//
	// init commands and vars
	//
	com_altivec = CvarSystem::get ("com_altivec", "1", CVAR_ARCHIVE);
#ifdef __EMSCRIPTEN__
	// Under Emscripten the browser handles throttling the frame rate.
	// Manual framerate throttling interacts poorly with Emscripten's
	// browser-driven event loop. So default throttling to off.
	com_maxfps = CvarSystem::get ("com_maxfps", "0", CVAR_ARCHIVE);
#else
	com_maxfps = CvarSystem::get ("com_maxfps", "85", CVAR_ARCHIVE);
#endif
	com_blood = CvarSystem::get ("com_blood", "1", CVAR_ARCHIVE);

	com_logfile = CvarSystem::get ("logfile", "0", CVAR_TEMP );

	com_timescale = CvarSystem::get ("timescale", "1", CVAR_CHEAT | CVAR_SYSTEMINFO );
	com_fixedtime = CvarSystem::get ("fixedtime", "0", CVAR_CHEAT);
	com_showtrace = CvarSystem::get ("com_showtrace", "0", CVAR_CHEAT);
	com_speeds = CvarSystem::get ("com_speeds", "0", 0);
	com_timedemo = CvarSystem::get ("timedemo", "0", CVAR_CHEAT);
	com_cameraMode = CvarSystem::get ("com_cameraMode", "0", CVAR_CHEAT);

	cl_paused = CvarSystem::get ("cl_paused", "0", CVAR_ROM);
	sv_paused = CvarSystem::get ("sv_paused", "0", CVAR_ROM);
	cl_packetdelay = CvarSystem::get ("cl_packetdelay", "0", CVAR_CHEAT);
	sv_packetdelay = CvarSystem::get ("sv_packetdelay", "0", CVAR_CHEAT);
	com_sv_running = CvarSystem::get ("sv_running", "0", CVAR_ROM);
	com_cl_running = CvarSystem::get ("cl_running", "0", CVAR_ROM);
	com_buildScript = CvarSystem::get( "com_buildScript", "0", 0 );
	com_ansiColor = CvarSystem::get( "com_ansiColor", "0", CVAR_ARCHIVE );

	com_unfocused = CvarSystem::get( "com_unfocused", "0", CVAR_ROM );
	com_maxfpsUnfocused = CvarSystem::get( "com_maxfpsUnfocused", "0", CVAR_ARCHIVE );
	com_minimized = CvarSystem::get( "com_minimized", "0", CVAR_ROM );
	com_maxfpsMinimized = CvarSystem::get( "com_maxfpsMinimized", "0", CVAR_ARCHIVE );
	com_abnormalExit = CvarSystem::get( "com_abnormalExit", "0", CVAR_ROM );
	com_busyWait = CvarSystem::get("com_busyWait", "0", CVAR_ARCHIVE);
	CvarSystem::get("com_errorMessage", "", CVAR_ROM | CVAR_NORESTART);

#ifdef CINEMATICS_INTRO
	com_introPlayed = CvarSystem::get( "com_introplayed", "0", CVAR_ARCHIVE);
#endif

	s = va("%s %s %s", Q3_VERSION, PLATFORM_STRING, PRODUCT_DATE );
	com_version = CvarSystem::get ("version", s, CVAR_ROM | CVAR_SERVERINFO );
	com_gamename = CvarSystem::get("com_gamename", GAMENAME_FOR_MASTER, CVAR_SERVERINFO | CVAR_INIT);
	com_protocol = CvarSystem::get("com_protocol", va("%i", PROTOCOL_VERSION), CVAR_SERVERINFO | CVAR_INIT);
#ifdef LEGACY_PROTOCOL
	com_legacyprotocol = CvarSystem::get("com_legacyprotocol", va("%i", PROTOCOL_LEGACY_VERSION), CVAR_INIT);

	// Keep for compatibility with old mods / mods that haven't updated yet.
	if(com_legacyprotocol->asInt() > 0)
		CvarSystem::get("protocol", com_legacyprotocol->asString(), CVAR_ROM);
	else
#endif
		CvarSystem::get("protocol", com_protocol->asString(), CVAR_ROM);

#ifndef DEDICATED
	con_autochat = CvarSystem::get("con_autochat", "1", CVAR_ARCHIVE);
#endif

	System::init();

	System::initPIDFile( FileSystem::getCurrentGameDir() );

	// Pick a random port value
	Com_RandomBytes( reinterpret_cast<byte*>(&qport), sizeof(int) );
	Netchan_Init( qport & 0xffff );

	ServerLoop::init();

	com_dedicated->modified = false;
#ifndef DEDICATED
	ClientLoop::init();
#endif

	// set com_frameTime so that if a map is started on the
	// command line it will still be able to count on com_frameTime
	// being random enough for a serverid
	com_frameTime = Common::milliseconds();

	// add + commands from command line
	if ( !Com_AddStartupCommands() ) {
		// if the user didn't give any commands, run default action
		if ( !com_dedicated->asInt() ) {
#ifdef CINEMATICS_LOGO
			CommandBuffer::addText ("cinematic " CINEMATICS_LOGO "\n");
#endif
#ifdef CINEMATICS_INTRO
			if( !com_introPlayed->asInt() ) {
				CvarSystem::set( com_introPlayed->name, "1" );
				CvarSystem::set( "nextmap", "cinematic " CINEMATICS_INTRO );
			}
#endif
		}
	}

	// start in full screen ui mode
	CvarSystem::set("r_uiFullScreen", "1");

	ClientLoop::startHunkUsers( false );

	// make sure single player is off by default
	CvarSystem::set("ui_singlePlayerActive", "0");

	com_fullyInitialized = true;

	// always set the cvar, but only print the info if it makes sense.
	Com_DetectAltivec();
#if idppc
	Common::printf ("Altivec support is %s\n", com_altivec->asInt() ? "enabled" : "disabled");
#endif

	com_pipefile = CvarSystem::get( "com_pipefile", "", CVAR_ARCHIVE|CVAR_LATCH );
	if( com_pipefile->asString()[0] )
	{
		pipefile = FileSystem::fCreateOpenPipeFile( com_pipefile->asString() );
	}

	Common::printf ("--- Common Initialization Complete ---\n");
}

/*
===============
Com_ReadFromPipe

Read whatever is in com_pipefile, if anything, and execute it
===============
*/
static void Com_ReadFromPipe( )
{
	static char buf[MAX_STRING_CHARS];
	static int accu = 0;
	int read;

	if( !pipefile )
		return;

	while( ( read = FileSystem::read( buf + accu, sizeof( buf ) - accu - 1, pipefile ) ) > 0 )
	{
		char *brk = nullptr;
		int i;

		for( i = accu; i < accu + read; ++i )
		{
			if( buf[ i ] == '\0' )
				buf[ i ] = '\n';
			if( buf[ i ] == '\n' || buf[ i ] == '\r' )
				brk = &buf[ i + 1 ];
		}
		buf[ accu + read ] = '\0';

		accu += read;

		if( brk )
		{
			char tmp = *brk;
			*brk = '\0';
			CommandBuffer::executeText( EXEC_APPEND, buf );
			*brk = tmp;

			accu -= brk - buf;
			memmove( buf, brk, accu + 1 );
		}
		else if( accu >= sizeof( buf ) - 1 ) // full
		{
			CommandBuffer::executeText( EXEC_APPEND, buf );
			accu = 0;
		}
	}
}


//==================================================================

static void Com_WriteConfigToFile( const char *filename ) {
	fileHandle_t	f;

	f = FileSystem::fOpenFileWrite_HomeConfig( filename );
	if ( !f ) {
		Common::printf ("Couldn't write %s.\n", filename );
		return;
	}

	FileSystem::printf (f, "// generated by " PRODUCT_NAME ", do not modify\n");
	Keys::writeBindings (f);
	CvarSystem::writeVariables (f);
	FileSystem::fCloseFile( f );
}


/*
===============
Com_WriteConfiguration

Writes key bindings and archived cvars to config file if modified
===============
*/
static void Com_WriteConfiguration( ) {
	// if we are quiting without fully initializing, make sure
	// we don't write out anything
	if ( !com_fullyInitialized ) {
		return;
	}

	if ( !(CvarSystem::modified( CVAR_ARCHIVE ) ) ) {
		return;
	}
	CvarSystem::clearModified( CVAR_ARCHIVE );

	Com_WriteConfigToFile( Q3CONFIG_CFG );

	// not needed for dedicated or standalone
#if !defined(DEDICATED) && !defined(STANDALONE)
	if(!com_standalone->asInt())
	{
		const char *gamedir;
		gamedir = CvarSystem::variableString( "fs_game" );
		if (UI_usesUniqueCDKey() && gamedir[0] != 0) {
			Com_WriteCDKey( gamedir, &cl_cdkey[16] );
		} else {
			Com_WriteCDKey( BASEGAME, cl_cdkey );
		}
	}
#endif
}


/*
===============
Com_WriteConfig_f

Write the config file to a specific name
===============
*/
static void Com_WriteConfig_f( ) {
	char	filename[MAX_QPATH];

	if ( CommandSystem::argc() != 2 ) {
		Common::printf( "Usage: writeconfig <filename>\n" );
		return;
	}

	Q_strncpyz( filename, CommandSystem::argv(1), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".cfg" );

	if (!COM_CompareExtension(filename, ".cfg"))
	{
		Common::printf("Com_WriteConfig_f: Only the \".cfg\" extension is supported by this command!\n");
		return;
	}

	Common::printf( "Writing %s.\n", filename );
	Com_WriteConfigToFile( filename );
}

/*
================
Com_ModifyMsec
================
*/
static int Com_ModifyMsec( int msec ) {
	int		clampTime;

	//
	// modify time for debugging values
	//
	if ( com_fixedtime->asInt() ) {
		msec = com_fixedtime->asInt();
	} else if ( com_timescale->asFloat() ) {
		msec *= com_timescale->asFloat();
	} else if (com_cameraMode->asInt()) {
		msec *= com_timescale->asFloat();
	}
	
	// don't let it scale below 1 msec
	if ( msec < 1 && com_timescale->asFloat()) {
		msec = 1;
	}

	if ( com_dedicated->asInt() ) {
		// dedicated servers don't want to clamp for a much longer
		// period, because it would mess up all the client's views
		// of time.
		if (com_sv_running->asInt() && msec > 500)
			Common::printf( "Hitch warning: %i msec frame time\n", msec );

		clampTime = 5000;
	} else 
	if ( !com_sv_running->asInt() ) {
		// clients of remote servers do not want to clamp time, because
		// it would skew their view of the server's time temporarily
		clampTime = 5000;
	} else {
		// for local single player gaming
		// we may want to clamp the time to prevent players from
		// flying off edges when something hitches.
		clampTime = 200;
	}

	if ( msec > clampTime ) {
		msec = clampTime;
	}

	return msec;
}

/*
=================
Com_TimeVal
=================
*/

static int Com_TimeVal(int minMsec)
{
	int timeVal;

	timeVal = System::milliseconds() - com_frameTime;

	if(timeVal >= minMsec)
		timeVal = 0;
	else
		timeVal = minMsec - timeVal;

	return timeVal;
}

/*
=================
Common::frame
=================
*/
void Common::frame( ) {

	int		msec, minMsec;
	int		timeVal, timeValSV;
	static int	lastTime = 0, bias = 0;
 
	int		timeBeforeFirstEvents;
	int		timeBeforeServer;
	int		timeBeforeEvents;
	int		timeBeforeClient;
	int		timeAfter;
  

	if ( setjmp (abortframe) ) {
		return;			// an ERR_DROP was thrown
	}

	timeBeforeFirstEvents =0;
	timeBeforeServer =0;
	timeBeforeEvents =0;
	timeBeforeClient = 0;
	timeAfter = 0;

	// write config file if anything changed
	Com_WriteConfiguration(); 

	//
	// main event loop
	//
	if ( com_speeds->asInt() ) {
		timeBeforeFirstEvents = System::milliseconds ();
	}

	// Figure out how much time we have
	if(!com_timedemo->asInt())
	{
		if(com_dedicated->asInt())
			minMsec = ServerLoop::frameMsec();
		else
		{
			if(com_minimized->asInt() && com_maxfpsMinimized->asInt() > 0)
				minMsec = 1000 / com_maxfpsMinimized->asInt();
			else if(com_unfocused->asInt() && com_maxfpsUnfocused->asInt() > 0)
				minMsec = 1000 / com_maxfpsUnfocused->asInt();
			else if(com_maxfps->asInt() > 0)
				minMsec = 1000 / com_maxfps->asInt();
			else
				minMsec = 1;
			
			timeVal = com_frameTime - lastTime;
			bias += timeVal - minMsec;
			
			if(bias > minMsec)
				bias = minMsec;
			
			// Adjust minMsec if previous frame took too long to render so
			// that framerate is stable at the requested value.
			minMsec -= bias;
		}
	}
	else
		minMsec = 1;

	do
	{
		if(com_sv_running->asInt())
		{
			timeValSV = ServerLoop::sendQueuedPackets();
			
			timeVal = Com_TimeVal(minMsec);

			if(timeValSV < timeVal)
				timeVal = timeValSV;
		}
		else
			timeVal = Com_TimeVal(minMsec);
		
		if(com_busyWait->asInt() || timeVal < 1)
			Network::sleep(0);
		else
			Network::sleep(timeVal - 1);
	} while(Com_TimeVal(minMsec));
	
	IN_Frame();

	lastTime = com_frameTime;
	com_frameTime = Common::eventLoop();
	
	msec = com_frameTime - lastTime;

	CommandBuffer::execute ();

	if (com_altivec->modified)
	{
		Com_DetectAltivec();
		com_altivec->modified = false;
	}

	// mess with msec if needed
	msec = Com_ModifyMsec(msec);

	//
	// server side
	//
	if ( com_speeds->asInt() ) {
		timeBeforeServer = System::milliseconds ();
	}

	ServerLoop::frame( msec );

	// if "dedicated" has been modified, start up
	// or shut down the client system.
	// Do this after the server may have started,
	// but before the client tries to auto-connect
	if ( com_dedicated->modified ) {
		// get the latched value
		CvarSystem::get( "dedicated", "0", 0 );
		com_dedicated->modified = false;
		if ( !com_dedicated->asInt() ) {
			ServerLoop::shutdown( "dedicated set to 0" );
			ClientLoop::flushMemory();
		}
	}

#ifndef DEDICATED
	//
	// client system
	//
	//
	// run event loop a second time to get server to client packets
	// without a frame of latency
	//
	if ( com_speeds->asInt() ) {
		timeBeforeEvents = System::milliseconds ();
	}
	Common::eventLoop();
	CommandBuffer::execute ();


	//
	// client side
	//
	if ( com_speeds->asInt() ) {
		timeBeforeClient = System::milliseconds ();
	}

	ClientLoop::frame( msec );

	if ( com_speeds->asInt() ) {
		timeAfter = System::milliseconds ();
	}
#else
	if ( com_speeds->asInt() ) {
		timeAfter = System::milliseconds ();
		timeBeforeEvents = timeAfter;
		timeBeforeClient = timeAfter;
	}
#endif


	Network::flushPacketQueue();

	//
	// report timing information
	//
	if ( com_speeds->asInt() ) {
		int			all, sv, ev, cl;

		all = timeAfter - timeBeforeServer;
		sv = timeBeforeEvents - timeBeforeServer;
		ev = timeBeforeServer - timeBeforeFirstEvents + timeBeforeClient - timeBeforeEvents;
		cl = timeAfter - timeBeforeClient;
		sv -= time_game;
		cl -= time_frontend + time_backend;

		Common::printf ("frame:%i all:%3i sv:%3i ev:%3i cl:%3i gm:%3i rf:%3i bk:%3i\n", 
					 com_frameNumber, all, sv, ev, cl, time_game, time_frontend, time_backend );
	}	

	//
	// trace optimization tracking
	//
	if ( com_showtrace->asInt() ) {
	
		extern	int c_traces, c_brush_traces, c_patch_traces;
		extern	int	c_pointcontents;

		Common::printf ("%4i traces  (%ib %ip) %4i points\n", c_traces,
			c_brush_traces, c_patch_traces, c_pointcontents);
		c_traces = 0;
		c_brush_traces = 0;
		c_patch_traces = 0;
		c_pointcontents = 0;
	}

	Com_ReadFromPipe( );

	com_frameNumber++;
}

/*
=================
Common::shutdown
=================
*/
void Common::shutdown () {
	if (logfile) {
		FileSystem::fCloseFile (logfile);
		logfile = 0;
	}

	if ( com_journalFile ) {
		FileSystem::fCloseFile( com_journalFile );
		com_journalFile = 0;
	}

	if( pipefile ) {
		FileSystem::fCloseFile( pipefile );
		FileSystem::remove_HomeData( com_pipefile->asString() );
	}

}

/*
===========================================
command line completion
===========================================
*/

/*
==================
Field_Clear
==================
*/
void EditField::clear() {
  memset(this->buffer, 0, MAX_EDIT_LINE);
	this->cursor = 0;
	this->scroll = 0;
}

static const char *completionString;
static char shortestMatch[MAX_TOKEN_CHARS];
static int	matchCount;
// field we are working on, passed to (&g_consoleCommand for instance)->autoComplete()
static EditField *completionField;

/*
===============
FindMatches

===============
*/
static void FindMatches( const char *s ) {
	int		i;

	if ( Q_stricmpn( s, completionString, strlen( completionString ) ) ) {
		return;
	}
	matchCount++;
	if ( matchCount == 1 ) {
		Q_strncpyz( shortestMatch, s, sizeof( shortestMatch ) );
		return;
	}

	// cut shortestMatch to the amount common with s
	for ( i = 0 ; shortestMatch[i] ; i++ ) {
		if ( i >= strlen( s ) ) {
			shortestMatch[i] = 0;
			break;
		}

		if ( tolower(shortestMatch[i]) != tolower(s[i]) ) {
			shortestMatch[i] = 0;
		}
	}
}

/*
===============
PrintMatches

===============
*/
static void PrintMatches( const char *s ) {
	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Common::printf( "    %s\n", s );
	}
}

/*
===============
PrintCvarMatches

===============
*/
static void PrintCvarMatches( const char *s ) {
	char value[ TRUNCATE_LENGTH ];

	if ( !Q_stricmpn( s, shortestMatch, strlen( shortestMatch ) ) ) {
		Com_TruncateLongString( value, CvarSystem::variableString( s ) );
		Common::printf( "    %s = \"%s\"\n", s, value );
	}
}

/*
===============
Field_FindFirstSeparator
===============
*/
static char *Field_FindFirstSeparator( char *s )
{
	int i;

	for( i = 0; i < strlen( s ); i++ )
	{
		if( s[ i ] == ';' )
			return &s[ i ];
	}

	return nullptr;
}

/*
===============
Field_Complete
===============
*/
static bool Field_Complete( )
{
	int completionOffset;

	if( matchCount == 0 )
		return true;

	completionOffset = strlen( completionField->buffer ) - strlen( completionString );

	Q_strncpyz( &completionField->buffer[ completionOffset ], shortestMatch,
		sizeof( completionField->buffer ) - completionOffset );

	completionField->cursor = strlen( completionField->buffer );

	if( matchCount == 1 )
	{
		Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
		completionField->cursor++;
		return true;
	}

	Common::printf( "]%s\n", completionField->buffer );

	return false;
}

#ifndef DEDICATED
/*
===============
Field_CompleteKeyname
===============
*/
void Field_CompleteKeyname( )
{
	matchCount = 0;
	shortestMatch[ 0 ] = 0;

	Key_KeynameCompletion( FindMatches );

	if( !Field_Complete( ) )
		Key_KeynameCompletion( PrintMatches );
}
#endif

/*
===============
Field_CompleteFilename
===============
*/
void Field_CompleteFilename( const char *dir, const char *ext,
		char *filter, bool stripExt,
		bool allowNonPureFilesOnDisk )
{
	matchCount = 0;
	shortestMatch[ 0 ] = 0;

	FileSystem::filenameCompletion( dir, ext, filter, stripExt, FindMatches, allowNonPureFilesOnDisk );

	if( !Field_Complete( ) )
		FileSystem::filenameCompletion( dir, ext, filter, stripExt, PrintMatches, allowNonPureFilesOnDisk );
}

/*
===============
Field_CompleteCommand
===============
*/
void Field_CompleteCommand( char *cmd,
		bool doCommands, bool doCvars )
{
	int		completionArgument = 0;

	// Skip leading whitespace and quotes
	cmd = Com_SkipCharset( cmd, " \"" );

	CommandSystem::tokenizeStringIgnoreQuotes( cmd );
	completionArgument = CommandSystem::argc( );

	// If there is trailing whitespace on the cmd
	if( *( cmd + strlen( cmd ) - 1 ) == ' ' )
	{
		completionString = "";
		completionArgument++;
	}
	else
		completionString = CommandSystem::argv( completionArgument - 1 );

#ifndef DEDICATED
	// add a '\' to the start of the buffer if it might be sent as chat otherwise
	if( con_autochat->asInt() && completionField->buffer[ 0 ] &&
			completionField->buffer[ 0 ] != '\\' )
	{
		if( completionField->buffer[ 0 ] != '/' )
		{
			// Buffer is full, refuse to complete
			if( strlen( completionField->buffer ) + 1 >=
				sizeof( completionField->buffer ) )
				return;

			memmove( &completionField->buffer[ 1 ],
				&completionField->buffer[ 0 ],
				strlen( completionField->buffer ) + 1 );
			completionField->cursor++;
		}

		completionField->buffer[ 0 ] = '\\';
	}
#endif

	if( completionArgument > 1 )
	{
		const char *baseCmd = CommandSystem::argv( 0 );
		char *p;

#ifndef DEDICATED
		// This should always be true
		if( baseCmd[ 0 ] == '\\' || baseCmd[ 0 ] == '/' )
			baseCmd++;
#endif

		if( ( p = Field_FindFirstSeparator( cmd ) ) )
			Field_CompleteCommand( p + 1, true, true ); // Compound command
		else
			CommandSystem::completeArgument( baseCmd, cmd, completionArgument ); 
	}
	else
	{
		if( completionString[0] == '\\' || completionString[0] == '/' )
			completionString++;

		matchCount = 0;
		shortestMatch[ 0 ] = 0;

		if( strlen( completionString ) == 0 )
			return;

		if( doCommands )
			CommandSystem::commandCompletion( FindMatches );

		if( doCvars )
			CvarSystem::commandCompletion( FindMatches );

		if( !Field_Complete( ) )
		{
			// run through again, printing matches
			if( doCommands )
				CommandSystem::commandCompletion( PrintMatches );

			if( doCvars )
				CvarSystem::commandCompletion( PrintCvarMatches );
		}
	}
}

/*
===============
Field_AutoComplete

Perform Tab expansion
===============
*/
void EditField::autoComplete()
{
	completionField = this;

	Field_CompleteCommand( completionField->buffer, true, true );
}

/*
==================
Com_RandomBytes

fills string array with len random bytes, preferably from the OS randomizer
==================
*/
void Com_RandomBytes( byte *string, int len )
{
	int i;

	if( System::randomBytes( string, len ) )
		return;

	Common::printf( "Com_RandomBytes: using weak randomization\n" );
	for( i = 0; i < len; i++ )
		string[i] = static_cast<unsigned char>( rand() % 256 );
}


/*
==================
Common::isVoipTarget

Returns non-zero if given clientNum is enabled in voipTargets, zero otherwise.
If clientNum is negative return if any bit is set.
==================
*/
bool Common::isVoipTarget(uint8_t *voipTargets, int voipTargetsSize, int clientNum)
{
	int index;
	if(clientNum < 0)
	{
		for(index = 0; index < voipTargetsSize; index++)
		{
			if(voipTargets[index])
				return true;
		}
		
		return false;
	}

	index = clientNum >> 3;
	
	if(index < voipTargetsSize)
		return (voipTargets[index] & (1 << (clientNum & 0x07)));

	return false;
}

/*
===============
Field_CompletePlayerName
===============
*/
static bool Field_CompletePlayerNameFinal( bool whitespace )
{
	int completionOffset;

	if( matchCount == 0 )
		return true;

	completionOffset = strlen( completionField->buffer ) - strlen( completionString );

	Q_strncpyz( &completionField->buffer[ completionOffset ], shortestMatch,
		sizeof( completionField->buffer ) - completionOffset );

	completionField->cursor = strlen( completionField->buffer );

	if( matchCount == 1 && whitespace )
	{
		Q_strcat( completionField->buffer, sizeof( completionField->buffer ), " " );
		completionField->cursor++;
		return true;
	}

	return false;
}

static void Name_PlayerNameCompletion( const char **names, int nameCount, void(*callback)(const char *s) ) 
{
	int i;

	for( i = 0; i < nameCount; i++ ) {
		callback( names[ i ] );
	}
}

bool Common::fieldStringToPlayerName( char *name, int length, const char *rawname )
{
	char		hex[5];
	int			i;
	int			ch;

	if( name == nullptr || rawname == nullptr )
		return false;

	if( length <= 0 )
		return true;

	for( i = 0; *rawname && i + 1 <= length; rawname++, i++ ) {
		if( *rawname == '\\' ) {
			Q_strncpyz( hex, rawname + 1, sizeof(hex) );
			ch = Com_HexStrToInt( hex );
			if( ch > -1 ) {
				name[i] = ch;
				rawname += 4; //hex string length, 0xXX
			} else {
				name[i] = *rawname;
			}
		} else {
			name[i] = *rawname;
		}
	}
	name[i] = '\0';

	return true;
}

bool Common::playerNameToFieldString( char *str, int length, const char *name )
{
	const char *p;
	int i;
	int x1, x2;

	if( str == nullptr || name == nullptr )
		return false;

	if( length <= 0 )
		return true;

	*str = '\0';
	p = name;

	for( i = 0; *p != '\0'; i++, p++ )
	{
		if( i + 1 >= length )
			break;

		if( *p <= ' ' )
		{
			if( i + 5 + 1 >= length )
				break;

			x1 = *p >> 4;
			x2 = *p & 15;

			str[i+0] = '\\';
			str[i+1] = '0';
			str[i+2] = 'x';
			str[i+3] = x1 > 9 ? x1 - 10 + 'a' : x1 + '0';
			str[i+4] = x2 > 9 ? x2 - 10 + 'a' : x2 + '0';

			i += 4;
		} else {
			str[i] = *p;
		}		
	}
	str[i] = '\0';

	return true;
}

void Field_CompletePlayerName( const char **names, int nameCount )
{
	bool whitespace;

	matchCount = 0;
	shortestMatch[ 0 ] = 0;

	if( nameCount <= 0 )
		return;

	Name_PlayerNameCompletion( names, nameCount, FindMatches );

	if( completionString[0] == '\0' )
	{
		Common::playerNameToFieldString( shortestMatch, sizeof( shortestMatch ), names[ 0 ] );
	}

	//allow to tab player names
	//if full player name switch to next player name
	if( completionString[0] != '\0'
		&& Q_stricmp( shortestMatch, completionString ) == 0 
		&& nameCount > 1 ) 
	{
		int i;

		for( i = 0; i < nameCount; i++ ) {
			if( Q_stricmp( names[ i ], completionString ) == 0 ) 
			{
				i++;
				if( i >= nameCount )
				{
					i = 0;
				}

				Common::playerNameToFieldString( shortestMatch, sizeof( shortestMatch ), names[ i ] );
				break;
			}
		}
	}

	if( matchCount > 1 )
	{
		Common::printf( "]%s\n", completionField->buffer );
		
		Name_PlayerNameCompletion( names, nameCount, PrintMatches );
	}

	whitespace = nameCount == 1? true: false;
	if( !Field_CompletePlayerNameFinal( whitespace ) )
	{

	}
}

int QDECL Common::strCompare( const void *a, const void *b )
{
    const char * const *pa = static_cast<const char * const *>(a);
    const char * const *pb = static_cast<const char * const *>(b);
    return strcmp( *pa, *pb );
}
