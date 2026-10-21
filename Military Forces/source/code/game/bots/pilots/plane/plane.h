#ifndef __BOT_PLANE_H__
#define __BOT_PLANE_H__

#include "../../core/mind.h"
#include "../../navigation/airnav.h"
#include "../../crew/crew.h"
#include "../../world/query.h"
#include "../../navigation/intercept.h"
#include <algorithm>
#include <cmath>

using behavior::Status;

// What a plane's pilot does: the conditions and steps of the plane tree. The steps choose where to point the nose;
// navigation, the manoeuvre planner and the safety steps at the end may overrule them to keep the aircraft flying.
// The conditions are here; the steps of each branch are in the file named after it, and the tree in tree.cpp.
struct PlaneTasks
{
	using Mind = Bots::Mind;

	using Goal = Bots::Goal;

	using Role = Bots::Role;

	static bool bomber( const Mind& c ) { return c.sense.can.groundStrike; }

	static bool hasBombs( const Mind& c )
	{
		return BotWeapons::hasWeaponType( c.client->vehicle_, c.ps, WT_IRONBOMB ) || BotWeapons::hasWeaponType( c.client->vehicle_, c.ps, WT_GUIDEDBOMB );
	}

	// how far the nose must turn to point at the enemy, signed
	static float enemyTurn( const Mind& c ) { return AngleSubtract( c.sense.enemyAngles[YAW], c.ps.vehicleAngles[YAW] ); }

	// free flight: not in a step that works the controls itself (take-off, final, roll-out)
	static bool flying( Mind& c ) { return !c.controls.claimed(); }

	static bool landing( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.rearm.pad >= 0; }

	static bool onGround( Mind& c ) { return c.landed(); }

	static bool onApproach( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.rearm.phase == 1; }
	// a missile within six seconds; not acted on once committed to a final approach

	static bool missileInbound( Mind& c )
	{
		return c.sense.missile && c.sense.missileTime <= 6.0f && !c.landed() && !(c.bot.desire == Goal::Rearm && c.bot.rearm.phase >= 2);
	}

	static bool evading( Mind& c ) { return c.bot.desire == Goal::Evade && (c.sense.blindsided || c.sense.enemy); }

	static bool blindsided( Mind& c ) { return c.sense.blindsided; }
	static bool outranged( Mind& c ) { return c.sense.outranged; }
	// attacking, and not just leaving after a pass

	static bool enemyHasTurrets( Mind& c ) { return c.sense.enemy && (availableVehicles[c.sense.enemy->client_->vehicle_].flags & VF_GUNSTATIONS); }
	// just through a firing pass on an aircraft: on out before coming round. From one with turrets, until clear of them

	static bool extending( Mind& c )
	{
		if( !c.sense.enemy || !c.sense.enemyAir || c.bot.desire != Goal::Attack || theLevel.time_ > c.bot.pursuit.egressTime )
			return false;
		return !enemyHasTurrets( c ) || Distance( c.sense.enemy->r.currentOrigin, c.ent->r.currentOrigin ) < turretClear;
	}

	// an air enemy is turned back on as soon as the fighter is clear, whatever is left of the time allowed for it
	static bool engaging( Mind& c )
	{
		return c.sense.enemy && c.bot.desire == Goal::Attack && (theLevel.time_ > c.bot.pursuit.egressTime || (c.sense.enemyAir && !extending( c )));
	}

	static bool canLevelBomb( Mind& c ) { return bomber( c ) && !c.sense.enemyAir && hasBombs( c ); }
	// bold or cunning bomber pilots dive on a ground target when high and close enough

	static bool canDiveBomb( Mind& c )
	{
		const bool diver = c.who().aggression > 0.8f || c.who().cunning > 0.75f;
		return canLevelBomb( c ) && diver && c.sense.height > 900 && c.sense.enemyDist < c.sense.height * 2.2f;
	}

