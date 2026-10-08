/*
 * $Id: cg_weapons.c,v 1.10 2005-11-21 17:28:20 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"




/*
==========================
CGame::fFARTrail
==========================
*/
void CGame::fFARTrail( ClientEntity * cent, const weaponInfo_t *wi )
{
	LocalEntity	* smoke;
	Vec3			up, pos, smoke_pos, velocity;
	int smoke_rad;
	static int	seed = 0x92;
	
	up[0] = 5 * Q_random(&seed);
	up[1] = 5 * Q_random(&seed);
	up[2] = (5 - 1) * Q_random(&seed) + 1;

	pos = cent->lerpOrigin;

	// draw smoke slightly behind the entitiy position (using -velocity of the entity)
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	VectorNormalize( velocity );

	// calc adjusted position
	velocity = velocity * ( -24.0f );		// -24.0f is just an arbitary distance which works OK with all current rocket models
	pos = pos + velocity;

	smoke_pos = pos;

	smoke_pos[0] += + Q_random( &seed ) * 2;
	smoke_pos[1] += + Q_random( &seed ) * 2;
	smoke_pos[2] += + Q_random( &seed ) * 2;
	smoke_rad = 1 + Q_random( &seed ) * 3;

	smoke = CGame::smokePuff( smoke_pos, up, 
						  smoke_rad, 
						  1.0f, 1.0f, 1.0f, 1.0f,
						  10500, 
						  cg.time, 3000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.missilePuffShader );
}


/*
==========================
CGame::missileTrail
==========================
*/
void CGame::missileTrail( ClientEntity * cent, const weaponInfo_t *wi )
{
	LocalEntity	* smoke;
	Vec3			up, pos, smoke_pos, velocity;
	int smoke_rad;
	static float smoke_rot = 0;
	static int	seed = 0x92;
	
	up[0] = 5 * Q_random(&seed);
	up[1] = 5 * Q_random(&seed);
	up[2] = (5 - 1) * Q_random(&seed) + 1;

	pos = cent->lerpOrigin;
	
	// draw smoke slightly behind the entitiy position (using -velocity of the entity)
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	VectorNormalize( velocity );

	// calc adjusted position
	velocity = velocity * ( -24.0f );		// -24.0f is just an arbitary distance which works OK with all current rocket models
	pos = pos + velocity;

	// draw trail
/*	smoke = CGame::smokePuff( pos, up, 
						  4, 
						  0.8f, 0.8f, 0.8f, 0.33f,
						  400, 
						  cg.time, 0,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );	*/

	smoke_pos = pos;
	if(smoke_rot > 360)
	{
		smoke_rot = 0;
	}

	smoke_pos[0] += sin(smoke_rot) * 5 + Q_random( &seed ) * 5;
	smoke_pos[1] += sin(smoke_rot) * 5 + Q_random( &seed ) * 5;
	smoke_pos[2] += cos(smoke_rot) * 5 + Q_random( &seed ) * 5;
	smoke_rot += 1;
	smoke_rad = 4 + Q_random( &seed ) * 5;

	smoke = CGame::smokePuff( smoke_pos, up, 
						  smoke_rad, 
						  1.0f, 1.0f, 1.0f, 1.0f,
						  10500, 
						  cg.time, 3000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.missilePuffShader );
}

/*
==========================
CGame::flareTrail
==========================
*/
void CGame::flareTrail( ClientEntity * cent, const weaponInfo_t *wi )
{
	LocalEntity	* smoke;
	Vec3			up, pos, smoke_pos, velocity;
	int smoke_rad;
	static int	seed = 0x92;
	
	up[0] = 5 * Q_random(&seed);
	up[1] = 5 * Q_random(&seed);
	up[2] = (5 - 1) * Q_random(&seed) + 1;
	
	pos = cent->lerpOrigin;

	// draw smoke slightly behind the entitiy position (using -velocity of the entity)
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	VectorNormalize( velocity );

	// calc adjusted position
	velocity = velocity * ( -24.0f );		// -24.0f is just an arbitary distance which works OK with all current rocket models
	pos = pos + velocity;

	smoke_pos = pos;

	smoke_pos[0] += Q_random( &seed ) * 5;
	smoke_pos[1] += Q_random( &seed ) * 5;
	smoke_pos[2] += Q_random( &seed ) * 5;
	smoke_rad = 1 + Q_random( &seed ) * 5;
	smoke = CGame::smokePuff( smoke_pos, up, 
						  smoke_rad, 
						  0.8f, 0.8f, 0.8f, 0.33f,
						  4000, 
						  cg.time, 3000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.flarePuffShader );
}


