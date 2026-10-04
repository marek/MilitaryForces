#include "../g_local.h"
#include "../g_level.h"
#include "bot_airnav.h"
#include <algorithm>
#include <cmath>
#include <queue>

bool AirNav::built_ = false;
Vec3 AirNav::mins_;
float AirNav::cellSize_ = 320.0f;
int AirNav::nx_ = 0;
int AirNav::ny_ = 0;
int AirNav::nz_ = 0;
std::vector<signed char> AirNav::state_;
std::vector<float> AirNav::cost_;
std::vector<int> AirNav::parent_;
std::vector<int> AirNav::stamp_;
int AirNav::generation_ = 0;

namespace
{
	constexpr int maxCells = 600000;
	constexpr int maxExpansions = 30000;
	constexpr float cellHalfBox = 105.0f;
}

void AirNav::reset()
{
	built_ = false;
	state_.clear();
	cost_.clear();
	parent_.clear();
	stamp_.clear();
	nx_ = ny_ = nz_ = 0;
}

void AirNav::build()
{
	built_ = true;
	vec3_t worldMins, worldMaxs;
	CollisionModel::modelBounds( 0, worldMins, worldMaxs );
	mins_ = Vec3( worldMins );
	cellSize_ = 320.0f;
	for( ;; )
	{
		nx_ = std::max( 1, static_cast<int>( ceil( (worldMaxs[0] - worldMins[0]) / cellSize_ ) ) );
		ny_ = std::max( 1, static_cast<int>( ceil( (worldMaxs[1] - worldMins[1]) / cellSize_ ) ) );
		nz_ = std::max( 1, static_cast<int>( ceil( (worldMaxs[2] - worldMins[2]) / cellSize_ ) ) );
		if( static_cast<long long>( nx_ ) * ny_ * nz_ <= maxCells )
			break;
		cellSize_ *= 1.25f;
	}
	const size_t total = static_cast<size_t>( nx_ ) * ny_ * nz_;
	state_.assign( total, 0 );
	cost_.assign( total, 0.0f );
	parent_.assign( total, -1 );
	stamp_.assign( total, 0 );
	generation_ = 0;
}

int AirNav::cellCount()
{
	return nx_ * ny_ * nz_;
}

int AirNav::freeCount()
{
	return static_cast<int>( std::count( state_.begin(), state_.end(), 1 ) );
}

int AirNav::cellAt( const Vec3& point )
{
	const int x = static_cast<int>( floor( (point.x() - mins_.x()) / cellSize_ ) );
	const int y = static_cast<int>( floor( (point.y() - mins_.y()) / cellSize_ ) );
	const int z = static_cast<int>( floor( (point.z() - mins_.z()) / cellSize_ ) );
	if( x < 0 || y < 0 || z < 0 || x >= nx_ || y >= ny_ || z >= nz_ )
		return -1;
	return (z * ny_ + y) * nx_ + x;
}

Vec3 AirNav::centre( int cell )
{
	const int x = cell % nx_;
	const int y = (cell / nx_) % ny_;
	const int z = cell / (nx_ * ny_);
	return mins_ + Vec3( (x + 0.5f) * cellSize_, (y + 0.5f) * cellSize_, (z + 0.5f) * cellSize_ );
}

