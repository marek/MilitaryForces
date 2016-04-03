/*
 * $Id: ui_shared.h,v 1.7 2005-11-21 17:28:32 thebjoern Exp $
*/
#ifndef __UI_SHARED_H__
#define __UI_SHARED_H__


#include "../game/q_shared.h"
#include "../cgame/tr_types.h"
#include "keycodes.h"

#include "menudef.h"

// custom chat system
typedef enum {
	CCHAT_ALL,
	CCHAT_TEAM,
	CCHAT_TARGET,
	CCHAT_ATTACK,
	MAX_CCHAT
} CustomChatMode;

#define	MAX_CHAT_LEN		256
#define CHAT_TEXT_SCALE		0.25f

typedef struct {
	bool active;			// active?
	CustomChatMode mode;		// e.g. global chat, team chat, ...
	char text[ MAX_CHAT_LEN ];	// TODO: possibly make this dynamic to save memory
	int cindex;					// character index we are currently on
	float cursorAlpha;			// gfx
	bool cursorDir;			// gfx
	bool lockEntry;			// set to true to prevent more characters being typed
} chat_t;

// cursor gfx
enum CursorEnum 
{
	CURSOR_NORMAL,
	CURSOR_WAIT,
	MAX_CURSORS
};

typedef enum {
	LEFT_JUSTIFY,
	CENTRE_JUSTIFY,
	RIGHT_JUSTIFY
} textJustify_t;

// simple timing macros
//#define CG_FRAME_SECOND_FRACTION (cg.frametime/1000.0f)
//#define UI_FRAME_SECOND_FRACTION (uiInfo.dcUI.frameTime_/1000.0f)



#define MAX_MENUNAME 32
#define MAX_ITEMTEXT 64
#define MAX_ITEMACTION 64
#define MAX_MENUDEFFILE 4096
#define MAX_MENUFILE 32768
#define MAX_MENUS 64
#define MAX_MENUITEMS 96
#define MAX_COLOR_RANGES 10
#define MAX_OPEN_MENUS 16

#define WINDOW_MOUSEOVER			0x00000001	// mouse is over it, non exclusive
#define WINDOW_HASFOCUS				0x00000002	// has cursor focus, exclusive
#define WINDOW_VISIBLE				0x00000004	// is visible
#define WINDOW_GREY						0x00000008	// is visible but grey ( non-active )
#define WINDOW_DECORATION			0x00000010	// for decoration only, no mouse, keyboard, etc.. 
#define WINDOW_FADINGOUT			0x00000020	// fading out, non-active
#define WINDOW_FADINGIN				0x00000040	// fading in
#define WINDOW_MOUSEOVERTEXT	0x00000080	// mouse is over it, non exclusive
#define WINDOW_INTRANSITION		0x00000100	// window is in transition
#define WINDOW_FORECOLORSET		0x00000200	// forecolor was explicitly set ( used to color alpha images or not )
#define WINDOW_HORIZONTAL			0x00000400	// for list boxes and sliders, vertical is default this is set of horizontal
#define WINDOW_LB_LEFTARROW		0x00000800	// mouse is over left/up arrow
#define WINDOW_LB_RIGHTARROW	0x00001000	// mouse is over right/down arrow
#define WINDOW_LB_THUMB				0x00002000	// mouse is over thumb
#define WINDOW_LB_PGUP				0x00004000	// mouse is over page up
#define WINDOW_LB_PGDN				0x00008000	// mouse is over page down
#define WINDOW_ORBITING				0x00010000	// item is in orbit
#define WINDOW_OOB_CLICK			0x00020000	// close on out of bounds click
#define WINDOW_WRAPPED				0x00040000	// manually wrap text
#define WINDOW_AUTOWRAPPED			0x00080000	// auto wrap text
#define WINDOW_FORCED					0x00100000	// forced open
#define WINDOW_POPUP					0x00200000	// popup
#define WINDOW_BACKCOLORSET		0x00400000	// backcolor was explicitly set 
#define WINDOW_TIMEDVISIBLE		0x00800000	// visibility timing ( NOT implemented )


// CGAME cursor type bits
#define CURSOR_NONE					0x00000001
#define CURSOR_ARROW				0x00000002
#define CURSOR_SIZER				0x00000004

#ifdef CGAME
#define STRING_POOL_SIZE 128*1024
#else
#define STRING_POOL_SIZE 384*1024
#endif
#define MAX_STRING_HANDLES 4096

#define MAX_SCRIPT_ARGS 12
#define MAX_EDITFIELD 256

