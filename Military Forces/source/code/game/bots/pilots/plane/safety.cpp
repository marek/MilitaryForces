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
		Intercept::predict( other, seconds, predicted );
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
// radius followed by a straight; if that does not fit, other manoeuvres are tried in order of preference. The
// manoeuvre chosen is held for a moment.
behavior::Status PlaneTasks::probeTerrain( Mind& c )
{
	// the first moments of a departure were planned on the ground; too low to manoeuvre yet
	if( departing( c ) && c.sense.height < 60.0f )
		return Status::Success;
	if( theLevel.time_ >= c.bot.avoid.probeTime )
		lookAhead( c );
	if( theLevel.time_ < c.bot.avoid.time )
	{
		c.view[YAW] = c.bot.avoid.yaw;
		c.view[PITCH] = c.bot.avoid.pitch;
	}
	return Status::Success;
}

// one look ahead: how tight the terrain is, whether the wanted line is open, and if not which manoeuvre is
void PlaneTasks::lookAhead( Mind& c )
{
	State& bot = c.bot;
	// look more often when something is in the way
	bot.avoid.probeTime = theLevel.time_ + (bot.avoid.confined || bot.avoid.probeBlocked ? 150 : 300) + ThinkRate::stagger( c.ent->s.number, 50 );
	bot.avoid.confined = confinedHere( c );
	const Probe probe = probeFor( c );
	const bool wasBlocked = bot.avoid.probeBlocked;
	checkWantedLine( c, probe );
	if( wasBlocked && !bot.avoid.probeBlocked )
		bot.avoid.time = 0;
	if( !bot.avoid.probeBlocked )
		return;
	Candidates candidates;
	listManoeuvres( c, probe, candidates );
	// with clearance first; if nothing fits, the longest bare line
	for( float width : { probe.margin, 0.0f } )
	{
		const Choice choice = chooseManoeuvre( c, probe, candidates, width );
		if( choice.chosen )
		{
			bot.avoid.yaw = probe.yaw + std::clamp( choice.chosen->turn, -100.0f, 100.0f );
			bot.avoid.pitch = choice.chosen->pitch;
		}
		bot.avoid.probeBest = choice.free;
		if( choice.braked )
			bot.avoid.brakeTime = theLevel.time_ + 600;
		if( choice.free > probe.radius * 1.2f )
			break;
	}
	bot.avoid.time = theLevel.time_ + 400;
}

// terrain tighter than the aircraft's turn: little room to either side, or little between floor and ceiling
bool PlaneTasks::confinedHere( const Mind& c )
{
	constexpr float lookAside = 1200.0f;
	Vec3 flat = { 0, c.ps.vehicleAngles[YAW], 0 }, right, left;
	trace_t tr;
	AngleVectors( flat, nullptr, right, nullptr );
	VectorNegate( right, left );
	BotWorld::pathBlocked( c.ent->r.currentOrigin, left, lookAside, c.ent->s.number, tr );
	const float leftClear = tr.fraction * lookAside;
	BotWorld::pathBlocked( c.ent->r.currentOrigin, right, lookAside, c.ent->s.number, tr );
	const float rightClear = tr.fraction * lookAside;
	return leftClear + rightClear < 1500 || c.room < 900;
}

// what this look ahead measures against: the corridor, the wanted line, how far ahead and the turns at hand
PlaneTasks::Probe PlaneTasks::probeFor( const Mind& c )
{
	Probe probe;
	probe.yaw = c.ps.vehicleAngles[YAW];
	// the corridor looked along is as wide as the aircraft's own box and a little more: a bomber does not fit where
	// a fighter does
	probe.margin = std::max( 60.0f, std::max( c.veh.maxs[0], c.veh.maxs[1] ) * 0.5f + 35.0f );
	probe.wantYaw = c.view[YAW];
	probe.wantPitch = c.view[PITCH];
	probe.wantTurn = AngleSubtract( probe.wantYaw, probe.yaw );
	// near the stall the nose can only come up a little, so plan the climb that can actually be flown
	probe.slow = c.speed < c.veh.stallspeed * 1.7f;
	const float flyablePitch = probe.slow ? std::max( c.view[PITCH], -10.0f ) : c.view[PITCH];
	probe.wantSlope = std::clamp( -static_cast<float>( tan( flyablePitch * std::numbers::pi_v<float> / 180.0 ) ), -0.4f, 0.5f );
	probe.horizon = std::clamp( c.airspeed * 3.0f, 900.0f, 2500.0f );
	probe.radius = c.turnRadius();
	const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	probe.brakedRadius = (c.airspeed + std::min( c.airspeed, c.veh.stallspeed * 1.2f )) * 0.5f / turnRate;
	probe.canBrake = probe.brakedRadius < probe.radius * 0.85f;
	return probe;
}

