// g_local.h -- local definitions for game module

#include "q_shared.h"

// define GAME_INCLUDE so that game.h does not define the
// short, server-visible gclient_t and edict_t structures,
// because we define the full size ones in this file
#define	GAME_INCLUDE
#include "game.h"

// the "gameversion" client command will print this plus compile date
#define	GAMEVERSION	"New AirQuake2"

// protocol bytes that can be directly added to messages
#define	svc_muzzleflash		1
#define	svc_muzzleflash2	2
#define	svc_temp_entity		3
#define	svc_layout			4
#define	svc_inventory		5
#define	svc_stufftext		11

//==================================================================

// view pitching times
#define DAMAGE_TIME		0.5
#define	FALL_TIME		0.3


// AirQuake2
#define MAX_BOTS								8
#define MAX_WAYPOINTS							10
#define MAX_WEAPONS								5
#define MAX_GUNTYPES							7
#define TEAM_1									1
#define TEAM_2									2
#define TEAM_NEUTRAL							4
#define MAX_TEAMS								5	// teams[3] and teams[4] structure is unused!
#define MAX_CLASSES								13


// edict->spawnflags
// these are set with checkboxes on each entity in the map editor
#define	SPAWNFLAG_NOT_EASY			0x00000100
#define	SPAWNFLAG_NOT_MEDIUM		0x00000200
#define	SPAWNFLAG_NOT_HARD			0x00000400
#define	SPAWNFLAG_NOT_DEATHMATCH	0x00000800
#define	SPAWNFLAG_NOT_COOP			0x00001000

// edict->flags
#define	FL_FLY					0x00000001
#define	FL_SWIM					0x00000002	// implied immunity to drowining
#define FL_IMMUNE_LASER			0x00000004
#define	FL_INWATER				0x00000008
//#define	FL_GODMODE				0x00000010
//#define	FL_NOTARGET				0x00000020
#define FL_IMMUNE_SLIME			0x00000040
#define FL_IMMUNE_LAVA			0x00000080
#define	FL_PARTIALGROUND		0x00000100	// not all corners are valid
#define	FL_WATERJUMP			0x00000200	// player jumping out of water
#define	FL_TEAMSLAVE			0x00000400	// not the first on the team
#define FL_NO_KNOCKBACK			0x00000800
#define FL_RESPAWN				0x80000000	// used for item respawning


#define	FRAMETIME		0.1

// memory tags to allow dynamic memory to be cleaned up
#define	TAG_GAME	765		// clear when unloading the dll
#define	TAG_LEVEL	766		// clear when loading a new level


#define BODY_QUEUE_SIZE		8

typedef enum
{
	DAMAGE_NO,
	DAMAGE_YES,			// will take damage if hit
	DAMAGE_AIM			// auto targeting recognizes this
} damage_t;

typedef enum 
{
	WEAPON_READY, 
	WEAPON_ACTIVATING,
	WEAPON_DROPPING,
	WEAPON_FIRING
} weaponstate_t;

typedef enum
{
	AMMO_BULLETS,
	AMMO_SHELLS,
	AMMO_ROCKETS,
	AMMO_SIDEWINDER,
	AMMO_AMRAAM,
	AMMO_BOMBS,
	AMMO_ATGM,
	AMMO_SMART,
	AMMO_FLARES
} ammo_t;


//deadflag
#define DEAD_NO					0
#define DEAD_DYING				1
#define DEAD_DEAD				2
#define DEAD_RESPAWNABLE		3

//gib types
#define GIB_ORGANIC				0
#define GIB_METALLIC			1

//monster ai flags
#define AI_STAND_GROUND			0x00000001
#define AI_TEMP_STAND_GROUND	0x00000002
#define AI_SOUND_TARGET			0x00000004
#define AI_LOST_SIGHT			0x00000008
#define AI_PURSUIT_LAST_SEEN	0x00000010
#define AI_PURSUE_NEXT			0x00000020
#define AI_PURSUE_TEMP			0x00000040
#define AI_HOLD_FRAME			0x00000080
#define AI_GOOD_GUY				0x00000100
#define AI_BRUTAL				0x00000200
#define AI_NOSTEP				0x00000400
#define AI_DUCKED				0x00000800
#define AI_COMBAT_POINT			0x00001000
#define AI_MEDIC				0x00002000
#define AI_RESURRECTING			0x00004000

//monster attack state
#define AS_STRAIGHT				1
#define AS_SLIDING				2
#define	AS_MELEE				3
#define	AS_MISSILE				4

// handedness values
#define RIGHT_HANDED			0
#define LEFT_HANDED				1
#define CENTER_HANDED			2


// game.serverflags values
#define SFL_CROSS_TRIGGER_1		0x00000001
#define SFL_CROSS_TRIGGER_2		0x00000002
#define SFL_CROSS_TRIGGER_3		0x00000004
#define SFL_CROSS_TRIGGER_4		0x00000008
#define SFL_CROSS_TRIGGER_5		0x00000010
#define SFL_CROSS_TRIGGER_6		0x00000020
#define SFL_CROSS_TRIGGER_7		0x00000040
#define SFL_CROSS_TRIGGER_8		0x00000080
#define SFL_CROSS_TRIGGER_MASK	0x000000ff


// edict->movetype values
typedef enum
{
MOVETYPE_NONE,			// never moves
MOVETYPE_NOCLIP,		// origin and angles change with no interaction
MOVETYPE_PUSH,			// no clip to world, push on box contact
MOVETYPE_STOP,			// no clip to world, stops on box contact

MOVETYPE_WALK,			// gravity
MOVETYPE_STEP,			// gravity, special edge handling
MOVETYPE_FLY,
MOVETYPE_TOSS,			// gravity
MOVETYPE_FLYMISSILE,	
MOVETYPE_BOUNCE,
MOVETYPE_PLANE,
MOVETYPE_HELO,
MOVETYPE_LQM
} movetype_t;


