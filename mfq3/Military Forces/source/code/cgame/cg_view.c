/*
 * $Id: cg_view.c,v 1.1 2005-08-22 15:41:16 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"


//============================================================================


/*
=================
CG_CalcVrect

Sets the coordinates of the rendered window
=================
*/
static void CG_CalcVrect (void) {
	int		size;

	// the intermission should allways be full screen
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		size = 100;
	} else {
		// bound normal viewsize
		if (cg_viewsize.integer < 30) {
			trap_Cvar_Set ("cg_viewsize","30");
			size = 30;
		} else if (cg_viewsize.integer > 100) {
			trap_Cvar_Set ("cg_viewsize","100");
			size = 100;
		} else {
			size = cg_viewsize.integer;
		}

	}
	cg.refdef.width = cgs.glconfig.vidWidth*size/100;
	cg.refdef.width &= ~1;

	cg.refdef.height = cgs.glconfig.vidHeight*size/100;
	cg.refdef.height &= ~1;

	cg.refdef.x = (cgs.glconfig.vidWidth - cg.refdef.width)/2;
	cg.refdef.y = (cgs.glconfig.vidHeight - cg.refdef.height)/2;
}

//==============================================================================

/*
===============
CG_OffsetVehicleView

===============
*/
#define	FOCUS_DISTANCE	512
//#define	_VIEW_SETTING_DISPLAY

