/************************************************/
/* Filename: aq2_bots.c  					    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 1999-07-10							*/
/*												*/
/* Description:									*/
/* NewAirQuake2 bot-specific stuff	    		*/
/*												*/
/************************************************/

#include "g_local.h"






void PlaneMovement_Bot( edict_t *plane );
void PlaneTouch_Bot( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
void PlaneDie_Bot (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void BotObituary (edict_t *self, edict_t *inflictor, edict_t *attacker);
void BotIncreaseThrust( edict_t *bot );









// --------------------------
// Bot Spawning and related
// --------------------------



// select a spawnpoint depending on the vehicle the bot has selected
int FindBotSpawnPoint( edict_t *vehicle )
{
	int cat = 0, count = 0, loop, which;
	edict_t *spot = NULL;

	if( strcmp(vehicle->classname, "plane" ) == 0 )
		cat = CAT_PLANE;
	else if( strcmp(vehicle->classname, "helo" ) == 0 )
		cat = CAT_HELO;
	else if( strcmp(vehicle->classname, "ground" ) == 0 )
		cat = CAT_GROUND;
	else if( strcmp(vehicle->classname, "LQM" ) == 0 )
		cat = CAT_LQM;
	else
		gi.error( "Bot chose invalid vehicle!\n" );

	if( deathmatch->value )
	{
		while( (spot = G_Find( spot, FOFS(classname), "info_player_deathmatch" )) != NULL )
		{
			if( (spot->sounds & cat) || (spot->sounds == 0) )
			{
				if( !teamplay->value || (teamplay->value && (vehicle->aqteam == spot->aqteam 
						|| vehicle->aqteam == TEAM_NEUTRAL || spot->aqteam == 0) ) )
				{
					if( strcmp( vehicle->classname, "plane" ) == 0 )
					{
						if( spot->ONOFF & ONOFF_LANDED )
							continue;
						else
							count++;
					}
					else
					{
						count++;
					}
				}
			}
		}
		spot = NULL;
		if( count == 0 )
		{
			gi.cvar_forceset( "num_bots", va("%d", 0) );
			gi.bprintf( PRINT_HIGH, "Not enough bot-spawnpoints found, num_bots reset to 0!\n" );
//			gi.error( "Bot found no spawn points!\n" );
			return 0;
		}
		which = (int)(random()*count + 1);
		for( loop = 0; loop < which; loop ++ )
		{
			spot = G_Find( spot, FOFS(classname), "info_player_deathmatch" );
			if( !(spot->sounds & cat) && (spot->sounds != 0) )
			{
				loop--;
			}
			else
			{
				if( teamplay->value && vehicle->aqteam != spot->aqteam 
					&& vehicle->aqteam != TEAM_NEUTRAL && spot->aqteam != 0 ) 
				{
					loop--;
				}
				else
				{
					if( strcmp( vehicle->classname, "plane" ) == 0 )
					{
						if( spot->ONOFF & ONOFF_LANDED )
							loop--;
					}

				}
			}
		}
	}

	if( spot == NULL )
		gi.error( "No vehicle spawn point available!\n" );

	KillBox( spot );

	VectorCopy( spot->s.origin, vehicle->s.origin );
	VectorCopy( spot->s.angles, vehicle->s.angles );

	return 1;
}


// find a random bot
void FindUnusedBot( edict_t *bot )
{
	int i, j;
	
	if( bots == MAX_BOTS )		// necessary ?
		return;
	
	j = (rand()%(MAX_BOTS-bots))+1;

//	gi.bprintf( PRINT_HIGH, "random: %d \n", j );

	for( i = 0; i < MAX_BOTS; i++ )
	{
		if( botinfo[i].inuse )
			continue;
		j--;
		if( j < 1 )
			break;
	}

	bot->botnumber = i;
	botinfo[i].inuse = true;

//	gi.bprintf( PRINT_HIGH, "selection: %d (%d)\n", i, j );

//	for( i = 0; i < MAX_BOTS; i++ )
//		gi.bprintf( PRINT_HIGH, "%d %s\n", botinfo[i].inuse, botinfo[i].botname );
}

void LoadBotsFromFile()
{
	FILE	*pFile;
	char	string[256], name[16];
	int		count = 0, 
			skill = 0,		
			veh = 111,		// three letter vehicle code (default = F16)
			wep = 1;		// favourite weapon

	if( (pFile = fopen( "airq2/aq2bots.txt", "r" )) != NULL )
	{
		while( fgets( string, 255, pFile ) != NULL )
		{
			if( (string[0] == '/' && string[1] == '/') || string[0] == '\0' )
				continue;
			sscanf( string, "%s %d %d %d", name, &skill, &veh, &wep );
			strncpy( botinfo[count].botname, name, 15 );
			botinfo[count].skill = skill;
			botinfo[count].fav_vehicle = veh;
			botinfo[count].fav_weapon = wep;
			count++;
			if( count == MAX_BOTS )
				break;
		}
		fclose( pFile );
	}
}



// bot shall select one of the planes (helos and ground vehicles will be added later!)
void SelectBotVehicle( edict_t *newbot )
{
	// favourite vehicle is loaded from file, will later be adjusted dependent on 
	// experience of bot;
	SelectVehicleAndSpawnIt(newbot, botinfo[newbot->botnumber].fav_vehicle);

	// additional info that is relevant
	newbot->HASCAN |= HASCAN_DRONE;
	newbot->think = PlaneMovement_Bot;
	newbot->touch = PlaneTouch_Bot;
	newbot->pain = VehiclePain;
	newbot->die = PlaneDie_Bot;
	newbot->ONOFF = ONOFF_AIRBORNE | ONOFF_PILOT_ONBOARD;
	newbot->action = ACTION_NONE;
	newbot->thrust = THRUST_MILITARY;
	newbot->drone_decision_time = level.time + rand()%3+1;
	newbot->delay = level.time + rand()%5+5;
	newbot->enemy = NULL;
	newbot->noise_index = 0;	// abused this variable for machinegun shots
	VectorCopy( newbot->s.angles, newbot->move_angles );
	// for collision detection
	VectorSet( newbot->pos1, newbot->maxs[0]+3, newbot->maxs[1]+3, newbot->maxs[2]+3 );
	VectorSet( newbot->pos2, newbot->mins[0]+3, newbot->mins[1]+3, newbot->mins[2]+3 );
}



// spawn a bot in deathmatch
void SpawnDMBot()
{
	edict_t	*newbot;

	if( bots >= MAX_BOTS )
		return;

	newbot = G_Spawn();

	FindUnusedBot( newbot );

	SelectBotVehicle( newbot );
	
	newbot->owner = NULL;

	if( !FindBotSpawnPoint( newbot ) )
	{
//		gi.error( "No spawn point found for bot!\n" );
		G_FreeEdict( newbot );
		return;
	}
	newbot->count = 0;	// frag counter	
	newbot->nextthink = level.time + 0.1;

	gi.linkentity( newbot );

	gi.bprintf( PRINT_HIGH, "%s entered the game.\n", botinfo[newbot->botnumber].botname );

	// bot counter
	bots++;
}





// spawn a bot in teamplay
void SpawnTeamBot()
{
	edict_t		*newbot;
	static int	team = TEAM_1;

	if( bots >= MAX_BOTS )
		return;

	if( team == TEAM_1 && teams[TEAM_1].bots >= num_bots_team1->value )
		team = TEAM_2;
	if( team == TEAM_2 && teams[TEAM_2].bots >= num_bots_team2->value )
		team = TEAM_NEUTRAL;
	if( team == TEAM_NEUTRAL && teams[TEAM_NEUTRAL].bots >= num_bots_neutral->value )
		team = TEAM_1;

	newbot = G_Spawn();

	if( !newbot->aqteam )
		newbot->aqteam = team;

	FindUnusedBot( newbot );

	SelectBotVehicle( newbot );

	newbot->owner = NULL;

	if( !FindBotSpawnPoint( newbot ) )
	{
//		gi.error( "No spawn point found for bot!\n" );
		G_FreeEdict( newbot );
		return;
	}
	newbot->count = 0;	// frag counter	
	newbot->nextthink = level.time + 0.1;

	gi.linkentity( newbot );

	gi.bprintf( PRINT_HIGH, "%s entered the game. (%d)\n", botinfo[newbot->botnumber].botname, 
		newbot->aqteam );

	// bot counter
	teams[team].bots++;
	team <<= 1;
	if( team > TEAM_NEUTRAL )
		team = TEAM_1;
	bots++;
}




// checked every couple frames to see if new bots need to be spawned
void CheckBotNum()
{
	if( level.time < bot_time )
		return;

	if( !deathmatch->value )
		return;

	if( level.intermissiontime )
		return;

	// teamplay
	if( teamplay->value )
	{
		if( teams[TEAM_1].bots < num_bots_team1->value || teams[TEAM_2].bots < num_bots_team2->value ||
			teams[TEAM_NEUTRAL].bots < num_bots_neutral->value )
			SpawnTeamBot();
	}
	// deathmatch
	else	
	{
		if( bots < num_bots->value )
			SpawnDMBot();
	}
	bot_time = level.time + 2.0 + rand()%4;
}



// respawn a bot after he was dead
void RespawnDMBot( edict_t *bot )
{
	if( level.intermissiontime )
		return;

	SelectBotVehicle( bot );

	if( !FindBotSpawnPoint( bot ) )
	{
		gi.error( "No spawn point found for bot!\n" );
	}

//	gi.bprintf( PRINT_HIGH, "%s respawned\n", bot->botname );

	gi.linkentity( bot );

	bot->nextthink = level.time + 0.1;
}


// respawn a bot after he was dead
void RespawnTeamBot( edict_t *bot )
{
	if( level.intermissiontime )
		return;

	SelectBotVehicle( bot );

	if( !FindBotSpawnPoint( bot ) )
	{
		gi.error( "No spawn point found for bot!\n" );
	}

//	gi.bprintf( PRINT_HIGH, "%s respawned (%d)\n", botinfo[bot->botnumber].botname, 
//			bot->aqteam );

	gi.linkentity( bot );

	bot->nextthink = level.time + 0.1;
}















// --------------------------
// Bot Movement and related
// --------------------------










void PlaneMovement_Bot( edict_t *plane )
{
	vec3_t	above, forward;
	float	turn;
	trace_t trace;
	vec3_t  pos1, pos2, pos3, pos4;
	trace_t traceup, tracedown, traceleft, traceright;
	float	tracedist, diff_h, diff_v, range;
	int		random_decision;
	edict_t	*something = NULL;
	int		VM = (strcmp(vid_ref->string, "gl") == 0 ? 1 : -1);
	
	VectorClear( plane->avelocity );

//	gi.bprintf( PRINT_HIGH, "%s %d\n", plane->botname, plane->aqteam );

	// intermission 
	if( level.intermissiontime )
	{
		plane->solid = SOLID_NOT;
		plane->movetype = MOVETYPE_NONE;
		plane->s.modelindex = 0;
		VectorClear( plane->velocity );
		VectorClear( plane->avelocity );
		return;
	}

	// check for overturning
	plane->s.angles[0] = anglemod( plane->s.angles[0] );
	plane->s.angles[1] = anglemod( plane->s.angles[1] );
	if( plane->s.angles[2] > 180  )
		plane->s.angles[2] -= 360;
	else if( plane->s.angles[2] < -180)
		plane->s.angles[2] += 360;

	PlaneSpeed( plane );

	// health-effects
	if( plane->health >= 26 )
		plane->s.effects = 0;
	else if( plane->health < 26 && plane->health > 10 )
	{
		plane->s.effects = EF_GRENADE;		// "smoking"
	}
	else if( plane->health <= 10 && plane->deadflag != DEAD_DEAD )
	{
		plane->s.effects = EF_ROCKET;		// "burning"
	}
	else
		plane->s.effects = 0;

	if( plane->health <= plane->gib_health && plane->deadflag != DEAD_DEAD )
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (plane->s.origin);
		gi.multicast (plane->s.origin, MULTICAST_PVS);
		SetVehicleModel(plane);
		plane->deadflag = DEAD_DEAD;
	}
	// stop sound on death
	if( plane->deadflag == DEAD_DEAD )
	{
		gi.sound(plane, CHAN_BODY, gi.soundindex( "death/0.wav" ), 1, ATTN_NORM, 0 );
		return;
	}
	// spin dying plane
	else if( plane->deadflag == DEAD_DYING )
	{
		plane->avelocity[2] = (plane->avelocity[2]>=0) ? 60 : -60;
		vectoangles( plane->velocity, pos1 );
		plane->s.angles[0] = pos1[0];
		plane->s.angles[1] = pos1[1];
		plane->gravity = 0.1;
		plane->nextthink = level.time + 0.1;
		return;
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

	// Artificial Intelligence State Machine: (AISM)
	// 1.avoid terrain
	// 2.avoid vehicles
	// 3.avoid incoming weapon
	// 4.depending on aggression: a.avoid lock
	//							  b.chase targets
	// 5.vice versa
	// 6.follow your way (waypoints, just look, go to base, patrol, follow friends etc)
	// independently apply throttle to maintain speed above stall speed,
	// use higher speed when chasing targets -> if distance growing then
	// apply throttle, watch for too high closure rates, when within certain
	// range reduce throttle, if necessary apply brakes

	// initialize targetangles to current heading
	if( !plane->action )
		VectorCopy( plane->s.angles, plane->move_angles );

	// tracing, ie scanning the terrain for CLOSE obstacles
	tracedist = plane->speed * 2.1;
	VectorCopy( plane->s.angles, pos1 );
	if( plane->s.angles[0] < 90 || plane->s.angles[0] > 270 )
		pos1[0] -= 30;
	else
		pos1[0] += 30;
	VectorCopy( plane->s.angles, pos2 );
	if( plane->s.angles[0] < 90 || plane->s.angles[0] > 270 )
		pos2[0] += 30;
	else
		pos2[0] -= 30;
	VectorCopy( plane->s.angles, pos3 );
	pos3[1] += 40;
	VectorCopy( plane->s.angles, pos4 );
	pos4[1] -= 40;
	AngleVectors( plane->s.angles, forward, NULL, NULL );
	VectorMA( plane->s.origin, tracedist, forward, forward );
	trace = gi.trace( plane->s.origin, plane->pos2, plane->pos1, forward, plane, MASK_PLAYERSOLID|MASK_WATER );

	AngleVectors( pos1, forward, NULL, NULL );
	VectorMA( plane->s.origin, tracedist, forward, forward );
	traceup = gi.trace( plane->s.origin, NULL, NULL, forward, plane, MASK_PLAYERSOLID|MASK_WATER );

	AngleVectors( pos2, forward, NULL, NULL );
	VectorMA( plane->s.origin, tracedist, forward, forward );
	tracedown = gi.trace( plane->s.origin, NULL, NULL, forward, plane, MASK_PLAYERSOLID|MASK_WATER );

	AngleVectors( pos3, forward, NULL, NULL );
	VectorMA( plane->s.origin, tracedist, forward, forward );
	traceleft = gi.trace( plane->s.origin, NULL, NULL, forward, plane, MASK_PLAYERSOLID|MASK_WATER );

	AngleVectors( pos4, forward, NULL, NULL );
	VectorMA( plane->s.origin, tracedist, forward, forward );
	traceright = gi.trace( plane->s.origin, NULL, NULL, forward, plane, MASK_PLAYERSOLID|MASK_WATER );

	if( trace.fraction < 0.6 || traceleft.fraction < 0.1 || traceright.fraction < 0.1 )
	{
		plane->action |= ACTION_AVOID_TERRAIN_FORCED;
	}

	// avoiding terrain horizontal 
	if( ( (trace.fraction < 0.8 || traceleft.fraction <= 0.2 || traceright.fraction <= 0.2) &&
		!(plane->action & ACTION_AVOID_TERRAIN) ) || 
		(plane->action & ACTION_AVOID_TERRAIN_FORCED) )
	{
		VectorCopy( plane->s.angles, plane->move_angles );
		if( trace.fraction > 0.7 )
			turn = 8;
		else if( trace.fraction > 0.2 )
			turn = 14;
		else
			turn = 20;
		plane->action |= ACTION_AVOID_TERRAIN;

//		gi.bprintf( PRINT_HIGH, "AVOID!\n" );
		if( traceleft.fraction > traceright.fraction )
		{
			plane->move_angles[1] += turn;
			if( (traceup.fraction > traceleft.fraction ||
					tracedown.fraction > traceleft.fraction) && traceleft.fraction < 0.4 )
			{
				if( !(plane->action & ACTION_AVOID_TERRAIN_VERTICAL ) )
					plane->action |= ACTION_AVOID_TERRAIN_VERTICAL_FORCED;
			}
		}
		else
		{
			plane->move_angles[1] -= turn;
			if( (traceup.fraction > traceright.fraction ||
					tracedown.fraction > traceright.fraction) && traceright.fraction < 0.4 )
			{
				if( !(plane->action & ACTION_AVOID_TERRAIN_VERTICAL ) )
					plane->action |= ACTION_AVOID_TERRAIN_VERTICAL_FORCED;
			}
		}
		if( trace.fraction < 0.6 && traceleft.fraction < 0.5 && traceright.fraction < 0.5
			&& plane->speed > plane->stallspeed + 40 )
		{
			plane->thrust--;
			plane->ONOFF |= ONOFF_BRAKE;
		}
		plane->action &= ~ACTION_AVOID_TERRAIN_FORCED;
		plane->action &= ~ACTION_RANDOM;
		plane->action &= ~ACTION_CHASE_TARGET;
		plane->action &= ~ACTION_HIT_BY_SOMEONE;
		plane->drone_decision_time = level.time + rand()%2+2;
	}

	// avoid terrain vertical
	if( ((traceup.fraction < 0.3 || tracedown.fraction < 0.3) && 
			!(plane->action & ACTION_AVOID_TERRAIN_VERTICAL)) || 
			(plane->action & ACTION_AVOID_TERRAIN_VERTICAL_FORCED) )
	{
//		gi.bprintf( PRINT_HIGH, "AVOID VERTICAL! %s\n", 
//			(plane->action & ACTION_AVOID_TERRAIN_VERTICAL_FORCED)?"(FORCED)":"");
		plane->move_angles[0] = plane->s.angles[0];
		plane->action |= ACTION_AVOID_TERRAIN_VERTICAL;
		if( tracedown.fraction > traceup.fraction )
		{
			if( tracedown.fraction >= 0.5 )
				plane->move_angles[0] += 15;
			else
				plane->move_angles[0] += 30;
		}
		else
		{
			if( traceup.fraction >= 0.5 )
				plane->move_angles[0] -= 15;
			else
				plane->move_angles[0] -= 30;
		}
		plane->action &= ~ACTION_AVOID_TERRAIN_VERTICAL_FORCED;
		plane->action &= ~ACTION_RANDOM;
		plane->action &= ~ACTION_CHASE_TARGET;
		plane->action &= ~ACTION_HIT_BY_SOMEONE;
		plane->drone_decision_time = level.time + rand()%2+2;
	}

	// avoiding stalls
	if( plane->speed < plane->stallspeed + 20 )
	{
		if( plane->thrust < plane->maxthrust )
			BotIncreaseThrust( plane );
		if( !(plane->DAMAGE & DAMAGE_BRAKE) )
			plane->ONOFF &= ~ONOFF_BRAKE;
		plane->action = ACTION_AVOID_STALL;
//		gi.bprintf( PRINT_HIGH, "%s thrust:%d\n", plane->botname, plane->thrust );
	}
	else
	{
		plane->action &= ~ACTION_AVOID_STALL;
	}

	if( !(plane->action) || (plane->action & ACTION_CHASE_TARGET) ||
		(plane->action & ACTION_HIT_BY_SOMEONE) )
	{
		// find enemy
		if( !plane->enemy )
		{
			while ((plane->enemy = findradius(plane->enemy, 
					plane->s.origin, 2000)) != NULL)			
			{
				if( plane->enemy == plane )
					continue;
				if( strcmp( plane->enemy->classname, "plane") != 0 &&
					strcmp( plane->enemy->classname, "helo") != 0 &&
					strcmp( plane->enemy->classname, "ground") != 0 &&
					strcmp( plane->enemy->classname, "LQM") != 0 )
					continue;
				else
				{
					if( strcmp( plane->enemy->classname, "LQM" ) == 0 && !bots_attack_LQM->value )
						continue;
					else if( !(plane->enemy->ONOFF & ONOFF_AIRBORNE) && !bots_attack_landed->value 
						&& ( strcmp( plane->enemy->classname, "plane" ) == 0 
						|| strcmp( plane->enemy->classname, "helo" ) == 0 ) )
						continue;
					if( teamplay->value )
					{
						if( (plane->aqteam == plane->enemy->aqteam) ||
							(plane->aqteam == TEAM_NEUTRAL) || 
							(plane->enemy->aqteam == TEAM_NEUTRAL) )
						continue;
					}
					if( !infront( plane, plane->enemy ) )
						continue;
					if( !visible( plane, plane->enemy ) )
						continue;
					if( plane->enemy->deadflag )
						continue;
					plane->distance = 0;
//					gi.bprintf( PRINT_HIGH, "Enemy: %s %s\n", plane->enemy->classname,
//							plane->enemy->owner->client->pers.netname );
					break;
				}
			}
		}
		// hunt enemy
		else
		{
			vec3_t targdir;
			float  rate = 0;

			if( plane->enemy->deadflag )
			{
				plane->enemy = NULL;
				VectorCopy( plane->s.angles, plane->move_angles );
				plane->action &= ~ACTION_CHASE_TARGET;
			}
			else if( (strcmp( plane->enemy->classname, "plane" ) == 0 ||
					strcmp( plane->enemy->classname, "helo" ) == 0) &&
					!(plane->enemy->ONOFF & ONOFF_AIRBORNE) && !bots_attack_landed->value )
			{
				plane->enemy = NULL;
				VectorCopy( plane->s.angles, plane->move_angles );
				plane->action &= ~ACTION_CHASE_TARGET;
			}
			else if( strcmp( plane->enemy->classname, "LQM" ) == 0 && !bots_attack_LQM->value )
			{
				plane->enemy = NULL;
				VectorCopy( plane->s.angles, plane->move_angles );
				plane->action &= ~ACTION_CHASE_TARGET;
			}
			else
			{
				if( !visible( plane, plane->enemy ) )
				{
					plane->enemy = NULL;
					VectorCopy( plane->s.angles, plane->move_angles );
				}
				if( plane->enemy )
				{
					VectorSubtract( plane->enemy->s.origin, plane->s.origin, targdir );
					range = VectorLength( targdir );
					rate = plane->distance - range;
					vectoangles( targdir, plane->move_angles );
					plane->action |= ACTION_CHASE_TARGET;
					
					// dependent on distance find the decision wheter to accel/decel
					if( strcmp( plane->enemy->classname, "plane" ) == 0 )
					{
						if( range > 700 )
						{
							BotIncreaseThrust( plane );
							if( !(plane->DAMAGE & DAMAGE_BRAKE) )
								plane->ONOFF &= ~ONOFF_BRAKE;
						}
						else
						{
							if( rate > 20 )
							{
								plane->thrust = 0;
								if( !(plane->DAMAGE & DAMAGE_BRAKE) )
										plane->ONOFF |= ONOFF_BRAKE;
								// emergency maneuver - do not collide with enemy!
								if( (range < 450 && rate > 25) || range < 300 )
								{
									if( traceright.fraction >= 0.9 )
										VectorSet( plane->move_angles, plane->s.angles[0], plane->s.angles[1]-90, 0 );
									else if( traceleft.fraction >= 0.9 )
										VectorSet( plane->move_angles, plane->s.angles[0], plane->s.angles[1]+90, 0 );
									plane->action &= ~ACTION_CHASE_TARGET;
									plane->action |= ACTION_AVOID_TERRAIN;
								}
							}
							else if( rate > 13)
							{
								if( plane->thrust > 0 )
									plane->thrust--;
								if( !(plane->DAMAGE & DAMAGE_BRAKE) )
										plane->ONOFF |= ONOFF_BRAKE;										
							}
							else if( rate > 6 )
							{
								if( plane->thrust > 0 )
									plane->thrust--;
								else
								{
									if( !(plane->DAMAGE & DAMAGE_BRAKE) )
										plane->ONOFF |= ONOFF_BRAKE;										
								}
							}
							else if( rate < -6 )
							{
								BotIncreaseThrust( plane );
								if( !(plane->DAMAGE & DAMAGE_BRAKE) )
									plane->ONOFF &= ~ONOFF_BRAKE;
							}
						}
					}

					// if range is closer than a certain distance get slower
					// if it is farther accelerate
					// use a variable for closure rate and use it for example
					// for emergency break, in case it almost rams you!
					// ie if range is close and closure rate high then avoid collision

					// test formation flight!!! use range and closure rate
//					if( strcmp( plane->botname, "Falcon" ) == 0 )
//						gi.bprintf( PRINT_HIGH, "range: %.1f cr:%.1f\n", range, plane->distance-range );
					plane->distance = range;
				}
				else
					plane->action &= ~ACTION_CHASE_TARGET;
			}
			plane->action &= ~ACTION_RANDOM;
			plane->action &= ~ACTION_HIT_BY_SOMEONE;
		}
	}

	// check 6
		// if you don't have an enemy look behind you, maybe someone sneaks up behind you!
		// but you need to do this only in close distance, because if farther away he cant
		// shoot anyways, if he tries to fire a missile, you see it on your lock indicator and
		// if he fires a autocannon you see the tracers...
		// this is only useful for planes, as the other categories cannot follow you
	if( !plane->enemy && !plane->action && level.time > plane->delay )
	{
		while ((plane->enemy = findradius(plane->enemy, 
				plane->s.origin, 1500)) != NULL)			
		{
			if( plane->enemy == plane )
				continue;
			if( strcmp( plane->enemy->classname, "plane") != 0 )
				continue;
			else
			{
				if( !(plane->enemy->ONOFF & ONOFF_AIRBORNE) && !bots_attack_landed->value )
					continue;
				if( teamplay->value )
				{
					if( (plane->aqteam == plane->enemy->aqteam) ||
						(plane->aqteam == TEAM_NEUTRAL) || 
						(plane->enemy->aqteam == TEAM_NEUTRAL) )
					continue;
				}
				// this time dont look infront
				if( infront( plane, plane->enemy ) )
					continue;
				// cant see this one anyways
				if( !visible( plane, plane->enemy ) )
					continue;
				// dead ones are not interesting
				if( plane->enemy->deadflag )
					continue;
				// reset distance
				plane->distance = 0;
//				gi.bprintf( PRINT_HIGH, "Check 6 found enemy\n" );
				// change this to be dependent on experience
				plane->delay = level.time + 10;
				break;
			}
		}
	}

	// avoid being killed
		// incoming missiles
	if( plane->lockstatus == LS_LAUNCH )
		Fire_Flare( plane );
/*	if( !(plane->action & ACTION_TERRAIN) && plane->lockstatus )
	{
		// only a lock, so find your enemy -> turn around and look into his eyes
		if( plane->lockstatus == LS_LOCK )
		{
			plane->action |= ACTION_TERRAIN;	// important!
			plane->move_angles[1] += (traceleft.fraction > traceright.fraction ? 179 : -179 );
		}
		// A LAUNCH! EVADE!
		else 
		{
			while ((plane->target_ent = findradius(plane->enemy, 
					plane->s.origin, 1000)) != NULL)			
			{
				if( plane->target_ent == plane )
					continue;
				if( strcmp( plane->target_ent->classname, "missile") != 0 )
					continue;
				else
				{
					// cant see this one, so it cant see you -> forget it
					if( !visible( plane, plane->enemy ) )
						continue;
					// dead ones are not interesting, already shot down -> forget it
					if( plane->enemy->deadflag )
						continue;
					// that's it! he is seeking us
					if( plane->target_ent->enemy == plane )
					{
						// spit out as many flares as possible
						Fire_Flare( plane );
						// use pos1 vector as it is no more needed here
						VectorSubtract( plane->target_ent->s.origin, plane->s.origin, pos1 );
						range = VectorLength( pos1 );
						// close distance -> make a hard turn and climb/descend, this should shake it off
						if( range < 400 )
						{
							plane->move_angles[0] += (tracedown.fraction > traceup.fraction ? 50 : -50 );
							plane->move_angles[1] += (traceleft.fraction > traceright.fraction ? 179 : -179 );
						}
						// farther away -> accelerate and dont brake to have more time
						else
						{
							BotIncreaseThrust( plane );
							if( !(plane->DAMAGE & DAMAGE_BRAKE) )
								plane->ONOFF &= ~ONOFF_BRAKE;
						}
					}
					break;
				}
			}

			plane->action |= ACTION_TERRAIN;	// important!
			plane->move_angles[1] += (traceleft.fraction > traceright.fraction ? 179 : -179 );

		}
	}*/
		// being hit by something -> determined in g_combat.c
	if( (plane->action & ACTION_HIT_BY_SOMEONE_FORCED) && 
		!(plane->action & ACTION_TERRAIN ) && !(plane->action & ACTION_CHASE_TARGET) )
	{
//		gi.bprintf( PRINT_HIGH, "hit\n" );
		if( traceright.fraction > 0.9 )
			VectorSet( plane->move_angles, plane->s.angles[0], plane->s.angles[1]-170, 0 );
		else if( traceleft.fraction > 0.9 )
			VectorSet( plane->move_angles, plane->s.angles[0], plane->s.angles[1]+170, 0 );
		plane->action &= ~ACTION_HIT_BY_SOMEONE_FORCED;
		plane->action |= ACTION_HIT_BY_SOMEONE;
	}

	// random decision
	if( !plane->action && level.time > plane->drone_decision_time )
	{
		plane->action |= ACTION_RANDOM;
		random_decision = rand()%6;
		switch( random_decision )
		{
		case 0:
			if( tracedown.fraction >= 1.0 && ( plane->s.angles[0] < 40 ||
					plane->s.angles[0] > 270 ) )
			{
				plane->move_angles[0] += (rand()%20+10);
//				gi.bprintf( PRINT_HIGH, "Random down!\n" );
			}
			break;
		case 1:
			if( traceup.fraction >= 1.0 && ( plane->s.angles[0] > 320 ||
					plane->s.angles[0] < 90 ) )
			{
				plane->move_angles[0] -= (rand()%20+10);
//				gi.bprintf( PRINT_HIGH, "Random up!\n" );
			}
			break;
		case 2:
			if( traceleft.fraction >= 1.0 )
			{
				plane->move_angles[1] += (rand()%120+30);
//				gi.bprintf( PRINT_HIGH, "Random left!\n" );
			}
			break;
		case 3:
			if( traceright.fraction >= 1.0 )
			{
				plane->move_angles[1] -= (rand()%120+30);
//				gi.bprintf( PRINT_HIGH, "Random right!\n" );
			}
			break;
		case 4:
			if( plane->speed > plane->stallspeed + 50 )
				plane->thrust --;
			if( plane->thrust < 0 )
				plane->thrust = 0;
//			gi.bprintf( PRINT_HIGH, "Random slow!\n" );
			break;
		case 5:
			BotIncreaseThrust( plane );
//			gi.bprintf( PRINT_HIGH, "Random fast!\n" );
			break;
		}
		plane->drone_decision_time = level.time + rand()%2+2;
	}

	// find waypoints (ie important targets, own base etc)
	// later this will also work for dynamic waypoints
/*	if( !plane->action )
	{
		// already has a waypoint
		if( plane->goalentity )
		{
			// again use pos1 and pos2 for calcs
			VectorSubtract( plane->goalentity->s.origin, plane->s.origin, pos1 );
			VectorSet( pos2, pos1[0], pos1[1], 0 );
			range = VectorLength( pos2 );
			vectoangles( pos1, plane->move_angles );
			// if close to waypoint switch to next waypoint (horizontal distance!)
			if( range < 64 )
			{
				if( plane->goalentity->target )
				{
					plane->goalentity = G_Find( NULL, FOFS( targetname ), plane->goalentity->target );
				}
			}
		}
		// no doesnt have one, get one
		else
		{
			// which one to get
				// healthy -> find enemy base or other target (only good bots try that)
			if( plane->health >= plane->max_health - 20 && plane->skill >= 3 )
			{
				FindWaypoint( plane->goalentity, WP_ENEMY_BASE|WP_ENEMY_STRUCTURE );
			}
				// weak bots are cowards and prefer to retreat, damages bots also retreat
			else
			{
				FindWaypoint( plane->goalentity, WP_OWN_BASE|WP_OWN_STRUCTURE );
			}
		}
	}*/

	// watch out for potential targets (but only if you are not avoiding terrain,
	// because humans cant look while trying to survive, too...)
/*	if( !(plane->action & ACTION_TERRAIN) )
	{
		// if you see something interesting "report" it
		// ie put it in the list of interesting targets
		// this time look in all directions (bot "uses freelook")
		while ((something = findradius(plane->enemy, 
				plane->s.origin, 2500)) != NULL)			
		{
			if( something == plane )
				continue;
			if( strcmp( something->classname, "func_explosive") != 0 )
				continue;
			else
			{
				// cant see this 
				if( !visible( plane, something ) )
					continue;
				for( loop = 0; loop < MAX_WAYPOINTS; loop ++ )
				{
					if( !(important_spots[loop]->inuse) )
					{
						important_spots[loop]->style = WP_ENEMY_STRUCTURE;
						break;
					}
				}
//					gi.bprintf( PRINT_HIGH, "Enemy: %s %s\n", plane->enemy->classname,
//							plane->enemy->owner->client->pers.netname );
				break;
			}
		}
	}*/

	// avoiding overspeed
	if( !plane->action )
	{
		if( plane->speed > plane->topspeed + 100 )
		{
			if( plane->thrust > 0 )
				plane->thrust--;
			if( !(plane->DAMAGE & DAMAGE_BRAKE) )
				plane->ONOFF |= ONOFF_BRAKE;
//			gi.bprintf( PRINT_HIGH, "%s thrust:%d\n", plane->botname, plane->thrust );
		}
		else if( plane->speed > plane->topspeed + 20 )
		{
			if( plane->thrust > 0 )
				plane->thrust--;
//			gi.bprintf( PRINT_HIGH, "%s thrust:%d\n", plane->botname, plane->thrust );
		}
		else
		{
			if( !(plane->DAMAGE & DAMAGE_BRAKE) )
				plane->ONOFF &= ~ONOFF_BRAKE;
		}
	}



//	gi.bprintf( PRINT_HIGH, "A:%d TH:%d D:%d P:%.1f(%.1f) Y:%.1f(%.1f) R:%.1f\n", plane->action, plane->thrust,
//			plane->DAMAGE, plane->s.angles[0], plane->move_angles[0], plane->s.angles[1], plane->move_angles[1], 
//			plane->s.angles[2] );

//	gi.bprintf( PRINT_HIGH, "%.1f u:%.1f d:%.1f l:%.1f r:%.1f\n", trace.fraction,
//		traceup.fraction, tracedown.fraction, traceleft.fraction, traceright.fraction );


	// movement in the air
	if( plane->ONOFF & ONOFF_AIRBORNE )
	{
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
		else
		{
			// apply rudder and aileron
			plane->move_angles[1] = anglemod( plane->move_angles[1] );

			plane->elevator = plane->turnspeed[0];
			if( plane->speed > plane->stallspeed )
			{
				plane->elevator *= (float)(plane->stallspeed/(plane->speed*1.2));
			}

			diff_h = plane->move_angles[1]-plane->s.angles[1];
			if( diff_h > 180 )
				diff_h -= 360;
			else if( diff_h < -180 )
				diff_h += 360;
			if( fabs(diff_h) >= 5 )
			{
				if( fabs(diff_h) > 20 )
					plane->avelocity[1] = (diff_h>0) ? plane->elevator : -plane->elevator;
				else
				{
					if( fabs(plane->elevator) >= 70 )
						plane->avelocity[1] = (diff_h>0) ? 70 : -70;
					else
						plane->avelocity[1] = (diff_h>0) ? plane->elevator : -plane->elevator;
				}
				if( diff_h > 0 && plane->s.angles[2] * VM < 85)
					plane->avelocity[2] = plane->turnspeed[2] * VM;
				else if( diff_h < 0 && plane->s.angles[2] * VM > -85)
					plane->avelocity[2] = -plane->turnspeed[2] * VM;
				else
					plane->avelocity[2] = 0;
			}
			else
			{
				plane->s.angles[1] = plane->move_angles[1];
				if( plane->s.angles[0] > 90 && plane->s.angles[0] < 270 )
				{
					if( fabs(plane->s.angles[2]) < 170 )
						plane->avelocity[2] = (plane->s.angles[2]>0) ? 60: -60;
					else if( fabs(plane->s.angles[2]) < 173 )
						plane->avelocity[2] = (plane->s.angles[2]>0) ? 30: -30;
				}
				else
				{
					if( fabs(plane->s.angles[2]) > 10 )
						plane->avelocity[2] = (plane->s.angles[2]>0) ? -60 : 60;
					else if( fabs(plane->s.angles[2]) > 3 )
						plane->avelocity[2] = (plane->s.angles[2]>0) ? -30 : 30;
				}
				if( plane->action & ACTION_AVOID_TERRAIN )
					plane->action &= ~ACTION_AVOID_TERRAIN;
				if( plane->move_angles[0] == plane->s.angles[0] )
				{
					plane->action &= ~ACTION_HIT_BY_SOMEONE;
					plane->action &= ~ACTION_RANDOM;
				}
			}
			// apply elevator
			if( plane->move_angles[0] < 0 )
				plane->move_angles[0] += 360;
			else if( plane->move_angles[0] >= 360 )
				plane->move_angles[0] -= 360;
			diff_v = plane->move_angles[0]-plane->s.angles[0];
			if( plane->s.angles[0] > 90 && plane->s.angles[0] < 270 )
				diff_v *= -1;
			if( diff_v > 180 )
				diff_v -= 360;
			else if( diff_v < -180 )
				diff_v += 360;
			if( fabs(diff_v) >= 5 )
			{
				if( diff_v > 0 )
					plane->avelocity[0] = 40;
				else
					plane->avelocity[0] = -40;
			}
			else
			{
				plane->s.angles[0] = plane->move_angles[0];
				if( plane->action & ACTION_AVOID_TERRAIN_VERTICAL )
					plane->action &= ~ACTION_AVOID_TERRAIN_VERTICAL;
				if( plane->move_angles[1] == plane->s.angles[1] )
				{
					plane->action &= ~ACTION_HIT_BY_SOMEONE;
					plane->action &= ~ACTION_RANDOM;
				}
			}
			// stall when too slow
			if( plane->speed < plane->stallspeed )
				plane->ONOFF |= ONOFF_STALLED;
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
					plane->move_angles[0] = plane->s.angles[0];
				}
			}	
			// get altitude 
			VectorCopy( plane->s.origin, above );
			above[2] -= 2000;
			trace = gi.trace( plane->s.origin, NULL, NULL, above, plane, MASK_SOLID );
			if( trace.fraction < 1 )
				plane->altitude = ((int)(plane->s.origin[2] - trace.endpos[2]))-12;
			else
				plane->altitude = 9999;
			// is in water ?
			if( plane->waterlevel )
			{
				VectorClear( plane->velocity );
				T_Damage( plane, world, world, vec3_origin, plane->s.origin, vec3_origin, 9999, 0, 0, MOD_SPLASH );
			}
			// attack if possible
			if( plane->enemy && (plane->action & ACTION_CHASE_TARGET) )
			{
				if( fabs(diff_h) < 5 && fabs(diff_v) < 5 )
				{
					Weapon_Autocannon_Fire( plane );
				}
				else if( fabs(diff_h) < 25 && fabs(diff_v) < 25 )
				{
					// use missiles
				}
				// if enemy is ground then try to reach safe altitude
				// and reduce diff_h to min, and ignore diff_v !! Use bombs!
				// write algorithm to find optimal weapon for present situation
				// aerial target: phoenix, amraam, sidewinder, autocannon,
				//					but autocannon also when close and within cone!
				// ground target: depending on max_health of target (ie how good
				//					is its armour) use different weapons, for LQM
				//					autocannon is ok, but when closer than a certain
				//					range then climb and use bombs, maybe also a rocket!
			}
		}
	}
//	debugging:
//	if( difference )
//		gi.bprintf( PRINT_HIGH, "%f\n", difference );
//	gi.bprintf( PRINT_HIGH, "waypoint: %s\n", drone->enemy->targetname );

	// lock weapons
/*	if( plane->weapon[plane->active_weapon] == WEAPON_SIDEWINDER ||
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
	}*/

	if( (plane->HASCAN & HASCAN_STEALTH) &&
		!(plane->ONOFF & ONOFF_WEAPONBAY) && 
		!(plane->ONOFF & ONOFF_GEAR) )
		plane->ONOFF |= ONOFF_STEALTH;
	else
		plane->ONOFF &= ~ONOFF_STEALTH;

	// set velocity vector
	AngleVectors(plane->s.angles, forward, NULL, NULL );
	VectorScale( forward, plane->speed , plane->velocity );

	plane->nextthink = level.time + 0.1;

}





