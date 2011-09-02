/************************************************/
/* Filename: aq2_plane.cpp					    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 1999-07-10							*/
/*												*/
/* Description:									*/
/* NewAirQuake2 plane-specific stuff     		*/
/*												*/
/************************************************/

#include "g_local.h"










void IsPlaneLanded( edict_t *plane )
{
	vec3_t	underneath;
	trace_t	trace;
	int		sub = 0;

	if( !(plane->ONOFF & ONOFF_AIRBORNE) )
		return;

	// check if to land the plane
	VectorSet( underneath, plane->s.origin[0], plane->s.origin[1],
					plane->s.origin[2] - (abs(plane->mins[2])+3) );
	trace = gi.trace( plane->s.origin, NULL, NULL, underneath, plane, MASK_ALL );

	if( strcmp( trace.ent->classname, "func_runway" ) == 0
		|| strcmp( trace.ent->classname, "func_door" ) == 0
		|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0 )
	{
		if(  ( ( plane->s.angles[0] >= 0 && plane->s.angles[0] <= 40
					&& fabs(plane->s.angles[2]) <= 30 )
			|| ( plane->s.angles[0] >= 140 && plane->s.angles[0] <= 180
					&& fabs(plane->s.angles[2]) >= 150 ) ) )
		{
			plane->groundentity = trace.ent;
			if( plane->ONOFF & ONOFF_GEAR )
			{
				plane->ONOFF &= ~(ONOFF_AIRBORNE | ONOFF_STALLED);
				plane->ONOFF |= ONOFF_LANDED;
				gi.sound( plane, CHAN_AUTO, gi.soundindex( "flying/touchdwn.wav" ), 1, ATTN_NORM, 0 );
				gi.cprintf( plane->owner, PRINT_MEDIUM, "LANDED!\n" );
			}
			else
			{
				plane->ONOFF &= ~(ONOFF_AIRBORNE | ONOFF_STALLED);
				plane->ONOFF |= ONOFF_LANDED_NOGEAR;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "You crash-landed!\n", fabs(plane->s.angles[0]) );
				sub = 3*( (fabs(plane->s.angles[0]) <= 30) ? (fabs(plane->s.angles[0])) :
							(180-fabs(plane->s.angles[0])) );
				gi.sound( plane, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
				plane->s.effects = EF_ROCKET;
				T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, sub, 0, 0, MOD_CRASHLAND );
			}

			plane->avelocity[0] = plane->avelocity[2] = 0;
			plane->velocity[2] = 0;
			if( plane->s.angles[0] >= 0 && plane->s.angles[0] <= 40 )
			{
				plane->s.angles[0] = plane->s.angles[2] = 0;
			}
			else
			{
				plane->s.angles[0] = plane->s.angles[2] = 180;
			}
			plane->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2] + abs(plane->mins[2])+2;
		}
		else if( plane->s.angles[0] > 180 && plane->s.angles[0] < 360 )
		{
			// nothing, just takeoff
			plane->ONOFF |= ONOFF_AIRBORNE;
		}	
	}
	else if( trace.contents & CONTENTS_SOLID )
	{
		if(  ( ( plane->s.angles[0] >= 0 && plane->s.angles[0] <= 40
					&& fabs(plane->s.angles[2]) <= 30 )
			|| ( plane->s.angles[0] >= 140 && plane->s.angles[0] <= 180
					&& fabs(plane->s.angles[2]) >= 150 ) ) )
		{
			plane->groundentity = trace.ent;
			if( plane->ONOFF & ONOFF_GEAR )
			{
				plane->ONOFF &= ~(ONOFF_AIRBORNE | ONOFF_STALLED);
				plane->ONOFF |= ONOFF_LANDED_TERRAIN;
				gi.sound( plane, CHAN_AUTO, gi.soundindex( "flying/touchdwn.wav" ), 1, ATTN_NORM, 0 );
				gi.cprintf( plane->owner, PRINT_MEDIUM, "LANDED!\n" );
			}
			else
			{
				plane->ONOFF &= ~(ONOFF_AIRBORNE | ONOFF_STALLED);
				plane->ONOFF |= ONOFF_LANDED_TERRAIN_NOGEAR;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "You crash-landed!\n" );
				sub = 3*( (fabs(plane->s.angles[0]) <= 30) ? (fabs(plane->s.angles[0])) :
							(180-fabs(plane->s.angles[0])) );
				gi.sound( plane, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
				plane->s.effects = EF_ROCKET;
				T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, sub, 0, 0, MOD_CRASHLAND );
			}

			plane->avelocity[0] = plane->avelocity[2] = 0;
			plane->velocity[2] = 0;
			if( plane->s.angles[0] >= 0 && plane->s.angles[0] <= 40 )
			{
				plane->s.angles[0] = plane->s.angles[2] = 0;
			}
			else
			{
				plane->s.angles[0] = plane->s.angles[2] = 180;
			}
			plane->s.origin[2] = trace.endpos[2] + abs(plane->mins[2])+2;
		}
		else if( plane->s.angles[0] > 180 && plane->s.angles[0] < 360 )
		{
			// nothing, just takeoff
			plane->ONOFF |= ONOFF_AIRBORNE;
		}
	}
	else if( trace.contents & CONTENTS_LIQUID )
	{
		if(  ( ( plane->s.angles[0] >= 0 && plane->s.angles[0] <= 40
					&& fabs(plane->s.angles[2]) <= 30 )
			|| ( plane->s.angles[0] >= 140 && plane->s.angles[0] <= 180
					&& fabs(plane->s.angles[2]) >= 150 ) ) )
		{
			plane->groundentity = trace.ent;
			plane->ONOFF &= ~(ONOFF_AIRBORNE | ONOFF_STALLED);
			gi.cprintf( plane->owner, PRINT_MEDIUM, "You splashed!\n" );
			if( plane->ONOFF & ONOFF_GEAR )
			{
				plane->ONOFF |= ONOFF_LANDED_WATER;
				sub = 2*( (fabs(plane->s.angles[0]) <= 30) ? (fabs(plane->s.angles[0])) :
						(180-fabs(plane->s.angles[0])) );
			}
			else
			{
				plane->ONOFF |= ONOFF_LANDED_WATER_NOGEAR;
				sub = ( (fabs(plane->s.angles[0]) <= 30) ? (fabs(plane->s.angles[0])) :
						(180-fabs(plane->s.angles[0])) );
			}
			gi.sound( plane, CHAN_AUTO, gi.soundindex( "player/watr_out.wav" ), 1, ATTN_NORM, 0 );
			plane->s.effects = EF_GRENADE;

			plane->avelocity[0] = plane->avelocity[2] = 0;
			plane->velocity[2] = 0;
			if( plane->s.angles[0] >= 0 && plane->s.angles[0] <= 40 )
			{
				plane->s.angles[0] = plane->s.angles[2] = 0;
			}
			else
			{
				plane->s.angles[0] = plane->s.angles[2] = 180;
			}
			plane->s.origin[2] = trace.endpos[2] + abs(plane->mins[2])+2;
			T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, sub, 0, 0, MOD_SPLASHLAND );
		}
		else if( plane->s.angles[0] > 180 && plane->s.angles[0] < 360 )
		{
			if( (plane->ONOFF & ONOFF_AIRBORNE) && plane->waterlevel )
			{
				T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 9999, 0, 0, MOD_SPLASHLAND );
			}
			else
			{
				// nothing, just takeoff
				plane->ONOFF |= ONOFF_AIRBORNE;
				plane->ONOFF &= ~(ONOFF_LANDED_WATER|ONOFF_LANDED_WATER_NOGEAR);
			}
		}
		else
		{
			T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, sub, 0, 0, MOD_SPLASHLAND );
		}
	}
}









