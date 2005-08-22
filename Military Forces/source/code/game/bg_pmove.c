/*
 * $Id: bg_pmove.c,v 1.1 2005-08-22 16:05:25 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_pmove.c -- both games player movement code
// takes a playerstate and a usercmd as input and returns a modifed playerstate

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_local.h"

pmove_t		*pm;
pml_t		pml;

// movement parameters
float	pm_stopspeed = 100.0f;
float	pm_wadeScale = 0.70f;

float	pm_accelerate = 10.0f;
float	pm_airaccelerate = 1.0f;
float	pm_flyaccelerate = 8.0f;

float	pm_friction = 6.0f;
float	pm_waterfriction = 1.0f;
float	pm_flightfriction = 3.0f;
float	pm_spectatorfriction = 5.0f;

int		c_pmove = 0;


/*
===============
PM_AddEvent

===============
*/
void PM_AddEvent( int newEvent ) {
	BG_AddPredictableEventToPlayerstate( newEvent, 0, pm->ps );
}

/*
===============
PM_AddTouchEnt
===============
*/
void PM_AddTouchEnt( int entityNum ) {
	int		i;

	if ( entityNum == ENTITYNUM_WORLD ) {
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
PM_ClipVelocity

Slide off of the impacting surface
==================
*/
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
	float	backoff;
	float	change;
	int		i;
	
	backoff = DotProduct (in, normal);
	
	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i=0 ; i<3 ; i++ ) {
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
}


/*
==================
PM_Friction

Handles both ground friction and water friction
==================
*/
static void PM_Friction( void ) {
	vec3_t	vec;
	float	*vel;
	float	speed, newspeed, control;
	float	drop;
	
	vel = pm->ps->velocity;
	
	VectorCopy( vel, vec );
	if ( pml.walking ) {
		vec[2] = 0;	// ignore slope movement
	}

	speed = VectorLength(vec);
	if (speed < 1) {
		vel[0] = 0;
		vel[1] = 0;		// allow sinking underwater
		// FIXME: still have z friction underwater?
		return;
	}

	drop = 0;

	// apply ground friction
	if ( pm->waterlevel <= 1 ) {
		if ( pml.walking && !(pml.groundTrace.surfaceFlags & SURF_SLICK) ) {
			// if getting knocked back, no friction
			if ( ! (pm->ps->pm_flags & PMF_TIME_KNOCKBACK) ) {
				control = speed < pm_stopspeed ? pm_stopspeed : speed;
				drop += control*pm_friction*pml.frametime;
			}
		}
	}

	// apply water friction even if just wading
	if ( pm->waterlevel ) {
		drop += speed*pm_waterfriction*pm->waterlevel*pml.frametime;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR) {
		drop += speed*pm_spectatorfriction*pml.frametime;
	}

	// scale the velocity
	newspeed = speed - drop;
	if (newspeed < 0) {
		newspeed = 0;
	}
	newspeed /= speed;

	vel[0] = vel[0] * newspeed;
	vel[1] = vel[1] * newspeed;
	vel[2] = vel[2] * newspeed;
}


/*
==============
PM_Accelerate

Handles user intended acceleration
==============
*/
static void PM_Accelerate( vec3_t wishdir, float wishspeed, float accel ) {
#if 1
	// q2 style
	int			i;
	float		addspeed, accelspeed, currentspeed;

	currentspeed = DotProduct (pm->ps->velocity, wishdir);
	addspeed = wishspeed - currentspeed;
	if (addspeed <= 0) {
		return;
	}
	accelspeed = accel*pml.frametime*wishspeed;
	if (accelspeed > addspeed) {
		accelspeed = addspeed;
	}
	
	for (i=0 ; i<3 ; i++) {
		pm->ps->velocity[i] += accelspeed*wishdir[i];	
	}
#else
	// proper way (avoids strafe jump maxspeed bug), but feels bad
	vec3_t		wishVelocity;
	vec3_t		pushDir;
	float		pushLen;
	float		canPush;

	VectorScale( wishdir, wishspeed, wishVelocity );
	VectorSubtract( wishVelocity, pm->ps->velocity, pushDir );
	pushLen = VectorNormalize( pushDir );

	canPush = accel*pml.frametime*wishspeed;
	if (canPush > pushLen) {
		canPush = pushLen;
	}

	VectorMA( pm->ps->velocity, canPush, pushDir, pm->ps->velocity );
#endif
}



/*
============
PM_CmdScale

Returns the scale factor to apply to cmd movements
This allows the clients to use axial -127 to 127 values for all directions
without getting a sqrt(2) distortion in speed.
============
*/
static float PM_CmdScale( usercmd_t *cmd ) {
	int		max;
	float	total;
	float	scale;

	max = abs( cmd->forwardmove );
	if ( abs( cmd->rightmove ) > max ) {
		max = abs( cmd->rightmove );
	}
	if ( abs( cmd->upmove ) > max ) {
		max = abs( cmd->upmove );
	}
	if ( !max ) {
		return 0;
	}

	total = sqrt( cmd->forwardmove * cmd->forwardmove
		+ cmd->rightmove * cmd->rightmove + cmd->upmove * cmd->upmove );
	scale = (float)pm->ps->speed * max / ( 127.0 * total );

	return scale;
}

/*
===================
PM_FlyMove

Only with the flight powerup
===================
*/
static void PM_FlyMove( void ) {
	int		i;
	vec3_t	wishvel;
	float	wishspeed;
	vec3_t	wishdir;
	float	scale;

	// camera bbox
	VectorSet( pm->maxs, 8, 8, 8 );
	VectorSet( pm->mins, -8, -8, -8 );

	// normal slowdown
	PM_Friction ();

	scale = PM_CmdScale( &pm->cmd );

	// MFQ3: maps are much bigger and open than normal (e.g. Q3) maps, so increase the speed that
	// the spectator camera can travel at
	if ( pm->ps->pm_type == PM_SPECTATOR )
	{
		scale *= 4.0f;
	}

	//
	// user intentions
	//
	if ( !scale ) {
		wishvel[0] = 0;
		wishvel[1] = 0;
		wishvel[2] = 0;
	} else {
		for (i=0 ; i<3 ; i++) {
			wishvel[i] = scale * pml.forward[i]*pm->cmd.forwardmove + scale * pml.right[i]*pm->cmd.rightmove;
		}

		wishvel[2] += scale * pm->cmd.upmove;
	}

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);

	PM_Accelerate (wishdir, wishspeed, pm_flyaccelerate);

	PM_SlideMove( qfalse );
}

