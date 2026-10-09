#include "../cg_local.h"
#include <algorithm>

/*
 * $Id: cg_plane.c,v 1.6 2007-06-17 17:11:12 minkis Exp $
*/



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
		drawInfo.basicInfo.flashStations = this->stationsFired;
		drawInfo.basicInfo.flashOwnAim = this->currentState.clientNum == cg.snap->ps.clientNum;
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
				reticlelock = reticle;
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

void CGame::cachePlane(int index)
{
	char name[128];
	char basename[128];
	int i;
	unsigned int caps = availableVehicles[index].caps;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/planes/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );

	for( i = 0; i < BP_PLANE_MAX_PARTS; i++ ) {
		

		switch(i) {
		case BP_PLANE_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			break;
		case BP_PLANE_CONTROLS:
			Com_sprintf( name, sizeof(name), "%s_controls.md3", basename );
			break;
		case BP_PLANE_COCKPIT:
			Com_sprintf( name, sizeof(name), "%s_cockpit.md3", basename );
			break;
		case BP_PLANE_GEAR:
			Com_sprintf( name, sizeof(name), "%s_gear.md3", basename );
			break;
		case BP_PLANE_BRAKES:
			Com_sprintf( name, sizeof(name), "%s_breaks.md3", basename );
			break;
		case BP_PLANE_BAY:
			Com_sprintf( name, sizeof(name), "%s_bay.md3", basename );
			break;
		case BP_PLANE_WINGLEFT:
			Com_sprintf( name, sizeof(name), "%s_wingL.md3", basename );
			break;
		case BP_PLANE_WINGRIGHT:
			Com_sprintf( name, sizeof(name), "%s_wingR.md3", basename );
			break;
		case BP_PLANE_SPECIAL:
			Com_sprintf( name, sizeof(name), "%s_special.md3", basename );
			break;
		case BP_PLANE_PROP:
			Com_sprintf( name, sizeof(name), "%s_prop.md3", basename );
			break;
		}

		availableVehicles[index].handle[i] = CGame::registerVehiclePart( name, 
			( i == BP_PLANE_GEAR && ( caps & HC_GEAR ) ) ||
			( i == BP_PLANE_BRAKES && ( caps & HC_SPEEDBRAKE ) ) ||
			( i == BP_PLANE_BAY && ( caps & HC_WEAPONBAY ) ) ||
			( ( i == BP_PLANE_WINGLEFT || i == BP_PLANE_WINGRIGHT ) && ( caps & HC_SWINGWING ) ) ||
			( i == BP_PLANE_PROP && ( caps & HC_PROP ) ) );

//		if( !availableVehicles[index].handle[i] ) {
//			CGame::printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_PLANE_BODY] ) {
		Common::printf( S_COLOR_YELLOW "WARNING: MFQ3: missing body %s.md3, model not loaded\n", basename );
	}
}

/*
 * $Id: cg_vehicledraw.c,v 1.6 2005-11-21 17:28:20 thebjoern Exp $
*/


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

void CGame::planePilot( const refEntity_t *parent, qhandle_t parentModel, int idx ) {

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
			CGame::positionRotatedEntityOnTag( &pilot[k], parent, parentModel, tag);
			CGame::addVehiclePart( &pilot[k] );
		}
	}
}

