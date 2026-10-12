#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "../team/team.h"
#include "../weapons/envelope.h"
#include "../navigation/intercept.h"
#include "../core/lod.h"
#include <algorithm>
#include <bit>
#include <cmath>

// what a vehicle is for, which decides whom it fights
Bots::Role BotSenses::roleOf( int vehicle )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	if( veh.cat & CAT_PLANE )
		return (veh.cls & CLASS_PLANE_BOMBER) ? Bots::Role::Bomber : Bots::Role::Fighter;
	if( veh.cat & CAT_HELO )
		return (veh.cls & CLASS_HELO_TRANSPORT) ? Bots::Role::TransportHelo : Bots::Role::AttackHelo;
	if( veh.cat & CAT_GROUND )
		return (veh.cls & CLASS_GROUND_SAM) ? Bots::Role::AirDefence : Bots::Role::Tank;
	if( veh.cat & CAT_BOAT )
		return Bots::Role::Boat;
	return Bots::Role::Infantry;
}

// how many players ride in this bot's vehicle
int BotSenses::ridersOf( GameEntity* ent )
{
	int riders = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other && other->inuse_ && other->client_ && other->client_->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED &&
			other->client_->sess_.ridingClient_ == ent->s.number )
			riders++;
	}
	return riders;
}

// A team mate on foot that this vehicle could take aboard, a real player first. The offer is held for a while and
// then dropped for a time, so a vehicle does not sit waiting for someone who is not interested.
GameEntity* BotSenses::walkerNear( GameEntity* ent, Bots::State& bot, float range )
{
	constexpr int offerTime = 15000;
	constexpr int restTime = 25000;
	constexpr float humanPreference = 0.3f;
	if( g_gametype.integer < GT_TEAM || theLevel.time_ < bot.liftGiveUp )
		return nullptr;
	if( BotSenses::ridersOf( ent ) >= availableVehicles[ent->client_->vehicle_].seats )
		return nullptr;
	GameEntity* walker = nullptr;
	float best = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) || !OnSameTeam( ent, other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		if( !(availableVehicles[other->client_->vehicle_].cat & CAT_LQM) )
			continue;
		const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		const float weighted = Bots::isBot( i ) ? dist : dist * humanPreference;
		if( dist < range && (!walker || weighted < best) )
		{
			walker = other;
			best = weighted;
		}
	}
	if( !walker )
	{
		bot.liftTime = 0;
		return nullptr;
	}
	if( !bot.liftTime )
		bot.liftTime = theLevel.time_;
	if( theLevel.time_ > bot.liftTime + offerTime )
	{
		bot.liftTime = 0;
		bot.liftGiveUp = theLevel.time_ + restTime;
		return nullptr;
	}
	return walker;
}

// the leader this bot follows, while that leader lives
GameEntity* BotSenses::leaderOf( const Bots::State& bot )
{
	GameEntity* leader = bot.leader >= 0 ? theLevel.getEntity( bot.leader ) : nullptr;
	return alive( leader ) ? leader : nullptr;
}

// who a leader is fighting: a bot's chosen enemy, a real player's locked target
int BotSenses::targetOf( GameEntity* leader )
{
	if( Bots::isBot( leader->s.number ) )
		return Bots::states_[leader->s.number].enemy;
	return leader->tracktarget_ && leader->tracktarget_->client_ ? leader->tracktarget_->s.number : -1;
}

// A team player picks someone to fight alongside: a real player first, otherwise a bot of the same kind that is
// not following anyone itself. The lower client number leads, so two bots never follow each other.
void BotSenses::chooseLeader( GameEntity* ent, Bots::State& bot )
{
	constexpr float humanRange = 9000.0f;
	constexpr float botRange = 9000.0f;
	constexpr float humanPreference = 0.3f;
	bot.leaderTime = theLevel.time_ + 2000;
	bot.leader = -1;
	if( !bot.teamPlayer || g_gametype.integer < GT_TEAM )
		return;
	const unsigned int ownCat = availableVehicles[ent->client_->vehicle_].cat;
	const bool flyer = (ownCat & (CAT_PLANE|CAT_HELO)) != 0;
	float best = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) || !OnSameTeam( ent, other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		const unsigned int cat = availableVehicles[other->client_->vehicle_].cat;
		if( cat & CAT_LQM )
			continue;
		const bool human = !Bots::isBot( i );
		const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		if( human )
		{
			// keep up with a real player in the same element: air with air, surface with surface
			if( flyer != ((cat & (CAT_PLANE|CAT_HELO)) != 0) || dist > humanRange )
				continue;
		}
		else if( !(cat & ownCat) || i > ent->s.number || Bots::states_[i].leader >= 0 || dist > botRange )
			continue;
		const float weighted = human ? dist * humanPreference : dist;
		if( bot.leader < 0 || weighted < best )
		{
			bot.leader = i;
			best = weighted;
		}
	}
}

