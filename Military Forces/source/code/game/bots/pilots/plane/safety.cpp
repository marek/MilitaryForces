#include "plane.h"

// can the aircraft fly a turn of that radius through that many degrees, climbing or diving at that slope, without
// meeting the world? The arc is checked in 30 degree pieces
bool Bots::turnClear( GameEntity* ent, float radius, float degrees, float climb )
{
	const playerState_t& ps = ent->client_->ps_;
	vec3_t flat = { 0, ps.vehicleAngles[YAW], 0 }, forward, right, from;
	AngleVectors( flat, forward, right, nullptr );
	const float side = degrees > 0 ? -1.0f : 1.0f;
	const float total = std::min( static_cast<float>( fabs( degrees ) ), 210.0f );
	VectorCopy( ent->r.currentOrigin, from );
	for( float swept = 30.0f; swept <= total + 29.0f; swept += 30.0f )
	{
		const float angle = std::min( swept, total ) * std::numbers::pi_v<float> / 180.0f;
		Vec3 point, dir;
		VectorMA( ent->r.currentOrigin, radius * sin( angle ), forward, point );
		VectorMA( point, side * radius * (1.0f - cos( angle )), right, point );
		point[2] += climb * radius * angle;
		VectorSubtract( point, from, dir );
		const float length = VectorNormalize( dir );
		trace_t tr;
		if( length > 1 && BotWorld::pathBlocked( from, dir, length, ent->s.number, tr, 50.0f ) && tr.entityNum == ENTITYNUM_WORLD && !tr.startsolid )
			return false;
		VectorCopy( point, from );
	}
	return true;
}

// where another aircraft will be: straight on, or round its turn when it is banked hard
void PlaneTasks::predictAircraft( GameEntity* other, float seconds, vec3_t out )
{
	const playerState_t& ps = other->client_->ps_;
	const float roll = ps.vehicleAngles[ROLL];
	const float rate = std::max( 10.0f, availableVehicles[other->client_->vehicle_].turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	const float speed = sqrtf( ps.velocity[0] * ps.velocity[0] + ps.velocity[1] * ps.velocity[1] );
	if( !(availableVehicles[other->client_->vehicle_].cat & CAT_PLANE) || fabs( roll ) < 45.0f || speed < 1.0f )
	{
		VectorMA( other->r.currentOrigin, seconds, ps.velocity, out );
		return;
	}
	const float radius = speed / rate;
	const float angle = rate * seconds;
	const float side = roll > 0 ? 1.0f : -1.0f;
	const float forward[2] = { ps.velocity[0] / speed, ps.velocity[1] / speed };
	const float right[2] = { forward[1], -forward[0] };
	out[0] = other->r.currentOrigin[0] + radius * (sin( angle ) * forward[0] + side * (1.0f - cos( angle )) * right[0]);
	out[1] = other->r.currentOrigin[1] + radius * (sin( angle ) * forward[1] + side * (1.0f - cos( angle )) * right[1]);
	out[2] = other->r.currentOrigin[2] + ps.velocity[2] * seconds;
}

// true when another aircraft is predicted to be near this point at that time
bool PlaneTasks::trafficAt( const Mind& c, const vec3_t point, float seconds )
{
	constexpr float separation = 220.0f;
	constexpr float collisionRange = 70.0f;
	constexpr float lookAhead = 3.0f;
	if( seconds > lookAhead )
		return false;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		if( !(availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) || (other->client_->ps_.ONOFF & OO_LANDED) )
			continue;
		// right of way: the bot with the higher number gives way, unless it would run into the other from behind;
		// head on there is no time for one to do it alone, so both break
		if( Bots::isBot( i ) && i > c.ent->s.number )
		{
			constexpr float sameWay = 0.5f, headOn = -0.7f;
			Vec3 mine, theirs;
			VectorNormalize2( c.ps.velocity, mine );
			VectorNormalize2( other->client_->ps_.velocity, theirs );
			const float agreement = DotProduct( mine, theirs );
			if( agreement < sameWay && agreement > headOn )
				continue;
		}
		Vec3 predicted;
		predictAircraft( other, seconds, predicted );
		// flying alongside is fine: only a path that closes on the other aircraft is blocked
		const float now = Distance( other->r.currentOrigin, c.ent->r.currentOrigin );
		const float then = Distance( predicted, point );
		if( then < collisionRange || (then < separation && then < now * 0.85f) )
			return true;
	}
	return false;
}

