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
// qcommon.h -- definitions common between client and server, but not game.or ref modules
#ifndef _QCOMMON_H_
#define _QCOMMON_H_

#include "../qcommon/cm_public.h"

//#define	PRE_RELEASE_DEMO

//============================================================================

//
// msg.c
//
struct usercmd_s;
struct entityState_s;
struct playerState_s;

class Message
{
public:
	Message() = default;
	Message( byte *data, int length );
	// an out of band message: byte aligned, not Huffman coded
	static Message outOfBand( byte *data, int length );

	byte *data() const { return data_; }
	int size() const { return curSize_; }
	int maxSize() const { return maxSize_; }
	int readCount() const { return readCount_; }
	int bit() const { return bit_; }
	bool overflowed() const { return overflowed_; }
	bool outOfBand() const { return oob_; }
	void setData( byte *data ) { data_ = data; }
	void setSize( int size ) { curSize_ = size; }
	void setReadCount( int count ) { readCount_ = count; }
	void setBit( int bit ) { bit_ = bit; }
	void setOutOfBand( bool oob ) { oob_ = oob; }
	void setAllowOverflow( bool allow ) { allowOverflow_ = allow; }

	void clear();
	void bitstream();
	void beginReading();
	void beginReadingOOB();
	void copy( byte *data, int length, Message *src );
	void writeBits( int value, int bits );
	int readBits( int bits );
	void writeChar( int c );
	void writeByte( int c );
	void writeData( const void *data, int length );
	void writeShort( int c );
	void writeLong( int c );
	void writeFloat( float f );
	void writeString( const char *s );
	void writeBigString( const char *s );
	void writeAngle( float f );
	void writeAngle16( float f );
	int readChar();
	int readByte();
	int lookaheadByte();
	int readShort();
	int readLong();
	float readFloat();
	char * readString();
	char * readBigString();
	char * readStringLine();
	float readAngle16();
	void readData( void *data, int len );
	void writeDeltaKey( int key, int oldV, int newV, int bits );
	int readDeltaKey( int key, int oldV, int bits );
	void writeDeltaKeyFloat( int key, float oldV, float newV );
	float readDeltaKeyFloat( int key, float oldV );
	void writeDeltaUsercmdKey( int key, usercmd_t *from, usercmd_t *to );
	void readDeltaUsercmdKey( int key, usercmd_t *from, usercmd_t *to );
	void writeDeltaEntity( struct entityState_s *from, struct entityState_s *to, bool force );
	bool writeEntityUpdate( struct entityState_s *from, struct entityState_s *to, bool create, uint64_t forced, uint64_t *written );
	void readDeltaEntity( entityState_t *from, entityState_t *to, int number );
	void writeDeltaPlayerstate( struct playerState_s *from, struct playerState_s *to );
	void readDeltaPlayerstate( playerState_t *from, playerState_t *to );

	static int hashKey( const char *string, int maxlen );

private:
	bool	allowOverflow_ = false;	// if false, do a Common::error
	bool	overflowed_ = false;	// set to true if the buffer size failed (with allowOverflow_ set)
	bool	oob_ = false;			// byte aligned and not Huffman coded
	byte	*data_ = nullptr;
	int		maxSize_ = 0;
	int		curSize_ = 0;
	int		readCount_ = 0;
	int		bit_ = 0;				// for bitwise reads and writes

	bool writeEntity( struct entityState_s *from, struct entityState_s *to, bool force, int create, uint64_t forced, uint64_t *written );
};


void MSG_ReportChangeVectors_f( );
void MSG_InitHuffStats( );
extern int msg_huffstatsHold;

//============================================================================

/*
==============================================================

NET

==============================================================
*/

inline constexpr int NET_ENABLEV4 = 0x01;
inline constexpr int NET_ENABLEV6 = 0x02;
// if this flag is set, always attempt ipv6 connections instead of ipv4 if a v6 address is found.
inline constexpr int NET_PRIOV6 = 0x04;
// disables ipv6 multicast support if set.
inline constexpr int NET_DISABLEMCAST = 0x08;


