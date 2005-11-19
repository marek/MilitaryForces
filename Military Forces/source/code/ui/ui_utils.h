#ifndef __UI_UTILS_H__
#define __UI_UTILS_H__

#include "../ui/ui_displaycontext.h"
#include "../ui/ui_scrollinfo.h"
#include <map>
#include <string>


struct UI_HashUtils;
struct ControlUtils;
struct UI_CaptureBase;
struct UI_PrecompilerTools;



struct UI_Utils
{
	struct Command;
	struct Command_SetFocus;
	struct Command_FadeIn;
	struct Command_FadeOut;
	struct Command_Show;
	struct Command_Hide;
	struct Command_SetColor;
	struct Command_Open;
	struct Command_Close;
	struct Command_SetAsset;
	struct Command_SetBackground;
	struct Command_SetItemColor;
	struct Command_SetPlayerModel;
	struct Command_SetPlayerHead;
	struct Command_Transition;
	struct Command_SetCvar;
	struct Command_Exec;
	struct Command_Play;
	struct Command_PlayLooped;
	struct Command_Orbit;

	struct StringDef;

	// funcs
	//void						init_Display(UI_DisplayContext* dc);

	void						window_Init( Window *w ); 
	void						window_Paint( Window *w, float fadeAmount, float fadeClamp, float fadeCycle );
	void						window_CloseCinematic( windowDef_t *window );
	void						window_CacheContents( windowDef_t *window );

	void						gradientBar_Paint( rectDef_t *rect, vec4_t color );

	void						item_TextColor( itemDef_t *item, vec4_t *newColor );
	bool						item_EnableShowViaCvar( itemDef_t *item, int flag );
	bool						item_SetFocus( itemDef_t *item, float x, float y );
	void						item_RunScript(itemDef_t *item, const char *s);
	void						item_UpdatePosition( itemDef_t *item );
	void						item_SetScreenCoords( itemDef_t *item, float x, float y );
	rectDef_t*					item_CorrectedTextRect( itemDef_t *item );
	bool						item_Parse( int handle, itemDef_t *item );
	void						item_InitControls( itemDef_t *item );
	void						item_ValidateTypeData( itemDef_t *item );
	void						item_Paint(itemDef_t *item);
	void						item_OwnerDraw_Paint( itemDef_t *item );
	void						item_Text_Paint( itemDef_t *item );
	void						item_TextField_Paint( itemDef_t *item );
	void						item_ListBox_Paint( itemDef_t *item );
	void						item_Bind_Paint( itemDef_t *item );
	void						item_Model_Paint( itemDef_t *item );
	void						item_YesNo_Paint( itemDef_t *item );
	void						item_Multi_Paint( itemDef_t *item );
	void						item_Slider_Paint(itemDef_t *item);
	void						item_Text_Wrapped_Paint( itemDef_t *item );
	void						item_Text_AutoWrapped_Paint( itemDef_t *item );
	void						item_SetTextExtents( itemDef_t *item, int *width, int *height, const char *text );
	int							item_ListBox_ThumbDrawPosition( itemDef_t *item );
	int							item_ListBox_ThumbPosition( itemDef_t *item );
	const char*					item_Multi_Setting( itemDef_t *item );
	float						item_Slider_ThumbPosition( itemDef_t *item );
	int							item_ListBox_MaxScroll( itemDef_t *item );
	void						item_CacheContents( itemDef_t *item );
	void						item_MouseEnter( itemDef_t *item, float x, float y );
	void						item_MouseLeave( itemDef_t *item );
	void						item_ListBox_MouseEnter(itemDef_t *item, float x, float y);
	int							item_ListBox_OverLB( itemDef_t *item, float x, float y );
	void						item_SetMouseOver( itemDef_t *item, bool focus );
	bool						item_Bind_HandleKey( itemDef_t *item, int key, int down );
	bool						item_TextField_HandleKey( itemDef_t *item, int key );
	void						item_Action( itemDef_t *item );
	bool						item_HandleKey( itemDef_t *item, int key, int down );
	void						item_StopCapture( itemDef_t *item );
	void						item_StartCapture( itemDef_t *item, int key );
	bool						item_OwnerDraw_HandleKey( itemDef_t *item, int key );
	bool						item_ListBox_HandleKey( itemDef_t *item, int key, int down, bool force );
	bool						item_YesNo_HandleKey( itemDef_t *item, int key );
	bool						item_Multi_HandleKey( itemDef_t *item, int key );
	bool						item_Slider_HandleKey( itemDef_t *item, int key, int down );
	int							item_Multi_FindCvarByValue( itemDef_t *item );
	int							item_Multi_CountSettings( itemDef_t *item );
	int							item_Slider_OverSlider( itemDef_t *item, float x, float y );
	void						item_Image_Paint( itemDef_t *item );
	void						item_Init( itemDef_t *item );

