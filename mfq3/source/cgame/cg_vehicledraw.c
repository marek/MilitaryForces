/*
 * $Id: cg_vehicledraw.c,v 1.4 2003-01-11 13:08:50 thebjoern Exp $
*/

#include "cg_local.h"

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

char *gi_tags[BP_GI_MAX_PARTS] =
{
	"<no tag>",		// vehicle body
	"tag_turret",	// turret
	"tag_weap",		// gun
	"tag_upgrade",	
	"tag_upgrade",	
	"tag_upgrade"	
};

char *boat_tags[BP_BOAT_MAX_PARTS] =
{
	"<no tag>",		// vehicle body
	"tag_turret",	// turret
	"tag_weap",		// gun
	"tag_turret2",	// turret
	"tag_weap2",		// gun
	"tag_turret3",	// turret
	"tag_weap",		// gun
	"tag_turret4",	// turret
	"tag_weap"		// gun
};


/*
=============
CG_PlanePilot
=============
*/
static void CG_PlanePilot( const refEntity_t *parent, qhandle_t parentModel, int idx ) {

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
	}
}

void CG_DrawPlane(DrawInfo_Plane_t* drawInfo)
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

	for( i = 0; i < BP_PLANE_MAX_PARTS; i++ ) {
	    memset( &part[i], 0, sizeof(part[0]) );	
	}
    memset( &vapor, 0, sizeof(vapor) );	
    memset( &burner, 0, sizeof(burner) );	
    memset( &burner2, 0, sizeof(burner2) );	
	memset( &reticle, 0, sizeof(reticle) );	

   // add the shadow
	switch( cg_shadows.integer )
	{
	case 1:
		CG_GenericShadow( &drawInfo->basicInfo, &shadowPlane );
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
	trap_R_AddRefEntityToScene( &part[BP_PLANE_BODY] );
	
	// if the model failed, allow the default nullmodel to be displayed, but nothing else
	if (!part[BP_PLANE_BODY].hModel) {
		return;
	}

	// other parts
	for( i = 1; i < BP_PLANE_MAX_PARTS; i++ ) {
		part[i].hModel = veh->handle[i];
		if( !part[i].hModel ) {
			continue;
		}
		VectorCopy( drawInfo->basicInfo.origin, part[i].lightingOrigin );
		AxisCopy( axisDefault, part[i].axis );
		if( i == BP_PLANE_PROP && (veh->caps & HC_PROP) ) {
			int ii;
			RotateAroundDirection( part[i].axis, cg.time );
			for( ii = 1; ii < veh->engines; ++ii ) {
				refEntity_t engine;
				memcpy( &engine, &part[i], sizeof(engine) );
				CG_PositionRotatedEntityOnTag( &engine, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], engine_tags[ii-1] );
				engine.shadowPlane = shadowPlane;
				engine.renderfx = renderfx;
				trap_R_AddRefEntityToScene( &engine );
			}
		}
		if( (i == BP_PLANE_WINGLEFT || i == BP_PLANE_WINGRIGHT) &&
			(veh->caps & HC_SWINGWING) ) {
			RotateAroundYaw( part[BP_PLANE_WINGLEFT].axis, drawInfo->swingAngle );
			RotateAroundYaw( part[BP_PLANE_WINGRIGHT].axis, -drawInfo->swingAngle );
		}
		CG_PositionRotatedEntityOnTag( &part[i], &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], plane_tags[i] );
		part[i].shadowPlane = shadowPlane;
		part[i].renderfx = renderfx;
		trap_R_AddRefEntityToScene( &part[i] );
	}
	// pilot
	CG_PlanePilot( &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], drawInfo->basicInfo.vehicleIndex );
	
	// vwep
	if( (veh->renderFlags & MFR_VWEP) && drawInfo->basicInfo.loadout ) {
		refEntity_t			vwep;

		for( i = 0; i < drawInfo->basicInfo.loadout->usedMounts; ++i ) {
			if( drawInfo->basicInfo.loadout->mounts[i].weapon && 
				drawInfo->basicInfo.loadout->mounts[i].num ) {
				memset( &vwep, 0, sizeof(vwep) );		
				vwep.hModel = availableWeapons[drawInfo->basicInfo.loadout->mounts[i].weapon].vwepHandle;

			//	scheherazade +
			//	This was an experiment wiht frames. i didnt complete it because i'd either
			//	have to add info into the draw info, or add parameters to pull in the ammo data,
			//	which i dont want to do because i dont know if you have a way you'd go about it that you like more, etc.
			//	if ( availableWeapons[drawInfo->basicInfo.loadout->mounts[i].weapon].type == WT_ROCKET )
			//	{
			//		if(drawInfo->basicInfo.loadout->mounts[i].num <= availableWeapons[drawInfo->basicInfo.loadout->mounts[i].weapon].numberPerPackage)
			//		vwep.frame = ( availableWeapons[drawInfo->basicInfo.loadout->mounts[i].weapon].numberPerPackage - drawInfo->basicInfo.loadout->mounts[i].num );
			//		Mental Note, num does not contain the remailing Number-In-Package, maybe it should so as to make this change easier.
			//		vwep.frame = ( availableWeapons[drawInfo->basicInfo.loadout->mounts[i].weapon].numberPerPackage - ammo[(drawInfo->basicInfo.loadout->mounts[i].weapon)] );
			//		vwep.frame = max_Ammo_per_ffar_pack - Ammo_left_in_ffar_pack
			//	}
			//	scheherazade -

				VectorCopy( drawInfo->basicInfo.origin, vwep.lightingOrigin );
				AxisCopy( axisDefault, vwep.axis );
				CG_PositionEntityOnTag( &vwep, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], 
						drawInfo->basicInfo.loadout->mounts[i].tag.name );
				vwep.shadowPlane = shadowPlane;
				vwep.renderfx = renderfx;
				trap_R_AddRefEntityToScene( &vwep );
			}
		}
	}

	// vapor
	if( drawInfo->basicInfo.ONOFF & OO_VAPOR ) {
		vapor.hModel = (veh->renderFlags & MFR_BIGVAPOR) ? cgs.media.vaporBig : cgs.media.vapor;
		vapor.customShader = cgs.media.vaporShader;
		if( drawInfo->basicInfo.speed < veh->stallspeed * SPEED_GREEN_ARC &&
			drawInfo->basicInfo.speed > veh->stallspeed * VAPORSPEED ) {
			vapor.frame = 1;
		}
		VectorCopy( drawInfo->basicInfo.origin, vapor.lightingOrigin );
		AxisCopy( axisDefault, vapor.axis );
		CG_PositionEntityOnTag( &vapor, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], "tag_vapor1" );
		vapor.shadowPlane = shadowPlane;
		vapor.renderfx = renderfx;
		trap_R_AddRefEntityToScene( &vapor );
	}
	
	// throttle/afterburner
	if( drawInfo->basicInfo.throttle > 10 ) {
		burner.hModel = cgs.media.afterburner[veh->effectModel];
		VectorCopy( drawInfo->basicInfo.origin, burner.lightingOrigin );
		AxisCopy( axisDefault, burner.axis );
		CG_PositionEntityOnTag( &burner, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], "tag_ab1" );
		burner.shadowPlane = shadowPlane;
		burner.renderfx = renderfx;
		burner.frame = drawInfo->bodyFrame;
		trap_R_AddRefEntityToScene( &burner );
		if( veh->engines > 1 ) {
			burner2.hModel = cgs.media.afterburner[veh->effectModel];
			VectorCopy( drawInfo->basicInfo.origin, burner2.lightingOrigin );
			AxisCopy( axisDefault, burner2.axis );
			CG_PositionRotatedEntityOnTag( &burner2, &part[BP_PLANE_BODY], veh->handle[BP_PLANE_BODY], "tag_ab2" );
			burner2.shadowPlane = shadowPlane;
			burner2.renderfx = renderfx;
			burner2.frame = drawInfo->bodyFrame;
			trap_R_AddRefEntityToScene( &burner2 );
		}
	}

	// sound
	if( drawInfo->basicInfo.entityNum >= 0 ) {
		if ( veh->caps & HC_PROP ) {
			trap_S_AddLoopingSound( drawInfo->basicInfo.entityNum, drawInfo->basicInfo.origin, vec3_origin, 
									cgs.media.engineProp );
		}
		else {
			if( drawInfo->basicInfo.throttle > 10 ) {
				trap_S_AddLoopingSound( drawInfo->basicInfo.entityNum, drawInfo->basicInfo.origin, vec3_origin, 
										cgs.media.engineJetAB );
			}
			else {
				trap_S_AddLoopingSound( drawInfo->basicInfo.entityNum, drawInfo->basicInfo.origin, vec3_origin,
										cgs.media.engineJet );
			}
		}
	}
	
	// muzzleflash
	if( drawInfo->basicInfo.drawMuzzleFlash ) {
		CG_VehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_PLANE_BODY], 
							   veh->handle[BP_PLANE_BODY], drawInfo->basicInfo.vehicleIndex );
	}
	
}