void CGame::missileTrail2( ClientEntity * cent, const weaponInfo_t *wi ) {
	Vec3  start, end;
	LocalEntity *le;
	refEntity_t   *re;

	start = cent->lastDrawnTrailPos;
	end = cent->lerpOrigin;


	// Only draw every so often dispite how often a trail is said to be updated
	// Or you may get a SHADER_MAX_VERTEXES error eith RT_RAIL_CORE draw method

	if(cent->TimeSinceLastTrail <= cg.time - 100)
	{

		// If the trail seems a bit to old, give it a starting origin, and skip this round
		if(cent->TimeSinceLastTrail <= cg.time - 200)
		{
			cent->TimeSinceLastTrail = cg.time;
			cent->lastDrawnTrailPos = cent->lerpOrigin;
			return;
		}
	
		le = CGame::allocLocalEntity();
		le->leFlags = LEF_PUFF_DONT_SCALE;
		re = &le->refEntity;

		le->leType = LE_FADE_RGB;
		le->startTime = cg.time;
		le->endTime = cg.time + 5000;
		le->fadeInTime = 2000;
		le->lifeRate = 1.0 / (le->endTime - le->startTime);
 
		re->shaderTime = cg.time / 1000.0f;
		re->reType = RT_LIGHTNING;
		re->customShader = cgs.media.railCoreShader;
	//	re->customShader = cgs.media.missileTrail2Shader;

		VectorCopy(start, re->origin);
		VectorCopy(start, le->pos.trBase);
		VectorClear(le->pos.trDelta);
		VectorCopy(end, re->oldorigin);

		re->shaderRGBA[0] = 0.3f * 255;
		re->shaderRGBA[1] = 0.3f * 255;
		re->shaderRGBA[2] = 0.3f * 255;
		re->shaderRGBA[3] = 0.3f * 255;

		le->color[0] = 1.0f;
		le->color[1] = 1.0f;
		le->color[2] = 1.0f;
		le->color[3] = 1.0f;

		// Update last time & Pos
		cent->TimeSinceLastTrail = cg.time;
		cent->lastDrawnTrailPos = cent->lerpOrigin;
	}


}


