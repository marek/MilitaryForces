/*
 * $Id: cg_consolecmds.c,v 1.4 2005-11-21 17:28:20 thebjoern Exp $
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


void CG_TargetCommand_f( ) {
	int		targetNum;
	char	test[4];

	targetNum = ClientGame::crosshairPlayer();
	if ( targetNum == -1 ) {
		return;
	}

	CommandSystem::argvBuffer( 1, test, 4 );
	CommandBuffer::addText( va( "gc %i %i", targetNum, atoi( test ) ) );
}


/*
==================
MFQ3 GPS
==================
*/
void ClientGame::toggleGps_f( ) {

	if( cg.time < cg.GPSTime ) {
		return;
	}
	cg.GPS = cg.GPS ? 0 : 1;
	cg.GPSTime = cg.time + 500;
}

void ClientGame::toggleRadarRange_f( ) {

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
void ClientGame::set_MFD_Mode( int num ) {
	bool found = false;
	char buffer[33], value;
	int start = cg.Mode_MFD[num] - 1;
	if( start < 0 ) start = MFD_MAX-1;
	
	while( !found && cg.Mode_MFD[num] != start ) {
		cg.Mode_MFD[num]++;
		if( cg.Mode_MFD[num] >= MFD_MAX ) cg.Mode_MFD[num] = MFD_OFF;
		value = cg.Mode_MFD[num];
		if( num ) value += MFD_MAX;
		CvarSystem::variableStringBuffer( cycleCvars[value], buffer, 32 );
		value = atoi(buffer);
		if( value ) found = true;
	}

	if( num == MFD_1 )
		CvarSystem::set( "mfd1_defaultpage", va("%d", cg.Mode_MFD[num]) );
	else if( num == MFD_2 )
		CvarSystem::set( "mfd2_defaultpage", va("%d", cg.Mode_MFD[num]) );
}

void ClientGame::set_MFD1_Mode_f( ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	ClientGame::set_MFD_Mode(MFD_1);

	cg.MFDTime = cg.time + 100;
}

void ClientGame::set_MFD2_Mode_f( ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	ClientGame::set_MFD_Mode(MFD_2);

	cg.MFDTime = cg.time + 100;
}


/*
==================
MFQ3 MFD Mode setting
==================
*/

void ClientGame::set_MFD_Page( int num ) {
	int page;

	if( CommandSystem::argc() < 2 ) {
		return;
	}

	page = atoi( ClientGame::argv(1) );

	if( page < 0 ) page = 0;
	else if( page >= MFD_MAX ) page = MFD_MAX - 1;

	cg.Mode_MFD[num] = page;

	if( num == MFD_1 )
		CvarSystem::set( "mfd1_defaultpage", va("%d", page) );
	else if( num == MFD_2 )
		CvarSystem::set( "mfd2_defaultpage", va("%d", page) );

}

void ClientGame::set_MFD1_Page_f( ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	ClientGame::set_MFD_Page(MFD_1);

	cg.MFDTime = cg.time + 100;
}

void ClientGame::set_MFD2_Page_f( ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	ClientGame::set_MFD_Page(MFD_2);

	cg.MFDTime = cg.time + 100;
}

/*
==================
MFQ3 HUD Color setting
==================
*/

void ClientGame::update_HUD_Color_f( )
{
	// update 
	cg.HUDColor = ClientGame::cvar_Get( "hud_color" );
}

/*
==================
MFQ3 HUD Color cycling
==================
*/

void ClientGame::cycle_HUD_Color_f( ) {
	if( cg.time < cg.HUDColorTime ) {
		return;
	}
	cg.HUDColor = hud_color.integer;
	cg.HUDColor++;
	if( cg.HUDColor >= HUD_MAX || cg.HUDColor < 0 ) {
		cg.HUDColor = 0;
	}
	CvarSystem::set( "hud_color", va("%d", cg.HUDColor) );

	cg.HUDColorTime = cg.time + 100;
}

/*
==================
MFQ3 MFD color setting
==================
*/

void ClientGame::update_MFD_Color_f( )
{
	// update 
	cg.MFDColor = ClientGame::cvar_Get( "mfd_color" );
}

/*
==================
MFQ3 MFD toggling
==================
*/

void ClientGame::toggle_MFD1_f( ) {
	int mfd1 = hud_mfd.integer;

	mfd1 = mfd1 ? 0 : 1;
	CvarSystem::set( "hud_mfd", va("%d", mfd1) );
	
}

void ClientGame::toggle_MFD2_f( ) {
	int mfd2 = hud_mfd2.integer;

	mfd2 = mfd2 ? 0 : 1;
	CvarSystem::set( "hud_mfd2", va("%d", mfd2) );

}


/*
==================
MFQ3 MFD color cycling
==================
*/

void ClientGame::cycle_MFD_Color_f( ) {
	if( cg.time < cg.MFDColorTime ) {
		return;
	}
	cg.MFDColor = mfd_color.integer;
	cg.MFDColor++;
	if( cg.MFDColor >= HUD_MAX || cg.MFDColor < 0 ) {
		cg.MFDColor = 0;
	}
	CvarSystem::set( "mfd_color", va("%d", cg.MFDColor) );

	cg.MFDColorTime = cg.time + 100;
}

/*
==================
MFQ3 MFD Camera mode cycling
==================
*/

void ClientGame::cycle_Camera_Mode_f( ) {
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

void ClientGame::set_Camera_Mode_f( ) {

	if( cg.time < cg.CameraModeTime ) {
		return;
	}

	if( CommandSystem::argc() < 2 ) {
		return;
	}

	cg.CameraMode = atoi( ClientGame::argv(1) );
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

void ClientGame::camBack_f( )
{
	cg.CameraMode = CAMERA_BACK;
}

void ClientGame::camDown_f( )
{
	cg.CameraMode = CAMERA_DOWN;
}

void ClientGame::camBomb_f( )
{
	cg.CameraMode = CAMERA_BOMB;
}

void ClientGame::camOld_f( )
{
	cg.CameraMode = CAMERA_TARGET;
}

/*
==================
MFQ3 INFO
==================
*/
void ClientGame::toggleInfo_f( ) {

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
void ClientGame::toggleView_f( ) {

	if( cg.time < cg.toggleViewTime ) {
		return;
	}

	if( !cg_thirdPerson.integer ) {
    	CvarSystem::set("cg_thirdPerson", va("%i",1));
	} else {
    	CvarSystem::set("cg_thirdPerson", va("%i",0));
	}
	cg.toggleViewTime = cg.time + 500;
}

/*
==================
MFQ3 camera
==================
*/

void ClientGame::adjustReset_f( ) {
	cg.cameraAdjustEnum = CAMADJ_NONE;
	cg.cameraAdjustAmount = 0;
}

constexpr float MOVE_BASIC = 0.4f;

/*
==================
MFQ3 camera zoom in/out
==================
*/
void ClientGame::zoomIn_f( ) {
	cg.cameraAdjustEnum = CAMADJ_INOUT;
	cg.cameraAdjustAmount = +MOVE_BASIC;
}

void ClientGame::zoomOut_f( ) {
	cg.cameraAdjustEnum = CAMADJ_INOUT;
	cg.cameraAdjustAmount = -MOVE_BASIC;
}

/*
==================
MFQ3 camera up/down
==================
*/
void ClientGame::cameraUp_f( ) {
	cg.cameraAdjustEnum = CAMADJ_UPDOWN;
	cg.cameraAdjustAmount = +MOVE_BASIC;
}

void ClientGame::cameraDown_f( ) {
	cg.cameraAdjustEnum = CAMADJ_UPDOWN;
	cg.cameraAdjustAmount = -MOVE_BASIC;
}


/*
=================
ClientGame::sizeUp_f

Keybinding command
=================
*/
void ClientGame::sizeUp_f() {
	CvarSystem::set("cg_viewsize", va("%i",(cg_viewsize.integer+10)));
}


/*
=================
ClientGame::sizeDown_f

Keybinding command
=================
*/
void ClientGame::sizeDown_f() {
	CvarSystem::set("cg_viewsize", va("%i",(cg_viewsize.integer-10)));
}


/*
=============
ClientGame::viewpos_f

Debugging command to print the current position
=============
*/
void ClientGame::viewpos_f() {
	ClientGame::printf ("(%i %i %i) : %i\n", static_cast<int>(cg.refdef.vieworg[0]),
		static_cast<int>(cg.refdef.vieworg[1]), static_cast<int>(cg.refdef.vieworg[2]), 
		static_cast<int>(cg.refdefViewAngles[YAW]));
}


void ClientGame::scoresDown_f( ) {

	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request New ones
		cg.scoresRequestTime = cg.time;
		CL_AddReliableCommand( "score" );

		// leave the current scores up if they were already
		// displayed, but if this is the first hit, clear them out
		if ( !cg.showScores ) {
			cg.showScores = true;
			cg.numScores = 0;
		}
	} else {
		// show the cached contents even if they just pressed if it
		// is within two seconds
		cg.showScores = true;
	}
}

void ClientGame::scoresUp_f( ) {
	if ( cg.showScores ) {
		cg.showScores = false;
		cg.scoreFadeTime = cg.time;
	}
}

void ClientGame::tellTarget_f( ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = ClientGame::crosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	CommandSystem::argsBuffer( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	CL_AddReliableCommand( command );
}

void ClientGame::tellAttacker_f( ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = ClientGame::lastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	CommandSystem::argsBuffer( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	CL_AddReliableCommand( command );
}

void ClientGame::testPlane_f( ) {
	ClientGame::testVehicle(CAT_PLANE);
}

void ClientGame::testGV_f( ) {
	ClientGame::testVehicle(CAT_GROUND);
}

void ClientGame::spawn_GI_f( ) {
	int idx;

	if( CommandSystem::argc() < 2 ) return;

	idx = atoi( ClientGame::argv(1) );

	if( idx < 0 || idx >= bg_numberOfGroundInstallations ) {
		ClientGame::printf("GroundInstallation number out of range (0-%d)\n", bg_numberOfGroundInstallations-1 );
		return;
	}

	ME_SpawnGroundInstallation(idx);

}

void ClientGame::exportToScript_f( ) {
	if( CommandSystem::argc() < 2 ) 
	{
		ClientGame::printf("Scriptname not specified!");
		return;
	}

	ME_ExportToScript( ClientGame::argv(1) );

}

void ClientGame::setSide_f( ) {
	ME_SetSide( ClientGame::argv(1) );
}

void ClientGame::importScript_f( ) {
	if( CommandSystem::argc() < 2 )
	{
		ClientGame::printf("Scriptname not specified!");
		return;
	}
	ME_ImportScript( ClientGame::argv(1) );
}

/*
=================
CG_ExtractDeveloper

Intercepts 'say <var> <values>' commands for development purposes
=================
*/

bool CG_ExtractDeveloper( )
{
	clientInfo_t * ci = nullptr;

	float value = 0;

	// get client information if we can
	if( cg.snap )
	{
		ci = &cgs.clientinfo[ cg.snap->ps.clientNum ];
	}
	else
	{
		// no information, nothing possible
		return false;
	}

	// convert 2nd param into a value
	value = atof( ClientGame::argv(1) );

	// check 2nd param against known variables

	if( !Q_stricmp( ClientGame::argv(0), "ox" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_XOFFSET ] = value;
		return true;
	}
	else if( !Q_stricmp( ClientGame::argv(0), "oy" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_YOFFSET ] = value;
		return true;
	}
	else if( !Q_stricmp( ClientGame::argv(0), "ax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_XADJUST ] = value;
		return true;
	}
	else if( !Q_stricmp( ClientGame::argv(0), "ay" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_YADJUST ] = value;
		return true;
	}
	else if( !Q_stricmp( ClientGame::argv(0), "pmax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_PITCHMAX ] = value;
		return true;
	}
	else if( !Q_stricmp( ClientGame::argv(0), "rmax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_ROLLMAX ] = value;
		return true;
	}
	else if( !Q_stricmp( ClientGame::argv(0), "pmod" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_PITCHMOD ] = value;
		return true;
	}
	else if( !Q_stricmp( ClientGame::argv(0), "rmod" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_ROLLMOD ] = value;
		return true;
	}

	return false;
}

struct consoleCommand_t {
	char	*cmd;
	void	(*function)();
};

static consoleCommand_t	commands[] = {
	{ "testgun", ClientGame::testGun_f },
	{ "testmodel", ClientGame::testModel_f },
	{ "nextframe", ClientGame::testModelNextFrame_f },
	{ "prevframe", ClientGame::testModelPrevFrame_f },
	{ "nextskin", ClientGame::testModelNextSkin_f },
	{ "prevskin", ClientGame::testModelPrevSkin_f },
	{ "testPlane", ClientGame::testPlane_f },
	{ "testGV", ClientGame::testGV_f },
	{ "testPlaneCmd", ClientGame::testPlaneCmd_f },
	{ "testGVCmd", ClientGame::testGVCmd_f },
	{ "viewpos", ClientGame::viewpos_f },
	{ "+scores", ClientGame::scoresDown_f },
	{ "-scores", ClientGame::scoresUp_f },
	{ "+zoom", ClientGame::zoomDown_f },
	{ "-zoom", ClientGame::zoomUp_f },
	{ "sizeup", ClientGame::sizeUp_f },
	{ "sizedown", ClientGame::sizeDown_f },
	{ "weapnext", ClientGame::nextWeapon_f },
	{ "weapprev", ClientGame::prevWeapon_f },
	{ "weapon", ClientGame::weapon_f },
	{ "tell_target", ClientGame::tellTarget_f },
	{ "tell_attacker", ClientGame::tellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "gps", ClientGame::toggleGps_f },
	{ "extinfo", ClientGame::toggleInfo_f },
	{ "toggleview", ClientGame::toggleView_f },
	{ "+zoomin", ClientGame::zoomIn_f },
	{ "-zoomin", ClientGame::adjustReset_f },
	{ "+zoomout", ClientGame::zoomOut_f },
	{ "-zoomout", ClientGame::adjustReset_f },
	{ "+cameraup", ClientGame::cameraUp_f },
	{ "-cameraup", ClientGame::adjustReset_f },
	{ "+cameradown", ClientGame::cameraDown_f },
	{ "-cameradown", ClientGame::adjustReset_f },
	{ "radarrange", ClientGame::toggleRadarRange_f },
	{ "mfd1_mode", ClientGame::set_MFD1_Mode_f },
	{ "mfd2_mode", ClientGame::set_MFD2_Mode_f },
	{ "mfd1_page", ClientGame::set_MFD1_Page_f },
	{ "mfd2_page", ClientGame::set_MFD2_Page_f },
	{ "cycle_hud_color", ClientGame::cycle_HUD_Color_f },
	{ "cycle_mfd_color", ClientGame::cycle_MFD_Color_f },
	{ "cycle_camera_mode", ClientGame::cycle_Camera_Mode_f },
	{ "set_camera_mode", ClientGame::set_Camera_Mode_f },
	{ "+backcam", ClientGame::camBack_f },
	{ "-backcam", ClientGame::camOld_f },
	{ "+downcam", ClientGame::camDown_f },
	{ "-downcam", ClientGame::camOld_f },
	{ "+bombcam", ClientGame::camBomb_f },
	{ "-bombcam", ClientGame::camOld_f },
	{ "toggle_mfd1", ClientGame::toggle_MFD1_f },
	{ "toggle_mfd2", ClientGame::toggle_MFD2_f },
	{ "update_hud_color", ClientGame::update_HUD_Color_f },
	{ "update_mfd_color", ClientGame::update_MFD_Color_f },
	{ "me_spawn_gi", ClientGame::spawn_GI_f },
	{ "me_export_to_script", ClientGame::exportToScript_f },
	{ "me_import_script", ClientGame::importScript_f },
	{ "me_side", ClientGame::setSide_f },
};

/*
=================
ClientGame::consoleCommand

The string has been tokenized and can be retrieved with
CommandSystem::argc() / CommandSystem::argv()
=================
*/
bool ClientGame::consoleCommand( ) {
	const char	*cmd;
	int		i;

	cmd = ClientGame::argv(0);

	// developer variable modification
	if( CG_ExtractDeveloper() )
	{
		return true;
	}

	// scan though all known client commands
	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) 
	{
		// match?
		if ( !Q_stricmp( cmd, commands[i].cmd ) )
		{
			// execute associated fn()
			commands[i].function();

			return true;
		}
	}

	return false;
}


/*
=================
ClientGame::initConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void ClientGame::initConsoleCommands( ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		CL_AddCgameCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	CL_AddCgameCommand ("kill");
	CL_AddCgameCommand ("say");
	CL_AddCgameCommand ("say_team");
	CL_AddCgameCommand ("tell");
	CL_AddCgameCommand ("vsay");
	CL_AddCgameCommand ("vsay_team");
	CL_AddCgameCommand ("vtell");
	CL_AddCgameCommand ("vtaunt");
	CL_AddCgameCommand ("vosay");
	CL_AddCgameCommand ("vosay_team");
	CL_AddCgameCommand ("votell");
	CL_AddCgameCommand ("noclip");
	CL_AddCgameCommand ("team");
	CL_AddCgameCommand ("follow");
	CL_AddCgameCommand ("levelshot");
	CL_AddCgameCommand ("addbot");
	CL_AddCgameCommand ("setviewpos");
	CL_AddCgameCommand ("callvote");
	CL_AddCgameCommand ("vote");
	CL_AddCgameCommand ("callteamvote");
	CL_AddCgameCommand ("teamvote");
	CL_AddCgameCommand ("stats");
	CL_AddCgameCommand ("teamtask");
	CL_AddCgameCommand ("loaddefered");	// spelled wrong, but not changing for demo
	CL_AddCgameCommand ("contact_tower");
	CL_AddCgameCommand ("radar");
	CL_AddCgameCommand ("unlock");

}
