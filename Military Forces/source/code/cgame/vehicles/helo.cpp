#include "../cg_local.h"
#include <algorithm>

/*
 * $Id: cg_helo.c,v 1.5 2005-11-21 17:28:20 thebjoern Exp $
*/


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
CGame::registerHelo
==========================
*/
void CGame::registerHelo( clientInfo_t *ci ) 
{
	int i;

	for( i = 0; i < BP_HELO_MAX_PARTS; i++ ) 
	{
		ci->parts[i] = availableVehicles[ci->vehicle].handle[i];
	}
}

/*
===============
CG_PlaneFlags
===============
*/
void ClientEntity::heloFlags() {
	/*int		powerups;
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
	}*/
}


/*
===============
CG_Plane
===============
*/
void ClientEntity::helo( clientInfo_t *ci ) 
{
	Vec3			velocity;	
	Vec3			smokePosition, forward;//, right, up, temp, start;
	DrawInfo_Helo_t drawInfo;
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

	// loadout
	this->setDrawLoadout( &drawInfo.basicInfo );

	// gear
	if( availableVehicles[ci->vehicle].caps & HC_GEAR ) {
		int timediff = cg.time - this->gearAnimStartTime;
		int geardown = availableVehicles[ci->vehicle].maxGearFrame;
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
			this->gearAnimFrame = (this->currentState.ONOFF & OO_GEAR) ? geardown : GEAR_UP;
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

    // turrets
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
		// two door guns share the first turret weapon: the right gun is drawn first, the left second
		if( (availableVehicles[ci->vehicle].flags & VF_DOORGUNS) && i < 2 ) {
			yaw = this->lerpAngles2[ROLL];
			if( yaw > 180 ) yaw -= 360;
			drawInfo.doorGunAim = yaw;
			yaw = doorGunYaw( i == 1, yaw );
			pitch = this->lerpAngles2[PITCH];
			if( pitch > 180 ) pitch -= 360;
			pitch = std::clamp( pitch, availableWeapons[availableVehicles[ci->vehicle].weapons[1]].minturns[0], availableWeapons[availableVehicles[ci->vehicle].weapons[1]].maxturns[0] );
		}
		drawInfo.turretAngle[i] = yaw;
		drawInfo.gunAngle[i] = pitch;
	}


	// smoke
	if( this->currentState.generic1 ) {
		AngleVectors( this->lerpAngles, forward, nullptr, nullptr );
		smokePosition = this->lerpOrigin + forward * ( availableVehicles[ci->vehicle].mins[0] );
		this->generic_Smoke( smokePosition, 10 );
	}

	// muzzleflash
	if( cg.time - this->muzzleFlashTime <= MUZZLE_FLASH_TIME ) {
		drawInfo.basicInfo.drawMuzzleFlash = true;
		drawInfo.basicInfo.flashWeaponIndex = this->muzzleFlashWeapon;
	}

	
	// draw helo
	CGame::drawHelo(&drawInfo);

	// flags
	this->heloFlags();

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
		// Once a missile has its lock its marks sit on the target and no longer follow the view, so the gun gets
		// its own reticle to be aimed with. Before the lock there is one reticle only, the selected weapon's
		const bool tracking = ( ps->stats[STAT_LOCKINFO] & LI_TRACKING ) != 0;
		const bool locked = ( ps->stats[STAT_LOCKINFO] & LI_LOCKING ) != 0;
		const bool gunSelected = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].type == WT_MACHINEGUN;
		if( !tracking || gunSelected || ( locked && availableVehicles[ci->vehicle].weapons[WP_MACHINEGUN] ) )
		{
			reticle = {};
			reticle.customShader = tracking && !gunSelected ? static_cast<int>( CH_GUNMODE ) : CGame::reticleOf( ci->vehicle, this->currentState.weaponNum );
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
CGame::heloObituary

General ugly function, needs to be made prettier some day...
=============
*/
void CGame::heloObituary( entityState_t *ent, clientInfo_t *ci ) 
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
	Q_strncpyz( targetName, Info::valueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
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
		Q_strncpyz( attackerName, Info::valueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
	    strcat( attackerName, S_COLOR_WHITE );
	    // check for kill messages about the current clientNum
	    if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
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

void CGame::cacheHelo(int index)
{
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/helos/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );

	for( i = 0; i < BP_HELO_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_HELO_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			break;
		case BP_HELO_MAINROTOR:
			Com_sprintf( name, sizeof(name), "%s_mrotor_blur.md3", basename );
			break;
		case BP_HELO_TAILROTOR:
			Com_sprintf( name, sizeof(name), "%s_trotor_blur.md3", basename );
			break;
		case BP_HELO_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_HELO_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		// door guns are one gun model on two mounts
		case BP_HELO_TURRET2:
			Com_sprintf( name, sizeof(name), (availableVehicles[index].flags & VF_DOORGUNS) ? "%s_tur.md3" : "%s_tur2.md3", basename );
			break;
		case BP_HELO_GUNBARREL2:
			Com_sprintf( name, sizeof(name), (availableVehicles[index].flags & VF_DOORGUNS) ? "%s_gun.md3" : "%s_gun2.md3", basename );
			break;
		case BP_HELO_GEAR:
			Com_sprintf( name, sizeof(name), "%s_gear.md3", basename );
			break;
		case BP_HELO_BAY:
			Com_sprintf( name, sizeof(name), "%s_bay.md3", basename );
			break;
		case BP_HELO_PARTS:
			Com_sprintf( name, sizeof(name), "%s_parts.md3", basename );
			break;
		}
		availableVehicles[index].handle[i] = CGame::registerVehiclePart( name, false );
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_HELO_BODY] ) {
		Common::printf( S_COLOR_YELLOW "WARNING: MFQ3: missing body %s.md3, model not loaded\n", basename );
	}
}

