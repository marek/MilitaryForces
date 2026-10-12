/*
 * $Id: g_utils.c,v 1.8 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_utils.c -- misc utility functions for game module

#include "g_local.h"
#include "g_level.h"


struct shaderRemap_t {
  char oldShader[MAX_QPATH];
  char newShader[MAX_QPATH];
  float timeOffset;
};

constexpr int MAX_SHADER_REMAPS = 128;

static int remapCount = 0;
shaderRemap_t remappedShaders[MAX_SHADER_REMAPS];

void AddRemap(const char *oldShader, const char *newShader, float timeOffset) {
	int i;

	for (i = 0; i < remapCount; i++) {
		if (Q_stricmp(oldShader, remappedShaders[i].oldShader) == 0) {
			// found it, just update this one
			strcpy(remappedShaders[i].newShader,newShader);
			remappedShaders[i].timeOffset = timeOffset;
			return;
		}
	}
	if (remapCount < MAX_SHADER_REMAPS) {
		strcpy(remappedShaders[remapCount].newShader,newShader);
		strcpy(remappedShaders[remapCount].oldShader,oldShader);
		remappedShaders[remapCount].timeOffset = timeOffset;
		remapCount++;
	}
}

const char *BuildShaderStateConfig() {
	static char	buff[MAX_STRING_CHARS];
	char out[(MAX_QPATH * 2) + 5];
	int i;
  
	memset(buff, 0, sizeof(buff));
	for (i = 0; i < remapCount; i++) {
		Com_sprintf(out, (MAX_QPATH * 2) + 5, "%s=%s:%5.2f@", remappedShaders[i].oldShader, remappedShaders[i].newShader, remappedShaders[i].timeOffset);
		Q_strcat( buff, sizeof( buff ), out);
	}
	return buff;
}

/*
=========================================================================

model / sound configstring indexes

=========================================================================
*/

/*
================
G_FindConfigstringIndex

================
*/
static int G_FindConfigstringIndex( char *name, int start, int max, bool create ) 
{
	int		i;
	char	s[MAX_STRING_CHARS];

	if ( !name || !name[0] ) 
		return 0;

	for ( i=1 ; i<max ; i++ ) 
	{
		GameHost::getConfigstring( start + i, s, sizeof( s ) );
		if ( !s[0] ) 
			break;

		if ( !strcmp( s, name ) ) 
			return i;
	}

	if ( !create ) 
		return 0;

	if ( i == max ) 
		Common::error( ERR_DROP, "G_FindConfigstringIndex: overflow" );

	GameHost::setConfigstring( start + i, name );

	return i;
}


int Game::modelIndex( char *name ) 
{
	return G_FindConfigstringIndex (name, CS_MODELS, MAX_MODELS, true);
}

int Game::soundIndex( char *name ) 
{
	return G_FindConfigstringIndex (name, CS_SOUNDS, MAX_SOUNDS, true);
}

//=====================================================================


/*
================
Game::teamCommand

Broadcasts a command to only a specific team
================
*/
void Game::teamCommand( ClientBase::eTeam team, char *cmd ) 
{
	int			i;
	GameClient*	client;	

	for ( i = 1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		client = theLevel.getClient(i);
		if( client && client->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED ) 
		{
			if ( client->sess_.sessionTeam_ == team )
				GameHost::sendServerCommand( i, va("%s", cmd ));
		}
	}
}


/*
=============
Game::find

Searches all active entities for the next one that holds
the matching string at fieldofs (use the FOFS() macro) in the structure.

Searches beginning at the entity after from, or the beginning if NULL
NULL will be returned if the end of the list is reached.

=============
*/
GameEntity* Game::find( GameEntity *from, int fieldofs, const char *match )
{
	//char	*s;

	//if (!from)
	//	from = theLevel.gameEntities_.at(0);// g_entities;
	//else
	//	theLevel.gameEntities_[from->s.number+1];	//from++;

	//for ( ; from < theLevel.gameEntities_[theLevel.num_entities_]; from++ ) // &g_entities[level.num_entities] ; from++)
	size_t i;
	if( !from )
		i = 0;
	else
		i = from->s.number + 1;

	for( ; i < theLevel.num_entities_; ++i )
	{
		GameEntity* search = theLevel.getEntity(i);

		if( !search || !search->inuse_ )
			continue;

		char *s = *reinterpret_cast<char **>(reinterpret_cast<byte *>(search) + fieldofs);
		if (!s)
			continue;
		if (!Q_stricmp (s, match))
		{
			from = search;
			return from;
		}
	}

	return nullptr;
}


