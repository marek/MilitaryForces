/*
 * $Id: ui_local.h,v 1.10 2005-11-26 10:06:29 thebjoern Exp $
*/
//
#ifndef __UI_LOCAL_H__
#define __UI_LOCAL_H__

#include "../game/q_shared.h"
#include "../cgame/tr_types.h"
#include "ui_public.h"
#include "keycodes.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "ui_shared.h"
#include "ui.h"
#include "ui_displaycontext_ui.h"
#include "ui_utils_ui.h"
#include "../renderercommon/tr_public.h"

struct UI_Utils;

// global display context

//extern displayContextDef_t * DC;
//extern UI_DisplayContext* gDC;

extern refexport_t	refExport;


extern vmCvar_t	ui_ffa_fraglimit;
extern vmCvar_t	ui_ffa_timelimit;

extern vmCvar_t	ui_tourney_fraglimit;
extern vmCvar_t	ui_tourney_timelimit;

extern vmCvar_t	ui_team_fraglimit;
extern vmCvar_t	ui_team_timelimit;
extern vmCvar_t	ui_team_friendly;

extern vmCvar_t	ui_ctf_capturelimit;
extern vmCvar_t	ui_ctf_timelimit;
extern vmCvar_t	ui_ctf_friendly;

extern vmCvar_t	ui_arenasFile;
//extern vmCvar_t	ui_botsFile;
extern vmCvar_t	ui_spScores1;
extern vmCvar_t	ui_spScores2;
extern vmCvar_t	ui_spScores3;
extern vmCvar_t	ui_spScores4;
extern vmCvar_t	ui_spScores5;
extern vmCvar_t	ui_spAwards;
extern vmCvar_t	ui_spVideos;
extern vmCvar_t	ui_spSkill;

extern vmCvar_t	ui_spSelection;

extern vmCvar_t	ui_browserMaster;
extern vmCvar_t	ui_browserGameType;
extern vmCvar_t	ui_browserSortKey;
extern vmCvar_t	ui_browserShowFull;
extern vmCvar_t	ui_browserShowEmpty;

extern vmCvar_t	ui_brassTime;
extern vmCvar_t	ui_drawCrosshair;
extern vmCvar_t	ui_drawCrosshairNames;
extern vmCvar_t	ui_marks;

extern vmCvar_t	ui_server1;
extern vmCvar_t	ui_server2;
extern vmCvar_t	ui_server3;
extern vmCvar_t	ui_server4;
extern vmCvar_t	ui_server5;
extern vmCvar_t	ui_server6;
extern vmCvar_t	ui_server7;
extern vmCvar_t	ui_server8;
extern vmCvar_t	ui_server9;
extern vmCvar_t	ui_server10;
extern vmCvar_t	ui_server11;
extern vmCvar_t	ui_server12;
extern vmCvar_t	ui_server13;
extern vmCvar_t	ui_server14;
extern vmCvar_t	ui_server15;
extern vmCvar_t	ui_server16;

//extern vmCvar_t	ui_cdkey;
//extern vmCvar_t	ui_cdkeychecked;

extern vmCvar_t	ui_captureLimit;
extern vmCvar_t	ui_fragLimit;
extern vmCvar_t	ui_gameType;
extern vmCvar_t	ui_netGameType;
//extern vmCvar_t	ui_netGameset;
extern vmCvar_t	ui_actualNetGameType;
extern vmCvar_t	ui_joinGameType;
extern vmCvar_t	ui_netSource;
extern vmCvar_t	ui_serverFilterType;
extern vmCvar_t	ui_dedicated;
extern vmCvar_t	ui_opponentName;
extern vmCvar_t	ui_menuFiles;
extern vmCvar_t	ui_currentTier;
extern vmCvar_t	ui_currentMap;
extern vmCvar_t	ui_currentNetMap;
extern vmCvar_t	ui_mapIndex;
extern vmCvar_t	ui_currentOpponent;
extern vmCvar_t	ui_selectedPlayer;
extern vmCvar_t	ui_selectedPlayerName;
extern vmCvar_t	ui_lastServerRefresh_0;
extern vmCvar_t	ui_lastServerRefresh_1;
extern vmCvar_t	ui_lastServerRefresh_2;
extern vmCvar_t	ui_lastServerRefresh_3;
extern vmCvar_t	ui_singlePlayerActive;
extern vmCvar_t	ui_scoreAccuracy;
extern vmCvar_t	ui_scoreImpressives;
extern vmCvar_t	ui_scoreExcellents;
extern vmCvar_t	ui_scoreDefends;
extern vmCvar_t	ui_scoreAssists;
extern vmCvar_t	ui_scoreGauntlets;
extern vmCvar_t	ui_scoreScore;
extern vmCvar_t	ui_scorePerfect;
extern vmCvar_t	ui_scoreTeam;
extern vmCvar_t	ui_scoreBase;
extern vmCvar_t	ui_scoreTimeBonus;
extern vmCvar_t	ui_scoreSkillBonus;
extern vmCvar_t	ui_scoreShutoutBonus;
extern vmCvar_t	ui_scoreTime;
extern vmCvar_t	ui_smallFont;
extern vmCvar_t	ui_bigFont;
extern vmCvar_t ui_serverStatusTimeOut;

