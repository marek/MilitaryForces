/*
 * $Id: cg_vehicle.c,v 1.24 2002-07-14 17:13:19 thebjoern Exp $
*/

#include "cg_local.h"




/*
===============
CG_Cache_Error

Wrapper for model errors, so that don't always have to prevent loading because
one (or more) model(s) may be missing.
===============
*/
void trap_Cache_Error( char * pString )
{
#ifdef _DEBUG
	// just notify in console when developing
	Com_Printf( pString );
#else
	// (release builds, VM builds, ...)
	trap_Error( pString );
#endif
}

static void CG_CachePlane(int index)
{
	char name[128];
	char basename[128];
	int i;

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
		availableVehicles[index].handle[i] = trap_R_RegisterModel( name );
//		if( !availableVehicles[index].handle[i] ) {
//			CG_Printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_PLANE_BODY] ) {
		trap_Cache_Error( va("MFQ3 Error: Invalid handle for body %s.md3\n", basename) );
	}
}

/*
===============
CG_CacheGroundVehicle

On startup cache it
===============
*/

static void CG_CacheGroundVehicle(int index)
{
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/ground/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );
// changed mg
	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_GV_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			break;
		case BP_GV_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_GV_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		case BP_GV_WHEEL:
			Com_sprintf( name, sizeof(name), "%s_w1.md3", basename );
			break;
		case BP_GV_WHEEL2:
			Com_sprintf( name, sizeof(name), "%s_w2.md3", basename );
			break;
		case BP_GV_WHEEL3:
			Com_sprintf( name, sizeof(name), "%s_w3.md3", basename );
			break;
		case BP_GV_WHEEL4:
			Com_sprintf( name, sizeof(name), "%s_w4.md3", basename );
			break;
		case BP_GV_WHEEL5:
			Com_sprintf( name, sizeof(name), "%s_w5.md3", basename );
			break;
		case BP_GV_WHEEL6:
			Com_sprintf( name, sizeof(name), "%s_w6.md3", basename );
			break;
		}
		availableVehicles[index].handle[i] = trap_R_RegisterModel( name );
//		if( !availableVehicles[index].handle[i] ) {
//			CG_Printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_GV_BODY] ) {
		trap_Cache_Error( va("MFQ3 Error: Invalid handle for body %s.md3\n", basename) );
	}
}


/*
===============
CG_CacheGroundVehicle

On startup cache it
===============
*/

static void CG_CacheHelo(int index)
{/*
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/ground/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );
// changed mg
	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_GV_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			break;
		case BP_GV_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_GV_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		case BP_GV_WHEEL:
			Com_sprintf( name, sizeof(name), "%s_w1.md3", basename );
			break;
		case BP_GV_WHEEL2:
			Com_sprintf( name, sizeof(name), "%s_w2.md3", basename );
			break;
		case BP_GV_WHEEL3:
			Com_sprintf( name, sizeof(name), "%s_w3.md3", basename );
			break;
		case BP_GV_WHEEL4:
			Com_sprintf( name, sizeof(name), "%s_w4.md3", basename );
			break;
		case BP_GV_WHEEL5:
			Com_sprintf( name, sizeof(name), "%s_w5.md3", basename );
			break;
		case BP_GV_WHEEL6:
			Com_sprintf( name, sizeof(name), "%s_w6.md3", basename );
			break;
		}
		availableVehicles[index].handle[i] = trap_R_RegisterModel( name );
//		if( !availableVehicles[index].handle[i] ) {
//			CG_Printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_PLANE_BODY] ) {
		trap_Cache_Error( va("MFQ3 Error: Invalid handle for body %s.md3\n", basename) );
	}*/
}


/*
===============
CG_CacheGroundVehicle

On startup cache it
===============
*/

static void CG_CacheLQM(int index)
{/*
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/ground/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );
// changed mg
	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_GV_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			break;
		case BP_GV_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_GV_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		case BP_GV_WHEEL:
			Com_sprintf( name, sizeof(name), "%s_w1.md3", basename );
			break;
		case BP_GV_WHEEL2:
			Com_sprintf( name, sizeof(name), "%s_w2.md3", basename );
			break;
		case BP_GV_WHEEL3:
			Com_sprintf( name, sizeof(name), "%s_w3.md3", basename );
			break;
		case BP_GV_WHEEL4:
			Com_sprintf( name, sizeof(name), "%s_w4.md3", basename );
			break;
		case BP_GV_WHEEL5:
			Com_sprintf( name, sizeof(name), "%s_w5.md3", basename );
			break;
		case BP_GV_WHEEL6:
			Com_sprintf( name, sizeof(name), "%s_w6.md3", basename );
			break;
		}
		availableVehicles[index].handle[i] = trap_R_RegisterModel( name );
//		if( !availableVehicles[index].handle[i] ) {
//			CG_Printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_PLANE_BODY] ) {
		trap_Cache_Error( va("MFQ3 Error: Invalid handle for body %s.md3\n", basename) );
	}*/
}


