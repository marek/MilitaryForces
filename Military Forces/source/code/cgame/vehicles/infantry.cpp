#include "../cg_local.h"
#include <algorithm>

/*
 * $Id: cg_lqm.c,v 1.5 2005-11-21 17:28:20 thebjoern Exp $
*/




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
	Vec3			velocity;	
	Vec3		    right, up, temp, start;
	DrawInfo_LQM_t drawInfo;
	int				ONOFF = this->currentState.ONOFF;


	drawInfo = {};
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
		Vec3	forward, ang, end;
		trace_t	tr;
//		playerState_t * ps = &cg.snap->ps;
		float len;
		float mindist = cg_thirdPersonRange.integer + availableVehicles[ci->vehicle].cam_dist[ CAMERA_V_DEFAULT ] + availableVehicles[ci->vehicle].maxs[0] + 20;

		reticle = {};	

		CGame::resetReticles();

		reticle.customShader = availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshair;
		reticle.hModel = cgs.media.reticle[availableWeapons[availableVehicles[ci->vehicle].weapons[this->currentState.weaponNum]].crosshair];

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

/*
 * $Id: cg_vehicle.c,v 1.12 2007-06-17 17:11:12 minkis Exp $
*/


constexpr int MAX_LQM_MODELS = 32;

struct LQMAnimations_s
{
	int			numAnimations;
	animation_t	anim[MAX_LQM_MODELS][MAX_ANIMATIONS];
};

using LQMAnimations_t = LQMAnimations_s;

LQMAnimations_t availableLQMAnimations;

