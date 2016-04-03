/*
 * $Id: g_local.h,v 1.13 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_local.h -- local definitions for game module

#ifndef __G_LOCAL_H__
#define __G_LOCAL_H__

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
//#include "bg_vehicleinfo.h"
#include "g_public.h"
#include "g_client.h"
#include <vector>

//==================================================================

#define INFINITE			1000000

#define	FRAMETIME			100					// msec
#define	EVENT_VALID_MSEC	300

#define	INTERMISSION_DELAY_TIME	1000
#define	SP_INTERMISSION_DELAY_TIME	5000

// gentity->flags
#define	FL_GODMODE				0x00000010
#define	FL_NOTARGET				0x00000020
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_DROPPED_ITEM			0x00001000
#define FL_NO_BOTS				0x00002000	// spawn point not for bot use
#define FL_NO_HUMANS			0x00004000	// spawn point just for bots

#define CS_NOKILL				0x00000001
#define CS_LASTPOS				0x00000002

// movers are things like doors, plats, buttons, etc
//typedef enum {
//	MOVER_POS1,
//	MOVER_POS2,
//	MOVER_1TO2,
//	MOVER_2TO1
//} moverState_t;

//============================================================================

//#define MAX_EVENT_QUEUE_SIZE	20

//typedef struct {
//	int			event;
//	int			eventParm;
//	bool	used;
//} event_t;

//struct waypoint_t;

//struct gentity_t 
//{
//	entityState_t	s;				// communicated by server to clients
//	entityShared_t	r;				// shared by both the server system and game
//
//	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
//	// EXPECTS THE FIELDS IN THAT ORDER!
//	//================================
//
//	struct gclient_t	*client;			// NULL if not a client
//
//	bool	inuse;
//
//	char		*classname;			// set in QuakeEd
//	int			spawnflags;			// set in QuakeEd
//
//	bool	neverFree;			// if true, FreeEntity will only unlink
//									// bodyque uses this
//
//	int			flags;				// FL_* variables
//
//	char		*model;
//	char		*model2;
//	int			freetime;			// level.time when the object was freed
//	
//	int			eventTime;			// events will be cleared EVENT_VALID_MSEC after set
//	bool	freeAfterEvent;
//	bool	unlinkAfterEvent;
//
//	bool	physicsObject;		// if true, it can be pushed by movers and fall off edges
//									// all game items are physicsObjects, 
//	float		physicsBounce;		// 1.0 = continuous bounce, 0.0 = no bounce
//	int			clipmask;			// brushes with this content value will be collided against
//									// when moving.  items and corpses do not collide against
//									// players, for instance
//
//	// movers
//	moverState_t moverState;
//	int			soundPos1;
//	int			sound1to2;
//	int			sound2to1;
//	int			soundPos2;
//	int			soundLoop;
//	gentity_t	*parent;
//	gentity_t	*nextTrain;
//	gentity_t	*prevTrain;
//	vec3_t		pos1, pos2;
//
//	char		*message;
//
//	int			timestamp;		// body queue sinking, etc
//
//	float		angle;			// set in editor, -1 = up, -2 = down
//	char		*target;
//	char		*targetname;
//	char		*team;
//	char		*targetShaderName;
//	char		*targetShaderNewName;
//	gentity_t	*target_ent;
//
//	float		speed;
//	vec3_t		movedir;
//
//	int			nextthink;
//	void		(*think)(gentity_t *self);
//	void		(*reached)(gentity_t *self);	// movers call this when hitting endpoint
//	void		(*blocked)(gentity_t *self, gentity_t *other);
//	void		(*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
//	void		(*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
//	void		(*pain)(gentity_t *self, gentity_t *attacker, int damage);
//	void		(*die)(gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int mod);
//
//	int			pain_debounce_time;
//	int			fly_sound_debounce_time;	// wind tunnel
//	int			last_move_time;
//
//	int			health;
//
//	bool	takedamage;
//
//	int			damage;
//	int			splashDamage;	
//	int			splashRadius;
//	int			methodOfDeath;
//	int			splashMethodOfDeath;
//
//	int			count;
//
//	gentity_t	*chain;
//	gentity_t	*enemy;
//	gentity_t	*activator;
//	gentity_t	*teamchain;		// next entity in team
//	gentity_t	*teammaster;	// master of the team
//
//	int			watertype;
//	int			waterlevel;
//
//	int			noise_index;
//
//	// timing variables
//	float		wait;
//	float		random;
//
//	gitem_t		*item;			// for bonus items
//
//	// MFQ3
//	int			 ONOFF;			// to remember status after leaving vehicle
//	int			 left;			// for projectiles
//	int			 bulletpos;		// for dual guns (or more)
//	float		 gearheight;	
//	int			 rechargetime;	// when to recharge next
//	gentity_t	*radio_target;
//	int			 ent_category;
//	int			 radartime;
//	int			 score;			// for func_explosive teamplay count
//	int			 tracerindex;	// for tracers
//	int			 idxScriptBegin;// first script for this ent
//	int			 idxScriptEnd;	// last script for this ent
//	waypoint_t*	 nextWaypoint;
//	gentity_t*	 tracktarget;
//	int			 locktime;
//	float		 targetcat;		// for weapons mfq3
//	float		 catmodifier;	// for weapons mfq3
//	float		 range;			// for weapons mfq3
//	float		 followcone;	// for weapons mfq3
//	int			 lastDist;
//	bool	 updateGear;
//	bool	 updateBay;
//	completeLoadout_t loadout;
//	bool	 eventSent;
//	int			 currentEventToSend;
//	int			 currentEventToAdd;
//	event_t		 eventQueue[MAX_EVENT_QUEUE_SIZE];
//	int			 gi_nextScanTime;
//	float		 gi_lockangle;
//	int			 gi_reloadTime;
//	int			 basicECMVulnerability;
//	int			 nukeTime;			// For nuke effect/damage
//	int			 nukeShockTime;		// For nuke effect/damage
//};


//typedef enum {
//	CON_DISCONNECTED,
//	CON_CONNECTING,
//	CON_CONNECTED
//} clientConnected_t;
//
//typedef enum {
//	SPECTATOR_NOT,
//	SPECTATOR_FREE,
//	SPECTATOR_FOLLOW,
//	SPECTATOR_SCOREBOARD
//} spectatorState_t;
//
//typedef enum {
//	TEAM_BEGIN,		// Beginning a team game, spawn at base
//	TEAM_ACTIVE		// Now actively playing
//} playerTeamStateState_t;

//typedef struct {
//	playerTeamStateState_t	state;
//
//	int			location;
//
//	int			captures;
//	int			basedefense;
//	int			carrierdefense;
//	int			flagrecovery;
//	int			fragcarrier;
//	int			assists;
//
//	float		lasthurtcarrier;
//	float		lastreturnedflag;
//	float		flagsince;
//	float		lastfraggedcarrier;
//} playerTeamState_t;

// the auto following clients don't follow a specific client
// number, but instead follow the first two active players
#define	FOLLOW_ACTIVE1	-1
#define	FOLLOW_ACTIVE2	-2

// client data that stays across multiple levels or tournament restarts
// this is achieved by writing all the data to cvar strings at game shutdown
// time and reading them back at connection time.  Anything added here
// MUST be dealt with in G_InitSessionData() / G_ReadSessionData() / G_WriteSessionData()
//typedef struct {
//	team_t		sessionTeam;
//	int			spectatorTime;		// for determining next-in-line to play
//	spectatorState_t	spectatorState;
//	int			spectatorClient;	// for chasecam and follow mode
//	int			wins, losses;		// tournament stats
//	int			teamLeader;			// true when this client is a team leader
//} clientSession_t;

//
#define MAX_NETNAME			36
#define	MAX_VOTE_COUNT		3

// client data that stays across multiple respawns, but is cleared
// on each level change or team change at ClientBegin()
//typedef struct {
//	clientConnected_t	connected;	
//	usercmd_t	cmd;				// we would lose angles if not persistant
//	bool	localClient;		// true if "ip" info key is "localhost"
//	bool	initialSpawn;		// the first spawn should be at a cool location
//	bool	predictItemPickup;	// based on cg_predictItems userinfo
//	bool	pmoveFixed;			//
//	char		netname[MAX_NETNAME];
//	int			maxHealth;			// for handicapping
//	int			enterTime;			// level.time the client entered the game
//	playerTeamState_t teamState;	// status in teamplay games
//	int			voteCount;			// to prevent people from constantly calling votes
//	int			teamVoteCount;		// to prevent people from constantly calling votes
//	bool	teamInfo;			// send team overlay updates?
//	int			lastRadar;			// MFQ3 to remember radar setting
//} clientPersistant_t;


// this structure is cleared on each ClientSpawn(),
// except for 'client->pers' and 'client->sess'
//struct gclient_t {
//	// ps MUST be the first element, because the server expects it
//	playerState_t	ps;				// communicated by server to clients
//
//	// the rest of the structure is private to game
//	clientPersistant_t	pers;
//	clientSession_t		sess;
//
//	bool	readyToExit;		// wishes to leave the intermission
//
//	bool	noclip;
//
//	int			lastCmdTime;		// level.time of last usercmd_t, for EF_CONNECTION
//									// we can't just use pers.lastCommand.time, because
//									// of the g_sycronousclients case
//	int			buttons;
//	int			oldbuttons;
//	int			latched_buttons;
//
//	vec3_t		oldOrigin;
//
//	// sum up damage over an entire frame, so
//	// shotgun blasts give a single big kick
//	int			damage_done;		// damage taken out of health
//	int			damage_knockback;	// impact damage
//	vec3_t		damage_from;		// origin for vector calculation
//	bool	damage_fromWorld;	// if true, don't use the damage_from vector
//
//	int			accurateCount;		// for "impressive" reward sound
//
//	int			accuracy_shots;		// total number of shots
//	int			accuracy_hits;		// total number of hits
//
//	//
//	int			lastkilled_client;	// last client that this client killed
//	int			lasthurt_client;	// last client that damaged this client
//	int			lasthurt_mod;		// type of damage the client did
//
//	// timers
//	int			respawnTime;		// can respawn when time > this, force after g_forcerespwan
//	int			inactivityTime;		// kick players when time > this
//	bool		inactivityWarning;	// true if the five seoond warning has been given
//
//	int			airOutTime;
//
//	int			lastKillTime;		// for multiple kill rewards
//
////	bool	fireHeld;			// used for hook
////	gentity_t	*hook;				// grapple hook if out
//
//	int			switchTeamTime;		// time the player switched teams
//
//	// timeResidual is used to handle events that happen every second
//	// like health countdowns and regeneration
//	int			timeResidual;
//
//	char		*areabits;
//
//	// MFQ3
//	int			vehicle;			// index of selected vehicle
//	int			nextVehicle;	
//	int			advancedControls;
//};


//
//
// MFQ3
//
//



// task types
typedef enum {
	ST_ON_WAYPOINT
} taskTypes_t;

// action types
typedef enum {
	AT_SET_STATE,
	AT_NEXT_WAYPOINT
} actionTypes_t;

struct GameEntity;


// scripts
//#define MAX_SCRIPT_TASKS		1024
//
//struct waypoint_t;

//struct scripttask_t 
//{
//	taskTypes_t			type;						// type of task
//	GameEntity*			owner;						// which drone ownes this task
//	char				name[MAX_NAME_LENGTH];
//
//	waypoint_t*			nextWp;							// set nextwaypoint of owner to this
////	void				(*action1)(gentity_t *self);
//};
//
//typedef struct scripttaskList_s scripttaskList_t;
//
//struct scripttaskList_s {
//	scripttask_t	scripts[MAX_SCRIPT_TASKS];	// all the scripts
//	unsigned int	usedSTs;					// how many of the list are actually used
//};

// waypoints
//int findWaypoint( const char* name );

//struct waypoint_t 
//{
//	char			name[MAX_NAME_LENGTH];	// name from targetname
//	vec3_t			pos;					// position in map
//	unsigned int	index;					// index in waypoint list
//};
//
//#define MAX_WAYPOINTS	2048		// might need to be increased
//
//struct waypointList_t 
//{
//	waypoint_t		waypoints[MAX_WAYPOINTS];	// all the waypoints
//	unsigned int	usedWPs;					// how many of the list are actually used
//};

//
//
//
//
//



//
// this structure is cleared as each map is entered
//
//#define	MAX_SPAWN_VARS			64
//#define	MAX_SPAWN_VARS_CHARS	2048
//
//struct level_locals_t
//{
//	gclient_t		*clients;		// [maxclients]
//
//	gentity_t		*gentities;
//	int				gentitySize;
//	int				num_entities;		// current number, <= MAX_GENTITIES
//
//	int				warmupTime;			// restart match at this time
//
//	fileHandle_t	logFile;
//
//	// store latched cvars here that we want to get at often
//	int				maxclients;
//
//	int				framenum;
//	int				time;					// in msec
//	int				previousTime;			// so movers can back up when blocked
//
//	int				startTime;				// level.time the map was started
//
//	int				teamScores[TEAM_NUM_TEAMS];
//	int				lastTeamLocationTime;		// last time of client team location update
//
//	bool			newSession;				// don't use any old session data, because
//										// we changed gametype
//
//	bool			restarted;				// waiting for a map_restart to fire
//
//	int				numConnectedClients;
//	int				numNonSpectatorClients;	// includes connecting clients
//	int				numPlayingClients;		// connected, non-spectators
//	int				sortedClients[MAX_CLIENTS];		// sorted by score
//	int				follow1, follow2;		// clientNums for auto-follow spectators
//
//	//int			snd_fry;				// sound index for standing in lava
//
//	int				warmupModificationCount;	// for detecting if g_warmup is changed
//
//	// voting state
//	char			voteString[MAX_STRING_CHARS];
//	char			voteDisplayString[MAX_STRING_CHARS];
//	int				voteTime;				// level.time vote was called
//	int				voteExecuteTime;		// time the vote is executed
//	int				voteYes;
//	int				voteNo;
//	int				numVotingClients;		// set by CalculateRanks
//
//	// team voting state
//	char			teamVoteString[2][MAX_STRING_CHARS];
//	int				teamVoteTime[2];		// level.time vote was called
//	int				teamVoteYes[2];
//	int				teamVoteNo[2];
//	int				numteamVotingClients[2];// set by CalculateRanks
//
//	// spawn variables
//	bool			spawning;				// the G_Spawn*() functions are valid
//	int				numSpawnVars;
//	char			*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
//	int				numSpawnVarChars;
//	char			spawnVarChars[MAX_SPAWN_VARS_CHARS];
//	// MFQ3 spawning
//	int				ent_category;
//
//	// intermission state
//	int				intermissionQueued;		// intermission was qualified, but
//										// wait INTERMISSION_DELAY_TIME before
//										// actually going there so the last
//										// frag can be watched.  Disable future
//										// kills during this delay
//	int				intermissiontime;		// time the intermission was started
//	char			*changemap;
//	bool			readyToExit;			// at least one client wants to exit
//	int				exitTime;
//	vec3_t			intermission_origin;	// also used for spectator spawns
//	vec3_t			intermission_angle;
//
//	bool			locationLinked;			// target_locations get linked
//	gentity_t		*locationHead;			// head of the location list
//
//	waypointList_t	waypointList;
//	scripttaskList_t scriptList; 
//};

// a temp ent holding the contents of the fields
struct SpawnFieldHolder
{
	char*	classname_;
	vec3_t	origin_;
	char*	model_;
	char*	model2_;
	int		spawnflags_;
	float	speed_;
	char*	target_;
	char*	targetname_;
	char*	message_;
	char*	team_;
	float	wait_;
	float	random_;
	int		count_;
	int		health_;
	int		damage_;
	vec3_t	angles_;
	char*	targetShaderName_;
	char*	targetShaderNewName_;
	int		ent_category_;
	int		score_;
};


struct spawn_t
{
	char	*name;
	void	(*spawn)(GameEntity *ent);
};

//
// g_spawn.c
//
bool	G_SpawnString( const char *key, const char *defaultString, char **out );
// spawn string returns a temporary reference, you must CopyString() if you want to keep it
bool	G_SpawnFloat( const char *key, const char *defaultString, float *out );
bool	G_SpawnInt( const char *key, const char *defaultString, int *out );
bool	G_SpawnVector( const char *key, const char *defaultString, float *out );
void		G_SpawnEntitiesFromString( void );
char *G_NewString( const char *string );

//
// g_cmds.c
//
void Cmd_Score_f (GameEntity *ent);
void StopFollowing( GameEntity *ent );
void BroadcastTeamChange( GameClient *client, int oldTeam );
void SetTeam( GameEntity *ent, char *s );
void Cmd_FollowCycle_f( GameEntity *ent, int dir );

//
// g_items.c
//
void G_CheckTeamItems( void );
void G_RunItem( GameEntity *ent );
void RespawnItem( GameEntity *ent );

void PrecacheItem (gitem_t *it);
GameEntity *Drop_Item( GameEntity *ent, gitem_t *item, float angle );
GameEntity *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity );
void SetRespawn (GameEntity *ent, float delay);
//void G_SpawnItem (SpawnFieldHolder *sfh, gitem_t *item);
//void FinishSpawningItem( GameEntity *ent );
void Think_Weapon (GameEntity *ent);
//void Touch_Item (GameEntity *ent, GameEntity *other, trace_t *trace);

void ClearRegisteredItems( void );
void RegisterItem( gitem_t *item );
void SaveRegisteredItems( void );

//
// g_utils.c
//
int G_ModelIndex( char *name );
int		G_SoundIndex( char *name );
void	G_TeamCommand( ClientBase::eTeam team, char *cmd );
void	G_KillBox (GameEntity *ent);
GameEntity *G_Find (GameEntity *from, int fieldofs, const char *match);
GameEntity *G_PickTarget (char *targetname);
void	G_UseTargets (GameEntity *ent, GameEntity *activator);
void	G_SetMovedir ( vec3_t angles, vec3_t movedir);

//void	G_InitGentity( GameEntity *e );
//GameEntity	*G_Spawn ();
GameEntity *G_TempEntity( vec3_t origin, int event );
void	G_Sound( GameEntity *ent, int channel, int soundIndex );
//void	G_FreeEntity( GameEntity *e );
bool	G_EntitiesFree();

void	G_TouchTriggers (GameEntity *ent);
void	G_TouchSolids (GameEntity *ent);

float	*tv (float x, float y, float z);
char	*vtos( const vec3_t v );

float vectoyaw( const vec3_t vec );

void G_AddPredictableEvent( GameEntity *ent, int event, int eventParm );
void G_AddEvent( GameEntity *ent, int event, int eventParm, bool addToQueue );
void G_SendEventFromQueue( GameEntity* ent );
void G_SetOrigin( GameEntity *ent, vec3_t origin );
void AddRemap(const char *oldShader, const char *newShader, float timeOffset);
const char *BuildShaderStateConfig();

// MFQ3
extern unsigned long G_GetGameset();

//
// g_combat.c
//
bool CanDamage (GameEntity *targ, vec3_t origin);
void G_Damage (GameEntity *targ, GameEntity *inflictor, GameEntity *attacker, vec3_t dir, vec3_t point, int damage, int dflags, int mod, long cat);
bool G_RadiusDamage (vec3_t origin, GameEntity *attacker, float damage, float radius, GameEntity *ignore, int mod, long cat);
int G_InvulnerabilityEffect( GameEntity *targ, vec3_t dir, vec3_t point, vec3_t impactpoint, vec3_t bouncedir );

void VehicleDeathImpl( GameEntity *self, GameEntity *inflictor, GameEntity *attacker, int damage, int meansOfDeath );
void TossVehicleFlags( GameEntity *self );
void ExplodeVehicle( GameEntity *self );

// MFQ3
// g_plane.c
//
//void checkTakeoffLandingPlane( GameEntity *self );
void Touch_Plane( GameEntity *ent, GameEntity *other, trace_t *trace );
void Plane_Pain( GameEntity *self, GameEntity *attacker, int damage );

// MFQ3
// g_groundvehicle.c
//
void Touch_GroundVehicle( GameEntity *ent, GameEntity *other, trace_t *trace );
void GroundVehicle_Pain( GameEntity *self, GameEntity *attacker, int damage );

// MFQ3
// g_helo.c
//
//void checkTakeoffLandingHelo( GameEntity *self );
void Touch_Helo( GameEntity *ent, GameEntity *other, trace_t *trace );
void Helo_Pain( GameEntity *self, GameEntity *attacker, int damage );

// MFQ3
// g_lqm.c
//
void Touch_LQM( GameEntity *ent, GameEntity *other, trace_t *trace );
void LQM_Pain( GameEntity *self, GameEntity *attacker, int damage );

// MFQ3
// g_boat.c
//
void Touch_Boat( GameEntity *ent, GameEntity *other, trace_t *trace );
void Boat_Pain( GameEntity *self, GameEntity *attacker, int damage );


//
// MFQ3
//
//void DroneInit();
//void G_LoadMissionScripts();
//void SP_misc_vehicle (GameEntity *ent);
//void SP_misc_groundinstallation (GameEntity *ent);


// damage flags
#define DAMAGE_RADIUS				0x00000001	// damage was indirect
#define DAMAGE_MFQ3_FREE			0x00000002	// --- unused ---
#define DAMAGE_NO_KNOCKBACK			0x00000004	// do not affect velocity, just view angles
#define DAMAGE_NO_PROTECTION		0x00000008  // shields, invulnerability, and godmode have no effect


//
// g_missile.c
//
void G_RunMissile( GameEntity *ent );

void fire_antiair (GameEntity *self);
void fire_antiground (GameEntity *self);
void fire_ffar (GameEntity *self);
void fire_ironbomb (GameEntity *self);
void fire_autocannon (GameEntity *self, bool main);
void fire_maingun (GameEntity *self);
void fire_flare (GameEntity *self);
void fire_flare2 (GameEntity *self, vec3_t start, vec3_t up, int age);
void fire_cflare (GameEntity *self);
void drop_fueltank (GameEntity *self);
void drop_crate (GameEntity *self);
void fire_nukebomb (GameEntity *self);
void fire_nukemissile (GameEntity *self);
void LaunchMissile_GI( GameEntity* ent );
void fire_flak_GI (GameEntity *self);
void fire_autocannon_GI (GameEntity *self);

//
// g_mover.c
//
void G_RunMover( GameEntity *ent );

//
// g_trigger.c
//
void trigger_teleporter_touch (GameEntity *self, GameEntity *other, trace_t *trace );


//
// g_misc.c
//

//
// g_mfq3util.c
//
int canLandOnIt( GameEntity *ent );
void untrack( GameEntity *ent );
void track( GameEntity* ent, GameEntity* target );
void unlock( GameEntity *ent );
void updateTargetTracking( GameEntity *ent );

//
// g_droneutil.c
//
//void onWaypointEvent( GameEntity* ent );
//void getTargetDirAndDist( GameEntity* ent );

//
// g_mfq3ents.c
//


//
// g_weapon.c
//
bool LogAccuracyHit( GameEntity *target, GameEntity *attacker );
void CalcMuzzlePoint ( GameEntity *ent, vec3_t forward, vec3_t right, vec3_t up, vec3_t muzzlePoint );
void SnapVectorTowards( vec3_t v, vec3_t to );

//
// g_client.c
//
int TeamCount( int ignoreClientNum, int team );
int TeamLeader( int team );
ClientBase::eTeam PickTeam( int ignoreClientNum );
void SetClientViewAngle( GameEntity *ent, vec3_t angle );
GameEntity *SelectSpawnPoint ( vec3_t avoidPoint, vec3_t origin, vec3_t angles, int category );
void respawn (GameEntity *ent);
void switch_vehicle (GameEntity *ent);
void BeginIntermission (void);
void InitClientPersistant (GameClient *client);
void InitClientResp (GameClient *client);
void ClientSpawn( GameEntity *ent );
void AddScore( GameEntity *ent, vec3_t origin, int score );
void CalculateRanks( void );
bool SpotWouldTelefrag( GameEntity *spot );

// mfq3
// mf_client.c
//
char *MF_ClientConnect( int clientNum, bool firstTime, bool isBot );
void MF_ClientBegin( int clientNum );
void MF_ClientSpawn( int clientNum, long cs_flags);

// mfq3
// mf_vehiclespawn.c
//
void MF_Spawn_Plane(GameEntity *ent, int idx, bool landed);
void MF_Spawn_GroundVehicle(GameEntity *ent, int idx);
void MF_Spawn_Helo(GameEntity *ent, int idx, bool landed);
void MF_Spawn_LQM(GameEntity *ent, int idx);
void MF_Spawn_Boat(GameEntity *ent, int idx);

// mfq3
// g_scripts.c
//
//int LoadVehicleScripts( GameEntity* ent, char *filename );

// mfq3
// drones
//
//void Drone_Ground_Think( GameEntity* ent );
//void Drone_Plane_Think( GameEntity* ent );
//void GroundInstallation_Think( GameEntity *ent );

//
// g_svcmds.c
//
bool ConsoleCommand();
void G_ProcessIPBans(void);
bool G_FilterPacket (char *from);

//
// g_weapon.c
//
void FireWeapon( GameEntity *ent );
void FireWeapon_GI( GameEntity *ent );
//
// p_hud.c
//
void MoveClientToIntermission (GameEntity *client);
void G_SetStats (GameEntity *ent);
void DeathmatchScoreboardMessage (GameEntity *client);

//
// g_cmds.c
//

//
// g_pweapon.c
//


//
// g_main.c
//
void FindIntermissionPoint( void );
void SetLeader(int team, int client);
void CheckTeamLeader( int team );
void G_RunThink (GameEntity *ent);
void QDECL G_LogPrintf( const char *fmt, ... );
void SendScoreboardMessageToAllClients( void );
void QDECL Com_Printf( const char *fmt, ... );
//void QDECL G_Error( const char *fmt, ... );

//
// g_client.c
//
void ClientUserinfoChanged( int clientNum );
void ClientDisconnect( int clientNum );
void ClientCommand( int clientNum );

//
// g_active.c
//
void ClientThink( int clientNum );
void ClientEndFrame( GameEntity *ent );
void G_RunClient( GameEntity *ent );

//
// g_team.c
//
bool OnSameTeam( GameEntity *ent1, GameEntity *ent2 );
void Team_CheckDroppedItem( GameEntity *dropped );
bool CheckObeliskAttack( GameEntity *obelisk, GameEntity *attacker );

//
// g_mem.c
//
void *G_Alloc( int size );
void G_InitMemory( void );
void Svcmd_GameMem_f( void );

//
// g_session.c
//
void G_ReadSessionData( GameClient *client );
void G_InitSessionData( GameClient *client, char *userinfo );

void G_InitWorldSession();
void G_WriteSessionData();

//
// g_arenas.c
//
void UpdateTournamentInfo( void );

#define MAX_FILEPATH			144

#include "g_team.h" // teamplay specific stuff


//extern	level_locals_t	level;
struct LevelLocals;
extern LevelLocals& theLevel;

//extern	gentity_t		g_entities[MAX_GENTITIES];

#define	FOFS(x) ((int)&(((GameEntity *)0)->x))
#define	FOFS2(x) ((int)&(((SpawnFieldHolder *)0)->x))

extern	vmCvar_t	g_gametype;
extern	vmCvar_t	g_dedicated;
extern	vmCvar_t	g_cheats;
extern	vmCvar_t	g_maxclients;			// allow this many total, including spectators
extern	vmCvar_t	g_maxGameClients;		// allow this many active
extern	vmCvar_t	g_restarted;

extern	vmCvar_t	g_dmflags;
extern	vmCvar_t	g_fraglimit;
extern	vmCvar_t	g_timelimit;
extern	vmCvar_t	g_capturelimit;
extern	vmCvar_t	g_friendlyFire;
extern	vmCvar_t	g_password;
extern	vmCvar_t	g_needpass;
extern	vmCvar_t	g_gravity;
extern	vmCvar_t	g_speed;
extern	vmCvar_t	g_knockback;
extern	vmCvar_t	g_forcerespawn;
extern	vmCvar_t	g_inactivity;
extern	vmCvar_t	g_debugMove;
extern	vmCvar_t	g_debugAlloc;
extern	vmCvar_t	g_debugDamage;
extern	vmCvar_t	g_weaponRespawn;
extern	vmCvar_t	g_weaponTeamRespawn;
extern	vmCvar_t	g_synchronousClients;
extern	vmCvar_t	g_motd;
extern	vmCvar_t	g_warmup;
extern	vmCvar_t	g_doWarmup;
extern	vmCvar_t	g_allowVote;
extern	vmCvar_t	g_teamAutoJoin;
extern	vmCvar_t	g_teamForceBalance;
extern	vmCvar_t	g_teamForceBalanceNum;
extern	vmCvar_t	g_banIPs;
extern	vmCvar_t	g_filterBan;
extern	vmCvar_t	g_redteam;
extern	vmCvar_t	g_blueteam;
extern	vmCvar_t	g_smoothClients;
extern	vmCvar_t	pmove_fixed;
extern	vmCvar_t	pmove_msec;
extern	vmCvar_t	g_rankings;
extern	vmCvar_t	g_enableDust;
extern	vmCvar_t	g_enableBreath;
extern	vmCvar_t	g_singlePlayer;
extern	vmCvar_t	g_spectSpeed;

// MFQ3
extern  vmCvar_t	mf_gameset;
extern  vmCvar_t	mf_lvcat;
extern  vmCvar_t	mf_mission;
extern	vmCvar_t	mf_allowNukes;


// system calls
int Sys_Milliseconds ();
void Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void Cvar_Update( vmCvar_t *vmCvar );
void Cvar_Set( const char *var_name, const char *value );
void Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
int Cvar_VariableIntegerValue( const char *var_name );
int	Cmd_Argc();
void Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength );
void Cbuf_ExecuteText (int exec_when, const char *text);
int FS_FOpenFileByMode( const char *qpath, fileHandle_t *f, fsMode_t mode );
void FS_FCloseFile( fileHandle_t f );
int FS_Read2( void *buffer, int len, fileHandle_t f );
int FS_Write( const void *buffer, int len, fileHandle_t h );
void SV_GetUsercmd( int clientNum, usercmd_t *cmd );
void Sys_SnapVector( float *v );
bool SV_GetEntityToken( char *buffer, int bufferSize );
bool SV_EntityContact( const vec3_t mins, const vec3_t maxs, EntityBase* gameEnt, bool capsule );
int SV_AreaEntities( const vec3_t mins, const vec3_t maxs, int *entityList, int maxcount );
void SV_LinkEntity( EntityBase* ent );
void SV_UnlinkEntity( EntityBase* ent );
bool SV_inPVS (const vec3_t p1, const vec3_t p2);
int SV_PointContents( const vec3_t p, int passEntityNum );
void SV_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask, bool capsule );
void SV_GetServerinfo( char *buffer, int bufferSize );
void SV_SetUserinfo( int index, const char *val );
void SV_GetUserinfo( int index, char *buffer, int bufferSize );
void SV_GetConfigstring( int index, char *buffer, int bufferSize );
void SV_SetConfigstring (int index, const char *val);
void SV_GameSendServerCommand( int clientNum, const char *text );
void SV_GameDropClient( int clientNum, const char *reason );
void SV_AdjustAreaPortalState( EntityBase* gameEnt, int open );
void SV_SetBrushModel( EntityBase* gameEnt, const char *name );
//void SV_LocateGameData( void* gEnts, int numGEntities, int sizeofGEntity_t,
//					   playerState_t *clients, int sizeofGameClient );
void SV_LocateGameData( std::vector<EntityBase*>* gameEntities, 
					    std::vector<ClientBase*>* gameClients );





#endif //__G_LOCAL_H__