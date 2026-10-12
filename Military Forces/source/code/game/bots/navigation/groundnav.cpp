#include "../../g_local.h"
#include "../../g_level.h"
#include "groundnav.h"
#include <algorithm>
#include <cmath>

// Paths over the ground and over water: a flat grid of cells laid over the map, each found out to be land, water
// or blocked the first time a search touches it, and an A* search across it.
bool GroundNav::built_ = false;
Vec3 GroundNav::mins_;
float GroundNav::cellSize_ = 160.0f;
int GroundNav::nx_ = 0;
int GroundNav::ny_ = 0;
std::vector<GroundNav::Kind> GroundNav::kind_;
std::vector<float> GroundNav::height_;
std::vector<float> GroundNav::cost_;
std::vector<int> GroundNav::parent_;
std::vector<int> GroundNav::stamp_;
int GroundNav::generation_ = 0;

namespace
{
	// a large map gets larger cells instead of more of them; a search that has looked at this many cells gives the
	// way to the nearest point it reached
	constexpr int maxCells = 500000;
	constexpr int maxExpansions = 8000;
	// one bit per neighbour direction: has the edge been traced, and was it open
	std::vector<unsigned char> edgeKnown;
	std::vector<unsigned char> edgeOpen;
	// the steepest rise between neighbouring cells a vehicle drives up, as a share of the cell size
	constexpr float maxStep = 0.55f;
	// what stands in a vehicle's way: solid things and the invisible walls mappers put up for players and for bots,
	// but not other vehicles, which move on
	constexpr int wayMask = MASK_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP;
	// how far above the ground the way between two cells is looked along: over the bumps of rough ground, under
	// the top of anything a vehicle cannot step onto
	constexpr float wayClearance = 12.0f;
	std::vector<std::pair<float, int>> openHeap;
	std::vector<Vec3> rawPath;
}

void GroundNav::reset()
{
	built_ = false;
	kind_.clear();
	height_.clear();
	cost_.clear();
	parent_.clear();
	stamp_.clear();
	nx_ = ny_ = 0;
}

// size the grid to the map; nothing is traced until a search asks
void GroundNav::build()
{
	built_ = true;
	Vec3 worldMins, worldMaxs;
	CollisionModel::modelBounds( 0, worldMins, worldMaxs );
	mins_ = Vec3( worldMins );
	cellSize_ = 160.0f;
	for( ;; )
	{
		nx_ = std::max( 1, static_cast<int>( ceil( (worldMaxs[0] - worldMins[0]) / cellSize_ ) ) );
		ny_ = std::max( 1, static_cast<int>( ceil( (worldMaxs[1] - worldMins[1]) / cellSize_ ) ) );
		if( static_cast<long long>( nx_ ) * ny_ <= maxCells )
			break;
		cellSize_ *= 1.25f;
	}
	const size_t total = static_cast<size_t>( nx_ ) * ny_;
	kind_.assign( total, Unknown );
	edgeKnown.assign( total, 0 );
	edgeOpen.assign( total, 0 );
	height_.assign( total, 0.0f );
	cost_.assign( total, 0.0f );
	parent_.assign( total, -1 );
	stamp_.assign( total, 0 );
	generation_ = 0;
}

int GroundNav::cellCount()
{
	return nx_ * ny_;
}

int GroundNav::knownCount()
{
	return static_cast<int>( std::count_if( kind_.begin(), kind_.end(), []( Kind kind ) { return kind != Unknown; } ) );
}

// the cell a point is over, or -1 outside the map
int GroundNav::cellAt( const Vec3& point )
{
	const int x = static_cast<int>( floor( (point.x() - mins_.x()) / cellSize_ ) );
	const int y = static_cast<int>( floor( (point.y() - mins_.y()) / cellSize_ ) );
	if( x < 0 || y < 0 || x >= nx_ || y >= ny_ )
		return -1;
	return y * nx_ + x;
}

Vec3 GroundNav::centre( int cell )
{
	const int x = cell % nx_;
	const int y = cell / nx_;
	return Vec3( mins_.x() + (x + 0.5f) * cellSize_, mins_.y() + (y + 0.5f) * cellSize_, height_[cell] );
}

