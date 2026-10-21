#ifndef __BOT_H__
#define __BOT_H__

#include "../q_shared.h"
#include <array>
#include <vector>
#include "behavior/behavior.h"
#include "goals/capabilities.h"
#include "core/skill.h"
#include "core/mistakes.h"

struct GameEntity;
struct GameClient;

class Bots
{
public:
	// the game's side: a map starts, a frame runs, bots are added and removed, a bot dies
	static void reset();
	static void runFrame();
	static int add( int vehicle, const char* team, const char* name );
	static void remove( int clientNum );
	static void removeAll();
	static void onConnect( int clientNum );
	static void onDisconnect( int clientNum );
	static bool isBot( int clientNum );
	static int count();
	static void onDeath( GameEntity* self, GameEntity* attacker, int meansOfDeath );
	// someone fired: the bots that hear it are suspicious of the shooter
	static void onShot( GameEntity* shooter );

	// console commands
	static void addCommand();
	static void kickCommand();
	static void listCommand();
	static void treeCommand();

	// what a vehicle is for, and what a bot is trying to do at the moment
	enum class Role { Fighter, Bomber, AttackHelo, TransportHelo, Tank, AirDefence, Boat, Infantry };
	enum class Goal { Roam, Attack, Evade, Climb, Hide, Rearm, Capture };

	// who a bot is: each trait from 0 to 1, read from the character file, and how much it likes each kind of vehicle
	struct Character
	{
		char		name[32];
		float		aggression;
		float		caution;
		float		patience;
		float		teamwork;
		float		aim;
		float		honour;
		float		cunning;
		float		prefer[5];
		float		reaction;
	};

	// how far a bot has noticed another player: not at all, something that gives it away, or seen and watched
	enum class Awareness { Unaware, Suspicious, Aware };

	// where and when another player was last seen, and how aware of it the bot is
	struct Seen
	{
		vec3_t		position = { 0, 0, 0 };
		int			time = 0;
		Awareness	level = Awareness::Unaware;
		int			awareTime = 0;	// when it was last seen while watched
	};

	// one bot's thinking for one frame (core/mind.h)
	struct Mind;

	// everything one bot remembers from frame to frame. Times are level times in milliseconds
	// going back to a pad for fuel and arms
	struct Rearm
	{
		int		pad = -1;	// the pad it is going to for fuel and arms
		int		phase = 0;	// survey, approach, final, on the ground
		int		badPad = -1;	// a pad given up on
		int		badPadTime = 0;	// and until when it is avoided

		void reset() { *this = {}; }
		// done with the pad it was going to, keeping in mind the one given up on
		void letGo()
		{
			pad = -1;
			phase = 0;
		}
	};

	// a plane's landing: the runway, the way down, and whether this map has anywhere to land
	struct Landing
	{
		float	dir = 1;	// which way along the runway it lands
		float	run = 3500;	// how long a run-in the terrain allows
		float	slope = 0.15f;	// the slope of the way down the terrain allows
		bool	outbound = false;	// flying away from the runway to turn round
		float	ceiling = 800;	// room above the runway
		float	offset = 45;	// the lane beside the centre line it lands on
		float	approachHeight = 0;	// the height to fly on the approach
		int		gearTime = 0;	// no further gear command before then
		int		padCheckTime = 0;	// when to look again for a pad this plane can land on
		bool	canLand = true;	// this map has a pad it can land on
		int		start = 0;	// when this landing was begun
		int		giveUp = 0;	// no landing attempt before then

		void reset() { *this = {}; }
	};

	// a plane's take-off: the plan, taxiing to a spot, holding short, giving way
	struct Takeoff
	{
		int		groundTime = 0;	// when it was last on the ground
		float	yaw = 0;	// the heading planned for the take-off roll
		float	departYaw = 0;	// and for the climb out
		int		planTime = 0;	// the take-off plan holds until then
		int		taxiTime = 0;	// taxiing to a better spot until then
		int		holdTime = 0;	// holding short until then
		int		heldSince = 0;	// waiting on the ground for traffic since then, 0 when not
		int		holds = 0;	// holds in a row for a run that is short or blocked by the world, so it does not wait for ever
		vec3_t	taxiSpot = { 0, 0, 0 };	// where it taxis to before rolling