/*
===============
CG_CacheGroundVehicle

On startup cache it
===============
*/

static void CG_CacheBoat(int index)
{
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/sea/%s/%s", availableVehicles[index].modelName,
			availableVehicles[index].modelName );
// changed mg
	for( i = 0; i < BP_BOAT_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_BOAT_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			// temp
//			{
//				md3Tag_t tag;
//				if( MF_findTag(name, "tag_turret", &tag) ) {
//					CG_Printf( "found tag_turret in %s: %.1f %.1f %.1f\n", name,
//						tag.origin[0], tag.origin[1], tag.origin[2] );
//				}
//			}
			// end temp

			break;
		case BP_BOAT_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_BOAT_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		case BP_BOAT_TURRET2:
			Com_sprintf( name, sizeof(name), "%s_tur2.md3", basename );
			break;
		case BP_BOAT_GUNBARREL2:
			Com_sprintf( name, sizeof(name), "%s_gun2.md3", basename );
			break;
		case BP_BOAT_TURRET3:
			Com_sprintf( name, sizeof(name), "%s_tur3.md3", basename );
			break;
		case BP_BOAT_GUNBARREL3:
			Com_sprintf( name, sizeof(name), "%s_gun3.md3", basename );
			break;
		case BP_BOAT_TURRET4:
			Com_sprintf( name, sizeof(name), "%s_tur4.md3", basename );
			break;
		case BP_BOAT_GUNBARREL4:
			Com_sprintf( name, sizeof(name), "%s_gun4.md3", basename );
			break;
		}
		availableVehicles[index].handle[i] = trap_R_RegisterModel( name );

//		if( !availableVehicles[index].handle[i] ) {
//			CG_Printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableVehicles[index].handle[BP_BOAT_BODY] ) {
		trap_Cache_Error( va("MFQ3 Error: Invalid handle for body %s.md3\n", basename) );
	}
}

/*
===============
CG_CacheGroundInstallation

On startup cache it
===============
*/

static void CG_CacheGroundInstallation(int index)
{
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/npc/%s/%s", 
				 availableGroundInstallations[index].modelName,
				 availableGroundInstallations[index].modelName );
// changed mg
	for( i = 0; i < BP_GI_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_GI_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			break;
		case BP_GI_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_GI_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		case BP_GI_UPGRADE:
		case BP_GI_UPGRADE2:
		case BP_GI_UPGRADE3:
			Com_sprintf( name, sizeof(name), "%s_upg.md3", basename );
			break;
		}
		availableGroundInstallations[index].handle[i] = trap_R_RegisterModel( name );
//		if( !availableVehicles[index].handle[i] ) {
//			CG_Printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableGroundInstallations[index].handle[BP_GV_BODY] ) {
		trap_Cache_Error( va("MFQ3 Error: Invalid handle for body %s.md3\n", basename) );
	}
}

/*
===============
CG_InitShadows

Initialise all the shadow parameters for each vehicle
===============
*/

