#include "../../core/mind.h"
#include <algorithm>
#include <cmath>
#include <numbers>

using behavior::Status;

// someone is on or just over the runway, or a bot with a lower number is already on final for it
bool Bots::runwayBusy( int padIndex, GameEntity* self )
{
	const BotWorld::Pad& pad = BotWorld::pads_[padIndex];
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == self || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		Vec3 rel;
		VectorSubtract( other->r.currentOrigin, pad.center, rel );
		const float along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
		const float side = rel[1] * pad.axis[0] - rel[0] * pad.axis[1];
		if( fabs( along ) < pad.halfLength + 150 && fabs( side ) < 220 && fabs( rel[2] ) < 160 )
			return true;
		if( isBot( i ) && i < self->s.number && states_[i].rearm.phase == 2 && states_[i].rearm.pad >= 0 && Distance( BotWorld::pads_[states_[i].rearm.pad].center, pad.center ) < 400 )
			return true;
	}
	return false;
}

// Landing as a sequence of steps: look the runway over, fly out and line up, come down the slope,
// roll out, taxi to the parking spot, sit until refuelled and rearmed, then give the runway back.

namespace
{
	enum Phase { Survey, Approach, Final, Ground };

	// slower than this on the ground a plane is taxiing, not rolling out
	constexpr float taxiSpeed = 60.0f;
	// the steepest path a plane holds on final while it is still slowing down (about 11 degrees)
	constexpr float steepestDescent = 0.2f;
	// nose slightly up on the ground keeps the tail down
	constexpr float groundPitch = 3.0f;

	// where the plane is relative to the runway, in the runway's own terms
	struct Geometry
	{
		const BotWorld::Pad&	pad;
		float				stall;
		float				along;		// along the runway from its centre, in the pad's own direction
		float				side;		// off the centre line
		float				above;		// height over the runway surface
		float				before;		// distance still to fly to the touchdown point; negative once past it
		float				lateral;	// off the centre line, positive to the left of the landing direction
		float				pos;		// along the runway from its centre, in the landing direction
		float				axisYaw;	// heading of the landing direction
		Vec3				travel;		// unit vector of the landing direction

		Geometry( const BotWorld::Pad& runway, const Bots::Mind& c ) : pad( runway ), stall( static_cast<float>( c.veh.stallspeed ) )
		{
			Vec3 rel, angles;
			VectorSubtract( c.ent->r.currentOrigin, pad.center, rel );
			along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
			side = rel[1] * pad.axis[0] - rel[0] * pad.axis[1];
			above = c.ent->r.currentOrigin[2] - pad.z;
			pos = along * c.bot.landing.dir;
			before = -(pad.halfLength - BotWorld::thresholdInset( pad )) - pos;
			VectorScale( pad.axis, c.bot.landing.dir, travel );
			vectoangles( travel, angles );
			axisYaw = angles[YAW];
			lateral = rel[0] * -travel[1] + rel[1] * travel[0];
		}
	};

	// abandon this landing: optionally avoid the pad for a while, and do not try to land again before retryDelay
	void giveUp( Bots::Mind& c, int padPenalty, int retryDelay )
	{
		if( padPenalty )
		{
			c.bot.rearm.avoid( c.bot.rearm.pad, theLevel.time_ + padPenalty );
		}
		c.bot.rearm.letGo();
		c.bot.landing.giveUp = theLevel.time_ + retryDelay;
		c.bot.desire = Bots::Goal::Roam;
		c.bot.goalEvalTime = 0;
		c.bot.commitTime = 0;
	}

	// the brake key toggles, so it is pressed only when the state is not the wanted one
	void wheelBrakes( Bots::Mind& c, bool on )
	{
		if( on != ((c.ps.ONOFF & OO_SPEEDBRAKE) != 0) )
			c.cmd.buttons |= BUTTON_BRAKE;
	}

	bool landed( const Bots::Mind& c )
	{
		return (c.ps.ONOFF & OO_LANDED) != 0;
	}

	// the strip this plane needs from touching down at a speed until it is slow enough to taxi: the game's own
	// ground slowing with the brakes on and the throttle shut, stepped through
	float rollOutRoom( const Bots::Mind& c, float speed )
	{
		constexpr float step = 0.05f, margin = 40.0f;
		const float base = c.veh.accel / 3.0f, top = static_cast<float>( c.veh.maxspeed ), stall = static_cast<float>( c.veh.stallspeed );
		float room = margin;
		for( int i = 0; i < 600 && speed > taxiSpeed; i++ )
		{
			float slowing = base * (speed / top) * 1.7f * 1.1f;
			if( speed < stall )
				slowing *= 1.0f + (stall - speed) / stall;
			room += speed * step;
			speed -= std::max( slowing, 5.0f ) * step;
		}
		return room;
	}

