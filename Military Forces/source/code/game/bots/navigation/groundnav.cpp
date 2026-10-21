#include "../../g_local.h"
#include "../../g_level.h"
#include "groundnav.h"
#include <algorithm>
#include <cmath>
#include <numbers>

// Paths over the ground and over water: the map as a quadtree of squares laid over a coarse grid, each square found
// out the first time a search reaches it. A square whose ground is one even surface a vehicle drives over, with
// nothing standing on it, is kept whole; anything else is split down to small squares, so open country costs a
// few large squares and a wall, a kerb or a gap in a wall is seen at the size of a vehicle. Whether a vehicle gets
// from one square into the next is found by walking the ground between them in short steps with the vehicles' own
// limits: the highest edge they step up and the steepest slope they climb. A* runs from square to square.
namespace
{
	enum Kind : signed char { Unknown, Land, Water, Blocked, Mixed };

	struct Node
	{
		float		cx = 0, cy = 0, half = 0;
		int			firstChild = -1;
		Kind		kind = Unknown;
		bool		runway = false;	// most of it is runway, where aircraft take off and land
		float		height = 0;	// the surface at the middle, and how it rises across the square
		float		slopeX = 0, slopeY = 0;
		int			linkStart = -1, linkCount = 0;
		int			stamp = 0, parent = -1;
		float		cost = 0;
		Vec3		entry;
	};

	// a way out of a square into a neighbour, crossing the border at that point
	struct Link
	{
		int			other;
		Vec3		cross;
	};

	// what the samples of a whole square found: one kind of surface, and as a plane, how far from it the worst sample lay
	struct Survey
	{
		bool		whole = true;
		int			count = 0, water = 0, runway = 0;
		float		height = 0, slopeX = 0, slopeY = 0, worst = 0;
	};

	// coarse squares 256 across, split down to 32; samples of a whole square every 64 units; the ground between
	// two squares walked every 4
	constexpr float coarseHalf = 128.0f;
	constexpr float leafHalf = 16.0f;
	constexpr float surfaceSpacing = 64.0f;
	constexpr int mostSamples = 16;
	constexpr float walkStep = 4.0f;
	// what is looked for on a square kept whole: lines 16 apart with a box 16 wide, so nothing is between them,
	// from just above a step up to a vehicle's height
	constexpr float lineSpacing = 16.0f;
	constexpr float lineHalf = 8.0f;
	constexpr float bodyHeight = 12.0f;
	// a step down off an edge a vehicle takes, as a share of the step up; deeper is a drop it does not drive off
	constexpr float dropShare = 3.0f;
	// a runway is driven along only where there is no other way: aircraft stand and roll there
	constexpr float runwayCost = 3.0f;
	constexpr float narrowCost = 1.2f;
	constexpr int maxNodes = 600000;
	constexpr int maxExpansions = 20000;
	// what stands in a vehicle's way: solid things and the invisible walls mappers put up for players and for bots,
	// but not other vehicles, which move on
	constexpr int wayMask = MASK_SOLID|CONTENTS_PLAYERCLIP|CONTENTS_BOTCLIP;

	std::vector<Node> nodes;
	std::vector<Link> links;
	std::vector<std::pair<float, int>> openHeap;
	std::vector<Vec3> rawPath;
	Vec3 worldMins, worldMaxs;
	int gridX = 0, gridY = 0;
	int generation = 0;
	bool built = false;
	// the limits of the most limited ground vehicle: every ground vehicle can go where a path leads
	float stepUp = 6.0f;
	float climbSlope = 0.8f;

	using Entry = std::pair<float, int>;
	bool later( const Entry& a, const Entry& b ) { return a.first > b.first; }

	float flat( const Vec3& a, const Vec3& b )
	{
		return sqrtf( (a.x() - b.x()) * (a.x() - b.x()) + (a.y() - b.y()) * (a.y() - b.y()) );
	}

	// the step and climb of the most limited ground vehicle
	void readLimits()
	{
		stepUp = 1000.0f;
		float climb = 90.0f;
		for( int i = 0; i < bg_numberOfVehicles; i++ )
		{
			if( !(availableVehicles[i].cat & CAT_GROUND) )
				continue;
			stepUp = std::min( stepUp, VehicleData::stepHeight( i ) );
			climb = std::min( climb, VehicleData::climbAngle( i ) );
		}
		if( stepUp >= 1000.0f )
			stepUp = 6.0f;
		climbSlope = tanf( std::min( climb, 80.0f ) * std::numbers::pi_v<float> / 180.0f );
	}

