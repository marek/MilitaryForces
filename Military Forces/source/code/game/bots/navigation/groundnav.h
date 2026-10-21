#ifndef __BOT_GROUNDNAV_H__
#define __BOT_GROUNDNAV_H__

#include "../../q_shared.h"
#include <vector>
#include "danger.h"

// paths over land and over water in a quadtree of the ground (see the .cpp)
class GroundNav
{
public:
	static void reset();
	// false when the goal is out of reach; out then leads towards the nearest point reached
	static bool findPath( const Vec3& from, const Vec3& to, bool water, std::vector<Vec3>& out, const PathDanger& danger = {} );
	// a vehicle can drive the straight line from a to b: over land (or water), no edge higher than it steps, no slope
	// steeper than it climbs, nothing in the way
	static bool lineOpen( const Vec3& a, const Vec3& b, bool water );
	// a vehicle following a path could not get on from here in this direction: the way between the two places is
	// closed for every later search, whatever the ground looked like
	static void blockAhead( const Vec3& here, const Vec3& towards );
	static int cellCount();
	static int knownCount();

private:
	static void straighten( Vec3 anchor, const std::vector<Vec3>& raw, bool water, std::vector<Vec3>& out );
};

#endif
