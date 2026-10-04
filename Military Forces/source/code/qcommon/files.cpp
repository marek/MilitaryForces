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
/*****************************************************************************
 * name:		files.c
 *
 * desc:		handle based filesystem for Quake III Arena 
 *
 * $Archive: /MissionPack/code/qcommon/files.c $
 *
 *****************************************************************************/


#include "../game/q_shared.h"
#include "qcommon.h"
#include "unzip.h"

/*
=============================================================================

QUAKE3 FILESYSTEM

All of Quake's data access is through a hierarchical file system, but the contents of 
the file system can be transparently merged from several sources.

A "qpath" is a reference to game file data.  MAX_ZPATH is 256 characters, which must include
a terminating zero. "..", "\\", and ":" are explicitly illegal in qpaths to prevent any
references outside the quake directory system.

The "base path" is the path to the directory holding all the game directories and usually
the executable.  It defaults to ".", but can be overridden with a "+set fs_basepath c:\quake3"
command line to allow code debugging in a different directory.  Basepath cannot
be modified at all after startup.  Any files that are created (demos, screenshots,
etc) will be created relative to the base path, so base path should usually be writable.

The "home path" is the path used for all write access. On win32 systems we have "base path"
== "home path", but on *nix systems the base installation is usually readonly, and
"home path" points to ~/.q3a or similar

The user can also install custom mods and content in "home path", so it should be searched
along with "home path" and "cd path" for game content.


The "base game" is the directory under the paths where data comes from by default, and
can be either "baseq3" or "demoq3".

The "current game" may be the same as the base game, or it may be the name of another
directory under the paths that should be searched for files before looking in the base game.
This is the basis for addons.

Clients automatically set the game directory after receiving a gamestate from a server,
so only servers need to worry about +set fs_game.

No other directories outside of the base game and current game will ever be referenced by
filesystem functions.

To save disk space and speed loading, directory trees can be collapsed into zip files.
The files use a ".pk3" extension to prevent users from unzipping them accidentally, but
otherwise the are simply normal uncompressed zip files.  A game directory can have multiple
zip files of the form "pak0.pk3", "pak1.pk3", etc.  Zip files are searched in decending order
from the highest number to the lowest, and will always take precedence over the filesystem.
This allows a pk3 distributed as a patch to override all existing data.

Because we will have updated executables freely available online, there is no point to
trying to restrict demo / oem versions of the game with code changes.  Demo / oem versions
should be exactly the same executables as release versions, but with different data that
automatically restricts where game media can come from to prevent add-ons from working.

File search order: when FileSystem::fOpenFileRead gets called it will go through the fs_searchpaths
structure and stop on the first successful hit. fs_searchpaths is built with successive
calls to FS_AddGameDirectory

Additionally, we search in several subdirectories:
current game is the current mode
base game is a variable to allow mods based on other mods
(such as baseq3 + missionpack content combination in a mod for instance)
BASEGAME is the hardcoded base game ("baseq3")

e.g. the qpath "sound/newstuff/test.wav" would be searched for in the following places:

home path + current game's zip files
home path + current game's directory
base path + current game's zip files
base path + current game's directory
cd path + current game's zip files
cd path + current game's directory

home path + base game's zip file
home path + base game's directory
base path + base game's zip file
base path + base game's directory
cd path + base game's zip file
cd path + base game's directory

home path + BASEGAME's zip file
home path + BASEGAME's directory
base path + BASEGAME's zip file
base path + BASEGAME's directory
cd path + BASEGAME's zip file
cd path + BASEGAME's directory

server download, to be written to home path + current game's directory


The filesystem can be safely shutdown and reinitialized with different
basedir / cddir / game combinations, but all other subsystems that rely on it
(sound, video) must also be forced to restart.

Because the same files are loaded by both the clip model (CM_) and renderer (TR_)
subsystems, a simple single-file caching scheme is used.  The CM_ subsystems will
load the file with a request to cache.  Only one file will be kept cached at a time,
so any models that are going to be referenced by both subsystems should alternate
between the CM_ load function and the ref load function.

TODO: A qpath that starts with a leading slash will always refer to the base game, even if another
game is currently active.  This allows character models, skins, and sounds to be downloaded
to a common directory no matter which game is active.

How to prevent downloading zip files?
Pass pk3 file names in systeminfo, and download before FileSystem::restart()?

Aborting a download disconnects the client from the server.

How to mark files as downloadable?  Commercial add-ons won't be downloadable.

Non-commercial downloads will want to download the entire zip file.
the game would have to be reset to actually read the zip in

Auto-update information

Path separators

Casing

  separate server gamedir and client gamedir, so if the user starts
  a local game after having connected to a network game, it won't stick
  with the network game.

  allow menu options for game selection?

Read / write config to floppy option.

Different version coexistance?

When building a pak file, make sure a q3config.cfg isn't present in it,
or configs will never get loaded from disk!

  todo:

  downloading (outside fs?)
  game directory passing and restarting

=============================================================================

*/

// every time a new demo pk3 file is built, this checksum must be updated.
// the easiest way to get it is to just run the game and see what it spits out
#ifndef STANDALONE
constexpr unsigned int DEMO_PAK0_CHECKSUM = 2985612116u;
static const unsigned int pak_checksums[] = {
	1566731103u,
	298122907u,
	412165236u,
	2991495316u,
	1197932710u,
	4087071573u,
	3709064859u,
	908855077u,
	977125798u
};

static const unsigned int missionpak_checksums[] =
{
	2430342401u,
	511014160u,
	2662638993u,
	1438664554u
};
#endif

// if this is defined, the executable positively won't work with any paks other
// than the demo pak, even if productid is present.  This is only used for our
// last demo release to prevent the mac and linux users from using the demo
// executable with the production windows pak before the mac/linux products
// hit the shelves a little later
// NOW defined in build files
//#define PRE_RELEASE_TADEMO

constexpr int MAX_ZPATH = 256;
constexpr int MAX_SEARCH_PATHS = 4096;
constexpr int MAX_FILEHASH_SIZE = 1024;

struct fileInPack_s {
	char					*name;		// name of the file
	unsigned long			pos;		// file info position in zip
	unsigned long			len;		// uncompress file size
	struct	fileInPack_s*	next;		// next file in the hash
};
using fileInPack_t = fileInPack_s;

struct pack_t {
	char			pakPathname[MAX_OSPATH];	// c:\quake3\baseq3
	char			pakFilename[MAX_OSPATH];	// c:\quake3\baseq3\pak0.pk3
	char			pakBasename[MAX_OSPATH];	// pak0
	char			pakGamename[MAX_OSPATH];	// baseq3
	unzFile			handle;						// handle to zip file
	unsigned int	checksum;					// regular checksum
	int				pure_checksum;				// checksum for pure
	int				numfiles;					// number of files in pk3
	int				referenced;					// referenced file flags
	int				hashSize;					// hash table size (power of 2)
	fileInPack_t*	*hashTable;					// hash table
	fileInPack_t*	buildBuffer;				// buffer with the filenames etc.
};

struct directory_t {
	char		path[MAX_OSPATH];		// c:\quake3
	char		fullpath[MAX_OSPATH];		// c:\quake3\baseq3
	char		gamedir[MAX_OSPATH];	// baseq3
};

struct searchpath_s {
	struct searchpath_s *next;

	pack_t		*pack;		// only one of pack / dir will be non NULL
	directory_t	*dir;
};
using searchpath_t = searchpath_s;

static	char		fs_gamedir[MAX_OSPATH];	// this will be a single file name with no separators
static	cvar_t		*fs_debug;
static	cvar_t		*fs_homeconfigpath;
static	cvar_t		*fs_homedatapath;
static	cvar_t		*fs_homestatepath;

static	cvar_t		*fs_apppath;
static	cvar_t		*fs_steampath;
static	cvar_t		*fs_gogpath;
static	cvar_t		*fs_microsoftstorepath;

static	cvar_t		*fs_basepath;
static	cvar_t		*fs_basegame;
static	cvar_t		*fs_gamedirvar;
static	searchpath_t	*fs_searchpaths;
static	int			fs_readCount;			// total bytes read
static	int			fs_loadCount;			// total files read
static	int			fs_loadStack;			// total files in memory
static	int			fs_packFiles = 0;		// total number of files in packs

static cvar_t *fs_pathVars[16];

static int fs_checksumFeed;

union qfile_gus {
	FILE*		o;
	unzFile		z;
};
using qfile_gut = qfile_gus;

struct qfile_us {
	qfile_gut	file;
	bool	unique;
};
using qfile_ut = qfile_us;

struct fileHandleData_t {
	qfile_ut	handleFiles;
	bool	handleSync;
	int			fileSize;
	int			zipFilePos;
	int			zipFileLen;
	bool	zipFile;
	char		name[MAX_ZPATH];
};

static fileHandleData_t	fsh[MAX_FILE_HANDLES];

// TTimo - https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=540
// wether we did a reorder on the current search path when joining the server
static bool fs_reordered;

// never load anything from pk3 files that are not present at the server when pure
static int			fs_numServerPaks = 0;
static unsigned int	fs_serverPaks[MAX_SEARCH_PATHS];				// checksums
static char			*fs_serverPakNames[MAX_SEARCH_PATHS];			// pk3 names

// only used for autodownload, to make sure the client has at least
// all the pk3 files that are referenced at the server side
static int			fs_numServerReferencedPaks;
static unsigned int	fs_serverReferencedPaks[MAX_SEARCH_PATHS];			// checksums
static char			*fs_serverReferencedPakNames[MAX_SEARCH_PATHS];		// pk3 names

// last valid game folder used
char lastValidBase[MAX_OSPATH];
char lastValidComBaseGame[MAX_OSPATH];
char lastValidFsBaseGame[MAX_OSPATH];
char lastValidGame[MAX_OSPATH];

#ifdef FS_MISSING
FILE*		missingFiles = NULL;
#endif

/* C99 defines __func__ */
#if __STDC_VERSION__ < 199901L
#  if __GNUC__ >= 2 || _MSC_VER >= 1300
#    define __func__ __FUNCTION__
#  else
#    define __func__ "(unknown)"
#  endif
#endif

/*
==============
FileSystem::initialized
==============
*/

bool FileSystem::initialized( ) {
	return (fs_searchpaths != nullptr);
}

/*
=================
FS_PakIsPure
=================
*/
bool FS_PakIsPure( pack_t *pack ) {
	int i;

	if ( fs_numServerPaks ) {
		for ( i = 0 ; i < fs_numServerPaks ; i++ ) {
			// FIXME: also use hashed file names
			// NOTE TTimo: a pk3 with same checksum but different name would be validated too
			//   I don't see this as allowing for any exploit, it would only happen if the client does manips of its file names 'not a bug'
			if ( pack->checksum == fs_serverPaks[i] ) {
				return true;		// on the aproved list
			}
		}
		return false;	// not on the pure server pak list
	}
	return true;
}


/*
=================
FileSystem::loadStack
return load stack
=================
*/
int FileSystem::loadStack( )
{
	return fs_loadStack;
}

/*
================
return a hash value for the filename
================
*/
static long FS_HashFileName( const char *fname, int hashSize ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		if (letter =='.') break;				// don't include extension
		if (letter =='\\') letter = '/';		// damn path names
		if (letter == PATH_SEP) letter = '/';		// damn path names
		hash+=static_cast<long>(letter)*(i+119);
		i++;
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	hash &= (hashSize-1);
	return hash;
}

static fileHandle_t	FS_HandleForFile() {
	int		i;

	for ( i = 1 ; i < MAX_FILE_HANDLES ; i++ ) {
		if ( fsh[i].handleFiles.file.o == nullptr ) {
			return i;
		}
	}
	Com_Error( ERR_DROP, "FS_HandleForFile: none free" );
	return 0;
}

static FILE	*FS_FileForHandle( fileHandle_t f ) {
	if ( f < 1 || f >= MAX_FILE_HANDLES ) {
		Com_Error( ERR_DROP, "FS_FileForHandle: out of range" );
	}
	if (fsh[f].zipFile == true) {
		Com_Error( ERR_DROP, "FS_FileForHandle: can't get FILE on zip file" );
	}
	if ( ! fsh[f].handleFiles.file.o ) {
		Com_Error( ERR_DROP, "FS_FileForHandle: NULL" );
	}
	
	return fsh[f].handleFiles.file.o;
}

void	FileSystem::forceFlush( fileHandle_t f ) {
	FILE *file;

	file = FS_FileForHandle(f);
	setvbuf( file, nullptr, _IONBF, 0 );
}

/*
================
FS_fplength
================
*/

long FS_fplength(FILE *h)
{
	long		pos;
	long		end;

	pos = ftell(h);
	fseek(h, 0, SEEK_END);
	end = ftell(h);
	fseek(h, pos, SEEK_SET);

	return end;
}

/*
================
FileSystem::filelength

If this is called on a non-unique FILE (from a pak file),
it will return the size of the pak file, not the expected
size of the file.
================
*/
long FileSystem::filelength(fileHandle_t f)
{
	FILE	*h;

	h = FS_FileForHandle(f);
	
	if(h == nullptr)
		return -1;
	else
		return FS_fplength(h);
}

/*
====================
FS_ReplaceSeparators

Fix things up differently for win/unix/mac
====================
*/
static void FS_ReplaceSeparators( char *path ) {
	char	*s;
	bool lastCharWasSep = false;

	for ( s = path ; *s ; s++ ) {
		if ( *s == '/' || *s == '\\' ) {
			if ( !lastCharWasSep ) {
				*s = PATH_SEP;
				lastCharWasSep = true;
			} else {
				memmove (s, s + 1, strlen (s));
			}
		} else {
			lastCharWasSep = false;
		}
	}
}

/*
===================
FileSystem::buildOSPath

Qpath may have either forward or backwards slashes
===================
*/
char *FileSystem::buildOSPath( const char *base, const char *game, const char *qpath ) {
	char	temp[MAX_OSPATH];
	static char ospath[2][MAX_OSPATH];
	static int toggle;
	
	toggle ^= 1;		// flip-flop to allow two returns without clash

	if( !game || !game[0] ) {
		game = fs_gamedir;
	}

	Com_sprintf( temp, sizeof(temp), "/%s/%s", game, qpath );
	FS_ReplaceSeparators( temp );
	Com_sprintf( ospath[toggle], sizeof( ospath[0] ), "%s%s", base, temp );
	
	return ospath[toggle];
}

/*
===================
FileSystem::baseDir_BuildOSPath

Qpath may have either forward or backwards slashes
===================
*/
char *FileSystem::baseDir_BuildOSPath( const char *base, const char *qpath ) {
	char *ospath = FileSystem::buildOSPath(base, qpath, "");
	ospath[strlen(ospath) - 1] = '\0';

	return ospath;
}


