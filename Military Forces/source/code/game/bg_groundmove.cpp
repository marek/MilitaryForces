/*
 * $Id: bg_groundmove.c,v 1.6 2006-09-24 17:01:13 minkis Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_local.h"

extern pmove_t		*pm;
extern pml_t		pml;

#define	PFRONT		0
#define PBACK		1
#define PLEFT		2
#define PRIGHT		3
#define	PFRONTC		4
#define PBACKC		5
#define PLEFTC		6
#define PRIGHTC		7
#define PMAX		8



/*
===================
PM_GroundVehicle_Brakes

===================
*/

void PM_GroundVehicle_Brakes()
{
	if( pm->cmd.serverTime < pm->ps->timers[TIMER_BRAKE] ) return;

	// can only be engaged when slow enough
	if( pm->ps->speed > 120 ) return;

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
PM_GroundVehicle_FuelFlow

===================
*/

static void PM_GroundVehicle_FuelFlow( int throttle )
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
PM_GroundVehicleAccelerate
===================
*/
static void PM_GroundVehicleAccelerate()
{
    float	topspeed = availableVehicles[pm->vehicle].maxspeed;
    float	currspeed = (float)pm->ps->speed/10;
	float	maxthrottle = availableVehicles[pm->vehicle].maxthrottle;
	float	minthrottle = availableVehicles[pm->vehicle].minthrottle;
    float	throttle = pm->ps->fixed_throttle;
    float	accel = availableVehicles[pm->vehicle].accel * pml.frametime;
	float	targetspeed;
	float	angle = pm->ps->vehicleAngles[0];
		
	// account for water
	topspeed /= (pm->waterlevel+1);

	if( pm->ps->ONOFF & OO_LANDED ) {
		
		
		// terrain angle for additional acceleration
		// Original
		if( angle > 180 ) angle -= 360;
		else if( angle < -180 ) angle += 360;
		if( Q_fabs(angle) < 36 ) angle /= 36;
		else angle /= 18;

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
		if( (pm->ps->ONOFF & OO_SPEEDBRAKE) ) {
			targetspeed = 0;
			if( angle < -1 ) targetspeed += topspeed  *(angle / 5);
		} else {
			targetspeed = topspeed*throttle/maxthrottle;
			targetspeed += topspeed * (angle / 5);
		}

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

	PM_GroundVehicle_FuelFlow( throttle );
}

/*
===================
PM_AdjustToTerrain

===================
*/
static void PM_AdjustToTerrain( void ) 
{
	trace_t		tr;
	vec3_t		dir, forward, right;
	vec3_t		start[PMAX], end[PMAX];
	float		hLength, hWidth, hLengthC, hWidthC;
	int			i;
	float		angleX, angleY, angleXC, angleYC;
	float		heightX, heightY, heightXC, heightYC;
	bool	fall = false;	
	float		height;

	if( pm->waterlevel && availableVehicles[pm->vehicle].caps & HC_AMPHIBIOUS ) {
		return;
	}

	// set the height
	VectorSet( start[0], pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2] + 30 );
	VectorSet( end[0], pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2] + pm->mins[2] - 2 );
	pm->trace ( &tr, 
				start[0], 
				0,//availableVehicles[pm->vehicle].mins, 
				0,//availableVehicles[pm->vehicle].maxs, 
				end[0], 
				pm->ps->clientNum, 
				MASK_SOLID,
				false);
	height = tr.endpos[2] - availableVehicles[pm->vehicle].mins[2] + 1;

	if( tr.fraction < 1.0f ) {

		// set up start and endpoints for full and close distance
		VectorSet( dir, 0, pm->ps->vehicleAngles[1], 0 );
		AngleVectors( dir, forward, right, 0 );
		VectorMA( pm->ps->origin, availableVehicles[pm->vehicle].maxs[0], forward, start[PFRONT] );
		VectorMA( pm->ps->origin, availableVehicles[pm->vehicle].mins[0], forward, start[PBACK] );
		VectorMA( pm->ps->origin, availableVehicles[pm->vehicle].maxs[1], right, start[PRIGHT] );
		VectorMA( pm->ps->origin, availableVehicles[pm->vehicle].mins[1], right, start[PLEFT] );
		VectorMA( pm->ps->origin, 1, forward, start[PFRONTC] );
		VectorMA( pm->ps->origin, -1, forward, start[PBACKC] );
		VectorMA( pm->ps->origin, 1, right, start[PRIGHTC] );
		VectorMA( pm->ps->origin, -1, right, start[PLEFTC] );
		for( i = 0; i < PMAX; i++ ) {
			VectorCopy( start[i], end[i] );
			start[i][2] = pm->ps->origin[2] + 20;
			end[i][2] = pm->ps->origin[2] - 40;
		//	start[i][2] = availableVehicles[pm->vehicle].mins[2];
		//	end[i][2] = availableVehicles[pm->vehicle].mins[2]-2;
		}
		// get the projected length&width
		VectorSubtract( start[PFRONT], start[PBACK], forward );
		hLength = VectorLength( forward );
		VectorSubtract( start[PRIGHT], start[PLEFT], right );
		hWidth = VectorLength( right );
		VectorSubtract( start[PFRONTC], start[PBACKC], forward );
		hLengthC = VectorLength( forward );
		VectorSubtract( start[PRIGHTC], start[PLEFTC], right );
		hWidthC = VectorLength( right );

		// do the ray tracing
		for( i = 0; i < PMAX; i++ ) {
			pm->trace ( &tr, 
						start[i], 
						vec3_origin, 
						vec3_origin,
						end[i], 
						pm->ps->clientNum, 
						MASK_SOLID,
						false);
			VectorCopy( tr.endpos, end[i] ); // New
			if( tr.fraction == 1 ) fall = true;
		}
		// New way
		VectorSubtract( end[PFRONT], end[PBACK], forward );
		vectoangles( forward, dir );
		angleX = dir[0];
		heightX = pm->ps->origin[2] + (end[PFRONT][2] - end[PBACK][2])/2;

		VectorSubtract( end[PRIGHT], end[PLEFT], forward );
		vectoangles( forward, dir );
		angleY = dir[0];
		heightY = pm->ps->origin[2] + (end[PRIGHT][2] - end[PLEFT][2])/2;

		VectorSubtract( end[PFRONTC], end[PBACKC], forward );
		vectoangles( forward, dir );
		angleXC = dir[0];
		heightXC = pm->ps->origin[2] + (end[PFRONTC][2] - end[PBACKC][2])/2;

		VectorSubtract( end[PRIGHTC], end[PLEFTC], forward );
		vectoangles( forward, dir );
		angleYC = dir[0];
		heightYC = pm->ps->origin[2] + (end[PRIGHTC][2] - end[PLEFTC][2])/2;

		// check which one to use
		if( !fall ) {
			pm->ps->vehicleAngles[PITCH] = angleX;
			pm->ps->vehicleAngles[ROLL] = angleY;
		} else {
			float diffp, diffr;
			if( heightXC < heightX ) {
				diffp = angleX - pm->ps->vehicleAngles[PITCH];
			} else {
				diffp = angleXC - pm->ps->vehicleAngles[PITCH];
			}
			while( diffp > 180 ) diffp -= 360;
			while( diffp < -180) diffp += 360;
			if( diffp > 0 ) {
				pm->ps->vehicleAngles[0] += availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
			} else if( diffp < 0 ) {
				pm->ps->vehicleAngles[0] -= availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
			}
			if( heightYC < heightY ) {
				diffr = angleY - pm->ps->vehicleAngles[ROLL];
			} else {
				diffr = angleYC - pm->ps->vehicleAngles[ROLL];
			}
			while( diffr > 180 ) diffr -= 360;
			while( diffr < -180) diffr += 360;
			if( diffr > 0 ) {
				pm->ps->vehicleAngles[2] += availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
			} else if( diffr < 0 ) {
				pm->ps->vehicleAngles[2] -= availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
			}
		}
		// set height
		pm->ps->origin[2] = height;
		if( !(pm->ps->ONOFF & OO_LANDED) ) {
			pm->ps->ONOFF |= OO_VAPOR;
		}
		pm->ps->ONOFF |= OO_LANDED;
	} else {
		pm->ps->ONOFF &= ~OO_LANDED;
		if( pm->ps->ONOFF & OO_STALLED ) {
			if( pm->ps->vehicleAngles[0] > 269 || pm->ps->vehicleAngles[0] < 90 ) 
				pm->ps->vehicleAngles[0] -= availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
		} else {
			if( pm->ps->vehicleAngles[0] < 89 || pm->ps->vehicleAngles[0] > 270 )
				pm->ps->vehicleAngles[0] += availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
		}
//		Com_Printf( "%.1f\n", pm->ps->vehicleAngles[2] );
	}
}

