#include "plane.h"

// parked where no heading has enough runway: find the nearest spot along the taxiway that has
bool PlaneTasks::findTaxiSpot( Mind& c, const vec3_t origin, float wanted )
{
	constexpr float taxiStep = 100.0f, taxiMax = 800.0f, edgeRoom = 50.0f;
	for( float dist = taxiStep; dist <= taxiMax; dist += taxiStep )
	{
		float best = 0;
		for( int step = 0; step < 24; step++ )
		{
			const float yaw = step * 15.0f;
			if( BotWorld::runwayRun( origin, yaw, dist ) < dist )
				continue;
			Vec3 level = { 0, yaw, 0 }, dir, spot;
			AngleVectors( level, dir, nullptr, nullptr );
			VectorMA( origin, dist, dir, spot );
			// not a spot on the very edge: the aircraft has to stop and turn there
			bool room = true;
			for( int side = 0; side < 4 && room; side++ )
				room = BotWorld::runwayRun( spot, side * 90.0f, edgeRoom ) >= edgeRoom;
			if( !room )
				continue;
			for( int turn = 0; turn < 12; turn++ )
			{
				const float run = BotWorld::runwayRun( spot, turn * 30.0f, wanted + 25.0f );
				if( run >= wanted && run > best )
				{
					best = run;
					VectorCopy( spot, c.bot.takeoff.taxiSpot );
				}
			}
		}
		if( best > 0 )
		{
			c.bot.takeoff.taxiTime = theLevel.time_ + 20000;
			c.bot.takeoff.planTime = 0;
#ifdef MF_BOT_DEBUG
			if( g_botDebug.integer >= 2 )
				Common::printf( "BOTTAKEOFF t %d bot %d taxi %.0f to %.0f %.0f run %.0f\n", theLevel.time_, c.ent->s.number, dist, c.bot.takeoff.taxiSpot[0], c.bot.takeoff.taxiSpot[1], best );
#endif
			return true;
		}
	}
	return false;
}

// Chooses the take-off heading: enough runway to reach flying speed, nothing standing on it, and from the lift-off
// point a climb, straight or turning, that fits the terrain and leads somewhere.
void PlaneTasks::planDeparture( Mind& c )
{
	constexpr float groundAcceleration = 85.0f;
	constexpr float climbSlope = 0.14f;
	constexpr float departure = 900.0f;
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
	const float stall = static_cast<float>( c.veh.stallspeed );
	const float groundRun = stall * rotateSpeed * stall * rotateSpeed / (2.0f * groundAcceleration) * 1.15f;
	const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	const float radius = stall * 1.15f / turnRate;
	// the run estimate is rough and a runway end is unforgiving: a comfortable length is half as much again
	const float comfortable = groundRun * 1.5f + 150.0f;
	float best = 0, bestRun = 0, longest = 0;
	bool bestClear = false;
	for( int step = 0; step < 24; step++ )
	{
		const float yaw = step * 15.0f;
		const float run = BotWorld::runwayRun( origin, yaw, 4000 );
		longest = std::max( longest, run );
		if( run < 150 )
			continue;
		Vec3 level = { 0, yaw, 0 }, dir, start;
		trace_t ground;
		AngleVectors( level, dir, nullptr, nullptr );
		VectorCopy( origin, start );
		start[2] += 14;
		const float roll = std::min( run, groundRun + 100.0f );
		BotWorld::pathBlocked( start, dir, roll, c.ent->s.number, ground, 30.0f );
		const bool rollClear = ground.startsolid || ground.fraction >= 1.0f;

		Pose liftOff;
		VectorMA( origin, std::min( run, groundRun ), dir, liftOff.origin );
		liftOff.origin[2] += 30;
		liftOff.yaw = yaw;
		liftOff.seconds = 3.0f;
		float departTurn = 0;
		bool departClear = false;
		for( float turn : { 0.0f, 30.0f, -30.0f, 60.0f, -60.0f, 90.0f, -90.0f } )
		{
			Pose end;
			if( sweepFrom( c, liftOff, radius, turn, climbSlope, departure, 60.0f, &end ) >= departure && wayOut( c, end, radius, departure ) )
			{
				departTurn = turn;
				departClear = true;
				break;
			}
		}
		const float score = (run >= groundRun ? 5000.0f : 0.0f) + (rollClear ? 3000.0f : 0.0f) + (departClear ? 4000.0f - fabs( departTurn ) * 5.0f : 0.0f) +
			// more runway is better up to twice a comfortable length: with the credit stopping at the bare
			// need, the first short heading tied with the long one, was chosen, was then found too short to
			// be comfortable, and the aircraft taxied about looking for a better place for the rest of the game
			std::min( run, comfortable * 2.0f ) * 2.0f;
		if( score > best )
		{
			best = score;
			bestRun = run;
			bestClear = rollClear;
			c.bot.takeoff.yaw = yaw;
			c.bot.takeoff.departYaw = yaw + departTurn;
		}
	}
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 )
		Common::printf( "BOTTAKEOFF t %d bot %d veh %d at %.0f %.0f %.0f facing %.0f chose %.0f then %.0f run %.0f need %.0f longest %.0f score %.0f\n", theLevel.time_, c.ent->s.number,
			c.client->vehicle_, origin[0], origin[1], origin[2], c.ps.vehicleAngles[YAW], c.bot.takeoff.yaw, c.bot.takeoff.departYaw, bestRun, groundRun, longest, best );
