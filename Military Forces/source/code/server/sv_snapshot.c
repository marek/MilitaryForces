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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "server.h"
#include "sv_server.h"
#include "sv_client.h"


/*
=============================================================================

Delta encode a client frame onto the network channel

A normal server packet will look like:

4	sequence number (high bit set if an oversize fragment)
<optional reliable commands>
1	svc_snapshot
4	last client reliable command
4	serverTime
1	lastframe for delta compression
1	snapFlags
1	areaBytes
<areabytes>
<playerstate>
<packetentities>

=============================================================================
*/

/*
=============
SV_EmitPacketEntities

Writes a delta update of an entityState_t list to the message.
=============
*/
static void SV_EmitPacketEntities( SV_Client::ClientSnapshot *from, SV_Client::ClientSnapshot *to, msg_t *msg )
{
	entityState_t	*oldent, *newent;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;

	// generate the delta update
	if ( !from ) 
	{
		from_num_entities = 0;
	} 
	else 
	{
		from_num_entities = from->num_entities_;
	}

	newent = NULL;
	oldent = NULL;
	newindex = 0;
	oldindex = 0;
	while ( newindex < to->num_entities_ || oldindex < from_num_entities ) 
	{
		if ( newindex >= to->num_entities_ ) 
		{
			newnum = 9999;
		} 
		else 
		{
			newent = &theSVS.snapshotEntities_[(to->first_entity_+newindex) % theSVS.numSnapshotEntities_];
			newnum = newent->number;
		}

		if ( oldindex >= from_num_entities ) 
		{
			oldnum = 9999;
		} 
		else 
		{
			oldent = &theSVS.snapshotEntities_[(from->first_entity_+oldindex) % theSVS.numSnapshotEntities_];
			oldnum = oldent->number;
		}

		if ( newnum == oldnum ) 
		{
			// delta update from old position
			// because the force parm is false, this will not result
			// in any bytes being emited if the entity has not changed at all
			MSG_WriteDeltaEntity (msg, oldent, newent, false );
			oldindex++;
			newindex++;
			continue;
		}

		if ( newnum < oldnum ) 
		{
			// this is a New entity, send it from the baseline
			MSG_WriteDeltaEntity (msg, &theSV.serverEntities_[newnum].baseline_, newent, true );
			newindex++;
			continue;
		}

		if ( newnum > oldnum )
		{
			// the old entity isn't present in the New message
			MSG_WriteDeltaEntity (msg, oldent, NULL, true );
			oldindex++;
			continue;
		}
	}

	MSG_WriteBits( msg, (MAX_GENTITIES-1), GENTITYNUM_BITS );	// end of packetentities
}



/*
==================
SV_WriteSnapshotToClient
==================
*/
static void SV_WriteSnapshotToClient( SV_Client *client, msg_t *msg ) 
{
	SV_Client::ClientSnapshot	*frame, *oldframe;
	int					lastframe;
	int					i;
	int					snapFlags;

	// this is the snapshot we are creating
	frame = &client->frames_[ client->netchan_.outgoingSequence & PACKET_MASK ];

	// try to use a previous frame as the source for delta compressing the snapshot
	if ( client->deltaMessage_ <= 0 || client->state_ != SV_Client::CS_ACTIVE ) 
	{
		// client is asking for a retransmit
		oldframe = NULL;
		lastframe = 0;
	} 
	else if ( client->netchan_.outgoingSequence - client->deltaMessage_ 
		>= (PACKET_BACKUP - 3) ) 
	{
		// client hasn't gotten a good message through in a long time
		Com_DPrintf ("%s: Delta request from out of date packet.\n", client->name_);
		oldframe = NULL;
		lastframe = 0;
	} 
	else 
	{
		// we have a valid snapshot to delta from
		oldframe = &client->frames_[ client->deltaMessage_ & PACKET_MASK ];
		lastframe = client->netchan_.outgoingSequence - client->deltaMessage_;

		// the snapshot's entities may still have rolled off the buffer, though
		if ( oldframe->first_entity_ <= theSVS.nextSnapshotEntities_ - theSVS.numSnapshotEntities_ ) {
			Com_DPrintf ("%s: Delta request from out of date entities.\n", client->name_);
			oldframe = NULL;
			lastframe = 0;
		}
	}

	MSG_WriteByte (msg, svc_snapshot);

	// NOTE, MRE: now sent at the start of every message from server to client
	// let the client know which reliable clientCommands we have received
	//MSG_WriteLong( msg, client->lastClientCommand );

	// send over the current server time so the client can drift
	// its view of time to try to match
	MSG_WriteLong (msg, theSVS.time_);

	// what we are delta'ing from
	MSG_WriteByte (msg, lastframe);

	snapFlags = theSVS.snapFlagServerBit_;
	if ( client->rateDelayed_ ) 
	{
		snapFlags |= SNAPFLAG_RATE_DELAYED;
	}
	if ( client->state_ != SV_Client::CS_ACTIVE ) 
	{
		snapFlags |= SNAPFLAG_NOT_ACTIVE;
	}

	MSG_WriteByte (msg, snapFlags);

	// send over the areabits
	MSG_WriteByte (msg, frame->areabytes_);
	MSG_WriteData (msg, frame->areabits_, frame->areabytes_);

	// delta encode the playerstate
	if ( oldframe ) {
		MSG_WriteDeltaPlayerstate( msg, &oldframe->ps_, &frame->ps_ );
	} else {
		MSG_WriteDeltaPlayerstate( msg, NULL, &frame->ps_ );
	}

	// delta encode the entities
	SV_EmitPacketEntities (oldframe, frame, msg);

	// padding for rate debugging
	if ( sv_padPackets->integer ) 
	{
		for ( i = 0 ; i < sv_padPackets->integer ; i++ ) 
		{
			MSG_WriteByte (msg, svc_nop);
		}
	}
}


