#ifndef __BOT_ENVELOPE_H__
#define __BOT_ENVELOPE_H__

// Where a weapon can hit from, worked out from its data: the least and greatest range, how far off the nose it can be
// fired, the heights it works at as the elevation its mount allows, whether it needs a lock, how long its shot takes,
// and for a missile the range inside which a target cannot outrun it. A bot shoots when the target is inside, closes when it is outside, and when evading gets out of the
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
	float	minElevation = -90.0f;	// degrees above the horizontal, from the shooter to the target, its mount allows
	float	maxElevation = 90.0f;

	bool reaches( float dist, bool targetAir, float elevation = 0.0f ) const
	{
		return (targetAir ? air : surface) && dist >= minRange && dist <= maxRange && elevation >= minElevation && elevation <= maxElevation;
	}
	bool contains( float dist, float aimError, bool targetAir ) const { return reaches( dist, targetAir ) && aimError <= cone; }
	float flightTime( float dist ) const { return speed > 0.0f ? dist / speed : 0.0f; }

	// a weapon's envelope; on a ground vehicle its guns are held to the gun's own elevation
	static Envelope of( int weapon, int vehicle = -1 );
	// the furthest any weapon with rounds left on this vehicle reaches against a target in the air or on the surface,
	// at that elevation from the vehicle
	static float reachOf( int vehicle, const int* ammo, bool targetAir, float elevation = 0.0f );
	// degrees above the horizontal from one point to another
	static float elevationOf( const vec3_t from, const vec3_t to );

private:
	static void limitElevation( Envelope& e, const struct completeWeaponData_t& data, int vehicle );
};

#endif
