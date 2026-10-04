#include "bot_mind.h"
#include <algorithm>
#include <cmath>

using behavior::Status;

namespace
{
	enum Phase { Survey, Approach, Final, Ground };

	struct Geometry
	{
		const Bots::Pad&	pad;
		float				stall;
		float				along;
		float				side;
		float				above;
		float				before;
		float				lateral;
		float				pos;
		float				axisYaw;
		vec3_t				travel;

		Geometry( const Bots::Pad& runway, const Bots::Mind& c ) : pad( runway ), stall( static_cast<float>( c.veh.stallspeed ) )
		{
			vec3_t rel, angles;
			VectorSubtract( c.ent->r.currentOrigin, pad.center, rel );
			along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
			side = rel[1] * pad.axis[0] - rel[0] * pad.axis[1];
			above = c.ent->r.currentOrigin[2] - pad.z;
			pos = along * c.bot.landDir;
			before = -(pad.halfLength - 150.0f) - pos;
			VectorScale( pad.axis, c.bot.landDir, travel );
			vectoangles( travel, angles );
			axisYaw = angles[YAW];
			lateral = rel[0] * -travel[1] + rel[1] * travel[0];
		}
	};

	void giveUp( Bots::Mind& c, int padPenalty, int retryDelay )
	{
		if( padPenalty )
		{
			c.bot.badPad = c.bot.pad;
			c.bot.badPadTime = theLevel.time_ + padPenalty;
		}
		c.bot.pad = -1;
		c.bot.landPhase = Survey;
		c.bot.landGiveUp = theLevel.time_ + retryDelay;
		c.bot.desire = Bots::Goal::Roam;
		c.bot.goalEvalTime = 0;
		c.bot.commitTime = 0;
	}

	void wheelBrakes( Bots::Mind& c, bool on )
	{
		if( on != ((c.ps.ONOFF & OO_SPEEDBRAKE) != 0) )
			c.cmd.buttons |= BUTTON_BRAKE;
	}

	bool landed( const Bots::Mind& c )
	{
		return (c.ps.ONOFF & OO_LANDED) != 0;
	}

	bool liftedOff( const Bots::Mind& c )
	{
		return !landed( c ) && c.sense.height > fabs( c.veh.mins[2] ) + c.veh.gearheight + 40.0f;
	}
}