// gitem_t->flags
#define	IT_WEAPON		1		// use makes active weapon
#define	IT_AMMO			2
#define IT_STAY_COOP	4
#define IT_KEY			8
#define IT_POWERUP		16

typedef struct gitem_s
{
	char		*classname;	// spawning name
	qboolean	(*pickup)(struct edict_s *ent, struct edict_s *other);
	void		(*use)(struct edict_s *ent, struct gitem_s *item);
	void		(*drop)(struct edict_s *ent, struct gitem_s *item);
	void		(*weaponthink)(struct edict_s *ent);
	char		*pickup_sound;
	char		*world_model;
	int			world_model_flags;
	char		*view_model;

	// client side info
	char		*icon;
	char		*pickup_name;	// for printing on pickup
	int			count_width;		// number of digits to display by icon

	int			quantity;		// for ammo how much, for weapons how much is used per shot
	char		*ammo;			// for weapons
	int			flags;			// IT_* flags

	int			weapmodel;		// weapon model index (for weapons)

	void		*info;
	int			tag;

	char		*precaches;		// string of all models, sounds, and images this item will use
} gitem_t;



//
// this structure is left intact through an entire game
// it should be initialized at dll load time, and read/written to
// the server.ssv file for savegames
//
typedef struct
{
	char		helpmessage1[512];
	char		helpmessage2[512];
	int			helpchanged;	// flash F1 icon if non 0, play sound
								// and increment only if 1, 2, or 3

	gclient_t	*clients;		// [maxclients]

	// can't store spawnpoint in level, because
	// it would get overwritten by the savegame restore
	char		spawnpoint[512];	// needed for coop respawns

	// store latched cvars here that we want to get at often
	int			maxclients;
	int			maxentities;

	// cross level triggers
	int			serverflags;

	// items
	int			num_items;

	qboolean	autosaved;
} game_locals_t;

// New AirQuake2
typedef struct
{
	int		category;		// categories available for this team
	int		score;			// score for this team (in frags)
	int		rounds;			// score for this team (in won rounds)
	int		players;		// number of players for this team
	int		bots;			// number of bots for this team
} teams_t;

typedef struct
{
	int		damage;
	float	firerate;
	float	spread;			
	int		barrels;		// for anti-aircraft guns
	int		barrel_dist;	// distance from origin of barrels (->origin +- barrel_dist = startpoint)
	int		bullet_speed;
} guntypes_t;

typedef struct
{
	qboolean	inuse;
	char		botname[16];
	int			skill;
	int			fav_weapon;
	int			fav_vehicle;	
} botinfo_t;

//
// this structure is cleared as each map is entered
// it is read/written to the level.sav file for savegames
//
typedef struct
{
	int			framenum;
	float		time;

	char		level_name[MAX_QPATH];	// the descriptive name (Outer Base, etc)
	char		mapname[MAX_QPATH];		// the server name (base1, etc)
	char		nextmap[MAX_QPATH];		// go here when fraglimit is hit

	// intermission state
	float		intermissiontime;		// time the intermission was started
	char		*changemap;
	int			exitintermission;
	vec3_t		intermission_origin;
	vec3_t		intermission_angle;

	edict_t		*sight_client;	// changed once each frame for coop games

	edict_t		*sight_entity;
	int			sight_entity_framenum;
	edict_t		*sound_entity;
	int			sound_entity_framenum;
	edict_t		*sound2_entity;
	int			sound2_entity_framenum;

	int			pic_health;

	int			total_secrets;
	int			found_secrets;

	int			total_goals;
	int			found_goals;

	int			total_monsters;
	int			killed_monsters;

	edict_t		*current_entity;	// entity running from G_RunFrame
	int			body_que;			// dead bodies

	int			power_cubes;		// ugly necessity for coop

	int			category;
} level_locals_t;


// spawn_temp_t is only used to hold entity field values that
// can be set from the editor, but aren't actualy present
// in edict_t during gameplay
typedef struct
{
	// world vars
	char		*sky;
	float		skyrotate;
	vec3_t		skyaxis;
	char		*nextmap;

	int			lip;
	int			distance;
	int			height;
	char		*noise;
	float		pausetime;
	char		*item;
	char		*gravity;

	float		minyaw;
	float		maxyaw;
	float		minpitch;
	float		maxpitch;
} spawn_temp_t;


typedef struct
{
	// fixed data
	vec3_t		start_origin;
	vec3_t		start_angles;
	vec3_t		end_origin;
	vec3_t		end_angles;

	int			sound_start;
	int			sound_middle;
	int			sound_end;

	float		accel;
	float		speed;
	float		decel;
	float		distance;

	float		wait;

	// state data
	int			state;
	vec3_t		dir;
	float		current_speed;
	float		move_speed;
	float		next_speed;
	float		remaining_distance;
	float		decel_distance;
	void		(*endfunc)(edict_t *);
} moveinfo_t;


typedef struct
{
	void	(*aifunc)(edict_t *self, float dist);
	float	dist;
	void	(*thinkfunc)(edict_t *self);
} mframe_t;

typedef struct
{
	int			firstframe;
	int			lastframe;
	mframe_t	*frame;
	void		(*endfunc)(edict_t *self);
} mmove_t;

