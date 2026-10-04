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
// cl_main.c  -- client main loop

#include "client.h"
#include "../ui/ui.h"
#include "../cgame/cg.h"
#include <climits>

#include "../sys/sys_local.h"
#include "../sys/sys_loadlib.h"

#ifdef USE_MUMBLE
#include "libmumblelink.h"
#endif

#ifdef USE_MUMBLE
cvar_t	*cl_useMumble;
cvar_t	*cl_mumbleScale;
#endif

#ifdef USE_VOIP
cvar_t	*cl_voipUseVAD;
cvar_t	*cl_voipVADThreshold;
cvar_t	*cl_voipSend;
cvar_t	*cl_voipSendTarget;
cvar_t	*cl_voipGainDuringCapture;
cvar_t	*cl_voipCaptureMult;
cvar_t	*cl_voipShowMeter;
cvar_t	*cl_voipProtocol;
cvar_t	*cl_voip;
#endif

#ifdef USE_RENDERER_DLOPEN
cvar_t	*cl_renderer;
#endif

cvar_t	*cl_nodelta;
cvar_t	*cl_debugMove;

cvar_t	*cl_noprint;
#ifdef UPDATE_SERVER_NAME
cvar_t	*cl_motd;
#endif

cvar_t	*rcon_client_password;
cvar_t	*rconAddress;

cvar_t	*cl_timeout;
cvar_t	*cl_maxpackets;
cvar_t	*cl_packetdup;
cvar_t	*cl_timeNudge;
cvar_t	*cl_showTimeDelta;
cvar_t	*cl_freezeDemo;

cvar_t	*cl_shownet;
cvar_t	*cl_showSend;
cvar_t	*cl_timedemo;
cvar_t	*cl_timedemoLog;
cvar_t	*cl_autoRecordDemo;
cvar_t	*cl_aviFrameRate;
cvar_t	*cl_aviMotionJpeg;
cvar_t	*cl_forceavidemo;

cvar_t	*cl_freelook;
cvar_t	*cl_sensitivity;

cvar_t	*cl_mouseAccel;
cvar_t	*cl_mouseAccelOffset;
cvar_t	*cl_mouseAccelStyle;
cvar_t	*cl_showMouseRate;

cvar_t	*m_pitch;
cvar_t	*m_yaw;
cvar_t	*m_forward;
cvar_t	*m_side;
cvar_t	*m_filter;

cvar_t	*j_pitch;
cvar_t	*j_yaw;
cvar_t	*j_forward;
cvar_t	*j_side;
cvar_t	*j_up;
cvar_t	*j_pitch_axis;
cvar_t	*j_yaw_axis;
cvar_t	*j_forward_axis;
cvar_t	*j_side_axis;
cvar_t	*j_up_axis;

cvar_t	*cl_activeAction;

cvar_t	*cl_motdString;

cvar_t	*cl_allowDownload;
cvar_t	*cl_conXOffset;
cvar_t	*cl_inGameVideo;

cvar_t	*cl_serverStatusResendTime;

cvar_t	*cl_lanForcePackets;

cvar_t	*cl_guidServerUniq;

cvar_t	*cl_consoleKeys;

cvar_t	*cl_rate;

clientActive_t		cl;
clientConnection_t	clc;
clientStatic_t		cls;

char				cl_reconnectArgs[MAX_OSPATH];
char				cl_oldGame[MAX_QPATH];
bool			cl_oldGameSet;

// Structure containing functions exported from refresh DLL
ClientGame& theCG = ClientGame::getInstance();

refexport_t	refExport;
#ifdef USE_RENDERER_DLOPEN
static void	*rendererLib = nullptr;
#endif

ping_t	cl_pinglist[MAX_PINGREQUESTS];

struct serverStatus_s
{
	char string[BIG_INFO_STRING];
	netadr_t address;
	int time, startTime;
	bool pending;
	bool print;
	bool retrieved;
};
using serverStatus_t = serverStatus_s;

serverStatus_t cl_serverStatusList[MAX_SERVERSTATUSREQUESTS];

#if defined __USEA3D && defined __A3D_GEOM
	void hA3Dg_ExportRenderGeom (refexport_t *incoming_re);
#endif

static int noGameRestart = false;

extern void SV_BotFrame( int time );
void CL_CheckForResend( );
void CL_ShowIP_f();
void CL_ServerStatus_f();
void CL_ServerStatusResponse( netadr_t from, Message *msg );

/*
===============
CL_CDDialog

Called by Com_Error when a cd is needed
===============
*/
void CL_CDDialog( ) {
	cls.cddialog = true;	// start it next frame
}

#ifdef USE_MUMBLE
static
void CL_UpdateMumble()
{
	vec3_t pos, forward, up;
	float scale = cl_mumbleScale->value;
	float tmp;
	
	if(!cl_useMumble->integer)
		return;

	// !!! FIXME: not sure if this is even close to correct.
	AngleVectors( cl.snap.ps.viewangles, forward, nullptr, up);

	pos[0] = cl.snap.ps.origin[0] * scale;
	pos[1] = cl.snap.ps.origin[2] * scale;
	pos[2] = cl.snap.ps.origin[1] * scale;

	tmp = forward[1];
	forward[1] = forward[2];
	forward[2] = tmp;

	tmp = up[1];
	up[1] = up[2];
	up[2] = tmp;

	if(cl_useMumble->integer > 1) {
		fprintf(stderr, "%f %f %f, %f %f %f, %f %f %f\n",
			pos[0], pos[1], pos[2],
			forward[0], forward[1], forward[2],
			up[0], up[1], up[2]);
	}

	mumble_update_coordinates(pos, forward, up);
}
#endif


#ifdef USE_VOIP
static
void CL_UpdateVoipIgnore(const char *idstr, bool ignore)
{
	if ((*idstr >= '0') && (*idstr <= '9')) {
		const int id = atoi(idstr);
		if ((id >= 0) && (id < MAX_CLIENTS)) {
			clc.voipIgnore[id] = ignore;
			CL_AddReliableCommand(va("voip %s %d",
			                         ignore ? "ignore" : "unignore", id), false);
			Com_Printf("VoIP: %s ignoring player #%d\n",
			            ignore ? "Now" : "No longer", id);
			return;
		}
	}
	Com_Printf("VoIP: invalid player ID#\n");
}

static
void CL_UpdateVoipGain(const char *idstr, float gain)
{
	if ((*idstr >= '0') && (*idstr <= '9')) {
		const int id = atoi(idstr);
		if (gain < 0.0f)
			gain = 0.0f;
		if ((id >= 0) && (id < MAX_CLIENTS)) {
			clc.voipGain[id] = gain;
			Com_Printf("VoIP: player #%d gain now set to %f\n", id, gain);
		}
	}
}

void CL_Voip_f( )
{
	const char *cmd = CommandSystem::argv(1);
	const char *reason = nullptr;

	if (clc.state != CA_ACTIVE)
		reason = "Not connected to a server";
	else if (!clc.voipCodecInitialized)
		reason = "Voip codec not initialized";
	else if (!clc.voipEnabled)
		reason = "Server doesn't support VoIP";
	else if (!clc.demoplaying && (CvarSystem::variableValue( "g_gametype" ) == GT_SINGLE_PLAYER || CvarSystem::variableValue("ui_singlePlayerActive")))
		reason = "running in single-player mode";

	if (reason != nullptr) {
		Com_Printf("VoIP: command ignored: %s\n", reason);
		return;
	}

	if (strcmp(cmd, "ignore") == 0) {
		CL_UpdateVoipIgnore(CommandSystem::argv(2), true);
	} else if (strcmp(cmd, "unignore") == 0) {
		CL_UpdateVoipIgnore(CommandSystem::argv(2), false);
	} else if (strcmp(cmd, "gain") == 0) {
		if (CommandSystem::argc() > 3) {
			CL_UpdateVoipGain(CommandSystem::argv(2), atof(CommandSystem::argv(3)));
		} else if (Q_isanumber(CommandSystem::argv(2))) {
			int id = atoi(CommandSystem::argv(2));
			if (id >= 0 && id < MAX_CLIENTS) {
				Com_Printf("VoIP: current gain for player #%d "
					"is %f\n", id, clc.voipGain[id]);
			} else {
				Com_Printf("VoIP: invalid player ID#\n");
			}
		} else {
			Com_Printf("usage: voip gain <playerID#> [value]\n");
		}
	} else if (strcmp(cmd, "muteall") == 0) {
		Com_Printf("VoIP: muting incoming voice\n");
		CL_AddReliableCommand("voip muteall", false);
		clc.voipMuteAll = true;
	} else if (strcmp(cmd, "unmuteall") == 0) {
		Com_Printf("VoIP: unmuting incoming voice\n");
		CL_AddReliableCommand("voip unmuteall", false);
		clc.voipMuteAll = false;
	} else {
		Com_Printf("usage: voip [un]ignore <playerID#>\n"
		           "       voip [un]muteall\n"
		           "       voip gain <playerID#> [value]\n");
	}
}


static
void CL_VoipNewGeneration()
{
	// don't have a zero generation so new clients won't match, and don't
	//  wrap to negative so MSG_ReadLong() doesn't "fail."
	clc.voipOutgoingGeneration++;
	if (clc.voipOutgoingGeneration <= 0)
		clc.voipOutgoingGeneration = 1;
	clc.voipPower = 0.0f;
	clc.voipOutgoingSequence = 0;

	opus_encoder_ctl(clc.opusEncoder, OPUS_RESET_STATE);
}

/*
===============
CL_VoipParseTargets

sets clc.voipTargets according to cl_voipSendTarget
Generally we don't want who's listening to change during a transmission,
so this is only called when the key is first pressed
===============
*/
void CL_VoipParseTargets()
{
	const char *target = cl_voipSendTarget->string;
	char *end;
	int val;

	Com_Memset(clc.voipTargets, 0, sizeof(clc.voipTargets));
	clc.voipFlags &= ~VOIP_SPATIAL;

	while(target)
	{
		while(*target == ',' || *target == ' ')
			target++;

		if(!*target)
			break;
		
		if(isdigit(*target))
		{
			val = strtol(target, &end, 10);
			target = end;
		}
		else
		{
			if(!Q_stricmpn(target, "all", 3))
			{
				Com_Memset(clc.voipTargets, ~0, sizeof(clc.voipTargets));
				return;
			}
			if(!Q_stricmpn(target, "spatial", 7))
			{
				clc.voipFlags |= VOIP_SPATIAL;
				target += 7;
				continue;
			}
			else
			{
				if(!Q_stricmpn(target, "attacker", 8))
				{
					val = theCG.lastAttacker();
					target += 8;
				}
				else if(!Q_stricmpn(target, "crosshair", 9))
				{
					val = theCG.crosshairPlayer();
					target += 9;
				}
				else
				{
					while(*target && *target != ',' && *target != ' ')
						target++;

					continue;
				}

				if(val < 0)
					continue;
			}
		}

		if(val < 0 || val >= MAX_CLIENTS)
		{
			Com_Printf(S_COLOR_YELLOW "WARNING: VoIP "
				   "target %d is not a valid client "
				   "number\n", val);

			continue;
		}

		clc.voipTargets[val / 8] |= 1 << (val % 8);
	}
}

/*
===============
CL_CaptureVoip

Record more audio from the hardware if required and encode it into Opus
 data for later transmission.
===============
*/
static
void CL_CaptureVoip()
{
	const float audioMult = cl_voipCaptureMult->value;
	const bool useVad = (cl_voipUseVAD->integer != 0);
	bool initialFrame = false;
	bool finalFrame = false;

#if USE_MUMBLE
	// if we're using Mumble, don't try to handle VoIP transmission ourselves.
	if (cl_useMumble->integer)
		return;
#endif

	// If your data rate is too low, you'll get Connection Interrupted warnings
	//  when VoIP packets arrive, even if you have a broadband connection.
	//  This might work on rates lower than 25000, but for safety's sake, we'll
	//  just demand it. Who doesn't have at least a DSL line now, anyhow? If
	//  you don't, you don't need VoIP.  :)
	if (cl_voip->modified || cl_rate->modified) {
		if ((cl_voip->integer) && (cl_rate->integer < 25000)) {
			Com_Printf(S_COLOR_YELLOW "Your network rate is too slow for VoIP.\n");
			Com_Printf("Set 'Data Rate' to 'LAN/Cable/xDSL' in 'Setup/System/Network'.\n");
			Com_Printf("Until then, VoIP is disabled.\n");
			CvarSystem::set("cl_voip", "0");
		}
		CvarSystem::set("cl_voipProtocol", cl_voip->integer ? "opus" : "");
		cl_voip->modified = false;
		cl_rate->modified = false;
	}

	if (!clc.voipCodecInitialized)
		return;  // just in case this gets called at a bad time.

	if (clc.voipOutgoingDataSize > 0)
		return;  // packet is pending transmission, don't record more yet.

	if (cl_voipUseVAD->modified) {
		CvarSystem::set("cl_voipSend", (useVad) ? "1" : "0");
		cl_voipUseVAD->modified = false;
	}

	if ((useVad) && (!cl_voipSend->integer))
		CvarSystem::set("cl_voipSend", "1");  // lots of things reset this.

	if (cl_voipSend->modified) {
		bool dontCapture = false;
		if (clc.state != CA_ACTIVE)
			dontCapture = true;  // not connected to a server.
		else if (!clc.voipEnabled)
			dontCapture = true;  // server doesn't support VoIP.
		else if (clc.demoplaying)
			dontCapture = true;  // playing back a demo.
		else if ( cl_voip->integer == 0 )
			dontCapture = true;  // client has VoIP support disabled.
		else if ( audioMult == 0.0f )
			dontCapture = true;  // basically silenced incoming audio.

		cl_voipSend->modified = false;

		if(dontCapture)
		{
			CvarSystem::set("cl_voipSend", "0");
			return;
		}

		if (cl_voipSend->integer) {
			initialFrame = true;
		} else {
			finalFrame = true;
		}
	}

	// try to get more audio data from the sound card...

	if (initialFrame) {
		S_MasterGain(Com_Clamp(0.0f, 1.0f, cl_voipGainDuringCapture->value));
		S_StartCapture();
		CL_VoipNewGeneration();
		CL_VoipParseTargets();
	}

	if ((cl_voipSend->integer) || (finalFrame)) { // user wants to capture audio?
		int samples = S_AvailableCaptureSamples();
		const int packetSamples = (finalFrame) ? VOIP_MAX_FRAME_SAMPLES : VOIP_MAX_PACKET_SAMPLES;

		// enough data buffered in audio hardware to process yet?
		if (samples >= packetSamples) {
			// audio capture is always MONO16.
			static int16_t sampbuffer[VOIP_MAX_PACKET_SAMPLES];
			float voipPower = 0.0f;
			int voipFrames;
			int i, bytes;

			if (samples > VOIP_MAX_PACKET_SAMPLES)
				samples = VOIP_MAX_PACKET_SAMPLES;

			// !!! FIXME: maybe separate recording from encoding, so voipPower
			// !!! FIXME:  updates faster than 4Hz?

			samples -= samples % VOIP_MAX_FRAME_SAMPLES;
			if (samples != 120 && samples != 240 && samples != 480 && samples != 960 && samples != 1920 && samples != 2880 ) {
				Com_Printf("Voip: bad number of samples %d\n", samples);
				return;
			}
			voipFrames = samples / VOIP_MAX_FRAME_SAMPLES;

			S_Capture(samples, reinterpret_cast<byte *>(sampbuffer));  // grab from audio card.

			// check the "power" of this packet...
			for (i = 0; i < samples; i++) {
				const float flsamp = static_cast<float>(sampbuffer[i]);
				const float s = fabs(flsamp);
				voipPower += s * s;
				sampbuffer[i] = static_cast<int16_t>((flsamp) * audioMult);
			}

			// encode raw audio samples into Opus data...
			bytes = opus_encode(clc.opusEncoder, sampbuffer, samples,
									(unsigned char *) clc.voipOutgoingData,
									sizeof (clc.voipOutgoingData));
			if ( bytes <= 0 ) {
				Com_DPrintf("VoIP: Error encoding %d samples\n", samples);
				bytes = 0;
			}

			clc.voipPower = (voipPower / (32768.0f * 32768.0f *
			                 (static_cast<float>(samples)))) * 100.0f;

			if ((useVad) && (clc.voipPower < cl_voipVADThreshold->value)) {
				CL_VoipNewGeneration();  // no "talk" for at least 1/4 second.
			} else {
				clc.voipOutgoingDataSize = bytes;
				clc.voipOutgoingDataFrames = voipFrames;

				Com_DPrintf("VoIP: Send %d frames, %d bytes, %f power\n",
				            voipFrames, bytes, clc.voipPower);

				#if 0
				static FILE *encio = NULL;
				if (encio == NULL) encio = fopen("voip-outgoing-encoded.bin", "wb");
				if (encio != NULL) { fwrite(clc.voipOutgoingData, bytes, 1, encio); fflush(encio); }
				static FILE *decio = NULL;
				if (decio == NULL) decio = fopen("voip-outgoing-decoded.bin", "wb");
				if (decio != NULL) { fwrite(sampbuffer, voipFrames * VOIP_MAX_FRAME_SAMPLES * 2, 1, decio); fflush(decio); }
				#endif
			}
		}
	}

	// User requested we stop recording, and we've now processed the last of
	//  any previously-buffered data. Pause the capture device, etc.
	if (finalFrame) {
		S_StopCapture();
		S_MasterGain(1.0f);
		clc.voipPower = 0.0f;  // force this value so it doesn't linger.
	}
}
#endif

/*
=======================================================================

CLIENT RELIABLE COMMAND COMMUNICATION

=======================================================================
*/

