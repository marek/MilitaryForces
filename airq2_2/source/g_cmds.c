#include "g_local.h"





char *ClientTeam (edict_t *ent)
{
	char		*p;
	static char	value[512];

	value[0] = 0;

	if (!ent->client)
		return value;

	strcpy(value, Info_ValueForKey (ent->client->pers.userinfo, "skin"));
	p = strchr(value, '/');
	if (!p)
		return value;

	if ((int)(dmflags->value) & DF_MODELTEAMS)
	{
		*p = 0;
		return value;
	}

	// if ((int)(dmflags->value) & DF_SKINTEAMS)
	return ++p;
}

qboolean OnSameTeam (edict_t *ent1, edict_t *ent2)
{
	char	ent1Team [512];
	char	ent2Team [512];

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		return false;

	strcpy (ent1Team, ClientTeam (ent1));
	strcpy (ent2Team, ClientTeam (ent2));

	if (strcmp(ent1Team, ent2Team) == 0)
		return true;
	return false;
}

//=================================================================================


/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	char		*s;
	int			num;

	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;

	s = gi.args();

	// select weapon
	if( (gi.argc() == 3) && (strcmp( gi.argv(1), "weapon" ) == 0) && !(ent->vehicle->deadflag) )
	{
		num = atoi( gi.argv(2) );
		if( num > 0 && num < MAX_WEAPONS )
		{
			if( ent->vehicle->weapon[num] != WEAPON_NONE )
			{
				ent->vehicle->active_weapon = num;
//				gi.bprintf( PRINT_HIGH, "Weapon %d\n", num );
			}
		}
	}
}


/*
=================
Cmd_WeapPrev_f
=================
*/
void Cmd_WeapPrev_f (edict_t *ent)
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;

	if( ent->vehicle->deadflag == DEAD_DEAD)
		return;

	ent->vehicle->active_weapon--;
	if( !(ent->vehicle->HASCAN & HASCAN_AUTOAIM) )
	{
		if( ent->vehicle->active_weapon < 1 )
		{
			ent->vehicle->active_weapon = MAX_WEAPONS - 1;
			while( ent->vehicle->weapon[ent->vehicle->active_weapon] == WEAPON_NONE )
				ent->vehicle->active_weapon--;
			if( ent->vehicle->active_weapon < 1 )
				ent->vehicle->active_weapon = 1;
		}
	}
	else
	{
		if( ent->vehicle->active_weapon < 0 )
		{
			ent->vehicle->active_weapon = MAX_WEAPONS - 1;
			while( ent->vehicle->weapon[ent->vehicle->active_weapon] == WEAPON_NONE )
				ent->vehicle->active_weapon--;
			if( ent->vehicle->active_weapon < 1 )
				ent->vehicle->active_weapon = 1;
		}
	}
//	gi.bprintf( PRINT_HIGH, "Switching to weapon %d\n", vehicle->active_weapon );
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
		return;

	if( ent->vehicle->deadflag == DEAD_DEAD )
		return;

	// assumes that there is always a weapon[1]
	ent->vehicle->active_weapon++;
	if( ent->vehicle->weapon[ent->vehicle->active_weapon] == WEAPON_NONE )
	{
		if( !(ent->vehicle->HASCAN & HASCAN_AUTOAIM ) )
			ent->vehicle->active_weapon = 1;
		else
			ent->vehicle->active_weapon = 0;
	}
	if( ent->vehicle->active_weapon >= MAX_WEAPONS )
	{
		if( !(ent->vehicle->HASCAN & HASCAN_AUTOAIM ) )
			ent->vehicle->active_weapon = 1;
		else
			ent->vehicle->active_weapon = 0;
	}

//	gi.bprintf( PRINT_HIGH, "Switching to weapon %d\n", vehicle->active_weapon );
}