void CG_DrawGV(DrawInfo_GV_t* drawInfo)
{
	refEntity_t	    part[BP_GV_MAX_PARTS];
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				i, tanksound;

	completeVehicleData_t* veh = &availableVehicles[drawInfo->basicInfo.vehicleIndex];

	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
	    memset( &part[i], 0, sizeof(part[0]) );	
	}

    // add the shadow
	switch( cg_shadows.integer )
	{
	case 1:
		CG_GenericShadow( &drawInfo->basicInfo, &shadowPlane );
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
    part[BP_GV_BODY].hModel = veh->handle[BP_GV_BODY];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_BODY].origin );
    VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_BODY].lightingOrigin );
	AxisCopy( drawInfo->basicInfo.axis, part[BP_GV_BODY].axis );
    part[BP_GV_BODY].shadowPlane = shadowPlane;
    part[BP_GV_BODY].renderfx = renderfx;
    VectorCopy (part[BP_GV_BODY].origin, part[BP_GV_BODY].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_BODY] );

    //
    // turret
    //
    part[BP_GV_TURRET].hModel = veh->handle[BP_GV_TURRET];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_TURRET].lightingOrigin );
	AxisCopy( axisDefault, part[BP_GV_TURRET].axis );
	RotateAroundYaw( part[BP_GV_TURRET].axis, drawInfo->turretAngle );
	CG_PositionRotatedEntityOnTag( &part[BP_GV_TURRET], &part[BP_GV_BODY], veh->handle[BP_GV_BODY], gv_tags[BP_GV_TURRET] );
	part[BP_GV_TURRET].shadowPlane = shadowPlane;
    part[BP_GV_TURRET].renderfx = renderfx;
    VectorCopy (part[BP_GV_TURRET].origin, part[BP_GV_TURRET].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_TURRET] );

    //
    // gun
    //
    part[BP_GV_GUNBARREL].hModel = veh->handle[BP_GV_GUNBARREL];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_GUNBARREL].lightingOrigin );
	AxisCopy( axisDefault, part[BP_GV_GUNBARREL].axis );
	RotateAroundPitch( part[BP_GV_GUNBARREL].axis, drawInfo->gunAngle );
	CG_PositionRotatedEntityOnTag( &part[BP_GV_GUNBARREL], &part[BP_GV_TURRET], veh->handle[BP_GV_TURRET], gv_tags[BP_GV_GUNBARREL] );
	part[BP_GV_GUNBARREL].shadowPlane = shadowPlane;
    part[BP_GV_GUNBARREL].renderfx = renderfx;
    VectorCopy (part[BP_GV_GUNBARREL].origin, part[BP_GV_GUNBARREL].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_GV_GUNBARREL] );

	//
	// wheels
	//
	// gearAnimStartTime is for timediff
	// gearAnim is for angles
	if( (veh->caps & HC_WHEELS) ) {
		int ii;
		for( ii = 0; ii < availableVehicles[drawInfo->basicInfo.vehicleIndex].wheels; ++ii ) {
			part[BP_GV_WHEEL+ii].hModel = veh->handle[BP_GV_WHEEL+ii];
			VectorCopy( drawInfo->basicInfo.origin, part[BP_GV_WHEEL+ii].lightingOrigin );
			AxisCopy( axisDefault, part[BP_GV_WHEEL+ii].axis );
			part[BP_GV_WHEEL+ii].shadowPlane = shadowPlane;
			part[BP_GV_WHEEL+ii].renderfx = renderfx;
			VectorCopy (part[BP_GV_WHEEL+ii].origin, part[BP_GV_WHEEL+ii].oldorigin);
			RotateAroundPitch( part[BP_GV_WHEEL+ii].axis, drawInfo->wheelAngle );
			CG_PositionRotatedEntityOnTag( &part[BP_GV_WHEEL+ii], &part[BP_GV_BODY], veh->handle[BP_GV_BODY], gv_tags[BP_GV_WHEEL+ii] );
			trap_R_AddRefEntityToScene( &part[BP_GV_WHEEL+ii] );
		}
	}

	// sound
	if( drawInfo->basicInfo.entityNum >= 0 ) {
		tanksound = drawInfo->basicInfo.speed * NUM_TANKSOUNDS / veh->maxspeed;
		if( tanksound >= NUM_TANKSOUNDS ) tanksound = NUM_TANKSOUNDS - 1;
		trap_S_AddLoopingSound( drawInfo->basicInfo.entityNum, 
								drawInfo->basicInfo.origin, 
								vec3_origin, 
								cgs.media.engineTank[tanksound] );
	}

	// muzzleflash
	if( drawInfo->basicInfo.drawMuzzleFlash ) {
		CG_VehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_GV_GUNBARREL], 
							   veh->handle[BP_GV_GUNBARREL], drawInfo->basicInfo.vehicleIndex );
	}
}

