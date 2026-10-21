#include "helo.h"

// when the straight line to the destination is blocked, follow the air path round the obstacle instead. A path once
// taken is kept until it is next searched: at the edge of an obstacle the straight line is clear one frame and blocked
// the next, and the nose would swing between the path and the destination with it
behavior::Status HeloTasks::navigate( Mind& c )
{
	constexpr float probeReach = 1500.0f, nearEnough = 600.0f, clearance = 60.0f, waypointReached = 250.0f;
	constexpr int searchEvery = 2000;
	Bots::State& bot = c.bot;
	const Vec3 here( c.ent->r.currentOrigin );
	const Vec3 toDest = c.steer.dest - here;
	const float range = toDest.length();
	const Vec3 probeEnd = range > probeReach ? here + toDest * (probeReach / range) : c.steer.dest;
	const bool keepPath = bot.path.following && theLevel.time_ < bot.path.searchTime;
	if( !c.steer.hasDest || c.landed() || range <= nearEnough ||
		(!keepPath && AirNav::clearLine( here, probeEnd, clearance ) && !c.riskyLeg( here, c.steer.dest, true )) )
	{
		bot.path.points.clear();
		bot.path.following = false;
		return Status::Success;
	}
	if( theLevel.time_ >= bot.path.searchTime )
	{
		AirNav::findPath( here, c.steer.dest, bot.path.points, Vec3(), 0.0f, c.danger( true ) );
		bot.path.index = 0;
		bot.path.searchTime = theLevel.time_ + searchEvery + ThinkRate::stagger( c.ent->s.number, 300 );
	}
	// on to the next waypoint once this one is reached or the one after can be flown to directly
	while( bot.path.index + 1 < bot.path.points.size() &&
		((bot.path.points[bot.path.index] - here).length() < waypointReached ||
		(AirNav::clearLine( here, bot.path.points[bot.path.index + 1], clearance ) && !c.riskyLeg( here, bot.path.points[bot.path.index + 1], true ))) )
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

// gear down to set down, up once clear of the ground: down it costs a fifth of the speed
behavior::Status HeloTasks::gear( Mind& c )
{
	constexpr float settingDown = 60.0f, clearOfGround = 150.0f;
	if( c.landed() )
		return Status::Success;
	if( c.steer.hold || c.steer.wait || c.steer.wantHeight < settingDown )
		HeloControl::gear( c, true );
	else if( c.sense.height > clearOfGround )
		HeloControl::gear( c, false );
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
// In transit it goes near full tilt and eases off by its stopping distance; near the ground it goes slower. At fighting
// distance it keeps moving across the enemy's view instead of hanging still, changing direction now and then, and
// opening the range when too close
HeloTasks::Travel HeloTasks::wantedTravel( const Mind& c )
{
	constexpr int orbitPeriod = 6000;
	bool orbit = false;
	float push = c.sense.height > 60 ? pushFor( c, orbit ) : 0.0f;
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

// the forward tilt for the manner of moving; orbit is set when it is at fighting distance and should circle
float HeloTasks::pushFor( const Mind& c, bool& orbit )
{
	constexpr float transitTilt = HeloControl::maxPitch * 0.88f, coverTilt = 12.0f, backTilt = -5.0f;
	constexpr float padSlope = 60.0f, padTilt = 3.0f, nearGround = 75.0f, arriving = 300.0f;
	switch( c.steer.mode )
	{
	case Run:
		return HeloControl::maxPitch;
	case ToPad:
		return std::clamp( c.steer.dist / padSlope, padTilt, transitTilt );
	case MoveToCover:
		return c.steer.dist > 150 ? coverTilt : 0.0f;
	case BackOff:
		return backTilt;
	case Hover:
		return 0.0f;
	default:
		break;
	}
	// a negative standoff asks to go right to the place
	if( c.steer.dist > c.steer.standoff + arriving )
		return std::min( c.sense.height > nearGround ? transitTilt : lowTilt, HeloControl::tiltToStop( c, c.steer.dist - std::max( c.steer.standoff, 0.0f ) ) );
	orbit = c.sense.enemy != nullptr;
	return 0.0f;
}

// the wanted direction first, then either side of it, then back the way it came; hover and climb if nothing is open.
// Rising ground is not an obstacle: where the level line is blocked but a climbing one is clear, go on and climb
void HeloTasks::findOpenDirection( Mind& c, Travel& travel )
{
	constexpr float climbSlope = 0.4f, blocked = 2.0f, highest = 1500.0f;
	const float reach = 200.0f + travel.speed * 2.0f;
	ContextMap map( travel.yaw, 8 );
	map.want( travel.yaw, 1.0f );
	avoidThreats( c, map, reach );
	avoidTraffic( c, map, travel.speed );
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

// the team's threat map as danger in each direction, by the pilot's caution: a cautious pilot goes round a SAM
// site's reach where the way round is open
void HeloTasks::avoidThreats( const Mind& c, ContextMap& map, float reach )
{
	constexpr float lookFurther = 4.0f, cautionWeight = 0.5f;
	const int team = c.client->sess_.sessionTeam_;
	const float caution = c.who().caution * cautionWeight;
	for( int s = 0; s < map.slots(); s++ )
	{
		Vec3 angles = { 0, map.yawOf( s ), 0 }, dir, at;
		AngleVectors( angles, dir, nullptr, nullptr );
		VectorMA( c.ent->r.currentOrigin, reach * lookFurther, dir, at );
		map.addDanger( s, caution * ThreatMap::danger( team, at, true ) );
	}
}

// other aircraft in the air as danger along where they will be: a direction that would bring it within reach of one in
// the next moments is closed, so two helicopters going for the same enemy do not fly into each other. A direction
// that opens the distance stays open, however close they are already
void HeloTasks::avoidTraffic( const Mind& c, ContextMap& map, float speed )
{
	constexpr float lookAhead = 2.0f, room = 60.0f, nearby = 1500.0f, blocked = 2.0f;
	const Vec3 here( c.ent->r.currentOrigin );
	const float own = radiusOf( c.client->vehicle_ );
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		const playerState_t& ps = other->client_->ps_;
		if( !(availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) || (ps.ONOFF & OO_LANDED) )
			continue;
		const Vec3 apart = here - Vec3( other->r.currentOrigin );
		if( apart.length() > nearby )
			continue;
		const float clearance = own + radiusOf( other->client_->vehicle_ ) + room;
		for( int s = 0; s < map.slots(); s++ )
		{
			Vec3 angles = { 0, map.yawOf( s ), 0 }, dir;
			AngleVectors( angles, dir, nullptr, nullptr );
			const Vec3 closing( dir[0] * speed - ps.velocity[0], dir[1] * speed - ps.velocity[1], c.ps.velocity[2] - ps.velocity[2] );
			const float rate = DotProduct( closing, closing );
			const float when = rate > 0.0f ? std::clamp( -DotProduct( apart, closing ) / rate, 0.0f, lookAhead ) : 0.0f;
			if( when > 0.0f && (apart + closing * when).length() < clearance )
				map.addDanger( s, blocked );
		}
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
