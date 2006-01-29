/*
 * $Id: mf_client.c,v 1.11 2006-01-29 14:03:41 thebjoern Exp $
*/

#include "g_local.h"
#include "g_level.h"
#include "g_entity.h"
#include "g_vehicle.h"
#include "g_plane.h"
#include "g_helo.h"
#include "g_boat.h"
#include "g_lqm.h"
#include "g_groundvehicle.h"


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

firstTime will be true the very first time a client connects
to the server machine, but false on map changes and tournement
restarts.
============
*/
char *MF_ClientConnect( int clientNum, bool firstTime, bool isBot ) 
{
	if( isBot ) 
		return "MFQ3 doesn't have bots yet";

	if( g_gametype.integer == GT_MISSION_EDITOR && theLevel.numConnectedClients_ >= 1 ) 
		return "MFQ3 IGME in use. Connecting not allowed.";

	char userinfo[MAX_INFO_STRING];
	SV_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );

	// check to see if they are on the banned IP list
	char* value = Info_ValueForKey (userinfo, "ip");
	if ( G_FilterPacket( value ) ) 
		return "Banned.";

	// check for a password
	value = Info_ValueForKey( userinfo, "password" );
	if( g_password.string[0] && Q_stricmp( g_password.string, "none" ) &&
		strcmp( g_password.string, value) != 0)
	{
		return "Invalid password";
	}

	// they can connect
	GameClient	*client = theLevel.createNewClient(clientNum);

	//// vehicle index for next spawning
	//int vehIndex = atoi( Info_ValueForKey( userinfo, "cg_nextVehicle" ) );

	//GameEntity* ent;
	//switch( availableVehicles[vehIndex].cat )
	//{
	//case CAT_PLANE:
	//	ent = new Entity_Plane;
	//	break;
	//case CAT_HELO:
	//	ent = new Entity_Helicopter;
	//	break;
	//case CAT_GROUND:
	//	ent = new Entity_GroundVehicle;
	//	break;
	//case CAT_BOAT:
	//	ent = new Entity_Boat;
	//	break;
	//case CAT_LQM:
	//	ent = new Entity_Infantry;
	//	break;
	//default:
	//	ent = new GameEntity;	// create this for now -> should be fixed later!
	//}
	//theLevel.setPlayerEntity(ent, clientNum);

	//memset( client, 0, sizeof(*client) );

	GameEntity	*ent = theLevel.getEntity(clientNum);//  &g_entities[ clientNum ];
	ent->client_ = theLevel.getClient(clientNum);// level.clients + clientNum;

	client->pers_.connected_ = GameClient::ClientPersistant::CON_CONNECTING;

	// read or initialize the session data
	if ( firstTime || theLevel.newSession_ ) 
		G_InitSessionData( client, userinfo );

	G_ReadSessionData( client );

	// for later
