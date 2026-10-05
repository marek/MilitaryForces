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
// client.h -- primary header for client

#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../renderercommon/tr_public.h"
#include "../ui/ui_public.h"
#include "../qcommon/client_host.h"
#include "keys.h"
#include "snd_public.h"
#include "../cgame/cg_public.h"
#include "../game/bg_public.h"

#ifdef USE_HTTP
#include "cl_http.h"
#endif /* USE_HTTP */

#ifdef USE_VOIP
#include <opus.h>
#endif

// file full of random crap that gets used to create cl_guid
#define QKEY_FILE "qkey"
inline constexpr int QKEY_SIZE = 2048;

inline constexpr int RETRANSMIT_TIMEOUT = 3000; // time between connection packet retransmits

// snapshots are a view of the server at a given time
struct clSnapshot_t {
	bool		valid;			// cleared if delta parsing was invalid
	int				snapFlags;		// rate delayed and dropped commands

	int				serverTime;		// server time the message is valid for (in msec)

	int				messageNum;		// copied from netchan->incoming_sequence
	int				deltaNum;		// messageNum the delta is from
	int				ping;			// time from when cmdNum-1 was sent to time packet was reeceived
	byte			areamask[MAX_MAP_AREA_BYTES];		// portalarea visibility bits

	int				cmdNum;			// the next cmdNum the server is expecting
	playerState_t	ps;						// complete information about the current player at this time

	int				numEntities;			// all of the entities that need to be presented
	int				parseEntitiesNum;		// at the time of this snapshot

	int				serverCommandNum;		// execute all commands up to this before
											// making the snapshot current
};



/*
=============================================================================

the clientActive_t structure is wiped completely at every
new gamestate_t, potentially several times during an established connection

=============================================================================
*/

struct outPacket_t {
	int		p_cmdNumber;		// cl.cmdNumber when packet was sent
	int		p_serverTime;		// usercmd->serverTime when packet was sent
	int		p_realtime;			// cls.realtime when packet was sent
};

// the parseEntities array must be large enough to hold PACKET_BACKUP frames of
// entities, so that when a delta compressed message arives from the server
// it can be un-deltad from the original 
#define	MAX_PARSE_ENTITIES	( PACKET_BACKUP * MAX_SNAPSHOT_ENTITIES )

extern int g_console_field_width;

struct clientActive_t {
	int			timeoutcount;		// it requres several frames in a timeout condition
									// to disconnect, preventing debugging breaks from
									// causing immediate disconnects on continue
	clSnapshot_t	snap;			// latest received from server

	int			serverTime;			// may be paused during play
	int			oldServerTime;		// to prevent time from flowing bakcwards
	int			oldFrameServerTime;	// to check tournament restarts
	int			serverTimeDelta;	// cl.serverTime = cls.realtime + cl.serverTimeDelta
									// this value changes as net lag varies
	bool	extrapolatedSnapshot;	// set if any cgame frame has been forced to extrapolate
									// cleared when CL_AdjustTimeDelta looks at it
	bool	newSnapshots;		// set on parse of any valid packet

	gameState_t	gameState;			// configstrings
	char		mapname[MAX_QPATH];	// extracted from CS_SERVERINFO

	int			parseEntitiesNum;	// index (not anded off) into cl_parse_entities[]

	int			mouseDx[2], mouseDy[2];	// added to by mouse events
	int			mouseIndex;
	int			joystickAxis[MAX_JOYSTICK_AXIS];	// set by joystick events

	// cgame communicates a few values to the client system
	int			cgameUserCmdValue;	// current weapon to add to usercmd_t
	float		cgameSensitivity;

	// cmds[cmdNumber] is the predicted command, [cmdNumber-1] is the last
	// properly generated command
	usercmd_t	cmds[CMD_BACKUP];	// each mesage will send several old cmds
	int			cmdNumber;			// incremented each frame, because multiple
									// frames may need to be packed into a single packet

	outPacket_t	outPackets[PACKET_BACKUP];	// information about each packet we have sent out

