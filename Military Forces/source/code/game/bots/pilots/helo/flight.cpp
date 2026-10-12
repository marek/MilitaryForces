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

// A helicopter goes where it tilts, in any direction, and stops when it levels. The pilot keeps the nose on what it
// is looking at and tilts toward the clear direction nearest to where it wants to go.
behavior::Status HeloTasks::steer( Mind& c )
{
	constexpr float maxPitch = 30.0f;
	constexpr float maxRoll = 40.0f;
	constexpr float levelling = 0.75f;
	c.steer.yawError = fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) );

	// how hard to push in the wanted direction, as forward tilt in degrees (negative backs away)
	float push = 0;
	bool orbit = false;
	if( c.sense.height > 60 )
	{
		switch( c.steer.mode )
		{
		case Run:
			push = 14.0f;
			break;
		case ToPad:
			push = std::clamp( c.steer.dist / 60.0f, 3.0f, 14.0f );
			break;
		case MoveToCover:
			push = c.steer.dist > 150 ? 12.0f : 0.0f;
			break;
		case BackOff:
			push = -5.0f;
			break;
		case Hover:
			break;
		default:
			if( c.steer.dist > c.steer.standoff + 300 )
				push = c.sense.height > 300 ? 14.0f : 9.0f;
			else if( c.sense.enemy )
				orbit = true;
			break;
		}
	}

	const float maxSpeed = static_cast<float>( c.veh.maxspeed );
	const float sideSpeed = std::max( 1.0f, maxSpeed );
	float travelYaw = c.view[YAW] + (push < 0 ? 180.0f : 0.0f);
	if( orbit )
	{
		// at fighting distance: keep moving across the enemy's view instead of hanging still, changing
		// direction now and then, and opening the range when too close
		constexpr int orbitPeriod = 6000;
		const float side = (((theLevel.time_ / orbitPeriod) + c.ent->s.number) & 1) ? 1.0f : -1.0f;
		travelYaw = c.view[YAW] + side * (c.steer.dist < c.steer.standoff ? 125.0f : 90.0f);
		push = 9.0f;
	}
	float speed = fabs( push ) / maxPitch * maxSpeed;
	c.steer.blocked = false;
	constexpr float climbSlope = 0.4f;
	if( speed > 1.0f )
	{
		// the wanted direction first, then either side of it, then back the way it came; hover if nothing is open.
		// Rising ground is not an obstacle: where the level line is blocked but a climbing one is clear, go on and climb
		const float reach = 200.0f + speed * 2.0f;
		bool found = false;
		for( float offset : { 0.0f, 45.0f, -45.0f, 90.0f, -90.0f, 135.0f, -135.0f, 180.0f } )
		{
			Vec3 angles = { 0, travelYaw + offset, 0 }, dir, rising;
			AngleVectors( angles, dir, nullptr, nullptr );
			VectorCopy( dir, rising );
			rising[2] = climbSlope;
			const bool level = clearAlong( c, dir, reach ) >= reach;
			if( !level && offset == 0.0f )
				c.steer.blocked = true;
			if( !level && clearAlong( c, rising, reach ) < reach )
				continue;
			travelYaw += offset;
			if( offset != 0.0f )
				speed *= 0.7f;
			if( !level )
			{
				speed *= 0.6f;
				c.steer.wantHeight = std::max( c.steer.wantHeight, std::min( c.sense.height + reach * climbSlope, 1500.0f ) );
			}
			found = true;
			break;
		}
		if( !found )
		{
			speed = 0;
			c.steer.wantHeight = std::max( c.steer.wantHeight, std::min( c.sense.height + 300.0f, 1500.0f ) );
		}
	}

	// the tilt is coarse, so the real motion can differ from the wanted one: stop if it leads into something
	// that cannot be climbed over in time
	vec3_t moving = { c.ps.velocity[0], c.ps.velocity[1], 0 };
	const float drift = VectorNormalize( moving );
	if( drift > 15.0f )
	{
		const float stopping = 80.0f + drift * 0.8f;
		vec3_t rising = { moving[0], moving[1], climbSlope };
		if( clearAlong( c, moving, stopping ) < stopping && clearAlong( c, rising, stopping ) < stopping )
			speed = 0;
	}

	// slow or hovering next to something: ease away from it
	if( speed < 20.0f )
	{
		constexpr float nearby = 140.0f;
		Vec3 away = { 0, 0, 0 };
		for( float around = 0.0f; around < 360.0f; around += 45.0f )
		{
			Vec3 angles = { 0, around, 0 }, dir;
			AngleVectors( angles, dir, nullptr, nullptr );
			const float free = clearAlong( c, dir, nearby );
			if( free < nearby )
				VectorMA( away, -(nearby - free) / nearby, dir, away );
		}
		if( VectorNormalize( away ) > 0.05f )
		{
			Vec3 angles;
			vectoangles( away, angles );
			travelYaw = angles[YAW];
			speed = maxSpeed * 0.12f;
		}
	}

	vec3_t travel = { 0, travelYaw, 0 }, body = { 0, c.ps.vehicleAngles[YAW], 0 }, dir, forward, right;
	AngleVectors( travel, dir, nullptr, nullptr );
	AngleVectors( body, forward, right, nullptr );
	// the wanted motion over the ground as nose-down and bank, each by what that tilt gives in speed
	const float wantPitch = std::clamp( DotProduct( dir, forward ) * speed / maxSpeed * maxPitch, -maxPitch, maxPitch );
	const float wantRoll = std::clamp( DotProduct( dir, right ) * speed / sideSpeed * maxRoll, -maxRoll * 0.8f, maxRoll * 0.8f );
	const float pitchError = wantPitch - c.ps.vehicleAngles[PITCH];
	// level precisely: a couple of degrees left over is a steady drift
	if( pitchError > levelling )
		c.cmd.forwardmove = moveFull;
	else if( pitchError < -levelling )
		c.cmd.forwardmove = -moveFull;
	const float rollError = wantRoll - c.ps.vehicleAngles[ROLL];
	if( rollError > levelling )
		c.cmd.rightmove = moveFull;
	else if( rollError < -levelling )
		c.cmd.rightmove = -moveFull;
	return Status::Success;
}
