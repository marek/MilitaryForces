// file: drone.c by Bjoern Drabeck
// handles drone movement


#include "g_local.h"
#include "drone.h"

void Cmd_Spawn_Drone(edict_t *ent)
{
	edict_t	*drone;
	edict_t *start = NULL;
	int 	randnum;

	if( (stricmp(level.mapname, "foxdetha") == 0) ||
		  (stricmp(level.mapname, "airfox2b") == 0) )
	{
		gi.bprintf( PRINT_HIGH, "No drones in this level!\n" );
		return;
	}
	if( !ent->client->veh_selected )
		return;
	if( ent->deadflag || ent->vehicle->deadflag )
		return;
	if( !deathmatch->value )
	{
		gi.bprintf( PRINT_HIGH, "No drones in single-player!\n" );
		return;
	}

	if( drone_num >= ceil(maxdrones->value) )
	{
		gi.bprintf( PRINT_HIGH, "Already %.0f drones!\n", ceil(maxdrones->value) );
		return;
	}
	else if( drone_num >= 12 )
	{
		gi.bprintf( PRINT_HIGH, "Not more than 12 drones allowed!\n", maxdrones->value );
		return;
	}
	if( !(level.category & 16) )
	{
		gi.bprintf( PRINT_HIGH, "No drone spawn-points in this level!\n");
		return;
	}
	start = finddronestart(0);

	if( !start )
	{
		gi.bprintf( PRINT_HIGH,	"No start point for drone!\n" );
		return;
	}

	drone = G_Spawn ();

	VectorCopy( start->s.origin, drone->s.origin );
	VectorSet( drone->s.angles, 0, random()*360, 0 );

	randnum = (random()*8)+1;

	switch( randnum )
	{
		case 1:
				Start_F14( drone );
				break;
		case 2:
				Start_F22( drone );
				break;
		case 3:
				Start_F16( drone );
				break;
		case 4:
				Start_F4( drone );
				break;
		case 5:
				Start_SU37( drone );
				break;
		case 6:
				Start_MiG21( drone );
				break;
		case 7:
				Start_MiG29( drone );
				break;
		case 8:
				Start_MiGMFI( drone );
				break;
		default:
				Start_MiG21( drone );
				break;
	}

	drone->HASCAN |= IS_DRONE;
	drone->mass = 1000;
	drone->solid = SOLID_BBOX;
	drone->deadflag = DEAD_NO;
	drone->clipmask = MASK_PLAYERSOLID;
	drone->waterlevel = 0;
	drone->watertype = 0;
	drone->s.skinnum = 0;

	drone->recharge_time = level.time;
	drone->deathtime = level.time;

	drone->s.frame = 0;
	drone->touch = drone_touch;
	drone->pain = drone_pain;
	drone->die = drone_die;
	drone->think = SteerDrone;
	drone->nextthink = level.time + 0.1;
	drone->drone_decision_time = level.time + (random()*3+3);
	drone->ONOFF = NOTHING_ON;

	if( drone->HASCAN & IS_EASTERN )
		drone->aqteam = 2;
	else
		drone->aqteam = 1;
	drone->count = 6;		// number of sidewinders that drone has

	drone_num ++;

	if( teamplay->value == 1 )
	{
		team[drone->aqteam-1].drones++;
	}

	gi.bprintf( PRINT_MEDIUM, "Drone spawned (%s)!\n", drone->friendlyname );
}


void drone_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->s.effects = EF_ROCKET;

}

