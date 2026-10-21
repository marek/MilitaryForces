#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "threat.h"
#include "query.h"
#include "hints.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

// how far it is straight down to ground or water
float BotWorld::heightAboveGround( const vec3_t origin, int passEntity )
{
	trace_t tr;
	Vec3 end;
	VectorCopy( origin, end );
	end[2] -= 16000;
	GameHost::trace( &tr, origin, nullptr, nullptr, end, passEntity, MASK_SOLID|MASK_WATER, false );
	return tr.fraction * 16000.0f;
}

// is something in the way within that distance, for a point or for a body of that half width? The trace tells what
bool BotWorld::pathBlocked( const vec3_t origin, const vec3_t dir, float dist, int passEntity, trace_t& tr, float halfWidth )
{
	Vec3 end;
	VectorMA( origin, dist, dir, end );
	if( halfWidth > 0 )
	{
		Vec3 mins = { -halfWidth, -halfWidth, -halfWidth * 0.4f }, maxs = { halfWidth, halfWidth, halfWidth * 0.4f };
		GameHost::trace( &tr, origin, mins, maxs, end, passEntity, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
		return tr.fraction < 1.0f;
	}
	GameHost::trace( &tr, origin, nullptr, nullptr, end, passEntity, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	return tr.fraction < 1.0f;
}

float BotWorld::padDistance( const vec3_t point )
{
	if( !BotWorld::padsBuilt_ )
		BotWorld::buildPads();
	float nearest = -1.0f;
	for( const BotWorld::Pad& pad : BotWorld::pads_ )
	{
		const float dist = Distance( point, pad.spot );
		if( nearest < 0.0f || dist < nearest )
			nearest = dist;
	}
	return nearest;
}

// how much room there is straight up
float BotWorld::ceilingAbove( const vec3_t origin, int passEntity )
{
	trace_t tr;
	Vec3 end;
	VectorCopy( origin, end );
	end[2] += 8000;
	GameHost::trace( &tr, origin, nullptr, nullptr, end, passEntity, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	return tr.fraction * 8000.0f;
}

// nothing solid between the point and the target
bool BotWorld::visible( GameEntity* ent, const vec3_t from, GameEntity* target )
{
	trace_t tr;
	GameHost::trace( &tr, from, nullptr, nullptr, target->r.currentOrigin, ent->s.number, MASK_SOLID, false );
	return tr.fraction >= 1.0f || tr.entityNum == target->s.number;
}

// Points spread evenly over the whole map, so that roaming takes a bot across it and not round the bases and its own
// spawn; each is also dropped onto the surface under it, from the height of the sky, to be driven or sailed to
void BotWorld::spreadRoamPoints( const Vec3& centre, const float reach[4], float sky )
{
	constexpr int across = 6;
	constexpr float spread = 0.85f;
	BotWorld::landPoints_.clear();
	BotWorld::waterPoints_.clear();
	for( int k = 0; k < across * across; k++ )
	{
		const float fx = (k % across + 0.5f) / across * 2.0f - 1.0f, fy = (k / across + 0.5f) / across * 2.0f - 1.0f;
		Vec3 point = centre;
		point += Vec3( fx * spread * reach[fx > 0 ? 0 : 1], fy * spread * reach[fy > 0 ? 2 : 3], 0 );
		BotWorld::roamPoints_.push_back( point );
		dropRoamPoint( point, sky );
	}
}

// a roam point on the surface under it: on water or on level enough land, never on the sky
void BotWorld::dropRoamPoint( const Vec3& point, float sky )
{
	constexpr float level = 0.7f;
	vec3_t top, bottom;
	point.copyTo( top );
	top[2] = sky;
	VectorCopy( top, bottom );
	bottom[2] = -65536.0f;
	trace_t tr;
	GameHost::trace( &tr, top, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER, false );
	if( tr.startsolid || tr.fraction >= 1.0f || (tr.surfaceFlags & SURF_SKY) )
		return;
	if( tr.contents & MASK_WATER )
		BotWorld::waterPoints_.push_back( Vec3( tr.endpos ) );
	else if( tr.plane.normal[2] >= level )
		BotWorld::landPoints_.push_back( Vec3( tr.endpos ) );
}

// Places to head for when there is nothing else to do: the pads, the middle of the map as the spawn points and
// pads place it, and a point out towards each corner as far as the map goes
void BotWorld::buildRoamPoints()
{
	if( !BotWorld::padsBuilt_ )
		BotWorld::buildPads();
	BotWorld::roamPoints_.clear();

	Vec3 sum( 0, 0, 0 );
	int count = 0;
	float top = -99999.0f;
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->inuse_ || !ent->classname_ )
			continue;
		if( Q_stricmpn( ent->classname_, "info_player", 11 ) != 0 && Q_stricmpn( ent->classname_, "team_CTF", 8 ) != 0 )
			continue;
		sum += Vec3( ent->s.origin );
		top = std::max( top, ent->s.origin[2] );
		count++;
	}
	for( const BotWorld::Pad& pad : BotWorld::pads_ )
	{
		bool near = false;
		for( const Vec3& point : BotWorld::roamPoints_ )
		{
			Vec3 p;
			point.copyTo( p );
			if( Distance( p, pad.spot ) < 1200 )
				near = true;
		}
		if( !near )
			BotWorld::roamPoints_.push_back( Vec3( pad.spot ) );
		sum += Vec3( pad.spot );
		top = std::max( top, pad.spot[2] );
		count++;
	}
	if( !count )
		return;

	Vec3 centre = sum * (1.0f / count);
	Vec3 origin;
	centre.copyTo( origin );
	origin[2] = top + 1500;
	float reach[4];
	for( int k = 0; k < 4; k++ )
	{
		Vec3 dir = { k == 0 ? 1.0f : k == 1 ? -1.0f : 0.0f, k == 2 ? 1.0f : k == 3 ? -1.0f : 0.0f, 0 };
		trace_t tr;
		BotWorld::pathBlocked( origin, dir, 40000, ENTITYNUM_NONE, tr );
		reach[k] = tr.fraction * 40000.0f;
	}
	BotWorld::roamPoints_.push_back( centre );
	for( int qx = 0; qx < 2; qx++ )
	{
		for( int qy = 0; qy < 2; qy++ )
		{
			Vec3 point = centre;
			point += Vec3( (qx ? 0.55f : -0.55f) * reach[qx ? 0 : 1], (qy ? 0.55f : -0.55f) * reach[qy ? 2 : 3], 0 );
			BotWorld::roamPoints_.push_back( point );
		}
	}
	spreadRoamPoints( centre, reach, origin[2] );
}

// is there runway under this point, and at what height?
bool BotWorld::runwayAt( const vec3_t point, float* surface )
{
	Vec3 start, end;
	VectorCopy( point, start );
	start[2] += 60;
	VectorCopy( point, end );
	end[2] -= 140;
	trace_t tr;
	GameHost::trace( &tr, start, nullptr, nullptr, end, ENTITYNUM_NONE, MASK_SOLID, false );
	if( tr.fraction >= 1.0f || tr.entityNum <= ENTITYNUM_WORLD )
		return false;
	GameEntity* ground = theLevel.getEntity( tr.entityNum );
	if( surface )
		*surface = tr.endpos[2];
	return ground && ground->classname_ && strcmp( ground->classname_, "func_runway" ) == 0;
}

// how far the runway goes on from a point in one direction
float BotWorld::runwayRun( const vec3_t origin, float yaw, float maxDist )
{
	Vec3 angles = { 0, yaw, 0 }, dir, point;
	AngleVectors( angles, dir, nullptr, nullptr );
	// wheels cannot cross a step between runway pieces, so the run ends where the surface height changes
	constexpr float maxStep = 0.5f;
	float run = 0, previous = 0, surface = 0;
	if( !BotWorld::runwayAt( origin, &previous ) )
		return 0;
	for( float dist = 25.0f; dist <= maxDist; dist += 25.0f )
	{
		VectorMA( origin, dist, dir, point );
		if( !BotWorld::runwayAt( point, &surface ) || fabs( surface - previous ) > maxStep )
			break;
		previous = surface;
		run = dist;
	}
	return run;
}

// the steepest slope the terrain forces on an approach to one end of a runway (dir -1 lands along the axis)
float BotWorld::approachSlope( const BotWorld::Pad& pad, float dir )
{
	float slope = 0.15f;
	for( float d = 75.0f; d <= 2500.0f; d += d < 1000.0f ? 75.0f : 250.0f )
	{
		Vec3 from, to, boxMins = { -60, -60, 0 }, boxMaxs = { 60, 60, 4 };
		trace_t tr;
		VectorMA( pad.center, -dir * (pad.halfLength - BotWorld::thresholdInset( pad ) + d), pad.axis, from );
		from[2] = pad.z + 600;
		to = from;
		to[2] -= 4000;
		GameHost::trace( &tr, from, boxMins, boxMaxs, to, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
		const float rise = tr.startsolid ? 600.0f : tr.endpos[2] - pad.z;
		slope = std::max( slope, (rise + 60.0f) / (d + 300.0f) );
	}
	return slope;
}

// where a team's flag stands
bool BotWorld::flagPosition( int team, vec3_t out )
{
	const char* name = team == ClientBase::TEAM_RED ? "team_CTF_redflag" : "team_CTF_blueflag";
	for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
	{
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->inuse_ || !ent->classname_ || Q_stricmp( ent->classname_, name ) != 0 )
			continue;
		VectorCopy( ent->r.currentOrigin, out );
		return true;
	}
	return false;
}

namespace
{
	// runway pieces that touch are one runway: maps build an airfield from many brushes
	constexpr int headings = 12;
	// a jet rolls about 600 units before it stops; the shortest strip the maps have is a little under 1,000
	constexpr float minRunway = 900.0f;
	struct Run
	{
		Vec3	center = { 0, 0, 0 };
		Vec3	axis = { 1, 0, 0 };
		float	length = 0;
		float	z = 0;
	};
	// the longest run found in each heading, and the one planes are sent to
	struct Airfield
	{
		Run	runs[headings];
		Run	best;
	};

	bool isRunway( const GameEntity* ent )
	{
		return ent && ent->inuse_ && ent->classname_ && strcmp( ent->classname_, "func_runway" ) == 0;
	}

	bool boxesTouch( const GameEntity* a, const GameEntity* b, float reach )
	{
		for( int k = 0; k < 3; k++ )
			if( a->r.absmin[k] - reach > b->r.absmax[k] || b->r.absmin[k] - reach > a->r.absmax[k] )
				return false;
		return true;
	}
}

namespace
{
	// one pass over every pair of touching pieces in different fields, joining each pair into the lower field
	bool joinTouching( const std::vector<GameEntity*>& pieces, std::vector<int>& field )
	{
		constexpr float touching = 16.0f;
		bool joined = false;
		for( size_t a = 0; a < pieces.size(); a++ )
		{
			for( size_t b = a + 1; b < pieces.size(); b++ )
			{
				if( field[a] == field[b] || !boxesTouch( pieces[a], pieces[b], touching ) )
					continue;
				const int from = std::max( field[a], field[b] ), to = std::min( field[a], field[b] );
				std::replace( field.begin(), field.end(), from, to );
				joined = true;
			}
		}
		return joined;
	}

	// runway pieces that touch are one airfield: each piece's field number, the lowest of the pieces it touches
	std::vector<int> groupPieces( const std::vector<GameEntity*>& pieces )
	{
		std::vector<int> field( pieces.size() );
		for( size_t i = 0; i < pieces.size(); i++ )
			field[i] = static_cast<int>( i );
		for( bool joined = true; joined; )
			joined = joinTouching( pieces, field );
		return field;
	}

	// the runs through one spot on the runway, longest kept per heading
	void measureSpot( Vec3 spot, float surface, Airfield& airfield )
	{
		spot[2] = surface;
		for( int step = 0; step < headings; step++ )
		{
			const float yaw = step * (180.0f / headings);
			const float ahead = BotWorld::runwayRun( spot, yaw, 8000 );
			const float behind = BotWorld::runwayRun( spot, yaw + 180.0f, 8000 );
			Run& run = airfield.runs[step];
			if( ahead + behind <= run.length )
				continue;
			run.length = ahead + behind;
			Vec3 angles = { 0, yaw, 0 };
			AngleVectors( angles, run.axis, nullptr, nullptr );
			VectorMA( spot, (ahead - behind) * 0.5f, run.axis, run.center );
			run.z = surface;
		}
	}

	// the runs through a grid of points over one runway piece
	void measurePiece( const GameEntity* piece, Airfield& airfield )
	{
		constexpr float grid = 128.0f;
		for( float x = piece->r.absmin[0] + grid * 0.5f; x < piece->r.absmax[0]; x += grid )
		{
			for( float y = piece->r.absmin[1] + grid * 0.5f; y < piece->r.absmax[1]; y += grid )
			{
				const Vec3 spot = { x, y, piece->r.absmax[2] };
				float surface = 0;
				if( BotWorld::runwayAt( spot, &surface ) )
					measureSpot( spot, surface, airfield );
			}
		}
	}

	// the longest straight run on each airfield, looked for from a grid of points over every piece: a piece is often
	// many brushes in one entity, an apron and a strip together, so its middle says nothing
	void measureRuns( const std::vector<GameEntity*>& pieces, const std::vector<int>& field, std::vector<Airfield>& fields )
	{
		for( size_t i = 0; i < pieces.size(); i++ )
			measurePiece( pieces[i], fields[field[i]] );
	}

	// nothing may stand on a run: aprons have hangars and stores on them that a rolling plane would hit
	bool runIsClear( const Run& run )
	{
		Vec3 from, to, boxMins = { -45, -45, 0 }, boxMaxs = { 45, 45, 40 };
		trace_t tr;
		VectorMA( run.center, -(run.length * 0.5f - 50.0f), run.axis, from );
		VectorMA( run.center, run.length * 0.5f - 50.0f, run.axis, to );
		from[2] = to[2] = run.z + 12.0f;
		GameHost::trace( &tr, from, boxMins, boxMaxs, to, ENTITYNUM_NONE, MASK_PLAYERSOLID, false );
		return !tr.startsolid && tr.fraction >= 1.0f;
	}

	// the steeper of the two ways in, the flatter end's
	float runSlope( const Run& run )
	{
		BotWorld::Pad probe;
		probe.center = run.center;
		probe.center[2] = run.z;
		probe.axis = run.axis;
		probe.halfLength = run.length * 0.5f;
		probe.z = run.z;
		return std::min( BotWorld::approachSlope( probe, 1.0f ), BotWorld::approachSlope( probe, -1.0f ) );
	}

	// of the runs long enough to land on, the one with the flattest way in from either end; failing that the longest
	void chooseRun( Airfield& airfield )
	{
		float bestSlope = 0;
		for( const Run& run : airfield.runs )
		{
			if( run.length > airfield.best.length && airfield.best.length < minRunway )
				airfield.best = run;
			if( run.length < minRunway || !runIsClear( run ) )
				continue;
			const float slope = runSlope( run );
			if( bestSlope == 0 || slope < bestSlope - 0.03f || (slope < bestSlope + 0.03f && run.length > airfield.best.length) )
			{
				bestSlope = slope;
				airfield.best = run;
			}
		}
	}

	// a recharge spot, and the runway of the airfield it stands on or beside, where a plane lands before it taxis to it
	BotWorld::Pad padFor( GameEntity* ent, const std::vector<GameEntity*>& pieces, const std::vector<int>& field, const std::vector<Airfield>& fields )
	{
		constexpr float beside = 64.0f;
		BotWorld::Pad pad;
		for( int k = 0; k < 3; k++ )
			pad.center[k] = (ent->r.absmin[k] + ent->r.absmax[k]) * 0.5f;
		VectorCopy( pad.center, pad.spot );
		VectorSet( pad.axis, 1, 0, 0 );
		pad.halfLength = 0;
		pad.z = ent->r.absmin[2];
		pad.runway = false;
		pad.category = ent->ent_category_ ? ent->ent_category_ : CAT_ANY;
		pad.team = ent->s.generic1;
		Vec3 start, end;
		VectorCopy( pad.center, start );
		start[2] = ent->r.absmax[2] + 8;
		end = start;
		end[2] -= 600;
		trace_t tr;
		GameHost::trace( &tr, start, nullptr, nullptr, end, ENTITYNUM_NONE, MASK_SOLID, false );
		if( tr.fraction < 1.0f )
			pad.z = tr.endpos[2];
		int on = -1;
		for( size_t p = 0; p < pieces.size(); p++ )
			if( boxesTouch( ent, pieces[p], beside ) && (on < 0 || fields[field[p]].best.length > fields[on].best.length) )
				on = field[p];
		if( on < 0 || fields[on].best.length <= 0 )
			return pad;
		const Run& run = fields[on].best;
		pad.center = run.center;
		pad.axis = run.axis;
		pad.z = run.z;
		pad.center[2] = pad.z;
		pad.halfLength = run.length * 0.5f;
		pad.runway = run.length >= minRunway;
		return pad;
	}
}

// Every place a vehicle can refuel, and for those on or beside an airfield the runway a plane lands on to reach it
void BotWorld::buildPads()
{
	BotWorld::padsBuilt_ = true;
	BotWorld::pads_.clear();
	std::vector<GameEntity*> pieces;
	for( int i = 0; i < theLevel.num_entities_; i++ )
		if( isRunway( theLevel.getEntity( i ) ) )
			pieces.push_back( theLevel.getEntity( i ) );
	const std::vector<int> field = groupPieces( pieces );
	std::vector<Airfield> fields( pieces.size() );
	measureRuns( pieces, field, fields );
	for( Airfield& airfield : fields )
		chooseRun( airfield );
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* ent = theLevel.getEntity( i );
		if( ent && ent->inuse_ && ent->classname_ && strcmp( ent->classname_, "trigger_recharge" ) == 0 )
			BotWorld::pads_.push_back( padFor( ent, pieces, field, fields ) );
	}
}