// How much being shot by someone favours them as the next target: 0.4 turns on them readily, 1 ignores them.
// Breaking off costs the work already put into the current target, so that is weighed against how badly the bot is hurt.
float BotSenses::turnOnAttacker( GameEntity* ent, const Bots::State& bot, int attacker )
{
	constexpr float eager = 0.4f, closeFight = 1500.0f, nearlyDead = 0.4f;
	GameEntity* current = bot.enemy >= 1 && bot.enemy <= theLevel.maxclients_ && bot.enemy != attacker ? theLevel.getEntity( bot.enemy ) : nullptr;
	if( !current || !current->client_ || current->health_ <= 0 )
		return eager;
	const Bots::Character& who = Bots::character( bot );
	const float mine = std::clamp( static_cast<float>( ent->health_ ) / std::max( 1, ent->client_->pers_.maxHealth_ ), 0.0f, 1.0f );
	const float theirs = std::clamp( static_cast<float>( current->health_ ) / std::max( 1, current->client_->pers_.maxHealth_ ), 0.0f, 1.0f );
	float invested = 0.0f;
	if( theirs < nearlyDead )
		invested += 0.45f;
	if( Distance( ent->r.currentOrigin, current->r.currentOrigin ) < closeFight )
		invested += 0.25f;
	if( bot.desire == Bots::Goal::Capture || (g_gametype.integer == GT_CTF && current->client_->ps_.objectives) )
		invested += 0.3f;
	invested += (who.patience - 0.5f) * 0.3f;
	// the more hurt, the less it can afford to leave the attacker alone
	const float danger = (1.0f - mine) * (0.6f + 0.6f * who.caution);
	return eager + (1.0f - eager) * std::clamp( invested - danger, 0.0f, 1.0f );
}

namespace
{
	// what a vehicle is good against, as weights on the distance to a target (below one: preferred); a kind it
	// cannot fight at all is no target
	struct TargetPreference
	{
		float	air = 1.0f;	// an aircraft in the air
		float	airHelo = -1.0f;	// a helicopter in the air, when it differs from any aircraft
		float	airBomber = -1.0f;	// a bomber in the air, likewise
		float	ground = 1.0f;	// anything on the ground or water
		float	parked = -1.0f;	// an aircraft on the ground, when it differs from the rest of the ground
		bool	noAir = false;	// does not fight aircraft in the air
		bool	airNeedsMissiles = false;	// fights them only with anti-aircraft missiles aboard
		bool	noSoldiers = false;	// does not go after people on foot
		float	reach = 0.0f;	// fights nothing further than this, when set
	};

	constexpr TargetPreference fighterTargets = { .air = 1.0f, .airHelo = 0.6f, .airBomber = 0.6f, .ground = 1.6f };
	constexpr TargetPreference bomberTargets = { .noAir = true, .noSoldiers = true };
	constexpr TargetPreference heloTargets = { .air = 3.0f, .airHelo = 1.2f };
	constexpr TargetPreference tankTargets = { .air = 1.5f, .parked = 0.5f, .airNeedsMissiles = true };
	constexpr TargetPreference airDefenceTargets = { .ground = 2.0f };
	constexpr TargetPreference boatTargets = {};
	constexpr TargetPreference soldierTargets = { .noAir = true, .reach = 1500.0f };

	const TargetPreference& preferenceOf( const Capabilities& can )
	{
		if( can.runway )
			return can.groundStrike ? bomberTargets : fighterTargets;
		if( can.hovers )
			return heloTargets;
		if( can.armoured )
			return can.antiAirVehicle ? airDefenceTargets : tankTargets;
		if( can.onWater )
			return boatTargets;
		return soldierTargets;
	}

