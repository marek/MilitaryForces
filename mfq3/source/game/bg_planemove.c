/*
 * $Id: bg_planemove.c,v 1.2 2002-01-07 00:06:02 thebjoern Exp $
*/

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"

extern pmove_t		*pm;
extern pml_t		pml;


/*
===================
PM_Plane_Brakes

===================
*/
void PM_Plane_Brakes()
{
	if( !(pm->ps->ONOFF & OO_LANDED) &&
		!(availableVehicles[pm->vehicle].caps & HC_SPEEDBRAKE) ) {
		return;
	}

	if( pm->cmd.serverTime < pm->ps->timers[TIMER_BRAKE] ) {
		return;
	}

	if( pm->ps->ONOFF & OO_SPEEDBRAKE ) {
		pm->ps->ONOFF &= ~OO_SPEEDBRAKE;
	}
	else {
		pm->ps->ONOFF |= OO_SPEEDBRAKE;
	}
	pm->ps->timers[TIMER_BRAKE] = pm->cmd.serverTime + 500;
}


/*
===================
PM_Toggle_Gear

===================
*/
void PM_Toggle_Gear()
{
	if( !(availableVehicles[pm->vehicle].caps & HC_GEAR) ) {
		return;
	}

	if( pm->ps->ONOFF & OO_LANDED ) {
		return;
	}

	if( pm->cmd.serverTime < pm->ps->timers[TIMER_GEAR] ) {
		return;
	}

	if( pm->ps->speed > availableVehicles[pm->vehicle].stallspeed * 10 * SPEED_GREEN_ARC ) {
		pm->ps->timers[TIMER_GEAR] = pm->cmd.serverTime + 500;
		return;
	}

	if( pm->ps->ONOFF & OO_GEAR ) {
		pm->ps->ONOFF &= ~OO_GEAR;
	}
	else {
		pm->ps->ONOFF |= OO_GEAR;
	}
	pm->ps->timers[TIMER_GEAR] = pm->cmd.serverTime + 2000;
}

/*
===================
PM_Plane_FuelFlow

===================
*/

static void PM_Plane_FuelFlow( int throttle )
{
	int fuelflow = (20 - throttle) * 1000;

	// afterburner takes additional fuel
	if( throttle > 10 ) fuelflow -= 4000;

	if( pm->cmd.serverTime < pm->ps->timers[TIMER_FUEL] + fuelflow ) {
		return;
	}	

	pm->ps->stats[STAT_FUEL]--;
	if( pm->ps->stats[STAT_FUEL] <= 0 ) {
		pm->ps->stats[STAT_FUEL] = 0;
		pm->ps->throttle = pm->ps->fixed_throttle = 0;
	}

	pm->ps->timers[TIMER_FUEL] = pm->cmd.serverTime;
}

