/*
 * $Id: cg_main.c,v 1.26 2002-02-11 16:29:42 sparky909_uk Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"

#ifdef _MENU_SCOREBOARD
	#include "../ui/ui_shared.h"

	// display context for new ui stuff
	displayContextDef_t cgDC;
#endif

int forceModelModificationCount = -1;

void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown( void );


/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {

	switch ( command ) {
	case CG_INIT:
		CG_Init( arg0, arg1, arg2 );
		return 0;
	case CG_SHUTDOWN:
		CG_Shutdown();
		return 0;
	case CG_CONSOLE_COMMAND:
		return CG_ConsoleCommand();
	case CG_DRAW_ACTIVE_FRAME:
		CG_DrawActiveFrame( arg0, arg1, arg2 );
		return 0;
	case CG_CROSSHAIR_PLAYER:
		return CG_CrosshairPlayer();
	case CG_LAST_ATTACKER:
		return CG_LastAttacker();
	case CG_KEY_EVENT:
		CG_KeyEvent(arg0, arg1);
		return 0;
	case CG_MOUSE_EVENT:
		CG_MouseEvent(arg0, arg1);
		return 0;
	case CG_EVENT_HANDLING:
		CG_EventHandling(arg0);
		return 0;
	default:
		CG_Error( "vmMain: unknown command %i", command );
		break;
	}
	return -1;
}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
weaponInfo_t		cg_weapons[WI_MAX];
itemInfo_t			cg_items[MAX_ITEMS];


vmCvar_t	cg_railTrailTime;
vmCvar_t	cg_centertime;
vmCvar_t	cg_runpitch;
vmCvar_t	cg_runroll;
vmCvar_t	cg_bobup;
vmCvar_t	cg_bobpitch;
vmCvar_t	cg_bobroll;
vmCvar_t	cg_swingSpeed;
vmCvar_t	cg_shadows;
vmCvar_t	cg_drawTimer;
vmCvar_t	cg_drawFPS;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
//vmCvar_t	cg_drawCrosshair;
vmCvar_t	cg_drawCrosshairNames;
vmCvar_t	cg_drawRewards;
vmCvar_t	cg_crosshairSize;
vmCvar_t	cg_crosshairX;
vmCvar_t	cg_crosshairY;
vmCvar_t	cg_draw2D;
vmCvar_t	cg_drawStatus;
vmCvar_t	cg_debugPosition;
vmCvar_t	cg_debugEvents;
vmCvar_t	cg_errorDecay;
vmCvar_t	cg_nopredict;
vmCvar_t	cg_showmiss;
vmCvar_t	cg_addMarks;
vmCvar_t	cg_viewsize;
vmCvar_t	cg_drawGun;
vmCvar_t	cg_gun_frame;
vmCvar_t	cg_gun_x;
vmCvar_t	cg_gun_y;
vmCvar_t	cg_gun_z;
vmCvar_t	cg_ignore;
vmCvar_t	cg_simpleItems;
vmCvar_t	cg_fov;
vmCvar_t	cg_zoomFov;
vmCvar_t	cg_thirdPerson;
vmCvar_t	cg_thirdPersonRange;
vmCvar_t	cg_thirdPersonAngle;
vmCvar_t	cg_stereoSeparation;
vmCvar_t	cg_lagometer;
vmCvar_t	cg_synchronousClients;
vmCvar_t 	cg_teamChatTime;
vmCvar_t 	cg_teamChatHeight;
vmCvar_t 	cg_stats;
vmCvar_t 	cg_buildScript;
vmCvar_t 	cg_forceModel;
vmCvar_t	cg_paused;
vmCvar_t	cg_freelook;
vmCvar_t	cg_advanced;
vmCvar_t	cg_predictItems;
vmCvar_t	cg_drawTeamOverlay;
vmCvar_t	cg_teamOverlayUserinfo;
vmCvar_t	cg_drawFriend;
vmCvar_t	cg_teamChatsOnly;
vmCvar_t	cg_hudFiles;
vmCvar_t 	cg_smoothClients;
vmCvar_t	pmove_fixed;
//vmCvar_t	cg_pmove_fixed;
vmCvar_t	pmove_msec;
vmCvar_t	cg_pmove_msec;
vmCvar_t	cg_cameraMode;
vmCvar_t	cg_timescaleFadeEnd;
vmCvar_t	cg_timescaleFadeSpeed;
vmCvar_t	cg_timescale;
vmCvar_t	cg_smallFont;
vmCvar_t	cg_bigFont;
vmCvar_t	cg_noTaunt;
// MFQ3
vmCvar_t	cg_thirdPersonHeight;

// MFQ3
vmCvar_t	cg_nextVehicle;
vmCvar_t	cg_vehicle;
vmCvar_t	cg_radarTargets;
vmCvar_t	cg_tracer;
vmCvar_t	cg_smoke;

vmCvar_t	cg_oldHUD;

vmCvar_t	hud_heading;
vmCvar_t	hud_speed;
vmCvar_t	hud_altitude;
vmCvar_t	hud_mfd;
vmCvar_t	hud_mfd2;
vmCvar_t	hud_center;
vmCvar_t	hud_health;
vmCvar_t	hud_throttle;
vmCvar_t	hud_weapons;

vmCvar_t	hud_color;
vmCvar_t	mfd_color;
vmCvar_t	mfd1_defaultpage;
vmCvar_t	mfd2_defaultpage;

vmCvar_t	m1cp_page0;
vmCvar_t	m1cp_page1;
vmCvar_t	m1cp_page2;
vmCvar_t	m1cp_page3;
vmCvar_t	m1cp_page4;
vmCvar_t	m1cp_page5;

vmCvar_t	m2cp_page0;
vmCvar_t	m2cp_page1;
vmCvar_t	m2cp_page2;
vmCvar_t	m2cp_page3;
vmCvar_t	m2cp_page4;
vmCvar_t	m2cp_page5;

vmCvar_t	cg_fxQuality;
vmCvar_t	cg_consoleTextStyle;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

cvarTable_t		cvarTable[] = {
	{ &cg_ignore, "cg_ignore", "0", 0 },	// used for debugging
	{ &cg_drawGun, "cg_drawGun", "1", CVAR_ARCHIVE },
	{ &cg_zoomFov, "cg_zoomfov", "22.5", CVAR_ARCHIVE },
	{ &cg_fov, "cg_fov", "90", CVAR_ARCHIVE },
	{ &cg_viewsize, "cg_viewsize", "100", CVAR_ARCHIVE },
	{ &cg_stereoSeparation, "cg_stereoSeparation", "0.4", CVAR_ARCHIVE  },
	{ &cg_shadows, "cg_shadows", "1", CVAR_ARCHIVE  },
	{ &cg_draw2D, "cg_draw2D", "1", CVAR_ARCHIVE  },
	{ &cg_drawStatus, "cg_drawStatus", "1", CVAR_ARCHIVE  },
	{ &cg_drawTimer, "cg_drawTimer", "1", CVAR_ARCHIVE  },
	{ &cg_drawFPS, "cg_drawFPS", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
//	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "24", CVAR_ARCHIVE },
	{ &cg_crosshairX, "cg_crosshairX", "0", CVAR_ARCHIVE },
	{ &cg_crosshairY, "cg_crosshairY", "0", CVAR_ARCHIVE },
	{ &cg_simpleItems, "cg_simpleItems", "0", CVAR_ARCHIVE },
	{ &cg_addMarks, "cg_marks", "1", CVAR_ARCHIVE },
	{ &cg_lagometer, "cg_lagometer", "0", CVAR_ARCHIVE },
	{ &cg_railTrailTime, "cg_railTrailTime", "400", CVAR_ARCHIVE  },
	{ &cg_gun_x, "cg_gunX", "0", CVAR_CHEAT },
	{ &cg_gun_y, "cg_gunY", "0", CVAR_CHEAT },
	{ &cg_gun_z, "cg_gunZ", "0", CVAR_CHEAT },
	{ &cg_centertime, "cg_centertime", "3", CVAR_CHEAT },
	{ &cg_runpitch, "cg_runpitch", "0.002", CVAR_ARCHIVE},
	{ &cg_runroll, "cg_runroll", "0.005", CVAR_ARCHIVE },
	{ &cg_bobup , "cg_bobup", "0.005", CVAR_ARCHIVE },
	{ &cg_bobpitch, "cg_bobpitch", "0.002", CVAR_ARCHIVE },
	{ &cg_bobroll, "cg_bobroll", "0.002", CVAR_ARCHIVE },
	{ &cg_swingSpeed, "cg_swingSpeed", "0.3", CVAR_CHEAT },
	{ &cg_debugPosition, "cg_debugposition", "0", CVAR_CHEAT },
	{ &cg_debugEvents, "cg_debugevents", "0", CVAR_CHEAT },
	{ &cg_errorDecay, "cg_errordecay", "100", 0 },
	{ &cg_nopredict, "cg_nopredict", "0", 0 },
	{ &cg_showmiss, "cg_showmiss", "0", 0 },
	{ &cg_thirdPersonRange, "cg_thirdPersonRange", "0", 0 },
	{ &cg_thirdPersonHeight, "cg_thirdPersonHeight", "0", 0 },
	{ &cg_thirdPerson, "cg_thirdPerson", "1", 0 },	// MFQ3: changed default from 0
	{ &cg_teamChatTime, "cg_teamChatTime", "3000", CVAR_ARCHIVE  },
	{ &cg_teamChatHeight, "cg_teamChatHeight", "0", CVAR_ARCHIVE  },
	{ &cg_forceModel, "cg_forceModel", "0", CVAR_ARCHIVE  },
	{ &cg_predictItems, "cg_predictItems", "1", CVAR_ARCHIVE },

	{ &cg_drawTeamOverlay, "cg_drawTeamOverlay", "0", CVAR_ARCHIVE },
	{ &cg_teamOverlayUserinfo, "teamoverlay", "0", CVAR_ROM | CVAR_USERINFO },
	{ &cg_stats, "cg_stats", "0", 0 },
	{ &cg_drawFriend, "cg_drawFriend", "1", CVAR_ARCHIVE },
	{ &cg_teamChatsOnly, "cg_teamChatsOnly", "0", CVAR_ARCHIVE },
	// the following variables are created in other parts of the system,
	// but we also reference them here
	{ &cg_buildScript, "com_buildScript", "0", 0 },	// force loading of all possible data amd error on failures
	{ &cg_paused, "cl_paused", "0", CVAR_ROM },
	{ &cg_freelook, "cl_freelook", "1", CVAR_ROM },
	{ &cg_advanced, "cg_advanced", "0", CVAR_ARCHIVE },
	{ &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo

	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	// MFQ3
	{ &cg_vehicle, "cg_vehicle", "-1", CVAR_ROM | CVAR_USERINFO },
	{ &cg_nextVehicle, "cg_nextVehicle", "-1", CVAR_ROM | CVAR_USERINFO },
	{ &cg_radarTargets, "cg_radarTargets", "20", CVAR_ARCHIVE },
	{ &cg_tracer, "cg_tracer", "3", CVAR_ARCHIVE },
	{ &cg_smoke, "cg_smoke", "1", CVAR_ARCHIVE },

	{ &cg_oldHUD, "cg_oldHUD", "0", CVAR_ARCHIVE },

	{ &hud_heading, "hud_heading", "1", CVAR_ARCHIVE },
	{ &hud_speed, "hud_speed", "1", CVAR_ARCHIVE },
	{ &hud_altitude, "hud_altitude", "1", CVAR_ARCHIVE },
	{ &hud_mfd, "hud_mfd", "1", CVAR_ARCHIVE },
	{ &hud_mfd2, "hud_mfd2", "1", CVAR_ARCHIVE },
	{ &hud_center, "hud_center", "1", CVAR_ARCHIVE },
	{ &hud_health, "hud_health", "1", CVAR_ARCHIVE },
	{ &hud_throttle, "hud_throttle", "1", CVAR_ARCHIVE },
	{ &hud_weapons, "hud_weapons", "1", CVAR_ARCHIVE },

	{ &hud_color, "hud_color", "0", CVAR_ARCHIVE },
	{ &mfd_color, "mfd_color", "0", CVAR_ARCHIVE },

	{ &mfd1_defaultpage, "mfd1_defaultpage", "1", CVAR_ARCHIVE },
	{ &mfd2_defaultpage, "mfd2_defaultpage", "2", CVAR_ARCHIVE },

	{ &m1cp_page0, "m1cp_page0", "1", CVAR_ARCHIVE },
	{ &m1cp_page1, "m1cp_page1", "1", CVAR_ARCHIVE },
	{ &m1cp_page2, "m1cp_page2", "1", CVAR_ARCHIVE },
	{ &m1cp_page3, "m1cp_page3", "1", CVAR_ARCHIVE },
	{ &m1cp_page4, "m1cp_page4", "1", CVAR_ARCHIVE },
	{ &m1cp_page5, "m1cp_page5", "1", CVAR_ARCHIVE },

	{ &m2cp_page0, "m2cp_page0", "1", CVAR_ARCHIVE },
	{ &m2cp_page1, "m2cp_page1", "1", CVAR_ARCHIVE },
	{ &m2cp_page2, "m2cp_page2", "1", CVAR_ARCHIVE },
	{ &m2cp_page3, "m2cp_page3", "1", CVAR_ARCHIVE },
	{ &m2cp_page4, "m2cp_page4", "1", CVAR_ARCHIVE },
	{ &m2cp_page5, "m2cp_page5", "1", CVAR_ARCHIVE },

	{ &cg_fxQuality, "cg_fxQuality", "2", CVAR_ARCHIVE },	// enum: quality_t in cg_local.h
	{ &cg_consoleTextStyle, "cg_consoleTextStyle", "3", CVAR_ARCHIVE },

	{ &pmove_fixed, "pmove_fixed", "1", CVAR_ROM},
	{ &pmove_msec, "pmove_msec", "8", 0},
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE},
	{ &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
	{ &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},

//	{ &cg_pmove_fixed, "cg_pmove_fixed", "1", CVAR_USERINFO | CVAR_ARCHIVE }
};

int		cvarTableSize = sizeof( cvarTable ) / sizeof( cvarTable[0] );

vec4_t HUDColors[HUD_MAX] = {
							{0.00f, 1.00f, 0.00f, 1.00f},	// HUD_GREEN
							{0.00f, 0.80f, 0.00f, 1.00f},	// HUD_DARK_GREEN
							{1.00f, 1.00f, 1.00f, 1.00f},	// HUD_WHITE
							{0.70f, 0.70f, 0.70f, 1.00f},	// HUD_GREY
							{1.00f, 0.00f, 0.00f, 1.00f},	// HUD_RED
							{0.80f, 0.00f, 0.00f, 1.00f},	// HUD_DARK_RED
							{0.00f, 0.00f, 1.00f, 1.00f},	// HUD_BLUE
							{0.00f, 0.00f, 0.80f, 1.00f},	// HUD_DARK_BLUE
							{1.00f, 1.00f, 0.00f, 1.00f},	// HUD_YELLOW
							{1.00f, 0.00f, 1.00f, 1.00f},	// HUD_MAGENTA
							{0.00f, 1.00f, 1.00f, 1.00f},	// HUD_CYAN
							{1.00f, 0.70f, 0.00f, 1.00f},	// HUD_ORANGE
							};


/*
=================
CG_RegisterCvars
=================
*/
void CG_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;
	char		var[MAX_TOKEN_CHARS];

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	trap_Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
	cgs.localServer = atoi( var );

	forceModelModificationCount = cg_forceModel.modificationCount;

}

