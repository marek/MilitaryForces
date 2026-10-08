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


// movement parameters




class PlayerMove
{
public:
	void clipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
	void addTouchEnt( int entityNum );
	void addEvent( int newEvent );
	void planeMove( );
	void planeMoveAdvanced( );
	bool toggle_Bay();
	bool togglePressed( int button, int heldFlag );
	bool gearInAirflow() const;
	void groundVehicleMove( );
	void heloMove( );
	void lQMMove( );
	void boatMove( );
	bool slideMove( bool gravity );

public:
	void boat_FuelFlow( int throttle );
	void boatAccelerate();
	void addTouchEnt_Boat( int entityNum );
	void groundVehicle_FuelFlow( int throttle );
	void groundVehicleAccelerate();
	void adjustToTerrain();
	void addTouchEnt_GV( int entityNum );
	void helo_FuelFlow( int throttle );
	void heloAccelerate();
	void addTouchEnt_Helo( int entityNum );
	void addTouchEnt_LQM( int entityNum );
	void plane_FuelFlow( int throttle );
	void planeAccelerate();
	void addTouchEnt_Plane( int entityNum );
	void friction();
	void accelerate( vec3_t wishdir, float wishspeed, float accel );
	float cmdScale( usercmd_t *cmd );
	void flyMove();
	void noclipMove();
	void setWaterLevel();
	void waterEvents();
	void flare();
	void weapon();
	void dropTimers();
	void vehicleBoundingBox();
	void vehicleMove();

	// one move of one player: the entry point is Pmove(), which runs single() for each time slice
	void single( pmove_t *pmove );

	void groundVehicle_Brakes();
	void helo_Brakes();
	void lqmGroundTrace();
	void plane_Brakes();
	bool slideMove_Boat();
	bool slideMove_GV();
	bool slideMove_Helo();
	bool slideMove_LQM();
	bool slideMove_Plane();
	void stepSlideMove_GV();
	void stepSlideMove_LQM();
	void throttle_Down();
	void throttle_Up();
	bool toggle_HeloGear();
	bool toggle_PlaneGear();

	pmove_t	*pm = nullptr;
	pml_t	pml = {};

private:
	static float pm_spectatorfriction;
	static float pm_waterfriction;
	static float pm_friction;
	static float pm_flyaccelerate;
	static float pm_accelerate;
	static float pm_stopspeed;
};

// MFQ3



#endif // __BG_LOCAL_H__