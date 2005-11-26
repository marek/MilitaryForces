/*
 * $Id: ui_main.c,v 1.14 2005-11-26 10:06:29 thebjoern Exp $
*/
/*
=======================================================================

USER INTERFACE MAIN

=======================================================================
*/

#include "ui_local.h"
#include "ui_utils.h"
#include "ui_controls.h"
#include "ui_precomp.h"

uiInfo_t uiInfo;


static const char *MonthAbbrev[] = {
	"Jan","Feb","Mar",
	"Apr","May","Jun",
	"Jul","Aug","Sep",
	"Oct","Nov","Dec"
};


static const char *skillLevels[] = {
  "I Can Win",
  "Bring It On",
  "Hurt Me Plenty",
  "Hardcore",
  "Nightmare"
};

static const int numSkillLevels = sizeof(skillLevels) / sizeof(const char*);


static const char *netSources[] = {
	"Local",
	"Mplayer",
	"Internet",
	"Favorites"
};
static const int numNetSources = sizeof(netSources) / sizeof(const char*);

static const serverFilter_t serverFilters[] = {
	{"All", "" },
	{"Military Forces", "" },
};

static const char *teamArenaGameTypes[] = {
	"DM",
	"TOURNAMENT",
	"SP",
	"MISSIONEDITOR",
	"TDM",
	"CTF",
	"1FCTF",
	"OVERLOAD",
	"HARVESTER",
	"TEAMTOURNAMENT"
};

static int const numTeamArenaGameTypes = sizeof(teamArenaGameTypes) / sizeof(const char*);

static const char *mfqGamesets[] = {	// make sure this matches exactly with gameset_items[] of bg_vehicledata.c
	"MDRN",
	"WW2",
	"WW1"
};

static int const numGamesets = sizeof(mfqGamesets) / sizeof(const char*);

static const char *teamArenaGameNames[] = {
	"Deathmatch",
	"Tournament",
	"Single Player",
	"MFQ3 Mission Editor",
	"Team Deathmatch",
	"Capture the Flag",
	"One Flag CTF",
	"Overload",
	"Harvester",
	"Team Tournament",
};

static int const numTeamArenaGameNames = sizeof(teamArenaGameNames) / sizeof(const char*);


static const int numServerFilters = sizeof(serverFilters) / sizeof(serverFilter_t);

static const char *sortKeys[] = {
	"Server Name",
	"Map Name",
	"Open Player Spots",
	"Game Type",
	"Ping Time"
};
static const int numSortKeys = sizeof(sortKeys) / sizeof(const char*);

static char* netnames[] = {
	"???",
	"UDP",
	"IPX",
	NULL
};

static int gamecodetoui[] = {4,2,3,0,5,1,6};
static int uitogamecode[] = {4,6,2,3,1,5,7};


static void UI_StartServerRefresh(bool full);
static void UI_StopServerRefresh( void );
static void UI_DoServerRefresh( void );
void UI_FeederSelection(float feederID, int index);
static void UI_BuildServerDisplayList(int force);
static void UI_BuildServerStatus(bool force);
static void UI_BuildFindPlayerList(bool force);
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 );
static int UI_MapCountByGameType(bool singlePlayer);
static int UI_HeadCountByTeam( void );
static void UI_ParseGameInfo(const char *teamFile);
static void UI_ParseTeamInfo(const char *teamFile);
static const char *UI_SelectedMap(int index, int *actual);
static const char *UI_SelectedHead(int index, int *actual);
static int UI_GetIndexFromSelection(int actual);

int ProcessNewUI( int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6 );

void UI_CustomChatInit( void );


vmCvar_t  ui_new;
vmCvar_t  ui_debug;
vmCvar_t  ui_initialized;
vmCvar_t  ui_teamArenaFirstRun;

void _UI_Init( int );
void _UI_Shutdown( void );
void _UI_KeyEvent( int key, int down );
void _UI_MouseEvent( int dx, int dy );
void _UI_Refresh( int realtime );
bool _UI_IsFullscreen( void );



void AssetCache() 
{
	int n;
	uiInfo.uiUtils.dc_->assets_.gradientBar = refExport.RegisterShaderNoMip( ASSET_GRADIENTBAR );
	uiInfo.uiUtils.dc_->assets_.verticalGradient = refExport.RegisterShaderNoMip( ASSET_VERTICALGRADIENT );
	uiInfo.uiUtils.dc_->assets_.fxBasePic = refExport.RegisterShaderNoMip( ART_FX_BASE );
	uiInfo.uiUtils.dc_->assets_.fxPic[0] = refExport.RegisterShaderNoMip( ART_FX_RED );
	uiInfo.uiUtils.dc_->assets_.fxPic[1] = refExport.RegisterShaderNoMip( ART_FX_YELLOW );
	uiInfo.uiUtils.dc_->assets_.fxPic[2] = refExport.RegisterShaderNoMip( ART_FX_GREEN );
	uiInfo.uiUtils.dc_->assets_.fxPic[3] = refExport.RegisterShaderNoMip( ART_FX_TEAL );
	uiInfo.uiUtils.dc_->assets_.fxPic[4] = refExport.RegisterShaderNoMip( ART_FX_BLUE );
	uiInfo.uiUtils.dc_->assets_.fxPic[5] = refExport.RegisterShaderNoMip( ART_FX_CYAN );
	uiInfo.uiUtils.dc_->assets_.fxPic[6] = refExport.RegisterShaderNoMip( ART_FX_WHITE );
	uiInfo.uiUtils.dc_->assets_.scrollBar = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR );
	uiInfo.uiUtils.dc_->assets_.scrollBarArrowDown = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWDOWN );
	uiInfo.uiUtils.dc_->assets_.scrollBarArrowUp = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWUP );
	uiInfo.uiUtils.dc_->assets_.scrollBarArrowLeft = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWLEFT );
	uiInfo.uiUtils.dc_->assets_.scrollBarArrowRight = refExport.RegisterShaderNoMip( ASSET_SCROLLBAR_ARROWRIGHT );
	uiInfo.uiUtils.dc_->assets_.scrollBarThumb = refExport.RegisterShaderNoMip( ASSET_SCROLL_THUMB );
	uiInfo.uiUtils.dc_->assets_.sliderBar = refExport.RegisterShaderNoMip( ASSET_SLIDER_BAR );
	uiInfo.uiUtils.dc_->assets_.sliderThumb = refExport.RegisterShaderNoMip( ASSET_SLIDER_THUMB );

	for( n = 0; n < NUM_CROSSHAIRS; n++ ) {
		uiInfo.uiUtils.dc_->assets_.crosshairShader[n] = refExport.RegisterShaderNoMip( va("gfx/2d/crosshair%c", 'a' + n ) );
	}

	uiInfo.newHighScoreSound = S_RegisterSound("sound/feedback/voc_newhighscore.wav", false);
}

void _UI_DrawSides(float x, float y, float w, float h, float size) 
{
	UI_AdjustFrom640( &x, &y, &w, &h );
	size *= uiInfo.uiUtils.dc_->xScale_;
	refExport.DrawStretchPic( x, y, size, h, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
	refExport.DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
}

void _UI_DrawTopBottom(float x, float y, float w, float h, float size) 
{
	UI_AdjustFrom640( &x, &y, &w, &h );
	size *= uiInfo.uiUtils.dc_->yScale_;
	refExport.DrawStretchPic( x, y, w, size, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
	refExport.DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, uiInfo.uiUtils.dc_->whiteShader_ );
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void _UI_DrawRect( float x, float y, float width, float height, float size, const float *color ) 
{
	refExport.SetColor( color );

	_UI_DrawTopBottom(x, y, width, height, size);
	_UI_DrawSides(x, y, width, height, size);

	refExport.SetColor( NULL );
}




int Text_Width(const char *text, float scale, int limit) 
{
	int count,len;
	float out;
	glyphInfo_t *glyph;
	float useScale;
// TTimo: FIXME: use const unsigned char to avoid getting a warning in linux debug (.so) when using glyph = &font->glyphs[*s];
// but use const char to build with lcc..
// const unsigned char *s = text; // bk001206 - unsigned
	const char *s = text;
	fontInfo_t *font = &uiInfo.uiUtils.dc_->assets_.textFont;
	if (scale <= ui_smallFont.value) 
	{
		font = &uiInfo.uiUtils.dc_->assets_.smallFont;
	} 
	else if (scale >= ui_bigFont.value) 
	{
		font = &uiInfo.uiUtils.dc_->assets_.bigFont;
	}
	useScale = scale * font->glyphScale;
	out = 0;
	if (text) 
	{
		len = strlen(text);
		if (limit > 0 && len > limit) 
		{
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) 
		{
			if ( Q_IsColorString(s) ) 
			{
				s += 2;
				continue;
			}
			else 
			{
				glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
				out += glyph->xSkip;
				s++;
				count++;
			}
    }
  }
  return out * useScale;
}

int Text_Height(const char *text, float scale, int limit) 
{
	int len, count;
	float max;
	glyphInfo_t *glyph;
	float useScale;
// TTimo: FIXME
//	const unsigned char *s = text; // bk001206 - unsigned
	const char *s = text; // bk001206 - unsigned
	fontInfo_t *font = &uiInfo.uiUtils.dc_->assets_.textFont;
	if (scale <= ui_smallFont.value) 
	{
		font = &uiInfo.uiUtils.dc_->assets_.smallFont;
	} 
	else if (scale >= ui_bigFont.value) 
	{
		font = &uiInfo.uiUtils.dc_->assets_.bigFont;
	}
	useScale = scale * font->glyphScale;
	max = 0;
	if (text) 
	{
		len = strlen(text);
		if (limit > 0 && len > limit) 
		{
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) 
		{
			if ( Q_IsColorString(s) ) 
			{
				s += 2;
				continue;
			} 
			else 
			{
				glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
				if (max < glyph->height)
				{
					max = glyph->height;
				}
				s++;
				count++;
			}
		}
	}
	return max * useScale;
}

void Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader)
{
	float w, h;
	w = width * scale;
	h = height * scale;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refExport.DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void Text_Paint(float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit, int style) 
{
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float useScale;
	fontInfo_t *font = &uiInfo.uiUtils.dc_->assets_.textFont;
	if (scale <= ui_smallFont.value) 
	{
		font = &uiInfo.uiUtils.dc_->assets_.smallFont;
	} 
	else if (scale >= ui_bigFont.value) 
	{
		font = &uiInfo.uiUtils.dc_->assets_.bigFont;
	}
	useScale = scale * font->glyphScale;
	if (text) 
	{
// TTimo: FIXME  	
//    const unsigned char *s = text; // bk001206 - unsigned
		const char *s = text; // bk001206 - unsigned
		refExport.SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = strlen(text);
		if (limit > 0 && len > limit) 
			len = limit;

		count = 0;
		while (s && *s && count < len) 
		{
			glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if ( Q_IsColorString( s ) ) 
			{
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				refExport.SetColor( newColor );
				s += 2;
				continue;
			} 
			else 
			{
				float yadj = useScale * glyph->top;
				if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) 
				{
					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					refExport.SetColor( colorBlack );
					Text_PaintChar(x + ofs, y - yadj + ofs, 
														glyph->imageWidth,
														glyph->imageHeight,
														useScale, 
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph);
					refExport.SetColor( newColor );
					colorBlack[3] = 1.0;
				}
				Text_PaintChar(x, y - yadj, 
													glyph->imageWidth,
													glyph->imageHeight,
													useScale, 
													glyph->s,
													glyph->t,
													glyph->s2,
													glyph->t2,
													glyph->glyph);

				x += (glyph->xSkip * useScale) + adjust;
				s++;
				count++;
			}
		}
		refExport.SetColor( NULL );
	}
}

void Text_PaintWithCursor(float x, float y, float scale, const vec4_t color, const char *text, int cursorPos, 
						  char cursor, int limit, int style) 
{
  int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph, *glyph2;
	float yadj;
	float useScale;
	fontInfo_t *font = &uiInfo.uiUtils.dc_->assets_.textFont;
	if (scale <= ui_smallFont.value) {
		font = &uiInfo.uiUtils.dc_->assets_.smallFont;
	} else if (scale >= ui_bigFont.value) {
		font = &uiInfo.uiUtils.dc_->assets_.bigFont;
	}
	useScale = scale * font->glyphScale;
	if (text) 
	{
// TTimo: FIXME
//    const unsigned char *s = text; // bk001206 - unsigned
		const char *s = text; // bk001206 - unsigned
		refExport.SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = strlen(text);
		if (limit > 0 && len > limit) 
		{
			len = limit;
		}
		count = 0;
		glyph2 = &font->glyphs[ (int) cursor]; // bk001206 - possible signed char
		while (s && *s && count < len) 
		{
			glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if ( Q_IsColorString( s ) ) 
			{
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				refExport.SetColor( newColor );
				s += 2;
				continue;
			} 
			else 
			{
				yadj = useScale * glyph->top;
				if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) 
				{
					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					refExport.SetColor( colorBlack );
					Text_PaintChar(x + ofs, y - yadj + ofs, 
														glyph->imageWidth,
														glyph->imageHeight,
														useScale, 
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph);
					colorBlack[3] = 1.0;
					refExport.SetColor( newColor );
				}
				Text_PaintChar(x, y - yadj, 
													glyph->imageWidth,
													glyph->imageHeight,
													useScale, 
													glyph->s,
													glyph->t,
													glyph->s2,
													glyph->t2,
													glyph->glyph);

				// CG_DrawPic(x, y - yadj, scale * uiDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * uiDC.Assets.textFont.glyphs[text[i]].imageHeight, uiDC.Assets.textFont.glyphs[text[i]].glyph);
			yadj = useScale * glyph2->top;
		    if (count == cursorPos && !((uiInfo.uiUtils.dc_->realTime_/BLINK_DIVISOR) & 1)) 
			{
					Text_PaintChar(x, y - yadj, 
														glyph2->imageWidth,
														glyph2->imageHeight,
														useScale, 
														glyph2->s,
														glyph2->t,
														glyph2->s2,
														glyph2->t2,
														glyph2->glyph);
				}

				x += (glyph->xSkip * useScale);
				s++;
				count++;
			}
		}
		// need to paint cursor at end of text
		if (cursorPos == len && !((uiInfo.uiUtils.dc_->realTime_/BLINK_DIVISOR) & 1)) 
		{
			yadj = useScale * glyph2->top;
			Text_PaintChar(x, y - yadj, 
                          glyph2->imageWidth,
                          glyph2->imageHeight,
                          useScale, 
                          glyph2->s,
                          glyph2->t,
                          glyph2->s2,
                          glyph2->t2,
                          glyph2->glyph);

		}
		refExport.SetColor( NULL );
	}
}


static void Text_Paint_Limit(float *maxX, float x, float y, float scale, const vec4_t color, const char* text, 
							 float adjust, int limit) 
{
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	if (text) 
	{
// TTimo: FIXME
//    const unsigned char *s = text; // bk001206 - unsigned
		const char *s = text; // bk001206 - unsigned
		float max = *maxX;
		float useScale;
		fontInfo_t *font = &uiInfo.uiUtils.dc_->assets_.textFont;
		if (scale <= ui_smallFont.value) 
		{
			font = &uiInfo.uiUtils.dc_->assets_.smallFont;
		} 
		else if (scale > ui_bigFont.value) 
		{
			font = &uiInfo.uiUtils.dc_->assets_.bigFont;
		}
		useScale = scale * font->glyphScale;
		refExport.SetColor( color );
		len = strlen(text);					 
		if (limit > 0 && len > limit) 
		{
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) 
		{
			glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
			if ( Q_IsColorString( s ) ) 
			{
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				refExport.SetColor( newColor );
				s += 2;
				continue;
			} 
			else 
			{
				float yadj = useScale * glyph->top;
				if (Text_Width(s, useScale, 1) + x > max) 
				{
					*maxX = 0;
					break;
				}
				Text_PaintChar(x, y - yadj, 
			                 glyph->imageWidth,
				               glyph->imageHeight,
				               useScale, 
						           glyph->s,
								       glyph->t,
								       glyph->s2,
									     glyph->t2,
										   glyph->glyph);
				x += (glyph->xSkip * useScale) + adjust;
				*maxX = x;
				count++;
				s++;
			}
		}
		refExport.SetColor( NULL );
	}
}


void UI_ShowPostGame(bool newHigh) {
	Cvar_Set ("cg_cameraOrbit", "0");
	Cvar_Set("cg_thirdPerson", "0");
	Cvar_Set( "sv_killserver", "1" );
	uiInfo.soundHighScore = newHigh;
	_UI_SetActiveMenu(UserInterface::MenuCommand::UIMENU_POSTGAME);
}

/*
=================
UI_DrawCenteredPic
=================
*/
void UI_DrawCenteredPic(qhandle_t image, int w, int h) {
  int x, y;
  x = (SCREEN_WIDTH - w) / 2;
  y = (SCREEN_HEIGHT - h) / 2;
  UI_DrawHandlePic(x, y, w, h, image);
}

/*
=================
UI_DrawMouse
=================
*/
void UI_DrawMouse( void )
{
	// server info structure
	uiClientState_t	cstate;

	qhandle_t cursorHandle = -1;
	int offsetX = 0;
	int offsetY = 0;

	// get client information
	GetClientState( &cstate );

	UI_SetColor( NULL );

	// select cursor to draw
	switch( uiInfo.uiUtils.dc_->cursorEnum_ )
	{
	default:
	case CURSOR_NORMAL:
		cursorHandle = uiInfo.uiUtils.dc_->assets_.cursor;
		offsetX = 0;
		offsetY = 0;
		break;

	case CURSOR_WAIT:
		cursorHandle = uiInfo.uiUtils.dc_->assets_.cursorWait;
		offsetX = -7;
		offsetY = -12;
		break;
	}

	// draw cursor (not during loading/chat)
	if( uiInfo.uiUtils.menu_Count() > 0 && cstate.connState != CA_LOADING && !uiInfo.customChat.active )
	{
		UI_DrawHandlePic( uiInfo.uiUtils.dc_->cursorX_ + offsetX, uiInfo.uiUtils.dc_->cursorY_ + offsetY, 32, 32, cursorHandle );
	}

#ifndef NDEBUG
/*
	if (uiInfo.uiDC.debug)
	{
		// cursor coordinates
		UI_DrawString( 0, 0, va("(%d,%d)",uis.cursorx,uis.cursory), UI_LEFT|UI_SMALLFONT, colorRed );
	}
*/
#endif
}

int frameCount = 0;
int startTime;

#define	UI_FPS_FRAMES	4

/*
=================
_UI_Refresh
=================
*/
void _UI_Refresh( int realtime )
{
	static int index;
	static int previousTimes[UI_FPS_FRAMES];
/*
	if ( !( Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}
*/
	uiInfo.uiUtils.dc_->frameTime_ = realtime - uiInfo.uiUtils.dc_->realTime_;
	uiInfo.uiUtils.dc_->realTime_ = realtime;

	previousTimes[index % UI_FPS_FRAMES] = uiInfo.uiUtils.dc_->frameTime_;
	index++;
	if ( index > UI_FPS_FRAMES ) {
		int i, total;
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < UI_FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		uiInfo.uiUtils.dc_->fps_ = 1000 * UI_FPS_FRAMES / total;
	}

	UI_UpdateCvars();

	if (uiInfo.uiUtils.menu_Count() > 0) {
		// paint all the menus
		uiInfo.uiUtils.menu_PaintAll();
		// refresh server browser list
		UI_DoServerRefresh();
		// refresh server status
		UI_BuildServerStatus(false);
		// refresh find player list
		UI_BuildFindPlayerList(false);
	} 

	// render the mouse cursor
	UI_DrawMouse();

	// draw the custom chat
	uiInfo.uiUtils.customChatDraw();
}

/*
=================
_UI_Shutdown
=================
*/
void _UI_Shutdown() 
{
	LAN_SaveServersToCache();
}

char *defaultMenu = NULL;

char *GetMenuBuffer(const char *filename) {
	int	len;
	fileHandle_t	f;
	static char buf[MAX_MENUFILE];

	len = FS_FOpenFileByMode( filename, &f, FS_READ );
	if ( !f ) {
		Com_Printf( S_COLOR_RED "menu file not found: %s, using default\n", filename );
		return defaultMenu;
	}
	if ( len >= MAX_MENUFILE ) {
		Com_Printf( S_COLOR_RED "menu file too large: %s is %i, max allowed is %i", filename, len, MAX_MENUFILE );
		FS_FCloseFile( f );
		return defaultMenu;
	}

	FS_Read2( buf, len, f );
	buf[len] = 0;
	FS_FCloseFile( f );
	//COM_Compress(buf);
  return buf;

}

bool Asset_Parse(int handle) 
{
	UI_PrecompilerTools::PC_Token  token;
	const char *tempStr;

	if (!uiInfo.uiUtils.getPrecompilerTools()->readTokenHandle(handle, &token))
		return false;
	if (Q_stricmp(token.string_, "{") != 0) {
		return false;
	}
    
	while ( 1 ) {

		memset(&token, 0, sizeof(UI_PrecompilerTools::PC_Token ));

		if (!uiInfo.uiUtils.getPrecompilerTools()->readTokenHandle(handle, &token))
			return false;

		if (Q_stricmp(token.string_, "}") == 0) {
			return true;
		}

		// font
		if (Q_stricmp(token.string_, "font") == 0) {
			int pointSize;
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &tempStr) || !uiInfo.uiUtils.pc_Int_Parse(handle,&pointSize)) {
				return false;
			}
			refExport.RegisterFont(tempStr, pointSize, &uiInfo.uiUtils.dc_->assets_.textFont);
			uiInfo.uiUtils.dc_->assets_.fontRegistered = true;
			continue;
		}

		if (Q_stricmp(token.string_, "smallFont") == 0) {
			int pointSize;
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &tempStr) || !uiInfo.uiUtils.pc_Int_Parse(handle,&pointSize)) {
				return false;
			}
			refExport.RegisterFont(tempStr, pointSize, &uiInfo.uiUtils.dc_->assets_.smallFont);
			continue;
		}

		if (Q_stricmp(token.string_, "bigFont") == 0) {
			int pointSize;
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &tempStr) || !uiInfo.uiUtils.pc_Int_Parse(handle,&pointSize)) {
				return false;
			}
			refExport.RegisterFont(tempStr, pointSize, &uiInfo.uiUtils.dc_->assets_.bigFont);
			continue;
		}


		// gradientbar
		if (Q_stricmp(token.string_, "gradientbar") == 0) {
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			uiInfo.uiUtils.dc_->assets_.gradientBar = refExport.RegisterShaderNoMip(tempStr);
			continue;
		}

		// enterMenuSound
		if (Q_stricmp(token.string_, "menuEnterSound") == 0) {
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			uiInfo.uiUtils.dc_->assets_.menuEnterSound = S_RegisterSound( tempStr, false );
			continue;
		}

		// exitMenuSound
		if (Q_stricmp(token.string_, "menuExitSound") == 0) {
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			uiInfo.uiUtils.dc_->assets_.menuExitSound = S_RegisterSound( tempStr, false );
			continue;
		}

		// itemFocusSound
		if (Q_stricmp(token.string_, "itemFocusSound") == 0) {
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			uiInfo.uiUtils.dc_->assets_.itemFocusSound = S_RegisterSound( tempStr, false );
			continue;
		}

		// menuBuzzSound
		if (Q_stricmp(token.string_, "menuBuzzSound") == 0) {
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &tempStr)) {
				return false;
			}
			uiInfo.uiUtils.dc_->assets_.menuBuzzSound = S_RegisterSound( tempStr, false );
			continue;
		}

		if (Q_stricmp(token.string_, "cursor") == 0) {
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &uiInfo.uiUtils.dc_->assets_.cursorStr )) {
				return false;
			}
			uiInfo.uiUtils.dc_->assets_.cursor = refExport.RegisterShaderNoMip( uiInfo.uiUtils.dc_->assets_.cursorStr );
			continue;
		}

		if (Q_stricmp(token.string_, "cursorWait") == 0) {
			if (!uiInfo.uiUtils.pc_String_Parse(handle, &uiInfo.uiUtils.dc_->assets_.cursorStrWait )) {
				return false;
			}
			uiInfo.uiUtils.dc_->assets_.cursorWait = refExport.RegisterShaderNoMip( uiInfo.uiUtils.dc_->assets_.cursorStrWait );
			continue;
		}

		if (Q_stricmp(token.string_, "fadeClamp") == 0) {
			if (!uiInfo.uiUtils.pc_Float_Parse(handle, &uiInfo.uiUtils.dc_->assets_.fadeClamp)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "fadeCycle") == 0) {
			if (!uiInfo.uiUtils.pc_Int_Parse(handle, &uiInfo.uiUtils.dc_->assets_.fadeCycle)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "fadeAmount") == 0) {
			if (!uiInfo.uiUtils.pc_Float_Parse(handle, &uiInfo.uiUtils.dc_->assets_.fadeAmount)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "shadowX") == 0) {
			if (!uiInfo.uiUtils.pc_Float_Parse(handle, &uiInfo.uiUtils.dc_->assets_.shadowX)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "shadowY") == 0) {
			if (!uiInfo.uiUtils.pc_Float_Parse(handle, &uiInfo.uiUtils.dc_->assets_.shadowY)) {
				return false;
			}
			continue;
		}

		if (Q_stricmp(token.string_, "shadowColor") == 0) {
			if (!uiInfo.uiUtils.pc_Color_Parse(handle, &uiInfo.uiUtils.dc_->assets_.shadowColor)) {
				return false;
			}
			uiInfo.uiUtils.dc_->assets_.shadowFadeClamp = uiInfo.uiUtils.dc_->assets_.shadowColor[3];
			continue;
		}

	}
	return false;
}

void Font_Report() 
{
	int i;
	Com_Printf("Font Info\n");
	Com_Printf("=========\n");
	for ( i = 32; i < 96; i++) 
	{
		Com_Printf("Glyph handle %i: %i\n", i, uiInfo.uiUtils.dc_->assets_.textFont.glyphs[i].glyph);
	}
}

void UI_Report() 
{
	uiInfo.uiUtils.string_Report();
	//Font_Report();

}

void UI_ParseMenu(const char *menuFile) 
{
	int handle;
	UI_PrecompilerTools::PC_Token  token;

	Com_Printf("Parsing menu file:%s\n", menuFile);

	handle = uiInfo.uiUtils.getPrecompilerTools()->loadSourceHandle(menuFile);
	if (!handle) {
		return;
	}

	while ( 1 ) {
		memset(&token, 0, sizeof(UI_PrecompilerTools::PC_Token ));
		if (!uiInfo.uiUtils.getPrecompilerTools()->readTokenHandle( handle, &token )) {
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
			if (Asset_Parse(handle)) {
				continue;
			} else {
				break;
			}
		}

		if (Q_stricmp(token.string_, "menudef") == 0) {
			// start a New menu
			uiInfo.uiUtils.menu_New(handle);
		}
	}
	uiInfo.uiUtils.getPrecompilerTools()->freeSourceHandle(handle);
}

bool Load_Menu(int handle) 
{
	UI_PrecompilerTools::PC_Token  token;

	if (!uiInfo.uiUtils.getPrecompilerTools()->readTokenHandle(handle, &token))
		return false;
	if (token.string_[0] != '{') {
		return false;
	}

	while ( 1 ) {

		if (!uiInfo.uiUtils.getPrecompilerTools()->readTokenHandle(handle, &token))
			return false;
    
		if ( token.string_[0] == 0 ) {
			return false;
		}

		if ( token.string_[0] == '}' ) {
			return true;
		}

		UI_ParseMenu(token.string_); 
	}
	return false;
}

void UI_LoadMenus(const char *menuFile, bool reset) 
{
	UI_PrecompilerTools::PC_Token  token;
	int handle;
	int start;

	start = Sys_Milliseconds();

	handle = uiInfo.uiUtils.getPrecompilerTools()->loadSourceHandle( menuFile );
	if (!handle) {
		Com_Error( ERR_DROP, S_COLOR_YELLOW "menu file not found: %s, using default\n", menuFile );
		handle = uiInfo.uiUtils.getPrecompilerTools()->loadSourceHandle( "ui/menus.txt" );
		if (!handle) {
			Com_Error( ERR_DROP, S_COLOR_RED "default menu file not found: ui/menus.txt, unable to continue!\n", menuFile );
		}
	}

	ui_new.integer = 1;

	if (reset) {
		uiInfo.uiUtils.menu_Reset();
	}

	while ( 1 ) {
		if (!uiInfo.uiUtils.getPrecompilerTools()->readTokenHandle(handle, &token))
			break;
		if( token.string_[0] == 0 || token.string_[0] == '}') {
			break;
		}

		if ( token.string_[0] == '}' ) {
			break;
		}

		if (Q_stricmp(token.string_, "loadmenu") == 0) {
			if (Load_Menu(handle)) {
				continue;
			} else {
				break;
			}
		}
	}

	//Com_Printf("UI menu load time = %d milli seconds\n", ServerUIApi::milliseconds() - start);

	uiInfo.uiUtils.getPrecompilerTools()->freeSourceHandle( handle );
}

