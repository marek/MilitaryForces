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
