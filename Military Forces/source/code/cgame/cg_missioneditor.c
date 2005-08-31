/*
 * $Id: cg_missioneditor.c,v 1.3 2005-08-31 19:20:06 thebjoern Exp $
*/

#include "cg_local.h"


#define MAX_SELECTION_DISTANCE			3000.0f

void ME_DrawVehicle( IGME_vehicle_t* veh );
void ME_CheckForSelection();
void ME_DragSelection( int x, int y );
void ME_DeleteSelection();
void ME_SpawnWaypoint();
IGME_vehicle_t* ME_GetSelectedVehicle();





void ME_KeyEvent(int key, int down)
{
	if( key == K_MOUSE1 && !down ) {
		trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_CGAME );
		cgs.IGME.dragmode = false;
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
		cgs.IGME.copyBufferRouteUsed = true;
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
		cgs.IGME.copyBufferVehicleUsed = true;
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
			veh->waypoints[i].selected = false;
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
		veh->selected = false;
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

	cgs.IGME.waypointmode = cgs.IGME.waypointmode ? false : true;

	if( cgs.IGME.waypointmode ) CG_Printf( "Waypoint mode is ON\n" );
	else CG_Printf( "Waypoint mode is OFF\n" );
}

void ME_ToggleWaypointDisplay()
{
	cgs.IGME.showAllWaypoints = cgs.IGME.showAllWaypoints ? false : true;

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
			cgs.IGME.dragmode = true;
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
			veh->waypoints[i-1].active = false;
			veh->waypoints[i-1].selected = false;
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
			veh->waypoints[i+1].active = true;
			veh->waypoints[i+1].selected = false;
			VectorCopy( wpt->origin, veh->waypoints[i+1].origin );
		}
	}
	veh->waypoints[start].active = false;
	veh->waypoints[start].selected = false;
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
				wpt->active = false;
				wpt->selected = false;
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
				veh->active = false;
				veh->selected = false;
				VectorClear( veh->angles );
				VectorClear( veh->origin );
				VectorClear( veh->selectorScale );
				for( j = 0; j < IGME_MAX_WAYPOINTS; ++j ) {
					wpt = &veh->waypoints[j];
					if( !wpt->active ) break; // as they are all in a row
					wpt->active = false;
					wpt->selected = false;
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
		cgs.IGME.vehicles[i].selected = false;
	}
	cgs.IGME.numSelections = 0;
}

void ME_SelectVehicle( IGME_vehicle_t* veh )
{
	int multisel = trap_Key_IsDown(K_CTRL);

	if( !veh ) {
		if( !multisel ) ME_DeselectAll();
		return;
	}

	if( veh->selected ) {
		if( multisel ) {
			veh->selected = false;
			cgs.IGME.numSelections--;
		} else {
			ME_DeselectAll();
			veh->selected = true;
			cgs.IGME.numSelections++;
		}
	} else {
		if( !multisel ) ME_DeselectAll();
		veh->selected = true;
		cgs.IGME.numSelections++;
	}
}

void ME_DeselectAllWaypoints()
{
	IGME_vehicle_t* selveh = ME_GetSelectedVehicle();
	int				i;

	if( !selveh ) return;

	for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
		selveh->waypoints[i].selected = false;
	}
	cgs.IGME.numWptSelections = 0;
}

