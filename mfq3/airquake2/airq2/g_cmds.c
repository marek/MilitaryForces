#include "g_local.h"
#include "m_player.h"


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

/*
void SelectNextItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}
*/
/*
void SelectPrevItem (edict_t *ent, int itflags)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;

	cl = ent->client;

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (cl->pers.selected_item + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (!(it->flags & itflags))
			continue;

		cl->pers.selected_item = index;
		return;
	}

	cl->pers.selected_item = -1;
}
*/
/*
void ValidateSelectedItem (edict_t *ent)
{
	gclient_t	*cl;

	cl = ent->client;

	if (cl->pers.inventory[cl->pers.selected_item])
		return;		// valid

	SelectNextItem (ent, -1);
}
*/
/*
==================
Cmd_Use_f

Use an inventory item
==================
*/
void Cmd_Use_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

//	gi.bprintf( PRINT_HIGH, "use!" );

	if( !ent->client->veh_selected )
		return;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	if( strcmp( it->pickup_name, "AutoCannon" ) == 0 &&
		ent->vehicle->HASCAN & HAS_AUTOAIM )
	{
		Use_Secondary( ent, it );
	}
	else
		it->use (ent, it);
}


/*
==================
Cmd_Drop_f

Drop an inventory item
==================
*/
/*
void Cmd_Drop_f (edict_t *ent)
{
	int			index;
	gitem_t		*it;
	char		*s;

	s = gi.args();
	it = FindItem (s);
	if (!it)
	{
		gi.cprintf (ent, PRINT_HIGH, "unknown item: %s\n", s);
		return;
	}
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	index = ITEM_INDEX(it);
	if (!ent->client->pers.inventory[index])
	{
		gi.cprintf (ent, PRINT_HIGH, "Out of item: %s\n", s);
		return;
	}

	it->drop (ent, it);
}
*/

/*
=================
Cmd_Inven_f
=================
*/
void Cmd_Inven_f (edict_t *ent)
{
//	int			i;
	gclient_t	*cl;

	if( !ent->client->veh_selected )
	{
		if( !ent->client->selectvehicle )
			ent->client->selectvehicle = true;
		else
			ent->client->selectvehicle = false;
		return;
	}
	if( ent->vehicle->deadflag )
		return;

	cl = ent->client;

	cl->showscores = false;
	cl->selectvehicle = false;
	cl->showhelp = false;

	if (cl->showinventory)
	{
		cl->showinventory = false;
		return;
	}



	cl->showinventory = true;

/*	gi.WriteByte (svc_inventory);
	for (i=0 ; i<MAX_ITEMS ; i++)
	{
		gi.WriteShort (cl->pers.inventory[i]);
	}
	gi.unicast (ent, true);
*/
}

/*
=================
Cmd_InvUse_f
=================
*/
/*
void Cmd_InvUse_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to use.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->use)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not usable.\n");
		return;
	}
	it->use (ent, it);
}
*/
/*
=================
Cmd_WeapPrev_f
=================
*/
void ActivateSecondary( edict_t *ent, gitem_t *it );

void Cmd_WeapPrev_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	if( !ent->vehicle )
		return;

	if( ent->vehicle->enemy )
	{
		ent->vehicle->enemy->lockstatus = 0;
		ent->vehicle->enemy = NULL;
	}

	cl = ent->client;

	if (!cl->pers.secondary)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.secondary);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		if( !(ent->vehicle->HASCAN & HAS_AUTOAIM) &&
			strcmp( it->pickup_name, "AutoCannon" ) == 0 )
			continue;
//		it->use (ent, it);
		ActivateSecondary( ent, it );
		if (cl->pers.secondary == it)
			return;	// successful
	}
}

/*
=================
Cmd_WeapNext_f
=================
*/
void Cmd_WeapNext_f (edict_t *ent)
{
	gclient_t	*cl;
	int			i, index;
	gitem_t		*it;
	int			selected_weapon;

	if( !ent->vehicle )
		return;

	if( ent->vehicle->enemy )
	{
		ent->vehicle->enemy->lockstatus = 0;
		ent->vehicle->enemy = NULL;
	}

	cl = ent->client;

	if (!cl->pers.secondary)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.secondary);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		if( !(ent->vehicle->HASCAN & HAS_AUTOAIM) &&
			strcmp( it->pickup_name, "AutoCannon" ) == 0 )
			continue;
		ActivateSecondary( ent, it );
