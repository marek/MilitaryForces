#include "../../g_local.h"
#include "../../g_level.h"
#include "intercept.h"
#include <algorithm>
#include <cmath>
#include <numbers>

Vec3 Intercept::lead( const vec3_t from, const vec3_t target, const vec3_t velocity, float shotSpeed, float limit )
{
	const float seconds = std::min( Distance( from, target ) / std::max( 1.0f, shotSpeed ), limit );
	Vec3 point;
	VectorMA( target, seconds, velocity, point );
	return point;
}

// the earliest time t at which |target + velocity t - from| = speed t
Vec3 Intercept::course( const vec3_t from, float speed, const vec3_t target, const vec3_t velocity, float limit )
{
	vec3_t rel;
	VectorSubtract( target, from, rel );
	const float a = DotProduct( velocity, velocity ) - speed * speed;
	const float b = 2.0f * DotProduct( rel, velocity );
	const float c = DotProduct( rel, rel );
	float seconds = limit;
	if( fabs( a ) < 1e-3f )
	{
		if( b < 0.0f )
			seconds = -c / b;
	}
	else
	{
		const float disc = b * b - 4.0f * a * c;
		if( disc >= 0.0f )
		{
			const float root = sqrtf( disc );
			const float t1 = (-b - root) / (2.0f * a), t2 = (-b + root) / (2.0f * a);
			const float t = t1 > 0.0f && (t1 < t2 || t2 <= 0.0f) ? t1 : t2;
			if( t > 0.0f )
				seconds = t;
		}
	}
	seconds = std::min( seconds, limit );
	Vec3 point;
	VectorMA( target, seconds, velocity, point );
	return point;
}

void Intercept::predict( GameEntity* other, float seconds, vec3_t out )
{
	const playerState_t& ps = other->client_->ps_;
	const float roll = ps.vehicleAngles[ROLL];
	const float rate = std::max( 10.0f, availableVehicles[other->client_->vehicle_].turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f;
	const float speed = sqrtf( ps.velocity[0] * ps.velocity[0] + ps.velocity[1] * ps.velocity[1] );
	if( !(availableVehicles[other->client_->vehicle_].cat & CAT_PLANE) || fabs( roll ) < 45.0f || speed < 1.0f )
	{
		VectorMA( other->r.currentOrigin, seconds, ps.velocity, out );
		return;
	}
	const float radius = speed / rate;
	const float angle = rate * seconds;
	const float side = roll > 0 ? 1.0f : -1.0f;
	const float forward[2] = { ps.velocity[0] / speed, ps.velocity[1] / speed };
	const float right[2] = { forward[1], -forward[0] };
	out[0] = other->r.currentOrigin[0] + radius * (sin( angle ) * forward[0] + side * (1.0f - cos( angle )) * right[0]);
	out[1] = other->r.currentOrigin[1] + radius * (sin( angle ) * forward[1] + side * (1.0f - cos( angle )) * right[1]);
	out[2] = other->r.currentOrigin[2] + ps.velocity[2] * seconds;
}
