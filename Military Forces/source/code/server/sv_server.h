#ifndef __SV_SERVER_H__
#define __SV_SERVER_H__

#include "../qcommon/qcommon.h"
#include "../game/q_shared.h"
#include "sv_entity.h"
#include <vector>


struct ClientBase;
struct EntityBase;
struct cmodel_t;

struct Server
{
	typedef std::vector<ClientBase*> ClientList;
	typedef std::vector<EntityBase*> EntityList;

	enum eServerState
	{
		SS_DEAD,			// no map loaded
		SS_LOADING,			// spawning level entities
		SS_GAME				// actively running
	};

	static Server&	getInstance();

	EntityBase*		getEntityByNum( int num );
	ClientBase*		getClientByNum( int num );
	SV_Entity*		SVEntityForGameEntity( EntityBase* gameEnt );
	EntityBase*		GameEntityForSVEntity( SV_Entity* svEnt );

	void			clear();

	void			setGameData( EntityList* ents, ClientList* clients );

	eServerState	state_;
	bool			restarting_;				// if true, send configstring changes during SS_LOADING
	int				serverId_;					// changes each server start
	int				restartedServerId_;			// serverId before a map_restart
	int				checksumFeed_;				// the feed key that we use to compute the pure checksum strings
	int				checksumFeedServerId_;		// the serverId associated with the current checksumFeed (always <= serverId)
	int				snapshotCounter_;			// incremented for each snapshot built
	int				timeResidual_;				// <= 1000 / sv_frame->value
	int				nextFrameTime_;				// when time > nextFrameTime, process world
	cmodel_t*		models_[MAX_MODELS];
	char*			configstrings_[MAX_CONFIGSTRINGS];
	SV_Entity		serverEntities_[MAX_GENTITIES];

	char*			entityParsePoint_;			// used during game VM init

	// the game virtual machine will update these on init and changes
	EntityList*		gameEntities_;			// replacement for gentities

	ClientList*		gameClients_;				// replacement for gameClients

	int				restartTime_;

protected:
					Server();
					~Server();


};

struct SV_Client;

struct ServerStatic
{
	typedef std::vector<SV_Client*> SVClientList;

	struct Challenge
	{
					Challenge();

		netadr_t	adr_;
		int			challenge_;
		int			time_;				// time the last packet was sent to the autherize server
		int			pingTime_;			// time the challenge response was sent to client
		int			firstTime_;			// time the adr was first used, for authorize timeout checks
		bool		connected_;
	};

	static const int k_MAX_CHALLENGES	= 1024;

	static ServerStatic& getInstance();

	void			initClients( int numClients );
	void			clearClients();

	void			changeMaxNumClients( int newNumClients, int newClientsBeginAt );

	void			clear();

	bool			initialized_;				// sv_init has completed

	int				time_;						// will be strictly increasing across level changes

	int				snapFlagServerBit_;			// ^= SNAPFLAG_SERVERCOUNT every SV_SpawnServer()

//	SV_Client*		clients_;					// [sv_maxclients->integer];
	SVClientList	svClients_;					// replaces former clients_ member

	int				numSnapshotEntities_;		// sv_maxclients->integer*PACKET_BACKUP*MAX_PACKET_ENTITIES
	int				nextSnapshotEntities_;		// next snapshotEntities to use
	entityState_t*	snapshotEntities_;			// [numSnapshotEntities]
	int				nextHeartbeatTime_;
	Challenge		challenges_[k_MAX_CHALLENGES];// to prevent invalid IPs from connecting
	netadr_t		redirectAddress_;			// for rcon return messages

	netadr_t		authorizeAddress_;			// for rcon return messages

protected:
					ServerStatic();
					~ServerStatic();
};



#endif // __SV_SERVER_H__

