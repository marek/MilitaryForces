#include "plane.h"
#include <array>
#include <limits>
#include <numbers>

namespace
{
	// room left between two bodies that pass each other on the ground
	constexpr float passingRoom = 16.0f;
	// another body within this much height is on the ground with this one
	constexpr float sameLevel = 120.0f;
	// clear ground wanted ahead of the nose while taxiing
	constexpr float taxiRoom = 60.0f;
	// seconds ahead that traffic is foreseen: while taxiing, and before and during the start of the roll
	constexpr float taxiForesight = 2.0f, rollForesight = 4.0f;
	constexpr float foresightStep = 0.5f;
	constexpr float standingSpeed = 5.0f;
	// waiting this long on traffic, the departure is planned again from where the aircraft stands
	constexpr int patience = 4000;
	// with nowhere to roll from or taxi to, it looks again after this
	constexpr int holdShortFor = 2000;
	// a departure planned is begun within this, or planned again
	constexpr int planLasts = 30000;
	// past the lift-off point the climb is still low enough to hit anything standing on the runway
	constexpr float climbOverTraffic = 450.0f;
	// lined up: the nose this close to the planned heading
	constexpr float lineUpTolerance = 6.0f;
	// an aircraft ahead that faces about the same way is in the queue for the same run
	constexpr float sameWay = 45.0f;
	// how long a run looks ahead, from where a heading is judged
	constexpr float longestLook = 4000.0f;

	// sampling a body's way along a straight line: finer than any body is long
	constexpr float sweepStep = 20.0f;

	// clear ground kept round the aircraft when it creeps round to a new heading, nearly on the spot
	constexpr float turningRoom = 4.0f;
	// a crawl for turning on the ground, and the speed the turn's radius is reckoned from, allowing for overshoot
	constexpr float creepSpeed = 8.0f, creepReckoned = 12.0f;
	constexpr float arcStep = 15.0f;
	// beyond this from the start of a way nobody is in it, at the speeds foreseen
	constexpr float farEnough = 300.0f;
	// the most points a way is sampled at; a straight takes no more than half
	constexpr size_t pathPoints = 96;

	bool standing( const GameEntity* ent )
	{
		return VectorLength( ent->client_->ps_.velocity ) < standingSpeed;
	}

	// a body that can be run into: a player in a vehicle or on foot, not riding in someone else's
	bool solidBody( const GameEntity* self, GameEntity* other )
	{
		return other != self && alive( other ) && other->client_->sess_.ridingClient_ < 0;
	}

	// an aircraft on the ground and still, as one stands while waiting for its turn to roll
	bool waitingAircraft( GameEntity* ent )
	{
		return (availableVehicles[ent->client_->vehicle_].cat & CAT_PLANE) && (ent->client_->ps_.ONOFF & OO_LANDED) && standing( ent );
	}

	// how far apart two bodies are over the ground: the larger of the gaps between their boxes along the world's
	// axes, below zero when they overlap. Bodies collide as the boxes the game gives them, which keep to the world's
	// axes whichever way the body faces
	float separation( const Vec3& a, const vec3_t aMins, const vec3_t aMaxs, const Vec3& b, const vec3_t bMins, const vec3_t bMaxs )
	{
		float gap = -std::numeric_limits<float>::max();
		for( int k = 0; k < 2; k++ )
			gap = std::max( { gap, b[k] + bMins[k] - (a[k] + aMaxs[k]), a[k] + aMins[k] - (b[k] + bMaxs[k]) } );
		return gap;
	}

	// The points an aircraft's origin passes through on a way it means to go on the ground, each with the room it
	// keeps there, sampled finer than any body is long
	class GroundPath
	{
	public:
		explicit GroundPath( const Vec3& start ) { add( start, 0.0f ); }

		// straight on from the last point
		void straight( float yaw, float length )
		{
			const Vec3 flat = { 0, yaw, 0 };
			Vec3 dir;
			AngleVectors( flat, dir, nullptr, nullptr );
			const Vec3 from = points_[count_ - 1];
			const float step = std::max( sweepStep, length / static_cast<float>( pathPoints / 2 ) );
			for( float d = step; d < length + step; d += step )
				add( from + dir * std::min( d, length ), passingRoom );
			length_ += length;
		}

