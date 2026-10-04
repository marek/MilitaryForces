#include "bot_mind.h"
#include "bot_airnav.h"
#include <algorithm>
#include <cmath>

void Bots::runwayHeading( GameEntity* ent, State& bot )
{
	vec3_t origin;
	VectorCopy( ent->r.currentOrigin, origin );
	origin[2] -= heightAboveGround( ent->r.currentOrigin, ent->s.number );
	if( !runwayAt( origin ) )
		return;
	float best = 0;
	for( int step = 0; step < 24; step++ )
	{
		const float yaw = step * 15.0f;
		const float run = runwayRun( origin, yaw, 4000 );
		vec3_t angles = { -6.0f, yaw, 0 }, dir, start;
		AngleVectors( angles, dir, nullptr, nullptr );
		VectorCopy( origin, start );
		start[2] += 40;
		trace_t tr;
		pathBlocked( start, dir, 2500, ent->s.number, tr );
		const float score = std::min( run, 500.0f ) * 4.0f + tr.fraction * 2500.0f;
		if( run >= 150 && score > best )
		{
			best = score;
			bot.takeoffYaw = yaw;
		}
	}
	if( best > 0 )
		bot.takeoffTime = theLevel.time_ + 30000;
}

bool Bots::turnClear( GameEntity* ent, float radius, float degrees, float climb )
{
	const playerState_t& ps = ent->client_->ps_;
	vec3_t flat = { 0, ps.vehicleAngles[YAW], 0 }, forward, right, from;
	AngleVectors( flat, forward, right, nullptr );
	const float side = degrees > 0 ? -1.0f : 1.0f;
	const float total = std::min( static_cast<float>( fabs( degrees ) ), 210.0f );
	VectorCopy( ent->r.currentOrigin, from );
	for( float swept = 30.0f; swept <= total + 29.0f; swept += 30.0f )
	{
		const float angle = std::min( swept, total ) * static_cast<float>( M_PI ) / 180.0f;
		vec3_t point, dir;
		VectorMA( ent->r.currentOrigin, radius * sin( angle ), forward, point );
		VectorMA( point, side * radius * (1.0f - cos( angle )), right, point );
		point[2] += climb * radius * angle;
		VectorSubtract( point, from, dir );
		const float length = VectorNormalize( dir );
		trace_t tr;
		if( length > 1 && pathBlocked( from, dir, length, ent->s.number, tr, 50.0f ) && tr.entityNum == ENTITYNUM_WORLD && !tr.startsolid )
			return false;
		VectorCopy( point, from );
	}
	return true;
}

bool Bots::runwayBusy( int padIndex, GameEntity* self )
{
	const Pad& pad = pads_[padIndex];
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == self || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		vec3_t rel;
		VectorSubtract( other->r.currentOrigin, pad.center, rel );
		const float along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
		const float side = rel[1] * pad.axis[0] - rel[0] * pad.axis[1];
		if( fabs( along ) < pad.halfLength + 150 && fabs( side ) < 220 && fabs( rel[2] ) < 160 )
			return true;
		if( isBot( i ) && i < self->s.number && states_[i].landPhase == 2 && states_[i].pad >= 0 && Distance( pads_[states_[i].pad].center, pad.center ) < 400 )
			return true;
	}
	return false;
}

using behavior::Status;

struct PlaneTasks
{
	using Mind = Bots::Mind;
	using Goal = Bots::Goal;
	using Role = Bots::Role;

	static bool bomber( const Mind& c ) { return c.sense.role == Role::Bomber; }
	static bool hasBombs( const Mind& c )
	{
		return Bots::hasWeaponType( c.client->vehicle_, c.ps, WT_IRONBOMB ) || Bots::hasWeaponType( c.client->vehicle_, c.ps, WT_GUIDEDBOMB );
	}
	static float enemyTurn( const Mind& c ) { return AngleSubtract( c.sense.enemyAngles[YAW], c.ps.vehicleAngles[YAW] ); }