// the nearest pad this vehicle may use: of its kind, of its team, and for a plane one with a runway
int BotWorld::findPad( GameEntity* ent, const Bots::Rearm* rearm )
{
	if( !BotWorld::padsBuilt_ )
		BotWorld::buildPads();

	const GameClient* client = ent->client_;
	const unsigned int cat = availableVehicles[client->vehicle_].cat;
	int best = -1;
	float bestDist = 0;
	for( size_t i = 0; i < BotWorld::pads_.size(); i++ )
	{
		const BotWorld::Pad& pad = BotWorld::pads_[i];
		if( !(pad.category & cat) || ((cat & CAT_PLANE) && !pad.runway) || (rearm && rearm->avoided( static_cast<int>( i ), theLevel.time_ )) )
			continue;
		if( g_gametype.integer >= GT_TEAM && pad.team != ClientBase::TEAM_FREE && pad.team != client->sess_.sessionTeam_ )
			continue;
		const float dist = Distance( ent->r.currentOrigin, pad.center );
		if( best < 0 || dist < bestDist )
		{
			best = static_cast<int>( i );
			bestDist = dist;
		}
	}
	return best;
}

// A spot within about a thousand units that the threat cannot see; hull down means one from which the bot's own
// gun still can see the threat
bool Bots::findHidePoint( GameEntity* ent, State& bot, GameEntity* threat, bool hullDown )
{
	PlaceQuery query;
	query.centre = Vec3( ent->r.currentOrigin );
	query.passEntity = ent->s.number;
	query.filters.push_back( PlaceQuery::hiddenFrom( ent, threat ) );
	if( hullDown )
		query.filters.push_back( PlaceQuery::seenOverFrom( ent, threat, availableVehicles[ent->client_->vehicle_].maxs[2] + 12 ) );
	query.scorers.push_back( PlaceQuery::nearer( query.centre ) );
	Vec3 place;
	if( !query.find( place ) )
		return false;
	place.copyTo( bot.cover.point );
	return true;
}