		// round at a crawl from one heading to another, the short way, about a circle of that radius
		void arc( float fromYaw, float toYaw, float radius )
		{
			const float turn = AngleSubtract( toYaw, fromYaw );
			const float side = turn >= 0.0f ? 1.0f : -1.0f;
			const float rad = std::numbers::pi_v<float> / 180.0f;
			const Vec3 start = points_[count_ - 1];
			// the centre of the turn is to the inside, abeam the start
			const Vec3 centre = start + Vec3( -sin( fromYaw * rad ), cos( fromYaw * rad ), 0.0f ) * (radius * side);
			for( float swung = arcStep; swung < fabs( turn ) + arcStep; swung += arcStep )
			{
				const float heading = (fromYaw + side * std::min( swung, fabs( turn ) )) * rad;
				add( centre + Vec3( sin( heading ), -cos( heading ), 0.0f ) * (radius * side), turningRoom );
			}
			length_ += fabs( turn ) * rad * radius;
		}

		// The first player with a body this way brings closer than the room kept, now or where it will be within the
		// seconds foreseen at the speed and direction it is going. Someone already that close and not neared further -
		// beside, or behind - does not count: going on is how the aircraft gets away from them
		const GameEntity* meets( const GameEntity* self, float seconds ) const
		{
			for( int i = 1; i <= theLevel.maxclients_; i++ )
			{
				GameEntity* other = theLevel.getEntity( i );
				if( solidBody( self, other ) && closesOn( self, other, seconds ) )
					return other;
			}
			return nullptr;
		}

		// the other, where it is going, comes nearer than the room this way needs at some point along it
		bool closesOn( const GameEntity* self, const GameEntity* other, float seconds ) const
		{
			const Vec3 at( other->r.currentOrigin ), velocity( other->client_->ps_.velocity );
			if( (at - points_[0]).length() > length_ + farEnough + velocity.length() * seconds )
				return false;
			for( float t = 0.0f; t <= seconds; t += foresightStep )
			{
				const Vec3 there = at + velocity * t;
				const float before = separation( points_[0], self->r.mins, self->r.maxs, there, other->r.mins, other->r.maxs );
				for( size_t k = 1; k < count_; k++ )
				{
					const float gap = separation( points_[k], self->r.mins, self->r.maxs, there, other->r.mins, other->r.maxs );
					if( gap < rooms_[k] && gap < before && fabs( points_[k][2] - there[2] ) < sameLevel )
						return true;
				}
			}
			return false;
		}

	private:
		void add( const Vec3& point, float room )
		{
			if( count_ == points_.size() )
				return;
			points_[count_] = point;
			rooms_[count_++] = room;
		}

		std::array<Vec3, pathPoints>	points_;
		std::array<float, pathPoints>	rooms_ = {};
		size_t					count_ = 0;
		float					length_ = 0.0f;
	};
}

// The ground the plane rolls on before its wheels leave it: the run to the stall with the nose coming up, and a
// margin, since the estimate is rough
float PlaneTasks::groundRun( const completeVehicleData_t& veh )
{
	constexpr float groundAcceleration = 85.0f, margin = 1.15f;
	const float rotate = static_cast<float>( veh.stallspeed ) * rotateSpeed;
	return rotate * rotate / (2.0f * groundAcceleration) * margin;
}

// the run is short and a runway end unforgiving: a comfortable length is half as much again
float PlaneTasks::comfortableRun( const completeVehicleData_t& veh )
{
	return groundRun( veh ) * 1.5f + 150.0f;
}

// Whoever is in the way of a run along a heading, now or within the seconds foreseen. Any player with a body
// counts - an aircraft, a vehicle crossing, a soldier - since a plane that touches anything is lost
const GameEntity* PlaneTasks::trafficOnWay( const Mind& c, const Vec3& from, float yaw, float length, float seconds )
{
	GroundPath way( from );
	way.straight( yaw, length );
	return way.meets( c.ent, seconds );
}

// whoever is in the way of creeping round from the present heading to another, and of going on along it for a while
const GameEntity* PlaneTasks::turnInTheWay( const Mind& c, float yaw, float onward, float seconds )
{
	const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	GroundPath way( Vec3( c.ent->r.currentOrigin ) );
	way.arc( c.ps.vehicleAngles[YAW], yaw, creepReckoned / turnRate );
	if( onward > 0.0f )
		way.straight( yaw, onward );
	return way.meets( c.ent, seconds );
}

