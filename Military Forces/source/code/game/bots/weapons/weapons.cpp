#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "envelope.h"
#include <algorithm>
#include <cmath>

// does it carry a main weapon of this type with rounds left?
bool BotWeapons::hasWeaponType( int vehicle, const playerState_t& ps, int type )
{
	for( int slot : secondaryWeaponSlots )
	{
		const int weapon = availableVehicles[vehicle].weapons[slot];
		if( weapon && ps.ammo[slot] > 0 && availableWeapons[weapon].type == type )
			return true;
	}
	return false;
}

// The main weapon to use on a target of that kind at that distance, or -1: missiles inside their range come first,
// then the gun or rockets that suit the distance. guided is set when the weapon needs a lock before it is fired.
int BotWeapons::findWeaponSlot( int vehicle, const playerState_t& ps, bool air, float dist, bool& guided )
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
		const Envelope zone = Envelope::of( weapon );
		if( !(air ? zone.air : zone.surface) )
			continue;
		const float range = zone.maxRange;
		float score = 0;
		switch( data.type )
		{
		case WT_ANTIAIRMISSILE:
			score = dist < zone.minRange ? 500.0f : dist <= range ? 10000.0f - range : 2000.0f + range * 0.01f;
			break;
		case WT_ANTIGROUNDMISSILE:
		case WT_ANTIRADARMISSILE:
			score = dist <= range ? 9000.0f - range * 0.1f : 2000.0f;
			break;
		case WT_ROCKET:
			score = zone.reaches( dist, air ) ? 6000.0f : 1000.0f;
			break;
		case WT_BALLISTICGUN:
			score = zone.reaches( dist, air ) ? 9500.0f : 7000.0f;
			break;
		case WT_MACHINEGUN:
			// only guns that are main weapons: a turret or a gun pod. The fixed gun has its own trigger
			if( !data.turret && !(data.fitsPylon & PF_GUNPOD) )
				continue;
			score = zone.reaches( dist, air ) ? 3000.0f : 800.0f;
			break;
		default:
			continue;
		}
		const bool lock = zone.guided;
		if( best < 0 || score > bestScore )
		{
			best = slot;
			bestScore = score;
			guided = lock;
		}
	}
	return best;
}

// every weapon it carries is down to a fifth or less
bool BotWeapons::outOfAmmo( int vehicle, const playerState_t& ps )
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

// switch the radar to the mode that sees this kind of target, if the vehicle has it
void BotWeapons::setRadar( GameEntity* ent, Bots::State& bot, bool air )
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

// Select and fire at the sensed enemy: nothing before the bot's reaction time to a new enemy has passed, nothing
// through a wall; the fixed gun when the aim is close, a guided weapon once it has its lock, the rest when lined up
void BotWeapons::fireWeapons( GameEntity* ent, Bots::State& bot, const Bots::Sense& sense, usercmd_t& cmd, float aimError )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];

	if( !sense.enemy )
		return;

	if( bot.reactEnemy != sense.enemy->s.number )
	{
		bot.reactEnemy = sense.enemy->s.number;
		bot.reactTime = theLevel.time_ + static_cast<int>( Bots::character( bot ).reaction * 1000.0f * Bots::handicap( bot ).reactionScale );
	}
	if( theLevel.time_ < bot.reactTime )
		return;

	trace_t tr;
	GameHost::trace( &tr, ent->r.currentOrigin, nullptr, nullptr, sense.enemy->r.currentOrigin, ent->s.number, MASK_SOLID, false );
	const bool clear = tr.fraction >= 1.0f || tr.entityNum == sense.enemy->s.number;

	bool guided = false;
	const int slot = BotWeapons::findWeaponSlot( client->vehicle_, ps, sense.enemyAir, sense.enemyDist, guided );
	if( slot >= 0 )
		cmd.weapon = slot;
	else
		cmd.weapon = ps.weaponNum;
	BotWeapons::setRadar( ent, bot, sense.enemyAir );

	if( !clear )
		return;

	// a fixed gun fires when the nose is on the target; a turret gun as far round as the turret reaches
	const int gun = veh.weapons[WP_MACHINEGUN];
	if( gun && ps.ammo[WP_MACHINEGUN] > 0 && Envelope::of( gun ).contains( sense.enemyDist, aimError, sense.enemyAir ) )
		cmd.buttons |= BUTTON_ATTACK;

	// the selection takes a moment to change: nothing is fired from the old one
	if( slot < 0 || ps.weaponNum != slot )
		return;

	if( guided )
	{
		if( (ps.stats[STAT_LOCKINFO] & LI_LOCKING) && ent->tracktarget_ == sense.enemy )
			cmd.buttons |= BUTTON_ATTACK_MAIN;
		return;
	}

	// rockets and shells splash: hold them when a team mate is next to the target
	const float splash = static_cast<float>( availableWeapons[veh.weapons[slot]].damageRadius );
	if( splash > 0 && BotWeapons::friendlyNear( ent, sense.enemy->r.currentOrigin, splash + 60.0f ) )
		return;
	// a poor shot fires a little before the aim is on
	if( Envelope::of( veh.weapons[slot] ).contains( sense.enemyDist, aimError / Bots::mistakes( bot ).earlyFire, sense.enemyAir ) )
		cmd.buttons |= BUTTON_ATTACK_MAIN;
}