/*
============
FileSystem::createPath

Creates any directories needed to store the given filename
============
*/
bool FileSystem::createPath (const char *OSPath) {
	char	*ofs;
	char	path[MAX_OSPATH];
	
	if (!OSPath || !*OSPath) {
		return false;
	}

	// make absolutely sure that it can't back up the path
	// FIXME: is c: allowed???
	if ( strstr( OSPath, ".." ) || strstr( OSPath, "::" ) ) {
		Com_Printf( "WARNING: refusing to create relative path \"%s\"\n", OSPath );
		return true;
	}

	Q_strncpyz( path, OSPath, sizeof( path ) );
	FS_ReplaceSeparators( path );

	// Skip creation of the root directory as it will always be there
	ofs = strchr( path, PATH_SEP );
	if ( ofs != nullptr ) {
		ofs++;
	}

	for (; ofs != nullptr && *ofs ; ofs++) {
		if (*ofs == PATH_SEP) {
			// create the directory
			*ofs = 0;
			if (!Sys_Mkdir (path)) {
				Com_Error( ERR_FATAL, "FileSystem::createPath: failed to create path \"%s\"",
					path );
			}
			*ofs = PATH_SEP;
		}
	}

	return false;
}

/*
=================
FS_CheckFilenameIsMutable

ERR_FATAL if trying to maniuplate a file with the platform library, QVM, or pk3 extension
=================
 */
static void FS_CheckFilenameIsMutable( const char *filename,
		const char *function )
{
	// Check if the filename ends with the library, QVM, or pk3 extension
	if( Sys_DllExtension( filename )
		|| COM_CompareExtension( filename, ".qvm" )
		|| COM_CompareExtension( filename, ".pk3" ) )
	{
		Com_Error( ERR_FATAL, "%s: Not allowed to manipulate '%s' due "
			"to %s extension", function, filename, COM_GetExtension( filename ) );
	}
}

/*
===========
FileSystem::remove

===========
*/
void FileSystem::remove( const char *osPath ) {
	FS_CheckFilenameIsMutable( osPath, __func__ );

	remove( osPath );
}

/*
===========
FileSystem::remove_HomeData

===========
*/
void FileSystem::remove_HomeData( const char *homePath ) {
	FS_CheckFilenameIsMutable( homePath, __func__ );

	remove( FileSystem::buildOSPath( fs_homedatapath->string,
			fs_gamedir, homePath ) );
}

/*
================
FS_FileInPathExists

Tests if path and file exists
================
*/
bool FS_FileInPathExists(const char *testpath)
{
	FILE *filep;

	filep = Sys_FOpen(testpath, "rb");
	
	if(filep)
	{
		fclose(filep);
		return true;
	}
	
	return false;
}

/*
================
FileSystem::fileExists_HomeData

Tests if the file exists in the current gamedir, this DOES NOT
search the paths.  This is to determine if opening a file to write
(which always goes into the current gamedir) will cause any overwrites.
NOTE TTimo: this goes with FS_FOpenFileWrite for opening the file afterwards
================
*/
bool FileSystem::fileExists_HomeData(const char *file)
{
	return FS_FileInPathExists(FileSystem::buildOSPath(fs_homedatapath->string, fs_gamedir, file));
}

/*
================
FS_BaseDir_FileExists_HomeData

Tests if the file exists 
================
*/
static bool FS_BaseDir_FileExists_HomeData( const char *file )
{
	return FS_FileInPathExists(FileSystem::baseDir_BuildOSPath(fs_homedatapath->string, file));
}


/*
===========
FS_OSPath_FOpenFileWrite
===========
*/
static fileHandle_t FS_OSPath_FOpenFileWrite( const char *ospath, const char *filename ) {
	fileHandle_t	f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	f = FS_HandleForFile();
	fsh[f].zipFile = false;

	if ( fs_debug->integer ) {
		Com_Printf( "FS_OSPath_FOpenFileWrite: %s\n", ospath );
	}

	FS_CheckFilenameIsMutable( ospath, __func__ );

	if( FileSystem::createPath( ospath ) ) {
		return 0;
	}

	fsh[f].handleFiles.file.o = Sys_FOpen( ospath, "wb" );

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	fsh[f].handleSync = false;
	if (!fsh[f].handleFiles.file.o) {
		f = 0;
	}
	return f;
}

/*
===========
FileSystem::baseDir_FOpenFileWrite_HomeConfig
===========
*/
fileHandle_t FileSystem::baseDir_FOpenFileWrite_HomeConfig( const char *filename ) {
	return FS_OSPath_FOpenFileWrite(
		FileSystem::baseDir_BuildOSPath(fs_homeconfigpath->string, filename), filename);
}

/*
===========
FileSystem::baseDir_FOpenFileWrite_HomeData
===========
*/
fileHandle_t FileSystem::baseDir_FOpenFileWrite_HomeData( const char *filename ) {
	return FS_OSPath_FOpenFileWrite(
		FileSystem::baseDir_BuildOSPath(fs_homedatapath->string, filename), filename);
}

/*
===========
FileSystem::baseDir_FOpenFileWrite_HomeState
===========
*/
fileHandle_t FileSystem::baseDir_FOpenFileWrite_HomeState( const char *filename ) {
	return FS_OSPath_FOpenFileWrite(
		FileSystem::baseDir_BuildOSPath(fs_homestatepath->string, filename), filename);
}

/*
===========
FileSystem::baseDir_FOpenFileRead

Search for a file somewhere below the home path then base path
in that order
===========
*/
long FileSystem::baseDir_FOpenFileRead(const char *filename, fileHandle_t *fp)
{
	char *ospath;
	fileHandle_t f = 0;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	f = FS_HandleForFile();
	fsh[f].zipFile = false;

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	// don't let sound stutter
	SoundSystem::clearSoundBuffer();

	for(int i = 0; i < ARRAY_LEN( fs_pathVars ) && !fsh[f].handleFiles.file.o; i++) {
		const cvar_t *pathVar = fs_pathVars[i];

		if (!pathVar || !pathVar->string[0]) {
			continue;
		}

		ospath = FileSystem::baseDir_BuildOSPath( pathVar->string, filename );

		if ( fs_debug->integer )
		{
			Com_Printf( "FileSystem::baseDir_FOpenFileRead (%s): %s\n", pathVar->name, ospath );
		}

		fsh[f].handleFiles.file.o = Sys_FOpen( ospath, "rb" );
		fsh[f].handleSync = false;
	}

	if (!fsh[f].handleFiles.file.o)
	{
		f = 0;
	}

	*fp = f;
	if (f) {
		return FileSystem::filelength(f);
	}

	return -1;
}


/*
===========
FileSystem::baseDir_Rename_HomeData

===========
*/
void FileSystem::baseDir_Rename_HomeData( const char *from, const char *to, bool safe ) {
	char			*from_ospath, *to_ospath;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	// don't let sound stutter
	SoundSystem::clearSoundBuffer();

	from_ospath = FileSystem::baseDir_BuildOSPath( fs_homedatapath->string, from );
	to_ospath = FileSystem::baseDir_BuildOSPath( fs_homedatapath->string, to );

	if ( fs_debug->integer ) {
		Com_Printf( "FileSystem::baseDir_Rename_HomeData: %s --> %s\n", from_ospath, to_ospath );
	}

	if ( safe ) {
		FS_CheckFilenameIsMutable( to_ospath, __func__ );
	}

	rename(from_ospath, to_ospath);
}

/*
==============
FileSystem::fCloseFile

If the FILE pointer is an open pak file, leave it open.

For some reason, other dll's can't just cal fclose()
on files returned by FS_FOpenFile...
==============
*/
void FileSystem::fCloseFile( fileHandle_t f ) {
	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if (fsh[f].zipFile == true) {
		unzCloseCurrentFile( fsh[f].handleFiles.file.z );
		if ( fsh[f].handleFiles.unique ) {
			unzClose( fsh[f].handleFiles.file.z );
		}
		Com_Memset( &fsh[f], 0, sizeof( fsh[f] ) );
		return;
	}

	// we didn't find it as a pak, so close it as a unique file
	if (fsh[f].handleFiles.file.o) {
		fclose (fsh[f].handleFiles.file.o);
	}
	Com_Memset( &fsh[f], 0, sizeof( fsh[f] ) );
}

/*
===========
FileSystem::fOpenFileWrite_HomeConfig
===========
*/
fileHandle_t FileSystem::fOpenFileWrite_HomeConfig( const char *filename ) {
	return FS_OSPath_FOpenFileWrite(
		FileSystem::buildOSPath(fs_homeconfigpath->string, fs_gamedir, filename), filename);
}

/*
===========
FileSystem::fOpenFileWrite_HomeData
===========
*/
fileHandle_t FileSystem::fOpenFileWrite_HomeData( const char *filename ) {
	return FS_OSPath_FOpenFileWrite(
		FileSystem::buildOSPath(fs_homedatapath->string, fs_gamedir, filename), filename);
}

/*
===========
FileSystem::fOpenFileWrite_HomeState
===========
*/
fileHandle_t FileSystem::fOpenFileWrite_HomeState( const char *filename ) {
	return FS_OSPath_FOpenFileWrite(
		FileSystem::buildOSPath(fs_homestatepath->string, fs_gamedir, filename), filename);
}

/*
===========
FileSystem::fOpenFileAppend_HomeData

===========
*/
fileHandle_t FileSystem::fOpenFileAppend_HomeData( const char *filename ) {
	char			*ospath;
	fileHandle_t	f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	f = FS_HandleForFile();
	fsh[f].zipFile = false;

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	// don't let sound stutter
	SoundSystem::clearSoundBuffer();

	ospath = FileSystem::buildOSPath( fs_homedatapath->string, fs_gamedir, filename );

	if ( fs_debug->integer ) {
		Com_Printf( "FileSystem::fOpenFileAppend_HomeData: %s\n", ospath );
	}

	FS_CheckFilenameIsMutable( ospath, __func__ );

	if( FileSystem::createPath( ospath ) ) {
		return 0;
	}

	fsh[f].handleFiles.file.o = Sys_FOpen( ospath, "ab" );
	fsh[f].handleSync = false;
	if (!fsh[f].handleFiles.file.o) {
		f = 0;
	}
	return f;
}

/*
===========
FileSystem::fCreateOpenPipeFile

===========
*/
fileHandle_t FileSystem::fCreateOpenPipeFile( const char *filename ) {
	char	    		*ospath;
	FILE					*fifo;
	fileHandle_t	f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	f = FS_HandleForFile();
	fsh[f].zipFile = false;

	Q_strncpyz( fsh[f].name, filename, sizeof( fsh[f].name ) );

	// don't let sound stutter
	SoundSystem::clearSoundBuffer();

	ospath = FileSystem::buildOSPath( fs_homedatapath->string, fs_gamedir, filename );

	if ( fs_debug->integer ) {
		Com_Printf( "FileSystem::fCreateOpenPipeFile: %s\n", ospath );
	}

	FS_CheckFilenameIsMutable( ospath, __func__ );

	fifo = Sys_Mkfifo( ospath );
	if( fifo ) {
		fsh[f].handleFiles.file.o = fifo;
		fsh[f].handleSync = false;
	}
	else
	{
		Com_Printf( S_COLOR_YELLOW "WARNING: Could not create new com_pipefile at %s. "
			"com_pipefile will not be used.\n", ospath );
		f = 0;
	}

	return f;
}

/*
===========
FileSystem::filenameCompare

Ignore case and seprator char distinctions
===========
*/
bool FileSystem::filenameCompare( const char *s1, const char *s2 ) {
	int		c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (c1 >= 'a' && c1 <= 'z') {
			c1 -= ('a' - 'A');
		}
		if (c2 >= 'a' && c2 <= 'z') {
			c2 -= ('a' - 'A');
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}

		if (c1 != c2) {
			return true;		// strings not equal
		}
	} while (c1);

	return false;		// strings are equal
}

/*
===========
FS_IsExt

Return true if ext matches file extension filename
===========
*/

bool FS_IsExt(const char *filename, const char *ext, int namelen)
{
	int extlen;

	extlen = strlen(ext);

	if(extlen > namelen)
		return false;

	filename += namelen - extlen;

	return !Q_stricmp(filename, ext);
}

/*
===========
FS_IsDemoExt

Return true if filename has a demo extension
===========
*/

bool FS_IsDemoExt(const char *filename, int namelen)
{
	const char *ext_test;
	int index, protocol;

	ext_test = strrchr(filename, '.');
	if(ext_test && !Q_stricmpn(ext_test + 1, DEMOEXT, ARRAY_LEN(DEMOEXT) - 1))
	{
		protocol = atoi(ext_test + ARRAY_LEN(DEMOEXT));

		if(protocol == com_protocol->integer)
			return true;

#ifdef LEGACY_PROTOCOL
		if(protocol == com_legacyprotocol->integer)
			return true;
#endif

		for(index = 0; demo_protocols[index]; index++)
		{
			if(demo_protocols[index] == protocol)
			return true;
		}
	}

	return false;
}

/*
===========
FS_FOpenFileReadDir

Tries opening file "filename" in searchpath "search"
Returns filesize and an open FILE pointer.
===========
*/
extern bool		com_fullyInitialized;