	// clearly airborne again (a bounce does not count)
	bool liftedOff( const Bots::Mind& c )
	{
		return !landed( c ) && c.sense.height > fabs( c.veh.mins[2] ) + c.veh.gearheight + 40.0f;
	}
}

namespace
{
	// one end of the runway to land towards: the steepest slope the terrain forces on the approach, and how far out
	// the path is clear
	void surveyEnd( const BotWorld::Pad& pad, float dir, float& slope, float& clear )
	{
		constexpr float lookOut = 3500.0f;
		Vec3 from, out;
		trace_t tr;
		slope = BotWorld::approachSlope( pad, dir );
		VectorMA( pad.center, -dir * (pad.halfLength - BotWorld::thresholdInset( pad )), pad.axis, from );
		from[2] = pad.z + 250;
		VectorScale( pad.axis, -dir, out );
		out[2] = slope;
		VectorNormalize( out );
		BotWorld::pathBlocked( from, out, lookOut, ENTITYNUM_NONE, tr );
		clear = tr.fraction * lookOut;
	}

	// the lane with the longest clear run, a little or well to either side of the centre line
	float bestLane( const BotWorld::Pad& pad, float landDir )
	{
		float bestLength = -1, offset = 45.0f;
		for( float lane : { 45.0f, -45.0f, 120.0f, -120.0f } )
		{
			Vec3 from, along;
			trace_t tr;
			VectorScale( pad.axis, landDir, along );
			VectorMA( pad.center, -(pad.halfLength - BotWorld::thresholdInset( pad )), along, from );
			from[0] += -along[1] * lane;
			from[1] += along[0] * lane;
			from[2] = pad.z + 30;
			BotWorld::pathBlocked( from, along, pad.halfLength * 2.0f - 300.0f, ENTITYNUM_NONE, tr );
			if( tr.fraction > bestLength )
			{
				bestLength = tr.fraction;
				offset = lane;
			}
		}
		return offset;
	}
}

// choose the landing direction: the end with the flatter, clearer approach, or no landing here at all. The nearer end
// is kept unless the other is clearly better; the pad is given up when neither will do
Status LandingSteps::survey( Bots::Mind& c )
{
	if( c.bot.rearm.pad < 0 )
		return Status::Failure;
	const BotWorld::Pad& pad = BotWorld::pads_[c.bot.rearm.pad];
	c.bot.rearm.phase = Survey;
	c.bot.landing.start = theLevel.time_;
	c.bot.landing.outbound = false;
	Vec3 rel;
	VectorSubtract( c.ent->r.currentOrigin, pad.center, rel );
	const float along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
	c.bot.landing.dir = along > 0 ? -1.0f : 1.0f;
	if( landed( c ) )
		return Status::Success;
	float slope[2], clear[2];
	for( int k = 0; k < 2; k++ )
		surveyEnd( pad, k ? 1.0f : -1.0f, slope[k], clear[k] );
	int chosen = c.bot.landing.dir > 0 ? 1 : 0;
	if( slope[1 - chosen] < slope[chosen] * 0.7f || (clear[chosen] < 1500 && clear[1 - chosen] > clear[chosen]) )
		chosen = 1 - chosen;
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 )
		Common::printf( "BOTSURVEY t %d bot %d pad %d slope %.2f %.2f clear %.0f %.0f chosen %d\n", theLevel.time_, c.ent->s.number, c.bot.rearm.pad, slope[0], slope[1], clear[0], clear[1], chosen );
#endif
	if( slope[chosen] > 0.6f || clear[chosen] < 1500 )
	{
		giveUp( c, 300000, 3000 );
		return Status::Failure;
	}
	c.bot.landing.dir = chosen ? 1.0f : -1.0f;
	c.bot.landing.slope = slope[chosen];
	c.bot.landing.run = std::max( 900.0f, clear[chosen] - 300.0f );
	Vec3 over;
	VectorCopy( pad.center, over );
	over[2] = pad.z + 40;
	c.bot.landing.ceiling = BotWorld::ceilingAbove( over, ENTITYNUM_NONE );
	c.bot.landing.offset = bestLane( pad, c.bot.landing.dir );
	return Status::Success;
}

