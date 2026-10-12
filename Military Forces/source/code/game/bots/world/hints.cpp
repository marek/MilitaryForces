#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "hints.h"
#include "query.h"
#include <cstring>

std::vector<BotHints::Hint> BotHints::hints_;
bool BotHints::built_ = false;
bool BotHints::fromMap_ = false;

namespace
{
	constexpr float sampleStep = 1000.0f;	// along the line between two bases
	constexpr float corridorHeight = 200.0f;	// above the ground
	constexpr float chokeWidth = 1500.0f;	// a way narrower than this between walls is a choke
	constexpr float lookAcross = 3000.0f;
}

void BotHints::reset()
{
	hints_.clear();
	built_ = false;
	fromMap_ = false;
}

void BotHints::fromEntity( const vec3_t origin, const char* kind )
{
	Kind k = Kind::Station;
	if( !Q_stricmp( kind, "ambush" ) )
		k = Kind::Ambush;
	else if( !Q_stricmp( kind, "corridor" ) )
		k = Kind::Corridor;
	else if( !Q_stricmp( kind, "choke" ) )
		k = Kind::Choke;
	hints_.push_back( { Vec3( origin ), k } );
	fromMap_ = true;
}

const BotHints::Hint* BotHints::any( Kind kind )
{
	if( !built_ )
		build();
	std::vector<const Hint*> found;
	for( const Hint& hint : hints_ )
	{
		if( hint.kind == kind )
			found.push_back( &hint );
	}
	return found.empty() ? nullptr : found[rand() % found.size()];
}

void BotHints::build()
{
	built_ = true;
	if( !fromMap_ )
		generate();
}

// The roam points are the patrol stations. Between the two teams' bases (or, without teams, from each roam point to
// the map's middle) the way is sampled for low corridors, chokes and places to lie in wait
void BotHints::generate()
{
	if( BotWorld::roamPoints_.empty() )
		BotWorld::buildRoamPoints();
	for( const Vec3& point : BotWorld::roamPoints_ )
		hints_.push_back( { point, Kind::Station } );
	Vec3 red, blue;
	if( baseOf( ClientBase::TEAM_RED, red ) && baseOf( ClientBase::TEAM_BLUE, blue ) )
		alongBaseLine( red, blue );
	else if( !BotWorld::roamPoints_.empty() )
	{
		const Vec3 middle = BotWorld::roamPoints_[0];
		for( size_t i = 1; i < BotWorld::roamPoints_.size(); i++ )
			alongBaseLine( BotWorld::roamPoints_[i], middle );
	}
}

// a team's base: its flag, or the middle of its pads
bool BotHints::baseOf( int team, Vec3& out )
{
	vec3_t flag;
	if( BotWorld::flagPosition( team, flag ) )
	{
		out = Vec3( flag );
		return true;
	}
	Vec3 sum( 0, 0, 0 );
	int count = 0;
	for( const BotWorld::Pad& pad : BotWorld::pads_ )
	{
		if( pad.team != team )
			continue;
		sum += Vec3( pad.spot );
		count++;
	}
	if( !count )
		return false;
	out = sum * (1.0f / count);
	return true;
}

// the ground under a point, from well above it; false over water or nothing
bool BotHints::groundBelow( Vec3& point )
{
	constexpr float above = 4000.0f, below = 8000.0f;
	Vec3 top = point, bottom = point;
	top[2] += above;
	bottom[2] -= below;
	trace_t tr;
	GameHost::trace( &tr, top, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER, false );
	if( tr.startsolid || tr.fraction >= 1.0f || (tr.contents & MASK_WATER) )
		return false;
	point = Vec3( tr.endpos );
	return true;
}

// Samples along the way between two places, leaving the ends alone: a corridor point low over the ground at each,
// a choke where walls stand close either side, and at the middle third a place hidden from the far end
void BotHints::alongBaseLine( const Vec3& from, const Vec3& to )
{
	const Vec3 way = to - from;
	const float length = way.length();
	if( length < sampleStep * 3.0f )
		return;
	Vec3 along = way * (1.0f / length), across( -along.y(), along.x(), 0.0f );
	for( float d = sampleStep; d < length - sampleStep; d += sampleStep )
	{
		Vec3 point = from + along * d;
		if( !groundBelow( point ) )
			continue;
		Vec3 low = point;
		low[2] += corridorHeight;
		hints_.push_back( { low, Kind::Corridor } );
		trace_t left, right;
		Vec3 eye = point;
		eye[2] += 60.0f;
		Vec3 minus = across * -1.0f;
		BotWorld::pathBlocked( eye, across, lookAcross, ENTITYNUM_NONE, left );
		BotWorld::pathBlocked( eye, minus, lookAcross, ENTITYNUM_NONE, right );
		if( (left.fraction + right.fraction) * lookAcross < chokeWidth )
			hints_.push_back( { point, Kind::Choke } );
		if( d < length / 3.0f || d > length * 2.0f / 3.0f )
			continue;
		PlaceQuery query;
		query.centre = point;
		query.firstRing = 300.0f;
		query.ringStep = 400.0f;
		const Vec3 farEnd = to;
		query.filters.push_back( [farEnd]( const Vec3& spot )
		{
			trace_t tr;
			Vec3 eye = spot;
			eye[2] += 40.0f;
			GameHost::trace( &tr, eye, nullptr, nullptr, farEnd, ENTITYNUM_NONE, MASK_SOLID, false );
			return tr.fraction < 1.0f;
		} );
		query.scorers.push_back( PlaceQuery::nearer( point ) );
		Vec3 ambush;
		if( query.find( ambush ) )
			hints_.push_back( { ambush, Kind::Ambush } );
	}
}
