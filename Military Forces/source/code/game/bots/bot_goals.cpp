#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
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
	static bool plane( const Situation& s ) { return s.sense.role == Role::Fighter || s.sense.role == Role::Bomber; }
	static bool armour( const Situation& s ) { return s.sense.role == Role::Tank || s.sense.role == Role::AirDefence; }
	static bool has( const Situation& s, int type ) { return Bots::hasWeaponType( s.vehicle, s.ps, type ); }
	static bool hasGroundWeapons( const Situation& s )
	{
		return has( s, WT_IRONBOMB ) || has( s, WT_GUIDEDBOMB ) || has( s, WT_ANTIGROUNDMISSILE ) || has( s, WT_ROCKET );
	}
	// how far a bomber is below the height it wants for a bomb run, with extra weight when well below
	static float bomberAltitudeNeed( const Situation& s )
	{
		const float want = std::clamp( s.sense.height + Bots::ceilingAbove( s.ent->r.currentOrigin, s.ent->s.number ) - 700.0f, 800.0f, 3500.0f );
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

	// fight: by aggression, less when hurt; a bomber without bombs has little to fight with
	static float attack( const Situation& s )
	{
		if( !s.sense.enemy )
			return 0.0f;
#ifdef MF_BOT_DEBUG
		// test switch: bots see each other but do not fight
		if( g_botPeaceful.integer )
			return 0.0f;
#endif
		float score = s.who.aggression * (0.4f + 0.6f * s.sense.health);
		if( s.sense.role == Role::Bomber && !hasGroundWeapons( s ) )
			score *= 0.5f;
		if( s.sense.role == Role::Bomber && s.sense.enemyAir && s.sense.enemyDist < 2500 )
			score += 0.4f * selfDefence( s );
		return score;
	}

	// get away: by caution, from damage while pressed, missiles and incoming fire; armour shrugs most of it off
	static float evade( const Situation& s )
	{
		// damage alone is no reason to keep running once nobody is near
		constexpr float pressedRange = 6000.0f;
		const bool pressed = s.sense.underFire || s.sense.locked || s.sense.launched || (s.sense.enemy && s.sense.enemyDist < pressedRange);
		float score = s.who.caution * ((pressed ? hurt( s ) * 1.2f : 0.0f) + missileThreat( s ) + takingFire( s ));
		if( armour( s ) && s.sense.health > 0.3f )
			score *= 0.4f;
		if( s.sense.role == Role::Bomber && s.sense.airThreat && s.sense.airThreatDist < 3000 )
			score += s.who.caution * 0.8f * (1.0f - selfDefence( s ));
		if( s.sense.stealth && s.sense.revealed && plane( s ) )
			score += s.who.caution * 0.6f;
		return score * duelResolve( s, 0.7f );
	}

	// gain height: a bomber for its run, a fighter to come down on a distant enemy, a stealth aircraft while unseen
	static float climb( const Situation& s )
	{
		float score = 0.0f;
		if( s.sense.role == Role::Bomber )
			score = 1.2f * s.who.patience * bomberAltitudeNeed( s ) * (1.0f - 0.5f * selfDefence( s ));
		else if( s.sense.role == Role::Fighter && s.sense.enemy && s.sense.enemyAir && s.sense.enemyDist > 3000 )
			score = 0.5f * s.who.patience * std::clamp( (s.sense.enemy->r.currentOrigin[2] + 400 - s.ent->r.currentOrigin[2]) / 800.0f, 0.0f, 1.0f );
		if( s.sense.stealth && !s.sense.revealed && s.sense.enemy && s.sense.enemyDist > 2500 )
			score += 0.3f * s.who.patience;
		return score;
	}

	// a tank takes cover from fire it cannot place and from aircraft it cannot shoot at
	static float hide( const Situation& s )
	{
		if( s.sense.role != Role::Tank )
			return 0.0f;
		float score = s.sense.blindsided ? s.who.caution * 1.2f : 0.0f;
		if( !has( s, WT_ANTIAIRMISSILE ) )
			score += s.who.caution * 1.6f * airThreatNear( s, 5000.0f );
		return score;
	}

	// a supply helicopter with no crates left has nothing to give until it restocks
	static bool suppliesGone( const Situation& s )
	{
		if( !Bots::carriesSupplies( s.vehicle ) )
			return false;
		for( int slot : allWeaponSlots )
		{
			const int type = availableWeapons[availableVehicles[s.vehicle].weapons[slot]].type;
			if( (type == WT_FUELCRATE || type == WT_AMMOCRATE || type == WT_HEALTHCRATE) && s.ps.ammo[slot] > 0 )
				return false;
		}
		return true;
	}

	// go back to a pad for fuel, repairs, ammunition or flares
	static float rearm( const Situation& s )
	{
		if( s.sense.role == Role::Infantry )
			return 0.0f;
		if( plane( s ) && (!g_botLanding.integer || theLevel.time_ < s.bot.landGiveUp) )
			return 0.0f;
		// fuel is the need that cannot wait: it grows as the tank empties, and below a quarter not even a duel outweighs it
		constexpr float fuelWanted = 0.45f, fuelUrgent = 0.25f, fuelPull = 8.0f;
		const float fuel = static_cast<float>( s.ps.stats[STAT_FUEL] ) / std::max( 1, s.ps.stats[STAT_MAX_FUEL] );
		float need = 0.0f;
		if( fuel < fuelWanted )
			need = 1.0f + ( fuelWanted - fuel ) * fuelPull;
		else if( s.sense.health < 0.45f )
			need = 0.2f + 0.8f * s.who.caution;
		else if( Bots::outOfAmmo( s.vehicle, s.ps ) || suppliesGone( s ) )
			need = 0.7f;
		else if( availableVehicles[s.vehicle].ammo[WP_FLARE] > 0 && s.ps.ammo[WP_FLARE] <= 0 && s.sense.role != Role::Tank )
			need = 0.3f * s.who.caution;
		if( fuel >= fuelUrgent )
			need *= duelResolve( s, 1.0f );
		// once on the way to a pad, see it through
		if( s.bot.pad >= 0 )
			need = std::max( need, 0.9f );
		// no pad this vehicle can use: nothing to go back to
		if( need > 0 && Bots::findPad( s.ent, theLevel.time_ < s.bot.badPadTime ? s.bot.badPad : -1 ) < 0 )
			return 0.0f;
		return need;
	}

	// the flag: surface vehicles and helicopters only, and nothing matters more than bringing it home
	static float capture( const Situation& s )
	{
		if( g_gametype.integer != GT_CTF || plane( s ) )
			return 0.0f;
		const bool carrying = (s.ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0;
		if( carrying )
			return 1.6f;
		// the nearer their flag, the less a passing enemy is worth turning aside for: a bot that has fought its way
		// to the enemy base goes on and takes the flag
		constexpr float flagNear = 4000.0f;
		const int own = s.ent->client_->sess_.sessionTeam_;
		vec3_t flag;
		float closeness = 0.0f;
		if( Bots::flagPosition( own == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED, flag ) )
			closeness = std::clamp( 1.0f - Distance( flag, s.ent->r.currentOrigin ) / flagNear, 0.0f, 1.0f );
		return (0.35f + 0.5f * s.who.teamwork) * (1.0f + 1.5f * closeness);
	}
};

namespace
{
	struct GoalEntry
	{
		Bots::Goal	goal;
		const char*	name;
		float		(*score)( const Situation& );
	};

	constexpr std::array<GoalEntry, 6> goalTable = { {
		{ Bots::Goal::Attack, "attack", GoalScoring::attack },
		{ Bots::Goal::Evade, "evade", GoalScoring::evade },
		{ Bots::Goal::Climb, "climb", GoalScoring::climb },
		{ Bots::Goal::Hide, "hide", GoalScoring::hide },
		{ Bots::Goal::Rearm, "rearm", GoalScoring::rearm },
		{ Bots::Goal::Capture, "capture", GoalScoring::capture }
	} };

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
		float score = goalTable[i].score( situation );
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
		if( bot.pad < 0 )
		{
			bot.pad = findPad( ent, theLevel.time_ < bot.badPadTime ? bot.badPad : -1 );
			bot.landPhase = 0;
		}
	}
	else
	{
		bot.pad = -1;
		bot.landPhase = 0;
	}
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
	bot.takeoffTime = 0;
	bot.taxiTime = 0;
	bot.holdTime = 0;
	bot.takeoffHolds = 0;
	bot.pad = -1;
	bot.landPhase = 0;
	bot.goalEvalTime = 0;
	bot.commitTime = 0;
	bot.desire = Goal::Roam;
}