	// a stealth aircraft not yet seen closes in without showing itself
	static bool stalking( Mind& c ) { return c.sense.stealth && !c.sense.revealed && c.sense.enemyDist > 2500; }

	static bool onNose( Mind& c ) { return fabs( enemyTurn( c ) ) < Bots::mistakes( c.bot ).pressAngle; }
	// in tight terrain, nothing of the world between the nose and the enemy

	static bool fireLineClear( Mind& c )
	{
		if( !c.bot.avoid.confined || theLevel.time_ < c.bot.pursuit.chaseTime )
			return true;
		trace_t line;
		Vec3 toEnemy;
		VectorCopy( c.sense.enemyDir, toEnemy );
		return !(BotWorld::pathBlocked( c.ent->r.currentOrigin, toEnemy, std::min( c.sense.enemyDist, 1500.0f ), c.ent->s.number, line, 60.0f ) &&
			line.entityNum == ENTITYNUM_WORLD && !line.startsolid);
	}

	// a turn chosen a moment ago is flown out before another is chosen
	static bool arcCommitted( Mind& c ) { return theLevel.time_ < c.bot.pursuit.chaseTime; }

	static bool climbing( Mind& c ) { return c.bot.desire == Goal::Climb; }
	// a bomber with nothing to fight, bombs left and height in hand

	static bool canBombStructure( Mind& c )
	{
		return bomber( c ) && c.bot.desire == Goal::Roam && theLevel.time_ > c.bot.pursuit.egressTime && c.sense.height > 700.0f && hasBombs( c );
	}

	static bool wantsWingman( Mind& c )
	{
		return c.bot.leader >= 0 && !c.sense.enemy;
	}

	// the nose comes up at this multiple of the stall speed
	static constexpr float rotateSpeed = 1.12f;

	// holding short for a moment, taxiing to a spot with more runway, and a departure planned and not yet flown
	static bool holdingShort( Mind& c ) { return theLevel.time_ < c.bot.takeoff.holdTime; }
	static bool taxiing( Mind& c ) { return theLevel.time_ < c.bot.takeoff.taxiTime; }
	static bool planned( const Mind& c ) { return theLevel.time_ < c.bot.takeoff.planTime; }
	static bool linedUp( const Mind& c );

	// the first seconds after the wheels leave the ground
	static bool departing( Mind& c )
	{
		constexpr int departTime = 4000;
		return !c.landed() && theLevel.time_ < c.bot.takeoff.groundTime + departTime;
	}

	// towards the roaming goal at the present height, a new goal when near it or when it is stale
	static void roam( Mind& c );

	// nose on the enemy; near the ground only in heading, and gently in tight terrain
	static void aimAtEnemy( Mind& c );

	// weapons free and closing; after a close pass the plane flies on out before coming round
	static void engage( Mind& c );
	static constexpr float interceptFrom = 3000.0f;	// an aircraft further off than this is intercepted
	static Vec3 interceptPoint( const Mind& c );

	// keep flying the turn and the climb or dive that the arc choice settled on
	static void flyChosenArc( Mind& c );

	// the ground roll to lift-off, and a comfortable length of runway for it
	static float groundRun( const completeVehicleData_t& veh );
	static float comfortableRun( const completeVehicleData_t& veh );

	// whoever is low in the way of a run along a heading, now or within the seconds foreseen; nullptr when nobody
	static const GameEntity* trafficOnWay( const Mind& c, const Vec3& from, float yaw, float length, float seconds );
	static const GameEntity* turnInTheWay( const Mind& c, float yaw, float onward, float seconds );
	static bool spotFree( const Mind& c, const Vec3& spot );
	static bool inSomeonesRun( const Mind& c, const Vec3& spot );
	static bool groundTaxiable( const Mind& c, const Vec3& from, const Vec3& to );

	// the run from here will not do: find the nearest spot on the runway that has one
	static bool findTaxiSpot( Mind& c, const vec3_t origin, float wanted );
	static float longestRun( const Vec3& spot, float wanted );
	static float yawOf( const Vec3& dir );

