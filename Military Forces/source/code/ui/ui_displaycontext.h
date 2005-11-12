#ifndef __UI_DISPLAYCONTEXT_H__
#define __UI_DISPLAYCONTEXT_H__


#include "..\game\q_shared.h"
#include "..\cgame\tr_types.h"
#include "..\ui\ui_shared.h"



struct UI_DisplayContext
{
							UI_DisplayContext() {}
	virtual					~UI_DisplayContext() {}

	virtual qhandle_t		registerShaderNoMip( const char* p );
	virtual void			setColor( const float* v );
	virtual void			drawHandlePic( float x, float y, float w, float h, qhandle_t asset ) = 0;
	virtual void			drawStretchPic( float x, float y, float w, float h, 
											float s1, float t1, float s2, float t2, qhandle_t hShader );
	virtual void			drawText( float x, float y, float scale, const vec4_t color, const char *text, 
									  float adjust, int limit, int style ) = 0;
	virtual int				textWidth( const char *text, float scale, int limit ) = 0;
	virtual int				textHeight( const char *text, float scale, int limit ) = 0;
	virtual qhandle_t		registerModel( const char *p );
	virtual void			modelBounds( qhandle_t model, vec3_t min, vec3_t max );
	virtual void			fillRect( float x, float y, float w, float h, const vec4_t color ) = 0;
	virtual void			drawRect( float x, float y, float w, float h, float size, const vec4_t color ) = 0;
	virtual void			drawSides( float x, float y, float w, float h, float size ) = 0;
	virtual void			drawTopBottom( float x, float y, float w, float h, float size ) = 0;
	virtual void			clearScene();
	virtual void			addRefEntityToScene( const refEntity_t *re );
	virtual void			renderScene( const refdef_t *fd );
	virtual void			registerFont( const char *pFontname, int pointSize, fontInfo_t *font );
	virtual void			ownerDrawItem( float x, float y, float w, float h, float text_x, float text_y, 
										   int ownerDraw, int ownerDrawFlags, int align, float special, float scale, 
										   const vec4_t color, qhandle_t shader, int textStyle, itemDef_t* item ) = 0;
//	virtual float			getValue( int ownerDraw );
	virtual bool			ownerDrawVisible( int flags ) { return false; }
	virtual void			runScript( char **p ) {}
//	virtual void			getTeamColor( vec4_t *color );
	virtual void			getCVarString( const char *cvar, char *buffer, int bufsize );
	virtual float			getCVarValue( const char *cvar ) = 0;
	virtual void			setCVar( const char *cvar, const char *value );
	virtual void			drawTextWithCursor( float x, float y, float scale, vec4_t const color, const char *text, 
												int cursorPos, char cursor, int limit, int style ) {}
	virtual void			setOverstrikeMode( bool b ) {}
	virtual bool			getOverstrikeMode() { return false; }
	virtual void			startLocalSound( sfxHandle_t sfx, int channelNum );
	virtual bool			ownerDrawHandleKey( int ownerDraw, int flags, float *special, int key ) { return false; }
	virtual int				feederCount( float feederID ) = 0;
	virtual const char*		feederItemText( float feederID, int index, int column, qhandle_t *handle ) = 0;
	virtual qhandle_t		feederItemImage( float feederID, int index ) { return 0; }
	virtual void			feederSelection( float feederID, int index ) = 0;
	virtual void			keynumToStringBuf( int keynum, char *buf, int buflen ) {}
	virtual void			getBindingBuf( int keynum, char *buf, int buflen ) {}
	virtual void			setBinding( int keynum, const char *binding ) {}
	virtual void			executeText( int exec_when, const char *text ) {}	
	virtual void			pause( bool b ) {}
	virtual int				ownerDrawWidth( int ownerDraw, float scale ) = 0;
	virtual sfxHandle_t		registerSound( const char *name, bool compressed );
	virtual void			startBackgroundTrack( const char *intro, const char *loop );
	virtual void			stopBackgroundTrack();
	virtual int				playCinematic( const char *name, float x, float y, float w, float h ) { return -1; }
	virtual void			stopCinematic( int handle ) {}
	virtual void			drawCinematic( int handle, float x, float y, float w, float h ) {}
	virtual void			runCinematicFrame( int handle ) {}

	// data
	float				yScale_;
	float				xScale_;
	float				bias_;
	int					realTime_;
	int					frameTime_;
	int					cursorX_;
	int					cursorY_;
	int					cursorEnum_;
	bool				debug_;

	cachedAssets_t		assets_;

	glconfig_t			glConfig_;
	qhandle_t			whiteShader_;
	qhandle_t			gradientImage_;
	qhandle_t			cursor_;
	float				fps_;

private:
	// disabled
						UI_DisplayContext( UI_DisplayContext const& );
	UI_DisplayContext&	operator=( UI_DisplayContext const& );
};




#endif // __UI_DISPLAYCONTEXT_H__

