// file: p_vehicle.c by Bjoern Drabeck
// general vehicle functions (touch, die, pain, names, check if on runwayy etc)
// for the time here are also the new spawn functions, but they will be
// relocated at a later time


#include "g_local.h"
#include "p_vehicle.h"


void Get_Vehicle_Names_f( edict_t *ent )
{
	if( ent->veh_category & 1 )		// plane
	{
		if( ent->veh_class == 1 )				// fighter
		{
			if( ent->veh_number == 1 )
			{
				strcpy( ent->veh_name, "F14 Tomcat" );
				strcpy( ent->veh_picname, "f-14_i" );
			}
			else if( ent->veh_number == 2 )
			{
				strcpy( ent->veh_name, "F22 Raptor" );
				strcpy( ent->veh_picname, "f-22_i" );
			}
			else if( ent->veh_number == 3 )
			{
				strcpy( ent->veh_name, "F16 Falcon" );
				strcpy( ent->veh_picname, "f-16_i" );
			}
			else if( ent->veh_number == 4 )
			{
				strcpy( ent->veh_name, "F4 Phantom" );
				strcpy( ent->veh_picname, "f-4_i" );
			}
			else if( ent->veh_number == 5 )
			{
				strcpy( ent->veh_name, "SU37 Super Flanker" );
				strcpy( ent->veh_picname, "su-37_i" );
			}
			else if( ent->veh_number == 6 )
			{
				strcpy( ent->veh_name, "MiG 21 Fishbed" );
				strcpy( ent->veh_picname, "mig21_i" );
			}
			else if( ent->veh_number == 7 )
			{
				strcpy( ent->veh_name, "MiG 29 Fulcrum" );
				strcpy( ent->veh_picname, "mig29_i" );
			}
			else if( ent->veh_number == 8 )
			{
				strcpy( ent->veh_name, "MiG MFI" );
				strcpy( ent->veh_picname, "migmfi_i" );
			}
		}
		else if( ent->veh_class == 2)			// fighterbomber
		{
			if( ent->veh_number == 1 )
			{
				strcpy( ent->veh_name, "GR1 Tornado" );
				strcpy( ent->veh_picname, "tornado_i" );
			}
			else if( ent->veh_number == 2 )
			{
				strcpy( ent->veh_name, "A10 Thunderbolt" );
				strcpy( ent->veh_picname, "a-10_i" );
			}
			else if( ent->veh_number == 3 )
			{
				strcpy( ent->veh_name, "F-117 Nighthawk" );
				strcpy( ent->veh_picname, "f-117_i" );
			}
			else if( ent->veh_number == 4 )
			{
				strcpy( ent->veh_name, "Su-24 Fencer" );
				strcpy( ent->veh_picname, "su-24_i" );
			}
			else if( ent->veh_number == 5 )
			{
				strcpy( ent->veh_name, "Su-25 Frogfoot" );
				strcpy( ent->veh_picname, "su-25_i" );
			}
			else if( ent->veh_number == 6 )
			{
				strcpy( ent->veh_name, "Su-34 Platypus" );
				strcpy( ent->veh_picname, "su-34_i" );
			}
		}
		else if( ent->veh_class == 3 )			// transport
		{

		}
	}
	else if( ent->veh_category & 2 )	// helo
	{
		if( ent->veh_class == 1 )				// attack
		{
			if( ent->veh_number == 1 )
			{
				strcpy( ent->veh_name, "AH64 Apache" );
				strcpy( ent->veh_picname, "apache_i" );
			}
			else if( ent->veh_number == 2 )
			{
				strcpy( ent->veh_name, "Mi28 Havoc" );
				strcpy( ent->veh_picname, "havoc_i" );
			}
		}
		else if( ent->veh_class == 2)			// scout
		{
			if( ent->veh_number == 1 )
			{
				strcpy( ent->veh_name, "RAH66 Comanche" );
				strcpy( ent->veh_picname, "comanche_i" );
			}
			else if( ent->veh_number == 2 )
			{
				strcpy( ent->veh_name, "Ka50 Hokum" );
				strcpy( ent->veh_picname, "hocum_i" );
			}
		}
		else if( ent->veh_class == 3 )			// transport
		{

		}
	}
	else if( ent->veh_category & 4 )	// ground
	{
		if( ent->veh_class == 1 )				// mbt
		{
			if( ent->veh_number == 1 )
			{
				strcpy( ent->veh_name, "M1A2 MBT" );
				strcpy( ent->veh_picname, "m1a2_i" );
			}
			else if( ent->veh_number == 2 )
			{
				strcpy( ent->veh_name, "T72 MBT" );
				strcpy( ent->veh_picname, "t-72_i" );
			}
		}
		else if( ent->veh_class == 2)			// apc
		{
			if( ent->veh_number == 1 )
			{
				strcpy( ent->veh_name, "Scorpion" );
				strcpy( ent->veh_picname, "scorpion_i" );
			}
			else if( ent->veh_number == 2 )
			{
				strcpy( ent->veh_name, "BRDM-2" );
				strcpy( ent->veh_picname, "brdm2_i" );
			}
		}
		else if( ent->veh_class == 3 )			// scout
		{

		}
		else if( ent->veh_class == 4 )			// special
		{
			if( ent->veh_number == 1 )
			{
				strcpy( ent->veh_name, "Lav 25" );
				strcpy( ent->veh_picname, "lav25_i" );
			}
			else if( ent->veh_number == 2 )
			{
				strcpy( ent->veh_name, "Rapier" );
				strcpy( ent->veh_picname, "rapier_i" );
			}
			else if( ent->veh_number == 3 )
			{
				strcpy( ent->veh_name, "2S6 Tunguska" );
				strcpy( ent->veh_picname, "2s6_i" );
			}
			else if( ent->veh_number == 4 )
			{
				strcpy( ent->veh_name, "SA-9" );
				strcpy( ent->veh_picname, "sa9_i" );
			}
		}
	}
	else if( ent->veh_category & 8 )	// infantry
	{
		if( ent->veh_class == 1 )				// pilot/driver
		{

		}
		else if( ent->veh_class == 2)			// team
		{

		}
	}
}