/*																																			
===================
CG_ForceModelChange
===================
*/
static void CG_ForceModelChange( void ) {
	int		i;

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char *clientInfo;

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0] ) {
			continue;
		}
		CG_NewClientInfo( i );
	}
}

/*
=================
CG_UpdateCvars
=================
*/
void CG_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		trap_Cvar_Update( cv->vmCvar );
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) {
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if ( cg_drawTeamOverlay.integer > 0 ) {
			trap_Cvar_Set( "teamoverlay", "1" );
		} else {
			trap_Cvar_Set( "teamoverlay", "0" );
		}
		// FIXME E3 HACK
		trap_Cvar_Set( "teamoverlay", "1" );
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount ) {
		forceModelModificationCount = cg_forceModel.modificationCount;
		CG_ForceModelChange();
	}
	cg.tracernum = (cg_tracer.integer > 0) ? cg_tracer.integer : 3;
}

int CG_CrosshairPlayer( void ) {
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) ) {
		return -1;
	}
	return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) {
	if ( !cg.attackerTime ) {
		return -1;
	}
	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

#ifdef _MENU_SCOREBOARD
	// add the text into the custom consol
	CG_Add_Console_Line( text );
#endif 

	// write the text into the console (only displayed if the cVar 'con_notifytime' is not -1)
	trap_Print( text );
}

void QDECL CG_Error( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	trap_Error( text );
}

#ifndef CGAME_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, error);
	vsprintf (text, error, argptr);
	va_end (argptr);

	CG_Error( "%s", text);
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	CG_Printf ("%s", text);
}

#endif

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


//========================================================================

