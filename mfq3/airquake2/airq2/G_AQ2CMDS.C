// file g_aq2cmds.c by Bjoern Drabeck
// this handles commands that are AirQuake2 specific

#include "g_local.h"
#include "drone.h"
#include "g_aq2cmds.h"


void Cmd_VehicleSelect_f ( edict_t *ent )
{
	vec3_t	forward,up;
	int		found;
	int		selectionnumber;
	int		joined = 0;

	if( ent->client->veh_selected )
		return;

	if( ent->sel_step == 0 )
	{
		ent->sel_step = 1;
		return;
	}
	else if( ent->sel_step == 1 )
	{
		ent->sel_step = 2;
		ent->veh_class = 1;
		return;
	}
	else if( ent->sel_step == 2 )
	{
		ent->sel_step = 3;
		if( ent->veh_category & 1 )
		{
			switch( ent->veh_class )
			{
				case 1:
						ent->veh_max = PLANE_FIGHTER;
						break;
				case 2:
						ent->veh_max = PLANE_GROUNDATTACK;
						break;
				case 3:
						ent->veh_max = PLANE_TRANSPORT;
						break;
			}
		}
		else if( ent->veh_category & 2 )
		{
			switch( ent->veh_class )
			{
				case 1:
						ent->veh_max = HELO_ATTACK;
						break;
				case 2:
						ent->veh_max = HELO_SCOUT;
						break;
				case 3:
						ent->veh_max = HELO_TRANSPORT;
						break;
			}
		}
		else if( ent->veh_category & 4 )
		{
			switch( ent->veh_class )
			{
				case 1:
						ent->veh_max = GROUND_MBT;
						break;
				case 2:
						ent->veh_max = GROUND_SCOUT;
						break;
				case 3:
						ent->veh_max = GROUND_APC;
						break;
				case 4:
						ent->veh_max = GROUND_SPECIAL;
						break;
			}
		}
		else if( ent->veh_category & 8 )
		{
			switch( ent->veh_class )
			{
				case 1:
						ent->veh_max = INF_PILOT;
						break;
				case 2:
						ent->veh_max = INF_TEAM;
						break;
			}
		}
		ent->veh_number = 1;
		if( teamplay->value == 1 )
		{
			if( ent->client->pers.team == 2 || ent->client->pers.team == 4 )
				ent->veh_number = ent->veh_max * 0.5 + 1;
		}
		Get_Vehicle_Names_f( ent );
		return;
	}

	if( !ent->client->breather_sound )
	{
		found = SelectVehicleStart( ent );

		if ( !found )
		{
			gi.cprintf( ent, PRINT_HIGH, "No spawn points available!\n" );
			return;
		}
	}
	else
		ent->client->breather_sound = 0;	// shows if you left vehicle

	ent->client->veh_selected = 1;
	if( ent->client->pers.team > 2 )
	{
		ent->client->pers.team -= 2;
		joined = 1;
	}

	// spawning the vehicle
	ent->vehicle = G_Spawn ();

	VectorCopy( ent->s.origin, ent->vehicle->s.origin );
	VectorSet( ent->vehicle->s.angles, 0, ent->s.angles[1], 0 );

	ent->vehicle->ONOFF = NOTHING_ON;
	ent->vehicle->HASCAN = HAS_NOTHING;
	ent->vehicle->DAMAGE = NO_DAMAGE;

	selectionnumber = ent->veh_category * 100 + ent->veh_class * 10 + ent->veh_number;

	switch( selectionnumber )
	{
		// AUTOCANNON-MAINGUN-SW-ST-AM-PH-ATGM-MAVERICKS-ROCKETS-BOMBS-LGBS-FLARES-JAMMER
		case 111:
				Start_F14( ent->vehicle );
				Start_Client( ent, 500, 0, 4, 0, 0, 4, 0, 0, 0, 0, 0, 35, 0 );
				checkifonrunway( ent );
				break;
		case 112:
				Start_F22 ( ent->vehicle );
				Start_Client( ent, 200, 0, 2, 0, 4, 0, 0, 0, 0, 0, 0, 30, 0 );
				checkifonrunway( ent );
				break;
		case 113:
				Start_F16( ent->vehicle );
				Start_Client( ent, 400, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 35, 0 );
				checkifonrunway( ent );
				break;
		case 114:
				Start_F4( ent->vehicle );
				Start_Client( ent, 200, 0, 4, 0, 2, 0, 0, 0, 0, 4, 0, 35, 1 );
				checkifonrunway( ent );
				break;
		// AUTOCANNON-MAINGUN-SW-ST-AM-PH-ATGM-MAVERICKS-ROCKETS-BOMBS-LGBS-FLARES-JAMMER
		case 115:
				Start_SU37( ent->vehicle );
				Start_Client( ent, 300, 0, 2, 0, 4, 0, 0, 0, 0, 0, 0, 35, 0 );
				checkifonrunway( ent );
				break;
		case 116:
				Start_MiG21( ent->vehicle );
				Start_Client( ent, 300, 0, 2, 0, 4, 0, 0, 0, 0, 4, 0, 35, 0 );
				checkifonrunway( ent );
				break;
		case 117:
				Start_MiG29( ent->vehicle );
				Start_Client( ent, 400, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 35, 0 );
				checkifonrunway( ent );
				break;
		case 118:
				Start_MiGMFI( ent->vehicle );
				Start_Client( ent, 200, 0, 2, 0, 2, 0, 0, 0, 0, 0, 0, 30, 0 );
				checkifonrunway( ent );
				break;
		case 121:
				Start_Tornado ( ent->vehicle );
				Start_Client( ent, 250, 0, 2, 0, 0, 0, 0, 0, 16, 8, 0, 30, 0 );
				checkifonrunway( ent );
				break;
		case 122:
				Start_A10( ent->vehicle );
				Start_Client( ent, 300, 0, 2, 0, 0, 0, 0, 6, 0, 6, 0, 40, 0 );
				checkifonrunway( ent );
				break;
		// AUTOCANNON-MAINGUN-SW-ST-AM-PH-ATGM-MAVERICKS-ROCKETS-BOMBS-LGBS-FLARES-JAMMER
		case 123:
				Start_F117( ent->vehicle );
				Start_Client( ent, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 4, 35, 0 );
				checkifonrunway( ent );
				break;
		case 124:
				Start_Su24( ent->vehicle );
				Start_Client( ent, 250, 0, 2, 0, 0, 0, 0, 6, 0, 10, 0, 30, 0 );
				checkifonrunway( ent );
				break;
		case 125:
				Start_SU25( ent->vehicle );
				Start_Client( ent, 400, 0, 2, 0, 0, 0, 0, 2, 48, 0, 0, 40, 0 );
				checkifonrunway( ent );
				break;
		case 126:
				Start_SU34( ent->vehicle );
				Start_Client( ent, 250, 0, 2, 0, 0, 0, 0, 4, 0, 0, 2, 35, 1 );
				checkifonrunway( ent );
				break;
		case 211:
				Start_AH64( ent->vehicle );
				Start_Client( ent, 250, 0, 0, 4, 0, 0, 8, 0, 16, 0, 0, 30, 0 );
				checkiflanded( ent );
				break;
		case 212:
				Start_MI28( ent->vehicle );
				Start_Client( ent, 350, 0, 0, 2, 0, 0, 8, 0, 32, 0, 0, 30, 0 );
				checkiflanded( ent );
				break;
		case 221:
				Start_RAH66( ent->vehicle );
				Start_Client( ent, 300, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 30, 0 );
				checkiflanded( ent );
				break;
		case 222:
				Start_KA50( ent->vehicle );
				Start_Client( ent, 350, 0, 0, 6, 0, 0, 0, 0, 24, 0, 0, 30, 0 );
				checkiflanded( ent );
				break;
		// AUTOCANNON-MAINGUN-SW-ST-AM-PH-ATGM-MAVERICKS-ROCKETS-BOMBS-LGBS-FLARES-JAMMER
		case 411:
				Start_M1( ent->vehicle );
				Start_Client( ent, 300, 45, 0, 0, 0, 0, 6, 0, 0, 0, 0, 20, 0 );
				SetOnGround( ent->vehicle );
				break;
		case 412:
				Start_T72( ent->vehicle );
				Start_Client( ent, 300, 45, 0, 0, 0, 0, 6, 0, 0, 0, 0, 20, 0 );
				SetOnGround( ent->vehicle );
				break;
		case 421:
				Start_Scorpion( ent->vehicle );
				Start_Client( ent, 200, 35, 0, 2, 0, 0, 0, 0, 0, 0, 0, 20, 0 );
				SetOnGround( ent->vehicle );
				break;
		case 422:
				Start_BRDM2( ent->vehicle );
				Start_Client( ent, 300, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 20, 0 );
				SetOnGround( ent->vehicle );
				break;
		case 441:
				Start_LAV25( ent->vehicle );
				Start_Client( ent, 300, 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 30, 0 );
				SetOnGround( ent->vehicle );
				break;
		case 442:
				Start_Rapier( ent->vehicle );
				Start_Client( ent, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 30, 0 );
				SetOnGround( ent->vehicle );
				break;
		case 443:
				Start_2S6( ent->vehicle );
				Start_Client( ent, 500, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 30, 0 );
				SetOnGround( ent->vehicle );
				break;
		case 444:
				Start_SA9( ent->vehicle );
				Start_Client( ent, 300, 0, 0, 0, 4, 0, 4, 0, 0, 0, 0, 30, 0 );
				SetOnGround( ent->vehicle );
				break;
		default:
				Start_F16( ent->vehicle );
				Start_Client( ent, 400, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 35, 0 );
				SetOnGround( ent->vehicle );
				break;
//				Start_LQM( ent->vehicle );
	}
	ent->vehicle->mass = 1000;
	ent->vehicle->solid = SOLID_BBOX;
	ent->vehicle->deadflag = DEAD_NO;
	ent->vehicle->clipmask = MASK_PLAYERSOLID;
	ent->vehicle->waterlevel = 0;
	ent->vehicle->watertype = 0;
	ent->vehicle->s.skinnum = 0;
	ent->vehicle->drone_decision_time = 0;	// used for unlocking
	ent->vehicle->ONOFF |= PILOT_ONBOARD;	// Pilot always on board a new vehicle
	ent->vehicle->altitude = 0;
	ent->vehicle->lockstatus = 0;
	ent->vehicle->recharge_time = level.time;
	ent->vehicle->deathtime = level.time;

	if( deathmatch->value )
	{
		ent->safetytime = level.time + 5.0;
		ent->vehicle->takedamage = DAMAGE_NO;
	}
	// general
	ent->vehicle->temptime = level.time;
	ent->vehicle->nextthink = level.time + 0.1;
	ent->vehicle->inuse = true;
	if( strcmp( ent->vehicle->classname, "helo" ) == 0 &&
			(ent->vehicle->ONOFF & LANDED) )
		ent->vehicle->s.frame = 9;
	else
		ent->vehicle->s.frame = 0;
	ent->vehicle->owner = ent;
	ent->vehicle->enemy = NULL;
	if( ent->vehicle->HASCAN & IS_EASTERN )
		ent->vehicle->aqteam = 2;
	else
		ent->vehicle->aqteam = 1;
	if( strcmp( ent->vehicle->classname, "LQM" ) == 0 && teamplay->value == 1 )
		ent->vehicle->aqteam = ent->client->pers.team;

	ent->oldenemy = ent->cam_target;
	ent->goalentity = NULL;
	ent->vehicle->fov = 90;
	gi.cvar_forceset( "fov", va("%d",ent->vehicle->fov) );
	gi.linkentity (ent->vehicle);

	AngleVectors( ent->cam_target->s.angles, forward, NULL, up );
	VectorScale( forward, -(ent->client->pers.cameradistance)+
			ent->cam_target->mins[0], forward );
	VectorScale( up, ent->client->pers.cameraheight, up );
	VectorAdd( ent->cam_target->s.origin, forward, ent->s.origin );
	VectorAdd( ent->s.origin, up, ent->s.origin );

	if( teamplay->value == 1 && joined )
	{
		gi.bprintf( PRINT_MEDIUM, "%s joined team %s.\n", ent->client->pers.netname,
			( (ent->client->pers.team == 1) ? teamname1->string : teamname2->string ) );
		team[ent->client->pers.team-1].players++;
	}
}