void vehicle_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->s.effects = EF_ROCKET;
}

void vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	char	selfname[40],attname[40];

//	self->s.frame = 0;		// eventuell hier setzen!

	if( teamplay->value == 1 )
	{
		sprintf( selfname, "[%s]%s", team[self->owner->client->pers.team-1].name,
			self->owner->client->pers.netname );
		if( attacker->owner )
		{
			sprintf( attname,"[%s]%s", team[attacker->owner->client->pers.team-1].name,
				attacker->owner->client->pers.netname );
		}
		else
			strcpy( attname, "Somebody" );
	}
	else
	{
		strcpy( selfname, self->owner->client->pers.netname );
		if( attacker->owner )
		{
			strcpy( attname, attacker->owner->client->pers.netname );
		}
		else
			strcpy( attname, "Somebody" );
	}

	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);
//	VehicleExplosion( self );

	if( attacker != self )
	{
		if( attacker->owner )
		{
			if( teamplay->value == 1 )
			{
				if( self->aqteam != attacker->aqteam )
				{
					if( attacker->owner->client && !self->deadflag )
					{
						attacker->owner->client->resp.score += 3;
						team[attacker->owner->client->pers.team-1].score += 3;
					}
				}
				else
				{
					if( attacker->owner->client && !self->deadflag )
					{
						attacker->owner->client->resp.score -= 3;
						team[attacker->owner->client->pers.team-1].score -= 3;
					}
				}
			}
			else
			{
				if( attacker->owner->client && !self->deadflag )
				{
					attacker->owner->client->resp.score += 3;
					team[attacker->owner->client->pers.team-1].score += 3;
				}
			}
		}
	}

	self->deadflag = DEAD_DYING;
	self->thrust = THRUST_0;
	if(	strcmp( self->classname, "plane" ) == 0 ||
		strcmp( self->classname, "helo" ) == 0 )
	{
		if( strcmp( self->classname, "helo" ) == 0 )
			self->avelocity[2] = 120;
		else
			self->avelocity[2] = 60;
		self->avelocity[1] = 0;
		if( self->ONOFF & LANDED )
		{
			self->deadflag = DEAD_DEAD;
			SetVehicleModel( self );
		}
	}
	else if( strcmp( self->classname, "ground" ) == 0 )
	{
		self->deadflag = DEAD_DEAD;
		self->owner->cam_target = self;
		SetVehicleModel( self );
	}

	if( !attacker->owner )
	{
		if( attacker->HASCAN & IS_DRONE )
		{
			if( teamplay->value == 1 )
				gi.bprintf( PRINT_MEDIUM, "%s was shot down by a [%s]drone (%s)! \n",
					selfname,
					team[attacker->aqteam-1].name,
		   			attacker->friendlyname );
			else
				gi.bprintf( PRINT_MEDIUM, "%s was shot down by a drone (%s)! \n",
					self->owner->client->pers.netname
		   			, attacker->friendlyname );
			if( teamplay->value == 1 )
			{
				team[attacker->aqteam-1].score += 3;
			}
		}
		return;
	}

	if( attacker != self )
	{
		if( !(attacker->HASCAN & IS_DRONE) )
		{
			if( attacker->owner->client && self->owner->client->death_msg_sent != 1 &&
					( teamplay->value != 1 || (teamplay->value == 1 && self->aqteam != attacker->aqteam) ) )
			{
				if( attacker->enemy )
				{
					if( attacker->enemy == self )
						attacker->enemy = NULL;
				}
				if( meansOfDeath == MOD_AUTOCANNON )
					gi.bprintf( PRINT_MEDIUM, "%s was outgunned by %s\n", selfname,
				   			attname );
				else if( meansOfDeath == MOD_SIDEWINDER )
					gi.bprintf( PRINT_MEDIUM, "%s couldn't evade %s's Sidewinder\n", selfname,
							attname );
				else if( meansOfDeath == MOD_STINGER )
					gi.bprintf( PRINT_MEDIUM, "%s found out that %s's Stingers are cheap but efficient\n", selfname,
							attname );
				else if( meansOfDeath == MOD_AMRAAM )
					gi.bprintf( PRINT_MEDIUM, "%s's AMRAAM ruined %s's whole day!\n", attname,
							selfname );
				else if( meansOfDeath == MOD_PHOENIX )
					gi.bprintf( PRINT_MEDIUM, "%s never saw %s's Phoenix coming\n", selfname,
							attname );
				else if( meansOfDeath == MOD_LGB )
					gi.bprintf( PRINT_MEDIUM, "%s was kissed by %s's LGB\n", selfname,
							attname );
				else if( meansOfDeath == MOD_MAVERICK )
					gi.bprintf( PRINT_MEDIUM, "%s wasn't fast enough for %s's Maverick\n", selfname,
							attname );
				else if( meansOfDeath == MOD_ATGM )
					gi.bprintf( PRINT_MEDIUM, "%s was hunted down by %s's ATGM\n", selfname,
							attname );
				else if( meansOfDeath == MOD_BOMBS )
					gi.bprintf( PRINT_MEDIUM, "%s was bombed to rubbish by %s\n", selfname,
							attname );
				else if( meansOfDeath == MOD_MAINGUN )
					gi.bprintf( PRINT_MEDIUM, "%s became familiar with %s's Artillery\n", selfname,
							attname );
				else if( meansOfDeath == MOD_ROCKET )
					gi.bprintf( PRINT_MEDIUM, "%s was killed by %s's Rockets\n", selfname,
							attname );
				else if( meansOfDeath == MOD_EXPLOSIVE )
					gi.bprintf( PRINT_MEDIUM, "%s was killed by an explosion!\n",
							selfname );
				else
					gi.bprintf( PRINT_MEDIUM, "%s was shot down by %s\n", selfname,
							attname );
				self->owner->client->death_msg_sent = 1;
			}
			else if( self->aqteam == attacker->aqteam )
			{
				gi.bprintf( PRINT_MEDIUM, "%s was killed by his stupid teammate %s\n", selfname,
						attname );
				self->owner->client->death_msg_sent = 1;
			}
		}
	}
	else // killed yourself
	{
		meansOfDeath = MOD_SELFKILL;
		gi.bprintf( PRINT_HIGH, "%s has to learn how to handle his vehicle!\n", selfname );
	}
}