// a spot this aircraft can stand on without touching anyone
bool PlaneTasks::spotFree( const Mind& c, const Vec3& spot )
{
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( solidBody( c.ent, other ) && fabs( other->r.currentOrigin[2] - spot[2] ) < sameLevel &&
			separation( spot, c.ent->r.mins, c.ent->r.maxs, Vec3( other->r.currentOrigin ), other->r.mins, other->r.maxs ) < passingRoom )
			return false;
	}
	return true;
}

// A spot in front of an aircraft that stands waiting to roll is where it will roll: nobody taxis into the queue
// ahead of the one at its head
bool PlaneTasks::inSomeonesRun( const Mind& c, const Vec3& spot )
{
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !solidBody( c.ent, other ) || !waitingAircraft( other ) || fabs( other->r.currentOrigin[2] - spot[2] ) >= sameLevel )
			continue;
		const Vec3 flat = { 0, other->client_->ps_.vehicleAngles[YAW], 0 };
		Vec3 dir;
		AngleVectors( flat, dir, nullptr, nullptr );
		const Vec3 from( other->r.currentOrigin );
		const float run = groundRun( availableVehicles[other->client_->vehicle_] ) + climbOverTraffic;
		const float before = separation( from, other->r.mins, other->r.maxs, spot, c.ent->r.mins, c.ent->r.maxs );
		for( float d = sweepStep; d < run + sweepStep; d += sweepStep )
		{
			const float gap = separation( from + dir * std::min( d, run ), other->r.mins, other->r.maxs, spot, c.ent->r.mins, c.ent->r.maxs );
			if( gap < passingRoom && gap < before )
				return true;
		}
	}
	return false;
}

// Off the runway a plane taxis straight over the ground to it: nothing standing in the way, and no step up or down
// that would lift the wheels off and drop the aircraft at walking pace
bool PlaneTasks::groundTaxiable( const Mind& c, const Vec3& from, const Vec3& to )
{
	constexpr float sample = 25.0f, maxStep = 6.0f, above = 60.0f, below = 140.0f, clearOfGround = 16.0f;
	Vec3 way = to - from;
	way[2] = 0;
	const float dist = way.length();
	if( dist < 1.0f )
		return true;
	way = way / dist;
	trace_t tr;
	Vec3 start = from, end = to;
	start[2] += clearOfGround;
	end[2] = start[2];
	Vec3 mins( c.veh.mins ), maxs( c.veh.maxs );
	mins[2] = 0;
	GameHost::trace( &tr, start, mins, maxs, end, c.ent->s.number, MASK_PLAYERSOLID & ~CONTENTS_BODY, false );
	if( tr.startsolid || tr.fraction < 1.0f )
		return false;
	float previous = from[2];
	for( float d = sample; d < dist + sample; d += sample )
	{
		Vec3 point = from + way * std::min( d, dist );
		Vec3 top = point, bottom = point;
		top[2] = previous + above;
		bottom[2] = previous - below;
		GameHost::trace( &tr, top, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID|MASK_WATER, false );
		if( tr.fraction >= 1.0f || (tr.contents & MASK_WATER) || fabs( tr.endpos[2] - previous ) > maxStep )
			return false;
		previous = tr.endpos[2];
	}
	return true;
}