// flies an arc of turnDegrees at the given radius and then straight on, returns the free distance up to horizon
float PlaneTasks::sweepFrom( const Mind& c, const Pose& start, float radius, float turnDegrees, float slope, float horizon, float halfWidth, Pose* finish )
{
	Vec3 flat = { 0, start.yaw, 0 }, forward, right, from;
	AngleVectors( flat, forward, right, nullptr );
	const float side = turnDegrees > 0 ? -1.0f : 1.0f;
	const float total = fabs( turnDegrees );
	const float airspeed = std::max( c.airspeed, 1.0f );
	float travelled = 0;
	VectorCopy( start.origin, from );
	const auto leg = [&]( const vec3_t to ) -> bool
	{
		Vec3 dir;
		trace_t tr;
		VectorSubtract( to, from, dir );
		const float length = VectorNormalize( dir );
		if( length < 1 )
			return true;
		// no trace needed while the leg stays inside one empty cell of the air navigation tree
		if( !AirNav::segmentKnownFree( Vec3( from ), Vec3( to ), halfWidth + 8.0f ) )
		{
			BotWorld::pathBlocked( from, dir, length, c.ent->s.number, tr, halfWidth );
			// already within the corridor's width of a wall: look along a narrower one, and only then along a bare
			// line, so that hugging the wall does not make every heading look open
			if( tr.startsolid && travelled <= 0 && start.seconds <= 0 )
				BotWorld::pathBlocked( from, dir, length, c.ent->s.number, tr, halfWidth * 0.4f );
			if( tr.startsolid && travelled <= 0 && start.seconds <= 0 )
				BotWorld::pathBlocked( from, dir, length, c.ent->s.number, tr );
			const bool vehicle = tr.entityNum >= 1 && tr.entityNum <= theLevel.maxclients_;
			if( tr.startsolid )
				tr.fraction = 0;
			if( tr.fraction < 1.0f && !vehicle )
			{
				travelled += tr.fraction * length;
				return false;
			}
		}
		// look for traffic at points along the leg, not only at its end
		constexpr float trafficStep = 150.0f;
		const int steps = std::max( 1, static_cast<int>( ceil( length / trafficStep ) ) );
		for( int step = 1; step <= steps; step++ )
		{
			const float along = length * step / steps;
			Vec3 point;
			VectorMA( from, along, dir, point );
			if( trafficAt( c, point, start.seconds + (travelled + along) / airspeed ) )
			{
				travelled += along - length / steps;
				return false;
			}
		}
		travelled += length;
		VectorCopy( to, from );
		return true;
	};
	for( float swept = std::min( 30.0f, total ); swept > 0 && travelled < horizon; swept += 30.0f )
	{
		const float angle = std::min( swept, total ) * std::numbers::pi_v<float> / 180.0f;
		Vec3 point;
		VectorMA( start.origin, radius * sin( angle ), forward, point );
		VectorMA( point, side * radius * (1.0f - cos( angle )), right, point );
		point[2] += slope * radius * angle;
		if( !leg( point ) )
			return travelled;
		if( swept >= total )
			break;
	}
	if( travelled < horizon )
	{
		Vec3 out = { 0, start.yaw + turnDegrees, 0 }, dir, end;
		AngleVectors( out, dir, nullptr, nullptr );
		dir[2] = slope;
		end = from + dir * ( horizon - travelled );
		leg( end );
	}
	if( finish )
	{
		VectorCopy( from, finish->origin );
		finish->yaw = start.yaw + turnDegrees;
		finish->seconds = start.seconds + travelled / airspeed;
	}
	return std::min( travelled, horizon );
}

// how far the aircraft gets from where it is now along a turn of that radius and slope before meeting something
float PlaneTasks::sweep( const Mind& c, float radius, float turnDegrees, float slope, float horizon, float halfWidth, Pose* finish )
{
	Pose start;
	VectorCopy( c.ent->r.currentOrigin, start.origin );
	start.yaw = c.ps.vehicleAngles[YAW];
	start.seconds = 0;
	return sweepFrom( c, start, radius, turnDegrees, slope, horizon, halfWidth, finish );
}