bool AirNav::isFree( int cell )
{
	if( cell < 0 )
		return false;
	if( state_[cell] == 0 )
	{
		const Vec3 point = centre( cell );
		vec3_t boxMins = { -cellHalfBox, -cellHalfBox, -cellHalfBox * 0.6f };
		vec3_t boxMaxs = { cellHalfBox, cellHalfBox, cellHalfBox * 0.6f };
		trace_t tr;
		SV_Trace( &tr, point, boxMins, boxMaxs, point, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
		state_[cell] = (tr.startsolid || tr.allsolid) ? -1 : 1;
	}
	return state_[cell] == 1;
}

int AirNav::nearestFree( const Vec3& point )
{
	const int cell = cellAt( point );
	if( cell >= 0 && isFree( cell ) )
		return cell;
	int best = -1;
	float bestDist = 0;
	for( int dz = -2; dz <= 2; dz++ )
	{
		for( int dy = -2; dy <= 2; dy++ )
		{
			for( int dx = -2; dx <= 2; dx++ )
			{
				const Vec3 probe = point + Vec3( dx * cellSize_, dy * cellSize_, dz * cellSize_ );
				const int other = cellAt( probe );
				if( other < 0 || !isFree( other ) )
					continue;
				const float dist = (centre( other ) - point).lengthSquared();
				if( best < 0 || dist < bestDist )
				{
					best = other;
					bestDist = dist;
				}
			}
		}
	}
	return best;
}

bool AirNav::clearLine( const Vec3& a, const Vec3& b, float halfWidth )
{
	vec3_t boxMins = { -halfWidth, -halfWidth, -halfWidth * 0.4f };
	vec3_t boxMaxs = { halfWidth, halfWidth, halfWidth * 0.4f };
	trace_t tr;
	SV_Trace( &tr, a, boxMins, boxMaxs, b, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	if( tr.startsolid )
		SV_Trace( &tr, a, nullptr, nullptr, b, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	return tr.fraction >= 1.0f || tr.entityNum != ENTITYNUM_WORLD;
}

bool AirNav::findPath( const Vec3& from, const Vec3& to, std::vector<Vec3>& out )
{
	out.clear();
	if( !built_ )
		build();

	const int start = nearestFree( from );
	const int goal = nearestFree( to );
	if( start < 0 || goal < 0 )
		return false;
	if( start == goal )
	{
		out.push_back( to );
		return true;
	}

	generation_++;
	using Entry = std::pair<float, int>;
	std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
	const Vec3 goalCentre = centre( goal );
	stamp_[start] = generation_;
	cost_[start] = 0;
	parent_[start] = -1;
	open.push( { (centre( start ) - goalCentre).length(), start } );

	bool found = false;
	int expansions = 0;
	while( !open.empty() && expansions < maxExpansions )
	{
		const int cell = open.top().second;
		const float queued = open.top().first;
		open.pop();
		const Vec3 here = centre( cell );
		if( queued > cost_[cell] + (here - goalCentre).length() + 1.0f )
			continue;
		if( cell == goal )
		{
			found = true;
			break;
		}
		expansions++;

		const int x = cell % nx_;
		const int y = (cell / nx_) % ny_;
		const int z = cell / (nx_ * ny_);
		for( int dz = -1; dz <= 1; dz++ )
		{
			for( int dy = -1; dy <= 1; dy++ )
			{
				for( int dx = -1; dx <= 1; dx++ )
				{
					if( !dx && !dy && !dz )
						continue;
					const int ox = x + dx, oy = y + dy, oz = z + dz;
					if( ox < 0 || oy < 0 || oz < 0 || ox >= nx_ || oy >= ny_ || oz >= nz_ )
						continue;
					const int other = (oz * ny_ + oy) * nx_ + ox;
					if( !isFree( other ) )
						continue;
					if( (dx && dy) || (dx && dz) || (dy && dz) )
					{
						if( dx && !isFree( (z * ny_ + y) * nx_ + ox ) ) continue;
						if( dy && !isFree( (z * ny_ + oy) * nx_ + x ) ) continue;
						if( dz && !isFree( (oz * ny_ + y) * nx_ + x ) ) continue;
					}
					float step = cellSize_ * sqrtf( static_cast<float>( dx * dx + dy * dy + dz * dz ) );
					int walls = 0;
					if( ox > 0 && !isFree( other - 1 ) ) walls++;
					if( ox < nx_ - 1 && !isFree( other + 1 ) ) walls++;
					if( oy > 0 && !isFree( other - nx_ ) ) walls++;
					if( oy < ny_ - 1 && !isFree( other + nx_ ) ) walls++;
					if( oz > 0 && !isFree( other - nx_ * ny_ ) ) walls++;
					if( oz < nz_ - 1 && !isFree( other + nx_ * ny_ ) ) walls++;
					step *= 1.0f + 0.35f * walls + (dz ? 0.3f : 0.0f);
					const float total = cost_[cell] + step;
					if( stamp_[other] == generation_ && total >= cost_[other] )
						continue;
					stamp_[other] = generation_;
					cost_[other] = total;
					parent_[other] = cell;
					open.push( { total + (centre( other ) - goalCentre).length(), other } );
				}
			}
		}
	}
	if( !found )
		return false;

	std::vector<Vec3> raw;
	raw.push_back( to );
	for( int cell = parent_[goal]; cell >= 0 && cell != start; cell = parent_[cell] )
		raw.push_back( centre( cell ) );
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
	return true;
}