void PlaneSound( edict_t *plane )
{
	float speedindex, volume;

	if( !plane->random )
	{
		plane->random = level.time;
		return;
	}

	if( level.time > plane->random && (plane->ONOFF & ONOFF_PILOT_ONBOARD) )
	{
		speedindex = floor( (plane->speed) / 35 );
		volume = plane->thrust * 0.05 + 0.3;

		if( volume > 1 )
			volume = 1;
		else if( volume < 0 )
			volume = 0;

//		gi.bprintf( PRINT_HIGH, "%f %f\n", speedindex, volume );

		if( plane->lockstatus )
		{
			if( plane->lockstatus == LS_LOCK )
				gi.sound( plane, CHAN_ITEM, gi.soundindex( "flying/in2.wav" ), 1.0, ATTN_STATIC, 0 );
			else
				gi.sound( plane, CHAN_ITEM, gi.soundindex( "flying/in1.wav" ), 1.0, ATTN_STATIC, 0 );
		}
		if( speedindex <= 0 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet0.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 1 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet1.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 2 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet2.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 3 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet3.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 4 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet4.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 5 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet5.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 6 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet6.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 7 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet7.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex >= 8 )
			gi.sound( plane, CHAN_BODY, gi.soundindex( "engines/jet8.wav" ), volume, ATTN_NORM, 0 );
		plane->random = level.time + 0.2;
	}

}








// this is for research only!
/*void PlaneMovement( edict_t *plane )
{
	plane->avelocity[0] = plane->elevator;
	plane->avelocity[1] = plane->rudder;
	plane->avelocity[2] = plane->aileron;

	gi.bprintf( PRINT_HIGH, "[0]:%.4f [1]:%.4f [2]:%.4f\n", plane->s.angles[0],
		plane->s.angles[1], plane->s.angles[2] );

	plane->nextthink = level.time + 0.1;
}*/









