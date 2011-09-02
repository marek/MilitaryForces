/************************************************/
/* Filename: aq2_veh.cpp					    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 1999-07-10							*/
/*												*/
/* Description:									*/
/* NewAirQuake2 vehicle-specific stuff   		*/
/*												*/
/************************************************/

#include "g_local.h"










void FindVehicleSpawnPoint( edict_t *vehicle )
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
		gi.error( "Invalid vehicle!\n" );

	if( deathmatch->value )
	{
		while( (spot = G_Find( spot, FOFS(classname), "info_player_deathmatch" )) != NULL )
		{
			if( spot->sounds & cat || spot->sounds == 0 )
			{
				if( !teamplay->value || (teamplay->value && (vehicle->aqteam == spot->aqteam 
						|| vehicle->aqteam == TEAM_NEUTRAL || spot->aqteam == 0) ) )
					count++;
			}
		}
		spot = NULL;
		if( count == 0 )
			gi.error( "No spawn points for this vehicle category!\n" );
		which = (int)(random()*count + 1);
//		gi.bprintf( PRINT_HIGH, "%d spots available, %d selected!\n", count, which );
		for( loop = 0; loop < which; loop ++ )
		{
			spot = G_Find( spot, FOFS(classname), "info_player_deathmatch" );
			if( !(spot->sounds & cat) && spot->sounds != 0 )
			{
				loop--;
			}
			else
			{
				if( teamplay->value && vehicle->aqteam != spot->aqteam 
					&& vehicle->aqteam != TEAM_NEUTRAL && spot->aqteam != 0 ) 
					loop--;
			}
		}
	}
	else
	{
//		gi.bprintf( PRINT_HIGH, "single player spot selected!\n" );
		while( (spot = G_Find( spot, FOFS(classname), "info_player_start" ) ) != NULL )
		{
			if( spot->sounds & cat || spot->sounds == 0 )
			{
				if( !teamplay->value || (teamplay->value && (vehicle->aqteam == spot->aqteam 
						|| vehicle->aqteam == TEAM_NEUTRAL || spot->aqteam == 0) ) )
					break;
			}
		}
	}
	if( spot == NULL )
		gi.error( "No vehicle spawn point available!\n" );

//	gi.bprintf( PRINT_HIGH, "classname: %s, sounds: %d\n", spot->classname, spot->sounds );

	VectorCopy( spot->s.origin, vehicle->s.origin );
	VectorCopy( spot->s.angles, vehicle->s.angles );

	KillBox( spot );

	if( cat == CAT_PLANE || cat == CAT_HELO )
	{
		if( spot->ONOFF & ONOFF_LANDED )
		{
			vehicle->ONOFF &= ~ONOFF_AIRBORNE;
			vehicle->ONOFF |= ONOFF_LANDED;
			if( vehicle->HASCAN & HASCAN_GEAR )
			{
				vehicle->ONOFF |= ONOFF_GEAR;
				vehicle->mins[2] -= 5;
			}
			vehicle->speed = 0;
			vehicle->thrust = THRUST_IDLE;
			vehicle->s.frame = 4;
			vehicle->s.origin[2] = spot->s.origin[2] + spot->mins[2] + abs(vehicle->mins[2]) + 2;
			vehicle->s.angles[2] = 0;
			if( vehicle->HASCAN & HASCAN_TAILDRAGGER )
				vehicle->s.angles[0] = -5;
			else
				vehicle->s.angles[0] = 0;
			VectorClear( vehicle->velocity );
			VectorClear( vehicle->avelocity );
		}
		else 
		{
			vehicle->ONOFF |= ONOFF_AIRBORNE;
			if( cat == CAT_PLANE )
				vehicle->thrust = THRUST_MILITARY;
			else
				vehicle->thrust = THRUST_HOVER;
		}
	}
	else if( cat == CAT_GROUND || cat == CAT_LQM )
	{
		if( !(spot->ONOFF & ONOFF_LANDED) )
		{
			vehicle->ONOFF |= ONOFF_AIRBORNE;
			vehicle->ONOFF &= ~(ONOFF_AUTOROTATION|ONOFF_LANDED);
			vehicle->s.angles[2] = 0;
			VectorClear( vehicle->velocity );
			VectorClear( vehicle->avelocity );
			if( cat == CAT_LQM )
			{
				vehicle->s.frame = 0;
				vehicle->s.modelindex3 = gi.modelindex ("vehicles/infantry/pilot/parachute.md2");
			}
		}
	}
}


