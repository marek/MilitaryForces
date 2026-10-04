#ifndef UI_MENUS_H
#define UI_MENUS_H

class Menus
{
public:
	static void report();
	static void load();
	static void loadMenus(const char *menuFile, bool reset);
	static void showPostGame(bool newHigh);
	static void clearScores();
	static void loadArenas();
	static void registerCvars( );
	static void updateCvars( );
	static void drawConnectScreen( bool overlay );
	static void shutdown( );
	static bool consoleCommand( int realTime );
	static float clampCvar( float min, float max, float value );
	static void drawNamedPic( float x, float y, float width, float height, const char *picname );
	static void drawHandlePic( float x, float y, float w, float h, qhandle_t hShader );
	static void fillRect( float x, float y, float width, float height, const float *color );
	static void drawRect( float x, float y, float width, float height, const float *color );
	static void drawTopBottom(float x, float y, float w, float h);
	static void drawSides(float x, float y, float w, float h);
	static void updateScreen( );
	static void setColor( const float *rgba );
	static bool cursorInRect (int x, int y, int width, int height);
	static void adjustFrom640( float *x, float *y, float *w, float *h );
	static void drawTextBox (int x, int y, int width, int lines);
	static char *argv( int arg );
	static char *cvar_VariableString( const char *var_name );
	static void startDemoLoop( );
	static void loadBestScores(const char *map, int game);
	static void setBestScores(postGameInfo_t *newInfo, bool postGame);
	static int parseInfos( char *buf, int max, char *infos[] );
	static int alphabeticMapNameQsortCompare( const void * arg1, const void * arg2 );
	static void drawCenteredPic(qhandle_t image, int w, int h);
	static void drawMouse( );
	static void parseMenu(const char *menuFile);
	static int ownerDrawWidth(int ownerDraw, float scale);
	static void ownerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, const vec4_t color, qhandle_t shader, int textStyle, itemDef_t* item);
	static bool ownerDrawVisible(int flags);
	static bool ownerDrawHandleKey(int ownerDraw, int flags, float *special, int key);
	static float getValue(int ownerDraw);
	static void serversSort(int column, bool force);
	static void runMenuScript(char **args);
	static void getTeamColor(vec4_t *color);
	static bool hasSkinForBase(const char *base, const char *team);
	static int feederCount(float feederID);
	static const char *feederItemText(float feederID, int index, int column, qhandle_t *handle);
	static qhandle_t feederItemImage(float feederID, int index);
	static void feederSelection(float feederID, int index);
	static void pause(bool b);
	static int playCinematic(const char *name, float x, float y, float w, float h);
	static void stopCinematic(int handle);
	static void drawCinematic(int handle, float x, float y, float w, float h);
	static void runCinematicFrame(int handle);
	static void customChatInit( );
	static void loadNonIngame();
};

#endif