	// the weight on the distance to this target, or below zero when it is no target for this vehicle
	float targetWeight( const TargetPreference& p, const completeVehicleData_t& target, bool air, bool antiAir, float dist )
	{
		if( p.reach > 0.0f && dist > p.reach )
			return -1.0f;
		if( !air )
		{
			if( p.noSoldiers && (target.cat & CAT_LQM) )
				return -1.0f;
			return p.parked >= 0.0f && (target.cat & (CAT_PLANE|CAT_HELO)) ? p.parked : p.ground;
		}
		if( p.noAir || (p.airNeedsMissiles && !antiAir) )
			return -1.0f;
		if( p.airHelo >= 0.0f && (target.cat & CAT_HELO) )
			return p.airHelo;
		if( p.airBomber >= 0.0f && (target.cls & CLASS_PLANE_BOMBER) )
			return p.airBomber;
		return p.air;
	}
}

// The enemy most worth fighting: the nearest, weighted by what this vehicle is good against, by who is shooting at
// it, by the leader's target and by the flag. An enemy out of sight is fought from memory for a while.
GameEntity* BotSenses::findEnemy( GameEntity* ent, Bots::State& bot, const Capabilities& can )
{
	bool harassing[MAX_CLIENTS + 1] = {};
	if( (can.runway || can.hovers) && g_gametype.integer >= GT_TEAM && ((can.runway && !can.groundStrike) || can.gunship) )
		findHarassers( ent, can, harassing );
	const Attention attention = attentionOf( ent, bot );
	GameEntity* best = nullptr;
	float bestScore = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !isOpponent( ent, other ) )
			continue;
		Contact contact;
		contact.harassing = harassing[i];
		if( !locate( ent, bot, other, attention, contact ) )
			continue;
		const float score = targetScore( ent, bot, can, other, contact );
		if( score >= 0.0f && (!best || score < bestScore) )
		{
			best = other;
			bestScore = score;
		}
	}
	return best;
}

// Who is shooting up the team's vehicles on the ground near here: an aircraft with nothing more pressing goes to
// their help. A team mate under fire gives its attacker away, so the aircraft knows where to look
void BotSenses::findHarassers( GameEntity* ent, const Capabilities& can, bool* harassing )
{
	constexpr int underFireTime = 6000;
	constexpr float planeHelpReach = 7000.0f, heloHelpReach = 4500.0f;
	const float reach = can.runway ? planeHelpReach : heloHelpReach;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* mate = theLevel.getEntity( i );
		if( mate == ent || !alive( mate ) || !OnSameTeam( ent, mate ) || mate->client_->vehicle_ < 0 )
			continue;
		if( !(availableVehicles[mate->client_->vehicle_].cat & (CAT_GROUND|CAT_BOAT|CAT_LQM)) )
			continue;
		const int by = mate->client_->lasthurt_client_;
		if( by < 1 || by > MAX_CLIENTS || theLevel.time_ > mate->client_->lastEnemyHitTime_ + underFireTime )
			continue;
		if( Distance( ent->r.currentOrigin, mate->r.currentOrigin ) <= reach )
			harassing[by] = true;
	}
}

// someone this bot could fight: alive, in a vehicle of its own, not on its team, and not passing as one of its team
// in a stolen vehicle still in its paint
bool BotSenses::isOpponent( GameEntity* ent, GameEntity* other )
{
	if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
		return false;
	if( g_gametype.integer < GT_TEAM )
		return true;
	if( OnSameTeam( ent, other ) )
		return false;
	return !(other->client_->sess_.paintTeam_ == ent->client_->sess_.sessionTeam_ && !other->client_->ruseBlown_);
}

// How many contacts the bot is keeping its eyes on, and the furthest of them. A bot keeps track of a few at once;
// a new one past that is noticed only when nearer than the furthest it is tracking
BotSenses::Attention BotSenses::attentionOf( GameEntity* ent, const Bots::State& bot )
{
	constexpr float patientWatcher = 0.6f;
	Attention attention;
	attention.limit = Bots::character( bot ).patience > patientWatcher ? 5 : 3;
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		const Bots::Seen& seen = bot.seen[i];
		if( seen.level != Bots::Awareness::Aware || theLevel.time_ - seen.awareTime >= trackingTime )
			continue;
		attention.tracked++;
		attention.furthest = std::max( attention.furthest, Distance( ent->r.currentOrigin, seen.position ) );
	}
	return attention;
}

