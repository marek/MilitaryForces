/*
 * $Id: cg_consolecmds.c,v 1.22 2003-01-14 00:24:09 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"

static char* cycleCvars[] =
{
	"m1cp_page0",
	"m1cp_page1",
	"m1cp_page2",
	"m1cp_page3",
	"m1cp_page4",
	"m1cp_page5",
	"m2cp_page0",
	"m2cp_page1",
	"m2cp_page2",
	"m2cp_page3",
	"m2cp_page4",
	"m2cp_page5",
};


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
MFQ3 MFD Mode cycling
==================
*/
static void CG_Set_MFD_Mode( int num ) {
	qboolean found = qfalse;
	char buffer[33], value;
	int start = cg.Mode_MFD[num] - 1;
	if( start < 0 ) start = MFD_MAX-1;
	
	while( !found && cg.Mode_MFD[num] != start ) {
		cg.Mode_MFD[num]++;
		if( cg.Mode_MFD[num] >= MFD_MAX ) cg.Mode_MFD[num] = MFD_OFF;
		value = cg.Mode_MFD[num];
		if( num ) value += MFD_MAX;
		trap_Cvar_VariableStringBuffer( cycleCvars[value], buffer, 32 );
		value = atoi(buffer);
		if( value ) found = qtrue;
	}

	if( num == MFD_1 )
		trap_Cvar_Set( "mfd1_defaultpage", va("%d", cg.Mode_MFD[num]) );
	else if( num == MFD_2 )
		trap_Cvar_Set( "mfd2_defaultpage", va("%d", cg.Mode_MFD[num]) );
}

static void CG_Set_MFD1_Mode_f( void ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	CG_Set_MFD_Mode(MFD_1);

	cg.MFDTime = cg.time + 100;
}

static void CG_Set_MFD2_Mode_f( void ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	CG_Set_MFD_Mode(MFD_2);

	cg.MFDTime = cg.time + 100;
}


/*
==================
MFQ3 MFD Mode setting
==================
*/

static void CG_Set_MFD_Page( int num ) {
	int page;

	if( trap_Argc() < 2 ) {
		return;
	}

	page = atoi( CG_Argv(1) );

	if( page < 0 ) page = 0;
	else if( page >= MFD_MAX ) page = MFD_MAX - 1;

	cg.Mode_MFD[num] = page;

	if( num == MFD_1 )
		trap_Cvar_Set( "mfd1_defaultpage", va("%d", page) );
	else if( num == MFD_2 )
		trap_Cvar_Set( "mfd2_defaultpage", va("%d", page) );

}

static void CG_Set_MFD1_Page_f( void ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	CG_Set_MFD_Page(MFD_1);

	cg.MFDTime = cg.time + 100;
}

static void CG_Set_MFD2_Page_f( void ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	CG_Set_MFD_Page(MFD_2);

	cg.MFDTime = cg.time + 100;
}

/*
==================
MFQ3 HUD Color setting
==================
*/

static void CG_Update_HUD_Color_f( void )
{
	int val = -1;

	// update 
	cg.HUDColor = CG_Cvar_Get( "hud_color" );
}

/*
==================
MFQ3 HUD Color cycling
==================
*/

static void CG_Cycle_HUD_Color_f( void ) {
	if( cg.time < cg.HUDColorTime ) {
		return;
	}
	cg.HUDColor = hud_color.integer;
	cg.HUDColor++;
	if( cg.HUDColor >= HUD_MAX || cg.HUDColor < 0 ) {
		cg.HUDColor = 0;
	}
	trap_Cvar_Set( "hud_color", va("%d", cg.HUDColor) );

	cg.HUDColorTime = cg.time + 100;
}

/*
==================
MFQ3 MFD color setting
==================
*/

static void CG_Update_MFD_Color_f( void )
{
	int val = -1;

	// update 
	cg.MFDColor = CG_Cvar_Get( "mfd_color" );
}

/*
==================
MFQ3 MFD toggling
==================
*/

