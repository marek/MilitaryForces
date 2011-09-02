#include "g_local.h"

void ClientUserinfoChanged (edict_t *ent, char *userinfo);

// checks wheter the spawning point for planes is over a runway or not
void IsRunwaySpawnPoint( edict_t *self )
{
	trace_t trace;
	vec3_t underneath;

	VectorSet( underneath, self->s.origin[0], self->s.origin[1],
				self->s.origin[2] - 32 );

	trace = gi.trace( self->s.origin, NULL, NULL, underneath, self, MASK_ALL );

	if( strcmp( trace.ent->classname, "func_runway" ) == 0 
		|| strcmp( trace.ent->classname, "func_door" ) == 0
		|| strcmp( trace.ent->classname, "func_door_rotating" ) == 0 )
	{
		self->ONOFF = ONOFF_LANDED;
	}
}

// checks wheter the spawning point for LQMs/GVs is aerial or not
// if aerial the LQM/GV starts on a parachute
void IsParaSpawnPoint( edict_t *self )
{
	trace_t trace;
	vec3_t underneath;

	VectorSet( underneath, self->s.origin[0], self->s.origin[1],
				self->s.origin[2] - 128 );

	trace = gi.trace( self->s.origin, NULL, NULL, underneath, self, MASK_ALL );

	if( trace.fraction < 1.0 )
	{
		self->ONOFF = ONOFF_LANDED;
	}
}


/*QUAKED info_player_start (1 0 0) (-16 -16 -24) (16 16 32)
The normal starting point for a level.
*/
void SP_info_player_start(edict_t *self)
{
	if( deathmatch->value )
	{
		G_FreeEdict( self );
		return;
	}

	// OVERRIDING FOR EARLY PHASE OF AIRQUAKE2:
	if( !self->sounds )
		self->sounds = CAT_AIR;
	self->sounds &= ~CAT_GROUND;
	self->sounds &= ~CAT_LQM;
	if( !self->sounds )
	{
		G_FreeEdict( self );
		return;
	}


	if( !self->sounds )
		level.category = CAT_ALL;
	else
		level.category |= self->sounds;

	if( self->sounds & CAT_PLANE )
	{
		self->think = IsRunwaySpawnPoint;
		self->nextthink = level.time + 1.2;
	}

	self->s.skinnum = 0;
	self->solid = SOLID_NOT;	
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, -16);
	gi.linkentity (self);
}

/*QUAKED info_player_deathmatch (1 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for deathmatch games
*/
void SP_info_player_deathmatch(edict_t *self)
{
	if (!deathmatch->value)
	{
		G_FreeEdict (self);
		return;
	}

	// OVERRIDING FOR EARLY PHASE OF AIRQUAKE2:
	if( !self->sounds )
		self->sounds = (CAT_AIR);
	self->sounds &= ~CAT_GROUND;
	self->sounds &= ~CAT_LQM;
	if( !(self->sounds & CAT_AIR) )
	{
		G_FreeEdict( self );
		return;
	}

	if( !self->sounds || self->sounds == CAT_ALL )
	{
		if( !teamplay->value )
			level.category = CAT_ALL;
		else
		{
			if( !self->aqteam )
			{
				if( !teamspots_only->value )
					teams[TEAM_1].category = teams[TEAM_2].category =
						teams[TEAM_NEUTRAL].category = level.category = CAT_ALL;
			}
			else
			{
				if( self->aqteam & TEAM_1 )
					teams[TEAM_1].category = CAT_ALL;
				if( self->aqteam & TEAM_2 )
					teams[TEAM_2].category = CAT_ALL;
				if( self->aqteam & TEAM_NEUTRAL )
					teams[TEAM_NEUTRAL].category = CAT_ALL;
			}
		}
	}
	else
	{
		if( !teamplay->value )
			level.category |= self->sounds;
		else
		{
			if( !self->aqteam )
			{
				if( !teamspots_only->value )
				{
					teams[TEAM_1].category |= self->sounds;
					teams[TEAM_2].category |= self->sounds;
					teams[TEAM_NEUTRAL].category |= self->sounds;
					level.category |= self->sounds;
				}
			}
			else
			{
				if( self->aqteam & TEAM_1 )
					teams[TEAM_1].category |= self->sounds;
				if( self->aqteam & TEAM_2 )
					teams[TEAM_2].category |= self->sounds;
				if( self->aqteam & TEAM_NEUTRAL )
					teams[TEAM_NEUTRAL].category |= self->sounds;
			}
		}
	}

	if( (self->sounds & CAT_PLANE) || (self->sounds & CAT_HELO) )
	{
		self->think = IsRunwaySpawnPoint;
		self->nextthink = level.time + 1.2;
	}
	else if( (self->sounds & CAT_GROUND) || (self->sounds & CAT_LQM) )
	{
		self->think = IsParaSpawnPoint;
		self->nextthink = level.time + 1.2;
	}

	self->s.skinnum = 0;
	self->solid = SOLID_NOT;	
	VectorSet (self->mins, -32, -32, -24);
	VectorSet (self->maxs, 32, 32, -16);
	gi.linkentity (self);

}



