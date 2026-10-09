#include "bot_mind.h"
#include "bot_airnav.h"
#include <algorithm>
#include <cmath>

// can the aircraft fly a turn of that radius through that many degrees, climbing or diving at that slope, without
// meeting the world? The arc is checked in 30 degree pieces
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
		Vec3 point, dir;
		VectorMA( ent->r.currentOrigin, radius * sin( angle ), forward, point );
		VectorMA( point, side * radius * (1.0f - cos( angle )), right, point );
		point[2] += climb * radius * angle;
		VectorSubtract( point, from, dir );
		const float length = VectorNormalize( dir );
		trace_t tr;
		if( length > 1 && BotWorld::pathBlocked( from, dir, length, ent->s.number, tr, 50.0f ) && tr.entityNum == ENTITYNUM_WORLD && !tr.startsolid )
			return false;
		VectorCopy( point, from );
	}
	return true;
}

// someone is on or just over the runway, or a bot with a lower number is already on final for it
bool Bots::runwayBusy( int padIndex, GameEntity* self )
{
	const BotWorld::Pad& pad = BotWorld::pads_[padIndex];
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == self || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		Vec3 rel;
		VectorSubtract( other->r.currentOrigin, pad.center, rel );
		const float along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
		const float side = rel[1] * pad.axis[0] - rel[0] * pad.axis[1];
		if( fabs( along ) < pad.halfLength + 150 && fabs( side ) < 220 && fabs( rel[2] ) < 160 )
			return true;
		if( isBot( i ) && i < self->s.number && states_[i].landPhase == 2 && states_[i].pad >= 0 && Distance( BotWorld::pads_[states_[i].pad].center, pad.center ) < 400 )
			return true;
	}
	return false;
}

using behavior::Status;

// What a plane's pilot does: the conditions and steps of the plane tree. The steps choose where to point the nose;
// navigation, the manoeuvre planner and the safety steps at the end may overrule them to keep the aircraft flying.
struct PlaneTasks
{
	using Mind = Bots::Mind;
	using Goal = Bots::Goal;
	using Role = Bots::Role;

	static bool bomber( const Mind& c ) { return c.sense.role == Role::Bomber; }
	static bool hasBombs( const Mind& c )
	{
		return BotWeapons::hasWeaponType( c.client->vehicle_, c.ps, WT_IRONBOMB ) || BotWeapons::hasWeaponType( c.client->vehicle_, c.ps, WT_GUIDEDBOMB );
	}
	// how far the nose must turn to point at the enemy, signed
	static float enemyTurn( const Mind& c ) { return AngleSubtract( c.sense.enemyAngles[YAW], c.ps.vehicleAngles[YAW] ); }

	// free flight: not in a step that works the controls itself (take-off, final, roll-out)
	static bool flying( Mind& c ) { return !c.direct; }
	static bool landing( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.pad >= 0; }
	static bool onGround( Mind& c ) { return c.landed(); }
	static bool onApproach( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.landPhase == 1; }
	// a missile within six seconds; not acted on once committed to a final approach
	static bool missileInbound( Mind& c )
	{
		return c.sense.missile && c.sense.missileTime <= 6.0f && !c.landed() && !(c.bot.desire == Goal::Rearm && c.bot.landPhase >= 2);
	}
	static bool evading( Mind& c ) { return c.bot.desire == Goal::Evade && (c.sense.blindsided || c.sense.enemy); }
	static bool blindsided( Mind& c ) { return c.sense.blindsided; }
	// attacking, and not just leaving after a pass
	static bool engaging( Mind& c ) { return c.sense.enemy && c.bot.desire == Goal::Attack && theLevel.time_ > c.bot.egressTime; }
	static bool canLevelBomb( Mind& c ) { return bomber( c ) && !c.sense.enemyAir && hasBombs( c ); }
	// bold or cunning bomber pilots dive on a ground target when high and close enough
	static bool canDiveBomb( Mind& c )
	{
		const bool diver = c.who().aggression > 0.8f || c.who().cunning > 0.75f;
		return canLevelBomb( c ) && diver && c.sense.height > 900 && c.sense.enemyDist < c.sense.height * 2.2f;
	}
	// a stealth aircraft not yet seen closes in without showing itself
	static bool stalking( Mind& c ) { return c.sense.stealth && !c.sense.revealed && c.sense.enemyDist > 2500; }
	static bool onNose( Mind& c ) { return fabs( enemyTurn( c ) ) < 20.0f; }
	// in tight terrain, nothing of the world between the nose and the enemy
	static bool fireLineClear( Mind& c )
	{
		if( !c.bot.confined || theLevel.time_ < c.bot.chaseTime )
			return true;
		trace_t line;
		Vec3 toEnemy;
		VectorCopy( c.sense.enemyDir, toEnemy );
		return !(BotWorld::pathBlocked( c.ent->r.currentOrigin, toEnemy, std::min( c.sense.enemyDist, 1500.0f ), c.ent->s.number, line, 60.0f ) &&
			line.entityNum == ENTITYNUM_WORLD && !line.startsolid);
	}
	// a turn chosen a moment ago is flown out before another is chosen
	static bool arcCommitted( Mind& c ) { return theLevel.time_ < c.bot.chaseTime; }
	static bool climbing( Mind& c ) { return c.bot.desire == Goal::Climb; }
	// a bomber with nothing to fight, bombs left and height in hand
	static bool canBombStructure( Mind& c )
	{
		return bomber( c ) && c.bot.desire == Goal::Roam && theLevel.time_ > c.bot.egressTime && c.sense.height > 700.0f && hasBombs( c );
	}
	static bool wantsWingman( Mind& c )
	{
		return c.bot.leader >= 0 && !c.sense.enemy;
	}
	// the nose comes up at this multiple of the stall speed
	static constexpr float rotateSpeed = 1.12f;

