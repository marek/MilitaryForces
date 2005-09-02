/*
 * $Id: bg_local.h,v 1.3 2005-09-02 08:00:21 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_local.h -- local definitions for the bg (both games) files

#ifndef __BG_LOCAL_H__
#define __BG_LOCAL_H__

#define	OVERCLIP		1.001f

// all of the locals will be zeroed before each
// pmove, just to make damn sure we don't have
// any differences when running on client or server
typedef struct {
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
} pml_t;

extern	pmove_t		*pm;
extern	pml_t		pml;

// movement parameters
extern	float	pm_stopspeed;
extern	float	pm_wadeScale;

extern	float	pm_accelerate;
extern	float	pm_airaccelerate;
extern	float	pm_flyaccelerate;

extern	float	pm_friction;
extern	float	pm_waterfriction;
extern	float	pm_flightfriction;

extern	int		c_pmove;

void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce );
void PM_AddTouchEnt( int entityNum );
void PM_AddEvent( int newEvent );

// MFQ3
void PM_PlaneMove( void );
void PM_PlaneMoveAdvanced( void );
void PM_Toggle_Bay();
void PM_GroundVehicleMove( void );
void PM_HeloMove( void );
void PM_LQMMove( void );
void PM_BoatMove( void );

bool	PM_SlideMove( bool gravity );
void		PM_StepSlideMove( bool gravity );


#endif // __BG_LOCAL_H__