/*
=============
Game::pickTarget

Selects a random entity from among the targets
=============
*/
constexpr int MAXCHOICES = 32;

GameEntity* Game::pickTarget( char *targetname )
{
	GameEntity*	ent = nullptr;
	int			num_choices = 0;
	GameEntity*	choice[MAXCHOICES];

	if (!targetname)
	{
		Common::printf("Game::pickTarget called with NULL targetname\n");
		return nullptr;
	}

	while(true)
	{
		ent = Game::find (ent, FOFS(targetname_), targetname);
		if (!ent)
			break;
		choice[num_choices++] = ent;
		if (num_choices == MAXCHOICES)
			break;
	}

	if (!num_choices)
	{
		Common::printf("Game::pickTarget: target %s not found\n", targetname);
		return nullptr;
	}

	return choice[rand() % num_choices];
}


/*
==============================
Game::useTargets

"activator" should be set to the entity that initiated the firing.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function

==============================
*/
void Game::useTargets( GameEntity *ent, GameEntity *activator ) 
{
	GameEntity		*t;
	
	if ( !ent ) 
		return;

	if (ent->targetShaderName_ && ent->targetShaderNewName_) 
	{
		float f = theLevel.time_ * 0.001;
		AddRemap(ent->targetShaderName_, ent->targetShaderNewName_, f);
		GameHost::setConfigstring(CS_SHADERSTATE, BuildShaderStateConfig());
	}

	if ( !ent->target_ ) 
		return;

	t = nullptr;
	while ( (t = Game::find (t, FOFS(targetname_), ent->target_)) != nullptr ) 
	{
		if ( t == ent )
		{
			Common::printf ("WARNING: Entity used itself.\n");
		} 
		else
		{
			//if ( t->use ) 
			t->useFunc_->execute( ent, activator );
		}
		if ( !ent->inuse_ ) 
		{
			Common::printf("entity was removed while using targets\n");
			return;
		}
	}
}


/*
=============
TempVector

This is just a convenience function
for making temporary vectors for function calls
=============
*/
float	*tv( float x, float y, float z )
{
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}


/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos( const vec3_t v ) 
{
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", static_cast<int>(v[0]), static_cast<int>(v[1]), static_cast<int>(v[2]));

	return s;
}


/*
===============
Game::setMovedir

The editor only specifies a single value for angles (yaw),
but we have special constants to generate an up or down direction.
Angles will be cleared, because it is being used to represent a direction
instead of an orientation.
===============
*/
void Game::setMovedir( vec3_t angles, vec3_t movedir ) 
{
	static vec3_t VEC_UP		= {0, -1, 0};
	static vec3_t MOVEDIR_UP	= {0, 0, 1};
	static vec3_t VEC_DOWN		= {0, -2, 0};
	static vec3_t MOVEDIR_DOWN	= {0, 0, -1};

	if ( VectorCompare (angles, VEC_UP) ) 
	{
		VectorCopy (MOVEDIR_UP, movedir);
	} 
	else if ( VectorCompare (angles, VEC_DOWN) )
	{
		VectorCopy (MOVEDIR_DOWN, movedir);
	} 
	else 
	{
		AngleVectors (angles, movedir, nullptr, nullptr);
	}
	VectorClear( angles );
}


float vectoyaw( const vec3_t vec ) 
{
	float	yaw;
	
	if (vec[YAW] == 0 && vec[PITCH] == 0) 
	{
		yaw = 0;
	} 
	else
	{
		if (vec[PITCH])
			yaw = ( atan2( vec[YAW], vec[PITCH]) * 180 / std::numbers::pi_v<float> );
		else if (vec[YAW] > 0)
			yaw = 90;
		else 
			yaw = 270;
		if (yaw < 0) 
			yaw += 360;
	}

	return yaw;
}