long FS_FOpenFileReadDir(const char *filename, searchpath_t *search, fileHandle_t *file, bool uniqueFILE, bool unpure)
{
	long			hash;
	pack_t		*pak;
	fileInPack_t	*pakFile;
	directory_t	*dir;
	char		*netpath;
	FILE		*filep;
	int			len;

	if(filename == nullptr)
		Com_Error(ERR_FATAL, "FileSystem::fOpenFileRead: NULL 'filename' parameter passed");

	// qpaths are not supposed to have a leading slash
	if(filename[0] == '/' || filename[0] == '\\')
		filename++;

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo" 
	if(strstr(filename, ".." ) || strstr(filename, "::"))
	{
		if(file == nullptr)
			return false;

		*file = 0;
		return -1;
	}

	// make sure the q3key file is only readable by the quake3.exe at initialization
	// any other time the key should only be accessed in memory using the provided functions
	if(com_fullyInitialized && strstr(filename, "q3key"))
	{
		if(file == nullptr)
			return false;

		*file = 0;
		return -1;
	}

	if(file == nullptr)
	{
		// just wants to see if file is there

		// is the element a pak file?
		if(search->pack)
		{
			hash = FS_HashFileName(filename, search->pack->hashSize);

			if(search->pack->hashTable[hash])
			{
				// look through all the pak file elements
				pak = search->pack;
				pakFile = pak->hashTable[hash];

				do
				{
					// case and separator insensitive comparisons
					if(!FileSystem::filenameCompare(pakFile->name, filename))
					{
						// found it!
						if(pakFile->len)
							return pakFile->len;
						else
						{
							// It's not nice, but legacy code depends
							// on positive value if file exists no matter
							// what size
							return 1;
						}
					}

					pakFile = pakFile->next;
				} while(pakFile != nullptr);
			}
		}
		else if(search->dir)
		{
			dir = search->dir;

			netpath = FileSystem::buildOSPath(dir->path, dir->gamedir, filename);
			filep = Sys_FOpen(netpath, "rb");

			if(filep)
			{
				len = FS_fplength(filep);
				fclose(filep);

				if(len)
					return len;
				else
					return 1;
			}
		}

		return 0;
	}

	*file = FS_HandleForFile();
	fsh[*file].handleFiles.unique = uniqueFILE;

	// is the element a pak file?
	if(search->pack)
	{
		hash = FS_HashFileName(filename, search->pack->hashSize);

		if(search->pack->hashTable[hash])
		{
			// disregard if it doesn't match one of the allowed pure pak files
			if(!unpure && !FS_PakIsPure(search->pack))
			{
				*file = 0;
				return -1;
			}

			// look through all the pak file elements
			pak = search->pack;
			pakFile = pak->hashTable[hash];

			do
			{
				// case and separator insensitive comparisons
				if(!FileSystem::filenameCompare(pakFile->name, filename))
				{
					// found it!

					// mark the pak as having been referenced and mark specifics on cgame and ui
					// shaders, txt, arena files  by themselves do not count as a reference as 
					// these are loaded from all pk3s 
					// from every pk3 file.. 
					len = strlen(filename);

					if (!(pak->referenced & FS_GENERAL_REF))
					{
						if(!FS_IsExt(filename, ".shader", len) &&
						   !FS_IsExt(filename, ".txt", len) &&
						   !FS_IsExt(filename, ".cfg", len) &&
						   !FS_IsExt(filename, ".config", len) &&
						   !FS_IsExt(filename, ".bot", len) &&
						   !FS_IsExt(filename, ".arena", len) &&
						   !FS_IsExt(filename, ".menu", len) &&
						   Q_stricmp(filename, "vm/qagame.qvm") != 0 &&
						   !strstr(filename, "levelshots"))
						{
							pak->referenced |= FS_GENERAL_REF;
						}
					}

					if(strstr(filename, "cgame.qvm"))
						pak->referenced |= FS_CGAME_REF;
					if(strstr(filename, "ui.qvm"))
						pak->referenced |= FS_UI_REF;

					if(uniqueFILE)
					{
						// open a new file on the pakfile
						fsh[*file].handleFiles.file.z = unzOpen(pak->pakFilename);

						if(fsh[*file].handleFiles.file.z == nullptr)
							Com_Error(ERR_FATAL, "Couldn't open %s", pak->pakFilename);
					}
					else
						fsh[*file].handleFiles.file.z = pak->handle;

					Q_strncpyz(fsh[*file].name, filename, sizeof(fsh[*file].name));
					fsh[*file].zipFile = true;

					// set the file position in the zip file (also sets the current file info)
					unzSetOffset(fsh[*file].handleFiles.file.z, pakFile->pos);

					// open the file in the zip
					unzOpenCurrentFile(fsh[*file].handleFiles.file.z);
					fsh[*file].zipFilePos = pakFile->pos;
					fsh[*file].zipFileLen = pakFile->len;

					if(fs_debug->integer)
					{
						Com_Printf("FileSystem::fOpenFileRead: %s (found in '%s')\n", 
								filename, pak->pakFilename);
					}

					return pakFile->len;
				}

				pakFile = pakFile->next;
			} while(pakFile != nullptr);
		}
	}
	else if(search->dir)
	{
		// check a file in the directory tree

		// if we are running restricted, the only files we
		// will allow to come from the directory are .cfg files
		len = strlen(filename);
		// FIXME TTimo I'm not sure about the fs_numServerPaks test
		// if you are using FileSystem::readFile to find out if a file exists,
		//   this test can make the search fail although the file is in the directory
		// I had the problem on https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=8
		// turned out I used FileSystem::fileExists_HomeData instead
		if(!unpure && fs_numServerPaks)
		{
			if(!FS_IsExt(filename, ".cfg", len) &&		// for config files
			   !FS_IsExt(filename, ".menu", len) &&		// menu files
			   !FS_IsExt(filename, ".game", len) &&		// menu files
			   !FS_IsExt(filename, ".dat", len) &&		// for journal files
			   !FS_IsDemoExt(filename, len))			// demos
			{
				*file = 0;
				return -1;
			}
		}

		dir = search->dir;

		netpath = FileSystem::buildOSPath(dir->path, dir->gamedir, filename);
		filep = Sys_FOpen(netpath, "rb");

		if (filep == nullptr)
		{
			*file = 0;
			return -1;
		}

		Q_strncpyz(fsh[*file].name, filename, sizeof(fsh[*file].name));
		fsh[*file].zipFile = false;

		if(fs_debug->integer)
		{
			Com_Printf("FileSystem::fOpenFileRead: %s (found in '%s%c%s')\n", filename,
					dir->path, PATH_SEP, dir->gamedir);
		}

		fsh[*file].handleFiles.file.o = filep;
		return FS_fplength(filep);
	}

	*file = 0;
	return -1;
}

/*
===========
FileSystem::fOpenFileRead

Finds the file in the search path.
Returns filesize and an open FILE pointer.
Used for streaming data out of either a
separate file or a ZIP file.
===========
*/
long FileSystem::fOpenFileRead(const char *filename, fileHandle_t *file, bool uniqueFILE)
{
	searchpath_t *search;
	long len;
	bool isLocalConfig;

	if(!fs_searchpaths)
		Com_Error(ERR_FATAL, "Filesystem call made without initialization");

	isLocalConfig = !strcmp(filename, "autoexec.cfg") || !strcmp(filename, Q3CONFIG_CFG);
	for(search = fs_searchpaths; search; search = search->next)
	{
		// autoexec.cfg and q3config.cfg can only be loaded outside of pk3 files.
		if (isLocalConfig && search->pack)
			continue;

		len = FS_FOpenFileReadDir(filename, search, file, uniqueFILE, false);

		if(file == nullptr)
		{
			if(len > 0)
				return len;
		}
		else
		{
			if(len >= 0 && *file)
				return len;
		}

	}
	
#ifdef FS_MISSING
	if(missingFiles)
		fprintf(missingFiles, "%s\n", filename);
#endif

	if(file)
	{
		*file = 0;
		return -1;
	}
	else
	{
		// When file is NULL, we're querying the existence of the file
		// If we've got here, it doesn't exist
		return 0;
	}
}


/*
=================
FileSystem::read

Properly handles partial reads
=================
*/
int FileSystem::read( void *buffer, int len, fileHandle_t f ) {
	int		block, remaining;
	int		read;
	byte	*buf;
	int		tries;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !f ) {
		return 0;
	}

	buf = static_cast<byte *>(buffer);
	fs_readCount += len;

	if (fsh[f].zipFile == false) {
		remaining = len;
		tries = 0;
		while (remaining) {
			block = remaining;
			read = fread (buf, 1, block, fsh[f].handleFiles.file.o);
			if (read == 0) {
				// we might have been trying to read from a CD, which
				// sometimes returns a 0 read on windows
				if (!tries) {
					tries = 1;
				} else {
					return len-remaining;	//Com_Error (ERR_FATAL, "FileSystem::read: 0 bytes read");
				}
			}

			if (read == -1) {
				Com_Error (ERR_FATAL, "FileSystem::read: -1 bytes read");
			}

			remaining -= read;
			buf += read;
		}
		return len;
	} else {
		return unzReadCurrentFile(fsh[f].handleFiles.file.z, buffer, len);
	}
}

/*
=================
FileSystem::write

Properly handles partial writes
=================
*/
int FileSystem::write( const void *buffer, int len, fileHandle_t h ) {
	int		block, remaining;
	int		written;
	byte	*buf;
	int		tries;
	FILE	*f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !h ) {
		return 0;
	}

	f = FS_FileForHandle(h);
	buf = (byte *)buffer;

	remaining = len;
	tries = 0;
	while (remaining) {
		block = remaining;
		written = fwrite (buf, 1, block, f);
		if (written == 0) {
			if (!tries) {
				tries = 1;
			} else {
				Com_Printf( "FileSystem::write: 0 bytes written\n" );
				return 0;
			}
		}

		if (written == -1) {
			Com_Printf( "FileSystem::write: -1 bytes written\n" );
			return 0;
		}

		remaining -= written;
		buf += written;
	}
	if ( fsh[h].handleSync ) {
		fflush( f );
	}
	return len;
}

void QDECL FileSystem::printf( fileHandle_t h, const char *fmt, ... ) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];

	va_start (argptr,fmt);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	FileSystem::write(msg, strlen(msg), h);
}

constexpr int PK3_SEEK_BUFFER_SIZE = 65536;

/*
=================
FileSystem::seek

=================
*/
int FileSystem::seek( fileHandle_t f, long offset, int origin ) {
	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
		return -1;
	}

	if (fsh[f].zipFile == true) {
		//FIXME: this is really, really crappy
		//(but better than what was here before)
		byte	buffer[PK3_SEEK_BUFFER_SIZE];
		int		remainder;
		int		currentPosition = FileSystem::fTell( f );

		// change negative offsets into FS_SEEK_SET
		if ( offset < 0 ) {
			switch( origin ) {
				case FS_SEEK_END:
					remainder = fsh[f].zipFileLen + offset;
					break;

				case FS_SEEK_CUR:
					remainder = currentPosition + offset;
					break;

				case FS_SEEK_SET:
				default:
					remainder = 0;
					break;
			}

			if ( remainder < 0 ) {
				remainder = 0;
			}

			origin = FS_SEEK_SET;
		} else {
			if ( origin == FS_SEEK_END ) {
				remainder = fsh[f].zipFileLen - currentPosition + offset;
			} else {
				remainder = offset;
			}
		}

		switch( origin ) {
			case FS_SEEK_SET:
				if ( remainder == currentPosition ) {
					return offset;
				}
				unzSetOffset(fsh[f].handleFiles.file.z, fsh[f].zipFilePos);
				unzOpenCurrentFile(fsh[f].handleFiles.file.z);
				//fallthrough

			case FS_SEEK_END:
			case FS_SEEK_CUR:
				while( remainder > PK3_SEEK_BUFFER_SIZE ) {
					FileSystem::read( buffer, PK3_SEEK_BUFFER_SIZE, f );
					remainder -= PK3_SEEK_BUFFER_SIZE;
				}
				FileSystem::read( buffer, remainder, f );
				return offset;

			default:
				Com_Error( ERR_FATAL, "Bad origin in FileSystem::seek" );
				return -1;
		}
	} else {
		FILE *file;
		file = FS_FileForHandle(f);
		int _origin = SEEK_SET;
		switch( origin ) {
		case FS_SEEK_CUR:
			_origin = SEEK_CUR;
			break;
		case FS_SEEK_END:
			_origin = SEEK_END;
			break;
		case FS_SEEK_SET:
			_origin = SEEK_SET;
			break;
		default:
			Com_Error( ERR_FATAL, "Bad origin in FileSystem::seek" );
			break;
		}

		return fseek( file, offset, _origin );
	}
}


/*
======================================================================================

CONVENIENCE FUNCTIONS FOR ENTIRE FILES

======================================================================================
*/

int	FileSystem::fileIsInPAK(const char *filename, int *pChecksum ) {
	searchpath_t	*search;
	pack_t			*pak;
	fileInPack_t	*pakFile;
	long			hash = 0;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !filename ) {
		Com_Error( ERR_FATAL, "FileSystem::fOpenFileRead: NULL 'filename' parameter passed" );
	}

	// qpaths are not supposed to have a leading slash
	if ( filename[0] == '/' || filename[0] == '\\' ) {
		filename++;
	}

	// make absolutely sure that it can't back up the path.
	// The searchpaths do guarantee that something will always
	// be prepended, so we don't need to worry about "c:" or "//limbo" 
	if ( strstr( filename, ".." ) || strstr( filename, "::" ) ) {
		return -1;
	}

	//
	// search through the path, one element at a time
	//

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		//
		if (search->pack) {
			hash = FS_HashFileName(filename, search->pack->hashSize);
		}
		// is the element a pak file?
		if ( search->pack && search->pack->hashTable[hash] ) {
			// disregard if it doesn't match one of the allowed pure pak files
			if ( !FS_PakIsPure(search->pack) ) {
				continue;
			}

			// look through all the pak file elements
			pak = search->pack;
			pakFile = pak->hashTable[hash];
			do {
				// case and separator insensitive comparisons
				if ( !FileSystem::filenameCompare( pakFile->name, filename ) ) {
					if (pChecksum) {
						*pChecksum = pak->pure_checksum;
					}
					return 1;
				}
				pakFile = pakFile->next;
			} while(pakFile != nullptr);
		}
	}
	return -1;
}

