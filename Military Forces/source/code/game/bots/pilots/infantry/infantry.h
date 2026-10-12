#ifndef __BOT_INFANTRY_H__
#define __BOT_INFANTRY_H__

#include "../../core/mind.h"

using behavior::Status;

// A soldier on foot wants a vehicle: the conditions and steps of the infantry tree. The conditions are here; the
// steps are in boarding.cpp and walking.cpp, the tree and the rider in tree.cpp.
struct InfantryTasks
{
	using Mind = Bots::Mind;

	// the nearest thing with a seat: an empty vehicle, or a team mate's stopped vehicle. A free gunner's seat is worth a longer walk
	static void findRide( Mind& c );

	// the nearest pad, where vehicles appear
	static void nearestPad( Mind& c );

	// close enough to climb in, and not just after the last try
	static bool rideInReach( Mind& c )
	{
		return c.target && Distance( c.ent->r.currentOrigin, c.target->r.currentOrigin ) < g_enterRange.value * 0.9f && theLevel.time_ > c.bot.commandTime;
	}

	static bool enemyClose( Mind& c ) { return c.sense.enemy && c.sense.enemyDist < 900 && !c.sense.enemyAir; }

	static bool seesRide( Mind& c ) { return c.target != nullptr; }
	// on foot too long, or with no pad anywhere near: start again in a vehicle. The patient wait longer

	static bool stranded( Mind& c )
	{
		const int patience = 20000 + static_cast<int>( 30000 * c.who().patience );
		return theLevel.time_ > c.bot.spawnTime + patience || (c.nearPad < 0 && theLevel.time_ > c.bot.spawnTime + 8000) || c.nearPadDist > 6000;
	}

	static bool padAway( Mind& c ) { return c.nearPad >= 0 && c.nearPadDist > 150; }

	static bool moving( Mind& c ) { return !c.controls.claimed(); }

	// the enter key
	static Status board( Mind& c );

	static Status shoot( Mind& c );

	static Status walkToRide( Mind& c );

	// choose a vehicle and give up this life for it
	static Status respawnInVehicle( Mind& c );

	static Status walkToPad( Mind& c );

	// nothing to walk to: short walks around
	static Status wander( Mind& c );

	static Status navigate( Mind& c );

	// face the course and walk; when not getting anywhere, sidestep for a moment and plan the path again
	static Status walk( Mind& c );

	static behavior::Tree<Mind> build();
};

#endif
