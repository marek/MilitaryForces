/*
 * $Id: mf_client.c,v 1.26 2005-06-26 05:08:12 minkis Exp $
*/

#include "g_local.h"


/*
===========
MF_ClientConnect

Called when a player begins connecting to the server.
Called again for every map change or tournement restart.

The session information will be valid after exit.

Return NULL if the client should be allowed, otherwise return
a string with the reason for denial.

Otherwise, the client will be sent the current gamestate
and will eventually get to ClientBegin.

firstTime will be qtrue the very first time a client connects
to the server machine, but qfalse on map changes and tournement
restarts.
============
*/
char *MF_ClientConnect( int clientNum, qboolean firstTime, qboolean isBot ) {
	char		*value;
	gclient_t	*client;
	char		userinfo[MAX_INFO_STRING];
	gentity_t	*ent;

	if( isBot ) {
		return "MFQ3 doesn't have bots yet";
	}

	if( g_gametype.integer == GT_MISSION_EDITOR && level.numConnectedClients >= 1 ) {
		return "MFQ3 IGME in use. Connecting not allowed.";
	}

	ent = &g_entities[ clientNum ];

	trap_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check to see if they are on the banned IP list
	value = Info_ValueForKey (userinfo, "ip");
	if ( G_FilterPacket( value ) ) {
		return "Banned.";
	}

	// check for a password
	value = Info_ValueForKey (userinfo, "password");
	if ( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
		strcmp( g_password.string, value) != 0) {
		return "Invalid password";
	}

	// they can connect
	ent->client = level.clients + clientNum;
	client = ent->client;

	memset( client, 0, sizeof(*client) );

	client->pers.connected = CON_CONNECTING;

	// read or initialize the session data
	if ( firstTime || level.newSession ) {
		G_InitSessionData( client, userinfo );
	}
	G_ReadSessionData( client );

	// for later
	if( isBot ) {
		ent->r.svFlags |= SVF_BOT;
		ent->inuse = qtrue;
//		if( !G_BotConnect( clientNum, !firstTime ) ) {
			return "BotConnectfailed";
//		}
	}

	// init vehicle
	Info_SetValueForKey( userinfo, "cg_vehicle", "-1" );
	Info_SetValueForKey( userinfo, "cg_nextVehicle", "-1" );
	trap_SetUserinfo( clientNum, userinfo );

	// get and distribute relevent paramters
	G_LogPrintf( "MF_ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) {
		trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers.netname) );
	}

	if ( g_gametype.integer >= GT_TEAM &&
		client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BroadcastTeamChange( client, -1 );
	}

	// count current clients and rank for scoreboard
	CalculateRanks();

	return NULL;
}


/*
===========
MF_ClientBegin

called when a client has finished connecting, and is ready
to be placed into the level.  This will happen every level load,
and on transition between teams, but doesn't happen on respawns

Special Version for MFQ3
============
*/
void MF_ClientBegin( int clientNum ) {
	gentity_t	*ent;
	gclient_t	*client;
	int			flags;

	ent = g_entities + clientNum;

	client = level.clients + clientNum;

	if ( ent->r.linked ) {
		trap_UnlinkEntity( ent );
	}
	G_InitGentity( ent );
	ent->client = client;

	client->pers.connected = CON_CONNECTED;
	client->pers.enterTime = level.time;
	client->pers.teamState.state = TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the new position
	flags = client->ps.eFlags;
	memset( &client->ps, 0, sizeof( client->ps ) );
	client->ps.eFlags = flags;

	// MFQ3 prepare
	client->vehicle = client->nextVehicle = -1;

	// locate ent at a spawn point
	MF_ClientSpawn( ent, 0 );

	if ( client->sess.sessionTeam != TEAM_SPECTATOR ) {
		if ( g_gametype.integer != GT_TOURNAMENT  ) {
			trap_SendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers.netname) );
		}
	}
	G_LogPrintf( "ClientBegin: %i\n", clientNum );

	// count current clients and rank for scoreboard
	CalculateRanks();
}


/*
===========
MF_ClientSpawn

Called every time a client is placed fresh in the world:
after the first ClientBegin, and after each respawn
Initializes all non-persistant parts of playerState
============
*/
gentity_t *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles );
gentity_t *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles );


