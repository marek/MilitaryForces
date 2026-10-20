#include "../../g_local.h"
#include "../../g_level.h"
#include "airnav.h"
#include <algorithm>
#include <cmath>

// Paths through the air: the map's space as an octree of cubes, split only where there is something solid, each
// cube examined the first time a search reaches it; A* from cube to cube through their free neighbours.
namespace
{
	enum Fill : signed char { Unknown, Free, Solid, Mixed };

	struct Node
	{
		Vec3		centre;
		float		half = 0;
		int			firstChild = -1;
		int			linkStart = -1;
		int			linkCount = 0;
		Fill		fill = Unknown;
		int			stamp = 0;
		int			parent = -1;
		float		cost = 0;
		Vec3		entry;
		Vec3		heading;
	};

	// the smallest cube is 128 units across; a path enters a cube no nearer its walls than the margin where it has room
	constexpr float leafHalf = 64.0f;
	constexpr float entryMargin = 160.0f;
	constexpr int maxNodes = 400000;
	constexpr int maxExpansions = 20000;
	constexpr int maxFaceSamples = 16;

	std::vector<Node> nodes;
	std::vector<int> links;
	std::vector<std::pair<float, int>> openHeap;
	std::vector<Vec3> rawPath;
	Vec3 worldMins, worldMaxs;
	int generation = 0;

	// one cube that holds the whole map; everything else is made on demand
	void build()
	{
		Vec3 boundsMins, boundsMaxs;
		CollisionModel::modelBounds( 0, boundsMins, boundsMaxs );
		worldMins = Vec3( boundsMins );
		worldMaxs = Vec3( boundsMaxs );
		const Vec3 size = worldMaxs - worldMins;
		float half = leafHalf;
		while( half * 2.0f < std::max( { size.x(), size.y(), size.z() } ) )
			half *= 2.0f;
		nodes.clear();
		links.clear();
		nodes.reserve( 4096 );
		Node root;
		root.centre = (worldMins + worldMaxs) * 0.5f;
		root.half = half;
		nodes.push_back( root );
		generation = 0;
	}