//		it->use (ent, it);
		if (cl->pers.secondary == it)
			return;	// successful
	}



// old weapon change
/*	if (!cl->pers.weapon)
		return;

	selected_weapon = ITEM_INDEX(cl->pers.weapon);

	// scan  for the next valid one
	for (i=1 ; i<=MAX_ITEMS ; i++)
	{
		index = (selected_weapon + MAX_ITEMS - i)%MAX_ITEMS;
		if (!cl->pers.inventory[index])
			continue;
		it = &itemlist[index];
		if (!it->use)
			continue;
		if (! (it->flags & IT_WEAPON) )
			continue;
		it->use (ent, it);
		if (cl->pers.weapon == it)
			return;	// successful
	}
*/
}

/*
=================
Cmd_WeapLast_f
=================
*/
/*
void Cmd_WeapLast_f (edict_t *ent)
{
	gclient_t	*cl;
	int			index;
	gitem_t		*it;

	cl = ent->client;

	if (!cl->pers.weapon || !cl->pers.lastweapon)
		return;

	index = ITEM_INDEX(cl->pers.lastweapon);
	if (!cl->pers.inventory[index])
		return;
	it = &itemlist[index];
	if (!it->use)
		return;
	if (! (it->flags & IT_WEAPON) )
		return;
	it->use (ent, it);
}
*/

