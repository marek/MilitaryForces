/*
 * $Id: cg_groundvehicle.c,v 1.16 2002-02-25 12:13:36 sparky909_uk Exp $
*/


#include "cg_local.h"

// make sure the tags are named properly!
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

/*
==========================
CG_RegisterGroundVehicle
==========================
*/
void CG_RegisterGroundVehicle( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
/*
		if( !ci->parts[i] )
		{
			CG_Printf( "Informational: RegisterGroundVehicle failed register part %d of vehicle %d (id %x)\n", i, 
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
static void CG_GroundVehicleFlags( centity_t *cent ) {
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
void CG_GroundVehicle( centity_t *cent, clientInfo_t *ci ) 
{
	refEntity_t	    part[BP_GV_MAX_PARTS];
	refEntity_t		reticle;
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				i, tanksound;
	float			speed;
	vec3_t			velocity;

	// get velocity
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	speed = VectorLength( velocity );

	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
	    memset( &part[i], 0, sizeof(part[0]) );	
	}
	memset( &reticle, 0, sizeof(reticle) );

    // get the rotation information
    VectorCopy( cent->currentState.angles, cent->lerpAngles );
    AnglesToAxis( cent->lerpAngles, part[BP_GV_BODY].axis );

    // add the talk baloon or disconnect icon
    CG_PlayerSprites( cent );
    
    // add the shadow
	switch( cg_shadows.integer )
	{
	case 1:
		CG_GenericShadow( cent, &shadowPlane );
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
    part[BP_GV_BODY].hModel = ci->parts[BP_GV_BODY];
    VectorCopy( cent->lerpOrigin, part[BP_GV_BODY].origin );
    VectorCopy( cent->lerpOrigin, part[BP_GV_BODY].lightingOrigin );
    part[BP_GV_BODY].shadowPlane = shadowPlane;
    part[BP_GV_BODY].renderfx = renderfx;
    VectorCopy (part[BP_GV_BODY].origin, part[BP_GV_BODY].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_BODY] );

    //
    // turret
    //
    part[BP_GV_TURRET].hModel = ci->parts[BP_GV_TURRET];
    VectorCopy( cent->lerpOrigin, part[BP_GV_TURRET].lightingOrigin );
	AxisCopy( axisDefault, part[BP_GV_TURRET].axis );
	RotateAroundYaw( part[BP_GV_TURRET].axis, cent->currentState.angles2[ROLL] );
	CG_PositionRotatedEntityOnTag( &part[BP_GV_TURRET], &part[BP_GV_BODY], ci->parts[BP_GV_BODY], gv_tags[BP_GV_TURRET] );
	part[BP_GV_TURRET].shadowPlane = shadowPlane;
    part[BP_GV_TURRET].renderfx = renderfx;
    VectorCopy (part[BP_GV_TURRET].origin, part[BP_GV_TURRET].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_TURRET] );

    //
    // gun
    //
    part[BP_GV_GUNBARREL].hModel = ci->parts[BP_GV_GUNBARREL];
    VectorCopy( cent->lerpOrigin, part[BP_GV_GUNBARREL].lightingOrigin );
	AxisCopy( axisDefault, part[BP_GV_GUNBARREL].axis );
	RotateAroundPitch( part[BP_GV_GUNBARREL].axis, cent->currentState.angles2[PITCH] );
	CG_PositionRotatedEntityOnTag( &part[BP_GV_GUNBARREL], &part[BP_GV_TURRET], ci->parts[BP_GV_TURRET], gv_tags[BP_GV_GUNBARREL] );
	part[BP_GV_GUNBARREL].shadowPlane = shadowPlane;
    part[BP_GV_GUNBARREL].renderfx = renderfx;
    VectorCopy (part[BP_GV_GUNBARREL].origin, part[BP_GV_GUNBARREL].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_GUNBARREL] );

	//
	// wheels
	//
	// gearAnimStartTime is for timediff
	// gearAnim is for angles
	if( (availableVehicles[ci->vehicle].caps & HC_WHEELS) ) {
		int ii;
		int timediff = cg.time - cent->gearAnimStartTime;
		float dist = speed * timediff / 1000;
		vec3_t v1, v2;
		float dot;
		AngleVectors( cent->currentState.angles, v1, 0, 0 );
		VectorCopy( cent->currentState.pos.trDelta, v2 );
		VectorNormalize( v2 );
		dot = DotProduct( v1, v2 );
		dist = ( (dist / availableVehicles[ci->vehicle].wheelCF)*360);
		cent->gearAnim += (dot > 0 ? dist : -dist);
		cent->gearAnim = AngleMod( cent->gearAnim );
		for( ii = 0; ii < availableVehicles[ci->vehicle].wheels; ++ii ) {
			part[BP_GV_WHEEL+ii].hModel = ci->parts[BP_GV_WHEEL+ii];
			VectorCopy( cent->lerpOrigin, part[BP_GV_WHEEL+ii].lightingOrigin );
			AxisCopy( axisDefault, part[BP_GV_WHEEL+ii].axis );
			part[BP_GV_WHEEL+ii].shadowPlane = shadowPlane;
			part[BP_GV_WHEEL+ii].renderfx = renderfx;
			VectorCopy (part[BP_GV_WHEEL+ii].origin, part[BP_GV_WHEEL+ii].oldorigin);
			RotateAroundPitch( part[BP_GV_WHEEL+ii].axis, cent->gearAnim );
			CG_PositionRotatedEntityOnTag( &part[BP_GV_WHEEL+ii], &part[BP_GV_BODY], ci->parts[BP_GV_BODY], gv_tags[BP_GV_WHEEL+ii] );
			trap_R_AddRefEntityToScene( &part[BP_GV_WHEEL+ii] );
		}
		cent->gearAnimStartTime = cg.time;
	}

/*
		AngleVectors( self->client->ps.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, ((float)self->client->ps.turretAngle)/10 );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, ((float)self->client->ps.gunAngle)/10 );
*/

	// reticles
	if( cent == &cg.predictedPlayerEntity )
	{
		vec3_t	forward, right, up, ang, start, end, temp;
		trace_t	tr;
		float len;
		playerState_t* ps = &cg.snap->ps;
		float mindist = cg_thirdPersonRange.integer + availableVehicles[ci->vehicle].cam_dist[ CAMERA_V_DEFAULT ] + availableVehicles[ci->vehicle].maxs[0] + 20;

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
			CG_Trace( &tr, cent->lerpOrigin, 0, 0, end, cg.snap->ps.clientNum, MASK_ALL ); 
			
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
			reticle.shadowPlane = shadowPlane;
			reticle.renderfx = renderfx;

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
				CG_Trace( &tr, start, 0, 0, end, cg.snap->ps.clientNum, MASK_ALL ); 
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
				reticlelock.shadowPlane = shadowPlane;
				reticlelock.renderfx = renderfx;
			}

			CG_AddReticleEntityToScene( &reticlelock, NULL );
		}
		else
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
			CG_Trace( &tr, start, 0, 0, end, cg.snap->ps.clientNum, MASK_ALL ); 

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
			reticle.shadowPlane = shadowPlane;
			reticle.renderfx = renderfx;
			
			CG_AddReticleEntityToScene( &reticle, NULL );
		}
	}

	// sound
	BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );
	tanksound = speed * NUM_TANKSOUNDS / availableVehicles[ci->vehicle].maxspeed;
	if( tanksound >= NUM_TANKSOUNDS ) tanksound = NUM_TANKSOUNDS - 1;
	trap_S_AddLoopingSound( cent->currentState.number, 
							cent->lerpOrigin, 
							velocity, 
							cgs.media.engineTank[tanksound] );

	// smoke
	CG_Generic_Smoke( cent, cent->lerpOrigin, 100 );
	
#pragma message("maybe use the hastePuffShader for a dust trail when tanks drive ?")
	// muzzleflash
	CG_VehicleMuzzleFlash( cent, &part[BP_GV_GUNBARREL], ci->parts[BP_GV_GUNBARREL], ci->vehicle );

	// CTF
	CG_GroundVehicleFlags( cent );
}

/*
=============
CG_GroundVehicleObituary

General ugly function, needs to be made prettier some day...
=============
*/
void CG_GroundVehicleObituary( entityState_t *ent, clientInfo_t *ci ) 
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

