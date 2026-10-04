/*
 * $Id: cg_snapshot.c,v 1.5 2005-11-21 17:28:20 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_snapshot.c -- things that happen on snapshot transition,
// not necessarily every single rendered frame

#include "cg_local.h"



/*
==================
CG_ResetEntity
==================
*/
void ClientEntity::resetEntity()
{
	// if the previous snapshot this entity was updated in is at least
	// an event window back in time then we can reset the previous event
	if ( this->snapShotTime < cg.time - EVENT_VALID_MSEC )
	{
		this->previousEvent = 0;
	}

	this->trailTime = cg.snap->serverTime;
	this->errorTime = 0;

	VectorCopy (this->currentState.origin, this->lerpOrigin);
	VectorCopy (this->currentState.angles, this->lerpAngles);

	if ( this->currentState.eType == ET_PLAYER )
	{
		// 	CG_ResetPlayerEntity( this );
	}
}

/*
===============
CG_TransitionEntity

cent->nextState is moved to cent->currentState and events are fired
===============
*/
void ClientEntity::transitionEntity() {
	this->currentState = this->nextState;
	this->currentValid = true;

	// reset if the entity wasn't in the last frame or was teleported
	if ( !this->interpolate ) {
		this->resetEntity();
	}

	// clear the next state.  if will be set by the next CGame::setNextSnap
	this->interpolate = false;

	// check for events
	this->checkEvents();
}


/*
==================
CG_SetInitialSnapshot

This will only happen on the very first snapshot, or
on tourney restarts.  All other times will use 
CGame::transitionSnapshot instead.

FIXME: Also called by map_restart?
==================
*/
void CG_SetInitialSnapshot( snapshot_t *snap ) {
	int				i;
	ClientEntity		*cent;
	entityState_t	*state;

	cg.snap = snap;

	GameShared::playerStateToEntityState( &snap->ps, &cg_entities[ snap->ps.clientNum ].currentState, false );

	// sort out solid entities
	CGame::buildSolidList();

	CGame::executeNewServerCommands( snap->serverCommandSequence );

	// set our local weapon selection pointer to
	// what the server has indicated the current weapon is
	CGame::respawn();

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		state = &cg.snap->entities[ i ];
		cent = &cg_entities[ state->number ];

		memcpy(&cent->currentState, state, sizeof(entityState_t));
		//cent->currentState = *state;
		cent->interpolate = false;
		cent->currentValid = true;

		cent->resetEntity();

		// check for events
		cent->checkEvents();
	}
}


/*
===================
CGame::transitionSnapshot

The transition point from snap to nextSnap has passed
===================
*/
void CGame::transitionSnapshot( ) {
	ClientEntity			*cent;
	snapshot_t			*oldFrame;
	int					i;

	if ( !cg.snap ) {
		Common::error( ERR_DROP, "CG_TransitionSnapshot: NULL cg.snap" );
	}
	if ( !cg.nextSnap ) {
		Common::error( ERR_DROP, "CG_TransitionSnapshot: NULL cg.nextSnap" );
	}

	// execute any server string commands before transitioning entities
	CGame::executeNewServerCommands( cg.nextSnap->serverCommandSequence );

	// if we had a map_restart, set everthing with initial
	if ( !cg.snap ) {
	}

	// clear the currentValid flag for all entities in the existing snapshot
	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		cent->currentValid = false;
	}

	// move nextSnap to snap and do the transitions
	oldFrame = cg.snap;
	cg.snap = cg.nextSnap;

	GameShared::playerStateToEntityState( &cg.snap->ps, &cg_entities[ cg.snap->ps.clientNum ].currentState, false );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = false;

	for ( i = 0 ; i < cg.snap->numEntities ; i++ ) {
		cent = &cg_entities[ cg.snap->entities[ i ].number ];
		cent->transitionEntity();

		// remember time of snapshot this entity was last updated in
		cent->snapShotTime = cg.snap->serverTime;
	}

	cg.nextSnap = nullptr;

	// check for playerstate transition events
	if ( oldFrame ) {
		playerState_t	*ops, *ps;

		ops = &oldFrame->ps;
		ps = &cg.snap->ps;
		// teleporting checks are irrespective of prediction
		if ( ( ps->eFlags ^ ops->eFlags ) & EF_TELEPORT_BIT ) {
			cg.thisFrameTeleport = true;	// will be cleared by prediction code
		}

		// if we are not doing client side movement prediction for any
		// reason, then the client events and view changes will be issued now
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW)
			|| cg_nopredict.integer || cg_synchronousClients.integer ) {
			CGame::transitionPlayerState( ps, ops );
		}
	}

}