void SpawnVehicle( edict_t *ent )
{
	int vehiclecode, i, cl = ent->client->cls;

	ent->vehicle = G_Spawn();

	// correction 
	if( ent->client->cat == CAT_HELO )
		cl = ent->client->cls - 3;
	else if( ent->client->cat == CAT_GROUND )
		cl = ent->client->cls - 6;
	else if( ent->client->cat == CAT_LQM )
		cl = ent->client->cls - 10;

	vehiclecode = ent->client->cat * 100 + cl * 10 + ent->client->veh;

//	gi.bprintf( PRINT_HIGH, "%d %d %d %d\n", vehiclecode, ent->client->cat, ent->client->cls,
//		ent->client->veh );

	SelectVehicleAndSpawnIt( ent->vehicle, vehiclecode );

	if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
	{
		if( ent->client->FLAGS & CLIENT_ALTCONTROLS_PLANE )
			ent->vehicle->think = PlaneMovement_Easy;
	}
	else if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
	{
		if( ent->client->FLAGS & CLIENT_ALTCONTROLS_HELO )
			ent->vehicle->think = HeloMovement_Easy;
	}
/*	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		if( ent->client->FLAGS & CLIENT_ALTCONTROLS_GROUND )
			ent->vehicle->think = GroundMovement_Easy;
	}*/
	else if( strcmp( ent->vehicle->classname, "LQM" ) == 0 )
	{
		if( ent->client->FLAGS & CLIENT_ALTCONTROLS_LQM )
			ent->vehicle->think = SteerLQM_Alt;
	}

	if( teamplay->value )
	{
		ent->vehicle->aqteam = ent->client->team;
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

	ent->vehicle->solid = SOLID_NOT;
	ent->vehicle->owner = ent;
	ent->vehicle->camera = ent;
	ent->vehicle->ONOFF |= ONOFF_PILOT_ONBOARD;
	ent->vehicle->autocannon_time = level.time + 0.6;
	ent->vehicle->touch_debounce_time = level.time + 2.0;	// invulnerable this time

	ent->client->fov = 90;
	gi.cvar_forceset( "fov", va("%d",ent->client->fov) );
	ent->client->cameratarget = ent->vehicle;
	ent->client->cameradistance = ent->vehicle->movedir[0];
	ent->client->cameraheight = ent->vehicle->movedir[1];
	ent->client->REPEAT = 0;
	ent->client->nextcam = NULL;
	ent->client->prevcam = NULL;
	ent->client->FLAGS |= CLIENT_VEH_SELECTED;
	ent->client->thrusttime = level.time;
	ent->client->showscores = ent->client->showinventory = 
	ent->client->selectvehicle = false;
	
	if( ent->mass == -1 )	// reselection
	{
		ent->vehicle->action = ent->action;
		if( ent->teammaster )
		{
			KillBox( ent->teammaster );
			ent->teammaster = NULL;
		}
		VectorCopy( ent->s.angles, ent->vehicle->s.angles );
		VectorCopy( ent->s.origin, ent->vehicle->s.origin );
		ent->vehicle->ONOFF &= ~ONOFF_AIRBORNE;
		ent->vehicle->ONOFF |= ONOFF_LANDED;
		if( ent->vehicle->HASCAN & HASCAN_GEAR )
		{
			ent->vehicle->ONOFF |= ONOFF_GEAR;
			ent->vehicle->mins[2] -= 5;
		}
		ent->vehicle->speed = 0;
		ent->vehicle->thrust = THRUST_IDLE;
		ent->vehicle->s.frame = 4;
		if( ent->vehicle->HASCAN & HASCAN_TAILDRAGGER )
			ent->vehicle->s.angles[0] = -5;
		else
			ent->vehicle->s.angles[0] = 0;
		VectorClear( ent->vehicle->velocity );
		VectorClear( ent->vehicle->avelocity );
		KillBox( ent->vehicle );
		gi.linkentity( ent->vehicle );
	}
	else	// new selection
	{
		ent->vehicle->action = 100;		// health of pilot
		FindVehicleSpawnPoint( ent->vehicle );
		gi.linkentity( ent->vehicle );
	}

	ent->mass = 0;
	ent->vehicle->solid = SOLID_BBOX;
}








void RemoveVehicle( edict_t *ent )
{
	int i, counter;
	edict_t *cam = NULL, *prev = NULL;

	if( !ent->vehicle )
		return;

	// get rid of all the cameras
	if( ent->vehicle->camera )
	{
		prev = ent->vehicle->camera;
		cam = ent->vehicle->camera->client->nextcam;
		counter = 1;
		while( cam != NULL )
		{
			prev = cam;
			cam = cam->client->nextcam;
			counter++;
		}
		cam = prev;
		for(i = 0; i < counter; i++ )
		{
			if( cam->client->prevcam != NULL )
				prev = cam->client->prevcam;			
			RemoveCamera( cam );
			cam = prev;
		}
	}
	G_FreeEdict( ent->vehicle );
	ent->vehicle = NULL;
	ent->client->FLAGS &= ~CLIENT_VEH_SELECTED;
}










void VehiclePain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->s.effects = EF_ROCKET;
}