/*
===============
PM_NoclipMove
===============
*/
static void PM_NoclipMove( void ) {
	float	speed, drop, friction, control, newspeed;
	int			i;
	vec3_t		wishvel;
	float		fmove, smove;
	vec3_t		wishdir;
	float		wishspeed;
	float		scale;

	pm->ps->viewheight = 5;	// MFQ3 arbitrary value

	// friction

	speed = VectorLength (pm->ps->velocity);
	if (speed < 1)
	{
		VectorCopy (vec3_origin, pm->ps->velocity);
	}
	else
	{
		drop = 0;

		friction = pm_friction*1.5;	// extra friction
		control = speed < pm_stopspeed ? pm_stopspeed : speed;
		drop += control*friction*pml.frametime;

		// scale the velocity
		newspeed = speed - drop;
		if (newspeed < 0)
			newspeed = 0;
		newspeed /= speed;

		VectorScale (pm->ps->velocity, newspeed, pm->ps->velocity);
	}

	// accelerate
	scale = PM_CmdScale( &pm->cmd );

	fmove = pm->cmd.forwardmove;
	smove = pm->cmd.rightmove;
	
	for (i=0 ; i<3 ; i++)
		wishvel[i] = pml.forward[i]*fmove + pml.right[i]*smove;
	wishvel[2] += pm->cmd.upmove;

	VectorCopy (wishvel, wishdir);
	wishspeed = VectorNormalize(wishdir);
	wishspeed *= scale;

	PM_Accelerate( wishdir, wishspeed, pm_accelerate );

	// move
	VectorMA (pm->ps->origin, pml.frametime, pm->ps->velocity, pm->ps->origin);
}