	// the client maintains its own idea of view angles, which are
	// sent to the server each frame.  It is cleared to 0 upon entering each level.
	// the server sends a delta each frame which is added to the locally
	// tracked view angles to account for standing on rotating objects,
	// and teleport direction changes
	vec3_t		viewangles;

	int			serverId;			// included in each client message so the server
												// can tell if it is for a prior map_restart
	// big stuff at end of structure so most offsets are 15 bits or less
	clSnapshot_t	snapshots[PACKET_BACKUP];

	entityState_t	entityBaselines[MAX_GENTITIES];	// for delta compression when not in previous frame

	entityState_t	parseEntities[MAX_PARSE_ENTITIES];

	int			entityAckBits;
};

extern	clientActive_t		cl;

/*
=============================================================================

the clientConnection_t structure is wiped when disconnecting from a server,
either to go to a full screen console, play a demo, or connect to a different server

A connection can be to either a server through the network layer or a
demo through a file.

=============================================================================
*/

inline constexpr int MAX_TIMEDEMO_DURATIONS = 4096;

struct clientConnection_t {

	connstate_t	state;				// connection status

	int			clientNum;
	int			lastPacketSentTime;			// for retransmits during connection
	int			lastPacketTime;				// for timeouts

	char		servername[MAX_OSPATH];		// name of server from original connect (used by reconnect)
	netadr_t	serverAddress;
	int			connectTime;				// for connection retransmits
	int			connectPacketCount;			// for display on connection dialog
	char		serverMessage[MAX_STRING_TOKENS];	// for display on connection dialog

	int			challenge;					// from the server to use for connecting
	int			checksumFeed;				// from the server for checksum calculations

	// these are our reliable messages that go to the server
	int			reliableSequence;
	int			reliableAcknowledge;		// the last one the server has executed
	char		reliableCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];

	// server message (unreliable) and command (reliable) sequence
	// numbers are NOT cleared at level changes, but continue to
	// increase as long as the connection is valid

	// message sequence is used by both the network layer and the
	// delta compression layer
	int			serverMessageSequence;

	// reliable messages received from server
	int			serverCommandSequence;
	int			lastExecutedServerCommand;		// last server command grabbed or executed with ClientHost::getServerCommand
	char		serverCommands[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];

	// file transfer from server
	fileHandle_t download;
	char		downloadTempName[MAX_OSPATH];
	char		downloadName[MAX_OSPATH];
#ifdef USE_HTTP
	bool	httpUsed;
	bool	disconnectedForHttpDownload;
	char		downloadURL[MAX_OSPATH];
#endif /* USE_HTTP */
	int		sv_allowDownload;
	char		sv_dlURL[MAX_CVAR_VALUE_STRING];
	int			downloadNumber;
	int			downloadBlock;	// block we are waiting for
	int			downloadCount;	// how many bytes we got
	int			downloadSize;	// how many bytes we got
	char		downloadList[MAX_INFO_STRING]; // list of paks we need to download
	bool	downloadRestart;	// if true, we need to do another FileSystem::restart because we downloaded a pak

	// demo information
	char		demoName[MAX_QPATH];
	bool	spDemoRecording;
	bool	demorecording;
	bool	demoplaying;
	bool	demowaiting;	// don't record until a non-delta message is received
	bool	firstDemoFrameSkipped;
	fileHandle_t	demofile;

	int			timeDemoFrames;		// counter of rendered frames
	int			timeDemoStart;		// cls.realtime before first frame
	int			timeDemoBaseTime;	// each frame will be at this time + frameNum * 50
	int			timeDemoLastFrame;// time the last frame was rendered
	int			timeDemoMinDuration;	// minimum frame duration
	int			timeDemoMaxDuration;	// maximum frame duration
	unsigned char	timeDemoDurations[ MAX_TIMEDEMO_DURATIONS ];	// log of frame durations

	float		aviVideoFrameRemainder;
	float		aviSoundFrameRemainder;