/*
================
MakeDamage

Inflicts locational damage on the
specified vehicle
================
*/
void MakeDamage( edict_t *targ )
{
	int location;

	location = rand()%50;
	switch( location )
	{
		case 1:
				if( targ->DAMAGE & DAMAGE_GEAR )
					break;
				targ->DAMAGE |= DAMAGE_GEAR;
				if( targ->HASCAN & HASCAN_GEAR )
				{
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
						gi.cprintf( targ->owner, PRINT_HIGH, "Gear damaged!\n" );
				}
				break;
		case 2:
				if( targ->DAMAGE & DAMAGE_BRAKE )
					break;
				targ->DAMAGE |= DAMAGE_BRAKE;
				if( targ->HASCAN & HASCAN_BRAKE )
				{
					if( strcmp( targ->classname,"plane" ) == 0 )
					{
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
							gi.cprintf( targ->owner, PRINT_HIGH, "Speedbrakes damaged!\n" );
					}
					else if( strcmp( targ->classname,"ground" ) == 0 )
					{
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
							gi.cprintf( targ->owner, PRINT_HIGH, "Handbrake damaged!\n" );
						}
 				}
				break;
		case 3:
				if( targ->DAMAGE & DAMAGE_AFTERBURNER )
					break;
				targ->DAMAGE |= DAMAGE_AFTERBURNER;
				if( targ->thrust > THRUST_MILITARY )
					targ->thrust = THRUST_MILITARY;
				if( targ->maxthrust > THRUST_MILITARY && strcmp( targ->classname, "plane" ) == 0 )
				{
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
						gi.cprintf( targ->owner, PRINT_HIGH, "Afterburner damaged!\n" );
				}
				else if( strcmp( targ->classname,"ground" ) == 0 )
				{
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
						gi.cprintf( targ->owner, PRINT_HIGH, "Reverse damaged!\n" );
				}
				break;
		case 4:
				if( targ->DAMAGE & DAMAGE_ENGINE_80 )
					break;
				targ->DAMAGE |= DAMAGE_ENGINE_80 | DAMAGE_AFTERBURNER;
				if( targ->thrust > 8 )
					targ->thrust = 8;
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
					gi.cprintf( targ->owner, PRINT_HIGH, "Engine damaged!\n" );
				break;
		case 5:
				if( targ->DAMAGE & DAMAGE_ENGINE_60 )
					break;
				targ->DAMAGE |= DAMAGE_ENGINE_60 | DAMAGE_ENGINE_80 | DAMAGE_AFTERBURNER;
				if ( targ->thrust > 6 )
					targ->thrust = 6;
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
					gi.cprintf( targ->owner, PRINT_HIGH, "Engine damaged!\n" );
				break;
		case 6:
				if( targ->DAMAGE & DAMAGE_ENGINE_40 )
					break;
				targ->DAMAGE |= DAMAGE_ENGINE_40 | DAMAGE_ENGINE_60 | DAMAGE_ENGINE_80 | DAMAGE_AFTERBURNER;
				if( targ->thrust > 4 )
						targ->thrust = 4;
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
					gi.cprintf( targ->owner, PRINT_HIGH, "Engine damaged!\n" );
				break;
		case 7:
				if( targ->DAMAGE & DAMAGE_ENGINE_20 )
					break;
				targ->DAMAGE |= DAMAGE_ENGINE_20 | DAMAGE_ENGINE_40 | DAMAGE_ENGINE_60 | 
						DAMAGE_ENGINE_80 | DAMAGE_AFTERBURNER;
				if( targ->thrust > 2 )
					targ->thrust = 2;
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
						gi.cprintf( targ->owner, PRINT_HIGH, "Engine damaged!\n" );
				break;
		case 8:
				if( targ->DAMAGE & DAMAGE_ENGINE_DESTROYED )
					break;
				targ->DAMAGE |= DAMAGE_ENGINE_DESTROYED | DAMAGE_ENGINE_20 | DAMAGE_ENGINE_40 |
						DAMAGE_ENGINE_60 | DAMAGE_ENGINE_80 | DAMAGE_AFTERBURNER;
				if( targ->thrust > 0 )
					targ->thrust = 0;
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
						gi.cprintf( targ->owner, PRINT_HIGH, "Engine destroyed!\n" );
				if( strcmp( targ->classname, "ground" ) == 0 )
				{
					targ->DAMAGE |= DAMAGE_AFTERBURNER;
				}
				break;
		case 9:
				if( targ->DAMAGE & DAMAGE_WEAPONBAY )
					break;
				if( targ->HASCAN & HASCAN_WEAPONBAY )
				{
					targ->DAMAGE |= DAMAGE_WEAPONBAY;
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
						gi.cprintf( targ->owner, PRINT_HIGH, "Baydoors damaged!\n" );
				}
		case 10:
				if( targ->DAMAGE & DAMAGE_AUTOPILOT )
					break;
				if( targ->HASCAN & HASCAN_AUTOPILOT )
				{
					targ->DAMAGE |= DAMAGE_AUTOPILOT;
					if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
					{
						targ->ONOFF &= ~ONOFF_AUTOPILOT;
						gi.cprintf( targ->owner, PRINT_HIGH, "Autopilot damaged!\n" );
					}
				}
				break;
		case 11:
				if( targ->DAMAGE & DAMAGE_GPS )
					break;
				targ->DAMAGE |= DAMAGE_GPS;
				if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) )
				{
					targ->ONOFF &= ~ONOFF_GPS;
					gi.cprintf( targ->owner, PRINT_HIGH, "GPS damaged!\n" );
				}
				break;
		case 12:
				if( targ->DAMAGE & DAMAGE_WEAPON_0 )
					break;
				targ->DAMAGE |= DAMAGE_WEAPON_0;
				if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) &&
					(targ->weapon[0] != WEAPON_NONE) )
				{
					targ->ammo[0] = -1;
					gi.cprintf( targ->owner, PRINT_HIGH, "Autocannon damaged!\n" );
				}
				break;
		case 13:
				if( targ->DAMAGE & DAMAGE_WEAPON_1 )
					break;
				targ->DAMAGE |= DAMAGE_WEAPON_1;
				if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) &&
					(targ->weapon[1] != WEAPON_NONE) )
				{
					targ->ammo[1] = -1;
					gi.cprintf( targ->owner, PRINT_HIGH, "%s damaged!\n", targ->weaponname[1] );
				}
				break;
		case 14:
				if( targ->DAMAGE & DAMAGE_WEAPON_2 )
					break;
				targ->DAMAGE |= DAMAGE_WEAPON_2;
				if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) &&
					(targ->weapon[2] != WEAPON_NONE) )
				{
					targ->ammo[2] = -1;
					gi.cprintf( targ->owner, PRINT_HIGH, "%s damaged!\n", targ->weaponname[2] );
				}
				break;
		case 15:
				if( targ->DAMAGE & DAMAGE_WEAPON_3 )
					break;
				targ->DAMAGE |= DAMAGE_WEAPON_3;
				if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) &&
					(targ->weapon[3] != WEAPON_NONE) )
				{
					targ->ammo[3] = -1;
					gi.cprintf( targ->owner, PRINT_HIGH, "%s damaged!\n", targ->weaponname[3] );
				}
				break;
		case 16:
				if( targ->DAMAGE & DAMAGE_WEAPON_4 )
					break;
				targ->DAMAGE |= DAMAGE_WEAPON_4;
				if( (targ->ONOFF & ONOFF_PILOT_ONBOARD) && !(targ->HASCAN & HASCAN_DRONE) &&
					(targ->weapon[4] != WEAPON_NONE) )
				{
					targ->ammo[4] = -1;
					gi.cprintf( targ->owner, PRINT_HIGH, "%s damaged!\n", targ->weaponname[4] );
				}
				break;
		default:
				break;
	}
	// helos dont have afterburners
	if( strcmp( targ->classname, "helo" ) == 0 && targ->DAMAGE & DAMAGE_AFTERBURNER )
		targ->DAMAGE &= ~DAMAGE_AFTERBURNER;
}