/*
==================
SV_UpdateServerCommandsToClient

(re)send all server commands the client hasn't acknowledged yet
==================
*/
void SV_UpdateServerCommandsToClient( SV_Client *client, msg_t *msg ) 
{
	int		i;

	// write any unacknowledged serverCommands
	for ( i = client->reliableAcknowledge_ + 1 ; i <= client->reliableSequence_ ; i++ ) {
		MSG_WriteByte( msg, svc_serverCommand );
		MSG_WriteLong( msg, i );
		MSG_WriteString( msg, client->reliableCommands_[ i & (MAX_RELIABLE_COMMANDS-1) ] );
	}
	client->reliableSent_ = client->reliableSequence_;
}

/*
=============================================================================

Build a client snapshot structure

=============================================================================
*/

#define	MAX_SNAPSHOT_ENTITIES	1024
typedef struct {
	int		numSnapshotEntities;
	int		snapshotEntities[MAX_SNAPSHOT_ENTITIES];	
} snapshotEntityNumbers_t;

/*
=======================
SV_QsortEntityNumbers
=======================
*/
static int QDECL SV_QsortEntityNumbers( const void *a, const void *b ) {
	int	*ea, *eb;

	ea = (int *)a;
	eb = (int *)b;

	if ( *ea == *eb ) {
		Com_Error( ERR_DROP, "SV_QsortEntityStates: duplicated entity" );
	}

	if ( *ea < *eb ) {
		return -1;
	}

	return 1;
}


/*
===============
SV_AddEntToSnapshot
===============
*/
static void SV_AddEntToSnapshot( SV_Entity *svEnt, EntityBase* gEnt, snapshotEntityNumbers_t *eNums ) 
{
	// if we have already added this entity to this snapshot, don't add again
	if ( svEnt->snapshotCounter_ == theSV.snapshotCounter_ ) {
		return;
	}
	svEnt->snapshotCounter_ = theSV.snapshotCounter_;

	// if we are full, silently discard entities
	if ( eNums->numSnapshotEntities == MAX_SNAPSHOT_ENTITIES ) {
		return;
	}

	eNums->snapshotEntities[ eNums->numSnapshotEntities ] = gEnt->s.number;
	eNums->numSnapshotEntities++;
}

