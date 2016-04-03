#ifndef __CG_H__
#define __CG_H__


enum stereoFrame_t;


struct ClientGame
{
	// the interface
	static ClientGame&			getInstance();

	void						init( int serverMessageNum, 
									  int serverCommandSequence,
									  int clientNum );

	void						shutdown();

	bool						consoleCommand() const;

	void						drawActiveFrame( int serverTime, 
												 stereoFrame_t stereoView, 
												 bool demoPlayback ) const;

	int							crosshairPlayer() const;

	int							lastAttacker() const;
	
	void						keyEvent( int key, bool down ) const;

	void						mouseEvent( int dx, int dy ) const;
	
	void						eventHandling( int type ) const;

	// --

	bool						isInitialized() const;

private:

								ClientGame();
								~ClientGame();

	// disabled
								ClientGame( ClientGame const& );
	ClientGame&					operator=( ClientGame const& );

	bool						initialized_;
};





#endif // __CG_H__

