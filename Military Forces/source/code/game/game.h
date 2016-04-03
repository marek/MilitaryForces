#ifndef __GAME_H__
#define __GAME_H__




struct ServerGame
{
	// the interface
	static ServerGame&			getInstance();

	void						init( int levelTime, int randomSeed );

	void						shutdown();

	char*						clientConnect( int clientNum, bool firstTime, bool isBot ) const;

	void						clientBegin( int clientNum ) const;

	void						clientUserInfoChanged( int clientNum ) const;

	void						clientDisconnect( int clientNum ) const;

	void						clientCommand( int clientNum ) const;

	void						clientThink( int clientNum ) const;

	void						gameRunFrame( int levelTime ) const;

	bool						consoleCommand() const;

	//--

	bool						isInitialized() const;

private:
								ServerGame();
								~ServerGame();

	// disabled
								ServerGame( ServerGame const& );
	ServerGame&					operator=( ServerGame const& );

	bool						initialized_;
};






#endif // __GAME_H__