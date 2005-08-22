/*
 * $Id: bg_boatmove.c,v 1.1 2005-08-22 15:41:17 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_local.h"

extern pmove_t		*pm;
extern pml_t		pml;



/*
===================
PM_Boat_FuelFlow

===================
*/

static void PM_Boat_FuelFlow( int throttle )
{
	int fuelflow = (20 - throttle) * 1000;

	if( pm->cmd.serverTime < pm->ps->timers[TIMER_FUEL] + fuelflow ) {
		return;
	}	

	pm->ps->stats[STAT_FUEL]--;
	if( pm->ps->stats[STAT_FUEL] <= 0 ) {
		pm->ps->throttle = 0;
		return;
	}

	pm->ps->timers[TIMER_FUEL] = pm->cmd.serverTime;
}

/*
===================
PM_BoatAccelerate

===================
*/
static void PM_BoatAccelerate()
{
    float	topspeed = availableVehicles[pm->vehicle].maxspeed;
    float	currspeed = (float)pm->ps->speed/10;
	float	maxthrottle = availableVehicles[pm->vehicle].maxthrottle;
	float	minthrottle = availableVehicles[pm->vehicle].minthrottle;
    float	throttle = pm->ps->fixed_throttle;
    float	accel = availableVehicles[pm->vehicle].accel * pml.frametime;
	float	targetspeed;
		
	if( pm->ps->ONOFF & OO_LANDED ) {

		// check for overriding throttle
		if( pm->cmd.forwardmove > 0 ) throttle = maxthrottle;
		else if( pm->cmd.forwardmove < 0 ) throttle = maxthrottle - minthrottle;

		// set it back
		pm->ps->throttle = throttle;

		// check for reverse
		if( throttle > maxthrottle ) {
			throttle = -(throttle - maxthrottle);
		}
		if( pm->ps->ONOFF & OO_STALLED ) {
			currspeed *= -1;
		}

		// check for fuel
		if( pm->ps->stats[STAT_FUEL] <= 0 ) throttle = 0;

		// what is the speed to accelerate towards (consider handbrake)
		targetspeed = topspeed*throttle/maxthrottle;

		if( currspeed < targetspeed - accel ) currspeed += accel;
		else if( currspeed > targetspeed + accel ) currspeed -= accel;
		else currspeed = targetspeed;

		// for negative speeds we set the flag OO_STALLED as speed is an unsigned int
		if( currspeed < 0 ) {
			currspeed *= -1;
			pm->ps->ONOFF |= OO_STALLED;
		} else {
			pm->ps->ONOFF &= ~OO_STALLED;
		}
	} else { // falling
		currspeed += (50 * pml.frametime);
	}

	// apply speed modifier
    pm->ps->speed = currspeed*10;

	// calculate fuel flow (for fuel flow we need abs value)
	if( throttle < 0 )
	{
		throttle *= -1;
	}

	PM_Boat_FuelFlow( throttle );
}


/*
===================
PM_BoatMove

===================
*/
qboolean	PM_SlideMove_Boat();