	// the first seconds after the wheels leave the ground
	static bool departing( Mind& c )
	{
		constexpr int departTime = 4000;
		return !c.landed() && theLevel.time_ < c.bot.groundTime + departTime;
	}

	// towards the roaming goal at the present height, a new goal when near it or when it is stale
	static void roam( Mind& c )
	{
		if( theLevel.time_ > c.bot.goalTime || Distance( c.ent->r.currentOrigin, c.bot.goal ) < 2000 )
			Bots::pickGoal( c.ent, c.bot, 9000 );
		c.headFor( c.bot.goal );
		c.holdAltitude();
		c.travelTo( Vec3( c.bot.goal[0], c.bot.goal[1], c.ent->r.currentOrigin[2] ) );
	}

	// nose on the enemy; near the ground only in heading, and gently in tight terrain
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

	// weapons free and closing; after a close pass the plane flies on out before coming round
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

	// keep flying the turn and the climb or dive that the arc choice settled on
	static void flyChosenArc( Mind& c )
	{
		aimAtEnemy( c );
		c.view[YAW] = c.ps.vehicleAngles[YAW] + c.bot.chaseTurn;
		if( c.bot.chasePitch != 0.0f )
			c.view[PITCH] = c.bot.chasePitch;
		if( c.bot.confined )
			c.view[PITCH] = std::clamp( c.view[PITCH], -14.0f, 14.0f );
	}

	// parked where no heading has enough runway: find the nearest spot along the taxiway that has
	static bool findTaxiSpot( Mind& c, const vec3_t origin, float wanted )
	{
		constexpr float taxiStep = 100.0f, taxiMax = 800.0f, edgeRoom = 50.0f;
		for( float dist = taxiStep; dist <= taxiMax; dist += taxiStep )
		{
			float best = 0;
			for( int step = 0; step < 24; step++ )
			{
				const float yaw = step * 15.0f;
				if( BotWorld::runwayRun( origin, yaw, dist ) < dist )
					continue;
				Vec3 level = { 0, yaw, 0 }, dir, spot;
				AngleVectors( level, dir, nullptr, nullptr );
				VectorMA( origin, dist, dir, spot );
				// not a spot on the very edge: the aircraft has to stop and turn there
				bool room = true;
				for( int side = 0; side < 4 && room; side++ )
					room = BotWorld::runwayRun( spot, side * 90.0f, edgeRoom ) >= edgeRoom;
				if( !room )
					continue;
				for( int turn = 0; turn < 12; turn++ )
				{
					const float run = BotWorld::runwayRun( spot, turn * 30.0f, wanted + 25.0f );
					if( run >= wanted && run > best )
					{
						best = run;
						VectorCopy( spot, c.bot.taxiSpot );
					}
				}
			}
			if( best > 0 )
			{
				c.bot.taxiTime = theLevel.time_ + 20000;
				c.bot.takeoffTime = 0;
#ifdef MF_BOT_DEBUG
				if( g_botDebug.integer >= 2 )
					Common::printf( "BOTTAKEOFF t %d bot %d taxi %.0f to %.0f %.0f run %.0f\n", theLevel.time_, c.ent->s.number, dist, c.bot.taxiSpot[0], c.bot.taxiSpot[1], best );
#endif
				return true;
			}
		}
		return false;
	}

