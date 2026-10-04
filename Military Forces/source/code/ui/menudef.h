/*
 * $Id: menudef.h,v 1.4 2005-11-12 14:28:14 thebjoern Exp $
*/

#ifndef __MENUDEF_H__
#define __MENUDEF_H__

inline constexpr int ITEM_TYPE_TEXT = 0; // simple text
inline constexpr int ITEM_TYPE_BUTTON = 1; // button, basically text with a border
inline constexpr int ITEM_TYPE_RADIOBUTTON = 2; // toggle button, may be grouped
inline constexpr int ITEM_TYPE_CHECKBOX = 3; // check box
inline constexpr int ITEM_TYPE_EDITFIELD = 4; // editable text, associated with a cvar
inline constexpr int ITEM_TYPE_COMBO = 5; // drop down list
inline constexpr int ITEM_TYPE_LISTBOX = 6; // scrollable list
inline constexpr int ITEM_TYPE_MODEL = 7; // model
inline constexpr int ITEM_TYPE_OWNERDRAW = 8; // owner draw, name specs what it is
inline constexpr int ITEM_TYPE_NUMERICFIELD = 9; // editable text, associated with a cvar
inline constexpr int ITEM_TYPE_SLIDER = 10; // mouse speed, volume, etc.
inline constexpr int ITEM_TYPE_YESNO = 11; // yes no cvar setting
inline constexpr int ITEM_TYPE_MULTI = 12; // multiple list setting, enumerated
inline constexpr int ITEM_TYPE_BIND = 13; // multiple list setting, enumerated
    
inline constexpr int ITEM_ALIGN_LEFT = 0; // left alignment
inline constexpr int ITEM_ALIGN_CENTER = 1; // center alignment
inline constexpr int ITEM_ALIGN_RIGHT = 2; // right alignment

inline constexpr int ITEM_TEXTSTYLE_NORMAL = 0; // normal text
inline constexpr int ITEM_TEXTSTYLE_BLINK = 1; // fast blinking
inline constexpr int ITEM_TEXTSTYLE_PULSE = 2; // slow pulsing
inline constexpr int ITEM_TEXTSTYLE_SHADOWED = 3; // drop shadow ( need a color for this )
inline constexpr int ITEM_TEXTSTYLE_OUTLINED = 4; // drop shadow ( need a color for this )
inline constexpr int ITEM_TEXTSTYLE_OUTLINESHADOWED = 5; // drop shadow ( need a color for this )
inline constexpr int ITEM_TEXTSTYLE_SHADOWEDMORE = 6; // drop shadow ( need a color for this )
                          
inline constexpr int WINDOW_BORDER_NONE = 0; // no border
inline constexpr int WINDOW_BORDER_FULL = 1; // full border based on border color ( single pixel )
inline constexpr int WINDOW_BORDER_HORZ = 2; // horizontal borders only
inline constexpr int WINDOW_BORDER_VERT = 3; // vertical borders only
inline constexpr int WINDOW_BORDER_KCGRADIENT = 4; // horizontal border using the gradient bars
  
inline constexpr int WINDOW_STYLE_EMPTY = 0; // no background
inline constexpr int WINDOW_STYLE_FILLED = 1; // filled with background color
inline constexpr int WINDOW_STYLE_GRADIENT = 2; // gradient bar based on background color
inline constexpr int WINDOW_STYLE_SHADER = 3; // gradient bar based on background color
//#define WINDOW_STYLE_TEAMCOLOR 4          // team color
inline constexpr int WINDOW_STYLE_CINEMATIC = 5; // cinematic

inline constexpr int MENU_TRUE = 1; // uh.. true
inline constexpr int MENU_FALSE = 0; // and false

inline constexpr int HUD_VERTICAL = 0x00;
inline constexpr int HUD_HORIZONTAL = 0x01;

// list box element types
inline constexpr int LISTBOX_TEXT = 0x00;
inline constexpr int LISTBOX_IMAGE = 0x01;

