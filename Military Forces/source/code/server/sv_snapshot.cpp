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

struct snapshotCandidate_t {
	int		number;
	int		bits;
	float	priority;
};

static int SV_EntityUpdateBits( entityState_t *from, entityState_t *to, bool create, uint64_t forced ) 
{
	byte	buf[1024];
	Message	msg;

	msg.init( buf, sizeof( buf ) );
	msg_huffstatsHold++;
	msg.writeEntityUpdate( from, to, create, forced, nullptr );
	msg_huffstatsHold--;
	return msg.bit;
}

static int QDECL SV_QsortSnapshotCandidates( const void *a, const void *b ) 
{
	const snapshotCandidate_t	*ca = static_cast<const snapshotCandidate_t *>(a);
	const snapshotCandidate_t	*cb = static_cast<const snapshotCandidate_t *>(b);

	if ( ca->priority > cb->priority ) {
		return -1;
	}
	if ( ca->priority < cb->priority ) {
		return 1;
	}
	return ca->number - cb->number;
}

static float SV_SnapshotEntityWeight( const playerState_t *ps, const vec3_t forward, 
									  entityState_t *state, entityState_t *known ) 
{
	EntityBase	*ent;
	vec3_t		dir;
	float		dist;
	float		weight;

	if ( state->eType >= ET_EVENTS ) {
		return 8.0f;
	}

	switch ( state->eType ) {
	case ET_PLAYER:
	case ET_VEHICLE:
	case ET_MISSILE:
		weight = 2.0f;
		break;
	case ET_MISC_VEHICLE:
	case ET_BULLET:
		weight = 1.5f;
		break;
	case ET_ITEM:
	case ET_EXPLOSIVE:
		weight = 0.5f;
		break;
	default:
		weight = 1.0f;
		break;
	}

	if ( !known ) {
		weight *= 2.0f;
	} else if ( known->event != state->event ) {
		weight *= 4.0f;
	}

	ent = theSV.getEntityByNum( state->number );
	if ( ent ) {
		VectorSubtract( ent->r.currentOrigin, ps->origin, dir );
		dist = VectorNormalize( dir );
		weight *= 0.25f + 4096.0f / ( 4096.0f + dist );
		if ( DotProduct( dir, forward ) > 0.5f ) {
			weight *= 1.5f;
		}
	}

	return weight;
}

SnapshotEntityTable * SV_Client::entityTable() 
{
	if ( !this->entityTable_ ) {
		this->entityTable_ = new SnapshotEntityTable;
		Com_Memset( this->entityTable_, 0, sizeof( *this->entityTable_ ) );
	}
	return this->entityTable_;
}

/*
=============
SV_SnapshotFrameLost

The client never applied this snapshot, so what it carried is sent again
=============
*/
void SV_Client::snapshotFrameLost( SnapshotEntityTable::SentFrame *sent ) 
{
	SnapshotEntityTable				*table = this->entityTable_;
	SnapshotEntityTable::SentEntity	*ent;
	int								i;

	sent->pending = false;

	if ( sent->overflow ) {
		this->entityReset_ = true;
		return;
	}

	for ( i = 0, ent = sent->entities ; i < sent->count ; i++, ent++ ) {
		switch ( ent->type ) {
		case SENT_CREATE:
			table->flags[ent->number] = ENTITY_REMOVE;
			break;
		case SENT_UPDATE:
			table->forced[ent->number] |= ent->mask;
			table->priority[ent->number] += 4.0f;
			break;
		case SENT_REMOVE:
			if ( !( table->flags[ent->number] & ENTITY_KNOWN ) ) {
				table->flags[ent->number] |= ENTITY_REMOVE;
			}
			break;
		}
	}
}

/*
=============
SV_SnapshotAcks

Sorts the snapshots sent so far into applied and lost ones from the
client's last report
=============
*/
void SV_Client::snapshotAcks() 
{
	SnapshotEntityTable				*table = this->entityTable_;
	SnapshotEntityTable::SentFrame	*sent;
	int								i, behind;

	if ( this->entityAck_ <= this->gamestateMessageNum_ 
		|| this->entityAck_ >= this->netchan_.outgoingSequence ) {
		return;
	}

	for ( i = 0, sent = table->frames ; i < PACKET_BACKUP ; i++, sent++ ) {
		if ( !sent->pending ) {
			continue;
		}
		behind = this->entityAck_ - sent->sequence;
		if ( behind < 0 ) {
			continue;
		}
		if ( behind == 0 || ( behind <= 32 && ( ( this->entityAckBits_ >> ( behind - 1 ) ) & 1 ) ) ) {
			sent->pending = false;
		} else {
			this->snapshotFrameLost( sent );
		}
	}
}

