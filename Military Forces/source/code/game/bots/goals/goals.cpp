#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "../team/team.h"
#include <span>
#include "../core/telemetry.h"
#include <algorithm>
#include <cmath>

// what a goal's score is worked out from
struct Situation
{
	GameEntity*				ent;
	Bots::State&			bot;
	const Bots::Sense&		sense;
	const Bots::Character&	who;
	const playerState_t&	ps;
	int						vehicle;
};

// One score per goal, from the bot's situation and its character; the highest wins. The small helpers are the
// needs and threats the scores are built from, each from 0 (none) upwards.
struct GoalScoring
{
	using Role = Bots::Role;

	static float hurt( const Situation& s ) { return 1.0f - s.sense.health; }
	static float missileThreat( const Situation& s ) { return s.sense.launched ? 0.5f : s.sense.locked ? 0.15f : 0.0f; }
	// being hit by someone unseen frightens far more than a fight it can see
	static float takingFire( const Situation& s ) { return !s.sense.underFire ? 0.0f : s.sense.blindsided ? 1.4f : 0.3f; }
	static bool plane( const Situation& s ) { return s.sense.can.runway; }
	static bool armour( const Situation& s ) { return s.sense.can.armoured; }
	static bool bomber( const Situation& s ) { return s.sense.can.groundStrike; }
	static bool fighter( const Situation& s ) { return s.sense.can.runway && !s.sense.can.groundStrike; }
	// armour without anti-aircraft weapons of its own kind: aircraft overhead are something to hide from
	static bool tank( const Situation& s ) { return s.sense.can.armoured && !s.sense.can.antiAirVehicle; }
	static bool has( const Situation& s, int type ) { return BotWeapons::hasWeaponType( s.vehicle, s.ps, type ); }
	static bool hasGroundWeapons( const Situation& s )
	{
		return has( s, WT_IRONBOMB ) || has( s, WT_GUIDEDBOMB ) || has( s, WT_ANTIGROUNDMISSILE ) || has( s, WT_ROCKET );
	}
	// how far a bomber is below the height it wants for a bomb run, with extra weight when well below
	static float bomberAltitudeNeed( const Situation& s )
	{
		const float want = bomberRunHeight( s.sense.height, BotWorld::ceilingAbove( s.ent->r.currentOrigin, s.ent->s.number ) );
		return std::clamp( 1.0f - s.sense.height / want, 0.0f, 1.0f ) + (s.sense.height < want * 0.6f ? 0.5f : 0.0f);
	}
	// an enemy aircraft: 1 when close, fading to nothing a little beyond the range
	static float airThreatNear( const Situation& s, float range )
	{
		return s.sense.airThreat ? std::clamp( 1.3f - s.sense.airThreatDist / range, 0.0f, 1.0f ) : 0.0f;
	}
	// how well a bomber can look after itself with its own guns: the old heavy bombers can, modern ones cannot
	static float selfDefence( const Situation& s )
	{
		const completeVehicleData_t& veh = availableVehicles[s.vehicle];
		if( (veh.gameset & MF_GAMESET_MODERN) || !veh.weapons[WP_MACHINEGUN] || s.ps.ammo[WP_MACHINEGUN] <= 0 )
			return 0.0f;
		return std::clamp( veh.maxhealth / 1000.0f, 0.0f, 1.0f );
	}
	// one against one, an honourable character does not run or leave to rearm
	static float duelResolve( const Situation& s, float weight ) { return s.sense.duel ? 1.0f - weight * s.who.honour : 1.0f; }

	// a stealth helicopter's fight is its missiles: with none left for this enemy it has nothing to stay for
	static bool strikeSpent( const Situation& s )
	{
		if( !s.sense.stealth || !s.sense.can.gunship || !s.sense.enemy )
			return false;
		bool guided = false;
		return BotWeapons::findWeaponSlot( s.vehicle, s.ps, s.sense.enemyAir, 0.0f, guided ) < 0 || !guided;
	}

	// Each goal's score is a list of terms, applied in order: a term adds to the score or scales it. A term that does
	// not apply adds nothing or scales by one. The terms are data in the goal table below.

