/*
 * $Id: cg_miscvehicle.c,v 1.5 2002-02-08 21:43:57 thebjoern Exp $
*/

#include "cg_local.h"


extern char *plane_tags[BP_PLANE_MAX_PARTS];
extern char *gv_tags[BP_GV_MAX_PARTS];

/*
===============
CG_Misc_Plane
===============
*/

static void CG_Misc_Plane( centity_t *cent ) 
{
	refEntity_t	    part[BP_PLANE_MAX_PARTS];
	refEntity_t		vapor;
	refEntity_t		burner;
	refEntity_t		burner2;
	refEntity_t		reticle;
//	qboolean	    shadow;
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

    // get the rotation information
    VectorCopy( cent->currentState.angles, cent->lerpAngles );
    AnglesToAxis( cent->lerpAngles, part[BP_PLANE_BODY].axis );

    // add the talk baloon or disconnect icon
//    CG_PlayerSprites( cent );
    
    // add the shadow
//    shadow = CG_PlaneShadow( cent, &shadowPlane );

//    if ( cg_shadows.integer == 3 && shadow ) {
//    	renderfx |= RF_SHADOW_PLANE;
//    }
    renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all

	//
	// animations
	//
		// control surfaces
	part[BP_PLANE_CONTROLS].frame = cent->currentState.vehicleAnim;
		// gear
//	if( availableVehicles[cent->currentState.modelindex].caps & HC_GEAR ) {
//		if( ONOFF & OO_GEAR ) {
//			part[BP_PLANE_GEAR].frame = 0;
//			part[BP_PLANE_CONTROLS].frame += 9;
//		}
//		else {
//			part[BP_PLANE_GEAR].frame = 2;
//		}
//	}
		// gear
	if( availableVehicles[cent->currentState.modelindex].caps & HC_GEAR ) {
		int timediff = cg.time - cent->gearAnimStartTime;
		int geardown = availableVehicles[cent->currentState.modelindex].maxGearFrame;
		if( ONOFF & OO_GEAR ) {
			part[BP_PLANE_CONTROLS].frame += 9;
		}
		if( cent->gearAnim == GEAR_ANIM_UP ) {
			cent->gearAnimFrame = geardown - timediff/25;
			if( cent->gearAnimFrame < GEAR_UP ) {
				cent->gearAnimFrame = GEAR_UP;
				cent->gearAnim = GEAR_ANIM_STOP;
			}
		} else if( cent->gearAnim == GEAR_ANIM_DOWN ) {
			cent->gearAnimFrame = GEAR_UP + timediff/25;
			if( cent->gearAnimFrame > geardown ) {
				cent->gearAnimFrame = geardown;
				cent->gearAnim = GEAR_ANIM_STOP;
			}
		}
		part[BP_PLANE_GEAR].frame = cent->gearAnimFrame;
	}

		// speedbrakes
	if( availableVehicles[cent->currentState.modelindex].caps & HC_SPEEDBRAKE ) {
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
    part[BP_PLANE_BODY].hModel = availableVehicles[cent->currentState.modelindex].handle[BP_PLANE_BODY];
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
		part[i].hModel = availableVehicles[cent->currentState.modelindex].handle[i];
		if( !part[i].hModel ) {
			continue;
		}
		VectorCopy( cent->lerpOrigin, part[i].lightingOrigin );
		AxisCopy( axisDefault, part[i].axis );
		if( i == BP_PLANE_PROP && (availableVehicles[cent->currentState.modelindex].caps & HC_PROP) ) {
			RotateAroundDirection( part[i].axis, cg.time );
//		} else if( i == BP_PLANE_PROP2 && (availableVehicles[cent->currentState.modelindex].caps & HC_PROP) ) {
//			if( availableVehicles[cent->currentState.modelindex].caps & HC_DUALENGINE ) {
//				RotateAroundDirection( part[i].axis, cg.time );
//			} else continue;
		}
		CG_PositionRotatedEntityOnTag( &part[i], &part[BP_PLANE_BODY], 
				availableVehicles[cent->currentState.modelindex].handle[BP_PLANE_BODY], plane_tags[i] );
		part[i].shadowPlane = shadowPlane;
		part[i].renderfx = renderfx;
		trap_R_AddRefEntityToScene( &part[i] );
	}
	if( cent->currentState.eFlags & EF_PILOT_ONBOARD ) {
		CG_PlanePilot( cent, &part[BP_PLANE_BODY], 
					availableVehicles[cent->currentState.modelindex].handle[BP_PLANE_BODY], 
					cent->currentState.modelindex );
	}
	// vapor
/*	if( ONOFF & (OO_VAPOR|OO_VAPOR_BIG) ) {
		vapor.hModel = (availableVehicles[cent->currentState.modelindex].renderFlags & MFR_BIGVAPOR) ?
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
	}*/
	// throttle/afterburner
	if( cent->currentState.frame > 10 ) {
		burner.hModel = cgs.media.afterburner[availableVehicles[cent->currentState.modelindex].effectModel];
		VectorCopy( cent->lerpOrigin, burner.lightingOrigin );
		AxisCopy( axisDefault, burner.axis );
		CG_PositionRotatedEntityOnTag( &burner, &part[BP_PLANE_BODY], 
				availableVehicles[cent->currentState.modelindex].handle[BP_PLANE_BODY], "tag_ab1" );
		burner.shadowPlane = shadowPlane;
		burner.renderfx = renderfx;
		burner.frame = ( cent->currentState.frame > 12 ? 0 : 1 );
		trap_R_AddRefEntityToScene( &burner );
		if( availableVehicles[cent->currentState.modelindex].engines > 1 ) {
			burner2.hModel = cgs.media.afterburner[availableVehicles[cent->currentState.modelindex].effectModel];
			VectorCopy( cent->lerpOrigin, burner2.lightingOrigin );
			AxisCopy( axisDefault, burner2.axis );
			CG_PositionRotatedEntityOnTag( &burner2, &part[BP_PLANE_BODY], 
					availableVehicles[cent->currentState.modelindex].handle[BP_PLANE_BODY], "tag_ab2" );
			burner2.shadowPlane = shadowPlane;
			burner2.renderfx = renderfx;
			burner2.frame = ( cent->currentState.frame > 12 ? 0 : 1 );
			trap_R_AddRefEntityToScene( &burner2 );
		}
	}

	// sound
	if( cent->currentState.eFlags & EF_PILOT_ONBOARD ) {
		if( availableVehicles[cent->currentState.modelindex].caps & HC_PROP ) {
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
	}

	// smoke
	if( cent->currentState.generic1 ) {
		localEntity_t	*smoke;
		vec3_t			up = {0, 0, 1};
		vec3_t			pos;
		vec3_t			forward;

		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorMA( cent->lerpOrigin, availableVehicles[cent->currentState.modelindex].mins[0], forward, pos );

		smoke = CG_SmokePuff( pos, up, 
					  cent->currentState.generic1, 
					  0.5, 0.5, 0.5, 0.66f,
					  100*cent->currentState.generic1, 
					  cg.time, 0,
					  LEF_PUFF_DONT_SCALE, 
					  cgs.media.smokePuffShader );	
	}
//	CG_VehicleMuzzleFlash( cent, &part[BP_PLANE_BODY], ci->parts[BP_PLANE_BODY], ci->vehicle );

//	CG_PlaneFlags( cent );

}


/*
===============
CG_Misc_GV
===============
*/

static void CG_Misc_GV( centity_t *cent ) 
{
	refEntity_t	    part[BP_PLANE_MAX_PARTS];
	refEntity_t		reticle;
//	qboolean	    shadow;
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
//    CG_PlayerSprites( cent );
    
    // add the shadow
//    shadow = CG_GroundVehicleShadow( cent, &shadowPlane );

//    if ( cg_shadows.integer == 3 && shadow ) {
//    	renderfx |= RF_SHADOW_PLANE;
//    }
    renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all

    //
    // add the hull
    //
    part[BP_GV_BODY].hModel = availableVehicles[cent->currentState.modelindex].handle[BP_GV_BODY];
    VectorCopy( cent->lerpOrigin, part[BP_GV_BODY].origin );
    VectorCopy( cent->lerpOrigin, part[BP_GV_BODY].lightingOrigin );
    part[BP_GV_BODY].shadowPlane = shadowPlane;
    part[BP_GV_BODY].renderfx = renderfx;
    VectorCopy (part[BP_GV_BODY].origin, part[BP_GV_BODY].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_BODY] );

    //
    // turret
    //
    part[BP_GV_TURRET].hModel = availableVehicles[cent->currentState.modelindex].handle[BP_GV_TURRET];
    VectorCopy( cent->lerpOrigin, part[BP_GV_TURRET].lightingOrigin );
	AxisCopy( axisDefault, part[BP_GV_TURRET].axis );
	RotateAroundYaw( part[BP_GV_TURRET].axis, cent->currentState.angles2[ROLL] );
	CG_PositionRotatedEntityOnTag( &part[BP_GV_TURRET], &part[BP_GV_BODY], 
		availableVehicles[cent->currentState.modelindex].handle[BP_GV_BODY], gv_tags[BP_GV_TURRET] );
	part[BP_GV_TURRET].shadowPlane = shadowPlane;
    part[BP_GV_TURRET].renderfx = renderfx;
    VectorCopy (part[BP_GV_TURRET].origin, part[BP_GV_TURRET].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_TURRET] );

    //
    // gun
    //
    part[BP_GV_GUNBARREL].hModel = availableVehicles[cent->currentState.modelindex].handle[BP_GV_GUNBARREL];
    VectorCopy( cent->lerpOrigin, part[BP_GV_GUNBARREL].lightingOrigin );
	AxisCopy( axisDefault, part[BP_GV_GUNBARREL].axis );
	RotateAroundPitch( part[BP_GV_GUNBARREL].axis, cent->currentState.angles2[PITCH] );
	CG_PositionRotatedEntityOnTag( &part[BP_GV_GUNBARREL], &part[BP_GV_TURRET], 
			availableVehicles[cent->currentState.modelindex].handle[BP_GV_TURRET], gv_tags[BP_GV_GUNBARREL] );
	part[BP_GV_GUNBARREL].shadowPlane = shadowPlane;
    part[BP_GV_GUNBARREL].renderfx = renderfx;
    VectorCopy (part[BP_GV_GUNBARREL].origin, part[BP_GV_GUNBARREL].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_GUNBARREL] );

	// sound
	if( cent->currentState.eFlags & EF_PILOT_ONBOARD ) {
		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );
		tanksound = speed * NUM_TANKSOUNDS / availableVehicles[cent->currentState.modelindex].maxspeed;
		if( tanksound >= NUM_TANKSOUNDS ) tanksound = NUM_TANKSOUNDS - 1;
		trap_S_AddLoopingSound( cent->currentState.number, 
								cent->lerpOrigin, 
								velocity, 
								cgs.media.engineTank[tanksound] );
	}

	// smoke 
	if( cent->currentState.generic1 ) {
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
	// muzzleflash
//	CG_VehicleMuzzleFlash( cent, &part[BP_GV_GUNBARREL], ci->parts[BP_GV_GUNBARREL], ci->vehicle );

	// CTF
//	CG_GroundVehicleFlags( cent );
}


void CG_Misc_Vehicle( centity_t *cent ) 
{
	if( (availableVehicles[cent->currentState.modelindex].id&CAT_ANY) & CAT_PLANE ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR)) {
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Misc_Plane( cent );
	}
	else if( (availableVehicles[cent->currentState.modelindex].id&CAT_ANY) & CAT_GROUND ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND)) {
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Misc_GV( cent );
	}
	else {
		trap_Error( "Error: CG_Misc_Vehicle got wrong CAT!\n" );
	}
}

