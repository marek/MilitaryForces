// file v_lock.c by Bjoern Drabeck
// handles lock and unlock functions and commands

#include "g_local.h"
#include "v_lock.h"

void Cmd_Unlock_f( edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;
	if( !ent->vehicle->enemy )
		return;
	gi.bprintf( PRINT_HIGH, "Unlocked!\n" );
	ent->vehicle->enemy->lockstatus = 0;
	ent->vehicle->enemy = NULL;
	ent->vehicle->drone_decision_time = level.time + 0.5;
}

void UnlockEnemyOnDie( edict_t *vehicle )
{
	if( !vehicle->lockstatus )
		return;
	else
	{
		int i;
		edict_t *testent;

		testent = &g_edicts[0];
		for (i=0 ; i<globals.num_edicts ; i++, testent++)
		{
			if( testent->enemy )
			{
				if( testent->enemy == vehicle )
				{
					testent->enemy = NULL;
					if( strcmp( testent->classname, "Sidewinder" ) == 0 ||
						strcmp( testent->classname, "Stinger" ) == 0 ||
						strcmp( testent->classname, "AMRAAM" ) == 0 ||
						strcmp( testent->classname, "Phoenix" ) == 0 ||
						strcmp( testent->classname, "Maverick" ) == 0 ||
						strcmp( testent->classname, "ATGM" ) == 0 )
						testent->think = nolockthink;
					else if( strcmp( testent->classname, "LGB" ) == 0 )
						testent->think = bomb_think;
				}
			}
		}
	}
}


void AutoCannonLockOn( edict_t *vehicle )
{
	vec3_t		forward, testvec;
	float		test;
	edict_t		*lock = NULL;

//	gi.bprintf( PRINT_HIGH, "Auto\n" );

	if( vehicle->enemy && vehicle->enemy->deadflag )
	{
		vehicle->enemy->lockstatus = 0;
		vehicle->enemy = NULL;
	}

	if( level.time < vehicle->drone_decision_time )
		return;

	if( vehicle->owner->client->pers.secondary != FindItem( "AutoCannon" ) )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		return;
	}

	if( vehicle->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] == 0 )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		return;
	}

/*	if( !vehicle->owner->client->pers.autoaim )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = 0;
		}
		return;
	}*/

	if( vehicle->enemy )
	{
		if( !visible(vehicle,vehicle->enemy) )
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
		if( vehicle->enemy )	// out of range
		{
			VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
			if( VectorLength( testvec ) > STANDARD_RANGE*AUTOCANNON_MODIFIER )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy )	// jammer
		{
			if( level.time < vehicle->enemy->jam_time )
			{
				if( (int)(random()*100) <= JAMMER_EFFECT )
				{
					vehicle->enemy->lockstatus = 0;
					vehicle->enemy = 0;
				}
			}
		}
		if( vehicle->enemy )
		{
			// test if within the target-cone
			AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > 29 )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy ) //if enemy is not lost then show him we lock
			if( !vehicle->enemy->lockstatus )
				vehicle->enemy->lockstatus = 1;
	}

	if( !vehicle->enemy )
	{
		while( (lock = findradius( lock, vehicle->s.origin, STANDARD_RANGE*AUTOCANNON_MODIFIER ) ) != NULL )
		{
			if( lock == vehicle )
				continue;
			if( /* strcmp( lock->classname, "plane" ) != 0 &&
				strcmp( lock->classname, "helo" ) != 0 &&*/
				strcmp( lock->classname, "ground" ) != 0 )
				continue;
			if( vehicle->owner )
				if( lock == vehicle->owner )
					continue;
			if( !lock->takedamage )
				continue;
			if( lock->deadflag )
				continue;
			if( strcmp(vehicle->classname, "plane") == 0 ||
				strcmp(vehicle->classname, "helo") == 0 )
			{
				if( !visible(vehicle,lock) )
					continue;
				if( !infront(vehicle, lock) )
					continue;
			}
			else if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				if( !visible(vehicle->movetarget,lock) )
					continue;
				if( !infront(vehicle->movetarget,lock) )
					continue;
			}
			// jammer
			if( level.time < lock->jam_time )
			{
				if( (int)(random()*100) <= JAMMER_EFFECT )
					continue;
			}
			// test if within the target-cone
			if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				AngleVectors( vehicle->movetarget->s.angles, forward, NULL, NULL );
			}
			else
			{
				AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			}
			VectorSubtract( lock->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > 19 )
				continue;

			vehicle->enemy = lock;
			lock->lockstatus = 1;
			break;
		}
	}

}