// Where to taxi to when the run from here will not do: the nearest spot with enough runway that is reached
// straight along the runway (or, from off it, over level ground), not on its very edge where the aircraft has to
// stop and turn, free of anyone standing there or in the way to it, and not ahead of an aircraft waiting to roll
bool PlaneTasks::findTaxiSpot( Mind& c, const vec3_t origin, float wanted )
{
	constexpr float taxiStep = 100.0f, taxiMax = 800.0f, edgeRoom = 50.0f;
	const Vec3 from( origin );
	const bool onRunway = BotWorld::runwayAt( origin );
	PlaceQuery query;
	query.centre = from;
	query.firstRing = taxiStep;
	query.ringStep = taxiStep;
	query.rings = static_cast<int>( taxiMax / taxiStep );
	query.around = 24;
	query.onGround = false;
	query.filters.push_back( []( const Vec3& spot ) { return BotWorld::runwayAt( spot ); } );
	query.filters.push_back( [&c, from, onRunway]( const Vec3& spot )
	{
		const Vec3 way = spot - from;
		if( !onRunway )
			return groundTaxiable( c, from, spot );
		return BotWorld::runwayRun( from, yawOf( way ), way.length() ) >= way.length() - 0.5f;
	} );
	query.filters.push_back( []( const Vec3& spot )
	{
		for( int side = 0; side < 4; side++ )
		{
			if( BotWorld::runwayRun( spot, side * 90.0f, edgeRoom ) < edgeRoom )
				return false;
		}
		return true;
	} );
	query.filters.push_back( [wanted]( const Vec3& spot ) { return longestRun( spot, wanted ) >= wanted; } );
	query.filters.push_back( [&c]( const Vec3& spot ) { return spotFree( c, spot ) && !inSomeonesRun( c, spot ); } );
	query.filters.push_back( [&c, from]( const Vec3& spot )
	{
		const Vec3 way = spot - from;
		return !trafficOnWay( c, from, yawOf( way ), way.length(), 0.0f );
	} );
	query.scorers.push_back( [wanted]( const Vec3& spot ) { return longestRun( spot, wanted ); } );
	Vec3 spot;
	if( !query.find( spot ) )
		return false;
	spot.copyTo( c.bot.takeoff.taxiSpot );
	c.bot.takeoff.taxiTime = theLevel.time_ + 20000;
	c.bot.takeoff.planTime = 0;
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 )
		Common::printf( "BOTTAKEOFF t %d bot %d taxi to %.0f %.0f run %.0f\n", theLevel.time_, c.ent->s.number, spot[0], spot[1], longestRun( spot, wanted ) );
#endif
	return true;
}

// The longest run from a spot in any of twelve headings, looked for a little beyond the length wanted. A run counts
// only as far as it also goes from either side of the spot: turning to line up at a crawl moves the aircraft
// sideways a little, and a spot on the edge of a strip leaves it rolling beside it
float PlaneTasks::longestRun( const Vec3& spot, float wanted )
{
	constexpr float lineUpDrift = 40.0f;
	float best = 0.0f;
	for( int turn = 0; turn < 12; turn++ )
	{
		const float yaw = turn * 30.0f;
		float run = BotWorld::runwayRun( spot, yaw, wanted + 25.0f );
		const Vec3 flat = { 0, yaw, 0 };
		Vec3 right;
		AngleVectors( flat, nullptr, right, nullptr );
		for( float side : { -lineUpDrift, lineUpDrift } )
		{
			if( run > best )
				run = std::min( run, BotWorld::runwayRun( spot + right * side, yaw, wanted + 25.0f ) );
		}
		best = std::max( best, run );
	}
	return best;
}

// the heading of a direction over the ground
float PlaneTasks::yawOf( const Vec3& dir )
{
	Vec3 angles;
	vectoangles( dir, angles );
	return angles[YAW];
}

// Chooses the take-off heading: enough runway to reach flying speed, nothing standing on it, and from the lift-off
// point a climb, straight or turning, that fits the terrain and leads somewhere. A run with an aircraft ahead that
// faces the same way is still taken: that one goes first and this one waits behind it
void PlaneTasks::planDeparture( Mind& c )
{
	Vec3 origin;
	VectorCopy( c.ent->r.currentOrigin, origin );
	origin[2] -= c.sense.height;
	c.bot.takeoff.holdTime = 0;
	if( !BotWorld::runwayAt( origin ) )
	{
		offRunway( c, origin );
		return;
	}
	const Roll roll = rollFor( c );
	Heading best;
	float longest = 0;
	for( int step = 0; step < 24; step++ )
	{
		const Heading heading = scoreHeading( c, origin, step * 15.0f, roll );
		longest = std::max( longest, heading.run );
		if( heading.score > best.score )
			best = heading;
	}
	if( best.score > 0 )
	{
		c.bot.takeoff.yaw = best.yaw;
		c.bot.takeoff.departYaw = best.yaw + best.departTurn;
	}
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 )
		Common::printf( "BOTTAKEOFF t %d bot %d veh %d at %.0f %.0f %.0f facing %.0f chose %.0f then %.0f run %.0f need %.0f longest %.0f score %.0f\n", theLevel.time_, c.ent->s.number,
			c.client->vehicle_, origin[0], origin[1], origin[2], c.ps.vehicleAngles[YAW], c.bot.takeoff.yaw, c.bot.takeoff.departYaw, best.run, roll.groundRun, longest, best.score );
#endif
	settleDeparture( c, origin, best, roll );
}