/*
=================
CG_RegisterItemSounds

The server says this item is used on this level
=================
*/
static void CG_RegisterItemSounds( int itemNum ) {
	gitem_t			*item;
	char			data[MAX_QPATH];
	char			*s, *start;
	int				len;

	item = &bg_itemlist[ itemNum ];

	if( item->pickup_sound ) {
		trap_S_RegisterSound( item->pickup_sound, qfalse );
	}

	// parse the space seperated precache string for other media
	s = item->sounds;
	if (!s || !s[0])
		return;

	while (*s) {
		start = s;
		while (*s && *s != ' ') {
			s++;
		}

		len = s-start;
		if (len >= MAX_QPATH || len < 5) {
			CG_Error( "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			trap_S_RegisterSound( data, qfalse );
		}
	}
}


/*
=================
CG_RegisterSounds

called during a precache command
=================
*/
static void CG_RegisterSounds( void ) {
	int		i;
	char	items[MAX_ITEMS+1];
	const char	*soundName;

	cgs.media.oneMinuteSound = trap_S_RegisterSound( "sound/feedback/1_minute.wav", qfalse );
	cgs.media.fiveMinuteSound = trap_S_RegisterSound( "sound/feedback/5_minute.wav", qfalse );
	cgs.media.suddenDeathSound = trap_S_RegisterSound( "sound/feedback/sudden_death.wav", qfalse );
	cgs.media.oneFragSound = trap_S_RegisterSound( "sound/feedback/1_frag.wav", qfalse );
	cgs.media.twoFragSound = trap_S_RegisterSound( "sound/feedback/2_frags.wav", qfalse );
	cgs.media.threeFragSound = trap_S_RegisterSound( "sound/feedback/3_frags.wav", qfalse );
	cgs.media.count3Sound = trap_S_RegisterSound( "sound/feedback/three.wav", qfalse );
	cgs.media.count2Sound = trap_S_RegisterSound( "sound/feedback/two.wav", qfalse );
	cgs.media.count1Sound = trap_S_RegisterSound( "sound/feedback/one.wav", qfalse );
	cgs.media.countFightSound = trap_S_RegisterSound( "sound/feedback/fight.wav", qfalse );
	cgs.media.countPrepareSound = trap_S_RegisterSound( "sound/feedback/prepare.wav", qfalse );

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {

		cgs.media.captureAwardSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", qfalse );
		cgs.media.redLeadsSound = trap_S_RegisterSound( "sound/feedback/redleads.wav", qfalse );
		cgs.media.blueLeadsSound = trap_S_RegisterSound( "sound/feedback/blueleads.wav", qfalse );
		cgs.media.teamsTiedSound = trap_S_RegisterSound( "sound/feedback/teamstied.wav", qfalse );
		cgs.media.hitTeamSound = trap_S_RegisterSound( "sound/feedback/hit_teammate.wav", qfalse );

		cgs.media.redScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_red_scores.wav", qfalse );
		cgs.media.blueScoredSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_scores.wav", qfalse );

		cgs.media.captureYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", qfalse );
		cgs.media.captureOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagcapture_opponent.wav", qfalse );

		cgs.media.returnYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_yourteam.wav", qfalse );
		cgs.media.returnOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", qfalse );

		cgs.media.takenYourTeamSound = trap_S_RegisterSound( "sound/teamplay/flagtaken_yourteam.wav", qfalse );
		cgs.media.takenOpponentSound = trap_S_RegisterSound( "sound/teamplay/flagtaken_opponent.wav", qfalse );

		if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.redFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_red_returned.wav", qfalse );
			cgs.media.blueFlagReturnedSound = trap_S_RegisterSound( "sound/teamplay/voc_blue_returned.wav", qfalse );
			cgs.media.enemyTookYourFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_flag.wav", qfalse );
			cgs.media.yourTeamTookEnemyFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_flag.wav", qfalse );
		}

		cgs.media.youHaveFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_you_flag.wav", qfalse );
		cgs.media.holyShitSound = trap_S_RegisterSound("sound/feedback/voc_holyshit.wav", qfalse);
		cgs.media.yourTeamTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_team_1flag.wav", qfalse );
		cgs.media.enemyTookTheFlagSound = trap_S_RegisterSound( "sound/teamplay/voc_enemy_1flag.wav", qfalse );
	}

	cgs.media.respawnSound = trap_S_RegisterSound( "sound/items/respawn1.wav", qfalse );

	cgs.media.talkSound = trap_S_RegisterSound( "sound/player/talk.wav", qfalse );

	cgs.media.hitSound = trap_S_RegisterSound( "sound/feedback/hit.wav", qfalse );

	cgs.media.impressiveSound = trap_S_RegisterSound( "sound/feedback/impressive.wav", qfalse );
	cgs.media.excellentSound = trap_S_RegisterSound( "sound/feedback/excellent.wav", qfalse );
	cgs.media.deniedSound = trap_S_RegisterSound( "sound/feedback/denied.wav", qfalse );
	cgs.media.assistSound = trap_S_RegisterSound( "sound/feedback/assist.wav", qfalse );
	cgs.media.defendSound = trap_S_RegisterSound( "sound/feedback/defense.wav", qfalse );

	cgs.media.takenLeadSound = trap_S_RegisterSound( "sound/feedback/takenlead.wav", qfalse);
	cgs.media.tiedLeadSound = trap_S_RegisterSound( "sound/feedback/tiedlead.wav", qfalse);
	cgs.media.lostLeadSound = trap_S_RegisterSound( "sound/feedback/lostlead.wav", qfalse);

	cgs.media.watrInSound = trap_S_RegisterSound( "sound/player/watr_in.wav", qfalse);
	cgs.media.watrOutSound = trap_S_RegisterSound( "sound/player/watr_out.wav", qfalse);
	cgs.media.watrUnSound = trap_S_RegisterSound( "sound/player/watr_un.wav", qfalse);

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS ) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
//		if ( items[ i ] == '1' || cg_buildScript.integer ) {
			CG_RegisterItemSounds( i );
//		}
	}

	for ( i = 1 ; i < MAX_SOUNDS ; i++ ) {
		soundName = CG_ConfigString( CS_SOUNDS+i );
		if ( !soundName[0] ) {
			break;
		}
		if ( soundName[0] == '*' ) {
			continue;	// custom sound
		}
		cgs.gameSounds[i] = trap_S_RegisterSound( soundName, qfalse );
	}

	// FIXME: only needed with item
	cgs.media.sfx_ric1 = trap_S_RegisterSound ("sound/weapons/machinegun/ric1.wav", qfalse);
	cgs.media.sfx_ric2 = trap_S_RegisterSound ("sound/weapons/machinegun/ric2.wav", qfalse);
	cgs.media.sfx_ric3 = trap_S_RegisterSound ("sound/weapons/machinegun/ric3.wav", qfalse);
	cgs.media.sfx_rockexp = trap_S_RegisterSound ("sound/weapons/rocket/rocklx1a.wav", qfalse);

	cgs.media.n_healthSound = trap_S_RegisterSound("sound/items/n_health.wav", qfalse );

	// MFQ3
	cgs.media.engineJet = trap_S_RegisterSound("sound/engines/jet.wav", qfalse );
	cgs.media.engineProp = trap_S_RegisterSound("sound/engines/prop.wav", qfalse );
	cgs.media.engineJetAB = trap_S_RegisterSound( "sound/engines/afterburner.wav", qfalse );

	// MFQ3: new sounds
	cgs.media.planeDeath[0] = trap_S_RegisterSound( "sound/explosions/explode1.wav", qfalse );	
	cgs.media.planeDeath[1] = trap_S_RegisterSound( "sound/explosions/explode2.wav", qfalse );	
	cgs.media.planeDeath[2] = trap_S_RegisterSound( "sound/explosions/explode3.wav", qfalse );	
	cgs.media.planeDeath[3] = trap_S_RegisterSound( "sound/explosions/explode4.wav", qfalse );	
	if( !cgs.media.planeDeath[0] )
	{
		// MFQ3: old quake3 sound (backup)
		cgs.media.planeDeath[0] = trap_S_RegisterSound( "sound/death/planedeath.wav", qfalse );	
		cgs.media.planeDeath[1] = cgs.media.planeDeath[0];
		cgs.media.planeDeath[2] = cgs.media.planeDeath[0];
		cgs.media.planeDeath[3] = cgs.media.planeDeath[0];
	}

	// added mg
	cgs.media.engineTank[0] = trap_S_RegisterSound("sound/engines/tank0.wav", qfalse );
	cgs.media.engineTank[1] = trap_S_RegisterSound("sound/engines/tank1.wav", qfalse );
	cgs.media.engineTank[2] = trap_S_RegisterSound("sound/engines/tank2.wav", qfalse );
	cgs.media.engineTank[3] = trap_S_RegisterSound("sound/engines/tank3.wav", qfalse );
	cgs.media.engineTank[4] = trap_S_RegisterSound("sound/engines/tank4.wav", qfalse );
	cgs.media.engineTank[5] = trap_S_RegisterSound("sound/engines/tank5.wav", qfalse );
	cgs.media.engineTank[6] = trap_S_RegisterSound("sound/engines/tank6.wav", qfalse );
	cgs.media.engineTank[7] = trap_S_RegisterSound("sound/engines/tank7.wav", qfalse );
	cgs.media.engineTank[8] = trap_S_RegisterSound("sound/engines/tank8.wav", qfalse );

}



