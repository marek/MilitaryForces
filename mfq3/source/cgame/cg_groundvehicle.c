/*
 * $Id: cg_groundvehicle.c,v 1.5 2001-12-23 02:02:14 thebjoern Exp $
*/


#include "cg_local.h"


// make sure the tags are named properly!
char *gv_tags[BP_GV_MAX_PARTS] =
{
	"<no tag>",		// vehicle body
	"tag_turret",	// turret
	"tag_weap",		// gun
};

/*
===============
CG_GroundVehicleShadow

===============
*/
static qboolean CG_GroundVehicleShadow( centity_t *cent, float *shadowPlane ) {

	// shadows for ground vehicles are only needed when the ground vehicle
	// is "airborne", ie for example it is falling down a cliff or bridge
	// - by default the flag OO_LANDED is set for ground vehicles and it
	// only gets removed when the vehicle is falling


	return qfalse;	// temp to make it compile

/*	vec3_t		end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t		trace;
	float		alpha;

	*shadowPlane = 0;

	if ( cg_shadows.integer == 0 ) {
		return qfalse;
	}

	// send a trace down from the player to the ground
	VectorCopy( cent->lerpOrigin, end );
	end[2] -= 256;

	trap_CM_BoxTrace( &trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID );

	// no shadow if too high
	if ( trace.fraction == 1.0 ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal, 
		(cent->currentState.angles[1]-180), alpha,alpha,alpha,1, qfalse, 24, qtrue ); // for a correct shadow use: cent->currentState.angles[1]-180

	return qtrue;
*/
}




/*
==========================
CG_RegisterGroundVehicle
==========================
*/
void CG_RegisterGroundVehicle( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
//		if( !ci->parts[i] ) {
//			CG_Printf( "Informational: RegisterPlane failed register part %d of vehicle %d (id %x)\n", i, 
//				ci->vehicle, availableVehicles[ci->vehicle].id );
//		}
	}
}


/*
=============
CG_GroundVehicleMuzzleFlash

=============
*//*
static void CG_GroundVehicleMuzzleFlash( centity_t *cent, const refEntity_t *parent, qhandle_t parentModel ) {
	refEntity_t		flash;
	vec3_t			angles;

	// impulse flash
	if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME ) {
		return;
	}

	// UGLY!
	memset( &flash, 0, sizeof( flash ) );
	VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;
	flash.hModel = cgs.media.vehicleMuzzleFlashModel;

	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = crandom() * 10;
	AnglesToAxis( angles, flash.axis );

	CG_PositionRotatedEntityOnTag( &flash, parent, parentModel, "tag_gun2");
	trap_R_AddRefEntityToScene( &flash );

}
*/

/*
===============
CG_GroundVehicleFlags
===============
*/
static void CG_GroundVehicleFlags( centity_t *cent ) {
	int		powerups;
	clientInfo_t	*ci;

	powerups = cent->currentState.objectives;
	if ( !powerups ) {
		return;
	}

	ci = &cgs.clientinfo[ cent->currentState.clientNum ];
	// redflag
	if ( powerups & OB_REDFLAG ) {
		CG_TrailItem( cent, cgs.media.redFlagModel );
//		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 1.0, 0.2f, 0.2f );
	}

	// blueflag
	if ( powerups & OB_BLUEFLAG ) {
		CG_TrailItem( cent, cgs.media.blueFlagModel );
//		trap_R_AddLightToScene( cent->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1.0 );
	}

}

/*
===============
CG_GroundVehicle
===============
*/
void CG_GroundVehicle( centity_t *cent, clientInfo_t *ci ) 
{
	refEntity_t	    part[BP_PLANE_MAX_PARTS];
	refEntity_t		reticle;
	qboolean	    shadow;
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				i, tanksound;
	vec3_t			velocity;
	float			speed;

	// get velocity
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	speed = VectorLength( velocity );

	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
	    memset( &part[i], 0, sizeof(part[0]) );	
	}
	memset( &reticle, 0, sizeof(reticle) );

    // make sure cockpit view wont show vehicle