	void build()
	{
		built = true;
		Vec3 boundsMins, boundsMaxs;
		CollisionModel::modelBounds( 0, boundsMins, boundsMaxs );
		worldMins = Vec3( boundsMins );
		worldMaxs = Vec3( boundsMaxs );
		gridX = std::max( 1, static_cast<int>( ceilf( (worldMaxs.x() - worldMins.x()) / (coarseHalf * 2.0f) ) ) );
		gridY = std::max( 1, static_cast<int>( ceilf( (worldMaxs.y() - worldMins.y()) / (coarseHalf * 2.0f) ) ) );
		nodes.clear();
		links.clear();
		nodes.reserve( static_cast<size_t>( gridX ) * gridY * 4 );
		for( int y = 0; y < gridY; y++ )
		{
			for( int x = 0; x < gridX; x++ )
			{
				Node node;
				node.cx = worldMins.x() + (x + 0.5f) * coarseHalf * 2.0f;
				node.cy = worldMins.y() + (y + 0.5f) * coarseHalf * 2.0f;
				node.half = coarseHalf;
				nodes.push_back( node );
			}
		}
		readLimits();
		generation = 0;
	}

	bool onRunway( const trace_t& tr )
	{
		GameEntity* ground = tr.entityNum > ENTITYNUM_WORLD || tr.entityNum < 0 ? nullptr : theLevel.getEntity( tr.entityNum );
		return ground && ground->classname_ && strcmp( ground->classname_, "func_runway" ) == 0;
	}

	// the surface at a point, looked for from a little above the height the search comes from, so that a roof over
	// it (a hangar, a bunker with a flag in it) is not taken for its floor; where that starts inside rising ground,
	// from higher up
	bool surfaceAt( float x, float y, float reference, float& height, bool& water, bool* runway = nullptr )
	{
		constexpr float underRoof = 100.0f, overRise = 260.0f, below = 700.0f;
		vec3_t top = { x, y, reference + underRoof }, bottom = { x, y, reference - below };
		trace_t tr;
		GameHost::trace( &tr, top, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER, false );
		if( tr.startsolid )
		{
			top[2] = reference + overRise;
			GameHost::trace( &tr, top, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER, false );
		}
		if( tr.startsolid || tr.fraction >= 1.0f )
			return false;
		height = tr.endpos[2];
		water = (tr.contents & MASK_WATER) != 0;
		if( runway )
			*runway = onRunway( tr );
		return true;
	}

	float planeHeight( const Node& node, float x, float y )
	{
		return node.height + node.slopeX * (x - node.cx) + node.slopeY * (y - node.cy);
	}

	// one line across a square, following its surface just above a step, with a box as wide as the line spacing
	bool lineClear( const Node& node, float ax, float ay, float bx, float by )
	{
		const Vec3 boxMins = { -lineHalf, -lineHalf, 0 }, boxMaxs = { lineHalf, lineHalf, bodyHeight };
		vec3_t a = { ax, ay, planeHeight( node, ax, ay ) + stepUp + 1.0f }, b = { bx, by, planeHeight( node, bx, by ) + stepUp + 1.0f };
		trace_t tr;
		GameHost::trace( &tr, a, boxMins, boxMaxs, b, ENTITYNUM_NONE, wayMask, false );
		return !tr.startsolid && tr.fraction >= 1.0f;
	}

	// nothing stands on the square: lines across it both ways
	bool squareClear( const Node& node )
	{
		const float inner = node.half - lineHalf;
		for( float across = -inner; across <= inner + 0.5f; across += lineSpacing )
		{
			if( !lineClear( node, node.cx + across, node.cy - inner, node.cx + across, node.cy + inner ) ||
				!lineClear( node, node.cx - inner, node.cy + across, node.cx + inner, node.cy + across ) )
				return false;
		}
		return true;
	}