//
// ui_qmenu.c
//

#define RCOLUMN_OFFSET			( BIGCHAR_WIDTH )
#define LCOLUMN_OFFSET			(-BIGCHAR_WIDTH )

inline constexpr int SLIDER_RANGE = 10;
#define	MAX_EDIT_LINE			256

inline constexpr int MAX_MENUDEPTH = 8;
#define MAX_MENUITEMS			96

inline constexpr int MTYPE_NULL = 0;
inline constexpr int MTYPE_SLIDER = 1;
inline constexpr int MTYPE_ACTION = 2;
inline constexpr int MTYPE_SPINCONTROL = 3;
inline constexpr int MTYPE_FIELD = 4;
inline constexpr int MTYPE_RADIOBUTTON = 5;
inline constexpr int MTYPE_BITMAP = 6;
inline constexpr int MTYPE_TEXT = 7;
inline constexpr int MTYPE_SCROLLLIST = 8;
inline constexpr int MTYPE_PTEXT = 9;
inline constexpr int MTYPE_BTEXT = 10;

inline constexpr int QMF_BLINK = 0x00000001;
inline constexpr int QMF_SMALLFONT = 0x00000002;
inline constexpr int QMF_LEFT_JUSTIFY = 0x00000004;
inline constexpr int QMF_CENTER_JUSTIFY = 0x00000008;
inline constexpr int QMF_RIGHT_JUSTIFY = 0x00000010;
inline constexpr int QMF_NUMBERSONLY = 0x00000020; // edit field is only numbers
inline constexpr int QMF_HIGHLIGHT = 0x00000040;
inline constexpr int QMF_HIGHLIGHT_IF_FOCUS = 0x00000080; // steady focus
inline constexpr int QMF_PULSEIFFOCUS = 0x00000100; // pulse if focus
inline constexpr int QMF_HASMOUSEFOCUS = 0x00000200;
inline constexpr int QMF_NOONOFFTEXT = 0x00000400;
inline constexpr int QMF_MOUSEONLY = 0x00000800; // only mouse input allowed
inline constexpr int QMF_HIDDEN = 0x00001000; // skips drawing
inline constexpr int QMF_GRAYED = 0x00002000; // grays and disables
inline constexpr int QMF_INACTIVE = 0x00004000; // disables any input
inline constexpr int QMF_NODEFAULTINIT = 0x00008000; // skip default initialization
inline constexpr int QMF_OWNERDRAW = 0x00010000;
inline constexpr int QMF_PULSE = 0x00020000;
inline constexpr int QMF_LOWERCASE = 0x00040000; // edit field is all lower case
inline constexpr int QMF_UPPERCASE = 0x00080000; // edit field is all upper case
inline constexpr int QMF_SILENT = 0x00100000;

// callback notifications
inline constexpr int QM_GOTFOCUS = 1;
inline constexpr int QM_LOSTFOCUS = 2;
inline constexpr int QM_ACTIVATED = 3;

struct _tag_menuframework
{
	int	cursor;
	int cursor_prev;

	int	nitems;
	void *items[MAX_MENUITEMS];

	void (*draw) ();
	sfxHandle_t (*key) (int key);

	bool	wrapAround;
	bool	fullscreen;
	bool	showlogo;
};
using menuframework_s = _tag_menuframework;

struct menucommon_s
{
	int type;
	const char *name;
	int	id;
	int x, y;
	int left;
	int	top;
	int	right;
	int	bottom;
	menuframework_s *parent;
	int menuPosition;
	unsigned flags;