void Cmd_VehicleDown_f (edict_t *ent)
{
	float modifier = 1;
	int	  min = 1;

	if( ent->client->veh_selected )
		return;

	if( teamplay->value == 1 )
	{
		if( ent->client->pers.team == 1 || ent->client->pers.team == 3 )
		{
			modifier = 0.5;
		}
		else
		{
			min = ent->veh_max * 0.5 + 1;
		}
	}

	if( !(level.category & 15) )
	{
		gi.bprintf( PRINT_HIGH, "No spawn points available!\n" );
		ShutdownGame();
	}

	if( ent->sel_step == 0 )
	{
		if( ent->client->pers.team == 3 )
			ent->client->pers.team = 4;
		else
			ent->client->pers.team = 3;
		return;
	}
	else if( ent->sel_step == 1 )	// decrease category
	{
		do
		{
			if( ent->veh_category > 1 )
				ent->veh_category /= 2;
			else
				ent->veh_category = 8;
		}while( !(ent->veh_category & level.category) );

		return;
	}
	else if( ent->sel_step == 2 )	// decrease class
	{
		ent->veh_class --;
		if( ent->veh_class < 1 )
		{
			if( ent->veh_category & 1 )
			ent->veh_class = 2;
			else if( ent->veh_category & 2 )
				ent->veh_class = 2;
			else if( ent->veh_category & 4 )
			{
				ent->veh_class = 4;
//				if( teamplay->value == 1 )
//					ent->veh_class = 1;
			}
			else
				ent->veh_class = 0;
		}
		if( ent->veh_class == 3 )	// temp until APCs are done
			ent->veh_class = 2;
		return;
	}

	ent->veh_number --;				// decrease vehicle
	if( ent->veh_number < min )
		ent->veh_number = ent->veh_max * modifier;

	Get_Vehicle_Names_f( ent );
}