	// Chooses the take-off heading: enough runway to reach flying speed, nothing standing on it, and from the lift-off
	// point a climb, straight or turning, that fits the terrain and leads somewhere.
	static void planDeparture( Mind& c );
	// what a take-off needs from a runway
	struct Roll
	{
		float	groundRun = 0;
		float	radius = 0;	// turn radius just after lift-off
		float	comfortable = 0;	// a comfortable length of runway
	};
	// one heading looked at for the take-off
	struct Heading
	{
		float	yaw = 0;
		float	run = 0;
		bool	rollClear = false;
		float	departTurn = 0;
		float	score = 0;
	};
	// on the grass beside a runway: taxi back onto it
	static void offRunway( Mind& c, const Vec3& origin );
	static Roll rollFor( const Mind& c );
	static Heading scoreHeading( const Mind& c, const Vec3& origin, float yaw, const Roll& roll );
	static bool rollIsClear( const Mind& c, const Vec3& origin, const Vec3& dir, float length );
	static void settleDeparture( Mind& c, const Vec3& origin, const Heading& best, const Roll& roll );

	// On the ground: wait when told to hold, plan a departure, taxi to a spot with enough runway if needed, line up,
	// wait for the run to be clear, then roll and lift off. Slow, it stops for anyone in its way and keeps off the
	// path of an aircraft on its roll
	static void prepareTakeOff( Mind& c );
	static Status holdShort( Mind& c );
	static Status taxiToSpot( Mind& c );
	static Status lineUpAndRoll( Mind& c );
	static Status giveWay( Mind& c );
	static bool waitForTraffic( Mind& c );
	static bool onTakeOffRollTowards( const GameEntity* other, const Vec3& rel, float dist );

	// follow the departure that was planned on the ground; the manoeuvre check still vets it
	static Status depart( Mind& c );

	// across the missile's path, then at the last moment a hard pull the other way; the skilled wait longer for it
	static Status defendMissile( Mind& c );

	// hit by someone unseen: turn hard to the side chosen when the hit came
	static Status breakAway( Mind& c );

	// directly away from the enemy
	static Status flee( Mind& c );
	static Status leaveReach( Mind& c );

	// nose on the target, release when lined up and close with no team mate in the blast, then pull out
	static Status diveBomb( Mind& c );

	// fly over the target at height and let the bombing step choose the release
	static Status levelBomb( Mind& c );
	static bool flyToRunIn( Mind& c, const Vec3& target );

	// towards the enemy at the present height
	static Status stalk( Mind& c );

	// the enemy is ahead and the way is clear: straight at it
	static Status pressAttack( Mind& c );

	// on with the chosen turn
	static Status flyArc( Mind& c );

	// the enemy is off the nose: the short way round, level, climbing or diving, then the long way round, taking the
	// first that is clear of the ground
	static Status chooseArc( Mind& c );

	// no turn towards the enemy is clear: fly on, shooting only if it crosses the nose
	static Status holdCourse( Mind& c );

	// straight on at full power, at the height it has: distance is what gets it out of the guns it has just flown past
	static Status extend( Mind& c );

	// roam; the height itself comes from the wanted altitude
	static Status gainHeight( Mind& c );

	// level-bomb a building
	static Status bombStructure( Mind& c );

	static Status wingman( Mind& c );

	// nothing to do: roam
	static Status patrol( Mind& c );


	// when the straight line to the destination is blocked, follow the air path round the obstacle, allowing for the turn radius
	static Status navigate( Mind& c );

	// lining up to land: hold the approach height and come back to approach speed
	static Status approachProfile( Mind& c );

	// keep to a height lane where there is little room between floor and ceiling
	static Status midChannel( Mind& c );


	// true when another aircraft is predicted to be near this point at that time
	static bool trafficAt( const Mind& c, const vec3_t point, float seconds );