	void (*callback)( void *self, int event );
	void (*statusbar)( void *self );
	void (*ownerdraw)( void *self );
};

struct mfield_t {
	int		cursor;
	int		scroll;
	int		widthInChars;
	char	buffer[MAX_EDIT_LINE];
	int		maxchars;
};

struct menufield_s
{
	menucommon_s	generic;
	mfield_t		field;
};

struct menuslider_s 
{
	menucommon_s generic;

	float minvalue;
	float maxvalue;
	float curvalue;

	float range;
};

struct menulist_s
{
	menucommon_s generic;

	int	oldvalue;
	int curvalue;
	int	numitems;
	int	top;
		
	const char **itemnames;

	int width;
	int height;
	int	columns;
	int	seperation;
};

struct menuaction_s
{
	menucommon_s generic;
};

struct menuradiobutton_s
{
	menucommon_s generic;
	int curvalue;
};

struct menubitmap_s
{
	menucommon_s	generic;
	char*			focuspic;	
	char*			errorpic;
	qhandle_t		shader;
	qhandle_t		focusshader;
	int				width;
	int				height;
	float*			focuscolor;
};

struct menutext_s
{
	menucommon_s	generic;
	char*			string;
	int				style;
	float*			color;
};

extern void			Menu_Cache( );
extern void			Menu_Focus( menucommon_s *m );
extern void			Menu_AddItem( menuframework_s *menu, void *item );
extern void			Menu_AdjustCursor( menuframework_s *menu, int dir );
extern void			Menu_Draw( menuframework_s *menu );
extern void			*Menu_ItemAtCursor( menuframework_s *m );
extern sfxHandle_t	Menu_ActivateItem( menuframework_s *s, menucommon_s* item );
extern void			Menu_SetCursor( menuframework_s *s, int cursor );
extern void			Menu_SetCursorToItem( menuframework_s *m, void* ptr );
extern sfxHandle_t	Menu_DefaultKey( menuframework_s *s, int key );
extern void			Bitmap_Init( menubitmap_s *b );
extern void			Bitmap_Draw( menubitmap_s *b );
extern void			ScrollList_Draw( menulist_s *l );
extern sfxHandle_t	ScrollList_Key( menulist_s *l, int key );
extern sfxHandle_t	menu_in_sound;
extern sfxHandle_t	menu_move_sound;
extern sfxHandle_t	menu_out_sound;
extern sfxHandle_t	menu_buzz_sound;
extern sfxHandle_t	menu_null_sound;
extern sfxHandle_t	weaponChangeSound;
extern vec4_t		menu_text_color;
extern vec4_t		menu_grayed_color;
extern vec4_t		menu_dark_color;
extern vec4_t		menu_highlight_color;
extern vec4_t		menu_red_color;
extern vec4_t		menu_black_color;
extern vec4_t		menu_dim_color;
extern vec4_t		color_black;
extern vec4_t		color_white;
extern vec4_t		color_yellow;
extern vec4_t		color_blue;
extern vec4_t		color_orange;
extern vec4_t		color_red;
extern vec4_t		color_dim;
extern vec4_t		name_color;
extern vec4_t		list_color;
extern vec4_t		listbar_color;
extern vec4_t		text_color_disabled; 
extern vec4_t		text_color_normal;
extern vec4_t		text_color_highlight;

extern char	*ui_medalNames[];
extern char	*ui_medalPicNames[];
extern char	*ui_medalSounds[];

//
// ui_mfield.c
//
extern void			MField_Clear( mfield_t *edit );
extern void			MField_KeyDownEvent( mfield_t *edit, int key );
extern void			MField_CharEvent( mfield_t *edit, int ch );
extern void			MField_Draw( mfield_t *edit, int x, int y, int style, vec4_t color );
extern void			MenuField_Init( menufield_s* m );
extern void			MenuField_Draw( menufield_s *f );
extern sfxHandle_t	MenuField_Key( menufield_s* m, int* key );

//
// ui_main.c
//
void UI_Report();
void UI_Load();
void UI_LoadMenus(const char *menuFile, bool reset);
void _UI_SetActiveMenu( UserInterface::MenuCommand menu );
int UI_AdjustTimeByGame(int time);
void UI_ShowPostGame(bool newHigh);
void UI_ClearScores();
void UI_LoadArenas();

//
// ui_menu.c
//
extern void MainMenu_Cache( );
extern void UI_MainMenu();
extern void UI_RegisterCvars( );
extern void UI_UpdateCvars( );

