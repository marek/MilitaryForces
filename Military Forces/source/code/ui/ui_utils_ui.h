#ifndef __UI_UTILS_UI_H__
#define __UI_UTILS_UI_H__

#include "ui_utils.h"



struct UI_UtilsUI : public UI_Utils
{
								UI_UtilsUI();
								~UI_UtilsUI();

	// ui specific
	bool						customChatUpdate( int key, int keyDown );
	void						customChatEnd( bool sendText );
	void						customChatDraw();

private:
	// disable
								UI_UtilsUI( UI_UtilsUI const& );
	UI_UtilsUI&					operator=( UI_UtilsUI const& );

	// data
	static const int			k_CHAT_X;
	static const int			k_CHAT_Y;
	static const int			k_CHAT_Y_IN_NEWHUD;
	static const int			k_CHAT_WIDTH;
	static const int			k_CHAT_HEIGHT;

	static const int			k_CHAT_OX;
	static const int			k_CHAT_OY;
	static const int			k_CHAT_AY;

	static const int			k_CURSOR_OX;
	static const int			k_CURSOR_OY;
	static const int			k_CURSOR_W;
	static const int			k_CURSOR_H;

};







#endif // __UI_UTILS_UI_H__