inline constexpr int PACKET_BACKUP = 32; // number of old messages that must be kept on client and
							// server for delta comrpession and ping estimation
#define	PACKET_MASK		(PACKET_BACKUP-1)

inline constexpr int MAX_PACKET_USERCMDS = 32; // max number of usercmd_t in a packet

inline constexpr int MAX_SNAPSHOT_ENTITIES = 2048;

inline constexpr int PORT_ANY = -1;

inline constexpr int MAX_RELIABLE_COMMANDS = 64; // max string commands buffered for restransmit

enum netadrtype_t {
	NA_BAD = 0,					// an address lookup failed
	NA_BOT,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IP6,
	NA_MULTICAST6,
	NA_UNSPEC
};

enum netsrc_t {
	NS_CLIENT,
	NS_SERVER
};

inline constexpr int NET_ADDRSTRMAXLEN = 48; // maximum length of an IPv6 address string including trailing '\0'
struct netadr_t {
	netadrtype_t	type;

	byte	ip[4];
	byte	ip6[16];

	unsigned short	port;
	unsigned long	scope_id;	// Needed for IPv6 link-local addresses
};




inline constexpr int MAX_MSGLEN = 0x20000; // max length of a message, which may
											// be fragmented into multiple packets

inline constexpr int MAX_DOWNLOAD_WINDOW = 48; // ACK window of 48 download chunks. Cannot set this higher, or clients
						// will overflow the reliable commands buffer
inline constexpr int MAX_DOWNLOAD_BLKSIZE = 1024; // 896 byte block chunks

#define NETCHAN_GENCHECKSUM(challenge, sequence) ((challenge) ^ ((sequence) * (challenge)))

/*
Netchan handles packet fragmentation and out of order / duplicate suppression
*/

class NetChannel
{
public:
	void setup( netsrc_t sock, netadr_t adr, int qport, int challenge, bool compat );
	void transmit( int length, const byte *data );
	void transmitNextFragment();
	bool process( Message *msg );

	const netadr_t& remoteAddress() const { return remoteAddress_; }
	void setRemoteType( netadrtype_t type ) { remoteAddress_.type = type; }
	void setRemotePort( unsigned short port ) { remoteAddress_.port = port; }
	int qport() const { return qport_; }
	int dropped() const { return dropped_; }
	int incomingSequence() const { return incomingSequence_; }
	int outgoingSequence() const { return outgoingSequence_; }
	bool unsentFragments() const { return unsentFragments_; }
	int lastSentTime() const { return lastSentTime_; }
	int lastSentSize() const { return lastSentSize_; }
#ifdef LEGACY_PROTOCOL
	bool compat() const { return compat_; }
#endif

private:
	netsrc_t	sock_ = NS_CLIENT;

	int			dropped_ = 0;			// between last packet and previous

	netadr_t	remoteAddress_ = {};
	int			qport_ = 0;				// qport value to write when transmitting

	// sequencing variables
	int			incomingSequence_ = 0;
	int			outgoingSequence_ = 0;

	// incoming fragment assembly buffer
	int			fragmentSequence_ = 0;
	int			fragmentLength_ = 0;
	byte		fragmentBuffer_[MAX_MSGLEN] = {};

	// outgoing fragment buffer
	// we need to space out the sending of large fragmented messages
	bool		unsentFragments_ = false;
	int			unsentFragmentStart_ = 0;
	int			unsentLength_ = 0;
	byte		unsentBuffer_[MAX_MSGLEN] = {};

	int			challenge_ = 0;
	int			lastSentTime_ = 0;
	int			lastSentSize_ = 0;

#ifdef LEGACY_PROTOCOL
	bool		compat_ = false;
#endif
};


void Netchan_Init( int qport );




/*
==============================================================

PROTOCOL

==============================================================
*/

inline constexpr int PROTOCOL_VERSION = 9;
// 1.31 - 67

