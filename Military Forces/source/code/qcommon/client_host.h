#ifndef CLIENT_HOST_H
#define CLIENT_HOST_H

#include "../game/q_shared.h"
#include "../cgame/tr_types.h"
#include "../cgame/cg_public.h"

// What the cgame and the menus ask of the client they run in: snapshots, commands, the loaded map, the display.

class ClientHost
{
public:
	static void addCgameCommand( const char *cmdName );
	static void addReliableCommand(const char *cmd, bool isDisconnectCmd = false);
	static void loadMap( const char *mapname );
	static void getGlconfig( glconfig_t *glconfig );
	static void getGameState( gameState_t *gs );
	static void getCurrentSnapshotNumber( int *snapshotNumber, int *serverTime );
	static bool getSnapshot( int snapshotNumber, snapshot_t *snapshot );
	static bool getServerCommand( int serverCommandNumber );
	static int getCurrentCmdNumber();
	static bool getUserCmd( int cmdNumber, usercmd_t *ucmd );
	static void setUserCmdValue( int userCmdValue, float sensitivityScale );
};

#endif