// a flare when a missile is launched and near; the less skilled a bot, the more often it is late
void BotWeapons::flares( GameEntity* ent, Bots::State& bot, const Bots::Sense& sense, usercmd_t& cmd )
{
	if( !sense.launched || ent->client_->ps_.ammo[WP_FLARE] <= 0 || theLevel.time_ < bot.flareTime )
		return;
	if( sense.missile && sense.missileDist > 1800 && sense.missileTime > Bots::mistakes( bot ).flareWait )
		return;
	if( random() > Bots::handicap( bot ).flareChance )
	{
		bot.flareTime = theLevel.time_ + 400;
		return;
	}
	cmd.buttons |= BUTTON_FLARE;
	bot.flareTime = theLevel.time_ + 700;
}

// what a vehicle is short of, as the crate type that would help, or 0
int BotWeapons::supplyNeed( GameEntity* who, bool urgentOnly )
{
	const playerState_t& ps = who->client_->ps_;
	const completeVehicleData_t& veh = availableVehicles[who->client_->vehicle_];
	if( ps.stats[STAT_FUEL] < ps.stats[STAT_MAX_FUEL] * (urgentOnly ? 0.4f : 0.55f) )
		return WT_FUELCRATE;
	if( who->health_ < who->client_->pers_.maxHealth_ * (urgentOnly ? 0.5f : 0.7f) )
		return WT_HEALTHCRATE;
	if( urgentOnly )
		return BotWeapons::outOfAmmo( who->client_->vehicle_, ps ) ? WT_AMMOCRATE : 0;
	// a fight uses ammunition: any weapon under two fifths is worth a crate
	for( int slot = WP_MACHINEGUN; slot < WP_FLARE; slot++ )
	{
		if( veh.weapons[slot] && veh.ammo[slot] > 0 && ps.ammo[slot] * 5 < ps.ammo[slot + 8] * 2 )
			return WT_AMMOCRATE;
	}
	return 0;
}

// a vehicle that drops crates for others
bool BotWeapons::carriesSupplies( int vehicle )
{
	for( unsigned int weapon : availableVehicles[vehicle].weapons )
	{
		const int type = availableWeapons[weapon].type;
		if( weapon && (type == WT_FUELCRATE || type == WT_AMMOCRATE || type == WT_HEALTHCRATE) )
			return true;
	}
	return false;
}

// a team mate's vehicle inside the given distance of a point
bool BotWeapons::friendlyNear( GameEntity* ent, const vec3_t point, float radius )
{
	if( g_gametype.integer < GT_TEAM )
		return false;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) || !OnSameTeam( ent, other ) )
			continue;
		if( Distance( other->r.currentOrigin, point ) < radius )
			return true;
	}
	return false;
}

// Level bombing: steer over the target and release when the bomb's fall carries it there, then leave. The
// release window widens for the less skilled.
void BotWeapons::bomb( GameEntity* ent, Bots::State& bot, const vec3_t target, usercmd_t& cmd, vec3_t view )
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

	Vec3 flat;
	VectorSubtract( target, ent->r.currentOrigin, flat );
	const float drop = -flat[2];
	flat[2] = 0;
	const float dist = VectorLength( flat );
	Vec3 angles;
	vectoangles( flat, angles );
	view[YAW] = angles[YAW];

	if( drop <= 0 || ps.weaponNum != slot )
		return;
	// how far ahead the bomb lands: its time to fall the height, at the plane's speed over the ground
	const float fall = sqrt( 2.0f * drop / DEFAULT_GRAVITY );
	const float speed = sqrt( ps.velocity[0] * ps.velocity[0] + ps.velocity[1] * ps.velocity[1] );
	const float lead = speed * fall;
	const float yawError = fabs( AngleSubtract( angles[YAW], ps.vehicleAngles[YAW] ) );
	// no release with team mates inside the blast; careless characters allow themselves less margin
	const float blast = static_cast<float>( availableWeapons[veh.weapons[slot]].damageRadius );
	const float safety = blast * (1.0f + 0.6f * Bots::character( bot ).caution) + 150.0f;
	if( yawError < 8 && fabs( dist - lead ) < Bots::handicap( bot ).bombError && !BotWeapons::friendlyNear( ent, target, safety ) )
	{
		cmd.buttons |= BUTTON_ATTACK_MAIN;
		bot.bombTime = theLevel.time_;
	}
	// past the release point: fly on out before coming round again
	if( dist < lead - 400 && theLevel.time_ > bot.bombTime + 500 )
		bot.pursuit.egressTime = theLevel.time_ + 8000;
}