void checkifonrunway( edict_t *ent )
{
	trace_t trace;
	vec3_t underneath;


	VectorSet( underneath, ent->vehicle->s.origin[0], ent->vehicle->s.origin[1],
				ent->vehicle->s.origin[2] - 128 );

	trace = gi.trace( ent->vehicle->s.origin, NULL, NULL, underneath, ent->vehicle, MASK_ALL );

	if( strcmp( trace.ent->classname, "func_runway" ) == 0 )
	{
		ent->vehicle->ONOFF |= LANDED;
		if( ent->vehicle->HASCAN & HAS_LANDINGGEAR )
			ent->vehicle->ONOFF |= GEARDOWN;
		ent->vehicle->actualspeed = ent->vehicle->currenttop = 0;
		ent->vehicle->thrust = THRUST_0;
		ent->vehicle->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2] + abs(ent->vehicle->mins[2])+1;
		ent->vehicle->s.angles[2] = ent->vehicle->s.angles[0] = 0;
		VectorClear( ent->vehicle->velocity );
		VectorClear( ent->vehicle->avelocity );
	}
}

void checkiflanded( edict_t *ent )
{
	trace_t trace;
	vec3_t underneath;


	VectorSet( underneath, ent->vehicle->s.origin[0], ent->vehicle->s.origin[1],
				ent->vehicle->s.origin[2] - 128 );

	trace = gi.trace( ent->vehicle->s.origin, NULL, NULL, underneath, ent->vehicle, MASK_ALL );

	if( strcmp( trace.ent->classname, "func_runway" ) == 0 ||
		( trace.contents & CONTENTS_SOLID ) )
	{
		ent->vehicle->ONOFF |= LANDED;
		if( ent->vehicle->HASCAN & HAS_LANDINGGEAR )
			ent->vehicle->ONOFF |= GEARDOWN;
		ent->vehicle->actualspeed = ent->vehicle->currenttop = 0;
		ent->vehicle->thrust = THRUST_0;
		ent->vehicle->s.origin[2] = trace.endpos[2] + abs(ent->vehicle->mins[2]) + 1.5;
//		ent->vehicle->s.origin[2] = trace.ent->s.origin[2] + trace.ent->maxs[2] +
//				fabs( ent->vehicle->mins[2] );
		ent->vehicle->s.angles[2] = 0;
		if( ent->vehicle->HASCAN & IS_TAILDRAGGER )
			ent->vehicle->s.angles[0] = -5;
		else
			ent->vehicle->s.angles[0] = 0;
		VectorClear( ent->vehicle->velocity );
		VectorClear( ent->vehicle->avelocity );
	}
}

