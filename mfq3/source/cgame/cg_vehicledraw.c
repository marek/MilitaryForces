/*
 * $Id: cg_vehicledraw.c,v 1.2 2002-07-14 17:13:19 thebjoern Exp $
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

}

void CG_DrawBoat(DrawInfo_Boat_t* drawInfo)
{

}

void CG_DrawHelo(DrawInfo_Helo_t* drawInfo)
{

}

void CG_DrawLQM(DrawInfo_LQM_t* drawInfo)
{

}
