/*
 * $Id: cg_weapons.c,v 1.29 2004-12-16 19:22:15 minkis Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"

/*
==========================
CG_FFARTrail
==========================
*/
static void CG_FFARTrail( centity_t * cent, const weaponInfo_t *wi )
{
	localEntity_t	* smoke;
	vec3_t			up, pos, smoke_pos, velocity;
	int smoke_rad;
	static int	seed = 0x92;
	
	up[0] = 5 * Q_random(&seed);
	up[1] = 5 * Q_random(&seed);
	up[2] = (5 - 1) * Q_random(&seed) + 1;

	VectorCopy( cent->lerpOrigin, pos );

	// draw smoke slightly behind the entitiy position (using -velocity of the entity)
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	VectorNormalize( velocity );

	// calc adjusted position
	VectorScale( velocity, -24.0f, velocity );		// -24.0f is just an arbitary distance which works OK with all current rocket models
	VectorAdd( pos, velocity, pos );

	VectorCopy( pos, smoke_pos );

	smoke_pos[0] += + Q_random( &seed ) * 2;
	smoke_pos[1] += + Q_random( &seed ) * 2;
	smoke_pos[2] += + Q_random( &seed ) * 2;
	smoke_rad = 1 + Q_random( &seed ) * 3;

	smoke = CG_SmokePuff( smoke_pos, up, 
						  smoke_rad, 
						  1.0f, 1.0f, 1.0f, 1.0f,
						  10500, 
						  cg.time, 3000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.missilePuffShader );
}


/*
==========================
CG_MissileTrail
==========================
*/
static void CG_MissileTrail( centity_t * cent, const weaponInfo_t *wi )
{
	localEntity_t	* smoke;
	vec3_t			up, pos, smoke_pos, velocity;
	int smoke_rad;
	static float smoke_rot = 0;
	static int	seed = 0x92;
	
	up[0] = 5 * Q_random(&seed);
	up[1] = 5 * Q_random(&seed);
	up[2] = (5 - 1) * Q_random(&seed) + 1;

	VectorCopy( cent->lerpOrigin, pos );
	
	// draw smoke slightly behind the entitiy position (using -velocity of the entity)
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	VectorNormalize( velocity );

	// calc adjusted position
	VectorScale( velocity, -24.0f, velocity );		// -24.0f is just an arbitary distance which works OK with all current rocket models
	VectorAdd( pos, velocity, pos );

	// draw trail
/*	smoke = CG_SmokePuff( pos, up, 
						  4, 
						  0.8f, 0.8f, 0.8f, 0.33f,
						  400, 
						  cg.time, 0,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );	*/

	VectorCopy( pos, smoke_pos );
	if(smoke_rot > 360)
	{
		smoke_rot = 0;
	}

	smoke_pos[0] += sin(smoke_rot) * 5 + Q_random( &seed ) * 5;
	smoke_pos[1] += sin(smoke_rot) * 5 + Q_random( &seed ) * 5;
	smoke_pos[2] += cos(smoke_rot) * 5 + Q_random( &seed ) * 5;
	smoke_rot += 1;
	smoke_rad = 4 + Q_random( &seed ) * 5;

	smoke = CG_SmokePuff( smoke_pos, up, 
						  smoke_rad, 
						  1.0f, 1.0f, 1.0f, 1.0f,
						  10500, 
						  cg.time, 3000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.missilePuffShader );
}

