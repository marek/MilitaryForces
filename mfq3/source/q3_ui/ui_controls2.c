/*
 * $Id: ui_controls2.c,v 1.2 2001-12-22 17:54:13 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
/*
=======================================================================

CONTROLS MENU

=======================================================================
*/


#include "ui_local.h"

#define ART_LOGO			"menu/art/background_sub"
#define ART_BACK			"menu/art/back"
#define ART_BACK_A			"menu/art/back_a"


typedef struct {
	char	*command;
	char	*label;
	int		id;
	int		defaultbind1;
	int		defaultbind2;
	int		bind1;
	int		bind2;
} bind_t;

typedef struct
{
	char*	name;
	float	defaultvalue;
	float	value;	
} configcvar_t;

#define SAVE_NOOP		0
#define SAVE_YES		1
#define SAVE_NO			2
#define SAVE_CANCEL		3

// control sections
#define C_MOVEMENT		0
#define C_CONTROLS		1
#define C_WEAPONS		2
#define C_MISC			3
#define C_LOOK			4
#define C_MAX			5

#define ID_MOVEMENT		100
#define ID_CONTROLS		101
#define ID_WEAPONS		102
#define ID_MISC			103
#define ID_DEFAULTS		104
#define ID_BACK			105
#define ID_SAVEANDEXIT	106
#define ID_EXIT			107
#define ID_LOOK			108

// bindable actions
#define ID_SHOWSCORES	0
#define ID_FORWARD		1	
#define ID_BACKPEDAL	2
#define ID_MOVELEFT		3
#define ID_MOVERIGHT	4
#define ID_MOVEUP		5	
#define ID_MOVEDOWN		6
#define ID_LEFT			7	
#define ID_RIGHT		8	
#define ID_LOOKUP		9	
#define ID_LOOKDOWN		10
#define ID_ZOOMVIEW		11
#define ID_ATTACK		12
#define ID_ATTACK2		13
#define ID_WEAPPREV		14
#define ID_WEAPNEXT		15
#define ID_CHAT			16
#define ID_CHAT2		17
#define ID_CHAT3		18
#define ID_CHAT4		19

// now mfq3
#define ID_GEAR			20
#define ID_BRAKE		21
#define ID_FREECAM		22
#define ID_INCREASE		23
#define ID_DECREASE		24
#define ID_WEAPON_MG	25
#define ID_WEAPON_1		26
#define ID_WEAPON_2		27
#define ID_WEAPON_3		28
#define ID_WEAPON_4		29
#define ID_WEAPON_5		30
#define ID_WEAPON_6		31
#define ID_WEAPON_FLARE	32
#define ID_ENCYC		33
#define ID_VEHSEL		34
#define ID_GPS			35
#define ID_TOGGLEVIEW	36
#define ID_CAMERAUP		37
#define ID_CAMERADOWN	38
#define ID_ZOOMIN		39
#define ID_ZOOMOUT		40
#define ID_CONTACTTOWER 41
#define ID_RADAR		42
#define ID_EXTINFO		43
#define ID_RADARRANGE	44
#define ID_UNLOCK		45

// all others
#define ID_INVERTMOUSE	46
#define ID_MOUSESPEED	47
#define ID_JOYENABLE	48
#define ID_JOYTHRESHOLD	49
#define ID_SMOOTHMOUSE	50
#define ID_ADVANCEDCTRL	51


typedef struct
{
	menuframework_s		menu;

	menutext_s			banner;

	menutext_s			movement;
	menutext_s			controls;
	menutext_s			weapons;
	menutext_s			misc;
	menutext_s			look;

	menuaction_s		walkforward;
	menuaction_s		backpedal;
	menuaction_s		stepleft;
	menuaction_s		stepright;
	menuaction_s		moveup;
	menuaction_s		movedown;
	menuaction_s		turnleft;
	menuaction_s		turnright;
	menuaction_s		attack;
	menuaction_s		attack2;
	menuaction_s		prevweapon;
	menuaction_s		nextweapon;
	menuaction_s		lookup;
	menuaction_s		lookdown;
	menuaction_s		zoomview;
	menuradiobutton_s	invertmouse;
	menuslider_s		sensitivity;
	menuradiobutton_s	smoothmouse;
	menuaction_s		showscores;
	qboolean			changesmade;
	menuaction_s		chat;
	menuaction_s		chat2;
	menuaction_s		chat3;
	menuaction_s		chat4;
	menuradiobutton_s	joyenable;
	menuslider_s		joythreshold;
	int					section;
	qboolean			waitingforkey;

	// mfq3
	menuaction_s		gear;
	menuaction_s		brake;
	menuaction_s		freecam;
	menuaction_s		increase;
	menuaction_s		decrease;
	menuaction_s		weapon_mg;
	menuaction_s		weapon_1;
	menuaction_s		weapon_2;
	menuaction_s		weapon_3;
	menuaction_s		weapon_4;
	menuaction_s		weapon_5;
	menuaction_s		weapon_6;
	menuaction_s		weapon_flare;
	menuaction_s		encyc;
	menuaction_s		vehsel;
	menuaction_s		gps;
	menuaction_s		radar;
	menuaction_s		extinfo;
	menuaction_s		toggleview;
	menuaction_s		cameraup;
	menuaction_s		cameradown;
	menuaction_s		zoomin;
	menuaction_s		zoomout;
	menuaction_s		contacttower;
	menuradiobutton_s	advancedctrl;
	menuaction_s		radarrange;
	menuaction_s		unlock;

	menubitmap_s		back;
	menutext_s			name;
} controls_t; 	

static controls_t s_controls;

static vec4_t controls_binding_color  = {1.00f, 0.43f, 0.00f, 1.00f};

