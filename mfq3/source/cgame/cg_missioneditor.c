/*
 * $Id: cg_missioneditor.c,v 1.6 2002-06-16 21:36:28 thebjoern Exp $
*/

#include "cg_local.h"


#define MAX_SELECTION_DISTANCE			3000.0f


extern char *plane_tags[BP_PLANE_MAX_PARTS];
extern char *gv_tags[BP_GV_MAX_PARTS];
extern char *boat_tags[BP_BOAT_MAX_PARTS];
extern char *gi_tags[BP_GI_MAX_PARTS];
extern char *engine_tags[3];

void ME_DrawVehicle( IGME_vehicle_t* veh );
void ME_CheckForSelection();
void ME_DragSelection( int x, int y );
void ME_DeleteSelection();
void ME_SpawnWaypoint();
IGME_vehicle_t* ME_GetSelectedVehicle();





void ME_KeyEvent(int key, qboolean down)
{
	if( key == K_MOUSE1 && !down ) {
		trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_CGAME );
		cgs.IGME.dragmode = qfalse;
	}
}

void ME_MouseEvent(int x, int y)
{
	if( cgs.IGME.dragmode ) {
		ME_DragSelection(x, y);
	}
}

void ME_CopySelection()
{
	IGME_vehicle_t* veh = 0;
	int				i;

	if( cgs.IGME.numSelections > 1 ) {
		CG_Printf( "Can only copy a SINGLE vehicle!\n" );
		return;
	} else if( cgs.IGME.numSelections < 1 ) {
		CG_Printf( "Select object to copy!\n" );
		return;
	}

	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		if( cgs.IGME.vehicles[i].selected ) {
			veh = &cgs.IGME.vehicles[i];
			break;
		}
	}

	if( !veh ) return;

	if( cgs.IGME.waypointmode ) {
		for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
			cgs.IGME.copyBufferRoute[i].active = veh->waypoints[i].active;
			cgs.IGME.copyBufferRoute[i].selected = veh->waypoints[i].selected;
			VectorCopy( veh->waypoints[i].origin, cgs.IGME.copyBufferRoute[i].origin );
		}	
		cgs.IGME.copyBufferRouteUsed = qtrue;
	} else {
		cgs.IGME.copyBufferVehicle.active = veh->active;
		VectorCopy( veh->angles, cgs.IGME.copyBufferVehicle.angles );
		VectorClear( cgs.IGME.copyBufferVehicle.origin );
		cgs.IGME.copyBufferVehicle.selected = veh->selected;
		VectorCopy( veh->selectorScale, cgs.IGME.copyBufferVehicle.selectorScale );
		cgs.IGME.copyBufferVehicle.vehidx = veh->vehidx;
		for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
			cgs.IGME.copyBufferVehicle.waypoints[i].active = veh->waypoints[i].active;
			cgs.IGME.copyBufferVehicle.waypoints[i].selected = veh->waypoints[i].selected;
			VectorCopy( veh->waypoints[i].origin, cgs.IGME.copyBufferVehicle.waypoints[i].origin );
		}
		cgs.IGME.copyBufferVehicleUsed = qtrue;
	}
	CG_Printf( "Copied to clipboard\n" );
}

