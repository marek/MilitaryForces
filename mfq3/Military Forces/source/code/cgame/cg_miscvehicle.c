/*
 * $Id: cg_miscvehicle.c,v 1.1 2005-08-22 15:41:16 thebjoern Exp $
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
/*	if( cent->currentState.generic1 && cent->miscTime > cg.time ) {
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
	}*/
}


/*
===============
CG_Misc_GV
===============
*/

static void CG_Misc_GV( centity_t *cent ) 
{
	vec3_t			velocity;
	DrawInfo_GV_t	drawInfo;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = cent->currentState.modelindex;
	drawInfo.basicInfo.ONOFF = cent->currentState.ONOFF;

	// get velocity
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

    // add the talk baloon or disconnect icon
//    CG_PlayerSprites( cent );
    
	// loadout
	drawInfo.basicInfo.loadout = &cg_loadouts[cent->currentState.number];

	// turret/gun angle
	drawInfo.gunAngle = cent->currentState.angles2[PITCH];
	drawInfo.turretAngle = cent->currentState.angles2[ROLL];

	// wheels
	if( (availableVehicles[cent->currentState.modelindex].caps & HC_WHEELS) ) {
		int timediff = cg.time - cent->gearAnimStartTime;
		float dist = drawInfo.basicInfo.speed * timediff / 1000;
		vec3_t v1, v2;
		float dot;
		AngleVectors( cent->currentState.angles, v1, 0, 0 );
		VectorCopy( cent->currentState.pos.trDelta, v2 );
		VectorNormalize( v2 );
		dot = DotProduct( v1, v2 );
		dist = ( (dist / availableVehicles[cent->currentState.modelindex].wheelCF)*360);
		cent->gearAnim += (dot > 0 ? dist : -dist);
		cent->gearAnim = AngleMod( cent->gearAnim );
		cent->gearAnimStartTime = cg.time;
		drawInfo.wheelAngle = (float)cent->gearAnim;
	}

	// muzzleflash
	if( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = qtrue;
		drawInfo.basicInfo.flashWeaponIndex = cent->muzzleFlashWeapon;
	}


	// smoke
//	if( cent->currentState.generic1 ) {
//		CG_Generic_Smoke( cent, cent->lerpOrigin, 100 );
//	}
	
	// draw plane
	CG_DrawGV(&drawInfo);

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
}

/*
===============
CG_Misc_Helo
===============
*/

static void CG_Misc_Helo( centity_t *cent ) 
{
	vec3_t			velocity;
	DrawInfo_Helo_t	drawInfo;
	int i;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = cent->currentState.modelindex;
	drawInfo.basicInfo.ONOFF = cent->currentState.ONOFF;

	// get velocity
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
   
	// loadout
	drawInfo.basicInfo.loadout = &cg_loadouts[cent->currentState.number];

	// turret/gun angle
	for(i = 0; i < 4; i++)
	{
		drawInfo.gunAngle[i] = cent->currentState.angles2[PITCH];
		drawInfo.turretAngle[i] = cent->currentState.angles2[ROLL];
	}

	// muzzleflash
	if( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = qtrue;
		drawInfo.basicInfo.flashWeaponIndex = cent->muzzleFlashWeapon;
	}

	
	// draw helo
	CG_DrawHelo(&drawInfo);

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
	vec3_t			velocity;
	DrawInfo_Boat_t	drawInfo;
	int i;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = cent->currentState.modelindex;
	drawInfo.basicInfo.ONOFF = cent->currentState.ONOFF;

	// get velocity
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
   
	// loadout
	drawInfo.basicInfo.loadout = &cg_loadouts[cent->currentState.number];

	// turret/gun angle
	for(i = 0; i < 4; i++)
	{
		drawInfo.gunAngle[i] = cent->currentState.angles2[PITCH];
		drawInfo.turretAngle[i] = cent->currentState.angles2[ROLL];
	}

	// muzzleflash
	if( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = qtrue;
		drawInfo.basicInfo.flashWeaponIndex = cent->muzzleFlashWeapon;
	}

	// draw boat
	CG_DrawBoat(&drawInfo);

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
}



/*
===============
CG_Misc_GI
===============
*/

static void CG_Misc_GI( centity_t *cent ) 
{
	DrawInfo_GI_t	drawInfo;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = cent->currentState.modelindex2;
	drawInfo.basicInfo.ONOFF = cent->currentState.ONOFF;

	// entitynum
	drawInfo.basicInfo.entityNum = cent->currentState.number;

    // get the rotation information
    VectorCopy( cent->currentState.angles, cent->lerpAngles );
    AnglesToAxis( cent->lerpAngles, drawInfo.basicInfo.axis );

	// position and orientation
	VectorCopy( cent->lerpOrigin, drawInfo.basicInfo.origin );
	VectorCopy( cent->lerpAngles, drawInfo.basicInfo.angles );

	// turret/gun angle
	drawInfo.gunAngle = cent->currentState.angles2[PITCH];
	drawInfo.turretAngle = cent->currentState.angles2[ROLL];

	// upgrades
	drawInfo.upgrades = availableGroundInstallations[drawInfo.basicInfo.vehicleIndex].upgrades;

	// draw plane
	CG_DrawGI(&drawInfo);

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
}






void CG_Misc_Vehicle( centity_t *cent ) 
{
	// check for ground installation
	if( cent->currentState.modelindex == 255 &&
		cent->currentState.modelindex2 >= 0 &&
		cent->currentState.modelindex2 < bg_numberOfGroundInstallations )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND)) {
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Misc_GI( cent );
	}
	// otherwise normal vehicle
	else if( availableVehicles[cent->currentState.modelindex].cat & CAT_PLANE ) {
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

