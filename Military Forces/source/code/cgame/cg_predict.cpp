/*
 * $Id: cg_predict.c,v 1.6 2006-01-29 14:03:40 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_predict.c -- this file generates cg.predictedPlayerState by either
// interpolating between snapshots from the server or locally predicting
// ahead the client's movement.
// It also handles local physics interaction, like fragments bouncing off walls

#include "cg_local.h"

pmove_t CGame::cg_pmove;

int CGame::cg_numSolidEntities;
ClientEntity *CGame::cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
int CGame::cg_numTriggerEntities;
ClientEntity *CGame::cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];

/*
====================
CGame::buildSolidList

When a New cg.snap has been set, this function builds a sublist
of the entities that are actually solid, to make for more
efficient collision detection
====================
*/
void CGame::buildSolidList( ) {
	int			i;
	ClientEntity	*cent;
	snapshot_t	*snap;
	entityState_t	*ent;

	cg_numSolidEntities = 0;
	cg_numTriggerEntities = 0;

	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		snap = cg.nextSnap;
	} else {
		snap = cg.snap;
	}

	for ( i = 0 ; i < snap->numEntities ; i++ ) {
		cent = &cg_entities[ snap->entities[ i ].number ];
		ent = &cent->currentState;

		if ( ent->eType == ET_ITEM || ent->eType == ET_TELEPORT_TRIGGER ) {
			cg_triggerEntities[cg_numTriggerEntities] = cent;
			cg_numTriggerEntities++;
			continue;
		}

		if ( cent->nextState.solid ) {
			cg_solidEntities[cg_numSolidEntities] = cent;
			cg_numSolidEntities++;
			continue;
		}
	}
}

/*
====================
CGame::clipMoveToEntities

====================
*/
void CGame::clipMoveToEntities( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end,
							int skipNumber, int mask, trace_t *tr ) {
	int			i, x, zd, zu;
	trace_t		trace;
	entityState_t	*ent;
	clipHandle_t 	cmodel;
	Vec3		bmins, bmaxs;
	Vec3		origin, angles;
	ClientEntity	*cent;

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];
		ent = &cent->currentState;

		if ( ent->number == skipNumber ) {
			continue;
		}
		if ( (ent->seatMask & SEATMASK_DECK) && ent->otherEntityNum == skipNumber ) {
			continue;
		}
		if ( ent->vehicleEnt == skipNumber && skipNumber > 0 && ent->seat == 0 ) {
			continue;
		}

		if ( ent->solid == SOLID_BMODEL ) {
			// special value for bmodel
			cmodel = CollisionModel::inlineModel( ent->modelindex );
			angles = cent->lerpAngles;
			GameShared::evaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin );
		} else {
			// encoded bbox
			x = (ent->solid & 255);
			zd = ((ent->solid>>8) & 255);
			zu = ((ent->solid>>16) & 255) - 32;

			bmins[0] = bmins[1] = -x;
			bmaxs[0] = bmaxs[1] = x;
			bmins[2] = -zd;
			bmaxs[2] = zu;

			cmodel = CollisionModel::tempBoxModel( bmins, bmaxs, false );
			if ( ent->seatMask & SEATMASK_DECK ) {
				GameShared::evaluateTrajectory( &cent->currentState.apos, cg.physicsTime, angles );
				GameShared::evaluateTrajectory( &cent->currentState.pos, cg.physicsTime, origin );
			} else {
				VectorCopy( vec3_origin, angles );
				origin = cent->lerpOrigin;
			}
		}


		CollisionModel::transformedBoxTrace ( &trace, start, end,
			const_cast<float *>(mins), const_cast<float *>(maxs), cmodel,  mask, origin, angles, false);

		if (trace.allsolid || trace.fraction < tr->fraction) {
			trace.entityNum = ent->number;
			*tr = trace;
		} else if (trace.startsolid) {
			tr->startsolid = true;
		}
		if ( tr->allsolid ) {
			return;
		}
	}
}