// ---------------- //
// VehicleExplosion //
// ---------------- //
// does a radius explosion if vehicle dies

void VehicleExplosion (edict_t *vehicle)
{
	float	points;
	edict_t	*ent = NULL;
	vec3_t	v;
	vec3_t	dir;

	while ((ent = findradius(ent, vehicle->s.origin, 100)) != NULL)
	{
		if (ent == vehicle)
			continue;
		if (!ent->takedamage)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (vehicle->s.origin, v, v);
		points = 100 - 0.5 * VectorLength (v);

		if (points > 0)
		{
			if (CanDamage (ent, vehicle))
			{
				VectorSubtract (ent->s.origin, vehicle->s.origin, dir);
				T_Damage (ent, vehicle, vehicle, dir, vehicle->s.origin,
			   		 vec3_origin, (int)points, 5000, 150, MOD_EXPLOSIVE);
			}
		}
	}
}








// ---------------- //
// SetVehicleModel  //
// ---------------- //
void SetVehicleModel (edict_t *vehicle )
{

	VectorClear( vehicle->avelocity );
	vehicle->solid = SOLID_NOT;
	vehicle->takedamage = DAMAGE_NO;

	if( strcmp( vehicle->classname, "LQM" ) != 0 )
	{
		vehicle->s.modelindex = gi.modelindex( "models/objects/debris2/tris.md2" );
	}
	else 
	{
		vehicle->s.modelindex3 = 0;
		vehicle->s.frame = 56;
		return;
	}
	vehicle->movetype = MOVETYPE_TOSS;
	vehicle->s.effects = 0;
	vehicle->gravity = .8;
	vehicle->s.frame = 0;
	vehicle->s.skinnum = 0;
	VectorSet( vehicle->mins, 0, 0, 0 );
	VectorSet( vehicle->maxs, 0, 0, 0 );
	if( vehicle->activator )
	{
		gi.unlinkentity( vehicle->activator );
		G_FreeEdict( vehicle->activator );
	}
	if( vehicle->movetarget )
	{
		gi.unlinkentity( vehicle->movetarget );
		G_FreeEdict( vehicle->movetarget );
	}
	if( strcmp( vehicle->classname, "helo" ) == 0 )
	{
		vehicle->s.modelindex2 = gi.modelindex( "models/objects/debris2/tris.md2" );
	}

}





void SelectVehicleAndSpawnIt( edict_t *vehicle, int which )
{
	switch( which )
	{
	case CODE_F16:
		Spawn_F16( vehicle );
		break;
	case CODE_F14:
		Spawn_F14( vehicle );
		break;
	case CODE_F22:
		Spawn_F22( vehicle );
		break;
	case CODE_F4:
		Spawn_F4( vehicle );
		break;
	case CODE_SU37:
		Spawn_SU37( vehicle );
		break;
	case CODE_MIG21:
		Spawn_MIG21( vehicle );
		break;
	case CODE_MIG29:
		Spawn_MIG29( vehicle );
		break;
	case CODE_MIGMFI:
		Spawn_MIGMFI( vehicle );
		break;
	case CODE_TORNADO:
		Spawn_Tornado( vehicle );
		break;
	case CODE_A10:
		Spawn_A10( vehicle );
		break;
	case CODE_F117:
		Spawn_F117( vehicle );
		break;
	case CODE_SU24:
		Spawn_SU24( vehicle );
		break;
	case CODE_SU25:
		Spawn_SU25( vehicle );
		break;
	case CODE_SU34:
		Spawn_SU34( vehicle );
		break;
	case CODE_AH64:
		Spawn_AH64( vehicle );
		break;
	case CODE_MI28:
		Spawn_MI28( vehicle );
		break;
	case CODE_RAH66:
		Spawn_RAH66( vehicle );
		break;
	case CODE_KA50:
		Spawn_KA50( vehicle );
		break;
	default:
		which = 999;
		Spawn_F16( vehicle );
		break;
	}
//	gi.bprintf( PRINT_HIGH, "%s chose %d\n", botinfo[vehicle->botnumber].botname, which );
}


void FindWaypoint( edict_t *wp, int type )
{

}