void UI_Load() 
{
	char lastName[1024];
	menuDef_t *menu = uiInfo.uiUtils.menu_GetFocused();

	char *menuSet = UI_Cvar_VariableString("ui_menuFiles");
	if (menu && menu->window.name) {
		strcpy(lastName, menu->window.name);
	}
	if (menuSet == NULL || menuSet[0] == '\0') {
		menuSet = "ui/menus.txt";
	}

	uiInfo.uiUtils.string_Init();

	UI_ParseGameInfo("gameinfo.txt");
	UI_LoadArenas();

	UI_LoadMenus(menuSet, true);
	uiInfo.uiUtils.menu_CloseAll();
	uiInfo.uiUtils.menu_ActivateByName(lastName);

}

static const char *handicapValues[] = {"None","95","90","85","80","75","70","65","60","55","50","45","40","35","30","25","20","15","10","5",NULL};

static void UI_DrawHandicap(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  int i, h;

  h = Com_Clamp( 5, 100, Cvar_VariableValue("handicap") );
  i = 20 - h / 5;

  Text_Paint(rect->x, rect->y, scale, color, handicapValues[i], 0, 0, textStyle);
}

static void UI_DrawClanName(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  Text_Paint(rect->x, rect->y, scale, color, UI_Cvar_VariableString("ui_teamName"), 0, 0, textStyle);
}


static void UI_SetCapFragLimits(bool uiVars) {
	int cap = 5;
	int frag = 10;

	if (uiVars) {
		Cvar_Set("ui_captureLimit", va("%d", cap));
		Cvar_Set("ui_fragLimit", va("%d", frag));
	} else {
		Cvar_Set("capturelimit", va("%d", cap));
		Cvar_Set("fraglimit", va("%d", frag));
	}
}
// ui_gameType assumes gametype 0 is -1 ALL and will not show
static void UI_DrawGameType(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  Text_Paint(rect->x, rect->y, scale, color, uiInfo.gameTypes[ui_gameType.integer].gameType, 0, 0, textStyle);
}

static void UI_DrawNetGameType(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
	if (ui_netGameType.integer < 0 || ui_netGameType.integer > uiInfo.numGameTypes) {
		Cvar_Set("ui_netGameType", "0");
		Cvar_Set("ui_actualNetGameType", "0");
	}
  Text_Paint(rect->x, rect->y, scale, color, uiInfo.gameTypes[ui_netGameType.integer].gameType , 0, 0, textStyle);
}

//static void UI_DrawNetGameset(rectDef_t *rect, float scale, vec4_t color, int textStyle) {
//	if (ui_netGameset.integer < 0 || ui_netGameset.integer > numGamesets) {
//		Cvar_Set("ui_netGameset", "0");
//	}
//  Text_Paint(rect->x, rect->y, scale, color, gameset_items[ui_netGameset.integer] , 0, 0, textStyle);
//}

static void UI_DrawJoinGameType(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
	if (ui_joinGameType.integer < 0 || ui_joinGameType.integer > uiInfo.numJoinGameTypes) {
		Cvar_Set("ui_joinGameType", "0");
	}
  Text_Paint(rect->x, rect->y, scale, color, uiInfo.joinGameTypes[ui_joinGameType.integer].gameType , 0, 0, textStyle);
}



static int UI_TeamIndexFromName(const char *name) {
  int i;

  if (name && *name) {
    for (i = 0; i < uiInfo.teamCount; i++) {
      if (Q_stricmp(name, uiInfo.teamList[i].teamName) == 0) {
        return i;
      }
    }
  } 

  return 0;

}

static void UI_DrawClanLogo(rectDef_t *rect, float scale, const vec4_t color) {
  int i;
  i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
  if (i >= 0 && i < uiInfo.teamCount) {
  	refExport.SetColor( color );

		if (uiInfo.teamList[i].teamIcon == -1) {
      uiInfo.teamList[i].teamIcon = refExport.RegisterShaderNoMip(uiInfo.teamList[i].imageName);
      uiInfo.teamList[i].teamIcon_Metal = refExport.RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[i].imageName));
      uiInfo.teamList[i].teamIcon_Name = refExport.RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
		}

  	UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon);
    refExport.SetColor(NULL);
  }
}

static void UI_DrawClanCinematic(rectDef_t *rect, float scale, const vec4_t color) {
  int i;
  i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
  if (i >= 0 && i < uiInfo.teamCount) {

		if (uiInfo.teamList[i].cinematic >= -2) {
			if (uiInfo.teamList[i].cinematic == -1) {
				uiInfo.teamList[i].cinematic = CIN_PlayCinematic(va("%s.roq", uiInfo.teamList[i].imageName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
			}
			if (uiInfo.teamList[i].cinematic >= 0) {
			  CIN_RunCinematic(uiInfo.teamList[i].cinematic);
				CIN_SetExtents(uiInfo.teamList[i].cinematic, rect->x, rect->y, rect->w, rect->h);
	 			CIN_DrawCinematic(uiInfo.teamList[i].cinematic);
			} else {
			  	refExport.SetColor( color );
				UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Metal);
				refExport.SetColor(NULL);
				uiInfo.teamList[i].cinematic = -2;
			}
		} else {
	  	refExport.SetColor( color );
			UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon);
			refExport.SetColor(NULL);
		}
	}

}

static void UI_DrawPreviewCinematic(rectDef_t *rect, float scale, const vec4_t color) {
	if (uiInfo.previewMovie > -2) {
		uiInfo.previewMovie = CIN_PlayCinematic(va("%s.roq", uiInfo.movieList[uiInfo.movieIndex]), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		if (uiInfo.previewMovie >= 0) {
			CIN_RunCinematic(uiInfo.previewMovie);
			CIN_SetExtents(uiInfo.previewMovie, rect->x, rect->y, rect->w, rect->h);
 			CIN_DrawCinematic(uiInfo.previewMovie);
		} else {
			uiInfo.previewMovie = -2;
		}
	} 

}



static void UI_DrawSkill(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  int i;
	i = Cvar_VariableValue( "g_spSkill" );
  if (i < 1 || i > numSkillLevels) {
    i = 1;
  }
  Text_Paint(rect->x, rect->y, scale, color, skillLevels[i-1],0, 0, textStyle);
}


static void UI_DrawTeamName(rectDef_t *rect, float scale, const vec4_t color, bool blue, int textStyle) {
  int i;
  i = UI_TeamIndexFromName(UI_Cvar_VariableString((blue) ? "ui_blueTeam" : "ui_redTeam"));
  if (i >= 0 && i < uiInfo.teamCount) {
    Text_Paint(rect->x, rect->y, scale, color, va("%s: %s", (blue) ? "Blue" : "Red", uiInfo.teamList[i].teamName),0, 0, textStyle);
  }
}

static void UI_DrawTeamMember(rectDef_t *rect, float scale, const vec4_t color, bool blue, int num, int textStyle) {
	// 0 - None
	// 1 - Human
	// 2..NumCharacters - Bot
	int value = Cvar_VariableValue(va(blue ? "ui_blueteam%i" : "ui_redteam%i", num));
	const char *text;
	if (value <= 0) 
	{
		text = "Closed";
	} 
	else if (value == 1) 
	{
		text = "Human";
	} 
	else 
	{
		value -= 2;

		if (ui_actualNetGameType.integer >= GT_TEAM) 
		{
			if (value >= uiInfo.characterCount) 
			{
				value = 0;
			}
			text = uiInfo.characterList[value].name;
		} 
		//else 
		//{
		//	if (value >= UI_GetNumBots()) 
		//	{
		//		value = 0;
		//	}
		//	text = UI_GetBotNameByNumber(value);
		//}
	}
	Text_Paint(rect->x, rect->y, scale, color, text, 0, 0, textStyle);
}

static void UI_DrawEffects(rectDef_t *rect, float scale, const vec4_t color) 
{
	UI_DrawHandlePic( rect->x, rect->y - 14, 128, 8, uiInfo.uiUtils.dc_->assets_.fxBasePic );
	UI_DrawHandlePic( rect->x + uiInfo.effectsColor * 16 + 8, rect->y - 16, 16, 12, 
		uiInfo.uiUtils.dc_->assets_.fxPic[uiInfo.effectsColor] );
}

static void UI_DrawMapPreview(rectDef_t *rect, float scale, const vec4_t color, bool net) 
{
	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
	if (map < 0 || map > uiInfo.mapCount) 
	{
		if (net) 
		{
			ui_currentNetMap.integer = 0;
			Cvar_Set("ui_currentNetMap", "0");
		} 
		else 
		{
			ui_currentMap.integer = 0;
			Cvar_Set("ui_currentMap", "0");
		}
		map = 0;
	}

	if (uiInfo.mapList[map].levelShot == -1) 
	{
		uiInfo.mapList[map].levelShot = refExport.RegisterShaderNoMip(uiInfo.mapList[map].imageName);
	}

	if (uiInfo.mapList[map].levelShot > 0) 
	{
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.mapList[map].levelShot);
	} 
	else 
	{
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, refExport.RegisterShaderNoMip("ui/assets/unknownmap_preview") );
	}
}						 


static void UI_DrawMapTimeToBeat(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
	int minutes, seconds, time;
	if (ui_currentMap.integer < 0 || ui_currentMap.integer > uiInfo.mapCount) {
		ui_currentMap.integer = 0;
		Cvar_Set("ui_currentMap", "0");
	}

	time = uiInfo.mapList[ui_currentMap.integer].timeToBeat[uiInfo.gameTypes[ui_gameType.integer].gtEnum];

	minutes = time / 60;
	seconds = time % 60;

  Text_Paint(rect->x, rect->y, scale, color, va("%02i:%02i", minutes, seconds), 0, 0, textStyle);
}



static void UI_DrawMapCinematic(rectDef_t *rect, float scale, const vec4_t color, bool net)
{
	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer; 
	if( map < 0 || map > uiInfo.mapCount )
	{
		if (net)
		{
			ui_currentNetMap.integer = 0;
			Cvar_Set("ui_currentNetMap", "0");
		}
		else
		{
			ui_currentMap.integer = 0;
			Cvar_Set("ui_currentMap", "0");
		}
		map = 0;
	}

	// uninitialised or setup?
	if( uiInfo.mapList[map].cinematic >= -1 )
	{
		// try to load ROQ?
		if( uiInfo.mapList[map].cinematic == -1 )
		{
			uiInfo.mapList[map].cinematic = CIN_PlayCinematic( va("%s.roq", uiInfo.mapList[map].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		}

		// ROQ available and loaded OK?
		if( uiInfo.mapList[map].cinematic >= 0 )
		{
			// play ROQ
			CIN_RunCinematic(uiInfo.mapList[map].cinematic);
			CIN_SetExtents(uiInfo.mapList[map].cinematic, rect->x, rect->y, rect->w, rect->h);
 			CIN_DrawCinematic(uiInfo.mapList[map].cinematic);
		}
		else
		{
			// default to map preview
			uiInfo.mapList[map].cinematic = -2;
		}
	}
	
	// just draw map preview?
	if( uiInfo.mapList[map].cinematic == -2 )
	{
		UI_DrawMapPreview(rect, scale, color, net);
	}
}

static bool updateModel = true;
static bool q3Model = false;

static void UI_DrawNetSource(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
	if (ui_netSource.integer < 0 || ui_netSource.integer > uiInfo.numGameTypes) {
		ui_netSource.integer = 0;
	}
  Text_Paint(rect->x, rect->y, scale, color, netSources[ui_netSource.integer], 0, 0, textStyle);
}

static void UI_DrawNetMapPreview(rectDef_t *rect, float scale, const vec4_t color) {

	if (uiInfo.serverStatus.currentServerPreview > 0) {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.serverStatus.currentServerPreview);
	} else {
		UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, refExport.RegisterShaderNoMip("menu/art/unknownmap"));
	}
}

static void UI_DrawNetMapCinematic(rectDef_t *rect, float scale, const vec4_t color) {
	if (ui_currentNetMap.integer < 0 || ui_currentNetMap.integer > uiInfo.mapCount) {
		ui_currentNetMap.integer = 0;
		Cvar_Set("ui_currentNetMap", "0");
	}

	if (uiInfo.serverStatus.currentServerCinematic >= 0) {
	  CIN_RunCinematic(uiInfo.serverStatus.currentServerCinematic);
	  CIN_SetExtents(uiInfo.serverStatus.currentServerCinematic, rect->x, rect->y, rect->w, rect->h);
 	  CIN_DrawCinematic(uiInfo.serverStatus.currentServerCinematic);
	} else {
		UI_DrawNetMapPreview(rect, scale, color);
	}
}



static void UI_DrawNetFilter(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
	if (ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters) {
		ui_serverFilterType.integer = 0;
	}
  Text_Paint(rect->x, rect->y, scale, color, va("Filter: %s", serverFilters[ui_serverFilterType.integer].description), 0, 0, textStyle);
}


static void UI_DrawTier(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  int i;
	i = Cvar_VariableValue( "ui_currentTier" );
  if (i < 0 || i >= uiInfo.tierCount) {
    i = 0;
  }
  Text_Paint(rect->x, rect->y, scale, color, va("Tier: %s", uiInfo.tierList[i].tierName),0, 0, textStyle);
}

static void UI_DrawTierMap(rectDef_t *rect, int index) {
  int i;
	i = Cvar_VariableValue( "ui_currentTier" );
  if (i < 0 || i >= uiInfo.tierCount) {
    i = 0;
  }

	if (uiInfo.tierList[i].mapHandles[index] == -1) {
		uiInfo.tierList[i].mapHandles[index] = refExport.RegisterShaderNoMip(va("levelshots/%s", uiInfo.tierList[i].maps[index]));
	}
												 
	UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.tierList[i].mapHandles[index]);
}

static const char *UI_EnglishMapName(const char *map) {
	int i;
	for (i = 0; i < uiInfo.mapCount; i++) {
		if (Q_stricmp(map, uiInfo.mapList[i].mapLoadName) == 0) {
			return uiInfo.mapList[i].mapName;
		}
	}
	return "";
}

static void UI_DrawTierMapName(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  int i, j;
	i = Cvar_VariableValue( "ui_currentTier" );
  if (i < 0 || i >= uiInfo.tierCount) {
    i = 0;
  }
	j = Cvar_VariableValue("ui_currentMap");
	if (j < 0 || j > MAPS_PER_TIER) {
		j = 0;
	}

  Text_Paint(rect->x, rect->y, scale, color, UI_EnglishMapName(uiInfo.tierList[i].maps[j]), 0, 0, textStyle);
}

static void UI_DrawTierGameType(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  int i, j;
	i = Cvar_VariableValue( "ui_currentTier" );
  if (i < 0 || i >= uiInfo.tierCount) {
    i = 0;
  }
	j = Cvar_VariableValue("ui_currentMap");
	if (j < 0 || j > MAPS_PER_TIER) {
		j = 0;
	}

  Text_Paint(rect->x, rect->y, scale, color, uiInfo.gameTypes[uiInfo.tierList[i].gameTypes[j]].gameType , 0, 0, textStyle);
}



static const char *UI_AIFromName(const char *name) {
	int j;
	for (j = 0; j < uiInfo.aliasCount; j++) {
		if (Q_stricmp(uiInfo.aliasList[j].name, name) == 0) {
			return uiInfo.aliasList[j].ai;
		}
	}
	return "James";
}



static bool updateOpponentModel = true;

static void UI_NextOpponent() {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  int j = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
	i++;
	if (i >= uiInfo.teamCount) {
		i = 0;
	}
	if (i == j) {
		i++;
		if ( i >= uiInfo.teamCount) {
			i = 0;
		}
	}
 	Cvar_Set( "ui_opponentName", uiInfo.teamList[i].teamName );
}

static void UI_PriorOpponent() {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
  int j = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
	i--;
	if (i < 0) {
		i = uiInfo.teamCount - 1;
	}
	if (i == j) {
		i--;
		if ( i < 0) {
			i = uiInfo.teamCount - 1;
		}
	}
 	Cvar_Set( "ui_opponentName", uiInfo.teamList[i].teamName );
}

static void	UI_DrawPlayerLogo(rectDef_t *rect, const vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

	if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon = refExport.RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = refExport.RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name = refExport.RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
	}

 	refExport.SetColor( color );
	UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon );
 	refExport.SetColor( NULL );
}

static void	UI_DrawPlayerLogoMetal(rectDef_t *rect, const vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
	if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon = refExport.RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = refExport.RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name = refExport.RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
	}

 	refExport.SetColor( color );
	UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Metal );
 	refExport.SetColor( NULL );
}

static void	UI_DrawPlayerLogoName(rectDef_t *rect, const vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
	if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon = refExport.RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = refExport.RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name = refExport.RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
	}

 	refExport.SetColor( color );
	UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Name );
 	refExport.SetColor( NULL );
}

static void	UI_DrawOpponentLogo(rectDef_t *rect, const vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
	if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon = refExport.RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = refExport.RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name = refExport.RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
	}

 	refExport.SetColor( color );
	UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon );
 	refExport.SetColor( NULL );
}

static void	UI_DrawOpponentLogoMetal(rectDef_t *rect, const vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
	if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon = refExport.RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = refExport.RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name = refExport.RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
	}

 	refExport.SetColor( color );
	UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Metal );
 	refExport.SetColor( NULL );
}

static void	UI_DrawOpponentLogoName(rectDef_t *rect, const vec3_t color) {
  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
	if (uiInfo.teamList[i].teamIcon == -1) {
    uiInfo.teamList[i].teamIcon = refExport.RegisterShaderNoMip(uiInfo.teamList[i].imageName);
    uiInfo.teamList[i].teamIcon_Metal = refExport.RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[i].imageName));
    uiInfo.teamList[i].teamIcon_Name = refExport.RegisterShaderNoMip(va("%s_name", uiInfo.teamList[i].imageName));
	}

 	refExport.SetColor( color );
	UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, uiInfo.teamList[i].teamIcon_Name );
 	refExport.SetColor( NULL );
}

static void UI_DrawAllMapsSelection(rectDef_t *rect, float scale, const vec4_t color, int textStyle, bool net) {
	int map = (net) ? ui_currentNetMap.integer : ui_currentMap.integer;
	if (map >= 0 && map < uiInfo.mapCount) {
	  Text_Paint(rect->x, rect->y, scale, color, uiInfo.mapList[map].mapName, 0, 0, textStyle);
	}
}

static void UI_DrawOpponentName(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  Text_Paint(rect->x, rect->y, scale, color, UI_Cvar_VariableString("ui_opponentName"), 0, 0, textStyle);
}


int UI_OwnerDrawWidth(int ownerDraw, float scale) 
{
	int i, h, value;
	const char *text;
	const char *s = NULL;

  switch (ownerDraw) {
    case UI_HANDICAP:
			  h = Com_Clamp( 5, 100, Cvar_VariableValue("handicap") );
				i = 20 - h / 5;
				s = handicapValues[i];
      break;
    case UI_CLANNAME:
				s = UI_Cvar_VariableString("ui_teamName");
      break;
    case UI_GAMETYPE:
				s = uiInfo.gameTypes[ui_gameType.integer].gameType;
      break;
    case UI_SKILL:
				i = Cvar_VariableValue( "g_spSkill" );
				if (i < 1 || i > numSkillLevels) {
					i = 1;
				}
			  s = skillLevels[i-1];
      break;
    case UI_BLUETEAMNAME:
			  i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_blueTeam"));
			  if (i >= 0 && i < uiInfo.teamCount) {
			    s = va("%s: %s", "Blue", uiInfo.teamList[i].teamName);
			  }
      break;
    case UI_REDTEAMNAME:
			  i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_redTeam"));
			  if (i >= 0 && i < uiInfo.teamCount) {
			    s = va("%s: %s", "Red", uiInfo.teamList[i].teamName);
			  }
      break;
    case UI_BLUETEAM1:
		case UI_BLUETEAM2:
		case UI_BLUETEAM3:
		case UI_BLUETEAM4:
		case UI_BLUETEAM5:
			value = Cvar_VariableValue(va("ui_blueteam%i", ownerDraw-UI_BLUETEAM1 + 1));
			if (value <= 0) {
				text = "Closed";
			} else if (value == 1) {
				text = "Human";
			} else {
				value -= 2;
				if (value >= uiInfo.aliasCount) {
					value = 0;
				}
				text = uiInfo.aliasList[value].name;
			}
			s = va("%i. %s", ownerDraw-UI_BLUETEAM1 + 1, text);
      break;
    case UI_REDTEAM1:
		case UI_REDTEAM2:
		case UI_REDTEAM3:
		case UI_REDTEAM4:
		case UI_REDTEAM5:
			value = Cvar_VariableValue(va("ui_redteam%i", ownerDraw-UI_REDTEAM1 + 1));
			if (value <= 0) {
				text = "Closed";
			} else if (value == 1) {
				text = "Human";
			} else {
				value -= 2;
				if (value >= uiInfo.aliasCount) {
					value = 0;
				}
				text = uiInfo.aliasList[value].name;
			}
			s = va("%i. %s", ownerDraw-UI_REDTEAM1 + 1, text);
      break;
		case UI_NETSOURCE:
			if (ui_netSource.integer < 0 || ui_netSource.integer > uiInfo.numJoinGameTypes) {
				ui_netSource.integer = 0;
			}
			s = va("Source: %s", netSources[ui_netSource.integer]);
			break;
		case UI_NETFILTER:
			if (ui_serverFilterType.integer < 0 || ui_serverFilterType.integer > numServerFilters) {
				ui_serverFilterType.integer = 0;
			}
			s = va("Filter: %s", serverFilters[ui_serverFilterType.integer].description );
			break;
		case UI_TIER:
			break;
		case UI_TIER_MAPNAME:
			break;
		case UI_TIER_GAMETYPE:
			break;
		case UI_ALLMAPS_SELECTION:
			break;
		case UI_OPPONENT_NAME:
			break;
		case UI_KEYBINDSTATUS:
			if (uiInfo.uiUtils.display_KeyBindPending()) {
				s = "Waiting for New key... Press ESCAPE to cancel";
			} else {
				s = "Press ENTER or CLICK to change, Press BACKSPACE to clear";
			}
			break;
		case UI_SERVERREFRESHDATE:
			s = UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer));
			break;
    default:
      break;
  }

	if (s) {
		return Text_Width(s, scale, 0);
	}
	return 0;
}

//static void UI_DrawBotName(rectDef_t *rect, float scale, vec4_t color, int textStyle) 
//{
//	int value = uiInfo.botIndex;
//	int game = Cvar_VariableValue("g_gametype");
//	const char *text = "";
//	if (game >= GT_TEAM) 
//	{
//		if (value >= uiInfo.characterCount) 
//		{
//			value = 0;
//		}
//		text = uiInfo.characterList[value].name;
//	} 
//	else 
//	{
//		if (value >= UI_GetNumBots()) 
//		{
//			value = 0;
//		}
//		text = UI_GetBotNameByNumber(value);
//	}
//	Text_Paint(rect->x, rect->y, scale, color, text, 0, 0, textStyle);
//}

//static void UI_DrawBotSkill(rectDef_t *rect, float scale, vec4_t color, int textStyle) {
//	if (uiInfo.skillIndex >= 0 && uiInfo.skillIndex < numSkillLevels) {
//	  Text_Paint(rect->x, rect->y, scale, color, skillLevels[uiInfo.skillIndex], 0, 0, textStyle);
//	}
//}

static void UI_DrawRedBlue(rectDef_t *rect, float scale, const vec4_t color, int textStyle) {
  Text_Paint(rect->x, rect->y, scale, color, (uiInfo.redBlue == 0) ? "Red" : "Blue", 0, 0, textStyle);
}

static void UI_DrawCrosshair(rectDef_t *rect, float scale, const vec4_t color) {
 	refExport.SetColor( color );
	if (uiInfo.currentCrosshair < 0 || uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
		uiInfo.currentCrosshair = 0;
	}
	UI_DrawHandlePic( rect->x, rect->y - rect->h, rect->w, rect->h, uiInfo.uiUtils.dc_->assets_.crosshairShader[uiInfo.currentCrosshair]);
 	refExport.SetColor( NULL );
}