// on the grass beside a runway: back onto it first
void PlaneTasks::offRunway( Mind& c, const Vec3& origin )
{
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 )
		Common::printf( "BOTTAKEOFF t %d bot %d not on a runway at %.0f %.0f %.0f height %.0f\n", theLevel.time_, c.ent->s.number, origin[0], origin[1], origin[2], c.sense.height );
#endif
	vec3_t from;
	origin.copyTo( from );
	if( !findTaxiSpot( c, from, comfortableRun( c.veh ) ) )
		c.bot.takeoff.holdTime = theLevel.time_ + holdShortFor;
}

// what this aircraft needs from a runway: its ground run, its turn radius just after lift-off, and a comfortable
// length
PlaneTasks::Roll PlaneTasks::rollFor( const Mind& c )
{
	const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	Roll roll;
	roll.groundRun = groundRun( c.veh );
	roll.radius = static_cast<float>( c.veh.stallspeed ) * 1.15f / turnRate;
	roll.comfortable = comfortableRun( c.veh );
	return roll;
}

// One heading's worth: enough run, a clear roll, a climb out that fits, of two long enough runs the one with nobody
// on it, and more runway up to twice a comfortable length. The credit does not stop at the bare need: then the first
// short heading tied with the long one, was chosen, was found too short to be comfortable, and the aircraft taxied
// about looking for a better place
PlaneTasks::Heading PlaneTasks::scoreHeading( const Mind& c, const Vec3& origin, float yaw, const Roll& roll )
{
	Heading h;
	h.yaw = yaw;
	h.run = BotWorld::runwayRun( origin, yaw, longestLook );
	if( h.run < 150 )
		return h;
	Vec3 level = { 0, yaw, 0 }, dir;
	AngleVectors( level, dir, nullptr, nullptr );
	h.rollClear = rollIsClear( c, origin, dir, std::min( h.run, roll.groundRun + 100.0f ) );
	const bool noTraffic = !trafficOnWay( c, origin, yaw, std::min( h.run, roll.groundRun + climbOverTraffic ), taxiForesight );
	Pose liftOff;
	VectorMA( origin, std::min( h.run, roll.groundRun ), dir, liftOff.origin );
	liftOff.origin[2] += 30;
	liftOff.yaw = yaw;
	liftOff.seconds = 3.0f;
	const bool departClear = climbOutTurn( c, liftOff, roll.radius, h.departTurn );
	h.score = (h.run >= roll.groundRun ? 5000.0f : 0.0f) + (h.rollClear ? 3000.0f : 0.0f) + (departClear ? 4000.0f - fabs( h.departTurn ) * 5.0f : 0.0f) +
		(noTraffic && h.run >= roll.comfortable ? 2000.0f : 0.0f) + std::min( h.run, roll.comfortable * 2.0f ) * 2.0f;
	return h;
}

// nothing of the world on the take-off roll; people on it come and go and are judged apart
bool PlaneTasks::rollIsClear( const Mind& c, const Vec3& origin, const Vec3& dir, float length )
{
	Vec3 start, end, boxMins = { -30, -30, -12 }, boxMaxs = { 30, 30, 12 };
	VectorCopy( origin, start );
	start[2] += 14;
	VectorMA( start, length, dir, end );
	trace_t ground;
	GameHost::trace( &ground, start, boxMins, boxMaxs, end, c.ent->s.number, (MASK_PLAYERSOLID & ~CONTENTS_BODY) | CONTENTS_BOTCLIP, false );
	return ground.startsolid || ground.fraction >= 1.0f;
}

// the gentlest turn after lift-off whose climb fits the terrain and leads somewhere
bool PlaneTasks::climbOutTurn( const Mind& c, const Pose& liftOff, float radius, float& turn )
{
	constexpr float climbSlope = 0.14f, departure = 900.0f;
	for( float option : { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f } )
	{
		Pose end;
		if( sweepFrom( c, liftOff, radius, option, climbSlope, departure, 60.0f, &end ) >= departure && wayOut( c, end, radius, departure ) )
		{
			turn = option;
			return true;
		}
	}
	return false;
}