//	if( isBot ) 
//	{
//		ent->r.svFlags |= SVF_BOT;
//		ent->inuse_ = true;
////		if( !G_BotConnect( clientNum, !firstTime ) ) {
//			return "BotConnectfailed";
////		}
//	}

	// init vehicle
	Info_SetValueForKey( userinfo, "cg_vehicle", "-1" );
	Info_SetValueForKey( userinfo, "cg_nextVehicle", "-1" );
	SV_SetUserinfo( clientNum, userinfo );

	// get and distribute relevent paramters
	G_LogPrintf( "MF_ClientConnect: %i\n", clientNum );
	ClientUserinfoChanged( clientNum );

	// don't do the "xxx connected" messages if they were caried over from previous level
	if ( firstTime ) 
		SV_GameSendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " connected\n\"", client->pers_.netname_) );

	if ( g_gametype.integer >= GT_TEAM &&
		client->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
	{
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
void MF_ClientBegin( int clientNum ) 
{
	//GameEntity	*ent;
	GameClient	*client = theLevel.getClient(clientNum);

	//ent = theLevel.getEntity(clientNum);// g_entities + clientNum;

	//if ( ent->r.linked ) 
	//	SV_UnlinkEntity( ent );

	//ent->init(-1);
	//ent->client_ = client;

	client->pers_.connected_ = GameClient::ClientPersistant::CON_CONNECTED;
	client->pers_.enterTime_ = theLevel.time_;
	client->pers_.teamState_.state_ = GameClient::ClientPersistant::PlayerTeamState::TEAM_BEGIN;

	// save eflags around this, because changing teams will
	// cause this to happen with a valid entity, and we
	// want to make sure the teleport bit is set right
	// so the viewpoint doesn't interpolate through the
	// world to the New position
	int flags = client->ps_.eFlags;
	memset( &client->ps_, 0, sizeof( client->ps_ ) );
	client->ps_.eFlags = flags;

	// MFQ3 prepare
	client->vehicle_ = client->nextVehicle_ = -1;

	// locate ent at a spawn point
	MF_ClientSpawn( clientNum, 0 );

	if ( client->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
	{
		if ( g_gametype.integer != GT_TOURNAMENT  ) 
		{
			SV_GameSendServerCommand( -1, va("print \"%s" S_COLOR_WHITE " entered the game\n\"", client->pers_.netname_) );
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
GameEntity *SelectSpectatorSpawnPoint( vec3_t origin, vec3_t angles );
GameEntity *SelectInitialSpawnPoint( vec3_t origin, vec3_t angles );


void MF_ClientSpawn( int clientNum, long cs_flags ) 
{
	vec3_t	spawn_origin, spawn_angles;
	int		i;
	GameClient::ClientPersistant saved;
	GameClient::ClientSession savedSess;
	int		persistant[MAX_PERSISTANT];
	GameEntity	*spawnPoint = 0;
	int		flags;
	int		savedPing;
//	char	*savedAreaBits;
	int		accuracy_hits, accuracy_shots;
	int		savedEvents[MAX_PS_EVENTS];
	int		eventSequence;
	int		savedNext;
	int		handicap;
	int		cc = 0;

	//int index = ent->s.number;// ent - g_entities;
	//client = ent->client_;

	char userinfo[MAX_INFO_STRING];
	SV_GetUserinfo( clientNum, userinfo, sizeof(userinfo) );

	// vehicle index for next spawning
	int vehIndex = atoi( Info_ValueForKey( userinfo, "cg_nextVehicle" ) );

	GameEntity* ent;
	switch( availableVehicles[vehIndex].cat )
	{
	case CAT_PLANE:
		ent = new Entity_Plane;
		break;
	case CAT_HELO:
		ent = new Entity_Helicopter;
		break;
	case CAT_GROUND:
		ent = new Entity_GroundVehicle;
		break;
	case CAT_BOAT:
		ent = new Entity_Boat;
		break;
	case CAT_LQM:
		ent = new Entity_Infantry;
		break;
	default:
		//Com_Error( ERR_DROP, "Invalid vehicle type in MF_ClientSpawn" );
#pragma message("-->make GameEntity_Spectator!!")
		ent = new GameEntity;	// not chosen yet.. just spectator 
		break;
	}
	theLevel.setPlayerEntity(ent, clientNum);

	// set client
	GameClient* client = theLevel.getClient(clientNum);

	// If nukes are banned, disallow loadouts with any nukebombs or nukemissiles
	if(mf_allowNukes.integer == 0 && vehIndex >= 0)
	{
		for(cc = 0; cc < MAX_WEAPONS_PER_VEHICLE; cc++)
		{
			if( availableWeapons[availableVehicles[vehIndex].weapons[cc]].type == WT_NUKEBOMB || 
				availableWeapons[availableVehicles[vehIndex].weapons[cc]].type == WT_NUKEMISSILE ) {
				client->ps_.pm_flags &= ~PMF_VEHICLESPAWN;
				return;
			}
		}
	} 

	// find a spawn point
	// do it before setting health back up, so farthest
	// ranging doesn't count this client
	i = 0;
	if(cs_flags & CS_LASTPOS) 
	{
		// use current position as spawn point
		VectorCopy(client->ps_.origin, spawn_origin);
		spawn_origin[2] += 25;
		VectorCopy(client->ps_.vehicleAngles, spawn_angles);
	} 
	else 
	{
		if ( client->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR || 
			(client->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR &&
			(vehIndex < 0 || vehIndex >= bg_numberOfVehicles)) ) 
		{
			spawnPoint = SelectSpectatorSpawnPoint ( 
							spawn_origin, spawn_angles);
		} 
		else if (g_gametype.integer >= GT_TEAM ) 
		{
			// all base oriented team games use the CTF spawn points
			spawnPoint = SelectCTFSpawnPoint ( 
							client->sess_.sessionTeam_, 
							client->pers_.teamState_.state_, 
							spawn_origin, spawn_angles, vehIndex);
		} 
		else 
		{
			do 
			{
				i++;
				// the first spawn should be at a good looking spot
				if ( !client->pers_.initialSpawn_ && client->pers_.localClient_ )
				{
					client->pers_.initialSpawn_ = true;
					spawnPoint = SelectInitialSpawnPoint( spawn_origin, spawn_angles );
				} 
				else 
				{
					// don't spawn near existing origin if possible
					spawnPoint = SelectSpawnPoint ( 
						client->ps_.origin, 
						spawn_origin, spawn_angles,
						availableVehicles[vehIndex].cat);
				}
	
				if( i > 25) 
				{
					Com_Printf(S_COLOR_YELLOW "WARNING: Couldn't find a spawn point after over 25 tries (Check mf_lvcat?) \n");
					client->ps_.pm_flags &= ~PMF_VEHICLESPAWN;
					return;
					break;
				}

				// Tim needs to prevent bots from spawning at the initial point
				// on q3dm0...
				if ( ( spawnPoint->flags_ & FL_NO_BOTS ) && ( ent->r.svFlags & SVF_BOT ) )
					continue;	// try again
				
				// just to be symetric, we have a nohumans option...
				if ( ( spawnPoint->flags_ & FL_NO_HUMANS ) && !( ent->r.svFlags & SVF_BOT ) ) 
					continue;	// try again

				// check for category on this spawn point
				if( !(spawnPoint->ent_category_ & availableVehicles[vehIndex].cat) ) 
					continue;
				
				break;

			} while ( 1 );
		}
	}

	// Do initial vehicle checks
	// if respawning in current spot, and want to be a boat, check for water!
	if(vehIndex >= 0 && (cs_flags & CS_LASTPOS && availableVehicles[vehIndex].cat & CAT_BOAT) ) 
	{
		trace_t	trace;
		vec3_t	endpos;
		VectorCopy( spawn_origin, endpos );
		endpos[2] -= 512;
		SV_Trace (&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_WATER, false );
		if((cs_flags & CS_LASTPOS) && trace.entityNum != ENTITYNUM_NONE ) 
		{ 
			// Check for water if respawning in current location
			client->ps_.pm_flags &= ~PMF_VEHICLESPAWN;
			return;							// Fuck that! dont do shit ;p
		}
	}

	client->pers_.teamState_.state_ = GameClient::ClientPersistant::PlayerTeamState::TEAM_ACTIVE;

	// toggle the teleport bit so the client knows to not lerp
	// and never clear the voted flag
	flags = ent->client_->ps_.eFlags & (EF_TELEPORT_BIT | EF_VOTED | EF_TEAMVOTED);
	flags ^= EF_TELEPORT_BIT;

	// save veh
	savedNext = client->nextVehicle_;

	// clear everything but the persistant data
	saved = client->pers_;
	savedSess = client->sess_;
	savedPing = client->ps_.ping;
	accuracy_hits = client->accuracy_hits_;
	accuracy_shots = client->accuracy_shots_;
	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) 
		persistant[i] = client->ps_.persistant[i];

	// also save the predictable events otherwise we might get double or dropped events
	for (i = 0; i < MAX_PS_EVENTS; i++) 
		savedEvents[i] = client->ps_.events[i];

	eventSequence = client->ps_.eventSequence;

	//memset (client, 0, sizeof(*client));
	client->clear();

	client->pers_ = saved;
	client->sess_ = savedSess;
	client->ps_.ping = savedPing;
	client->accuracy_hits_ = accuracy_hits;
	client->accuracy_shots_ = accuracy_shots;
	client->lastkilled_client_ = -1;

	for ( i = 0 ; i < MAX_PERSISTANT ; i++ ) 
		client->ps_.persistant[i] = persistant[i];

	for (i = 0; i < MAX_PS_EVENTS; i++) 
		client->ps_.events[i] = savedEvents[i];

	client->ps_.eventSequence = eventSequence;
	// increment the spawncount so the client will detect the respawn
	client->ps_.persistant[PERS_SPAWN_COUNT]++;
	client->ps_.persistant[PERS_TEAM] = client->sess_.sessionTeam_;
	client->ps_.clientNum = clientNum;

	client->airOutTime_ = theLevel.time_ + 25000;

	client->ps_.eFlags = flags;

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	//ent->client_ = client;
	ent->takedamage_ = true;
	ent->inuse_ = true;
	ent->classname_ = "player";
	ent->r.contents = CONTENTS_BODY;
	ent->clipmask_ = MASK_PLAYERSOLID;
	ent->setDie(new VehicleBase::VehicleDeath);
	ent->waterlevel_ = 0;
	ent->watertype_ = 0;
	ent->flags_ = 0;
	ent->left_ = false; //first projectile starts right
	ent->bulletpos_ = 0;

	client->vehicle_ = client->nextVehicle_ = savedNext;

	// vehicles only stuff
	if( vehIndex >= 0 && vehIndex < bg_numberOfVehicles )
	{
		// distribute the weapons
		MF_getDefaultLoadoutForVehicle( vehIndex, &ent->loadout_, &ent->client_->ps_ );

		Info_SetValueForKey( userinfo, "cg_vehicle", va( "%d", vehIndex ) );
		SV_SetUserinfo( clientNum, userinfo );
		ClientUserinfoChanged( clientNum );

		// cat specific stuff
		if( availableVehicles[vehIndex].cat & CAT_PLANE ) 
		{
			// check for landed spawnpoint
			bool landed = false;
			trace_t	trace;
			vec3_t	endpos;
			GameEntity *test;
			VectorCopy( spawn_origin, endpos );
			endpos[2] -= 128;
			SV_Trace (&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_SOLID, false );
			if( trace.entityNum != ENTITYNUM_NONE ) 
			{
				test = theLevel.getEntity(trace.entityNum);//&g_entities[trace.entityNum];
				if( canLandOnIt(test) ) 
				{
					landed = true;
					spawn_origin[2] = test->r.maxs[2] - availableVehicles[vehIndex].mins[2] + 
						availableVehicles[vehIndex].gearheight;
				}			
			}
			MF_Spawn_Plane( ent, vehIndex, landed );
		} 
		else if( availableVehicles[vehIndex].cat & CAT_GROUND ) 
		{
			trace_t	trace;
			vec3_t	endpos;
			GameEntity *test;
			VectorCopy( spawn_origin, endpos );
			endpos[2] -= 512;

			SV_Trace (&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_SOLID, false );
			if( trace.entityNum != ENTITYNUM_NONE ) 
			{
				test = theLevel.getEntity(trace.entityNum);// &g_entities[trace.entityNum];
				spawn_origin[2] = trace.endpos[2] - availableVehicles[vehIndex].mins[2] + 1;
			}
			MF_Spawn_GroundVehicle( ent, vehIndex );
		} 
		else if( availableVehicles[vehIndex].cat & CAT_HELO ) 
		{
			MF_Spawn_Helo( ent, vehIndex, false );
		} 
		else if( availableVehicles[vehIndex].cat & CAT_LQM ) 
		{
			trace_t	trace;
			vec3_t	endpos;
			GameEntity *test;
			VectorCopy( spawn_origin, endpos );
			endpos[2] -= 512;

			SV_Trace(&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_SOLID, false );
			if( trace.entityNum != ENTITYNUM_NONE ) 
			{
				test = theLevel.getEntity(trace.entityNum);// &g_entities[trace.entityNum];
				spawn_origin[2] = trace.endpos[2] - availableVehicles[vehIndex].mins[2] + 0.1;
			}
			MF_Spawn_LQM( ent, vehIndex );
		} 
		else if( availableVehicles[vehIndex].cat & CAT_BOAT )
		{
			trace_t	trace;
			vec3_t	endpos;
			GameEntity *test;
			VectorCopy( spawn_origin, endpos );
			endpos[2] -= 512;
			SV_Trace(&trace, spawn_origin, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_ALL, false );
			if( trace.entityNum != ENTITYNUM_NONE ) 
			{
				test = theLevel.getEntity(trace.entityNum);// &g_entities[trace.entityNum];
				spawn_origin[2] = trace.endpos[2];
			}
			MF_Spawn_Boat( ent, vehIndex );
		}

		VectorCopy (availableVehicles[vehIndex].mins, ent->r.mins);
		VectorCopy (availableVehicles[vehIndex].maxs, ent->r.maxs);

		// set health
		handicap = atoi( Info_ValueForKey( userinfo, "handicap" ) );
		if ( handicap < 1 || handicap > 100 ) 
			handicap = 100;

		client->pers_.maxHealth_ = availableVehicles[vehIndex].maxhealth * handicap / 100;
		client->ps_.stats[STAT_MAX_HEALTH] = client->pers_.maxHealth_;

		// fuel
		client->ps_.stats[STAT_FUEL] = client->ps_.stats[STAT_MAX_FUEL] = availableVehicles[vehIndex].maxfuel;
		client->ps_.stats[STAT_LOCKINFO] = 0;
		client->ps_.timers[TIMER_FUEL] = theLevel.time_;

		// which weapons do we have
		for( i = WP_MACHINEGUN; i <= WP_FLARE; i++ ) 
		{
			if( availableVehicles[vehIndex].weapons[i] != WI_NONE ) 
			{
				client->ps_.ammo[i] = client->ps_.ammo[i+8] = availableVehicles[vehIndex].ammo[i] *
					availableWeapons[availableVehicles[vehIndex].weapons[i]].numberPerPackage;
			}
		}

		// select the first weapon
		for( i = WP_MACHINEGUN; i <= WP_FLARE; i++ ) 
		{
			if( availableVehicles[vehIndex].weapons[i] != WI_NONE ) 
			{
				client->ps_.weaponIndex = availableVehicles[vehIndex].weapons[i];
				break;
			}
		}
		if( client->ps_.weaponIndex == WI_NONE ) 
			client->ps_.weaponIndex = availableVehicles[vehIndex].weapons[0];

	}
	else 
	{ 
		// spectator only stuff
		VectorSet( ent->r.mins, -8, -8, -8 );
		VectorSet( ent->r.maxs, 8, 8, 8 );
		if( client->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
		{
			client->ps_.pm_flags |= PMF_VEHICLESELECT;
			client->ps_.pm_type = PM_FREEZE;
			client->vehicle_ = -1;
		}
	}

	// health will count down towards max_health
	ent->health_ = client->ps_.stats[STAT_HEALTH] = client->ps_.stats[STAT_MAX_HEALTH] + 25;

	G_SetOrigin( ent, spawn_origin );
	VectorCopy( spawn_origin, client->ps_.origin );

	// the respawned flag will be cleared after the attack and jump keys come up
	client->ps_.pm_flags |= PMF_RESPAWNED;

	//SV_GetUsercmd( client - level.clients, &ent->client->pers.cmd );
	SV_GetUsercmd( client->ps_.clientNum, &ent->client_->pers_.cmd_ );
	SetClientViewAngle( ent, spawn_angles );

	if ( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{

	} 
	else 
	{
		if(!(cs_flags & CS_NOKILL))
			G_KillBox( ent );
		SV_LinkEntity (ent);
	}

	// reset radar
	ent->client_->ps_.ONOFF |= ent->client_->pers_.lastRadar_;

	// don't allow full run speed for a bit
	client->ps_.pm_flags |= PMF_TIME_KNOCKBACK;
	client->ps_.pm_time = 100;

	client->respawnTime_ = theLevel.time_;
	client->inactivityTime_ = theLevel.time_ + g_inactivity.integer * 1000;
	client->latched_buttons_ = 0;

	// set default animations
	client->ps_.vehicleAnim = 0;
	client->ps_.gunAngle = 0;
	client->ps_.turretAngle = 0;

	if ( theLevel.intermissiontime_ )
	{
		MoveClientToIntermission( ent );
	} 
	else 
	{
		// fire the targets of the spawn point
		G_UseTargets( spawnPoint, ent );
	}

	// reset lockinfo
	unlock( ent );

	// run a client frame to drop exactly to the floor,
	// initialize animations and other things
	client->ps_.commandTime = theLevel.time_ - 100;
	ent->client_->pers_.cmd_.serverTime = theLevel.time_;
	ClientThink( ent->s.number );// ent-g_entities );

	// positively link the client, even if the command times are weird
	if( ent->client_->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
	{
		BG_PlayerStateToEntityState( &client->ps_, &ent->s, true );
		VectorCopy( ent->client_->ps_.origin, ent->r.currentOrigin );
		SV_LinkEntity( ent );
	}

	// run the presend to set anything else
	ClientEndFrame( ent );

	// clear entity state values
	BG_PlayerStateToEntityState( &client->ps_, &ent->s, true );
}
