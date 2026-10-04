#ifndef __BOT_GROUNDNAV_H__
#define __BOT_GROUNDNAV_H__

#include "../q_shared.h"
#include <vector>

class GroundNav
{
public:
	static void reset();
	static bool findPath( const Vec3& from, const Vec3& to, bool water, std::vector<Vec3>& out );
	static int cellCount();
	static int knownCount();

private:
	enum Kind : signed char { Unknown = 0, Land = 1, Water = 2, Blocked = -1 };

	static void build();
	static int cellAt( const Vec3& point );
	static Vec3 centre( int cell );
	static Kind evaluate( int cell, float referenceHeight );
	static bool passable( int from, int to, Kind wanted );
	static bool lineOpen( const Vec3& a, const Vec3& b, Kind wanted );

	static bool built_;
	static Vec3 mins_;
	static float cellSize_;
	static int nx_, ny_;
	static std::vector<Kind> kind_;
	static std::vector<float> height_;
	static std::vector<float> cost_;
	static std::vector<int> parent_;
	static std::vector<int> stamp_;
	static int generation_;
};

#endif