//
// ui_credits.c
//
extern void UI_CreditMenu( );

//
// ui_ingame.c
//
extern void InGame_Cache( );
extern void UI_InGameMenu();

//
// ui_confirm.c
//
extern void ConfirmMenu_Cache( );
extern void UI_ConfirmMenu( const char *question, void (*draw)( ), void (*action)( bool result ) );

//
// ui_setup.c
//
extern void UI_SetupMenu_Cache( );
extern void UI_SetupMenu();

//
// ui_team.c
//
extern void UI_TeamMainMenu( );
extern void TeamMain_Cache( );

//
// ui_connect.c
//
extern void UI_DrawConnectScreen( bool overlay );

//
// ui_controls2.c
//
extern void UI_ControlsMenu( );
//extern void Controls_Cache( void );

//
// ui_demo2.c
//
extern void UI_DemosMenu( );
extern void Demos_Cache( );

//
// ui_cinematics.c
//
extern void UI_CinematicsMenu( );
extern void UI_CinematicsMenu_f( );
extern void UI_CinematicsMenu_Cache( );

//
// ui_mods.c
//
extern void UI_ModsMenu( );
extern void UI_ModsMenu_Cache( );

//
// ui_cdkey.c
//
//extern void UI_CDKeyMenu( void );
//extern void UI_CDKeyMenu_Cache( void );
//extern void UI_CDKeyMenu_f( void );

//
// ui_playermodel.c
//
extern void UI_PlayerModelMenu( );
extern void PlayerModel_Cache( );

//
// ui_playersettings.c
//
extern void UI_PlayerSettingsMenu( );
extern void PlayerSettings_Cache( );

//
// ui_preferences.c
//
extern void UI_PreferencesMenu( );
extern void Preferences_Cache( );

//
// ui_specifyleague.c
//
extern void UI_SpecifyLeagueMenu( );
extern void SpecifyLeague_Cache( );

//
// ui_specifyserver.c
//
extern void UI_SpecifyServerMenu( );
extern void SpecifyServer_Cache( );

//
// ui_servers2.c
//
inline constexpr int MAX_FAVORITESERVERS = 16;

extern void UI_ArenaServersMenu( );
extern void ArenaServers_Cache( );

//
// ui_startserver.c
//
extern void UI_StartServerMenu( bool multiplayer );
extern void StartServer_Cache( );
extern void ServerOptions_Cache( );
extern void UI_BotSelectMenu( char *bot );
extern void UI_BotSelectMenu_Cache( );

//
// ui_serverinfo.c
//
extern void UI_ServerInfoMenu( );
extern void ServerInfo_Cache( );

//
// ui_video.c
//
extern void UI_GraphicsOptionsMenu( );
extern void GraphicsOptions_Cache( );
extern void DriverInfo_Cache( );

//
// ui_players.c
//

//FIXME ripped from cg_local.h
struct lerpFrame_t {
	int			oldFrame;
	int			oldFrameTime;		// time when ->oldFrame was exactly on

	int			frame;
	int			frameTime;			// time when ->frame will be exactly on

	float		backlerp;

	float		yawAngle;
	bool	yawing;
	float		pitchAngle;
	bool	pitching;

	int			animationNumber;	// may include ANIM_TOGGLEBIT
	int			animationTime;		// time when the first frame of the animation will be exact
};

struct playerInfo_t {
	// model info
	qhandle_t		legsModel;
	qhandle_t		legsSkin;
	lerpFrame_t		legs;

	qhandle_t		torsoModel;
	qhandle_t		torsoSkin;
	lerpFrame_t		torso;

	qhandle_t		headModel;
	qhandle_t		headSkin;

	qhandle_t		weaponModel;
	qhandle_t		barrelModel;
	qhandle_t		flashModel;
	vec3_t			flashDlightColor;
	int				muzzleFlashTime;

	// currently in use drawing parms
	vec3_t			viewAngles;
	vec3_t			moveAngles;
	weapon_t		currentWeapon;
	int				legsAnim;
	int				torsoAnim;

	// animation vars
	weapon_t		weapon;
	weapon_t		lastWeapon;
	weapon_t		pendingWeapon;
	int				weaponTimer;
	int				pendingLegsAnim;
	int				torsoAnimationTimer;

	int				pendingTorsoAnim;
	int				legsAnimationTimer;

	bool		chat;
	bool		newModel;

