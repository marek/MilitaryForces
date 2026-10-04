#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

Bots::Goal Bots::chooseGoal( GameEntity* ent, State& bot, const Sense& sense )
{
	const Character& who = character( bot );
	const playerState_t& ps = ent->client_->ps_;
	const bool airborne = sense.role == Role::Fighter || sense.role == Role::Bomber;

	float attack = sense.enemy ? who.aggression * (0.4f + 0.6f * sense.health) : 0.0f;
	float evade = who.caution * ((1.0f - sense.health) * 1.2f + (sense.launched ? 0.5f : sense.locked ? 0.15f : 0.0f));
	float climb = 0;
	float hide = 0;
	if( sense.underFire )
		evade += who.caution * (sense.blindsided ? 1.4f : 0.3f);
	if( sense.blindsided && sense.role == Role::Tank )
		hide += who.caution * 1.2f;

	if( sense.role == Role::Bomber )
	{
		const float want = std::clamp( sense.height + ceilingAbove( ent->r.currentOrigin, ent->s.number ) - 700.0f, 800.0f, 3500.0f );
		climb = 1.2f * who.patience * std::clamp( 1.0f - sense.height / want, 0.0f, 1.0f ) + (sense.height < want * 0.6f ? 0.6f : 0.0f);
		if( sense.airThreat && sense.airThreatDist < 3000 )
			evade += who.caution * 0.8f;
		if( !hasWeaponType( ent->client_->vehicle_, ps, WT_IRONBOMB ) && !hasWeaponType( ent->client_->vehicle_, ps, WT_GUIDEDBOMB ) &&
			!hasWeaponType( ent->client_->vehicle_, ps, WT_ANTIGROUNDMISSILE ) && !hasWeaponType( ent->client_->vehicle_, ps, WT_ROCKET ) )
			attack *= 0.5f;
	}
	else if( sense.role == Role::Fighter )
	{
		if( sense.enemy && sense.enemyAir && sense.enemyDist > 3000 )
			climb = 0.5f * who.patience * std::clamp( (sense.enemy->r.currentOrigin[2] + 400 - ent->r.currentOrigin[2]) / 800.0f, 0.0f, 1.0f );
	}
	else if( sense.role == Role::Tank )
	{
		if( sense.airThreat && !hasWeaponType( ent->client_->vehicle_, ps, WT_ANTIAIRMISSILE ) )
			hide = who.caution * 1.6f * std::clamp( 1.3f - sense.airThreatDist / 5000.0f, 0.0f, 1.0f );
	}

	if( sense.stealth && sense.revealed && airborne )
		evade += who.caution * 0.6f;
	if( sense.stealth && !sense.revealed && sense.enemy && sense.enemyDist > 2500 )
		climb += 0.3f * who.patience;

	float rearm = 0;
	if( ps.stats[STAT_FUEL] < ps.stats[STAT_MAX_FUEL] * 0.35f )
		rearm = 1.0f;
	else if( sense.health < 0.45f )
		rearm = 0.2f + 0.8f * who.caution;
	else if( outOfAmmo( ent->client_->vehicle_, ps ) )
		rearm = 0.7f;
	else if( availableVehicles[ent->client_->vehicle_].ammo[WP_FLARE] > 0 && ps.ammo[WP_FLARE] <= 0 && sense.role != Role::Tank )
		rearm = 0.3f * who.caution;
	if( sense.duel )
	{
		rearm *= 1.0f - who.honour;
		evade *= 1.0f - 0.7f * who.honour;
	}
	if( bot.pad >= 0 )
		rearm = std::max( rearm, 0.9f );
	if( rearm > 0 && sense.role != Role::Infantry && findPad( ent ) < 0 )
		rearm = 0;
	if( sense.role == Role::Infantry )
		rearm = 0;

	const float keep = 1.25f + 0.25f * who.patience;
	switch( bot.desire )
	{
	case Goal::Attack: attack *= keep; break;
	case Goal::Evade: evade *= keep; break;
	case Goal::Climb: climb *= keep; break;
	case Goal::Hide: hide *= keep; break;
	default: break;
	}

	Goal goal = Goal::Roam;
	float best = 0.2f;
	if( attack > best ) { best = attack; goal = Goal::Attack; }
	if( evade > best ) { best = evade; goal = Goal::Evade; }
	if( climb > best ) { best = climb; goal = Goal::Climb; }
	if( hide > best ) { best = hide; goal = Goal::Hide; }
	if( rearm > best ) { best = rearm; goal = Goal::Rearm; }
	if( g_gametype.integer == GT_CTF && sense.role != Role::Fighter && sense.role != Role::Bomber )
	{
		const bool carrying = (ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0;
		const float capture = carrying ? 1.6f : 0.35f + 0.5f * who.teamwork;
		if( capture > best && (carrying || goal != Goal::Rearm) )
		{
			best = capture;
			goal = Goal::Capture;
		}
	}
	if( goal == Goal::Rearm )
	{
		if( bot.pad < 0 )
		{
			bot.pad = findPad( ent );
			bot.landPhase = 0;
		}
	}
	else
	{
		bot.pad = -1;
		bot.landPhase = 0;
	}
	return goal;
}

bool Bots::replenished( GameEntity* ent )
{
	const GameClient* client = ent->client_;
	if( client->ps_.stats[STAT_FUEL] < client->ps_.stats[STAT_MAX_FUEL] * 0.95f || ent->health_ < client->pers_.maxHealth_ * 0.95f )
		return false;
	for( int slot : allWeaponSlots )
	{
		if( client->ps_.ammo[slot] < client->ps_.ammo[slot + 8] )
			return false;
	}
	return true;
}

void Bots::finishRearm( GameEntity* ent, State& bot )
{
	runwayHeading( ent, bot );
	bot.pad = -1;
	bot.landPhase = 0;
	bot.goalEvalTime = 0;
	bot.commitTime = 0;
	bot.desire = Goal::Roam;
}