/*
=============
SV_SnapshotEntityReset

Returns true if this snapshot has to carry every entity
=============
*/
bool SV_Client::snapshotEntityReset( bool requested ) 
{
	SnapshotEntityTable	*table = this->entityTable();
	int					i;

	this->snapshotAcks();

	if ( !requested ) {
		this->entityResetDone_ = false;
	} else if ( !this->entityResetDone_ ) {
		this->entityReset_ = true;
	}

	if ( !this->entityReset_ ) {
		return false;
	}

	this->entityReset_ = false;
	this->entityResetDone_ = requested;

	Com_Memset( table->flags, 0, sizeof( table->flags ) );
	Com_Memset( table->forced, 0, sizeof( table->forced ) );
	Com_Memset( table->priority, 0, sizeof( table->priority ) );
	for ( i = 0 ; i < PACKET_BACKUP ; i++ ) {
		table->frames[i].pending = false;
	}
	return true;
}

/*
=============
SV_EmitPacketEntities

Writes the entity updates of a snapshot. Every update is sent once and the
client reports which snapshots it applied; what a lost snapshot carried is
sent again. Without the reset the snapshot is kept under sv_snapshotBudget
bytes: updates that do not fit wait and gain priority.
=============
*/
void SV_Client::emitPacketEntities( SV_Client::ClientSnapshot *frame, Message *msg, bool reset )
{
	static entityState_t		*states[MAX_GENTITIES];
	static byte					action[MAX_GENTITIES];
	static snapshotCandidate_t	candidates[MAX_SNAPSHOT_ENTITIES];
	SnapshotEntityTable			*table = this->entityTable_;
	SnapshotEntityTable::SentFrame	*sent;
	entityState_t				*state;
	entityState_t				removed;
	vec3_t						forward;
	uint64_t					mask;
	bool						unlimited;
	int							numCandidates;
	int							remaining;
	int							bits;
	int							num;
	int							i;

	unlimited = reset || sv_snapshotBudget->integer <= 0 
		|| this->netchan_.remoteAddress.type == NA_LOOPBACK 
		|| ( sv_lanForceRate->integer && Sys_IsLANAddress( this->netchan_.remoteAddress ) );

	remaining = sv_snapshotBudget->integer * 8 - msg->bit - GENTITYNUM_BITS - 8;

	AngleVectors( frame->ps_.viewangles, forward, nullptr, nullptr );

	numCandidates = 0;
	for ( i = 0 ; i < frame->num_entities_ ; i++ ) {
		state = &theSVS.snapshotEntities_[(frame->first_entity_+i) % theSVS.numSnapshotEntities_];
		num = state->number;
		states[num] = state;

		if ( !( table->flags[num] & ENTITY_KNOWN ) ) {
			action[num] = 1 + SENT_CREATE;
			bits = SV_EntityUpdateBits( &theSV.serverEntities_[num].baseline_, state, true, 0 );
		} else {
			bits = SV_EntityUpdateBits( &table->sent[num], state, false, table->forced[num] );
			if ( !bits ) {
				table->priority[num] = 0;
				continue;
			}
			action[num] = 1 + SENT_UPDATE;
		}

		if ( unlimited || numCandidates == MAX_SNAPSHOT_ENTITIES ) {
			continue;
		}

		table->priority[num] += SV_SnapshotEntityWeight( &frame->ps_, forward, state, 
			( table->flags[num] & ENTITY_KNOWN ) ? &table->sent[num] : nullptr );
		candidates[numCandidates].number = num;
		candidates[numCandidates].bits = bits;
		candidates[numCandidates].priority = table->priority[num];
		numCandidates++;
	}

	for ( num = 0 ; num < MAX_GENTITIES ; num++ ) {
		if ( table->flags[num] && !states[num] ) {
			action[num] = 1 + SENT_REMOVE;
			remaining -= GENTITYNUM_BITS + 2;
		}
	}

	if ( remaining < 200 * 8 ) {
		remaining = 200 * 8;
	}

	qsort( candidates, numCandidates, sizeof( candidates[0] ), SV_QsortSnapshotCandidates );

	for ( i = 0 ; i < numCandidates ; i++ ) {
		if ( candidates[i].bits <= remaining ) {
			remaining -= candidates[i].bits;
		} else {
			action[candidates[i].number] = 0;
		}
	}

	sent = &table->frames[ this->netchan_.outgoingSequence & PACKET_MASK ];
	if ( sent->pending ) {
		this->snapshotFrameLost( sent );
	}
	sent->sequence = this->netchan_.outgoingSequence;
	sent->pending = true;
	sent->overflow = reset;
	sent->count = 0;

	for ( num = 0 ; num < MAX_GENTITIES ; num++ ) {
		if ( !action[num] ) {
			continue;
		}
		state = states[num];
		mask = 0;

		switch ( action[num] - 1 ) {
		case SENT_UPDATE:
			msg->writeEntityUpdate( &table->sent[num], state, false, table->forced[num], &mask );
			table->sent[num] = *state;
			break;
		case SENT_CREATE:
			msg->writeEntityUpdate( &theSV.serverEntities_[num].baseline_, state, true, 0, nullptr );
			table->sent[num] = *state;
			table->flags[num] = ENTITY_KNOWN;
			break;
		case SENT_REMOVE:
			removed.number = num;
			msg->writeEntityUpdate( &removed, nullptr, false, 0, nullptr );
			table->flags[num] = 0;
			break;
		}
		table->forced[num] = 0;
		table->priority[num] = 0;

		if ( sent->count < MAX_SENT_ENTITIES ) {
			sent->entities[sent->count].number = num;
			sent->entities[sent->count].type = action[num] - 1;
			sent->entities[sent->count].mask = mask;
			sent->count++;
		} else {
			sent->overflow = true;
		}
		action[num] = 0;
	}

	for ( i = 0 ; i < frame->num_entities_ ; i++ ) {
		state = &theSVS.snapshotEntities_[(frame->first_entity_+i) % theSVS.numSnapshotEntities_];
		states[state->number] = nullptr;
	}

	msg->writeBits( ENTITYNUM_WIRE_END, GENTITYNUM_BITS );	// end of packetentities
}

