/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// sv_game.c -- interface to the game dll

#include "server.h"

//#include "../botlib/botlib.h"
#include "../game/game.h"
#include "sv_server.h"
#include "sv_client.h"


//botlib_export_t	*botlib_export;

//void SV_GameError( const char *string ) 
//{
//	Common::error( ERR_DROP, "%s", string );
//}

//void SV_GamePrint( const char *string ) 
//{
//	Common::printf( "%s", string );
//}

 //these functions must be used instead of pointer arithmetic, because
 //the game allocates gentities with private information after the server shared part
//int	SV_NumForGentity( sharedEntity_t *ent ) 
//{
//	int		num;
//
//	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;
//
//	return num;
//}

//sharedEntity_t *SV_GentityNum( int num ) 
//{
//	sharedEntity_t *ent;
//
//	ent = (sharedEntity_t *)((byte *)sv.gentities + sv.gentitySize*(num));
//
//	return ent;
//}

//playerState_t *SV_GameClientNum( int num ) 
//{
//	playerState_t	*ps;
//
//	ps = (playerState_t *)((byte *)sv.gameClients + sv.gameClientSize*(num));
//
//	return ps;
//}

//svEntity_t	*SV_SvEntityForGentity( const entityState_t* s, const entityShared_t* r ) 
//{
//	if ( !s || !r || s->number < 0 || s->number >= MAX_GENTITIES ) 
//	{
//		Common::error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
//	}
//	return &sv.svEntities[ s->number ];
//}

//sharedEntity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) 
//{
//	int		num;
//
//	num = svEnt - sv.svEntities;
//	return theSV.getEntityByNum( num );
//}

/*
===============
GameHost::sendServerCommand

Sends a command string to a client
===============
*/
void GameHost::sendServerCommand( int clientNum, const char *text ) 
{
	if ( clientNum == -1 ) 
	{
		ServerMain::sendServerCommand( nullptr, "%s", text );
	} 
	else 
	{
		if ( clientNum < 1 || clientNum > sv_maxclients->integer || !theSVS.svClients_.at(clientNum) ) 
			return;

		ServerMain::sendServerCommand( theSVS.svClients_.at(clientNum), "%s", text );	
	}
}


/*
===============
GameHost::dropClient

Disconnects the client with a message
===============
*/
void GameHost::dropClient( int clientNum, const char *reason ) 
{
	if ( clientNum < 1 || clientNum > sv_maxclients->integer || !theSVS.svClients_.at(clientNum) ) 
		return;

	(theSVS.svClients_.at(clientNum))->drop( reason );	
}


/*
=================
GameHost::setBrushModel

sets mins and maxs for inline bmodels
=================
*/
void GameHost::setBrushModel( EntityBase* gameEnt, const char *name ) 
{
	clipHandle_t	h;
	vec3_t			mins, maxs;

	if (!name) 
		Common::error( ERR_DROP, "SV_SetBrushModel: NULL" );

	if (name[0] != '*') 
		Common::error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );

	gameEnt->s.modelindex = atoi( name + 1 );

	h = CollisionModel::inlineModel( gameEnt->s.modelindex );
	CollisionModel::modelBounds( h, mins, maxs );
	VectorCopy (mins, gameEnt->r.mins);
	VectorCopy (maxs, gameEnt->r.maxs);
	gameEnt->r.bmodel = true;

	gameEnt->r.contents = -1;		// we don't know exactly what is in the brushes

	GameHost::linkEntity( gameEnt );		// FIXME: remove
}



/*
=================
GameHost::inPVS

Also checks portalareas so that doors block sight
=================
*/
bool GameHost::inPVS (const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CollisionModel::pointLeafnum (p1);
	cluster = CollisionModel::leafCluster (leafnum);
	area1 = CollisionModel::leafArea (leafnum);
	mask = CollisionModel::clusterPVS (cluster);

	leafnum = CollisionModel::pointLeafnum (p2);
	cluster = CollisionModel::leafCluster (leafnum);
	area2 = CollisionModel::leafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;
	if (!CollisionModel::areasConnected (area1, area2))
		return false;		// a door blocks sight
	return true;
}