/*
=================
Cmd_Kill_f
=================
*/
void Cmd_Kill_f (edict_t *ent)
{
	if((level.time - ent->client->respawn_time) < 5)
		return;
//	if( strcmp( ent->client->pers.netname, "Bjoern" ) != 0 &&
//		strcmp( ent->client->pers.netname, "Miles" ) != 0 )
//		return;
	if( ent->client->FLAGS & CLIENT_VEH_SELECTED )	
	{
		if( ent->vehicle->deadflag )
			return;
		ent->vehicle->health = 0;
		if( strcmp( ent->vehicle->classname, "LQM" ) != 0 )
		{
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (ent->vehicle->s.origin);
			gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);
			VehicleExplosion( ent->vehicle );
			if( ent->vehicle->ONOFF & ONOFF_AIRBORNE )
				ent->vehicle->deadflag = DEAD_DYING;
			else
			{
				ent->vehicle->deadflag = DEAD_DEAD;
				SetVehicleModel( ent->vehicle );
				VectorClear( ent->vehicle->velocity );
				VectorClear( ent->vehicle->avelocity );
			}
		}
		else
		{
			if( !(ent->vehicle->ONOFF & ONOFF_AIRBORNE) )
			{
				ent->vehicle->speed = 0;
				VectorClear( ent->vehicle->velocity );
			}
			ent->vehicle->deadflag = DEAD_DYING;
			ent->vehicle->mins[2] = -2;
			ent->vehicle->s.angles[0] = 0;
			gi.sound (ent->vehicle, CHAN_VOICE, gi.soundindex(va("*death%i.wav", (rand()%4)+1)), 1, ATTN_NORM, 0);
			gi.bprintf( PRINT_HIGH, "%s commited suicide.\n", ent->client->pers.netname );
		}
	}
}

/*
=================
Cmd_PutAway_f
=================
*/
void Cmd_PutAway_f (edict_t *ent)
{
	ent->client->showscores = false;
	ent->client->showhelp = false;
	ent->client->showinventory = false;
	if( ent->client->selectvehicle )
	{
		ent->client->sel_step--;
		if( teamplay->value )
		{
			// re-selecting
			if( ent->mass == -1 )
			{
				if( ent->client->sel_step < SEL_CAT )
				{
					ent->client->selectvehicle = false;
					ent->client->sel_step = SEL_CAT;
				}
			}
			// new selection
			else
			{
				if( ent->client->sel_step < SEL_TEAM )
				{
					ent->client->selectvehicle = false;
					ent->client->sel_step = SEL_TEAM;
				}
				else if( ent->client->sel_step == SEL_TEAM )
				{
					teams[ent->client->team].players--;
				}
			}
		}
		else
		{
			if( ent->client->sel_step < SEL_CAT )
			{
				ent->client->selectvehicle = false;
				ent->client->sel_step = SEL_CAT;
			}
		}
	}
}


int PlayerSort (void const *a, void const *b)
{
	int		anum, bnum;

	anum = *(int *)a;
	bnum = *(int *)b;

	anum = game.clients[anum].ps.stats[STAT_FRAGS];
	bnum = game.clients[bnum].ps.stats[STAT_FRAGS];

	if (anum < bnum)
		return -1;
	if (anum > bnum)
		return 1;
	return 0;
}

/*
=================
Cmd_Players_f
=================
*/
void Cmd_Players_f (edict_t *ent)
{
	int		i;
	int		count;
	char	small[64];
	char	large[1280];
	int		index[256];

	count = 0;
	for (i = 0 ; i < maxclients->value ; i++)
		if (game.clients[i].pers.connected)
		{
			index[count] = i;
			count++;
		}

	// sort by frags
	qsort (index, count, sizeof(index[0]), PlayerSort);

	// print information
	large[0] = 0;

	for (i = 0 ; i < count ; i++)
	{
		Com_sprintf (small, sizeof(small), "%3i %s\n",
			game.clients[index[i]].ps.stats[STAT_FRAGS],
			game.clients[index[i]].pers.netname);
		if (strlen (small) + strlen(large) > sizeof(large) - 100 )
		{	// can't print all of them in one packet
			strcat (large, "...\n");
			break;
		}
		strcat (large, small);
	}

	gi.cprintf (ent, PRINT_HIGH, "%s\n%i players\n", large, count);
}




/*
==================
Cmd_Inven_f
==================
*/

void Cmd_Inven_f (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	cl->showscores = false;
	cl->showhelp = false;	

	if( !(ent->client->FLAGS & CLIENT_VEH_SELECTED) )
	{
		cl->showinventory = false;
		if( !cl->selectvehicle )
			cl->selectvehicle = true;
		else
			cl->selectvehicle = false;
		return;
	}
	if( ent->vehicle->deadflag == DEAD_DEAD )
	{
		cl->showinventory = false;
		return;
	}
	
	cl->selectvehicle = false;
	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}

	cl->showinventory = true;
}


