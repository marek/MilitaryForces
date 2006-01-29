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
//	Com_Error( ERR_DROP, "%s", string );
//}

//void SV_GamePrint( const char *string ) 
//{
//	Com_Printf( "%s", string );
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
//		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
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
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text ) 
{
	if ( clientNum == -1 ) 
	{
		SV_SendServerCommand( NULL, "%s", text );
	} 
	else 
	{
		if ( clientNum < 1 || clientNum > sv_maxclients->integer ) 
			return;

		SV_SendServerCommand( theSVS.svClients_.at(clientNum), "%s", text );	
	}
}


/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( int clientNum, const char *reason ) 
{
	if ( clientNum < 1 || clientNum > sv_maxclients->integer ) 
		return;

	SV_DropClient( theSVS.svClients_.at(clientNum), reason );	
}


/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( EntityBase* gameEnt, const char *name ) 
{
	clipHandle_t	h;
	vec3_t			mins, maxs;

	if (!name) 
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );

	if (name[0] != '*') 
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );

	gameEnt->s.modelindex = atoi( name + 1 );

	h = CM_InlineModel( gameEnt->s.modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy (mins, gameEnt->r.mins);
	VectorCopy (maxs, gameEnt->r.maxs);
	gameEnt->r.bmodel = true;

	gameEnt->r.contents = -1;		// we don't know exactly what is in the brushes

	SV_LinkEntity( gameEnt );		// FIXME: remove
}



/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
bool SV_inPVS (const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;
	if (!CM_AreasConnected (area1, area2))
		return false;		// a door blocks sight
	return true;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
bool SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);

	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return false;

	return true;
}


/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( EntityBase* gameEnt, int open ) 
{
	SV_Entity	*svEnt;

	svEnt = theSV.SVEntityForGameEntity( gameEnt );
	if ( svEnt->areanum2_ == -1 ) 
		return;

	CM_AdjustAreaPortalState( svEnt->areanum_, svEnt->areanum2_, open );
}


/*
==================
SV_GameAreaEntities
==================
*/
bool SV_EntityContact( const vec3_t mins, const vec3_t maxs, EntityBase* gameEnt, bool capsule ) 
{
	const float		*origin, *angles;
	clipHandle_t	ch;
	trace_t			trace;

	// check for exact collision
	origin = gameEnt->r.currentOrigin;
	angles = gameEnt->r.currentAngles;

	ch = SV_ClipHandleForEntity( gameEnt );
	CM_TransformedBoxTrace ( &trace, vec3_origin, vec3_origin, mins, maxs,
		ch, -1, origin, angles, capsule );

	return trace.startsolid;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) 
{
	if ( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( std::vector<EntityBase*>* gameEntities, 
					    std::vector<ClientBase*>* gameClients )
//void SV_LocateGameData( void* gEnts, int numGEntities, int sizeofGEntity_t,
//					   playerState_t *clients, int sizeofGameClient ) 
{
	theSV.setGameData( gameEntities, gameClients );
//	theSV.gameEntities_ = gameEntities;
//	theSV.gameClients_ = gameClients;
}


/*
===============
SV_GetUsercmd

===============
*/
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) 
{
	if ( clientNum < 1 || clientNum > sv_maxclients->integer ) 
	{
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = theSVS.svClients_.at(clientNum)->lastUsercmd_;
}

//==============================================


/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs() 
{
	if( !theSG.isInitialized() )
		return;

	theSG.shutdown();
}

/*
==================
SV_InitGame

Called for both a full init and a restart
==================
*/
void SV_InitGame() 
{
	// start the entity parsing at the beginning
	theSV.entityParsePoint_ = CM_EntityString();

	// clear all gentity pointers that might still be set from
	// a previous level
	//   now done before GAME_INIT call
	//for( int i = 1 ; i <= sv_maxclients->integer ; i++ ) 
	//	theSVS.svClients_.at(i)->gentity_ = 0;
	
	// use the current msec count for a random seed
	// init for this gamestate
	//VM_Call( gvm, GAME_INIT, svs.time, Com_Milliseconds(), restart );
	theSG.init( theSVS.time_, Com_Milliseconds() );
}



/*
===================
SV_RestartGameProgs

Called on a map_restart, but not on a normal map change
===================
*/
void SV_RestartGameProgs( void ) 
{
	if( !theSG.isInitialized() )
		return;

	theSG.shutdown();

	SV_InitGame();
}

/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
bool SV_GameCommand() 
{
	if ( theSV.state_ != Server::SS_GAME ) 
		return false;

	//return (VM_Call( gvm, GAME_CONSOLE_COMMAND )!=0);
	return theSG.consoleCommand();
}