// list feeders
inline constexpr int FEEDER_HEADS = 0x00; // model heads
inline constexpr int FEEDER_MAPS = 0x01; // text maps based on game type
inline constexpr int FEEDER_SERVERS = 0x02; // servers
inline constexpr int FEEDER_CLANS = 0x03; // clan names
inline constexpr int FEEDER_ALLMAPS = 0x04; // all maps available, in graphic format
inline constexpr int FEEDER_REDTEAM_LIST = 0x05; // red team members
inline constexpr int FEEDER_BLUETEAM_LIST = 0x06; // blue team members
inline constexpr int FEEDER_PLAYER_LIST = 0x07; // players
inline constexpr int FEEDER_TEAM_LIST = 0x08; // team members for team voting
inline constexpr int FEEDER_MODS = 0x09; // team members for team voting
inline constexpr int FEEDER_DEMOS = 0x0a; // team members for team voting
inline constexpr int FEEDER_SCOREBOARD = 0x0b; // team members for team voting
inline constexpr int FEEDER_Q3HEADS = 0x0c; // model heads
inline constexpr int FEEDER_SERVERSTATUS = 0x0d; // server status
inline constexpr int FEEDER_FINDPLAYER = 0x0e; // find player
inline constexpr int FEEDER_CINEMATICS = 0x0f; // cinematics
inline constexpr int FEEDER_SPECTATOR_LIST = 0x10; // spectators

// display flags
inline constexpr int CG_SHOW_BLUE_TEAM_HAS_REDFLAG = 0x00000001;
inline constexpr int CG_SHOW_RED_TEAM_HAS_BLUEFLAG = 0x00000002;
inline constexpr int CG_SHOW_ANYTEAMGAME = 0x00000004;
inline constexpr int CG_SHOW_HARVESTER = 0x00000008;
inline constexpr int CG_SHOW_ONEFLAG = 0x00000010;
inline constexpr int CG_SHOW_CTF = 0x00000020;
inline constexpr int CG_SHOW_OBELISK = 0x00000040;
inline constexpr int CG_SHOW_HEALTHCRITICAL = 0x00000080;
inline constexpr int CG_SHOW_SINGLEPLAYER = 0x00000100;
inline constexpr int CG_SHOW_TOURNAMENT = 0x00000200;
inline constexpr int CG_SHOW_DURINGINCOMINGVOICE = 0x00000400;
inline constexpr int CG_SHOW_IF_PLAYER_HAS_FLAG = 0x00000800;
inline constexpr int CG_SHOW_LANPLAYONLY = 0x00001000;
inline constexpr int CG_SHOW_MINED = 0x00002000;
inline constexpr int CG_SHOW_HEALTHOK = 0x00004000;
inline constexpr int CG_SHOW_TEAMINFO = 0x00008000;
inline constexpr int CG_SHOW_NOTEAMINFO = 0x00010000;
inline constexpr int CG_SHOW_OTHERTEAMHASFLAG = 0x00020000;
inline constexpr int CG_SHOW_YOURTEAMHASENEMYFLAG = 0x00040000;
inline constexpr int CG_SHOW_ANYNONTEAMGAME = 0x00080000;
inline constexpr int CG_SHOW_2DONLY = 0x10000000;


inline constexpr int UI_SHOW_LEADER = 0x00000001;
inline constexpr int UI_SHOW_NOTLEADER = 0x00000002;
inline constexpr int UI_SHOW_FAVORITESERVERS = 0x00000004;
inline constexpr int UI_SHOW_ANYNONTEAMGAME = 0x00000008;
inline constexpr int UI_SHOW_ANYTEAMGAME = 0x00000010;
inline constexpr int UI_SHOW_NEWHIGHSCORE = 0x00000020;
inline constexpr int UI_SHOW_DEMOAVAILABLE = 0x00000040;
inline constexpr int UI_SHOW_NEWBESTTIME = 0x00000080;
inline constexpr int UI_SHOW_FFA = 0x00000100;
inline constexpr int UI_SHOW_NOTFFA = 0x00000200;
inline constexpr int UI_SHOW_NETANYNONTEAMGAME = 0x00000400;
inline constexpr int UI_SHOW_NETANYTEAMGAME = 0x00000800;
inline constexpr int UI_SHOW_NOTFAVORITESERVERS = 0x00001000;




