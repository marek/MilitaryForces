#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "query.h"
#include <cmath>
#include <numbers>

bool PlaceQuery::find( Vec3& out ) const
{
	for( int ring = 0; ring < rings; ring++ )
	{
		const float radius = firstRing + ring * ringStep;
		bool found = false;
		float best = 0.0f;
		for( int step = 0; step < around; step++ )
		{
			Vec3 point;
			if( !candidate( radius, step * (2.0f * std::numbers::pi_v<float> / around), point ) )
				continue;
			if( !std::all_of( filters.begin(), filters.end(), [&point]( const Filter& f ) { return f( point ); } ) )
				continue;
			float score = 0.0f;
			for( const Scorer& scorer : scorers )
				score += scorer( point );
			if( !found || score > best )
			{
				out = point;
				best = score;
				found = true;
			}
		}
		if( found )
			return true;
	}
	return false;
}

// a point on a ring, dropped onto the ground below when asked: not in water, and not up or down a cliff
bool PlaceQuery::candidate( float radius, float angle, Vec3& point ) const
{
	constexpr float above = 300.0f, below = 2000.0f, standOn = 20.0f;
	point = centre;
	point[0] += cos( angle ) * radius;
	point[1] += sin( angle ) * radius;
	if( !onGround )
		return true;
	point[2] += above;
	Vec3 down = point;
	down[2] -= below;
	trace_t tr;
	GameHost::trace( &tr, point, nullptr, nullptr, down, passEntity, MASK_SOLID|MASK_WATER, false );
	if( tr.startsolid || tr.fraction >= 1.0f || (tr.contents & MASK_WATER) )
		return false;
	VectorCopy( tr.endpos, point );
	point[2] += standOn;
	return fabs( point[2] - centre[2] ) <= radius * climbShare;
}

PlaceQuery::Filter PlaceQuery::hiddenFrom( GameEntity* looker, GameEntity* threat )
{
	return [looker, threat]( const Vec3& point ) { return !BotWorld::visible( looker, point, threat ); };
}

// the threat can be seen from this height over the place: a hull-down tank sees and shoots over the crest
PlaceQuery::Filter PlaceQuery::seenOverFrom( GameEntity* looker, GameEntity* threat, float height )
{
	return [looker, threat, height]( const Vec3& point )
	{
		Vec3 top = point;
		top[2] += height;
		return BotWorld::visible( looker, top, threat );
	};
}

PlaceQuery::Scorer PlaceQuery::nearer( const Vec3& to )
{
	return [to]( const Vec3& point ) { return -(point - to).length(); };
}