#endif
	// with less than a comfortable length, taxi to a better place
	if( bestRun < comfortable && findTaxiSpot( c, origin, comfortable ) )
		return;
	// someone is standing on the run: give them a moment to move before rolling
	constexpr int holdLimit = 8;
	if( best > 0 && !bestClear && c.bot.takeoff.holds < holdLimit )
	{
		c.bot.takeoff.holds++;
		c.bot.takeoff.holdTime = theLevel.time_ + 1000;
		return;
	}
	if( best > 0 )
		c.bot.takeoff.planTime = theLevel.time_ + 30000;
}

// On the ground: wait when told to hold, plan a departure, taxi to a spot with enough runway if needed, then roll
// and lift off. Slow, it stops for anyone ahead and keeps off the path of an aircraft on its roll
behavior::Status PlaneTasks::takeOff( Mind& c )
{
	c.controls.claim();
	c.bot.takeoff.groundTime = theLevel.time_;
	if( theLevel.time_ < c.bot.takeoff.holdTime )
	{
		c.view[YAW] = c.ps.vehicleAngles[YAW];
		c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
		return Status::Running;
	}
	if( theLevel.time_ >= c.bot.takeoff.taxiTime && theLevel.time_ >= c.bot.takeoff.planTime && c.speed < 20 )
		planDeparture( c );
	if( theLevel.time_ < c.bot.takeoff.holdTime )
		return Status::Running;
	if( theLevel.time_ < c.bot.takeoff.taxiTime )
	{
		constexpr float taxiSpeed = 35.0f;
		Vec3 toSpot;
		VectorSubtract( c.bot.takeoff.taxiSpot, c.ent->r.currentOrigin, toSpot );
		toSpot[2] = 0;
		c.headFor( c.bot.takeoff.taxiSpot );
		c.view[PITCH] = 0.0f;
		if( VectorLength( toSpot ) < 40 )
		{
			c.cmd.forwardmove = -moveFull;
			// stopped on the spot: the next plan starts from here
			if( c.speed <= 1 )
				c.bot.takeoff.taxiTime = 0;
		}
		else
			c.cmd.forwardmove = c.speed < taxiSpeed ? moveFull : c.speed > taxiSpeed * 1.5f ? -moveFull : moveNone;
	}
	else
	{
		c.view[YAW] = theLevel.time_ < c.bot.takeoff.planTime ? c.bot.takeoff.yaw : c.ps.vehicleAngles[YAW];
		// leave the ground with speed in hand: lifting off right at the stall drops the aircraft back
		c.view[PITCH] = c.speed > c.veh.stallspeed * rotateSpeed ? -8.0f : 0.0f;
		if( fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) ) > 6 )
			c.cmd.forwardmove = c.speed < 12 ? moveFull : -moveFull;
	}
	bool waiting = false;
	if( c.speed < c.veh.stallspeed * 0.5f )
	{
		vec3_t flat = { 0, c.ps.vehicleAngles[YAW], 0 }, forward;
		AngleVectors( flat, forward, nullptr, nullptr );
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			Vec3 rel;
			rel = other->r.currentOrigin - c.ent->r.currentOrigin;
			const float dist = VectorNormalize( rel );
			if( dist < 320 && fabs( rel[2] * dist ) < 120 && DotProduct( rel, forward ) > 0.4f )
			{
				// taxiing round one that will not move: only what is right under the nose stops it
				constexpr float underNose = 110.0f;
				if( theLevel.time_ < c.bot.takeoff.sidestepTime && dist > underNose )
					continue;
				c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
				waiting = true;
				// Two aircraft that each wait for the other would stand there for the rest of the game. After
				// a few seconds of waiting on one that stands still, taxi round it, to the right as traffic
				// does (to the left where the right is not clear)
				constexpr int patience = 5000, sidestepFor = 9000;
				constexpr float sidestep = 220.0f, aheadOfIt = 120.0f;
				const bool standing = VectorLength( other->client_->ps_.velocity ) < 5.0f;
				if( standing && c.bot.takeoff.heldSince && theLevel.time_ - c.bot.takeoff.heldSince > patience )
				{
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
						break;
					}
				}
				break;
			}
			// stay off the path of an aircraft on its take-off roll, unless already standing in it
			constexpr float rollSpeed = 60.0f, rollAhead = 1500.0f, rollWidth = 250.0f, onTheLine = 60.0f;
			Vec3 track;
			VectorCopy( other->client_->ps_.velocity, track );
			track[2] = 0;
			if( !(other->client_->ps_.ONOFF & OO_LANDED) || VectorNormalize( track ) < rollSpeed )
				continue;
			const float along = -DotProduct( rel, track ) * dist;
			const float aside = sqrt( std::max( 0.0f, dist * dist - along * along ) );
			if( along > 0 && along < rollAhead && aside < rollWidth && aside > onTheLine )
			{
				c.cmd.forwardmove = c.speed > 1 ? -moveFull : moveNone;
				break;
			}
		}
	}
	if( !waiting )
		c.bot.takeoff.heldSince = 0;
	else if( !c.bot.takeoff.heldSince )
		c.bot.takeoff.heldSince = theLevel.time_;
	return Status::Running;
}

// follow the departure that was planned on the ground; the manoeuvre check still vets it
behavior::Status PlaneTasks::depart( Mind& c )
{
	c.view[YAW] = c.bot.takeoff.departYaw;
	c.view[PITCH] = -8.0f;
	return Status::Running;
}
