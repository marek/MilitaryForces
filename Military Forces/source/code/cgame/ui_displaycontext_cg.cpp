#include "ui_displaycontext_cg.h"
#include "cg_local.h"

const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle);
void CG_FeederSelection(float feederID, int index);
int CG_OwnerDrawWidth(int ownerDraw, float scale);





void
UI_DisplayContextCG::drawHandlePic( float x, float y, float w, float h, qhandle_t asset )
{
	CG_DrawPic( x, y, w, h, asset );
}

void
UI_DisplayContextCG::drawText( float x, float y, float scale, const vec4_t color, const char *text, 
							   float adjust, int limit, int style )
{
	CG_Text_Paint( x, y, scale, color, text, adjust, limit, style );
}

int
UI_DisplayContextCG::textWidth( const char *text, float scale, int limit )
{
	return CG_Text_Width( text, scale, limit );
}

int
UI_DisplayContextCG::textHeight( const char *text, float scale, int limit )
{
	return CG_Text_Height( text, scale, limit );
}

void
UI_DisplayContextCG::fillRect( float x, float y, float w, float h, const vec4_t color ) 
{
	CG_FillRect( x, y, w, h, color );
}

void
UI_DisplayContextCG::drawRect( float x, float y, float w, float h, float size, const vec4_t color )
{
	CG_DrawRect( x, y, w, h, size, color );
}

void
UI_DisplayContextCG::drawSides( float x, float y, float w, float h, float size )
{
	CG_DrawSides( x, y, w, h, size );
}

void
UI_DisplayContextCG::drawTopBottom( float x, float y, float w, float h, float size )
{
	CG_DrawTopBottom( x, y, w, h, size );
}

void
UI_DisplayContextCG::ownerDrawItem( float x, float y, float w, float h, float text_x, float text_y, 
					 			    int ownerDraw, int ownerDrawFlags, int align, float special, float scale, 
									const vec4_t color, qhandle_t shader, int textStyle, itemDef_t* item )
{
	CG_OwnerDraw( x, y, w, h, text_x, text_y, ownerDraw, ownerDrawFlags, align,
				  special, scale, color, shader, textStyle, item );
}

float
UI_DisplayContextCG::getCVarValue( const char *cvar )
{
	return CG_Cvar_Get( cvar );
}

int
UI_DisplayContextCG::feederCount( float feederID )
{
	return CG_FeederCount( feederID );
}

const char*
UI_DisplayContextCG::feederItemText( float feederID, int index, int column, qhandle_t *handle )
{	
	return CG_FeederItemText( feederID, index, column, handle );
}

void
UI_DisplayContextCG::feederSelection( float feederID, int index )
{
	CG_FeederSelection( feederID, index );
}

int
UI_DisplayContextCG::ownerDrawWidth( int ownerDraw, float scale )
{
	return CG_OwnerDrawWidth( ownerDraw, scale );
}







