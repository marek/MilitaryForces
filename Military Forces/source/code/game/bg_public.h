/*
 * $Id: bg_public.h,v 1.18 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// bg_public.h -- definitions shared by both the server game and client game modules

// because games can change separately from the main system version, we need a
// second version that must match between game and cgame

#ifndef __BG_PUBLIC_H__
#define __BG_PUBLIC_H__

#include "../qcommon/qfiles.h"


//#define	GAME_VERSION		"v0.9"
#define	GAME_IDENTIFIER		"mfgame"			// use to identify mfq3 servers

inline constexpr float DEFAULT_GRAVITY = 800.0f;

inline constexpr int GIB_HEALTH = -50;

inline constexpr int MAX_ITEMS = 256;

inline constexpr int RANK_TIED_FLAG = 0x4000;

inline constexpr int ITEM_RADIUS = 15; // item sizes are needed for client side pickup detection

inline constexpr int SCORE_NOT_PRESENT = -9999; // for the CS_SCORES[12] when only one player is present

inline constexpr int VOTE_TIME = 30000; // 30 seconds before vote times out

//
// config strings are a general means of communicating variable length strings
// from the server to all connected clients.
//

// CS_SERVERINFO and CS_SYSTEMINFO are defined in q_shared.h
inline constexpr int CS_MUSIC = 2;
inline constexpr int CS_MESSAGE = 3; // from the map worldspawn's message field
inline constexpr int CS_MOTD = 4; // g_motd string for server message of the day
inline constexpr int CS_WARMUP = 5; // server time when the match will be restarted
inline constexpr int CS_SCORES1 = 6;
inline constexpr int CS_SCORES2 = 7;
inline constexpr int CS_VOTE_TIME = 8;
inline constexpr int CS_VOTE_STRING = 9;
inline constexpr int CS_VOTE_YES = 10;
inline constexpr int CS_VOTE_NO = 11;

inline constexpr int CS_TEAMVOTE_TIME = 12;
inline constexpr int CS_TEAMVOTE_STRING = 14;
inline constexpr int CS_TEAMVOTE_YES = 16;
inline constexpr int CS_TEAMVOTE_NO = 18;

inline constexpr int CS_GAME_VERSION = 20;
inline constexpr int CS_LEVEL_START_TIME = 21; // so the timer only shows the current level
inline constexpr int CS_INTERMISSION = 22; // when 1, fraglimit/timelimit has been hit and intermission will start in a second or two
inline constexpr int CS_FLAGSTATUS = 23; // string indicating flag status in CTF
inline constexpr int CS_SHADERSTATE = 24;
inline constexpr int CS_BOTINFO = 25;

inline constexpr int CS_ITEMS = 27; // string of 0's and 1's that tell which items are present

inline constexpr int CS_MODELS = 32;
#define	CS_SOUNDS				(CS_MODELS+MAX_MODELS)
#define	CS_PLAYERS				(CS_SOUNDS+MAX_SOUNDS)
#define CS_LOCATIONS			(CS_PLAYERS+MAX_CLIENTS)

#define CS_MAX					(CS_LOCATIONS+MAX_LOCATIONS)

#if (CS_MAX) > MAX_CONFIGSTRINGS
#error overflow: (CS_MAX) > MAX_CONFIGSTRINGS
#endif

enum gametype_t {
	GT_FFA,				// free for all
	GT_TOURNAMENT,		// one on one tournament
	GT_SINGLE_PLAYER,	// single player ffa
	GT_MISSION_EDITOR,	// editing MFQ3 missions

	//-- team games go after this --

	GT_TEAM,			// team deathmatch
	GT_CTF,				// capture the flag
	GT_MAX_GAME_TYPE
};

enum gender_t { GENDER_MALE, GENDER_FEMALE, GENDER_NEUTER };

/*
===================================================================================

PMOVE MODULE

The pmove code takes a player_state_t and a usercmd_t and generates a New player_state_t
and some other output data.  Used for local prediction on the client game and true
movement on the server game.
===================================================================================
*/

enum pmtype_t {
	PM_NORMAL,		// can accelerate and turn
	PM_VEHICLE,		// MFQ3
	PM_NOCLIP,		// noclip movement
	PM_SPECTATOR,	// still run into walls
	PM_DEAD,		// no acceleration or turning, but free falling
	PM_FREEZE,		// stuck in place with no control
	PM_INTERMISSION	// no movement or status bar
};

// pmove->pm_flags
inline constexpr int PMF_MFQ3_FREE1 = 1;
inline constexpr int PMF_RECHARGING = 2; // set when recharging
inline constexpr int PMF_VEHICLESPAWN = 4; // spawning vehicle
inline constexpr int PMF_ME_FREEZE = 8; // mission editor freeze (when manipulating objects)
inline constexpr int PMF_MFQ3_FREE3 = 16; // --- unused ---
inline constexpr int PMF_MFQ3_FREE5 = 32; // --- unused ---
inline constexpr int PMF_TIME_KNOCKBACK = 64; // pm_time is an air-accelerate only time
inline constexpr int PMF_MFQ3_FREE6 = 256; // --- unused ---
inline constexpr int PMF_RESPAWNED = 512; // clear after attack and jump buttons come up
inline constexpr int PMF_USE_ITEM_HELD = 1024;
inline constexpr int PMF_MFQ3_FREE = 2048; // --- unused ---
inline constexpr int PMF_FOLLOW = 4096; // spectate following another player
inline constexpr int PMF_SCOREBOARD = 8192; // spectate as a scoreboard
inline constexpr int PMF_VEHICLESELECT = 16384; // selecting vehicle
//#define PMF_INVULEXPAND		16384	// invulnerability sphere set to full size

#define	PMF_ALL_TIMES	(PMF_TIME_KNOCKBACK)

inline constexpr int MAXTOUCH = 32;
struct pmove_t {
	// state (in / out)
	playerState_t	*ps;

	// command (in)
	usercmd_t	cmd;
	int			tracemask;			// collide against these types of surfaces
	int			debugLevel;			// if set, diagnostic output will be printed

	int			framecount;

	// results (out)
	int			numtouch;
	int			touchents[MAXTOUCH];

	vec3_t		mins, maxs;			// bounding box size

	int			watertype;
	int			waterlevel;

	// for fixed msec Pmove
	int			pmove_msec;
	int			pmove_fixed;