void SetVehicleModel (edict_t *vehicle )
{

	VectorClear( vehicle->avelocity );
	if( strcmp( vehicle->classname, "LQM" ) != 0 )
	{
		gi.setmodel( vehicle, "models/objects/debris1/tris.md2" );
		vehicle->s.effects = EF_ROCKET;
	}
	vehicle->gravity = .8;
	vehicle->s.frame = 0;
	vehicle->s.skinnum = 0;
	vehicle->solid = SOLID_NOT;
	vehicle->movetype = MOVETYPE_TOSS;
	vehicle->takedamage = DAMAGE_NO;
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
	else if( strcmp( vehicle->classname, "LQM" ) == 0 )
		vehicle->s.modelindex2 = 0;
}

void CountFrags( edict_t *ent )
{
	int mod = meansOfDeath;

	switch( mod )
	{
		case MOD_SUICIDE:
							ent->client->resp.score -= 3;
							break;
		case MOD_GEAR:
		case MOD_RUNWAY:
		case MOD_CRASH:
		case MOD_PLANESPLASH:
		case MOD_SELFKILL:
							team[ent->client->pers.team-1].score -= 1;
							ent->client->resp.score --;
							break;
		case MOD_MIDAIR:
		case MOD_AUTOCANNON:
		case MOD_SIDEWINDER:
		case MOD_STINGER:
		case MOD_AMRAAM:
		case MOD_PHOENIX:
		case MOD_LGB:
		case MOD_MAVERICK:
		case MOD_MAINGUN:
		case MOD_ATGM:
		case MOD_ROCKET:
		case MOD_BOMBS:
							break;
	}
}
/*
void AQ2SaveVars( gclient_t *client )
{
	FILE	*file;

	file = fopen ("aq2.dat", "wb");
	if (!file)
		gi.error ("Couldn't open aq2.dat");

	fwrite (&client->pers.cameradistance, sizeof(client->pers.cameradistance), 1, file);
	fwrite (&client->pers.cameraheight, sizeof(client->pers.cameraheight), 1, file);
	fwrite (&client->pers.autocoord, sizeof(client->pers.autocoord), 1, file);
	fwrite (&client->pers.autoroll, sizeof(client->pers.autoroll), 1, file);
	fwrite (&client->pers.videomodefix, sizeof(client->pers.videomodefix), 1, file);

	fclose( file );

}*/
/*
void AQ2LoadVars( gclient_t *client )
{
	FILE	*file;


	file = fopen ("aq2.dat", "rb");
	if (!file)
		return;

	fread (&client->pers.cameradistance, sizeof(client->pers.cameradistance), 1, file);
	fread (&client->pers.cameraheight, sizeof(client->pers.cameraheight), 1, file);
	fread (&client->pers.autocoord, sizeof(client->pers.autocoord), 1, file);
	fread (&client->pers.autoroll, sizeof(client->pers.autoroll), 1, file);
	fread (&client->pers.videomodefix, sizeof(client->pers.videomodefix), 1, file );

// +software

	fclose( file );

	if( client->pers.cameradistance < 36 )
		client->pers.cameradistance = 36;
	else if( client->pers.cameradistance > 92 )
		client->pers.cameradistance = 92;
	if( client->pers.cameraheight < 1 )
		client->pers.cameraheight = 1;
	else if( client->pers.cameraheight > 40 )
		client->pers.cameraheight = 40;
	if( client->pers.autocoord > 0 )
		client->pers.autocoord = 1;
	else
		client->pers.autocoord = 0;
	if( client->pers.autoroll > 0 )
		client->pers.autoroll = 1;
	else
		client->pers.autoroll = 0;
	if( client->pers.videomodefix < 1 )
		client->pers.videomodefix = -1;
	else
		client->pers.videomodefix = 1;
}*/