/*
======================
CL_AddReliableCommand

The given command will be transmitted to the server, and is guaranteed to
not have future usercmd_t executed before it is executed
======================
*/
void CL_AddReliableCommand(const char *cmd, bool isDisconnectCmd)
{
	int unacknowledged = clc.reliableSequence - clc.reliableAcknowledge;
	
	// if we would be losing an old command that hasn't been acknowledged,
	// we must drop the connection
	// also leave one slot open for the disconnect command in this case.
	
	if ((isDisconnectCmd && unacknowledged > MAX_RELIABLE_COMMANDS) ||
	    (!isDisconnectCmd && unacknowledged >= MAX_RELIABLE_COMMANDS))
	{
		if(com_errorEntered)
			return;
		else
			Com_Error(ERR_DROP, "Client command overflow");
	}

	Q_strncpyz(clc.reliableCommands[++clc.reliableSequence & (MAX_RELIABLE_COMMANDS - 1)],
		   cmd, sizeof(*clc.reliableCommands));
}

/*
=======================================================================

CLIENT SIDE DEMO RECORDING

=======================================================================
*/

/*
====================
CL_WriteDemoMessage

Dumps the current net message, prefixed by the length
====================
*/

void CL_WriteDemoMessage ( Message *msg, int headerBytes ) {
	int		len, swlen;

	// write the packet sequence
	len = clc.serverMessageSequence;
	swlen = LittleLong( len );
	FS_Write (&swlen, 4, clc.demofile);
	// skip the packet sequencing information
	len = msg->cursize - headerBytes;
	swlen = LittleLong(len);
	FS_Write (&swlen, 4, clc.demofile);
	FS_Write ( msg->data + headerBytes, len, clc.demofile );
}


/*
====================
CL_StopRecording_f

stop recording a demo
====================
*/
void CL_StopRecord_f( ) {
	int		len;

	if ( !clc.demorecording ) {
		Com_Printf ("Not recording a demo.\n");
		return;
	}

	// finish up
	len = -1;
	FS_Write (&len, 4, clc.demofile);
	FS_Write (&len, 4, clc.demofile);
	FS_FCloseFile (clc.demofile);
	clc.demofile = 0;
	clc.demorecording = false;
	clc.spDemoRecording = false;
	Com_Printf ("Stopped demo.\n");
}

/* 
================== 
CL_DemoFilename
================== 
*/  
void CL_DemoFilename( int number, char *fileName, int fileNameSize ) {
	int		a,b,c,d;

	if(number < 0 || number > 9999)
		number = 9999;

	a = number / 1000;
	number -= a*1000;
	b = number / 100;
	number -= b*100;
	c = number / 10;
	number -= c*10;
	d = number;

	Com_sprintf( fileName, fileNameSize, "demo%i%i%i%i"
		, a, b, c, d );
}

/*
====================
CL_Record_f

record <demoname>

Begins recording a demo from the current position
====================
*/
static char		demoName[MAX_QPATH];	// compiler bug workaround
void CL_Record_f( ) {
	char		name[MAX_OSPATH];
	byte		bufData[MAX_MSGLEN];
	Message	buf;
	int			i;
	int			len;
	entityState_t	*ent;
	entityState_t	nullstate;
	char		*s;

	if ( CommandSystem::argc() > 2 ) {
		Com_Printf ("record <demoname>\n");
		return;
	}

	if ( clc.demorecording ) {
		if (!clc.spDemoRecording) {
			Com_Printf ("Already recording.\n");
		}
		return;
	}

	if ( clc.state != CA_ACTIVE ) {
		Com_Printf ("You must be in a level to record.\n");
		return;
	}

  // sync 0 doesn't prevent recording, so not forcing it off .. everyone does g_sync 1 ; record ; g_sync 0 ..
	if ( Network::isLocalAddress( clc.serverAddress ) &&
		!CvarSystem::variableValue( "g_synchronousClients" ) &&
		// Don't nag if we're simply recording everything automatically.
		!cl_autoRecordDemo->integer )
	{
		Com_Printf (S_COLOR_YELLOW "WARNING: You should set 'g_synchronousClients 1' for smoother demo recording\n");
	}

	if ( CommandSystem::argc() == 2 ) {
		s = CommandSystem::argv(1);
		Q_strncpyz( demoName, s, sizeof( demoName ) );
#ifdef LEGACY_PROTOCOL
		if(clc.compat)
			Com_sprintf(name, sizeof(name), "demos/%s.%s%d", demoName, DEMOEXT, com_legacyprotocol->integer);
		else
#endif
			Com_sprintf(name, sizeof(name), "demos/%s.%s%d", demoName, DEMOEXT, com_protocol->integer);
	} else {
		int		number;

		// scan for a free demo name
		for ( number = 0 ; number <= 9999 ; number++ ) {
			CL_DemoFilename( number, demoName, sizeof( demoName ) );
#ifdef LEGACY_PROTOCOL
			if(clc.compat)
				Com_sprintf(name, sizeof(name), "demos/%s.%s%d", demoName, DEMOEXT, com_legacyprotocol->integer);
			else
#endif
				Com_sprintf(name, sizeof(name), "demos/%s.%s%d", demoName, DEMOEXT, com_protocol->integer);

			if (!FS_FileExists_HomeData(name))
				break;	// file doesn't exist
		}
	}

	// open the demo file

	Com_Printf ("recording to %s.\n", name);
	clc.demofile = FS_FOpenFileWrite_HomeData( name );
	if ( !clc.demofile ) {
		Com_Printf ("ERROR: couldn't open.\n");
		return;
	}
	clc.demorecording = true;
	if (CvarSystem::variableValue("ui_recordSPDemo")) {
	  clc.spDemoRecording = true;
	} else {
	  clc.spDemoRecording = false;
	}

	Q_strncpyz( clc.demoName, demoName, sizeof( clc.demoName ) );

	// don't start saving messages until a non-delta compressed message is received
	clc.demowaiting = true;

	// write out the gamestate message
	buf.init( bufData, sizeof(bufData));
	buf.bitstream();

	// NOTE, MRE: all server->client messages now acknowledge
	buf.writeLong( clc.reliableSequence );

	buf.writeByte( svc_gamestate);
	buf.writeLong( clc.serverCommandSequence );

	// configstrings
	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		if ( !cl.gameState.stringOffsets[i] ) {
			continue;
		}
		s = cl.gameState.stringData + cl.gameState.stringOffsets[i];
		buf.writeByte( svc_configstring);
		buf.writeShort( i);
		buf.writeBigString( s);
	}

	// baselines
	Com_Memset (&nullstate, 0, sizeof(nullstate));
	for ( i = 0; i < MAX_GENTITIES ; i++ ) {
		ent = &cl.entityBaselines[i];
		if ( !ent->number ) {
			continue;
		}
		buf.writeByte( svc_baseline);		
		buf.writeDeltaEntity( &nullstate, ent, true );
	}

	buf.writeByte( svc_EOF );
	
	// finished writing the gamestate stuff

	// write the client num
	buf.writeLong( clc.clientNum);
	// write the checksum feed
	buf.writeLong( clc.checksumFeed);

	// finished writing the client packet
	buf.writeByte( svc_EOF );

	// write it to the demo file
	len = LittleLong( clc.serverMessageSequence - 1 );
	FS_Write (&len, 4, clc.demofile);

	len = LittleLong (buf.cursize);
	FS_Write (&len, 4, clc.demofile);
	FS_Write (buf.data, buf.cursize, clc.demofile);

	// the rest of the demo file will be copied from net messages
}

/*
=======================================================================

CLIENT SIDE DEMO PLAYBACK

=======================================================================
*/

/*
=================
CL_DemoFrameDurationSDev
=================
*/
static float CL_DemoFrameDurationSDev( )
{
	int i;
	int numFrames;
	float mean = 0.0f;
	float variance = 0.0f;

	if( ( clc.timeDemoFrames - 1 ) > MAX_TIMEDEMO_DURATIONS )
		numFrames = MAX_TIMEDEMO_DURATIONS;
	else
		numFrames = clc.timeDemoFrames - 1;

	for( i = 0; i < numFrames; i++ )
		mean += clc.timeDemoDurations[ i ];
	mean /= numFrames;

	for( i = 0; i < numFrames; i++ )
	{
		float x = clc.timeDemoDurations[ i ];

		variance += ( ( x - mean ) * ( x - mean ) );
	}
	variance /= numFrames;

	return sqrt( variance );
}

/*
=================
CL_DemoCompleted
=================
*/
void CL_DemoCompleted( )
{
	char buffer[ MAX_STRING_CHARS ];

	if( cl_timedemo && cl_timedemo->integer )
	{
		int	time;
		
		time = Sys_Milliseconds() - clc.timeDemoStart;
		if( time > 0 )
		{
			// Millisecond times are frame durations:
			// minimum/average/maximum/std deviation
			Com_sprintf( buffer, sizeof( buffer ),
					"%i frames %3.1f seconds %3.1f fps %d.0/%.1f/%d.0/%.1f ms\n",
					clc.timeDemoFrames,
					time/1000.0,
					clc.timeDemoFrames*1000.0 / time,
					clc.timeDemoMinDuration,
					time / static_cast<float>(clc.timeDemoFrames),
					clc.timeDemoMaxDuration,
					CL_DemoFrameDurationSDev( ) );
			Com_Printf( "%s", buffer );

			// Write a log of all the frame durations
			if( cl_timedemoLog && strlen( cl_timedemoLog->string ) > 0 )
			{
				int i;
				int numFrames;
				fileHandle_t f;

				if( ( clc.timeDemoFrames - 1 ) > MAX_TIMEDEMO_DURATIONS )
					numFrames = MAX_TIMEDEMO_DURATIONS;
				else
					numFrames = clc.timeDemoFrames - 1;

				f = FS_FOpenFileWrite_HomeData( cl_timedemoLog->string );
				if( f )
				{
					FS_Printf( f, "# %s", buffer );

					for( i = 0; i < numFrames; i++ )
						FS_Printf( f, "%d\n", clc.timeDemoDurations[ i ] );

					FS_FCloseFile( f );
					Com_Printf( "%s written\n", cl_timedemoLog->string );
				}
				else
				{
					Com_Printf( "Couldn't open %s for writing\n",
							cl_timedemoLog->string );
				}
			}
		}
	}

	CL_Disconnect( true );
	CL_NextDemo();
}

/*
=================
CL_ReadDemoMessage
=================
*/
void CL_ReadDemoMessage( ) {
	int			r;
	Message		buf;
	byte		bufData[ MAX_MSGLEN ];
	int			s;

	if ( !clc.demofile ) {
		CL_DemoCompleted ();
		return;
	}

	// get the sequence number
	r = FS_Read( &s, 4, clc.demofile);
	if ( r != 4 ) {
		CL_DemoCompleted ();
		return;
	}
	clc.serverMessageSequence = LittleLong( s );

	// init the message
	buf.init( bufData, sizeof( bufData ) );

	// get the length
	r = FS_Read (&buf.cursize, 4, clc.demofile);
	if ( r != 4 ) {
		CL_DemoCompleted ();
		return;
	}
	buf.cursize = LittleLong( buf.cursize );
	if ( buf.cursize == -1 ) {
		CL_DemoCompleted ();
		return;
	}
	if ( buf.cursize > buf.maxsize ) {
		Com_Error (ERR_DROP, "CL_ReadDemoMessage: demoMsglen > MAX_MSGLEN");
	}
	r = FS_Read( buf.data, buf.cursize, clc.demofile );
	if ( r != buf.cursize ) {
		Com_Printf( "Demo file was truncated.\n");
		CL_DemoCompleted ();
		return;
	}

	clc.lastPacketTime = cls.realtime;
	buf.readcount = 0;
	CL_ParseServerMessage( &buf );
}

/*
====================
CL_WalkDemoExt
====================
*/
static int CL_WalkDemoExt(char *arg, char *name, int *demofile)
{
	int i = 0;
	*demofile = 0;

#ifdef LEGACY_PROTOCOL
	if(com_legacyprotocol->integer > 0)
	{
		Com_sprintf(name, MAX_OSPATH, "demos/%s.%s%d", arg, DEMOEXT, com_legacyprotocol->integer);
		FS_FOpenFileRead(name, demofile, true);
		
		if (*demofile)
		{
			Com_Printf("Demo file: %s\n", name);
			return com_legacyprotocol->integer;
		}
	}
	
	if(com_protocol->integer != com_legacyprotocol->integer)
#endif
	{
		Com_sprintf(name, MAX_OSPATH, "demos/%s.%s%d", arg, DEMOEXT, com_protocol->integer);
		FS_FOpenFileRead(name, demofile, true);

		if (*demofile)
		{
			Com_Printf("Demo file: %s\n", name);
			return com_protocol->integer;
		}
	}

	Com_Printf("Not found: %s\n", name);

	while(demo_protocols[i])
	{
#ifdef LEGACY_PROTOCOL
		if(demo_protocols[i] == com_legacyprotocol->integer)
			continue;
#endif
		if(demo_protocols[i] == com_protocol->integer)
			continue;
	
		Com_sprintf (name, MAX_OSPATH, "demos/%s.%s%d", arg, DEMOEXT, demo_protocols[i]);
		FS_FOpenFileRead( name, demofile, true );
		if (*demofile)
		{
			Com_Printf("Demo file: %s\n", name);

			return demo_protocols[i];
		}
		else
			Com_Printf("Not found: %s\n", name);
		i++;
	}
	
	return -1;
}

/*
====================
CL_CompleteDemoName
====================
*/
static void CL_CompleteDemoName( char *args, int argNum )
{
	if( argNum == 2 )
	{
		char demoFilter[ 16 ];

		Com_sprintf( demoFilter, sizeof(demoFilter), "*.%s*", DEMOEXT );
		Field_CompleteFilename( "demos", "", demoFilter, false, true );
	}
}

/*
====================
CL_PlayDemo_f

demo <demoname>

====================
*/
void CL_PlayDemo_f( ) {
	char		name[MAX_OSPATH];
	char		arg[MAX_OSPATH];
	char		*ext_test;
	int			protocol, i;
	char		retry[MAX_OSPATH];

	if (CommandSystem::argc() != 2) {
		Com_Printf ("demo <demoname>\n");
		return;
	}

	// make sure a local server is killed
	// 2 means don't force disconnect of local client
	CvarSystem::set( "sv_killserver", "2" );

	// open the demo file
	Q_strncpyz( arg, CommandSystem::argv(1), sizeof( arg ) );
	
	CL_Disconnect( true );

	// check for an extension .DEMOEXT_?? (?? is protocol)
	ext_test = strrchr(arg, '.');
	
	if(ext_test && !Q_stricmpn(ext_test + 1, DEMOEXT, ARRAY_LEN(DEMOEXT) - 1))
	{
		protocol = atoi(ext_test + ARRAY_LEN(DEMOEXT));

		for(i = 0; demo_protocols[i]; i++)
		{
			if(demo_protocols[i] == protocol)
				break;
		}

		if(demo_protocols[i] || protocol == com_protocol->integer
#ifdef LEGACY_PROTOCOL
		   || protocol == com_legacyprotocol->integer
#endif
		  )
		{
			Com_sprintf(name, sizeof(name), "demos/%s", arg);
			FS_FOpenFileRead(name, &clc.demofile, true);
		}
		else
		{
			int len;

			Com_Printf("Protocol %d not supported for demos\n", protocol);
			len = ext_test - arg;

			if(len >= ARRAY_LEN(retry))
				len = ARRAY_LEN(retry) - 1;

			Q_strncpyz(retry, arg, len + 1);
			retry[len] = '\0';
			protocol = CL_WalkDemoExt(retry, name, &clc.demofile);
		}
	}
	else
		protocol = CL_WalkDemoExt(arg, name, &clc.demofile);
	
	if (!clc.demofile) {
		Com_Error( ERR_DROP, "couldn't open %s", name);
		return;
	}
	Q_strncpyz( clc.demoName, arg, sizeof( clc.demoName ) );

	Con_Close();

	clc.state = CA_CONNECTED;
	clc.demoplaying = true;
	Q_strncpyz( clc.servername, arg, sizeof( clc.servername ) );

#ifdef LEGACY_PROTOCOL
	if(protocol <= com_legacyprotocol->integer)
		clc.compat = true;
	else
		clc.compat = false;
#endif

	// read demo messages until connected
	while ( clc.state >= CA_CONNECTED && clc.state < CA_PRIMED ) {
		CL_ReadDemoMessage();
	}
	// don't get the first snapshot this frame, to prevent the long
	// time from the gamestate load from messing causing a time skip
	clc.firstDemoFrameSkipped = false;
}


/*
====================
CL_StartDemoLoop

Closing the main menu will restart the demo loop
====================
*/
void CL_StartDemoLoop( ) {
	// start the demo loop again
	CommandBuffer::addText ("d1\n");
	Key_SetCatcher( 0 );
}

/*
==================
CL_NextDemo

Called when a demo or cinematic finishes
If the "nextdemo" cvar is set, that command will be issued
==================
*/
void CL_NextDemo( ) {
	char	v[MAX_STRING_CHARS];

	Q_strncpyz( v, CvarSystem::variableString ("nextdemo"), sizeof(v) );
	v[MAX_STRING_CHARS-1] = 0;
	Com_DPrintf("CL_NextDemo: %s\n", v );
	if (!v[0]) {
		return;
	}

	CvarSystem::set ("nextdemo","");
	CommandBuffer::addText (v);
	CommandBuffer::addText ("\n");
	CommandBuffer::execute();
}


//======================================================================