/*
============
FileSystem::readFileDir

Filename are relative to the quake search path
a null buffer will just return the file length without loading
If searchPath is non-NULL search only in that specific search path
============
*/
long FileSystem::readFileDir(const char *qpath, void *searchPath, bool unpure, void **buffer)
{
	fileHandle_t	h;
	searchpath_t	*search;
	byte*			buf;
	bool		isConfig;
	long				len;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !qpath || !qpath[0] ) {
		Com_Error( ERR_FATAL, "FileSystem::readFile with empty name" );
	}

	buf = nullptr;	// quiet compiler warning

	// if this is a .cfg file and we are playing back a journal, read
	// it from the journal file
	if ( strstr( qpath, ".cfg" ) ) {
		isConfig = true;
		if ( com_journal && com_journal->integer == 2 ) {
			int		r;

			Com_DPrintf( "Loading %s from journal file.\n", qpath );
			r = FileSystem::read( &len, sizeof( len ), com_journalDataFile );
			if ( r != sizeof( len ) ) {
				if (buffer != nullptr) *buffer = nullptr;
				return -1;
			}
			// if the file didn't exist when the journal was created
			if (!len) {
				if (buffer == nullptr) {
					return 1;			// hack for old journal files
				}
				*buffer = nullptr;
				return -1;
			}
			if (buffer == nullptr) {
				return len;
			}

			buf = static_cast<byte *>(Hunk_AllocateTempMemory(len+1));
			*buffer = buf;

			r = FileSystem::read( buf, len, com_journalDataFile );
			if ( r != len ) {
				Com_Error( ERR_FATAL, "Read from journalDataFile failed" );
			}

			fs_loadCount++;
			fs_loadStack++;

			// guarantee that it will have a trailing 0 for string operations
			buf[len] = 0;

			return len;
		}
	} else {
		isConfig = false;
	}

	search = static_cast<searchpath_t *>(searchPath);

	if(search == nullptr)
	{
		// look for it in the filesystem or pack files
		len = FileSystem::fOpenFileRead(qpath, &h, false);
	}
	else
	{
		// look for it in a specific search path only
		len = FS_FOpenFileReadDir(qpath, search, &h, false, unpure);
	}

	if ( h == 0 ) {
		if ( buffer ) {
			*buffer = nullptr;
		}
		// if we are journalling and it is a config file, write a zero to the journal file
		if ( isConfig && com_journal && com_journal->integer == 1 ) {
			Com_DPrintf( "Writing zero for %s to journal file.\n", qpath );
			len = 0;
			FileSystem::write( &len, sizeof( len ), com_journalDataFile );
			FileSystem::flush( com_journalDataFile );
		}
		return -1;
	}

	if ( !buffer ) {
		if ( isConfig && com_journal && com_journal->integer == 1 ) {
			Com_DPrintf( "Writing len for %s to journal file.\n", qpath );
			FileSystem::write( &len, sizeof( len ), com_journalDataFile );
			FileSystem::flush( com_journalDataFile );
		}
		FileSystem::fCloseFile( h);
		return len;
	}

	fs_loadCount++;
	fs_loadStack++;

	buf = static_cast<byte *>(Hunk_AllocateTempMemory(len+1));
	*buffer = buf;

	FileSystem::read (buf, len, h);

	// guarantee that it will have a trailing 0 for string operations
	buf[len] = 0;
	FileSystem::fCloseFile( h );

	// if we are journalling and it is a config file, write it to the journal file
	if ( isConfig && com_journal && com_journal->integer == 1 ) {
		Com_DPrintf( "Writing %s to journal file.\n", qpath );
		FileSystem::write( &len, sizeof( len ), com_journalDataFile );
		FileSystem::write( buf, len, com_journalDataFile );
		FileSystem::flush( com_journalDataFile );
	}
	return len;
}

/*
============
FileSystem::readFile

Filename are relative to the quake search path
a null buffer will just return the file length without loading
============
*/
long FileSystem::readFile(const char *qpath, void **buffer)
{
	return FileSystem::readFileDir(qpath, nullptr, false, buffer);
}

/*
=============
FileSystem::freeFile
=============
*/
void FileSystem::freeFile( void *buffer ) {
	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}
	if ( !buffer ) {
		Com_Error( ERR_FATAL, "FileSystem::freeFile( NULL )" );
	}
	fs_loadStack--;

	Hunk_FreeTempMemory( buffer );

	// if all of our temp files are free, clear all of our space
	if ( fs_loadStack == 0 ) {
		Hunk_ClearTempMemory();
	}
}

/*
============
FileSystem::writeFile

Filename are relative to the quake search path
============
*/
void FileSystem::writeFile( const char *qpath, const void *buffer, int size ) {
	fileHandle_t f;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !qpath || !buffer ) {
		Com_Error( ERR_FATAL, "FileSystem::writeFile: NULL parameter" );
	}

	f = FileSystem::fOpenFileWrite_HomeData( qpath );
	if ( !f ) {
		Com_Printf( "Failed to open %s\n", qpath );
		return;
	}

	FileSystem::write( buffer, size, f );

	FileSystem::fCloseFile( f );
}



/*
==========================================================================

ZIP FILE LOADING

==========================================================================
*/

/*
=================
FS_LoadZipFile

Creates a new pak_t in the search chain for the contents
of a zip file.
=================
*/
static pack_t *FS_LoadZipFile(const char *zipfile, const char *basename)
{
	fileInPack_t	*buildBuffer;
	pack_t			*pack;
	unzFile			uf;
	int				err;
	unz_global_info gi;
	char			filename_inzip[MAX_ZPATH];
	unz_file_info	file_info;
	int				i, len;
	long			hash;
	int				fs_numHeaderLongs;
	int				*fs_headerLongs;
	char			*namePtr;

	fs_numHeaderLongs = 0;

	uf = unzOpen(zipfile);
	err = unzGetGlobalInfo (uf,&gi);

	if (err != UNZ_OK)
		return nullptr;

	len = 0;
	unzGoToFirstFile(uf);
	for (i = 0; i < gi.number_entry; i++)
	{
		err = unzGetCurrentFileInfo(uf, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0);
		if (err != UNZ_OK) {
			break;
		}
		len += strlen(filename_inzip) + 1;
		unzGoToNextFile(uf);
	}

	buildBuffer = static_cast<fileInPack_t *>(Z_Malloc( (gi.number_entry * sizeof( fileInPack_t )) + len ));
	namePtr = (reinterpret_cast<char *>(buildBuffer)) + gi.number_entry * sizeof( fileInPack_t );
	fs_headerLongs = static_cast<int *>(Z_Malloc( ( gi.number_entry + 1 ) * sizeof(int) ));
	fs_headerLongs[ fs_numHeaderLongs++ ] = LittleLong( fs_checksumFeed );

	// get the hash table size from the number of files in the zip
	// because lots of custom pk3 files have less than 32 or 64 files
	for (i = 1; i <= MAX_FILEHASH_SIZE; i <<= 1) {
		if (i > gi.number_entry) {
			break;
		}
	}

	pack = static_cast<pack_t *>(Z_Malloc( sizeof( pack_t ) + i * sizeof(fileInPack_t *) ));
	pack->hashSize = i;
	pack->hashTable = reinterpret_cast<fileInPack_t **>((reinterpret_cast<char *>(pack)) + sizeof( pack_t ));
	for(i = 0; i < pack->hashSize; i++) {
		pack->hashTable[i] = nullptr;
	}

	Q_strncpyz( pack->pakFilename, zipfile, sizeof( pack->pakFilename ) );
	Q_strncpyz( pack->pakBasename, basename, sizeof( pack->pakBasename ) );

	// strip .pk3 if needed
	if ( strlen( pack->pakBasename ) > 4 && !Q_stricmp( pack->pakBasename + strlen( pack->pakBasename ) - 4, ".pk3" ) ) {
		pack->pakBasename[strlen( pack->pakBasename ) - 4] = 0;
	}

	pack->handle = uf;
	pack->numfiles = gi.number_entry;
	unzGoToFirstFile(uf);

	for (i = 0; i < gi.number_entry; i++)
	{
		err = unzGetCurrentFileInfo(uf, &file_info, filename_inzip, sizeof(filename_inzip), nullptr, 0, nullptr, 0);
		if (err != UNZ_OK) {
			break;
		}
		if (file_info.uncompressed_size > 0) {
			fs_headerLongs[fs_numHeaderLongs++] = LittleLong(file_info.crc);
		}
		Q_strlwr( filename_inzip );
		hash = FS_HashFileName(filename_inzip, pack->hashSize);
		buildBuffer[i].name = namePtr;
		strcpy( buildBuffer[i].name, filename_inzip );
		namePtr += strlen(filename_inzip) + 1;
		// store the file position in the zip
		buildBuffer[i].pos = unzGetOffset(uf);
		buildBuffer[i].len = file_info.uncompressed_size;
		buildBuffer[i].next = pack->hashTable[hash];
		pack->hashTable[hash] = &buildBuffer[i];
		unzGoToNextFile(uf);
	}

	pack->checksum = Com_BlockChecksum( &fs_headerLongs[ 1 ], sizeof(*fs_headerLongs) * ( fs_numHeaderLongs - 1 ) );
	pack->pure_checksum = Com_BlockChecksum( fs_headerLongs, sizeof(*fs_headerLongs) * fs_numHeaderLongs );
	pack->checksum = LittleLong( pack->checksum );
	pack->pure_checksum = LittleLong( pack->pure_checksum );

	Z_Free(fs_headerLongs);

	pack->buildBuffer = buildBuffer;
	return pack;
}

/*
=================
FS_FreePak

Frees a pak structure and releases all associated resources
=================
*/

static void FS_FreePak(pack_t *thepak)
{
	unzClose(thepak->handle);
	Z_Free(thepak->buildBuffer);
	Z_Free(thepak);
}

/*
=================
FS_GetZipChecksum

Compares whether the given pak file matches a referenced checksum
=================
*/
bool FileSystem::compareZipChecksum(const char *zipfile)
{
	pack_t *thepak;
	int index;
	unsigned int checksum;

	thepak = FS_LoadZipFile(zipfile, "");

	if(!thepak)
		return false;

	checksum = thepak->checksum;
	FS_FreePak(thepak);

	for(index = 0; index < fs_numServerReferencedPaks; index++)
	{
		if(checksum == fs_serverReferencedPaks[index])
			return true;
	}

	return false;
}

/*
=================================================================================

DIRECTORY SCANNING FUNCTIONS

=================================================================================
*/

#define	MAX_FOUND_FILES	0x1000

static int FS_ReturnPath( const char *zname, char *zpath, int *depth ) {
	int len, at, newdep;

	newdep = 0;
	zpath[0] = 0;
	len = 0;
	at = 0;

	while(zname[at] != 0)
	{
		if (zname[at]=='/' || zname[at]=='\\') {
			len = at;
			newdep++;
		}
		at++;
	}
	strcpy(zpath, zname);
	zpath[len] = 0;
	*depth = newdep;

	return len;
}

/*
==================
FS_AddFileToList
==================
*/
static int FS_AddFileToList( char *name, char *list[MAX_FOUND_FILES], int nfiles ) {
	int		i;

	if ( nfiles == MAX_FOUND_FILES - 1 ) {
		return nfiles;
	}
	for ( i = 0 ; i < nfiles ; i++ ) {
		if ( !Q_stricmp( name, list[i] ) ) {
			return nfiles;		// already in list
		}
	}
	list[nfiles] = CopyString( name );
	nfiles++;

	return nfiles;
}

/*
===============
FS_ListFilteredFiles

Returns a uniqued list of files that match the given criteria
from all search paths
===============
*/
char **FS_ListFilteredFiles( const char *path, const char *extension,
	char *filter, int *numfiles, bool stripPath,
	bool allowNonPureFilesOnDisk ) {
	int				nfiles;
	char			**listCopy;
	char			*list[MAX_FOUND_FILES];
	searchpath_t	*search;
	int				i;
	int				pathLength;
	int				extensionLength;
	int				length, pathDepth, pathSkip;
	pack_t			*pak;
	fileInPack_t	*buildBuffer;
	char			zpath[MAX_ZPATH];

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !path ) {
		*numfiles = 0;
		return nullptr;
	}
	if ( !extension ) {
		extension = "";
	}

	pathLength = strlen( path );
	if ( path[pathLength-1] == '\\' || path[pathLength-1] == '/' ) {
		pathLength--;
	}
	extensionLength = strlen( extension );
	nfiles = 0;
	FS_ReturnPath(path, zpath, &pathDepth);

	//
	// search through the path, one element at a time, adding to list
	//
	for (search = fs_searchpaths ; search ; search = search->next) {
		// is the element a pak file?
		if (search->pack) {

			//ZOID:  If we are pure, don't search for files on paks that
			// aren't on the pure list
			if ( !FS_PakIsPure(search->pack) ) {
				continue;
			}

			// look through all the pak file elements
			pak = search->pack;
			buildBuffer = pak->buildBuffer;
			for (i = 0; i < pak->numfiles; i++) {
				char	*name;
				int		zpathLen, depth;

				// check for directory match
				name = buildBuffer[i].name;

				if (stripPath) {
					pathSkip = pathLength;
					if (pathLength) {
						pathSkip++; // include the '/'
					}
				} else {
					pathSkip = 0;
				}

				if (filter) {
					// case insensitive
					if (!Com_FilterPath( filter, name, false ))
						continue;

					// unique the match
					nfiles = FS_AddFileToList( name + pathSkip, list, nfiles );
				}
				else {

					zpathLen = FS_ReturnPath(name, zpath, &depth);

					if ( (depth-pathDepth)>2 || pathLength > zpathLen || Q_stricmpn( name, path, pathLength ) ) {
						continue;
					}

					// check for extension match
					length = strlen( name );
					if ( length < extensionLength ) {
						continue;
					}

					if ( Q_stricmp( name + length - extensionLength, extension ) ) {
						continue;
					}

					// unique the match
					nfiles = FS_AddFileToList( name + pathSkip, list, nfiles );
				}
			}
		} else if (search->dir) { // scan for files in the filesystem
			char	*netpath;
			int		numSysFiles;
			char	**sysFiles;
			char	*name;

			// don't scan directories for files if we are pure or restricted
			if ( fs_numServerPaks && !allowNonPureFilesOnDisk ) {
				continue;
			} else {
				netpath = FileSystem::buildOSPath( search->dir->path, search->dir->gamedir, path );
				sysFiles = Sys_ListFiles( netpath, extension, filter, &numSysFiles, false );
				for ( i = 0 ; i < numSysFiles ; i++ ) {
					// unique the match
					name = sysFiles[i];
					nfiles = FS_AddFileToList( name, list, nfiles );
				}
				Sys_FreeFileList( sysFiles );
			}
		}
	}

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return nullptr;
	}

	listCopy = static_cast<char **>(Z_Malloc( ( nfiles + 1 ) * sizeof( *listCopy ) ));
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = nullptr;

	return listCopy;
}

/*
=================
FileSystem::listFiles
=================
*/
char **FileSystem::listFiles( const char *path, const char *extension, int *numfiles ) {
	return FS_ListFilteredFiles( path, extension, nullptr, numfiles, true, false );
}

/*
=================
FileSystem::freeFileList
=================
*/
void FileSystem::freeFileList( char **list ) {
	int		i;

	if ( !fs_searchpaths ) {
		Com_Error( ERR_FATAL, "Filesystem call made without initialization" );
	}

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}


/*
================
FileSystem::getFileList
================
*/
int	FileSystem::getFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	int		nFiles, i, nTotal, nLen;
	char **pFiles = nullptr;

	*listbuf = 0;
	nFiles = 0;
	nTotal = 0;

	if (Q_stricmp(path, "$modlist") == 0) {
		return FileSystem::getModList(listbuf, bufsize);
	}

	pFiles = FileSystem::listFiles(path, extension, &nFiles);

	for (i =0; i < nFiles; i++) {
		nLen = strlen(pFiles[i]) + 1;
		if (nTotal + nLen + 1 < bufsize) {
			strcpy(listbuf, pFiles[i]);
			listbuf += nLen;
			nTotal += nLen;
		}
		else {
			nFiles = i;
			break;
		}
	}

	FileSystem::freeFileList(pFiles);

	return nFiles;
}

