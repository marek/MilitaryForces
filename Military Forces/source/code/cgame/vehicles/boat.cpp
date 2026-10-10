#include "../cg_local.h"
#include <algorithm>

/*
 * $Id: cg_boat.c,v 1.5 2005-11-20 11:21:38 thebjoern Exp $
*/




/*
==========================
CGame::registerBoat
==========================
*/
void CGame::registerBoat( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_BOAT_MAX_PARTS; i++ ) {
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
	}
}



/*
===============
CG_Boat
===============
*/
void ClientEntity::boat( clientInfo_t *ci ) 
{
	Vec3			velocity;
	DrawInfo_Boat_t	drawInfo;
	int				i;

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

    //
    // turrets
    //
	for( i = 0; i < 4; ++i ) {
		float yaw = this->lerpAngles2[ROLL];
		float pitch = this->lerpAngles2[PITCH];
		float max, min;
		if( yaw > 180 ) yaw -= 360;
		max = availableWeapons[availableVehicles[ci->vehicle].weapons[i+1]].maxturns[1];
		min = availableWeapons[availableVehicles[ci->vehicle].weapons[i+1]].minturns[1];
		if( max > min ) {
			if( yaw > max ) yaw = max;
			else if( yaw < min ) yaw = min;
		} else {
			if( yaw > 0 && yaw < min ) yaw = min;
			else if( yaw < 0 && yaw > max ) yaw = max;
		}
		if( pitch > 180 ) pitch -= 360;
		max = availableWeapons[availableVehicles[ci->vehicle].weapons[i+1]].maxturns[0];
		min = availableWeapons[availableVehicles[ci->vehicle].weapons[i+1]].minturns[0];
		if( pitch > max ) pitch = max;
		else if( pitch < min ) pitch = min;
		drawInfo.turretAngle[i] = yaw;
		drawInfo.gunAngle[i] = pitch;
	}

	// smoke 
	if( this->currentState.generic1 ) {
		this->generic_Smoke( this->lerpOrigin, 100 );
	}

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}

	CGame::drawBoat(&drawInfo);

	// reticles
	if( this == &cg.predictedPlayerEntity )
	{
		refEntity_t	reticle;
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
void CGame::boatObituary( entityState_t *ent, clientInfo_t *ci ) 
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
CGame::cacheBoat

On startup cache it
===============
*/

void CGame::cacheBoat(int index)
{
	char name[128];
	char basename[128];
	char body[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/sea/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );
// changed mg
	for( i = 0; i < BP_BOAT_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_BOAT_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			// temp
//			{
//				md3Tag_t tag;
//				if( VehicleData::findTag(name, "tag_turret", &tag) ) {
//					CGame::printf( "found tag_turret in %s: %.1f %.1f %.1f\n", name,
//						tag.origin[0], tag.origin[1], tag.origin[2] );
//				}
//			}
			// end temp

			break;
		case BP_BOAT_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_BOAT_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		case BP_BOAT_TURRET2:
			Com_sprintf( name, sizeof(name), "%s_tur2.md3", basename );
			break;
		case BP_BOAT_GUNBARREL2:
			Com_sprintf( name, sizeof(name), "%s_gun2.md3", basename );
			break;
		case BP_BOAT_TURRET3:
			Com_sprintf( name, sizeof(name), "%s_tur3.md3", basename );
			break;
		case BP_BOAT_GUNBARREL3:
			Com_sprintf( name, sizeof(name), "%s_gun3.md3", basename );
			break;
		case BP_BOAT_TURRET4:
			Com_sprintf( name, sizeof(name), "%s_tur4.md3", basename );
			break;
		case BP_BOAT_GUNBARREL4:
			Com_sprintf( name, sizeof(name), "%s_gun4.md3", basename );
			break;
		}
		if( i == BP_BOAT_BODY ) {
			availableVehicles[index].handle[i] = refExport.RegisterModel(name);
		} else if( i & 1 ) {
			static const char *turretTags[] = { "tag_turret", "tag_turret2", "tag_turret3", "tag_turret4" };
			Com_sprintf( body, sizeof(body), "%s.md3", basename );
			availableVehicles[index].handle[i] = CGame::registerVehiclePart( name, 
				availableVehicles[index].handle[BP_BOAT_BODY] && VehicleData::findTag( body, turretTags[i/2] ).has_value() );
		} else {
			availableVehicles[index].handle[i] = CGame::registerVehiclePart( name, availableVehicles[index].handle[i-1] != 0 );
		}
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_BOAT_BODY] ) {
	}
}

