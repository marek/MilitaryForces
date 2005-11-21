/*
 * $Id: ui_atoms.c,v 1.7 2005-11-21 17:28:32 thebjoern Exp $
*/

/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

bool		m_entersound;		// after a frame, so caching won't disrupt the sound

bool newUI = false;


/*
=================
UI_ClampCvar
=================
*/
float UI_ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

/*
=================
UI_StartDemoLoop
=================
*/
void UI_StartDemoLoop( void ) {
	Cbuf_ExecuteText( EXEC_APPEND, "d1\n" );
}

char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	Cmd_ArgvBuffer( arg, buffer, sizeof( buffer ) );

	return buffer;
}


char *UI_Cvar_VariableString( const char *var_name ) {
	static char	buffer[MAX_STRING_CHARS];

	Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}



void UI_SetBestScores(postGameInfo_t *newInfo, bool postGame) {
	Cvar_Set("ui_scoreAccuracy",     va("%i%%", newInfo->accuracy));
	Cvar_Set("ui_scoreImpressives",	va("%i", newInfo->impressives));
	Cvar_Set("ui_scoreExcellents", 	va("%i", newInfo->excellents));
	Cvar_Set("ui_scoreDefends", 			va("%i", newInfo->defends));
	Cvar_Set("ui_scoreAssists", 			va("%i", newInfo->assists));
	Cvar_Set("ui_scoreGauntlets", 		va("%i", newInfo->gauntlets));
	Cvar_Set("ui_scoreScore", 				va("%i", newInfo->score));
	Cvar_Set("ui_scorePerfect",	 		va("%i", newInfo->perfects));
	Cvar_Set("ui_scoreTeam",					va("%i to %i", newInfo->redScore, newInfo->blueScore));
	Cvar_Set("ui_scoreBase",					va("%i", newInfo->baseScore));
	Cvar_Set("ui_scoreTimeBonus",		va("%i", newInfo->timeBonus));
	Cvar_Set("ui_scoreSkillBonus",		va("%i", newInfo->skillBonus));
	Cvar_Set("ui_scoreShutoutBonus",	va("%i", newInfo->shutoutBonus));
	Cvar_Set("ui_scoreTime",					va("%02i:%02i", newInfo->time / 60, newInfo->time % 60));
	Cvar_Set("ui_scoreCaptures",		va("%i", newInfo->captures));
  if (postGame) {
		Cvar_Set("ui_scoreAccuracy2",     va("%i%%", newInfo->accuracy));
		Cvar_Set("ui_scoreImpressives2",	va("%i", newInfo->impressives));
		Cvar_Set("ui_scoreExcellents2", 	va("%i", newInfo->excellents));
		Cvar_Set("ui_scoreDefends2", 			va("%i", newInfo->defends));
		Cvar_Set("ui_scoreAssists2", 			va("%i", newInfo->assists));
		Cvar_Set("ui_scoreGauntlets2", 		va("%i", newInfo->gauntlets));
		Cvar_Set("ui_scoreScore2", 				va("%i", newInfo->score));
		Cvar_Set("ui_scorePerfect2",	 		va("%i", newInfo->perfects));
		Cvar_Set("ui_scoreTeam2",					va("%i to %i", newInfo->redScore, newInfo->blueScore));
		Cvar_Set("ui_scoreBase2",					va("%i", newInfo->baseScore));
		Cvar_Set("ui_scoreTimeBonus2",		va("%i", newInfo->timeBonus));
		Cvar_Set("ui_scoreSkillBonus2",		va("%i", newInfo->skillBonus));
		Cvar_Set("ui_scoreShutoutBonus2",	va("%i", newInfo->shutoutBonus));
		Cvar_Set("ui_scoreTime2",					va("%02i:%02i", newInfo->time / 60, newInfo->time % 60));
		Cvar_Set("ui_scoreCaptures2",		va("%i", newInfo->captures));
	}
}

void UI_LoadBestScores(const char *map, int game) {
	char		fileName[MAX_QPATH];
	fileHandle_t f;
	postGameInfo_t newInfo;
	memset(&newInfo, 0, sizeof(postGameInfo_t));
	Com_sprintf(fileName, MAX_QPATH, "games/%s_%i.game", map, game);
	if (FS_FOpenFileByMode(fileName, &f, FS_READ) >= 0) {
		int size = 0;
		FS_Read2(&size, sizeof(int), f);
		if (size == sizeof(postGameInfo_t)) {
			FS_Read2(&newInfo, sizeof(postGameInfo_t), f);
		}
		FS_FCloseFile(f);
	}
	UI_SetBestScores(&newInfo, false);

	Com_sprintf(fileName, MAX_QPATH, "demos/%s_%d.dm_%d", map, game, Cvar_VariableValue("protocol"));
	uiInfo.demoAvailable = false;
	if (FS_FOpenFileByMode(fileName, &f, FS_READ) >= 0) {
		uiInfo.demoAvailable = true;
		FS_FCloseFile(f);
	} 
}

