/*
 * $Id: cg_plane.c,v 1.6 2001-12-24 02:17:35 thebjoern Exp $
*/


#include "cg_local.h"

char *plane_tags[BP_PLANE_MAX_PARTS] =
{
	"<no tag>",		// plane body
	"tag_controls",	// controls
	"tag_cockpit",	// cockpit
	"tag_gear",		// gear
	"tag_breaks",	// brakes
	"tag_bay",		// bay
	"tag_swing",	// swing
	"tag_special",	// special
	"tag_prop1"		// prop
};

/*
===============
CG_PlaneShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
static qboolean CG_PlaneShadow( centity_t *cent, float *shadowPlane ) {
	vec3_t		end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t		trace;
	float		alpha;
return qfalse;
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
}




/*
==========================
CG_RegisterPlane
==========================
*/
void CG_RegisterPlane( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_PLANE_MAX_PARTS; i++ ) {
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
//		if( !ci->parts[i] ) {
//			CG_Printf( "Informational: RegisterPlane failed register part %d of vehicle %d (id %x)\n", i, 
//				ci->vehicle, availableVehicles[ci->vehicle].id );
//		}
	}
}


/*
=============
CG_PlanePilot

=============
*/
void CG_PlanePilot( centity_t *cent, const refEntity_t *parent, qhandle_t parentModel, int idx ) {

	refEntity_t		pilot[MAX_PILOTS*3];	// three parts per pilot
	unsigned int	i,j,k;
	unsigned int	pilots = ((availableVehicles[idx].renderFlags & MFR_DUALPILOT) ? 2 : 1);
	char			tag[12];

	// pilot
	for( i = 0; i < pilots; i++ ) {
		for( j = 0; j < 3; j++ ) {
			k = 3*i+j;
			memset( &pilot[k], 0, sizeof( pilot[k] ) );
			VectorCopy( parent->lightingOrigin, pilot[k].lightingOrigin );
			pilot[k].shadowPlane = parent->shadowPlane;
			pilot[k].renderfx = parent->renderfx;
			if( j == 0 ) pilot[k].hModel = (cgs.gameset != MF_GAMESET_MODERN ? cgs.media.pilotWW2 : cgs.media.pilot);
			else if( j == 1 ) pilot[k].hModel = (cgs.gameset != MF_GAMESET_MODERN ? cgs.media.pilotHeadWW2 : cgs.media.pilotHead);
			else pilot[k].hModel = (cgs.gameset != MF_GAMESET_MODERN ? cgs.media.pilotSeatWW2 : cgs.media.pilotSeat);
			AxisCopy( axisDefault, pilot[k].axis );
			Com_sprintf( tag, 12, "tag_pilot%d", i+1 );
			CG_PositionRotatedEntityOnTag( &pilot[k], parent, parentModel, tag);
			trap_R_AddRefEntityToScene( &pilot[k] );
		}
	}
}

/*
=============
CG_PlaneMuzzleFlash

=============
*//*
static void CG_PlaneMuzzleFlash( centity_t *cent, const refEntity_t *parent, qhandle_t parentModel, int idx ) {

	refEntity_t		flash[MAX_MUZZLEFLASHES];
	vec3_t			angles;
	unsigned int	i;
	unsigned int	flashes;
	char			tag[10];

	// impulse flash
	if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME ) {
		return;
	}

	flashes = availableWeapons[availableVehicles[idx].weapons[0]].barrels;

	if( flashes > MAX_MUZZLEFLASHES ) flashes = MAX_MUZZLEFLASHES;

	for( i = 0; i < flashes; i++ ) {
		memset( &flash[i], 0, sizeof( flash[i] ) );
		VectorCopy( parent->lightingOrigin, flash[i].lightingOrigin );
		flash[i].shadowPlane = parent->shadowPlane;
		flash[i].renderfx = parent->renderfx;
		flash[i].hModel = cgs.media.vehicleMuzzleFlashModel;

		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = crandom() * 10;
		AnglesToAxis( angles, flash[i].axis );

		if( i > 0 ) {
			Com_sprintf( tag, 10, "tag_gun%d", i+1 );
		} else {
			strcpy( tag, "tag_gun" );
		}
		CG_PositionRotatedEntityOnTag( &flash[i], parent, parentModel, tag);
		trap_R_AddRefEntityToScene( &flash[i] );
	}
}*/