// a building to bomb: the nearest that can be destroyed, those by the enemy's pads first and never one by the
// bot's own; the choice is kept for a few seconds
GameEntity* Bots::findStructure( GameEntity* ent, State& bot )
{
	if( theLevel.time_ < bot.structureTime )
	{
		GameEntity* current = bot.structure >= 0 ? theLevel.getEntity( bot.structure ) : nullptr;
		if( current && standing( current ) )
			return current;
		if( bot.structure < 0 )
			return nullptr;
	}
	bot.structureTime = theLevel.time_ + 6000;
	bot.structure = -1;
	if( !BotWorld::padsBuilt_ )
		BotWorld::buildPads();
	const int team = ent->client_->sess_.sessionTeam_;
	GameEntity* best = nullptr;
	float bestScore = 0;
	for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !other || !standing( other ) )
			continue;
		const float score = structureScore( ent, other, team );
		if( score >= 0.0f && (!best || score < bestScore) )
		{
			best = other;
			bestScore = score;
		}
	}
	if( best )
		bot.structure = best->s.number;
	return best;
}

// a building that can still be destroyed
bool Bots::standing( GameEntity* structure )
{
	return structure->inuse_ && structure->s.eType == ET_EXPLOSIVE && structure->takedamage_ && structure->health_ > 0;
}