typedef struct
{
	mmove_t		*currentmove;
	int			aiflags;
	int			nextframe;
	float		scale;

	void		(*stand)(edict_t *self);
	void		(*idle)(edict_t *self);
	void		(*search)(edict_t *self);
	void		(*walk)(edict_t *self);
	void		(*run)(edict_t *self);
	void		(*dodge)(edict_t *self, edict_t *other, float eta);
	void		(*attack)(edict_t *self);
	void		(*melee)(edict_t *self);
	void		(*sight)(edict_t *self, edict_t *other);
	qboolean	(*checkattack)(edict_t *self);

	float		pausetime;
	float		attack_finished;

	vec3_t		saved_goal;
	float		search_time;
	float		trail_time;
	vec3_t		last_sighting;
	int			attack_state;
	int			lefty;
	float		idle_time;
	int			linkcount;

} monsterinfo_t;



extern	game_locals_t	game;
extern	level_locals_t	level;
extern	game_import_t	gi;
extern	game_export_t	globals;
extern	spawn_temp_t	st;

extern	int	sm_meat_index;
extern	int	snd_fry;


// means of death
//#define MOD_BLASTER			1

#define MOD_R_SPLASH		9

#define MOD_SLIME			18
#define MOD_LAVA			19
#define MOD_CRUSH			20
#define MOD_TELEFRAG		21
#define MOD_FALLING			22

#define MOD_EXPLOSIVE		25
#define MOD_BARREL			26
#define MOD_EXIT			28

#define MOD_TARGET_LASER	30
#define MOD_TRIGGER_HURT	31
#define MOD_HIT				32
#define MOD_TARGET_BLASTER	33

// AirQuake2 mods -> the above will be deleted later!
#define MOD_UNKNOWN			0
#define MOD_SIDEWINDER		1
#define MOD_STINGER			2
#define MOD_AMRAAM			3
#define MOD_PHOENIX			4
#define MOD_HELLFIRE		5
#define MOD_MAVERICK		6
#define MOD_ANTIRADAR		7
#define MOD_ROCKET			8
#define MOD_LASERBOMBS		9
#define MOD_RUNWAYBOMBS		10
#define MOD_BOMBS			11
#define MOD_WATER			17
#define MOD_SUICIDE			23
#define MOD_SPLASH			29
#define MOD_CRASH			34
#define MOD_MIDAIR			35
#define MOD_SKY				36
#define MOD_GROUND			37
#define MOD_LQM				38
#define MOD_ACCIDENT		39
#define MOD_VEHICLE			40
#define MOD_NOCOUNT			41		// if this doesn't change frag count
#define MOD_NOGEAR			42
#define MOD_CRASHLAND		43
#define MOD_SPLASHLAND		44
#define MOD_AUTOCANNON		45		// autocannon

#define MOD_FRIENDLY_FIRE	0x8000000

extern	int	meansOfDeath;


extern	edict_t			*g_edicts;

#define	FOFS(x) (int)&(((edict_t *)0)->x)
#define	STOFS(x) (int)&(((spawn_temp_t *)0)->x)
#define	LLOFS(x) (int)&(((level_locals_t *)0)->x)
#define	CLOFS(x) (int)&(((gclient_t *)0)->x)

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

extern	cvar_t	*maxentities;
extern	cvar_t	*deathmatch;
extern	cvar_t	*coop;
extern	cvar_t	*dmflags;
extern	cvar_t	*skill;
extern	cvar_t	*fraglimit;
extern	cvar_t	*timelimit;
extern	cvar_t	*password;
extern	cvar_t	*needpass;
extern	cvar_t	*g_select_empty;
extern	cvar_t	*dedicated;

extern	cvar_t	*filterban;

//extern	cvar_t	*sv_gravity;
extern	cvar_t	*sv_maxvelocity;

extern	cvar_t	*gun_x, *gun_y, *gun_z;
extern	cvar_t	*sv_rollspeed;
extern	cvar_t	*sv_rollangle;

extern	cvar_t	*run_pitch;
extern	cvar_t	*run_roll;
extern	cvar_t	*bob_up;
extern	cvar_t	*bob_pitch;
extern	cvar_t	*bob_roll;

extern	cvar_t	*sv_cheats;
extern	cvar_t	*maxclients;

extern	cvar_t	*flood_msgs;
extern	cvar_t	*flood_persecond;
extern	cvar_t	*flood_waitdelay;

extern	cvar_t	*sv_maplist;

// NewAirQuake2 cvars
extern cvar_t	*missileview;
extern cvar_t	*rollcamera;
extern cvar_t	*teamplay;
extern cvar_t	*num_bots;
extern cvar_t	*num_bots_team1;
extern cvar_t	*num_bots_team2;
extern cvar_t	*num_bots_neutral;
extern cvar_t	*name_team1;
extern cvar_t	*name_team2;
extern cvar_t   *name_neutral;
extern cvar_t	*teamspots_only;
extern cvar_t	*num_tracers;
extern cvar_t	*bots_attack_LQM;
extern cvar_t	*bots_attack_landed;
extern cvar_t   *rockettrail;

// anti-cheat
extern cvar_t	*cl_forwardspeed;
extern cvar_t	*cl_anglespeedkey;
extern cvar_t	*cl_backspeed;
extern cvar_t	*cl_pitchspeed;
extern cvar_t	*cl_sidespeed;
extern cvar_t	*cl_upspeed;
extern cvar_t	*cl_yawspeed;

extern cvar_t	*vid_ref;

// NewAirQuake2 globals
extern int			bots;
extern float		bot_time;
extern teams_t		teams[MAX_TEAMS];
extern guntypes_t	guntypes[MAX_GUNTYPES];
extern edict_t *important_spots[MAX_WAYPOINTS];
extern int			max_vehicles[MAX_CLASSES];
extern botinfo_t	botinfo[MAX_BOTS];

#define world	(&g_edicts[0])