	// samples over a square, fitted as a plane; not whole when a sample finds nothing or the kinds are mixed
	Survey survey( const Node& node, float reference )
	{
		Survey out;
		const int perSide = std::min( mostSamples, std::max( 3, static_cast<int>( node.half * 2.0f / surfaceSpacing ) + 1 ) );
		float heights[mostSamples * mostSamples], offsets[mostSamples];
		float sumXH = 0, sumYH = 0, sumXX = 0;
		for( int i = 0; i < perSide; i++ )
		{
			offsets[i] = (static_cast<float>( i ) / (perSide - 1) * 2.0f - 1.0f) * node.half;
			sumXX += offsets[i] * offsets[i] * perSide;
		}
		for( int k = 0; k < perSide * perSide && out.whole; k++ )
		{
			const float ox = offsets[k % perSide], oy = offsets[k / perSide];
			bool water, runway;
			out.whole = surfaceAt( node.cx + ox, node.cy + oy, reference, heights[k], water, &runway );
			out.count++;
			out.water += water;
			out.runway += runway;
			out.height += heights[k];
			sumXH += ox * heights[k];
			sumYH += oy * heights[k];
		}
		if( !out.whole || (out.water && out.water != out.count) )
		{
			out.whole = false;
			return out;
		}
		out.height /= out.count;
		out.slopeX = sumXH / sumXX;
		out.slopeY = sumYH / sumXX;
		for( int k = 0; k < out.count; k++ )
		{
			const float fitted = out.height + out.slopeX * offsets[k % perSide] + out.slopeY * offsets[k / perSide];
			out.worst = std::max( out.worst, fabsf( heights[k] - fitted ) );
		}
		return out;
	}

	// four children, to be found out when a search reaches them
	void split( int index )
	{
		const int first = static_cast<int>( nodes.size() );
		nodes[index].kind = Mixed;
		nodes[index].firstChild = first;
		const Node parent = nodes[index];
		const float quarter = parent.half * 0.5f;
		for( int child = 0; child < 4; child++ )
		{
			Node made;
			made.cx = parent.cx + ((child & 1) ? quarter : -quarter);
			made.cy = parent.cy + ((child & 2) ? quarter : -quarter);
			made.half = quarter;
			nodes.push_back( made );
		}
	}

	// the smallest square is what its middle is; the ways in and out of it are walked
	void classifyLeaf( int index, float reference )
	{
		Node& leaf = nodes[index];
		float height = reference;
		bool water = false, runway = false;
		const bool found = surfaceAt( leaf.cx, leaf.cy, reference, height, water, &runway );
		leaf.slopeX = leaf.slopeY = 0;
		leaf.height = height;
		leaf.runway = runway;
		leaf.kind = !found ? Blocked : water ? Water : Land;
	}

	// one even surface of one kind with nothing on it, kept whole; at the smallest size, what its middle is; otherwise
	// split into four
	void classify( int index, float reference )
	{
		const Survey found = survey( nodes[index], reference );
		if( found.whole )
		{
			Node& node = nodes[index];
			node.height = found.height;
			node.slopeX = found.slopeX;
			node.slopeY = found.slopeY;
			const bool water = found.water == found.count;
			const bool even = found.worst <= stepUp * 0.5f && (water || sqrtf( node.slopeX * node.slopeX + node.slopeY * node.slopeY ) <= climbSlope);
			if( even && squareClear( node ) )
			{
				node.kind = water ? Water : Land;
				node.runway = found.runway * 2 >= found.count;
				return;
			}
		}
		if( nodes[index].half > leafHalf && nodes.size() + 4 <= static_cast<size_t>( maxNodes ) )
			split( index );
		else
			classifyLeaf( index, reference );
	}

	int coarseAt( float x, float y )
	{
		const int gx = static_cast<int>( floorf( (x - worldMins.x()) / (coarseHalf * 2.0f) ) );
		const int gy = static_cast<int>( floorf( (y - worldMins.y()) / (coarseHalf * 2.0f) ) );
		if( gx < 0 || gy < 0 || gx >= gridX || gy >= gridY )
			return -1;
		return gy * gridX + gx;
	}

