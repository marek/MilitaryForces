/*
 * $Id: cg_helo.c,v 1.1 2005-08-22 16:02:25 thebjoern Exp $
*/


#include "cg_local.h"
/*
#define VAPORSPEED	(SPEED_GREEN_ARC - (SPEED_GREEN_ARC - SPEED_YELLOW_ARC)/2)

// make sure the tags are named properly!
char *plane_tags[BP_PLANE_MAX_PARTS] =
{
	"<no tag>",		// plane body
	"tag_controls",	// controls
	"tag_cockpit",	// cockpit
	"tag_gear",		// gear
	"tag_breaks",	// brakes
	"tag_bay",		// bay
	"tag_wingL",	// wing left
	"tag_wingR",	// wing right
	"tag_special",	// special
	"tag_prop1"	// prop
};

char *engine_tags[3] =
{
	"tag_prop2",
	"tag_prop3",
	"tag_prop4"
};

/*
==========================
CG_RegisterHelo
==========================
*/
void CG_RegisterHelo( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_HELO_MAX_PARTS; i++ ) 
	{
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
	}
}

/*
=============
CG_PlanePilot
=============
*/
void CG_HeloPilot( centity_t *cent, const refEntity_t *parent, qhandle_t parentModel, int idx ) {
/*
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
	}*/
}


/*
===============
CG_PlaneFlags
===============
*/
static void CG_HeloFlags( centity_t *cent ) {
	/*int		powerups;
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
	}*/
}


/*
===============
CG_Plane
===============
*/
void CG_Helo( centity_t *cent, clientInfo_t *ci ) 
{
	vec3_t			velocity;	
	vec3_t			smokePosition, forward;//, right, up, temp, start;
	DrawInfo_Helo_t drawInfo;
	int				ONOFF = cent->currentState.ONOFF;
	int				i;

	memset( &drawInfo, 0, sizeof(drawInfo) );
	drawInfo.basicInfo.vehicleIndex = ci->vehicle;
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

	// loadout
	drawInfo.basicInfo.loadout = &cg_loadouts[cent->currentState.number];

    // turrets
	for( i = 0; i < 4; ++i ) {
		float yaw = cent->currentState.angles2[ROLL];
		float pitch = cent->currentState.angles2[PITCH];
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
	if( cent->currentState.generic1 ) {
		AngleVectors( cent->lerpAngles, forward, NULL, NULL );
		VectorMA( cent->lerpOrigin, availableVehicles[ci->vehicle].mins[0], forward, smokePosition );
		CG_Generic_Smoke( cent, smokePosition, 10 );
	}

	// muzzleflash
	if( cg.time - cent->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = qtrue;
		drawInfo.basicInfo.flashWeaponIndex = cent->muzzleFlashWeapon;
	}

	
	// draw helo
	CG_DrawHelo(&drawInfo);

	// flags
	CG_HeloFlags( cent );

	// reticles
	if( cent == &cg.predictedPlayerEntity )
	{
		refEntity_t	reticle;
		vec3_t	forward, right, up, ang, start, end, temp;
		trace_t	tr;
		float len;
		playerState_t* ps = &cg.snap->ps;
		float mindist = cg_thirdPersonRange.integer + availableVehicles[ci->vehicle].cam_dist[ CAMERA_V_DEFAULT ] + availableVehicles[ci->vehicle].maxs[0] + 20;

		memset( &reticle, 0, sizeof(reticle) );

		CG_ResetReticles();

		// are we tracking a target?
		if( ps->stats[STAT_LOCKINFO] & LI_TRACKING )
		{
			refEntity_t reticlelock;
			qboolean building = qfalse;
			centity_t* target = &cg_entities[cent->currentState.tracktarget];
			
			if( target->currentState.eType == ET_EXPLOSIVE )
			{
				building = qtrue;
			}

			reticle.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairtrack;
			reticle.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairtrack];

			if( building )
			{
				VectorSubtract( cgs.inlineModelMidpoints[target->currentState.modelindex], cent->lerpOrigin, forward );
			}
			else
			{
				VectorSubtract( target->lerpOrigin, cent->lerpOrigin, forward );
			}

			len = VectorNormalize( forward );
			vectoangles( forward, ang );
			AnglesToAxis( ang, reticle.axis );
			VectorMA( cent->lerpOrigin, 2000, forward, end );
			CG_Trace( &tr, cent->lerpOrigin, 0, 0, end, cg.snap->ps.clientNum, MASK_SOLID ); 
			
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

			CG_AddReticleEntityToScene( &reticle, target );

			// are we locked onto this target?
			if( ps->stats[STAT_LOCKINFO] & LI_LOCKING )
			{
				// copy the reticle entity as our reticle-lock entity
				memcpy( &reticlelock, &reticle, sizeof(reticle) );
				
				reticlelock.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairlock;
				reticlelock.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairlock];
				reticlelock.frame = 1;
			}
			else
			{
				memset( &reticlelock, 0, sizeof(reticlelock) );	
				
				reticlelock.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairtrack;
				reticlelock.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].crosshairtrack];
				
				AngleVectors( cent->currentState.angles, forward, right, up );
				RotatePointAroundVector( temp, up, forward, cent->currentState.angles2[ROLL] );
				CrossProduct( up, temp, right );
				RotatePointAroundVector( forward, right, temp, cent->currentState.angles2[PITCH] );
				VectorMA( cent->lerpOrigin, availableVehicles[ci->vehicle].gunoffset[0], forward, start );
				VectorMA( start, availableVehicles[ci->vehicle].gunoffset[1], right, start );
				VectorMA( start, availableVehicles[ci->vehicle].gunoffset[2], up, start );
				VectorMA( start, 2000, forward, end );
				vectoangles( forward, ang );
//				ang[2] = 0;
				AnglesToAxis( ang, reticlelock.axis );
				VectorScale( reticlelock.axis[0], 10.0f, reticlelock.axis[0] );
				VectorScale( reticlelock.axis[1], 10.0f, reticlelock.axis[1] );
				VectorScale( reticlelock.axis[2], 10.0f, reticlelock.axis[2] );
				reticlelock.nonNormalizedAxes = qtrue;
				CG_Trace( &tr, start, 0, 0, end, cg.snap->ps.clientNum, MASK_SOLID ); 
				VectorCopy( tr.endpos, end );
				CG_Trace( &tr, cg.refdef.vieworg, 0, 0, end, cg.snap->ps.clientNum, MASK_SOLID ); 
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

			CG_AddReticleEntityToScene( &reticlelock, NULL );
		}
		else if(!(ps->stats[STAT_LOCKINFO] & LI_TRACKING) || (availableWeapons[availableVehicles[ci->vehicle].weapons[cent->currentState.weaponNum]].type == WT_MACHINEGUN))
		{
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
}


/*
=============
CG_HeloObituary

General ugly function, needs to be made prettier some day...
=============
*/
void CG_HeloObituary( entityState_t *ent, clientInfo_t *ci ) 
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
		default:
			message = "was killed by";
			break;
		}

		if (message)
		{
			CG_Printf( "%s %s %s%s\n", targetName, message, attackerName, message2);
			return;
		}
    }

    // we don't know what it was
    CG_Printf( "%s died.\n", targetName );
}