/*
===============
UI_BuildPlayerList
===============
*/
static void UI_BuildPlayerList() 
{
	uiClientState_t	cs;
	int		n, count, team, team2, playerTeamNumber;
	char	info[MAX_INFO_STRING];

	GetClientState( &cs );
	GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
	uiInfo.playerNumber = cs.clientNum;
	uiInfo.teamLeader = atoi(Info_ValueForKey(info, "tl"));
	team = atoi(Info_ValueForKey(info, "t"));
	GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	count = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	uiInfo.playerCount = 0;
	uiInfo.myTeamCount = 0;
	playerTeamNumber = 0;
	for( n = 0; n < count; n++ ) 
	{
		GetConfigString( CS_PLAYERS + n, info, MAX_INFO_STRING );

		if (info[0]) 
		{
			Q_strncpyz( uiInfo.playerNames[uiInfo.playerCount], Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
			Q_CleanStr( uiInfo.playerNames[uiInfo.playerCount] );
			uiInfo.playerCount++;
			team2 = atoi(Info_ValueForKey(info, "t"));
			if (team2 == team) 
			{
				Q_strncpyz( uiInfo.teamNames[uiInfo.myTeamCount], Info_ValueForKey( info, "n" ), MAX_NAME_LENGTH );
				Q_CleanStr( uiInfo.teamNames[uiInfo.myTeamCount] );
				uiInfo.teamClientNums[uiInfo.myTeamCount] = n;
				if (uiInfo.playerNumber == n) 
					playerTeamNumber = uiInfo.myTeamCount;
				uiInfo.myTeamCount++;
			}
		}
	}

	if (!uiInfo.teamLeader) 
		Cvar_Set("cg_selectedPlayer", va("%d", playerTeamNumber));

	n = Cvar_VariableValue("cg_selectedPlayer");
	if (n < 0 || n > uiInfo.myTeamCount) 
		n = 0;
	if (n < uiInfo.myTeamCount) 
		Cvar_Set("cg_selectedPlayerName", uiInfo.teamNames[n]);
}


static void UI_DrawSelectedPlayer(rectDef_t *rect, float scale, const vec4_t color, int textStyle) 
{
	if (uiInfo.uiUtils.dc_->realTime_ > uiInfo.playerRefresh) {
		uiInfo.playerRefresh = uiInfo.uiUtils.dc_->realTime_ + 3000;
		UI_BuildPlayerList();
	}
  Text_Paint(rect->x, rect->y, scale, color, (uiInfo.teamLeader) ? UI_Cvar_VariableString("cg_selectedPlayerName") : 
			UI_Cvar_VariableString("name") , 0, 0, textStyle);
}

static void UI_DrawServerRefreshDate(rectDef_t *rect, float scale, const vec4_t color, int textStyle) 
{
	if (uiInfo.serverStatus.refreshActive) 
	{
		vec4_t lowLight, newColor;
		lowLight[0] = 0.8 * color[0]; 
		lowLight[1] = 0.8 * color[1]; 
		lowLight[2] = 0.8 * color[2]; 
		lowLight[3] = 0.8 * color[3]; 
		uiInfo.uiUtils.lerpColor(color,lowLight,newColor,0.5f+0.5f*sinf(uiInfo.uiUtils.dc_->realTime_ / PULSE_DIVISOR));
	  Text_Paint(rect->x, rect->y, scale, newColor, va("Getting info for %d servers (ESC to cancel)", LAN_GetServerCount(ui_netSource.integer)), 0, 0, textStyle);
	} 
	else 
	{
		char buff[64];
		Q_strncpyz(buff, UI_Cvar_VariableString(va("ui_lastServerRefresh_%i", ui_netSource.integer)), 64);
		Text_Paint(rect->x, rect->y, scale, color, va("Refresh Time: %s", buff), 0, 0, textStyle);
	}
}

static void UI_DrawServerMOTD(rectDef_t *rect, float scale, const vec4_t color) {
	if (uiInfo.serverStatus.motdLen) {
		float maxX;
	 
		if (uiInfo.serverStatus.motdWidth == -1) {
			uiInfo.serverStatus.motdWidth = 0;
			uiInfo.serverStatus.motdPaintX = rect->x + 1;
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

		if (uiInfo.serverStatus.motdOffset > uiInfo.serverStatus.motdLen) {
			uiInfo.serverStatus.motdOffset = 0;
			uiInfo.serverStatus.motdPaintX = rect->x + 1;
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

		if (uiInfo.uiUtils.dc_->realTime_ > uiInfo.serverStatus.motdTime) {
			uiInfo.serverStatus.motdTime = uiInfo.uiUtils.dc_->realTime_ + 10;
			if (uiInfo.serverStatus.motdPaintX <= rect->x + 2) {
				if (uiInfo.serverStatus.motdOffset < uiInfo.serverStatus.motdLen) {
					uiInfo.serverStatus.motdPaintX += Text_Width(&uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], scale, 1) - 1;
					uiInfo.serverStatus.motdOffset++;
				} else {
					uiInfo.serverStatus.motdOffset = 0;
					if (uiInfo.serverStatus.motdPaintX2 >= 0) {
						uiInfo.serverStatus.motdPaintX = uiInfo.serverStatus.motdPaintX2;
					} else {
						uiInfo.serverStatus.motdPaintX = rect->x + rect->w - 2;
					}
					uiInfo.serverStatus.motdPaintX2 = -1;
				}
			} else {
				//serverStatus.motdPaintX--;
				uiInfo.serverStatus.motdPaintX -= 2;
				if (uiInfo.serverStatus.motdPaintX2 >= 0) {
					//serverStatus.motdPaintX2--;
					uiInfo.serverStatus.motdPaintX2 -= 2;
				}
			}
		}

		maxX = rect->x + rect->w - 2;
		Text_Paint_Limit(&maxX, uiInfo.serverStatus.motdPaintX, rect->y + rect->h - 3, scale, color, &uiInfo.serverStatus.motd[uiInfo.serverStatus.motdOffset], 0, 0); 
		if (uiInfo.serverStatus.motdPaintX2 >= 0) {
			float maxX2 = rect->x + rect->w - 2;
			Text_Paint_Limit(&maxX2, uiInfo.serverStatus.motdPaintX2, rect->y + rect->h - 3, scale, color, uiInfo.serverStatus.motd, 0, uiInfo.serverStatus.motdOffset); 
		}
		if (uiInfo.serverStatus.motdOffset && maxX > 0) {
			// if we have an offset ( we are skipping the first part of the string ) and we fit the string
			if (uiInfo.serverStatus.motdPaintX2 == -1) {
						uiInfo.serverStatus.motdPaintX2 = rect->x + rect->w - 2;
			}
		} else {
			uiInfo.serverStatus.motdPaintX2 = -1;
		}

	}
}

static void UI_DrawKeyBindStatus(rectDef_t *rect, float scale, const vec4_t color, int textStyle) 
{
//	int ofs = 0; TTimo: unused
	if (uiInfo.uiUtils.display_KeyBindPending()) 
	{
		Text_Paint(rect->x, rect->y, scale, color, "Waiting for New key... Press ESCAPE to cancel", 0, 0, textStyle);
	} 
	else 
	{
		Text_Paint(rect->x, rect->y, scale, color, "Press ENTER or CLICK to change, Press BACKSPACE to clear", 0, 0, textStyle);
	}
}

static void UI_DrawGLInfo(rectDef_t *rect, float scale, const vec4_t color, int textStyle) 
{
	char * eptr;
	char buff[1024];
	const char *lines[64];
	int y, numLines, i;

	Text_Paint(rect->x + 2, rect->y, scale, color, va("VENDOR: %s", uiInfo.uiUtils.dc_->glConfig_.vendor_string), 0, 30, textStyle);
	Text_Paint(rect->x + 2, rect->y + 15, scale, color, va("VERSION: %s: %s", uiInfo.uiUtils.dc_->glConfig_.version_string,
		uiInfo.uiUtils.dc_->glConfig_.renderer_string), 0, 30, textStyle);
	Text_Paint(rect->x + 2, rect->y + 30, scale, color, va ("PIXELFORMAT: color(%d-bits) Z(%d-bits) stencil(%d-bits)", 
		uiInfo.uiUtils.dc_->glConfig_.colorBits, uiInfo.uiUtils.dc_->glConfig_.depthBits, uiInfo.uiUtils.dc_->glConfig_.stencilBits), 0, 30,
		textStyle);

	// build null terminated extension strings
  // in TA this was not directly crashing, but displaying a nasty broken shader right in the middle
  // brought down the string size to 1024, there's not much that can be shown on the screen anyway
	Q_strncpyz(buff, uiInfo.uiUtils.dc_->glConfig_.extensions_string, 1024);
	eptr = buff;
	y = rect->y + 45;
	numLines = 0;
	while ( y < rect->y + rect->h && *eptr )
	{
		while ( *eptr && *eptr == ' ' )
			*eptr++ = '\0';

		// track start of valid string
		if (*eptr && *eptr != ' ') {
			lines[numLines++] = eptr;
		}

		while ( *eptr && *eptr != ' ' )
			eptr++;
	}

	i = 0;
	while (i < numLines) {
		Text_Paint(rect->x + 2, y, scale, color, lines[i++], 0, 20, textStyle);
		if (i < numLines) {
			Text_Paint(rect->x + rect->w / 2, y, scale, color, lines[i++], 0, 20, textStyle);
		}
		y += 10;
		if (y > rect->y + rect->h - 11) {
			break;
		}
	}


}

/*
=================
UI_DrawVehiclePreview
=================
*/
static void UI_DrawVehiclePreview( rectDef_t *rect, float scale, const vec4_t color, int textStyle )
{
#if 0
	// * EXPERIMENTAL MODEL RATHER THAN PICTURE CODE *
	refdef_t		refdef;
	refEntity_t		plane;
	vec3_t			origin;
	vec3_t			angles;
	float			x, y, w, h;
	
	static int		old = 0;
	static float	angle = 180;
	static qhandle_t hModel = 0;
	
	if( !hModel ) 
		hModel = refExport.RegisterModel("models/mapobjects/f-14/f-14.md3");

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );

	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 58;
	refdef.fov_y = 8;
	refdef.time = uiInfo.uiUtils.dc_->realTime_;

	refExport.ClearScene();

	// add the model
	origin[0] = 520;
	origin[1] = -50;
	origin[2] = -20;
	memset( &plane, 0, sizeof(plane) );
	VectorSet( angles, 10, angle, 0 );
	AnglesToAxis( angles, plane.axis );
	plane.hModel = hModel;
	VectorCopy( origin, plane.origin );
	VectorCopy( origin, plane.lightingOrigin );
	plane.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( plane.origin, plane.oldorigin );

	refExport.AddRefEntityToScene( &plane );

	refExport.RenderScene( &refdef );

	// spin
	if( uiInfo.uiDC.realTime > old + 20 ) { 
		angle++;
		old =  uiInfo.uiDC.realTime;
	}
	if( angle >= 360 ) angle -= 360;
	//	UI_FillRect(rect->x, rect->y, rect->w, rect->h, color );
/*
	Text_Paint(rect->x, rect->y, scale, color, "Waiting for New key... Press ESCAPE to cancel", 0, 0, textStyle);
*/
#else
	// DRAW VEHICLE ICON CODE
	int vehicle = -1;
	int vehicleCat = -1;
//	char * dir = NULL;
	char * pIconString = NULL;
	static qhandle_t pictureHandle = -1;
	static int lastVehicle = -1;

	// find out the current UI vehicle index
	vehicle = Cvar_VariableValue("ui_vehicle");
	vehicleCat = Cvar_VariableValue("ui_vehicleCat");

	// valid?
	if( vehicle >= 0 )
	{
		if(UI_CAT_GI == vehicleCat &&  vehicle < bg_numberOfGroundInstallations) {
			if( pictureHandle == -1 || lastVehicle != vehicle ) {
				// create filename string
				pIconString = va( "models/vehicles/npc/%s/%s_icon", availableGroundInstallations[ vehicle ].modelName, availableGroundInstallations[ vehicle ].modelName );
				pictureHandle = refExport.RegisterShaderNoMip( pIconString );
			}
		} else if (vehicle < bg_numberOfVehicles) {
			// register different shader? (only do this on change)
			if( pictureHandle == -1 || lastVehicle != vehicle) {
				// create filename string
				pIconString = MF_CreateModelPathname( vehicle, "models/vehicles/%s/%s/%s_icon" );
				pictureHandle = refExport.RegisterShaderNoMip( pIconString );
			}
		}

		// draw (if valid picture handle)
		if( pictureHandle )
			UI_DrawHandlePic( rect->x, rect->y, rect->w, rect->h, pictureHandle );

		// save last done
		lastVehicle = vehicle;
	}
	else
	{
		// invalid vehicle draw nothing
	}

#endif
}

/*
=================
UI_DrawCredits
=================
*/

#define	HOFFSET			4
#define	VOFFSET			4
#define SVSPACER		12
#define BVSPACER		18
#define	SPEED_DIVIDER	40

static void UI_DrawCredits( rectDef_t *rect, float scale, const vec4_t inColor, int textStyle )
{
	// statics
	static float z = 0;
	static bool initialised = false;
	vec4_t color;

	memcpy( color, inColor, sizeof(vec4_t) );

	// dynamic locals
	int bx = rect->x + HOFFSET, by = rect->y + VOFFSET;
	int centreY = rect->y + (rect->h * 0.5f);
	int i, y = 0;

	// initialise?
	if( initialised == false )
	{
		z = rect->h - BVSPACER;

		// never again
		initialised = true;
	}

	// for all credit lines
	for( i=0; i<uiInfo.creditLines; i++ )
	{
		// default size is 'small'
		int spacer = SVSPACER;
		float tScale = 0.0f;
		bool draw = false;
		int drawY = 0;
		char firstChar = 0;

		// get line
		char * pLine = uiInfo.pCredits[ i ];

		// move to base of text
		y += spacer;
		drawY = by + y + z;

		// which text size?
		firstChar = *pLine++;
		if( firstChar == 'B' )
		{
			// 'big'
			tScale = 0.35f;
			spacer = BVSPACER;
			draw = true;
			color[2] = 0.5f;
		}
		else if( firstChar == 'S' )
		{
			// 'small'
			tScale = 0.25f;
			spacer = SVSPACER; 
			draw = true;
			color[2] = 1.0f;
		}
		else if( firstChar == 'R' && (drawY < by) )
		{
			// reset
			initialised = false;
		}

		// draw line
		if( draw )
		{
			float alpha = 0.0f;

			// outside draw zone? (approximations on bounding box - this is just to reduce draws)
			if( drawY < rect->y || drawY > (rect->y+rect->h) )
			{
				continue;
			}

			// alter alpha based upon distance from centre vertical
			alpha = 1.0f - ((float)abs( centreY - drawY )/90) + 0.40f;
			MF_LimitFloat( &alpha, 0.0f, 1.0f );
			color[3] = alpha;

			Text_Paint( bx, drawY, tScale, color, pLine, 0, 0, textStyle );
		}
	}
	
	// scroll
	z -= (float)uiInfo.uiUtils.dc_->frameTime_/SPEED_DIVIDER;
}

/*
=================
UI_OwnerDraw
=================
*/
void UI_OwnerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, 
				  int align, float special, float scale, const vec4_t color, qhandle_t shader, 
				  int textStyle, itemDef_t* item) 
{
	rectDef_t rect;

  rect.x = x + text_x;
  rect.y = y + text_y;
  rect.w = w;
  rect.h = h;

  switch (ownerDraw) {
    case UI_HANDICAP:
      UI_DrawHandicap(&rect, scale, color, textStyle);
      break;
    case UI_EFFECTS:
      UI_DrawEffects(&rect, scale, color);
      break;
    case UI_CLANNAME:
      UI_DrawClanName(&rect, scale, color, textStyle);
      break;
    case UI_CLANLOGO:
      UI_DrawClanLogo(&rect, scale, color);
      break;
    case UI_CLANCINEMATIC:
      UI_DrawClanCinematic(&rect, scale, color);
      break;
    case UI_PREVIEWCINEMATIC:
      UI_DrawPreviewCinematic(&rect, scale, color);
      break;
    case UI_GAMETYPE:
      UI_DrawGameType(&rect, scale, color, textStyle);
      break;
    case UI_NETGAMETYPE:
      UI_DrawNetGameType(&rect, scale, color, textStyle);
      break;
    case UI_NETGAMESET:
      //UI_DrawNetGameset(&rect, scale, color, textStyle);
      break;
    case UI_JOINGAMETYPE:
	  UI_DrawJoinGameType(&rect, scale, color, textStyle);
	  break;
    case UI_MAPPREVIEW:
      UI_DrawMapPreview(&rect, scale, color, true);
      break;
    case UI_MAP_TIMETOBEAT:
      UI_DrawMapTimeToBeat(&rect, scale, color, textStyle);
      break;
    case UI_MAPCINEMATIC:
      UI_DrawMapCinematic(&rect, scale, color, false);
      break;
    case UI_STARTMAPCINEMATIC:
      UI_DrawMapCinematic(&rect, scale, color, true);
      break;
    case UI_SKILL:
      UI_DrawSkill(&rect, scale, color, textStyle);
      break;
    case UI_BLUETEAMNAME:
      UI_DrawTeamName(&rect, scale, color, true, textStyle);
      break;
    case UI_REDTEAMNAME:
      UI_DrawTeamName(&rect, scale, color, false, textStyle);
      break;
    case UI_BLUETEAM1:
		case UI_BLUETEAM2:
		case UI_BLUETEAM3:
		case UI_BLUETEAM4:
		case UI_BLUETEAM5:
      UI_DrawTeamMember(&rect, scale, color, true, ownerDraw - UI_BLUETEAM1 + 1, textStyle);
      break;
    case UI_REDTEAM1:
		case UI_REDTEAM2:
		case UI_REDTEAM3:
		case UI_REDTEAM4:
		case UI_REDTEAM5:
      UI_DrawTeamMember(&rect, scale, color, false, ownerDraw - UI_REDTEAM1 + 1, textStyle);
      break;
		case UI_NETSOURCE:
      UI_DrawNetSource(&rect, scale, color, textStyle);
			break;
    case UI_NETMAPPREVIEW:
      UI_DrawNetMapPreview(&rect, scale, color);
      break;
    case UI_NETMAPCINEMATIC:
      UI_DrawNetMapCinematic(&rect, scale, color);
      break;
		case UI_NETFILTER:
      UI_DrawNetFilter(&rect, scale, color, textStyle);
			break;
		case UI_TIER:
			UI_DrawTier(&rect, scale, color, textStyle);
			break;
		case UI_TIERMAP1:
			UI_DrawTierMap(&rect, 0);
			break;
		case UI_TIERMAP2:
			UI_DrawTierMap(&rect, 1);
			break;
		case UI_TIERMAP3:
			UI_DrawTierMap(&rect, 2);
			break;
		case UI_PLAYERLOGO:
			UI_DrawPlayerLogo(&rect, color);
			break;
		case UI_PLAYERLOGO_METAL:
			UI_DrawPlayerLogoMetal(&rect, color);
			break;
		case UI_PLAYERLOGO_NAME:
			UI_DrawPlayerLogoName(&rect, color);
			break;
		case UI_OPPONENTLOGO:
			UI_DrawOpponentLogo(&rect, color);
			break;
		case UI_OPPONENTLOGO_METAL:
			UI_DrawOpponentLogoMetal(&rect, color);
			break;
		case UI_OPPONENTLOGO_NAME:
			UI_DrawOpponentLogoName(&rect, color);
			break;
		case UI_TIER_MAPNAME:
			UI_DrawTierMapName(&rect, scale, color, textStyle);
			break;
		case UI_TIER_GAMETYPE:
			UI_DrawTierGameType(&rect, scale, color, textStyle);
			break;
		case UI_ALLMAPS_SELECTION:
			UI_DrawAllMapsSelection(&rect, scale, color, textStyle, true);
			break;
		case UI_MAPS_SELECTION:
			UI_DrawAllMapsSelection(&rect, scale, color, textStyle, false);
			break;
		case UI_OPPONENT_NAME:
			UI_DrawOpponentName(&rect, scale, color, textStyle);
			break;
		//case UI_BOTNAME:
		//	UI_DrawBotName(&rect, scale, color, textStyle);
		//	break;
		//case UI_BOTSKILL:
		//	UI_DrawBotSkill(&rect, scale, color, textStyle);
		//	break;
		case UI_REDBLUE:
			UI_DrawRedBlue(&rect, scale, color, textStyle);
			break;
		case UI_CROSSHAIR:
			UI_DrawCrosshair(&rect, scale, color);
			break;
		case UI_SELECTEDPLAYER:
			UI_DrawSelectedPlayer(&rect, scale, color, textStyle);
			break;
		case UI_SERVERREFRESHDATE:
			UI_DrawServerRefreshDate(&rect, scale, color, textStyle);
			break;
		case UI_SERVERMOTD:
			UI_DrawServerMOTD(&rect, scale, color);
			break;
		case UI_GLINFO:
			UI_DrawGLInfo(&rect,scale, color, textStyle);
			break;
		case UI_KEYBINDSTATUS:
			UI_DrawKeyBindStatus(&rect,scale, color, textStyle);
			break;
		// MFQ3: drawing a preview vehicle model
		case UI_PREVIEW_VEHICLE:
			UI_DrawVehiclePreview(&rect,scale, color, textStyle);
			break;
		// MFQ3: drawing the credits
		case UI_CREDITS:
			UI_DrawCredits(&rect,scale, color, textStyle);
			break;

    default:
      break;
  }

}

bool UI_OwnerDrawVisible(int flags) 
{
	bool vis = true;

	while (flags) {

		if (flags & UI_SHOW_FFA) {
			if (Cvar_VariableValue("g_gametype") != GT_FFA) {
				vis = false;
			}
			flags &= ~UI_SHOW_FFA;
		}

		if (flags & UI_SHOW_NOTFFA) {
			if (Cvar_VariableValue("g_gametype") == GT_FFA) {
				vis = false;
			}
			flags &= ~UI_SHOW_NOTFFA;
		}

		if (flags & UI_SHOW_LEADER) {
			// these need to show when this client can give orders to a player or a group
			if (!uiInfo.teamLeader) {
				vis = false;
			} else {
				// if showing yourself
				if (ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber) { 
					vis = false;
				}
			}
			flags &= ~UI_SHOW_LEADER;
		} 
		if (flags & UI_SHOW_NOTLEADER) {
			// these need to show when this client is assigning their own status or they are NOT the leader
			if (uiInfo.teamLeader) {
				// if not showing yourself
				if (!(ui_selectedPlayer.integer < uiInfo.myTeamCount && uiInfo.teamClientNums[ui_selectedPlayer.integer] == uiInfo.playerNumber)) { 
					vis = false;
				}
				// these need to show when this client can give orders to a player or a group
			}
			flags &= ~UI_SHOW_NOTLEADER;
		} 
		if (flags & UI_SHOW_FAVORITESERVERS) {
			// this assumes you only put this type of display flag on something showing in the proper context
			if (ui_netSource.integer != AS_FAVORITES) {
				vis = false;
			}
			flags &= ~UI_SHOW_FAVORITESERVERS;
		} 
		if (flags & UI_SHOW_NOTFAVORITESERVERS) {
			// this assumes you only put this type of display flag on something showing in the proper context
			if (ui_netSource.integer == AS_FAVORITES) {
				vis = false;
			}
			flags &= ~UI_SHOW_NOTFAVORITESERVERS;
		} 
		if (flags & UI_SHOW_ANYTEAMGAME) {
			if (uiInfo.gameTypes[ui_gameType.integer].gtEnum <= GT_TEAM ) {
				vis = false;
			}
			flags &= ~UI_SHOW_ANYTEAMGAME;
		} 
		if (flags & UI_SHOW_ANYNONTEAMGAME) {
			if (uiInfo.gameTypes[ui_gameType.integer].gtEnum > GT_TEAM ) {
				vis = false;
			}
			flags &= ~UI_SHOW_ANYNONTEAMGAME;
		} 
		if (flags & UI_SHOW_NETANYTEAMGAME) {
			if (uiInfo.gameTypes[ui_netGameType.integer].gtEnum <= GT_TEAM ) {
				vis = false;
			}
			flags &= ~UI_SHOW_NETANYTEAMGAME;
		} 
		if (flags & UI_SHOW_NETANYNONTEAMGAME) {
			if (uiInfo.gameTypes[ui_netGameType.integer].gtEnum > GT_TEAM ) {
				vis = false;
			}
			flags &= ~UI_SHOW_NETANYNONTEAMGAME;
		} 
		if (flags & UI_SHOW_NEWHIGHSCORE) {
			if (uiInfo.newHighScoreTime < uiInfo.uiUtils.dc_->realTime_) {
				vis = false;
			} else {
				if (uiInfo.soundHighScore) {
					if (Cvar_VariableValue("sv_killserver") == 0) {
						// wait on server to go down before playing sound
						S_StartLocalSound(uiInfo.newHighScoreSound, CHAN_ANNOUNCER);
						uiInfo.soundHighScore = false;
					}
				}
			}
			flags &= ~UI_SHOW_NEWHIGHSCORE;
		} 
		if (flags & UI_SHOW_NEWBESTTIME) {
			if (uiInfo.newBestTime < uiInfo.uiUtils.dc_->realTime_) {
				vis = false;
			}
			flags &= ~UI_SHOW_NEWBESTTIME;
		} 
		if (flags & UI_SHOW_DEMOAVAILABLE) {
			if (!uiInfo.demoAvailable) {
				vis = false;
			}
			flags &= ~UI_SHOW_DEMOAVAILABLE;
		} else {
			flags = 0;
		}
	}
  return vis;
}

static bool UI_Handicap_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
    int h;
    h = Com_Clamp( 5, 100, Cvar_VariableValue("handicap") );
		if (key == K_MOUSE2) {
	    h -= 5;
		} else {
	    h += 5;
		}
    if (h > 100) {
      h = 5;
    } else if (h < 0) {
			h = 100;
		}
  	Cvar_Set( "handicap", va( "%i", h) );
    return true;
  }
  return false;
}

static bool UI_Effects_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {

		if (key == K_MOUSE2) {
	    uiInfo.effectsColor--;
		} else {
	    uiInfo.effectsColor++;
		}

    if( uiInfo.effectsColor > 6 ) {
	  	uiInfo.effectsColor = 0;
		} else if (uiInfo.effectsColor < 0) {
	  	uiInfo.effectsColor = 6;
		}

	  Cvar_SetValue( "color1", uitogamecode[uiInfo.effectsColor] );
    return true;
  }
  return false;
}

static bool UI_ClanName_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
    int i;
    i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
		if (uiInfo.teamList[i].cinematic >= 0) {
		  CIN_StopCinematic(uiInfo.teamList[i].cinematic);
			uiInfo.teamList[i].cinematic = -1;
		}
		if (key == K_MOUSE2) {
	    i--;
		} else {
	    i++;
		}
    if (i >= uiInfo.teamCount) {
      i = 0;
    } else if (i < 0) {
			i = uiInfo.teamCount - 1;
		}
  	Cvar_Set( "ui_teamName", uiInfo.teamList[i].teamName);
	UI_HeadCountByTeam();
	UI_FeederSelection(FEEDER_HEADS, 0);
	updateModel = true;
    return true;
  }
  return false;
}

static bool UI_GameType_HandleKey(int flags, float *special, int key, bool resetMap) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		int oldCount = UI_MapCountByGameType(true);

		// hard coded mess here
		if (key == K_MOUSE2) {
			ui_gameType.integer--;
			if (ui_gameType.integer == 2) {
				ui_gameType.integer = 1;
			} else if (ui_gameType.integer < 2) {
				ui_gameType.integer = uiInfo.numGameTypes - 1;
			}
		} else {
			ui_gameType.integer++;
			if (ui_gameType.integer >= uiInfo.numGameTypes) {
				ui_gameType.integer = 1;
			} else if (ui_gameType.integer == 2) {
				ui_gameType.integer = 3;
			}
		}
    
		if (uiInfo.gameTypes[ui_gameType.integer].gtEnum == GT_TOURNAMENT) {
			Cvar_Set("ui_Q3Model", "1");
		} else {
			Cvar_Set("ui_Q3Model", "0");
		}

		Cvar_Set("ui_gameType", va("%d", ui_gameType.integer));
		UI_SetCapFragLimits(true);
		UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
		if (resetMap && oldCount != UI_MapCountByGameType(true)) 
		{
	  		Cvar_Set( "ui_currentMap", "0");
			uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_MAPS, 0, NULL);
		}
    return true;
  }
  return false;
}

static bool UI_NetGameType_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {

		if (key == K_MOUSE2) {
			ui_netGameType.integer--;
		} else {
			ui_netGameType.integer++;
		}

    if (ui_netGameType.integer < 0) {
      ui_netGameType.integer = uiInfo.numGameTypes - 1;
		} else if (ui_netGameType.integer >= uiInfo.numGameTypes) {
      ui_netGameType.integer = 0;
    } 

  	Cvar_Set( "ui_netGameType", va("%d", ui_netGameType.integer));
  	Cvar_Set( "ui_actualnetGameType", va("%d", uiInfo.gameTypes[ui_netGameType.integer].gtEnum));
  	Cvar_Set( "ui_currentNetMap", "0");
	UI_MapCountByGameType(false);
	uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, NULL);
    return true;
  }
  return false;
}

//static bool UI_NetGameset_HandleKey(int flags, float *special, int key)
//{
//	// valid action?
//	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER)
//	{
//		// cycle up/down through values
//		if (key == K_MOUSE2)
//		{
//			ui_netGameset.integer--;
//		}
//		else
//		{
//			ui_netGameset.integer++;
//		}
//
//		// wrap (both ends)
//		if( ui_netGameset.integer < 0 )
//		{
//			ui_netGameset.integer = numGamesets - 1;
//		}
//		else if (ui_netGameset.integer >= numGamesets )
//		{
//			ui_netGameset.integer = 0;
//		} 
//
//		// re-set the value into cvar
//		Cvar_Set( "ui_netGameset", va("%d", ui_netGameset.integer));
//
//		return true;
//	}
//
//	return false;
//}

static bool UI_JoinGameType_HandleKey(int flags, float *special, int key) {
	if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {

		if (key == K_MOUSE2) {
			ui_joinGameType.integer--;
		} else {
			ui_joinGameType.integer++;
		}

		if (ui_joinGameType.integer < 0) {
			ui_joinGameType.integer = uiInfo.numJoinGameTypes - 1;
		} else if (ui_joinGameType.integer >= uiInfo.numJoinGameTypes) {
			ui_joinGameType.integer = 0;
		}

		Cvar_Set( "ui_joinGameType", va("%d", ui_joinGameType.integer));
		UI_BuildServerDisplayList(1);
		return true;
	}
	return false;
}



static bool UI_Skill_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
  	int i = Cvar_VariableValue( "g_spSkill" );

		if (key == K_MOUSE2) {
	    i--;
		} else {
	    i++;
		}

    if (i < 1) {
			i = numSkillLevels;
		} else if (i > numSkillLevels) {
      i = 1;
    }

    Cvar_Set("g_spSkill", va("%i", i));
    return true;
  }
  return false;
}

static bool UI_TeamName_HandleKey(int flags, float *special, int key, bool blue) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
    int i;
    i = UI_TeamIndexFromName(UI_Cvar_VariableString((blue) ? "ui_blueTeam" : "ui_redTeam"));

		if (key == K_MOUSE2) {
	    i--;
		} else {
	    i++;
		}

    if (i >= uiInfo.teamCount) {
      i = 0;
    } else if (i < 0) {
			i = uiInfo.teamCount - 1;
		}

    Cvar_Set( (blue) ? "ui_blueTeam" : "ui_redTeam", uiInfo.teamList[i].teamName);

    return true;
  }
  return false;
}

static bool UI_TeamMember_HandleKey(int flags, float *special, int key, bool blue, int num) 
{
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) 
  {
		// 0 - None
		// 1 - Human
		// 2..NumCharacters - Bot
		char *cvar = va(blue ? "ui_blueteam%i" : "ui_redteam%i", num);
		int value = Cvar_VariableValue(cvar);

		if (key == K_MOUSE2) 
		{
			value--;
		} 
		else 
		{
			value++;
		}

		if (ui_actualNetGameType.integer >= GT_TEAM) 
		{
			if (value >= uiInfo.characterCount + 2) 
			{
				value = 0;
			} 
			else if (value < 0) 
			{
				value = uiInfo.characterCount + 2 - 1;
			}
		} 
		//else 
		//{
		//	if (value >= UI_GetNumBots() + 2) 
		//	{
		//		value = 0;
		//	} 
		//	else if (value < 0) 
		//	{
		//		value = UI_GetNumBots() + 2 - 1;
		//	}
		//}

		Cvar_Set(cvar, va("%i", value));
		return true;
	}
	return false;
}