	static bool flying( Mind& c ) { return !c.direct; }
	static bool landing( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.pad >= 0; }
	static bool onGround( Mind& c ) { return c.landed(); }
	static bool onApproach( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.landPhase == 1; }
	static bool missileInbound( Mind& c )
	{
		return c.sense.missile && c.sense.missileTime <= 6.0f && !c.landed() && !(c.bot.desire == Goal::Rearm && c.bot.landPhase >= 2);
	}
	static bool evading( Mind& c ) { return c.bot.desire == Goal::Evade && (c.sense.blindsided || c.sense.enemy); }
	static bool blindsided( Mind& c ) { return c.sense.blindsided; }
	static bool engaging( Mind& c ) { return c.sense.enemy && c.bot.desire == Goal::Attack && theLevel.time_ > c.bot.egressTime; }
	static bool canLevelBomb( Mind& c ) { return bomber( c ) && !c.sense.enemyAir && hasBombs( c ); }
	static bool canDiveBomb( Mind& c )
	{
		const bool diver = c.who().aggression > 0.8f || c.who().cunning > 0.75f;
		return canLevelBomb( c ) && diver && c.sense.height > 900 && c.sense.enemyDist < c.sense.height * 2.2f;
	}
	static bool stalking( Mind& c ) { return c.sense.stealth && !c.sense.revealed && c.sense.enemyDist > 2500; }
	static bool onNose( Mind& c ) { return fabs( enemyTurn( c ) ) < 20.0f; }
	static bool fireLineClear( Mind& c )
	{
		if( !c.bot.confined || theLevel.time_ < c.bot.chaseTime )
			return true;
		trace_t line;
		vec3_t toEnemy;
		VectorCopy( c.sense.enemyDir, toEnemy );
		return !(Bots::pathBlocked( c.ent->r.currentOrigin, toEnemy, std::min( c.sense.enemyDist, 1500.0f ), c.ent->s.number, line, 60.0f ) &&
			line.entityNum == ENTITYNUM_WORLD && !line.startsolid);
	}
	static bool arcCommitted( Mind& c ) { return theLevel.time_ < c.bot.chaseTime; }
	static bool climbing( Mind& c ) { return c.bot.desire == Goal::Climb; }
	static bool canBombStructure( Mind& c )
	{
		return bomber( c ) && c.bot.desire == Goal::Roam && theLevel.time_ > c.bot.egressTime && c.sense.height > 700.0f && hasBombs( c );
	}
	static bool wantsWingman( Mind& c )
	{
		return g_gametype.integer >= GT_TEAM && c.sense.role == Role::Fighter && !c.sense.enemy && c.who().teamwork > 0.5f;
	}
	// only in the first seconds after leaving the runway; low and slow elsewhere is handled by the terrain check
	static bool climbingOut( Mind& c )
	{
		constexpr int climbOutTime = 6000;
		return theLevel.time_ < c.bot.groundTime + climbOutTime && c.sense.height < 150 && c.speed < c.veh.stallspeed * 1.6f;
	}

	static void roam( Mind& c )
	{
		if( theLevel.time_ > c.bot.goalTime || Distance( c.ent->r.currentOrigin, c.bot.goal ) < 2000 )
			Bots::pickGoal( c.ent, c.bot, 9000 );
		c.headFor( c.bot.goal );
		c.holdAltitude();
		c.travelTo( Vec3( c.bot.goal[0], c.bot.goal[1], c.ent->r.currentOrigin[2] ) );
	}