//============================================================================

/*
=============
PM_SetWaterLevel	FIXME: avoid this twice?  certainly if not moving
=============
*/
static void PM_SetWaterLevel( void ) {
	vec3_t		point, start, end;
	int			cont;
//	int			sample1;
//	int			sample2;
	float		mins_z = availableVehicles[pm->vehicle].mins[2];
	float		maxs_z = availableVehicles[pm->vehicle].maxs[2];
	trace_t		tr;

/* old version
	//
	// get waterlevel, accounting for ducking
	//
	pm->waterlevel = 0;
	pm->watertype = 0;

	point[0] = pm->ps->origin[0];
	point[1] = pm->ps->origin[1];
	point[2] = pm->ps->origin[2] + mins_z + 1;	
	cont = pm->pointcontents( point, pm->ps->clientNum );

	if ( cont & MASK_WATER ) {
		sample2 = pm->ps->viewheight - mins_z;
		sample1 = sample2 / 2;

		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2] + mins_z + sample1;
		cont = pm->pointcontents (point, pm->ps->clientNum );
		if ( cont & MASK_WATER ) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + mins_z + sample2;
			cont = pm->pointcontents (point, pm->ps->clientNum );
			if ( cont & MASK_WATER ){
				pm->waterlevel = 3;
			}
		}
	}
*/
	pm->waterlevel = 0;
	pm->watertype = 0;

	if( availableVehicles[pm->vehicle].cat & CAT_BOAT ) {
		point[0] = pm->ps->origin[0];
		point[1] = pm->ps->origin[1];
		point[2] = pm->ps->origin[2] + mins_z;	
	} else {
		point[0] = pm->ps->origin[0];
		point[1] = pm->ps->origin[1];
		point[2] = pm->ps->origin[2] + mins_z + 1;	
	}
	cont = pm->pointcontents( point, pm->ps->clientNum );

	// new version
	if ( cont & MASK_WATER ) {
		pm->watertype = cont;
		pm->waterlevel = 1;
		point[2] = pm->ps->origin[2];
		cont = pm->pointcontents (point, pm->ps->clientNum );
		if ( cont & MASK_WATER ) {
			pm->waterlevel = 2;
			point[2] = pm->ps->origin[2] + maxs_z;
			cont = pm->pointcontents (point, pm->ps->clientNum );
			if ( cont & MASK_WATER ){
				pm->waterlevel = 3;
			} else {
				if( availableVehicles[pm->vehicle].caps & HC_AMPHIBIOUS ) {
					pm->ps->vehicleAngles[0] = pm->ps->vehicleAngles[2] = 0;
					VectorCopy( pm->ps->origin, start );
					start[2] += 10;
					VectorCopy( pm->ps->origin, end );
					end[2] -= 10;
					pm->trace ( &tr, 
								start, 
								0, 
								0, 
								end, 
								pm->ps->clientNum, 
								MASK_WATER );
					if( tr.fraction < 1.0f && pm->ps->origin[2] < tr.endpos[2]) {
						pm->ps->origin[2] = tr.endpos[2];
					}
				}
			}
		}
	}

}



//===================================================================


/*
==============
PM_WaterEvents

Generate sound events for entering and leaving water
==============
*/
static void PM_WaterEvents( void ) {		// FIXME?

//    if( pm->ps->pm_type == PM_VEHICLE ) {
//		return;
  //  }

	//
	// if just entered a water volume, play a sound
	//
	if (!pml.previous_waterlevel && pm->waterlevel) {
		PM_AddEvent( EV_WATER_TOUCH );
	}

	//
	// if just completely exited a water volume, play a sound
	//
	if (pml.previous_waterlevel && !pm->waterlevel) {
		PM_AddEvent( EV_WATER_LEAVE );
	}

	//
	// check for head just going under water
	//
	if (pml.previous_waterlevel != 3 && pm->waterlevel == 3) {
		PM_AddEvent( EV_WATER_UNDER );
	}

	//
	// check for head just coming out of water
	//
	if (pml.previous_waterlevel == 3 && pm->waterlevel != 3) {
		PM_AddEvent( EV_WATER_CLEAR );
	}
}