		void reset() { *this = {}; }
	};

	// hiding: where, until when, and a helicopter coming up out of it to fire
	struct Cover
	{
		vec3_t	point = { 0, 0, 0 };	// where it hides
		int		time = 0;	// stays in cover, or keeps its hiding place, until then
		bool	has = false;	// it has a hiding place
		int		popTime = 0;	// a helicopter up out of cover goes back down then
		vec3_t	lastPop = { 0, 0, 0 };	// where a stealth helicopter last came up to fire: not there again
		int		strikeAmmo = 0;	// what it carried when it came up, to tell when a missile has gone
		bool	breakingOff = false;	// getting out of a fight at close quarters
		bool	exposed = false;	// up out of cover and shooting

		void reset() { *this = {}; }
	};

	// chasing an enemy: the turn chosen towards it, and flying on out after a pass
	struct Pursuit
	{
		int		egressTime = 0;	// flying on out after a pass until then
		int		chaseTime = 0;	// a chosen turn is flown out until then
		float	turn = 0;	// the turn chosen towards the enemy
		float	pitch = 0;	// and its climb or dive
		Vec3	runIn;	// where a bomb run starts from
		int		runInTime = 0;	// chosen afresh after then

		void reset() { *this = {}; }
	};

	// the manoeuvre planner: the heading it holds, and what it found looking ahead
	struct Avoidance
	{
		int		time = 0;	// the manoeuvre planner's heading holds until then
		float	yaw = 0;
		float	pitch = 0;
		int		probeTime = 0;	// when the manoeuvre planner next looks ahead
		bool	probeBlocked = false;
		int		brakeTime = 0;	// slowing for a tight turn until then
		float	probeWant = 0;	// free distance on the wanted line, at the last look ahead
		float	probeBest = 0;	// and on the line chosen
		bool	confined = false;	// in terrain tighter than its turn

		void reset() { *this = {}; }
	};

	// a path round an obstacle, from the path finder
	struct Path
	{
		std::vector<Vec3>	points;
		size_t	index = 0;
		int		searchTime = 0;	// when to search for a path again
		int		skipTime = 0;	// when to look again whether the next waypoint can be gone to directly
		bool	following = false;	// flying or driving that path now
		bool	cornerAhead = false;	// the path turns sharply soon

		void reset() { *this = {}; }
	};

	// noticing it has not moved, and backing out
	struct Stuck
	{
		vec3_t	lastOrigin = { 0, 0, 0 };	// where it was at the last stuck check
		int		checkTime = 0;
		int		reverseTime = 0;	// backing out of a blocked spot until then
		int		turn = 0;	// and steering this way meanwhile

		void reset() { *this = {}; }
	};