// maintain a list of compatible protocols for demo playing
// NOTE: that stuff only works with two digits protocols
extern int demo_protocols[];

#if !defined UPDATE_SERVER_NAME && !defined STANDALONE
#define	UPDATE_SERVER_NAME	"update.quake3arena.com"
#endif
// override on command line, config files etc.
#ifndef MASTER_SERVER_NAME
#define MASTER_SERVER_NAME	"master.quake3arena.com"
#endif

#ifndef STANDALONE
  #ifndef AUTHORIZE_SERVER_NAME
    #define	AUTHORIZE_SERVER_NAME	"authorize.quake3arena.com"
  #endif
  #ifndef PORT_AUTHORIZE
  #define	PORT_AUTHORIZE		27952
  #endif
#endif

inline constexpr int PORT_MASTER = 27950;
inline constexpr int PORT_UPDATE = 27951;
inline constexpr int PORT_SERVER = 27960;
inline constexpr int NUM_SERVER_PORTS = 4; // broadcast scan this many ports after
									// PORT_SERVER so a single machine can
									// run multiple servers


// the svc_strings[] array in cl_parse.c should mirror this
//
// server to client
//
enum svc_ops_e {
	svc_bad,
	svc_nop,
	svc_gamestate,
	svc_configstring,			// [short] [string] only in gamestate messages
	svc_baseline,				// only in gamestate messages
	svc_serverCommand,			// [string] to be executed by client game module
	svc_download,				// [short] size [size bytes]
	svc_snapshot,
	svc_EOF,

// new commands, supported only by ioquake3 protocol but not legacy
	svc_voipSpeex,     // not wrapped in USE_VOIP, so this value is reserved.
	svc_voipOpus,      //
};


//
// client to server
//
enum clc_ops_e {
	clc_bad,
	clc_nop, 		
	clc_move,				// [[usercmd_t]
	clc_moveNoDelta,		// [[usercmd_t]
	clc_clientCommand,		// [string] message
	clc_EOF,

// new commands, supported only by ioquake3 protocol but not legacy
	clc_voipSpeex,   // not wrapped in USE_VOIP, so this value is reserved.
	clc_voipOpus,    //
};

/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but entire text
files can be execed.

*/

// allocates an initial text buffer that will grow as needed

// Adds command text at the end of the buffer, does NOT add a final \n

// this can be used in place of either CommandBuffer::addText or CommandBuffer::insertText

// Pulls off \n terminated lines of text from the command buffer and sends
// them through CommandSystem::executeString.  Stops when the buffer is empty.
// Normally called once per frame, but may be explicitly invoked.
// Do not call inside a command function, or current args will be destroyed.

//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/



// called by the init functions of other parts of the program to
// register commands and functions to call for them.
// The cmd_name is referenced later, so it should not be in temp memory
// if function is NULL, the command will be forwarded to the server
// as a clc_clientCommand instead of executed locally



// don't allow VMs to remove system commands

// callback with each valid string

// The functions that execute commands get their parameters with these
// functions. CommandSystem::argv () will return an empty string, not a NULL
// if arg > argc, so string operations are allways safe.

// Takes a null terminated string.  Does not need to be /n terminated.
// breaks the string up into arg tokens.

// Parses a single line of text into arguments and tries to execute it
// as if it was typed at the console


/*
==============================================================

CVAR

==============================================================
*/

/*

cvar_t variables are used to hold scalar or string variables that can be changed
or displayed at the console or prog code as well as accessed directly
in C code.

The user can access cvars from the console in three ways:
r_draworder			prints the current value
r_draworder 0		sets the current value to 0
set r_draworder 0	as above, but creates the cvar if not present

Cvars are restricted from having the same names as commands to keep this
interface from being ambiguous.

The are also occasionally used to communicated information between different
modules of the program.

*/

// creates the variable if it doesn't exist, or returns the existing one
// if it exists, the value will not be changed, but flags will be ORed in
// that allows variables to be unarchived without needing bitflags
// if value is "", the value will not override a previously set value.