/*
void ShowCrosshair( edict_t *ent )
{
	char	stats[64];

	Com_sprintf( stats, sizeof(stats), "xv %i yv %i picn nolock ",
   			 ent->vehicle->thrust*50, 200 );

	gi.WriteByte(svc_layout);
	gi.WriteString(stats);
	gi.unicast(ent,false);

}
*/


int SelectVehicleStart (edict_t *ent) // former putclientinserver
{
	vec3_t	mins = { -16, -16, -16};
	vec3_t	maxs = { 16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	int    found;
	edict_t	*spot = NULL;
	int 	oldvideomode;
	int 	oldcamheight, oldcamdist, oldcoord, oldroll, oldteam;

//	gi.bprintf( PRINT_HIGH, "PutInServer!\n" );
/*	spot = G_Find (spot, FOFS(classname), "info_player_start");
	VectorCopy (spot->s.origin, ent->s.origin);
	ent->s.origin[2] += 9;
	VectorCopy (spot->s.angles, ent->s.angles);
*/
	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	found = SelectAQ2SpawnPoint (ent, spawn_origin, spawn_angles);

	if( !found )
		return 0;

	index = ent-g_edicts-1;
	client = ent->client;

	oldvideomode = client->pers.videomodefix;
	oldcamdist = client->pers.cameradistance;
	oldcamheight = client->pers.cameraheight;
	oldcoord = client->pers.autocoord;
	oldroll = client->pers.autoroll;
	oldteam = client->pers.team;
//	oldautoaim = client->pers.autoaim;

	if( !oldvideomode )
		oldvideomode = 1;

	// deathmatch wipes most client data every spawn
	if (deathmatch->value)
	{
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		InitClientPersistant (client);
		ClientUserinfoChanged (ent, userinfo);
	}
	else if (coop->value)
	{
		int			n;
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));
		// this is kind of ugly, but it's how we want to handle keys in coop
		for (n = 0; n < MAX_ITEMS; n++)
		{
			if (itemlist[n].flags & IT_KEY)
				resp.coop_respawn.inventory[n] = client->pers.inventory[n];
		}
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
		if (resp.score > client->pers.score)
			client->pers.score = resp.score;
	}
	else
	{
		memset (&resp, 0, sizeof(resp));
	}

	// clear everything but the persistant data
	saved = client->pers;
	memset (client, 0, sizeof(*client));
	client->pers = saved;
	if (client->pers.health <= 0)
		InitClientPersistant(client);
	client->resp = resp;

	// copy some data from the client to the entity
	FetchClientEntData (ent);

	// clear entity values
	ent->groundentity = NULL;
	ent->client = &game.clients[index];
	ent->takedamage = DAMAGE_NO;				// +BD changed from AIM
	ent->movetype = MOVETYPE_NONE;			// +BD changed from WALK
	ent->viewheight = 0;						// +BD changed from 22
	ent->inuse = true;
	ent->classname = "camera";
	ent->mass = 0;								// +BD changed from 200
	ent->solid = SOLID_NOT;						// +BD changed from BBOX
	ent->deadflag = DEAD_NO;
	ent->air_finished = level.time + 12;
	ent->clipmask = 0;							// +BD changed from MASK_PLAYERSOLID;
	ent->model = NULL;							// +BD changed from: "players/male/tris.md2";
	ent->pain = NULL;							// +BD changed from: player_pain;
	ent->die = NULL;							// +BD changed from: player_die;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;

	if( teamplay->value == 1 )
	{
		if( ent->client->pers.team > 0 )
			ent->sel_step = 1;
	}