// item spawnflags
#define ITEM_TRIGGER_SPAWN		0x00000001
#define ITEM_NO_TOUCH			0x00000002
// 6 bits reserved for editor flags
// 8 bits used as power cube id bits for coop games
#define DROPPED_ITEM			0x00010000
#define	DROPPED_PLAYER_ITEM		0x00020000
#define ITEM_TARGETS_USED		0x00040000

//
// fields are needed for spawning from the entity string
// and saving / loading games
//
#define FFL_SPAWNTEMP		1
#define FFL_NOSPAWN			2

typedef enum {
	F_INT, 
	F_FLOAT,
	F_LSTRING,			// string on disk, pointer in memory, TAG_LEVEL
	F_GSTRING,			// string on disk, pointer in memory, TAG_GAME
	F_VECTOR,
	F_ANGLEHACK,
	F_EDICT,			// index on disk, pointer in memory
	F_ITEM,				// index on disk, pointer in memory
	F_CLIENT,			// index on disk, pointer in memory
	F_FUNCTION,
	F_MMOVE,
	F_IGNORE
} fieldtype_t;

typedef struct
{
	char	*name;
	int		ofs;
	fieldtype_t	type;
	int		flags;
} field_t;


extern	field_t fields[];
extern	gitem_t	itemlist[];


//
// g_cmds.c
//
void Cmd_Help_f (edict_t *ent);
void Cmd_Score_f (edict_t *ent);

//
// g_items.c
//
void PrecacheItem (gitem_t *it);
void InitItems (void);
void SetItemNames (void);
gitem_t	*FindItem (char *pickup_name);
gitem_t	*FindItemByClassname (char *classname);
#define	ITEM_INDEX(x) ((x)-itemlist)
void SetRespawn (edict_t *ent, float delay);
void SpawnItem (edict_t *ent, gitem_t *item);
gitem_t	*GetItemByIndex (int index);
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf);

//
// g_utils.c
//
qboolean	KillBox (edict_t *ent);
void	G_ProjectSource (vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result);
edict_t *G_Find (edict_t *from, int fieldofs, char *match);
edict_t *findradius (edict_t *from, vec3_t org, float rad);
edict_t *G_PickTarget (char *targetname);
void	G_UseTargets (edict_t *ent, edict_t *activator);
void	G_SetMovedir (vec3_t angles, vec3_t movedir);

void	G_InitEdict (edict_t *e);
edict_t	*G_Spawn (void);
void	G_FreeEdict (edict_t *e);

void	G_TouchTriggers (edict_t *ent);
void	G_TouchSolids (edict_t *ent);

char	*G_CopyString (char *in);

float	*tv (float x, float y, float z);
char	*vtos (vec3_t v);

float vectoyaw (vec3_t vec);
void vectoangles (vec3_t vec, vec3_t angles);

qboolean infront (edict_t *self, edict_t *other);
qboolean visible (edict_t *self, edict_t *other);

//
// g_combat.c
//
qboolean OnSameTeam (edict_t *ent1, edict_t *ent2);
qboolean CanDamage (edict_t *targ, edict_t *inflictor);
void T_Damage (edict_t *targ, edict_t *inflictor, edict_t *attacker, vec3_t dir, vec3_t point, vec3_t normal, int damage, int knockback, int dflags, int mod);
void T_RadiusDamage (edict_t *inflictor, edict_t *attacker, float damage, edict_t *ignore, float radius, int mod);
void SpawnDamage (int type, vec3_t origin, vec3_t normal, int damage);

// damage flags
#define DAMAGE_RADIUS			0x00000001	// damage was indirect
#define DAMAGE_ENERGY			0x00000004	// damage is from an energy based weapon
#define DAMAGE_NO_KNOCKBACK		0x00000008	// do not affect velocity, just view angles
#define DAMAGE_BULLET			0x00000010  // damage is from a bullet (used for ricochets)
#define DAMAGE_NO_PROTECTION	0x00000020  // shields, invulnerability, and godmode have no effect

#define DEFAULT_BULLET_HSPREAD	300
#define DEFAULT_BULLET_VSPREAD	500
#define DEFAULT_SHOTGUN_HSPREAD	1000
#define DEFAULT_SHOTGUN_VSPREAD	500
#define DEFAULT_DEATHMATCH_SHOTGUN_COUNT	12
#define DEFAULT_SHOTGUN_COUNT	12

//
// g_misc.c
//
void misc_vehicle_think( edict_t *vehicle );
void misc_vehicle_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
void misc_vehicle_pain (edict_t *self, edict_t *other, float kick, int damage);
void misc_vehicle_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//
// g_weapon.c
//
void ThrowDebris ( edict_t *self, char *modelname, float speed, vec3_t origin);
void fire_bullet ( edict_t *self, vec3_t start, vec3_t aimdir, int damage, int kick, int hspread, int vspread, int mod);
void fire_round  ( edict_t *self, vec3_t start, vec3_t aimdir );
void fire_rocket ( edict_t *self, vec3_t start, vec3_t dir );
void fire_flare  ( edict_t *self, vec3_t start, vec3_t dir );
void fire_bombs  ( edict_t *self, vec3_t start, vec3_t dir );
void fire_AAM    ( edict_t *self, vec3_t start, vec3_t dir );
void fire_ATGM( edict_t *self, vec3_t start, vec3_t dir );
void fire_LGB( edict_t *self, vec3_t start, vec3_t dir );
void nolockthink(edict_t *ent);
void bomb_think(edict_t *ent);

//
// g_spawn.c
//
void ED_CallSpawn (edict_t *ent);

//
// g_client.c
//
void respawn (edict_t *ent);
void BeginIntermission (edict_t *targ);
void PutClientInServer (edict_t *ent);
void InitClientPersistant (gclient_t *client);
void InitClientResp (gclient_t *client);
void InitBodyQue (void);
void ClientBeginServerFrame (edict_t *ent);