// basically a slightly modified CvarSystem::get for the interpreted modules

// updates an interpreted modules' version of a cvar

// will create the variable with no flags if it doesn't exist

// same as CvarSystem::set, but allows more control over setting of cvar

// sometimes we set variables from an untrusted source: fail if flags & CVAR_PROTECTED

// don't set the cvar immediately

// expands value to a string and calls CvarSystem::set/CvarSystem::setSafe

// returns 0 if not defined or non numeric

// returns an empty string if not defined

// returns CVAR_NONEXISTENT if cvar doesn't exist or the flags of that particular CVAR.

// callback with each valid string


// reset all testing vars to a safe value

// called by CommandSystem::executeString when CommandSystem::argv(0) doesn't match a known
// command.  Returns true if the command was a variable reference that
// was handled. (print or change)

// writes lines containing "set variable value" for all variables
// with the archive flag set to true.


// returns an info string containing all the cvars that have the given bit set
// in their flags ( CVAR_USERINFO, CVAR_SERVERINFO, CVAR_SYSTEMINFO, etc )



// whenever a cvar is modifed, its flags will be OR'd into this, so
// a single check can determine if any CVAR_USERINFO, CVAR_SERVERINFO,
// etc, variables have been modified since the last check.  The bit
// can then be cleared to allow another change detection.

/*
==============================================================

FILESYSTEM

No stdio calls should be used by any part of the game, because
we need to deal with all sorts of directory and seperator char
issues.
==============================================================
*/

// referenced flags
// these are in loop specific order so don't change the order
inline constexpr int FS_GENERAL_REF = 0x01;
inline constexpr int FS_UI_REF = 0x02;
inline constexpr int FS_CGAME_REF = 0x04;
// number of id paks that will never be autodownloaded from baseq3/missionpack
inline constexpr int NUM_ID_PAKS = 9;
inline constexpr int NUM_TA_PAKS = 4;

inline constexpr int MAX_FILE_HANDLES = 64;

#ifdef DEDICATED
#	define Q3CONFIG_CFG CONFIG_PREFIX "_server.cfg"
#else
#	define Q3CONFIG_CFG CONFIG_PREFIX ".cfg"
#endif



// shutdown and restart the filesystem so changes to fs_gamedir can take effect

// directory should not have either a leading or trailing /
// if extension is "/", only subdirectories will be returned
// the returned files will not include any directories or /




int FS_FindVM(void **startSearch, char *found, int foundlen, const char *name, int enableDll);





// will properly create any needed paths and deal with seperater character issues

// if uniqueFILE is true, then a new FILE will be fopened even if the file
// is found in an already open pak file.  If uniqueFILE is false, you must call
// FileSystem::fCloseFile instead of fclose, otherwise the pak FILE would be improperly closed
// It is generally safe to always set uniqueFILE to true, because the majority of
// file IO goes through FileSystem::readFile, which Does The Right Thing already.

// returns 1 if a file is in the PAK file, otherwise -1


// properly handles partial reads and reads from other dlls

// note: you can't just fclose from another DLL, due to MS libc issues

// returns the length of the file
// a null buffer will just return the file length without loading
// as a quick check for existence. -1 length == not present
// A 0 byte will always be appended at the end, so string ops are safe.
// the buffer should be considered read-only, because it may be cached
// for other uses.

// forces flush on files we're writing to.

// frees the memory returned by FileSystem::readFile

// writes a complete file, creating any subdirectories needed

// doesn't work for files that are opened from a pack file

// where are we?


// like fprintf

// opens a file for reading, writing, or appending depending on the value of mode

// seek on a file


// Returns a space separated string containing the checksums of all loaded pk3 files.
// Servers with sv_pure set will get this string and pass it to clients.

// Returns a space separated string containing the checksums of all loaded 
// AND referenced pk3 files. Servers with sv_pure set will get this string 
// back from clients for pure validation 

