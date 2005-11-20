/*
 * $Id: cg_lqm.c,v 1.4 2005-11-20 11:21:38 thebjoern Exp $
*/


#include "cg_local.h"


/*
==========================
CG_RegisterLQM
==========================
*/
void CG_RegisterLQM( clientInfo_t *ci ) 
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
static void CG_LQMFlags( centity_t *cent ) {
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
void CG_LQM( centity_t *cent, clientInfo_t *ci ) 
{
	vec3_t			velocity;	
	vec3_t		    right, up, temp, start;
	DrawInfo_LQM_t drawInfo;
	int				ONOFF = cent->currentState.ONOFF;


	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = ci->vehicle;
	drawInfo.basicInfo.ONOFF = ONOFF;

	// Copy Weapon Index
	drawInfo.weaponIndex = cent->currentState.weaponIndex;

	// Copy animation state
	drawInfo.anim = cent->currentState.vehicleAnim;

	// Copy animation frames
	drawInfo.lastTorsoAngle = cent->bayAnim;
	drawInfo.lastLegsAngle = cent->gearAnim;
	drawInfo.torsoFrame = cent->bayAnimFrame;
	drawInfo.legsFrame = cent->gearAnimFrame;
	drawInfo.torsoTime = cent->bayAnimStartTime;
	drawInfo.legsTime = cent->gearAnimStartTime;

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

	// loadout
	drawInfo.basicInfo.usedLoadout = 0;//&cg_loadouts[cent->currentState.number];

	// muzzleflash
	if( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = cent->muzzleFlashWeapon;
	}

	// draw lqm
	CG_DrawLQM(&drawInfo);

	// return frames
	cent->bayAnim = drawInfo.lastTorsoAngle;
	cent->gearAnim = drawInfo.lastLegsAngle;
	cent->bayAnimFrame = drawInfo.torsoFrame;
	cent->bayAnimStartTime = drawInfo.torsoTime;
	cent->gearAnimFrame = drawInfo.legsFrame;
	cent->gearAnimStartTime = drawInfo.legsTime;
	

	// flags
	CG_LQMFlags( cent );


	// reticles
	if( cent == &cg.predictedPlayerEntity )
	{
		refEntity_t		reticle;
		vec3_t	forward, ang, end;
		trace_t	tr;
//		playerState_t * ps = &cg.snap->ps;
		float len;
		float mindist = cg_thirdPersonRange.integer + availableVehicles[ci->vehicle].cam_dist[ CAMERA_V_DEFAULT ] + availableVehicles[ci->vehicle].maxs[0] + 20;

		memset( &reticle, 0, sizeof(reticle) );	

		CG_ResetReticles();

		reticle.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshair;
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
		CG_Trace( &tr, start, 0, 0, end, cg.snap->ps.clientNum, MASK_SOLID ); 

		if( tr.entityNum < MAX_CLIENTS )
		{
			cg.crosshairClientNum = tr.entityNum;
			cg.crosshairClientTime = cg.time;
		}

		VectorCopy( tr.endpos, end );
		CG_Trace( &tr, cg.refdef.vieworg, 0, 0, end, cg.snap->ps.clientNum, MASK_SOLID ); 
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
		
		CG_AddReticleEntityToScene( &reticle, NULL );
	}
}

/*
=============
CG_GroundVehicleObituary

General ugly function, needs to be made prettier some day...
=============
*/
void CG_LQMObituary( entityState_t *ent, clientInfo_t *ci ) 
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
#pragma message("remove the problem checking here!")
	if( Info_ValueForKey( targetInfo, "n" ) != 0 ) {
		Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	}
	else {
		Com_Error( ERR_DROP, "MFQ3 Error (3): Invalid targetinfo\n" );
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
			Com_Error( ERR_DROP, "MFQ3 Error (1): Bad attackerInfo!\n" );
		}
	    strcat( attackerName, S_COLOR_WHITE );
	    // check for kill messages about the current clientNum
	    if ( target == cg.snap->ps.clientNum ) {
			if( attackerName != 0 ) {
				Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
			}
			else {
				Com_Error( ERR_DROP, "MFQ3 Error (2): No attackerName!\n" );
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
			CG_Printf( "%s %s %s%s\n", targetName, message, attackerName, message2);
			return;
		}
    }

    // we don't know what it was
    CG_Printf( "%s died.\n", targetName );
}

