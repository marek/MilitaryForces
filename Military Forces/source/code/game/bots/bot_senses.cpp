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

GameEntity* Bots::findEnemy( GameEntity* ent, State& bot, Role role )
{
	GameClient* client = ent->client_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const bool flyer = (veh.cat & (CAT_PLANE|CAT_HELO)) != 0;
	const float maxRange = flyer ? planeEnemyRange : surfaceEnemyRange;
	const float radarRange = veh.radarRange > 0 ? static_cast<float>( veh.radarRange ) : 8000.0f;
	const bool antiAir = hasWeaponType( client->vehicle_, client->ps_, WT_ANTIAIRMISSILE );

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
		if( g_botDebug.integer >= 2 && bot.desire != previous )
			Common::printf( "BOTGOAL t %d bot %d veh %d goal %d -> %d health %.2f enemy %d dist %.0f visible %d locked %d launched %d underfire %d blind %d threat %.0f\n",
				theLevel.time_, ent->s.number, client->vehicle_, static_cast<int>( previous ), static_cast<int>( bot.desire ), out.health, bot.enemy,
				out.enemyDist, out.enemyVisible, out.locked, out.launched, out.underFire, out.blindsided, out.airThreat ? out.airThreatDist : -1.0f );
		bot.goalEvalTime = theLevel.time_ + 1000;
		if( bot.desire != previous )
			bot.commitTime = theLevel.time_ + 3000 + static_cast<int>( 3000 * character( bot ).patience );
	}
}