// clears referenced booleans on loaded pk3s

// If the string is empty, all data sources will be allowed.
// If not empty, only pk3 files that match one of the space
// separated checksums will be checked for files, with the
// sole exception of .cfg files.





/*
==============================================================

Edit fields and command line history/completion

==============================================================
*/

#define	MAX_EDIT_LINE	256
class EditField
{
public:
	void clear();
	void autoComplete();
	void draw( int x, int y, int width, bool showCursor, bool noColorEscape );
	void bigDraw( int x, int y, int width, bool showCursor, bool noColorEscape );
	void paste();
	void keyDownEvent( int key );
	void charEvent( int ch );
	void hist_Add();

	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];

private:
	void variableSizeDraw( int x, int y, int width, int size, bool drawSmall, bool showCursor, bool noColorEscape );
};

void Field_CompleteKeyname( );
void Field_CompleteFilename( const char *dir, const char *ext,
		char *filter, bool stripExt,
		bool allowNonPureFilesOnDisk );
void Field_CompleteCommand( char *cmd,
		bool doCommands, bool doCvars );
void Field_CompletePlayerName( const char **names, int count );

/*
==============================================================

MISC

==============================================================
*/

// centralizing the declarations for cl_cdkey
// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=470
extern char cl_cdkey[34];

// returned by System::getProcessorFeatures

// centralized and cleaned, that's the max string you can send to a Common::printf / Common::dPrintf (above gets truncated)
inline constexpr int MAXPRINTMSG = 4096;



struct sysEvent_t {
	int				evTime;
	sysEventType_t	evType;
	int				evValue, evValue2;
	int				evPtrLength;	// bytes of data pointed to by evPtr, for journaling
	void			*evPtr;			// this must be manually freed if not NULL
};


char		*CopyString( const char *in );
void		Info_Print( const char *s );


unsigned	Com_BlockChecksum( const void *buffer, int length );
char		*Com_MD5File(const char *filename, int length, const char *prefix, int prefix_len);


// checks for and removes command line "+set var arg" constructs
// if match is NULL, all set commands will be executed, otherwise
// only a set with the exact name.  Only used during startup.



extern	cvar_t	*com_developer;
extern	cvar_t	*com_dedicated;
extern	cvar_t	*com_speeds;
extern	cvar_t	*com_timescale;
extern	cvar_t	*com_sv_running;
extern	cvar_t	*com_cl_running;
extern	cvar_t	*com_version;
extern	cvar_t	*com_blood;
extern	cvar_t	*com_buildScript;		// for building release pak files
extern	cvar_t	*com_journal;
extern	cvar_t	*com_cameraMode;
extern	cvar_t	*com_ansiColor;
extern	cvar_t	*com_unfocused;
extern	cvar_t	*com_maxfpsUnfocused;
extern	cvar_t	*com_minimized;
extern	cvar_t	*com_maxfpsMinimized;
extern	cvar_t	*com_altivec;
extern	cvar_t	*com_standalone;
extern	cvar_t	*com_basegame;
extern	cvar_t	*com_homepath;

// both client and server must agree to pause
extern	cvar_t	*cl_paused;
extern	cvar_t	*sv_paused;

extern	cvar_t	*cl_packetdelay;
extern	cvar_t	*sv_packetdelay;

extern	cvar_t	*com_gamename;
extern	cvar_t	*com_protocol;
#ifdef LEGACY_PROTOCOL
extern	cvar_t	*com_legacyprotocol;
#endif
#ifndef DEDICATED
extern  cvar_t  *con_autochat;
#endif

// com_speeds times
extern	int		time_game;
extern	int		time_frontend;
extern	int		time_backend;		// renderer backend time

extern	int		com_frameTime;

extern	bool	com_errorEntered;
extern	bool	com_fullyInitialized;

extern	fileHandle_t	com_journalFile;
extern	fileHandle_t	com_journalDataFile;

