/*
 * $Id: cg_miscvehicle.c,v 1.5 2005-11-20 11:21:38 thebjoern Exp $
*/

#include "cg_local.h"


extern char *plane_tags[BP_PLANE_MAX_PARTS];
extern char *gv_tags[BP_GV_MAX_PARTS];

/*
===============
CG_Misc_Plane
===============
*/

void ClientEntity::misc_Plane() 
{
	vec3_t			velocity;	
	vec3_t			smokePosition, forward;
	DrawInfo_Plane_t drawInfo;
	int				ONOFF = this->currentState.ONOFF;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = this->currentState.modelindex;
	drawInfo.basicInfo.ONOFF = ONOFF;
		
	// get speed
	VectorCopy( this->currentState.pos.trDelta, velocity );
	drawInfo.basicInfo.speed = VectorLength( velocity );
	
	// entitynum
	drawInfo.basicInfo.entityNum = this->currentState.number;

    // get the rotation information
    this->vehicleLerpAngles();
    AnglesToAxis( this->lerpAngles, drawInfo.basicInfo.axis );

	// position and orientation
	VectorCopy( this->lerpOrigin, drawInfo.basicInfo.origin );
	VectorCopy( this->lerpAngles, drawInfo.basicInfo.angles );

	// throttle
	drawInfo.basicInfo.throttle = this->currentState.frame;
    
	// control surfaces
	drawInfo.controlFrame = this->currentState.vehicleAnim;
	
	// gear
	if( availableVehicles[this->currentState.modelindex].caps & HC_GEAR ) {
		int timediff = cg.time - this->gearAnimStartTime;
		int geardown = availableVehicles[this->currentState.modelindex].maxGearFrame;
		if( ONOFF & OO_GEAR ) {
			drawInfo.controlFrame += 9;
		}
		if( this->gearAnim == GEAR_ANIM_UP ) {
			this->gearAnimFrame = geardown - timediff/25;
			if( this->gearAnimFrame < GEAR_UP ) {
				this->gearAnimFrame = GEAR_UP;
				this->gearAnim = GEAR_ANIM_STOP;
			}
		} else if( this->gearAnim == GEAR_ANIM_DOWN ) {
			this->gearAnimFrame = GEAR_UP + timediff/25;
			if( this->gearAnimFrame > geardown ) {
				this->gearAnimFrame = geardown;
				this->gearAnim = GEAR_ANIM_STOP;
			}
		}
		drawInfo.gearFrame = this->gearAnimFrame;
	}
	
	// bay
	if( availableVehicles[this->currentState.modelindex].caps & HC_WEAPONBAY ) {
		int timediff = cg.time - this->bayAnimStartTime;
		int baydown = availableVehicles[this->currentState.modelindex].maxBayFrame;
		if( this->bayAnim == BAY_ANIM_UP ) {
			this->bayAnimFrame = baydown - timediff/25;
			if( this->bayAnimFrame < BAY_UP ) {
				this->bayAnimFrame = BAY_UP;
				this->bayAnim = BAY_ANIM_STOP;
			}
		} else if( this->bayAnim == BAY_ANIM_DOWN ) {
			this->bayAnimFrame = BAY_UP + timediff/25;
			if( this->bayAnimFrame > baydown ) {
				this->bayAnimFrame = baydown;
				this->bayAnim = BAY_ANIM_STOP;
			}
		}
		drawInfo.bayFrame = this->bayAnimFrame;
	}

	// speedbrakes
	if( availableVehicles[this->currentState.modelindex].caps & HC_SPEEDBRAKE ) {
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
	if( this->currentState.frame <= 10 ) {
		drawInfo.bodyFrame = 0;
	} else {
		drawInfo.bodyFrame = ( this->currentState.frame > 12 ? 2 : 1 );
	}

	// swingwings
	if( availableVehicles[this->currentState.modelindex].caps & HC_SWINGWING ) {
		drawInfo.swingAngle = this->lerpAngles2[PITCH];
	}

	// loadout
	this->setDrawLoadout( &drawInfo.basicInfo );

	// smoke
	if( this->currentState.generic1 ) {
		AngleVectors( this->lerpAngles, forward, nullptr, nullptr );
		VectorMA( this->lerpOrigin, availableVehicles[this->currentState.modelindex].mins[0], forward, smokePosition );
		this->generic_Smoke( smokePosition, 10 );
	}

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}
	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}

	// draw plane
	CG_DrawPlane(&drawInfo);
	
	// flags
//	CG_PlaneFlags( this );

	// smoke
