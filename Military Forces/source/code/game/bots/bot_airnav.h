#ifndef __BOT_AIRNAV_H__
#define __BOT_AIRNAV_H__

#include "../q_shared.h"
#include <vector>

// paths through the air in an octree of free space (see the .cpp)
class AirNav
{
public:
	static void reset();
	// false when the goal is out of reach; out then leads towards the nearest point reached. With a turn radius,
	// the path favours space the aircraft can turn in
	static bool findPath( const Vec3& from, const Vec3& to, std::vector<Vec3>& out, const Vec3& heading = Vec3(), float turnRadius = 0.0f );
	// nothing of the world on the straight line, for a body of that width
	static bool clearLine( const Vec3& a, const Vec3& b, float halfWidth );
	// both ends well inside one free cube: clear without tracing
	static bool segmentKnownFree( const Vec3& a, const Vec3& b, float margin );
	static int cellCount();
	static int freeCount();
};

#endif
