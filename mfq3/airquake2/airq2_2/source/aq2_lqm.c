/************************************************/
/* Filename: aq2_lqm.cpp					    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 1999-07-10							*/
/*												*/
/* Description:									*/
/* NewAirQuake2 lqm-specific stuff      		*/
/*												*/
/************************************************/

#include "g_local.h"



void SteerLQM_Alt( edict_t *vehicle)
{
	vec3_t	hSpeed, forward, up, below;
	trace_t	tr;
	int		VM = (vehicle->owner->client->FLAGS & CLIENT_NONGL_MODE) ? -1 : 1;

//	if( vehicle->groundentity )
//		gi.bprintf( PRINT_HIGH, "%s\n", vehicle->groundentity->classname );

	VectorClear( vehicle->avelocity );

	vehicle->s.angles[0] = anglemod( vehicle->s.angles[0] );
	vehicle->s.angles[1] = anglemod( vehicle->s.angles[1] );


	if( vehicle->ONOFF & ONOFF_AIRBORNE )// flying
	{
		if( vehicle->ONOFF & ONOFF_AUTOROTATION ) // no parachute
		{
			// open chute
			if( level.time > vehicle->drone_decision_time && !vehicle->deadflag)
			{
				if( !vehicle->waterlevel )
				{
					vehicle->s.frame = 0;
					vehicle->s.angles[2] = 0;
					VectorClear( vehicle->velocity );
					vehicle->s.modelindex3 = gi.modelindex ("vehicles/infantry/pilot/parachute.md2");
					vehicle->ONOFF &= ~(ONOFF_AUTOROTATION);
				}
				else
				{
					vehicle->s.angles[2] = vehicle->s.angles[0] = 0;
					vehicle->ONOFF &= ~ONOFF_AIRBORNE;
					vehicle->ONOFF |= ONOFF_LANDED;
				}
			}
			if( vehicle->avelocity[0] < 0 )
				vehicle->avelocity[0] += 5;
			else
				vehicle->avelocity[0] = 0;
			if( fabs( vehicle->avelocity[2]*VM ) > 5 )
			{
				if( vehicle->avelocity[2]*VM > 0 )
					vehicle->avelocity[2] -= 2*VM;
				else
					vehicle->avelocity[2] += 2*VM;
			}
			else
				vehicle->avelocity[2] = 0;
			if( fabs( vehicle->avelocity[1] ) > 5 )
			{
				if( vehicle->avelocity[1] > 0 )
					vehicle->avelocity[1] -= 2;
				else
					vehicle->avelocity[1] += 2;
			}
			else
				vehicle->avelocity[1] = 0;
			// did we touch the sky?
			VectorCopy( vehicle->s.origin, up );
			up[2] += 64;
			tr = gi.trace( vehicle->s.origin, NULL, NULL, up, vehicle, MASK_ALL );
			if( tr.fraction < 1.0 && (tr.surface->flags & SURF_SKY ||
		   		strncmp(tr.surface->name,"clip",strlen(tr.surface->name))== 0) )
			{
				vehicle->s.origin[2] -= 1;
				vehicle->velocity[2] = 0;
			}
			// make speed
			VectorCopy( vehicle->velocity, hSpeed );
			hSpeed[2] = 0;
			VectorScale( hSpeed, 0.9, hSpeed );
			hSpeed[2] = vehicle->velocity[2];
			VectorCopy( hSpeed, vehicle->velocity );
			SV_AddGravity( vehicle );
			if( vehicle->waterlevel > 1 )
				vehicle->ONOFF &= ~ONOFF_AIRBORNE;
			// fire weapons
			if( vehicle->owner->client->buttons & BUTTON_ATTACK )
				Weapon_Autocannon_Fire( vehicle );
		}
		else	// chute open
		{
			// steer the chute
			if( !vehicle->deadflag )
			{
				vehicle->avelocity[1] = vehicle->aileron;
				if( vehicle->avelocity[1] > 10 )
				{
					if( vehicle->s.angles[2]*VM < 30 )
						vehicle->avelocity[2] = 15*VM;
					else
						vehicle->avelocity[2] = 0;
				}
				else if( vehicle->avelocity[1] < -10 )
				{
					if( vehicle->s.angles[2]*VM > -30 )
						vehicle->avelocity[2] = -15*VM;
					else
						vehicle->avelocity[2] = 0;
				}
				vehicle->avelocity[0] = vehicle->elevator;
				if( vehicle->avelocity[0] > 10 )
				{
					if( vehicle->s.angles[0] < 40 )
						vehicle->avelocity[0] = 45;
					else if( vehicle->s.angles[0] > 40 && vehicle->s.angles[0] < 90 )
						vehicle->avelocity[0] = 0;
				}
				else if( vehicle->avelocity[0] < -10 )
				{
					if( vehicle->s.angles[0] > 350 )
						vehicle->avelocity[0] = -45;
					else if( vehicle->s.angles[0] < 350 && vehicle->s.angles[0] > 270 )
						vehicle->avelocity[0] = 0;
				}
			}
//			gi.bprintf( PRINT_HIGH, "%.0f\n", vehicle->s.angles[0] );
			if( vehicle->deadflag || !vehicle->aileron )
			{
				if( fabs(vehicle->s.angles[2]*VM) > 10 && fabs(vehicle->s.angles[2]*VM) < 90)
				{
					if( vehicle->s.angles[2]*VM > 0 )
						vehicle->avelocity[2] = -55*VM;
					else
						vehicle->avelocity[2] = 55*VM;
				}
				else if( fabs(vehicle->s.angles[2]*VM) > 90 && fabs(vehicle->s.angles[2]*VM) < 170)
				{
					if( vehicle->s.angles[2]*VM > 180 )
						vehicle->avelocity[2] = -55*VM;
					else
						vehicle->avelocity[2] = 55*VM;
				}
				else
					vehicle->s.angles[2] = 0;
			}
			if( vehicle->deadflag || !vehicle->elevator )
			{
				if( vehicle->s.angles[0] >= 270 || vehicle->s.angles[0] < 90 )
				{
					if( vehicle->s.angles[0] < 90 && vehicle->s.angles[0] > 6 )
						vehicle->avelocity[0] = -55;
					else if( vehicle->s.angles[0] >= 270 && vehicle->s.angles[0] < 354 )
						vehicle->avelocity[0] = 55;
					else
						vehicle->s.angles[0] = 0;
				}
				else if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] >= 90 )
				{
					if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] > 190 )
						vehicle->avelocity[0] = -55;
					else if( vehicle->s.angles[0] >= 90 && vehicle->s.angles[0] < 170 )
						vehicle->avelocity[0] = 55;
					else
						vehicle->s.angles[0] = 0;
				}
				if( vehicle->velocity[2] >= -40 )
					vehicle->velocity[2] -= 3;
				else if( vehicle->velocity[2] < -42 && !(vehicle->ONOFF & ONOFF_STALLED) )
					vehicle->velocity[2] += 2;
			}
			else if( vehicle->elevator && vehicle->s.angles[0] > 40 && vehicle->s.angles[0] < 350 )
			{
				if( vehicle->s.angles[0] >= 270 || vehicle->s.angles[0] < 90 )
				{
					if( vehicle->s.angles[0] < 90 && vehicle->s.angles[0] > 45 )
						vehicle->avelocity[0] = -55;
					else if( vehicle->s.angles[0] >= 270 && vehicle->s.angles[0] < 345 )
						vehicle->avelocity[0] = 55;
				}
				else if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] >= 90 )
				{
					if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] > 190 )
						vehicle->avelocity[0] = -55;
					else if( vehicle->s.angles[0] >= 90 && vehicle->s.angles[0] < 170 )
						vehicle->avelocity[0] = 55;
					else
						vehicle->s.angles[0] = 0;
				}
				if( vehicle->velocity[2] >= -40 )
					vehicle->velocity[2] -= 3;
				else if( vehicle->velocity[2] < -42 && !(vehicle->ONOFF & ONOFF_STALLED) )
					vehicle->velocity[2] += 2;
			}

			AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			forward[2] = 0;
			VectorNormalize( forward );
			if( vehicle->s.angles[0] >= 0 && vehicle->s.angles[0] < 50 )
			{
				VectorScale( forward, vehicle->s.angles[0], forward );
				if( vehicle->velocity[2] > -80 )
					vehicle->velocity[2] -= (vehicle->s.angles[0]/10);
			}
			else 
			{
				VectorScale( forward, 0, forward );
				if( vehicle->s.angles[0] < 360 && vehicle->s.angles[0] > 310 && vehicle->velocity[2] < 5 )
				{
					if( !(vehicle->ONOFF & ONOFF_STALLED) )
						vehicle->velocity[2] += ((360-vehicle->s.angles[0])/2);
					else
						vehicle->velocity[2] -= 3;
				}
			}
			forward[2] = vehicle->velocity[2];

			VectorCopy( forward, vehicle->velocity );

			if( VectorLength( vehicle->velocity ) < 30 )
				vehicle->ONOFF |= ONOFF_STALLED;
			else
				vehicle->ONOFF &= ~ONOFF_STALLED;

			if( vehicle->waterlevel )
			{
				vehicle->s.angles[0] = vehicle->s.angles[2] = 0;

				vehicle->s.frame = 1;
				vehicle->ONOFF |= ONOFF_LANDED;
				vehicle->ONOFF &= ~(ONOFF_AIRBORNE);
				vehicle->s.modelindex3 = 0;
//				vehicle->s.origin[2] += 2;
			}