void GetVehicleNames( edict_t *ent, char *name, char *pic )
{
	if( ent->client->cat & CAT_PLANE )		// plane
	{
		if( ent->client->cls == CLS_PLANE_FIGHTER )			// fighter
		{
			if( ent->client->veh == 1 )
			{
				strcpy( name, "F14 Tomcat" );
				strcpy( pic, "f-14_i" );
			}
			else if( ent->client->veh == 2 )
			{
				strcpy( name, "F22 Raptor" );
				strcpy( pic, "f-22_i" );
			}
			else if( ent->client->veh == 3 )
			{
				strcpy( name, "F16 Falcon" );
				strcpy( pic, "f-16_i" );
			}
			else if( ent->client->veh == 4 )
			{
				strcpy( name, "F4 Phantom" );
				strcpy( pic, "f-4_i" );
			}
			else if( ent->client->veh == 5 )
			{
				strcpy( name, "SU37 Super Flanker" );
				strcpy( pic, "su-37_i" );
			}
			else if( ent->client->veh == 6 )
			{
				strcpy( name, "MiG 21 Fishbed" );
				strcpy( pic, "mig21_i" );
			}
			else if( ent->client->veh == 7 )
			{
				strcpy( name, "MiG 29 Fulcrum" );
				strcpy( pic, "mig29_i" );
			}
			else if( ent->client->veh == 8 )
			{
				strcpy( name, "MiG MFI" );
				strcpy( pic, "migmfi_i" );
			}
		}
		else if( ent->client->cls == CLS_PLANE_ATTACK)			// fighterbomber
		{
			if( ent->client->veh == 1 )
			{
				strcpy( name, "GR1 Tornado" );
				strcpy( pic, "tornado_i" );
			}
			else if( ent->client->veh == 2 )
			{
				strcpy( name, "A10 Thunderbolt" );
				strcpy( pic, "a-10_i" );
			}
			else if( ent->client->veh == 3 )
			{
				strcpy( name, "F-117 Nighthawk" );
				strcpy( pic, "f-117_i" );
			}
			else if( ent->client->veh == 4 )
			{
				strcpy( name, "Su-24 Fencer" );
				strcpy( pic, "su-24_i" );
			}
			else if( ent->client->veh == 5 )
			{
				strcpy( name, "Su-25 Frogfoot" );
				strcpy( pic, "su-25_i" );
			}
			else if( ent->client->veh == 6 )
			{
				strcpy( name, "Su-34 Platypus" );
				strcpy( pic, "su-34_i" );
			}
		}
		else if( ent->client->cls == CLS_PLANE_TRANSPORT )			// transport
		{

		}
	}
	else if( ent->client->cat & CAT_HELO )	// helo
	{
		if( ent->client->cls == CLS_HELO_ATTACK )				// attack
		{
			if( ent->client->veh == 1 )
			{
				strcpy( name, "AH64 Apache" );
				strcpy( pic, "apache_i" );
			}
			else if( ent->client->veh == 2 )
			{
				strcpy( name, "Mi28 Havoc" );
				strcpy( pic, "havoc_i" );
			}
		}
		else if( ent->client->cls == CLS_HELO_SCOUT)			// scout
		{
			if( ent->client->veh == 1 )
			{
				strcpy( name, "RAH66 Comanche" );
				strcpy( pic, "comanche_i" );
			}
			else if( ent->client->veh == 2 )
			{
				strcpy( name, "Ka50 Hokum" );
				strcpy( pic, "hocum_i" );
			}
		}
		else if( ent->client->cls == CLS_HELO_TRANSPORT )			// transport
		{

		}
	}
	else if( ent->client->cat & CAT_GROUND )	// ground
	{
		if( ent->client->cls == CLS_GROUND_MBT )				// mbt
		{
			if( ent->client->veh == 1 )
			{
				strcpy( name, "M1A2 MBT" );
				strcpy( pic, "m1a2_i" );
			}
			else if( ent->client->veh == 2 )
			{
				strcpy( name, "T72 MBT" );
				strcpy( pic, "t-72_i" );
			}
		}
		else if( ent->client->cls == CLS_GROUND_APC)			// apc
		{
			if( ent->client->veh == 1 )
			{
				strcpy( name, "Scorpion" );
				strcpy( pic, "scorpion_i" );
			}
			else if( ent->client->veh == 2 )
			{
				strcpy( name, "BRDM-2" );
				strcpy( pic, "brdm2_i" );
			}
		}
		else if( ent->client->cls == CLS_GROUND_SCOUT )			// scout
		{

		}
		else if( ent->client->cls == CLS_GROUND_SPECIAL )			// special
		{
			if( ent->client->veh == 1 )
			{
				strcpy( name, "Lav 25" );
				strcpy( pic, "lav25_i" );
			}
			else if( ent->client->veh == 2 )
			{
				strcpy( name, "Rapier" );
				strcpy( pic, "rapier_i" );
			}
			else if( ent->client->veh == 3 )
			{
				strcpy( name, "2S6 Tunguska" );
				strcpy( pic, "2s6_i" );
			}
			else if( ent->client->veh == 4 )
			{
				strcpy( name, "SA-9" );
				strcpy( pic, "sa9_i" );
			}
		}
	}
	else if( ent->client->cat & CAT_LQM )	// infantry
	{
		if( ent->client->cls == CLS_LQM_PILOT )				// pilot/driver
		{

		}
		else if( ent->client->cls == CLS_LQM_INFANTRY)			// team
		{

		}
	}
}

void VehicleSelectionPrev( edict_t *ent )
{
	int		present_cat;

	// SELECT TEAM
	if( ent->client->sel_step == SEL_TEAM )
	{
		if( ent->client->team < TEAM_NEUTRAL )
			ent->client->team<<=1;
		else
			ent->client->team = TEAM_1;
	}
	// SELECT CATEGORY
	else if( ent->client->sel_step == SEL_CAT )
	{
		present_cat = ent->client->cat;
		ent->client->cat<<=1;
		if( ent->client->cat == CAT_TOO_HIGH )
			ent->client->cat = CAT_PLANE;
		// put selection on a valid vehicle
		while( ent->client->cat != present_cat )
		{
			if( !((teamplay->value?teams[ent->client->team].category:level.category) & ent->client->cat) )
			{
				ent->client->cat<<=1;
				if( ent->client->cat == CAT_TOO_HIGH )
					ent->client->cat = CAT_PLANE;
			}
			else
				break;
		}
		if( ent->client->cat & present_cat )
		{
			if( !((teamplay->value?teams[ent->client->team].category:level.category) & ent->client->cat) )
				gi.error( "No spawn points available!" );
		}
	}
	// SELECT CLASS
	else if( ent->client->sel_step == SEL_CLS )
	{
		ent->client->cls++;
		if( ent->client->cat & CAT_PLANE )
		{
			if( ent->client->cls > CLS_PLANE_ATTACK )
				ent->client->cls = CLS_PLANE_FIGHTER;
		}
		else if( ent->client->cat & CAT_HELO )
		{
			if( ent->client->cls > CLS_HELO_SCOUT )
				ent->client->cls = CLS_HELO_ATTACK;
		}
		else if( ent->client->cat & CAT_GROUND )
		{

		}
		else if( ent->client->cat & CAT_LQM )
		{

		}
	}
	// SELECT VEHICLE
	else if( ent->client->sel_step == SEL_VEH )
	{
		ent->client->veh++;
		if( teamplay->value && ent->client->team != TEAM_NEUTRAL )
		{
			if( ent->client->team == TEAM_2 )
			{
				if( ent->client->veh > max_vehicles[ent->client->cls] )
					ent->client->veh = max_vehicles[ent->client->cls]/2+1;
			}
			else
			{
				if( ent->client->veh > max_vehicles[ent->client->cls]/2 )
					ent->client->veh = 1;
			}
		}
		else
		{
			if( ent->client->veh > max_vehicles[ent->client->cls] )
				ent->client->veh = 1;
		}
	}

	ent->client->thrusttime = level.time + 0.3;
//	gi.bprintf( PRINT_HIGH, "%d\n", ent->client->team );
}