void PlaneMovement( edict_t *plane )
{
	int		VM = (plane->owner->client->FLAGS & CLIENT_NONGL_MODE) ? -1 : 1;
	double  sinroll, cosroll, sinpitch, cospitch;
	vec3_t	forward, above, targetangle;
	trace_t	trace;

//	gi.bprintf( PRINT_HIGH, "%d\n", plane->ONOFF );

	// check for overturning
	plane->s.angles[0] = anglemod( plane->s.angles[0] );
	plane->s.angles[1] = anglemod( plane->s.angles[1] );
	if( plane->s.angles[2] > 180 )
		plane->s.angles[2] -= 360;
	else if( plane->s.angles[2] < -180 )
		plane->s.angles[2] += 360;


	// health-effects
	if( plane->owner->health >= 26 )
		plane->s.effects = 0;
	if( plane->owner->health < 26 && plane->owner->health > 10 )
	{
		plane->s.effects = EF_GRENADE;
	}
	else if( plane->owner->health <= 10 && plane->deadflag != DEAD_DEAD )
	{
		plane->s.effects = EF_ROCKET;
	}
	else
		plane->s.effects = 0;

	// set speed
	PlaneSpeed( plane );

	// DEAD vehicles don't need to be steered!
	if( plane->deadflag == DEAD_DEAD )
	{
		if( plane->enemy )
		{
			plane->enemy->lockstatus = 0;
			plane->enemy = NULL;
		}
		return;
	}
	else if( plane->deadflag == DEAD_DYING )
	{
		plane->avelocity[2] = (plane->avelocity[2]>=0) ? 60 : -60;
		vectoangles( plane->velocity, targetangle );
		plane->s.angles[0] = targetangle[0];
		plane->s.angles[1] = targetangle[1];
		plane->gravity = 0.1;
		plane->nextthink = level.time + 0.1;
		if( plane->enemy )
		{
			plane->enemy->lockstatus = 0;
			plane->enemy = NULL;
		}
		return;
	}
	else if( plane->ONOFF & ONOFF_SINKING )
	{
		plane->gravity = 0.01;
		plane->nextthink = level.time + 0.1;
		if( plane->enemy )
		{
			plane->enemy->lockstatus = 0;
			plane->enemy = NULL;
		}
		return;
	}

	// cleanup
	VectorClear( plane->avelocity );

	// speed effects
	if( plane->speed > (plane->stallspeed + 100) && (plane->ONOFF & ONOFF_GEAR)
			&& (plane->ONOFF & ONOFF_AIRBORNE) && !(plane->DAMAGE & DAMAGE_GEAR))
	{
		plane->ONOFF ^= ONOFF_GEAR;
		plane->mins[2] += 4;
		gi.cprintf( plane->owner, PRINT_MEDIUM, "Gear retracted!\n" );
	}

	// frames
	if( plane->speed <= (plane->stallspeed + 100) && !(plane->ONOFF & ONOFF_GEAR)
			 && !(plane->ONOFF & ONOFF_BRAKE) )
		plane->s.frame = 0;
	else if( plane->speed > (plane->stallspeed + 100) && !(plane->ONOFF & ONOFF_GEAR)
	 			 && !(plane->ONOFF & ONOFF_BRAKE) )
		plane->s.frame = 1;
	else if( plane->speed <= (plane->stallspeed + 100) && (plane->ONOFF & ONOFF_GEAR)
	 			 && (plane->ONOFF & ONOFF_BRAKE ) )
		plane->s.frame = 2;
	else if( plane->speed <= (plane->stallspeed + 100) && !(plane->ONOFF & ONOFF_GEAR)
			 && (plane->ONOFF & ONOFF_BRAKE ) )
		plane->s.frame = 3;
	else if( plane->speed <= (plane->stallspeed + 100) && (plane->ONOFF & ONOFF_GEAR)
	  			 && !(plane->ONOFF & ONOFF_BRAKE ) )
		plane->s.frame = 4;
	else if( plane->speed > (plane->stallspeed + 100) && !(plane->ONOFF & ONOFF_GEAR)
	  			 && (plane->ONOFF & ONOFF_BRAKE ) )
		plane->s.frame = 5;
	if( plane->ONOFF & ONOFF_WEAPONBAY )
		plane->s.frame += 6;

	// skins
	if( plane->thrust > THRUST_MILITARY )
		plane->s.skinnum = 1;
	else
		plane->s.skinnum = 0;

	plane->elevator *= (plane->turnspeed[0]/400);
	plane->aileron *= (plane->turnspeed[2]/400);
	plane->rudder *= (plane->turnspeed[1]/400);

	// speed dependent maneuverability for planes
	if( plane->speed > plane->stallspeed )
	{
		plane->elevator *= (float)(plane->stallspeed/(plane->speed*1.2));
		if( plane->ONOFF & ONOFF_AIRBORNE )
			plane->rudder *= (float)(plane->stallspeed/plane->speed);
	}

	// movement in the air
	if( plane->ONOFF & ONOFF_AIRBORNE )
	{
		if( plane->HASCAN & HASCAN_WEAPONBAY )
		{
			if( level.time > plane->wait && !(plane->DAMAGE & DAMAGE_WEAPONBAY) &&
				(plane->ONOFF & ONOFF_WEAPONBAY) )
			{
				plane->ONOFF &= ~ONOFF_WEAPONBAY;
			}
		}

		// stalled
		if( plane->ONOFF & ONOFF_STALLED )
		{
			if( plane->speed < plane->stallspeed )// stay stalled
			{
				if( plane->s.angles[0] < 70 || plane->s.angles[0] > 110 )
				{
					if( plane->s.angles[0] >= 270 || plane->s.angles[0] <= 70 )
						plane->avelocity[0] = plane->turnspeed[0]/2;
					else
						plane->avelocity[0] = -plane->turnspeed[0]/2;
				}
				if( fabs(plane->s.angles[2]) > 20 )
					plane->avelocity[1] = plane->turnspeed[1];
				else
					plane->avelocity[1] = 0;
			}
			else // end stall
			{
					plane->ONOFF ^= ONOFF_STALLED;
			}
			if( plane->s.angles[0] > 70 && plane->s.angles[0] < 110 )
				plane->avelocity[0] = 0;
		}
		// normal movement
		else
		{
			// angluar corrections
			sinpitch = sin((plane->s.angles[0]*M_PI)/180.0);
			cospitch = cos((plane->s.angles[0]*M_PI)/180.0);
			if( plane->s.angles[0] < 90 || plane->s.angles[0] >= 270 )
			{
				sinroll = -sin((plane->s.angles[2]*M_PI*VM)/180.0);
				cosroll = cos((plane->s.angles[2]*M_PI*VM)/180.0);
			}
			else
			{
				sinroll = -sin((-plane->s.angles[2]*M_PI*VM)/180.0);
				cosroll = cos((-plane->s.angles[2]*M_PI*VM)/180.0);
			}
			// elevator use
			if( !(plane->owner->client->FLAGS & CLIENT_COORD) )
			{
				plane->avelocity[0] = plane->elevator*cosroll;
				plane->avelocity[1] = plane->elevator*sinroll;
			}
			else
			{
				if( plane->elevator )
				{
					plane->avelocity[0] = plane->elevator*cosroll;
					plane->avelocity[1] = plane->elevator*sinroll;
				}
				if( fabs(plane->s.angles[2]) > 5 )
				{
					if( !(plane->owner->client->FLAGS & CLIENT_AUTOROLL) )
						plane->avelocity[1] += (-plane->turnspeed[1]*sinroll);
					else if( (plane->owner->client->FLAGS & CLIENT_AUTOROLL) && plane->aileron )
						plane->avelocity[1] += (-plane->turnspeed[1]*sinroll);
				}
			}
			// rudder use
			if( plane->s.angles[0] < 90 || plane->s.angles[0] >= 270 )
			{
				plane->avelocity[1] += plane->rudder*cosroll;
				plane->avelocity[0] -= plane->rudder*sinroll;
			}
			else
			{
				plane->avelocity[1] -= plane->rudder*cosroll;
				plane->avelocity[0] += plane->rudder*sinroll;
			}
			// aileron use
//			if( plane->s.angles[0] >= 270 || plane->s.angles[0] < 90 )
			{
				plane->avelocity[2] += plane->aileron*VM;//*cospitch;
//				plane->avelocity[1] += plane->aileron*sinpitch;
			}
//			else
			{
//				plane->avelocity[2] -= plane->aileron*VM;//*cospitch;
//				plane->avelocity[1] += plane->aileron*sinpitch;
			}
			// stall when too slow
			if( plane->speed < plane->stallspeed )
				plane->ONOFF |= ONOFF_STALLED;
			// auto-roll-back!
			if( (!plane->aileron && (plane->owner->client->FLAGS & CLIENT_AUTOROLL) && !plane->elevator
					&& !(plane->rudder) ) )
			{
				if( ( plane->s.angles[0] < 90 || plane->s.angles[0] >= 270 ) )
				{
					if( fabs( plane->s.angles[2] ) > 10 )
					{
						if( plane->s.angles[2]*VM > 10 )
							plane->avelocity[2] = -plane->turnspeed[2]/2*VM;
						else if( plane->s.angles[2]*VM < -10 )
							plane->avelocity[2] = plane->turnspeed[2]/2*VM;
					}
					else if( fabs( plane->s.angles[2] ) > 5 )
					{
						if( plane->s.angles[2]*VM > 5 )
							plane->avelocity[2] = -plane->turnspeed[2]/3*VM;
						else if( plane->s.angles[2]*VM < -5 )
							plane->avelocity[2] = plane->turnspeed[2]/3*VM;
					}
					else
						plane->s.angles[2] = 0;
				}
				else if( plane->s.angles[0] >= 90 && plane->s.angles[0] < 270 )
				{
					if( fabs( plane->s.angles[2] ) < 170 )
					{
						if( plane->s.angles[2]*VM < 170 && plane->s.angles[2]*VM > 0 )
							plane->avelocity[2] = plane->turnspeed[2]/2*VM;
						else if( plane->s.angles[2]*VM > -170 )
							plane->avelocity[2] = -plane->turnspeed[2]/2*VM;
					}
					else if( fabs( plane->s.angles[2] ) < 175 )
					{
						if( plane->s.angles[2]*VM < 175 && plane->s.angles[2]*VM > 0 )
							plane->avelocity[2] = plane->turnspeed[2]/3*VM;
						else if( plane->s.angles[2]*VM > -175 )
							plane->avelocity[2] = -plane->turnspeed[2]/3*VM;
					}
					else
						plane->s.angles[2]= 180;
				}
			}
			// did we touch the sky?
			VectorCopy( plane->s.origin, above );
			above[2] += 64;
			trace = gi.trace( plane->s.origin, NULL, NULL, above, plane, MASK_ALL );
			if( trace.fraction < 1.0 && (trace.surface->flags & SURF_SKY ||
		   		strncmp(trace.surface->name,"clip",strlen(trace.surface->name))== 0) )
			{
				if( plane->s.angles[0] > 180 && plane->s.angles[0] < 360 )
				{
					if( plane->s.angles[0] < 270 )
						plane->s.angles[0] = 180;
					else
						plane->s.angles[0] = 0;
				}
			}		
			// get altitude and check whether to land the plane
			VectorCopy( plane->s.origin, above );
			above[2] -= 2000;
			trace = gi.trace( plane->s.origin, NULL, NULL, above, plane, MASK_SOLID );
			if( trace.fraction < 1 )
				plane->altitude = ((int)(plane->s.origin[2] - trace.endpos[2]))-12;
			else
				plane->altitude = 9999;
			if( plane->altitude < 80 )
				IsPlaneLanded( plane );
		}
		if( plane->waterlevel )
		{
			VectorClear( plane->velocity );
			T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 9999, 0, 0, MOD_SPLASH );
		}
	}
	// and movement on the ground
	else
	{
		VectorSet( above, plane->s.origin[0], plane->s.origin[1],
						plane->s.origin[2] - 32 );

		trace = gi.trace( plane->s.origin, NULL, NULL, above, plane, MASK_ALL );

		if( plane->ONOFF & ONOFF_WEAPONBAY )
			plane->ONOFF &= ~ONOFF_WEAPONBAY;

		// are you standing on a runway ?
		if( strcmp( trace.ent->classname, "func_runway" ) == 0
		|| strcmp( trace.ent->classname, "func_door" ) == 0
		|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0 )
		{
			plane->groundentity = trace.ent;
			if( plane->ONOFF & ONOFF_LANDED_TERRAIN )
			{
				plane->ONOFF |= ONOFF_LANDED;
				plane->ONOFF &= ~ONOFF_LANDED_TERRAIN;
			}
			else if( plane->ONOFF & ONOFF_LANDED_TERRAIN_NOGEAR )
			{
				plane->ONOFF |= ONOFF_LANDED_NOGEAR;
				plane->ONOFF &= ~ONOFF_LANDED_TERRAIN_NOGEAR;
			}

			if( plane->elevator < 0 && plane->speed >= plane->stallspeed )
			{
				if( plane->s.angles[0] < 10 )
					plane->avelocity[0] = plane->elevator;
				else
					plane->avelocity[0] = -plane->elevator;
				plane->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN 
						| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR);
				plane->ONOFF |= ONOFF_AIRBORNE;
				plane->groundentity = NULL;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
			else
			{
				plane->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2]
			   			 + abs(plane->mins[2])+2;
//				if( !(plane->ONOFF & ONOFF_GEAR) )
//					plane->s.origin[2] -= 4;
			}
			if( (plane->rudder || plane->aileron) && ( plane->ONOFF & ONOFF_LANDED ) )
			{
				if( plane->speed )
				{
					if( plane->rudder )
						plane->avelocity[1] = 2*plane->rudder;
					else if( plane->aileron )
					{
						plane->avelocity[1] = plane->aileron;
						if( plane->avelocity[1] > 0 &&
								plane->avelocity[1] > 2*plane->turnspeed[1] )
							plane->avelocity[1] = 2*plane->turnspeed[1];
						else if( plane->avelocity[1] < 0 &&
								plane->avelocity[1] < -2*plane->turnspeed[1] )
							plane->avelocity[1] = -2*plane->turnspeed[1];
					}
				}
			}
			// crashlanded planes
			if( (plane->ONOFF & ONOFF_LANDED_NOGEAR) && plane->speed < 2 )
			{
				if( plane->s.angles[2] < 15 && (plane->HASCAN & HASCAN_ROUNDBODY) )
					plane->avelocity[2] = 10;
				else
				{
					VectorClear( plane->velocity );
					plane->avelocity[2] = 0;
					plane->think = EmptyPlaneDie;
					plane->nextthink = level.time + 10;
					return;
				}
			}
		}
		// moving on terrain
		else if( trace.contents & CONTENTS_SOLID )
		{
			plane->groundentity = trace.ent;
			if( plane->ONOFF & ONOFF_LANDED )
			{
				plane->ONOFF |= ONOFF_LANDED_TERRAIN;
				plane->ONOFF &= ~ONOFF_LANDED;
			}
			else if( plane->ONOFF & ONOFF_LANDED_NOGEAR )
			{
				plane->ONOFF |= ONOFF_LANDED_TERRAIN_NOGEAR;
				plane->ONOFF &= ~ONOFF_LANDED_NOGEAR;
			}

			if( plane->elevator < 0 && plane->speed >= plane->stallspeed )
			{
				if( plane->s.angles[0] < 10 )
					plane->avelocity[0] = plane->elevator;
				else
					plane->avelocity[0] = -plane->elevator;
				plane->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN 
						| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR);
				plane->ONOFF |= ONOFF_AIRBORNE;
				plane->groundentity = NULL;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
			else
			{
				plane->s.origin[2] = trace.endpos[2] + abs(plane->mins[2])+2;
//				if( !(plane->ONOFF & ONOFF_GEAR) )
//					plane->s.origin[2] -= 4;
			}
			if( (plane->rudder || plane->aileron) && ( (plane->ONOFF & ONOFF_LANDED_TERRAIN) 
					&& plane->ONOFF & ONOFF_GEAR ) )
			{
				if( plane->speed )
				{
					if( plane->rudder )
						plane->avelocity[1] = plane->rudder;
					else if( plane->aileron )
					{
						plane->avelocity[1] = plane->aileron;
						if( plane->avelocity[1] > 0 &&
								plane->avelocity[1] > plane->turnspeed[1] )
							plane->avelocity[1] = plane->turnspeed[1];
						else if( plane->avelocity[1] < 0 &&
								plane->avelocity[1] < -plane->turnspeed[1] )
							plane->avelocity[1] = -plane->turnspeed[1];
					}
				}
			}
			// crashlanded planes
			if( (plane->ONOFF & ONOFF_LANDED_TERRAIN_NOGEAR) && plane->speed < 2 )
			{
				if( plane->s.angles[2] < 15 && (plane->HASCAN & HASCAN_ROUNDBODY) )
					plane->avelocity[2] = 10;
				else
				{
					VectorClear( plane->velocity );
					plane->avelocity[2] = 0;
					plane->think = EmptyPlaneDie;
					plane->nextthink = level.time + 10;
					return;
				}
			}
		}
		// moving on water
		else if( trace.contents & CONTENTS_LIQUID )
		{
//			gi.bprintf(PRINT_HIGH, "on water\n" );
			plane->groundentity = trace.ent;
			if( plane->elevator < 0 && plane->speed >= plane->stallspeed )
			{
				if( plane->s.angles[0] < 10 )
					plane->avelocity[0] = plane->elevator;
				else
					plane->avelocity[0] = -plane->elevator;
				plane->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN 
						| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR);
				plane->ONOFF |= ONOFF_AIRBORNE;
				plane->groundentity = NULL;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
			else
			{
				if( plane->speed > 5 )
				{
					plane->s.origin[2] = trace.endpos[2] + abs(plane->mins[2])+2;
				}
				else	// sinking
				{
					plane->ONOFF |= ONOFF_SINKING;
					plane->DAMAGE |= DAMAGE_ENGINE_DESTROYED | DAMAGE_ENGINE_20 | DAMAGE_ENGINE_40 |
							DAMAGE_ENGINE_60 | DAMAGE_ENGINE_80 | DAMAGE_AFTERBURNER;
					plane->thrust = THRUST_IDLE;
				}
			}
		}
		// if not then you stall
		else
		{
			plane->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN | ONOFF_LANDED_WATER_NOGEAR
						| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR | ONOFF_LANDED_WATER);
			plane->ONOFF |= ONOFF_AIRBORNE;
			if( plane->speed < plane->stallspeed )
			{
				plane->ONOFF |= ONOFF_STALLED;
			}
			else
			{
				plane->groundentity = NULL;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
		}
	}

	// make velocity
	AngleVectors( plane->s.angles, forward, NULL, NULL );
	VectorScale( forward, plane->speed, plane->velocity );

	// add down velocity when slow
