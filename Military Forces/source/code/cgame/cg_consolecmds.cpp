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

	targetNum = CGame::crosshairPlayer();
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
void CGame::toggleGps_f( ) {

	if( cg.time < cg.GPSTime ) {
		return;
	}
	cg.GPS = cg.GPS ? 0 : 1;
	cg.GPSTime = cg.time + 500;
}

void CGame::toggleRadarRange_f( ) {

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
void CGame::set_MFD_Mode( int num ) {
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

void CGame::set_MFD1_Mode_f( ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	CGame::set_MFD_Mode(MFD_1);

	cg.MFDTime = cg.time + 100;
}

void CGame::set_MFD2_Mode_f( ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	CGame::set_MFD_Mode(MFD_2);

	cg.MFDTime = cg.time + 100;
}


/*
==================
MFQ3 MFD Mode setting
==================
*/

void CGame::set_MFD_Page( int num ) {
	int page;

	if( CommandSystem::argc() < 2 ) {
		return;
	}

	page = atoi( CGame::argv(1) );

	if( page < 0 ) page = 0;
	else if( page >= MFD_MAX ) page = MFD_MAX - 1;

	cg.Mode_MFD[num] = page;

	if( num == MFD_1 )
		CvarSystem::set( "mfd1_defaultpage", va("%d", page) );
	else if( num == MFD_2 )
		CvarSystem::set( "mfd2_defaultpage", va("%d", page) );

}

void CGame::set_MFD1_Page_f( ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	CGame::set_MFD_Page(MFD_1);

	cg.MFDTime = cg.time + 100;
}

void CGame::set_MFD2_Page_f( ) {
	if( cg.time < cg.MFDTime ) {
		return;
	}
	CGame::set_MFD_Page(MFD_2);

	cg.MFDTime = cg.time + 100;
}

/*
==================
MFQ3 HUD Color setting
==================
*/

void CGame::update_HUD_Color_f( )
{
	// update 
	cg.HUDColor = CGame::cvar_Get( "hud_color" );
}

/*
==================
MFQ3 HUD Color cycling
==================
*/

void CGame::cycle_HUD_Color_f( ) {
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

void CGame::update_MFD_Color_f( )
{
	// update 
	cg.MFDColor = CGame::cvar_Get( "mfd_color" );
}

/*
==================
MFQ3 MFD toggling
==================
*/

void CGame::toggle_MFD1_f( ) {
	int mfd1 = hud_mfd.integer;

	mfd1 = mfd1 ? 0 : 1;
	CvarSystem::set( "hud_mfd", va("%d", mfd1) );
	
}

void CGame::toggle_MFD2_f( ) {
	int mfd2 = hud_mfd2.integer;

	mfd2 = mfd2 ? 0 : 1;
	CvarSystem::set( "hud_mfd2", va("%d", mfd2) );

}


/*
==================
MFQ3 MFD color cycling
==================
*/

void CGame::cycle_MFD_Color_f( ) {
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

void CGame::cycle_Camera_Mode_f( ) {
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

void CGame::set_Camera_Mode_f( ) {

	if( cg.time < cg.CameraModeTime ) {
		return;
	}

	if( CommandSystem::argc() < 2 ) {
		return;
	}

	cg.CameraMode = atoi( CGame::argv(1) );
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

void CGame::camBack_f( )
{
	cg.CameraMode = CAMERA_BACK;
}

void CGame::camDown_f( )
{
	cg.CameraMode = CAMERA_DOWN;
}

void CGame::camBomb_f( )
{
	cg.CameraMode = CAMERA_BOMB;
}

void CGame::camOld_f( )
{
	cg.CameraMode = CAMERA_TARGET;
}

/*
==================
MFQ3 INFO
==================
*/
void CGame::toggleInfo_f( ) {

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
void CGame::toggleView_f( ) {

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

void CGame::adjustReset_f( ) {
	cg.cameraAdjustEnum = CAMADJ_NONE;
	cg.cameraAdjustAmount = 0;
}

constexpr float MOVE_BASIC = 0.4f;

/*
==================
MFQ3 camera zoom in/out
==================
*/
void CGame::zoomIn_f( ) {
	cg.cameraAdjustEnum = CAMADJ_INOUT;
	cg.cameraAdjustAmount = +MOVE_BASIC;
}

void CGame::zoomOut_f( ) {
	cg.cameraAdjustEnum = CAMADJ_INOUT;
	cg.cameraAdjustAmount = -MOVE_BASIC;
}

/*
==================
MFQ3 camera up/down
==================
*/
void CGame::cameraUp_f( ) {
	cg.cameraAdjustEnum = CAMADJ_UPDOWN;
	cg.cameraAdjustAmount = +MOVE_BASIC;
}

void CGame::cameraDown_f( ) {
	cg.cameraAdjustEnum = CAMADJ_UPDOWN;
	cg.cameraAdjustAmount = -MOVE_BASIC;
}


/*
=================
CGame::sizeUp_f

Keybinding command
=================
*/
void CGame::sizeUp_f() {
	CvarSystem::set("cg_viewsize", va("%i",(cg_viewsize.integer+10)));
}


/*
=================
CGame::sizeDown_f

Keybinding command
=================
*/
void CGame::sizeDown_f() {
	CvarSystem::set("cg_viewsize", va("%i",(cg_viewsize.integer-10)));
}


/*
=============
CGame::viewpos_f

Debugging command to print the current position
=============
*/
void CGame::viewpos_f() {
	CGame::printf ("(%i %i %i) : %i\n", static_cast<int>(cg.refdef.vieworg[0]),
		static_cast<int>(cg.refdef.vieworg[1]), static_cast<int>(cg.refdef.vieworg[2]), 
		static_cast<int>(cg.refdefViewAngles[YAW]));
}


void CGame::scoresDown_f( ) {

	if ( cg.scoresRequestTime + 2000 < cg.time ) {
		// the scores are more than two seconds out of data,
		// so request New ones
		cg.scoresRequestTime = cg.time;
		ClientHost::addReliableCommand( "score" );

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

void CGame::scoresUp_f( ) {
	if ( cg.showScores ) {
		cg.showScores = false;
		cg.scoreFadeTime = cg.time;
	}
}

void CGame::tellTarget_f( ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CGame::crosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	CommandSystem::argsBuffer( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	ClientHost::addReliableCommand( command );
}

void CGame::tellAttacker_f( ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CGame::lastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	CommandSystem::argsBuffer( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	ClientHost::addReliableCommand( command );
}

void CGame::testPlane_f( ) {
	CGame::testVehicle(CAT_PLANE);
}

void CGame::testGV_f( ) {
	CGame::testVehicle(CAT_GROUND);
}

void CGame::spawn_GI_f( ) {
	int idx;

	if( CommandSystem::argc() < 2 ) return;

	idx = atoi( CGame::argv(1) );

	if( idx < 0 || idx >= bg_numberOfGroundInstallations ) {
		CGame::printf("GroundInstallation number out of range (0-%d)\n", bg_numberOfGroundInstallations-1 );
		return;
	}

	ME_SpawnGroundInstallation(idx);

}

void CGame::exportToScript_f( ) {
	if( CommandSystem::argc() < 2 ) 
	{
		CGame::printf("Scriptname not specified!");
		return;
	}

	ME_ExportToScript( CGame::argv(1) );

}

void CGame::setSide_f( ) {
	ME_SetSide( CGame::argv(1) );
}

void CGame::importScript_f( ) {
	if( CommandSystem::argc() < 2 )
	{
		CGame::printf("Scriptname not specified!");
		return;
	}
	ME_ImportScript( CGame::argv(1) );
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
	value = atof( CGame::argv(1) );

	// check 2nd param against known variables

	if( !Q_stricmp( CGame::argv(0), "ox" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_XOFFSET ] = value;
		return true;
	}
	else if( !Q_stricmp( CGame::argv(0), "oy" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_YOFFSET ] = value;
		return true;
	}
	else if( !Q_stricmp( CGame::argv(0), "ax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_XADJUST ] = value;
		return true;
	}
	else if( !Q_stricmp( CGame::argv(0), "ay" ) )
	{
		availableVehicles[ ci->vehicle ].shadowCoords[ SHC_YADJUST ] = value;
		return true;
	}
	else if( !Q_stricmp( CGame::argv(0), "pmax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_PITCHMAX ] = value;
		return true;
	}
	else if( !Q_stricmp( CGame::argv(0), "rmax" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_ROLLMAX ] = value;
		return true;
	}
	else if( !Q_stricmp( CGame::argv(0), "pmod" ) )
	{
		availableVehicles[ ci->vehicle ].shadowAdjusts[ SHO_PITCHMOD ] = value;
		return true;
	}
	else if( !Q_stricmp( CGame::argv(0), "rmod" ) )
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

static void CG_DevVehicle_f( ) {
	if( !CvarSystem::variableIntegerValue( "sv_cheats" ) ) {
		Common::printf( "dev_vehicle needs cheats (devmap)\n" );
		return;
	}
	CvarSystem::set( "cg_nextVehicle", CGame::argv( 1 ) );
}

static void CG_DevStatus_f( ) {
	if( !cg.snap || !CvarSystem::variableIntegerValue( "sv_cheats" ) ) 
		return;
	const playerState_t *ps = &cg.snap->ps;
	Common::printf( "DEVSTATUS veh %d health %d fuel %d ammo %d %d %d %d %d %d %d %d onoff %x reveal %d time %d org %.1f %.1f %.1f ground %d weapon %d %d lock %d target %d anim %d sink %.0f speed %d throttle %d geartimers %d %d\n",
		cgs.clientinfo[ps->clientNum].vehicle, ps->stats[STAT_HEALTH], ps->stats[STAT_FUEL],
		ps->ammo[0], ps->ammo[1], ps->ammo[2], ps->ammo[3], ps->ammo[4], ps->ammo[5], ps->ammo[6], ps->ammo[7],
		ps->ONOFF, ps->timers[TIMER_REVEAL], cg.snap->serverTime, ps->origin[0], ps->origin[1], ps->origin[2], ps->groundEntityNum, ps->weaponNum, ps->weaponIndex,
		ps->stats[STAT_LOCKINFO], ps->tracktarget, ps->vehicleAnim, -ps->velocity[2], ps->speed, ps->throttle, ps->timers[TIMER_GEAR], ps->timers[TIMER_GEARANIM] );
}

static consoleCommand_t	commands[] = {
	{ "dev_status", CG_DevStatus_f },
	{ "dev_vehicle", CG_DevVehicle_f },
	{ "testgun", CGame::testGun_f },
	{ "testmodel", CGame::testModel_f },
	{ "nextframe", CGame::testModelNextFrame_f },
	{ "prevframe", CGame::testModelPrevFrame_f },
	{ "nextskin", CGame::testModelNextSkin_f },
	{ "prevskin", CGame::testModelPrevSkin_f },
	{ "testPlane", CGame::testPlane_f },
	{ "testGV", CGame::testGV_f },
	{ "testPlaneCmd", CGame::testPlaneCmd_f },
	{ "testGVCmd", CGame::testGVCmd_f },
	{ "viewpos", CGame::viewpos_f },
	{ "+scores", CGame::scoresDown_f },
	{ "-scores", CGame::scoresUp_f },
	{ "+zoom", CGame::zoomDown_f },
	{ "-zoom", CGame::zoomUp_f },
	{ "sizeup", CGame::sizeUp_f },
	{ "sizedown", CGame::sizeDown_f },
	{ "weapnext", CGame::nextWeapon_f },
	{ "weapprev", CGame::prevWeapon_f },
	{ "weapon", CGame::weapon_f },
	{ "tell_target", CGame::tellTarget_f },
	{ "tell_attacker", CGame::tellAttacker_f },
	{ "tcmd", CG_TargetCommand_f },
	{ "gps", CGame::toggleGps_f },
	{ "extinfo", CGame::toggleInfo_f },
	{ "toggleview", CGame::toggleView_f },
	{ "+zoomin", CGame::zoomIn_f },
	{ "-zoomin", CGame::adjustReset_f },
	{ "+zoomout", CGame::zoomOut_f },
	{ "-zoomout", CGame::adjustReset_f },
	{ "+cameraup", CGame::cameraUp_f },
	{ "-cameraup", CGame::adjustReset_f },
	{ "+cameradown", CGame::cameraDown_f },
	{ "-cameradown", CGame::adjustReset_f },
	{ "radarrange", CGame::toggleRadarRange_f },
	{ "mfd1_mode", CGame::set_MFD1_Mode_f },
	{ "mfd2_mode", CGame::set_MFD2_Mode_f },
	{ "mfd1_page", CGame::set_MFD1_Page_f },
	{ "mfd2_page", CGame::set_MFD2_Page_f },
	{ "cycle_hud_color", CGame::cycle_HUD_Color_f },
	{ "cycle_mfd_color", CGame::cycle_MFD_Color_f },
	{ "cycle_camera_mode", CGame::cycle_Camera_Mode_f },
	{ "set_camera_mode", CGame::set_Camera_Mode_f },
	{ "+backcam", CGame::camBack_f },
	{ "-backcam", CGame::camOld_f },
	{ "+downcam", CGame::camDown_f },
	{ "-downcam", CGame::camOld_f },
	{ "+bombcam", CGame::camBomb_f },
	{ "-bombcam", CGame::camOld_f },
	{ "toggle_mfd1", CGame::toggle_MFD1_f },
	{ "toggle_mfd2", CGame::toggle_MFD2_f },
	{ "update_hud_color", CGame::update_HUD_Color_f },
	{ "update_mfd_color", CGame::update_MFD_Color_f },
	{ "me_spawn_gi", CGame::spawn_GI_f },
	{ "me_export_to_script", CGame::exportToScript_f },
	{ "me_import_script", CGame::importScript_f },
	{ "me_side", CGame::setSide_f },
};

/*
=================
CGame::consoleCommand

The string has been tokenized and can be retrieved with
CommandSystem::argc() / CommandSystem::argv()
=================
*/
bool CGame::consoleCommand( ) {
	const char	*cmd;
	int		i;

	cmd = CGame::argv(0);

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
CGame::initConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CGame::initConsoleCommands( ) {
	int		i;

	for ( i = 0 ; i < sizeof( commands ) / sizeof( commands[0] ) ; i++ ) {
		ClientHost::addCgameCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	ClientHost::addCgameCommand ("kill");
	ClientHost::addCgameCommand ("say");
	ClientHost::addCgameCommand ("say_team");
	ClientHost::addCgameCommand ("tell");
	ClientHost::addCgameCommand ("vsay");
	ClientHost::addCgameCommand ("vsay_team");
	ClientHost::addCgameCommand ("vtell");
	ClientHost::addCgameCommand ("vtaunt");
	ClientHost::addCgameCommand ("vosay");
	ClientHost::addCgameCommand ("vosay_team");
	ClientHost::addCgameCommand ("votell");
	ClientHost::addCgameCommand ("noclip");
	ClientHost::addCgameCommand ("team");
	ClientHost::addCgameCommand ("follow");
	ClientHost::addCgameCommand ("levelshot");
	ClientHost::addCgameCommand ("addbot");
	ClientHost::addCgameCommand ("setviewpos");
	ClientHost::addCgameCommand ("callvote");
	ClientHost::addCgameCommand ("vote");
	ClientHost::addCgameCommand ("callteamvote");
	ClientHost::addCgameCommand ("teamvote");
	ClientHost::addCgameCommand ("stats");
	ClientHost::addCgameCommand ("teamtask");
	ClientHost::addCgameCommand ("loaddefered");	// spelled wrong, but not changing for demo
	ClientHost::addCgameCommand ("contact_tower");
	ClientHost::addCgameCommand ("radar");
	ClientHost::addCgameCommand ("dev_place");
	ClientHost::addCgameCommand ("unlock");

}