/*
===================
CGame::setNextSnap

A New snapshot has just been read in from the client system.
===================
*/
void CGame::setNextSnap( snapshot_t *snap ) {
	int					num;
	entityState_t		*es;
	ClientEntity			*cent;

	cg.nextSnap = snap;

	GameShared::playerStateToEntityState( &snap->ps, &cg_entities[ snap->ps.clientNum ].nextState, false );
	cg_entities[ cg.snap->ps.clientNum ].interpolate = true;

	// check for extrapolation errors
	for ( num = 0 ; num < snap->numEntities ; num++ ) {
		es = &snap->entities[num];
		cent = &cg_entities[ es->number ];

		memcpy(&cent->nextState, es, sizeof(entityState_t));
		//cent->nextState = *es;

		// if this frame is a teleport, or the entity wasn't in the
		// previous frame, don't interpolate
		if ( !cent->currentValid || ( ( cent->currentState.eFlags ^ es->eFlags ) & EF_TELEPORT_BIT )  ) {
			cent->interpolate = false;
		} else {
			cent->interpolate = true;

			if ( cg.snap && es->pos.trType == TR_LINEAR_STOP &&
				es->pos.trTime - cent->currentState.pos.trTime > 3 * ( snap->serverTime - cg.snap->serverTime ) / 2 ) {
				float	f = cent->entityAngleErrorFraction();
				int		i;

				for ( i = 0 ; i < 3 ; i++ ) {
					cent->errorAngles2[i] = AngleSubtract( cent->currentState.angles2[i], es->angles2[i] ) 
						+ ( f > 0 ? f * cent->errorAngles2[i] : 0 );
				}
				VectorCopy( cent->lerpAngles, cent->errorAngles );
				cent->errorTime = cg.time;
				cent->errorSnapTime = snap->serverTime;
			}
		}
	}

	// if the next frame is a teleport for the playerstate, we
	// can't interpolate during demos
	if ( cg.snap && ( ( snap->ps.eFlags ^ cg.snap->ps.eFlags ) & EF_TELEPORT_BIT ) ) {
		cg.nextFrameTeleport = true;
	} else {
		cg.nextFrameTeleport = false;
	}

	// if changing follow mode, don't interpolate
	if ( cg.nextSnap->ps.clientNum != cg.snap->ps.clientNum ) {
		cg.nextFrameTeleport = true;
	}

	// if changing server restarts, don't interpolate
	if ( ( cg.nextSnap->snapFlags ^ cg.snap->snapFlags ) & SNAPFLAG_SERVERCOUNT ) {
		cg.nextFrameTeleport = true;
	}

	// sort out solid entities
	CGame::buildSolidList();
}