//	if( (plane->ONOFF & ONOFF_AIRBORNE) && plane->speed < plane->stallspeed * 1.5 )
	{
//		plane->velocity[2] -= ((plane->stallspeed * 1.5) - plane->speed );
	}

	// lock weapons
	if( plane->weapon[plane->active_weapon] == WEAPON_SIDEWINDER ||
		plane->weapon[plane->active_weapon] == WEAPON_STINGER ||
		plane->weapon[plane->active_weapon] == WEAPON_AMRAAM ||
		plane->weapon[plane->active_weapon] == WEAPON_PHOENIX )
		AAM_Lock( plane );
	else if( plane->weapon[plane->active_weapon] == WEAPON_HELLFIRE ||
		plane->weapon[plane->active_weapon] == WEAPON_MAVERICK ||
		plane->weapon[plane->active_weapon] == WEAPON_ANTIRADAR ||
		plane->weapon[plane->active_weapon] == WEAPON_RUNWAYBOMBS ||
		plane->weapon[plane->active_weapon] == WEAPON_LASERBOMBS )
		ATGM_Lock( plane );
	else if( plane->weapon[plane->active_weapon] == WEAPON_AUTOCANNON )
		Autocannon_Lock( plane );
	else if( plane->weapon[plane->active_weapon] == WEAPON_JAMMER )
	{
		if( plane->ONOFF & ONOFF_JAMMER )
			Jammer_Active( plane );
		else
			Reload_Jammer( plane );
		if( plane->enemy )
		{
			plane->enemy->lockstatus = 0;
			plane->enemy = NULL;
		}
	}
	else if( plane->enemy )
	{
		plane->enemy->lockstatus = 0;
		plane->enemy = NULL;
	}

	// fire autocannon
	if( plane->owner->client->buttons & BUTTON_ATTACK )
		Weapon_Autocannon_Fire( plane );

	if( (plane->HASCAN & HASCAN_STEALTH) &&
		!(plane->ONOFF & ONOFF_WEAPONBAY) && 
		!(plane->ONOFF & ONOFF_GEAR) )
		plane->ONOFF |= ONOFF_STEALTH;
	else
		plane->ONOFF &= ~ONOFF_STEALTH;

//	gi.bprintf( PRINT_HIGH, "%s\n", (plane->ONOFF & ONOFF_STEALTH)?"Stealthy!":"Visible" );

	// resetting stuff
	plane->radio_target = NULL;
	plane->ONOFF &= ~ONOFF_IS_CHANGEABLE;
	plane->teammaster = NULL;

	plane->nextthink = level.time + 0.1;
}