/*
================
CGame::trace
================
*/
void	CGame::trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, 
					 int skipNumber, int mask, bool capsule ) 
{
	trace_t	t;

	CollisionModel::boxTrace ( &t, start, end, const_cast<float *>(mins), const_cast<float *>(maxs), 0, mask, capsule);
	t.entityNum = t.fraction != 1.0 ? ENTITYNUM_WORLD : ENTITYNUM_NONE;
	// check all other solid models
	CGame::clipMoveToEntities (start, mins, maxs, end, skipNumber, mask, &t);

	*result = t;
}

/*
================
CGame::pointContents
================
*/
int		CGame::pointContents( const vec3_t point, int passEntityNum ) {
	int			i;
	entityState_t	*ent;
	ClientEntity	*cent;
	clipHandle_t cmodel;
	int			contents;

	contents = CollisionModel::pointContents (point, 0);

	for ( i = 0 ; i < cg_numSolidEntities ; i++ ) {
		cent = cg_solidEntities[ i ];

		ent = &cent->currentState;

		if ( ent->number == passEntityNum ) {
			continue;
		}

		if (ent->solid != SOLID_BMODEL) { // special value for bmodel
			continue;
		}

		cmodel = CollisionModel::inlineModel( ent->modelindex );
		if ( !cmodel ) {
			continue;
		}

		contents |= CollisionModel::transformedPointContents( point, cmodel, ent->origin, ent->angles );
	}

	return contents;
}


/*
========================
CGame::interpolatePlayerState

Generates cg.predictedPlayerState by interpolating between
cg.snap->player_state and cg.nextFrame->player_state
========================
*/
void CGame::interpolatePlayerState( bool grabAngles ) {
	float			f;
	int				i;
	playerState_t	*out;
	snapshot_t		*prev, *next;

	out = &cg.predictedPlayerState;
	prev = cg.snap;
	next = cg.nextSnap;

	*out = cg.snap->ps;

	// if we are still allowing local input, short circuit the view angles
	if ( grabAngles ) {
		usercmd_t	cmd;
		int			cmdNum;

		cmdNum = ClientHost::getCurrentCmdNumber();
		ClientHost::getUserCmd( cmdNum, &cmd );

		PM_UpdateViewAngles( out, &cmd );
	}

	// if the next frame is a teleport, we can't lerp to it
	if ( cg.nextFrameTeleport ) {
		return;
	}

	if ( !next || next->serverTime <= prev->serverTime ) {
		return;
	}

	f = static_cast<float>( cg.time - prev->serverTime ) / ( next->serverTime - prev->serverTime );

    for ( i = 0 ; i < 3 ; i++ ) {
		out->origin[i] = prev->ps.origin[i] + f * (next->ps.origin[i] - prev->ps.origin[i] );
		if ( !grabAngles ) {
			out->viewangles[i] = LerpAngle( prev->ps.viewangles[i], next->ps.viewangles[i], f );
		}
		out->velocity[i] = prev->ps.velocity[i] + 
				f * (next->ps.velocity[i] - prev->ps.velocity[i] );
		if( cg.snap->ps.pm_type == PM_VEHICLE ) {
			out->vehicleAngles[i] = prev->ps.vehicleAngles[i] + f * (next->ps.vehicleAngles[i] - 
							prev->ps.vehicleAngles[i]);
			out->gunAngle = prev->ps.gunAngle + f * (next->ps.gunAngle - prev->ps.gunAngle);
			out->turretAngle = prev->ps.turretAngle + f * (next->ps.turretAngle - prev->ps.turretAngle);
#pragma message( "this is the problem: predict correctly here!" )
		}

    }
   
	if( cg.snap->ps.pm_type == PM_VEHICLE ) {
		VectorCopy( out->vehicleAngles, cg.predictedPlayerEntity.currentState.angles );
		cg.predictedPlayerEntity.currentState.angles2[PITCH] = out->gunAngle;
		cg.predictedPlayerEntity.currentState.angles2[ROLL] = out->turretAngle;
    }

}