char *helo_tags[BP_HELO_MAX_PARTS] =
{
	"<no tag>",		// plane body
	"tag_mrotor",	// prop
	"tag_trotor",	// prop
	"tag_turret",	// turret
	"tag_weap",		// gun
	"tag_turret2",	// turret
	"tag_weap2",	// gun
	"tag_gear",		// gear
	"tag_bay",		// bay
	"tag_parts",	// variant parts
};

void CGame::drawHelo(DrawInfo_Helo_t* drawInfo)
{
refEntity_t	    part[BP_HELO_MAX_PARTS];
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				i;

	completeVehicleData_t* veh = &availableVehicles[drawInfo->basicInfo.vehicleIndex];
	cg_drawTeam = drawInfo->basicInfo.team;

	for( i = 0; i < BP_HELO_MAX_PARTS; i++ ) {
	    memset( &part[i], 0, sizeof(part[0]) );	
	}

	// use the same origin for all
    renderfx |= RF_LIGHTING_ORIGIN;

    //
    // add the hull
    //
    part[BP_HELO_BODY].hModel = veh->handle[BP_HELO_BODY];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_HELO_BODY].origin );
    VectorCopy( drawInfo->basicInfo.origin, part[BP_HELO_BODY].lightingOrigin );
	AxisCopy( drawInfo->basicInfo.axis, part[BP_HELO_BODY].axis );
    part[BP_HELO_BODY].shadowPlane = shadowPlane;
    part[BP_HELO_BODY].renderfx = renderfx;
    VectorCopy (part[BP_HELO_BODY].origin, part[BP_HELO_BODY].oldorigin);
    CGame::addVehiclePart( &part[BP_HELO_BODY] );

	// a helicopter with pilot seats in its model gets its crew drawn, like the planes
	orientation_t seat;
	if( refExport.LerpTag( &seat, veh->handle[BP_HELO_BODY], 0, 0, 1.0f, "tag_pilot1" ) )
		CGame::planePilot( &part[BP_HELO_BODY], veh->handle[BP_HELO_BODY], drawInfo->basicInfo.vehicleIndex );

	// rotors
	for( i = BP_HELO_MAINROTOR; i <= BP_HELO_TAILROTOR; i++ ) {
		part[i].hModel = veh->handle[i];
		if( !part[i].hModel ) {
			continue;
		}
		VectorCopy( drawInfo->basicInfo.origin, part[i].lightingOrigin );
		AxisCopy( axisDefault, part[i].axis );
		if( i == BP_HELO_MAINROTOR ) 
			RotateAroundYaw( part[i].axis, cg.time*1.25+drawInfo->basicInfo.entityNum*100 );	// make rotors more random by entitynum so it doesnt look stupid in a screenshot when everyone has the same rotor position
		if( i == BP_HELO_TAILROTOR )
			RotateAroundPitch( part[i].axis, cg.time*1.25+drawInfo->basicInfo.entityNum*100);

		CGame::positionRotatedEntityOnTag( &part[i], &part[BP_HELO_BODY], veh->handle[BP_HELO_BODY], helo_tags[i] );
		part[i].shadowPlane = shadowPlane;
		part[i].renderfx = renderfx;
		CGame::addVehiclePart( &part[i] );
	}


	// gear, bay, variant parts
	for( i = BP_HELO_GEAR; i <= BP_HELO_PARTS; i++ ) {
		part[i].hModel = veh->handle[i];
		if( !part[i].hModel ) {
			continue;
		}
		if( i == BP_HELO_GEAR ) part[i].frame = drawInfo->gearFrame;
		else if( i == BP_HELO_BAY ) part[i].frame = drawInfo->bayFrame;
		VectorCopy( drawInfo->basicInfo.origin, part[i].lightingOrigin );
		AxisCopy( axisDefault, part[i].axis );
		CGame::positionEntityOnTag( &part[i], &part[BP_HELO_BODY], veh->handle[BP_HELO_BODY], helo_tags[i] );
		part[i].shadowPlane = shadowPlane;
		part[i].renderfx = renderfx;
		CGame::addVehiclePart( &part[i] );
	}

	// vwep
	if( (veh->renderFlags & MFR_VWEP) && drawInfo->basicInfo.usedLoadout ) 
	{
		refEntity_t			vwep;
		int					parent = drawInfo->basicInfo.usedLoadout->mountsOnBay ? BP_HELO_BAY : BP_HELO_BODY;

		for( i = 0; i < drawInfo->basicInfo.usedLoadout->usedMounts; ++i )
		{
			if( drawInfo->basicInfo.usedLoadout->mounts[i].weapon && 
				drawInfo->basicInfo.numWeaponsOnMount[i] ) 
			{
				vwep = {};		
				vwep.hModel = availableWeapons[drawInfo->basicInfo.usedLoadout->mounts[i].weapon].vwepHandle;
				VectorCopy( drawInfo->basicInfo.origin, vwep.lightingOrigin );
				AxisCopy( axisDefault, vwep.axis );
				CGame::positionEntityOnTag( &vwep, &part[parent], veh->handle[parent], 
						drawInfo->basicInfo.usedLoadout->mounts[i].tag.name );
				vwep.shadowPlane = shadowPlane;
				vwep.renderfx = renderfx;
				CGame::addVehiclePart( &vwep );
			}
		}
	}