/*
==================
SV_WriteSnapshotToClient
==================
*/
void SV_Client::writeSnapshotToClient( Message *msg ) 
{
	SV_Client::ClientSnapshot	*frame, *oldframe;
	int					lastframe;
	int					i;
	int					snapFlags;
	bool				reset;

	// this is the snapshot we are creating
	frame = &this->frames_[ this->netchan_.outgoingSequence & PACKET_MASK ];

	// try to use a previous frame as the source for delta compressing the snapshot
	if ( this->deltaMessage_ <= 0 || this->state_ != SV_Client::CS_ACTIVE ) 
	{
		// this is asking for a retransmit
		oldframe = nullptr;
		lastframe = 0;
	} 
	else if ( this->netchan_.outgoingSequence - this->deltaMessage_ 
		>= (PACKET_BACKUP - 3) ) 
	{
		// this hasn't gotten a good message through in a long time
		Common::dPrintf ("%s: Delta request from out of date packet.\n", this->name_);
		oldframe = nullptr;
		lastframe = 0;
	} 
	else 
	{
		// we have a valid snapshot to delta from
		oldframe = &this->frames_[ this->deltaMessage_ & PACKET_MASK ];
		lastframe = this->netchan_.outgoingSequence - this->deltaMessage_;

	}

	msg->writeByte( svc_snapshot);

	// NOTE, MRE: now sent at the start of every message from server to this
	// let the this know which reliable clientCommands we have received
	//msg->writeLong( this->lastClientCommand );

	// send over the current server time so the this can drift
	// its view of time to try to match
	if( this->oldServerTime_ ) {
		// The server has not yet got an acknowledgement of the
		// new gamestate from this this, so continue to send it
		// a time as if the server has not restarted. Note from
		// the this's perspective this time is strictly speaking
		// incorrect, but since it'll be busy loading a map at
		// the time it doesn't really matter.
		msg->writeLong( theSV.time_ + this->oldServerTime_);
	} else {
		msg->writeLong( theSV.time_);
	}

	// what we are delta'ing from
	msg->writeByte( lastframe);

	snapFlags = theSVS.snapFlagServerBit_;
	if ( this->rateDelayed_ ) 
	{
		snapFlags |= SNAPFLAG_RATE_DELAYED;
	}
	if ( this->state_ != SV_Client::CS_ACTIVE ) 
	{
		snapFlags |= SNAPFLAG_NOT_ACTIVE;
	}

	reset = this->snapshotEntityReset( this->deltaMessage_ <= 0 || this->state_ != SV_Client::CS_ACTIVE );
	if ( reset ) 
	{
		snapFlags |= SNAPFLAG_ENTITY_RESET;
	}

	msg->writeByte( snapFlags);

	// send over the areabits
	msg->writeByte( frame->areabytes_);
	msg->writeData( frame->areabits_, frame->areabytes_);

	// delta encode the playerstate
	if ( oldframe ) {
		msg->writeDeltaPlayerstate( &oldframe->ps_, &frame->ps_ );
	} else {
		msg->writeDeltaPlayerstate( nullptr, &frame->ps_ );
	}

	// delta encode the entities
	this->emitPacketEntities( frame, msg, reset );

	// padding for rate debugging
	if ( sv_padPackets->integer ) 
	{
		for ( i = 0 ; i < sv_padPackets->integer ; i++ ) 
		{
			msg->writeByte( svc_nop);
		}
	}
}