	// fight: by aggression, less when hurt
	static float aggression( const Situation& s )
	{
		if( !s.sense.enemy )
			return 0.0f;
#ifdef MF_BOT_DEBUG
		// test switch: bots see each other but do not fight
		if( g_botPeaceful.integer )
			return 0.0f;
#endif
		return s.who.aggression * (0.4f + 0.6f * s.sense.health);
	}
	// a bomber without bombs has little to fight with
	static float noPayload( const Situation& s ) { return bomber( s ) && !hasGroundWeapons( s ) ? 0.5f : 1.0f; }
	static float spentStrike( const Situation& s ) { return strikeSpent( s ) ? 0.3f : 1.0f; }
	// a bomber with guns of its own stands up to fighters close by
	static float bomberSelfDefence( const Situation& s )
	{
		return bomber( s ) && s.sense.enemyAir && s.sense.enemyDist < 2500 ? 0.4f * selfDefence( s ) : 0.0f;
	}

	// get away: by caution, from damage while pressed, missiles and incoming fire; armour shrugs most of it off
	static float hurtWhilePressed( const Situation& s )
	{
		// damage alone is no reason to keep running once nobody is near
		constexpr float pressedRange = 6000.0f;
		const bool pressed = s.sense.underFire || s.sense.locked || s.sense.launched || (s.sense.enemy && s.sense.enemyDist < pressedRange);
		return s.who.caution * (pressed ? hurt( s ) * 1.2f : 0.0f);
	}
	static float missile( const Situation& s ) { return s.who.caution * missileThreat( s ); }
	static float incoming( const Situation& s ) { return s.who.caution * takingFire( s ); }
	static float armourShrugs( const Situation& s ) { return armour( s ) && s.sense.health > 0.3f ? 0.4f : 1.0f; }
	static float bomberUnderAirThreat( const Situation& s )
	{
		return bomber( s ) && s.sense.airThreat && s.sense.airThreatDist < 3000 ? s.who.caution * 0.8f * (1.0f - selfDefence( s )) : 0.0f;
	}
	static float stealthRevealed( const Situation& s ) { return s.sense.stealth && s.sense.revealed && plane( s ) ? s.who.caution * 0.6f : 0.0f; }
	static float duelResolveEvade( const Situation& s ) { return duelResolve( s, 0.7f ); }
	// inside the enemy's reach with nothing that reaches back: get out of it, the cautious sooner
	static float outranged( const Situation& s ) { return s.sense.outranged ? s.who.caution * 0.5f : 0.0f; }

	// gain height: a bomber with a load for its run, a fighter to come down on a distant enemy, a stealth aircraft while unseen
	static float bombRunHeight( const Situation& s )
	{
		return bomber( s ) && hasGroundWeapons( s ) ? 1.2f * s.who.patience * bomberAltitudeNeed( s ) * (1.0f - 0.5f * selfDefence( s )) : 0.0f;
	}
	static float heightOverEnemy( const Situation& s )
	{
		if( !fighter( s ) || !s.sense.enemy || !s.sense.enemyAir || s.sense.enemyDist <= 3000 )
			return 0.0f;
		return 0.5f * s.who.patience * std::clamp( (s.sense.enemy->r.currentOrigin[2] + 400 - s.ent->r.currentOrigin[2]) / 800.0f, 0.0f, 1.0f );
	}
	static float stealthApproach( const Situation& s )
	{
		return s.sense.stealth && !s.sense.revealed && s.sense.enemy && s.sense.enemyDist > 2500 ? 0.3f * s.who.patience : 0.0f;
	}

	// a tank takes cover from fire it cannot place and from aircraft it cannot shoot at
	static float unseenFire( const Situation& s ) { return tank( s ) && s.sense.blindsided ? s.who.caution * 1.2f : 0.0f; }
	static float aircraftOverhead( const Situation& s )
	{
		return tank( s ) && !has( s, WT_ANTIAIRMISSILE ) ? s.who.caution * 1.6f * airThreatNear( s, 5000.0f ) : 0.0f;
	}