/*
=====================
CL_ShutdownAll
=====================
*/
void CL_ShutdownAll(bool shutdownRef)
{
	if(CL_VideoRecording())
		CL_CloseAVI();

	if(clc.demorecording)
		CL_StopRecord_f();

	// clear sounds
	S_DisableSounds();
	// shutdown CGame
	CL_ShutdownCGame();
	// shutdown UI
	CL_ShutdownUI();

	// shutdown the renderer
	if(shutdownRef)
		CL_ShutdownRef();
	else if(refExport.Shutdown)
		refExport.Shutdown(false);		// don't destroy window or context

	cls.uiStarted = false;
	cls.cgameStarted = false;
	cls.rendererStarted = false;
	cls.soundRegistered = false;
}

/*
=================
CL_ClearMemory

Called by Com_GameRestart
=================
*/
void CL_ClearMemory(bool shutdownRef)
{
	// shutdown all the client stuff
	CL_ShutdownAll(shutdownRef);

	// if not running a server clear the whole hunk
	if ( !com_sv_running || !com_sv_running->integer ) {
		// clear the whole hunk
		Hunk_Clear();
		// clear collision map data
		CM_ClearMap();
	}
	else {
		// clear all the client data on the hunk
		Hunk_ClearToMark();
	}
}

/*
=================
CL_FlushMemory

Called by CL_MapLoading, CL_Connect_f, CL_PlayDemo_f, and CL_ParseGamestate the only
ways a client gets into a game
Also called by Com_Error
=================
*/
void CL_FlushMemory()
{
	CL_ClearMemory(false);
	CL_StartHunkUsers(false);
}

/*
=====================
CL_MapLoading

A local server is starting to load a map, so update the
screen to let the user know about it, then dump all client
memory on the hunk from cgame, ui, and renderer
=====================
*/
void CL_MapLoading( ) {
	if ( com_dedicated->integer ) {
		clc.state = CA_DISCONNECTED;
		Key_SetCatcher( KEYCATCH_CONSOLE );
		return;
	}

	if ( !com_cl_running->integer ) {
		return;
	}

	Con_Close();
	Key_SetCatcher( 0 );

	// if we are already connected to the local host, stay connected
	if ( clc.state >= CA_CONNECTED && !Q_stricmp( clc.servername, "localhost" ) ) {
		clc.state = CA_CONNECTED;		// so the connect screen is drawn
		Com_Memset( cls.updateInfoString, 0, sizeof( cls.updateInfoString ) );
		Com_Memset( clc.serverMessage, 0, sizeof( clc.serverMessage ) );
		Com_Memset( &cl.gameState, 0, sizeof( cl.gameState ) );
		clc.lastPacketSentTime = -9999;
		SCR_UpdateScreen();
	} else {
		// clear nextmap so the cinematic shutdown doesn't execute it
		CvarSystem::set( "nextmap", "" );
		CL_Disconnect( true );
		Q_strncpyz( clc.servername, "localhost", sizeof(clc.servername) );
		clc.state = CA_CHALLENGING;		// so the connect screen is drawn
		Key_SetCatcher( 0 );
		SCR_UpdateScreen();
		clc.connectTime = -RETRANSMIT_TIMEOUT;
		Network::stringToAdr( clc.servername, &clc.serverAddress, NA_UNSPEC);
		// we don't need a challenge on the localhost

		CL_CheckForResend();
	}
}

/*
=====================
CL_ClearState

Called before parsing a gamestate
=====================
*/
void CL_ClearState () {

//	S_StopAllSounds();

	Com_Memset( &cl, 0, sizeof( cl ) );
}

/*
====================
CL_UpdateGUID

update cl_guid using QKEY_FILE and optional prefix
====================
*/
static void CL_UpdateGUID( const char *prefix, int prefix_len )
{
	fileHandle_t f;
	int len;

	len = FS_BaseDir_FOpenFileRead( QKEY_FILE, &f );
	FS_FCloseFile( f );

	if( len != QKEY_SIZE ) 
		CvarSystem::set( "cl_guid", "" );
	else
		CvarSystem::set( "cl_guid", Com_MD5File( QKEY_FILE, QKEY_SIZE,
			prefix, prefix_len ) );
}

static void CL_OldGame()
{
	if(cl_oldGameSet)
	{
		// change back to previous fs_game
		cl_oldGameSet = false;
		CvarSystem::set2("fs_game", cl_oldGame, true);
		FS_ConditionalRestart(clc.checksumFeed, false);
	}
}

/*
=====================
CL_Disconnect

Called when a connection, demo, or cinematic is being terminated.
Goes from a connected state to either a menu state or a console state
Sends a disconnect message to the server
This is also called on Com_Error and Com_Quit, so it shouldn't cause any errors
=====================
*/
void CL_Disconnect( bool showMainMenu ) {
	if ( !com_cl_running || !com_cl_running->integer ) {
		return;
	}

	// shutting down the client so enter full screen ui mode
	CvarSystem::set("r_uiFullScreen", "1");

	if ( clc.demorecording ) {
		CL_StopRecord_f ();
	}

	if (clc.download) {
		FS_FCloseFile( clc.download );
		clc.download = 0;
	}
	*clc.downloadTempName = *clc.downloadName = 0;
	CvarSystem::set( "cl_downloadName", "" );

#ifdef USE_MUMBLE
	if (cl_useMumble->integer && mumble_islinked()) {
		Com_Printf("Mumble: Unlinking from Mumble application\n");
		mumble_unlink();
	}
#endif

#ifdef USE_VOIP
	if (cl_voipSend->integer) {
		int tmp = cl_voipUseVAD->integer;
		cl_voipUseVAD->integer = 0;  // disable this for a moment.
		clc.voipOutgoingDataSize = 0;  // dump any pending VoIP transmission.
		CvarSystem::set("cl_voipSend", "0");
		CL_CaptureVoip();  // clean up any state...
		cl_voipUseVAD->integer = tmp;
	}

	if (clc.voipCodecInitialized) {
		int i;
		opus_encoder_destroy(clc.opusEncoder);
		for (i = 0; i < MAX_CLIENTS; i++) {
			opus_decoder_destroy(clc.opusDecoder[i]);
		}
		clc.voipCodecInitialized = false;
	}
	CommandSystem::removeCommand ("voip");
#endif

	if ( clc.demofile ) {
		FS_FCloseFile( clc.demofile );
		clc.demofile = 0;
	}

	if ( theUI.isInitialized() && showMainMenu ) {
		theUI.setActiveMenu( UserInterface::MenuCommand::UIMENU_NONE );
	}

	SCR_StopCinematic ();
	S_ClearSoundBuffer();

	// send a disconnect message to the server
	// send it a few times in case one is dropped
	if ( clc.state >= CA_CONNECTED ) {
		CL_AddReliableCommand("disconnect", true);
		CL_WritePacket();
		CL_WritePacket();
		CL_WritePacket();
	}
	
	// Remove pure paks
	FS_PureServerSetLoadedPaks("", "");
	FS_PureServerSetReferencedPaks( "", "" );
	
	CL_ClearState ();

	// wipe the client connection
	Com_Memset( &clc, 0, sizeof( clc ) );

	clc.state = CA_DISCONNECTED;

	// allow cheats locally
	CvarSystem::set( "sv_cheats", "1" );

	// not connected to a pure server anymore
	cl_connectedToPureServer = false;

#ifdef USE_VOIP
	// not connected to voip server anymore.
	clc.voipEnabled = false;
#endif

	// Stop recording any video
	if( CL_VideoRecording( ) ) {
		// Finish rendering current frame
		SCR_UpdateScreen( );
		CL_CloseAVI( );
	}

	CL_UpdateGUID( nullptr, 0 );

	if(!noGameRestart)
		CL_OldGame();
	else
		noGameRestart = false;
}


/*
===================
CL_ForwardCommandToServer

adds the current command line as a clientCommand
things like godmode, noclip, etc, are commands directed to the server,
so when they are typed in at the console, they will need to be forwarded.
===================
*/
void CL_ForwardCommandToServer( const char *string ) {
	char	*cmd;

	cmd = CommandSystem::argv(0);

	// ignore key up commands
	if ( cmd[0] == '-' ) {
		return;
	}

	if ( clc.demoplaying || clc.state < CA_CONNECTED || cmd[0] == '+' ) {
		Com_Printf ("Unknown command \"%s" S_COLOR_WHITE "\"\n", cmd);
		return;
	}

	if ( CommandSystem::argc() > 1 ) {
		CL_AddReliableCommand(string, false);
	} else {
		CL_AddReliableCommand(cmd, false);
	}
}

/*
===================
CL_RequestMotd

===================
*/
void CL_RequestMotd( ) {
#ifdef UPDATE_SERVER_NAME
	char		info[MAX_INFO_STRING];

	if ( !cl_motd->integer ) {
		return;
	}
	Com_Printf( "Resolving %s\n", UPDATE_SERVER_NAME );
	if ( !Network::stringToAdr( UPDATE_SERVER_NAME, &cls.updateServer, NA_IP ) ) {
		Com_Printf( "Couldn't resolve address\n" );
		return;
	}
	cls.updateServer.port = BigShort( PORT_UPDATE );
	Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", UPDATE_SERVER_NAME,
		cls.updateServer.ip[0], cls.updateServer.ip[1],
		cls.updateServer.ip[2], cls.updateServer.ip[3],
		BigShort( cls.updateServer.port ) );
	
	info[0] = 0;

	Com_sprintf( cls.updateChallenge, sizeof( cls.updateChallenge ), "%i", (int)((((unsigned int)rand() << 16) ^ (unsigned int)rand()) ^ Com_Milliseconds()));

	Info_SetValueForKey( info, "challenge", cls.updateChallenge );
	Info_SetValueForKey( info, "renderer", cls.glconfig.renderer_string );
	Info_SetValueForKey( info, "version", com_version->string );

	Network::outOfBandPrint( NS_CLIENT, cls.updateServer, "getmotd \"%s\"\n", info );
#endif
}

/*
===================
CL_RequestAuthorization

Authorization server protocol
-----------------------------

All commands are text in Q3 out of band packets (leading 0xff 0xff 0xff 0xff).

Whenever the client tries to get a challenge from the server it wants to
connect to, it also blindly fires off a packet to the authorize server:

getKeyAuthorize <challenge> <cdkey>

cdkey may be "demo"


#OLD The authorize server returns a:
#OLD 
#OLD keyAthorize <challenge> <accept | deny>
#OLD 
#OLD A client will be accepted if the cdkey is valid and it has not been used by any other IP
#OLD address in the last 15 minutes.


The server sends a:

getIpAuthorize <challenge> <ip>

The authorize server returns a:

ipAuthorize <challenge> <accept | deny | demo | unknown >

A client will be accepted if a valid cdkey was sent by that ip (only) in the last 15 minutes.
If no response is received from the authorize server after two tries, the client will be let
in anyway.
===================
*/
#ifndef STANDALONE
void CL_RequestAuthorization( void ) {
	char	nums[64];
	int		i, j, l;
	cvar_t	*fs;

	if ( !cls.authorizeServer.port ) {
		Com_Printf( "Resolving %s\n", AUTHORIZE_SERVER_NAME );
		if ( !Network::stringToAdr( AUTHORIZE_SERVER_NAME, &cls.authorizeServer, NA_IP ) ) {
			Com_Printf( "Couldn't resolve address\n" );
			return;
		}

		cls.authorizeServer.port = BigShort( PORT_AUTHORIZE );
		Com_Printf( "%s resolved to %i.%i.%i.%i:%i\n", AUTHORIZE_SERVER_NAME,
			cls.authorizeServer.ip[0], cls.authorizeServer.ip[1],
			cls.authorizeServer.ip[2], cls.authorizeServer.ip[3],
			BigShort( cls.authorizeServer.port ) );
	}
	if ( cls.authorizeServer.type == NA_BAD ) {
		return;
	}

	// only grab the alphanumeric values from the cdkey, to avoid any dashes or spaces
	j = 0;
	l = strlen( cl_cdkey );
	if ( l > 32 ) {
		l = 32;
	}
	for ( i = 0 ; i < l ; i++ ) {
		if ( ( cl_cdkey[i] >= '0' && cl_cdkey[i] <= '9' )
				|| ( cl_cdkey[i] >= 'a' && cl_cdkey[i] <= 'z' )
				|| ( cl_cdkey[i] >= 'A' && cl_cdkey[i] <= 'Z' )
			 ) {
			nums[j] = cl_cdkey[i];
			j++;
		}
	}
	nums[j] = 0;

	fs = CvarSystem::get ("cl_anonymous", "0", CVAR_INIT|CVAR_SYSTEMINFO );

	Network::outOfBandPrint(NS_CLIENT, cls.authorizeServer, "getKeyAuthorize %i %s", fs->integer, nums );
}
#endif
/*
======================================================================

CONSOLE COMMANDS

======================================================================
*/

/*
==================
CL_ForwardToServer_f
==================
*/
void CL_ForwardToServer_f( ) {
	if ( clc.state != CA_ACTIVE || clc.demoplaying ) {
		Com_Printf ("Not connected to a server.\n");
		return;
	}
	
	// don't forward the first argument
	if ( CommandSystem::argc() > 1 ) {
		CL_AddReliableCommand(CommandSystem::args(), false);
	}
}

/*
==================
CL_Disconnect_f
==================
*/
void CL_Disconnect_f( ) {
	SCR_StopCinematic();
	CvarSystem::set("ui_singlePlayerActive", "0");
	if ( clc.state != CA_DISCONNECTED && clc.state != CA_CINEMATIC ) {
		Com_Error (ERR_DISCONNECT, "Disconnected from server");
	}
}


/*
================
CL_Reconnect_f

================
*/
void CL_Reconnect_f( ) {
	if ( !strlen( cl_reconnectArgs ) )
		return;
	CvarSystem::set("ui_singlePlayerActive", "0");
	CommandBuffer::addText( va("connect %s\n", cl_reconnectArgs ) );
}

/*
================
CL_Connect_f

================
*/
void CL_Connect_f( ) {
	char	server[MAX_OSPATH];
	const char	*serverString;
	int argc = CommandSystem::argc();
	netadrtype_t family = NA_UNSPEC;

	if ( argc != 2 && argc != 3 ) {
		Com_Printf( "usage: connect [-4|-6] server\n");
		return;	
	}
	
	if(argc == 2)
		Q_strncpyz( server, CommandSystem::argv(1), sizeof( server ) );
	else
	{
		if(!strcmp(CommandSystem::argv(1), "-4"))
			family = NA_IP;
		else if(!strcmp(CommandSystem::argv(1), "-6"))
			family = NA_IP6;
		else
			Com_Printf( "warning: only -4 or -6 as address type understood.\n");
		
		Q_strncpyz( server, CommandSystem::argv(2), sizeof( server ) );
	}

	// save arguments for reconnect
	Q_strncpyz( cl_reconnectArgs, CommandSystem::args(), sizeof( cl_reconnectArgs ) );

	CvarSystem::set("ui_singlePlayerActive", "0");

	// fire a message off to the motd server
	CL_RequestMotd();

	// clear any previous "server full" type messages
	clc.serverMessage[0] = 0;

	if ( com_sv_running->integer && !strcmp( server, "localhost" ) ) {
		// if running a local server, kill it
		SV_Shutdown( "Server quit" );
	}

	// make sure a local server is killed
	CvarSystem::set( "sv_killserver", "1" );
	SV_Frame( 0 );

	noGameRestart = true;
	CL_Disconnect( true );
	Con_Close();

	Q_strncpyz( clc.servername, server, sizeof(clc.servername) );

	if (!Network::stringToAdr(clc.servername, &clc.serverAddress, family) ) {
		Com_Printf ("Bad server address\n");
		clc.state = CA_DISCONNECTED;
		return;
	}
	if (clc.serverAddress.port == 0) {
		clc.serverAddress.port = BigShort( PORT_SERVER );
	}

	serverString = Network::adrToStringwPort(clc.serverAddress);

	Com_Printf( "%s resolved to %s\n", clc.servername, serverString);

	if( cl_guidServerUniq->integer )
		CL_UpdateGUID( serverString, strlen( serverString ) );
	else
		CL_UpdateGUID( nullptr, 0 );

	// if we aren't playing on a lan, we need to authenticate
	// with the cd key
	if(Network::isLocalAddress(clc.serverAddress))
		clc.state = CA_CHALLENGING;
	else
	{
		clc.state = CA_CONNECTING;
		
		// Set a client challenge number that ideally is mirrored back by the server.
		clc.challenge = ((static_cast<unsigned int>(rand()) << 16) ^ static_cast<unsigned int>(rand())) ^ Com_Milliseconds();
	}

	Key_SetCatcher( 0 );
	clc.connectTime = -99999;	// CL_CheckForResend() will fire immediately
	clc.connectPacketCount = 0;

	// server connection string
	CvarSystem::set( "cl_currentServerAddress", server );
}

constexpr int MAX_RCON_MESSAGE = 1024;

/*
==================
CL_CompleteRcon
==================
*/
static void CL_CompleteRcon( char *args, int argNum )
{
	if( argNum == 2 )
	{
		// Skip "rcon "
		char *p = Com_SkipTokens( args, 1, " " );

		if( p > args )
			Field_CompleteCommand( p, true, true );
	}
}

/*
==================
CL_CompletePlayerName
==================
*/
static void CL_CompletePlayerName( char *args, int argNum )
{
	if( argNum == 2 )
	{
		char		names[MAX_CLIENTS][MAX_NAME_LENGTH];
		const char	*namesPtr[MAX_CLIENTS];
		int			i;
		int			clientCount;
		int			nameCount;
		const char *info;
		const char *name;

		//configstring
		info = cl.gameState.stringData + cl.gameState.stringOffsets[CS_SERVERINFO];
		clientCount = atoi( Info_ValueForKey( info, "sv_maxclients" ) );

		nameCount = 0;

		for( i = 0; i < clientCount; i++ ) {
			if( i == clc.clientNum )
				continue;

			info = cl.gameState.stringData + cl.gameState.stringOffsets[CS_PLAYERS+i];

			name = Info_ValueForKey( info, "n" );
			if( name[0] == '\0' )
				continue;
			Q_strncpyz( names[nameCount], name, sizeof(names[nameCount]) );
			Q_CleanStr( names[nameCount] );

			namesPtr[nameCount] = names[nameCount];
			nameCount++;
		}
		qsort( (void*)namesPtr, nameCount, sizeof( namesPtr[0] ), Com_strCompare );

		Field_CompletePlayerName( namesPtr, nameCount );
	}
}