//===================================================================================


/*
=================
CG_RegisterGraphics

This function may execute for a couple of minutes with a slow disk.
=================
*/
static void CG_RegisterGraphics( void ) {
	int			i;
	char		items[MAX_ITEMS+1];
	static char		*sb_nums[11] = {
		"gfx/2d/numbers/zero_32b",
		"gfx/2d/numbers/one_32b",
		"gfx/2d/numbers/two_32b",
		"gfx/2d/numbers/three_32b",
		"gfx/2d/numbers/four_32b",
		"gfx/2d/numbers/five_32b",
		"gfx/2d/numbers/six_32b",
		"gfx/2d/numbers/seven_32b",
		"gfx/2d/numbers/eight_32b",
		"gfx/2d/numbers/nine_32b",
		"gfx/2d/numbers/minus_32b",
	};
	static char		*rd_icons[RD_MAX_ICONS] = {
		"hud/ground_enemy.tga",
		"hud/ground_friend.tga",
		"hud/ground_unknown.tga",
		"hud/building_enemy.tga",
		"hud/building_friend.tga",
		"hud/building_unknown.tga",
		"hud/air_same_enemy.tga",
		"hud/air_same_friend.tga",
		"hud/air_same_unknown.tga",
		"hud/air_high_enemy.tga",
		"hud/air_high_friend.tga",
		"hud/air_high_unknown.tga",
		"hud/air_low_enemy.tga",
		"hud/air_low_friend.tga",
		"hud/air_low_unknown.tga"
	};
	static char		*mfq3_nums[14] = {
		"newhud/numbers/zero",
		"newhud/numbers/one",
		"newhud/numbers/two",
		"newhud/numbers/three",
		"newhud/numbers/four",
		"newhud/numbers/five",
		"newhud/numbers/six",
		"newhud/numbers/seven",
		"newhud/numbers/eight",
		"newhud/numbers/nine",
		"newhud/numbers/minus",
		"newhud/numbers/colon",
		"newhud/numbers/percent",
		"newhud/numbers/point"
	};
	static char		*mfq3_health[10] = {
		"newhud/hlth/1",
		"newhud/hlth/2",
		"newhud/hlth/3",
		"newhud/hlth/4",
		"newhud/hlth/5",
		"newhud/hlth/6",
		"newhud/hlth/7",
		"newhud/hlth/8",
		"newhud/hlth/9",
		"newhud/hlth/10"
	};
	static char		*mfq3_throttle_1[10] = {
		"newhud/thr/1_1",
		"newhud/thr/1_2",
		"newhud/thr/1_3",
		"newhud/thr/1_4",
		"newhud/thr/1_5",
		"newhud/thr/1_6",
		"newhud/thr/1_7",
		"newhud/thr/1_8",
		"newhud/thr/1_9",
		"newhud/thr/1_10",
	};
	static char		*mfq3_throttle_2_1[10] = {
		"newhud/thr/2_1_1",
		"newhud/thr/2_1_2",
		"newhud/thr/2_1_3",
		"newhud/thr/2_1_4",
		"newhud/thr/2_1_5",
		"newhud/thr/2_1_6",
		"newhud/thr/2_1_7",
		"newhud/thr/2_1_8",
		"newhud/thr/2_1_9",
		"newhud/thr/2_1_10",
	};
	static char		*mfq3_throttle_2_2[10] = {
		"newhud/thr/2_2_1",
		"newhud/thr/2_2_2",
		"newhud/thr/2_2_3",
		"newhud/thr/2_2_4",
		"newhud/thr/2_2_5",
		"newhud/thr/2_2_6",
		"newhud/thr/2_2_7",
		"newhud/thr/2_2_8",
		"newhud/thr/2_2_9",
		"newhud/thr/2_2_10",
	};
	static char		*mfq3_throttle_1_ab[15] = {
		"newhud/thr/ab1_1",
		"newhud/thr/ab1_2",
		"newhud/thr/ab1_3",
		"newhud/thr/ab1_4",
		"newhud/thr/ab1_5",
		"newhud/thr/ab1_6",
		"newhud/thr/ab1_7",
		"newhud/thr/ab1_8",
		"newhud/thr/ab1_9",
		"newhud/thr/ab1_10",
		"newhud/thr/ab1_11",
		"newhud/thr/ab1_12",
		"newhud/thr/ab1_13",
		"newhud/thr/ab1_14",
		"newhud/thr/ab1_15"
	};
	static char		*mfq3_throttle_2_1_ab[15] = {
		"newhud/thr/ab2_1_1",
		"newhud/thr/ab2_1_2",
		"newhud/thr/ab2_1_3",
		"newhud/thr/ab2_1_4",
		"newhud/thr/ab2_1_5",
		"newhud/thr/ab2_1_6",
		"newhud/thr/ab2_1_7",
		"newhud/thr/ab2_1_8",
		"newhud/thr/ab2_1_9",
		"newhud/thr/ab2_1_10",
		"newhud/thr/ab2_1_11",
		"newhud/thr/ab2_1_12",
		"newhud/thr/ab2_1_13",
		"newhud/thr/ab2_1_14",
		"newhud/thr/ab2_1_15"
	};
	static char		*mfq3_throttle_2_2_ab[15] = {
		"newhud/thr/ab2_2_1",
		"newhud/thr/ab2_2_2",
		"newhud/thr/ab2_2_3",
		"newhud/thr/ab2_2_4",
		"newhud/thr/ab2_2_5",
		"newhud/thr/ab2_2_6",
		"newhud/thr/ab2_2_7",
		"newhud/thr/ab2_2_8",
		"newhud/thr/ab2_2_9",
		"newhud/thr/ab2_2_10",
		"newhud/thr/ab2_2_11",
		"newhud/thr/ab2_2_12",
		"newhud/thr/ab2_2_13",
		"newhud/thr/ab2_2_14",
		"newhud/thr/ab2_2_15"
	};
	static char		*mfq3_throttle_rev[5] = {
		"newhud/thr/rev_1",
		"newhud/thr/rev_2",
		"newhud/thr/rev_3",
		"newhud/thr/rev_4",
		"newhud/thr/rev_5",
	};

	static char		*mfq3_hudrets[HR_MAX] = {
		"newhud/reticle/reticle1.tga",
		"newhud/reticle/reticle2.tga",
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	trap_R_ClearScene();

	CG_LoadingString( cgs.mapname );

	trap_R_LoadWorldMap( cgs.mapname );

	// precache status bar pics
	CG_LoadingString( "game media" );

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = trap_R_RegisterShader( sb_nums[i] );
	}

	cgs.media.scoreboardName = trap_R_RegisterShaderNoMip( "menu/tab/name.tga" );
	cgs.media.scoreboardPing = trap_R_RegisterShaderNoMip( "menu/tab/ping.tga" );
	cgs.media.scoreboardScore = trap_R_RegisterShaderNoMip( "menu/tab/score.tga" );
	cgs.media.scoreboardTime = trap_R_RegisterShaderNoMip( "menu/tab/time.tga" );

	cgs.media.smokePuffShader = trap_R_RegisterShader( "smokePuff" );
	cgs.media.smokePuffRageProShader = trap_R_RegisterShader( "smokePuffRagePro" );
	cgs.media.lagometerShader = trap_R_RegisterShader("lagometer" );
	cgs.media.connectionShader = trap_R_RegisterShader( "disconnected" );

	cgs.media.selectShader = trap_R_RegisterShader( "gfx/2d/select" );

	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("gfx/2d/crosshair%c", 'a'+i) );
	}

	cgs.media.backTileShader = trap_R_RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader = trap_R_RegisterShader( "icons/noammo" );


	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
		cgs.media.redFlagModel = trap_R_RegisterModel( "models/flags/r_flag.md3" );
		cgs.media.blueFlagModel = trap_R_RegisterModel( "models/flags/b_flag.md3" );
		cgs.media.redFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_red1" );
		cgs.media.redFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.redFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_red3" );
		cgs.media.blueFlagShader[0] = trap_R_RegisterShaderNoMip( "icons/iconf_blu1" );
		cgs.media.blueFlagShader[1] = trap_R_RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.blueFlagShader[2] = trap_R_RegisterShaderNoMip( "icons/iconf_blu3" );
	}

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.friendShader = trap_R_RegisterShader( "sprites/foe" );
		cgs.media.teamStatusBar = trap_R_RegisterShader( "gfx/2d/colorbar.tga" );
	}

	cgs.media.balloonShader = trap_R_RegisterShader( "sprites/balloon3" );

	// MFQ3 stuff
	cgs.media.vapor = trap_R_RegisterModel( "models/effects/vapor.md3" );
	cgs.media.vaporBig = trap_R_RegisterModel( "models/effects/vapor_big.md3" );
	cgs.media.vaporShader = trap_R_RegisterShader( "models/effects/turbulence" );
	cgs.media.afterburner[AB_BALL] = trap_R_RegisterModel( "models/effects/burner_ball.md3" );
	cgs.media.afterburner[AB_BLUE] = trap_R_RegisterModel( "models/effects/burner_blue.md3" );
	cgs.media.afterburner[AB_RED] = trap_R_RegisterModel( "models/effects/burner_red.md3" );
	cgs.media.afterburner[AB_RED_SMALL] = trap_R_RegisterModel( "models/effects/burner_red_small.md3" );
	cgs.media.vehicleMuzzleFlashModel = trap_R_RegisterModel("models/effects/muzzleflash_md.md3");
	cgs.media.bulletTracer = trap_R_RegisterModel("models/effects/tracer_yellow.md3");
	cgs.media.pilot = trap_R_RegisterModel( "models/effects/pilot_body.md3" );
	cgs.media.pilotHead = trap_R_RegisterModel( "models/effects/pilot_head.md3" );
	cgs.media.pilotSeat = trap_R_RegisterModel( "models/effects/pilot_seat.md3" );
	cgs.media.pilotWW2 = trap_R_RegisterModel( "models/effects/pilot_body_ww2.md3" );
	cgs.media.pilotHeadWW2 = trap_R_RegisterModel( "models/effects/pilot_head_ww2.md3" );
	cgs.media.pilotSeatWW2 = trap_R_RegisterModel( "models/effects/pilot_seat_ww2.md3" );
	cgs.media.reticle[CH_GUNMODE] = trap_R_RegisterModel( "models/effects/ch_gunmode.md3" );
	cgs.media.reticle[CH_BOMBMODE] = trap_R_RegisterModel( "models/effects/ch_agmode.md3" );
	cgs.media.reticle[CH_ROCKETMODE] = trap_R_RegisterModel( "models/effects/ch_rocketmode.md3" );
	cgs.media.reticle[CH_MISSILEMODE] = trap_R_RegisterModel( "models/effects/ch_missilemode.md3" );
	cgs.media.reticle[CH_MISSILEMODELOCK] = trap_R_RegisterModel( "models/effects/ch_missilemodelock.md3" );
	cgs.media.reticle[CH_DEFAULT] = trap_R_RegisterModel( "models/effects/ch_default.md3" );
	// end MFQ3

	// MFQ3 HUD
	cgs.media.HUDmain = trap_R_RegisterShaderNoMip( "hud/main.tga" );
	cgs.media.HUDradar = trap_R_RegisterShaderNoMip( "hud/radar.tga" );
	cgs.media.HUDgps = trap_R_RegisterShaderNoMip( "hud/gps.tga" );
	cgs.media.HUDext = trap_R_RegisterShaderNoMip( "hud/side.tga" );
	for( i = 0; i <= MF_THROTTLE_AFTERBURNER; i++ ) {
		cgs.media.throttle[i] = trap_R_RegisterShaderNoMip(va("hud/%i.tga",i));
	}
	for( i = 1; i < bg_numberOfWeapons; i++ ) {
		availableWeapons[i].iconHandle = trap_R_RegisterShaderNoMip(availableWeapons[i].iconName);
	}
	for ( i=0 ; i< RD_MAX_ICONS ; i++) {
		cgs.media.radarIcons[i] = trap_R_RegisterShaderNoMip( rd_icons[i] );
	}
	// end MFQ3 HUD

	// MFQ3 new HUD
	cgs.media.HUDheading = trap_R_RegisterShaderNoMip( "newhud/heading_tape.tga" );
	for ( i=0 ; i<14 ; i++) {
		cgs.media.HUDnumbers[i] = trap_R_RegisterShaderNoMip( mfq3_nums[i] );
	}
	cgs.media.HUDvaluebox = trap_R_RegisterShaderNoMip( "newhud/valuebox.tga" );
	cgs.media.HUDind_h = trap_R_RegisterShaderNoMip( "newhud/ind_h.tga" );
	cgs.media.HUDind_v = trap_R_RegisterShaderNoMip( "newhud/ind_v.tga" );
	cgs.media.HUDind_v_r = trap_R_RegisterShaderNoMip( "newhud/ind_v_r.tga" );
	cgs.media.HUDcaret_h = trap_R_RegisterShaderNoMip( "newhud/caret_g_h.tga" );
	cgs.media.HUDcaret_v_g_l = trap_R_RegisterShaderNoMip( "newhud/caret_g_v.tga" );
	cgs.media.HUDcaret_v_r_l = trap_R_RegisterShaderNoMip( "newhud/caret_r_v.tga" );
	cgs.media.HUDmfd = trap_R_RegisterShaderNoMip( "newhud/mfd.tga" );
	cgs.media.HUDspeed = trap_R_RegisterShaderNoMip( "newhud/speed_tape.tga" );
	cgs.media.HUDrwr = trap_R_RegisterShaderNoMip( "newhud/rwr.tga" );
	cgs.media.HUDalt = trap_R_RegisterShaderNoMip( "newhud/alt_tape.tga" );
	cgs.media.HUDsolid = trap_R_RegisterShaderNoMip( "newhud/solid.tga" );
	for ( i=0 ; i<10 ; i++) {
		cgs.media.HUDhealth[i] = trap_R_RegisterShaderNoMip( mfq3_health[i] );
	}
	for ( i=0 ; i<15 ; i++) {
		cgs.media.HUDthrottle_1_ab[i] = trap_R_RegisterShader( mfq3_throttle_1_ab[i] );
		cgs.media.HUDthrottle_2_1_ab[i] = trap_R_RegisterShader( mfq3_throttle_2_1_ab[i] );
		cgs.media.HUDthrottle_2_2_ab[i] = trap_R_RegisterShader( mfq3_throttle_2_2_ab[i] );
	}
	for ( i=0 ; i<10 ; i++) {
		cgs.media.HUDthrottle_1[i] = trap_R_RegisterShader( mfq3_throttle_1[i] );
		cgs.media.HUDthrottle_2_1[i] = trap_R_RegisterShader( mfq3_throttle_2_1[i] );
		cgs.media.HUDthrottle_2_2[i] = trap_R_RegisterShader( mfq3_throttle_2_2[i] );
	}
	for ( i=0 ; i<5 ; i++) {
		cgs.media.HUDthrottle_rev[i] = trap_R_RegisterShader( mfq3_throttle_rev[i] );
	}
	cgs.media.HUDhealthtext = trap_R_RegisterShaderNoMip( "newhud/text/hlth.tga" );
	cgs.media.HUDthrottletext = trap_R_RegisterShaderNoMip( "newhud/text/thr.tga" );
	cgs.media.HUDalphabet = trap_R_RegisterShaderNoMip( "newhud/alphabet.tga" );
	for ( i=0 ; i<HR_MAX ; i++) {
		cgs.media.HUDreticles[i] = trap_R_RegisterShaderNoMip( mfq3_hudrets[i] );
	}
	// end MFQ3 new HUD


	cgs.media.bulletFlashModel = trap_R_RegisterModel("models/weaphits/bullet.md3");
	cgs.media.dishFlashModel = trap_R_RegisterModel("models/weaphits/boom01.md3");

	cgs.media.medalImpressive = trap_R_RegisterShaderNoMip( "medal_impressive" );
	cgs.media.medalExcellent = trap_R_RegisterShaderNoMip( "medal_excellent" );
	cgs.media.medalDefend = trap_R_RegisterShaderNoMip( "medal_defend" );
	cgs.media.medalAssist = trap_R_RegisterShaderNoMip( "medal_assist" );
	cgs.media.medalCapture = trap_R_RegisterShaderNoMip( "medal_capture" );


	memset( cg_items, 0, sizeof( cg_items ) );
	memset( cg_weapons, 0, sizeof( cg_weapons ) );

	// only register the items that the server says we need
	strcpy( items, CG_ConfigString( CS_ITEMS) );

	for ( i = 1 ; i < bg_numItems ; i++ ) {
		if ( items[ i ] == '1' || cg_buildScript.integer ) {
//			CG_LoadingItem( i );
			CG_RegisterItemVisuals( i );
		}
	}

	// wall marks
	cgs.media.bulletMarkShader = trap_R_RegisterShader( "gfx/damage/bullet_mrk" );
	cgs.media.burnMarkShader = trap_R_RegisterShader( "gfx/damage/burn_med_mrk" );

	// default shadows (one per class of vehicle - remember that vehicles can specify their own shadows if perferred)
	for( i = 0; i < MF_MAX_CATEGORIES; i ++ )
	{
		cgs.media.shadowMarkShader[i] = trap_R_RegisterShader( va( "%sshadow", cat_fileRef[i] ) );
	}

	// register the inline models
	cgs.numInlineModels = trap_CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char name[10];
		vec3_t mins, maxs;
		int	j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = trap_R_RegisterModel( name );
		trap_R_ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
		for ( j = 0 ; j < 3 ; j++ ) {
			cgs.inlineModelMidpoints[i][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );
		}
	}

	// register all the server specified models
	for (i=1 ; i<MAX_MODELS ; i++) {
		const char		*modelName;

		modelName = CG_ConfigString( CS_MODELS+i );
		if ( !modelName[0] ) {
			break;
		}
		cgs.gameModels[i] = trap_R_RegisterModel( modelName );
	}

	// MFQ3 register all the vehicles
	CG_LoadingString( "MFQ3 vehicles" );
	CG_CacheVehicles();

}



