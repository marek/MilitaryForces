/*
 * $Id: bg_local.h,v 1.3 2005-09-02 08:00:21 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_local.h -- local definitions for the bg (both games) files

#ifndef __BG_LOCAL_H__
#define __BG_LOCAL_H__

inline constexpr float OVERCLIP = 1.001f;

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
struct pml_t {
	vec3_t		forward, right, up;
	float		frametime;

	int			msec;

	bool	walking;
	bool	groundPlane;
	trace_t		groundTrace;

	float		impactSpeed;

	vec3_t		previous_origin;
	vec3_t		previous_velocity;
	int			previous_waterlevel;
};

extern	pmove_t		*pm;
extern	pml_t		pml;

// movement parameters




class PlayerMove
{
public:
	static void clipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
	static void addTouchEnt( int entityNum );
	static void addEvent( int newEvent );
	static void planeMove( );
	static void planeMoveAdvanced( );
	static void toggle_Bay();
	static void groundVehicleMove( );
	static void heloMove( );
	static void lQMMove( );
	static void boatMove( );
	static bool slideMove( bool gravity );

public:
	static void boat_FuelFlow( int throttle );
	static void boatAccelerate();
	static void addTouchEnt_Boat( int entityNum );
	static void groundVehicle_FuelFlow( int throttle );
	static void groundVehicleAccelerate();
	static void adjustToTerrain();
	static void addTouchEnt_GV( int entityNum );
	static void helo_FuelFlow( int throttle );
	static void heloAccelerate();
	static void addTouchEnt_Helo( int entityNum );
	static void addTouchEnt_LQM( int entityNum );
	static void plane_FuelFlow( int throttle );
	static void planeAccelerate();
	static void addTouchEnt_Plane( int entityNum );
	static void friction();
	static void accelerate( vec3_t wishdir, float wishspeed, float accel );
	static float cmdScale( usercmd_t *cmd );
	static void flyMove();
	static void noclipMove();
	static void setWaterLevel();
	static void waterEvents();
	static void flare();
	static void weapon();
	static void dropTimers();
	static void vehicleBoundingBox();
	static void vehicleMove();
};

// MFQ3

void		PM_StepSlideMove( bool gravity );


#endif // __BG_LOCAL_H__