//
// g_svcmds.c
//
void	ServerCommand (void);
qboolean SV_FilterPacket (char *from);

//
// g_phys.c
//
void SV_AddGravity (edict_t *ent);

//
// p_view.c
//
void ClientEndServerFrame (edict_t *ent);

//
// p_hud.c
//
void MoveClientToIntermission (edict_t *client);
void G_SetStats (edict_t *ent);
void DeathmatchScoreboardMessage (edict_t *client, edict_t *killer);

//
// p_weapon.c
//
void Fire_Weapon( edict_t *ent );
void Weapon_Autocannon_Fire (edict_t *vehicle);
void Fire_Flare( edict_t *vehicle );
void AAM_Lock( edict_t *vehicle );
void ATGM_Lock( edict_t *vehicle );
void Autocannon_Lock( edict_t *vehicle );
void Reload_Jammer( edict_t *vehicle );
void Jammer_Active( edict_t *vehicle );
void UnlockEnemyOnDie( edict_t *vehicle );

// 
// p_client.c
//
qboolean ClientConnect (edict_t *ent, char *userinfo);
void ClientBegin (edict_t *ent);
edict_t *SelectDeathmatchSpawnPoint (void);

//
// g_phys.c
//
void G_RunEntity (edict_t *ent);

//
// g_main.c
//
void SaveClientData (void);
void FetchClientEntData (edict_t *ent);

//
// aq2_plane.c
//
void PlaneMovement( edict_t *plane );
void PlaneMovement_Easy( edict_t *plane );
void PlaneTouch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
void PlaneDie (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);
void PlaneSpeed( edict_t *plane );
void EmptyPlaneDie( edict_t *plane );

//
// aq2_helo.c
//
void HeloMovement( edict_t *helo );
void HeloMovement_Easy( edict_t *helo );
void HeloTouch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
void HeloDie (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//
// aq2_lqm.c
//
void SteerLQM_Alt( edict_t *vehicle);
void SteerLQM( edict_t *vehicle);
void LQM_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf );
void LQM_pain (edict_t *self, edict_t *other, float kick, int damage);
void LQM_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

//
// aq2_veh.c
//
void VehiclePain (edict_t *self, edict_t *other, float kick, int damage);
void SetVehicleModel (edict_t *vehicle );
void SelectVehicleAndSpawnIt( edict_t *vehicle, int which );
void VehicleExplosion (edict_t *vehicle);
//void SpawnVehicle( edict_t *ent );
void RemoveVehicle( edict_t *ent );
void MakeDamage( edict_t *targ );
void FindWaypoint( edict_t *wp, int type );
void VehicleSelectionNext( edict_t *ent );
void VehicleSelectionPrev( edict_t *ent );
void VehicleSelectionDisplay( edict_t *ent );
void VehicleInventory( edict_t *ent );
void SelectionStep( edict_t *ent );

//
// aq2_bots.c
//
void CheckBotNum();
void LoadBotsFromFile();

//
// aq2_cam.c
//
void RemoveCamera( edict_t *ent );
void GotoOwnVehicle( edict_t *ent );
void FindNextCamTarget( edict_t *ent );
void FindPrevCamTarget( edict_t *ent );
void SetCamera( edict_t *ent );
void FindCameraPoint( edict_t *ent, vec3_t origin, vec3_t angles );
void SetCamTarget( edict_t *ent, edict_t *target );

//
// aq2_cmds.c
//
void Cmd_Cameramode (edict_t *ent);
void Cmd_Increase (edict_t *ent);
void Cmd_Decrease (edict_t *ent);
void Cmd_Gear (edict_t *ent);
void Cmd_Brake (edict_t *ent);
void Cmd_Roll (edict_t *ent);
void Cmd_Coord (edict_t *ent);
void Cmd_Rollfix (edict_t *ent);
void Cmd_Tower (edict_t *ent );
void Cmd_Cam_x( edict_t *ent );
void Cmd_Cam_y( edict_t *ent );
void Cmd_Eject( edict_t *ent);
void Cmd_Zoom( edict_t *ent );
void Cmd_Botscores();
void Cmd_GPS( edict_t *ent );
void Cmd_Control( edict_t *ent );
void Cmd_Bay( edict_t *ent );
void Cmd_Weapon( edict_t *ent );
void Cmd_Flare( edict_t *ent );

//
// aq2_data.c
//
	// fighter planes
void Spawn_F16( edict_t *plane );
void Spawn_F14( edict_t *plane );
void Spawn_F4( edict_t *plane );
void Spawn_F22( edict_t *plane );
void Spawn_MIG21( edict_t *plane );
void Spawn_MIG29( edict_t *plane );
void Spawn_MIGMFI( edict_t *plane );
void Spawn_SU37( edict_t *plane );
	// attack planes
void Spawn_A10( edict_t *plane );
void Spawn_F117( edict_t *plane );
void Spawn_Tornado( edict_t *plane );
void Spawn_SU24( edict_t *plane );
void Spawn_SU25( edict_t *plane );
void Spawn_SU34( edict_t *plane );
	// attack helos
void Spawn_AH64( edict_t *plane );
void Spawn_MI28( edict_t *plane );
	// scout helos
void Spawn_RAH66( edict_t *plane );
void Spawn_KA50( edict_t *plane );
	// infantry
void Spawn_LQM (edict_t *vehicle );


//============================================================================

// client_t->anim_priority
#define	ANIM_BASIC		0		// stand / run
#define	ANIM_WAVE		1
#define	ANIM_JUMP		2
#define	ANIM_PAIN		3
#define	ANIM_ATTACK		4
#define	ANIM_DEATH		5
#define	ANIM_REVERSE	6