void CG_DrawBoat(DrawInfo_Boat_t* drawInfo)
{
	refEntity_t	    part[BP_BOAT_MAX_PARTS];
	float			shadowPlane = 0;
	int				renderfx = 0;
	int				i, tanksound;

	completeVehicleData_t* veh = &availableVehicles[drawInfo->basicInfo.vehicleIndex];

	for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
	    memset( &part[i], 0, sizeof(part[0]) );	
	}

    // add the shadow
	switch( cg_shadows.integer )
	{
	case 1:
		CG_GenericShadow( &drawInfo->basicInfo, &shadowPlane );
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
    part[BP_BOAT_BODY].hModel = veh->handle[BP_BOAT_BODY];
    VectorCopy( drawInfo->basicInfo.origin, part[BP_BOAT_BODY].origin );
    VectorCopy( drawInfo->basicInfo.origin, part[BP_BOAT_BODY].lightingOrigin );
	AxisCopy( drawInfo->basicInfo.axis, part[BP_GV_BODY].axis );
    part[BP_BOAT_BODY].shadowPlane = shadowPlane;
    part[BP_BOAT_BODY].renderfx = renderfx;
    VectorCopy (part[BP_BOAT_BODY].origin, part[BP_BOAT_BODY].oldorigin);
    trap_R_AddRefEntityToScene( &part[BP_BOAT_BODY] );

    //
    // turrets
    //
	for( i = 0; i < 4; ++i ) {
		int j = 2*i;
		// turret
		part[BP_BOAT_TURRET+j].hModel = veh->handle[BP_BOAT_TURRET+j];
		if( !part[BP_BOAT_TURRET+j].hModel ) break;
		VectorCopy( drawInfo->basicInfo.origin, part[BP_BOAT_TURRET+j].lightingOrigin );
		AxisCopy( axisDefault, part[BP_BOAT_TURRET+j].axis );
		RotateAroundYaw( part[BP_BOAT_TURRET+j].axis, drawInfo->turretAngle[i] );
		CG_PositionRotatedEntityOnTag( &part[BP_BOAT_TURRET+j], &part[BP_BOAT_BODY], veh->handle[BP_BOAT_BODY], boat_tags[BP_BOAT_TURRET+j] );
		part[BP_BOAT_TURRET+j].shadowPlane = shadowPlane;
		part[BP_BOAT_TURRET+j].renderfx = renderfx;
		VectorCopy (part[BP_BOAT_TURRET+j].origin, part[BP_BOAT_TURRET+j].oldorigin);
		trap_R_AddRefEntityToScene( &part[BP_BOAT_TURRET+j] );
		// gun
		part[BP_BOAT_GUNBARREL+j].hModel = veh->handle[BP_BOAT_GUNBARREL+j];
		VectorCopy( drawInfo->basicInfo.origin, part[BP_BOAT_GUNBARREL+j].lightingOrigin );
		AxisCopy( axisDefault, part[BP_BOAT_GUNBARREL+j].axis );
		RotateAroundPitch( part[BP_BOAT_GUNBARREL+j].axis, drawInfo->gunAngle[i] );
		CG_PositionRotatedEntityOnTag( &part[BP_BOAT_GUNBARREL+j], &part[BP_BOAT_TURRET+j], veh->handle[BP_BOAT_TURRET+j], boat_tags[BP_BOAT_GUNBARREL+j] );
		part[BP_BOAT_GUNBARREL+j].shadowPlane = shadowPlane;
		part[BP_BOAT_GUNBARREL+j].renderfx = renderfx;
		VectorCopy (part[BP_BOAT_GUNBARREL+j].origin, part[BP_BOAT_GUNBARREL+j].oldorigin);
		trap_R_AddRefEntityToScene( &part[BP_BOAT_GUNBARREL+j] );

	}

	// sound
	if( drawInfo->basicInfo.entityNum >= 0 ) {
		tanksound = drawInfo->basicInfo.speed * NUM_TANKSOUNDS / veh->maxspeed;
		if( tanksound >= NUM_TANKSOUNDS ) tanksound = NUM_TANKSOUNDS - 1;
		trap_S_AddLoopingSound( drawInfo->basicInfo.entityNum, 
								drawInfo->basicInfo.origin, 
								vec3_origin, 
								cgs.media.engineTank[tanksound] );
	}

	// muzzleflash
	if( drawInfo->basicInfo.drawMuzzleFlash ) {
		CG_VehicleMuzzleFlash( drawInfo->basicInfo.flashWeaponIndex, &part[BP_BOAT_GUNBARREL], 
							   veh->handle[BP_BOAT_GUNBARREL], drawInfo->basicInfo.vehicleIndex );
	}
}