	bool		barrelSpinning;
	float			barrelAngle;
	int				barrelTime;

	int				realWeapon;
};

void UI_DrawPlayer( float x, float y, float w, float h, playerInfo_t *pi, int time );
void UI_PlayerInfo_SetModel( playerInfo_t *pi, const char *model, const char *headmodel, char *teamName );
void UI_PlayerInfo_SetInfo( playerInfo_t *pi, int legsAnim, int torsoAnim, vec3_t viewAngles, vec3_t moveAngles, weapon_t weaponNum, bool chat );
bool UI_RegisterClientModelname( playerInfo_t *pi, const char *modelSkinName , const char *headName, const char *teamName);

//
// ui_atoms.c
//
// this is only used in the old ui, the New ui has it's own version
struct uiStatic_t {
	int					frametime;
	int					realtime;
	int					cursorx;
	int					cursory;
	glconfig_t 	glconfig;
	bool		debug;
	qhandle_t		whiteShader;
	qhandle_t		menuBackShader;
	qhandle_t		menuBackShader2;
	qhandle_t		menuBackNoLogoShader;
	qhandle_t		charset;
	qhandle_t		charsetProp;
	qhandle_t		charsetPropGlow;
	qhandle_t		charsetPropB;
	qhandle_t		cursor;
	qhandle_t		rb_on;
	qhandle_t		rb_off;
	float				scale;
	float				bias;
	bool		demoversion;
	bool		firstdraw;
};


// New ui stuff
inline constexpr int UI_NUMFX = 7;
inline constexpr int MAX_HEADS = 64;
inline constexpr int MAX_ALIASES = 64;
inline constexpr int MAX_HEADNAME = 32;
inline constexpr int MAX_TEAMS = 64;
inline constexpr int MAX_GAMETYPES = 16;
inline constexpr int MAX_MAPS = 128;
inline constexpr int MAX_SPMAPS = 16;
inline constexpr int PLAYERS_PER_TEAM = 5;
inline constexpr int MAX_ADDRESSLENGTH = 64;
inline constexpr int MAX_HOSTNAMELENGTH = 22;
inline constexpr int MAX_MAPNAMELENGTH = 16;
inline constexpr int MAX_STATUSLENGTH = 64;
inline constexpr int MAX_LISTBOXWIDTH = 59;
inline constexpr double UI_FONT_THRESHOLD = 0.1;
inline constexpr int MAX_DISPLAY_SERVERS = 2048;
inline constexpr int MAX_SERVERSTATUS_LINES = 128;
inline constexpr int MAX_SERVERSTATUS_TEXT = 1024;
inline constexpr int MAX_FOUNDPLAYER_SERVERS = 16;
inline constexpr int TEAM_MEMBERS = 5;
inline constexpr int GAMES_ALL = 0;
inline constexpr int GAMES_FFA = 1;
inline constexpr int GAMES_TEAMPLAY = 2;
inline constexpr int GAMES_TOURNEY = 3;
inline constexpr int GAMES_CTF = 4;
inline constexpr int MAPS_PER_TIER = 3;
inline constexpr int MAX_TIERS = 16;
inline constexpr int MAX_MODS = 64;
inline constexpr int MAX_DEMOS = 256;
inline constexpr int MAX_MOVIES = 256;
inline constexpr int MAX_PLAYERMODELS = 256;


struct characterInfo {
  const char *name;
	const char *imageName;
  qhandle_t headImage;
	const char *base;
	bool active;
	int reference;
};

struct aliasInfo {
	const char *name;
	const char *ai;
	const char *action;
};

struct teamInfo {
  const char *teamName;
	const char *imageName;
	const char *teamMembers[TEAM_MEMBERS];
  qhandle_t teamIcon;
  qhandle_t teamIcon_Metal;
  qhandle_t teamIcon_Name;
	int cinematic;
};

struct gameTypeInfo {
  const char *gameType;
  int gtEnum;
};

struct mapInfo {
  const char *mapName;
  const char *mapLoadName;
	const char *imageName;
	const char *opponentName;
	int teamMembers;
  int typeBits;
	int cinematic;
	int timeToBeat[MAX_GAMETYPES];
	qhandle_t levelShot;
	bool active;
};

struct tierInfo {
	const char *tierName;
	const char *maps[MAPS_PER_TIER];
	int gameTypes[MAPS_PER_TIER];
	qhandle_t mapHandles[MAPS_PER_TIER];
};