/*QUAKED info_player_intermission (1 0 1) (-16 -16 -24) (16 16 32)
The deathmatch intermission point will be at one of these
Use 'angles' instead of 'angle', so you can set pitch or roll as well as yaw.  'pitch yaw roll'
*/
void SP_info_player_intermission(void)
{
}


//=======================================================================

/*
==============
InitClientPersistant

This is only called when the game first initializes in single player,
but is called after each death and level change in deathmatch
==============
*/
void InitClientPersistant (gclient_t *client)
{
	memset (&client->pers, 0, sizeof(client->pers));

	client->pers.health			= 100;
	client->pers.max_health		= 100;

	client->pers.connected = true;
}


void InitClientResp (gclient_t *client)
{
	memset (&client->resp, 0, sizeof(client->resp));
	client->resp.enterframe = level.framenum;
	client->resp.coop_respawn = client->pers;
}

/*
==================
SaveClientData

Some information that should be persistant, like health, 
is still stored in the edict structure, so it needs to
be mirrored out to the client structure before all the
edicts are wiped.
==================
*/
void SaveClientData (void)
{
	int		i;
	edict_t	*ent;

	for (i=0 ; i<game.maxclients ; i++)
	{
		ent = &g_edicts[1+i];
		if (!ent->inuse)
			continue;
		game.clients[i].pers.health = ent->health;
		game.clients[i].pers.max_health = ent->max_health;
		game.clients[i].pers.savedFlags = 0;
//		if (coop->value)
//			game.clients[i].pers.score = ent->client->resp.score;
	}
}

void FetchClientEntData (edict_t *ent)
{
	ent->health = ent->client->pers.health;
	ent->max_health = ent->client->pers.max_health;
	ent->flags |= ent->client->pers.savedFlags;
//	if (coop->value)
//		ent->client->resp.score = ent->client->pers.score;
}



/*
=======================================================================

  SelectSpawnPoint

=======================================================================
*/

/*
================
PlayersRangeFromSpot

Returns the distance to the nearest player from the given spot
================
*/
float	PlayersRangeFromSpot (edict_t *spot)
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

		VectorSubtract (spot->s.origin, player->s.origin, v);
		playerdistance = VectorLength (v);

		if (playerdistance < bestplayerdistance)
			bestplayerdistance = playerdistance;
	}

	return bestplayerdistance;
}

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point, but NOT the two points closest
to other players
================
*/
edict_t *SelectRandomDeathmatchSpawnPoint (void)
{
	edict_t	*spot, *spot1, *spot2;
	int		count = 0;
	int		selection;
	float	range, range1, range2;

	spot = NULL;
	range1 = range2 = 99999;
	spot1 = spot2 = NULL;

	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		count++;
		range = PlayersRangeFromSpot(spot);
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
		spot = G_Find (spot, FOFS(classname), "info_player_deathmatch");
		if (spot == spot1 || spot == spot2)
			selection++;
	} while(selection--);

	return spot;
}