// owner draw types
// ideally these should be done outside of this file but
// this makes it much easier for the macro expansion to 
// convert them for the designers ( from the .menu files )
inline constexpr int CG_OWNERDRAW_BASE = 1;
inline constexpr int CG_PLAYER_ARMOR_ICON = 1;
inline constexpr int CG_PLAYER_ARMOR_VALUE = 2;
inline constexpr int CG_PLAYER_HEAD = 3;
inline constexpr int CG_PLAYER_HEALTH = 4;
inline constexpr int CG_PLAYER_AMMO_ICON = 5;
inline constexpr int CG_PLAYER_AMMO_VALUE = 6;
inline constexpr int CG_SELECTEDPLAYER_HEAD = 7;
inline constexpr int CG_SELECTEDPLAYER_NAME = 8;
inline constexpr int CG_SELECTEDPLAYER_LOCATION = 9;
inline constexpr int CG_SELECTEDPLAYER_STATUS = 10;
inline constexpr int CG_SELECTEDPLAYER_WEAPON = 11;
inline constexpr int CG_SELECTEDPLAYER_POWERUP = 12;

inline constexpr int CG_FLAGCARRIER_HEAD = 13;
inline constexpr int CG_FLAGCARRIER_NAME = 14;
inline constexpr int CG_FLAGCARRIER_LOCATION = 15;
inline constexpr int CG_FLAGCARRIER_STATUS = 16;
inline constexpr int CG_FLAGCARRIER_WEAPON = 17;
inline constexpr int CG_FLAGCARRIER_POWERUP = 18;

inline constexpr int CG_PLAYER_ITEM = 19;
inline constexpr int CG_PLAYER_SCORE = 20;

inline constexpr int CG_BLUE_FLAGHEAD = 21;
inline constexpr int CG_BLUE_FLAGSTATUS = 22;
inline constexpr int CG_BLUE_FLAGNAME = 23;
inline constexpr int CG_RED_FLAGHEAD = 24;
inline constexpr int CG_RED_FLAGSTATUS = 25;
inline constexpr int CG_RED_FLAGNAME = 26;

inline constexpr int CG_BLUE_SCORE = 27;
inline constexpr int CG_RED_SCORE = 28;
inline constexpr int CG_RED_NAME = 29;
inline constexpr int CG_BLUE_NAME = 30;
inline constexpr int CG_HARVESTER_SKULLS = 31; // only shows in harvester
inline constexpr int CG_ONEFLAG_STATUS = 32; // only shows in one flag
inline constexpr int CG_PLAYER_LOCATION = 33;
inline constexpr int CG_TEAM_COLOR = 34;
inline constexpr int CG_CTF_POWERUP = 35;
                                        
inline constexpr int CG_AREA_POWERUP = 36;
inline constexpr int CG_AREA_LAGOMETER = 37; // painted with old system
inline constexpr int CG_PLAYER_HASFLAG = 38;
inline constexpr int CG_GAME_TYPE = 39; // not done

