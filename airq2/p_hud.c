#include "g_local.h"



/*
======================================================================

INTERMISSION

======================================================================
*/

void MoveClientToIntermission (edict_t *ent)
{
	ent->client->selectvehicle = false;
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

	// clean up powerup info
	ent->client->quad_framenum = 0;
	ent->client->invincible_framenum = 0;
	ent->client->breather_framenum = 0;
	ent->client->enviro_framenum = 0;
//	ent->client->grenade_blew_up = false;
//	ent->client->grenade_time = 0;

	ent->viewheight = 0;
	ent->s.modelindex = 0;
	ent->s.modelindex2 = 0;
	ent->s.modelindex3 = 0;
	ent->s.modelindex4 = 0;
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
	int		i, n;
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
		if (client->health <= 0)
			respawn(client);
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
				// strip players of all keys between units
				for (n = 0; n < MAX_ITEMS; n++)
				{
					if (itemlist[n].flags & IT_KEY)
						client->client->pers.inventory[n] = 0;
				}
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
TeamplayScoreboardMessage

==================
*/
void TeamplayScoreboardMessage (edict_t *ent)
{
	char	shortstring1[40],shortstring2[30],shortstring3[30],shortstring4[30];
	char	positionstring[4], positionstring2[4], positionstring3[4], positionstring4[4],scorestring[6];
	char	entry[1024];
	char 	string[1400];
	edict_t	*cl_ent;
	int		i, j;
	int 	linenumber[2];
	int		stringlength;

	linenumber[0] = linenumber[1] = 70;

	sprintf( shortstring1, "%d Players", team[TEAM1].players );
	sprintf( shortstring2, "%d Drones", team[TEAM1].drones );
	sprintf( shortstring3, "%d Players", team[TEAM2].players );
	sprintf( shortstring4, "%d Drones", team[TEAM2].drones );

	Com_sprintf (string, sizeof(string),
				"xv 0 yv 20 string2 \"%s\" "
				"xv 0 yv 27 string2 \"%s\" "
				"xv 160 yv 20 string2 \"%s\" "
				"xv 160 yv 27 string2 \"%s\" "
				"xv 0 yv 40 string \"%s\" "
				"xv 0 yv 50 string \"%s\" "
				"xv 160 yv 40 string \"%s\" "
				"xv 160 yv 50 string \"%s\" ",

				team[0].name,
				"--------------------",
				team[1].name,
				"--------------------",
				shortstring1,
				shortstring2,
				shortstring3,
				shortstring4
				);

	stringlength = strlen(string);

	for (i=0 ; i<game.maxclients ; i++)
	{
		cl_ent = g_edicts + 1 + i;
		if( !cl_ent->inuse )
			continue;
		if( linenumber[cl_ent->client->pers.team-1] > 220 )
			continue;
		// score, name, vehicle
		sprintf( shortstring1, "%s (%s) ", cl_ent->client->pers.netname,
			( (cl_ent->client->veh_selected&&!(cl_ent->deadflag)) ? cl_ent->vehicle->friendlyname : "DEAD" ) );
		sprintf( scorestring, "%d", cl_ent->client->resp.score );
		// where to place it
		sprintf( positionstring, "%d", linenumber[cl_ent->client->pers.team-1] );
		sprintf( positionstring2, "%d", ( cl_ent->client->pers.team == 1 ? 0 : 160 ) );
		sprintf( positionstring3, "%d", ( cl_ent->client->pers.team == 1 ? 25 : 185 ) );

		linenumber[cl_ent->client->pers.team-1] += 10;

		Com_sprintf (entry, sizeof(entry),
				"xv \"%s\" yv \"%s\" string \"%s\" "
				"xv \"%s\" yv \"%s\" string \"%s\" ",

				positionstring2,
				positionstring,
				scorestring,
				positionstring3,
				positionstring,
				shortstring1
				);

		j = strlen(entry);
		if (stringlength + j > 1024)
			break;
		strcpy (string + stringlength, entry);
		stringlength += j;
	}
	sprintf( positionstring, "%d", linenumber[0] );
	sprintf( positionstring2, "%d", linenumber[1] );
	sprintf( positionstring3, "%d", linenumber[0]+10 );
	sprintf( positionstring4, "%d", linenumber[1]+10 );
	strcpy( shortstring1, "------------------" );
	strcpy( shortstring2, shortstring1 );
	sprintf( shortstring3, "%d Totalscore", team[0].score );
	sprintf( shortstring4, "%d Totalscore", team[1].score );
	Com_sprintf (entry, sizeof(entry),
				"xv 0 yv \"%s\" string2 \"%s\" "
				"xv 160 yv \"%s\" string2 \"%s\" "
				"xv 0 yv \"%s\" string2 \"%s\" "
				"xv 160 yv \"%s\" string2 \"%s\" ",

				positionstring,
				shortstring1,
				positionstring2,
				shortstring2,
				positionstring3,
				shortstring3,
				positionstring4,
				shortstring4
				);
	j = strlen(entry);
	if (stringlength + j < 1024)
	{
		strcpy (string + stringlength, entry);
		stringlength += j;
	}

	gi.WriteByte (svc_layout);
	gi.WriteString (string);
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
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;
	int		picnum;
	int		x, y;
	gclient_t	*cl;
	edict_t		*cl_ent;
	char	*tag;

	if( teamplay->value == 1 )
	{
		TeamplayScoreboardMessage( ent );
		return;
	}

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
			if (stringlength + j > 1024)
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
		}

		// send the layout
		Com_sprintf (entry, sizeof(entry),
			"client %i %i %i %i %i %i ",
			x, y, sorted[i], cl->resp.score, cl->ping, (level.framenum - cl->resp.enterframe)/600);
		j = strlen(entry);
		if (stringlength + j > 1024)
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

	if (!deathmatch->value && !coop->value)
		return;

	if( !ent->client->veh_selected )
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

	if( !ent->client->veh_selected )
		return;
	ent->client->showinventory = false;
	ent->client->showscores = false;

	if (ent->client->showhelp && (ent->client->resp.game_helpchanged == game.helpchanged))
	{
		ent->client->showhelp = false;
		return;
	}

	ent->client->showhelp = true;
	ent->client->resp.helpchanged = 0;
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
	gitem_t		*item;
//	int			index, cells;
//	int			power_armor_type;
	char		string[1024];
	char		shortstring1[20],shortstring2[24],shortstring3[20],shortstring4[20],shortstring5[20];
	char		shortstring6[20],shortstring7[20], shortstring8[20], shortstring9[20], shortstring10[20];
	char		shortstring11[20];
	char		colorstring1[9],colorstring2[9],colorstring3[9],colorstring4[9],colorstring5[9];
	char		colorstring6[9],colorstring7[9],colorstring8[9],colorstring9[9],colorstring10[9];
	char		colorstring11[9];
	char		tempstring[30];
	int			linenumber = 70;

	//
	// health
	//
	ent->client->ps.stats[STAT_HEALTH_ICON] = level.pic_health;
	ent->client->ps.stats[STAT_HEALTH] = ent->health;

	//
	// ammo
	//
	if ( !ent->client->pers.secondary || !ent->client->veh_selected )
	{
		ent->client->ps.stats[STAT_AMMO_ICON] = 0;
		ent->client->ps.stats[STAT_AMMO] = 0;
	}
	else
	{
		item = &itemlist[ent->client->ammo_index2];
		ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex (item->icon);
		ent->client->ps.stats[STAT_AMMO] = ent->client->pers.inventory[ent->client->ammo_index2];
	}

	//
	// armor
	//

	// no armor in AQ2 !

	//
	// pickup message // disabled +BD (v0.62)
	//
/*	if (level.time > ent->client->pickup_msg_time)
	{
		ent->client->ps.stats[STAT_PICKUP_ICON] = 0;
		ent->client->ps.stats[STAT_PICKUP_STRING] = 0;
	}
*/
	//
	// timers
	//

	// no timers in AQ2 (for now)

	//
	// selected item
	//

	// removed in AQ2 (for now)

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
/*		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
*/	}
	else
	{
		if (ent->client->showscores || ent->client->showhelp || ent->client->selectvehicle
		|| ent->client->showinventory )
			ent->client->ps.stats[STAT_LAYOUTS] |= 1;
/*		if (ent->client->showinventory && ent->client->pers.health > 0)
			ent->client->ps.stats[STAT_LAYOUTS] |= 2;
*/	}

	//
	// frags
	//
	ent->client->ps.stats[STAT_FRAGS] = ent->client->resp.score;

	//
	// +BD show current selection (vehicle)
	//
	if( ent->client->veh_selected )
	{
		ent->client->selectvehicle = false;
		ent->client->ps.stats[STAT_SELBACKGROUND] = 0;
	}
	else if( !ent->client->veh_selected && !ent->client->selectvehicle )
	{
		ent->client->ps.stats[STAT_SELBACKGROUND] = 0;
	}
	else if( !ent->client->veh_selected && ent->client->selectvehicle &&
			!ent->client->showscores && !ent->client->showinventory &&
			!level.intermissiontime )
	{
		ent->client->ps.stats[STAT_SELBACKGROUND] = gi.imageindex ( "mfd" );

		if( teamplay->value != 1 && ent->sel_step == 0 )
			ent->sel_step = 1;
		else if( teamplay->value == 1 && ent->client->pers.team > 0
			&& ent->client->pers.team < 3 && ent->sel_step == 0 )
			ent->sel_step = 1;

		if( ent->sel_step == 0 )
		{
			if( !ent->client->pers.team )
				ent->client->pers.team = 3;
			strcpy( shortstring1, teamname1->string );
			strcpy( shortstring2, teamname2->string );
			sprintf( shortstring3, "  (%d players)", team[TEAM1].players );
			sprintf( shortstring4, "  (%d players)", team[TEAM2].players );

			if( ent->client->pers.team == 3 )
			{
				strcpy( colorstring1, "string" );
				strcpy( colorstring2, "string2" );
			}
			else if( ent->client->pers.team == 4 )
			{
				strcpy( colorstring1, "string2" );
				strcpy( colorstring2, "string" );
			}

			Com_sprintf (string, sizeof(string),
				"xv 60 yv 40 string2 \"%s\" "
				"xv 60 yv 47 string2 \"%s\" "
				"xv 60 yv 70 \"%s\" \"%s\" "
				"xv 60 yv 80 string2 \"%s\" "
				"xv 60 yv 100 \"%s\" \"%s\" "
				"xv 60 yv 110 string2 \"%s\" "
				"xv 60 yv 145 string2 \"%s\" "
				"xv 60 yv 155 string2 \"%s\" "
				"xv 60 yv 165 string2 \"%s\" ",

				"Select Team:",
				"------------",
				colorstring1,
				shortstring1,
				shortstring3,
				colorstring2,
				shortstring2,
				shortstring4,
				"INCREASE and DECREASE",
				"to cycle, FIRE to",
			    "select, ESC to go back"
				);

			gi.WriteByte (svc_layout);
			gi.WriteString (string);
			gi.unicast(ent,false);
		}
		else if( ent->sel_step == 1 )
		{
			if( level.category & 1 )
			{
				strcpy( shortstring1, "Airplane" );
				if( !ent->veh_category )
					ent->veh_category = 1;
			}
			else
				strcpy( shortstring1, "(Airplane)" );
			if( level.category & 2 )
			{
				strcpy( shortstring2, "Helicopter" );
				if( !ent->veh_category )
					ent->veh_category = 2;
			}
			else
				strcpy( shortstring2, "(Helicopter)" );
			if( level.category & 4 )
			{
				strcpy( shortstring3, "Ground Vehicle" );
				if( !ent->veh_category )
					ent->veh_category = 4;
			}
			else
				strcpy( shortstring3, "(Ground Vehicle)" );
			if( level.category & 8 )
			{
				strcpy( shortstring4, "Infantry" );
				if( !ent->veh_category )
					ent->veh_category = 8;
			}
			else
				strcpy( shortstring4, "(Infantry)" );

			strcpy( colorstring1, "string2" );
			strcpy( colorstring2, "string2" );
			strcpy( colorstring3, "string2" );
			strcpy( colorstring4, "string2" );

			if( ent->veh_category & 1 ) // plane
			{
				strcpy( colorstring1, "string" );
			}
			else if( ent->veh_category & 2 ) // helo
			{
				strcpy( colorstring2, "string" );
			}
			else if( ent->veh_category & 4 ) // ground
			{
				strcpy( colorstring3, "string" );
			}
			else if( ent->veh_category & 8 ) // infantry
			{
				strcpy( colorstring4, "string" );
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

				"Select Category:",
				"----------------",
				colorstring1,
				shortstring1,
				colorstring2,
				shortstring2,
				colorstring3,
				shortstring3,
				colorstring4,
				shortstring4,
				"INCREASE and DECREASE",
				"to cycle, FIRE to",
			    "select, ESC to go back"
				);

			gi.WriteByte (svc_layout);
			gi.WriteString (string);
			gi.unicast(ent,false);

		}
		else if( ent->sel_step == 2 )
		{
			strcpy( colorstring1, "string2" );
			strcpy( colorstring2, "string2" );
			strcpy( colorstring3, "string2" );
			strcpy( colorstring4, "string2" );
			strcpy( colorstring5, "string2" );

			if( ent->veh_category & 1 ) // plane
			{
				strcpy( shortstring1, "Fighter" );
				strcpy( shortstring2, "Ground-Attack" );
				strcpy( shortstring3, "(Transport)" );
				strcpy( shortstring4, "" );
				strcpy( shortstring5, "" );
			}
			else if( ent->veh_category & 2 ) // helo
			{
				strcpy( shortstring1, "Attack" );
				strcpy( shortstring2, "Scout" );
				strcpy( shortstring3, "(Transport)" );
				strcpy( shortstring4, "" );
				strcpy( shortstring5, "" );
			}
			else if( ent->veh_category & 4 ) // ground
			{
				strcpy( shortstring1, "Tank" );
				strcpy( shortstring2, "Scout" );
				strcpy( shortstring3, "(APC)" );
				strcpy( shortstring4, "Special" );
				strcpy( shortstring5, "" );
			}
			else if( ent->veh_category & 8 ) // infantry
			{
				strcpy( shortstring1, "(Pilot/Driver)" );
				strcpy( shortstring2, "(Infantry Team)" );
				strcpy( shortstring3, "" );
				strcpy( shortstring4, "" );
				strcpy( shortstring5, "" );
			}
			switch( ent->veh_class )
			{
				case 1:
						strcpy( colorstring1, "string" );
						break;
				case 2:
						strcpy( colorstring2, "string" );
						break;
				case 3:
						strcpy( colorstring3, "string" );
						break;
				case 4:
						strcpy( colorstring4, "string" );
						break;
				case 5:
						strcpy( colorstring5, "string" );
						break;
			}


			Com_sprintf (string, sizeof(string),
				"xv 60 yv 40 string2 \"%s\" "
				"xv 60 yv 47 string2 \"%s\" "
				"xv 60 yv 80 \"%s\" \"%s\" "
				"xv 60 yv 90 \"%s\" \"%s\" "
				"xv 60 yv 100 \"%s\" \"%s\" "
				"xv 60 yv 110 \"%s\" \"%s\" "
				"xv 60 yv 120 \"%s\" \"%s\" "
				"xv 60 yv 145 string2 \"%s\" "
				"xv 60 yv 155 string2 \"%s\" "
				"xv 60 yv 165 string2 \"%s\" ",

				"Select Class:",
				"-------------",
				colorstring1,
				shortstring1,
				colorstring2,
				shortstring2,
				colorstring3,
				shortstring3,
				colorstring4,
				shortstring4,
				colorstring5,
				shortstring5,
				"INCREASE and DECREASE",
				"to cycle, FIRE to",
			    "select, ESC to go back"
				);

			gi.WriteByte (svc_layout);
			gi.WriteString (string);
			gi.unicast(ent,false);
		}
		else if( ent->sel_step == 3 )
		{
			strcpy( shortstring2, ent->veh_picname );
			strcpy( shortstring1, ent->veh_name );

			Com_sprintf (string, sizeof(string),
				"xv 139 yv 35 picn \"%s\" "
				"xv 60 yv 40 string2 \"%s\" "
				"xv 60 yv 47 string2 \"%s\" "
				"xv 60 yv 57 string \"%s\" "
				"xv 60 yv 145 string2 \"%s\" "
				"xv 60 yv 155 string2 \"%s\" "
				"xv 60 yv 165 string2 \"%s\" ",

				shortstring2,
				"Select Type:",
				"------------",
				shortstring1,
				"INCREASE and DECREASE",
				"to cycle, FIRE to",
			    "select, ESC to go back"
				);

			gi.WriteByte (svc_layout);
			gi.WriteString (string);
			gi.unicast(ent,false);
		}
	}

	//
	// +BD speed gauge
	//
	if( ent->client->veh_selected )
		ent->client->ps.stats[STAT_SPEED] = ent->vehicle->actualspeed/8;
	else
		ent->client->ps.stats[STAT_SPEED] = 0;