static void CG_Toggle_MFD1_f( void ) {
	int mfd1 = hud_mfd.integer;

	mfd1 = mfd1 ? 0 : 1;
	trap_Cvar_Set( "hud_mfd", va("%d", mfd1) );
	
}

static void CG_Toggle_MFD2_f( void ) {
	int mfd2 = hud_mfd2.integer;

	mfd2 = mfd2 ? 0 : 1;
	trap_Cvar_Set( "hud_mfd2", va("%d", mfd2) );

}


/*
==================
MFQ3 MFD color cycling
==================
*/

static void CG_Cycle_MFD_Color_f( void ) {
	if( cg.time < cg.MFDColorTime ) {
		return;
	}
	cg.MFDColor = mfd_color.integer;
	cg.MFDColor++;
	if( cg.MFDColor >= HUD_MAX || cg.MFDColor < 0 ) {
		cg.MFDColor = 0;
	}
	trap_Cvar_Set( "mfd_color", va("%d", cg.MFDColor) );

	cg.MFDColorTime = cg.time + 100;
}

/*
==================
MFQ3 MFD Camera mode cycling
==================
*/

static void CG_Cycle_Camera_Mode_f( void ) {
	if( cg.time < cg.CameraModeTime ) {
		return;
	}
	cg.CameraMode++;
	if( cg.CameraMode >= CAMERA_MAX || cg.CameraMode < 0 ) {
		cg.CameraMode = 0;
	}

	cg.CameraModeTime = cg.time + 100;
}

/*
==================
MFQ3 MFD Camera Mode setting
==================
*/

static void CG_Set_Camera_Mode_f( void ) {

	if( cg.time < cg.CameraModeTime ) {
		return;
	}

	if( trap_Argc() < 2 ) {
		return;
	}

	cg.CameraMode = atoi( CG_Argv(1) );
	if( cg.CameraMode >= CAMERA_MAX || cg.CameraMode < 0 ) {
		cg.CameraMode = 0;
	}

	cg.CameraModeTime = cg.time + 100;
}

/*
==================
MFQ3 MFD camera mode quick action keys
==================
*/

static void CG_CamBack_f( void )
{
	cg.CameraMode = CAMERA_BACK;
}

static void CG_CamDown_f( void )
{
	cg.CameraMode = CAMERA_DOWN;
}

static void CG_CamBomb_f( void )
{
	cg.CameraMode = CAMERA_BOMB;
}