	int			vehicle;		// MFQ3
	bool	updateGear;
	bool	updateBay;
	float		*gunnerAngles;

	int			advancedControls;	// MFQ3

	// callbacks to test the world
	// these will be different functions during game and cgame
	void		(*trace)( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentMask, bool capsule );
	int			(*pointcontents)( const vec3_t point, int passEntityNum );
};

// if a full pmove isn't done on the client, you can just update the angles
void PM_UpdateViewAngles( playerState_t *ps, const usercmd_t *cmd );
void Pmove (pmove_t *pmove);

//===================================================================================

// 3rd person camera vector interpretations

typedef enum {
	CAMERA_V_MIN,
	CAMERA_V_MAX,
	CAMERA_V_DEFAULT
};

// player_state->stats[] indexes
// NOTE: may not have more than 16
enum statIndex_t {
	STAT_HEALTH,
	STAT_CLIENTS_READY,				// bit mask of clients wishing to exit the intermission (FIXME: configstring?)
	STAT_MAX_HEALTH,				// health limit, changable by handicap
	STAT_FUEL,
	STAT_MAX_FUEL,
	STAT_LOCKINFO,
};

enum timerIndex_t {
	TIMER_FUEL,
	TIMER_THROTTLE,
	TIMER_GEAR,
	TIMER_GEARANIM,
	TIMER_BRAKE,
	TIMER_MACHINEGUN,
	TIMER_WEAPON,
	TIMER_BAY,
	TIMER_BAYANIM,
	TIMER_BAYCLOSE,
	TIMER_FLARE,
	TIMER_REVEAL
};

// player_state->persistant[] indexes
// these fields are the only part of player_state that isn't
// cleared on respawn
// NOTE: may not have more than 16
enum persEnum_t {
	PERS_SCORE,						// !!! MUST NOT CHANGE, SERVER AND GAME BOTH REFERENCE !!!
	PERS_HITS,						// total points damage inflicted so damage beeps can sound on change
	PERS_RANK,						// player rank or team rank
	PERS_TEAM,						// player team
	PERS_SPAWN_COUNT,				// incremented every respawn
	PERS_PLAYEREVENTS,				// 16 bits that can be flipped for events
	PERS_ATTACKER,					// clientnum of last damage inflicter
	PERS_KILLED,					// count of the number of times you died
	PERS_CAPTURES,					// captures
	PERS_DEATHS						// deaths
};


// entityState_t->eFlags
inline constexpr int EF_DEAD = 0x00000001; // don't draw a foe marker over players with EF_DEAD
inline constexpr int EF_TELEPORT_BIT = 0x00000004; // toggled every time the origin abruptly changes
inline constexpr int EF_PLAYER_EVENT = 0x00000010;
inline constexpr int EF_BOUNCE = 0x00000010; // for missiles
inline constexpr int EF_BOUNCE_HALF = 0x00000020; // for missiles
inline constexpr int EF_PILOT_ONBOARD = 0x00000040; // MFQ3 to see if pilots are in this vehicle
inline constexpr int EF_NODRAW = 0x00000080; // may have an event, but no model (unspawned items)
inline constexpr int EF_MFQ3_FREE2 = 0x00000100; // --- not used ---
inline constexpr int EF_MFQ3_FREE3 = 0x00000200; // --- not used ---
inline constexpr int EF_MOVER_STOP = 0x00000400; // will push otherwise
inline constexpr int EF_CONNECTION = 0x00002000; // draw a connection trouble sprite
inline constexpr int EF_VOTED = 0x00004000; // already cast a vote
inline constexpr int EF_TEAMVOTED = 0x00080000; // already cast a team vote

// NOTE: may not have more than 16
enum objective_t {	// NOTE: these are BITS to set and not a normal enum!!
	OB_REDFLAG = 1,
	OB_BLUEFLAG = 2

};


enum weapon_t {
	WP_MACHINEGUN,
	WP_WEAPON1,
	WP_WEAPON2,
	WP_WEAPON3,
	WP_WEAPON4,
	WP_WEAPON5,
	WP_WEAPON6,
	WP_FLARE
};

// entityState_t->event values
// entity events are for effects that take place reletive
// to an existing entities origin.  Very network efficient.

// two bits at the top of the entityState->event field
// will be incremented with each change in the event so
// that an identical event started twice in a row can
// be distinguished.  And off the value with ~EV_EVENT_BITS
// to retrieve the actual event number
inline constexpr int EV_EVENT_BIT1 = 0x00000100;
inline constexpr int EV_EVENT_BIT2 = 0x00000200;
#define	EV_EVENT_BITS		(EV_EVENT_BIT1|EV_EVENT_BIT2)

#define	EVENT_VALID_MSEC	300

enum entity_event_t {
	EV_NONE,

	EV_WATER_TOUCH,	// foot touches
	EV_WATER_LEAVE,	// foot leaves
	EV_WATER_UNDER,	// head touches
	EV_WATER_CLEAR,	// head leaves

	EV_ITEM_PICKUP,			// normal item pickups are predictable
	EV_GLOBAL_ITEM_PICKUP,	// powerup / team sounds are broadcast to everyone

	EV_FIRE_WEAPON,
	EV_FIRE_MG,
	EV_FIRE_FLARE,

	EV_ITEM_RESPAWN,
	EV_ITEM_POP,

	EV_GENERAL_SOUND,
	EV_GLOBAL_SOUND,		// no attenuation
	EV_GLOBAL_TEAM_SOUND,

	EV_MISSILE_HIT,
	EV_MISSILE_MISS,
	EV_MISSILE_MISS_METAL,
	EV_BULLET,				// otherEntity is the shooter

	EV_PAIN,
	EV_DEATH1,
	EV_DEATH2,
	EV_DEATH3,
	EV_OBITUARY,

	// MFQ3
	EV_VEHICLE_HIT,			// smoke puff when hit
	EV_VEHICLE_DIE,			// explosion when dying
	EV_VEHICLE_GIB,			// even bigger explosion when gibbed

	EV_BUILDING_EXPLODE,	// building explosion

	EV_GEAR_UP,				// start gear up anim
	EV_GEAR_DOWN,			// start gear down anim
	EV_GEAR_UP_FULL,		// bring gear up fully
	EV_GEAR_DOWN_FULL,		// bring gear down fully
	EV_GEAR_STOP,			// stop gear anim as it is