/*																																			
=======================
CG_BuildSpectatorString

=======================
*/
void CG_BuildSpectatorString() {
	int i;
	cg.spectatorList[0] = 0;
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_SPECTATOR ) {
			Q_strcat(cg.spectatorList, sizeof(cg.spectatorList), va("%s     ", cgs.clientinfo[i].name));
		}
	}
	i = strlen(cg.spectatorList);
	if (i != cg.spectatorLen) {
		cg.spectatorLen = i;
		cg.spectatorWidth = -1;
	}
}


/*																																			
===================
CG_RegisterClients
===================
*/
static void CG_RegisterClients( void ) {
	int		i;

	CG_NewClientInfo(cg.clientNum);

	for (i=0 ; i<MAX_CLIENTS ; i++) {
		const char		*clientInfo;

		if (cg.clientNum == i) {
			continue;
		}

		clientInfo = CG_ConfigString( CS_PLAYERS+i );
		if ( !clientInfo[0]) {
			continue;
		}
		CG_NewClientInfo( i );
	}
	CG_BuildSpectatorString();
}

#ifdef _MENU_SCOREBOARD	// NOTE: functions copied from v1.29h code to enable new scoreboard method

/*
=================
CG_Asset_Parse
=================
*/
qboolean CG_Asset_Parse(int handle) {
	pc_token_t token;
	const char *tempStr;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
		return qfalse;
	}
    
	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			return qtrue;
		}

		// font
		if (Q_stricmp(token.string, "font") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.textFont);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string, "smallFont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.smallFont);
			continue;
		}

		// font
		if (Q_stricmp(token.string, "bigfont") == 0) {
			int pointSize;
			if (!PC_String_Parse(handle, &tempStr) || !PC_Int_Parse(handle, &pointSize)) {
				return qfalse;
			}
			cgDC.registerFont(tempStr, pointSize, &cgDC.Assets.bigFont);
			continue;
		}

		// gradientbar
		if (Q_stricmp(token.string, "gradientbar") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip(tempStr);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string, "menuEnterSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuEnterSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string, "menuExitSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuExitSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string, "itemFocusSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.itemFocusSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string, "menuBuzzSound") == 0) {
			if (!PC_String_Parse(handle, &tempStr)) {
				return qfalse;
			}
			cgDC.Assets.menuBuzzSound = trap_S_RegisterSound( tempStr, qfalse );
			continue;
		}

		if (Q_stricmp(token.string, "cursor") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStr)) {
				return qfalse;
			}
			cgDC.Assets.cursor = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string, "cursorWait") == 0) {
			if (!PC_String_Parse(handle, &cgDC.Assets.cursorStrWait)) {
				return qfalse;
			}
			cgDC.Assets.cursorWait = trap_R_RegisterShaderNoMip( cgDC.Assets.cursorStrWait );
			continue;
		}

		if (Q_stricmp(token.string, "fadeClamp") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeClamp)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeCycle") == 0) {
			if (!PC_Int_Parse(handle, &cgDC.Assets.fadeCycle)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "fadeAmount") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.fadeAmount)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowX") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowX)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowY") == 0) {
			if (!PC_Float_Parse(handle, &cgDC.Assets.shadowY)) {
				return qfalse;
			}
			continue;
		}

		if (Q_stricmp(token.string, "shadowColor") == 0) {
			if (!PC_Color_Parse(handle, &cgDC.Assets.shadowColor)) {
				return qfalse;
			}
			cgDC.Assets.shadowFadeClamp = cgDC.Assets.shadowColor[3];
			continue;
		}
	}
	return qfalse; // bk001204 - why not?
}

