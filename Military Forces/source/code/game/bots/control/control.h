#ifndef __BOT_CONTROL_H__
#define __BOT_CONTROL_H__

#include "../core/mind.h"

// Controllers: the keys a player would press to make the vehicle do what the steps asked for. They know how the
// vehicle answers its controls and nothing of why it is going there.

// a helicopter moves by tilting: nose down to go forward, a bank to go sideways, each by what that tilt gives in speed
struct HeloControl
{
	static constexpr float maxPitch = 30.0f;	// the forward tilt at full speed
	static constexpr float maxRoll = 40.0f;	// the bank at full sideways speed
	static constexpr float levelling = 0.75f;	// how near the wanted tilt counts as there

	// tilt to move over the ground along a heading at a speed
	static void tilt( Bots::Mind& c, float yaw, float speed );
};

#endif