// Where an opponent is, as far as this bot knows: seen now, given away by shooting, told by the team, or remembered.
// False when it knows nothing of it. Each contact's awareness rises with what gives it away and fades with time
bool BotSenses::locate( GameEntity* ent, Bots::State& bot, GameEntity* other, const Attention& attention, Contact& contact )
{
	GameClient* client = ent->client_;
	const int i = other->s.number;
	Bots::Seen& seen = bot.seen[i];
	contact.dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
	// being shot gives the shooter away: the bot knows roughly where its attacker is, seen or not
	constexpr int grudgeTime = 8000;
	contact.attackedBy = client->lasthurt_client_ == i && theLevel.time_ < bot.hitTime + grudgeTime;
	if( contact.attackedBy || contact.harassing )
	{
		VectorCopy( other->r.currentOrigin, seen.position );
		seen.time = theLevel.time_;
		seen.level = std::max( seen.level, Bots::Awareness::Suspicious );
	}
	if( inSight( ent, other, contact.dist, Bots::handicap( bot ).sightShare ) && noticed( ent, bot, seen, other->r.currentOrigin, contact.dist, attention ) )
	{
		VectorCopy( other->r.currentOrigin, seen.position );
		seen.time = seen.awareTime = theLevel.time_;
		seen.level = Bots::Awareness::Aware;
		BotTeam::report( client->sess_.sessionTeam_, i, other->r.currentOrigin );
		return true;
	}
	if( seen.level == Bots::Awareness::Aware && theLevel.time_ - seen.awareTime >= trackingTime )
		seen.level = Bots::Awareness::Suspicious;
	// what a team mate saw, once word of it has got round
	vec3_t told;
	int toldTime;
	if( BotTeam::known( client->sess_.sessionTeam_, i, told, toldTime ) && toldTime > seen.time )
	{
		VectorCopy( told, seen.position );
		seen.time = toldTime;
		seen.level = std::max( seen.level, Bots::Awareness::Suspicious );
	}
	if( !seen.time || theLevel.time_ > seen.time + Bots::memoryTime( bot ) )
	{
		seen.level = Bots::Awareness::Unaware;
		return false;
	}
	// remembered, not seen: counts as further away, and is forgotten once the bot gets there and finds nothing
	contact.dist = Distance( ent->r.currentOrigin, seen.position ) * 1.5f;
	if( contact.dist < 300 )
	{
		seen.time = 0;
		seen.level = Bots::Awareness::Unaware;
		return false;
	}
	return true;
}

// within range, not hidden by stealth, and nothing in the way
bool BotSenses::inSight( GameEntity* ent, GameEntity* other, float dist, float sightShare )
{
	const completeVehicleData_t& veh = availableVehicles[ent->client_->vehicle_];
	const bool flyer = (veh.cat & (CAT_PLANE|CAT_HELO)) != 0;
	const float maxRange = (flyer ? planeEnemyRange : surfaceEnemyRange) * sightShare;
	const float radarRange = veh.radarRange > 0 ? static_cast<float>( veh.radarRange ) : 8000.0f;
	GameClient* oc = other->client_;
	return dist <= maxRange && !VehicleData::stealthHidden( oc->vehicle_, oc->ps_.timers[TIMER_REVEAL], theLevel.time_, dist, radarRange ) &&
		BotWorld::visible( ent, ent->r.currentOrigin, other );
}

// In sight is not yet noticed. A contact already watched stays watched; one ahead is noticed; one behind only when the
// bot looks round, which it does now and then, the better pilots more often. Past the few it can keep track of, a new
// one is noticed only when nearer than the furthest it is tracking
bool BotSenses::noticed( GameEntity* ent, const Bots::State& bot, const Bots::Seen& seen, const vec3_t where, float dist, const Attention& attention )
{
	if( seen.level == Bots::Awareness::Aware && theLevel.time_ - seen.awareTime < trackingTime )
		return true;
	if( !ahead( ent, where ) && !lookingRound( ent, bot ) )
		return false;
	return attention.tracked < attention.limit || dist < attention.furthest;
}

// within about a hundred degrees either side of the nose
bool BotSenses::ahead( GameEntity* ent, const vec3_t point )
{
	constexpr float sideLimit = -0.2f;
	vec3_t forward, to;
	AngleVectors( ent->client_->ps_.vehicleAngles, forward, nullptr, nullptr );
	VectorSubtract( point, ent->r.currentOrigin, to );
	VectorNormalize( to );
	return DotProduct( forward, to ) > sideLimit;
}

// a look round behind: part of every few seconds, more often for the more skilled
bool BotSenses::lookingRound( GameEntity* ent, const Bots::State& bot )
{
	constexpr int lookFor = 700;
	const int period = Bots::handicap( bot ).scanPeriod;
	return (theLevel.time_ + ent->s.number * 397) % period < lookFor;
}