/*
=================
CG_ParseMenu
=================
*/
void CG_ParseMenu(const char *menuFile) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource(menuFile);
	if (!handle)
		handle = trap_PC_LoadSource("ui/testhud.menu");
	if (!handle)
		return;

	while ( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( token.string[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string, "assetGlobalDef") == 0) {
			if (CG_Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}

		if (Q_stricmp(token.string, "menudef") == 0) {
			// start a new menu
			Menu_New(handle);
		}
	}
	trap_PC_FreeSource(handle);
}

/*
=================
CG_Load_Menu
=================
*/
qboolean CG_Load_Menu(char **p) {
	char *token;

	token = COM_ParseExt(p, qtrue);

	if (token[0] != '{') {
		return qfalse;
	}

	while ( 1 ) {

		token = COM_ParseExt(p, qtrue);
    
		if (Q_stricmp(token, "}") == 0) {
			return qtrue;
		}

		if ( !token || token[0] == 0 ) {
			return qfalse;
		}

		CG_ParseMenu(token); 
	}
	return qfalse;
}

/*
=================
CG_LoadMenus
=================
*/
void CG_LoadMenus(const char *menuFile) {
	char	*token;
	char *p;
	int	len, start;
	fileHandle_t	f;
	static char buf[MAX_MENUDEFFILE];

	start = trap_Milliseconds();

	len = trap_FS_FOpenFile( menuFile, &f, FS_READ );
	if ( !f ) {
		trap_Error( va( S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile ) );
		len = trap_FS_FOpenFile( "ui/hud.txt", &f, FS_READ );
		if (!f) {
			trap_Error( va( S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!\n", menuFile ) );
		}
	}

	if ( len >= MAX_MENUDEFFILE ) {
		trap_Error( va( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE ) );
		trap_FS_FCloseFile( f );
		return;
	}

	trap_FS_Read( buf, len, f );
	buf[len] = 0;
	trap_FS_FCloseFile( f );
	
	COM_Compress(buf);

	Menu_Reset();

	p = buf;

	while ( 1 ) {
		token = COM_ParseExt( &p, qtrue );
		if( !token || token[0] == 0 || token[0] == '}') {
			break;
		}

		//if ( Q_stricmp( token, "{" ) ) {
		//	Com_Printf( "Missing { in menu file\n" );
		//	break;
		//}

		//if ( menuCount == MAX_MENUS ) {
		//	Com_Printf( "Too many menus!\n" );
		//	break;
		//}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if (Q_stricmp(token, "loadmenu") == 0) {
			if (CG_Load_Menu(&p)) {
				continue;
			} else {
				break;
			}
		}
	}

	Com_Printf("UI menu load time = %d milli seconds\n", trap_Milliseconds() - start);

}

/*
=================
CG_FeederCount
=================
*/
static int CG_FeederCount(float feederID) {
	int i, count;
	count = 0;
	if (feederID == FEEDER_REDTEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_RED) {
				count++;
			}
		}
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == TEAM_BLUE) {
				count++;
			}
		}
	} else if (feederID == FEEDER_SCOREBOARD) {
		return cg.numScores;
	}
	return count;
}

/*
=================
CG_SetScoreSelection
=================
*/

