/*
 * $Id: cg_main.c,v 1.16 2005-11-26 10:06:29 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_main.c -- initialization and primary entry point for cgame
#include "cg_local.h"

#include "../ui/ui_shared.h"
#include "ui_displaycontext_cg.h"
#include "ui_utils_cg.h"
#include "../ui/ui_precomp.h"

// display context for New ui stuff
//displayContextDef_t cgDC;
//UI_DisplayContextCG cgUtils.dc_;


UI_UtilsCG cgUtils;

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
//int vmMain( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11  ) {
//
//	switch ( command ) {
//	case CG_INIT:
//		CG_Init( arg0, arg1, arg2 );
//		return 0;
//	case CG_SHUTDOWN:
//		CG_Shutdown();
//		return 0;
//	case CG_CONSOLE_COMMAND:
//		return CG_ConsoleCommand();
//	case CG_DRAW_ACTIVE_FRAME:
//		CG_DrawActiveFrame( arg0, static_cast<stereoFrame_t>(arg1), arg2 );
//		return 0;
//	case CG_CROSSHAIR_PLAYER:
//		return CG_CrosshairPlayer();
	//case CG_LAST_ATTACKER:
	//	return CG_LastAttacker();
//	case CG_KEY_EVENT:
//		CG_KeyEvent(arg0, arg1);
//		return 0;
//	case CG_MOUSE_EVENT:
//		CG_MouseEvent(arg0, arg1);
//		return 0;
//	case CG_EVENT_HANDLING:
//		CG_EventHandling(arg0);
//		return 0;
//	default:
//		CG_Error( "vmMain: unknown command %i", command );
//		break;
//	}
//	return -1;
//}


cg_t				cg;
cgs_t				cgs;
centity_t			cg_entities[MAX_GENTITIES];
completeLoadout_t	cg_loadouts[MAX_GENTITIES];
weaponInfo_t		cg_weapons[WI_MAX];
itemInfo_t			cg_items[MAX_ITEMS];
particleWorld_t		pw;

// (unused cVars)
//vmCvar_t	cg_drawCrosshair;
//vmCvar_t	cg_pmove_fixed;
//vmCvar_t	cg_thirdPersonAngle;

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
vmCvar_t	cg_drawParticleStats;
vmCvar_t	cg_drawSnapshot;
vmCvar_t	cg_draw3dIcons;
vmCvar_t	cg_drawIcons;
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
//vmCvar_t	pmove_fixed;
//vmCvar_t	pmove_msec;
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

// MFQ3: development cVars
vmCvar_t	cg_shadowDebug;

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

cvarTable_t		cvarTable[] = {
//	{ &cg_pmove_fixed, "cg_pmove_fixed", "1", CVAR_USERINFO | CVAR_ARCHIVE },
//	{ &cg_thirdPersonAngle, "cg_thirdPersonAngle", "0", 0 },
//	{ &cg_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },

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
	{ &cg_drawParticleStats, "cg_drawParticleStats", "0", CVAR_ARCHIVE  },
	{ &cg_drawSnapshot, "cg_drawSnapshot", "0", CVAR_ARCHIVE  },
	{ &cg_draw3dIcons, "cg_draw3dIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawIcons, "cg_drawIcons", "1", CVAR_ARCHIVE  },
	{ &cg_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &cg_drawRewards, "cg_drawRewards", "1", CVAR_ARCHIVE },
	{ &cg_crosshairSize, "cg_crosshairSize", "1", CVAR_ARCHIVE },	// default: small
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
	{ &cg_advanced, "cg_advanced", "0", CVAR_ARCHIVE | CVAR_USERINFO },
	{ &cg_synchronousClients, "g_synchronousClients", "0", 0 },	// communicated by systeminfo

	{ &cg_timescaleFadeEnd, "cg_timescaleFadeEnd", "1", 0},
	{ &cg_timescaleFadeSpeed, "cg_timescaleFadeSpeed", "0", 0},
	{ &cg_timescale, "timescale", "1", 0},
	{ &cg_smoothClients, "cg_smoothClients", "0", CVAR_USERINFO | CVAR_ARCHIVE},
	{ &cg_cameraMode, "com_cameraMode", "0", CVAR_CHEAT},

	// MFQ3+
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

	{ &pmove_fixed, "pmove_fixed", "1", CVAR_ROM },
	{ &pmove_msec, "pmove_msec", "8", 0 },
	{ &cg_noTaunt, "cg_noTaunt", "0", CVAR_ARCHIVE },
	{ &cg_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE },
	{ &cg_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE },

// MFQ3: development cVars
	{ &cg_shadowDebug, "cg_shadowDebug", "0", CVAR_ARCHIVE},	// set this to 1 to enable the shadow development code

	// MFQ3-
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
		Cvar_Register( cv->vmCvar, cv->cvarName,
			cv->defaultString, cv->cvarFlags );
	}

	// see if we are also running the server on this machine
	Cvar_VariableStringBuffer( "sv_running", var, sizeof( var ) );
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
void CG_UpdateCvars( void ) 
{
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		Cvar_Update( cv->vmCvar );
	}

	// check for modications here

	// If team overlay is on, ask for updates from the server.  If its off,
	// let the server know so we don't receive it
	if ( drawTeamOverlayModificationCount != cg_drawTeamOverlay.modificationCount ) 
	{
		drawTeamOverlayModificationCount = cg_drawTeamOverlay.modificationCount;

		if ( cg_drawTeamOverlay.integer > 0 ) 
		{
			Cvar_Set( "teamoverlay", "1" );
		} 
		else 
		{
			Cvar_Set( "teamoverlay", "0" );
		}
		// FIXME E3 HACK
		Cvar_Set( "teamoverlay", "1" );
	}

	// if force model changed
	if ( forceModelModificationCount != cg_forceModel.modificationCount ) 
	{
		forceModelModificationCount = cg_forceModel.modificationCount;
		CG_ForceModelChange();
	}
	cg.tracernum = (cg_tracer.integer > 0) ? cg_tracer.integer : 3;
}

int CG_CrosshairPlayer( void ) 
{
	if ( cg.time > ( cg.crosshairClientTime + 1000 ) )
		return -1;
	
	return cg.crosshairClientNum;
}

int CG_LastAttacker( void ) 
{
	if ( !cg.attackerTime ) 
		return -1;

	return cg.snap->ps.persistant[PERS_ATTACKER];
}

void QDECL CG_Printf( const char *msg, ... ) 
{
	va_list		argptr;
	char		text[1024];

	va_start (argptr, msg);
	vsprintf (text, msg, argptr);
	va_end (argptr);

	// add the text into the custom consol
	CG_Add_Console_Line( text );

	// write the text into the console (only displayed if the cVar 'con_notifytime' is not -1)
	Com_Printf( "%s", text );
}

/*
================
CG_Argv
================
*/
const char *CG_Argv( int arg ) 
{
	static char	buffer[MAX_STRING_CHARS];

	Cmd_ArgvBuffer( arg, buffer, sizeof( buffer ) );

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
		S_RegisterSound( item->pickup_sound, false );
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
			Com_Error( ERR_DROP, "PrecacheItem: %s has bad precache string", 
				item->classname);
			return;
		}
		memcpy (data, start, len);
		data[len] = 0;
		if ( *s ) {
			s++;
		}

		if ( !strcmp(data+len-3, "wav" )) {
			S_RegisterSound( data, false );
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

	cgs.media.oneMinuteSound = S_RegisterSound( "sound/feedback/1_minute.wav", false );
	cgs.media.fiveMinuteSound = S_RegisterSound( "sound/feedback/5_minute.wav", false );
	cgs.media.suddenDeathSound = S_RegisterSound( "sound/feedback/sudden_death.wav", false );
	cgs.media.oneFragSound = S_RegisterSound( "sound/feedback/1_frag.wav", false );
	cgs.media.twoFragSound = S_RegisterSound( "sound/feedback/2_frags.wav", false );
	cgs.media.threeFragSound = S_RegisterSound( "sound/feedback/3_frags.wav", false );
	cgs.media.count3Sound = S_RegisterSound( "sound/feedback/three.wav", false );
	cgs.media.count2Sound = S_RegisterSound( "sound/feedback/two.wav", false );
	cgs.media.count1Sound = S_RegisterSound( "sound/feedback/one.wav", false );
	cgs.media.countFightSound = S_RegisterSound( "sound/feedback/fight.wav", false );
	cgs.media.countPrepareSound = S_RegisterSound( "sound/feedback/prepare.wav", false );

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {

		cgs.media.captureAwardSound = S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", false );
		cgs.media.redLeadsSound = S_RegisterSound( "sound/feedback/redleads.wav", false );
		cgs.media.blueLeadsSound = S_RegisterSound( "sound/feedback/blueleads.wav", false );
		cgs.media.teamsTiedSound = S_RegisterSound( "sound/feedback/teamstied.wav", false );
		cgs.media.hitTeamSound = S_RegisterSound( "sound/feedback/hit_teammate.wav", false );

		cgs.media.redScoredSound = S_RegisterSound( "sound/teamplay/voc_red_scores.wav", false );
		cgs.media.blueScoredSound = S_RegisterSound( "sound/teamplay/voc_blue_scores.wav", false );

		cgs.media.captureYourTeamSound = S_RegisterSound( "sound/teamplay/flagcapture_yourteam.wav", false );
		cgs.media.captureOpponentSound = S_RegisterSound( "sound/teamplay/flagcapture_opponent.wav", false );

		cgs.media.returnYourTeamSound = S_RegisterSound( "sound/teamplay/flagreturn_yourteam.wav", false );
		cgs.media.returnOpponentSound = S_RegisterSound( "sound/teamplay/flagreturn_opponent.wav", false );

		cgs.media.takenYourTeamSound = S_RegisterSound( "sound/teamplay/flagtaken_yourteam.wav", false );
		cgs.media.takenOpponentSound = S_RegisterSound( "sound/teamplay/flagtaken_opponent.wav", false );

		if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
			cgs.media.redFlagReturnedSound = S_RegisterSound( "sound/teamplay/voc_red_returned.wav", false );
			cgs.media.blueFlagReturnedSound = S_RegisterSound( "sound/teamplay/voc_blue_returned.wav", false );
			cgs.media.enemyTookYourFlagSound = S_RegisterSound( "sound/teamplay/voc_enemy_flag.wav", false );
			cgs.media.yourTeamTookEnemyFlagSound = S_RegisterSound( "sound/teamplay/voc_team_flag.wav", false );
		}

		cgs.media.youHaveFlagSound = S_RegisterSound( "sound/teamplay/voc_you_flag.wav", false );
		cgs.media.holyShitSound = S_RegisterSound( "sound/feedback/voc_holyshit.wav", false);
		cgs.media.yourTeamTookTheFlagSound = S_RegisterSound( "sound/teamplay/voc_team_1flag.wav", false );
		cgs.media.enemyTookTheFlagSound = S_RegisterSound( "sound/teamplay/voc_enemy_1flag.wav", false );
	}

	cgs.media.respawnSound = S_RegisterSound( "sound/items/respawn1.wav", false );

	cgs.media.talkSound = S_RegisterSound( "sound/player/talk.wav", false );

	cgs.media.hitSound = S_RegisterSound( "sound/feedback/hit.wav", false );

	cgs.media.impressiveSound = S_RegisterSound( "sound/feedback/impressive.wav", false );
	cgs.media.excellentSound = S_RegisterSound( "sound/feedback/excellent.wav", false );
	cgs.media.deniedSound = S_RegisterSound( "sound/feedback/denied.wav", false );
	cgs.media.assistSound = S_RegisterSound( "sound/feedback/assist.wav", false );
	cgs.media.defendSound = S_RegisterSound( "sound/feedback/defense.wav", false );

	cgs.media.takenLeadSound = S_RegisterSound( "sound/feedback/takenlead.wav", false);
	cgs.media.tiedLeadSound = S_RegisterSound( "sound/feedback/tiedlead.wav", false);
	cgs.media.lostLeadSound = S_RegisterSound( "sound/feedback/lostlead.wav", false);

	cgs.media.watrInSound = S_RegisterSound( "sound/player/watr_in.wav", false);
	cgs.media.watrOutSound = S_RegisterSound( "sound/player/watr_out.wav", false);
	cgs.media.watrUnSound = S_RegisterSound( "sound/player/watr_un.wav", false);

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
		cgs.gameSounds[i] = S_RegisterSound( soundName, false );
	}

	// FIXME: only needed with item
	cgs.media.sfx_ric1 = S_RegisterSound( "sound/weapons/machinegun/ric1.wav", false);
	cgs.media.sfx_ric2 = S_RegisterSound( "sound/weapons/machinegun/ric2.wav", false);
	cgs.media.sfx_ric3 = S_RegisterSound( "sound/weapons/machinegun/ric3.wav", false);
	cgs.media.sfx_rockexp = S_RegisterSound( "sound/weapons/rocket/rocklx1a.wav", false);

	cgs.media.n_healthSound = S_RegisterSound( "sound/items/n_health.wav", false );

	// MFQ3
	cgs.media.engineJet = S_RegisterSound( "sound/engines/jet.wav", false );
	cgs.media.engineProp = S_RegisterSound( "sound/engines/prop.wav", false );
	cgs.media.engineJetAB = S_RegisterSound( "sound/engines/afterburner.wav", false );
	cgs.media.engineHelo = S_RegisterSound( "sound/engines/helo.wav", false );

	// Nuclear Bomb
	cgs.media.nukeExplodeSound = S_RegisterSound( "sound/items/kam_explode.wav", false );
	cgs.media.nukeImplodeSound = S_RegisterSound( "sound/items/kam_implode.wav", false );
	cgs.media.nukeFarSound = S_RegisterSound( "sound/items/nuke_far.wav", false );
	cgs.media.nukeEffectModel = refExport.RegisterModel( "models/weaphits/nukeblast.md3" );
	cgs.media.nukeShockWave = refExport.RegisterModel( "models/weaphits/nukeshockwave.md3" );
	cgs.media.nukeCloudModel = refExport.RegisterModel( "models/weaphits/nukecloud.md3" );

	// MFQ3: New sounds
	cgs.media.planeDeath[0] = S_RegisterSound( "sound/explosions/explode1.wav", false );	
	cgs.media.planeDeath[1] = S_RegisterSound( "sound/explosions/explode2.wav", false );	
	cgs.media.planeDeath[2] = S_RegisterSound( "sound/explosions/explode3.wav", false );	
	cgs.media.planeDeath[3] = S_RegisterSound( "sound/explosions/explode4.wav", false );	
	if( !cgs.media.planeDeath[0] )
	{
		// MFQ3: old quake3 sound (backup)
		cgs.media.planeDeath[0] = S_RegisterSound( "sound/death/planedeath.wav", false );	
		cgs.media.planeDeath[1] = cgs.media.planeDeath[0];
		cgs.media.planeDeath[2] = cgs.media.planeDeath[0];
		cgs.media.planeDeath[3] = cgs.media.planeDeath[0];
	}

	// added mg
	cgs.media.engineTank[0] = S_RegisterSound( "sound/engines/tank0.wav", false );
	cgs.media.engineTank[1] = S_RegisterSound( "sound/engines/tank1.wav", false );
	cgs.media.engineTank[2] = S_RegisterSound( "sound/engines/tank2.wav", false );
	cgs.media.engineTank[3] = S_RegisterSound( "sound/engines/tank3.wav", false );
	cgs.media.engineTank[4] = S_RegisterSound( "sound/engines/tank4.wav", false );
	cgs.media.engineTank[5] = S_RegisterSound( "sound/engines/tank5.wav", false );
	cgs.media.engineTank[6] = S_RegisterSound( "sound/engines/tank6.wav", false );
	cgs.media.engineTank[7] = S_RegisterSound( "sound/engines/tank7.wav", false );
	cgs.media.engineTank[8] = S_RegisterSound( "sound/engines/tank8.wav", false );

}