/*
=================
Cmd_InvDrop_f
=================
*/
/*
void Cmd_InvDrop_f (edict_t *ent)
{
	gitem_t		*it;

	ValidateSelectedItem (ent);

	if (ent->client->pers.selected_item == -1)
	{
		gi.cprintf (ent, PRINT_HIGH, "No item to drop.\n");
		return;
	}

	it = &itemlist[ent->client->pers.selected_item];
	if (!it->drop)
	{
		gi.cprintf (ent, PRINT_HIGH, "Item is not dropable.\n");
		return;
	}
	it->drop (ent, it);
}
*/
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
	ent->sel_step--;
	if( teamplay->value == 1 )
	{
		if( ent->sel_step < 1 )
		{
			if( ent->sel_step < 0 )
			{
				ent->client->selectvehicle = false;
				ent->sel_step = 0;
			}
			else if( (ent->client->pers.team == 3 || ent->client->pers.team == 4) && ent->sel_step <= 1)
				ent->sel_step = 0;
			else if( (ent->client->pers.team == 1 || ent->client->pers.team == 2) && ent->sel_step <= 1 )
			{
				ent->client->selectvehicle = false;
				ent->sel_step = 1;
			}
		}
	}
	else
	{
		if( ent->sel_step < 1 )
		{
			ent->client->selectvehicle = false;
			ent->sel_step = 1;
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
Cmd_Say_f
==================
*/
void Cmd_Say_f (edict_t *ent, qboolean team, qboolean arg0)
{
	int		j;
	edict_t	*other;
	char	*p;
	char	text[2048];

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
//	else if (Q_stricmp (cmd, "drop") == 0)
//a		Cmd_Drop_f (ent);
	else if (Q_stricmp (cmd, "inven") == 0)
		Cmd_Inven_f (ent);
//	else if (Q_stricmp (cmd, "invnext") == 0)
//		SelectNextItem (ent, -1);
//	else if (Q_stricmp (cmd, "invprev") == 0)
//		SelectPrevItem (ent, -1);
//	else if (Q_stricmp (cmd, "invnextw") == 0)
//		SelectNextItem (ent, IT_WEAPON);
//	else if (Q_stricmp (cmd, "invprevw") == 0)
//		SelectPrevItem (ent, IT_WEAPON);
//	else if (Q_stricmp (cmd, "invnextp") == 0)
//		SelectNextItem (ent, IT_POWERUP);
//	else if (Q_stricmp (cmd, "invprevp") == 0)
//		SelectPrevItem (ent, IT_POWERUP);
//	else if (Q_stricmp (cmd, "invuse") == 0)
//		Cmd_InvUse_f (ent);
	else if (Q_stricmp (cmd, "weapprev") == 0)
		Cmd_WeapPrev_f (ent);
	else if (Q_stricmp (cmd, "weapnext") == 0)
		Cmd_WeapNext_f (ent);
//	else if (Q_stricmp (cmd, "weaplast") == 0)
//		Cmd_WeapLast_f (ent);
	else if (Q_stricmp (cmd, "kill") == 0)
		Cmd_KillVehicle_f (ent);	// +BD
	else if (Q_stricmp (cmd, "putaway") == 0)
		Cmd_PutAway_f (ent);
	// now the airquake 2 commands:
	else if (Q_stricmp (cmd, "increase") == 0)
	{
		if( ent->client->veh_selected && !ent->deadflag )
			Cmd_Thrustup_f (ent);
		else if( !ent->client->veh_selected )
			 Cmd_VehicleDown_f (ent);
	}
	else if (Q_stricmp (cmd, "decrease") == 0)
	{
		if( ent->client->veh_selected && !ent->deadflag )
			Cmd_Thrustdown_f (ent);
		else if( !ent->client->veh_selected )
			Cmd_VehicleUp_f(ent);
	}
	else if (Q_stricmp (cmd, "cameraup") == 0)
		Cmd_CameraUp_f (ent);
	else if (Q_stricmp (cmd, "cameradown") == 0)
		Cmd_CameraDown_f (ent);
	else if (Q_stricmp (cmd, "cameranear") == 0)
		Cmd_CameraNear_f (ent);
	else if (Q_stricmp (cmd, "camerafar") == 0)
		Cmd_CameraFar_f (ent);
	else if (Q_stricmp (cmd, "cameramode") == 0)
		Cmd_CameraMode_f (ent);
	else if (Q_stricmp (cmd, "speedbrake") == 0)
		Cmd_Speedbrake_f (ent);
	else if (Q_stricmp (cmd, "gear") == 0)
		Cmd_Gear_f (ent);
	else if (Q_stricmp (cmd, "flare") == 0)
		Cmd_Flare_f (ent);
	else if (Q_stricmp (cmd, "coord") == 0)
		Cmd_Coord_f (ent);
	else if (Q_stricmp (cmd, "roll") == 0)
		Cmd_Roll_f (ent);
	else if (Q_stricmp (cmd, "videomode") == 0)
		Cmd_FixRoll_f(ent);
//	else if (Q_stricmp (cmd, "drone") == 0)
//	 	Cmd_Spawn_Drone(ent);
	else if (Q_stricmp (cmd, "aqversion") == 0 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "AirQuake 2 - Version 0.85 - OFFICIAL UPGRADE" );
		ent->vehicle->DAMAGE |= WEAPONBAY_DAMAGE;
	}
//	else if( Q_stricmp( cmd, "ass" ) == 0 )
//		gi.bprintf( PRINT_HIGH, "cat:%d\n", level.category );
/*	else if( Q_stricmp (cmd, "ripple") == 0)
	{
//		Cmd_Ripple_f(ent);
//		ent->vehicle->DAMAGE = 511;
//		ent->vehicle->DAMAGE &= ~ENGINE0;
	}*/
	else if( Q_stricmp (cmd, "gps") == 0)
		Cmd_GPS_f(ent);
//	else if( Q_stricmp (cmd, "light") == 0)
//		Cmd_Light_f(ent);
	else if( Q_stricmp (cmd, "contact_tower") == 0 )
		Cmd_Tower_f(ent);
	else if( Q_stricmp (cmd, "autopilot") == 0)
		Cmd_Autopilot_f(ent);
	else if( Q_stricmp (cmd, "zoom") == 0 )
		Cmd_Zoom_f( ent );
	else if( Q_stricmp (cmd, "unlock" ) == 0 )
		Cmd_Unlock_f( ent );
//	else if( Q_stricmp (cmd, "autoaim" ) == 0 )
//		Cmd_Autoaim_f (ent);
	else if( Q_stricmp (cmd, "bay") == 0 )
		Cmd_OpenBay_f( ent );
	else if( Q_stricmp (cmd, "pickle" ) == 0 )
		Cmd_PickleToggle_f (ent);
	else if( Q_stricmp (cmd, "vehicleinfo" ) == 0 )
		Cmd_VehicleInfo_f (ent);
	else if( Q_stricmp (cmd, "eject" ) == 0 )
		Cmd_Eject_f( ent );
	else	// anything that doesn't match a command will be a chat
		Cmd_Say_f (ent, false, true);
}