void ME_PasteSelection()
{
	IGME_vehicle_t* veh = 0;
	int				i;

	if( cgs.IGME.waypointmode ) {
		if( !cgs.IGME.copyBufferRouteUsed ) {
			CG_Printf( "Clipboard empty, copy something first!" );
			return;
		}
		veh = ME_GetSelectedVehicle();
		if( !veh ) return;
		for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
			veh->waypoints[i].active = cgs.IGME.copyBufferRoute[i].active;
			veh->waypoints[i].selected = qfalse;
			VectorCopy( cgs.IGME.copyBufferRoute[i].origin, veh->waypoints[i].origin );
		}	
	} else {
		if( !cgs.IGME.copyBufferVehicleUsed ) {
			CG_Printf( "Clipboard empty, copy something first!" );
			return;
		}
		for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
			if( !cgs.IGME.vehicles[i].active ) {
				veh = &cgs.IGME.vehicles[i];
				break;
			}
		}
		if( !veh ) {
			CG_Printf( "No more space for vehicles left! Delete some first!\n" );
			return;
		}
		veh->active = cgs.IGME.copyBufferVehicle.active;
		VectorCopy( cgs.IGME.copyBufferVehicle.angles, veh->angles );
		VectorCopy( cg.predictedPlayerState.origin, veh->origin );
		veh->selected = qfalse;
		VectorCopy( cgs.IGME.copyBufferVehicle.selectorScale, veh->selectorScale );
		veh->vehidx = cgs.IGME.copyBufferVehicle.vehidx;
		for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
			veh->waypoints[i].active = cgs.IGME.copyBufferVehicle.waypoints[i].active;
			veh->waypoints[i].selected = cgs.IGME.copyBufferVehicle.waypoints[i].selected;
			VectorCopy( cgs.IGME.copyBufferVehicle.waypoints[i].origin, veh->waypoints[i].origin );
		}
	}
	CG_Printf( "Pasted\n" );
}

void ME_ToggleWaypointMode()
{
	IGME_vehicle_t* veh = 0;

	if( cgs.IGME.numSelections != 1 ) {
		CG_Printf( "You have to select a single vehicle to enter WAYPOINTMODE\n" );
		return;
	}

	veh = ME_GetSelectedVehicle();
	if( !veh ) return;
	if( veh->groundInstallation ) {
		CG_Printf( "Static units dont really need waypoints, do they ?\n" );
		return;
	}

	cgs.IGME.waypointmode = cgs.IGME.waypointmode ? qfalse : qtrue;

	if( cgs.IGME.waypointmode ) CG_Printf( "Waypoint mode is ON\n" );
	else CG_Printf( "Waypoint mode is OFF\n" );
}

void ME_ToggleWaypointDisplay()
{
	cgs.IGME.showAllWaypoints = cgs.IGME.showAllWaypoints ? qfalse : qtrue;

	if( cgs.IGME.showAllWaypoints ) CG_Printf( "ALL Waypoints are displayed\n" );
	else CG_Printf( "Only Waypoints of SELECTED vehicle are displayed\n" );
}

void CG_Draw_IGME()
{
	int				i;
	usercmd_t		cmd;
	int				cmdNum;
	IGME_vehicle_t* veh;

	// check for selections
	cmdNum = trap_GetCurrentCmdNumber();
	trap_GetUserCmd( cmdNum, &cmd );
	if( trap_Key_IsDown(K_MOUSE1) && !trap_Key_GetCatcher() ) {
		if( (cgs.IGME.waypointmode && cgs.IGME.numWptSelections) ||
			(!cgs.IGME.waypointmode && cgs.IGME.numSelections) ) {
			trap_Key_SetCatcher( KEYCATCH_CGAME );
			cgs.IGME.dragmode = qtrue;
		}
	} else if( (trap_Key_IsDown(K_MOUSE3) || trap_Key_IsDown(K_SPACE)) &&
				cg.time >= cgs.IGME.selectionTime ) {
		ME_CheckForSelection();
		cgs.IGME.selectionTime = cg.time + 250;
	} else if( trap_Key_IsDown(K_BACKSPACE) && !trap_Key_GetCatcher() && cg.time >= cgs.IGME.selectionTime ) {
		ME_DeleteSelection();
		cgs.IGME.selectionTime = cg.time + 250;
	} else if( (cmd.buttons & BUTTON_GEAR) && cg.time >= cgs.IGME.selectionTime ) {
		ME_ToggleWaypointMode();
		cgs.IGME.selectionTime = cg.time + 250;
	} else if( (cmd.buttons & BUTTON_INCREASE) && cgs.IGME.waypointmode && 
				cg.time >= cgs.IGME.selectionTime ) {
		ME_SpawnWaypoint();
		cgs.IGME.selectionTime = cg.time + 250;
	} else if( (cmd.buttons & BUTTON_DECREASE) && cgs.IGME.waypointmode && 
				cg.time >= cgs.IGME.selectionTime ) {
		ME_ToggleWaypointDisplay();
		cgs.IGME.selectionTime = cg.time + 250;
	} else if( !cgs.IGME.dragmode && trap_Key_IsDown(K_CTRL) && trap_Key_IsDown('c') && 
				cg.time >= cgs.IGME.selectionTime ) {
		ME_CopySelection();
		cgs.IGME.selectionTime = cg.time + 500;
	} else if( !cgs.IGME.dragmode && trap_Key_IsDown(K_CTRL) && trap_Key_IsDown('v') &&
				cg.time >= cgs.IGME.selectionTime ) {
		ME_PasteSelection();
		cgs.IGME.selectionTime = cg.time + 500;
	}

	// draw vehicles
	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		veh = &cgs.IGME.vehicles[i]; 
		if( !veh->active ) continue;
		ME_DrawVehicle(veh);
	}
}