static bool UI_NetSource_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		
		if (key == K_MOUSE2) {
			ui_netSource.integer--;
		} else {
			ui_netSource.integer++;
		}
    
		if (ui_netSource.integer >= numNetSources) {
			ui_netSource.integer = 0;
		} else if (ui_netSource.integer < 0) {
			ui_netSource.integer = numNetSources - 1;
		}

		UI_BuildServerDisplayList(1);
		if (ui_netSource.integer != AS_GLOBAL) {
			UI_StartServerRefresh(true);
		}
  	Cvar_Set( "ui_netSource", va("%d", ui_netSource.integer));
    return true;
  }
  return false;
}

static bool UI_NetFilter_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {

		if (key == K_MOUSE2) {
			ui_serverFilterType.integer--;
		} else {
			ui_serverFilterType.integer++;
		}

    if (ui_serverFilterType.integer >= numServerFilters) {
      ui_serverFilterType.integer = 0;
    } else if (ui_serverFilterType.integer < 0) {
      ui_serverFilterType.integer = numServerFilters - 1;
		}
		UI_BuildServerDisplayList(1);
    return true;
  }
  return false;
}

static bool UI_OpponentName_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		if (key == K_MOUSE2) {
			UI_PriorOpponent();
		} else {
			UI_NextOpponent();
		}
    return true;
  }
  return false;
}

//static bool UI_BotName_HandleKey(int flags, float *special, int key) {
//  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
//		int game = Cvar_VariableValue("g_gametype");
//		int value = uiInfo.botIndex;
//
//		if (key == K_MOUSE2) {
//			value--;
//		} else {
//			value++;
//		}
//
//		if (game >= GT_TEAM) {
//			if (value >= uiInfo.characterCount + 2) {
//				value = 0;
//			} else if (value < 0) {
//				value = uiInfo.characterCount + 2 - 1;
//			}
//		} else {
//			if (value >= UI_GetNumBots() + 2) {
//				value = 0;
//			} else if (value < 0) {
//				value = UI_GetNumBots() + 2 - 1;
//			}
//		}
//		uiInfo.botIndex = value;
//    return true;
//  }
//  return false;
//}

static bool UI_BotSkill_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		if (key == K_MOUSE2) {
			uiInfo.skillIndex--;
		} else {
			uiInfo.skillIndex++;
		}
		if (uiInfo.skillIndex >= numSkillLevels) {
			uiInfo.skillIndex = 0;
		} else if (uiInfo.skillIndex < 0) {
			uiInfo.skillIndex = numSkillLevels-1;
		}
    return true;
  }
	return false;
}

static bool UI_RedBlue_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		uiInfo.redBlue ^= 1;
		return true;
	}
	return false;
}

static bool UI_Crosshair_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		if (key == K_MOUSE2) {
			uiInfo.currentCrosshair--;
		} else {
			uiInfo.currentCrosshair++;
		}

		if (uiInfo.currentCrosshair >= NUM_CROSSHAIRS) {
			uiInfo.currentCrosshair = 0;
		} else if (uiInfo.currentCrosshair < 0) {
			uiInfo.currentCrosshair = NUM_CROSSHAIRS - 1;
		}
		Cvar_Set("cg_drawCrosshair", va("%d", uiInfo.currentCrosshair)); 
		return true;
	}
	return false;
}



static bool UI_SelectedPlayer_HandleKey(int flags, float *special, int key) {
  if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_ENTER || key == K_KP_ENTER) {
		int selected;

		UI_BuildPlayerList();
		if (!uiInfo.teamLeader) {
			return false;
		}
		selected = Cvar_VariableValue("cg_selectedPlayer");
		
		if (key == K_MOUSE2) {
			selected--;
		} else {
			selected++;
		}

		if (selected > uiInfo.myTeamCount) {
			selected = 0;
		} else if (selected < 0) {
			selected = uiInfo.myTeamCount;
		}

		if (selected == uiInfo.myTeamCount) {
		 	Cvar_Set( "cg_selectedPlayerName", "Everyone");
		} else {
		 	Cvar_Set( "cg_selectedPlayerName", uiInfo.teamNames[selected]);
		}
	 	Cvar_Set( "cg_selectedPlayer", va("%d", selected));
	}
	return false;
}


bool UI_OwnerDrawHandleKey(int ownerDraw, int flags, float *special, int key) 
{
  switch (ownerDraw) {
    case UI_HANDICAP:
      return UI_Handicap_HandleKey(flags, special, key);
      break;
    case UI_EFFECTS:
      return UI_Effects_HandleKey(flags, special, key);
      break;
    case UI_CLANNAME:
      return UI_ClanName_HandleKey(flags, special, key);
      break;
    case UI_GAMETYPE:
      return UI_GameType_HandleKey(flags, special, key, true);
      break;
    case UI_NETGAMETYPE:
      return UI_NetGameType_HandleKey(flags, special, key);
      break;
    case UI_NETGAMESET:
      //return UI_NetGameset_HandleKey(flags, special, key);
      break;
    case UI_JOINGAMETYPE:
      return UI_JoinGameType_HandleKey(flags, special, key);
      break;
    case UI_SKILL:
      return UI_Skill_HandleKey(flags, special, key);
      break;
    case UI_BLUETEAMNAME:
      return UI_TeamName_HandleKey(flags, special, key, true);
      break;
    case UI_REDTEAMNAME:
      return UI_TeamName_HandleKey(flags, special, key, false);
      break;
    case UI_BLUETEAM1:
		case UI_BLUETEAM2:
		case UI_BLUETEAM3:
		case UI_BLUETEAM4:
		case UI_BLUETEAM5:
      UI_TeamMember_HandleKey(flags, special, key, true, ownerDraw - UI_BLUETEAM1 + 1);
      break;
    case UI_REDTEAM1:
		case UI_REDTEAM2:
		case UI_REDTEAM3:
		case UI_REDTEAM4:
		case UI_REDTEAM5:
      UI_TeamMember_HandleKey(flags, special, key, false, ownerDraw - UI_REDTEAM1 + 1);
      break;
		case UI_NETSOURCE:
      UI_NetSource_HandleKey(flags, special, key);
			break;
		case UI_NETFILTER:
      UI_NetFilter_HandleKey(flags, special, key);
			break;
		case UI_OPPONENT_NAME:
			UI_OpponentName_HandleKey(flags, special, key);
			break;
		//case UI_BOTNAME:
		//	return UI_BotName_HandleKey(flags, special, key);
		//	break;
		//case UI_BOTSKILL:
		//	return UI_BotSkill_HandleKey(flags, special, key);
		//	break;
		case UI_REDBLUE:
			UI_RedBlue_HandleKey(flags, special, key);
			break;
		case UI_CROSSHAIR:
			UI_Crosshair_HandleKey(flags, special, key);
			break;
		case UI_SELECTEDPLAYER:
			UI_SelectedPlayer_HandleKey(flags, special, key);
			break;
    default:
      break;
  }

  return false;
}


float UI_GetValue(int ownerDraw) 
{
  return 0;
}

/*
=================
UI_ServersQsortCompare
=================
*/
static int QDECL UI_ServersQsortCompare( const void *arg1, const void *arg2 ) 
{
	return LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey, uiInfo.serverStatus.sortDir, *(int*)arg1, *(int*)arg2);
}


/*
=================
UI_ServersSort
=================
*/
void UI_ServersSort(int column, bool force) {

	if ( !force ) {
		if ( uiInfo.serverStatus.sortKey == column ) {
			return;
		}
	}

	uiInfo.serverStatus.sortKey = column;
	qsort( &uiInfo.serverStatus.displayServers[0], uiInfo.serverStatus.numDisplayServers, sizeof(int), UI_ServersQsortCompare);
}

/*
static void UI_StartSinglePlayer() {
	int i,j, k, skill;
	char buff[1024];
	i = Cvar_VariableValue( "ui_currentTier" );
  if (i < 0 || i >= tierCount) {
    i = 0;
  }
	j = Cvar_VariableValue("ui_currentMap");
	if (j < 0 || j > MAPS_PER_TIER) {
		j = 0;
	}

 	Cvar_SetValue( "singleplayer", 1 );
 	Cvar_SetValue( "g_gametype", Com_Clamp( 0, 7, tierList[i].gameTypes[j] ) );
	Cbuf_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", tierList[i].maps[j] ) );
	skill = Cvar_VariableValue( "g_spSkill" );

	if (j == MAPS_PER_TIER-1) {
		k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
		Com_sprintf( buff, sizeof(buff), "wait ; addbot %s %i %s 250 %s\n", UI_AIFromName(teamList[k].teamMembers[0]), skill, "", teamList[k].teamMembers[0]);
	} else {
		k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));
		for (i = 0; i < PLAYERS_PER_TEAM; i++) {
			Com_sprintf( buff, sizeof(buff), "wait ; addbot %s %i %s 250 %s\n", UI_AIFromName(teamList[k].teamMembers[i]), skill, "Blue", teamList[k].teamMembers[i]);
			Cbuf_ExecuteText( EXEC_APPEND, buff );
		}

		k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
		for (i = 1; i < PLAYERS_PER_TEAM; i++) {
			Com_sprintf( buff, sizeof(buff), "wait ; addbot %s %i %s 250 %s\n", UI_AIFromName(teamList[k].teamMembers[i]), skill, "Red", teamList[k].teamMembers[i]);
			Cbuf_ExecuteText( EXEC_APPEND, buff );
		}
		Cbuf_ExecuteText( EXEC_APPEND, "wait 5; team Red\n" );
	}
	

}
*/

/*
===============
UI_LoadMods
===============
*/
static void UI_LoadMods() 
{
	int		numdirs;
	char	dirlist[2048];
	char	*dirptr;
  char  *descptr;
	int		i;
	int		dirlen;

	uiInfo.modCount = 0;
	numdirs = FS_GetFileList( "$modlist", "", dirlist, sizeof(dirlist) );
	dirptr  = dirlist;
	for( i = 0; i < numdirs; i++ ) 
	{
		dirlen = strlen( dirptr ) + 1;
		descptr = dirptr + dirlen;
		uiInfo.modList[uiInfo.modCount].modName = uiInfo.uiUtils.string_Alloc(dirptr);
		uiInfo.modList[uiInfo.modCount].modDescr = uiInfo.uiUtils.string_Alloc(descptr);
		dirptr += dirlen + strlen(descptr) + 1;
		uiInfo.modCount++;
		if (uiInfo.modCount >= MAX_MODS)
			break;
	}
}


/*
===============
UI_LoadTeams
===============
*/
static void UI_LoadTeams() {
	char	teamList[4096];
	char	*teamName;
	int		i, len, count;

	count = FS_GetFileList( "", "team", teamList, 4096 );

	if (count) {
		teamName = teamList;
		for ( i = 0; i < count; i++ ) {
			len = strlen( teamName );
			UI_ParseTeamInfo(teamName);
			teamName += len + 1;
		}
	}

}


/*
===============
UI_LoadMovies
===============
*/
static void UI_LoadMovies() 
{
	char	movielist[4096];
	char	*moviename;
	int		i, len;

	uiInfo.movieCount = FS_GetFileList( "video", "roq", movielist, 4096 );

	if (uiInfo.movieCount) 
	{
		if (uiInfo.movieCount > MAX_MOVIES) 
			uiInfo.movieCount = MAX_MOVIES;

		moviename = movielist;
		for ( i = 0; i < uiInfo.movieCount; i++ ) 
		{
			len = strlen( moviename );
			if (!Q_stricmp(moviename +  len - 4,".roq"))
				moviename[len-4] = '\0';

			Q_strupr(moviename);
			uiInfo.movieList[i] = uiInfo.uiUtils.string_Alloc(moviename);
			moviename += len + 1;
		}
	}

}



/*
===============
UI_LoadDemos
===============
*/
static void UI_LoadDemos() {
	char	demolist[4096];
	char demoExt[32];
	char	*demoname;
	int		i, len;

	Com_sprintf(demoExt, sizeof(demoExt), "mfq3_dm_%d", (int)Cvar_VariableValue("protocol"));

	uiInfo.demoCount = FS_GetFileList( "demos", demoExt, demolist, 4096 );

	Com_sprintf(demoExt, sizeof(demoExt), ".mfq3_dm_%d", (int)Cvar_VariableValue("protocol"));

	if (uiInfo.demoCount) {
		if (uiInfo.demoCount > MAX_DEMOS) {
			uiInfo.demoCount = MAX_DEMOS;
		}
		demoname = demolist;
		for ( i = 0; i < uiInfo.demoCount; i++ ) {
			len = strlen( demoname );
			if (!Q_stricmp(demoname +  len - strlen(demoExt), demoExt)) {
				demoname[len-strlen(demoExt)] = '\0';
			}
			Q_strupr(demoname);
			uiInfo.demoList[i] = uiInfo.uiUtils.string_Alloc(demoname);
			demoname += len + 1;
		}
	}

}


static bool UI_SetNextMap(int actual, int index) 
{
	int i;
	for (i = actual + 1; i < uiInfo.mapCount; i++) 
	{
		if (uiInfo.mapList[i].active) 
		{
			uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_MAPS, index + 1, "skirmish");
			return true;
		}
	}
	return false;
}


static void UI_StartSkirmish(bool next) 
{
	int i, k, g, delay, temp;
	float skill;
	char buff[MAX_STRING_CHARS];

	if (next) 
	{
		int actual;
		int index = Cvar_VariableValue("ui_mapIndex");
	 	UI_MapCountByGameType(true);
		UI_SelectedMap(index, &actual);
		if (UI_SetNextMap(actual, index)) 
		{
		} 
		else 
		{
			UI_GameType_HandleKey(0, 0, K_MOUSE1, false);
			UI_MapCountByGameType(true);
			uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_MAPS, 0, "skirmish");
		}
	}

	g = uiInfo.gameTypes[ui_gameType.integer].gtEnum;
	Cvar_SetValue( "g_gametype", g );
	Cbuf_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", uiInfo.mapList[ui_currentMap.integer].mapLoadName) );
	skill = Cvar_VariableValue( "g_spSkill" );
	Cvar_Set("ui_scoreMap", uiInfo.mapList[ui_currentMap.integer].mapName);

	k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_opponentName"));

	Cvar_Set("ui_singlePlayerActive", "1");

	// set up sp overrides, will be replaced on postgame
	temp = Cvar_VariableValue( "capturelimit" );
	Cvar_Set("ui_saveCaptureLimit", va("%i", temp));
	temp = Cvar_VariableValue( "fraglimit" );
	Cvar_Set("ui_saveFragLimit", va("%i", temp));

	UI_SetCapFragLimits(false);

	temp = Cvar_VariableValue( "cg_drawTimer" );
	Cvar_Set("ui_drawTimer", va("%i", temp));
	temp = Cvar_VariableValue( "g_doWarmup" );
	Cvar_Set("ui_doWarmup", va("%i", temp));
	temp = Cvar_VariableValue( "g_friendlyFire" );
	Cvar_Set("ui_friendlyFire", va("%i", temp));
	temp = Cvar_VariableValue( "sv_maxClients" );
	Cvar_Set("ui_maxClients", va("%i", temp));
	temp = Cvar_VariableValue( "g_warmup" );
	Cvar_Set("ui_Warmup", va("%i", temp));
	temp = Cvar_VariableValue( "sv_pure" );
	Cvar_Set("ui_pure", va("%i", temp));

	Cvar_Set("cg_cameraOrbit", "0");
//	Cvar_Set("cg_thirdPerson", "0"); - removed MFQ3
	Cvar_Set("cg_drawTimer", "1");
	Cvar_Set("g_doWarmup", "1");
	Cvar_Set("g_warmup", "15");
	Cvar_Set("sv_pure", "0");
	Cvar_Set("g_friendlyFire", "0");
	Cvar_Set("g_redTeam", UI_Cvar_VariableString("ui_teamName"));
	Cvar_Set("g_blueTeam", UI_Cvar_VariableString("ui_opponentName"));

	if (Cvar_VariableValue("ui_recordSPDemo")) {
		Com_sprintf(buff, MAX_STRING_CHARS, "%s_%i", uiInfo.mapList[ui_currentMap.integer].mapLoadName, g);
		Cvar_Set("ui_recordSPDemoName", buff);
	}

	delay = 500;

	if (g == GT_TOURNAMENT) {
		Cvar_Set("sv_maxClients", "2");
		Com_sprintf( buff, sizeof(buff), "wait ; addbot %s %f "", %i \n", uiInfo.mapList[ui_currentMap.integer].opponentName, skill, delay);
		Cbuf_ExecuteText( EXEC_APPEND, buff );
	} else {
		temp = uiInfo.mapList[ui_currentMap.integer].teamMembers * 2;
		Cvar_Set("sv_maxClients", va("%d", temp));
		for (i =0; i < uiInfo.mapList[ui_currentMap.integer].teamMembers; i++) {
			Com_sprintf( buff, sizeof(buff), "addbot %s %f %s %i %s\n", UI_AIFromName(uiInfo.teamList[k].teamMembers[i]), skill, (g == GT_FFA) ? "" : "Blue", delay, uiInfo.teamList[k].teamMembers[i]);
			Cbuf_ExecuteText( EXEC_APPEND, buff );
			delay += 500;
		}
		k = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
		for (i =0; i < uiInfo.mapList[ui_currentMap.integer].teamMembers-1; i++) {
			Com_sprintf( buff, sizeof(buff), "addbot %s %f %s %i %s\n", UI_AIFromName(uiInfo.teamList[k].teamMembers[i]), skill, (g == GT_FFA) ? "" : "Red", delay, uiInfo.teamList[k].teamMembers[i]);
			Cbuf_ExecuteText( EXEC_APPEND, buff );
			delay += 500;
		}
	}
	if (g >= GT_TEAM ) {
		Cbuf_ExecuteText( EXEC_APPEND, "wait 5; team Red\n" );
	}
}

static void UI_Update(const char *name) {
	int	val = Cvar_VariableValue(name);

 	if (Q_stricmp(name, "ui_SetName") == 0) {
		Cvar_Set( "name", UI_Cvar_VariableString("ui_Name"));
 	} else if (Q_stricmp(name, "ui_setRate") == 0) {
		float rate = Cvar_VariableValue("rate");
		if (rate >= 5000) {
			Cvar_Set("cl_maxpackets", "30");
			Cvar_Set("cl_packetdup", "1");
		} else if (rate >= 4000) {
			Cvar_Set("cl_maxpackets", "15");
			Cvar_Set("cl_packetdup", "2");		// favor less prediction errors when there's packet loss
		} else {
			Cvar_Set("cl_maxpackets", "15");
			Cvar_Set("cl_packetdup", "1");		// favor lower bandwidth
		}
 	} else if (Q_stricmp(name, "ui_GetName") == 0) {
		Cvar_Set( "ui_Name", UI_Cvar_VariableString("name"));
 	} else if (Q_stricmp(name, "r_colorbits") == 0) {
		switch (val) {
			case 0:
				Cvar_SetValue( "r_depthbits", 0 );
				Cvar_SetValue( "r_stencilbits", 0 );
			break;
			case 16:
				Cvar_SetValue( "r_depthbits", 16 );
				Cvar_SetValue( "r_stencilbits", 0 );
			break;
			case 32:
				Cvar_SetValue( "r_depthbits", 24 );
			break;
		}
	} else if (Q_stricmp(name, "r_lodbias") == 0) {
		switch (val) {
			case 0:
				Cvar_SetValue( "r_subdivisions", 4 );
			break;
			case 1:
				Cvar_SetValue( "r_subdivisions", 12 );
			break;
			case 2:
				Cvar_SetValue( "r_subdivisions", 20 );
			break;
		}
	} else if (Q_stricmp(name, "ui_glCustom") == 0) {
		switch (val) {
			case 0:	// high quality
				Cvar_SetValue( "r_fullScreen", 1 );
				Cvar_SetValue( "r_subdivisions", 4 );
				Cvar_SetValue( "r_vertexlight", 0 );
				Cvar_SetValue( "r_lodbias", 0 );
				Cvar_SetValue( "r_colorbits", 32 );
				Cvar_SetValue( "r_depthbits", 24 );
				Cvar_SetValue( "r_picmip", 0 );
				Cvar_SetValue( "r_mode", 4 );
				Cvar_SetValue( "r_texturebits", 32 );
				Cvar_SetValue( "r_fastSky", 0 );
				Cvar_SetValue( "r_inGameVideo", 1 );
				Cvar_SetValue( "cg_shadows", 1 );
				Cvar_SetValue( "cg_brassTime", 2500 );
				Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
			break;
			case 1: // normal 
				Cvar_SetValue( "r_fullScreen", 1 );
				Cvar_SetValue( "r_subdivisions", 12 );
				Cvar_SetValue( "r_vertexlight", 0 );
				Cvar_SetValue( "r_lodbias", 0 );
				Cvar_SetValue( "r_colorbits", 0 );
				Cvar_SetValue( "r_depthbits", 24 );
				Cvar_SetValue( "r_picmip", 1 );
				Cvar_SetValue( "r_mode", 3 );
				Cvar_SetValue( "r_texturebits", 0 );
				Cvar_SetValue( "r_fastSky", 0 );
				Cvar_SetValue( "r_inGameVideo", 1 );
				Cvar_SetValue( "cg_brassTime", 2500 );
				Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_LINEAR" );
				Cvar_SetValue( "cg_shadows", 0 );
			break;
			case 2: // fast
				Cvar_SetValue( "r_fullScreen", 1 );
				Cvar_SetValue( "r_subdivisions", 8 );
				Cvar_SetValue( "r_vertexlight", 0 );
				Cvar_SetValue( "r_lodbias", 1 );
				Cvar_SetValue( "r_colorbits", 0 );
				Cvar_SetValue( "r_depthbits", 0 );
				Cvar_SetValue( "r_picmip", 1 );
				Cvar_SetValue( "r_mode", 3 );
				Cvar_SetValue( "r_texturebits", 0 );
				Cvar_SetValue( "cg_shadows", 0 );
				Cvar_SetValue( "r_fastSky", 1 );
				Cvar_SetValue( "r_inGameVideo", 0 );
				Cvar_SetValue( "cg_brassTime", 0 );
				Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_NEAREST" );
			break;
			case 3: // fastest
				Cvar_SetValue( "r_fullScreen", 1 );
				Cvar_SetValue( "r_subdivisions", 20 );
				Cvar_SetValue( "r_vertexlight", 1 );
				Cvar_SetValue( "r_lodbias", 2 );
				Cvar_SetValue( "r_colorbits", 16 );
				Cvar_SetValue( "r_depthbits", 16 );
				Cvar_SetValue( "r_mode", 3 );
				Cvar_SetValue( "r_picmip", 2 );
				Cvar_SetValue( "r_texturebits", 16 );
				Cvar_SetValue( "cg_shadows", 0 );
				Cvar_SetValue( "cg_brassTime", 0 );
				Cvar_SetValue( "r_fastSky", 1 );
				Cvar_SetValue( "r_inGameVideo", 0 );
				Cvar_Set( "r_texturemode", "GL_LINEAR_MIPMAP_NEAREST" );
			break;
		}
	} else if (Q_stricmp(name, "ui_mousePitch") == 0) {
		if (val == 0) {
			Cvar_SetValue( "m_pitch", 0.022f );
		} else {
			Cvar_SetValue( "m_pitch", -0.022f );
		}
	}
}

/*
===============
UI_InitialiseUI
===============
*/

static void UI_InitialiseUI( void )
{
	fileHandle_t creditsFile;
	int fileSize = 0;

	// parse our credits text file on main menu run
	memset( uiInfo.pCredits, 0, sizeof( uiInfo.pCredits ) );
	uiInfo.creditLines = 0;
	uiInfo.pCreditsData = NULL;

	// open file
	fileSize = FS_FOpenFileByMode( "ui\\credits.txt", &creditsFile, FS_READ );	
	if( fileSize )
	{
//		int index = 0;
		char * pCredits = NULL;
		char * pLineEnd = NULL;

		// read in the entire credits file
		uiInfo.pCreditsData = uiInfo.uiUtils.alloc( fileSize );
		pCredits = (char *)uiInfo.pCreditsData;
		FS_Read2( pCredits, fileSize, creditsFile );
	
		// assign line pointers
		do
		{	
			// assign
			uiInfo.pCredits[ uiInfo.creditLines++ ] = pCredits;

			// find next line end
			pLineEnd = strstr( pCredits, "\n" );
			if( pLineEnd )
			{
				// convert to null terminator
				*pLineEnd++ = 0;
	
				// move work pointer
				pCredits = pLineEnd;
			}
		}
		while( pLineEnd && pCredits < ((char *)uiInfo.pCreditsData+fileSize) );

		// close file
		FS_FCloseFile( creditsFile );
	}


	// TODO: free uitInfo.pCreditsData
}

/*
===============
UI_RefreshVehicleSelect
===============
*/
static void UI_RefreshVehicleSelect( int change_vehicle )
{
	int vehicleCat = -1;
	int vehicleClass = -1;
	static int vehicleType = -1;
	int vehicle = -1;
	int gameset = -1;
	int gametype = -1;
	int team = -1;
	const char * pCat = NULL;
	const char * pClass = NULL;
	const char * pVehicle = NULL;
	int levelCats = 0;
	char info[MAX_INFO_STRING];
	bool allowNukes;

	// Get server info
	GetConfigString( CS_SERVERINFO, info, sizeof(info) );

	// Nukes
	allowNukes = (atoi( Info_ValueForKey( info, "mf_allowNukes" ) )!=0);

	// ----------> GAMESET & TEAM <-----------

	// get gameset & team (as MF_GAMESET_x and MF_TEAM_x)
	gameset = MF_UI_GetGameset( false );
	team = MF_UI_GetTeam();
	gametype = Cvar_VariableValue("g_gametype");
	// ----------> AVAILABLE VEHICLES <-----------

	switch( team )
	{
	default:
	case MF_TEAM_ANY:
		Cvar_Set( "ui_availableVehicleTxt", "ALL" );
		break;
	case MF_TEAM_1:
		Cvar_Set( "ui_availableVehicleTxt", "RED TEAM" );
		break;
	case MF_TEAM_2:
		Cvar_Set( "ui_availableVehicleTxt", "BLUE TEAM" );
		break;
	}

	// ----------> CATAGORY <-----------

	// find out the current UI vehicle (based upon the known catagory+class)
	vehicle = Cvar_VariableValue("ui_vehicle");

	// find out the current UI catagory
	vehicleCat = Cvar_VariableValue("ui_vehicleCat");

tryCatAgain:

	// is this catagory valid for the current gameset+team+catagory?

	pCat = cat_items[ vehicleCat ];	// get the text
	vehicle = MF_getIndexOfVehicleEx( (vehicle-1), gameset, team, vehicleCat, -1, -1, 0, allowNukes);

	// ----------> LEVEL SPAWNS <-----------

	// level check (don't allow selection of vehicles that can't spawn on the current level)
//	if(atoi(Info_ValueForKey( info, "sv_cheats" )) == 1)
//		levelCats = CAT_ANY;
	//else
		levelCats = atoi( Info_ValueForKey( info, "mf_lvcat" ) );

	if( pCat && vehicle >= 0 && (levelCats & (1 << vehicleCat)) ) 
	{
		// update the text in the dialog for the UI catagory
		Cvar_Set( "ui_vehicleCatTxt", pCat );
	} 
	else 
	{
		// the 'ui_vehicleCat' must be invalid try the next one
		vehicleCat++;
		
		// wrap?
		if( vehicleCat >= MF_MAX_CATEGORIES ) 
			vehicleCat = 0;

		goto tryCatAgain;
	}
	Cvar_Set( "ui_vehicleCat", va( "%d", vehicleCat ) );

	// ----------> CLASS <-----------

	// find out the current UI class (based upon the known catagory)
	vehicleClass = Cvar_VariableValue("ui_vehicleClass");

tryClassAgain:

	// get the text
	pClass = class_items[ vehicleCat ][ vehicleClass ];

	// check
	vehicle = MF_getIndexOfVehicleEx( (vehicle-1), gameset, team, vehicleCat, vehicleClass, -1, 0, allowNukes);
		
	if( pClass && vehicle >= 0 ) 
	{
		// update the text in the dialog for the UI class
		Cvar_Set( "ui_vehicleClassTxt", pClass );
	} 
	else
	{
		// the 'ui_vehicleCat' must be invalid try the next one
		vehicleClass++;

		// wrap?
		if( vehicleClass >= MF_MAX_CLASSES )
			vehicleClass = 0;

		goto tryClassAgain;
	}
	Cvar_Set( "ui_vehicleClass", va( "%d", vehicleClass ) );

	// ----------> VEHICLE <-----------
		
	// is the vehicle valid for the current gameset+team+catagory+class?

	// Use proper selector for vehicle/GI
	vehicle = MF_getIndexOfVehicleEx( (vehicle-1), gameset, team, vehicleCat, vehicleClass, vehicleType, change_vehicle, allowNukes);

	// -1 means this is not a suitable combination
	if( vehicle == -1 )
	{
		// prevent change & say so
		Cvar_Set( "ui_vehicle", "-1" );
		Cvar_Set( "ui_vehicleTxt", "<Nothing Available>" );
		Cvar_Set( "ui_vehicleLoadout", "-1" );
		Cvar_Set( "ui_vehicleLoadoutTxt", "<Nothing Available>" );
		return;
	}

	// get the text
	pVehicle = availableVehicles[ vehicle ].tinyName;
	
	if( pVehicle ) 
	{
		// update the text in the dialog for the UI vehicle
		Cvar_Set( "ui_vehicleTxt", pVehicle );
		// If it wasnt rejected in the first place its safe to update vehicleType
		vehicleType = vehicle;
	} 
	else 
	{
		// the 'ui_vehicle' must be invalid, reset
		Cvar_Set( "ui_vehicle", "0" );
		vehicle = 0;
	}

	// always set back the vehicle index
	Cvar_Set( "ui_vehicle", va( "%d", vehicle ));
	
	// get the text
	pVehicle = availableVehicles[ vehicle ].descriptiveName;

	if( pVehicle ) 
	{
		// update the text in the dialog for the UI vehicle
		Cvar_Set( "ui_vehicleLoadoutTxt", pVehicle );
	} 
	else 
	{
		// the 'ui_vehicleLoadout' must be invalid, reset
		Cvar_Set( "ui_vehicleLoadout", "0" );
		vehicle = 0;
	}
		
}