	struct State
	{
		bool	active = false;	// this client is a bot
		int		vehicle = -1;	// the vehicle it has chosen to fly or drive
		int		enemy = -1;	// client number of the enemy it is fighting
		int		enemyTime = 0;	// when to look for an enemy again
		vec3_t	home = { 0, 0, 0 };	// where it first appeared
		bool	hasHome = false;
		vec3_t	goal = { 0, 0, 0 };	// where it is roaming to
		int		goalTime = 0;	// when that goal goes stale
		int		spawnTime = 0;	// when this life began
		int		commandTime = 0;	// no further client command (enter, radar, respawn) before then
		float	aimPhase = 0;	// where the aim wander is in its swing
		Vec3	handView;	// the view the bot last sent, and when: its hand turns from there
		int		handTime = 0;
		bool	wasAlive = false;	// alive in the last frame: a change marks a life's start or end
		int		character = 0;	// index of its character
		Goal	desire = Goal::Roam;	// the goal it is pursuing
		int		goalEvalTime = 0;	// when the goal is next reviewed
		int		commitTime = 0;	// a goal just taken is kept until then
		int		lastHealth = 0;
		int		hitTime = 0;	// when it was last hit
		float	breakYaw = 0;	// the heading to break to when hit by someone unseen
		int		structure = -1;	// the building chosen to bomb; for a supply helicopter, who was last supplied
		int		structureTime = 0;
		int		lifeScore = 0;	// its score when this life began
		int		badLives = 0;	// lives in a row without scoring
		std::array<int, 5> ownLives = {};	// lives per kind of vehicle
		std::array<int, 5> ownFailures = {};	// and how many of them scored nothing
		int		goalsReached = 0;
		int		goalsExpired = 0;
		int		leader = -1;	// client number of the leader it follows
		int		leaderTime = 0;
		bool	teamPlayer = false;	// this life is played alongside a leader
		int		stillSince = 0;	// riding as a passenger: since when the transport has stood still
		// a lift offered to a team mate on foot: to whom and since when, waiting beside them since when, last held when;
		// whom it last passed over, left alone until then (kept across lives)
		struct Lift
		{
			int		walker = -1;
			int		since = 0;
			int		besideSince = 0;
			int		heldAt = 0;
			int		passedOver = -1;
			int		passedOverUntil = 0;
		}		lift;
		vec3_t	trail[6] = {};	// where it was in each of the last six seconds
		int		trailTime = 0;
		int		logTime = 0;
		std::array<Seen, MAX_CLIENTS + 1> seen;	// where each other player was last seen
		int		flareTime = 0;
		int		bombTime = 0;	// when a bomb or a crate was last released
		int		reactTime = 0;	// it may shoot at a new enemy from then
		int		reactEnemy = -1;
		float	breakSide = 1;	// the side to break to from a missile
		// the parts each behaviour owns
		Rearm	rearm;
		Landing	landing;
		Takeoff	takeoff;
		Cover	cover;
		Pursuit	pursuit;
		Avoidance	avoid;
		Path	path;
		Stuck	stuck;
		behavior::Memory tree;	// what the behaviour tree remembers between frames
		const behavior::Tree<Mind>* treeUsed = nullptr;	// the tree that memory belongs to
		int		turretTime = 0;		// start of the current turret burst, or of the pause after it
	};


	// what a bot perceives in one frame; worked out afresh each time
	struct Sense
	{
		GameEntity*	enemy = nullptr;
		float		enemyDist = 0;	// to the point to aim at, which leads a moving enemy
		vec3_t		enemyDir = { 0, 0, 0 };
		vec3_t		enemyAngles = { 0, 0, 0 };
		bool	enemyAir = false;	// the enemy is an aircraft in the air
		float		height = 0;	// above the ground or water
		bool	enemyVisible = false;
		bool	locked = false;	// someone has a lock on it, or a missile is on its way
		bool	launched = false;	// a missile is on its way
		float		health = 1;	// as a share of full health
		GameEntity*	airThreat = nullptr;	// the nearest enemy aircraft in the air
		float		airThreatDist = 0;
		Role		role = Role::Fighter;
		Capabilities	can;	// what its vehicle can do
		bool	stealth = false;	// flying a stealth aircraft
		bool	revealed = false;	// and it is showing on radar at the moment
		bool	underFire = false;	// hit within the last seconds
		bool	blindsided = false;	// and by someone it is not looking at
		GameEntity*	missile = nullptr;	// the missile that will arrive first
		float		missileDist = 0;
		float		missileTime = 0;	// seconds until it does
		float		missileYaw = 0;	// the direction it comes from
		bool	duel = false;	// one enemy aircraft and nobody else near
		bool	outranged = false;	// the enemy's weapons reach it here and its own do not reach back
	};