/*
==========================
CG_FlareTrail
==========================
*/
static void CG_FlareTrail( centity_t * cent, const weaponInfo_t *wi )
{
	localEntity_t	* smoke;
	vec3_t			up, pos, smoke_pos, velocity;
	int smoke_rad;
	static int	seed = 0x92;
	
	up[0] = 5 * Q_random(&seed);
	up[1] = 5 * Q_random(&seed);
	up[2] = (5 - 1) * Q_random(&seed) + 1;
	
	VectorCopy( cent->lerpOrigin, pos );

	// draw smoke slightly behind the entitiy position (using -velocity of the entity)
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	VectorNormalize( velocity );

	// calc adjusted position
	VectorScale( velocity, -24.0f, velocity );		// -24.0f is just an arbitary distance which works OK with all current rocket models
	VectorAdd( pos, velocity, pos );

	VectorCopy( pos, smoke_pos );

	smoke_pos[0] += Q_random( &seed ) * 5;
	smoke_pos[1] += Q_random( &seed ) * 5;
	smoke_pos[2] += Q_random( &seed ) * 5;
	smoke_rad = 1 + Q_random( &seed ) * 5;
	smoke = CG_SmokePuff( smoke_pos, up, 
						  smoke_rad, 
						  0.8f, 0.8f, 0.8f, 0.33f,
						  4000, 
						  cg.time, 3000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.flarePuffShader );
}


static void CG_MissileTrail2( centity_t * cent, const weaponInfo_t *wi ) {
	vec3_t  start, end;
	localEntity_t *le;
	refEntity_t   *re;

	VectorCopy(cent->lastDrawnTrailPos, start);
	VectorCopy(cent->lerpOrigin, end);


	// Only draw every so often dispite how often a trail is said to be updated
	// Or you may get a SHADER_MAX_VERTEXES error eith RT_RAIL_CORE draw method

	if(cent->TimeSinceLastTrail <= cg.time - 100)
	{

		// If the trail seems a bit to old, give it a starting origin
		if(cent->TimeSinceLastTrail <= cg.time - 200)
		{
			VectorCopy(cent->lerpOrigin, start); 
		}
	
		le = CG_AllocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		re = &le->refEntity;



		le->leType = LE_FADE_RGB;
		le->startTime = cg.time;
		le->endTime = cg.time + 5000;
		le->fadeInTime = 2000;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
 
		re->shaderTime = cg.time / 1000.0f;
		re->reType = RT_LIGHTNING; //RT_RAIL_CORE; //RT_BEAM; // RT_RAIL_CORE;
		re->customShader = cgs.media.railCoreShader;
	//	re->customShader = cgs.media.missileTrail2Shader;

		VectorCopy(start, re->origin);
		VectorCopy(start, le->pos.trBase);
	//	VectorCopy(end, le->pos.trDelta);
		VectorCopy(end, re->oldorigin);

		re->shaderRGBA[0] = 0.3f;
		re->shaderRGBA[1] = 0.3f;
		re->shaderRGBA[2] = 0.3f;
		re->shaderRGBA[3] = 0.3f;

		le->color[0] = 1.0f;
		le->color[1] = 1.0f;
		le->color[2] = 1.0f;
		le->color[3] = 1.0f;

		AxisClear( re->axis );

	
		// Update last time & Pos
		cent->TimeSinceLastTrail = cg.time;
		VectorCopy(cent->lerpOrigin, cent->lastDrawnTrailPos);
	}


}