// how much this opponent is worth fighting, as a distance shortened by everything that makes it the one to go for;
// below zero when this vehicle cannot fight it
float BotSenses::targetScore( GameEntity* ent, Bots::State& bot, const Capabilities& can, GameEntity* other, const Contact& contact )
{
	GameClient* client = ent->client_;
	GameClient* oc = other->client_;
	const int i = other->s.number;
	const completeVehicleData_t& ov = availableVehicles[oc->vehicle_];
	const bool air = (ov.cat & (CAT_PLANE|CAT_HELO)) != 0 && !(oc->ps_.ONOFF & OO_LANDED);
	const bool antiAir = BotWeapons::hasWeaponType( client->vehicle_, client->ps_, WT_ANTIAIRMISSILE );
	const float preference = targetWeight( preferenceOf( can ), ov, air, antiAir, contact.dist );
	if( preference < 0.0f )
		return -1.0f;
	GameEntity* leader = BotSenses::leaderOf( bot );
	const int leaderTarget = leader ? BotSenses::targetOf( leader ) : -1;
	const BotTeam::Assignment& teamJob = BotTeam::of( ent->s.number );
	const bool attackedBy = contact.attackedBy;
	float score = contact.dist * preference;
	// stay with the current target unless another is clearly better
	if( i == bot.enemy )
		score *= 0.7f;
	// whoever is shooting at it pulls hard, unless that means giving up a fight that is nearly won
	if( attackedBy )
		score *= BotSenses::turnOnAttacker( ent, bot, i );
	// fight what the leader is fighting
	if( leaderTarget == i )
		score *= 0.35f;
	// whoever is shooting up the team's vehicles on the ground nearby: the better the team player, the sooner
	// it drops what it was doing for them
	if( contact.harassing )
		score *= 0.7f - 0.4f * Bots::character( bot ).teamwork;
	// the team's plan: the enemy it was sent for; for a guard or an escort, whoever is near what it guards and
	// nobody far from it; for air cover, aircraft over the place it covers
	if( teamJob.job != BotTeam::Job::None )
	{
		constexpr float guardedNear = 2500.0f, guardedFar = 5000.0f;
		if( teamJob.target == i )
			score *= 0.3f;
		if( teamJob.hasPlace && (teamJob.job == BotTeam::Job::Defend || teamJob.job == BotTeam::Job::Escort || teamJob.job == BotTeam::Job::AirCover) )
		{
			const float from = Distance( other->r.currentOrigin, teamJob.place );
			if( from < guardedNear && (teamJob.job != BotTeam::Job::AirCover || air) )
				score *= 0.5f;
			else if( from > guardedFar )
				score *= 2.0f;
		}
	}
	// whoever carries our flag comes before everyone
	if( g_gametype.integer == GT_CTF && (oc->ps_.objectives & (client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? OB_REDFLAG : OB_BLUEFLAG)) )
		score *= 0.15f;
	return score;
}

// everything a bot knows this frame: its enemy and where to aim, its own state, missiles, aircraft about, and its goal
void BotSenses::sense( GameEntity* ent, Bots::State& bot, Bots::Sense& out )
{
	const Capabilities can = Capabilities::of( ent->client_->vehicle_ );
	trackEnemy( ent, bot, can, out );
	if( out.enemy )
		aimAt( ent, bot, can, out );
	out.height = BotWorld::heightAboveGround( ent->r.currentOrigin, ent->s.number );
	recordTrail( ent, bot );
	ownState( ent, can, out );
	noticeHits( ent, bot, out );
	if( out.launched )
		findMissile( ent, out );
	findAirThreat( ent, out );
	out.duel = out.enemy && out.enemyAir && foesNear( ent ) == 1 && ((out.can.runway && !out.can.groundStrike) || out.can.gunship);
	out.outranged = outranged( ent, out );
	reviewGoal( ent, bot, out );
}

