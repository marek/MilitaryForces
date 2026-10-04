#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

Bots::Role Bots::roleOf( int vehicle )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	if( veh.cat & CAT_PLANE )
		return (veh.cls & CLASS_PLANE_BOMBER) ? Role::Bomber : Role::Fighter;
	if( veh.cat & CAT_HELO )
		return (veh.cls & CLASS_HELO_TRANSPORT) ? Role::TransportHelo : Role::AttackHelo;
	if( veh.cat & CAT_GROUND )
		return (veh.cls & CLASS_GROUND_SAM) ? Role::AirDefence : Role::Tank;
	if( veh.cat & CAT_BOAT )
		return Role::Boat;
	return Role::Infantry;
}

int Bots::ridersOf( GameEntity* ent )
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
GameEntity* Bots::walkerNear( GameEntity* ent, State& bot, float range )
{
	constexpr int offerTime = 15000;
	constexpr int restTime = 25000;
	constexpr float humanPreference = 0.3f;
	if( g_gametype.integer < GT_TEAM || theLevel.time_ < bot.liftGiveUp )
		return nullptr;
	if( ridersOf( ent ) >= availableVehicles[ent->client_->vehicle_].seats )
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
		const float weighted = isBot( i ) ? dist : dist * humanPreference;
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

GameEntity* Bots::leaderOf( const State& bot )
{
	GameEntity* leader = bot.leader >= 0 ? theLevel.getEntity( bot.leader ) : nullptr;
	return alive( leader ) ? leader : nullptr;
}

// who a leader is fighting: a bot's chosen enemy, a real player's locked target
int Bots::targetOf( GameEntity* leader )
{
	if( isBot( leader->s.number ) )
		return states_[leader->s.number].enemy;
	return leader->tracktarget_ && leader->tracktarget_->client_ ? leader->tracktarget_->s.number : -1;
}

// A team player picks someone to fight alongside: a real player first, otherwise a bot of the same kind that is
// not following anyone itself. The lower client number leads, so two bots never follow each other.
void Bots::chooseLeader( GameEntity* ent, State& bot )
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
		const bool human = !isBot( i );
		const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		if( human )
		{
			// keep up with a real player in the same element: air with air, surface with surface
			if( flyer != ((cat & (CAT_PLANE|CAT_HELO)) != 0) || dist > humanRange )
				continue;
		}
		else if( !(cat & ownCat) || i > ent->s.number || states_[i].leader >= 0 || dist > botRange )
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
float Bots::turnOnAttacker( GameEntity* ent, const State& bot, int attacker )
{
	constexpr float eager = 0.4f, closeFight = 1500.0f, nearlyDead = 0.4f;
	GameEntity* current = bot.enemy >= 1 && bot.enemy <= theLevel.maxclients_ && bot.enemy != attacker ? theLevel.getEntity( bot.enemy ) : nullptr;
	if( !current || !current->client_ || current->health_ <= 0 )
		return eager;
	const Character& who = character( bot );
	const float mine = std::clamp( static_cast<float>( ent->health_ ) / std::max( 1, ent->client_->pers_.maxHealth_ ), 0.0f, 1.0f );
	const float theirs = std::clamp( static_cast<float>( current->health_ ) / std::max( 1, current->client_->pers_.maxHealth_ ), 0.0f, 1.0f );
	float invested = 0.0f;
	if( theirs < nearlyDead )
		invested += 0.45f;
	if( Distance( ent->r.currentOrigin, current->r.currentOrigin ) < closeFight )
		invested += 0.25f;
	if( bot.desire == Goal::Capture || (g_gametype.integer == GT_CTF && current->client_->ps_.objectives) )
		invested += 0.3f;
	invested += (who.patience - 0.5f) * 0.3f;
	// the more hurt, the less it can afford to leave the attacker alone
	const float danger = (1.0f - mine) * (0.6f + 0.6f * who.caution);
	return eager + (1.0f - eager) * std::clamp( invested - danger, 0.0f, 1.0f );
}

GameEntity* Bots::findEnemy( GameEntity* ent, State& bot, Role role )
{
	GameClient* client = ent->client_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const bool flyer = (veh.cat & (CAT_PLANE|CAT_HELO)) != 0;
	const float maxRange = flyer ? planeEnemyRange : surfaceEnemyRange;
	const float radarRange = veh.radarRange > 0 ? static_cast<float>( veh.radarRange ) : 8000.0f;
	const bool antiAir = hasWeaponType( client->vehicle_, client->ps_, WT_ANTIAIRMISSILE );

	GameEntity* leader = leaderOf( bot );
	const int leaderTarget = leader ? targetOf( leader ) : -1;

	GameEntity* best = nullptr;
	float bestScore = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) )
			continue;
		GameClient* oc = other->client_;
		if( oc->sess_.ridingClient_ >= 0 )
			continue;
		if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
			continue;
		if( g_gametype.integer >= GT_TEAM && oc->sess_.paintTeam_ == client->sess_.sessionTeam_ && !oc->ruseBlown_ )
			continue;

		float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		Seen& seen = bot.seen[i];
		// being shot gives the shooter away: the bot knows roughly where its attacker is, seen or not
		constexpr int grudgeTime = 8000;
		const bool attackedBy = client->lasthurt_client_ == i && theLevel.time_ < bot.hitTime + grudgeTime;
		if( attackedBy )
		{
			VectorCopy( other->r.currentOrigin, seen.position );
			seen.time = theLevel.time_;
		}
		const bool sees = dist <= maxRange &&
			!VehicleData::stealthHidden( oc->vehicle_, oc->ps_.timers[TIMER_REVEAL], theLevel.time_, dist, radarRange ) &&
			visible( ent, ent->r.currentOrigin, other );
		if( sees )
		{
			VectorCopy( other->r.currentOrigin, seen.position );
			seen.time = theLevel.time_;
		}
		else
		{
			if( !seen.time || theLevel.time_ > seen.time + memoryTime( bot ) )
				continue;
			dist = Distance( ent->r.currentOrigin, seen.position ) * 1.5f;
			if( dist < 300 )
			{
				seen.time = 0;
				continue;
			}
		}

		const completeVehicleData_t& ov = availableVehicles[oc->vehicle_];
		const bool air = (ov.cat & (CAT_PLANE|CAT_HELO)) != 0 && !(oc->ps_.ONOFF & OO_LANDED);
		float score = dist;
		switch( role )
		{
		case Role::Fighter:
			if( !air )
				score *= 1.6f;
			else if( (ov.cat & CAT_HELO) || (ov.cls & CLASS_PLANE_BOMBER) )
				score *= 0.6f;
			break;
		case Role::Bomber:
			if( air || (ov.cat & CAT_LQM) )
				continue;
			break;
		case Role::AttackHelo:
		case Role::TransportHelo:
			if( air )
				score *= (ov.cat & CAT_HELO) ? 1.2f : 3.0f;
			break;
		case Role::Tank:
			if( air && !antiAir )
				continue;
			if( air )
				score *= 1.5f;
			// an aircraft sitting on a runway is the best target a tank gun can have
			else if( ov.cat & (CAT_PLANE|CAT_HELO) )
				score *= 0.5f;
			break;
		case Role::AirDefence:
			if( !air )
				score *= 2.0f;
			break;
		case Role::Boat:
			break;
		case Role::Infantry:
			if( dist > 1500 || air )
				continue;
			break;
		}
		if( i == bot.enemy )
			score *= 0.7f;
		// whoever is shooting at it pulls hard, unless that means giving up a fight that is nearly won
		if( attackedBy )
			score *= turnOnAttacker( ent, bot, i );
		// fight what the leader is fighting
		if( leaderTarget == i )
			score *= 0.35f;
		if( g_gametype.integer == GT_CTF && (oc->ps_.objectives & (client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? OB_REDFLAG : OB_BLUEFLAG)) )
			score *= 0.15f;
		if( !best || score < bestScore )
		{
			best = other;
			bestScore = score;
		}
	}
	return best;
}

