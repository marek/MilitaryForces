#include "g_client.h"
#include "g_entity.h"
#include "g_level.h"
#include "g_local.h"
#include "g_vehicle.h"
#include "g_item.h"

LevelLocals::LevelLocals()
{
	resetAll();
}

LevelLocals::~LevelLocals()
{
	resetClientsAndEntities();
}

LevelLocals&
LevelLocals::getInstance()
{
	static LevelLocals ll;
	return ll;
}

void
LevelLocals::resetAll()
{
	resetClientsAndEntities();

	num_entities_ = 0;
	warmupTime_ = 0;
	logFile_ = 0;
	maxclients_ = 0;
	framenum_ = 0;
	time_ = 0;
	previousTime_ = 0;
	startTime_ = 0;
	memset( teamScores_, 0, sizeof(teamScores_) );
	lastTeamLocationTime_ = 0;
	newSession_= false;	
	restarted_ = false;
	numConnectedClients_ = 0;
	numNonSpectatorClients_ = 0;
	numPlayingClients_ = 0;
	memset( sortedClients_, 0, sizeof(sortedClients_) );
	follow1_ = 0; 
	follow2_ = 0;
	warmupModificationCount_ = 0;
	memset( voteString_, 0 , sizeof(voteString_) );
	memset( voteDisplayString_, 0, sizeof(voteDisplayString_) );
	voteTime_ = 0;
	voteExecuteTime_ = 0;
	voteYes_ = 0;
	voteNo_ = 0;
	numVotingClients_ = 0;
	memset( teamVoteString_, 0, sizeof(teamVoteString_) );
	memset( teamVoteTime_, 0, sizeof(teamVoteTime_) );
	memset( teamVoteYes_, 0, sizeof(teamVoteYes_) );
	memset( teamVoteNo_, 0, sizeof(teamVoteNo_) );
	memset( numteamVotingClients_, 0, sizeof(numteamVotingClients_) );
	spawning_ = false;
	numSpawnVars_ = 0;
	memset( spawnVars_, 0, sizeof(spawnVars_) );
	numSpawnVarChars_ = 0;
	memset( spawnVarChars_, 0, sizeof(spawnVarChars_) );
	ent_category_ = 0;
	intermissionQueued_ = 0;
	intermissiontime_ = 0;
	changemap_ = 0;
	readyToExit_ = false;
	exitTime_ = 0;
	VectorClear( intermission_origin_ );
	VectorClear( intermission_angle_ );
	locationLinked_ = false;
	locationHead_ = 0;
}

void
LevelLocals::resetClientsAndEntities()
{
	for( size_t i = 0; i < clients_.size(); ++i )
	{
		delete clients_.at(i);
	}
	clients_.clear();
	clients_.resize(MAX_CLIENTS+1);	// first one is empty (world)
	for( size_t i = 0; i <= MAX_CLIENTS; ++i )
	{
		clients_.at(i) = 0;
	}

	for( size_t i = 0; i < gameEntities_.size(); ++i )
	{
		delete gameEntities_.at(i);
	}
	gameEntities_.clear();
	gameEntities_.resize(MAX_CLIENTS+1);// 0 = world, after that the clients 1 -> MAX_CLIENTS + 1
	for( size_t i = 0; i <= MAX_CLIENTS; ++i )
	{
		gameEntities_.at(i) = new GameEntity;// 0;
	}
	num_entities_ = 0;
}

void
LevelLocals::addEntity(GameEntity* newEnt)
{
	// player, needs to be put in front slot
	if( newEnt->client_ )
	{
		for( size_t i = 1; i <= MAX_CLIENTS; ++i )
		{
			if( !gameEntities_.at(i) )
			{
				gameEntities_.at(i) = newEnt;
				clients_.at(i) = newEnt->client_;
				break;
			}
		}
	}
	// normal entity, append
	else
	{
		gameEntities_.push_back(newEnt);
		num_entities_ = gameEntities_.size();
	}
}

void
LevelLocals::addWorld(GameEntity* world)
{
	gameEntities_.at(0) = world;
}

void
LevelLocals::removeEntity(GameEntity* entToRemove)
{
	// unlink if necessary
	if ( entToRemove->r.linked ) 
		SV_UnlinkEntity( entToRemove );

	// player ents stay there, but empty
	if( entToRemove->client_ )
	{
		int num = entToRemove->s.number;
		delete gameEntities_.at(num);
		gameEntities_.at(num) = 0;
		//delete clients_.at(num);
		//clients_.at(num) = 0;
	}
	// others are removed
	else
	{
		EntityList::iterator it = gameEntities_.begin();
		EntityList::iterator itEnd = gameEntities_.end();
		for( ; it != itEnd; ++it )
		{
			if( *it == entToRemove )
			{
				delete (*it);
				gameEntities_.erase(it);
				num_entities_ = gameEntities_.size();
				break;
			}
		}
	}
}