Status LandingSteps::survey( Bots::Mind& c )
{
	if( c.bot.pad < 0 )
		return Status::Failure;
	const Bots::Pad& pad = Bots::pads_[c.bot.pad];
	c.bot.landPhase = Survey;
	c.bot.landStart = theLevel.time_;
	c.bot.landOutbound = false;

	vec3_t rel;
	VectorSubtract( c.ent->r.currentOrigin, pad.center, rel );
	const float along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
	c.bot.landDir = along > 0 ? -1.0f : 1.0f;
	if( landed( c ) )
		return Status::Success;

	float slope[2], clear[2];
	for( int k = 0; k < 2; k++ )
	{
		const float dir = k ? 1.0f : -1.0f;
		vec3_t from, out;
		trace_t tr;
		slope[k] = 0.15f;
		for( float d = 75.0f; d <= 2500.0f; d += d < 1000.0f ? 75.0f : 250.0f )
		{
			vec3_t to, boxMins = { -60, -60, 0 }, boxMaxs = { 60, 60, 4 };
			VectorMA( pad.center, -dir * (pad.halfLength - 150.0f + d), pad.axis, from );
			from[2] = pad.z + 600;
			VectorCopy( from, to );
			to[2] -= 4000;
			SV_Trace( &tr, from, boxMins, boxMaxs, to, ENTITYNUM_NONE, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
			const float rise = tr.startsolid ? 600.0f : tr.endpos[2] - pad.z;
			slope[k] = std::max( slope[k], (rise + 60.0f) / (d + 300.0f) );
		}
		VectorMA( pad.center, -dir * (pad.halfLength - 150.0f), pad.axis, from );
		from[2] = pad.z + 250;
		VectorScale( pad.axis, -dir, out );
		out[2] = slope[k];
		VectorNormalize( out );
		Bots::pathBlocked( from, out, 3500, ENTITYNUM_NONE, tr );
		clear[k] = tr.fraction * 3500.0f;
	}

	int chosen = c.bot.landDir > 0 ? 1 : 0;
	if( slope[1 - chosen] < slope[chosen] * 0.7f || (clear[chosen] < 1500 && clear[1 - chosen] > clear[chosen]) )
		chosen = 1 - chosen;
	if( slope[chosen] > 0.6f || clear[chosen] < 1500 )
	{
		giveUp( c, 300000, 3000 );
		return Status::Failure;
	}
	c.bot.landDir = chosen ? 1.0f : -1.0f;
	c.bot.landSlope = slope[chosen];
	c.bot.landRun = std::max( 900.0f, clear[chosen] - 300.0f );

	vec3_t over;
	VectorCopy( pad.center, over );
	over[2] = pad.z + 40;
	c.bot.landCeiling = Bots::ceilingAbove( over, ENTITYNUM_NONE );

	float bestLength = -1;
	for( float offset : { 45.0f, -45.0f, 120.0f, -120.0f } )
	{
		vec3_t from, lane;
		trace_t tr;
		VectorScale( pad.axis, c.bot.landDir, lane );
		VectorMA( pad.center, -(pad.halfLength - 150.0f), lane, from );
		from[0] += -lane[1] * offset;
		from[1] += lane[0] * offset;
		from[2] = pad.z + 30;
		Bots::pathBlocked( from, lane, pad.halfLength * 2.0f - 300.0f, ENTITYNUM_NONE, tr );
		if( tr.fraction > bestLength )
		{
			bestLength = tr.fraction;
			c.bot.landOffset = offset;
		}
	}
	return Status::Success;
}

Status LandingSteps::approach( Bots::Mind& c )
{
	if( c.bot.pad < 0 )
		return Status::Failure;
	if( landed( c ) )
		return Status::Success;

	const Geometry g( Bots::pads_[c.bot.pad], c );
	c.bot.landPhase = Approach;
	if( theLevel.time_ > c.bot.landStart + 120000 )
	{
		giveUp( c, 0, 90000 );
		return Status::Failure;
	}

	const float turnRate = std::max( 10.0f, c.veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	const float radius = std::max( c.speed, g.stall * 1.7f ) / turnRate;
	const float wantRun = std::clamp( radius * 5.0f + 1200.0f, 1800.0f, c.bot.landRun );
	c.bot.approachHeight = g.pad.z + std::min( std::clamp( wantRun * c.bot.landSlope * 0.75f, 300.0f, 800.0f ), std::max( 150.0f, c.bot.landCeiling - 250.0f ) );
	const float headingError = fabs( AngleSubtract( g.axisYaw, c.ps.vehicleAngles[YAW] ) );

	if( c.bot.landOutbound )
	{
		vec3_t outDir, wish = { 0, g.axisYaw + 180.0f, 0 };
		trace_t tr;
		AngleVectors( wish, outDir, nullptr, nullptr );
		if( g.before > wantRun * 0.95f || Bots::pathBlocked( c.ent->r.currentOrigin, outDir, radius * 2.5f, c.ent->s.number, tr ) )
			c.bot.landOutbound = false;
	}
	else if( g.before < wantRun * 0.35f || (g.before < wantRun * 0.6f && (headingError > 60 || fabs( g.lateral ) > radius * 2.0f)) )
		c.bot.landOutbound = true;

	const float heading = c.bot.landOutbound ? g.axisYaw + 180.0f + (g.lateral >= 0 ? -20.0f : 20.0f) :
		g.axisYaw - std::clamp( static_cast<float>( atan2( g.lateral, radius * 2.5f ) * 180.0 / std::numbers::pi_v<float> ), -60.0f, 60.0f );
	vec3_t wish = { 0, heading, 0 }, dir;
	AngleVectors( wish, dir, nullptr, nullptr );
	VectorMA( c.ent->r.currentOrigin, 2500.0f, dir, c.bot.goal );
	c.bot.goal[2] = c.bot.approachHeight;
	c.bot.goalTime = theLevel.time_ + 3000;

	const bool linedUp = !c.bot.landOutbound && g.before > wantRun * 0.3f && fabs( g.lateral ) < std::max( 150.0f, radius * 0.3f ) &&
		headingError < 15 && fabs( c.ent->r.currentOrigin[2] - c.bot.approachHeight ) < 300;
	if( linedUp && !Bots::runwayBusy( c.bot.pad, c.ent ) )
		return Status::Success;

	c.headFor( c.bot.goal );
	return Status::Running;
}

Status LandingSteps::final( Bots::Mind& c )
{
	if( c.bot.pad < 0 )
		return Status::Failure;
	if( landed( c ) )
		return Status::Success;

	const Geometry g( Bots::pads_[c.bot.pad], c );
	c.bot.landPhase = Final;
	c.direct = true;
	if( g.before < -g.pad.halfLength || (g.before < -200 && g.above > 150) || fabs( g.side ) > 900 || (g.before > 300 && Bots::runwayBusy( c.bot.pad, c.ent )) )
	{
		c.direct = false;
		return Status::Failure;
	}

	vec3_t target;
	VectorMA( g.pad.center, g.pos + 900.0f, g.travel, target );
	target[0] += -g.travel[1] * c.bot.landOffset;
	target[1] += g.travel[0] * c.bot.landOffset;
	c.headFor( target );

	vec3_t under;
	VectorCopy( c.ent->r.currentOrigin, under );
	under[2] = g.pad.z;
	const bool overRunway = Bots::runwayAt( under );
	const float touch = fabs( c.veh.mins[2] ) + c.veh.gearheight;
	const bool shortField = g.pad.halfLength < 500;
	const float wantAbove = overRunway ? touch : std::clamp( g.before * c.bot.landSlope, shortField ? touch + 22.0f : 120.0f, 800.0f );
	c.view[PITCH] = std::clamp( (g.above - wantAbove) / 6.0f, -10.0f, 14.0f );
	if( overRunway )
		c.view[PITCH] = g.above > touch + 30 ? std::clamp( (g.above - touch) / 6.0f, 2.0f, 10.0f ) : 1.5f;
	else
	{
		vec3_t nose;
		trace_t tr;
		AngleVectors( c.ps.vehicleAngles, nose, nullptr, nullptr );
		if( c.sense.height < (shortField ? 14.0f + touch : 60.0f) ||
			(Bots::pathBlocked( c.ent->r.currentOrigin, nose, shortField ? 250.0f : 500.0f, c.ent->s.number, tr ) && tr.entityNum == ENTITYNUM_WORLD) )
			c.view[PITCH] = -12.0f;
	}

	const float wantSpeed = g.stall * (overRunway ? 1.15f : g.before > 1200 ? 1.7f : shortField ? 1.2f : 1.45f);
	c.cmd.forwardmove = c.speed < wantSpeed ? moveFull : -moveFull;
	const bool wantBrake = c.speed > wantSpeed * 1.25f;
	if( wantBrake != ((c.ps.ONOFF & OO_SPEEDBRAKE) != 0) )
		c.cmd.buttons |= BUTTON_BRAKE;
	if( !(c.ps.ONOFF & OO_GEAR) && c.speed < g.stall * SPEED_GREEN_ARC * 0.95f && theLevel.time_ > c.bot.gearTime )
	{
		c.cmd.buttons |= BUTTON_GEAR;
		c.bot.gearTime = theLevel.time_ + 4000;
	}
	return Status::Running;
}

Status LandingSteps::rollOut( Bots::Mind& c )
{
	if( c.bot.pad < 0 || liftedOff( c ) )
		return Status::Failure;
	c.bot.landPhase = Ground;
	c.direct = true;
	c.view[YAW] = c.ps.vehicleAngles[YAW];
	c.view[PITCH] = 3.0f;
	if( c.speed <= 60 )
		return Status::Success;
	wheelBrakes( c, true );
	c.cmd.forwardmove = -moveFull;
	return Status::Running;
}

Status LandingSteps::taxi( Bots::Mind& c )
{
	if( c.bot.pad < 0 || liftedOff( c ) )
		return Status::Failure;
	const Bots::Pad& pad = Bots::pads_[c.bot.pad];
	c.bot.landPhase = Ground;
	c.direct = true;

	vec3_t toSpot;
	VectorSubtract( pad.spot, c.ent->r.currentOrigin, toSpot );
	toSpot[2] = 0;
	const float spotDist = VectorLength( toSpot );
	c.headFor( pad.spot );
	c.view[PITCH] = 3.0f;
	c.cmd.forwardmove = moveNone;
	if( c.speed > 60 )
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

Status LandingSteps::refuel( Bots::Mind& c )
{
	if( c.bot.pad < 0 || liftedOff( c ) )
		return Status::Failure;
	c.bot.landPhase = Ground;
	c.direct = true;
	c.view[YAW] = c.ps.vehicleAngles[YAW];
	c.view[PITCH] = 3.0f;
	c.cmd.forwardmove = moveNone;
	wheelBrakes( c, true );
	return Bots::replenished( c.ent ) ? Status::Success : Status::Running;
}

Status LandingSteps::leaveRunway( Bots::Mind& c )
{
	c.direct = true;
	c.cmd.forwardmove = moveNone;
	Bots::finishRearm( c.ent, c.bot );
	return Status::Success;
}