/*
===============
SV_AddEntitiesVisibleFromPoint
===============
*/
static void SV_AddEntitiesVisibleFromPoint( vec3_t origin, 
											SV_Client::ClientSnapshot *frame, 
											snapshotEntityNumbers_t *eNums, 
											bool portal ) 
{
	int		i;
	EntityBase *ent;
	SV_Entity	*svEnt;
	int		l;
	int		clientarea, clientcluster;
	int		leafnum;
	int		c_fullsend;
	byte	*clientpvs;
	byte	*bitvector;

	// during an error shutdown message we may need to transmit
	// the shutdown message after the server has shutdown, so
	// specfically check for it
	if ( !theSV.state_ ) 
		return;

	leafnum = CM_PointLeafnum (origin);
	clientarea = CM_LeafArea (leafnum);
	clientcluster = CM_LeafCluster (leafnum);

	// calculate the visible areas
	frame->areabytes_ = CM_WriteAreaBits( frame->areabits_, clientarea );

	clientpvs = CM_ClusterPVS (clientcluster);

	c_fullsend = 0;

	size_t numEnts = theSV.gameEntities_->size();
	for ( size_t e = 0 ; e < numEnts ; e++ ) 
	{
		ent = theSV.getEntityByNum(e);

		// never send entities that aren't linked in
		if ( !ent->r.linked ) 
			continue;

		if (ent->s.number != e) 
		{
			Com_DPrintf ("FIXING ENT->S.NUMBER!!!\n");
			ent->s.number = e;
		}

		// entities can be flagged to explicitly not be sent to the client
		if ( ent->r.svFlags & SVF_NOCLIENT ) 
			continue;

		// entities can be flagged to be sent to only one client
		if ( ent->r.svFlags & SVF_SINGLECLIENT ) 
		{
			if ( ent->r.singleClient != frame->ps_.clientNum ) 
				continue;
		}
		// entities can be flagged to be sent to everyone but one client
		if ( ent->r.svFlags & SVF_NOTSINGLECLIENT ) 
		{
			if ( ent->r.singleClient == frame->ps_.clientNum ) 
				continue;
		}
		// entities can be flagged to be sent to a given mask of clients
		if ( ent->r.svFlags & SVF_CLIENTMASK ) 
		{
			if (frame->ps_.clientNum >= 32)
				Com_Error( ERR_DROP, "SVF_CLIENTMASK: cientNum > 32\n" );
			if (~ent->r.singleClient & (1 << frame->ps_.clientNum))
				continue;
		}

		svEnt = theSV.SVEntityForGameEntity( ent );

		// don't double add an entity through portals
		if ( svEnt->snapshotCounter_ == theSV.snapshotCounter_ ) 
			continue;

		// broadcast entities are always sent
		if ( ent->r.svFlags & SVF_BROADCAST ) 
		{
			SV_AddEntToSnapshot( svEnt, ent, eNums );
			continue;
		}

		// ignore if not touching a PV leaf
		// check area
		if ( !CM_AreasConnected( clientarea, svEnt->areanum_ ) ) 
		{
			// doors can legally straddle two areas, so
			// we may need to check another one
			if ( !CM_AreasConnected( clientarea, svEnt->areanum2_ ) ) 
				continue;		// blocked by a door
		}

		bitvector = clientpvs;

		// check individual leafs
		if ( !svEnt->numClusters_ ) {
			continue;
		}
		l = 0;
		for ( i=0 ; i < svEnt->numClusters_ ; i++ ) 
		{
			l = svEnt->clusternums_[i];
			if ( bitvector[l >> 3] & (1 << (l&7) ) ) 
				break;
		}

		// if we haven't found it to be visible,
		// check overflow clusters that coudln't be stored
		if ( i == svEnt->numClusters_ ) 
		{
			if ( svEnt->lastCluster_ ) 
			{
				for ( ; l <= svEnt->lastCluster_ ; l++ ) 
				{
					if ( bitvector[l >> 3] & (1 << (l&7) ) ) 
						break;
				}
				if ( l == svEnt->lastCluster_ ) 
					continue;	// not visible
			} 
			else 
				continue;
		}

		// add it
		SV_AddEntToSnapshot( svEnt, ent, eNums );

		// if its a portal entity, add everything visible from its camera position
		if ( ent->r.svFlags & SVF_PORTAL ) 
		{
			if ( ent->s.generic1 ) 
			{
				vec3_t dir;
				VectorSubtract(ent->s.origin, origin, dir);
				if ( VectorLengthSquared(dir) > (float) ent->s.generic1 * ent->s.generic1 ) 
					continue;
			}
			SV_AddEntitiesVisibleFromPoint( ent->s.origin2, frame, eNums, true );
		}

	}
}