static bind_t g_bindings[] = 
{
	{"+scores",			"Show Scoreboard",	ID_SHOWSCORES,	K_TAB,			-1,		-1, -1},
	{"+forward", 		"Forward (Max)",	ID_FORWARD,		K_UPARROW,		-1,		-1, -1},
	{"+back", 			"Back/Stop (Min)",	ID_BACKPEDAL,	K_DOWNARROW,	-1,		-1, -1},
	{"+moveleft", 		"Turn left (tank)",	ID_MOVELEFT,	',',			-1,		-1, -1},
	{"+moveright", 		"Turn right (tank)",ID_MOVERIGHT,	'.',			-1,		-1, -1},
	{"+moveup",			"up / jump",		ID_MOVEUP,		K_SPACE,		-1,		-1, -1},
	{"+movedown",		"down / crouch",	ID_MOVEDOWN,	'c',			-1,		-1, -1},
	{"+left", 			"Left (view)",		ID_LEFT,		K_LEFTARROW,	-1,		-1, -1},
	{"+right", 			"Right (view)", 	ID_RIGHT,		K_RIGHTARROW,	-1,		-1, -1},
	{"+lookup", 		"Up (view)",		ID_LOOKUP,		K_PGDN,			-1,		-1, -1},
	{"+lookdown", 		"Down (view)",		ID_LOOKDOWN,	K_DEL,			-1,		-1, -1},
	{"+zoom", 			"Zoom View",		ID_ZOOMVIEW,	-1,				-1,		-1, -1},

	{"+attack", 		"Fire MG",			ID_ATTACK,		K_CTRL,			-1,		-1, -1},
	{"+button2", 		"Fire Main Weapon",	ID_ATTACK2,		-1,				-1,		-1, -1},
	{"weapprev",		"Previous Weapon",	ID_WEAPPREV,	'[',			-1,		-1, -1},
	{"weapnext", 		"Next Weapon",		ID_WEAPNEXT,	']',			-1,		-1, -1},
	{"messagemode", 	"Chat",				ID_CHAT,		't',			-1,		-1, -1},
	{"messagemode2", 	"Chat - Team",		ID_CHAT2,		-1,				-1,		-1, -1},
	{"messagemode3", 	"Chat - Target",	ID_CHAT3,		-1,				-1,		-1, -1},
	{"messagemode4", 	"Chat - Attacker",	ID_CHAT4,		-1,				-1,		-1, -1},

	{"+button7",		"Landing Gear",		ID_GEAR,		-1,				-1,		-1, -1},
	{"+button8",	 	"(Speed)Brakes",	ID_BRAKE,		-1,				-1,		-1, -1},
	{"+button9",	 	"Freelook Camera",	ID_FREECAM,		-1,				-1,		-1, -1},
	{"+button5", 		"Throttle Up",		ID_INCREASE,	-1,				-1,		-1, -1},
	{"+button6", 		"Throttle Down",	ID_DECREASE,	-1,				-1,		-1, -1},
	{"weapon 0",		"Select MG",		ID_WEAPON_MG,	'0',			-1,		-1, -1},
	{"weapon 1",		"Select Weapon 1",	ID_WEAPON_1,	'1',			-1,		-1, -1},
	{"weapon 2",		"Select Weapon 2",	ID_WEAPON_2,	'2',			-1,		-1, -1},
	{"weapon 3",		"Select Weapon 3",	ID_WEAPON_3,	'3',			-1,		-1, -1},
	{"weapon 4",		"Select Weapon 4",	ID_WEAPON_4,	'4',			-1,		-1, -1},
	{"weapon 5",		"Select Weapon 5",	ID_WEAPON_5,	'5',			-1,		-1, -1},
	{"weapon 6",		"Select Weapon 6",	ID_WEAPON_6,	'6',			-1,		-1, -1},
	{"+button10",		"Fire Flare",		ID_WEAPON_FLARE,'7',			-1,		-1, -1},
	{"encyclopedia",	"Encyclopedia",		ID_ENCYC,		-1,				-1,		-1, -1},
	{"vehicleselect",	"Vehicle Selection",ID_VEHSEL,		-1,				-1,		-1, -1},
	{"gps",				"GPS on/off",		ID_GPS,			-1,				-1,		-1, -1},
	{"toggleview",		"Toggle Camera",	ID_TOGGLEVIEW,	-1,			-1,		-1, -1},
	{"cameraup",		"Camera Up",		ID_CAMERAUP,	-1,				-1,		-1, -1},
	{"cameradown",		"Camera Down",		ID_CAMERADOWN,	-1,				-1,		-1, -1},
	{"zoomin",			"Zoom In",			ID_ZOOMIN,		-1,				-1,		-1, -1},
	{"zoomout",			"Zoom Out",			ID_ZOOMOUT,		-1,				-1,		-1, -1},
	{"contact_tower",	"Contact Tower",	ID_CONTACTTOWER,-1,				-1,		-1, -1},
	{"radar",			"RADAR on/off",		ID_RADAR,		-1,				-1,		-1, -1},
	{"extinfo",			"Ext.Info on/off",	ID_EXTINFO,		-1,				-1,		-1, -1},
	{"radarrange",		"Toggle RADAR Range",ID_RADARRANGE,	-1,				-1,		-1, -1},
	{"unlock",			"Unlock Target",	ID_UNLOCK,		-1,				-1,		-1, -1},
	
	{(char*)NULL,		(char*)NULL,		0,				-1,				-1,		-1,	-1},
};

static configcvar_t g_configcvars[] =
{
	{"m_pitch",			0,					0},
	{"sensitivity",		0,					0},
	{"in_joystick",		0,					0},
	{"joy_threshold",	0,					0},
	{"m_filter",		0,					0},
	{"cg_advanced",		0,					0},
	{NULL,				0,					0}
};

static menucommon_s *g_movement_controls[] =
{
	(menucommon_s *)&s_controls.advancedctrl,
	(menucommon_s *)&s_controls.walkforward,
	(menucommon_s *)&s_controls.backpedal,
	(menucommon_s *)&s_controls.increase,
	(menucommon_s *)&s_controls.decrease,
	(menucommon_s *)&s_controls.stepleft,      
	(menucommon_s *)&s_controls.stepright,     
//	(menucommon_s *)&s_controls.moveup,        
//	(menucommon_s *)&s_controls.movedown,      
	(menucommon_s *)&s_controls.turnleft,      
	(menucommon_s *)&s_controls.turnright,     
	(menucommon_s *)&s_controls.lookup,
	(menucommon_s *)&s_controls.lookdown,
	(menucommon_s *)&s_controls.gear,
	(menucommon_s *)&s_controls.brake,
	NULL
};

static menucommon_s *g_weapons_controls[] = {
	(menucommon_s *)&s_controls.attack,           
	(menucommon_s *)&s_controls.attack2,           
	(menucommon_s *)&s_controls.weapon_flare,
	(menucommon_s *)&s_controls.nextweapon,
	(menucommon_s *)&s_controls.prevweapon,
	(menucommon_s *)&s_controls.weapon_mg,
	(menucommon_s *)&s_controls.weapon_1,
	(menucommon_s *)&s_controls.weapon_2,
	(menucommon_s *)&s_controls.weapon_3,
	(menucommon_s *)&s_controls.weapon_4,
	(menucommon_s *)&s_controls.weapon_5,
	(menucommon_s *)&s_controls.weapon_6,
	NULL,
};

static menucommon_s *g_controls_controls[] = {
	(menucommon_s *)&s_controls.sensitivity,
	(menucommon_s *)&s_controls.smoothmouse,
	(menucommon_s *)&s_controls.invertmouse,
	(menucommon_s *)&s_controls.joyenable,
	(menucommon_s *)&s_controls.joythreshold,
	NULL,
};

static menucommon_s *g_misc_controls[] = {
	(menucommon_s *)&s_controls.vehsel,
	(menucommon_s *)&s_controls.encyc,
	(menucommon_s *)&s_controls.gps,
	(menucommon_s *)&s_controls.radar,
	(menucommon_s *)&s_controls.radarrange,
	(menucommon_s *)&s_controls.unlock,
	(menucommon_s *)&s_controls.extinfo,
	(menucommon_s *)&s_controls.contacttower,
	(menucommon_s *)&s_controls.showscores, 
	(menucommon_s *)&s_controls.chat,
	(menucommon_s *)&s_controls.chat2,
	(menucommon_s *)&s_controls.chat3,
	(menucommon_s *)&s_controls.chat4,
	NULL,
};

static menucommon_s *g_look_controls[] = {
	(menucommon_s *)&s_controls.freecam,
	(menucommon_s *)&s_controls.toggleview,
	(menucommon_s *)&s_controls.cameraup,
	(menucommon_s *)&s_controls.cameradown,
	(menucommon_s *)&s_controls.zoomin,
	(menucommon_s *)&s_controls.zoomout,
	(menucommon_s *)&s_controls.zoomview,
	NULL,
};