	// How empty a vehicle's weapons are, 0 full to 1 empty, across those its job is about: a bomber's payload, a supply
	// helicopter's crates, and for everyone the whole of what it carries; the emptier of the two counts. Flares are a
	// need of their own
	static float ammunitionUsed( const Situation& s )
	{
		const bool supplier = BotWeapons::carriesSupplies( s.vehicle );
		float used = 0, jobUsed = 0;
		int slots = 0, jobSlots = 0;
		for( int slot = WP_MACHINEGUN; slot < WP_FLARE; slot++ )
		{
			const int most = s.ps.ammo[slot + 8];
			if( !availableVehicles[s.vehicle].weapons[slot] || most <= 0 )
				continue;
			const float empty = 1.0f - std::clamp( static_cast<float>( s.ps.ammo[slot] ) / most, 0.0f, 1.0f );
			used += empty;
			slots++;
			const int type = availableWeapons[availableVehicles[s.vehicle].weapons[slot]].type;
			const bool payload = bomber( s ) && (type == WT_IRONBOMB || type == WT_GUIDEDBOMB || type == WT_ANTIGROUNDMISSILE || type == WT_ROCKET);
			const bool crate = supplier && (type == WT_FUELCRATE || type == WT_AMMOCRATE || type == WT_HEALTHCRATE);
			if( payload || crate )
			{
				jobUsed += empty;
				jobSlots++;
			}
		}
		return std::max( slots ? used / slots : 0.0f, jobSlots ? jobUsed / jobSlots : 0.0f );
	}

	// The wish to go back to a pad grows with what is missing, from nothing when full: as the cube of it, so a little
	// used hardly counts and running out counts fast; half the ammunition gone is not yet worth the trip when nothing
	// else is going on. Fuel is the need that cannot wait: at under half a tank it is a full reason on its own, and
	// below a quarter not even a duel outweighs it
	static constexpr float fuelWanted = 0.45f, fuelUrgent = 0.25f;
	static float fraction( int have, int most ) { return most > 0 ? std::clamp( static_cast<float>( have ) / most, 0.0f, 1.0f ) : 1.0f; }
	static float fuelNeed( const Situation& s )
	{
		constexpr float urgentPull = 8.0f;
		const float fuel = fraction( s.ps.stats[STAT_FUEL], s.ps.stats[STAT_MAX_FUEL] );
		const float missing = (1.0f - fuel) / (1.0f - fuelWanted);
		return missing * missing * missing + std::max( 0.0f, fuelWanted - fuel ) * urgentPull;
	}
	// repairs: as much at under half health as the character's caution makes of it
	static float damageNeed( const Situation& s )
	{
		constexpr float repairWanted = 0.45f;
		const float missing = (1.0f - s.sense.health) / (1.0f - repairWanted);
		return std::min( missing * missing * missing, 2.0f ) * (0.2f + 0.8f * s.who.caution);
	}
	static float ammunitionNeed( const Situation& s )
	{
		constexpr float emptyWeight = 1.1f;
		const float used = ammunitionUsed( s );
		return used * used * used * emptyWeight;
	}
	static float flareNeed( const Situation& s )
	{
		const int most = s.ps.ammo[WP_FLARE + 8];
		if( availableVehicles[s.vehicle].ammo[WP_FLARE] <= 0 || most <= 0 || tank( s ) )
			return 0.0f;
		const float used = 1.0f - fraction( s.ps.ammo[WP_FLARE], most );
		return used * used * used * 0.3f * s.who.caution;
	}
	// an honourable character in a duel does not leave to rearm, unless the tank is nearly dry
	static float duelResolveRearm( const Situation& s )
	{
		return fraction( s.ps.stats[STAT_FUEL], s.ps.stats[STAT_MAX_FUEL] ) >= fuelUrgent ? duelResolve( s, 1.0f ) : 1.0f;
	}
	// once on the way to a pad with something still missing, see it through
	static float underWay( const Situation& s ) { return s.bot.rearm.pad >= 0 && !Bots::replenished( s.ent ) ? 0.7f : 0.0f; }
	// nothing to go back to: full of all a pad gives, on foot, a plane that does not land, or no pad this vehicle can use
	static float padToGo( const Situation& s )
	{
		if( s.sense.can.onFoot || Bots::replenished( s.ent ) )
			return 0.0f;
		if( plane( s ) && (!g_botLanding.integer || theLevel.time_ < s.bot.landing.giveUp) )
			return 0.0f;
		return BotWorld::findPad( s.ent, theLevel.time_ < s.bot.rearm.badPadTime ? s.bot.rearm.badPad : -1 ) >= 0 ? 1.0f : 0.0f;
	}