/*
=====================
CL_Rcon_f

  Send the rest of the command line over as
  an unconnected command.
=====================
*/
void CL_Rcon_f( ) {
	char	message[MAX_RCON_MESSAGE];
	netadr_t	to;

	if ( !rcon_client_password->string[0] ) {
		Com_Printf ("You must set 'rconpassword' before\n"
					"issuing an rcon command.\n");
		return;
	}

	message[0] = -1;
	message[1] = -1;
	message[2] = -1;
	message[3] = -1;
	message[4] = 0;

	Q_strcat (message, MAX_RCON_MESSAGE, "rcon ");

	Q_strcat (message, MAX_RCON_MESSAGE, rcon_client_password->string);
	Q_strcat (message, MAX_RCON_MESSAGE, " ");

	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=543
	Q_strcat (message, MAX_RCON_MESSAGE, CommandSystem::cmd()+5);

	if ( clc.state >= CA_CONNECTED ) {
		to = clc.netchan.remoteAddress;
	} else {
		if (!strlen(rconAddress->string)) {
			Com_Printf ("You must either be connected,\n"
						"or set the 'rconAddress' cvar\n"
						"to issue rcon commands\n");

			return;
		}
		Network::stringToAdr (rconAddress->string, &to, NA_UNSPEC);
		if (to.port == 0) {
			to.port = BigShort (PORT_SERVER);
		}
	}
	
	Network::sendPacket (NS_CLIENT, strlen(message)+1, message, to);
	cls.rconAddress = to;
}

/*
=================
CL_SendPureChecksums
=================
*/
void CL_SendPureChecksums( ) {
	char cMsg[MAX_INFO_VALUE];

	// if we are pure we need to send back a command with our referenced pk3 checksums
	Com_sprintf(cMsg, sizeof(cMsg), "cp %d %s", cl.serverId, FS_ReferencedPakPureChecksums());

	CL_AddReliableCommand(cMsg, false);
}

/*
=================
CL_ResetPureClientAtServer
=================
*/
void CL_ResetPureClientAtServer( ) {
	CL_AddReliableCommand("vdr", false);
}

/*
=================
CL_Vid_Restart_f

Restart the video subsystem

we also have to reload the UI and CGame because the renderer
doesn't know what graphics to reload
=================
*/
void CL_Vid_Restart_f( ) {

	// Settings may have changed so stop recording now
	if( CL_VideoRecording( ) ) {
		CL_CloseAVI( );
	}

	if(clc.demorecording)
		CL_StopRecord_f();

	// don't let them loop during the restart
	S_StopAllSounds();

	if(!FS_ConditionalRestart(clc.checksumFeed, true))
	{
		// if not running a server clear the whole hunk
		if(com_sv_running->integer)
		{
			// clear all the client data on the hunk
			Hunk_ClearToMark();
		}
		else
		{
			// clear the whole hunk
			Hunk_Clear();
		}
	
		// shutdown the UI
		CL_ShutdownUI();
		// shutdown the CGame
		CL_ShutdownCGame();
		// shutdown the renderer and clear the renderer interface
		CL_ShutdownRef();
		// client is no longer pure until new checksums are sent
		CL_ResetPureClientAtServer();
		// clear pak references
		FS_ClearPakReferences( FS_UI_REF | FS_CGAME_REF );
		// reinitialize the filesystem if the game directory or checksum has changed

		cls.rendererStarted = false;
		cls.uiStarted = false;
		cls.cgameStarted = false;
		cls.soundRegistered = false;

		// unpause so the cgame definitely gets a snapshot and renders a frame
		CvarSystem::set("cl_paused", "0");

		// initialize the renderer interface
		CL_InitRef();

		// startup all the client stuff
		CL_StartHunkUsers(false);

		// start the cgame if connected
		if(clc.state > CA_CONNECTED && clc.state != CA_CINEMATIC)
		{
			cls.cgameStarted = true;
			CL_InitCGame();
			// send pure checksums
			CL_SendPureChecksums();
		}
	}
}

/*
=================
CL_Snd_Restart

Restart the sound subsystem
=================
*/
void CL_Snd_Shutdown()
{
	S_Shutdown();
	cls.soundStarted = false;
}

/*
=================
CL_Snd_Restart_f

Restart the sound subsystem
The cgame and game must also be forced to restart because
handles will be invalid
=================
*/
void CL_Snd_Restart_f()
{
	CL_Snd_Shutdown();
	// sound will be reinitialized by vid_restart
	CL_Vid_Restart_f();
}


/*
==================
CL_PK3List_f
==================
*/
void CL_OpenedPK3List_f( ) {
	Com_Printf("Opened PK3 Names: %s\n", FS_LoadedPakNames());
}

/*
==================
CL_PureList_f
==================
*/
void CL_ReferencedPK3List_f( ) {
	Com_Printf("Referenced PK3 Names: %s\n", FS_ReferencedPakNames());
}

/*
==================
CL_Configstrings_f
==================
*/
void CL_Configstrings_f( ) {
	int		i;
	int		ofs;

	if ( clc.state != CA_ACTIVE ) {
		Com_Printf( "Not connected to a server.\n");
		return;
	}

	for ( i = 0 ; i < MAX_CONFIGSTRINGS ; i++ ) {
		ofs = cl.gameState.stringOffsets[ i ];
		if ( !ofs ) {
			continue;
		}
		Com_Printf( "%4i: %s\n", i, cl.gameState.stringData + ofs );
	}
}

/*
==============
CL_Clientinfo_f
==============
*/
void CL_Clientinfo_f( ) {
	Com_Printf( "--------- Client Information ---------\n" );
	Com_Printf( "state: %i\n", clc.state );
	Com_Printf( "Server: %s\n", clc.servername );
	Com_Printf ("User info settings:\n");
	Info_Print( CvarSystem::infoString( CVAR_USERINFO ) );
	Com_Printf( "--------------------------------------\n" );
}


//====================================================================

/*
=================
CL_DownloadsComplete

Called when all downloading has been completed
=================
*/
void CL_DownloadsComplete( ) {

#ifdef USE_HTTP
	// if we downloaded with HTTP
	if(clc.httpUsed) {
		clc.httpUsed = false;
		if( clc.disconnectedForHttpDownload ) {
			if(clc.downloadRestart) {
				FS_Restart(clc.checksumFeed);
				clc.downloadRestart = false;
			}
			clc.disconnectedForHttpDownload = false;
			CL_Reconnect_f();
			return;
		}
	}
#endif

	// if we downloaded files we need to restart the file system
	if (clc.downloadRestart) {
		clc.downloadRestart = false;

		FS_Restart(clc.checksumFeed); // We possibly downloaded a pak, restart the file system to load it

		// inform the server so we get new gamestate info
		CL_AddReliableCommand("donedl", false);

		// by sending the donedl command we request a new gamestate
		// so we don't want to load stuff yet
		return;
	}

	// let the client game init and load data
	clc.state = CA_LOADING;

	// Pump the loop, this may change gamestate!
	Com_EventLoop();

	// if the gamestate was changed by calling Com_EventLoop
	// then we loaded everything already and we don't want to do it again.
	if ( clc.state != CA_LOADING ) {
		return;
	}

	// starting to load a map so we get out of full screen ui mode
	CvarSystem::set("r_uiFullScreen", "0");

	// flush client memory and start loading stuff
	// this will also (re)load the UI
	// if this is a local client then only the client part of the hunk
	// will be cleared, note that this is done after the hunk mark has been set
	CL_FlushMemory();

	// initialize the CGame
	cls.cgameStarted = true;
	CL_InitCGame();

	// set pure checksums
	CL_SendPureChecksums();

	CL_WritePacket();
	CL_WritePacket();
	CL_WritePacket();
}

/*
=================
CL_InitDownload
=================
*/
static void CL_InitDownload( const char *localName ) {
	Q_strncpyz ( clc.downloadName, localName, sizeof(clc.downloadName) );
	Com_sprintf( clc.downloadTempName, sizeof(clc.downloadTempName), "%s.tmp", localName );

	// Set so UI gets access to it
	CvarSystem::set( "cl_downloadName", localName );
	CvarSystem::set( "cl_downloadSize", "0" );
	CvarSystem::set( "cl_downloadCount", "0" );
	CvarSystem::setValue( "cl_downloadTime", cls.realtime );

	clc.downloadBlock = 0; // Starting new file
	clc.downloadCount = 0;
}

/*
=================
CL_BeginDownload

Requests a file to download from the server.  Stores it in the current
game directory.
=================
*/
static void CL_BeginDownload( const char *remoteName ) {
	CL_AddReliableCommand(va("download %s", remoteName), false);
}

#ifdef USE_HTTP
/*
=================
CL_BeginHttpDownload
=================
*/
static void CL_BeginHttpDownload( const char *remoteURL ) {
	if(Q_strncmp(remoteURL, "http://", strlen("http://")) != 0 &&
		Q_strncmp(remoteURL, "https://", strlen("https://")) != 0) {
		Com_Error(ERR_DROP, "Download Error: %s is a malformed/"
			"unsupported URL", remoteURL);
	}

	Com_Printf("URL: %s\n", remoteURL);

	CL_HTTP_BeginDownload(remoteURL);
	Q_strncpyz(clc.downloadURL, remoteURL, sizeof(clc.downloadURL));

	clc.download = FS_BaseDir_FOpenFileWrite_HomeData(clc.downloadTempName);
	if(!clc.download) {
		Com_Error(ERR_DROP, "CL_BeginHTTPDownload: failed to open "
			"%s for writing", clc.downloadTempName);
	}

	if(!(clc.sv_allowDownload & DLF_NO_DISCONNECT) &&
		!clc.disconnectedForHttpDownload) {

		CL_AddReliableCommand("disconnect", true);
		CL_WritePacket();
		CL_WritePacket();
		CL_WritePacket();
		clc.disconnectedForHttpDownload = true;
	}

	clc.httpUsed = true;
}
#endif /* USE_HTTP */

/*
=================
CL_NextDownload

A download completed or failed
=================
*/
void CL_NextDownload()
{
	char *s;
	char *remoteName, *localName;
	bool usedHTTP = false;

	// A download has finished, check whether this matches a referenced checksum
	if(*clc.downloadName)
	{
		char *zippath = FS_BaseDir_BuildOSPath(CvarSystem::variableString("fs_homedatapath"), clc.downloadName);

		if(!FS_CompareZipChecksum(zippath))
			Com_Error(ERR_DROP, "Incorrect checksum for file: %s", clc.downloadName);
	}

	*clc.downloadTempName = *clc.downloadName = 0;
	CvarSystem::set("cl_downloadName", "");

	// We are looking to start a download here
	if (*clc.downloadList) {
		s = clc.downloadList;

		// format is:
		//  @remotename@localname@remotename@localname, etc.

		if (*s == '@')
			s++;
		remoteName = s;
		
		if ( (s = strchr(s, '@')) == nullptr ) {
			CL_DownloadsComplete();
			return;
		}

		*s++ = 0;
		localName = s;
		if ( (s = strchr(s, '@')) != nullptr )
			*s++ = 0;
		else
			s = localName + strlen(localName); // point at the nul byte
#ifdef USE_HTTP
		if(!(cl_allowDownload->integer & DLF_NO_REDIRECT)) {
			if(clc.sv_allowDownload & DLF_NO_REDIRECT) {
				Com_Printf("WARNING: server does not "
					"allow download redirection "
					"(sv_allowDownload is %d)\n",
					clc.sv_allowDownload);
			}
			else if(!*clc.sv_dlURL) {
				Com_Printf("WARNING: server allows "
					"download redirection, but does not "
					"have sv_dlURL set\n");
			}
			else if(CL_HTTP_Available()) {
				CL_InitDownload(localName);
				CL_BeginHttpDownload(va("%s/%s",
					clc.sv_dlURL, remoteName));

				usedHTTP = true;
			}
		}
		else if(!(clc.sv_allowDownload & DLF_NO_REDIRECT)) {
			Com_Printf("WARNING: server allows download "
				"redirection, but it disabled by client "
				"configuration (cl_allowDownload is %d)\n",
				cl_allowDownload->integer);
		}
#endif /* USE_HTTP */
		if(!usedHTTP) {
			if((cl_allowDownload->integer & DLF_NO_UDP)) {
				Com_Error(ERR_DROP, "UDP Downloads are "
					"disabled on your client. "
					"(cl_allowDownload is %d)",
					cl_allowDownload->integer);
				return;	
			}
			else {
				CL_InitDownload( localName );
				CL_BeginDownload( remoteName );
			}
		}
		clc.downloadRestart = true;

		// move over the rest
		memmove( clc.downloadList, s, strlen(s) + 1);

		return;
	}

	CL_DownloadsComplete();
}

/*
=================
CL_InitDownloads

After receiving a valid game state, we valid the cgame and local zip files here
and determine if we need to download them
=================
*/
void CL_InitDownloads() {
  char missingfiles[1024];

  if ( !(cl_allowDownload->integer & DLF_ENABLE) )
  {
    // autodownload is disabled on the client
    // but it's possible that some referenced files on the server are missing
    if (FS_ComparePaks( missingfiles, sizeof( missingfiles ), false ) )
    {      
      // NOTE TTimo I would rather have that printed as a modal message box
      //   but at this point while joining the game we don't know wether we will successfully join or not
      Com_Printf( "\nWARNING: You are missing some files referenced by the server:\n%s"
                  "You might not be able to join the game\n"
                  "Go to the setting menu to turn on autodownload, or get the file elsewhere\n\n", missingfiles );
    }
  }
  else if ( FS_ComparePaks( clc.downloadList, sizeof( clc.downloadList ) , true ) ) {

    Com_Printf("Need paks: %s\n", clc.downloadList );

		if ( *clc.downloadList ) {
			// if autodownloading is not enabled on the server
			clc.state = CA_CONNECTED;

			*clc.downloadTempName = *clc.downloadName = 0;
			CvarSystem::set( "cl_downloadName", "" );

			CL_NextDownload();
			return;
		}

	}
		
	CL_DownloadsComplete();
}

/*
=================
CL_CheckForResend

Resend a connect message if the last one has timed out
=================
*/
void CL_CheckForResend( ) {
	int		port;
	char	info[MAX_INFO_STRING];
	char	data[MAX_INFO_STRING + 10];

	// don't send anything if playing back a demo
	if ( clc.demoplaying ) {
		return;
	}

	// resend if we haven't gotten a reply yet
	if ( clc.state != CA_CONNECTING && clc.state != CA_CHALLENGING ) {
		return;
	}

	if ( cls.realtime - clc.connectTime < RETRANSMIT_TIMEOUT ) {
		return;
	}

	clc.connectTime = cls.realtime;	// for retransmit requests
	clc.connectPacketCount++;


	switch ( clc.state ) {
	case CA_CONNECTING:
		// requesting a challenge .. IPv6 users always get in as authorize server supports no ipv6.
#ifndef STANDALONE
		if (!com_standalone->integer && clc.serverAddress.type == NA_IP && !Sys_IsLANAddress( clc.serverAddress ) )
			CL_RequestAuthorization();
#endif

		// The challenge request shall be followed by a client challenge so no malicious server can hijack this connection.
		// Add the gamename so the server knows we're running the correct game or can reject the client
		// with a meaningful message
		Com_sprintf(data, sizeof(data), "getchallenge %d %s", clc.challenge, com_gamename->string);

		Network::outOfBandPrint(NS_CLIENT, clc.serverAddress, "%s", data);
		break;
		
	case CA_CHALLENGING:
		// sending back the challenge
		port = CvarSystem::variableValue ("net_qport");

		Q_strncpyz( info, CvarSystem::infoString( CVAR_USERINFO ), sizeof( info ) );
		
#ifdef LEGACY_PROTOCOL
		if(com_legacyprotocol->integer == com_protocol->integer)
			clc.compat = true;

		if(clc.compat)
			Info_SetValueForKey(info, "protocol", va("%i", com_legacyprotocol->integer));
		else
#endif
			Info_SetValueForKey(info, "protocol", va("%i", com_protocol->integer));
		Info_SetValueForKey( info, "qport", va("%i", port ) );
		Info_SetValueForKey( info, "challenge", va("%i", clc.challenge ) );
		
		Com_sprintf( data, sizeof(data), "connect \"%s\"", info );
		Network::outOfBandData( NS_CLIENT, clc.serverAddress, reinterpret_cast<byte *>(data), strlen ( data ) );
		// the most current userinfo has been sent, so watch for any
		// newer changes to userinfo variables
		cvar_modifiedFlags &= ~CVAR_USERINFO;
		break;

	default:
		Com_Error( ERR_FATAL, "CL_CheckForResend: bad clc.state" );
	}
}


/*
===================
CL_MotdPacket

===================
*/
void CL_MotdPacket( netadr_t from ) {
#ifdef UPDATE_SERVER_NAME
	char	*challenge;
	char	*info;

	// if not from our server, ignore it
	if ( !Network::compareAdr( from, cls.updateServer ) ) {
		return;
	}

	info = CommandSystem::argv(1);

	// check challenge
	challenge = Info_ValueForKey( info, "challenge" );
	if ( strcmp( challenge, cls.updateChallenge ) ) {
		return;
	}

	challenge = Info_ValueForKey( info, "motd" );

	Q_strncpyz( cls.updateInfoString, info, sizeof( cls.updateInfoString ) );
	CvarSystem::set( "cl_motdString", challenge );
#endif
}