#ifdef USE_VOIP
	bool voipEnabled;
	bool voipCodecInitialized;

	// incoming data...
	// !!! FIXME: convert from parallel arrays to array of a struct.
	OpusDecoder *opusDecoder[MAX_CLIENTS];
	byte voipIncomingGeneration[MAX_CLIENTS];
	int voipIncomingSequence[MAX_CLIENTS];
	float voipGain[MAX_CLIENTS];
	bool voipIgnore[MAX_CLIENTS];
	bool voipMuteAll;

	// outgoing data...
	// if voipTargets[i / 8] & (1 << (i % 8)),
	// then we are sending to clientnum i.
	uint8_t voipTargets[(MAX_CLIENTS + 7) / 8];
	uint8_t voipFlags;
	OpusEncoder *opusEncoder;
	int voipOutgoingDataSize;
	int voipOutgoingDataFrames;
	int voipOutgoingSequence;
	byte voipOutgoingGeneration;
	byte voipOutgoingData[1024];
	float voipPower;
#endif

#ifdef LEGACY_PROTOCOL
	bool compat;
#endif

	// big stuff at end of structure so most offsets are 15 bits or less
	NetChannel	netchan;
};

extern	clientConnection_t clc;

/*
==================================================================

the clientStatic_t structure is never wiped, and is used even when
no client connection is active at all
(except when ClientLoop::shutdown is called)

==================================================================
*/

struct ping_t {
	netadr_t	adr;
	int			start;
	int			time;
	char		info[MAX_INFO_STRING];
};

struct serverInfo_t {
	netadr_t	adr;
	char	  	hostName[MAX_HOSTNAME_LENGTH];
	char	  	mapName[MAX_NAME_LENGTH];
	char	  	game[MAX_NAME_LENGTH];
	int			netType;
	int			gameType;
	int		  	clients;
	int		  	maxClients;
	int			minPing;
	int			maxPing;
	int			ping;
	bool	visible;
	int			punkbuster;
	int			g_humanplayers;
	int			g_needpass;
};

struct clientStatic_t {
	bool	cddialog;			// bring up the cd needed dialog next frame

	// when the server clears the hunk, all of these must be restarted
	bool	rendererStarted;
	bool	soundStarted;
	bool	soundRegistered;
	bool	uiStarted;
	bool	cgameStarted;

	int			framecount;
	int			frametime;			// msec since last frame

	int			realtime;			// ignores pause
	int			realFrametime;		// ignoring pause, so console always works

	int			numlocalservers;
	serverInfo_t	localServers[MAX_OTHER_SERVERS];

	int			numglobalservers;
	serverInfo_t  globalServers[MAX_GLOBAL_SERVERS];
	// additional global servers
	int			numGlobalServerAddresses;
	netadr_t		globalServerAddresses[MAX_GLOBAL_SERVERS];

	int			numfavoriteservers;
	serverInfo_t	favoriteServers[MAX_OTHER_SERVERS];

	int pingUpdateSource;		// source currently pinging or updating

	// update server info
	netadr_t	updateServer;
	char		updateChallenge[MAX_TOKEN_CHARS];
	char		updateInfoString[MAX_INFO_STRING];

	netadr_t	authorizeServer;

	netadr_t	rconAddress;

	// rendering info
	glconfig_t	glconfig;
	qhandle_t	charSetShader;
	qhandle_t	whiteShader;
	qhandle_t	consoleShader;
};

extern	clientStatic_t		cls;

extern	char		cl_oldGame[MAX_QPATH];
extern	bool	cl_oldGameSet;

//=============================================================================

extern	refexport_t		refExport;		// interface to refresh .dll

struct UserInterface;
extern UserInterface& theUI;

struct ClientGame;
extern ClientGame& theCG;


//
// cvars
//
extern	cvar_t	*cl_nodelta;
extern	cvar_t	*cl_debugMove;
extern	cvar_t	*cl_noprint;
extern	cvar_t	*cl_timegraph;
extern	cvar_t	*cl_maxpackets;
extern	cvar_t	*cl_packetdup;
extern	cvar_t	*cl_shownet;
extern	cvar_t	*cl_showSend;
extern	cvar_t	*cl_timeNudge;
extern	cvar_t	*cl_showTimeDelta;
extern	cvar_t	*cl_freezeDemo;