	// the square a point is in, finding out and splitting on the way down as far as needed
	int leafAt( float x, float y, float reference )
	{
		int index = coarseAt( x, y );
		while( index >= 0 )
		{
			if( nodes[index].kind == Unknown )
				classify( index, reference );
			if( nodes[index].kind != Mixed )
				return index;
			const Node& node = nodes[index];
			index = node.firstChild + (x >= node.cx ? 1 : 0) + (y >= node.cy ? 2 : 0);
		}
		return -1;
	}

	bool smallest( const Node& node )
	{
		return node.half <= leafHalf;
	}

	// where a way through a square starts: the middle of a small one, which has to be walked to; on a whole square,
	// anywhere, so just inside the border
	Vec3 anchorIn( const Node& node, const Vec3& cross )
	{
		if( smallest( node ) )
			return Vec3( node.cx, node.cy, node.height );
		constexpr float inside = 8.0f;
		const float inner = node.half - inside;
		const float x = std::clamp( cross.x(), node.cx - inner, node.cx + inner ), y = std::clamp( cross.y(), node.cy - inner, node.cy + inner );
		return Vec3( x, y, planeHeight( node, x, y ) );
	}

	// one step of a walk: the ground there looked for from a step above the last; false where it cannot be driven
	bool stepOnto( float x, float y, float& height, bool water )
	{
		const Vec3 boxMins = { -3, -3, 0 }, boxMaxs = { 3, 3, 1 };
		vec3_t top = { x, y, height + stepUp + 1.0f }, bottom = { x, y, height - stepUp * dropShare };
		trace_t tr;
		GameHost::trace( &tr, top, boxMins, boxMaxs, bottom, ENTITYNUM_NONE, wayMask|MASK_WATER, false );
		if( tr.startsolid || tr.fraction >= 1.0f || ((tr.contents & MASK_WATER) != 0) != water )
			return false;
		height = tr.endpos[2];
		return true;
	}

	// Walks the ground from one point to another in short steps, each looked for from a step above the last: what
	// rises more than a step is in the way, as is a drop deeper than a vehicle drives off, a slope steeper than it
	// climbs, a hole, and the wrong kind of surface
	bool walk( const Vec3& from, const Vec3& to, bool water )
	{
		constexpr int slopeSpan = 8;
		const float dx = to.x() - from.x(), dy = to.y() - from.y();
		const int steps = std::max( 1, static_cast<int>( ceilf( sqrtf( dx * dx + dy * dy ) / walkStep ) ) );
		float history[slopeSpan];
		float height = from.z();
		std::fill( history, history + slopeSpan, height );
		for( int i = 1; i <= steps; i++ )
		{
			const float along = static_cast<float>( i ) / steps;
			if( !stepOnto( from.x() + dx * along, from.y() + dy * along, height, water ) )
				return false;
			// over the last few steps, no steeper than a vehicle climbs
			if( !water && height - history[i % slopeSpan] > climbSlope * walkStep * std::min( i, slopeSpan ) + 1.0f )
				return false;
			history[i % slopeSpan] = height;
		}
		return true;
	}

	bool linkedTo( int start, int other )
	{
		for( int k = start; k < static_cast<int>( links.size() ); k++ )
		{
			if( links[k].other == other )
				return true;
		}
		return false;
	}

	// the ways across one side of a square, sampled every 16 units just outside it: per neighbour the first crossing
	// that can be driven
	void linksOnSide( int index, int side, int start )
	{
		constexpr float outside = 4.0f, spacing = 16.0f;
		const Node here = nodes[index];
		const bool water = here.kind == Water;
		const float nx = side == 0 ? 1.0f : side == 1 ? -1.0f : 0.0f, ny = side == 2 ? 1.0f : side == 3 ? -1.0f : 0.0f;
		for( float along = -here.half + spacing * 0.5f; along < here.half; along += spacing )
		{
			const float bx = here.cx + nx * here.half + (nx ? 0.0f : along), by = here.cy + ny * here.half + (ny ? 0.0f : along);
			const float borderHeight = smallest( here ) ? here.height : planeHeight( here, bx, by );
			const int other = leafAt( bx + nx * outside, by + ny * outside, borderHeight );
			if( other < 0 || nodes[other].kind != here.kind || linkedTo( start, other ) )
				continue;
			const Vec3 cross( bx, by, borderHeight );
			if( walk( anchorIn( nodes[index], cross ), cross, water ) && walk( cross, anchorIn( nodes[other], cross ), water ) )
				links.push_back( { other, cross } );
		}
	}