void drone_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	if( attacker )
	{
		if( attacker != self && !(attacker->HASCAN & IS_DRONE) && attacker->owner)// drone
		{
			if( attacker->owner->client )
			{
				if( attacker->enemy )
				{
					if( attacker->enemy == self )
						attacker->enemy = NULL;
				}
				if( !self->deadflag )
				{
					if( teamplay->value == 1 )
					{
						if( self->aqteam != attacker->aqteam )
						{
							attacker->owner->client->resp.score += 1;
							team[attacker->owner->client->pers.team-1].score += 1;
							if( self->HASCAN & IS_EASTERN )
								gi.bprintf( PRINT_MEDIUM, "[%s]%s shot down a Warsaw Pact drone (%s)!\n",
									team[attacker->owner->client->pers.team-1].name,
									attacker->owner->client->pers.netname, self->friendlyname );
							else
								gi.bprintf( PRINT_MEDIUM, "[%s]%s shot down a NATO drone (%s)!\n",
									team[attacker->owner->client->pers.team-1].name,
									attacker->owner->client->pers.netname, self->friendlyname );
						}
						else
						{
							attacker->owner->client->resp.score -= 2;
							team[attacker->owner->client->pers.team-1].score -= 2;
							gi.bprintf( PRINT_MEDIUM, "[%s]%s killed a teammate-drone (%s)!\n",
								team[attacker->owner->client->pers.team-1].name,
								attacker->owner->client->pers.netname, self->friendlyname );
						}
					}
					else
					{
						attacker->owner->client->resp.score += 1;
						team[attacker->owner->client->pers.team-1].score += 1;
						gi.bprintf( PRINT_MEDIUM, "%s shot down a drone (%s)!\n",
								attacker->owner->client->pers.netname, self->friendlyname );
					}
				}
			}
		}
		else if( attacker->HASCAN & IS_DRONE )
		{
			gi.bprintf( PRINT_MEDIUM, "A [%s]drone (%s) was shot down by a [%s]drone (%s)!\n",
				team[self->aqteam-1].name, self->friendlyname,
				team[attacker->aqteam-1].name, attacker->friendlyname );
			if( teamplay->value == 1 )
				team[attacker->aqteam-1].score += 2;
		}
	}


	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->deadflag = DEAD_DYING;
	self->thrust = THRUST_0;
	if(	strcmp( self->classname, "plane" ) == 0 )
	{
		self->avelocity[2] = 60;
		self->avelocity[1] = 0;
		if( self->ONOFF & LANDED )// landed
		{
			self->deadflag = DEAD_DEAD;
			drone_num --;
			if( teamplay->value == 1 )
			{
				team[self->aqteam-1].drones--;
			}
			SetVehicleModel( self );
			UnlockEnemyOnDie( self );
			self->think = G_FreeEdict;
			self->nextthink = level.time + 2;
		}
	}


}