	struct Pose
	{
		Vec3	origin;
		float	yaw;
		float	seconds;
	};
	static bool climbOutTurn( const Mind& c, const Pose& liftOff, float radius, float& turn );

	// flies an arc of turnDegrees at the given radius and then straight on, returns the free distance up to horizon
	static float sweepFrom( const Mind& c, const Pose& start, float radius, float turnDegrees, float slope, float horizon, float halfWidth, Pose* finish = nullptr );

	// how far the aircraft gets from where it is now along a turn of that radius and slope before meeting something
	static float sweep( const Mind& c, float radius, float turnDegrees, float slope, float horizon, float halfWidth, Pose* finish = nullptr );

	// a manoeuvre is only worth flying if there is a way on from where it ends: straight ahead, over the top, or a turn back
	static bool wayOut( const Mind& c, const Pose& end, float brakedRadius, float horizon );

	// Plans the next second or two of flight. The heading the pilot wants is flown ahead as an arc at the aircraft's turn
	// radius followed by a straight; if that does not fit, other manoeuvres are tried in order of preference.
	static Status probeTerrain( Mind& c );

	// one look ahead of the manoeuvre planner
	struct Probe
	{
		float	yaw = 0;	// the heading now
		float	margin = 0;	// half the corridor width
		float	wantYaw = 0;
		float	wantPitch = 0;
		float	wantTurn = 0;
		float	wantSlope = 0;
		bool	slow = false;	// near the stall
		float	horizon = 0;	// how far ahead to look
		float	radius = 0;	// the turn radius now
		float	brakedRadius = 0;	// and with the air brake out
		bool	canBrake = false;
	};
	struct Candidate { float turn; float slope; float pitch; float penalty; };
	struct Candidates
	{
		std::array<Candidate, 34> list;
		size_t count = 0;
	};
	struct Choice
	{
		const Candidate*	chosen = nullptr;
		float				free = 0;	// how far it flies clear
		bool				braked = false;
	};
	static void lookAhead( Mind& c );
	static bool confinedHere( const Mind& c );
	static Probe probeFor( const Mind& c );
	static void checkWantedLine( Mind& c, const Probe& probe );
	static void listManoeuvres( const Mind& c, const Probe& probe, Candidates& out );
	static Choice chooseManoeuvre( const Mind& c, const Probe& probe, const Candidates& candidates, float width );
	static Choice flyCandidate( const Mind& c, const Probe& probe, const Candidate& candidate, float width );

	// slow down, with the air brake if need be, where the plane has to turn tighter than its speed allows
	static Status cornerSpeed( Mind& c );

	// pull up below the least height, push down under a low ceiling
	static Status floorAndCeiling( Mind& c );

	// too slow: nose down for speed where there is height to spend, no steep climbs, and out of a stall before anything else
	static Status keepEnergy( Mind& c );

	// the enemy this plane is after has its team's flag, or is close to a team mate who carries the other one
	static bool flagEmergency( const Mind& c );

	// The afterburner drinks fuel and makes the aircraft fast where fast is dangerous, so it is a decision, not a
	// reflex. Getting off the ground always has it; so does a moment's recovery from a stall or a slow pull-up, and
	// a climb that a fight makes urgent. Beyond that it needs open air and a reason.
	static bool wantsAfterburner( Mind& c );
	static Status decideAfterburner( Mind& c );

	// shoot when attacking or when the enemy crosses the nose; a step that needs one particular weapon keeps it selected
	static Status fire( Mind& c );

	using State = Bots::State;

	static behavior::Tree<Mind> build();
	using Tree = behavior::Tree<Mind>;
	using Node = behavior::Node<Mind>;
	static const Node* decide( Tree& t );
	static const Node* takeOffRoll( Tree& t );
	static const Node* landAndRearm( Tree& t );
	static const Node* attack( Tree& t );
	static const Node* freeFlight( Tree& t );
};

#endif