/*
===============
UI_ClearScores
===============
*/
void UI_ClearScores() {
	char	gameList[4096];
	char *gameFile;
	int		i, len, count, size;
	fileHandle_t f;
	postGameInfo_t newInfo;

	count = FS_GetFileList( "games", "game", gameList, sizeof(gameList) );

	size = sizeof(postGameInfo_t);
	memset(&newInfo, 0, size);

	if (count > 0) {
		gameFile = gameList;
		for ( i = 0; i < count; i++ ) {
			len = strlen(gameFile);
			if (FS_FOpenFileByMode(va("games/%s",gameFile), &f, FS_WRITE) >= 0) {
				FS_Write(&size, sizeof(int), f);
				FS_Write(&newInfo, size, f);
				FS_FCloseFile(f);
			}
			gameFile += len + 1;
		}
	}
	
	UI_SetBestScores(&newInfo, false);

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
	Q_strncpyz( map, Info_ValueForKey( info, "mapname" ), sizeof(map) );
	game = atoi(Info_ValueForKey(info, "g_gametype"));

	// compose file name
	Com_sprintf(fileName, MAX_QPATH, "games/%s_%i.game", map, game);
	// see if we have one already
	memset(&oldInfo, 0, sizeof(postGameInfo_t));
	if (FS_FOpenFileByMode(fileName, &f, FS_READ) >= 0) {
	// if so load it
		size = 0;
		FS_Read2(&size, sizeof(int), f);
		if (size == sizeof(postGameInfo_t)) {
			FS_Read2(&oldInfo, sizeof(postGameInfo_t), f);
		}
		FS_FCloseFile(f);
	}					 

	newInfo.accuracy = atoi(UI_Argv(3));
	newInfo.impressives = atoi(UI_Argv(4));
	newInfo.excellents = atoi(UI_Argv(5));
	newInfo.defends = atoi(UI_Argv(6));
	newInfo.assists = atoi(UI_Argv(7));
	newInfo.gauntlets = atoi(UI_Argv(8));
	newInfo.baseScore = atoi(UI_Argv(9));
	newInfo.perfects = atoi(UI_Argv(10));
	newInfo.redScore = atoi(UI_Argv(11));
	newInfo.blueScore = atoi(UI_Argv(12));
	time = atoi(UI_Argv(13));
	newInfo.captures = atoi(UI_Argv(14));

	newInfo.time = (time - Cvar_VariableValue("ui_matchStartTime")) / 1000;
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

	newInfo.skillBonus = Cvar_VariableValue("g_spSkill");
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
		if (FS_FOpenFileByMode(fileName, &f, FS_WRITE) >= 0) {
			size = sizeof(postGameInfo_t);
			FS_Write(&size, sizeof(int), f);
			FS_Write(&newInfo, sizeof(postGameInfo_t), f);
			FS_FCloseFile(f);
		}
	}

	if (newInfo.time < oldInfo.time) {
		uiInfo.newBestTime = uiInfo.uiUtils.dc_->realTime_ + 20000;
	}
 
	// put back all the ui overrides
	Cvar_Set("capturelimit", UI_Cvar_VariableString("ui_saveCaptureLimit"));
	Cvar_Set("fraglimit", UI_Cvar_VariableString("ui_saveFragLimit"));
	Cvar_Set("cg_drawTimer", UI_Cvar_VariableString("ui_drawTimer"));
	Cvar_Set("g_doWarmup", UI_Cvar_VariableString("ui_doWarmup"));
	Cvar_Set("g_Warmup", UI_Cvar_VariableString("ui_Warmup"));
	Cvar_Set("sv_pure", UI_Cvar_VariableString("ui_pure"));
	Cvar_Set("g_friendlyFire", UI_Cvar_VariableString("ui_friendlyFire"));

	UI_SetBestScores(&newInfo, true);
	UI_ShowPostGame(newHigh);


}

/*
==================
MFQ3 Custom Chat
==================
*/