/*
================
SelectFarthestDeathmatchSpawnPoint

================
*/
edict_t *SelectFarthestDeathmatchSpawnPoint (void)
{
	edict_t	*bestspot;
	float	bestdistance, bestplayerdistance;
	edict_t	*spot;


	spot = NULL;
	bestspot = NULL;
	bestdistance = 0;
	while ((spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) != NULL)
	{
		bestplayerdistance = PlayersRangeFromSpot (spot);

		if (bestplayerdistance > bestdistance)
		{
			bestspot = spot;
			bestdistance = bestplayerdistance;
		}
	}

	if (bestspot)
	{
		return bestspot;
	}

	// if there is a player just spawned on each and every start spot
	// we have no choice to turn one into a telefrag meltdown
	spot = G_Find (NULL, FOFS(classname), "info_player_deathmatch");

	return spot;
}

edict_t *SelectDeathmatchSpawnPoint (void)
{
	if ( (int)(dmflags->value) & DF_SPAWN_FARTHEST)
		return SelectFarthestDeathmatchSpawnPoint ();
	else
		return SelectRandomDeathmatchSpawnPoint ();
}


/*edict_t *SelectCoopSpawnPoint (edict_t *ent)
{
	int		index;
	edict_t	*spot = NULL;
	char	*target;

	index = ent->client - game.clients;

	// player 0 starts in normal player spawn point
	if (!index)
		return NULL;

	spot = NULL;

	// assume there are four coop spots at each spawnpoint
	while (1)
	{
		spot = G_Find (spot, FOFS(classname), "info_player_coop");
		if (!spot)
			return NULL;	// we didn't have enough...

		target = spot->targetname;
		if (!target)
			target = "";
		if ( Q_stricmp(game.spawnpoint, target) == 0 )
		{	// this is a coop spawn point for one of the clients here
			index--;
			if (!index)
				return spot;		// this is it
		}
	}


	return spot;
}
*/

/*
===========
SelectSpawnPoint

Chooses a player start, deathmatch start, coop start, etc
============
*/
/*void	SelectSpawnPoint (edict_t *ent, vec3_t origin, vec3_t angles)
{
	edict_t	*spot = NULL;

	if (deathmatch->value)
		spot = SelectDeathmatchSpawnPoint ();

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
}
*/
//======================================================================


void InitBodyQue (void)
{
	int		i;
	edict_t	*ent;

	level.body_que = 0;
	for (i=0; i<BODY_QUEUE_SIZE ; i++)
	{
		ent = G_Spawn();
		ent->classname = "bodyque";
	}
}

void body_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	if (self->health < -40)
	{
		gi.sound (self, CHAN_BODY, gi.soundindex ("misc/udeath.wav"), 1, ATTN_NORM, 0);
		self->s.origin[2] -= 48;
		self->takedamage = DAMAGE_NO;
	}
}

void CopyToBodyQue (edict_t *ent)
{
	edict_t		*body;

	// grab a body que and cycle to the next one
	body = &g_edicts[(int)maxclients->value + level.body_que + 1];
	level.body_que = (level.body_que + 1) % BODY_QUEUE_SIZE;

	// FIXME: send an effect on the removed body

	gi.unlinkentity (ent);

	gi.unlinkentity (body);
	body->s = ent->s;
	body->s.number = body - g_edicts;

	body->svflags = ent->svflags;
	VectorCopy (ent->mins, body->mins);
	VectorCopy (ent->maxs, body->maxs);
	VectorCopy (ent->absmin, body->absmin);
	VectorCopy (ent->absmax, body->absmax);
	VectorCopy (ent->size, body->size);
	body->solid = ent->solid;
	body->clipmask = ent->clipmask;
	body->owner = ent->owner;
	body->movetype = ent->movetype;

	body->die = body_die;
	body->takedamage = DAMAGE_YES;

	gi.linkentity (body);
}


void respawn (edict_t *self)
{
	if (deathmatch->value || coop->value)
	{
//		CopyToBodyQue (self);
		self->svflags &= ~SVF_NOCLIENT;
		PutClientInServer (self);

		// hold in place briefly
		self->client->ps.pmove.pm_flags = PMF_TIME_TELEPORT;
		self->client->ps.pmove.pm_time = 14;

		self->client->respawn_time = level.time;

		return;
	}

	// restart the entire server
	gi.AddCommandString ("menu_loadgame\n");
}

//==============================================================


