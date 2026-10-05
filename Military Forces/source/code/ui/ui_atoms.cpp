/*
 * $Id: ui_atoms.c,v 1.7 2005-11-21 17:28:32 thebjoern Exp $
*/

/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"




/*
=================
Menus::clampCvar
=================
*/
float Menus::clampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*
=================
Menus::startDemoLoop
=================
*/
void Menus::startDemoLoop( ) {
	CommandBuffer::executeText( EXEC_APPEND, "d1\n" );
}

char *Menus::argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	CommandSystem::argvBuffer( arg, buffer, sizeof( buffer ) );

	return buffer;
}


char *Menus::cvar_VariableString( const char *var_name ) {
	static char	buffer[MAX_STRING_CHARS];

	CvarSystem::variableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}



void Menus::setBestScores(postGameInfo_t *newInfo, bool postGame) {
	CvarSystem::set("ui_scoreAccuracy",     va("%i%%", newInfo->accuracy));
	CvarSystem::set("ui_scoreImpressives",	va("%i", newInfo->impressives));
	CvarSystem::set("ui_scoreExcellents", 	va("%i", newInfo->excellents));
	CvarSystem::set("ui_scoreDefends", 			va("%i", newInfo->defends));
	CvarSystem::set("ui_scoreAssists", 			va("%i", newInfo->assists));
	CvarSystem::set("ui_scoreGauntlets", 		va("%i", newInfo->gauntlets));
	CvarSystem::set("ui_scoreScore", 				va("%i", newInfo->score));
	CvarSystem::set("ui_scorePerfect",	 		va("%i", newInfo->perfects));
	CvarSystem::set("ui_scoreTeam",					va("%i to %i", newInfo->redScore, newInfo->blueScore));
	CvarSystem::set("ui_scoreBase",					va("%i", newInfo->baseScore));
	CvarSystem::set("ui_scoreTimeBonus",		va("%i", newInfo->timeBonus));
	CvarSystem::set("ui_scoreSkillBonus",		va("%i", newInfo->skillBonus));
	CvarSystem::set("ui_scoreShutoutBonus",	va("%i", newInfo->shutoutBonus));
	CvarSystem::set("ui_scoreTime",					va("%02i:%02i", newInfo->time / 60, newInfo->time % 60));
	CvarSystem::set("ui_scoreCaptures",		va("%i", newInfo->captures));
  if (postGame) {
		CvarSystem::set("ui_scoreAccuracy2",     va("%i%%", newInfo->accuracy));
		CvarSystem::set("ui_scoreImpressives2",	va("%i", newInfo->impressives));
		CvarSystem::set("ui_scoreExcellents2", 	va("%i", newInfo->excellents));
		CvarSystem::set("ui_scoreDefends2", 			va("%i", newInfo->defends));
		CvarSystem::set("ui_scoreAssists2", 			va("%i", newInfo->assists));
		CvarSystem::set("ui_scoreGauntlets2", 		va("%i", newInfo->gauntlets));
		CvarSystem::set("ui_scoreScore2", 				va("%i", newInfo->score));
		CvarSystem::set("ui_scorePerfect2",	 		va("%i", newInfo->perfects));
		CvarSystem::set("ui_scoreTeam2",					va("%i to %i", newInfo->redScore, newInfo->blueScore));
		CvarSystem::set("ui_scoreBase2",					va("%i", newInfo->baseScore));
		CvarSystem::set("ui_scoreTimeBonus2",		va("%i", newInfo->timeBonus));
		CvarSystem::set("ui_scoreSkillBonus2",		va("%i", newInfo->skillBonus));
		CvarSystem::set("ui_scoreShutoutBonus2",	va("%i", newInfo->shutoutBonus));
		CvarSystem::set("ui_scoreTime2",					va("%02i:%02i", newInfo->time / 60, newInfo->time % 60));
		CvarSystem::set("ui_scoreCaptures2",		va("%i", newInfo->captures));
	}
}

void Menus::loadBestScores(const char *map, int game) {
	char		fileName[MAX_QPATH];
	fileHandle_t f;
	postGameInfo_t newInfo;
	memset(&newInfo, 0, sizeof(postGameInfo_t));
	Com_sprintf(fileName, MAX_QPATH, "games/%s_%i.game", map, game);
	if (FileSystem::fOpenFileByMode(fileName, &f, FS_READ) >= 0) {
		int size = 0;
		FileSystem::read(&size, sizeof(int), f);
		if (size == sizeof(postGameInfo_t)) {
			FileSystem::read(&newInfo, sizeof(postGameInfo_t), f);
		}
		FileSystem::fCloseFile(f);
	}
	Menus::setBestScores(&newInfo, false);

	Com_sprintf(fileName, MAX_QPATH, "demos/%s_%d.dm_%d", map, game, static_cast<int>(CvarSystem::variableValue("protocol")));
	uiInfo.demoAvailable = false;
	if (FileSystem::fOpenFileByMode(fileName, &f, FS_READ) >= 0) {
		uiInfo.demoAvailable = true;
		FileSystem::fCloseFile(f);
	} 
}