//	ent->client->ps.stats[STAT_SPEED_PIC] = gi.imageindex( "speed" );

	//
	// +BD ripple, modifies ammo icon if ripple != 1
	//
/*	if( ent->client->veh_selected )
	{
		if( strcmp(ent->client->pers.secondary->pickup_name, "Rocket Launcher") == 0 )
		{
			switch( ent->vehicle->ripple_f )
			{
				case 1:
						ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex( "grffar_1" );
						break;
				case 2:
						ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex( "grffar_2" );
						break;
				case 4:
						ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex( "grffar_4" );
						break;
			}
		}
		else if( strcmp(ent->client->pers.secondary->pickup_name, "Iron Bombs") == 0 )
		{
			switch( ent->vehicle->ripple_b )
			{
				case 1:
						ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex( "grbomb_1" );
						break;
				case 2:
						ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex( "grbomb_2" );
						break;
				case 4:
						ent->client->ps.stats[STAT_AMMO_ICON] = gi.imageindex( "grbomb_4" );
						break;
			}
		}
	}
*/	//
	// +BD speedbrake icon and gear icon, flares etc
	//
	if( ent->client->veh_selected )
	{
		ent->client->ps.stats[STAT_FLARES] = ent->vehicle->flare;
		ent->client->ps.stats[STAT_FLARE_PIC] = gi.imageindex( "flare" );
		ent->client->ps.stats[STAT_SWITCH_PIC] = gi.imageindex( "switch" );
		ent->client->ps.stats[STAT_AUTOCANNON_PIC] = gi.imageindex( "cannon" );
		ent->client->ps.stats[STAT_AUTOCANNON_AMMO] =
	   			 ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))];
		if( ent->vehicle->DAMAGE & SPEEDBRAKE_DAMAGE )
		{
			ent->client->ps.stats[STAT_BRAKE] = gi.imageindex( "brake_dam" );
		}
		else
		{
			if( ent->vehicle->ONOFF & BRAKE )
				ent->client->ps.stats[STAT_BRAKE] = gi.imageindex( "brake" );
			else
				ent->client->ps.stats[STAT_BRAKE] = 0;
		}
		if( ent->vehicle->DAMAGE & GEAR_DAMAGE )
		{
			ent->client->ps.stats[STAT_GEAR] = gi.imageindex( "gear_dam" );
		}
		else
		{
			if( ent->vehicle->ONOFF & GEARDOWN )
				ent->client->ps.stats[STAT_GEAR] = gi.imageindex( "gear" );
			else
				ent->client->ps.stats[STAT_GEAR] = 0;
		}
		if( ent->vehicle->DAMAGE & WEAPONBAY_DAMAGE )
		{
			ent->client->ps.stats[STAT_BAY_PIC] = gi.imageindex( "bay_dam" );
		}
		else
		{
			if( ent->vehicle->ONOFF & BAY_OPEN )
				ent->client->ps.stats[STAT_BAY_PIC] = gi.imageindex( "bay" );
			else
				ent->client->ps.stats[STAT_BAY_PIC] = 0;
		}
		if( ent->vehicle->thrust == THRUST_0 )
			ent->client->ps.stats[STAT_THROTTLE_PIC] = gi.imageindex( "Throt_1" );
		else if( ent->vehicle->thrust == THRUST_25 )
			ent->client->ps.stats[STAT_THROTTLE_PIC] = gi.imageindex( "Throt_2" );
		else if( ent->vehicle->thrust == THRUST_50 )
			ent->client->ps.stats[STAT_THROTTLE_PIC] = gi.imageindex( "Throt_3" );
		else if( ent->vehicle->thrust == THRUST_75 )
			ent->client->ps.stats[STAT_THROTTLE_PIC] = gi.imageindex( "Throt_4" );
	 	else if( ent->vehicle->thrust == THRUST_100 )
			ent->client->ps.stats[STAT_THROTTLE_PIC] = gi.imageindex( "Throt_5" );
		else if( ent->vehicle->thrust == THRUST_AFTERBURNER )
			ent->client->ps.stats[STAT_THROTTLE_PIC] = gi.imageindex( "Throt_6" );
		if( ent->vehicle->lockstatus == 0 )
			ent->client->ps.stats[STAT_LOCK_PIC] = gi.imageindex( "Health_1" );
		else if( ent->vehicle->lockstatus == 1 )
			ent->client->ps.stats[STAT_LOCK_PIC] = gi.imageindex( "Health_2" );
		else if( ent->vehicle->lockstatus == 2 )
			ent->client->ps.stats[STAT_LOCK_PIC] = gi.imageindex( "Health_3" );
		if( ent->vehicle->enemy )
		{
			if( teamplay->value != 1 )
				ent->client->ps.stats[STAT_TEMP_CROSSHAIR] = gi.imageindex( "lock" );
			else
			{
			if( ent->vehicle->enemy->aqteam != ent->vehicle->aqteam )
				ent->client->ps.stats[STAT_TEMP_CROSSHAIR] = gi.imageindex( "lock" );
			else
				ent->client->ps.stats[STAT_TEMP_CROSSHAIR] = gi.imageindex( "frlock" );
			}
		}
		else
		{
			ent->client->ps.stats[STAT_TEMP_CROSSHAIR] = gi.imageindex( "nolock" );
		}
		if( ent->client->showinventory )
			ent->client->ps.stats[STAT_TEMP_CROSSHAIR] = 0;
		if( ent->vehicle->ONOFF & GPS )
		{
			ent->client->ps.stats[STAT_GPS_X] = ent->vehicle->s.origin[0];
			ent->client->ps.stats[STAT_GPS_Y] = ent->vehicle->s.origin[1];
					ent->client->ps.stats[STAT_GPS_PIC] = gi.imageindex("gps");
			if( strcmp( ent->vehicle->classname, "plane" ) == 0 ||
				strcmp( ent->vehicle->classname, "helo" ) == 0 )
				ent->client->ps.stats[STAT_ALTITUDE] = ent->vehicle->altitude;
			else
				ent->client->ps.stats[STAT_ALTITUDE] = 0;
			ent->client->ps.stats[STAT_HDG] = (int)(ent->vehicle->s.angles[1]);
		}
		else
		{
			ent->client->ps.stats[STAT_GPS_X] = 0;
			ent->client->ps.stats[STAT_GPS_Y] = 0;
			ent->client->ps.stats[STAT_ALTITUDE] = 0;
			ent->client->ps.stats[STAT_GPS_PIC] = 0;
			ent->client->ps.stats[STAT_HDG] = 0;
		}
		if( strcmp( ent->vehicle->classname,"plane" ) == 0 ||
			strcmp( ent->vehicle->classname,"helo" ) == 0 )
		{
			if( ent->vehicle->DAMAGE & AUTOPILOT_DAMAGE )
			{
				ent->client->ps.stats[STAT_AUTOPILOT] = gi.imageindex( "auto_dam" );
			}
			else
			{
				if( ent->vehicle->ONOFF & AUTOPILOT )
					ent->client->ps.stats[STAT_AUTOPILOT] = gi.imageindex( "auto" );
				else
					ent->client->ps.stats[STAT_AUTOPILOT] = 0;
			}
		}
	}
	else
	{
		ent->client->ps.stats[STAT_BRAKE] = 0;
		ent->client->ps.stats[STAT_GEAR] = 0;
		ent->client->ps.stats[STAT_BAY_PIC] = 0;
		ent->client->ps.stats[STAT_THROTTLE_PIC] = 0;
		ent->client->ps.stats[STAT_FLARE_PIC] = 0;
		ent->client->ps.stats[STAT_SWITCH_PIC] = 0;
		ent->client->ps.stats[STAT_FLARES] = 0;
		ent->client->ps.stats[STAT_LOCK_PIC] = 0;
		ent->client->ps.stats[STAT_TEMP_CROSSHAIR] = 0;
		ent->client->ps.stats[STAT_AUTOPILOT] = 0;
		ent->client->ps.stats[STAT_AUTOCANNON_PIC] = 0;
		ent->client->ps.stats[STAT_AUTOCANNON_AMMO] = 0;
	}


	//
	// +BD new inventory
	//
	if( ent->client->veh_selected && ent->client->showinventory &&
		!ent->client->showscores && !ent->client->selectvehicle )
	{
		ent->client->ps.stats[STAT_SELBACKGROUND] = gi.imageindex ( "mfd" );

		if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Rocket Launcher"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] != 1 )
				sprintf( tempstring, "%d Rockets",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]  );
			else
				sprintf( tempstring, "1 Rocket" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < 10 )
				strcpy( shortstring1, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < 100 )
				strcpy( shortstring1, " " );
			else
				strcpy( shortstring1, "" );
			strcat( shortstring1, tempstring );
			sprintf( colorstring1, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
			strcpy( shortstring1, "" );
			strcpy( colorstring1, "140" );
		}
		if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("AutoCannon"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] != 1 )
				sprintf( tempstring, "%d Autocannon Rounds",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]  );
			else
				sprintf( tempstring, "1 Autocannon Round" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] < 10 )
				strcpy( shortstring2, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] < 100 )
				strcpy( shortstring2, " " );
			else
				strcpy( shortstring2, "" );
			strcat( shortstring2, tempstring );
			sprintf( colorstring2, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
			strcpy( shortstring2, "" );
			strcpy( colorstring2, "140" );
		}
		if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("SW Launcher"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] != 1 )
			{
				if( !(ent->vehicle->HASCAN & IS_EASTERN) )
					sprintf( tempstring, "%d Sidewinders",
						ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))]  );
				else
					sprintf( tempstring, "%d Archers",
						ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))]  );
			}
			else
			{
				if( !(ent->vehicle->HASCAN & IS_EASTERN) )
					sprintf( tempstring, "1 Sidewinder" );
				else
					sprintf( tempstring, "1 Archer");
			}
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] < 10 )
				strcpy( shortstring3, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] < 100 )
				strcpy( shortstring3, " " );
			else
				strcpy( shortstring3, "" );
			strcat( shortstring3, tempstring );
			sprintf( colorstring3, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
			strcpy( shortstring3, "" );
			strcpy( colorstring3, "140" );
		}
		if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Iron Bombs"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] != 1 )
				sprintf( tempstring, "%d Bombs",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))]  );
			else
				sprintf( tempstring, "1 Bomb" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] < 10 )
				strcpy( shortstring4, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] < 100 )
				strcpy( shortstring4, " " );
			else
				strcpy( shortstring4, "" );
			strcat( shortstring4, tempstring );
			sprintf( colorstring4, "%d", linenumber );
			linenumber += 10;
		}
		else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Laser Bombs"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] != 1 )
				sprintf( tempstring, "%d Laser Guided Bombs",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))]  );
			else
				sprintf( tempstring, "1 Laser Guided Bomb" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] < 10 )
				strcpy( shortstring4, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] < 100 )
				strcpy( shortstring4, " " );
			else
				strcpy( shortstring4, "" );
			strcat( shortstring4, tempstring );
			sprintf( colorstring4, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
			strcpy( shortstring4, "" );
			strcpy( colorstring4, "140" );
		}
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("MainGun"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] != 1 )
				sprintf( tempstring, "%d Gunshells",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))]  );
			else
				sprintf( tempstring, "1 Gunshell" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] < 10 )
				strcpy( shortstring5, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] < 100 )
				strcpy( shortstring5, " " );
			else
				strcpy( shortstring5, "" );
			strcat( shortstring5, tempstring );
			sprintf( colorstring5, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
			strcpy( shortstring5, "" );
			strcpy( colorstring5, "140" );
		}
		if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM Launcher"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] != 1 )
				sprintf( tempstring, "%d ATGMs",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))]  );
			else
				sprintf( tempstring, "1 ATGM" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] < 10 )
				strcpy( shortstring6, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] < 100 )
				strcpy( shortstring6, " " );
			else
				strcpy( shortstring6, "" );
			strcat( shortstring6, tempstring );
			sprintf( colorstring6, "%d", linenumber );
			linenumber += 10;
		}
		else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick Launcher"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] != 1 )
				sprintf( tempstring, "%d Mavericks",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))]  );
			else
				sprintf( tempstring, "1 Maverick" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] < 10 )
				strcpy( shortstring6, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] < 100 )
				strcpy( shortstring6, " " );
			else
				strcpy( shortstring6, "" );
			strcat( shortstring6, tempstring );
			sprintf( colorstring6, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
 			strcpy( shortstring6, "" );
			strcpy( colorstring6, "140" );
		}
		if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("ST Launcher"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] != 1 )
				sprintf( tempstring, "%d Stingers",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))]  );
			else
				sprintf( tempstring, "1 Stinger" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] < 10 )
				strcpy( shortstring9, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] < 100 )
				strcpy( shortstring9, " " );
			else
				strcpy( shortstring9, "" );
			strcat( shortstring9, tempstring );
			sprintf( colorstring9, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
			strcpy( shortstring9, "" );
			strcpy( colorstring9, "140" );
		}
		if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("AM Launcher"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] != 1 )
			{
				if( !(ent->vehicle->HASCAN & IS_EASTERN) )
					sprintf( tempstring, "%d AMRAAMs",
						ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))]  );
				else
					sprintf( tempstring, "%d Alamos",
						ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))]  );
			}
			else
			{
				if( !(ent->vehicle->HASCAN & IS_EASTERN) )
					sprintf( tempstring, "1 AMRAAM" );
				else
					sprintf( tempstring, "1 Alamo" );
			}
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] < 10 )
				strcpy( shortstring10, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] < 100 )
				strcpy( shortstring10, " " );
			else
				strcpy( shortstring10, "" );
			strcat( shortstring10, tempstring );
			sprintf( colorstring10, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
			strcpy( shortstring10, "" );
			strcpy( colorstring10, "140" );
		}
		if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("PH Launcher"))] )
		{
			if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] != 1 )
				sprintf( tempstring, "%d Phoenix",
					ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))]  );
			else
				sprintf( tempstring, "1 Phoenix" );
			if(	ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] < 10 )
				strcpy( shortstring11, "  " );
			else if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] < 100 )
				strcpy( shortstring11, " " );
			else
				strcpy( shortstring11, "" );
			strcat( shortstring11, tempstring );
			sprintf( colorstring11, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
			strcpy( shortstring11, "" );
			strcpy( colorstring11, "140" );
		}
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ECM"))] )
		{
			strcpy( shortstring8, "  1 ECM System" );
			sprintf( colorstring8, "%d", linenumber );
			linenumber += 10;
		}
		else
		{
 			strcpy( shortstring8, "" );
			strcpy( colorstring8, "140" );
		}
		if( ent->client->pers.max_flares )
		{
			if( ent->vehicle->flare != 1 )
				sprintf( tempstring, "%d Flares", ent->vehicle->flare );
			else
				sprintf( tempstring, "1 Flare" );
			if(	ent->vehicle->flare < 10 )
				strcpy( shortstring7, "  " );
			else if( ent->vehicle->flare < 100 )
				strcpy( shortstring7, " " );
			else
				strcpy( shortstring7, "" );
			strcat( shortstring7, tempstring );
			sprintf( colorstring7, "%d", linenumber );
		}
		else
		{
			strcpy( shortstring7, "" );
			strcpy( colorstring7, "140" );
		}

		Com_sprintf (string, sizeof(string),
			"xv 60 yv 50 string2 \"%s\" "
			"xv 60 yv 57 string2 \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" "
			"xv 60 yv \"%s\" string \"%s\" ",

			"Inventory:",
			"----------",
			colorstring1,
			shortstring1,
			colorstring2,
			shortstring2,
			colorstring3,
			shortstring3,
			colorstring4,
			shortstring4,
			colorstring5,
			shortstring5,
			colorstring6,
			shortstring6,
			colorstring9,
			shortstring9,
			colorstring10,
			shortstring10,
			colorstring11,
			shortstring11,
			colorstring8,
			shortstring8,
			colorstring7,
			shortstring7
			);

		gi.WriteByte (svc_layout);
		gi.WriteString (string);
		gi.unicast(ent,false);
	}
	else if( ent->client->veh_selected && !ent->client->showinventory )
	{
		ent->client->ps.stats[STAT_SELBACKGROUND] = 0;
	}
	else if( !ent->client->veh_selected )
	{
		ent->client->showinventory = 0;
	}

	//
	// show enemy name
	//

/*	if( ent->client->veh_selected )
	{
		if( ent->vehicle->enemy && !ent->client->showinventory )
		{
			if( ent->vehicle->enemy->HASCAN & IS_DRONE )
				sprintf( shortstring1, "lock: %s (drone)", ent->vehicle->enemy->friendlyname );
		   	else if( !(ent->vehicle->enemy->ONOFF & PILOT_ONBOARD ) )
				sprintf( shortstring1, "lock: %s (empty)", ent->vehicle->enemy->friendlyname );
			else
				sprintf( shortstring1, "lock: %s (%s)", ent->vehicle->enemy->friendlyname,
			   		ent->vehicle->enemy->owner->client->pers.netname );
			// put here the part to show the name!
		}
	}
*/

	//
	// help icon / current weapon if not shown
	//
	if (ent->client->resp.helpchanged && (level.framenum&8) )
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex ("i_help");
	else if ( (ent->client->pers.hand == CENTER_HANDED || ent->client->ps.fov > 91)
		&& ent->client->pers.weapon)
		ent->client->ps.stats[STAT_HELPICON] = gi.imageindex (ent->client->pers.weapon->icon);
	else
		ent->client->ps.stats[STAT_HELPICON] = 0;
}