void CG_SetScoreSelection(void *p) {
	menuDef_t *menu = (menuDef_t*)p;
	playerState_t *ps = &cg.snap->ps;
	int i, red, blue;
	red = blue = 0;
	for (i = 0; i < cg.numScores; i++) {
		if (cg.scores[i].team == TEAM_RED) {
			red++;
		} else if (cg.scores[i].team == TEAM_BLUE) {
			blue++;
		}
		if (ps->clientNum == cg.scores[i].client) {
			cg.selectedScore = i;
		}
	}

	if (menu == NULL) {
		// just interested in setting the selected score
		return;
	}

	if ( cgs.gametype >= GT_TEAM ) {
		int feeder = FEEDER_REDTEAM_LIST;
		i = red;
		if (cg.scores[cg.selectedScore].team == TEAM_BLUE) {
			feeder = FEEDER_BLUETEAM_LIST;
			i = blue;
		}
		Menu_SetFeederSelection(menu, feeder, i, NULL);
	} else {
		Menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
	}
}

/*
=================
CG_InfoFromScoreIndex
=================
*/
static clientInfo_t * CG_InfoFromScoreIndex(int index, int team, int *scoreIndex) {
	int i, count;
	if ( cgs.gametype >= GT_TEAM ) {
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (count == index) {
					*scoreIndex = i;
					return &cgs.clientinfo[cg.scores[i].client];
				}
				count++;
			}
		}
	}
	*scoreIndex = index;
	return &cgs.clientinfo[ cg.scores[index].client ];
}

/*
=================
CG_FeederItemText
=================
*/
static const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle) {
	//gitem_t *item;
	clientInfo_t *info = NULL;
	int scoreIndex = 0;
	int team = -1;
	int vehicleIndex = -1;
	qboolean spectator = qfalse;
	qboolean dontShowVehicle = qfalse;
	score_t *sp = NULL;
	centity_t *self = &cg.predictedPlayerEntity;

	*handle = -1;

	if (feederID == FEEDER_REDTEAM_LIST) {
		team = TEAM_RED;
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		team = TEAM_BLUE;
	}

	info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
	sp = &cg.scores[scoreIndex];

	// spectator?
	if (info->infoValid && info->team == TEAM_SPECTATOR )
	{
		spectator = qtrue;
	}

	if (info && info->infoValid)
	{
		switch (column)
		{
			// CTF flag icon/Handicap (etc.) (currently unused in MFQ3)
			case 0:
/*
				if ( info->powerups & ( 1 << PW_NEUTRALFLAG ) ) {
					item = BG_FindItemForPowerup( PW_NEUTRALFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_REDFLAG ) ) {
					item = BG_FindItemForPowerup( PW_REDFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else if ( info->powerups & ( 1 << PW_BLUEFLAG ) ) {
					item = BG_FindItemForPowerup( PW_BLUEFLAG );
					*handle = cg_items[ ITEM_INDEX(item) ].icon;
				} else {
					if ( info->botSkill > 0 && info->botSkill <= 5 ) {
						*handle = cgs.media.botSkillShaders[ info->botSkill - 1 ];
					} else if ( info->handicap < 100 ) {
					return va("%i", info->handicap );
					}
				}
*/
			break;

			// Team Task (currently unused in MFQ3)
			case 1:
/*
				if (team == -1) {
					return "";
				} else {
					*handle = CG_StatusHandle(info->teamTask);
				}
*/
			break;

			// Status
			case 2:
				if ( cg.snap->ps.stats[ STAT_CLIENTS_READY ] & ( 1 << sp->client ) )
				{
					return "Ready";
				}

				if (team == -1)
				{
					if (cgs.gametype == GT_TOURNAMENT)
					{
						return va("%i/%i", info->wins, info->losses);
					}
					else if (info->infoValid && info->team == TEAM_SPECTATOR )
					{
						return "Spectator";
					} 
					else
					{
						return "";
					}
				}
				else
				{
					if (info->teamLeader)
					{
						return "Leader";
					}
				}
			break;

			// Name
			case 3:
				// return name
				return info->name;
			break;

			// Vehicle
			case 4:

				// team game being played?
				if( cgs.gametype >= GT_TEAM )
				{
					// enemy? (different team identifier than us)
					if( info->team != cgs.clientinfo[ self->currentState.clientNum ].team )
					{
						// don't show vehicles for our enemies
						dontShowVehicle = qtrue;
					}
				}

				// get vehicle index
				vehicleIndex = info->vehicle;

				// is it valid?
				if( vehicleIndex >= 0 && !spectator && !dontShowVehicle )
				{
					// use name + vehicle description
					return availableVehicles[ vehicleIndex ].tinyName;
				}
				else
				{
					// was the information blocked?
					if( dontShowVehicle )
					{
						return "n/a";
					}
					else
					{
						// null
						return "-";
					}
				}
				break;

			// Score
			case 5:
				if( spectator )
				{
					return "-";
				}
				else
					return va("%i", info->score);
			break;

			// Deaths
			case 6:
				if( spectator )
				{
					return "-";
				}
				return va("%i", info->deaths);
			break;

			// Time
			case 7:
				return va("%4i", sp->time);
			break;

			// Ping
			case 8:
				if ( sp->ping == -1 )
				{
					return "Connect";
				} 
				return va("%4i", sp->ping);
			break;
		}
	}

	return "";
}

/*
=================
CG_FeederSelection
=================
*/
static void CG_FeederSelection(float feederID, int index) {
	if ( cgs.gametype >= GT_TEAM ) {
		int i, count;
		int team = (feederID == FEEDER_REDTEAM_LIST) ? TEAM_RED : TEAM_BLUE;
		count = 0;
		for (i = 0; i < cg.numScores; i++) {
			if (cg.scores[i].team == team) {
				if (index == count) {
					cg.selectedScore = i;
				}
				count++;
			}
		}
	} else {
		cg.selectedScore = index;
	}
}

#endif	// NOTE: functions copied from v1.29h code to enable new scoreboard method

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		CG_Error( "CG_ConfigString: bad index: %i", index );
	}
	return cgs.gameState.stringData + cgs.gameState.stringOffsets[ index ];
}

//==================================================================

/*
======================
CG_StartMusic

======================
*/
void CG_StartMusic( void ) {
	char	*s;
	char	parm1[MAX_QPATH], parm2[MAX_QPATH];

	// start the background music
	s = (char *)CG_ConfigString( CS_MUSIC );
	Q_strncpyz( parm1, COM_Parse( &s ), sizeof( parm1 ) );
	Q_strncpyz( parm2, COM_Parse( &s ), sizeof( parm2 ) );

	trap_S_StartBackgroundTrack( parm1, parm2 );
}

/*
=================
CG_Cvar_Get

Generic get value from Cvar
=================
*/
float CG_Cvar_Get( const char * cvar )
{
	// allocate temp string buffer
	char buff[128];
	memset( buff, 0, sizeof(buff) );

	// get variable as string
	trap_Cvar_VariableStringBuffer( cvar, buff, sizeof(buff) );

	// return converted value as float
	return atof( buff );
}

#ifdef _MENU_SCOREBOARD	// NOTE: functions copied from v1.29h code to enable new scoreboard method

/*
=================
CG_OwnerDrawWidth();
=================
*/
static int CG_OwnerDrawWidth(int ownerDraw, float scale) {
	switch (ownerDraw) {
	  case CG_GAME_TYPE:
			return CG_Text_Width(CG_GameTypeString(), scale, 0);
	  case CG_GAME_STATUS:
			return CG_Text_Width(CG_GetGameStatusText(), scale, 0);
			break;
/*
	  case CG_KILLER:
			return CG_Text_Width(CG_GetKillerText(), scale, 0);
			break;
	  case CG_RED_NAME:
			return CG_Text_Width(cg_redTeamName.string, scale, 0);
			break;
	  case CG_BLUE_NAME:
			return CG_Text_Width(cg_blueTeamName.string, scale, 0);
			break;
*/
	}
	return 0;
}

