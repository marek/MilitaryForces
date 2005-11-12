#include "ui_utils_ui.h"
#include "ui_displaycontext_ui.h"
#include "ui_local.h"






const int UI_UtilsUI::k_CHAT_X = (128+4);
const int UI_UtilsUI::k_CHAT_Y = 4;
const int UI_UtilsUI::k_CHAT_Y_IN_NEWHUD = (480-k_CHAT_HEIGHT-4-32);
const int UI_UtilsUI::k_CHAT_WIDTH = (320+64-4-4);
const int UI_UtilsUI::k_CHAT_HEIGHT = 16;
const int UI_UtilsUI::k_CHAT_OX	= 4;
const int UI_UtilsUI::k_CHAT_OY	= 4;
const int UI_UtilsUI::k_CHAT_AY	= 9;
const int UI_UtilsUI::k_CURSOR_OX = 2;
const int UI_UtilsUI::k_CURSOR_OY = 2;
const int UI_UtilsUI::k_CURSOR_W = 8;
const int UI_UtilsUI::k_CURSOR_H = 12;



UI_UtilsUI::UI_UtilsUI()
{
	dc_ = new UI_DisplayContextUI;
}

UI_UtilsUI::~UI_UtilsUI()
{
	delete dc_;
}


bool 
UI_UtilsUI::customChatUpdate( int key, int keyDown )
{
	int i = uiInfo.customChat.cindex;
	int maxTextWidth = 0;

	bool store = false;

	// not active?
	if( !uiInfo.customChat.active )
	{
		return false;
	}

	// no-lock
	uiInfo.customChat.lockEntry = false;

	// extended?
	if( key & K_CHAR_FLAG )
	{
		// convert
		key &= ~K_CHAR_FLAG;

		store = true;
	}

	// intercept characters
	switch( key )
	{
	// ESCAPE
	case K_ESCAPE:
		// abort
		customChatEnd( false );
		break;

	// Return
	case 13:
	// Enter
	case 169:
		// say the current text
		customChatEnd( true );
		break;

	// Backspace
	case 127:
	// Del
	case 140:
		// at least one char?
		if( i > 0 && keyDown )
		{
			// remove the last char
			uiInfo.customChat.text[ i-1 ] = 0;
			uiInfo.customChat.cindex--;
		}
		break;
	}

	// valid normal range?
	if( key < 0x20 || key >= 0x7F )
	{
		store = false;
	}

	// work out text limit
	switch( uiInfo.customChat.mode )
	{
	case CCHAT_ALL:
		maxTextWidth = 320;
		break;
	case CCHAT_TEAM:
		maxTextWidth = 286;
		break;
	case CCHAT_TARGET:
		maxTextWidth = 280;
		break;
	case CCHAT_ATTACK:
		maxTextWidth = 268;
		break;
	}

	// hit the text limit?
	if( dc_->textWidth( uiInfo.customChat.text, CHAT_TEXT_SCALE, 0 ) > maxTextWidth )
	{
		store = false;

		// lock
		uiInfo.customChat.lockEntry = true;
	}

	// store the character?
	if( store )
	{
		uiInfo.customChat.text[ i ] = key;
		uiInfo.customChat.text[ i+1 ] = 0;
		uiInfo.customChat.cindex++;
	}

	return true;
}


void 
UI_UtilsUI::customChatEnd( bool sendText )
{
	char * pMode = NULL;

	// work out which say
	switch( uiInfo.customChat.mode )
	{
	case CCHAT_ALL:
		pMode = "say";
		break;
	case CCHAT_TEAM:
		pMode = "say_team";
		break;
	case CCHAT_TARGET:
		// not sure what to do here
		break;
	case CCHAT_ATTACK:
		// not sure what to do here
		break;
	}

	// send the text?
	if( sendText && pMode )
	{
		trap_Cmd_ExecuteText( EXEC_APPEND, va( "%s %s", pMode, uiInfo.customChat.text ) );
	}

	// disable chat
	uiInfo.customChat.active = false;

	// re-enable normal keyboard operation
	trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_UI );
	trap_Key_ClearStates();
}

void 
UI_UtilsUI::customChatDraw()
{
	int spacer1 = 0, spacer2 = 0;
	char * pMode = NULL;
	int x = k_CHAT_X, y = k_CHAT_Y;
	float alpha = 1.0f; 
	float fx = 1.0f;
	float textYAdjust = 0;

	// lock out when not active
	if( !uiInfo.customChat.active )
	{
		return;
	}

	// choose prefix
	switch( uiInfo.customChat.mode )
	{
	case CCHAT_ALL:
		pMode = "Chat: ";
		break;
	case CCHAT_TEAM:
		pMode = "Team Chat: ";
		break;
	case CCHAT_TARGET:
		pMode = "Target Chat: ";
		break;
	case CCHAT_ATTACK:
		pMode = "Attacker Chat: ";
		break;
	}

	// draw the chat background
	dc_->fillRect( x, y, k_CHAT_WIDTH, k_CHAT_HEIGHT, *createColourVector( 0,0.4f,0,0.75f * alpha,NULL ) );
	dc_->drawRect( x, y, k_CHAT_WIDTH, k_CHAT_HEIGHT, 1, *createColourVector( 0,0,0,0.75f * alpha,NULL ) );

	spacer1 = dc_->textWidth( pMode, CHAT_TEXT_SCALE, 0 );

	// apply height adjuster (to keep the text base constant)
	if( uiInfo.customChat.text[0] )
	{
		textYAdjust = dc_->textHeight( uiInfo.customChat.text, CHAT_TEXT_SCALE, 0 );
	}

	// draw the current text line
	drawStringNew( x+k_CHAT_OX, y+k_CHAT_OY, CHAT_TEXT_SCALE, *createColourVector(1,1,0,1 * alpha,NULL), pMode, 0, 0, ITEM_TEXTSTYLE_SHADOWED, LEFT_JUSTIFY );
	drawStringNew( x+k_CHAT_OX+spacer1, y+k_CHAT_OY+k_CHAT_AY-textYAdjust, CHAT_TEXT_SCALE, *createColourVector(1,1,1,1 * alpha,NULL), uiInfo.customChat.text, 0, 0, ITEM_TEXTSTYLE_SHADOWED, LEFT_JUSTIFY );

	spacer2 = dc_->textWidth( uiInfo.customChat.text, CHAT_TEXT_SCALE, 0 );

	// fade the cursor
	if( uiInfo.customChat.cursorDir )
	{
		// down
		uiInfo.customChat.cursorAlpha -= (4.0f * getFrameSecondFraction() );
		if( uiInfo.customChat.cursorAlpha <= 0.0f )
		{
			uiInfo.customChat.cursorAlpha = 0.0f;
			uiInfo.customChat.cursorDir = false;
		}
	}
	else
	{
		// down
		uiInfo.customChat.cursorAlpha += (4.0f * getFrameSecondFraction() );
		if( uiInfo.customChat.cursorAlpha >= 1.0f )
		{
			uiInfo.customChat.cursorAlpha = 1.0f;
			uiInfo.customChat.cursorDir = true;
		}
	}

	// work out cursor attributes
	fx = 1.0f;
	if( uiInfo.customChat.lockEntry )
	{
		fx = 0.0f;
	}

	// draw the cursor
	dc_->fillRect( x+spacer1+spacer2+k_CURSOR_OX+4, y+k_CURSOR_OY, k_CURSOR_W, k_CURSOR_H, 
		*createColourVector( 1,fx,fx,uiInfo.customChat.cursorAlpha * alpha,NULL ) );
}









