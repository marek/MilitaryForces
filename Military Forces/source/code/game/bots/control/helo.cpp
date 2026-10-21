#include "control.h"
#include <algorithm>

void HeloControl::tilt( Bots::Mind& c, float yaw, float speed )
{
	const float maxSpeed = std::max( 1.0f, static_cast<float>( c.veh.maxspeed ) );
	const float sideSpeed = maxSpeed;
	vec3_t travel = { 0, yaw, 0 }, body = { 0, c.ps.vehicleAngles[YAW], 0 }, dir, forward, right;
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
}

float HeloControl::tiltToStop( const Bots::Mind& c, float room )
{
	// it goes at the speed its tilt gives, so levelling at the pitch rate covers half that speed for tilt / rate
	// seconds, after a moment to react
	constexpr float reaction = 0.3f;
	if( room <= 0.0f )
		return 0.0f;
	const float rate = std::max( 1.0f, c.veh.turnspeed[PITCH] );
	const float perDegree = static_cast<float>( c.veh.maxspeed ) / maxPitch;
	const float a = perDegree / (2.0f * rate), b = perDegree * reaction;
	return (sqrtf( b * b + 4.0f * a * room ) - b) / (2.0f * a);
}

void HeloControl::gear( Bots::Mind& c, bool down )
{
	if( (c.veh.caps & HC_GEAR) && ((c.ps.ONOFF & OO_GEAR) != 0) != down )
		c.cmd.buttons |= BUTTON_GEAR;
}