static menucommon_s **g_controls[] = {
	g_movement_controls,
	g_controls_controls,
	g_weapons_controls,
	g_misc_controls,
	g_look_controls
};

/*
=================
Controls_InitCvars
=================
*/
static void Controls_InitCvars( void )
{
	int				i;
	configcvar_t*	cvarptr;

	cvarptr = g_configcvars;
	for (i=0; ;i++,cvarptr++)
	{
		if (!cvarptr->name)
			break;

		// get current value
		cvarptr->value = trap_Cvar_VariableValue( cvarptr->name );

		// get default value
		trap_Cvar_Reset( cvarptr->name );
		cvarptr->defaultvalue = trap_Cvar_VariableValue( cvarptr->name );

		// restore current value
		trap_Cvar_SetValue( cvarptr->name, cvarptr->value );
	}
}

/*
=================
Controls_GetCvarDefault
=================
*/
static float Controls_GetCvarDefault( char* name )
{
	configcvar_t*	cvarptr;
	int				i;

	cvarptr = g_configcvars;
	for (i=0; ;i++,cvarptr++)
	{
		if (!cvarptr->name)
			return (0);

		if (!strcmp(cvarptr->name,name))
			break;
	}

	return (cvarptr->defaultvalue);
}

/*
=================
Controls_GetCvarValue
=================
*/
static float Controls_GetCvarValue( char* name )
{
	configcvar_t*	cvarptr;
	int				i;

	cvarptr = g_configcvars;
	for (i=0; ;i++,cvarptr++)
	{
		if (!cvarptr->name)
			return (0);

		if (!strcmp(cvarptr->name,name))
			break;
	}

	return (cvarptr->value);
}


/*
=================
Controls_Update
=================
*/
static void Controls_Update( void ) {
	int		i;
	int		j;
	int		y;
	menucommon_s	**controls;
	menucommon_s	*control;

	// disable all controls in all groups
	for( i = 0; i < C_MAX; i++ ) {
		controls = g_controls[i];
		for( j = 0;	control = controls[j]; j++ ) {
			control->flags |= (QMF_HIDDEN|QMF_INACTIVE);
		}
	}

	controls = g_controls[s_controls.section];

	// enable controls in active group (and count number of items for vertical centering)
	for( j = 0;	control = controls[j]; j++ ) {
		control->flags &= ~(QMF_GRAYED|QMF_HIDDEN|QMF_INACTIVE);
	}

	// position controls
	y = ( SCREEN_HEIGHT - j * SMALLCHAR_HEIGHT ) / 2;
	for( j = 0;	control = controls[j]; j++, y += SMALLCHAR_HEIGHT ) {
		control->x      = 340;
		control->y      = y;
		control->left   = 340 - 19*SMALLCHAR_WIDTH;
		control->right  = 340 + 21*SMALLCHAR_WIDTH;
		control->top    = y;
		control->bottom = y + SMALLCHAR_HEIGHT;
	}

	if( s_controls.waitingforkey ) {
		// disable everybody
		for( i = 0; i < s_controls.menu.nitems; i++ ) {
			((menucommon_s*)(s_controls.menu.items[i]))->flags |= QMF_GRAYED;
		}

		// enable action item
		((menucommon_s*)(s_controls.menu.items[s_controls.menu.cursor]))->flags &= ~QMF_GRAYED;

		// don't gray out player's name
		s_controls.name.generic.flags &= ~QMF_GRAYED;

		return;
	}

	// enable everybody
	for( i = 0; i < s_controls.menu.nitems; i++ ) {
		((menucommon_s*)(s_controls.menu.items[i]))->flags &= ~QMF_GRAYED;
	}

	// makes sure flags are right on the group selection controls
	s_controls.controls.generic.flags  &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
	s_controls.movement.generic.flags &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
	s_controls.weapons.generic.flags  &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
	s_controls.misc.generic.flags     &= ~(QMF_GRAYED|QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);

	s_controls.controls.generic.flags  |= QMF_PULSEIFFOCUS;
	s_controls.movement.generic.flags |= QMF_PULSEIFFOCUS;
	s_controls.weapons.generic.flags  |= QMF_PULSEIFFOCUS;
	s_controls.misc.generic.flags     |= QMF_PULSEIFFOCUS;

	// set buttons
	switch( s_controls.section ) {
	case C_MOVEMENT:
		s_controls.movement.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.movement.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;
	
	case C_CONTROLS:
		s_controls.controls.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.controls.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;
	
	case C_WEAPONS:
		s_controls.weapons.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.weapons.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;		

	case C_MISC:
		s_controls.misc.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.misc.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;

	case C_LOOK:
		s_controls.look.generic.flags &= ~QMF_PULSEIFFOCUS;
		s_controls.look.generic.flags |= (QMF_HIGHLIGHT|QMF_HIGHLIGHT_IF_FOCUS);
		break;

	}
}


/*
=================
Controls_DrawKeyBinding
=================
*/
static void Controls_DrawKeyBinding( void *self )
{
	menuaction_s*	a;
	int				x;
	int				y;
	int				b1;
	int				b2;
	qboolean		c;
	char			name[32];
	char			name2[32];

	a = (menuaction_s*) self;

	x =	a->generic.x;
	y = a->generic.y;

	c = (Menu_ItemAtCursor( a->generic.parent ) == a);

	b1 = g_bindings[a->generic.id].bind1;
	if (b1 == -1)
		strcpy(name,"???");
	else
	{
		trap_Key_KeynumToStringBuf( b1, name, 32 );
		Q_strupr(name);

		b2 = g_bindings[a->generic.id].bind2;
		if (b2 != -1)
		{
			trap_Key_KeynumToStringBuf( b2, name2, 32 );
			Q_strupr(name2);

			strcat( name, " or " );
			strcat( name, name2 );
		}
	}

	if (c)
	{
		UI_FillRect( a->generic.left, a->generic.top, a->generic.right-a->generic.left+1, a->generic.bottom-a->generic.top+1, color_darkred );//listbar_color ); 

		UI_DrawString( x - SMALLCHAR_WIDTH, y, g_bindings[a->generic.id].label, UI_RIGHT|UI_SMALLFONT, color_beige );//text_color_highlight );
		UI_DrawString( x + SMALLCHAR_WIDTH, y, name, UI_LEFT|UI_SMALLFONT|UI_PULSE, color_beige );//text_color_highlight );

		if (s_controls.waitingforkey)
		{
			UI_DrawChar( x, y, '=', UI_CENTER|UI_BLINK|UI_SMALLFONT, color_beige );//text_color_highlight);
			UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.80, "Waiting for new key ... ESCAPE to cancel", UI_SMALLFONT|UI_CENTER|UI_PULSE, colorWhite );
		}
		else
		{
			UI_DrawChar( x, y, 13, UI_CENTER|UI_BLINK|UI_SMALLFONT, color_beige );//text_color_highlight);
			UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.78, "Press ENTER or CLICK to change", UI_SMALLFONT|UI_CENTER, colorWhite );
			UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.82, "Press BACKSPACE to clear", UI_SMALLFONT|UI_CENTER, colorWhite );
		}
	}
	else
	{
		if (a->generic.flags & QMF_GRAYED)
		{
			UI_DrawString( x - SMALLCHAR_WIDTH, y, g_bindings[a->generic.id].label, UI_RIGHT|UI_SMALLFONT, text_color_disabled );
			UI_DrawString( x + SMALLCHAR_WIDTH, y, name, UI_LEFT|UI_SMALLFONT, text_color_disabled );
		}
		else
		{
			UI_DrawString( x - SMALLCHAR_WIDTH, y, g_bindings[a->generic.id].label, UI_RIGHT|UI_SMALLFONT, color_grey );//controls_binding_color );
			UI_DrawString( x + SMALLCHAR_WIDTH, y, name, UI_LEFT|UI_SMALLFONT, color_grey );//controls_binding_color );
		}
	}
}