/*
===================
CL_InitServerInfo
===================
*/
void CL_InitServerInfo( serverInfo_t *server, netadr_t *address ) {
	server->adr = *address;
	server->clients = 0;
	server->hostName[0] = '\0';
	server->mapName[0] = '\0';
	server->maxClients = 0;
	server->maxPing = 0;
	server->minPing = 0;
	server->ping = -1;
	server->game[0] = '\0';
	server->gameType = 0;
	server->netType = 0;
	server->punkbuster = 0;
	server->g_humanplayers = 0;
	server->g_needpass = 0;
}

#define MAX_SERVERSPERPACKET	256

/*
===================
CL_ServersResponsePacket
===================
*/
void CL_ServersResponsePacket( const netadr_t* from, Message *msg, bool extended ) {
	int				i, j, count, total;
	netadr_t addresses[MAX_SERVERSPERPACKET];
	int				numservers;
	byte*			buffptr;
	byte*			buffend;
	
	Com_Printf("CL_ServersResponsePacket from %s\n", Network::adrToStringwPort(*from));

	if (cls.numglobalservers == -1) {
		// state to detect lack of servers or lack of response
		cls.numglobalservers = 0;
		cls.numGlobalServerAddresses = 0;
	}

	// parse through server response string
	numservers = 0;
	buffptr    = msg->data;
	buffend    = buffptr + msg->cursize;

	// advance to initial token
	do
	{
		if(*buffptr == '\\' || (extended && *buffptr == '/'))
			break;
		
		buffptr++;
	} while (buffptr < buffend);

	while (buffptr + 1 < buffend)
	{
		// IPv4 address
		if (*buffptr == '\\')
		{
			buffptr++;

			if (buffend - buffptr < sizeof(addresses[numservers].ip) + sizeof(addresses[numservers].port) + 1)
				break;

			for(i = 0; i < sizeof(addresses[numservers].ip); i++)
				addresses[numservers].ip[i] = *buffptr++;

			addresses[numservers].type = NA_IP;
		}
		// IPv6 address, if it's an extended response
		else if (extended && *buffptr == '/')
		{
			buffptr++;

			if (buffend - buffptr < sizeof(addresses[numservers].ip6) + sizeof(addresses[numservers].port) + 1)
				break;
			
			for(i = 0; i < sizeof(addresses[numservers].ip6); i++)
				addresses[numservers].ip6[i] = *buffptr++;
			
			addresses[numservers].type = NA_IP6;
			addresses[numservers].scope_id = from->scope_id;
		}
		else
			// syntax error!
			break;
			
		// parse out port
		addresses[numservers].port = (*buffptr++) << 8;
		addresses[numservers].port += *buffptr++;
		addresses[numservers].port = BigShort( addresses[numservers].port );

		// syntax check
		if (*buffptr != '\\' && *buffptr != '/')
			break;
	
		numservers++;
		if (numservers >= MAX_SERVERSPERPACKET)
			break;
	}

	count = cls.numglobalservers;

	for (i = 0; i < numservers && count < MAX_GLOBAL_SERVERS; i++) {
		// build net address
		serverInfo_t *server = &cls.globalServers[count];

		// Tequila: It's possible to have sent many master server requests. Then
		// we may receive many times the same addresses from the master server.
		// We just avoid to add a server if it is still in the global servers list.
		for (j = 0; j < count; j++)
		{
			if (Network::compareAdr(cls.globalServers[j].adr, addresses[i]))
				break;
		}

		if (j < count)
			continue;

		CL_InitServerInfo( server, &addresses[i] );
		// advance to next slot
		count++;
	}

	// if getting the global list
	if ( count >= MAX_GLOBAL_SERVERS && cls.numGlobalServerAddresses < MAX_GLOBAL_SERVERS )
	{
		// if we couldn't store the servers in the main list anymore
		for (; i < numservers && cls.numGlobalServerAddresses < MAX_GLOBAL_SERVERS; i++)
		{
			// just store the addresses in an additional list
			cls.globalServerAddresses[cls.numGlobalServerAddresses++] = addresses[i];
		}
	}

	cls.numglobalservers = count;
	total = count + cls.numGlobalServerAddresses;

	Com_Printf("%d servers parsed (total %d)\n", numservers, total);
}

/*
=================
CL_ConnectionlessPacket

Responses to broadcasts, etc
=================
*/
void CL_ConnectionlessPacket( netadr_t from, Message *msg ) {
	char	*s;
	char	*c;
	int challenge = 0;

	msg->beginReadingOOB();
	msg->readLong();	// skip the -1

	s = msg->readStringLine();

	CommandSystem::tokenizeString( s );

	c = CommandSystem::argv(0);

	Com_DPrintf ("CL packet %s: %s\n", Network::adrToStringwPort(from), c);

	// challenge from the server we are connecting to
	if (!Q_stricmp(c, "challengeResponse"))
	{
		char *strver;
		int ver;
	
		if (clc.state != CA_CONNECTING)
		{
			Com_DPrintf("Unwanted challenge response received. Ignored.\n");
			return;
		}
		
		c = CommandSystem::argv(2);
		if(*c)
			challenge = atoi(c);

		strver = CommandSystem::argv(3);
		if(*strver)
		{
			ver = atoi(strver);
			
			if(ver != com_protocol->integer)
			{
#ifdef LEGACY_PROTOCOL
				if(com_legacyprotocol->integer > 0)
				{
					// Server is ioq3 but has a different protocol than we do.
					// Fall back to idq3 protocol.
					clc.compat = true;

					Com_Printf(S_COLOR_YELLOW "Warning: Server reports protocol version %d, "
						   "we have %d. Trying legacy protocol %d.\n",
						   ver, com_protocol->integer, com_legacyprotocol->integer);
				}
				else
#endif
				{
					Com_Printf(S_COLOR_YELLOW "Warning: Server reports protocol version %d, we have %d. "
						   "Trying anyways.\n", ver, com_protocol->integer);
				}
			}
		}
#ifdef LEGACY_PROTOCOL
		else
			clc.compat = true;
		
		if(clc.compat)
		{
			if(!Network::compareAdr(from, clc.serverAddress))
			{
				// This challenge response is not coming from the expected address.
				// Check whether we have a matching client challenge to prevent
				// connection hi-jacking.
			
				if(!*c || challenge != clc.challenge)
				{
					Com_DPrintf("Challenge response received from unexpected source. Ignored.\n");
					return;
				}
			}
		}
		else
#endif
		{
			if(!*c || challenge != clc.challenge)
			{
				Com_Printf("Bad challenge for challengeResponse. Ignored.\n");
				return;
			}
		}

		// start sending challenge response instead of challenge request packets
		clc.challenge = atoi(CommandSystem::argv(1));
		clc.state = CA_CHALLENGING;
		clc.connectPacketCount = 0;
		clc.connectTime = -99999;

		// take this address as the new server address.  This allows
		// a server proxy to hand off connections to multiple servers
		clc.serverAddress = from;
		Com_DPrintf ("challengeResponse: %d\n", clc.challenge);
		return;
	}

	// server connection
	if ( !Q_stricmp(c, "connectResponse") ) {
		if ( clc.state >= CA_CONNECTED ) {
			Com_Printf ("Dup connect received. Ignored.\n");
			return;
		}
		if ( clc.state != CA_CHALLENGING ) {
			Com_Printf ("connectResponse packet while not connecting. Ignored.\n");
			return;
		}
		if ( !Network::compareAdr( from, clc.serverAddress ) ) {
			Com_Printf( "connectResponse from wrong address. Ignored.\n" );
			return;
		}

#ifdef LEGACY_PROTOCOL
		if(!clc.compat)
#endif
		{
			c = CommandSystem::argv(1);

			if(*c)
				challenge = atoi(c);
			else
			{
				Com_Printf("Bad connectResponse received. Ignored.\n");
				return;
			}
			
			if(challenge != clc.challenge)
			{
				Com_Printf("ConnectResponse with bad challenge received. Ignored.\n");
				return;
			}
		}

#ifdef LEGACY_PROTOCOL
		clc.netchan.setup( NS_CLIENT, from, CvarSystem::variableValue("net_qport"), clc.challenge, clc.compat );
#else
		clc.netchan.setup( NS_CLIENT, from, CvarSystem::variableValue("net_qport"), clc.challenge, false );
#endif

		clc.state = CA_CONNECTED;
		clc.lastPacketSentTime = -9999;		// send first packet immediately
		return;
	}

	// server responding to an info broadcast
	if ( !Q_stricmp(c, "infoResponse") ) {
		CL_ServerInfoPacket( from, msg );
		return;
	}

	// server responding to a get playerlist
	if ( !Q_stricmp(c, "statusResponse") ) {
		CL_ServerStatusResponse( from, msg );
		return;
	}

	// echo request from server
	if ( !Q_stricmp(c, "echo") ) {
		// NOTE: we may have to add exceptions for auth and update servers
		if ( Network::compareAdr( from, clc.serverAddress ) || Network::compareAdr( from, cls.rconAddress ) ) {
			Network::outOfBandPrint( NS_CLIENT, from, "%s", CommandSystem::argv(1) );
		}
		return;
	}

	// cd check
	if ( !Q_stricmp(c, "keyAuthorize") ) {
		// we don't use these now, so dump them on the floor
		return;
	}

	// global MOTD from id
	if ( !Q_stricmp(c, "motd") ) {
		CL_MotdPacket( from );
		return;
	}

	// echo request from server
	if ( !Q_stricmp(c, "print") ) {
		// NOTE: we may have to add exceptions for auth and update servers
		if ( Network::compareAdr( from, clc.serverAddress ) || Network::compareAdr( from, cls.rconAddress ) ) {
			s = msg->readString();

			Q_strncpyz( clc.serverMessage, s, sizeof( clc.serverMessage ) );
			Com_Printf( "%s", s );
		}
		return;
	}

	// list of servers sent back by a master server (classic)
	if ( !Q_strncmp(c, "getserversResponse", 18) ) {
		CL_ServersResponsePacket( &from, msg, false );
		return;
	}

	// list of servers sent back by a master server (extended)
	if ( !Q_strncmp(c, "getserversExtResponse", 21) ) {
		CL_ServersResponsePacket( &from, msg, true );
		return;
	}

	Com_DPrintf ("Unknown connectionless packet command.\n");
}


/*
=================
CL_PacketEvent

A packet has arrived from the main event loop
=================
*/
void CL_PacketEvent( netadr_t from, Message *msg ) {
	int		headerBytes;

	clc.lastPacketTime = cls.realtime;

	if ( msg->cursize >= 4 && *reinterpret_cast<int *>(msg->data) == -1 ) {
		CL_ConnectionlessPacket( from, msg );
		return;
	}

	if ( clc.state < CA_CONNECTED ) {
		return;		// can't be a valid sequenced packet
	}

	if ( msg->cursize < 4 ) {
		Com_Printf ("%s: Runt packet\n", Network::adrToStringwPort( from ));
		return;
	}

	//
	// packet from server
	//
	if ( !Network::compareAdr( from, clc.netchan.remoteAddress ) ) {
		Com_DPrintf ("%s:sequenced packet without connection\n"
			, Network::adrToStringwPort( from ) );
		// FIXME: send a client disconnect?
		return;
	}

	if (!CL_Netchan_Process( &clc.netchan, msg) ) {
		return;		// out of order, duplicated, etc
	}

	// the header is different lengths for reliable and unreliable messages
	headerBytes = msg->readcount;

	// track the last message received so it can be returned in 
	// client messages, allowing the server to detect a dropped
	// gamestate
	clc.serverMessageSequence = LittleLong( *reinterpret_cast<int *>(msg->data) );

	clc.lastPacketTime = cls.realtime;
	CL_ParseServerMessage( msg );

	//
	// we don't know if it is ok to save a demo message until
	// after we have parsed the frame
	//
	if ( clc.demorecording && !clc.demowaiting ) {
		CL_WriteDemoMessage( msg, headerBytes );
	}
}

/*
==================
CL_CheckTimeout

==================
*/
void CL_CheckTimeout( ) {
	//
	// check timeout
	//
	if ( ( !CL_CheckPaused() || !sv_paused->integer ) 
		&& clc.state >= CA_CONNECTED && clc.state != CA_CINEMATIC
	    && cls.realtime - clc.lastPacketTime > cl_timeout->value*1000) {
		if (++cl.timeoutcount > 5) {	// timeoutcount saves debugger
			Com_Printf ("\nServer connection timed out.\n");
			CL_Disconnect( true );
			return;
		}
	} else {
		cl.timeoutcount = 0;
	}
}

/*
==================
CL_CheckPaused
Check whether client has been paused.
==================
*/
bool CL_CheckPaused()
{
	// if cl_paused->modified is set, the cvar has only been changed in
	// this frame. Keep paused in this frame to ensure the server doesn't
	// lag behind.
	if(cl_paused->integer || cl_paused->modified)
		return true;
	
	return false;
}

//============================================================================

/*
==================
CL_CheckUserinfo

==================
*/
void CL_CheckUserinfo( ) {
	// don't add reliable commands when not yet connected
	if(clc.state < CA_CONNECTED)
		return;

	// don't overflow the reliable command buffer when paused
	if(CL_CheckPaused())
		return;

	// send a reliable userinfo update if needed
	if(cvar_modifiedFlags & CVAR_USERINFO)
	{
		cvar_modifiedFlags &= ~CVAR_USERINFO;
		CL_AddReliableCommand(va("userinfo \"%s\"", CvarSystem::infoString( CVAR_USERINFO ) ), false);
	}
}

/*
==================
CL_Frame

==================
*/
void CL_Frame ( int msec ) {

	if ( !com_cl_running->integer ) {
		return;
	}

#ifdef USE_HTTP
	if(clc.httpUsed) {
		bool finished = CL_HTTP_PerformDownload();

		if(finished) {
			if(clc.download) {
				FS_FCloseFile(clc.download);
				clc.download = 0;
			}

			FS_BaseDir_Rename_HomeData(clc.downloadTempName, clc.downloadName, false);
			clc.downloadRestart = true;
			CL_NextDownload();
		}

		// we can't process frames normally when in disconnected
		// download mode since the ui vm expects clc.state to be
		// CA_CONNECTED
		if(clc.disconnectedForHttpDownload) {
			cls.realFrametime = msec;
			cls.frametime = msec;
			cls.realtime += cls.frametime;
			SCR_UpdateScreen();
			S_Update();
			Con_RunConsole();
			cls.framecount++;
			return;
		}
	}
#endif

	if ( cls.cddialog ) {
		// bring up the cd error dialog if needed
		cls.cddialog = false;
		theUI.setActiveMenu( UserInterface::MenuCommand::UIMENU_NEED_CD );
	} else	if ( clc.state == CA_DISCONNECTED && !( Key_GetCatcher( ) & KEYCATCH_UI )
		&& !com_sv_running->integer && theUI.isInitialized() ) {
		// if disconnected, bring up the menu
		S_StopAllSounds();
		theUI.setActiveMenu( UserInterface::MenuCommand::UIMENU_MAIN );
	}

	// if recording an avi, lock to a fixed fps
	if ( CL_VideoRecording( ) && cl_aviFrameRate->integer && msec) {
		// save the current screen
		if ( clc.state == CA_ACTIVE || cl_forceavidemo->integer) {
			float fps = MIN(cl_aviFrameRate->value * com_timescale->value, 1000.0f);
			float frameDuration = MAX(1000.0f / fps, 1.0f) + clc.aviVideoFrameRemainder;

			CL_TakeVideoFrame( );

			msec = static_cast<int>(frameDuration);
			clc.aviVideoFrameRemainder = frameDuration - msec;
		}
	}
	
	if( cl_autoRecordDemo->integer ) {
		if( clc.state == CA_ACTIVE && !clc.demorecording && !clc.demoplaying ) {
			// If not recording a demo, and we should be, start one
			qtime_t	now;
			char		*nowString;
			char		*p;
			char		mapName[ MAX_QPATH ];
			char		serverName[ MAX_OSPATH ];

			Com_RealTime( &now );
			nowString = va( "%04d%02d%02d%02d%02d%02d",
					1900 + now.tm_year,
					1 + now.tm_mon,
					now.tm_mday,
					now.tm_hour,
					now.tm_min,
					now.tm_sec );

			Q_strncpyz( serverName, clc.servername, MAX_OSPATH );
			// Replace the ":" in the address as it is not a valid
			// file name character
			p = strstr( serverName, ":" );
			if( p ) {
				*p = '.';
			}

			Q_strncpyz( mapName, COM_SkipPath( cl.mapname ), sizeof( cl.mapname ) );
			COM_StripExtension(mapName, mapName, sizeof(mapName));

			CommandBuffer::executeText( EXEC_NOW,
					va( "record %s-%s-%s", nowString, serverName, mapName ) );
		}
		else if( clc.state != CA_ACTIVE && clc.demorecording ) {
			// Recording, but not CA_ACTIVE, so stop recording
			CL_StopRecord_f( );
		}
	}

	// save the msec before checking pause
	cls.realFrametime = msec;

	// decide the simulation time
	cls.frametime = msec;

	cls.realtime += cls.frametime;

	if ( cl_timegraph->integer ) {
		SCR_DebugGraph ( cls.realFrametime * 0.25 );
	}

	// see if we need to update any userinfo
	CL_CheckUserinfo();

	// if we haven't gotten a packet in a long time,
	// drop the connection
	CL_CheckTimeout();

	// send intentions now
	CL_SendCmd();

	// resend a connection request if necessary
	CL_CheckForResend();

	// decide on the serverTime to render
	CL_SetCGameTime();

	// update the screen
	SCR_UpdateScreen();

	// update audio
	S_Update();

#ifdef USE_VOIP
	CL_CaptureVoip();
#endif

#ifdef USE_MUMBLE
	CL_UpdateMumble();
#endif

	// advance local effects for next frame
	SCR_RunCinematic();

	Con_RunConsole();

	cls.framecount++;
}