void CGame::drawPlane(DrawInfo_Plane_t* drawInfo)
{
	refEntity_t	    part[BP_PLANE_MAX_PARTS];
	refEntity_t		vapor;
	refEntity_t		burner;
	refEntity_t		burner2;
	refEntity_t		reticle;
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				i;
	completeVehicleData_t* veh = &availableVehicles[drawInfo->basicInfo.vehicleIndex];
	cg_drawTeam = drawInfo->basicInfo.team;

	for( i = 0; i < BP_PLANE_MAX_PARTS; i++ ) 
	    memset( &part[i], 0, sizeof(part[0]) );	
    vapor = {};	
    burner = {};	
    burner2 = {};	
	reticle = {};	

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
	
	// control surfaces
	part[BP_PLANE_CONTROLS].frame = drawInfo->controlFrame;
	
	// gear
	part[BP_PLANE_GEAR].frame = drawInfo->gearFrame;
	
	// bay
	part[BP_PLANE_BAY].frame = drawInfo->bayFrame;

	// speedbrakes
	part[BP_PLANE_BRAKES].frame = drawInfo->brakeFrame;
	
	part[BP_PLANE_COCKPIT].frame = drawInfo->cockpitFrame;

    // plane body
    part[BP_PLANE_BODY].hModel = veh->handle[BP_PLANE_BODY];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_PLANE_BODY].origin );
	AxisCopy( drawInfo->basicInfo.axis, part[BP_PLANE_BODY].axis );
    VectorCopy( drawInfo->basicInfo.origin, part[BP_PLANE_BODY].lightingOrigin );
    part[BP_PLANE_BODY].shadowPlane = shadowPlane;
    part[BP_PLANE_BODY].renderfx = renderfx;
    VectorCopy (part[BP_PLANE_BODY].origin, part[BP_PLANE_BODY].oldorigin);
	part[BP_PLANE_BODY].frame = drawInfo->bodyFrame;
	CGame::addVehiclePart( &part[BP_PLANE_BODY] );
	
	// if the model failed, allow the default nullmodel to be displayed, but nothing else
	if (!part[BP_PLANE_BODY].hModel) 
		return;

	// other parts
	for( i = 1; i < BP_PLANE_MAX_PARTS; i++ ) 
	{
		part[i].hModel = veh->handle[i];
		if( !part[i].hModel ) 
			continue;
		VectorCopy( drawInfo->basicInfo.origin, part[i].lightingOrigin );
		AxisCopy( axisDefault, part[i].axis );
		if( i == BP_PLANE_PROP && (veh->caps & HC_PROP) ) 
		{
			int ii;
			RotateAroundDirection( part[i].axis, cg.time );
			for( ii = 1; ii < veh->engines; ++ii ) 
			{
				refEntity_t engine;
				engine = part[i];
				CGame::positionRotatedEntityOnTag( &engine, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], engine_tags[ii-1] );
				engine.shadowPlane = shadowPlane;
				engine.renderfx = renderfx;
				CGame::addVehiclePart( &engine );
			}
		}
		if( (veh->caps & HC_SWINGWING) && 
			(i == BP_PLANE_WINGLEFT || i == BP_PLANE_WINGRIGHT)  ) 
		{
			//Common::printf( "Swingwing: %f\n", drawInfo->swingAngle );
			if( i == BP_PLANE_WINGLEFT ) 
				RotateAroundYaw( part[BP_PLANE_WINGLEFT].axis, drawInfo->swingAngle );
			else if( i == BP_PLANE_WINGRIGHT )
				RotateAroundYaw( part[BP_PLANE_WINGRIGHT].axis, -drawInfo->swingAngle );
		}
		CGame::positionRotatedEntityOnTag( &part[i], &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], plane_tags[i] );
		part[i].shadowPlane = shadowPlane;
		part[i].renderfx = renderfx;
		CGame::addVehiclePart( &part[i] );
	}
	// pilot
	CGame::planePilot( &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], drawInfo->basicInfo.vehicleIndex );
	
	// vwep
	if( (veh->renderFlags & MFR_VWEP) && drawInfo->basicInfo.usedLoadout ) 
	{
		refEntity_t			vwep;

//		for( i = 0; i < drawInfo->basicInfo.loadout->usedMounts; ++i ) 
		for( i = 0; i < drawInfo->basicInfo.usedLoadout->usedMounts; ++i )
		{
			if( drawInfo->basicInfo.usedLoadout->mounts[i].weapon && 
				drawInfo->basicInfo.numWeaponsOnMount[i] ) 
			{
				vwep = {};		
				vwep.hModel = availableWeapons[drawInfo->basicInfo.usedLoadout->mounts[i].weapon].vwepHandle;
				VectorCopy( drawInfo->basicInfo.origin, vwep.lightingOrigin );
				AxisCopy( axisDefault, vwep.axis );
				CGame::positionEntityOnTag( &vwep, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], 
						drawInfo->basicInfo.usedLoadout->mounts[i].tag.name );
				vwep.shadowPlane = shadowPlane;
				vwep.renderfx = renderfx;
				CGame::addVehiclePart( &vwep );
			}
		}
	}

	// vapor
	if( drawInfo->basicInfo.ONOFF & OO_VAPOR ) 
	{
		vapor.hModel = (veh->renderFlags & MFR_BIGVAPOR) ? cgs.media.vaporBig : cgs.media.vapor;
		vapor.customShader = cgs.media.vaporShader;
		if( drawInfo->basicInfo.speed < veh->stallspeed * SPEED_GREEN_ARC &&
			drawInfo->basicInfo.speed > veh->stallspeed * VAPORSPEED ) 
		{
			vapor.frame = 1;
		}
		VectorCopy( drawInfo->basicInfo.origin, vapor.lightingOrigin );
		AxisCopy( axisDefault, vapor.axis );
		CGame::positionEntityOnTag( &vapor, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], "tag_vapor1" );
		vapor.shadowPlane = shadowPlane;
		vapor.renderfx = renderfx;
		CGame::addVehiclePart( &vapor );
	}
	
	// throttle/afterburner
	if( drawInfo->basicInfo.throttle > 10 ) 
	{
		burner.hModel = cgs.media.afterburner[veh->effectModel];
		VectorCopy( drawInfo->basicInfo.origin, burner.lightingOrigin );
		AxisCopy( axisDefault, burner.axis );
		CGame::positionEntityOnTag( &burner, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], "tag_ab1" );
		burner.shadowPlane = shadowPlane;
		burner.renderfx = renderfx;
		burner.frame = drawInfo->bodyFrame;
		CGame::addVehiclePart( &burner );
		if( veh->engines > 1 ) 
		{
			burner2.hModel = cgs.media.afterburner[veh->effectModel];
			VectorCopy( drawInfo->basicInfo.origin, burner2.lightingOrigin );
			AxisCopy( axisDefault, burner2.axis );
			CGame::positionRotatedEntityOnTag( &burner2, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], "tag_ab2" );
			burner2.shadowPlane = shadowPlane;
			burner2.renderfx = renderfx;
			burner2.frame = drawInfo->bodyFrame;
			CGame::addVehiclePart( &burner2 );
		}
	}

	// sound
	if( drawInfo->basicInfo.entityNum >= 0 ) 
	{
		if ( veh->caps & HC_PROP ) {
			SoundSystem::addLoopingSound( drawInfo->basicInfo.entityNum, drawInfo->basicInfo.origin, vec3_origin, 
									cgs.media.engineProp );
		}
		else {
			if( drawInfo->basicInfo.throttle > 10 ) 
			{
				SoundSystem::addLoopingSound( drawInfo->basicInfo.entityNum, drawInfo->basicInfo.origin, vec3_origin, 
										cgs.media.engineJetAB );
			}
			else 
			{
				SoundSystem::addLoopingSound( drawInfo->basicInfo.entityNum, drawInfo->basicInfo.origin, vec3_origin,
										cgs.media.engineJet );
			}
		}
	}
	
	// muzzleflash
	if( drawInfo->basicInfo.drawMuzzleFlash ) 
	{
		if( veh->flags & VF_GUNSTATIONS )
			CGame::stationMuzzleFlashes( drawInfo->basicInfo.vehicleIndex, drawInfo->basicInfo.flashStations, &part[BP_PLANE_BODY],
							   drawInfo->basicInfo.flashOwnAim ? cg.snap->ps.viewangles : nullptr );
		else
			CGame::vehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_PLANE_BODY], 
							   veh->handle[BP_PLANE_BODY], drawInfo->basicInfo.vehicleIndex );
	}
	
}