	friend struct LandingSteps;
	friend struct PlaneTasks;
	friend struct HeloTasks;
	friend struct SurfaceTasks;
	friend struct InfantryTasks;
	friend struct GoalScoring;
	friend struct Crew;
	friend class BotWeapons;
	friend class BotSenses;
	friend class BotTeam;
	friend class BotOrders;

private:
	// one bot's frame, and the pilot or driver for the kind of vehicle it is in
	static void think( GameEntity* ent, State& bot, usercmd_t& cmd );
	static bool settleLife( GameEntity* ent, State& bot );
	static void startLife( GameEntity* ent, State& bot );
	static void turnHand( State& bot, vec3_t view );
	static void chaseFlagThief( GameEntity* ent, State& bot );
	static void followPlan( GameEntity* ent, State& bot );
	static void keepPlaneFuel( GameEntity* ent, State& bot );
	static void pilot( GameEntity* ent, State& bot, usercmd_t& cmd, vec3_t view );
	static void thinkPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void thinkHelo( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void thinkSurface( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void thinkInfantry( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view );
	static void thinkRider( GameEntity* ent, State& bot, usercmd_t& cmd, vec3_t view );
	static bool leaveStoppedRide( GameEntity* ent, State& bot, GameEntity* pilot );
	static GameEntity* nearestFromSeat( GameEntity* ent, GameEntity* pilot, float reach );

	// senses, weapons, world knowledge and vehicle choice; each is described where it is defined
	static Goal chooseGoal( GameEntity* ent, State& bot, const Sense& sense );
	static const Character& character( const State& bot );
	static float skill( const State& bot );
	static Handicap handicap( const State& bot );
	static Mistakes mistakes( const State& bot );
	static int memoryTime( const State& bot );
	static bool findHidePoint( GameEntity* ent, State& bot, GameEntity* threat, bool hullDown = false );
	static GameEntity* findStructure( GameEntity* ent, State& bot );
	static bool standing( GameEntity* structure );
	static float structureScore( GameEntity* ent, GameEntity* structure, int team );
	static void loadCharacters();
	static bool replenished( GameEntity* ent );
	static void finishRearm( GameEntity* ent, State& bot );
	static bool runwayBusy( int padIndex, GameEntity* self );
	static bool teamHasSupplier( int team, GameEntity* except = nullptr );
	static bool turnClear( GameEntity* ent, float radius, float degrees, float climb = 0 );
	static void logDeath( GameEntity* self, GameEntity* attacker, int meansOfDeath );
	static void logState( GameEntity* ent, State& bot, const usercmd_t& cmd, const vec3_t view );
	static void pickGoal( GameEntity* ent, State& bot, float radius );
	static GameEntity* nearestQuarry( GameEntity* ent, unsigned int cat );
	static void goalNear( GameEntity* ent, State& bot, GameEntity* quarry, unsigned int cat );
	static void goalAtRoamPoint( GameEntity* ent, State& bot, const std::vector<Vec3>& points, bool air );
	static bool goalAtHint( GameEntity* ent, State& bot, unsigned int cat );
	static void goalAroundHome( GameEntity* ent, State& bot, float radius, unsigned int cat );
	static int pickVehicle( int teamFlag, bool allowInfantry = true, const Character* who = nullptr, const State* memory = nullptr, int teamKind = -1 );
	static float failureSensitivity( const Character& who );
	// what a bot choosing a vehicle goes by
	struct Choice
	{
		unsigned int		cats = 0;
		int					teamFlag = 0;
		bool				allowInfantry = true;
		const Character*	who = nullptr;
		const State*		memory = nullptr;
		int					teamKind = -1;
		bool				teamHasSupplier = true;
	};
	static unsigned int allowedCategories();
	static int vehicleWeight( int vehicle, const Choice& choice );
	static bool carriesNuke( int vehicle );
	static float switchDesire( const State& bot, int kind );
	static unsigned int spawnCategories();
	static void fill();
	static void select( GameEntity* ent, State& bot );

	// one state per client slot; the map's pads, roam points and characters; lives and early crashes per kind of vehicle
	static std::array<State, MAX_CLIENTS + 1> states_;
	static int fillTime_;
	static std::array<int, 5> kindLives_;
	static std::array<int, 5> kindCrashes_;
	// the same per vehicle type: one aircraft may not fit a map its kind otherwise copes with
	static std::array<int, MAX_VEHICLE_TYPES> typeLives_;
	static std::array<int, MAX_VEHICLE_TYPES> typeCrashes_;
	static bool unfitHere( int type );
	static int kindOf( int vehicle );
	static std::vector<Character> characters_;
};

// What bots know of the map, the same for all of them: where vehicles refuel and which runway leads there, what is
// runway, what blocks a line or a body, how much room there is above and below, where the flags stand, and places
// worth roaming to. Nothing here depends on which bot asks.
class BotWorld
{
public:
	// a place to refuel and rearm, with the runway that leads to it when it has one
	struct Pad
	{
		Vec3			center;	// middle of the runway; of the pad itself when there is none
		Vec3			spot;	// where to stop to be refuelled
		Vec3			axis;	// direction of the runway
		float			halfLength;	// half the runway's length, 0 without one
		float			z;	// height of the surface
		bool		runway;	// long enough for a plane to land on
		unsigned int	category;	// the kinds of vehicle it serves
		int				team;	// whose it is in a team game
	};

	static bool visible( GameEntity* ent, const vec3_t from, GameEntity* target );
	static float ceilingAbove( const vec3_t origin, int passEntity );
	static void buildPads();
	static bool flagPosition( int team, vec3_t out );
	static void buildRoamPoints();
	static void spreadRoamPoints( const Vec3& centre, const float reach[4], float sky );
	static void dropRoamPoint( const Vec3& point, float sky );
	static bool runwayAt( const vec3_t point, float* surface = nullptr );
	static float runwayRun( const vec3_t origin, float yaw, float maxDist );
	// the touchdown point sits this far inside the runway's end: close to it on a short strip, where every unit of roll counts
	static float thresholdInset( const Pad& pad ) { return pad.halfLength < 600.0f ? 60.0f : 150.0f; }
	static float approachSlope( const Pad& pad, float dir );
	static int findPad( GameEntity* ent, int skip = -1 );
	static float heightAboveGround( const vec3_t origin, int passEntity );
	// how far the nearest pad of any kind is: where a soldier on foot goes to take a vehicle
	static float padDistance( const vec3_t point );
	static bool pathBlocked( const vec3_t origin, const vec3_t dir, float dist, int passEntity, trace_t& tr, float halfWidth = 0 );
	static std::vector<Pad> pads_;
	static bool padsBuilt_;
	static std::vector<Vec3> roamPoints_;	// places to fly to: the pads and points spread over the map
	static std::vector<Vec3> landPoints_;	// the points spread over the map that are on land, at the surface
	static std::vector<Vec3> waterPoints_;	// and those on water
};

// What a bot does with its weapons: which one for which target, when to fire it, flares, bombs, and what a vehicle
// is short of. Asks Bots for the bot's character and skill, which decide reaction, aim window and care for team mates.
class BotWeapons
{
public:
	static bool hasWeaponType( int vehicle, const playerState_t& ps, int type );
	static void bomb( GameEntity* ent, Bots::State& bot, const vec3_t target, usercmd_t& cmd, vec3_t view );
	static void flares( GameEntity* ent, Bots::State& bot, const Bots::Sense& sense, usercmd_t& cmd );
	static bool friendlyNear( GameEntity* ent, const vec3_t point, float radius );
	static int supplyNeed( GameEntity* who, bool urgentOnly = false );
	static bool carriesSupplies( int vehicle );
	static void fireWeapons( GameEntity* ent, Bots::State& bot, const Bots::Sense& sense, usercmd_t& cmd, float aimError );
	static int findWeaponSlot( int vehicle, const playerState_t& ps, bool air, float dist, bool& guided );
	static bool outOfAmmo( int vehicle, const playerState_t& ps );
	static void setRadar( GameEntity* ent, Bots::State& bot, bool air );
};

// What a bot perceives: what its vehicle is for, who is worth fighting and where they were last seen, who leads it,
// who rides with it or wants a lift, and the frame's summary of all that. Keeps what it learns in the bot's state.
class BotSenses
{
public:
	static GameEntity* findEnemy( GameEntity* ent, Bots::State& bot, const Capabilities& can );
	static bool outranged( GameEntity* ent, const Bots::Sense& out );
	static void chooseLeader( GameEntity* ent, Bots::State& bot );
	static GameEntity* leaderOf( const Bots::State& bot );
	static int targetOf( GameEntity* leader );
	static Bots::Role roleOf( int vehicle );
	static float turnOnAttacker( GameEntity* ent, const Bots::State& bot, int attacker );
	// A team mate on foot within range who wants a lift from this vehicle, a real player first, or null. The offer is to
	// one walker at a time and one vehicle per walker; it is withdrawn when they have not come aboard in time (beside
	// is how near counts as waiting beside them), and that walker is passed over for a while
	static GameEntity* walkerNear( GameEntity* ent, Bots::State& bot, float range, float beside );
	// someone on foot within range who wants a lift and has not turned this vehicle down lately, without making an offer
	static GameEntity* liftSeeker( GameEntity* ent, const Bots::State& bot, float range );
	static int ridersOf( GameEntity* ent );
	static void sense( GameEntity* ent, Bots::State& bot, Bots::Sense& out );

private:
	// a contact stays watched this long after it was last seen
	static constexpr int trackingTime = 3000;
	// the contacts a bot is keeping its eyes on
	struct Attention
	{
		int		tracked = 0;
		int		limit = 3;
		float	furthest = 0;
	};
	// what one opponent is to the bot this frame
	struct Contact
	{
		float	dist = 0;
		bool	attackedBy = false;
		bool	harassing = false;
	};
	static void findHarassers( GameEntity* ent, const Capabilities& can, bool* harassing );
	static void trackEnemy( GameEntity* ent, Bots::State& bot, const Capabilities& can, Bots::Sense& out );
	static void aimAt( GameEntity* ent, Bots::State& bot, const Capabilities& can, Bots::Sense& out );
	static bool turretFlank( GameEntity* ent, const Bots::State& bot, const Bots::Sense& out, bool fighter, float dist, Vec3& target );
	static bool thinnestSide( GameEntity* ent, GameEntity* bomber, Vec3& side );
	static bool teamFlank( GameEntity* ent, const Bots::Sense& out, float dist, Vec3& target );
	static void cunningApproach( GameEntity* ent, const Bots::State& bot, const Bots::Sense& out, bool fighter, float dist, Vec3& target );
	static void recordTrail( GameEntity* ent, Bots::State& bot );
	static void ownState( GameEntity* ent, const Capabilities& can, Bots::Sense& out );
	static void noticeHits( GameEntity* ent, Bots::State& bot, Bots::Sense& out );
	static void findMissile( GameEntity* ent, Bots::Sense& out );
	static void findAirThreat( GameEntity* ent, Bots::Sense& out );
	static int foesNear( GameEntity* ent );
	static void reviewGoal( GameEntity* ent, Bots::State& bot, const Bots::Sense& out );
	static bool isOpponent( GameEntity* ent, GameEntity* other );
	static Attention attentionOf( GameEntity* ent, const Bots::State& bot );
	static bool locate( GameEntity* ent, Bots::State& bot, GameEntity* other, const Attention& attention, Contact& contact );
	static bool inSight( GameEntity* ent, GameEntity* other, float dist, float sightShare );
	static bool onRadar( GameEntity* ent, GameEntity* other, float dist );
	static bool noticed( GameEntity* ent, const Bots::State& bot, const Bots::Seen& seen, const vec3_t where, float dist, const Attention& attention );
	static bool ahead( GameEntity* ent, const vec3_t point );
	static bool lookingRound( GameEntity* ent, const Bots::State& bot );
	static float targetScore( GameEntity* ent, Bots::State& bot, const Capabilities& can, GameEntity* other, const Contact& contact );
};

#endif