void ME_MoveWptsBackInLine(IGME_vehicle_t* veh, int start)
{
	int i;
	IGME_waypoint_t*	wpt = 0;

	if( !veh || start < 0 ) return;

	for( i = start; i < IGME_MAX_WAYPOINTS; ++i ) {
		wpt = &veh->waypoints[i];
		if( !wpt->active ) {
			veh->waypoints[i-1].active = qfalse;
			veh->waypoints[i-1].selected = qfalse;
			VectorClear( veh->waypoints[i-1].origin );
			return;
		}
		veh->waypoints[i-1].active = wpt->active;
		veh->waypoints[i-1].selected = wpt->selected;
		VectorCopy( wpt->origin, veh->waypoints[i-1].origin );
	}
}

void ME_MoveWptsAheadInLine(IGME_vehicle_t* veh, int start)
{
	int i;
	IGME_waypoint_t*	wpt = 0;

	if( !veh || start < 0 ) return;

	// last one should be free so we start one before that
	for( i = IGME_MAX_WAYPOINTS - 2; i >= start; --i ) {
		wpt = &veh->waypoints[i];
		if( wpt->active ) { // last active one
			veh->waypoints[i+1].active = qtrue;
			veh->waypoints[i+1].selected = qfalse;
			VectorCopy( wpt->origin, veh->waypoints[i+1].origin );
		}
	}
	veh->waypoints[start].active = qfalse;
	veh->waypoints[start].selected = qfalse;
	VectorClear( veh->waypoints[start].origin );
}

void ME_DeleteSelection() 
{
	int i;

	if( cgs.IGME.waypointmode ) {
		IGME_vehicle_t*		selveh = ME_GetSelectedVehicle();
		IGME_waypoint_t*	wpt = 0;
		if( !selveh ) return;
		if( !cgs.IGME.numWptSelections ) return;
		for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
			wpt = &selveh->waypoints[i];
			if( !wpt->active ) break; // as they are all in a row
			if( wpt->selected ) {
				wpt->active = qfalse;
				wpt->selected = qfalse;
				VectorClear( wpt->origin );
				ME_MoveWptsBackInLine(selveh, i+1);
				--i;
			}
		}
	} else {
		IGME_vehicle_t*		veh = 0;
		IGME_waypoint_t*	wpt = 0;
		int					j;
		if( !cgs.IGME.numSelections ) return;
		for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
			veh = &cgs.IGME.vehicles[i];
			if( veh->selected ) {
				veh->vehidx = -1;
				veh->active = qfalse;
				veh->selected = qfalse;
				VectorClear( veh->angles );
				VectorClear( veh->origin );
				VectorClear( veh->selectorScale );
				for( j = 0; j < IGME_MAX_WAYPOINTS; ++j ) {
					wpt = &veh->waypoints[j];
					if( !wpt->active ) break; // as they are all in a row
					wpt->active = qfalse;
					wpt->selected = qfalse;
					VectorClear( wpt->origin );
				}
			}
		}
	}
}

void ME_DeselectAll()
{
	int i;
	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		cgs.IGME.vehicles[i].selected = qfalse;
	}
	cgs.IGME.numSelections = 0;
}