/*
=================
Controls_StatusBar
=================
*/
static void Controls_StatusBar( void *self )
{
	UI_DrawString(SCREEN_WIDTH * 0.50, SCREEN_HEIGHT * 0.80, "Use Arrow Keys or CLICK to change", UI_SMALLFONT|UI_CENTER, colorWhite );
}


/*
=================
Controls_GetKeyAssignment
=================
*/
static void Controls_GetKeyAssignment (char *command, int *twokeys)
{
	int		count;
	int		j;
	char	b[256];

	twokeys[0] = twokeys[1] = -1;
	count = 0;

	for ( j = 0; j < 256; j++ )
	{
		trap_Key_GetBindingBuf( j, b, 256 );
		if ( *b == 0 ) {
			continue;
		}
		if ( !Q_stricmp( b, command ) ) {
			twokeys[count] = j;
			count++;
			if (count == 2)
				break;
		}
	}
}

/*
=================
Controls_GetConfig
=================
*/
static void Controls_GetConfig( void )
{
	int		i;
	int		twokeys[2];
	bind_t*	bindptr;

	// put the bindings into a local store
	bindptr = g_bindings;

	// iterate each command, get its numeric binding
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)
			break;

		Controls_GetKeyAssignment(bindptr->command, twokeys);

		bindptr->bind1 = twokeys[0];
		bindptr->bind2 = twokeys[1];
	}

	s_controls.invertmouse.curvalue  = Controls_GetCvarValue( "m_pitch" ) < 0;
	s_controls.smoothmouse.curvalue  = UI_ClampCvar( 0, 1, Controls_GetCvarValue( "m_filter" ) );
	s_controls.sensitivity.curvalue  = UI_ClampCvar( 2, 30, Controls_GetCvarValue( "sensitivity" ) );
	s_controls.joyenable.curvalue    = UI_ClampCvar( 0, 1, Controls_GetCvarValue( "in_joystick" ) );
	s_controls.joythreshold.curvalue = UI_ClampCvar( 0.05f, 0.75f, Controls_GetCvarValue( "joy_threshold" ) );
	s_controls.advancedctrl.curvalue = UI_ClampCvar( 0, 1, Controls_GetCvarValue( "cg_advanced" ) );
}

/*
=================
Controls_SetConfig
=================
*/
static void Controls_SetConfig( void )
{
	int		i;
	bind_t*	bindptr;

	// set the bindings from the local store
	bindptr = g_bindings;

	// iterate each command, get its numeric binding
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)
			break;

		if (bindptr->bind1 != -1)
		{	
			trap_Key_SetBinding( bindptr->bind1, bindptr->command );

			if (bindptr->bind2 != -1)
				trap_Key_SetBinding( bindptr->bind2, bindptr->command );
		}
	}

	if ( s_controls.invertmouse.curvalue )
		trap_Cvar_SetValue( "m_pitch", -fabs( trap_Cvar_VariableValue( "m_pitch" ) ) );
	else
		trap_Cvar_SetValue( "m_pitch", fabs( trap_Cvar_VariableValue( "m_pitch" ) ) );

	trap_Cvar_SetValue( "m_filter", s_controls.smoothmouse.curvalue );
	trap_Cvar_SetValue( "sensitivity", s_controls.sensitivity.curvalue );
	trap_Cvar_SetValue( "in_joystick", s_controls.joyenable.curvalue );
	trap_Cvar_SetValue( "joy_threshold", s_controls.joythreshold.curvalue );
	trap_Cvar_Set( "cg_advanced", va("%i",s_controls.advancedctrl.curvalue) );
	trap_Cmd_ExecuteText( EXEC_APPEND, "in_restart\n" );
}

/*
=================
Controls_SetDefaults
=================
*/
static void Controls_SetDefaults( void )
{
	int	i;
	bind_t*	bindptr;

	// set the bindings from the local store
	bindptr = g_bindings;

	// iterate each command, set its default binding
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)
			break;

		bindptr->bind1 = bindptr->defaultbind1;
		bindptr->bind2 = bindptr->defaultbind2;
	}

	s_controls.invertmouse.curvalue  = Controls_GetCvarDefault( "m_pitch" ) < 0;
	s_controls.smoothmouse.curvalue  = Controls_GetCvarDefault( "m_filter" );
	s_controls.sensitivity.curvalue  = Controls_GetCvarDefault( "sensitivity" );
	s_controls.joyenable.curvalue    = Controls_GetCvarDefault( "in_joystick" );
	s_controls.joythreshold.curvalue = Controls_GetCvarDefault( "joy_threshold" );
	s_controls.advancedctrl.curvalue = Controls_GetCvarDefault( "cg_advanced" );
}

/*
=================
Controls_MenuKey
=================
*/
static sfxHandle_t Controls_MenuKey( int key )
{
	int			id;
	int			i;
	qboolean	found;
	bind_t*		bindptr;
	found = qfalse;

	if (!s_controls.waitingforkey)
	{
		switch (key)
		{
			case K_BACKSPACE:
			case K_DEL:
			case K_KP_DEL:
				key = -1;
				break;
		
			case K_MOUSE2:
			case K_ESCAPE:
				if (s_controls.changesmade)
					Controls_SetConfig();
				goto ignorekey;	

			default:
				goto ignorekey;
		}
	}
	else
	{
		if (key & K_CHAR_FLAG)
			goto ignorekey;

		switch (key)
		{
			case K_ESCAPE:
				s_controls.waitingforkey = qfalse;
				Controls_Update();
				return (menu_out_sound);
	
			case '`':
				goto ignorekey;
		}
	}

	s_controls.changesmade = qtrue;
	
	if (key != -1)
	{
		// remove from any other bind
		bindptr = g_bindings;
		for (i=0; ;i++,bindptr++)
		{
			if (!bindptr->label)	
				break;

			if (bindptr->bind2 == key)
				bindptr->bind2 = -1;

			if (bindptr->bind1 == key)
			{
				bindptr->bind1 = bindptr->bind2;	
				bindptr->bind2 = -1;
			}
		}
	}

	// assign key to local store
	id      = ((menucommon_s*)(s_controls.menu.items[s_controls.menu.cursor]))->id;
	bindptr = g_bindings;
	for (i=0; ;i++,bindptr++)
	{
		if (!bindptr->label)	
			break;
		
		if (bindptr->id == id)
		{
			found = qtrue;
			if (key == -1)
			{
				if( bindptr->bind1 != -1 ) {
					trap_Key_SetBinding( bindptr->bind1, "" );
					bindptr->bind1 = -1;
				}
				if( bindptr->bind2 != -1 ) {
					trap_Key_SetBinding( bindptr->bind2, "" );
					bindptr->bind2 = -1;
				}
			}
			else if (bindptr->bind1 == -1) {
				bindptr->bind1 = key;
			}
			else if (bindptr->bind1 != key && bindptr->bind2 == -1) {
				bindptr->bind2 = key;
			}
			else
			{
				trap_Key_SetBinding( bindptr->bind1, "" );
				trap_Key_SetBinding( bindptr->bind2, "" );
				bindptr->bind1 = key;
				bindptr->bind2 = -1;
			}						
			break;
		}
	}				
		
	s_controls.waitingforkey = qfalse;

	if (found)
	{	
		Controls_Update();
		return (menu_out_sound);
	}

ignorekey:
	return Menu_DefaultKey( &s_controls.menu, key );
}