/*
==========================
CG_NukeTrail
==========================
*/
//static void CG_NukeTrail( ClientEntity * cent, const weaponInfo_t *wi )
//{
//	if(cent->TimeSinceLastTrail <= cg.time - 200)
//	{
//		LocalEntity	* smoke;
//		vec3_t			up, pos, velocity;
//	
//		VectorCopy( cent->lerpOrigin, pos );
//	
//		// draw smoke slightly behind the entitiy position (using -velocity of the entity)
//		VectorCopy( cent->currentState.pos.trDelta, velocity );
//		VectorNormalize( velocity );
//
//		// calc adjusted position
//		VectorScale( velocity, -24.0f, velocity );		// -24.0f is just an arbitary distance which works OK with all current rocket models
//		VectorAdd( pos, velocity, pos );
//
//		// draw trail
//
//		smoke = CGame::smokePuff( pos, up, 
//							20, 
//							0.0f, 0.0f, 0.0f, 0.8f,
//							8000, 
//							cg.time, 6000,
//							LEF_PUFF_DONT_SCALE, 
//							cgs.media.nukePuffShader );	
//		cent->TimeSinceLastTrail = cg.time;
//	}
//}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CGame::registerWeapons() {
	weaponInfo_t	*weaponInfo;
	gitem_t			*ammo;
	int				i;
	
	// common
	cgs.media.grenadeExplosionShader = refExport.RegisterShader( "grenadeExplosion" );
	cgs.media.rocketExplosionShader[0] = refExport.RegisterShader( "rocketExplosion" );
	cgs.media.rocketExplosionShader[1] = refExport.RegisterShader( "quakeRocketExplosion" );

	// for all weapon indexes in cg_weapons[]
	for( i = 0; i < WI_MAX; i++ )
	{
		CGame::loadingStep( i, WI_MAX );
		weaponInfo = &cg_weapons[i];
		memset( weaponInfo, 0, sizeof( *weaponInfo ) );
		weaponInfo->registered = true;

		for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
			if ( ammo->giType == IT_AMMO && ammo->giTag == availableWeapons[i].type ) {
				break;
			}
		}
		if ( ammo->classname && ammo->world_model[0] ) {
			weaponInfo->ammoModel = refExport.RegisterModel( ammo->world_model[0] );
		}

		// switch on weapon type
		switch( availableWeapons[i].type )
		{
		case WT_ROCKET:
			// find out which model to use
			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			
			// MFQ3: New sound
			weaponInfo->missileSound = SoundSystem::registerSound( "sound/weapons/rocket/rocketFly1.wav", false );
			if( !weaponInfo->missileSound )
			{
				// MFQ3: old quake3 sound (backup)
				weaponInfo->missileSound = SoundSystem::registerSound( "sound/weapons/rocket/rockfly.wav", false );
			}

			// FFAR trail with no dynamic lighting
			weaponInfo->missileTrailFunc = CGame::fFARTrail;
			//weaponInfo->missileTrailFunc = CGame::missileTrail2;
			weaponInfo->missileDlight = 0;
			
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			//MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

			// MFQ3: New sound
			weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/rocket/rocketFire1.wav", false );

			break;
		case WT_ANTIAIRMISSILE:
		case WT_ANTIGROUNDMISSILE:
		case WT_ANTIRADARMISSILE:
			// find out which model to use
			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			
			// MFQ3: New sound
			weaponInfo->missileSound = SoundSystem::registerSound( "sound/weapons/rocket/rocketFly1.wav", false );


			// FFAR trail with no dynamic lighting
			weaponInfo->missileTrailFunc = CGame::missileTrail2;
			weaponInfo->missileDlight = 0;
			
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			//MAKERGB( weaponInfo->flashDlightColor, 1, 0.75f, 0 );

			// MFQ3: New sound
			weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/rocket/rocketFire1.wav", false );

			break;

		case WT_IRONBOMB:
		case WT_GUIDEDBOMB:
			// find out which model to use
			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = SoundSystem::registerSound( "sound/weapons/rocket/rockfly.wav", false );
			weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/rocket/rocklf1a.wav", false );
			break;

		case WT_MACHINEGUN:
			// MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );

			// MFQ3: New sounds
			switch( i )
			{
			case WI_MG_2XCAL303:
			case WI_MG_2XCAL312:
			case WI_MG_8XCAL50:
			case WI_MG_6XCAL50:
				// olden sound
				weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/machinegun/machineGun2.wav", false );
				break;

			case WI_MG_12_7MM:
			case WI_MG_14_5MM:
				// modern sound (noisey)
				weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/machinegun/machineGun3.wav", false );
				break;

			case WI_MGT_7_62MM:
				weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/machinegun/minigun.wav", false );
				break;
			
			case WI_MG_20MM:
			case WI_MG_M4A1:
			default:
				// modern sound (silenced)
				weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/machinegun/machineGun1.wav", false );
				break;
			}

			// don't bother with multiple bullet noises for now
			weaponInfo->flashSound[1] = weaponInfo->flashSound[0];
			weaponInfo->flashSound[2] = weaponInfo->flashSound[0];
			weaponInfo->flashSound[3] = weaponInfo->flashSound[0];

			// MFQ3: old quake3 sounds (backup)

			cgs.media.bulletExplosionShader = refExport.RegisterShader( "bulletExplosion" );
			break;

		case WT_BALLISTICGUN:
			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = SoundSystem::registerSound( "sound/weapons/rocket/rockfly.wav", false );

			// no dynamic lighting
			weaponInfo->missileDlight = 0;
			
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );

			weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/rocket/rocklf1a.wav", false );
			break;

		case WT_FLARE:
			switch(i){
				case WI_CFLARE:
				case WI_FLARE:
				
					weaponInfo->missileTrailFunc = CGame::flareTrail;
					break;
				case WI_BURNINGMAN:
					weaponInfo->missileSound = SoundSystem::registerSound( "sound/misc/scream.wav", false );
					break;
				default:
				break;
			}

			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileDlight = 100;
			MAKERGB( weaponInfo->missileDlightColor, 1, 0.75f, 0 );
			
			break;

		case WT_FUELTANK:
			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileDlight = 0;
			break;		

		case WT_FLAK:
			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileDlight = 0;
			break;

		case WT_NUKEBOMB:
			// find out which model to use
			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			weaponInfo->missileSound = SoundSystem::registerSound( "sound/weapons/rocket/rockfly.wav", false );
			weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/rocket/rocklf1a.wav", false );
			break;
		case WT_NUKEMISSILE:
			//switch(i)
			//{
			//	default:
			//		{

						// find out which model to use
						weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
						weaponInfo->missileSound = SoundSystem::registerSound( "sound/weapons/rocket/rockfly.wav", false );
						weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/rocket/rocklf1a.wav", false );
						weaponInfo->missileTrailFunc = CGame::missileTrail;
			//		}
			//		break;
			//}

			break;

		// Pickup creates
		case WT_FUELCRATE:
		case WT_AMMOCRATE:
		case WT_HEALTHCRATE:
			weaponInfo->missileModel = refExport.RegisterModel( availableWeapons[i].modelName );
			weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/rocket/rocklf1a.wav", false );
			break;

		default:
			//MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
			weaponInfo->flashSound[0] = SoundSystem::registerSound( "sound/weapons/rocket/rocklf1a.wav", false );
			break;
		}
	}
}