void ME_SelectVehicle( IGME_vehicle_t* veh )
{
	qboolean multisel = trap_Key_IsDown(K_CTRL);

	if( !veh ) {
		if( !multisel ) ME_DeselectAll();
		return;
	}

	if( veh->selected ) {
		if( multisel ) {
			veh->selected = qfalse;
			cgs.IGME.numSelections--;
		} else {
			ME_DeselectAll();
			veh->selected = qtrue;
			cgs.IGME.numSelections++;
		}
	} else {
		if( !multisel ) ME_DeselectAll();
		veh->selected = qtrue;
		cgs.IGME.numSelections++;
	}
}

void ME_DeselectAllWaypoints()
{
	IGME_vehicle_t* selveh = ME_GetSelectedVehicle();
	int				i;

	if( !selveh ) return;

	for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
		selveh->waypoints[i].selected = qfalse;
	}
	cgs.IGME.numWptSelections = 0;
}

void ME_SelectWaypoint( IGME_waypoint_t* wpt )
{
	qboolean multisel = trap_Key_IsDown(K_CTRL);

	if( !wpt ) {
		if( !multisel ) ME_DeselectAllWaypoints();
		return;
	}

	if( wpt->selected ) {
		if( multisel ) {
			wpt->selected = qfalse;
			cgs.IGME.numWptSelections--;
		} else {
			ME_DeselectAllWaypoints();
			wpt->selected = qtrue;
			cgs.IGME.numWptSelections++;
		}
	} else {
		if( !multisel ) ME_DeselectAllWaypoints();
		wpt->selected = qtrue;
		cgs.IGME.numWptSelections++;
	}
}

IGME_vehicle_t* ME_GetVehicleUnderCrosshair()
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
		if( veh->groundInstallation ) {
			VectorAdd( availableGroundInstallations[veh->vehidx].mins, veh->origin, mins );
			VectorAdd( availableGroundInstallations[veh->vehidx].maxs, veh->origin, maxs );
		} else {
			VectorAdd( availableVehicles[veh->vehidx].mins, veh->origin, mins );
			VectorAdd( availableVehicles[veh->vehidx].maxs, veh->origin, maxs );
		}
		MakeBoxFromExtents(&bbox, mins, maxs, veh->angles);
		if( RayIntersectBox(&ray, &bbox) ) {
			closestDist = dist;
			selveh = veh;
		} 
	}
	return selveh;
}

IGME_waypoint_t* ME_GetWaypointUnderCrosshair()
{
	vec3_t			mins, maxs, dir;
	float			dist, closestDist = MAX_SELECTION_DISTANCE;
	IGME_waypoint_t* selwpt = 0;
	IGME_waypoint_t* wpt = 0;
	box3_t			bbox;
	ray3_t			ray;
	int				i;
	IGME_vehicle_t* selveh = ME_GetSelectedVehicle();
	vec3_t			wmins = { -25, -25, -25 },
					wmaxs = { 25, 25, 25 };

	if( !selveh ) return 0;

	VectorCopy( cg.predictedPlayerState.origin, ray.start );
	AngleVectors( cg.predictedPlayerState.viewangles, ray.dir, 0, 0 );
	VectorScale( ray.dir, MAX_SELECTION_DISTANCE, ray.dir );

	for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
		wpt = &selveh->waypoints[i];
		// not active
		if( !wpt->active ) continue;
		VectorSubtract( wpt->origin, ray.start, dir );
		dist = VectorNormalize( dir );
		// too far away
		if( dist > closestDist ) continue;
		VectorAdd( wmins, wpt->origin, mins );
		VectorAdd( wmaxs, wpt->origin, maxs );
		MakeBoxFromExtents(&bbox, mins, maxs, vec3_origin);
		if( RayIntersectBox(&ray, &bbox) ) {
			closestDist = dist;
			selwpt = wpt;
		} 
	}
	return selwpt;
}

void ME_CheckForSelection()
{
	if( cgs.IGME.waypointmode ) {
		ME_SelectWaypoint(ME_GetWaypointUnderCrosshair());
	} else {
		ME_SelectVehicle(ME_GetVehicleUnderCrosshair());
	}
}