void VehicleSelectionNext( edict_t *ent )
{
	int	present_cat;

	// SELECT TEAM
	if( ent->client->sel_step == SEL_TEAM )
	{
		if( ent->client->team > TEAM_1 )
			ent->client->team>>=1;
		else
			ent->client->team = TEAM_NEUTRAL;
	}
	// SELECT CATEGORY
	else if( ent->client->sel_step == SEL_CAT )
	{
		present_cat = ent->client->cat;
		ent->client->cat>>=1;
		if( ent->client->cat == 0 )
			ent->client->cat = (CAT_TOO_HIGH>>1);
		// put selection on a valid vehicle
		while( ent->client->cat != present_cat )
		{
			if( !((teamplay->value?teams[ent->client->team].category:level.category) & ent->client->cat) )
			{
				ent->client->cat>>=1;
				if( ent->client->cat == 0 )
					ent->client->cat = (CAT_TOO_HIGH>>1);
			}
			else
				break;
		}
		if( ent->client->cat & present_cat )
		{
			if( !((teamplay->value?teams[ent->client->team].category:level.category) & ent->client->cat) )
				gi.error( "No spawn points available!" );
		}
	}
	// SELECT CLASS
	else if( ent->client->sel_step == SEL_CLS )
	{
		ent->client->cls--;
		if( ent->client->cat & CAT_PLANE )
		{
			if( ent->client->cls < CLS_PLANE_FIGHTER )
				ent->client->cls = CLS_PLANE_ATTACK;
		}
		else if( ent->client->cat & CAT_HELO )
		{
			if( ent->client->cls < CLS_HELO_ATTACK )
				ent->client->cls = CLS_HELO_SCOUT;
		}
		else if( ent->client->cat & CAT_GROUND )
		{

		}
		else if( ent->client->cat & CAT_LQM )
		{

		}
	}
	// SELECT VEHICLE
	else if( ent->client->sel_step == SEL_VEH )
	{
		ent->client->veh--;
		if( teamplay->value && ent->client->team != TEAM_NEUTRAL )
		{
			if( ent->client->team == TEAM_2 )
			{
				if( ent->client->veh < max_vehicles[ent->client->cls]/2+1 )
					ent->client->veh = max_vehicles[ent->client->cls];
			}
			else
			{
				if( ent->client->veh < 1 )
					ent->client->veh = max_vehicles[ent->client->cls]/2;
			}
		}
		else
		{
			if( ent->client->veh < 1 )
				ent->client->veh = max_vehicles[ent->client->cls];
		}
	}

	ent->client->thrusttime = level.time + 0.3;
//	gi.bprintf( PRINT_HIGH, "%d\n", ent->client->team );
}

void SelectionStep( edict_t *ent )
{
	int lowest = 1;

	// SELECT TEAM
	if( ent->client->sel_step == SEL_TEAM )
	{
		ent->client->sel_step++;
		if( !ent->client->cat )
			ent->client->cat = CAT_PLANE;
		teams[ent->client->team].players++;
		// put selection on a valid vehicle
		while( ent->client->cat < CAT_TOO_HIGH )
		{
			if( !(level.category & ent->client->cat) )
				ent->client->cat<<=1;
			else
				break;
		}
		if( ent->client->cat & CAT_TOO_HIGH )
			gi.error( "No spawn points available!" );
	}
	// SELECT CATEGORY
	else if( ent->client->sel_step == SEL_CAT )
	{
		ent->client->sel_step++;

		if( ent->client->cat == CAT_PLANE )
		{
			if( !ent->client->cls || ent->client->cls < CLS_PLANE_FIGHTER || ent->client->cls > CLS_PLANE_ATTACK )
				ent->client->cls = CLS_PLANE_FIGHTER;
		}
		else if( ent->client->cat == CAT_HELO )
		{
			if( !ent->client->cls || ent->client->cls < CLS_HELO_ATTACK || ent->client->cls > CLS_HELO_SCOUT )
				ent->client->cls = CLS_HELO_ATTACK;
		}
/*		else if( ent->client->cat == CAT_GROUND )
		{
			if( !ent->client->cls || ent->client->cls < CLS_PLANE_FIGHTER || ent->client->cls > CLS_PLANE_ATTACK )
				ent->client->cls = CLS_GROUND_MBT;
		}*/
		else if( ent->client->cat == CAT_LQM )
		{
			if( !ent->client->cls || ent->client->cls < CLS_LQM_PILOT || ent->client->cls > CLS_LQM_INFANTRY )
				ent->client->cls = CLS_LQM_PILOT;
		}

	}
	// SELECT CLASS
	else if( ent->client->sel_step == SEL_CLS )
	{
		if( teamplay->value && ent->client->team == TEAM_2 )
		{
			lowest = max_vehicles[ent->client->cls]/2+1;
		}
		ent->client->sel_step++;
		ent->client->veh = lowest;
	}
	// SELECT VEHICLE
	else if( ent->client->sel_step == SEL_VEH )
	{
		ent->client->selectvehicle = false;
		SpawnVehicle( ent );
	}
}