//    if( cent == &cg.predictedPlayerEntity && !cg.renderingThirdPerson ) {
//		renderfx |= RF_THIRD_PERSON;
//	}

    // get the rotation information
    VectorCopy( cent->currentState.angles, cent->lerpAngles );
    AnglesToAxis( cent->lerpAngles, part[BP_GV_BODY].axis );

    // add the talk baloon or disconnect icon
    CG_PlayerSprites( cent );
    
    // add the shadow
    shadow = CG_GroundVehicleShadow( cent, &shadowPlane );

    if ( cg_shadows.integer == 3 && shadow ) {
    	renderfx |= RF_SHADOW_PLANE;
    }
    renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all

    //
    // add the hull
    //
    part[BP_GV_BODY].hModel = ci->parts[BP_GV_BODY];
    VectorCopy( cent->lerpOrigin, part[BP_GV_BODY].origin );
    VectorCopy( cent->lerpOrigin, part[BP_GV_BODY].lightingOrigin );
    part[BP_GV_BODY].shadowPlane = shadowPlane;
    part[BP_GV_BODY].renderfx = renderfx;
    VectorCopy (part[BP_GV_BODY].origin, part[BP_GV_BODY].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_BODY] );

    //
    // turret
    //
    part[BP_GV_TURRET].hModel = ci->parts[BP_GV_TURRET];
    VectorCopy( cent->lerpOrigin, part[BP_GV_TURRET].lightingOrigin );
	AxisCopy( axisDefault, part[BP_GV_TURRET].axis );
	RotateAroundYaw( part[BP_GV_TURRET].axis, cent->currentState.angles2[ROLL] );
	CG_PositionRotatedEntityOnTag( &part[BP_GV_TURRET], &part[BP_GV_BODY], ci->parts[BP_GV_BODY], gv_tags[BP_GV_TURRET] );
	part[BP_GV_TURRET].shadowPlane = shadowPlane;
    part[BP_GV_TURRET].renderfx = renderfx;
    VectorCopy (part[BP_GV_TURRET].origin, part[BP_GV_TURRET].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_TURRET] );

    //
    // gun
    //
    part[BP_GV_GUNBARREL].hModel = ci->parts[BP_GV_GUNBARREL];
    VectorCopy( cent->lerpOrigin, part[BP_GV_GUNBARREL].lightingOrigin );
	AxisCopy( axisDefault, part[BP_GV_GUNBARREL].axis );
	RotateAroundPitch( part[BP_GV_GUNBARREL].axis, cent->currentState.angles2[PITCH] );
	CG_PositionRotatedEntityOnTag( &part[BP_GV_GUNBARREL], &part[BP_GV_TURRET], ci->parts[BP_GV_TURRET], gv_tags[BP_GV_GUNBARREL] );
	part[BP_GV_GUNBARREL].shadowPlane = shadowPlane;
    part[BP_GV_GUNBARREL].renderfx = renderfx;
    VectorCopy (part[BP_GV_GUNBARREL].origin, part[BP_GV_GUNBARREL].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_GUNBARREL] );