void Cmd_VehicleUp_f (edict_t *ent)
{
	short max;	// used to cycle through class
	float modifier = 1;
	int   min = 1;

	if( ent->client->veh_selected )
		return;

	if( teamplay->value == 1 )
	{
		if( ent->client->pers.team == 1 || ent->client->pers.team == 3 )
		{
			modifier = 0.5;
		}
		else
		{
			min = ent->veh_max * 0.5 + 1;
		}
	}

	if( !(level.category & 15) )
	{
		gi.bprintf( PRINT_HIGH, "No spawn points available!\n" );
		ShutdownGame();
	}

	if( ent->sel_step == 0 )
	{
		if( ent->client->pers.team == 3 )
			ent->client->pers.team = 4;
		else
			ent->client->pers.team = 3;
		return;
	}
	else if( ent->sel_step == 1 )	// increase category
	{
		do
		{
			if( ent->veh_category < 8 && ent->veh_category > 0 )
				ent->veh_category *= 2;
			else
				ent->veh_category = 1;
		}while( !(ent->veh_category & level.category) );

		return;
	}
	else if( ent->sel_step == 2 )	// increase class
	{
		ent->veh_class ++;
		if( ent->veh_category & 1 )
			max = 2;
		else if( ent->veh_category & 2 )
			max = 2;
		else if( ent->veh_category & 4 )
		{
			max = 4;
//			if( teamplay->value == 1 )
//				max = 1;
		}
		else
			max = 0;

		if( ent->veh_class == 3 )	// temp until APCs are done!
			ent->veh_class = 4;

		if( ent->veh_class > max )
			ent->veh_class = 1;
		return;
	}

	ent->veh_number ++;				// increase vehicle
	if( ent->veh_number > ent->veh_max * modifier )
		ent->veh_number = min;

	Get_Vehicle_Names_f( ent );
}




void Cmd_Thrustup_f (edict_t *ent )
{
	if( ent->vehicle->thrust == THRUST_AFTERBURNER && strcmp( ent->vehicle->classname,"ground") == 0 )
	{
		ent->vehicle->thrust = THRUST_0;
		return;
	}
	if( strcmp( ent->vehicle->classname,"ground") == 0 && ent->vehicle->ONOFF & BRAKE )
	{
		ent->vehicle->ONOFF &= ~ BRAKE;
/*		gi.cprintf( ent, PRINT_MEDIUM, "Release Handbrake first!\n" );
		return;*/
	}
	if (ent->vehicle->ONOFF & AUTOPILOT )
		return;
	ent->vehicle->thrust ++;
	if( ent->vehicle->thrust > THRUST_100 )
	{
		if( ent->vehicle->HASCAN & HAS_AFTERBURNER )// afterburner
		{
			ent->vehicle->thrust = THRUST_AFTERBURNER;
			if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
				ent->vehicle->s.skinnum = 1;
		}
		else
			ent->vehicle->thrust = THRUST_100;
	}
	if( ent->vehicle->DAMAGE & AFTERBURNER_DAMAGE && ent->vehicle->thrust > THRUST_100 )
	{
		if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
		{
			ent->vehicle->thrust = THRUST_0;
			gi.cprintf( ent, PRINT_MEDIUM, "Reverse is damaged!\n" );
		}
		else
		{
			ent->vehicle->thrust = THRUST_100;
			gi.cprintf( ent, PRINT_MEDIUM, "Afterburner is damaged!\n" );
			ent->vehicle->s.skinnum = 0;
		}
	}
	if( ent->vehicle->DAMAGE & ENGINE75 && ent->vehicle->thrust > THRUST_75 )
	{
		ent->vehicle->thrust = THRUST_75;
		gi.cprintf( ent, PRINT_MEDIUM, "Engines are damaged!\n" );
	}
	if( ent->vehicle->DAMAGE & ENGINE50 && ent->vehicle->thrust > THRUST_50 )
	{
		ent->vehicle->thrust = THRUST_50;
		gi.cprintf( ent, PRINT_MEDIUM, "Engines are fairly damaged!\n" );
	}
	if( ent->vehicle->DAMAGE & ENGINE25 && ent->vehicle->thrust > THRUST_25 )
	{
		ent->vehicle->thrust = THRUST_25;
		gi.cprintf( ent, PRINT_MEDIUM, "Engines are severely damaged!\n" );
	}
	if( ent->vehicle->DAMAGE & ENGINE0 && ent->vehicle->thrust > THRUST_0 )
	{
		ent->vehicle->thrust = THRUST_0;
		gi.cprintf( ent, PRINT_MEDIUM, "Engines are destroyed!\n" );
	}
}