/*
===============
UI_CycleVehicleSelect
===============
*/
static void UI_CycleVehicleSelect( int cycleIdx )
{
	int idx = -1;
	int operation = 0;
	const char * pVar = NULL;

	// switch on what was asked to cycle
	switch( cycleIdx )
	{
	// Catagory
	case 0:
		pVar = "ui_vehicleCat";

		// reset dependent
		Cvar_Set( "ui_vehicleClass", "0" );

		break;

	// Class
	case 1:
		pVar = "ui_vehicleClass";
		break;

	// Vehicle
	case 2:
		// Nothing will be done here
		pVar = "ui_vehicle";
		operation = 1;
		break;
	// Loadout
	case 3:
		pVar = "ui_vehicle";
		operation = 2;
		break;
	default:
		return;
	}


	if(cycleIdx != 2) {
		// cycle numeric var
		idx = Cvar_VariableValue( pVar );
		idx++;
		Cvar_Set( pVar, va( "%d", idx ));
	}
	// update the dialog
	UI_RefreshVehicleSelect(operation);

}

/*
===============
UI_SelectVehicle
===============
*/
static void UI_SelectVehicle( void )
{
	int vehicleCat;
	int idx = -1;

	// get the UI vehicle index
	idx = Cvar_VariableValue( "ui_vehicle" );
	vehicleCat = Cvar_VariableValue("ui_vehicleCat");

	if( Cvar_VariableValue("g_gametype") == GT_MISSION_EDITOR ) {
		if(UI_CAT_GI == vehicleCat)
			Cbuf_ExecuteText( EXEC_APPEND, va( "me_spawngi %d\n", idx ) );
		else
			Cbuf_ExecuteText( EXEC_APPEND, va( "me_spawn %d\n", idx ) );
	} else {
		// set into the trigger index
		Cvar_Set( "cg_nextVehicle", va( "%d", idx ));	
	}
}

/*
===============
UI_RunMenuScript
===============
*/
void UI_RunMenuScript(char **args) 
{
	const char *name, *name2;
	char buff[1024];

	if (uiInfo.uiUtils.string_Parse(args, &name)) 
	{
		if (Q_stricmp(name, "StartServer") == 0) 
		{
			int i, clients, oldclients;
			float skill;
			// Cvar_Set("cg_thirdPerson", "0"); - removed MFQ3
			Cvar_Set("cg_cameraOrbit", "0");
			Cvar_Set("ui_singlePlayerActive", "0");
			Cvar_SetValue( "dedicated", Com_Clamp( 0, 2, ui_dedicated.integer ) );
			Cvar_SetValue( "g_gametype", Com_Clamp( 0, 8, uiInfo.gameTypes[ui_netGameType.integer].gtEnum ) );
			Cvar_Set("g_redTeam", UI_Cvar_VariableString("ui_teamName"));
			Cvar_Set("g_blueTeam", UI_Cvar_VariableString("ui_opponentName"));
			Cbuf_ExecuteText( EXEC_APPEND, va( "wait ; wait ; map %s\n", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName ) );
			skill = Cvar_VariableValue( "g_spSkill" );
			// set max clients based on spots
			oldclients = Cvar_VariableValue( "sv_maxClients" );
			clients = 0;
			for (i = 0; i < PLAYERS_PER_TEAM; i++) 
			{
				int bot = Cvar_VariableValue( va("ui_blueteam%i", i+1));
				if (bot >= 0) 
				{
					clients++;
				}
				bot = Cvar_VariableValue( va("ui_redteam%i", i+1));
				if (bot >= 0) 
				{
					clients++;
				}
			}
			if (clients == 0) 
			{
				clients = 8;
			}
			
			if (oldclients > clients) 
			{
				clients = oldclients;
			}

			Cvar_Set("sv_maxClients", va("%d",clients));
		} 
		else if (Q_stricmp(name, "updateSPMenu") == 0) 
		{
			UI_SetCapFragLimits(true);
			UI_MapCountByGameType(true);
			ui_mapIndex.integer = UI_GetIndexFromSelection(ui_currentMap.integer);
			Cvar_Set("ui_mapIndex", va("%d", ui_mapIndex.integer));
			uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_MAPS, ui_mapIndex.integer, "skirmish");
			UI_GameType_HandleKey(0, 0, K_MOUSE1, false);
			UI_GameType_HandleKey(0, 0, K_MOUSE2, false);
		} 
		else if (Q_stricmp(name, "resetDefaults") == 0) 
		{
			Cbuf_ExecuteText( EXEC_APPEND, "exec default.cfg\n");
			Cbuf_ExecuteText( EXEC_APPEND, "cvar_restart\n");
			uiInfo.uiUtils.getControlUtils()->setDefaults();
			Cvar_Set("com_introPlayed", "1" );
			Cbuf_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		} 
		else if (Q_stricmp(name, "getCDKey") == 0) 
		{
			//char out[17];
			//CLUI_GetCDKey(buff, 17);
			//Cvar_Set("cdkey1", "");
			//Cvar_Set("cdkey2", "");
			//Cvar_Set("cdkey3", "");
			//Cvar_Set("cdkey4", "");
			//if (strlen(buff) == CDKEY_LEN) {
			//	Q_strncpyz(out, buff, 5);
			//	Cvar_Set("cdkey1", out);
			//	Q_strncpyz(out, buff + 4, 5);
			//	Cvar_Set("cdkey2", out);
			//	Q_strncpyz(out, buff + 8, 5);
			//	Cvar_Set("cdkey3", out);
			//	Q_strncpyz(out, buff + 12, 5);
			//	Cvar_Set("cdkey4", out);
			//}

		} 
		else if (Q_stricmp(name, "verifyCDKey") == 0) 
		{
			//buff[0] = '\0';
			//Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey1")); 
			//Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey2")); 
			//Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey3")); 
			//Q_strcat(buff, 1024, UI_Cvar_VariableString("cdkey4")); 
			//Cvar_Set("cdkey", buff);
			//if (CL_CDKeyValidate(buff, UI_Cvar_VariableString("cdkeychecksum"))) 
			//{
			//	Cvar_Set("ui_cdkeyvalid", "CD Key Appears to be valid.");
			//	CLUI_SetCDKey(buff);
			//} 
			//else 
			//{
			//	Cvar_Set("ui_cdkeyvalid", "CD Key does not appear to be valid.");
			//}
		} 
		else if (Q_stricmp(name, "loadArenas") == 0) 
		{
			UI_LoadArenas();
			UI_MapCountByGameType(false);
			uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_ALLMAPS, 0, "createserver");
		} 
		else if (Q_stricmp(name, "saveControls") == 0) 
		{
			uiInfo.uiUtils.getControlUtils()->setConfig(true);
		}
		else if (Q_stricmp(name, "loadControls") == 0) 
		{
			uiInfo.uiUtils.getControlUtils()->getConfig();
		} 
		else if (Q_stricmp(name, "clearError") == 0) 
		{
			Cvar_Set("com_errorMessage", "");
		} 
		else if (Q_stricmp(name, "loadGameInfo") == 0)
		{
			UI_ParseGameInfo("gameinfo.txt");
			UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
		}
		else if (Q_stricmp(name, "resetScores") == 0) 
		{
			UI_ClearScores();
		}
		else if (Q_stricmp(name, "RefreshServers") == 0)
		{
			UI_StartServerRefresh(true);
			UI_BuildServerDisplayList(1);
		}
		else if (Q_stricmp(name, "RefreshFilter") == 0) 
		{
			UI_StartServerRefresh(false);
			UI_BuildServerDisplayList(1);
		} 
		else if (Q_stricmp(name, "RunSPDemo") == 0) 
		{
			if (uiInfo.demoAvailable) 
			{
			  Cbuf_ExecuteText( EXEC_APPEND, va("demo %s_%i\n", uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum));
			}
		}
		else if (Q_stricmp(name, "LoadDemos") == 0) 
		{
			UI_LoadDemos();
		} 
		else if (Q_stricmp(name, "LoadMovies") == 0)
		{
			UI_LoadMovies();
		} 
		else if (Q_stricmp(name, "LoadMods") == 0) 
		{
			UI_LoadMods();
		}
		else if (Q_stricmp(name, "playMovie") == 0) 
		{
			if (uiInfo.previewMovie >= 0) 
			{
			  CIN_StopCinematic(uiInfo.previewMovie);
			}
			Cbuf_ExecuteText( EXEC_APPEND, va("cinematic %s.roq 2\n", uiInfo.movieList[uiInfo.movieIndex]));
		} 
		else if (Q_stricmp(name, "RunDemo") == 0) 
		{
			Cbuf_ExecuteText( EXEC_APPEND, va("demo %s\n", uiInfo.demoList[uiInfo.demoIndex]));
		} 
		else if (Q_stricmp(name, "closeJoin") == 0) 
		{
			if (uiInfo.serverStatus.refreshActive) 
			{
				UI_StopServerRefresh();
				uiInfo.serverStatus.nextDisplayRefresh = 0;
				uiInfo.nextServerStatusRefresh = 0;
				uiInfo.nextFindPlayerRefresh = 0;
				UI_BuildServerDisplayList(1);
			} 
			else 
			{
				uiInfo.uiUtils.menu_CloseByName("joinserver");
				uiInfo.uiUtils.menu_OpenByName("main");
			}
		} 
		else if (Q_stricmp(name, "StopRefresh") == 0) 
		{
			UI_StopServerRefresh();
			uiInfo.serverStatus.nextDisplayRefresh = 0;
			uiInfo.nextServerStatusRefresh = 0;
			uiInfo.nextFindPlayerRefresh = 0;
		} 
		else if (Q_stricmp(name, "UpdateFilter") == 0)
		{
			if (ui_netSource.integer == AS_LOCAL) 
			{
				UI_StartServerRefresh(true);
			}
			UI_BuildServerDisplayList(1);
			UI_FeederSelection(FEEDER_SERVERS, 0);
		} 
		else if (Q_stricmp(name, "ServerStatus") == 0) 
		{
			LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], uiInfo.serverStatusAddress, sizeof(uiInfo.serverStatusAddress));
			UI_BuildServerStatus(true);
		} 
		else if (Q_stricmp(name, "FoundPlayerServerStatus") == 0)
		{
			Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
			UI_BuildServerStatus(true);
			uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
		} 
		else if (Q_stricmp(name, "FindPlayer") == 0)
		{
			UI_BuildFindPlayerList(true);
			// clear the displayed server status info
			uiInfo.serverStatusInfo.numLines = 0;
			uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_FINDPLAYER, 0, NULL);
		} 
		else if (Q_stricmp(name, "JoinServer") == 0) 
		{
			// Cvar_Set("cg_thirdPerson", "0"); - removed MFQ3
			Cvar_Set("cg_cameraOrbit", "0");
			Cvar_Set("ui_singlePlayerActive", "0");
			if (uiInfo.serverStatus.currentServer >= 0 && uiInfo.serverStatus.currentServer < uiInfo.serverStatus.numDisplayServers) {
				LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, 1024);
				Cbuf_ExecuteText( EXEC_APPEND, va( "connect %s\n", buff ) );
			}
		} 
		else if (Q_stricmp(name, "FoundPlayerJoinServer") == 0)
		{
			Cvar_Set("ui_singlePlayerActive", "0");
			if (uiInfo.currentFoundPlayerServer >= 0 && uiInfo.currentFoundPlayerServer < uiInfo.numFoundPlayerServers) {
				Cbuf_ExecuteText( EXEC_APPEND, va( "connect %s\n", uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer] ) );
			}
		} 
		else if (Q_stricmp(name, "Quit") == 0) 
		{
			Cvar_Set("ui_singlePlayerActive", "0");
			Cbuf_ExecuteText( EXEC_NOW, "quit");
		}
		else if (Q_stricmp(name, "Controls") == 0)
		{
		  Cvar_Set( "cl_paused", "1" );
			Key_SetCatcher( KEYCATCH_UI );
			uiInfo.uiUtils.menu_CloseAll();
			uiInfo.uiUtils.menu_ActivateByName("setup_menu2");
		} 
		else if (Q_stricmp(name, "Leave") == 0) 
		{
			Cbuf_ExecuteText( EXEC_APPEND, "disconnect\n" );
			Key_SetCatcher( KEYCATCH_UI );
			uiInfo.uiUtils.menu_CloseAll();
			uiInfo.uiUtils.menu_ActivateByName("main");
		} 
		else if (Q_stricmp(name, "ServerSort") == 0) 
		{
			int sortColumn;
			if (uiInfo.uiUtils.int_Parse(args, &sortColumn)) 
			{
				// if same column we're already sorting on then flip the direction
				if (sortColumn == uiInfo.serverStatus.sortKey)
				{
					uiInfo.serverStatus.sortDir = !uiInfo.serverStatus.sortDir;
				}
				// make sure we sort again
				UI_ServersSort(sortColumn, true);
			}
		} 
		else if (Q_stricmp(name, "nextSkirmish") == 0) 
		{
			UI_StartSkirmish(true);
		} 
		else if (Q_stricmp(name, "SkirmishStart") == 0) 
		{
			UI_StartSkirmish(false);
		} 
		else if (Q_stricmp(name, "closeingame") == 0) 
		{
			Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
			Key_ClearStates();
			Cvar_Set( "cl_paused", "0" );
			uiInfo.uiUtils.menu_CloseAll();
		} 
		else if (Q_stricmp(name, "voteMap") == 0) 
		{
			if (ui_currentNetMap.integer >=0 && ui_currentNetMap.integer < uiInfo.mapCount) 
			{
				Cbuf_ExecuteText( EXEC_APPEND, va("callvote map %s\n",uiInfo.mapList[ui_currentNetMap.integer].mapLoadName) );
			}
		}
		else if (Q_stricmp(name, "voteKick") == 0) 
		{
			if (uiInfo.playerIndex >= 0 && uiInfo.playerIndex < uiInfo.playerCount) 
			{
				Cbuf_ExecuteText( EXEC_APPEND, va("callvote kick %s\n",uiInfo.playerNames[uiInfo.playerIndex]) );
			}
		} 
		else if (Q_stricmp(name, "voteGame") == 0) 
		{
			if (ui_netGameType.integer >= 0 && ui_netGameType.integer < uiInfo.numGameTypes)
			{
				Cbuf_ExecuteText( EXEC_APPEND, va("callvote g_gametype %i\n",uiInfo.gameTypes[ui_netGameType.integer].gtEnum) );
			}
		//} else if (Q_stricmp(name, "voteSet") == 0) {	// TEMP DISABLED FOR NOW
//			if (ui_netGameset.integer >= 0 && ui_netGameset.integer < numGamesets) 
				//Cbuf_ExecuteText( EXEC_APPEND, va("callvote mf_gameset %s\n",gameset_codes[ui_netGameset.integer]) );
		} 
		else if (Q_stricmp(name, "voteLeader") == 0) 
		{
			if (uiInfo.teamIndex >= 0 && uiInfo.teamIndex < uiInfo.myTeamCount)
			{
				Cbuf_ExecuteText( EXEC_APPEND, va("callteamvote leader %s\n",uiInfo.teamNames[uiInfo.teamIndex]) );
			}
		} 
		else if (Q_stricmp(name, "addFavorite") == 0) 
		{
			if (ui_netSource.integer != AS_FAVORITES)
			{
				char name[MAX_NAME_LENGTH];
				char addr[MAX_NAME_LENGTH];
				int res;

				LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
				name[0] = addr[0] = '\0';
				Q_strncpyz(name, 	Info_ValueForKey(buff, "hostname"), MAX_NAME_LENGTH);
				Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
				if (strlen(name) > 0 && strlen(addr) > 0) 
				{
					res = LAN_AddServer(AS_FAVORITES, name, addr);
					if (res == 0)
					{
						// server already in the list
						Com_Printf("Favorite already in list\n");
					}
					else if (res == -1) 
					{
						// list full
						Com_Printf("Favorite list full\n");
					}
					else 
					{
						// successfully added
						Com_Printf("Added favorite server %s\n", addr);
					}
				}
			}
		} 
		else if (Q_stricmp(name, "deleteFavorite") == 0)
		{
			if (ui_netSource.integer == AS_FAVORITES)
			{
				char addr[MAX_NAME_LENGTH];
				LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.serverStatus.currentServer], buff, MAX_STRING_CHARS);
				addr[0] = '\0';
				Q_strncpyz(addr, 	Info_ValueForKey(buff, "addr"), MAX_NAME_LENGTH);
				if (strlen(addr) > 0) 
				{
					LAN_RemoveServer(AS_FAVORITES, addr);
				}
			}
		} 
		else if (Q_stricmp(name, "createFavorite") == 0)
		{
			if (ui_netSource.integer == AS_FAVORITES)
			{
				char name[MAX_NAME_LENGTH];
				char addr[MAX_NAME_LENGTH];
				int res;

				name[0] = addr[0] = '\0';
				Q_strncpyz(name, 	UI_Cvar_VariableString("ui_favoriteName"), MAX_NAME_LENGTH);
				Q_strncpyz(addr, 	UI_Cvar_VariableString("ui_favoriteAddress"), MAX_NAME_LENGTH);
				if (strlen(name) > 0 && strlen(addr) > 0) 
				{
					res = LAN_AddServer(AS_FAVORITES, name, addr);
					if (res == 0)
					{
						// server already in the list
						Com_Printf("Favorite already in list\n");
					}
					else if (res == -1)
					{
						// list full
						Com_Printf("Favorite list full\n");
					}
					else 
					{
						// successfully added
						Com_Printf("Added favorite server %s\n", addr);
					}
				}
			}
		} 
		else if (Q_stricmp(name, "orders") == 0)
		{
			const char *orders;
			if (uiInfo.uiUtils.string_Parse(args, &orders)) 
			{
				int selectedPlayer = Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer < uiInfo.myTeamCount) 
				{
					strcpy(buff, orders);
					Cbuf_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]) );
					Cbuf_ExecuteText( EXEC_APPEND, "\n" );
				} 
				else 
				{
					int i;
					for (i = 0; i < uiInfo.myTeamCount; i++)
					{
						if (Q_stricmp(UI_Cvar_VariableString("name"), uiInfo.teamNames[i]) == 0) 
						{
							continue;
						}
						strcpy(buff, orders);
						Cbuf_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamNames[i]) );
						Cbuf_ExecuteText( EXEC_APPEND, "\n" );
					}
				}
				Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
				Key_ClearStates();
				Cvar_Set( "cl_paused", "0" );
				uiInfo.uiUtils.menu_CloseAll();
			}
		} 
		else if (Q_stricmp(name, "voiceOrdersTeam") == 0)
		{
			const char *orders;
			if (uiInfo.uiUtils.string_Parse(args, &orders)) 
			{
				int selectedPlayer = Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer == uiInfo.myTeamCount) {
					Cbuf_ExecuteText( EXEC_APPEND, orders );
					Cbuf_ExecuteText( EXEC_APPEND, "\n" );
				}
				Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
				Key_ClearStates();
				Cvar_Set( "cl_paused", "0" );
				uiInfo.uiUtils.menu_CloseAll();
			}
		} 
		else if (Q_stricmp(name, "voiceOrders") == 0)
		{
			const char *orders;
			if (uiInfo.uiUtils.string_Parse(args, &orders))
			{
				int selectedPlayer = Cvar_VariableValue("cg_selectedPlayer");
				if (selectedPlayer < uiInfo.myTeamCount)
				{
					strcpy(buff, orders);
					Cbuf_ExecuteText( EXEC_APPEND, va(buff, uiInfo.teamClientNums[selectedPlayer]) );
					Cbuf_ExecuteText( EXEC_APPEND, "\n" );
				}
				Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
				Key_ClearStates();
				Cvar_Set( "cl_paused", "0" );
				uiInfo.uiUtils.menu_CloseAll();
			}
		} 
		else if (Q_stricmp(name, "glCustom") == 0) 
		{
			Cvar_Set("ui_glCustom", "4");
		} 
		else if (Q_stricmp(name, "update") == 0) 
		{
			if (uiInfo.uiUtils.string_Parse(args, &name2))
			{
				UI_Update(name2);
			}
		}
		else if( Q_stricmp(name, "selectVehicle") == 0 )
		{
			// MFQ3: select a vehicle
			UI_SelectVehicle();
		}
		else if( Q_stricmp(name, "refreshVehicleSelect") == 0 )
		{
			// MFQ3: refresh the vehicle select dialog
			UI_RefreshVehicleSelect(0);
		}
		else if( Q_stricmp(name, "cycleVehicleSelect") == 0 )
		{
			int cycleIdx = -1;

			// MFQ3: cycle through catagory/class/vehicle
			if( uiInfo.uiUtils.int_Parse(args, &cycleIdx) )
			{
				 UI_CycleVehicleSelect ( cycleIdx );
			}
		}
		else if( Q_stricmp(name, "normalCursor") == 0 )
		{
			// reset cursor
			uiInfo.uiUtils.dc_->cursorEnum_ = CURSOR_NORMAL;
		}
		else if( Q_stricmp(name, "uiInitialise") == 0 )
		{
			// UI: initialise anything on main menu open
			UI_InitialiseUI();
		}
		else
		{
			Com_Printf("unknown UI script %s\n", name);
		}
	}
}

void UI_GetTeamColor(vec4_t *color) 
{
}

/*
==================
UI_MapCountByGameType
==================
*/
static int UI_MapCountByGameType(bool singlePlayer) {
	int i, c, game;
	c = 0;
	game = singlePlayer ? uiInfo.gameTypes[ui_gameType.integer].gtEnum : uiInfo.gameTypes[ui_netGameType.integer].gtEnum;
	if (game == GT_SINGLE_PLAYER) {
		game++;
	} 
	if (game == GT_TEAM) {
		game = GT_FFA;
	}

	for (i = 0; i < uiInfo.mapCount; i++) {
		uiInfo.mapList[i].active = false;
		if ( uiInfo.mapList[i].typeBits & (1 << game)) {
			if (singlePlayer) {
				if (!(uiInfo.mapList[i].typeBits & (1 << GT_SINGLE_PLAYER))) {
					continue;
				}
			}
			c++;
			uiInfo.mapList[i].active = true;
		}
	}
	return c;
}

bool UI_hasSkinForBase(const char *base, const char *team) {
	char	test[1024];
	
	Com_sprintf( test, sizeof( test ), "models/players/%s/%s/lower_default.skin", base, team );

	if (FS_FOpenFileByMode(test, 0, FS_READ)) {
		return true;
	}
	Com_sprintf( test, sizeof( test ), "models/players/characters/%s/%s/lower_default.skin", base, team );

	if (FS_FOpenFileByMode(test, 0, FS_READ)) {
		return true;
	}
	return false;
}

/*
==================
UI_MapCountByTeam
==================
*/
static int UI_HeadCountByTeam() {
	static int init = 0;
	int i, j, k, c, tIndex;
	
	c = 0;
	if (!init) {
		for (i = 0; i < uiInfo.characterCount; i++) {
			uiInfo.characterList[i].reference = 0;
			for (j = 0; j < uiInfo.teamCount; j++) {
			  if (UI_hasSkinForBase(uiInfo.characterList[i].base, uiInfo.teamList[j].teamName)) {
					uiInfo.characterList[i].reference |= (1<<j);
			  }
			}
		}
		init = 1;
	}

	tIndex = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));

	// do names
	for (i = 0; i < uiInfo.characterCount; i++) {
		uiInfo.characterList[i].active = false;
		for(j = 0; j < TEAM_MEMBERS; j++) {
			if (uiInfo.teamList[tIndex].teamMembers[j] != NULL) {
				if (uiInfo.characterList[i].reference&(1<<tIndex)) {// && Q_stricmp(uiInfo.teamList[tIndex].teamMembers[j], uiInfo.characterList[i].name)==0) {
					uiInfo.characterList[i].active = true;
					c++;
					break;
				}
			}
		}
	}

	// and then aliases
	for(j = 0; j < TEAM_MEMBERS; j++) {
		for(k = 0; k < uiInfo.aliasCount; k++) {
			if (uiInfo.aliasList[k].name != NULL) {
				if (Q_stricmp(uiInfo.teamList[tIndex].teamMembers[j], uiInfo.aliasList[k].name)==0) {
					for (i = 0; i < uiInfo.characterCount; i++) {
						if (uiInfo.characterList[i].headImage != -1 && uiInfo.characterList[i].reference&(1<<tIndex) && Q_stricmp(uiInfo.aliasList[k].ai, uiInfo.characterList[i].name)==0) {
							if (uiInfo.characterList[i].active == false) {
								uiInfo.characterList[i].active = true;
								c++;
							}
							break;
						}
					}
				}
			}
		}
	}
	return c;
}

/*
==================
UI_InsertServerIntoDisplayList
==================
*/
static void UI_InsertServerIntoDisplayList(int num, int position) {
	int i;

	if (position < 0 || position > uiInfo.serverStatus.numDisplayServers ) {
		return;
	}
	//
	uiInfo.serverStatus.numDisplayServers++;
	for (i = uiInfo.serverStatus.numDisplayServers; i > position; i--) {
		uiInfo.serverStatus.displayServers[i] = uiInfo.serverStatus.displayServers[i-1];
	}
	uiInfo.serverStatus.displayServers[position] = num;
}

/*
==================
UI_RemoveServerFromDisplayList
==================
*/
static void UI_RemoveServerFromDisplayList(int num) {
	int i, j;

	for (i = 0; i < uiInfo.serverStatus.numDisplayServers; i++) {
		if (uiInfo.serverStatus.displayServers[i] == num) {
			uiInfo.serverStatus.numDisplayServers--;
			for (j = i; j < uiInfo.serverStatus.numDisplayServers; j++) {
				uiInfo.serverStatus.displayServers[j] = uiInfo.serverStatus.displayServers[j+1];
			}
			return;
		}
	}
}

/*
==================
UI_BinaryServerInsertion
==================
*/
static void UI_BinaryServerInsertion(int num) {
	int mid, offset, res, len;

	// use binary search to insert server
	len = uiInfo.serverStatus.numDisplayServers;
	mid = len;
	offset = 0;
	res = 0;
	while(mid > 0) {
		mid = len >> 1;
		//
		res = LAN_CompareServers( ui_netSource.integer, uiInfo.serverStatus.sortKey,
					uiInfo.serverStatus.sortDir, num, uiInfo.serverStatus.displayServers[offset+mid]);
		// if equal
		if (res == 0) {
			UI_InsertServerIntoDisplayList(num, offset+mid);
			return;
		}
		// if larger
		else if (res == 1) {
			offset += mid;
			len -= mid;
		}
		// if smaller
		else {
			len -= mid;
		}
	}
	if (res == 1) {
		offset++;
	}
	UI_InsertServerIntoDisplayList(num, offset);
}