static void CG_CamOld_f( void )
{
	cg.CameraMode = CAMERA_TARGET;
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
MFQ3 camera
==================
*/

static void CG_AdjustReset_f( void ) {
	cg.cameraAdjustEnum = CAMADJ_NONE;
	cg.cameraAdjustAmount = 0;
}

#define MOVE_BASIC	0.4f

/*
==================
MFQ3 camera zoom in/out
==================
*/
static void CG_ZoomIn_f( void ) {
	cg.cameraAdjustEnum = CAMADJ_INOUT;
	cg.cameraAdjustAmount = +MOVE_BASIC;
}

static void CG_ZoomOut_f( void ) {
	cg.cameraAdjustEnum = CAMADJ_INOUT;
	cg.cameraAdjustAmount = -MOVE_BASIC;
}

/*
==================
MFQ3 camera up/down
==================
*/
static void CG_CameraUp_f( void ) {
	cg.cameraAdjustEnum = CAMADJ_UPDOWN;
	cg.cameraAdjustAmount = +MOVE_BASIC;
}

static void CG_CameraDown_f( void ) {
	cg.cameraAdjustEnum = CAMADJ_UPDOWN;
	cg.cameraAdjustAmount = -MOVE_BASIC;
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

static void CG_Spawn_GI_f( void ) {
	int idx;

	if( trap_Argc() < 2 ) return;

	idx = atoi( CG_Argv(1) );

	if( idx < 0 || idx >= bg_numberOfGroundInstallations ) {
		CG_Printf("GroundInstallation number out of range (0-%d)\n", bg_numberOfGroundInstallations-1 );
		return;
	}

	ME_SpawnGroundInstallation(idx);

}

static void CG_ExportToScript_f( void ) {
	if( trap_Argc() < 2 ) 
	{
		CG_Printf("Scriptname not specified!");
		return;
	}

	ME_ExportToScript( CG_Argv(1) );

}

/*
=================
CG_ExtractDeveloper

Intercepts 'say <var> <values>' commands for development purposes
=================
*/

qboolean CG_ExtractDeveloper( void )
{
	clientInfo_t * ci = NULL;

	float value = 0;

	// get client information if we can
	if( cg.snap )
	{
		ci = &cgs.clientinfo[ cg.snap->ps.clientNum ];
	}
	else
	{
		// no information, nothing possible
		return qfalse;
	}

	// convert 2nd param into a value
	value = atof( CG_Argv(1) );

	// check 2nd param against known variables

	if( !Q_stricmp( CG_Argv(0), "ox" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_XOFFSET ] = value;
		return qtrue;
	}
	else if( !Q_stricmp( CG_Argv(0), "oy" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_YOFFSET ] = value;
		return qtrue;
	}
	else if( !Q_stricmp( CG_Argv(0), "ax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_XADJUST ] = value;
		return qtrue;
	}
	else if( !Q_stricmp( CG_Argv(0), "ay" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_YADJUST ] = value;
		return qtrue;
	}
	else if( !Q_stricmp( CG_Argv(0), "pmax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_PITCHMAX ] = value;
		return qtrue;
	}
	else if( !Q_stricmp( CG_Argv(0), "rmax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_ROLLMAX ] = value;
		return qtrue;
	}
	else if( !Q_stricmp( CG_Argv(0), "pmod" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_PITCHMOD ] = value;
		return qtrue;
	}
	else if( !Q_stricmp( CG_Argv(0), "rmod" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_ROLLMOD ] = value;
		return qtrue;
	}

	return qfalse;
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
	{ "+zoomin", CG_ZoomIn_f },
	{ "-zoomin", CG_AdjustReset_f },
	{ "+zoomout", CG_ZoomOut_f },
	{ "-zoomout", CG_AdjustReset_f },
	{ "+cameraup", CG_CameraUp_f },
	{ "-cameraup", CG_AdjustReset_f },
	{ "+cameradown", CG_CameraDown_f },
	{ "-cameradown", CG_AdjustReset_f },
	{ "radarrange", CG_ToggleRadarRange_f },
	{ "mfd1_mode", CG_Set_MFD1_Mode_f },
	{ "mfd2_mode", CG_Set_MFD2_Mode_f },
	{ "mfd1_page", CG_Set_MFD1_Page_f },
	{ "mfd2_page", CG_Set_MFD2_Page_f },
	{ "cycle_hud_color", CG_Cycle_HUD_Color_f },
	{ "cycle_mfd_color", CG_Cycle_MFD_Color_f },
	{ "cycle_camera_mode", CG_Cycle_Camera_Mode_f },
	{ "set_camera_mode", CG_Set_Camera_Mode_f },
	{ "+backcam", CG_CamBack_f },
	{ "-backcam", CG_CamOld_f },
	{ "+downcam", CG_CamDown_f },
	{ "-downcam", CG_CamOld_f },
	{ "+bombcam", CG_CamBomb_f },
	{ "-bombcam", CG_CamOld_f },
	{ "toggle_mfd1", CG_Toggle_MFD1_f },
	{ "toggle_mfd2", CG_Toggle_MFD2_f },
	{ "update_hud_color", CG_Update_HUD_Color_f },
	{ "update_mfd_color", CG_Update_MFD_Color_f },
	{ "me_spawn_gi", CG_Spawn_GI_f },
	{ "me_export_to_script", CG_ExportToScript_f },
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

	// developer variable modification
	if( CG_ExtractDeveloper() )
	{
		return qtrue;
	}

	// scan though all known client commands
	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) 
	{
		// match?
		if ( !Q_stricmp( cmd, commands[i].cmd ) )
		{
			// execute associated fn()
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
