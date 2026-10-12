#ifndef __BOT_ENVELOPE_H__
#define __BOT_ENVELOPE_H__

// Where a weapon can hit from, worked out from its data: the least and greatest range, how far off the nose it can be
// fired, whether it needs a lock, how long its shot takes, and for a missile the range inside which a target cannot
// outrun it. A bot shoots when the target is inside, closes when it is outside, and when evading gets out of the
// enemy's.
struct Envelope
{
	float	minRange = 0.0f;
	float	maxRange = 0.0f;
	float	cone = 0.0f;	// degrees off the line to the target it may be fired at
	float	speed = 0.0f;	// of the shot
	float	noEscape = 0.0f;	// a missile: inside this a target cannot outrun it
	bool	air = false;	// can hit an aircraft in the air
	bool	surface = false;	// can hit what is on the ground or water
	bool	guided = false;	// needs a lock

	bool reaches( float dist, bool targetAir ) const { return (targetAir ? air : surface) && dist >= minRange && dist <= maxRange; }
	bool contains( float dist, float aimError, bool targetAir ) const { return reaches( dist, targetAir ) && aimError <= cone; }
	float flightTime( float dist ) const { return speed > 0.0f ? dist / speed : 0.0f; }

	static Envelope of( int weapon );
	// the furthest any weapon with rounds left on this vehicle reaches against a target in the air or on the surface
	static float reachOf( int vehicle, const int* ammo, bool targetAir );
};

#endif