static void UI_Chat( int mode )
{
	// reset & begin
	Key_SetCatcher( KEYCATCH_UI );
	uiInfo.customChat.active = true;
	uiInfo.customChat.text[0] = 0;
	uiInfo.customChat.cindex = 0;
	uiInfo.customChat.mode = static_cast<CustomChatMode>(mode);
}
/*
=================
UI_ConsoleCommand
=================
*/
bool UI_ConsoleCommand( int realTime ) {
	char	*cmd;

	uiInfo.uiUtils.dc_->frameTime_ = realTime - uiInfo.uiUtils.dc_->realTime_;
	uiInfo.uiUtils.dc_->realTime_ = realTime;

	cmd = UI_Argv( 0 );

	// ensure minimum menu data is available
	//Menu_Cache();

	if ( Q_stricmp (cmd, "ui_test") == 0 ) {
		UI_ShowPostGame(true);
	}

	if ( Q_stricmp (cmd, "ui_report") == 0 ) {
		UI_Report();
		return true;
	}
	
	if ( Q_stricmp (cmd, "ui_load") == 0 ) {
		UI_Load();
		return true;
	}

	if ( Q_stricmp (cmd, "remapShader") == 0 ) {
		if (Cmd_Argc() == 4) {
			char shader1[MAX_QPATH];
			char shader2[MAX_QPATH];
			Q_strncpyz(shader1, UI_Argv(1), sizeof(shader1));
			Q_strncpyz(shader2, UI_Argv(2), sizeof(shader2));
			refExport.RemapShader(shader1, shader2, UI_Argv(3));
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
		Cvar_Set( "cl_paused", "1" );
		Key_SetCatcher( KEYCATCH_UI );
		uiInfo.uiUtils.menu_CloseAll();

		uiInfo.uiUtils.menu_ActivateByName( "ingame_select_team" );

		return true;
	}

	// MFQ3: bring up the 'vehicle selection' dialog (i.e. bind command to key)
	if( Q_stricmp (cmd, "vehicleselect") == 0 ) 
	{
		Cvar_Set( "cl_paused", "1" );
		Key_SetCatcher( KEYCATCH_UI );
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
UI_Shutdown
=================
*/
void UI_Shutdown( void ) {
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
#if 0
	*x = *x * uiInfo.uiDC.scale + uiInfo.uiDC.bias;
	*y *= uiInfo.uiDC.scale;
	*w *= uiInfo.uiDC.scale;
	*h *= uiInfo.uiDC.scale;
#endif

	*x *= uiInfo.uiUtils.dc_->xScale_;
	*y *= uiInfo.uiUtils.dc_->yScale_;
	*w *= uiInfo.uiUtils.dc_->xScale_;
	*h *= uiInfo.uiUtils.dc_->yScale_;

}

void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname ) {
	qhandle_t	hShader;

	hShader = refExport.RegisterShaderNoMip( picname );
	UI_AdjustFrom640( &x, &y, &width, &height );
	refExport.DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
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
	
	UI_AdjustFrom640( &x, &y, &w, &h );
	refExport.DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color ) 
{
	refExport.SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );
	refExport.DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );

	refExport.SetColor( NULL );
}

void UI_DrawSides(float x, float y, float w, float h) 
{
	UI_AdjustFrom640( &x, &y, &w, &h );
	refExport.DrawStretchPic( x, y, 1, h, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
	refExport.DrawStretchPic( x + w - 1, y, 1, h, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
}

void UI_DrawTopBottom(float x, float y, float w, float h) 
{
	UI_AdjustFrom640( &x, &y, &w, &h );
	refExport.DrawStretchPic( x, y, w, 1, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
	refExport.DrawStretchPic( x, y + h - 1, w, 1, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( float x, float y, float width, float height, const float *color ) {
	refExport.SetColor( color );

  UI_DrawTopBottom(x, y, width, height);
  UI_DrawSides(x, y, width, height);

	refExport.SetColor( NULL );
}

void UI_SetColor( const float *rgba ) 
{
	refExport.SetColor( rgba );
}

void UI_UpdateScreen( void ) 
{
	SCR_UpdateScreen();
}


void UI_DrawTextBox (int x, int y, int width, int lines)
{
	UI_FillRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorBlack );
	UI_DrawRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorWhite );
}

bool UI_CursorInRect (int x, int y, int width, int height)
{
	if (uiInfo.uiUtils.dc_->cursorX_ < x ||
		uiInfo.uiUtils.dc_->cursorY_ < y ||
		uiInfo.uiUtils.dc_->cursorX_ > x+width ||
		uiInfo.uiUtils.dc_->cursorY_ > y+height)
		return false;

	return true;
}