// the enemy to fight, looked for now and then; with none of its own it takes its leader's. One out of sight is kept
// while it is remembered
void BotSenses::trackEnemy( GameEntity* ent, Bots::State& bot, const Capabilities& can, Bots::Sense& out )
{
	if( theLevel.time_ >= bot.enemyTime )
	{
		GameEntity* enemy = BotSenses::findEnemy( ent, bot, can );
		bot.enemy = enemy ? enemy->s.number : -1;
		// with no enemy of its own it takes its leader's
		if( !enemy && bot.leader >= 1 && Bots::isBot( bot.leader ) && Bots::states_[bot.leader].enemy >= 0 )
			bot.enemy = Bots::states_[bot.leader].enemy;
		bot.enemyTime = theLevel.time_ + (700 + (rand() % 300)) * ThinkRate::slowdown( ent, bot );
	}
	if( bot.enemy >= 0 )
	{
		GameEntity* enemy = theLevel.getEntity( bot.enemy );
		if( alive( enemy ) )
			out.enemy = enemy;
		else
			bot.enemy = -1;
	}
	if( out.enemy )
	{
		Bots::Seen& seen = bot.seen[out.enemy->s.number];
		out.enemyVisible = BotWorld::visible( ent, ent->r.currentOrigin, out.enemy );
		if( out.enemyVisible && seen.time )
		{
			VectorCopy( out.enemy->r.currentOrigin, seen.position );
			seen.time = theLevel.time_;
		}
		else if( !seen.time || theLevel.time_ > seen.time + Bots::memoryTime( bot ) )
		{
			out.enemy = nullptr;
			bot.enemy = -1;
			bot.enemyTime = 0;
		}
	}
}