// fly away from the runway far enough to turn round, then come back lined up at approach height
Status LandingSteps::approach( Bots::Mind& c )
{
	if( c.bot.rearm.pad < 0 )
		return Status::Failure;
	if( landed( c ) )
		return Status::Success;

	const Geometry g( BotWorld::pads_[c.bot.rearm.pad], c );
	c.bot.rearm.phase = Approach;
	if( theLevel.time_ > c.bot.landing.start + 120000 )
	{
		giveUp( c, 0, 90000 );
		return Status::Failure;
	}

	// the run-in has to be several turn radii long, and its height follows from the slope the terrain allows
	const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	const float radius = std::max( c.speed, g.stall * 1.7f ) / turnRate;
	const float wantRun = std::clamp( radius * 5.0f + 1200.0f, 1800.0f, c.bot.landing.run );
	c.bot.landing.approachHeight = g.pad.z + std::min( std::clamp( wantRun * c.bot.landing.slope * 0.75f, 300.0f, 800.0f ), std::max( 150.0f, c.bot.landing.ceiling - 250.0f ) );
	const float headingError = fabs( AngleSubtract( g.axisYaw, c.ps.vehicleAngles[YAW] ) );
	// inbound and near the centre line the height comes down with the distance left, so the final starts on the slope
	if( !c.bot.landing.outbound && fabs( g.lateral ) < 400 && headingError < 30 )
		c.bot.landing.approachHeight = std::min( c.bot.landing.approachHeight, g.pad.z + std::max( 120.0f, g.before * std::min( c.bot.landing.slope, steepestDescent ) * 0.9f ) );

	// outbound until far enough out (or something is in the way); turn outbound when too close or badly placed
	if( c.bot.landing.outbound )
	{
		Vec3 outDir, wish = { 0, g.axisYaw + 180.0f, 0 };
		trace_t tr;
		AngleVectors( wish, outDir, nullptr, nullptr );
		if( g.before > wantRun * 0.95f || BotWorld::pathBlocked( c.ent->r.currentOrigin, outDir, radius * 2.5f, c.ent->s.number, tr ) )
			c.bot.landing.outbound = false;
	}
	else if( g.before < wantRun * 0.35f || (g.before < wantRun * 0.6f && (headingError > 60 || fabs( g.lateral ) > radius * 2.0f)) )
		c.bot.landing.outbound = true;

	// outbound: fly the reciprocal, angled towards the centre line. Inbound: cut back onto the centre line
	const float heading = c.bot.landing.outbound ? g.axisYaw + 180.0f + (g.lateral >= 0 ? -20.0f : 20.0f) :
		g.axisYaw - std::clamp( static_cast<float>( atan2( g.lateral, radius * 2.5f ) * 180.0 / std::numbers::pi_v<float> ), -60.0f, 60.0f );
	Vec3 wish = { 0, heading, 0 }, dir;
	AngleVectors( wish, dir, nullptr, nullptr );
	VectorMA( c.ent->r.currentOrigin, 2500.0f, dir, c.bot.goal );
	c.bot.goal[2] = c.bot.landing.approachHeight;
	c.bot.goalTime = theLevel.time_ + 3000;

	// low enough to come down to the touchdown point at a slope the plane can fly: starting the final from higher only ends in a go-around
	const bool canDescend = g.above < g.before * steepestDescent + 100.0f;
	const bool linedUp = !c.bot.landing.outbound && g.before > wantRun * 0.3f && fabs( g.lateral ) < std::max( 150.0f, radius * 0.3f ) &&
		headingError < 15 && fabs( c.ent->r.currentOrigin[2] - c.bot.landing.approachHeight ) < 300 && canDescend;
	if( linedUp && !Bots::runwayBusy( c.bot.rearm.pad, c.ent ) )
		return Status::Success;
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 && theLevel.time_ % 1000 < 25 )
		Common::printf( "BOTAPPROACH t %d bot %d before %.0f lateral %.0f heading %.0f above %.0f out %d wantRun %.0f height %.0f radius %.0f descend %d busy %d speed %.0f fuel %d\n", theLevel.time_,
			c.ent->s.number, g.before, g.lateral, headingError, g.above, c.bot.landing.outbound, wantRun, c.bot.landing.approachHeight - g.pad.z, radius, canDescend, Bots::runwayBusy( c.bot.rearm.pad, c.ent ), c.speed, c.ps.stats[STAT_FUEL] );