/*
		AngleVectors( self->client->ps.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ((float)self->client->ps.turretAngle)/10 );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ((float)self->client->ps.gunAngle)/10 );
  */

	// reticle
	if( cent == &cg.predictedPlayerEntity ) {
		vec3_t	forward, right, up, ang, start, end, temp;
		trace_t	tr;
		float len;
		playerState_t* ps = &cg.snap->ps;
		float mindist = cg_thirdPersonRange.integer + availableVehicles[ci->vehicle].cam_dist + 
			availableVehicles[ci->vehicle].maxs[0] + 20;

		if( ps->stats[STAT_LOCKINFO] & LI_TRACKING ) {
			refEntity_t		reticlelock;
			centity_t* target = &cg_entities[cent->currentState.tracktarget];
			reticle.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshair];
			VectorSubtract( target->lerpOrigin, cent->lerpOrigin, forward );
			len = VectorNormalize( forward );
			vectoangles( forward, ang );
			AnglesToAxis( ang, reticle.axis );
			VectorMA( cent->lerpOrigin, 2000, forward, end );
			CG_Trace( &tr, cent->lerpOrigin, 0, 0, end, cg.snap->ps.clientNum, MASK_ALL ); 
			if ( tr.entityNum < MAX_CLIENTS ) {
				cg.crosshairClientNum = tr.entityNum;
				cg.crosshairClientTime = cg.time;
			}
			VectorCopy( tr.endpos, end );
			CG_Trace( &tr, cg.refdef.vieworg, 0, 0, end, cg.snap->ps.clientNum, MASK_SOLID ); 
			VectorSubtract( tr.endpos, cg.refdef.vieworg, forward );
			len = VectorNormalize(forward);
			if( len > mindist ) {
				VectorMA( cg.refdef.vieworg, mindist, forward, reticle.origin );
			} else {
				VectorMA( cg.refdef.vieworg, len - 5, forward, reticle.origin );
			}
			VectorCopy( reticle.origin, reticle.lightingOrigin );
			reticle.shadowPlane = shadowPlane;
			reticle.renderfx = renderfx;
			trap_R_AddRefEntityToScene( &reticle );
			if( ps->stats[STAT_LOCKINFO] & LI_LOCKING ) {
				memcpy( &reticlelock, &reticle, sizeof(reticle) );
				reticlelock.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshair+1];
				reticlelock.frame = 1;
			} else {
				memset( &reticlelock, 0, sizeof(reticlelock) );	
				reticlelock.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshair+1];
				AngleVectors( cent->currentState.angles, forward, right, up );
				RotatePointAroundVector( temp, up, forward, cent->currentState.angles2[ROLL] );
				CrossProduct( up, temp, right );
				RotatePointAroundVector( forward, right, temp, cent->currentState.angles2[PITCH] );
				VectorMA( cent->lerpOrigin, availableVehicles[ci->vehicle].gunoffset[0], forward, start );
				VectorMA( start, availableVehicles[ci->vehicle].gunoffset[1], right, start );
				VectorMA( start, availableVehicles[ci->vehicle].gunoffset[2], up, start );
				VectorMA( start, 2000, forward, end );
				vectoangles( forward, ang );