/*
===================
PM_PlaneAccelerate

===================
*/
static void PM_PlaneAccelerate()
{
    float	throttle = pm->ps->fixed_throttle;
    float	topspeed = availableVehicles[pm->vehicle].maxspeed;
    int		maxthrottle = availableVehicles[pm->vehicle].maxthrottle;
	int		minthrottle = availableVehicles[pm->vehicle].minthrottle;
    float	currspeed = (float)pm->ps->speed/10;
    float	stallspeed = (float)availableVehicles[pm->vehicle].stallspeed;
    float	accel = availableVehicles[pm->vehicle].accel * pml.frametime/3;
    float	decel = accel;

    float	acceleration;

	// check for overriding throttle
	if( pm->ps->stats[STAT_HEALTH] > 0 ) {
		if( pm->cmd.forwardmove > 0 ) throttle = maxthrottle;
		else if( pm->cmd.forwardmove < 0 ) throttle = minthrottle;
	}

	// check for fuel
	if( pm->ps->stats[STAT_FUEL] <= 0 ) throttle = 0;

	// set it back
	pm->ps->throttle = throttle;

    // accel depends on thrust
    accel *= ( 1 + ( throttle - maxthrottle ) / maxthrottle );
	if( throttle > 10 ) accel *= (throttle/10);

    // decel depends on speed
    decel *= ( 1 + ( currspeed - topspeed ) / topspeed);
	if( currspeed > topspeed ) decel *= (currspeed/topspeed);
	// decel depends on gear/brakes
	if( pm->ps->ONOFF & OO_SPEEDBRAKE ) {
		decel *= 1.7f;
	}
	if( pm->ps->ONOFF & OO_GEAR ) {
		decel *= 1.1f;
	}
	if( pm->ps->ONOFF & OO_LANDED ) {
		float factor = (stallspeed - currspeed) / stallspeed;
		if( factor > 0 ) {
			decel *= factor+1;
		}
		accel *= 0.8f;
	}
    acceleration = accel - decel;

    // accel depens on angle
    acceleration += pm->ps->vehicleAngles[0]/90;

    currspeed += acceleration;
    if( currspeed < 0 )
		currspeed = 0;

	if( (pm->ps->ONOFF & OO_LANDED) && currspeed > stallspeed * 1.5f ) 
		currspeed = stallspeed *1.5f;

    if( currspeed <= stallspeed ) {
		pm->ps->ONOFF |= OO_STALLED;
    }

    pm->ps->speed = currspeed*10;

	// fuel flow
	PM_Plane_FuelFlow( throttle );

}

/*
===================
PM_PlaneMove

===================
*/
qboolean	PM_SlideMove_Plane();