	EV_BAY_UP,				// start open anim
	EV_BAY_DOWN,			// start close anim
	EV_BAY_UP_FULL,			// close fully
	EV_BAY_DOWN_FULL,		// open fully
	EV_BAY_STOP,			// stop bay anim as it is

	EV_NUKE,

	EV_FLAK,

	EV_DEBUG_LINE

};


enum global_team_sound_t {
	GTS_RED_CAPTURE,
	GTS_BLUE_CAPTURE,
	GTS_RED_RETURN,
	GTS_BLUE_RETURN,
	GTS_RED_TAKEN,
	GTS_BLUE_TAKEN,
	GTS_REDTEAM_SCORED,
	GTS_BLUETEAM_SCORED,
	GTS_REDTEAM_TOOK_LEAD,
	GTS_BLUETEAM_TOOK_LEAD,
	GTS_TEAMS_ARE_TIED,
	GTS_NUKE				// come on a nuke just has to be a global sound
};


// animations
enum LQManimNumber_t {
	BOTH_DEATH1,
	BOTH_DEAD1,
	BOTH_DEATH2,
	BOTH_DEAD2,
	BOTH_DEATH3,
	BOTH_DEAD3,

	TORSO_GESTURE,

	TORSO_ATTACK,
	TORSO_ATTACK2,

	TORSO_DROP,
	TORSO_RAISE,

	TORSO_STAND,
	TORSO_STAND2,

	LEGS_WALKCR,
	LEGS_WALK,
	LEGS_RUN,
	LEGS_BACK,
	LEGS_SWIM,

	LEGS_JUMP,
	LEGS_LAND,

	LEGS_JUMPB,
	LEGS_LANDB,

	LEGS_IDLE,
	LEGS_IDLECR,

	LEGS_TURN,

	TORSO_GETFLAG,
	TORSO_GUARDBASE,
	TORSO_PATROL,
	TORSO_FOLLOWME,
	TORSO_AFFIRMATIVE,
	TORSO_NEGATIVE,

	MAX_ANIMATIONS,

	LEGS_BACKCR,
	LEGS_BACKWALK,
	FLAG_RUN,
	FLAG_STAND,
	FLAG_STAND2RUN,

	MAX_TOTALANIMATIONS
};


struct animation_s {
	int		firstFrame;
	int		numFrames;
	int		loopFrames;			// 0 to numFrames
	int		frameLerp;			// msec between frames
	int		initialLerp;		// msec to get to first frame
	int		reversed;			// true if animation is reversed
	int		flipflop;			// true if animation should flipflop back to base
};
using animation_t = animation_s;

//typedef enum {
//	TEAM_FREE,
//	TEAM_RED,
//	TEAM_BLUE,
//	TEAM_SPECTATOR,
//
//	TEAM_NUM_TEAMS
//} team_t;

// Time between location updates
inline constexpr int TEAM_LOCATION_UPDATE_TIME = 1000;

// How many players on the overlay
inline constexpr int TEAM_MAXOVERLAY = 32;

//team task
enum teamtask_t {
	TEAMTASK_NONE,
	TEAMTASK_OFFENSE, 
	TEAMTASK_DEFENSE,
	TEAMTASK_PATROL,
	TEAMTASK_FOLLOW,
	TEAMTASK_RETRIEVE,
	TEAMTASK_ESCORT,
	TEAMTASK_CAMP
};

// means of death
enum meansOfDeath_t {
	MOD_UNKNOWN,
	MOD_WATER,
	MOD_SLIME,
	MOD_LAVA,
	MOD_CRUSH,
	MOD_TELEFRAG,
	MOD_FALLING,
	MOD_SUICIDE,
	// now MFQ3 stuff (note grappling was removed!)
	MOD_FFAR,
	MOD_FFAR_SPLASH,
	MOD_IRONBOMB,
	MOD_IRONBOMB_SPLASH,
	MOD_AUTOCANNON,
	MOD_MAINGUN,
	MOD_VEHICLEEXPLOSION,
	MOD_CRASH,
	MOD_NUKE
};


//---------------------------------------------------------

// gitem_t->type
enum itemType_t {
	IT_BAD,
	IT_AMMO,				// EFX: rotate
	IT_HEALTH,				// EFX: static external sphere + rotating internal
	IT_FUEL,
	IT_TEAM
};

inline constexpr int MAX_ITEM_MODELS = 4;

struct gitem_s {
	char		*classname;	// spawning name
	char		*pickup_sound;
	char		*world_model[MAX_ITEM_MODELS];

	char		*icon;
	char		*pickup_name;	// for printing on pickup

	int			quantity;		// for ammo how much, or duration of powerup
	itemType_t  giType;			// IT_* flags

	int			giTag;

	char		*precaches;		// string of all models and images this item will use
	char		*sounds;		// string of all sounds this item will use
};
using gitem_t = gitem_s;

// included in both the game dll and the client
extern	gitem_t	bg_itemlist[];
extern	int		bg_numItems;

gitem_t	*BG_FindItem( const char *pickupName );
gitem_t	*BG_FindItemForPowerup( objective_t pw );
#define	ITEM_INDEX(x) ((x)-bg_itemlist)

bool	BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps, int idx );


// g_dmflags->integer flags
inline constexpr int DF_NO_FALLING = 8;
inline constexpr int DF_FIXED_FOV = 16;
inline constexpr int DF_NO_FOOTSTEPS = 32;

// content masks
inline constexpr int MASK_ALL = (-1);
#define	MASK_SOLID				(CONTENTS_SOLID)
#define	MASK_PLAYERSOLID		(CONTENTS_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BODY)
#define	MASK_DEADSOLID			(CONTENTS_SOLID|CONTENTS_PLAYERCLIP)
#define	MASK_WATER				(CONTENTS_WATER|CONTENTS_LAVA|CONTENTS_SLIME)
#define	MASK_OPAQUE				(CONTENTS_SOLID|CONTENTS_SLIME|CONTENTS_LAVA)
#define	MASK_SHOT				(CONTENTS_SOLID|CONTENTS_BODY|CONTENTS_CORPSE)


