#include "helo.h"

// when the straight line to the destination is blocked, follow the air path round the obstacle instead
behavior::Status HeloTasks::navigate( Mind& c )
{
	Bots::State& bot = c.bot;
	const Vec3 here( c.ent->r.currentOrigin );
	const Vec3 toDest = c.steer.dest - here;
	const float range = toDest.length();
	const Vec3 probeEnd = range > 1500.0f ? here + toDest * (1500.0f / range) : c.steer.dest;
	if( !c.steer.hasDest || c.landed() || range <= 600.0f || AirNav::clearLine( here, probeEnd, 60.0f ) )
	{
		bot.path.points.clear();
		bot.path.following = false;
		return Status::Success;
	}
	if( theLevel.time_ >= bot.path.searchTime )
	{
		AirNav::findPath( here, c.steer.dest, bot.path.points );
		bot.path.index = 0;
		bot.path.searchTime = theLevel.time_ + 2000;
	}
	// on to the next waypoint once this one is reached or the one after can be flown to directly
	while( bot.path.index + 1 < bot.path.points.size() &&
		((bot.path.points[bot.path.index] - here).length() < 250.0f || AirNav::clearLine( here, bot.path.points[bot.path.index + 1], 60.0f )) )
		bot.path.index++;
	bot.path.following = bot.path.index < bot.path.points.size();
	if( !bot.path.following )
		return Status::Success;
	const Vec3 waypoint = bot.path.points[bot.path.index];
	Vec3 point;
	waypoint.copyTo( point );
	c.headFor( point );
	c.steer.wantHeight = std::clamp( c.sense.height + (waypoint.z() - here.z()), 60.0f, 1500.0f );
	c.steer.dist = std::max( c.steer.dist, c.steer.standoff + 400.0f );
	return Status::Success;
}

// climb or sink to the wanted height above the ground
behavior::Status HeloTasks::collective( Mind& c )
{
	// never climb into the roof, whatever asked for the height
	constexpr float headroom = 150.0f;
	if( !c.landed() )
	{
		const float highest = c.sense.height + BotWorld::ceilingAbove( c.ent->r.currentOrigin, c.ent->s.number ) - headroom;
		c.steer.wantHeight = std::min( c.steer.wantHeight, std::max( highest, 40.0f ) );
	}
	const float throttle = signedThrottle( c.ps, c.client->vehicle_ );
	float wantThrottle = std::clamp( (c.steer.wantHeight - c.sense.height) / 60.0f, -3.0f, static_cast<float>( c.veh.maxthrottle ) );
	if( c.steer.wait )
		wantThrottle = 0;
	else if( c.landed() )
		wantThrottle = std::max( wantThrottle, c.veh.maxthrottle * 0.6f );
	// setting down: a steady gentle sink for the last stretch
	if( c.steer.hold && c.sense.height < 150 )
		wantThrottle = -1.0f;
	if( throttle < floor( wantThrottle ) )
		c.cmd.buttons |= BUTTON_INCREASE;
	else if( throttle > ceil( wantThrottle ) )
		c.cmd.buttons |= BUTTON_DECREASE;
	return Status::Success;
}

// free distance for the whole airframe along a direction, up to reach
float HeloTasks::clearAlong( const Mind& c, const vec3_t dir, float reach )
{
	constexpr float margin = 24.0f;
	Vec3 mins, maxs, end;
	for( int k = 0; k < 3; k++ )
	{
		mins[k] = c.veh.mins[k] - margin;
		maxs[k] = c.veh.maxs[k] + margin;
	}
	VectorMA( c.ent->r.currentOrigin, reach, dir, end );
	trace_t tr;
	GameHost::trace( &tr, c.ent->r.currentOrigin, mins, maxs, end, c.ent->s.number, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	if( tr.startsolid )
		GameHost::trace( &tr, c.ent->r.currentOrigin, c.veh.mins, c.veh.maxs, end, c.ent->s.number, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	return tr.startsolid ? 0.0f : tr.fraction * reach;
}

// Where the helicopter goes this frame and how fast: the manner of moving gives a push, open air gives the direction,
// and the tilt controller flies it
behavior::Status HeloTasks::steer( Mind& c )
{
	c.steer.yawError = fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) );
	Travel travel = wantedTravel( c );
	c.steer.blocked = false;
	if( travel.speed > 1.0f )
		findOpenDirection( c, travel );
	stopBeforeDrift( c, travel );
	easeAwayFromWalls( c, travel );
	HeloControl::tilt( c, travel.yaw, travel.speed );
	return Status::Success;
}