void Cmd_Thrustdown_f (edict_t *ent )
{
	if( ent->vehicle->thrust == THRUST_AFTERBURNER && strcmp( ent->vehicle->classname,"ground") == 0 )
		return;
	if( strcmp( ent->vehicle->classname,"ground") == 0 && ent->vehicle->ONOFF & BRAKE )
	{
		ent->vehicle->ONOFF &= ~BRAKE;
/*		gi.cprintf( ent, PRINT_MEDIUM, "Release Handbrake first!\n" );
		return;*/
	}
	if( ent->vehicle->ONOFF & AUTOPILOT )
		return;
	ent->vehicle->thrust --;
	if( ent->vehicle->thrust < THRUST_0 )
	{
		if( strcmp(ent->vehicle->classname, "ground") == 0 )
		{
			if( ent->vehicle->DAMAGE & AFTERBURNER_DAMAGE )
			{
				gi.cprintf( ent, PRINT_MEDIUM, "Reverse is damaged!\n" );
				ent->vehicle->thrust = THRUST_0;
			}
			else
				ent->vehicle->thrust = THRUST_AFTERBURNER;
		}
		else
			ent->vehicle->thrust = THRUST_0;
	}
	ent->vehicle->s.skinnum = 0;
}


void Cmd_Speedbrake_f (edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;
	if( !(ent->vehicle->HASCAN & HAS_SPEEDBRAKES) || ent->vehicle->deadflag )
		return;
	if( ent->vehicle->ONOFF & AUTOPILOT )
		return;
	if( ent->vehicle->DAMAGE & SPEEDBRAKE_DAMAGE )
	{
		if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
		{
			gi.cprintf( ent, PRINT_HIGH, "Speedbrakes are damaged!\n" );
			return;
		}
		else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
		{
			gi.cprintf( ent, PRINT_HIGH, "Handbrake is damaged!\n" );
			return;
		}
	}
	if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
	{
		ent->vehicle->ONOFF ^= BRAKE;
		if( ent->vehicle->ONOFF & BRAKE )
		{
			gi.cprintf( ent, PRINT_MEDIUM, "Speedbrakes extended!\n" );
		}
		else
			gi.cprintf( ent, PRINT_MEDIUM, "Speedbrakes retracted!\n" );
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		if( ent->vehicle->thrust )
		{
			gi.cprintf( ent, PRINT_MEDIUM, "Only usable when stopped!\n" );
			return;
		}
		ent->vehicle->ONOFF ^= BRAKE;
		if( ent->vehicle->ONOFF & BRAKE )
			gi.cprintf( ent, PRINT_MEDIUM, "Handbrake set!\n" );
		else
			gi.cprintf( ent, PRINT_MEDIUM, "Handbrake released!\n" );
	}
}



void Cmd_Gear_f (edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;
	if( !(ent->vehicle->HASCAN & HAS_LANDINGGEAR) || (ent->vehicle->ONOFF & LANDED)
   			 || ent->vehicle->deadflag )
		return;
	if( ent->vehicle->ONOFF & AUTOPILOT )
		return;
	if( ent->vehicle->actualspeed > (ent->vehicle->speed +399) &&
   		 strcmp( ent->vehicle->classname,"helo") != 0 )
	{
		gi.cprintf( ent, PRINT_MEDIUM,"Speed too high for gear operations!\n");
		return;
	}
	if( ent->vehicle->DAMAGE & GEAR_DAMAGE )
	{
		gi.cprintf( ent, PRINT_HIGH, "Gear is damaged!\n" );
		return;
	}
	ent->vehicle->ONOFF ^= GEARDOWN;
	if( ent->vehicle->ONOFF & GEARDOWN )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Gear down!\n" );
		if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
			ent->vehicle->s.frame += 8;
	}
	else
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Gear up!\n" );
		if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
			ent->vehicle->s.frame -= 8;
	}
}

void Cmd_Flare_f (edict_t *ent )
{
	if( !ent->client->veh_selected || ent->vehicle->deadflag )
		return;
	if( level.time < ent->vehicle->flare_time )
		return;

	Weapon_Blaster_Fire( ent );

	ent->vehicle->flare_time = level.time + 0.8;
}


void Cmd_Coord_f (edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;

	ent->client->pers.autocoord = ent->client->pers.autocoord ? 0 : 1;
	if( ent->client->pers.autocoord == 0 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Auto-Coordination OFF\n" );
	}
	else
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Auto-Coordination ON\n" );
	}
//	if( !deathmatch->value )
//		AQ2SaveVars( ent->client );
}

void Cmd_Roll_f (edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;

	ent->client->pers.autoroll = ent->client->pers.autoroll ? 0 : 1;
	if( ent->client->pers.autoroll == 0 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Auto-Roll-Back OFF\n" );
	}
	else
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Auto-Roll-Back ON\n" );
	}
//	if( !deathmatch->value )
//		AQ2SaveVars( ent->client );
}