/*
=================
CGame::registerItemVisuals

The server says this item is used on this level
=================
*/
void CGame::registerItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( &itemInfo ) );
	itemInfo->registered = true;

	itemInfo->models[0] = refExport.RegisterModel( item->world_model[0] );

	itemInfo->icon = refExport.RegisterShader( item->icon );

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_HEALTH ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = refExport.RegisterModel( item->world_model[1] );
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
CGame::nextWeapon_f
===============
*/
void CGame::nextWeapon_f( ) {
	int start = cg.weaponSelect;
	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}
	
	while( true ) {
		cg.weaponSelect++;
		if( cg.weaponSelect > WP_WEAPON6 ) cg.weaponSelect = WP_MACHINEGUN;
		if( cg.snap->ps.ammo[cg.weaponSelect+8] ) break;
		if( cg.weaponSelect == start ) break;
	}
}

/*
===============
CGame::prevWeapon_f
===============
*/
void CGame::prevWeapon_f( ) {
	int start = cg.weaponSelect;
	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}
	
	while( true ) {
		cg.weaponSelect--;
		if( cg.weaponSelect < WP_MACHINEGUN ) cg.weaponSelect = WP_WEAPON6;
		if( cg.snap->ps.ammo[cg.weaponSelect+8] ) break;
		if( cg.weaponSelect == start ) break;
	}
}

/*
===============
CGame::weapon_f
===============
*/
void CGame::weapon_f( ) {
	int		num;

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW ) {
		return;
	}

	num = atoi( CGame::argv( 1 ) );

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