// client data that stays across multiple level loads
typedef struct
{
	char		userinfo[MAX_INFO_STRING];
	char		netname[16];
	int			hand;

	qboolean	connected;			// a loadgame will leave valid entities that
									// just don't have a connection yet

	// values saved and restored from edicts when changing levels
	int			health;
	int			max_health;
	int			savedFlags;

	int			game_helpchanged;
	int			helpchanged;

} client_persistant_t;

// client data that stays across deathmatch respawns
typedef struct
{
	client_persistant_t	coop_respawn;	// what to set client->pers to on a respawn
	int			enterframe;			// level.framenum the client entered the game
	int			score;				// frags, etc
	vec3_t		cmd_angles;			// angles sent over in the last command

} client_respawn_t;

// this structure is cleared on each PutClientInServer(),
// except for 'client->pers'
struct gclient_s
{
	// known to server
	player_state_t	ps;				// communicated by server to clients
	int				ping;

	// private to game
	client_persistant_t	pers;
	client_respawn_t	resp;
	pmove_state_t		old_pmove;	// for detecting out-of-pmove changes

	qboolean	showscores;			// set layout stat
	qboolean	showinventory;		// set layout stat
	qboolean	showhelp;
	qboolean	selectvehicle;

	int			buttons;
	int			oldbuttons;
	int			latched_buttons;

	vec3_t		v_angle;			// aiming direction
	vec3_t		oldviewangles;
	vec3_t		oldvelocity;

	int			old_waterlevel;

	int			machinegun_shots;	// for weapon raising

	float		flood_locktill;		// locked from talking
	float		flood_when[10];		// when messages were said
	int			flood_whenhead;		// head pointer for when said

	float		respawn_time;		// can respawn when time > this

	// NewAirQuake2
	int			fov;
	int			cameramode;			
	int			cameradistance;
	int			cameraheight;
	int			REPEAT;
	edict_t		*cameratarget;
	vec3_t		cameraposition;
	int			FLAGS;
	float		thrusttime;
	int			sel_step;			// 0 = team, 1 = cat, 2 = class, 3 = veh
	int			team, cat, 
				cls, veh;
	edict_t		*nextcam, *prevcam;	// for cameras, to find out all that look at one target
};


struct edict_s
{
	entity_state_t	s;
	struct gclient_s	*client;	// NULL if not a player
									// the server expects the first part
									// of gclient_s to be a player_state_t
									// but the rest of it is opaque

	qboolean	inuse;
	int			linkcount;

	// FIXME: move these fields to a server private sv_entity_t
	link_t		area;				// linked to a division node or leaf
	
	int			num_clusters;		// if -1, use headnode instead
	int			clusternums[MAX_ENT_CLUSTERS];
	int			headnode;			// unused if num_clusters != -1
	int			areanum, areanum2;

	//================================

	int			svflags;
	vec3_t		mins, maxs;
	vec3_t		absmin, absmax, size;
	solid_t		solid;
	int			clipmask;
	edict_t		*owner;


	// DO NOT MODIFY ANYTHING ABOVE THIS, THE SERVER
	// EXPECTS THE FIELDS IN THAT ORDER!

	//================================
	int			movetype;
	int			flags;

	char		*model;
	float		freetime;			// sv.time when the object was freed
	
	//
	// only used locally in game, not by server
	//
	char		*message;
	char		*classname;
	int			spawnflags;

	float		timestamp;

	float		angle;			// set in qe3, -1 = up, -2 = down
	char		*target;
	char		*targetname;
	char		*killtarget;
	char		*team;
	char		*pathtarget;
	char		*deathtarget;
	char		*combattarget;
	edict_t		*target_ent;

	float		speed, accel, decel;
	vec3_t		movedir;
	vec3_t		pos1, pos2;

	vec3_t		velocity;
	vec3_t		avelocity;
	int			mass;
	float		gravity;		// per entity gravity multiplier (1.0 is normal)
								// use for lowgrav artifact, flares

	edict_t		*goalentity;
	edict_t		*movetarget;
	float		ideal_yaw;

	float		nextthink;
	void		(*prethink) (edict_t *ent);
	void		(*think)(edict_t *self);
	void		(*blocked)(edict_t *self, edict_t *other);	//move to moveinfo?
	void		(*touch)(edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf);
	void		(*use)(edict_t *self, edict_t *other, edict_t *activator);
	void		(*pain)(edict_t *self, edict_t *other, float kick, int damage);
	void		(*die)(edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point);

	float		touch_debounce_time;		// are all these legit?  do we need more/less of them?

	int			health;
	int			max_health;
	int			gib_health;
	int			deadflag;

	char		*map;			// target_changelevel

	int			takedamage;
	int			dmg;
	int			radius_dmg;
	float		dmg_radius;		// also abused for the side a weapons is launched from
	int			sounds;			//make this a spawntemp var?
	int			count;

	edict_t		*chain;
	edict_t		*enemy;
	edict_t		*oldenemy;
	edict_t		*activator;
	edict_t		*groundentity;
	int			groundentity_linkcount;
	edict_t		*teamchain;
	edict_t		*teammaster;

	edict_t		*mynoise;		// can go in client only
	edict_t		*mynoise2;

	int			noise_index;	
	float		volume;			// also abused for flare launch delay!
	float		attenuation;	// also abused for secondary weapon launch delay!

	// timing variables
	float		wait;			// used for LQM/ground vehicle drowning and helo/plane baydoor times
	float		delay;			// before firing targets
	float		random;			// also abused for vehicle sound

	int			watertype;
	int			waterlevel;

	vec3_t		move_origin;
	vec3_t		move_angles;

	int			style;			// also used as areaportal number