//===================================================================================


/*
=================
CG_PrecacheVehiclePreview

Simply does a refExport.RegisterShaderNoMip() for all our icons
=================
*/
static void CG_PrecacheVehiclePreview( void )
{
	char * pIconString = NULL;
	int i = 0;
	qhandle_t h;

	// update loading display
	CG_LoadingString( "precache icons" );

	// for all icons
	for( i = 0; i < bg_numberOfVehicles; i++ )
	{
		// create filename string
		pIconString = MF_CreateModelPathname( i, "models/vehicles/%s/%s/%s_icon" );

		// precache
		h = refExport.RegisterShaderNoMip( pIconString );
	}
}

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
		"newhud/reticle/reticle3.tga",
		"newhud/reticle/reticle4.tga",
		"newhud/reticle/reticle5.tga",
		"newhud/reticle/reticle6.tga",
		"newhud/reticle/reticle7.tga",
		"newhud/reticle/reticle8.tga",
		"newhud/reticle/reticle9.tga",
		"newhud/reticle/reticle10.tga"
	};

	// clear any references to old media
	memset( &cg.refdef, 0, sizeof( cg.refdef ) );
	refExport.ClearScene();

	CG_LoadingString( cgs.mapname );

	refExport.LoadWorld( cgs.mapname );

	// precache status bar pics
	CG_LoadingString( "game media" );

	for ( i=0 ; i<11 ; i++) {
		cgs.media.numberShaders[i] = refExport.RegisterShader( sb_nums[i] );
	}

	cgs.media.smokePuffShader = refExport.RegisterShader( "smokePuff" );
	cgs.media.smokePuffRageProShader = refExport.RegisterShader( "smokePuffRagePro" );
	cgs.media.lagometerShader = refExport.RegisterShader( "lagometer" );
	cgs.media.connectionShader = refExport.RegisterShader( "disconnected" );


	// LQM gibs
	cgs.media.gibAbdomen = refExport.RegisterModel( "models/gibs/abdomen.md3" );
	cgs.media.gibArm = refExport.RegisterModel( "models/gibs/arm.md3" );
	cgs.media.gibChest = refExport.RegisterModel( "models/gibs/chest.md3" );
	cgs.media.gibFist = refExport.RegisterModel( "models/gibs/fist.md3" );
	cgs.media.gibFoot = refExport.RegisterModel( "models/gibs/foot.md3" );
	cgs.media.gibForearm = refExport.RegisterModel( "models/gibs/forearm.md3" );
	cgs.media.gibIntestine = refExport.RegisterModel( "models/gibs/intestine.md3" );
	cgs.media.gibLeg = refExport.RegisterModel( "models/gibs/leg.md3" );
	cgs.media.gibSkull = refExport.RegisterModel( "models/gibs/skull.md3" );
	cgs.media.gibBrain = refExport.RegisterModel( "models/gibs/brain.md3" );

	// MFQ3
	cgs.media.missilePuffShader = refExport.RegisterShader( "missilePuff" );
	cgs.media.flarePuffShader = refExport.RegisterShader( "flarePuff" );
	cgs.media.railCoreShader = refExport.RegisterShader( "railCore" );
	cgs.media.missileTrail2Shader = refExport.RegisterShader( "missileTrail2" );
	cgs.media.nukePuffShader = refExport.RegisterShader( "nukePuff" );

	// disable for now for faster loading and less mem use
	//for( i = 0; i < NUKE_CLOUD_FRAMES; i++) {
	//	cgs.media.nukeCloud[i] = refExport.RegisterShader( va("nukecloud%d", i) );
	//}
	
	cgs.media.selectShader = refExport.RegisterShader( "gfx/2d/select" );

	for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
		cgs.media.crosshairShader[i] = refExport.RegisterShader( va("gfx/2d/crosshair%c", 'a'+i) );
	}

	cgs.media.backTileShader = refExport.RegisterShader( "gfx/2d/backtile" );
	cgs.media.noammoShader = refExport.RegisterShader( "icons/noammo" );


	if ( cgs.gametype == GT_CTF || cg_buildScript.integer ) {
		cgs.media.redFlagModel = refExport.RegisterModel( "models/flags/r_flag.md3" );
		cgs.media.blueFlagModel = refExport.RegisterModel( "models/flags/b_flag.md3" );
		cgs.media.redFlagShader[0] = refExport.RegisterShaderNoMip( "icons/iconf_red1" );
		cgs.media.redFlagShader[1] = refExport.RegisterShaderNoMip( "icons/iconf_red2" );
		cgs.media.redFlagShader[2] = refExport.RegisterShaderNoMip( "icons/iconf_red3" );
		cgs.media.blueFlagShader[0] = refExport.RegisterShaderNoMip( "icons/iconf_blu1" );
		cgs.media.blueFlagShader[1] = refExport.RegisterShaderNoMip( "icons/iconf_blu2" );
		cgs.media.blueFlagShader[2] = refExport.RegisterShaderNoMip( "icons/iconf_blu3" );
	}

	if ( cgs.gametype >= GT_TEAM || cg_buildScript.integer ) {
		cgs.media.friendShader = refExport.RegisterShader( "sprites/foe" );
		cgs.media.teamStatusBar = refExport.RegisterShader( "gfx/2d/colorbar.tga" );
	}

	cgs.media.balloonShader = refExport.RegisterShader( "sprites/balloon3" );
	cgs.media.particleShader = refExport.RegisterShader( "gfx/misc/particle.tga" );

	// MFQ3 stuff
	cgs.media.vapor = refExport.RegisterModel( "models/effects/vapor.md3" );
	cgs.media.vaporBig = refExport.RegisterModel( "models/effects/vapor_big.md3" );
	cgs.media.vaporShader = refExport.RegisterModel( "models/effects/turbulence" );
	cgs.media.afterburner[AB_BALL] = refExport.RegisterModel( "models/effects/burner_ball.md3" );
	cgs.media.afterburner[AB_BLUE] = refExport.RegisterModel( "models/effects/burner_blue.md3" );
	cgs.media.afterburner[AB_RED] = refExport.RegisterModel( "models/effects/burner_red.md3" );
	cgs.media.afterburner[AB_RED_SMALL] = refExport.RegisterModel( "models/effects/burner_red_small.md3" );
	cgs.media.vehicleMuzzleFlashModel = refExport.RegisterModel( "models/effects/muzzleflash_md.md3");
	cgs.media.bulletTracer = refExport.RegisterModel("models/effects/tracer_yellow.md3");
	cgs.media.pilot = refExport.RegisterModel( "models/effects/pilot_body.md3" );
	cgs.media.pilotHead = refExport.RegisterModel( "models/effects/pilot_head.md3" );
	cgs.media.pilotSeat = refExport.RegisterModel("models/effects/pilot_seat.md3" );
	cgs.media.pilotWW2 = refExport.RegisterModel( "models/effects/pilot_body_ww2.md3" );
	cgs.media.pilotHeadWW2 = refExport.RegisterModel( "models/effects/pilot_head_ww2.md3" );
	cgs.media.pilotSeatWW2 = refExport.RegisterModel( "models/effects/pilot_seat_ww2.md3" );
	cgs.media.reticle[CH_GUNMODE] = refExport.RegisterModel( "models/effects/ch_gunmode.md3" );
	cgs.media.reticle[CH_BOMBMODE] = refExport.RegisterModel( "models/effects/ch_agmode.md3" );
	cgs.media.reticle[CH_ROCKETMODE] = refExport.RegisterModel( "models/effects/ch_rocketmode.md3" );
	cgs.media.reticle[CH_MISSILEMODE] = refExport.RegisterModel( "models/effects/ch_missilemode.md3" );
	cgs.media.reticle[CH_MISSILEMODELOCK] = refExport.RegisterModel( "models/effects/ch_missilemodelock.md3" );
	cgs.media.reticle[CH_DEFAULT] = refExport.RegisterModel( "models/effects/ch_default.md3" );
	// end MFQ3

	// MFQ3 HUD
	cgs.media.HUDmain = refExport.RegisterShaderNoMip( "hud/main.tga" );
	cgs.media.HUDradar = refExport.RegisterShaderNoMip( "hud/radar.tga" );
	cgs.media.HUDgps = refExport.RegisterShaderNoMip( "hud/gps.tga" );
	cgs.media.HUDext = refExport.RegisterShaderNoMip( "hud/side.tga" );
	for( i = 0; i <= MF_THROTTLE_AFTERBURNER; i++ ) {
		cgs.media.throttle[i] = refExport.RegisterShaderNoMip( va("hud/%i.tga",i));
	}
	for( i = 1; i < bg_numberOfWeapons; i++ ) {
		availableWeapons[i].iconHandle = refExport.RegisterShaderNoMip( availableWeapons[i].iconName );
		if( availableWeapons[i].modelName && strlen(availableWeapons[i].modelName) ) {
			availableWeapons[i].modelHandle = refExport.RegisterModel(availableWeapons[i].modelName);
		}
		if( availableWeapons[i].vwepName && strlen(availableWeapons[i].vwepName) ) {
			availableWeapons[i].vwepHandle = refExport.RegisterModel( availableWeapons[i].vwepName);
		}
	}
	for ( i=0 ; i< RD_MAX_ICONS ; i++) {
		cgs.media.radarIcons[i] = refExport.RegisterShaderNoMip( rd_icons[i] );
	}
	// end MFQ3 HUD

	// MFQ3 New HUD
	cgs.media.HUDheading = refExport.RegisterShaderNoMip( "newhud/heading_tape.tga" );
	cgs.media.HUDvaluebox = refExport.RegisterShaderNoMip( "newhud/valuebox.tga" );
	cgs.media.HUDvaluebox2 = refExport.RegisterShaderNoMip( "newhud/valuebox2.tga" );
	cgs.media.HUDind_h = refExport.RegisterShaderNoMip( "newhud/ind_h.tga" );
	cgs.media.HUDind_v = refExport.RegisterShaderNoMip( "newhud/ind_v.tga" );
	cgs.media.HUDind_v_r = refExport.RegisterShaderNoMip( "newhud/ind_v_r.tga" );
	cgs.media.HUDcaret_h = refExport.RegisterShaderNoMip( "newhud/caret_g_h.tga" );
	cgs.media.HUDcaret_v_g_l = refExport.RegisterShaderNoMip( "newhud/caret_g_v.tga" );
	cgs.media.HUDcaret_v_r_l = refExport.RegisterShaderNoMip( "newhud/caret_r_v.tga" );
	cgs.media.HUDmfd = refExport.RegisterShaderNoMip( "newhud/mfd.tga" );
	cgs.media.HUDspeed = refExport.RegisterShaderNoMip( "newhud/speed_tape.tga" );
	cgs.media.HUDrwr = refExport.RegisterShaderNoMip( "newhud/rwr.tga" );
	cgs.media.HUDalt = refExport.RegisterShaderNoMip( "newhud/alt_tape.tga" );
	cgs.media.HUDsolid = refExport.RegisterShaderNoMip( "newhud/solid.tga" );
	for ( i=0 ; i<10 ; i++) {
		cgs.media.HUDhealth[i] = refExport.RegisterShaderNoMip( mfq3_health[i] );
	}
	for ( i=0 ; i<15 ; i++) {
		cgs.media.HUDthrottle_1_ab[i] = refExport.RegisterShaderNoMip( mfq3_throttle_1_ab[i] );
		cgs.media.HUDthrottle_2_1_ab[i] = refExport.RegisterShaderNoMip( mfq3_throttle_2_1_ab[i] );
		cgs.media.HUDthrottle_2_2_ab[i] = refExport.RegisterShaderNoMip( mfq3_throttle_2_2_ab[i] );
	}
	for ( i=0 ; i<10 ; i++) {
		cgs.media.HUDthrottle_1[i] = refExport.RegisterShaderNoMip( mfq3_throttle_1[i] );
		cgs.media.HUDthrottle_2_1[i] = refExport.RegisterShaderNoMip( mfq3_throttle_2_1[i] );
		cgs.media.HUDthrottle_2_2[i] = refExport.RegisterShaderNoMip( mfq3_throttle_2_2[i] );
	}
	for ( i=0 ; i<5 ; i++) {
		cgs.media.HUDthrottle_rev[i] = refExport.RegisterShaderNoMip( mfq3_throttle_rev[i] );
	}
	cgs.media.HUDhealthtext = refExport.RegisterShaderNoMip( "newhud/text/hlth.tga" );
	cgs.media.HUDthrottletext = refExport.RegisterShaderNoMip( "newhud/text/thr.tga" );
	cgs.media.HUDalphabet = refExport.RegisterShaderNoMip( "newhud/alphabet.tga" );
	cgs.media.HUDnumberline = refExport.RegisterShaderNoMip( "newhud/numberline.tga" );
	for ( i=0 ; i<HR_MAX ; i++) {
		cgs.media.HUDreticles[i] = refExport.RegisterShaderNoMip( mfq3_hudrets[i] );
	}
	// end MFQ3 New HUD

	// MFQ3 IGME
	if( cgs.gametype == GT_MISSION_EDITOR ) {
		cgs.media.IGME_selector = refExport.RegisterModel( "models/effects/selector.md3");
		cgs.media.IGME_waypoint = refExport.RegisterShaderNoMip( "models/tex_shared/powerup_green");
		cgs.media.IGME_waypoint2 = refExport.RegisterShaderNoMip( "models/tex_shared/powerup_gray");
	}
	// end MFQ3 IGME

	// MFQ3: precache the vehicle icons here (they are used by the UI, but registered models
	// are engine-global)
	CG_PrecacheVehiclePreview();

	cgs.media.bulletFlashModel = refExport.RegisterModel( "models/weaphits/bullet.md3");
	cgs.media.dishFlashModel = refExport.RegisterModel( "models/weaphits/boom01.md3");

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
	cgs.media.bulletMarkShader = refExport.RegisterShader( "gfx/damage/bullet_mrk" );
	cgs.media.burnMarkShader = refExport.RegisterShader( "gfx/damage/burn_med_mrk" );

	// default shadows (one per class of vehicle - remember that vehicles can specify their own shadows if perferred)
	for( i = 0; i < MF_MAX_CATEGORIES; i ++ )
	{
		cgs.media.shadowMarkShader[i] = refExport.RegisterShader( va( "%sshadow", cat_fileRef[i] ) );
	}

	// register the inline models
	cgs.numInlineModels = CM_NumInlineModels();
	for ( i = 1 ; i < cgs.numInlineModels ; i++ ) {
		char name[10];
		vec3_t mins, maxs;
		int	j;

		Com_sprintf( name, sizeof(name), "*%i", i );
		cgs.inlineDrawModel[i] = refExport.RegisterModel( name );
		refExport.ModelBounds( cgs.inlineDrawModel[i], mins, maxs );
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
		cgs.gameModels[i] = refExport.RegisterModel( modelName);
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

	// NOTE: functions copied from v1.29h code to enable New scoreboard method

/*
=================
CG_Asset_Parse
=================
*/
bool CG_Asset_Parse(int handle) {
	UI_PrecompilerTools::PC_Token token;
	const char *tempStr;

	if (!cgUtils.getPrecompilerTools()->readTokenHandle(handle, &token))
		return false;
	if (Q_stricmp(token.string_, "{") != 0) {
		return false;
	}
    
	while ( 1 ) {
		if (!cgUtils.getPrecompilerTools()->readTokenHandle(handle, &token))
			return false;

		if (Q_stricmp(token.string_, "}") == 0) {
			return true;
		}

		// font
		if (Q_stricmp(token.string_, "font") == 0) {
			int pointSize;
			if (!cgUtils.pc_String_Parse(handle, &tempStr) || 
				!cgUtils.pc_Int_Parse(handle, &pointSize)) {
				return false;
			}
			cgUtils.dc_->registerFont(tempStr, pointSize, &cgUtils.dc_->assets_.textFont);
			continue;
		}

		// smallFont
		if (Q_stricmp(token.string_, "smallFont") == 0) {
			int pointSize;
			if (!cgUtils.pc_String_Parse(handle, &tempStr) || 
				!cgUtils.pc_Int_Parse(handle, &pointSize)) {
				return false;
			}
			cgUtils.dc_->registerFont(tempStr, pointSize, &cgUtils.dc_->assets_.smallFont);
			continue;
		}

		// font
		if (Q_stricmp(token.string_, "bigfont") == 0) {
			int pointSize;
			if (!cgUtils.pc_String_Parse(handle, &tempStr) || 
				!cgUtils.pc_Int_Parse(handle, &pointSize)) {
				return false;
			}
			cgUtils.dc_->registerFont(tempStr, pointSize, &cgUtils.dc_->assets_.bigFont);
			continue;
		}

		// gradientbar
		if (Q_stricmp(token.string_, "gradientbar") == 0) {
			if (!cgUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			cgUtils.dc_->assets_.gradientBar = refExport.RegisterShaderNoMip(tempStr);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string_, "menuEnterSound") == 0) {
			if (!cgUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			cgUtils.dc_->assets_.menuEnterSound = S_RegisterSound( tempStr, false );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string_, "menuExitSound") == 0) {
			if (!cgUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			cgUtils.dc_->assets_.menuExitSound = S_RegisterSound( tempStr, false );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string_, "itemFocusSound") == 0) {
			if (!cgUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			cgUtils.dc_->assets_.itemFocusSound = S_RegisterSound( tempStr, false );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string_, "menuBuzzSound") == 0) {
			if (!cgUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			cgUtils.dc_->assets_.menuBuzzSound = S_RegisterSound( tempStr, false );
			continue;
		}

		if (Q_stricmp(token.string_, "cursor") == 0) {
			if (!cgUtils.pc_String_Parse(handle, &cgUtils.dc_->assets_.cursorStr)) {
				return false;
			}
			cgUtils.dc_->assets_.cursor = refExport.RegisterShaderNoMip( cgUtils.dc_->assets_.cursorStr);
			continue;
		}

		if (Q_stricmp(token.string_, "cursorWait") == 0) {
			if (!cgUtils.pc_String_Parse(handle, &cgUtils.dc_->assets_.cursorStrWait)) {
				return false;
			}
			cgUtils.dc_->assets_.cursorWait = refExport.RegisterShaderNoMip( cgUtils.dc_->assets_.cursorStrWait );
			continue;
		}

		if (Q_stricmp(token.string_, "fadeClamp") == 0) {
			if (!cgUtils.pc_Float_Parse(handle, &cgUtils.dc_->assets_.fadeClamp)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "fadeCycle") == 0) {
			if (!cgUtils.pc_Int_Parse(handle, &cgUtils.dc_->assets_.fadeCycle)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "fadeAmount") == 0) {
			if (!cgUtils.pc_Float_Parse(handle, &cgUtils.dc_->assets_.fadeAmount)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "shadowX") == 0) {
			if (!cgUtils.pc_Float_Parse(handle, &cgUtils.dc_->assets_.shadowX)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "shadowY") == 0) {
			if (!cgUtils.pc_Float_Parse(handle, &cgUtils.dc_->assets_.shadowY)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "shadowColor") == 0) {
			if (!cgUtils.pc_Color_Parse(handle, &cgUtils.dc_->assets_.shadowColor)) {
				return false;
			}
			cgUtils.dc_->assets_.shadowFadeClamp = cgUtils.dc_->assets_.shadowColor[3];
			continue;
		}
	}
	return false; // bk001204 - why not?
}

/*
=================
CG_ParseMenu
=================
*/
void CG_ParseMenu(const char *menuFile) {
	UI_PrecompilerTools::PC_Token token;
	int handle;

	handle = cgUtils.getPrecompilerTools()->loadSourceHandle(menuFile); 
	if (!handle)
		handle = cgUtils.getPrecompilerTools()->loadSourceHandle("ui/testhud.menu");
	if (!handle)
		return;

	while ( 1 ) {
		if (!cgUtils.getPrecompilerTools()->readTokenHandle( handle, &token )) {
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

		if ( token.string_[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string_, "assetGlobalDef") == 0) {
			if (CG_Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}

		if (Q_stricmp(token.string_, "menudef") == 0) {
			// start a New menu
			cgUtils.menu_New(handle);
		}
	}
	cgUtils.getPrecompilerTools()->freeSourceHandle(handle);
}

/*
=================
CG_Load_Menu
=================
*/
bool CG_Load_Menu(char **p) {
	char *token;

	token = COM_ParseExt(p, true);

	if (token[0] != '{') {
		return false;
	}

	while ( 1 ) {

		token = COM_ParseExt(p, true);
    
		if (Q_stricmp(token, "}") == 0) {
			return true;
		}

		if ( !token || token[0] == 0 ) {
			return false;
		}

		CG_ParseMenu(token); 
	}
	return false;
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

	start = Sys_Milliseconds();

	len = FS_FOpenFileByMode( menuFile, &f, FS_READ );
	if ( !f ) {
		Com_Error( ERR_DROP, S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile );
		len = FS_FOpenFileByMode( "ui/hud.txt", &f, FS_READ );
		if (!f) {
			Com_Error( ERR_DROP, S_COLOR_RED "default menu file not found: ui/hud.txt, unable to continue!\n", menuFile );
		}
	}

	if ( len >= MAX_MENUDEFFILE ) {
		Com_Error( ERR_DROP, S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", menuFile, len, MAX_MENUDEFFILE );
		FS_FCloseFile( f );
		return;
	}

	FS_Read2( buf, len, f );
	buf[len] = 0;
	FS_FCloseFile( f );
	
	COM_Compress(buf);

	cgUtils.menu_Reset();

	p = buf;

	while ( 1 ) {
		token = COM_ParseExt( &p, true );
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

}

/*
=================
CG_FeederCount
=================
*/
int CG_FeederCount(float feederID) {
	int i, count;
	count = 0;

	// red team list?
	if (feederID == FEEDER_REDTEAM_LIST)
	{
		for (i = 0; i < cg.numScores; i++)
		{
			// only count red
			if (cg.scores[i].team == TEAM_RED)
			{
				count++;
			}
		}
	}
	// blue team list?
	else if (feederID == FEEDER_BLUETEAM_LIST)
	{
		for (i = 0; i < cg.numScores; i++)
		{
			// only count blue
			if (cg.scores[i].team == TEAM_BLUE)
			{
				count++;
			}
		}
	}
	// free-for-all list?
	else if (feederID == FEEDER_SCOREBOARD)
	{
		for (i = 0; i < cg.numScores; i++)
		{
			// DONT count spectators
			if (cg.scores[i].team != TEAM_SPECTATOR)
			{
				count++;
			}
		}
	}
	// spectator list?
	else if (feederID == FEEDER_SPECTATOR_LIST)
	{
		for (i = 0; i < cg.numScores; i++)
		{
			// only count spectators
			if (cg.scores[i].team == TEAM_SPECTATOR)
			{
				count++;
			}
		}
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
		cgUtils.menu_SetFeederSelection(menu, feeder, i, NULL);
	} else {
		cgUtils.menu_SetFeederSelection(menu, FEEDER_SCOREBOARD, cg.selectedScore, NULL);
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
const char *CG_FeederItemText(float feederID, int index, int column, qhandle_t *handle) 
{
	//gitem_t *item;
	clientInfo_t *info = NULL;
	int scoreIndex = 0;
	int team = -1;
	int vehicleIndex = -1;
	bool spectator = false;
	bool dontShowVehicle = false;
	score_t *sp = NULL;
	centity_t *self = &cg.predictedPlayerEntity;

	*handle = -1;

	if (feederID == FEEDER_REDTEAM_LIST) {
		team = TEAM_RED;
	} else if (feederID == FEEDER_BLUETEAM_LIST) {
		team = TEAM_BLUE;
	} else if (feederID == FEEDER_SPECTATOR_LIST) {
		team = TEAM_SPECTATOR;
	}

	info = CG_InfoFromScoreIndex(index, team, &scoreIndex);
	sp = &cg.scores[scoreIndex];

	// spectator?
	if (info->infoValid && info->team == TEAM_SPECTATOR )
	{
		spectator = true;
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

				if( spectator )
				{
					return "";
				}

				// team game being played?
				if( cgs.gametype >= GT_TEAM )
				{
					// enemy? (different team identifier than us)
					if( info->team != cgs.clientinfo[ self->currentState.clientNum ].team )
					{
						// don't show vehicles for our enemies
						dontShowVehicle = true;
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
					return "";
				}
				else
					return va("%i", info->score);
			break;

			// Deaths
			case 6:
				if( spectator )
				{
					return "";
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
void CG_FeederSelection(float feederID, int index) 
{
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

// NOTE: functions copied from v1.29h code to enable New scoreboard method

//===========================================================================

/*
=================
CG_ConfigString
=================
*/
const char *CG_ConfigString( int index ) {
	if ( index < 0 || index >= MAX_CONFIGSTRINGS ) {
		Com_Error( ERR_DROP, "CG_ConfigString: bad index: %i", index );
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

	S_StartBackgroundTrack( parm1, parm2 );
}

// NOTE: functions copied from v1.29h code to enable New scoreboard method

/*
=================
CG_OwnerDrawWidth();
=================
*/
int CG_OwnerDrawWidth(int ownerDraw, float scale) 
{
	switch (ownerDraw)
	{
	  case CG_GAME_TYPE:
			return CG_Text_Width(CG_GameTypeString(), scale, 0);

	  case CG_GAME_STATUS:
			return CG_Text_Width(CG_GetGameStatusText(), scale, 0);
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

	cgUtils.menu_Reset();
	
	Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));
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
	cgUtils.dc_->assets_.gradientBar = refExport.RegisterShaderNoMip( ASSET_GRADIENTBAR );
	cgUtils.dc_->assets_.verticalGradient = refExport.RegisterShaderNoMip( ASSET_VERTICALGRADIENT );
	cgUtils.dc_->assets_.fxBasePic = refExport.RegisterShaderNoMip( ART_FX_BASE );
	cgUtils.dc_->assets_.fxPic[0] = refExport.RegisterShaderNoMip( ART_FX_RED );
	cgUtils.dc_->assets_.fxPic[1] = refExport.RegisterShaderNoMip( ART_FX_YELLOW );
	cgUtils.dc_->assets_.fxPic[2] = refExport.RegisterShaderNoMip( ART_FX_GREEN );
	cgUtils.dc_->assets_.fxPic[3] = refExport.RegisterShaderNoMip( ART_FX_TEAL );
	cgUtils.dc_->assets_.fxPic[4] = refExport.RegisterShaderNoMip( ART_FX_BLUE );
	cgUtils.dc_->assets_.fxPic[5] = refExport.RegisterShaderNoMip( ART_FX_CYAN );
	cgUtils.dc_->assets_.fxPic[6] = refExport.RegisterShaderNoMip( ART_FX_WHITE );
	cgUtils.dc_->assets_.scrollBar = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR );
	cgUtils.dc_->assets_.scrollBarArrowDown = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	cgUtils.dc_->assets_.scrollBarArrowUp = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	cgUtils.dc_->assets_.scrollBarArrowLeft = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	cgUtils.dc_->assets_.scrollBarArrowRight = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	cgUtils.dc_->assets_.scrollBarThumb = refExport.RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	cgUtils.dc_->assets_.sliderBar = refExport.RegisterShaderNoMip( ASSET_SLIDER_BAR );
	cgUtils.dc_->assets_.sliderThumb = refExport.RegisterShaderNoMip( ASSET_SLIDER_THUMB );
}

// NOTE: functions copied from v1.29h code to enable New scoreboard method

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
	memset( cg_loadouts, 0, sizeof(cg_loadouts) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );

	cg.clientNum = clientNum;
	
	// misc
	cg.cameraAdjustAmount = 0.0f;
	cg.cameraAdjustCount = 0;
	cg.cameraAdjustEnum = CAMADJ_NONE;
	cg.zoomAmount = 1;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader		= refExport.RegisterShader( "gfx/2d/bigchars" );
	cgs.media.whiteShader		= refExport.RegisterShader( "white" );
	cgs.media.charsetProp		= refExport.RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropGlow	= refExport.RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
	cgs.media.charsetPropB		= refExport.RegisterShaderNoMip( "menu/art/font2_prop.tga" );

	CG_RegisterCvars();

	CG_InitConsoleCommands();

	// this has the effect of stopping the usual Q3 console text being displayed (because
	// we want to custom handle this textual display - but still retain the history that
	// gets written to the "drop-down" console)
	Cvar_Set( "con_notifytime", "-1" );

	cg.weaponSelect = WP_WEAPON1;

	cgs.redflag = cgs.blueflag = -1; // For compatibily, default to unset for
	cgs.flagStatus = -1;
	// old servers

	// get the rendering configuration from the client system
	CL_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	CL_GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, MF_VERSION ) ) {
		Com_Error( ERR_DROP, "Client/Server game mismatch: %s/%s", MF_VERSION, s );
	}

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	// MFQ3: vehicle needs to be reset
	if( cg_vehicle.integer != -1 ) {
		Cvar_Set( "cg_vehicle", "-1" );
	}
	if( cg_nextVehicle.integer != -1 ) {
		Cvar_Set( "cg_nextVehicle", "-1" );
	}

	// MFQ3: HUD color
	cg.HUDColor = hud_color.integer;
	if( cg.HUDColor >= HUD_MAX || cg.HUDColor < 0 ) {
		cg.HUDColor = 0;
		Cvar_Set( "hud_color", va("%d", cg.HUDColor) );
	}

	// MFQ3: MFD color
	cg.MFDColor = mfd_color.integer;
	if( cg.MFDColor >= HUD_MAX || cg.MFDColor < 0 ) {
		cg.MFDColor = 0;
		Cvar_Set( "mfd_color", va("%d", cg.MFDColor) );
	}

	// MFQ3: MFD pages
	cg.Mode_MFD[MFD_1] = mfd1_defaultpage.integer;
	if( cg.Mode_MFD[MFD_1] >= MFD_MAX ) {
		cg.Mode_MFD[MFD_1] = MFD_OFF;
		Cvar_Set( "mfd1_defaultpage", va("%d", cg.Mode_MFD[MFD_1]) );
	}
	cg.Mode_MFD[MFD_2] = mfd2_defaultpage.integer;
	if( cg.Mode_MFD[MFD_2] >= MFD_MAX ) {
		cg.Mode_MFD[MFD_2] = MFD_OFF;
		Cvar_Set( "mfd2_defaultpage", va("%d", cg.Mode_MFD[MFD_2]) );
	}

	CG_ParseServerinfo();

	// load the New map
	CG_LoadingString( "collision map" );

	CL_CM_LoadMap( cgs.mapname );

	cgUtils.string_Init();

	CG_LoadingString( "sounds" );
	CG_LoadingString( "graphics" );

	CG_RegisterSounds();
	CG_RegisterGraphics();
	CG_RegisterWeapons();

	CG_LoadingString( "clients" );

	CG_RegisterClients();

	CG_AssetCache();       // cache UI gfx
	CG_LoadHudMenu();      // load New hud stuff

	CG_InitLocalEntities();

	CG_InitMarkPolys();


	// remove the last loading update
	cg.infoScreenText[0] = 0;

	// Make sure we have update values (scores)
	CG_SetConfigValues();

	CG_StartMusic();

	CG_LoadingString( "" );

	CG_ShaderStateChanged();

	S_ClearLoopingSounds( true );

	// MFQ3 vehicles
	MF_LoadAllVehicleData();

	// init all the vehicle's shadow system
	CG_InitShadows();

	// mission editor
	if( cgs.gametype == GT_MISSION_EDITOR )
		ME_Init_MissionEditor();


	// Add a particle system origin
	/*{
		particleSystem_t	ps;
		particleEmitter_t	pe;
		particle_t			p;
		int					i;

		memset(&pw, 0, sizeof(pw));
		memset(&ps, 0, sizeof(ps));
		memset(&pe, 0, sizeof(pe));
		memset(&p, 0, sizeof(p));
		
		
		CG_ParticleWorldInit(&pw);

		ps.cent = NULL;
		ps.dead = false;
		ps.lifetime = 0;
		VectorSet(ps.pos, 0, 0, 3000);
		i = CG_AddParticleSystem(&pw, &ps);
		if(i >= 0)
		{
			p.lifetime = 20000;
			p.gravity = 1600;
			p.radius = 10;
			p.alphaDecayStartValue = 1.0f;
			p.alphaDecayEndValue = 1.0f;
			//p.alphaDecayStartTime = 1000;
			//p.alphaDecayEndTime = 10000;
			p.bounce = 1.0f;
			p.numShaders = 1;
			p.dieOnCollision = false;
			p.shaders[0] = cgs.media.particleShader;
			p.rotationvel = 1.0f;
			p.growstart = 0;
			p.growend = 20000;
			p.growrate = 5.0f;
		//	p.attractionEmitter = -500.0f;
		//	p.attractionEntity = 500.0f;

			pe.ejectInterval = 10;
			VectorSet(pe.ejectVelocity,0.0f,0.0f,500.0f);
			VectorSet(pe.accelVarianceMax,100.0f,100.0f,0.0f);
			VectorSet(pe.accelVarianceMin,-100.0f,-100.0f,0.0f);
			//VectorClear(pe.ejectVelocity);
			pe.lifetime = 0;
			pe.parentSytem = i;
			pe.numEjectPerInterval = 5;
			pe.particleTemplate = p;
			i = CG_AddParticleEmitter(&pw, &pe);
			pw.emitters[i].particleTemplate.parentEmitter = i;
		}
	}*/

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

	// clean up missioneditor
	memset( &cgs.IGME, 0, sizeof( cgs.IGME ) );
}

/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      1 - team menu
      2 - hud editor

*/
void CG_EventHandling(int type)
{
}

/*
=================
CG_KeyEvent
=================
*/
void CG_KeyEvent(int key, bool down)
{
	if( cgs.gametype == GT_MISSION_EDITOR ) {
		ME_KeyEvent(key, down);
	}
}

/*
=================
CG_MouseEvent
=================
*/
void CG_MouseEvent(int x, int y)
{
	if( cgs.gametype == GT_MISSION_EDITOR ) {
		ME_MouseEvent(x, y);
	}
}
