#ifndef __BOT_MISTAKES_H__
#define __BOT_MISTAKES_H__

// The mistakes a bot's character makes, as people do, small enough to look human: an aggressive pilot follows a ground
// target lower than is wise, a poor shot overshoots its turn onto a target and fires a little early, an incautious
// pilot lets a missile come closer before the flares, an impatient one presses an attack with the enemy not yet
// on the nose. Everything here is worked out from the character's traits.
struct Mistakes
{
	float	fixationFloor = 1.0f;	// share of the safe floor kept while attacking something on the ground
	float	overshoot = 0.0f;	// degrees the nose swings past a target it turns onto
	float	earlyFire = 1.0f;	// on the aim window for unguided fire
	float	flareWait = 2.0f;	// seconds of a missile's flight left before the flares go
	float	pressAngle = 20.0f;	// degrees off the nose at which an attack is pressed
};

#endif
