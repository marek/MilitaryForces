/*
 * $Id: ui_training.c,v 1.1 2001-11-15 15:47:38 moorman Exp $
*/

/*
=======================================================================

TRAINNIG MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK					"menu/art/back"
#define ART_BACK_A					"menu/art/back_a"
#define ART_LOGO					"menu/art/main"
#define ART_SORRY					"menu/art/sorry"
	
#define ID_BACK				10


typedef struct {
	menuframework_s	menu;

	menubitmap_s	back;
} training_t;

static training_t	s_training;


/*
===============
Training_MenuEvent
===============
*/
static void Training_MenuEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {

	case ID_BACK:
		UI_PopMenu();
		break;

	}
}


/*
===============
TrainingMenu_Draw
===============
*/
static void TrainingMenu_Draw( void ) {
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	UI_DrawNamedPic( 150, 150, 400, 280, ART_SORRY );// until we got it

	// standard menu drawing
	Menu_Draw( &s_training.menu );
}

/*
===============
Training_MenuInit
===============
*/
static void Training_MenuInit( void ) {

	memset( &s_training, 0 ,sizeof(training_t) );

	Training_Cache();

	s_training.menu.fullscreen = qtrue;
	s_training.menu.wrapAround = qtrue;
	s_training.menu.draw = TrainingMenu_Draw;

	s_training.back.generic.type			= MTYPE_BITMAP;
	s_training.back.generic.name			= ART_BACK;
	s_training.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_training.back.generic.id				= ID_BACK;
	s_training.back.generic.callback		= Training_MenuEvent;
	s_training.back.generic.x				= 10;
	s_training.back.generic.y				= 180;
	s_training.back.width					= 128;
	s_training.back.height					= 16;
	s_training.back.focuspic				= ART_BACK_A;

	Menu_AddItem( &s_training.menu, &s_training.back );
}

/*
=================
Demos_Cache
=================
*/
void Training_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_LOGO );
	trap_R_RegisterShaderNoMip( ART_SORRY );
	trap_R_RegisterShaderNoMip( ART_BACK );
	trap_R_RegisterShaderNoMip( ART_BACK_A );
}

/*
===============
UI_DemosMenu
===============
*/
void UI_TrainingMenu( void ) {
	Training_MenuInit();
	UI_PushMenu( &s_training.menu );
}