void ME_SelectWaypoint( IGME_waypoint_t* wpt )
{
	int multisel = trap_Key_IsDown(K_CTRL);

	if( !wpt ) {
		if( !multisel ) ME_DeselectAllWaypoints();
		return;
	}

	if( wpt->selected ) {
		if( multisel ) {
			wpt->selected = false;
			cgs.IGME.numWptSelections--;
		} else {
			ME_DeselectAllWaypoints();
			wpt->selected = true;
			cgs.IGME.numWptSelections++;
		}
	} else {
		if( !multisel ) ME_DeselectAllWaypoints();
		wpt->selected = true;
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
	bool		boxinit = false;
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
			boxinit = true;
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
	int			vertical = trap_Key_IsDown(K_CTRL);
	int			rotate = trap_Key_IsDown(K_SHIFT);

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
			trace_t	tr;
			vec3_t start, end;
			wpt = &veh->waypoints[i];
			if( !wpt->selected ) continue;
			VectorCopy( wpt->origin, start );
			VectorAdd( wpt->origin, forward, end );
			CG_Trace( &tr, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, MASK_ALL );
			VectorCopy( tr.endpos, wpt->origin );
		}
	} else {
		IGME_vehicle_t* veh;
		for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
			trace_t	tr;
			vec3_t start, end;
			veh = &cgs.IGME.vehicles[i];
			if( !veh->selected ) continue;
			VectorCopy( veh->origin, start );
			VectorAdd( veh->origin, forward, end );
			CG_Trace( &tr, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, MASK_ALL );
			VectorCopy( tr.endpos, veh->origin );
		}
	}
}

void ME_SpawnWaypoint()
{
	IGME_vehicle_t* veh = ME_GetVehicleUnderCrosshair();
	IGME_vehicle_t* selveh = ME_GetSelectedVehicle();
	int				i;
	int			insert = trap_Key_IsDown(K_CTRL);
		
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
				selveh->waypoints[i+1].active = true;
				VectorCopy( cg.predictedPlayerState.origin, selveh->waypoints[i+1].origin );
				return;
			}
		} else {
			for( i = 0; i < IGME_MAX_WAYPOINTS; ++i ) {
				if( !selveh->waypoints[i].active ) {
					selveh->waypoints[i].active = true;
					VectorCopy( cg.predictedPlayerState.origin, selveh->waypoints[i].origin );
					return;
				}
			}
		}
	}
}

static IGME_vehicle_t* ME_SpawnVehicleAt( int vehidx, vec3_t pos, vec3_t angles ) 
{
	IGME_vehicle_t*	 veh = 0;
	int i;

	if( cgs.IGME.waypointmode ) {
		CG_Printf( "You are in waypoint mode, spawning vehicles not allowed now!\n" );
		return 0;
	}

	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		if( !cgs.IGME.vehicles[i].active ) {
			veh = &cgs.IGME.vehicles[i];
			break;
		}
	}
	if( !veh ) {
		CG_Printf( "Too many vehicles spawned! Please delete one first!\n" );
		return 0;
	}

	VectorCopy( pos, veh->origin );
	VectorCopy( angles, veh->angles );
	veh->angles[0] = veh->angles[2] = 0;
	veh->vehidx = vehidx;
	veh->active = true;
	veh->selected = false;
	veh->groundInstallation = false;

	for( i = 0; i < 3; ++i ) {
		float wid = availableVehicles[veh->vehidx].maxs[i] - 
					availableVehicles[veh->vehidx].mins[i];
		veh->selectorScale[i] = wid/200.0f;
	}
	return veh;
}

IGME_vehicle_t* ME_SpawnVehicle( int vehidx ) 
{
	// spawns at current player location
	return ME_SpawnVehicleAt( vehidx, 
							  cg.predictedPlayerState.origin,
							  cg.predictedPlayerState.viewangles );
}

static void ME_SpawnGroundInstallationAt( int idx, vec3_t pos, vec3_t angles )
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

	VectorCopy( pos, veh->origin );
	VectorCopy( angles, veh->angles );
	veh->angles[0] = veh->angles[2] = 0;
	veh->vehidx = idx;
	veh->active = true;
	veh->selected = false;
	veh->groundInstallation = true;

	for( i = 0; i < 3; ++i ) {
		float wid = availableGroundInstallations[veh->vehidx].maxs[i] - 
					availableGroundInstallations[veh->vehidx].mins[i];
		veh->selectorScale[i] = wid/200.0f;
	}
}

