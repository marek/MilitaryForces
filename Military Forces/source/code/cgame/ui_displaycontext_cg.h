#ifndef __UI_DISPLAYCONTEXT_CG_H__
#define __UI_DISPLAYCONTEXT_CG_H__

#include "../ui/ui_displaycontext.h"



struct UI_DisplayContextCG : public UI_DisplayContext
{
								UI_DisplayContextCG() {}
	virtual						~UI_DisplayContextCG() {}

	virtual void				drawHandlePic( float x, float y, float w, float h, qhandle_t asset );
	virtual void				drawText( float x, float y, float scale, const vec4_t color, const char *text, 
										  float adjust, int limit, int style );
	virtual int					textWidth( const char *text, float scale, int limit );
	virtual int					textHeight( const char *text, float scale, int limit );
	virtual void				fillRect( float x, float y, float w, float h, const vec4_t color );
	virtual void				drawRect( float x, float y, float w, float h, float size, const vec4_t color );
	virtual void				drawSides( float x, float y, float w, float h, float size );
	virtual void				drawTopBottom( float x, float y, float w, float h, float size );
	virtual void				ownerDrawItem( float x, float y, float w, float h, float text_x, float text_y, 
											   int ownerDraw, int ownerDrawFlags, int align, float special, float scale, 
											   const vec4_t color, qhandle_t shader, int textStyle, itemDef_t* item );
	virtual float				getCVarValue( const char *cvar );
	virtual int					feederCount( float feederID );
	virtual const char*			feederItemText( float feederID, int index, int column, qhandle_t *handle );
	virtual void				feederSelection( float feederID, int index );
	virtual int					ownerDrawWidth( int ownerDraw, float scale );

};







#endif //__UI_DISPLAYCONTEXT_CG_H__