#ifndef __UI_DISPLAYCONTEXT_CG_H__
#define __UI_DISPLAYCONTEXT_CG_H__

#include "../ui/ui_displaycontext.h"



struct UI_DisplayContextCG : public UI_DisplayContext
{
								UI_DisplayContextCG() {}
	~UI_DisplayContextCG() override {}

	void				drawHandlePic( float x, float y, float w, float h, qhandle_t asset ) override;
	void				drawText( float x, float y, float scale, const vec4_t color, const char *text, 
										  float adjust, int limit, int style ) override;
	int					textWidth( const char *text, float scale, int limit ) override;
	int					textHeight( const char *text, float scale, int limit ) override;
	void				fillRect( float x, float y, float w, float h, const vec4_t color ) override;
	void				drawRect( float x, float y, float w, float h, float size, const vec4_t color ) override;
	void				drawSides( float x, float y, float w, float h, float size ) override;
	void				drawTopBottom( float x, float y, float w, float h, float size ) override;
	void				ownerDrawItem( float x, float y, float w, float h, float text_x, float text_y, 
											   int ownerDraw, int ownerDrawFlags, int align, float special, float scale, 
											   const vec4_t color, qhandle_t shader, int textStyle, itemDef_t* item ) override;
	float				getCVarValue( const char *cvar ) override;
	int					feederCount( float feederID ) override;
	const char*			feederItemText( float feederID, int index, int column, qhandle_t *handle ) override;
	void				feederSelection( float feederID, int index ) override;
	int					ownerDrawWidth( int ownerDraw, float scale ) override;

};







#endif //__UI_DISPLAYCONTEXT_CG_H__