#ifndef __UI_H__
#define __UI_H__





struct UserInterface
{
	enum MenuCommand
	{
		UIMENU_NONE,
		UIMENU_MAIN,
		UIMENU_INGAME,
		UIMENU_NEED_CD,
		UIMENU_BAD_CD_KEY,
		UIMENU_TEAM,
		UIMENU_POSTGAME
	};

	// the interface
	static UserInterface&		getInstance();

	void						init( bool inGameLoad );
	
	void						shutdown();

	void						keyEvent( int key, bool down ) const;

	void						mouseEvent( int dx, int dy ) const;

	void						refresh( int realTime ) const;

	bool						isFullScreen() const;

	void						setActiveMenu( MenuCommand menu ) const;

	bool						consoleCommand( int realTime ) const;

	void						drawConnectScreen( bool overlay ) const;

	bool						hasUniqueCDKey() const;

	// --

	bool						isInitialized() const;

private:

								UserInterface();
								~UserInterface();

	// disabled
								UserInterface( UserInterface const& );
	UserInterface&				operator=( UserInterface const& );

	// data
	bool						initialized_;
};







#endif // __UI_H__