//
// entityState_t->eType
//
enum entityType_t {
	ET_GENERAL,
	ET_PLAYER,
	ET_ITEM,
	ET_MISSILE,
	ET_BULLET,
	ET_MOVER,
	ET_BEAM,
	ET_PORTAL,
	ET_SPEAKER,
	ET_TELEPORT_TRIGGER,
	ET_INVISIBLE,
	ET_VEHICLE,				// MFQ3
	ET_MISC_VEHICLE,		// MFQ3 // hmmm is this really necessary ?
	ET_EXPLOSIVE,			// func_runways and func_explosives

	ET_EVENTS				// any of the EV_* events can be added freestanding
							// by setting eType to ET_EVENTS + eventNum
							// this avoids having to set eFlags and eventNum
};



void	BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result );
void	BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result );

void	BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps );

void	BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, bool snap );
void	BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, bool snap );

bool	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime );


inline constexpr int ARENAS_PER_TIER = 4;
inline constexpr int MAX_ARENAS = 1024;
inline constexpr int MAX_SCRIPTS = 1024;
inline constexpr int MAX_ARENAS_TEXT = 8192;

inline constexpr int MAX_BOTS = 1024;
inline constexpr int MAX_BOTS_TEXT = 8192;
inline constexpr int MAX_MISSION_TEXT = 32000;

// MFQ3 
// (everything below)

// number of sounds for things
inline constexpr int NUM_TANKSOUNDS = 9;
inline constexpr int NUM_HELOSOUNDS = 1;
inline constexpr int NUM_EXPLOSION_SOUNDS = 4;

// modelname strings
inline constexpr int MF_MAX_MODELNAME = 128;

inline constexpr int MF_MAX_GAMESETS = 8;
inline constexpr int MF_MAX_TEAMS = 8;
inline constexpr int MF_MAX_CATEGORIES = 8;
inline constexpr int MF_MAX_CLASSES = 8;

// Which "ui_vehicleCat" are ground installtions
inline constexpr int UI_CAT_GI = 5;

// can now be any number of gamesets (full 8 byte)
// just expand list as needed and make sure ANY is set properly
inline constexpr int MF_GAMESET_MIN = 0x0001;
inline constexpr int MF_GAMESET_MODERN = 0x0001;
inline constexpr int MF_GAMESET_WW2 = 0x0002;
inline constexpr int MF_GAMESET_WW1 = 0x0004;
inline constexpr int MF_GAMESET_MAX = 0x0004;
inline constexpr int MF_GAMESET_ANY = 0xFFFF;

inline constexpr int MF_TEAM_MIN = 0x0001;
inline constexpr int MF_TEAM_1 = 0x0001;
inline constexpr int MF_TEAM_2 = 0x0002;
inline constexpr int MF_TEAM_MAX = 0x0002;
inline constexpr int MF_TEAM_ANY = 0xFFFF;

inline constexpr int CAT_MIN = 0x0001;
inline constexpr int CAT_PLANE = 0x0001;
inline constexpr int CAT_GROUND = 0x0002;
inline constexpr int CAT_HELO = 0x0004;
inline constexpr int CAT_LQM = 0x0008;
inline constexpr int CAT_BOAT = 0x0010;
inline constexpr int CAT_NPC = 0x0020;
inline constexpr int CAT_MAX = 0x0020;
inline constexpr int CAT_ANY = 0xFFFF;

inline constexpr int CLASS_MIN = 0x0001;
inline constexpr int CLASS_MAX = 0x0002;
inline constexpr int CLASS_ANY = 0xFFFF;

// plane classes				
inline constexpr int CLASS_PLANE_FIGHTER = 0x0001;
inline constexpr int CLASS_PLANE_BOMBER = 0x0002;
inline constexpr int CLASS_PLANE_TRANSPORT = 0x0004;
inline constexpr int CLASS_PLANE_MAX = 0x0004;

// helo classes					
inline constexpr int CLASS_HELO_ATTACK = 0x0001;
inline constexpr int CLASS_HELO_RECON = 0x0002;
inline constexpr int CLASS_HELO_TRANSPORT = 0x0004;
inline constexpr int CLASS_HELO_MAX = 0x0004;

// ground vehicle classes		
inline constexpr int CLASS_GROUND_MBT = 0x0001;
inline constexpr int CLASS_GROUND_RECON = 0x0002;
inline constexpr int CLASS_GROUND_APC = 0x0004;
inline constexpr int CLASS_GROUND_SAM = 0x0008;
inline constexpr int CLASS_GROUND_ARTY = 0x0016;
inline constexpr int CLASS_GROUND_MAX = 0x0016;

// LQM classes					
inline constexpr int CLASS_LQM_SPECIAL = 0x0001;
inline constexpr int CLASS_LQM_INFANTRY = 0x0002;
inline constexpr int CLASS_LQM_MAX = 0x0002;

// boat classes					
inline constexpr int CLASS_BOAT_PATROL = 0x0001;
inline constexpr int CLASS_BOAT_TRANSPORT = 0x0002;
inline constexpr int CLASS_BOAT_MAX = 0x0002;

// strings for categories and classes etc..
extern const char *gameset_items[MF_MAX_GAMESETS+1];
extern const char *gameset_codes[MF_MAX_GAMESETS+1];
extern const char *team_items[MF_MAX_GAMESETS][MF_MAX_TEAMS+1];
extern const char *cat_fileRef[MF_MAX_CATEGORIES+1];
extern const char *cat_items[MF_MAX_CATEGORIES+1];
extern const char *class_items[MF_MAX_CATEGORIES][MF_MAX_CLASSES+1];

// Vehicle flags
inline constexpr int VF_NOTSELECTABLE = 1; // cannot be selected (turrets, etc)
inline constexpr int VF_STEALTH = 2;

inline constexpr int SEATMASK_PILOT = 0x0001;
inline constexpr int SEATMASK_ENTERABLE = 0x8000;
inline constexpr int SEATMASK_DECK = 0x4000;

inline constexpr float STEALTH_RANGE_FACTOR = 0.25f;

// border speeds
inline constexpr float SPEED_YELLOW_ARC = 1.3f;
inline constexpr float SPEED_GREEN_ARC = 2.0f;