/*
==================
UI_BuildServerDisplayList
==================
*/
static void UI_BuildServerDisplayList(int force) 
{
	int i, count, clients, maxClients, ping, game, len, visible;
	char info[MAX_STRING_CHARS];
//	bool startRefresh = true; TTimo: unused
	static int numinvisible;

	if (!(force || uiInfo.uiUtils.dc_->realTime_ > uiInfo.serverStatus.nextDisplayRefresh)) 
		return;

	// if we shouldn't reset
	if ( force == 2 ) 
		force = 0;

	// do motd updates here too
	Cvar_VariableStringBuffer( "cl_motdString", uiInfo.serverStatus.motd, sizeof(uiInfo.serverStatus.motd) );
	len = strlen(uiInfo.serverStatus.motd);
	if (len == 0) 
	{
		strcpy(uiInfo.serverStatus.motd, "Welcome to Team Arena!");
		len = strlen(uiInfo.serverStatus.motd);
	} 
	if (len != uiInfo.serverStatus.motdLen) 
	{
		uiInfo.serverStatus.motdLen = len;
		uiInfo.serverStatus.motdWidth = -1;
	} 

	if (force) 
	{
		numinvisible = 0;
		// clear number of displayed servers
		uiInfo.serverStatus.numDisplayServers = 0;
		uiInfo.serverStatus.numPlayersOnServers = 0;
		// set list box index to zero
		uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_SERVERS, 0, NULL);
		// mark all servers as visible so we store ping updates for them
		LAN_MarkServerVisible(ui_netSource.integer, -1, true);
	}

	// get the server count (comes from the master)
	count = LAN_GetServerCount(ui_netSource.integer);
	if (count == -1 || (ui_netSource.integer == AS_LOCAL && count == 0) ) 
	{
		// still waiting on a response from the master
		uiInfo.serverStatus.numDisplayServers = 0;
		uiInfo.serverStatus.numPlayersOnServers = 0;
		uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiUtils.dc_->realTime_ + 500;
		return;
	}

	visible = false;
	for (i = 0; i < count; i++) 
	{
		// if we already got info for this server
		if (!LAN_ServerIsVisible(ui_netSource.integer, i)) 
			continue;

		visible = true;
		// get the ping for this server
		ping = LAN_GetServerPing(ui_netSource.integer, i);
		if (ping > 0 || ui_netSource.integer == AS_FAVORITES) 
		{

			LAN_GetServerInfo(ui_netSource.integer, i, info, MAX_STRING_CHARS);

			// mask out all but "Military Forces" servers
			if( Q_stricmp( Info_ValueForKey( info, "game" ), GAME_IDENTIFIER ) != 0 )
				continue;

			clients = atoi(Info_ValueForKey(info, "clients"));
			uiInfo.serverStatus.numPlayersOnServers += clients;

			if (ui_browserShowEmpty.integer == 0) 
			{
				if (clients == 0) 
				{
					LAN_MarkServerVisible(ui_netSource.integer, i, false);
					continue;
				}
			}

			if (ui_browserShowFull.integer == 0) 
			{
				maxClients = atoi(Info_ValueForKey(info, "sv_maxclients"));
				if (clients == maxClients) 
				{
					LAN_MarkServerVisible(ui_netSource.integer, i, false);
					continue;
				}
			}

			if (uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum != -1) 
			{
				game = atoi(Info_ValueForKey(info, "gametype"));
				if (game != uiInfo.joinGameTypes[ui_joinGameType.integer].gtEnum) 
				{
					LAN_MarkServerVisible(ui_netSource.integer, i, false);
					continue;
				}
			}
				
			if (ui_serverFilterType.integer > 0) 
			{
				if (Q_stricmp(Info_ValueForKey(info, "game"), serverFilters[ui_serverFilterType.integer].basedir) != 0) 
				{
					LAN_MarkServerVisible(ui_netSource.integer, i, false);
					continue;
				}
			}
			// make sure we never add a favorite server twice
			if (ui_netSource.integer == AS_FAVORITES) 
				UI_RemoveServerFromDisplayList(i);

			// insert the server into the list
			UI_BinaryServerInsertion(i);
			// done with this server
			if (ping > 0) 
			{
				LAN_MarkServerVisible(ui_netSource.integer, i, false);
				numinvisible++;
			}
		}
	}

	uiInfo.serverStatus.refreshtime = uiInfo.uiUtils.dc_->realTime_;

	// if there were no servers visible for ping updates
	//if (!visible) {
//		UI_StopServerRefresh();
//		uiInfo.serverStatus.nextDisplayRefresh = 0;
	//}
}

typedef struct
{
	char *name, *altName;
} serverStatusCvar_t;

serverStatusCvar_t serverStatusCvars[] = {
	{"sv_hostname", "Name"},
	{"Address", ""},
	{"gamename", "Game Name"},
	{"g_gametype", "Game Type"},
	{"mf_gameset", "MFQ3 Game Set"},
	{"mf_version", "MFQ3 Version"},
	{"mapname", "Map"},
	{"version", "Version"},
	{"protocol", "Protocol"},
	{"timelimit", "Time Limit"},
	{"fraglimit", "Frag Limit"},
	{"capturelimit", "Capture Limit"},
	{NULL, NULL}
};

/*
==================
UI_SortServerStatusInfo
==================
*/
static void UI_SortServerStatusInfo( serverStatusInfo_t *info ) {
	int i, j, index;
	char *tmp1, *tmp2;

	index = 0;
	for (i = 0; serverStatusCvars[i].name; i++) {
		for (j = 0; j < info->numLines; j++) {
			if ( !info->lines[j][1] || info->lines[j][1][0] ) {
				continue;
			}
			if ( !Q_stricmp(serverStatusCvars[i].name, info->lines[j][0]) ) {
				// swap lines
				tmp1 = info->lines[index][0];
				tmp2 = info->lines[index][3];
				info->lines[index][0] = info->lines[j][0];
				info->lines[index][3] = info->lines[j][3];
				info->lines[j][0] = tmp1;
				info->lines[j][3] = tmp2;
				//
				if ( strlen(serverStatusCvars[i].altName) ) {
					info->lines[index][0] = serverStatusCvars[i].altName;
				}
				index++;
			}
		}
	}
}

/*
==================
UI_GetServerStatusInfo
==================
*/
static int UI_GetServerStatusInfo( const char *serverAddress, serverStatusInfo_t *info ) {
	char *p, *score, *ping, *name;
	int i, len;

	if (!info) {
		LAN_GetServerStatus( serverAddress, NULL, 0);
		return false;
	}
	memset(info, 0, sizeof(*info));
	if ( LAN_GetServerStatus( serverAddress, info->text, sizeof(info->text)) ) {
		Q_strncpyz(info->address, serverAddress, sizeof(info->address));
		p = info->text;
		info->numLines = 0;
		info->lines[info->numLines][0] = "Address";
		info->lines[info->numLines][1] = "";
		info->lines[info->numLines][2] = "";
		info->lines[info->numLines][3] = info->address;
		info->numLines++;
		// get the cvars
		while (p && *p) {
			p = strchr(p, '\\');
			if (!p) break;
			*p++ = '\0';
			if (*p == '\\')
				break;
			info->lines[info->numLines][0] = p;
			info->lines[info->numLines][1] = "";
			info->lines[info->numLines][2] = "";
			p = strchr(p, '\\');
			if (!p) break;
			*p++ = '\0';
			info->lines[info->numLines][3] = p;

			info->numLines++;
			if (info->numLines >= MAX_SERVERSTATUS_LINES)
				break;
		}
		// get the player list
		if (info->numLines < MAX_SERVERSTATUS_LINES-3) {
			// empty line
			info->lines[info->numLines][0] = "";
			info->lines[info->numLines][1] = "";
			info->lines[info->numLines][2] = "";
			info->lines[info->numLines][3] = "";
			info->numLines++;
			// header
			info->lines[info->numLines][0] = "num";
			info->lines[info->numLines][1] = "score";
			info->lines[info->numLines][2] = "ping";
			info->lines[info->numLines][3] = "name";
			info->numLines++;
			// parse players
			i = 0;
			len = 0;
			while (p && *p) {
				if (*p == '\\')
					*p++ = '\0';
				if (!p)
					break;
				score = p;
				p = strchr(p, ' ');
				if (!p)
					break;
				*p++ = '\0';
				ping = p;
				p = strchr(p, ' ');
				if (!p)
					break;
				*p++ = '\0';
				name = p;
				Com_sprintf(&info->pings[len], sizeof(info->pings)-len, "%d", i);
				info->lines[info->numLines][0] = &info->pings[len];
				len += strlen(&info->pings[len]) + 1;
				info->lines[info->numLines][1] = score;
				info->lines[info->numLines][2] = ping;
				info->lines[info->numLines][3] = name;
				info->numLines++;
				if (info->numLines >= MAX_SERVERSTATUS_LINES)
					break;
				p = strchr(p, '\\');
				if (!p)
					break;
				*p++ = '\0';
				//
				i++;
			}
		}
		UI_SortServerStatusInfo( info );
		return true;
	}
	return false;
}

/*
==================
stristr
==================
*/
static char *stristr(char *str, char *charset) {
	int i;

	while(*str) {
		for (i = 0; charset[i] && str[i]; i++) {
			if (toupper(charset[i]) != toupper(str[i])) break;
		}
		if (!charset[i]) return str;
		str++;
	}
	return NULL;
}

/*
==================
UI_BuildFindPlayerList
==================
*/
static void UI_BuildFindPlayerList(bool force) {
	static int numFound, numTimeOuts;
	int i, j, resend;
	serverStatusInfo_t info;
	char name[MAX_NAME_LENGTH+2];
	char infoString[MAX_STRING_CHARS];

	if (!force) {
		if (!uiInfo.nextFindPlayerRefresh || uiInfo.nextFindPlayerRefresh > uiInfo.uiUtils.dc_->realTime_) {
			return;
		}
	}
	else {
		memset(&uiInfo.pendingServerStatus, 0, sizeof(uiInfo.pendingServerStatus));
		uiInfo.numFoundPlayerServers = 0;
		uiInfo.currentFoundPlayerServer = 0;
		Cvar_VariableStringBuffer( "ui_findPlayer", uiInfo.findPlayerName, sizeof(uiInfo.findPlayerName));
		Q_CleanStr(uiInfo.findPlayerName);
		// should have a string of some length
		if (!strlen(uiInfo.findPlayerName)) {
			uiInfo.nextFindPlayerRefresh = 0;
			return;
		}
		// set resend time
		resend = ui_serverStatusTimeOut.integer / 2 - 10;
		if (resend < 50) {
			resend = 50;
		}
		Cvar_Set("cl_serverStatusResendTime", va("%d", resend));
		// reset all server status requests
		LAN_GetServerStatus( NULL, NULL, 0);
		//
		uiInfo.numFoundPlayerServers = 1;
		Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
						sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
							"searching %d...", uiInfo.pendingServerStatus.num);
		numFound = 0;
		numTimeOuts++;
	}
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		// if this pending server is valid
		if (uiInfo.pendingServerStatus.server[i].valid) {
			// try to get the server status for this server
			if (UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, &info ) ) {
				//
				numFound++;
				// parse through the server status lines
				for (j = 0; j < info.numLines; j++) {
					// should have ping info
					if ( !info.lines[j][2] || !info.lines[j][2][0] ) {
						continue;
					}
					// clean string first
					Q_strncpyz(name, info.lines[j][3], sizeof(name));
					Q_CleanStr(name);
					// if the player name is a substring
					if (stristr(name, uiInfo.findPlayerName)) {
						// add to found server list if we have space (always leave space for a line with the number found)
						if (uiInfo.numFoundPlayerServers < MAX_FOUNDPLAYER_SERVERS-1) {
							//
							Q_strncpyz(uiInfo.foundPlayerServerAddresses[uiInfo.numFoundPlayerServers-1],
										uiInfo.pendingServerStatus.server[i].adrstr,
											sizeof(uiInfo.foundPlayerServerAddresses[0]));
							Q_strncpyz(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
										uiInfo.pendingServerStatus.server[i].name,
											sizeof(uiInfo.foundPlayerServerNames[0]));
							uiInfo.numFoundPlayerServers++;
						}
						else {
							// can't add any more so we're done
							uiInfo.pendingServerStatus.num = uiInfo.serverStatus.numDisplayServers;
						}
					}
				}
				Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
								sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
									"searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
				// retrieved the server status so reuse this spot
				uiInfo.pendingServerStatus.server[i].valid = false;
			}
		}
		// if empty pending slot or timed out
		if (!uiInfo.pendingServerStatus.server[i].valid ||
			uiInfo.pendingServerStatus.server[i].startTime < uiInfo.uiUtils.dc_->realTime_ - ui_serverStatusTimeOut.integer) {
			if (uiInfo.pendingServerStatus.server[i].valid) {
				numTimeOuts++;
			}
			// reset server status request for this address
			UI_GetServerStatusInfo( uiInfo.pendingServerStatus.server[i].adrstr, NULL );
			// reuse pending slot
			uiInfo.pendingServerStatus.server[i].valid = false;
			// if we didn't try to get the status of all servers in the main browser yet
			if (uiInfo.pendingServerStatus.num < uiInfo.serverStatus.numDisplayServers) {
				uiInfo.pendingServerStatus.server[i].startTime = uiInfo.uiUtils.dc_->realTime_;
				LAN_GetServerAddressString(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num],
							uiInfo.pendingServerStatus.server[i].adrstr, sizeof(uiInfo.pendingServerStatus.server[i].adrstr));
				LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[uiInfo.pendingServerStatus.num], infoString, sizeof(infoString));
				Q_strncpyz(uiInfo.pendingServerStatus.server[i].name, Info_ValueForKey(infoString, "hostname"), sizeof(uiInfo.pendingServerStatus.server[0].name));
				uiInfo.pendingServerStatus.server[i].valid = true;
				uiInfo.pendingServerStatus.num++;
				Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1],
								sizeof(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1]),
									"searching %d/%d...", uiInfo.pendingServerStatus.num, numFound);
			}
		}
	}
	for (i = 0; i < MAX_SERVERSTATUSREQUESTS; i++) {
		if (uiInfo.pendingServerStatus.server[i].valid) {
			break;
		}
	}
	// if still trying to retrieve server status info
	if (i < MAX_SERVERSTATUSREQUESTS) {
		uiInfo.nextFindPlayerRefresh = uiInfo.uiUtils.dc_->realTime_ + 25;
	}
	else {
		// add a line that shows the number of servers found
		if (!uiInfo.numFoundPlayerServers) {
			Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1], sizeof(uiInfo.foundPlayerServerAddresses[0]), "no servers found");
		}
		else {
			Com_sprintf(uiInfo.foundPlayerServerNames[uiInfo.numFoundPlayerServers-1], sizeof(uiInfo.foundPlayerServerAddresses[0]),
						"%d server%s found with player %s", uiInfo.numFoundPlayerServers-1,
						uiInfo.numFoundPlayerServers == 2 ? "":"s", uiInfo.findPlayerName);
		}
		uiInfo.nextFindPlayerRefresh = 0;
		// show the server status info for the selected server
		UI_FeederSelection(FEEDER_FINDPLAYER, uiInfo.currentFoundPlayerServer);
	}
}

/*
==================
UI_BuildServerStatus
==================
*/
static void UI_BuildServerStatus(bool force)
{
	uiClientState_t	cstate;	// server info structure

	static char * pAddressName = NULL;	// server address
	static bool pending = false;	// pending flag

	if( uiInfo.nextFindPlayerRefresh )
	{
		return;
	}
	
	// not forced?
	if( !force ) 
	{
		// still waiting?
		if( !uiInfo.nextServerStatusRefresh || uiInfo.nextServerStatusRefresh > uiInfo.uiUtils.dc_->realTime_ )
		{
			return;
		}
	}
	else
	{
		// set feeder
		uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
		uiInfo.serverStatusInfo.numLines = 0;
		
		// reset all server status requests
		LAN_GetServerStatus( NULL, NULL, 0);
	}

	// normal case
	if( !pending )
	{
		pAddressName = uiInfo.serverStatusAddress;
	}

	// NOT already a request pended?
	if( !pending )
	{
		// if the variable cl_paused == 1, then we assume that the feeder is for the
		// current server (that the client is connected to) and the request is of in-game origin
		if( Cvar_VariableValue( "cl_paused" ) == 1 )
		{
			// special case: in game popup

			// change to the wait cursor
			uiInfo.uiUtils.dc_->cursorEnum_ = CURSOR_WAIT;
			UI_DrawMouse();

			// get client information
			GetClientState( &cstate );

			// use the known server address
			pAddressName = cstate.servername;

			// do refresh in 50ms
			uiInfo.nextServerStatusRefresh = uiInfo.uiUtils.dc_->realTime_ + 50;

			// pend it (don't do it immediatley)
			pending = true;

			return;
		}
		else
		{
			// normal case: front end UI
			if (uiInfo.serverStatus.currentServer < 0 || uiInfo.serverStatus.currentServer > uiInfo.serverStatus.numDisplayServers || uiInfo.serverStatus.numDisplayServers == 0)
			{
				return;
			}
		}
	}

	// get information
	if( UI_GetServerStatusInfo( pAddressName, &uiInfo.serverStatusInfo ) )
	{
		// reset pend
		pending = false;

		// ready so do
		uiInfo.nextServerStatusRefresh = 0;
		UI_GetServerStatusInfo( uiInfo.serverStatusAddress, NULL );
	}
	else
	{
		// try again in 500ms
		uiInfo.nextServerStatusRefresh = uiInfo.uiUtils.dc_->realTime_ + 500;
	}
}

/*
==================
UI_FeederCount
==================
*/
int UI_FeederCount(float feederID) 
{
	if (feederID == FEEDER_HEADS) {
		return UI_HeadCountByTeam();
	} else if (feederID == FEEDER_Q3HEADS) {
		return uiInfo.q3HeadCount;
	} else if (feederID == FEEDER_CINEMATICS) {
		return uiInfo.movieCount;
	} else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
		return UI_MapCountByGameType(feederID == FEEDER_MAPS ? true : false);
	} else if (feederID == FEEDER_SERVERS) {
		return uiInfo.serverStatus.numDisplayServers;
	} else if (feederID == FEEDER_SERVERSTATUS) {
		return uiInfo.serverStatusInfo.numLines;
	} else if (feederID == FEEDER_FINDPLAYER) {
		return uiInfo.numFoundPlayerServers;
	} else if (feederID == FEEDER_PLAYER_LIST) {
		if (uiInfo.uiUtils.dc_->realTime_ > uiInfo.playerRefresh) {
			uiInfo.playerRefresh = uiInfo.uiUtils.dc_->realTime_ + 3000;
			UI_BuildPlayerList();
		}
		return uiInfo.playerCount;
	} else if (feederID == FEEDER_TEAM_LIST) {
		if (uiInfo.uiUtils.dc_->realTime_ > uiInfo.playerRefresh) {
			uiInfo.playerRefresh = uiInfo.uiUtils.dc_->realTime_ + 3000;
			UI_BuildPlayerList();
		}
		return uiInfo.myTeamCount;
	} else if (feederID == FEEDER_MODS) {
		return uiInfo.modCount;
	} else if (feederID == FEEDER_DEMOS) {
		return uiInfo.demoCount;
	}
	return 0;
}

static const char *UI_SelectedMap(int index, int *actual) {
	int i, c;
	c = 0;
	*actual = 0;
	for (i = 0; i < uiInfo.mapCount; i++) {
		if (uiInfo.mapList[i].active) {
			if (c == index) {
				*actual = i;
				return uiInfo.mapList[i].mapName;
			} else {
				c++;
			}
		}
	}
	return "";
}

static const char *UI_SelectedHead(int index, int *actual) {
	int i, c;
	c = 0;
	*actual = 0;
	for (i = 0; i < uiInfo.characterCount; i++) {
		if (uiInfo.characterList[i].active) {
			if (c == index) {
				*actual = i;
				return uiInfo.characterList[i].name;
			} else {
				c++;
			}
		}
	}
	return "";
}

static int UI_GetIndexFromSelection(int actual) {
	int i, c;
	c = 0;
	for (i = 0; i < uiInfo.mapCount; i++) {
		if (uiInfo.mapList[i].active) {
			if (i == actual) {
				return c;
			}
				c++;
		}
	}
  return 0;
}

static void UI_UpdatePendingPings() { 
	LAN_ResetPings(ui_netSource.integer);
	uiInfo.serverStatus.refreshActive = true;
	uiInfo.serverStatus.refreshtime = uiInfo.uiUtils.dc_->realTime_ + 1000;

}

/*
==================
UI_CreateGameCodes
==================
*/
static char * UI_CreateGameCodes( char * pInfo )
{
	char exInfo[ 1024 ];
	char * pGameset = NULL, * pAddress = NULL;
	int gameType, gameset;

	// get enums
	gameType = atoi( Info_ValueForKey( pInfo, "gametype") );

	// valid?
	if( gameType >= 0 && gameType < numTeamArenaGameTypes )
	{
		// get server address
		pAddress = Info_ValueForKey( pInfo, "addr");
		if( !pAddress )
		{
			return "(Error)";
		}

		// get complete info text
		if( LAN_GetServerStatus( pAddress, exInfo, 1024 ) )
		{
			pGameset = Info_ValueForKey( exInfo, "mf_gameset");

			// convert gameset string to enum
			gameset = MF_UI_Gameset_StringToValue( pGameset, true );
			if( gameset < numGamesets )
			{
				return va( "%s/%s", mfqGamesets[gameset],teamArenaGameTypes[gameType] );
			}
		}
		else
		{
			// reset info requests
			LAN_GetServerStatus( pAddress, NULL, 0 );
	
			// pending...
			return "...";
		}
	}

	return "(Unknown)";
}

/*
==================
UI_FeederItemText
==================
*/

const char *UI_FeederItemText(float feederID, int index, int column, qhandle_t *handle)
{
	static char info[MAX_STRING_CHARS];
	static char hostname[1024];
	static char clientBuff[32];
	static int lastColumn = -1;
	static int lastTime = 0;
	*handle = -1;
	if (feederID == FEEDER_HEADS) {
		int actual;
		return UI_SelectedHead(index, &actual);
	} else if (feederID == FEEDER_Q3HEADS) {
		if (index >= 0 && index < uiInfo.q3HeadCount) {
			return uiInfo.q3HeadNames[index];
		}
	} else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
		int actual;
		return UI_SelectedMap(index, &actual);
	} else if (feederID == FEEDER_SERVERS) {
		if (index >= 0 && index < uiInfo.serverStatus.numDisplayServers) {
			int ping;
			if (lastColumn != column || lastTime > uiInfo.uiUtils.dc_->realTime_ + 5000) {
				LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
				lastColumn = column;
				lastTime = uiInfo.uiUtils.dc_->realTime_;
			}
			ping = atoi(Info_ValueForKey(info, "ping"));
			if (ping == -1) {
				// if we ever see a ping that is out of date, do a server refresh
				// UI_UpdatePendingPings();
			}
			switch (column)
			{
				case SORT_HOST : 
					if (ping <= 0)
					{
						return Info_ValueForKey(info, "addr");
					}
					else
					{
						if ( ui_netSource.integer == AS_LOCAL ) {
							Com_sprintf( hostname, sizeof(hostname), "%s [%s]",
											Info_ValueForKey(info, "hostname"),
											netnames[atoi(Info_ValueForKey(info, "nettype"))] );
							return hostname;
						}
						else {
							if (atoi(Info_ValueForKey(info, "sv_allowAnonymous")) != 0) {				// anonymous server
								Com_sprintf( hostname, sizeof(hostname), "(A) %s",
												Info_ValueForKey(info, "hostname"));
							} else {
								Com_sprintf( hostname, sizeof(hostname), "%s",
												Info_ValueForKey(info, "hostname"));
							}
							return hostname;
						}
					}

				case SORT_MAP :
					return Info_ValueForKey(info, "mapname");

				case SORT_CLIENTS : 
					Com_sprintf( clientBuff, sizeof(clientBuff), "%s (%s)", Info_ValueForKey(info, "clients"), Info_ValueForKey(info, "sv_maxclients"));
					return clientBuff;

				case SORT_GAME :
					return UI_CreateGameCodes( info );

				case SORT_PING : 
					if (ping <= 0) {
						return "...";
					} else {
						return Info_ValueForKey(info, "ping");
					}
			}
		}
	} else if (feederID == FEEDER_SERVERSTATUS) {
		if ( index >= 0 && index < uiInfo.serverStatusInfo.numLines ) {
			if ( column >= 0 && column < 4 ) {

				// make sure cursor is reset when we have finished
				uiInfo.uiUtils.dc_->cursorEnum_ = CURSOR_NORMAL;

				return uiInfo.serverStatusInfo.lines[index][column];
			}
		}
	} else if (feederID == FEEDER_FINDPLAYER) {
		if ( index >= 0 && index < uiInfo.numFoundPlayerServers ) {
			//return uiInfo.foundPlayerServerAddresses[index];
			return uiInfo.foundPlayerServerNames[index];
		}
	} else if (feederID == FEEDER_PLAYER_LIST) {
		if (index >= 0 && index < uiInfo.playerCount) {
			return uiInfo.playerNames[index];
		}
	} else if (feederID == FEEDER_TEAM_LIST) {
		if (index >= 0 && index < uiInfo.myTeamCount) {
			return uiInfo.teamNames[index];
		}
	} else if (feederID == FEEDER_MODS) {
		if (index >= 0 && index < uiInfo.modCount) {
			if (uiInfo.modList[index].modDescr && *uiInfo.modList[index].modDescr) {
				return uiInfo.modList[index].modDescr;
			} else {
				return uiInfo.modList[index].modName;
			}
		}
	} else if (feederID == FEEDER_CINEMATICS) {
		if (index >= 0 && index < uiInfo.movieCount) {
			return uiInfo.movieList[index];
		}
	} else if (feederID == FEEDER_DEMOS) {
		if (index >= 0 && index < uiInfo.demoCount) {
			return uiInfo.demoList[index];
		}
	}
	return "";
}


qhandle_t UI_FeederItemImage(float feederID, int index) 
{
  if (feederID == FEEDER_HEADS) {
	int actual;
	UI_SelectedHead(index, &actual);
	index = actual;
	if (index >= 0 && index < uiInfo.characterCount) {
		if (uiInfo.characterList[index].headImage == -1) {
			uiInfo.characterList[index].headImage = refExport.RegisterShaderNoMip(uiInfo.characterList[index].imageName);
		}
		return uiInfo.characterList[index].headImage;
	}
  } else if (feederID == FEEDER_Q3HEADS) {
    if (index >= 0 && index < uiInfo.q3HeadCount) {
      return uiInfo.q3HeadIcons[index];
    }
	} else if (feederID == FEEDER_ALLMAPS || feederID == FEEDER_MAPS) {
		int actual;
		UI_SelectedMap(index, &actual);
		index = actual;
		if (index >= 0 && index < uiInfo.mapCount) {
			if (uiInfo.mapList[index].levelShot == -1) {
				uiInfo.mapList[index].levelShot = refExport.RegisterShaderNoMip(uiInfo.mapList[index].imageName);
			}
			return uiInfo.mapList[index].levelShot;
		}
	}
  return 0;
}

