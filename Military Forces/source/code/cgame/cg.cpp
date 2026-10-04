#include "cg.h"
#include "cg_local.h"

#pragma message( "+++TODO+++ decls (remove when finished porting to C++!)" )
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown();





ClientGame::ClientGame() :
	initialized_(false)
{

}

ClientGame::~ClientGame()
= default;

ClientGame&
ClientGame::getInstance()
{
	static ClientGame cg;
	return cg;
}

void
ClientGame::init( int serverMessageNum, 
				  int serverCommandSequence,
				  int clientNum )
{
	CG_Init( serverMessageNum, serverCommandSequence, clientNum );
	initialized_ = true;
}

void
ClientGame::shutdown()
{
	CG_Shutdown();
	initialized_ = false;
}

bool
ClientGame::consoleCommand() const
{
	return CGame::consoleCommand();
}

void
ClientGame::drawActiveFrame( int serverTime, 
							 stereoFrame_t stereoView, 
							 bool demoPlayback ) const
{
	CGame::drawActiveFrame( serverTime, stereoView, demoPlayback );
}

int 
ClientGame::crosshairPlayer() const
{
	return CGame::crosshairPlayer();
}

int
ClientGame::lastAttacker() const
{
	return CGame::lastAttacker();
}

void
ClientGame::keyEvent( int key, bool down ) const
{
	CGame::keyEvent( key, down );
}

void
ClientGame::mouseEvent( int dx, int dy ) const
{
	CGame::mouseEvent( dx, dy );
}

void
ClientGame::eventHandling( int type ) const
{
	CGame::eventHandling( type );
}

bool
ClientGame::isInitialized() const
{
	return initialized_;
}



