#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

float Bots::heightAboveGround( const vec3_t origin, int passEntity )
{
	trace_t tr;
	Vec3 end;
	VectorCopy( origin, end );
	end[2] -= 16000;
	GameHost::trace( &tr, origin, nullptr, nullptr, end, passEntity, MASK_SOLID|MASK_WATER, false );
	return tr.fraction * 16000.0f;
}

bool Bots::pathBlocked( const vec3_t origin, const vec3_t dir, float dist, int passEntity, trace_t& tr, float halfWidth )
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

float Bots::ceilingAbove( const vec3_t origin, int passEntity )
{
	trace_t tr;
	Vec3 end;
	VectorCopy( origin, end );
	end[2] += 8000;
	GameHost::trace( &tr, origin, nullptr, nullptr, end, passEntity, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	return tr.fraction * 8000.0f;
}

bool Bots::visible( GameEntity* ent, const vec3_t from, GameEntity* target )
{
	trace_t tr;
	GameHost::trace( &tr, from, nullptr, nullptr, target->r.currentOrigin, ent->s.number, MASK_SOLID, false );
	return tr.fraction >= 1.0f || tr.entityNum == target->s.number;
}

void Bots::buildRoamPoints()
{
	if( !padsBuilt_ )
		buildPads();
	roamPoints_.clear();

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
	for( const Pad& pad : pads_ )
	{
		bool near = false;
		for( const Vec3& point : roamPoints_ )
		{
			Vec3 p;
			point.copyTo( p );
			if( Distance( p, pad.spot ) < 1200 )
				near = true;
		}
		if( !near )
			roamPoints_.push_back( Vec3( pad.spot ) );
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
		pathBlocked( origin, dir, 40000, ENTITYNUM_NONE, tr );
		reach[k] = tr.fraction * 40000.0f;
	}
	roamPoints_.push_back( centre );
	for( int qx = 0; qx < 2; qx++ )
	{
		for( int qy = 0; qy < 2; qy++ )
		{
			Vec3 point = centre;
			point += Vec3( (qx ? 0.55f : -0.55f) * reach[qx ? 0 : 1], (qy ? 0.55f : -0.55f) * reach[qy ? 2 : 3], 0 );
			roamPoints_.push_back( point );
		}
	}
}

bool Bots::steerAround( GameEntity* ent, float wantYaw, float travel, float& yaw )
{
	constexpr std::array<float, 9> offsets = { 0.0f, 20.0f, -20.0f, 45.0f, -45.0f, 75.0f, -75.0f, 110.0f, -110.0f };
	for( float offset : offsets )
	{
		Vec3 angles = { 0, wantYaw + offset, 0 }, dir;
		AngleVectors( angles, dir, nullptr, nullptr );
		trace_t tr;
		if( !pathBlocked( ent->r.currentOrigin, dir, travel, ent->s.number, tr ) || tr.entityNum != ENTITYNUM_WORLD )
		{
			yaw = wantYaw + offset;
			return true;
		}
	}
	return false;
}

bool Bots::runwayAt( const vec3_t point, float* surface )
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

float Bots::runwayRun( const vec3_t origin, float yaw, float maxDist )
{
	Vec3 angles = { 0, yaw, 0 }, dir, point;
	AngleVectors( angles, dir, nullptr, nullptr );
	// wheels cannot cross a step between runway pieces, so the run ends where the surface height changes
	constexpr float maxStep = 1.5f;
	float run = 0, previous = 0, surface = 0;
	if( !runwayAt( origin, &previous ) )
		return 0;
	for( float dist = 25.0f; dist <= maxDist; dist += 25.0f )
	{
		VectorMA( origin, dist, dir, point );
		if( !runwayAt( point, &surface ) || fabs( surface - previous ) > maxStep )
			break;
		previous = surface;
		run = dist;
	}
	return run;
}

bool Bots::flagPosition( int team, vec3_t out )
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

void Bots::buildPads()
{
	padsBuilt_ = true;
	pads_.clear();
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->inuse_ || !ent->classname_ || strcmp( ent->classname_, "trigger_recharge" ) != 0 )
			continue;

		Pad pad;
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
		{
			pad.z = tr.endpos[2];
			GameEntity* ground = tr.entityNum > ENTITYNUM_WORLD ? theLevel.getEntity( tr.entityNum ) : nullptr;
			if( ground && ground->classname_ && strcmp( ground->classname_, "func_runway" ) == 0 )
			{
				Vec3 spot;
				VectorCopy( pad.center, spot );
				spot[2] = pad.z;
				float best = 0;
				for( int step = 0; step < 12; step++ )
				{
					const float yaw = step * 15.0f;
					const float ahead = runwayRun( spot, yaw, 4000 );
					const float behind = runwayRun( spot, yaw + 180.0f, 4000 );
					if( ahead + behind > best )
					{
						best = ahead + behind;
						Vec3 angles = { 0, yaw, 0 };
						AngleVectors( angles, pad.axis, nullptr, nullptr );
						VectorMA( spot, (ahead - behind) * 0.5f, pad.axis, pad.center );
					}
				}
				pad.halfLength = best * 0.5f;
				pad.runway = best >= 1000.0f;
				pad.center[2] = pad.z;
			}
		}
		pads_.push_back( pad );
	}
}

