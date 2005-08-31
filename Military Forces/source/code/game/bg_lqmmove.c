/*
 * $Id: bg_lqmmove.c,v 1.2 2005-08-31 19:20:06 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_local.h"
#include <algorithm>

extern pmove_t		*pm;
extern pml_t		pml;


#define	MIN_WALK_NORMAL	0.4f
#define	JUMP_VELOCITY	370/3;
#define	STEPSIZE		18*LQM_SCALE

/*
===================
PM_LQMMove

===================
*/
bool	PM_SlideMove_LQM();
void	PM_StepSlideMove_LQM();


void PM_LQMGroundTrace( void )
{
	trace_t		trace;
	vec3_t		end;

	// set the height

	VectorSet( end, pm->ps->origin[0], (float)pm->ps->origin[1], (float)pm->ps->origin[2] + (float)pm->mins[2] + 2);

	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, end, pm->ps->clientNum, MASK_SOLID);
	pml.groundTrace = trace;

	// if the trace didn't hit anything, we are in free fall
	if ( trace.fraction == 1.0 ) {
		// go into jump animation
		if(pm->ps->velocity[2] <= 0) {
			pm->ps->vehicleAnim |= A_LQM_JUMP;
			pm->ps->ONOFF |= OO_STALLED;	// start landing
		}

		pml.groundPlane = false;
		pml.walking = false;
		return;
	}

	// check if getting thrown off the ground
	if ( pm->ps->velocity[2] > 0 && DotProduct( pm->ps->velocity, trace.plane.normal ) > 10 ) {
		
		// go into jump animation
		pm->ps->vehicleAnim |= A_LQM_JUMP;

		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = false;
		pml.walking = false;
		return;
	}
	
	// slopes that are too steep will not be considered onground
	if ( trace.plane.normal[2] < MIN_WALK_NORMAL ) {
		pm->ps->groundEntityNum = ENTITYNUM_NONE;
		pml.groundPlane = true;
		pml.walking = false;
		return;
	}

	pml.groundPlane = true;
	pml.walking = true;

		
}

void PM_LQMMove( void ) 
{
    vec3_t	viewdir;
    bool	dead = (pm->ps->stats[STAT_HEALTH] <= 0);
	bool	verydead = (pm->ps->stats[STAT_HEALTH] <= GIB_HEALTH);
	int		anim = 0;
	int		maxspeed = availableVehicles[pm->vehicle].maxspeed;
	int		maxspeed2 = sqrt(((float)maxspeed*maxspeed/2));
	int		speedtemp;
	vec3_t	liftvel, forwardvel, rightvel, deltavel;

	if( verydead ) return;

	// clear FX
	pm->ps->ONOFF &= ~OO_VAPOR;

	// Clear vectors
	VectorClear(forwardvel);
	VectorClear(liftvel);
	VectorClear(rightvel);
	VectorClear(deltavel);

    // local vectors
    VectorCopy( pm->ps->viewangles, viewdir );

	// set yaw to 0 <= x <= 360
	viewdir[YAW] = AngleMod( viewdir[YAW] );

	// Copy curret animations
	anim = pm->ps->vehicleAnim;

    if( dead ) {
		pm->ps->vehicleAnim = A_LQM_DIE;
		VectorClear(pm->ps->velocity);
		PM_SlideMove_LQM();
		return;
    }

	// ground movement
	if( pm->ps->ONOFF & OO_LANDED ) {
		
	}
	// air movement
	else {
		liftvel[2] = pm->ps->velocity[2];

		// Crouch/Jump Movement
		if( pm->cmd.buttons & BUTTON_INCREASE ) {
			anim &= ~A_LQM_CROUCH;
			anim |= A_LQM_JUMP;
			maxspeed *= (float)0.5;
			maxspeed2 *= (float)0.5;
			if(liftvel[2] == 0 && !(pm->ps->ONOFF & OO_STALLED))
				liftvel[2] += JUMP_VELOCITY;
		} else if( pm->cmd.buttons & BUTTON_DECREASE ) {
			anim &= ~A_LQM_JUMP;
			anim |= A_LQM_CROUCH;
			maxspeed *= (float)0.5;
			maxspeed2 *= (float)0.5;
		} else {
			if(!((anim & A_LQM_EJECT) && (anim & A_LQM_CROUCH)))
				anim &= ~A_LQM_CROUCH;
		}

		// Forward Movement
		if(pm->cmd.forwardmove != 0) {
			speedtemp = pm->cmd.rightmove != 0 ? maxspeed2 : maxspeed;
			// Forward speed
			VectorCopy(viewdir, forwardvel);
			forwardvel[PITCH] = forwardvel[ROLL] = 0;				// Don't let vehicle angels effect verticle acceleration
			AngleVectors(forwardvel, forwardvel, NULL, NULL );
			if(pm->cmd.forwardmove > 0) {
				anim |= A_LQM_FORWARD;
				anim &= ~A_LQM_BACKWARD;
				VectorScale(forwardvel, speedtemp, forwardvel);
			} else {
				anim |= A_LQM_BACKWARD;
				anim &= ~A_LQM_FORWARD;
				VectorScale(forwardvel, -speedtemp, forwardvel);
			}
		} else
			anim &= ~(A_LQM_FORWARD|A_LQM_BACKWARD);

		// Left/Right Movement
		if(pm->cmd.rightmove != 0) {
			speedtemp = pm->cmd.forwardmove != 0 ? maxspeed2 : maxspeed;
			// Right speed
			VectorCopy(viewdir,  rightvel);
			rightvel[PITCH] = rightvel[ROLL] = 0;
			AngleVectors( rightvel,  NULL, rightvel, NULL );
			if(pm->cmd.rightmove > 0) {
				anim |= A_LQM_RIGHT;
				anim &= ~A_LQM_LEFT;
				VectorScale( rightvel, speedtemp, rightvel );
			} else {
				anim |= A_LQM_LEFT;
				anim &= ~A_LQM_RIGHT;
				VectorScale( rightvel, -speedtemp, rightvel );
			}
		} else
			anim &= ~(A_LQM_LEFT|A_LQM_RIGHT);

		
		/* // Parachute?
		if(pm->cmd.buttons & BUTTON_BRAKE)
			anim |= A_LQM_CROUCH;
		else
			anim &= ~A_LQM_CROUCH;*/

		VectorAdd(forwardvel, rightvel, deltavel);
		VectorAdd(liftvel, deltavel, deltavel);
		// Gravity
		deltavel[2] = std::max(deltavel[2]-DEFAULT_GRAVITY*pml.frametime ,-DEFAULT_GRAVITY);
    }
	// return angles
	VectorCopy( viewdir, pm->ps->vehicleAngles );

	// Return anim
	pm->ps->vehicleAnim = anim;

	// Set velocity
	VectorCopy(deltavel, pm->ps->velocity);

	// speed
	pm->ps->speed = VectorLength(pm->ps->velocity);

	// Move the lqm
	PM_LQMGroundTrace();
	if(pml.walking) {
		pm->ps->velocity[2] = 0;
		pm->ps->ONOFF &= ~OO_STALLED;	// Done landing
		if((anim & A_LQM_CROUCH) && (anim & A_LQM_EJECT)) 
			pm->ps->vehicleAnim &= ~A_LQM_JUMP;
	}
	PM_StepSlideMove_LQM();
}

