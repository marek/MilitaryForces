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
// cl_parse.c  -- parse a message received from the server

#include "client.h"
#include "../ui/ui.h"
#include "../cgame/cg.h"

char *svc_strings[256] = {
	"svc_bad",

	"svc_nop",
	"svc_gamestate",
	"svc_configstring",
	"svc_baseline",	
	"svc_serverCommand",
	"svc_download",
	"svc_snapshot",
	"svc_EOF",
	"svc_voipSpeex",
	"svc_voipOpus",
};

void SHOWNET( Message *msg, char *s) {
	if ( cl_shownet->integer >= 2) {
		Common::printf ("%3i:%s\n", msg->readcount-1, s);
	}
}


/*
=========================================================================

MESSAGE PARSING

=========================================================================
*/

/*
==================
CL_DeltaEntity

Parses deltas from the given base and adds the resulting entity
to the current frame
==================
*/
void CL_DeltaEntity (Message *msg, clSnapshot_t *frame, int newnum, entityState_t *old, 
					 bool unchanged, bool discard = false) {
	entityState_t	*state;

	// save the parsed entity state into the big circular buffer so
	// it can be used as the source for a later delta
	state = &cl.parseEntities[cl.parseEntitiesNum & (MAX_PARSE_ENTITIES-1)];

	if ( unchanged ) {
		*state = *old;
	} else {
		msg->readDeltaEntity( old, state, newnum );
	}

	if ( state->number == ENTITYNUM_WIRE_END ) {
		return;		// entity was delta removed
	}
	if ( discard ) {
		return;
	}
	cl.parseEntitiesNum++;
	frame->numEntities++;
}