//============================================================================

/*
================
CL_RefPrintf

DLL glue
================
*/
static Q_PRINTF_FUNC(2, 3) void QDECL CL_RefPrintf( int print_level, const char *fmt, ...) {
	va_list		argptr;
	char		msg[MAXPRINTMSG];
	
	va_start (argptr,fmt);
	Q_vsnprintf (msg, sizeof(msg), fmt, argptr);
	va_end (argptr);

	if ( print_level == PRINT_ALL ) {
		Com_Printf ("%s", msg);
	} else if ( print_level == PRINT_WARNING ) {
		Com_Printf (S_COLOR_YELLOW "%s", msg);		// yellow
	} else if ( print_level == PRINT_ERROR ) {
		Com_Printf (S_COLOR_RED "%s", msg);			// red
	} else if ( print_level == PRINT_DEVELOPER ) {
		Com_DPrintf (S_COLOR_RED "%s", msg);		// red - developer only
	}
}



/*
============
CL_ShutdownRef
============
*/
void CL_ShutdownRef( ) {
	if ( refExport.Shutdown ) {
		refExport.Shutdown( true );
	}

	Com_Memset( &refExport, 0, sizeof( refExport ) );

#ifdef USE_RENDERER_DLOPEN
	if ( rendererLib ) {
		Sys_UnloadLibrary( rendererLib );
		rendererLib = nullptr;
	}
#endif
}

/*
============
CL_InitRenderer
============
*/
void CL_InitRenderer( ) {
	// this sets up the renderer and calls R_Init
	refExport.BeginRegistration( &cls.glconfig );

	// load character sets
	cls.charSetShader = refExport.RegisterShader( "gfx/2d/bigchars" );
	cls.whiteShader = refExport.RegisterShader( "white" );
	cls.consoleShader = refExport.RegisterShader( "console" );
	g_console_field_width = cls.glconfig.vidWidth / g_smallchar_width - 2;
	g_consoleField.widthInChars = g_console_field_width;
}

/*
============================
CL_StartHunkUsers

After the server has cleared the hunk, these will need to be restarted
This is the only place that any of these functions are called from
============================
*/
void CL_StartHunkUsers( bool rendererOnly ) {
	if (!com_cl_running) {
		return;
	}

	if ( !com_cl_running->integer ) {
		return;
	}

	if ( !cls.rendererStarted ) {
		cls.rendererStarted = true;
		CL_InitRenderer();
	}

	if ( rendererOnly ) {
		return;
	}

	if ( !cls.soundStarted ) {
		cls.soundStarted = true;
		S_Init();
	}

	if ( !cls.soundRegistered ) {
		cls.soundRegistered = true;
		S_BeginRegistration();
	}

	if( com_dedicated->integer ) {
		return;
	}

	if ( !cls.uiStarted ) {
		cls.uiStarted = true;
		CL_InitUI();
	}
}

/*
============
CL_RefMalloc
============
*/
void *CL_RefMalloc( int size ) {
	return Z_TagMalloc( size, TAG_RENDERER );
}

int CL_ScaledMilliseconds() {
	return Sys_Milliseconds()*com_timescale->value;
}

/*
============
CL_InitRef
============
*/
void CL_InitRef( ) {
	refimport_t	ri;
	refexport_t	*ret;
#ifdef USE_RENDERER_DLOPEN
	GetRefAPI_t		GetRefAPI;
	char			dllName[MAX_OSPATH];
#endif

	Com_Printf( "----- Initializing Renderer ----\n" );

#ifdef USE_RENDERER_DLOPEN
	cl_renderer = CvarSystem::get("cl_renderer", "opengl2", CVAR_ARCHIVE | CVAR_LATCH);

	Com_sprintf(dllName, sizeof(dllName), "renderer_%s" DLL_EXT, cl_renderer->string);

	if(!(rendererLib = Sys_LoadDll(dllName, false)) && strcmp(cl_renderer->string, cl_renderer->resetString))
	{
		Com_Printf("failed:\n\"%s\"\n", Sys_LibraryError());
		CvarSystem::forceReset("cl_renderer");

		Com_sprintf(dllName, sizeof(dllName), "renderer_%s" DLL_EXT, cl_renderer->resetString);
		rendererLib = Sys_LoadDll(dllName, false);
	}

	if(!rendererLib)
	{
		Com_Printf("failed:\n\"%s\"\n", Sys_LibraryError());
		Com_Error(ERR_FATAL, "Failed to load renderer");
	}

	GetRefAPI = reinterpret_cast<GetRefAPI_t>(Sys_LoadFunction(rendererLib, "GetRefAPI"));
	if(!GetRefAPI)
	{
		Com_Error(ERR_FATAL, "Can't load symbol GetRefAPI: '%s'",  Sys_LibraryError());
	}
#endif

	ri.Cmd_AddCommand = CommandSystem::addCommand;
	ri.Cmd_RemoveCommand = CommandSystem::removeCommand;
	ri.Cmd_Argc = CommandSystem::argc;
	ri.Cmd_Argv = CommandSystem::argv;
	ri.Cmd_ExecuteText = CommandBuffer::executeText;
	ri.Printf = CL_RefPrintf;
	ri.Error = Com_Error;
	ri.Milliseconds = CL_ScaledMilliseconds;
	ri.Malloc = CL_RefMalloc;
	ri.Free = Z_Free;
#ifdef HUNK_DEBUG
	ri.Hunk_AllocDebug = Hunk_AllocDebug;
#else
	ri.Hunk_Alloc = Hunk_Alloc;
#endif
	ri.Hunk_AllocateTempMemory = Hunk_AllocateTempMemory;
	ri.Hunk_FreeTempMemory = Hunk_FreeTempMemory;

	ri.CM_ClusterPVS = CM_ClusterPVS;
	ri.CM_DrawDebugSurface = CM_DrawDebugSurface;

	ri.FS_ReadFile = FS_ReadFile;
	ri.FS_FreeFile = FS_FreeFile;
	ri.FS_WriteFile = FS_WriteFile;
	ri.FS_FreeFileList = FS_FreeFileList;
	ri.FS_ListFiles = FS_ListFiles;
	ri.FS_FileIsInPAK = FS_FileIsInPAK;
	ri.FS_FileExists = FS_FileExists_HomeData;
	ri.Cvar_Get = CvarSystem::get;
	ri.Cvar_Set = CvarSystem::set;
	ri.Cvar_SetValue = CvarSystem::setValue;
	ri.Cvar_CheckRange = CvarSystem::checkRange;
	ri.Cvar_SetDescription = CvarSystem::setDescription;
	ri.Cvar_VariableIntegerValue = CvarSystem::variableIntegerValue;

	// cinematic stuff

	ri.CIN_UploadCinematic = CIN_UploadCinematic;
	ri.CIN_PlayCinematic = CIN_PlayCinematic;
	ri.CIN_RunCinematic = CIN_RunCinematic;
  
	ri.CL_WriteAVIVideoFrame = CL_WriteAVIVideoFrame;

	ri.IN_Init = IN_Init;
	ri.IN_Shutdown = IN_Shutdown;
	ri.IN_Restart = IN_Restart;

	ri.ftol = Q_ftol;

	ri.Sys_SetEnv = Sys_SetEnv;
	ri.Sys_GLimpSafeInit = Sys_GLimpSafeInit;
	ri.Sys_GLimpInit = Sys_GLimpInit;
	ri.Sys_LowPhysicalMemory = Sys_LowPhysicalMemory;

	ret = GetRefAPI( REF_API_VERSION, &ri );

#if defined __USEA3D && defined __A3D_GEOM
	hA3Dg_ExportRenderGeom (ret);
#endif

	Com_Printf( "-------------------------------\n");

	if ( !ret ) {
		Com_Error (ERR_FATAL, "Couldn't initialize refresh" );
	}

	refExport = *ret;

	// unpause so the cgame definitely gets a snapshot and renders a frame
	CvarSystem::set( "cl_paused", "0" );
}


//===========================================================================================


void CL_SetModel_f( ) {
	char	*arg;
	char	name[256];

	arg = CommandSystem::argv( 1 );
	if (arg[0]) {
		CvarSystem::set( "model", arg );
		CvarSystem::set( "headmodel", arg );
	} else {
		CvarSystem::variableStringBuffer( "model", name, sizeof(name) );
		Com_Printf("model is set to %s\n", name);
	}
}


//===========================================================================================


/*
===============
CL_Video_f

video
video [filename]
===============
*/
void CL_Video_f( )
{
  char  filename[ MAX_OSPATH ];
  int   i, last;

  if( !clc.demoplaying )
  {
    Com_Printf( "The video command can only be used when playing back demos\n" );
    return;
  }

  if( CommandSystem::argc( ) == 2 )
  {
    // explicit filename
    Com_sprintf( filename, MAX_OSPATH, "videos/%s.avi", CommandSystem::argv( 1 ) );
  }
  else
  {
    // scan for a free filename
    for( i = 0; i <= 9999; i++ )
    {
      int a, b, c, d;

      last = i;

      a = last / 1000;
      last -= a * 1000;
      b = last / 100;
      last -= b * 100;
      c = last / 10;
      last -= c * 10;
      d = last;

      Com_sprintf( filename, MAX_OSPATH, "videos/video%d%d%d%d.avi",
          a, b, c, d );

      if( !FS_FileExists_HomeData( filename ) )
        break; // file doesn't exist
    }

    if( i > 9999 )
    {
      Com_Printf( S_COLOR_RED "ERROR: no free file names to create video\n" );
      return;
    }
  }

  CL_OpenAVIForWriting( filename );
}

/*
===============
CL_StopVideo_f
===============
*/
void CL_StopVideo_f( )
{
  CL_CloseAVI( );
}

/*
===============
CL_GenerateQKey

test to see if a valid QKEY_FILE exists.  If one does not, try to generate
it by filling it with 2048 bytes of random data.
===============
*/
static void CL_GenerateQKey()
{
	int len = 0;
	unsigned char buff[ QKEY_SIZE ];
	fileHandle_t f;

	len = FS_BaseDir_FOpenFileRead( QKEY_FILE, &f );
	FS_FCloseFile( f );
	if( len == QKEY_SIZE ) {
		Com_Printf( "QKEY found.\n" );
		return;
	}
	else {
		if( len > 0 ) {
			Com_Printf( "QKEY file size != %d, regenerating\n",
				QKEY_SIZE );
		}

		Com_Printf( "QKEY building random string\n" );
		Com_RandomBytes( buff, sizeof(buff) );

		f = FS_BaseDir_FOpenFileWrite_HomeState( QKEY_FILE );
		if( !f ) {
			Com_Printf( "QKEY could not open %s for write\n",
				QKEY_FILE );
			return;
		}
		FS_Write( buff, sizeof(buff), f );
		FS_FCloseFile( f );
		Com_Printf( "QKEY generated\n" );
	}
} 

void CL_Sayto_f( ) {
	char		*rawname;
	char		name[MAX_NAME_LENGTH];
	char		cleanName[MAX_NAME_LENGTH];
	const char	*info;
	int			count;
	int			i;
	int			clientNum;
	char		*p;

	if ( CommandSystem::argc() < 3 ) {
		Com_Printf ("sayto <player name> <text>\n");
		return;
	}

	rawname = CommandSystem::argv(1);

	Com_FieldStringToPlayerName( name, MAX_NAME_LENGTH, rawname );

	info = cl.gameState.stringData + cl.gameState.stringOffsets[CS_SERVERINFO];
	count = atoi( Info_ValueForKey( info, "sv_maxclients" ) );

	clientNum = -1;
	for( i = 0; i < count; i++ ) {

		info = cl.gameState.stringData + cl.gameState.stringOffsets[CS_PLAYERS+i];
		Q_strncpyz( cleanName, Info_ValueForKey( info, "n" ), sizeof(cleanName) );
		Q_CleanStr( cleanName );

		if ( !Q_stricmp( cleanName, name ) ) {
			clientNum = i;
			break;
		}
	}
	if( clientNum <= -1 )
	{
		Com_Printf ("No such player name: %s.\n", name);
		return;
	}

	p = CommandSystem::argsFrom(2);

	if ( *p == '"' ) {
		p++;
		p[strlen(p)-1] = 0;
	}

	CL_AddReliableCommand(va("tell %i \"%s\"", clientNum, p ), false);
}