void PlaneMovement_Easy( edict_t *plane )
{
	int		VM = (plane->owner->client->FLAGS & CLIENT_NONGL_MODE) ? -1 : 1;
	double  sinroll;//, cosroll, sinpitch, cospitch;
	vec3_t	forward, above, targetangle;
	trace_t	trace;

//	gi.bprintf( PRINT_HIGH, "%d\n", plane->ONOFF );

	// check for overturning
	plane->s.angles[0] = anglemod( plane->s.angles[0] );
	plane->s.angles[1] = anglemod( plane->s.angles[1] );
	if( plane->s.angles[2] > 180 )
		plane->s.angles[2] -= 360;
	else if( plane->s.angles[2] < -180 )
		plane->s.angles[2] += 360;


	// health-effects
	if( plane->owner->health >= 26 )
		plane->s.effects = 0;
	if( plane->owner->health < 26 && plane->owner->health > 10 )
	{
		plane->s.effects = EF_GRENADE;
	}
	else if( plane->owner->health <= 10 && plane->deadflag != DEAD_DEAD )
	{
		plane->s.effects = EF_ROCKET;
	}
	else
		plane->s.effects = 0;

	// set speed
	PlaneSpeed( plane );

	// DEAD vehicles don't need to be steered!
	if( plane->deadflag == DEAD_DEAD )
	{
		if( plane->enemy )
		{
			plane->enemy->lockstatus = 0;
			plane->enemy = NULL;
		}
		return;
	}
	else if( plane->deadflag == DEAD_DYING )
	{
		if( plane->enemy )
		{
			plane->enemy->lockstatus = 0;
			plane->enemy = NULL;
		}
		plane->avelocity[2] = (plane->avelocity[2]>=0) ? 60 : -60;
		vectoangles( plane->velocity, targetangle );
		plane->s.angles[0] = targetangle[0];
		plane->s.angles[1] = targetangle[1];
		plane->gravity = 0.1;
		plane->nextthink = level.time + 0.1;
		return;
	}
	else if( plane->ONOFF & ONOFF_SINKING )
	{
		if( plane->enemy )
		{
			plane->enemy->lockstatus = 0;
			plane->enemy = NULL;
		}
		plane->gravity = 0.01;
		plane->nextthink = level.time + 0.1;
		return;
	}

	// cleanup
	VectorClear( plane->avelocity );

	// speed effects
	if( plane->speed > (plane->stallspeed + 100) && (plane->ONOFF & ONOFF_GEAR)
			&& (plane->ONOFF & ONOFF_AIRBORNE) && !(plane->DAMAGE & DAMAGE_GEAR))
	{
		plane->ONOFF ^= ONOFF_GEAR;
		plane->mins[2] += 4;
		gi.cprintf( plane->owner, PRINT_MEDIUM, "Gear retracted!\n" );
	}

	// frames
	if( plane->speed <= (plane->stallspeed + 100) && !(plane->ONOFF & ONOFF_GEAR)
			 && !(plane->ONOFF & ONOFF_BRAKE) )
		plane->s.frame = 0;
	else if( plane->speed > (plane->stallspeed + 100) && !(plane->ONOFF & ONOFF_GEAR)
	 			 && !(plane->ONOFF & ONOFF_BRAKE) )
		plane->s.frame = 1;
	else if( plane->speed <= (plane->stallspeed + 100) && (plane->ONOFF & ONOFF_GEAR)
	 			 && (plane->ONOFF & ONOFF_BRAKE ) )
		plane->s.frame = 2;
	else if( plane->speed <= (plane->stallspeed + 100) && !(plane->ONOFF & ONOFF_GEAR)
			 && (plane->ONOFF & ONOFF_BRAKE ) )
		plane->s.frame = 3;
	else if( plane->speed <= (plane->stallspeed + 100) && (plane->ONOFF & ONOFF_GEAR)
	  			 && !(plane->ONOFF & ONOFF_BRAKE ) )
		plane->s.frame = 4;
	else if( plane->speed > (plane->stallspeed + 100) && !(plane->ONOFF & ONOFF_GEAR)
	  			 && (plane->ONOFF & ONOFF_BRAKE ) )
		plane->s.frame = 5;
	if( plane->ONOFF & ONOFF_WEAPONBAY )
		plane->s.frame += 6;

	// skins
	if( plane->thrust > THRUST_MILITARY )
		plane->s.skinnum = 1;
	else
		plane->s.skinnum = 0;

	plane->elevator *= (plane->turnspeed[0]/400);
	plane->aileron *= (plane->turnspeed[2]/400);
	plane->rudder *= (plane->turnspeed[1]/400);

	// speed dependent maneuverability for planes
	if( plane->speed > plane->stallspeed )
	{
		plane->elevator *= (float)(plane->stallspeed/(plane->speed*1.2));
		if( plane->ONOFF & ONOFF_AIRBORNE )
			plane->rudder *= (float)(plane->stallspeed/plane->speed);
	}

	// movement in the air
	if( plane->ONOFF & ONOFF_AIRBORNE )
	{
		if( plane->HASCAN & HASCAN_WEAPONBAY )
		{
			if( level.time > plane->wait && !(plane->DAMAGE & DAMAGE_WEAPONBAY) &&
				(plane->ONOFF & ONOFF_WEAPONBAY) )
			{
				plane->ONOFF &= ~ONOFF_WEAPONBAY;
				plane->attenuation = level.time + 1.0;
			}
		}

		// stalled
		if( plane->ONOFF & ONOFF_STALLED )
		{
			if( plane->speed < plane->stallspeed )// stay stalled
			{
				if( plane->s.angles[0] < 70 || plane->s.angles[0] > 110 )
				{
					if( plane->s.angles[0] >= 270 || plane->s.angles[0] <= 70 )
						plane->avelocity[0] = plane->turnspeed[0]/2;
					else
						plane->avelocity[0] = -plane->turnspeed[0]/2;
				}
				if( fabs(plane->s.angles[2]) > 20 )
					plane->avelocity[1] = plane->turnspeed[1];
				else
					plane->avelocity[1] = 0;
			}
			else // end stall
			{
				plane->ONOFF ^= ONOFF_STALLED;
			}
			if( plane->s.angles[0] > 70 && plane->s.angles[0] < 110 )
				plane->avelocity[0] = 0;
		}
		// normal movement
		else
		{
			// angluar corrections
/*			sinpitch = sin((plane->s.angles[0]*M_PI)/180.0);
			cospitch = cos((plane->s.angles[0]*M_PI)/180.0);
			if( plane->s.angles[0] < 90 || plane->s.angles[0] >= 270 )
			{*/
				sinroll = -sin((plane->s.angles[2]*M_PI*VM)/180.0);
/*				cosroll = cos((plane->s.angles[2]*M_PI*VM)/180.0);
			}
			else
			{
				sinroll = -sin((-plane->s.angles[2]*M_PI*VM)/180.0);
				cosroll = cos((-plane->s.angles[2]*M_PI*VM)/180.0);
			}*/
			// elevator use
//			gi.bprintf( PRINT_HIGH, "%.1f %d\n", plane->s.angles[0], plane->elevator );
			if( plane->elevator )
			{
				if( ( plane->s.angles[0] < 90 || plane->s.angles[0] >= 270 ) )
				{
					if( (plane->elevator > 0 && (plane->s.angles[0] < 80||plane->s.angles[0] > 270) ) ||
						(plane->elevator < 0 && (plane->s.angles[0] > 280||plane->s.angles[0] < 90) ) )
						plane->avelocity[0] = plane->elevator;
				}
				else
				{
					if( (plane->elevator > 0 && plane->s.angles[0] < 260) ||
						(plane->elevator < 0 && plane->s.angles[0] > 100) )
						plane->avelocity[0] = -plane->elevator;
				}
			}
			// rudder use
			if( plane->rudder )
			{
				plane->avelocity[1] = plane->rudder;
			}
			// aileron use
			if( plane->aileron )
			{
				if( ( plane->s.angles[0] < 90 || plane->s.angles[0] >= 270 ) )
				{
					if( ( plane->aileron*VM > 0 && plane->s.angles[2] < 80 ) ||
						( plane->aileron*VM < 0 && plane->s.angles[2] > -80) )
					{
						plane->avelocity[2] = plane->aileron*VM;
					}
					else if( fabs(plane->s.angles[2]) >= 90 )
						plane->aileron = 0;
					if( fabs(plane->s.angles[2]) > 5 )
					{
						if( plane->s.angles[2] > 0 && plane->aileron*VM > 0 )
							plane->avelocity[1] -= plane->turnspeed[0]*sinroll;
						else if( plane->s.angles[2] < 0 && plane->aileron*VM < 0 )
							plane->avelocity[1] -= plane->turnspeed[0]*sinroll;
					}
				}
				else
				{
					if( ( plane->aileron*VM < 0 && (plane->s.angles[2] > 100 || plane->s.angles[2] <= -90) ) ||
						( plane->aileron*VM > 0 && (plane->s.angles[2] < -100 || plane->s.angles[2] >= 90) ) )
						plane->avelocity[2] = plane->aileron*VM;
					else if( fabs(plane->s.angles[2]) <= 90 )
						plane->aileron = 0;
					if( fabs(plane->s.angles[2]) < 175 )
					{
						if( plane->s.angles[2] > 0 && plane->aileron*VM < 0 )
							plane->avelocity[1] += plane->turnspeed[0]*sinroll;
						else if( plane->s.angles[2] < 0 && plane->aileron*VM > 0 )
							plane->avelocity[1] += plane->turnspeed[0]*sinroll;
					}
				}
			}
			// stall when too slow
			if( plane->speed < plane->stallspeed )
				plane->ONOFF |= ONOFF_STALLED;
			// auto-roll-back!
//			gi.bprintf( PRINT_HIGH, "%.1f\n", plane->s.angles[2] );
			if( (!plane->aileron) )			
			{
				if( ( plane->s.angles[0] < 90 || plane->s.angles[0] >= 270 ) )
				{
					if( fabs( plane->s.angles[2] ) > 10 )
					{
						if( plane->s.angles[2]*VM > 10 )
							plane->avelocity[2] = -plane->turnspeed[2]*VM;
						else if( plane->s.angles[2]*VM < -10 )
							plane->avelocity[2] = plane->turnspeed[2]*VM;
					}
					else if( fabs( plane->s.angles[2] ) > 5 )
					{
						if( plane->s.angles[2]*VM > 5 )
							plane->avelocity[2] = -plane->turnspeed[2]/3*VM;
						else if( plane->s.angles[2]*VM < -5 )
							plane->avelocity[2] = plane->turnspeed[2]/3*VM;
					}
					else
						plane->s.angles[2] = 0;
				}
				else 
				{
					if( fabs( plane->s.angles[2] ) < 170 )
					{
						if( plane->s.angles[2]*VM < 170 && plane->s.angles[2]*VM > 0 )
							plane->avelocity[2] = plane->turnspeed[2]*VM;
						else if( plane->s.angles[2]*VM > -170 )
							plane->avelocity[2] = -plane->turnspeed[2]*VM;
					}
					else if( fabs( plane->s.angles[2] ) < 175 )
					{
						if( plane->s.angles[2]*VM < 175 && plane->s.angles[2]*VM > 0 )
							plane->avelocity[2] = plane->turnspeed[2]/3*VM;
						else if( plane->s.angles[2]*VM > -175 )
							plane->avelocity[2] = -plane->turnspeed[2]/3*VM;
					}
					else
						plane->s.angles[2] = 180;
				}
			}
			// did we touch the sky?
			VectorCopy( plane->s.origin, above );
			above[2] += 64;
			trace = gi.trace( plane->s.origin, NULL, NULL, above, plane, MASK_ALL );
			if( trace.fraction < 1.0 && (trace.surface->flags & SURF_SKY ||
		   		strncmp(trace.surface->name,"clip",strlen(trace.surface->name))== 0) )
			{
				if( plane->s.angles[0] > 180 && plane->s.angles[0] < 360 )
				{
					if( plane->s.angles[0] < 270 )
						plane->s.angles[0] = 180;
					else
						plane->s.angles[0] = 0;
				}
			}		
			// get altitude and check whether to land the plane
			VectorCopy( plane->s.origin, above );
			above[2] -= 2000;
			trace = gi.trace( plane->s.origin, NULL, NULL, above, plane, MASK_SOLID );
			if( trace.fraction < 1 )
				plane->altitude = ((int)(plane->s.origin[2] - trace.endpos[2]))-12;
			else
				plane->altitude = 9999;
			if( plane->altitude < 80 )
				IsPlaneLanded( plane );
		}
		if( plane->waterlevel )
		{
			VectorClear( plane->velocity );
			T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 9999, 0, 0, MOD_SPLASH );
		}
	}
	// and movement on the ground
	else
	{
		VectorSet( above, plane->s.origin[0], plane->s.origin[1],
						plane->s.origin[2] - 32 );

		trace = gi.trace( plane->s.origin, NULL, NULL, above, plane, MASK_ALL );

		if( plane->ONOFF & ONOFF_WEAPONBAY )
			plane->ONOFF &= ~ONOFF_WEAPONBAY;

		// are you standing on a runway ?
		if( strcmp( trace.ent->classname, "func_runway" ) == 0
		|| strcmp( trace.ent->classname, "func_door" ) == 0
		|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0 )
		{
			plane->groundentity = trace.ent;
			if( plane->ONOFF & ONOFF_LANDED_TERRAIN )
			{
				plane->ONOFF |= ONOFF_LANDED;
				plane->ONOFF &= ~ONOFF_LANDED_TERRAIN;
			}
			else if( plane->ONOFF & ONOFF_LANDED_TERRAIN_NOGEAR )
			{
				plane->ONOFF |= ONOFF_LANDED_NOGEAR;
				plane->ONOFF &= ~ONOFF_LANDED_TERRAIN_NOGEAR;
			}

			if( plane->elevator < 0 && plane->speed >= plane->stallspeed )
			{
				if( plane->s.angles[0] < 10 )
					plane->avelocity[0] = plane->elevator;
				else
					plane->avelocity[0] = -plane->elevator;
				plane->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN 
						| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR);
				plane->ONOFF |= ONOFF_AIRBORNE;
				plane->groundentity = NULL;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
			else
			{
				plane->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2]
			   			 + abs(plane->mins[2])+2;
//				if( !(plane->ONOFF & ONOFF_GEAR) )
//					plane->s.origin[2] -= 4;
			}
			if( (plane->rudder || plane->aileron) && ( plane->ONOFF & ONOFF_LANDED ) )
			{
				if( plane->speed )
				{
					if( plane->rudder )
						plane->avelocity[1] = 2*plane->rudder;
					else if( plane->aileron )
					{
						plane->avelocity[1] = plane->aileron;
						if( plane->avelocity[1] > 0 &&
								plane->avelocity[1] > 2*plane->turnspeed[1] )
							plane->avelocity[1] = 2*plane->turnspeed[1];
						else if( plane->avelocity[1] < 0 &&
								plane->avelocity[1] < -2*plane->turnspeed[1] )
							plane->avelocity[1] = -2*plane->turnspeed[1];
					}
				}
			}
			// crashlanded planes
			if( (plane->ONOFF & ONOFF_LANDED_NOGEAR) && plane->speed < 2 )
			{
				if( plane->s.angles[2] < 15 && (plane->HASCAN & HASCAN_ROUNDBODY) )
					plane->avelocity[2] = 10;
				else
				{
					VectorClear( plane->velocity );
					plane->avelocity[2] = 0;
					plane->think = EmptyPlaneDie;
					plane->nextthink = level.time + 10;
					return;
				}
			}
		}
		// moving on terrain
		else if( trace.contents & CONTENTS_SOLID )
		{
			plane->groundentity = trace.ent;
			if( plane->ONOFF & ONOFF_LANDED )
			{
				plane->ONOFF |= ONOFF_LANDED_TERRAIN;
				plane->ONOFF &= ~ONOFF_LANDED;
			}
			else if( plane->ONOFF & ONOFF_LANDED_NOGEAR )
			{
				plane->ONOFF |= ONOFF_LANDED_TERRAIN_NOGEAR;
				plane->ONOFF &= ~ONOFF_LANDED_NOGEAR;
			}

			if( plane->elevator < 0 && plane->speed >= plane->stallspeed )
			{
				if( plane->s.angles[0] < 10 )
					plane->avelocity[0] = plane->elevator;
				else
					plane->avelocity[0] = -plane->elevator;
				plane->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN 
						| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR);
				plane->ONOFF |= ONOFF_AIRBORNE;
				plane->groundentity = NULL;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
			else
			{
				plane->s.origin[2] = trace.endpos[2] + abs(plane->mins[2])+2;
//				if( !(plane->ONOFF & ONOFF_GEAR) )
//					plane->s.origin[2] -= 4;
			}
			if( (plane->rudder || plane->aileron) && ( (plane->ONOFF & ONOFF_LANDED_TERRAIN) 
					&& plane->ONOFF & ONOFF_GEAR ) )
			{
				if( plane->speed )
				{
					if( plane->rudder )
						plane->avelocity[1] = plane->rudder;
					else if( plane->aileron )
					{
						plane->avelocity[1] = plane->aileron;
						if( plane->avelocity[1] > 0 &&
								plane->avelocity[1] > plane->turnspeed[1] )
							plane->avelocity[1] = plane->turnspeed[1];
						else if( plane->avelocity[1] < 0 &&
								plane->avelocity[1] < -plane->turnspeed[1] )
							plane->avelocity[1] = -plane->turnspeed[1];
					}
				}
			}
			// crashlanded planes
			if( (plane->ONOFF & ONOFF_LANDED_TERRAIN_NOGEAR) && plane->speed < 2 )
			{
				if( plane->s.angles[2] < 15 && (plane->HASCAN & HASCAN_ROUNDBODY) )
					plane->avelocity[2] = 10;
				else
				{
					VectorClear( plane->velocity );
					plane->avelocity[2] = 0;
					plane->think = EmptyPlaneDie;
					plane->nextthink = level.time + 10;
					return;
				}
			}
		}
		// moving on water
		else if( trace.contents & CONTENTS_LIQUID )
		{
//			gi.bprintf(PRINT_HIGH, "on water\n" );
			plane->groundentity = trace.ent;
			if( plane->elevator < 0 && plane->speed >= plane->stallspeed )
			{
				if( plane->s.angles[0] < 10 )
					plane->avelocity[0] = plane->elevator;
				else
					plane->avelocity[0] = -plane->elevator;
				plane->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN 
						| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR);
				plane->ONOFF |= ONOFF_AIRBORNE;
				plane->groundentity = NULL;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
			else
			{
				if( plane->speed > 5 )
				{
					plane->s.origin[2] = trace.endpos[2] + abs(plane->mins[2])+2;
				}
				else	// sinking
				{
					plane->ONOFF |= ONOFF_SINKING;
					plane->DAMAGE |= DAMAGE_ENGINE_DESTROYED | DAMAGE_ENGINE_20 | DAMAGE_ENGINE_40 |
							DAMAGE_ENGINE_60 | DAMAGE_ENGINE_80 | DAMAGE_AFTERBURNER;
					plane->thrust = THRUST_IDLE;
				}
			}
		}
		// if not then you stall
		else
		{
			plane->ONOFF &= ~(ONOFF_LANDED | ONOFF_LANDED_TERRAIN | ONOFF_LANDED_WATER_NOGEAR
						| ONOFF_LANDED_NOGEAR | ONOFF_LANDED_TERRAIN_NOGEAR | ONOFF_LANDED_WATER);
			plane->ONOFF |= ONOFF_AIRBORNE;
			if( plane->speed < plane->stallspeed )
			{
				plane->ONOFF |= ONOFF_STALLED;
			}
			else
			{
				plane->groundentity = NULL;
				gi.cprintf( plane->owner, PRINT_MEDIUM, "AIRBORNE!\n" );
			}
		}
	}

	// make velocity
	AngleVectors( plane->s.angles, forward, NULL, NULL );
	VectorScale( forward, plane->speed, plane->velocity );

	// add down velocity when slow
