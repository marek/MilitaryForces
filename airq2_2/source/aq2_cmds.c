/************************************************/
/* Filename: aq2_cmds.cpp					    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 1999-07-04							*/
/*												*/
/* Description:									*/
/* Handles NewAirQuake2-specific Commands		*/
/*												*/
/************************************************/

#include "g_local.h"

void Cmd_Cameramode (edict_t *ent)
{
	char		*argument, output[25];
	int			oldmode = ent->client->cameramode;

	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "No vehicle selected.\n" );
		return;
	}
	if( ent->client->REPEAT & REPEAT_FREELOOK )
	{
		return;
	}

	// get the command
	argument = gi.args();

	if( stricmp( argument, "next" ) == 0 )
	{
		ent->client->cameramode ++;
		if( ent->client->cameramode >= CM_NUM_MODES )
			ent->client->cameramode = 0;
		if( ent->client->cameramode >= CM_TURRET_FORWARD && ent->client->cameramode < CM_MISSILE_FORWARD
			&& strcmp( ent->vehicle->classname, "ground" ) != 0 )
			ent->client->cameramode = CM_MISSILE_FORWARD;
		if( ent->client->cameramode >= CM_MISSILE_FORWARD && missileview->value < 1 
			&& strcmp( ent->vehicle->classname, "ground" ) != 0 )
			ent->client->cameramode = CM_HULL_FORWARD;
	}
	else if( stricmp( argument, "prev" ) == 0 )
	{
		ent->client->cameramode --;
		if( ent->client->cameramode < 0 )
			ent->client->cameramode = CM_NUM_MODES - 1;
		if( ent->client->cameramode >= CM_MISSILE_FORWARD && missileview->value < 1 
			&& strcmp( ent->vehicle->classname, "ground" ) != 0 )
			ent->client->cameramode = CM_TURRET_DOWN;
		if( ent->client->cameramode >= CM_TURRET_FORWARD && ent->client->cameramode < CM_MISSILE_FORWARD
			&& strcmp( ent->vehicle->classname, "ground" ) != 0 )
			ent->client->cameramode = CM_HULL_DOWN;
	}
	else if( stricmp( argument, "nextvehicle" ) == 0 )
	{
		FindNextCamTarget( ent );
	}
	else if( stricmp( argument, "prevvehicle" ) == 0 )
	{
		FindPrevCamTarget( ent );
	}
	else if( stricmp( argument, "ownvehicle" ) == 0 )
	{
		GotoOwnVehicle( ent );
	}
	else if( stricmp( argument, "default" ) == 0 )
	{
		ent->client->cameradistance = ent->client->cameratarget->movedir[0];
		ent->client->cameraheight = ent->client->cameratarget->movedir[1];
	}
	else if( stricmp( argument, "near" ) == 0 )
	{
		ent->client->cameradistance += 5;
		if( ent->client->cameradistance > -( abs(ent->vehicle->maxs[0]) + abs(ent->vehicle->mins[0]) ) )
			ent->client->cameradistance = -( abs(ent->vehicle->maxs[0]) + abs(ent->vehicle->mins[0]) );
//		gi.bprintf( PRINT_HIGH, "%d\n", ent->client->cameradistance );
	}
	else if( stricmp( argument, "far" ) == 0 )
	{
		ent->client->cameradistance -= 5;
		if( ent->client->cameradistance < MAX_CAMERADISTANCE )
			ent->client->cameradistance = MAX_CAMERADISTANCE;
//		gi.bprintf( PRINT_HIGH, "%d\n", ent->client->cameradistance );
	}
	else if( stricmp( argument, "high" ) == 0 )
	{
		ent->client->cameraheight += 5;
		if( ent->client->cameraheight > MAX_CAMERAHEIGHT )
			ent->client->cameraheight = MAX_CAMERAHEIGHT;
//		gi.bprintf( PRINT_HIGH, "%d\n", ent->client->cameradistance );
	}
	else if( stricmp( argument, "low" ) == 0 )
	{
		ent->client->cameraheight -= 5;
		if( ent->client->cameraheight < -MAX_CAMERAHEIGHT )
			ent->client->cameraheight = -MAX_CAMERAHEIGHT;
//		gi.bprintf( PRINT_HIGH, "%d\n", ent->client->cameradistance );
	}
	else if( stricmp( argument, "forward" ) == 0 )
	{
		if( oldmode >= CM_HULL_FORWARD && oldmode < CM_TURRET_FORWARD )
			ent->client->cameramode = CM_HULL_FORWARD;
		else if( oldmode >= CM_TURRET_FORWARD && oldmode < CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_TURRET_FORWARD;
		else if( oldmode >= CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_MISSILE_FORWARD;
	}
	else if( stricmp( argument, "back" ) == 0 )
	{
		if( oldmode >= CM_HULL_FORWARD && oldmode < CM_TURRET_FORWARD )
			ent->client->cameramode = CM_HULL_BACK;
		else if( oldmode >= CM_TURRET_FORWARD && oldmode < CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_TURRET_BACK;
		else if( oldmode >= CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_MISSILE_BACK;
	}
	else if( stricmp( argument, "left" ) == 0 )
	{
		if( oldmode >= CM_HULL_FORWARD && oldmode < CM_TURRET_FORWARD )
			ent->client->cameramode = CM_HULL_LEFT;
		else if( oldmode >= CM_TURRET_FORWARD && oldmode < CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_TURRET_LEFT;
		else if( oldmode >= CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_MISSILE_LEFT;
	}
	else if( stricmp( argument, "right" ) == 0 )
	{
		if( oldmode >= CM_HULL_FORWARD && oldmode < CM_TURRET_FORWARD )
			ent->client->cameramode = CM_HULL_RIGHT;
		else if( oldmode >= CM_TURRET_FORWARD && oldmode < CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_TURRET_RIGHT;
		else if( oldmode >= CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_MISSILE_RIGHT;
	}
	else if( stricmp( argument, "up" ) == 0 )
	{
		if( oldmode >= CM_HULL_FORWARD && oldmode < CM_TURRET_FORWARD )
			ent->client->cameramode = CM_HULL_UP;
		else if( oldmode >= CM_TURRET_FORWARD && oldmode < CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_TURRET_UP;
		else if( oldmode >= CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_MISSILE_UP;
	}
	else if( stricmp( argument, "down" ) == 0 )
	{
		if( oldmode >= CM_HULL_FORWARD && oldmode < CM_TURRET_FORWARD )
			ent->client->cameramode = CM_HULL_DOWN;
		else if( oldmode >= CM_TURRET_FORWARD && oldmode < CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_TURRET_DOWN;
		else if( oldmode >= CM_MISSILE_FORWARD )
			ent->client->cameramode = CM_MISSILE_DOWN;
	}
	else if( stricmp( argument, "hull" ) == 0 )
	{
		ent->client->cameramode = CM_HULL_FORWARD;
	}
	else if( stricmp( argument, "turret" ) == 0 )
	{
		if( strcmp( ent->vehicle->classname, "ground" ) != 0 )
		{
			gi.cprintf( ent, PRINT_MEDIUM, "Mode not available!\n" );
			return;
		}
		ent->client->cameramode = CM_TURRET_FORWARD;
	}
	else if( stricmp( argument, "missile" ) == 0 )
	{
		if( missileview->value < 1 )
		{
			gi.cprintf( ent, PRINT_MEDIUM, "Server disabled this mode!\n" );
			return;
		}
		ent->client->cameramode = CM_MISSILE_FORWARD;
	}
	else 
	{
		gi.cprintf( ent, PRINT_MEDIUM, "Unknown Cameramode.\n" );
		return;
	}

	// if the cameramode has changed...
	if( oldmode != ent->client->cameramode )
	{
		switch( ent->client->cameramode )
		{
		case CM_HULL_FORWARD:
			strcpy( output, "Hull-View Forward" );
			break;
		case CM_HULL_BACK:
			strcpy( output, "Hull-View Back" );
			break;
		case CM_HULL_LEFT:
			strcpy( output, "Hull-View Left" );
			break;
		case CM_HULL_RIGHT:
			strcpy( output, "Hull-View Right" );
			break;
		case CM_HULL_UP:
			strcpy( output, "Hull-View Up" );
			break;
		case CM_HULL_DOWN:
			strcpy( output, "Hull-View Down" );
			break;
		case CM_TURRET_FORWARD:
			strcpy( output, "Turret-View Forward" );
			break;
		case CM_TURRET_BACK:
			strcpy( output, "Turret-View Back" );
			break;
		case CM_TURRET_LEFT:
			strcpy( output, "Turret-View Left" );
			break;
		case CM_TURRET_RIGHT:
			strcpy( output, "Turret-View Right" );
			break;
		case CM_TURRET_UP:
			strcpy( output, "Turret-View Up" );
			break;
		case CM_TURRET_DOWN:
			strcpy( output, "Turret-View Down" );
			break;
		case CM_MISSILE_FORWARD:
			strcpy( output, "Missile-View Forward" );
			break;
		case CM_MISSILE_BACK:
			strcpy( output, "Missile-View Back" );
			break;
		case CM_MISSILE_LEFT:
			strcpy( output, "Missile-View Left" );
			break;
		case CM_MISSILE_RIGHT:
			strcpy( output, "Missile-View Right" );
			break;
		case CM_MISSILE_UP:
			strcpy( output, "Missile-View Up" );
			break;
		case CM_MISSILE_DOWN:
			strcpy( output, "Missile-View Down" );
			break;
		}
		gi.cprintf( ent, PRINT_MEDIUM, "%s\n", output );
	}
}

void Cmd_Cam_x( edict_t *ent )
{
	char		*argument;
	int			distance = ent->client->cameradistance * -1;

	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;	

	// get the command
	argument = gi.args();

	sscanf( argument, "%d", &distance );

//	gi.bprintf( PRINT_HIGH, "%s = %d\n", argument, distance );

	distance *= -1;

	if( distance < MAX_CAMERADISTANCE )
		distance = MAX_CAMERADISTANCE;
	else if( distance > -( abs(ent->vehicle->maxs[0]) + abs(ent->vehicle->mins[0]) ) )
		distance = -( abs(ent->vehicle->maxs[0]) + abs(ent->vehicle->mins[0]) );

	gi.cprintf( ent, PRINT_HIGH, "distance %d\n", (distance*-1) );

	ent->client->cameradistance = distance;
}

void Cmd_Cam_y( edict_t *ent )
{
	char		*argument;
	int			height = ent->client->cameraheight;

	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;	

	// get the command
	argument = gi.args();

	sscanf( argument, "%d", &height );

//	gi.bprintf( PRINT_HIGH, "%s = %d\n", argument, height );

	if( height < -MAX_CAMERAHEIGHT )
		height = -MAX_CAMERAHEIGHT;
	else if( height > MAX_CAMERAHEIGHT )
		height = MAX_CAMERAHEIGHT;

	gi.cprintf( ent, PRINT_HIGH, "height %d\n", height );

	ent->client->cameraheight = height;
}


void Cmd_Increase (edict_t *ent)
{
	int maxpossible;

	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
	{
		if( ent->client->selectvehicle )
			VehicleSelectionNext( ent );
		return;
	}

	if( ent->vehicle->deadflag )
		return;
	if( ent->vehicle->DAMAGE & DAMAGE_ENGINE_DESTROYED )
		return;

	if( ent->vehicle->DAMAGE & DAMAGE_ENGINE_20 )
		maxpossible = 2;
	else if( ent->vehicle->DAMAGE & DAMAGE_ENGINE_40 )
		maxpossible = 4;
	else if( ent->vehicle->DAMAGE & DAMAGE_ENGINE_60 )
		maxpossible = 6;
	else if( ent->vehicle->DAMAGE & DAMAGE_ENGINE_80 )
		maxpossible = 8;
	else if( ent->vehicle->DAMAGE & DAMAGE_AFTERBURNER )
		maxpossible = THRUST_MILITARY;

	ent->vehicle->thrust++;
	if( ent->vehicle->thrust > ent->vehicle->maxthrust )
		ent->vehicle->thrust = ent->vehicle->maxthrust;
	if( ent->vehicle->thrust > maxpossible )
		ent->vehicle->thrust = maxpossible;
	
	ent->client->thrusttime = level.time + 0.1;

//	gi.bprintf( PRINT_HIGH, "%d\n", ent->vehicle->thrust );
}

void Cmd_Decrease (edict_t *ent)
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
	{
		if( ent->client->selectvehicle )
			VehicleSelectionPrev( ent );
		return;
	}

	if( ent->vehicle->deadflag )
		return;
	if( ent->vehicle->DAMAGE & DAMAGE_ENGINE_DESTROYED )
		return;

	ent->vehicle->thrust--;
	if( ent->vehicle->thrust < ent->vehicle->minthrust )
		ent->vehicle->thrust = ent->vehicle->minthrust;

	ent->client->thrusttime = level.time + 0.1;

//	gi.bprintf( PRINT_HIGH, "%d\n", ent->vehicle->thrust );
}

void Cmd_Gear (edict_t *ent)
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;	
	if( !(ent->vehicle->HASCAN & HASCAN_GEAR) || !(ent->vehicle->ONOFF & ONOFF_AIRBORNE)
   			 || ent->vehicle->deadflag )
		return;
	if( ent->vehicle->speed > (ent->vehicle->stallspeed + 100) &&
   		 strcmp( ent->vehicle->classname,"helo") != 0 )
	{
		gi.cprintf( ent, PRINT_MEDIUM,"Speed too high for gear operations!\n");
		return;
	}
	if( !(ent->vehicle->ONOFF & ONOFF_AIRBORNE) && !(ent->vehicle->ONOFF & ONOFF_LANDED) )
		return;
	if( ent->vehicle->DAMAGE & DAMAGE_GEAR )
	{
		gi.cprintf( ent, PRINT_HIGH, "Gear is damaged!\n" );
		return;
	}

	ent->vehicle->ONOFF ^= ONOFF_GEAR;

	if( ent->vehicle->ONOFF & ONOFF_GEAR )
	{
		ent->vehicle->mins[2] -= 4;
		gi.cprintf( ent, PRINT_MEDIUM, "Gear down!\n" );
	}
	else
	{
		ent->vehicle->mins[2] += 4;
		gi.cprintf( ent, PRINT_MEDIUM, "Gear up!\n" );
	}
}


void Cmd_Bay( edict_t *ent )
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;	
	if( !(ent->vehicle->HASCAN & HASCAN_WEAPONBAY) || ent->vehicle->deadflag )
		return;
	if( !(ent->vehicle->ONOFF & ONOFF_AIRBORNE) )
		return;
	if( level.time < ent->vehicle->attenuation )
		return;
	if( ent->vehicle->DAMAGE & DAMAGE_WEAPONBAY )
	{
		if( strcmp( ent->vehicle->classname, "plane" ) == 0 ||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
		{
			gi.cprintf( ent, PRINT_HIGH, "Bay-Doors are damaged!\n" );
			return;
		}
	}
	ent->vehicle->ONOFF ^= ONOFF_WEAPONBAY;

	if( strcmp( ent->vehicle->classname, "plane" ) == 0 ||
		strcmp( ent->vehicle->classname, "helo" ) == 0 )
	{
		if( ent->vehicle->ONOFF & ONOFF_WEAPONBAY )
		{
			gi.cprintf( ent, PRINT_MEDIUM, "Weaponbay opened!\n" );
			ent->vehicle->wait = level.time + 3.0;
		}
		else
		{
			gi.cprintf( ent, PRINT_MEDIUM, "Weaponbay closed!\n" );
		}
	}
	ent->vehicle->attenuation = level.time + 1.0;
}

void Cmd_Weapon( edict_t *ent )
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;	
	if( ent->vehicle->deadflag )
		return;
	Fire_Weapon( ent->vehicle );
}

void Cmd_Flare( edict_t *ent )
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;	
	if( ent->vehicle->deadflag )
		return;
	Fire_Flare( ent->vehicle );
}

void Cmd_Brake (edict_t *ent)
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;	
	if( !(ent->vehicle->HASCAN & HASCAN_BRAKE) || ent->vehicle->deadflag )
		return;
	if( ent->client->REPEAT & REPEAT_BRAKE )
		return;

	if( ent->vehicle->DAMAGE & DAMAGE_BRAKE )
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

	ent->vehicle->ONOFF ^= ONOFF_BRAKE;

	if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
	{
		if( ent->vehicle->ONOFF & ONOFF_BRAKE )
		{
			ent->client->FLAGS |= CLIENT_NO_REPEATBRAKE;
			gi.cprintf( ent, PRINT_MEDIUM, "Speedbrakes extended!\n" );
		}
		else
		{
			ent->client->FLAGS &= ~CLIENT_NO_REPEATBRAKE;
			gi.cprintf( ent, PRINT_MEDIUM, "Speedbrakes retracted!\n" );
		}
	}
}





void Cmd_Roll (edict_t *ent)
{
	ent->client->FLAGS ^= CLIENT_AUTOROLL;
	
	if( !(ent->client->FLAGS & CLIENT_AUTOROLL) )
	{
		gi.cprintf( ent, PRINT_HIGH, "Auto-rollback turned OFF!\n" );
	}
	else
	{
		gi.cprintf( ent, PRINT_HIGH, "Auto-rollback turned ON!\n" );
	}

}






void Cmd_Coord (edict_t *ent)
{
	ent->client->FLAGS ^= CLIENT_COORD;

	if( !(ent->client->FLAGS & CLIENT_COORD ) )
	{
		gi.cprintf( ent, PRINT_HIGH, "Auto-coordination turned OFF!\n" );
	}
	else
	{
		gi.cprintf( ent, PRINT_HIGH, "Auto-coordination turned ON!\n" );
	}

}



void Cmd_Control( edict_t *ent )
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;

	if( ent->vehicle->deadflag )
		return;

	if( strcmp( ent->vehicle->classname, "plane") == 0 )
	{
		ent->client->FLAGS ^= CLIENT_ALTCONTROLS_PLANE;
		if( !(ent->client->FLAGS & CLIENT_ALTCONTROLS_PLANE ) )
		{
			ent->vehicle->think = PlaneMovement;
			gi.cprintf( ent, PRINT_HIGH, "Alternative Controls for Plane turned OFF!\n" );
		}
		else
		{
			ent->vehicle->think = PlaneMovement_Easy;
			gi.cprintf( ent, PRINT_HIGH, "Alternative Controls for Plane turned ON!\n" );
		}		
	}
	else if( strcmp( ent->vehicle->classname, "helo") == 0 )
	{
		ent->client->FLAGS ^= CLIENT_ALTCONTROLS_HELO;
		if( !(ent->client->FLAGS & CLIENT_ALTCONTROLS_HELO ) )
		{
//			ent->vehicle->think = PlaneMovement;
			gi.cprintf( ent, PRINT_HIGH, "Alternative Controls for Helo turned OFF!\n" );
		}
		else
		{
//			ent->vehicle->think = PlaneMovement_Easy;
			gi.cprintf( ent, PRINT_HIGH, "Alternative Controls for Helo turned ON!\n" );
		}
	}
	else if( strcmp( ent->vehicle->classname, "ground") == 0 )
	{
		ent->client->FLAGS ^= CLIENT_ALTCONTROLS_GROUND;
		if( !(ent->client->FLAGS & CLIENT_ALTCONTROLS_GROUND) )
		{
//			ent->vehicle->think = PlaneMovement;
			gi.cprintf( ent, PRINT_HIGH, "Alternative Controls for Ground Vehicle turned OFF!\n" );
		}
		else
		{
//			ent->vehicle->think = PlaneMovement_Easy;
			gi.cprintf( ent, PRINT_HIGH, "Alternative Controls for Ground Vehicle turned ON!\n" );
		}
	}
	else if( strcmp( ent->vehicle->classname, "LQM") == 0 )
	{
		ent->client->FLAGS ^= CLIENT_ALTCONTROLS_LQM;
		if( !(ent->client->FLAGS & CLIENT_ALTCONTROLS_LQM ) )
		{
			ent->vehicle->think = SteerLQM;
			gi.cprintf( ent, PRINT_HIGH, "Alternative Controls for LQM turned OFF!\n" );
		}
		else
		{
			ent->vehicle->think = SteerLQM_Alt;
			gi.cprintf( ent, PRINT_HIGH, "Alternative Controls for LQM turned ON!\n" );
		}
	}

}




void Cmd_Rollfix (edict_t *ent)
{
	if( ent->client->FLAGS & CLIENT_NONGL_MODE )
	{
		ent->client->FLAGS &= ~CLIENT_NONGL_MODE;
		gi.cprintf( ent, PRINT_HIGH, "Videomodefix turned OFF!\n" );
	}
	else
	{
		ent->client->FLAGS |= CLIENT_NONGL_MODE;
		gi.cprintf( ent, PRINT_HIGH, "Videomodefix turned ON!\n" );
	}
}






void Cmd_Tower( edict_t *ent )
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;
	if( ent->vehicle->deadflag )
		return;
	if( !ent->vehicle->radio_target )
	{
		gi.cprintf( ent, PRINT_HIGH, "There is nothing to bother the tower with!\n" );
		return;
	}
	gi.cprintf( ent, PRINT_HIGH, "Tower, request operation of %s!\n",
				ent->vehicle->radio_target->target );
	G_UseTargets( ent->vehicle->radio_target, ent->vehicle );
}









void Cmd_Change( edict_t *ent)
{
	if( strcmp( ent->vehicle->classname, "LQM" ) == 0 )
	{
		if( !ent->vehicle->groundentity )
			return;
		if( ent->vehicle->speed > 1 || ent->vehicle->thrust != 0 ||
			strcmp( ent->vehicle->groundentity->classname, "plane" ) == 0 ||
			strcmp( ent->vehicle->groundentity->classname, "helo" ) == 0 ||
			strcmp( ent->vehicle->groundentity->classname, "ground" ) == 0 )
			return;
		ent->action = ent->vehicle->health*100/ent->vehicle->max_health;
	}
	else
	{
		if( ent->vehicle->thrust != 0 || (ent->vehicle->health < ent->vehicle->max_health) 
				|| ent->vehicle->DAMAGE ) 
		return;
	}

	VectorCopy( ent->vehicle->s.angles, ent->s.angles );

	if( ent->vehicle->teammaster )
		ent->teammaster = ent->vehicle->teammaster;

	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		if( ent->vehicle->movetarget )
		{
			gi.unlinkentity( ent->vehicle->movetarget );
			G_FreeEdict( ent->vehicle->movetarget );
		}
		if( ent->vehicle->activator && strcmp( ent->vehicle->classname, "LQM" ) != 0 )
		{
			VectorCopy( ent->vehicle->activator->s.angles, ent->s.angles );
			gi.unlinkentity( ent->vehicle->activator );
			G_FreeEdict( ent->vehicle->activator );
		}
	}
	VectorCopy( ent->vehicle->s.origin, ent->s.origin );
	ent->s.origin[2] += 6;
	gi.sound( ent->vehicle, CHAN_BODY, gi.soundindex( "death/0.wav" ), 1, ATTN_NORM, 0 );
	RemoveVehicle( ent );

	ent->mass = -1;	// set this to indicate reselection of vehicle!
	ent->vehicle = NULL;
	ent->client->FLAGS &= ~CLIENT_VEH_SELECTED;
	ent->client->showscores = ent->client->showinventory = false;
	ent->client->selectvehicle = true;

}




void Cmd_Eject( edict_t *ent)
{
	edict_t	*LQM;
	vec3_t	up, forward, right, dir;
	int		i, VM = (ent->client->FLAGS & CLIENT_NONGL_MODE) ? 1 : -1;

#pragma message( "make LQM leave and enter helo" )

	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED ) )
		return;

	if( ent->vehicle->deadflag == DEAD_DEAD )
		return;

	if( (ent->vehicle->ONOFF & ONOFF_IS_CHANGEABLE) && (ent->vehicle->think != EmptyPlaneDie) )
	{
//		gi.bprintf( PRINT_HIGH, "Change\n" );
		Cmd_Change( ent );
		return;
	}

	if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
	{
		if( ent->vehicle->ONOFF & ONOFF_AIRBORNE )
		{
			ent->vehicle->thrust = THRUST_IDLE;
			ent->vehicle->deadflag = DEAD_DYING;
			ent->vehicle->health = 0;
			ent->vehicle->owner = NULL;
			ent->vehicle->think = misc_vehicle_think;
			ent->vehicle->touch = misc_vehicle_touch;
			ent->vehicle->pain = misc_vehicle_pain;
			ent->vehicle->die = misc_vehicle_die;
			ent->vehicle->think = misc_vehicle_think;
			ent->vehicle->touch_debounce_time = level.time;
			ent->vehicle->nextthink = level.time + 0.1;
			if( ent->vehicle->enemy )
			{
				ent->vehicle->enemy->lockstatus = LS_NOLOCK;
				ent->vehicle->enemy = NULL;
			}
			RemoveCamera( ent );
			
			LQM = G_Spawn();
			Spawn_LQM( LQM );
			LQM->health = (ent->vehicle->action*LQM->max_health)/100;	// adjust health

			VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
			VectorCopy( ent->vehicle->s.angles, LQM->s.angles );
			VectorCopy( ent->vehicle->velocity, LQM->velocity );
			VectorCopy( ent->vehicle->avelocity, LQM->avelocity );
			if( LQM->s.angles[0] > 90 && LQM->s.angles[0] < 270 )
			{
				LQM->s.angles[0] -= 180;
				LQM->s.angles[0] = anglemod( LQM->s.angles[0] );
				LQM->s.angles[1] -= 180;
				LQM->s.angles[1] = anglemod( LQM->s.angles[1] );
				LQM->s.angles[2] -= 180;
				LQM->s.angles[2] = anglemod( LQM->s.angles[2] );
			}
			VectorCopy( LQM->s.angles, dir );
			dir[2] *= VM;
			AngleVectors( dir, forward, NULL, up );
			VectorScale( up, 12, up );
			VectorAdd( LQM->s.origin, up, LQM->s.origin );
			VectorMA( LQM->s.origin, ent->vehicle->mins[0]-4, forward, LQM->s.origin );
			VectorMA( LQM->velocity, 14, up, LQM->velocity );
			LQM->camera = ent;
			LQM->aqteam = ent->vehicle->aqteam;
				
			gi.linkentity( LQM );
			
			ent->vehicle = LQM;
			ent->vehicle->groundentity = NULL;
			ent->vehicle->ONOFF = ONOFF_AUTOROTATION | ONOFF_AIRBORNE;
			ent->vehicle->drone_decision_time = level.time + 1.5;
			ent->vehicle->owner = ent;

			ent->client->cameratarget = ent->vehicle;
			ent->client->nextcam = NULL;
			ent->client->prevcam = NULL;
			ent->client->cameradistance = ent->vehicle->movedir[0];
			ent->client->cameraheight = ent->vehicle->movedir[1];
			ent->client->FLAGS &= ~CLIENT_NO_REPEATBRAKE;
		}
		else
		{
			ent->vehicle->thrust = THRUST_IDLE;
			ent->vehicle->s.skinnum = 0;
			ent->vehicle->ONOFF &= ~ONOFF_PILOT_ONBOARD;
			ent->vehicle->owner = NULL;
			if( ent->vehicle->think == PlaneMovement || ent->vehicle->think == PlaneMovement_Easy )
			{
				ent->vehicle->think = misc_vehicle_think;
				ent->vehicle->nextthink = level.time + 0.1;
			}
			if( ent->vehicle->enemy )
			{
				ent->vehicle->enemy->lockstatus = LS_NOLOCK;
				ent->vehicle->enemy = NULL;
			}
			ent->vehicle->touch = misc_vehicle_touch;
			ent->vehicle->pain = misc_vehicle_pain;
			ent->vehicle->die = misc_vehicle_die;
			ent->vehicle->touch_debounce_time = level.time;
			RemoveCamera( ent );
			LQM = G_Spawn();
			Spawn_LQM( LQM );
			LQM->health = ent->vehicle->action*LQM->max_health/100;	// adjust health

			if( ent->vehicle->speed > 30 )	// eject
			{
				LQM->gravity = 0.1;
				VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
				VectorCopy( ent->vehicle->s.angles, LQM->s.angles );
				VectorCopy( ent->vehicle->velocity, LQM->velocity );
				VectorCopy( ent->vehicle->avelocity, LQM->avelocity );
				AngleVectors( LQM->s.angles, NULL, NULL, up );
				VectorScale( up, 16, up );
				VectorAdd( LQM->s.origin, up, LQM->s.origin );
				VectorMA( LQM->velocity, 9, up, LQM->velocity );
				LQM->aqteam = ent->vehicle->aqteam;

				ent->vehicle = LQM;
				ent->vehicle->groundentity = NULL;
				ent->vehicle->ONOFF = ONOFF_AUTOROTATION | ONOFF_AIRBORNE;
				ent->vehicle->drone_decision_time = level.time + 2.0;
				ent->vehicle->owner = ent;
			}
			else							// get out
			{
				VectorSet( LQM->s.angles, 0, ent->vehicle->s.angles[1], 0 );
				AngleVectors( LQM->s.angles, NULL, NULL, up );
				VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
				VectorMA( LQM->s.origin, ent->vehicle->maxs[2]+abs(LQM->mins[2])+2, up, LQM->s.origin );
				LQM->aqteam = ent->vehicle->aqteam;

				ent->vehicle = LQM;
				ent->vehicle->ONOFF |= ONOFF_LANDED;
				ent->vehicle->owner = ent;
			}
			LQM->camera = ent;
			ent->client->cameratarget = ent->vehicle;
			ent->client->nextcam = NULL;
			ent->client->prevcam = NULL;
			ent->client->cameradistance = ent->vehicle->movedir[0];
			ent->client->cameraheight = ent->vehicle->movedir[1];
			ent->client->FLAGS &= ~CLIENT_NO_REPEATBRAKE;
			gi.linkentity( LQM );
		}
		if( teamplay->value )
		{
			// neutral team has no weapons
			if( ent->vehicle->aqteam == TEAM_NEUTRAL )
			{
				for( i = 0; i < MAX_WEAPONS; i++ )
				{
					ent->vehicle->weapon[i] = WEAPON_NONE; 
					ent->vehicle->maxammo[i] = ent->vehicle->ammo[i] = 0;
					strcpy( ent->vehicle->weaponname[i], "None" ); 
					ent->vehicle->weaponicon[i] = gi.imageindex( "None" );
					ent->vehicle->weaponicon_dam[i] = gi.imageindex( "None" );
				}
			}
		}
	}
	else if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
	{
		if( ent->vehicle->ONOFF & ONOFF_AIRBORNE )
		{
			ent->vehicle->thrust = THRUST_IDLE;
			ent->vehicle->deadflag = DEAD_DYING;
			ent->vehicle->health = 0;
			ent->vehicle->owner = NULL;
			ent->vehicle->think = misc_vehicle_think;
			ent->vehicle->touch = misc_vehicle_touch;
			ent->vehicle->pain = misc_vehicle_pain;
			ent->vehicle->die = misc_vehicle_die;
			ent->vehicle->think = misc_vehicle_think;
			ent->vehicle->touch_debounce_time = level.time;
			ent->vehicle->nextthink = level.time + 0.1;
			if( ent->vehicle->enemy )
			{
				ent->vehicle->enemy->lockstatus = LS_NOLOCK;
				ent->vehicle->enemy = NULL;
			}
			RemoveCamera( ent );
			
			LQM = G_Spawn();
			Spawn_LQM( LQM );
			LQM->health = (ent->vehicle->action*LQM->max_health)/100;	// adjust health
#pragma message("check if airborne or not when ejecting")
#pragma message("add misc vehicle helo behaviour" )
#pragma message("do not automatically open chute")
			VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
			VectorCopy( ent->vehicle->s.angles, LQM->s.angles );
			VectorCopy( ent->vehicle->velocity, LQM->velocity );
			LQM->velocity[2] += -20;
			VectorCopy( ent->vehicle->avelocity, LQM->avelocity );
			if( LQM->s.angles[0] > 90 && LQM->s.angles[0] < 270 )
			{
				LQM->s.angles[0] -= 180;
				LQM->s.angles[0] = anglemod( LQM->s.angles[0] );
				LQM->s.angles[1] -= 180;
				LQM->s.angles[1] = anglemod( LQM->s.angles[1] );
				LQM->s.angles[2] -= 180;
				LQM->s.angles[2] = anglemod( LQM->s.angles[2] );
			}
			VectorCopy( LQM->s.angles, dir );
			dir[2] *= VM;
			AngleVectors( dir, forward, right, up );// forward is side here!
			VectorMA( LQM->velocity, -14, right, LQM->velocity );
			VectorScale( up, ent->vehicle->mins[2]-18, up );
			VectorAdd( LQM->s.origin, up, LQM->s.origin );
			VectorMA( LQM->s.origin, -6, right, LQM->s.origin );
			VectorMA( LQM->s.origin, -14, forward, LQM->s.origin );
			LQM->camera = ent;
			LQM->aqteam = ent->vehicle->aqteam;
				
			gi.linkentity( LQM );
			
			ent->vehicle = LQM;
			ent->vehicle->groundentity = NULL;
			ent->vehicle->ONOFF = ONOFF_AUTOROTATION | ONOFF_AIRBORNE;
			ent->vehicle->drone_decision_time = level.time + 1.5;
			ent->vehicle->owner = ent;

			ent->client->cameratarget = ent->vehicle;
			ent->client->nextcam = NULL;
			ent->client->prevcam = NULL;
			ent->client->cameradistance = ent->vehicle->movedir[0];
			ent->client->cameraheight = ent->vehicle->movedir[1];
			ent->client->FLAGS &= ~CLIENT_NO_REPEATBRAKE;
		}
		else
		{
			ent->vehicle->thrust = THRUST_IDLE;
			ent->vehicle->s.skinnum = 0;
			ent->vehicle->ONOFF &= ~ONOFF_PILOT_ONBOARD;
			ent->vehicle->owner = NULL;
			if( ent->vehicle->think == HeloMovement || ent->vehicle->think == HeloMovement_Easy )
			{
				ent->vehicle->think = misc_vehicle_think;
				ent->vehicle->nextthink = level.time + 0.1;
			}
			if( ent->vehicle->enemy )
			{
				ent->vehicle->enemy->lockstatus = LS_NOLOCK;
				ent->vehicle->enemy = NULL;
			}
			ent->vehicle->touch = misc_vehicle_touch;
			ent->vehicle->pain = misc_vehicle_pain;
			ent->vehicle->die = misc_vehicle_die;
			ent->vehicle->touch_debounce_time = level.time;
			RemoveCamera( ent );
			LQM = G_Spawn();
			Spawn_LQM( LQM );
			LQM->health = ent->vehicle->action*LQM->max_health/100;	// adjust health

			// get out
			VectorSet( LQM->s.angles, 0, ent->vehicle->s.angles[1], 0 );
			AngleVectors( LQM->s.angles, NULL, forward, NULL );
			VectorCopy( ent->vehicle->s.origin, LQM->s.origin );
			VectorMA( LQM->s.origin, ent->vehicle->maxs[1]+abs(LQM->mins[1])+2, forward, LQM->s.origin );
			LQM->aqteam = ent->vehicle->aqteam;

			ent->vehicle = LQM;
			ent->vehicle->ONOFF |= ONOFF_LANDED;
			ent->vehicle->owner = ent;

			LQM->camera = ent;
			ent->client->cameratarget = ent->vehicle;
			ent->client->nextcam = NULL;
			ent->client->prevcam = NULL;
			ent->client->cameradistance = ent->vehicle->movedir[0];
			ent->client->cameraheight = ent->vehicle->movedir[1];
			ent->client->FLAGS &= ~CLIENT_NO_REPEATBRAKE;
			gi.linkentity( LQM );
		}
		if( teamplay->value )
		{
			// neutral team has no weapons
			if( ent->vehicle->aqteam == TEAM_NEUTRAL )
			{
				for( i = 0; i < MAX_WEAPONS; i++ )
				{
					ent->vehicle->weapon[i] = WEAPON_NONE; 
					ent->vehicle->maxammo[i] = ent->vehicle->ammo[i] = 0;
					strcpy( ent->vehicle->weaponname[i], "None" ); 
					ent->vehicle->weaponicon[i] = gi.imageindex( "None" );
					ent->vehicle->weaponicon_dam[i] = gi.imageindex( "None" );
				}
			}
		}
	}
	else if( strcmp( ent->vehicle->classname, "LQM" ) == 0 && ent->vehicle->activator )
	{
		if( ent->vehicle->enemy )
		{
			ent->vehicle->enemy->lockstatus = LS_NOLOCK;
			ent->vehicle->enemy = NULL;
		}
		LQM = ent->vehicle->activator;
		ent->vehicle->activator->action = ent->vehicle->health*100/ent->vehicle->max_health;
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
			ent->vehicle->think = PlaneMovement;
			ent->vehicle->touch = PlaneTouch;
			ent->vehicle->pain = VehiclePain;
			ent->vehicle->die = PlaneDie;
		}
/*		else if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
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
		}*/
		ent->client->cameradistance = ent->vehicle->movedir[0];
		ent->client->cameraheight = ent->vehicle->movedir[1];
		ent->vehicle->nextthink = level.time + 0.1;
		ent->vehicle->ONOFF |= ONOFF_PILOT_ONBOARD;
	}
	else if( strcmp( ent->vehicle->classname, "LQM" ) == 0 && (ent->vehicle->ONOFF & ONOFF_AIRBORNE) 
			&& (ent->vehicle->ONOFF & ONOFF_AUTOROTATION) && (ent->vehicle->ONOFF & ONOFF_GEAR) )
	{
		ent->vehicle->drone_decision_time = level.time + 1.5;
		gi.cprintf( ent, PRINT_HIGH, "Opening chute...\n" );
		ent->vehicle->ONOFF &= ~ONOFF_GEAR;
	}
}