/*
===============
CG_PlaneFlags
===============
*/
static void CG_PlaneFlags( centity_t *cent ) {
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
CG_Plane
===============
*/
void CG_Plane( centity_t *cent, clientInfo_t *ci ) 
{
	refEntity_t	    part[BP_PLANE_MAX_PARTS];
	refEntity_t		vapor;
	refEntity_t		burner;
	refEntity_t		burner2;
	refEntity_t		reticle;
	qboolean	    shadow;
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				i;
	int				ONOFF = cent->currentState.ONOFF;
	vec3_t			velocity;	

	// get velocity
	BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

	for( i = 0; i < BP_PLANE_MAX_PARTS; i++ ) {
	    memset( &part[i], 0, sizeof(part[0]) );	
	}
    memset( &vapor, 0, sizeof(vapor) );	
    memset( &burner, 0, sizeof(burner) );	
    memset( &burner2, 0, sizeof(burner2) );	
	memset( &reticle, 0, sizeof(reticle) );	

    // make sure cockpit view wont show vehicle
//    if( cent == &cg.predictedPlayerEntity && !cg.renderingThirdPerson ) {
//		renderfx |= RF_THIRD_PERSON;
//	}

    // get the rotation information
    VectorCopy( cent->currentState.angles, cent->lerpAngles );
    AnglesToAxis( cent->lerpAngles, part[BP_PLANE_BODY].axis );

    // add the talk baloon or disconnect icon
    CG_PlayerSprites( cent );
    
    // add the shadow
    shadow = CG_PlaneShadow( cent, &shadowPlane );

    if ( cg_shadows.integer == 3 && shadow ) {
    	renderfx |= RF_SHADOW_PLANE;
    }
    renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all

	//
	// animations
	//
		// control surfaces
	part[BP_PLANE_CONTROLS].frame = cent->currentState.vehicleAnim;
		// gear
	if( availableVehicles[ci->vehicle].caps & HC_GEAR ) {
		if( ONOFF & OO_GEAR ) {
			part[BP_PLANE_GEAR].frame = 0;
			part[BP_PLANE_CONTROLS].frame += 9;
		}
		else {
			part[BP_PLANE_GEAR].frame = 2;
		}
	}
		// speedbrakes
	if( availableVehicles[ci->vehicle].caps & HC_SPEEDBRAKE ) {
		if( ONOFF & OO_SPEEDBRAKE ) {
			part[BP_PLANE_BRAKES].frame = 1;
		}
		else {
			part[BP_PLANE_BRAKES].frame = 0;
		}
	}
	if( ONOFF & OO_COCKPIT ) {
		part[BP_PLANE_COCKPIT].frame = 1;
	}
//	CG_Printf( "CG Anim is %d\n", part[BP_PLANE_CONTROLS].frame );

    //
    // add the plane body
    //
    part[BP_PLANE_BODY].hModel = ci->parts[BP_PLANE_BODY];
    VectorCopy( cent->lerpOrigin, part[BP_PLANE_BODY].origin );

    VectorCopy( cent->lerpOrigin, part[BP_PLANE_BODY].lightingOrigin );
    part[BP_PLANE_BODY].shadowPlane = shadowPlane;
    part[BP_PLANE_BODY].renderfx = renderfx;
    VectorCopy (part[BP_PLANE_BODY].origin, part[BP_PLANE_BODY].oldorigin);
	if( cent->currentState.frame <= 10 ) {
		part[BP_PLANE_BODY].frame = 0;
	} else {
		part[BP_PLANE_BODY].frame = ( cent->currentState.frame > 12 ? 2 : 1 );
	}
    trap_R_AddRefEntityToScene( &part[BP_PLANE_BODY] );

	// if the model failed, allow the default nullmodel to be displayed
	if (!part[BP_PLANE_BODY].hModel) {
		return;
	}

	for( i = 1; i < BP_PLANE_MAX_PARTS; i++ ) {
		part[i].hModel = ci->parts[i];
		if( !part[i].hModel ) {
			continue;
		}
		VectorCopy( cent->lerpOrigin, part[i].lightingOrigin );
		AxisCopy( axisDefault, part[i].axis );
		if( i == BP_PLANE_PROP && (availableVehicles[ci->vehicle].caps & HC_PROP) ) {
			RotateAroundDirection( part[BP_PLANE_PROP].axis, cg.time );
		}
		CG_PositionRotatedEntityOnTag( &part[i], &part[BP_PLANE_BODY], ci->parts[BP_PLANE_BODY], plane_tags[i] );
		part[i].shadowPlane = shadowPlane;
		part[i].renderfx = renderfx;
		trap_R_AddRefEntityToScene( &part[i] );
	}
	CG_PlanePilot( cent, &part[BP_PLANE_BODY], ci->parts[BP_PLANE_BODY], ci->vehicle );
	// vapor
	if( ONOFF & (OO_VAPOR|OO_VAPOR_BIG) ) {
		vapor.hModel = (availableVehicles[ci->vehicle].renderFlags & MFR_BIGVAPOR) ?
			cgs.media.vaporBig : cgs.media.vapor;
		vapor.customShader = cgs.media.vaporShader;
		if( ONOFF & OO_VAPOR ) vapor.frame = 1;
		VectorCopy( cent->lerpOrigin, vapor.lightingOrigin );
		AxisCopy( axisDefault, vapor.axis );
		CG_PositionRotatedEntityOnTag( &vapor, &part[BP_PLANE_BODY], ci->parts[BP_PLANE_BODY], "tag_vapor1" );
		vapor.shadowPlane = shadowPlane;
		vapor.renderfx = renderfx;
		trap_R_AddRefEntityToScene( &vapor );
//		CG_Printf( "Vapor frame %d\n", vapor.frame );
	}
	// throttle/afterburner
	if( cent->currentState.frame > 10 ) {
		burner.hModel = cgs.media.afterburner[availableVehicles[ci->vehicle].effectModel];
		VectorCopy( cent->lerpOrigin, burner.lightingOrigin );
		AxisCopy( axisDefault, burner.axis );
		CG_PositionRotatedEntityOnTag( &burner, &part[BP_PLANE_BODY], ci->parts[BP_PLANE_BODY], "tag_ab1" );
		burner.shadowPlane = shadowPlane;
		burner.renderfx = renderfx;
		burner.frame = ( cent->currentState.frame > 12 ? 0 : 1 );
		trap_R_AddRefEntityToScene( &burner );
		if( availableVehicles[ci->vehicle].caps & HC_DUALENGINE ) {
			burner2.hModel = cgs.media.afterburner[availableVehicles[ci->vehicle].effectModel];
			VectorCopy( cent->lerpOrigin, burner2.lightingOrigin );
			AxisCopy( axisDefault, burner2.axis );
			CG_PositionRotatedEntityOnTag( &burner2, &part[BP_PLANE_BODY], ci->parts[BP_PLANE_BODY], "tag_ab2" );
			burner2.shadowPlane = shadowPlane;
			burner2.renderfx = renderfx;
			burner2.frame = ( cent->currentState.frame > 12 ? 0 : 1 );
			trap_R_AddRefEntityToScene( &burner2 );
		}
	}

	// reticle
	if( cent == &cg.predictedPlayerEntity ) {
		vec3_t	forward, ang, end;
		trace_t	tr;
		float len;
		playerState_t* ps = &cg.snap->ps;
		float mindist = cg_thirdPersonRange.integer + availableVehicles[ci->vehicle].cam_dist + 
			availableVehicles[ci->vehicle].maxs[0] + 20;

		if( ps->stats[STAT_LOCKINFO] & LI_TRACKING ) {
			refEntity_t		reticlelock;
			qboolean		building = qfalse;
			centity_t* target = &cg_entities[cent->currentState.tracktarget];
			if( target->currentState.eType == ET_EXPLOSIVE ) building = qtrue;
			reticle.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairtrack];
			if( building ) {
				VectorSubtract( cgs.inlineModelMidpoints[target->currentState.modelindex], cent->lerpOrigin, forward );
			} else {
				VectorSubtract( target->lerpOrigin, cent->lerpOrigin, forward );
			}
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
				reticlelock.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairlock];
				reticlelock.frame = 1;
			} else {
				memset( &reticlelock, 0, sizeof(reticlelock) );	
				reticlelock.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairlock];
				VectorSet( ang, cent->currentState.angles[0], cent->currentState.angles[1], 0 );
				AnglesToAxis( ang, reticlelock.axis );
				VectorScale( reticlelock.axis[0], 0.5f, reticlelock.axis[0] );
				VectorScale( reticlelock.axis[1], 0.5f, reticlelock.axis[1] );
				VectorScale( reticlelock.axis[2], 0.5f, reticlelock.axis[2] );
				reticlelock.nonNormalizedAxes = qtrue;
				AngleVectors(cent->currentState.angles, forward, 0, 0);
				VectorMA( cent->lerpOrigin, 2000, forward, end );
				CG_Trace( &tr, cent->lerpOrigin, 0, 0, end, cg.snap->ps.clientNum, MASK_ALL ); 
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
				reticle.shadowPlane = shadowPlane;
				reticle.renderfx = renderfx;
				reticlelock.frame = 1;
			}
			trap_R_AddRefEntityToScene( &reticlelock );
		} else {
			reticle.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshair];
			VectorSet( ang, cent->currentState.angles[0], cent->currentState.angles[1], 0 );
			AnglesToAxis( ang, reticle.axis );
			VectorScale( reticle.axis[0], 2.0f, reticle.axis[0] );
			VectorScale( reticle.axis[1], 2.0f, reticle.axis[1] );
			VectorScale( reticle.axis[2], 2.0f, reticle.axis[2] );
			reticle.nonNormalizedAxes = qtrue;
			AngleVectors(cent->currentState.angles, forward, 0, 0);
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
		}
	}

	// sound
	if ( availableVehicles[ci->vehicle].caps & HC_PROP ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, cgs.media.engineProp );
	}
	else {
		if( cent->currentState.frame > 10 ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, cgs.media.engineJetAB );
		}
		else {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, cgs.media.engineJet );
		}
	}
	// smoke
	if( cent->currentState.generic1 && cg_smoke.integer ) {
		localEntity_t	*smoke;
		vec3_t			up = {0, 0, 1};
		vec3_t			pos;
		vec3_t			forward;

		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorMA( cent->lerpOrigin, availableVehicles[ci->vehicle].mins[0], forward, pos );

		smoke = CG_SmokePuff( pos, up, 
					  cent->currentState.generic1, 
					  0.5, 0.5, 0.5, 0.66f,
					  100*cent->currentState.generic1, 
					  cg.time, 0,
					  LEF_PUFF_DONT_SCALE, 
					  cgs.media.smokePuffShader );	
	}
	CG_VehicleMuzzleFlash( cent, &part[BP_PLANE_BODY], ci->parts[BP_PLANE_BODY], ci->vehicle );

	CG_PlaneFlags( cent );

