/*
 * $Id: cg_lqm.c,v 1.5 2005-11-21 17:28:20 thebjoern Exp $
*/


#include "cg_local.h"


/*
==========================
CGame::registerLQM
==========================
*/
void CGame::registerLQM( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_LQM_MAX_PARTS; i++ ) {
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
	}
}


/*
===============
CG_GroundVehicleFlags
===============
*/
void ClientEntity::lQMFlags() {
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
void ClientEntity::lQM( clientInfo_t *ci ) 
{
	vec3_t			velocity;	
	vec3_t		    right, up, temp, start;
	DrawInfo_LQM_t drawInfo;
	int				ONOFF = this->currentState.ONOFF;


	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = ci->vehicle;
	drawInfo.basicInfo.team = cgs.gametype >= GT_TEAM ? (ci->paintTeam ? ci->paintTeam : ci->team) : 0;
	drawInfo.basicInfo.ONOFF = ONOFF;

	// Copy Weapon Index
	drawInfo.weaponIndex = this->currentState.weaponIndex;

	// Copy animation state
	drawInfo.anim = this->currentState.vehicleAnim;

	// Copy animation frames
	drawInfo.lastTorsoAngle = this->bayAnim;
	drawInfo.lastLegsAngle = this->gearAnim;
	drawInfo.torsoFrame = this->bayAnimFrame;
	drawInfo.legsFrame = this->gearAnimFrame;
	drawInfo.torsoTime = this->bayAnimStartTime;
	drawInfo.legsTime = this->gearAnimStartTime;

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

	// loadout
	drawInfo.basicInfo.usedLoadout = nullptr;//&cg_loadouts[this->currentState.number];

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}

	// draw lqm
	CGame::drawLQM(&drawInfo);

	// return frames
	this->bayAnim = drawInfo.lastTorsoAngle;
	this->gearAnim = drawInfo.lastLegsAngle;
	this->bayAnimFrame = drawInfo.torsoFrame;
	this->bayAnimStartTime = drawInfo.torsoTime;
	this->gearAnimFrame = drawInfo.legsFrame;
	this->gearAnimStartTime = drawInfo.legsTime;
	

	// flags
	this->lQMFlags();


	// reticles
	if( this == &cg.predictedPlayerEntity )
	{
		refEntity_t		reticle;
		vec3_t	forward, ang, end;
		trace_t	tr;
//		playerState_t * ps = &cg.snap->ps;
		float len;
		float mindist = cg_thirdPersonRange.integer + availableVehicles[ci->vehicle].cam_dist[ CAMERA_V_DEFAULT ] + availableVehicles[ci->vehicle].maxs[0] + 20;

		memset( &reticle, 0, sizeof(reticle) );	

		CGame::resetReticles();

		reticle.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshair;
		reticle.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshair];

		AngleVectors( this->currentState.angles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, this->lerpAngles2[ROLL] );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( forward, right, temp, this->lerpAngles2[PITCH] );
		VectorMA( this->lerpOrigin, availableVehicles[ci->vehicle].gunoffset[0], forward, start );
		VectorMA( start, availableVehicles[ci->vehicle].gunoffset[1], right, start );
		VectorMA( start, availableVehicles[ci->vehicle].gunoffset[2], up, start );
		VectorMA( start, 2000, forward, end );
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

/*
=============
CGame::groundVehicleObituary

General ugly function, needs to be made prettier some day...
=============
*/
void CGame::lQMObituary( entityState_t *ent, clientInfo_t *ci ) 
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