/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void PutClientInServer (edict_t *ent)
{
	vec3_t	mins = {-16, -16, -24};
	vec3_t	maxs = {16, 16, 32};
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	client_persistant_t	saved;
	client_respawn_t	resp;
	int		oldflags = ent->client->FLAGS,
			oldteam = ent->client->team,
			oldcls = ent->client->cls,
			oldcat = ent->client->cat,
			oldveh = ent->client->veh,
			oldsel = ent->client->sel_step;

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	FindCameraPoint( ent, spawn_origin, spawn_angles);

	index = ent-g_edicts-1;
	client = ent->client;

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
		char		userinfo[MAX_INFO_STRING];

		resp = client->resp;
		memcpy (userinfo, client->pers.userinfo, sizeof(userinfo));

		resp.coop_respawn.game_helpchanged = client->pers.game_helpchanged;
		resp.coop_respawn.helpchanged = client->pers.helpchanged;
		client->pers = resp.coop_respawn;
		ClientUserinfoChanged (ent, userinfo);
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
	ent->takedamage = DAMAGE_NO;
	ent->movetype = MOVETYPE_NONE;
	ent->inuse = true;
	ent->classname = "camera";
	ent->mass = 0;
	ent->solid = SOLID_NOT;
	ent->deadflag = DEAD_NO;
	ent->clipmask = 0;//MASK_PLAYERSOLID;
	ent->model = NULL;//"players/male/tris.md2";
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags &= ~FL_NO_KNOCKBACK;
	ent->svflags &= ~SVF_DEADMONSTER;
	ent->gravity = 0;

	VectorCopy (mins, ent->mins);
	VectorCopy (maxs, ent->maxs);
	VectorClear (ent->velocity);

	// clear playerstate values
	memset (&ent->client->ps, 0, sizeof(client->ps));

	client->ps.pmove.origin[0] = spawn_origin[0]*8;
	client->ps.pmove.origin[1] = spawn_origin[1]*8;
	client->ps.pmove.origin[2] = spawn_origin[2]*8;

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

	// clear entity state values
	ent->s.effects = 0;
	ent->s.modelindex = 0;//255;		// will use the skin specified model
	ent->s.modelindex2 = 0;//255;		// custom gun model
	// sknum is player num and weapon number
	ent->s.skinnum = ent - g_edicts - 1;

	ent->s.frame = 0;
	VectorCopy (spawn_origin, ent->s.origin);
	ent->s.origin[2] += 1;	// make sure off ground
	VectorCopy (ent->s.origin, ent->s.old_origin);

	// set the delta angle
	for (i=0 ; i<3 ; i++)
	{
		client->ps.pmove.delta_angles[i] = ANGLE2SHORT(spawn_angles[i] - client->resp.cmd_angles[i]);
	}

	ent->s.angles[PITCH] = 0;
	ent->s.angles[YAW] = spawn_angles[YAW];
	ent->s.angles[ROLL] = 0;
	VectorCopy (ent->s.angles, client->ps.viewangles);
	VectorCopy (ent->s.angles, client->v_angle);

	if (!KillBox (ent))
	{	// could't spawn in?
	}

	gi.linkentity (ent);

	// important data for AirQuake2
	ent->client->team = oldteam;
	ent->client->cat = oldcat;
	ent->client->cls = oldcls;
	ent->client->veh = oldveh;
	ent->client->sel_step = oldsel;
	if( !ent->client->team )
		ent->client->team = TEAM_1;
	if( !ent->client->sel_step )
	{
		if( teamplay->value )
			ent->client->sel_step = SEL_TEAM;
	}
	if( !ent->client->cat )
		ent->client->cat = CAT_PLANE;
	ent->client->selectvehicle = true;
	ent->client->showscores = ent->client->showinventory = false;
	ent->client->FLAGS = oldflags;
	ent->client->FLAGS &= ~CLIENT_NO_REPEATBRAKE;
	ent->delay = level.time + 0.5;
}

/*
=====================
ClientBeginDeathmatch

A client has just connected to the server in 
deathmatch mode, so clear everything out before starting them.
=====================
*/
void ClientBeginDeathmatch (edict_t *ent)
{
	G_InitEdict (ent);

	InitClientResp (ent->client);

	// locate ent at a spawn point
	PutClientInServer (ent);

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}

	gi.bprintf (PRINT_HIGH, "%s entered the game\n", ent->client->pers.netname);

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}


