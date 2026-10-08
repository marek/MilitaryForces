/*
 * $Id: bg_slidemove.c,v 1.4 2005-11-21 17:28:20 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_slidemove.c -- part of bg_pmove functionality

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_local.h"

/*

input: origin, velocity, bounds, groundPlane, trace function

output: origin, velocity, impacts, stairup boolean

*/

/*
==================
PlayerMove::slideMove

Returns true if the velocity was clipped in some way
==================
*/
inline constexpr int MAX_CLIP_PLANES = 5;
bool	PlayerMove::slideMove( bool gravity ) {
	int			bumpcount, numbumps;
	Vec3		dir;
	float		d;
	int			numplanes;
	vec3_t		planes[MAX_CLIP_PLANES];
	Vec3		primal_velocity;
	Vec3		clipVelocity;
	int			i, j, k;
	trace_t	trace;
	Vec3		end;
	float		time_left;
	float		into;
	Vec3		endVelocity;
	Vec3		endClipVelocity;

	numbumps = 4;

	VectorCopy (pm->ps->velocity, primal_velocity);

	if ( gravity ) {
		VectorCopy( pm->ps->velocity, endVelocity );
		endVelocity[2] -= DEFAULT_GRAVITY * pml.frametime;// MFQ3 instead pm->ps->gravity
		pm->ps->velocity[2] = ( pm->ps->velocity[2] + endVelocity[2] ) * 0.5;
		primal_velocity[2] = endVelocity[2];
		if ( pml.groundPlane ) {
			// slide along the ground plane
			PlayerMove::clipVelocity (pm->ps->velocity, pml.groundTrace.plane.normal, 
				pm->ps->velocity, OVERCLIP );
		}
	}

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
					pm->tracemask,
					false);

		if (trace.allsolid) {

			pm->trace ( &trace, 
					pm->ps->origin, 
					nullptr, 
					nullptr, 
					end, 
					pm->ps->clientNum, 
					pm->tracemask,
					false);

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
		PlayerMove::addTouchEnt( trace.entityNum );

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
			PlayerMove::clipVelocity (pm->ps->velocity, planes[i], clipVelocity, OVERCLIP );

			// slide along the plane
			PlayerMove::clipVelocity (endVelocity, planes[i], endClipVelocity, OVERCLIP );

			// see if there is a second plane that the New move enters
			for ( j = 0 ; j < numplanes ; j++ ) {
				if ( j == i ) {
					continue;
				}
				if ( DotProduct( clipVelocity, planes[j] ) >= 0.1 ) {
					continue;		// move doesn't interact with the plane
				}

				// try clipping the move to the plane
				PlayerMove::clipVelocity( clipVelocity, planes[j], clipVelocity, OVERCLIP );
				PlayerMove::clipVelocity( endClipVelocity, planes[j], endClipVelocity, OVERCLIP );

				// see if it goes back into the first clip plane
				if ( DotProduct( clipVelocity, planes[i] ) >= 0 ) {
					continue;
				}

				// slide the original velocity along the crease
				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, pm->ps->velocity );
				clipVelocity = dir * ( d );

				CrossProduct (planes[i], planes[j], dir);
				VectorNormalize( dir );
				d = DotProduct( dir, endVelocity );
				endClipVelocity = dir * ( d );

				// see if there is a third plane the the New move enters
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
			endVelocity = endClipVelocity;
			break;
		}
	}

	if ( gravity ) {
		VectorCopy( endVelocity, pm->ps->velocity );
	}

	// don't change velocity if in a timer (FIXME: is this correct?)
	if ( pm->ps->pm_time ) {
		VectorCopy( primal_velocity, pm->ps->velocity );
	}

	return ( bumpcount != 0 );
}