///*
//=================
//G_Spawn
//
//Either finds a free entity, or allocates a New one.
//
//  The slots from 0 to MAX_CLIENTS-1 are always reserved for clients, and will
//never be used by anything else.
//
//Try to avoid reusing an entity that was recently freed, because it
//can cause the client to think the entity morphed into something else
//instead of being removed and recreated, which can cause interpolated
//angles and bad trails.
//=================
//*/
//GameEntity*	G_Spawn() 
//{
//	int			i = 0;
//	GameEntity*	e = 0;
//
//	for( int force = 0 ; force < 2 ; force++ ) 
//	{
//		// if we go through all entities and can't find one to free,
//		// override the normal minimum times before use
//		//e = theLevel.gameEntities_.at(MAX_CLIENTS);// &g_entities[MAX_CLIENTS];
//		for( i = MAX_CLIENTS + 1 ; i < theLevel.num_entities_ ; i++ ) //, e++ ) 
//		{
//			e = theLevel.getEntity(i);
//
//			if( e->inuse_ ) 
//				continue;
//
//			// the first couple seconds of server time can involve a lot of
//			// freeing and allocating, so relax the replacement policy
//			if( !force && e->freetime_ > theLevel.startTime_ + 2000 && theLevel.time_ - e->freetime_ < 1000 )
//				continue;
//
//			// reuse this slot
//			e->init(-1);
//			return e;
//		}
//		if ( i != MAX_GENTITIES ) 
//			break;
//	}
//	if( i == ENTITYNUM_MAX_NORMAL ) 
//	{
//		for( i = 0; i < MAX_GENTITIES; i++ ) 
//		{
//			GameEntity* currEnt = theLevel.getEntity(i);
//			Common::printf("%4i: %s\n", i, currEnt->classname_);// g_entities[i].classname);
//		}
//		Common::error( ERR_DROP, "G_Spawn: no free entities" );
//	}
//	
//	// open up a New slot
//	theLevel.num_entities_++;
//
//	// let the server system know that there are more entities
//	//GameHost::locateGameData( &theLevel.gameEntities_, &theLevel.clients_ );
//	//GameHost::locateGameData( (void*)level.gentities, level.num_entities, sizeof( gentity_t ), 
//	//	&level.clients[0].ps, sizeof( level.clients[0] ) );
//
//	e->init(-1);
//	return e;
//}

///*
//=================
//G_FreeEntity
//
//Marks the entity as free
//=================
//*/
//void G_FreeEntity( GameEntity *ed ) 
//{
//	GameHost::unlinkEntity( ed );		// unlink from world
//
//	if( ed->neverFree_ ) 
//		return;
//
//	//memset( ed, 0, sizeof(*ed) );
//	ed->cleanAll();
//	ed->classname_ = "freed";
//	ed->freetime_ = theLevel.time_;
//	ed->inuse_ = false;
//}

/*
=================
Game::tempEntity

Spawns an event entity that will be auto-removed
The origin will be snapped to save net bandwidth, so care
must be taken if the origin is right on a surface (snap towards start vector first)
=================
*/
GameEntity *Game::tempEntity( vec3_t origin, int event ) 
{
	GameEntity	*e;
	Vec3		snapped;

	e = theLevel.spawnEntity();
	e->s.eType = ET_EVENTS + event;

	e->classname_ = "tempEntity";
	e->eventTime_ = theLevel.time_;
	e->freeAfterEvent_ = true;

	VectorCopy( origin, snapped );
	SnapVector( snapped );		// save network bandwidth
	Game::setOrigin( e, snapped );

	// find cluster for PVS
	GameHost::linkEntity( e );

	return e;
}



/*
==============================================================================

Kill box

==============================================================================
*/

/*
=================
Game::killBox

Kills all entities that would touch the proposed New positioning
of ent.  Ent should be unlinked before calling this!
=================
*/
void Game::killBox( GameEntity *ent )
{
	int			i, num;
	int			touch[MAX_GENTITIES];
	GameEntity	*hit;
	Vec3		mins, maxs;

	const bool soldier = ent->client_->vehicle_ >= 0 && (availableVehicles[ent->client_->vehicle_].cat & CAT_LQM);

	VectorAdd( ent->client_->ps_.origin, ent->r.mins, mins );
	VectorAdd( ent->client_->ps_.origin, ent->r.maxs, maxs );
	num = GameHost::areaEntities( mins, maxs, touch, MAX_GENTITIES );

	for( i=0 ; i<num ; i++ )
	{
		hit = theLevel.getEntity(touch[i]);
		if( !hit || !hit->client_ ) 
			continue;

		// a soldier arriving over a boat lands on its deck
		if( soldier && hit->health_ > 0 && hit->client_->vehicle_ >= 0 && availableVehicles[hit->client_->vehicle_].hasDeck )
			continue;

		// nail it
		Game::damage ( hit, ent, ent, nullptr, nullptr, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG, CAT_ANY );
	}
}

//==============================================================================