void Cmd_KillVehicle_f( edict_t *ent )
{

	if( !ent->client->veh_selected )
		return;

	if( teamplay->value == 0 && ent->deadflag )
		return;

	if((level.time - ent->client->respawn_time) < 3)
		return;

//	ent->vehicle->flags = ent->flags &= ~FL_GODMODE;
	if( !ent->deadflag )
	{
		ent->vehicle->health = 0;

		gi.bprintf( PRINT_MEDIUM, "%s suicides.\n", ent->client->pers.netname );


		if( strcmp( ent->vehicle->classname, "LQM" ) != 0 )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (ent->vehicle->s.origin);
			gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);

			ThrowDebris( ent->vehicle, "models/objects/debris2/tris.md2",
					5, ent->vehicle->s.origin );
	//		ThrowDebris( ent->vehicle, "models/objects/debris2/tris.md2",
	//				5, ent->vehicle->s.origin );
	//		ThrowDebris( ent->vehicle, "models/objects/debris3/tris.md2",
	//				5, ent->vehicle->s.origin );
	//		ThrowDebris( ent->vehicle, "models/objects/debris2/tris.md2",
	//				5, ent->vehicle->s.origin );
			VehicleExplosion( ent->vehicle );
		}

		ent->vehicle->thrust = THRUST_0;
		if( strcmp( ent->vehicle->classname, "plane" ) == 0 ||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
		{
			if( ent->vehicle->ONOFF & LANDED )
			{
				ent->vehicle->deadflag = DEAD_DEAD;
				SetVehicleModel( ent->vehicle );
			}
			else
			{
				ent->vehicle->deadflag = DEAD_DYING;
			}
		}
		else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
		{
			ent->vehicle->deadflag = DEAD_DEAD;
			if( ent->vehicle->activator )
				ent->vehicle->activator->deadflag = DEAD_DEAD;
			ent->vehicle->movetarget->deadflag = DEAD_DEAD;
			ent->cam_target = ent->vehicle;
			SetVehicleModel( ent->vehicle );
		}
		else if( strcmp( ent->vehicle->classname, "LQM" ) == 0 )
		{
			ent->vehicle->deadflag = DEAD_DEAD;
			ent->cam_target = ent->vehicle;
			SetVehicleModel( ent->vehicle );
		}
	}
	if( teamplay->value == 1 )
	{
		gi.bprintf( PRINT_MEDIUM, "%s has left the %s Team!\n", ent->client->pers.netname,
				( (ent->client->pers.team == 1) ? "NATO" : "Warsaw Pact" ) );
		team[ent->client->pers.team-1].players--;
		ent->client->pers.team = 0;
		ent->client->resp.score = 0;
		meansOfDeath = 0;
	}
	else
	{
		meansOfDeath = MOD_SUICIDE;
	}
	ent->deathtime = level.time + 1.0;
}




void Cmd_FixRoll_f( edict_t *ent )
{
	if( ent->client->pers.videomodefix == 1 )
	{
		ent->client->pers.videomodefix = -1;
		gi.cprintf( ent, PRINT_MEDIUM, "Roll fix enabled!\n" );
	}
	else
	{
		ent->client->pers.videomodefix = 1;
		gi.cprintf( ent, PRINT_MEDIUM, "Roll fix disabled!\n" );
	}
//	if( !deathmatch->value )
//		AQ2SaveVars( ent->client );
}


/*
void Cmd_Ripple_f(edict_t *ent)
{
//	int selected_weapon;

//	gi.cprintf( ent, PRINT_HIGH, "ripple changed!\n" );
//	selected_weapon = ITEM_INDEX(ent->client->pers.weapon);
	if( !ent->client->veh_selected )
		return;

	if( strcmp(ent->client->pers.secondary->pickup_name, "Iron Bombs") == 0 )
	{
		ent->vehicle->ripple_b *= 2;
		if( ent->vehicle->ripple_b > 4 )
			ent->vehicle->ripple_b = 1;
	}
	else if( strcmp(ent->client->pers.secondary->pickup_name, "Rocket Launcher") == 0 )
	{
		ent->vehicle->ripple_f *= 2;
		if( ent->vehicle->ripple_f > 4 )
			ent->vehicle->ripple_f = 1;
	}
}*/


void Cmd_GPS_f(edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;
//	ent->vehicle->gps = ent->vehicle->gps ? 0 : 1;
	if( ent->vehicle->DAMAGE & GPS_DAMAGE )
	{
		gi.cprintf( ent, PRINT_HIGH, "GPS is damaged!\n" );
		return;
	}
	ent->vehicle->ONOFF ^= GPS;
}

void Cmd_Autopilot_f(edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;
	if( ent->vehicle->deadflag )
		return;
	if( strcmp( ent->vehicle->classname, "plane") != 0 &&
		strcmp( ent->vehicle->classname, "helo") != 0 )
		return;
	if( ent->vehicle->ONOFF & LANDED )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Autopilot on ground not possible!\n" );
		return;
	}
	if( ent->vehicle->DAMAGE & AUTOPILOT_DAMAGE )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Autopilot is damaged!\n" );
		return;
	}
	else if( ent->vehicle->DAMAGE & ENGINE75 || ent->vehicle->DAMAGE & AFTERBURNER_DAMAGE )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Engine damaged! Autopilot not possible!\n" );
		return;
	}
	if( ent->vehicle->fov < 90 )
	{
		ent->vehicle->fov = 90;
		ent->client->ps.fov = ent->vehicle->fov;
	}
	if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
		ent->vehicle->thrust = THRUST_100;
	else if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
		ent->vehicle->thrust = THRUST_50;

	ent->vehicle->ONOFF ^= AUTOPILOT;
	ent->vehicle->ONOFF &= ~BRAKE;

	if( ent->vehicle->HASCAN & HAS_LANDINGGEAR )
		ent->vehicle->ONOFF &= ~GEARDOWN;

}


