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

Envelope Envelope::of( int weapon )
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
	return e;
}

float Envelope::reachOf( int vehicle, const int* ammo, bool targetAir )
{
	if( vehicle < 0 )
		return 0.0f;
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	float reach = 0.0f;
	for( int slot = WP_MACHINEGUN; slot < WP_FLARE; slot++ )
	{
		if( !veh.weapons[slot] || ammo[slot] <= 0 )
			continue;
		const Envelope e = of( veh.weapons[slot] );
		if( targetAir ? e.air : e.surface )
			reach = std::max( reach, e.maxRange );
	}
	return reach;
}
