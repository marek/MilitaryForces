/*
 * $Id: cg_miscvehicle.c,v 1.10 2002-07-14 17:13:19 thebjoern Exp $
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
	vec3_t			velocity;	
	vec3_t			smokePosition, forward;
	DrawInfo_Plane_t drawInfo;
	int				ONOFF = cent->currentState.ONOFF;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = cent->currentState.modelindex;
	drawInfo.basicInfo.ONOFF = ONOFF;
		
	// get speed
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	drawInfo.basicInfo.speed = VectorLength( velocity );
	
	// entitynum
	drawInfo.basicInfo.entityNum = cent->currentState.number;

    // get the rotation information
    VectorCopy( cent->currentState.angles, cent->lerpAngles );
    AnglesToAxis( cent->lerpAngles, drawInfo.basicInfo.axis );

	// position and orientation
	VectorCopy( cent->lerpOrigin, drawInfo.basicInfo.origin );
	VectorCopy( cent->lerpAngles, drawInfo.basicInfo.angles );

	// throttle
	drawInfo.basicInfo.throttle = cent->currentState.frame;
    
	// control surfaces
	drawInfo.controlFrame = cent->currentState.vehicleAnim;
	
	// gear
	if( availableVehicles[cent->currentState.modelindex].caps & HC_GEAR ) {
		int timediff = cg.time - cent->gearAnimStartTime;
		int geardown = availableVehicles[cent->currentState.modelindex].maxGearFrame;
		if( ONOFF & OO_GEAR ) {
			drawInfo.controlFrame += 9;
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
		drawInfo.gearFrame = cent->gearAnimFrame;
	}
	
	// bay
	if( availableVehicles[cent->currentState.modelindex].caps & HC_WEAPONBAY ) {
		int timediff = cg.time - cent->bayAnimStartTime;
		int baydown = availableVehicles[cent->currentState.modelindex].maxBayFrame;
		if( cent->bayAnim == BAY_ANIM_UP ) {
			cent->bayAnimFrame = baydown - timediff/25;
			if( cent->bayAnimFrame < BAY_UP ) {
				cent->bayAnimFrame = BAY_UP;
				cent->bayAnim = BAY_ANIM_STOP;
			}
		} else if( cent->bayAnim == BAY_ANIM_DOWN ) {
			cent->bayAnimFrame = BAY_UP + timediff/25;
			if( cent->bayAnimFrame > baydown ) {
				cent->bayAnimFrame = baydown;
				cent->bayAnim = BAY_ANIM_STOP;
			}
		}
		drawInfo.bayFrame = cent->bayAnimFrame;
	}

	// speedbrakes
	if( availableVehicles[cent->currentState.modelindex].caps & HC_SPEEDBRAKE ) {
		if( ONOFF & OO_SPEEDBRAKE ) {
			drawInfo.brakeFrame = 1;
		}
		else {
			drawInfo.brakeFrame = 0;
		}
	}

	// cockpit
	if( drawInfo.basicInfo.speed < 1 ) {
		drawInfo.cockpitFrame = 1;
	} 

	// body
	if( cent->currentState.frame <= 10 ) {
		drawInfo.bodyFrame = 0;
	} else {
		drawInfo.bodyFrame = ( cent->currentState.frame > 12 ? 2 : 1 );
	}

	// swingwings
	if( availableVehicles[cent->currentState.modelindex].caps & HC_SWINGWING ) {
		drawInfo.swingAngle = cent->currentState.angles2[PITCH];
	}

	// loadout
	drawInfo.basicInfo.loadout = &cg_loadouts[cent->currentState.number];

	// smoke
	if( cent->currentState.generic1 ) {
		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorMA( cent->lerpOrigin, availableVehicles[cent->currentState.modelindex].mins[0], forward, smokePosition );
		CG_Generic_Smoke( cent, smokePosition, 10 );
	}

	// muzzleflash
	if( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = qtrue;
		drawInfo.basicInfo.flashWeaponIndex = cent->muzzleFlashWeapon;
	}
	// muzzleflash
	if( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = qtrue;
		drawInfo.basicInfo.flashWeaponIndex = cent->muzzleFlashWeapon;
	}

	// draw plane
	CG_DrawPlane(&drawInfo);
	
	// flags
//	CG_PlaneFlags( cent );

	// smoke
	if( cent->currentState.generic1 && cent->miscTime > cg.time ) {
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
		cent->miscTime = cg.time + 10;
	}
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
	if ( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
//		CG_VehicleMuzzleFlash( cent->muzzleFlashWeapon, &part[BP_GV_GUNBARREL], ci->parts[BP_GV_GUNBARREL], ci->vehicle );
	}

	// CTF
//	CG_GroundVehicleFlags( cent );
}

/*
===============
CG_Misc_Helo
===============
*/

static void CG_Misc_Helo( centity_t *cent ) 
{

}

/*
===============
CG_Misc_LQM
===============
*/

static void CG_Misc_LQM( centity_t *cent ) 
{

}

/*
===============
CG_Misc_Boat
===============
*/

static void CG_Misc_Boat( centity_t *cent ) 
{

}


void CG_Misc_Vehicle( centity_t *cent ) 
{
	if( availableVehicles[cent->currentState.modelindex].cat & CAT_PLANE ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR)) {
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Misc_Plane( cent );
	}
	else if( availableVehicles[cent->currentState.modelindex].cat & CAT_GROUND ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND)) {
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Misc_GV( cent );
	}
	else if( availableVehicles[cent->currentState.modelindex].cat & CAT_HELO ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR)) {
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Misc_Helo( cent );
	}
	else if( availableVehicles[cent->currentState.modelindex].cat & CAT_LQM ) {
		CG_Misc_LQM( cent );
	}
	else if( availableVehicles[cent->currentState.modelindex].cat & CAT_BOAT ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND)) {
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Misc_Boat( cent );
	}
	else {
		trap_Error( "Error: CG_Misc_Vehicle got wrong CAT!\n" );
	}
}