/*
=============
SV_BuildClientSnapshot

Decides which entities are going to be visible to the client, and
copies off the playerstate and areabits.

This properly handles multiple recursive portals, but the render
currently doesn't.

For viewing through other player's eyes, clent can be something other than client->gentity
=============
*/
static void SV_BuildClientSnapshot( SV_Client *client ) 
{
	vec3_t						org;
	SV_Client::ClientSnapshot	*frame;
	snapshotEntityNumbers_t		entityNumbers;
	int							i;
	EntityBase					*ent;
	entityState_t				*state;
	SV_Entity					*svEnt;
	EntityBase					*clent;
	int							clientNum;
	playerState_t				*ps;

	// bump the counter used to prevent double adding
	theSV.snapshotCounter_++;

	// this is the frame we are creating
	frame = &client->frames_[ client->netchan_.outgoingSequence & PACKET_MASK ];

	// clear everything in this snapshot
	entityNumbers.numSnapshotEntities = 0;
	Com_Memset( frame->areabits_, 0, sizeof( frame->areabits_ ) );

	frame->num_entities_ = 0;
	
	clent = theSV.getEntityByNum(client->clientNum_);//client->gentity_;
	if ( !clent || client->state_ == SV_Client::CS_ZOMBIE ) 
		return;

	// grab the current playerState_t
	ps = &theSV.getClientByNum( client->clientNum_ )->ps_;
	frame->ps_ = *ps;

	// never send client's own entity, because it can
	// be regenerated from the playerstate
	clientNum = frame->ps_.clientNum;
	if ( clientNum < 1 || clientNum >= MAX_GENTITIES ) 
	{
		Com_Error( ERR_DROP, "SV_BuildClientSnapshot: bad client" );
	}
	svEnt = &theSV.serverEntities_[ clientNum ];

	svEnt->snapshotCounter_ = theSV.snapshotCounter_;

	// find the client's viewpoint
	VectorCopy( ps->origin, org );
	org[2] += ps->viewheight;

	// add all the entities directly visible to the eye, which
	// may include portal entities that merge other viewpoints
	SV_AddEntitiesVisibleFromPoint( org, frame, &entityNumbers, false );

	// if there were portals visible, there may be out of order entities
	// in the list which will need to be resorted for the delta compression
	// to work correctly.  This also catches the error condition
	// of an entity being included twice.
	qsort( entityNumbers.snapshotEntities, entityNumbers.numSnapshotEntities, 
		sizeof( entityNumbers.snapshotEntities[0] ), SV_QsortEntityNumbers );

	// now that all viewpoint's areabits have been OR'd together, invert
	// all of them to make it a mask vector, which is what the renderer wants
	for ( i = 0 ; i < MAX_MAP_AREA_BYTES/4 ; i++ ) 
	{
		((int *)frame->areabits_)[i] = ((int *)frame->areabits_)[i] ^ -1;
	}

	// copy the entity states out
	frame->num_entities_ = 0;
	frame->first_entity_ = theSVS.nextSnapshotEntities_;
	for ( i = 0 ; i < entityNumbers.numSnapshotEntities ; i++ ) 
	{
		ent = theSV.getEntityByNum(entityNumbers.snapshotEntities[i]);
		state = &theSVS.snapshotEntities_[theSVS.nextSnapshotEntities_ % theSVS.numSnapshotEntities_];
		*state = ent->s;
		theSVS.nextSnapshotEntities_++;
		// this should never hit, map should always be restarted first in SV_Frame
		if ( theSVS.nextSnapshotEntities_ >= 0x7FFFFFFE ) 
			Com_Error(ERR_FATAL, "svs.nextSnapshotEntities wrapped");
		frame->num_entities_++;
	}
}


/*
====================
SV_RateMsec

Return the number of msec a given size message is supposed
to take to clear, based on the current rate
====================
*/
#define	HEADER_RATE_BYTES	48		// include our header, IP header, and some overhead
static int SV_RateMsec( SV_Client *client, int messageSize ) 
{
	int		rate;
	int		rateMsec;

	// individual messages will never be larger than fragment size
	if ( messageSize > 1500 ) 
		messageSize = 1500;

	rate = client->rate_;
	if ( sv_maxRate->integer ) 
	{
		if ( sv_maxRate->integer < 1000 ) 
			Cvar_Set( "sv_MaxRate", "1000" );
		if ( sv_maxRate->integer < rate ) 
			rate = sv_maxRate->integer;
	}
	rateMsec = ( messageSize + HEADER_RATE_BYTES ) * 1000 / rate;

	return rateMsec;
}