	itemDef_t*					menu_FindItemByName( menuDef_t *menu, const char *p );
	itemDef_t*					menu_ClearFocus( menuDef_t *menu );
	void						menu_FadeItemByName( menuDef_t *menu, const char *p, bool fadeOut );
	void						menu_ShowItemByName( menuDef_t *menu, const char *p, bool bShow ) ;
	void						menu_OpenByName( const char *p );
	void						menu_CloseByName( const char *p );
	int							menu_ItemsMatchingGroup( menuDef_t *menu, const char *name );
	itemDef_t*					menu_GetMatchingItemByNumber( menuDef_t *menu, int index, const char *name );
	void						menu_TransitionItemByName( menuDef_t *menu, const char *p, rectDef_t rectFrom, 
														   rectDef_t rectTo, int time, float amt );
	void						menu_OrbitItemByName( menuDef_t *menu, const char *p, float x, float y, float cx, 
													  float cy, int time );
	void						menu_RunCloseScript( menuDef_t *menu );
	void						menu_UpdatePosition( menuDef_t *menu );
	void						menu_PostParse( menuDef_t *menu );
	void						menu_CloseAll();
	menuDef_t*					menu_FindByName( const char *p );
	void						menu_ShowByName( const char *p );
	void						menu_CloseCinematics( menuDef_t *menu );
	void						menu_HandleOOBClick( menuDef_t *menu, int key, int down );
	bool						menu_OverActiveItem( menuDef_t *menu, float x, float y );
	menuDef_t*					menu_GetFocused();
	bool						menu_AnyFullScreenVisible();
	menuDef_t*					menu_ActivateByName( const char *p );
	void						menu_Activate( menuDef_t *menu );
	void						menu_New( int handle );
	void						menu_Init( menuDef_t *menu );
	bool						menu_Parse( int handle, menuDef_t *menu );
	void						menu_Paint( menuDef_t *menu, bool forcePaint );
	void						menu_CacheContents( menuDef_t *menu );
	void						menu_HandleMouseMove( menuDef_t *menu, float x, float y );
	void						menu_HandleKey( menuDef_t *menu, int key, int down );
	itemDef_t*					menu_SetNextCursorItem( menuDef_t *menu );
	itemDef_t*					menu_SetPrevCursorItem( menuDef_t *menu );
	int							menu_Count() const;
	void						menu_Reset();
	void						menu_PaintAll();
	itemDef_t*					menu_GetFocusedItem( menuDef_t *menu );
	void						menu_ScrollFeeder( menuDef_t *menu, int feeder, int down );
	void						menu_SetFeederSelection( menuDef_t *menu, int feeder, int index, const char *name );
	itemDef_t*					menu_HitTest( menuDef_t *menu, float x, float y );

	void						display_CloseCinematics();
	int							display_VisibleMenuCount();
	void						display_CacheAll();
	void						display_HandleKey( int key, bool down, int x, int y );
	void*						display_CaptureItem( int x, int y );
	UI_DisplayContext*			display_GetContext();
	int							display_CursorType(int x, int y);
	bool						display_MouseMove( void *p, int x, int y );
	bool						display_KeyBindPending() const;

	void						pc_SourceError( int handle, char *format, ... );
	bool						pc_Float_Parse( int handle, float *f );
	bool						pc_Color_Parse(int handle, vec4_t *c);
	bool						pc_Int_Parse( int handle, int *i );
	bool						pc_Rect_Parse( int handle, rectDef_t *r );
	bool						pc_String_Parse( int handle, const char **out );
	bool						pc_Script_Parse( int handle, const char **out );