/*	if( this->currentState.generic1 && this->miscTime > cg.time ) {
		LocalEntity	*smoke;
		vec3_t			up = {0, 0, 1};
		vec3_t			pos;
		vec3_t			forward;

		AngleVectors( this->lerpAngles, forward, NULL, NULL );
		VectorMA( this->lerpOrigin, availableVehicles[this->currentState.modelindex].mins[0], forward, pos );

		smoke = CG_SmokePuff( pos, up, 
					  this->currentState.generic1, 
					  0.5, 0.5, 0.5, 0.66f,
					  100*this->currentState.generic1, 
					  cg.time, 0,
					  LEF_PUFF_DONT_SCALE, 
					  cgs.media.smokePuffShader );	
		this->miscTime = cg.time + 10;
	}*/
}


/*
===============
CG_Misc_GV
===============
*/

void ClientEntity::misc_GV() 
{
	vec3_t			velocity;
	DrawInfo_GV_t	drawInfo;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = this->currentState.modelindex;
	drawInfo.basicInfo.ONOFF = this->currentState.ONOFF;

	// get velocity
	VectorCopy( this->currentState.pos.trDelta, velocity );
	drawInfo.basicInfo.speed = VectorLength( velocity );

	// entitynum
	drawInfo.basicInfo.entityNum = this->currentState.number;

    // get the rotation information
    this->vehicleLerpAngles();
    AnglesToAxis( this->lerpAngles, drawInfo.basicInfo.axis );

	// position and orientation
	VectorCopy( this->lerpOrigin, drawInfo.basicInfo.origin );
	VectorCopy( this->lerpAngles, drawInfo.basicInfo.angles );

	// loadout
	this->setDrawLoadout( &drawInfo.basicInfo );

	// turret/gun angle
	drawInfo.gunAngle = this->lerpAngles2[PITCH];
	drawInfo.turretAngle = this->lerpAngles2[ROLL];

	// wheels
	if( (availableVehicles[this->currentState.modelindex].caps & HC_WHEELS) ) {
		int timediff = cg.time - this->gearAnimStartTime;
		float dist = drawInfo.basicInfo.speed * timediff / 1000;
		vec3_t v1, v2;
		float dot;
		AngleVectors( this->currentState.angles, v1, nullptr, nullptr );
		VectorCopy( this->currentState.pos.trDelta, v2 );
		VectorNormalize( v2 );
		dot = DotProduct( v1, v2 );
		dist = ( (dist / availableVehicles[this->currentState.modelindex].wheelCF)*360);
		this->gearAnim += (dot > 0 ? dist : -dist);
		this->gearAnim = AngleMod( this->gearAnim );
		this->gearAnimStartTime = cg.time;
		drawInfo.wheelAngle = static_cast<float>(this->gearAnim);
	}

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}


	// smoke