/*
=======================
Sys_ConcatenateFileLists

mkv: Naive implementation. Concatenates three lists into a
     new list, and frees the old lists from the heap.
bk001129 - from cvs1.17 (mkv)

FIXME TTimo those two should move to common.c next to Sys_ListFiles
=======================
 */
static unsigned int Sys_CountFileList(char **list)
{
	int i = 0;

	if (list)
	{
		while (*list)
		{
			list++;
			i++;
		}
	}
	return i;
}

static char** Sys_ConcatenateFileLists( char **list0, char **list1 )
{
	int totalLength = 0;
	char** cat = nullptr, **dst, **src;

	totalLength += Sys_CountFileList(list0);
	totalLength += Sys_CountFileList(list1);

	/* Create new list. */
	dst = cat = static_cast<char **>(Z_Malloc( ( totalLength + 1 ) * sizeof( char* ) ));

	/* Copy over lists. */
	if (list0)
	{
		for (src = list0; *src; src++, dst++)
			*dst = *src;
	}
	if (list1)
	{
		for (src = list1; *src; src++, dst++)
			*dst = *src;
	}

	// Terminate the list
	*dst = nullptr;

	// Free our old lists.
	// NOTE: not freeing their content, it's been merged in dst and still being used
	if (list0) Z_Free( list0 );
	if (list1) Z_Free( list1 );

	return cat;
}

/*
================
FileSystem::getModDescription
================
*/
void FileSystem::getModDescription( const char *modDir, char *description, int descriptionLen ) {
	fileHandle_t	descHandle;
	char			descPath[MAX_QPATH];
	int				nDescLen;
	FILE			*file;

	Com_sprintf( descPath, sizeof ( descPath ), "%s%cdescription.txt", modDir, PATH_SEP );
	nDescLen = FileSystem::baseDir_FOpenFileRead( descPath, &descHandle );

	if ( nDescLen > 0 ) {
		file = FS_FileForHandle(descHandle);
		Com_Memset( description, 0, descriptionLen );
		nDescLen = fread(description, 1, descriptionLen, file);
		if (nDescLen >= 0) {
			description[nDescLen] = '\0';
		}
	} else {
		Q_strncpyz( description, modDir, descriptionLen );
	}

	if ( descHandle ) {
		FileSystem::fCloseFile( descHandle );
	}
}

/*
================
FileSystem::getModList

Returns a list of mod directory names
A mod directory is a peer to baseq3 with a pk3 or pk3dir in it
================
*/
int	FileSystem::getModList( char *listbuf, int bufsize ) {
	int nMods, i, j, k, nTotal, nLen, nPaks = 0, nDirs = 0, nPakDirs = 0, nPotential, nDescLen;
	char **pFiles = nullptr;
	char **pPaks = nullptr;
	char **pDirs = nullptr;
	char *name, *path;
	char description[MAX_OSPATH];

	int dummy;
	char **pFiles0 = nullptr;
	bool bDrop = false;

	*listbuf = 0;
	nMods = nTotal = 0;

	// iterate through paths and get list of potential mods
	for (i = 0; i < ARRAY_LEN(fs_pathVars); i++) {
		const cvar_t *pathVar = fs_pathVars[i];

		if (!pathVar || !pathVar->string[0]) {
			continue;
		}

		pFiles0 = Sys_ListFiles(pathVar->string, nullptr, nullptr, &dummy, true);
		// Sys_ConcatenateFileLists frees the lists so Sys_FreeFileList isn't required
		pFiles = Sys_ConcatenateFileLists(pFiles, pFiles0);
	}

	nPotential = Sys_CountFileList(pFiles);

	for (i = 0; i < nPotential; i++) {
		name = pFiles[i];
		// NOTE: cleaner would involve more changes
		// ignore duplicate mod directories
		if (i != 0) {
			bDrop = false;
			for (j = 0; j < i; j++) {
				if (Q_stricmp(pFiles[j], name) == 0) {
					// this one can be dropped
					bDrop = true;
					break;
				}
			}
		}
		// we also drop "baseq3" "." and ".."
		if (bDrop || Q_stricmp(name, com_basegame->string) == 0 || Q_stricmpn(name, ".", 1) == 0) {
			continue;
		}

		// in order to be a valid mod the directory must contain at least one .pk3 or .pk3dir
		// we didn't keep the information when we merged the directory names, as to what OS Path it was found under
		// so we will try each of them here
		for (j = 0; j < ARRAY_LEN(fs_pathVars); j++) {
			const cvar_t *pathVar = fs_pathVars[j];

			if (!pathVar || !pathVar->string[0]) {
				continue;
			}

			path = FileSystem::baseDir_BuildOSPath(pathVar->string, name);
			nPaks = nDirs = nPakDirs = 0;
			pPaks = Sys_ListFiles(path, ".pk3", nullptr, &nPaks, false);
			pDirs = Sys_ListFiles(path, "/", nullptr, &nDirs, false);
			for (k = 0; k < nDirs; k++) {
				// we only want to count directories ending with ".pk3dir"
				if (FS_IsExt(pDirs[k], ".pk3dir", strlen(pDirs[k]))) {
					nPakDirs++;
				}
			}
			// we only use Sys_ListFiles to check whether files are present
			Sys_FreeFileList(pPaks);
			Sys_FreeFileList(pDirs);

			if (nPaks > 0 || nPakDirs > 0) {
				break;
			}
		}

		if (nPaks > 0 || nPakDirs > 0) {
			nLen = strlen(name) + 1;
			// nLen is the length of the mod path
			// we need to see if there is a description available
			FileSystem::getModDescription(name, description, sizeof(description));
			nDescLen = strlen(description) + 1;

			if (nTotal + nLen + 1 + nDescLen + 1 < bufsize) {
				strcpy(listbuf, name);
				listbuf += nLen;
				strcpy(listbuf, description);
				listbuf += nDescLen;
				nTotal += nLen + nDescLen;
				nMods++;
			} else {
				break;
			}
		}
	}
	Sys_FreeFileList( pFiles );

	return nMods;
}




//============================================================================

/*
================
FS_Dir_f
================
*/
void FS_Dir_f( ) {
	char	*path;
	char	*extension;
	char	**dirnames;
	int		ndirs;
	int		i;

	if ( CommandSystem::argc() < 2 || CommandSystem::argc() > 3 ) {
		Com_Printf( "usage: dir <directory> [extension]\n" );
		return;
	}

	if ( CommandSystem::argc() == 2 ) {
		path = CommandSystem::argv( 1 );
		extension = "";
	} else {
		path = CommandSystem::argv( 1 );
		extension = CommandSystem::argv( 2 );
	}

	Com_Printf( "Directory of %s %s\n", path, extension );
	Com_Printf( "---------------\n" );

	dirnames = FileSystem::listFiles( path, extension, &ndirs );

	for ( i = 0; i < ndirs; i++ ) {
		Com_Printf( "%s\n", dirnames[i] );
	}
	FileSystem::freeFileList( dirnames );
}

/*
===========
FS_ConvertPath
===========
*/
void FS_ConvertPath( char *s ) {
	while (*s) {
		if ( *s == '\\' || *s == ':' ) {
			*s = '/';
		}
		s++;
	}
}

/*
===========
FS_PathCmp

Ignore case and seprator char distinctions
===========
*/
int FS_PathCmp( const char *s1, const char *s2 ) {
	int		c1, c2;

	do {
		c1 = *s1++;
		c2 = *s2++;

		if (c1 >= 'a' && c1 <= 'z') {
			c1 -= ('a' - 'A');
		}
		if (c2 >= 'a' && c2 <= 'z') {
			c2 -= ('a' - 'A');
		}

		if ( c1 == '\\' || c1 == ':' ) {
			c1 = '/';
		}
		if ( c2 == '\\' || c2 == ':' ) {
			c2 = '/';
		}
		
		if (c1 < c2) {
			return -1;		// strings not equal
		}
		if (c1 > c2) {
			return 1;
		}
	} while (c1);
	
	return 0;		// strings are equal
}

/*
================
FS_SortFileList
================
*/
void FS_SortFileList(char **filelist, int numfiles) {
	int i, j, k, numsortedfiles;
	char **sortedlist;

	sortedlist = static_cast<char **>(Z_Malloc( ( numfiles + 1 ) * sizeof( *sortedlist ) ));
	sortedlist[0] = nullptr;
	numsortedfiles = 0;
	for (i = 0; i < numfiles; i++) {
		for (j = 0; j < numsortedfiles; j++) {
			if (FS_PathCmp(filelist[i], sortedlist[j]) < 0) {
				break;
			}
		}
		for (k = numsortedfiles; k > j; k--) {
			sortedlist[k] = sortedlist[k-1];
		}
		sortedlist[j] = filelist[i];
		numsortedfiles++;
	}
	Com_Memcpy(filelist, sortedlist, numfiles * sizeof( *filelist ) );
	Z_Free(sortedlist);
}

/*
================
FS_NewDir_f
================
*/
void FS_NewDir_f( ) {
	char	*filter;
	char	**dirnames;
	int		ndirs;
	int		i;

	if ( CommandSystem::argc() < 2 ) {
		Com_Printf( "usage: fdir <filter>\n" );
		Com_Printf( "example: fdir *q3dm*.bsp\n");
		return;
	}

	filter = CommandSystem::argv( 1 );

	Com_Printf( "---------------\n" );

	dirnames = FS_ListFilteredFiles( "", "", filter, &ndirs, false, false );

	FS_SortFileList(dirnames, ndirs);

	for ( i = 0; i < ndirs; i++ ) {
		FS_ConvertPath(dirnames[i]);
		Com_Printf( "%s\n", dirnames[i] );
	}
	Com_Printf( "%d files listed\n", ndirs );
	FileSystem::freeFileList( dirnames );
}

/*
============
FS_Path_f

============
*/
void FS_Path_f( ) {
	searchpath_t	*s;
	int				i;

	Com_Printf ("We are looking in the current search path:\n");
	for (s = fs_searchpaths; s; s = s->next) {
		if (s->pack) {
			Com_Printf ("%s (%i files)\n", s->pack->pakFilename, s->pack->numfiles);
			if ( fs_numServerPaks ) {
				if ( !FS_PakIsPure(s->pack) ) {
					Com_Printf( "    not on the pure list\n" );
				} else {
					Com_Printf( "    on the pure list\n" );
				}
			}
		} else {
			Com_Printf ("%s%c%s\n", s->dir->path, PATH_SEP, s->dir->gamedir );
		}
	}


	Com_Printf( "\n" );
	for ( i = 1 ; i < MAX_FILE_HANDLES ; i++ ) {
		if ( fsh[i].handleFiles.file.o ) {
			Com_Printf( "handle %i: %s\n", i, fsh[i].name );
		}
	}
}

/*
============
FS_TouchFile_f
============
*/
void FS_TouchFile_f( ) {
	fileHandle_t	f;

	if ( CommandSystem::argc() != 2 ) {
		Com_Printf( "Usage: touchFile <file>\n" );
		return;
	}

	FileSystem::fOpenFileRead( CommandSystem::argv( 1 ), &f, false );
	if ( f ) {
		FileSystem::fCloseFile( f );
	}
}

/*
============
FileSystem::which
============
*/

bool FileSystem::which(const char *filename, void *searchPath)
{
	searchpath_t *search = static_cast<searchpath_t *>(searchPath);

	if(FS_FOpenFileReadDir(filename, search, nullptr, false, false) > 0)
	{
		if(search->pack)
		{
			Com_Printf("File \"%s\" found in \"%s\"\n", filename, search->pack->pakFilename);
			return true;
		}
		else if(search->dir)
		{
			Com_Printf( "File \"%s\" found at \"%s\"\n", filename, search->dir->fullpath);
			return true;
		}
	}

	return false;
}

/*
============
FS_Which_f
============
*/
void FS_Which_f( ) {
	searchpath_t	*search;
	char		*filename;

	filename = CommandSystem::argv(1);

	if ( !filename[0] ) {
		Com_Printf( "Usage: which <file>\n" );
		return;
	}

	// qpaths are not supposed to have a leading slash
	if ( filename[0] == '/' || filename[0] == '\\' ) {
		filename++;
	}

	// just wants to see if file is there
	for(search = fs_searchpaths; search; search = search->next)
	{
		if(FileSystem::which(filename, search))
			return;
	}

	Com_Printf("File not found: \"%s\"\n", filename);
}


//===========================================================================


static int QDECL paksort( const void *a, const void *b ) {
	char	*aa, *bb;

	aa = *static_cast<char * const *>(a);
	bb = *static_cast<char * const *>(b);

	return FS_PathCmp( aa, bb );
}