//				ang[2] = 0;
				AnglesToAxis( ang, reticlelock.axis );
				VectorScale( reticlelock.axis[0], 10.0f, reticlelock.axis[0] );
				VectorScale( reticlelock.axis[1], 10.0f, reticlelock.axis[1] );
				VectorScale( reticlelock.axis[2], 10.0f, reticlelock.axis[2] );
				reticlelock.nonNormalizedAxes = qtrue;
				CG_Trace( &tr, start, 0, 0, end, cg.snap->ps.clientNum, MASK_ALL ); 
				VectorCopy( tr.endpos, end );
				CG_Trace( &tr, cg.refdef.vieworg, 0, 0, end, cg.snap->ps.clientNum, MASK_SOLID ); 
				VectorSubtract( tr.endpos, cg.refdef.vieworg, forward );
				len = VectorNormalize(forward);
				if( len > mindist ) {
					VectorMA( cg.refdef.vieworg, mindist, forward, reticlelock.origin );
				} else {
					VectorMA( cg.refdef.vieworg, len - 5, forward, reticlelock.origin );
				}
				VectorCopy( reticlelock.origin, reticlelock.lightingOrigin );
				reticlelock.shadowPlane = shadowPlane;
				reticlelock.renderfx = renderfx;
			}
			trap_R_AddRefEntityToScene( &reticlelock );
		} else {
			reticle.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshair];
			AngleVectors( cent->currentState.angles, forward, right, up );
			RotatePointAroundVector( temp, up, forward, cent->currentState.angles2[ROLL] );
			CrossProduct( up, temp, right );
			RotatePointAroundVector( forward, right, temp, cent->currentState.angles2[PITCH] );
			VectorMA( cent->lerpOrigin, availableVehicles[ci->vehicle].gunoffset[0], forward, start );
			VectorMA( start, availableVehicles[ci->vehicle].gunoffset[1], right, start );
			VectorMA( start, availableVehicles[ci->vehicle].gunoffset[2], up, start );
			VectorMA( start, 2000, forward, end );
			vectoangles( forward, ang );
			ang[2] = 0;
			AnglesToAxis( ang, reticle.axis );
			CG_Trace( &tr, start, 0, 0, end, cg.snap->ps.clientNum, MASK_ALL ); 
			if ( tr.entityNum < MAX_CLIENTS ) {
				cg.crosshairClientNum = tr.entityNum;
				cg.crosshairClientTime = cg.time;
			}
			VectorCopy( tr.endpos, end );
			CG_Trace( &tr, cg.refdef.vieworg, 0, 0, end, cg.snap->ps.clientNum, MASK_SOLID ); 
			VectorSubtract( tr.endpos, cg.refdef.vieworg, forward );
			len = VectorNormalize(forward);
			if( len > mindist ) {
				VectorMA( cg.refdef.vieworg, mindist, forward, reticle.origin );
			} else {
				VectorMA( cg.refdef.vieworg, len - 5, forward, reticle.origin );
			}
			VectorCopy( reticle.origin, reticle.lightingOrigin );
			reticle.shadowPlane = shadowPlane;
			reticle.renderfx = renderfx;
			trap_R_AddRefEntityToScene( &reticle );
		}
	}

	// sound
	BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );
	tanksound = speed * NUM_TANKSOUNDS / availableVehicles[ci->vehicle].maxspeed;
	if( tanksound >= NUM_TANKSOUNDS ) tanksound = NUM_TANKSOUNDS - 1;
	trap_S_AddLoopingSound( cent->currentState.number, 
							cent->lerpOrigin, 
							velocity, 
							cgs.media.engineTank[tanksound] );

	// smoke 
	if( cent->currentState.generic1 && cg_smoke.integer ) {
		localEntity_t	*smoke;
		vec3_t			up = {0, 0, 20};
		vec3_t			pos;
		vec3_t			forward;

		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorCopy( cent->lerpOrigin, pos );
		pos[2] += 12;

		smoke = CG_SmokePuff( pos, up, 
					  cent->currentState.generic1, 
					  0.5, 0.5, 0.5, 0.66f,
					  200*cent->currentState.generic1, 
					  cg.time, 0,
					  LEF_PUFF_DONT_SCALE, 
					  cgs.media.smokePuffShader );	
	}
#pragma message("maybe use the hastePuffShader for a dust trail when tanks drive ?")
	// muzzleflash
	CG_VehicleMuzzleFlash( cent, &part[BP_GV_GUNBARREL], ci->parts[BP_GV_GUNBARREL], ci->vehicle );

	// CTF
	CG_GroundVehicleFlags( cent );
}

