#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

bool Bots::hasWeaponType( int vehicle, const playerState_t& ps, int type )
{
	for( int slot : secondaryWeaponSlots )
	{
		const int weapon = availableVehicles[vehicle].weapons[slot];
		if( weapon && ps.ammo[slot] > 0 && availableWeapons[weapon].type == type )
			return true;
	}
	return false;
}

int Bots::findWeaponSlot( int vehicle, const playerState_t& ps, bool air, float dist, bool& guided )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	int best = -1;
	float bestScore = 0;
	guided = false;
	for( int slot : secondaryWeaponSlots )
	{
		const int weapon = veh.weapons[slot];
		if( !weapon || ps.ammo[slot] <= 0 )
			continue;
		const completeWeaponData_t& data = availableWeapons[weapon];
		const float range = data.range > 0 ? static_cast<float>( data.range ) : 3000.0f;
		bool lock = false;
		float score = 0;
		switch( data.type )
		{
		case WT_ANTIAIRMISSILE:
			if( !air )
				continue;
			lock = true;
			score = dist <= range ? 10000.0f - range : 2000.0f + range * 0.01f;
			if( dist < 500 )
				score = 500.0f;
			break;
		case WT_ANTIGROUNDMISSILE:
		case WT_ANTIRADARMISSILE:
			if( air )
				continue;
			lock = true;
			score = dist <= range ? 9000.0f - range * 0.1f : 2000.0f;
			break;
		case WT_ROCKET:
			if( air )
				continue;
			score = dist < 3000 ? 6000.0f : 1000.0f;
			break;
		case WT_BALLISTICGUN:
			if( air )
				continue;
			score = dist < 5000 ? 9500.0f : 7000.0f;
			break;
		case WT_MACHINEGUN:
			if( !data.turret && !(data.fitsPylon & PF_GUNPOD) )
				continue;
			score = dist < 2500 ? 3000.0f : 800.0f;
			break;
		default:
			continue;
		}
		if( best < 0 || score > bestScore )
		{
			best = slot;
			bestScore = score;
			guided = lock;
		}
	}
	return best;
}

bool Bots::outOfAmmo( int vehicle, const playerState_t& ps )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	bool any = false;
	for( int slot = WP_MACHINEGUN; slot < WP_FLARE; slot++ )
	{
		if( !veh.weapons[slot] || veh.ammo[slot] <= 0 )
			continue;
		any = true;
		if( ps.ammo[slot] * 5 > ps.ammo[slot + 8] )
			return false;
	}
	return any;
}

void Bots::setRadar( GameEntity* ent, State& bot, bool air )
{
	const playerState_t& ps = ent->client_->ps_;
	const completeVehicleData_t& veh = availableVehicles[ent->client_->vehicle_];
	const int want = air ? OO_RADAR_AIR : OO_RADAR_GROUND;
	if( (air && veh.radarRange <= 0) || (!air && veh.radarRange2 <= 0) )
		return;
	if( (ps.ONOFF & OO_RADAR) == want || theLevel.time_ < bot.commandTime )
		return;
	bot.commandTime = theLevel.time_ + 300;
	Cmd_Radar_f( ent );
}

void Bots::fireWeapons( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, float aimError )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];

	if( !sense.enemy )
		return;

	trace_t tr;
	SV_Trace( &tr, ent->r.currentOrigin, nullptr, nullptr, sense.enemy->r.currentOrigin, ent->s.number, MASK_SOLID, false );
	const bool clear = tr.fraction >= 1.0f || tr.entityNum == sense.enemy->s.number;

	bool guided = false;
	const int slot = findWeaponSlot( client->vehicle_, ps, sense.enemyAir, sense.enemyDist, guided );
	if( slot >= 0 )
		cmd.weapon = slot;
	else
		cmd.weapon = ps.weaponNum;
	setRadar( ent, bot, sense.enemyAir );

	if( !clear )
		return;

	const int gun = veh.weapons[WP_MACHINEGUN];
	if( gun && ps.ammo[WP_MACHINEGUN] > 0 && aimError < 3.0f && sense.enemyDist < 2500 )
		cmd.buttons |= BUTTON_ATTACK;

	if( slot < 0 || ps.weaponNum != slot )
		return;

	if( guided )
	{
		if( (ps.stats[STAT_LOCKINFO] & LI_LOCKING) && ent->tracktarget_ == sense.enemy )
			cmd.buttons |= BUTTON_ATTACK_MAIN;
		return;
	}

	const int type = availableWeapons[veh.weapons[slot]].type;
	const float range = type == WT_BALLISTICGUN ? 6000.0f : 3000.0f;
	if( aimError < (type == WT_ROCKET ? 4.0f : 2.0f) && sense.enemyDist < range )
		cmd.buttons |= BUTTON_ATTACK_MAIN;
}

void Bots::flares( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd )
{
	if( !sense.launched || ent->client_->ps_.ammo[WP_FLARE] <= 0 || theLevel.time_ < bot.flareTime )
		return;
	if( sense.missile && sense.missileDist > 1800 && sense.missileTime > 2.0f )
		return;
	if( random() > skill( bot ) )
	{
		bot.flareTime = theLevel.time_ + 400;
		return;
	}
	cmd.buttons |= BUTTON_FLARE;
	bot.flareTime = theLevel.time_ + 700;
}

void Bots::bomb( GameEntity* ent, State& bot, const vec3_t target, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];

	int slot = -1;
	for( int i = WP_WEAPON1; i < WP_FLARE && slot < 0; i++ )
	{
		const int weapon = veh.weapons[i];
		if( weapon && ps.ammo[i] > 0 && (availableWeapons[weapon].type == WT_IRONBOMB || availableWeapons[weapon].type == WT_GUIDEDBOMB) )
			slot = i;
	}
	if( slot < 0 )
		return;
	cmd.weapon = slot;

	vec3_t flat;
	VectorSubtract( target, ent->r.currentOrigin, flat );
	const float drop = -flat[2];
	flat[2] = 0;
	const float dist = VectorLength( flat );
	vec3_t angles;
	vectoangles( flat, angles );
	view[YAW] = angles[YAW];

	if( drop <= 0 || ps.weaponNum != slot )
		return;
	const float fall = sqrt( 2.0f * drop / DEFAULT_GRAVITY );
	const float speed = sqrt( ps.velocity[0] * ps.velocity[0] + ps.velocity[1] * ps.velocity[1] );
	const float lead = speed * fall;
	const float yawError = fabs( AngleSubtract( angles[YAW], ps.vehicleAngles[YAW] ) );
	if( yawError < 8 && fabs( dist - lead ) < 250.0f + 400.0f * (1.0f - skill( bot )) )
	{
		cmd.buttons |= BUTTON_ATTACK_MAIN;
		bot.bombTime = theLevel.time_;
	}
	if( dist < lead - 400 && theLevel.time_ > bot.bombTime + 500 )
		bot.egressTime = theLevel.time_ + 8000;
}