// returns first selected vehicle
IGME_vehicle_t* ME_GetSelectedVehicle()
{
	int				i;
	IGME_vehicle_t* veh = 0;

	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		if( cgs.IGME.vehicles[i].selected ) {
			veh = &cgs.IGME.vehicles[i];
			break;
		}
	}
	return veh;
}

// returns first selected waypoint
int ME_GetSelectedWaypointNumber()
{
	int				i;
	IGME_vehicle_t* selveh = ME_GetSelectedVehicle();

	if( !selveh ) return 0;

	for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
		if( selveh->waypoints[i].selected ) {
			return i;
		}
	}
	return -1;
}

void ME_RotateSelection( int delta )
{
	IGME_vehicle_t* veh;
	sbox3_t			box;
	qboolean		boxinit = qfalse;
	vec3_t			mins = { -1, -1, -1 },
					maxs = { 1, 1, 1 };
	vec3_t			center;
	vec3_t			dir, angles;
	int				i;
	float			dist;

	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		veh = &cgs.IGME.vehicles[i];
		if( !veh->selected ) continue;
		if( !boxinit ) {
			VectorAdd( veh->origin, mins, box.mins );
			VectorAdd( veh->origin, maxs, box.maxs );
			boxinit = qtrue;
		} else {
			AddToBox( &box, veh->origin );		
		}
	}	
	BoxCenter( &box, &center );	

	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		veh = &cgs.IGME.vehicles[i];
		if( !veh->selected ) continue;
		// get direction vector and dist
		VectorSubtract( veh->origin, center, dir );
		dist = VectorNormalize(dir);
		vectoangles( dir, angles );
		// rotate
		veh->angles[1] += delta;
		AngleMod( veh->angles[1] );
		angles[1] += delta;
		AngleMod( angles[1] );
		// move object
		AngleVectors(angles, dir, 0, 0);
		VectorScale(dir, dist, dir);
		VectorAdd(center, dir, veh->origin);
	}

//			if( rotate ) {
//				veh->angles[1] -= x;
//				AngleMod( veh->angles[1] );
//			}

}

void ME_DragSelection( int x, int y )
{
	vec3_t			angles, forward, right;
	int				i;
	qboolean		vertical = trap_Key_IsDown(K_CTRL);
	qboolean		rotate = trap_Key_IsDown(K_SHIFT);

	if( vertical ) {
		// move vertically
		VectorSet( forward, 0, 0, 1 );
		VectorScale( forward, -y, forward );
	} else if( rotate ) {
		ME_RotateSelection( -x );
		return;
	} else {
		// move horizontally
		VectorSet( angles, 0, cg.predictedPlayerState.viewangles[1], 0 );
		AngleVectors(  angles, forward, right, 0 );
		VectorScale( forward, -y, forward );
		VectorScale( right, x, right );
		VectorAdd( forward, right, forward );
	}

	if( cgs.IGME.waypointmode ) {
		IGME_vehicle_t* veh = ME_GetSelectedVehicle();
		IGME_waypoint_t* wpt;
		if( !veh ) return;
		for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
			wpt = &veh->waypoints[i];
			if( !wpt->selected ) continue;
			VectorAdd( wpt->origin, forward, wpt->origin );
		}
	} else {
		IGME_vehicle_t* veh;
		for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
			veh = &cgs.IGME.vehicles[i];
			if( !veh->selected ) continue;
			VectorAdd( veh->origin, forward, veh->origin );
		}
	}
}