// body part defines
// planes
inline constexpr int BP_PLANE_BODY = 0;
inline constexpr int BP_PLANE_CONTROLS = 1;
inline constexpr int BP_PLANE_COCKPIT = 2;
inline constexpr int BP_PLANE_GEAR = 3;
inline constexpr int BP_PLANE_BRAKES = 4;
inline constexpr int BP_PLANE_BAY = 5;
inline constexpr int BP_PLANE_WINGLEFT = 6;
inline constexpr int BP_PLANE_WINGRIGHT = 7;
inline constexpr int BP_PLANE_SPECIAL = 8;
inline constexpr int BP_PLANE_PROP = 9;
inline constexpr int BP_PLANE_MAX_PARTS = 10;


// ground vehicles
inline constexpr int BP_GV_BODY = 0;
inline constexpr int BP_GV_TURRET = 1;
inline constexpr int BP_GV_GUNBARREL = 2;
inline constexpr int BP_GV_WHEEL = 3;
inline constexpr int BP_GV_WHEEL2 = 4;
inline constexpr int BP_GV_WHEEL3 = 5;
inline constexpr int BP_GV_WHEEL4 = 6;
inline constexpr int BP_GV_WHEEL5 = 7;
inline constexpr int BP_GV_WHEEL6 = 8;
inline constexpr int BP_GV_MAX_PARTS = 9;

// LQM
inline constexpr int BP_LQM_LEGS = 0;
inline constexpr int BP_LQM_TORSO = 1;
inline constexpr int BP_LQM_HEAD = 2;
//#define BP_LQM_WEAP			3
//#define BP_LQM_GUNBARREL		4
inline constexpr int BP_LQM_MAX_PARTS = 3;


// boats
inline constexpr int BP_BOAT_BODY = 0;
inline constexpr int BP_BOAT_TURRET = 1;
inline constexpr int BP_BOAT_GUNBARREL = 2;
inline constexpr int BP_BOAT_TURRET2 = 3;
inline constexpr int BP_BOAT_GUNBARREL2 = 4;
inline constexpr int BP_BOAT_TURRET3 = 5;
inline constexpr int BP_BOAT_GUNBARREL3 = 6;
inline constexpr int BP_BOAT_TURRET4 = 7;
inline constexpr int BP_BOAT_GUNBARREL4 = 8;
inline constexpr int BP_BOAT_MAX_PARTS = 9;

// helos
inline constexpr int BP_HELO_BODY = 0;
inline constexpr int BP_HELO_MAINROTOR = 1;
inline constexpr int BP_HELO_TAILROTOR = 2;
inline constexpr int BP_HELO_TURRET = 3;
inline constexpr int BP_HELO_GUNBARREL = 4;
inline constexpr int BP_HELO_TURRET2 = 5;
inline constexpr int BP_HELO_GUNBARREL2 = 6;
inline constexpr int BP_HELO_GEAR = 7;
inline constexpr int BP_HELO_BAY = 8;
inline constexpr int BP_HELO_PARTS = 9;
inline constexpr int BP_HELO_MAX_PARTS = 10;


// ground installations
inline constexpr int BP_GI_BODY = 0;
inline constexpr int BP_GI_TURRET = 1;
inline constexpr int BP_GI_GUNBARREL = 2;
inline constexpr int BP_GI_UPGRADE = 3;
inline constexpr int BP_GI_UPGRADE2 = 4;
inline constexpr int BP_GI_UPGRADE3 = 5;
inline constexpr int BP_GI_MAX_PARTS = 6;


// total max parts (no cat may exceed this!)
inline constexpr int BP_MAX_PARTS = 16;

// fallback modes if no custom shader can be found for a vehicle
inline constexpr int SHADOW_DEFAULT = -1;
inline constexpr int SHADOW_NONE = -2;

enum ShadowCoords {
	SHC_XOFFSET,
	SHC_YOFFSET,
	SHC_XADJUST,
	SHC_YADJUST
};

enum ShadowOrientationAdjusts {
	SHO_PITCHMAX,
	SHO_ROLLMAX,
	SHO_PITCHMOD,
	SHO_ROLLMOD
};


// pylon flags
inline constexpr int PF_NA = 0x0000; // nothing
inline constexpr int PF_AA_LT = 0x0001; // sidewinder, stinger
inline constexpr int PF_AA_MED = 0x0002; // amraam
inline constexpr int PF_AA_HVY = 0x0004; // sparrow
inline constexpr int PF_AA_PHX = 0x0008; // phoenix
inline constexpr int PF_AG_LT = 0x0010; // light bombs 500lb, stinger
inline constexpr int PF_AG_MED = 0x0020; // 800 lb
inline constexpr int PF_AG_HVY = 0x0040; // 1000lb, 2000lb
inline constexpr int PF_AG_GDA = 0x0080; // can carry guided bombs
inline constexpr int PF_FFAR = 0x0100; // FFAR
inline constexpr int PF_TANK_LT = 0x0200; // droptank
inline constexpr int PF_TANK_HVY = 0x0400; // droptank big
inline constexpr int PF_JAMMER = 0x0800; // jammer
inline constexpr int PF_GUNPOD = 0x1000; // gunpod
inline constexpr int PF_FLAREPOD = 0x2000; // extra flares
inline constexpr int PF_DONT_DROP = 0x4000; // dont drop on launch (wingtip)
inline constexpr int PF_HARM = 0x8000; // harm

inline constexpr int PF_DONT_REMOVE = 0x3900;

inline constexpr int MAX_LOADOUTS = 100;
inline constexpr int MAX_MOUNTS_PER_VEHICLE = 32;

struct mountInfo_t
{
	int			pos;	// pos on the wing
	int			group;	// group if any
	int			left;	// left or right side
	int			flags;	// what can it carry
	md3Tag_t	tag;	// tag
	int			weapon;	// what is actually on
	int			num;	// how many are actually on
};

struct completeLoadout_t
{
	mountInfo_t	mounts[MAX_MOUNTS_PER_VEHICLE];
	int			usedMounts;
	bool		mountsOnBay;
	animation_t	animations[MAX_TOTALANIMATIONS];
};