/*
===============
Menus::clearScores
===============
*/
void Menus::clearScores() {
	char	gameList[4096];
	char *gameFile;
	int		i, len, count, size;
	fileHandle_t f;
	postGameInfo_t newInfo;

	count = FileSystem::getFileList( "games", "game", gameList, sizeof(gameList) );

	size = sizeof(postGameInfo_t);
	memset(&newInfo, 0, size);

	if (count > 0) {
		gameFile = gameList;
		for ( i = 0; i < count; i++ ) {
			len = strlen(gameFile);
			if (FileSystem::fOpenFileByMode(va("games/%s",gameFile), &f, FS_WRITE) >= 0) {
				FileSystem::write(&size, sizeof(int), f);
				FileSystem::write(&newInfo, size, f);
				FileSystem::fCloseFile(f);
			}
			gameFile += len + 1;
		}
	}
	
	Menus::setBestScores(&newInfo, false);

}



static void	UI_Cache_f() 
{
	uiInfo.uiUtils.display_CacheAll();
}

/*
=======================
UI_CalcPostGameStats
=======================
*/
static void UI_CalcPostGameStats() {
	char		map[MAX_QPATH];
	char		fileName[MAX_QPATH];
	char		info[MAX_INFO_STRING];
	fileHandle_t f;
	int size, game, time, adjustedTime;
	postGameInfo_t oldInfo;
	postGameInfo_t newInfo;
	bool newHigh = false;

	GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	Q_strncpyz( map, Info::valueForKey( info, "mapname" ), sizeof(map) );
	game = atoi(Info::valueForKey(info, "g_gametype"));

	// compose file name
	Com_sprintf(fileName, MAX_QPATH, "games/%s_%i.game", map, game);
	// see if we have one already
	memset(&oldInfo, 0, sizeof(postGameInfo_t));
	if (FileSystem::fOpenFileByMode(fileName, &f, FS_READ) >= 0) {
	// if so load it
		size = 0;
		FileSystem::read(&size, sizeof(int), f);
		if (size == sizeof(postGameInfo_t)) {
			FileSystem::read(&oldInfo, sizeof(postGameInfo_t), f);
		}
		FileSystem::fCloseFile(f);
	}					 

	newInfo.accuracy = atoi(Menus::argv(3));
	newInfo.impressives = atoi(Menus::argv(4));
	newInfo.excellents = atoi(Menus::argv(5));
	newInfo.defends = atoi(Menus::argv(6));
	newInfo.assists = atoi(Menus::argv(7));
	newInfo.gauntlets = atoi(Menus::argv(8));
	newInfo.baseScore = atoi(Menus::argv(9));
	newInfo.perfects = atoi(Menus::argv(10));
	newInfo.redScore = atoi(Menus::argv(11));
	newInfo.blueScore = atoi(Menus::argv(12));
	time = atoi(Menus::argv(13));
	newInfo.captures = atoi(Menus::argv(14));

	newInfo.time = (time - CvarSystem::variableValue("ui_matchStartTime")) / 1000;
	adjustedTime = uiInfo.mapList[ui_currentMap.integer].timeToBeat[game];
	if (newInfo.time < adjustedTime) { 
		newInfo.timeBonus = (adjustedTime - newInfo.time) * 10;
	} else {
		newInfo.timeBonus = 0;
	}

	if (newInfo.redScore > newInfo.blueScore && newInfo.blueScore <= 0) {
		newInfo.shutoutBonus = 100;
	} else {
		newInfo.shutoutBonus = 0;
	}

	newInfo.skillBonus = CvarSystem::variableValue("g_spSkill");
	if (newInfo.skillBonus <= 0) {
		newInfo.skillBonus = 1;
	}
	newInfo.score = newInfo.baseScore + newInfo.shutoutBonus + newInfo.timeBonus;
	newInfo.score *= newInfo.skillBonus;

	// see if the score is higher for this one
	newHigh = (newInfo.redScore > newInfo.blueScore && newInfo.score > oldInfo.score);

	if  (newHigh) {
		// if so write out the New one
		uiInfo.newHighScoreTime = uiInfo.uiUtils.dc_->realTime_ + 20000;
		if (FileSystem::fOpenFileByMode(fileName, &f, FS_WRITE) >= 0) {
			size = sizeof(postGameInfo_t);
			FileSystem::write(&size, sizeof(int), f);
			FileSystem::write(&newInfo, sizeof(postGameInfo_t), f);
			FileSystem::fCloseFile(f);
		}
	}

	if (newInfo.time < oldInfo.time) {
		uiInfo.newBestTime = uiInfo.uiUtils.dc_->realTime_ + 20000;
	}
 
	// put back all the ui overrides
	CvarSystem::set("capturelimit", Menus::cvar_VariableString("ui_saveCaptureLimit"));
	CvarSystem::set("fraglimit", Menus::cvar_VariableString("ui_saveFragLimit"));
	CvarSystem::set("cg_drawTimer", Menus::cvar_VariableString("ui_drawTimer"));
	CvarSystem::set("g_doWarmup", Menus::cvar_VariableString("ui_doWarmup"));
	CvarSystem::set("g_Warmup", Menus::cvar_VariableString("ui_Warmup"));
	CvarSystem::set("sv_pure", Menus::cvar_VariableString("ui_pure"));
	CvarSystem::set("g_friendlyFire", Menus::cvar_VariableString("ui_friendlyFire"));

	Menus::setBestScores(&newInfo, true);
	Menus::showPostGame(newHigh);


}

