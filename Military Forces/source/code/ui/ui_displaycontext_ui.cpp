#include "ui_displaycontext_ui.h"
#include "ui_local.h"


void Text_Paint(float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit, int style);
int Text_Width(const char *text, float scale, int limit);
int Text_Height(const char *text, float scale, int limit);
void _UI_DrawRect( float x, float y, float width, float height, float size, const float *color );
void _UI_DrawSides(float x, float y, float w, float h, float size);
void _UI_DrawTopBottom(float x, float y, float w, float h, float size);
void UI_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, 
				  int align, float special, float scale, const vec4_t color, qhandle_t shader, 
				  int textStyle, itemDef_t * item);
float UI_GetValue(int ownerDraw);
bool UI_OwnerDrawVisible(int flags);
void UI_RunMenuScript(char **args);
void Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, 
						  char cursor, int limit, int style);
bool UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key);
int UI_FeederCount(float feederID);
const char *UI_FeederItemText(float feederID, int index, int column, qhandle_t *handle);
void Text_PaintWithCursor(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, 
						  int limit, int style);
qhandle_t UI_FeederItemImage(float feederID, int index);
void UI_FeederSelection(float feederID, int index);
void Text_PaintWithCursor(float x, float y, float scale, const vec4_t color, const char *text, int cursorPos, 
						  char cursor, int limit, int style); 
void UI_Pause(bool b);
int UI_OwnerDrawWidth(int ownerDraw, float scale);
int UI_PlayCinematic(const char *name, float x, float y, float w, float h);
void UI_StopCinematic(int handle);
void UI_DrawCinematic(int handle, float x, float y, float w, float h);
void UI_RunCinematicFrame(int handle);






void
UI_DisplayContextUI::drawHandlePic( float x, float y, float w, float h, qhandle_t asset )
{
	UI_DrawHandlePic( x, y, w, h, asset );
}

void
UI_DisplayContextUI::drawText( float x, float y, float scale, const vec4_t color, const char *text, 
							   float adjust, int limit, int style )
{
	Text_Paint( x, y, scale, color, text, adjust, limit, style );
}

int
UI_DisplayContextUI::textWidth( const char *text, float scale, int limit )
{
	return Text_Width( text, scale, limit );
}

int
UI_DisplayContextUI::textHeight( const char *text, float scale, int limit )
{
	return Text_Height( text, scale, limit );
}

void
UI_DisplayContextUI::fillRect( float x, float y, float w, float h, const vec4_t color ) 
{
	UI_FillRect( x, y, w, h, color );
}

void
UI_DisplayContextUI::drawRect( float x, float y, float w, float h, float size, const vec4_t color )
{
	_UI_DrawRect( x, y, w, h, size, color );
}

void
UI_DisplayContextUI::drawSides( float x, float y, float w, float h, float size )
{
	_UI_DrawSides( x, y, w, h, size );
}

void
UI_DisplayContextUI::drawTopBottom( float x, float y, float w, float h, float size )
{
	_UI_DrawTopBottom( x, y, w, h, size );
}

void
UI_DisplayContextUI::ownerDrawItem( float x, float y, float w, float h, float text_x, float text_y, 
					 			    int ownerDraw, int ownerDrawFlags, int align, float special, float scale, 
									const vec4_t color, qhandle_t shader, int textStyle, itemDef_t* item )
{
	UI_OwnerDraw( x, y, w, h, text_x, text_y, ownerDraw, ownerDrawFlags, 
				  align, special, scale, color, shader, textStyle, item );
}

//float
//UI_DisplayContextUI::getValue( int ownerDraw )
//{
//	return UI_GetValue( ownerDraw );
//}

bool
UI_DisplayContextUI::ownerDrawVisible( int flags )
{
	return UI_OwnerDrawVisible( flags );
}

void
UI_DisplayContextUI::runScript( char **p )
{
	UI_RunMenuScript( p );
}

//void
//UI_DisplayContextUI::getTeamColor( vec4_t *color )
//{
//	UI_GetTeamColor( color );
//}

float
UI_DisplayContextUI::getCVarValue( const char *cvar )
{
	return trap_Cvar_VariableValue( cvar );
}

void
UI_DisplayContextUI::drawTextWithCursor( float x, float y, float scale, vec4_t const color, const char *text, 
										 int cursorPos, char cursor, int limit, int style )
{
	Text_PaintWithCursor( x, y, scale, color, text, cursorPos, cursor, limit, style );
}

void
UI_DisplayContextUI::setOverstrikeMode( bool b )
{
	trap_Key_SetOverstrikeMode( b );
}

bool
UI_DisplayContextUI::getOverstrikeMode()
{
	return trap_Key_GetOverstrikeMode();
}

bool
UI_DisplayContextUI::ownerDrawHandleKey( int ownerDraw, int flags, float *special, int key )
{
	return UI_OwnerDrawHandleKey( ownerDraw, flags, special, key );
}

int
UI_DisplayContextUI::feederCount( float feederID )
{
	return UI_FeederCount( feederID );
}

const char*
UI_DisplayContextUI::feederItemText( float feederID, int index, int column, qhandle_t *handle )
{
	return UI_FeederItemText( feederID, index, column, handle );
}

qhandle_t
UI_DisplayContextUI::feederItemImage( float feederID, int index )
{
	return UI_FeederItemImage( feederID, index );
}

void
UI_DisplayContextUI::feederSelection( float feederID, int index )
{
	UI_FeederSelection( feederID, index );
}

void
UI_DisplayContextUI::keynumToStringBuf( int keynum, char *buf, int buflen )
{
	trap_Key_KeynumToStringBuf( keynum, buf, buflen );
}

void
UI_DisplayContextUI::getBindingBuf( int keynum, char *buf, int buflen )
{
	trap_Key_GetBindingBuf( keynum, buf, buflen );
}

void
UI_DisplayContextUI::setBinding( int keynum, const char *binding )
{
	trap_Key_SetBinding( keynum, binding );
}

void
UI_DisplayContextUI::executeText( int exec_when, const char *text )
{
	trap_Cmd_ExecuteText( exec_when, text );
}

void
UI_DisplayContextUI::pause( bool b )
{
	UI_Pause( b );
}

int
UI_DisplayContextUI::ownerDrawWidth( int ownerDraw, float scale )
{
	return UI_OwnerDrawWidth( ownerDraw, scale );
}

int
UI_DisplayContextUI::playCinematic( const char *name, float x, float y, float w, float h )
{
	return UI_PlayCinematic( name, x, y, w, h );
}

void
UI_DisplayContextUI::stopCinematic( int handle )
{
	UI_StopCinematic( handle );
}

void
UI_DisplayContextUI::drawCinematic( int handle, float x, float y, float w, float h )
{
	UI_DrawCinematic( handle, x, y, w, h );
}

void
UI_DisplayContextUI::runCinematicFrame( int handle )
{
	UI_RunCinematicFrame( handle );
}






