	// the flag: surface vehicles and helicopters only, and nothing matters more than bringing it home
	static float flag( const Situation& s )
	{
		if( g_gametype.integer != GT_CTF || plane( s ) )
			return 0.0f;
		const bool carrying = (s.ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0;
		if( carrying )
			return 1.6f;
		// the team's plan: those told to guard, escort or hunt leave the flag to those sent for it
		const BotTeam::Job job = BotTeam::of( s.ent->s.number ).job;
		if( job != BotTeam::Job::None && job != BotTeam::Job::Attack )
			return 0.0f;
		// the nearer their flag, the less a passing enemy is worth turning aside for: a bot that has fought its way
		// to the enemy base goes on and takes the flag
		constexpr float flagNear = 4000.0f;
		const int own = s.ent->client_->sess_.sessionTeam_;
		vec3_t flag;
		float closeness = 0.0f;
		if( BotWorld::flagPosition( own == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED, flag ) )
			closeness = std::clamp( 1.0f - Distance( flag, s.ent->r.currentOrigin ) / flagNear, 0.0f, 1.0f );
		return (0.35f + 0.5f * s.who.teamwork) * (1.0f + 1.5f * closeness);
	}
};

namespace
{
	enum class Combine { Add, Scale };
	struct Term
	{
		const char*	name;
		Combine		combine;
		float		(*value)( const Situation& );
	};
	using G = GoalScoring;
	constexpr Term attackTerms[] = { { "aggression", Combine::Add, G::aggression }, { "no payload", Combine::Scale, G::noPayload },
		{ "strike spent", Combine::Scale, G::spentStrike }, { "self defence", Combine::Add, G::bomberSelfDefence } };
	constexpr Term evadeTerms[] = { { "hurt while pressed", Combine::Add, G::hurtWhilePressed }, { "missile", Combine::Add, G::missile },
		{ "incoming", Combine::Add, G::incoming }, { "armour", Combine::Scale, G::armourShrugs }, { "bomber under air threat", Combine::Add, G::bomberUnderAirThreat },
		{ "stealth revealed", Combine::Add, G::stealthRevealed }, { "outranged", Combine::Add, G::outranged }, { "duel", Combine::Scale, G::duelResolveEvade } };
	constexpr Term climbTerms[] = { { "bomb run height", Combine::Add, G::bombRunHeight }, { "height over enemy", Combine::Add, G::heightOverEnemy },
		{ "stealth approach", Combine::Add, G::stealthApproach } };
	constexpr Term hideTerms[] = { { "unseen fire", Combine::Add, G::unseenFire }, { "aircraft overhead", Combine::Add, G::aircraftOverhead } };
	constexpr Term rearmTerms[] = { { "fuel", Combine::Add, G::fuelNeed }, { "damage", Combine::Add, G::damageNeed },
		{ "ammunition", Combine::Add, G::ammunitionNeed }, { "flares", Combine::Add, G::flareNeed }, { "duel", Combine::Scale, G::duelResolveRearm },
		{ "under way", Combine::Add, G::underWay }, { "pad to go to", Combine::Scale, G::padToGo } };
	constexpr Term captureTerms[] = { { "flag", Combine::Add, G::flag } };

	struct GoalEntry
	{
		Bots::Goal				goal;
		const char*				name;
		std::span<const Term>	terms;
	};

	constexpr std::array<GoalEntry, 6> goalTable = { {
		{ Bots::Goal::Attack, "attack", attackTerms },
		{ Bots::Goal::Evade, "evade", evadeTerms },
		{ Bots::Goal::Climb, "climb", climbTerms },
		{ Bots::Goal::Hide, "hide", hideTerms },
		{ Bots::Goal::Rearm, "rearm", rearmTerms },
		{ Bots::Goal::Capture, "capture", captureTerms }
	} };