/*
=================
Controls_ResetDefaults_Action
=================
*/
static void Controls_ResetDefaults_Action( qboolean result ) {
	if( !result ) {
		return;
	}

	s_controls.changesmade = qtrue;
	Controls_SetDefaults();
	Controls_Update();
}

/*
=================
Controls_ResetDefaults_Draw
=================
*/
static void Controls_ResetDefaults_Draw( void ) {
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 0, "WARNING: This will reset all", UI_CENTER|UI_SMALLFONT, color_yellow );
	UI_DrawProportionalString( SCREEN_WIDTH/2, 356 + PROP_HEIGHT * 1, "controls to their default values.", UI_CENTER|UI_SMALLFONT, color_yellow );
}

/*
=================
Controls_MenuEvent
=================
*/
static void Controls_MenuEvent( void* ptr, int event )
{
	switch (((menucommon_s*)ptr)->id)
	{
		case ID_MOVEMENT:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_MOVEMENT; 
				Controls_Update();
			}
			break;

		case ID_CONTROLS:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_CONTROLS; 
				Controls_Update();
			}
			break;

		case ID_WEAPONS:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_WEAPONS; 
				Controls_Update();
			}
			break;

		case ID_MISC:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_MISC; 
				Controls_Update();
			}
			break;

		case ID_LOOK:
			if (event == QM_ACTIVATED)
			{
				s_controls.section = C_LOOK; 
				Controls_Update();
			}
			break;

		case ID_DEFAULTS:
			if (event == QM_ACTIVATED)
			{
				UI_ConfirmMenu( "SET TO DEFAULTS?", Controls_ResetDefaults_Draw, Controls_ResetDefaults_Action );
			}
			break;

		case ID_BACK:
			if (event == QM_ACTIVATED)
			{
				if (s_controls.changesmade)
					Controls_SetConfig();
				UI_PopMenu();
			}
			break;

		case ID_SAVEANDEXIT:
			if (event == QM_ACTIVATED)
			{
				Controls_SetConfig();
				UI_PopMenu();
			}
			break;

		case ID_EXIT:
			if (event == QM_ACTIVATED)
			{
				UI_PopMenu();
			}
			break;

		case ID_MOUSESPEED:
		case ID_INVERTMOUSE:
		case ID_SMOOTHMOUSE:
		case ID_JOYENABLE:
		case ID_JOYTHRESHOLD:
		case ID_ADVANCEDCTRL:
			if (event == QM_ACTIVATED)
			{
				s_controls.changesmade = qtrue;
			}
			break;		
	}
}

/*
=================
Controls_ActionEvent
=================
*/
static void Controls_ActionEvent( void* ptr, int event )
{
	if ((event == QM_ACTIVATED) && !s_controls.waitingforkey)
	{
		s_controls.waitingforkey = 1;
		Controls_Update();
	}
}

/*
===============
ControlsMenu_Draw
===============
*/
static void ControlsMenu_Draw( void ) {
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	// standard menu drawing
	Menu_Draw( &s_controls.menu );
}