/*
==========================
CG_NukeTrail
==========================
*/
static void CG_NukeTrail( centity_t * cent, const weaponInfo_t *wi )
{
	if(cent->TimeSinceLastTrail <= cg.time - 200)
	{
		localEntity_t	* smoke;
		vec3_t			up, pos, velocity;
	
		VectorCopy( cent->lerpOrigin, pos );
	
		// draw smoke slightly behind the entitiy position (using -velocity of the entity)
		VectorCopy( cent->currentState.pos.trDelta, velocity );
		VectorNormalize( velocity );

		// calc adjusted position
		VectorScale( velocity, -24.0f, velocity );		// -24.0f is just an arbitary distance which works OK with all current rocket models
		VectorAdd( pos, velocity, pos );

		// draw trail

		smoke = CG_SmokePuff( pos, up, 
							20, 
							0.0f, 0.0f, 0.0f, 0.8f,
							8000, 
							cg.time, 6000,
							LEF_PUFF_DONT_SCALE, 
							cgs.media.nukePuffShader );	
		cent->TimeSinceLastTrail = cg.time;
	}
}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapons() {
	weaponInfo_t	*weaponInfo;
	gitem_t			*ammo;
	int				i;
	
	// common
	cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
	cgs.media.rocketExplosionShader[0] = trap_R_RegisterShader( "rocketExplosion" );
	cgs.media.rocketExplosionShader[1] = trap_R_RegisterShader( "quakeRocketExplosion" );

	// for all weapon indexes in cg_weapons[]
	for( i = 0; i < WI_MAX; i++ )
	{
		weaponInfo = &cg_weapons[i];
		memset( weaponInfo, 0, sizeof( *weaponInfo ) );
		weaponInfo->registered = qtrue;

		for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
			if ( ammo->giType == IT_AMMO && ammo->giTag == availableWeapons[i].type ) {
				break;
			}
		}
		if ( ammo->classname && ammo->world_model[0] ) {
			weaponInfo->ammoModel = trap_R_RegisterModel( ammo->world_model[0] );
		}

		// switch on weapon type
		switch( availableWeapons[i].type )
		{
		case WT_ROCKET:
			// find out which model to use
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			
			// MFQ3: new sound
			weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rocketFly1.wav", qfalse );
			if( !weaponInfo->missileSound )
			{
				// MFQ3: old quake3 sound (backup)
				weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
			}

			// FFAR trail with no dynamic lighting
			weaponInfo->missileTrailFunc = CG_FFARTrail;
			//weaponInfo->missileTrailFunc = CG_MissileTrail2;
			weaponInfo->missileDlight = 0;
			
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			//MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

			// MFQ3: new sound
			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocketFire1.wav", qfalse );
			if( !weaponInfo->flashSound[0] )
			{
				// MFQ3: old quake3 sound (backup)
				weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			}
			break;
		case WT_ANTIAIRMISSILE:
		case WT_ANTIGROUNDMISSILE:
		case WT_ANTIRADARMISSILE:
			// find out which model to use
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			
			// MFQ3: new sound
			weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rocketFly1.wav", qfalse );
			if( !weaponInfo->missileSound )
			{
				// MFQ3: old quake3 sound (backup)
				weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
			}

			// FFAR trail with no dynamic lighting
			weaponInfo->missileTrailFunc = CG_MissileTrail2;
			weaponInfo->missileDlight = 0;
			
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			//MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

			// MFQ3: new sound
			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocketFire1.wav", qfalse );
			if( !weaponInfo->flashSound[0] )
			{
				// MFQ3: old quake3 sound (backup)
				weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			}
			break;

		case WT_IRONBOMB:
		case WT_GUIDEDBOMB:
			// find out which model to use
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			break;

		case WT_MACHINEGUN:
			// MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );

			// MFQ3: new sounds
			switch( i )
			{
			case WI_MG_2XCAL303:
			case WI_MG_2XCAL312:
			case WI_MG_8XCAL50:
			case WI_MG_6XCAL50:
				// olden sound
				weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machineGun2.wav", qfalse );
				break;

			case WI_MG_12_7MM:
			case WI_MG_14_5MM:
				// modern sound (noisey)
				weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machineGun3.wav", qfalse );
				break;

			case WI_MG_20MM:
			default:
				// modern sound (silenced)
				weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machineGun1.wav", qfalse );
				break;
			}

			// don't bother with multiple bullet noises for now
			weaponInfo->flashSound[1] = weaponInfo->flashSound[0];
			weaponInfo->flashSound[2] = weaponInfo->flashSound[0];
			weaponInfo->flashSound[3] = weaponInfo->flashSound[0];

			// MFQ3: old quake3 sounds (backup)
			if( !weaponInfo->flashSound[0] )
			{
				weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf1b.wav", qfalse );
				weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf2b.wav", qfalse );
				weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf3b.wav", qfalse );
				weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf4b.wav", qfalse );
			}

			cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
			break;

		case WT_BALLISTICGUN:
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );

			// no dynamic lighting
			weaponInfo->missileDlight = 0;
			
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			//MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			break;

		case WT_FLARE:
			switch(i){
				case WI_CFLARE:
				case WI_FLARE:
				
					weaponInfo->missileTrailFunc = CG_FlareTrail;
					break;
				case WI_BURNINGMAN:
					weaponInfo->missileSound = trap_S_RegisterSound( "sound/misc/scream.wav", qfalse );
					break;
				default:
				break;
			}

			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileDlight = 100;
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			
			break;

		case WT_FUELTANK:
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileDlight = 0;
			break;		
		case WT_NUKEBOMB:
			// find out which model to use
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			break;
		case WT_NUKEMISSILE:
			switch(i)
			{
				default:
					{

						// find out which model to use
						weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
						weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
						weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
						weaponInfo->missileTrailFunc = CG_MissileTrail2;
					}
					break;
			}

			break;
		default:
			//MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			break;
		}
	}
}

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	itemInfo->icon = trap_R_RegisterShader( item->icon );

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_HEALTH ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/