void SteerDrone( edict_t *vehicle )
{
	vec3_t		forward, targetangle,forward2;
	vec3_t		pos1,pos2,pos3,pos4;
	trace_t 	trace,traceup,tracedown,traceleft,traceright;
	vec3_t 		above,border1,border2;
	float		sinpitch,cospitch;
	float		sinroll,cosroll,decision;
	int			VM = 1;
	edict_t		*lock;

	VectorClear( vehicle->avelocity );
	vehicle->rudder= vehicle->aileron = vehicle->elevator = 0;

	// plot course
	VectorCopy( vehicle->mins, border1 );
	VectorCopy( vehicle->maxs, border2 );
	border1[0] -= 3;
	border1[1] -= 3;
	border1[2] -= 3;
	border2[0] += 3;
	border2[1] += 3;
	border2[2] += 3;
	VectorCopy( vehicle->s.angles, pos1 );
	if( vehicle->s.angles[0] < 90 ||vehicle->s.angles[0] > 270 )
		pos1[0] -= 40;
	else
		pos1[0] += 40;
	VectorCopy( vehicle->s.angles, pos2 );
	if( vehicle->s.angles[0] < 90 ||vehicle->s.angles[0] > 270 )
		pos2[0] += 40;
	else
		pos2[0] -= 40;
	VectorCopy( vehicle->s.angles, pos3 );
	pos3[1] += 22;
	VectorCopy( vehicle->s.angles, pos4 );
	pos4[1] -= 22;

	AngleVectors( vehicle->s.angles, forward, NULL, NULL );
	VectorMA( vehicle->s.origin, 650, forward, forward );
	trace = gi.trace( vehicle->s.origin, border1, border2, forward, vehicle, MASK_ALL );

	AngleVectors( pos1, forward, NULL, NULL );
	VectorMA( vehicle->s.origin, 400, forward, forward );
	traceup = gi.trace( vehicle->s.origin, NULL, NULL, forward, vehicle, MASK_ALL );

	AngleVectors( pos2, forward, NULL, NULL );
	VectorMA( vehicle->s.origin, 400, forward, forward );
	tracedown = gi.trace( vehicle->s.origin, NULL, NULL, forward, vehicle, MASK_ALL );

	AngleVectors( pos3, forward, NULL, NULL );
	VectorMA( vehicle->s.origin, 450, forward, forward );
	traceleft = gi.trace( vehicle->s.origin, NULL, NULL, forward, vehicle, MASK_ALL );

	AngleVectors( pos4, forward, NULL, NULL );
	VectorMA( vehicle->s.origin, 450, forward, forward );
	traceright = gi.trace( vehicle->s.origin, NULL, NULL, forward, vehicle, MASK_ALL );


	// drone terrain following
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
		vehicle->drone_decision_time = level.time + random()*2 + 2;
		vehicle->ONOFF &= ~(AUTOPILOT | AUTOROTATION);
	}
	else	// neutralize rudders
	{
		vehicle->elevator = 0;
		vehicle->rudder = 0;
		vehicle->aileron = 0;
	}
	// dont touch ground
	if( tracedown.fraction < 0.3 )
		vehicle->elevator -= 160;
	else if( traceup.fraction < 0.3 )
		vehicle->elevator += 160;
	// continue loopings
	if( vehicle->s.angles[0] > 230 && vehicle->s.angles[0] < 310 )
		vehicle->elevator = -350;
	// adjust throttle on speed
	if( vehicle->actualspeed < vehicle->speed + 80  )
	{
		vehicle->thrust ++;
		vehicle->ONOFF &= ~BRAKE;
		if( vehicle->thrust > THRUST_100 )
		{
			if( vehicle->HASCAN & HAS_AFTERBURNER ) // afterburner
			{
				vehicle->thrust = THRUST_AFTERBURNER;
				vehicle->s.skinnum = 1;
			}
			else
		 		vehicle->thrust = THRUST_100;
		}
//		gi.bprintf( PRINT_HIGH, "Thrust to %d\n", vehicle->thrust );
	}
	if( vehicle->actualspeed > vehicle->topspeed + 160 &&
			vehicle->thrust > THRUST_0 )
	{
		vehicle->ONOFF &= ~BRAKE;
		vehicle->thrust --;
		vehicle->s.skinnum = 0;
//		gi.bprintf( PRINT_HIGH, "Thrust to %d\n", vehicle->thrust );
	}
	if( vehicle->actualspeed > vehicle->topspeed + 200 &&
			vehicle->thrust > THRUST_0 )
	{
		vehicle->ONOFF |= BRAKE;
	}

	VectorCopy( vehicle->s.origin, above );
	above[2] += 200;
	trace = gi.trace( vehicle->s.origin, NULL, NULL, above, vehicle, MASK_ALL );
	if( trace.fraction < 1.0 && (trace.surface->flags & SURF_SKY ||
   		strncmp(trace.surface->name,"clip",strlen(trace.surface->name))== 0) &&
		!vehicle->elevator )
	{
		vehicle->elevator = 200;
	}

	// combat skills
	if( !vehicle->rudder &&	!vehicle->elevator )
	{
		// evade missiles
		if( vehicle->lockstatus > 1 )
		{
			if( !(vehicle->ONOFF & AUTOROTATION) )
			{
//				gi.bprintf( PRINT_HIGH, "asdhu\n" );
				vehicle->pos1[0] = vehicle->elevator = 100 * (((random()*10) > 5) ? -1 : 1);
				vehicle->pos1[1] = vehicle->rudder = 100 * (((random()*10) > 5) ? -1 : 1);
				vehicle->ONOFF |= AUTOROTATION;
			}
			else
			{
//				gi.bprintf( PRINT_HIGH, "%.0f %.0f\n", vehicle->pos1[0], vehicle->pos1[1] );
				vehicle->elevator = vehicle->pos1[0];
				vehicle->rudder = vehicle->pos1[1];
			}
		}
		else if( vehicle->lockstatus == 1 && !(vehicle->ONOFF & AUTOROTATION) )
		{
			if( !(vehicle->ONOFF & AUTOPILOT) )
			{
//				gi.bprintf( PRINT_HIGH, "asdhu\n" );
				vehicle->pos1[0] = vehicle->elevator = 80 * (((random()*10) > 5) ? -1 : 1);
				vehicle->pos1[1] = vehicle->rudder = 60 * (((random()*10) > 5) ? -1 : 1);
				vehicle->ONOFF |= AUTOPILOT;
			}
			else
			{
//				gi.bprintf( PRINT_HIGH, "%.0f %.0f\n", vehicle->pos1[0], vehicle->pos1[1] );
				vehicle->elevator = vehicle->pos1[0];
				vehicle->rudder = vehicle->pos1[1];
			}
		}
		// come to enemy
		else
		{
			vehicle->ONOFF &= ~AUTOROTATION;
			vehicle->ONOFF &= ~AUTOPILOT;
			if( vehicle->enemy )
			{
 				if( !vehicle->enemy->deadflag )
				{
					float difference;
					VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, vehicle->pos2 );
					// rudder
					difference = vehicle->pos2[1]-vehicle->s.angles[1];
					if( difference > 180 )
						difference -= 360;
					else if( difference < -180 )
						difference += 360;
					if( fabs( difference ) > 5 )
					{
						if( difference > 0 )
							vehicle->rudder = 80;
						else
							vehicle->rudder = -80;
					}
					else
						vehicle->s.angles[1] = vehicle->pos2[1];
					// apply elevator
					if( vehicle->pos2[0] > 180 )
						vehicle->pos2[0] -= 360;
					else if( vehicle->pos2[0] < -180 )
						vehicle->pos2[0] += 360;
					if( vehicle->pos2[0] > vehicle->s.angles[0] + 3 )
						vehicle->elevator = 40;
					else if( vehicle->pos2[0] < vehicle->s.angles[0] - 3 )
						vehicle->elevator = -40;
					else
						vehicle->s.angles[0] = vehicle->pos2[0];
				}
			}
		}
		if( vehicle->elevator || vehicle->rudder )
			vehicle->drone_decision_time = level.time + random()*2 + 2;
	}

	// drone random decisions
	if( level.time > vehicle->drone_decision_time && !vehicle->rudder &&
			!vehicle->elevator && !vehicle->enemy )
	{
		vehicle->ONOFF &= ~(AUTOROTATION | AUTOPILOT);
		decision = floor(random() * 10);
		if( decision == 1 )
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to climb!\n" );
			vehicle->elevator = -160;
		}
		else if( decision == 2 )
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to descend!\n" );
			vehicle->elevator = 160;
		}
		else if( decision == 3 )
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to steep climb!\n" );
			vehicle->elevator = -350;
		}
		else if( decision == 4 )
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to steep descend!\n" );
			vehicle->elevator = 350;
		}
		else if( decision == 5 )
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to turn left!\n" );
			vehicle->rudder = 200;
		}
		else if( decision == 6 )
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to turn right!\n" );
			vehicle->rudder = -200;
		}
		else if( decision == 7 )
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to turn hard left!\n");
			vehicle->rudder = 400;
		}
		else if( decision == 8 )
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to turn hard right!\n");
			vehicle->rudder = -400;
		}
		else if( decision == 9 )
		{
			vehicle->ONOFF |= BRAKE;
		}
		else
		{
//			gi.bprintf( PRINT_HIGH, "Drone decides to stay at attitude!\n" );
		}


		vehicle->drone_decision_time = level.time + (random()*3+2);
	}


	// fire flares
	if( vehicle->lockstatus == 2 )
	{
//		gi.bprintf( PRINT_HIGH, "Enemy has fired!\n" );
		if( vehicle->flare && level.time > vehicle->flare_time )
		{
			fire_flare (vehicle, vehicle->s.origin, forward2,
			0, 180, EF_BLASTER, 0 );// +BD new version
			vehicle->flare --;
			vehicle->flare_time = level.time + 1.0;
		}
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



	// health-effects
	if( vehicle->health >= 26 )
		vehicle->s.effects = 0;
	else if( vehicle->health < 26 && vehicle->health > 10 )
	{
		vehicle->s.effects = EF_GRENADE;
	}
	else if( vehicle->health <= 10 && vehicle->deadflag != DEAD_DEAD )
	{
		vehicle->s.effects = EF_ROCKET;
	}
	else
		vehicle->s.effects = 0;


	SetPlaneSpeed( vehicle );

	// you can destroy dying planes
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
//		ThrowDebris( vehicle, "models/objects/debris3/tris.md2",
//			5, vehicle->s.origin );
//		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
//			5, vehicle->s.origin );
		VehicleExplosion( vehicle );
		SetVehicleModel(vehicle);
		vehicle->think = G_FreeEdict;
		vehicle->nextthink = level.time + 2;
		vehicle->deadflag = DEAD_DEAD;
		if( teamplay->value == 1 )
		{
			team[vehicle->aqteam-1].drones--;
		}
		drone_num--;
	}
	// unlock enemies on death
	if( vehicle->deadflag && vehicle->enemy )
	{
		vehicle->enemy->lockstatus = 0;
		vehicle->enemy = NULL;
	}
	// dead planes are not steered
	if( vehicle->deadflag == DEAD_DEAD )
	{
		gi.sound( vehicle, CHAN_BODY, gi.soundindex( "death/0.wav" ), 1, ATTN_NORM, 0 );
		vehicle->s.frame = 0;
		vehicle->s.effects = 0;
		return;
	}
	// dying planes spin
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

	// reset angular velocities
	if( !(vehicle->ONOFF & STALLED) )// stalled
		vehicle->avelocity[0] = vehicle->avelocity[1] = vehicle->avelocity[2] = 0;

	// check for overturning
	vehicle->s.angles[0] = anglemod( vehicle->s.angles[0] );
	vehicle->s.angles[1] = anglemod( vehicle->s.angles[1] );
	if( vehicle->s.angles[2] > 180 )
		vehicle->s.angles[2] -= 360;
	else if( vehicle->s.angles[2] < -180 )
		vehicle->s.angles[2] += 360;

	// aerial movement
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
//			if( !vehicle->owner->client->pers.autocoord )
			{
				vehicle->avelocity[0] = vehicle->elevator*cosroll;
				vehicle->avelocity[1] = vehicle->elevator*sinroll;
			}
