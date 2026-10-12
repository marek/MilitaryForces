#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "orders.h"
#include <algorithm>
#include <vector>

void SayTeam( GameEntity* ent, const char* text );

std::array<BotOrders::Order, MAX_CLIENTS + 1> BotOrders::orders_;
std::array<BotTeam::Assignment, MAX_CLIENTS + 1> BotOrders::made_;
std::array<int, MAX_CLIENTS + 1> BotOrders::spokeAt_ = {};
std::array<int, MAX_CLIENTS + 1> BotOrders::reportedAt_ = {};

namespace
{
	constexpr int orderLasts = 60000;
	constexpr int quietFor = 10000;	// between two things one bot says
	constexpr float lookAlong = 8000.0f;	// how far ahead an attack order points
	constexpr float escortBehind = 400.0f;
	constexpr float overBase = 3000.0f;	// an enemy aircraft this near a base is reported

	struct Name
	{
		const char*			word;
		BotOrders::Kind		kind;
	};
	constexpr Name names[] = { { "cover", BotOrders::Kind::Cover }, { "supply", BotOrders::Kind::Supply }, { "pickup", BotOrders::Kind::Pickup },
		{ "attack", BotOrders::Kind::Attack }, { "defend", BotOrders::Kind::Defend } };
}

void BotOrders::reset()
{
	orders_.fill( Order{} );
	spokeAt_.fill( 0 );
	reportedAt_.fill( 0 );
}

void BotOrders::say( GameEntity* bot, const char* text )
{
	const int i = bot->s.number;
	if( i < 1 || i > MAX_CLIENTS || (spokeAt_[i] && theLevel.time_ < spokeAt_[i] + quietFor) )
		return;
	spokeAt_[i] = theLevel.time_;
	SayTeam( bot, text );
}

void BotOrders::command( GameEntity* from )
{
	char word[MAX_TOKEN_CHARS];
	CommandSystem::argvBuffer( 1, word, sizeof( word ) );
	Kind kind = Kind::None;
	for( const Name& name : names )
	{
		if( !Q_stricmp( word, name.word ) )
			kind = name.kind;
	}
	if( kind == Kind::None || g_gametype.integer < GT_TEAM )
	{
		GameHost::sendServerCommand( from->s.number, "print \"order cover|supply|pickup|attack|defend (team games)\n\"" );
		return;
	}
	std::vector<std::pair<float, GameEntity*>> fitting;
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		GameEntity* bot = theLevel.getEntity( i );
		if( bot != from && Bots::isBot( i ) && alive( bot ) && OnSameTeam( bot, from ) && fits( bot, kind ) )
			fitting.push_back( { Distance( bot->r.currentOrigin, from->r.currentOrigin ), bot } );
	}
	std::sort( fitting.begin(), fitting.end(), []( const auto& a, const auto& b ) { return a.first < b.first; } );
	const Vec3 place = placeOf( from, kind );
	const int count = std::min( howMany( kind ), static_cast<int>( fitting.size() ) );
	for( int n = 0; n < count; n++ )
	{
		GameEntity* bot = fitting[n].second;
		orders_[bot->s.number] = { kind, from->s.number, place, theLevel.time_ + orderLasts };
		say( bot, "On my way." );
	}
	if( !count )
		GameHost::sendServerCommand( from->s.number, "print \"No bot on the team can do that now.\n\"" );
}

// who can carry an order out: an armed vehicle to cover, attack or defend; a supply carrier to supply; a vehicle
// with a free seat to pick someone up
bool BotOrders::fits( GameEntity* bot, Kind kind )
{
	const int vehicle = bot->client_->vehicle_;
	if( vehicle < 0 || bot->client_->sess_.ridingClient_ >= 0 )
		return false;
	const Capabilities can = Capabilities::of( vehicle );
	switch( kind )
	{
	case Kind::Supply:
		return can.supplies;
	case Kind::Pickup:
		return availableVehicles[vehicle].seats > 1 && !can.runway && BotSenses::ridersOf( bot ) + 1 < availableVehicles[vehicle].seats;
	case Kind::Cover:
	case Kind::Attack:
	case Kind::Defend:
		return !can.onFoot && !can.supplies;
	default:
		return false;
	}
}