#endif

	c.headFor( c.bot.goal );
	return Status::Running;
}

namespace
{
	// go around: still high past the threshold, far off to the side, or someone is on the runway. Still flying with
	// too little strip left to stop on is a go-around too: better than rolling off the far end
	bool mustGoAround( Bots::Mind& c, const Geometry& g, bool busy )
	{
		const float left = g.pad.halfLength - g.pos;
		const bool around = left < rollOutRoom( c, c.speed ) || (g.before < -200 && g.above > 150) || fabs( g.side ) > 900 || (g.before > 300 && busy);
#ifdef MF_BOT_DEBUG
		if( around && g_botDebug.integer >= 2 )
			Common::printf( "BOTGOAROUND t %d bot %d veh %d before %.0f above %.0f side %.0f half %.0f busy %d speed %.0f fuel %d\n", theLevel.time_, c.ent->s.number,
				c.ent->client_->vehicle_, g.before, g.above, g.side, g.pad.halfLength, busy, c.speed, c.ps.stats[STAT_FUEL] );
#endif
		return around;
	}

	// One straight slope down to the touchdown point: the slope the survey found clears the ground before the runway,
	// so the plane crosses the threshold already low and has the whole strip to stop on. Nose down by the slope itself,
	// corrected by how far off it the plane is: correcting alone trails the slope by its own error. Over the runway it
	// is flown down without a float, which uses the strip up, but nearly level at the end: a nose that reaches the
	// ground before the wheels is a crash. Too low before the runway, or terrain ahead of the nose: pull up
	void flyDownSlope( Bots::Mind& c, const Geometry& g, bool overRunway )
	{
		const float touch = fabs( c.veh.mins[2] ) + c.veh.gearheight;	// height of the origin when the wheels touch
		const bool shortField = g.pad.halfLength < 500;
		const float wantAbove = std::clamp( touch + std::max( 0.0f, g.before ) * c.bot.landing.slope, touch, 800.0f );
		const float slopeAngle = g.before > 0 ? atan( c.bot.landing.slope ) * 180.0f / std::numbers::pi_v<float> : 0.0f;
		c.view[PITCH] = std::clamp( slopeAngle + (g.above - wantAbove) / 6.0f, -10.0f, 16.0f );
		if( overRunway )
		{
			c.view[PITCH] = g.above > touch + 8 ? std::clamp( (g.above - touch) / 4.0f, 3.0f, 10.0f ) : 3.0f;
			return;
		}
		Vec3 nose;
		trace_t tr;
		AngleVectors( c.ps.vehicleAngles, nose, nullptr, nullptr );
		const bool near = shortField || g.before < 500;
		if( c.sense.height < (near ? 14.0f + touch : 60.0f) ||
			(BotWorld::pathBlocked( c.ent->r.currentOrigin, nose, near ? 250.0f : 500.0f, c.ent->s.number, tr ) && tr.entityNum == ENTITYNUM_WORLD) )
			c.view[PITCH] = -12.0f;
	}

	// slow in steps, to just above the stall by the threshold; the gear only comes down below the green-arc speed, asked
	// for once and given time to travel
	void slowForTouchdown( Bots::Mind& c, const Geometry& g, bool overRunway )
	{
		const bool shortField = g.pad.halfLength < 500;
		const float wantSpeed = g.stall * (overRunway ? 1.15f : g.before > 1200 ? 1.7f : shortField ? 1.2f : g.before > 500 ? 1.45f : 1.25f);
		c.cmd.forwardmove = c.speed < wantSpeed ? moveFull : -moveFull;
		const bool wantBrake = c.speed > wantSpeed * 1.25f;
		if( wantBrake != ((c.ps.ONOFF & OO_SPEEDBRAKE) != 0) )
			c.cmd.buttons |= BUTTON_BRAKE;
		if( !(c.ps.ONOFF & OO_GEAR) && c.speed < g.stall * SPEED_GREEN_ARC * 0.95f && theLevel.time_ > c.bot.landing.gearTime )
		{
			c.cmd.buttons |= BUTTON_GEAR;
			c.bot.landing.gearTime = theLevel.time_ + 4000;
		}
	}
}

