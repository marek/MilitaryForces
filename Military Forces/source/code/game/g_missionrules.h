#ifndef __G_MISSIONRULES_H__
#define __G_MISSIONRULES_H__

#include "bg_public.h"

class GameEntity;

// what makes a mission something to play: where players start, the objectives, the checkpoints, how it ends
class MissionRules
{
public:
	static void clear();
	static void begin( const char* id, const mission_overview_t& overview, const mission_rules_t& rules );
	static void notePlaced( const GameEntity* ent, const char* tag );
	static const mission_playerstart_t* startFor( int clientNum );
	static void chooseStartVehicle( int clientNum );
	static void applyStart( GameEntity* ent, const mission_playerstart_t& start );
	static void placedDestroyed( const GameEntity* ent );
	static void playerDestroyed( const GameEntity* ent );
	static void runFrame();
	static void listCommand();
	static void startCommand();
};

#endif