/*
======================
CGame::parseLQMAnimationFile

Read a configuration file containing animation coutns and rates
models/players/visor/animation.cfg, etc
======================
*/
animation_t * CGame::parseLQMAnimationFile( const char *filename ) {
	char		*text_p, *prev;
	int			len;
	int			i;
	char		*token;
	float		fps;
	int			skip;
	char		text[20000];
	fileHandle_t	f;
	animation_t *animations;

	animations = reinterpret_cast<animation_t *>(&availableLQMAnimations.anim[availableLQMAnimations.numAnimations]);

	// load the file
	const OpenFile file( filename, FS_READ );
	f = file.handle();
	len = file.length();
	if ( len <= 0 ) {
		return nullptr;
	}
	if ( len >= sizeof( text ) - 1 ) {
		CGame::printf( "File %s too long\n", filename );
		return nullptr;
	}
	FileSystem::read( text, len, f );
	text[len] = 0;

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning

	// read optional parameters
	while ( true ) {
		prev = text_p;	// so we can unget
		token = COM_Parse( &text_p );
		if ( !token[0] ) {
			break;
		}

		if ( !Q_stricmp( token, "footsteps" ) || !Q_stricmp( token, "sex" ) ) {
			token = COM_Parse( &text_p );
			if ( !token[0] ) {
				break;
			}
			continue;
		} else if ( !Q_stricmp( token, "headoffset" ) || !Q_stricmp( token, "headOffset" ) ) {
			for ( i = 0 ; i < 3 ; i++ ) {
				token = COM_Parse( &text_p );
				if ( !token[0] ) {
					break;
				}
			}
			continue;
		} else if ( !Q_stricmp( token, "fixedlegs" ) || !Q_stricmp( token, "fixedtorso" ) ) {
			continue;
		}

		// if it is a number, start parsing animations
		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}
		Common::printf( "unknown token '%s' is %s\n", token, filename );
	}

	// read information for each frame
	for ( i = 0 ; i < MAX_ANIMATIONS ; i++ ) {

		token = COM_Parse( &text_p );
		if ( !*token ) {
			if( i >= TORSO_GETFLAG && i <= TORSO_NEGATIVE ) {
				animations[i].firstFrame = animations[TORSO_GESTURE].firstFrame;
				animations[i].frameLerp = animations[TORSO_GESTURE].frameLerp;
				animations[i].initialLerp = animations[TORSO_GESTURE].initialLerp;
				animations[i].loopFrames = animations[TORSO_GESTURE].loopFrames;
				animations[i].numFrames = animations[TORSO_GESTURE].numFrames;
				animations[i].reversed = false;
				animations[i].flipflop = false;
				continue;
			}
			break;
		}
		animations[i].firstFrame = atoi( token );
		// leg only frames are adjusted to not count the upper body only frames
		if ( i == LEGS_WALKCR ) {
			skip = animations[LEGS_WALKCR].firstFrame - animations[TORSO_GESTURE].firstFrame;
		}
		if ( i >= LEGS_WALKCR && i<TORSO_GETFLAG) {
			animations[i].firstFrame -= skip;
		}

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		animations[i].numFrames = atoi( token );

		animations[i].reversed = false;
		animations[i].flipflop = false;
		// if numFrames is negative the animation is reversed
		if (animations[i].numFrames < 0) {
			animations[i].numFrames = -animations[i].numFrames;
			animations[i].reversed = true;
		}

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		animations[i].loopFrames = atoi( token );

		token = COM_Parse( &text_p );
		if ( !*token ) {
			break;
		}
		fps = atof( token );
		if ( fps == 0 ) {
			fps = 1;
		}
		animations[i].frameLerp = 1000 / fps;
		animations[i].initialLerp = 1000 / fps;
	}

	if ( i != MAX_ANIMATIONS ) {
		CGame::printf( "Error parsing animation file: %s", filename );
		return nullptr;
	}

	// crouch backward animation
	animations[LEGS_BACKCR] = animations[LEGS_WALKCR];
	animations[LEGS_BACKCR].reversed = true;
	// walk backward animation
	animations[LEGS_BACKWALK] = animations[LEGS_WALK];
	animations[LEGS_BACKWALK].reversed = true;
	// flag moving fast
	animations[FLAG_RUN].firstFrame = 0;
	animations[FLAG_RUN].numFrames = 16;
	animations[FLAG_RUN].loopFrames = 16;
	animations[FLAG_RUN].frameLerp = 1000 / 15;
	animations[FLAG_RUN].initialLerp = 1000 / 15;
	animations[FLAG_RUN].reversed = false;
	// flag not moving or moving slowly
	animations[FLAG_STAND].firstFrame = 16;
	animations[FLAG_STAND].numFrames = 5;
	animations[FLAG_STAND].loopFrames = 0;
	animations[FLAG_STAND].frameLerp = 1000 / 20;
	animations[FLAG_STAND].initialLerp = 1000 / 20;
	animations[FLAG_STAND].reversed = false;
	// flag speeding up
	animations[FLAG_STAND2RUN].firstFrame = 16;
	animations[FLAG_STAND2RUN].numFrames = 5;
	animations[FLAG_STAND2RUN].loopFrames = 1;
	animations[FLAG_STAND2RUN].frameLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].initialLerp = 1000 / 15;
	animations[FLAG_STAND2RUN].reversed = true;

	availableLQMAnimations.numAnimations++;
	return animations;
}

/*
===============
CGame::cacheGroundVehicle

On startup cache it
===============
*/

void CGame::cacheLQM(int index)
{
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/lqms/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );
// changed mg
	for( i = 0; i < BP_LQM_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_LQM_LEGS:
			Com_sprintf( name, sizeof(name), "%s_legs.md3", basename );
			break;
		case BP_LQM_TORSO:
			Com_sprintf( name, sizeof(name), "%s_torso.md3", basename );
			break;
		case BP_LQM_HEAD:
			Com_sprintf( name, sizeof(name), "%s_head.md3", basename );
			break;
		}
		availableVehicles[index].handle[i] = CGame::registerVehiclePart( name, true );
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_LQM_TORSO] ) {
		Common::printf( S_COLOR_YELLOW "WARNING: MFQ3: missing body %s_torso.md3, model not loaded\n", basename );
		return;
	}

	// Parse LQM Animations
	Com_sprintf( name, sizeof(name), "models/vehicles/lqms/%s/animation.cfg",  availableVehicles[index].modelName );
	availableVehicles[index].animations = CGame::parseLQMAnimationFile(name);
	if(!availableVehicles[index].animations) 
		Common::printf( S_COLOR_YELLOW "WARNING: MFQ3: cannot parse %s: animation.cfg\n", basename );
}

