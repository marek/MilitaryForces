#include "sv_server.h"
#include "../game/g_public.h"
#include "sv_client.h"
#include "server.h"



Server::Server() :
	state_(SS_DEAD),
	restarting_(false),
	serverId_(0),
	restartedServerId_(0),
	checksumFeed_(0),
	checksumFeedServerId_(0),
	snapshotCounter_(0),
	timeResidual_(0),
	nextFrameTime_(0),
	entityParsePoint_(nullptr),
	gameEntities_(nullptr),
	gameClients_(nullptr),
	restartTime_(0),
	time_(0)
{
	for(auto & model : models_)
		model = nullptr;

	for(auto & configstring : configstrings_)
		configstring = nullptr;

}

Server::~Server()
= default;

void
Server::clear()
{
	state_ = SS_DEAD;
	restarting_ = false;
	serverId_ = 0;
	restartedServerId_ = 0;
	checksumFeed_ = 0;
	checksumFeedServerId_ = 0;
	snapshotCounter_ = 0;
	timeResidual_ = 0;
	nextFrameTime_ = 0;
	entityParsePoint_ = nullptr;
	gameEntities_ = nullptr;
	gameClients_ = nullptr;
	restartTime_ = 0;
	time_ = 0;

	memset(	models_, 0, sizeof(models_) );
	memset(	configstrings_, 0, sizeof(configstrings_) );
	memset(	serverEntities_, 0, sizeof(serverEntities_) );
}

Server&
Server::getInstance()
{
	static Server theServer;
	return theServer;
}

void
Server::setGameData( EntityList* ents, ClientList* clients )
{
	gameEntities_ = ents;	
	gameClients_ = clients;
}

EntityBase*	
Server::getEntityByNum( int num )
{
	return gameEntities_->at(num);
}

ClientBase*		
Server::getClientByNum( int num )
{
	return gameClients_->at(num);
}

SV_Entity*
Server::SVEntityForGameEntity( EntityBase* gameEnt )
{
	if ( !gameEnt || gameEnt->s.number < 0 || gameEnt->s.number >= MAX_GENTITIES ) 
	{
		Common::error( ERR_DROP, "Server::getServerEntity: bad gameEnt" );
	}
	return &serverEntities_[gameEnt->s.number];
}

EntityBase*
Server::GameEntityForSVEntity( SV_Entity* svEnt )
{
	int	num;
	num = svEnt - serverEntities_;
	return getEntityByNum( num );
}




// -----------------------------


ServerStatic::Challenge::Challenge() :
	challenge_(0),
	clientChallenge_(0),
	time_(0),
	pingTime_(0),
	firstTime_(0),
	wasrefused_(false),
	connected_(false)
{
}

ServerStatic::ServerStatic() :
	initialized_(false),
	time_(0),
	snapFlagServerBit_(0),
	//clients_(0),
	numSnapshotEntities_(0),
	nextSnapshotEntities_(0),
	snapshotEntities_(nullptr),
	nextHeartbeatTime_(0)
{
}

ServerStatic::~ServerStatic()
= default;

ServerStatic&
ServerStatic::getInstance()
{
	static ServerStatic ss;
	return ss;
}

void
ServerStatic::clear()
{
	initialized_ = false;
	time_ = 0;
	snapFlagServerBit_ = 0;
	//clients_ = 0;
	clearClients();

	numSnapshotEntities_ = 0;
	nextSnapshotEntities_ = 0;
	snapshotEntities_ = nullptr;
	nextHeartbeatTime_ = 0;
	memset( masterResolveTime_, 0, sizeof(masterResolveTime_) );

	memset( challenges_, 0, sizeof(challenges_) );
	memset( &redirectAddress_, 0, sizeof(redirectAddress_) );	
	memset( &authorizeAddress_, 0, sizeof(authorizeAddress_) );
}

void
ServerStatic::clearClients()
{
	for(auto & svClient : svClients_)
	{
		if( svClient )
			svClient->free();
		delete svClient;
		svClient = nullptr;
	}
	svClients_.clear();
}

void
ServerStatic::initClients( int numClients )
{
	if( svClients_.size() )
		clearClients();

	svClients_.resize( numClients + 1 );
	for(auto & svClient : svClients_)
	{
		svClient = nullptr;
	}
}

// int newNumClients = new total number of clients
// int newClientsBeginAt = position from where to initialize to 0 (because its empty clients)
void
ServerStatic::changeMaxNumClients( int newNumClients, int newClientsBeginAt )
{
#pragma message("verify this works!!!!!")
	// can this be dangerous ? probably not as it is bounded beforehand anyway in ServerMain::changeMaxClients,
	// so it shouldnt throw away any old clients anyway
	svClients_.resize( newNumClients + 1 );
	for( size_t i = newClientsBeginAt; i < svClients_.size(); ++i )
	{
		svClients_.at(i) = nullptr;
	}

	//SV_Client* oldClients = reinterpret_cast<SV_Client*>(Hunk_AllocateTempMemory( count * sizeof(SV_Client) ));
	//// copy the clients to hunk memory
	//for ( i = 0 ; i < count ; i++ ) 
	//{
	//	if ( theSVS.clients_[i].state_ >= SV_Client::CS_CONNECTED ) 
	//		oldClients[i] = theSVS.clients_[i];
	//	else 
	//		Com_Memset(&oldClients[i], 0, sizeof(SV_Client));
	//}

	//// free old clients arrays
	//Z_Free( theSVS.clients_ );

	//// allocate New clients
	//theSVS.clients_ = reinterpret_cast<SV_Client*>(Z_Malloc ( sv_maxclients->integer * sizeof(SV_Client) ));
	//Com_Memset( theSVS.clients_, 0, sv_maxclients->integer * sizeof(SV_Client) );

	//// copy the clients over
	//for ( i = 0 ; i < count ; i++ )
	//{
	//	if ( oldClients[i].state_ >= SV_Client::CS_CONNECTED ) 
	//		theSVS.clients_[i] = oldClients[i];
	//}

	//// free the old clients on the hunk
	//Hunk_FreeTempMemory( oldClients );
}