void PlaneTouch_Bot( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
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





void PlaneDie_Bot (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
//	if( !self->deadflag )
//		gi.bprintf( PRINT_HIGH, "%s dies!\n", self->botname );

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
			if( !self->deadflag )
				BotObituary( self, inflictor, attacker );
			self->deadflag = DEAD_DEAD;
		}
		else
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			if( !self->deadflag )
				BotObituary( self, inflictor, attacker );
			self->deadflag = DEAD_DYING;
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
		BotObituary( self, inflictor, attacker );
		self->deadflag = DEAD_DEAD;
	}

	self->s.frame = 0;
	gi.unlinkentity( self );
	if( self->deadflag == DEAD_DEAD )
	{
		if( !teamplay->value )
		{
			if( bots <= num_bots->value )
			{
				self->s.modelindex = 0;
				self->think = RespawnDMBot;
			}
			else
			{
				self->think = G_FreeEdict;
				botinfo[self->botnumber].inuse = false;
				bots--;
			}
		}
		else if( teamplay->value && num_bots)
		{
			if( (self->aqteam == TEAM_1 && teams[TEAM_1].bots <= num_bots_team1->value) ||
				(self->aqteam == TEAM_2 && teams[TEAM_2].bots <= num_bots_team2->value) ||
				(self->aqteam == TEAM_NEUTRAL && teams[TEAM_NEUTRAL].bots <= num_bots_neutral->value) )
			{
				self->s.modelindex = 0;
				self->think = RespawnTeamBot;
			}
			else
			{
				self->think = G_FreeEdict;
				bots--;
				botinfo[self->botnumber].inuse = false;
				if( teamplay->value )
					teams[self->aqteam].bots--;
			}
		}
		self->nextthink = level.time + rand() % 6 + 1.5;
	}
}

