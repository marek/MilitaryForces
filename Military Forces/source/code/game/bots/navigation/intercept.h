#ifndef __BOT_INTERCEPT_H__
#define __BOT_INTERCEPT_H__

#include "../../q_shared.h"

struct GameEntity;

// Heading for where an enemy will be, not where it is: the point to aim at for a shot of a given speed, the course
// that meets a target going somewhere, and where an aircraft will be after a while, round its turn when it is banked
namespace Intercept
{
	// the target moved on by the shot's flight time, the time no longer than limit seconds
	Vec3 lead( const vec3_t from, const vec3_t target, const vec3_t velocity, float shotSpeed, float limit );
	// where to head at this speed to meet a target holding its velocity; with no meeting possible (it is faster and
	// going away), where it will be in limit seconds
	Vec3 course( const vec3_t from, float speed, const vec3_t target, const vec3_t velocity, float limit );
	// where another aircraft will be: straight on, or round its turn when it is banked hard
	void predict( GameEntity* other, float seconds, vec3_t out );
}

#endif