/*
====================
CL_Init
====================
*/
void CL_Init( ) {
	Com_Printf( "----- Client Initialization -----\n" );

	Con_Init ();

	if(!com_fullyInitialized)
	{
		CL_ClearState();
		clc.state = CA_DISCONNECTED;	// no longer CA_UNINITIALIZED
		cl_oldGameSet = false;
	}

	cls.realtime = 0;

	CL_InitInput ();

	//
	// register our variables
	//
	cl_noprint = CvarSystem::get( "cl_noprint", "0", 0 );
#ifdef UPDATE_SERVER_NAME
	cl_motd = CvarSystem::get ("cl_motd", "1", 0);
#endif

	cl_timeout = CvarSystem::get ("cl_timeout", "200", 0);

	cl_timeNudge = CvarSystem::get ("cl_timeNudge", "0", CVAR_TEMP );
	cl_shownet = CvarSystem::get ("cl_shownet", "0", CVAR_TEMP );
	cl_showSend = CvarSystem::get ("cl_showSend", "0", CVAR_TEMP );
	cl_showTimeDelta = CvarSystem::get ("cl_showTimeDelta", "0", CVAR_TEMP );
	cl_freezeDemo = CvarSystem::get ("cl_freezeDemo", "0", CVAR_TEMP );
	rcon_client_password = CvarSystem::get ("rconPassword", "", CVAR_TEMP );
	cl_activeAction = CvarSystem::get( "activeAction", "", CVAR_TEMP );

	cl_timedemo = CvarSystem::get ("timedemo", "0", 0);
	cl_timedemoLog = CvarSystem::get ("cl_timedemoLog", "", CVAR_ARCHIVE);
	cl_autoRecordDemo = CvarSystem::get ("cl_autoRecordDemo", "0", CVAR_ARCHIVE);
	cl_aviFrameRate = CvarSystem::get ("cl_aviFrameRate", "25", CVAR_ARCHIVE);
	cl_aviMotionJpeg = CvarSystem::get ("cl_aviMotionJpeg", "1", CVAR_ARCHIVE);
	cl_forceavidemo = CvarSystem::get ("cl_forceavidemo", "0", 0);

	rconAddress = CvarSystem::get ("rconAddress", "", 0);

	cl_yawspeed = CvarSystem::get ("cl_yawspeed", "140", CVAR_ARCHIVE);
	cl_pitchspeed = CvarSystem::get ("cl_pitchspeed", "140", CVAR_ARCHIVE);
	cl_anglespeedkey = CvarSystem::get ("cl_anglespeedkey", "1.5", 0);

	cl_maxpackets = CvarSystem::get ("cl_maxpackets", "30", CVAR_ARCHIVE );
	cl_packetdup = CvarSystem::get ("cl_packetdup", "1", CVAR_ARCHIVE );

	cl_run = CvarSystem::get ("cl_run", "1", CVAR_ARCHIVE);
	cl_sensitivity = CvarSystem::get ("sensitivity", "5", CVAR_ARCHIVE);
	cl_mouseAccel = CvarSystem::get ("cl_mouseAccel", "0", CVAR_ARCHIVE);
	cl_freelook = CvarSystem::get( "cl_freelook", "1", CVAR_ARCHIVE );

	// 0: legacy mouse acceleration
	// 1: new implementation
	cl_mouseAccelStyle = CvarSystem::get( "cl_mouseAccelStyle", "0", CVAR_ARCHIVE );
	// offset for the power function (for style 1, ignored otherwise)
	// this should be set to the max rate value
	cl_mouseAccelOffset = CvarSystem::get( "cl_mouseAccelOffset", "5", CVAR_ARCHIVE );
	CvarSystem::checkRange(cl_mouseAccelOffset, 0.001f, 50000.0f, false);

	cl_showMouseRate = CvarSystem::get ("cl_showmouserate", "0", 0);

	cl_allowDownload = CvarSystem::get ("cl_allowDownload", "0", CVAR_ARCHIVE);

	cl_conXOffset = CvarSystem::get ("cl_conXOffset", "0", 0);
#ifdef __APPLE__
	// In game video is REALLY slow in Mac OS X right now due to driver slowness
	cl_inGameVideo = CvarSystem::get ("r_inGameVideo", "0", CVAR_ARCHIVE);
#else
	cl_inGameVideo = CvarSystem::get ("r_inGameVideo", "1", CVAR_ARCHIVE);
#endif

	cl_serverStatusResendTime = CvarSystem::get ("cl_serverStatusResendTime", "750", 0);

	// init autoswitch so the ui will have it correctly even
	// if the cgame hasn't been started
	CvarSystem::get ("cg_autoswitch", "1", CVAR_ARCHIVE);

	m_pitch = CvarSystem::get ("m_pitch", "0.022", CVAR_ARCHIVE);
	m_yaw = CvarSystem::get ("m_yaw", "0.022", CVAR_ARCHIVE);
	m_forward = CvarSystem::get ("m_forward", "0.25", CVAR_ARCHIVE);
	m_side = CvarSystem::get ("m_side", "0.25", CVAR_ARCHIVE);
#ifdef __APPLE__
	// Input is jittery on OS X w/o this
	m_filter = CvarSystem::get ("m_filter", "1", CVAR_ARCHIVE);
#else
	m_filter = CvarSystem::get ("m_filter", "0", CVAR_ARCHIVE);
#endif

	j_pitch =        CvarSystem::get ("j_pitch",        "0.022", CVAR_ARCHIVE);
	j_yaw =          CvarSystem::get ("j_yaw",          "-0.022", CVAR_ARCHIVE);
	j_forward =      CvarSystem::get ("j_forward",      "-0.25", CVAR_ARCHIVE);
	j_side =         CvarSystem::get ("j_side",         "0.25", CVAR_ARCHIVE);
	j_up =           CvarSystem::get ("j_up",           "0", CVAR_ARCHIVE);

	j_pitch_axis =   CvarSystem::get ("j_pitch_axis",   "3", CVAR_ARCHIVE);
	j_yaw_axis =     CvarSystem::get ("j_yaw_axis",     "2", CVAR_ARCHIVE);
	j_forward_axis = CvarSystem::get ("j_forward_axis", "1", CVAR_ARCHIVE);
	j_side_axis =    CvarSystem::get ("j_side_axis",    "0", CVAR_ARCHIVE);
	j_up_axis =      CvarSystem::get ("j_up_axis",      "4", CVAR_ARCHIVE);

	CvarSystem::checkRange(j_pitch_axis, 0, MAX_JOYSTICK_AXIS-1, true);
	CvarSystem::checkRange(j_yaw_axis, 0, MAX_JOYSTICK_AXIS-1, true);
	CvarSystem::checkRange(j_forward_axis, 0, MAX_JOYSTICK_AXIS-1, true);
	CvarSystem::checkRange(j_side_axis, 0, MAX_JOYSTICK_AXIS-1, true);
	CvarSystem::checkRange(j_up_axis, 0, MAX_JOYSTICK_AXIS-1, true);

	cl_motdString = CvarSystem::get( "cl_motdString", "", CVAR_ROM );

	CvarSystem::get( "cl_maxPing", "800", CVAR_ARCHIVE );

	cl_lanForcePackets = CvarSystem::get ("cl_lanForcePackets", "1", CVAR_ARCHIVE);

	cl_guidServerUniq = CvarSystem::get ("cl_guidServerUniq", "1", CVAR_ARCHIVE);

	// ~ and `, as keys and characters
	cl_consoleKeys = CvarSystem::get( "cl_consoleKeys", "~ ` 0x7e 0x60", CVAR_ARCHIVE);

	// userinfo
	CvarSystem::get ("name", "UnnamedPlayer", CVAR_USERINFO | CVAR_ARCHIVE );
	cl_rate = CvarSystem::get ("rate", "100000", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("snaps", "40", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("model", "sarge", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("headmodel", "sarge", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("team_model", "james", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("team_headmodel", "*james", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("g_redTeam", "Stroggs", CVAR_SERVERINFO | CVAR_ARCHIVE);
	CvarSystem::get ("g_blueTeam", "Pagans", CVAR_SERVERINFO | CVAR_ARCHIVE);
	CvarSystem::get ("color1",  "4", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("color2", "5", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("handicap", "100", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("teamtask", "0", CVAR_USERINFO );
	CvarSystem::get ("sex", "male", CVAR_USERINFO | CVAR_ARCHIVE );
	CvarSystem::get ("cl_anonymous", "0", CVAR_USERINFO | CVAR_ARCHIVE );

	CvarSystem::get ("password", "", CVAR_USERINFO);
	CvarSystem::get ("cg_predictItems", "1", CVAR_USERINFO | CVAR_ARCHIVE );

#ifdef USE_MUMBLE
	cl_useMumble = CvarSystem::get ("cl_useMumble", "0", CVAR_ARCHIVE | CVAR_LATCH);
	cl_mumbleScale = CvarSystem::get ("cl_mumbleScale", "0.0254", CVAR_ARCHIVE);
#endif

#ifdef USE_VOIP
	cl_voipSend = CvarSystem::get ("cl_voipSend", "0", 0);
	cl_voipSendTarget = CvarSystem::get ("cl_voipSendTarget", "spatial", 0);
	cl_voipGainDuringCapture = CvarSystem::get ("cl_voipGainDuringCapture", "0.2", CVAR_ARCHIVE);
	cl_voipCaptureMult = CvarSystem::get ("cl_voipCaptureMult", "2.0", CVAR_ARCHIVE);
	cl_voipUseVAD = CvarSystem::get ("cl_voipUseVAD", "0", CVAR_ARCHIVE);
	cl_voipVADThreshold = CvarSystem::get ("cl_voipVADThreshold", "0.25", CVAR_ARCHIVE);
	cl_voipShowMeter = CvarSystem::get ("cl_voipShowMeter", "1", CVAR_ARCHIVE);

	cl_voip = CvarSystem::get ("cl_voip", "1", CVAR_ARCHIVE);
	CvarSystem::checkRange( cl_voip, 0, 1, true );
	cl_voipProtocol = CvarSystem::get ("cl_voipProtocol", cl_voip->integer ? "opus" : "", CVAR_USERINFO | CVAR_ROM);
#endif

#ifdef USE_HTTP
	if(!CL_HTTP_Init()) {
		Com_Printf("WARNING: couldn't initialize HTTP download support\n");
	}
#endif

	// cgame might not be initialized before menu is used
	CvarSystem::get ("cg_viewsize", "100", CVAR_ARCHIVE );
	// Make sure cg_stereoSeparation is zero as that variable is deprecated and should not be used anymore.
	CvarSystem::get ("cg_stereoSeparation", "0", CVAR_ROM);

	//
	// register our commands
	//
	CommandSystem::addCommand ("cmd", CL_ForwardToServer_f);
	CommandSystem::addCommand ("configstrings", CL_Configstrings_f);
	CommandSystem::addCommand ("clientinfo", CL_Clientinfo_f);
	CommandSystem::addCommand ("snd_restart", CL_Snd_Restart_f);
	CommandSystem::addCommand ("vid_restart", CL_Vid_Restart_f);
	CommandSystem::addCommand ("disconnect", CL_Disconnect_f);
	CommandSystem::addCommand ("record", CL_Record_f);
	CommandSystem::addCommand ("demo", CL_PlayDemo_f);
	CommandSystem::setCommandCompletionFunc( "demo", CL_CompleteDemoName );
	CommandSystem::addCommand ("cinematic", CL_PlayCinematic_f);
	CommandSystem::addCommand ("stoprecord", CL_StopRecord_f);
	CommandSystem::addCommand ("connect", CL_Connect_f);
	CommandSystem::addCommand ("reconnect", CL_Reconnect_f);
	CommandSystem::addCommand ("localservers", CL_LocalServers_f);
	CommandSystem::addCommand ("globalservers", CL_GlobalServers_f);
	CommandSystem::addCommand ("rcon", CL_Rcon_f);
	CommandSystem::setCommandCompletionFunc( "rcon", CL_CompleteRcon );
	CommandSystem::addCommand ("ping", CL_Ping_f );
	CommandSystem::addCommand ("serverstatus", CL_ServerStatus_f );
	CommandSystem::addCommand ("showip", CL_ShowIP_f );
	CommandSystem::addCommand ("fs_openedList", CL_OpenedPK3List_f );
	CommandSystem::addCommand ("fs_referencedList", CL_ReferencedPK3List_f );
	CommandSystem::addCommand ("model", CL_SetModel_f );
	CommandSystem::addCommand ("video", CL_Video_f );
	CommandSystem::addCommand ("stopvideo", CL_StopVideo_f );
	if( !com_dedicated->integer ) {
		CommandSystem::addCommand ("sayto", CL_Sayto_f );
		CommandSystem::setCommandCompletionFunc( "sayto", CL_CompletePlayerName );
	}
	CL_InitRef();

	SCR_Init ();

//	CommandBuffer::execute ();

	CvarSystem::set( "cl_running", "1" );

	CL_GenerateQKey();
	CvarSystem::get( "cl_guid", "", CVAR_USERINFO | CVAR_ROM );
	CL_UpdateGUID( nullptr, 0 );

	Com_Printf( "----- Client Initialization Complete -----\n" );
}


/*
===============
CL_Shutdown

===============
*/
void CL_Shutdown(const char *finalmsg, bool disconnect, bool quit)
{
	static bool recursive = false;
	
	// check whether the client is running at all.
	if(!(com_cl_running && com_cl_running->integer))
		return;
	
	Com_Printf( "----- Client Shutdown (%s) -----\n", finalmsg );

	if ( recursive ) {
		Com_Printf( "WARNING: Recursive shutdown\n" );
		return;
	}
	recursive = true;

	noGameRestart = quit;

	if(disconnect)
		CL_Disconnect(true);
	
	CL_ClearMemory(true);
	CL_Snd_Shutdown();

#ifdef USE_HTTP
	CL_HTTP_Shutdown();
#endif

	CommandSystem::removeCommand ("cmd");
	CommandSystem::removeCommand ("configstrings");
	CommandSystem::removeCommand ("clientinfo");
	CommandSystem::removeCommand ("snd_restart");
	CommandSystem::removeCommand ("vid_restart");
	CommandSystem::removeCommand ("disconnect");
	CommandSystem::removeCommand ("record");
	CommandSystem::removeCommand ("demo");
	CommandSystem::removeCommand ("cinematic");
	CommandSystem::removeCommand ("stoprecord");
	CommandSystem::removeCommand ("connect");
	CommandSystem::removeCommand ("reconnect");
	CommandSystem::removeCommand ("localservers");
	CommandSystem::removeCommand ("globalservers");
	CommandSystem::removeCommand ("rcon");
	CommandSystem::removeCommand ("ping");
	CommandSystem::removeCommand ("serverstatus");
	CommandSystem::removeCommand ("showip");
	CommandSystem::removeCommand ("fs_openedList");
	CommandSystem::removeCommand ("fs_referencedList");
	CommandSystem::removeCommand ("model");
	CommandSystem::removeCommand ("video");
	CommandSystem::removeCommand ("stopvideo");

	CL_ShutdownInput();
	Con_Shutdown();

	CvarSystem::set( "cl_running", "0" );

	recursive = false;

	Com_Memset( &cls, 0, sizeof( cls ) );
	Key_SetCatcher( 0 );

	Com_Printf( "-----------------------\n" );

}

static void CL_SetServerInfo(serverInfo_t *server, const char *info, int ping) {
	if (server) {
		if (info) {
			server->clients = atoi(Info_ValueForKey(info, "clients"));
			Q_strncpyz(server->hostName,Info_ValueForKey(info, "hostname"), MAX_HOSTNAME_LENGTH);
			Q_strncpyz(server->mapName, Info_ValueForKey(info, "mapname"), MAX_NAME_LENGTH);
			server->maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
			Q_strncpyz(server->game,Info_ValueForKey(info, "game"), MAX_NAME_LENGTH);
			server->gameType = atoi(Info_ValueForKey(info, "gametype"));
			server->netType = atoi(Info_ValueForKey(info, "nettype"));
			server->minPing = atoi(Info_ValueForKey(info, "minping"));
			server->maxPing = atoi(Info_ValueForKey(info, "maxping"));
			server->punkbuster = atoi(Info_ValueForKey(info, "punkbuster"));
			server->g_humanplayers = atoi(Info_ValueForKey(info, "g_humanplayers"));
			server->g_needpass = atoi(Info_ValueForKey(info, "g_needpass"));
		}
		server->ping = ping;
	}
}

static void CL_SetServerInfoByAddress(netadr_t from, const char *info, int ping) {
	int i;

	for (i = 0; i < MAX_OTHER_SERVERS; i++) {
		if (Network::compareAdr(from, cls.localServers[i].adr)) {
			CL_SetServerInfo(&cls.localServers[i], info, ping);
		}
	}

	for (i = 0; i < MAX_GLOBAL_SERVERS; i++) {
		if (Network::compareAdr(from, cls.globalServers[i].adr)) {
			CL_SetServerInfo(&cls.globalServers[i], info, ping);
		}
	}

	for (i = 0; i < MAX_OTHER_SERVERS; i++) {
		if (Network::compareAdr(from, cls.favoriteServers[i].adr)) {
			CL_SetServerInfo(&cls.favoriteServers[i], info, ping);
		}
	}

}

/*
===================
CL_ServerInfoPacket
===================
*/
void CL_ServerInfoPacket( netadr_t from, Message *msg ) {
	int		i, type;
	char	info[MAX_INFO_STRING];
	char	*infoString;
	int		prot;
	char	*gamename;
	bool gameMismatch;

	infoString = msg->readString();

	// if this isn't the correct gamename, ignore it
	gamename = Info_ValueForKey( infoString, "gamename" );

#ifdef LEGACY_PROTOCOL
	// gamename is optional for legacy protocol
	if (com_legacyprotocol->integer && !*gamename)
		gameMismatch = false;
	else
#endif
		gameMismatch = !*gamename || strcmp(gamename, com_gamename->string) != 0;

	if (gameMismatch)
	{
		Com_DPrintf( "Game mismatch in info packet: %s\n", infoString );
		return;
	}

	// if this isn't the correct protocol version, ignore it
	prot = atoi( Info_ValueForKey( infoString, "protocol" ) );

	if(prot != com_protocol->integer
#ifdef LEGACY_PROTOCOL
	   && prot != com_legacyprotocol->integer
#endif
	  )
	{
		Com_DPrintf( "Different protocol info packet: %s\n", infoString );
		return;
	}

	// iterate servers waiting for ping response
	for (i=0; i<MAX_PINGREQUESTS; i++)
	{
		if ( cl_pinglist[i].adr.port && !cl_pinglist[i].time && Network::compareAdr( from, cl_pinglist[i].adr ) )
		{
			// calc ping time
			cl_pinglist[i].time = Sys_Milliseconds() - cl_pinglist[i].start;
			Com_DPrintf( "ping time %dms from %s\n", cl_pinglist[i].time, Network::adrToString( from ) );

			// save of info
			Q_strncpyz( cl_pinglist[i].info, infoString, sizeof( cl_pinglist[i].info ) );

			// tack on the net type
			// NOTE: make sure these types are in sync with the netnames strings in the UI
			switch (from.type)
			{
				case NA_BROADCAST:
				case NA_IP:
					type = 1;
					break;
				case NA_IP6:
					type = 2;
					break;
				default:
					type = 0;
					break;
			}
			Info_SetValueForKey( cl_pinglist[i].info, "nettype", va("%d", type) );
			CL_SetServerInfoByAddress(from, infoString, cl_pinglist[i].time);

			return;
		}
	}

	// if not just sent a local broadcast or pinging local servers
	if (cls.pingUpdateSource != AS_LOCAL) {
		return;
	}

	for ( i = 0 ; i < MAX_OTHER_SERVERS ; i++ ) {
		// empty slot
		if ( cls.localServers[i].adr.port == 0 ) {
			break;
		}

		// avoid duplicate
		if ( Network::compareAdr( from, cls.localServers[i].adr ) ) {
			return;
		}
	}

	if ( i == MAX_OTHER_SERVERS ) {
		Com_DPrintf( "MAX_OTHER_SERVERS hit, dropping infoResponse\n" );
		return;
	}

	// add this to the list
	cls.numlocalservers = i+1;
	CL_InitServerInfo( &cls.localServers[i], &from );

	Q_strncpyz( info, msg->readString(), MAX_INFO_STRING );
	if (strlen(info)) {
		if (info[strlen(info)-1] != '\n') {
			Q_strcat(info, sizeof(info), "\n");
		}
		Com_Printf( "%s: %s", Network::adrToStringwPort( from ), info );
	}
}

/*
===================
CL_GetServerStatus
===================
*/
serverStatus_t *CL_GetServerStatus( netadr_t from ) {
	int i, oldest, oldestTime;

	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if ( Network::compareAdr( from, cl_serverStatusList[i].address ) ) {
			return &cl_serverStatusList[i];
		}
	}
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if ( cl_serverStatusList[i].retrieved ) {
			return &cl_serverStatusList[i];
		}
	}
	oldest = -1;
	oldestTime = 0;
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if (oldest == -1 || cl_serverStatusList[i].startTime < oldestTime) {
			oldest = i;
			oldestTime = cl_serverStatusList[i].startTime;
		}
	}
	return &cl_serverStatusList[oldest];
}

/*
===================
CL_ServerStatus
===================
*/
int CL_ServerStatus( const char *serverAddress, char *serverStatusString, int maxLen ) {
	int i;
	netadr_t	to;
	serverStatus_t *serverStatus;

	// if no server address then reset all server status requests
	if ( !serverAddress ) {
		for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
			cl_serverStatusList[i].address.port = 0;
			cl_serverStatusList[i].retrieved = true;
		}
		return false;
	}
	// get the address
	if ( !Network::stringToAdr( serverAddress, &to, NA_UNSPEC) ) {
		return false;
	}
	serverStatus = CL_GetServerStatus( to );
	// if no server status string then reset the server status request for this address
	if ( !serverStatusString ) {
		serverStatus->retrieved = true;
		return false;
	}

	// if this server status request has the same address
	if ( Network::compareAdr( to, serverStatus->address) ) {
		// if we received a response for this server status request
		if (!serverStatus->pending) {
			Q_strncpyz(serverStatusString, serverStatus->string, maxLen);
			serverStatus->retrieved = true;
			serverStatus->startTime = 0;
			return true;
		}
		// resend the request regularly
		else if ( serverStatus->startTime < Com_Milliseconds() - cl_serverStatusResendTime->integer ) {
			serverStatus->print = false;
			serverStatus->pending = true;
			serverStatus->retrieved = false;
			serverStatus->time = 0;
			serverStatus->startTime = Com_Milliseconds();
			Network::outOfBandPrint( NS_CLIENT, to, "getstatus" );
			return false;
		}
	}
	// if retrieved
	else if ( serverStatus->retrieved ) {
		serverStatus->address = to;
		serverStatus->print = false;
		serverStatus->pending = true;
		serverStatus->retrieved = false;
		serverStatus->startTime = Com_Milliseconds();
		serverStatus->time = 0;
		Network::outOfBandPrint( NS_CLIENT, to, "getstatus" );
		return false;
	}
	return false;
}

/*
===================
CL_ServerStatusResponse
===================
*/
void CL_ServerStatusResponse( netadr_t from, Message *msg ) {
	char	*s;
	char	info[MAX_INFO_STRING];
	int		i, l, score, ping;
	int		len;
	serverStatus_t *serverStatus;

	serverStatus = nullptr;
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if ( Network::compareAdr( from, cl_serverStatusList[i].address ) ) {
			serverStatus = &cl_serverStatusList[i];
			break;
		}
	}
	// if we didn't request this server status
	if (!serverStatus) {
		return;
	}

	s = msg->readStringLine();

	len = 0;
	Com_sprintf(&serverStatus->string[len], sizeof(serverStatus->string)-len, "%s", s);

	if (serverStatus->print) {
		Com_Printf("Server settings:\n");
		// print cvars
		while (*s) {
			for (i = 0; i < 2 && *s; i++) {
				if (*s == '\\')
					s++;
				l = 0;
				while (*s) {
					info[l++] = *s;
					if (l >= MAX_INFO_STRING-1)
						break;
					s++;
					if (*s == '\\') {
						break;
					}
				}
				info[l] = '\0';
				if (i) {
					Com_Printf("%s\n", info);
				}
				else {
					Com_Printf("%-24s", info);
				}
			}
		}
	}

	len = strlen(serverStatus->string);
	Com_sprintf(&serverStatus->string[len], sizeof(serverStatus->string)-len, "\\");

	if (serverStatus->print) {
		Com_Printf("\nPlayers:\n");
		Com_Printf("num: score: ping: name:\n");
	}
	for (i = 0, s = msg->readStringLine(); *s; s = msg->readStringLine(), i++) {

		len = strlen(serverStatus->string);
		Com_sprintf(&serverStatus->string[len], sizeof(serverStatus->string)-len, "\\%s", s);

		if (serverStatus->print) {
			score = ping = 0;
			sscanf(s, "%d %d", &score, &ping);
			s = strchr(s, ' ');
			if (s)
				s = strchr(s+1, ' ');
			if (s)
				s++;
			else
				s = "unknown";
			Com_Printf("%-2d   %-3d    %-3d   %s\n", i, score, ping, s );
		}
	}
	len = strlen(serverStatus->string);
	Com_sprintf(&serverStatus->string[len], sizeof(serverStatus->string)-len, "\\");

	serverStatus->time = Com_Milliseconds();
	serverStatus->address = from;
	serverStatus->pending = false;
	if (serverStatus->print) {
		serverStatus->retrieved = true;
	}
}

/*
==================
CL_LocalServers_f
==================
*/
void CL_LocalServers_f( ) {
	char		*message;
	int			i, j;
	netadr_t	to;

	Com_Printf( "Scanning for servers on the local network...\n");

	// reset the list, waiting for response
	cls.numlocalservers = 0;
	cls.pingUpdateSource = AS_LOCAL;

	for (i = 0; i < MAX_OTHER_SERVERS; i++) {
		bool b = cls.localServers[i].visible;
		Com_Memset(&cls.localServers[i], 0, sizeof(cls.localServers[i]));
		cls.localServers[i].visible = b;
	}
	Com_Memset( &to, 0, sizeof( to ) );

	// The 'xxx' in the message is a challenge that will be echoed back
	// by the server.  We don't care about that here, but master servers
	// can use that to prevent spoofed server responses from invalid ip
	message = "\377\377\377\377getinfo xxx";

	// send each message twice in case one is dropped
	for ( i = 0 ; i < 2 ; i++ ) {
		// send a broadcast packet on each server port
		// we support multiple server ports so a single machine
		// can nicely run multiple servers
		for ( j = 0 ; j < NUM_SERVER_PORTS ; j++ ) {
			to.port = BigShort( (short)(PORT_SERVER + j) );

			to.type = NA_BROADCAST;
			Network::sendPacket( NS_CLIENT, strlen( message ), message, to );
			to.type = NA_MULTICAST6;
			Network::sendPacket( NS_CLIENT, strlen( message ), message, to );
		}
	}
}

/*
==================
CL_GlobalServers_f

Originally master 0 was Internet and master 1 was MPlayer.
ioquake3 2008; added support for requesting five separate master servers using 0-4.
ioquake3 2017; made master 0 fetch all master servers and 1-5 request a single master server.
==================
*/
void CL_GlobalServers_f( ) {
	netadr_t	to;
	int			count, i, masterNum;
	char		command[1024], *masteraddress;
	
	if ((count = CommandSystem::argc()) < 3 || (masterNum = atoi(CommandSystem::argv(1))) < 0 || masterNum > MAX_MASTER_SERVERS)
	{
		Com_Printf("usage: globalservers <master# 0-%d> <protocol> [keywords]\n", MAX_MASTER_SERVERS);
		return;	
	}

	// request from all master servers
	if ( masterNum == 0 ) {
		int numAddress = 0;

		for ( i = 1; i <= MAX_MASTER_SERVERS; i++ ) {
			sprintf(command, "sv_master%d", i);
			masteraddress = CvarSystem::variableString(command);

			if(!*masteraddress)
				continue;

			numAddress++;

			Com_sprintf(command, sizeof(command), "globalservers %d %s %s\n", i, CommandSystem::argv(2), CommandSystem::argsFrom(3));
			CommandBuffer::addText(command);
		}

		if ( !numAddress ) {
			Com_Printf( "CL_GlobalServers_f: Error: No master server addresses.\n");
		}
		return;
	}

	sprintf(command, "sv_master%d", masterNum);
	masteraddress = CvarSystem::variableString(command);
	
	if(!*masteraddress)
	{
		Com_Printf( "CL_GlobalServers_f: Error: No master server address given.\n");
		return;	
	}

	// reset the list, waiting for response
	// -1 is used to distinguish a "no response"

	i = Network::stringToAdr(masteraddress, &to, NA_UNSPEC);
	
	if(!i)
	{
		Com_Printf( "CL_GlobalServers_f: Error: could not resolve address of master %s\n", masteraddress);
		return;	
	}
	else if(i == 2)
		to.port = BigShort(PORT_MASTER);

	Com_Printf("Requesting servers from %s (%s)...\n", masteraddress, Network::adrToStringwPort(to));

	cls.numglobalservers = -1;
	cls.pingUpdateSource = AS_GLOBAL;

	// Use the extended query for IPv6 masters
	if (to.type == NA_IP6 || to.type == NA_MULTICAST6)
	{
		int v4enabled = CvarSystem::variableIntegerValue("net_enabled") & NET_ENABLEV4;
		
		if(v4enabled)
		{
			Com_sprintf(command, sizeof(command), "getserversExt %s %s",
				com_gamename->string, CommandSystem::argv(2));
		}
		else
		{
			Com_sprintf(command, sizeof(command), "getserversExt %s %s ipv6",
				com_gamename->string, CommandSystem::argv(2));
		}
	}
	else if ( !Q_stricmp( com_gamename->string, LEGACY_MASTER_GAMENAME ) )
		Com_sprintf(command, sizeof(command), "getservers %s",
			CommandSystem::argv(2));
	else
		Com_sprintf(command, sizeof(command), "getservers %s %s",
			com_gamename->string, CommandSystem::argv(2));

	for (i=3; i < count; i++)
	{
		Q_strcat(command, sizeof(command), " ");
		Q_strcat(command, sizeof(command), CommandSystem::argv(i));
	}

	Network::outOfBandPrint( NS_SERVER, to, "%s", command );
}


/*
==================
CL_GetPing
==================
*/
void CL_GetPing( int n, char *buf, int buflen, int *pingtime )
{
	const char	*str;
	int		time;
	int		maxPing;

	if (n < 0 || n >= MAX_PINGREQUESTS || !cl_pinglist[n].adr.port)
	{
		// empty or invalid slot
		buf[0]    = '\0';
		*pingtime = 0;
		return;
	}

	str = Network::adrToStringwPort( cl_pinglist[n].adr );
	Q_strncpyz( buf, str, buflen );

	time = cl_pinglist[n].time;
	if (!time)
	{
		// check for timeout
		time = Sys_Milliseconds() - cl_pinglist[n].start;
		maxPing = CvarSystem::variableIntegerValue( "cl_maxPing" );
		if( maxPing < 100 ) {
			maxPing = 100;
		}
		if (time < maxPing)
		{
			// not timed out yet
			time = 0;
		}
	}

	CL_SetServerInfoByAddress(cl_pinglist[n].adr, cl_pinglist[n].info, cl_pinglist[n].time);

	*pingtime = time;
}

/*
==================
CL_GetPingInfo
==================
*/
void CL_GetPingInfo( int n, char *buf, int buflen )
{
	if (n < 0 || n >= MAX_PINGREQUESTS || !cl_pinglist[n].adr.port)
	{
		// empty or invalid slot
		if (buflen)
			buf[0] = '\0';
		return;
	}

	Q_strncpyz( buf, cl_pinglist[n].info, buflen );
}

/*
==================
CL_ClearPing
==================
*/
void CL_ClearPing( int n )
{
	if (n < 0 || n >= MAX_PINGREQUESTS)
		return;

	cl_pinglist[n].adr.port = 0;
}

/*
==================
CL_GetPingQueueCount
==================
*/
int CL_GetPingQueueCount( )
{
	int		i;
	int		count;
	ping_t*	pingptr;

	count   = 0;
	pingptr = cl_pinglist;

	for (i=0; i<MAX_PINGREQUESTS; i++, pingptr++ ) {
		if (pingptr->adr.port) {
			count++;
		}
	}

	return (count);
}

/*
==================
CL_GetFreePing
==================
*/
ping_t* CL_GetFreePing( )
{
	ping_t*	pingptr;
	ping_t*	best;	
	int		oldest;
	int		i;
	int		time;

	pingptr = cl_pinglist;
	for (i=0; i<MAX_PINGREQUESTS; i++, pingptr++ )
	{
		// find free ping slot
		if (pingptr->adr.port)
		{
			if (!pingptr->time)
			{
				if (Sys_Milliseconds() - pingptr->start < 500)
				{
					// still waiting for response
					continue;
				}
			}
			else if (pingptr->time < 500)
			{
				// results have not been queried
				continue;
			}
		}

		// clear it
		pingptr->adr.port = 0;
		return (pingptr);
	}

	// use oldest entry
	pingptr = cl_pinglist;
	best    = cl_pinglist;
	oldest  = INT_MIN;
	for (i=0; i<MAX_PINGREQUESTS; i++, pingptr++ )
	{
		// scan for oldest
		time = Sys_Milliseconds() - pingptr->start;
		if (time > oldest)
		{
			oldest = time;
			best   = pingptr;
		}
	}

	return (best);
}

/*
==================
CL_Ping_f
==================
*/
void CL_Ping_f( ) {
	netadr_t	to;
	ping_t*		pingptr;
	char*		server;
	int			argc;
	netadrtype_t	family = NA_UNSPEC;

	argc = CommandSystem::argc();

	if ( argc != 2 && argc != 3 ) {
		Com_Printf( "usage: ping [-4|-6] server\n");
		return;	
	}
	
	if(argc == 2)
		server = CommandSystem::argv(1);
	else
	{
		if(!strcmp(CommandSystem::argv(1), "-4"))
			family = NA_IP;
		else if(!strcmp(CommandSystem::argv(1), "-6"))
			family = NA_IP6;
		else
			Com_Printf( "warning: only -4 or -6 as address type understood.\n");
		
		server = CommandSystem::argv(2);
	}

	Com_Memset( &to, 0, sizeof(netadr_t) );

	if ( !Network::stringToAdr( server, &to, family ) ) {
		return;
	}

	pingptr = CL_GetFreePing();

	memcpy( &pingptr->adr, &to, sizeof (netadr_t) );
	pingptr->start = Sys_Milliseconds();
	pingptr->time  = 0;

	CL_SetServerInfoByAddress(pingptr->adr, nullptr, 0);
		
	Network::outOfBandPrint( NS_CLIENT, to, "getinfo xxx" );
}

/*
==================
CL_UpdateVisiblePings_f
==================
*/
bool CL_UpdateVisiblePings_f(int source) {
	int			slots, i;
	char		buff[MAX_STRING_CHARS];
	int			pingTime;
	int			max;
	bool status = false;

	if (source < 0 || source > AS_FAVORITES) {
		return false;
	}

	cls.pingUpdateSource = source;

	slots = CL_GetPingQueueCount();
	if (slots < MAX_PINGREQUESTS) {
		serverInfo_t *server = nullptr;

		switch (source) {
			case AS_LOCAL :
				server = &cls.localServers[0];
				max = cls.numlocalservers;
			break;
			case AS_GLOBAL :
				server = &cls.globalServers[0];
				max = cls.numglobalservers;
			break;
			case AS_FAVORITES :
				server = &cls.favoriteServers[0];
				max = cls.numfavoriteservers;
			break;
			default:
				return false;
		}
		for (i = 0; i < max; i++) {
			if (server[i].visible) {
				if (server[i].ping == -1) {
					int j;

					if (slots >= MAX_PINGREQUESTS) {
						break;
					}
					for (j = 0; j < MAX_PINGREQUESTS; j++) {
						if (!cl_pinglist[j].adr.port) {
							continue;
						}
						if (Network::compareAdr( cl_pinglist[j].adr, server[i].adr)) {
							// already on the list
							break;
						}
					}
					if (j >= MAX_PINGREQUESTS) {
						status = true;
						for (j = 0; j < MAX_PINGREQUESTS; j++) {
							if (!cl_pinglist[j].adr.port) {
								break;
							}
						}
						memcpy(&cl_pinglist[j].adr, &server[i].adr, sizeof(netadr_t));
						cl_pinglist[j].start = Sys_Milliseconds();
						cl_pinglist[j].time = 0;
						Network::outOfBandPrint( NS_CLIENT, cl_pinglist[j].adr, "getinfo xxx" );
						slots++;
					}
				}
				// if the server has a ping higher than cl_maxPing or
				// the ping packet got lost
				else if (server[i].ping == 0) {
					// if we are updating global servers
					if (source == AS_GLOBAL) {
						//
						if ( cls.numGlobalServerAddresses > 0 ) {
							// overwrite this server with one from the additional global servers
							cls.numGlobalServerAddresses--;
							CL_InitServerInfo(&server[i], &cls.globalServerAddresses[cls.numGlobalServerAddresses]);
							// NOTE: the server[i].visible flag stays untouched
						}
					}
				}
			}
		}
	} 

	if (slots) {
		status = true;
	}
	for (i = 0; i < MAX_PINGREQUESTS; i++) {
		if (!cl_pinglist[i].adr.port) {
			continue;
		}
		CL_GetPing( i, buff, MAX_STRING_CHARS, &pingTime );
		if (pingTime != 0) {
			CL_ClearPing(i);
			status = true;
		}
	}

	return status;
}

/*
==================
CL_ServerStatus_f
==================
*/
void CL_ServerStatus_f() {
	netadr_t	to, *toptr = nullptr;
	char		*server;
	serverStatus_t *serverStatus;
	int			argc;
	netadrtype_t	family = NA_UNSPEC;

	argc = CommandSystem::argc();

	if ( argc != 2 && argc != 3 )
	{
		if (clc.state != CA_ACTIVE || clc.demoplaying)
		{
			Com_Printf ("Not connected to a server.\n");
			Com_Printf( "usage: serverstatus [-4|-6] server\n");
			return;
		}

		toptr = &clc.serverAddress;
	}
	
	if(!toptr)
	{
		Com_Memset( &to, 0, sizeof(netadr_t) );
	
		if(argc == 2)
			server = CommandSystem::argv(1);
		else
		{
			if(!strcmp(CommandSystem::argv(1), "-4"))
				family = NA_IP;
			else if(!strcmp(CommandSystem::argv(1), "-6"))
				family = NA_IP6;
			else
				Com_Printf( "warning: only -4 or -6 as address type understood.\n");
		
			server = CommandSystem::argv(2);
		}

		toptr = &to;
		if ( !Network::stringToAdr( server, toptr, family ) )
			return;
	}

	Network::outOfBandPrint( NS_CLIENT, *toptr, "getstatus" );

	serverStatus = CL_GetServerStatus( *toptr );
	serverStatus->address = *toptr;
	serverStatus->print = true;
	serverStatus->pending = true;
}

/*
==================
CL_ShowIP_f
==================
*/
void CL_ShowIP_f() {
	Sys_ShowIP();
}

/*
=================
CL_CDKeyValidate
=================
*/
bool CL_CDKeyValidate( const char *key, const char *checksum ) {
#ifdef STANDALONE
	return true;
#else
	char	ch;
	byte	sum;
	char	chs[3];
	int i, len;

	len = strlen(key);
	if( len != CDKEY_LEN ) {
		return false;
	}

	if( checksum && strlen( checksum ) != CDCHKSUM_LEN ) {
		return false;
	}

	sum = 0;
	// for loop gets rid of conditional assignment warning
	for (i = 0; i < len; i++) {
		ch = *key++;
		switch(tolower(ch)) {
		case '2':
		case '3':
		case '7':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'g':
		case 'h':
		case 'j':
		case 'l':
		case 'p':
		case 'r':
		case 's':
		case 't':
		case 'w':
			sum += ch;
			continue;
		default:
			return false;
		}
	}

	sprintf(chs, "%02x", sum);
	
	if (checksum && !Q_stricmp(chs, checksum)) {
		return true;
	}

	if (!checksum) {
		return true;
	}

	return false;
#endif
}