	// the ways out of a square into its neighbours
	void findLinks( int index )
	{
		const int start = static_cast<int>( links.size() );
		if( nodes[index].kind == Land || nodes[index].kind == Water )
		{
			for( int side = 0; side < 4; side++ )
				linksOnSide( index, side, start );
		}
		nodes[index].linkStart = start;
		nodes[index].linkCount = static_cast<int>( links.size() ) - start;
	}

	bool linked( int from, int to )
	{
		if( nodes[from].linkStart < 0 )
			findLinks( from );
		for( int k = 0; k < nodes[from].linkCount; k++ )
		{
			if( links[nodes[from].linkStart + k].other == to )
				return true;
		}
		return false;
	}

	// the cost of going from a point to a crossing into the next square: its length, more for climbing, a narrow
	// place, a runway, and danger
	float stepCost( const Vec3& here, const Link& link, const PathDanger& danger )
	{
		const float length = std::max( flat( here, link.cross ), leafHalf );
		float step = length * (1.0f + 2.0f * fabsf( link.cross.z() - here.z() ) / length);
		const Node& next = nodes[link.other];
		if( smallest( next ) )
			step *= narrowCost;
		if( next.runway )
			step *= runwayCost;
		return step * danger.factor( link.cross );
	}

	// every square this one leads to, queued where this way to it is cheaper than any found before
	void expand( int cell, const Vec3& to, const PathDanger& danger )
	{
		if( nodes[cell].linkStart < 0 )
			findLinks( cell );
		const Vec3 here = nodes[cell].entry;
		for( int k = 0; k < nodes[cell].linkCount; k++ )
		{
			const Link link = links[nodes[cell].linkStart + k];
			if( link.other < 0 )
				continue;
			const float total = nodes[cell].cost + stepCost( here, link, danger );
			Node& next = nodes[link.other];
			if( next.stamp == generation && total >= next.cost )
				continue;
			next.stamp = generation;
			next.cost = total;
			next.parent = cell;
			next.entry = link.cross;
			openHeap.push_back( { total + flat( link.cross, to ), link.other } );
			std::push_heap( openHeap.begin(), openHeap.end(), later );
		}
	}

	// A* over the squares; end is the goal when found, otherwise the square that came nearest to it
	bool search( int start, int goal, const Vec3& from, const Vec3& to, const PathDanger& danger, int& end )
	{
		generation++;
		openHeap.clear();
		nodes[start].stamp = generation;
		nodes[start].cost = 0;
		nodes[start].parent = -1;
		nodes[start].entry = from;
		openHeap.push_back( { flat( from, to ), start } );
		end = start;
		float closest = flat( from, to );
		for( int expansions = 0; !openHeap.empty() && expansions < maxExpansions; )
		{
			std::pop_heap( openHeap.begin(), openHeap.end(), later );
			const auto [queued, cell] = openHeap.back();
			openHeap.pop_back();
			const float remaining = flat( nodes[cell].entry, to );
			// an entry left over from before a shorter way to this square was found
			if( queued > nodes[cell].cost + remaining + 1.0f )
				continue;
			if( cell == goal )
			{
				end = goal;
				return true;
			}
			if( remaining < closest )
			{
				closest = remaining;
				end = cell;
			}
			expansions++;
			expand( cell, to, danger );
		}
		return false;
	}

	// the crossings back from the end, through a small square by its middle as its ways were walked
	void tracePath( int start, int end, bool found, const Vec3& to, std::vector<Vec3>& raw )
	{
		raw.clear();
		if( found )
			raw.push_back( Vec3( to.x(), to.y(), nodes[end].height ) );
		else
			raw.push_back( anchorIn( nodes[end], nodes[end].entry ) );
		for( int cell = end; cell >= 0 && cell != start; cell = nodes[cell].parent )
		{
			// the list is reversed below: the middle goes after the crossing into the square
			if( smallest( nodes[cell] ) && cell != end )
				raw.push_back( Vec3( nodes[cell].cx, nodes[cell].cy, nodes[cell].height ) );
			raw.push_back( nodes[cell].entry );
		}
		std::reverse( raw.begin(), raw.end() );
	}
}