/*
=============
CG_GroundVehicleObituary

general ugly function, needs to be made prettier some day...same with plane obituary
=============
*/
void CG_GroundVehicleObituary( entityState_t *ent, clientInfo_t *ci ) 
{
	// add some cool stuff here :-)

    int			mod;
    int			target, attacker;
    char		*message;
    char		*message2;
    const char	*targetInfo;
    const char	*attackerInfo;
    char		targetName[32];
    char		attackerName[32];
    gender_t	gender;

    target = ent->otherEntityNum;
    attacker = ent->otherEntityNum2;
    mod = ent->eventParm;

    if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
	    attacker = ENTITYNUM_WORLD;
	    attackerInfo = NULL;
    } else {
	    attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
    }

    targetInfo = CG_ConfigString( CS_PLAYERS + target );
    if ( !targetInfo ) {
	    return;
    }
	if( Info_ValueForKey( targetInfo, "n" ) != 0 ) {
		Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	}
	else {
		trap_Error( "MFQ3 Error (3): Invalid targetinfo\n" );
	}	
    strcat( targetName, S_COLOR_WHITE );

    message2 = "";

    // check for single client messages

    switch( mod ) {
    case MOD_SUICIDE:
	    message = "suicides";
	    break;
    case MOD_FALLING:
	    message = "cratered";
	    break;
    case MOD_CRUSH:
	    message = "was squished";
	    break;
    case MOD_WATER:
	    message = "sank like a rock";
	    break;
    case MOD_SLIME:
	    message = "melted";
	    break;
    case MOD_LAVA:
	    message = "does a back flip into the lava";
	    break;
    default:
	    message = NULL;
	    break;
    }

    if (attacker == target) {
	gender = ci->gender;
	switch (mod) {
	case MOD_CRASH:
		message = "was unable to control his vehicle";
		break;
	default:
		if ( gender == GENDER_FEMALE )
			message = "killed herself";
		else if ( gender == GENDER_NEUTER )
			message = "killed itself";
		else
			message = "killed himself";
		break;
	}
    }

    if (message) {
	    CG_Printf( "%s %s.\n", targetName, message);
	    return;
    }

    // check for kill messages from the current clientNum
    if ( attacker == cg.snap->ps.clientNum ) {
	char	*s;

	if ( cgs.gametype < GT_TEAM ) {
		s = va("You fragged %s\n%s place with %i", targetName, 
			CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
			cg.snap->ps.persistant[PERS_SCORE] );
	} else {
		s = va("You fragged %s", targetName );
	}
	CG_CenterPrint( s, SCREEN_HEIGHT * 0.25, BIGCHAR_WIDTH );

	// print the text message as well
    }

    // check for double client messages
    if ( !attackerInfo ) {
	    attacker = ENTITYNUM_WORLD;
	    strcpy( attackerName, "noname" );
    } else {
		if( Info_ValueForKey( attackerInfo, "n" ) != 0 ) {
			Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		}
		else {
			trap_Error( "MFQ3 Error (1): Bad attackerInfo!\n" );
		}
	    strcat( attackerName, S_COLOR_WHITE );
	    // check for kill messages about the current clientNum
	    if ( target == cg.snap->ps.clientNum ) {
			if( attackerName != 0 ) {
				Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
			}
			else {
				trap_Error( "MFQ3 Error (2): No attackerName!\n" );
			}
	    }
    }

    if ( attacker != ENTITYNUM_WORLD ) {
	switch (mod) {

	case MOD_FFAR:
		message = "couldn't evade";
		message2 = "'s rocket attack";
		break;
	case MOD_FFAR_SPLASH:
		message = "was shredded by";
		message2 = "'s rocket shrapnel";
		break;
	case MOD_IRONBOMB:
	case MOD_IRONBOMB_SPLASH:
		message = "was bombed to smithereens by";
		message2 = "'s iron bombs";
		break;
	case MOD_AUTOCANNON:
		message = "was blown out of the sky by";
		message2 = "'s bullets";
		break;
	case MOD_MAINGUN:
		message = "was shelled by";
		break;
	case MOD_VEHICLEEXPLOSION:
		message = "died in";
		message2 = "'s explosion";
		break;
	case MOD_TELEFRAG:
		message = "tried to invade";
		message2 = "'s personal space";
		break;
	default:
		message = "was killed by";
		break;
	}

	if (message) {
		CG_Printf( "%s %s %s%s\n", 
			targetName, message, attackerName, message2);
		return;
	}
    }

    // we don't know what it was
    CG_Printf( "%s died.\n", targetName );
}