inline constexpr int CG_SELECTEDPLAYER_ARMOR = 40;
inline constexpr int CG_SELECTEDPLAYER_HEALTH = 41;
inline constexpr int CG_PLAYER_STATUS = 42;
inline constexpr int CG_FRAGGED_MSG = 43; // painted with old system
inline constexpr int CG_PROXMINED_MSG = 44; // painted with old system
inline constexpr int CG_AREA_FPSINFO = 45; // painted with old system
inline constexpr int CG_AREA_SYSTEMCHAT = 46; // painted with old system
inline constexpr int CG_AREA_TEAMCHAT = 47; // painted with old system
inline constexpr int CG_AREA_CHAT = 48; // painted with old system
inline constexpr int CG_GAME_STATUS = 49;
inline constexpr int CG_KILLER = 50;
inline constexpr int CG_PLAYER_ARMOR_ICON2D = 51;
inline constexpr int CG_PLAYER_AMMO_ICON2D = 52;
inline constexpr int CG_ACCURACY = 53;
inline constexpr int CG_ASSISTS = 54;
inline constexpr int CG_DEFEND = 55;
inline constexpr int CG_EXCELLENT = 56;
inline constexpr int CG_IMPRESSIVE = 57;
inline constexpr int CG_PERFECT = 58;
inline constexpr int CG_GAUNTLET = 59;
inline constexpr int CG_SPECTATORS = 60;
inline constexpr int CG_TEAMINFO = 61;
inline constexpr int CG_VOICE_HEAD = 62;
inline constexpr int CG_VOICE_NAME = 63;
inline constexpr int CG_PLAYER_HASFLAG2D = 64;
inline constexpr int CG_HARVESTER_SKULLS2D = 65; // only shows in harvester
inline constexpr int CG_CAPFRAGLIMIT = 66;
inline constexpr int CG_1STPLACE = 67;
inline constexpr int CG_2NDPLACE = 68;
inline constexpr int CG_CAPTURES = 69;
inline constexpr int CG_GAME_MISC = 70;
inline constexpr int CG_GAME_REDSCORE = 71;
inline constexpr int CG_GAME_BLUESCORE = 72;
inline constexpr int CG_GAME_REDCOUNT = 73;
inline constexpr int CG_GAME_BLUECOUNT = 74;
inline constexpr int CG_GAME_SPECTATORCOUNT = 75;
inline constexpr int CG_GAME_PLAYERSCOUNT = 76;

inline constexpr int UI_OWNERDRAW_BASE = 200;
inline constexpr int UI_HANDICAP = 200;
inline constexpr int UI_EFFECTS = 201;
inline constexpr int UI_PLAYERMODEL = 202;
inline constexpr int UI_CLANNAME = 203;
inline constexpr int UI_CLANLOGO = 204;
inline constexpr int UI_GAMETYPE = 205;
inline constexpr int UI_MAPPREVIEW = 206;
inline constexpr int UI_SKILL = 207;
inline constexpr int UI_BLUETEAMNAME = 208;
inline constexpr int UI_REDTEAMNAME = 209;
inline constexpr int UI_BLUETEAM1 = 210;
inline constexpr int UI_BLUETEAM2 = 211;
inline constexpr int UI_BLUETEAM3 = 212;
inline constexpr int UI_BLUETEAM4 = 213;
inline constexpr int UI_BLUETEAM5 = 214;
inline constexpr int UI_REDTEAM1 = 215;
inline constexpr int UI_REDTEAM2 = 216;
inline constexpr int UI_REDTEAM3 = 217;
inline constexpr int UI_REDTEAM4 = 218;
inline constexpr int UI_REDTEAM5 = 219;
inline constexpr int UI_NETSOURCE = 220;
inline constexpr int UI_NETMAPPREVIEW = 221;
inline constexpr int UI_NETFILTER = 222;
inline constexpr int UI_TIER = 223;
inline constexpr int UI_OPPONENTMODEL = 224;
inline constexpr int UI_TIERMAP1 = 225;
inline constexpr int UI_TIERMAP2 = 226;
inline constexpr int UI_TIERMAP3 = 227;
inline constexpr int UI_PLAYERLOGO = 228;
inline constexpr int UI_OPPONENTLOGO = 229;
inline constexpr int UI_PLAYERLOGO_METAL = 230;
inline constexpr int UI_OPPONENTLOGO_METAL = 231;
inline constexpr int UI_PLAYERLOGO_NAME = 232;
inline constexpr int UI_OPPONENTLOGO_NAME = 233;
inline constexpr int UI_TIER_MAPNAME = 234;
inline constexpr int UI_TIER_GAMETYPE = 235;
inline constexpr int UI_ALLMAPS_SELECTION = 236;
inline constexpr int UI_OPPONENT_NAME = 237;
inline constexpr int UI_VOTE_KICK = 238;
//#define UI_BOTNAME 239
//#define UI_BOTSKILL 240
inline constexpr int UI_REDBLUE = 241;
inline constexpr int UI_CROSSHAIR = 242;
inline constexpr int UI_SELECTEDPLAYER = 243;
inline constexpr int UI_MAPCINEMATIC = 244;
inline constexpr int UI_NETGAMETYPE = 245;
inline constexpr int UI_NETMAPCINEMATIC = 246;
inline constexpr int UI_SERVERREFRESHDATE = 247;
inline constexpr int UI_SERVERMOTD = 248;
inline constexpr int UI_GLINFO = 249;
inline constexpr int UI_KEYBINDSTATUS = 250;
inline constexpr int UI_CLANCINEMATIC = 251;
inline constexpr int UI_MAP_TIMETOBEAT = 252;
inline constexpr int UI_JOINGAMETYPE = 253;
inline constexpr int UI_PREVIEWCINEMATIC = 254;
inline constexpr int UI_STARTMAPCINEMATIC = 255;
inline constexpr int UI_MAPS_SELECTION = 256;
inline constexpr int UI_PREVIEW_VEHICLE = 257;
inline constexpr int UI_NETGAMESET = 258;
inline constexpr int UI_CREDITS = 259;