void Cmd_Zoom_f( edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;
	if( ent->vehicle->ONOFF & AUTOPILOT )
		return;

	if( ent->vehicle->fov > 60 )
		ent->vehicle->fov = 60;
	else if( ent->vehicle->fov > 30 && ent->vehicle->fov <= 60)
		ent->vehicle->fov = 30;
	else if( ent->vehicle->fov > 10 && ent->vehicle->fov <= 30 )
		ent->vehicle->fov = 10;
	else if( ent->vehicle->fov <= 10 )
		ent->vehicle->fov = 90;
	ent->client->ps.fov = ent->vehicle->fov;
}

void Cmd_Tower_f( edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;
	if( !ent->vehicle->radio_target )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "There is nothing to bother the tower with!\n" );
		return;
	}
	gi.cprintf( ent, PRINT_MEDIUM, "Tower, request operation of %s!\n",
				ent->vehicle->radio_target->target );
	G_UseTargets( ent->vehicle->radio_target, ent->vehicle );
}


void Cmd_Light_f( edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;

	ent->vehicle->ONOFF ^= LIGHT;

	if( !(ent->vehicle->ONOFF & LIGHT) )
	{
		gi.bprintf( PRINT_HIGH, "Light off!\n" );
		ent->vehicle->target_ent->think = G_FreeEdict;
		ent->vehicle->target_ent->nextthink = level.time + 0.1;
	}
	else
	{
		gi.bprintf( PRINT_HIGH, "Light on!\n" );
		SpawnLight( ent );
	}
}


/*void Cmd_Autoaim_f(edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	ent->client->pers.autoaim = (ent->client->pers.autoaim) ? 0 : 1;

	if( ent->client->pers.autoaim )
		gi.cprintf( ent, PRINT_MEDIUM, "Autoaim ON!\n" );
	else
		gi.cprintf( ent, PRINT_MEDIUM, "Autoaim OFF!\n" );
}
*/




void Cmd_Pickle_f(edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	if( !ent->client->pers.secondary )
		return;

	if( ent->vehicle->deadflag )
		return;

	if( strcmp( ent->client->pers.secondary->pickup_name, "AutoCannon" ) == 0 &&
		level.time >= ent->client->autocannon_time )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] > 0 )
			Weapon_Autocannon_Fire( ent );
	}

	if( ent->vehicle->HASCAN & HAS_WEAPONBAY )
	{
		if( !(ent->vehicle->ONOFF & BAY_OPEN) &&
		 	strcmp( ent->client->pers.secondary->pickup_name, "AutoCannon" ) != 0)
		{
			Cmd_OpenBay_f( ent );
			return;
		}
	}

	if( level.time < ent->client->pickle_time )
	{
		return;
	}
	if( strcmp( ent->client->pers.secondary->pickup_name, "SW Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] )
		{
			Weapon_Sidewinder_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "ST Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] )
		{
			Weapon_Stinger_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "AM Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] )
		{
			Weapon_AMRAAM_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "PH Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] )
		{
			Weapon_Phoenix_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "ATGM Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] )
		{
			Weapon_ATGM_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "Maverick Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] )
		{
			Weapon_Maverick_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "Iron Bombs" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] > 0 )
		{
			Weapon_Bombs_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "Laser Bombs" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] > 0 )
		{
			Weapon_LGB_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "MainGun" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] )
			Weapon_MainGun_Fire( ent );
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "ECM" ) == 0 )
	{
			Weapon_ECM_Fire( ent );
	}
	else if( strcmp( ent->client->pers.secondary->pickup_name, "Rocket Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] > 0 )
		{
			Weapon_RocketLauncher_Fire( ent );
			ent->vehicle->timestamp = level.time + 5.0;
		}
	}
}



void Cmd_PickleToggle_f(edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	ent->vehicle->ONOFF ^= PICKLE_ON;
}

void Cmd_OpenBay_f(edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	if( strcmp( ent->vehicle->classname, "LQM" ) == 0 )
	{
		if( ent->vehicle->s.modelindex == 0 )
		{
			ent->vehicle->s.modelindex = gi.modelindex( "vehicles/infantry/pilot/pilot.md2" );
			ent->vehicle->s.modelindex2 = gi.modelindex( "vehicles/infantry/pilot/mp5.md2" );
		}
		else
		{
			ent->vehicle->s.modelindex = 0;
			ent->vehicle->s.modelindex2 = 0;
		}
	}

	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY) )
		return;

	if( ent->vehicle->deadflag )
		return;

	if( ent->vehicle->DAMAGE & WEAPONBAY_DAMAGE )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Baydoors damaged!\n");
		return;
	}

	ent->vehicle->ONOFF ^= BAY_OPEN;

	if( ent->vehicle->ONOFF & BAY_OPEN )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Weaponbay opened.\n" );
		if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
			ent->vehicle->s.frame += 16;
		ent->vehicle->timestamp = level.time + 5.0;
		ent->client->pickle_time = level.time + 0.6;
	}
	else if( level.time > ent->client->pickle_time )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Weaponbay closed.\n" );
		if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
			ent->vehicle->s.frame -= 16;
		ent->vehicle->timestamp = level.time - 0.1;
	}
}

void AutoCloseBay( edict_t *ent )
{
	if( !ent->client->veh_selected )
		return;

	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY) )
		return;

	if( ent->vehicle->deadflag )
		return;

	if( ent->vehicle->DAMAGE & WEAPONBAY_DAMAGE )
	{
		return;
	}

	ent->vehicle->ONOFF &= ~BAY_OPEN;
	gi.cprintf( ent, PRINT_MEDIUM, "Weaponbay closed.\n" );
	if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
		ent->vehicle->s.frame -= 16;
	ent->vehicle->timestamp = level.time - 0.1;
}

