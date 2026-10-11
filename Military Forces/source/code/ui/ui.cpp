#include "ui.h"
#include "ui_local.h"








UserInterface::UserInterface() :
	initialized_(false)
{
}

UserInterface::~UserInterface()
= default;

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
	return Menus::consoleCommand( realTime );
}

void
UserInterface::drawConnectScreen( bool overlay ) const
{
	Menus::drawConnectScreen( overlay );
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