	// empty, solid, or split into eight to be looked at when needed
	void classify( int index )
	{
		const Vec3 centre = nodes[index].centre;
		const float half = nodes[index].half;
		for( int k = 0; k < 3; k++ )
		{
			if( centre.v[k] + half <= worldMins.v[k] || centre.v[k] - half >= worldMaxs.v[k] )
			{
				nodes[index].fill = Solid;
				return;
			}
		}
		Vec3 boxMins = { -half, -half, -half }, boxMaxs = { half, half, half };
		trace_t tr;
		GameHost::trace( &tr, centre, boxMins, boxMaxs, centre, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
		if( !tr.startsolid && !tr.allsolid )
		{
			nodes[index].fill = Free;
			return;
		}
		if( half <= leafHalf )
		{
			// a vehicle that happens to be there now does not make the air solid
			const bool vehicle = tr.entityNum >= 1 && tr.entityNum <= theLevel.maxclients_;
			nodes[index].fill = vehicle ? Free : Solid;
			return;
		}
		if( nodes.size() + 8 > static_cast<size_t>( maxNodes ) )
		{
			nodes[index].fill = Solid;
			return;
		}
		const int first = static_cast<int>( nodes.size() );
		nodes[index].fill = Mixed;
		nodes[index].firstChild = first;
		const float quarter = half * 0.5f;
		for( int child = 0; child < 8; child++ )
		{
			Node node;
			node.centre = centre + Vec3( (child & 1) ? quarter : -quarter, (child & 2) ? quarter : -quarter, (child & 4) ? quarter : -quarter );
			node.half = quarter;
			nodes.push_back( node );
		}
	}

	// the cube a point is in, splitting on the way down as far as needed
	int leafAt( const Vec3& point )
	{
		if( nodes.empty() )
			return -1;
		for( int k = 0; k < 3; k++ )
		{
			if( fabs( point.v[k] - nodes[0].centre.v[k] ) >= nodes[0].half )
				return -1;
		}
		int index = 0;
		for( ;; )
		{
			if( nodes[index].fill == Unknown )
				classify( index );
			if( nodes[index].fill != Mixed )
				return index;
			const Vec3& centre = nodes[index].centre;
			const int child = (point.x() >= centre.x() ? 1 : 0) | (point.y() >= centre.y() ? 2 : 0) | (point.z() >= centre.z() ? 4 : 0);
			index = nodes[index].firstChild + child;
		}
	}

	// the empty cubes beyond one face of a cube, sampled on a grid over the face, added once each
	void linkFace( int index, int axis, float side, int samples, int start )
	{
		const Vec3 centre = nodes[index].centre;
		const float half = nodes[index].half;
		const int u = (axis + 1) % 3, v = (axis + 2) % 3;
		for( int i = 0; i < samples; i++ )
		{
			for( int j = 0; j < samples; j++ )
			{
				Vec3 point = centre;
				point.v[axis] += side * (half + leafHalf * 0.5f);
				point.v[u] += ((i + 0.5f) / samples * 2.0f - 1.0f) * half;
				point.v[v] += ((j + 0.5f) / samples * 2.0f - 1.0f) * half;
				const int other = leafAt( point );
				if( other >= 0 && nodes[other].fill == Free && std::find( links.begin() + start, links.end(), other ) == links.end() )
					links.push_back( other );
			}
		}
	}

	// the empty cubes that touch this one, found by sampling points just outside each of its faces
	void findLinks( int index )
	{
		const int start = static_cast<int>( links.size() );
		const int samples = std::clamp( static_cast<int>( nodes[index].half / leafHalf ), 1, maxFaceSamples );
		for( int axis = 0; axis < 3; axis++ )
		{
			for( float side : { -1.0f, 1.0f } )
				linkFace( index, axis, side, samples, start );
		}
		nodes[index].linkStart = start;
		nodes[index].linkCount = static_cast<int>( links.size() ) - start;
	}

	// the nearest empty cube among the twenty-six a ring's step away round a point, or -1
	int nearestOnRing( const Vec3& point, int ring )
	{
		const float reach = ring * leafHalf * 2.0f;
		int best = -1;
		float bestDist = 0;
		for( int n = 0; n < 27; n++ )
		{
			const int dx = n % 3 - 1, dy = n / 3 % 3 - 1, dz = n / 9 - 1;
			if( !dx && !dy && !dz )
				continue;
			const int other = leafAt( point + Vec3( dx * reach, dy * reach, dz * reach ) );
			if( other < 0 || nodes[other].fill != Free )
				continue;
			const float dist = (nodes[other].centre - point).lengthSquared();
			if( best < 0 || dist < bestDist )
			{
				best = other;
				bestDist = dist;
			}
		}
		return best;
	}

	// the empty cube at a point, or the nearest one within a few cubes of it
	int nearestFree( const Vec3& point )
	{
		const int leaf = leafAt( point );
		if( leaf >= 0 && nodes[leaf].fill == Free )
			return leaf;
		for( int ring = 1; ring <= 4; ring++ )
		{
			const int best = nearestOnRing( point, ring );
			if( best >= 0 )
				return best;
		}
		return -1;
	}

	// where a path coming from that point enters the cube: as near as it can stay to where it was, clear of the walls
	Vec3 entryInto( const Node& node, const Vec3& from )
	{
		const float inner = node.half - std::min( node.half, entryMargin );
		return Vec3( std::clamp( from.x(), node.centre.x() - inner, node.centre.x() + inner ),
			std::clamp( from.y(), node.centre.y() - inner, node.centre.y() + inner ),
			std::clamp( from.z(), node.centre.z() - inner, node.centre.z() + inner ) );
	}
}

void AirNav::reset()
{
	nodes.clear();
	links.clear();
}

// both ends inside one empty cell, so nothing of the static world can be in between
bool AirNav::segmentKnownFree( const Vec3& a, const Vec3& b, float margin )
{
	if( nodes.empty() )
		build();
	const int leaf = leafAt( a );
	if( leaf < 0 || nodes[leaf].fill != Free )
		return false;
	const float inner = nodes[leaf].half - margin;
	const Vec3& centre = nodes[leaf].centre;
	for( int k = 0; k < 3; k++ )
	{
		if( fabs( a.v[k] - centre.v[k] ) > inner || fabs( b.v[k] - centre.v[k] ) > inner )
			return false;
	}
	return true;
}

int AirNav::cellCount()
{
	return static_cast<int>( nodes.size() );
}

int AirNav::freeCount()
{
	return static_cast<int>( std::count_if( nodes.begin(), nodes.end(), []( const Node& node ) { return node.fill == Free; } ) );
}

// a body of that width can fly the straight line: nothing of the world in the way (other vehicles do not count)
bool AirNav::clearLine( const Vec3& a, const Vec3& b, float halfWidth )
{
	Vec3 boxMins = { -halfWidth, -halfWidth, -halfWidth * 0.4f };
	Vec3 boxMaxs = { halfWidth, halfWidth, halfWidth * 0.4f };
	trace_t tr;
	GameHost::trace( &tr, a, boxMins, boxMaxs, b, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	if( tr.startsolid )
		GameHost::trace( &tr, a, nullptr, nullptr, b, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	return tr.fraction >= 1.0f || tr.entityNum != ENTITYNUM_WORLD;
}

namespace
{
	// the empty cube at the goal; a goal inside something solid: the nearest free point on the way back from it
	int goalCell( const Vec3& from, const Vec3& to )
	{
		int goal = nearestFree( to );
		const Vec3 back = from - to;
		const float span = back.length();
		for( float along = leafHalf * 4.0f; goal < 0 && along < span; along += leafHalf * 4.0f )
		{
			const int leaf = leafAt( to + back * (along / span) );
			if( leaf >= 0 && nodes[leaf].fill == Free )
				goal = leaf;
		}
		return goal;
	}

	// what it costs to go on from a cube into the next: the distance, more in tight places than in open air, and with a
	// turn radius given, more for a turn, far more where there is no room to turn
	float stepCost( const Node& here, const Node& next, const Vec3& move, const Vec3& leaving, float turnRadius )
	{
		float step = std::max( move.length(), leafHalf ) + fabs( move.z() ) * 0.3f;
		if( next.half <= leafHalf )
			step *= 1.8f;
		else if( next.half <= leafHalf * 2.0f )
			step *= 1.35f;
		if( turnRadius > 0.0f )
		{
			const float turn = 1.0f - here.heading.dot( leaving );
			if( turn > 0.3f )
				step += turnRadius * turn * (std::min( here.half, next.half ) < turnRadius ? 6.0f : 1.5f);
		}
		return step;
	}

	using Entry = std::pair<float, int>;
	bool later( const Entry& a, const Entry& b ) { return a.first > b.first; }

	// every empty cube next to this one, queued when this way to it is cheaper than any found before
	void expand( int cell, const Vec3& to, float turnRadius, const PathDanger& danger )
	{
		if( nodes[cell].linkStart < 0 )
			findLinks( cell );
		const Vec3 here = nodes[cell].entry;
		const float soFar = nodes[cell].cost;
		for( int i = 0; i < nodes[cell].linkCount; i++ )
		{
			const int other = links[nodes[cell].linkStart + i];
			Node& next = nodes[other];
			const Vec3 entry = entryInto( next, here );
			const Vec3 move = entry - here;
			const Vec3 leaving = move.normalized();
			const float total = soFar + stepCost( nodes[cell], next, move, leaving, turnRadius ) * danger.factor( entry );
			if( next.stamp == generation && total >= next.cost )
				continue;
			next.stamp = generation;
			next.cost = total;
			next.parent = cell;
			next.entry = entry;
			next.heading = leaving;
			openHeap.push_back( { total + (entry - to).length(), other } );
			std::push_heap( openHeap.begin(), openHeap.end(), later );
		}
	}

	// from each point keep the furthest of the next few that can be flown to directly
	void straighten( const Vec3& from, const std::vector<Vec3>& raw, std::vector<Vec3>& out )
	{
		Vec3 anchor = from;
		size_t i = 0;
		while( i < raw.size() )
		{
			size_t furthest = i;
			for( size_t j = std::min( raw.size() - 1, i + 8 ); j > i; j-- )
			{
				if( AirNav::clearLine( anchor, raw[j], 80.0f ) )
				{
					furthest = j;
					break;
				}
			}
			out.push_back( raw[furthest] );
			anchor = raw[furthest];
			i = furthest + 1;
		}
	}

	// A* over the cubes; reached is the goal when found, otherwise the cube that came nearest to it
	bool search( int start, int goal, const Vec3& from, const Vec3& to, const Vec3& heading, float turnRadius, const PathDanger& danger, int& reached )
	{
		generation++;
		openHeap.clear();
		nodes[start].stamp = generation;
		nodes[start].cost = 0;
		nodes[start].parent = -1;
		nodes[start].entry = from;
		nodes[start].heading = heading;
		openHeap.push_back( { (from - to).length(), start } );
		int expansions = 0;
		reached = start;
		float closestDistance = (from - to).length();
		while( !openHeap.empty() && expansions < maxExpansions )
		{
			std::pop_heap( openHeap.begin(), openHeap.end(), later );
			const int cell = openHeap.back().second;
			const float queued = openHeap.back().first;
			openHeap.pop_back();
			const Vec3 here = nodes[cell].entry;
			if( queued > nodes[cell].cost + (here - to).length() + 1.0f )
				continue;
			if( cell == goal )
			{
				reached = goal;
				return true;
			}
			const float remaining = (here - to).length();
			if( remaining < closestDistance )
			{
				closestDistance = remaining;
				reached = cell;
			}
			expansions++;
			expand( cell, to, turnRadius, danger );
		}
		return false;
	}
}

// A* from one point to another. With a turn radius given, turns cost more, and far more in cubes too small to
// turn in, so a fast aircraft is routed where it can fly. Returns false with a path towards the nearest point
// reached when the goal is not
bool AirNav::findPath( const Vec3& from, const Vec3& to, std::vector<Vec3>& out, const Vec3& heading, float turnRadius, const PathDanger& danger )
{
	out.clear();
	if( nodes.empty() )
		build();
	const int start = nearestFree( from );
	const int goal = goalCell( from, to );
	if( start < 0 || goal < 0 )
		return false;
	if( start == goal )
	{
		out.push_back( to );
		return true;
	}
	int reached = start;
	const bool found = search( start, goal, from, to, heading, turnRadius, danger, reached );
	if( !found && reached == start )
		return false;
	std::vector<Vec3>& raw = rawPath;
	raw.clear();
	if( found )
		raw.push_back( to );
	for( int cell = reached; cell >= 0 && cell != start; cell = nodes[cell].parent )
		raw.push_back( nodes[cell].entry );
	std::reverse( raw.begin(), raw.end() );
	straighten( from, raw, out );
	return found;
}
