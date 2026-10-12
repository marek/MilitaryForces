#include "plane.h"

// parked where no heading has enough runway: the nearest spot along the taxiway that has, reached straight along the
// runway and not on its very edge, where the aircraft has to stop and turn
bool PlaneTasks::findTaxiSpot( Mind& c, const vec3_t origin, float wanted )
{
	constexpr float taxiStep = 100.0f, taxiMax = 800.0f, edgeRoom = 50.0f;
	const Vec3 from( origin );
	PlaceQuery query;
	query.centre = from;
	query.firstRing = taxiStep;
	query.ringStep = taxiStep;
	query.rings = static_cast<int>( taxiMax / taxiStep );
	query.around = 24;
	query.onGround = false;
	query.filters.push_back( [from]( const Vec3& spot )
	{
		const Vec3 way = spot - from;
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

// the longest run from a spot in any of twelve headings, looked for a little beyond the length wanted
float PlaneTasks::longestRun( const Vec3& spot, float wanted )
{
	float best = 0.0f;
	for( int turn = 0; turn < 12; turn++ )
		best = std::max( best, BotWorld::runwayRun( spot, turn * 30.0f, wanted + 25.0f ) );
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
// point a climb, straight or turning, that fits the terrain and leads somewhere. With less than a comfortable length
// it taxis to a better place; with someone standing on the run it gives them a moment to move
void PlaneTasks::planDeparture( Mind& c )
{
	Vec3 origin;
	VectorCopy( c.ent->r.currentOrigin, origin );
	origin[2] -= c.sense.height;
	if( !BotWorld::runwayAt( origin ) )
	{
#ifdef MF_BOT_DEBUG
		if( g_botDebug.integer >= 2 )
			Common::printf( "BOTTAKEOFF t %d bot %d not on a runway at %.0f %.0f %.0f height %.0f\n", theLevel.time_, c.ent->s.number, origin[0], origin[1], origin[2], c.sense.height );
#endif
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

// what this aircraft needs from a runway: its ground run, its turn radius just after lift-off, and a comfortable
// length, half as much again as the rough estimate since a runway end is unforgiving
PlaneTasks::Roll PlaneTasks::rollFor( const Mind& c )
{
	constexpr float groundAcceleration = 85.0f;
	const float stall = static_cast<float>( c.veh.stallspeed );
	const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	Roll roll;
	roll.groundRun = stall * rotateSpeed * stall * rotateSpeed / (2.0f * groundAcceleration) * 1.15f;
	roll.radius = stall * 1.15f / turnRate;
	roll.comfortable = roll.groundRun * 1.5f + 150.0f;
	return roll;
}

// One heading's worth: enough run, a clear roll, a climb out that fits, and more runway up to twice a comfortable
// length. The credit does not stop at the bare need: then the first short heading tied with the long one, was
// chosen, was found too short to be comfortable, and the aircraft taxied about looking for a better place
PlaneTasks::Heading PlaneTasks::scoreHeading( const Mind& c, const Vec3& origin, float yaw, const Roll& roll )
{
	Heading h;
	h.yaw = yaw;
	h.run = BotWorld::runwayRun( origin, yaw, 4000 );
	if( h.run < 150 )
		return h;
	Vec3 level = { 0, yaw, 0 }, dir;
	AngleVectors( level, dir, nullptr, nullptr );
	h.rollClear = rollIsClear( c, origin, dir, std::min( h.run, roll.groundRun + 100.0f ) );
	Pose liftOff;
	VectorMA( origin, std::min( h.run, roll.groundRun ), dir, liftOff.origin );
	liftOff.origin[2] += 30;
	liftOff.yaw = yaw;
	liftOff.seconds = 3.0f;
	const bool departClear = climbOutTurn( c, liftOff, roll.radius, h.departTurn );
	h.score = (h.run >= roll.groundRun ? 5000.0f : 0.0f) + (h.rollClear ? 3000.0f : 0.0f) + (departClear ? 4000.0f - fabs( h.departTurn ) * 5.0f : 0.0f) +
		std::min( h.run, roll.comfortable * 2.0f ) * 2.0f;
	return h;
}

// nothing standing on the take-off roll
bool PlaneTasks::rollIsClear( const Mind& c, const Vec3& origin, const Vec3& dir, float length )
{
	Vec3 start;
	VectorCopy( origin, start );
	start[2] += 14;
	trace_t ground;
	BotWorld::pathBlocked( start, dir, length, c.ent->s.number, ground, 30.0f );
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

// with less than a comfortable length, taxi to a better place; someone standing on the run: a moment to move
// before rolling, a few times at most; otherwise the plan holds for half a minute
void PlaneTasks::settleDeparture( Mind& c, const Vec3& origin, const Heading& best, const Roll& roll )
{
	constexpr int holdLimit = 8;
	vec3_t from;
	origin.copyTo( from );
	if( best.run < roll.comfortable && findTaxiSpot( c, from, roll.comfortable ) )
		return;
	if( best.score > 0 && !best.rollClear && c.bot.takeoff.holds < holdLimit )
	{
		c.bot.takeoff.holds++;
		c.bot.takeoff.holdTime = theLevel.time_ + 1000;
		return;
	}
	if( best.score > 0 )
		c.bot.takeoff.planTime = theLevel.time_ + 30000;
}

// on the ground the controls are the take-off's; a heading is planned once the aircraft is stopped and nothing holds
// it or sends it taxiing
void PlaneTasks::prepareTakeOff( Mind& c )
{
	c.controls.claim();
	c.bot.takeoff.groundTime = theLevel.time_;
	if( !holdingShort( c ) && !taxiing( c ) && theLevel.time_ >= c.bot.takeoff.planTime && c.speed < 20 )
		planDeparture( c );
}

// stopped short of the run, nose where it is
behavior::Status PlaneTasks::holdShort( Mind& c )
{
	c.view[YAW] = c.ps.vehicleAngles[YAW];
	c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
	return Status::Running;
}

// to the spot with enough runway, at walking pace; stopped on it, the next plan starts from there
behavior::Status PlaneTasks::taxiToSpot( Mind& c )
{
	constexpr float taxiSpeed = 35.0f, onTheSpot = 40.0f;
	Vec3 toSpot;
	VectorSubtract( c.bot.takeoff.taxiSpot, c.ent->r.currentOrigin, toSpot );
	toSpot[2] = 0;
	c.headFor( c.bot.takeoff.taxiSpot );
	c.view[PITCH] = 0.0f;
	if( VectorLength( toSpot ) >= onTheSpot )
	{
		c.cmd.forwardmove = c.speed < taxiSpeed ? moveFull : c.speed > taxiSpeed * 1.5f ? -moveFull : moveNone;
		return Status::Running;
	}
	c.cmd.forwardmove = -moveFull;
	if( c.speed <= 1 )
		c.bot.takeoff.taxiTime = 0;
	return Status::Running;
}

// along the planned heading; the nose comes up with speed in hand, since lifting off right at the stall drops the
// aircraft back
behavior::Status PlaneTasks::lineUpAndRoll( Mind& c )
{
	c.view[YAW] = theLevel.time_ < c.bot.takeoff.planTime ? c.bot.takeoff.yaw : c.ps.vehicleAngles[YAW];
	c.view[PITCH] = c.speed > c.veh.stallspeed * rotateSpeed ? -8.0f : 0.0f;
	if( fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) ) > 6 )
		c.cmd.forwardmove = c.speed < 12 ? moveFull : -moveFull;
	return Status::Running;
}

// slow on the ground: stop for what is in front, and keep off another aircraft's take-off roll
behavior::Status PlaneTasks::giveWay( Mind& c )
{
	const bool waiting = c.speed < c.veh.stallspeed * 0.5f && waitForTraffic( c );
	if( !waiting )
		c.bot.takeoff.heldSince = 0;
	else if( !c.bot.takeoff.heldSince )
		c.bot.takeoff.heldSince = theLevel.time_;
	return Status::Running;
}

// true while waiting on an aircraft ahead; brakes for one on its take-off roll without counting as waiting
bool PlaneTasks::waitForTraffic( Mind& c )
{
	// taxiing round one that will not move: only what is right under the nose stops it
	constexpr float underNose = 110.0f;
	vec3_t flat = { 0, c.ps.vehicleAngles[YAW], 0 }, forward;
	AngleVectors( flat, forward, nullptr, nullptr );
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		Vec3 rel = other->r.currentOrigin - c.ent->r.currentOrigin;
		const float dist = VectorNormalize( rel );
		if( dist < 320 && fabs( rel[2] * dist ) < 120 && DotProduct( rel, forward ) > 0.4f )
		{
			if( theLevel.time_ < c.bot.takeoff.sidestepTime && dist > underNose )
				continue;
			c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
			sidestepIfStuck( c, other, flat, forward );
			return true;
		}
		if( onTakeOffRollTowards( other, rel, dist ) )
		{
			c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
			return false;
		}
	}
	return false;
}

// Two aircraft that each wait for the other would stand there for the rest of the game. After a few seconds of
// waiting on one that stands still, taxi round it, to the right as traffic does (to the left where the right is not
// clear)
void PlaneTasks::sidestepIfStuck( Mind& c, const GameEntity* other, const vec3_t flat, const vec3_t forward )
{
	constexpr int patience = 5000, sidestepFor = 9000;
	constexpr float sidestep = 220.0f, aheadOfIt = 120.0f;
	const bool standing = VectorLength( other->client_->ps_.velocity ) < 5.0f;
	if( !standing || !c.bot.takeoff.heldSince || theLevel.time_ - c.bot.takeoff.heldSince <= patience )
		return;
	vec3_t right;
	AngleVectors( flat, nullptr, right, nullptr );
	for( float side : { 1.0f, -1.0f } )
	{
		Vec3 spot;
		VectorMA( c.ent->r.currentOrigin, side * sidestep, right, spot );
		VectorMA( spot, aheadOfIt, forward, spot );
		trace_t way;
		GameHost::trace( &way, c.ent->r.currentOrigin, nullptr, nullptr, spot, c.ent->s.number, MASK_PLAYERSOLID, false );
		if( way.fraction < 1.0f )
			continue;
		VectorCopy( spot, c.bot.takeoff.taxiSpot );
		c.bot.takeoff.taxiTime = c.bot.takeoff.sidestepTime = theLevel.time_ + sidestepFor;
		c.bot.takeoff.heldSince = 0;
		return;
	}
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

// follow the departure that was planned on the ground; the manoeuvre check still vets it
behavior::Status PlaneTasks::depart( Mind& c )
{
	c.view[YAW] = c.bot.takeoff.departYaw;
	c.view[PITCH] = -8.0f;
	return Status::Running;
}