char *boat_tags[BP_BOAT_MAX_PARTS] =
{
	"<no tag>",		// vehicle body
	"tag_turret",	// turret
	"tag_weap",		// gun
	"tag_turret2",	// turret
	"tag_weap2",		// gun
	"tag_turret3",	// turret
	"tag_weap",		// gun
	"tag_turret4",	// turret
	"tag_weap"		// gun
};

void CGame::drawBoat(DrawInfo_Boat_t* drawInfo)
{
	refEntity_t	    part[BP_BOAT_MAX_PARTS];
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
    part[BP_BOAT_BODY].hModel = veh->handle[BP_BOAT_BODY];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_BOAT_BODY].origin );
    VectorCopy( drawInfo->basicInfo.origin, part[BP_BOAT_BODY].lightingOrigin );
	AxisCopy( drawInfo->basicInfo.axis, part[BP_GV_BODY].axis );
    part[BP_BOAT_BODY].shadowPlane = shadowPlane;
    part[BP_BOAT_BODY].renderfx = renderfx;
    VectorCopy (part[BP_BOAT_BODY].origin, part[BP_BOAT_BODY].oldorigin);
    CGame::addVehiclePart( &part[BP_BOAT_BODY] );

    //
    // turrets
    //
	for( i = 0; i < 2; ++i ) {
		int j = 2*i;
		// turret
		part[BP_BOAT_TURRET+j].hModel = veh->handle[BP_BOAT_TURRET+j];
		if( !part[BP_BOAT_TURRET+j].hModel ) break;
		VectorCopy( drawInfo->basicInfo.origin, part[BP_BOAT_TURRET+j].lightingOrigin );
		AxisCopy( axisDefault, part[BP_BOAT_TURRET+j].axis );
		RotateAroundYaw( part[BP_BOAT_TURRET+j].axis, drawInfo->turretAngle[i] );
		CGame::positionRotatedEntityOnTag( &part[BP_BOAT_TURRET+j], &part[BP_BOAT_BODY], veh->handle[BP_BOAT_BODY], boat_tags[BP_BOAT_TURRET+j] );
		part[BP_BOAT_TURRET+j].shadowPlane = shadowPlane;
		part[BP_BOAT_TURRET+j].renderfx = renderfx;
		VectorCopy (part[BP_BOAT_TURRET+j].origin, part[BP_BOAT_TURRET+j].oldorigin);
		CGame::addVehiclePart( &part[BP_BOAT_TURRET+j] );
		// gun
		part[BP_BOAT_GUNBARREL+j].hModel = veh->handle[BP_BOAT_GUNBARREL+j];
		VectorCopy( drawInfo->basicInfo.origin, part[BP_BOAT_GUNBARREL+j].lightingOrigin );
		AxisCopy( axisDefault, part[BP_BOAT_GUNBARREL+j].axis );
		RotateAroundPitch( part[BP_BOAT_GUNBARREL+j].axis, drawInfo->gunAngle[i] );
		CGame::positionRotatedEntityOnTag( &part[BP_BOAT_GUNBARREL+j], &part[BP_BOAT_TURRET+j], veh->handle[BP_BOAT_TURRET+j], boat_tags[BP_BOAT_GUNBARREL+j] );
		part[BP_BOAT_GUNBARREL+j].shadowPlane = shadowPlane;
		part[BP_BOAT_GUNBARREL+j].renderfx = renderfx;
		VectorCopy (part[BP_BOAT_GUNBARREL+j].origin, part[BP_BOAT_GUNBARREL+j].oldorigin);
		CGame::addVehiclePart( &part[BP_BOAT_GUNBARREL+j] );

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
		CGame::vehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_BOAT_GUNBARREL], 
							   veh->handle[BP_BOAT_GUNBARREL], drawInfo->basicInfo.vehicleIndex );
	}
}
