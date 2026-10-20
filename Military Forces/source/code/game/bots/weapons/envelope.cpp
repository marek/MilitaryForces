#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "envelope.h"
#include <algorithm>

namespace
{
	constexpr float missileArming = 500.0f;	// too close for a missile to arm and turn
	constexpr float missileDefault = 3000.0f, rocketRange = 3000.0f, shellRange = 6000.0f;
	constexpr float fixedGunCone = 3.0f, rocketCone = 4.0f, shellCone = 2.0f;
	constexpr float noEscapeShare = 0.5f;
}

Envelope Envelope::of( int weapon, int vehicle )
{
	Envelope e;
	if( weapon <= 0 )
		return e;
	const completeWeaponData_t& data = availableWeapons[weapon];
	e.speed = static_cast<float>( data.muzzleVelocity );
	switch( data.type )
	{
	case WT_ANTIAIRMISSILE:
	case WT_ANTIGROUNDMISSILE:
	case WT_ANTIRADARMISSILE:
		e.guided = true;
		e.air = data.type == WT_ANTIAIRMISSILE;
		e.surface = !e.air;
		e.minRange = e.air ? missileArming : 0.0f;
		e.maxRange = data.range > 0 ? data.range : missileDefault;
		e.cone = data.lockcone > 0 ? data.lockcone : 15.0f;
		e.noEscape = e.maxRange * noEscapeShare;
		break;
	case WT_ROCKET:
		e.surface = true;
		e.maxRange = rocketRange;
		e.cone = rocketCone;
		break;
	case WT_BALLISTICGUN:
		e.surface = true;
		e.maxRange = shellRange;
		e.cone = shellCone;
		break;
	case WT_MACHINEGUN:
	case WT_FLAK:
		e.air = e.surface = true;
		e.maxRange = gunRange;
		e.cone = fixedGunCone;
		// a turret gun as far round as the turret reaches
		if( data.turret )
			e.cone = data.maxturns[YAW] > data.minturns[YAW] ? std::clamp( data.maxturns[YAW], fixedGunCone, 90.0f ) : 90.0f;
		break;
	default:
		break;
	}
	limitElevation( e, data, vehicle );
	return e;
}

// A turret's pitch limits (pitch down is positive) are the heights it can reach. A ground vehicle's guns go no higher
// or lower than its gun can be laid: up to 360 less its gear height, down to its tail angle, as its movement holds them
void Envelope::limitElevation( Envelope& e, const completeWeaponData_t& data, int vehicle )
{
	if( data.turret && data.maxturns[PITCH] > data.minturns[PITCH] )
	{
		e.minElevation = -data.maxturns[PITCH];
		e.maxElevation = -data.minturns[PITCH];
	}
	if( vehicle < 0 || e.guided || !(availableVehicles[vehicle].cat & CAT_GROUND) )
		return;
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	if( veh.gearheight > 180.0f && veh.gearheight < 360.0f )
		e.maxElevation = std::min( e.maxElevation, 360.0f - veh.gearheight );
	if( veh.tailangle > 0.0f && veh.tailangle < 90.0f )
		e.minElevation = std::max( e.minElevation, -veh.tailangle );
}

float Envelope::elevationOf( const vec3_t from, const vec3_t to )
{
	vec3_t dir;
	VectorSubtract( to, from, dir );
	const float flat = sqrtf( dir[0] * dir[0] + dir[1] * dir[1] );
	return atan2f( dir[2], std::max( flat, 1.0f ) ) * 180.0f / static_cast<float>( M_PI );
}

float Envelope::reachOf( int vehicle, const int* ammo, bool targetAir, float elevation )
{
	if( vehicle < 0 )
		return 0.0f;
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	float reach = 0.0f;
	for( int slot = WP_MACHINEGUN; slot < WP_FLARE; slot++ )
	{
		if( !veh.weapons[slot] || ammo[slot] <= 0 )
			continue;
		const Envelope e = of( veh.weapons[slot], vehicle );
		if( (targetAir ? e.air : e.surface) && elevation >= e.minElevation && elevation <= e.maxElevation )
			reach = std::max( reach, e.maxRange );
	}
	return reach;
}