/*
==============
PM_Flare

Generates flare events and modifes the flare counter
==============
*/
static void PM_Flare( void ) {

	if( pm->ps->pm_flags & PMF_RESPAWNED ) {
		return;
	}

	// ignore if spectator
	if( pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	// check for dead player
	if( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// make weapon function
	if( pm->ps->timers[TIMER_FLARE] > 0 ) {
		pm->ps->timers[TIMER_FLARE] -= pml.msec;
		if( pm->ps->timers[TIMER_FLARE] > 0 ) {
			return;
		} else {
			pm->ps->timers[TIMER_FLARE] = 0;
		}
	}

	// fire flare
	if( pm->cmd.buttons & BUTTON_FLARE ) {
		// check for out of ammo
		if ( pm->ps->ammo[WP_FLARE] > 0 ) {
			pm->ps->ammo[WP_FLARE]--;
			PM_AddEvent( EV_FIRE_FLARE );
			pm->ps->timers[TIMER_FLARE] += availableWeapons[availableVehicles[pm->vehicle].weapons[WP_FLARE]].fireInterval;;
		}
	} 
}

/*
==============
PM_Weapon

Generates weapon events and modifes the weapon counter
==============
*/
static void PM_Weapon( void ) {
	qboolean	canShoot = qtrue,
				canShootMG = qtrue;

	// don't allow attack until all buttons are up and dont attack while recharging
	if( pm->ps->pm_flags & (PMF_RESPAWNED|PMF_RECHARGING) ) {
		return;
	}

	// ignore if spectator
	if( pm->ps->persistant[PERS_TEAM] == TEAM_SPECTATOR ) {
		return;
	}

	// check for dead player
	if( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	// check for change
	if( pm->ps->weaponNum != pm->cmd.weapon && availableVehicles[pm->vehicle].weapons[pm->cmd.weapon] != WI_NONE ) {
		pm->ps->weaponNum = pm->cmd.weapon;
		pm->ps->timers[TIMER_WEAPON] += 100;
	}

	// moved this out from the above if{} to see if that fixes the droptank problem
	pm->ps->weaponIndex = availableVehicles[pm->vehicle].weapons[pm->ps->weaponNum];

	// make weapon function
	if( pm->ps->timers[TIMER_MACHINEGUN] > 0 ) {
		pm->ps->timers[TIMER_MACHINEGUN] -= pml.msec;
		if( pm->ps->timers[TIMER_MACHINEGUN] > 0 ) {
			canShootMG = qfalse;
		} else {
			pm->ps->timers[TIMER_MACHINEGUN] = 0;
		}
	}
	if( pm->ps->timers[TIMER_WEAPON] > 0 ) {
		pm->ps->timers[TIMER_WEAPON] -= pml.msec;
		if( pm->ps->timers[TIMER_WEAPON] > 0 ) {
			canShoot = qfalse;
		} else {
			pm->ps->timers[TIMER_WEAPON] = 0;
		}
	}

	// allow firing of primary with MG button when no MG is available
	if( (pm->cmd.buttons & BUTTON_ATTACK) && !availableVehicles[pm->vehicle].weapons[0] ) {
		pm->cmd.buttons |= BUTTON_ATTACK_MAIN;
	}

	// weaponbays
	if( availableVehicles[pm->vehicle].caps & HC_WEAPONBAY ) {
		if( !(pm->ps->ONOFF & OO_WEAPONBAY) ) {
			if( pm->cmd.buttons & BUTTON_ATTACK_MAIN ) PM_Toggle_Bay();
			canShoot = qfalse;
		}
	}

	// landed
	if( (availableVehicles[pm->vehicle].cat & CAT_PLANE) ||
		(availableVehicles[pm->vehicle].cat & CAT_HELO) ) {
		if((pm->ps->ONOFF & OO_LANDED) ||
		   ((availableWeapons[pm->ps->weaponIndex].type == WT_IRONBOMB ||
			 availableWeapons[pm->ps->weaponIndex].type == WT_GUIDEDBOMB) &&
			Q_fabs(pm->ps->vehicleAngles[2]) > 60 ) ) {
			canShoot = qfalse;
		} 
	}

	// fueltank override
	if( availableWeapons[pm->ps->weaponIndex].type == WT_FUELTANK &&
		pm->ps->timers[TIMER_WEAPON] <= 0 )
		canShoot = qtrue;

//#ifndef QAGAME
//	Com_Printf("Weaponindex: %d\n", pm->ps->weaponIndex);
//#endif


	// check for MG primary fire
	if( (pm->cmd.buttons & BUTTON_ATTACK_MAIN) && pm->ps->weaponNum == WP_MACHINEGUN ) {
		pm->cmd.buttons |= BUTTON_ATTACK;
	}

	// fire machinegun
	if( canShootMG && (pm->cmd.buttons & BUTTON_ATTACK) ) {
		// check for out of ammo
		if ( pm->ps->ammo[WP_MACHINEGUN] > 0 ) {
			pm->ps->ammo[WP_MACHINEGUN]--;
			PM_AddEvent( EV_FIRE_MG );
			pm->ps->timers[TIMER_MACHINEGUN] += availableWeapons[availableVehicles[pm->vehicle].weapons[WP_MACHINEGUN]].fireInterval;
		}
	} 
	// fire main weapon
	if( canShoot && (pm->cmd.buttons & BUTTON_ATTACK_MAIN) ) {
		// check for out of ammo
		if ( !pm->ps->ammo[pm->ps->weaponNum] ) {
			pm->ps->timers[TIMER_WEAPON] += 100;
			return;
		}
		if( pm->ps->weaponNum == WP_MACHINEGUN ) return;

		// check for droptank
		if( availableWeapons[pm->ps->weaponIndex].type == WT_FUELTANK )
		{
			if( pm->ps->stats[STAT_FUEL] < pm->ps->stats[STAT_MAX_FUEL] )
			{
				pm->ps->ammo[pm->ps->weaponNum]--;
				pm->ps->stats[STAT_FUEL]++;
				if( !pm->ps->ammo[pm->ps->weaponNum] )
					PM_AddEvent( EV_FIRE_WEAPON );
			}
		}
		else
		{
			pm->ps->ammo[pm->ps->weaponNum]--;
			PM_AddEvent( EV_FIRE_WEAPON );
			pm->ps->timers[TIMER_BAYCLOSE] = pm->cmd.serverTime + 5000;
		}
		pm->ps->timers[TIMER_WEAPON] += availableWeapons[pm->ps->weaponIndex].fireInterval;
	}
}

/*
================
PM_DropTimers
================
*/
static void PM_DropTimers( void ) {
	// drop misc timing counter
	if ( pm->ps->pm_time ) {
		if ( pml.msec >= pm->ps->pm_time ) {
			pm->ps->pm_flags &= ~PMF_ALL_TIMES;
			pm->ps->pm_time = 0;
		} else {
			pm->ps->pm_time -= pml.msec;
		}
	}
}

/*
================
PM_UpdateViewAngles

This can be used as another entry point when only the viewangles
are being updated isntead of a full move
================
*/
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd ) {
	short		temp;
	int		i;

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;		// no view changes at all
	}

	if ( ps->pm_type != PM_SPECTATOR && ps->stats[STAT_HEALTH] <= 0 &&
		ps->pm_type != PM_VEHICLE ) {
		return;		// no view changes at all
	}

	if( (ps->pm_flags & PMF_VEHICLESELECT) && (ps->pm_type != PM_SPECTATOR) ) {
		return;
	}

	if( (ps->pm_flags & PMF_ME_FREEZE) ) {
		return;
	}

	// circularly clamp the angles with deltas
	for (i=0 ; i<3 ; i++) {
		temp = cmd->angles[i] + ps->delta_angles[i];
		if ( i == PITCH ) {
			// don't let the player look up or down more than 90 degrees
			if ( temp > 16000 ) {
				ps->delta_angles[i] = 16000 - cmd->angles[i];
				temp = 16000;
			} else if ( temp < -16000 ) {
				ps->delta_angles[i] = -16000 - cmd->angles[i];
				temp = -16000;
			}
		}
		ps->viewangles[i] = SHORT2ANGLE(temp);
	}

}

/*
==============
PM_VehicleBoundingBox

Sets mins, maxs, and pm->ps->viewheight
==============
*/
static void PM_VehicleBoundingBox (void)
{
/*    vec3_t	mins, maxs;
	float	diff1, diff2;
	float	corr = sin(fabs(pm->ps->vehicleAngles[2])*M_PI/180);

    VectorCopy( availableVehicles[pm->vehicle].mins, mins );
    VectorCopy( availableVehicles[pm->vehicle].maxs, maxs );

    if( pm->ps->vehicleAngles[2] >= 0 )
    {
	diff1 = abs(maxs[1]-maxs[2])*corr;
	diff2 = abs(mins[1]-mins[2])*corr;
	maxs[1] -= diff1;
	maxs[2] += diff1;
	mins[1] += diff2;
	mins[2] -= diff2;    
    }
    else 
    {
	diff1 = abs(maxs[1]+mins[2])*corr;
	diff2 = abs(mins[1]+maxs[2])*corr;
	maxs[1] -= diff2;
	maxs[2] += diff2;
	mins[1] += diff1;
	mins[2] -= diff1;
    }
   
    VectorCopy( mins, pm->mins );
    VectorCopy( maxs, pm->maxs );
*/
//    Com_Printf( "a: %.1f (%.1f*%.1f*%.1f) b: %.1f %.1f / %.1f %.1f\n", pm->ps->vehicleAngles[2], corr, diff1, diff2, 
//	    maxs[1], mins[1], maxs[2], mins[2] );

	// for now static bounding box
    VectorCopy( availableVehicles[pm->vehicle].mins, pm->mins );
    VectorCopy( availableVehicles[pm->vehicle].maxs, pm->maxs );


}

/*
void PM_Throttle_Up( gentity_t *vehicle )
{
	if( level.time < vehicle->client->throttleTimer ) {
		return;
	}
	if( vehicle->client->ps.stats[STAT_FUEL] <= 0 ) {
		return;
	}

	// ughh argh ouch
	if( vehicle->THROTTLE > availableVehicles[vehicle->client->vehicle].maxthrottle ) {
		vehicle->THROTTLE  = -(vehicle->THROTTLE - availableVehicles[vehicle->client->vehicle].maxthrottle);
	}

    if(vehicle->THROTTLE < availableVehicles[vehicle->client->vehicle].maxthrottle ) {
		vehicle->THROTTLE++;
    }

	// cant handle negative -> workaround
	if( vehicle->THROTTLE < 0 ) {
		vehicle->THROTTLE = availableVehicles[vehicle->client->vehicle].maxthrottle - vehicle->THROTTLE;
	}

	vehicle->client->throttleTimer = level.time + 250;
}

void PM_Throttle_Down( gentity_t *vehicle )
{
	if( level.time < vehicle->client->throttleTimer ) {
		return;
	}

	// ughh argh ouch
	if( vehicle->THROTTLE > availableVehicles[vehicle->client->vehicle].maxthrottle ) {
		vehicle->THROTTLE  = -(vehicle->THROTTLE - availableVehicles[vehicle->client->vehicle].maxthrottle);
	}

    if(vehicle->THROTTLE > availableVehicles[vehicle->client->vehicle].minthrottle ) {
		vehicle->THROTTLE--;
    }
	// cant handle negative -> workaround
	if( vehicle->THROTTLE < 0 ) {
		vehicle->THROTTLE = availableVehicles[vehicle->client->vehicle].maxthrottle - vehicle->THROTTLE;
	}

	vehicle->client->throttleTimer = level.time + 250;
}*/

void PM_Throttle_Up()
{
	if( pm->cmd.serverTime < pm->ps->timers[TIMER_THROTTLE] ) {
		return;
	}
	if( pm->ps->stats[STAT_FUEL] <= 0 ) {
		return;
	}
	if( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		return;
	}
	// ughh argh ouch
	if( pm->ps->fixed_throttle > availableVehicles[pm->vehicle].maxthrottle ) {
		pm->ps->fixed_throttle  = -(pm->ps->fixed_throttle - availableVehicles[pm->vehicle].maxthrottle);
	}

    if( pm->ps->fixed_throttle < availableVehicles[pm->vehicle].maxthrottle ) {
		pm->ps->fixed_throttle++;
    }

	// cant handle negative -> workaround
	if( pm->ps->fixed_throttle < 0 ) {
		pm->ps->fixed_throttle = availableVehicles[pm->vehicle].maxthrottle - pm->ps->fixed_throttle;
	}

	pm->ps->timers[TIMER_THROTTLE] = pm->cmd.serverTime + 250;
}

void PM_Throttle_Down()
{
	if( pm->cmd.serverTime < pm->ps->timers[TIMER_THROTTLE] ) {
		return;
	}
	if( pm->ps->stats[STAT_FUEL] <= 0 ) {
		return;
	}
	if( pm->ps->stats[STAT_HEALTH] <= 0 ) {
		return;
	}
	// ughh argh ouch
	if( pm->ps->fixed_throttle > availableVehicles[pm->vehicle].maxthrottle ) {
		pm->ps->fixed_throttle  = -(pm->ps->fixed_throttle - availableVehicles[pm->vehicle].maxthrottle);
	}

    if( pm->ps->fixed_throttle > availableVehicles[pm->vehicle].minthrottle ) {
		pm->ps->fixed_throttle--;
    }
	// cant handle negative -> workaround
	if( pm->ps->fixed_throttle < 0 ) {
		pm->ps->fixed_throttle = availableVehicles[pm->vehicle].maxthrottle - pm->ps->fixed_throttle;
	}

	pm->ps->timers[TIMER_THROTTLE] = pm->cmd.serverTime + 250;
}

/*
===================
PM_VehicleMove

===================
*/
static void PM_VehicleMove( void ) 
{
	if( availableVehicles[pm->vehicle].cat & CAT_PLANE ) {
		if( pm->advancedControls )
			PM_PlaneMoveAdvanced();
		else
			PM_PlaneMove();
	}
	else if( availableVehicles[pm->vehicle].cat & CAT_HELO ) {
		PM_HeloMove();
	}
	else if( availableVehicles[pm->vehicle].cat & CAT_LQM ) {
		PM_LQMMove();
	}
	else if( availableVehicles[pm->vehicle].cat & CAT_BOAT ) {
		PM_BoatMove();
	}
	else if( availableVehicles[pm->vehicle].cat & CAT_GROUND ) {
		PM_GroundVehicleMove();
	}
	else {
#ifdef QAGAME
		Com_Error( ERR_DROP, "Server: Invalid vehicle type in PM_VehicleMove" );
#else
		Com_Error( ERR_DROP, "Client: Invalid vehicle type in PM_VehicleMove" );
#endif
	}
}

/*
================
PmoveSingle

================
*/
void trap_SnapVector( float *v );

void PmoveSingle (pmove_t *pmove) {
	pm = pmove;

	// this counter lets us debug movement problems with a journal
	// by setting a conditional breakpoint fot the previous frame
	c_pmove++;

	// clear results
	pm->numtouch = 0;
	pm->watertype = 0;
	pm->waterlevel = 0;

	if( pm->vehicle >= 0 && pm->ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
		pm->tracemask &= ~CONTENTS_BODY;	// corpses can fly through bodies
	}

	// set the talk balloon flag
	if ( pm->cmd.buttons & BUTTON_TALK ) {
		pm->ps->eFlags |= EF_TALK;
	} else {
		pm->ps->eFlags &= ~EF_TALK;
	}

	// clear the respawned flag if attack and use are cleared
	if ( pm->ps->stats[STAT_HEALTH] > 0 && 
		!( pm->cmd.buttons & (BUTTON_ATTACK) ) ) { // | SECOND_FIRE_BUTTON
		pm->ps->pm_flags &= ~PMF_RESPAWNED;
	}

	// if talk button is down, dissallow all other input
	// this is to prevent any possible intercept proxy from
	// adding fake talk balloons
	if ( pmove->cmd.buttons & BUTTON_TALK ) {
		// keep the talk button set tho for when the cmd.serverTime > 66 msec
		// and the same cmd is used multiple times in Pmove
		pmove->cmd.buttons = BUTTON_TALK;
		pmove->cmd.forwardmove = 0;
		pmove->cmd.rightmove = 0;
		pmove->cmd.upmove = 0;
	}

	// clear all pmove local vars
	memset (&pml, 0, sizeof(pml));

	// determine the time
	pml.msec = pmove->cmd.serverTime - pm->ps->commandTime;
	if ( pml.msec < 1 ) {
		pml.msec = 1;
	} else if ( pml.msec > 200 ) {
		pml.msec = 200;
	}
	pm->ps->commandTime = pmove->cmd.serverTime;

	// save old org in case we get stuck
	VectorCopy (pm->ps->origin, pml.previous_origin);

	// save old velocity for crashlanding
//	VectorCopy (pm->ps->velocity, pml.previous_velocity);

	pml.frametime = pml.msec * 0.001;

	// update the viewangles
	PM_UpdateViewAngles( pm->ps, &pm->cmd );

	AngleVectors (pm->ps->viewangles, pml.forward, pml.right, pml.up);

	if ( pm->ps->pm_type == PM_DEAD ) {
		pm->cmd.forwardmove = 0;
		pm->cmd.rightmove = 0;
		pm->cmd.upmove = 0;
		if( pm->ps->stats[STAT_HEALTH] <= GIB_HEALTH ) return;
	}

	if ( pm->ps->pm_type == PM_SPECTATOR ) {
		PM_FlyMove ();
		PM_DropTimers ();
		return;
	}

	if ( pm->ps->pm_type == PM_NOCLIP ) {
		PM_NoclipMove ();
		PM_DropTimers ();
		return;
	}

	if (pm->ps->pm_type == PM_FREEZE || pm->vehicle < 0 ) {
		return;		// no movement at all
	}

	if ( pm->ps->pm_type == PM_INTERMISSION ) {
		return;		// no movement at all
	}

	// throttle
	if( pm->cmd.buttons & BUTTON_INCREASE ) {
		PM_Throttle_Up();
	} else if( pm->cmd.buttons & BUTTON_DECREASE ) {
		PM_Throttle_Down();
	}

	// set watertype, and waterlevel
	PM_SetWaterLevel();
	pml.previous_waterlevel = pmove->waterlevel;

	// set mins, maxs, and viewheight
    PM_VehicleBoundingBox();

	PM_DropTimers();

	// vehicle
	PM_VehicleMove();

	// set watertype, and waterlevel
	PM_SetWaterLevel();

	// weapons
	PM_Weapon();

	// flares
	PM_Flare();

	// entering / leaving water splashes
	PM_WaterEvents();

	// snap some parts of playerstate to save network bandwidth
	trap_SnapVector( pm->ps->velocity );
}


/*
================
Pmove

Can be called by either the server or the client
================
*/
void Pmove (pmove_t *pmove) {
	int			finalTime;

	finalTime = pmove->cmd.serverTime;

	if ( finalTime < pmove->ps->commandTime ) {
		return;	// should not happen
	}

	if ( finalTime > pmove->ps->commandTime + 1000 ) {
		pmove->ps->commandTime = finalTime - 1000;
	}

	pmove->ps->pmove_framecount = (pmove->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

	// chop the move up if it is too long, to prevent framerate
	// dependent behavior
	while ( pmove->ps->commandTime != finalTime ) {
		int		msec;

		msec = finalTime - pmove->ps->commandTime;

		if ( pmove->pmove_fixed ) {
			if ( msec > pmove->pmove_msec ) {
				msec = pmove->pmove_msec;
			}
		}
		else {
			if ( msec > 66 ) {
				msec = 66;
			}
		}
		pmove->cmd.serverTime = pmove->ps->commandTime + msec;
		PmoveSingle( pmove );

	}

	//PM_CheckStuck();

}