// its distance, shortened near an enemy base; in a team game one only near the bot's own pads is no target (below 0)
float Bots::structureScore( GameEntity* ent, GameEntity* structure, int team )
{
	constexpr float baseReach = 2500.0f, enemyBaseShare = 0.4f;
	Vec3 centre;
	for( int k = 0; k < 3; k++ )
		centre[k] = (structure->r.absmin[k] + structure->r.absmax[k]) * 0.5f;
	const float dist = Distance( ent->r.currentOrigin, centre );
	if( g_gametype.integer < GT_TEAM )
		return dist;
	bool own = false, enemy = false;
	for( const BotWorld::Pad& pad : BotWorld::pads_ )
	{
		if( pad.team == ClientBase::TEAM_FREE || Distance( pad.spot, centre ) > baseReach )
			continue;
		(pad.team == team ? own : enemy) = true;
	}
	if( own && !enemy )
		return -1.0f;
	return enemy ? dist * enemyBaseShare : dist;
}

void Bots::pickGoal( GameEntity* ent, State& bot, float radius )
{
	if( !bot.hasHome )
	{
		VectorCopy( ent->r.currentOrigin, bot.home );
		bot.hasHome = true;
	}
	if( bot.goalTime )
	{
		if( theLevel.time_ > bot.goalTime )
			bot.goalsExpired++;
		else
			bot.goalsReached++;
	}
	if( BotWorld::roamPoints_.empty() )
		BotWorld::buildRoamPoints();
	const unsigned int cat = ent->client_->vehicle_ >= 0 ? availableVehicles[ent->client_->vehicle_].cat : 0;
	GameEntity* quarry = nullptr;
	// Everyone has a rough idea where the enemy is. Part of the time a bot goes looking for the nearest opponent
	// it could fight, the aggressive ones more often, so nobody is left alone for long.
	if( !(cat & CAT_LQM) && random() < 0.3f + 0.4f * character( bot ).aggression )
		quarry = nearestQuarry( ent, cat );
	if( quarry )
		goalNear( ent, bot, quarry, cat );
	else if( !goalAtHint( ent, bot, cat ) )
	{
		if( (cat & (CAT_PLANE|CAT_HELO)) && !BotWorld::roamPoints_.empty() && radius > 3500 )
			goalAtRoamPoint( ent, bot, BotWorld::roamPoints_, true );
		else if( (cat & CAT_GROUND) && !BotWorld::landPoints_.empty() )
			goalAtRoamPoint( ent, bot, BotWorld::landPoints_, false );
		else if( (cat & CAT_BOAT) && !BotWorld::waterPoints_.empty() )
			goalAtRoamPoint( ent, bot, BotWorld::waterPoints_, false );
		else
			goalAroundHome( ent, bot, radius, cat );
	}
	bot.goalTime = theLevel.time_ + 20000;
}