// Someone standing on the best run who is not in the queue for it - parked across it, or facing another way - is
// gone round if there is a place to go round to; one that is moving, or waiting to roll the same way, goes first.
// With nothing better to taxi to, look again in a moment, and after a while roll the best there is, if it is long
// enough at all
void PlaneTasks::settleDeparture( Mind& c, const Vec3& origin, const Heading& best, const Roll& roll )
{
	constexpr int holdLimit = 8;
	const GameEntity* traffic = best.score > 0 ? trafficOnWay( c, origin, best.yaw, std::min( best.run, roll.groundRun + climbOverTraffic ), 0.0f ) : nullptr;
	const bool queued = traffic && (!standing( traffic ) || fabs( AngleSubtract( traffic->client_->ps_.vehicleAngles[YAW], best.yaw ) ) < sameWay);
	const bool shortRun = best.run < roll.comfortable;
	vec3_t from;
	origin.copyTo( from );
	if( (best.score <= 0 || shortRun || !best.rollClear || (traffic && !queued)) && findTaxiSpot( c, from, roll.comfortable ) )
		return;
	if( best.score <= 0 || ((shortRun || !best.rollClear) && (c.bot.takeoff.holds < holdLimit || best.run < roll.groundRun)) )
	{
		c.bot.takeoff.holds++;
		c.bot.takeoff.holdTime = theLevel.time_ + holdShortFor;
		return;
	}
	c.bot.takeoff.holds = 0;
	c.bot.takeoff.planTime = theLevel.time_ + planLasts;
}

// on the ground the controls are the take-off's; a heading is planned once the aircraft is stopped and nothing holds
// it or sends it taxiing
void PlaneTasks::prepareTakeOff( Mind& c )
{
	c.controls.claim();
	c.bot.takeoff.groundTime = theLevel.time_;
	if( !holdingShort( c ) && !taxiing( c ) && !planned( c ) && c.speed < 20 )
		planDeparture( c );
}

// stopped short of the run, nose where it is
behavior::Status PlaneTasks::holdShort( Mind& c )
{
	c.view[YAW] = c.ps.vehicleAngles[YAW];
	c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
	return Status::Running;
}

// to the spot with enough runway: round at a crawl until it points there, then at walking pace; stopped on it, the
// next plan starts from there
behavior::Status PlaneTasks::taxiToSpot( Mind& c )
{
	constexpr float taxiSpeed = 35.0f, onTheSpot = 40.0f, pointing = 20.0f;
	Vec3 toSpot;
	VectorSubtract( c.bot.takeoff.taxiSpot, c.ent->r.currentOrigin, toSpot );
	toSpot[2] = 0;
	c.headFor( c.bot.takeoff.taxiSpot );
	c.view[PITCH] = 0.0f;
	if( VectorLength( toSpot ) >= onTheSpot )
	{
		const float pace = fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) ) > pointing ? creepSpeed : taxiSpeed;
		c.cmd.forwardmove = c.speed < pace ? moveFull : c.speed > pace * 1.5f ? -moveFull : moveNone;
		return Status::Running;
	}
	c.cmd.forwardmove = -moveFull;
	if( c.speed <= 1 )
		c.bot.takeoff.taxiTime = 0;
	return Status::Running;
}

// Round to the planned heading at a crawl, then along it; the nose comes up with speed in hand, since lifting off
// right at the stall drops the aircraft back. With no plan it stands. Lined up, the run is judged again from where
// the turn left it before the roll starts: off the end or the side of a strip at speed the wheels leave the ground
// below the stall
behavior::Status PlaneTasks::lineUpAndRoll( Mind& c )
{
	constexpr float rollStarted = 30.0f;
	if( !planned( c ) )
		return holdShort( c );
	c.view[YAW] = c.bot.takeoff.yaw;
	c.view[PITCH] = c.speed > c.veh.stallspeed * rotateSpeed ? -8.0f : 0.0f;
	if( !linedUp( c ) )
		c.cmd.forwardmove = c.speed < creepSpeed ? moveFull : -moveFull;
	else if( c.speed < rollStarted )
	{
		Vec3 ground( c.ent->r.currentOrigin );
		ground[2] -= c.sense.height;
		// the run is measured in steps, so it is looked for a step beyond the need
		constexpr float runStep = 25.0f;
		const float need = groundRun( c.veh );
		if( BotWorld::runwayRun( ground, c.bot.takeoff.yaw, need + runStep ) < need )
		{
			c.bot.takeoff.planTime = 0;
			return holdShort( c );
		}
	}
	return Status::Running;
}