/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int start = cg.weaponSelect;
	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}
	
	while( 1 ) {
		cg.weaponSelect++;
		if( cg.weaponSelect > WP_WEAPON6 ) cg.weaponSelect = WP_MACHINEGUN;
		if( cg.snap->ps.ammo[cg.weaponSelect+8] ) break;
		if( cg.weaponSelect == start ) break;
	}
}

/*
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int start = cg.weaponSelect;
	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}
	
	while( 1 ) {
		cg.weaponSelect--;
		if( cg.weaponSelect < WP_MACHINEGUN ) cg.weaponSelect = WP_WEAPON6;
		if( cg.snap->ps.ammo[cg.weaponSelect+8] ) break;
		if( cg.weaponSelect == start ) break;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CG_Argv( 1 ) );

	if ( num < WP_MACHINEGUN || num > WP_WEAPON6 ) {
		return;
	}

	if ( !cg.snap->ps.ammo[num+8] ) {
		return;		// don't have the weapon
	}

	cg.weaponSelect = num;
}




/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_MG event
================
*/
void CG_FireMachinegun( centity_t *cent, qboolean main )
{
	entityState_t * ent;
	weaponInfo_t * weap;
	int c = -1;

	// get the current entity state and weapon info
	ent = &cent->currentState;

	if( main ) {
		weap = &cg_weapons[ ent->weaponIndex ];
		cent->muzzleFlashWeapon = ent->weaponIndex;
	} else {
		int	clientNum;
	    clientNum = cent->currentState.clientNum;
		if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
			weap = &cg_weapons[ WI_MG_20MM ];// hacky
			cent->muzzleFlashWeapon = WI_MG_20MM;// very hacky
		} else {
			clientInfo_t * ci;
			ci = &cgs.clientinfo[ clientNum ];
			weap = &cg_weapons[availableVehicles[ci->vehicle].weapons[0]];
			cent->muzzleFlashWeapon = availableVehicles[ci->vehicle].weapons[0];
		}
	}

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// play a sound

	// find out how many sounds are available
	for ( c = 0 ; c < 4 ; c++ )
	{
		// NOT available?
		if ( !weap->flashSound[c] )
		{
			break;
		}
	}

	// got something to play?
	if ( c > 0 )
	{
		// random index of count
		c = rand() % c;

		// final check
		if ( weap->flashSound[c] )
		{
			// play
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}
}

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
	if ( ent->weaponIndex == WI_NONE ) {
		return;
	}

	if( availableWeapons[ent->weaponIndex].type == WT_FUELTANK ) {
		while( MF_findWeaponsOfType(ent->weaponIndex, &cg_loadouts[ent->number]) )
			MF_removeWeaponFromLoadout(ent->weaponIndex, &cg_loadouts[ent->number], 0, 0, 0 );
		return;
	}

	if( availableWeapons[ent->weaponIndex].type == WT_MACHINEGUN ) {
		CG_FireMachinegun(cent, qtrue);
		return;	
	}
	weap = &cg_weapons[ent->weaponIndex];


	// play a sound
	for ( c = 0 ; c < 4 ; c++ ) {
		if ( !weap->flashSound[c] ) {
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
		if ( weap->flashSound[c] )
		{
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}
	MF_removeWeaponFromLoadout(ent->weaponIndex, &cg_loadouts[ent->number], 0, 0, 0 );
}


/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall( int weaponIndex, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType ) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius;
	float			light;
	vec3_t			lightColor;
	localEntity_t	*le;
	int				r;
	qboolean		isSprite;
	int				duration;

	mark = 0;
	radius = 1;
	sfx = 0;
	mod = 0;
	shader = 0;
	light = 0;
	lightColor[0] = 1;
	lightColor[1] = 1;
	lightColor[2] = 0;

	// set defaults
	isSprite = qfalse;
	duration = 600;

	switch ( weaponIndex ) {
	default:
	case WI_FFAR:
	case WI_FFAR_SMALL:
	case WI_FFAR_LARGE:
	case WI_SIDEWINDER:
	case WI_SPARROW:
	case WI_AMRAAM:
	case WI_PHOENIX:
	case WI_STINGER:
	case WI_ATOLL:
	case WI_ARCHER:
	case WI_ALAMO:
	case WI_HELLFIRE:
	case WI_MAVERICK:
	case WI_HARM:
	case WI_AASAM:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 0.75;
		light = 200;
		isSprite = qtrue;
		duration = 600;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		break;

	case WI_MK82:
	case WI_MK82R:
	case WI_MK83:
	case WI_MK83R:
	case WI_MK84:
	case WI_GBU15:
	case WI_DURANDAL:
	case WI_ECM:
	case WI_LASE:
	case WI_CM:
	case WI_DROPTANK:
	case WI_DROPTANK_SMALL:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader[1];
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 2;
		light = 400;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		break;

	case WI_MG_2XCAL303:
	case WI_MG_2XCAL312:
	case WI_MG_8XCAL50:
	case WI_MG_6XCAL50:
	case WI_MG_12_7MM:
	case WI_MG_14_5MM:
	case WI_MG_20MM:
	case WI_MG_2X20MM:
	case WI_MGT_2X30MM:
	case WI_MGT_12_7MM:
		mod = cgs.media.bulletFlashModel;
		shader = cgs.media.bulletExplosionShader;
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 8;
		break;

	case WI_125MM_GUN:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader[0];
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 1;
		light = 200;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		break;
	case WI_CFLARE:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.grenadeExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 0.50;
		light = 200;
		isSprite = qtrue;
		duration = 600;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		break;
	case WI_FLARE:
	case WI_NB10MT:
	case WI_NB5MT:
	case WI_NB1MT:
	case WI_NM10MT:
	case WI_NM5MT:
	case WI_NM1MT:
		break;
	}

	//
	// explosion sound fx
	//
	if( sfx )
	{
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	// MFQ3: disable dynamic lighting? (it makes MF looks much too quakey)
	light = 0;

	//
	// create the explosion
	//
	if( mod )
	{
		le = CG_MakeExplosion( origin, dir, 
							   mod,
							   shader,
							   0, duration,
							   isSprite );

		// dynamic light the explosion
		le->light = light;
		VectorCopy( lightColor, le->lightColor );

		// create radius override
		le->radius = radius;
	}

	//
	// impact mark
	//
	CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, qfalse, radius, qfalse );
}