// a manoeuvre is only worth flying if there is a way on from where it ends: straight ahead, over the top, or a turn back
bool PlaneTasks::wayOut( const Mind& c, const Pose& end, float brakedRadius, float horizon )
{
	if( sweepFrom( c, end, brakedRadius, 0.0f, 0.0f, horizon, 60.0f ) >= horizon )
		return true;
	if( sweepFrom( c, end, brakedRadius, 0.0f, 0.17f, horizon, 60.0f ) >= horizon )
		return true;
	// follow a bend, turn off, or turn back
	for( float turn : { 45.0f, -45.0f, 90.0f, -90.0f, 180.0f, -180.0f } )
	{
		const float length = brakedRadius * fabs( turn ) * std::numbers::pi_v<float> / 180.0f + (fabs( turn ) < 100.0f ? horizon * 0.6f : 200.0f);
		if( sweepFrom( c, end, brakedRadius, turn, 0.0f, length, 60.0f ) >= length )
			return true;
	}
	return false;
}

// Plans the next second or two of flight. The heading the pilot wants is flown ahead as an arc at the aircraft's turn
// radius followed by a straight; if that does not fit, other manoeuvres are tried in order of preference.
behavior::Status PlaneTasks::probeTerrain( Mind& c )
{
	State& bot = c.bot;
	GameEntity* ent = c.ent;
	const float yaw = c.ps.vehicleAngles[YAW];
	// the first moments of a departure were planned on the ground; too low to manoeuvre yet
	if( departing( c ) && c.sense.height < 60.0f )
		return Status::Success;
	if( theLevel.time_ >= bot.avoid.probeTime )
	{
		// look more often when something is in the way
		bot.avoid.probeTime = theLevel.time_ + (bot.avoid.confined || bot.avoid.probeBlocked ? 150 : 300);
		Vec3 flat = { 0, yaw, 0 }, right, left;
		trace_t tr;
		AngleVectors( flat, nullptr, right, nullptr );
		VectorNegate( right, left );
		BotWorld::pathBlocked( ent->r.currentOrigin, left, 1200, ent->s.number, tr );
		const float leftClear = tr.fraction * 1200.0f;
		BotWorld::pathBlocked( ent->r.currentOrigin, right, 1200, ent->s.number, tr );
		const float rightClear = tr.fraction * 1200.0f;
		bot.avoid.confined = leftClear + rightClear < 1500 || c.room < 900;

		// the corridor looked along is as wide as the aircraft's own box and a little more: a bomber does not fit
		// where a fighter does
		const float margin = std::max( 60.0f, std::max( c.veh.maxs[0], c.veh.maxs[1] ) * 0.5f + 35.0f );
		const float wantYaw = c.view[YAW];
		const float wantTurn = AngleSubtract( wantYaw, yaw );
		// near the stall the nose can only come up a little, so plan the climb that can actually be flown
		const bool slow = c.speed < c.veh.stallspeed * 1.7f;
		const float flyablePitch = slow ? std::max( c.view[PITCH], -10.0f ) : c.view[PITCH];
		const float wantSlope = std::clamp( -static_cast<float>( tan( flyablePitch * std::numbers::pi_v<float> / 180.0 ) ), -0.4f, 0.5f );
		const float horizon = std::clamp( c.airspeed * 3.0f, 900.0f, 2500.0f );
		const bool wasBlocked = bot.avoid.probeBlocked;
		const float radius = c.turnRadius();
		const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
		const float brakedRadius = (c.airspeed + std::min( c.airspeed, c.veh.stallspeed * 1.2f )) * 0.5f / turnRate;
		const bool canBrake = brakedRadius < radius * 0.85f;

		Pose wantEnd;
		bot.avoid.probeWant = sweep( c, radius, wantTurn, wantSlope, horizon, margin, &wantEnd );
		if( bot.avoid.probeWant >= horizon && !wayOut( c, wantEnd, brakedRadius, horizon ) )
			bot.avoid.probeWant = horizon * 0.9f;
		bot.avoid.probeBest = bot.avoid.probeWant;
		// a jet spawns fast: slow down at once when there is no room
		if( (bot.avoid.confined || bot.avoid.probeWant < horizon) && theLevel.time_ < bot.spawnTime + 4000 )
			bot.avoid.brakeTime = theLevel.time_ + 600;
		bot.avoid.probeBlocked = bot.avoid.probeWant < horizon;
		if( wasBlocked && !bot.avoid.probeBlocked )
			bot.avoid.time = 0;
		if( bot.avoid.probeBlocked )
		{
			constexpr std::array<float, 11> turns = { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f, 135.0f, -135.0f, 180.0f, -180.0f };
			struct Climb { float slope; float pitch; };
			const std::array<Climb, 3> climbs = { { { wantSlope, c.view[PITCH] }, { slow ? 0.17f : 0.35f, slow ? -10.0f : -19.0f }, { -0.25f, 14.0f } } };
			const bool canDive = c.sense.height > c.minHeight + 300.0f;

			// candidates in order of preference, so the search can stop at the first one that is clear
			struct Candidate { float turn; float slope; float pitch; float penalty; };
			std::array<Candidate, turns.size() * climbs.size() + 1> candidates;
			size_t count = 0;
			// the wanted heading itself comes first: it may still be the best of a bad set
			candidates[count++] = { wantTurn, wantSlope, c.view[PITCH], -500.0f };
			for( float turn : turns )
			{
				for( size_t k = 0; k < climbs.size(); k++ )
				{
					if( k == 2 && !canDive )
						continue;
					float penalty = fabs( AngleSubtract( yaw + turn, wantYaw ) ) * 2.0f + fabs( turn ) + (k ? 60.0f : 0.0f);
					// keep right: when both sides are open, pass traffic on the right
					if( turn > 0 )
						penalty += 15.0f;
					candidates[count++] = { turn, climbs[k].slope, climbs[k].pitch, penalty };
				}
			}
			std::sort( candidates.begin(), candidates.begin() + count, []( const Candidate& a, const Candidate& b ) { return a.penalty < b.penalty; } );

			// with clearance first; if nothing fits, the longest bare line
			for( float width : { margin, 0.0f } )
			{
				float best = -99999.0f, bestFree = 0;
				bool bestBraked = false;
				const Candidate* chosen = nullptr;
				for( size_t i = 0; i < count; i++ )
				{
					const Candidate& candidate = candidates[i];
					Pose end;
					float free = sweep( c, radius, candidate.turn, candidate.slope, horizon, width, &end );
					bool braked = false;
					// the same turn with the air brake out, at the radius the lower speed gives
					if( free < horizon && fabs( candidate.turn ) > 10.0f && canBrake )
					{
						Pose slowEnd;
						const float slowFree = sweep( c, brakedRadius, candidate.turn, candidate.slope, horizon, width, &slowEnd );
						if( slowFree > free )
						{
							free = slowFree;
							braked = true;
							end = slowEnd;
						}
					}
					// leads into a dead end: better than a wall, worse than any open path
					if( free >= horizon && width > 0 && !wayOut( c, end, brakedRadius, horizon ) )
						free = horizon * 0.9f;
					const float score = (free < horizon ? free * 2.0f - horizon * 2.0f : horizon) - std::max( candidate.penalty, 0.0f ) - (braked ? 40.0f : 0.0f);
					if( score > best )
					{
						best = score;
						bestFree = free;
						bestBraked = braked;
						chosen = &candidate;
					}
					if( free >= horizon )
						break;
				}
				if( chosen )
				{
					bot.avoid.yaw = yaw + std::clamp( chosen->turn, -100.0f, 100.0f );
					bot.avoid.pitch = chosen->pitch;
				}
				bot.avoid.probeBest = bestFree;
				if( bestBraked )
					bot.avoid.brakeTime = theLevel.time_ + 600;
				if( bestFree > radius * 1.2f )
					break;
			}
			bot.avoid.time = theLevel.time_ + 400;
		}
	}
	if( theLevel.time_ < bot.avoid.time )
	{
		c.view[YAW] = bot.avoid.yaw;
		c.view[PITCH] = bot.avoid.pitch;
	}
	return Status::Success;
}