void Cmd_Zoom( edict_t *ent )
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED ) )
		return;


	if( ent->client->fov > 60 )
		ent->client->fov = 60;
	else if( ent->client->fov > 30 && ent->client->fov <= 60)
		ent->client->fov = 30;
	else if( ent->client->fov > 10 && ent->client->fov <= 30 )
		ent->client->fov = 10;
	else if( ent->client->fov <= 10 )
		ent->client->fov = 90;
	ent->client->ps.fov = ent->client->fov;
}



void Cmd_Botscores()
{
	int i;

	for( i = 0; i < MAX_EDICTS; i++ )
	{
		if( g_edicts[i].inuse )
		{
			if( g_edicts[i].HASCAN & HASCAN_DRONE )
			{
				gi.bprintf( PRINT_HIGH, "%s has %d frags\n", botinfo[g_edicts[i].botnumber].botname, 
					g_edicts[i].count );
			}
		}
	}
}


void Cmd_GPS( edict_t *ent )
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;

	if( ent->vehicle->deadflag )
		return;

	if( !(ent->vehicle->DAMAGE & DAMAGE_GPS) && !(ent->vehicle->ONOFF & ONOFF_GPS) )
		ent->vehicle->ONOFF |= ONOFF_GPS;
	else 
		ent->vehicle->ONOFF &= ~ONOFF_GPS;
}