// Where to aim and fly: ahead of a moving enemy by the flight time of the gun's shot, or to the side of a turreted
// bomber where its guns bear least, or the bearing the team gave, or behind and below an aircraft that is not hunting it
void BotSenses::aimAt( GameEntity* ent, Bots::State& bot, const Capabilities& can, Bots::Sense& out )
{
	const bool fighter = can.runway && !can.groundStrike;
	Vec3 target;
	VectorCopy( out.enemyVisible ? out.enemy->r.currentOrigin : bot.seen[out.enemy->s.number].position, target );
	const float dist = Distance( ent->r.currentOrigin, target );
	const int weapon = availableVehicles[ent->client_->vehicle_].weapons[0];
	const float shotSpeed = weapon && availableWeapons[weapon].muzzleVelocity > 0 ? static_cast<float>( availableWeapons[weapon].muzzleVelocity ) : 2500.0f;
	if( out.enemyVisible )
		target = Intercept::lead( ent->r.currentOrigin, target, out.enemy->client_->ps_.velocity, shotSpeed, 1.5f );
	if( !turretFlank( ent, bot, out, fighter, dist, target ) && !teamFlank( ent, out, dist, target ) )
		cunningApproach( ent, bot, out, fighter, dist, target );
	VectorSubtract( target, ent->r.currentOrigin, out.enemyDir );
	out.enemyDist = VectorNormalize( out.enemyDir );
	vectoangles( out.enemyDir, out.enemyAngles );
	out.enemyAngles[PITCH] = AngleSubtract( out.enemyAngles[PITCH], 0 );
	out.enemyAir = (availableVehicles[out.enemy->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) != 0 &&
		!(out.enemy->client_->ps_.ONOFF & OO_LANDED);
}

// An aircraft with gun turrets is approached from where the fewest of them can bear: each turret only covers
// its own field. That choice is made out of gun range, where it costs nothing: the fighter flies for a point
// at gun range on the thinnest side. Inside gun range it does what a person does, which is put the nose on
// the bomber and shoot; a fighter that goes on circling for a better side there is only a target.
// Pilots with little cunning do not think of the approach at all
bool BotSenses::turretFlank( GameEntity* ent, const Bots::State& bot, const Bots::Sense& out, bool fighter, float dist, Vec3& target )
{
	if( !(fighter && (availableVehicles[out.enemy->client_->vehicle_].flags & VF_GUNSTATIONS) &&
		out.enemyVisible && dist > gunRange && Bots::character( bot ).cunning > 0.25f) )
		return false;
	Vec3 side;
	if( !thinnestSide( ent, out.enemy, side ) )
		return false;
	target = Vec3( out.enemy->r.currentOrigin ) + side * gunRange;
	return true;
}

// the direction from a turreted aircraft on which fewer of its gun stations bear than on where this one is now; of
// the sides equally thinly covered, the one nearest to where it already is. Level first: a turreted bomber's blind
// side is level (vectoangles pitch: positive is below the bomber)
bool BotSenses::thinnestSide( GameEntity* ent, GameEntity* bomber, Vec3& side )
{
	const int theirs = bomber->client_->vehicle_;
	const float* hull = bomber->client_->ps_.vehicleAngles;
	Vec3 toMe = Vec3( ent->r.currentOrigin ) - Vec3( bomber->r.currentOrigin ), myBearing;
	vectoangles( toMe, myBearing );
	const int onMe = std::popcount( VehicleData::gunStationsBearing( theirs, hull, myBearing ) );
	int fewest = onMe;
	float nearest = 0;
	for( float pitch : { 0.0f, -20.0f, 20.0f, -50.0f, 50.0f } )
	{
		for( float yaw = 0.0f; yaw < 360.0f; yaw += 30.0f )
		{
			Vec3 angles = { pitch, hull[YAW] + yaw, 0 }, dir;
			const int bearing = std::popcount( VehicleData::gunStationsBearing( theirs, hull, angles ) );
			AngleVectors( angles, dir, nullptr, nullptr );
			const float along = DotProduct( dir, toMe );
			if( bearing > fewest || (bearing == fewest && (bearing >= onMe || along <= nearest)) )
				continue;
			fewest = bearing;
			nearest = along;
			side = dir;
		}
	}
	return fewest < onMe;
}

// Several of the team on one enemy: each comes in along its own bearing and height, and where they are to go
// in together it waits at its place until the others are at theirs
bool BotSenses::teamFlank( GameEntity* ent, const Bots::Sense& out, float dist, Vec3& target )
{
	const BotTeam::Assignment& teamJob = BotTeam::of( ent->s.number );
	if( !teamJob.flank || !out.enemyVisible )
		return false;
	{
		const bool flyer = (availableVehicles[ent->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) != 0;
		const float standOff = flyer ? gunRange : 900.0f;
		if( dist > standOff * 1.4f || theLevel.time_ < teamJob.goTime )
		{
			Vec3 side;
			const Vec3 bearing( 0, teamJob.bearing, 0 );
			AngleVectors( bearing, side, nullptr, nullptr );
			target = Vec3( out.enemy->r.currentOrigin ) + side * standOff;
			if( flyer )
				target[2] += teamJob.lift;
			return true;
		}
	}
	return false;
}

// a cunning fighter goes for the place behind and below an aircraft that is not hunting it
void BotSenses::cunningApproach( GameEntity* ent, const Bots::State& bot, const Bots::Sense& out, bool fighter, float dist, Vec3& target )
{
	const unsigned int enemyCat = availableVehicles[out.enemy->client_->vehicle_].cat;
	if( fighter && (enemyCat & (CAT_PLANE|CAT_HELO)) && dist > 1200 && dist < 6000 &&
		Bots::character( bot ).cunning > 0.5f && out.enemy->tracktarget_ != ent )
	{
		Vec3 behind;
		AngleVectors( out.enemy->client_->ps_.vehicleAngles, behind, nullptr, nullptr );
		target = target + behind * ( -700.0f );
		target[2] -= 250.0f * Bots::character( bot ).cunning;
	}
}

// where it was over the last six seconds, for the crash record
void BotSenses::recordTrail( GameEntity* ent, Bots::State& bot )
{
	if( theLevel.time_ < bot.trailTime )
		return;
	for( int i = 5; i > 0; i-- )
		VectorCopy( bot.trail[i - 1], bot.trail[i] );
	VectorCopy( ent->r.currentOrigin, bot.trail[0] );
	bot.trailTime = theLevel.time_ + 1000;
}

// its own vehicle: what it can do, how hurt, locked on or launched at, stealthy and whether showing
void BotSenses::ownState( GameEntity* ent, const Capabilities& can, Bots::Sense& out )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	out.role = BotSenses::roleOf( client->vehicle_ );
	out.can = can;
	out.health = std::clamp( static_cast<float>( ent->health_ ) / std::max( 1, client->pers_.maxHealth_ ), 0.0f, 1.0f );
	out.locked = (ps.stats[STAT_LOCKINFO] & (LI_BEING_LOCKED|LI_BEING_LAUNCHED)) != 0;
	out.launched = (ps.stats[STAT_LOCKINFO] & LI_BEING_LAUNCHED) != 0;
	out.stealth = (availableVehicles[client->vehicle_].flags & VF_STEALTH) != 0;
	out.revealed = ps.timers[TIMER_REVEAL] > theLevel.time_;
}

// hit: remember when, and which way to break if this is the first hit in a while
void BotSenses::noticeHits( GameEntity* ent, Bots::State& bot, Bots::Sense& out )
{
	const playerState_t& ps = ent->client_->ps_;
	if( ent->health_ < bot.lastHealth - 2 )
	{
		if( theLevel.time_ > bot.hitTime + 4000 )
			bot.breakYaw = ps.vehicleAngles[YAW] + (rand() & 1 ? 80.0f : -80.0f);
		bot.hitTime = theLevel.time_;
	}
	bot.lastHealth = ent->health_;
	out.underFire = bot.hitTime && theLevel.time_ < bot.hitTime + 2500;
	if( out.underFire )
	{
		// blindsided: shot at by someone it is not looking towards
		bool seen = false;
		if( out.enemy && out.enemyVisible )
		{
			Vec3 forward;
			AngleVectors( ps.viewangles, forward, nullptr, nullptr );
			seen = DotProduct( forward, out.enemyDir ) > 0.3f || ent->tracktarget_ == out.enemy;
		}
		out.blindsided = !seen;
	}
}

// the missile that will arrive first
void BotSenses::findMissile( GameEntity* ent, Bots::Sense& out )
{
	const playerState_t& ps = ent->client_->ps_;
	for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !other || !other->inuse_ || other->s.eType != ET_MISSILE || other->tracktarget_ != ent )
			continue;
		Vec3 rel;
		rel = other->r.currentOrigin - ent->r.currentOrigin;
		const float dist = VectorLength( rel );
		if( out.missile && dist > out.missileDist )
			continue;
		Vec3 relVel, angles;
		VectorSubtract( other->s.pos.trDelta, ps.velocity, relVel );
		const float closing = -DotProduct( rel, relVel ) / std::max( dist, 1.0f );
		vectoangles( rel, angles );
		out.missile = other;
		out.missileDist = dist;
		out.missileTime = closing > 1 ? dist / closing : 99.0f;
		out.missileYaw = angles[YAW];
	}
}