void GroundNav::reset()
{
	built = false;
	nodes.clear();
	links.clear();
}

int GroundNav::cellCount()
{
	return static_cast<int>( nodes.size() );
}

int GroundNav::knownCount()
{
	return static_cast<int>( std::count_if( nodes.begin(), nodes.end(), []( const Node& node ) { return node.kind != Unknown; } ) );
}

void GroundNav::blockAhead( const Vec3& here, const Vec3& towards )
{
	if( !built )
		return;
	const float dx = towards.x() - here.x(), dy = towards.y() - here.y();
	const float length = sqrtf( dx * dx + dy * dy );
	const int from = leafAt( here.x(), here.y(), here.z() );
	if( length < 1.0f || from < 0 )
		return;
	if( nodes[from].linkStart < 0 )
		findLinks( from );
	// the square the vehicle was heading into: the first other one along its line
	for( float reach = leafHalf; reach <= leafHalf * 6.0f; reach += leafHalf )
	{
		const int to = leafAt( here.x() + dx / length * reach, here.y() + dy / length * reach, here.z() );
		if( to < 0 || to == from )
			continue;
		for( int k = 0; k < nodes[from].linkCount; k++ )
		{
			Link& link = links[nodes[from].linkStart + k];
			link.other = link.other == to ? -1 : link.other;
		}
		return;
	}
}

// a straight line through squares of the right kind, each joined to the next by a way that can be driven
bool GroundNav::lineOpen( const Vec3& a, const Vec3& b, bool water )
{
	if( !built )
		build();
	const Kind wanted = water ? Water : Land;
	const float dx = b.x() - a.x(), dy = b.y() - a.y();
	const int steps = std::max( 1, static_cast<int>( sqrtf( dx * dx + dy * dy ) / (leafHalf * 0.5f) ) );
	int previous = leafAt( a.x(), a.y(), a.z() );
	if( previous < 0 || nodes[previous].kind != wanted )
		return false;
	for( int i = 1; i <= steps; i++ )
	{
		const float x = a.x() + dx * i / steps, y = a.y() + dy * i / steps;
		const Node& last = nodes[previous];
		const int cell = leafAt( x, y, smallest( last ) ? last.height : planeHeight( last, x, y ) );
		if( cell < 0 || nodes[cell].kind != wanted || (cell != previous && !linked( previous, cell )) )
			return false;
		previous = cell;
	}
	return true;
}

// A* from one point to another over land or over water, paying for danger. Returns false with a path towards the
// nearest point reached when the goal itself is out of reach. Corners that a straight line can skip are left out
bool GroundNav::findPath( const Vec3& from, const Vec3& to, bool water, std::vector<Vec3>& out, const PathDanger& danger )
{
	out.clear();
	if( !built )
		build();
	const Kind wanted = water ? Water : Land;
	const int start = leafAt( from.x(), from.y(), from.z() );
	if( start < 0 || (nodes[start].kind != wanted && !smallest( nodes[start] )) )
		return false;
	// the vehicle is standing there, whatever the sample in the middle of a small square says
	if( nodes[start].kind != wanted )
	{
		nodes[start].kind = wanted;
		nodes[start].height = from.z();
		nodes[start].linkStart = -1;
	}
	const int goal = leafAt( to.x(), to.y(), to.z() );
	if( start == goal )
	{
		out.push_back( to );
		return true;
	}
	int end = start;
	const bool found = search( start, goal, from, to, danger, end );
	if( end == start )
		return false;
	tracePath( start, end, found, to, rawPath );
	straighten( from, rawPath, water, out );
	return found;
}

// from each point keep the furthest of the next few that can be driven to directly
void GroundNav::straighten( Vec3 anchor, const std::vector<Vec3>& raw, bool water, std::vector<Vec3>& out )
{
	constexpr size_t lookAhead = 12;
	size_t i = 0;
	while( i < raw.size() )
	{
		size_t furthest = i;
		for( size_t j = std::min( raw.size() - 1, i + lookAhead ); j > i && furthest == i; j-- )
			furthest = lineOpen( anchor, raw[j], water ) ? j : furthest;
		out.push_back( raw[furthest] );
		anchor = raw[furthest];
		i = furthest + 1;
	}
}
