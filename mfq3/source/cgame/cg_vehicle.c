/*
 * $Id: cg_vehicle.c,v 1.8 2002-02-15 09:58:31 thebjoern Exp $
*/

#include "cg_local.h"

/*
===============
CG_CachePlane

On startup cache it
===============
*/

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
		trap_Error( va("MFQ3 Error: Invalid handle for body %s.md3\n", basename) );
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
	if( !availableVehicles[index].handle[BP_PLANE_BODY] ) {
		trap_Error( va("MFQ3 Error: Invalid handle for body %s.md3\n", basename) );
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
	qhandle_t customShadowShader = -1;

	// later there could also be a check to only cache the current gameset
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
			else {
				trap_Error( va("Invalid Vehicle type in CG_CacheVehicles (%d,%d)", 
					i, availableVehicles[i].cat));
			}
		}

		// cache the vehicle's custom shadow shader (if we can find one - format is <modelName>Shadow, "like f-16Shadow")

		// try to load custom shader
		customShadowShader = trap_R_RegisterShader( va( "%sShadow", availableVehicles[i].modelName ) );
		
		// replace SHADOW_x fallback op. with the custom shader handle?
		if( customShadowShader )
		{
			availableVehicles[i].shadowShader = customShadowShader;
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
void CG_VehicleMuzzleFlash( centity_t *cent, const refEntity_t *parent, qhandle_t parentModel, int idx ) {

	refEntity_t		flash[MAX_MUZZLEFLASHES];
	vec3_t			angles;
	unsigned int	i;
	unsigned int	flashes;
	char			tag[10];

	// impulse flash
	if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME ) {
		return;
	}

	flashes = availableWeapons[availableVehicles[idx].weapons[0]].barrels;

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

