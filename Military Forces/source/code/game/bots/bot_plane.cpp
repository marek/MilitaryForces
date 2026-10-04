#include "bot_mind.h"
#include "bot_airnav.h"
#include <algorithm>
#include <cmath>

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
		const float angle = std::min( swept, total ) * std::numbers::pi_v<float> / 180.0f;
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
		return c.bot.leader >= 0 && !c.sense.enemy;
	}
	static constexpr float rotateSpeed = 1.12f;

	static bool departing( Mind& c )
	{
		constexpr int departTime = 4000;
		return !c.landed() && theLevel.time_ < c.bot.groundTime + departTime;
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

	// Chooses the take-off heading: enough runway to reach flying speed, nothing standing on it, and from the lift-off
	// point a climb, straight or turning, that fits the terrain and leads somewhere.
	static void planDeparture( Mind& c )
	{
		constexpr float groundAcceleration = 85.0f;
		constexpr float climbSlope = 0.14f;
		constexpr float departure = 900.0f;
		vec3_t origin;
		VectorCopy( c.ent->r.currentOrigin, origin );
		origin[2] -= c.sense.height;
		if( !Bots::runwayAt( origin ) )
		{
#ifdef MF_BOT_DEBUG
			if( g_botDebug.integer >= 2 )
				Common::printf( "BOTTAKEOFF t %d bot %d not on a runway at %.0f %.0f %.0f height %.0f\n", theLevel.time_, c.ent->s.number, origin[0], origin[1], origin[2], c.sense.height );
#endif
			return;
		}
		const float stall = static_cast<float>( c.veh.stallspeed );
		const float groundRun = stall * rotateSpeed * stall * rotateSpeed / (2.0f * groundAcceleration) * 1.15f;
		const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
		const float radius = stall * 1.15f / turnRate;
		float best = 0, bestRun = 0, longest = 0;
		for( int step = 0; step < 24; step++ )
		{
			const float yaw = step * 15.0f;
			const float run = Bots::runwayRun( origin, yaw, 4000 );
			longest = std::max( longest, run );
			if( run < 150 )
				continue;
			vec3_t level = { 0, yaw, 0 }, dir, start;
			trace_t ground;
			AngleVectors( level, dir, nullptr, nullptr );
			VectorCopy( origin, start );
			start[2] += 14;
			const float roll = std::min( run, groundRun + 100.0f );
			Bots::pathBlocked( start, dir, roll, c.ent->s.number, ground, 30.0f );
			const bool rollClear = ground.startsolid || ground.fraction >= 1.0f;

			Pose liftOff;
			VectorMA( origin, std::min( run, groundRun ), dir, liftOff.origin );
			liftOff.origin[2] += 30;
			liftOff.yaw = yaw;
			liftOff.seconds = 3.0f;
			float departTurn = 0;
			bool departClear = false;
			for( float turn : { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f } )
			{
				Pose end;
				if( sweepFrom( c, liftOff, radius, turn, climbSlope, departure, 60.0f, &end ) >= departure && wayOut( c, end, radius, departure ) )
				{
					departTurn = turn;
					departClear = true;
					break;
				}
			}
			const float score = (run >= groundRun ? 5000.0f : 0.0f) + (rollClear ? 3000.0f : 0.0f) + (departClear ? 4000.0f - fabs( departTurn ) * 5.0f : 0.0f) +
				std::min( run, groundRun * 2.0f ) * 2.0f;
			if( score > best )
			{
				best = score;
				bestRun = run;
				c.bot.takeoffYaw = yaw;
				c.bot.departYaw = yaw + departTurn;
			}
		}
#ifdef MF_BOT_DEBUG
		if( g_botDebug.integer >= 2 )
			Common::printf( "BOTTAKEOFF t %d bot %d veh %d at %.0f %.0f %.0f facing %.0f chose %.0f then %.0f run %.0f need %.0f longest %.0f score %.0f\n", theLevel.time_, c.ent->s.number,
				c.client->vehicle_, origin[0], origin[1], origin[2], c.ps.vehicleAngles[YAW], c.bot.takeoffYaw, c.bot.departYaw, bestRun, groundRun, longest, best );
#endif
		if( best > 0 )
			c.bot.takeoffTime = theLevel.time_ + 30000;
	}

	static Status takeOff( Mind& c )
	{
		c.direct = true;
		c.bot.groundTime = theLevel.time_;
		if( theLevel.time_ >= c.bot.takeoffTime && c.speed < 20 )
			planDeparture( c );
		c.view[YAW] = theLevel.time_ < c.bot.takeoffTime ? c.bot.takeoffYaw : c.ps.vehicleAngles[YAW];
		// leave the ground with speed in hand: lifting off right at the stall drops the aircraft back
		c.view[PITCH] = c.speed > c.veh.stallspeed * rotateSpeed ? -8.0f : 0.0f;
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

	// follow the departure that was planned on the ground; the manoeuvre check still vets it
	static Status depart( Mind& c )
	{
		c.view[YAW] = c.bot.departYaw;
		c.view[PITCH] = -8.0f;
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
				const float blast = static_cast<float>( availableWeapons[weapon].damageRadius );
				if( c.ps.weaponNum == slot && DotProduct( nose, c.sense.enemyDir ) > 0.99f && c.sense.enemyDist < 1800 &&
					!Bots::friendlyNear( c.ent, c.sense.enemy->r.currentOrigin, blast * 1.3f + 150.0f ) )
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
		GameEntity* leader = Bots::leaderOf( c.bot );
		if( !leader )
			return Status::Failure;
		// off the leader's wing, on the side given by the client number so two wingmen take different sides
		vec3_t forward, right;
		AngleVectors( leader->client_->ps_.vehicleAngles, forward, right, nullptr );
		VectorMA( leader->r.currentOrigin, 600.0f, forward, c.bot.goal );
		VectorMA( c.bot.goal, (c.ent->s.number & 1) ? 300.0f : -300.0f, right, c.bot.goal );
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
		// in a corridor, traffic one way flies above the middle and traffic the other way below it
		constexpr float laneOffset = 0.17f;
		const float lane = cos( c.ps.vehicleAngles[YAW] * std::numbers::pi_v<float> / 180.0f ) >= 0.0f ? 0.5f + laneOffset : 0.5f - laneOffset;
		if( c.room < c.cruise * 2.0f && !(c.attacking && c.sense.enemyAir) && !onApproach( c ) )
			c.view[PITCH] = std::clamp( (c.sense.height - c.room * lane) / 20.0f, -25.0f, 20.0f );
		return Status::Success;
	}

	// where another aircraft will be: straight on, or round its turn when it is banked hard
	static void predictAircraft( GameEntity* other, float seconds, vec3_t out )
	{
		const playerState_t& ps = other->client_->ps_;
		const float roll = ps.vehicleAngles[ROLL];
		const float rate = std::max( 10.0f, availableVehicles[other->client_->vehicle_].turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
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
			// look for traffic at points along the leg, not only at its end
			constexpr float trafficStep = 150.0f;
			const int steps = std::max( 1, static_cast<int>( ceil( length / trafficStep ) ) );
			for( int step = 1; step <= steps; step++ )
			{
				const float along = length * step / steps;
				vec3_t point;
				VectorMA( from, along, dir, point );
				if( trafficAt( c, point, start.seconds + (travelled + along) / airspeed ) )
				{
					travelled += along - length / steps;
					return false;
				}
			}
			travelled += length;
			VectorCopy( to, from );
			return true;
		};
		for( float swept = std::min( 30.0f, total ); swept > 0 && travelled < horizon; swept += 30.0f )
		{
			const float angle = std::min( swept, total ) * std::numbers::pi_v<float> / 180.0f;
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
		if( sweepFrom( c, end, brakedRadius, 0.0f, 0.17f, horizon, 60.0f ) >= horizon )
			return true;
		// follow a bend, turn off, or turn back
		for( float turn : { 45.0f, -45.0f, 90.0f, -90.0f, 180.0f, -180.0f } )
		{
			const float length = brakedRadius * fabs( turn ) * std::numbers::pi_v<float> / 180.0f + (fabs( turn ) < 100.0f ? horizon * 0.6f : 200.0f);
			if( sweepFrom( c, end, brakedRadius, turn, 0.0f, length, 60.0f ) >= length )
				return true;
		}
		return false;
	}

	// Plans the next second or two of flight. The heading the pilot wants is flown ahead as an arc at the aircraft's turn
	// radius followed by a straight; if that does not fit, other manoeuvres are tried in order of preference.
	static Status probeTerrain( Mind& c )
	{
		State& bot = c.bot;
		GameEntity* ent = c.ent;
		const float yaw = c.ps.vehicleAngles[YAW];
		// the first moments of a departure were planned on the ground; too low to manoeuvre yet
		if( departing( c ) && c.sense.height < 60.0f )
			return Status::Success;
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

			constexpr float margin = 60.0f;
			const float wantYaw = c.view[YAW];
			const float wantTurn = AngleSubtract( wantYaw, yaw );
			// near the stall the nose can only come up a little, so plan the climb that can actually be flown
			const bool slow = c.speed < c.veh.stallspeed * 1.7f;
			const float flyablePitch = slow ? std::max( c.view[PITCH], -10.0f ) : c.view[PITCH];
			const float wantSlope = std::clamp( -static_cast<float>( tan( flyablePitch * std::numbers::pi_v<float> / 180.0 ) ), -0.4f, 0.5f );
			const float horizon = std::clamp( c.airspeed * 3.0f, 900.0f, 2500.0f );
			const bool wasBlocked = bot.probeBlocked;
			const float radius = c.turnRadius();
			const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
			const float brakedRadius = (c.airspeed + std::min( c.airspeed, c.veh.stallspeed * 1.2f )) * 0.5f / turnRate;
			const bool canBrake = brakedRadius < radius * 0.85f;

			Pose wantEnd;
			bot.probeWant = sweep( c, radius, wantTurn, wantSlope, horizon, margin, &wantEnd );
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
				struct Climb { float slope; float pitch; };
				const std::array<Climb, 3> climbs = { { { wantSlope, c.view[PITCH] }, { slow ? 0.17f : 0.35f, slow ? -10.0f : -19.0f }, { -0.25f, 14.0f } } };
				const bool canDive = c.sense.height > c.minHeight + 300.0f;

				// candidates in order of preference, so the search can stop at the first one that is clear
				struct Candidate { float turn; float slope; float pitch; float penalty; };
				std::array<Candidate, turns.size() * climbs.size() + 1> candidates;
				size_t count = 0;
				// the wanted heading itself comes first: it may still be the best of a bad set
				candidates[count++] = { wantTurn, wantSlope, c.view[PITCH], -500.0f };
				for( float turn : turns )
				{
					for( size_t k = 0; k < climbs.size(); k++ )
					{
						if( k == 2 && !canDive )
							continue;
						float penalty = fabs( AngleSubtract( yaw + turn, wantYaw ) ) * 2.0f + fabs( turn ) + (k ? 60.0f : 0.0f);
						// keep right: when both sides are open, pass traffic on the right
						if( turn > 0 )
							penalty += 15.0f;
						candidates[count++] = { turn, climbs[k].slope, climbs[k].pitch, penalty };
					}
				}
				std::sort( candidates.begin(), candidates.begin() + count, []( const Candidate& a, const Candidate& b ) { return a.penalty < b.penalty; } );

				// with clearance first; if nothing fits, the longest bare line
				for( float width : { margin, 0.0f } )
				{
					float best = -99999.0f, bestFree = 0;
					bool bestBraked = false;
					const Candidate* chosen = nullptr;
					for( size_t i = 0; i < count; i++ )
					{
						const Candidate& candidate = candidates[i];
						Pose end;
						float free = sweep( c, radius, candidate.turn, candidate.slope, horizon, width, &end );
						bool braked = false;
						// the same turn with the air brake out, at the radius the lower speed gives
						if( free < horizon && fabs( candidate.turn ) > 10.0f && canBrake )
						{
							Pose slowEnd;
							const float slowFree = sweep( c, brakedRadius, candidate.turn, candidate.slope, horizon, width, &slowEnd );
							if( slowFree > free )
							{
								free = slowFree;
								braked = true;
								end = slowEnd;
							}
						}
						// leads into a dead end: better than a wall, worse than any open path
						if( free >= horizon && width > 0 && !wayOut( c, end, brakedRadius, horizon ) )
							free = horizon * 0.9f;
						const float score = (free < horizon ? free * 2.0f - horizon * 2.0f : horizon) - std::max( candidate.penalty, 0.0f ) - (braked ? 40.0f : 0.0f);
						if( score > best )
						{
							best = score;
							bestFree = free;
							bestBraked = braked;
							chosen = &candidate;
						}
						if( free >= horizon )
							break;
					}
					if( chosen )
					{
						bot.avoidYaw = yaw + std::clamp( chosen->turn, -100.0f, 100.0f );
						bot.avoidPitch = chosen->pitch;
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
		const float aimError = acos( std::clamp( DotProduct( fwd, c.sense.enemyDir ), -1.0f, 1.0f ) ) * 180.0f / std::numbers::pi_v<float>;
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
			t.when( "just airborne", departing, t.action( "depart", depart ) ),
			t.when( "evading", evading, t.selector( "evade", {
				t.when( "blindsided", blindsided, t.action( "break away", breakAway ) ),
				t.action( "flee", flee ) } ) ),
			t.when( "engaging", engaging, attack ),
			t.when( "wants height", climbing, t.action( "gain height", gainHeight ) ),
			t.when( "bomber with payload", canBombStructure, t.action( "bomb structure", bombStructure ) ),
			t.when( "has a leader and no target", wantsWingman, t.action( "wingman", wingman ) ),
			t.action( "patrol", patrol ) } );

		const auto* safety = t.always( "avoid", t.action( "plan manoeuvre", probeTerrain ), {
			t.action( "corner speed", cornerSpeed ),
			t.action( "floor and ceiling", floorAndCeiling ) } );

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