extern	cvar_t	*cl_yawspeed;
extern	cvar_t	*cl_pitchspeed;
extern	cvar_t	*cl_run;
extern	cvar_t	*cl_anglespeedkey;

extern	cvar_t	*cl_sensitivity;
extern	cvar_t	*cl_freelook;

extern	cvar_t	*cl_mouseAccel;
extern	cvar_t	*cl_mouseAccelOffset;
extern	cvar_t	*cl_mouseAccelStyle;
extern	cvar_t	*cl_showMouseRate;

extern	cvar_t	*m_pitch;
extern	cvar_t	*m_yaw;
extern	cvar_t	*m_forward;
extern	cvar_t	*m_side;
extern	cvar_t	*m_filter;

extern	cvar_t	*j_pitch;
extern	cvar_t	*j_yaw;
extern	cvar_t	*j_forward;
extern	cvar_t	*j_side;
extern	cvar_t	*j_up;
extern	cvar_t	*j_pitch_axis;
extern	cvar_t	*j_yaw_axis;
extern	cvar_t	*j_forward_axis;
extern	cvar_t	*j_side_axis;
extern	cvar_t	*j_up_axis;

extern	cvar_t	*cl_timedemo;
extern	cvar_t	*cl_aviFrameRate;
extern	cvar_t	*cl_aviMotionJpeg;

extern	cvar_t	*cl_activeAction;

extern	cvar_t	*cl_allowDownload;
extern	cvar_t	*cl_conXOffset;
extern	cvar_t	*cl_inGameVideo;

extern	cvar_t	*cl_lanForcePackets;
extern	cvar_t	*cl_autoRecordDemo;

extern	cvar_t	*cl_consoleKeys;

#ifdef USE_MUMBLE
extern	cvar_t	*cl_useMumble;
#endif

#ifdef USE_VOIP
// cl_voipSendTarget is a string: "all" to broadcast to everyone, "none" to
//  send to no one, or a comma-separated list of client numbers:
//  "0,7,2,23" ... an empty string is treated like "all".
extern	cvar_t	*cl_voipSend;
extern	cvar_t	*cl_voipSendTarget;
extern	cvar_t	*cl_voipShowMeter;
extern	cvar_t	*cl_voip;

// 20ms at 48k
inline constexpr int VOIP_MAX_FRAME_SAMPLES = ( 20 * 48 );

// 3 frame is 60ms of audio, the max opus will encode at once
inline constexpr int VOIP_MAX_PACKET_FRAMES = 3;
#define VOIP_MAX_PACKET_SAMPLES		( VOIP_MAX_FRAME_SAMPLES * VOIP_MAX_PACKET_FRAMES )
#endif

//=================================================

//
// cl_main
//



void CL_GetChallengePacket ();





//
// cl_input
//
struct kbutton_t {
	int			down[2];		// key nums holding it down
	unsigned	downtime;		// msec timestamp
	unsigned	msec;			// msec down this frame if both a down and up happened
	bool	active;			// current state
	bool	wasPressed;		// set when down, not cleared when up
};

void CL_ReadPackets ();

void IN_CenterView ();

void CL_VerifyCode( );

int Key_StringToKeynum( char *str );
char *Key_KeynumToString (int keynum);

//
// cl_parse.c
//
extern int cl_connectedToPureServer;
extern int cl_connectedToCheatServer;

#ifdef USE_VOIP
#endif


//====================================================================

void	CL_FavoriteServers_f( );


//
// console
//
extern int g_smallchar_width;
extern int g_smallchar_height;

void Con_DrawCharacter (int cx, int line, int num);

class Console
{
public:
	static void checkResize ();
	static void init();
	static void shutdown();
	static void clear_f ();
	static void toggleConsole_f ();
	static void drawNotify ();
	static void clearNotify ();
	static void runConsole ();
	static void drawConsole ();
	static void pageUp( );
	static void pageDown( );
	static void top( );
	static void bottom( );
	static void close( );
};


//
// cl_scrn.c
//






//
// cl_cin.c
//

int CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits);
e_status CIN_StopCinematic(int handle);
e_status CIN_RunCinematic (int handle);
void CIN_DrawCinematic (int handle);
void CIN_SetExtents (int handle, int x, int y, int w, int h);
void CIN_SetLooping (int handle, bool loop);
void CIN_UploadCinematic(int handle);
void CIN_CloseAllVideos();

//
// cl_cgame.c
//
void CL_ShaderStateChanged();

//
// cl_ui.c
//
int Key_GetCatcher( );
void Key_SetCatcher( int catcher );
void LAN_LoadCachedServers( );
void LAN_SaveServersToCache( );


//
// cl_net_chan.c
//

//
// cl_main.c
//
class Client
{
public:
	static void parseGamestate( Message *msg );
	static void writeAVIVideoFrame( const byte *imageBuffer, int size );
	static int scaledMilliseconds();
	static void disconnect_f ();
	static void vid_Restart_f( );
	static void snd_Restart_f ();
	static void startDemoLoop( );
	static void nextDemo( );
	static void readDemoMessage( );
	static void stopRecord_f();
	static void initDownloads();
	static void nextDownload();
	static void getPing( int n, char *buf, int buflen, int *pingtime );
	static void getPingInfo( int n, char *buf, int buflen );
	static void clearPing( int n );
	static int getPingQueueCount( );
	static void shutdownRef( );
	static int serverStatus( const char *serverAddress, char *serverStatusString, int maxLen );
	static bool checkPaused();
	static void initInput();
	static void shutdownInput();
	static void sendCmd ();
	static void clearState ();
	static void writePacket( );
	static float keyState (kbutton_t *key);
	static void voip_f( );
	static void systemInfoChanged( );
	static void parseServerMessage( Message *msg );
	static void serverInfoPacket( netadr_t from, Message *msg );
	static void localServers_f( );
	static void globalServers_f( );
	static void ping_f( );
	static bool updateVisiblePings_f( int source );
	static void loadConsoleHistory( );
	static void saveConsoleHistory( );
	static void playCinematic_f( );
	static void initCGame( );
	static void cGameRendering( stereoFrame_t stereo );
	static void setCGameTime( );
	static void firstSnapshot( );
	static void initUI( );
	static void netchan_Transmit( NetChannel *chan, Message* msg);
	static bool netchan_Process( NetChannel *chan, Message *msg );
	static bool openAVIForWriting( const char *filename );
	static void takeVideoFrame( );
	static void writeAVIAudioFrame( const byte *pcmBuffer, int size );
	static bool closeAVI( );
	static bool videoRecording( );
	static void writeDemoMessage ( Message *msg, int headerBytes );

public:
	static ID_INLINE void safeFS_Write( const void *buf, int len, fileHandle_t f );
	static ID_INLINE void wRITE_STRING( const char *s );
	static ID_INLINE void wRITE_4BYTES( int x );
	static ID_INLINE void wRITE_2BYTES( int x );
	static ID_INLINE void sTART_CHUNK( const char *s );
	static ID_INLINE void eND_CHUNK();
	static bool checkFileSize( int bytesToAdd );
	static int cIN_HandleForVideo();
	static void rllSetupTable();
	static void updateMumble();
	static void updateVoipIgnore( const char *idstr, bool ignore );
	static void updateVoipGain( const char *idstr, float gain );
	static void voipNewGeneration();
	static void captureVoip();
	static float demoFrameDurationSDev();
	static int walkDemoExt( char *arg, char *name, int *demofile );
	static void completeDemoName( char *args, int argNum );
	static void updateGUID( const char *prefix, int prefix_len );
	static void oldGame();
	static void completeRcon( char *args, int argNum );
	static void completePlayerName( char *args, int argNum );
	static void initDownload( const char *localName );
	static void beginDownload( const char *remoteName );
	static void beginHttpDownload( const char *remoteURL );
	static void netchan_Encode( Message *msg );
	static void netchan_Decode( Message *msg );
	static void parseServerInfo();
	static bool shouldIgnoreVoipSender( int sender );
	static void playVoip( int sender, int samplecnt, const byte *data, int flags );
	static void parseVoip( Message *msg, bool ignoreData );
};

#endif // __CLIENT_H__