/*
================
FS_AddGameDirectory

Sets fs_gamedir, adds the directory to the head of the path,
then loads the zip headers
================
*/
static void FS_AddGameDirectory( const char *path, const char *dir ) {
	searchpath_t	*sp;
	searchpath_t	*search;
	pack_t			*pak;
	char			curpath[MAX_OSPATH + 1], *pakfile;
	int				numfiles;
	char			**pakfiles;
	int				pakfilesi;
	char			**pakfilestmp;
	int				numdirs;
	char			**pakdirs;
	int				pakdirsi;
	char			**pakdirstmp;

	int				pakwhich;
	int				len;

	// Unique
	for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
		if ( sp->dir && !Q_stricmp(sp->dir->path, path) && !Q_stricmp(sp->dir->gamedir, dir)) {
			return;			// we've already got this one
		}
	}

	Q_strncpyz( fs_gamedir, dir, sizeof( fs_gamedir ) );

	// find all pak files in this directory
	Q_strncpyz(curpath, FileSystem::baseDir_BuildOSPath(path, dir), sizeof(curpath));

	// Get .pk3 files
	pakfiles = Sys_ListFiles(curpath, ".pk3", nullptr, &numfiles, false);

	if ( pakfiles ) {
		qsort( pakfiles, numfiles, sizeof(char*), paksort );
	}

	if ( fs_numServerPaks ) {
		numdirs = 0;
		pakdirs = nullptr;
	} else {
		// Get top level directories (we'll filter them later since the Sys_ListFiles filtering is terrible)
		pakdirs = Sys_ListFiles(curpath, "/", nullptr, &numdirs, false);

		qsort( pakdirs, numdirs, sizeof(char *), paksort );
	}

	pakfilesi = 0;
	pakdirsi = 0;

	while((pakfilesi < numfiles) || (pakdirsi < numdirs))
	{
		// Check if a pakfile or pakdir comes next
		if (pakfilesi >= numfiles) {
			// We've used all the pakfiles, it must be a pakdir.
			pakwhich = 0;
		}
		else if (pakdirsi >= numdirs) {
			// We've used all the pakdirs, it must be a pakfile.
			pakwhich = 1;
		}
		else {
			// Could be either, compare to see which name comes first
			// Need tmp variables for appropriate indirection for paksort()
			pakfilestmp = &pakfiles[pakfilesi];
			pakdirstmp = &pakdirs[pakdirsi];
			pakwhich = (paksort(pakfilestmp, pakdirstmp) < 0);
		}

		if (pakwhich) {
			// The next .pk3 file is before the next .pk3dir
			pakfile = FileSystem::buildOSPath(path, dir, pakfiles[pakfilesi]);
			if ((pak = FS_LoadZipFile(pakfile, pakfiles[pakfilesi])) == nullptr) {
				// This isn't a .pk3! Next!
				pakfilesi++;
				continue;
			}

			Q_strncpyz(pak->pakPathname, curpath, sizeof(pak->pakPathname));
			// store the game name for downloading
			Q_strncpyz(pak->pakGamename, dir, sizeof(pak->pakGamename));

			fs_packFiles += pak->numfiles;

			search = static_cast<searchpath_t *>(Z_Malloc(sizeof(searchpath_t)));
			search->pack = pak;
			search->next = fs_searchpaths;
			fs_searchpaths = search;

			pakfilesi++;
		}
		else {
			// The next .pk3dir is before the next .pk3 file
			// But wait, this could be any directory, we're filtering to only ending with ".pk3dir" here.
			len = strlen(pakdirs[pakdirsi]);
			if (!FS_IsExt(pakdirs[pakdirsi], ".pk3dir", len)) {
				// This isn't a .pk3dir! Next!
				pakdirsi++;
				continue;
			}

			pakfile = FileSystem::buildOSPath(path, dir, pakdirs[pakdirsi]);

			// add the directory to the search path
			search = static_cast<searchpath_t *>(Z_Malloc(sizeof(searchpath_t)));
			search->dir = static_cast<directory_t *>(Z_Malloc(sizeof(*search->dir)));

			Q_strncpyz(search->dir->path, curpath, sizeof(search->dir->path));	// c:\quake3\baseq3
			Q_strncpyz(search->dir->fullpath, pakfile, sizeof(search->dir->fullpath));	// c:\quake3\baseq3\mypak.pk3dir
			Q_strncpyz(search->dir->gamedir, pakdirs[pakdirsi], sizeof(search->dir->gamedir)); // mypak.pk3dir

			search->next = fs_searchpaths;
			fs_searchpaths = search;

			pakdirsi++;
		}
	}

	// done
	Sys_FreeFileList( pakfiles );
	Sys_FreeFileList( pakdirs );

	//
	// add the directory to the search path
	//
	search = static_cast<searchpath_t *>(Z_Malloc (sizeof(searchpath_t)));
	search->dir = static_cast<directory_t *>(Z_Malloc( sizeof( *search->dir ) ));

	Q_strncpyz(search->dir->path, path, sizeof(search->dir->path));
	Q_strncpyz(search->dir->fullpath, curpath, sizeof(search->dir->fullpath));
	Q_strncpyz(search->dir->gamedir, dir, sizeof(search->dir->gamedir));

	search->next = fs_searchpaths;
	fs_searchpaths = search;
}

/*
================
FS_AddGameDirectories
================
*/
static void FS_AddGameDirectories(const char *dir)
{
	// add search path elements in reverse priority order
	for(int i = ARRAY_LEN(fs_pathVars) - 1; i >= 0; i--) {
		const cvar_t *pathVar = fs_pathVars[i];

		if (!pathVar || !pathVar->string[0]) {
			continue;
		}

		FS_AddGameDirectory(pathVar->string, dir);
	}
}

/*
================
FileSystem::idPak
================
*/
bool FileSystem::idPak(char *pak, char *base, int numPaks)
{
	int i;

	for (i = 0; i < NUM_ID_PAKS; i++) {
		if ( !FileSystem::filenameCompare(pak, va("%s/pak%d", base, i)) ) {
			break;
		}
	}
	if (i < numPaks) {
		return true;
	}
	return false;
}

/*
================
FileSystem::checkDirTraversal

Check whether the string contains stuff like "../" to prevent directory traversal bugs
and return true if it does.
================
*/

bool FileSystem::checkDirTraversal(const char *checkdir)
{
	if(strstr(checkdir, "../") || strstr(checkdir, "..\\"))
		return true;
	
	return false;
}

/*
================
FileSystem::invalidGameDir

return true if path is a reference to current directory or directory traversal
or a sub-directory
================
*/
bool FileSystem::invalidGameDir( const char *gamedir ) {
	if ( !strcmp( gamedir, "." ) || !strcmp( gamedir, ".." )
		|| strchr( gamedir, '/' ) || strchr( gamedir, '\\' ) ) {
		return true;
	}

	return false;
}

/*
================
FileSystem::comparePaks

----------------
dlstring == true

Returns a list of pak files that we should download from the server. They all get stored
in the current gamedir and an FileSystem::restart will be fired up after we download them all.

The string is the format:

@remotename@localname [repeat]

static int		fs_numServerReferencedPaks;
static int		fs_serverReferencedPaks[MAX_SEARCH_PATHS];
static char		*fs_serverReferencedPakNames[MAX_SEARCH_PATHS];

----------------
dlstring == false

we are not interested in a download string format, we want something human-readable
(this is used for diagnostics while connecting to a pure server)

================
*/
bool FileSystem::comparePaks( char *neededpaks, int len, bool dlstring ) {
	searchpath_t	*sp;
	bool havepak;
	char *origpos = neededpaks;
	int i;

	if (!fs_numServerReferencedPaks)
		return false; // Server didn't send any pack information along

	*neededpaks = 0;

	for ( i = 0 ; i < fs_numServerReferencedPaks ; i++ )
	{
		// Ok, see if we have this pak file
		havepak = false;

		// never autodownload any of the id paks
		if(FileSystem::idPak(fs_serverReferencedPakNames[i], BASEGAME, NUM_ID_PAKS)
#ifndef STANDALONE
				|| FileSystem::idPak(fs_serverReferencedPakNames[i], BASETA, NUM_TA_PAKS)
#endif
			)
		{
			continue;
		}

		// Make sure the server cannot make us write to non-quake3 directories.
		if(FileSystem::checkDirTraversal(fs_serverReferencedPakNames[i]))
		{
			Com_Printf("WARNING: Invalid download name %s\n", fs_serverReferencedPakNames[i]);
			continue;
		}

		for ( sp = fs_searchpaths ; sp ; sp = sp->next ) {
			if ( sp->pack && sp->pack->checksum == fs_serverReferencedPaks[i] ) {
				havepak = true; // This is it!
				break;
			}
		}

		if ( !havepak && fs_serverReferencedPakNames[i] && *fs_serverReferencedPakNames[i] ) { 
			// Don't got it

			if (dlstring)
			{
				// We need this to make sure we won't hit the end of the buffer or the server could
				// overwrite non-pk3 files on clients by writing so much crap into neededpaks that
				// Q_strcat cuts off the .pk3 extension.

				origpos += strlen(origpos);

				// Remote name
				Q_strcat( neededpaks, len, "@");
				Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
				Q_strcat( neededpaks, len, ".pk3" );

				// Local name
				Q_strcat( neededpaks, len, "@");
				// Do we have one with the same name?
				if ( FS_BaseDir_FileExists_HomeData( va( "%s.pk3", fs_serverReferencedPakNames[i] ) ) )
				{
					char st[MAX_ZPATH];
					// We already have one called this, we need to download it to another name
					// Make something up with the checksum in it
					Com_sprintf( st, sizeof( st ), "%s.%08x.pk3", fs_serverReferencedPakNames[i], fs_serverReferencedPaks[i] );
					Q_strcat( neededpaks, len, st );
				}
				else
				{
					Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
					Q_strcat( neededpaks, len, ".pk3" );
				}

				// Find out whether it might have overflowed the buffer and don't add this file to the
				// list if that is the case.
				if(strlen(origpos) + (origpos - neededpaks) >= len - 1)
				{
					*origpos = '\0';
					break;
				}
			}
			else
			{
				Q_strcat( neededpaks, len, fs_serverReferencedPakNames[i] );
				Q_strcat( neededpaks, len, ".pk3" );
				// Do we have one with the same name?
				if ( FS_BaseDir_FileExists_HomeData( va( "%s.pk3", fs_serverReferencedPakNames[i] ) ) )
				{
					Q_strcat( neededpaks, len, " (local file exists with wrong checksum)");
				}
				Q_strcat( neededpaks, len, "\n");
			}
		}
	}

	if ( *neededpaks ) {
		return true;
	}

	return false; // We have them all
}

/*
================
FileSystem::shutdown

Frees all resources.
================
*/
void FileSystem::shutdown( bool closemfp ) {
	searchpath_t	*p, *next;
	int	i;

	for(i = 0; i < MAX_FILE_HANDLES; i++) {
		if (fsh[i].fileSize) {
			FileSystem::fCloseFile(i);
		}
	}

	// free everything
	for(p = fs_searchpaths; p; p = next)
	{
		next = p->next;

		if(p->pack)
			FS_FreePak(p->pack);
		if (p->dir)
			Z_Free(p->dir);

		Z_Free(p);
	}

	// any FS_ calls will now be an error until reinitialized
	fs_searchpaths = nullptr;

	CommandSystem::removeCommand( "path" );
	CommandSystem::removeCommand( "dir" );
	CommandSystem::removeCommand( "fdir" );
	CommandSystem::removeCommand( "touchFile" );
	CommandSystem::removeCommand( "which" );

#ifdef FS_MISSING
	if (closemfp) {
		fclose(missingFiles);
	}
#endif
}

#ifndef STANDALONE
void Com_AppendCDKey( const char *filename );
void Com_ReadCDKey( const char *filename );
#endif

/*
================
FS_ReorderPurePaks
NOTE TTimo: the reordering that happens here is not reflected in the cvars (\cvarlist *pak*)
  this can lead to misleading situations, see https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=540
================
*/
static void FS_ReorderPurePaks( )
{
	searchpath_t *s;
	int i;
	searchpath_t **p_insert_index, // for linked list reordering
		**p_previous; // when doing the scan

	fs_reordered = false;

	// only relevant when connected to pure server
	if ( !fs_numServerPaks )
		return;

	p_insert_index = &fs_searchpaths; // we insert in order at the beginning of the list
	for ( i = 0 ; i < fs_numServerPaks ; i++ ) {
		p_previous = p_insert_index; // track the pointer-to-current-item
		for (s = *p_insert_index; s; s = s->next) {
			// the part of the list before p_insert_index has been sorted already
			if (s->pack && fs_serverPaks[i] == s->pack->checksum) {
				fs_reordered = true;
				// move this element to the insert list
				*p_previous = s->next;
				s->next = *p_insert_index;
				*p_insert_index = s;
				// increment insert list
				p_insert_index = &s->next;
				break; // iterate to next server pack
			}
			p_previous = &s->next;
		}
	}
}

/*
================
FS_AddPathVar
================
*/
static void FS_AddPathVar( cvar_t *pathVar ) {
	if( !pathVar || !*pathVar->string ) {
		return;
	}

	// Check for duplicates
	for(auto & fs_pathVar : fs_pathVars) {
		if ( fs_pathVar && !Q_stricmp( fs_pathVar->string, pathVar->string ) ) {
			return;
		}
	}

	// Add to first empty slot
	for(auto & fs_pathVar : fs_pathVars) {
		if ( !fs_pathVar ) {
			fs_pathVar = pathVar;
			return;
		}
	}

	Com_Error( ERR_FATAL, "FS_AddPathVar: exhausted fs_pathVars array" );
}

/*
================
FS_InitPathVars
================
*/
static void FS_InitPathVars( ) {
	memset( fs_pathVars, 0, sizeof( fs_pathVars ) );

	FS_AddPathVar( fs_homeconfigpath );
	FS_AddPathVar( fs_homedatapath );
	FS_AddPathVar( fs_homestatepath );
	FS_AddPathVar( fs_basepath );
	FS_AddPathVar( fs_apppath );
	FS_AddPathVar( fs_steampath );
	FS_AddPathVar( fs_gogpath );
	FS_AddPathVar( fs_microsoftstorepath );
}