void ME_SpawnWaypoint()
{
	IGME_vehicle_t* veh = ME_GetVehicleUnderCrosshair();
	IGME_vehicle_t* selveh = ME_GetSelectedVehicle();
	int				i;
	qboolean		insert = trap_Key_IsDown(K_CTRL);
		
	// vehicle following
	if( veh && !veh->selected ) {

	// waypoints
	} else {
		if( selveh->waypoints[IGME_MAX_WAYPOINTS-1].active ) {
			CG_Printf( "Too many waypoints!\n" );
			return;
		}
		if( insert ) {
			if( cgs.IGME.numWptSelections != 1 ) {
				CG_Printf( "Select ONE single waypoint after which to insert!\n" );
				return;
			}
			if( insert ) {
				i = ME_GetSelectedWaypointNumber();
				ME_MoveWptsAheadInLine(selveh, i+1);
				selveh->waypoints[i+1].active = qtrue;
				VectorCopy( cg.predictedPlayerState.origin, selveh->waypoints[i+1].origin );
				return;
			}
		} else {
			for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
				if( !selveh->waypoints[i].active ) {
					selveh->waypoints[i].active = qtrue;
					VectorCopy( cg.predictedPlayerState.origin, selveh->waypoints[i].origin );
					return;
				}
			}
		}
	}
}

void ME_SpawnVehicle( int vehidx ) 
{
	IGME_vehicle_t*	 veh = 0;
	int i;

	if( cgs.IGME.waypointmode ) {
		CG_Printf( "You are in waypoint mode, spawning vehicles not allowed now!\n" );
		return;
	}

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
	veh->groundInstallation = qfalse;

	for( i = 0; i < 3; ++i ) {
		float wid = availableVehicles[veh->vehidx].maxs[i] - 
					availableVehicles[veh->vehidx].mins[i];
		veh->selectorScale[i] = wid/200.0f;
	}
}

void ME_SpawnGroundInstallation( int idx )
{
	IGME_vehicle_t*	 veh = 0;
	int i;

	if( cgs.IGME.waypointmode ) {
		CG_Printf( "You are in waypoint mode, spawning ground installations not allowed now!\n" );
		return;
	}

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
	veh->vehidx = idx;
	veh->active = qtrue;
	veh->selected = qfalse;
	veh->groundInstallation = qtrue;

	for( i = 0; i < 3; ++i ) {
		float wid = availableGroundInstallations[veh->vehidx].maxs[i] - 
					availableGroundInstallations[veh->vehidx].mins[i];
		veh->selectorScale[i] = wid/200.0f;
	}
}