struct serverFilter_s {
	const char *description;
	const char *basedir;
};
using serverFilter_t = serverFilter_s;

struct pinglist_t {
	char	adrstr[MAX_ADDRESSLENGTH];
	int		start;
};


struct serverStatus_s {
	pinglist_t pingList[MAX_PINGREQUESTS];
	int		numqueriedservers;
	int		currentping;
	int		nextpingtime;
	int		maxservers;
	int		refreshtime;
	int		numServers;
	int		sortKey;
	int		sortDir;
	int		lastCount;
	bool refreshActive;
	int		currentServer;
	int		displayServers[MAX_DISPLAY_SERVERS];
	int		numDisplayServers;
	int		numPlayersOnServers;
	int		nextDisplayRefresh;
	int		nextSortTime;
	qhandle_t currentServerPreview;
	int		currentServerCinematic;
	int		motdLen;
	int		motdWidth;
	int		motdPaintX;
	int		motdPaintX2;
	int		motdOffset;
	int		motdTime;
	char	motd[MAX_STRING_CHARS];
};
using serverStatus_t = serverStatus_s;


struct pendingServer_t {
	char		adrstr[MAX_ADDRESSLENGTH];
	char		name[MAX_ADDRESSLENGTH];
	int			startTime;
	int			serverNum;
	bool	valid;
};

struct pendingServerStatus_t
{
	int num;
	pendingServer_t server[MAX_SERVERSTATUSREQUESTS];
};

struct serverStatusInfo_t
{
	char address[MAX_ADDRESSLENGTH];
	char *lines[MAX_SERVERSTATUS_LINES][4];
	char text[MAX_SERVERSTATUS_TEXT];
	char pings[MAX_CLIENTS * 3];
	int numLines;
};

struct modInfo_t
{
	const char *modName;
	const char *modDescr;
};

inline constexpr int MAX_CREDITS_LINES = 128; // max character pointers to allocate

struct uiInfo_t
{
	//displayContextDef_t uiDC;
	//UI_DisplayContextUI dcUI;
	UI_UtilsUI uiUtils;
	int newHighScoreTime;
	int newBestTime;
	int showPostGameTime;
	bool newHighScore;
	bool demoAvailable;
	bool soundHighScore;
	
	int characterCount;
	int botIndex;
	characterInfo characterList[MAX_HEADS];

	int aliasCount;
	aliasInfo aliasList[MAX_ALIASES];

	int teamCount;
	teamInfo teamList[MAX_TEAMS];

	int numGameTypes;
	gameTypeInfo gameTypes[MAX_GAMETYPES];

	int numJoinGameTypes;
	gameTypeInfo joinGameTypes[MAX_GAMETYPES];

	int redBlue;
	int playerCount;
	int myTeamCount;
	int teamIndex;
	int playerRefresh;
	int playerIndex;
	int playerNumber; 
	int teamLeader;
	char playerNames[MAX_CLIENTS][MAX_NAME_LENGTH];
	char teamNames[MAX_CLIENTS][MAX_NAME_LENGTH];
	int teamClientNums[MAX_CLIENTS];

	int mapCount;
	mapInfo mapList[MAX_MAPS];

	int tierCount;
	tierInfo tierList[MAX_TIERS];

	int skillIndex;

	modInfo_t modList[MAX_MODS];
	int modCount;
	int modIndex;

	const char *demoList[MAX_DEMOS];
	int demoCount;
	int demoIndex;

	const char *movieList[MAX_MOVIES];
	int movieCount;
	int movieIndex;
	int previewMovie;

	serverStatus_t serverStatus;

	// for the showing the status of a server
	char serverStatusAddress[MAX_ADDRESSLENGTH];
	serverStatusInfo_t serverStatusInfo;
	int nextServerStatusRefresh;

