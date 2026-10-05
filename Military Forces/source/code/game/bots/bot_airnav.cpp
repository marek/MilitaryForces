#include "../g_local.h"
#include "../g_level.h"
#include "bot_airnav.h"
#include <algorithm>
#include <cmath>

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

	void findLinks( int index )
	{
		const Vec3 centre = nodes[index].centre;
		const float half = nodes[index].half;
		const int start = static_cast<int>( links.size() );
		const int samples = std::clamp( static_cast<int>( half / leafHalf ), 1, maxFaceSamples );
		for( int axis = 0; axis < 3; axis++ )
		{
			const int u = (axis + 1) % 3, v = (axis + 2) % 3;
			for( float side : { -1.0f, 1.0f } )
			{
				for( int i = 0; i < samples; i++ )
				{
					for( int j = 0; j < samples; j++ )
					{
						Vec3 point = centre;
						point.v[axis] += side * (half + leafHalf * 0.5f);
						point.v[u] += ((i + 0.5f) / samples * 2.0f - 1.0f) * half;
						point.v[v] += ((j + 0.5f) / samples * 2.0f - 1.0f) * half;
						const int other = leafAt( point );
						if( other < 0 || nodes[other].fill != Free )
							continue;
						if( std::find( links.begin() + start, links.end(), other ) == links.end() )
							links.push_back( other );
					}
				}
			}
		}
		nodes[index].linkStart = start;
		nodes[index].linkCount = static_cast<int>( links.size() ) - start;
	}

	int nearestFree( const Vec3& point )
	{
		const int leaf = leafAt( point );
		if( leaf >= 0 && nodes[leaf].fill == Free )
			return leaf;
		for( int ring = 1; ring <= 4; ring++ )
		{
			int best = -1;
			float bestDist = 0;
			for( int dz = -1; dz <= 1; dz++ )
			{
				for( int dy = -1; dy <= 1; dy++ )
				{
					for( int dx = -1; dx <= 1; dx++ )
					{
						if( !dx && !dy && !dz )
							continue;
						const float reach = ring * leafHalf * 2.0f;
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
				}
			}
			if( best >= 0 )
				return best;
		}
		return -1;
	}

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

bool AirNav::findPath( const Vec3& from, const Vec3& to, std::vector<Vec3>& out, const Vec3& heading, float turnRadius )
{
	out.clear();
	if( nodes.empty() )
		build();

	const int start = nearestFree( from );
	int goal = nearestFree( to );
	const Vec3 back = from - to;
	const float span = back.length();
	for( float along = leafHalf * 4.0f; goal < 0 && along < span; along += leafHalf * 4.0f )
	{
		const int leaf = leafAt( to + back * (along / span) );
		if( leaf >= 0 && nodes[leaf].fill == Free )
			goal = leaf;
	}
	if( start < 0 || goal < 0 )
		return false;
	if( start == goal )
	{
		out.push_back( to );
		return true;
	}

	generation++;
	using Entry = std::pair<float, int>;
	const auto later = []( const Entry& a, const Entry& b ) { return a.first > b.first; };
	std::vector<Entry>& open = openHeap;
	open.clear();
	nodes[start].stamp = generation;
	nodes[start].cost = 0;
	nodes[start].parent = -1;
	nodes[start].entry = from;
	nodes[start].heading = heading;
	open.push_back( { (from - to).length(), start } );

	bool found = false;
	int expansions = 0;
	int closest = start;
	float closestDistance = (from - to).length();
	while( !open.empty() && expansions < maxExpansions )
	{
		std::pop_heap( open.begin(), open.end(), later );
		const int cell = open.back().second;
		const float queued = open.back().first;
		open.pop_back();
		const Vec3 here = nodes[cell].entry;
		const float soFar = nodes[cell].cost;
		if( queued > soFar + (here - to).length() + 1.0f )
			continue;
		if( cell == goal )
		{
			found = true;
			break;
		}
		const float remaining = (here - to).length();
		if( remaining < closestDistance )
		{
			closestDistance = remaining;
			closest = cell;
		}
		expansions++;
		const Vec3 arriving = nodes[cell].heading;
		const float roomHere = nodes[cell].half;

		if( nodes[cell].linkStart < 0 )
			findLinks( cell );
		const int linkStart = nodes[cell].linkStart;
		const int linkCount = nodes[cell].linkCount;
		for( int i = 0; i < linkCount; i++ )
		{
			const int other = links[linkStart + i];
			Node& next = nodes[other];
			const Vec3 entry = entryInto( next, here );
			const Vec3 move = entry - here;
			float step = std::max( move.length(), leafHalf ) + fabs( move.z() ) * 0.3f;
			if( next.half <= leafHalf )
				step *= 1.8f;
			else if( next.half <= leafHalf * 2.0f )
				step *= 1.35f;
			const Vec3 leaving = move.normalized();
			if( turnRadius > 0.0f )
			{
				const float turn = 1.0f - arriving.dot( leaving );
				if( turn > 0.3f )
					step += turnRadius * turn * (std::min( roomHere, next.half ) < turnRadius ? 6.0f : 1.5f);
			}
			const float total = soFar + step;
			if( next.stamp == generation && total >= next.cost )
				continue;
			next.stamp = generation;
			next.cost = total;
			next.parent = cell;
			next.entry = entry;
			next.heading = leaving;
			open.push_back( { total + (entry - to).length(), other } );
			std::push_heap( open.begin(), open.end(), later );
		}
	}
	if( !found && closest == start )
		return false;

	std::vector<Vec3>& raw = rawPath;
	raw.clear();
	if( found )
		raw.push_back( to );
	for( int cell = found ? goal : closest; cell >= 0 && cell != start; cell = nodes[cell].parent )
		raw.push_back( nodes[cell].entry );
	std::reverse( raw.begin(), raw.end() );

	Vec3 anchor = from;
	size_t i = 0;
	while( i < raw.size() )
	{
		size_t furthest = i;
		for( size_t j = std::min( raw.size() - 1, i + 8 ); j > i; j-- )
		{
			if( clearLine( anchor, raw[j], 80.0f ) )
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
