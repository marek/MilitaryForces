/*
 * $Id: cg_missioneditor.c,v 1.1 2002-06-12 14:35:33 thebjoern Exp $
*/

#include "cg_local.h"

#define MAX_SELECTION_DISTANCE			1000.0f

extern char *plane_tags[BP_PLANE_MAX_PARTS];
extern char *gv_tags[BP_GV_MAX_PARTS];
extern char *engine_tags[3];

void ME_DrawVehicle( IGME_vehicle_t* veh );
void ME_CheckForSelection();

void CG_Draw_IGME()
{
	int				i;
	usercmd_t		cmd;
	int				cmdNum;
	IGME_vehicle_t* veh;

	// check for selections
	cmdNum = trap_GetCurrentCmdNumber();
	trap_GetUserCmd( cmdNum, &cmd );
	if( (cmd.buttons & BUTTON_ATTACK) && cg.time >= cgs.IGME.selectionTime ) {
		ME_CheckForSelection();
		cgs.IGME.selectionTime = cg.time + 250;
	}
	
	// draw vehicles
	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		veh = &cgs.IGME.vehicles[i]; 
		if( !veh->active ) continue;
		ME_DrawVehicle(veh);
	}
}

void ME_DeselectAll()
{
	int i;
	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		cgs.IGME.vehicles[i].selected = qfalse;
	}
}

void ME_SelectVehicle( IGME_vehicle_t* veh )
{
	qboolean multisel = trap_Key_IsDown(K_CTRL);

	if( !veh ) {
		ME_DeselectAll();
		return;
	}

	if( veh->selected ) {
		veh->selected = qfalse;
		if( !multisel ) ME_DeselectAll();
	} else {
		if( !multisel ) ME_DeselectAll();
		veh->selected = qtrue;
	}
}

void ME_CheckForSelection()
{
	vec3_t			mins, maxs, dir;
	float			dist, closestDist = MAX_SELECTION_DISTANCE;
	IGME_vehicle_t* selveh = 0;
	IGME_vehicle_t*	veh = 0;
	box3_t			bbox;
	ray3_t			ray;
	int				i;

	VectorCopy( cg.predictedPlayerState.origin, ray.start );
	AngleVectors( cg.predictedPlayerState.viewangles, ray.dir, 0, 0 );
	VectorScale( ray.dir, MAX_SELECTION_DISTANCE, ray.dir );

	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		veh = &cgs.IGME.vehicles[i];
		// not active
		if( !veh->active ) continue;
		VectorSubtract( veh->origin, ray.start, dir );
		dist = VectorNormalize( dir );
		// too far away
		if( dist > closestDist ) continue;
		VectorAdd( availableVehicles[veh->vehidx].mins, veh->origin, mins );
		VectorAdd( availableVehicles[veh->vehidx].maxs, veh->origin, maxs );
		MakeBoxFromExtents(&bbox, mins, maxs, veh->angles);
		if( RayIntersectBox(&ray, &bbox) ) {
			closestDist = dist;
			selveh = veh;
		} 
	}
	ME_SelectVehicle(selveh);
}

void ME_SpawnVehicle( int vehidx ) 
{
	IGME_vehicle_t*	 veh = 0;
	int i;

	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		if( !cgs.IGME.vehicles[i].active ) {
			veh = &cgs.IGME.vehicles[i];
			break;
		}
	}
	if( !veh ) {
		CG_Printf( "Too many vehicles spawned! Please delete one first!\n" );
		return;
	}

	VectorCopy( cg.predictedPlayerState.origin, veh->origin );
	VectorCopy( cg.predictedPlayerState.viewangles, veh->angles );
	veh->angles[0] = veh->angles[2] = 0;
	veh->vehidx = vehidx;
	veh->active = qtrue;
	veh->selected = qfalse;

	for( i = 0; i < 3; ++i ) {
		float wid = availableVehicles[veh->vehidx].maxs[i] - 
					availableVehicles[veh->vehidx].mins[i];
		veh->selectorScale[i] = wid/200.0f;
	}

}

void ME_DrawVehicle( IGME_vehicle_t* veh ) 
{
	if( !veh ) return;

	switch( availableVehicles[veh->vehidx].cat ) {
	case CAT_PLANE:
		{
			refEntity_t part[BP_PLANE_MAX_PARTS];
			int i;
			for( i = 0; i < BP_PLANE_MAX_PARTS; i++ ) {
				memset( &part[i], 0, sizeof(part[0]) );	
			}	
			// body
		    AnglesToAxis( veh->angles, part[BP_PLANE_BODY].axis );
			part[BP_PLANE_BODY].hModel = availableVehicles[veh->vehidx].handle[BP_PLANE_BODY];		
			VectorCopy( veh->origin, part[BP_PLANE_BODY].origin );	
			VectorCopy( veh->origin, part[BP_PLANE_BODY].oldorigin);
			trap_R_AddRefEntityToScene( &part[BP_PLANE_BODY] );
			// other parts
			for( i = 1; i < BP_PLANE_MAX_PARTS; i++ ) {
				part[i].hModel = availableVehicles[veh->vehidx].handle[i];
				if( !part[i].hModel ) continue;
				VectorCopy( veh->origin, part[i].lightingOrigin );
				AxisCopy( axisDefault, part[i].axis );
				if( i == BP_PLANE_PROP && (availableVehicles[veh->vehidx].caps & HC_PROP) ) {
					int ii;
					for( ii = 1; ii < availableVehicles[veh->vehidx].engines; ++ii ) {
						refEntity_t engine;
						memcpy( &engine, &part[i], sizeof(engine) );
						CG_PositionRotatedEntityOnTag( &engine, &part[BP_PLANE_BODY], 
							availableVehicles[veh->vehidx].handle[BP_PLANE_BODY], engine_tags[ii-1] );
						trap_R_AddRefEntityToScene( &engine );
					}
				}
				CG_PositionRotatedEntityOnTag( &part[i], &part[BP_PLANE_BODY], 
						availableVehicles[veh->vehidx].handle[BP_PLANE_BODY], plane_tags[i] );
				trap_R_AddRefEntityToScene( &part[i] );
			}
		}
		break;
	case CAT_GROUND:
		break;
	case CAT_BOAT:
		break;
	case CAT_HELO:
		break;
	case CAT_LQM:
		break;
	}

	// draw selectors
	if( veh->selected ) {
		refEntity_t	sel;
		int j;
		memset( &sel, 0, sizeof(sel) );
		sel.hModel = cgs.media.IGME_selector;
		VectorCopy( veh->origin, sel.origin );
		VectorCopy( veh->origin, sel.lightingOrigin );
		VectorCopy( veh->origin, sel.oldorigin );
		AnglesToAxis( veh->angles, sel.axis );
		for( j = 0; j < 3; ++j ) {
			VectorScale( sel.axis[j], veh->selectorScale[j], sel.axis[j] );
		}
		sel.nonNormalizedAxes = qtrue;
		trap_R_AddRefEntityToScene( &sel );
	}
}