void CG_DrawHelo(DrawInfo_Helo_t* drawInfo)
{

}

void CG_DrawLQM(DrawInfo_LQM_t* drawInfo)
{

}

void CG_DrawGI(DrawInfo_GI_t* drawInfo)
{
	groundInstallationData_t* veh = &availableGroundInstallations[drawInfo->basicInfo.vehicleIndex];
	refEntity_t part[BP_GI_MAX_PARTS];
	int i;
	for( i = 0; i < BP_GI_MAX_PARTS; i++ ) {
		memset( &part[i], 0, sizeof(part[0]) );	
	}	
	// body
	part[BP_GI_BODY].hModel = veh->handle[BP_GI_BODY];		
	VectorCopy( drawInfo->basicInfo.origin, part[BP_GI_BODY].origin );	
	VectorCopy( drawInfo->basicInfo.origin, part[BP_GI_BODY].oldorigin);
	AxisCopy( drawInfo->basicInfo.axis, part[BP_GI_BODY].axis );
	trap_R_AddRefEntityToScene( &part[BP_GI_BODY] );
	// other parts
	for( i = 1; i < BP_GI_MAX_PARTS; i++ ) {
		if( (i == BP_GI_UPGRADE && !veh->upgrades) ||
			(i == BP_GI_UPGRADE2 && veh->upgrades < 2 ) ||
			(i == BP_GI_UPGRADE3 && veh->upgrades < 3 ) ) {
			break;;
		}
		part[i].hModel = veh->handle[i];
		if( !part[i].hModel ) continue;
		VectorCopy( drawInfo->basicInfo.origin, part[i].lightingOrigin );
		AxisCopy( axisDefault, part[i].axis );
		if( i == BP_GI_GUNBARREL ) {
			RotateAroundPitch( part[i].axis, drawInfo->gunAngle );
			CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_TURRET], 
				veh->handle[BP_GI_TURRET], gi_tags[i] );
		} else if( i == BP_GI_UPGRADE ) {
			CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_GUNBARREL], 
				veh->handle[BP_GI_GUNBARREL], gi_tags[i] );
		} else if( i == BP_GI_UPGRADE2 ) {
			CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_UPGRADE], 
				veh->handle[BP_GI_UPGRADE], gi_tags[i] );
		} else if( i == BP_GI_UPGRADE3 ) {
			CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_UPGRADE2], 
				veh->handle[BP_GI_UPGRADE2], gi_tags[i] );
		} else {
			RotateAroundYaw( part[i].axis, drawInfo->turretAngle );
			CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_BODY], 
				veh->handle[BP_GI_BODY], gi_tags[i] );
		}
		trap_R_AddRefEntityToScene( &part[i] );
	}
}