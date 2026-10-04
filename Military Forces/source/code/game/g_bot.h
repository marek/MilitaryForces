#ifndef __G_BOT_H__
#define __G_BOT_H__

#include "q_shared.h"
#include <array>
#include <vector>

struct GameEntity;
struct GameClient;

class Bots
{
public:
	static void reset();
	static void runFrame();
	static int add( int vehicle, const char* team, const char* name );
	static void remove( int clientNum );
	static void removeAll();
	static void onConnect( int clientNum );
	static void onDisconnect( int clientNum );
	static bool isBot( int clientNum );
	static int count();

	static void addCommand();
	static void kickCommand();
	static void listCommand();

	enum class Role { Fighter, Bomber, AttackHelo, TransportHelo, Tank, AirDefence, Boat, Infantry };
	enum class Goal { Roam, Attack, Evade, Climb, Hide, Rearm };

	struct Character
	{
		const char*	name;
		float		aggression;
		float		caution;
		float		patience;
		float		teamwork;
		float		aim;
		float		honour;
		float		cunning;
	};

private:
	struct State
	{
		bool	active = false;
		int		vehicle = -1;
		int		enemy = -1;
		int		enemyTime = 0;
		vec3_t	home = { 0, 0, 0 };
		bool	hasHome = false;
		vec3_t	goal = { 0, 0, 0 };
		int		goalTime = 0;
		vec3_t	lastOrigin = { 0, 0, 0 };
		int		stuckCheckTime = 0;
		int		reverseTime = 0;
		int		reverseTurn = 0;
		int		egressTime = 0;
		int		spawnTime = 0;
		int		commandTime = 0;
		int		avoidTime = 0;
		float	avoidYaw = 0;
		float	aimPhase = 0;
		bool	wasAlive = false;
		int		character = 0;
		Goal	desire = Goal::Roam;
		int		goalEvalTime = 0;
		int		commitTime = 0;
		int		lastHealth = 0;
		int		hitTime = 0;
		float	breakYaw = 0;
		int		structure = -1;
		int		structureTime = 0;
		vec3_t	hidePoint = { 0, 0, 0 };
		int		hideTime = 0;
		bool	hasHide = false;
		int		popTime = 0;
		bool	exposed = false;
		int		flareTime = 0;
		int		bombTime = 0;
		int		pad = -1;
		int		landPhase = 0;
		float	landDir = 1;
		float	takeoffYaw = 0;
		int		takeoffTime = 0;
	};

	struct Pad
	{
		vec3_t			center;
		vec3_t			spot;
		vec3_t			axis;
		float			halfLength;
		float			z;
		bool			runway;
		unsigned int	category;
		int				team;
	};

	struct Sense
	{
		GameEntity*	enemy = nullptr;
		float		enemyDist = 0;
		vec3_t		enemyDir = { 0, 0, 0 };
		vec3_t		enemyAngles = { 0, 0, 0 };
		bool		enemyAir = false;
		float		height = 0;
		bool		enemyVisible = false;
		bool		locked = false;
		bool		launched = false;
		float		health = 1;
		GameEntity*	airThreat = nullptr;
		float		airThreatDist = 0;
		Role		role = Role::Fighter;
		bool		stealth = false;
		bool		revealed = false;
		bool		underFire = false;
		bool		blindsided = false;
		bool		duel = false;
	};

	static void think( GameEntity* ent, State& bot, usercmd_t& cmd );
	static void thinkPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void thinkHelo( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void thinkSurface( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void thinkInfantry( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void thinkRider( GameEntity* ent, State& bot, usercmd_t& cmd, vec3_t view );

	static GameEntity* findEnemy( GameEntity* ent, State& bot, Role role );
	static Role roleOf( int vehicle );
	static Goal chooseGoal( GameEntity* ent, State& bot, const Sense& sense );
	static const Character& character( const State& bot );
	static float skill( const State& bot );
	static bool visible( GameEntity* ent, const vec3_t from, GameEntity* target );
	static bool hasWeaponType( int vehicle, const playerState_t& ps, int type );
	static bool findHidePoint( GameEntity* ent, State& bot, GameEntity* threat );
	static void bomb( GameEntity* ent, State& bot, const vec3_t target, usercmd_t& cmd, vec3_t view );
	static GameEntity* findStructure( GameEntity* ent, State& bot );
	static void flares( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd );
	static float ceilingAbove( const vec3_t origin, int passEntity );
	static void buildPads();
	static void buildRoamPoints();
	static bool steerAround( GameEntity* ent, float wantYaw, float travel, float& yaw );
	static bool runwayAt( const vec3_t point );
	static float runwayRun( const vec3_t origin, float yaw, float maxDist );
	static void runwayHeading( GameEntity* ent, State& bot );
	static int findPad( GameEntity* ent );
	static bool replenished( GameEntity* ent );
	static void finishRearm( GameEntity* ent, State& bot );
	static bool landPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void sense( GameEntity* ent, State& bot, Sense& out );
	static void pickGoal( GameEntity* ent, State& bot, float radius );
	static void fireWeapons( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, float aimError );
	static int findWeaponSlot( int vehicle, const playerState_t& ps, bool air, bool& guided );
	static void setRadar( GameEntity* ent, State& bot, bool air );
	static float heightAboveGround( const vec3_t origin, int passEntity );
	static bool pathBlocked( const vec3_t origin, const vec3_t dir, float dist, int passEntity, trace_t& tr );
	static int pickVehicle( int teamFlag, bool allowInfantry = true );
	static unsigned int spawnCategories();
	static void fill();
	static void select( GameEntity* ent, State& bot );

	static std::array<State, MAX_CLIENTS + 1> states_;
	static int fillTime_;
	static std::vector<Pad> pads_;
	static bool padsBuilt_;
	static std::vector<Vec3> roamPoints_;
};

#endif
