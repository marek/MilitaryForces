#ifndef __UI_DISPLAYCONTEXT_UI_H__
#define __UI_DISPLAYCONTEXT_UI_H__

#include "ui_displaycontext.h"



struct UI_DisplayContextUI : public UI_DisplayContext
{
							UI_DisplayContextUI() {}
	virtual					~UI_DisplayContextUI() {}

	virtual void			drawHandlePic( float x, float y, float w, float h, qhandle_t asset );
	virtual void			drawText( float x, float y, float scale, const vec4_t color, const char *text, 
									  float adjust, int limit, int style );
	virtual int				textWidth( const char *text, float scale, int limit );
	virtual int				textHeight( const char *text, float scale, int limit );
	virtual void			fillRect( float x, float y, float w, float h, const vec4_t color );
	virtual void			drawRect( float x, float y, float w, float h, float size, const vec4_t color );
	virtual void			drawSides( float x, float y, float w, float h, float size );
	virtual void			drawTopBottom( float x, float y, float w, float h, float size );
	virtual void			ownerDrawItem( float x, float y, float w, float h, float text_x, float text_y, 
										   int ownerDraw, int ownerDrawFlags, int align, float special, float scale, 
										   const vec4_t color, qhandle_t shader, int textStyle, itemDef_t* item );
//	virtual float			getValue( int ownerDraw );
	virtual bool			ownerDrawVisible( int flags );
	virtual void			runScript( char **p );
//	virtual void			getTeamColor( vec4_t *color );
	virtual float			getCVarValue( const char *cvar );
	virtual void			drawTextWithCursor( float x, float y, float scale, vec4_t const color, const char *text, 
												int cursorPos, char cursor, int limit, int style );
	virtual void			setOverstrikeMode( bool b );
	virtual bool			getOverstrikeMode();
	virtual bool			ownerDrawHandleKey( int ownerDraw, int flags, float *special, int key );
	virtual int				feederCount( float feederID );
	virtual const char*		feederItemText( float feederID, int index, int column, qhandle_t *handle );
	virtual qhandle_t		feederItemImage( float feederID, int index );
	virtual void			feederSelection( float feederID, int index );
	virtual void			keynumToStringBuf( int keynum, char *buf, int buflen );
	virtual void			getBindingBuf( int keynum, char *buf, int buflen );
	virtual void			setBinding( int keynum, const char *binding );
	virtual void			executeText( int exec_when, const char *text );	
	virtual void			pause( bool b );
	virtual int				ownerDrawWidth( int ownerDraw, float scale );
	virtual int				playCinematic( const char *name, float x, float y, float w, float h );
	virtual void			stopCinematic( int handle );
	virtual void			drawCinematic( int handle, float x, float y, float w, float h );
	virtual void			runCinematicFrame( int handle );

};





#endif // __UI_DISPLAYCONTEXT_UI_H__