#ifndef __UI_DISPLAYCONTEXT_UI_H__
#define __UI_DISPLAYCONTEXT_UI_H__

#include "ui_displaycontext.h"



struct UI_DisplayContextUI : public UI_DisplayContext
{
							UI_DisplayContextUI() {}
	~UI_DisplayContextUI() override {}

	void			drawHandlePic( float x, float y, float w, float h, qhandle_t asset ) override;
	void			drawText( float x, float y, float scale, const vec4_t color, const char *text, 
									  float adjust, int limit, int style ) override;
	int				textWidth( const char *text, float scale, int limit ) override;
	int				textHeight( const char *text, float scale, int limit ) override;
	void			fillRect( float x, float y, float w, float h, const vec4_t color ) override;
	void			drawRect( float x, float y, float w, float h, float size, const vec4_t color ) override;
	void			drawSides( float x, float y, float w, float h, float size ) override;
	void			drawTopBottom( float x, float y, float w, float h, float size ) override;
	void			ownerDrawItem( float x, float y, float w, float h, float text_x, float text_y, 
										   int ownerDraw, int ownerDrawFlags, int align, float special, float scale, 
										   const vec4_t color, qhandle_t shader, int textStyle, itemDef_t* item ) override;
//	virtual float			getValue( int ownerDraw );
	bool			ownerDrawVisible( int flags ) override;
	void			runScript( char **p ) override;
//	virtual void			getTeamColor( vec4_t *color );
	float			getCVarValue( const char *cvar ) override;
	void			drawTextWithCursor( float x, float y, float scale, vec4_t const color, const char *text, 
												int cursorPos, char cursor, int limit, int style ) override;
	void			setOverstrikeMode( bool b ) override;
	bool			getOverstrikeMode() override;
	bool			ownerDrawHandleKey( int ownerDraw, int flags, float *special, int key ) override;
	int				feederCount( float feederID ) override;
	const char*		feederItemText( float feederID, int index, int column, qhandle_t *handle ) override;
	qhandle_t		feederItemImage( float feederID, int index ) override;
	void			feederSelection( float feederID, int index ) override;
	void			keynumToStringBuf( int keynum, char *buf, int buflen ) override;
	void			getBindingBuf( int keynum, char *buf, int buflen ) override;
	void			setBinding( int keynum, const char *binding ) override;
	void			executeText( int exec_when, const char *text ) override;	
	void			pause( bool b ) override;
	int				ownerDrawWidth( int ownerDraw, float scale ) override;
	int				playCinematic( const char *name, float x, float y, float w, float h ) override;
	void			stopCinematic( int handle ) override;
	void			drawCinematic( int handle, float x, float y, float w, float h ) override;
	void			runCinematicFrame( int handle ) override;

};





#endif // __UI_DISPLAYCONTEXT_UI_H__