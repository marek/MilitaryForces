/************************************************/
/* Filename: aq2_helo.c						    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 2000-01-08							*/
/*												*/
/* Description:									*/
/* NewAirQuake2 helo-specific stuff     		*/
/*												*/
/************************************************/

#include "g_local.h"











void HeloSound( edict_t *helo )
{
	float speedindex, volume;

	if( !helo->random )
	{
		helo->random = level.time;
		return;
	}

	if( level.time > helo->random && (helo->ONOFF & ONOFF_PILOT_ONBOARD) )
	{
		speedindex = floor( (helo->speed) / 35 );
		volume = helo->thrust * 0.05 + 0.3;

		if( volume > 1 )
			volume = 1;
		else if( volume < 0 )
			volume = 0;

//		gi.bprintf( PRINT_HIGH, "%f %f\n", speedindex, volume );

		if( helo->lockstatus )
		{
			if( helo->lockstatus == LS_LOCK )
				gi.sound( helo, CHAN_ITEM, gi.soundindex( "flying/in2.wav" ), 1.0, ATTN_STATIC, 0 );
			else
				gi.sound( helo, CHAN_ITEM, gi.soundindex( "flying/in1.wav" ), 1.0, ATTN_STATIC, 0 );
		}
		gi.sound( helo, CHAN_BODY, gi.soundindex( "engines/chopper.wav" ), volume, ATTN_NORM, 0 );

		helo->random = level.time + 0.2;
	}

}


void SetHeloSpeed( edict_t *helo )
{
	float	tgtspdside;
	vec3_t	forward,right,up;
	int		VM = (helo->owner->client->FLAGS & CLIENT_NONGL_MODE) ? -1 : 1;

//	gi.bprintf( PRINT_HIGH, "old:%.1f new:%.1f\n", helo->style, helo->s.angles[0] );

	if( helo->deadflag == DEAD_DEAD )
	{
		helo->speed = 0;
		return;
	}

	// calc speeds
	if( !(helo->ONOFF & ONOFF_AUTOROTATION ) )
	{
//		gi.bprintf( PRINT_HIGH, "STANDARD\n" );// debugging
		if( !(helo->ONOFF & ONOFF_LANDED) )
		{
			if( helo->thrust >= THRUST_IDLE && helo->thrust <= THRUST_HOVER )
				helo->stallspeed = (helo->thrust * 30) - 150;
			else if( helo->thrust > THRUST_HOVER )
				helo->stallspeed = (helo->thrust - THRUST_IDLE ) * 25;
		}
		else
		{
			if( helo->thrust >= THRUST_HOVER && helo->thrust < THRUST_MILITARY )
				helo->stallspeed = 50;
			else if( helo->thrust == THRUST_MILITARY )
				helo->stallspeed = 120;
			else
				helo->stallspeed = 0;
		}
	}
	// auto-rotation
	if( helo->elevator < 0 && helo->style > helo->s.angles[0] && helo->thrust < THRUST_HOVER - 2 )
	{
		helo->stallspeed += (int)(15*(helo->style-helo->s.angles[0]));
//		gi.bprintf( PRINT_HIGH, "autorotation\n" );// debugging
		helo->ONOFF |= ONOFF_AUTOROTATION;
		if( helo->stallspeed > 30 )
			helo->stallspeed = 30;
	}
	else
		helo->ONOFF &= ~ONOFF_AUTOROTATION;

	// in-ground-effect
//	if( !(helo->ONOFF & ONOFF_AUTOROTATION ) && helo->altitude < 50 )
	{
//		helo->tempspeed +=	(70-helo->altitude);
	}


	if( helo->s.angles[0] > 5 )
		helo->speed = (helo->topspeed/35)*(helo->s.angles[0]-5);
	else if( helo->s.angles[0] < -5 )
		helo->speed = (helo->topspeed/45)*(helo->s.angles[0]+5);
	else
		helo->speed = 0;


	if( helo->s.angles[2] > 5 )
		tgtspdside = -5 * (fabs(helo->s.angles[2])-5)*VM;
	else if( helo->s.angles[2] < -5 )
		tgtspdside = 5 * (fabs(helo->s.angles[2])-5)*VM;
	else
		tgtspdside = 0;

	// NOTE: helo->currentop used for vertical speed!

	if( helo->stallspeed > helo->verticalspeed )
	{
		if( helo->thrust > THRUST_MILITARY - 2 )
			helo->verticalspeed += 40;
		else if( helo->ONOFF & ONOFF_AUTOROTATION )
			helo->verticalspeed += 40;
		else
			helo->verticalspeed += 20;
	}
	else if( helo->stallspeed < helo->verticalspeed )
	{
		if( helo->thrust < THRUST_HOVER / 2 )
			helo->verticalspeed -= 40;
		else
			helo->verticalspeed -= 20;
	}

	// give speed
	if( !(helo->ONOFF & ONOFF_LANDED) )
	{
		AngleVectors( helo->s.angles, forward, right, up );
		forward[2] = 0;	// more realistic: forward[2] /= 3;
		VectorNormalize( forward );
		right[2] = 0;
		VectorNormalize( right );
		VectorScale( forward, helo->speed, forward );
		VectorScale( up, helo->verticalspeed, up );
		up[0] = up[1] = 0;
		VectorScale( right, tgtspdside, right );
		VectorAdd( forward, right, helo->velocity );
		if( VectorLength(helo->velocity) > helo->topspeed )
		{
			VectorNormalize( helo->velocity );
			VectorScale( helo->velocity, helo->topspeed, helo->velocity );
		}
		helo->speed = VectorLength( helo->velocity );
		if( helo->s.angles[0] < 0 )
			helo->speed *= -1;
		VectorAdd( helo->velocity, up, helo->velocity );
	}
	else
	{
		if( helo->thrust > THRUST_MILITARY - 2 )
		{
			AngleVectors( helo->s.angles, forward, right, up );
			helo->speed = VectorLength( helo->velocity );
			VectorAdd( helo->velocity, up, helo->velocity );
		}
		else
		{
			helo->speed = 0;
			VectorScale( helo->velocity, helo->speed, helo->velocity );
		}
	}

//	gi.bprintf( PRINT_HIGH, "%.2f\n", VectorLength(helo->velocity) );

	helo->style = helo->s.angles[0];

	HeloSound( helo );
}

