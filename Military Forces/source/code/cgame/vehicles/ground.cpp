#include "../cg_local.h"
#include <algorithm>

/*
 * $Id: cg_groundvehicle.c,v 1.7 2005-11-21 17:28:20 thebjoern Exp $
*/





/*
==========================
CGame::registerGroundVehicle
==========================
*/
void CGame::registerGroundVehicle( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
/*
		if( !ci->parts[i] )
		{
			CGame::printf( "Informational: RegisterGroundVehicle failed register part %d of vehicle %d (id %x)\n", i, 
				ci->vehicle, availableVehicles[ci->vehicle].id );
		}
*/
	}
}


/*
===============
CG_GroundVehicleFlags
===============
*/
void ClientEntity::groundVehicleFlags() {
	int		powerups;
	clientInfo_t	*ci;

	powerups = this->currentState.objectives;
	if ( !powerups ) {
		return;
	}

	ci = &cgs.clientinfo[ this->currentState.clientNum ];

	// redflag
	if ( powerups & OB_REDFLAG ) {
		this->trailItem( cgs.media.redFlagModel );
//		refExport.AddLightToScene( this->lerpOrigin, 200 + (rand()&31), 1.0, 0.2f, 0.2f );
	}

	// blueflag
	if ( powerups & OB_BLUEFLAG ) {
		this->trailItem( cgs.media.blueFlagModel );
//		refExport.AddLightToScene( this->lerpOrigin, 200 + (rand()&31), 0.2f, 0.2f, 1.0 );
	}
}

/*
===============
CG_GroundVehicle
===============
*/
void ClientEntity::groundVehicle( clientInfo_t *ci ) 
{
	Vec3			velocity;
	DrawInfo_GV_t	drawInfo;

	drawInfo = {};
	drawInfo.basicInfo.vehicleIndex = ci->vehicle;
	drawInfo.basicInfo.team = cgs.gametype >= GT_TEAM ? (ci->paintTeam ? ci->paintTeam : ci->team) : 0;
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
	drawInfo.basicInfo.usedLoadout = nullptr;//&cg_loadouts[this->currentState.number];

	// turret/gun angle
	drawInfo.gunAngle = this->lerpAngles2[PITCH];
	drawInfo.turretAngle = this->lerpAngles2[ROLL];
	drawInfo.turretGone = (availableVehicles[drawInfo.basicInfo.vehicleIndex].flags & VF_COOKOFF) && (this->currentState.eFlags & EF_DEAD);

	// wheels
	if( (availableVehicles[ci->vehicle].caps & HC_WHEELS) ) {
		int timediff = cg.time - this->gearAnimStartTime;
		float dist = drawInfo.basicInfo.speed * timediff / 1000;
		Vec3 v1, v2;
		float dot;
		AngleVectors( this->currentState.angles, v1, nullptr, nullptr );
		VectorCopy( this->currentState.pos.trDelta, v2 );
		VectorNormalize( v2 );
		dot = DotProduct( v1, v2 );
		dist = ( (dist / availableVehicles[ci->vehicle].wheelCF)*360);
		this->gearAnim += (dot > 0 ? dist : -dist);
		this->gearAnim = AngleMod( this->gearAnim );
		this->gearAnimStartTime = cg.time;
		drawInfo.wheelAngle = static_cast<float>(this->gearAnim);
	
		drawInfo.wheelDirection = this->currentState.vehicleAnim;
	}

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}


	// smoke
	if( this->currentState.generic1 ) {
		this->generic_Smoke( this->lerpOrigin, 100 );
	}
	
	// draw plane
	CGame::drawGV(&drawInfo);