#define ART_FX_BASE			"menu/art/fx_base"
#define ART_FX_BLUE			"menu/art/fx_blue"
#define ART_FX_CYAN			"menu/art/fx_cyan"
#define ART_FX_GREEN		"menu/art/fx_grn"
#define ART_FX_RED			"menu/art/fx_red"
#define ART_FX_TEAL			"menu/art/fx_teal"
#define ART_FX_WHITE		"menu/art/fx_white"
#define ART_FX_YELLOW		"menu/art/fx_yel"

#define ASSET_GRADIENTBAR			"ui/assets/gradientbar2.tga"
#define	ASSET_VERTICALGRADIENT		"ui/assets/verticalgradient.tga"
#define ASSET_SCROLLBAR             "ui/assets/scrollbar.tga"
#define ASSET_SCROLLBAR_ARROWDOWN   "ui/assets/scrollbar_arrow_dwn_a.tga"
#define ASSET_SCROLLBAR_ARROWUP     "ui/assets/scrollbar_arrow_up_a.tga"
#define ASSET_SCROLLBAR_ARROWLEFT   "ui/assets/scrollbar_arrow_left.tga"
#define ASSET_SCROLLBAR_ARROWRIGHT  "ui/assets/scrollbar_arrow_right.tga"
#define ASSET_SCROLL_THUMB          "ui/assets/scrollbar_thumb.tga"
#define ASSET_SLIDER_BAR			"ui/assets/slider.tga"
#define ASSET_SLIDER_THUMB			"ui/assets/sliderbutton.tga"
#define SCROLLBAR_SIZE 16.0
#define SLIDER_WIDTH 96.0
#define SLIDER_HEIGHT 16.0
#define SLIDER_THUMB_WIDTH 12.0
#define SLIDER_THUMB_HEIGHT 20.0
#define	NUM_CROSSHAIRS			1

typedef struct {
  const char *command;
  const char *args[MAX_SCRIPT_ARGS];
} scriptDef_t;


struct rectDef_t
{
	float x;    // horiz position
	float y;    // vert position
	float w;    // width
	float h;    // height;
} ;

typedef rectDef_t Rectangle;

// FIXME: do something to separate text vs window stuff
typedef struct {
  Rectangle rect;                 // client coord rectangle
  Rectangle rectClient;           // screen coord rectangle
  const char *name;               //
  const char *group;              // if it belongs to a group
  const char *cinematicName;		  // cinematic name
  int cinematic;								  // cinematic handle
  int style;                      //
  int border;                     //
  int ownerDraw;									// ownerDraw style
	int ownerDrawFlags;							// show flags for ownerdraw items
  float borderSize;               // 
  int flags;                      // visible, focus, mouseover, cursor
  Rectangle rectEffects;          // for various effects
  Rectangle rectEffects2;         // for various effects
  int offsetTime;                 // time based value for various effects
  int nextTime;                   // time next effect should cycle
  vec4_t foreColor;               // text color
  vec4_t backColor;               // border color
  vec4_t borderColor;             // border color
  vec4_t outlineColor;            // border color
  qhandle_t background;           // background asset  
} windowDef_t;

typedef windowDef_t Window;

typedef struct {
	vec4_t	color;
	float		low;
	float		high;
} colorRangeDef_t;

// FIXME: combine flags into bitfields to save space
// FIXME: consolidate all of the common stuff in one structure for menus and items
// THINKABOUTME: is there any compelling reason not to have items contain items
// and do away with a menu per say.. major issue is not being able to dynamically allocate 
// and destroy stuff.. Another point to consider is adding an alloc free call for vm's and have 
// the engine just allocate the pool for it based on a cvar
// many of the vars are re-used for different item types, as such they are not always named appropriately
// the benefits of c++ in DOOM will greatly help crap like this
// FIXME: need to put a type ptr that points to specific type info per type
// 
#define MAX_LB_COLUMNS 16

typedef struct columnInfo_s {
	int pos;
	int width;
	int maxChars;
} columnInfo_t;

typedef struct listBoxDef_s {
	int startPos;
	int endPos;
	int drawPadding;
	int cursorPos;
	float elementWidth;
	float elementHeight;
	int elementStyle;
	int numColumns;
	columnInfo_t columnInfo[MAX_LB_COLUMNS];
	const char *doubleClick;
	bool notselectable;
} listBoxDef_t;

typedef struct editFieldDef_s {
  float minVal;                  //	edit field limits
  float maxVal;                  //
  float defVal;                  //
	float range;									 // 
  int maxChars;                  // for edit fields
  int maxPaintChars;             // for edit fields
	int paintOffset;							 // 
} editFieldDef_t;