/*
==================
re->updateServerCommandsToClient()send all server commands the client hasn't acknowledged yet
==================
*/
void SV_Client::updateServerCommandsToClient( Message *msg ) 
{
	int		i;

	// write any unacknowledged serverCommands
	for ( i = this->reliableAcknowledge_ + 1 ; i <= this->reliableSequence_ ; i++ ) {
		msg->writeByte( svc_serverCommand );
		msg->writeLong( i );
		msg->writeString( this->reliableCommands_[ i & (MAX_RELIABLE_COMMANDS-1) ] );
	}
	this->reliableSent_ = this->reliableSequence_;
}

/*
=============================================================================

Build a client snapshot structure

=============================================================================
*/

struct snapshotEntityNumbers_t {
	int		numSnapshotEntities;
	int		snapshotEntities[MAX_SNAPSHOT_ENTITIES];	
};

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
		Common::error( ERR_DROP, "SV_QsortEntityStates: duplicated entity" );
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

	leafnum = CollisionModel::pointLeafnum (origin);
	clientarea = CollisionModel::leafArea (leafnum);
	clientcluster = CollisionModel::leafCluster (leafnum);

	// calculate the visible areas
	frame->areabytes_ = CollisionModel::writeAreaBits( frame->areabits_, clientarea );

	clientpvs = CollisionModel::clusterPVS (clientcluster);

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
			Common::dPrintf ("FIXING ENT->S.NUMBER!!!\n");
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
				Common::error( ERR_DROP, "SVF_CLIENTMASK: cientNum > 32\n" );
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
		if ( !CollisionModel::areasConnected( clientarea, svEnt->areanum_ ) ) 
		{
			// doors can legally straddle two areas, so
			// we may need to check another one
			if ( !CollisionModel::areasConnected( clientarea, svEnt->areanum2_ ) ) 
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
				if ( VectorLengthSquared(dir) > static_cast<float>(ent->s.generic1) * ent->s.generic1 ) 
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
void SV_Client::buildClientSnapshot() 
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
	frame = &this->frames_[ this->netchan_.outgoingSequence & PACKET_MASK ];

	// clear everything in this snapshot
	entityNumbers.numSnapshotEntities = 0;
	Com_Memset( frame->areabits_, 0, sizeof( frame->areabits_ ) );

	frame->num_entities_ = 0;
	
	clent = theSV.getEntityByNum(this->clientNum_);//this->gentity_;
	if ( !clent || this->state_ == SV_Client::CS_ZOMBIE ) 
		return;

	// grab the current playerState_t
	ps = &theSV.getClientByNum( this->clientNum_ )->ps_;
	frame->ps_ = *ps;

	// never send this's own entity, because it can
	// be regenerated from the playerstate
	clientNum = frame->ps_.clientNum;
	if ( clientNum < 1 || clientNum >= MAX_GENTITIES ) 
	{
		Common::error( ERR_DROP, "SV_BuildClientSnapshot: bad this" );
	}
	svEnt = &theSV.serverEntities_[ clientNum ];

	svEnt->snapshotCounter_ = theSV.snapshotCounter_;

	// find the this's viewpoint
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
		(reinterpret_cast<int *>(frame->areabits_))[i] = (reinterpret_cast<int *>(frame->areabits_))[i] ^ -1;
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
			Common::error(ERR_FATAL, "svs.nextSnapshotEntities wrapped");
		frame->num_entities_++;
	}
}