void VehicleSelectionDisplay( edict_t *ent )
{
	char		string[1024];
	char		string1[24],string2[24],string3[24],string4[24];

	// SELECT TEAM
	if( ent->client->sel_step == SEL_TEAM )
	{
		if( !teamplay->value )
		{
			// put selection on a valid vehicle
			while( ent->client->cat < CAT_TOO_HIGH )
			{
				if( !(level.category & ent->client->cat) )
					ent->client->cat<<=1;
				else
					break;
			}
			if( ent->client->cat & CAT_TOO_HIGH )
				gi.error( "No spawn points available!" );
			ent->client->sel_step = SEL_CAT;
			return;
		}
		sprintf( string1, " (%d players, %.0f bots)", teams[TEAM_1].players, num_bots_team1->value );
		sprintf( string2, " (%d players, %.0f bots)", teams[TEAM_2].players, num_bots_team2->value );
		sprintf( string3, " (%d players, %.0f bots)", teams[TEAM_NEUTRAL].players, num_bots_neutral->value );
		Com_sprintf (string, sizeof(string),
				"xv 60 yv 40 string2 \"%s\" "
				"xv 60 yv 47 string2 \"%s\" "
				"xv 60 yv 60 \"%s\" \"%s\" "
				"xv 60 yv 70 string2 \"%s\" "
				"xv 60 yv 90 \"%s\" \"%s\" "
				"xv 60 yv 100 string2 \"%s\" "
				"xv 60 yv 120 \"%s\" \"%s\" "
				"xv 60 yv 130 string2 \"%s\" "
				"xv 60 yv 145 string2 \"%s\" "
				"xv 60 yv 155 string2 \"%s\" "
				"xv 60 yv 165 string2 \"%s\" ",

				"Select Team:",
				"----------------",
				(ent->client->team == TEAM_1 ? "string" : "string2"),
				name_team1->string,
				string1,
				(ent->client->team == TEAM_2 ? "string" : "string2"),
				name_team2->string,
				string2,
				(ent->client->team == TEAM_NEUTRAL ? "string" : "string2"),
				name_neutral->string,
				string3,
				"INCREASE and DECREASE",
				"to cycle, FIRE to",
			    "select, ESC to go back"
				);

			gi.WriteByte (svc_layout);
			gi.WriteString (string);
			gi.unicast(ent,true);

	}
	// SELECT CATEGORY
	else if( ent->client->sel_step == SEL_CAT )
	{
		if( (teamplay->value?teams[ent->client->team].category:level.category) & CAT_PLANE )
			strcpy( string1, "Plane" );
		else
			strcpy( string1, "(Plane)" );
		if( (teamplay->value?teams[ent->client->team].category:level.category) & CAT_HELO )
			strcpy( string2, "Helo" );
		else
			strcpy( string2, "(Helo)" );
		if( (teamplay->value?teams[ent->client->team].category:level.category) & CAT_GROUND )
			strcpy( string3, "Ground" );
		else
			strcpy( string3, "(Ground)" );
		if( (teamplay->value?teams[ent->client->team].category:level.category) & CAT_LQM )
			strcpy( string4, "Infantry" );
		else
			strcpy( string4, "(Infantry)" );
		Com_sprintf (string, sizeof(string),
				"xv 60 yv 40 string2 \"%s\" "
				"xv 60 yv 47 string2 \"%s\" "
				"xv 60 yv 80 \"%s\" \"%s\" "
				"xv 60 yv 90 \"%s\" \"%s\" "
				"xv 60 yv 100 \"%s\" \"%s\" "
				"xv 60 yv 110 \"%s\" \"%s\" "
				"xv 60 yv 145 string2 \"%s\" "
				"xv 60 yv 155 string2 \"%s\" "
				"xv 60 yv 165 string2 \"%s\" ",

				"Select Category:",
				"----------------",
				(ent->client->cat == CAT_PLANE ? "string" : "string2"),
				string1,
				(ent->client->cat == CAT_HELO ? "string" : "string2"),
				string2,
				(ent->client->cat == CAT_GROUND ? "string" : "string2"),
				string3,
				(ent->client->cat == CAT_LQM ? "string" : "string2"),
				string4,
				"INCREASE and DECREASE",
				"to cycle, FIRE to",
			    "select, ESC to go back"
				);

		gi.WriteByte (svc_layout);
		gi.WriteString (string);
		gi.unicast(ent,true);
	}
	// SELECT CLASS
	else if( ent->client->sel_step == SEL_CLS )
	{
		int	cl1, cl2, cl3, cl4;

		if( ent->client->cat & CAT_PLANE ) // plane
		{
			strcpy( string1, "Fighter" );
			strcpy( string2, "Ground-Attack" );
			strcpy( string3, "(Transport)" );
			strcpy( string4, "" );
			cl1 = CLS_PLANE_FIGHTER;
			cl2 = CLS_PLANE_ATTACK;
			cl3 = CLS_PLANE_TRANSPORT;
			cl4 = CLS_PLANE_FIGHTER;
		}
		else if( ent->client->cat & CAT_HELO ) // helo
		{
			strcpy( string1, "Attack" );
			strcpy( string2, "Scout" );
			strcpy( string3, "(Transport)" );
			strcpy( string4, "" );
			cl1 = CLS_HELO_ATTACK;
			cl2 = CLS_HELO_SCOUT;
			cl3 = CLS_HELO_TRANSPORT;
			cl4 = CLS_HELO_ATTACK;
		}
		else if( ent->client->cat & CAT_GROUND ) // ground
		{
			strcpy( string1, "Tank" );
			strcpy( string2, "Scout" );
			strcpy( string3, "(APC)" );
			strcpy( string4, "Special" );
			cl1 = CLS_GROUND_MBT;
			cl2 = CLS_GROUND_SCOUT;
			cl3 = CLS_GROUND_APC;
			cl4 = CLS_GROUND_SPECIAL;
		}
		else if( ent->client->cat & CAT_LQM ) // infantry
		{
			strcpy( string1, "(Pilot/Driver)" );
			strcpy( string2, "(Infantry Team)" );
			strcpy( string3, "" );
			strcpy( string4, "" );
			cl1 = CLS_LQM_PILOT;
			cl2 = CLS_LQM_INFANTRY;
			cl3 = CLS_LQM_PILOT;
			cl4 = CLS_LQM_PILOT;
		}

		Com_sprintf (string, sizeof(string),
				"xv 60 yv 40 string2 \"%s\" "
				"xv 60 yv 47 string2 \"%s\" "
				"xv 60 yv 80 \"%s\" \"%s\" "
				"xv 60 yv 90 \"%s\" \"%s\" "
				"xv 60 yv 100 \"%s\" \"%s\" "
				"xv 60 yv 110 \"%s\" \"%s\" "
				"xv 60 yv 145 string2 \"%s\" "
				"xv 60 yv 155 string2 \"%s\" "
				"xv 60 yv 165 string2 \"%s\" ",

				"Select Class:",
				"----------------",
				(ent->client->cls == cl1 ? "string" : "string2"),
				string1,
				(ent->client->cls == cl2 ? "string" : "string2"),
				string2,
				(ent->client->cls == cl3 ? "string" : "string2"),
				string3,
				(ent->client->cls == cl4 ? "string" : "string2"),
				string4,
				"INCREASE and DECREASE",
				"to cycle, FIRE to",
			    "select, ESC to go back"
				);

		gi.WriteByte (svc_layout);
		gi.WriteString (string);
		gi.unicast(ent,true);
	}
	// SELECT VEHICLE
	else if( ent->client->sel_step == SEL_VEH )
	{
		GetVehicleNames( ent, string1, string2 );
		Com_sprintf (string, sizeof(string),
				"xv 139 yv 35 picn \"%s\" "
				"xv 60 yv 40 string2 \"%s\" "
				"xv 60 yv 47 string2 \"%s\" "
				"xv 60 yv 57 string \"%s\" "
				"xv 60 yv 145 string2 \"%s\" "
				"xv 60 yv 155 string2 \"%s\" "
				"xv 60 yv 165 string2 \"%s\" ",

				string2,
				"Select Type:",
				"------------",
				string1,
				"INCREASE and DECREASE",
				"to cycle, FIRE to",
			    "select, ESC to go back"
				);
		gi.WriteByte (svc_layout);
		gi.WriteString (string);
		gi.unicast(ent,true);
	}
}