int BotOrders::howMany( Kind kind )
{
	switch( kind )
	{
	case Kind::Cover: return 2;
	case Kind::Attack:
	case Kind::Defend: return 3;
	default: return 1;
	}
}

// where an order points: for an attack, where the player is looking; otherwise where the player is
Vec3 BotOrders::placeOf( GameEntity* from, Kind kind )
{
	if( kind != Kind::Attack )
		return Vec3( from->r.currentOrigin );
	vec3_t forward, end;
	AngleVectors( from->client_->ps_.viewangles, forward, nullptr, nullptr );
	VectorMA( from->r.currentOrigin, lookAlong, forward, end );
	trace_t tr;
	GameHost::trace( &tr, from->r.currentOrigin, nullptr, nullptr, end, from->s.number, MASK_SOLID, false );
	return Vec3( tr.endpos );
}

const BotTeam::Assignment* BotOrders::assignmentOf( int clientNum )
{
	if( clientNum < 1 || clientNum > MAX_CLIENTS )
		return nullptr;
	Order& order = orders_[clientNum];
	GameEntity* from = order.from >= 0 ? theLevel.getEntity( order.from ) : nullptr;
	if( order.kind == Kind::None || theLevel.time_ > order.until || !alive( from ) )
	{
		order.kind = Kind::None;
		return nullptr;
	}
	BotTeam::Assignment& a = made_[clientNum];
	a = BotTeam::Assignment{};
	a.hasPlace = true;
	a.place = order.place;
	switch( order.kind )
	{
	case Kind::Cover:
	{
		vec3_t forward;
		const vec3_t level = { 0, from->client_->ps_.vehicleAngles[YAW], 0 };
		AngleVectors( level, forward, nullptr, nullptr );
		a.job = BotTeam::Job::Escort;
		a.ward = order.from;
		a.place = Vec3( from->r.currentOrigin ) - Vec3( forward ) * escortBehind;
		break;
	}
	case Kind::Supply:
	case Kind::Pickup:
		a.job = order.kind == Kind::Supply ? BotTeam::Job::Supply : BotTeam::Job::Escort;
		a.ward = order.from;
		a.place = Vec3( from->r.currentOrigin );
		break;
	case Kind::Attack:
		a.job = BotTeam::Job::Attack;
		break;
	case Kind::Defend:
		a.job = BotTeam::Job::Defend;
		break;
	default:
		return nullptr;
	}
	return &a;
}

// an enemy aircraft over one of the team's bases, and a call for supplies when short and none is coming
void BotOrders::report( GameEntity* bot, const Bots::State& state, const Bots::Sense& sense )
{
	const int i = bot->s.number;
	if( g_gametype.integer < GT_TEAM || (reportedAt_[i] && theLevel.time_ < reportedAt_[i] + quietFor) )
		return;
	const int team = bot->client_->sess_.sessionTeam_;
	if( sense.airThreat && sense.enemyVisible )
	{
		for( const BotWorld::Pad& pad : BotWorld::pads_ )
		{
			if( pad.team == team && Distance( pad.spot, sense.airThreat->r.currentOrigin ) < overBase )
			{
				reportedAt_[i] = theLevel.time_;
				say( bot, "Enemy aircraft over our base!" );
				return;
			}
		}
	}
	if( state.desire == Bots::Goal::Rearm && BotWeapons::supplyNeed( bot, true ) && Bots::teamHasSupplier( team, bot ) )
	{
		reportedAt_[i] = theLevel.time_;
		say( bot, "Need supplies here." );
	}
}