/*
=======================
SV_SendMessageToClient

Called by SV_SendClientSnapshot and SV_SendClientGameState
=======================
*/
void SV_SendMessageToClient( msg_t *msg, SV_Client *client ) 
{
	int			rateMsec;

	// record information about the message
	client->frames_[client->netchan_.outgoingSequence & PACKET_MASK].messageSize_ = msg->cursize;
	client->frames_[client->netchan_.outgoingSequence & PACKET_MASK].messageSent_ = theSVS.time_;
	client->frames_[client->netchan_.outgoingSequence & PACKET_MASK].messageAcked_ = -1;

	// send the datagram
	SV_Netchan_Transmit( client, msg );	//msg->cursize, msg->data );

	// set nextSnapshotTime based on rate and requested number of updates

	// local clients get snapshots every frame
	// added sv_lanForceRate check
	if ( client->netchan_.remoteAddress.type == NA_LOOPBACK || 
		(sv_lanForceRate->integer && Sys_IsLANAddress (client->netchan_.remoteAddress)) ) 
	{
		client->nextSnapshotTime_ = theSVS.time_ - 1;
		return;
	}
	
	// normal rate / snapshotMsec calculation
	rateMsec = SV_RateMsec( client, msg->cursize );

	if ( rateMsec < client->snapshotMsec_ ) 
	{
		// never send more packets than this, no matter what the rate is at
		rateMsec = client->snapshotMsec_;
		client->rateDelayed_ = false;
	} 
	else
	{
		client->rateDelayed_ = true;
	}

	client->nextSnapshotTime_ = theSVS.time_ + rateMsec;

	// don't pile up empty snapshots while connecting
	if ( client->state_ != SV_Client::CS_ACTIVE ) 
	{
		// a gigantic connection message may have already put the nextSnapshotTime
		// more than a second away, so don't shorten it
		// do shorten if client is downloading
		if ( !*client->downloadName_ && client->nextSnapshotTime_ < theSVS.time_ + 1000 ) 
			client->nextSnapshotTime_ = theSVS.time_ + 1000;
	}
}


/*
=======================
SV_SendClientSnapshot

Also called by SV_FinalMessage

=======================
*/
void SV_SendClientSnapshot( SV_Client *client ) 
{
	byte		msg_buf[MAX_MSGLEN];
	msg_t		msg;

	// build the snapshot
	SV_BuildClientSnapshot( client );

	// bots need to have their snapshots build, but
	// the query them directly without needing to be sent
	EntityBase* gentity = theSV.getEntityByNum(client->clientNum_);
	if( gentity && gentity->r.svFlags & SVF_BOT ) 
		return;

	MSG_Init (&msg, msg_buf, sizeof(msg_buf));
	msg.allowoverflow = true;

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand_ );

	// (re)send any reliable server commands
	SV_UpdateServerCommandsToClient( client, &msg );

	// send over all the relevant entityState_t
	// and the playerState_t
	SV_WriteSnapshotToClient( client, &msg );

	// Add any download data if the client is downloading
	SV_WriteDownloadToClient( client, &msg );

	// check for overflow
	if ( msg.overflowed ) 
	{
		Com_Printf ("WARNING: msg overflowed for %s\n", client->name_);
		MSG_Clear (&msg);
	}

	SV_SendMessageToClient( &msg, client );
}


/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages() 
{
	// send a message to each connected client
	for( int i = 1; i <= sv_maxclients->integer; i++ ) 
	{
		SV_Client* c = theSVS.svClients_.at(i);

		if( !c || !c->state_ ) 
			continue;		// not connected

		if( theSVS.time_ < c->nextSnapshotTime_ ) 
			continue;		// not time yet

		// send additional message fragments if the last message
		// was too large to send at once
		if( c->netchan_.unsentFragments ) 
		{
			c->nextSnapshotTime_ = theSVS.time_ + 
				SV_RateMsec( c, c->netchan_.unsentLength - c->netchan_.unsentFragmentStart );
			SV_Netchan_TransmitNextFragment( c );
			continue;
		}

		// generate and send a New message
		SV_SendClientSnapshot( c );
	}
}