// what a cell is, found by looking down at its middle from a little above the height the search comes from (so the
// floor under a bridge and the bridge deck are told apart by where the search is)
GroundNav::Kind GroundNav::evaluate( int cell, float referenceHeight )
{
	if( kind_[cell] != Unknown )
		return kind_[cell];

	const int x = cell % nx_;
	const int y = cell / nx_;
	// first from just above a vehicle's height, so that a roof over the cell (a hangar, a bunker with a flag in it)
	// is not taken for its floor; where that starts inside rising ground, from higher up
	constexpr float underRoof = 100.0f, overRise = 260.0f;
	Vec3 top = { mins_.x() + (x + 0.5f) * cellSize_, mins_.y() + (y + 0.5f) * cellSize_, referenceHeight + underRoof };
	vec3_t bottom = { top[0], top[1], referenceHeight - 700.0f };
	trace_t tr;
	GameHost::trace( &tr, top, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER, false );
	if( tr.startsolid )
	{
		top[2] = referenceHeight + overRise;
		GameHost::trace( &tr, top, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER, false );
	}
	if( tr.startsolid || tr.fraction >= 1.0f )
	{
		kind_[cell] = Blocked;
		return Blocked;
	}
	height_[cell] = tr.endpos[2];
	kind_[cell] = (tr.contents & MASK_WATER) ? Water : Land;
	return kind_[cell];
}

// can a vehicle go from one cell to its neighbour: the right kind, not too steep, and nothing standing between
// them. The trace for each pair is made once and remembered
bool GroundNav::passable( int from, int to, Kind wanted )
{
	if( evaluate( to, height_[from] ) != wanted )
		return false;
	if( wanted == Land && fabs( height_[to] - height_[from] ) > cellSize_ * maxStep )
		return false;

	const int dx = to % nx_ - from % nx_, dy = to / nx_ - from / nx_;
	const unsigned char bit = static_cast<unsigned char>( 1 << ((dy + 1) * 3 + (dx + 1) - ((dy + 1) * 3 + (dx + 1) > 4 ? 1 : 0)) );
	if( !(edgeKnown[from] & bit) )
	{
		// from a little above the ground, so that a low wall a vehicle cannot climb is seen as well as a high one
		Vec3 a, b, boxMins = { -28, -28, 0 }, boxMaxs = { 28, 28, 48 };
		centre( from ).copyTo( a );
		centre( to ).copyTo( b );
		a[2] += wayClearance;
		b[2] += wayClearance;
		trace_t tr;
		GameHost::trace( &tr, a, boxMins, boxMaxs, b, ENTITYNUM_NONE, wayMask, false );
		edgeKnown[from] |= bit;
		if( tr.startsolid || tr.fraction >= 1.0f )
			edgeOpen[from] |= bit;
	}
	return (edgeOpen[from] & bit) != 0;
}

void GroundNav::blockAhead( const Vec3& here, const Vec3& towards )
{
	if( !built_ )
		return;
	Vec3 dir = towards - here;
	const float length = sqrtf( dir.x() * dir.x() + dir.y() * dir.y() );
	if( length < 1.0f )
		return;
	const int from = cellAt( here );
	// the neighbour the vehicle was heading into: one cell on along its line
	const int to = cellAt( here + dir * (cellSize_ / length) );
	if( from < 0 || to < 0 || from == to )
		return;
	const int dx = to % nx_ - from % nx_, dy = to / nx_ - from / nx_;
	if( abs( dx ) > 1 || abs( dy ) > 1 )
		return;
	const auto bitFor = []( int x, int y ) { return static_cast<unsigned char>( 1 << ((y + 1) * 3 + (x + 1) - ((y + 1) * 3 + (x + 1) > 4 ? 1 : 0)) ); };
	edgeKnown[from] |= bitFor( dx, dy );
	edgeOpen[from] &= static_cast<unsigned char>( ~bitFor( dx, dy ) );
	edgeKnown[to] |= bitFor( -dx, -dy );
	edgeOpen[to] &= static_cast<unsigned char>( ~bitFor( -dx, -dy ) );
}

// a straight line that stays on known cells of the right kind, without a step too high and with nothing in the way
bool GroundNav::lineOpen( const Vec3& a, const Vec3& b, Kind wanted )
{
	const Vec3 delta = b - a;
	const float length = sqrtf( delta.x() * delta.x() + delta.y() * delta.y() );
	const int steps = std::max( 1, static_cast<int>( length / (cellSize_ * 0.5f) ) );
	int previous = cellAt( a );
	if( previous < 0 || kind_[previous] != wanted )
		return false;
	for( int i = 1; i <= steps; i++ )
	{
		const Vec3 point = a + delta * (static_cast<float>( i ) / steps);
		const int cell = cellAt( point );
		if( cell < 0 )
			return false;
		if( cell == previous )
			continue;
		if( kind_[cell] != wanted )
			return false;
		if( wanted == Land && fabs( height_[cell] - height_[previous] ) > cellSize_ * maxStep )
			return false;
		previous = cell;
	}
	Vec3 from, to, boxMins = { -28, -28, 0 }, boxMaxs = { 28, 28, 48 };
	a.copyTo( from );
	b.copyTo( to );
	from[2] += wayClearance;
	to[2] += wayClearance;
	trace_t tr;
	GameHost::trace( &tr, from, boxMins, boxMaxs, to, ENTITYNUM_NONE, wayMask, false );
	return tr.startsolid || tr.fraction >= 1.0f;
}