//	if( this->currentState.generic1 ) {
//		this->generic_Smoke( this->lerpOrigin, 100 );
//	}
	
	// draw plane
	CG_DrawGV(&drawInfo);

	// smoke 
	if( this->currentState.generic1 ) {
		LocalEntity	*smoke;
		vec3_t			up = {0, 0, 20};
		vec3_t			pos;
		vec3_t			forward;

		AngleVectors( this->lerpAngles, forward, nullptr, nullptr );
		VectorCopy( this->lerpOrigin, pos );
		pos[2] += 12;

		smoke = CG_SmokePuff( pos, up, 
					  this->currentState.generic1, 
					  0.5, 0.5, 0.5, 0.66f,
					  200*this->currentState.generic1, 
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

void ClientEntity::misc_Helo() 
{
	vec3_t			velocity;
	DrawInfo_Helo_t	drawInfo;
	int i;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = this->currentState.modelindex;
	drawInfo.basicInfo.ONOFF = this->currentState.ONOFF;

	// get velocity
	VectorCopy( this->currentState.pos.trDelta, velocity );
	drawInfo.basicInfo.speed = VectorLength( velocity );

	// entitynum
	drawInfo.basicInfo.entityNum = this->currentState.number;

    // get the rotation information
    this->vehicleLerpAngles();
    AnglesToAxis( this->lerpAngles, drawInfo.basicInfo.axis );

	// position and orientation
	VectorCopy( this->lerpOrigin, drawInfo.basicInfo.origin );
	VectorCopy( this->lerpAngles, drawInfo.basicInfo.angles );
   
	// loadout
	this->setDrawLoadout( &drawInfo.basicInfo );

	// turret/gun angle
	for(i = 0; i < 4; i++)
	{
		drawInfo.gunAngle[i] = this->lerpAngles2[PITCH];
		drawInfo.turretAngle[i] = this->lerpAngles2[ROLL];
	}

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}

	
	// draw helo
	CG_DrawHelo(&drawInfo);

	// smoke 
	if( this->currentState.generic1 ) {
		LocalEntity	*smoke;
		vec3_t			up = {0, 0, 20};
		vec3_t			pos;
		vec3_t			forward;

		AngleVectors( this->lerpAngles, forward, nullptr, nullptr );
		VectorCopy( this->lerpOrigin, pos );
		pos[2] += 12;

		smoke = CG_SmokePuff( pos, up, 
					  this->currentState.generic1, 
					  0.5, 0.5, 0.5, 0.66f,
					  200*this->currentState.generic1, 
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

void ClientEntity::misc_LQM() 
{

}

/*
===============
CG_Misc_Boat
===============
*/

void ClientEntity::misc_Boat() 
{
	vec3_t			velocity;
	DrawInfo_Boat_t	drawInfo;
	int i;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = this->currentState.modelindex;
	drawInfo.basicInfo.ONOFF = this->currentState.ONOFF;

	// get velocity
	VectorCopy( this->currentState.pos.trDelta, velocity );
	drawInfo.basicInfo.speed = VectorLength( velocity );

	// entitynum
	drawInfo.basicInfo.entityNum = this->currentState.number;

    // get the rotation information
    this->vehicleLerpAngles();
    AnglesToAxis( this->lerpAngles, drawInfo.basicInfo.axis );

	// position and orientation
	VectorCopy( this->lerpOrigin, drawInfo.basicInfo.origin );
	VectorCopy( this->lerpAngles, drawInfo.basicInfo.angles );
   
	// loadout
	this->setDrawLoadout( &drawInfo.basicInfo );

	// turret/gun angle
	for(i = 0; i < 4; i++)
	{
		drawInfo.gunAngle[i] = this->lerpAngles2[PITCH];
		drawInfo.turretAngle[i] = this->lerpAngles2[ROLL];
	}

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}

	// draw boat
	CG_DrawBoat(&drawInfo);

	// smoke 
	if( this->currentState.generic1 ) {
		LocalEntity	*smoke;
		vec3_t			up = {0, 0, 20};
		vec3_t			pos;
		vec3_t			forward;

		AngleVectors( this->lerpAngles, forward, nullptr, nullptr );
		VectorCopy( this->lerpOrigin, pos );
		pos[2] += 12;

		smoke = CG_SmokePuff( pos, up, 
					  this->currentState.generic1, 
					  0.5, 0.5, 0.5, 0.66f,
					  200*this->currentState.generic1, 
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

void ClientEntity::misc_GI() 
{
	DrawInfo_GI_t	drawInfo;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = this->currentState.modelindex2;
	drawInfo.basicInfo.ONOFF = this->currentState.ONOFF;

	// entitynum
	drawInfo.basicInfo.entityNum = this->currentState.number;

    // get the rotation information
    this->vehicleLerpAngles();
    AnglesToAxis( this->lerpAngles, drawInfo.basicInfo.axis );

	// position and orientation
	VectorCopy( this->lerpOrigin, drawInfo.basicInfo.origin );
	VectorCopy( this->lerpAngles, drawInfo.basicInfo.angles );

	// turret/gun angle
	drawInfo.gunAngle = this->lerpAngles2[PITCH];
	drawInfo.turretAngle = this->lerpAngles2[ROLL];

	// upgrades
	drawInfo.upgrades = availableGroundInstallations[drawInfo.basicInfo.vehicleIndex].upgrades;

	// draw plane
	CG_DrawGI(&drawInfo);

	// smoke 
	if( this->currentState.generic1 ) {
		LocalEntity	*smoke;
		vec3_t			up = {0, 0, 20};
		vec3_t			pos;
		vec3_t			forward;

		AngleVectors( this->lerpAngles, forward, nullptr, nullptr );
		VectorCopy( this->lerpOrigin, pos );
		pos[2] += 12;

		smoke = CG_SmokePuff( pos, up, 
					  this->currentState.generic1, 
					  0.5, 0.5, 0.5, 0.66f,
					  200*this->currentState.generic1, 
					  cg.time, 0,
					  LEF_PUFF_DONT_SCALE, 
					  cgs.media.smokePuffShader );	
	}
}






void ClientEntity::misc_Vehicle() 
{
	// check for ground installation
	if( this->currentState.modelindex == 255 &&
		this->currentState.modelindex2 >= 0 &&
		this->currentState.modelindex2 < bg_numberOfGroundInstallations )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND)) {
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->misc_GI();
	}
	// otherwise normal vehicle
	else if( availableVehicles[this->currentState.modelindex].cat & CAT_PLANE ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR)) {
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->misc_Plane();
	}
	else if( availableVehicles[this->currentState.modelindex].cat & CAT_GROUND ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND)) {
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->misc_GV();
	}
	else if( availableVehicles[this->currentState.modelindex].cat & CAT_HELO ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR)) {
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->misc_Helo();
	}
	else if( availableVehicles[this->currentState.modelindex].cat & CAT_LQM ) {
		this->misc_LQM();
	}
	else if( availableVehicles[this->currentState.modelindex].cat & CAT_BOAT ) {
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND)) {
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->misc_Boat();
	}
	else {
		Common::error( ERR_DROP, "Error: CG_Misc_Vehicle got wrong CAT!\n" );
	}
}

