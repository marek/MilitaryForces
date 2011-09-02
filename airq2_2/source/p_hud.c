#include "g_local.h"



/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	if (deathmatch->value || coop->value)
		ent->client->showscores = true;
	VectorCopy (level.intermission_origin, ent->s.origin);
	ent->client->ps.pmove.origin[0] = level.intermission_origin[0]*8;
	ent->client->ps.pmove.origin[1] = level.intermission_origin[1]*8;
	ent->client->ps.pmove.origin[2] = level.intermission_origin[2]*8;
	VectorCopy (level.intermission_angle, ent->client->ps.viewangles);
	ent->client->ps.pmove.pm_type = PM_FREEZE;
	ent->client->ps.gunindex = 0;
	ent->client->ps.blend[3] = 0;
	ent->client->ps.rdflags &= ~RDF_UNDERWATER;

//	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex = 0;
	ent->s.effects = 0;
	ent->s.sound = 0;
	ent->solid = SOLID_NOT;

	// add the layout

	if (deathmatch->value || coop->value)
	{
		DeathmatchScoreboardMessage (ent, NULL);
		gi.unicast (ent, true);
	}

}

void BeginIntermission (edict_t *targ)
{
	int		i;
	edict_t	*ent, *client;

	if (level.intermissiontime)
		return;		// already activated

	game.autosaved = false;

	// respawn any dead clients
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
/*		if (client->health <= 0)
		{
			respawn(client);
		}*/
		client->client->selectvehicle = client->client->showinventory = false;
		RemoveVehicle( client );
		RemoveCamera( client );
	}

	level.intermissiontime = level.time;
	level.changemap = targ->map;

	if (strstr(level.changemap, "*"))
	{
		if (coop->value)
		{
			for (i=0 ; i<maxclients->value ; i++)
			{
				client = g_edicts + 1 + i;
				if (!client->inuse)
					continue;
			}
		}
	}
	else
	{
		if (!deathmatch->value)
		{
			level.exitintermission = 1;		// go immediately to the next level
			return;
		}
	}

	level.exitintermission = 0;

	// find an intermission spot
	ent = G_Find (NULL, FOFS(classname), "info_player_intermission");
	if (!ent)
	{	// the map creator forgot to put in an intermission point...
		ent = G_Find( NULL, FOFS(classname), "misc_camera");
		if(!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_start");
		if (!ent)
			ent = G_Find (NULL, FOFS(classname), "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while (i--)
		{
			ent = G_Find (ent, FOFS(classname), "info_player_intermission");
			if (!ent)	// wrap around the list
				ent = G_Find (ent, FOFS(classname), "info_player_intermission");
		}
	}

	VectorCopy (ent->s.origin, level.intermission_origin);
	VectorCopy (ent->s.angles, level.intermission_angle);

	// move all clients to the intermission point
	for (i=0 ; i<maxclients->value ; i++)
	{
		client = g_edicts + 1 + i;
		if (!client->inuse)
			continue;
		MoveClientToIntermission (client);
	}

}


/*
==================
DeathmatchScoreboardMessage

==================
*/
void DeathmatchScoreboardMessage (edict_t *ent, edict_t *killer)
{
	char	entry[1024];
	char	string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];	// array = MAX_CLIENTS + MAX_BOTS
	int		sortedscores[MAX_CLIENTS]; // same here
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

	// sort the clients by score
	total = 0;
	for (i=0 ; i<game.maxclients ; i++)
	{
 		cl_ent = g_edicts + 1 + i;
		if (!cl_ent->inuse)
			continue;
		score = game.clients[i].resp.score;
		for (j=0 ; j<total ; j++)
		{
			if (score > sortedscores[j])
				break;
		}
		for (k=total ; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}

	// print level name and exit rules
	string[0] = 0;

	stringlength = strlen(string);

	// add the clients in sorted order
	if (total > 12)
		total = 12;

	for (i=0 ; i<total ; i++)
	{
		cl = &game.clients[sorted[i]];
		cl_ent = g_edicts + 1 + sorted[i];

		picnum = gi.imageindex ("i_fixme");
		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// add a dogtag
		if (cl_ent == ent)
			tag = "tag1";
		else if (cl_ent == killer)
			tag = "tag2";
		else
			tag = NULL;
		if (tag)
		{
			Com_sprintf (entry, sizeof(entry),
				"xv %i yv %i picn %s ",x+32, y, tag);
			j = strlen(entry);
			if (stringlength + j >= 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j >= 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void DeathmatchScoreboard (edict_t *ent)
{
	DeathmatchScoreboardMessage (ent, ent->enemy);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Score_f

Display the scoreboard
==================
*/
void Cmd_Score_f (edict_t *ent)
{
	ent->client->showinventory = false;
	ent->client->showhelp = false;
	ent->client->selectvehicle = false;

	if (!deathmatch->value)
		return;

	if (ent->client->showscores)
	{
		ent->client->showscores = false;
		return;
	}

	ent->client->showscores = true;
	DeathmatchScoreboard (ent);
}


/*
==================
HelpComputer

Draw help computer.
==================
*/
void HelpComputer (edict_t *ent)
{
	char	string[1024];
	char	*sk;

	if (skill->value == 0)
		sk = "easy";
	else if (skill->value == 1)
		sk = "medium";
	else if (skill->value == 2)
		sk = "hard";
	else
		sk = "hard+";

	// send the layout
	Com_sprintf (string, sizeof(string),
		"xv 32 yv 8 picn help "			// background
		"xv 202 yv 12 string2 \"%s\" "		// skill
		"xv 0 yv 24 cstring2 \"%s\" "		// level name
		"xv 0 yv 54 cstring2 \"%s\" "		// help 1
		"xv 0 yv 110 cstring2 \"%s\" "		// help 2
		"xv 50 yv 164 string2 \" kills     goals    secrets\" "
		"xv 50 yv 172 string2 \"%3i/%3i     %i/%i       %i/%i\" ", 
		sk,
		level.level_name,
		game.helpmessage1,
		game.helpmessage2,
		level.killed_monsters, level.total_monsters, 
		level.found_goals, level.total_goals,
		level.found_secrets, level.total_secrets);

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
	gi.unicast (ent, true);
}


/*
==================
Cmd_Help_f

Display the current help message
==================
*/
void Cmd_Help_f (edict_t *ent)
{
	// this is for backwards compatability
	if (deathmatch->value)
	{
		Cmd_Score_f (ent);
		return;
	}

	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->pers.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->pers.helpchanged = 0;
	HelpComputer (ent);
}


//=======================================================================

/*
===============
G_SetStats
===============
*/
void G_SetStats (edict_t *ent)
{
	//
	// layouts
	//
	ent->client->ps.stats[STAT_LAYOUTS] = 0;

	if (deathmatch->value)
	{
		if (ent->client->pers.health <= 0 || level.intermissiontime
			|| ent->client->showscores || ent->client->selectvehicle
			|| ent->client->showinventory )
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp || ent->client->selectvehicle
		|| ent->client->showinventory )
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
	}
	//
	// frags
	//
	if( deathmatch->value )
		ent->client->ps.stats[STAT_SINGLE_PLAYER] = 1;
	else
		ent->client->ps.stats[STAT_SINGLE_PLAYER] = 0;
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// help icon 
	//
	if (ent->client->pers.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;

	// 
	// New AirQuake2 stuff
	//
	if( ent->client->FLAGS & CLIENT_VEH_SELECTED )
	{
		// health + lock
		if(ent->vehicle->lockstatus == LS_NOLOCK )
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex( "Health_1" );
		else if( ent->vehicle->lockstatus == LS_LOCK )
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex( "Health_2" );
		else
			ent->client->ps.stats[STAT_HEALTH_ICON] = gi.imageindex( "Health_3" );
		ent->client->ps.stats[STAT_HEALTH] = ent->health;
		// autopilot, gear, brake, weaponbay
		if( strcmp( ent->vehicle->classname, "LQM" ) != 0 )
		{
			ent->client->ps.stats[STAT_SWITCHES] = gi.imageindex( "Switch" );
			if( ent->vehicle->DAMAGE & DAMAGE_WEAPONBAY )
				ent->client->ps.stats[STAT_WEAPONBAY] = gi.imageindex( "bay_dam" );
			else
			{
				if( ent->vehicle->ONOFF & ONOFF_WEAPONBAY )
					ent->client->ps.stats[STAT_WEAPONBAY] = gi.imageindex( "Bay" );
				else
					ent->client->ps.stats[STAT_WEAPONBAY] = 0;
			}
			if( ent->vehicle->DAMAGE & DAMAGE_GEAR )
				ent->client->ps.stats[STAT_GEAR] = gi.imageindex( "gear_dam" );
			else
			{
				if( ent->vehicle->ONOFF & ONOFF_GEAR )
					ent->client->ps.stats[STAT_GEAR] = gi.imageindex( "Gear" );
				else
					ent->client->ps.stats[STAT_GEAR] = 0;
			}
			if( ent->vehicle->DAMAGE & DAMAGE_BRAKE )
				ent->client->ps.stats[STAT_BRAKE] = gi.imageindex( "brake_dam" );
			else
			{
				if( ent->vehicle->ONOFF & ONOFF_BRAKE )
					ent->client->ps.stats[STAT_BRAKE] = gi.imageindex( "Brake" );
				else
					ent->client->ps.stats[STAT_BRAKE] = 0;
			}
			if( ent->vehicle->DAMAGE & DAMAGE_AUTOPILOT )
				ent->client->ps.stats[STAT_AUTOPILOT] = gi.imageindex( "auto_dam" );
			else
			{
				if( ent->vehicle->ONOFF & ONOFF_AUTOPILOT )
					ent->client->ps.stats[STAT_AUTOPILOT] = gi.imageindex( "Auto" );
				else
					ent->client->ps.stats[STAT_AUTOPILOT] = 0;
			}
		}
		else
		{
			ent->client->ps.stats[STAT_SWITCHES] = 0;
			ent->client->ps.stats[STAT_WEAPONBAY] = 0;
			ent->client->ps.stats[STAT_GEAR] = 0;
			ent->client->ps.stats[STAT_BRAKE] = 0;
			ent->client->ps.stats[STAT_AUTOPILOT] = 0;
		}
		ent->client->ps.stats[STAT_FLARES_ICON] = gi.imageindex( "Flare" );
		ent->client->ps.stats[STAT_FLARES] = ent->vehicle->flares;
		ent->client->ps.stats[STAT_AUTOCANNON_ICON] = gi.imageindex( "Cannon" );
		if( ent->vehicle->ammo[0] >= 0 )
		{

			ent->client->ps.stats[STAT_AUTOCANNON] = ent->vehicle->ammo[0];
		}
		else
		{
			ent->client->ps.stats[STAT_AUTOCANNON] = 0;
		}
		if( strcmp( ent->vehicle->classname, "plane" ) == 0 ||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
		{
			if( ent->vehicle->thrust < 1 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_1" );
			else if( ent->vehicle->thrust < 4 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_2" );
			else if( ent->vehicle->thrust < 7 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_3" );
			else if( ent->vehicle->thrust < 9 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_4" );
			else if( ent->vehicle->thrust <= THRUST_MILITARY )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_5" );
			else
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_6" );
		}
		else 
		{
			if( ent->vehicle->thrust < 0 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_6" );
			else if( ent->vehicle->thrust == 0 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_1" );
			else if( ent->vehicle->thrust < 4 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_2" );
			else if( ent->vehicle->thrust < 7 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_3" );
			else if( ent->vehicle->thrust < 9 )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_4" );
			else if( ent->vehicle->thrust <= THRUST_MILITARY )
				ent->client->ps.stats[STAT_THROTTLE_ICON] = gi.imageindex( "Throt_5" );
		}
		ent->client->ps.stats[STAT_THROTTLE] = ent->vehicle->speed;
		if( ent->vehicle->ONOFF & ONOFF_GPS )
		{
			ent->client->ps.stats[STAT_GPS] = gi.imageindex( "Gps" );
			ent->client->ps.stats[STAT_HEADING] = ent->vehicle->s.angles[1];
			ent->client->ps.stats[STAT_LONGITUDE] = ent->vehicle->s.origin[0];
			ent->client->ps.stats[STAT_LATITUDE] = ent->vehicle->s.origin[1];
			ent->client->ps.stats[STAT_ALTITUDE] = ent->vehicle->altitude;
		}
		else
		{
			ent->client->ps.stats[STAT_GPS] = 0;
			ent->client->ps.stats[STAT_HEADING] = 0;
			ent->client->ps.stats[STAT_LONGITUDE] = 0;
			ent->client->ps.stats[STAT_LATITUDE] = 0;
			ent->client->ps.stats[STAT_ALTITUDE] = 0;
		}
		if( ent->vehicle->ammo[ent->vehicle->active_weapon] >= 0 )
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = ent->vehicle->weaponicon[ent->vehicle->active_weapon];
			ent->client->ps.stats[STAT_AMMO] = ent->vehicle->ammo[ent->vehicle->active_weapon];
		}
		else
		{
			ent->client->ps.stats[STAT_AMMO_ICON] = ent->vehicle->weaponicon_dam[ent->vehicle->active_weapon];
			ent->client->ps.stats[STAT_AMMO] = 0;
		}
		if( ent->client->showinventory )
		{
			ent->client->ps.stats[STAT_MFD] = gi.imageindex( "mfd" );
			VehicleInventory( ent );
		}
		else
			ent->client->ps.stats[STAT_MFD] = 0;
		if( ent->client->cameratarget != ent->vehicle && !ent->client->showscores && !ent->client->showinventory)
		{
			ent->client->ps.stats[STAT_HAS_CAMTARGET] = 1;
		}
		else
		{
			ent->client->ps.stats[STAT_HAS_CAMTARGET] = 0;
		}
		if( !ent->client->selectvehicle && !ent->client->showscores
				&& !ent->client->showinventory && !ent->vehicle->deadflag )
		{
			if( ent->vehicle->enemy )
			{
				if( teamplay->value != 1 )
					ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex( "lock" );
				else
				{
					if( ent->vehicle->enemy->aqteam != ent->vehicle->aqteam )
						ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex( "lock" );
					else
						ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex( "frlock" );
				}
			}
			else
			{
				ent->client->ps.stats[STAT_CROSSHAIR] = gi.imageindex( "nolock" );
			}
		}
		else
			ent->client->ps.stats[STAT_CROSSHAIR] = 0;
	}
	else if( !ent->client->selectvehicle )
	{
		ent->client->ps.stats[STAT_HEALTH_ICON] = 0;
		ent->client->ps.stats[STAT_HEALTH] = 0;
		ent->client->ps.stats[STAT_SWITCHES] = 0;
		ent->client->ps.stats[STAT_WEAPONBAY] = 0;
		ent->client->ps.stats[STAT_GEAR] = 0;
		ent->client->ps.stats[STAT_BRAKE] = 0;
		ent->client->ps.stats[STAT_AUTOPILOT] = 0;
		ent->client->ps.stats[STAT_FLARES_ICON] = 0;
		ent->client->ps.stats[STAT_FLARES] = 0;
		ent->client->ps.stats[STAT_AUTOCANNON_ICON] = 0;
		ent->client->ps.stats[STAT_AUTOCANNON] = 0;
		ent->client->ps.stats[STAT_THROTTLE_ICON] = 0;
		ent->client->ps.stats[STAT_THROTTLE] = 0;
		ent->client->ps.stats[STAT_GPS] = 0;
		ent->client->ps.stats[STAT_HEADING] = 0;
		ent->client->ps.stats[STAT_LONGITUDE] = 0;
		ent->client->ps.stats[STAT_LATITUDE] = 0;
		ent->client->ps.stats[STAT_ALTITUDE] = 0;
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
		ent->client->ps.stats[STAT_MFD] = 0;
		ent->client->ps.stats[STAT_CROSSHAIR] = 0;
	}
	else if( ent->client->selectvehicle && !ent->client->showscores
		&& !ent->client->showinventory )
	{
		ent->client->ps.stats[STAT_MFD] = gi.imageindex ( "mfd" );
		ent->client->ps.stats[STAT_CROSSHAIR] = 0;
		VehicleSelectionDisplay( ent );
	}
}