//	if( (plane->ONOFF & ONOFF_AIRBORNE) && plane->speed < plane->stallspeed * 1.5 )
	{
//		plane->velocity[2] -= ((plane->stallspeed * 1.5) - plane->speed );
	}

	// lock weapons
	if( plane->weapon[plane->active_weapon] == WEAPON_SIDEWINDER ||
		plane->weapon[plane->active_weapon] == WEAPON_STINGER ||
		plane->weapon[plane->active_weapon] == WEAPON_AMRAAM ||
		plane->weapon[plane->active_weapon] == WEAPON_PHOENIX )
		AAM_Lock( plane );
	else if( plane->weapon[plane->active_weapon] == WEAPON_HELLFIRE ||
		plane->weapon[plane->active_weapon] == WEAPON_MAVERICK ||
		plane->weapon[plane->active_weapon] == WEAPON_ANTIRADAR ||
		plane->weapon[plane->active_weapon] == WEAPON_RUNWAYBOMBS ||
		plane->weapon[plane->active_weapon] == WEAPON_LASERBOMBS )
		ATGM_Lock( plane );
	else if( plane->weapon[plane->active_weapon] == WEAPON_AUTOCANNON )
		Autocannon_Lock( plane );
	else if( plane->weapon[plane->active_weapon] == WEAPON_JAMMER )
	{
		if( plane->ONOFF & ONOFF_JAMMER )
			Jammer_Active( plane );
		else
			Reload_Jammer( plane );
		if( plane->enemy )
		{
			plane->enemy->lockstatus = 0;
			plane->enemy = NULL;
		}
	}
	else if( plane->enemy )
	{
		plane->enemy->lockstatus = 0;
		plane->enemy = NULL;
	}

	// fire autocannon
	if( plane->owner->client->buttons & BUTTON_ATTACK )
		Weapon_Autocannon_Fire( plane );

	if( (plane->HASCAN & HASCAN_STEALTH) &&
		!(plane->ONOFF & ONOFF_WEAPONBAY) && 
		!(plane->ONOFF & ONOFF_GEAR) )
		plane->ONOFF |= ONOFF_STEALTH;
	else
		plane->ONOFF &= ~ONOFF_STEALTH;

	// resetting stuff
	plane->radio_target = NULL;
	plane->ONOFF &= ~ONOFF_IS_CHANGEABLE;
	plane->teammaster = NULL;

	plane->nextthink = level.time + 0.1;
}