void PM_BoatMove( void ) 
{
    vec3_t		viewdir;
    vec3_t		vehdir;
	vec3_t		turretdir;
    vec3_t		diff;
    vec3_t		turnspeed;
	vec3_t		forward, up;
	vec3_t		temp;
    qboolean	dead = (pm->ps->stats[STAT_HEALTH] <= 0);
	int			i;
	float		smove = pm->cmd.rightmove;
	float		turret_yaw = ((float)pm->ps->turretAngle)/10;
	float		gun_pitch = ((float)pm->ps->gunAngle)/10;
	float		speed;
	qboolean	reverse = qfalse;
	float		lean = 0;

	// set speed
	PM_BoatAccelerate();

	// speed
	speed = (float)pm->ps->speed/10;
	VectorCopy( pm->ps->vehicleAngles, vehdir );
	vehdir[0] = 0;
	if( pm->ps->ONOFF & OO_STALLED ) {
		AngleVectors( vehdir, forward, NULL, NULL );
		VectorNegate( forward, forward );
		reverse = qtrue;
	} else {
		AngleVectors( vehdir, forward, NULL, NULL );
	}
	VectorNormalize( pm->ps->velocity );
	VectorAdd( forward, pm->ps->velocity, forward );
	VectorScale( forward, speed, pm->ps->velocity );

	PM_SlideMove_Boat();

	// get the turnspeeds
	for( i = HULL_YAW; i <= GUN_PITCH; i++ ) {
	    turnspeed[i] = availableVehicles[pm->vehicle].turnspeed[i] * pml.frametime;
	}

	// set the hull angle dependent on speed
	if( !reverse ) {
		pm->ps->vehicleAngles[0] = (speed/(float)availableVehicles[pm->vehicle].maxspeed) *
				availableVehicles[pm->vehicle].tailangle;
		if( smove > 0 ) {
			lean = (speed/(float)availableVehicles[pm->vehicle].maxspeed) *
				availableVehicles[pm->vehicle].gearheight;
		} else if( smove < 0 ) {
			lean = -(speed/(float)availableVehicles[pm->vehicle].maxspeed) *
				availableVehicles[pm->vehicle].gearheight;
		}

	}
	if( pm->ps->vehicleAngles[2] != lean ) {
		diff[2] = lean - pm->ps->vehicleAngles[2];
		if( diff[2] < -turnspeed[2] ) {
			pm->ps->vehicleAngles[2] -= turnspeed[2];
		} else if( diff[2] > turnspeed[2] ) {
			pm->ps->vehicleAngles[2] += turnspeed[2];
		} else {
			pm->ps->vehicleAngles[2] += diff[2];
		}			
	}

	// get the actual turret angles
	AngleVectors( pm->ps->vehicleAngles, forward, 0, up );
	RotatePointAroundVector( temp, up, forward, turret_yaw );
	vectoangles( temp, turretdir );
	turretdir[PITCH] += gun_pitch;

//	Com_Printf( "vehd = %.1f %.1f %.1f (%.1f %.1f)\n", pm->ps->vehicleAngles[0], pm->ps->vehicleAngles[1],
//			pm->ps->vehicleAngles[2], turret_yaw, gun_pitch );
//	Com_Printf( "view = %.1f %.1f %.1f\n", pm->ps->viewangles[0], pm->ps->viewangles[1],
//			pm->ps->viewangles[2] );
//	Com_Printf( "turr = %.1f %.1f %.1f\n", turretdir[0], turretdir[1], turretdir[2] );

	// local vectors
    VectorCopy( pm->ps->vehicleAngles, vehdir );
	if( (pm->cmd.buttons & BUTTON_FREELOOK) && !dead ) { // freelook
		VectorCopy( turretdir, viewdir );
	}
	else { // normal - turret follows camera
		VectorCopy( pm->ps->viewangles, viewdir );
	}

	// set to 0<=x<=360
	viewdir[YAW] = AngleMod( viewdir[YAW] );
	vehdir[YAW] = AngleMod( vehdir[YAW] );
	turretdir[YAW] = AngleMod( turretdir[YAW] );
	viewdir[PITCH] = AngleMod( viewdir[PITCH] );
	vehdir[PITCH] = AngleMod( vehdir[PITCH] );
	turretdir[PITCH] = AngleMod( turretdir[PITCH] );

	// turn the hull
	if( pm->ps->ONOFF & OO_LANDED ) {
		// create a turning modifier for normal wheeled vehicles
		float topSpeed = availableVehicles[pm->vehicle].maxspeed;

		// maxium turn is allowed at a percentage of the top speed (0.5f behind 50% of top speed)
		float turnModifier = speed/(topSpeed * 0.2f);
		MF_LimitFloat( &turnModifier, 0.0f, 1.0f );

		if( smove > 0 ) {
			vehdir[YAW] -= (turnspeed[HULL_YAW] * turnModifier);
		} else if( smove < 0 ) {
			vehdir[YAW] += (turnspeed[HULL_YAW] * turnModifier);
		}
	}

	if( !(pm->cmd.buttons & BUTTON_FREELOOK) ) {
		float min, max;
		// get the angle difference
		for( i = PITCH; i <= YAW; i++ ) {
			diff[i] = viewdir[i] - turretdir[i];
			if( diff[i] > 180 ) diff[i] -= 360;
			else if( diff[i] < -180 ) diff[i] += 360;
		}	
		// turn the turret
		if( diff[YAW] < -turnspeed[TURRET_YAW] ) turret_yaw -= turnspeed[TURRET_YAW];
		else if( diff[YAW] > turnspeed[TURRET_YAW] ) turret_yaw += turnspeed[TURRET_YAW];
		else turret_yaw += diff[YAW];
		if( turret_yaw > 180 ) turret_yaw -= 360;// limit to +- 180
		min = availableWeapons[pm->ps->weaponIndex].minturns[1];
		max = availableWeapons[pm->ps->weaponIndex].maxturns[1];
		if( max > min ) {
			if( turret_yaw > max ) turret_yaw = max;
			else if( turret_yaw < min ) turret_yaw = min;
		} else {
			if( turret_yaw > 0 && turret_yaw < min ) turret_yaw = min;
			else if( turret_yaw < 0 && turret_yaw > max ) turret_yaw = max;
		}
		if( turret_yaw < 0 ) turret_yaw += 360;// clamp back to pos
		else if( turret_yaw > 360 ) turret_yaw -= 360;


		// turn the gun
		if( diff[PITCH] < -turnspeed[GUN_PITCH] ) gun_pitch -= turnspeed[GUN_PITCH];
		else if( diff[PITCH] > turnspeed[GUN_PITCH] ) gun_pitch += turnspeed[GUN_PITCH];
		else gun_pitch += diff[PITCH];
		min = availableWeapons[pm->ps->weaponIndex].minturns[0];
		max = availableWeapons[pm->ps->weaponIndex].maxturns[0];
		if( gun_pitch > 180 ) gun_pitch -= 360;// limit to +-180 to make it easier
		if( gun_pitch > max ) gun_pitch = max;
		else if( gun_pitch < min ) gun_pitch = min;
		if( gun_pitch < 0 ) gun_pitch += 360;// clamp it back to pos
		else if( gun_pitch > 360 ) gun_pitch -= 360;

//		Com_Printf( "gun %.1f (d %.1f)\n", gun_pitch, diff[PITCH] );
	}

	// return angles
	VectorCopy( vehdir, pm->ps->vehicleAngles );
	pm->ps->turretAngle = (int)(turret_yaw*10);
	pm->ps->gunAngle = (int)(gun_pitch*10);

}