void ME_DrawVehicle( IGME_vehicle_t* veh ) 
{
	if( !veh ) return;

	if( veh->groundInstallation ) {
		refEntity_t part[BP_GI_MAX_PARTS];
		int i;
		for( i = 0; i < BP_GI_MAX_PARTS; i++ ) {
			memset( &part[i], 0, sizeof(part[0]) );	
		}	
		// body
		AnglesToAxis( veh->angles, part[BP_GI_BODY].axis );
		part[BP_GI_BODY].hModel = availableGroundInstallations[veh->vehidx].handle[BP_GI_BODY];		
		VectorCopy( veh->origin, part[BP_GI_BODY].origin );	
		VectorCopy( veh->origin, part[BP_GI_BODY].oldorigin);
		trap_R_AddRefEntityToScene( &part[BP_GI_BODY] );
		// other parts
		for( i = 1; i < BP_GI_MAX_PARTS; i++ ) {
			if( (i == BP_GI_UPGRADE && !availableGroundInstallations[veh->vehidx].upgrades) ||
				(i == BP_GI_UPGRADE2 && availableGroundInstallations[veh->vehidx].upgrades < 2 ) ||
				(i == BP_GI_UPGRADE3 && availableGroundInstallations[veh->vehidx].upgrades < 3 ) ) {
				break;;
			}
			part[i].hModel = availableGroundInstallations[veh->vehidx].handle[i];
			if( !part[i].hModel ) continue;
			VectorCopy( veh->origin, part[i].lightingOrigin );
			AxisCopy( axisDefault, part[i].axis );
			if( i == BP_GI_GUNBARREL ) {
				CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_TURRET], 
					availableGroundInstallations[veh->vehidx].handle[BP_GI_TURRET], gi_tags[i] );
			} else if( i == BP_GI_UPGRADE ) {
				CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_GUNBARREL], 
					availableGroundInstallations[veh->vehidx].handle[BP_GI_GUNBARREL], gi_tags[i] );
			} else if( i == BP_GI_UPGRADE2 ) {
				CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_UPGRADE], 
					availableGroundInstallations[veh->vehidx].handle[BP_GI_UPGRADE], gi_tags[i] );
			} else if( i == BP_GI_UPGRADE3 ) {
				CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_UPGRADE2], 
					availableGroundInstallations[veh->vehidx].handle[BP_GI_UPGRADE2], gi_tags[i] );
			} else {
				CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GI_BODY], 
					availableGroundInstallations[veh->vehidx].handle[BP_GI_BODY], gi_tags[i] );
			}
			trap_R_AddRefEntityToScene( &part[i] );
		}
	} else {
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
			{
				refEntity_t part[BP_GV_MAX_PARTS];
				int i;
				for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
					memset( &part[i], 0, sizeof(part[0]) );	
				}	
				// body
				AnglesToAxis( veh->angles, part[BP_GV_BODY].axis );
				part[BP_GV_BODY].hModel = availableVehicles[veh->vehidx].handle[BP_GV_BODY];		
				VectorCopy( veh->origin, part[BP_GV_BODY].origin );	
				VectorCopy( veh->origin, part[BP_GV_BODY].oldorigin);
				trap_R_AddRefEntityToScene( &part[BP_GV_BODY] );
				// other parts
				for( i = 1; i < BP_GV_MAX_PARTS; i++ ) {
					part[i].hModel = availableVehicles[veh->vehidx].handle[i];
					if( !part[i].hModel ) continue;
					VectorCopy( veh->origin, part[i].lightingOrigin );
					AxisCopy( axisDefault, part[i].axis );
					if( i == BP_GV_GUNBARREL ) {
						CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GV_TURRET], 
							availableVehicles[veh->vehidx].handle[BP_GV_TURRET], gv_tags[i] );
					} else {
						CG_PositionRotatedEntityOnTag( &part[i], &part[BP_GV_BODY], 
							availableVehicles[veh->vehidx].handle[BP_GV_BODY], gv_tags[i] );
					}
					trap_R_AddRefEntityToScene( &part[i] );
				}
			}
			break;
		case CAT_BOAT:
			{
				refEntity_t part[BP_BOAT_MAX_PARTS];
				int i;
				for( i = 0; i < BP_BOAT_MAX_PARTS; i++ ) {
					memset( &part[i], 0, sizeof(part[0]) );	
				}	
				// body
				AnglesToAxis( veh->angles, part[BP_BOAT_BODY].axis );
				part[BP_BOAT_BODY].hModel = availableVehicles[veh->vehidx].handle[BP_BOAT_BODY];		
				VectorCopy( veh->origin, part[BP_BOAT_BODY].origin );	
				VectorCopy( veh->origin, part[BP_BOAT_BODY].oldorigin);
				trap_R_AddRefEntityToScene( &part[BP_BOAT_BODY] );
				// other parts
				for( i = 1; i < BP_BOAT_MAX_PARTS; i++ ) {
					part[i].hModel = availableVehicles[veh->vehidx].handle[i];
					if( !part[i].hModel ) continue;
					VectorCopy( veh->origin, part[i].lightingOrigin );
					AxisCopy( axisDefault, part[i].axis );
					if( i == BP_BOAT_GUNBARREL ) {
						CG_PositionRotatedEntityOnTag( &part[i], &part[BP_BOAT_TURRET], 
							availableVehicles[veh->vehidx].handle[BP_BOAT_TURRET], boat_tags[i] );
					} else {
						CG_PositionRotatedEntityOnTag( &part[i], &part[BP_BOAT_BODY], 
							availableVehicles[veh->vehidx].handle[BP_BOAT_BODY], boat_tags[i] );
					}
					trap_R_AddRefEntityToScene( &part[i] );
				}
				// turrets
				for( i = 0; i < 4; ++i ) {
					int j = 2*i;
					// turret
					part[BP_BOAT_TURRET+j].hModel = availableVehicles[veh->vehidx].handle[BP_BOAT_TURRET+j];
					if( !part[BP_BOAT_TURRET+j].hModel ) break;
					VectorCopy( veh->origin, part[BP_BOAT_TURRET+j].lightingOrigin );
					AxisCopy( axisDefault, part[BP_BOAT_TURRET+j].axis );
					CG_PositionRotatedEntityOnTag( &part[BP_BOAT_TURRET+j], &part[BP_BOAT_BODY], 
						availableVehicles[veh->vehidx].handle[BP_BOAT_BODY], boat_tags[BP_BOAT_TURRET+j] );
					VectorCopy (part[BP_BOAT_TURRET+j].origin, part[BP_BOAT_TURRET+j].oldorigin);
					trap_R_AddRefEntityToScene( &part[BP_BOAT_TURRET+j] );
					// gun
					part[BP_BOAT_GUNBARREL+j].hModel = availableVehicles[veh->vehidx].handle[BP_BOAT_GUNBARREL+j];
					VectorCopy( veh->origin, part[BP_BOAT_GUNBARREL+j].lightingOrigin );
					AxisCopy( axisDefault, part[BP_BOAT_GUNBARREL+j].axis );
					CG_PositionRotatedEntityOnTag( &part[BP_BOAT_GUNBARREL+j], &part[BP_BOAT_TURRET+j], 
						availableVehicles[veh->vehidx].handle[BP_BOAT_TURRET+j], boat_tags[BP_BOAT_GUNBARREL+j] );
					VectorCopy (part[BP_BOAT_GUNBARREL+j].origin, part[BP_BOAT_GUNBARREL+j].oldorigin);
					trap_R_AddRefEntityToScene( &part[BP_BOAT_GUNBARREL+j] );
				}
			}
			break;
		case CAT_HELO:
			break;
		case CAT_LQM:
			break;
		}
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
	// draw waypoints
	if( cgs.IGME.waypointmode && (veh->selected || cgs.IGME.showAllWaypoints) ) {
		int k;
		vec3_t lastpos, dir, ang;
		VectorCopy( veh->origin, lastpos );
		for( k = 0; k < IGME_MAX_WAYPOINTS; ++k ) {
			refEntity_t	wpt, lnk;
			int l;
			float dist;
			if( !veh->waypoints[k].active ) break;//as they are in a row
			memset( &wpt, 0, sizeof(wpt) );
			wpt.hModel = cgs.media.IGME_selector;
			VectorCopy( veh->waypoints[k].origin, wpt.origin );
			VectorCopy( wpt.origin, wpt.lightingOrigin );
			VectorCopy( wpt.origin, wpt.oldorigin );
			AxisCopy( axisDefault, wpt.axis );
			for( l = 0; l < 3; ++l ) {
				VectorScale( wpt.axis[l], 0.25f, wpt.axis[l] );
			}
			wpt.nonNormalizedAxes = qtrue;
			if( veh->waypoints[k].selected ) {
				wpt.customShader = cgs.media.IGME_waypoint;
			} else {
				wpt.customShader = cgs.media.IGME_waypoint2;
			}
			trap_R_AddRefEntityToScene( &wpt );
			// draw link
			VectorSubtract( wpt.origin, lastpos, dir );
			dist = VectorNormalize( dir );
			dist *= 0.97f;
			vectoangles( dir, ang );
			memset( &lnk, 0, sizeof(lnk) );
			lnk.hModel = cgs.media.IGME_selector;
			VectorMA( lastpos, dist/2, dir, lnk.origin );
			VectorCopy( lnk.origin, lnk.lightingOrigin );
			VectorCopy( lnk.origin, lnk.oldorigin );
			AnglesToAxis( ang, lnk.axis );
			VectorScale( lnk.axis[0], dist/200.0f, lnk.axis[0] );
			VectorScale( lnk.axis[1], 0.01f, lnk.axis[1] );
			VectorScale( lnk.axis[2], 0.01f, lnk.axis[2] );
			lnk.nonNormalizedAxes = qtrue;
			lnk.customShader = cgs.media.IGME_waypoint2;
			trap_R_AddRefEntityToScene( &lnk );
			VectorCopy( veh->waypoints[k].origin, lastpos );
		}
	}
}