void PlaneTouch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( self->deadflag == DEAD_DEAD || strcmp( other->classname, "freed" ) == 0
		|| strcmp( other->classname, "debris" ) == 0 )
		return;
	if( strcmp( other->classname, "bolt" ) == 0 )
	{
		return;
	}
	if( self->ONOFF & ONOFF_SINKING )
	{
		self->ONOFF &= ~ONOFF_SINKING;
		self->think = EmptyPlaneDie;
		self->nextthink = level.time + 10.0;
		return;
	}
//	if( (self->ONOFF & ONOFF_LANDED_WATER) || (self->ONOFF & ONOFF_LANDED_WATER_NOGEAR) )
//		return;
	if( strcmp( other->classname, "camera" ) == 0 )
		return;
	if( surf && (surf->flags & SURF_SKY) )
	{
		if( !self->deadflag )
			return;
		T_Damage( self, world, world, vec3_origin, self->s.origin, vec3_origin, 9999, 0, 0, MOD_SKY );
		return;
	}
	if( strcmp( other->classname, "func_runway") == 0 && !(self->ONOFF & ONOFF_AIRBORNE) )
		return;
	else if( ( strcmp( other->classname, "plane") == 0 || strcmp( other->classname, "helo" ) == 0 ) 
			   	 && (other->ONOFF & ONOFF_PILOT_ONBOARD) )
	{
		if( !self->deadflag || self->deadflag == DEAD_DYING )
		{
			T_Damage( self, other, other, vec3_origin, self->s.origin, vec3_origin, 9999, 0, 0, MOD_MIDAIR );
			return;
		}
	}
	else if( strcmp( other->classname, "ground" ) == 0 && (other->ONOFF & ONOFF_PILOT_ONBOARD ) )
	{
		if( !self->deadflag || self->deadflag == DEAD_DYING )
		{
			T_Damage( self, other, other, vec3_origin, self->s.origin, vec3_origin, 9999, 0, 0, MOD_GROUND );
			return;
		}
	}
	else if( strcmp( other->classname, "LQM" ) == 0 && self->speed > 5 )
	{
		T_Damage( self, other, other, vec3_origin, self->s.origin, vec3_origin, 
					((self->speed/8)+random()*20), 0, 0, MOD_LQM );
	}
	else
	{
		if( !(self->ONOFF & ONOFF_AIRBORNE) )
		{
			if( VectorLength(self->velocity) >= 10 )
			{
				gi.sound( self, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
				T_Damage( self, world, world, vec3_origin, self->s.origin, vec3_origin, 
						(VectorLength(self->velocity)/2), 0, 0, MOD_ACCIDENT );
			}
			self->speed = 0;
			VectorClear( self->avelocity );
			VectorClear( self->velocity );
		}
		else
		{
			if( ( strcmp( other->classname, "plane" ) == 0 ||
		   		strcmp( other->classname, "helo" ) == 0 ||
			    strcmp( other->classname, "ground" ) == 0 ) &&
				!(other->ONOFF & ONOFF_PILOT_ONBOARD ) )
			{
				T_Damage( self, world, world, vec3_origin, self->s.origin, vec3_origin, 9999, 0, 0, MOD_VEHICLE );
			}
			else
			{
				T_Damage( self, world, world, vec3_origin, self->s.origin, vec3_origin, 9999, 0, 0, MOD_CRASH );
			}
		}
	}
}