//			gi.bprintf( PRINT_HIGH, "%.0f\n", vehicle->velocity[2] );
		}
		vehicle->ideal_yaw = 0;
	}
	else	// on ground
	{
		vehicle->s.angles[0] = vehicle->s.angles[2] = 0;
		// drowning ?
		if( vehicle->wait )
		{
			if( level.time - vehicle->wait >= 40 )
				T_Damage( vehicle, vehicle, vehicle, vec3_origin, vehicle->s.origin, vec3_origin, 1, 0, 0, MOD_WATER );
			else if( level.time - vehicle->wait >= 35 && level.time > vehicle->random )
			{
				gi.sound( vehicle, CHAN_BODY, gi.soundindex( "player/male/gurp1.wav" ), 1.0, ATTN_NORM, 0 );
				vehicle->random = level.time + 1.0;
			}
//			gi.bprintf( PRINT_HIGH, "%.1f\n", level.time - vehicle->wait );
		}
		if( !vehicle->deadflag )
		{
			if( !vehicle->thrust && !vehicle->rudder)
			{
				vehicle->s.frame++;
				if( vehicle->s.frame > 31 )
					vehicle->s.frame = 1;
			}
			else
			{
				if( vehicle->thrust > 0 )
				{
					vehicle->s.frame++;
					if( vehicle->s.frame < 32 || vehicle->s.frame > 37 )
						vehicle->s.frame = 32;
				}
				else
				{
					vehicle->s.frame--;
					if( vehicle->s.frame < 32 || vehicle->s.frame > 37 )
						vehicle->s.frame = 37;
				}
			}
		}
		else
		{
			if( vehicle->s.frame < 43 )
				vehicle->s.frame = 43;
			if( vehicle->s.frame < 56 )
				vehicle->s.frame ++;
			if( vehicle->s.frame == 56 )
			{
				vehicle->deadflag = DEAD_DEAD;
				if( vehicle->s.angles[0] <= 90 || vehicle->s.angles[0] > 270 )
					vehicle->s.angles[0] = 0;
				else 
					vehicle->s.angles[0] = 180;
				vehicle->solid = SOLID_NOT;
				return;
			}
		}
		VectorCopy( vehicle->s.origin, up );
		up[2] += 6;
		VectorCopy( vehicle->s.origin, below );
		if( !vehicle->waterlevel )
			below[2] -= 18;
		else
		{
			vehicle->ONOFF &= ~ONOFF_BRAKE;
			below[2] -= 2;
		}
		tr = gi.trace( up, vehicle->mins, vehicle->maxs, below, vehicle, MASK_PLAYERSOLID );

		if( tr.fraction < 1 )
		{
			vehicle->s.origin[2] = tr.endpos[2] + fabs(vehicle->mins[2]);// + 2;
			vehicle->groundentity = tr.ent;
		}
		else
		{
			if( !vehicle->waterlevel )
			{
				vehicle->ONOFF |= (ONOFF_AIRBORNE|ONOFF_AUTOROTATION|ONOFF_GEAR);	// gear lets you manually open chute
				vehicle->thrust = 0;
				vehicle->drone_decision_time = level.time + 15;
			}
		}
		if( vehicle->deadflag )
		{
//			vehicle->deadflag = DEAD_DEAD;
			vehicle->thrust = 0;
			vehicle->speed = 0;
			vehicle->nextthink = level.time + 0.1;
			return;
		}
		if( vehicle->elevator )
		{
			vehicle->ideal_yaw += (vehicle->elevator*FRAMETIME);
			if( vehicle->ideal_yaw > 69 )
				vehicle->ideal_yaw = 69;
			else if( vehicle->ideal_yaw < -69 )
				vehicle->ideal_yaw = -69;
		}
//		gi.bprintf( PRINT_HIGH, "%.0f\n", vehicle->ideal_yaw );

		// give speed
		if( vehicle->owner->client->REPEAT & REPEAT_INCREASE )
			vehicle->thrust = vehicle->maxthrust;
		else if( vehicle->owner->client->REPEAT & REPEAT_DECREASE )
			vehicle->thrust = vehicle->minthrust;
		else
			vehicle->thrust = 0;
		vehicle->speed = vehicle->thrust * (vehicle->ONOFF & ONOFF_BRAKE ? 12 : 7);
		if( vehicle->waterlevel )
		{
			vehicle->s.angles[0] = vehicle->ideal_yaw;
			if( vehicle->waterlevel == 1 && vehicle->ideal_yaw < 0 )
				vehicle->s.angles[0] = 0;
			vehicle->speed /= 2;
		}
		vehicle->avelocity[1] = vehicle->aileron;
		AngleVectors( vehicle->s.angles, forward, up, NULL );
		vehicle->s.angles[0] = 0;
		VectorScale( forward, vehicle->speed, vehicle->velocity );
		if( vehicle->rudder )
		{
			VectorMA( vehicle->velocity, (vehicle->rudder > 0 ? -12 : 12), up, vehicle->velocity );
/*			if( !vehicle->thrust )
			{
				vehicle->s.frame++;
				if( vehicle->s.frame < 32 || vehicle->s.frame > 37 )
					vehicle->s.frame = 32;
			}
*/		}
			// jump?
		if( (vehicle->ONOFF & ONOFF_BRAKE) && !vehicle->waterlevel )
		{
			vehicle->velocity[2] += 32;
			vehicle->s.origin[2] += 2;
			vehicle->ONOFF &= ~ONOFF_LANDED;
			vehicle->ONOFF |= (ONOFF_AIRBORNE|ONOFF_AUTOROTATION|ONOFF_GEAR);
			vehicle->ONOFF &= ~ONOFF_BRAKE;
			vehicle->drone_decision_time = level.time + 15;
			vehicle->s.frame = 35;
		}
		VectorNormalize( forward );
		VectorScale( forward, 15, forward );
		VectorAdd( vehicle->s.origin, forward, up );
		tr = gi.trace( vehicle->s.origin, NULL, NULL, up, vehicle, MASK_ALL );
		if( tr.fraction < 1 )
		{
			if( ( strcmp( tr.ent->classname, "plane" ) == 0 ||
				strcmp( tr.ent->classname, "helo" ) == 0 ||
				strcmp( tr.ent->classname, "ground" ) == 0 )
				&& !(tr.ent->ONOFF & ONOFF_PILOT_ONBOARD) )
			{
				vehicle->activator = tr.ent;
			}
		}
		else
		{
			vehicle->activator = NULL;
		}
		// lock weapons
		if( vehicle->weapon[vehicle->active_weapon] == WEAPON_SIDEWINDER ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_STINGER ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_AMRAAM ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_PHOENIX )
			AAM_Lock( vehicle );
		else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_HELLFIRE ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_MAVERICK ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_ANTIRADAR )
			ATGM_Lock( vehicle );
		else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_AUTOCANNON )
			Autocannon_Lock( vehicle );
		else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_JAMMER )
		{
			if( vehicle->ONOFF & ONOFF_JAMMER )
				Jammer_Active( vehicle );
			else
				Reload_Jammer( vehicle );
			if( vehicle->enemy )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		else if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		// fire weapons
		if( vehicle->owner->client->buttons & BUTTON_ATTACK )
			Weapon_Autocannon_Fire( vehicle );
	}

	vehicle->nextthink = level.time + 0.1;
	vehicle->radio_target = NULL;
	vehicle->ONOFF &= ~ONOFF_IS_CHANGEABLE;
	vehicle->teammaster = NULL;
}