/*
=================
ServerMain::inPVSIgnorePortals

Does NOT check portalareas
=================
*/
bool ServerMain::inPVSIgnorePortals( const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CollisionModel::pointLeafnum (p1);
	cluster = CollisionModel::leafCluster (leafnum);
	area1 = CollisionModel::leafArea (leafnum);
	mask = CollisionModel::clusterPVS (cluster);

	leafnum = CollisionModel::pointLeafnum (p2);
	cluster = CollisionModel::leafCluster (leafnum);
	area2 = CollisionModel::leafArea (leafnum);

	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;

	return true;
}


/*
========================
GameHost::adjustAreaPortalState
========================
*/
void GameHost::adjustAreaPortalState( EntityBase* gameEnt, int open ) 
{
	SV_Entity	*svEnt;

	svEnt = theSV.SVEntityForGameEntity( gameEnt );
	if ( svEnt->areanum2_ == -1 ) 
		return;

	CollisionModel::adjustAreaPortalState( svEnt->areanum_, svEnt->areanum2_, open );
}


/*
==================
SV_GameAreaEntities
==================
*/
bool GameHost::entityContact( const vec3_t mins, const vec3_t maxs, EntityBase* gameEnt, bool capsule ) 
{
	const float		*origin, *angles;
	clipHandle_t	ch;
	trace_t			trace;

	// check for exact collision
	origin = gameEnt->r.currentOrigin;
	angles = gameEnt->r.currentAngles;

	ch = ServerMain::clipHandleForEntity( gameEnt );
	CollisionModel::transformedBoxTrace ( &trace, vec3_origin, vec3_origin, const_cast<float *>(mins), const_cast<float *>(maxs),
		ch, -1, origin, angles, capsule );

	return trace.startsolid;
}


/*
===============
GameHost::getServerinfo

===============
*/
void GameHost::getServerinfo( char *buffer, int bufferSize ) 
{
	if ( bufferSize < 1 ) {
		Common::error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, CvarSystem::infoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
GameHost::locateGameData

===============
*/
void GameHost::locateGameData( std::vector<std::unique_ptr<EntityBase>>* gameEntities, 
					    std::vector<std::unique_ptr<ClientBase>>* gameClients )
//void GameHost::locateGameData( void* gEnts, int numGEntities, int sizeofGEntity_t,
//					   playerState_t *clients, int sizeofGameClient ) 
{
	theSV.setGameData( gameEntities, gameClients );
//	theSV.gameEntities_ = gameEntities;
//	theSV.gameClients_ = gameClients;
}


/*
===============
GameHost::getUsercmd

===============
*/
void GameHost::getUsercmd( int clientNum, usercmd_t *cmd ) 
{
	if ( clientNum < 1 || clientNum > sv_maxclients->integer || !theSVS.svClients_.at(clientNum) ) 
	{
		Common::error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = theSVS.svClients_.at(clientNum)->lastUsercmd_;
}

//==============================================


/*
===============
ServerLoop::shutdownGameProgs

Called every time a map changes
===============
*/
void ServerLoop::shutdownGameProgs() 
{
	if( !theSG.isInitialized() )
		return;

	theSG.shutdown();
}

/*
==================
ServerMain::initGame

Called for both a full init and a restart
==================
*/
void ServerMain::initGame() 
{
	// start the entity parsing at the beginning
	theSV.entityParsePoint_ = CollisionModel::entityString();

	// clear all gentity pointers that might still be set from
	// a previous level
	//   now done before GAME_INIT call
	//for( int i = 1 ; i <= sv_maxclients->integer ; i++ ) 
	//	theSVS.svClients_.at(i)->gentity_ = 0;
	
	// use the current msec count for a random seed
	// init for this gamestate
	//VM_Call( gvm, GAME_INIT, svs.time, Common::milliseconds(), restart );
	theSG.init( theSV.time_, Common::milliseconds() );
}



/*
===================
ServerMain::restartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void ServerMain::restartGameProgs( ) 
{
	if( !theSG.isInitialized() )
		return;

	theSG.shutdown();

	ServerMain::initGame();
}

/*
====================
ServerLoop::gameCommand

See if the current console command is claimed by the game
====================
*/
bool ServerLoop::gameCommand() 
{
	if ( theSV.state_ != Server::SS_GAME ) 
		return false;

	//return (VM_Call( gvm, GAME_CONSOLE_COMMAND )!=0);
	return theSG.consoleCommand();
}