// A* from one point to another over land or over water. Returns false with a path towards the nearest point
// reached when the goal itself is out of reach. Corners that a straight line can skip are left out
bool GroundNav::findPath( const Vec3& from, const Vec3& to, bool water, std::vector<Vec3>& out )
{
	out.clear();
	if( !built_ )
		build();

	const Kind wanted = water ? Water : Land;
	const int start = cellAt( from );
	const int goal = cellAt( to );
	if( start < 0 || goal < 0 )
		return false;
	if( kind_[start] == Unknown )
		evaluate( start, from.z() );
	// the vehicle is standing there, whatever the sample in the middle of the cell says
	if( kind_[start] != wanted )
	{
		kind_[start] = wanted;
		height_[start] = from.z();
	}
	if( start == goal )
	{
		out.push_back( to );
		return true;
	}

	generation_++;
	using Entry = std::pair<float, int>;
	const auto later = []( const Entry& a, const Entry& b ) { return a.first > b.first; };
	const auto flatDistance = [&]( int cell, const Vec3& point )
	{
		const int x = cell % nx_;
		const int y = cell / nx_;
		const float dx = mins_.x() + (x + 0.5f) * cellSize_ - point.x();
		const float dy = mins_.y() + (y + 0.5f) * cellSize_ - point.y();
		return sqrtf( dx * dx + dy * dy );
	};

	std::vector<Entry>& open = openHeap;
	open.clear();
	stamp_[start] = generation_;
	cost_[start] = 0;
	parent_[start] = -1;
	open.push_back( { flatDistance( start, to ), start } );

	int closest = start;
	float closestDistance = flatDistance( start, to );
	bool found = false;
	int expansions = 0;
	while( !open.empty() && expansions < maxExpansions )
	{
		std::pop_heap( open.begin(), open.end(), later );
		const int cell = open.back().second;
		const float queued = open.back().first;
		open.pop_back();
		const float remaining = flatDistance( cell, to );
		// an entry left over from before a shorter way to this cell was found
		if( queued > cost_[cell] + remaining + 1.0f )
			continue;
		if( cell == goal )
		{
			found = true;
			break;
		}
		if( remaining < closestDistance )
		{
			closestDistance = remaining;
			closest = cell;
		}
		expansions++;

		const int x = cell % nx_;
		const int y = cell / nx_;
		for( int dy = -1; dy <= 1; dy++ )
		{
			for( int dx = -1; dx <= 1; dx++ )
			{
				if( !dx && !dy )
					continue;
				const int ox = x + dx, oy = y + dy;
				if( ox < 0 || oy < 0 || ox >= nx_ || oy >= ny_ )
					continue;
				const int other = oy * nx_ + ox;
				if( !passable( cell, other, wanted ) )
					continue;
				// no cutting a corner past a blocked cell
				if( dx && dy && (!passable( cell, y * nx_ + ox, wanted ) || !passable( cell, oy * nx_ + x, wanted )) )
					continue;
				float step = cellSize_ * ((dx && dy) ? 1.4142f : 1.0f);
				// climbing and descending cost more than level ground
				step *= 1.0f + 2.0f * fabs( height_[other] - height_[cell] ) / cellSize_;
				const float total = cost_[cell] + step;
				if( stamp_[other] == generation_ && total >= cost_[other] )
					continue;
				stamp_[other] = generation_;
				cost_[other] = total;
				parent_[other] = cell;
				open.push_back( { total + flatDistance( other, to ), other } );
				std::push_heap( open.begin(), open.end(), later );
			}
		}
	}

	const int end = found ? goal : closest;
	if( end == start )
		return false;

	std::vector<Vec3>& raw = rawPath;
	raw.clear();
	if( found )
		raw.push_back( Vec3( to.x(), to.y(), height_[goal] ) );
	for( int cell = found ? parent_[goal] : end; cell >= 0 && cell != start; cell = parent_[cell] )
		raw.push_back( centre( cell ) );
	std::reverse( raw.begin(), raw.end() );

	// straighten: from each point keep the furthest of the next few that can be driven to directly
	Vec3 anchor( from.x(), from.y(), height_[start] );
	size_t i = 0;
	while( i < raw.size() )
	{
		size_t furthest = i;
		for( size_t j = std::min( raw.size() - 1, i + 6 ); j > i; j-- )
		{
			if( lineOpen( anchor, raw[j], wanted ) )
			{
				furthest = j;
				break;
			}
		}
		out.push_back( raw[furthest] );
		anchor = raw[furthest];
		i = furthest + 1;
	}
	return found;
}