/*	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);*/ 		// +BD removed
	VectorClear( mins );
	VectorClear( maxs );

	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

//	gi.cvar_forceset( "dmflags", va( "%d", DF_FIXED_FOV ) );// +BD
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		client->ps.fov = 90;
	}
	else
	{
		client->ps.fov = atoi(Info_ValueForKey(client->pers.userinfo, "fov"));
		if (client->ps.fov < 1)
			client->ps.fov = 90;
		else if (client->ps.fov > 160)
			client->ps.fov = 160;
	}

	client->ps.gunindex = 0; // +BD changed from: gi.modelindex(client->pers.weapon->view_model);

	// clear entity state values
	ent->s.effects = 0;
	ent->s.skinnum = ent - g_edicts - 1;
	ent->s.modelindex = 0;		// will use the skin specified model // +BD removed 255
	ent->s.modelindex2 = 0;		// custom gun model				// +BD detto
	ent->s.frame = 0;
	ent->gravity = 0;			// +BD
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	client->pers.videomodefix = oldvideomode;
	client->pers.cameradistance = oldcamdist;
	client->pers.cameraheight = oldcamheight;
	client->pers.autocoord = oldcoord;
	client->pers.autoroll = oldroll;
	client->pers.team = oldteam;
//	client->pers.autoaim = oldautoaim;

//	gi.linkentity (ent);			// +BD removed

	// force the current weapon up
	client->newweapon = client->pers.weapon = NULL;
	ChangeWeapon (ent);

//	gi.cprintf( ent, PRINT_MEDIUM, "Select your vehicle, %s!\n", ent->client->pers.netname );	// +BD

	return 1;
}

extern edict_t *SelectDeathmatchSpawnPoint (void);
extern edict_t *SelectCoopSpawnPoint (edict_t *ent);

