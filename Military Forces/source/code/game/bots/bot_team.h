#ifndef __BOT_TEAM_H__
#define __BOT_TEAM_H__

#include "bot.h"

// What a team of bots decides together in a team game: what it fields, what it is doing, and what each bot is asked
// to do about it. The plan belongs to the team, not to any bot, and is remade every few seconds and whenever a flag
// changes hands. A bot's assignment is a wish among its other wishes: the pilots and drivers read it where they pick
// a place to go and someone to fight. Real players are never given one; the plan is laid round what they do.
class BotTeam
{
public:
	enum class Job { None, Attack, Defend, Escort, AirCover, Supply, Chase, CutOff };
	// what the team fields, as the make-up counts it
	enum class Kind { Tank, AirDefence, AttackHelo, Supply, Fighter, Bomber, Boat, Other, Count };

	struct Assignment
	{
		Job		job = Job::None;
		bool	hasPlace = false;	// somewhere to be
		Vec3	place;
		int		target = -1;	// the enemy to go for first
		int		ward = -1;	// the team mate to stay with
		int		goTime = 0;	// held at the place until then: a push or a joint attack that goes in together
		bool	flank = false;	// come at the target along the bearing below, not straight in
		float	bearing = 0;	// from the target, in degrees
		float	lift = 0;	// and this much above or below it
	};

	static void reset();
	static void runFrame();
	// 0: every bot for itself; 1: balanced teams, shared sightings, a plan; 2: timed pushes, cutting off, joint attacks
	static int level();
	static const Assignment& of( int clientNum );
	static Kind kindOf( int vehicle );
	// the kind of vehicle the team is shortest of, for a bot about to choose; false when it may please itself
	static bool wantedKind( GameEntity* ent, Kind& kind );
	// the side a joining bot should take so that both have the same number and the same skill
	static const char* sideToJoin( const Bots::State& joiner );
	// what one bot sees its team knows, after a delay that depends on skill
	static void report( int team, int enemy, const vec3_t position );
	static bool known( int team, int enemy, vec3_t position, int& time );
	static void onDeath( GameEntity* self, GameEntity* attacker );
	static void listCommand();
	static const Bots::State& stateOf( int clientNum );
};

#endif