/*
==================
CL_ParsePacketEntities

==================
*/
void CL_ParsePacketEntities( Message *msg, clSnapshot_t *oldframe, clSnapshot_t *newframe) {
	int			newnum;
	entityState_t	*oldstate;
	int			oldindex, oldnum;
	bool		create;

	newframe->parseEntitiesNum = cl.parseEntitiesNum;
	newframe->numEntities = 0;

	// delta from the entities present in oldframe
	oldindex = 0;
	oldstate = nullptr;
	if (!oldframe) {
		oldnum = 99999;
	} else {
		if ( oldindex >= oldframe->numEntities ) {
			oldnum = 99999;
		} else {
			oldstate = &cl.parseEntities[
				(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}

	while ( true ) {
		// read the entity index number
		newnum = msg->readBits( GENTITYNUM_BITS );

		if ( newnum == ENTITYNUM_WIRE_END ) {
			break;
		}

		if ( msg->readcount > msg->cursize ) {
			Common::error (ERR_DROP,"CL_ParsePacketEntities: end of message");
		}

		create = msg->readBits( 1 ) != 0;

		while ( oldnum < newnum ) {
			// one or more entities from the old packet are unchanged
			if ( cl_shownet->integer == 3 ) {
				Common::printf ("%3i:  unchanged: %i\n", msg->readcount, oldnum);
			}
			CL_DeltaEntity( msg, newframe, oldnum, oldstate, true );
			
			oldindex++;

			if ( oldindex >= oldframe->numEntities ) {
				oldnum = 99999;
			} else {
				oldstate = &cl.parseEntities[
					(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
		}
		if (oldnum == newnum) {
			// delta from previous state
			if ( cl_shownet->integer == 3 ) {
				Common::printf ("%3i:  delta: %i\n", msg->readcount, newnum);
			}
			CL_DeltaEntity( msg, newframe, newnum, create ? &cl.entityBaselines[newnum] : oldstate, false );

			oldindex++;

			if ( oldindex >= oldframe->numEntities ) {
				oldnum = 99999;
			} else {
				oldstate = &cl.parseEntities[
					(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
				oldnum = oldstate->number;
			}
			continue;
		}

		if ( oldnum > newnum ) {
			// delta from baseline
			if ( cl_shownet->integer == 3 ) {
				Common::printf ("%3i:  baseline: %i\n", msg->readcount, newnum);
			}
			CL_DeltaEntity( msg, newframe, newnum, &cl.entityBaselines[newnum], false, !create );
			continue;
		}

	}

	// any remaining entities in the old frame are copied over
	while ( oldnum != 99999 ) {
		// one or more entities from the old packet are unchanged
		if ( cl_shownet->integer == 3 ) {
			Common::printf ("%3i:  unchanged: %i\n", msg->readcount, oldnum);
		}
		CL_DeltaEntity( msg, newframe, oldnum, oldstate, true );
		
		oldindex++;

		if ( oldindex >= oldframe->numEntities ) {
			oldnum = 99999;
		} else {
			oldstate = &cl.parseEntities[
				(oldframe->parseEntitiesNum + oldindex) & (MAX_PARSE_ENTITIES-1)];
			oldnum = oldstate->number;
		}
	}
}


/*
================
CL_ParseSnapshot

If the snapshot is parsed properly, it will be copied to
cl.snap and saved in cl.snapshots[].  If the snapshot is invalid
for any reason, no changes to the state will be made at all.
================
*/
void CL_ParseSnapshot( Message *msg ) {
	int			len;
	clSnapshot_t	*old;
	clSnapshot_t	newSnap;
	int			deltaNum;
	int			oldMessageNum;
	int			i, packetNum;

	// get the reliable sequence acknowledge number
	// NOTE: now sent with all server to client messages
	//clc.reliableAcknowledge = msg->readLong();

	// read in the new snapshot to a temporary buffer
	// we will only copy to cl.snap if it is valid
	Com_Memset (&newSnap, 0, sizeof(newSnap));

	// we will have read any new server commands in this
	// message before we got to svc_snapshot
	newSnap.serverCommandNum = clc.serverCommandSequence;

	newSnap.serverTime = msg->readLong();

	// if we were just unpaused, we can only *now* really let the
	// change come into effect or the client hangs.
	cl_paused->modified = false;

	newSnap.messageNum = clc.serverMessageSequence;

	deltaNum = msg->readByte();
	if ( !deltaNum ) {
		newSnap.deltaNum = -1;
	} else {
		newSnap.deltaNum = newSnap.messageNum - deltaNum;
	}
	newSnap.snapFlags = msg->readByte();

	// If the frame is delta compressed from data that we
	// no longer have available, we must suck up the rest of
	// the frame, but not use it, then ask for a non-compressed
	// message 
	if ( newSnap.deltaNum <= 0 ) {
		newSnap.valid = true;		// uncompressed frame
		old = nullptr;
		if ( newSnap.snapFlags & SNAPFLAG_ENTITY_RESET ) {
			clc.demowaiting = false;	// we can start recording now
		}
	} else {
		old = &cl.snapshots[newSnap.deltaNum & PACKET_MASK];
		if ( !old->valid ) {
			// should never happen
			Common::printf ("Delta from invalid frame (not supposed to happen!).\n");
		} else if ( old->messageNum != newSnap.deltaNum ) {
			// The frame that the server did the delta from
			// is too old, so we can't reconstruct it properly.
			Common::printf ("Delta frame too old.\n");
		} else {
			newSnap.valid = true;	// valid delta parse
		}
	}

	// read areamask
	len = msg->readByte();
	
	if(len > sizeof(newSnap.areamask))
	{
		Common::error (ERR_DROP,"CL_ParseSnapshot: Invalid size %d for areamask", len);
		return;
	}
	
	msg->readData( &newSnap.areamask, len);

	// read playerinfo
	SHOWNET( msg, "playerstate" );
	if ( old ) {
		msg->readDeltaPlayerstate( &old->ps, &newSnap.ps );
	} else {
		msg->readDeltaPlayerstate( nullptr, &newSnap.ps );
	}

	// read packet entities
	SHOWNET( msg, "packet entities" );
	if ( ( newSnap.snapFlags & SNAPFLAG_ENTITY_RESET ) || !cl.snap.valid ) {
		CL_ParsePacketEntities( msg, nullptr, &newSnap );
	} else {
		CL_ParsePacketEntities( msg, &cl.snap, &newSnap );
	}

	// if not valid, dump the entire thing now that it has
	// been properly read
	if ( !newSnap.valid ) {
		return;
	}

	// clear the valid flags of any snapshots between the last
	// received and this one, so if there was a dropped packet
	// it won't look like something valid to delta from next
	// time we wrap around in the buffer
	oldMessageNum = cl.snap.messageNum + 1;

	if ( newSnap.messageNum - oldMessageNum >= PACKET_BACKUP ) {
		oldMessageNum = newSnap.messageNum - ( PACKET_BACKUP - 1 );
	}
	for ( ; oldMessageNum < newSnap.messageNum ; oldMessageNum++ ) {
		cl.snapshots[oldMessageNum & PACKET_MASK].valid = false;
	}

	if ( cl.snap.valid ) {
		i = newSnap.messageNum - cl.snap.messageNum;
		if ( i >= 1 && i <= 32 ) {
			cl.entityAckBits = ( i == 32 ? 0 : cl.entityAckBits << i ) | ( 1 << ( i - 1 ) );
		} else {
			cl.entityAckBits = 0;
		}
	} else {
		cl.entityAckBits = 0;
	}

	// copy to the current good spot
	cl.snap = newSnap;
	cl.snap.ping = 999;
	// calculate ping time
	for ( i = 0 ; i < PACKET_BACKUP ; i++ ) {
		packetNum = ( clc.netchan.outgoingSequence() - 1 - i ) & PACKET_MASK;
		if ( cl.snap.ps.commandTime >= cl.outPackets[ packetNum ].p_serverTime ) {
			cl.snap.ping = cls.realtime - cl.outPackets[ packetNum ].p_realtime;
			break;
		}
	}
	// save the frame off in the backup array for later delta comparisons
	cl.snapshots[cl.snap.messageNum & PACKET_MASK] = cl.snap;

	if (cl_shownet->integer == 3) {
		Common::printf( "   snapshot:%i  delta:%i  ping:%i\n", cl.snap.messageNum,
		cl.snap.deltaNum, cl.snap.ping );
	}

	cl.newSnapshots = true;
}


//=====================================================================

int cl_connectedToPureServer;
int cl_connectedToCheatServer;

/*
==================
Client::systemInfoChanged

The systeminfo configstring has been changed, so parse
new information out of it.  This will happen at every
gamestate, and possibly during gameplay.
==================
*/
void Client::systemInfoChanged( ) {
	char			*systemInfo;
	const char		*s, *t;
	char			key[BIG_INFO_KEY];
	char			value[BIG_INFO_VALUE];
	bool		gameSet;

	systemInfo = cl.gameState.stringData + cl.gameState.stringOffsets[ CS_SYSTEMINFO ];
	// NOTE TTimo:
	// when the serverId changes, any further messages we send to the server will use this new serverId
	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=475
	// in some cases, outdated cp commands might get sent with this news serverId
	cl.serverId = atoi( Info::valueForKey( systemInfo, "sv_serverid" ) );

#ifdef USE_VOIP
#ifdef LEGACY_PROTOCOL
	if(clc.compat)
		clc.voipEnabled = false;
	else
#endif
	{
		s = Info::valueForKey( systemInfo, "sv_voipProtocol" );
		clc.voipEnabled = !Q_stricmp(s, "opus");
	}
#endif

	// don't set any vars when playing a demo
	if ( clc.demoplaying ) {
		return;
	}

	s = Info::valueForKey( systemInfo, "sv_cheats" );
	cl_connectedToCheatServer = atoi( s );
	if ( !cl_connectedToCheatServer ) {
		CvarSystem::setCheatState();
	}

	// check pure server string
	s = Info::valueForKey( systemInfo, "sv_paks" );
	t = Info::valueForKey( systemInfo, "sv_pakNames" );
	FileSystem::pureServerSetLoadedPaks( s, t );

	s = Info::valueForKey( systemInfo, "sv_referencedPaks" );
	t = Info::valueForKey( systemInfo, "sv_referencedPakNames" );
	FileSystem::pureServerSetReferencedPaks( s, t );

	gameSet = false;
	// scan through all the variables in the systeminfo and locally set cvars to match
	s = systemInfo;
	while ( s ) {
		int cvar_flags;
		
		Info::nextPair( &s, key, value );
		if ( !key[0] ) {
			break;
		}
		
		// ehw!
		if (!Q_stricmp(key, "fs_game"))
		{
			if(FileSystem::invalidGameDir(value))
			{
				Common::printf(S_COLOR_YELLOW "WARNING: Server sent invalid fs_game value %s\n", value);
				continue;
			}
				
			gameSet = true;
		}

		if((cvar_flags = CvarSystem::flags(key)) == CVAR_NONEXISTENT)
			CvarSystem::get(key, value, CVAR_SERVER_CREATED | CVAR_ROM);
		else
		{
			// If this cvar may not be modified by a server discard the value.
			if(!(cvar_flags & (CVAR_SYSTEMINFO | CVAR_SERVER_CREATED | CVAR_USER_CREATED)))
			{
#ifndef STANDALONE
				if(Q_stricmp(key, "g_synchronousClients") && Q_stricmp(key, "pmove_fixed") &&
				   Q_stricmp(key, "pmove_msec"))
#endif
				{
					Common::printf(S_COLOR_YELLOW "WARNING: server is not allowed to set %s=%s\n", key, value);
					continue;
				}
			}

			CvarSystem::setSafe(key, value);
		}
	}
	// if game folder should not be set and it is set at the client side
	if ( !gameSet && *CvarSystem::variableString("fs_game") ) {
		CvarSystem::set( "fs_game", "" );
	}
	cl_connectedToPureServer = CvarSystem::variableValue( "sv_pure" );
}

/*
==================
Client::parseServerInfo
==================
*/
void Client::parseServerInfo()
{
	const char *serverInfo;

	serverInfo = cl.gameState.stringData
		+ cl.gameState.stringOffsets[ CS_SERVERINFO ];

	clc.sv_allowDownload = atoi(Info::valueForKey(serverInfo,
		"sv_allowDownload"));
	Q_strncpyz(clc.sv_dlURL,
		Info::valueForKey(serverInfo, "sv_dlURL"),
		sizeof(clc.sv_dlURL));
}

/*
==================
CL_ParseGamestate
==================
*/
void CL_ParseGamestate( Message *msg ) {
	int				i;
	entityState_t	*es;
	int				newnum;
	entityState_t	nullstate;
	int				cmd;
	char			*s;
	char oldGame[MAX_QPATH];

	Console::close();

	clc.connectPacketCount = 0;

	// wipe local client state
	Client::clearState();

	// a gamestate always marks a server command sequence
	clc.serverCommandSequence = msg->readLong();

	// parse all the configstrings and baselines
	cl.gameState.dataCount = 1;	// leave a 0 at the beginning for uninitialized configstrings
	while ( true ) {
		cmd = msg->readByte();

		if ( cmd == svc_EOF ) {
			break;
		}
		
		if ( cmd == svc_configstring ) {
			int		len;

			i = msg->readShort();
			if ( i < 0 || i >= MAX_CONFIGSTRINGS ) {
				Common::error( ERR_DROP, "configstring > MAX_CONFIGSTRINGS" );
			}
			s = msg->readBigString();
			len = strlen( s );

			if ( len + 1 + cl.gameState.dataCount > MAX_GAMESTATE_CHARS ) {
				Common::error( ERR_DROP, "MAX_GAMESTATE_CHARS exceeded" );
			}

			// append it to the gameState string buffer
			cl.gameState.stringOffsets[ i ] = cl.gameState.dataCount;
			Com_Memcpy( cl.gameState.stringData + cl.gameState.dataCount, s, len + 1 );
			cl.gameState.dataCount += len + 1;
		} else if ( cmd == svc_baseline ) {
			newnum = msg->readBits( GENTITYNUM_BITS );
			if ( newnum < 0 || newnum >= MAX_GENTITIES ) {
				Common::error( ERR_DROP, "Baseline number out of range: %i", newnum );
			}
			Com_Memset (&nullstate, 0, sizeof(nullstate));
			es = &cl.entityBaselines[ newnum ];
			msg->readDeltaEntity( &nullstate, es, newnum );
		} else {
			Common::error( ERR_DROP, "CL_ParseGamestate: bad command byte" );
		}
	}

	clc.clientNum = msg->readLong();
	// read the checksum feed
	clc.checksumFeed = msg->readLong();

	// save old gamedir
	CvarSystem::variableStringBuffer("fs_game", oldGame, sizeof(oldGame));

	// parse useful values out of CS_SERVERINFO
	Client::parseServerInfo();

	// parse serverId and other cvars
	Client::systemInfoChanged();

	// stop recording now so the demo won't have an unnecessary level load at the end.
	if(cl_autoRecordDemo->integer && clc.demorecording)
		Client::stopRecord_f();
	
	// reinitialize the filesystem if the game directory has changed
	if(!cl_oldGameSet && (CvarSystem::flags("fs_game") & CVAR_MODIFIED))
	{
		cl_oldGameSet = true;
		Q_strncpyz(cl_oldGame, oldGame, sizeof(cl_oldGame));
	}

	FileSystem::conditionalRestart(clc.checksumFeed, false);

	// This used to call CL_StartHunkUsers, but now we enter the download state before loading the
	// cgame
	Client::initDownloads();

	// make sure the game starts
	CvarSystem::set( "cl_paused", "0" );
}


//=====================================================================

/*
=====================
CL_ParseDownload

A download message has been received from the server
=====================
*/
void CL_ParseDownload ( Message *msg ) {
	int		size;
	unsigned char data[MAX_MSGLEN];
	uint16_t block;

	if (!*clc.downloadTempName) {
		Common::printf("Server sending download, but no download was requested\n");
		CL_AddReliableCommand("stopdl", false);
		return;
	}

	// read the data
	block = msg->readShort();

	if(!block && !clc.downloadBlock)
	{
		// block zero is special, contains file size
		clc.downloadSize = msg->readLong();

		CvarSystem::setValue( "cl_downloadSize", clc.downloadSize );

		if (clc.downloadSize < 0)
		{
			Common::error( ERR_DROP, "%s", msg->readString() );
			return;
		}
	}

	size = msg->readShort();
	if (size < 0 || size > sizeof(data))
	{
		Common::error(ERR_DROP, "CL_ParseDownload: Invalid size %d for download chunk", size);
		return;
	}
	
	msg->readData( data, size);

	if((clc.downloadBlock & 0xFFFF) != block)
	{
		Common::dPrintf( "CL_ParseDownload: Expected block %d, got %d\n", (clc.downloadBlock & 0xFFFF), block);
		return;
	}

	// open the file if not opened yet
	if (!clc.download)
	{
		clc.download = FileSystem::baseDir_FOpenFileWrite_HomeData( clc.downloadTempName );

		if (!clc.download) {
			Common::printf( "Could not create %s\n", clc.downloadTempName );
			CL_AddReliableCommand("stopdl", false);
			Client::nextDownload();
			return;
		}
	}

	if (size)
		FileSystem::write( data, size, clc.download );

	CL_AddReliableCommand(va("nextdl %d", clc.downloadBlock), false);
	clc.downloadBlock++;

	clc.downloadCount += size;

	// So UI gets access to it
	CvarSystem::setValue( "cl_downloadCount", clc.downloadCount );

	if (!size) { // A zero length block means EOF
		if (clc.download) {
			FileSystem::fCloseFile( clc.download );
			clc.download = 0;

			// rename the file
			FileSystem::baseDir_Rename_HomeData ( clc.downloadTempName, clc.downloadName, false );
		}

		// send intentions now
		// We need this because without it, we would hold the last nextdl and then start
		// loading right away.  If we take a while to load, the server is happily trying
		// to send us that last block over and over.
		// Write it twice to help make sure we acknowledge the download
		Client::writePacket();
		Client::writePacket();

		// get another file if needed
		Client::nextDownload ();
	}
}

#ifdef USE_VOIP
bool Client::shouldIgnoreVoipSender(int sender)
{
	if (!cl_voip->integer)
		return true;  // VoIP is disabled.
	else if ((sender == clc.clientNum) && (!clc.demoplaying))
		return true;  // ignore own voice (unless playing back a demo).
	else if (clc.voipMuteAll)
		return true;  // all channels are muted with extreme prejudice.
	else if (clc.voipIgnore[sender])
		return true;  // just ignoring this guy.
	else if (clc.voipGain[sender] == 0.0f)
		return true;  // too quiet to play.

	return false;
}

/*
=====================
Client::playVoip

Play raw data
=====================
*/

void Client::playVoip(int sender, int samplecnt, const byte *data, int flags)
{
	if(flags & VOIP_DIRECT)
	{
		SoundSystem::rawSamples(sender + 1, samplecnt, 48000, 2, 1,
	             data, clc.voipGain[sender], -1);
	}

	if(flags & VOIP_SPATIAL)
	{
		SoundSystem::rawSamples(sender + MAX_CLIENTS + 1, samplecnt, 48000, 2, 1,
	             data, 1.0f, sender);
	}
}

/*
=====================
Client::parseVoip

A VoIP message has been received from the server
=====================
*/
void Client::parseVoip( Message *msg, bool ignoreData ) {
	static short decoded[VOIP_MAX_PACKET_SAMPLES*4]; // !!! FIXME: don't hard code

	const int sender = msg->readShort();
	const int generation = msg->readByte();
	const int sequence = msg->readLong();
	const int frames = msg->readByte();
	const int packetsize = msg->readShort();
	const int flags = msg->readBits( VOIP_FLAGCNT);
	unsigned char encoded[4000];
	int	numSamples;
	int seqdiff;
	int written = 0;
	int i;

	Common::dPrintf("VoIP: %d-byte packet from client %d\n", packetsize, sender);

	if (sender < 0)
		return;   // short/invalid packet, bail.
	else if (generation < 0)
		return;   // short/invalid packet, bail.
	else if (sequence < 0)
		return;   // short/invalid packet, bail.
	else if (frames < 0)
		return;   // short/invalid packet, bail.
	else if (packetsize < 0)
		return;   // short/invalid packet, bail.

	if (packetsize > sizeof (encoded)) {  // overlarge packet?
		int bytesleft = packetsize;
		while (bytesleft) {
			int br = bytesleft;
			if (br > sizeof (encoded))
				br = sizeof (encoded);
			msg->readData( encoded, br);
			bytesleft -= br;
		}
		return;   // overlarge packet, bail.
	}

	msg->readData( encoded, packetsize);

	if (ignoreData) {
		return; // just ignore legacy speex voip data
	} else if (!clc.voipCodecInitialized) {
		return;   // can't handle VoIP without libopus!
	} else if (sender >= MAX_CLIENTS) {
		return;   // bogus sender.
	} else if (Client::shouldIgnoreVoipSender(sender)) {
		return;   // Channel is muted, bail.
	}

	// !!! FIXME: make sure data is narrowband? Does decoder handle this?

	Common::dPrintf("VoIP: packet accepted!\n");

	seqdiff = sequence - clc.voipIncomingSequence[sender];

	// This is a new "generation" ... a new recording started, reset the bits.
	if (generation != clc.voipIncomingGeneration[sender]) {
		Common::dPrintf("VoIP: new generation %d!\n", generation);
		opus_decoder_ctl(clc.opusDecoder[sender], OPUS_RESET_STATE);
		clc.voipIncomingGeneration[sender] = generation;
		seqdiff = 0;
	} else if (seqdiff < 0) {   // we're ahead of the sequence?!
		// This shouldn't happen unless the packet is corrupted or something.
		Common::dPrintf("VoIP: misordered sequence! %d < %d!\n",
		            sequence, clc.voipIncomingSequence[sender]);
		// reset the decoder just in case.
		opus_decoder_ctl(clc.opusDecoder[sender], OPUS_RESET_STATE);
		seqdiff = 0;
	} else if (seqdiff * VOIP_MAX_PACKET_SAMPLES*2 >= sizeof (decoded)) { // dropped more than we can handle?
		// just start over.
		Common::dPrintf("VoIP: Dropped way too many (%d) frames from client #%d\n",
		            seqdiff, sender);
		opus_decoder_ctl(clc.opusDecoder[sender], OPUS_RESET_STATE);
		seqdiff = 0;
	}

	if (seqdiff != 0) {
		Common::dPrintf("VoIP: Dropped %d frames from client #%d\n",
		            seqdiff, sender);
		// tell opus that we're missing frames...
		for (i = 0; i < seqdiff; i++) {
			assert((written + VOIP_MAX_PACKET_SAMPLES) * 2 < sizeof (decoded));
			numSamples = opus_decode(clc.opusDecoder[sender], nullptr, 0, decoded + written, VOIP_MAX_PACKET_SAMPLES, 0);
			if ( numSamples <= 0 ) {
				Common::dPrintf("VoIP: Error decoding frame %d from client #%d\n", i, sender);
				continue;
			}
			written += numSamples;
		}
	}

	numSamples = opus_decode(clc.opusDecoder[sender], encoded, packetsize, decoded + written, ARRAY_LEN(decoded) - written, 0);

	if ( numSamples <= 0 ) {
		Common::dPrintf("VoIP: Error decoding voip data from client #%d\n", sender);
		numSamples = 0;
	}

	#if 0
	static FILE *encio = NULL;
	if (encio == NULL) encio = fopen("voip-incoming-encoded.bin", "wb");
	if (encio != NULL) { fwrite(encoded, packetsize, 1, encio); fflush(encio); }
	static FILE *decio = NULL;
	if (decio == NULL) decio = fopen("voip-incoming-decoded.bin", "wb");
	if (decio != NULL) { fwrite(decoded+written, numSamples*2, 1, decio); fflush(decio); }
	#endif

	written += numSamples;

	Common::dPrintf("VoIP: playback %d bytes, %d samples, %d frames\n",
	            written * 2, written, frames);

	if(written > 0)
		Client::playVoip(sender, written, reinterpret_cast<const byte *>(decoded), flags);

	clc.voipIncomingSequence[sender] = sequence + frames;
}
#endif


/*
=====================
CL_ParseCommandString

Command strings are just saved off until cgame asks for them
when it transitions a snapshot
=====================
*/
void CL_ParseCommandString( Message *msg ) {
	char	*s;
	int		seq;
	int		index;

	seq = msg->readLong();
	s = msg->readString();

	// see if we have already executed stored it off
	if ( clc.serverCommandSequence >= seq ) {
		return;
	}
	clc.serverCommandSequence = seq;

	index = seq & (MAX_RELIABLE_COMMANDS-1);
	Q_strncpyz( clc.serverCommands[ index ], s, sizeof( clc.serverCommands[ index ] ) );
}


/*
=====================
Client::parseServerMessage
=====================
*/
void Client::parseServerMessage( Message *msg ) {
	int			cmd;

	if ( cl_shownet->integer == 1 ) {
		Common::printf ("%i ",msg->cursize);
	} else if ( cl_shownet->integer >= 2 ) {
		Common::printf ("------------------\n");
	}

	msg->bitstream();

	// get the reliable sequence acknowledge number
	clc.reliableAcknowledge = msg->readLong();
	// 
	if ( clc.reliableAcknowledge < clc.reliableSequence - MAX_RELIABLE_COMMANDS ) {
		clc.reliableAcknowledge = clc.reliableSequence;
	}

	//
	// parse the message
	//
	while ( true ) {
		if ( msg->readcount > msg->cursize ) {
			Common::error (ERR_DROP,"CL_ParseServerMessage: read past end of server message");
			break;
		}

		cmd = msg->readByte();

		if (cmd == svc_EOF) {
			SHOWNET( msg, "END OF MESSAGE" );
			break;
		}

		if ( cl_shownet->integer >= 2 ) {
			if ( (cmd < 0) || (!svc_strings[cmd]) ) {
				Common::printf( "%3i:BAD CMD %i\n", msg->readcount-1, cmd );
			} else {
				SHOWNET( msg, svc_strings[cmd] );
			}
		}
	
	// other commands
		switch ( cmd ) {
		default:
			Common::error (ERR_DROP,"CL_ParseServerMessage: Illegible server message");
			break;			
		case svc_nop:
			break;
		case svc_serverCommand:
			CL_ParseCommandString( msg );
			break;
		case svc_gamestate:
			CL_ParseGamestate( msg );
			break;
		case svc_snapshot:
			CL_ParseSnapshot( msg );
			break;
		case svc_download:
			CL_ParseDownload( msg );
			break;
		case svc_voipSpeex:
#ifdef USE_VOIP
			Client::parseVoip( msg, true );
#endif
			break;
		case svc_voipOpus:
#ifdef USE_VOIP
			Client::parseVoip( msg, !clc.voipEnabled );
#endif
			break;
		}
	}
}