void VehicleInventory( edict_t *ent )
{
	char		string[1024] = "";
	char		strings[MAX_WEAPONS][30], flarestring[12];
	int			i;

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		if( ent->vehicle->weapon[i] == WEAPON_NONE )
			strcpy( strings[i], "" );
		else
		{
			if( ent->vehicle->ammo[i] >= 0 )
			{
				if( ent->vehicle->weapon[i] == WEAPON_AUTOCANNON || ent->vehicle->weapon[i] == WEAPON_MAINGUN ||
					ent->vehicle->weapon[i] == WEAPON_ARTILLERY )
				{
					sprintf( strings[i], "%3d %s %s", ent->vehicle->ammo[i], ent->vehicle->weaponname[i],
						(ent->vehicle->ammo[i]>1 ? "Rounds" : "Round" ) );
				}
				else if( ent->vehicle->weapon[i] == WEAPON_JAMMER )
				{
					sprintf( strings[i], "  1 Jammer (%s)", (ent->vehicle->ONOFF & ONOFF_JAMMER ? "ON" : "OFF") );
				}
				else
				{
					sprintf( strings[i], "%3d %s%s", ent->vehicle->ammo[i], ent->vehicle->weaponname[i],
						(ent->vehicle->ammo[i]>1 ? "s" : "" ) );
				}
			}
			else
			{
				sprintf( strings[i], "--- %s", ent->vehicle->weaponname[i] );
			}
		}
	}

	sprintf( flarestring, "%3d Flare%s", ent->vehicle->flares, ( ent->vehicle->flares != 1 ? "s" : "" ) );

	Com_sprintf (string, sizeof(string),
				"xv 60 yv 40 string2 \"%s\" "
				"xv 60 yv 47 string2 \"%s\" "
				"xv 60 yv 70 \"%s\" \"%s\" "
				"xv 60 yv 80 \"%s\" \"%s\" "
				"xv 60 yv 90 \"%s\" \"%s\" "
				"xv 60 yv 100 \"%s\" \"%s\" "
				"xv 60 yv 110 \"%s\" \"%s\" "
				"xv 60 yv 120 \"%s\" \"%s\" ",

				"Weapon Status:",
				"----------------",
				((ent->vehicle->DAMAGE & DAMAGE_WEAPON_0) ? "string2" : "string"),
				strings[0],
				((ent->vehicle->DAMAGE & DAMAGE_WEAPON_1) ? "string2" : "string"),
				strings[1],
				((ent->vehicle->DAMAGE & DAMAGE_WEAPON_2) ? "string2" : "string"),
				strings[2],
				((ent->vehicle->DAMAGE & DAMAGE_WEAPON_3) ? "string2" : "string"),
				strings[3],
				((ent->vehicle->DAMAGE & DAMAGE_WEAPON_4) ? "string2" : "string"),
				strings[4],
				((ent->vehicle->DAMAGE & DAMAGE_FLARE) ? "string2" : "string" ),
				flarestring
				);
	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast(ent,false);
}