	// Per role, a weight on each goal's score, in the goal table's order: attack, evade, climb, hide, rearm, capture.
	// The terms already ask what a vehicle can do; this is where a role is made keener or less keen on a goal as a
	// whole. g_botWeights can scale a cell for a sweep ("Bomber.attack=1.2")
	constexpr std::array<const char*, 8> roleNames = { "Fighter", "Bomber", "AttackHelo", "TransportHelo", "Tank", "AirDefence", "Boat", "Infantry" };
	constexpr float roleWeights[roleNames.size()][goalTable.size()] = {
		/* Fighter */		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
		/* Bomber */		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
		/* AttackHelo */	{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
		/* TransportHelo */	{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
		/* Tank */			{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
		/* AirDefence */	{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
		/* Boat */			{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
		/* Infantry */		{ 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f },
	};

	float roleWeight( Bots::Role role, size_t goal )
	{
		const size_t r = static_cast<size_t>( role );
		char key[64];
		Com_sprintf( key, sizeof( key ), "%s.%s", roleNames[r], goalTable[goal].name );
		return roleWeights[r][goal] * BotTelemetry::weight( key );
	}

	// a goal's score: its terms in order
	float scoreOf( const GoalEntry& entry, const Situation& situation )
	{
		float score = 0.0f;
		for( const Term& term : entry.terms )
		{
			const float value = term.value( situation ) * (term.combine == Combine::Add ? BotTelemetry::weight( term.name ) : 1.0f);
			score = term.combine == Combine::Add ? score + value : score * value;
		}
		return score;
	}

	constexpr float roamScore = 0.2f;
}

// the goal with the highest score; the one already held gets a bonus so that close scores do not flip it back and forth
Bots::Goal Bots::chooseGoal( GameEntity* ent, State& bot, const Sense& sense )
{
	const Character& who = character( bot );
	const Situation situation{ ent, bot, sense, who, ent->client_->ps_, ent->client_->vehicle_ };
	const float keep = 1.25f + 0.25f * who.patience;

	Goal goal = Goal::Roam;
	float best = roamScore;
	std::array<float, goalTable.size()> scores;
	for( size_t i = 0; i < goalTable.size(); i++ )
	{
		float score = scoreOf( goalTable[i], situation ) * roleWeight( sense.role, i );
		if( goalTable[i].goal == bot.desire && goalTable[i].goal != Goal::Rearm )
			score *= keep;
		scores[i] = score;
		if( score > best )
		{
			best = score;
			goal = goalTable[i].goal;
		}
	}

#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 && goal != bot.desire )
	{
		Common::printf( "BOTSCORE t %d bot %d", theLevel.time_, ent->s.number );
		for( size_t i = 0; i < goalTable.size(); i++ )
			Common::printf( " %s %.2f", goalTable[i].name, scores[i] );
		Common::printf( "\n" );
	}
#endif

	// rearming needs a pad chosen; any other goal lets it go
	if( goal == Goal::Rearm )
	{
		if( bot.rearm.pad < 0 )
		{
			bot.rearm.pad = BotWorld::findPad( ent, theLevel.time_ < bot.rearm.badPadTime ? bot.rearm.badPad : -1 );
			bot.rearm.phase = 0;
		}
	}
	else
		bot.rearm.letGo();
	return goal;
}

// full again: fuel, health and every weapon
bool Bots::replenished( GameEntity* ent )
{
	const GameClient* client = ent->client_;
	if( client->ps_.stats[STAT_FUEL] < client->ps_.stats[STAT_MAX_FUEL] * 0.95f || ent->health_ < client->pers_.maxHealth_ * 0.95f )
		return false;
	for( int slot : allWeaponSlots )
	{
		if( client->ps_.ammo[slot] < client->ps_.ammo[slot + 8] )
			return false;
	}
	return true;
}

// done at the pad: clear the landing and take-off state and look for something to do
void Bots::finishRearm( GameEntity* ent, State& bot )
{
	bot.takeoff.reset();
	bot.rearm.letGo();
	bot.goalEvalTime = 0;
	bot.commitTime = 0;
	bot.desire = Goal::Roam;
}
