#ifndef __BOT_ORDERS_H__
#define __BOT_ORDERS_H__

#include "team.h"

// Orders from a player to the bots of the team, and what bots tell the team. An order goes to the nearest bots that
// can carry it out and stands for a minute, as a strong wish like an assignment of the team's plan: fuel, missiles and
// being badly hurt still come first. A bot says something to its team at most once every ten seconds.
class BotOrders
{
public:
	enum class Kind { None, Cover, Supply, Pickup, Attack, Defend };

	// "order cover|supply|pickup|attack|defend" from a player
	static void command( GameEntity* from );
	// the assignment an order makes of this bot, or nullptr when it has none
	static const BotTeam::Assignment* assignmentOf( int clientNum );
	// a short team message from a bot, when it has not spoken lately
	static void say( GameEntity* bot, const char* text );
	// what bots tell the team unasked: an enemy aircraft over the base, a call for supplies
	static void report( GameEntity* bot, const Bots::State& state, const Bots::Sense& sense );
	static void reset();

private:
	struct Order
	{
		Kind	kind = Kind::None;
		int		from = -1;
		Vec3	place;
		int		until = 0;
	};
	static bool fits( GameEntity* bot, Kind kind );
	static int howMany( Kind kind );
	static Vec3 placeOf( GameEntity* from, Kind kind );

	static std::array<Order, MAX_CLIENTS + 1>					orders_;
	static std::array<BotTeam::Assignment, MAX_CLIENTS + 1>	made_;
	static std::array<int, MAX_CLIENTS + 1>					spokeAt_;
	static std::array<int, MAX_CLIENTS + 1>					reportedAt_;
};

#endif