/*
	{
		localEntity_t	*smoke, *smoke2;
		vec3_t			up = {0, 0, 1};
		vec3_t			pos, pos2, right;

		AngleVectors( cent->lerpAngles, NULL, right, NULL );
		VectorMA( cent->lerpOrigin, availableVehicles[ci->vehicle].maxs[1], right, pos );
		VectorMA( cent->lerpOrigin, availableVehicles[ci->vehicle].mins[1], right, pos2 );

		smoke = CG_SmokePuff( pos, up, 
					  4, 
					  1.0, 1.0, 1.0, 1.0,
					  10000, 
					  cg.time, 0,
					  LEF_PUFF_DONT_SCALE, 
					  cgs.media.smokePuffShader );	
		smoke2 = CG_SmokePuff( pos2, up, 
					  4, 
					  1.0, 1.0, 1.0, 1.0,
					  10000, 
					  cg.time, 0,
					  LEF_PUFF_DONT_SCALE, 
					  cgs.media.smokePuffShader );	
	}
*/
}

/*
=============
CG_PlaneObituary
=============
*/
void CG_PlaneObituary( entityState_t *ent, clientInfo_t *ci ) 
{
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
#pragma message("remove the problem checking here!")
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
		message = "was unable to control his plane";
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
#pragma message("when we are sure this doesn't happen, then remove the two checks!" )
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