// the wanted line flown ahead; one that leads into a dead end counts as not quite open
void PlaneTasks::checkWantedLine( Mind& c, const Probe& probe )
{
	State& bot = c.bot;
	Pose wantEnd;
	bot.avoid.probeWant = sweep( c, probe.radius, probe.wantTurn, probe.wantSlope, probe.horizon, probe.margin, &wantEnd );
	if( bot.avoid.probeWant >= probe.horizon && !wayOut( c, wantEnd, probe.brakedRadius, probe.horizon ) )
		bot.avoid.probeWant = probe.horizon * 0.9f;
	bot.avoid.probeBest = bot.avoid.probeWant;
	// a jet spawns fast: slow down at once when there is no room
	if( (bot.avoid.confined || bot.avoid.probeWant < probe.horizon) && theLevel.time_ < bot.spawnTime + 4000 )
		bot.avoid.brakeTime = theLevel.time_ + 600;
	bot.avoid.probeBlocked = bot.avoid.probeWant < probe.horizon;
}

// every turn and climb or dive worth trying, best first, so the search can stop at the first one that is clear. The
// wanted line itself leads: it may still be the best of a bad set. Both sides open, traffic is passed on the right
void PlaneTasks::listManoeuvres( const Mind& c, const Probe& probe, Candidates& out )
{
	constexpr std::array<float, 11> turns = { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f, 135.0f, -135.0f, 180.0f, -180.0f };
	constexpr float leaveWanted = -500.0f, offLevel = 60.0f, keepRight = 15.0f;
	struct Climb { float slope; float pitch; };
	const std::array<Climb, 3> climbs = { { { probe.wantSlope, probe.wantPitch }, { probe.slow ? 0.17f : 0.35f, probe.slow ? -10.0f : -19.0f }, { -0.25f, 14.0f } } };
	const bool canDive = c.sense.height > c.minHeight + 300.0f;
	out.count = 0;
	out.list[out.count++] = { probe.wantTurn, probe.wantSlope, probe.wantPitch, leaveWanted };
	for( float turn : turns )
	{
		for( size_t k = 0; k < climbs.size(); k++ )
		{
			if( k == 2 && !canDive )
				continue;
			const float penalty = fabs( AngleSubtract( probe.yaw + turn, probe.wantYaw ) ) * 2.0f + fabs( turn ) + (k ? offLevel : 0.0f) + (turn > 0 ? keepRight : 0.0f);
			out.list[out.count++] = { turn, climbs[k].slope, climbs[k].pitch, penalty };
		}
	}
	std::sort( out.list.begin(), out.list.begin() + out.count, []( const Candidate& a, const Candidate& b ) { return a.penalty < b.penalty; } );
}

// the best of the candidates at this corridor width: open lines first, the least penalty among them, stopping at
// the first that is fully open
PlaneTasks::Choice PlaneTasks::chooseManoeuvre( const Mind& c, const Probe& probe, const Candidates& candidates, float width )
{
	Choice choice;
	float best = -99999.0f;
	for( size_t i = 0; i < candidates.count; i++ )
	{
		const Candidate& candidate = candidates.list[i];
		const Choice tried = flyCandidate( c, probe, candidate, width );
		const float score = (tried.free < probe.horizon ? tried.free * 2.0f - probe.horizon * 2.0f : probe.horizon) - std::max( candidate.penalty, 0.0f ) - (tried.braked ? 40.0f : 0.0f);
		if( score > best )
		{
			best = score;
			choice = tried;
			choice.chosen = &candidate;
		}
		if( tried.free >= probe.horizon )
			break;
	}
	return choice;
}

// how far one candidate flies clear: as it is, or the same turn with the air brake out at the radius the lower speed
// gives. One that leads into a dead end is better than a wall and worse than any open path
PlaneTasks::Choice PlaneTasks::flyCandidate( const Mind& c, const Probe& probe, const Candidate& candidate, float width )
{
	Choice tried;
	Pose end;
	tried.free = sweep( c, probe.radius, candidate.turn, candidate.slope, probe.horizon, width, &end );
	if( tried.free < probe.horizon && fabs( candidate.turn ) > 10.0f && probe.canBrake )
	{
		Pose slowEnd;
		const float slowFree = sweep( c, probe.brakedRadius, candidate.turn, candidate.slope, probe.horizon, width, &slowEnd );
		if( slowFree > tried.free )
		{
			tried.free = slowFree;
			tried.braked = true;
			end = slowEnd;
		}
	}
	if( tried.free >= probe.horizon && width > 0 && !wayOut( c, end, probe.brakedRadius, probe.horizon ) )
		tried.free = probe.horizon * 0.9f;
	return tried;
}