/*
===================
CG_TouchItem
===================
*/
void ClientEntity::touchItem() {
	gitem_t		*item;
	clientInfo_t *ci;

	if ( !cg_predictItems.integer ) {
		return;
	}
	if ( !GameShared::playerTouchesItem( &cg.predictedPlayerState, &this->currentState, cg.time ) ) {
		return;
	}

	// never pick an item up twice in a prediction
	if ( this->miscTime == cg.time ) {
		return;
	}

	ci = &cgs.clientinfo[cg.predictedPlayerState.clientNum];

	if ( !GameShared::canItemBeGrabbed( cgs.gametype, &this->currentState, &cg.predictedPlayerState, ci->vehicle ) ) {
		return;		// can't hold it
	}

	item = &bg_itemlist[ this->currentState.modelindex ];

	// Special case for flags.  
	// We don't predict touching our own flag
	if( cgs.gametype == GT_CTF ) {
		if (cg.predictedPlayerState.persistant[PERS_TEAM] == ClientBase::TEAM_RED &&
			item->giTag == OB_REDFLAG)
			return;
		if (cg.predictedPlayerState.persistant[PERS_TEAM] == ClientBase::TEAM_BLUE &&
			item->giTag == OB_BLUEFLAG)
			return;
	}

	// grab it
	GameShared::addPredictableEventToPlayerstate( EV_ITEM_PICKUP, this->currentState.modelindex , &cg.predictedPlayerState);

	// remove it from the frame so it won't be drawn
	this->currentState.eFlags |= EF_NODRAW;

	// don't touch it again this prediction
	this->miscTime = cg.time;
}


/*
=========================
CGame::touchTriggerPrediction

Predict push triggers and items
=========================
*/
void CGame::touchTriggerPrediction( ) {
	int			i;
	trace_t		trace;
	entityState_t	*ent;
	clipHandle_t cmodel;
	ClientEntity	*cent;
	bool	spectator;

	// dead clients don't activate triggers
	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	spectator = ( cg.predictedPlayerState.pm_type == PM_SPECTATOR );

	if ( cg.predictedPlayerState.pm_type != PM_NORMAL && !spectator ) {
		return;
	}

	for ( i = 0 ; i < cg_numTriggerEntities ; i++ ) {
		cent = cg_triggerEntities[ i ];
		ent = &cent->currentState;

		if ( ent->eType == ET_ITEM && !spectator ) {
			cent->touchItem();
			continue;
		}

		if ( ent->solid != SOLID_BMODEL ) {
			continue;
		}

		cmodel = CollisionModel::inlineModel( ent->modelindex );
		if ( !cmodel ) {
			continue;
		}

		CollisionModel::boxTrace( &trace, cg.predictedPlayerState.origin, cg.predictedPlayerState.origin, 
			cg_pmove.mins, cg_pmove.maxs, cmodel, -1, false );

		if ( !trace.startsolid ) {
			continue;
		}

		if ( ent->eType == ET_TELEPORT_TRIGGER ) {
			cg.hyperspace = true;
		} 
	}
}