// list of vehicles (data)
struct completeVehicleData_t
{
    char		    *descriptiveName;	// long descriptive name
	char			*tinyName;			// small ident name
    char		    *modelName;			// just the directory of the model
	qhandle_t		shadowShader;	// the shadow shader (use SHADOW_DEFAULT to use the default shader or SHADOW_NONE for no shadow)
//    unsigned long   id;				// gameset, team, cat and class
	unsigned int	gameset;		// gameset 
	unsigned int	team;			// team 
	unsigned int	cat;			// category bits
	unsigned int	cls;			// class bits
	unsigned int	flags;			// special flags
	unsigned int	caps;			// capabilities
    qhandle_t	    handle[BP_MAX_PARTS];// ditto
    vec3_t		    mins;
    vec3_t		    maxs;
    vec3_t		    turnspeed;	    // how fast can it turn around the three axis
	vec3_t			cam_dist;		// how far away is the camera {min, max, defualt}
	vec3_t			cam_height;		// how high is the default {min, max, default}
    unsigned int    stallspeed;	
    unsigned int    maxspeed;	    // max speed at military thrust
    int			    minthrottle;    // can be less than 0 for ground vehicles
    int			    maxthrottle;    // if > 10 means afterburner
	int				engines;		// how many engines does it actually have
	int				wheels;			// how many wheels does it have 
	float			wheelCF;		// circumference
    unsigned int    accel;			// vehicle dependent acceleration
    unsigned int    maxhealth;	    // health
	vec3_t			gunoffset;		// guntag
	unsigned int	maxfuel;		// maximum fuel
	float			gearheight;		// height of gear
	float				tailangle;		// for taildraggers on ground
	unsigned int	weapons[MAX_WEAPONS_PER_VEHICLE];// use index from available Weapons
	unsigned int	ammo[MAX_WEAPONS_PER_VEHICLE];// how much of them (is also max_ammo)
	unsigned int	turret[MAX_WEAPONS_PER_VEHICLE];// on which turret is this weapon
	vec3_t			cockpitview;	// to place the camera
	unsigned int	effectModel;	// num of afterburner model (for planes)
	unsigned int	radarRange;		// how far goes the radar AIR
	unsigned int	radarRange2;	// how far goes the radar GV
	float			trackCone;		// how can radar track it
	float			trackCone2;		// how can ground radar track it
	float			swingangle;		// for swing wings
	unsigned int	gearTime;		// time it takes gear to operate
	unsigned int	maxGearFrame;	// number of gear animation frames - SET AUTOMATICALLY!
	unsigned int	bayTime;		// time it takes bay to operate
	unsigned int	maxBayFrame;	// number of bay animation frames - SET AUTOMATICALLY!
	unsigned int	renderFlags;	// special stuff for rendering only
	vec4_t			shadowCoords;	// shadow apply coords { offsetX, offsetY, xAdjust, yAdjust }
	vec4_t			shadowAdjusts;	// shadow apply adjustments { pitchMax, rollMax, pitchMod, rollMod }
	animation_t		*animations;	// Animation pointer
	bool			hasDeck;
	vec3_t			deckPos;
	int				seats;
};

extern completeVehicleData_t availableVehicles[];

bool MF_StealthHidden( int vehicle, int revealTime, int time, float dist, float radarRange );
bool MF_VehicleHasTurret( int vehicle );

// number of available vehicles
extern int bg_numberOfVehicles;

extern completeLoadout_t availableLoadouts[MAX_LOADOUTS];

// types of weapons
enum weaponType_t {
	WT_NONE = -1,
	WT_MACHINEGUN,
	WT_BALLISTICGUN,
	WT_ROCKET,
	WT_IRONBOMB,
	WT_GUIDEDBOMB,
	WT_ANTIAIRMISSILE,
	WT_ANTIGROUNDMISSILE,
	WT_ANTIRADARMISSILE,
	WT_FUELTANK,
	WT_FLARE,
	WT_FLAK,
	WT_NUKEBOMB,
	WT_NUKEMISSILE,
	WT_FUELCRATE,
	WT_AMMOCRATE,
	WT_HEALTHCRATE
};

// weaponflags
inline constexpr int WF_NONE = 0;
inline constexpr int WF_HAS_FIRE_FRAME = 1; // for missiles
inline constexpr int WF_FIRE_IN_PAIRS = 2; // for bombs, tanks etc


// list of ground installations
struct groundInstallationData_t
{
    char		    *descriptiveName;	// long descriptive name
	char			*tinyName;			// small ident name
    char		    *modelName;			// just the directory of the model
	unsigned int	gameset;		// gameset 
    qhandle_t	    handle[BP_MAX_PARTS];// ditto
    vec3_t		    mins;
    vec3_t		    maxs;
    vec3_t		    turnspeed;	    // how fast can it turn around the three axis
	float			maxpitch;		// how far can it look up
	float			minpitch;		// how far can it look down
    unsigned int    maxhealth;	    // health
	unsigned int	weapon;
	unsigned int	ammo;
	unsigned int	radarRange;		// how far goes the radar AIR
	unsigned int	radarRange2;	// how far goes the radar GV
	float			trackCone;		// how can radar track it
	float			trackCone2;		// how can ground radar track it
	unsigned int	upgrades;		// for larger models (for double, triple etc load);
	int				reloadTime;		// how long after last firing it takes to reload a weapon
};

extern groundInstallationData_t availableGroundInstallations[];

// number of available vehicles
extern int bg_numberOfGroundInstallations;


// list of weapons
struct completeWeaponData_t
{
	weaponType_t	type;				// behaviour
	unsigned int	gameset;			// in which epoche can it appear
	char			*descriptiveName;	// displayed in-game
	char			*shortName;			// displayed in-game, short form
	char			*shortName2;		// displayed in-game, short form, but for GVs
	char			*modelName;			// model
	qhandle_t		modelHandle;		// handle
	char			*vwepName;			// vwep model
	qhandle_t		vwepHandle;			// vwep handle
	char			*iconName;			// icon for HUD
	qhandle_t		iconHandle;			// handle for icon
	unsigned int	category;			// which category can it damage
	float			noncatmod;			// modifier for hitting wrong category
	unsigned int	muzzleVelocity;		// speed at which it starts
	float			range;				// target acquiring range
	unsigned int	fuelrange;			// for how long lasts its fuel
	unsigned int	fireInterval;		// time between two shots
	unsigned int	damage;				// damage done to target
	unsigned int	damageRadius;		// radius that damage is applied to
	unsigned int	spread;				// how far off the target direction
	unsigned int	barrels;			// mainly for guns
	float			barrelDistance;		// how far off the center
	float			lockcone;			// degrees of half cone to lock
	float			followcone;			// degrees of half cone to follow
	int				lockdelay;			// how long does it take to lock on
	unsigned int	crosshair;			// which crosshair to use when this weapon is selected
	unsigned int	crosshairtrack;		// which crosshair to use when this weapon is tracking
	unsigned int	crosshairlock;		// which crosshair to use when this weapon is locked
	unsigned int	turret;				// is it a turret
	vec3_t			maxturns;			// max turn angles if turret
	vec3_t			minturns;			// min turn angles if turret
	unsigned int	numberPerPackage;	// for example how many rockets in FFAR etc (per mount)
	unsigned int	fitsCategory;		// which vehicle can carry this weapon
	unsigned int	fitsPylon;			// on which type of pylon does this go 
	unsigned int	basicECMVulnerability;// how likely in percent to be distracted
	unsigned int	flags;
};