// one spent casing thrown clear of a gun: it keeps the vehicle's speed, tumbles and falls
static void launchCasing( const Vec3& origin, const Vec3& velocity )
{
	LocalEntity* le = CGame::allocLocalEntity();
	refEntity_t* re = &le->refEntity;
	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + cg_casingTime.integer * 1000;
	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = cgs.media.casingModel;
	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	VectorSet( le->angles.trBase, random() * 360.0f, random() * 360.0f, random() * 360.0f );
	VectorSet( le->angles.trDelta, crandom() * 600.0f, crandom() * 600.0f, crandom() * 300.0f );
	le->leFlags = LEF_TUMBLE;
	le->bounceFactor = 0.3f;
	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

// casings leave each gun that fired: the Huey's door guns that bear, a bomber's turrets, or the vehicle's gun
void ClientEntity::ejectCasings( bool main, int weaponIndex )
{
	// how fast a casing is thrown sideways and upwards
	constexpr float throwSide = 45.0f, throwUp = 25.0f;

	const int clientNum = this->currentState.clientNum;
	if( cg_casingTime.integer <= 0 || !cgs.media.casingModel || clientNum < 0 || clientNum >= MAX_CLIENTS )
		return;
	const int vehicle = cgs.clientinfo[clientNum].vehicle;
	if( vehicle < 0 || vehicle >= bg_numberOfVehicles )
		return;
	const completeVehicleData_t& veh = availableVehicles[vehicle];

	Vec3 forward, right, up, base, speed;
	AngleVectors( this->lerpAngles, forward, right, up );
	VectorCopy( this->lerpOrigin, base );
	VectorCopy( this->currentState.pos.trDelta, speed );

	const auto launch = [&]( float f, float r, float u, float side ) {
		const Vec3 origin = base + forward * f + right * r + up * u;
		const Vec3 velocity = speed + right * ( side * throwSide * ( 0.7f + random() * 0.6f ) ) + up * ( throwUp * ( 0.5f + random() ) ) + forward * ( crandom() * 10.0f );
		launchCasing( origin, velocity );
	};

	if( main && (veh.flags & VF_DOORGUNS) && availableWeapons[weaponIndex].turret )
	{
		float aim = this->lerpAngles2[ROLL];
		if( aim > 180 ) aim -= 360;
		if( doorGunBears( false, aim ) ) launch( DOORGUN_FORWARD, DOORGUN_SIDE, DOORGUN_UP, 1.0f );
		if( doorGunBears( true, aim ) ) launch( DOORGUN_FORWARD, -DOORGUN_SIDE, DOORGUN_UP, -1.0f );
		return;
	}
	if( !main && (veh.flags & VF_GUNSTATIONS) )
	{
		// only the crew's own client knows where they look; for other aircraft the look is taken as straight ahead
		vec3_t aimAngles;
		VectorCopy( clientNum == cg.snap->ps.clientNum ? cg.snap->ps.viewangles : this->lerpAngles, aimAngles );
		const unsigned bearing = VehicleData::gunStationsBearing( vehicle, this->lerpAngles, aimAngles );
		unsigned bit = 1;
		for( const GunStation& station : VehicleData::gunStations( vehicle ) )
		{
			if( bearing & bit ) launch( station.forward, station.right, station.up, crandom() > 0 ? 1.0f : -1.0f );
			bit <<= 1;
		}
		return;
	}
	launch( veh.gunoffset[0], veh.gunoffset[1], veh.gunoffset[2], veh.gunoffset[1] < 0 ? -1.0f : 1.0f );
}

/*
================
CG_FireWeapon

Caused by an EV_FIRE_MG event
================
*/
void ClientEntity::fireMachinegun( bool main )
{
	entityState_t * ent;
	weaponInfo_t * weap;
	int c = -1;

	// get the current entity state and weapon info
	ent = &this->currentState;

	if( main ) {
		weap = &cg_weapons[ ent->weaponIndex ];
		this->muzzleFlashWeapon = ent->weaponIndex;
	} else {
		int	clientNum;
	    clientNum = this->currentState.clientNum;
		if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
			weap = &cg_weapons[ WI_MG_20MM ];// hacky
			this->muzzleFlashWeapon = WI_MG_20MM;// very hacky
		} else {
			clientInfo_t * ci;
			ci = &cgs.clientinfo[ clientNum ];
			weap = &cg_weapons[availableVehicles[ci->vehicle].weapons[0]];
			this->muzzleFlashWeapon = availableVehicles[ci->vehicle].weapons[0];
		}
	}

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	this->muzzleFlashTime = cg.time;

	this->ejectCasings( main, this->muzzleFlashWeapon );

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
			SoundSystem::startSound( nullptr, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}
}

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void ClientEntity::fireWeapon() {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &this->currentState;
	if ( ent->weaponIndex == WI_NONE ) {
		return;
	}

	completeLoadout_t* loadout = this->entityLoadout();
	if( loadout && availableWeapons[ent->weaponIndex].type == WT_FUELTANK ) {
		while( VehicleData::findWeaponsOfType( ent->weaponIndex, loadout ) )
			VehicleData::removeWeaponFromLoadout( ent->weaponIndex, loadout, nullptr, nullptr, nullptr, 0 );
		return;
	}

	if( availableWeapons[ent->weaponIndex].type == WT_MACHINEGUN ) {
		this->fireMachinegun( true );
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
			SoundSystem::startSound( nullptr, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}
	if( loadout )
		VehicleData::removeWeaponFromLoadout( ent->weaponIndex, loadout, nullptr, nullptr, nullptr, 0 );
}


/*
=================
CGame::missileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CGame::missileHitWall( int weaponIndex, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType ) {
	qhandle_t		mod;
	qhandle_t		mark;
	qhandle_t		shader;
	sfxHandle_t		sfx;
	float			radius;
	float			light;
	Vec3			lightColor;
	LocalEntity	*le;
	int				r;
	bool		isSprite;
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
	isSprite = false;
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
		isSprite = true;
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
		isSprite = true;
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
	case WI_MG_M4A1:
	case WI_MG_4X14_5MM:
	case WI_MG_20MM:
	case WI_MG_2X20MM:
	case WI_MGT_7_62MM:
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

		radius = 0.2f; //radius = 8;
		duration = 300;
		break;

	case WI_125MM_GUN:
		mod = cgs.media.dishFlashModel;
		shader = cgs.media.rocketExplosionShader[0];
		sfx = cgs.media.sfx_rockexp;
		mark = cgs.media.burnMarkShader;
		radius = 1;
		light = 200;
		isSprite = true;
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
		isSprite = true;
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
		SoundSystem::startSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	// MFQ3: disable dynamic lighting? (it makes MF looks much too quakey)
	light = 0;

	//
	// create the explosion
	//
	if( mod )
	{
		le = CGame::makeExplosion( origin, dir, 
							   mod,
							   shader,
							   0, duration,
							   isSprite );

		// dynamic light the explosion
		le->light = light;
		le->lightColor = lightColor;

		// create radius override
		le->radius = radius;
	}

	//
	// impact mark
	//
	CGame::impactMark( mark, origin, dir, random()*360, 1,1,1,1, false, radius, false );
}

/*
=================
CGame::genericExplosion
=================
*/
void CGame::cookOff( vec3_t origin, int vehicle )
{
	Vec3 up = { 0, 0, 1 };

	if( vehicle < 0 || vehicle >= bg_numberOfVehicles )
		return;

	for( int i = 0; i < 14; i++ )
	{
		Vec3 point;
		VectorCopy( origin, point );
		point[0] += crandom() * (1.0f + i * 0.5f);
		point[1] += crandom() * (1.0f + i * 0.5f);
		point[2] += 4.0f + i * 7.0f;
		LocalEntity* flame = CGame::makeExplosion( point, up, cgs.media.dishFlashModel, cgs.media.rocketExplosionShader[ i & 1 ],
			i * 160, 700 + i * 60, true );
		flame->radius = 0.35f + i * 0.09f;
	}

	const qhandle_t turret = availableVehicles[vehicle].handle[BP_GV_TURRET];
	if( !turret )
		return;

	LocalEntity* le = CGame::allocLocalEntity();
	refEntity_t* re = &le->refEntity;
	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + std::max( 9, cg_debrisTime.integer ) * 1000;
	VectorCopy( origin, re->origin );
	re->origin[2] += 6;
	AxisCopy( axisDefault, re->axis );
	re->hModel = turret;
	re->customShader = cgs.media.charredShader;
	le->pos.trType = TR_GRAVITY;
	VectorCopy( re->origin, le->pos.trBase );
	VectorSet( le->pos.trDelta, crandom() * 70.0f, crandom() * 70.0f, 520.0f + random() * 180.0f );
	le->pos.trTime = cg.time;
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	VectorSet( le->angles.trDelta, 200.0f + random() * 200.0f, crandom() * 120.0f, crandom() * 90.0f );
	le->leFlags = LEF_TUMBLE;
	le->bounceFactor = 0.25f;
	le->leBounceSoundType = LEBS_NONE;
	le->leMarkType = LEMT_BURN;
}

void CGame::genericExplosion( vec3_t origin, int type )
{
	LocalEntity	* le = nullptr;
	Vec3			up = {0, 0, 1};
	Vec3			adjust = {0, 0, 0};
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
		le = CGame::makeExplosion( adjust, up, 
							   cgs.media.dishFlashModel,	
							   cgs.media.rocketExplosionShader[ rand() & 0x01 ],
							   offset, 900, 
							   true );

		// apply radius modifier
		le->radius = radiusModifier;

		// adjust timing offset
		offset += 250;

	// MFQ3: disable dynamic lighting? (it makes MF looks much too quakey)
//	le->light = 300;
	}

	// * SMOKE SPRITE *

	le->lightColor = Vec3( 1, 0.75, 0 );

	// and also smoke for gib/building explosion
	if( type > 0 )
	{
		LocalEntity * smoke = nullptr;

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
			smoke = CGame::smokePuff( adjust, up, 75 * m, 0.8f, 0.8f, 0.8f, 1.00f, (i * 2000), cg.time,
								  0, LEF_NONE, 
								  cgs.media.smokePuffShader );
		}
	}

	// * SOUND FX *

	// play 1 of the x explosion sounds
	soundIdx = rand() % NUM_EXPLOSION_SOUNDS;
	SoundSystem::startSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.planeDeath[ soundIdx ] );

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
		SoundSystem::startSound( adjust, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.planeDeath[ soundIdx ] );
	}

}

/*
=================
CGame::vehicleHit
=================
*/
void CGame::vehicleHit( vec3_t origin, int damage )
{
	Vec3			up = {0, 0, 1};
	LocalEntity	*smoke = nullptr;

	// make smoke
	smoke = CGame::smokePuff( origin, up, 
				  damage, 
				  0.8f, 0.8f, 0.8f, 1.00f,
				  1000, 
				  cg.time, 0,
				  LEF_NONE, 
				  cgs.media.smokePuffShader );

	smoke->leType = LE_MOVE_SCALE_FADE;
}