	// to retrieve the status of server to find a player
	pendingServerStatus_t pendingServerStatus;
	char findPlayerName[MAX_STRING_CHARS];
	char foundPlayerServerAddresses[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	char foundPlayerServerNames[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	int currentFoundPlayerServer;
	int numFoundPlayerServers;
	int nextFindPlayerRefresh;

	int currentCrosshair;
	int startPostGameTime;
	sfxHandle_t newHighScoreSound;

	int q3HeadCount;
	char q3HeadNames[MAX_PLAYERMODELS][64];
	qhandle_t q3HeadIcons[MAX_PLAYERMODELS];
	int q3SelectedHead;

	int effectsColor;

	bool inGameLoad;

	chat_t customChat;	// custom MFQ3 chat console

	void * pCreditsData;					// credits data block
	char * pCredits[ MAX_CREDITS_LINES ];	// credits line list
	int creditLines;						// active lines in the list

};

extern uiInfo_t uiInfo;


extern void			UI_Init( );
extern void			UI_Shutdown( );
extern void			UI_KeyEvent( int key );
extern void			UI_MouseEvent( int dx, int dy );
extern void			UI_Refresh( int realtime );
extern bool		UI_ConsoleCommand( int realTime );
extern float		UI_ClampCvar( float min, float max, float value );
extern void			UI_DrawNamedPic( float x, float y, float width, float height, const char *picname );
extern void			UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ); 
extern void			UI_FillRect( float x, float y, float width, float height, const float *color );
extern void			UI_DrawRect( float x, float y, float width, float height, const float *color );
extern void     UI_DrawTopBottom(float x, float y, float w, float h);
extern void     UI_DrawSides(float x, float y, float w, float h);
extern void			UI_UpdateScreen( );
extern void			UI_SetColor( const float *rgba );
extern void			UI_LerpColor(vec4_t a, vec4_t b, vec4_t c, float t);
extern void			UI_DrawBannerString( int x, int y, const char* str, int style, vec4_t color );
extern float		UI_ProportionalSizeScale( int style );
extern void			UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color );
extern int			UI_ProportionalStringWidth( const char* str );
extern void			UI_DrawString( int x, int y, const char* str, int style, vec4_t color );
extern void			UI_DrawChar( int x, int y, int ch, int style, vec4_t color );
extern bool 	UI_CursorInRect (int x, int y, int width, int height);
extern void			UI_AdjustFrom640( float *x, float *y, float *w, float *h );
extern void			UI_DrawTextBox (int x, int y, int width, int lines);
extern bool		UI_IsFullscreen( );
extern void			UI_SetActiveMenu( UserInterface::MenuCommand menu );
extern void			UI_PushMenu ( menuframework_s *menu );
extern void			UI_PopMenu ();
extern void			UI_ForceMenuOff ();
extern char			*UI_Argv( int arg );
extern char			*UI_Cvar_VariableString( const char *var_name );
extern void			UI_Refresh( int time );
extern void			UI_KeyEvent( int key );
extern void			UI_StartDemoLoop( );
extern bool		m_entersound;
void UI_LoadBestScores(const char *map, int game);
extern uiStatic_t	uis;

//
// ui_spLevel.c
//
void UI_SPLevelMenu_Cache( );
void UI_SPLevelMenu( );
void UI_SPLevelMenu_f( );
void UI_SPLevelMenu_ReInit( );

//
// ui_spArena.c
//
void UI_SPArena_Start( const char *arenaInfo );

//
// ui_spPostgame.c
//
void UI_SPPostgameMenu_Cache( );
void UI_SPPostgameMenu_f( );

//
// ui_spSkill.c
//
void UI_SPSkillMenu( const char *arenaInfo );
void UI_SPSkillMenu_Cache( );

// system calls
int Sys_Milliseconds ();
int	Cmd_Argc();
void Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength );
void Cbuf_ExecuteText (int exec_when, const char *text);
int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
void FS_FCloseFile( fileHandle_t f );
int FS_Read( void *buffer, int len, fileHandle_t f );
int FS_Write( const void *buffer, int len, fileHandle_t h );
int	FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
void SCR_UpdateScreen();
void S_StartLocalSound( sfxHandle_t sfxHandle, int channelNum );
sfxHandle_t	S_RegisterSound( const char *name, bool compressed );
void S_StopBackgroundTrack();
void S_StartBackgroundTrack( const char *intro, const char *loop );
void Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void Key_GetBindingBuf( int keynum, char *buf, int buflen );
void Key_SetBinding( int keynum, const char *binding );
bool Key_IsDown( int keynum );
bool Key_GetOverstrikeMode();
void Key_SetOverstrikeMode( bool state );
void Key_ClearStates();
int Key_GetCatcher();
void Key_SetCatcher( int catcher );
void CL_GetGlconfig( glconfig_t *glconfig );
int GetConfigString(int index, char *buf, int size);
int LAN_GetServerCount( int source );
void LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int LAN_GetServerPing( int source, int n );
void LAN_SaveServersToCache();
void LAN_LoadCachedServers();
void LAN_ResetPings(int source);
void LAN_MarkServerVisible(int source, int n, bool visible );
int LAN_ServerIsVisible(int source, int n );
bool LAN_UpdateVisiblePings(int source );
int LAN_AddServer(int source, const char *name, const char *address);
void LAN_RemoveServer(int source, const char *addr);
int LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
int LAN_GetServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
//void CLUI_GetCDKey( char *buf, int buflen );
//void CLUI_SetCDKey( char *buf );
int Com_RealTime(qtime_t *qtime);
int CIN_PlayCinematic( const char *arg, int x, int y, int w, int h, int systemBits );
e_status CIN_StopCinematic(int handle);
e_status CIN_RunCinematic (int handle);
void CIN_DrawCinematic (int handle);
void CIN_SetExtents (int handle, int x, int y, int w, int h);
//bool CL_CDKeyValidate( const char *key, const char *checksum );
void GetClientState( uiClientState_t *state );