/*
=================
CG_LoadHudMenu();
=================
*/
void CG_LoadHudMenu() {
	char buff[1024];
	const char *hudSet;

	cgDC.registerShaderNoMip = &trap_R_RegisterShaderNoMip;
	cgDC.setColor = &trap_R_SetColor;
	cgDC.drawHandlePic = &CG_DrawPic;
	cgDC.drawStretchPic = &trap_R_DrawStretchPic;
	cgDC.drawText = &CG_Text_Paint;
	cgDC.textWidth = &CG_Text_Width;
	cgDC.textHeight = &CG_Text_Height;
	cgDC.registerModel = &trap_R_RegisterModel;
	cgDC.modelBounds = &trap_R_ModelBounds;
	cgDC.fillRect = &CG_FillRect;
	cgDC.drawRect = &CG_DrawRect;   
	cgDC.drawSides = &CG_DrawSides;
	cgDC.drawTopBottom = &CG_DrawTopBottom;
	cgDC.clearScene = &trap_R_ClearScene;
	cgDC.addRefEntityToScene = &trap_R_AddRefEntityToScene;
	cgDC.renderScene = &trap_R_RenderScene;
	cgDC.registerFont = &trap_R_RegisterFont;
	cgDC.ownerDrawItem = &CG_OwnerDraw;
	//cgDC.getValue = &CG_GetValue;
	//cgDC.ownerDrawVisible = &CG_OwnerDrawVisible;
	//cgDC.runScript = &CG_RunMenuScript;
	//cgDC.getTeamColor = &CG_GetTeamColor;
	cgDC.setCVar = trap_Cvar_Set;
	cgDC.getCVarString = trap_Cvar_VariableStringBuffer;
	cgDC.getCVarValue = CG_Cvar_Get;
	//cgDC.drawTextWithCursor = &CG_Text_PaintWithCursor;
	//cgDC.setOverstrikeMode = &trap_Key_SetOverstrikeMode;
	//cgDC.getOverstrikeMode = &trap_Key_GetOverstrikeMode;
	cgDC.startLocalSound = &trap_S_StartLocalSound;
	//cgDC.ownerDrawHandleKey = &CG_OwnerDrawHandleKey;
	cgDC.feederCount = &CG_FeederCount;
	//cgDC.feederItemImage = &CG_FeederItemImage;
	cgDC.feederItemText = &CG_FeederItemText;
	cgDC.feederSelection = &CG_FeederSelection;
	//cgDC.setBinding = &trap_Key_SetBinding;
	//cgDC.getBindingBuf = &trap_Key_GetBindingBuf;
	//cgDC.keynumToStringBuf = &trap_Key_KeynumToStringBuf;
	//cgDC.executeText = &trap_Cmd_ExecuteText;
	cgDC.Error = &Com_Error; 
	cgDC.Print = &Com_Printf; 
	cgDC.ownerDrawWidth = &CG_OwnerDrawWidth;
	//cgDC.Pause = &CG_Pause;
	cgDC.registerSound = &trap_S_RegisterSound;
	cgDC.startBackgroundTrack = &trap_S_StartBackgroundTrack;
	cgDC.stopBackgroundTrack = &trap_S_StopBackgroundTrack;
	//cgDC.playCinematic = &CG_PlayCinematic;
	//cgDC.stopCinematic = &CG_StopCinematic;
	//cgDC.drawCinematic = &CG_DrawCinematic;
	//cgDC.runCinematicFrame = &CG_RunCinematicFrame;
	
	Init_Display(&cgDC);

	Menu_Reset();
	
	trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));
	hudSet = buff;
	if (hudSet[0] == '\0') {
		hudSet = "ui/hud.txt";
	}

	CG_LoadMenus(hudSet);
}

/*
=================
CG_AssetCache
=================
*/

void CG_AssetCache()
{
	//if (Assets.textFont == NULL) {
	//  trap_R_RegisterFont("fonts/arial.ttf", 72, &Assets.textFont);
	//}
	//Assets.background = trap_R_RegisterShaderNoMip( ASSET_BACKGROUND );
	//Com_Printf("Menu Size: %i bytes\n", sizeof(Menus));
	cgDC.Assets.gradientBar = trap_R_RegisterShaderNoMip( ASSET_GRADIENTBAR );
	cgDC.Assets.verticalGradient = trap_R_RegisterShaderNoMip( ASSET_VERTICALGRADIENT );
	cgDC.Assets.fxBasePic = trap_R_RegisterShaderNoMip( ART_FX_BASE );
	cgDC.Assets.fxPic[0] = trap_R_RegisterShaderNoMip( ART_FX_RED );
	cgDC.Assets.fxPic[1] = trap_R_RegisterShaderNoMip( ART_FX_YELLOW );
	cgDC.Assets.fxPic[2] = trap_R_RegisterShaderNoMip( ART_FX_GREEN );
	cgDC.Assets.fxPic[3] = trap_R_RegisterShaderNoMip( ART_FX_TEAL );
	cgDC.Assets.fxPic[4] = trap_R_RegisterShaderNoMip( ART_FX_BLUE );
	cgDC.Assets.fxPic[5] = trap_R_RegisterShaderNoMip( ART_FX_CYAN );
	cgDC.Assets.fxPic[6] = trap_R_RegisterShaderNoMip( ART_FX_WHITE );
	cgDC.Assets.scrollBar = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgDC.Assets.scrollBarArrowDown = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgDC.Assets.scrollBarArrowUp = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgDC.Assets.scrollBarArrowLeft = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgDC.Assets.scrollBarArrowRight = trap_R_RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgDC.Assets.scrollBarThumb = trap_R_RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgDC.Assets.sliderBar = trap_R_RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgDC.Assets.sliderThumb = trap_R_RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}

#endif	// NOTE: functions copied from v1.29h code to enable new scoreboard method

/*
=================
CG_Init

Called after every level change or subsystem restart
Will perform callbacks to make the loading info screen update.
=================
*/
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum ) {
	const char	*s;

	// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );

	cg.clientNum = clientNum;
	
	// misc
	cg.cameraAdjustAmount = 0.0f;
	cg.cameraAdjustCount = 0;
	cg.cameraAdjustEnum = CAMADJ_NONE;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= trap_R_RegisterShader( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= trap_R_RegisterShader( "white" );
	cgs.media.charsetProp		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropGlow	= trap_R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
	cgs.media.charsetPropB		= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	// this has the effect of stopping the usual Q3 console text being displayed (because
	// we want to custom handle this textual display - but still retain the history that
	// gets written to the "drop-down" console)
#ifdef _MENU_SCOREBOARD
	trap_Cvar_Set( "con_notifytime", "-1" );
#endif

	cg.weaponSelect = WP_WEAPON1;

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	cgs.flagStatus = -1;
	// old servers

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	// MFQ3: vehicle needs to be reset
	if( cg_vehicle.integer != -1 ) {
		trap_Cvar_Set( "cg_vehicle", "-1" );
	}
	if( cg_nextVehicle.integer != -1 ) {
		trap_Cvar_Set( "cg_nextVehicle", "-1" );
	}

	// MFQ3: HUD color
	cg.HUDColor = hud_color.integer;
	if( cg.HUDColor >= HUD_MAX || cg.HUDColor < 0 ) {
		cg.HUDColor = 0;
		trap_Cvar_Set( "hud_color", va("%d", cg.HUDColor) );
	}

	// MFQ3: MFD color
	cg.MFDColor = mfd_color.integer;
	if( cg.MFDColor >= HUD_MAX || cg.MFDColor < 0 ) {
		cg.MFDColor = 0;
		trap_Cvar_Set( "mfd_color", va("%d", cg.MFDColor) );
	}

	// MFQ3: MFD pages
	cg.Mode_MFD[MFD_1] = mfd1_defaultpage.integer;
	if( cg.Mode_MFD[MFD_1] >= MFD_MAX ) {
		cg.Mode_MFD[MFD_1] = MFD_OFF;
		trap_Cvar_Set( "mfd1_defaultpage", va("%d", cg.Mode_MFD[MFD_1]) );
	}
	cg.Mode_MFD[MFD_2] = mfd2_defaultpage.integer;
	if( cg.Mode_MFD[MFD_2] >= MFD_MAX ) {
		cg.Mode_MFD[MFD_2] = MFD_OFF;
		trap_Cvar_Set( "mfd2_defaultpage", va("%d", cg.Mode_MFD[MFD_2]) );
	}

	CG_ParseServerinfo();

	// load the new map
	CG_LoadingString( "collision map" );

	trap_CM_LoadMap( cgs.mapname );

#ifdef _MENU_SCOREBOARD
	String_Init();
#endif

	CG_LoadingString( "sounds" );

	CG_RegisterSounds();

	CG_LoadingString( "graphics" );

	CG_RegisterGraphics();

	CG_LoadingString( "clients" );

	CG_RegisterClients();

#ifdef _MENU_SCOREBOARD
	CG_AssetCache();       // cache UI gfx
	CG_LoadHudMenu();      // load new hud stuff
#endif

	CG_InitLocalEntities();

	CG_InitMarkPolys();

	CG_RegisterWeapons();

	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_StartMusic();

	CG_LoadingString( "" );

	CG_ShaderStateChanged();

	trap_S_ClearLoopingSounds( qtrue );

}

/*
=================
CG_Shutdown

Called before every level change or subsystem restart
=================
*/
void CG_Shutdown( void ) {
	// some mods may need to do cleanup work here,
	// like closing files or archiving session data
}


/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
void CG_EventHandling(int type) {
}



void CG_KeyEvent(int key, qboolean down) {
}

void CG_MouseEvent(int x, int y) {
}