/*			else
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
			}*/

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

//			if( !vehicle->aileron && vehicle->owner->client->pers.autoroll && !vehicle->elevator
//					&& !vehicle->rudder )
			{
				if( ( vehicle->s.angles[0] < 90 || vehicle->s.angles[0] >= 270 )
					&& fabs(vehicle->s.angles[2]) > 6 )
				{
					if( vehicle->s.angles[2]*VM > 6 )
						vehicle->avelocity[2] = -vehicle->turnspeed[2]/2*VM;
					else if( vehicle->s.angles[2]*VM < -6 )
						vehicle->avelocity[2] = vehicle->turnspeed[2]/2*VM;
				}
				else if( vehicle->s.angles[0] >= 90 && vehicle->s.angles[0] < 270
					&& fabs( vehicle->s.angles[2]) < 174 )
				{
					if( vehicle->s.angles[2]*VM < 174 && vehicle->s.angles[2]*VM > 0 )
						vehicle->avelocity[2] = vehicle->turnspeed[2]/2*VM;
					else if( vehicle->s.angles[2]*VM > -174 )
						vehicle->avelocity[2] = -vehicle->turnspeed[2]/2*VM;
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
					vehicle->avelocity[2] += 48*VM;
				else
					vehicle->avelocity[2] -= 48*VM;

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


	// water-death
	if( vehicle->waterlevel )
	{
		UnlockEnemyOnDie( vehicle );
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (vehicle->s.origin);
		gi.multicast (vehicle->s.origin, MULTICAST_PVS);
		VehicleExplosion( vehicle );
		vehicle->deadflag = DEAD_DEAD;
//		ThrowDebris( vehicle, "models/objects/debris1/tris.md2",
//			5, vehicle->s.origin );
//		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
//			5, vehicle->s.origin );
//		ThrowDebris( vehicle, "models/objects/debris3/tris.md2",
//			5, vehicle->s.origin );
//		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
//			5, vehicle->s.origin );
		SetVehicleModel( vehicle );
		vehicle->think = G_FreeEdict;
		vehicle->nextthink = level.time + 2;
		if( teamplay->value == 1 )
		{
			team[vehicle->aqteam-1].drones--;
		}
		drone_num--;
//		gi.bprintf( PRINT_MEDIUM, "%s splashed.", vehicle->owner->client->pers.netname);
//		meansOfDeath = MOD_PLANESPLASH;
	}

	// debugging
//	gi.bprintf( PRINT_HIGH, "%d el %d ru %d ai\n", vehicle->elevator,
//				vehicle->rudder, vehicle->aileron );

//	gi.bprintf( PRINT_HIGH, "%.2f a[0] %.2f a[1] %.2f a[2]\n", vehicle->s.angles[0],
//				vehicle->s.angles[1], vehicle->s.angles[2] );

//	gi.bprintf( PRINT_HIGH, "%.2f up[2] %.2f |up| %.2f up[0] %.2f up[1]\n", up[2],
//   			sqrt( up[0]*up[0]+up[1]*up[1]), up[0], up[1]  );

//	gi.bprintf( PRINT_HIGH, "%d buttons\n", vehicle->owner->client->buttons );

//	gi.bprintf( PRINT_HIGH, "%f v %f h\n", vehicle->avelocity[1], vehicle->s.angles[1] );

	// give speed to the plane
	AngleVectors( vehicle->s.angles, forward, NULL, NULL );
	VectorScale( forward, vehicle->actualspeed/4 , vehicle->velocity );

	vehicle->nextthink = level.time + 0.1;

	if( !vehicle->deadflag )
	{
		DroneSidewinderLockOn( vehicle );
	}

	if( vehicle->enemy )
	{
		if( level.time > vehicle->recharge_time && vehicle->count &&
				!vehicle->enemy->deadflag )
		{
			if( vehicle->pos1[2] < 60 )
			{
				Drone_Sidewinder_Fire( vehicle );
				vehicle->count --;
				vehicle->recharge_time = level.time + 1.6;
/*				if( vehicle->pos1[2] < 25 )
				{
					vec3_t	start;
//					gi.bprintf( PRINT_HIGH, "Shooting solution!\n");
					AngleVectors( vehicle->s.angles, forward, NULL, NULL );
					VectorScale( forward, vehicle->maxs[0], start );
					fire_blaster ( vehicle, start, forward,
						vehicle->gundamage, 2400, EF_HYPERBLASTER, true );
					gi.WriteByte (svc_muzzleflash);
					gi.WriteShort (vehicle-g_edicts);
					gi.WriteByte (MZ_CHAINGUN3);
					gi.multicast (vehicle->s.origin, MULTICAST_PVS);
				}*/
			}
		}
	}

	vehicle->elevator = 0;
	vehicle->rudder = 0;
	vehicle->aileron = 0;

//	gi.bprintf( PRINT_HIGH, "%.0f\n", vehicle->pos1[2] );

	gi.linkentity( vehicle );
}


void drone_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( self->deadflag == DEAD_DEAD || strcmp( other->classname, "freed" ) == 0
		|| strcmp( other->classname, "debris" ) == 0 )
		return;
	if( strcmp( other->classname, "rocket" ) == 0 ||
   		strcmp( other->classname, "sidewinder" ) == 0 ||
		strcmp( other->classname, "bolt" ) == 0 ||
		strcmp( other->classname, "ATGM" ) == 0 ||
		strcmp( other->classname, "shell" ) == 0 ||
		strcmp( other->classname, "bomb" ) == 0 )
		return;
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
//		ThrowDebris( self, "models/objects/debris1/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris3/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//			5, self->s.origin );
		VehicleExplosion( self );
		SetVehicleModel( self );
		self->think = G_FreeEdict;
		self->nextthink = level.time + 2;
		self->deadflag = DEAD_DEAD;
		if( teamplay->value == 1 )
		{
			team[self->aqteam-1].drones--;
		}
		drone_num --;
	}
	else if( strcmp( other->classname, "func_runway") == 0
   		 && ( self->ONOFF & LANDED ) )
		return;
	else if( ( strcmp( other->classname, "plane") == 0 || strcmp( other->classname, "helo" ) == 0
				|| strcmp( other->classname, "ground") == 0 )
				&& !(other->HASCAN & IS_DRONE) ) // drone
	{
		if( !self->deadflag )
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
			self->deadflag = DEAD_DEAD;
//			ThrowDebris( self, "models/objects/debris1/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris3/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
			VehicleExplosion( self );
			SetVehicleModel( self );
			self->think = G_FreeEdict;
			self->nextthink = level.time + 2;
			if( teamplay->value == 1 )
			{
				team[self->aqteam-1].drones--;
			}
			drone_num --;
		}
	}
	else
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
		ThrowDebris( self, "models/objects/debris1/tris.md2",
			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris3/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//			5, self->s.origin );
		VehicleExplosion( self );
		SetVehicleModel( self );
		self->think = G_FreeEdict;
		self->nextthink = level.time + 2;
		self->deadflag = DEAD_DEAD;
		if( teamplay->value == 1 )
		{
			team[self->aqteam-1].drones--;
//			team[self->aqteam-1].score--;
		}
		drone_num --;
		gi.bprintf( PRINT_MEDIUM, "A drone crashed.(%s)\n",
	   		self->friendlyname );
	}
}