// list of weapons...
extern completeWeaponData_t	availableWeapons[];

// number of available vehicles
extern int bg_numberOfWeapons;

// ...and their index (make sure in symc with availableWeapons[] !!
enum weaponIndex_t
{
	WI_NONE,			// 0
	WI_MG_2XCAL303,
	WI_MG_2XCAL312,
	WI_MG_8XCAL50,
	WI_MG_6XCAL50,
	WI_MG_12_7MM,
	WI_MG_14_5MM,
	WI_MG_4X14_5MM,
	WI_MG_20MM,
	WI_MG_30MM,
	WI_MG_2X20MM,
	WI_MG_M4A1,
	WI_MGT_2X30MM,		// 10
	WI_MGT_7_62MM,
	WI_MGT_12_7MM,
	WI_FFAR,
	WI_FFAR_SMALL,
	WI_FFAR_LARGE,
	WI_100MM_GUN,
	WI_125MM_GUN,
	WI_MK82,
	WI_MK82R,
	WI_MK83,
	WI_MK83R,			// 20
	WI_MK84,
	WI_GBU15,			
	WI_DURANDAL,		
	WI_SIDEWINDER,
	WI_AMRAAM,
	WI_SPARROW,
	WI_PHOENIX,
	WI_STINGER,
	WI_ATOLL,			
	WI_ARCHER,			// 30
	WI_ALAMO,
	WI_HELLFIRE,	
	WI_MAVERICK,
	WI_HARM,
	WI_AASAM,
	WI_ECM,
	WI_LASE,
	WI_CM,				//This is like WI_FLARE, but will show up on a pylon instead of just being a value concerning the internal flare count
	WI_DROPTANK,
	WI_DROPTANK_PAIR,	// 40
	WI_DROPTANK_SMALL,	
	WI_DROPTANK_SMALL_PAIR,
	WI_FUELCRATE,
	WI_HEALTHCRATE,
	WI_AMMOCRATE,
	WI_FLARE,	
	WI_CFLARE,
	WI_BURNINGMAN,
	WI_NB10MT,
	WI_NB5MT,
	WI_NB1MT,
	WI_NM10MT,
	WI_NM5MT,
	WI_NM1MT,
	WI_FLAK,
	WI_S8,
	WI_S5,
	WI_AT6,
	WI_MGT_20MM,
	WI_MGT_YAKB,
	WI_MAX
};


// IGME
inline constexpr int IGME_MAX_VEHICLES = 64;
inline constexpr int IGME_MAX_WAYPOINTS = 32;

struct mission_waypoint_t {
	bool		used;
	vec3_t			origin;
};

// mission scripts
struct mission_overview_t {
	char			mapname[MAX_NAME_LENGTH];
	int				gameset;
	int				gametype;			
	char			missionname[MAX_NAME_LENGTH];
	char			objective[MAX_NAME_LENGTH];
	bool		valid;
};

struct mission_groundInstallation_t {
	bool		used;
	int				index;
	char			objectname[MAX_NAME_LENGTH];
	char			descriptivename[MAX_QPATH];
	char			teamname[MAX_NAME_LENGTH];
	vec3_t			origin;
	vec3_t			angles;
};


struct mission_vehicle_t {
	bool		used;
	int				index;
	char			objectname[MAX_NAME_LENGTH];
	char			descriptivename[MAX_QPATH];
	int				team;
	char			side[MAX_NAME_LENGTH];
	vec3_t			origin;
	vec3_t			angles;
	mission_waypoint_t waypoints[IGME_MAX_WAYPOINTS];
};












void MF_ParseMissionScripts( char *buf, mission_overview_t* overview, 
		mission_vehicle_t* vehs, mission_groundInstallation_t* gis);
void MF_CheckMissionScriptOverviewValid( mission_overview_t* overview, bool updateFormat );
void MF_SetMissionScriptOverviewDefaults( mission_overview_t* overview );
int MF_getIndexOfVehicle( int start, int gameset, int team, int cat, int cls, int vehicleType, int change_vehicle, bool allowNukes );
int MF_getIndexOfVehicleEx( int start, int gameset, int team, int cat, int cls, int vehicleType, int change_vehicle, bool allowNukes );
int MF_getIndexOfGI( int start, int gameset, int GIType, int mode);
int MF_getItemIndexFromHex(int hexValue);
int MF_getNumberOfItems(const char **itemlist);
char * MF_CreateModelPathname( int vehicle, char * pFormatString );
char * MF_CreateGIPathname( int vehicle, char * pFormatString );
void MF_LimitFloat( float * value, float min, float max );
void MF_LimitInt( int * value, int min, int max );
int MF_ExtractEnumFromId( int vehicle, unsigned int op );
bool MF_findTag(const char* fileName, const char* tagname, md3Tag_t* tag);
bool MF_distributeWeaponsOnPylons( int idx, completeLoadout_t* loadout );
void MF_calculateAllDefaultLoadouts();
void MF_getDefaultLoadoutForVehicle( int idx, completeLoadout_t* loadout, playerState_t* ps );
bool MF_removeWeaponFromLoadout( int weaponIndex, completeLoadout_t* loadout, playerState_t* ps,
									 bool* wingtip, vec3_t pos, int launchPos );
int MF_addWeaponToLoadout( int weaponIndex, completeLoadout_t* loadout, playerState_t* ps );
bool MF_getNumberOfFrames(const char* fileName, int* number);
bool MF_getNumberOfTags(const char* fileName, int* number);
int MF_getTagsContaining(const char* fileName, const char* str, md3Tag_t* tags, int num);
bool MF_getDimensions(const char* fileName, int frame, vec3_t* maxs, vec3_t* mins);
void MF_LoadAllVehicleData();
int MF_findWeaponsOfType( int weaponIndex, completeLoadout_t* loadout );