/*
===============
Game::addPredictableEvent

Use for non-pmove events that would also be predicted on the
client side: jumppads and item pickups
Adds an event+parm and twiddles the event counter
===============
*/
void Game::addPredictableEvent( GameEntity *ent, int event, int eventParm )
{
	if( !ent->client_ ) 
		return;

	GameShared::addPredictableEventToPlayerstate( event, eventParm, &ent->client_->ps_ );
}

/*
===============
Event queue
===============
*/

void Game::sendEventFromQueue( GameEntity* ent )
{
	int i = ent->currentEventToSend_;

	// nothing to be done
	if( !ent->eventQueue_[i].used_ ) 
		return;

	Game::addEvent( ent, ent->eventQueue_[i].event_, ent->eventQueue_[i].eventParm_, false );
	ent->eventQueue_[i].used_ = false;

	i++;
	if( i >= GameEntity::k_MAX_EVENT_QUEUE_SIZE ) 
		i = 0;
	ent->currentEventToSend_ = i;
}

static void G_AddEventToQueue( GameEntity* ent, int event, int eventParm )
{
	int i = ent->currentEventToAdd_;

	if( ent->eventQueue_[i].used_ ) 
	{
		Common::printf( "Event queue full! Event lost!\n" );
		return;
	}
	ent->eventQueue_[i].event_ = event;
	ent->eventQueue_[i].eventParm_ = eventParm;
	ent->eventQueue_[i].used_ = true;

	i++;
	if( i >= GameEntity::k_MAX_EVENT_QUEUE_SIZE ) 
		i = 0;
	ent->currentEventToAdd_ = i;
}

/*
===============
Game::addEvent

Adds an event+parm and twiddles the event counter
===============
*/
void Game::addEvent( GameEntity *ent, int event, int eventParm, bool addToQueue )
{
	int bits = 0;

	if( !event ) 
	{
		Common::printf( "Game::addEvent: zero event added for entity %i\n", ent->s.number );
		return;
	}

	if( ent->eventSent_ ) 
	{
		if( addToQueue ) 
			G_AddEventToQueue( ent, event, eventParm );
		return;
	}

	// clients need to add the event in playerState_t instead of entityState_t
	if ( ent->client_ )
	{
		bits = ent->client_->ps_.externalEvent & EV_EVENT_BITS;
		bits = ( bits + EV_EVENT_BIT1 ) & EV_EVENT_BITS;
		ent->client_->ps_.externalEvent = event | bits;
		ent->client_->ps_.externalEventParm = eventParm;
		ent->client_->ps_.externalEventTime = theLevel.time_;
	}
	else
	{
		bits = ent->s.event & EV_EVENT_BITS;
		bits = ( bits + EV_EVENT_BIT1 ) & EV_EVENT_BITS;
		ent->s.event = event | bits;
		ent->s.eventParm = eventParm;
	}
	ent->eventTime_ = theLevel.time_;
	ent->eventSent_ = true;
}


/*
=============
Game::sound
=============
*/
void Game::sound( GameEntity *ent, int channel, int soundIndex ) 
{
	GameEntity
		*te;

	te = Game::tempEntity( ent->r.currentOrigin, EV_GENERAL_SOUND );
	te->s.eventParm = soundIndex;
}


//==============================================================================


/*
================
Game::setOrigin

Sets the pos trajectory for a fixed position
================
*/
void Game::setOrigin( GameEntity *ent, vec3_t origin ) 
{
	VectorCopy( origin, ent->s.pos.trBase );
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.pos.trTime = 0;
	ent->s.pos.trDuration = 0;
	VectorClear( ent->s.pos.trDelta );

	VectorCopy( origin, ent->r.currentOrigin );
}



// --------------------
//		mfq3 stuff
// --------------------


unsigned long Game::getGameset()
{
	char buffer[33];

	CvarSystem::variableStringBuffer("mf_gameset", buffer, 32);

	if( strcmp( buffer, "modern" ) == 0 ) 
		return MF_GAMESET_MODERN;
	else if( strcmp( buffer, "ww2" ) == 0 ) 
		return MF_GAMESET_WW2;
	else if( strcmp( buffer, "ww1" ) == 0 ) 
		return MF_GAMESET_WW1;
	else 
	{	
		// default gameset
		Common::printf( "mf_gameset is not valid, defaulting 'modern'\n" );
		CvarSystem::set("mf_gameset", "modern");
		return MF_GAMESET_MODERN;
	}
}