void SteerLQM( edict_t *vehicle)
{
	vec3_t	hSpeed, forward, up, below;
	trace_t	tr;
	int		VM = (vehicle->owner->client->FLAGS & CLIENT_NONGL_MODE) ? -1 : 1;

//	if( vehicle->groundentity )
//		gi.bprintf( PRINT_HIGH, "%s\n", vehicle->groundentity->classname );

	VectorClear( vehicle->avelocity );

	vehicle->s.angles[0] = anglemod( vehicle->s.angles[0] );
	vehicle->s.angles[1] = anglemod( vehicle->s.angles[1] );


	if( vehicle->ONOFF & ONOFF_AIRBORNE )// flying
	{
		if( vehicle->ONOFF & ONOFF_AUTOROTATION ) // no parachute
		{
			// open chute
			if( level.time > vehicle->drone_decision_time && !vehicle->deadflag)
			{
				if( !vehicle->waterlevel )
				{
					vehicle->s.frame = 0;
					vehicle->s.angles[2] = 0;
					VectorClear( vehicle->velocity );
					vehicle->s.modelindex3 = gi.modelindex ("vehicles/infantry/pilot/parachute.md2");
					vehicle->ONOFF &= ~(ONOFF_AUTOROTATION);
				}
				else
				{
					vehicle->s.angles[2] = vehicle->s.angles[0] = 0;
					vehicle->ONOFF &= ~ONOFF_AIRBORNE;
					vehicle->ONOFF |= ONOFF_LANDED;
				}
			}
			if( vehicle->avelocity[0] < 0 )
				vehicle->avelocity[0] += 5;
			else
				vehicle->avelocity[0] = 0;
			if( fabs( vehicle->avelocity[2]*VM ) > 5 )
			{
				if( vehicle->avelocity[2]*VM > 0 )
					vehicle->avelocity[2] -= 2*VM;
				else
					vehicle->avelocity[2] += 2*VM;
			}
			else
				vehicle->avelocity[2] = 0;
			if( fabs( vehicle->avelocity[1] ) > 5 )
			{
				if( vehicle->avelocity[1] > 0 )
					vehicle->avelocity[1] -= 2;
				else
					vehicle->avelocity[1] += 2;
			}
			else
				vehicle->avelocity[1] = 0;
			// did we touch the sky?
			VectorCopy( vehicle->s.origin, up );
			up[2] += 64;
			tr = gi.trace( vehicle->s.origin, NULL, NULL, up, vehicle, MASK_ALL );
			if( tr.fraction < 1.0 && (tr.surface->flags & SURF_SKY ||
		   		strncmp(tr.surface->name,"clip",strlen(tr.surface->name))== 0) )
			{
				vehicle->s.origin[2] -= 1;
				vehicle->velocity[2] = 0;
			}
			// make speed
			VectorCopy( vehicle->velocity, hSpeed );
			hSpeed[2] = 0;
			VectorScale( hSpeed, 0.9, hSpeed );
			hSpeed[2] = vehicle->velocity[2];
			VectorCopy( hSpeed, vehicle->velocity );
			SV_AddGravity( vehicle );
			if( vehicle->waterlevel > 1 )
				vehicle->ONOFF &= ~ONOFF_AIRBORNE;
		}
		else	// chute open
		{
			// steer the chute
			if( !vehicle->deadflag )
			{
				vehicle->avelocity[1] = vehicle->aileron;
				if( vehicle->avelocity[1] > 10 )
				{
					if( vehicle->s.angles[2]*VM < 30 )
						vehicle->avelocity[2] = 15*VM;
					else
						vehicle->avelocity[2] = 0;
				}
				else if( vehicle->avelocity[1] < -10 )
				{
					if( vehicle->s.angles[2]*VM > -30 )
						vehicle->avelocity[2] = -15*VM;
					else
						vehicle->avelocity[2] = 0;
				}
				vehicle->avelocity[0] = vehicle->elevator;
				if( vehicle->avelocity[0] > 10 )
				{
					if( vehicle->s.angles[0] < 40 )
						vehicle->avelocity[0] = 45;
					else if( vehicle->s.angles[0] > 40 && vehicle->s.angles[0] < 90 )
						vehicle->avelocity[0] = 0;
				}
				else if( vehicle->avelocity[0] < -10 )
				{
					if( vehicle->s.angles[0] > 350 )
						vehicle->avelocity[0] = -45;
					else if( vehicle->s.angles[0] < 350 && vehicle->s.angles[0] > 270 )
						vehicle->avelocity[0] = 0;
				}
			}
//			gi.bprintf( PRINT_HIGH, "%.0f\n", vehicle->s.angles[0] );
			if( vehicle->deadflag || !vehicle->aileron )
			{
				if( fabs(vehicle->s.angles[2]*VM) > 10 && fabs(vehicle->s.angles[2]*VM) < 90)
				{
					if( vehicle->s.angles[2]*VM > 0 )
						vehicle->avelocity[2] = -55*VM;
					else
						vehicle->avelocity[2] = 55*VM;
				}
				else if( fabs(vehicle->s.angles[2]*VM) > 90 && fabs(vehicle->s.angles[2]*VM) < 170)
				{
					if( vehicle->s.angles[2]*VM > 180 )
						vehicle->avelocity[2] = -55*VM;
					else
						vehicle->avelocity[2] = 55*VM;
				}
				else
					vehicle->s.angles[2] = 0;
			}
			if( vehicle->deadflag || !vehicle->elevator )
			{
				if( vehicle->s.angles[0] >= 270 || vehicle->s.angles[0] < 90 )
				{
					if( vehicle->s.angles[0] < 90 && vehicle->s.angles[0] > 6 )
						vehicle->avelocity[0] = -55;
					else if( vehicle->s.angles[0] >= 270 && vehicle->s.angles[0] < 354 )
						vehicle->avelocity[0] = 55;
					else
						vehicle->s.angles[0] = 0;
				}
				else if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] >= 90 )
				{
					if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] > 190 )
						vehicle->avelocity[0] = -55;
					else if( vehicle->s.angles[0] >= 90 && vehicle->s.angles[0] < 170 )
						vehicle->avelocity[0] = 55;
					else
						vehicle->s.angles[0] = 0;
				}
				if( vehicle->velocity[2] >= -40 )
					vehicle->velocity[2] -= 3;
				else if( vehicle->velocity[2] < -42 && !(vehicle->ONOFF & ONOFF_STALLED) )
					vehicle->velocity[2] += 2;
			}
			else if( vehicle->elevator && vehicle->s.angles[0] > 40 && vehicle->s.angles[0] < 350 )
			{
				if( vehicle->s.angles[0] >= 270 || vehicle->s.angles[0] < 90 )
				{
					if( vehicle->s.angles[0] < 90 && vehicle->s.angles[0] > 45 )
						vehicle->avelocity[0] = -55;
					else if( vehicle->s.angles[0] >= 270 && vehicle->s.angles[0] < 345 )
						vehicle->avelocity[0] = 55;
				}
				else if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] >= 90 )
				{
					if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] > 190 )
						vehicle->avelocity[0] = -55;
					else if( vehicle->s.angles[0] >= 90 && vehicle->s.angles[0] < 170 )
						vehicle->avelocity[0] = 55;
					else
						vehicle->s.angles[0] = 0;
				}
				if( vehicle->velocity[2] >= -40 )
					vehicle->velocity[2] -= 3;
				else if( vehicle->velocity[2] < -42 && !(vehicle->ONOFF & ONOFF_STALLED) )
					vehicle->velocity[2] += 2;
			}

			AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			forward[2] = 0;
			VectorNormalize( forward );
			if( vehicle->s.angles[0] >= 0 && vehicle->s.angles[0] < 50 )
			{
				VectorScale( forward, vehicle->s.angles[0], forward );
				if( vehicle->velocity[2] > -80 )
					vehicle->velocity[2] -= (vehicle->s.angles[0]/10);
			}
			else 
			{
				VectorScale( forward, 0, forward );
				if( vehicle->s.angles[0] < 360 && vehicle->s.angles[0] > 310 && vehicle->velocity[2] < 5 )
				{
					if( !(vehicle->ONOFF & ONOFF_STALLED) )
						vehicle->velocity[2] += ((360-vehicle->s.angles[0])/2);
					else
						vehicle->velocity[2] -= 3;
				}
			}
			forward[2] = vehicle->velocity[2];

			VectorCopy( forward, vehicle->velocity );

			if( VectorLength( vehicle->velocity ) < 30 )
				vehicle->ONOFF |= ONOFF_STALLED;
			else
				vehicle->ONOFF &= ~ONOFF_STALLED;

			if( vehicle->waterlevel )
			{
				vehicle->s.angles[0] = vehicle->s.angles[2] = 0;

				vehicle->s.frame = 1;
				vehicle->ONOFF |= ONOFF_LANDED;
				vehicle->ONOFF &= ~(ONOFF_AIRBORNE);
				vehicle->s.modelindex3 = 0;
//				vehicle->s.origin[2] += 2;
			}