int Bots::findPad( GameEntity* ent, int skip )
{
	if( !padsBuilt_ )
		buildPads();

	const GameClient* client = ent->client_;
	const unsigned int cat = availableVehicles[client->vehicle_].cat;
	int best = -1;
	float bestDist = 0;
	for( size_t i = 0; i < pads_.size(); i++ )
	{
		const Pad& pad = pads_[i];
		if( !(pad.category & cat) || ((cat & CAT_PLANE) && !pad.runway) || static_cast<int>( i ) == skip )
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

bool Bots::findHidePoint( GameEntity* ent, State& bot, GameEntity* threat, bool hullDown )
{
	float bestDist = 0;
	bool found = false;
	for( int ring = 0; ring < 3; ring++ )
	{
		const float radius = 350.0f + ring * 450.0f;
		for( int step = 0; step < 12; step++ )
		{
			const float angle = step * (2.0f * std::numbers::pi_v<float> / 12.0f);
			Vec3 point, down;
			VectorCopy( ent->r.currentOrigin, point );
			point[0] += cos( angle ) * radius;
			point[1] += sin( angle ) * radius;
			point[2] += 300;
			down = point;
			down[2] -= 2000;
			trace_t tr;
			GameHost::trace( &tr, point, nullptr, nullptr, down, ent->s.number, MASK_SOLID|MASK_WATER, false );
			if( tr.startsolid || tr.fraction >= 1.0f || (tr.contents & MASK_WATER) )
				continue;
			VectorCopy( tr.endpos, point );
			point[2] += 20;
			if( fabs( point[2] - ent->r.currentOrigin[2] ) > radius * 0.5f )
				continue;
			if( visible( ent, point, threat ) )
				continue;
			if( hullDown )
			{
				Vec3 top;
				top = point;
				top[2] += availableVehicles[ent->client_->vehicle_].maxs[2] + 12;
				if( !visible( ent, top, threat ) )
					continue;
			}
			const float dist = Distance( ent->r.currentOrigin, point );
			if( !found || dist < bestDist )
			{
				VectorCopy( point, bot.hidePoint );
				bestDist = dist;
				found = true;
			}
		}
		if( found )
			break;
	}
	return found;
}

GameEntity* Bots::findStructure( GameEntity* ent, State& bot )
{
	if( theLevel.time_ < bot.structureTime )
	{
		GameEntity* current = bot.structure >= 0 ? theLevel.getEntity( bot.structure ) : nullptr;
		if( current && current->inuse_ && current->s.eType == ET_EXPLOSIVE && current->takedamage_ && current->health_ > 0 )
			return current;
		if( bot.structure < 0 )
			return nullptr;
	}
	bot.structureTime = theLevel.time_ + 6000;
	bot.structure = -1;
	if( !padsBuilt_ )
		buildPads();

	const int team = ent->client_->sess_.sessionTeam_;
	GameEntity* best = nullptr;
	float bestScore = 0;
	for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !other || !other->inuse_ || other->s.eType != ET_EXPLOSIVE || !other->takedamage_ || other->health_ <= 0 )
			continue;
		Vec3 centre;
		for( int k = 0; k < 3; k++ )
			centre[k] = (other->r.absmin[k] + other->r.absmax[k]) * 0.5f;
		float score = Distance( ent->r.currentOrigin, centre );
		if( g_gametype.integer >= GT_TEAM )
		{
			bool own = false, enemy = false;
			for( const Pad& pad : pads_ )
			{
				if( pad.team == ClientBase::TEAM_FREE || Distance( pad.spot, centre ) > 2500 )
					continue;
				if( pad.team == team )
					own = true;
				else
					enemy = true;
			}
			if( own && !enemy )
				continue;
			if( enemy )
				score *= 0.4f;
		}
		if( !best || score < bestScore )
		{
			best = other;
			bestScore = score;
		}
	}
	if( best )
		bot.structure = best->s.number;
	return best;
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
	if( roamPoints_.empty() )
		buildRoamPoints();
	const unsigned int cat = ent->client_->vehicle_ >= 0 ? availableVehicles[ent->client_->vehicle_].cat : 0;

	// Everyone has a rough idea where the enemy is. Part of the time a bot goes looking for the nearest opponent
	// it could fight, the aggressive ones more often, so nobody is left alone for long.
	GameEntity* quarry = nullptr;
	if( !(cat & CAT_LQM) && random() < 0.3f + 0.4f * character( bot ).aggression )
	{
		float nearest = 0;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
				continue;
			// a boat cannot go after someone inland, nor a ground vehicle after someone at sea or in the air
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
	}
	if( quarry )
	{
		constexpr float vagueness = 900.0f;
		VectorCopy( quarry->r.currentOrigin, bot.goal );
		bot.goal[0] += crandom() * vagueness;
		bot.goal[1] += crandom() * vagueness;
		if( !(cat & (CAT_PLANE|CAT_HELO)) )
			bot.goal[2] = ent->r.currentOrigin[2];
	}
	else if( (cat & (CAT_PLANE|CAT_HELO)) && !roamPoints_.empty() && radius > 3500 )
	{
		const Vec3& point = roamPoints_[rand() % roamPoints_.size()];
		point.copyTo( bot.goal );
		bot.goal[0] += crandom() * 800.0f;
		bot.goal[1] += crandom() * 800.0f;
	}
	else
	{
		const float angle = random() * 2.0f * std::numbers::pi_v<float>;
		const float dist = radius * (0.3f + 0.7f * random());
		bot.goal[0] = bot.home[0] + cos( angle ) * dist;
		bot.goal[1] = bot.home[1] + sin( angle ) * dist;
		bot.goal[2] = bot.home[2];
		if( cat & CAT_BOAT )
		{
			for( int tries = 0; tries < 8; tries++ )
			{
				Vec3 probe;
				VectorCopy( bot.goal, probe );
				probe[2] = ent->r.currentOrigin[2] - 6;
				if( GameHost::pointContents( probe, ent->s.number ) & MASK_WATER )
					break;
				const float a = random() * 2.0f * std::numbers::pi_v<float>;
				const float d = radius * (0.2f + 0.8f * random());
				bot.goal[0] = ent->r.currentOrigin[0] + cos( a ) * d;
				bot.goal[1] = ent->r.currentOrigin[1] + sin( a ) * d;
			}
		}
	}
	bot.goalTime = theLevel.time_ + 20000;
}
