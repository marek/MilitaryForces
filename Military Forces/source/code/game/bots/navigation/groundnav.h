#ifndef __BOT_GROUNDNAV_H__
#define __BOT_GROUNDNAV_H__

#include "../../q_shared.h"
#include <vector>
#include "danger.h"

// paths over land and over water on a grid of cells (see the .cpp)
class GroundNav
{
public:
	static void reset();
	// false when the goal is out of reach; out then leads towards the nearest point reached
	static bool findPath( const Vec3& from, const Vec3& to, bool water, std::vector<Vec3>& out, const PathDanger& danger = {} );
	// a vehicle following a path could not get on from here in this direction: the way between the two cells is closed
	// for every later search, whatever the trace between their middles said
	static void blockAhead( const Vec3& here, const Vec3& towards );
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
	static float flatDistance( int cell, const Vec3& point );
	static bool search( int start, int goal, const Vec3& to, Kind wanted, const PathDanger& danger, int& end );
	static void expandTowards( int cell, int dx, int dy, const Vec3& to, Kind wanted, const PathDanger& danger );
	static void straighten( Vec3 anchor, const std::vector<Vec3>& raw, Kind wanted, std::vector<Vec3>& out );

	static bool built_;
	static Vec3 mins_;	// the map's corner the grid starts from
	static float cellSize_;
	static int nx_, ny_;
	static std::vector<Kind> kind_;	// per cell: what it is and how high its surface lies
	static std::vector<float> height_;
	static std::vector<float> cost_;	// per cell, for the search in progress: cost so far, where it came from, and which search wrote them
	static std::vector<int> parent_;
	static std::vector<int> stamp_;
	static int generation_;
};

#endif