// MFQ3
static void CG_OffsetVehicleView( qboolean spectator ) 
{
    vec3_t	    forward, right, up;
    vec3_t	    focusAngles;
    vec3_t	    view;
    trace_t	    trace;
    vec3_t		mins, maxs, start; 
	float		dist, height;
	static int	lastVehicle = -1;

#ifdef _VIEW_SETTING_DISPLAY
	char test[ 64 ];
#endif
	// reset 3rd person camera distance and height to vehicle default (only when valid)
	if( cg_vehicle.integer != lastVehicle )
	{
		// reset
		cg_thirdPersonRange.value = availableVehicles[ cg_vehicle.integer ].cam_dist[ CAMERA_V_DEFAULT ];
		cg_thirdPersonHeight.value = availableVehicles[ cg_vehicle.integer ].cam_height[ CAMERA_V_DEFAULT ];

		// save vehicle we reset for
		lastVehicle = cg_vehicle.integer;
	}

	// get vars
	dist = cg_thirdPersonRange.value;
	height = cg_thirdPersonHeight.value;

	// clamp distance
	MF_LimitFloat( &dist,
					availableVehicles[cg_vehicle.integer].cam_dist[ CAMERA_V_MIN ],
					availableVehicles[cg_vehicle.integer].cam_dist[ CAMERA_V_MAX ] );

	// clamp height
	MF_LimitFloat( &height,
					availableVehicles[cg_vehicle.integer].cam_height[ CAMERA_V_MIN ],
					availableVehicles[cg_vehicle.integer].cam_height[ CAMERA_V_MAX ] );

#ifdef _VIEW_SETTING_DISPLAY
	sprintf( test,"d=%f h=%f", dist, height );
	CG_CenterPrint( test, 100, 10 );
#endif

	// update vars
	cg_thirdPersonRange.value = dist;
	cg_thirdPersonHeight.value = height;

	if( spectator ) {
		VectorSet( mins, -8, -8, -8 );
		VectorSet( maxs, 8, 8, 8 );
	} else {
		VectorSet( mins, -2, -2, -2 );
		VectorSet( maxs, 2, 2, 2 );
	}

    VectorCopy( cg.refdefViewAngles, focusAngles );

//    if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= GIB_HEALTH ) 
//    {
//	}

    AngleVectors( focusAngles, forward, right, up);

    VectorCopy( cg.refdef.vieworg, start );
	if( !spectator ) {
		start[2] += availableVehicles[cg_vehicle.integer].maxs[2];
	}
    VectorCopy( cg.refdef.vieworg, view );

	if( !spectator ) {
		VectorMA( view, /*-(availableVehicles[cg_vehicle.integer].cam_dist[ CAMERA_V_DEFAULT ]*/ - dist/*)*/, forward, view );
		view[2] += /*(availableVehicles[cg_vehicle.integer].cam_height[ CAMERA_V_DEFAULT ]*/ + height /*)*/;
	}

    CG_Trace( &trace, start, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

    if ( trace.fraction < 1.0 ) 
    {
		VectorCopy( trace.endpos, view );
		view[2] += (1.0 - trace.fraction) * 32;
		// try another trace to this position, because a tunnel may have the ceiling
		// close enogh that this is poking out

		CG_Trace( &trace, start, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
		VectorCopy( trace.endpos, view );
    }

    VectorCopy( view, cg.refdef.vieworg );
}



// MFQ3
static void CG_OffsetVehicleViewCFV( qboolean spectator ) 
{
    vec3_t	    forward, right, up;
    vec3_t	    focusAngles;
    vec3_t	    view;
    trace_t	    trace;
    vec3_t		mins, maxs, start; 
	float		dist, height;
	static int	lastVehicle = -1;

#ifdef _VIEW_SETTING_DISPLAY
	char test[ 64 ];
#endif
	// reset 3rd person camera distance and height to vehicle default (only when valid)
	if( cg_vehicle.integer != lastVehicle )
	{
		// reset
		cg_thirdPersonRange.value = availableVehicles[ cg_vehicle.integer ].cam_dist[ CAMERA_V_DEFAULT ];
		cg_thirdPersonHeight.value = availableVehicles[ cg_vehicle.integer ].cam_height[ CAMERA_V_DEFAULT ];

		// save vehicle we reset for
		lastVehicle = cg_vehicle.integer;
	}

	// get vars
	dist = cg_thirdPersonRange.value;
	height = cg_thirdPersonHeight.value;

	// clamp distance
	MF_LimitFloat( &dist,
					availableVehicles[cg_vehicle.integer].cam_dist[ CAMERA_V_MIN ],
					availableVehicles[cg_vehicle.integer].cam_dist[ CAMERA_V_MAX ] );

	// clamp height
	MF_LimitFloat( &height,
					availableVehicles[cg_vehicle.integer].cam_height[ CAMERA_V_MIN ],
					availableVehicles[cg_vehicle.integer].cam_height[ CAMERA_V_MAX ] );

#ifdef _VIEW_SETTING_DISPLAY
	sprintf( test,"d=%f h=%f", dist, height );
	CG_CenterPrint( test, 100, 10 );
#endif

	// update vars
	cg_thirdPersonRange.value = dist;
	cg_thirdPersonHeight.value = height;

	if( spectator ) {
		VectorSet( mins, -8, -8, -8 );
		VectorSet( maxs, 8, 8, 8 );
	} else {
		VectorSet( mins, -2, -2, -2 );
		VectorSet( maxs, 2, 2, 2 );
	}

//    VectorCopy( cg.refdefViewAngles, focusAngles );
	VectorCopy( cg.predictedPlayerEntity.currentState.angles, focusAngles );

//    if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= GIB_HEALTH ) 
//    {
//	}

    AngleVectors( focusAngles, forward, right, up);

    VectorCopy( cg.refdef.vieworg, start );
	if( !spectator ) {
		if( cg_advanced.integer )
			VectorMA(start, availableVehicles[cg_vehicle.integer].maxs[2], up, start);
		else
			start[2] += availableVehicles[cg_vehicle.integer].maxs[2];
	}
    VectorCopy( cg.refdef.vieworg, view );

	if( !spectator ) {
		VectorMA( view, /*-(availableVehicles[cg_vehicle.integer].cam_dist[ CAMERA_V_DEFAULT ]*/ - dist/*)*/, forward, view );
		if( cg_advanced.integer )
			VectorMA(view, availableVehicles[cg_vehicle.integer].maxs[2], up, view);
		else
			view[2] += /*(availableVehicles[cg_vehicle.integer].cam_height[ CAMERA_V_DEFAULT ]*/ + height /*)*/;
	}

    CG_Trace( &trace, start, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );

    if ( trace.fraction < 1.0 ) 
    {
		VectorCopy( trace.endpos, view );
		view[2] += (1.0 - trace.fraction) * 32;
		// try another trace to this position, because a tunnel may have the ceiling
		// close enogh that this is poking out

		CG_Trace( &trace, start, mins, maxs, view, cg.predictedPlayerState.clientNum, MASK_SOLID );
		VectorCopy( trace.endpos, view );
    }

    VectorCopy( view, cg.refdef.vieworg );
}


/*
===============
CG_OffsetCockpitView
MFQ3
===============
*/
static void CG_OffsetCockpitView( void ) 
{
    vec3_t	    forward, right, up;
    vec3_t	    view;

    AngleVectors( cg.predictedPlayerEntity.currentState.angles, forward, right, up);

    VectorCopy( cg.refdef.vieworg, view );

	VectorMA( view, availableVehicles[cg_vehicle.integer].cockpitview[0], forward, view );
	VectorMA( view, availableVehicles[cg_vehicle.integer].cockpitview[1], right, view );
	VectorMA( view, availableVehicles[cg_vehicle.integer].cockpitview[2], up, view );

    VectorCopy( view, cg.refdef.vieworg );
}



/*
static void CG_OffsetCockpitView( void ) {
	float			*origin;
	float			*angles;
	float			ratio;
	float			delta;
	vec3_t			predictedVelocity;
	
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		return;
	}

	origin = cg.refdef.vieworg;
	angles = cg.refdefViewAngles;

	// if dead, fix the angle and don't add any kick
	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		angles[ROLL] = 40;
		angles[PITCH] = -15;
		angles[YAW] = 0;
		origin[2] += cg.predictedPlayerState.viewheight;
		return;
	}

	VectorCopy( cg.predictedPlayerEntity.currentState.angles, angles );

	// add angles based on damage kick
	if ( cg.damageTime ) {
		ratio = cg.time - cg.damageTime;
		if ( ratio < DAMAGE_DEFLECT_TIME ) {
			ratio /= DAMAGE_DEFLECT_TIME;
			angles[PITCH] += ratio * cg.v_dmg_pitch;
			angles[ROLL] += ratio * cg.v_dmg_roll;
		} else {
			ratio = 1.0 - ( ratio - DAMAGE_DEFLECT_TIME ) / DAMAGE_RETURN_TIME;
			if ( ratio > 0 ) {
				angles[PITCH] += ratio * cg.v_dmg_pitch;
				angles[ROLL] += ratio * cg.v_dmg_roll;
			}
		}
	}

	// add angles based on velocity
	VectorCopy( cg.predictedPlayerState.velocity, predictedVelocity );

	delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[0]);
	angles[PITCH] += delta * cg_runpitch.value;
	
	delta = DotProduct ( predictedVelocity, cg.refdef.viewaxis[1]);
	angles[ROLL] -= delta * cg_runroll.value;

}
*/

//======================================================================

void CG_ZoomDown_f( void )
{ 
	// locked-out?
	if( cg.zoomLockOut )
	{
		return;
	}
	
	// alter the zoom amount
	cg.zoomAmount *= 2;

	// reset?
	if( cg.zoomAmount > 8 )	// x8 is max zoom
	{
		// off
		cg.zoomed = qfalse;
		cg.zoomAmount = 1;
	}
	else
	{
		// on
		cg.zoomed = qtrue;
	}

	// lock-out
	cg.zoomLockOut = qfalse;
}

void CG_ZoomUp_f( void )
{ 
	// reset lock-out
	cg.zoomLockOut = qfalse;
}

/*
====================
CG_CalcFov

Fixed fov at intermissions, otherwise account for fov variable and zooms.
====================
*/
#define	WAVE_AMPLITUDE	1
#define	WAVE_FREQUENCY	0.4

static int CG_CalcFov( void ) {
	float	x;
	float	phase;
	float	v;
	int		contents;
	float	fov_x, fov_y;
//	float	zoomFov;
//	float	f;
	int		inwater;

//	if ( cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		// if in intermission, use a fixed value
		fov_x = 90;
//	} else {
		// user selectable
//		if ( cgs.dmflags & DF_FIXED_FOV ) {
			// dmflag to prevent wide fov for all clients
//			fov_x = 90;
//		} else {
//			fov_x = cg_fov.value;
//			if ( fov_x < 1 ) {
//				fov_x = 1;
//			} else if ( fov_x > 160 ) {
//				fov_x = 160;
//			}
//		}

		// account for zooms
/*		zoomFov = cg_zoomFov.value;
		if ( zoomFov < 1 ) {
			zoomFov = 1;
		} else if ( zoomFov > 160 ) {
			zoomFov = 160;
		}

		if ( cg.zoomed ) {
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = zoomFov;
			} else {
				fov_x = fov_x + f * ( zoomFov - fov_x );
			}
		} else {
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
			if ( f > 1.0 ) {
				fov_x = fov_x;
			} else {
				fov_x = zoomFov + f * ( fov_x - zoomFov );
			}
		}*/
//	}

	x = cg.refdef.width / tan( fov_x / 360 * M_PI );
	fov_y = atan2( cg.refdef.height, x );
	fov_y = fov_y * 360 / M_PI;

	// warp if underwater
	contents = CG_PointContents( cg.refdef.vieworg, -1 );
	if ( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ){
		phase = cg.time / 1000.0 * WAVE_FREQUENCY * M_PI * 2;
		v = WAVE_AMPLITUDE * sin( phase );
		fov_x += v;
		fov_y -= v;
		inwater = qtrue;
	}
	else {
		inwater = qfalse;
	}


	// set it
	cg.refdef.fov_x = fov_x;
	cg.refdef.fov_y = fov_y;

	if ( !cg.zoomed ) {
		cg.zoomSensitivity = 1;
	} else {
		cg.zoomSensitivity = cg.refdef.fov_y / 75.0;
	}

	return inwater;
}

/*
===============
CG_CalcViewValues
===============
*/

static void CG_CalcMiscViewValues( void )
{
	qboolean noAdjust = qfalse;

	float mult = 0.0f;
	
	// faster?
	if( cg.cameraAdjustCount < 50 )
	{
		mult = 1.0f;
	}
	else
	{
		mult = 2.0f;
	}

	// adjustment active?
	switch( cg.cameraAdjustEnum )
	{
	case CAMADJ_INOUT:
		cg_thirdPersonRange.value += cg.cameraAdjustAmount * mult;
		break;

	case CAMADJ_UPDOWN:
		cg_thirdPersonHeight.value += cg.cameraAdjustAmount * mult;
		break;

	default:
		noAdjust = qtrue;
		break;
	}

	// count up?
	if( !noAdjust )
	{
		cg.cameraAdjustCount++;
	}
	else
	{
		// reset
		cg.cameraAdjustCount = 0;
	}
}

/*
===============
CG_CalcViewValues

Sets cg.refdef view values
===============
*/
static int CG_CalcViewValues( void ) {
	playerState_t	*ps;

	memset( &cg.refdef, 0, sizeof( cg.refdef ) );

	CG_CalcMiscViewValues();

	// strings for in game rendering
	// Q_strncpyz( cg.refdef.text[0], "Park Ranger", sizeof(cg.refdef.text[0]) );
	// Q_strncpyz( cg.refdef.text[1], "19", sizeof(cg.refdef.text[1]) );

	// calculate size of 3D view
	CG_CalcVrect();

	ps = &cg.predictedPlayerState;

	// intermission view
	if ( ps->pm_type == PM_INTERMISSION ) { //|| (ps->pm_flags & PMF_VEHICLESELECT) ) {
		VectorCopy( ps->origin, cg.refdef.vieworg );
		VectorCopy( ps->viewangles, cg.refdefViewAngles );
		AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );
		return CG_CalcFov();
	}

	VectorCopy( ps->origin, cg.refdef.vieworg );
	if( cg_advanced.integer )
	{
		VectorCopy( cg.predictedPlayerEntity.currentState.angles, cg.refdefViewAngles );
	}
	else
	{
		VectorCopy( ps->viewangles, cg.refdefViewAngles );
	}

	// add error decay
	if ( cg_errorDecay.value > 0 ) {
		int		t;
		float	f;

		t = cg.time - cg.predictedErrorTime;
		f = ( cg_errorDecay.value - t ) / cg_errorDecay.value;
		if ( f > 0 && f < 1 ) {
			VectorMA( cg.refdef.vieworg, f, cg.predictedError, cg.refdef.vieworg );
		} else {
			cg.predictedErrorTime = 0;
		}
	}

	if( ps->pm_type == PM_SPECTATOR ) {
	    CG_OffsetVehicleView(qtrue);
	} else if ( cg.renderingThirdPerson ) {
		if( cg_advanced.integer )
			CG_OffsetVehicleViewCFV(qfalse);
		else
			CG_OffsetVehicleView(qfalse);
	} else {
		CG_OffsetCockpitView();
	}

	// position eye reletive to origin
	AnglesToAxis( cg.refdefViewAngles, cg.refdef.viewaxis );

	if ( cg.hyperspace ) {
		cg.refdef.rdflags |= RDF_NOWORLDMODEL | RDF_HYPERSPACE;
	}

	// field of view
	return CG_CalcFov();
}


/*
=====================
CG_AddBufferedSound
=====================
*/
void CG_AddBufferedSound( sfxHandle_t sfx ) {
	if ( !sfx )
		return;
	cg.soundBuffer[cg.soundBufferIn] = sfx;
	cg.soundBufferIn = (cg.soundBufferIn + 1) % MAX_SOUNDBUFFER;
	if (cg.soundBufferIn == cg.soundBufferOut) {
		cg.soundBufferOut++;
	}
}

/*
=====================
CG_PlayBufferedSounds
=====================
*/
static void CG_PlayBufferedSounds( void ) {
	if ( cg.soundTime < cg.time ) {
		if (cg.soundBufferOut != cg.soundBufferIn && cg.soundBuffer[cg.soundBufferOut]) {
			trap_S_StartLocalSound(cg.soundBuffer[cg.soundBufferOut], CHAN_ANNOUNCER);
			cg.soundBuffer[cg.soundBufferOut] = 0;
			cg.soundBufferOut = (cg.soundBufferOut + 1) % MAX_SOUNDBUFFER;
			cg.soundTime = cg.time + 750;
		}
	}
}

//=========================================================================

/*
=================
CG_DrawActiveFrame

Generates and draws a game scene and status information at the given time.
=================
*/
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, qboolean demoPlayback ) {
	int		inwater;

	cg.time = serverTime;
	cg.demoPlayback = demoPlayback;

	// update cvars
	CG_UpdateCvars();

	// if we are only updating the screen as a loading
	// pacifier, don't even try to read snapshots
	if ( cg.infoScreenText[0] != 0 )
	{
		CG_DrawInformation();
		return;
	}

	// any looped sounds will be respecified as entities
	// are added to the render list
	trap_S_ClearLoopingSounds(qfalse);

	// clear all the render lists
	trap_R_ClearScene();

	// set up cg.snap and possibly cg.nextSnap
	CG_ProcessSnapshots();

	// if we haven't received any snapshots yet, all
	// we can draw is the information screen
	if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) ) {
		CG_DrawInformation();
		return;
	}

	// let the client system know what our weapon and zoom settings are
	trap_SetUserCmdValue( cg.weaponSelect, cg.zoomSensitivity );

	// this counter will be bumped for every valid scene we generate
	cg.clientFrame++;

	// update cg.predictedPlayerState
	CG_PredictPlayerState();

	// decide on third person view
	cg.renderingThirdPerson = cg_thirdPerson.integer || (cg.snap->ps.stats[STAT_HEALTH] <= 0);

	// build cg.refdef
	inwater = CG_CalcViewValues();

	// build the render lists
	if ( !cg.hyperspace ) {
		CG_AddPacketEntities();			// adter calcViewValues, so predicted player state is correct
		CG_AddMarks();
		CG_AddLocalEntities();
	}

	// add buffered sounds
	CG_PlayBufferedSounds();

	// finish up the rest of the refdef
	if( cg.testModelEntity.hModel ) {
		CG_AddTestModel();
	}
	if( cg.testVehicleParts[0].hModel ) {
		CG_AddTestVehicle();
	}
	if( cgs.gametype == GT_MISSION_EDITOR ) {
		CG_Draw_IGME();
	}

	cg.refdef.time = cg.time;
	memcpy( cg.refdef.areamask, cg.snap->areamask, sizeof( cg.refdef.areamask ) );

	// update audio positions
	trap_S_Respatialize( cg.snap->ps.clientNum, cg.refdef.vieworg, cg.refdef.viewaxis, inwater );

	// make sure the lagometerSample and frame timing isn't done twice when in stereo
	if ( stereoView != STEREO_RIGHT ) {
		cg.frametime = cg.time - cg.oldTime;
		if ( cg.frametime < 0 ) {
			cg.frametime = 0;
		}
		cg.oldTime = cg.time;
		CG_AddLagometerFrameInfo();
	}
	if (cg_timescale.value != cg_timescaleFadeEnd.value) {
		if (cg_timescale.value < cg_timescaleFadeEnd.value) {
			cg_timescale.value += cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
			if (cg_timescale.value > cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		else {
			cg_timescale.value -= cg_timescaleFadeSpeed.value * ((float)cg.frametime) / 1000;
			if (cg_timescale.value < cg_timescaleFadeEnd.value)
				cg_timescale.value = cg_timescaleFadeEnd.value;
		}
		if (cg_timescaleFadeSpeed.value) {
			trap_Cvar_Set("timescale", va("%f", cg_timescale.value));
		}
	}

	// actually issue the rendering calls
	CG_DrawActive( stereoView );

	if ( cg_stats.integer ) {
		CG_Printf( "cg.clientFrame:%i\n", cg.clientFrame );
	}
}

#define DTOR 0.01745329252

/*
=================
CG_WorldToScreenCoords

Converts a world vector point into screen coords
=================
*/
qboolean CG_WorldToScreenCoords( vec3_t worldPoint, int * pX, int * pY, qboolean virtualXY )
{
	vec3_t v, dp, n, p;
	float tanthetah, tanthetav;
	float d, cv, ch, w, h;
	float zoom = 1.0f;

	// move to local coords
	v[0] = worldPoint[0] - cg.refdef.vieworg[0];
	v[1] = worldPoint[1] - cg.refdef.vieworg[1];
	v[2] = worldPoint[2] - cg.refdef.vieworg[2];

	// apply current view axis
	dp[0] = DotProduct( v, cg.refdef.viewaxis[1] );
	dp[1] = DotProduct( v, cg.refdef.viewaxis[0] );
	dp[2] = DotProduct( v, cg.refdef.viewaxis[2] );

	// don't continue if point is behind out view plane
	if( dp[1] < 0 )
	{
		return qfalse;
	}

	// calc FOV adjustments
	tanthetah = tan( cg.refdef.fov_x * DTOR / 2);
	tanthetav = tan( cg.refdef.fov_y * DTOR / 2);

	// create the normalised vector
	d = zoom / dp[1];
	n[0] = d * dp[0] / tanthetah;
	n[1] = dp[1];
	n[2] = d * dp[2] / tanthetav;

	// work out viewport
	w = cg.refdef.width;
	h = cg.refdef.height;

	// use 640x480 instead of our current viewport?
	if( virtualXY )
	{
		w = 640;
		h = 480;
	}

	cv = h * 0.5f;
	ch = w * 0.5f;
		
	// valid?
	if( n[0] >= -1 && n[0] <= +1 && n[2] >= -1 && n[2] <= +1 )
	{
	   p[0] = ch - w * n[0] / 2;
	   p[1] = cv - h * n[2] / 2;
	   p[2] = 0;

	   // convert 
	   *pX = (int)p[0];
	   *pY = (int)p[1];

	   // on-screen
	   return qtrue;
	}

	// off-screen
	return qfalse;
}