//			gi.bprintf( PRINT_HIGH, "%.0f\n", vehicle->velocity[2] );
		}
		vehicle->ideal_yaw = 0;
	}
	else	// on ground
	{
		vehicle->s.angles[0] = vehicle->s.angles[2] = 0;
		// drowning ?
		if( vehicle->wait )
		{
			if( level.time - vehicle->wait >= 40 )
				T_Damage( vehicle, vehicle, vehicle, vec3_origin, vehicle->s.origin, vec3_origin, 1, 0, 0, MOD_WATER );
			else if( level.time - vehicle->wait >= 35 && level.time > vehicle->random )
			{
				gi.sound( vehicle, CHAN_BODY, gi.soundindex( "player/male/gurp1.wav" ), 1.0, ATTN_NORM, 0 );
				vehicle->random = level.time + 1.0;
			}
//			gi.bprintf( PRINT_HIGH, "%.1f\n", level.time - vehicle->wait );
		}
		if( !vehicle->deadflag )
		{
			if( !vehicle->thrust && !vehicle->rudder)
			{
				vehicle->s.frame++;
				if( vehicle->s.frame > 31 )
					vehicle->s.frame = 1;
			}
			else
			{
				if( vehicle->thrust > 0 )
				{
					vehicle->s.frame++;
					if( vehicle->s.frame < 32 || vehicle->s.frame > 37 )
						vehicle->s.frame = 32;
				}
				else
				{
					vehicle->s.frame--;
					if( vehicle->s.frame < 32 || vehicle->s.frame > 37 )
						vehicle->s.frame = 37;
				}
			}
		}
		else
		{
			if( vehicle->s.frame < 43 )
				vehicle->s.frame = 43;
			if( vehicle->s.frame < 56 )
				vehicle->s.frame ++;
			if( vehicle->s.frame == 56 )
			{
				vehicle->deadflag = DEAD_DEAD;
				if( vehicle->s.angles[0] <= 90 || vehicle->s.angles[0] > 270 )
					vehicle->s.angles[0] = 0;
				else 
					vehicle->s.angles[0] = 180;
				vehicle->solid = SOLID_NOT;
				return;
			}
		}
		VectorCopy( vehicle->s.origin, up );
		up[2] += 6;
		VectorCopy( vehicle->s.origin, below );
		if( !vehicle->waterlevel )
			below[2] -= 18;
		else
		{
			vehicle->ONOFF &= ~ONOFF_BRAKE;
			below[2] -= 2;
		}
		tr = gi.trace( up, vehicle->mins, vehicle->maxs, below, vehicle, MASK_PLAYERSOLID );

		if( tr.fraction < 1 )
		{
			vehicle->s.origin[2] = tr.endpos[2] + fabs(vehicle->mins[2]);// + 2;
			vehicle->groundentity = tr.ent;
		}
		else
		{
			if( !vehicle->waterlevel )
			{
				vehicle->ONOFF |= (ONOFF_AIRBORNE|ONOFF_AUTOROTATION|ONOFF_GEAR);	// gear lets you manually open chute
				vehicle->thrust = 0;
				vehicle->drone_decision_time = level.time + 15;
			}
		}
		if( vehicle->deadflag )
		{
//			vehicle->deadflag = DEAD_DEAD;
			vehicle->thrust = 0;
			vehicle->speed = 0;
			vehicle->nextthink = level.time + 0.1;
			return;
		}
		if( vehicle->elevator )
		{
			vehicle->ideal_yaw += (vehicle->elevator*FRAMETIME);
			if( vehicle->ideal_yaw > 69 )
				vehicle->ideal_yaw = 69;
			else if( vehicle->ideal_yaw < -69 )
				vehicle->ideal_yaw = -69;
		}
//		gi.bprintf( PRINT_HIGH, "%.0f\n", vehicle->ideal_yaw );

		// give speed
		vehicle->speed = vehicle->thrust * (vehicle->ONOFF & ONOFF_BRAKE ? 12 : 7);
		if( vehicle->waterlevel )
		{
			vehicle->s.angles[0] = vehicle->ideal_yaw;
			if( vehicle->waterlevel == 1 && vehicle->ideal_yaw < 0 )
				vehicle->s.angles[0] = 0;
			vehicle->speed /= 2;
		}
		vehicle->avelocity[1] = vehicle->aileron;
		AngleVectors( vehicle->s.angles, forward, up, NULL );
		vehicle->s.angles[0] = 0;
		VectorScale( forward, vehicle->speed, vehicle->velocity );
		if( vehicle->rudder )
		{
			VectorMA( vehicle->velocity, (vehicle->rudder > 0 ? -12 : 12), up, vehicle->velocity );
/*			if( !vehicle->thrust )
			{
				vehicle->s.frame++;
				if( vehicle->s.frame < 32 || vehicle->s.frame > 37 )
					vehicle->s.frame = 32;
			}
*/		}
			// jump?
		if( vehicle->ONOFF & ONOFF_BRAKE && !vehicle->waterlevel )
		{
			vehicle->velocity[2] += 32;
			vehicle->s.origin[2] += 2;
			vehicle->ONOFF &= ~ONOFF_LANDED;
			vehicle->ONOFF |= (ONOFF_AIRBORNE|ONOFF_AUTOROTATION|ONOFF_GEAR);
			vehicle->ONOFF &= ~ONOFF_BRAKE;
			vehicle->drone_decision_time = level.time + 15;
			vehicle->s.frame = 35;
		}
		VectorNormalize( forward );
		VectorScale( forward, 15, forward );
		VectorAdd( vehicle->s.origin, forward, up );
		tr = gi.trace( vehicle->s.origin, NULL, NULL, up, vehicle, MASK_ALL );
		if( tr.fraction < 1 )
		{
			if( ( strcmp( tr.ent->classname, "plane" ) == 0 ||
				strcmp( tr.ent->classname, "helo" ) == 0 ||
				strcmp( tr.ent->classname, "ground" ) == 0 )
				&& !(tr.ent->ONOFF & ONOFF_PILOT_ONBOARD) )
			{
				vehicle->activator = tr.ent;
			}
		}
		else
		{
			vehicle->activator = NULL;
		}
		// lock weapons
		if( vehicle->weapon[vehicle->active_weapon] == WEAPON_SIDEWINDER ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_STINGER ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_AMRAAM ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_PHOENIX )
			AAM_Lock( vehicle );
		else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_HELLFIRE ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_MAVERICK ||
			vehicle->weapon[vehicle->active_weapon] == WEAPON_ANTIRADAR )
			ATGM_Lock( vehicle );
		else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_AUTOCANNON )
			Autocannon_Lock( vehicle );
		else if( vehicle->weapon[vehicle->active_weapon] == WEAPON_JAMMER )
		{
			if( vehicle->ONOFF & ONOFF_JAMMER )
				Jammer_Active( vehicle );
			else
				Reload_Jammer( vehicle );
			if( vehicle->enemy )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		else if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		// fire weapons
		if( vehicle->owner->client->buttons & BUTTON_ATTACK )
			Weapon_Autocannon_Fire( vehicle );
	}