void Bots::sense( GameEntity* ent, State& bot, Sense& out )
{
	if( theLevel.time_ >= bot.enemyTime )
	{
		GameEntity* enemy = findEnemy( ent, bot, roleOf( ent->client_->vehicle_ ) );
		bot.enemy = enemy ? enemy->s.number : -1;
		if( !enemy && bot.leader >= 1 && isBot( bot.leader ) && states_[bot.leader].enemy >= 0 )
			bot.enemy = states_[bot.leader].enemy;
		bot.enemyTime = theLevel.time_ + 700 + (rand() % 300);
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
		Seen& seen = bot.seen[out.enemy->s.number];
		out.enemyVisible = visible( ent, ent->r.currentOrigin, out.enemy );
		if( out.enemyVisible && seen.time )
		{
			VectorCopy( out.enemy->r.currentOrigin, seen.position );
			seen.time = theLevel.time_;
		}
		else if( !seen.time || theLevel.time_ > seen.time + memoryTime( bot ) )
		{
			out.enemy = nullptr;
			bot.enemy = -1;
			bot.enemyTime = 0;
		}
	}
	if( out.enemy )
	{
		vec3_t target;
		VectorCopy( out.enemyVisible ? out.enemy->r.currentOrigin : bot.seen[out.enemy->s.number].position, target );
		const float dist = Distance( ent->r.currentOrigin, target );
		const int weapon = availableVehicles[ent->client_->vehicle_].weapons[0];
		const float shotSpeed = weapon && availableWeapons[weapon].muzzleVelocity > 0 ? static_cast<float>( availableWeapons[weapon].muzzleVelocity ) : 2500.0f;
		if( out.enemyVisible )
			VectorMA( target, std::min( dist / shotSpeed, 1.5f ), out.enemy->client_->ps_.velocity, target );
		const unsigned int enemyCat = availableVehicles[out.enemy->client_->vehicle_].cat;
		if( roleOf( ent->client_->vehicle_ ) == Role::Fighter && (enemyCat & (CAT_PLANE|CAT_HELO)) && dist > 1200 && dist < 6000 &&
			character( bot ).cunning > 0.5f && out.enemy->tracktarget_ != ent )
		{
			vec3_t behind;
			AngleVectors( out.enemy->client_->ps_.vehicleAngles, behind, nullptr, nullptr );
			VectorMA( target, -700.0f, behind, target );
			target[2] -= 250.0f * character( bot ).cunning;
		}
		VectorSubtract( target, ent->r.currentOrigin, out.enemyDir );
		out.enemyDist = VectorNormalize( out.enemyDir );
		vectoangles( out.enemyDir, out.enemyAngles );
		out.enemyAngles[PITCH] = AngleSubtract( out.enemyAngles[PITCH], 0 );
		out.enemyAir = (availableVehicles[out.enemy->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) != 0 &&
			!(out.enemy->client_->ps_.ONOFF & OO_LANDED);
	}
	out.height = heightAboveGround( ent->r.currentOrigin, ent->s.number );
	if( theLevel.time_ >= bot.trailTime )
	{
		for( int i = 5; i > 0; i-- )
			VectorCopy( bot.trail[i - 1], bot.trail[i] );
		VectorCopy( ent->r.currentOrigin, bot.trail[0] );
		bot.trailTime = theLevel.time_ + 1000;
	}

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	out.role = roleOf( client->vehicle_ );
	out.health = std::clamp( static_cast<float>( ent->health_ ) / std::max( 1, client->pers_.maxHealth_ ), 0.0f, 1.0f );
	out.locked = (ps.stats[STAT_LOCKINFO] & (LI_BEING_LOCKED|LI_BEING_LAUNCHED)) != 0;
	out.launched = (ps.stats[STAT_LOCKINFO] & LI_BEING_LAUNCHED) != 0;
	out.stealth = (availableVehicles[client->vehicle_].flags & VF_STEALTH) != 0;
	out.revealed = ps.timers[TIMER_REVEAL] > theLevel.time_;

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
		bool seen = false;
		if( out.enemy && out.enemyVisible )
		{
			vec3_t forward;
			AngleVectors( ps.viewangles, forward, nullptr, nullptr );
			seen = DotProduct( forward, out.enemyDir ) > 0.3f || ent->tracktarget_ == out.enemy;
		}
		out.blindsided = !seen;
	}

	if( out.launched )
	{
		for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( !other || !other->inuse_ || other->s.eType != ET_MISSILE || other->tracktarget_ != ent )
				continue;
			vec3_t rel;
			VectorSubtract( other->r.currentOrigin, ent->r.currentOrigin, rel );
			const float dist = VectorLength( rel );
			if( out.missile && dist > out.missileDist )
				continue;
			vec3_t relVel, angles;
			VectorSubtract( other->s.pos.trDelta, ps.velocity, relVel );
			const float closing = -DotProduct( rel, relVel ) / std::max( dist, 1.0f );
			vectoangles( rel, angles );
			out.missile = other;
			out.missileDist = dist;
			out.missileTime = closing > 1 ? dist / closing : 99.0f;
			out.missileYaw = angles[YAW];
		}
	}

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
	out.duel = out.enemy && out.enemyAir && foes == 1 && (out.role == Role::Fighter || out.role == Role::AttackHelo);

	if( theLevel.time_ >= bot.goalEvalTime && (theLevel.time_ >= bot.commitTime || out.launched || out.underFire || bot.desire == Goal::Roam) )
	{
		const Goal previous = bot.desire;
		bot.desire = chooseGoal( ent, bot, out );
#ifdef MF_BOT_DEBUG
		if( g_botDebug.integer >= 2 && bot.desire != previous )
			Common::printf( "BOTGOAL t %d bot %d veh %d goal %d -> %d health %.2f enemy %d dist %.0f visible %d locked %d launched %d underfire %d blind %d threat %.0f\n",
				theLevel.time_, ent->s.number, client->vehicle_, static_cast<int>( previous ), static_cast<int>( bot.desire ), out.health, bot.enemy,
				out.enemyDist, out.enemyVisible, out.locked, out.launched, out.underFire, out.blindsided, out.airThreat ? out.airThreatDist : -1.0f );
#endif
		bot.goalEvalTime = theLevel.time_ + 1000;
		if( bot.desire != previous )
			bot.commitTime = theLevel.time_ + 3000 + static_cast<int>( 3000 * character( bot ).patience );
	}
}