char *lqm_tags[BP_LQM_MAX_PARTS] =
{
	"<no tag>",		// vehicle body		
	"tag_torso",
	"tag_head"		
};

// whether a frame belongs to one of the soldier's animations
static bool inFrameRange( const completeVehicleData_t* veh, int frame, int anim )
{
	return frame >= veh->animations[anim].firstFrame && frame <= veh->animations[anim].firstFrame + veh->animations[anim].numFrames - 1;
}

void CGame::drawLQM(DrawInfo_LQM_t* drawInfo)
{
	refEntity_t	    part[BP_LQM_MAX_PARTS+1];	// +1 for muzzle flash
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				anim = drawInfo->anim;
	int				i;
	Vec3			angles;
	vec3_t			axis[3];
	float			diff = 0;
	int				*lastTorsoAngle = &drawInfo->lastTorsoAngle;
	int				*lastLegsAngle = &drawInfo->lastLegsAngle;
	int				legsTurnAngle = 45;
	int				torsoTurnAngle = 30;
	int				lastLegsFrame;

	completeVehicleData_t* veh = &availableVehicles[drawInfo->basicInfo.vehicleIndex];
	cg_drawTeam = drawInfo->basicInfo.team;

	// a soldier of a vehicle set this game did not load has no animations to draw with
	if( !veh->animations )
		return;

	for( i = 0; i < BP_LQM_MAX_PARTS+1; i++ ) 
	{
	    memset( &part[i], 0, sizeof(part[0]) );	
	}

	// use the same origin for all
    renderfx |= RF_LIGHTING_ORIGIN;

	/*
	if(drawInfo->basicInfo.entityNum == cg.clientNum)
		renderfx |= RF_THIRD_PERSON;*/

	if( (anim & A_LQM_FORWARD) || 
		(anim & A_LQM_BACKWARD) || 
		(anim & A_LQM_LEFT) || 
		(anim & A_LQM_RIGHT) || 
		drawInfo->basicInfo.drawMuzzleFlash ) 
	{
		legsTurnAngle = 0;
		torsoTurnAngle = 0;
	}

	// Time Next Animations	
	lastLegsFrame = drawInfo->legsFrame;
	if(drawInfo->legsTime < cg.time) 
	{
		drawInfo->legsFrame++;
		if( inFrameRange( veh, drawInfo->legsFrame, LEGS_WALKCR) ) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_WALKCR].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, LEGS_JUMP)) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_JUMP].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, LEGS_JUMPB)) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_JUMPB].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, LEGS_RUN)) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_RUN].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, LEGS_BACK)) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_BACK].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, BOTH_DEATH1)) 
			drawInfo->legsTime = cg.time + veh->animations[BOTH_DEATH1].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, LEGS_IDLE)) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_IDLE].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, LEGS_IDLECR)) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_IDLECR].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, LEGS_LAND)) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_LAND].frameLerp; 
		else if(inFrameRange( veh, drawInfo->legsFrame, LEGS_LANDB)) 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_LANDB].frameLerp; 
		else 
			drawInfo->legsTime = cg.time + veh->animations[LEGS_RUN].frameLerp; 
	}

	// I droped the idea of using offsets for "legsFrame/torsoFrame", 
	// less variables to keep track, slightly more calculation, but just as doable
	if(drawInfo->basicInfo.ONOFF & OO_STALLED) 
	{
		if(inFrameRange( veh, lastLegsFrame, LEGS_JUMP))
			drawInfo->legsFrame = veh->animations[LEGS_LAND].firstFrame;
		if(inFrameRange( veh, lastLegsFrame, LEGS_JUMPB))
			drawInfo->legsFrame = veh->animations[LEGS_LANDB].firstFrame;

		if(inFrameRange( veh, drawInfo->legsFrame, LEGS_LANDB)) 
		{
			if(drawInfo->legsFrame > veh->animations[LEGS_LANDB].firstFrame+veh->animations[LEGS_LANDB].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_LANDB].firstFrame + veh->animations[LEGS_LANDB].numFrames-1;
			part[BP_LQM_LEGS].frame = drawInfo->legsFrame;	
		} 
		else 
		{
			if(drawInfo->legsFrame > veh->animations[LEGS_LAND].firstFrame+veh->animations[LEGS_LAND].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_LAND].firstFrame + veh->animations[LEGS_LAND].numFrames-1;
			part[BP_LQM_LEGS].frame = drawInfo->legsFrame;	
		}
	} 
	else if(anim & A_LQM_BACKWARD) 
	{
		if((anim & A_LQM_CROUCH) && (anim & A_LQM_EJECT)) 
		{	// jump
			if(drawInfo->legsFrame < veh->animations[LEGS_JUMPB].firstFrame) 
				drawInfo->legsFrame = veh->animations[LEGS_JUMPB].firstFrame;
			else if(lastLegsFrame > veh->animations[LEGS_JUMPB].firstFrame+veh->animations[LEGS_JUMPB].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_JUMPB].firstFrame;	// Make sure to start from begining
			else if(drawInfo->legsFrame > veh->animations[LEGS_JUMPB].firstFrame+veh->animations[LEGS_JUMPB].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_JUMPB].firstFrame + veh->animations[LEGS_JUMPB].numFrames-1;
			part[BP_LQM_LEGS].frame = drawInfo->legsFrame;			
		} 
		else if(anim & A_LQM_CROUCH) 
		{
			if(drawInfo->legsFrame < veh->animations[LEGS_WALKCR].firstFrame) 
				drawInfo->legsFrame = veh->animations[LEGS_WALKCR].firstFrame;
			else if(drawInfo->legsFrame > veh->animations[LEGS_WALKCR].firstFrame+veh->animations[LEGS_WALKCR].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_WALKCR].firstFrame;
			part[BP_LQM_LEGS].frame = ( veh->animations[LEGS_WALKCR].firstFrame + 
				veh->animations[LEGS_WALKCR].numFrames-1 ) - 
				( drawInfo->legsFrame-veh->animations[LEGS_WALKCR].firstFrame );
		} 
		else 
		{
			if(drawInfo->legsFrame < veh->animations[LEGS_BACK].firstFrame) 
				drawInfo->legsFrame = veh->animations[LEGS_BACK].firstFrame;
			else if(drawInfo->legsFrame > veh->animations[LEGS_BACK].firstFrame+veh->animations[LEGS_BACK].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_BACK].firstFrame;
			part[BP_LQM_LEGS].frame =  drawInfo->legsFrame;
		}
	} 
	else if((anim & A_LQM_FORWARD) || (anim & A_LQM_LEFT) || (anim & A_LQM_RIGHT)) 
	{
		if((anim & A_LQM_CROUCH) && (anim & A_LQM_EJECT)) // jump
		{
			if(drawInfo->legsFrame < veh->animations[LEGS_JUMP].firstFrame) 
				drawInfo->legsFrame = veh->animations[LEGS_JUMP].firstFrame;
			else if(lastLegsFrame > veh->animations[LEGS_JUMP].firstFrame+veh->animations[LEGS_JUMP].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_JUMP].firstFrame;	// Make sure to start from begining
			else if(drawInfo->legsFrame > veh->animations[LEGS_JUMP].firstFrame+veh->animations[LEGS_JUMP].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_JUMP].firstFrame + veh->animations[LEGS_JUMP].numFrames-1;
			part[BP_LQM_LEGS].frame = drawInfo->legsFrame;	
		} 
		else if(anim & A_LQM_CROUCH) 
		{
			if(anim & A_LQM_RIGHT) 
			{	// Play left animation backwards, so legs arent facing backward
				if(drawInfo->legsFrame < veh->animations[LEGS_WALKCR].firstFrame) 
					drawInfo->legsFrame = veh->animations[LEGS_WALKCR].firstFrame;
				else if(drawInfo->legsFrame > veh->animations[LEGS_WALKCR].firstFrame+veh->animations[LEGS_WALKCR].numFrames-1) 
					drawInfo->legsFrame = veh->animations[LEGS_WALKCR].firstFrame;
				part[BP_LQM_LEGS].frame = ( veh->animations[LEGS_WALKCR].firstFrame +
					veh->animations[LEGS_WALKCR].numFrames-1 ) - 
					(drawInfo->legsFrame-veh->animations[LEGS_WALKCR].firstFrame);
			} 
			else 
			{
				if(drawInfo->legsFrame < veh->animations[LEGS_WALKCR].firstFrame) 
					drawInfo->legsFrame = veh->animations[LEGS_WALKCR].firstFrame;
				else if(drawInfo->legsFrame > veh->animations[LEGS_WALKCR].firstFrame+veh->animations[LEGS_WALKCR].numFrames-1) 
					drawInfo->legsFrame = veh->animations[LEGS_WALKCR].firstFrame;
				part[BP_LQM_LEGS].frame = drawInfo->legsFrame;
			}
		} 
		else 
		{
			if(drawInfo->legsFrame < veh->animations[LEGS_RUN].firstFrame) 
				drawInfo->legsFrame = veh->animations[LEGS_RUN].firstFrame;
			else if(drawInfo->legsFrame > veh->animations[LEGS_RUN].firstFrame+veh->animations[LEGS_RUN].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_RUN].firstFrame;
			part[BP_LQM_LEGS].frame = drawInfo->legsFrame;
		}
	} 
	else if (anim & A_LQM_DIE) 
	{
		if(drawInfo->legsFrame < veh->animations[BOTH_DEATH1].firstFrame) 
			drawInfo->legsFrame = veh->animations[BOTH_DEATH1].firstFrame;
		else if(drawInfo->legsFrame > veh->animations[BOTH_DEATH1].firstFrame+veh->animations[BOTH_DEATH1].numFrames-1) 
			drawInfo->legsFrame = veh->animations[BOTH_DEAD1].firstFrame;
		part[BP_LQM_LEGS].frame = drawInfo->legsFrame;
	} 
	else 
	{
		if((anim & A_LQM_CROUCH) && (anim & A_LQM_EJECT)) // jump
		{
			if(drawInfo->legsFrame < veh->animations[LEGS_JUMP].firstFrame) 
				drawInfo->legsFrame = veh->animations[LEGS_JUMP].firstFrame;
			else if(lastLegsFrame > veh->animations[LEGS_JUMP].firstFrame+veh->animations[LEGS_JUMP].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_JUMP].firstFrame;	// Make sure to start from begining
			else if(drawInfo->legsFrame > veh->animations[LEGS_JUMP].firstFrame+veh->animations[LEGS_JUMP].numFrames-1)
				drawInfo->legsFrame = veh->animations[LEGS_JUMP].firstFrame + veh->animations[LEGS_JUMP].numFrames-1;
			part[BP_LQM_LEGS].frame = drawInfo->legsFrame;	
		} 
		else if(anim & A_LQM_CROUCH) 
		{
			if(drawInfo->legsFrame < veh->animations[LEGS_IDLECR].firstFrame) 
				drawInfo->legsFrame = veh->animations[LEGS_IDLECR].firstFrame;
			else if(drawInfo->legsFrame > veh->animations[LEGS_IDLECR].firstFrame+veh->animations[LEGS_IDLECR].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_IDLECR].firstFrame;
			part[BP_LQM_LEGS].frame = drawInfo->legsFrame;
		} 
		else 
		{
			if(drawInfo->legsFrame < veh->animations[LEGS_IDLE].firstFrame) 
				drawInfo->legsFrame = veh->animations[LEGS_IDLE].firstFrame;
			else if(drawInfo->legsFrame > veh->animations[LEGS_IDLE].firstFrame+veh->animations[LEGS_IDLECR].numFrames-1) 
				drawInfo->legsFrame = veh->animations[LEGS_IDLE].firstFrame;
			part[BP_LQM_LEGS].frame = drawInfo->legsFrame;
		}
	}
		
	// Torso Animations
	if(drawInfo->basicInfo.drawMuzzleFlash) 
	{
		drawInfo->torsoFrame = veh->animations[TORSO_ATTACK].firstFrame;
		drawInfo->torsoTime = cg.time + veh->animations[TORSO_ATTACK].frameLerp;
	} 
	else if (anim & A_LQM_DIE) 
	{
		if(drawInfo->torsoTime < cg.time) 
		{
			drawInfo->torsoFrame++;
			drawInfo->torsoTime = cg.time + veh->animations[BOTH_DEATH1].frameLerp; 
		}
		if(drawInfo->torsoFrame > veh->animations[BOTH_DEATH1].numFrames-1) 
			drawInfo->torsoFrame = veh->animations[BOTH_DEAD1].firstFrame;
		part[BP_LQM_TORSO].frame = drawInfo->torsoFrame;
	} 
	else if (drawInfo->torsoFrame >= veh->animations[TORSO_ATTACK].firstFrame && 
		drawInfo->torsoFrame < veh->animations[TORSO_ATTACK].firstFrame+veh->animations[TORSO_ATTACK].numFrames-1) 
	{
		if(drawInfo->torsoTime < cg.time) 
		{
			drawInfo->torsoFrame++;
			drawInfo->torsoTime = cg.time + veh->animations[TORSO_ATTACK].frameLerp; 
		}
	} 
	else 
		drawInfo->torsoFrame = veh->animations[TORSO_STAND].firstFrame;
	part[BP_LQM_TORSO].frame = drawInfo->torsoFrame;

	//
    // Add legs
	//
    part[BP_LQM_LEGS].hModel = veh->handle[BP_LQM_LEGS];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_LQM_LEGS].origin );
    VectorCopy( drawInfo->basicInfo.origin, part[BP_LQM_LEGS].lightingOrigin );
	VectorCopy( drawInfo->basicInfo.angles, angles );
	angles[0] = 0;

	if((anim & A_LQM_FORWARD) && (anim & A_LQM_LEFT)) 
		angles[1] += 45;
	else if((anim & A_LQM_FORWARD) && (anim & A_LQM_RIGHT)) 
		angles[1] -= 45;
	else if((anim & A_LQM_BACKWARD) && (anim & A_LQM_LEFT)) 
		angles[1] -= 45;
	else if((anim & A_LQM_BACKWARD) && (anim & A_LQM_RIGHT)) 
		angles[1] += 45;
	else if (anim & A_LQM_LEFT) 
		angles[1] += 90;
	else if ((anim & A_LQM_RIGHT) && (anim & A_LQM_CROUCH)) 
		angles[1] += 90;
	else if (anim & A_LQM_RIGHT) 
		angles[1] -= 90;
	
	diff =  drawInfo->basicInfo.angles[1] - angles[1];
	if(abs(angles[1] - *lastLegsAngle) < legsTurnAngle) 
	{
		diff += angles[1] - *lastLegsAngle;
		angles[1] = *lastLegsAngle;
	} 
	else 
		*lastLegsAngle = angles[1];
	

	AnglesToAxis(angles, axis);
	for(i = 0; i<3;i++)
		VectorScale(axis[i],(float)LQM_SCALE,axis[i]);
	AxisCopy(axis, part[BP_LQM_LEGS].axis );
    part[BP_LQM_LEGS].shadowPlane = shadowPlane;
    part[BP_LQM_LEGS].renderfx = renderfx;
    VectorCopy (part[BP_LQM_LEGS].origin, part[BP_LQM_LEGS].oldorigin);
    CGame::addVehiclePart( &part[BP_LQM_LEGS] );

	//
	// Add Torso
	//
	part[BP_LQM_TORSO].hModel = veh->handle[BP_LQM_TORSO];
	VectorCopy( drawInfo->basicInfo.origin, part[BP_LQM_TORSO].lightingOrigin );
	VectorCopy( drawInfo->basicInfo.angles, angles );
	angles[2] = 0;
	angles[1] = diff;

	if(abs(angles[1] - *lastTorsoAngle) < torsoTurnAngle) 
	{
		diff = angles[1] - *lastTorsoAngle;
		angles[1] = *lastTorsoAngle;
	} 
	else 
	{ 
		diff = 0;
		*lastTorsoAngle = angles[1];
	}

	// Counter Leg & camera Left/right movement
	angles[0] = angles[0] < 0 ? std::max(-25.0f,  angles[0]) :  std::min(25.0f,  angles[0]);
	AnglesToAxis(angles, axis);
	AxisCopy(axis, part[BP_LQM_TORSO].axis );
	CGame::positionRotatedEntityOnTag( &part[BP_LQM_TORSO], &part[BP_LQM_LEGS], veh->handle[BP_LQM_LEGS], lqm_tags[BP_LQM_TORSO] );
	part[BP_LQM_TORSO].shadowPlane = shadowPlane;
	part[BP_LQM_TORSO].renderfx = renderfx;
	CGame::addVehiclePart( &part[BP_LQM_TORSO] );

	// Add Head
	part[BP_LQM_HEAD].hModel = veh->handle[BP_LQM_HEAD];
	VectorCopy( drawInfo->basicInfo.origin, part[BP_LQM_HEAD].lightingOrigin );
	VectorCopy( drawInfo->basicInfo.angles, angles );
	angles[2] = 0;
	angles[1] = diff;
	angles[0] = angles[0] < 0 ? std::max(-25.0f,  angles[0]) : std::min(25.0f, angles[0]);
	AnglesToAxis(angles, axis);
	AxisCopy(axis, part[BP_LQM_HEAD].axis );
	CGame::positionRotatedEntityOnTag( &part[BP_LQM_HEAD], &part[BP_LQM_TORSO], veh->handle[BP_LQM_TORSO], lqm_tags[BP_LQM_HEAD] );
	part[BP_LQM_HEAD].shadowPlane = shadowPlane;
	part[BP_LQM_HEAD].renderfx = renderfx;
	CGame::addVehiclePart( &part[BP_LQM_HEAD] );

	// Parachute: the canopy and ropes of the supply crate, swaying gently above the soldier
	if( (anim & A_LQM_CHUTE) && cgs.media.parachute )
	{
		constexpr float canopyScale = 0.45f;
		constexpr float sway = 7.0f;
		refEntity_t chute;
		chute = {};
		chute.hModel = cgs.media.parachute;
		VectorCopy( drawInfo->basicInfo.origin, chute.origin );
		chute.origin[2] += 1.0f;
		VectorCopy( chute.origin, chute.lightingOrigin );
		VectorCopy( chute.origin, chute.oldorigin );
		vec3_t canopyAngles = { sway * static_cast<float>( sin( cg.time * 0.0021 ) ), drawInfo->basicInfo.angles[1], sway * static_cast<float>( cos( cg.time * 0.0017 ) ) };
		AnglesToAxis( canopyAngles, chute.axis );
		for( int k = 0; k < 3; k++ )
			VectorScale( chute.axis[k], canopyScale, chute.axis[k] );
		chute.nonNormalizedAxes = true;
		chute.renderfx = renderfx | RF_NOSHADOW;
		CGame::addVehiclePart( &chute );
	}

	// Add Weapon
	part[BP_LQM_MAX_PARTS].hModel = availableWeapons[drawInfo->weaponIndex].modelHandle;
	VectorCopy( drawInfo->basicInfo.origin, part[BP_LQM_MAX_PARTS].lightingOrigin );
	AxisCopy( axisDefault , part[BP_LQM_MAX_PARTS].axis); 
	CGame::positionRotatedEntityOnTag( &part[BP_LQM_MAX_PARTS], &part[BP_LQM_TORSO], veh->handle[BP_LQM_TORSO], "tag_weap" );
	part[BP_LQM_MAX_PARTS].shadowPlane = shadowPlane;
	part[BP_LQM_MAX_PARTS].renderfx = renderfx;
	CGame::addVehiclePart( &part[BP_LQM_MAX_PARTS] );

	// muzzleflash
	if( drawInfo->basicInfo.drawMuzzleFlash ) {
		CGame::vehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_LQM_MAX_PARTS], 
							   availableWeapons[drawInfo->weaponIndex].modelHandle, drawInfo->basicInfo.vehicleIndex );
	}

}