/*
================
FS_Startup
================
*/
static void FS_Startup( const char *gameName )
{
	cvar_t *fs_homepath = CvarSystem::get("fs_homepath", "", CVAR_INIT|CVAR_PROTECTED);
	const char *configPath = Sys_DefaultHomeConfigPath();
	const char *dataPath = Sys_DefaultHomeDataPath();
	const char *statePath = Sys_DefaultHomeStatePath();

	if(*(fs_homepath)->string) {
		// Setting fs_homepath manually overrides everything else
		configPath = dataPath = statePath = fs_homepath->string;
	} else if(!*configPath || !*dataPath || !*statePath) {
		// #shouldneverhappen; just a sensible fallback
		configPath = dataPath = statePath = Sys_DefaultInstallPath();
	}

	Com_Printf( "----- FS_Startup -----\n" );

	fs_packFiles = 0;

	fs_debug = CvarSystem::get( "fs_debug", "0", 0 );
	fs_basepath = CvarSystem::get ("fs_basepath", Sys_DefaultInstallPath(), CVAR_INIT|CVAR_PROTECTED );
	fs_basegame = CvarSystem::get ("fs_basegame", "", CVAR_INIT );
	fs_homeconfigpath = CvarSystem::get ("fs_homeconfigpath", configPath, CVAR_INIT|CVAR_PROTECTED );
	fs_homedatapath = CvarSystem::get ("fs_homedatapath", dataPath, CVAR_INIT|CVAR_PROTECTED );
	fs_homestatepath = CvarSystem::get ("fs_homestatepath", statePath, CVAR_INIT|CVAR_PROTECTED );
	fs_gamedirvar = CvarSystem::get ("fs_game", "", CVAR_INIT|CVAR_SYSTEMINFO );
	fs_steampath = CvarSystem::get ("fs_steampath", Sys_SteamPath(), CVAR_INIT|CVAR_PROTECTED );
	fs_gogpath = CvarSystem::get ("fs_gogpath", Sys_GogPath(), CVAR_INIT|CVAR_PROTECTED );
	fs_microsoftstorepath = CvarSystem::get ("fs_microsoftstorepath", Sys_MicrosoftStorePath(), CVAR_INIT|CVAR_PROTECTED );

#ifdef __APPLE__
	fs_apppath = CvarSystem::get ("fs_apppath", Sys_DefaultAppPath(), CVAR_INIT|CVAR_PROTECTED );
#endif

	FS_InitPathVars( );

	if (!gameName[0]) {
		CvarSystem::forceReset( "com_basegame" );
	}

	if (!FileSystem::filenameCompare(fs_gamedirvar->string, gameName)) {
		// This is the standard base game. Servers and clients should
		// use "" and not the standard basegame name because this messes
		// up pak file negotiation and lots of other stuff
		CvarSystem::forceReset( "fs_game" );
	}

	if (FileSystem::invalidGameDir(gameName)) {
		Com_Error( ERR_DROP, "Invalid com_basegame '%s'", gameName );
	}
	if (FileSystem::invalidGameDir(fs_basegame->string)) {
		Com_Error( ERR_DROP, "Invalid fs_basegame '%s'", fs_basegame->string );
	}
	if (FileSystem::invalidGameDir(fs_gamedirvar->string)) {
		Com_Error( ERR_DROP, "Invalid fs_game '%s'", fs_gamedirvar->string );
	}

	FileSystem::createPath(fs_homeconfigpath->string);
	FileSystem::createPath(fs_homedatapath->string);
	FileSystem::createPath(fs_homestatepath->string);

	FS_AddGameDirectories(gameName);

	// check for additional base game so mods can be based upon other mods
	if ( fs_basegame->string[0] && Q_stricmp( fs_basegame->string, gameName ) ) {
		FS_AddGameDirectories(fs_basegame->string);
	}

	// check for additional game folder for mods
	if ( fs_gamedirvar->string[0] && Q_stricmp( fs_gamedirvar->string, gameName ) ) {
		FS_AddGameDirectories(fs_gamedirvar->string);
	}

#ifndef STANDALONE
	if (!com_standalone->integer) {
		Com_ReadCDKey(BASEGAME);
		if (fs_gamedirvar->string[0]) {
			Com_AppendCDKey(fs_gamedirvar->string);
		}
	}
#endif

	// add our commands
	CommandSystem::addCommand ("path", FS_Path_f);
	CommandSystem::addCommand ("dir", FS_Dir_f );
	CommandSystem::addCommand ("fdir", FS_NewDir_f );
	CommandSystem::addCommand ("touchFile", FS_TouchFile_f );
	CommandSystem::addCommand ("which", FS_Which_f );

	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=506
	// reorder the pure pk3 files according to server order
	FS_ReorderPurePaks();

	// print the current search paths
	FS_Path_f();

	fs_gamedirvar->modified = false; // We just loaded, it's not modified

	Com_Printf( "----------------------\n" );

#ifdef FS_MISSING
	if (missingFiles == NULL) {
		missingFiles = Sys_FOpen( "\\missing.txt", "ab" );
	}
#endif
	Com_Printf( "%d files in pk3 files\n", fs_packFiles );
}

#ifndef STANDALONE
/*
===================
FS_AppendUserFriendlyPakList
===================
*/
static void FS_AppendUserFriendlyPakList( char *buf, size_t bufsize, int foundPakFiles, int numPakFiles )
{
	int missingCount = 0;
	int missingIndex = 0;
	int missingMin = numPakFiles;
	int missingMax = 0;
	bool missingConsecutive = true;
	int i;

	for ( i = 0; i < numPakFiles; i++ ) {
		if ( foundPakFiles & ( 1 << i ) ) {
			continue;
		}

		missingCount++;

		if ( missingMin > i ) {
			missingMin = i;
		}
		if ( missingMax < i ) {
			missingMax = i;
		}

		if ( i > missingMin && ( foundPakFiles & ( 1<< ( i - 1 ) ) ) ) {
			missingConsecutive = false;
		}
	}

	if ( missingConsecutive && missingMax >= missingMin + 2 ) {
		Q_strcat( buf, bufsize,
				va( " \"pak%d.pk3\" through \"pak%d.pk3\"",
					missingMin, missingMax ) );
	} else {
		for ( i = 0; i < numPakFiles; i++ ) {
			if ( foundPakFiles & (1<<i) ) {
				continue;
			}

			if ( missingIndex > 0 && missingCount > 2 ) {
				Q_strcat( buf, bufsize, "," );
			}

			if ( missingIndex == missingCount - 1 && missingCount > 1 ) {
				Q_strcat( buf, bufsize, " and" );
			}

			Q_strcat( buf, bufsize, va( " \"pak%d.pk3\"", i ) );

			missingIndex++;
		}
	}
}

/*
===================
FS_CheckPak0

Check whether any of the original id pak files is present,
and start up in standalone mode, if there are none and a
different com_basegame was set.
Note: If you're building a game that doesn't depend on the
Q3 media pak0.pk3, you'll want to remove this by defining
STANDALONE in q_shared.h
===================
*/
static void FS_CheckPak0( void )
{
	searchpath_t	*path;
	pack_t		*curpack;
	const char	*pakBasename;
	bool founddemo = false;
	unsigned int foundPak = 0, foundTA = 0;
	bool installHome = false;
	char *installPath;

	for( path = fs_searchpaths; path; path = path->next )
	{
		if(!path->pack)
			continue;

		curpack = path->pack;
		pakBasename = curpack->pakBasename;

		if(!Q_stricmpn( curpack->pakGamename, "demoq3", MAX_OSPATH )
				&& !Q_stricmpn( pakBasename, "pak0", MAX_OSPATH ))
		{
			if(curpack->checksum == DEMO_PAK0_CHECKSUM)
				founddemo = true;
		}

		else if(!Q_stricmpn( curpack->pakGamename, BASEGAME, MAX_OSPATH )
				&& strlen(pakBasename) == 4 && !Q_stricmpn( pakBasename, "pak", 3 )
				&& pakBasename[3] >= '0' && pakBasename[3] <= '0' + NUM_ID_PAKS - 1)
		{
			if( curpack->checksum != pak_checksums[pakBasename[3]-'0'] )
			{
				if(pakBasename[3] == '0')
				{
					Com_Printf("\n\n"
							"**************************************************\n"
							"WARNING: " BASEGAME "/pak0.pk3 is present but its checksum (%u)\n"
							"is not correct. Please re-copy pak0.pk3 from your\n"
							"legitimate Q3 CDROM.\n"
							"**************************************************\n\n\n",
							curpack->checksum );
				}
				else
				{
					Com_Printf("\n\n"
							"**************************************************\n"
							"WARNING: " BASEGAME "/pak%d.pk3 is present but its checksum (%u)\n"
							"is not correct. Please re-install the point release\n"
							"**************************************************\n\n\n",
							pakBasename[3]-'0', curpack->checksum );
				}
			}

			foundPak |= 1<<(pakBasename[3]-'0');
		}
		else if(!Q_stricmpn(curpack->pakGamename, BASETA, MAX_OSPATH)
				&& strlen(pakBasename) == 4 && !Q_stricmpn(pakBasename, "pak", 3)
				&& pakBasename[3] >= '0' && pakBasename[3] <= '0' + NUM_TA_PAKS - 1)

		{
			if(curpack->checksum != missionpak_checksums[pakBasename[3]-'0'])
			{
				Com_Printf("\n\n"
						"**************************************************\n"
						"WARNING: " BASETA "/pak%d.pk3 is present but its checksum (%u)\n"
						"is not correct. Please re-install Team Arena\n"
						"**************************************************\n\n\n",
						pakBasename[3]-'0', curpack->checksum );				
			}

			foundTA |= 1 << (pakBasename[3]-'0');
		}
		else
		{
			int index;

			// Finally check whether this pak's checksum is listed because the user tried
			// to trick us by renaming the file, and set foundPak's highest bit to indicate this case.

			for(index = 0; index < ARRAY_LEN(pak_checksums); index++)
			{
				if(curpack->checksum == pak_checksums[index])
				{
					Com_Printf("\n\n"
							"**************************************************\n"
							"WARNING: %s is renamed pak file %s%cpak%d.pk3\n"
							"Running in standalone mode won't work\n"
							"Please rename, or remove this file\n"
							"**************************************************\n\n\n",
							curpack->pakFilename, BASEGAME, PATH_SEP, index);


					foundPak |= 0x80000000;
				}
			}

			for(index = 0; index < ARRAY_LEN(missionpak_checksums); index++)
			{
				if(curpack->checksum == missionpak_checksums[index])
				{
					Com_Printf("\n\n"
							"**************************************************\n"
							"WARNING: %s is renamed pak file %s%cpak%d.pk3\n"
							"Running in standalone mode won't work\n"
							"Please rename, or remove this file\n"
							"**************************************************\n\n\n",
							curpack->pakFilename, BASETA, PATH_SEP, index);

					foundTA |= 0x80000000;
				}
			}
		}
	}

	if(!foundPak && !foundTA && Q_stricmp(com_basegame->string, BASEGAME))
	{
		CvarSystem::set("com_standalone", "1");
	}
	else
		CvarSystem::set("com_standalone", "0");

	if(!com_standalone->integer)
	{
		if(!(foundPak & 0x01))
		{
			if(founddemo)
			{
				Com_Printf( "\n\n"
						"**************************************************\n"
						"WARNING: It looks like you're using pak0.pk3\n"
						"from the demo. This may work fine, but it is not\n"
						"guaranteed or supported.\n"
						"**************************************************\n\n\n" );

				foundPak |= 0x01;
			}
		}
	}

#if defined(__linux__)
	{
		const char *p;

		// Users can't write to the default Flatpak fs_basepath
		if( ( p = getenv( "FLATPAK_ID" ) ) != NULL && *p != '\0' )
			installHome = true;
	}
#elif defined(__APPLE__)
	// If we're running from an .app, it makes more sense to recommend
	// using fs_homepath as fs_basepath is likely not suitable
	if( strstr( fs_apppath->string, "Contents/MacOS" ) )
		installHome = true;
#endif

	if(installHome)
		installPath = fs_homedatapath->string;
	else
		installPath = fs_basepath->string;

	if(!com_standalone->integer && (foundPak & ((1<<NUM_ID_PAKS)-1)) != ((1<<NUM_ID_PAKS)-1))
	{
		char errorText[MAX_STRING_CHARS] = "";
		char gamePath[MAX_OSPATH];

		Com_sprintf(gamePath, sizeof(gamePath), "%s%c%s%c",
			installPath, PATH_SEP, BASEGAME, PATH_SEP);

		Q_strcat(errorText, sizeof(errorText),
				"Quake 3 data files are missing. Please copy");

		FS_AppendUserFriendlyPakList(errorText, sizeof(errorText), foundPak, NUM_ID_PAKS);

		Q_strcat(errorText, sizeof(errorText),
				va(" from the \"%s\" directory in your Quake 3 install or CD-ROM to:\n\n"
				"%s\n\n", BASEGAME, gamePath));

		Q_strcat(errorText, sizeof(errorText),
				"Quake 3 must be purchased to legitimately obtain pak0. "
				"Quake 3 1.32 point release files (pak1 through pak8) "
				"are freely available. For details see:\n\n"
				"https://buy.ioquake3.org\n\n");

		if(installHome)
		{
			Q_strcat(errorText, sizeof(errorText),
					va("Also check that every file "
						"in the \"%s\" directory is present and readable", BASEGAME));
		}
		else
		{
			Q_strcat(errorText, sizeof(errorText),
					va("Also check that your ioq3 executable is in "
						"the correct place and that every file "
						"in the \"%s\" directory is present and readable", BASEGAME));
		}

		Sys_OpenFolderInFileManager(gamePath, true);
		Com_Error(ERR_FATAL, "%s", errorText);
	}

	if(!com_standalone->integer && (foundTA & ((1<<NUM_TA_PAKS)-1)) != ((1<<NUM_TA_PAKS)-1)
			&& (!Q_stricmp(fs_gamedirvar->string, BASETA) || !Q_stricmp(fs_basegame->string, BASETA)))
	{
		char errorText[MAX_STRING_CHARS] = "";
		char gamePath[MAX_OSPATH];

		Com_sprintf(gamePath, sizeof(gamePath), "%s%c%s%c",
			installPath, PATH_SEP, BASETA, PATH_SEP);

		Q_strcat(errorText, sizeof(errorText),
				"Quake 3 Team Arena data files are missing. Please copy");

		FS_AppendUserFriendlyPakList(errorText, sizeof(errorText), foundTA, NUM_TA_PAKS);

		Q_strcat(errorText, sizeof(errorText),
				va(" from the \"%s\" directory in your Quake 3 Team Arena install or CD-ROM to:\n\n"
				"%s\n\n", BASETA, gamePath));

		Q_strcat(errorText, sizeof(errorText),
				"Quake 3 Team Arena must be purchased to legitimately obtain pak0. "
				"Quake 3 Team Arena point release files (pak1 through pak3) "
				"are freely available. For details see:\n\n"
				"https://buy.ioquake3.org\n\n");

		if(installHome)
		{
			Q_strcat(errorText, sizeof(errorText),
					va("Also check that every file "
						"in the \"%s\" directory is present and readable", BASETA));
		}
		else
		{
			Q_strcat(errorText, sizeof(errorText),
					va("Also check that your ioq3 executable is in "
						"the correct place and that every file "
						"in the \"%s\" directory is present and readable", BASETA));
		}

		Sys_OpenFolderInFileManager(gamePath, true);
		Com_Error(ERR_FATAL, "%s", errorText);
	}
}
#endif

/*
=====================
FileSystem::loadedPakChecksums

Returns a space separated string containing the checksums of all loaded pk3 files.
Servers with sv_pure set will get this string and pass it to clients.
=====================
*/
const char *FileSystem::loadedPakChecksums( ) {
	static char	info[BIG_INFO_STRING];
	searchpath_t	*search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file? 
		if ( !search->pack ) {
			continue;
		}

		Q_strcat( info, sizeof( info ), va("%i ", search->pack->checksum ) );
	}

	return info;
}

/*
=====================
FileSystem::loadedPakNames

Returns a space separated string containing the names of all loaded pk3 files.
Servers with sv_pure set will get this string and pass it to clients.
=====================
*/
const char *FileSystem::loadedPakNames( ) {
	static char	info[BIG_INFO_STRING];
	searchpath_t	*search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( !search->pack ) {
			continue;
		}

		if (*info) {
			Q_strcat(info, sizeof( info ), " " );
		}
		Q_strcat( info, sizeof( info ), search->pack->pakBasename );
	}

	return info;
}