	// Chooses the take-off heading: enough runway to reach flying speed, nothing standing on it, and from the lift-off
	// point a climb, straight or turning, that fits the terrain and leads somewhere.
	static void planDeparture( Mind& c )
	{
		constexpr float groundAcceleration = 85.0f;
		constexpr float climbSlope = 0.14f;
		constexpr float departure = 900.0f;
		Vec3 origin;
		VectorCopy( c.ent->r.currentOrigin, origin );
		origin[2] -= c.sense.height;
		if( !BotWorld::runwayAt( origin ) )
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
		bool bestClear = false;
		for( int step = 0; step < 24; step++ )
		{
			const float yaw = step * 15.0f;
			const float run = BotWorld::runwayRun( origin, yaw, 4000 );
			longest = std::max( longest, run );
			if( run < 150 )
				continue;
			Vec3 level = { 0, yaw, 0 }, dir, start;
			trace_t ground;
			AngleVectors( level, dir, nullptr, nullptr );
			VectorCopy( origin, start );
			start[2] += 14;
			const float roll = std::min( run, groundRun + 100.0f );
			BotWorld::pathBlocked( start, dir, roll, c.ent->s.number, ground, 30.0f );
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
				bestClear = rollClear;
				c.bot.takeoffYaw = yaw;
				c.bot.departYaw = yaw + departTurn;
			}
		}
#ifdef MF_BOT_DEBUG
		if( g_botDebug.integer >= 2 )
			Common::printf( "BOTTAKEOFF t %d bot %d veh %d at %.0f %.0f %.0f facing %.0f chose %.0f then %.0f run %.0f need %.0f longest %.0f score %.0f\n", theLevel.time_, c.ent->s.number,
				c.client->vehicle_, origin[0], origin[1], origin[2], c.ps.vehicleAngles[YAW], c.bot.takeoffYaw, c.bot.departYaw, bestRun, groundRun, longest, best );
#endif
		// the run estimate is rough and a runway end is unforgiving: with less than a comfortable length, taxi to a better place
		const float comfortable = groundRun * 1.5f + 150.0f;
		if( bestRun < comfortable && findTaxiSpot( c, origin, comfortable ) )
			return;
		// someone is standing on the run: give them a moment to move before rolling
		constexpr int holdLimit = 8;
		if( best > 0 && !bestClear && c.bot.takeoffHolds < holdLimit )
		{
			c.bot.takeoffHolds++;
			c.bot.holdTime = theLevel.time_ + 1000;
			return;
		}
		if( best > 0 )
			c.bot.takeoffTime = theLevel.time_ + 30000;
	}

	// On the ground: wait when told to hold, plan a departure, taxi to a spot with enough runway if needed, then roll
	// and lift off. Slow, it stops for anyone ahead and keeps off the path of an aircraft on its roll
	static Status takeOff( Mind& c )
	{
		c.direct = true;
		c.bot.groundTime = theLevel.time_;
		if( theLevel.time_ < c.bot.holdTime )
		{
			c.view[YAW] = c.ps.vehicleAngles[YAW];
			c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
			return Status::Running;
		}
		if( theLevel.time_ >= c.bot.taxiTime && theLevel.time_ >= c.bot.takeoffTime && c.speed < 20 )
			planDeparture( c );
		if( theLevel.time_ < c.bot.holdTime )
			return Status::Running;
		if( theLevel.time_ < c.bot.taxiTime )
		{
			constexpr float taxiSpeed = 35.0f;
			Vec3 toSpot;
			VectorSubtract( c.bot.taxiSpot, c.ent->r.currentOrigin, toSpot );
			toSpot[2] = 0;
			c.headFor( c.bot.taxiSpot );
			c.view[PITCH] = 0.0f;
			if( VectorLength( toSpot ) < 40 )
			{
				c.cmd.forwardmove = -moveFull;
				// stopped on the spot: the next plan starts from here
				if( c.speed <= 1 )
					c.bot.taxiTime = 0;
			}
			else
				c.cmd.forwardmove = c.speed < taxiSpeed ? moveFull : c.speed > taxiSpeed * 1.5f ? -moveFull : moveNone;
		}
		else
		{
			c.view[YAW] = theLevel.time_ < c.bot.takeoffTime ? c.bot.takeoffYaw : c.ps.vehicleAngles[YAW];
			// leave the ground with speed in hand: lifting off right at the stall drops the aircraft back
			c.view[PITCH] = c.speed > c.veh.stallspeed * rotateSpeed ? -8.0f : 0.0f;
			if( fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) ) > 6 )
				c.cmd.forwardmove = c.speed < 12 ? moveFull : -moveFull;
		}
		if( c.speed < c.veh.stallspeed * 0.5f )
		{
			vec3_t flat = { 0, c.ps.vehicleAngles[YAW], 0 }, forward;
			AngleVectors( flat, forward, nullptr, nullptr );
			for( int i = 1; i <= theLevel.maxclients_; i++ )
			{
				GameEntity* other = theLevel.getEntity( i );
				if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
					continue;
				Vec3 rel;
				rel = other->r.currentOrigin - c.ent->r.currentOrigin;
				const float dist = VectorNormalize( rel );
				if( dist < 320 && fabs( rel[2] * dist ) < 120 && DotProduct( rel, forward ) > 0.4f )
				{
					c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
					break;
				}
				// stay off the path of an aircraft on its take-off roll, unless already standing in it
				constexpr float rollSpeed = 60.0f, rollAhead = 1500.0f, rollWidth = 250.0f, onTheLine = 60.0f;
				Vec3 track;
				VectorCopy( other->client_->ps_.velocity, track );
				track[2] = 0;
				if( !(other->client_->ps_.ONOFF & OO_LANDED) || VectorNormalize( track ) < rollSpeed )
					continue;
				const float along = -DotProduct( rel, track ) * dist;
				const float aside = sqrt( std::max( 0.0f, dist * dist - along * along ) );
				if( along > 0 && along < rollAhead && aside < rollWidth && aside > onTheLine )
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

	// across the missile's path, then at the last moment a hard pull the other way; the skilled wait longer for it
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

	// hit by someone unseen: turn hard to the side chosen when the hit came
	static Status breakAway( Mind& c )
	{
		c.view[YAW] = c.bot.breakYaw;
		c.holdAltitude();
		return Status::Running;
	}

	// directly away from the enemy
	static Status flee( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW] + 180.0f;
		c.holdAltitude();
		return Status::Running;
	}

	// nose on the target, release when lined up and close with no team mate in the blast, then pull out
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
				Vec3 nose;
				AngleVectors( c.ps.vehicleAngles, nose, nullptr, nullptr );
				const float blast = static_cast<float>( availableWeapons[weapon].damageRadius );
				if( c.ps.weaponNum == slot && DotProduct( nose, c.sense.enemyDir ) > 0.99f && c.sense.enemyDist < 1800 &&
					!BotWeapons::friendlyNear( c.ent, c.sense.enemy->r.currentOrigin, blast * 1.3f + 150.0f ) )
					c.cmd.buttons |= BUTTON_ATTACK_MAIN;
				break;
			}
		}
		if( c.sense.enemyDist < 1000 || c.sense.height < 700 )
			c.bot.egressTime = theLevel.time_ + 6000;
		return Status::Running;
	}

	// fly over the target at height and let the bombing step choose the release
	static Status levelBomb( Mind& c )
	{
		roam( c );
		c.hasDest = false;
		c.bombing = true;
		BotWeapons::bomb( c.ent, c.bot, c.sense.enemy->r.currentOrigin, c.cmd, c.view );
		return Status::Running;
	}

	// towards the enemy at the present height
	static Status stalk( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW];
		c.holdAltitude();
		c.travelTo( c.enemyPosition() );
		return Status::Running;
	}

	// the enemy is ahead and the way is clear: straight at it
	static Status pressAttack( Mind& c )
	{
		engage( c );
		aimAtEnemy( c );
		return Status::Running;
	}

	// on with the chosen turn
	static Status flyArc( Mind& c )
	{
		engage( c );
		flyChosenArc( c );
		return Status::Running;
	}

	// the enemy is off the nose: the short way round, level, climbing or diving, then the long way round, taking the
	// first that is clear of the ground
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

	// no turn towards the enemy is clear: fly on, shooting only if it crosses the nose
	static Status holdCourse( Mind& c )
	{
		engage( c );
		c.attacking = false;
		c.snapShot = fabs( enemyTurn( c ) ) < 6.0f;
		c.view[YAW] = c.ps.vehicleAngles[YAW];
		c.holdAltitude();
		return Status::Running;
	}

	// roam; the height itself comes from the wanted altitude
	static Status gainHeight( Mind& c )
	{
		roam( c );
		return Status::Running;
	}

	// level-bomb a building
	static Status bombStructure( Mind& c )
	{
		GameEntity* structure = Bots::findStructure( c.ent, c.bot );
		if( !structure )
			return Status::Failure;
		roam( c );
		Vec3 target;
		for( int k = 0; k < 3; k++ )
			target[k] = (structure->r.absmin[k] + structure->r.absmax[k]) * 0.5f;
		BotWeapons::bomb( c.ent, c.bot, target, c.cmd, c.view );
		return Status::Running;
	}

	static Status wingman( Mind& c )
	{
		GameEntity* leader = Bots::leaderOf( c.bot );
		if( !leader )
			return Status::Failure;
		// off the leader's wing, on the side given by the client number so two wingmen take different sides
		Vec3 forward, right;
		AngleVectors( leader->client_->ps_.vehicleAngles, forward, right, nullptr );
		VectorMA( leader->r.currentOrigin, 600.0f, forward, c.bot.goal );
		VectorMA( c.bot.goal, (c.ent->s.number & 1) ? 300.0f : -300.0f, right, c.bot.goal );
		c.bot.goalTime = theLevel.time_ + 2000;
		c.headFor( c.bot.goal );
		c.holdAltitude();
		c.travelTo( Vec3( c.bot.goal[0], c.bot.goal[1], c.ent->r.currentOrigin[2] ) );
		return Status::Running;
	}

	// nothing to do: roam
	static Status patrol( Mind& c )
	{
		roam( c );
		return Status::Running;
	}

	// flares against a missile
	static Status countermeasures( Mind& c )
	{
		BotWeapons::flares( c.ent, c.bot, c.sense, c.cmd );
		return Status::Success;
	}

	// when the straight line to the destination is blocked, follow the air path round the obstacle, allowing for the turn radius
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
		Vec3 legDir, legAngles;
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
			Vec3 nextDir, nextAngles;
			next.copyTo( nextDir );
			vectoangles( nextDir, nextAngles );
			bot.cornerAhead = fabs( AngleSubtract( nextAngles[YAW], legAngles[YAW] ) ) > 35.0f && leg.length() < radius * 3.5f;
		}
		return Status::Success;
	}

	// lining up to land: hold the approach height and come back to approach speed
	static Status approachProfile( Mind& c )
	{
		// firm enough to follow the approach height as it comes down towards the runway
		c.view[PITCH] = std::clamp( (c.ent->r.currentOrigin[2] - c.bot.approachHeight) / 12.0f, -25.0f, 16.0f );
		// inbound the plane is already slowing for the final: fast, it can neither come down nor turn onto the centre line
		const float limit = c.veh.stallspeed * (c.bot.landOutbound ? 2.2f : 1.7f);
		if( c.speed > limit )
			c.cmd.forwardmove = -moveFull;
		const bool wantBrake = !c.bot.landOutbound && c.speed > limit * 1.15f;
		if( (c.veh.caps & HC_SPEEDBRAKE) && wantBrake != ((c.ps.ONOFF & OO_SPEEDBRAKE) != 0) )
			c.cmd.buttons |= BUTTON_BRAKE;
		return Status::Success;
	}

	// keep to a height lane where there is little room between floor and ceiling
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
			// right of way: the bot with the higher number gives way, unless it would run into the other from behind;
			// head on there is no time for one to do it alone, so both break
			if( Bots::isBot( i ) && i > c.ent->s.number )
			{
				constexpr float sameWay = 0.5f, headOn = -0.7f;
				Vec3 mine, theirs;
				VectorNormalize2( c.ps.velocity, mine );
				VectorNormalize2( other->client_->ps_.velocity, theirs );
				const float agreement = DotProduct( mine, theirs );
				if( agreement < sameWay && agreement > headOn )
					continue;
			}
			Vec3 predicted;
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
		Vec3	origin;
		float	yaw;
		float	seconds;
	};

	// flies an arc of turnDegrees at the given radius and then straight on, returns the free distance up to horizon
	static float sweepFrom( const Mind& c, const Pose& start, float radius, float turnDegrees, float slope, float horizon, float halfWidth, Pose* finish = nullptr )
	{
		Vec3 flat = { 0, start.yaw, 0 }, forward, right, from;
		AngleVectors( flat, forward, right, nullptr );
		const float side = turnDegrees > 0 ? -1.0f : 1.0f;
		const float total = fabs( turnDegrees );
		const float airspeed = std::max( c.airspeed, 1.0f );
		float travelled = 0;
		VectorCopy( start.origin, from );
		const auto leg = [&]( const vec3_t to ) -> bool
		{
			Vec3 dir;
			trace_t tr;
			VectorSubtract( to, from, dir );
			const float length = VectorNormalize( dir );
			if( length < 1 )
				return true;
			// no trace needed while the leg stays inside one empty cell of the air navigation tree
			if( !AirNav::segmentKnownFree( Vec3( from ), Vec3( to ), halfWidth + 8.0f ) )
			{
				BotWorld::pathBlocked( from, dir, length, c.ent->s.number, tr, halfWidth );
				// already within the corridor's width of a wall: look along a narrower one, and only then along a bare
				// line, so that hugging the wall does not make every heading look open
				if( tr.startsolid && travelled <= 0 && start.seconds <= 0 )
					BotWorld::pathBlocked( from, dir, length, c.ent->s.number, tr, halfWidth * 0.4f );
				if( tr.startsolid && travelled <= 0 && start.seconds <= 0 )
					BotWorld::pathBlocked( from, dir, length, c.ent->s.number, tr );
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
				Vec3 point;
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
			Vec3 point;
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
			Vec3 out = { 0, start.yaw + turnDegrees, 0 }, dir, end;
			AngleVectors( out, dir, nullptr, nullptr );
			dir[2] = slope;
			end = from + dir * ( horizon - travelled );
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

	// how far the aircraft gets from where it is now along a turn of that radius and slope before meeting something
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
			Vec3 flat = { 0, yaw, 0 }, right, left;
			trace_t tr;
			AngleVectors( flat, nullptr, right, nullptr );
			VectorNegate( right, left );
			BotWorld::pathBlocked( ent->r.currentOrigin, left, 1200, ent->s.number, tr );
			const float leftClear = tr.fraction * 1200.0f;
			BotWorld::pathBlocked( ent->r.currentOrigin, right, 1200, ent->s.number, tr );
			const float rightClear = tr.fraction * 1200.0f;
			bot.confined = leftClear + rightClear < 1500 || c.room < 900;

			// the corridor looked along is as wide as the aircraft's own box and a little more: a bomber does not fit
			// where a fighter does
			const float margin = std::max( 60.0f, std::max( c.veh.maxs[0], c.veh.maxs[1] ) * 0.5f + 35.0f );
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

	// slow down, with the air brake if need be, where the plane has to turn tighter than its speed allows
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

	// pull up below the least height, push down under a low ceiling
	static Status floorAndCeiling( Mind& c )
	{
		// a plane lining up to land flies lower than the cruising floor, or it never gets down to the runway
		const float floor = onApproach( c ) ? std::min( c.minHeight, approachFloor ) : c.minHeight;
		if( c.sense.height < floor )
			c.view[PITCH] = std::min( c.view[PITCH], -35.0f );
		else if( c.ceiling < 250 )
			c.view[PITCH] = std::max( c.view[PITCH], c.sense.height > c.minHeight + 150 ? 12.0f : 0.0f );
		return Status::Success;
	}

	// too slow: nose down for speed where there is height to spend, no steep climbs, and out of a stall before anything else
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

	// the enemy this plane is after has its team's flag, or is close to a team mate who carries the other one
	static bool flagEmergency( const Mind& c )
	{
		constexpr float carrierThreatened = 3000.0f;
		if( g_gametype.integer != GT_CTF || !c.sense.enemy || !c.sense.enemy->client_ )
			return false;
		if( c.sense.enemy->client_->ps_.objectives & (OB_REDFLAG|OB_BLUEFLAG) )
			return true;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* mate = theLevel.getEntity( i );
			if( mate == c.ent || !alive( mate ) || !OnSameTeam( c.ent, mate ) || !(mate->client_->ps_.objectives & (OB_REDFLAG|OB_BLUEFLAG)) )
				continue;
			if( Distance( mate->r.currentOrigin, c.sense.enemy->r.currentOrigin ) < carrierThreatened )
				return true;
		}
		return false;
	}

	// The afterburner drinks fuel and makes the aircraft fast where fast is dangerous, so it is a decision, not a
	// reflex. Getting off the ground always has it; so does a moment's recovery from a stall or a slow pull-up, and
	// a climb that a fight makes urgent. Beyond that it needs open air and a reason.
	static bool wantsAfterburner( Mind& c )
	{
		const bool stalling = (c.ps.ONOFF & OO_STALLED) || c.speed < c.veh.stallspeed * 1.15f;
		// Going back to rearm is two things. The landing itself - lining up, the final, the ground - never has it.
		// The trip to the strip while that is still far off may be hurried like any other, further down
		constexpr float stripFar = 6000.0f;
		const bool rearming = c.bot.desire == Goal::Rearm;
		const bool tripToStrip = rearming && c.bot.pad >= 0 && c.bot.landPhase <= 1 && Distance( BotWorld::pads_[c.bot.pad].center, c.ent->r.currentOrigin ) > stripFar;
		if( rearming && !tripToStrip )
			return false;
		// the take-off roll and the first seconds of the climb out
		if( c.landed() || departing( c ) )
			return true;
		// a blip to get flying again, and thrust for a hard pull-up that the speed alone will not carry
		if( stalling || (c.view[PITCH] < -12.0f && c.speed < c.veh.stallspeed * 1.5f) )
			return true;
		// height wanted quickly, with an enemy about; a climb on the way somewhere is flown at military power
		if( c.bot.desire == Goal::Climb && c.sense.enemy )
			return true;
		// nothing more in a canyon or a corridor, or with a corner or an obstacle to fly round: speed there is a crash
		const bool tight = c.bot.confined || c.bot.cornerAhead || theLevel.time_ < c.bot.avoidTime || theLevel.time_ < c.bot.brakeTime;
		if( tight )
			return false;
		// shot at, or locked: some pilots light it and some do not, the cautious more often. With ground near enough
		// to get down behind it is likelier than out in the open sky. Settled once for each incident (each hit, or
		// each stretch of some seconds under a lock) so it does not flicker
		constexpr float coverBelow = 1200.0f;
		if( c.sense.underFire || c.sense.blindsided || c.sense.locked )
		{
			const int incident = c.sense.underFire ? c.bot.hitTime : theLevel.time_ / 6000;
			const int draw = (incident * 7 + c.ent->s.number * 31) % 100;
			const int chance = 25 + static_cast<int>( 40 * c.who().caution ) + (c.sense.height < coverBelow ? 20 : -10);
			if( draw < chance )
				return true;
		}
		// the rest needs open air: height in hand and room overhead
		if( c.sense.height <= c.minHeight * 1.5f || c.ceiling <= 600.0f )
			return false;
		// a missile on its way
		if( c.defending || c.sense.missile || c.sense.launched )
			return true;
		// running away from someone near enough to matter
		constexpr float chased = 4000.0f;
		if( c.bot.desire == Goal::Evade && c.sense.enemy && c.sense.enemyDist < chased )
			return true;
		// the rest is hurrying, which a tank under three tenths cannot afford
		if( c.ps.stats[STAT_FUEL] < static_cast<int>( c.veh.maxfuel ) * 3 / 10 )
			return false;
		// a strip that is still far off, with the fuel to hurry there
		if( tripToStrip )
			return true;
		// the flag: a plane carries none, but it hurries after an enemy who has its team's, and to the fight when
		// its enemy is near a team mate who is carrying one
		if( flagEmergency( c ) )
			return true;
		// a wingman left behind catches up with the leader
		constexpr float farBehind = 4000.0f, farFromFight = 5000.0f;
		if( const GameEntity* leader = Bots::leaderOf( c.bot ); leader && Distance( leader->r.currentOrigin, c.ent->r.currentOrigin ) > farBehind )
			return true;
		// intercepting an enemy that is still far off; once near, the fight is flown at military power
		return c.sense.enemy && c.bot.desire == Goal::Attack && c.sense.enemyDist > farFromFight;
	}

	// shoot when attacking or when the enemy crosses the nose; a step that needs one particular weapon keeps it selected
	static Status fire( Mind& c )
	{
		if( !c.attacking && !c.snapShot && !c.weaponsFree )
			return Status::Success;
		Vec3 fwd;
		AngleVectors( c.ps.vehicleAngles, fwd, nullptr, nullptr );
		const float aimError = acos( std::clamp( DotProduct( fwd, c.sense.enemyDir ), -1.0f, 1.0f ) ) * 180.0f / std::numbers::pi_v<float>;
		BotWeapons::fireWeapons( c.ent, c.bot, c.sense, c.cmd, aimError );
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
		mind.cruise = std::clamp( sense.height + BotWorld::ceilingAbove( ent->r.currentOrigin, ent->s.number ) - 700.0f, 800.0f, 3500.0f );
	else if( bot.desire == Goal::Climb )
		mind.cruise = sense.height + 500.0f;
	else if( !sense.enemy || !sense.enemyAir )
		mind.cruise = planeCruiseHeight - 800.0f * character( bot ).cunning;
	mind.airspeed = VectorLength( ps.velocity );
	mind.ceiling = BotWorld::ceilingAbove( ent->r.currentOrigin, ent->s.number );
	mind.room = sense.height + mind.ceiling;
	mind.minHeight = std::min( planeMinHeight, mind.room * 0.3f );

	cmd.forwardmove = moveFull;
	view[YAW] = ps.vehicleAngles[YAW];
	mind.holdAltitude();

	runTree( tree, mind, "plane" );

	// turrets: look at a nearby enemy aircraft and fire whatever bears, in bursts, holding course meanwhile as a
	// player does with free look. Not while landing, taking off or low, where steering matters more
	constexpr float turretRange = 2200.0f;
	constexpr int turretBurst = 1800, turretPause = 1200;
	// the target is the nearest enemy aircraft, whatever the bomber itself is after: a bomber's own enemy is on the ground
	GameEntity* raider = sense.airThreat && sense.airThreatDist < turretRange ? sense.airThreat : nullptr;
	if( (mind.veh.flags & VF_GUNSTATIONS) && raider && !(ps.ONOFF & OO_LANDED) && bot.desire != Goal::Rearm &&
		sense.height > mind.minHeight && ps.ammo[WP_MACHINEGUN] > 0 && BotWorld::visible( ent, ent->r.currentOrigin, raider ) )
	{
		if( theLevel.time_ > bot.turretTime + turretBurst + turretPause )
			bot.turretTime = theLevel.time_;
		if( theLevel.time_ < bot.turretTime + turretBurst )
		{
			// lead the target by the bullets' flight time
			const float flight = sense.airThreatDist / std::max( 1.0f, static_cast<float>( availableWeapons[mind.veh.weapons[WP_MACHINEGUN]].muzzleVelocity ) );
			Vec3 raiderVelocity;
			VectorCopy( raider->s.pos.trDelta, raiderVelocity );
			const Vec3 aimAt = Vec3( raider->r.currentOrigin ) + raiderVelocity * flight - Vec3( ent->r.currentOrigin );
			Vec3 aim;
			vectoangles( aimAt, aim );
			// a gunner in a bucking bomber is no marksman: each burst is laid off by an error of its own, a few
			// degrees for a poor gunner and under one for the best, so some bursts hit and many pass by
			constexpr float poorGunner = 5.0f, bestGunner = 0.8f;
			const float spread = bestGunner + (poorGunner - bestGunner) * (1.0f - std::clamp( skill( bot ), 0.0f, 1.0f ));
			const int burst = bot.turretTime / 97 + ent->s.number * 13;
			aim[YAW] += spread * (((burst * 37) % 200) / 100.0f - 1.0f);
			aim[PITCH] += spread * (((burst * 91) % 200) / 100.0f - 1.0f);
			if( VehicleData::gunStationsBearing( client->vehicle_, ps.vehicleAngles, aim ) )
			{
				VectorCopy( aim, view );
				cmd.buttons |= BUTTON_FREELOOK | BUTTON_ATTACK;
			}
		}
	}

	// full power is asked for all over the tree; in the air it means military power unless the afterburner is earned
	constexpr int militaryPower = 10;
	const float throttle = signedThrottle( ps, client->vehicle_ );
	if( mind.veh.maxthrottle > militaryPower && (ps.ONOFF & OO_LANDED) )
	{
		// the cruise setting must not push a landed aircraft along
		if( cmd.forwardmove != moveFull && throttle > 0 )
			cmd.buttons |= BUTTON_DECREASE;
	}
	else if( mind.veh.maxthrottle > militaryPower && cmd.forwardmove == moveFull )
	{
		if( !PlaneTasks::wantsAfterburner( mind ) )
			cmd.forwardmove = moveNone;
		// the afterburner key only overrides the set throttle. That is brought to military power underneath, so that
		// letting the afterburner go leaves the engine there and not at wherever it was set on the ground
		if( throttle < militaryPower )
			cmd.buttons |= BUTTON_INCREASE;
		else if( throttle > militaryPower )
			cmd.buttons |= BUTTON_DECREASE;
	}
}
