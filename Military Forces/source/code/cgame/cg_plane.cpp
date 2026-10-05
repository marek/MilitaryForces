/*
 * $Id: cg_plane.c,v 1.6 2007-06-17 17:11:12 minkis Exp $
*/


#include "cg_local.h"

/*
==========================
CGame::registerPlane
==========================
*/
void CGame::registerPlane( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_PLANE_MAX_PARTS; i++ ) 
	{
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
/*
		if( !ci->parts[i] )
		{
			CGame::printf( "Informational: RegisterPlane failed register part %d of vehicle %d (id %x)\n", i, 
				ci->vehicle, availableVehicles[ci->vehicle].id );
		}
*/
	}
}

/*
===============
CG_PlaneFlags
===============
*/
void ClientEntity::planeFlags() {
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
CG_Plane
===============
*/
void ClientEntity::plane( clientInfo_t *ci ) 
{
	Vec3			velocity;	
	Vec3			smokePosition, forward;
	DrawInfo_Plane_t drawInfo;
	int				ONOFF = this->currentState.ONOFF;
	int				i;

	drawInfo = {};
	drawInfo.basicInfo.vehicleIndex = ci->vehicle;
	drawInfo.basicInfo.team = cgs.gametype >= GT_TEAM ? (ci->paintTeam ? ci->paintTeam : ci->team) : 0;
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
	if( availableVehicles[ci->vehicle].caps & HC_GEAR ) {
		int timediff = cg.time - this->gearAnimStartTime;
		int geardown = availableVehicles[ci->vehicle].maxGearFrame;
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
		} else if( this->gearAnimStartTime == 0 ) {
			// nothing animating: draw what the state says, in case the gear event never arrived
			this->gearAnimFrame = (ONOFF & OO_GEAR) ? geardown : GEAR_UP;
		}
		drawInfo.gearFrame = this->gearAnimFrame;
	}
	
	// bay
	if( availableVehicles[ci->vehicle].caps & HC_WEAPONBAY ) {
		int timediff = cg.time - this->bayAnimStartTime;
		int baydown = availableVehicles[ci->vehicle].maxBayFrame;
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
	if( availableVehicles[ci->vehicle].caps & HC_SPEEDBRAKE ) {
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
	if( this->currentState.frame <= 10 ) 
	{
		drawInfo.bodyFrame = 0;
	} 
	else 
	{
		drawInfo.bodyFrame = ( this->currentState.frame > 12 ? 2 : 1 );
	}

	// swingwings
	if( availableVehicles[ci->vehicle].caps & HC_SWINGWING ) 
	{
		drawInfo.swingAngle = this->lerpAngles2[PITCH];
	}

	// loadout
	this->setDrawLoadout( &drawInfo.basicInfo );

	// smoke
	if( this->currentState.generic1 ) 
	{
		AngleVectors( this->lerpAngles, forward, nullptr, nullptr );
		smokePosition = this->lerpOrigin + forward * ( availableVehicles[ci->vehicle].mins[0] );
		this->generic_Smoke( smokePosition, 10 );
	}

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) 
	{
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}
	
	// draw plane
	CGame::drawPlane(&drawInfo);

	// flags
	this->planeFlags();

	// reticles
	if( this == &cg.predictedPlayerEntity )
	{
		refEntity_t		reticle;
		Vec3	forward, ang, end;
		trace_t	tr;
		playerState_t * ps = &cg.snap->ps;
		float len;
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
			reticle.hModel = cgs.media.reticle[ availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairtrack ];
			
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
			if ( tr.entityNum > ENTITYNUM_WORLD && tr.entityNum < MAX_CLIENTS )
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
				memcpy( &reticlelock, &reticle, sizeof(reticle) );
				reticlelock.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairlock;
				reticlelock.hModel = cgs.media.reticle[ availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairlock ];
				reticlelock.frame = 1;
			}
			else
			{
				reticlelock = {};	
				reticlelock.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairtrack;
				reticlelock.hModel = cgs.media.reticle[ availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshairtrack ];
				ang = Vec3( this->currentState.angles[0], this->currentState.angles[1], 0 );
				AnglesToAxis( ang, reticlelock.axis );
				VectorScale( reticlelock.axis[0], 0.5f, reticlelock.axis[0] );
				VectorScale( reticlelock.axis[1], 0.5f, reticlelock.axis[1] );
				VectorScale( reticlelock.axis[2], 0.5f, reticlelock.axis[2] );
				reticlelock.nonNormalizedAxes = true;
				AngleVectors(this->currentState.angles, forward, nullptr, nullptr);
				end = this->lerpOrigin + forward * ( 2000 );
				CGame::trace( &tr, this->lerpOrigin, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
				VectorCopy( tr.endpos, end );
				CGame::trace( &tr, cg.refdef.vieworg, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
				VectorSubtract( tr.endpos, cg.refdef.vieworg, forward );
				len = VectorNormalize(forward);
				if( len > mindist ) {
					VectorMA( cg.refdef.vieworg, mindist, forward, reticlelock.origin );
				} else {
					VectorMA( cg.refdef.vieworg, len - 5, forward, reticlelock.origin );
				}
				VectorCopy( reticlelock.origin, reticlelock.lightingOrigin );
				reticlelock.renderfx = RF_LIGHTING_ORIGIN|RF_SHADOW_PLANE;
				reticlelock.frame = 1;
			}
			
			CGame::addReticleEntityToScene( &reticlelock, nullptr );
		}
		else
		{
			reticle.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshair;
			reticle.hModel = cgs.media.reticle[ availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshair ];
			ang = Vec3( this->currentState.angles[0], this->currentState.angles[1], 0 );
			AnglesToAxis( ang, reticle.axis );
			VectorScale( reticle.axis[0], 2.0f, reticle.axis[0] );
			VectorScale( reticle.axis[1], 2.0f, reticle.axis[1] );
			VectorScale( reticle.axis[2], 2.0f, reticle.axis[2] );
			reticle.nonNormalizedAxes = true;
			AngleVectors(this->currentState.angles, forward, nullptr, nullptr);
			end = this->lerpOrigin + forward * ( 2000 );
			CGame::trace( &tr, this->lerpOrigin, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
			if ( tr.entityNum > ENTITYNUM_WORLD && tr.entityNum < MAX_CLIENTS ) {
				cg.crosshairClientNum = tr.entityNum;
				cg.crosshairClientTime = cg.time;
			}
			VectorCopy( tr.endpos, end );
			CGame::trace( &tr, cg.refdef.vieworg, nullptr, nullptr, end, cg.snap->ps.clientNum, MASK_SOLID ); 
			VectorSubtract( tr.endpos, cg.refdef.vieworg, forward );
			len = VectorNormalize(forward);
			if( len > mindist ) {
				VectorMA( cg.refdef.vieworg, mindist, forward, reticle.origin );
			} else {
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
CGame::planeObituary

General ugly function, needs to be made prettier some day...
=============
*/
void CGame::planeObituary( entityState_t *ent, clientInfo_t *ci ) 
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
#pragma message("when we are sure this doesn't happen, then remove the two checks!" )
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
		switch (mod)
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

		if (message)
		{
			CGame::printf( "%s %s %s%s\n", targetName, message, attackerName, message2);
			return;
		}
    }

    // we don't know what it was
    CGame::printf( "%s died.\n", targetName );
}