// slow on the ground: stop for what is in the way, and keep off another aircraft's take-off roll. Kept waiting
// a while, the departure is planned again from where the aircraft stands
behavior::Status PlaneTasks::giveWay( Mind& c )
{
	const bool waiting = c.speed < c.veh.stallspeed * 0.5f && waitForTraffic( c );
	if( !waiting )
		c.bot.takeoff.heldSince = 0;
	else if( !c.bot.takeoff.heldSince )
		c.bot.takeoff.heldSince = theLevel.time_;
	else if( theLevel.time_ - c.bot.takeoff.heldSince > patience && c.speed < 1 )
	{
		c.bot.takeoff.heldSince = 0;
		c.bot.takeoff.taxiTime = 0;
		c.bot.takeoff.planTime = 0;
	}
	return Status::Running;
}

// True while waiting on traffic: anyone under the nose or about to be, wherever the aircraft is going, and once
// lined up anyone on the run or who will be on it while it rolls. It brakes for one on its take-off roll without
// counting as waiting
bool PlaneTasks::waitForTraffic( Mind& c )
{
	const Vec3 origin( c.ent->r.currentOrigin );
	const float heading = c.ps.vehicleAngles[YAW];
	const GameEntity* ahead = nullptr;
	if( taxiing( c ) )
	{
		Vec3 toSpot = Vec3( c.bot.takeoff.taxiSpot ) - origin;
		toSpot[2] = 0;
		ahead = turnInTheWay( c, yawOf( toSpot ), std::min( taxiRoom, toSpot.length() ), taxiForesight );
	}
	else if( planned( c ) && !linedUp( c ) )
		ahead = turnInTheWay( c, c.bot.takeoff.yaw, 0.0f, taxiForesight );
	else if( planned( c ) )
	{
		Vec3 ground = origin;
		ground[2] -= c.sense.height;
		const float roll = groundRun( c.veh ) + climbOverTraffic;
		ahead = trafficOnWay( c, origin, c.bot.takeoff.yaw, std::max( BotWorld::runwayRun( ground, c.bot.takeoff.yaw, roll ), taxiRoom ), rollForesight );
	}
	else
		ahead = trafficOnWay( c, origin, heading, taxiRoom, taxiForesight );
	if( ahead )
	{
#ifdef MF_BOT_DEBUG
		if( g_botDebug.integer >= 2 && !c.bot.takeoff.heldSince )
			Common::printf( "BOTTAKEOFF t %d bot %d waits for %d at %.0f %.0f %s\n", theLevel.time_, c.ent->s.number, ahead->s.number, ahead->r.currentOrigin[0],
				ahead->r.currentOrigin[1], taxiing( c ) ? "taxiing" : !planned( c ) ? "holding" : linedUp( c ) ? "lined up" : "lining up" );
#endif
		c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
		return true;
	}
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !solidBody( c.ent, other ) )
			continue;
		Vec3 rel = Vec3( other->r.currentOrigin ) - origin;
		const float dist = VectorNormalize( rel );
		if( onTakeOffRollTowards( other, rel, dist ) )
		{
			c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
			return false;
		}
	}
	return false;
}

// another aircraft rolling for take-off with this one in its path ahead, unless already standing on its line
bool PlaneTasks::onTakeOffRollTowards( const GameEntity* other, const Vec3& rel, float dist )
{
	constexpr float rollSpeed = 60.0f, rollAhead = 1500.0f, rollWidth = 250.0f, onTheLine = 60.0f;
	Vec3 track;
	VectorCopy( other->client_->ps_.velocity, track );
	track[2] = 0;
	if( !(other->client_->ps_.ONOFF & OO_LANDED) || VectorNormalize( track ) < rollSpeed )
		return false;
	const float along = -DotProduct( rel, track ) * dist;
	const float aside = sqrt( std::max( 0.0f, dist * dist - along * along ) );
	return along > 0 && along < rollAhead && aside < rollWidth && aside > onTheLine;
}

bool PlaneTasks::linedUp( const Mind& c )
{
	return fabs( AngleSubtract( c.bot.takeoff.yaw, c.ps.vehicleAngles[YAW] ) ) < lineUpTolerance;
}

// follow the departure that was planned on the ground; the manoeuvre check still vets it
behavior::Status PlaneTasks::depart( Mind& c )
{
	c.view[YAW] = c.bot.takeoff.departYaw;
	c.view[PITCH] = -8.0f;
	return Status::Running;
}