void Cmd_VehicleInfo_f(edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	gi.cprintf( ent, PRINT_HIGH, "Name:%s, Topspeed:%.0f (AB:%0.f), Stall:%.0f\n",
		ent->vehicle->friendlyname, ent->vehicle->topspeed/8, ent->vehicle->topspeed*4/24,
	    ent->vehicle->speed/8 );
	gi.cprintf( ent, PRINT_HIGH, "Maneuverability: %d(Pitch), %d(Yaw), %d(Roll)\n",
		ent->vehicle->turnspeed[0], ent->vehicle->turnspeed[1], ent->vehicle->turnspeed[2] );
	gi.cprintf( ent, PRINT_HIGH, "MaxHealth: %d, ExplodeHealth: %d, Flags: %d\n",
		ent->vehicle->max_health, ent->vehicle->gib_health, ent->vehicle->HASCAN );
	gi.cprintf( ent, PRINT_HIGH, "Maxs(%.0f/%.0f/%.0f), Mins(%.0f/%.0f/%.0f), Stealth: %d, Flares:%d\n",
   		ent->vehicle->maxs[0], ent->vehicle->maxs[1], ent->vehicle->maxs[2], ent->vehicle->mins[0],
	    ent->vehicle->mins[1], ent->vehicle->mins[2], ent->vehicle->stealth, ent->client->pers.max_flares );
	gi.cprintf( ent, PRINT_HIGH, "Autocannon: Firerate %.1f Damage %d Spread %.2f Ammo %d\n",
		ent->vehicle->gunfirerate, ent->vehicle->gundamage, ent->vehicle->gunspread,
	   	ent->client->pers.max_bullets );
	gi.cprintf( ent, PRINT_HIGH, "Gunshells: %d (%d Damage), Sidewinders: %d, Stingers: %d\n",
		ent->client->pers.max_shells, ent->vehicle->maingundamage, ent->client->pers.max_sidewinder,
		ent->client->pers.max_stinger );
	gi.cprintf( ent, PRINT_HIGH, "AMRAAMs: %d, Phoenix: %d, Hellfire: %d, Maverick: %d\n",
		ent->client->pers.max_amraam, ent->client->pers.max_phoenix, ent->client->pers.max_ATGMs,
		ent->client->pers.max_mavericks );
	gi.cprintf( ent, PRINT_HIGH, "Rockets: %d, Iron Bombs: %d, LGBs: %d, Jammer: %d (%d)\n",
		ent->client->pers.max_rockets, ent->client->pers.max_bombs, ent->client->pers.max_lgbs,
		ent->client->pers.max_jammer, JAMMER_EFFECT );

}