//
// ui_addbots.c
//
void UI_AddBots_Cache( );
void UI_AddBotsMenu( );

//
// ui_removebots.c
//
void UI_RemoveBots_Cache( );
void UI_RemoveBotsMenu( );

//
// ui_teamorders.c
//
extern void UI_TeamOrdersMenu( );
extern void UI_TeamOrdersMenu_f( );
extern void UI_TeamOrdersMenu_Cache( );

//
// ui_loadconfig.c
//
void UI_LoadConfig_Cache( );
void UI_LoadConfigMenu( );

//
// ui_saveconfig.c
//
void UI_SaveConfigMenu_Cache( );
void UI_SaveConfigMenu( );

//
// ui_display.c
//
void UI_DisplayOptionsMenu_Cache( );
void UI_DisplayOptionsMenu( );

//
// ui_sound.c
//
void UI_SoundOptionsMenu_Cache( );
void UI_SoundOptionsMenu( );

//
// ui_network.c
//
void UI_NetworkOptionsMenu_Cache( );
void UI_NetworkOptionsMenu( );

//
// ui_gameinfo.c
//
enum awardType_t {
	AWARD_ACCURACY,
	AWARD_IMPRESSIVE,
	AWARD_EXCELLENT,
	AWARD_GAUNTLET,
	AWARD_FRAGS,
	AWARD_PERFECT
};

const char *UI_GetArenaInfoByNumber( int num );
const char *UI_GetArenaInfoByMap( const char *map );
const char *UI_GetSpecialArenaInfo( const char *tag );
int UI_GetNumArenas( );
int UI_GetNumSPArenas( );
int UI_GetNumSPTiers( );

//char *UI_GetBotInfoByNumber( int num );
//char *UI_GetBotInfoByName( const char *name );
//int UI_GetNumBots( void );
//void UI_LoadBots( void );
//char *UI_GetBotNameByNumber( int num );

void UI_GetBestScore( int level, int *score, int *skill );
void UI_SetBestScore( int level, int score );
int UI_TierCompleted( int levelWon );
bool UI_ShowTierVideo( int tier );
bool UI_CanShowTierVideo( int tier );
int  UI_GetCurrentGame( );
void UI_NewGame( );
void UI_LogAwardData( int award, int data );
int UI_GetAwardLevel( int award );

void UI_SPUnlock_f( );
void UI_SPUnlockMedals_f( );

void UI_InitGameinfo( );

//
// ui_login.c
//
void Login_Cache( );
void UI_LoginMenu( );

//
// ui_signup.c
//
void Signup_Cache( );
void UI_SignupMenu( );

//
// ui_rankstatus.c
//
void RankStatus_Cache( );
void UI_RankStatusMenu( );


// New ui 

#define ASSET_BACKGROUND "uiBackground"

// for tracking sp game info in Team Arena
struct postGameInfo_s {
	int score;
	int redScore;
	int blueScore;
	int perfects;
	int accuracy;
	int impressives;
	int excellents;
	int defends;
	int assists;
	int gauntlets;
	int	captures;
	int time;
	int timeBonus;
	int shutoutBonus;
	int skillBonus;
	int baseScore;
};
using postGameInfo_t = postGameInfo_s;

// MFQ3 util

unsigned int MF_UI_GetTeam( );
unsigned long MF_UI_GetGameset( bool asEnum );
unsigned long MF_UI_Gameset_StringToValue( char * pString, bool asEnum );

#endif	// __UI_LOCAL_H__

