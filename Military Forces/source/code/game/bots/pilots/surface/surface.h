#ifndef __BOT_SURFACE_H__
#define __BOT_SURFACE_H__

#include "../../core/mind.h"
#include "../../crew/crew.h"

using behavior::Status;

// What a driver or a boat's helmsman does: the conditions and the steps of the surface behaviour tree. The conditions
// are here; the steps of each branch are in the file named after it, and the tree in tree.cpp.
struct SurfaceTasks
{
	using Mind = Bots::Mind;

	using Goal = Bots::Goal;

	static bool groundEnemy( const Mind& c ) { return c.sense.enemy && !c.sense.enemyAir; }

	static bool hidingFromAircraft( Mind& c ) { return c.bot.desire == Goal::Hide && c.sense.airThreat; }
	// a cautious gunner at middle range fights from behind a rise instead of in the open

	static bool wantsHullDown( Mind& c )
	{
		return c.bot.desire == Goal::Attack && groundEnemy( c ) && !c.boat && c.sense.enemyDist > 900 && c.sense.enemyDist < 4500 && c.who().caution > 0.5f;
	}

	static bool rearming( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.rearm.pad >= 0; }

	static bool evading( Mind& c ) { return c.bot.desire == Goal::Evade && c.sense.enemy; }

	// a boat fights only what it can get at: another boat, or one in reach of its guns from the water
	static bool engaging( Mind& c )
	{
		if( !groundEnemy( c ) || c.bot.desire == Goal::Capture )
			return false;
		return !c.boat || (availableVehicles[c.sense.enemy->client_->vehicle_].cat & CAT_BOAT) || c.sense.enemyDist < weaponReach( c );
	}

	static bool reversing( Mind& c ) { return theLevel.time_ < c.bot.stuck.reverseTime; }

	static bool hasEnemy( Mind& c ) { return c.sense.enemy != nullptr; }

	static bool hasLeader( Mind& c ) { return c.bot.leader >= 0 && c.bot.desire != Goal::Capture; }

	// is there water under the point that far along the hull's line (negative: behind)? drop is set when the ground
	// there is missing or falls away too steeply to drive down
	static bool waterAt( const Mind& c, const vec3_t forward, float reach, bool& drop )
	{
		Vec3 ahead, below;
		trace_t water;
		VectorMA( c.ent->r.currentOrigin, reach, forward, ahead );
		ahead[2] += 120;
		below = ahead;
		below[2] -= 500;
		GameHost::trace( &water, ahead, nullptr, nullptr, below, c.ent->s.number, MASK_SOLID|MASK_WATER, false );
		drop = water.fraction >= 1.0f || c.ent->r.currentOrigin[2] - water.endpos[2] > fabs( reach ) * 0.65f;
		return water.fraction < 1.0f && (water.contents & MASK_WATER);
	}

	// backing out of trouble must not end in the water
	static bool waterBehind( Mind& c )
	{
		if( c.boat || !reversing( c ) )
			return false;
		vec3_t forward, flat = { 0, c.ps.vehicleAngles[YAW], 0 };
		bool drop;
		AngleVectors( flat, forward, nullptr, nullptr );
		return waterAt( c, forward, -200.0f, drop );
	}

	// stop once the place steered for is this close
	static void holdNear( Mind& c, float radius );

	// a spot out of the aircraft's sight, looked for again every few seconds
	static Status driveToCover( Mind& c );

	// no cover known: straight away from it
	static Status runFromAircraft( Mind& c );

	// a spot from which only the turret shows to the enemy
	static Status hullDown( Mind& c );

	// onto the pad and wait there until full
	static Status driveToPad( Mind& c );

	// an aircraft on the ground within gun range: no need to drive up to it
	static bool canShellAirfield( Mind& c )
	{
		constexpr float nearest = 1200.0f;
		constexpr float farthest = 6000.0f;
		if( c.boat || !c.sense.enemy || c.sense.enemyAir || c.bot.desire == Goal::Capture || c.bot.desire == Goal::Evade )
			return false;
		if( !(availableVehicles[c.sense.enemy->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) )
			return false;
		return c.sense.enemyDist > nearest && c.sense.enemyDist < farthest && BotWeapons::hasWeaponType( c.client->vehicle_, c.ps, WT_BALLISTICGUN );
	}

	// stop and lob shells at it from where we are, at the place it was last seen if it is out of sight
	static Status shellAirfield( Mind& c );

	// straight away from the enemy
	static Status withdraw( Mind& c );

	// close to gun range and stop there to shoot
	static Status engage( Mind& c );

	static bool walkerWaiting( Mind& c )
	{
		constexpr float pickupRange = 1500.0f, beside = 200.0f;
		// boats and wheeled vehicles stop for a team mate on foot; tanks do not
		const bool givesLifts = c.veh.seats > 1 && (c.boat || (c.veh.caps & HC_WHEELS));
		c.target = givesLifts && !c.sense.enemyVisible ? BotSenses::walkerNear( c.ent, c.bot, pickupRange, beside ) : nullptr;
		return c.target != nullptr;
	}

	// pull up beside a team mate on foot and wait so they can climb aboard
	static Status pickUpWalker( Mind& c );

	// an unarmed transport with someone aboard is a taxi
	static bool carryingPassenger( Mind& c )
	{
		return !c.boat && (c.veh.caps & HC_WHEELS) && !VehicleData::vehicleHasTurret( c.client->vehicle_ ) && BotSenses::ridersOf( c.ent ) > 0;
	}

	// take the passenger to the nearest free vehicle we know of, otherwise home to base, and stop there
	static Status deliverPassenger( Mind& c );

	// short of fuel, health or ammunition and a pickup that helps is lying nearby: go and get it
	static Status driveToSupplies( Mind& c );

	// drive with the leader, stopping a little short
	static Status convoy( Mind& c );

	// nothing else to do: on to the roaming or capture goal
	static Status driveToGoal( Mind& c );

	// turn the wished course into one that follows the ground path round obstacles
	static Status navigate( Mind& c );

	// a wall, water, a drop, shore for a boat, or no progress for a while: back out and try another way
	static Status watchAhead( Mind& c );
	static bool wallAhead( const Mind& c, const vec3_t dir, float reach );
	static float cornerSpeed( const Mind& c, float turnLeft );
	static float weaponReach( const Mind& c );
	static bool keepFiringSpot( Mind& c );
	static bool findFiringSpot( Mind& c );
	static bool waterOrDropAhead( const Mind& c, const vec3_t forward );
	static bool shoreAhead( const Mind& c, const vec3_t forward );
	static bool noProgress( Mind& c );
	static void backOut( Mind& c );

	// water behind: forward again if the way is open, turning as it was while backing
	static Status creepForward( Mind& c );

	static Status reverse( Mind& c );

	// turn the hull onto the course; drive unless holding
	static Status steer( Mind& c );

	// lay the gun on the enemy; a shell is led by the target's movement and raised for its drop
	static Status gunnery( Mind& c );

	static behavior::Tree<Mind> build();
};

#endif