enum memtag_t {
	TAG_FREE,
	TAG_GENERAL,
	TAG_BOTLIB,
	TAG_RENDERER,
	TAG_SMALL,
	TAG_STATIC
};

/*

--- low memory ----
server vm
server clipmap
---mark---
renderer initialization (shaders, etc)
UI vm
cgame vm
renderer map
renderer models

---free---

temp file loading
--- high memory ---

*/

#if !defined(NDEBUG) && !defined(BSPC)
	#define ZONE_DEBUG
#endif

#ifdef ZONE_DEBUG
void *Z_TagMallocDebug( int size, int tag, const char *label, const char *file, int line );	// NOT 0 filled memory
void *Z_MallocDebug( int size, const char *label, const char *file, int line );			// returns 0 filled memory
void *S_MallocDebug( int size, const char *label, const char *file, int line );			// returns 0 filled memory
// debug builds record who asked for each block: the calling function, file and line
inline void *Z_TagMalloc( int size, int tag, const std::source_location& at = std::source_location::current() ) {
	return Z_TagMallocDebug( size, tag, at.function_name(), at.file_name(), static_cast<int>( at.line() ) );
}
inline void *Z_Malloc( int size, const std::source_location& at = std::source_location::current() ) {
	return Z_MallocDebug( size, at.function_name(), at.file_name(), static_cast<int>( at.line() ) );
}
inline void *S_Malloc( int size, const std::source_location& at = std::source_location::current() ) {
	return S_MallocDebug( size, at.function_name(), at.file_name(), static_cast<int>( at.line() ) );
}
#else
void *Z_TagMalloc( int size, int tag );	// NOT 0 filled memory
void *Z_Malloc( int size );			// returns 0 filled memory
void *S_Malloc( int size );			// NOT 0 filled memory only for small allocations
#endif
void Z_Free( void *ptr );
void Z_FreeTags( int tag );
int Z_AvailableMemory( );
void Z_LogHeap( );

void Hunk_Clear( );
void Hunk_ClearToMark( );
void Hunk_SetMark( );
bool Hunk_CheckMark( );
void Hunk_ClearTempMemory( );
void *Hunk_AllocateTempMemory( int size );
void Hunk_FreeTempMemory( void *buf );
int	Hunk_MemoryRemaining( );
void Hunk_Log( );


// commandLine should not include the executable name (argv[0])


/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

//
// client interface
//
void CL_InitKeyCommands( );
// the keyboard binding interface must be setup before execing
// config files, but the rest of client startup will happen later

void CL_Init( );
void CL_Disconnect( bool showMainMenu );
void CL_Shutdown(const char *finalmsg, bool disconnect, bool quit);
void CL_Frame( int msec );
bool CL_GameCommand( );
void CL_KeyEvent (int key, bool down, unsigned time);

void CL_CharEvent( int key );
// char events are for field typing, not game control

void CL_MouseEvent( int dx, int dy, int time );

void CL_JoystickEvent( int axis, int value, int time );

void CL_PacketEvent( netadr_t from, Message *msg );

void CL_ConsolePrint( char *text );

void CL_MapLoading( );
// do a screen update before starting to load a map
// when the server is going to load a new map, the entire hunk
// will be cleared, so the client must shutdown cgame, ui, and
// the renderer

void	CL_ForwardCommandToServer( const char *string );
// adds the current command line as a clc_clientCommand to the client message.
// things like godmode, noclip, etc, are commands directed to the server,
// so when they are typed in at the console, they will need to be forwarded.

void CL_CDDialog( );
// bring up the "need a cd to play" dialog

void CL_FlushMemory( );
// dump all memory on an error

void CL_ShutdownAll(bool shutdownRef);
// shutdown client

void CL_InitRef();
// initialize renderer interface

void CL_StartHunkUsers( bool rendererOnly );
// start all the client stuff using the hunk

void CL_Snd_Shutdown();
// Restart sound subsystem

void Key_KeynameCompletion( void(*callback)(const char *s) );
// for keyname autocompletion

// for writing the config files

