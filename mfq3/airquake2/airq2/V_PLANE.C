// file v_plane.c by Bjoern Drabeck
// handles plane movement and sounds


#include "g_local.h"
#include "v_plane.h"


void PlaneSound( edict_t *vehicle )
{
	float speedindex, volume;

	if( !vehicle->temptime )
		vehicle->temptime = level.time;

	if( level.time > vehicle->temptime && vehicle->ONOFF & PILOT_ONBOARD )
	{
		speedindex = floor( (vehicle->actualspeed/8) / 25 );
		volume = vehicle->thrust * 0.1 + 0.3;

		if( speedindex <= 0 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet0.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 1 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet1.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 2 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet2.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 3 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet3.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 4 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet4.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 5 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet5.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 6 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet6.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 7 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet7.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex >= 8 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/jet8.wav" ), volume, ATTN_NORM, 0 );
		vehicle->temptime = level.time + 0.2;
	}

}


void SetPlaneSpeed (edict_t *vehicle)
{
	float acceleration = vehicle->accel;
	float deceleration = vehicle->decel;
	float Winkel;

	if( vehicle->deadflag == DEAD_DEAD )
	{
		vehicle->actualspeed = 0;
		return;
	}

	// influence of angle on velocity
	if( vehicle->s.angles[0] < 357 && vehicle->s.angles[0] >= 270 )
		Winkel = -(360 - vehicle->s.angles[0]);
	else if( vehicle->s.angles[0] < 270 && vehicle->s.angles[0] > 183 )
		Winkel = 180 - vehicle->s.angles[0];
	else if( vehicle->s.angles[0] > 3 && vehicle->s.angles[0] <= 90 )
		Winkel = vehicle->s.angles[0];
	else if( vehicle->s.angles[0] > 90 && vehicle->s.angles[0] < 177 )
		Winkel = 180 - vehicle->s.angles[0];
	else
		Winkel = 0;

	if( Winkel > 0 )
		Winkel = (ceil(Winkel/10.0)*2.2);
	else
		Winkel = (floor(Winkel/10.0)*2.5);


	Winkel *= (sv_gravity->value / 800);	// add dependency on gravity
	Winkel += vehicle->thrust/2.0;			// and thrust

	if( vehicle->thrust < THRUST_AFTERBURNER )
		vehicle->currenttop = (vehicle->topspeed / 4.0) * vehicle->thrust;
	else
		vehicle->currenttop = vehicle->topspeed * ( 4.0 / 3.0 );

	if( vehicle->ONOFF & BRAKE )
	{
		vehicle->currenttop *= (3.6/5.0);
	}

	if( (vehicle->ONOFF & GEARDOWN) && !(vehicle->ONOFF & LANDED) )
	{
		vehicle->currenttop *= (4.1/5.0);
	}
	if( vehicle->ONOFF & BAY_OPEN )
	{
		vehicle->currenttop *= (4.3/5.0);
	}

	// effect of runway on the plane
	if( vehicle->ONOFF & LANDED )
	{
		vehicle->currenttop *= 0.55;
		deceleration = vehicle->decel;
		if( vehicle->thrust < THRUST_AFTERBURNER )
			acceleration *= 0.7;
		else
			acceleration *= 1.1;
	}
	else
	{
		// acceleration stronger with afterburner
		if( vehicle->thrust < THRUST_AFTERBURNER )
			acceleration = ( vehicle->accel - 1.0 );
		else
			acceleration = vehicle->accel * 1.5;
	}

	// influence of speedbrakes
	if( vehicle->ONOFF & BRAKE )
	{
		deceleration *= 1.5;
		acceleration /= 2.8;
		// the higher the speed the better the speedbrakes
		if( vehicle->actualspeed/8 > 100 )
			deceleration *= (vehicle->actualspeed/800);
	}

	// the higher the speed the higher the drag
	if( (vehicle->actualspeed/8) > 150 )
		deceleration *= (vehicle->actualspeed/800);

	// calculate the actual speed
	if( vehicle->actualspeed >= vehicle->currenttop )
	{
		vehicle->actualspeed -= deceleration;
		if( vehicle->actualspeed < vehicle->currenttop )
			vehicle->actualspeed = vehicle->currenttop;

	}
	else
	{
		vehicle->actualspeed += acceleration;
		if( vehicle->actualspeed > vehicle->currenttop )
			vehicle->actualspeed = vehicle->currenttop;
	}

	// add influence of angle
	if( !( vehicle->ONOFF & LANDED) )
		vehicle->actualspeed += Winkel;

	// remove ?
	if( vehicle->ONOFF & STALLED )
		if( vehicle->actualspeed > vehicle->speed * 1.6 )		// if stalled the
			vehicle->actualspeed = vehicle->speed * 1.6;		// speed doesn't become faster

	// speedlimit
	if( vehicle->actualspeed > vehicle->topspeed * ( 4.5 / 3.0 ) )
		vehicle->actualspeed = vehicle->topspeed * ( 4.5 / 3.0 );

	PlaneSound( vehicle );
}

void SteerPlane( edict_t *vehicle )
{
	vec3_t		forward,targetangle;
	vec3_t		pos1,pos2,pos3,pos4;
	vec3_t		border1,border2;
	trace_t 	trace,traceup,tracedown,traceleft,traceright;
	vec3_t 		underneath, above;
	float		sinpitch,cospitch;
	float		sinroll,cosroll;
	int			VM = vehicle->owner->client->pers.videomodefix;
	int			bay = 0;

	// health-effects
	if( vehicle->owner->health >= 26 )
		vehicle->s.effects = 0;
	if( vehicle->owner->health < 26 && vehicle->owner->health > 10 )
	{
		vehicle->s.effects = EF_GRENADE;
	}
	else if( vehicle->owner->health <= 10 && vehicle->deadflag != DEAD_DEAD )
	{
		vehicle->s.effects = EF_ROCKET;
	}
	else
		vehicle->s.effects = 0;


	SetPlaneSpeed( vehicle );

	if( vehicle->health <= vehicle->gib_health && vehicle->deadflag != DEAD_DEAD )
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (vehicle->s.origin);
		gi.multicast (vehicle->s.origin, MULTICAST_PVS);
//		ThrowDebris( vehicle, "models/objects/debris1/tris.md2",
//			5, vehicle->s.origin );
//		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
//			5, vehicle->s.origin );
		ThrowDebris( vehicle, "models/objects/debris3/tris.md2",
			5, vehicle->s.origin );
		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
			5, vehicle->s.origin );
		SetVehicleModel(vehicle);
		vehicle->deadflag = DEAD_DEAD;
	}
	// DEAD vehicles don't need to be steered!
	if( vehicle->deadflag && vehicle->enemy )
	{
		vehicle->enemy->lockstatus = 0;
		vehicle->enemy = NULL;
	}
	if( vehicle->deadflag == DEAD_DEAD )
	{
		return;
	}
	else if( vehicle->deadflag == DEAD_DYING )
	{
		vehicle->avelocity[2] = (vehicle->s.angles[2]>=0) ? 60 : -60;
		vectoangles( vehicle->velocity, targetangle );
		vehicle->s.angles[0] = targetangle[0];
		vehicle->s.angles[1] = targetangle[1];
		vehicle->movetype = MOVETYPE_TOSS;
		vehicle->gravity = 0.1;
		vehicle->nextthink = level.time + 0.1;
		return;
	}

	// speed effects
	if( vehicle->actualspeed > (vehicle->speed + 399) && (vehicle->ONOFF & GEARDOWN)
			&& !(vehicle->ONOFF & LANDED) && !(vehicle->DAMAGE & GEAR_DAMAGE))
	{
		vehicle->ONOFF ^= GEARDOWN;
		gi.cprintf( vehicle->owner, PRINT_MEDIUM, "Gear retracted!\n" );
	}
	// frames
	if( vehicle->actualspeed <= (vehicle->speed + 399) && !(vehicle->ONOFF & GEARDOWN)
			 && !(vehicle->ONOFF & BRAKE) )
		vehicle->s.frame = 0;
	else if( vehicle->actualspeed > (vehicle->speed + 399) && !(vehicle->ONOFF & GEARDOWN)
	 			 && !(vehicle->ONOFF & BRAKE) )
		vehicle->s.frame = 1;
	else if( vehicle->actualspeed <= (vehicle->speed + 399) && (vehicle->ONOFF & GEARDOWN)
	 			 && (vehicle->ONOFF & BRAKE ) )
		vehicle->s.frame = 2;
	else if( vehicle->actualspeed <= (vehicle->speed + 399) && !(vehicle->ONOFF & GEARDOWN)
			 && (vehicle->ONOFF & BRAKE ) )
		vehicle->s.frame = 3;
	else if( vehicle->actualspeed <= (vehicle->speed + 399) && (vehicle->ONOFF & GEARDOWN)
	  			 && !(vehicle->ONOFF & BRAKE ) )
		vehicle->s.frame = 4;
	else if( vehicle->actualspeed > (vehicle->speed + 399) && !(vehicle->ONOFF & GEARDOWN)
	  			 && (vehicle->ONOFF & BRAKE ) )
		vehicle->s.frame = 5;
	if( vehicle->ONOFF & BAY_OPEN )
		vehicle->s.frame += 6;

	// adjust control sensitivity on zoom factor
	if( vehicle->fov == 60 )
	{
		vehicle->elevator /= 2;
		vehicle->aileron /= 2;
		vehicle->rudder /= 2;
	}
	else if( vehicle->fov == 30 )
	{
		vehicle->elevator /= 4;
		vehicle->aileron /= 4;
		vehicle->rudder /= 4;
	}
	else if( vehicle->fov == 10 )
	{
		vehicle->elevator /= 6;
		vehicle->aileron /= 6;
		vehicle->rudder /= 6;
	}

	// reset angular velocities
	if( !( vehicle->ONOFF & STALLED ) )
		vehicle->avelocity[0] = vehicle->avelocity[1] = vehicle->avelocity[2] = 0;

	// check for overturning
	vehicle->s.angles[0] = anglemod( vehicle->s.angles[0] );
	vehicle->s.angles[1] = anglemod( vehicle->s.angles[1] );
	if( vehicle->s.angles[2] > 180 )
		vehicle->s.angles[2] -= 360;
	else if( vehicle->s.angles[2] < -180 )
		vehicle->s.angles[2] += 360;

	// autopilot
	if( vehicle->ONOFF & AUTOPILOT )
	{
		VectorCopy( vehicle->mins, border1 );
		VectorCopy( vehicle->maxs, border2 );
		border1[0] -= 3;
		border1[1] -= 3;
		border1[2] -= 3;
		border2[0] += 3;
		border2[1] += 3;
		border2[2] += 3;
		vehicle->aileron = vehicle->elevator = vehicle->rudder = 0;
		VectorCopy( vehicle->s.angles, pos1 );
		pos1[0] -= 40;
		VectorCopy( vehicle->s.angles, pos2 );
		pos2[0] += 40;
		VectorCopy( vehicle->s.angles, pos3 );
		pos3[1] += 18;
		VectorCopy( vehicle->s.angles, pos4 );
		pos4[1] -= 18;

		AngleVectors( vehicle->s.angles, forward, NULL, NULL );
		VectorMA( vehicle->s.origin, 350, forward, forward );
		trace = gi.trace( vehicle->s.origin, border1, border2, forward, vehicle, MASK_ALL );

		AngleVectors( pos1, forward, NULL, NULL );
		VectorMA( vehicle->s.origin, 350, forward, forward );
		traceup = gi.trace( vehicle->s.origin, NULL, NULL, forward, vehicle, MASK_ALL );

		AngleVectors( pos2, forward, NULL, NULL );
		VectorMA( vehicle->s.origin, 350, forward, forward );
		tracedown = gi.trace( vehicle->s.origin, NULL, NULL, forward, vehicle, MASK_ALL );

		AngleVectors( pos3, forward, NULL, NULL );
		VectorMA( vehicle->s.origin, 350, forward, forward );
		traceleft = gi.trace( vehicle->s.origin, NULL, NULL, forward, vehicle, MASK_ALL );

		AngleVectors( pos4, forward, NULL, NULL );
		VectorMA( vehicle->s.origin, 350, forward, forward );
		traceright = gi.trace( vehicle->s.origin, NULL, NULL, forward, vehicle, MASK_ALL );

		// avoid terrain
		if( trace.fraction < 1 )
		{
	 		if( tracedown.fraction < 0.4 )
				vehicle->elevator = -90;
			else if( traceup.fraction < 0.4 )
				vehicle->elevator = 90;
			if( traceleft.fraction > traceright.fraction )
				vehicle->rudder = 50;
			else
				vehicle->rudder = -50;

			if( traceleft.fraction < 0.5 && traceright.fraction < 0.5 )
				vehicle->elevator = -300;
			if( traceleft.fraction < 0.25 || traceright.fraction < 0.25 )
				vehicle->rudder *= 2.5;
		}
		else	// neutralize control surfaces
		{
			vehicle->elevator = 0;
			vehicle->rudder = 0;
			vehicle->aileron = 0;
		}

		// continue loopings
		if( vehicle->s.angles[0] > 230 && vehicle->s.angles[0] < 310 )
			vehicle->elevator = -400;
		// adjust throttle on speed
		if( vehicle->actualspeed < ( vehicle->speed + 80  ) )
		{
			vehicle->thrust ++;
			if( vehicle->thrust > THRUST_100 )
			{
				if( vehicle->HASCAN & HAS_AFTERBURNER )// afterburner
				{
					vehicle->thrust = THRUST_AFTERBURNER;
					vehicle->s.skinnum = 1;
				}
				else
			 		vehicle->thrust = THRUST_100;
			}
			// check for damage
			if( vehicle->DAMAGE & AFTERBURNER_DAMAGE && vehicle->thrust > THRUST_100 )
				vehicle->thrust = THRUST_100;
			if( vehicle->DAMAGE & ENGINE75 && vehicle->thrust > THRUST_75 )
			{
				vehicle->thrust = THRUST_75;
				gi.cprintf( vehicle->owner, PRINT_MEDIUM,
					"Autopilot disengaged due to engine damage!\n" );
				vehicle->ONOFF ^= AUTOPILOT;
			}
			if( vehicle->DAMAGE & ENGINE50 && vehicle->thrust > THRUST_50 )
			{
				vehicle->thrust = THRUST_50;
				gi.cprintf( vehicle->owner, PRINT_MEDIUM,
					"Autopilot disengaged due to engine damage!\n" );
				vehicle->ONOFF ^= AUTOPILOT;
			}
			if( vehicle->DAMAGE & ENGINE25 && vehicle->thrust > THRUST_25 )
			{
				vehicle->thrust = THRUST_25;
				gi.cprintf( vehicle->owner, PRINT_MEDIUM,
					"Autopilot disengaged due to engine damage!\n" );
				vehicle->ONOFF ^= AUTOPILOT;
			}
			if( vehicle->DAMAGE & ENGINE0 && vehicle->thrust > THRUST_0 )
			{
				vehicle->thrust = THRUST_0;
				gi.cprintf( vehicle->owner, PRINT_MEDIUM,
					"Autopilot disengaged due to engine destruction!\n" );
				vehicle->ONOFF ^= AUTOPILOT;
			}
		}
		if( vehicle->actualspeed > vehicle->topspeed + 160 &&
				vehicle->thrust > THRUST_0 )
		{
			vehicle->thrust --;
			vehicle->s.skinnum = 0;
		}
		VectorCopy( vehicle->s.origin, above );
		above[2] += 100;
		trace = gi.trace( vehicle->s.origin, NULL, NULL, above, vehicle, MASK_ALL );
		if( trace.fraction < 1.0 && (trace.surface->flags & SURF_SKY ||
	   		strncmp(trace.surface->name,"clip",strlen(trace.surface->name))== 0) &&
			!vehicle->elevator )
		{
			vehicle->elevator = 100;
		}

	}


	// movement on ground
	if( vehicle->ONOFF & LANDED )
	{
		VectorSet( underneath, vehicle->s.origin[0], vehicle->s.origin[1],
						vehicle->s.origin[2] - 32 );

		trace = gi.trace( vehicle->s.origin, NULL, NULL, underneath, vehicle, MASK_ALL );

		// are you standing on a runway ?
		if( strcmp( trace.ent->classname, "func_runway" ) == 0
		|| strcmp( trace.ent->classname, "func_door" ) == 0
		|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0 )
		{
			if( vehicle->elevator < 0 && vehicle->actualspeed >= vehicle->speed )
			{
				if( vehicle->s.angles[0] < 10 )
					vehicle->avelocity[0] = vehicle->elevator;
				else
					vehicle->avelocity[0] = -vehicle->elevator;
				vehicle->ONOFF &= ~LANDED;
				gi.cprintf( vehicle->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
			else
			{
				vehicle->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2]
			   			 + abs(vehicle->mins[2])+1;

			}
			if( vehicle->rudder || vehicle->aileron )
			{
				if( vehicle->actualspeed )
				{
					if( vehicle->rudder )
						vehicle->avelocity[1] = 2*vehicle->rudder;
					else if( vehicle->aileron )
					{
						vehicle->avelocity[1] = vehicle->aileron;
						if( vehicle->avelocity[1] > 0 &&
								vehicle->avelocity[1] > 2*vehicle->turnspeed[1] )
							vehicle->avelocity[1] = 2*vehicle->turnspeed[1];
						else if( vehicle->avelocity[1] < 0 &&
								vehicle->avelocity[1] < -2*vehicle->turnspeed[1] )
							vehicle->avelocity[1] = -2*vehicle->turnspeed[1];
					}
				}
			}
		}
		// if not then you stall
		else
		{
			vehicle->ONOFF &= ~LANDED;
			if( vehicle->actualspeed < vehicle->speed )
			{
				vehicle->ONOFF |= STALLED;
			}
			else
			{
				gi.cprintf( vehicle->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
		}
	}
	// and movement in the air
	else
	{
		// movement when stalled
		if( vehicle->ONOFF & STALLED )
		{
			if( vehicle->actualspeed < vehicle->speed )// stay stalled
			{
				if( vehicle->s.angles[0] < 70 || vehicle->s.angles[0] > 110 )
				{
					if( vehicle->s.angles[0] >= 270 || vehicle->s.angles[0] <= 70 )
						vehicle->avelocity[0] = vehicle->turnspeed[0]/2;
					else
						vehicle->avelocity[0] = -vehicle->turnspeed[0]/2;
				}
				if( fabs(vehicle->s.angles[2]) > 20 )
					vehicle->avelocity[1] = vehicle->turnspeed[1];
				else
					vehicle->avelocity[1] = 0;
			}
			else // end stall
			{
					vehicle->ONOFF ^= STALLED;
			}
			if( vehicle->s.angles[0] > 70 && vehicle->s.angles[0] < 110 )
				vehicle->avelocity[0] = 0;
		}
		// and normal movement
		else
		{
			// angluar corrections
			sinpitch = sin((vehicle->s.angles[0]*M_PI)/180.0);
			cospitch = cos((vehicle->s.angles[0]*M_PI)/180.0);
			if( vehicle->s.angles[0] < 90 ||vehicle->s.angles[0] >= 270 )
			{
				sinroll = -sin((vehicle->s.angles[2]*M_PI*VM)/180.0);
				cosroll = cos((vehicle->s.angles[2]*M_PI*VM)/180.0);
			}
			else
			{
				sinroll = -sin((-vehicle->s.angles[2]*M_PI*VM)/180.0);
				cosroll = cos((-vehicle->s.angles[2]*M_PI*VM)/180.0);
			}


			// elevator use
			if( !vehicle->owner->client->pers.autocoord ||
					( vehicle->ONOFF & AUTOPILOT ) )
			{
				vehicle->avelocity[0] = vehicle->elevator*cosroll;
				vehicle->avelocity[1] = vehicle->elevator*sinroll;
			}
			else
			{
				if( vehicle->elevator )
				{
					vehicle->avelocity[0] = vehicle->elevator*cosroll;
					vehicle->avelocity[1] = vehicle->elevator*sinroll;
				}
				if( fabs(vehicle->s.angles[2]) > 5 )
				{
					if( !vehicle->owner->client->pers.autoroll )
						vehicle->avelocity[1] += -vehicle->turnspeed[1]*sinroll;
					else if( vehicle->owner->client->pers.autoroll && vehicle->aileron )
						vehicle->avelocity[1] += -vehicle->turnspeed[1]*sinroll;
				}
			}

			// rudder use
			if( vehicle->s.angles[0] < 90 || vehicle->s.angles[0] >= 270 )
			{
				vehicle->avelocity[1] += vehicle->rudder*cosroll;
				vehicle->avelocity[0] -= vehicle->rudder*sinroll;
			}
			else
			{
				vehicle->avelocity[1] -= vehicle->rudder*cosroll;
				vehicle->avelocity[0] += vehicle->rudder*sinroll;
			}

			// aileron use
			if( vehicle->s.angles[0] >= 270 || vehicle->s.angles[0] < 90 )
			{
				vehicle->avelocity[2] += vehicle->aileron*VM*cospitch;
				vehicle->avelocity[1] += vehicle->aileron*sinpitch;
			}
			else
			{
				vehicle->avelocity[2] -= vehicle->aileron*VM*cospitch;
				vehicle->avelocity[1] += vehicle->aileron*sinpitch;
			}

			// auto-roll-back!
			if( (!vehicle->aileron && vehicle->owner->client->pers.autoroll && !vehicle->elevator
					&& !vehicle->rudder ) || ( vehicle->ONOFF & AUTOPILOT ) )
			{
				if( ( vehicle->s.angles[0] < 90 || vehicle->s.angles[0] >= 270 ) )
				{
					if( fabs( vehicle->s.angles[2] ) > 10 )
					{
						if( vehicle->s.angles[2]*VM > 10 )
							vehicle->avelocity[2] = -vehicle->turnspeed[2]/2*VM;
						else if( vehicle->s.angles[2]*VM < -10 )
							vehicle->avelocity[2] = vehicle->turnspeed[2]/2*VM;
					}
					else if( fabs( vehicle->s.angles[2] ) > 5 )
					{
						if( vehicle->s.angles[2]*VM > 5 )
							vehicle->avelocity[2] = -vehicle->turnspeed[2]/3*VM;
						else if( vehicle->s.angles[2]*VM < -5 )
							vehicle->avelocity[2] = vehicle->turnspeed[2]/3*VM;
					}
					else
						vehicle->s.angles[2] = 0;
				}
				else if( vehicle->s.angles[0] >= 90 && vehicle->s.angles[0] < 270 )
				{
					if( fabs( vehicle->s.angles[2] ) < 170 )
					{
						if( vehicle->s.angles[2]*VM < 170 && vehicle->s.angles[2]*VM > 0 )
							vehicle->avelocity[2] = vehicle->turnspeed[2]/2*VM;
						else if( vehicle->s.angles[2]*VM > -170 )
							vehicle->avelocity[2] = -vehicle->turnspeed[2]/2*VM;
					}
					else if( fabs( vehicle->s.angles[2] ) < 175 )
					{
						if( vehicle->s.angles[2]*VM < 175 && vehicle->s.angles[2]*VM > 0 )
							vehicle->avelocity[2] = vehicle->turnspeed[2]/3*VM;
						else if( vehicle->s.angles[2]*VM > -175 )
							vehicle->avelocity[2] = -vehicle->turnspeed[2]/3*VM;
					}
					else
						vehicle->s.angles[2]= 180;
				}
			}

			// new movement code
/*			AngleVectors( vehicle->s.angles, forward, right, up );

			vehicle->avelocity[0] = vehicle->elevator * (right[1]-right[0]*forward[1]/forward[0])+
					vehicle->rudder * ( up[1] - up[0] * forward[1] / forward[0] );
			vehicle->avelocity[1] = vehicle->elevator * (right[2]-right[0]*forward[2]/forward[0])+
					vehicle->rudder * ( up[2] - up[0] * forward[2] / forward[0] );
			vehicle->avelocity[2] = vehicle->elevator * (right[0]/forward[0]) +
					vehicle->rudder * ( up[0]/forward[0] ) +
					vehicle->aileron;
*/
			// stall plane when too slow
			if( vehicle->actualspeed < vehicle->speed )
				vehicle->ONOFF |= STALLED;

			// roll with rudder (depends on videomode)
			if( vehicle->rudder )
			{
				if( vehicle->rudder > 0 )
					vehicle->avelocity[2] += 16*VM;
				else
					vehicle->avelocity[2] -= 16*VM;
			}

			// lose height when banked
/*			if( (vehicle->s.angles[0] < 90 || vehicle->s.angles[0] >= 270)
				&& fabs(vehicle->s.angles[2]) > 20 )
				vehicle->avelocity[0] += 10;
			else if( vehicle->s.angles[0] >= 90 && vehicle->s.angles[0] < 270
				&& fabs(vehicle->s.angles[2]) < 160 )
				vehicle->avelocity[0] -= 10;
*/
		}
		// check if to land the plane
		VectorSet( underneath, vehicle->s.origin[0], vehicle->s.origin[1],
						vehicle->s.origin[2] - (abs(vehicle->mins[2])+3) );
		trace = gi.trace( vehicle->s.origin, NULL, NULL, underneath, vehicle, MASK_ALL );

			if( strcmp( trace.ent->classname, "func_runway" ) == 0
			|| strcmp( trace.ent->classname, "func_door" ) == 0
			|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0 )
			{
				if(  ( ( vehicle->s.angles[0] >= 0 && vehicle->s.angles[0] <= 40
							&& fabs(vehicle->s.angles[2]) <= 30 )
					|| ( vehicle->s.angles[0] >= 140 && vehicle->s.angles[0] <= 180
							&& fabs(vehicle->s.angles[2]) >= 150 ) )
							&& ( vehicle->ONOFF & GEARDOWN ) )
				{
					vehicle->ONOFF |= LANDED;
					vehicle->ONOFF &= ~(AUTOPILOT | STALLED );// 17
					vehicle->avelocity[0] = vehicle->avelocity[2] = 0;
					vehicle->velocity[2] = 0;
					if( vehicle->s.angles[0] >= 0 && vehicle->s.angles[0] <= 40 )
					{
						vehicle->s.angles[0] = vehicle->s.angles[2] = 0;
					}
					else
					{
						vehicle->s.angles[0] = vehicle->s.angles[2] = 180;
					}
					vehicle->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2] + abs(vehicle->mins[2])+1;
					gi.sound( vehicle, CHAN_AUTO, gi.soundindex( "flying/touchdwn.wav" ), 1, ATTN_NORM, 0 );
					gi.cprintf( vehicle->owner, PRINT_MEDIUM, "LANDED!\n" );
				}
				else if( vehicle->s.angles[0] > 180 && vehicle->s.angles[0] < 360 )
				{
					// nothing, just takeoff
				}
				else
				{
					UnlockEnemyOnDie( vehicle );
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_EXPLOSION1);
					gi.WritePosition (vehicle->s.origin);
					gi.multicast (vehicle->s.origin, MULTICAST_PVS);
					vehicle->deadflag = DEAD_DEAD;
					vehicle->thrust = THRUST_0;
//					ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
//							5, vehicle->s.origin );
//					ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
//							5, vehicle->s.origin );
					ThrowDebris( vehicle, "models/objects/debris3/tris.md2",
							5, vehicle->s.origin );
					ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
							5, vehicle->s.origin );
					VehicleExplosion( vehicle );
					SetVehicleModel (vehicle );
					if( !(vehicle->ONOFF & GEARDOWN ) )
					{
						gi.bprintf( PRINT_MEDIUM, "%s likes it the hard way.\n",
									vehicle->owner->client->pers.netname );
						meansOfDeath = MOD_GEAR;
					}
					else
					{
						gi.bprintf( PRINT_MEDIUM, "%s crashed into a runway.\n",
									vehicle->owner->client->pers.netname );
						meansOfDeath = MOD_RUNWAY;
					}
				}
			}
			// did we touch the sky?
			VectorCopy( vehicle->s.origin, above );
			above[2] += 64;
			trace = gi.trace( vehicle->s.origin, NULL, NULL, above, vehicle, MASK_ALL );
			if( trace.fraction < 1.0 && (trace.surface->flags & SURF_SKY ||
		   		strncmp(trace.surface->name,"clip",strlen(trace.surface->name))== 0) )
			{
				if( vehicle->s.angles[0] > 180 && vehicle->s.angles[0] < 360 )
				{
					if( vehicle->s.angles[0] < 270 )
						vehicle->s.angles[0] = 180;
					else
						vehicle->s.angles[0] = 0;
				}
			}

	}
	// water-death
	if( vehicle->waterlevel )
	{
		UnlockEnemyOnDie( vehicle );
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (vehicle->s.origin);
		gi.multicast (vehicle->s.origin, MULTICAST_PVS);
		vehicle->deadflag = DEAD_DEAD;
		ThrowDebris( vehicle, "models/objects/debris1/tris.md2",
			5, vehicle->s.origin );
		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
			5, vehicle->s.origin );
		ThrowDebris( vehicle, "models/objects/debris3/tris.md2",
			5, vehicle->s.origin );
		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
			5, vehicle->s.origin );
		VehicleExplosion( vehicle );
		SetVehicleModel( vehicle );
		gi.bprintf( PRINT_MEDIUM, "%s splashed.", vehicle->owner->client->pers.netname);
		meansOfDeath = MOD_PLANESPLASH;
	}

	// debugging