/*
=================
CGame::predictPlayerState

Generates cg.predictedPlayerState for the current cg.time
cg.predictedPlayerState is guaranteed to be valid after exiting.

For demo playback, this will be an interpolation between two valid
playerState_t.

For normal gameplay, it will be the result of predicted usercmd_t on
top of the most recent playerState_t received from the server.

Each New snapshot will usually have one or more New usercmd over the last,
but we simulate all unacknowledged commands each time, not just the New ones.
This means that on an internet connection, quite a few pmoves may be issued
each frame.

OPTIMIZE: don't re-simulate unless the newly arrived snapshot playerState_t
differs from the predicted one.  Would require saving all intermediate
playerState_t during prediction.

We detect prediction errors and allow them to be decayed off over several frames
to ease the jerk.
=================
*/
void CGame::predictPlayerState( ) {
	int			cmdNum, current;
	playerState_t	oldPlayerState;
	bool	moved;
	usercmd_t	oldestCmd;
	usercmd_t	latestCmd;

	cg.hyperspace = false;	// will be set if touching a trigger_teleport

	// if this is the first frame we must guarantee
	// predictedPlayerState is valid even if there is some
	// other error condition
	if ( !cg.validPPS ) {
		cg.validPPS = true;
		cg.predictedPlayerState = cg.snap->ps;
	}


	// demo playback just copies the moves
	if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) ) {
		CGame::interpolatePlayerState( false );
		return;
	}

	// non-predicting local movement will grab the latest angles
	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		CGame::interpolatePlayerState( true );
		return;
	}

	// prepare for pmove
	cg_pmove.ps = &cg.predictedPlayerState;
	cg_pmove.trace = CGame::trace;
	cg_pmove.pointcontents = CGame::pointContents;
	if ( cg_pmove.ps->pm_type == PM_DEAD ) {
		cg_pmove.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	}
	else {
		cg_pmove.tracemask = MASK_PLAYERSOLID;
	}
	if ( cg.snap->ps.persistant[PERS_TEAM] == ClientBase::TEAM_SPECTATOR ) {
		cg_pmove.tracemask &= ~CONTENTS_BODY;	// spectators can fly through bodies
	}

	// save the state before the pmove so we can detect transitions
	oldPlayerState = cg.predictedPlayerState;

	current = ClientHost::getCurrentCmdNumber();

	// if we don't have the commands right after the snapshot, we
	// can't accurately predict a current position, so just freeze at
	// the last good position we had
	cmdNum = current - CMD_BACKUP + 1;
	ClientHost::getUserCmd( cmdNum, &oldestCmd );
	if ( oldestCmd.serverTime > cg.snap->ps.commandTime 
		&& oldestCmd.serverTime < cg.time ) {	// special check for map_restart
		if ( cg_showmiss.integer ) {
			CGame::printf ("exceeded PACKET_BACKUP on commands\n");
		}
		return;
	}

	// get the latest command so we can know which commands are from previous map_restarts
	ClientHost::getUserCmd( current, &latestCmd );

	// get the most recent information we have, even if
	// the server time is beyond our current cg.time,
	// because predicted player positions are going to 
	// be ahead of everything else anyway
	if ( cg.nextSnap && !cg.nextFrameTeleport && !cg.thisFrameTeleport ) {
		cg.predictedPlayerState = cg.nextSnap->ps;
		cg.physicsTime = cg.nextSnap->serverTime;
	} else {
		cg.predictedPlayerState = cg.snap->ps;
		cg.physicsTime = cg.snap->serverTime;
	}

	if ( pmove_msec.integer < 8 ) {
		CvarSystem::set("pmove_msec", "8");
		CvarSystem::update(&pmove_msec);
	}
	else if (pmove_msec.integer > 33) {
		CvarSystem::set("pmove_msec", "33");
		CvarSystem::update(&pmove_msec);
	}

	cg_pmove.pmove_fixed = pmove_fixed.integer;// | cg_pmove_fixed.integer;
	cg_pmove.pmove_msec = pmove_msec.integer;

	// run cmds
	moved = false;
	for ( cmdNum = current - CMD_BACKUP + 1 ; cmdNum <= current ; cmdNum++ ) {
		// get the command
		ClientHost::getUserCmd( cmdNum, &cg_pmove.cmd );

		if ( cg_pmove.pmove_fixed ) {
			PM_UpdateViewAngles( cg_pmove.ps, &cg_pmove.cmd );
		}

		// don't do anything if the time is before the snapshot player time
		if ( cg_pmove.cmd.serverTime <= cg.predictedPlayerState.commandTime ) {
			continue;
		}

		// don't do anything if the command was from a previous map_restart
		if ( cg_pmove.cmd.serverTime > latestCmd.serverTime ) {
			continue;
		}

		// check for a prediction error from last frame
		// on a lan, this will often be the exact value
		// from the snapshot, but on a wan we will have
		// to predict several commands to get to the point
		// we want to compare
		if ( cg.predictedPlayerState.commandTime == oldPlayerState.commandTime ) {
			Vec3	delta;
			float	len;

			if ( cg.thisFrameTeleport ) {
				// a teleport will not cause an error decay
				cg.predictedError = Vec3();
				if ( cg_showmiss.integer ) {
					CGame::printf( "PredictionTeleport\n" );
				}
				cg.thisFrameTeleport = false;
			} else {
				Vec3	adjusted;
				CGame::adjustPositionForMover( cg.predictedPlayerState.origin, 
					cg.predictedPlayerState.groundEntityNum, cg.physicsTime, cg.oldTime, adjusted );

				if ( cg_showmiss.integer ) {
					if (!VectorCompare( oldPlayerState.origin, adjusted )) {
						CGame::printf("prediction error\n");
					}
				}
				VectorSubtract( oldPlayerState.origin, adjusted, delta );
				len = VectorLength( delta );
				if ( len > 0.1 ) {
					if ( cg_showmiss.integer ) {
						CGame::printf("Prediction miss: %f\n", len);
					}
					if ( cg_errorDecay.integer ) {
						int		t;
						float	f;

						t = cg.time - cg.predictedErrorTime;
						f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
						if ( f < 0 ) {
							f = 0;
						}
						if ( f > 0 && cg_showmiss.integer ) {
							CGame::printf("Double prediction decay: %f\n", f);
						}
						cg.predictedError = cg.predictedError * ( f );
					} else {
						cg.predictedError = Vec3();
					}
					cg.predictedError = delta + cg.predictedError;
					cg.predictedErrorTime = cg.oldTime;
				}
			}
		}

		if ( cg_pmove.pmove_fixed ) {
			cg_pmove.cmd.serverTime = ((cg_pmove.cmd.serverTime + pmove_msec.integer-1) / 
										pmove_msec.integer) * pmove_msec.integer;
		}

		// MFQ3 vehicle
		cg_pmove.vehicle = cg_vehicle.integer;
		cg_pmove.advancedControls = cg_advanced.integer;

		Pmove (&cg_pmove);

		// MFQ3 vehicle
		if( cg_pmove.vehicle >= 0 ) {
			VectorCopy( cg_pmove.ps->vehicleAngles, cg.predictedPlayerEntity.currentState.angles );
		}

		moved = true;

		// add push trigger movement effects
		CGame::touchTriggerPrediction();

		// check for predictable events that changed from previous predictions
		//CGame::checkChangedPredictableEvents(&cg.predictedPlayerState);
	}

	if ( cg_showmiss.integer > 1 ) {
		CGame::printf( "[%i : %i] ", cg_pmove.cmd.serverTime, cg.time );
	}

	if ( !moved ) {
		if ( cg_showmiss.integer ) {
			CGame::printf( "not moved\n" );
		}
		return;
	}

	// adjust for the movement of the groundentity
	CGame::adjustPositionForMover( cg.predictedPlayerState.origin, 
		cg.predictedPlayerState.groundEntityNum, 
		cg.physicsTime, cg.time, cg.predictedPlayerState.origin );

	if ( cg_showmiss.integer ) {
		if (cg.predictedPlayerState.eventSequence > oldPlayerState.eventSequence + MAX_PS_EVENTS) {
			CGame::printf("WARNING: dropped event\n");
		}
	}

	// fire events and other transition triggered things
	CGame::transitionPlayerState( &cg.predictedPlayerState, &oldPlayerState );

	if ( cg_showmiss.integer ) {
		if (cg.eventSequence > cg.predictedPlayerState.eventSequence) {
			CGame::printf("WARNING: double event\n");
			cg.eventSequence = cg.predictedPlayerState.eventSequence;
		}
	}
}