/*
=================
CG_GenericExplosion
=================
*/
void CG_GenericExplosion( vec3_t origin, int type )
{
	localEntity_t	* le = NULL;
	vec3_t			up = {0, 0, 1};
	vec3_t			adjust = {0, 0, 0};
	int				soundIdx = -1;
	int				i, m, offset = 0;
	int				density = 0;
	float			radiusModifier = 1.0f;
	float			maxAdjust = 16;
	
	// * DENSITY *

	// adjust density based on cg_fxQuality setting
	switch( cg_fxQuality.integer )
	{
		case LOW_QUALITY:
			// reduced fx density
			density = 1;
			break;

		default:
		case MEDIUM_QUALITY:
		case HIGH_QUALITY:

			// normal fx density
			density = 3;
			break;
	}

	// building explosion?
	if( type == 2 && density > 1 )
	{
		// x5 size explosions for buildings
		radiusModifier = 5.0f;

		// and more density and area
		density *= 2;
		maxAdjust *= 2;
	}

	// * EXPLOSION SPRITE *

	// explosion effect for all types
	for( i = 0; i<density; i++ )
	{
		VectorCopy( origin, adjust );

		// adjust origin
		if( density > 1 )
		{
			adjust[0] = origin[0] + (random()*maxAdjust)-(maxAdjust/2);
			adjust[1] = origin[1] + (random()*maxAdjust)-(maxAdjust/2);
		}

		// make explosion
		le = CG_MakeExplosion( adjust, up, 
							   cgs.media.dishFlashModel,	
							   cgs.media.rocketExplosionShader[ rand() & 0x01 ],
							   offset, 900, 
							   qtrue );

		// apply radius modifier
		le->radius = radiusModifier;

		// adjust timing offset
		offset += 250;

	// MFQ3: disable dynamic lighting? (it makes MF looks much too quakey)
//	le->light = 300;
	}

	// * SMOKE SPRITE *

	VectorSet( le->lightColor, 1, 0.75, 0 );

	// and also smoke for gib/building explosion
	if( type > 0 )
	{
		localEntity_t * smoke = NULL;

		for( i = 1; i<=density; i++ )
		{
			VectorCopy( origin, adjust );

			// adjust origin
			if( density > 1 )
			{
				adjust[0] = origin[0] + (random()*maxAdjust)-(maxAdjust/2);
				adjust[1] = origin[1] + (random()*maxAdjust)-(maxAdjust/2);
			}

			// if building, bigger smoke clouds
			if( type == 2 )
			{
				// bigger clouds
				m = i;
			}
			else
			{
				// all smoke is the same scale
				m = 1;
			}

			// make smoke
			smoke = CG_SmokePuff( adjust, up, 75 * m, 0.8f, 0.8f, 0.8f, 1.00f, (i * 2000), cg.time,
								  0, 0, 
								  cgs.media.smokePuffShader );
		}
	}

	// * SOUND FX *

	// play 1 of the x explosion sounds
	soundIdx = rand() % NUM_EXPLOSION_SOUNDS;
	trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.planeDeath[ soundIdx ] );

	// if building, play more sound
	if( type == 2 )
	{
		// create point adjusted from building origin
		adjust[0] = origin[0] + (random()*maxAdjust)-(maxAdjust/2);
		adjust[1] = origin[1] + (random()*maxAdjust)-(maxAdjust/2);

		// choose a different sound
		soundIdx++;
		if( soundIdx >= NUM_EXPLOSION_SOUNDS )
		{
			soundIdx = 0;
		}

		// play additional sound
		trap_S_StartSound( adjust, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.planeDeath[ soundIdx ] );
	}

}

/*
=================
CG_VehicleHit
=================
*/
void CG_VehicleHit( vec3_t origin, int damage )
{
	vec3_t			up = {0, 0, 1};
	localEntity_t	*smoke = NULL;

	// make smoke
	smoke = CG_SmokePuff( origin, up, 
				  damage, 
				  0.8f, 0.8f, 0.8f, 1.00f,
				  1000, 
				  cg.time, 0,
				  0, 
				  cgs.media.smokePuffShader );

	smoke->leType = LE_MOVE_SCALE_FADE;
}