	gitem_t		*item;			// for bonus items

	// common data blocks
	moveinfo_t		moveinfo;
	monsterinfo_t	monsterinfo;

	// New AirQuake2 data
	edict_t		*vehicle;		// your vehicle
	edict_t		*camera;		// who watches this vehicle ?
	edict_t		*radio_target;	// for trigger_radio
	int			aqteam;
	int			rudder;
	int			elevator;
	int			aileron;
	int			altitude;
	vec3_t		turnspeed;
	int			thrust;
	int			maxthrust;
	int			minthrust;
	int			stallspeed;
	int			topspeed;
	long		ONOFF;
	long		HASCAN;
	long		DAMAGE;
	float		recharge_time;
	guntypes_t	*guntype;
	float		drone_decision_time;	// also for LQM chute times
	int			botnumber;
	int			action;
	float		distance;
	int			lockstatus;
	float		autocannon_time;
	int			ammo[MAX_WEAPONS];
	int			maxammo[MAX_WEAPONS];
	int			weapon[MAX_WEAPONS];
	int			weaponicon[MAX_WEAPONS];
	int			weaponicon_dam[MAX_WEAPONS];
	char		weaponname[MAX_WEAPONS][20];
	int			active_weapon;
	int			flares, maxflares;
	float		jam_time;
	float		verticalspeed;	// for helos
};

//------------------------------//
//     NewAirQuake2 defines	    //
//------------------------------//

// Technical
#define MAXTHRUST							15
#define MINTHRUST							-5
#define THRUST_MILITARY 					10
#define THRUST_HOVER						 5
#define THRUST_IDLE							0
// repeatable keys
#define REPEAT_FREELOOK						 1
#define REPEAT_INCREASE						 2
#define REPEAT_DECREASE						 4
#define REPEAT_BRAKE						 8
#define REPEAT_FIRE							16
// Number of Cameramodes
#define CM_NUM_MODES						18
// Cameramodes
#define CM_HULL_FORWARD						0
#define CM_HULL_BACK						 1
#define CM_HULL_LEFT						 2
#define CM_HULL_RIGHT						 3
#define CM_HULL_UP							 4
#define CM_HULL_DOWN						 5
#define CM_TURRET_FORWARD					 6
#define CM_TURRET_BACK						 7
#define CM_TURRET_LEFT						 8
#define CM_TURRET_RIGHT						 9
#define CM_TURRET_UP						10
#define CM_TURRET_DOWN						11
#define CM_MISSILE_FORWARD					12
#define CM_MISSILE_BACK						13
#define CM_MISSILE_LEFT						14
#define CM_MISSILE_RIGHT					15
#define CM_MISSILE_UP						16
#define CM_MISSILE_DOWN						17
// Camera
#define MAX_CAMERADISTANCE					-200
#define MAX_CAMERAHEIGHT					50
// ONOFF FLAGS
#define ONOFF_NOTHING_ON			 		 0
#define ONOFF_PILOT_ONBOARD					 1
#define ONOFF_GEAR							 2
#define ONOFF_AIRBORNE						 4
#define ONOFF_WEAPONBAY						 8
#define ONOFF_BRAKE							16
#define ONOFF_STALLED						32
#define ONOFF_LANDED						64
#define ONOFF_LANDED_NOGEAR					128
#define ONOFF_LANDED_TERRAIN				256
#define ONOFF_LANDED_TERRAIN_NOGEAR 		512
#define ONOFF_LANDED_WATER					1024
#define ONOFF_LANDED_WATER_NOGEAR			2048
#define ONOFF_SINKING						4096
#define ONOFF_IS_CHANGEABLE					8192
#define ONOFF_AUTOROTATION					16384
#define ONOFF_AUTOPILOT						32768
#define ONOFF_GPS							65536
#define ONOFF_JAMMER						131072
#define ONOFF_STEALTH						262144
// HASCAN FLAGS
#define HASCAN_NOTHING						 0
#define HASCAN_BRAKE						 1
#define HASCAN_GEAR							 2
#define HASCAN_WEAPONBAY					 4
#define HASCAN_TAILDRAGGER					 8
#define HASCAN_TERRAINLANDING				16
#define HASCAN_AUTOAIM						32
#define HASCAN_DRONE						64
#define HASCAN_SCORED						128		// score already counted ?
#define HASCAN_SCORED_DEAD					256
#define HASCAN_AUTOPILOT					512
#define HASCAN_EASTERN						1024
#define HASCAN_CARRIER						2048
#define HASCAN_ROUNDBODY					4096	// rolls after crash
#define HASCAN_STEALTH						8192
// DAMAGE
#define DAMAGE_NONE							 0
#define DAMAGE_GEAR							 1
#define DAMAGE_BRAKE						 2
#define DAMAGE_WEAPONBAY					 4
#define DAMAGE_AFTERBURNER					 8
#define DAMAGE_ENGINE_80					16
#define DAMAGE_ENGINE_60					32
#define DAMAGE_ENGINE_40					64
#define DAMAGE_ENGINE_20					128
#define DAMAGE_ENGINE_DESTROYED				256
#define DAMAGE_AUTOPILOT					512
#define DAMAGE_GPS							1024
#define DAMAGE_WEAPON_0						2048
#define DAMAGE_WEAPON_1						4096
#define DAMAGE_WEAPON_2						8192
#define DAMAGE_WEAPON_3						16384
#define DAMAGE_WEAPON_4						32768
#define DAMAGE_FLARE						65536
// client FLAGS
#define CLIENT_NOFLAGS						0
#define CLIENT_AUTOROLL						1
#define CLIENT_COORD						2
#define CLIENT_VEH_SELECTED					4
#define CLIENT_NONGL_MODE					8
#define CLIENT_NO_REPEATBRAKE				16
#define CLIENT_ALTCONTROLS_PLANE			32
#define CLIENT_ALTCONTROLS_HELO				64
#define CLIENT_ALTCONTROLS_GROUND			128
#define CLIENT_ALTCONTROLS_LQM				256
// weapons flags
#define WEAPON_NONE							0
#define WEAPON_AUTOCANNON					1	// bullets
#define WEAPON_FFAR							2	// rockets
#define WEAPON_SIDEWINDER					3	// sidewinder
#define WEAPON_STINGER						4	// sidewinder
#define WEAPON_AMRAAM						5	// amraam
#define WEAPON_PHOENIX						6	// amraam
#define WEAPON_IRONBOMBS					7	// bombs
#define WEAPON_LASERBOMBS					8	// bombs
#define WEAPON_RUNWAYBOMBS					9	// smart weapon
#define WEAPON_HELLFIRE						10	// agm	
#define WEAPON_MAVERICK						11	// agm
#define WEAPON_ANTIRADAR					12	// smart weapon
#define WEAPON_MAINGUN						13	// shells
#define WEAPON_ARTILLERY					14	// shells
#define WEAPON_JAMMER						15	// NO AMMO
#define WEAPON_HORN							16	// for not armed vehicles
// modifier
#define JAMMER_EFFECT  						  50
#define STANDARD_RANGE						1400
#define HELLFIRE_MODIFIER					 1.0
#define SIDEWINDER_MODIFIER					 1.0
#define STINGER_MODIFIER					 1.0
#define AMRAAM_MODIFIER						 1.7
#define PHOENIX_MODIFIER					 2.1
#define MAVERICK_MODIFIER					 1.3
#define LGB_MODIFIER						 0.8
#define AUTOCANNON_MODIFIER					 0.66
#define JAMMER_MODIFIER						 1.5
#define ANTIRADAR_MODIFIER					 1.5
#define RUNWAYBOMB_MODIFIER					 0.8
// bots action
#define ACTION_NONE								0
#define ACTION_AVOID_TERRAIN					1
#define ACTION_AVOID_TERRAIN_FORCED				2
#define ACTION_AVOID_TERRAIN_VERTICAL			4
#define ACTION_AVOID_TERRAIN_VERTICAL_FORCED	8
#define ACTION_AVOID_STALL						16
#define ACTION_RANDOM							32
#define ACTION_CHASE_TARGET						64
#define ACTION_HIT_BY_SOMEONE					128
#define ACTION_HIT_BY_SOMEONE_FORCED			256
	// all terrain stuff