/*
===========
ClientBegin

called when a client has finished connecting, and is ready
to be placed into the game.  This will happen every level load.
============
*/
void ClientBegin (edict_t *ent)
{
	int		i;

	ent->client = game.clients + (ent - g_edicts - 1);

	if (deathmatch->value)
	{
		ClientBeginDeathmatch (ent);
		return;
	}

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == true)
	{
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		for (i=0 ; i<3 ; i++)
			ent->client->ps.pmove.delta_angles[i] = ANGLE2SHORT(ent->client->ps.viewangles[i]);
	}
	else
	{
		// a spawn point will completely reinitialize the entity
		// except for the persistant data that was initialized at
		// ClientConnect() time
		G_InitEdict (ent);
		ent->classname = "player";
		InitClientResp (ent->client);
		PutClientInServer (ent);
	}

	if (level.intermissiontime)
	{
		MoveClientToIntermission (ent);
	}

	// make sure all view stuff is valid
	ClientEndServerFrame (ent);
}

/*
===========
ClientUserInfoChanged

called whenever the player updates a userinfo variable.

The game can override any of the settings in place
(forcing skins or names, etc) before copying it off.
============
*/
void ClientUserinfoChanged (edict_t *ent, char *userinfo)
{
	char	*s;
	int		playernum;

	// check for malformed or illegal info strings
	if (!Info_Validate(userinfo))
	{
		strcpy (userinfo, "\\name\\badinfo\\skin\\male/grunt");
	}

	// set name
	s = Info_ValueForKey (userinfo, "name");
	strncpy (ent->client->pers.netname, s, sizeof(ent->client->pers.netname)-1);

	// set skin
	s = Info_ValueForKey (userinfo, "skin");

	playernum = ent-g_edicts-1;

	// combine name and skin into a configstring
	gi.configstring (CS_PLAYERSKINS+playernum, va("%s\\%s", ent->client->pers.netname, s) );

	// fov
	if (deathmatch->value && ((int)dmflags->value & DF_FIXED_FOV))
	{
		ent->client->ps.fov = 90;
	}
	else
	{
		ent->client->ps.fov = atoi(Info_ValueForKey(userinfo, "fov"));
		if (ent->client->ps.fov < 1)
			ent->client->ps.fov = 90;
		else if (ent->client->ps.fov > 160)
			ent->client->ps.fov = 160;
	}

	// handedness
	s = Info_ValueForKey (userinfo, "hand");
	if (strlen(s))
	{
		ent->client->pers.hand = atoi(s);
	}

	// save off the userinfo in case we want to check something later
	strncpy (ent->client->pers.userinfo, userinfo, sizeof(ent->client->pers.userinfo)-1);
}


/*
===========
ClientConnect

Called when a player begins connecting to the server.
The game can refuse entrance to a client by returning false.
If the client is allowed, the connection process will continue
and eventually get to ClientBegin()
Changing levels will NOT cause this to be called again, but
loadgames will.
============
*/
qboolean ClientConnect (edict_t *ent, char *userinfo)
{
	char	*value;

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if (SV_FilterPacket(value)) 
	{
		Info_SetValueForKey(userinfo, "rejmsg", "Banned.");
		return false;
	}

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if (*password->string && strcmp(password->string, "none") && 
		strcmp(password->string, value)) 
	{
		Info_SetValueForKey(userinfo, "rejmsg", "Password required or incorrect.");
			return false;
	}


	// they can connect
	ent->client = game.clients + (ent - g_edicts - 1);

	// if there is already a body waiting for us (a loadgame), just
	// take it, otherwise spawn one from scratch
	if (ent->inuse == false)
	{
		// clear the respawning variables
		InitClientResp (ent->client);
//		if (!game.autosaved || !ent->client->pers.weapon)
			InitClientPersistant (ent->client);
	}

	ClientUserinfoChanged (ent, userinfo);

	if (game.maxclients > 1)
		gi.dprintf ("%s connected\n", ent->client->pers.netname);

	ent->svflags = 0; // make sure we start with known default
	ent->client->pers.connected = true;
	// important data for AirQuake2
	ent->delay = level.time + 0.5;	// for selection
	ent->client->sel_step = SEL_TEAM;
	ent->client->FLAGS = CLIENT_COORD;
	if( strcmp( vid_ref->string, "soft" ) == 0 )
		ent->client->FLAGS |= CLIENT_NONGL_MODE;
	ent->client->team = TEAM_1;
	ent->client->cat = CAT_PLANE;
	ent->client->thrusttime = level.time;
	ent->client->selectvehicle = true;
	return true;
}

