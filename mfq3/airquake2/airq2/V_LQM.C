// file: v_lqm.c
// handles LQM stuff



#include "g_local.h"
#include "v_lqm.h"



void SteerLQM( edict_t *vehicle)
{
	vec3_t	hSpeed, forward, up, below;
	trace_t	tr;

	vehicle->s.angles[0] = anglemod( vehicle->s.angles[0] );
	vehicle->s.angles[1] = anglemod( vehicle->s.angles[1] );

	if( level.time > vehicle->drone_decision_time )
	{
		if( !(vehicle->ONOFF & LANDED) && (vehicle->ONOFF & AUTOROTATION) )	// open chute
		{
			vehicle->s.frame = 0;
			VectorClear( vehicle->velocity );
			vehicle->s.modelindex2 = gi.modelindex ("vehicles/infantry/pilot/parachute.md2");
			vehicle->ONOFF &= ~AUTOROTATION;
		}
	}

	if( !(vehicle->ONOFF & LANDED) )// flying
	{
		if( vehicle->ONOFF & AUTOROTATION ) // no parachute
		{
			if( vehicle->avelocity[0] < 0 )
				vehicle->avelocity[0] += 5;
			else
				vehicle->avelocity[0] = 0;
			if( fabs( vehicle->avelocity[2] ) > 5 )
			{
				if( vehicle->avelocity[2] > 0 )
					vehicle->avelocity[2] -= 2;
				else
					vehicle->avelocity[2] += 2;
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
			VectorCopy( vehicle->velocity, hSpeed );
			hSpeed[2] = 0;
			VectorScale( hSpeed, 0.9, hSpeed );
			hSpeed[2] = vehicle->velocity[2];
			VectorCopy( hSpeed, vehicle->velocity );
			SV_AddGravity( vehicle );
		}
		else	// chute open
		{
			if( fabs(vehicle->s.angles[2]) > 10 && fabs(vehicle->s.angles[2]) < 90)
			{
				if( vehicle->s.angles[2] > 0 )
					vehicle->avelocity[2] = -55;
				else
					vehicle->avelocity[2] = 55;
			}
			else if( fabs(vehicle->s.angles[2]) > 90 && fabs(vehicle->s.angles[2]) < 170)
			{
				if( vehicle->s.angles[2] > 180 )
					vehicle->avelocity[2] = -55;
				else
					vehicle->avelocity[2] = 55;
			}
			else
				vehicle->s.angles[2] = 0;

			if( vehicle->s.angles[0] >= 270 || vehicle->s.angles[0] < 90 )
			{
				if( vehicle->s.angles[0] < 90 && vehicle->s.angles[0] > 8 )
					vehicle->avelocity[0] = -55;
				else if( vehicle->s.angles[0] >= 270 && vehicle->s.angles[0] < 350 )
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

			if( vehicle->velocity[2] <= -50 )
				vehicle->velocity[2] = -50;
			else
				vehicle->velocity[2] -= 2;
			vehicle->avelocity[1] = vehicle->aileron;
		}
	}
	else	// on ground
	{
		if( !vehicle->deadflag )
		{
			if( !vehicle->thrust )
			{
				vehicle->s.frame++;
				if( vehicle->s.frame > 31 )
					vehicle->s.frame = 1;
			}
			else
			{
				vehicle->s.frame++;
				if( vehicle->s.frame < 32 || vehicle->s.frame > 37 )
					vehicle->s.frame = 32;
			}
		}
		else
		{
			if( vehicle->s.frame < 43 )
				vehicle->s.frame = 43;
			if( vehicle->s.frame < 56 )
				vehicle->s.frame ++;
		}
		switch( vehicle->thrust )
		{
			case 1:
					vehicle->actualspeed = 32;
					break;
			case 2:
					vehicle->actualspeed = 64;
					break;
			case 3:
					vehicle->actualspeed = 96;
					break;
			case 4:
					vehicle->actualspeed = 128;
					break;
			default:
					vehicle->actualspeed = 0;
					break;
		}
		VectorCopy( vehicle->s.origin, up );
		up[2] += 12;
		VectorCopy( vehicle->s.origin, below );
		below[2] -= 28;
		tr = gi.trace( up, NULL, NULL, below, vehicle, MASK_ALL );

		if( tr.fraction < 1 )
			vehicle->s.origin[2] = tr.endpos[2] + fabs(vehicle->mins[2]) + 2;
		else
		{
			vehicle->ONOFF &= ~LANDED;
			vehicle->ONOFF |= AUTOROTATION;
			vehicle->thrust = 0;
			vehicle->drone_decision_time = level.time + 2.5;
		}
		if( vehicle->deadflag )
		{
			vehicle->mins[2] = -4;
			vehicle->deadflag = DEAD_DEAD;
			vehicle->thrust = 0;
			vehicle->actualspeed = 0;
			vehicle->nextthink = level.time + 0.1;
			return;
		}
		vehicle->avelocity[1] = vehicle->aileron;
		AngleVectors( vehicle->s.angles, forward, NULL, NULL );
		VectorScale( forward, vehicle->actualspeed/4, vehicle->velocity );
		VectorNormalize( forward );
		VectorScale( forward, 20, forward );
		VectorAdd( vehicle->s.origin, forward, up );
		tr = gi.trace( vehicle->s.origin, NULL, NULL, up, vehicle, MASK_ALL );
		if( tr.fraction < 1 )
		{
			if( ( strcmp( tr.ent->classname, "plane" ) == 0 ||
				strcmp( tr.ent->classname, "helo" ) == 0 ||
				strcmp( tr.ent->classname, "ground" ) == 0 )
				&& !(tr.ent->ONOFF & PILOT_ONBOARD) )
			{
				vehicle->activator = tr.ent;
			}
		}
		else
		{
			vehicle->activator = NULL;
		}
	}
	vehicle->nextthink = level.time + 0.1;
	vehicle->radio_target = NULL;
	vehicle->ONOFF &= ~IS_CHANGEABLE;
}

void LQM_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( !(self->ONOFF & LANDED) && !(self->ONOFF & AUTOROTATION) )	// open chute
	{
		self->s.angles[0] = self->s.angles[2] = 0;
		gi.cprintf( self->owner, PRINT_MEDIUM, "You landed.\n" );
		self->s.frame = 1;
		self->s.modelindex2 = gi.modelindex( "vehicles/infantry/pilot/mp5.md2" );
		self->ONOFF |= LANDED;
		self->s.origin[2] += 2;
	}
	else if( !(self->ONOFF & LANDED) && self->ONOFF & AUTOROTATION )
	{
		gi.bprintf( PRINT_MEDIUM, "%s fell to his death.\n", self->owner->client->pers.netname );
		self->deadflag = DEAD_DEAD;
	}
}

void LQM_pain (edict_t *self, edict_t *other, float kick, int damage)
{

}

void LQM_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if( self->ONOFF & LANDED )
	{
		self->deadflag = DEAD_DEAD;
		SetVehicleModel( self );
		self->deathtime = level.time + 1.0;
	}
	else
	{
		self->deadflag = DEAD_DYING;
	}
}