//	if( vehicle->aileron ||vehicle->elevator || vehicle->rudder )
//		gi.bprintf( PRINT_HIGH, "%d el %d ru %d ai\n", vehicle->elevator,
//				vehicle->rudder, vehicle->aileron );

//	gi.bprintf( PRINT_HIGH, "%.2f a[0] %.2f a[1] %.2f a[2]\n", vehicle->s.angles[0],
//				vehicle->s.angles[1], vehicle->s.angles[2] );

//	gi.bprintf( PRINT_HIGH, "%.2f up[2] %.2f |up| %.2f up[0] %.2f up[1]\n", up[2],
//   			sqrt( up[0]*up[0]+up[1]*up[1]), up[0], up[1]  );

//	gi.bprintf( PRINT_HIGH, "%d buttons\n", vehicle->owner->client->buttons );

//	gi.bprintf( PRINT_HIGH, "%f v %f h\n", vehicle->avelocity[1], vehicle->s.angles[1] );

// 	gi.bprintf( PRINT_HIGH, "%d\n", vehicle->ONOFF );

//	gi.bprintf( PRINT_HIGH, "%.2f\n", VectorLength(vehicle->velocity) );

	// give speed to the plane
	AngleVectors( vehicle->s.angles, forward, NULL, NULL );
	VectorScale( forward, vehicle->actualspeed/4 , vehicle->velocity );

	vehicle->nextthink = level.time + 0.1;

	if( !vehicle->deadflag )
	{
		if( vehicle->owner->client->pers.secondary == FindItem( "SW Launcher" ) ||
			 vehicle->owner->client->pers.secondary == FindItem( "ST Launcher" ) ||
			 vehicle->owner->client->pers.secondary == FindItem( "AM Launcher" ) ||
			 vehicle->owner->client->pers.secondary == FindItem( "PH Launcher" ) )
			SidewinderLockOn( vehicle );
		else if( vehicle->owner->client->pers.secondary == FindItem( "ATGM Launcher" )||
			vehicle->owner->client->pers.secondary == FindItem ("Maverick Launcher") ||
			vehicle->owner->client->pers.secondary == FindItem ("Laser Bombs") )
			ATGMLockOn( vehicle );
		else if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
	}
	// this time "above" is below the plane...to get the altitude for A/P
	VectorCopy( vehicle->s.origin, above );
	above[2] -= 2000;
	trace = gi.trace( vehicle->s.origin, NULL, NULL, above, vehicle, MASK_SOLID );
	if( trace.fraction < 1 )
		vehicle->altitude = ((int)(vehicle->s.origin[2] - trace.endpos[2]))-12;
	else
		vehicle->altitude = 9999;
	vehicle->radio_target = NULL;
	vehicle->ONOFF &= ~IS_CHANGEABLE;

	if( vehicle->ONOFF & BAY_OPEN && level.time > vehicle->timestamp )
		AutoCloseBay( vehicle->owner );

	if( vehicle->ONOFF & PICKLE_ON )
		Cmd_Pickle_f( vehicle->owner );

	if( vehicle->ONOFF & IS_JAMMING )
	{
		edict_t *lock = NULL;

//		gi.bprintf( PRINT_HIGH, "jam!\n");
		while( (lock = findradius( lock, vehicle->s.origin, STANDARD_RANGE*JAMMER_MODIFIER ) ) != NULL )
		{
			if( strcmp( lock->classname, "plane" ) != 0 &&
				strcmp( lock->classname, "helo" ) != 0 &&
				strcmp( lock->classname, "ground" )!= 0 )
				continue;
			if( vehicle->owner )
			{
				if( lock == vehicle->owner )
					continue;
			}
			if( !lock->takedamage )
				continue;
			if( lock->deadflag )
				continue;
			lock->jam_time = level.time + 1;
		}
	}
