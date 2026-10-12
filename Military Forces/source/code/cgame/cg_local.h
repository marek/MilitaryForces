/*
 * $Id: cg_local.h,v 1.21 2007-06-17 17:11:12 minkis Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#ifndef __CG_LOCAL_H__
#define __CG_LOCAL_H__

#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "tr_types.h"
#include "../game/bg_public.h"
#include "cg_public.h"
#include "../qcommon/proportional_font.h"
#include "../qcommon/client_host.h"
#include "../ui/ui_shared.h"
#include "../renderercommon/tr_public.h"

// used for 2D drawing using the shared UI code
//extern displayContextDef_t * DC;

// global client enumerations

enum explosion_t {
	EXPLODE_VEHICLE_DIE,
	EXPLODE_VEHICLE_GIB,
	EXPLODE_BUILDING
};

enum quality_t {
	LOW_QUALITY,
	MEDIUM_QUALITY,
	HIGH_QUALITY
};

// The entire cgame module is unloaded and reloaded on each level change,
// so there is NO persistant data between levels on the client side.
// If you absolutely need something stored, it can either be kept
// by the server in the server stored userinfos, or stashed in a cvar.

inline constexpr int MAX_RETICLES = 3;	// the tracked target, its lock, and the gun

inline constexpr int POWERUP_BLINKS = 5;

inline constexpr int POWERUP_BLINK_TIME = 1000;
inline constexpr int FADE_TIME = 200;
inline constexpr int PULSE_TIME = 200;
inline constexpr int DAMAGE_DEFLECT_TIME = 100;
inline constexpr int DAMAGE_RETURN_TIME = 400;
inline constexpr int DAMAGE_TIME = 500;
inline constexpr int LAND_DEFLECT_TIME = 150;
inline constexpr int LAND_RETURN_TIME = 300;
inline constexpr int STEP_TIME = 200;
inline constexpr int DUCK_TIME = 100;
inline constexpr int PAIN_TWITCH_TIME = 200;
inline constexpr int WEAPON_SELECT_TIME = 1400;
inline constexpr int ITEM_SCALEUP_TIME = 1000;
inline constexpr int ZOOM_TIME = 150;
inline constexpr int ITEM_BLOB_TIME = 200;
inline constexpr int MUZZLE_FLASH_TIME = 20;
inline constexpr int SINK_TIME = 1000; // time for fragments to sink into ground before going away
inline constexpr int ATTACKER_HEAD_TIME = 10000;
inline constexpr int REWARD_TIME = 3000;

inline constexpr double PULSE_SCALE = 1.5; // amount to scale up the icons when activating

inline constexpr int MAX_STEP_CHANGE = 32;

inline constexpr int MAX_VERTS_ON_POLY = 10;
inline constexpr int MAX_MARK_POLYS = 1024;

inline constexpr int STAT_MINUS = 10; // num frame for '-' stats digit
inline constexpr int STAT_COLON = 11; // num frame for ':' stats digit
inline constexpr int STAT_PERCENT = 12; // num frame for '%' stats digit
inline constexpr int STAT_POINT = 13; // num frame for '.' stats digit

inline constexpr int HUDNUM_WIDTH = 8;
inline constexpr int HUDNUM_HEIGHT = 8;

inline constexpr int ICON_SIZE = 48;
inline constexpr int HUDCHAR_WIDTH = 32;
inline constexpr int HUDCHAR_HEIGHT = 48;
inline constexpr int TEXT_ICON_SPACE = 4;

inline constexpr int TEAMCHAT_WIDTH = 80;
inline constexpr int TEAMCHAT_HEIGHT = 8;

// very large characters
inline constexpr int GIANT_WIDTH = 32;
inline constexpr int GIANT_HEIGHT = 48;

inline constexpr int TEAM_OVERLAY_MAXNAME_WIDTH = 12;
inline constexpr int TEAM_OVERLAY_MAXLOCATION_WIDTH = 16;

#define DEFAULT_REDTEAM_NAME		"Warsaw Pact"
#define DEFAULT_BLUETEAM_NAME		"NATO"

enum class impactSound_t {
	IMPACTSOUND_DEFAULT,
	IMPACTSOUND_METAL,
	IMPACTSOUND_FLESH
};

using enum impactSound_t;

// radar
inline constexpr int RD_GROUND_ENEMY = 0;
inline constexpr int RD_GROUND_FRIEND = 1;
inline constexpr int RD_GROUND_UNKNOWN = 2;
inline constexpr int RD_BUILDING_ENEMY = 3;
inline constexpr int RD_BUILDING_FRIEND = 4;
inline constexpr int RD_BUILDING_UNKNOWN = 5;
inline constexpr int RD_AIR_SAME_ENEMY = 6;
inline constexpr int RD_AIR_SAME_FRIEND = 7;
inline constexpr int RD_AIR_SAME_UNKNOWN = 8;
inline constexpr int RD_AIR_HIGH_ENEMY = 9;
inline constexpr int RD_AIR_HIGH_FRIEND = 10;
inline constexpr int RD_AIR_HIGH_UNKNOWN = 11;
inline constexpr int RD_AIR_LOW_ENEMY = 12;
inline constexpr int RD_AIR_LOW_FRIEND = 13;
inline constexpr int RD_AIR_LOW_UNKNOWN = 14;
inline constexpr int RD_MAX_ICONS = 15;

inline constexpr int MAX_RADAR_TARGETS = 32;

// MFD
enum MFD_Modes {
	MFD_OFF,
	MFD_RWR,
	MFD_STATUS,
	MFD_INVENTORY,
	MFD_CAMERA,
	MFD_INFO,
	MFD_MAX					
};

inline constexpr int MFD_1 = 0;
inline constexpr int MFD_2 = 1;
inline constexpr int NUM_MFDS_IN_COCKPIT = 2;

// cameramode
enum Camera_Modes {
	CAMERA_TARGET,
	CAMERA_BACK,
	CAMERA_DOWN,
	CAMERA_BOMB,
	CAMERA_MAX		
};

inline constexpr int MAX_CONSOLE_LINES = 8;

// custom console
struct strConsoleLine {
	char text[ 1024 ];	// TODO: possibly make this dynamic to save memory
	float life;
};

//=================================================

// player entities need to track more information
// than any other type of entity.

// note that not every player entity is a client entity,
// because corpses after respawn are outside the normal
// client numbering range

struct playerEntity_t {
	int				painTime;
	int				painDirection;	// flip from 0 to 1

};

//=================================================



// ClientEntity have a direct corespondence with gentity_t in the game, but
// only the entityState_t is directly communicated to the cgame
inline constexpr int ANGLE_ERROR_DECAY = 150;

struct clientInfo_t;
class LocalEntity;
struct particle_s;
using particle_t = particle_s;
struct BasicDrawInfo_t;

class ClientEntity
{
public:
	void boat( clientInfo_t *ci );
	void draw_MFD( int mfdnum, int vehicle, int targetrange, int targetheading, int altitude, playerState_t *ps, int stallcolor );
	LocalEntity * generic_Smoke( vec3_t smokePosition, int density );
	void nukeEffect( entityState_t * es );
	void flakEffect( entityState_t * es );
	void setEntitySoundPosition();
	void entityEffects();
	void general();
	void speaker();
	void item();
	void missile();
	void bullet_Tracer();
	void mover();
	void explosive();
	void beam();
	void portal();
	float entityAngleErrorFraction() const;
	void interpolateEntityPosition();
	void vehicleLerpAngles();
	void calcEntityLerpPositions();
	void emptyVehicleIcon();
	void addCEntity();
	void painEvent( int health );
	void entityEvent( vec3_t position );
	void checkEvents();
	void groundVehicleFlags();
	void groundVehicle( clientInfo_t *ci );
	void heloFlags();
	void helo( clientInfo_t *ci );
	void lQMFlags();
	void lQM( clientInfo_t *ci );
	void misc_Plane();
	void misc_GV();
	void misc_Helo();
	void misc_LQM();
	void misc_Boat();
	void misc_GI();
	void misc_Vehicle();
	void removeParticleSystemFromLocalEntity();
	void removeParticleSystemFromCEntity();
	void attractParticleToEntity( particle_t * a );
	void planeFlags();
	void plane( clientInfo_t *ci );
	void trailItem( qhandle_t hModel );
	void touchItem();
	void resetEntity();
	void transitionEntity();
	int loadoutVehicle();
	completeLoadout_t* entityLoadout();
	void setDrawLoadout( BasicDrawInfo_t* info );
	void vehicle();
	void fireMachinegun( bool main );
	void ejectCasings( bool main, int weaponIndex );
	void fireWeapon();

	entityState_t	currentState;	// from cg.frame
	entityState_t	nextState;		// from cg.nextFrame, if available
	bool		interpolate;	// true if next is valid to interpolate to
	bool		currentValid;	// true if cg.frame holds this entity

	int				muzzleFlashTime;	// move to playerEntity?
	int				previousEvent;

	int				trailTime;		// so missile trails can handle dropped initial packets
	int				miscTime;
	int				TimeSinceLastTrail;	// Time since last trail was drawn


	int				snapShotTime;	// last time this entity was found in a snapshot

	playerEntity_t	pe;

	int				errorTime;		// decay the error from this time
	int				errorSnapTime;
	Vec3			errorAngles;
	Vec3			errorAngles2;
	
	bool		extrapolated;	// false if origin / angles is an interpolation

	Vec3			lastDrawnTrailPos;		// Position of last drawn trail
	// exact interpolated position of entity on this frame
	Vec3			lerpOrigin;
	Vec3			lerpAngles;
	Vec3			lerpAngles2;

	bool		destroyableStructure;

	int				gearAnim;
	int				gearAnimFrame;
	int				gearAnimStartTime;

	int				bayAnim;
	int				bayAnimFrame;
	int				bayAnimStartTime;

	int				muzzleFlashWeapon;
	int				stationsFired;		// gun positions of the last machine gun shot, one bit each
};


//======================================================================

// local entities are created as a result of events or predicted actions,
// and live independantly from all server transmitted entities

struct markPoly_s {
	struct markPoly_s	*prevMark, *nextMark;
	int			time;
	qhandle_t	markShader;
	bool	alphaFade;		// fade alpha instead of rgb
	float		color[4];
	poly_t		poly;
	polyVert_t	verts[MAX_VERTS_ON_POLY];
};
using markPoly_t = markPoly_s;


enum class leType_t {
	LE_MARK,
	LE_EXPLOSION,
	LE_SPRITE_EXPLOSION,
	LE_FRAGMENT,
	LE_MOVE_SCALE_FADE,
	LE_FADE_RGB,
	LE_SCALE_FADE,
	LE_NUKE
};


using enum leType_t;

enum class leFlag_t {
	LEF_NONE			 = 0,
	LEF_PUFF_DONT_SCALE  = 0x0001,			// do not scale size over time
	LEF_TUMBLE			 = 0x0002,			// tumble over time, used for ejecting shells
	LEF_NO_RADIUS_KILL	 = 0x0004,			// don't ()->freeLocalEntity() just because the camera is 'inside' the radius of a sprite (e.g. smoke puffs)
	LEF_SOUND1			 = 0x0008,			// sound 1 for nuke
	LEF_SOUND2			 = 0x0010			// sound 2 for nuke
};
using enum leFlag_t;
constexpr bool isFlagSet( leFlag_t ) { return true; }

enum class leMarkType_t {
	LEMT_NONE,
	LEMT_BURN,
	LEMT_BLOOD
};

using enum leMarkType_t;			// fragment local entities can leave marks on walls

enum class leBounceSoundType_t {
	LEBS_NONE,
	LEBS_BRASS,
	LEBS_BLOOD
};

using enum leBounceSoundType_t;	// fragment local entities can make sounds on impacts

class LocalEntity
{
public:
	void freeLocalEntity();
	void fragmentBounceMark( trace_t *trace );
	void reflectVelocity( trace_t *trace );
	void addFragment();
	void addFadeRGB();
	void addMoveScaleFade();
	void addScaleFade();
	void addExplosion();
	void addSpriteExplosion();
	void addNuke();

	LocalEntity	*prev, *next;
	leType_t		leType;
	leFlag_t		leFlags;

	int				startTime;
	int				endTime;
	int				fadeInTime;

	float			lifeRate;			// 1.0 / (endTime - startTime)

	trajectory_t	pos;
	trajectory_t	angles;

	float			bounceFactor;		// 0.0 = no bounce, 1.0 = perfect

	float			color[4];

	float			radius;

	float			light;
	Vec3			lightColor;

	leMarkType_t		leMarkType;		// mark to leave on fragment impact
	leBounceSoundType_t	leBounceSoundType;

	refEntity_t		refEntity;		
};

//======================================================================


struct score_t {
	int				client;
	int				score;
	int				ping;
	int				time;
	int				scoreFlags;
	int				powerUps;
	int				accuracy;
	int				impressiveCount;
	int				excellentCount;
	int				guantletCount;
	int				defendCount;
	int				assistCount;
	int				captures;
	int				deaths;
	int				perfect;
	int				team;
};

// each client has an associated clientInfo_t
// that contains media references necessary to present the
// client model and other color coded effects
// this is regenerated each time a client's configstring changes,
// usually as a result of a userinfo (name, model, etc) change
inline constexpr int MAX_CUSTOM_SOUNDS = 32;

struct clientInfo_t {
	bool		infoValid;

	char			name[MAX_QPATH];
	ClientBase::eTeam team;
	int				paintTeam;
	bool			paintBlown;

	int				botSkill;		// 0 = not bot, 1-5 = bot

	Vec3			color;

	int				score;			// updated by score servercmds
	int				location;		// location index for team mode
	int				health;			// you only get this info about your teammates
	int				curWeapon;

	int				handicap;
	int				wins, losses;	// in tourney mode

	int				teamTask;		// task in teamplay (offence/defence)
	int				teamLeader;		// true when this is a team leader

	int				objectives;		// so can display flag status


	char			redTeam[MAX_TEAMNAME];
	char			blueTeam[MAX_TEAMNAME];

	gender_t		gender;			// from model

	sfxHandle_t		sounds[MAX_CUSTOM_SOUNDS];

	// mfq3 specific
	int				vehicle;
	int				deaths;
	qhandle_t		parts[BP_MAX_PARTS];
	int				advancedControls;
};

// each WP_* weapon enum has an associated weaponInfo_t
// that contains media references necessary to present the
// weapon and its effects
struct weaponInfo_s {
	bool		registered;

	float			flashDlight;
	Vec3			flashDlightColor;
	sfxHandle_t		flashSound[4];		// fast firing weapons randomly choose

	qhandle_t		ammoModel;

	qhandle_t		missileModel;
	sfxHandle_t		missileSound;
	void			(*missileTrailFunc)( ClientEntity *, const struct weaponInfo_s *wi );
	float			missileDlight;
	Vec3			missileDlightColor;
	int				missileRenderfx;

	float			trailRadius;
	float			wiTrailTime;

	sfxHandle_t		readySound;
	sfxHandle_t		firingSound;
};
using weaponInfo_t = weaponInfo_s;


// each IT_* item has an associated itemInfo_t
// that constains media references necessary to present the
// item and its effects
struct itemInfo_t {
	bool		registered;
	qhandle_t		models[MAX_ITEM_MODELS];
	qhandle_t		icon;
};


struct powerupInfo_t {
	int				itemNum;
};

// reticle structure passed to the HUD code for rendering
struct reticle_t {
	int ox;	// real x & y
	int oy;
	int x;	// draw x & y
	int y;
	int w;	// width & height
	int h;
	qhandle_t shader;		// shader index (gfx)
	ClientEntity * pTarget;	// target entity pointer (can be NULL)
};

inline constexpr int MAX_REWARDSTACK = 10;
inline constexpr int MAX_SOUNDBUFFER = 20;

//======================================================================

// occurs, and they will have visible effects for #define STEP_TIME or whatever msec after

inline constexpr int MAX_PREDICTED_EVENTS = 16;
 
struct cg_t {
	int			clientFrame;		// incremented each frame

	int			clientNum;
	
	bool		demoPlayback;
	bool		levelShot;			// taking a level menu screenshot
	int			deferredPlayerLoading;
	int			intermissionStarted;	// don't play voice rewards, because game will end shortly

	// there are only one or two snapshot_t that are relevent at a time
	int			latestSnapshotNum;	// the number of snapshots the client system has received
	int			latestSnapshotTime;	// the time from latestSnapshotNum, so we don't need to read the snapshot yet

	snapshot_t	*snap;				// cg.snap->serverTime <= cg.time
	snapshot_t	*nextSnap;			// cg.nextSnap->serverTime > cg.time, or NULL
	snapshot_t	activeSnapshots[2];

	float		frameInterpolation;	// (float)( cg.time - cg.frame->serverTime ) / (cg.nextFrame->serverTime - cg.frame->serverTime)

	bool	thisFrameTeleport;
	bool	nextFrameTeleport;

	int			frametime;		// cg.time - cg.oldTime

	// the gun positions of the player's own vehicle, swung here as the server swings them
	float		stationPitch[MAX_VEHICLE_TURRETS];
	float		stationYaw[MAX_VEHICLE_TURRETS];
	unsigned	stationsLagging;

	int			time;			// this is the time value that the client
								// is rendering at.
	int			oldTime;		// time at last frame, used for missile trails and prediction checking

	int			physicsTime;	// either cg.snap->time or cg.nextSnap->time

	int			timelimitWarnings;	// 5 min, 1 min, overtime
	int			fraglimitWarnings;

	bool	mapRestart;			// set on a map restart to set back the weapon

	bool	renderingThirdPerson;		// during deaths, chasecams, etc

	// prediction state
	bool	hyperspace;				// true if prediction has hit a trigger_teleport
	playerState_t	predictedPlayerState;
	ClientEntity		predictedPlayerEntity;
	bool	validPPS;				// clear until the first call to CGame::predictPlayerState
	int			predictedErrorTime;
	Vec3		predictedError;

	int			eventSequence;
	int			predictableEvents[MAX_PREDICTED_EVENTS];

	// input state sent to server
	int			weaponSelect;

	// auto rotating items
	Vec3		autoAngles;
	vec3_t		autoAxis[3];
	Vec3		autoAnglesFast;
	vec3_t		autoAxisFast[3];

	// view rendering
	refdef_t	refdef;
	Vec3		refdefViewAngles;		// will be converted to refdef.viewaxis

	// zoom key
	bool	zoomed;					// are we applying any zoom?
	bool	zoomLockOut;			// lock zoom until zoom button releases
	int			zoomAmount;				// x2, x4, x8 ...
	int			zoomTime;				// (unused)
	float		zoomSensitivity;		// (unused)

	// camera adjusting
	int			cameraAdjustEnum;		// camera adjustment identifier enum
	float		cameraAdjustAmount;		// camera adjustment amount
	int			cameraAdjustCount;

	// information screen text during loading
	char		infoScreenText[MAX_STRING_CHARS];
	float		infoLoadFraction;
	float		infoLoadStart;
	float		infoLoadEnd;
	int			infoLoadRefreshTime;

	// scoreboard
	int			scoresRequestTime;
	int			numScores;
	int			selectedScore;
	int			teamScores[2];
	score_t		scores[MAX_CLIENTS];
	bool	showScores;
	bool	scoreBoardShowing;
	int			scoreFadeTime;
	char		killerName[MAX_NAME_LENGTH];
	char			spectatorList[MAX_STRING_CHARS];		// list of names
	int				spectatorLen;												// length of list
	float			spectatorWidth;											// width in device units
	int				spectatorTime;											// next time to offset
	char		scoreboardMisc[MAX_STRING_CHARS];	// scoreboard extra info

	// centerprinting
	int			centerPrintTime;
	int			centerPrintCharWidth;
	int			centerPrintY;
	char		centerPrint[1024];
	int			centerPrintLines;

	// kill timers for carnage reward
	int			lastKillTime;

	// crosshair client ID
	int			crosshairClientNum;
	int			crosshairClientTime;

	// attacking player
	int			attackerTime;

	// reward medals
	int			rewardStack;
	int			rewardTime;
	int			rewardCount[MAX_REWARDSTACK];
	qhandle_t	rewardShader[MAX_REWARDSTACK];
	qhandle_t	rewardSound[MAX_REWARDSTACK];

	// sound buffer mainly for announcer sounds
	int			soundBufferIn;
	int			soundBufferOut;
	int			soundTime;
	qhandle_t	soundBuffer[MAX_SOUNDBUFFER];

	// warmup countdown
	int			warmup;
	int			warmupCount;

	//==========================

	int			itemPickup;
	int			itemPickupTime;
	int			itemPickupBlendTime;	// the pulse around the crosshair is timed seperately

	// blend blobs
	float		damageTime;
	float		damageX, damageY, damageValue;

	// view movement
	float		v_dmg_time;
	float		v_dmg_pitch;
	float		v_dmg_roll;


	// development tool
	refEntity_t		testModelEntity;
	char			testModelName[MAX_QPATH];
	bool		testGun;
	refEntity_t		testVehicleParts[BP_MAX_PARTS];
	char			testVehicleName[MAX_QPATH];
	long			testVehicleCat;
	refEntity_t		testBurner;
	refEntity_t		testBurner2;
	int				testBurnerNum;
	int				testNumBurners;
	refEntity_t		testVapor;
	int				testVaporNum;
	int				testVaporFrame;

	// MFQ3
	int				INFO;
	int				INFOTime;
	int				RADARTime;
	int				RADARRangeSetting;
	int				GPS;
	int				GPSTime;
	int				MFDTime;
	int				Mode_MFD[NUM_MFDS_IN_COCKPIT];
	int				toggleViewTime;
	ClientEntity		*radarEnts[MAX_RADAR_TARGETS];
	unsigned int	radarTargets;
	// team mates the radar showed this frame, for their name labels
	int				radarFriends[MAX_RADAR_TARGETS];
	int				numRadarFriends;
	int				tracernum;
	int				HUDColor;
	int				HUDColorTime;
	int				MFDColor;
	int				MFDColorTime;
	refdef_t		HUDCamera;
	int				CameraMode;
	int				CameraModeTime;
	bool		drawingMFD;
	reticle_t		HUDReticle[ MAX_RETICLES ];
	int				reticleIdx;
};


enum HUDColors_t {
	HUD_GREEN,
	HUD_DARK_GREEN,
	HUD_WHITE,
	HUD_GREY,
	HUD_RED,
	HUD_DARK_RED,
	HUD_BLUE,
	HUD_DARK_BLUE,
	HUD_YELLOW,
	HUD_MAGENTA,
	HUD_CYAN,
	HUD_ORANGE,
	HUD_MAX
};

enum HUDReticles_t {
	HR_GUIDED_WHITE,
	HR_TRACKING_ENEMY,
	HR_LOCKED_ENEMY,
	HR_GUIDED,
	HR_UNGUIDED_AND_GUN,
	HR_BOMB,
	HR_TARGET_ENEMY,
	HR_MAX
};

// colors
extern vec4_t		HUDColors[HUD_MAX];

// all of the model, shader, and sound references that are
// loaded at gamestate time are stored in cgMedia_t
// Other media that can be tied to clients, weapons, or items are
// stored in the clientInfo_t, itemInfo_t, weaponInfo_t, and powerupInfo_t
struct cgMedia_t {
	qhandle_t	charsetShader;
	qhandle_t	charsetProp;
	qhandle_t	charsetPropGlow;
	qhandle_t	charsetPropB;
	qhandle_t	whiteShader;

	qhandle_t	redFlagModel;
	qhandle_t	blueFlagModel;
	qhandle_t	redFlagShader[3];
	qhandle_t	blueFlagShader[3];

	qhandle_t	teamStatusBar;



	qhandle_t	particleShader;


	qhandle_t	selectShader;
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	lagometerShader;
	qhandle_t	backTileShader;

	qhandle_t	smokePuffShader;
	qhandle_t	smokePuffRageProShader;

	qhandle_t	numberShaders[11];

	qhandle_t	shadowMarkShader[MF_MAX_CATEGORIES];

	// wall mark shaders
	qhandle_t	bulletMarkShader;
	qhandle_t	burnMarkShader;
	qhandle_t	charredShader;

	// MFQ3 effect models
	qhandle_t	vapor;
	qhandle_t	vaporBig;
	qhandle_t	vaporShader;
	qhandle_t	afterburner[AB_MAX];
	qhandle_t	bulletTracer;
	qhandle_t	pilot;
	qhandle_t	pilotHead;
	qhandle_t	pilotSeat;
	qhandle_t	pilotWW2;
	qhandle_t	pilotHeadWW2;
	qhandle_t	pilotSeatWW2;
	qhandle_t	parachute;
	qhandle_t	missilePuffShader;
	qhandle_t	flarePuffShader;
	qhandle_t   railCoreShader;
	qhandle_t	missileTrail2Shader;
	qhandle_t	nukePuffShader;
	qhandle_t	nukeCloud[NUKE_CLOUD_FRAMES];

	qhandle_t	reticle[CH_NUMCROSSHAIRS];

	// MFQ3 effect shaders
	qhandle_t	vehicleMuzzleFlashModel;

	// MFQ3 HUD
	qhandle_t	radarIcons[RD_MAX_ICONS];

	// MFQ3 NEW HUD
	qhandle_t	HUDheading;
	qhandle_t	HUDvaluebox;
	qhandle_t	HUDvaluebox2;
	qhandle_t	HUDind_h;
	qhandle_t	HUDind_v;
	qhandle_t	HUDind_v_r;
	qhandle_t	HUDcaret_h;
	qhandle_t	HUDcaret_v_g_l;
	qhandle_t	HUDcaret_v_r_l;
	qhandle_t	HUDmfd;
	qhandle_t	HUDspeed;
	qhandle_t	HUDrwr;
	qhandle_t	HUDalt;
	qhandle_t	HUDsolid;
	qhandle_t	HUDhealth[10];
	qhandle_t	HUDthrottle_1[10];
	qhandle_t	HUDthrottle_2_1[10];
	qhandle_t	HUDthrottle_2_2[10];
	qhandle_t	HUDthrottle_1_ab[15];
	qhandle_t	HUDthrottle_2_1_ab[15];
	qhandle_t	HUDthrottle_2_2_ab[15];
	qhandle_t	HUDthrottle_rev[5];
	qhandle_t	HUDhealthtext;
	qhandle_t	HUDthrottletext;
	qhandle_t	HUDalphabet;
	qhandle_t	HUDnumberline;
	qhandle_t	HUDreticles[HR_MAX];
	
	// IGME
	qhandle_t	IGME_selector;
	qhandle_t	IGME_waypoint;
	qhandle_t	IGME_waypoint2;

	// weapon effect models
	qhandle_t	bulletFlashModel;
	qhandle_t	dishFlashModel;
	qhandle_t	casingModel;

	// weapon effect shaders
	qhandle_t	bulletExplosionShader;
	qhandle_t	rocketExplosionShader[2];
	qhandle_t	grenadeExplosionShader;

	// Nuke
	qhandle_t	nukeEffectModel;
	qhandle_t	nukeShockWave;
	qhandle_t	nukeCloudModel;
	sfxHandle_t nukeExplodeSound;
	sfxHandle_t nukeImplodeSound;
	sfxHandle_t nukeFarSound;

	// gib explosions
	qhandle_t	gibAbdomen;
	qhandle_t	gibArm;
	qhandle_t	gibChest;
	qhandle_t	gibFist;
	qhandle_t	gibFoot;
	qhandle_t	gibForearm;
	qhandle_t	gibIntestine;
	qhandle_t	gibLeg;
	qhandle_t	gibSkull;
	qhandle_t	gibBrain;

	// sounds
	sfxHandle_t	sfx_ric1;
	sfxHandle_t	sfx_ric2;
	sfxHandle_t	sfx_ric3;
	sfxHandle_t	sfx_rockexp;

	sfxHandle_t	respawnSound;
	sfxHandle_t talkSound;

	sfxHandle_t oneMinuteSound;
	sfxHandle_t fiveMinuteSound;
	sfxHandle_t suddenDeathSound;

	sfxHandle_t threeFragSound;
	sfxHandle_t twoFragSound;
	sfxHandle_t oneFragSound;

	sfxHandle_t hitSound;
	sfxHandle_t hitTeamSound;

	sfxHandle_t takenLeadSound;
	sfxHandle_t tiedLeadSound;
	sfxHandle_t lostLeadSound;

	sfxHandle_t watrInSound;
	sfxHandle_t watrOutSound;
	sfxHandle_t watrUnSound;

	// teamplay sounds
	sfxHandle_t redScoredSound;
	sfxHandle_t blueScoredSound;
	sfxHandle_t redLeadsSound;
	sfxHandle_t blueLeadsSound;
	sfxHandle_t teamsTiedSound;

	sfxHandle_t	captureYourTeamSound;
	sfxHandle_t	captureOpponentSound;
	sfxHandle_t	returnYourTeamSound;
	sfxHandle_t	returnOpponentSound;

	sfxHandle_t redFlagReturnedSound;
	sfxHandle_t blueFlagReturnedSound;
	sfxHandle_t	enemyTookYourFlagSound;
	sfxHandle_t yourTeamTookEnemyFlagSound;
	sfxHandle_t	youHaveFlagSound;

	// tournament sounds
	sfxHandle_t	count3Sound;
	sfxHandle_t	count2Sound;
	sfxHandle_t	count1Sound;
	sfxHandle_t	countFightSound;
	sfxHandle_t	countPrepareSound;

	qhandle_t cursor;

	sfxHandle_t	n_healthSound;

	// MFQ3
	sfxHandle_t engineJet;
	sfxHandle_t	engineJetAB;
	sfxHandle_t engineProp;
	sfxHandle_t planeDeath[NUM_EXPLOSION_SOUNDS];
	sfxHandle_t engineTank[NUM_TANKSOUNDS];
	sfxHandle_t	engineHelo;

};


enum CameraAdjust {
	CAMADJ_NONE,
	CAMADJ_INOUT,
	CAMADJ_UPDOWN,
	MAX_CAMERA_ADJUSTS
};

// IGME

enum class IGME_OBJECT_TYPES {
	IGMEO_VEHICLE,
	IGMEO_WAYPOINT
};

using enum IGME_OBJECT_TYPES;

struct IGME_waypoint_t {
	bool		active;
	bool		selected;
	Vec3			origin;
};

struct IGME_vehicle_t {
	bool		active;
	bool		selected;
	bool		groundInstallation;
	int				vehidx;
	char			side[8];
	char			tag[MAX_NAME_LENGTH];
	Vec3			angles;
	Vec3			origin;
	Vec3			selectorScale;
	IGME_waypoint_t waypoints[IGME_MAX_WAYPOINTS];
};

struct IGME_data_t {
	IGME_vehicle_t	vehicles[IGME_MAX_VEHICLES];
	int				selectionTime;
	int				numSelections;
	int				numWptSelections;
	bool		dragmode;
	bool		waypointmode;
	bool		showAllWaypoints;
	IGME_vehicle_t	copyBufferVehicle;
	bool		copyBufferVehicleUsed;
	IGME_waypoint_t	copyBufferRoute[IGME_MAX_WAYPOINTS];
	bool		copyBufferRouteUsed;
};

// end IGME

// MFQ3 vehicle drawing
struct BasicDrawInfo_t {
    int					vehicleIndex;
	float				speed;
	vec3_t				axis[3];
	Vec3				origin;
	Vec3				angles;
	int					ONOFF;
	completeLoadout_t*	usedLoadout;
	int					numWeaponsOnMount[32];
	int					throttle;
	int					entityNum;
	int					flashWeaponIndex;
	int					team;
	bool			drawMuzzleFlash;
	int				flashStations;
};

struct DrawInfo_Plane_t {
    BasicDrawInfo_t		basicInfo;
	int					controlFrame;
	int					gearFrame;
	int					bayFrame;
	int					brakeFrame;
	int					cockpitFrame;
	int					bodyFrame;
	float				swingAngle;
	bool				stationsKnown;	// otherwise the gun positions rest where the hull points
	float				stationPitch[MAX_VEHICLE_TURRETS];	// where each points, in the hull's space
	float				stationYaw[MAX_VEHICLE_TURRETS];
};

struct DrawInfo_GV_t {
    BasicDrawInfo_t		basicInfo;
	float				turretAngle;
	float				gunAngle;
	float				wheelAngle;
	float				wheelDirection;
	bool				turretGone = false;
};

struct DrawInfo_Boat_t {
    BasicDrawInfo_t		basicInfo;
	float				turretAngle[4];
	float				gunAngle[4];
};

struct DrawInfo_Helo_t {
    BasicDrawInfo_t		basicInfo;
	float				turretAngle[4];
	float				gunAngle[4];
	float				doorGunAim = 0;	// where the door guns are being aimed, before each gun's own stops
	int					gearFrame;
	int					bayFrame;
};

struct DrawInfo_LQM_t {
    BasicDrawInfo_t		basicInfo;
	int					anim;
	int					torsoFrame;
	int					legsFrame;
	int					torsoTime;
	int					legsTime;
	int					lastLegsAngle;
	int					lastTorsoAngle;
	int					weaponIndex;
};

struct DrawInfo_GI_t {
	BasicDrawInfo_t		basicInfo;
	int					upgrades;
	float				turretAngle;
	float				gunAngle;
};

// end MFQ3 vehicle drawing

// The client game static (cgs) structure hold everything
// loaded or calculated from the gamestate.  It will NOT
// be cleared when a tournement restart is done, allowing
// all clients to begin playing instantly
struct cgs_t {
	gameState_t		gameState;			// gamestate from server
	glconfig_t		glconfig;			// rendering configuration
	float			screenXScale;		// derived from glconfig
	float			screenYScale;
	float			screenScale;
	float			screenXOffset;
	float			screenYOffset;

	int				serverCommandSequence;	// reliable command stream counter
	int				processedSnapshotNum;// the number of snapshots cgame has requested

	int				localServer;		// detected on startup by checking sv_running

	// parsed from serverinfo
	gametype_t		gametype;
	int				dmflags;
	int				teamflags;
	int				fraglimit;
	int				capturelimit;
	int				timelimit;
	int				maxclients;
	char			mapname[MAX_QPATH];
	char			redTeam[MAX_QPATH];
	char			blueTeam[MAX_QPATH];

	int				voteTime;
	int				voteYes;
	int				voteNo;
	bool		voteModified;			// beep whenever changed
	char			voteString[MAX_STRING_TOKENS];

	int				teamVoteTime[2];
	int				teamVoteYes[2];
	int				teamVoteNo[2];
	bool		teamVoteModified[2];	// beep whenever changed
	char			teamVoteString[2][MAX_STRING_TOKENS];

	int				levelStartTime;

	int				scores1, scores2;		// from configstrings
	int				redflag, blueflag;		// flag status from configstrings
	int				flagStatus;


	// mfq3
	unsigned long	gameset;

	//
	// locally derived information from gamestate
	//
	qhandle_t		gameModels[MAX_MODELS];
	sfxHandle_t		gameSounds[MAX_SOUNDS];

	int				numInlineModels;
	qhandle_t		inlineDrawModel[MAX_MODELS];
	vec3_t			inlineModelMidpoints[MAX_MODELS];

	clientInfo_t	clientinfo[MAX_CLIENTS];

	// teamchat width is *3 because of embedded color codes
	char			teamChatMsgs[TEAMCHAT_HEIGHT][TEAMCHAT_WIDTH*3+1];
	int				teamChatMsgTimes[TEAMCHAT_HEIGHT];
	int				teamChatPos;
	int				teamLastChatPos;

	bool eventHandling;

	// orders

	// media
	cgMedia_t		media;

	// IGME
	IGME_data_t		IGME;

};

//==============================================================================

//extern	vmCvar_t		cg_pmove_fixed;
//extern	vmCvar_t		cg_drawCrosshair;

extern	cgs_t			cgs;
extern	cg_t			cg;
extern	ClientEntity		cg_entities[MAX_GENTITIES];
extern	weaponInfo_t	cg_weapons[WI_MAX];
extern	itemInfo_t		cg_items[MAX_ITEMS];

// MFQ3
extern	completeLoadout_t cg_loadouts[MAX_GENTITIES];
extern	int cg_loadoutKeys[MAX_GENTITIES];
// -MFQ3

extern	vmCvar_t		cg_centertime;
extern	vmCvar_t		cg_runpitch;
extern	vmCvar_t		cg_runroll;
extern	vmCvar_t		cg_shadows;
extern	vmCvar_t		cg_drawTimer;
extern	vmCvar_t		cg_drawFPS;
extern	vmCvar_t		cg_drawParticleStats;
extern	vmCvar_t		cg_drawSnapshot;
extern	vmCvar_t		cg_draw3dIcons;
extern	vmCvar_t		cg_drawIcons;
extern	vmCvar_t		cg_drawCrosshairNames;
extern	vmCvar_t		cg_drawRewards;
extern	vmCvar_t		cg_drawTeamOverlay;
extern	vmCvar_t		cg_crosshairX;
extern	vmCvar_t		cg_crosshairY;
extern	vmCvar_t		cg_crosshairSize;
extern	vmCvar_t		cg_drawStatus;
extern	vmCvar_t		cg_drawFriend;
extern	vmCvar_t		cg_draw2D;
extern	vmCvar_t		cg_debugEvents;
extern	vmCvar_t		cg_errorDecay;
extern	vmCvar_t		cg_nopredict;
extern	vmCvar_t		cg_showmiss;
extern	vmCvar_t		cg_addMarks;
extern	vmCvar_t		cg_gun_x;
extern	vmCvar_t		cg_gun_y;
extern	vmCvar_t		cg_gun_z;
extern	vmCvar_t		cg_viewsize;
extern	vmCvar_t		cg_simpleItems;
extern	vmCvar_t		cg_fov;
extern	vmCvar_t		cg_zoomFov;
extern	vmCvar_t		cg_thirdPersonRange;
extern	vmCvar_t		cg_thirdPersonAngle;
extern	vmCvar_t		cg_thirdPerson;
extern	vmCvar_t		cg_stereoSeparation;
extern	vmCvar_t		cg_lagometer;
extern	vmCvar_t		cg_synchronousClients;
extern	vmCvar_t		cg_teamChatTime;
extern	vmCvar_t		cg_teamChatHeight;
extern	vmCvar_t		cg_stats;
extern	vmCvar_t		cg_paused;
extern	vmCvar_t		cg_advanced;
extern	vmCvar_t		cg_predictItems;
extern	vmCvar_t		cg_teamChatsOnly;
extern	vmCvar_t		cg_smoothClients;
extern	vmCvar_t		pmove_fixed;
extern	vmCvar_t		pmove_msec;
extern	vmCvar_t		cg_timescaleFadeEnd;
extern	vmCvar_t		cg_timescaleFadeSpeed;
extern	vmCvar_t		cg_timescale;
extern  vmCvar_t		cg_smallFont;
extern  vmCvar_t		cg_bigFont;

// MFQ3+
extern	vmCvar_t		cg_vehicle;
extern	vmCvar_t		cg_nextVehicle;
extern	vmCvar_t		cg_thirdPersonHeight;
extern	vmCvar_t		cg_radarTargets;
extern  vmCvar_t		cg_smoke;


extern	vmCvar_t		hud_heading;
extern	vmCvar_t		hud_speed;
extern	vmCvar_t		hud_altitude;
extern	vmCvar_t		hud_mfd;
extern	vmCvar_t		hud_mfd2;
extern	vmCvar_t		hud_center;
extern  vmCvar_t		hud_health;
extern	vmCvar_t		hud_throttle;
extern	vmCvar_t		hud_weapons;

extern	vmCvar_t		hud_color;
extern	vmCvar_t		mfd_color;
extern	vmCvar_t		mfd1_defaultpage;
extern	vmCvar_t		mfd2_defaultpage;

extern	vmCvar_t		m1cp_page0;
extern	vmCvar_t		m1cp_page1;
extern	vmCvar_t		m1cp_page2;
extern	vmCvar_t		m1cp_page3;
extern	vmCvar_t		m1cp_page4;
extern	vmCvar_t		m1cp_page5;
extern	vmCvar_t		m2cp_page0;
extern	vmCvar_t		m2cp_page1;
extern	vmCvar_t		m2cp_page2;
extern	vmCvar_t		m2cp_page3;
extern	vmCvar_t		m2cp_page4;
extern	vmCvar_t		m2cp_page5;

extern	vmCvar_t		cg_fxQuality;
extern	vmCvar_t		cg_debrisTime;
extern	vmCvar_t		cg_casingTime;
extern	vmCvar_t		cg_consoleTextStyle;

extern	vmCvar_t		cg_shadowDebug;

// MFQ3-

//
// cg_main.c
//
void CG_Init( int serverMessageNum, int serverCommandSequence, int clientNum );
void CG_Shutdown();

//void QDECL CG_Error( const char *msg, ... );



void CG_RankRunFrame( );
score_t *CG_GetSelectedScore();

void CG_AddRegisterMessage(const char * s);

//
// cg_util.c
//
unsigned long MF_CG_GetGameset( bool asEnum );

//
// cg_testmodel.c
//


//
// cg_view.c
//


//
// cg_drawtools.c
//
enum class screenPlacement_e {
	PLACE_CENTER,
	PLACE_LEFT,
	PLACE_RIGHT,
	PLACE_STRETCH
};
using enum screenPlacement_e;

void CG_DrawString( float x, float y, const char *string, 
				   float charWidth, float charHeight, const float *modulate );







//void CG_Draw_HUD_Label( int x, int y, char * pText, float alpha );

//
// cg_draw.c, cg_newDraw.c
//
extern	int sortedTeamPlayers[TEAM_MAXOVERLAY];
extern	int	numSortedTeamPlayers;
extern	int drawTeamOverlayModificationCount;

void CG_SelectPrevPlayer();
void CG_SelectNextPlayer();
float CG_GetValue(int ownerDraw);
bool CG_OwnerDrawVisible(int flags);
void CG_RunMenuScript(char **args);
void CG_ShowResponseHead();
void CG_SetPrintString(int type, const char *p);
void CG_InitTeamChat();
void CG_GetTeamColor(vec4_t *color);
const char *CG_GetKillerText();
void CG_CheckOrderPending();
bool CG_YourTeamHasFlag();
bool CG_OtherTeamHasFlag();
qhandle_t CG_StatusHandle(int task);



//
// cg_player.c
//
void CG_TrailItem( ClientEntity *cent, qhandle_t hModel );	// ditto

//
// cg_vehicle.c
//


//
// cg_vehicledraw.c
//

//
// cg_plane.c
//

//
// cg_groundvehicle.c
//


//
// cg_helo.c
//


//
// cg_lqm.c
//


//
// cg_boat.c
//


//
// cg_predict.c
//


//
// cg_events.c
//


//
// cg_ents.c
//




//
// cg_weapons.c
//


void CG_Bullet( vec3_t origin, int sourceEntityNum, vec3_t normal, int fleshEntityNum );

//
// cg_marks.c
//



//
// cg_localents.c
//

//
// cg_effects.c
//



//
// cg_snapshot.c
//

//
// cg_info.c
//

//
// cg_scoreboard.c
//

//
// cg_consolecmds.c
//

//
// cg_servercmds.c
//

//
// cg_playerstate.c
//

//
// cg_missioneditor.c
//
IGME_vehicle_t* ME_SpawnVehicle( int idx );
void ME_SpawnGroundInstallation( int idx );
void ME_ExportToScript( const char* scriptname );
void ME_ImportScript( const char* scriptname );
void ME_SetSide( const char* side );
void ME_KeyEvent(int key, bool down);
void ME_MouseEvent(int x, int y);
void ME_Init_MissionEditor();

//===============================================

// system calls
int Key_GetCatcher();
void Key_SetCatcher( int catcher );




// aids for VM testing
void		testPrintInt( char *string, int i );
void		testPrintFloat( char *string, float f );


enum {
  SYSTEM_PRINT,
  CHAT_PRINT,
  TEAMCHAT_PRINT
};


// cg_particles.c



inline constexpr int MAX_WORLD_PARTICLES = 16384;
inline constexpr int MAX_WORLD_EMITTERS = 500;
inline constexpr int MAX_WORLD_SYSTEMS = 200;
inline constexpr int MAX_PARTICLE_SHADERS = 100;




// Particle systems are nothing more than a collection of emitters
struct particleSystem_s
{
	ClientEntity	* cent;

	Vec3		pos;		// position of system  if cent != null
	// age properties
	bool		dead;
	long		birthtime;
	long		lifetime;
	long		lastthink;

};
using particleSystem_t = particleSystem_s;


struct particle_s
{
	int	parentEmitter;
	
	// age properties
	bool		dead;
	long		birthtime;
	long		lifetime;
	long		lastthink;
	long		lastAttractionThink;
	
	// size
	float	radius;				// radius of particle
	long	growstart;			// when to start growing
	long 	growend;			// when to end growing
	float	growrate;			// how fast to grow
	
	// phsyics
	Vec3		trDelta;
	Vec3		trBase;
	Vec3		trAttraction;
	float		maxspeed;
	float		accel;				// acceleration rate
	float		acceltime;			// how long to accelerate for
	float		rotation;			// current rotation
	float  		rotationvel;		// rotation velocity
	float		attractionSystem;	// attraction to particles of the same system in a world
	float 		attractionEmitter;	// attraction to particles of the same emitter in a world
	float		attractionWorld;	// attraction to all particles in the same world 
	float		attractionEntity;	// Attraction to entities;
	float		bounce;				// Bounce  1.0 = perfect bounce
	int			pm_type;			//
	bool		dieOnCollision;		//
	float		gravity;			// gravity value;
		
	// visual properties
	bool				reallight;		// whether or not to be affected by world lights
	qhandle_t			shaders[100];	// visual shader to use
	unsigned int 		shaderFPS;		// shader animation time; 0 = by age, > 0 by fps
	unsigned int		numShaders;		// number of shaders
	float				alpha;			// alpha transparacy of particle
	long 				alphaDecayStartTime;
	long				alphaDecayEndTime;
	float				alphaDecayStartValue;
	float				alphaDecayEndValue;
};
using particle_t = particle_s;

// Particle emitters spawn New particles in a certain way
struct particleEmitter_s
{
	bool					dead;

	long					birthtime;
	long					lifetime;
	long					lastthink;

	int						parentSytem;
	int						inheritVelocity;   // -1 = eject with opposite velocity, 0 = stationary, 1 = eject with same velocity

	particle_t				particleTemplate;
	
	Vec3					ejectVelocity;

	Vec3					accelVarianceMax;
	Vec3					accelVarianceMin;
	float					rotationVariance;
	float					velVariance;
	float					maxspeedVariance;
	float					gravityVariance;
	float					bounceVariance;
	float					radiusVariance;
	float					lifetimeVariance;

	long					numEjectPerInterval;
	long					ejectInterval;
	long					lastEjectTime;
};
using particleEmitter_t = particleEmitter_s;


// Particle worlds are nothing more than a collection of systems
struct particleWorld_s					                         
{
	particle_t			particles[MAX_WORLD_PARTICLES];
	particleSystem_t	systems[MAX_WORLD_SYSTEMS];
	particleEmitter_t	emitters[MAX_WORLD_EMITTERS];
	int					numParticles;
};
using particleWorld_t = particleWorld_s;

class CGame
{
public:
	static const char *configString( int index );
	static const char *argv( int arg );
	static void QDECL printf( const char *msg, ... ) Q_PRINTF_FUNC(1, 2);
	static void startMusic( );
	static void updateCvars( );
	static int crosshairPlayer( );
	static int lastAttacker( );
	static void loadMenus(const char *menuFile);
	static void keyEvent(int key, bool down);
	static void mouseEvent(int x, int y);
	static void eventHandling(int type);
	static void setScoreSelection(void *menu);
	static void buildSpectatorString();
	static int feederCount(float feederID);
	static float cvar_Get( const char * cvar );
	static void testPlaneCmd_f();
	static void testGVCmd_f();
	static void testVehicle(long cat);
	static void addTestVehicle ();
	static void addTestModel ();
	static void testModel_f ();
	static void testGun_f ();
	static void testModelNextFrame_f ();
	static void testModelPrevFrame_f ();
	static void testModelNextSkin_f ();
	static void testModelPrevSkin_f ();
	static void zoomDown_f( );
	static void zoomUp_f( );
	static void addBufferedSound( sfxHandle_t sfx);
	static void drawActiveFrame( int serverTime, stereoFrame_t stereoView, bool demoPlayback );
	struct ScreenPoint { int x, y; };
	static std::optional<ScreenPoint> worldToScreenCoords( vec3_t worldPoint, bool virtualXY );
	static void drawProgressBar( rectDef_t *rect, vec4_t color, vec4_t textcolor, float scale, int textStyle, float progress );
	static void setScreenPlacement( screenPlacement_e placement );
	static screenPlacement_e getScreenPlacement( );
	static void adjustFrom640( float *x, float *y, float *w, float *h );
	static void virtualFromScreen( float *x, float *y );
	static void fillRect( float x, float y, float width, float height, const float *color );
	static void drawPic( float x, float y, float width, float height, qhandle_t hShader );
	static void drawStringExt( int x, int y, const char *string, const float *setColor, bool forceColor, bool shadow, int charWidth, int charHeight, int maxChars );
	static void drawBigString( int x, int y, const char *s, float alpha );
	static void drawBigStringColor( int x, int y, const char *s, vec4_t color );
	static void drawSmallString( int x, int y, const char *s, float alpha );
	static void drawSmallStringColor( int x, int y, const char *s, vec4_t color );
	static int drawStrlen( const char *str );
	static float *fadeColor( int startMsec, int totalMsec );
	static float *teamColor( int team );
	static void tileClear( );
	static void drawRect( float x, float y, float width, float height, float size, const float *color );
	static void drawSides(float x, float y, float w, float h, float size);
	static void drawTopBottom(float x, float y, float w, float h, float size);
	static bool genericShadow( BasicDrawInfo_t *drawInfo, float *shadowPlane );
	static void resetReticles( );
	static int reticleOf( int vehicle, int weaponNum );
	static void addReticleEntityToScene( refEntity_t * pReticle, ClientEntity * pTarget );
	static void draw_Reticles();
	static void addLagometerFrameInfo( );
	static void addLagometerSnapshotInfo( snapshot_t *snap );
	static void centerPrint( const char *str, int y, int charWidth );
	static void drawActive( stereoFrame_t stereoView );
	static void drawFlagModel( float x, float y, float w, float h, int team, bool force2D );
	static void drawTeamBackground( int x, int y, int w, int h, float alpha, int team );
	static void ownerDraw(float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, const vec4_t color, qhandle_t shader, int textStyle, itemDef_t * item );
	static void text_Paint(float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit, int style);
	static int text_Width(const char *text, float scale, int limit);
	static int text_Height(const char *text, float scale, int limit);
	static const char *getGameStatusText();
	static void draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles );
	static void text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader);
	static const char *gameTypeString();
	static void drawStatusBar_MFQ3_new();
	static void add_Console_Line( char * pText );
	static bool newHUDActive( );
	static void newClientInfo( int clientNum );
	static sfxHandle_t customSound( int clientNum, const char *soundName );
	static void cacheVehicles();
	static void registerVehicle( clientInfo_t *ci );
	static void vehicleObituary( entityState_t *ent );
	static void vehicleMuzzleFlash( int weaponIdx, const refEntity_t *parent, qhandle_t parentModel, int idx );
	static void initShadows( );
	static qhandle_t teamSkin( qhandle_t model, int team );
	static void drawPlane(DrawInfo_Plane_t* drawInfo);
	static void drawGV(DrawInfo_GV_t* drawInfo);
	static void drawBoat(DrawInfo_Boat_t* drawInfo);
	static void drawHelo(DrawInfo_Helo_t* drawInfo);
	static void drawLQM(DrawInfo_LQM_t* drawInfo);
	static void drawGI(DrawInfo_GI_t* drawInfo);
	static void registerPlane( clientInfo_t *ci );
	static void planeObituary( entityState_t *ent, clientInfo_t *ci );
	static void registerGroundVehicle( clientInfo_t *ci );
	static void groundVehicleObituary( entityState_t *ent, clientInfo_t *ci );
	static void registerHelo( clientInfo_t *ci );
	static void heloObituary( entityState_t *ent, clientInfo_t *ci );
	static void registerLQM( clientInfo_t *ci );
	static void lQMObituary( entityState_t *ent, clientInfo_t *ci );
	static void registerBoat( clientInfo_t *ci );
	static void boatObituary( entityState_t *ent, clientInfo_t *ci );
	static void buildSolidList( );
	static int pointContents( const vec3_t point, int passEntityNum );
	static void trace( trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask, bool capsule = true );
	static void predictPlayerState( );
	static const char *placeString( int rank );
	static void addPacketEntities( );
	static void adjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out );
	static void positionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName );
	static void positionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, qhandle_t parentModel, char *tagName );
	static void nextWeapon_f( );
	static void prevWeapon_f( );
	static void weapon_f( );
	static void registerWeapons();
	static void registerItemVisuals( int itemNum );
	static void missileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType );
	static int apparentTeam( int clientNum, int viewerTeam );
	static void genericExplosion( vec3_t origin, int type );
	static void cookOff( vec3_t origin, int vehicle );
	static void vehicleHit( vec3_t origin, int damage );
	static void initMarkPolys( );
	static void addMarks( );
	static void impactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir, float orientation, float r, float g, float b, float a, bool alphaFade, float radius, bool temporary );
	static void impactMarkEx( qhandle_t markShader, const vec3_t origin, const vec3_t dir, float orientation, float red, float green, float blue, float alpha, bool alphaFade, float xRadius, float yRadius, bool temporary );
	static void initLocalEntities( );
	static LocalEntity *allocLocalEntity( );
	static void addLocalEntities( );
	static LocalEntity *smokePuff( const vec3_t p, const vec3_t vel, float radius, float r, float g, float b, float a, float duration, int startTime, int fadeInTime, leFlag_t leFlags, qhandle_t hShader );
	static LocalEntity *makeExplosion( vec3_t origin, vec3_t dir, qhandle_t hModel, qhandle_t shader, int offset, int duration, bool isSprite );
	static void gibPlayer( vec3_t playerOrigin );
	static void processSnapshots( );
	static void loadingString( const char *s );
	static void loadingStage( const char *s, float start, float end );
	static void loadingStep( int done, int total );
	static void drawInformation( );
	static void drawOldTourneyScoreboard( );
	static bool consoleCommand( );
	static void initConsoleCommands( );
	static void executeNewServerCommands( int latestSequence );
	static void parseServerinfo( );
	static void setConfigValues( );
	static void shaderStateChanged();
	static void respawn( );
	static void transitionPlayerState( playerState_t *ps, playerState_t *ops );
	static void checkChangedPredictableEvents( playerState_t *ps );
	static void draw_IGME();
	static void particleWorldRender(particleWorld_t * w);
	static void particleWorldThink(particleWorld_t * w);
	static int addParticleEmitter(particleWorld_t * w, particleEmitter_t * e);
	static int addParticleSystem(particleWorld_t * w, particleSystem_t * s);
	static void particleWorldInit(particleWorld_t * w);
	static void particleThink(particleWorld_t * w, particle_t * p);

public:
	static void toggleGps_f();
	static void toggleRadarRange_f();
	static void runHeldKeys();
	static void set_MFD_Mode( int num );
	static void set_MFD1_Mode_f();
	static void set_MFD2_Mode_f();
	static void set_MFD_Page( int num );
	static void set_MFD1_Page_f();
	static void set_MFD2_Page_f();
	static void update_HUD_Color_f();
	static void cycle_HUD_Color_f();
	static void update_MFD_Color_f();
	static void toggle_MFD1_f();
	static void toggle_MFD2_f();
	static void cycle_MFD_Color_f();
	static void cycle_Camera_Mode_f();
	static void set_Camera_Mode_f();
	static void camBack_f();
	static void camDown_f();
	static void camBomb_f();
	static void camOld_f();
	static void toggleInfo_f();
	static void toggleFriendLabels_f();
	static void toggleView_f();
	static void adjustReset_f();
	static void zoomIn_f();
	static void zoomOut_f();
	static void cameraUp_f();
	static void cameraDown_f();
	static void sizeUp_f();
	static void sizeDown_f();
	static void viewpos_f();
	static void scoresDown_f();
	static void scoresUp_f();
	static void tellTarget_f();
	static void tellAttacker_f();
	static void testPlane_f();
	static void testGV_f();
	static void spawn_GI_f();
	static void exportToScript_f();
	static void setSide_f();
	static void importScript_f();
	static float drawSnapshot( float y );
	static float drawFPS( float y );
	static float drawParticleStats( float y );
	static void drawDisconnect();
	static void drawLagometer( float y );
	static void drawCustomConsole();
	static void drawUpperLeft();
	static void drawUpperRight( bool scoreboard );
	static void drawCenterString();
	static void drawCrosshairNames();
	static void drawSpectator();
	static void drawVote();
	static void drawTeamVote();
	static bool drawScoreboard();
	static void drawIntermission();
	static bool drawFollow();
	static void drawWarmup();
	static void drawTourneyScoreboard();
	static void draw2D_MFQ3();
	static void drawGameType( rectDef_t *rect, float scale, const vec4_t color, qhandle_t shader, int textStyle );
	static void drawGameMisc( itemDef_t * item );
	static void drawTeamScore( itemDef_t * item, int team );
	static void drawTeamCount( itemDef_t * item, int team );
	static void drawGameStatus( rectDef_t *rect, float scale, const vec4_t color, qhandle_t shader, int textStyle );
	static void drawHUDPic( float x, float y, float width, float height, qhandle_t hShader, vec4_t color );
	static int getStartPos( int col );
	static int getStartPosNum( int col );
	static int drawChar_MFQ3( int x, int y, int ch, bool right );
	static int drawString_MFQ3( int x, int y, const char *string, const float *setColor, int maxChars );
	static int drawString_MFQ3_R( int x, int y, const char *string, const float *setColor, int maxChars );
	static void drawRadarSymbols_AIR_new( int vehicle, float range, int x, int y );
	static void drawRadarSymbols_GROUND_new( int vehicle, float range, int x, int y );
	static void draw_Coords( int xcoord, int ycoord );
	static void draw_Redundant( int vehicle, int health, int throttle, int ammo[16], int selweap );
	static void draw_Center( int vehicle, int health, int throttle );
	static void hUD_Camera( int mfdnum, int vehicle );
	static void draw_AltTape( int value );
	static void draw_SpeedTape( int value, int stallspeed, int gearspeed, int scale );
	static void draw_HeadingTape( int value, int targetheading );
	static void tileClearBox( int x, int y, int w, int h, qhandle_t hShader );
	static void uI_DrawBannerString2( int x, int y, const char* str, vec4_t color );
	static void uI_DrawProportionalString2( int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset );
	static bool polyMeshGeneratedShadow( BasicDrawInfo_t *drawInfo, float *shadowPlane );
	static void itemPickup( int itemNum );
	static void loadingRefresh( bool force );
	static void forceModelChange();
	static void registerItemSounds( int itemNum );
	static void registerSounds();
	static void precacheVehiclePreview();
	static void registerGraphics();
	static void registerClients();
	static clientInfo_t * infoFromScoreIndex( int index, int team, int *scoreIndex );
	static IGME_vehicle_t* mE_SpawnVehicleAt( int vehidx, vec3_t pos, vec3_t angles );
	static IGME_vehicle_t* mE_SpawnGroundInstallationAt( int idx, vec3_t pos, vec3_t angles );
	static void mE_SpawnMissionGroundInstallations( mission_groundInstallation_t* gis );
	static void mE_SpawnMissionVehicles( mission_vehicle_t* vehs );
	static bool mE_LoadOverviewAndEntities( char *filename, mission_overview_t* overview, mission_vehicle_t* vehs, mission_groundInstallation_t* gis, mission_rules_t* rules );
	static void colorFromString( const char *v, vec3_t color );
	static void clipMoveToEntities( const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask, trace_t *tr );
	static void interpolatePlayerState( bool grabAngles );
	static void touchTriggerPrediction();
	static void centerGiantLine( float y, const char *string );
	static void parseScores();
	static void parseTeamInfo();
	static void parseWarmup();
	static void configStringModified();
	static void addToTeamChat( const char *str );
	static void mapRestart();
	static void removeChatEscapeChar( char *text );
	static void printExtractExtra();
	static void serverCommand();
	static void transitionSnapshot();
	static void setNextSnap( snapshot_t *snap );
	static snapshot_t * readNextSnapshot();
	static void registerTestVehicle();
	static animation_t * parseLQMAnimationFile( const char *filename );
	static qhandle_t registerVehiclePart( const char *name, bool expected );
	static void cachePlane( int index );
	static void cacheGroundVehicle( int index );
	static void cacheHelo( int index );
	static void cacheLQM( int index );
	static void cacheBoat( int index );
	static void cacheGroundInstallation( int index );
	static void addVehiclePart( refEntity_t *re );
	static void planePilot( const refEntity_t *parent, qhandle_t parentModel, int idx );
	static void calcVrect();
	static void offsetVehicleView( bool spectator );
	static void offsetVehicleViewCFV( bool spectator );
	static void offsetCockpitView();
	static int calcFov();
	static void calcMiscViewValues();
	static int calcViewValues();
	static void playBufferedSounds();
	static void fFARTrail( ClientEntity * cent, const weaponInfo_t *wi );
	static void missileTrail( ClientEntity * cent, const weaponInfo_t *wi );
	static void flareTrail( ClientEntity * cent, const weaponInfo_t *wi );
	static void missileTrail2( ClientEntity * cent, const weaponInfo_t *wi );

private:
	static int number_spaces[16];
	static int alphabet_spaces[33];
	static screenPlacement_e cg_screenPlacement;
	static int markTotal;
	static ClientEntity *cg_triggerEntities[MAX_ENTITIES_IN_SNAPSHOT];
	static int cg_numTriggerEntities;
	static ClientEntity *cg_solidEntities[MAX_ENTITIES_IN_SNAPSHOT];
	static int cg_numSolidEntities;
	static pmove_t cg_pmove;
	static bool localClient;
	static int cg_drawTeam;

private:
	static markPoly_t cg_markPolys[MAX_MARK_POLYS];
	static int consoleIdx;
};
extern particleWorld_t pw;
extern refexport_t	refExport;

// tag names of the vehicle parts, cg_vehicledraw.cpp
extern char *plane_tags[BP_PLANE_MAX_PARTS];
extern char *gv_tags[BP_GV_MAX_PARTS];


#endif // __CG_LOCAL_H__