/*
==================
MFQ3 Custom Chat
==================
*/

static void UI_Chat( int mode )
{
	// reset & begin
	Key_SetCatcher( ( KEYCATCH_UI ) | ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) );
	uiInfo.customChat.active = true;
	uiInfo.customChat.text[0] = 0;
	uiInfo.customChat.cindex = 0;
	uiInfo.customChat.mode = static_cast<CustomChatMode>(mode);
}
/*
=================
Menus::consoleCommand
=================
*/
bool Menus::consoleCommand( int realTime ) {
	char	*cmd;

	uiInfo.uiUtils.dc_->frameTime_ = realTime - uiInfo.uiUtils.dc_->realTime_;
	uiInfo.uiUtils.dc_->realTime_ = realTime;

	cmd = Menus::argv( 0 );

	// ensure minimum menu data is available
	//Menu_Cache();

	if ( Q_stricmp (cmd, "ui_test") == 0 ) {
		Menus::showPostGame(true);
	}

	if ( Q_stricmp (cmd, "ui_report") == 0 ) {
		Menus::report();
		return true;
	}
	
	if ( Q_stricmp (cmd, "ui_load") == 0 ) {
		Menus::load();
		return true;
	}

	if ( Q_stricmp (cmd, "remapShader") == 0 ) {
		if (CommandSystem::argc() == 4) {
			char shader1[MAX_QPATH];
			char shader2[MAX_QPATH];
			Q_strncpyz(shader1, Menus::argv(1), sizeof(shader1));
			Q_strncpyz(shader2, Menus::argv(2), sizeof(shader2));
			refExport.RemapShader(shader1, shader2, Menus::argv(3));
			return true;
		}
	}

	if ( Q_stricmp (cmd, "postgame") == 0 ) {
		UI_CalcPostGameStats();
		return true;
	}

	if ( Q_stricmp (cmd, "ui_cache") == 0 ) {
		UI_Cache_f();
		return true;
	}

	if ( Q_stricmp (cmd, "ui_teamOrders") == 0 ) {
		//UI_TeamOrdersMenu_f();
		return true;
	}


	if ( Q_stricmp (cmd, "ui_cdkey") == 0 ) {
		//UI_CDKeyMenu_f();
		return true;
	}

	// MFQ3: bring up the 'team selection' dialog (i.e. bind command to key)
	if( Q_stricmp (cmd, "teamselect") == 0 ) 
	{
		CvarSystem::set( "cl_paused", "1" );
		Key_SetCatcher( ( KEYCATCH_UI ) | ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) );
		uiInfo.uiUtils.menu_CloseAll();

		uiInfo.uiUtils.menu_ActivateByName( "ingame_select_team" );

		return true;
	}

	// MFQ3: bring up the 'vehicle selection' dialog (i.e. bind command to key)
	if( Q_stricmp (cmd, "vehicleselect") == 0 ) 
	{
		CvarSystem::set( "cl_paused", "1" );
		Key_SetCatcher( ( KEYCATCH_UI ) | ( Key_GetCatcher( ) & KEYCATCH_CONSOLE ) );
		uiInfo.uiUtils.menu_CloseAll();

		uiInfo.uiUtils.menu_ActivateByName( "ingame_select_vehicle" );

		return true;
	}

