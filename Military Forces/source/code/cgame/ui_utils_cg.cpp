#include "ui_utils_cg.h"
#include "ui_displaycontext_cg.h"
#include "cg_local.h"



const int	UI_UtilsCG::k_TIMER_WIDTH = 60;
const int	UI_UtilsCG::k_TIMER_HEIGHT = 20;

const float	UI_UtilsCG::k_HUD_LABEL_SCALE =	0.25f;
const int	UI_UtilsCG::k_HUD_LABEL_TL = 2;
const int	UI_UtilsCG::k_HUD_LABEL_BR = (k_HUD_LABEL_TL*2);


UI_UtilsCG::UI_UtilsCG()
{
	dc_ = new UI_DisplayContextCG;
}

UI_UtilsCG::~UI_UtilsCG()
{
	delete dc_;
}

float
UI_UtilsCG::drawTimer( float y )
{
	char		*s;
	int			dx = 0, dy = 0, w = 0;
	int			mins, seconds, tens;
	int			msec;

	// calc milliseconds
	msec = cg.time - cgs.levelStartTime;

	// convert msec to time components
	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va( "%i:%i%i", mins, tens, seconds );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	// alter the draw position?
	if( CG_NewHUDActive() )
	{
		// fit around the New HUD (draw bottom middle-right)
		dx = 480;
		dy = 460;
	}
	else
	{
		// draw top right
		dx = 604;
		dy = y + 8;
	}

	// draw the timer background (TODO: possibly replace this with HUD when near/in the HUD)
	dc_->fillRect( dx-(k_TIMER_WIDTH/2), dy-4, k_TIMER_WIDTH, k_TIMER_HEIGHT, *createColourVector( 0,0.5f,0,0.75f,NULL ) );
	dc_->drawRect( dx-(k_TIMER_WIDTH/2), dy-4, k_TIMER_WIDTH, k_TIMER_HEIGHT, 1, *createColourVector( 0,0,0,1.0f,NULL ) );

	// draw timer string
	drawStringNew( dx, dy, 0.35f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWED, CENTRE_JUSTIFY );

	return y + k_TIMER_HEIGHT + 4;	
}

float
UI_UtilsCG::drawCountdownTimer( float y ) 
{
	char		*s;
	int			dx = 0, dy = 0, w = 0;
	int			mins, seconds, tens;
	int			msec;
	bool	flash = false;

	// no need for the countdown timer?
	if( cgs.timelimit == 0 )
	{
		return y;
	}

	// calc milliseconds
	msec = (cgs.timelimit * 60 * 1000) - (cg.time - cgs.levelStartTime) + 1000;	// the +1000 is just a correction offset
	
	// rubbish msec value? (i.e. negative)
	if( msec < 0 ) msec = 0;
	
	// convert msec to time components
	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	// flashing countdown? (within last minute of play)
	if( msec <= (60*1000) && (msec & 0x100) )
	{
		flash = true;
	}

	s = va( "%i:%i%i", mins, tens, seconds );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	// alter the draw position?
	if( CG_NewHUDActive() )
	{
		// fit around the New HUD (draw bottom middle-left)
		dx = 160;
		dy = 460;
	}
	else
	{
		// draw top right
		dx = 604;
		dy = y + 8;
	}

	// draw the timer background (TODO: possibly replace this with HUD when near/in the HUD)
	dc_->fillRect( dx-(k_TIMER_WIDTH/2), dy-4, k_TIMER_WIDTH, k_TIMER_HEIGHT, *createColourVector( 0.7f,0,0,0.75f,NULL ) );
	dc_->drawRect( dx-(k_TIMER_WIDTH/2), dy-4, k_TIMER_WIDTH, k_TIMER_HEIGHT, 1, *createColourVector( 0,0,0,1.0f,NULL ) );

	// draw timer string
	if( !flash )
	{
		drawStringNew( dx, dy, 0.35f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWED, CENTRE_JUSTIFY );
	}

	return y + k_TIMER_HEIGHT + 4;
}


void 
UI_UtilsCG::draw_HUD_Label( int x, int y, char * pText, float alpha )
{
	int labelWidth = 0;
	int labelHeight = 0;

	// get dimensions
	labelWidth = dc_->textWidth( pText, k_HUD_LABEL_SCALE, 0 );
	labelHeight = dc_->textHeight( pText, k_HUD_LABEL_SCALE, 0 );

	// draw background
	dc_->fillRect( x-(labelWidth/2)-k_HUD_LABEL_TL, y-k_HUD_LABEL_TL,
		labelWidth+k_HUD_LABEL_BR, labelHeight+k_HUD_LABEL_BR,
		*createColourVector( 0,0,0,(0.5f * alpha),NULL ) );

	dc_->drawRect( x-(labelWidth/2)-k_HUD_LABEL_TL, y-k_HUD_LABEL_TL,
		labelWidth+k_HUD_LABEL_BR, labelHeight+k_HUD_LABEL_BR,
		1, *createColourVector( 0,0,0,(0.75f * alpha),NULL ) );

	// draw timer string
	drawStringNew( x, (y+1), k_HUD_LABEL_SCALE, *createColourVector( 0,0,0,(1.0f * alpha),NULL ), pText, 0, 0, ITEM_TEXTSTYLE_NORMAL, CENTRE_JUSTIFY );
}