void MF_ClientSpawn(gentity_t *ent, long cs_flags) {
	int		index;
	vec3_t	spawn_origin, spawn_angles;
	gclient_t	*client;
	int		i;
	clientPersistant_t	saved;
	clientSession_t		savedSess;
	int		persistant[MAX_PERSISTANT];
	gentity_t	*spawnPoint;
	int		flags;
	int		savedPing;
//	char	*savedAreaBits;
	int		accuracy_hits, accuracy_shots;
	int		savedEvents[MAX_PS_EVENTS];
	int		eventSequence;
	char	userinfo[MAX_INFO_STRING];
	int		vehIndex;
	int		savedNext;
	int		vehCurrent;
	int		handicap;
	int		hasNuke = 0;
	int		newLoadoutForced = 0;
	int		cc = 0;
	int		dd = 0;

	index = ent - g_entities;
	client = ent->client;

	trap_GetUserinfo( index, userinfo, sizeof(userinfo) );
	// vehicle index for next spawning
	vehIndex = atoi( Info_ValueForKey( userinfo, "cg_nextVehicle" ) );

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	if(cs_flags & CS_LASTPOS) {
		// use current position as spawn point
		VectorCopy(client->ps.origin, spawn_origin);
		VectorCopy(client->ps.vehicleAngles, spawn_angles);
	} else {
		if ( client->sess.sessionTeam == TEAM_SPECTATOR || (client->sess.sessionTeam != TEAM_SPECTATOR &&
			(vehIndex < 0 || vehIndex >= bg_numberOfVehicles)) ) {
			spawnPoint = SelectSpectatorSpawnPoint ( 
							spawn_origin, spawn_angles);
		} else if (g_gametype.integer >= GT_TEAM ) {
			// all base oriented team games use the CTF spawn points
			spawnPoint = SelectCTFSpawnPoint ( 
							client->sess.sessionTeam, 
							client->pers.teamState.state, 
							spawn_origin, spawn_angles, vehIndex);
		} else {
			do {
				// the first spawn should be at a good looking spot
				if ( !client->pers.initialSpawn && client->pers.localClient ) {
					client->pers.initialSpawn = qtrue;
					spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles );
				} else {
					// don't spawn near existing origin if possible
					spawnPoint = SelectSpawnPoint ( 
						client->ps.origin, 
						spawn_origin, spawn_angles,
						availableVehicles[vehIndex].cat);
				}
	
				// Tim needs to prevent bots from spawning at the initial point
				// on q3dm0...
				if ( ( spawnPoint->flags & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) ) {
					continue;	// try again
				}
				// just to be symetric, we have a nohumans option...
				if ( ( spawnPoint->flags & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) {
					continue;	// try again
				}

				// check for category on this spawn point
				if( !(spawnPoint->ent_category & availableVehicles[vehIndex].cat) ) {
					continue;
				}
				
				break;

			} while ( 1 );
		}
	}

	// Do initial vehicle checks
	// if respawning in current spot, and want to be a boat, check for water!
	if(cs_flags & CS_LASTPOS && availableVehicles[vehIndex].cat & CAT_BOAT ) {
		trace_t	trace;
		vec3_t	endpos;
		VectorCopy( spawn_origin, endpos );
		endpos[2] -= 512;
		trap_Trace (&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_WATER );
		if((cs_flags & CS_LASTPOS) && trace.entityNum != ENTITYNUM_NONE ) { // Check for water if respawning in current location
			return;							// Fuck that! dont do shit ;p
		}
	}

	// If nukes are banned, disallow loadouts with any nukebombs or nukemissiles
	if(mf_allowNukes.integer == 0 && vehIndex >= 0)
	{
		for(cc = 0; cc < MAX_WEAPONS_PER_VEHICLE; cc++)
		{
			if( availableWeapons[availableVehicles[vehIndex].weapons[cc]].type == WT_NUKEBOMB || 
				availableWeapons[availableVehicles[vehIndex].weapons[cc]].type == WT_NUKEMISSILE ) {
				hasNuke = 1;
				break;
			}
		}
	} 
	// Now find another appropriate vehicle instead
	if(hasNuke)
	{
		for(cc = 0; cc < bg_numberOfVehicles; ++cc)
		{
			for(dd = 0; dd < MAX_WEAPONS_PER_VEHICLE; dd++)
			{
				if( availableWeapons[availableVehicles[cc].weapons[dd]].type != WT_NUKEBOMB && 
					availableWeapons[availableVehicles[cc].weapons[dd]].type != WT_NUKEMISSILE && 
					availableVehicles[cc].gameset == availableVehicles[vehIndex].gameset) {
					newLoadoutForced = 1;
					client->nextVehicle = vehIndex = cc;
					break;
				}
						
			}
			if(newLoadoutForced)
				break;
		}
	}


	client->pers.teamState.state = TEAM_ACTIVE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client->ps.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// save veh
	savedNext = client->nextVehicle;
	vehCurrent = client->vehicle;

	// clear everything but the persistant data
	saved = client->pers;
	savedSess = client->sess;
	savedPing = client->ps.ping;
	accuracy_hits = client->accuracy_hits;
	accuracy_shots = client->accuracy_shots;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		persistant[i] = client->ps.persistant[i];
	}
	// also save the predictable events otherwise we might get double or dropped events
	for (i = 0; i < MAX_PS_EVENTS; i++) {
		savedEvents[i] = client->ps.events[i];
	}
	eventSequence = client->ps.eventSequence;

	memset (client, 0, sizeof(*client));

	client->pers = saved;
	client->sess = savedSess;
	client->ps.ping = savedPing;
	client->accuracy_hits = accuracy_hits;
	client->accuracy_shots = accuracy_shots;
	client->lastkilled_client = -1;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) {
		client->ps.persistant[i] = persistant[i];
	}
	for (i = 0; i < MAX_PS_EVENTS; i++) {
		client->ps.events[i] = savedEvents[i];
	}
	client->ps.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps.persistant[PERS_SPAWN_COUNT]++;
	client->ps.persistant[PERS_TEAM] = client->sess.sessionTeam;
	client->ps.clientNum = index;

	client->airOutTime = level.time + 25000;

	client->ps.eFlags = flags;

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->client = &level.clients[index];
	ent->takedamage = qtrue;
	ent->inuse = qtrue;
	ent->classname = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask = MASK_PLAYERSOLID;
	ent->die = Vehicle_Death;
	ent->waterlevel = 0;
	ent->watertype = 0;
	ent->flags = 0;
	ent->left = qfalse; //first projectile starts right
	ent->bulletpos = 0;

	// distribute the weapons
	MF_getDefaultLoadoutForVehicle( vehIndex, &ent->loadout );
	//	G_AddEvent( ent, EV_GET_DEFAULT_LOADOUT, 0 );
	ent->loadoutUpdated = qfalse;

	Info_SetValueForKey( userinfo, "cg_vehicle", va( "%d", vehIndex ) );
	trap_SetUserinfo( index, userinfo );
	ClientUserinfoChanged( index );

	// cat specific stuff
	if( availableVehicles[vehIndex].cat & CAT_PLANE ) {
		// check for landed spawnpoint
		qboolean landed = qfalse;
		trace_t	trace;
		vec3_t	endpos;
		gentity_t *test;
		VectorCopy( spawn_origin, endpos );
		endpos[2] -= 128;
		trap_Trace (&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_ALL );
		if( trace.entityNum != ENTITYNUM_NONE ) {
			test = &g_entities[trace.entityNum];
			if( canLandOnIt(test) ) {
				landed = qtrue;
				spawn_origin[2] = test->r.maxs[2] - availableVehicles[vehIndex].mins[2] + 
					availableVehicles[vehIndex].gearheight;
			}			
		}
		MF_Spawn_Plane( ent, vehIndex, landed );
	} else if( availableVehicles[vehIndex].cat & CAT_GROUND ) {
		trace_t	trace;
		vec3_t	endpos;
		gentity_t *test;
		VectorCopy( spawn_origin, endpos );
		endpos[2] -= 512;

		trap_Trace (&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_ALL );
		if( trace.entityNum != ENTITYNUM_NONE ) {
			test = &g_entities[trace.entityNum];
				spawn_origin[2] = trace.endpos[2] - availableVehicles[vehIndex].mins[2] + 1;
		}
		MF_Spawn_GroundVehicle( ent, vehIndex );
	} else if( availableVehicles[vehIndex].cat & CAT_HELO ) {
		MF_Spawn_Helo( ent, vehIndex, qfalse );
	} else if( availableVehicles[vehIndex].cat & CAT_LQM ) {
		MF_Spawn_LQM( ent, vehIndex );
	} else if( availableVehicles[vehIndex].cat & CAT_BOAT ) {
		trace_t	trace;
		vec3_t	endpos;
		gentity_t *test;
		VectorCopy( spawn_origin, endpos );
		endpos[2] -= 512;
		trap_Trace (&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_ALL );
		if( trace.entityNum != ENTITYNUM_NONE ) {
			test = &g_entities[trace.entityNum];
				spawn_origin[2] = trace.endpos[2];
		}
		MF_Spawn_Boat( ent, vehIndex );
	}

	// vehicles only stuff
	if( vehIndex >= 0 && vehIndex < bg_numberOfVehicles ) {
		client->vehicle = client->nextVehicle = savedNext;

		VectorCopy (availableVehicles[vehIndex].mins, ent->r.mins);
		VectorCopy (availableVehicles[vehIndex].maxs, ent->r.maxs);

		// set health
		handicap = atoi( Info_ValueForKey( userinfo, "handicap" ) );
		if ( handicap < 1 || handicap > 100 ) {
			handicap = 100;
		}
		client->pers.maxHealth = availableVehicles[vehIndex].maxhealth * handicap / 100;
		client->ps.stats[STAT_MAX_HEALTH] = client->pers.maxHealth;

		// fuel
		client->ps.stats[STAT_FUEL] = client->ps.stats[STAT_MAX_FUEL] = availableVehicles[vehIndex].maxfuel;
		client->ps.stats[STAT_LOCKINFO] = 0;
		client->ps.timers[TIMER_FUEL] = level.time;

		// which weapons do we have
		for( i = WP_MACHINEGUN; i <= WP_FLARE; i++ ) {
			if( availableVehicles[vehIndex].weapons[i] != WI_NONE ) {
				client->ps.ammo[i] = client->ps.ammo[i+8] = availableVehicles[vehIndex].ammo[i] *
					availableWeapons[availableVehicles[vehIndex].weapons[i]].numberPerPackage;
			}
		}

		// select the first weapon
		for( i = WP_MACHINEGUN; i <= WP_FLARE; i++ ) {
			if( availableVehicles[vehIndex].weapons[i] != WI_NONE ) {
				client->ps.weaponIndex = availableVehicles[vehIndex].weapons[i];
				break;
			}
		}
		if( client->ps.weaponIndex == WI_NONE ) {
			client->ps.weaponIndex = availableVehicles[vehIndex].weapons[0];
		}

	} else { // spectator only stuff
		VectorSet( ent->r.mins, -8, -8, -8 );
		VectorSet( ent->r.maxs, 8, 8, 8 );
		if( client->sess.sessionTeam != TEAM_SPECTATOR ) {
			client->ps.pm_flags |= PMF_VEHICLESELECT;
			client->ps.pm_type = PM_FREEZE;
			client->vehicle = -1;
		}
	}

	// health will count down towards max_health
	ent->health = client->ps.stats[STAT_HEALTH] = client->ps.stats[STAT_MAX_HEALTH] + 25;

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps.pm_flags |= PMF_RESPAWNED;

	trap_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
	SetClientViewAngle( ent, spawn_angles );

	if ( ent->client->sess.sessionTeam == TEAM_SPECTATOR ) {

	} else {
		if(!(cs_flags & CS_NOKILL))
			G_KillBox( ent );
		trap_LinkEntity (ent);
	}

	// reset radar
	ent->client->ps.ONOFF |= ent->client->pers.lastRadar;

	// don't allow full run speed for a bit
	client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps.pm_time = 100;

	client->respawnTime = level.time;
	client->inactivityTime = level.time + g_inactivity.integer * 1000;
	client->latched_buttons = 0;

	// set default animations
	client->ps.vehicleAnim = 0;
	client->ps.gunAngle = 0;
	client->ps.turretAngle = 0;

	if ( level.intermissiontime ) {
		MoveClientToIntermission( ent );
	} else {
		// fire the targets of the spawn point
		G_UseTargets( spawnPoint, ent );
	}

	// reset lockinfo
	unlock( ent );

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps.commandTime = level.time - 100;
	ent->client->pers.cmd.serverTime = level.time;
	ClientThink( ent-g_entities );

	// positively link the client, even if the command times are weird
	if( ent->client->sess.sessionTeam != TEAM_SPECTATOR ) {
		BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
		VectorCopy( ent->client->ps.origin, ent->r.currentOrigin );
		trap_LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps, &ent->s, qtrue );
}
