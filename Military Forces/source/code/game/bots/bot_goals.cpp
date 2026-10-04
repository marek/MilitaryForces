#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

struct Situation
{
	GameEntity*				ent;
	Bots::State&			bot;
	const Bots::Sense&		sense;
	const Bots::Character&	who;
	const playerState_t&	ps;
	int						vehicle;
};

struct GoalScoring
{
	using Role = Bots::Role;

	static float hurt( const Situation& s ) { return 1.0f - s.sense.health; }
	static float missileThreat( const Situation& s ) { return s.sense.launched ? 0.5f : s.sense.locked ? 0.15f : 0.0f; }
	static float takingFire( const Situation& s ) { return !s.sense.underFire ? 0.0f : s.sense.blindsided ? 1.4f : 0.3f; }
	static bool plane( const Situation& s ) { return s.sense.role == Role::Fighter || s.sense.role == Role::Bomber; }
	static bool armour( const Situation& s ) { return s.sense.role == Role::Tank || s.sense.role == Role::AirDefence; }
	static bool has( const Situation& s, int type ) { return Bots::hasWeaponType( s.vehicle, s.ps, type ); }
	static bool hasGroundWeapons( const Situation& s )
	{
		return has( s, WT_IRONBOMB ) || has( s, WT_GUIDEDBOMB ) || has( s, WT_ANTIGROUNDMISSILE ) || has( s, WT_ROCKET );
	}
	static float bomberAltitudeNeed( const Situation& s )
	{
		const float want = std::clamp( s.sense.height + Bots::ceilingAbove( s.ent->r.currentOrigin, s.ent->s.number ) - 700.0f, 800.0f, 3500.0f );
		return std::clamp( 1.0f - s.sense.height / want, 0.0f, 1.0f ) + (s.sense.height < want * 0.6f ? 0.5f : 0.0f);
	}
	static float airThreatNear( const Situation& s, float range )
	{
		return s.sense.airThreat ? std::clamp( 1.3f - s.sense.airThreatDist / range, 0.0f, 1.0f ) : 0.0f;
	}
	static float selfDefence( const Situation& s )
	{
		const completeVehicleData_t& veh = availableVehicles[s.vehicle];
		if( (veh.gameset & MF_GAMESET_MODERN) || !veh.weapons[WP_MACHINEGUN] || s.ps.ammo[WP_MACHINEGUN] <= 0 )
			return 0.0f;
		return std::clamp( veh.maxhealth / 1000.0f, 0.0f, 1.0f );
	}
	static float duelResolve( const Situation& s, float weight ) { return s.sense.duel ? 1.0f - weight * s.who.honour : 1.0f; }

	static float attack( const Situation& s )
	{
		if( !s.sense.enemy )
			return 0.0f;
		float score = s.who.aggression * (0.4f + 0.6f * s.sense.health);
		if( s.sense.role == Role::Bomber && !hasGroundWeapons( s ) )
			score *= 0.5f;
		if( s.sense.role == Role::Bomber && s.sense.enemyAir && s.sense.enemyDist < 2500 )
			score += 0.4f * selfDefence( s );
		return score;
	}

	static float evade( const Situation& s )
	{
		float score = s.who.caution * (hurt( s ) * 1.2f + missileThreat( s ) + takingFire( s ));
		if( armour( s ) && s.sense.health > 0.3f )
			score *= 0.4f;
		if( s.sense.role == Role::Bomber && s.sense.airThreat && s.sense.airThreatDist < 3000 )
			score += s.who.caution * 0.8f * (1.0f - selfDefence( s ));
		if( s.sense.stealth && s.sense.revealed && plane( s ) )
			score += s.who.caution * 0.6f;
		return score * duelResolve( s, 0.7f );
	}

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

	static float hide( const Situation& s )
	{
		if( s.sense.role != Role::Tank )
			return 0.0f;
		float score = s.sense.blindsided ? s.who.caution * 1.2f : 0.0f;
		if( !has( s, WT_ANTIAIRMISSILE ) )
			score += s.who.caution * 1.6f * airThreatNear( s, 5000.0f );
		return score;
	}

	static float rearm( const Situation& s )
	{
		if( s.sense.role == Role::Infantry )
			return 0.0f;
		if( plane( s ) && (!g_botLanding.integer || theLevel.time_ < s.bot.landGiveUp) )
			return 0.0f;
		float need = 0.0f;
		if( s.ps.stats[STAT_FUEL] < s.ps.stats[STAT_MAX_FUEL] * 0.35f )
			need = 1.0f;
		else if( s.sense.health < 0.45f )
			need = 0.2f + 0.8f * s.who.caution;
		else if( Bots::outOfAmmo( s.vehicle, s.ps ) )
			need = 0.7f;
		else if( availableVehicles[s.vehicle].ammo[WP_FLARE] > 0 && s.ps.ammo[WP_FLARE] <= 0 && s.sense.role != Role::Tank )
			need = 0.3f * s.who.caution;
		need *= duelResolve( s, 1.0f );
		if( s.bot.pad >= 0 )
			need = std::max( need, 0.9f );
		if( need > 0 && Bots::findPad( s.ent, theLevel.time_ < s.bot.badPadTime ? s.bot.badPad : -1 ) < 0 )
			return 0.0f;
		return need;
	}

	static float capture( const Situation& s )
	{
		if( g_gametype.integer != GT_CTF || plane( s ) )
			return 0.0f;
		const bool carrying = (s.ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0;
		return carrying ? 1.6f : 0.35f + 0.5f * s.who.teamwork;
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
		if( goalTable[i].goal == bot.desire && goalTable[i].goal != Goal::Rearm && goalTable[i].goal != Goal::Capture )
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

void Bots::finishRearm( GameEntity* ent, State& bot )
{
	bot.takeoffTime = 0;
	bot.pad = -1;
	bot.landPhase = 0;
	bot.goalEvalTime = 0;
	bot.commitTime = 0;
	bot.desire = Goal::Roam;
}