void PlaneSpeed( edict_t *plane )
{
	float	Winkel;
	float	currenttop, 
			acceleration = plane->accel/8, 
			deceleration = plane->decel/8;

	if( plane->deadflag == DEAD_DEAD )
	{
		plane->speed = 0;
		return;
	}	

	// influence of angle on velocity
	if( plane->s.angles[0] < 357 && plane->s.angles[0] >= 270 )
		Winkel = -(360 - plane->s.angles[0]);
	else if( plane->s.angles[0] < 270 && plane->s.angles[0] > 183 )
		Winkel = 180 - plane->s.angles[0];
	else if( plane->s.angles[0] > 3 && plane->s.angles[0] <= 90 )
		Winkel = plane->s.angles[0];
	else if( plane->s.angles[0] > 90 && plane->s.angles[0] < 177 )
		Winkel = 180 - plane->s.angles[0];
	else
		Winkel = 0;

	if( Winkel > 0 )
		Winkel = (ceil(Winkel/10.0)*2.2);
	else
		Winkel = (floor(Winkel/10.0)*2.5);

//	Winkel *= (sv_gravity->value / 800);	// add dependency on gravity
	Winkel += plane->thrust/2.0;			// and thrust

	// current top speed
	currenttop = (plane->topspeed / THRUST_MILITARY ) * plane->thrust;

	if( plane->ONOFF & ONOFF_BRAKE )
	{
		if( !(plane->ONOFF & ONOFF_AIRBORNE) )
			currenttop *= (3.6/5.0);
		else
		{
			currenttop *= (3.0/5.0);
			deceleration *= 1.2;
		}
	}

	if( (plane->ONOFF & ONOFF_GEAR) && (plane->ONOFF & ONOFF_AIRBORNE) )
	{
		currenttop *= (4.1/5.0);
	}
	if( plane->ONOFF & ONOFF_WEAPONBAY )
	{
		currenttop *= (4.3/5.0);
	}

	// acceleration depends on thrust
	acceleration *= (1 + ( (float)plane->thrust - THRUST_MILITARY )/15);

	// runway
	if( !(plane->ONOFF & ONOFF_AIRBORNE) )
	{
		if( plane->ONOFF & ONOFF_LANDED )
		{
			currenttop *= 0.55;
			deceleration *= 1.5;
			acceleration *= 0.8;
		}
		else if( plane->ONOFF & ONOFF_LANDED_NOGEAR )
		{
			currenttop = 0;
			deceleration *= 6;
			acceleration = 0;
			if( plane->speed > 0 )
			{
				T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 1, 0, 0, MOD_NOGEAR );
				plane->s.effects = EF_ROCKET;
			}
		}
		else if( plane->ONOFF & ONOFF_LANDED_TERRAIN )
		{
			if( plane->HASCAN & HASCAN_TERRAINLANDING )
			{
				currenttop *= 0.8;
				deceleration *= 2;
				acceleration *= 0.75;
			}
			else
			{
				currenttop *= 0.25;
				deceleration *= 4;
				acceleration *= 0.2;
				if( plane->speed > 30 && (plane->ONOFF & ONOFF_GEAR) )
				{
					plane->DAMAGE |= DAMAGE_GEAR;
					plane->ONOFF &= ~(ONOFF_GEAR | ONOFF_LANDED_TERRAIN);
					plane->ONOFF |= ONOFF_LANDED_TERRAIN_NOGEAR;
					plane->mins[2] += 4;
					if( plane->owner != NULL )
						gi.cprintf( plane->owner, PRINT_HIGH, "Gear ripped off!\n" );
					T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 20, 0, 0, MOD_NOGEAR );
					gi.sound( plane, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
				}
			}
		}
		else if( plane->ONOFF & ONOFF_LANDED_TERRAIN_NOGEAR )
		{
			currenttop = 0;
			deceleration *= 9;
			acceleration = 0;
			if( plane->speed > 0 )
			{
				T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 2, 0, 0, MOD_NOGEAR );
				plane->s.effects = EF_ROCKET;
			}
		}
		else if( (plane->ONOFF & ONOFF_LANDED_WATER) || (plane->ONOFF & ONOFF_LANDED_WATER_NOGEAR) )
		{
			currenttop = 0;
			deceleration *= 6;
			acceleration = 0;
			if( plane->speed > 5 )
			{
				T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 1, 0, 0, MOD_NOGEAR );
				plane->s.effects = EF_GRENADE;
			}
			if( plane->speed > 30 && (plane->ONOFF & ONOFF_GEAR) )
			{
				plane->DAMAGE |= DAMAGE_GEAR;
				plane->ONOFF &= ~(ONOFF_GEAR | ONOFF_LANDED_TERRAIN);
				plane->ONOFF |= ONOFF_LANDED_TERRAIN_NOGEAR;
				plane->mins[2] += 4;
				if( plane->owner != NULL )
					gi.cprintf( plane->owner, PRINT_HIGH, "Gear ripped off!\n" );
				T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 20, 0, 0, MOD_NOGEAR );
				gi.sound( plane, CHAN_AUTO, gi.soundindex( "pain/xfire.wav" ), 1, ATTN_NORM, 0 );
			}
		}
	}

	// influence of speedbrakes
	if( plane->ONOFF & ONOFF_BRAKE )
	{
		if( plane->ONOFF & ONOFF_AIRBORNE )
		{
			deceleration *= 1.6;
			acceleration /= 2.8;
			// the higher the speed the better the speedbrakes
			if( plane->speed > 200 )
				deceleration *= (plane->speed/200);
		}
		else
		{
			deceleration *= 1.3;
			acceleration /= 2.6;
		}
	}


	// the higher the speed the higher the drag
	if( (plane->speed) > 250 )
		deceleration *= (plane->speed/250);

	// calculate the actual speed
	if( plane->speed >= currenttop )
	{
		plane->speed -= deceleration;
		if( plane->speed < currenttop )
			plane->speed = currenttop;

	}
	else
	{
		plane->speed += acceleration;
		if( plane->speed > currenttop )
			plane->speed = currenttop;
	}

	// add influence of angle
	if( plane->ONOFF & ONOFF_AIRBORNE )
		plane->speed += (Winkel/8);

	if( !(plane->DAMAGE & DAMAGE_ENGINE_DESTROYED) )
		PlaneSound( plane );
	else
		gi.sound( plane, CHAN_BODY, gi.soundindex( "death/0.wav" ), 1, ATTN_NORM, 0 );
}






void EmptyPlaneDie( edict_t *plane )
{
	gi.sound( plane, CHAN_BODY, gi.soundindex( "death/0.wav" ), 1, ATTN_NORM, 0 );

//	gi.bprintf( PRINT_HIGH, "empty dead!\n" );

	plane->deadflag = DEAD_DEAD;

	if( !(plane->ONOFF & ONOFF_PILOT_ONBOARD) )
	{
		plane->think = G_FreeEdict;
		plane->nextthink = level.time + 3;
	}

	gi.WriteByte (svc_temp_entity);
	if( (plane->ONOFF & ONOFF_SINKING) || (plane->ONOFF & ONOFF_LANDED_WATER_NOGEAR)
			|| (plane->ONOFF & ONOFF_LANDED_WATER) )
		gi.WriteByte(TE_ROCKET_EXPLOSION_WATER);
	else
		gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (plane->s.origin);
	gi.multicast (plane->s.origin, MULTICAST_PVS);
	VehicleExplosion( plane );
	SetVehicleModel( plane );
}





void PlaneObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
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
}







void PlaneDie (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	gi.bprintf( PRINT_HIGH, "Plane die!\n" );

	if( self->ONOFF & ONOFF_AIRBORNE )
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
	gi.linkentity( self );
}




