#include "game.h"


#pragma message( "+++TODO+++ decls (remove when finished porting to C++!)" )
void G_InitGame( int levelTime, int randomSeed );
void G_ShutdownGame();
char *MF_ClientConnect( int clientNum, bool firstTime, bool isBot );
void MF_ClientBegin( int clientNum );
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientCommand( int clientNum );
void ClientThink( int clientNum );
void G_RunFrame( int levelTime );
bool ConsoleCommand();







ServerGame::ServerGame() :
	initialized_(false)
{

}

ServerGame::~ServerGame()
{

}

ServerGame&
ServerGame::getInstance()
{
	static ServerGame sg;
	return sg;
}

void
ServerGame::init( int levelTime, int randomSeed )
{
	G_InitGame( levelTime, randomSeed );
	initialized_ = true;
}

void
ServerGame::shutdown()
{
	G_ShutdownGame();
	initialized_ = false;
}

char*
ServerGame::clientConnect( int clientNum, bool firstTime, bool isBot ) const
{
	return MF_ClientConnect( clientNum, firstTime, isBot );
}

void
ServerGame::clientBegin( int clientNum ) const
{
	MF_ClientBegin( clientNum );
}

void
ServerGame::clientUserInfoChanged( int clientNum ) const
{
	ClientUserinfoChanged( clientNum );
}

void
ServerGame::clientDisconnect( int clientNum ) const
{
	ClientDisconnect( clientNum );
}

void
ServerGame::clientCommand( int clientNum ) const
{
	ClientCommand( clientNum );
}

void
ServerGame::clientThink( int clientNum ) const
{
	ClientThink( clientNum );
}

void
ServerGame::gameRunFrame( int levelTime ) const
{
	G_RunFrame( levelTime );
}

bool
ServerGame::consoleCommand() const
{
	return ConsoleCommand();
}

bool
ServerGame::isInitialized() const
{
	return initialized_;
}