/*
========================
CGame::readNextSnapshot

This is the only place New snapshots are requested
This may increment cgs.processedSnapshotNum multiple
times if the client system fails to return a
valid snapshot.
========================
*/
snapshot_t * CGame::readNextSnapshot( ) {
	int	r;
	snapshot_t	*dest;

	if ( cg.latestSnapshotNum > cgs.processedSnapshotNum + 1000 ) {
		CGame::printf( "WARNING: CG_ReadNextSnapshot: way out of range, %i > %i", 
			cg.latestSnapshotNum, cgs.processedSnapshotNum );
	}

	while ( cgs.processedSnapshotNum < cg.latestSnapshotNum ) {
		// decide which of the two slots to load it into
		if ( cg.snap == &cg.activeSnapshots[0] ) {
			dest = &cg.activeSnapshots[1];
		} else {
			dest = &cg.activeSnapshots[0];
		}

		// try to read the snapshot from the client system
		cgs.processedSnapshotNum++;
		r = ClientHost::getSnapshot( cgs.processedSnapshotNum, dest );

		// FIXME: why would ClientHost::getSnapshot return a snapshot with the same server time
		if ( cg.snap && r && dest->serverTime == cg.snap->serverTime ) {
			//continue;
		}

		// if it succeeded, return
		if ( r ) {
			CGame::addLagometerSnapshotInfo( dest );
			return dest;
		}

		// a GetSnapshot will return failure if the snapshot
		// never arrived, or  is so old that its entities
		// have been shoved off the end of the circular
		// buffer in the client system.

		// record as a dropped packet
		CGame::addLagometerSnapshotInfo( nullptr );

		// If there are additional snapshots, continue trying to
		// read them.
	}

	// nothing left to read
	return nullptr;
}


/*
============
CGame::processSnapshots

We are trying to set up a renderable view, so determine
what the simulated time is, and try to get snapshots
both before and after that time if available.

If we don't have a valid cg.snap after exiting this function,
then a 3D game view cannot be rendered.  This should only happen
right after the initial connection.  After cg.snap has been valid
once, it will never turn invalid.

Even if cg.snap is valid, cg.nextSnap may not be, if the snapshot
hasn't arrived yet (it becomes an extrapolating situation instead
of an interpolating one)

============
*/
void CGame::processSnapshots( ) {
	snapshot_t		*snap;
	int				n;

	// see what the latest snapshot the client system has is
	ClientHost::getCurrentSnapshotNumber( &n, &cg.latestSnapshotTime );
	if ( n != cg.latestSnapshotNum ) {
		if ( n < cg.latestSnapshotNum ) {
			// this should never happen
			Common::error( ERR_DROP, "CG_ProcessSnapshots: n < cg.latestSnapshotNum" );
		}
		cg.latestSnapshotNum = n;
	}

	// If we have yet to receive a snapshot, check for it.
	// Once we have gotten the first snapshot, cg.snap will
	// always have valid data for the rest of the game
	while ( !cg.snap ) {
		snap = CGame::readNextSnapshot();
		if ( !snap ) {
			// we can't continue until we get a snapshot
			return;
		}

		// set our weapon selection to what
		// the playerstate is currently using
		if ( !( snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
			CG_SetInitialSnapshot( snap );
		}
	}

	// loop until we either have a valid nextSnap with a serverTime
	// greater than cg.time to interpolate towards, or we run
	// out of available snapshots
	do {
		// if we don't have a nextframe, try and read a New one in
		if ( !cg.nextSnap ) {
			snap = CGame::readNextSnapshot();

			// if we still don't have a nextframe, we will just have to
			// extrapolate
			if ( !snap ) {
				break;
			}

			CGame::setNextSnap( snap );


			// if time went backwards, we have a level restart
			if ( cg.nextSnap->serverTime < cg.snap->serverTime ) {
				Common::error( ERR_DROP, "CG_ProcessSnapshots: Server time went backwards" );
			}
		}

		// if our time is < nextFrame's, we have a nice interpolating state
		if ( cg.time >= cg.snap->serverTime && cg.time < cg.nextSnap->serverTime ) {
			break;
		}

		// we have passed the transition from nextFrame to frame
		CGame::transitionSnapshot();
	} while ( true );

	// assert our valid conditions upon exiting
	if ( cg.snap == nullptr ) {
		Common::error( ERR_DROP, "CG_ProcessSnapshots: cg.snap == NULL" );
	}
	if ( cg.time < cg.snap->serverTime ) {
		// this can happen right after a vid_restart
		cg.time = cg.snap->serverTime;
	}
	if ( cg.nextSnap != nullptr && cg.nextSnap->serverTime <= cg.time ) {
		Common::error( ERR_DROP, "CG_ProcessSnapshots: cg.nextSnap->serverTime <= cg.time" );
	}

}

