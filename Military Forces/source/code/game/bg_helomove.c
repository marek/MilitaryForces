/*
 * $Id: bg_helomove.c,v 1.3 2005-08-31 19:20:06 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_local.h"
#include <algorithm>

extern pmove_t		*pm;
extern pml_t		pml;

#define MAX_HELO_PITCH		30.0f
#define MAX_HELO_ROLL		40.0f
#define MAX_HELO_TARGROLL	20	
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
PM_Helo_FuelFlow

===================
*/

static void PM_Helo_FuelFlow( int throttle )
{
	int fuelflow = (20 - throttle) * 1000;

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
PM_AdjustToTerrain

===================
*/
//static void PM_AdjustToTerrainHelo( void ) 
//{
//	trace_t		tr;
//	vec3_t		dir, forward, right;
//	vec3_t		start[PMAX], end[PMAX];
//	float		hLength, hWidth, hLengthC, hWidthC;
//	int			i;
//	float		angleX, angleY, angleXC, angleYC;
//	float		heightX, heightY, heightXC, heightYC;
//	bool	fall = false;	
//	float		height;
//
//	// set the height
//	VectorSet( start[0], pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2] + 30 );
//	VectorSet( end[0], pm->ps->origin[0], pm->ps->origin[1], pm->ps->origin[2] + pm->mins[2] - 2 );
//	pm->trace ( &tr, 
//				start[0], 
//				0,//availableVehicles[pm->vehicle].mins, 
//				0,//availableVehicles[pm->vehicle].maxs, 
//				end[0], 
//				pm->ps->clientNum, 
//				MASK_SOLID );
//	height = tr.endpos[2] - availableVehicles[pm->vehicle].mins[2] + 1;
//
//	if( tr.fraction < 1.0f ) {
//
//		// set up start and endpoints for full and close distance
//		VectorSet( dir, 0, pm->ps->vehicleAngles[1], 0 );
//		AngleVectors( dir, forward, right, 0 );
//		VectorMA( pm->ps->origin, availableVehicles[pm->vehicle].maxs[0], forward, start[PFRONT] );
//		VectorMA( pm->ps->origin, availableVehicles[pm->vehicle].mins[0], forward, start[PBACK] );
//		VectorMA( pm->ps->origin, availableVehicles[pm->vehicle].maxs[1], right, start[PRIGHT] );
//		VectorMA( pm->ps->origin, availableVehicles[pm->vehicle].mins[1], right, start[PLEFT] );
//		VectorMA( pm->ps->origin, 1, forward, start[PFRONTC] );
//		VectorMA( pm->ps->origin, -1, forward, start[PBACKC] );
//		VectorMA( pm->ps->origin, 1, right, start[PRIGHTC] );
//		VectorMA( pm->ps->origin, -1, right, start[PLEFTC] );
//		for( i = 0; i < PMAX; i++ ) {
//			VectorCopy( start[i], end[i] );
//			start[i][2] = pm->ps->origin[2] + 20;
//			end[i][2] = pm->ps->origin[2] - 40;
//		}
//		// get the projected length&width
//		VectorSubtract( start[PFRONT], start[PBACK], forward );
//		hLength = VectorLength( forward );
//		VectorSubtract( start[PRIGHT], start[PLEFT], right );
//		hWidth = VectorLength( right );
//		VectorSubtract( start[PFRONTC], start[PBACKC], forward );
//		hLengthC = VectorLength( forward );
//		VectorSubtract( start[PRIGHTC], start[PLEFTC], right );
//		hWidthC = VectorLength( right );
//
//		// do the ray tracing
//		for( i = 0; i < PMAX; i++ ) {
//			pm->trace ( &tr, 
//						start[i], 
//						vec3_origin, 
//						vec3_origin,
//						end[i], 
//						pm->ps->clientNum, 
//						MASK_SOLID );
//			VectorCopy( tr.endpos, end[i] ); // new
//			if( tr.fraction == 1 ) fall = true;
//		}
//		// new way
//		VectorSubtract( end[PFRONT], end[PBACK], forward );
//		vectoangles( forward, dir );
//		angleX = dir[0];
//		heightX = pm->ps->origin[2] + (end[PFRONT][2] - end[PBACK][2])/2;
//
//		VectorSubtract( end[PRIGHT], end[PLEFT], forward );
//		vectoangles( forward, dir );
//		angleY = dir[0];
//		heightY = pm->ps->origin[2] + (end[PRIGHT][2] - end[PLEFT][2])/2;
//
//		VectorSubtract( end[PFRONTC], end[PBACKC], forward );
//		vectoangles( forward, dir );
//		angleXC = dir[0];
//		heightXC = pm->ps->origin[2] + (end[PFRONTC][2] - end[PBACKC][2])/2;
//
//		VectorSubtract( end[PRIGHTC], end[PLEFTC], forward );
//		vectoangles( forward, dir );
//		angleYC = dir[0];
//		heightYC = pm->ps->origin[2] + (end[PRIGHTC][2] - end[PLEFTC][2])/2;
//
//		// check which one to use
//		if( !fall ) {
//			pm->ps->vehicleAngles[PITCH] = angleX;
//			pm->ps->vehicleAngles[ROLL] = angleY;
//		} else {
//			float diffp, diffr;
//			if( heightXC < heightX ) {
//				diffp = angleX - pm->ps->vehicleAngles[PITCH];
//			} else {
//				diffp = angleXC - pm->ps->vehicleAngles[PITCH];
//			}
//			while( diffp > 180 ) diffp -= 360;
//			while( diffp < -180) diffp += 360;
//			if( diffp > 0 ) {
//				pm->ps->vehicleAngles[0] += availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
//			} else if( diffp < 0 ) {
//				pm->ps->vehicleAngles[0] -= availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
//			}
//			if( heightYC < heightY ) {
//				diffr = angleY - pm->ps->vehicleAngles[ROLL];
//			} else {
//				diffr = angleYC - pm->ps->vehicleAngles[ROLL];
//			}
//			while( diffr > 180 ) diffr -= 360;
//			while( diffr < -180) diffr += 360;
//			if( diffr > 0 ) {
//				pm->ps->vehicleAngles[2] += availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
//			} else if( diffr < 0 ) {
//				pm->ps->vehicleAngles[2] -= availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
//			}
//		}
//		// set height
//		pm->ps->origin[2] = height;
//	} else {
//		pm->ps->ONOFF &= ~(OO_LANDED|OO_LANDEDTERRAIN);
//		if( pm->ps->ONOFF & OO_STALLED ) {
//			if( pm->ps->vehicleAngles[0] > 269 || pm->ps->vehicleAngles[0] < 90 ) 
//				pm->ps->vehicleAngles[0] -= availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
//		} else {
//			if( pm->ps->vehicleAngles[0] < 89 || pm->ps->vehicleAngles[0] > 270 )
//				pm->ps->vehicleAngles[0] += availableVehicles[pm->vehicle].turnspeed[0] * pml.frametime;
//		}
////		Com_Printf( "%.1f\n", pm->ps->vehicleAngles[2] );
//	}
//}

/*
===================
PM_Toggle_Gear

===================
*/
void PM_Toggle_HeloGear()
{
	if( !(availableVehicles[pm->vehicle].caps & HC_GEAR) ) {
		return;
	}

	if( pm->ps->ONOFF & OO_LANDED ) {
		return;
	}

	if( pm->cmd.serverTime < pm->ps->timers[TIMER_GEAR] && pm->ps->timers[TIMER_GEARANIM] ) {
		return;
	}

	if( pm->ps->speed > availableVehicles[pm->vehicle].stallspeed * 10 * SPEED_GREEN_ARC ) {
		pm->ps->timers[TIMER_GEAR] = pm->cmd.serverTime + availableVehicles[pm->vehicle].gearTime + 100;
		return;
	}

	if( pm->ps->ONOFF & OO_GEAR ) {
		PM_AddEvent( EV_GEAR_UP );
		pm->ps->timers[TIMER_GEARANIM] = pm->cmd.serverTime + availableVehicles[pm->vehicle].gearTime;
//		pm->ps->ONOFF &= ~OO_GEAR;
	}
	else {
		PM_AddEvent( EV_GEAR_DOWN );
		pm->ps->timers[TIMER_GEARANIM] = pm->cmd.serverTime + availableVehicles[pm->vehicle].gearTime;
//		pm->ps->ONOFF |= OO_GEAR;
	}
	pm->ps->timers[TIMER_GEAR] = pm->cmd.serverTime + availableVehicles[pm->vehicle].gearTime + 100;
}

void PM_Helo_Brakes()
{
	if( pm->cmd.serverTime < pm->ps->timers[TIMER_BRAKE] ) return;

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
PM_HeloAccelerate

===================
*/
static void PM_HeloAccelerate()
{
    float	throttle = pm->ps->fixed_throttle;
    int		maxthrottle = availableVehicles[pm->vehicle].maxthrottle;
//	int		minthrottle = availableVehicles[pm->vehicle].minthrottle;
	float	maxforwardspeed = availableVehicles[pm->vehicle].maxspeed;
	float	maxrightspeed = availableVehicles[pm->vehicle].turnspeed[YAW];
	float	maxliftspeed = maxforwardspeed*0.00525;
	float	maxspeed = sqrt(maxforwardspeed*maxforwardspeed + 
							maxrightspeed*maxrightspeed + 
							maxliftspeed*maxliftspeed);
	float	stallspeed = static_cast<float>(availableVehicles[pm->vehicle].stallspeed);
	float	curforwardspeed;
	float	curliftspeed;
	float	curliftspeedadjust;
	float	currightspeed;
	float	curspeed;
	float	curspeedadjust;
	float	totalthrottle;
	vec3_t	vehdir;
	
	// Copy Vehicle Direction
	VectorCopy( pm->ps->vehicleAngles, vehdir );
	vehdir[YAW] = AngleMod( vehdir[YAW] );

	curforwardspeed = (vehdir[PITCH]/MAX_HELO_PITCH)*availableVehicles[pm->vehicle].maxspeed;
	currightspeed = (vehdir[ROLL]/MAX_HELO_ROLL)*availableVehicles[pm->vehicle].turnspeed[YAW];
	curliftspeed = throttle > maxthrottle ? -(throttle-maxthrottle)*20 : throttle * 25;	// real vert speed	
	curliftspeedadjust = throttle > maxthrottle ? 0 : throttle * 25;					// adjusted vert speed, don't want down movement to effect fuel usage "more"
	curspeed = sqrt(curforwardspeed*curforwardspeed + currightspeed*currightspeed + curliftspeed*curliftspeed);						// real total speed
	curspeedadjust = sqrt(curforwardspeed*curforwardspeed + currightspeed*currightspeed + curliftspeedadjust*curliftspeedadjust);	// adjusted total speed, dont want down movement to effect throttle more
	totalthrottle = (curspeedadjust/maxspeed)*10;

	// check for fuel 
	if( pm->ps->stats[STAT_FUEL] <= 0 ) { 
		pm->ps->throttle = 0;
	}

	if( (pm->ps->ONOFF & OO_LANDED) && curspeed > stallspeed * 1.5f ) 
		curspeed = stallspeed *1.5f;

    pm->ps->speed = curspeed*10;

	pm->ps->throttle = throttle;

	// fuel flow
	PM_Helo_FuelFlow( totalthrottle );

}

/*
===================
PM_HeloMove

===================
*/
bool	PM_SlideMove_Helo();

void PM_HeloMove( void ) 
{
    vec3_t	viewdir;
    vec3_t	vehdir;
    vec3_t	diff;
    vec3_t	turnspeed;
    float	targroll;
    bool	dead = (pm->ps->stats[STAT_HEALTH] <= 0);
	bool	verydead = (pm->ps->stats[STAT_HEALTH] <= GIB_HEALTH);
    int		i;
//	int		anim = 0;
	// Speed related stuff
    float	throttle = pm->ps->fixed_throttle;
    int		maxthrottle = availableVehicles[pm->vehicle].maxthrottle;
//	int		maxforwardspeed = availableVehicles[pm->vehicle].maxspeed;
//	int		maxrightspeed = availableVehicles[pm->vehicle].turnspeed[YAW];
//	int		maxliftspeed = maxforwardspeed*0.00525;
//	int		maxspeed = sqrt(maxforwardspeed*maxforwardspeed + maxrightspeed*maxrightspeed + maxliftspeed*maxliftspeed);
	float	curforwardspeed;
	float	curliftspeed;
	float	currightspeed;
//	float	curspeed = pm->ps->speed;
	vec3_t  forwardvel,rightvel, liftvel, deltavel;
	// Turret stuff
	vec3_t		forward, up;
	vec3_t		temp;
	float		turret_yaw = pm->ps->turretAngle;
	float		gun_pitch = pm->ps->gunAngle;
	vec3_t		turretdir;
	float		min, max;
	float		turnModifier = 1.0f;

	if( verydead ) return;

	// clear FX
	pm->ps->ONOFF &= ~OO_VAPOR;


	// gear
	if( !dead && (pm->cmd.buttons & BUTTON_GEAR) ) {
		PM_Toggle_HeloGear();
	}

	// gearanim
	if( !dead && pm->ps->timers[TIMER_GEARANIM] &&
		pm->cmd.serverTime >= pm->ps->timers[TIMER_GEARANIM] ) {
		pm->ps->timers[TIMER_GEARANIM] = 0;
		if( pm->ps->ONOFF & OO_GEAR ) {
			pm->ps->ONOFF &= ~OO_GEAR;
		} else {
			pm->ps->ONOFF |= OO_GEAR;
		}
	}
	// update gear
	if( pm->updateGear ) {
		// sync anim
		if( pm->ps->ONOFF & OO_GEAR ) {
			PM_AddEvent( EV_GEAR_DOWN_FULL );
		} else {
			PM_AddEvent( EV_GEAR_UP_FULL );
		}
		pm->updateGear = false;
	}

	// get the actual turret angles
	AngleVectors( pm->ps->vehicleAngles, forward, 0, up );
	RotatePointAroundVector( temp, up, forward, turret_yaw );
	vectoangles( temp, turretdir );
	turretdir[PITCH] += gun_pitch;

    // local vectors
    VectorCopy( pm->ps->vehicleAngles, vehdir );
	if( pm->ps->ONOFF & OO_LANDED ) vehdir[PITCH] = vehdir[ROLL] = 0;

	//
	// Set current speeds (Do it here to allow more realistic crash)
	//
	if((pm->ps->ONOFF & OO_LANDED) && (throttle > maxthrottle))
		pm->ps->fixed_throttle = throttle = 0;
	curforwardspeed = (vehdir[PITCH]/MAX_HELO_PITCH)*availableVehicles[pm->vehicle].maxspeed;
	currightspeed = (vehdir[ROLL]/MAX_HELO_ROLL)*availableVehicles[pm->vehicle].turnspeed[YAW];
	curliftspeed = throttle > maxthrottle ? -(throttle-maxthrottle)*20 : throttle * 25;	
			// Forward speed
			VectorCopy(vehdir, forwardvel);
			forwardvel[PITCH] = 0;				// Don't let vehicle angels effect verticle acceleration
			AngleVectors(forwardvel, forwardvel, NULL, NULL );
			VectorScale(forwardvel, curforwardspeed, forwardvel);

			// Sideways speed
			VectorCopy(vehdir,  rightvel);
			rightvel[PITCH] = rightvel[ROLL] = 0;
			AngleVectors( rightvel,  NULL, rightvel, NULL );
			VectorScale( rightvel, currightspeed*2.5, rightvel );

			// Lift Speed
			VectorCopy(vehdir,  liftvel);
			liftvel[PITCH] = liftvel[YAW] = 0;
			AngleVectors( liftvel, NULL , NULL, liftvel );
			VectorScale( liftvel, curliftspeed, liftvel );

			// delta vel
			VectorAdd(forwardvel,rightvel,deltavel);
			VectorAdd(liftvel,deltavel,deltavel);

	// brake
	if( pm->cmd.buttons & BUTTON_BRAKE ) {
		PM_Helo_Brakes();
	}
	
	// freelook - plane keeps current heading
	if( (pm->cmd.buttons & BUTTON_FREELOOK) && !dead ) {
		VectorCopy( vehdir, viewdir );
		viewdir[0] = 0;
	}
	else { // normal - plane follows camera
		VectorCopy( pm->ps->viewangles, viewdir );
	}

	// set yaw to 0 <= x <= 360
	viewdir[YAW] = AngleMod( viewdir[YAW] );
	vehdir[YAW] = AngleMod( vehdir[YAW] );
	turretdir[YAW] = AngleMod( turretdir[YAW] );
	turretdir[PITCH] = AngleMod( turretdir[PITCH] );

	// Check acceleration
	PM_HeloAccelerate();

    if( dead ) {
		pm->ps->vehicleAngles[PITCH] = 30*sin(pm->ps->origin[2]/25);
		pm->ps->vehicleAngles[ROLL] = 20*sin(pm->ps->origin[2]/50);	

		VectorCopy(deltavel, pm->ps->velocity);			// copy to velocity
		pm->ps->velocity[ROLL] = -DEFAULT_GRAVITY/10;

		// dirty trick to remember bankangle
		if( pm->ps->vehicleAngles[1] <= 0 ) {
			pm->ps->vehicleAngles[1] -= availableVehicles[pm->vehicle].turnspeed[1]*1.3 * pml.frametime;
			if( pm->ps->vehicleAngles[1] < -360 ) pm->ps->vehicleAngles[1] += 360;
		}
		else {
			pm->ps->vehicleAngles[1] += availableVehicles[pm->vehicle].turnspeed[1]*1.3 * pml.frametime;
			if( pm->ps->vehicleAngles[1] > 360 ) pm->ps->vehicleAngles[1] -= 360;
		}
		PM_SlideMove_Helo();
		return;
    }

	for( i = PITCH; i <= ROLL; i++ ) {
	    turnspeed[i] = availableVehicles[pm->vehicle].turnspeed[i] * pml.frametime;
	}

	// ground movement
	if( pm->ps->ONOFF & OO_LANDED ) {
		if(!(pm->ps->ONOFF & OO_LANDEDTERRAIN)) {
			if(pm->cmd.forwardmove > 0) {
					VectorCopy(vehdir, forwardvel);
					forwardvel[PITCH] = forwardvel[ROLL] = 0;
					AngleVectors(forwardvel, forwardvel, NULL, NULL );
					pm->ps->speed = availableVehicles[pm->vehicle].turnspeed[PITCH];
					VectorScale(forwardvel, availableVehicles[pm->vehicle].turnspeed[PITCH], forwardvel);
			} else if (pm->cmd.forwardmove < 0) {
					VectorCopy(vehdir, forwardvel);
					forwardvel[PITCH] = forwardvel[ROLL] = 0;
					AngleVectors(forwardvel, forwardvel, NULL, NULL );
					pm->ps->speed = availableVehicles[pm->vehicle].turnspeed[PITCH]*10;
					VectorScale(forwardvel, -availableVehicles[pm->vehicle].turnspeed[PITCH], forwardvel);
			}
				
			// turn the hull
			if(pm->ps->ONOFF & OO_STALLED)
				turnModifier *= -1;

			if(pm->cmd.rightmove > 0 )
				vehdir[YAW] -= turnspeed[YAW]/2 * turnModifier;
			else if(pm->cmd.rightmove < 0 )
				vehdir[YAW] += turnspeed[YAW]/2 * turnModifier;

			VectorCopy(forwardvel, deltavel);
		}
	// air movement
	} else {
		// yaw
		diff[YAW] = viewdir[YAW] - vehdir[YAW];
		if( diff[YAW] > 180 ) diff[YAW] -= 360;
		else if( diff[YAW] < -180 ) diff[YAW] += 360;
		if( diff[YAW] < -turnspeed[YAW] ) vehdir[YAW] -= turnspeed[YAW];
		else if( diff[YAW] > turnspeed[YAW]) vehdir[YAW] += turnspeed[YAW];
		else vehdir[YAW] = viewdir[YAW];

		if( pm->ps->ONOFF & OO_SPEEDBRAKE ) {
			// pitch
			if(pm->cmd.forwardmove > 0)
				vehdir[PITCH] += turnspeed[PITCH]*1.25;
			else if(pm->cmd.forwardmove < 0)
				vehdir[PITCH] -= turnspeed[PITCH]*1.25;
			if( pm->ps->ONOFF & OO_SPEEDBRAKE && pm->cmd.forwardmove == 0) {
				if(vehdir[PITCH] > 0)
					vehdir[PITCH] -= std::min(vehdir[PITCH], turnspeed[PITCH]*1.05f);
				else if(vehdir[PITCH] < 0)
					vehdir[PITCH] += std::max(vehdir[PITCH], turnspeed[PITCH]*1.05f);
			}

			// roll
			if(pm->cmd.rightmove > 0)
				vehdir[ROLL] += turnspeed[ROLL]*2;
			else if(pm->cmd.rightmove < 0)
				vehdir[ROLL] -= turnspeed[ROLL]*2;
			
			// handle roll dependended on yaw otherwise return the vehicle ROLL to normal
			if( diff[YAW] > 1 ) targroll = -MAX_HELO_TARGROLL;
			else if( diff[YAW] < -1 ) targroll = MAX_HELO_TARGROLL;
			else targroll = 0;
			diff[ROLL] = targroll - vehdir[ROLL];
			if( diff[ROLL] < -turnspeed[ROLL] ) vehdir[ROLL] -= turnspeed[ROLL];
			else if( diff[ROLL] > turnspeed[ROLL] ) vehdir[ROLL] += turnspeed[ROLL];
			else vehdir[ROLL] = targroll;
		} else {
			// pitch
			if(pm->cmd.forwardmove > 0)
				vehdir[PITCH] += turnspeed[PITCH];
			else if(pm->cmd.forwardmove < 0)
				vehdir[PITCH] -= turnspeed[PITCH];

			// roll
			if(pm->cmd.rightmove > 0)
				vehdir[ROLL] += turnspeed[ROLL];
			else if(pm->cmd.rightmove < 0)
				vehdir[ROLL] -= turnspeed[ROLL];
		}


		// limit roll and pitch
		vehdir[PITCH] = vehdir[PITCH] < 0 ? std::max(-MAX_HELO_PITCH, vehdir[PITCH]) : std::min (MAX_HELO_PITCH, vehdir[PITCH]);
		vehdir[ROLL] = vehdir[ROLL] < 0 ? std::max(-MAX_HELO_ROLL, vehdir[ROLL]) : std::min (MAX_HELO_ROLL, vehdir[ROLL]);

		if( (availableVehicles[pm->vehicle].caps & HC_GEAR) && (pm->ps->ONOFF & OO_GEAR) &&
			(pm->ps->speed > availableVehicles[pm->vehicle].stallspeed * 10 * SPEED_GREEN_ARC) &&
			!pm->ps->timers[TIMER_GEARANIM] ) {
			PM_AddEvent( EV_GEAR_UP );
			pm->ps->timers[TIMER_GEAR] = pm->cmd.serverTime + availableVehicles[pm->vehicle].gearTime + 100;
			pm->ps->timers[TIMER_GEARANIM] = pm->cmd.serverTime + availableVehicles[pm->vehicle].gearTime;
		}
    }

	// Show turrets
	// get the angle difference
	for( i = PITCH; i <= YAW; i++ ) {
		diff[i] = pm->ps->viewangles[i] - turretdir[i];
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

	// return angles
	VectorCopy( vehdir, pm->ps->vehicleAngles );
	pm->ps->turretAngle = turret_yaw;
	pm->ps->gunAngle = gun_pitch;

	// speed
	if( pm->ps->ONOFF & OO_LANDED ) vehdir[0] = 0;
	
	// Copy final vel
	VectorCopy(deltavel, pm->ps->velocity);

	PM_SlideMove_Helo();
}

/*
===============
PM_AddTouchEnt_Plane
===============
*/
static void PM_AddTouchEnt_Helo( int entityNum ) {
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
bool	PM_SlideMove_Helo() {
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
		VectorMA(pm->ps->origin, time_left, pm->ps->velocity, end );			// calculate position we are trying to move to

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
			PM_AddTouchEnt_Helo( trace.entityNum );
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