// how hard to push in the wanted direction, as forward tilt in degrees (negative backs away), by the manner of moving.
// At fighting distance it keeps moving across the enemy's view instead of hanging still, changing direction now and
// then, and opening the range when too close
HeloTasks::Travel HeloTasks::wantedTravel( const Mind& c )
{
	constexpr float fullTilt = 14.0f, lowTilt = 9.0f, coverTilt = 12.0f, backTilt = -5.0f;
	constexpr int orbitPeriod = 6000;
	float push = 0;
	bool orbit = false;
	if( c.sense.height > 60 )
	{
		switch( c.steer.mode )
		{
		case Run:
			push = fullTilt;
			break;
		case ToPad:
			push = std::clamp( c.steer.dist / 60.0f, 3.0f, fullTilt );
			break;
		case MoveToCover:
			push = c.steer.dist > 150 ? coverTilt : 0.0f;
			break;
		case BackOff:
			push = backTilt;
			break;
		case Hover:
			break;
		default:
			if( c.steer.dist > c.steer.standoff + 300 )
				push = c.sense.height > 300 ? fullTilt : lowTilt;
			else
				orbit = c.sense.enemy != nullptr;
			break;
		}
	}
	Travel travel;
	travel.yaw = c.view[YAW] + (push < 0 ? 180.0f : 0.0f);
	if( orbit )
	{
		const float side = (((theLevel.time_ / orbitPeriod) + c.ent->s.number) & 1) ? 1.0f : -1.0f;
		travel.yaw = c.view[YAW] + side * (c.steer.dist < c.steer.standoff ? 125.0f : 90.0f);
		push = lowTilt;
	}
	travel.speed = fabs( push ) / HeloControl::maxPitch * static_cast<float>( c.veh.maxspeed );
	return travel;
}

// the wanted direction first, then either side of it, then back the way it came; hover and climb if nothing is open.
// Rising ground is not an obstacle: where the level line is blocked but a climbing one is clear, go on and climb
void HeloTasks::findOpenDirection( Mind& c, Travel& travel )
{
	constexpr float climbSlope = 0.4f, blocked = 2.0f, highest = 1500.0f;
	const float reach = 200.0f + travel.speed * 2.0f;
	ContextMap map( travel.yaw, 8 );
	map.want( travel.yaw, 1.0f );
	std::array<bool, ContextMap::maxSlots> mustClimb = {};
	const int slot = map.resolve( [&]( int s )
	{
		Vec3 angles = { 0, map.yawOf( s ), 0 }, dir, rising;
		AngleVectors( angles, dir, nullptr, nullptr );
		if( clearAlong( c, dir, reach ) >= reach )
			return 0.0f;
		if( s == 0 )
			c.steer.blocked = true;
		VectorCopy( dir, rising );
		rising[2] = climbSlope;
		mustClimb[s] = true;
		return clearAlong( c, rising, reach ) >= reach ? 0.0f : blocked;
	} );
	if( slot < 0 )
	{
		travel.speed = 0;
		c.steer.wantHeight = std::max( c.steer.wantHeight, std::min( c.sense.height + 300.0f, highest ) );
		return;
	}
	travel.yaw = map.yawOf( slot );
	if( slot != 0 )
		travel.speed *= 0.7f;
	if( mustClimb[slot] )
	{
		travel.speed *= 0.6f;
		c.steer.wantHeight = std::max( c.steer.wantHeight, std::min( c.sense.height + reach * climbSlope, highest ) );
	}
}

// the tilt is coarse, so the real motion can differ from the wanted one: stop if it leads into something that cannot
// be climbed over in time
void HeloTasks::stopBeforeDrift( const Mind& c, Travel& travel )
{
	constexpr float climbSlope = 0.4f;
	vec3_t moving = { c.ps.velocity[0], c.ps.velocity[1], 0 };
	const float drift = VectorNormalize( moving );
	if( drift <= 15.0f )
		return;
	const float stopping = 80.0f + drift * 0.8f;
	vec3_t rising = { moving[0], moving[1], climbSlope };
	if( clearAlong( c, moving, stopping ) < stopping && clearAlong( c, rising, stopping ) < stopping )
		travel.speed = 0;
}

// slow or hovering next to something: ease away from it
void HeloTasks::easeAwayFromWalls( const Mind& c, Travel& travel )
{
	constexpr float nearby = 140.0f, easeSpeed = 0.12f;
	if( travel.speed >= 20.0f )
		return;
	Vec3 away = { 0, 0, 0 };
	for( float around = 0.0f; around < 360.0f; around += 45.0f )
	{
		Vec3 angles = { 0, around, 0 }, dir;
		AngleVectors( angles, dir, nullptr, nullptr );
		const float free = clearAlong( c, dir, nearby );
		if( free < nearby )
			VectorMA( away, -(nearby - free) / nearby, dir, away );
	}
	if( VectorNormalize( away ) <= 0.05f )
		return;
	Vec3 angles;
	vectoangles( away, angles );
	travel.yaw = angles[YAW];
	travel.speed = static_cast<float>( c.veh.maxspeed ) * easeSpeed;
}