/*edict_t *finddronestart()
{
	trace_t trace;
	vec3_t underneath;
	edict_t *start = NULL;

	while( 1 )
	{
	// WRITE A NEW SELECTSPAWNFUNCTION!
		start = SelectDeathmatchSpawnPoint();

		VectorSet( underneath, start->s.origin[0], start->s.origin[1],
				start->s.origin[2] - 128 );

		trace = gi.trace( start->s.origin, NULL, NULL, underneath, start, MASK_ALL );

		if( strcmp( trace.ent->classname, "func_runway" ) != 0 )
			break;
	}

	return start;
}*/
edict_t *finddronestart (int tmnr)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;
	trace_t	tr;
	vec3_t  up, down;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

// PUT THE TEAMPLAY START IN HERE!

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if( !(spot->sounds & 16) )
			continue;

		VectorCopy( spot->s.origin, up );
		up[2] += spot->maxs[2];
		VectorCopy( spot->s.origin, down );
		down[2] += spot->mins[2];

		tr = gi.trace( up, spot->maxs, spot->mins, down, spot, MASK_ALL );

		if( tr.fraction < 1 )
		{
			gi.bprintf( PRINT_LOW, "Drone spot not free...selecting another...\n" );
			continue;
		}

		count++;
		range = PlayersRangeFromSpot(spot);
		if (range < range1)
		{
			range1 = range;
			spot1 = spot;
		}
		else if (range < range2)
		{
			range2 = range;
			spot2 = spot;
		}
	}

	if (!count)
		return NULL;

	if (count <= 2)
	{
		spot1 = spot2 = NULL;
	}
	else
		count -= 2;

	selection = rand() % count;

	spot = NULL;
	do
	{
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");

		if (spot == spot1 || spot == spot2 || !(spot->sounds & 16) )
			selection++;
	} while(selection--);

	return spot;
}


