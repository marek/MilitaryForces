#ifndef __BOT_AIRNAV_H__
#define __BOT_AIRNAV_H__

#include "../q_shared.h"
#include <vector>

class AirNav
{
public:
	static void reset();
	static bool findPath( const Vec3& from, const Vec3& to, std::vector<Vec3>& out );
	static bool clearLine( const Vec3& a, const Vec3& b, float halfWidth );
	static int cellCount();
	static int freeCount();

private:
	static void build();
	static int cellAt( const Vec3& point );
	static Vec3 centre( int cell );
	static bool isFree( int cell );
	static int nearestFree( const Vec3& point );

	static bool built_;
	static Vec3 mins_;
	static float cellSize_;
	static int nx_, ny_, nz_;
	static std::vector<signed char> state_;
	static std::vector<float> cost_;
	static std::vector<int> parent_;
	static std::vector<int> stamp_;
	static int generation_;
};

#endif