// the nearest opponent this vehicle could go after: a boat cannot go after someone inland, nor a ground vehicle after
// someone at sea or in the air
GameEntity* Bots::nearestQuarry( GameEntity* ent, unsigned int cat )
{
	GameEntity* quarry = nullptr;
	float nearest = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
			continue;
		const unsigned int otherCat = availableVehicles[other->client_->vehicle_].cat;
		const bool otherFlying = (otherCat & (CAT_PLANE|CAT_HELO)) && !(other->client_->ps_.ONOFF & OO_LANDED);
		if( (cat & (CAT_GROUND|CAT_BOAT)) && (otherFlying || ((cat & CAT_BOAT) != 0) != ((otherCat & CAT_BOAT) != 0)) )
			continue;
		const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		if( !quarry || dist < nearest )
		{
			quarry = other;
			nearest = dist;
		}
	}
	return quarry;
}

// roughly where the quarry is: a rough idea, not a fix
void Bots::goalNear( GameEntity* ent, State& bot, GameEntity* quarry, unsigned int cat )
{
	constexpr float vagueness = 900.0f;
	VectorCopy( quarry->r.currentOrigin, bot.goal );
	bot.goal[0] += crandom() * vagueness;
	bot.goal[1] += crandom() * vagueness;
	if( !(cat & (CAT_PLANE|CAT_HELO)) )
		bot.goal[2] = ent->r.currentOrigin[2];
}