/*
===============
PM_AddTouchEnt_LQM
===============
*/
static void PM_AddTouchEnt_LQM( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
		if( pm->ps->pm_type != PM_VEHICLE )
		    return;
	}
	if ( pm->numtouch == MAXTOUCH ) {
		return;
	}

	// see if it is already added
	for ( i = 0 ; i < pm->numtouch ; i++ ) {
		if ( pm->touchents[ i ] == entityNum ) {
			return;
		}
	}

	// add it
	pm->touchents[pm->numtouch] = entityNum;
	pm->numtouch++;
}


/*
==================
PM_SlideMove_Plane

Returns true if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5
bool	PM_SlideMove_LQM() {
	int			bumpcount, numbumps;
	vec3_t		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	vec3_t		primal_velocity;
	vec3_t		clipVelocity;
	int			i, j, k;
	trace_t	trace;
	vec3_t		end;
	float		time_left;
	float		into;
	vec3_t		endVelocity;
	vec3_t		endClipVelocity;
	
	numbumps = 4;

	VectorCopy (pm->ps->velocity, primal_velocity);

	time_left = pml.frametime;

	// never turn against the ground plane
	if ( pml.groundPlane ) {
		numplanes = 1;
		VectorCopy( pml.groundTrace.plane.normal, planes[0] );
	} else {
		numplanes = 0;
	}

	// never turn against original velocity
	VectorNormalize2( pm->ps->velocity, planes[numplanes] );
	numplanes++;

	for ( bumpcount=0 ; bumpcount < numbumps ; bumpcount++ ) {

		// calculate position we are trying to move to
		VectorMA( pm->ps->origin, time_left, pm->ps->velocity, end );

		// see if we can make it there
		pm->trace ( &trace, 
					pm->ps->origin, 
					pm->mins, 
					pm->maxs, 
					end, 
					pm->ps->clientNum, 
					pm->tracemask);

		if (trace.allsolid) {

			pm->trace ( &trace, 
					pm->ps->origin, 
					0, 
					0, 
					end, 
					pm->ps->clientNum, 
					pm->tracemask);

			if( trace.allsolid ) {
				// entity is completely trapped in another solid
				pm->ps->velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
				return true;
			}
		}

		if (trace.fraction > 0) {
			// actually covered some distance
			VectorCopy (trace.endpos, pm->ps->origin);
		}

		if (trace.fraction == 1) {
			 break;		// moved the entire distance
		}

		// save entity for contact
		if(	trace.entityNum != ENTITYNUM_WORLD ||
			pm->ps->stats[STAT_HEALTH] <= 0 ||
			(trace.entityNum == ENTITYNUM_WORLD &&
//				!(trace.surfaceFlags & SURF_NOIMPACT) &&
			!(trace.surfaceFlags & SURF_SKY)) ) {
			PM_AddTouchEnt_LQM( trace.entityNum );
		}

		time_left -= time_left * trace.fraction;

		if (numplanes >= MAX_CLIP_PLANES) {
			// this shouldn't really happen
			VectorClear( pm->ps->velocity );
			return true;
		}

		//
		// if this is the same plane we hit before, nudge velocity
		// out along it, which fixes some epsilon issues with
		// non-axial planes
		//
		for ( i = 0 ; i < numplanes ; i++ ) {
			if ( DotProduct( trace.plane.normal, planes[i] ) > 0.99 ) {
				VectorAdd( trace.plane.normal, pm->ps->velocity, pm->ps->velocity );
				break;
			}
		}
		if ( i < numplanes ) {
			continue;
		}
		VectorCopy (trace.plane.normal, planes[numplanes]);
		numplanes++;

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		for ( i = 0 ; i < numplanes ; i++ ) {
			into = DotProduct( pm->ps->velocity, planes[i] );
			if ( into >= 0.1 ) {
				continue;		// move doesn't interact with the plane
			}

			// see how hard we are hitting things
			if ( -into > pml.impactSpeed ) {
				pml.impactSpeed = -into;
			}

			// slide along the plane
			PM_ClipVelocity (pm->ps->velocity, planes[i], clipVelocity, OVERCLIP );

			// slide along the plane
			PM_ClipVelocity (endVelocity, planes[i], endClipVelocity, OVERCLIP );

			// see if there is a second plane that the new move enters
			for ( j = 0 ; j < numplanes ; j++ ) {
				if ( j == i ) {
					continue;
				}
				if ( DotProduct( clipVelocity, planes[j] ) >= 0.1 ) {
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				PM_ClipVelocity( clipVelocity, planes[j], clipVelocity, OVERCLIP );
				PM_ClipVelocity( endClipVelocity, planes[j], endClipVelocity, OVERCLIP );

				// see if it goes back into the first clip plane
				if ( DotProduct( clipVelocity, planes[i] ) >= 0 ) {
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, pm->ps->velocity );
				VectorScale( dir, d, clipVelocity );

				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, endVelocity );
				VectorScale( dir, d, endClipVelocity );

				// see if there is a third plane the the new move enters
				for ( k = 0 ; k < numplanes ; k++ ) {
					if ( k == i || k == j ) {
						continue;
					}
					if ( DotProduct( clipVelocity, planes[k] ) >= 0.1 ) {
						continue;		// move doesn't interact with the plane
					}

					// stop dead at a tripple plane interaction
					VectorClear( pm->ps->velocity );
					return true;
				}
			}

			// if we have fixed all interactions, try another move
			VectorCopy( clipVelocity, pm->ps->velocity );
			VectorCopy( endClipVelocity, endVelocity );
			break;
		}
	}

	// don't change velocity if in a timer (FIXME: is this correct?)
	if ( pm->ps->pm_time ) {
		VectorCopy( primal_velocity, pm->ps->velocity );
	}

	return ( bumpcount != 0 );
	return 0;
}

void PM_StepSlideMove_LQM() {
	vec3_t		start_o, start_v;
	vec3_t		down_o, down_v;
	trace_t		trace;
	vec3_t		up, down;
	float		stepSize;

	VectorCopy (pm->ps->origin, start_o);
	VectorCopy (pm->ps->velocity, start_v);

	if ( PM_SlideMove_LQM() == 0 )
		return;		// we got exactly where we wanted to go first try	

	VectorCopy(start_o, down);
	down[2] -= STEPSIZE;
	pm->trace (&trace, start_o, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
	VectorSet(up, 0, 0, 1);
	// never step up when you still have up velocity
	if ( pm->ps->velocity[2] > 0 && (trace.fraction == 1.0 || DotProduct(trace.plane.normal, up) < 0.7))
		return;

	VectorCopy (pm->ps->origin, down_o);
	VectorCopy (pm->ps->velocity, down_v);

	VectorCopy (start_o, up);
	up[2] += STEPSIZE;

	// test the player position if they were a stepheight higher
	pm->trace (&trace, start_o, pm->mins, pm->maxs, up, pm->ps->clientNum, pm->tracemask);
	if ( trace.allsolid )
		return;		// can't step up

	stepSize = trace.endpos[2] - start_o[2];
	// try slidemove from this position
	VectorCopy (trace.endpos, pm->ps->origin);
	VectorCopy (start_v, pm->ps->velocity);

	PM_SlideMove_LQM();

	// push down the final amount
	VectorCopy (pm->ps->origin, down);
	down[2] -= stepSize;
	pm->trace (&trace, pm->ps->origin, pm->mins, pm->maxs, down, pm->ps->clientNum, pm->tracemask);
	if ( !trace.allsolid )
		VectorCopy (trace.endpos, pm->ps->origin);
	if ( trace.fraction < 1.0 )
		PM_ClipVelocity( pm->ps->velocity, trace.plane.normal, pm->ps->velocity, OVERCLIP );

}