#ifdef USE_VOIP
/*
==================
SV_WriteVoipToClient

Check to see if there is any VoIP queued for a client, and send if there is.
==================
*/
void SV_Client::writeVoipToClient( Message *msg )
{
	int totalbytes = 0;
	int i;
	voipServerPacket_t *packet;

	if(this->queuedVoipPackets_)
	{
		// Write as many VoIP packets as we reasonably can...
		for(i = 0; i < this->queuedVoipPackets_; i++)
		{
			packet = this->voipPacket_[(i + this->queuedVoipIndex_) % ARRAY_LEN(this->voipPacket_)];

			if(!*this->downloadName_)
			{
        			totalbytes += packet->len;
	        		if (totalbytes > (msg->maxsize - msg->cursize) / 2)
		        		break;

        			msg->writeByte( svc_voipOpus);
        			msg->writeShort( packet->sender);
	        		msg->writeByte( static_cast<byte>(packet->generation));
		        	msg->writeLong( packet->sequence);
		        	msg->writeByte( packet->frames);
        			msg->writeShort( packet->len);
        			msg->writeBits( packet->flags, VOIP_FLAGCNT);
	        		msg->writeData( packet->data, packet->len);
                        }

			Z_Free(packet);
		}

		this->queuedVoipPackets_ -= i;
		this->queuedVoipIndex_ += i;
		this->queuedVoipIndex_ %= ARRAY_LEN(this->voipPacket_);
	}
}
#endif

/*
=======================
SV_SendMessageToClient

Called by SV_SendClientSnapshot and SV_SendClientGameState
=======================
*/
void SV_Client::sendMessageToClient( Message *msg )
{
	// record information about the message
	this->frames_[this->netchan_.outgoingSequence & PACKET_MASK].messageSize_ = msg->cursize;
	this->frames_[this->netchan_.outgoingSequence & PACKET_MASK].messageSent_ = theSVS.time_;
	this->frames_[this->netchan_.outgoingSequence & PACKET_MASK].messageAcked_ = -1;

	// send the datagram
	this->netchanTransmit( msg );
}


/*
=======================
SV_SendClientSnapshot

Also called by ServerMain::finalMessage

=======================
*/
void SV_Client::sendClientSnapshot() 
{
	byte		msg_buf[MAX_MSGLEN];
	Message		msg;

	// build the snapshot
	this->buildClientSnapshot();

	// bots need to have their snapshots build, but
	// the query them directly without needing to be sent
	EntityBase* gentity = theSV.getEntityByNum(this->clientNum_);
	if( gentity && gentity->r.svFlags & SVF_BOT ) 
		return;

	msg.init( msg_buf, sizeof(msg_buf));
	msg.allowoverflow = true;

	// NOTE, MRE: all server->client messages now acknowledge
	// let the this know which reliable clientCommands we have received
	msg.writeLong( this->lastClientCommand_ );

	// (re)send any reliable server commands
	this->updateServerCommandsToClient( &msg );

	// send over all the relevant entityState_t
	// and the playerState_t
	this->writeSnapshotToClient( &msg );

#ifdef USE_VOIP
	this->writeVoipToClient( &msg );
#endif

	// check for overflow
	if ( msg.overflowed ) 
	{
		Common::printf ("WARNING: msg overflowed for %s\n", this->name_);
		msg.clear();
	}

	this->sendMessageToClient( &msg );
}


/*
=======================
ServerMain::sendClientMessages
=======================
*/
void ServerMain::sendClientMessages()
{
	// send a message to each connected client
	for( int i = 1; i <= sv_maxclients->integer; i++ ) 
	{
		SV_Client* c = theSVS.svClients_.at(i);
		
		if(!c || !c->state_)
			continue;		// not connected

		if(theSVS.time_ - c->lastSnapshotTime_ < c->snapshotMsec_ * com_timescale->value)
			continue;		// It's not time yet

		if(*c->downloadName_)
			continue;		// Client is downloading, don't send snapshots

		if(c->netchan_.unsentFragments || c->netchan_start_queue_)
		{
			c->rateDelayed_ = true;
			continue;		// Drop this snapshot if the packet queue is still full or delta compression will break
		}

		if(!(c->netchan_.remoteAddress.type == NA_LOOPBACK ||
		     (sv_lanForceRate->integer && Sys_IsLANAddress(c->netchan_.remoteAddress))))
		{
			// rate control for clients not on LAN 
			if(c->rateMsec() > 0)
			{
				// Not enough time since last packet passed through the line
				c->rateDelayed_ = true;
				continue;
			}
		}

		// generate and send a new message
		c->sendClientSnapshot();
		c->lastSnapshotTime_ = theSVS.time_;
		c->rateDelayed_ = false;
	}
}