/*	if( vehicle->lockstatus == 1 )
		gi.sound( vehicle, CHAN_VOICE, gi.soundindex( "flying/in2.wav" ), 1, ATTN_STATIC, 0 );
	else if( vehicle->lockstatus == 2 )
		gi.sound( vehicle, CHAN_VOICE, gi.soundindex( "flying/in1.wav" ), 1, ATTN_STATIC, 0 );*/
	gi.linkentity( vehicle );
}





void plane_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( strcmp( other->classname, "trigger_radio" ) != 0 )
	{
		self->radio_target = NULL;
	}
	if( self->deadflag == DEAD_DEAD || strcmp( other->classname, "freed" ) == 0
		|| strcmp( other->classname, "debris" ) == 0 )
		return;
	if( strcmp( other->classname, "rocket" ) == 0 ||
   		strcmp( other->classname, "sidewinder" ) == 0 ||
		strcmp( other->classname, "bolt" ) == 0 ||
		strcmp( other->classname, "ATGM" ) == 0 ||
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
	if( surf && (surf->flags & SURF_SKY) )
	{
//		self->health -= 5;
		if( self->health > 0 )
			return;
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
	if( strcmp( other->classname, "func_runway") == 0 && (self->ONOFF & LANDED) )
	{
			return;
	}
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
		if( !self->deadflag || self->deadflag == DEAD_DYING )
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
			if( strcmp( other->classname, "ground" ) == 0 )
				VehicleExplosion( self );
			SetVehicleModel( self );
			gi.bprintf( PRINT_MEDIUM, "%s crashed into %s\n", self->owner->client->pers.netname,
						other->owner->client->pers.netname );
			meansOfDeath = MOD_MIDAIR;
		}
	}
	else
	{
		if( self->ONOFF & LANDED )
		{
			if( VectorLength(self->velocity) >= 5 )
			{
				self->health -= (VectorLength(self->velocity));
				MakeDamage( self );
				gi.sound( self, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
			}
			self->actualspeed = 0;
			VectorClear( self->avelocity );
			VectorClear( self->velocity );
		}
		else
		{
			self->health = 0;
		}
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
//			ThrowDebris( self, "models/objects/debris1/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
			ThrowDebris( self, "models/objects/debris3/tris.md2",
				5, self->s.origin );
			ThrowDebris( self, "models/objects/debris2/tris.md2",
				5, self->s.origin );
			VehicleExplosion( self );
			SetVehicleModel( self );
			if( !self->deadflag )
				meansOfDeath = MOD_CRASH;
			self->deadflag = DEAD_DEAD;
			if( ( strcmp( other->classname, "plane" ) == 0 ||
		   		strcmp( other->classname, "helo" ) == 0 ||
			    strcmp( other->classname, "ground" ) == 0 ) &&
				!(other->ONOFF & PILOT_ONBOARD ) &&
				!(other->HASCAN & IS_DRONE ) )
				gi.bprintf( PRINT_MEDIUM, "%s crashed into a parked vehicle!\n",
					self->owner->client->pers.netname );
			else if( other->HASCAN && IS_DRONE )
				gi.bprintf( PRINT_MEDIUM, "%s crashed into a drone (%s)!\n",
					self->owner->client->pers.netname, other->friendlyname );
			else
				gi.bprintf( PRINT_MEDIUM, "%s crashed.\n", self->owner->client->pers.netname );
		}
	}
}