/* MFQ3: TODO - MM

	// MFQ3: bring up the 'vehicle encyclopedia' dialog (i.e. bind command to key)
	if( Q_stricmp (cmd, "encyclopedia") == 0 )
	{
		UI_EncyclopediaMenu();
		return true;
	}
*/
	// MFQ3: bring up the custom chat console
	if( Q_stricmp( cmd, "cmessagemode") == 0 )
	{
		UI_Chat( CCHAT_ALL );
		return true;
	}
	else if( Q_stricmp( cmd, "cmessagemode2") == 0 )
	{
		UI_Chat( CCHAT_TEAM );
		return true;
	}
	else if( Q_stricmp( cmd, "cmessagemode3") == 0 )
	{
		UI_Chat( CCHAT_TARGET );
		return true;
	}
	else if( Q_stricmp( cmd, "cmessagemode4") == 0 )
	{
		UI_Chat( CCHAT_ATTACK );
		return true;
	}

	return false;
}

/*
=================
Menus::shutdown
=================
*/
void Menus::shutdown( ) {
}

/*
================
Menus::adjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void Menus::adjustFrom640( float *x, float *y, float *w, float *h ) {
	uiInfo.uiUtils.adjustFrom640( x, y, w, h );
}

void Menus::drawNamedPic( float x, float y, float width, float height, const char *picname ) {
	qhandle_t	hShader;

	hShader = refExport.RegisterShaderNoMip( picname );
	Menus::adjustFrom640( &x, &y, &width, &height );
	refExport.DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

void Menus::drawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	if( x <= 0 && y <= 0 && x + w >= 640 && y + h >= 480 )
	{
		float vidW = uiInfo.uiUtils.dc_->glConfig_.vidWidth;
		float vidH = uiInfo.uiUtils.dc_->glConfig_.vidHeight;
		float cover = uiInfo.uiUtils.dc_->xScale_ > uiInfo.uiUtils.dc_->yScale_ ? uiInfo.uiUtils.dc_->xScale_ : uiInfo.uiUtils.dc_->yScale_;
		float visibleS = vidW / ( w * cover );
		float visibleT = vidH / ( h * cover );
		float ds = s1 - s0;
		float dt = t1 - t0;
		float cropS = 0.5f * ( 1.0f - visibleS );

		refExport.DrawStretchPic( 0, 0, vidW, vidH, s0 + ds * cropS, t0, s1 - ds * cropS, t0 + dt * visibleT, hShader );
		return;
	}

	Menus::adjustFrom640( &x, &y, &w, &h );
	refExport.DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
Menus::fillRect

Coordinates are 640*480 virtual values
=================
*/
void Menus::fillRect( float x, float y, float width, float height, const float *color ) 
{
	refExport.SetColor( color );

	Menus::adjustFrom640( &x, &y, &width, &height );
	refExport.DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );

	refExport.SetColor( nullptr );
}

void Menus::drawSides(float x, float y, float w, float h) 
{
	Menus::adjustFrom640( &x, &y, &w, &h );
	refExport.DrawStretchPic( x, y, 1, h, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
	refExport.DrawStretchPic( x + w - 1, y, 1, h, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
}

void Menus::drawTopBottom(float x, float y, float w, float h) 
{
	Menus::adjustFrom640( &x, &y, &w, &h );
	refExport.DrawStretchPic( x, y, w, 1, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
	refExport.DrawStretchPic( x, y + h - 1, w, 1, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
}
/*
================
Menus::drawRect

Coordinates are 640*480 virtual values
=================
*/
void Menus::drawRect( float x, float y, float width, float height, const float *color ) {
	refExport.SetColor( color );

  Menus::drawTopBottom(x, y, width, height);
  Menus::drawSides(x, y, width, height);

	refExport.SetColor( nullptr );
}

void Menus::setColor( const float *rgba ) 
{
	refExport.SetColor( rgba );
}

void Menus::updateScreen( ) 
{
	Screen::updateScreen();
}


void Menus::drawTextBox (int x, int y, int width, int lines)
{
	Menus::fillRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorBlack );
	Menus::drawRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorWhite );
}

bool Menus::cursorInRect (int x, int y, int width, int height)
{
	if (uiInfo.uiUtils.dc_->cursorX_ < x ||
		uiInfo.uiUtils.dc_->cursorY_ < y ||
		uiInfo.uiUtils.dc_->cursorX_ > x+width ||
		uiInfo.uiUtils.dc_->cursorY_ > y+height)
		return false;

	return true;
}
