#ifndef GAME_HOST_H
#define GAME_HOST_H

#include <vector>
#include "../game/q_shared.h"

struct EntityBase;
struct ClientBase;

// What the game asks of the server it runs in: the world's collision and visibility, client data, config strings.

class GameHost
{
public:
	static void getUsercmd( int clientNum, usercmd_t *cmd );
	static bool getEntityToken( char *buffer, int bufferSize );
	static bool entityContact( const vec3_t mins, const vec3_t maxs, EntityBase* gameEnt, bool capsule );
	static int areaEntities( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
	static void linkEntity( EntityBase* ent );
	static void unlinkEntity( EntityBase* ent );
	static bool inPVS (const vec3_t p1, const vec3_t p2);
	static int pointContents( const vec3_t p, int passEntityNum );
	static void trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, bool capsule );
	static void getServerinfo( char *buffer, int bufferSize );
	static void setUserinfo( int index, const char *val );
	static void getUserinfo( int index, char *buffer, int bufferSize );
	static void getConfigstring( int index, char *buffer, int bufferSize );
	static void setConfigstring (int index, const char *val);
	static void sendServerCommand( int clientNum, const char *text );
	static void dropClient( int clientNum, const char *reason );
	static int botAllocateClient();
	static void botFreeClient( int clientNum );
	static void botUserCommand( int clientNum, usercmd_t *cmd );
	static void adjustAreaPortalState( EntityBase* gameEnt, int open );
	static void setBrushModel( EntityBase* gameEnt, const char *name );
	static void locateGameData( std::vector<EntityBase*>* gameEntities, std::vector<ClientBase*>* gameClients );
};

#endif