#define MAX_MULTI_CVARS 32

typedef struct multiDef_s {
	const char *cvarList[MAX_MULTI_CVARS];
	const char *cvarStr[MAX_MULTI_CVARS];
	float cvarValue[MAX_MULTI_CVARS];
	int count;
	bool strDef;
} multiDef_t;

typedef struct modelDef_s {
	int angle;
	vec3_t origin;
	float fov_x;
	float fov_y;
	int rotationSpeed;
} modelDef_t;

#define CVAR_ENABLE		0x00000001
#define CVAR_DISABLE	0x00000002
#define CVAR_SHOW			0x00000004
#define CVAR_HIDE			0x00000008

struct itemDef_t 
{
  Window window;                 // common positional, border, style, layout info
  Rectangle textRect;            // rectangle the text ( if any ) consumes     
  int type;                      // text, button, radiobutton, checkbox, textfield, listbox, combo
  int alignment;                 // left center right
  int textalignment;             // ( optional ) alignment for text within rect based on text width
  float textalignx;              // ( optional ) text alignment x coord
  float textaligny;              // ( optional ) text alignment x coord
  float textscale;               // scale percentage from 72pts
  int textStyle;                 // ( optional ) style, normal and shadowed are it for now
  const char *text;              // display text
  void *parent;                  // menu owner
  qhandle_t asset;               // handle to asset
  const char *mouseEnterText;    // mouse enter script
  const char *mouseExitText;     // mouse exit script
  const char *mouseEnter;        // mouse enter script
  const char *mouseExit;         // mouse exit script 
  const char *action;            // select script
  const char *onFocus;           // select script
  const char *leaveFocus;        // select script
  const char *cvar;              // associated cvar 
  const char *cvarTest;          // associated cvar for enable actions
	const char *enableCvar;			   // enable, disable, show, or hide based on value, this can contain a list
	int cvarFlags;								 //	what type of action to take on cvarenables
  sfxHandle_t focusSound;
	int numColors;								 // number of color ranges
	colorRangeDef_t colorRanges[MAX_COLOR_RANGES];
	float special;								 // used for feeder id's etc.. diff per type
  int cursorPos;                 // cursor position in characters
	void *typeData;								 // type specific data ptr's	
};

typedef struct {
  Window window;
  const char  *font;								// font
  bool fullScreen;							// covers entire screen 
  int itemCount;										// number of items;
  int fontIndex;										// 
  int cursorItem;										// which item as the cursor
	int fadeCycle;										//
	float fadeClamp;									//
	float fadeAmount;									//
  const char *onOpen;								// run when the menu is first opened
  const char *onClose;							// run when the menu is closed
  const char *onESC;								// run when the menu is closed
	const char *soundName;						// background loop sound for menu

  vec4_t focusColor;								// focus color for items
  vec4_t disableColor;							// focus color for items
  itemDef_t *items[MAX_MENUITEMS];	// items this menu contains   
} menuDef_t;

struct cachedAssets_t
{
  const char *fontStr;
  const char *cursorStr;
  const char *cursorStrWait;
  const char *gradientStr;

  fontInfo_t textFont;
  fontInfo_t smallFont;
  fontInfo_t bigFont;

  qhandle_t midPanelGfx;
  qhandle_t cursor;
  qhandle_t cursorWait;
  qhandle_t gradientBar;
  qhandle_t verticalGradient;
  qhandle_t scrollBarArrowUp;
  qhandle_t scrollBarArrowDown;
  qhandle_t scrollBarArrowLeft;
  qhandle_t scrollBarArrowRight;
  qhandle_t scrollBar;
  qhandle_t scrollBarThumb;
  qhandle_t buttonMiddle;
  qhandle_t buttonInside;
  qhandle_t solidBox;
  qhandle_t sliderBar;
  qhandle_t sliderThumb;
  sfxHandle_t menuEnterSound;
  sfxHandle_t menuExitSound;
  sfxHandle_t menuBuzzSound;
  sfxHandle_t itemFocusSound;
  float fadeClamp;
  int fadeCycle;
  float fadeAmount;
  float shadowX;
  float shadowY;
  vec4_t shadowColor;
  float shadowFadeClamp;
  bool fontRegistered;

  // player settings
  qhandle_t fxBasePic;
  qhandle_t fxPic[7];
  qhandle_t	crosshairShader[NUM_CROSSHAIRS];
};

#endif	// __UI_SHARED_H__