/*
=====================
FileSystem::loadedPakPureChecksums

Returns a space separated string containing the pure checksums of all loaded pk3 files.
Servers with sv_pure use these checksums to compare with the checksums the clients send
back to the server.
=====================
*/
const char *FileSystem::loadedPakPureChecksums( ) {
	static char	info[BIG_INFO_STRING];
	searchpath_t	*search;

	info[0] = 0;

	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file? 
		if ( !search->pack ) {
			continue;
		}

		Q_strcat( info, sizeof( info ), va("%i ", search->pack->pure_checksum ) );
	}

	return info;
}

/*
=====================
FileSystem::referencedPakChecksums

Returns a space separated string containing the checksums of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded. 
=====================
*/
const char *FileSystem::referencedPakChecksums( ) {
	static char	info[BIG_INFO_STRING];
	searchpath_t *search;

	info[0] = 0;


	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {
			if (search->pack->referenced || Q_stricmpn(search->pack->pakGamename, com_basegame->string, strlen(com_basegame->string))) {
				Q_strcat( info, sizeof( info ), va("%i ", search->pack->checksum ) );
			}
		}
	}

	return info;
}

/*
=====================
FileSystem::referencedPakPureChecksums

Returns a space separated string containing the pure checksums of all referenced pk3 files.
Servers with sv_pure set will get this string back from clients for pure validation 

The string has a specific order, "cgame ui @ ref1 ref2 ref3 ..."
=====================
*/
const char *FileSystem::referencedPakPureChecksums( ) {
	static char	info[BIG_INFO_STRING];
	searchpath_t	*search;
	int nFlags, numPaks, checksum;

	info[0] = 0;

	checksum = fs_checksumFeed;
	numPaks = 0;
	for (nFlags = FS_CGAME_REF; nFlags; nFlags = nFlags >> 1) {
		if (nFlags & FS_GENERAL_REF) {
			// add a delimter between must haves and general refs
			//Q_strcat(info, sizeof(info), "@ ");
			info[strlen(info)+1] = '\0';
			info[strlen(info)+2] = '\0';
			info[strlen(info)] = '@';
			info[strlen(info)] = ' ';
		}
		for ( search = fs_searchpaths ; search ; search = search->next ) {
			// is the element a pak file and has it been referenced based on flag?
			if ( search->pack && (search->pack->referenced & nFlags)) {
				Q_strcat( info, sizeof( info ), va("%i ", search->pack->pure_checksum ) );
				if (nFlags & (FS_CGAME_REF | FS_UI_REF)) {
					break;
				}
				checksum ^= search->pack->pure_checksum;
				numPaks++;
			}
		}
	}
	// last checksum is the encoded number of referenced pk3s
	checksum ^= numPaks;
	Q_strcat( info, sizeof( info ), va("%i ", checksum ) );

	return info;
}

/*
=====================
FileSystem::referencedPakNames

Returns a space separated string containing the names of all referenced pk3 files.
The server will send this to the clients so they can check which files should be auto-downloaded. 
=====================
*/
const char *FileSystem::referencedPakNames( ) {
	static char	info[BIG_INFO_STRING];
	searchpath_t	*search;

	info[0] = 0;

	// we want to return ALL pk3's from the fs_game path
	// and referenced one's from baseq3
	for ( search = fs_searchpaths ; search ; search = search->next ) {
		// is the element a pak file?
		if ( search->pack ) {
			if (search->pack->referenced || Q_stricmpn(search->pack->pakGamename, com_basegame->string, strlen(com_basegame->string))) {
				if (*info) {
					Q_strcat(info, sizeof( info ), " " );
				}
				Q_strcat( info, sizeof( info ), search->pack->pakGamename );
				Q_strcat( info, sizeof( info ), "/" );
				Q_strcat( info, sizeof( info ), search->pack->pakBasename );
			}
		}
	}

	return info;
}

/*
=====================
FileSystem::clearPakReferences
=====================
*/
void FileSystem::clearPakReferences( int flags ) {
	searchpath_t *search;

	if ( !flags ) {
		flags = -1;
	}
	for ( search = fs_searchpaths; search; search = search->next ) {
		// is the element a pak file and has it been referenced?
		if ( search->pack ) {
			search->pack->referenced &= ~flags;
		}
	}
}


/*
=====================
FileSystem::pureServerSetLoadedPaks

If the string is empty, all data sources will be allowed.
If not empty, only pk3 files that match one of the space
separated checksums will be checked for files, with the
exception of .cfg and .dat files.
=====================
*/
void FileSystem::pureServerSetLoadedPaks( const char *pakSums, const char *pakNames ) {
	int		i, c, d;

	CommandSystem::tokenizeString( pakSums );

	c = CommandSystem::argc();
	if ( c > MAX_SEARCH_PATHS ) {
		c = MAX_SEARCH_PATHS;
	}

	fs_numServerPaks = c;

	for ( i = 0 ; i < c ; i++ ) {
		fs_serverPaks[i] = atoi( CommandSystem::argv( i ) );
	}

	if (fs_numServerPaks) {
		Com_DPrintf( "Connected to a pure server.\n" );
	}
	else
	{
		if (fs_reordered)
		{
			// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=540
			// force a restart to make sure the search order will be correct
			Com_DPrintf( "FS search reorder is required\n" );
			FileSystem::restart(fs_checksumFeed);
			return;
		}
	}

	for ( i = 0 ; i < c ; i++ ) {
		if (fs_serverPakNames[i]) {
			Z_Free(fs_serverPakNames[i]);
		}
		fs_serverPakNames[i] = nullptr;
	}
	if ( pakNames && *pakNames ) {
		CommandSystem::tokenizeString( pakNames );

		d = CommandSystem::argc();
		if ( d > MAX_SEARCH_PATHS ) {
			d = MAX_SEARCH_PATHS;
		}

		for ( i = 0 ; i < d ; i++ ) {
			fs_serverPakNames[i] = CopyString( CommandSystem::argv( i ) );
		}
	}
}

/*
=====================
FileSystem::pureServerSetReferencedPaks

The checksums and names of the pk3 files referenced at the server
are sent to the client and stored here. The client will use these
checksums to see if any pk3 files need to be auto-downloaded. 
=====================
*/
void FileSystem::pureServerSetReferencedPaks( const char *pakSums, const char *pakNames ) {
	int		i, c, d = 0;

	CommandSystem::tokenizeString( pakSums );

	c = CommandSystem::argc();
	if ( c > MAX_SEARCH_PATHS ) {
		c = MAX_SEARCH_PATHS;
	}

	for ( i = 0 ; i < c ; i++ ) {
		fs_serverReferencedPaks[i] = atoi( CommandSystem::argv( i ) );
	}

	for (i = 0 ; i < ARRAY_LEN(fs_serverReferencedPakNames); i++)
	{
		if(fs_serverReferencedPakNames[i])
			Z_Free(fs_serverReferencedPakNames[i]);

		fs_serverReferencedPakNames[i] = nullptr;
	}

	if ( pakNames && *pakNames ) {
		CommandSystem::tokenizeString( pakNames );

		d = CommandSystem::argc();

		if(d > c)
			d = c;

		for ( i = 0 ; i < d ; i++ ) {
			fs_serverReferencedPakNames[i] = CopyString( CommandSystem::argv( i ) );
		}
	}

	// ensure that there are as many checksums as there are pak names.
	if(d < c)
		c = d;

	fs_numServerReferencedPaks = c;	
}

/*
================
FileSystem::initFilesystem

Called only at initial startup, not when the filesystem
is resetting due to a game change
================
*/
void FileSystem::initFilesystem( ) {
	// allow command line parms to override our defaults
	// we have to specially handle this, because normal command
	// line variable sets don't happen until after the filesystem
	// has already been initialized
	Com_StartupVariable("fs_basepath");
	Com_StartupVariable("fs_homepath");
	Com_StartupVariable("fs_homeconfigpath");
	Com_StartupVariable("fs_homedatapath");
	Com_StartupVariable("fs_homestatepath");
	Com_StartupVariable("fs_game");

	if(!FileSystem::filenameCompare(CvarSystem::variableString("fs_game"), com_basegame->string))
		CvarSystem::set("fs_game", "");

	// try to start up normally
	FS_Startup(com_basegame->string);

#ifndef STANDALONE
	FS_CheckPak0( );
#endif

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	if ( FileSystem::readFile( "default.cfg", nullptr ) <= 0 ) {
		Com_Error( ERR_FATAL, "Couldn't load default.cfg" );
	}

	Q_strncpyz(lastValidBase, fs_basepath->string, sizeof(lastValidBase));
	Q_strncpyz(lastValidComBaseGame, com_basegame->string, sizeof(lastValidComBaseGame));
	Q_strncpyz(lastValidFsBaseGame, fs_basegame->string, sizeof(lastValidFsBaseGame));
	Q_strncpyz(lastValidGame, fs_gamedirvar->string, sizeof(lastValidGame));
}


/*
================
FileSystem::restart
================
*/
void FileSystem::restart( int checksumFeed ) {
	const char *lastGameDir;

	// free anything we currently have loaded
	FileSystem::shutdown(false);

	// set the checksum feed
	fs_checksumFeed = checksumFeed;

	// clear pak references
	FileSystem::clearPakReferences(0);

	// try to start up normally
	FS_Startup(com_basegame->string);

#ifndef STANDALONE
	FS_CheckPak0( );
#endif

	// if we can't find default.cfg, assume that the paths are
	// busted and error out now, rather than getting an unreadable
	// graphics screen when the font fails to load
	if ( FileSystem::readFile( "default.cfg", nullptr ) <= 0 ) {
		// this might happen when connecting to a pure server not using BASEGAME/pak0.pk3
		// (for instance a TA demo server)
		if (lastValidBase[0]) {
			FileSystem::pureServerSetLoadedPaks("", "");
			CvarSystem::set("fs_basepath", lastValidBase);
			CvarSystem::set("com_basegame", lastValidComBaseGame);
			CvarSystem::set("fs_basegame", lastValidFsBaseGame);
			CvarSystem::set("fs_game", lastValidGame);
			lastValidBase[0] = '\0';
			lastValidComBaseGame[0] = '\0';
			lastValidFsBaseGame[0] = '\0';
			lastValidGame[0] = '\0';
			FileSystem::restart(checksumFeed);
			Com_Error( ERR_DROP, "Invalid game folder" );
			return;
		}
		Com_Error( ERR_FATAL, "Couldn't load default.cfg" );
	}

	lastGameDir = ( lastValidGame[0] ) ? lastValidGame : lastValidComBaseGame;

	if ( Q_stricmp( FileSystem::getCurrentGameDir(), lastGameDir ) ) {
		Sys_RemovePIDFile( lastGameDir );
		Sys_InitPIDFile( FileSystem::getCurrentGameDir() );

		// skip the q3config.cfg if "safe" is on the command line
		if ( !Com_SafeMode() ) {
			CommandBuffer::addText ("exec " Q3CONFIG_CFG "\n");
		}
	}

	Q_strncpyz(lastValidBase, fs_basepath->string, sizeof(lastValidBase));
	Q_strncpyz(lastValidComBaseGame, com_basegame->string, sizeof(lastValidComBaseGame));
	Q_strncpyz(lastValidFsBaseGame, fs_basegame->string, sizeof(lastValidFsBaseGame));
	Q_strncpyz(lastValidGame, fs_gamedirvar->string, sizeof(lastValidGame));

}

/*
=================
FileSystem::conditionalRestart

Restart if necessary
Return true if restarting due to game directory changed, false otherwise
=================
*/
bool FileSystem::conditionalRestart(int checksumFeed, bool disconnect)
{
	if(fs_gamedirvar->modified)
	{
		if(FileSystem::filenameCompare(lastValidGame, fs_gamedirvar->string) &&
				(*lastValidGame || FileSystem::filenameCompare(fs_gamedirvar->string, com_basegame->string)) &&
				(*fs_gamedirvar->string || FileSystem::filenameCompare(lastValidGame, com_basegame->string)))
		{
			Com_GameRestart(checksumFeed, disconnect);
			return true;
		}
		else
			fs_gamedirvar->modified = false;
	}

	if(checksumFeed != fs_checksumFeed)
		FileSystem::restart(checksumFeed);
	else if(fs_numServerPaks && !fs_reordered)
		FS_ReorderPurePaks();

	return false;
}

/*
========================================================================================

Handle based file calls for virtual machines

========================================================================================
*/

int		FileSystem::fOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	int		r;
	bool	sync;

	sync = false;

	switch( mode ) {
		case FS_READ:
			r = FileSystem::fOpenFileRead( qpath, f, true );
			break;
		case FS_WRITE:
			*f = FileSystem::fOpenFileWrite_HomeData( qpath );
			r = 0;
			if (*f == 0) {
				r = -1;
			}
			break;
		case FS_APPEND_SYNC:
			sync = true;
		case FS_APPEND:
			*f = FileSystem::fOpenFileAppend_HomeData( qpath );
			r = 0;
			if (*f == 0) {
				r = -1;
			}
			break;
		default:
			Com_Error( ERR_FATAL, "FileSystem::fOpenFileByMode: bad mode" );
			return -1;
	}

	if (!f) {
		return r;
	}

	if ( *f ) {
		fsh[*f].fileSize = r;
	}
	fsh[*f].handleSync = sync;

	return r;
}

int		FileSystem::fTell( fileHandle_t f ) {
	int pos;
	if (fsh[f].zipFile == true) {
		pos = unztell(fsh[f].handleFiles.file.z);
	} else {
		pos = ftell(fsh[f].handleFiles.file.o);
	}
	return pos;
}

void	FileSystem::flush( fileHandle_t f ) {
	fflush(fsh[f].handleFiles.file.o);
}

void	FileSystem::filenameCompletion( const char *dir, const char *ext, char *filter,
		bool stripExt, void(*callback)(const char *s), bool allowNonPureFilesOnDisk ) {
	char	**filenames;
	int		nfiles;
	int		i;
	char	filename[ MAX_STRING_CHARS ];

	filenames = FS_ListFilteredFiles( dir, ext, filter,
		&nfiles, true, allowNonPureFilesOnDisk );

	FS_SortFileList( filenames, nfiles );

	for( i = 0; i < nfiles; i++ ) {
		FS_ConvertPath( filenames[ i ] );
		Q_strncpyz( filename, filenames[ i ], MAX_STRING_CHARS );

		if( stripExt ) {
			COM_StripExtension(filename, filename, sizeof(filename));
		}

		callback( filename );
	}
	FileSystem::freeFileList( filenames );
}

const char *FileSystem::getCurrentGameDir()
{
	if(fs_gamedirvar->string[0])
		return fs_gamedirvar->string;

	return com_basegame->string;
}