int SelectAQ2SpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

	if (deathmatch->value)
	{
/*		while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
		{
			if( ent->veh_category & spot->sounds )
				break;
		}
		if( !spot )
			return 0;
		spot = NULL;
		while( 1 )
	   	{*/
		if( teamplay->value == 1 && level.category & 96 )
			spot = SelectAQ2DMSpawnPoint (ent->veh_category, ent->client->pers.team);
		else
			spot = SelectAQ2DMSpawnPoint (ent->veh_category, 0);
/*			if( ent->veh_category & spot->sounds )
				break;
		}*/
//			gi.bprintf( PRINT_HIGH, "%d\n", spot->sounds );
	}
/*	else if (coop->value)
		spot = SelectCoopSpawnPoint (ent);
*/
	// find a single player start spot
	if (!spot)
	{
		while ((spot = G_Find (spot, FOFS(classname), "info_player_start")) != NULL)
		{
			if (!game.spawnpoint[0] && !spot->targetname)
				break;

			if (!game.spawnpoint[0] || !spot->targetname)
				continue;

			if (Q_stricmp(game.spawnpoint, spot->targetname) == 0)
				break;
		}

		if (!spot)
		{
			if (!game.spawnpoint[0])
			{	// there wasn't a spawnpoint without a target, so use any
				spot = G_Find (spot, FOFS(classname), "info_player_start");
			}
			if (!spot)
				gi.error ("Couldn't find spawn point %s\n", game.spawnpoint);
		}
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return 1;
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

	while ((ent = findradius(ent, vehicle->s.origin, 150)) != NULL)
	{
		if (ent == vehicle)
			continue;
		if (!ent->takedamage)
			continue;

		VectorAdd (ent->mins, ent->maxs, v);
		VectorMA (ent->s.origin, 0.5, v, v);
		VectorSubtract (vehicle->s.origin, v, v);
		points = 150 - 0.5 * VectorLength (v);
//		if (ent == attacker)
//			points = points * 0.5;
		if (points > 0)
		{
			if (CanDamage (ent, vehicle))
			{
				VectorSubtract (ent->s.origin, vehicle->s.origin, dir);
				T_Damage (ent, vehicle, vehicle, dir, vehicle->s.origin,
			   		 vec3_origin, (int)points, (int)points, 150, MOD_EXPLOSIVE);
			}
		}
	}
}