#define VOICECHAT_GETFLAG			"getflag"				// command someone to get the flag
#define VOICECHAT_OFFENSE			"offense"				// command someone to go on offense
#define VOICECHAT_DEFEND			"defend"				// command someone to go on defense
#define VOICECHAT_DEFENDFLAG		"defendflag"			// command someone to defend the flag
#define VOICECHAT_PATROL			"patrol"				// command someone to go on patrol (roam)
#define VOICECHAT_CAMP				"camp"					// command someone to camp (we don't have sounds for this one)
#define VOICECHAT_FOLLOWME			"followme"				// command someone to follow you
#define VOICECHAT_RETURNFLAG		"returnflag"			// command someone to return our flag
#define VOICECHAT_FOLLOWFLAGCARRIER	"followflagcarrier"		// command someone to follow the flag carrier
#define VOICECHAT_YES				"yes"					// yes, affirmative, etc.
#define VOICECHAT_NO				"no"					// no, negative, etc.
#define VOICECHAT_ONGETFLAG			"ongetflag"				// I'm getting the flag
#define VOICECHAT_ONOFFENSE			"onoffense"				// I'm on offense
#define VOICECHAT_ONDEFENSE			"ondefense"				// I'm on defense
#define VOICECHAT_ONPATROL			"onpatrol"				// I'm on patrol (roaming)
#define VOICECHAT_ONCAMPING			"oncamp"				// I'm camping somewhere
#define VOICECHAT_ONFOLLOW			"onfollow"				// I'm following
#define VOICECHAT_ONFOLLOWCARRIER	"onfollowcarrier"		// I'm following the flag carrier
#define VOICECHAT_ONRETURNFLAG		"onreturnflag"			// I'm returning our flag
#define VOICECHAT_INPOSITION		"inposition"			// I'm in position
#define VOICECHAT_IHAVEFLAG			"ihaveflag"				// I have the flag
#define VOICECHAT_BASEATTACK		"baseattack"			// the base is under attack
#define VOICECHAT_ENEMYHASFLAG		"enemyhasflag"			// the enemy has our flag (CTF)
#define VOICECHAT_STARTLEADER		"startleader"			// I'm the leader
#define VOICECHAT_STOPLEADER		"stopleader"			// I resign leadership
#define VOICECHAT_TRASH				"trash"					// lots of trash talk
#define VOICECHAT_WHOISLEADER		"whoisleader"			// who is the team leader
#define VOICECHAT_WANTONDEFENSE		"wantondefense"			// I want to be on defense
#define VOICECHAT_WANTONOFFENSE		"wantonoffense"			// I want to be on offense
#define VOICECHAT_KILLINSULT		"kill_insult"			// I just killed you
#define VOICECHAT_TAUNT				"taunt"					// I want to taunt you
#define VOICECHAT_DEATHINSULT		"death_insult"			// you just killed me
#define VOICECHAT_KILLGAUNTLET		"kill_gauntlet"			// I just killed you with the gauntlet
#define VOICECHAT_PRAISE			"praise"				// you did something good


#endif // __MENUDEF_H__