void UI_FeederSelection(float feederID, int index) 
{
	static char info[MAX_STRING_CHARS];
  if (feederID == FEEDER_HEADS) {
	int actual;
	UI_SelectedHead(index, &actual);
	index = actual;
    if (index >= 0 && index < uiInfo.characterCount) {
		Cvar_Set( "team_model", va("%s", uiInfo.characterList[index].base));
		Cvar_Set( "team_headmodel", va("*%s", uiInfo.characterList[index].name)); 
		updateModel = true;
    }
  } else if (feederID == FEEDER_Q3HEADS) {
    if (index >= 0 && index < uiInfo.q3HeadCount) {
      Cvar_Set( "model", uiInfo.q3HeadNames[index]);
      Cvar_Set( "headmodel", uiInfo.q3HeadNames[index]);
			updateModel = true;
		}
  } else if (feederID == FEEDER_MAPS || feederID == FEEDER_ALLMAPS) {
		int actual, map;
		map = (feederID == FEEDER_ALLMAPS) ? ui_currentNetMap.integer : ui_currentMap.integer;
		if (uiInfo.mapList[map].cinematic >= 0) {
		  CIN_StopCinematic(uiInfo.mapList[map].cinematic);
		  uiInfo.mapList[map].cinematic = -1;
		}
		UI_SelectedMap(index, &actual);
		Cvar_Set("ui_mapIndex", va("%d", index));
		ui_mapIndex.integer = index;

		if (feederID == FEEDER_MAPS) {
			ui_currentMap.integer = actual;
			Cvar_Set("ui_currentMap", va("%d", actual));
	  	uiInfo.mapList[ui_currentMap.integer].cinematic = CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[ui_currentMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
			UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);
			Cvar_Set("ui_opponentModel", uiInfo.mapList[ui_currentMap.integer].opponentName);
			updateOpponentModel = true;
		} else {
			ui_currentNetMap.integer = actual;
			Cvar_Set("ui_currentNetMap", va("%d", actual));
	  	uiInfo.mapList[ui_currentNetMap.integer].cinematic = CIN_PlayCinematic(va("%s.roq", uiInfo.mapList[ui_currentNetMap.integer].mapLoadName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		}

  } else if (feederID == FEEDER_SERVERS) {
		const char *mapName = NULL;
		uiInfo.serverStatus.currentServer = index;
		LAN_GetServerInfo(ui_netSource.integer, uiInfo.serverStatus.displayServers[index], info, MAX_STRING_CHARS);
		uiInfo.serverStatus.currentServerPreview = refExport.RegisterShaderNoMip(va("levelshots/%s", Info_ValueForKey(info, "mapname")));
		if (uiInfo.serverStatus.currentServerCinematic >= 0) {
		  CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
			uiInfo.serverStatus.currentServerCinematic = -1;
		}
		mapName = Info_ValueForKey(info, "mapname");
		if (mapName && *mapName) {
			uiInfo.serverStatus.currentServerCinematic = CIN_PlayCinematic(va("%s.roq", mapName), 0, 0, 0, 0, (CIN_loop | CIN_silent) );
		}
  } else if (feederID == FEEDER_SERVERSTATUS) {
		//
  } else if (feederID == FEEDER_FINDPLAYER) {
	  uiInfo.currentFoundPlayerServer = index;
	  //
	  if ( index < uiInfo.numFoundPlayerServers-1) {
			// build a New server status for this server
			Q_strncpyz(uiInfo.serverStatusAddress, uiInfo.foundPlayerServerAddresses[uiInfo.currentFoundPlayerServer], sizeof(uiInfo.serverStatusAddress));
			uiInfo.uiUtils.menu_SetFeederSelection(NULL, FEEDER_SERVERSTATUS, 0, NULL);
			UI_BuildServerStatus(true);
	  }
  } else if (feederID == FEEDER_PLAYER_LIST) {
		uiInfo.playerIndex = index;
  } else if (feederID == FEEDER_TEAM_LIST) {
		uiInfo.teamIndex = index;
  } else if (feederID == FEEDER_MODS) {
		uiInfo.modIndex = index;
  } else if (feederID == FEEDER_CINEMATICS) {
		uiInfo.movieIndex = index;
		if (uiInfo.previewMovie >= 0) {
		  CIN_StopCinematic(uiInfo.previewMovie);
		}
		uiInfo.previewMovie = -1;
  } else if (feederID == FEEDER_DEMOS) {
		uiInfo.demoIndex = index;
	}
}

static bool Team_Parse(char **p) {
  char *token;
  const char *tempStr;
	int i;

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

    if (token[0] == '{') {
      // seven tokens per line, team name and icon, and 5 team member names
      if (!uiInfo.uiUtils.string_Parse(p, &uiInfo.teamList[uiInfo.teamCount].teamName) || 
		  !uiInfo.uiUtils.string_Parse(p, &tempStr)) {
        return false;
      }
    

			uiInfo.teamList[uiInfo.teamCount].imageName = tempStr;
	    uiInfo.teamList[uiInfo.teamCount].teamIcon = refExport.RegisterShaderNoMip(uiInfo.teamList[uiInfo.teamCount].imageName);
		  uiInfo.teamList[uiInfo.teamCount].teamIcon_Metal = refExport.RegisterShaderNoMip(va("%s_metal",uiInfo.teamList[uiInfo.teamCount].imageName));
			uiInfo.teamList[uiInfo.teamCount].teamIcon_Name = refExport.RegisterShaderNoMip(va("%s_name", uiInfo.teamList[uiInfo.teamCount].imageName));

			uiInfo.teamList[uiInfo.teamCount].cinematic = -1;

			for (i = 0; i < TEAM_MEMBERS; i++) {
				uiInfo.teamList[uiInfo.teamCount].teamMembers[i] = NULL;
				if (!uiInfo.uiUtils.string_Parse(p, &uiInfo.teamList[uiInfo.teamCount].teamMembers[i])) {
					return false;
				}
			}

      Com_Printf("Loaded team %s with team icon %s.\n", uiInfo.teamList[uiInfo.teamCount].teamName, tempStr);
      if (uiInfo.teamCount < MAX_TEAMS) {
        uiInfo.teamCount++;
      } else {
        Com_Printf("Too many teams, last team replaced!\n");
      }
      token = COM_ParseExt(p, true);
      if (token[0] != '}') {
        return false;
      }
    }
  }

  return false;
}

static bool Character_Parse(char **p) {
  char *token;
  const char *tempStr;

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

    if (token[0] == '{') {
      // two tokens per line, character name and sex
      if (!uiInfo.uiUtils.string_Parse(p, &uiInfo.characterList[uiInfo.characterCount].name) || 
		  !uiInfo.uiUtils.string_Parse(p, &tempStr)) {
        return false;
      }
    
      uiInfo.characterList[uiInfo.characterCount].headImage = -1;
			uiInfo.characterList[uiInfo.characterCount].imageName = 
				uiInfo.uiUtils.string_Alloc(va("models/players/heads/%s/icon_default.tga", 
				uiInfo.characterList[uiInfo.characterCount].name));

	  if (tempStr && (!Q_stricmp(tempStr, "female"))) {
        uiInfo.characterList[uiInfo.characterCount].base = uiInfo.uiUtils.string_Alloc(va("Janet"));
      } else if (tempStr && (!Q_stricmp(tempStr, "male"))) {
        uiInfo.characterList[uiInfo.characterCount].base = uiInfo.uiUtils.string_Alloc(va("James"));
	  } else {
        uiInfo.characterList[uiInfo.characterCount].base = uiInfo.uiUtils.string_Alloc(va("%s",tempStr));
	  }

      Com_Printf("Loaded %s character %s.\n", uiInfo.characterList[uiInfo.characterCount].base, uiInfo.characterList[uiInfo.characterCount].name);
      if (uiInfo.characterCount < MAX_HEADS) {
        uiInfo.characterCount++;
      } else {
        Com_Printf("Too many characters, last character replaced!\n");
      }
     
      token = COM_ParseExt(p, true);
      if (token[0] != '}') {
        return false;
      }
    }
  }

  return false;
}


static bool Alias_Parse(char **p) {
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

    if (token[0] == '{') {
      // three tokens per line, character name, bot alias, and preferred action a - all purpose, d - defense, o - offense
      if (!uiInfo.uiUtils.string_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].name) || 
		  !uiInfo.uiUtils.string_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].ai) || 
		  !uiInfo.uiUtils.string_Parse(p, &uiInfo.aliasList[uiInfo.aliasCount].action)) {
        return false;
      }
    
      Com_Printf("Loaded character alias %s using character ai %s.\n", uiInfo.aliasList[uiInfo.aliasCount].name, 
		  uiInfo.aliasList[uiInfo.aliasCount].ai);
      if (uiInfo.aliasCount < MAX_ALIASES) {
        uiInfo.aliasCount++;
      } else {
        Com_Printf("Too many aliases, last alias replaced!\n");
      }
     
      token = COM_ParseExt(p, true);
      if (token[0] != '}') {
        return false;
      }
    }
  }

  return false;
}



// mode 
// 0 - high level parsing
// 1 - team parsing
// 2 - character parsing
static void UI_ParseTeamInfo(const char *teamFile) {
	char	*token;
  char *p;
  char *buff = NULL;
  //static int mode = 0; TTimo: unused

  buff = GetMenuBuffer(teamFile);
  if (!buff) {
    return;
  }

  p = buff;

	while ( 1 ) {
		token = COM_ParseExt( &p, true );
		if( !token || token[0] == 0 || token[0] == '}') {
			break;
		}

		if ( Q_stricmp( token, "}" ) == 0 ) {
      break;
    }

    if (Q_stricmp(token, "teams") == 0) {

      if (Team_Parse(&p)) {
        continue;
      } else {
        break;
      }
    }

    if (Q_stricmp(token, "characters") == 0) {
      Character_Parse(&p);
    }

    if (Q_stricmp(token, "aliases") == 0) {
      Alias_Parse(&p);
    }

  }

}


static bool GameType_Parse(char **p, bool join) {
	char *token;

	token = COM_ParseExt(p, true);

	if (token[0] != '{') {
		return false;
	}

	if (join) {
		uiInfo.numJoinGameTypes = 0;
	} else {
		uiInfo.numGameTypes = 0;
	}

	while ( 1 ) {
		token = COM_ParseExt(p, true);

		if (Q_stricmp(token, "}") == 0) {
			return true;
		}

		if ( !token || token[0] == 0 ) {
			return false;
		}

		if (token[0] == '{') {
			// two tokens per line, character name and sex
			if (join) {
				if (!uiInfo.uiUtils.string_Parse(p, &uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gameType) || 
					!uiInfo.uiUtils.int_Parse(p, &uiInfo.joinGameTypes[uiInfo.numJoinGameTypes].gtEnum)) {
					return false;
				}
			} else {
				if (!uiInfo.uiUtils.string_Parse(p, &uiInfo.gameTypes[uiInfo.numGameTypes].gameType) || 
					!uiInfo.uiUtils.int_Parse(p, &uiInfo.gameTypes[uiInfo.numGameTypes].gtEnum)) {
					return false;
				}
			}
    
			if (join) {
				if (uiInfo.numJoinGameTypes < MAX_GAMETYPES) {
					uiInfo.numJoinGameTypes++;
				} else {
					Com_Printf("Too many net game types, last one replace!\n");
				}		
			} else {
				if (uiInfo.numGameTypes < MAX_GAMETYPES) {
					uiInfo.numGameTypes++;
				} else {
					Com_Printf("Too many game types, last one replace!\n");
				}		
			}
     
			token = COM_ParseExt(p, true);
			if (token[0] != '}') {
				return false;
			}
		}
	}
	return false;
}

static bool MapList_Parse(char **p) {
	char *token;

	token = COM_ParseExt(p, true);

	if (token[0] != '{') {
		return false;
	}

	uiInfo.mapCount = 0;

	while ( 1 ) {
		token = COM_ParseExt(p, true);

		if (Q_stricmp(token, "}") == 0) {
			return true;
		}

		if ( !token || token[0] == 0 ) {
			return false;
		}

		if (token[0] == '{') {
			if (!uiInfo.uiUtils.string_Parse(p, &uiInfo.mapList[uiInfo.mapCount].mapName) || 
				!uiInfo.uiUtils.string_Parse(p, &uiInfo.mapList[uiInfo.mapCount].mapLoadName) 
				||!uiInfo.uiUtils.int_Parse(p, &uiInfo.mapList[uiInfo.mapCount].teamMembers) ) {
				return false;
			}

			if (!uiInfo.uiUtils.string_Parse(p, &uiInfo.mapList[uiInfo.mapCount].opponentName)) {
				return false;
			}

			uiInfo.mapList[uiInfo.mapCount].typeBits = 0;

			while (1) {
				token = COM_ParseExt(p, true);
				if (token[0] >= '0' && token[0] <= '9') {
					uiInfo.mapList[uiInfo.mapCount].typeBits |= (1 << (token[0] - 0x030));
					if (!uiInfo.uiUtils.int_Parse(p, &uiInfo.mapList[uiInfo.mapCount].timeToBeat[token[0] - 0x30])) {
						return false;
					}
				} else {
					break;
				} 
			}

			//mapList[mapCount].imageName = String_Alloc(va("levelshots/%s", mapList[mapCount].mapLoadName));
			//if (uiInfo.mapCount == 0) {
			  // only load the first cinematic, selection loads the others
  			//  uiInfo.mapList[uiInfo.mapCount].cinematic = CIN_PlayCinematic(va("%s.roq",uiInfo.mapList[uiInfo.mapCount].mapLoadName), false, false, true, 0, 0, 0, 0);
			//}
  		uiInfo.mapList[uiInfo.mapCount].cinematic = -1;
			uiInfo.mapList[uiInfo.mapCount].levelShot = refExport.RegisterShaderNoMip(va("levelshots/%s_small", uiInfo.mapList[uiInfo.mapCount].mapLoadName));

			if (uiInfo.mapCount < MAX_MAPS) {
				uiInfo.mapCount++;
			} else {
				Com_Printf("Too many maps, last one replaced!\n");
			}
		}
	}
	return false;
}

static void UI_ParseGameInfo(const char *teamFile) {
	char	*token;
	char *p;
	char *buff = NULL;
	//int mode = 0; TTimo: unused

	buff = GetMenuBuffer(teamFile);
	if (!buff) {
		return;
	}

	p = buff;

	while ( 1 ) {
		token = COM_ParseExt( &p, true );
		if( !token || token[0] == 0 || token[0] == '}') {
			break;
		}

		if ( Q_stricmp( token, "}" ) == 0 ) {
			break;
		}

		if (Q_stricmp(token, "gametypes") == 0) {

			if (GameType_Parse(&p, false)) {
				continue;
			} else {
				break;
			}
		}

		if (Q_stricmp(token, "joingametypes") == 0) {

			if (GameType_Parse(&p, true)) {
				continue;
			} else {
				break;
			}
		}

		if (Q_stricmp(token, "maps") == 0) {
			// start a New menu
			MapList_Parse(&p);
		}

	}
}

void UI_Pause(bool b) 
{
	if (b) {
		// pause the game and set the ui keycatcher
	  Cvar_Set( "cl_paused", "1" );
		Key_SetCatcher( KEYCATCH_UI );
	} else {
		// unpause the game and clear the ui keycatcher
		Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
		Key_ClearStates();
		Cvar_Set( "cl_paused", "0" );
	}
}


int UI_PlayCinematic(const char *name, float x, float y, float w, float h) 
{
	return CIN_PlayCinematic(name, x, y, w, h, (CIN_loop | CIN_silent));
}

void UI_StopCinematic(int handle) 
{
	if (handle >= 0) {
	  CIN_StopCinematic(handle);
	} else {
		handle = abs(handle);
		if (handle == UI_MAPCINEMATIC) {
			if (uiInfo.mapList[ui_currentMap.integer].cinematic >= 0) {
			  CIN_StopCinematic(uiInfo.mapList[ui_currentMap.integer].cinematic);
			  uiInfo.mapList[ui_currentMap.integer].cinematic = -1;
			}
		} else if (handle == UI_NETMAPCINEMATIC) {
			if (uiInfo.serverStatus.currentServerCinematic >= 0) {
			  CIN_StopCinematic(uiInfo.serverStatus.currentServerCinematic);
				uiInfo.serverStatus.currentServerCinematic = -1;
			}
		} else if (handle == UI_CLANCINEMATIC) {
		  int i = UI_TeamIndexFromName(UI_Cvar_VariableString("ui_teamName"));
		  if (i >= 0 && i < uiInfo.teamCount) {
				if (uiInfo.teamList[i].cinematic >= 0) {
				  CIN_StopCinematic(uiInfo.teamList[i].cinematic);
					uiInfo.teamList[i].cinematic = -1;
				}
			}
		}
	}
}

void UI_DrawCinematic(int handle, float x, float y, float w, float h) 
{
	CIN_SetExtents(handle, x, y, w, h);
	CIN_DrawCinematic(handle);
}

void UI_RunCinematicFrame(int handle) 
{
	CIN_RunCinematic(handle);
}


/*
=================
MFQ3 CUSTOM CONSOLE
=================
*/

/*
=================
UI_CustomChatInit
=================
*/

void UI_CustomChatInit( void )
{
	// clear memory
	memset( &uiInfo.customChat, 0, sizeof( chat_t ) );
}

/*
=================
UI_Init
=================
*/
void _UI_Init( bool inGameLoad ) 
{
	const char *menuSet;
	int start;

	//uiInfo.inGameLoad = inGameLoad;

	UI_RegisterCvars();
	uiInfo.uiUtils.initMemory();

	// cache redundant calulations
	CL_GetGlconfig( &uiInfo.uiUtils.dc_->glConfig_ );

	// for 640x480 virtualized screen
	uiInfo.uiUtils.dc_->yScale_ = uiInfo.uiUtils.dc_->glConfig_.vidHeight * (1.0/480.0);
	uiInfo.uiUtils.dc_->xScale_ = uiInfo.uiUtils.dc_->glConfig_.vidWidth * (1.0/640.0);
	if ( uiInfo.uiUtils.dc_->glConfig_.vidWidth * 480 > uiInfo.uiUtils.dc_->glConfig_.vidHeight * 640 ) {
		// wide screen
		uiInfo.uiUtils.dc_->bias_ = 0.5 * ( uiInfo.uiUtils.dc_->glConfig_.vidWidth - ( uiInfo.uiUtils.dc_->glConfig_.vidHeight * (640.0/480.0) ) );
	}
	else {
		// no wide screen
		uiInfo.uiUtils.dc_->bias_ = 0;
	}

	uiInfo.uiUtils.string_Init();
  
	uiInfo.uiUtils.dc_->whiteShader_ = refExport.RegisterShaderNoMip( "white" );
	uiInfo.uiUtils.dc_->cursor_	= refExport.RegisterShaderNoMip( "menu/art/cursor" );
	uiInfo.uiUtils.dc_->cursorEnum_ = CURSOR_NORMAL;

	AssetCache();

	start = Sys_Milliseconds();

	uiInfo.teamCount = 0;
	uiInfo.characterCount = 0;
	uiInfo.aliasCount = 0;

	UI_ParseTeamInfo("teaminfo.txt");
	UI_LoadTeams();
	UI_ParseGameInfo("gameinfo.txt");

	menuSet = UI_Cvar_VariableString("ui_menuFiles");
	if (menuSet == NULL || menuSet[0] == '\0') {
		menuSet = "ui/menus.txt";
	}

#if 0
	if (uiInfo.inGameLoad) {
		UI_LoadMenus("ui/ingame.txt", true);
	} else { // bk010222: left this: UI_LoadMenus(menuSet, true);
	}
#else 
	UI_LoadMenus(menuSet, true);
	UI_LoadMenus("ui/ingame.txt", false);
#endif
	
	uiInfo.uiUtils.menu_CloseAll();

	LAN_LoadCachedServers();
	UI_LoadBestScores(uiInfo.mapList[ui_currentMap.integer].mapLoadName, uiInfo.gameTypes[ui_gameType.integer].gtEnum);

	//UI_LoadBots();

	// sets defaults for ui temp cvars
	uiInfo.effectsColor = gamecodetoui[(int)Cvar_VariableValue("color1")-1];
	uiInfo.currentCrosshair = Cvar_VariableValue("cg_drawCrosshair");
	Cvar_Set("ui_mousePitch", (Cvar_VariableValue("m_pitch") >= 0) ? "0" : "1");

	uiInfo.serverStatus.currentServerCinematic = -1;
	uiInfo.previewMovie = -1;

	if (Cvar_VariableValue("ui_TeamArenaFirstRun") == 0) {
		Cvar_Set("s_volume", "0.8");
		Cvar_Set("s_musicvolume", "0.5");
		Cvar_Set("ui_TeamArenaFirstRun", "1");
	}

	Cvar_Register(NULL, "debug_protocol", "", 0 );

	Cvar_Set("ui_actualNetGameType", va("%d", ui_netGameType.integer));

	// set the MFQ3 version string
	Cvar_Set( "ui_mfq3Version", MF_VERSION );

	// MFQ3 data
	MF_LoadAllVehicleData();

	// setup the custom chat interface
	UI_CustomChatInit();
}


/*
=================
UI_KeyEvent
=================
*/
void _UI_KeyEvent( int key, bool down )
{
	// if custom console is up, forward down key presses to the handler
	if( uiInfo.uiUtils.customChatUpdate( key, down ) )
	{
		return;
	}

	if (uiInfo.uiUtils.menu_Count() > 0)
	{
		menuDef_t *menu = uiInfo.uiUtils.menu_GetFocused();
		if (menu)
		{
			if (key == K_ESCAPE && down && !uiInfo.uiUtils.menu_AnyFullScreenVisible())
			{
				uiInfo.uiUtils.menu_CloseAll();
			}
			else
			{
				uiInfo.uiUtils.menu_HandleKey(menu, key, down );
			}
		}
		else
		{
			Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
			Key_ClearStates();
			Cvar_Set( "cl_paused", "0" );
		}
	}
/*
	if ((s > 0) && (s != menu_null_sound))
	{
		S_StartLocalSound( s, CHAN_LOCAL_SOUND );
	}
*/
}

/*
=================
UI_MouseEvent
=================
*/
void _UI_MouseEvent( int dx, int dy )
{
	// update mouse screen position
	uiInfo.uiUtils.dc_->cursorX_ += dx;
	if (uiInfo.uiUtils.dc_->cursorX_ < 0)
		uiInfo.uiUtils.dc_->cursorX_ = 0;
	else if (uiInfo.uiUtils.dc_->cursorX_ > SCREEN_WIDTH)
		uiInfo.uiUtils.dc_->cursorX_ = SCREEN_WIDTH;

	uiInfo.uiUtils.dc_->cursorY_ += dy;
	if (uiInfo.uiUtils.dc_->cursorY_ < 0)
		uiInfo.uiUtils.dc_->cursorY_ = 0;
	else if (uiInfo.uiUtils.dc_->cursorY_ > SCREEN_HEIGHT)
		uiInfo.uiUtils.dc_->cursorY_ = SCREEN_HEIGHT;

  if (uiInfo.uiUtils.menu_Count() > 0) {
    //menuDef_t *menu = Menu_GetFocused();
    //Menu_HandleMouseMove(menu, uiInfo.uiDC.cursorx, uiInfo.uiDC.cursory);
		uiInfo.uiUtils.display_MouseMove(NULL, uiInfo.uiUtils.dc_->cursorX_, uiInfo.uiUtils.dc_->cursorY_);
  }

}

void UI_LoadNonIngame() {
	const char *menuSet = UI_Cvar_VariableString("ui_menuFiles");
	if (menuSet == NULL || menuSet[0] == '\0') {
		menuSet = "ui/menus.txt";
	}
	UI_LoadMenus(menuSet, false);
	uiInfo.inGameLoad = false;
}

void _UI_SetActiveMenu( UserInterface::MenuCommand menu ) {
	char buf[256];

	// this should be the ONLY way the menu system is brought up
	// enusure minumum menu data is cached
  if (uiInfo.uiUtils.menu_Count() > 0) {
		vec3_t v;
		v[0] = v[1] = v[2] = 0;
	  switch ( menu ) {
	  case UserInterface::MenuCommand::UIMENU_NONE:
			Key_SetCatcher( Key_GetCatcher() & ~KEYCATCH_UI );
			Key_ClearStates();
			Cvar_Set( "cl_paused", "0" );
			uiInfo.uiUtils.menu_CloseAll();

		  return;
	  case UserInterface::MenuCommand::UIMENU_MAIN:
			//Cvar_Set( "sv_killserver", "1" );
			Key_SetCatcher( KEYCATCH_UI );
			//S_StartLocalSound( S_RegisterSound("sound/misc/menu_background.wav", false) , CHAN_LOCAL_SOUND );
			//S_StartBackgroundTrack("sound/misc/menu_background.wav", NULL);
			if (uiInfo.inGameLoad) {
				UI_LoadNonIngame();
			}
			uiInfo.uiUtils.menu_CloseAll();
			uiInfo.uiUtils.menu_ActivateByName("main");
			Cvar_VariableStringBuffer("com_errorMessage", buf, sizeof(buf));
			if (strlen(buf)) {
				if (!ui_singlePlayerActive.integer) {
					uiInfo.uiUtils.menu_ActivateByName("error_popmenu");
				} else {
					Cvar_Set("com_errorMessage", "");
				}
			}
		  return;
	  case UserInterface::MenuCommand::UIMENU_TEAM:
			Key_SetCatcher( KEYCATCH_UI );
			uiInfo.uiUtils.menu_ActivateByName("team");
		  return;
	  case UserInterface::MenuCommand::UIMENU_NEED_CD:
		  return;
	  case UserInterface::MenuCommand::UIMENU_BAD_CD_KEY:
		  return;
	  case UserInterface::MenuCommand::UIMENU_POSTGAME:
			//Cvar_Set( "sv_killserver", "1" );
			Key_SetCatcher( KEYCATCH_UI );
			if (uiInfo.inGameLoad) {
				UI_LoadNonIngame();
			}
			uiInfo.uiUtils.menu_CloseAll();
			uiInfo.uiUtils.menu_ActivateByName("endofgame");
		  //UI_ConfirmMenu( "Bad CD Key", NULL, NeedCDKeyAction );
		  return;
	  case UserInterface::MenuCommand::UIMENU_INGAME:
		  Cvar_Set( "cl_paused", "1" );
			Key_SetCatcher( KEYCATCH_UI );
			UI_BuildPlayerList();
			uiInfo.uiUtils.menu_CloseAll();
			uiInfo.uiUtils.menu_ActivateByName("ingame");
		  return;
	  }
  }
}

bool _UI_IsFullscreen( void ) 
{
	return uiInfo.uiUtils.menu_AnyFullScreenVisible();
}



static connstate_t	lastConnState;
static char			lastLoadingText[MAX_INFO_VALUE];

static void UI_ReadableSize ( char *buf, int bufsize, int value )
{
	if (value > 1024*1024*1024 ) { // gigs
		Com_sprintf( buf, bufsize, "%d", value / (1024*1024*1024) );
		Com_sprintf( buf+strlen(buf), bufsize-strlen(buf), ".%02d GB", 
			(value % (1024*1024*1024))*100 / (1024*1024*1024) );
	} else if (value > 1024*1024 ) { // megs
		Com_sprintf( buf, bufsize, "%d", value / (1024*1024) );
		Com_sprintf( buf+strlen(buf), bufsize-strlen(buf), ".%02d MB", 
			(value % (1024*1024))*100 / (1024*1024) );
	} else if (value > 1024 ) { // kilos
		Com_sprintf( buf, bufsize, "%d KB", value / 1024 );
	} else { // bytes
		Com_sprintf( buf, bufsize, "%d bytes", value );
	}
}

// Assumes time is in msec
static void UI_PrintTime ( char *buf, int bufsize, int time ) {
	time /= 1000;  // change to seconds

	if (time > 3600) { // in the hours range
		Com_sprintf( buf, bufsize, "%d hr %d min", time / 3600, (time % 3600) / 60 );
	} else if (time > 60) { // mins
		Com_sprintf( buf, bufsize, "%d min %d sec", time / 60, time % 60 );
	} else  { // secs
		Com_sprintf( buf, bufsize, "%d sec", time );
	}
}

void Text_PaintCenter(float x, float y, float scale, vec4_t color, const char *text, float adjust) {
	int len = Text_Width(text, scale, 0);
	Text_Paint(x - len / 2, y, scale, color, text, 0, 0, ITEM_TEXTSTYLE_SHADOWEDMORE);
}