/*
=================
Controls_MenuInit
=================
*/
static void Controls_MenuInit( void )
{
	static char playername[32];

	// zero set all our globals
	memset( &s_controls, 0 ,sizeof(controls_t) );

	Controls_Cache();

	s_controls.menu.key        = Controls_MenuKey;
	s_controls.menu.wrapAround = qtrue;
	s_controls.menu.fullscreen = qtrue;
	s_controls.menu.draw = ControlsMenu_Draw;

	s_controls.banner.generic.type	= MTYPE_BTEXT;
	s_controls.banner.generic.flags	= QMF_CENTER_JUSTIFY;
	s_controls.banner.generic.x		= 320;
	s_controls.banner.generic.y		= 80;
	s_controls.banner.string		= "CONTROLS";
	s_controls.banner.color			= color_white;
	s_controls.banner.style			= UI_CENTER;

	s_controls.controls.generic.type     = MTYPE_PTEXT;
	s_controls.controls.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.controls.generic.id	    = ID_CONTROLS;
	s_controls.controls.generic.callback	= Controls_MenuEvent;
	s_controls.controls.generic.x	    = 152;
	s_controls.controls.generic.y	    = 240 - 2 * PROP_HEIGHT;
	s_controls.controls.string			= "INPUT";
	s_controls.controls.style			= UI_RIGHT;
	s_controls.controls.color			= color_red;

	s_controls.movement.generic.type     = MTYPE_PTEXT;
	s_controls.movement.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.movement.generic.id	     = ID_MOVEMENT;
	s_controls.movement.generic.callback = Controls_MenuEvent;
	s_controls.movement.generic.x	     = 152;
	s_controls.movement.generic.y	     = 240 - PROP_HEIGHT;
	s_controls.movement.string			= "CONTROL";
	s_controls.movement.style			= UI_RIGHT;
	s_controls.movement.color			= color_red;

	s_controls.weapons.generic.type	    = MTYPE_PTEXT;
	s_controls.weapons.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.weapons.generic.id	    = ID_WEAPONS;
	s_controls.weapons.generic.callback	= Controls_MenuEvent;
	s_controls.weapons.generic.x	    = 152;
	s_controls.weapons.generic.y	    = 240;
	s_controls.weapons.string			= "SHOOT";
	s_controls.weapons.style			= UI_RIGHT;
	s_controls.weapons.color			= color_red;

	s_controls.misc.generic.type	 = MTYPE_PTEXT;
	s_controls.misc.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.misc.generic.id	     = ID_MISC;
	s_controls.misc.generic.callback = Controls_MenuEvent;
	s_controls.misc.generic.x		 = 152;
	s_controls.misc.generic.y		 = 240 + PROP_HEIGHT * 2;
	s_controls.misc.string			= "MISC";
	s_controls.misc.style			= UI_RIGHT;
	s_controls.misc.color			= color_red;

	s_controls.look.generic.type	 = MTYPE_PTEXT;
	s_controls.look.generic.flags    = QMF_RIGHT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.look.generic.id	     = ID_LOOK;
	s_controls.look.generic.callback = Controls_MenuEvent;
	s_controls.look.generic.x		 = 152;
	s_controls.look.generic.y		 = 240 + PROP_HEIGHT;
	s_controls.look.string			= "LOOK";
	s_controls.look.style			= UI_RIGHT;
	s_controls.look.color			= color_red;

	s_controls.back.generic.type	 = MTYPE_BITMAP;
	s_controls.back.generic.name     = ART_BACK;
	s_controls.back.generic.flags    = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_controls.back.generic.x		 = 20;
	s_controls.back.generic.y		 = 425;
	s_controls.back.generic.id	     = ID_BACK;
	s_controls.back.generic.callback = Controls_MenuEvent;
	s_controls.back.width  		     = 96;
	s_controls.back.height  		 = 32;
	s_controls.back.focuspic         = ART_BACK_A;

	s_controls.walkforward.generic.type	     = MTYPE_ACTION;
	s_controls.walkforward.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.walkforward.generic.callback  = Controls_ActionEvent;
	s_controls.walkforward.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.walkforward.generic.id 	     = ID_FORWARD;

	s_controls.backpedal.generic.type	   = MTYPE_ACTION;
	s_controls.backpedal.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.backpedal.generic.callback  = Controls_ActionEvent;
	s_controls.backpedal.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.backpedal.generic.id 	   = ID_BACKPEDAL;

	s_controls.stepleft.generic.type	  = MTYPE_ACTION;
	s_controls.stepleft.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.stepleft.generic.callback  = Controls_ActionEvent;
	s_controls.stepleft.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.stepleft.generic.id 		  = ID_MOVELEFT;

	s_controls.stepright.generic.type	   = MTYPE_ACTION;
	s_controls.stepright.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.stepright.generic.callback  = Controls_ActionEvent;
	s_controls.stepright.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.stepright.generic.id        = ID_MOVERIGHT;

	s_controls.moveup.generic.type	    = MTYPE_ACTION;
	s_controls.moveup.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.moveup.generic.callback  = Controls_ActionEvent;
	s_controls.moveup.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.moveup.generic.id        = ID_MOVEUP;

	s_controls.movedown.generic.type	  = MTYPE_ACTION;
	s_controls.movedown.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.movedown.generic.callback  = Controls_ActionEvent;
	s_controls.movedown.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.movedown.generic.id        = ID_MOVEDOWN;

	s_controls.turnleft.generic.type	  = MTYPE_ACTION;
	s_controls.turnleft.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.turnleft.generic.callback  = Controls_ActionEvent;
	s_controls.turnleft.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.turnleft.generic.id        = ID_LEFT;

	s_controls.turnright.generic.type	   = MTYPE_ACTION;
	s_controls.turnright.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.turnright.generic.callback  = Controls_ActionEvent;
	s_controls.turnright.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.turnright.generic.id        = ID_RIGHT;

	s_controls.attack.generic.type	    = MTYPE_ACTION;
	s_controls.attack.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.attack.generic.callback  = Controls_ActionEvent;
	s_controls.attack.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.attack.generic.id        = ID_ATTACK;

	s_controls.attack2.generic.type	     = MTYPE_ACTION;
	s_controls.attack2.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.attack2.generic.callback  = Controls_ActionEvent;
	s_controls.attack2.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.attack2.generic.id        = ID_ATTACK2;

	s_controls.prevweapon.generic.type	    = MTYPE_ACTION;
	s_controls.prevweapon.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.prevweapon.generic.callback  = Controls_ActionEvent;
	s_controls.prevweapon.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.prevweapon.generic.id        = ID_WEAPPREV;

	s_controls.nextweapon.generic.type	    = MTYPE_ACTION;
	s_controls.nextweapon.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.nextweapon.generic.callback  = Controls_ActionEvent;
	s_controls.nextweapon.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.nextweapon.generic.id        = ID_WEAPNEXT;

	s_controls.lookup.generic.type	    = MTYPE_ACTION;
	s_controls.lookup.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.lookup.generic.callback  = Controls_ActionEvent;
	s_controls.lookup.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.lookup.generic.id        = ID_LOOKUP;

	s_controls.lookdown.generic.type	  = MTYPE_ACTION;
	s_controls.lookdown.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.lookdown.generic.callback  = Controls_ActionEvent;
	s_controls.lookdown.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.lookdown.generic.id        = ID_LOOKDOWN;

	s_controls.zoomview.generic.type	  = MTYPE_ACTION;
	s_controls.zoomview.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.zoomview.generic.callback  = Controls_ActionEvent;
	s_controls.zoomview.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.zoomview.generic.id        = ID_ZOOMVIEW;

	s_controls.showscores.generic.type	    = MTYPE_ACTION;
	s_controls.showscores.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.showscores.generic.callback  = Controls_ActionEvent;
	s_controls.showscores.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.showscores.generic.id        = ID_SHOWSCORES;

	s_controls.invertmouse.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.invertmouse.generic.flags	 = QMF_SMALLFONT;
	s_controls.invertmouse.generic.x	     = SCREEN_WIDTH/2;
	s_controls.invertmouse.generic.name	     = "Invert Mouse";
	s_controls.invertmouse.generic.id        = ID_INVERTMOUSE;
	s_controls.invertmouse.generic.callback  = Controls_MenuEvent;
	s_controls.invertmouse.generic.statusbar = Controls_StatusBar;

	s_controls.smoothmouse.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.smoothmouse.generic.flags	 = QMF_SMALLFONT;
	s_controls.smoothmouse.generic.x	     = SCREEN_WIDTH/2;
	s_controls.smoothmouse.generic.name	     = "Smooth Mouse";
	s_controls.smoothmouse.generic.id        = ID_SMOOTHMOUSE;
	s_controls.smoothmouse.generic.callback  = Controls_MenuEvent;
	s_controls.smoothmouse.generic.statusbar = Controls_StatusBar;

	s_controls.sensitivity.generic.type	     = MTYPE_SLIDER;
	s_controls.sensitivity.generic.x		 = SCREEN_WIDTH/2;
	s_controls.sensitivity.generic.flags	 = QMF_SMALLFONT;
	s_controls.sensitivity.generic.name	     = "Mouse Speed";
	s_controls.sensitivity.generic.id 	     = ID_MOUSESPEED;
	s_controls.sensitivity.generic.callback  = Controls_MenuEvent;
	s_controls.sensitivity.minvalue		     = 2;
	s_controls.sensitivity.maxvalue		     = 30;
	s_controls.sensitivity.generic.statusbar = Controls_StatusBar;

	s_controls.advancedctrl.generic.type     = MTYPE_RADIOBUTTON;
	s_controls.advancedctrl.generic.flags	 = QMF_SMALLFONT;
	s_controls.advancedctrl.generic.x	     = SCREEN_WIDTH/2;
	s_controls.advancedctrl.generic.name     = "Advanced Controls";
	s_controls.advancedctrl.generic.id       = ID_ADVANCEDCTRL;
	s_controls.advancedctrl.generic.callback = Controls_MenuEvent;
	s_controls.advancedctrl.generic.statusbar= Controls_StatusBar;

	s_controls.chat.generic.type	  = MTYPE_ACTION;
	s_controls.chat.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chat.generic.callback  = Controls_ActionEvent;
	s_controls.chat.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chat.generic.id        = ID_CHAT;

	s_controls.chat2.generic.type	   = MTYPE_ACTION;
	s_controls.chat2.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chat2.generic.callback  = Controls_ActionEvent;
	s_controls.chat2.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chat2.generic.id        = ID_CHAT2;

	s_controls.chat3.generic.type	   = MTYPE_ACTION;
	s_controls.chat3.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chat3.generic.callback  = Controls_ActionEvent;
	s_controls.chat3.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chat3.generic.id        = ID_CHAT3;

	s_controls.chat4.generic.type	   = MTYPE_ACTION;
	s_controls.chat4.generic.flags     = QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.chat4.generic.callback  = Controls_ActionEvent;
	s_controls.chat4.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.chat4.generic.id        = ID_CHAT4;

	s_controls.joyenable.generic.type      = MTYPE_RADIOBUTTON;
	s_controls.joyenable.generic.flags	   = QMF_SMALLFONT;
	s_controls.joyenable.generic.x	       = SCREEN_WIDTH/2;
	s_controls.joyenable.generic.name	   = "Joystick";
	s_controls.joyenable.generic.id        = ID_JOYENABLE;
	s_controls.joyenable.generic.callback  = Controls_MenuEvent;
	s_controls.joyenable.generic.statusbar = Controls_StatusBar;

	s_controls.joythreshold.generic.type	  = MTYPE_SLIDER;
	s_controls.joythreshold.generic.x		  = SCREEN_WIDTH/2;
	s_controls.joythreshold.generic.flags	  = QMF_SMALLFONT;
	s_controls.joythreshold.generic.name	  = "Joystick Threshold";
	s_controls.joythreshold.generic.id 	      = ID_JOYTHRESHOLD;
	s_controls.joythreshold.generic.callback  = Controls_MenuEvent;
	s_controls.joythreshold.minvalue		  = 0.05f;
	s_controls.joythreshold.maxvalue		  = 0.75f;
	s_controls.joythreshold.generic.statusbar = Controls_StatusBar;

	s_controls.gear.generic.type			= MTYPE_ACTION;
	s_controls.gear.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.gear.generic.callback		= Controls_ActionEvent;
	s_controls.gear.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_controls.gear.generic.id				= ID_GEAR;

	s_controls.brake.generic.type			= MTYPE_ACTION;
	s_controls.brake.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.brake.generic.callback		= Controls_ActionEvent;
	s_controls.brake.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_controls.brake.generic.id				= ID_BRAKE;

	s_controls.freecam.generic.type			= MTYPE_ACTION;
	s_controls.freecam.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.freecam.generic.callback		= Controls_ActionEvent;
	s_controls.freecam.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.freecam.generic.id			= ID_FREECAM;

	s_controls.increase.generic.type		= MTYPE_ACTION;
	s_controls.increase.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.increase.generic.callback	= Controls_ActionEvent;
	s_controls.increase.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.increase.generic.id			= ID_INCREASE;

	s_controls.decrease.generic.type		= MTYPE_ACTION;
	s_controls.decrease.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.decrease.generic.callback	= Controls_ActionEvent;
	s_controls.decrease.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.decrease.generic.id			= ID_DECREASE;

	s_controls.weapon_mg.generic.type		= MTYPE_ACTION;
	s_controls.weapon_mg.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.weapon_mg.generic.callback	= Controls_ActionEvent;
	s_controls.weapon_mg.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.weapon_mg.generic.id			= ID_WEAPON_MG;

	s_controls.weapon_1.generic.type		= MTYPE_ACTION;
	s_controls.weapon_1.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.weapon_1.generic.callback	= Controls_ActionEvent;
	s_controls.weapon_1.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.weapon_1.generic.id			= ID_WEAPON_1;

	s_controls.weapon_2.generic.type		= MTYPE_ACTION;
	s_controls.weapon_2.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.weapon_2.generic.callback	= Controls_ActionEvent;
	s_controls.weapon_2.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.weapon_2.generic.id			= ID_WEAPON_2;

	s_controls.weapon_3.generic.type		= MTYPE_ACTION;
	s_controls.weapon_3.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.weapon_3.generic.callback	= Controls_ActionEvent;
	s_controls.weapon_3.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.weapon_3.generic.id			= ID_WEAPON_3;

	s_controls.weapon_4.generic.type		= MTYPE_ACTION;
	s_controls.weapon_4.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.weapon_4.generic.callback	= Controls_ActionEvent;
	s_controls.weapon_4.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.weapon_4.generic.id			= ID_WEAPON_4;

	s_controls.weapon_5.generic.type		= MTYPE_ACTION;
	s_controls.weapon_5.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.weapon_5.generic.callback	= Controls_ActionEvent;
	s_controls.weapon_5.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.weapon_5.generic.id			= ID_WEAPON_5;

	s_controls.weapon_6.generic.type		= MTYPE_ACTION;
	s_controls.weapon_6.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.weapon_6.generic.callback	= Controls_ActionEvent;
	s_controls.weapon_6.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.weapon_6.generic.id			= ID_WEAPON_6;

	s_controls.weapon_flare.generic.type	= MTYPE_ACTION;
	s_controls.weapon_flare.generic.flags	= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.weapon_flare.generic.callback= Controls_ActionEvent;
	s_controls.weapon_flare.generic.ownerdraw= Controls_DrawKeyBinding;
	s_controls.weapon_flare.generic.id		= ID_WEAPON_FLARE;

	s_controls.encyc.generic.type			= MTYPE_ACTION;
	s_controls.encyc.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.encyc.generic.callback		= Controls_ActionEvent;
	s_controls.encyc.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_controls.encyc.generic.id				= ID_ENCYC;

	s_controls.vehsel.generic.type			= MTYPE_ACTION;
	s_controls.vehsel.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.vehsel.generic.callback		= Controls_ActionEvent;
	s_controls.vehsel.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_controls.vehsel.generic.id			= ID_VEHSEL;

	s_controls.gps.generic.type				= MTYPE_ACTION;
	s_controls.gps.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.gps.generic.callback			= Controls_ActionEvent;
	s_controls.gps.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_controls.gps.generic.id				= ID_GPS;

	s_controls.radar.generic.type			= MTYPE_ACTION;
	s_controls.radar.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.radar.generic.callback		= Controls_ActionEvent;
	s_controls.radar.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_controls.radar.generic.id				= ID_RADAR;

	s_controls.radarrange.generic.type		= MTYPE_ACTION;
	s_controls.radarrange.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.radarrange.generic.callback	= Controls_ActionEvent;
	s_controls.radarrange.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.radarrange.generic.id		= ID_RADARRANGE;

	s_controls.unlock.generic.type			= MTYPE_ACTION;
	s_controls.unlock.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.unlock.generic.callback		= Controls_ActionEvent;
	s_controls.unlock.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_controls.unlock.generic.id			= ID_UNLOCK;

	s_controls.extinfo.generic.type			= MTYPE_ACTION;
	s_controls.extinfo.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.extinfo.generic.callback		= Controls_ActionEvent;
	s_controls.extinfo.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.extinfo.generic.id			= ID_EXTINFO;

	s_controls.toggleview.generic.type		= MTYPE_ACTION;
	s_controls.toggleview.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.toggleview.generic.callback	= Controls_ActionEvent;
	s_controls.toggleview.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.toggleview.generic.id		= ID_TOGGLEVIEW;

	s_controls.cameraup.generic.type		= MTYPE_ACTION;
	s_controls.cameraup.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.cameraup.generic.callback	= Controls_ActionEvent;
	s_controls.cameraup.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.cameraup.generic.id			= ID_CAMERAUP;

	s_controls.cameradown.generic.type		= MTYPE_ACTION;
	s_controls.cameradown.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.cameradown.generic.callback	= Controls_ActionEvent;
	s_controls.cameradown.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.cameradown.generic.id		= ID_CAMERADOWN;

	s_controls.zoomin.generic.type			= MTYPE_ACTION;
	s_controls.zoomin.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.zoomin.generic.callback		= Controls_ActionEvent;
	s_controls.zoomin.generic.ownerdraw		= Controls_DrawKeyBinding;
	s_controls.zoomin.generic.id			= ID_ZOOMIN;

	s_controls.zoomout.generic.type			= MTYPE_ACTION;
	s_controls.zoomout.generic.flags		= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.zoomout.generic.callback		= Controls_ActionEvent;
	s_controls.zoomout.generic.ownerdraw	= Controls_DrawKeyBinding;
	s_controls.zoomout.generic.id			= ID_ZOOMOUT;

	s_controls.contacttower.generic.type	= MTYPE_ACTION;
	s_controls.contacttower.generic.flags	= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS|QMF_GRAYED|QMF_HIDDEN;
	s_controls.contacttower.generic.callback = Controls_ActionEvent;
	s_controls.contacttower.generic.ownerdraw = Controls_DrawKeyBinding;
	s_controls.contacttower.generic.id		= ID_CONTACTTOWER;

	s_controls.name.generic.type	= MTYPE_PTEXT;
	s_controls.name.generic.flags	= QMF_CENTER_JUSTIFY|QMF_INACTIVE;
	s_controls.name.generic.x		= 320;
	s_controls.name.generic.y		= 430;
	s_controls.name.string			= playername;
	s_controls.name.style			= UI_CENTER;
	s_controls.name.color			= color_grey;//text_color_normal;

	Menu_AddItem( &s_controls.menu, &s_controls.banner );
	Menu_AddItem( &s_controls.menu, &s_controls.name );

	Menu_AddItem( &s_controls.menu, &s_controls.controls );
	Menu_AddItem( &s_controls.menu, &s_controls.movement );
	Menu_AddItem( &s_controls.menu, &s_controls.weapons );
	Menu_AddItem( &s_controls.menu, &s_controls.misc );
	Menu_AddItem( &s_controls.menu, &s_controls.look );

	Menu_AddItem( &s_controls.menu, &s_controls.sensitivity );
	Menu_AddItem( &s_controls.menu, &s_controls.smoothmouse );
	Menu_AddItem( &s_controls.menu, &s_controls.invertmouse );
	Menu_AddItem( &s_controls.menu, &s_controls.lookup );
	Menu_AddItem( &s_controls.menu, &s_controls.lookdown );
	Menu_AddItem( &s_controls.menu, &s_controls.zoomview );
	Menu_AddItem( &s_controls.menu, &s_controls.joyenable );
	Menu_AddItem( &s_controls.menu, &s_controls.joythreshold );

	Menu_AddItem( &s_controls.menu, &s_controls.walkforward );
	Menu_AddItem( &s_controls.menu, &s_controls.backpedal );
	Menu_AddItem( &s_controls.menu, &s_controls.stepleft );
	Menu_AddItem( &s_controls.menu, &s_controls.stepright );
	Menu_AddItem( &s_controls.menu, &s_controls.moveup );
	Menu_AddItem( &s_controls.menu, &s_controls.movedown );
	Menu_AddItem( &s_controls.menu, &s_controls.turnleft );
	Menu_AddItem( &s_controls.menu, &s_controls.turnright );

	Menu_AddItem( &s_controls.menu, &s_controls.attack );
	Menu_AddItem( &s_controls.menu, &s_controls.attack2 );
	Menu_AddItem( &s_controls.menu, &s_controls.nextweapon );
	Menu_AddItem( &s_controls.menu, &s_controls.prevweapon );
	Menu_AddItem( &s_controls.menu, &s_controls.weapon_mg );
	Menu_AddItem( &s_controls.menu, &s_controls.weapon_1 );
	Menu_AddItem( &s_controls.menu, &s_controls.weapon_2 );
	Menu_AddItem( &s_controls.menu, &s_controls.weapon_3 );
	Menu_AddItem( &s_controls.menu, &s_controls.weapon_4 );
	Menu_AddItem( &s_controls.menu, &s_controls.weapon_5 );
	Menu_AddItem( &s_controls.menu, &s_controls.weapon_6 );
	Menu_AddItem( &s_controls.menu, &s_controls.weapon_flare );

	Menu_AddItem( &s_controls.menu, &s_controls.showscores );
	Menu_AddItem( &s_controls.menu, &s_controls.chat );
	Menu_AddItem( &s_controls.menu, &s_controls.chat2 );
	Menu_AddItem( &s_controls.menu, &s_controls.chat3 );
	Menu_AddItem( &s_controls.menu, &s_controls.chat4 );

	Menu_AddItem( &s_controls.menu, &s_controls.gear );
	Menu_AddItem( &s_controls.menu, &s_controls.brake );
	Menu_AddItem( &s_controls.menu, &s_controls.freecam );
	Menu_AddItem( &s_controls.menu, &s_controls.increase );
	Menu_AddItem( &s_controls.menu, &s_controls.decrease );
	Menu_AddItem( &s_controls.menu, &s_controls.encyc );
	Menu_AddItem( &s_controls.menu, &s_controls.vehsel );
	Menu_AddItem( &s_controls.menu, &s_controls.gps );
	Menu_AddItem( &s_controls.menu, &s_controls.radar );
	Menu_AddItem( &s_controls.menu, &s_controls.extinfo );
	Menu_AddItem( &s_controls.menu, &s_controls.toggleview );
	Menu_AddItem( &s_controls.menu, &s_controls.cameraup );
	Menu_AddItem( &s_controls.menu, &s_controls.cameradown );
	Menu_AddItem( &s_controls.menu, &s_controls.zoomin );
	Menu_AddItem( &s_controls.menu, &s_controls.zoomout );
	Menu_AddItem( &s_controls.menu, &s_controls.contacttower );
	Menu_AddItem( &s_controls.menu, &s_controls.advancedctrl );
	Menu_AddItem( &s_controls.menu, &s_controls.radarrange );
	Menu_AddItem( &s_controls.menu, &s_controls.unlock );

	Menu_AddItem( &s_controls.menu, &s_controls.back );

	trap_Cvar_VariableStringBuffer( "name", s_controls.name.string, 16 );
	Q_CleanStr( s_controls.name.string );

	// initialize the configurable cvars
	Controls_InitCvars();

	// initialize the current config
	Controls_GetConfig();

	// initial default section
	s_controls.section = C_CONTROLS;

	// update the ui
	Controls_Update();
}


/*
=================
Controls_Cache
=================
*/
void Controls_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_LOGO );
	trap_R_RegisterShaderNoMip( ART_BACK );
	trap_R_RegisterShaderNoMip( ART_BACK_A );
}


/*
=================
UI_ControlsMenu
=================
*/
void UI_ControlsMenu( void ) {
	Controls_MenuInit();
	UI_PushMenu( &s_controls.menu );
}
