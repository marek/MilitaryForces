/*
 * $Id: cg_consolecmds.c,v 1.3 2002-01-19 02:24:02 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"


void CG_TargetCommand_f( void ) {
	int		targetNum;
	char	test[4];

	targetNum = CG_CrosshairPlayer();
	if (!targetNum ) {
		return;
	}

	trap_Argv( 1, test, 4 );
	trap_SendConsoleCommand( va( "gc %i %i", targetNum, atoi( test ) ) );
}


/*
==================
MFQ3 GPS
==================
*/
static void CG_ToggleGps_f( void ) {

	if( cg.time < cg.GPSTime ) {
		return;
	}
	cg.GPS = cg.GPS ? 0 : 1;
	cg.GPSTime = cg.time + 500;
}

static void CG_ToggleRadarRange_f( void ) {

	if( cg.time < cg.RADARTime ) {
		return;
	}
	cg.RADARRangeSetting++;
	if( cg.RADARRangeSetting > 2 ) cg.RADARRangeSetting = 0;

	cg.RADARTime = cg.time + 100;
}

/*
==================
MFQ3 INFO
==================
*/
static void CG_ToggleInfo_f( void ) {

	if( cg.time < cg.INFOTime ) {
		return;
	}
	cg.INFO = cg.INFO ? 0 : 1;
	cg.INFOTime = cg.time + 500;
}

/*
==================
MFQ3 toggle view between external and cockpit view
==================
*/
static void CG_ToggleView_f( void ) {

	if( cg.time < cg.toggleViewTime ) {
		return;
	}

	if( !cg_thirdPerson.integer ) {
    	trap_Cvar_Set("cg_thirdPerson", va("%i",1));
	} else {
    	trap_Cvar_Set("cg_thirdPerson", va("%i",0));
	}
	cg.toggleViewTime = cg.time + 500;
}


/*
==================
MFQ3 zoom
==================
*/
static void CG_ZoomIn_f( void ) {
	trap_Cvar_Set("cg_thirdPersonRange", va("%i",(int)(cg_thirdPersonRange.integer-1)));
}

static void CG_ZoomOut_f( void ) {
	trap_Cvar_Set("cg_thirdPersonRange", va("%i",(int)(cg_thirdPersonRange.integer+1)));
}

/*
==================
MFQ3 camera up/down
==================
*/
static void CG_CameraUp_f( void ) {
	trap_Cvar_Set("cg_thirdPersonHeight", va("%i",(int)(cg_thirdPersonHeight.integer+1)));
}

static void CG_CameraDown_f( void ) {
	trap_Cvar_Set("cg_thirdPersonHeight", va("%i",(int)(cg_thirdPersonHeight.integer-1)));
}


/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	CG_Printf ("(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdefViewAngles[YAW]);
}


static void CG_ScoresDown_f( void ) {

	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request new ones
		cg.scoresRequestTime = cg.time;
		trap_SendClientCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = qtrue;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = qtrue;
	}
}

static void CG_ScoresUp_f( void ) {
	if ( cg.showScores ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	}
}

static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

static void CG_TestPlane_f( void ) {
	CG_TestVehicle(CAT_PLANE);
}

static void CG_TestGV_f( void ) {
	CG_TestVehicle(CAT_GROUND);
}


typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "testPlane", CG_TestPlane_f },
	{ "testGV", CG_TestGV_f },
	{ "testPlaneCmd", CG_TestPlaneCmd_f },
	{ "testGVCmd", CG_TestGVCmd_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
	{ "-scores", CG_ScoresUp_f },
	{ "+zoom", CG_ZoomDown_f },
	{ "-zoom", CG_ZoomUp_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_attacker", CG_TellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "gps", CG_ToggleGps_f },
	{ "extinfo", CG_ToggleInfo_f },
	{ "toggleview", CG_ToggleView_f },
	{ "zoomin", CG_ZoomIn_f },
	{ "zoomout", CG_ZoomOut_f },
	{ "cameraup", CG_CameraUp_f },
	{ "cameradown", CG_CameraDown_f },
	{ "radarrange", CG_ToggleRadarRange_f },
};


/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	int		i;

	cmd = CG_Argv(0);

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand ("kill");
	trap_AddCommand ("say");
	trap_AddCommand ("say_team");
	trap_AddCommand ("tell");
	trap_AddCommand ("vsay");
	trap_AddCommand ("vsay_team");
	trap_AddCommand ("vtell");
	trap_AddCommand ("vtaunt");
	trap_AddCommand ("vosay");
	trap_AddCommand ("vosay_team");
	trap_AddCommand ("votell");
	trap_AddCommand ("noclip");
	trap_AddCommand ("team");
	trap_AddCommand ("follow");
	trap_AddCommand ("levelshot");
	trap_AddCommand ("addbot");
	trap_AddCommand ("setviewpos");
	trap_AddCommand ("callvote");
	trap_AddCommand ("vote");
	trap_AddCommand ("callteamvote");
	trap_AddCommand ("teamvote");
	trap_AddCommand ("stats");
	trap_AddCommand ("teamtask");
	trap_AddCommand ("loaddefered");	// spelled wrong, but not changing for demo
	trap_AddCommand ("contact_tower");
	trap_AddCommand ("radar");
	trap_AddCommand ("unlock");
}