/*
===================
PM_GroundVehicleMove

===================
*/
bool	PM_SlideMove_GV();

void PM_GroundVehicleMove( void ) 
{
	// like in the q3tank mod and airq2 the left/right keys turn the hull of the tank
	// while the mouse turns the turret/gunbarrel; and also like in airq2 we will later
	// also introduce a feature to lock the turret/gun to the hull which is more 
	// convenient in some situations. Usually when you turn the body of the tank the
	// turret keeps facing the previous direction, which is very useful in combat, because
	// you can easier keep tracking the enemy while driving zik-zak to avoid enemy hits,
	// sometimes however you might wanna keep the turret facing the current direction
	// (often forward) while driving so you don't have to use both controls. Also another
	// feature that will be introduced later will be the "auto-return" for the turret 
	// which makes it turn back to look in the direction the tank is driving after the
	// user hasn't moved it for 2 secs (or whatever value, might be set in a cvar as well)

    vec3_t		viewdir;
    vec3_t		vehdir;
	vec3_t		turretdir;
    vec3_t		diff;
    vec3_t		turnspeed;
	vec3_t		forward, up;
	vec3_t		temp;
    bool	dead = (pm->ps->stats[STAT_HEALTH] <= 0);
	int			i;
	float		smove = pm->cmd.rightmove;
	float		turret_yaw = pm->ps->turretAngle;
	float		gun_pitch = pm->ps->gunAngle;
	float		speed;
    float		topSpeed, currSpeed, turnModifier;
	int anim = 0;

	// brake
	if( pm->cmd.buttons & BUTTON_BRAKE ) {
		PM_GroundVehicle_Brakes();
	}

	// set speed
	PM_GroundVehicleAccelerate();

	// speed
	speed = (float)pm->ps->speed/10;
	if( pm->ps->ONOFF & OO_STALLED ) {
		AngleVectors( pm->ps->vehicleAngles, forward, NULL, NULL );
		VectorNegate( forward, forward );
	} else {
		AngleVectors( pm->ps->vehicleAngles, forward, NULL, NULL );
	}
	VectorNormalize( pm->ps->velocity );
	VectorAdd( forward, pm->ps->velocity, forward );
	VectorScale( forward, speed, pm->ps->velocity );

	PM_SlideMove_GV();

	// put the vehicle into the landscape with proper angles
	PM_AdjustToTerrain();

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


	// get the turnspeeds
	for( i = HULL_YAW; i <= GUN_PITCH; i++ ) {
	    turnspeed[i] = availableVehicles[pm->vehicle].turnspeed[i] * pml.frametime;
	}
	
	// non-track ground-vehicle?
	if( availableVehicles[pm->vehicle].caps & HC_WHEELS )
	{
		// create a turning modifier for normal wheeled vehicles
		topSpeed = availableVehicles[pm->vehicle].maxspeed;
		currSpeed = (float)pm->ps->speed/10;

		// maxium turn is allowed at a percentage of the top speed (0.5f behind 50% of top speed)
		turnModifier = currSpeed/(topSpeed * 0.2f);
		MF_LimitFloat( &turnModifier, 0.0f, 1.0f );
	}
	else
	{
		// n/a
		turnModifier = 1.0f;
	}

	// turn the hull
	if( pm->ps->ONOFF & OO_LANDED ) {
		if(pm->ps->ONOFF & OO_STALLED)
			turnModifier *= -1;

		if( smove > 0 ) {
			vehdir[YAW] -= turnspeed[HULL_YAW] * turnModifier;
			anim = 1;
		} else if( smove < 0 ) {
			vehdir[YAW] += turnspeed[HULL_YAW] * turnModifier;
			anim = 2;
		}
		pm->ps->vehicleAnim = anim;
	}

	if( !(pm->cmd.buttons & BUTTON_FREELOOK) ) {
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
		if( turret_yaw < 0 ) turret_yaw += 360;
		else if( turret_yaw > 360 ) turret_yaw -= 360;

		// turn the gun
		if( diff[PITCH] < -turnspeed[GUN_PITCH] ) gun_pitch -= turnspeed[GUN_PITCH];
		else if( diff[PITCH] > turnspeed[GUN_PITCH] ) gun_pitch += turnspeed[GUN_PITCH];
		else gun_pitch += diff[PITCH];
		if( gun_pitch < 0 ) gun_pitch += 360;
		else if( gun_pitch > 360 ) gun_pitch -= 360;
		if( gun_pitch < 90 && gun_pitch > availableVehicles[pm->vehicle].tailangle ) {
			gun_pitch = availableVehicles[pm->vehicle].tailangle;
		} else if ( gun_pitch > 90 && gun_pitch < availableVehicles[pm->vehicle].gearheight ) {
			gun_pitch = availableVehicles[pm->vehicle].gearheight;
		}
//		Com_Printf( "gun %.1f (d %.1f)\n", gun_pitch, diff[PITCH] );
	}

	// return angles
	VectorCopy( vehdir, pm->ps->vehicleAngles );
	pm->ps->turretAngle = turret_yaw;
	pm->ps->gunAngle = gun_pitch;

}


/*
===============
PM_AddTouchEnt_GV
===============
*/
static void PM_AddTouchEnt_GV( int entityNum ) {
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
PM_SlideMove_GV

Returns true if the velocity was clipped in some way
==================
*/
#define	MAX_CLIP_PLANES	5
bool	PM_SlideMove_GV() {
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
					pm->tracemask,
					false);

		if (trace.allsolid) {

			pm->trace ( &trace, 
					pm->ps->origin, 
					0, 
					0, 
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
		if(	trace.entityNum != ENTITYNUM_WORLD ||
			pm->ps->stats[STAT_HEALTH] <= 0 ||
			(trace.entityNum == ENTITYNUM_WORLD &&
			!(trace.surfaceFlags & SURF_SKY)) ) {
			PM_AddTouchEnt_GV( trace.entityNum );
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

			// see if there is a second plane that the New move enters
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