void CG_InitShadows( void )
{
	qhandle_t customShadowShader = -1;
	float xRad = 0, yRad = 0;
	int vehicle = 0;

	// cache the vehicle's custom shadow shader (if we can find one - format is <modelName>Shadow, "like f-16Shadow")
	for( vehicle = 0; vehicle < bg_numberOfVehicles; vehicle++ )
	{
		// try to load custom shader
		customShadowShader = trap_R_RegisterShader( va( "%sShadow", availableVehicles[ vehicle ].modelName ) );
		
		// replace SHADOW_x fallback op. with the custom shader handle?
		if( customShadowShader )
		{
			availableVehicles[ vehicle ].shadowShader = customShadowShader;
		}

		// setup default shadow size adjusters if none set
		if( availableVehicles[ vehicle ].shadowCoords[ SHC_XADJUST ] == 0 &&
			availableVehicles[ vehicle ].shadowCoords[ SHC_YADJUST ] == 0 )
		{
			// create x & y adjusters from the vehicle's bound box
			xRad = availableVehicles[ vehicle ].maxs[ 0 ];
			yRad = availableVehicles[ vehicle ].maxs[ 1 ];
			xRad -= availableVehicles[ vehicle ].mins[ 0 ];
			yRad -= availableVehicles[ vehicle ].mins[ 1 ];
			xRad *= 0.5f;
			yRad *= 0.5f;
			availableVehicles[ vehicle ].shadowCoords[ SHC_XADJUST ] = xRad;
			availableVehicles[ vehicle ].shadowCoords[ SHC_YADJUST ] = yRad;
		}

		// setup default shadow orientation modifiers/adjusters if none set
		// NOTE: will be set for all vehicles, but currently only used on flying ones
		
		// maximums
		if( availableVehicles[ vehicle ].shadowAdjusts[ SHO_PITCHMAX ] == 0 &&
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_ROLLMAX ] == 0 )
		{
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_PITCHMAX ] = 90.0f;
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_ROLLMAX ] = 90.0f;
		}

		// modifiers
		if( availableVehicles[ vehicle ].shadowAdjusts[ SHO_PITCHMOD ] == 0 &&
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_ROLLMOD ] == 0 )
		{
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_PITCHMOD ] = 0.8f;
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_ROLLMOD ] = 0.8f;
		}
	}
}

/*
===============
CG_CacheVehicles
===============
*/

void CG_CacheVehicles()
{
	int i;

	for( i = 0; i < bg_numberOfVehicles; i++ )
	{
		if( availableVehicles[i].gameset & cgs.gameset ) {
			CG_LoadingString(availableVehicles[i].descriptiveName);
			if( availableVehicles[i].cat & CAT_PLANE ) {
					CG_CachePlane(i);
			}
			else if( availableVehicles[i].cat & CAT_GROUND ) {
					CG_CacheGroundVehicle(i);
			}
			else if( availableVehicles[i].cat & CAT_HELO ) {
					CG_CacheHelo(i);
			}
			else if( availableVehicles[i].cat & CAT_LQM ) {
					CG_CacheLQM(i);
			}
			else if( availableVehicles[i].cat & CAT_BOAT ) {
					CG_CacheBoat(i);
			}
			else {
				trap_Error( va("Invalid Vehicle type in CG_CacheVehicles (%d,%d)", 
					i, availableVehicles[i].cat));
			}
		}
	}
	for( i = 0; i < bg_numberOfGroundInstallations; i++ ) {
		if( availableGroundInstallations[i].gameset & cgs.gameset ) {
			CG_LoadingString(availableGroundInstallations[i].descriptiveName);
			CG_CacheGroundInstallation(i);
		}
	}

}

/*
===============
CG_RegisterVehicle

load the vehicle
===============
*/
void CG_RegisterVehicle( clientInfo_t *ci )
{
//	Com_Printf( "RegVehicle = %i\n" ,ci->vehicle);    
	if( availableVehicles[ci->vehicle].cat & CAT_PLANE ) {
		CG_RegisterPlane(ci);
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_GROUND ) {
		CG_RegisterGroundVehicle(ci);
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_HELO ) {
		CG_RegisterHelo(ci);
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_LQM ) {
		CG_RegisterLQM(ci);
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_BOAT ) {
		CG_RegisterBoat(ci);
	}
	else
		CG_Error( "Invalid vehicle type in register vehicle" );
}

/*
===============
CG_Vehicle
===============
*/
void CG_Vehicle( centity_t *cent ) 
{
    clientInfo_t * ci;
    int	clientNum;

    // get client num and info
    clientNum = cent->currentState.clientNum;
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
	{
		trap_Error( "Bad clientNum on player entity");
    }
    ci = &cgs.clientinfo[ clientNum ];
	//Com_Printf( "Vehicle = %i\n" ,ci->vehicle);    

	// don't continue if client info is invalid
    if ( !ci->infoValid )
	{
		return;
    }

	// don't draw ourselves, if we are rendering the MFDs
	if( cent == &cg.predictedPlayerEntity && cg.drawingMFD )
	{
		return;
	}

	// plane?
	if( availableVehicles[ci->vehicle].cat & CAT_PLANE )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR))
		{
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Plane( cent, ci );
	}
	// ground vehicle?
	else if( availableVehicles[ci->vehicle].cat & CAT_GROUND )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND))
		{
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_GroundVehicle( cent, ci );
	}
	// helo?
	else if( availableVehicles[ci->vehicle].cat & CAT_HELO )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR))
		{
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Helo( cent, ci );
	}
	// lqm?
	else if( availableVehicles[ci->vehicle].cat & CAT_LQM )
	{
		CG_LQM( cent, ci );
	}
	// boat ?
	else if( availableVehicles[ci->vehicle].cat & CAT_BOAT )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND))
		{
			cg.radarEnts[cg.radarTargets++] = cent;
		}
		CG_Boat( cent, ci );
	}
	else
	{
		trap_Error( "Error: CG_Vehicle got wrong CAT!\n" );
	}
}