// down the slope to the touchdown point, slowing to just above the stall, gear down
Status LandingSteps::final( Bots::Mind& c )
{
	if( c.bot.rearm.pad < 0 )
		return Status::Failure;
	if( landed( c ) )
		return Status::Success;
	const Geometry g( BotWorld::pads_[c.bot.rearm.pad], c );
	c.bot.rearm.phase = Final;
	c.controls.claim();
	if( mustGoAround( c, g, Bots::runwayBusy( c.bot.rearm.pad, c.ent ) ) )
	{
		c.controls.release();
		return Status::Failure;
	}
	Vec3 target;
	VectorMA( g.pad.center, g.pos + 900.0f, g.travel, target );
	target[0] += -g.travel[1] * c.bot.landing.offset;
	target[1] += g.travel[0] * c.bot.landing.offset;
	c.headFor( target );
	Vec3 under = c.ent->r.currentOrigin;
	under[2] = g.pad.z;
	const bool overRunway = BotWorld::runwayAt( under );
	flyDownSlope( c, g, overRunway );
	slowForTouchdown( c, g, overRunway );
	return Status::Running;
}

// straight ahead on the brakes until slow enough to taxi
Status LandingSteps::rollOut( Bots::Mind& c )
{
	if( c.bot.rearm.pad < 0 || liftedOff( c ) )
		return Status::Failure;
	c.bot.rearm.phase = Ground;
	c.controls.claim();
	c.view[YAW] = c.ps.vehicleAngles[YAW];
	c.view[PITCH] = groundPitch;
	if( c.speed <= taxiSpeed )
		return Status::Success;
	wheelBrakes( c, true );
	c.cmd.forwardmove = -moveFull;
	return Status::Running;
}

// to the parking spot at walking pace, then stop on it
Status LandingSteps::taxi( Bots::Mind& c )
{
	if( c.bot.rearm.pad < 0 || liftedOff( c ) )
		return Status::Failure;
	const BotWorld::Pad& pad = BotWorld::pads_[c.bot.rearm.pad];
	c.bot.rearm.phase = Ground;
	c.controls.claim();

	Vec3 toSpot;
	toSpot = pad.spot - c.ent->r.currentOrigin;
	toSpot[2] = 0;
	const float spotDist = VectorLength( toSpot );
	// along the runway until abeam the pad, then across to it: the straight line from where the roll ended may leave the runway
	Vec3 rel = Vec3( c.ent->r.currentOrigin ) - pad.center, spotRel = Vec3( pad.spot ) - pad.center, abeam;
	const float along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
	const float spotAlong = std::clamp( spotRel[0] * pad.axis[0] + spotRel[1] * pad.axis[1], -pad.halfLength + 50.0f, pad.halfLength - 50.0f );
	VectorMA( c.ent->r.currentOrigin, spotAlong - along, pad.axis, abeam );
	if( fabs( spotAlong - along ) > 80.0f )
		c.headFor( abeam );
	else
		c.headFor( pad.spot );
	c.view[PITCH] = groundPitch;
	c.cmd.forwardmove = moveNone;
	if( c.speed > taxiSpeed )
	{
		wheelBrakes( c, true );
		c.cmd.forwardmove = -moveFull;
		return Status::Running;
	}
	if( spotDist < 50 )
	{
		wheelBrakes( c, true );
		if( c.speed > 1 )
		{
			c.cmd.forwardmove = -moveFull;
			return Status::Running;
		}
		return Status::Success;
	}
	wheelBrakes( c, false );
	if( c.speed < 35 )
		c.cmd.forwardmove = moveFull;
	return Status::Running;
}

// sit still, brakes on, until fuel and weapons are full
Status LandingSteps::refuel( Bots::Mind& c )
{
	if( c.bot.rearm.pad < 0 || liftedOff( c ) )
		return Status::Failure;
	c.bot.rearm.phase = Ground;
	c.controls.claim();
	c.view[YAW] = c.ps.vehicleAngles[YAW];
	c.view[PITCH] = groundPitch;
	c.cmd.forwardmove = moveNone;
	wheelBrakes( c, true );
	return Bots::replenished( c.ent ) ? Status::Success : Status::Running;
}

// done: hand the bot back to its goals
Status LandingSteps::leaveRunway( Bots::Mind& c )
{
	c.controls.claim();
	c.cmd.forwardmove = moveNone;
	Bots::finishRearm( c.ent, c.bot );
	return Status::Success;
}