inline constexpr int MF_THROTTLE_REVERSE = -5;
inline constexpr int MF_THROTTLE_IDLE = 0;
inline constexpr int MF_THROTTLE_MILITARY = 10;
inline constexpr int MF_THROTTLE_AFTERBURNER = 15;

// this is for the PW_HASCAN_FLAGS
inline constexpr int HC_NOTHING = 0;
inline constexpr int HC_GEAR = 1;
inline constexpr int HC_SPEEDBRAKE = 2;
inline constexpr int HC_PROP = 4;
inline constexpr int HC_VAPOR = 8;
inline constexpr int HC_TAILDRAGGER = 16;
inline constexpr int HC_DUALGUNS = 32; // will be removed
inline constexpr int HC_SWINGWING = 64;
inline constexpr int HC_AMPHIBIOUS = 128;
inline constexpr int HC_WHEELS = 256; // vehicle has wheels (rather than tracks)
inline constexpr int HC_WEAPONBAY = 512;

// LQM Animations (replace instead of using frame index, and let CG read animation.cfg)
inline constexpr float LQM_SCALE = 0.14f;
inline constexpr int A_LQM_STAND = 0;
inline constexpr int A_LQM_FORWARD = 1;
inline constexpr int A_LQM_BACKWARD = 2;
inline constexpr int A_LQM_CROUCH = 4;
inline constexpr int A_LQM_LEFT = 8;
inline constexpr int A_LQM_RIGHT = 16;
inline constexpr int A_LQM_EJECT = 32;
inline constexpr int A_LQM_FLY = 64;
inline constexpr int A_LQM_DIE = 128;
#define A_LQM_JUMP				(A_LQM_CROUCH|A_LQM_EJECT)

// this is for the PW_ONOFF_FLAGS
inline constexpr int OO_NOTHING_ON = 0;
inline constexpr int OO_STALLED = 1;
inline constexpr int OO_LANDED = 2;
inline constexpr int OO_SPEEDBRAKE = 4;
inline constexpr int OO_GEAR = 8;
inline constexpr int OO_VAPOR = 16;
inline constexpr int OO_WEAPONBAY = 32;
inline constexpr int OO_RADAR_AIR = 64;
inline constexpr int OO_RADAR_GROUND = 128;
inline constexpr int OO_RADAR = 192; // OO_RADAR_GROUND|OO_RADAR_AIR
inline constexpr int OO_LANDEDTERRAIN = 256;

// afterburners
inline constexpr int AB_BALL = 0;
inline constexpr int AB_BLUE = 1;
inline constexpr int AB_RED = 2;
inline constexpr int AB_RED_SMALL = 3;
inline constexpr int AB_MAX = 4;

// render flags
inline constexpr int MFR_DUALPILOT = 1;
inline constexpr int MFR_BIGVAPOR = 2;
inline constexpr int MFR_VWEP = 4;

// misc
inline constexpr int MAX_MUZZLEFLASHES = 8;
inline constexpr int MAX_PILOTS = 2;

// crosshairs
enum CrossHairs_t {
	CH_DEFAULT,
	CH_GUNMODE,
	CH_BOMBMODE,
	CH_ROCKETMODE,
	CH_MISSILEMODE,
	CH_MISSILEMODETRACK,
	CH_MISSILEMODELOCK,
	CH_NUMCROSSHAIRS
};

// lock flags
inline constexpr int LI_NOLOCK = 0;
inline constexpr int LI_TRACKING = 1;
inline constexpr int LI_LOCKING = 2;
inline constexpr int LI_BEING_LOCKED = 4;
inline constexpr int LI_BEING_LAUNCHED = 8;

// gear anim frames
inline constexpr int GEAR_UP = 0;
inline constexpr int GEAR_DOWN_DEFAULT = 47;
inline constexpr int GEAR_DOWN_MAX = 200;

inline constexpr int GEAR_ANIM_STOP = 0;
inline constexpr int GEAR_ANIM_UP = 1;
inline constexpr int GEAR_ANIM_DOWN = 2;

// bay anim frames
inline constexpr int BAY_UP = 0;
inline constexpr int BAY_DOWN_DEFAULT = 47;
inline constexpr int BAY_DOWN_MAX = 200;

inline constexpr int BAY_ANIM_STOP = 0;
inline constexpr int BAY_ANIM_UP = 1;
inline constexpr int BAY_ANIM_DOWN = 2;

/////////////////////
// Nuke
/////////////////////
// 1st shockwave times
inline constexpr int NUKE_SHOCKWAVE_STARTTIME = 0;
inline constexpr int NUKE_SHOCKWAVEFADE_STARTTIME = 1500;
inline constexpr int NUKE_SHOCKWAVE_ENDTIME = 2000;
// explosion/implosion times
inline constexpr int NUKE_EXPLODE_STARTTIME = 250;
inline constexpr int NUKE_IMPLODE_STARTTIME = 2000;
inline constexpr int NUKE_IMPLODE_ENDTIME = 2250;
// 2nd shockwave times
inline constexpr int NUKE_SHOCKWAVE2_STARTTIME = 2000;
inline constexpr int NUKE_SHOCKWAVE2FADE_STARTTIME = 2500;
inline constexpr int NUKE_SHOCKWAVE2_ENDTIME = 3000;
// nuke cloud times
inline constexpr int NUKE_CLOUD_FRAMES = 101;
inline constexpr int NUKE_CLOUD_STARTTIME = 2500;
inline constexpr int NUKE_CLOUD_FADETIME = 20000;
inline constexpr int NUKE_CLOUD_ENDTIME = 30000;

// radius of the models without scaling
inline constexpr int NUKE_SHOCKWAVEMODEL_RADIUS = 88;
inline constexpr int NUKE_BOOMSPHEREMODEL_RADIUS = 72;
// maximum radius of the models during the effect
inline constexpr int NUKE_SHOCKWAVE_MAXRADIUS = 1000;
inline constexpr int NUKE_SHOCKWAVE2_MAXRADIUS = 1000;



#endif // __BG_PUBLIC_H__
