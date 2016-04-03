#include "ui.h"



#pragma message( "+++TODO+++ decls (remove when finished porting to C++!)" )
void _UI_Init( bool );
void _UI_Shutdown();
void _UI_KeyEvent( int, bool );
void _UI_MouseEvent( int, int );
void _UI_Refresh( int );
bool _UI_IsFullscreen();
void _UI_SetActiveMenu( UserInterface::MenuCommand );
bool UI_ConsoleCommand( int );
void UI_DrawConnectScreen( bool overlay );





UserInterface::UserInterface() :
	initialized_(false)
{
}

UserInterface::~UserInterface()
{
}

UserInterface& 
UserInterface::getInstance()
{
	static UserInterface ui;
	return ui;
}

void
UserInterface::init( bool inGameLoad )
{
	_UI_Init( inGameLoad );	
	initialized_ = true;
}

void
UserInterface::shutdown()
{
	_UI_Shutdown();
	initialized_ = false;
}

void
UserInterface::keyEvent( int key, bool down ) const
{
	_UI_KeyEvent( key, down );
}

void
UserInterface::mouseEvent( int dx, int dy ) const
{
	_UI_MouseEvent( dx, dy );
}

void
UserInterface::refresh( int realTime ) const
{
	_UI_Refresh( realTime );
}

bool
UserInterface::isFullScreen() const
{
	return _UI_IsFullscreen();
}

void
UserInterface::setActiveMenu( MenuCommand menu ) const
{
	_UI_SetActiveMenu( menu );
}

bool
UserInterface::consoleCommand( int realTime ) const
{
	return UI_ConsoleCommand( realTime );
}

void
UserInterface::drawConnectScreen( bool overlay ) const
{
	UI_DrawConnectScreen( overlay );
}

//bool
//UserInterface::hasUniqueCDKey() const
//{
//	return false;
//}

bool
UserInterface::isInitialized() const
{
	return initialized_;
}