/*
===============
PM_AddTouchEnt_Boat
===============
*/
static void PM_AddTouchEnt_Boat( int entityNum ) {
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
PM_SlideMove_Boat

Returns qtrue if the velocity was clipped in some way
==================
*/

qboolean PM_SlideMove_Boat() {
	float		time_left;
	vec3_t		end;
	vec3_t		up, down;
	trace_t		trace;
	float		angle;
	float		base;

	time_left = pml.frametime;

	// put on water, dependent on speed
	VectorCopy( pm->ps->origin, up );
	VectorCopy( up, down );
	up[2] += 10;
	down[2] -= 10;
	pm->trace( &trace,
			   up,
			   0,
			   0,
			   down,
			   pm->ps->clientNum,
			   MASK_ALL );
	pm->ps->origin[2] = trace.endpos[2];

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

	if( trace.allsolid ) {

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
			return qtrue;
		}
	}

	if( pm->waterlevel && trace.fraction > 0 ) {
		// actually covered some distance
		VectorCopy (trace.endpos, pm->ps->origin);
	} else {
		VectorClear(pm->ps->velocity);
		pm->ps->speed = 0;
	}

//	if( trace.fraction == 1 ) {
//	}

	// save entity for contact
	if(	trace.entityNum != ENTITYNUM_WORLD ||
		pm->ps->stats[STAT_HEALTH] <= 0 ||
		(trace.entityNum == ENTITYNUM_WORLD &&
//				!(trace.surfaceFlags & SURF_NOIMPACT) &&
		!(trace.surfaceFlags & SURF_SKY)) ) {
		PM_AddTouchEnt_Boat( trace.entityNum );
	}

	// angle
	angle = -DEG2RAD( pm->ps->vehicleAngles[0] );
	base = sin(angle) * -pm->mins[0];
	pm->ps->origin[2] += base;

	return qtrue;
}

/*
#define	MAX_CLIP_PLANES	5
qboolean	PM_SlideMove_Boat() {
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
	vec3_t		mins;
	
	VectorCopy( availableVehicles[pm->vehicle].mins, mins );
	mins[2] += 4;
	
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
					mins, 
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
				return qtrue;
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
			PM_AddTouchEnt_Boat( trace.entityNum );
		}

		time_left -= time_left * trace.fraction;

		if (numplanes >= MAX_CLIP_PLANES) {
			// this shouldn't really happen
			VectorClear( pm->ps->velocity );
			return qtrue;
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
					return qtrue;
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
}

*/