void PM_PlaneMove( void ) 
{
    vec3_t	viewdir;
    vec3_t	vehdir;
    vec3_t	diff;
    vec3_t	turnspeed;
    float	targroll;
    qboolean	dead = (pm->ps->stats[STAT_HEALTH] <= 0);
	qboolean	verydead = (pm->ps->stats[STAT_HEALTH] <= GIB_HEALTH);
    int		i;
	int		anim = 0;

	if( verydead ) return;

	// clear FX
	pm->ps->ONOFF &= ~(OO_VAPOR|OO_VAPOR_BIG);

	// gear
	if( !dead && (pm->cmd.buttons & BUTTON_GEAR) ) {
		PM_Toggle_Gear();
	}
	// brake
	if( !dead && (pm->cmd.buttons & BUTTON_BRAKE) ) {
		PM_Plane_Brakes();
	}

    // local vectors
    VectorCopy( pm->ps->vehicleAngles, vehdir );
	if( pm->ps->ONOFF & OO_LANDED ) vehdir[0] = 0;
	// freelook - plane keeps current heading
	if( (pm->cmd.buttons & BUTTON_FREELOOK) && !dead ) {
		VectorCopy( vehdir, viewdir );
	}
	else { // normal - plane follows camera
		VectorCopy( pm->ps->viewangles, viewdir );
	}

	// set yaw to 0 <= x <= 360
	viewdir[YAW] = AngleMod( viewdir[YAW] );
	vehdir[YAW] = AngleMod( vehdir[YAW] );

	PM_PlaneAccelerate();

    if( dead ) {
		if( pm->ps->vehicleAngles[PITCH] < 70 ) {
			pm->ps->vehicleAngles[PITCH] += 40 * pml.frametime; // nose drops at 60 deg/sec
		}
		AngleVectors( pm->ps->vehicleAngles, pm->ps->velocity, NULL, NULL );
		VectorScale( pm->ps->velocity, (float)pm->ps->speed/10, pm->ps->velocity );
		// dirty trick to remember bankangle
		if( pm->ps->vehicleAngles[2] <= 0 ) {
			pm->ps->vehicleAngles[2] -= availableVehicles[pm->vehicle].turnspeed[2]/2 * pml.frametime;
			if( pm->ps->vehicleAngles[2] < -360 ) pm->ps->vehicleAngles[2] += 360;
		}
		else {
			pm->ps->vehicleAngles[2] += availableVehicles[pm->vehicle].turnspeed[2]/2 * pml.frametime;
			if( pm->ps->vehicleAngles[2] > 360 ) pm->ps->vehicleAngles[2] -= 360;
		}
		PM_SlideMove_Plane();
		return;
    }

	for( i = PITCH; i <= ROLL; i++ ) {
	    turnspeed[i] = availableVehicles[pm->vehicle].turnspeed[i] * pml.frametime;
	}

	// ground movement
	if( pm->ps->ONOFF & OO_LANDED ) {
		float speed = (float)pm->ps->speed/10;
		if( (availableVehicles[pm->vehicle].caps & HC_TAILDRAGGER) ) {
			if( speed > availableVehicles[pm->vehicle].stallspeed/2 ) {
				vehdir[0] = 0;
			} else if( speed > ((float)availableVehicles[pm->vehicle].stallspeed/2 + 
							(float)availableVehicles[pm->vehicle].tailangle) ) {
				vehdir[0] = -((float)availableVehicles[pm->vehicle].stallspeed/2 - speed);
			} else {
				vehdir[0] = (float)availableVehicles[pm->vehicle].tailangle;
			}
		} else {
			vehdir[0] = 0;
		}
		vehdir[2] = 0;
		for( i = PITCH; i <= YAW; i++ ) {
			diff[i] = viewdir[i] - vehdir[i];
			if( diff[i] > 180 ) diff[i] -= 360;
			else if( diff[i] < -180 ) diff[i] += 360;
		}
		if( speed > 0 ) {
			if( diff[YAW] < -turnspeed[YAW] ) vehdir[YAW] -= turnspeed[YAW];
			else if( diff[YAW] > turnspeed[YAW] ) vehdir[YAW] += turnspeed[YAW];
			else vehdir[YAW] = viewdir[YAW];
			if( speed > availableVehicles[pm->vehicle].stallspeed &&
				diff[PITCH] < 0 ) {
				if( diff[PITCH] < -turnspeed[PITCH] ) vehdir[PITCH] -= turnspeed[PITCH];
				else vehdir[PITCH] = viewdir[PITCH];
				pm->ps->ONOFF &= ~(OO_LANDED|OO_STALLED);
			}	
		}
		if( speed < 20 && pm->ps->throttle == 0 ) {
			pm->ps->ONOFF |= OO_COCKPIT;
		} else {
			pm->ps->ONOFF &= ~OO_COCKPIT;
		}
	}
	// air movement
	else {
	    // stalled
	    if( pm->ps->ONOFF & OO_STALLED ) {
			if( vehdir[PITCH] < 50 ) vehdir[PITCH] += turnspeed[PITCH];
			if( vehdir[ROLL] < -5 ) vehdir[YAW] += turnspeed[YAW];
			else if( vehdir[ROLL] > 5 ) vehdir[YAW] -= turnspeed[YAW];

			if( viewdir[PITCH] >= vehdir[PITCH]  ) {//&&
				//viewdir[YAW] < vehdir[YAW] + 10 &&		// disable because it was too difficult
				//viewdir[YAW] > vehdir[YAW] - 10 ) {
				pm->ps->ONOFF &= ~OO_STALLED;
			}
//			Com_Printf( "Stalled\n" );    
	    }
	    // normal flight
	    else {
//			Com_Printf( "Normal\n" );
			// yaw/pitch
			for( i = PITCH; i <= YAW; i++ ) {
				diff[i] = viewdir[i] - vehdir[i];
    
				if( diff[i] > 180 ) diff[i] -= 360;
				else if( diff[i] < -180 ) diff[i] += 360;

				if( diff[i] < -turnspeed[i] ) vehdir[i] -= turnspeed[i];
				else if( diff[i] > turnspeed[i] ) vehdir[i] += turnspeed[i];
				else vehdir[i] = viewdir[i];
			}

			// roll
			if( diff[YAW] > 1 ) targroll = -90;
			else if( diff[YAW] < -1 ) targroll = 90;
			else targroll = 0;

			diff[ROLL] = targroll - vehdir[ROLL];

			if( diff[ROLL] < -turnspeed[ROLL] ) vehdir[ROLL] -= turnspeed[ROLL];
			else if( diff[ROLL] > turnspeed[ROLL] ) vehdir[ROLL] += turnspeed[ROLL];
			else vehdir[ROLL] = targroll;

			// animations dependent on movement
			if( diff[PITCH] < -8 || diff[YAW] > 8 || diff[YAW] < -8 ) {	// up
				anim = 3;
				// vapor effect on positive Gs
				if( availableVehicles[pm->vehicle].caps & HC_VAPOR ) {
					int speed = pm->ps->speed/10;
					if( diff[PITCH] < -70 || diff[YAW] > 70 || diff[YAW] < -70 ) {
						if( speed < availableVehicles[pm->vehicle].stallspeed * SPEED_GREEN_ARC &&
							speed > availableVehicles[pm->vehicle].stallspeed * SPEED_YELLOW_ARC ) {
							pm->ps->ONOFF |= OO_VAPOR_BIG;
							pm->ps->ONOFF &= ~OO_VAPOR;
						}
					}
					else if( diff[PITCH] < -50 || diff[YAW] > 50 || diff[YAW] < -50 ) {
						if( speed < availableVehicles[pm->vehicle].stallspeed * SPEED_GREEN_ARC &&
							speed > availableVehicles[pm->vehicle].stallspeed * SPEED_YELLOW_ARC ) {
							pm->ps->ONOFF |= OO_VAPOR;
							pm->ps->ONOFF &= ~OO_VAPOR_BIG;
						}
					}
				}
			}
			else if( diff[PITCH] > 8 ) { // down
				anim = 6;
			}
			if( diff[ROLL] < -8 ) {
				anim++;
			}
			else if( diff[ROLL] > 8 ) {
				anim += 2;
			}
			pm->ps->vehicleAnim = anim;

		//	Com_Printf( va("anim is %d\n", anim) );
		//	Com_Printf( va( "diff %.1f %.1f %.1f\n", diff[0], diff[1], diff[2] ) ); 
	      //  Com_Printf( va("hdg: %.1f diff: %.1f\n", viewdir[YAW], diff[YAW]) );
	    //   Com_Printf( va("view: %.1f %.1f %.1f\n", viewdir[PITCH], viewdir[YAW], viewdir[ROLL]) );
	        //Com_Printf( va("move: %.1f %.1f %.1f\n", vehdir[PITCH], vehdir[YAW], vehdir[ROLL]) );
			//}
		}
		if( (availableVehicles[pm->vehicle].caps & HC_GEAR) && (pm->ps->ONOFF & OO_GEAR) &&
			(pm->ps->speed > availableVehicles[pm->vehicle].stallspeed * 10 * SPEED_GREEN_ARC) ) {
			pm->ps->ONOFF &= ~OO_GEAR;
			pm->ps->timers[TIMER_GEAR] = pm->cmd.serverTime + 2000;
		}
		// wheel brakes off when airborne
		if( !(availableVehicles[pm->vehicle].caps & HC_SPEEDBRAKE) &&
			(pm->ps->ONOFF & OO_SPEEDBRAKE) ) {
			pm->ps->ONOFF &= ~ OO_SPEEDBRAKE;
		}
    }
	// return angles
	VectorCopy( vehdir, pm->ps->vehicleAngles );

	// speed
	if( pm->ps->ONOFF & OO_LANDED ) vehdir[0] = 0;
	AngleVectors( vehdir, pm->ps->velocity, NULL, NULL );
	VectorScale( pm->ps->velocity, (float)pm->ps->speed/10, pm->ps->velocity );

	PM_SlideMove_Plane();
}

/*
===============
PM_AddTouchEnt_Plane
===============
*/
static void PM_AddTouchEnt_Plane( int entityNum ) {
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

Returns qtrue if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5
static qboolean	PM_SlideMove_Plane() {
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
			PM_AddTouchEnt_Plane( trace.entityNum );
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