void DroneSidewinderLockOn( edict_t *vehicle )
{
	edict_t		*lock = NULL;
	vec3_t		testvec, forward;
	float		test;

	if( vehicle->enemy && ( vehicle->enemy->deadflag || strcmp( vehicle->enemy->classname, "freed") == 0) )
	{
		vehicle->enemy->lockstatus = 0;
		vehicle->enemy = NULL;
	}


/*	if( vehicle->owner->client->pers.weapon != FindItem( "SW Launcher" ) )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		return;
	}
*/
/*	if( vehicle->owner->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] == 0 )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		return;
	}
*/
/*	if( vehicle->enemy )
	{
		if( vehicle->enemy->drone )
			vehicle->enemy = NULL;
	}
*/	// was not necessary
	if( vehicle->enemy )
	{
		if( !visible(vehicle,vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		else if( !infront(vehicle, vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		if( vehicle->enemy ) // if there still is a lock
		{
			if( !infront(vehicle,vehicle->enemy) )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy )	// stealth
		{
			if( vehicle->enemy->stealth > 0 )
			{
				if( !( vehicle->enemy->ONOFF & BAY_OPEN) && !(vehicle->enemy->ONOFF & GEARDOWN) )
				{
					if( (int)(random()*100) <= vehicle->enemy->stealth	)
					{
						vehicle->enemy->lockstatus = 0;
						vehicle->enemy = NULL;
					}
				}
			}
		}
		if( vehicle->enemy )	// jammer
		{
			if( level.time < vehicle->enemy->jam_time )
			{
				if( (int)(random()*100) <= 60 )
				{
					vehicle->enemy->lockstatus = 0;
					vehicle->enemy = 0;
				}
			}
		}
		if( vehicle->enemy )
		{
			AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			vehicle->pos1[2] = test;
		}
		else
			vehicle->pos1[2] = 180;
		if( vehicle->enemy ) //if enemy is not lost then show him we lock
			if( !vehicle->enemy->lockstatus )
				vehicle->enemy->lockstatus = 1;
	}

	if( !vehicle->enemy )
	{
		while( (lock = findradius( lock, vehicle->s.origin, STANDARD_RANGE ) ) != NULL )
		{
			if( lock == vehicle )
				continue;
			if( strcmp( lock->classname, "plane" ) != 0 &&
				strcmp( lock->classname, "helo" ) != 0 &&
				!(lock->svflags & SVF_MONSTER) )
				continue;
			if( vehicle->owner )
				if( lock == vehicle->owner )
					continue;
			if( !lock->takedamage )
				continue;
			if( !visible(vehicle,lock) )
				continue;
			if( !infront(vehicle, lock) )
				continue;
			if( lock->ONOFF & LANDED && !attacklanded->value )
				continue;
			// stealth
			if( lock->stealth > 0 )
			{
				if( !( lock->ONOFF & BAY_OPEN) && !(lock->ONOFF & GEARDOWN) )
				{
					if( (int)(random()*100) <= lock->stealth )
						continue;
				}
			}
			// jammer
			if( level.time < lock->jam_time )
			{
				if( (int)(random()*100) <= 60 )
					continue;
			}
			if( teamplay->value == 1 )
			{
				if( lock->aqteam == vehicle->aqteam )
					continue;
			}
			else
			{
				if( lock->HASCAN & IS_DRONE ) // drone
					continue;
			}
			// test if within the target-cone
			AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			VectorSubtract( lock->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			vehicle->pos1[2] = test;

			vehicle->enemy = lock;
			lock->lockstatus = 1;
		}
	}

}


void SpawnTeamDrone( void )
{
	edict_t	*drone;
	edict_t *start = NULL;
	int 	randnum;
	int		eastnumber, westnumber;

	if( teamplay->value == 1 )
		drone_side = drone_side ? 0 : 1;
	else
	{
		randnum = random()*10;
		if( randnum >= 5 )
			drone_side = 1;
		else
			drone_side = 0;
	}

	if( eastdrones->value >= maxdrones->value )
		eastnumber = maxdrones->value;
	else if( eastdrones->value < 0 )
		eastnumber = eastdrones->value;
	else
		eastnumber = eastdrones->value;
	westnumber = maxdrones->value - eastnumber;

	if( drone_side && team[TEAM2].drones >= eastnumber )
		return;

	if( !drone_side && team[TEAM1].drones >= westnumber )
		return;

	if( (stricmp(level.mapname, "foxdetha") == 0) ||
		  (stricmp(level.mapname, "airfox2b") == 0) )
	{
		gi.bprintf( PRINT_HIGH, "No drones in this level!\n" );
		return;
	}
	if( !deathmatch->value )
	{
		gi.bprintf( PRINT_HIGH, "No drones in single-player!\n" );
		return;
	}

	if( drone_num >= ceil(maxdrones->value) )
	{
		gi.bprintf( PRINT_HIGH, "Already %.0f drones!\n", ceil(maxdrones->value) );
		return;
	}
	else if( drone_num >= 12 )
	{
		gi.bprintf( PRINT_HIGH, "Not more than 12 drones allowed!\n", maxdrones->value );
		return;
	}
	if( !(level.category & 16) )
	{
		gi.bprintf( PRINT_HIGH, "No drone spawn-points in this level!\n");
		return;
	}
	start = finddronestart(drone_side+1);

	if( !start )
	{
		gi.bprintf( PRINT_HIGH,	"No start point for drone!\n" );
		return;
	}

	drone = G_Spawn ();

	VectorCopy( start->s.origin, drone->s.origin );
	VectorSet( drone->s.angles, 0, random()*360, 0 );

	if( !drone_side )
		randnum = (random()*4)+1;
	else
		randnum = (random()*4)+5;

	switch( randnum )
	{
		case 1:
				Start_F14( drone );
				break;
		case 2:
				Start_F22( drone );
				break;
		case 3:
				Start_F16( drone );
				break;
		case 4:
				Start_F4( drone );
				break;
		case 5:
				Start_SU37( drone );
				break;
		case 6:
				Start_MiG21( drone );
				break;
		case 7:
				Start_MiG29( drone );
				break;
		case 8:
				Start_MiGMFI( drone );
				break;
		default:
				Start_MiG21( drone );
				break;
	}

	drone->HASCAN |= IS_DRONE;
	drone->mass = 1000;
	drone->solid = SOLID_BBOX;
	drone->deadflag = DEAD_NO;
	drone->clipmask = MASK_PLAYERSOLID;
	drone->waterlevel = 0;
	drone->watertype = 0;
	drone->s.skinnum = 0;

	drone->recharge_time = level.time;
	drone->deathtime = level.time;

	drone->s.frame = 0;
	drone->touch = drone_touch;
	drone->pain = drone_pain;
	drone->die = drone_die;
	drone->think = SteerDrone;
	drone->nextthink = level.time + 0.1;
	drone->drone_decision_time = level.time + (random()*3+3);
	drone->ONOFF = NOTHING_ON;

	if( drone->HASCAN & IS_EASTERN )
		drone->aqteam = 2;
	else
		drone->aqteam = 1;
	drone->count = 6;		// number of sidewinders that drone has

	drone_num ++;

	if( teamplay->value == 1 )
	{
		team[drone->aqteam-1].drones++;
	}

	gi.bprintf( PRINT_MEDIUM, "Drone spawned (%s)!\n", drone->friendlyname );
}