//
    // turrets
    //
	for( i = 0; i < 2; ++i ) {
		int j = 2*i;
		// turret
		part[BP_HELO_TURRET+j].hModel = veh->handle[BP_HELO_TURRET+j];
		if( !part[BP_HELO_TURRET+j].hModel ) break;
		VectorCopy( drawInfo->basicInfo.origin, part[BP_HELO_TURRET+j].lightingOrigin );
		AxisCopy( axisDefault, part[BP_HELO_TURRET+j].axis );
		RotateAroundYaw( part[BP_HELO_TURRET+j].axis, drawInfo->turretAngle[i] );
		CGame::positionRotatedEntityOnTag( &part[BP_HELO_TURRET+j], &part[BP_HELO_BODY], veh->handle[BP_HELO_BODY], helo_tags[BP_HELO_TURRET+j] );
		part[BP_HELO_TURRET+j].shadowPlane = shadowPlane;
		part[BP_HELO_TURRET+j].renderfx = renderfx;
		VectorCopy (part[BP_HELO_TURRET+j].origin, part[BP_HELO_TURRET+j].oldorigin);
		CGame::addVehiclePart( &part[BP_HELO_TURRET+j] );
		// gun
		part[BP_HELO_GUNBARREL+j].hModel = veh->handle[BP_HELO_GUNBARREL+j];
		VectorCopy( drawInfo->basicInfo.origin, part[BP_HELO_GUNBARREL+j].lightingOrigin );
		AxisCopy( axisDefault, part[BP_HELO_GUNBARREL+j].axis );
		RotateAroundPitch( part[BP_HELO_GUNBARREL+j].axis, drawInfo->gunAngle[i] );
		// door guns are the same gun model on both mounts, so the second barrel sits on the same tag as the first
		CGame::positionRotatedEntityOnTag( &part[BP_HELO_GUNBARREL+j], &part[BP_HELO_TURRET+j], veh->handle[BP_HELO_TURRET+j],
			helo_tags[BP_HELO_GUNBARREL + ( (veh->flags & VF_DOORGUNS) ? 0 : j )] );
		part[BP_HELO_GUNBARREL+j].shadowPlane = shadowPlane;
		part[BP_HELO_GUNBARREL+j].renderfx = renderfx;
		VectorCopy (part[BP_HELO_GUNBARREL+j].origin, part[BP_HELO_GUNBARREL+j].oldorigin);
		CGame::addVehiclePart( &part[BP_HELO_GUNBARREL+j] );

	}

	// sound
	if( drawInfo->basicInfo.entityNum >= 0 ) {
			SoundSystem::addLoopingSound( drawInfo->basicInfo.entityNum, drawInfo->basicInfo.origin, vec3_origin, 
									cgs.media.engineHelo );
	}

	// muzzleflash
	if( drawInfo->basicInfo.drawMuzzleFlash ) {
		if( veh->flags & VF_DOORGUNS ) {
			// only the door gun that bears on the aim is firing
			for( int gun = 0; gun < 2; ++gun ) {
				if( doorGunBears( gun == 1, drawInfo->doorGunAim ) && part[BP_HELO_GUNBARREL + 2 * gun].hModel )
					CGame::vehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_HELO_GUNBARREL + 2 * gun],
										   veh->handle[BP_HELO_GUNBARREL + 2 * gun], drawInfo->basicInfo.vehicleIndex );
			}
		} else {
			CGame::vehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_HELO_GUNBARREL], 
								   veh->handle[BP_HELO_GUNBARREL], drawInfo->basicInfo.vehicleIndex );
		}
	}

}
