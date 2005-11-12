#ifndef __UI_UTILS_CG_H__
#define __UI_UTILS_CG_H__

#include "../ui/ui_utils.h"



struct UI_UtilsCG : public UI_Utils
{
								UI_UtilsCG();
								~UI_UtilsCG();

	// cg specific
	float						drawTimer( float y );	
	float						drawCountdownTimer( float y );
	void						draw_HUD_Label( int x, int y, char * pText, float alpha );

private:
	// disable
								UI_UtilsCG( UI_UtilsCG const& );
	UI_UtilsCG&					operator=( UI_UtilsCG const& );

	static const int			k_TIMER_WIDTH;
	static const int			k_TIMER_HEIGHT;

	static const float			k_HUD_LABEL_SCALE;
	static const int			k_HUD_LABEL_TL;
	static const int			k_HUD_LABEL_BR;
};







#endif // __UI_UTILS_CG_H__