	bool 						float_Parse( char **p, float *f );
	bool 						color_Parse( char **p, vec4_t *c );
	bool 						int_Parse( char **p, int *i );
	bool 						rect_Parse( char **p, rectDef_t *r );
	bool 						string_Parse( char **p, const char **out );

	void						fade( int *flags, float *f, float clamp, int *nextTime, int offsetTime, 
									  bool bFlags, float fadeAmount );
	bool						rect_ContainsPoint( rectDef_t *rect, float x, float y );
	void						toWindowCoords( float *x, float *y, windowDef_t *window );
	void						rect_ToWindowCoords( rectDef_t *rect, windowDef_t *window );
	void						adjustFrom640( float *x, float *y, float *w, float *h );
	void						lerpColor( const vec4_t a, const vec4_t b, vec4_t c, float t );
	bool						isVisible( int flags );
	vec4_t*						createColourVector( float r, float g, float b, float a, vec4_t* pVector );
	unsigned char*				createColourChar( unsigned char r, unsigned char g, unsigned char b, unsigned char a, 
												  unsigned char * pArray );
	void						verticalGradient_Paint(rectDef_t *rect, vec4_t color);
	void						drawStringNewAlpha( int x, int y, const char * pText, float alpha, textJustify_t formatting );
	void						drawStringNewColour( int x, int y, const char * pText, vec4_t colour, textJustify_t formatting );
	void						drawStringNewBasic( int x, int y, float scale, const char * pText );
	void						drawStringNew( int x, int y, float scale, vec4_t colour, const char * pText, int hSpacing, 
											   int numChars, int style, textJustify_t formatting );

	void*						alloc( int size );
	void						initMemory();
	int							outOfMemory();
	const char*					string_Alloc( const char *p );
	void						string_Report();
	void						string_Init();
	long						hashForString( const char *str );

	float						getFrameSecondFraction() { return dc_->frameTime_ / 1000.0f; }

	ControlUtils*				getControlUtils() { return controlUtils_; }						

	UI_DisplayContext*			getDisplayContext() { return dc_; }

	UI_PrecompilerTools*		getPrecompilerTools() { return precomp_; }

	static const int			SCROLL_TIME_START =	500;
	static const int			SCROLL_TIME_ADJUST = 150;
	static const int			SCROLL_TIME_ADJUSTOFFSET = 40;
	static const int			SCROLL_TIME_FLOOR = 20;
	static const int			DOUBLE_CLICK_DELAY = 300;

protected:

								UI_Utils();
	virtual						~UI_Utils();

	typedef std::map<std::string,Command*> CommandMap;
	typedef std::map<std::string,Command*>::iterator CommandMapIter;

	// data members
	UI_DisplayContext*			dc_;

	CommandMap					commands_;

	menuDef_t					menus_[MAX_MENUS];      // defined menus
	int							menuCount_;               // how many

	menuDef_t*					menuStack_[MAX_OPEN_MENUS];
	int							openMenuCount_;

	UI_HashUtils*				hashUtils_;

	bool						editingField_;
	itemDef_t*					bindItem_;
	itemDef_t*					editItem_;

	itemDef_t*					itemCapture_;

	ControlUtils*				controlUtils_;

	bool						waitingForKey_;

	UI_ScrollInfo				scrollInfo_;

	int							lastListBoxClickTime_;

	UI_CaptureBase*				captureTool_;

	vec4_t						colourVector_;
	unsigned char				colourCharArray_[4];
	
	bool						debugMode_;

	static const int			MEM_POOL_SIZE = 1024 * 1024;
	static const int			HASH_TABLE_SIZE = 2048;

	char						memoryPool_[MEM_POOL_SIZE];
	int							allocPoint_;
	bool						outOfMemory_;

	int							strPoolIndex_;
	char						strPool_[STRING_POOL_SIZE];

	int							strHandleCount_;
	StringDef*					strHandle_[HASH_TABLE_SIZE];

	UI_PrecompilerTools*		precomp_;

private:
	// disable	
								UI_Utils( UI_Utils const& );
	UI_Utils&					operator=( UI_Utils const& );
};

struct UI_Utils::StringDef
{
	struct StringDef *next;
	const char *str;
};






#endif // __UI_UTILS_H__