void ME_SpawnGroundInstallation( int idx )
{
	// spawn at player loc
	ME_SpawnGroundInstallationAt( idx, 
								  cg.predictedPlayerState.origin, 
								  cg.predictedPlayerState.viewangles );
}

void ME_DrawVehicle( IGME_vehicle_t* veh ) 
{
	if( !veh ) return;

	if( veh->groundInstallation ) {
		DrawInfo_GI_t drawInfo;
		memset( &drawInfo, 0, sizeof(drawInfo) );
		VectorCopy( veh->angles, drawInfo.basicInfo.angles );
		VectorCopy( veh->origin, drawInfo.basicInfo.origin );
		AnglesToAxis( veh->angles, drawInfo.basicInfo.axis );
		drawInfo.basicInfo.vehicleIndex = veh->vehidx;
		drawInfo.basicInfo.entityNum = -1;
		drawInfo.turretAngle = 0;
		drawInfo.gunAngle = 10;
		drawInfo.upgrades = availableGroundInstallations[veh->vehidx].upgrades;
		CG_DrawGI(&drawInfo);
	} else {
		switch( availableVehicles[veh->vehidx].cat ) {
		case CAT_PLANE:
			{
				DrawInfo_Plane_t drawInfo;
				memset( &drawInfo, 0, sizeof(drawInfo) );
				VectorCopy( veh->angles, drawInfo.basicInfo.angles );
				VectorCopy( veh->origin, drawInfo.basicInfo.origin );
				AnglesToAxis( veh->angles, drawInfo.basicInfo.axis );
				drawInfo.basicInfo.vehicleIndex = veh->vehidx;
				drawInfo.basicInfo.entityNum = -1;
				drawInfo.basicInfo.usedLoadout = &availableLoadouts[veh->vehidx];
				CG_DrawPlane(&drawInfo);	
			}
			break;
		case CAT_GROUND:
			{
				DrawInfo_GV_t drawInfo;
				memset( &drawInfo, 0, sizeof(drawInfo) );
				VectorCopy( veh->angles, drawInfo.basicInfo.angles );
				VectorCopy( veh->origin, drawInfo.basicInfo.origin );
				AnglesToAxis( veh->angles, drawInfo.basicInfo.axis );
				drawInfo.basicInfo.vehicleIndex = veh->vehidx;
				drawInfo.basicInfo.entityNum = -1;
				drawInfo.basicInfo.usedLoadout = &availableLoadouts[veh->vehidx];
				CG_DrawGV(&drawInfo);	
			}
			break;
		case CAT_BOAT:
			{
				DrawInfo_Boat_t drawInfo;
				memset( &drawInfo, 0, sizeof(drawInfo) );
				VectorCopy( veh->angles, drawInfo.basicInfo.angles );
				VectorCopy( veh->origin, drawInfo.basicInfo.origin );
				AnglesToAxis( veh->angles, drawInfo.basicInfo.axis );
				drawInfo.basicInfo.vehicleIndex = veh->vehidx;
				drawInfo.basicInfo.entityNum = -1;
				drawInfo.basicInfo.usedLoadout = &availableLoadouts[veh->vehidx];
				CG_DrawBoat(&drawInfo);
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
		sel.nonNormalizedAxes = true;
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
			wpt.nonNormalizedAxes = true;
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
			lnk.nonNormalizedAxes = true;
			lnk.customShader = cgs.media.IGME_waypoint2;
			trap_R_AddRefEntityToScene( &lnk );
			VectorCopy( veh->waypoints[k].origin, lastpos );
		}
	}
}

void ME_ExportToScript( const char* scriptname )
{
	char			scriptfile[256];
	char			outstring[1024];
	const char		*info, *mapname;
	fileHandle_t	f;
	static char		buf[MAX_MENUDEFFILE];
	int				i, j;
	IGME_vehicle_t* veh;

	if( !scriptname )
	{
		CG_Printf("Invalid scriptname!\n");
		return;
	}

	// set up mission filename
	info = CG_ConfigString( CS_SERVERINFO );
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf(scriptfile, 255, "missions/%s/%s.mis", mapname, scriptname);

	if( trap_FS_FOpenFile(scriptfile, &f, FS_WRITE) < 0 ) {
		CG_Printf("Unable to write to file %s. Exporting to script cancelled.", scriptfile);
		return;
	}
	
	CG_Printf("Saving to script: %s\n", scriptfile);

	// first line comment
	Com_sprintf( outstring, sizeof(outstring), "//Missionscript for map %s\n\n",mapname );
	trap_FS_Write( outstring, strlen(outstring), f );

	// header
	Com_sprintf( outstring, sizeof(outstring), "Overview\n{\n\tmap\t%s\n", mapname );
	trap_FS_Write( outstring, strlen(outstring), f );

	Com_sprintf( outstring, sizeof(outstring), "\tgameset\t%d\n", cgs.gameset );
	trap_FS_Write( outstring, strlen(outstring), f );

	Com_sprintf( outstring, sizeof(outstring), "\tgametype\t%d\n", cgs.gametype );
	trap_FS_Write( outstring, strlen(outstring), f );

	Com_sprintf( outstring, sizeof(outstring), "\tmission\t%s\n", scriptname );
	trap_FS_Write( outstring, strlen(outstring), f );

	Com_sprintf( outstring, sizeof(outstring), "\tgoal\tSearchAndDestroy\n}\n\n" );
	trap_FS_Write( outstring, strlen(outstring), f );

	// entities and groundinstallations
	Com_sprintf( outstring, sizeof(outstring), "Entities\n{\n", mapname );
	trap_FS_Write( outstring, strlen(outstring), f );
	for( i = 0; i < IGME_MAX_VEHICLES; ++i ) {
		veh = &cgs.IGME.vehicles[i]; 
		if( !veh->active ) continue;
		if( veh->groundInstallation	)
		{
			Com_sprintf( outstring, sizeof(outstring), "\tGroundInstallation\n\t{\n", mapname );
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tIndex\t%d\n", veh->vehidx );
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tName\tSAM Turret\n" );
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tTeam\t?\n" ); // ADD TEAM HERE!
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tOrigin\t%f;%f;%f\n", veh->origin[0],
				veh->origin[1], veh->origin[2]);
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tAngles\t%f;%f;%f\n", veh->angles[0],
				veh->angles[1], veh->angles[2]);
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t}\n\n", mapname );
			trap_FS_Write( outstring, strlen(outstring), f );
		}
		else
		{
			Com_sprintf( outstring, sizeof(outstring), "\tVehicle\n\t{\n", mapname );
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tIndex\t%d\n", veh->vehidx );
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tName\t%s\n", availableVehicles[veh->vehidx].modelName );
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tTeam\t%d\n", availableVehicles[veh->vehidx].team ); 
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tOrigin\t%f;%f;%f\n", veh->origin[0],
				veh->origin[1], veh->origin[2]);
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tAngles\t%f;%f;%f\n", veh->angles[0],
				veh->angles[1], veh->angles[2]);
			trap_FS_Write( outstring, strlen(outstring), f );

			Com_sprintf( outstring, sizeof(outstring), "\t\tWaypoints\n\t\t{\n" );
			trap_FS_Write( outstring, strlen(outstring), f );

			for( j = 0; j < IGME_MAX_WAYPOINTS; ++j ) 
			{
				if( !veh->waypoints[j].active ) break;//as they are in a row
				Com_sprintf( outstring, sizeof(outstring), "\t\t\tOrigin\t%f;%f;%f\n", veh->waypoints[j].origin[0],
					veh->waypoints[j].origin[1], veh->waypoints[j].origin[2]);
				trap_FS_Write( outstring, strlen(outstring), f );

			}

			Com_sprintf( outstring, sizeof(outstring), "\t\t}\n\t}\n\n", mapname );
			trap_FS_Write( outstring, strlen(outstring), f );
		}
	}
	Com_sprintf( outstring, sizeof(outstring), "}\n", mapname );
	trap_FS_Write( outstring, strlen(outstring), f );



	CG_Printf("Successfully saved to script\n");

	trap_FS_FCloseFile(f);







	// also possible to send it to ui like this:
//	trap_SendConsoleCommand("ui_save_to_script whatever");
}


static void ME_SpawnMissionGroundInstallations(mission_groundInstallation_t* gis)
{
	int	i;

	for( i = 0; i < IGME_MAX_VEHICLES/4; ++i )
	{
		if( !gis[i].used ) break;
		ME_SpawnGroundInstallationAt(gis[i].index,
									 gis[i].origin,
									 gis[i].angles);
	}
}


static void ME_SpawnMissionVehicles(mission_vehicle_t* vehs)
{
	int	i, j, k = 0;
	IGME_vehicle_t* veh;

	for( i = 0; i < IGME_MAX_VEHICLES/4; ++i )
	{
		if( !vehs[i].used ) break;
		veh = ME_SpawnVehicleAt(vehs[i].index,
					 		    vehs[i].origin,
								vehs[i].angles);
		if( !veh ) continue;
		for( j = 0; j < IGME_MAX_WAYPOINTS; ++j )
		{
			if( vehs[i].waypoints[j].used )
			{
				veh->waypoints[k].active = true;
				veh->waypoints[k].selected = false;
				VectorCopy( vehs[i].waypoints[j].origin, veh->waypoints[k].origin );
				k++;
			}
		}
	}

}


static bool ME_LoadOverviewAndEntities( char *filename,
										mission_overview_t* overview,
										mission_vehicle_t* vehs, 
										mission_groundInstallation_t* gis)
{
	int					len;
	fileHandle_t		f;
	char				inbuffer[MAX_MISSION_TEXT];

	// open the file, fill it into buffer and close it, afterwards parse it
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( !f ) 
	{
		Com_Printf( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return false;
	}
	if ( len >= MAX_MISSION_TEXT ) 
	{
		Com_Printf( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_MISSION_TEXT ) );
		trap_FS_FCloseFile( f );
		return false;
	}

	trap_FS_Read( inbuffer, len, f );
	inbuffer[len] = 0;
	trap_FS_FCloseFile( f );

	Com_Printf( va(S_COLOR_GREEN "Successfully opened mission script: %s\n", filename) );

	MF_ParseMissionScripts(inbuffer, overview, vehs, gis);

	return true;
}