void ATGMLockOn( edict_t *vehicle )
{
	edict_t		*lock = NULL;
	vec3_t		forward, testvec;
	float		test;
	float		range;
	int			cone;

	if( level.time < vehicle->drone_decision_time )
		return;

	if( vehicle->enemy && vehicle->enemy->deadflag )
	{
		vehicle->enemy->lockstatus = 0;
		vehicle->enemy = NULL;
	}

	if( vehicle->owner->client->pers.secondary != FindItem( "ATGM Launcher" ) &&
		vehicle->owner->client->pers.secondary != FindItem( "Maverick Launcher") &&
	    vehicle->owner->client->pers.secondary != FindItem("Laser Bombs") )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		return;
	}

	if( vehicle->owner->client->pers.secondary == FindItem( "ATGM Launcher" ) )
	{
		range = STANDARD_RANGE * HELLFIRE_MODIFIER;
		cone = 14;
	}
	else if( vehicle->owner->client->pers.secondary == FindItem( "Laser Bombs" ) )
	{
		range = STANDARD_RANGE * LGB_MODIFIER;
		cone = 35;
	}
	else if( vehicle->owner->client->pers.secondary == FindItem( "Maverick Launcher" ) )
	{
		range = STANDARD_RANGE * MAVERICK_MODIFIER;
		cone = 14;
	}

/*	if( vehicle->owner->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] == 0 &&
		vehicle->owner->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] == 0 &&
		vehicle->owner->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] == 0 )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		return;
	}*/

	if( vehicle->enemy )
	{
		if( !visible(vehicle,vehicle->enemy) )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		if( vehicle->enemy ) // if there still is a lock
		{
			if( !infront(vehicle, vehicle->enemy) )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy ) // accidentally got a lock on a wrong vehicle ?
		{
			if( strcmp( vehicle->enemy->classname, "plane" ) == 0 ||
				strcmp( vehicle->enemy->classname, "helo" ) == 0 )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy )	// out of range
		{
			VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
			if( VectorLength( testvec ) > range )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy )	// jammer
		{
			if( level.time < vehicle->enemy->jam_time )
			{
				if( (int)(random()*100) <= JAMMER_EFFECT )
				{
					vehicle->enemy->lockstatus = 0;
					vehicle->enemy = 0;
				}
			}
		}
		if( vehicle->enemy )
		{
			// test if within the target-cone
			AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > cone )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy ) //if enemy is not lost then show him we lock
			if( !vehicle->enemy->lockstatus )
				vehicle->enemy->lockstatus = 1;
	}

	if( !vehicle->enemy )
	{
/*		AngleVectors( vehicle->s.angles, forward, NULL, NULL );
		VectorScale( forward, vehicle->radar_range, stop );
		trace = gi.trace( vehicle->s.origin, vehicle->mins, vehicle->maxs, stop, vehicle, MASK_ALL );
		if( trace.fraction < 1 )
		{
			if( strcmp( trace.ent->classname, "plane" ) == 0 ||
				strcmp( trace.ent->classname, "helo" ) == 0 ||
				strcmp( trace.ent->classname, "ground" ) == 0 )
			{
				gi.bprintf( PRINT_HIGH, "Target!\n" );
				vehicle->enemy = lock;
				lock->lockstatus = 1;
			}
		}*/
		while( (lock = findradius( lock, vehicle->s.origin, range ) ) != NULL )
		{
			if( lock == vehicle )
				continue;
			if( strcmp( lock->classname, "ground" ) != 0 )
				continue;
			if( vehicle->owner )
				if( lock == vehicle->owner )
					continue;
			if( !lock->takedamage )
				continue;
			if( lock->deadflag )
				continue;
			if( strcmp( vehicle->classname, "plane" ) == 0 ||
				strcmp( vehicle->classname, "helo" ) == 0 )
			{
				if( !visible(vehicle,lock) )
					continue;
				if( !infront(vehicle,lock) )
					continue;
			}
			else if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				if( !visible(vehicle->movetarget,lock) )
					continue;
				if( !infront(vehicle->movetarget,lock) )
					continue;
			}
			// jammer
			if( level.time < lock->jam_time )
			{
				if( (int)(random()*100) <= JAMMER_EFFECT )
					continue;
			}
			// test if within the target-cone
			if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				AngleVectors( vehicle->movetarget->s.angles, forward, NULL, NULL );
			}
			else
			{
				AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			}
			VectorSubtract( lock->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > 6 )
				continue;
			vehicle->enemy = lock;
			lock->lockstatus = 1;
			break;
		}
	}

}