/*
=================
UI_DisplayDownloadInfo
=================
*/
static void UI_DisplayDownloadInfo( const char *downloadName, float centerPoint, float yStart, float scale )
{
	static char dlText[]	= "Downloading:";
	static char etaText[]	= "Estimated time left:";
	static char xferText[]	= "Transfer rate:";

	int downloadSize, downloadCount, downloadTime;
	char dlSizeBuf[64], totalSizeBuf[64], xferRateBuf[64], dlTimeBuf[64];
	int xferRate;
	int leftWidth;
	const char *s;

	scale = 0.30f;
	yStart += 48.0f;

	downloadSize = Cvar_VariableValue( "cl_downloadSize" );
	downloadCount = Cvar_VariableValue( "cl_downloadCount" );
	downloadTime = Cvar_VariableValue( "cl_downloadTime" );

	leftWidth = 320;

	UI_SetColor(colorWhite);
	Text_PaintCenter(centerPoint, yStart + 112, scale + 0.2f, colorWhite, dlText, 0);
	Text_PaintCenter(centerPoint, yStart + 164, scale, colorWhite, etaText, 0);
	Text_PaintCenter(centerPoint, yStart + 196, scale, colorWhite, xferText, 0);

	if (downloadSize > 0) {
		s = va( "%s (%d%%)", downloadName, downloadCount * 100 / downloadSize );
	} else {
		s = downloadName;
	}

	Text_PaintCenter(centerPoint, yStart+132, scale, colorWhite, s, 0);

	UI_ReadableSize( dlSizeBuf,		sizeof dlSizeBuf,		downloadCount );
	UI_ReadableSize( totalSizeBuf,	sizeof totalSizeBuf,	downloadSize );

	if( downloadCount < 4096 || !downloadTime )
	{
		Text_PaintCenter(leftWidth, yStart+180, scale, colorWhite, "estimating", 0);
		Text_PaintCenter(leftWidth, yStart+148, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
	}
	else
	{
		if ((uiInfo.uiUtils.dc_->realTime_ - downloadTime) / 1000)
		{
			xferRate = downloadCount / ((uiInfo.uiUtils.dc_->realTime_ - downloadTime) / 1000);
		}
		else
		{
			xferRate = 0;
		}
		UI_ReadableSize( xferRateBuf, sizeof xferRateBuf, xferRate );

		// Extrapolate estimated completion time
		if( downloadSize && xferRate )
		{
			int n = downloadSize / xferRate; // estimated time for entire d/l in secs

			// We do it in K (/1024) because we'd overflow around 4MB
			UI_PrintTime ( dlTimeBuf, sizeof dlTimeBuf, (n - (((downloadCount/1024) * n) / (downloadSize/1024))) * 1000);

			Text_PaintCenter(leftWidth, yStart+180, scale, colorWhite, dlTimeBuf, 0);
			Text_PaintCenter(leftWidth, yStart+148, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
		}
		else
		{
			Text_PaintCenter(leftWidth, yStart+180, scale, colorWhite, "estimating", 0);
			
			if (downloadSize)
			{
				Text_PaintCenter(leftWidth, yStart+148, scale, colorWhite, va("(%s of %s copied)", dlSizeBuf, totalSizeBuf), 0);
			} else {
				Text_PaintCenter(leftWidth, yStart+148, scale, colorWhite, va("(%s copied)", dlSizeBuf), 0);
			}
		}

		if( xferRate )
		{
			Text_PaintCenter(leftWidth, yStart+212, scale, colorWhite, va("%s/Sec", xferRateBuf), 0);
		}
	}
}

/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawConnectScreen( bool overlay )
{
	uiClientState_t	cstate;
	char * s = NULL;
	char info[MAX_INFO_VALUE];
	char text[256];
	char exInfo[1024];
	char * pGameset = NULL;
	float centerPoint, yStart, scale;
	int gameset = 0;
	menuDef_t * menu = NULL;

	static bool resetRequest = false;
	static bool giveUpLoadingGfx = false;
	static float w = 0.0f;
	
	// find the menu
	menu = uiInfo.uiUtils.menu_FindByName( "Connect" );

	// paint connect menu?
	if( !overlay && menu )
	{
		uiInfo.uiUtils.menu_Paint( menu, true );
	}

	// adjust coords for overlay mode
	if( !overlay )
	{
		centerPoint = 320;
		yStart = 200;
		scale = 0.5f;
	}
	else
	{
		// DRAW NOTHING MORE IF IN 'OVERLAY MODE'
		return;
	}

	// get server information
	GetClientState( &cstate );

	// got the mid-panel shader?
	if( uiInfo.uiUtils.dc_->assets_.midPanelGfx )
	{
		// draw gfx
		// NOTE: this only gets drawn until CGame takes over the drawing of this
		UI_DrawHandlePic( 0, 160, 640, 160, uiInfo.uiUtils.dc_->assets_.midPanelGfx );
	}
	else
	{
		// not loaded the gfx yet

		// try?
		if( !giveUpLoadingGfx )
		{
			// get complete info text
			if( LAN_GetServerStatus( cstate.servername, exInfo, 1024 ) )
			{
				pGameset = Info_ValueForKey( exInfo, "mf_gameset");

				// convert gameset string to enum
				gameset = MF_UI_Gameset_StringToValue( pGameset, true );
				if( gameset < numGamesets )
				{
					// attempt load - once only!
					uiInfo.uiUtils.dc_->assets_.midPanelGfx = refExport.RegisterShaderNoMip( va( "ui\\assets\\mid-%s", pGameset ) );

					// didn't work?
					if( !uiInfo.uiUtils.dc_->assets_.midPanelGfx )
					{
						// don't try again
						giveUpLoadingGfx = true;
					}
				}
			}
			else
			{
				// reset info requests
				if( !resetRequest )
				{
					LAN_GetServerStatus( cstate.servername, NULL, 0 );
					resetRequest = true;
				}
			}
		}
	}

	// get info string
	info[0] = 0;
	if( GetConfigString( CS_SERVERINFO, info, sizeof(info) ) )
	{
		Text_PaintCenter( centerPoint, yStart, scale, colorWhite, va( "Loading map - %s", Info_ValueForKey( info, "mapname" )), 0 );
	}

	// connect to local server?
	if( !Q_stricmp( cstate.servername,"localhost" ) )
	{
		// local server
		Text_PaintCenter(centerPoint, yStart + 48, scale, colorWhite, va( "Connecting to localhost..." ), ITEM_TEXTSTYLE_SHADOWEDMORE);
	}
	else
	{
		// remote server
		strcpy( text, va( "Connecting to server %s...", cstate.servername ) );
		Text_PaintCenter( centerPoint, yStart + 48, scale, colorWhite,text , ITEM_TEXTSTYLE_SHADOWEDMORE );
	}
/*
	UI_DrawProportionalString( 320, 96, "Press Esc to abort", UI_CENTER|UI_SMALLFONT|UI_DROPSHADOW, menu_text_color );

	// display global MOTD at bottom
	Text_PaintCenter( centerPoint, 600, scale, colorWhite, Info_ValueForKey( cstate.updateInfoString, "motd" ), 0 );
*/	
	// print any server info (server full, bad version, etc)
	if( cstate.connState < CA_CONNECTED )
	{
		Text_PaintCenter(centerPoint, yStart + 96, scale, colorWhite, cstate.messageString, 0);
	}

	if( lastConnState > cstate.connState )
	{
		lastLoadingText[0] = '\0';
	}
	
	lastConnState = cstate.connState;

	switch( cstate.connState )
	{
	case CA_CONNECTING:
		s = va("Awaiting connection...%i", cstate.connectPacketCount);
		break;

	case CA_CHALLENGING:
		s = va("Awaiting challenge...%i", cstate.connectPacketCount);
		break;

	case CA_CONNECTED:
		{
		char downloadName[MAX_INFO_VALUE];

			Cvar_VariableStringBuffer( "cl_downloadName", downloadName, sizeof(downloadName) );
			if( *downloadName )
			{
				UI_DisplayDownloadInfo( downloadName, centerPoint, yStart, scale );
				return;
			}
		}
		s = "Awaiting gamestate...";
		break;

	case CA_LOADING:
		return;

	case CA_PRIMED:
		return;

	default:
		return;
	}

	// connecting to remove server?
	if( Q_stricmp( cstate.servername,"localhost" ) )
	{
		Text_PaintCenter( centerPoint, yStart + 96, scale, colorWhite, s, 0 );
	}

	// password required / connection rejected information goes here
}


/*
================
cvars
================
*/

typedef struct {
	vmCvar_t	*vmCvar;
	char		*cvarName;
	char		*defaultString;
	int			cvarFlags;
} cvarTable_t;

vmCvar_t	ui_ffa_fraglimit;
vmCvar_t	ui_ffa_timelimit;

vmCvar_t	ui_tourney_fraglimit;
vmCvar_t	ui_tourney_timelimit;

vmCvar_t	ui_team_fraglimit;
vmCvar_t	ui_team_timelimit;
vmCvar_t	ui_team_friendly;

vmCvar_t	ui_ctf_capturelimit;
vmCvar_t	ui_ctf_timelimit;
vmCvar_t	ui_ctf_friendly;

vmCvar_t	ui_arenasFile;
//vmCvar_t	ui_botsFile;
vmCvar_t	ui_spScores1;
vmCvar_t	ui_spScores2;
vmCvar_t	ui_spScores3;
vmCvar_t	ui_spScores4;
vmCvar_t	ui_spScores5;
vmCvar_t	ui_spAwards;
vmCvar_t	ui_spVideos;
vmCvar_t	ui_spSkill;

vmCvar_t	ui_spSelection;

vmCvar_t	ui_browserMaster;
vmCvar_t	ui_browserGameType;
vmCvar_t	ui_browserSortKey;
vmCvar_t	ui_browserShowFull;
vmCvar_t	ui_browserShowEmpty;

vmCvar_t	ui_brassTime;
vmCvar_t	ui_drawCrosshair;
vmCvar_t	ui_drawCrosshairNames;
vmCvar_t	ui_marks;

vmCvar_t	ui_server1;
vmCvar_t	ui_server2;
vmCvar_t	ui_server3;
vmCvar_t	ui_server4;
vmCvar_t	ui_server5;
vmCvar_t	ui_server6;
vmCvar_t	ui_server7;
vmCvar_t	ui_server8;
vmCvar_t	ui_server9;
vmCvar_t	ui_server10;
vmCvar_t	ui_server11;
vmCvar_t	ui_server12;
vmCvar_t	ui_server13;
vmCvar_t	ui_server14;
vmCvar_t	ui_server15;
vmCvar_t	ui_server16;

//vmCvar_t	ui_cdkeychecked;

vmCvar_t	ui_redteam;
vmCvar_t	ui_redteam1;
vmCvar_t	ui_redteam2;
vmCvar_t	ui_redteam3;
vmCvar_t	ui_redteam4;
vmCvar_t	ui_redteam5;
vmCvar_t	ui_blueteam;
vmCvar_t	ui_blueteam1;
vmCvar_t	ui_blueteam2;
vmCvar_t	ui_blueteam3;
vmCvar_t	ui_blueteam4;
vmCvar_t	ui_blueteam5;
vmCvar_t	ui_teamName;
vmCvar_t	ui_dedicated;
vmCvar_t	ui_gameType;
vmCvar_t	ui_netGameType;
//vmCvar_t	ui_netGameset;
vmCvar_t	ui_actualNetGameType;
vmCvar_t	ui_joinGameType;
vmCvar_t	ui_netSource;
vmCvar_t	ui_serverFilterType;
vmCvar_t	ui_opponentName;
vmCvar_t	ui_menuFiles;
vmCvar_t	ui_currentTier;
vmCvar_t	ui_currentMap;
vmCvar_t	ui_currentNetMap;
vmCvar_t	ui_mapIndex;
vmCvar_t	ui_currentOpponent;
vmCvar_t	ui_selectedPlayer;
vmCvar_t	ui_selectedPlayerName;
vmCvar_t	ui_lastServerRefresh_0;
vmCvar_t	ui_lastServerRefresh_1;
vmCvar_t	ui_lastServerRefresh_2;
vmCvar_t	ui_lastServerRefresh_3;
vmCvar_t	ui_singlePlayerActive;
vmCvar_t	ui_scoreAccuracy;
vmCvar_t	ui_scoreImpressives;
vmCvar_t	ui_scoreExcellents;
vmCvar_t	ui_scoreCaptures;
vmCvar_t	ui_scoreDefends;
vmCvar_t	ui_scoreAssists;
vmCvar_t	ui_scoreGauntlets;
vmCvar_t	ui_scoreScore;
vmCvar_t	ui_scorePerfect;
vmCvar_t	ui_scoreTeam;
vmCvar_t	ui_scoreBase;
vmCvar_t	ui_scoreTimeBonus;
vmCvar_t	ui_scoreSkillBonus;
vmCvar_t	ui_scoreShutoutBonus;
vmCvar_t	ui_scoreTime;
vmCvar_t	ui_captureLimit;
vmCvar_t	ui_fragLimit;
vmCvar_t	ui_smallFont;
vmCvar_t	ui_bigFont;
vmCvar_t	ui_findPlayer;
vmCvar_t	ui_Q3Model;
vmCvar_t	ui_hudFiles;
vmCvar_t	ui_recordSPDemo;
vmCvar_t	ui_realCaptureLimit;
vmCvar_t	ui_realWarmUp;
vmCvar_t	ui_serverStatusTimeOut;

// MFQ3
//vmCvar_t	ui_gameset;			// UI copy of the gameset var (not sure were going to need this - MM)

// (vehicle select dialog)
vmCvar_t	ui_vehicleCat;			// vehicle catagory value
vmCvar_t	ui_vehicleClass;		// vehicle class value
vmCvar_t	ui_vehicleLoadout;		// vehicle loadout value
vmCvar_t	ui_vehicle;				// vehicle value

vmCvar_t	ui_vehicleCatTxt;		// vehicle catagory text
vmCvar_t	ui_vehicleClassTxt;		// vehicle class text
vmCvar_t	ui_vehicleLoadoutTxt;	// vehicle loadout text
vmCvar_t	ui_vehicleTxt;			// vehicle text

vmCvar_t	ui_availableVehiclesTxt;	// available vehicles text

vmCvar_t	ui_mfq3Version;		// version text

// bk001129 - made static to avoid aliasing
static cvarTable_t		cvarTable[] = {
	{ &ui_ffa_fraglimit, "ui_ffa_fraglimit", "20", CVAR_ARCHIVE },
	{ &ui_ffa_timelimit, "ui_ffa_timelimit", "0", CVAR_ARCHIVE },

	{ &ui_tourney_fraglimit, "ui_tourney_fraglimit", "0", CVAR_ARCHIVE },
	{ &ui_tourney_timelimit, "ui_tourney_timelimit", "15", CVAR_ARCHIVE },

	{ &ui_team_fraglimit, "ui_team_fraglimit", "0", CVAR_ARCHIVE },
	{ &ui_team_timelimit, "ui_team_timelimit", "20", CVAR_ARCHIVE },
	{ &ui_team_friendly, "ui_team_friendly",  "1", CVAR_ARCHIVE },

	{ &ui_ctf_capturelimit, "ui_ctf_capturelimit", "8", CVAR_ARCHIVE },
	{ &ui_ctf_timelimit, "ui_ctf_timelimit", "30", CVAR_ARCHIVE },
	{ &ui_ctf_friendly, "ui_ctf_friendly",  "0", CVAR_ARCHIVE },

	{ &ui_arenasFile, "g_arenasFile", "", CVAR_INIT|CVAR_ROM },
//	{ &ui_botsFile, "g_botsFile", "", CVAR_INIT|CVAR_ROM },
	{ &ui_spScores1, "g_spScores1", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spScores2, "g_spScores2", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spScores3, "g_spScores3", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spScores4, "g_spScores4", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spScores5, "g_spScores5", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spAwards, "g_spAwards", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spVideos, "g_spVideos", "", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_spSkill, "g_spSkill", "2", CVAR_ARCHIVE },

	{ &ui_spSelection, "ui_spSelection", "", CVAR_ROM },

	{ &ui_browserMaster, "ui_browserMaster", "0", CVAR_ARCHIVE },
	{ &ui_browserGameType, "ui_browserGameType", "0", CVAR_ARCHIVE },
	{ &ui_browserSortKey, "ui_browserSortKey", "4", CVAR_ARCHIVE },
	{ &ui_browserShowFull, "ui_browserShowFull", "1", CVAR_ARCHIVE },
	{ &ui_browserShowEmpty, "ui_browserShowEmpty", "1", CVAR_ARCHIVE },

	{ &ui_brassTime, "cg_brassTime", "2500", CVAR_ARCHIVE },
	{ &ui_drawCrosshair, "cg_drawCrosshair", "4", CVAR_ARCHIVE },
	{ &ui_drawCrosshairNames, "cg_drawCrosshairNames", "1", CVAR_ARCHIVE },
	{ &ui_marks, "cg_marks", "1", CVAR_ARCHIVE },

	// MFQ3
//	{ &ui_gameset, "ui_gameset", "modern", CVAR_ARCHIVE | CVAR_ROM },

	// (vehicle select dialog)
	{ &ui_vehicleCat, "ui_vehicleCat", "1", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_vehicleClass, "ui_vehicleClass", "1", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_vehicleClass, "ui_vehicleLoadout", "1", CVAR_ARCHIVE | CVAR_ROM },
	{ &ui_vehicle, "ui_vehicle", "0", CVAR_ARCHIVE | CVAR_ROM },

	{ &ui_vehicleCat, "ui_vehicleCatTxt", "<Category>", CVAR_ROM },
	{ &ui_vehicleClass, "ui_vehicleClassTxt", "<Class>", CVAR_ROM },
	{ &ui_vehicleClass, "ui_vehicleLoadoutTxt", "<Loadout>", CVAR_ROM },
	{ &ui_vehicle, "ui_vehicleTxt", "<Vehicle>", CVAR_ROM },

	{ &ui_availableVehiclesTxt, "ui_availableVehiclesTxt", "<Available Vehicles>", CVAR_ROM },
	
	// (misc)
	{ &ui_mfq3Version, "ui_mfq3Version", "<MFQ3 Version>", CVAR_ROM },

	{ &ui_server1, "server1", "", CVAR_ARCHIVE },
	{ &ui_server2, "server2", "", CVAR_ARCHIVE },
	{ &ui_server3, "server3", "", CVAR_ARCHIVE },
	{ &ui_server4, "server4", "", CVAR_ARCHIVE },
	{ &ui_server5, "server5", "", CVAR_ARCHIVE },
	{ &ui_server6, "server6", "", CVAR_ARCHIVE },
	{ &ui_server7, "server7", "", CVAR_ARCHIVE },
	{ &ui_server8, "server8", "", CVAR_ARCHIVE },
	{ &ui_server9, "server9", "", CVAR_ARCHIVE },
	{ &ui_server10, "server10", "", CVAR_ARCHIVE },
	{ &ui_server11, "server11", "", CVAR_ARCHIVE },
	{ &ui_server12, "server12", "", CVAR_ARCHIVE },
	{ &ui_server13, "server13", "", CVAR_ARCHIVE },
	{ &ui_server14, "server14", "", CVAR_ARCHIVE },
	{ &ui_server15, "server15", "", CVAR_ARCHIVE },
	{ &ui_server16, "server16", "", CVAR_ARCHIVE },
	//{ &ui_cdkeychecked, "ui_cdkeychecked", "0", CVAR_ROM },
	{ &ui_new, "ui_new", "0", CVAR_TEMP },
	{ &ui_debug, "ui_debug", "0", CVAR_TEMP },
	{ &ui_initialized, "ui_initialized", "0", CVAR_TEMP },
	{ &ui_teamName, "ui_teamName", "NATO", CVAR_ARCHIVE },
	{ &ui_opponentName, "ui_opponentName", "Warsaw Pact", CVAR_ARCHIVE },
	{ &ui_redteam, "ui_redteam", "Warsaw Pact", CVAR_ARCHIVE },
	{ &ui_blueteam, "ui_blueteam", "NATO", CVAR_ARCHIVE },
	{ &ui_dedicated, "ui_dedicated", "0", CVAR_ARCHIVE },
	{ &ui_gameType, "ui_gametype", "0", CVAR_ARCHIVE },
	{ &ui_joinGameType, "ui_joinGametype", "0", CVAR_ARCHIVE },
	{ &ui_netGameType, "ui_netGametype", "0", CVAR_ARCHIVE },
//	{ &ui_netGameset, "ui_netGameset", "0", CVAR_ARCHIVE },
	{ &ui_actualNetGameType, "ui_actualNetGametype", "0", CVAR_ARCHIVE },
	{ &ui_redteam1, "ui_redteam1", "0", CVAR_ARCHIVE },
	{ &ui_redteam2, "ui_redteam2", "0", CVAR_ARCHIVE },
	{ &ui_redteam3, "ui_redteam3", "0", CVAR_ARCHIVE },
	{ &ui_redteam4, "ui_redteam4", "0", CVAR_ARCHIVE },
	{ &ui_redteam5, "ui_redteam5", "0", CVAR_ARCHIVE },
	{ &ui_blueteam1, "ui_blueteam1", "0", CVAR_ARCHIVE },
	{ &ui_blueteam2, "ui_blueteam2", "0", CVAR_ARCHIVE },
	{ &ui_blueteam3, "ui_blueteam3", "0", CVAR_ARCHIVE },
	{ &ui_blueteam4, "ui_blueteam4", "0", CVAR_ARCHIVE },
	{ &ui_blueteam5, "ui_blueteam5", "0", CVAR_ARCHIVE },
	{ &ui_netSource, "ui_netSource", "0", CVAR_ARCHIVE },
	{ &ui_menuFiles, "ui_menuFiles", "ui/menus.txt", CVAR_ARCHIVE },
	{ &ui_currentTier, "ui_currentTier", "0", CVAR_ARCHIVE },
	{ &ui_currentMap, "ui_currentMap", "0", CVAR_ARCHIVE },
	{ &ui_currentNetMap, "ui_currentNetMap", "0", CVAR_ARCHIVE },
	{ &ui_mapIndex, "ui_mapIndex", "0", CVAR_ARCHIVE },
	{ &ui_currentOpponent, "ui_currentOpponent", "0", CVAR_ARCHIVE },
	{ &ui_selectedPlayer, "cg_selectedPlayer", "0", CVAR_ARCHIVE},
	{ &ui_selectedPlayerName, "cg_selectedPlayerName", "", CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_0, "ui_lastServerRefresh_0", "", CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_1, "ui_lastServerRefresh_1", "", CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_2, "ui_lastServerRefresh_2", "", CVAR_ARCHIVE},
	{ &ui_lastServerRefresh_3, "ui_lastServerRefresh_3", "", CVAR_ARCHIVE},
	{ &ui_singlePlayerActive, "ui_singlePlayerActive", "0", 0},
	{ &ui_scoreAccuracy, "ui_scoreAccuracy", "0", CVAR_ARCHIVE},
	{ &ui_scoreImpressives, "ui_scoreImpressives", "0", CVAR_ARCHIVE},
	{ &ui_scoreExcellents, "ui_scoreExcellents", "0", CVAR_ARCHIVE},
	{ &ui_scoreCaptures, "ui_scoreCaptures", "0", CVAR_ARCHIVE},
	{ &ui_scoreDefends, "ui_scoreDefends", "0", CVAR_ARCHIVE},
	{ &ui_scoreAssists, "ui_scoreAssists", "0", CVAR_ARCHIVE},
	{ &ui_scoreGauntlets, "ui_scoreGauntlets", "0",CVAR_ARCHIVE},
	{ &ui_scoreScore, "ui_scoreScore", "0", CVAR_ARCHIVE},
	{ &ui_scorePerfect, "ui_scorePerfect", "0", CVAR_ARCHIVE},
	{ &ui_scoreTeam, "ui_scoreTeam", "0 to 0", CVAR_ARCHIVE},
	{ &ui_scoreBase, "ui_scoreBase", "0", CVAR_ARCHIVE},
	{ &ui_scoreTime, "ui_scoreTime", "00:00", CVAR_ARCHIVE},
	{ &ui_scoreTimeBonus, "ui_scoreTimeBonus", "0", CVAR_ARCHIVE},
	{ &ui_scoreSkillBonus, "ui_scoreSkillBonus", "0", CVAR_ARCHIVE},
	{ &ui_scoreShutoutBonus, "ui_scoreShutoutBonus", "0", CVAR_ARCHIVE},
	{ &ui_fragLimit, "ui_fragLimit", "10", 0},
	{ &ui_captureLimit, "ui_captureLimit", "5", 0},
	{ &ui_smallFont, "ui_smallFont", "0.25", CVAR_ARCHIVE},
	{ &ui_bigFont, "ui_bigFont", "0.4", CVAR_ARCHIVE},
	{ &ui_findPlayer, "ui_findPlayer", "Sarge", CVAR_ARCHIVE},
	{ &ui_Q3Model, "ui_q3model", "0", CVAR_ARCHIVE},
	{ &ui_hudFiles, "cg_hudFiles", "ui/hud.txt", CVAR_ARCHIVE},
	{ &ui_recordSPDemo, "ui_recordSPDemo", "0", CVAR_ARCHIVE},
	{ &ui_teamArenaFirstRun, "ui_teamArenaFirstRun", "0", CVAR_ARCHIVE},
	{ &ui_realWarmUp, "g_warmup", "20", CVAR_ARCHIVE},
	{ &ui_realCaptureLimit, "capturelimit", "8", CVAR_SERVERINFO | CVAR_ARCHIVE | CVAR_NORESTART},
	{ &ui_serverStatusTimeOut, "ui_serverStatusTimeOut", "7000", CVAR_ARCHIVE},

};

// bk001129 - made static to avoid aliasing
static int		cvarTableSize = sizeof(cvarTable) / sizeof(cvarTable[0]);


/*
=================
UI_RegisterCvars
=================
*/
void UI_RegisterCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		Cvar_Register( cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags );
	}
}

/*
=================
UI_UpdateCvars
=================
*/
void UI_UpdateCvars( void ) {
	int			i;
	cvarTable_t	*cv;

	for ( i = 0, cv = cvarTable ; i < cvarTableSize ; i++, cv++ ) {
		Cvar_Update( cv->vmCvar );
	}
}


/*
=================
ArenaServers_StopRefresh
=================
*/
static void UI_StopServerRefresh( void )
{
	int count;

	if (!uiInfo.serverStatus.refreshActive) {
		// not currently refreshing
		return;
	}
	uiInfo.serverStatus.refreshActive = false;
	Com_Printf("%d servers listed in browser with %d players.\n",
					uiInfo.serverStatus.numDisplayServers,
					uiInfo.serverStatus.numPlayersOnServers);
	count = LAN_GetServerCount(ui_netSource.integer);
	if (count - uiInfo.serverStatus.numDisplayServers > 0) {
		Com_Printf("%d servers not listed due to packet loss or pings higher than %d\n",
						count - uiInfo.serverStatus.numDisplayServers,
						Cvar_VariableValue("cl_maxPing"));
	}

}


/*
=================
UI_DoServerRefresh
=================
*/
static void UI_DoServerRefresh( void )
{
	bool wait = false;

	if (!uiInfo.serverStatus.refreshActive) {
		return;
	}
	if (ui_netSource.integer != AS_FAVORITES) {
		if (ui_netSource.integer == AS_LOCAL) {
			if (!LAN_GetServerCount(ui_netSource.integer)) {
				wait = true;
			}
		} else {
			if (LAN_GetServerCount(ui_netSource.integer) < 0) {
				wait = true;
			}
		}
	}

	if (uiInfo.uiUtils.dc_->realTime_ < uiInfo.serverStatus.refreshtime) {
		if (wait) {
			return;
		}
	}

	// if still trying to retrieve pings
	if (LAN_UpdateVisiblePings(ui_netSource.integer)) {
		uiInfo.serverStatus.refreshtime = uiInfo.uiUtils.dc_->realTime_ + 1000;
	} else if (!wait) {
		// get the last servers in the list
		UI_BuildServerDisplayList(2);
		// stop the refresh
		UI_StopServerRefresh();
	}
	//
	UI_BuildServerDisplayList(0);
}

/*
=================
UI_StartServerRefresh
=================
*/
static void UI_StartServerRefresh(bool full)
{
	//int		i;
	//char	*ptr;

	qtime_t q;
	Com_RealTime(&q);
 	Cvar_Set( va("ui_lastServerRefresh_%i", ui_netSource.integer), va("%s-%i, %i at %i:%i", MonthAbbrev[q.tm_mon],q.tm_mday, 1900+q.tm_year,q.tm_hour,q.tm_min));

	if (!full) {
		UI_UpdatePendingPings();
		return;
	}

	uiInfo.serverStatus.refreshActive = true;
	uiInfo.serverStatus.nextDisplayRefresh = uiInfo.uiUtils.dc_->realTime_ + 1000;
	// clear number of displayed servers
	uiInfo.serverStatus.numDisplayServers = 0;
	uiInfo.serverStatus.numPlayersOnServers = 0;
	// mark all servers as visible so we store ping updates for them
	LAN_MarkServerVisible(ui_netSource.integer, -1, true);
	// reset all the pings
	LAN_ResetPings(ui_netSource.integer);
	//
	if( ui_netSource.integer == AS_LOCAL ) {
		Cbuf_ExecuteText( EXEC_NOW, "localservers\n" );
		uiInfo.serverStatus.refreshtime = uiInfo.uiUtils.dc_->realTime_ + 1000;
		return;
	}

	uiInfo.serverStatus.refreshtime = uiInfo.uiUtils.dc_->realTime_ + 5000;
	//if( ui_netSource.integer == AS_GLOBAL || ui_netSource.integer == AS_MPLAYER ) {
	//	if( ui_netSource.integer == AS_GLOBAL ) {
	//		i = 0;
	//	}
	//	else {
	//		i = 1;
	//	}

	//	ptr = UI_Cvar_VariableString("debug_protocol");
	//	if (strlen(ptr)) {
	//		Cbuf_ExecuteText( EXEC_NOW, va( "globalservers %d %s full empty\n", i, ptr));
	//	}
	//	else {
	//		Cbuf_ExecuteText( EXEC_NOW, va( "globalservers %d %d full empty\n", i, Cvar_VariableValue( "protocol" ) ) );
	//	}
	//}
}