/*
		AngleVectors( self->client->ps.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, self->client->ps.turretAngle );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, self->client->ps.gunAngle );
*/

	// CTF
	this->groundVehicleFlags();

	// reticles
	if( this == &cg.predictedPlayerEntity )
	{
		refEntity_t		reticle;
		Vec3	forward, right, up, ang, start, end, temp;
		trace_t	tr;
		float len;
		playerState_t* ps = &cg.snap->ps;
		float mindist = cg_thirdPersonRange.integer + availableVehicles[ci->vehicle].cam_dist[ CAMERA_V_DEFAULT ] + availableVehicles[ci->vehicle].maxs[0] + 20;

		reticle = {};

		CGame::resetReticles();

		// are we tracking a target?
		if( ps->stats[STAT_LOCKINFO] & LI_TRACKING )
		{
			refEntity_t reticlelock;
			bool building = false;
			ClientEntity* target = &cg_entities[this->currentState.tracktarget];
			
			if( target->currentState.eType == ET_EXPLOSIVE )
			{
				building = true;
			}

			reticle.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairtrack;
			reticle.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairtrack];

			if( building )
			{
				VectorSubtract( cgs.inlineModelMidpoints[target->currentState.modelindex], this->lerpOrigin, forward );
			}
			else
			{
				forward = target->lerpOrigin - this->lerpOrigin;
			}

			len = VectorNormalize( forward );
			vectoangles( forward, ang );
			AnglesToAxis( ang, reticle.axis );
			end = this->lerpOrigin + forward * ( 2000 );
			CGame::trace( &tr, this->lerpOrigin, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
			
			if( tr.entityNum > ENTITYNUM_WORLD && tr.entityNum < MAX_CLIENTS )
			{
				cg.crosshairClientNum = tr.entityNum;
				cg.crosshairClientTime = cg.time;
			}

			VectorCopy( tr.endpos, end );
			CGame::trace( &tr, cg.refdef.vieworg, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
			VectorSubtract( tr.endpos, cg.refdef.vieworg, forward );
			len = VectorNormalize(forward);
			
			if( len > mindist )
			{
				VectorMA( cg.refdef.vieworg, mindist, forward, reticle.origin );
			}
			else
			{
				VectorMA( cg.refdef.vieworg, len - 5, forward, reticle.origin );
			}

			VectorCopy( reticle.origin, reticle.lightingOrigin );
			reticle.renderfx = RF_LIGHTING_ORIGIN|RF_SHADOW_PLANE;

			CGame::addReticleEntityToScene( &reticle, target );

			// are we locked onto this target?
			if( ps->stats[STAT_LOCKINFO] & LI_LOCKING )
			{
				// copy the reticle entity as our reticle-lock entity
				reticlelock = reticle;
				
				reticlelock.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairlock;
				reticlelock.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairlock];
				reticlelock.frame = 1;
			}
			else
			{
				reticlelock = {};	
				
				reticlelock.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairtrack;
				reticlelock.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairtrack];
				
				AngleVectors( this->currentState.angles, forward, right, up );
				RotatePointAroundVector( temp, up, forward, this->lerpAngles2[ROLL] );
				CrossProduct( up, temp, right );
				RotatePointAroundVector( forward, right, temp, this->lerpAngles2[PITCH] );
				start = this->lerpOrigin + forward * ( availableVehicles[ci->vehicle].gunoffset[0] );
				start = start + right * ( availableVehicles[ci->vehicle].gunoffset[1] );
				start = start + up * ( availableVehicles[ci->vehicle].gunoffset[2] );
				end = start + forward * ( 2000 );
				vectoangles( forward, ang );
//				ang[2] = 0;
				AnglesToAxis( ang, reticlelock.axis );
				VectorScale( reticlelock.axis[0], 10.0f, reticlelock.axis[0] );
				VectorScale( reticlelock.axis[1], 10.0f, reticlelock.axis[1] );
				VectorScale( reticlelock.axis[2], 10.0f, reticlelock.axis[2] );
				reticlelock.nonNormalizedAxes = true;
				CGame::trace( &tr, start, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
				VectorCopy( tr.endpos, end );
				CGame::trace( &tr, cg.refdef.vieworg, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
				VectorSubtract( tr.endpos, cg.refdef.vieworg, forward );
				len = VectorNormalize(forward);

				if( len > mindist )
				{
					VectorMA( cg.refdef.vieworg, mindist, forward, reticlelock.origin );
				}
				else
				{
					VectorMA( cg.refdef.vieworg, len - 5, forward, reticlelock.origin );
				}

				VectorCopy( reticlelock.origin, reticlelock.lightingOrigin );
				reticlelock.renderfx = RF_LIGHTING_ORIGIN|RF_SHADOW_PLANE;
			}

			CGame::addReticleEntityToScene( &reticlelock, nullptr );
		}
		else
		{
			reticle.customShader = CGame::reticleOf( ci->vehicle, this->currentState.weaponNum );
			reticle.hModel = cgs.media.reticle[reticle.customShader];

			AngleVectors( this->currentState.angles, forward, right, up );
			RotatePointAroundVector( temp, up, forward, this->lerpAngles2[ROLL] );
			CrossProduct( up, temp, right );
			RotatePointAroundVector( forward, right, temp, this->lerpAngles2[PITCH] );
			start = this->lerpOrigin + forward * ( availableVehicles[ci->vehicle].gunoffset[0] );
			start = start + right * ( availableVehicles[ci->vehicle].gunoffset[1] );
			start = start + up * ( availableVehicles[ci->vehicle].gunoffset[2] );
			end = start + forward * ( 2000 );
			vectoangles( forward, ang );
			ang[2] = 0;
			AnglesToAxis( ang, reticle.axis );
			CGame::trace( &tr, start, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 

			if( tr.entityNum > ENTITYNUM_WORLD && tr.entityNum < MAX_CLIENTS )
			{
				cg.crosshairClientNum = tr.entityNum;
				cg.crosshairClientTime = cg.time;
			}

			VectorCopy( tr.endpos, end );
			CGame::trace( &tr, cg.refdef.vieworg, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
			VectorSubtract( tr.endpos, cg.refdef.vieworg, forward );
			len = VectorNormalize(forward);
			
			if( len > mindist )
			{
				VectorMA( cg.refdef.vieworg, mindist, forward, reticle.origin );
			}
			else
			{
				VectorMA( cg.refdef.vieworg, len - 5, forward, reticle.origin );
			}

			VectorCopy( reticle.origin, reticle.lightingOrigin );
			reticle.renderfx = RF_LIGHTING_ORIGIN|RF_SHADOW_PLANE;
			
			CGame::addReticleEntityToScene( &reticle, nullptr );
		}
	}
}

/*
=============
CGame::groundVehicleObituary

General ugly function, needs to be made prettier some day...
=============
*/
void CGame::groundVehicleObituary( entityState_t *ent, clientInfo_t *ci ) 
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
	    attackerInfo = nullptr;
    } else {
	    attackerInfo = CGame::configString( CS_PLAYERS + attacker );
    }

    targetInfo = CGame::configString( CS_PLAYERS + target );
    if ( !targetInfo ) {
	    return;
    }
#pragma message("remove the problem checking here!")
	if( Info::valueForKey( targetInfo, "n" ) != nullptr ) {
		Q_strncpyz( targetName, Info::valueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	}
	else {
		Common::error( ERR_DROP, "MFQ3 Error (3): Invalid targetinfo\n" );
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
	    message = nullptr;
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
	    CGame::printf( "%s %s.\n", targetName, message);
	    return;
    }

    // check for kill messages from the current clientNum
    if ( attacker == cg.snap->ps.clientNum ) {
	char	*s;

	if ( cgs.gametype < GT_TEAM ) {
		s = va("You fragged %s\n%s place with %i", targetName, 
			CGame::placeString( cg.snap->ps.persistant[PERS_RANK] + 1 ),
			cg.snap->ps.persistant[PERS_SCORE] );
	} else {
		s = va("You fragged %s", targetName );
	}
	CGame::centerPrint( s, SCREEN_HEIGHT * 0.25, BIGCHAR_WIDTH );

	// print the text message as well
    }

    // check for double client messages
    if ( !attackerInfo ) {
	    attacker = ENTITYNUM_WORLD;
	    strcpy( attackerName, "noname" );
    } else {
		if( Info::valueForKey( attackerInfo, "n" ) != nullptr ) {
			Q_strncpyz( attackerName, Info::valueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		}
		else {
			Common::error( ERR_DROP, "MFQ3 Error (1): Bad attackerInfo!\n" );
		}
	    strcat( attackerName, S_COLOR_WHITE );
	    // check for kill messages about the current clientNum
	    if ( target == cg.snap->ps.clientNum ) {
			if( attackerName != nullptr ) {
				Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
			}
			else {
				Common::error( ERR_DROP, "MFQ3 Error (2): No attackerName!\n" );
			}
	    }
    }

    if( attacker != ENTITYNUM_WORLD )
	{
		switch(mod)
		{

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
		case MOD_NUKE:
			message = "got owned with a MinkiCorp(r) brand Nuclear Weapon by";
		default:
			message = "was killed by";
			break;
		}

		if( message )
		{
			CGame::printf( "%s %s %s%s\n", targetName, message, attackerName, message2);
			return;
		}
    }

    // we don't know what it was
    CGame::printf( "%s died.\n", targetName );
}

/*
===============
CGame::cacheGroundVehicle

On startup cache it
===============
*/

void CGame::cacheGroundVehicle(int index)
{
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/ground/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );
// changed mg
	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_GV_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			break;
		case BP_GV_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_GV_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		case BP_GV_WHEEL:
			Com_sprintf( name, sizeof(name), "%s_w1.md3", basename );
			break;
		case BP_GV_WHEEL2:
			Com_sprintf( name, sizeof(name), "%s_w2.md3", basename );
			break;
		case BP_GV_WHEEL3:
			Com_sprintf( name, sizeof(name), "%s_w3.md3", basename );
			break;
		case BP_GV_WHEEL4:
			Com_sprintf( name, sizeof(name), "%s_w4.md3", basename );
			break;
		case BP_GV_WHEEL5:
			Com_sprintf( name, sizeof(name), "%s_w5.md3", basename );
			break;
		case BP_GV_WHEEL6:
			Com_sprintf( name, sizeof(name), "%s_w6.md3", basename );
			break;
		}
		availableVehicles[index].handle[i] = CGame::registerVehiclePart( name, 
			i >= BP_GV_WHEEL && ( availableVehicles[index].caps & HC_WHEELS ) && i - BP_GV_WHEEL < availableVehicles[index].wheels );
//		if( !availableVehicles[index].handle[i] ) {
//			CGame::printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_GV_BODY] ) {
		Common::printf( S_COLOR_YELLOW "WARNING: MFQ3: missing body %s.md3, model not loaded\n", basename );
	}
}