// call before filesystem access


// AVI files have the start of pixel lines 4 byte-aligned
inline constexpr int AVI_LINE_PADDING = 4;

//
// server interface
//
void SV_Init( );
void SV_Shutdown( const char *finalmsg );
void SV_Frame( int msec );
void SV_PacketEvent( netadr_t from, Message *msg );
int SV_FrameMsec();
bool SV_GameCommand( );
int SV_SendQueuedPackets();

//
// UI interface
//
bool UI_GameCommand( );
bool UI_usesUniqueCDKey();

//
// input interface
//
void IN_Init( void *windowData );
void IN_Frame( );
void IN_Shutdown( );
void IN_Restart( );

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

inline constexpr int MAX_JOYSTICK_AXIS = 16;






// System::milliseconds should only be used for profiling purposes,
// any game related timing information should come from event timestamps


// the system console is shown when a dedicated server is running
void	Sys_DisplaySystemConsole( bool show );


void	Sys_SetErrorText( const char *text );

void	Sys_SendPacket( int length, const void *data, netadr_t to );

bool	Sys_StringToAdr( const char *s, netadr_t *a, netadrtype_t family );
//Does NOT parse port numbers, only base addresses.

bool	Sys_IsLANAddress (netadr_t adr);
void		Sys_ShowIP();


#ifdef __APPLE__
#endif









/* This is based on the Adaptive Huffman algorithm described in Sayood's Data
 * Compression book.  The ranks are not actually stored, but implicitly defined
 * by the location of a node within a doubly-linked list */

#define NYT HMAX					/* NYT = Not Yet Transmitted */
#define INTERNAL_NODE (HMAX+1)

struct nodetype {
	struct	nodetype *left, *right, *parent; /* tree structure */ 
	struct	nodetype *next, *prev; /* doubly-linked list */
	struct	nodetype **head; /* highest ranked node in block */
	int		weight;
	int		symbol;
};
using node_t = nodetype;

inline constexpr int HMAX = 256; /* Maximum symbol */

class HuffmanTree
{
public:
	void addRef( byte ch );
	void transmit( int ch, byte *fout, int maxoffset );
	void offsetTransmit( int ch, byte *fout, int *offset, int maxoffset );

	int			blocNode;
	int			blocPtrs;

	node_t*		tree;
	node_t*		lhead;
	node_t*		ltail;
	node_t*		loc[HMAX+1];
	node_t**	freelist;

	node_t		nodeList[768];
	node_t*		nodePtrs[768];

private:
	node_t **get_ppnode();
	void free_ppnode( node_t **ppnode );
	void swap( node_t *node1, node_t *node2 );
	void increment( node_t *node );
};

struct huffman_t {
	HuffmanTree		compressor;
	HuffmanTree		decompressor;
};

void	Huff_Compress(Message *buf, int offset);
void	Huff_Decompress(Message *buf, int offset);
void	Huff_Init(huffman_t *huff);
int		Huff_Receive (node_t *node, int *ch, byte *fin);
void	Huff_offsetReceive (node_t *node, int *ch, byte *fin, int *offset, int maxoffset);
void	Huff_putBit( int bit, byte *fout, int *offset);
int		Huff_getBit( byte *fout, int *offset);

// don't use if you don't know what you're doing.
int		Huff_getBloc();
void	Huff_setBloc(int _bloc);


extern huffman_t clientHuffTables;

inline constexpr int SV_ENCODE_START = 4;
inline constexpr int SV_DECODE_START = 12;
inline constexpr int CL_ENCODE_START = 12;
inline constexpr int CL_DECODE_START = 4;

// flags for sv_allowDownload and cl_allowDownload
inline constexpr int DLF_ENABLE = 1;
inline constexpr int DLF_NO_REDIRECT = 2;
inline constexpr int DLF_NO_UDP = 4;
inline constexpr int DLF_NO_DISCONNECT = 8;

#include "../qcommon/network.h"

#endif // _QCOMMON_H_