// the nearest enemy aircraft in the air: what surface vehicles hide from
void BotSenses::findAirThreat( GameEntity* ent, Bots::Sense& out )
{
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
			continue;
		if( !(availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) || (other->client_->ps_.ONOFF & OO_LANDED) )
			continue;
		const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		if( dist < 6000 && (!out.airThreat || dist < out.airThreatDist) )
		{
			out.airThreat = other;
			out.airThreatDist = dist;
		}
	}
}

// enemies within five thousand units: one alone in the air is a duel
int BotSenses::foesNear( GameEntity* ent )
{
	int foes = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
			continue;
		if( Distance( ent->r.currentOrigin, other->r.currentOrigin ) < 5000 )
			foes++;
	}
	return foes;
}

// the goal is reviewed once a second, but a goal just taken is kept for a few seconds unless something forces the matter
void BotSenses::reviewGoal( GameEntity* ent, Bots::State& bot, const Bots::Sense& out )
{
	GameClient* client = ent->client_;
	if( theLevel.time_ >= bot.goalEvalTime && (theLevel.time_ >= bot.commitTime || out.launched || out.underFire || bot.desire == Bots::Goal::Roam) )
	{
		const Bots::Goal previous = bot.desire;
		bot.desire = Bots::chooseGoal( ent, bot, out );
#ifdef MF_BOT_DEBUG
		if( g_botDebug.integer >= 2 && bot.desire != previous )
			Common::printf( "BOTGOAL t %d bot %d veh %d goal %d -> %d health %.2f enemy %d dist %.0f visible %d locked %d launched %d underfire %d blind %d threat %.0f\n",
				theLevel.time_, ent->s.number, client->vehicle_, static_cast<int>( previous ), static_cast<int>( bot.desire ), out.health, bot.enemy,
				out.enemyDist, out.enemyVisible, out.locked, out.launched, out.underFire, out.blindsided, out.airThreat ? out.airThreatDist : -1.0f );
#endif
		bot.goalEvalTime = theLevel.time_ + Bots::handicap( bot ).reconsiderEvery * ThinkRate::slowdown( ent, bot );
		if( bot.desire != previous )
			bot.commitTime = theLevel.time_ + 3000 + static_cast<int>( 3000 * Bots::character( bot ).patience );
	}
}

// the enemy's weapons reach this vehicle where its own do not reach back: a fight to get out of, or to close
bool BotSenses::outranged( GameEntity* ent, const Bots::Sense& out )
{
	if( !out.enemy || !out.enemy->client_ || out.enemy->client_->vehicle_ < 0 )
		return false;
	const bool selfAir = (out.can.runway || out.can.hovers) && !(ent->client_->ps_.ONOFF & OO_LANDED);
	const float theirs = Envelope::reachOf( out.enemy->client_->vehicle_, out.enemy->client_->ps_.ammo, selfAir );
	const float ours = Envelope::reachOf( ent->client_->vehicle_, ent->client_->ps_.ammo, out.enemyAir );
	return out.enemyDist <= theirs && out.enemyDist > ours;
}
