#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
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

// The enemy most worth fighting: the nearest, weighted by what this vehicle is good against, by who is shooting at
// it, by the leader's target and by the flag. An enemy out of sight is fought from memory for a while.
GameEntity* BotSenses::findEnemy( GameEntity* ent, Bots::State& bot, Bots::Role role )
{
	GameClient* client = ent->client_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const bool flyer = (veh.cat & (CAT_PLANE|CAT_HELO)) != 0;
	const float maxRange = flyer ? planeEnemyRange : surfaceEnemyRange;
	const float radarRange = veh.radarRange > 0 ? static_cast<float>( veh.radarRange ) : 8000.0f;
	const bool antiAir = BotWeapons::hasWeaponType( client->vehicle_, client->ps_, WT_ANTIAIRMISSILE );

	GameEntity* leader = BotSenses::leaderOf( bot );
	const int leaderTarget = leader ? BotSenses::targetOf( leader ) : -1;

	// Who is shooting up the team's vehicles on the ground near here: an aircraft with nothing more pressing goes
	// to their help. A team mate under fire gives its attacker away, so the aircraft knows where to look
	constexpr int underFireTime = 6000;
	constexpr float planeHelpReach = 7000.0f, heloHelpReach = 4500.0f;
	bool harassing[MAX_CLIENTS + 1] = {};
	if( flyer && g_gametype.integer >= GT_TEAM && ( role == Bots::Role::Fighter || role == Bots::Role::AttackHelo ) )
	{
		const float reach = (veh.cat & CAT_PLANE) ? planeHelpReach : heloHelpReach;
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
		// a stolen vehicle still in our paint passes as one of ours until it gives itself away
		if( g_gametype.integer >= GT_TEAM && oc->sess_.paintTeam_ == client->sess_.sessionTeam_ && !oc->ruseBlown_ )
			continue;

		float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		Bots::Seen& seen = bot.seen[i];
		// being shot gives the shooter away: the bot knows roughly where its attacker is, seen or not
		constexpr int grudgeTime = 8000;
		const bool attackedBy = client->lasthurt_client_ == i && theLevel.time_ < bot.hitTime + grudgeTime;
		if( attackedBy || harassing[i] )
		{
			VectorCopy( other->r.currentOrigin, seen.position );
			seen.time = theLevel.time_;
		}
		const bool sees = dist <= maxRange &&
			!VehicleData::stealthHidden( oc->vehicle_, oc->ps_.timers[TIMER_REVEAL], theLevel.time_, dist, radarRange ) &&
			BotWorld::visible( ent, ent->r.currentOrigin, other );
		if( sees )
		{
			VectorCopy( other->r.currentOrigin, seen.position );
			seen.time = theLevel.time_;
		}
		else
		{
			if( !seen.time || theLevel.time_ > seen.time + Bots::memoryTime( bot ) )
				continue;
			// remembered, not seen: counts as further away, and is forgotten once the bot gets there and finds nothing
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
		case Bots::Role::Fighter:
			if( !air )
				score *= 1.6f;
			else if( (ov.cat & CAT_HELO) || (ov.cls & CLASS_PLANE_BOMBER) )
				score *= 0.6f;
			break;
		case Bots::Role::Bomber:
			if( air || (ov.cat & CAT_LQM) )
				continue;
			break;
		case Bots::Role::AttackHelo:
		case Bots::Role::TransportHelo:
			if( air )
				score *= (ov.cat & CAT_HELO) ? 1.2f : 3.0f;
			break;
		case Bots::Role::Tank:
			if( air && !antiAir )
				continue;
			if( air )
				score *= 1.5f;
			// an aircraft sitting on a runway is the best target a tank gun can have
			else if( ov.cat & (CAT_PLANE|CAT_HELO) )
				score *= 0.5f;
			break;
		case Bots::Role::AirDefence:
			if( !air )
				score *= 2.0f;
			break;
		case Bots::Role::Boat:
			break;
		case Bots::Role::Infantry:
			if( dist > 1500 || air )
				continue;
			break;
		}
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
		if( harassing[i] )
			score *= 0.7f - 0.4f * Bots::character( bot ).teamwork;
		// whoever carries our flag comes before everyone
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

// everything a bot knows this frame: its enemy and where to aim, its own state, missiles, aircraft about, and its goal
void BotSenses::sense( GameEntity* ent, Bots::State& bot, Bots::Sense& out )
{
	if( theLevel.time_ >= bot.enemyTime )
	{
		GameEntity* enemy = BotSenses::findEnemy( ent, bot, BotSenses::roleOf( ent->client_->vehicle_ ) );
		bot.enemy = enemy ? enemy->s.number : -1;
		// with no enemy of its own it takes its leader's
		if( !enemy && bot.leader >= 1 && Bots::isBot( bot.leader ) && Bots::states_[bot.leader].enemy >= 0 )
			bot.enemy = Bots::states_[bot.leader].enemy;
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
	if( out.enemy )
	{
		Vec3 target;
		VectorCopy( out.enemyVisible ? out.enemy->r.currentOrigin : bot.seen[out.enemy->s.number].position, target );
		const float dist = Distance( ent->r.currentOrigin, target );
		// aim ahead of a moving enemy by the flight time of the gun's shot
		const int weapon = availableVehicles[ent->client_->vehicle_].weapons[0];
		const float shotSpeed = weapon && availableWeapons[weapon].muzzleVelocity > 0 ? static_cast<float>( availableWeapons[weapon].muzzleVelocity ) : 2500.0f;
		if( out.enemyVisible )
			VectorMA( target, std::min( dist / shotSpeed, 1.5f ), out.enemy->client_->ps_.velocity, target );
		const unsigned int enemyCat = availableVehicles[out.enemy->client_->vehicle_].cat;
		// An aircraft with gun turrets is approached from where the fewest of them can bear: each turret only covers
		// its own field, and a fighter that flies into four of them at once does not live. While more turrets bear
		// on where the fighter is than on the best side, it flies for that side and holds its fire; once there it
		// goes in. Pilots with little cunning do not think of it
		bool flanking = false;
		if( BotSenses::roleOf( ent->client_->vehicle_ ) == Bots::Role::Fighter && (availableVehicles[out.enemy->client_->vehicle_].flags & VF_GUNSTATIONS) &&
			out.enemyVisible && dist > 500 && Bots::character( bot ).cunning > 0.25f )
		{
			const int theirs = out.enemy->client_->vehicle_;
			const float* hull = out.enemy->client_->ps_.vehicleAngles;
			Vec3 toMe = Vec3( ent->r.currentOrigin ) - Vec3( out.enemy->r.currentOrigin ), myBearing;
			vectoangles( toMe, myBearing );
			const int onMe = std::popcount( VehicleData::gunStationsBearing( theirs, hull, myBearing ) );
			int fewest = onMe;
			float nearest = 0;
			Vec3 side;
			// (vectoangles pitch: positive is below the bomber). Level first: a turreted bomber's blind side is level
			for( float pitch : { 0.0f, -20.0f, 20.0f, -50.0f, 50.0f } )
			{
				for( float yaw = 0.0f; yaw < 360.0f; yaw += 30.0f )
				{
					Vec3 angles = { pitch, hull[YAW] + yaw, 0 }, dir;
					const int bearing = std::popcount( VehicleData::gunStationsBearing( theirs, hull, angles ) );
					AngleVectors( angles, dir, nullptr, nullptr );
					// of the sides equally thinly covered, the one nearest to where the fighter already is
					const float along = DotProduct( dir, toMe );
					if( bearing < fewest || (bearing == fewest && bearing < onMe && along > nearest) )
					{
						fewest = bearing;
						nearest = along;
						side = dir;
					}
				}
			}
			if( fewest < onMe )
			{
				constexpr float standOff = 900.0f;
				target = Vec3( out.enemy->r.currentOrigin ) + side * standOff;
				flanking = true;
			}
		}
		// a cunning fighter goes for the place behind and below an aircraft that is not hunting it
		if( !flanking && BotSenses::roleOf( ent->client_->vehicle_ ) == Bots::Role::Fighter && (enemyCat & (CAT_PLANE|CAT_HELO)) && dist > 1200 && dist < 6000 &&
			Bots::character( bot ).cunning > 0.5f && out.enemy->tracktarget_ != ent )
		{
			Vec3 behind;
			AngleVectors( out.enemy->client_->ps_.vehicleAngles, behind, nullptr, nullptr );
			target = target + behind * ( -700.0f );
			target[2] -= 250.0f * Bots::character( bot ).cunning;
		}
		VectorSubtract( target, ent->r.currentOrigin, out.enemyDir );
		out.enemyDist = VectorNormalize( out.enemyDir );
		vectoangles( out.enemyDir, out.enemyAngles );
		out.enemyAngles[PITCH] = AngleSubtract( out.enemyAngles[PITCH], 0 );
		out.enemyAir = (availableVehicles[out.enemy->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) != 0 &&
			!(out.enemy->client_->ps_.ONOFF & OO_LANDED);
	}
	out.height = BotWorld::heightAboveGround( ent->r.currentOrigin, ent->s.number );
	// where it was over the last six seconds, for the crash record
	if( theLevel.time_ >= bot.trailTime )
	{
		for( int i = 5; i > 0; i-- )
			VectorCopy( bot.trail[i - 1], bot.trail[i] );
		VectorCopy( ent->r.currentOrigin, bot.trail[0] );
		bot.trailTime = theLevel.time_ + 1000;
	}

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	out.role = BotSenses::roleOf( client->vehicle_ );
	out.health = std::clamp( static_cast<float>( ent->health_ ) / std::max( 1, client->pers_.maxHealth_ ), 0.0f, 1.0f );
	out.locked = (ps.stats[STAT_LOCKINFO] & (LI_BEING_LOCKED|LI_BEING_LAUNCHED)) != 0;
	out.launched = (ps.stats[STAT_LOCKINFO] & LI_BEING_LAUNCHED) != 0;
	out.stealth = (availableVehicles[client->vehicle_].flags & VF_STEALTH) != 0;
	out.revealed = ps.timers[TIMER_REVEAL] > theLevel.time_;

	// hit: remember when, and which way to break if this is the first hit in a while
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

	// the missile that will arrive first
	if( out.launched )
	{
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

	// one enemy aircraft and nobody else near is a duel
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
	out.duel = out.enemy && out.enemyAir && foes == 1 && (out.role == Bots::Role::Fighter || out.role == Bots::Role::AttackHelo);

	// the goal is reviewed once a second, but a goal just taken is kept for a few seconds unless something forces the matter
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
		bot.goalEvalTime = theLevel.time_ + 1000;
		if( bot.desire != previous )
			bot.commitTime = theLevel.time_ + 3000 + static_cast<int>( 3000 * Bots::character( bot ).patience );
	}
}