/*
===========
ClientDisconnect

Called when a player drops from the server.
Will not be called between levels.
============
*/
void ClientDisconnect (edict_t *ent)
{
	int		playernum;

	if (!ent->client)
		return;

	gi.bprintf (PRINT_HIGH, "%s disconnected\n", ent->client->pers.netname);

	if( ent->client->cameratarget )
	{
		RemoveCamera( ent );
	}

	if( ent->client->FLAGS & CLIENT_VEH_SELECTED )
	{
		RemoveVehicle( ent );
	}

	if( teamplay->value )
	{
		teams[ent->client->team].players--;
	}

	gi.unlinkentity (ent);
	ent->s.modelindex = 0;
	ent->solid = SOLID_NOT;
	ent->inuse = false;
	ent->classname = "disconnected";
	ent->client->pers.connected = false;

	playernum = ent-g_edicts-1;
	gi.configstring (CS_PLAYERSKINS+playernum, "");
}


//==============================================================


edict_t	*pm_passent;

// pmove doesn't need to know about passent and contentmask
trace_t	PM_trace (vec3_t start, vec3_t mins, vec3_t maxs, vec3_t end)
{
	if (pm_passent->health > 0)
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_PLAYERSOLID);
	else
		return gi.trace (start, mins, maxs, end, pm_passent, MASK_DEADSOLID);
}