/*
================
VehicleRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	VehicleRangeFromSpot (edict_t *spot)
{
	edict_t	*player;
	float	bestplayerdistance;
	vec3_t	v;
	int		n;
	float	playerdistance;


	bestplayerdistance = 9999999;

	for (n = 1; n <= maxclients->value; n++)
	{
		player = &g_edicts[n];

		if (!player->inuse)
			continue;

		if (player->health <= 0)
			continue;

		if( !player->client->veh_selected )
			continue;

		VectorSubtract (spot->s.origin, player->vehicle->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}


/*
================
SelectAQ2DMSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectAQ2DMSpawnPoint (int cat, int tmnr )
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	if( tmnr > 2 )
		tmnr -= 2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		if( !(spot->sounds & cat) )
			continue;
		if( teamplay->value == 1 && spot->aqteam != tmnr )
			continue;
		count++;
		range = VehicleRangeFromSpot(spot);
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
		do
		{
			spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		}while( !(spot->sounds & cat) || (spot->aqteam != tmnr) );
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
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

	location = (int)(random()*20+1);
	switch( location )
	{
		case 1:
				targ->DAMAGE |= GEAR_DAMAGE;
				if( targ->HASCAN & HAS_LANDINGGEAR )
				{
					if( targ->ONOFF & PILOT_ONBOARD )
						gi.cprintf( targ->owner, PRINT_HIGH, "Gear damaged!\n" );
				}
				break;
		case 2:
				targ->DAMAGE |= SPEEDBRAKE_DAMAGE;
					if( targ->HASCAN & HAS_SPEEDBRAKES )
				{
					if( strcmp( targ->classname,"plane" ) == 0 )
					{
						if( targ->ONOFF & PILOT_ONBOARD )
							gi.cprintf( targ->owner, PRINT_HIGH, "Speedbrakes damaged!\n" );
					}
					else if( strcmp( targ->classname,"ground" ) == 0 )
					{
						if( targ->ONOFF & PILOT_ONBOARD )
							gi.cprintf( targ->owner, PRINT_HIGH, "Handbrake damaged!\n" );
						}
 				}
				break;
		case 3:
				targ->DAMAGE |= AFTERBURNER_DAMAGE;
				if( targ->thrust > THRUST_100 )
					targ->thrust = THRUST_100;
				if( targ->HASCAN & HAS_AFTERBURNER )
				{
					if( targ->ONOFF & PILOT_ONBOARD )
						gi.cprintf( targ->owner, PRINT_HIGH, "Afterburner damaged!\n" );
				}
				else if( strcmp( targ->classname,"ground" ) == 0 )
				{
					if( targ->ONOFF & PILOT_ONBOARD )
						gi.cprintf( targ->owner, PRINT_HIGH, "Reverse damaged!\n" );
				}
				break;
		case 4:
				targ->DAMAGE |= ENGINE75 | AFTERBURNER_DAMAGE;
				if( targ->thrust > THRUST_75 )
					targ->thrust = THRUST_75;
				if( targ->ONOFF & PILOT_ONBOARD )
					gi.cprintf( targ->owner, PRINT_HIGH, "Engine damaged!\n" );
				break;
		case 5:
				targ->DAMAGE |= ENGINE50 | ENGINE75 | AFTERBURNER_DAMAGE;
				if ( targ->thrust > THRUST_50 )
					targ->thrust = THRUST_50;
				if( targ->ONOFF & PILOT_ONBOARD )
					gi.cprintf( targ->owner, PRINT_HIGH, "Engine damaged!\n" );
				break;
		case 6:
				targ->DAMAGE |= ENGINE25 | ENGINE50 | ENGINE75 | AFTERBURNER_DAMAGE;
				if( targ->thrust > THRUST_25 )
						targ->thrust = THRUST_25;
				if( targ->ONOFF & PILOT_ONBOARD )
					gi.cprintf( targ->owner, PRINT_HIGH, "Engine damaged!\n" );
				break;
		case 7:
				targ->DAMAGE |= ENGINE0 | ENGINE25 | ENGINE50 |ENGINE75 | AFTERBURNER_DAMAGE;
				if( targ->thrust > THRUST_0 )
					targ->thrust = THRUST_0;
				if( targ->ONOFF & PILOT_ONBOARD )
						gi.cprintf( targ->owner, PRINT_HIGH, "Engine destroyed!\n" );
				if( strcmp( targ->classname, "ground" ) == 0 )
				{
					targ->DAMAGE |= AFTERBURNER_DAMAGE;
				}
					break;
		case 8:
				targ->DAMAGE |= GPS_DAMAGE;
				if( targ->ONOFF & GPS )
					targ->ONOFF ^= GPS;
				if( targ->ONOFF & PILOT_ONBOARD )
					gi.cprintf( targ->owner, PRINT_HIGH, "GPS damaged!\n" );
					break;
		case 9:
				targ->DAMAGE |= AUTOPILOT_DAMAGE;
					if( targ->ONOFF & AUTOPILOT )
					targ->ONOFF ^= AUTOPILOT;
				if( strcmp( targ->classname, "ground" ) != 0 )
				{
					if( targ->ONOFF & PILOT_ONBOARD )
						gi.cprintf( targ->owner, PRINT_HIGH, "Autopilot damaged!\n" );
					}
				break;
		case 10:
				if( targ->HASCAN & HAS_WEAPONBAY )
				{
					targ->DAMAGE |= WEAPONBAY_DAMAGE;
					if( targ->ONOFF & PILOT_ONBOARD )
						gi.cprintf( targ->owner, PRINT_HIGH, "Baydoors damaged!\n" );
				}
		default:
				break;
	}
	// helos dont have afterburners
	if( strcmp( targ->classname, "helo" ) == 0 && targ->DAMAGE & AFTERBURNER_DAMAGE )
		targ->DAMAGE &= ~AFTERBURNER_DAMAGE;
}