char *gv_tags[BP_GV_MAX_PARTS] =
{
	"<no tag>",		// vehicle body
	"tag_turret",	// turret
	"tag_weap",		// gun
	"tag_wheel",	// wheel
	"tag_wheel2",	// wheel
	"tag_wheel3",	// wheel
	"tag_wheel4",	// wheel
	"tag_wheel5",	// wheel
	"tag_wheel6"	// wheel
};

void CGame::drawGV(DrawInfo_GV_t* drawInfo)
{
	refEntity_t	    part[BP_GV_MAX_PARTS];
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				i, tanksound;

	completeVehicleData_t* veh = &availableVehicles[drawInfo->basicInfo.vehicleIndex];
	cg_drawTeam = drawInfo->basicInfo.team;

	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
	    memset( &part[i], 0, sizeof(part[0]) );	
	}

    // add the shadow
	switch( cg_shadows.integer )
	{
	case 1:
		CGame::genericShadow( &drawInfo->basicInfo, &shadowPlane );
		break;
	case 3:
		renderfx |= RF_SHADOW_PLANE;
		break;
	}

	// use the same origin for all
    renderfx |= RF_LIGHTING_ORIGIN;

    //
    // add the hull
    //
    part[BP_GV_BODY].hModel = veh->handle[BP_GV_BODY];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_BODY].origin );
    VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_BODY].lightingOrigin );
	AxisCopy( drawInfo->basicInfo.axis, part[BP_GV_BODY].axis );
    part[BP_GV_BODY].shadowPlane = shadowPlane;
    part[BP_GV_BODY].renderfx = renderfx;
    VectorCopy (part[BP_GV_BODY].origin, part[BP_GV_BODY].oldorigin);
    CGame::addVehiclePart( &part[BP_GV_BODY] );

	if( !drawInfo->turretGone )
	{
	    //
	    // turret
	    //
	    part[BP_GV_TURRET].hModel = veh->handle[BP_GV_TURRET];
	    VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_TURRET].lightingOrigin );
		AxisCopy( axisDefault, part[BP_GV_TURRET].axis );
		RotateAroundYaw( part[BP_GV_TURRET].axis, drawInfo->turretAngle );
		CGame::positionRotatedEntityOnTag( &part[BP_GV_TURRET], &part[BP_GV_BODY], veh->handle[BP_GV_BODY], gv_tags[BP_GV_TURRET] );
		part[BP_GV_TURRET].shadowPlane = shadowPlane;
	    part[BP_GV_TURRET].renderfx = renderfx;
	    VectorCopy (part[BP_GV_TURRET].origin, part[BP_GV_TURRET].oldorigin);
	    CGame::addVehiclePart( &part[BP_GV_TURRET] );

	    //
	    // gun
	    //
	    part[BP_GV_GUNBARREL].hModel = veh->handle[BP_GV_GUNBARREL];
	    VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_GUNBARREL].lightingOrigin );
		AxisCopy( axisDefault, part[BP_GV_GUNBARREL].axis );
		RotateAroundPitch( part[BP_GV_GUNBARREL].axis, drawInfo->gunAngle );
		CGame::positionRotatedEntityOnTag( &part[BP_GV_GUNBARREL], &part[BP_GV_TURRET], veh->handle[BP_GV_TURRET], gv_tags[BP_GV_GUNBARREL] );
		part[BP_GV_GUNBARREL].shadowPlane = shadowPlane;
	    part[BP_GV_GUNBARREL].renderfx = renderfx;
	    VectorCopy (part[BP_GV_GUNBARREL].origin, part[BP_GV_GUNBARREL].oldorigin);
	    CGame::addVehiclePart( &part[BP_GV_GUNBARREL] );
	}

	//
	// wheels
	//
	// gearAnimStartTime is for timediff
	// gearAnim is for angles
	if( (veh->caps & HC_WHEELS) ) {
		int ii;
		float turnModifier;

		turnModifier = (drawInfo->basicInfo.speed / 10) / (availableVehicles[drawInfo->basicInfo.vehicleIndex].maxspeed * 0.2f);
		for( ii = 0; ii < availableVehicles[drawInfo->basicInfo.vehicleIndex].wheels; ++ii ) {


			part[BP_GV_WHEEL+ii].hModel = veh->handle[BP_GV_WHEEL+ii];
			VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_WHEEL+ii].lightingOrigin );
			AxisCopy( axisDefault, part[BP_GV_WHEEL+ii].axis );
			part[BP_GV_WHEEL+ii].shadowPlane = shadowPlane;
			part[BP_GV_WHEEL+ii].renderfx = renderfx;
			VectorCopy (part[BP_GV_WHEEL+ii].origin, part[BP_GV_WHEEL+ii].oldorigin);
			//RotateAroundPitch( part[BP_GV_WHEEL+ii].axis, drawInfo->wheelAngle );


			if(availableVehicles[drawInfo->basicInfo.vehicleIndex].wheels > 2 && ii < 2)
			{

				if(drawInfo->wheelDirection == 1)
				{
					RotateAroundYaw(part[BP_GV_WHEEL+ii].axis, 325 + 20 * turnModifier);
				}
				else if(drawInfo->wheelDirection == 2)
				{
					RotateAroundYaw(part[BP_GV_WHEEL+ii].axis, 35 - 20 * turnModifier);
				}
			}

			RotateAroundPitch( part[BP_GV_WHEEL+ii].axis, drawInfo->wheelAngle );

			
			CGame::positionRotatedEntityOnTag( &part[BP_GV_WHEEL+ii], &part[BP_GV_BODY], veh->handle[BP_GV_BODY], gv_tags[BP_GV_WHEEL+ii] );
			
			CGame::addVehiclePart( &part[BP_GV_WHEEL+ii] );
		}
	}

	// sound
	if( drawInfo->basicInfo.entityNum >= 0 ) {
		tanksound = drawInfo->basicInfo.speed * NUM_TANKSOUNDS / veh->maxspeed;
		if( tanksound >= NUM_TANKSOUNDS ) tanksound = NUM_TANKSOUNDS - 1;
		SoundSystem::addLoopingSound( drawInfo->basicInfo.entityNum, 
								drawInfo->basicInfo.origin, 
								vec3_origin, 
								cgs.media.engineTank[tanksound] );
	}

	// muzzleflash
	if( drawInfo->basicInfo.drawMuzzleFlash ) {
		CGame::vehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_GV_GUNBARREL], 
							   veh->handle[BP_GV_GUNBARREL], drawInfo->basicInfo.vehicleIndex );
	}
}
