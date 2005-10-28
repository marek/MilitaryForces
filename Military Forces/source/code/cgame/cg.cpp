#include "cg.h"

#pragma message( "+++TODO+++ decls (remove when finished porting to C++!)" )
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown();
bool CG_ConsoleCommand();
void CG_DrawActiveFrame( int serverTime, stereoFrame_t stereoView, bool demoPlayback );
int CG_CrosshairPlayer();
int CG_LastAttacker();
void CG_KeyEvent( int key, bool down );
void CG_MouseEvent( int dx, int dy );
void CG_EventHandling( int type );





ClientGame::ClientGame() :
	initialized_(false)
{

}

ClientGame::~ClientGame()
{

}

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
	return CG_ConsoleCommand();
}

void
ClientGame::drawActiveFrame( int serverTime, 
							 stereoFrame_t stereoView, 
							 bool demoPlayback ) const
{
	CG_DrawActiveFrame( serverTime, stereoView, demoPlayback );
}

int 
ClientGame::crosshairPlayer() const
{
	return CG_CrosshairPlayer();
}

int
ClientGame::lastAttacker() const
{
	return CG_LastAttacker();
}

void
ClientGame::keyEvent( int key, bool down ) const
{
	CG_KeyEvent( key, down );
}

void
ClientGame::mouseEvent( int dx, int dy ) const
{
	CG_MouseEvent( dx, dy );
}

void
ClientGame::eventHandling( int type ) const
{
	CG_EventHandling( type );
}

bool
ClientGame::isInitialized() const
{
	return initialized_;
}