void ME_ImportScript( const char* scriptname )
{
	char				filename[256];
	mission_overview_t	overview;
	mission_vehicle_t	vehicles[IGME_MAX_VEHICLES/4];
	mission_groundInstallation_t installations[IGME_MAX_VEHICLES/4];
	const char			*info, *mapname;

	memset(&overview, 0, sizeof(overview));
	memset(&vehicles[0], 0, sizeof(vehicles));
	memset(&installations[0], 0, sizeof(installations));

	// set up mission filename
	info = CG_ConfigString( CS_SERVERINFO );
	mapname = Info_ValueForKey( info, "mapname" );
	Com_sprintf(filename, 255, "missions/%s/%s.mis", mapname, scriptname);

	if( !ME_LoadOverviewAndEntities(filename, &overview, vehicles, installations) )
	{
		Com_Printf( "Unable to load script\n" );
		return;
	}

	// clean out old stuff
	memset( &cgs.IGME, 0, sizeof(cgs.IGME) );

	// spawn new stuff
	ME_SpawnMissionVehicles(vehicles);
	ME_SpawnMissionGroundInstallations(installations);

//	trap_Printf( va("Loaded: %s\n", inbuffer) );
}

// loads up initial mission (specified in mf_mission)
void ME_Init_MissionEditor()
{
	const char	*info;
	const char	*scriptname;

	info = CG_ConfigString( CS_SERVERINFO );
	scriptname = Info_ValueForKey( info, "mf_mission" );

	ME_ImportScript(scriptname);
}