//	vehicle->altitude = vehicle->s.origin[2];
	vehicle->nextthink = level.time + 0.1;
	vehicle->radio_target = NULL;
	vehicle->ONOFF &= ~ONOFF_IS_CHANGEABLE;
	vehicle->teammaster = NULL;
}




void LQM_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	int sub = 0;
//	gi.bprintf( PRINT_HIGH, "touch lqm (%s)\n", other->classname );
	if( self->deadflag == DEAD_DEAD )
	{
		SetVehicleModel( self );
		VectorClear( self->velocity );
		return;
	}
	if( surf && (surf->flags & SURF_SKY) )
	{
//		gi.bprintf( PRINT_HIGH, "here (%s)\n", surf->name );
		return;
	}
	if( strcmp( other->classname, "bolt" ) == 0 )
	{
		return;
	}
	if( ( strcmp( other->classname, "plane" ) == 0 ||
		strcmp( other->classname, "helo" ) == 0 ||
		strcmp( other->classname, "ground" ) == 0 )
		&& !self->deadflag && (other->speed > 0) )
	{
		if( self->ONOFF & ONOFF_AIRBORNE )
		{
			self->gravity = 1.0;
			self->ONOFF |= ONOFF_AUTOROTATION;
			self->s.modelindex3 = 0;
		}
		SpawnDamage (TE_BLOOD, self->s.origin, other->velocity, 0 );
		T_Damage( self, other, other, vec3_origin, self->s.origin, vec3_origin, 9999, 0, 0, MOD_VEHICLE );
		return;
	}
	if( (self->ONOFF & ONOFF_AIRBORNE) && !(self->ONOFF & ONOFF_AUTOROTATION) )	// open chute
	{
		float len = VectorLength( self->velocity );

		if( len > 50 && !self->waterlevel)
		{
			sub = ((len - 50)/5);
			T_Damage( self, world, world, vec3_origin, self->s.origin, vec3_origin, sub, 0, 0, MOD_CRASH );
		}

		self->s.frame = 1;
		self->ONOFF |= ONOFF_LANDED;
		self->ONOFF &= ~(ONOFF_AIRBORNE);
		self->s.modelindex3 = 0;
//		self->s.origin[2] += 2;
	}
	else if( (self->ONOFF & ONOFF_AIRBORNE) && (self->ONOFF & ONOFF_AUTOROTATION) )
	{
		float len = VectorLength( self->velocity );

		self->ONOFF |= ONOFF_LANDED;
		self->ONOFF &= ~ONOFF_AIRBORNE;
		self->ONOFF &= ~ONOFF_AUTOROTATION;
		self->s.modelindex3 = 0;
		self->s.frame = 1;

		if( len > 50 && !self->waterlevel )
		{
			sub = ((len - 50)/5);
			T_Damage( self, world, world, vec3_origin, self->s.origin, vec3_origin, sub, 0, 0, MOD_CRASH );
		}
		
		if( self->health <= 0 && self->deadflag )
		{
			self->deadflag = DEAD_DEAD;
			self->s.frame = 56;
			SetVehicleModel( self );
			VectorAdd( self->velocity, other->velocity, self->velocity );
//			VectorClear( self->velocity );
		}
	}
}

