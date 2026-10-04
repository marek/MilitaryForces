#ifndef __BOT_AIRNAV_H__
#define __BOT_AIRNAV_H__

#include "../q_shared.h"
#include <vector>

class AirNav
{
public:
	static void reset();
	static bool findPath( const Vec3& from, const Vec3& to, std::vector<Vec3>& out, const Vec3& heading = Vec3(), float turnRadius = 0.0f );
	static bool clearLine( const Vec3& a, const Vec3& b, float halfWidth );
	static bool segmentKnownFree( const Vec3& a, const Vec3& b, float margin );
	static int cellCount();
	static int freeCount();
};

#endif