/*
=============
CG_VehicleObituary
=============
*/
void CG_VehicleObituary( entityState_t *ent ) 
{
    clientInfo_t	*ci;
    int		        clientNum = ent->otherEntityNum;

    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
	    CG_Error( "CG_VehicleObituary: target out of range" );
    }
    ci = &cgs.clientinfo[clientNum];

    if ( !ci->infoValid ) {
		CG_Printf( "No obituary available!\n" );
		return;
    }

	if( availableVehicles[ci->vehicle].cat & CAT_PLANE ) {
		CG_PlaneObituary( ent, ci );
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_GROUND ) {
		CG_GroundVehicleObituary( ent, ci );
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_HELO ) {
		CG_HeloObituary( ent, ci );
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_LQM ) {
		CG_LQMObituary( ent, ci );
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_BOAT ) {
		CG_BoatObituary( ent, ci );
	}
	else {
		CG_PlaneObituary( ent, ci );
//		trap_Error( va("Invalid vehicle type in Vehicle obituary %s-%d\n", ci->name, ci->vehicle) );
	}

}

/*
=============
CG_VehicleMuzzleFlash

=============
*/
void CG_VehicleMuzzleFlash( int weaponIdx, const refEntity_t *parent, qhandle_t parentModel, int idx ) {

	refEntity_t		flash[MAX_MUZZLEFLASHES];
	vec3_t			angles;
	unsigned int	i;
	unsigned int	flashes;
	char			tag[10];

//	flashes = availableWeapons[availableVehicles[idx].weapons[0]].barrels;
	flashes = availableWeapons[weaponIdx].barrels;

	if( flashes > MAX_MUZZLEFLASHES ) flashes = MAX_MUZZLEFLASHES;

	for( i = 0; i < flashes; i++ ) {
		memset( &flash[i], 0, sizeof( flash[i] ) );
		VectorCopy( parent->lightingOrigin, flash[i].lightingOrigin );
		flash[i].shadowPlane = parent->shadowPlane;
		flash[i].renderfx = parent->renderfx;
		flash[i].hModel = cgs.media.vehicleMuzzleFlashModel;

		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = crandom() * 10;
		AnglesToAxis( angles, flash[i].axis );

		if( i > 0 ) {
			Com_sprintf( tag, 10, "tag_gun%d", i+1 );
		} else {
			strcpy( tag, "tag_gun" );
		}
		CG_PositionRotatedEntityOnTag( &flash[i], parent, parentModel, tag);
		trap_R_AddRefEntityToScene( &flash[i] );
	}
}



/*
=============
CG_VehicleLoadout

=============
*/
void CG_VehicleLoadout( centity_t* cent ) {
	int idx = -1;

	if( cent->currentState.eType == ET_VEHICLE ) {
	    clientInfo_t	*ci;
		int				clientNum;
	    clientNum = cent->currentState.clientNum;
		if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
			trap_Error( "Bad clientNum on player entity (CG_VehicleLoadout)");
		}
		ci = &cgs.clientinfo[ clientNum ];
		idx = ci->vehicle;
	} else if( cent->currentState.eType == ET_MISC_VEHICLE ) {
		idx = cent->currentState.modelindex;
	} else return;

	MF_getDefaultLoadoutForVehicle( idx, &cg_loadouts[cent->currentState.number] );
}



/*
=============
CG_AddToVehicleLoadout

=============
*/
void CG_AddToVehicleLoadout( centity_t* cent, int weaponIndex ) {
	int idx = -1;

	if( cent->currentState.eType == ET_VEHICLE ) {
	    clientInfo_t	*ci;
		int				clientNum;
	    clientNum = cent->currentState.clientNum;
		if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
			trap_Error( "Bad clientNum on player entity (CG_VehicleLoadout)");
		}
		ci = &cgs.clientinfo[ clientNum ];
		idx = ci->vehicle;
	} else if( cent->currentState.eType == ET_MISC_VEHICLE ) {
		idx = cent->currentState.modelindex;
	} else return;

	MF_addWeaponToLoadout( weaponIndex, &cg_loadouts[cent->currentState.number] );
}