void LQM_pain (edict_t *self, edict_t *other, float kick, int damage)
{

}



void LQMObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int		mod = meansOfDeath;
	char	*message = NULL, *message2 = NULL, *message3 = "";
	int		print = 0;

//	gi.bprintf( PRINT_HIGH, "LQM obituary!\n" );

	if( !(self->HASCAN & HASCAN_SCORED) )
		print = 1;

// AND ALSO CLEAN THIS UGLY SHIT UP! Just check whether there is an attacker or not!

	switch( mod )
	{
	case MOD_AUTOCANNON:
		if( strcmp( attacker->classname, "LQM" ) == 0 )
			message = "was machinegunned by";
		else
		{
			message = "was cut in half by";
			message3 = "'s Autocannon";
		}
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_SIDEWINDER:	
		message = "was blown apart by";
		message3 = "'s Sidewinder";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_STINGER:		
		message = "was disintegrated by";
		message3 = "'s Stinger";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_AMRAAM:	
		message = "got to know";
		message3 = "'s AMRAAM";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_PHOENIX:		
		message = "couldn't outrun";
		message3 = "'s Phoenix";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_HELLFIRE:	
		message = "was punted away by";
		message3 = "'s Hellfire";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_MAVERICK:	
		message = "was hunted down by";
		message3 = "'s Maverick";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_ANTIRADAR:	
		message = "stopped emitting after being hit by";
		message3 = "'s Anti-Radar Missile";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_ROCKET:
		message = "met an old friends";
		message3 = "'s Rocket";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_LASERBOMBS:
		message = "couldn't get rid of";
		message3 = "'s LGB";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_RUNWAYBOMBS:
		message = "feels like an old runway because of";
		message3 = "'s Durandal";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	case MOD_BOMBS:
		message = "was blasted apart by";
		message3 = "'s Iron Bombs";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count++;
			}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score++;
		}
		break;
	default:
		message = "died";
		message2 = "";
		message3 = "";
		break;
	}

	self->HASCAN |= HASCAN_SCORED;

	if( print )
		gi.bprintf( PRINT_HIGH, "%s %s %s%s\n", self->owner->client->pers.netname, message, message2, message3 );
}




void LQM_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if( !self->deadflag )
	{
		if( !(self->ONOFF & ONOFF_AIRBORNE) )
			self->deadflag = DEAD_DYING;
		else
		{
			self->deadflag = DEAD_DEAD;
			self->solid = SOLID_NOT;
		}
		gi.sound (self, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
	}
	else 
	{
		self->deadflag = DEAD_DEAD;
		self->solid = SOLID_NOT;
	}
	if( self->deadflag == DEAD_DEAD )
	{
		self->owner->client->respawn_time = level.time + 2.0;
	}
	self->mins[2] = -2;
	self->s.angles[0] = 0;
	self->movetype = MOVETYPE_TOSS;
	SetVehicleModel( self );
	LQMObituary( self, inflictor, attacker );
}