GameEntity*
LevelLocals::getEntity(int entNum)
{
	return reinterpret_cast<GameEntity*>(gameEntities_.at(entNum));
}

GameClient*
LevelLocals::getClient(int clientNum)
{
	return reinterpret_cast<GameClient*>(clients_.at(clientNum));
}

GameClient*
LevelLocals::createNewClient(int clientNum)
{
	GameClient* client = reinterpret_cast<GameClient*>(clients_.at(clientNum));
	if( client )
		delete client;
	client = new GameClient;
	client->ps_.clientNum = clientNum;

	clients_.at(clientNum) = client;

	return client;
}

void
LevelLocals::setPlayerEntity(GameEntity* playerEnt, int clientNum)
{
	GameEntity* ent = reinterpret_cast<GameEntity*>(gameEntities_.at(clientNum));
	if( ent )
		delete ent;

	playerEnt->client_ = reinterpret_cast<GameClient*>(clients_.at(clientNum));
	playerEnt->s.number = clientNum;
	gameEntities_.at(clientNum) = playerEnt;
}

void		
LevelLocals::createEntityFromSpawnFields(SpawnFieldHolder* sfh, spawn_t* s)
{
	GameEntity* ent;
	// not much distinction yet, just to give the idea.. needs to be done properly later
	if( !strcmp( sfh->classname_, "misc_vehicle" ) )
	{
		ent = new VehicleBase;
	}
	else
	{
		ent = new GameEntity;
	}
	ent->init(gameEntities_.size());

	// copy spawn fields
	setEntityFromSpawnFields(ent, sfh);
	
	// call spawn func
	s->spawn( ent );

	if( !ent->inuse_ )
	{
		delete ent;
		return;
	}

	addEntity( ent );
}

void
LevelLocals::createItemFromSpawnFields( SpawnFieldHolder* sfh, gitem_t* item )
{
	// create and init the ent
	GameEntity* ent = new GameItem;
	ent->init(gameEntities_.size());
	setEntityFromSpawnFields(ent, sfh);

	G_SpawnFloat( "random", "0", &ent->random_ );
	G_SpawnFloat( "wait", "0", &ent->wait_ );

	RegisterItem( item );
	ent->item_ = item;

	ent->nextthink_ = theLevel.time_ + FRAMETIME * 2;
	ent->setThink(new GameItem::Think_FinishSpawningItem);

	ent->physicsBounce_ = 0.50;	

	addEntity( ent );
}

void
LevelLocals::setEntityFromSpawnFields( GameEntity* ent, SpawnFieldHolder* sfh )
{
	ent->classname_ = sfh->classname_;
	VectorCopy( sfh->origin_, ent->s.origin );
	ent->model_ = sfh->model_;
	ent->model2_ = sfh->model2_;
	ent->spawnflags_ = sfh->spawnflags_;
	ent->speed_ = sfh->speed_;
	ent->target_ = sfh->target_;
	ent->targetname_ = sfh->targetname_;
	ent->message_ = sfh->message_;
	ent->team_ = sfh->team_;
	ent->wait_ = sfh->wait_;
	ent->random_ = sfh->random_;
	ent->count_ = sfh->count_;
	ent->health_ = sfh->health_;
	ent->damage_ = sfh->damage_;
	VectorCopy( sfh->angles_, ent->s.angles );
	ent->targetShaderName_ = sfh->targetShaderName_;
	ent->targetShaderNewName_ = sfh->targetShaderNewName_;
	ent->ent_category_ = sfh->ent_category_;
	ent->score_ = sfh->score_;

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
}

void
LevelLocals::setServerGameData()
{
	SV_LocateGameData( &gameEntities_, &clients_ );
}

GameEntity*
LevelLocals::spawnEntity()
{
	// find if there was an empty one
	size_t numEnts = gameEntities_.size();
	for( size_t i = MAX_CLIENTS + 1 ; i < numEnts ; i++ )
	{
		GameEntity* ent = reinterpret_cast<GameEntity*>(gameEntities_.at(i));
		if( !ent )
		{
			ent = new GameEntity;
			ent->init(i+1);
			return ent;
		}
		if( ent->inuse_ )
			continue;
		if( ent->freetime_ > startTime_ + 2000 || time_ - ent->freetime_ < 1000 )
			continue;
		ent->init(i);
		return ent;
	}
	// we reached the ent without finding an empty one
	if( i == ENTITYNUM_MAX_NORMAL ) 
	{
		for( i = 0; i < MAX_GENTITIES; i++ ) 
		{
			GameEntity* currEnt = getEntity(i);
			Com_Printf("%4i: %s\n", i, currEnt->classname_);// g_entities[i].classname);
		}
		Com_Error( ERR_DROP, "LevelLocals::spawnEntity: no free entities" );
	}
	// we reached the end but there is still space in the vector until we hit max_gentities
	else
	{
		GameEntity* ent = new GameEntity;
		ent->init(num_entities_);
		addEntity(ent);
		return ent;
	}
	return 0;
}