void SidewinderLockOn( edict_t *vehicle )
{
	vec3_t		forward, testvec;
	float		test;
	edict_t		*lock = NULL;
	float		range;

//	gi.bprintf( PRINT_HIGH, "Side\n" );

	if( vehicle->enemy && ( vehicle->enemy->deadflag || strcmp( vehicle->enemy->classname, "freed" ) == 0 ))
	{
		vehicle->enemy->lockstatus = 0;
		vehicle->enemy = NULL;
	}

	if( level.time < vehicle->drone_decision_time )
		return;

	if( vehicle->owner->client->pers.secondary != FindItem( "SW Launcher" ) &&
		 vehicle->owner->client->pers.secondary != FindItem( "ST Launcher" ) &&
		 vehicle->owner->client->pers.secondary != FindItem( "AM Launcher" ) &&
		 vehicle->owner->client->pers.secondary != FindItem( "PH Launcher" ) )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		return;
	}
	if( vehicle->owner->client->pers.secondary == FindItem( "SW Launcher" ) )
		range = STANDARD_RANGE * SIDEWINDER_MODIFIER;
	else if( vehicle->owner->client->pers.secondary == FindItem( "ST Launcher" ) )
		range = STANDARD_RANGE * STINGER_MODIFIER;
	else if( vehicle->owner->client->pers.secondary == FindItem( "AM Launcher" ) )
		range = STANDARD_RANGE * AMRAAM_MODIFIER;
	else if( vehicle->owner->client->pers.secondary == FindItem( "PH Launcher" ) )
		range = STANDARD_RANGE * PHOENIX_MODIFIER;

/*	if( vehicle->owner->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] == 0 )
	{
		if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
		return;
	}*/


	if( vehicle->enemy )
	{
		if( !visible(vehicle,vehicle->enemy) )
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
		if( vehicle->enemy ) // accidentally got a lock on a wrong vehicle ?
		{
			if( strcmp( vehicle->enemy->classname, "ground" ) == 0 )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy )	// out of range
		{
			VectorSubtract( vehicle->enemy->s.origin, vehicle->s.origin, testvec );
			if( VectorLength( testvec ) > range )
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
				if( (int)(random()*100) <= JAMMER_EFFECT )
				{
					vehicle->enemy->lockstatus = 0;
					vehicle->enemy = 0;
				}
			}
		}
		if( vehicle->enemy )
		{
			// test if within the target-cone
			AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > 15 )
			{
				vehicle->enemy->lockstatus = 0;
				vehicle->enemy = NULL;
			}
		}
		if( vehicle->enemy ) //if enemy is not lost then show him we lock
		{
			if( teamplay->value != 1 )
			{
				if( !vehicle->enemy->lockstatus )
					vehicle->enemy->lockstatus = 1;
			}
			else
			{
				if( vehicle->enemy->HASCAN & IS_DRONE )
				{
					if( vehicle->enemy->aqteam != vehicle->aqteam )
					{
						if( !vehicle->enemy->lockstatus )
							vehicle->enemy->lockstatus = 1;
					}
				}
				else
				{
					if( !vehicle->enemy->lockstatus )
						vehicle->enemy->lockstatus = 1;
				}
			}
		}
	}

	if( !vehicle->enemy )
	{
		while( (lock = findradius( lock, vehicle->s.origin, range ) ) != NULL )
		{
			if( lock == vehicle )
				continue;
			if( strcmp( lock->classname, "plane" ) != 0 &&
				strcmp( lock->classname, "helo" ) != 0 )
				continue;
			if( vehicle->owner )
				if( lock == vehicle->owner )
					continue;
			if( !lock->takedamage )
				continue;
			if( lock->deadflag )
				continue;
			if( strcmp(vehicle->classname, "plane") == 0 ||
				strcmp(vehicle->classname, "helo") == 0 )
			{
				if( !visible(vehicle,lock) )
					continue;
				if( !infront(vehicle, lock) )
					continue;
			}
			else if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				if( !visible(vehicle->movetarget,lock) )
					continue;
				if( !infront(vehicle->movetarget,lock) )
					continue;
			}
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
				if( (int)(random()*100) <= JAMMER_EFFECT )
					continue;
			}
			// test if within the target-cone
			if( strcmp( vehicle->classname, "ground" ) == 0 )
			{
				AngleVectors( vehicle->movetarget->s.angles, forward, NULL, NULL );
			}
			else
			{
				AngleVectors( vehicle->s.angles, forward, NULL, NULL );
			}
			VectorSubtract( lock->s.origin, vehicle->s.origin, testvec );
			VectorNormalize( forward );
			VectorNormalize( testvec );
			test = acos( testvec[0]*forward[0]+testvec[1]*forward[1]+
					testvec[2]*forward[2] );
			test *= (180/M_PI);
			if( test > 8 )
				continue;
			vehicle->enemy = lock;
			if( teamplay->value != 1 )
				lock->lockstatus = 1;
			else
			{
				if( vehicle->enemy->HASCAN & IS_DRONE )
				{
					if( vehicle->enemy->aqteam != vehicle->aqteam )
						lock->lockstatus = 1;
				}
				else
					lock->lockstatus = 1;
			}
			break;
		}
	}

}