	static void aimAtEnemy( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW];
		if( c.sense.enemyAir || c.sense.height > planeMinHeight * 1.5f )
			c.view[PITCH] = std::clamp( c.sense.enemyAngles[PITCH], -50.0f, 35.0f );
		else
			c.holdAltitude();
		if( c.bot.confined )
			c.view[PITCH] = std::clamp( c.view[PITCH], -14.0f, 14.0f );
	}

	static void engage( Mind& c )
	{
		c.attacking = true;
		c.weaponsFree = true;
		c.travelTo( c.enemyPosition() );
		if( !c.sense.enemyAir && c.sense.enemyDist < 900 )
			c.bot.egressTime = theLevel.time_ + 5000;
		if( c.sense.enemyAir && c.sense.enemyDist < 500 )
			c.bot.egressTime = theLevel.time_ + 2500;
	}

	static void flyChosenArc( Mind& c )
	{
		aimAtEnemy( c );
		c.view[YAW] = c.ps.vehicleAngles[YAW] + c.bot.chaseTurn;
		if( c.bot.chasePitch != 0.0f )
			c.view[PITCH] = c.bot.chasePitch;
		if( c.bot.confined )
			c.view[PITCH] = std::clamp( c.view[PITCH], -14.0f, 14.0f );
	}

	static Status takeOff( Mind& c )
	{
		c.direct = true;
		c.bot.groundTime = theLevel.time_;
		if( theLevel.time_ >= c.bot.takeoffTime && c.speed < 20 )
			Bots::runwayHeading( c.ent, c.bot );
		c.view[YAW] = theLevel.time_ < c.bot.takeoffTime ? c.bot.takeoffYaw : c.ps.vehicleAngles[YAW];
		c.view[PITCH] = c.speed > c.veh.stallspeed * 0.97f ? -8.0f : 0.0f;
		if( fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) ) > 6 )
			c.cmd.forwardmove = c.speed < 12 ? moveFull : -moveFull;
		if( c.speed < c.veh.stallspeed * 0.5f )
		{
			vec3_t flat = { 0, c.ps.vehicleAngles[YAW], 0 }, forward;
			AngleVectors( flat, forward, nullptr, nullptr );
			for( int i = 1; i <= theLevel.maxclients_; i++ )
			{
				GameEntity* other = theLevel.getEntity( i );
				if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
					continue;
				vec3_t rel;
				VectorSubtract( other->r.currentOrigin, c.ent->r.currentOrigin, rel );
				const float dist = VectorNormalize( rel );
				if( dist < 320 && fabs( rel[2] * dist ) < 120 && DotProduct( rel, forward ) > 0.4f )
				{
					c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
					break;
				}
			}
		}
		return Status::Running;
	}

	static Status defendMissile( Mind& c )
	{
		const float toLeft = fabs( AngleSubtract( c.sense.missileYaw + 90.0f, c.ps.vehicleAngles[YAW] ) );
		const float toRight = fabs( AngleSubtract( c.sense.missileYaw - 90.0f, c.ps.vehicleAngles[YAW] ) );
		if( c.sense.missileTime > 1.5f )
			c.bot.breakSide = toLeft < toRight ? 1.0f : -1.0f;
		if( c.sense.missileTime > 1.2f * (0.5f + Bots::skill( c.bot )) )
		{
			c.view[YAW] = c.sense.missileYaw + 90.0f * c.bot.breakSide;
			c.view[PITCH] = c.sense.height > planeMinHeight * 2.0f ? 8.0f : -5.0f;
		}
		else
		{
			c.view[YAW] = c.sense.missileYaw - 100.0f * c.bot.breakSide;
			c.view[PITCH] = -28.0f;
		}
		c.cmd.forwardmove = moveFull;
		c.defending = true;
		return Status::Running;
	}

	static Status breakAway( Mind& c )
	{
		c.view[YAW] = c.bot.breakYaw;
		c.holdAltitude();
		return Status::Running;
	}

	static Status flee( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW] + 180.0f;
		c.holdAltitude();
		return Status::Running;
	}

	static Status diveBomb( Mind& c )
	{
		c.attacking = true;
		c.weaponsFree = true;
		c.travelTo( c.enemyPosition() );
		aimAtEnemy( c );
		for( int slot : secondaryWeaponSlots )
		{
			const int weapon = c.veh.weapons[slot];
			if( weapon && c.ps.ammo[slot] > 0 && (availableWeapons[weapon].type == WT_IRONBOMB || availableWeapons[weapon].type == WT_GUIDEDBOMB) )
			{
				c.forcedWeapon = slot;
				vec3_t nose;
				AngleVectors( c.ps.vehicleAngles, nose, nullptr, nullptr );
				if( c.ps.weaponNum == slot && DotProduct( nose, c.sense.enemyDir ) > 0.99f && c.sense.enemyDist < 1800 )
					c.cmd.buttons |= BUTTON_ATTACK_MAIN;
				break;
			}
		}
		if( c.sense.enemyDist < 1000 || c.sense.height < 700 )
			c.bot.egressTime = theLevel.time_ + 6000;
		return Status::Running;
	}

	static Status levelBomb( Mind& c )
	{
		roam( c );
		c.hasDest = false;
		c.bombing = true;
		Bots::bomb( c.ent, c.bot, c.sense.enemy->r.currentOrigin, c.cmd, c.view );
		return Status::Running;
	}

	static Status stalk( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW];
		c.holdAltitude();
		c.travelTo( c.enemyPosition() );
		return Status::Running;
	}

	static Status pressAttack( Mind& c )
	{
		engage( c );
		aimAtEnemy( c );
		return Status::Running;
	}

	static Status flyArc( Mind& c )
	{
		engage( c );
		flyChosenArc( c );
		return Status::Running;
	}

	static Status chooseArc( Mind& c )
	{
		const float turn = enemyTurn( c );
		const float longWay = turn > 0 ? turn - 360.0f : turn + 360.0f;
		const bool canDive = c.sense.height > planeMinHeight + 300.0f;
		const float radius = c.turnRadius();
		struct Option { float degrees; float climb; };
		const std::array<Option, 6> options = { {
			{ turn, 0.0f }, { turn, 0.25f }, { turn, -0.25f }, { longWay, 0.0f }, { longWay, 0.25f }, { longWay, -0.25f }
		} };
		for( const Option& option : options )
		{
			if( option.climb < 0 && !canDive )
				continue;
			if( !Bots::turnClear( c.ent, radius, option.degrees, option.climb ) )
				continue;
			engage( c );
			c.bot.chaseTurn = option.degrees > 0 ? 80.0f : -80.0f;
			c.bot.chasePitch = option.climb > 0 ? -14.0f : option.climb < 0 ? 14.0f : 0.0f;
			if( option.degrees != turn || option.climb != 0.0f )
			{
				c.bot.chaseTime = theLevel.time_ + 900;
				flyChosenArc( c );
			}
			else
			{
				c.bot.chaseTime = 0;
				aimAtEnemy( c );
			}
			return Status::Running;
		}
		return Status::Failure;
	}

	static Status holdCourse( Mind& c )
	{
		engage( c );
		c.attacking = false;
		c.snapShot = fabs( enemyTurn( c ) ) < 6.0f;
		c.view[YAW] = c.ps.vehicleAngles[YAW];
		c.holdAltitude();
		return Status::Running;
	}

	static Status gainHeight( Mind& c )
	{
		roam( c );
		return Status::Running;
	}

	static Status bombStructure( Mind& c )
	{
		GameEntity* structure = Bots::findStructure( c.ent, c.bot );
		if( !structure )
			return Status::Failure;
		roam( c );
		vec3_t target;
		for( int k = 0; k < 3; k++ )
			target[k] = (structure->r.absmin[k] + structure->r.absmax[k]) * 0.5f;
		Bots::bomb( c.ent, c.bot, target, c.cmd, c.view );
		return Status::Running;
	}

	static Status wingman( Mind& c )
	{
		GameEntity* leader = nullptr;
		float leaderDist = 0;
		for( int i = 1; i < c.ent->s.number; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( !alive( other ) || !OnSameTeam( c.ent, other ) || Bots::roleOf( other->client_->vehicle_ ) != Role::Fighter || (other->client_->ps_.ONOFF & OO_LANDED) )
				continue;
			const float dist = Distance( c.ent->r.currentOrigin, other->r.currentOrigin );
			if( dist < 9000 && (!leader || dist < leaderDist) )
			{
				leader = other;
				leaderDist = dist;
			}
		}
		c.bot.leader = leader ? leader->s.number : -1;
		if( !leader )
			return Status::Failure;
		vec3_t forward, right;
		AngleVectors( leader->client_->ps_.vehicleAngles, forward, right, nullptr );
		VectorMA( leader->r.currentOrigin, 600.0f, forward, c.bot.goal );
		VectorMA( c.bot.goal, 300.0f, right, c.bot.goal );
		c.bot.goalTime = theLevel.time_ + 2000;
		c.headFor( c.bot.goal );
		c.holdAltitude();
		c.travelTo( Vec3( c.bot.goal[0], c.bot.goal[1], c.ent->r.currentOrigin[2] ) );
		return Status::Running;
	}

	static Status patrol( Mind& c )
	{
		roam( c );
		return Status::Running;
	}

	static Status countermeasures( Mind& c )
	{
		Bots::flares( c.ent, c.bot, c.sense, c.cmd );
		return Status::Success;
	}

	static Status navigate( Mind& c )
	{
		State& bot = c.bot;
		if( c.bombing || c.defending )
			return Status::Success;
		if( bot.desire == Goal::Rearm && bot.landPhase == 1 )
			c.travelTo( Vec3( bot.goal[0], bot.goal[1], bot.approachHeight ) );
		const Vec3 here( c.ent->r.currentOrigin );
		const Vec3 toDest = c.dest - here;
		const float range = toDest.length();
		const Vec3 probeEnd = range > 2500.0f ? here + toDest * (2500.0f / range) : c.dest;
		if( !c.hasDest || range <= 300.0f || (!bot.confined && AirNav::clearLine( here, probeEnd, 70.0f )) )
		{
			bot.path.clear();
			bot.following = false;
			bot.cornerAhead = false;
			return Status::Success;
		}
		const float radius = c.turnRadius();
		if( theLevel.time_ >= bot.pathTime )
		{
			AirNav::findPath( here, c.dest, bot.path, Vec3( c.ps.velocity ).normalized(), radius );
			bot.pathIndex = 0;
			bot.pathTime = theLevel.time_ + 1500;
		}
		while( bot.pathIndex + 1 < bot.path.size() &&
			((bot.path[bot.pathIndex] - here).length() < radius * 1.3f || AirNav::clearLine( here, bot.path[bot.pathIndex + 1], 70.0f )) )
			bot.pathIndex++;
		if( bot.pathIndex >= bot.path.size() )
		{
			bot.following = false;
			bot.cornerAhead = false;
			return Status::Success;
		}

		const Vec3 leg = bot.path[bot.pathIndex] - here;
		vec3_t legDir, legAngles;
		leg.copyTo( legDir );
		vectoangles( legDir, legAngles );
		const float aimAt = c.sense.enemy ? fabs( enemyTurn( c ) ) : 180.0f;
		c.snapShot = c.attacking && aimAt < 6.0f;
		c.attacking = false;
		c.view[YAW] = legAngles[YAW];
		c.view[PITCH] = std::clamp( AngleSubtract( legAngles[PITCH], 0 ), -20.0f, 20.0f );
		bot.following = true;
		bot.cornerAhead = false;
		if( bot.pathIndex + 1 < bot.path.size() )
		{
			const Vec3 next = bot.path[bot.pathIndex + 1] - bot.path[bot.pathIndex];
			vec3_t nextDir, nextAngles;
			next.copyTo( nextDir );
			vectoangles( nextDir, nextAngles );
			bot.cornerAhead = fabs( AngleSubtract( nextAngles[YAW], legAngles[YAW] ) ) > 35.0f && leg.length() < radius * 3.5f;
		}
		return Status::Success;
	}

	static Status approachProfile( Mind& c )
	{
		c.view[PITCH] = std::clamp( (c.ent->r.currentOrigin[2] - c.bot.approachHeight) / 30.0f, -25.0f, 20.0f );
		if( c.speed > c.veh.stallspeed * 2.2f )
			c.cmd.forwardmove = -moveFull;
		return Status::Success;
	}

	static Status midChannel( Mind& c )
	{
		if( c.room < c.cruise * 2.0f && !(c.attacking && c.sense.enemyAir) && !onApproach( c ) )
			c.view[PITCH] = std::clamp( (c.sense.height - c.room * 0.5f) / 20.0f, -25.0f, 20.0f );
		return Status::Success;
	}

	static Status climbOut( Mind& c )
	{
		c.view[YAW] = c.ps.vehicleAngles[YAW];
		c.view[PITCH] = -8.0f;
		return Status::Success;
	}

	// where another aircraft will be: straight on, or round its turn when it is banked hard
	static void predictAircraft( GameEntity* other, float seconds, vec3_t out )
	{
		const playerState_t& ps = other->client_->ps_;
		const float roll = ps.vehicleAngles[ROLL];
		const float rate = std::max( 10.0f, availableVehicles[other->client_->vehicle_].turnspeed[YAW] ) * static_cast<float>( M_PI ) / 180.0f;
		const float speed = sqrtf( ps.velocity[0] * ps.velocity[0] + ps.velocity[1] * ps.velocity[1] );
		if( !(availableVehicles[other->client_->vehicle_].cat & CAT_PLANE) || fabs( roll ) < 45.0f || speed < 1.0f )
		{
			VectorMA( other->r.currentOrigin, seconds, ps.velocity, out );
			return;
		}
		const float radius = speed / rate;
		const float angle = rate * seconds;
		const float side = roll > 0 ? 1.0f : -1.0f;
		const float forward[2] = { ps.velocity[0] / speed, ps.velocity[1] / speed };
		const float right[2] = { forward[1], -forward[0] };
		out[0] = other->r.currentOrigin[0] + radius * (sin( angle ) * forward[0] + side * (1.0f - cos( angle )) * right[0]);
		out[1] = other->r.currentOrigin[1] + radius * (sin( angle ) * forward[1] + side * (1.0f - cos( angle )) * right[1]);
		out[2] = other->r.currentOrigin[2] + ps.velocity[2] * seconds;
	}

	// true when another aircraft is predicted to be near this point at that time
	static bool trafficAt( const Mind& c, const vec3_t point, float seconds )
	{
		constexpr float separation = 220.0f;
		constexpr float collisionRange = 70.0f;
		constexpr float lookAhead = 3.0f;
		if( seconds > lookAhead )
			return false;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( !(availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) || (other->client_->ps_.ONOFF & OO_LANDED) )
				continue;
			// right of way: the bot with the higher number gives way, unless it would run into the other from behind
			if( Bots::isBot( i ) && i > c.ent->s.number )
			{
				vec3_t mine, theirs;
				VectorNormalize2( c.ps.velocity, mine );
				VectorNormalize2( other->client_->ps_.velocity, theirs );
				if( DotProduct( mine, theirs ) < 0.5f )
					continue;
			}
			vec3_t predicted;
			predictAircraft( other, seconds, predicted );
			// flying alongside is fine: only a path that closes on the other aircraft is blocked
			const float now = Distance( other->r.currentOrigin, c.ent->r.currentOrigin );
			const float then = Distance( predicted, point );
			if( then < collisionRange || (then < separation && then < now * 0.85f) )
				return true;
		}
		return false;
	}

	struct Pose
	{
		vec3_t	origin;
		float	yaw;
		float	seconds;
	};

	// flies an arc of turnDegrees at the given radius and then straight on, returns the free distance up to horizon
	static float sweepFrom( const Mind& c, const Pose& start, float radius, float turnDegrees, float slope, float horizon, float halfWidth, Pose* finish = nullptr )
	{
		vec3_t flat = { 0, start.yaw, 0 }, forward, right, from;
		AngleVectors( flat, forward, right, nullptr );
		const float side = turnDegrees > 0 ? -1.0f : 1.0f;
		const float total = fabs( turnDegrees );
		const float airspeed = std::max( c.airspeed, 1.0f );
		float travelled = 0;
		VectorCopy( start.origin, from );
		const auto leg = [&]( const vec3_t to ) -> bool
		{
			vec3_t dir;
			trace_t tr;
			VectorSubtract( to, from, dir );
			const float length = VectorNormalize( dir );
			if( length < 1 )
				return true;
			// no trace needed while the leg stays inside one empty cell of the air navigation tree
			if( !AirNav::segmentKnownFree( Vec3( from ), Vec3( to ), halfWidth + 8.0f ) )
			{
				Bots::pathBlocked( from, dir, length, c.ent->s.number, tr, halfWidth );
				if( tr.startsolid && travelled <= 0 && start.seconds <= 0 )
					Bots::pathBlocked( from, dir, length, c.ent->s.number, tr );
				const bool vehicle = tr.entityNum >= 1 && tr.entityNum <= theLevel.maxclients_;
				if( tr.startsolid )
					tr.fraction = 0;
				if( tr.fraction < 1.0f && !vehicle )
				{
					travelled += tr.fraction * length;
					return false;
				}
			}
			if( trafficAt( c, to, start.seconds + (travelled + length) / airspeed ) )
			{
				travelled += length * 0.5f;
				return false;
			}
			travelled += length;
			VectorCopy( to, from );
			return true;
		};
		for( float swept = std::min( 30.0f, total ); swept > 0 && travelled < horizon; swept += 30.0f )
		{
			const float angle = std::min( swept, total ) * static_cast<float>( M_PI ) / 180.0f;
			vec3_t point;
			VectorMA( start.origin, radius * sin( angle ), forward, point );
			VectorMA( point, side * radius * (1.0f - cos( angle )), right, point );
			point[2] += slope * radius * angle;
			if( !leg( point ) )
				return travelled;
			if( swept >= total )
				break;
		}
		if( travelled < horizon )
		{
			vec3_t out = { 0, start.yaw + turnDegrees, 0 }, dir, end;
			AngleVectors( out, dir, nullptr, nullptr );
			dir[2] = slope;
			VectorMA( from, horizon - travelled, dir, end );
			leg( end );
		}
		if( finish )
		{
			VectorCopy( from, finish->origin );
			finish->yaw = start.yaw + turnDegrees;
			finish->seconds = start.seconds + travelled / airspeed;
		}
		return std::min( travelled, horizon );
	}

	static float sweep( const Mind& c, float radius, float turnDegrees, float slope, float horizon, float halfWidth, Pose* finish = nullptr )
	{
		Pose start;
		VectorCopy( c.ent->r.currentOrigin, start.origin );
		start.yaw = c.ps.vehicleAngles[YAW];
		start.seconds = 0;
		return sweepFrom( c, start, radius, turnDegrees, slope, horizon, halfWidth, finish );
	}

	// a manoeuvre is only worth flying if there is a way on from where it ends: straight ahead, over the top, or a turn back
	static bool wayOut( const Mind& c, const Pose& end, float brakedRadius, float horizon )
	{
		if( sweepFrom( c, end, brakedRadius, 0.0f, 0.0f, horizon, 60.0f ) >= horizon )
			return true;
		if( sweepFrom( c, end, brakedRadius, 0.0f, 0.35f, horizon, 60.0f ) >= horizon )
			return true;
		const float around = brakedRadius * static_cast<float>( M_PI ) + 200.0f;
		for( float turn : { 180.0f, -180.0f, 90.0f, -90.0f } )
		{
			if( sweepFrom( c, end, brakedRadius, turn, 0.0f, around, 60.0f ) >= around )
				return true;
		}
		return false;
	}

	static Status probeTerrain( Mind& c )
	{
		State& bot = c.bot;
		GameEntity* ent = c.ent;
		const float yaw = c.ps.vehicleAngles[YAW];
		if( theLevel.time_ >= bot.probeTime )
		{
			// look more often when something is in the way
			bot.probeTime = theLevel.time_ + (bot.confined || bot.probeBlocked ? 150 : 300);
			vec3_t flat = { 0, yaw, 0 }, right, left;
			trace_t tr;
			AngleVectors( flat, nullptr, right, nullptr );
			VectorNegate( right, left );
			Bots::pathBlocked( ent->r.currentOrigin, left, 1200, ent->s.number, tr );
			const float leftClear = tr.fraction * 1200.0f;
			Bots::pathBlocked( ent->r.currentOrigin, right, 1200, ent->s.number, tr );
			const float rightClear = tr.fraction * 1200.0f;
			bot.confined = leftClear + rightClear < 1500 || c.room < 900;

			const bool steering = theLevel.time_ < bot.avoidTime;
			const float wantYaw = c.view[YAW];
			const float wantTurn = AngleSubtract( wantYaw, yaw );
			const float wantSlope = std::clamp( -static_cast<float>( tan( c.view[PITCH] * M_PI / 180.0 ) ), -0.4f, 0.5f );
			const float horizon = std::clamp( c.airspeed * 3.0f, 900.0f, 2500.0f );
			const bool wasBlocked = bot.probeBlocked;
			const float radius = c.turnRadius();
			const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * static_cast<float>( M_PI ) / 180.0f;
			const float brakedRadius = (c.airspeed + std::min( c.airspeed, c.veh.stallspeed * 1.2f )) * 0.5f / turnRate;
			const bool canBrake = brakedRadius < radius * 0.85f;
			Pose wantEnd;
			bot.probeWant = sweep( c, radius, wantTurn, wantSlope, horizon, 60.0f, &wantEnd );
			if( bot.probeWant >= horizon && !wayOut( c, wantEnd, brakedRadius, horizon ) )
				bot.probeWant = horizon * 0.9f;
			bot.probeBest = bot.probeWant;
			// a jet spawns fast: slow down at once when there is no room
			if( (bot.confined || bot.probeWant < horizon) && theLevel.time_ < bot.spawnTime + 4000 )
				bot.brakeTime = theLevel.time_ + 600;
			bot.probeBlocked = bot.probeWant < horizon;
			if( wasBlocked && !bot.probeBlocked )
				bot.avoidTime = 0;
			if( bot.probeBlocked )
			{
				constexpr std::array<float, 11> turns = { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f, 135.0f, -135.0f, 180.0f, -180.0f };
				struct Climb { float slope; float pitch; bool keep; };
				const std::array<Climb, 3> climbs = { { { wantSlope, c.view[PITCH], true }, { 0.35f, -19.0f, false }, { -0.25f, 14.0f, false } } };
				const bool canDive = c.sense.height > c.minHeight + 300.0f;
				const float previousYaw = bot.avoidYaw;
				const float previousPitch = bot.avoidPitch;

				// candidates in order of preference, so the search can stop at the first one that is clear
				struct Candidate { float turn; const Climb* climb; float command; float penalty; };
				std::array<Candidate, turns.size() * climbs.size()> candidates;
				size_t count = 0;
				for( float turn : turns )
				{
					for( const Climb& climb : climbs )
					{
						if( climb.slope < 0 && !climb.keep && !canDive )
							continue;
						const float command = yaw + std::clamp( turn, -100.0f, 100.0f );
						float penalty = fabs( AngleSubtract( yaw + turn, wantYaw ) ) * 2.0f + fabs( turn ) + (climb.keep ? 0.0f : 60.0f);
						// stay with the manoeuvre already begun
						if( steering )
							penalty += fabs( AngleSubtract( command, previousYaw ) ) * 1.5f + (climb.pitch * previousPitch < 0 ? 150.0f : 0.0f);
						candidates[count++] = { turn, &climb, command, penalty };
					}
				}
				std::sort( candidates.begin(), candidates.begin() + count, []( const Candidate& a, const Candidate& b ) { return a.penalty < b.penalty; } );

				for( float width : { 60.0f, 0.0f } )
				{
					float best = -99999.0f, bestFree = 0;
					bool bestBraked = false;
					for( size_t i = 0; i < count; i++ )
					{
						const Candidate& candidate = candidates[i];
						Pose end;
						float free = sweep( c, radius, candidate.turn, candidate.climb->slope, horizon, width, &end );
						bool braked = false;
						if( free < horizon && candidate.turn != 0.0f && canBrake )
						{
							Pose slowEnd;
							const float slow = sweep( c, brakedRadius, candidate.turn, candidate.climb->slope, horizon, width, &slowEnd );
							if( slow > free )
							{
								free = slow;
								braked = true;
								end = slowEnd;
							}
						}
						// leads into a dead end: better than a wall, worse than any open path
						if( free >= horizon && width > 0 && !wayOut( c, end, brakedRadius, horizon ) )
							free = horizon * 0.9f;
						const float score = (free < horizon ? free * 2.0f - horizon * 2.0f : horizon) - candidate.penalty - (braked ? 40.0f : 0.0f);
						if( score > best )
						{
							best = score;
							bestFree = free;
							bestBraked = braked;
							bot.avoidYaw = candidate.command;
							bot.avoidPitch = candidate.climb->pitch;
						}
						if( free >= horizon )
							break;
					}
					bot.probeBest = bestFree;
					if( bestBraked )
						bot.brakeTime = theLevel.time_ + 600;
					if( bestFree > radius * 1.2f )
						break;
				}
				bot.avoidTime = theLevel.time_ + 400;
			}
		}
		if( theLevel.time_ < bot.avoidTime )
		{
			c.view[YAW] = bot.avoidYaw;
			c.view[PITCH] = bot.avoidPitch;
		}
		return Status::Success;
	}

	static Status cornerSpeed( Mind& c )
	{
		const bool canBrake = (c.veh.caps & HC_SPEEDBRAKE) != 0;
		const bool braking = (c.ps.ONOFF & OO_SPEEDBRAKE) != 0;
		if( c.bot.confined || c.bot.cornerAhead || theLevel.time_ < c.bot.brakeTime )
		{
			const float slow = c.veh.stallspeed * (theLevel.time_ < c.bot.brakeTime ? 1.15f : 1.25f);
			if( c.speed > slow )
				c.cmd.forwardmove = -moveFull;
			if( (c.speed > slow * 1.3f) != braking && canBrake )
				c.cmd.buttons |= BUTTON_BRAKE;
		}
		else if( braking && c.bot.desire != Goal::Rearm && canBrake )
			c.cmd.buttons |= BUTTON_BRAKE;
		return Status::Success;
	}

	static Status avoidTraffic( Mind& c )
	{
		State& bot = c.bot;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( !(availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) || (other->client_->ps_.ONOFF & OO_LANDED) )
				continue;
			vec3_t rel, relVel, miss;
			VectorSubtract( other->r.currentOrigin, c.ent->r.currentOrigin, rel );
			if( VectorLength( rel ) > 3000 )
				continue;
			VectorSubtract( other->client_->ps_.velocity, c.ps.velocity, relVel );
			const float closing = DotProduct( relVel, relVel );
			if( closing < 1.0f )
				continue;
			const float when = -DotProduct( rel, relVel ) / closing;
			if( when <= 0 || when > (bot.confined ? 3.0f : 2.0f) )
				continue;
			VectorMA( rel, when, relVel, miss );
			if( VectorLength( miss ) > 260 )
				continue;
			vec3_t right;
			AngleVectors( c.ps.vehicleAngles, nullptr, right, nullptr );
			const float side = DotProduct( rel, right ) + DotProduct( miss, right );
			const float dodgePitch = (rel[2] + miss[2] >= 0 && c.sense.height > c.minHeight + 250) ? 22.0f : -28.0f;
			bot.egressTime = std::max( bot.egressTime, theLevel.time_ + 1500 );
			if( bot.confined )
			{
				c.view[PITCH] = std::clamp( dodgePitch, -14.0f, 12.0f );
				// keep right to pass head-on traffic
				vec3_t ahead;
				AngleVectors( c.ps.vehicleAngles, ahead, nullptr, nullptr );
				if( DotProduct( rel, ahead ) > 0 )
					c.view[YAW] = c.ps.vehicleAngles[YAW] - 25.0f;
				break;
			}
			// turn away from the other aircraft; the terrain check after this node vets the result
			c.view[YAW] = c.ps.vehicleAngles[YAW] + (side > 0 ? 45.0f : -45.0f);
			c.view[PITCH] = dodgePitch;
			break;
		}
		return Status::Success;
	}

	static Status floorAndCeiling( Mind& c )
	{
		if( c.sense.height < c.minHeight )
			c.view[PITCH] = std::min( c.view[PITCH], -35.0f );
		else if( c.ceiling < 250 )
			c.view[PITCH] = std::max( c.view[PITCH], c.sense.height > c.minHeight + 150 ? 12.0f : 0.0f );
		return Status::Success;
	}

	static Status keepEnergy( Mind& c )
	{
		if( c.speed < c.veh.stallspeed * (c.bot.confined ? 1.08f : 1.3f) && c.sense.height > c.minHeight * 2 )
			c.view[PITCH] = std::max( c.view[PITCH], 5.0f );
		else if( c.speed < c.veh.stallspeed * 1.7f )
			c.view[PITCH] = std::max( c.view[PITCH], -10.0f );
		if( c.ps.ONOFF & OO_STALLED )
			c.view[PITCH] = c.speed > c.veh.stallspeed ? c.ps.vehicleAngles[PITCH] + 1.0f : std::max( c.view[PITCH], c.ps.vehicleAngles[PITCH] + 1.0f );
		return Status::Success;
	}

	static Status fire( Mind& c )
	{
		if( !c.attacking && !c.snapShot && !c.weaponsFree )
			return Status::Success;
		vec3_t fwd;
		AngleVectors( c.ps.vehicleAngles, fwd, nullptr, nullptr );
		const float aimError = acos( std::clamp( DotProduct( fwd, c.sense.enemyDir ), -1.0f, 1.0f ) ) * 180.0f / M_PI;
		Bots::fireWeapons( c.ent, c.bot, c.sense, c.cmd, aimError );
		if( c.forcedWeapon >= 0 )
			c.cmd.weapon = c.forcedWeapon;
		return Status::Success;
	}

	using State = Bots::State;

	static behavior::Tree<Mind> build()
	{
		behavior::Tree<Mind> t;
		const auto* hold = t.action( "hold course", holdCourse );

		const auto* land = t.sequence( "land and rearm", {
			t.action( "survey runway", LandingSteps::survey ),
			t.retry( "circuit", t.sequence( "approach and final", {
				t.action( "approach", LandingSteps::approach ),
				t.action( "final", LandingSteps::final ) } ) ),
			t.action( "roll out", LandingSteps::rollOut ),
			t.action( "taxi", LandingSteps::taxi ),
			t.action( "refuel", LandingSteps::refuel ),
			t.action( "leave runway", LandingSteps::leaveRunway ) } );

		const auto* pursue = t.selector( "pursue", {
			t.when( "on the nose", onNose, t.selector( "shot", {
				t.when( "line of fire clear", fireLineClear, t.action( "press attack", pressAttack ) ),
				hold } ) ),
			t.when( "committed to an arc", arcCommitted, t.action( "fly arc", flyArc ) ),
			t.action( "choose arc", chooseArc ),
			hold } );

		const auto* attack = t.selector( "attack", {
			t.when( "can dive bomb", canDiveBomb, t.action( "dive bomb", diveBomb ) ),
			t.when( "can level bomb", canLevelBomb, t.action( "level bomb", levelBomb ) ),
			t.when( "unseen stealth", stalking, t.action( "stalk", stalk ) ),
			pursue } );

		const auto* act = t.selector( "plane", {
			t.when( "missile inbound", missileInbound, t.action( "defend missile", defendMissile ) ),
			t.when( "wants to rearm", landing, land ),
			t.when( "on the ground", onGround, t.action( "take off", takeOff ) ),
			t.when( "evading", evading, t.selector( "evade", {
				t.when( "blindsided", blindsided, t.action( "break away", breakAway ) ),
				t.action( "flee", flee ) } ) ),
			t.when( "engaging", engaging, attack ),
			t.when( "wants height", climbing, t.action( "gain height", gainHeight ) ),
			t.when( "bomber with payload", canBombStructure, t.action( "bomb structure", bombStructure ) ),
			t.when( "team fighter without target", wantsWingman, t.action( "wingman", wingman ) ),
			t.action( "patrol", patrol ) } );

		const auto* safety = t.selector( "terrain", {
			t.when( "just airborne", climbingOut, t.action( "climb out", climbOut ) ),
			t.always( "avoid", t.action( "avoid traffic", avoidTraffic ), {
				t.action( "probe terrain", probeTerrain ),
				t.action( "corner speed", cornerSpeed ),
				t.action( "floor and ceiling", floorAndCeiling ) } ) } );

		const auto* flight = t.when( "in free flight", flying, t.always( "fly", t.action( "countermeasures", countermeasures ), {
			t.action( "navigate", navigate ),
			t.when( "on approach", onApproach, t.action( "approach profile", approachProfile ) ),
			t.action( "mid channel", midChannel ),
			safety,
			t.action( "keep energy", keepEnergy ),
			t.action( "fire", fire ) } ) );

		t.setRoot( t.always( "pilot", act, { flight } ) );
		return t;
	}
};

void Bots::thinkPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = PlaneTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };

	mind.cruise = planeCruiseHeight;
	if( sense.role == Role::Bomber )
		mind.cruise = std::clamp( sense.height + ceilingAbove( ent->r.currentOrigin, ent->s.number ) - 700.0f, 800.0f, 3500.0f );
	else if( bot.desire == Goal::Climb )
		mind.cruise = sense.height + 500.0f;
	else if( !sense.enemy || !sense.enemyAir )
		mind.cruise = planeCruiseHeight - 800.0f * character( bot ).cunning;
	mind.airspeed = VectorLength( ps.velocity );
	mind.ceiling = ceilingAbove( ent->r.currentOrigin, ent->s.number );
	mind.room = sense.height + mind.ceiling;
	mind.minHeight = std::min( planeMinHeight, mind.room * 0.3f );

	cmd.forwardmove = moveFull;
	view[YAW] = ps.vehicleAngles[YAW];
	mind.holdAltitude();

	runTree( tree, mind, "plane" );
}