void BotObituary (edict_t *self, edict_t *inflictor, edict_t *attacker)
{
	int		mod = meansOfDeath;
	char	*message = "", *message2 = "", *message3 = "";

// CLEAN THIS UGLY BITCH OF CODE UP!

	switch( mod )
	{
	case MOD_AUTOCANNON:	// autocannon gives an additional frag
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
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_SIDEWINDER:
		message = "was blown apart by";
		message3 = "'s Sidewinder";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_STINGER:		
		message = "was killed by";
		message3 = "'s Stinger";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_AMRAAM:	
		message = "was hunted down by";
		message3 = "'s AMRAAM";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_PHOENIX:		
		message = "couldn't outrun";
		message3 = "'s Phoenix";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_HELLFIRE:	
		message = "was eliminated by";
		message3 = "'s Hellfire";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_MAVERICK:	
		message = "was turned inside out by";
		message3 = "'s Maverick";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_ANTIRADAR:	
		message = "doesn't emit much after contact with";
		message3 = "'s AntiRadar Missile";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_ROCKET:
		message = "was shredded";
		message3 = "'s Rocket";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_LASERBOMBS:
		message = "now knows that this red dot was";
		message3 = "'s LGB";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_RUNWAYBOMBS:
		message = "got to know";
		message3 = "'s Anti-Runway Bombs";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	case MOD_BOMBS:
		message = "ate";
		message3 = "'s Iron Bombs";
		if( attacker->HASCAN & HASCAN_DRONE )
		{
			message2 = botinfo[attacker->botnumber].botname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->count += 4;
		}
		else
		{
			message2 = attacker->owner->client->pers.netname;
			if( !(self->HASCAN & HASCAN_SCORED) )
				attacker->owner->client->resp.score += 4;
		}
		break;
	default:
		message = "died";
		message2 = "";
		message3 = "";
		break;
	}

	self->HASCAN |= HASCAN_SCORED;

	gi.bprintf( PRINT_HIGH, "%s %s %s%s\n", botinfo[self->botnumber].botname, 
			message, message2, message3 );
}



void BotIncreaseThrust( edict_t *bot )
{
	int maxpossible = bot->maxthrust;

	if( bot->DAMAGE & DAMAGE_ENGINE_20 )
		maxpossible = 2;
	else if( bot->DAMAGE & DAMAGE_ENGINE_40 )
		maxpossible = 4;
	else if( bot->DAMAGE & DAMAGE_ENGINE_60 )
		maxpossible = 6;
	else if( bot->DAMAGE & DAMAGE_ENGINE_80 )
		maxpossible = 8;
	else if( bot->DAMAGE & DAMAGE_AFTERBURNER )
		maxpossible = THRUST_MILITARY;

	if( bot->thrust < maxpossible )
		bot->thrust++;

//	if( maxpossible < bot->maxthrust )
//		gi.bprintf( PRINT_HIGH, "Bots engine damaged! (%d)\n", maxpossible );
}


