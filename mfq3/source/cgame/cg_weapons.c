/*
 * $Id: cg_weapons.c,v 1.2 2001-12-22 02:28:43 thebjoern Exp $
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
static void CG_FFARTrail( centity_t *ent, const weaponInfo_t *wi ) {
	localEntity_t	*smoke;
	vec3_t			up = {0, 0, 1};
	vec3_t			pos, velocity;


	AngleVectors( ent->lerpAngles, velocity, NULL, NULL );
	VectorCopy( ent->lerpOrigin, pos );

	smoke = CG_SmokePuff( pos, up, 
				  8, 
				  0.5, 0.5, 0.5, 0.66f,
				  400, 
				  cg.time, 0,
				  LEF_PUFF_DONT_SCALE, 
				  cgs.media.smokePuffShader );	

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

	for( i = 0; i < WI_MAX; i++ ) {
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

		switch( availableWeapons[i].type ) {

		case WT_ROCKET:
		case WT_ANTIAIRMISSILE:
		case WT_ANTIGROUNDMISSILE:
		case WT_ANTIRADARMISSILE:
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
			weaponInfo->missileTrailFunc = CG_FFARTrail;
			weaponInfo->missileDlight = 200;
			
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
			break;

		case WT_IRONBOMB:
		case WT_GUIDEDBOMB:
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );

			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
			break;

		case WT_MACHINEGUN:
	//		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf1b.wav", qfalse );
			weaponInfo->flashSound[1] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf2b.wav", qfalse );
			weaponInfo->flashSound[2] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf3b.wav", qfalse );
			weaponInfo->flashSound[3] = trap_S_RegisterSound( "sound/weapons/machinegun/machgf4b.wav", qfalse );
			cgs.media.bulletExplosionShader = trap_R_RegisterShader( "bulletExplosion" );
			break;


		case WT_BALLISTICGUN:
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rockfly.wav", qfalse );
			weaponInfo->missileDlight = 200;
			
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

			weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
			cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
			break;

		case WT_FLARE:
			weaponInfo->missileModel = trap_R_RegisterModel( availableWeapons[i].modelName );
			break;

		 default:
			MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
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
void CG_FireMachinegun( centity_t *cent ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
	weap = &cg_weapons[WI_MG_20MM];

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

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
	radius = 32;
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
	case WI_SIDEWINDER:
	case WI_SPARROW:
	case WI_AMRAAM:
	case WI_PHOENIX:
	case WI_STINGER:
	case WI_ATOLL:
	case WI_ARCHER:
	case WI_ALAMO:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 50;
		light = 200;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		break;
	case WI_MK82:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 150;
		light = 400;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		break;
	case WI_MG_8XCAL50:
	case WI_MG_6XCAL50:
	case WI_MG_12_7MM:
	case WI_MG_14_5MM:
	case WI_MG_20MM:
	case WI_MG_2X20MM:
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
		shader = cgs.media.rocketExplosionShader;
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 50;
		light = 200;
		isSprite = qtrue;
		duration = 1000;
		lightColor[0] = 1;
		lightColor[1] = 0.75;
		lightColor[2] = 0.0;
		break;
	}

	if ( sfx ) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//
	// create the explosion
	//
	if ( mod ) {
		le = CG_MakeExplosion( origin, dir, 
							   mod,	shader,
							   duration, isSprite );
		le->light = light;
		VectorCopy( lightColor, le->lightColor );
	}

	//
	// impact mark
	//
	CG_ImpactMark( mark, origin, dir, random()*360, 1,1,1,1, qfalse, radius, qfalse );
}

/*
=================
CG_VehicleExplosion
=================
*/
void CG_VehicleExplosion( vec3_t origin, int type ) {
	localEntity_t	*le;
	vec3_t			up = {0, 0, 1};

	// explosion effect for both
	le = CG_MakeExplosion( origin, 
						   up, 
						   cgs.media.dishFlashModel,	
						   cgs.media.rocketExplosionShader,
						   700, 
						   qtrue );
	le->light = 300;
	VectorSet( le->lightColor, 1, 0.75, 0 );

	// and also smoke for gib explosion
	if( type == 1 ) {
		localEntity_t	*smoke;

		smoke = CG_SmokePuff( origin, up, 
					  50, 
					  0.8f, 0.8f, 0.8f, 1.00f,
					  3000, 
					  cg.time, 0,
					  0, 
					  cgs.media.smokePuffShader );
		smoke->leType = LE_MOVE_SCALE_FADE;
	}
	trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.planeDeath );
}

/*
=================
CG_VehicleHit
=================
*/
void CG_VehicleHit( vec3_t origin, int damage ) {
	vec3_t			up = {0, 0, 1};
	localEntity_t	*smoke;

	smoke = CG_SmokePuff( origin, up, 
				  damage, 
				  0.8f, 0.8f, 0.8f, 1.00f,
				  1000, 
				  cg.time, 0,
				  0, 
				  cgs.media.smokePuffShader );
	smoke->leType = LE_MOVE_SCALE_FADE;
}