/*
==================
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		i, j;
	edict_t	*other;
	char	*p;
	char	text[2048];
	gclient_t *cl;

	if (gi.argc () < 2 && !arg0)
		return;

	if (!((int)(dmflags->value) & (DF_MODELTEAMS | DF_SKINTEAMS)))
		team = false;

	if (team)
		Com_sprintf (text, sizeof(text), "(%s): ", ent->client->pers.netname);
	else
		Com_sprintf (text, sizeof(text), "%s: ", ent->client->pers.netname);

	if (arg0)
	{
		strcat (text, gi.argv(0));
		strcat (text, " ");
		strcat (text, gi.args());
	}
	else
	{
		p = gi.args();

		if (*p == '"')
		{
			p++;
			p[strlen(p)-1] = 0;
		}
		strcat(text, p);
	}

	// don't let text be too long for malicious reasons
	if (strlen(text) > 150)
		text[150] = 0;

	strcat(text, "\n");

	if (flood_msgs->value) {
		cl = ent->client;

        if (level.time < cl->flood_locktill) {
			gi.cprintf(ent, PRINT_HIGH, "You can't talk for %d more seconds\n",
				(int)(cl->flood_locktill - level.time));
            return;
        }
        i = cl->flood_whenhead - flood_msgs->value + 1;
        if (i < 0)
            i = (sizeof(cl->flood_when)/sizeof(cl->flood_when[0])) + i;
		if (cl->flood_when[i] && 
			level.time - cl->flood_when[i] < flood_persecond->value) {
			cl->flood_locktill = level.time + flood_waitdelay->value;
			gi.cprintf(ent, PRINT_CHAT, "Flood protection:  You can't talk for %d seconds.\n",
				(int)flood_waitdelay->value);
            return;
        }
		cl->flood_whenhead = (cl->flood_whenhead + 1) %
			(sizeof(cl->flood_when)/sizeof(cl->flood_when[0]));
		cl->flood_when[cl->flood_whenhead] = level.time;
	}

	if (dedicated->value)
		gi.cprintf(NULL, PRINT_CHAT, "%s", text);

	for (j = 1; j <= game.maxclients; j++)
	{
		other = &g_edicts[j];
		if (!other->inuse)
			continue;
		if (!other->client)
			continue;
		if (team)
		{
			if (!OnSameTeam(ent, other))
				continue;
		}
		gi.cprintf(other, PRINT_CHAT, "%s", text);
	}
}

void Cmd_PlayerList_f(edict_t *ent)
{
	int i;
	char st[80];
	char text[1400];
	edict_t *e2;

	// connect time, ping, score, name
	*text = 0;
	for (i = 0, e2 = g_edicts + 1; i < maxclients->value; i++, e2++) {
		if (!e2->inuse)
			continue;

		Com_sprintf(st, sizeof(st), "%02d:%02d %4d %3d %s\n",
			(level.framenum - e2->client->resp.enterframe) / 600,
			((level.framenum - e2->client->resp.enterframe) % 600)/10,
			e2->client->ping,
			e2->client->resp.score,
			e2->client->pers.netname );
		if (strlen(text) + strlen(st) > sizeof(text) - 50) {
			sprintf(text+strlen(text), "And more...\n");
			gi.cprintf(ent, PRINT_HIGH, "%s", text);
			return;
		}
		strcat(text, st);
	}
	gi.cprintf(ent, PRINT_HIGH, "%s", text);
}


/*
=================
ClientCommand
=================
*/
void ClientCommand (edict_t *ent)
{
	char	*cmd;

	if (!ent->client)
		return;		// not fully in game yet

	cmd = gi.argv(0);

	if (Q_stricmp (cmd, "players") == 0)
	{
		Cmd_Players_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "say") == 0)
	{
		Cmd_Say_f (ent, false, false);
		return;
	}
	if (Q_stricmp (cmd, "say_team") == 0)
	{
		Cmd_Say_f (ent, true, false);
		return;
	}
	if (Q_stricmp (cmd, "score") == 0)
	{
		Cmd_Score_f (ent);
		return;
	}
	if (Q_stricmp (cmd, "help") == 0)
	{
		Cmd_Help_f (ent);
		return;
	}

	if (level.intermissiontime)
		return;

	if (Q_stricmp (cmd, "use") == 0)
		Cmd_Use_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_Kill_f (ent);
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	else if (Q_stricmp(cmd, "playerlist") == 0)
		Cmd_PlayerList_f(ent);
	// NewAirQuake2 commands here
	else if (Q_stricmp( cmd, "cam" ) == 0 )
		Cmd_Cameramode( ent );
	else if( Q_stricmp( cmd, "cam_x" ) == 0 )
		Cmd_Cam_x( ent );
	else if( Q_stricmp( cmd, "cam_y" ) == 0 )
		Cmd_Cam_y( ent );
	else if (Q_stricmp( cmd, "freecam" ) == 0 )
		ent->client->REPEAT |= REPEAT_FREELOOK;
	else if (Q_stricmp( cmd, "lockcam" ) == 0 )
		ent->client->REPEAT &= ~REPEAT_FREELOOK;
	else if (Q_stricmp( cmd, "increase" ) == 0 )
		Cmd_Increase( ent );
	else if (Q_stricmp( cmd, "beginthrottleup" ) == 0 )
		ent->client->REPEAT |= REPEAT_INCREASE;
	else if (Q_stricmp( cmd, "endthrottleup" ) == 0 )
		ent->client->REPEAT &= ~REPEAT_INCREASE;
	else if (Q_stricmp( cmd, "decrease" ) == 0 )
		Cmd_Decrease( ent );
	else if (Q_stricmp( cmd, "beginthrottledown" ) == 0 )
		ent->client->REPEAT |= REPEAT_DECREASE;
	else if (Q_stricmp( cmd, "endthrottledown" ) == 0 )
		ent->client->REPEAT &= ~REPEAT_DECREASE;
	else if( Q_stricmp( cmd, "fire" ) == 0 )
		Cmd_Weapon( ent );
	else if( Q_stricmp( cmd, "beginfire" ) == 0 )
		ent->client->REPEAT |= REPEAT_FIRE;
	else if( Q_stricmp( cmd, "endfire" ) == 0 )
		ent->client->REPEAT &= ~REPEAT_FIRE;
	else if (Q_stricmp( cmd, "gear" ) == 0 )
		Cmd_Gear( ent );
	else if( Q_stricmp( cmd, "brake" ) == 0 )
		Cmd_Brake( ent );
	else if( Q_stricmp( cmd, "beginbrake" ) == 0 )
		ent->client->REPEAT |= REPEAT_BRAKE;
	else if( Q_stricmp( cmd, "endbrake" ) == 0 )
		ent->client->REPEAT &= ~REPEAT_BRAKE;
	else if( Q_stricmp( cmd, "roll" ) == 0 )
		Cmd_Roll( ent );
	else if( Q_stricmp( cmd, "coord" ) == 0 )
		Cmd_Coord( ent );
	else if( Q_stricmp( cmd, "videomode" ) == 0 )
		Cmd_Rollfix( ent );
	else if( Q_stricmp (cmd, "contact_tower") == 0 )
		Cmd_Tower(ent);
	else if( Q_stricmp (cmd, "eject") == 0 )
		Cmd_Eject(ent);
	else if( Q_stricmp (cmd, "zoom") == 0 )
		Cmd_Zoom(ent);
	else if( Q_stricmp( cmd, "flare" ) == 0 )
		Cmd_Flare(ent);
	else if( Q_stricmp( cmd, "aq2version") == 0 )
	{
		gi.cprintf( ent, PRINT_HIGH, "AirQuake2 2.0 beta55\n" );
//		gi.bprintf( PRINT_HIGH, "All:%d, 1:%d, 2:%d, N:%d\n", level.category, teams[TEAM_1].category,
//			teams[TEAM_2].category, teams[TEAM_NEUTRAL].category );
	}
	else if( Q_stricmp( cmd, "botscores") == 0 )
		Cmd_Botscores();
	else if( Q_stricmp( cmd, "gps" ) == 0 )
		Cmd_GPS( ent );
	else if( Q_stricmp( cmd, "control_method" ) == 0 )
		Cmd_Control( ent );
	else if( Q_stricmp( cmd, "weaponbay" ) == 0 )
		Cmd_Bay( ent );
	// end NAQ2
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
