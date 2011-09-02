// handles helicopters
// by Bjoern Drabeck


#include "g_local.h"
#include "v_helo.h"

void HeloSound( edict_t *vehicle )
{
	float volume;

	if( !vehicle->temptime )
		vehicle->temptime = level.time;

	if( level.time > vehicle->temptime && vehicle->ONOFF & PILOT_ONBOARD )
	{
		volume = vehicle->thrust * 0.1 + 0.3;

		if( vehicle->thrust > THRUST_0 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/chopper.wav" ), volume, ATTN_NORM, 0 );
		else
		{
//			if( !(vehicle->ONOFF & LANDED) )	// later change for unmanned helo!
				gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/chopper.wav" ), volume, ATTN_NORM, 0 );
		}
		vehicle->temptime = level.time + 0.2;
	}

}

void SetHeloSpeed( edict_t *vehicle )
{
	float	tgtspdside;
	vec3_t	forward,right,up;
	int		VM = vehicle->owner->client->pers.videomodefix;

//	gi.bprintf( PRINT_HIGH, "old:%.1f new:%.1f\n", vehicle->tempangle, vehicle->s.angles[0] );

	if( vehicle->deadflag == DEAD_DEAD )
	{
		vehicle->actualspeed = 0;
		return;
	}

	// calc speeds
	if( !(vehicle->ONOFF & AUTOROTATION ) )
	{
//		gi.bprintf( PRINT_HIGH, "STANDARD\n" );// debugging
		if( !(vehicle->ONOFF & LANDED) )
		{
			switch( vehicle->thrust )
			{
				case 0:
						vehicle->tempspeed = -150;
						break;
				case 1:
						vehicle->tempspeed = -50;
						break;
				case 2:
						vehicle->tempspeed = 0;
						break;
				case 3:
						vehicle->tempspeed = 50;
						break;
				case 4:
						vehicle->tempspeed = 120;
						break;
			}
		}
		else
		{
			if( vehicle->thrust == THRUST_75 )
				vehicle->tempspeed = 50;
			else if( vehicle->thrust == THRUST_100 )
				vehicle->tempspeed = 120;
			else
				vehicle->tempspeed = 0;
		}
	}
	// auto-rotation
	if( vehicle->elevator < 0 && vehicle->tempangle > vehicle->s.angles[0] && vehicle->thrust < THRUST_25 )
	{
		vehicle->tempspeed += (int)(15*(vehicle->tempangle-vehicle->s.angles[0]));
//		gi.bprintf( PRINT_HIGH, "autorotation\n" );// debugging
		vehicle->ONOFF |= AUTOROTATION;
		if( vehicle->tempspeed > 30 )
			vehicle->tempspeed = 30;
	}
	else
		vehicle->ONOFF &= ~AUTOROTATION;

	// in-ground-effect
/*	if( !(vehicle->ONOFF & AUTOROTATION ) && vehicle->altitude < 50 )
	{
		vehicle->tempspeed +=	(70-vehicle->altitude);
	}
*/

	if( vehicle->s.angles[0] > 5 )
		vehicle->actualspeed = (vehicle->topspeed/35)*(vehicle->s.angles[0]-5);
	else if( vehicle->s.angles[0] < -5 )
		vehicle->actualspeed = (vehicle->topspeed/45)*(vehicle->s.angles[0]+5);
	else
		vehicle->actualspeed = 0;


	if( vehicle->s.angles[2] > 5 )
		tgtspdside = -5 * (fabs(vehicle->s.angles[2])-5)*VM;
	else if( vehicle->s.angles[2] < -5 )
		tgtspdside = 5 * (fabs(vehicle->s.angles[2])-5)*VM;
	else
		tgtspdside = 0;

	// NOTE: vehicle->currentop used for vertical speed!

	if( vehicle->tempspeed > vehicle->currenttop )
	{
		if( vehicle->thrust > THRUST_75 )
			vehicle->currenttop += 10;
		else if( vehicle->ONOFF & AUTOROTATION )
			vehicle->currenttop += 10;
		else
			vehicle->currenttop += 5;
	}
	else if( vehicle->tempspeed < vehicle->currenttop )
	{
		if( vehicle->thrust < THRUST_25 )
			vehicle->currenttop -= 10;
		else
			vehicle->currenttop -= 5;
	}

	// give speed
	if( !(vehicle->ONOFF & LANDED) )
	{
		AngleVectors( vehicle->s.angles, forward, right, up );
		forward[2] = 0;	// more realistic: forward[2] /= 3;
		VectorNormalize( forward );
		right[2] = 0;
		VectorNormalize( right );
		VectorScale( forward, vehicle->actualspeed/4, forward );
		VectorScale( up, vehicle->currenttop, up );
		up[0] = up[1] = 0;
		VectorScale( right, tgtspdside, right );
		VectorAdd( forward, right, vehicle->velocity );
		if( VectorLength(vehicle->velocity) > vehicle->topspeed/4 )
		{
			VectorNormalize( vehicle->velocity );
			VectorScale( vehicle->velocity, vehicle->topspeed/4, vehicle->velocity );
		}
		vehicle->actualspeed = VectorLength( vehicle->velocity )*4;
		if( vehicle->s.angles[0] < 0 )
			vehicle->actualspeed *= -1;
		VectorAdd( vehicle->velocity, up, vehicle->velocity );
	}
	else
	{
		if( vehicle->thrust > THRUST_75 )
		{
			AngleVectors( vehicle->s.angles, forward, right, up );
			vehicle->actualspeed = VectorLength( vehicle->velocity )*4;
			VectorAdd( vehicle->velocity, up, vehicle->velocity );
		}
		else
		{
			vehicle->actualspeed = 0;
			VectorScale( vehicle->velocity, vehicle->actualspeed, vehicle->velocity );
		}
	}

//	gi.bprintf( PRINT_HIGH, "%.2f\n", VectorLength(vehicle->velocity) );

	vehicle->tempangle = vehicle->s.angles[0];

	HeloSound( vehicle );
}

void SteerHelo( edict_t *vehicle)
{
	vec3_t 	below, above;
	trace_t	trace;
	int		VM = vehicle->owner->client->pers.videomodefix;
	int		bay = 0;

	VectorClear( vehicle->avelocity );
	VectorClear( vehicle->velocity );

	if( vehicle->ONOFF & AUTOPILOT )
	{
		vehicle->elevator = vehicle->aileron = vehicle->rudder = 0;
	}
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
	// health-effects
	if( vehicle->owner->health >= 26 )
		vehicle->s.effects = 0;
	else if( vehicle->owner->health < 26 && vehicle->owner->health > 10 )
	{
		vehicle->s.effects = EF_GRENADE;
	}
	else if( vehicle->owner->health <= 10 && vehicle->deadflag != DEAD_DEAD )
	{
		vehicle->s.effects = EF_ROCKET;
	}
	else
		vehicle->s.effects = 0;

	// dead
	if( vehicle->deadflag )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		vehicle->s.frame = 0;
		vehicle->s.renderfx = 0;
		vehicle->avelocity[2] = 120;
		vehicle->gravity = 0.3;
		vehicle->movetype = MOVETYPE_TOSS;
		return;
	}

	// overturning ?
	vehicle->s.angles[1] = anglemod( vehicle->s.angles[1] );

	// frames
	vehicle->s.renderfx = RF_TRANSLUCENT;
	if( vehicle->ONOFF & BAY_OPEN )
		bay = 1;
	if( !(vehicle->ONOFF & LANDED ) ) // not landed
	{
		if( vehicle->ONOFF & GEARDOWN )
		{
			if( vehicle->s.frame < 16 + 16 * bay )
				vehicle->s.frame ++;
			else
				vehicle->s.frame = 9 +  16 * bay;
		}
		else
		{
			if( vehicle->s.frame < 8 +  16 * bay )
				vehicle->s.frame ++;
			else
				vehicle->s.frame = 1 +  16 * bay;
		}
	}
	else		// landed
	{
		if( vehicle->ONOFF & PILOT_ONBOARD )
		{
			if( vehicle->s.frame < 16 + 16 * bay )
				vehicle->s.frame ++;
			else
				vehicle->s.frame = 9 + 16 * bay;
		}
		else
		{
			vehicle->s.renderfx = 0;
			vehicle->s.frame = 0;
		}
	}

	// speed
	SetHeloSpeed( vehicle );

	// control input
	if( !(vehicle->ONOFF & LANDED) )
	{
		// rudder
		if( vehicle->rudder )
			vehicle->avelocity[1] = vehicle->rudder;

		// elevator
		if( vehicle->elevator )
		{
			if( vehicle->elevator > 0 )
			{
				if( vehicle->s.angles[0] < 55 && vehicle->s.angles[0] > -80 )
					vehicle->avelocity[0] = vehicle->elevator;
			}
			else if( vehicle->elevator < 0 )
			{
				if( vehicle->s.angles[0] > -35 && vehicle->s.angles[0] < 80 )
					vehicle->avelocity[0] = vehicle->elevator;
			}
		}

		if( vehicle->s.angles[0] > 55 )
			vehicle->s.angles[0] = 55;
		else if( vehicle->s.angles[0] < -35 )
			vehicle->s.angles[0] = -35;

		// aileron
		if( vehicle->aileron )
		{
			if( vehicle->aileron > 0 )
			{
				if( vehicle->s.angles[2]*VM < 30 && vehicle->s.angles[2]*VM > -60 )
					vehicle->avelocity[2] = vehicle->aileron*VM;
			}
			else if( vehicle->aileron < 0 )
			{
				if( vehicle->s.angles[2]*VM > -30 && vehicle->s.angles[2]*VM < 60 )
					vehicle->avelocity[2] = vehicle->aileron*VM;
			}
		}
		else
		{
			if( vehicle->s.angles[2] > 3 && vehicle->s.angles[2] < 90 )
				vehicle->avelocity[2] = -40;
			else if( vehicle->s.angles[2] < -3 && vehicle->s.angles[2] > -90 )
				vehicle->avelocity[2] = 40;
		}

		// yaw when banked at speed
		if( fabs(vehicle->s.angles[2]) > 5 && fabs(vehicle->s.angles[0]) > 12 )
		{
			vehicle->avelocity[1] += (vehicle->s.angles[2]*VM);
		}

		// auto-coord
/*		if( vehicle->owner->client->pers.autocoord )
		{
			vehicle->avelocity[1] += vehicle->aileron;
		}
*/
		// auto-roll-back
		if( (vehicle->owner->client->pers.autoroll && !vehicle->elevator)
				 || ( vehicle->ONOFF & AUTOPILOT ) )
		{
			if( vehicle->s.angles[0] > 2 && vehicle->s.angles[0] < 90 )
				vehicle->avelocity[0] = -20;
			else if( vehicle->s.angles[0] < -2 && vehicle->s.angles[0] > -90 )
				vehicle->avelocity[0] = 20;
		}
	}

	// did we touch the sky?
	VectorCopy( vehicle->s.origin, above );
	above[2] += 64;
	trace = gi.trace( vehicle->s.origin, NULL, NULL, above, vehicle, MASK_ALL );
	if( trace.fraction < 1.0 && ( trace.surface->flags & SURF_SKY ||
  		strncmp(trace.surface->name,"clip",strlen(trace.surface->name))== 0 ) )
	{
		if( vehicle->velocity[2] > 0 )
		{
			vehicle->velocity[2] = 0;
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
//		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
//			5, vehicle->s.origin );
//		ThrowDebris( vehicle, "models/objects/debris3/tris.md2",
//			5, vehicle->s.origin );
		ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
			5, vehicle->s.origin );
		VehicleExplosion( vehicle );
		SetVehicleModel( vehicle );
		if( teamplay->value == 1 )
			gi.bprintf( PRINT_MEDIUM, "[%s]%s splashed.",
				team[vehicle->owner->client->pers.team-1].name,
				vehicle->owner->client->pers.netname);
		else
			gi.bprintf( PRINT_MEDIUM, "%s splashed.", vehicle->owner->client->pers.netname);
		meansOfDeath = MOD_PLANESPLASH;
	}

//	gi.bprintf( PRINT_HIGH, "%d\n", vehicle->elevator );//debugging
//	gi.bprintf( PRINT_HIGH, "%.0f\n", vehicle->velocity[2] );// debugging
//	gi.bprintf( PRINT_HIGH, "%.1f\n", VectorLength( vehicle->velocity ) );
//	gi.bprintf( PRINT_HIGH, "%d\n",(int)(random()*3+3) );

	// check for landing
	if( !(vehicle->ONOFF & LANDED ) )
	{
		VectorCopy( vehicle->s.origin, below );
		below[2] = vehicle->s.origin[2] + vehicle->mins[2] - 1.5;
		trace = gi.trace( vehicle->s.origin, NULL, NULL, below, vehicle, MASK_ALL );

		if( strcmp( trace.ent->classname,"func_runway") == 0
		|| ( trace.contents & CONTENTS_SOLID ) )
		{
			if( ( fabs(vehicle->actualspeed) < 200 ) && ( vehicle->velocity[2] >= -60 ) &&
				(vehicle->ONOFF & GEARDOWN) )
			{
				vehicle->ONOFF |= LANDED;
				vehicle->ONOFF &= ~AUTOPILOT;
				gi.cprintf( vehicle->owner, PRINT_HIGH, "You have landed.\n" );
				VectorClear( vehicle->velocity );
				VectorClear( vehicle->avelocity );
				if( vehicle->HASCAN & IS_TAILDRAGGER )
					vehicle->s.angles[0] = -5;
				else
					vehicle->s.angles[0] = 0;
				vehicle->s.angles[2] = 0;
//				vehicle->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2] + 12;
//				vehicle->s.origin[2] = trace.endpos[2] + vehicle->s.origin[2] - (abs(vehicle->mins[2])+2);
				vehicle->s.origin[2] =  trace.endpos[2] + abs(vehicle->mins[2]) + 2;
			}
			else
			{
//				gi.bprintf( PRINT_HIGH, "%.1f %.1f\n", vehicle->actualspeed, vehicle->velocity[2] );
				if( vehicle->ONOFF & GEARDOWN )
				{
					vehicle->ONOFF |= LANDED;
					if( fabs(vehicle->actualspeed) >= 200 )
						vehicle->health -= ((fabs(vehicle->actualspeed)-200)/4);
					if( vehicle->velocity[2] < -60 )
						vehicle->health += (vehicle->velocity[2]+60);
					vehicle->s.angles[0] = -5;
					vehicle->s.angles[2] = 0;
					vehicle->s.origin[2] =  trace.endpos[2] + abs(vehicle->mins[2]) + 2;
				}
				else
				{
//					vehicle->ONOFF |= LANDED;
					vehicle->health -= (fabs(vehicle->actualspeed)/8);
					vehicle->health += (vehicle->velocity[2])/2;
				}
				VectorClear( vehicle->avelocity );
				VectorClear( vehicle->velocity );
				MakeDamage( vehicle );
				gi.sound( vehicle, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );

				if( vehicle->health <= 0 )
				{
					if( vehicle->enemy )
					{
					vehicle->enemy->lockstatus = 0;
					vehicle->enemy = NULL;
					}
					gi.WriteByte (svc_temp_entity);
					gi.WriteByte (TE_EXPLOSION1);
					gi.WritePosition (vehicle->s.origin);
					gi.multicast (vehicle->s.origin, MULTICAST_PVS);
					ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
						5, vehicle->s.origin );
//					ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
//						5, vehicle->s.origin );
//					ThrowDebris( vehicle, "models/objects/debris3/tris.md2",
//						5, vehicle->s.origin );
					ThrowDebris( vehicle, "models/objects/debris2/tris.md2",
						5, vehicle->s.origin );
					VehicleExplosion( vehicle );
					SetVehicleModel( vehicle );
					if( !vehicle->deadflag )
						meansOfDeath = MOD_CRASH;
					vehicle->deadflag = DEAD_DEAD;
					gi.bprintf( PRINT_MEDIUM, "%s crashed.\n", vehicle->owner->client->pers.netname );
				}
			}
		}
	}
	else
	{
		if( vehicle->thrust > THRUST_50 )
		{
			vehicle->ONOFF ^= LANDED;
			gi.cprintf( vehicle->owner, PRINT_HIGH, "Airborne.\n" );
		}
		else
		{
			VectorClear( vehicle->velocity );
			VectorClear( vehicle->avelocity );
		}
		VectorCopy( vehicle->s.origin, below );
		below[2] =  vehicle->s.origin[2] + vehicle->mins[2] - 3;
		trace = gi.trace( vehicle->s.origin, NULL, NULL, below, vehicle, MASK_ALL );
		if( trace.fraction == 1 )
		{
			vehicle->ONOFF ^= LANDED;
		}

	}

	// sidewinders
	if( !vehicle->deadflag )
	{
		if( vehicle->owner->client->pers.secondary == FindItem( "AutoCannon" ) &&
				(vehicle->HASCAN & HAS_AUTOAIM) )
			AutoCannonLockOn( vehicle );
		else if( vehicle->owner->client->pers.secondary == FindItem( "SW Launcher" ) ||
				 vehicle->owner->client->pers.secondary == FindItem( "ST Launcher" ) ||
				 vehicle->owner->client->pers.secondary == FindItem( "AM Launcher" ) ||
				 vehicle->owner->client->pers.secondary == FindItem( "PH Launcher" ) )
			SidewinderLockOn( vehicle );
		else if( vehicle->owner->client->pers.secondary == FindItem( "ATGM Launcher" ) ||
				vehicle->owner->client->pers.secondary == FindItem ("Maverick Launcher") ||
				vehicle->owner->client->pers.secondary == FindItem ("Laser Bombs") )
			ATGMLockOn( vehicle );
		else if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
	}

//	gi.bprintf( PRINT_HIGH, "%d\n", vehicle->ONOFF );

	// altitude
	VectorCopy( vehicle->s.origin, below );
	below[2] -= 2000;
	trace = gi.trace( vehicle->s.origin, NULL, NULL, below, vehicle, MASK_SOLID );
	if( trace.fraction < 1 )
		vehicle->altitude = ((int)(vehicle->s.origin[2] - trace.endpos[2]))-4;
	else
		vehicle->altitude = 9999;

//	gi.bprintf( PRINT_HIGH, "%d\n", level.category );// debugging

//	gi.bprintf( PRINT_HIGH, "%f\n", random() );

	vehicle->radio_target = NULL;
	vehicle->ONOFF &= ~IS_CHANGEABLE;

	if( vehicle->ONOFF & BAY_OPEN && level.time > vehicle->timestamp )
		Cmd_OpenBay_f( vehicle->owner );

	if( vehicle->ONOFF & PICKLE_ON )
		Cmd_Pickle_f( vehicle->owner );

	vehicle->nextthink = level.time + 0.1;

	gi.linkentity( vehicle );

}



void helo_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
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
	}
}