unsigned CheckBlock (void *b, int c)
{
	int	v,i;
	v = 0;
	for (i=0 ; i<c ; i++)
		v+= ((byte *)b)[i];
	return v;
}
void PrintPmove (pmove_t *pm)
{
	unsigned	c1, c2;

	c1 = CheckBlock (&pm->s, sizeof(pm->s));
	c2 = CheckBlock (&pm->cmd, sizeof(pm->cmd));
	Com_Printf ("sv %3i:%i %i\n", pm->cmd.impulse, c1, c2);
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame.
==============
*/
void ClientThink (edict_t *ent, usercmd_t *ucmd)
{
	gclient_t	*client;
//	edict_t	*other;
	int		i;//, j;
	pmove_t	pm;

//	gi.bprintf( PRINT_HIGH, "%d %d %d %d\n", ent->client->sel_step, ent->client->cat, ent->client->cls,
//		ent->client->veh );

	ent->client->ps.pmove.pm_flags = PMF_NO_PREDICTION;

	level.current_entity = ent;
	client = ent->client;

	if (level.intermissiontime)
	{
		client->ps.pmove.pm_type = PM_FREEZE;
		// can exit intermission after five seconds
		if (level.time > level.intermissiontime + 5.0 
			&& (ucmd->buttons & BUTTON_ANY) )
			level.exitintermission = true;
		return;
	}

	if( !(ucmd->buttons & BUTTON_ANY) )
	{
		// saftey code of repeatable buttons goes here
//		gi.bprintf( PRINT_HIGH, "%d\n", ucmd->buttons );// if ucmd = 0 then end all cmds!
		ent->client->REPEAT = 0;
	}

	if( ent->client->FLAGS & CLIENT_VEH_SELECTED )
	{
		ent->health = ent->vehicle->health*100/ent->vehicle->max_health;
		ent->deadflag = ent->vehicle->deadflag;

//		ent->client->resp.score = ent->vehicle->speed;
//		gi.bprintf( PRINT_HIGH, "%.1f %.1f %.1f\n", ent->vehicle->maxs[0], 
//			ent->vehicle->maxs[1], ent->vehicle->maxs[2] );
	}

	if( (!ent->client->cameratarget || !ent->client->cameratarget->inuse) 
		&& (ent->client->FLAGS & CLIENT_VEH_SELECTED) )
	{
		ent->client->cameratarget = ent->vehicle;
		if( !ent->vehicle->camera )
			ent->vehicle->camera = ent;
		else if( ent->vehicle->camera && ent->vehicle->camera != ent )
		{
			ent->client->prevcam = NULL;
			ent->client->nextcam = ent->vehicle->camera;
			ent->vehicle->camera->client->prevcam = ent;
			ent->vehicle->camera = ent;
		}
	}

	if( ent->deadflag == DEAD_DEAD )
	{
		if( ent->client->FLAGS & CLIENT_VEH_SELECTED )
		{
			gi.sound( ent->vehicle, CHAN_BODY, gi.soundindex( "death/0.wav" ), 1, ATTN_NORM, 0 );
//			SetVehicleModel( ent->vehicle );
		}
		if( (ent->client->buttons & BUTTON_ATTACK) && level.time > ent->client->respawn_time )
		{
			RemoveVehicle( ent );
			RemoveCamera( ent );
			respawn( ent );
			return;
		}
	}

	if( ent->deadflag != DEAD_DEAD && (ent->client->FLAGS & CLIENT_VEH_SELECTED) )
	{
		if( ucmd->forwardmove > 400 )
			ucmd->forwardmove = 400;
		else if( ucmd->forwardmove < -400 )
			ucmd->forwardmove = -400;
		if( ucmd->upmove > 400 )
			ucmd->upmove = 400;
		else if( ucmd->upmove < -400 )
			ucmd->upmove = -400;
		if( ucmd->sidemove > 400 )
			ucmd->sidemove = 400;
		else if( ucmd->sidemove < -400 )
			ucmd->sidemove = -400;

		if( !(ent->client->REPEAT & REPEAT_FREELOOK) )
		{
			ent->vehicle->elevator = (float)(ucmd->forwardmove);//*ent->vehicle->turnspeed[0]/400);
			if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
			{
				ent->vehicle->rudder = -(float)(ucmd->sidemove);//*ent->vehicle->turnspeed[2]/400);
				ent->vehicle->aileron = (float)(ucmd->upmove);//*ent->vehicle->turnspeed[1]/400);
			}
			else
			{
				ent->vehicle->aileron = -(float)(ucmd->sidemove);//*ent->vehicle->turnspeed[2]/400);
				ent->vehicle->rudder = (float)(ucmd->upmove);//*ent->vehicle->turnspeed[1]/400);
			}
			ent->client->cameraposition[0] = ent->vehicle->s.angles[0];
			ent->client->cameraposition[1] = ent->vehicle->s.angles[1];
			ent->client->cameraposition[2] = 0;

			// speed dependent maneuverability for planes
/*			if( strcmp( ent->vehicle->classname, "plane" ) == 0 && ent->vehicle->speed > ent->vehicle->stallspeed )
			{
				ent->vehicle->elevator *= (float)(ent->vehicle->stallspeed/(ent->vehicle->speed*1.2));
				if( ent->vehicle->ONOFF & ONOFF_AIRBORNE )
					ent->vehicle->rudder *= (float)(ent->vehicle->stallspeed/ent->vehicle->speed);
			}*/	
		//	gi.bprintf( PRINT_HIGH, "%.1f: %d %d %d\n", plane->speed, plane->elevator, plane->rudder,
//			plane->aileron );
		// adjust control sensitivity on zoom factor
			if( ent->client->fov == 60 )
			{
				ent->vehicle->elevator /= 2;
				ent->vehicle->aileron /= 2;
				ent->vehicle->rudder /= 2;
			}
			else if( ent->client->fov == 30 )
			{
				ent->vehicle->elevator /= 4;
				ent->vehicle->aileron /= 4;
				ent->vehicle->rudder /= 4;
			}
			else if( ent->client->fov == 10 )
			{
				ent->vehicle->elevator /= 6;
				ent->vehicle->aileron /= 6;
				ent->vehicle->rudder /= 6;
			}
		}
		else
		{
			ent->client->cameraposition[0] += ucmd->forwardmove/80;
			ent->client->cameraposition[1] += ucmd->sidemove/80;
			ent->client->cameraposition[2] = (ent->vehicle->s.angles[0] > 90 && 
				ent->vehicle->s.angles[0] < 270) ? 180 : 0;//+= ucmd->upmove/80;
			ent->vehicle->elevator = 0;
			ent->vehicle->aileron = 0;
			ent->vehicle->rudder = 0;
		}
		if( !(ent->client->FLAGS & CLIENT_NO_REPEATBRAKE ) && !(ent->vehicle->DAMAGE & DAMAGE_BRAKE) &&
			((ent->vehicle->HASCAN & HASCAN_BRAKE) || strcmp(ent->vehicle->classname, "LQM") == 0) )
		{
			if( ent->client->REPEAT & REPEAT_BRAKE ) {
				ent->vehicle->ONOFF |= ONOFF_BRAKE;
			}
			else
				ent->vehicle->ONOFF &= ~ONOFF_BRAKE;
		}
		if( ent->client->REPEAT & REPEAT_FIRE )
			Cmd_Weapon( ent );
	}

	if( (ent->client->REPEAT & REPEAT_INCREASE) && level.time > ent->client->thrusttime )
	{
		Cmd_Increase( ent );
	}
	else if( (ent->client->REPEAT & REPEAT_DECREASE) && level.time > ent->client->thrusttime )
	{
		Cmd_Decrease( ent );
	}
	if( (ent->client->buttons & BUTTON_ATTACK) &&
		!(ent->client->FLAGS & CLIENT_VEH_SELECTED) &&
		ent->client->selectvehicle )
	{
		if( level.time > ent->delay )
		{
			SelectionStep( ent );
			ent->delay = level.time + 0.4;
		}
	}

	// former pos of intermission

	pm_passent = ent;
   
	// set up for pmove
	memset (&pm, 0, sizeof(pm));

	client->ps.pmove.pm_type = PM_DEAD;

	client->ps.pmove.gravity = 0;//sv_gravity->value;
	pm.s = client->ps.pmove;

	for (i=0 ; i<3 ; i++)
	{
		pm.s.origin[i] = ent->s.origin[i]*8;
		pm.s.velocity[i] = ent->velocity[i]*8;
	}

	if (memcmp(&client->old_pmove, &pm.s, sizeof(pm.s)))
	{
		pm.snapinitial = true;
//		gi.dprintf ("pmove changed!\n");
	}

	pm.cmd = *ucmd;

	pm.trace = PM_trace;	// adds default parms
	pm.pointcontents = gi.pointcontents;

	// perform a pmove
	gi.Pmove (&pm);

	// save results of pmove
	client->ps.pmove = pm.s;
	client->old_pmove = pm.s;

	for (i=0 ; i<3 ; i++)
	{
		ent->s.origin[i] = pm.s.origin[i]*0.125;
		ent->velocity[i] = pm.s.velocity[i]*0.125;
	}

	VectorCopy (pm.mins, ent->mins);
	VectorCopy (pm.maxs, ent->maxs);

	client->resp.cmd_angles[0] = SHORT2ANGLE(ucmd->angles[0]);
	client->resp.cmd_angles[1] = SHORT2ANGLE(ucmd->angles[1]);
	client->resp.cmd_angles[2] = SHORT2ANGLE(ucmd->angles[2]);

//	ent->viewheight = pm.viewheight;
	ent->waterlevel = pm.waterlevel;
	ent->watertype = pm.watertype;
	ent->groundentity = pm.groundentity;
	if (pm.groundentity)
		ent->groundentity_linkcount = pm.groundentity->linkcount;

	gi.linkentity (ent);

	client->oldbuttons = client->buttons;
	client->buttons = ucmd->buttons;
	client->latched_buttons |= client->buttons & ~client->oldbuttons;
}


/*
==============
ClientBeginServerFrame

This will be called once for each server frame, before running
any other entities in the world.
==============
*/
void ClientBeginServerFrame (edict_t *ent)
{
	gclient_t	*client;
	int			buttonMask;

	if (level.intermissiontime)
		return;


	client = ent->client;

	if (ent->deadflag)
	{
		// wait for any button just going down
		if ( level.time > client->respawn_time)
		{
			// in deathmatch, only wait for attack button
			if (deathmatch->value)
				buttonMask = BUTTON_ATTACK;
			else
				buttonMask = -1;

		}
		return;
	}

	client->latched_buttons = 0;
}