void Cmd_Eject_f(edict_t *ent)
{
	edict_t	*LQM;
	vec3_t	up, forward;

	if( !ent->client->veh_selected )
		return;

	if( ent->vehicle->deadflag == DEAD_DEAD )
		return;

	if( ent->vehicle->ONOFF & IS_CHANGEABLE )
	{
		Cmd_Change_f( ent );
		return;
	}
	return;
	if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
	{
		if( !(ent->vehicle->ONOFF & LANDED) )
		{
			// get rid of old vehicle
			ent->vehicle->deadflag = DEAD_DYING;
			ent->vehicle->HASCAN |= IS_DRONE;
			ent->vehicle->touch = drone_touch;
			ent->vehicle->pain = drone_pain;
			ent->vehicle->die = drone_die;
			ent->vehicle->think = SteerDrone;
			ent->vehicle->owner = NULL;
			ent->vehicle->nextthink = level.time + 0.1;
			// get the LQM
			LQM = G_Spawn();
			Start_LQM( LQM );
			LQM->aqteam = ent->client->pers.team;
			LQM->gravity = 0.1;
			VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
			VectorCopy( ent->vehicle->s.angles, LQM->s.angles );
			VectorCopy( ent->vehicle->velocity, LQM->velocity );
			VectorCopy( ent->vehicle->avelocity, LQM->avelocity );
			AngleVectors( LQM->s.angles, NULL, NULL, up );
			VectorScale( up, 12, up );
			VectorAdd( LQM->s.origin, up, LQM->s.origin );
			VectorMA( LQM->velocity, 12, up, LQM->velocity );
//			VectorSet( LQM->avelocity, -(200 + (random()*100)-50), 0,
//							10 + (random()*20)-10 );
			gi.linkentity( LQM );
			ent->vehicle = LQM;
			ent->vehicle->deadflag = DEAD_NO;
			ent->vehicle->nextthink = level.time + 0.1;
			ent->vehicle->ONOFF = AUTOROTATION;
			ent->vehicle->movetype = MOVETYPE_FLYMISSILE;
			ent->vehicle->drone_decision_time = level.time + 1.5;
			ent->vehicle->owner = ent;
			ClearClient( ent );
			Start_Client( ent, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		}
		else if( ent->vehicle->ONOFF & LANDED && ent->vehicle->thrust == 0 )
		{
			ent->vehicle->ONOFF &= ~PILOT_ONBOARD;
			ent->vehicle->nextthink = level.time - 1;
			ent->vehicle->s.frame = 0;
			// get the LQM
			LQM = G_Spawn();
			Start_LQM( LQM );
			LQM->aqteam = ent->client->pers.team;
			AngleVectors( LQM->s.angles, forward, NULL, NULL );
			VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
			VectorMA( LQM->s.origin, ent->vehicle->maxs[0]+5, forward, LQM->s.origin );
			VectorCopy( ent->vehicle->s.angles, LQM->s.angles );
			gi.linkentity( LQM );
			LQM->ONOFF |= LANDED;
			ent->vehicle = LQM;
			ent->vehicle->deadflag = DEAD_NO;
			ent->vehicle->nextthink = level.time + 0.1;
			ent->vehicle->movetype = MOVETYPE_FLYMISSILE;
			ent->vehicle->drone_decision_time = level.time + 1.5;
			ent->vehicle->owner = ent;
			ClearClient( ent );
			Start_Client( ent, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		}
	}
	else if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
	{
		if( ent->vehicle->ONOFF & LANDED  && ent->vehicle->thrust == 0 )
		{
			ent->vehicle->ONOFF &= ~PILOT_ONBOARD;
			ent->vehicle->nextthink = level.time - 1;
			ent->vehicle->s.frame = 0;
			// get the LQM
			LQM = G_Spawn();
			Start_LQM( LQM );
			LQM->aqteam = ent->client->pers.team;
			AngleVectors( LQM->s.angles, forward, NULL, NULL );
			VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
			VectorMA( LQM->s.origin, ent->vehicle->maxs[0]+5, forward, LQM->s.origin );
			VectorCopy( ent->vehicle->s.angles, LQM->s.angles );
			gi.linkentity( LQM );
			LQM->ONOFF |= LANDED;
			ent->vehicle = LQM;
			ent->vehicle->deadflag = DEAD_NO;
			ent->vehicle->nextthink = level.time + 0.1;
			ent->vehicle->movetype = MOVETYPE_FLYMISSILE;
			ent->vehicle->drone_decision_time = level.time + 1.5;
			ent->vehicle->owner = ent;
			ClearClient( ent );
			Start_Client( ent, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		}
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		if( ent->vehicle->thrust == 0 )
		{
			ent->vehicle->ONOFF &= ~PILOT_ONBOARD;
			ent->vehicle->nextthink = level.time - 1;
			ent->vehicle->s.frame = 0;
			// get the LQM
			LQM = G_Spawn();
			Start_LQM( LQM );
			LQM->aqteam = ent->client->pers.team;
			AngleVectors( LQM->s.angles, forward, NULL, NULL );
			VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
			VectorMA( LQM->s.origin, ent->vehicle->maxs[0]+5, forward, LQM->s.origin );
			VectorCopy( ent->vehicle->s.angles, LQM->s.angles );
			gi.linkentity( LQM );
			LQM->ONOFF |= LANDED;
			ent->vehicle = LQM;
			ent->vehicle->deadflag = DEAD_NO;
			ent->vehicle->nextthink = level.time + 0.1;
			ent->vehicle->movetype = MOVETYPE_FLYMISSILE;
			ent->vehicle->drone_decision_time = level.time + 1.5;
			ent->vehicle->owner = ent;
			ClearClient( ent );
			Start_Client( ent, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		}
	}
	else if( strcmp( ent->vehicle->classname, "LQM" ) == 0 && ent->vehicle->activator )
	{
		LQM = ent->vehicle->activator;
		ent->vehicle->solid = SOLID_NOT;
		ent->vehicle->takedamage = DAMAGE_NO;
		ent->vehicle->s.modelindex = 0;
		ent->vehicle->owner = NULL;
		gi.unlinkentity( ent->vehicle );
		G_FreeEdict( ent->vehicle );
		ent->vehicle = LQM;
		ent->vehicle->owner = ent;
		if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
		{
			ent->vehicle->think = SteerPlane;
			ent->vehicle->touch = plane_touch;
			ent->vehicle->pain = vehicle_pain;
			ent->vehicle->die = vehicle_die;
//			ent->vehicle->s.origin[2] += 5;
			checkifonrunway( ent );
		}
		else if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
		{
			ent->vehicle->think = SteerHelo;
			ent->vehicle->touch = helo_touch;
			ent->vehicle->pain = vehicle_pain;
			ent->vehicle->die = vehicle_die;
//			ent->vehicle->s.origin[2] += 5;
			checkiflanded( ent );
		}
		else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
		{
			ent->vehicle->think = SteerGround;
			ent->vehicle->touch = ground_touch;
			ent->vehicle->pain = vehicle_pain;
			ent->vehicle->die = vehicle_die;
			ent->vehicle->movetype = MOVETYPE_GROUND;
			ent->vehicle->activator = LQM->activator;
			ent->vehicle->movetarget = LQM->movetarget;
//			ent->vehicle->s.origin[2] += 3;
		}
		ent->vehicle->nextthink = level.time + 0.1;
		ent->vehicle->ONOFF |= PILOT_ONBOARD;
		ClearClient( ent );
		Start_Client( ent, 100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
	}
}

void Cmd_Change_f(edict_t *ent)
{
	if( ent->vehicle->thrust != 0 || ent->vehicle->health < ent->vehicle->max_health
		|| ent->vehicle->DAMAGE )
		return;

	VectorCopy( ent->vehicle->s.angles, ent->s.angles );
	if( ent->vehicle->movetarget )
	{
		gi.unlinkentity( ent->vehicle->movetarget );
		G_FreeEdict( ent->vehicle->movetarget );
	}
	if( ent->vehicle->activator )
	{
		VectorCopy( ent->vehicle->activator->s.angles, ent->s.angles );
		gi.unlinkentity( ent->vehicle->activator );
		G_FreeEdict( ent->vehicle->activator );
	}
	VectorCopy( ent->vehicle->s.origin, ent->s.origin );
	gi.sound( ent->vehicle, CHAN_BODY, gi.soundindex( "death/0.wav" ), 1, ATTN_NORM, 0 );
	gi.unlinkentity( ent->vehicle );
	G_FreeEdict( ent->vehicle );

	ent->client->veh_selected = 0;
	ent->sel_step = 1;
	ent->client->showscores = ent->client->showinventory = 0;
	ent->client->selectvehicle = true;
	ent->client->breather_sound = 1;		// we ejected/changed
	ClearClient( ent );
}

void ClearClient( edict_t *ent )
{
	// set new weapons
	ent->client->pers.max_bullets	 = 0;
	ent->client->pers.max_shells	 = 0;
	ent->client->pers.max_sidewinder = 0;
	ent->client->pers.max_ATGMs		 = 0;
	ent->client->pers.max_mavericks  = 0;
	ent->client->pers.max_rockets	 = 0;
	ent->client->pers.max_bombs		 = 0;
	ent->client->pers.max_lgbs		 = 0;
	ent->client->pers.max_flares	 = 0;
	ent->client->pers.max_jammer     = 0;
	ent->client->pers.max_stinger    = 0;
	ent->client->pers.max_amraam	 = 0;
	ent->client->pers.max_phoenix	 = 0;
	// set old weapons to 0
	ent->client->pers.max_grenades	 = 0;
	ent->client->pers.max_cells		 = 0;
	ent->client->pers.max_slugs		 = 0;

	// clear inventory
	memset(ent->client->pers.inventory, 0, sizeof(ent->client->pers.inventory));

	ent->vehicle->flare = 0;
}