// one of a few roam points drawn at random, the least dangerous of them by the team's threat map: a cautious pilot
// weighs the danger more, and none of them refuses to go where the fighting is. A point on the surface is gone to as
// it is, a point in the air with some scatter
void Bots::goalAtRoamPoint( GameEntity* ent, State& bot, const std::vector<Vec3>& points, bool air )
{
	constexpr int drawn = 4;
	const float scatter = air ? 800.0f : 0.0f;
	const int team = ent->client_->sess_.sessionTeam_;
	const float caution = character( bot ).caution;
	const Vec3* best = nullptr;
	float bestScore = 0.0f;
	for( int i = 0; i < drawn; i++ )
	{
		const Vec3& point = points[rand() % points.size()];
		vec3_t at;
		point.copyTo( at );
		const float score = caution * ThreatMap::danger( team, at, air ) + random() * 0.3f;
		if( !best || score < bestScore )
		{
			best = &point;
			bestScore = score;
		}
	}
	best->copyTo( bot.goal );
	bot.goal[0] += crandom() * scatter;
	bot.goal[1] += crandom() * scatter;
}

// Now and then, by cunning, a bot goes to a place worth knowing: a gunship to a spot to lie in wait, a pilot along a
// low corridor, armour to a choke on the way between the bases
bool Bots::goalAtHint( GameEntity* ent, State& bot, unsigned int cat )
{
	constexpr float hintShare = 0.4f;
	if( random() > hintShare * character( bot ).cunning )
		return false;
	const Capabilities can = Capabilities::of( ent->client_->vehicle_ );
	const BotHints::Hint* hint = nullptr;
	if( can.gunship )
		hint = BotHints::any( BotHints::Kind::Ambush );
	else if( can.runway )
		hint = BotHints::any( BotHints::Kind::Corridor );
	else if( can.armoured && !can.antiAirVehicle )
		hint = BotHints::any( BotHints::Kind::Choke );
	if( !hint )
		return false;
	hint->at.copyTo( bot.goal );
	if( !(cat & (CAT_PLANE|CAT_HELO)) )
		bot.goal[2] = ent->r.currentOrigin[2];
	return true;
}

// somewhere within the radius of where it first appeared; a boat's goal is moved until it is on water
void Bots::goalAroundHome( GameEntity* ent, State& bot, float radius, unsigned int cat )
{
	const float angle = random() * 2.0f * std::numbers::pi_v<float>;
	const float dist = radius * (0.3f + 0.7f * random());
	bot.goal[0] = bot.home[0] + cos( angle ) * dist;
	bot.goal[1] = bot.home[1] + sin( angle ) * dist;
	bot.goal[2] = bot.home[2];
	if( !(cat & CAT_BOAT) )
		return;
	for( int tries = 0; tries < 8; tries++ )
	{
		Vec3 probe;
		VectorCopy( bot.goal, probe );
		probe[2] = ent->r.currentOrigin[2] - 6;
		if( GameHost::pointContents( probe, ent->s.number ) & MASK_WATER )
			return;
		const float a = random() * 2.0f * std::numbers::pi_v<float>;
		const float d = radius * (0.2f + 0.8f * random());
		bot.goal[0] = ent->r.currentOrigin[0] + cos( a ) * d;
		bot.goal[1] = ent->r.currentOrigin[1] + sin( a ) * d;
	}
}