void HeloMovement( edict_t *helo )
{
	int		VM = (helo->owner->client->FLAGS & CLIENT_NONGL_MODE) ? -1 : 1;
	int		bay = 0;
	vec3_t	above, below;
	trace_t	trace;

	VectorClear( helo->avelocity );
	VectorClear( helo->velocity );

	// health-effects
	if( helo->owner->health >= 26 )
		helo->s.effects = 0;
	else if( helo->owner->health < 26 && helo->owner->health > 10 )
	{
		helo->s.effects = EF_GRENADE;
	}
	else if( helo->owner->health <= 10 && helo->deadflag != DEAD_DEAD )
	{
		helo->s.effects = EF_ROCKET;
	}
	else
		helo->s.effects = 0;

	// dead
	if( helo->deadflag )
	{
		if( helo->enemy )
		{
			helo->enemy->lockstatus = 0;
			helo->enemy = NULL;
		}
		helo->s.frame = 0;
		helo->s.renderfx = 0;
		helo->avelocity[2] = 120;
		helo->gravity = 0.3;
		helo->movetype = MOVETYPE_TOSS;
		return;
	}

	// overturning ?
	helo->s.angles[1] = anglemod( helo->s.angles[1] );

	// frames
	helo->s.renderfx = RF_TRANSLUCENT;
	if( helo->ONOFF & ONOFF_WEAPONBAY )
		bay = 1;
	if( helo->ONOFF & ONOFF_AIRBORNE ) // not landed
	{
		if( helo->ONOFF & ONOFF_GEAR )
		{
			if( helo->s.frame < 16 + 16 * bay )
				helo->s.frame ++;
			else
				helo->s.frame = 9 +  16 * bay;
		}
		else
		{
			if( helo->s.frame < 8 +  16 * bay )
				helo->s.frame ++;
			else
				helo->s.frame = 1 +  16 * bay;
		}
	}
	else		// landed
	{
		if( helo->ONOFF & ONOFF_PILOT_ONBOARD )
		{
			if( helo->s.frame < 16 + 16 * bay )
				helo->s.frame ++;
			else
				helo->s.frame = 9 + 16 * bay;
		}
		else
		{
			helo->s.renderfx = 0;
			helo->s.frame = 0;
		}
	}
#pragma message( "check helo frames (opeing bay/gear down)" )
#pragma message( "check states after landing/takeoff, why cant I put down/up gear, bay?" )
#pragma message( "check bounding box, shouldnt change when bringing down/up gear for helo")
#pragma message( "check speed for going up/down, too fast up, too slow down, add gravity down!")
#pragma message( "helo shall not fly nose down, only accelerate so, then return to horizontal" )
#pragma message( "find solution for shooting and aiming with helo, helmet mounted visor ?" )
	SetHeloSpeed( helo );

	// control input
	if( !(helo->ONOFF & ONOFF_LANDED) )
	{
		// rudder
		if( helo->rudder )
			helo->avelocity[1] = helo->rudder;

		// elevator
		if( helo->elevator )
		{
			if( helo->elevator > 0 )
			{
				if( helo->s.angles[0] < 55 && helo->s.angles[0] > -80 )
					helo->avelocity[0] = helo->elevator;
			}
			else if( helo->elevator < 0 )
			{
				if( helo->s.angles[0] > -35 && helo->s.angles[0] < 80 )
					helo->avelocity[0] = helo->elevator;
			}
		}

		if( helo->s.angles[0] > 55 )
			helo->s.angles[0] = 55;
		else if( helo->s.angles[0] < -35 )
			helo->s.angles[0] = -35;

		// aileron
		if( helo->aileron )
		{
			if( helo->aileron > 0 )
			{
				if( helo->s.angles[2]*VM < 30 && helo->s.angles[2]*VM > -60 )
					helo->avelocity[2] = helo->aileron*VM;
			}
			else if( helo->aileron < 0 )
			{
				if( helo->s.angles[2]*VM > -30 && helo->s.angles[2]*VM < 60 )
					helo->avelocity[2] = helo->aileron*VM;
			}
		}
		else
		{
			if( helo->s.angles[2] > 3 && helo->s.angles[2] < 90 )
				helo->avelocity[2] = -40;
			else if( helo->s.angles[2] < -3 && helo->s.angles[2] > -90 )
				helo->avelocity[2] = 40;
		}

		// yaw when banked at speed
		if( fabs(helo->s.angles[2]) > 5 && fabs(helo->s.angles[0]) > 12 )
		{
			helo->avelocity[1] += (helo->s.angles[2]*VM);
		}

		// auto-coord
//		if( vehicle->owner->client->pers.autocoord )
		{
//			vehicle->avelocity[1] += vehicle->aileron;
		}

		// auto-roll-back
		if( ((helo->owner->client->FLAGS & CLIENT_NONGL_MODE) && !helo->elevator)
				 || ( helo->ONOFF & ONOFF_AUTOPILOT ) )
		{
			if( helo->s.angles[0] > 2 && helo->s.angles[0] < 90 )
				helo->avelocity[0] = -20;
			else if( helo->s.angles[0] < -2 && helo->s.angles[0] > -90 )
				helo->avelocity[0] = 20;
		}
	}

	// did we touch the sky?
	VectorCopy( helo->s.origin, above );
	above[2] += 64;
	trace = gi.trace( helo->s.origin, NULL, NULL, above, helo, MASK_ALL );
	if( trace.fraction < 1.0 && ( trace.surface->flags & SURF_SKY ||
  		strncmp(trace.surface->name,"clip",strlen(trace.surface->name))== 0 ) )
	{
		if( helo->velocity[2] > 0 )
		{
			helo->velocity[2] = 0;
		}
	}

	// water-death
	if( helo->waterlevel )
	{
		VectorClear( helo->velocity );
		T_Damage( helo, world, world, vec3_origin, helo->s.origin, vec3_origin, 9999, 0, 0, MOD_SPLASH );
	}

	// check for landing
	if( !(helo->ONOFF & ONOFF_LANDED ) )
	{
		VectorCopy( helo->s.origin, below );
		below[2] = helo->s.origin[2] + helo->mins[2] - 2;
		trace = gi.trace( helo->s.origin, NULL, NULL, below, helo, MASK_ALL );

		if( strcmp( trace.ent->classname,"func_runway") == 0
		|| strcmp( trace.ent->classname, "func_door" ) == 0
		|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0
		|| ( trace.contents & CONTENTS_SOLID ) )
		{
			if( ( fabs(helo->speed) < 30 ) && ( helo->velocity[2] >= -60 ) &&
				(helo->ONOFF & ONOFF_GEAR) )
			{
				helo->ONOFF |= ONOFF_LANDED;
				helo->ONOFF &= ~ONOFF_AIRBORNE;
				helo->ONOFF &= ~ONOFF_AUTOPILOT;
				gi.cprintf( helo->owner, PRINT_HIGH, "You have landed.\n" );
				VectorClear( helo->velocity );
				VectorClear( helo->avelocity );
				if( helo->HASCAN & HASCAN_TAILDRAGGER )
					helo->s.angles[0] = -5;
				else
					helo->s.angles[0] = 0;
				helo->s.angles[2] = 0;
//				helo->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2] + 12;
//				helo->s.origin[2] = trace.endpos[2] + helo->s.origin[2] - (abs(helo->mins[2])+2);
				helo->s.origin[2] =  trace.endpos[2] + abs(helo->mins[2]) + 2;
			}
			else
			{
//				gi.bprintf( PRINT_HIGH, "%.1f %.1f\n", vehicle->actualspeed, vehicle->velocity[2] );
				if( helo->ONOFF & ONOFF_GEAR )
				{
					helo->ONOFF |= ONOFF_LANDED;
					helo->ONOFF &= ~ONOFF_AIRBORNE;
					if( fabs(helo->speed) >= 30 )
						T_Damage( helo, world, world, vec3_origin, helo->s.origin, 
							vec3_origin, (int)(fabs(helo->speed)-30), 0, 0, MOD_SPLASH );
					if( helo->velocity[2] < -60 )
						T_Damage( helo, world, world, vec3_origin, helo->s.origin, 
							vec3_origin, -(helo->velocity[2]+60), 0, 0, MOD_SPLASH );
					helo->s.angles[0] = -5;
					helo->s.angles[2] = 0;
					helo->s.origin[2] =  trace.endpos[2] + abs(helo->mins[2]) + 2;
				}
				else
				{
//					vehicle->ONOFF |= LANDED;
					T_Damage( helo, world, world, vec3_origin, helo->s.origin, vec3_origin, (int)fabs(helo->speed), 0, 0, MOD_SPLASH );
					T_Damage( helo, world, world, vec3_origin, helo->s.origin, vec3_origin, (helo->velocity[2])/2, 0, 0, MOD_SPLASH );
				}
				VectorClear( helo->avelocity );
				VectorClear( helo->velocity );
				MakeDamage( helo );
				gi.sound( helo, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
			}
		}
	}
	else
	{
		if( helo->thrust > THRUST_HOVER )
		{
			helo->ONOFF ^= ONOFF_LANDED;
			gi.cprintf( helo->owner, PRINT_HIGH, "Airborne.\n" );
		}
		else
		{
			VectorClear( helo->velocity );
			VectorClear( helo->avelocity );
		}
		VectorCopy( helo->s.origin, below );
		below[2] =  helo->s.origin[2] + helo->mins[2] - 3;
		trace = gi.trace( helo->s.origin, NULL, NULL, below, helo, MASK_ALL );
		if( trace.fraction == 1 )
		{
			helo->ONOFF ^= ONOFF_LANDED;
		}
	}

	// altitude
	VectorCopy( helo->s.origin, below );
	below[2] -= 2000;
	trace = gi.trace( helo->s.origin, NULL, NULL, below, helo, MASK_SOLID );
	if( trace.fraction < 1 )
		helo->altitude = ((int)(helo->s.origin[2] - trace.endpos[2]))-4;
	else
		helo->altitude = 9999;

	// lock weapons
	if( helo->weapon[helo->active_weapon] == WEAPON_SIDEWINDER ||
		helo->weapon[helo->active_weapon] == WEAPON_STINGER ||
		helo->weapon[helo->active_weapon] == WEAPON_AMRAAM ||
		helo->weapon[helo->active_weapon] == WEAPON_PHOENIX )
		AAM_Lock( helo );
	else if( helo->weapon[helo->active_weapon] == WEAPON_HELLFIRE ||
		helo->weapon[helo->active_weapon] == WEAPON_MAVERICK ||
		helo->weapon[helo->active_weapon] == WEAPON_ANTIRADAR ||
		helo->weapon[helo->active_weapon] == WEAPON_RUNWAYBOMBS ||
		helo->weapon[helo->active_weapon] == WEAPON_LASERBOMBS )
		ATGM_Lock( helo );
	else if( helo->weapon[helo->active_weapon] == WEAPON_AUTOCANNON )
		Autocannon_Lock( helo );
	else if( helo->weapon[helo->active_weapon] == WEAPON_JAMMER )
	{
		if( helo->ONOFF & ONOFF_JAMMER )
			Jammer_Active( helo );
		else
			Reload_Jammer( helo );
		if( helo->enemy )
		{
			helo->enemy->lockstatus = 0;
			helo->enemy = NULL;
		}
	}
	else if( helo->enemy )
	{
		helo->enemy->lockstatus = 0;
		helo->enemy = NULL;
	}

	// fire autocannon
	if( helo->owner->client->buttons & BUTTON_ATTACK )
		Weapon_Autocannon_Fire( helo );

	if( (helo->HASCAN & HASCAN_STEALTH) &&
		!(helo->ONOFF & ONOFF_WEAPONBAY) && 
		!(helo->ONOFF & ONOFF_GEAR) )
		helo->ONOFF |= ONOFF_STEALTH;
	else
		helo->ONOFF &= ~ONOFF_STEALTH;

//	gi.bprintf( PRINT_HIGH, "%s\n", (plane->ONOFF & ONOFF_STEALTH)?"Stealthy!":"Visible" );

	// resetting stuff
	helo->radio_target = NULL;
	helo->ONOFF &= ~ONOFF_IS_CHANGEABLE;
	helo->teammaster = NULL;

	helo->nextthink = level.time + 0.1;

	gi.linkentity( helo );
}


void HeloMovement_Easy( edict_t *helo )
{
	int		VM = (helo->owner->client->FLAGS & CLIENT_NONGL_MODE) ? -1 : 1;
	int		bay = 0;
	vec3_t	above, below;
	trace_t	trace;

	VectorClear( helo->avelocity );
	VectorClear( helo->velocity );

	// health-effects
	if( helo->owner->health >= 26 )
		helo->s.effects = 0;
	else if( helo->owner->health < 26 && helo->owner->health > 10 )
	{
		helo->s.effects = EF_GRENADE;
	}
	else if( helo->owner->health <= 10 && helo->deadflag != DEAD_DEAD )
	{
		helo->s.effects = EF_ROCKET;
	}
	else
		helo->s.effects = 0;

	// dead
	if( helo->deadflag )
	{
		if( helo->enemy )
		{
			helo->enemy->lockstatus = 0;
			helo->enemy = NULL;
		}
		helo->s.frame = 0;
		helo->s.renderfx = 0;
		helo->avelocity[2] = 120;
		helo->gravity = 0.3;
		helo->movetype = MOVETYPE_TOSS;
		return;
	}

	// overturning ?
	helo->s.angles[1] = anglemod( helo->s.angles[1] );

	// frames
	helo->s.renderfx = RF_TRANSLUCENT;
	if( helo->ONOFF & ONOFF_WEAPONBAY )
		bay = 1;
	if( helo->ONOFF & ONOFF_AIRBORNE ) // not landed
	{
		if( helo->ONOFF & ONOFF_GEAR )
		{
			if( helo->s.frame < 16 + 16 * bay )
				helo->s.frame ++;
			else
				helo->s.frame = 9 +  16 * bay;
		}
		else
		{
			if( helo->s.frame < 8 +  16 * bay )
				helo->s.frame ++;
			else
				helo->s.frame = 1 +  16 * bay;
		}
	}
	else		// landed
	{
		if( helo->ONOFF & ONOFF_PILOT_ONBOARD )
		{
			if( helo->s.frame < 16 + 16 * bay )
				helo->s.frame ++;
			else
				helo->s.frame = 9 + 16 * bay;
		}
		else
		{
			helo->s.renderfx = 0;
			helo->s.frame = 0;
		}
	}

	SetHeloSpeed( helo );

	// control input
	if( !(helo->ONOFF & ONOFF_LANDED) )
	{
		// rudder
		if( helo->rudder )
			helo->avelocity[1] = helo->rudder;

		// elevator
		if( helo->elevator )
		{
			if( helo->elevator > 0 )
			{
				if( helo->s.angles[0] < 55 && helo->s.angles[0] > -80 )
					helo->avelocity[0] = helo->elevator;
			}
			else if( helo->elevator < 0 )
			{
				if( helo->s.angles[0] > -35 && helo->s.angles[0] < 80 )
					helo->avelocity[0] = helo->elevator;
			}
		}

		if( helo->s.angles[0] > 55 )
			helo->s.angles[0] = 55;
		else if( helo->s.angles[0] < -35 )
			helo->s.angles[0] = -35;

		// aileron
		if( helo->aileron )
		{
			if( helo->aileron > 0 )
			{
				if( helo->s.angles[2]*VM < 30 && helo->s.angles[2]*VM > -60 )
					helo->avelocity[2] = helo->aileron*VM;
			}
			else if( helo->aileron < 0 )
			{
				if( helo->s.angles[2]*VM > -30 && helo->s.angles[2]*VM < 60 )
					helo->avelocity[2] = helo->aileron*VM;
			}
		}
		else
		{
			if( helo->s.angles[2] > 3 && helo->s.angles[2] < 90 )
				helo->avelocity[2] = -40;
			else if( helo->s.angles[2] < -3 && helo->s.angles[2] > -90 )
				helo->avelocity[2] = 40;
		}

		// yaw when banked at speed
		if( fabs(helo->s.angles[2]) > 5 && fabs(helo->s.angles[0]) > 12 )
		{
			helo->avelocity[1] += (helo->s.angles[2]*VM);
		}

		// auto-coord
//		if( vehicle->owner->client->pers.autocoord )
		{
//			vehicle->avelocity[1] += vehicle->aileron;
		}

		// auto-roll-back
		if( ((helo->owner->client->FLAGS & CLIENT_NONGL_MODE) && !helo->elevator)
				 || ( helo->ONOFF & ONOFF_AUTOPILOT ) )
		{
			if( helo->s.angles[0] > 2 && helo->s.angles[0] < 90 )
				helo->avelocity[0] = -20;
			else if( helo->s.angles[0] < -2 && helo->s.angles[0] > -90 )
				helo->avelocity[0] = 20;
		}
	}

	// did we touch the sky?
	VectorCopy( helo->s.origin, above );
	above[2] += 64;
	trace = gi.trace( helo->s.origin, NULL, NULL, above, helo, MASK_ALL );
	if( trace.fraction < 1.0 && ( trace.surface->flags & SURF_SKY ||
  		strncmp(trace.surface->name,"clip",strlen(trace.surface->name))== 0 ) )
	{
		if( helo->velocity[2] > 0 )
		{
			helo->velocity[2] = 0;
		}
	}

	// water-death
	if( helo->waterlevel )
	{
		VectorClear( helo->velocity );
		T_Damage( helo, world, world, vec3_origin, helo->s.origin, vec3_origin, 9999, 0, 0, MOD_SPLASH );
	}

	// check for landing
	if( !(helo->ONOFF & ONOFF_LANDED ) )
	{
		VectorCopy( helo->s.origin, below );
		below[2] = helo->s.origin[2] + helo->mins[2] - 2;
		trace = gi.trace( helo->s.origin, NULL, NULL, below, helo, MASK_ALL );

		if( strcmp( trace.ent->classname,"func_runway") == 0
		|| strcmp( trace.ent->classname, "func_door" ) == 0
		|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0
		|| ( trace.contents & CONTENTS_SOLID ) )
		{
			if( ( fabs(helo->speed) < 30 ) && ( helo->velocity[2] >= -60 ) &&
				(helo->ONOFF & ONOFF_GEAR) )
			{
				helo->ONOFF |= ONOFF_LANDED;
				helo->ONOFF &= ~ONOFF_AIRBORNE;
				helo->ONOFF &= ~ONOFF_AUTOPILOT;
				gi.cprintf( helo->owner, PRINT_HIGH, "You have landed.\n" );
				VectorClear( helo->velocity );
				VectorClear( helo->avelocity );
				if( helo->HASCAN & HASCAN_TAILDRAGGER )
					helo->s.angles[0] = -5;
				else
					helo->s.angles[0] = 0;
				helo->s.angles[2] = 0;
//				helo->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2] + 12;
//				helo->s.origin[2] = trace.endpos[2] + helo->s.origin[2] - (abs(helo->mins[2])+2);
				helo->s.origin[2] =  trace.endpos[2] + abs(helo->mins[2]) + 2;
			}
			else
			{
//				gi.bprintf( PRINT_HIGH, "%.1f %.1f\n", vehicle->actualspeed, vehicle->velocity[2] );
				if( helo->ONOFF & ONOFF_GEAR )
				{
					helo->ONOFF |= ONOFF_LANDED;
					helo->ONOFF &= ~ONOFF_AIRBORNE;
					if( fabs(helo->speed) >= 30 )
						T_Damage( helo, world, world, vec3_origin, helo->s.origin, 
							vec3_origin, (int)(fabs(helo->speed)-30), 0, 0, MOD_SPLASH );
					if( helo->velocity[2] < -60 )
						T_Damage( helo, world, world, vec3_origin, helo->s.origin, 
							vec3_origin, -(helo->velocity[2]+60), 0, 0, MOD_SPLASH );
					helo->s.angles[0] = -5;
					helo->s.angles[2] = 0;
					helo->s.origin[2] =  trace.endpos[2] + abs(helo->mins[2]) + 2;
				}
				else
				{
//					vehicle->ONOFF |= LANDED;
					T_Damage( helo, world, world, vec3_origin, helo->s.origin, vec3_origin, (int)fabs(helo->speed), 0, 0, MOD_SPLASH );
					T_Damage( helo, world, world, vec3_origin, helo->s.origin, vec3_origin, (helo->velocity[2])/2, 0, 0, MOD_SPLASH );
				}
				VectorClear( helo->avelocity );
				VectorClear( helo->velocity );
				MakeDamage( helo );
				gi.sound( helo, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
			}
		}
	}
	else
	{
		if( helo->thrust > THRUST_HOVER )
		{
			helo->ONOFF ^= ONOFF_LANDED;
			gi.cprintf( helo->owner, PRINT_HIGH, "Airborne.\n" );
		}
		else
		{
			VectorClear( helo->velocity );
			VectorClear( helo->avelocity );
		}
		VectorCopy( helo->s.origin, below );
		below[2] =  helo->s.origin[2] + helo->mins[2] - 3;
		trace = gi.trace( helo->s.origin, NULL, NULL, below, helo, MASK_ALL );
		if( trace.fraction == 1 )
		{
			helo->ONOFF ^= ONOFF_LANDED;
		}
	}

	// altitude
	VectorCopy( helo->s.origin, below );
	below[2] -= 2000;
	trace = gi.trace( helo->s.origin, NULL, NULL, below, helo, MASK_SOLID );
	if( trace.fraction < 1 )
		helo->altitude = ((int)(helo->s.origin[2] - trace.endpos[2]))-4;
	else
		helo->altitude = 9999;

#pragma message( "check why crosshair stays green after death of enemy and weapon change" )
#pragma message( "check for take off and landing" )
#pragma message( "check why you fall down after spawing in helo, not correct onground check ?" )

	// lock weapons
	if( helo->weapon[helo->active_weapon] == WEAPON_SIDEWINDER ||
		helo->weapon[helo->active_weapon] == WEAPON_STINGER ||
		helo->weapon[helo->active_weapon] == WEAPON_AMRAAM ||
		helo->weapon[helo->active_weapon] == WEAPON_PHOENIX )
		AAM_Lock( helo );
	else if( helo->weapon[helo->active_weapon] == WEAPON_HELLFIRE ||
		helo->weapon[helo->active_weapon] == WEAPON_MAVERICK ||
		helo->weapon[helo->active_weapon] == WEAPON_ANTIRADAR ||
		helo->weapon[helo->active_weapon] == WEAPON_RUNWAYBOMBS ||
		helo->weapon[helo->active_weapon] == WEAPON_LASERBOMBS )
		ATGM_Lock( helo );
	else if( helo->weapon[helo->active_weapon] == WEAPON_AUTOCANNON )
		Autocannon_Lock( helo );
	else if( helo->weapon[helo->active_weapon] == WEAPON_JAMMER )
	{
		if( helo->ONOFF & ONOFF_JAMMER )
			Jammer_Active( helo );
		else
			Reload_Jammer( helo );
		if( helo->enemy )
		{
			helo->enemy->lockstatus = 0;
			helo->enemy = NULL;
		}
	}
	else if( helo->enemy )
	{
		helo->enemy->lockstatus = 0;
		helo->enemy = NULL;
	}

	// fire autocannon
	if( helo->owner->client->buttons & BUTTON_ATTACK )
		Weapon_Autocannon_Fire( helo );

	if( (helo->HASCAN & HASCAN_STEALTH) &&
		!(helo->ONOFF & ONOFF_WEAPONBAY) && 
		!(helo->ONOFF & ONOFF_GEAR) )
		helo->ONOFF |= ONOFF_STEALTH;
	else
		helo->ONOFF &= ~ONOFF_STEALTH;

//	gi.bprintf( PRINT_HIGH, "%s\n", (plane->ONOFF & ONOFF_STEALTH)?"Stealthy!":"Visible" );

	// resetting stuff
	helo->radio_target = NULL;
	helo->ONOFF &= ~ONOFF_IS_CHANGEABLE;
	helo->teammaster = NULL;

	helo->nextthink = level.time + 0.1;

	gi.linkentity( helo );
}



void HeloTouch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
#pragma message( "properly implement helo touch" )
/*
	if( strcmp( other->classname, "trigger_radio" ) != 0 )
	{
		self->radio_target = NULL;
	}
	if( self->deadflag == DEAD_DEAD || strcmp( other->classname, "freed" ) == 0
		|| strcmp( other->classname, "debris" ) == 0 )
		return;
	if( strcmp( other->classname, "rocket" ) == 0 ||
   		strcmp( other->classname, "sidewinder" ) == 0 ||
		strcmp( other->classname, "ATGM" ) == 0 ||
		strcmp( other->classname, "bolt" ) == 0 ||
		strcmp( other->classname, "shell" ) == 0 ||
		strcmp( other->classname, "bomb" ) == 0 )
	{
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		return;
	}
	if( self->deadflag == DEAD_DYING )
	{
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		ThrowDebris( self, "models/objects/debris2/tris.md2",
			5, self->s.origin );
		ThrowDebris( self, "models/objects/debris2/tris.md2",
			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris3/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//			5, self->s.origin );
		VehicleExplosion( self );
		SetVehicleModel( self );
		self->deadflag = DEAD_DEAD;
	}
	if( surf && (surf->flags & SURF_SKY) )
	{
//		self->health -= 5;
		if( self->health > 0 )
			return;
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		ThrowDebris( self, "models/objects/debris2/tris.md2",
			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris3/tris.md2",
//			5, self->s.origin );
		ThrowDebris( self, "models/objects/debris2/tris.md2",
			5, self->s.origin );
		VehicleExplosion( self );
		SetVehicleModel( self );
		if( !self->deadflag )
			meansOfDeath = MOD_CRASH;
		self->deadflag = DEAD_DEAD;
//		gi.bprintf( PRINT_MEDIUM, "%s crashed.\n", self->owner->client->pers.netname );

	}
	if( strcmp( other->classname, "func_runway") == 0 && self->velocity[2] > -62 )
		return;
	if( strcmp( other->classname, "trigger_recharge") == 0 )
		return;
	else if( ( strcmp( other->classname, "plane") == 0 || strcmp( other->classname, "helo" ) == 0
				|| strcmp( other->classname, "ground") == 0 ) &&
			   	 !(other->HASCAN & IS_DRONE) && (other->ONOFF & PILOT_ONBOARD) )
	{
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		if( !self->deadflag )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			self->deadflag = DEAD_DEAD;
			ThrowDebris( self, "models/objects/debris1/tris.md2",
				5, self->s.origin );
			ThrowDebris( self, "models/objects/debris2/tris.md2",
				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris3/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
			VehicleExplosion( self );
			SetVehicleModel( self );
			gi.bprintf( PRINT_MEDIUM, "%s crashed into %s\n", self->owner->client->pers.netname,
						other->owner->client->pers.netname );
			meansOfDeath = MOD_MIDAIR;
		}
	}
	else if( ( strcmp( other->classname, "plane" ) == 0 ||
	   		strcmp( other->classname, "helo" ) == 0 ||
		    strcmp( other->classname, "ground" ) == 0 ) &&
			!(other->ONOFF & PILOT_ONBOARD) &&
			!(other->HASCAN & IS_DRONE) )
	{
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		if( !self->deadflag )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			self->deadflag = DEAD_DEAD;
			ThrowDebris( self, "models/objects/debris1/tris.md2",
				5, self->s.origin );
			ThrowDebris( self, "models/objects/debris2/tris.md2",
				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris3/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
			VehicleExplosion( self );
			SetVehicleModel( self );
			gi.bprintf( PRINT_MEDIUM, "%s crashed into a parked vehicle\n", self->owner->client->pers.netname);
			meansOfDeath = MOD_MIDAIR;
		}
	}
	else if( other->HASCAN & IS_DRONE )
	{
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		if( !self->deadflag )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			self->deadflag = DEAD_DEAD;
			ThrowDebris( self, "models/objects/debris1/tris.md2",
				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris3/tris.md2",
//				5, self->s.origin );
			ThrowDebris( self, "models/objects/debris2/tris.md2",
				5, self->s.origin );
			VehicleExplosion( self );
			SetVehicleModel( self );
			gi.bprintf( PRINT_MEDIUM, "%s crashed into a drone (%s)\n", self->owner->client->pers.netname,
					other->friendlyname);
			meansOfDeath = MOD_MIDAIR;
		}
	}
	else
	{
		if( fabs(self->actualspeed) >= 200 )
		{
			if( fabs(self->actualspeed) >= 200 )
				self->health -= ((fabs(self->actualspeed)-200)/4);
			VectorClear( self->avelocity );
			VectorClear( self->velocity );
			gi.sound( self, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
			MakeDamage( self );
			if( self->health <= 0 )
			{
				if( self->enemy )
				{
					self->enemy->lockstatus = 0;
					self->enemy = NULL;
				}
				gi.WriteByte (svc_temp_entity);
				gi.WriteByte (TE_EXPLOSION1);
				gi.WritePosition (self->s.origin);
				gi.multicast (self->s.origin, MULTICAST_PVS);
				ThrowDebris( self, "models/objects/debris2/tris.md2",
					5, self->s.origin );
//				ThrowDebris( self, "models/objects/debris2/tris.md2",
//					5, self->s.origin );
//				ThrowDebris( self, "models/objects/debris3/tris.md2",
//					5, self->s.origin );
				ThrowDebris( self, "models/objects/debris2/tris.md2",
					5, self->s.origin );
				VehicleExplosion( self );
				SetVehicleModel( self );
				if( !self->deadflag )
					meansOfDeath = MOD_CRASH;
				self->deadflag = DEAD_DEAD;
				if( other->HASCAN && IS_DRONE )
					gi.bprintf( PRINT_MEDIUM, "%s crashed into a drone (%s)!\n",
						self->owner->client->pers.netname, other->friendlyname );
				else
					gi.bprintf( PRINT_MEDIUM, "%s crashed.\n", self->owner->client->pers.netname );
			}
		}
		if( self->velocity[2] > 0 )
		{
			if( self->enemy )
			{
				self->enemy->lockstatus = 0;
				self->enemy = NULL;
			}
			self->health = 0;
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			ThrowDebris( self, "models/objects/debris2/tris.md2",
				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris3/tris.md2",
//				5, self->s.origin );
			ThrowDebris( self, "models/objects/debris2/tris.md2",
				5, self->s.origin );
			VehicleExplosion( self );
			SetVehicleModel( self );
			if( !self->deadflag )
				meansOfDeath = MOD_CRASH;
			self->deadflag = DEAD_DEAD;
			if( other->HASCAN && IS_DRONE )
				gi.bprintf( PRINT_MEDIUM, "%s crashed into a drone (%s)!\n",
					self->owner->client->pers.netname, other->friendlyname );
			else
				gi.bprintf( PRINT_MEDIUM, "%s crashed.\n", self->owner->client->pers.netname );

		}
	}*/
}



void HeloObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
#pragma message( "properly implement obituary for helo" )
/*
	int		mod = meansOfDeath;
	char	*message = NULL, *message2 = NULL, *message3 = "";
	int		print = 0;

//	gi.bprintf( PRINT_HIGH, "Plane obituary!\n" );

	if( !(self->HASCAN & HASCAN_SCORED) && !(self->HASCAN & HASCAN_SCORED_DEAD) )
		print = 1;

	// CLEAN THIS UGLY CODE UP! EASY, JUST CHECK IF THERE IS AN ATTACKER OR NOT!

	switch( mod )
	{
	case MOD_AUTOCANNON:
		if( strcmp( attacker->classname, "LQM" ) == 0 )
		{
			message = "was shot down by";
			message3 = "'s puny machinegun";
		}
		else
			message = "was outgunned by";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_SIDEWINDER:	
		message = "was blown apart by";
		message3 = "'s Sidewinder";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_STINGER:		
		message = "was disintegrated by";
		message3 = "'s Stinger";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_AMRAAM:	
		message = "got to know";
		message3 = "'s AMRAAM";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_PHOENIX:		
		message = "couldn't outrun";
		message3 = "'s Phoenix";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_HELLFIRE:	
		message = "was punted away by";
		message3 = "'s Hellfire";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_MAVERICK:	
		message = "was hunted down by";
		message3 = "'s Maverick";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_ANTIRADAR:	
		message = "stopped emitting after being hit by";
		message3 = "'s Anti-Radar Missile";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_ROCKET:
		message = "met an old friends";
		message3 = "'s Rocket";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_LASERBOMBS:
		message = "couldn't get rid of";
		message3 = "'s LGB";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_RUNWAYBOMBS:
		message = "feels like an old runway because of";
		message3 = "'s Durandal";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	case MOD_BOMBS:
		message = "was blasted apart by";
		message3 = "'s Iron Bombs";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
		message2 = botinfo[attacker->botnumber].botname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->count += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->count += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
						attacker->count += 1;
				self->HASCAN |= (HASCAN_SCORED_DEAD|HASCAN_SCORED);
				}
			}
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( self->deadflag == DEAD_DYING )
			{
				if( !(self->HASCAN & HASCAN_SCORED) )
				{
					attacker->owner->client->resp.score += 2;
					self->HASCAN |= HASCAN_SCORED;
				}
			}
			else if( self->deadflag == DEAD_DEAD )
			{
				if( !(self->HASCAN & HASCAN_SCORED ) )
				{
					attacker->owner->client->resp.score += 3;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
				else if( !(self->HASCAN & HASCAN_SCORED_DEAD) )
				{
					attacker->owner->client->resp.score += 1;
					self->HASCAN |= (HASCAN_SCORED|HASCAN_SCORED_DEAD);
				}
			}
		}
		break;
	default:
		message = "died";
		message2 = "";
		message3 = "";
		break;
	}

	if( print )
		gi.bprintf( PRINT_HIGH, "%s %s %s%s\n", self->owner->client->pers.netname, message, message2, message3 );
*/
}



void HeloDie (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
#pragma message( "properly implement helo die" )
//	gi.bprintf( PRINT_HIGH, "Helo die!\n" );

/*	if( self->ONOFF & ONOFF_AIRBORNE )
	{
		if( self->health <= self->gib_health )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			SetVehicleModel( self );
			VehicleExplosion( self );
			self->deadflag = DEAD_DEAD;
			PlaneObituary( self, inflictor, attacker );
		}
		else
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			self->deadflag = DEAD_DYING;
			PlaneObituary( self, inflictor, attacker );
		}
	}
	else
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION2);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		SetVehicleModel(self);
		VehicleExplosion( self );
		self->deadflag = DEAD_DEAD;
		PlaneObituary( self, inflictor, attacker );
	}

	if( self->deadflag == DEAD_DEAD )
		self->owner->client->respawn_time = level.time + 2.0;
	self->s.frame = 0;
	gi.linkentity( self );*/
}