#define ACTION_TERRAIN							15

// other
#define GRAVITY									800		// instead of sv_gravity
// lock
#define LS_NOLOCK								0
#define LS_LOCK									1
#define LS_LAUNCH								2
// targettype
#define WP_ENEMY_BASE							1
#define WP_ENEMY_STRUCTURE						2
#define WP_OWN_BASE								4
#define WP_OWN_STRUCTURE						8
// vehicle selection
#define SEL_TEAM								0
#define SEL_CAT									1
#define SEL_CLS									2
#define SEL_VEH									3
// categories
#define CAT_PLANE								1
#define CAT_HELO								2
#define CAT_GROUND								4
#define CAT_LQM									8
#define CAT_ALL									(CAT_PLANE|CAT_HELO|CAT_GROUND|CAT_LQM)
#define CAT_AIR									(CAT_PLANE|CAT_HELO)
#define CAT_NOTAIR								(CAT_GROUND|CAT_LQM)
#define CAT_TOO_HIGH							16
// classes
#define CLS_PLANE_FIGHTER						1
#define CLS_PLANE_ATTACK						2
#define CLS_PLANE_TRANSPORT						3
#define CLS_HELO_ATTACK							4
#define CLS_HELO_SCOUT							5
#define CLS_HELO_TRANSPORT						6
#define CLS_GROUND_MBT							7
#define CLS_GROUND_APC							8
#define CLS_GROUND_SCOUT						9
#define CLS_GROUND_SPECIAL						10
#define CLS_LQM_PILOT							11
#define CLS_LQM_INFANTRY						12
// number of available vehicles
#define PLANE_ATTACK							6
#define PLANE_FIGHTER							8
#define PLANE_TRANSPORT							0
#define HELO_ATTACK								2
#define HELO_SCOUT								2
#define HELO_TRANSPORT							0
#define GROUND_MBT								0
#define GROUND_APC								0
#define GROUND_SCOUT							0
#define GROUND_SPECIAL							0
#define LQM_PILOT								0
#define LQM_INFANTRY							0
// guns
#define GUN_VULCAN								0	// very fast, not too strong
#define GUN_VULCAN_DUAL							1	// same, but two barrels
#define GUN_MAUSER								2	// not so fast, stronger
#define GUN_GAU									3	// not so fast, STRONG!
#define GUN_HEAVY								4	// slow, strong
#define GUN_LIGHT_RIFLE							5	// not so fast, weak
#define GUN_RIFLE								6	// not so fast, slightly stronger
// 3 letter vehicle codes
	// plane-fighter
#define CODE_F14								111
#define CODE_F22								112
#define CODE_F16								113	
#define CODE_F4									114
#define CODE_SU37								115
#define CODE_MIG21								116
#define CODE_MIG29								117
#define CODE_MIGMFI								118
	// plane-attack
#define CODE_TORNADO							121	
#define CODE_A10								122
#define CODE_F117								123
#define CODE_SU24								124
#define CODE_SU25								125
#define CODE_SU34								126
	// helo-attack							
#define CODE_AH64								211
#define CODE_MI28								212
	// helo-scout
#define CODE_RAH66								221
#define CODE_KA50								222



