#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

void Bots::thinkInfantry( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;

	GameEntity* ride = nullptr;
	float rideDist = 0;
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !other || !other->inuse_ || other == ent || other->health_ <= 0 )
			continue;
		bool candidate = false;
		if( other->s.eType == ET_MISC_VEHICLE && (other->s.seatMask & SEATMASK_ENTERABLE) )
			candidate = true;
		else if( other->client_ && alive( other ) && i <= theLevel.maxclients_ && other->client_->sess_.ridingClient_ < 0 &&
			!(availableVehicles[other->client_->vehicle_].cat & (CAT_LQM|CAT_PLANE)) &&
			g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) &&
			VectorLength( other->client_->ps_.velocity ) < 40 )
			candidate = true;
		if( !candidate )
			continue;
		float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		if( dist > 4000 )
			continue;
		if( other->client_ && VehicleData::vehicleHasTurret( other->client_->vehicle_ ) && !(other->s.seatMask & (1 << 1)) )
			dist *= 0.4f;
		if( !ride || dist < rideDist )
		{
			ride = other;
			rideDist = dist;
		}
	}

	vec3_t dir, angles;
	if( ride )
	{
		VectorSubtract( ride->r.currentOrigin, ent->r.currentOrigin, dir );
		if( Distance( ent->r.currentOrigin, ride->r.currentOrigin ) < g_enterRange.value * 0.9f && theLevel.time_ > bot.commandTime )
		{
			bot.commandTime = theLevel.time_ + 1500;
			Cmd_Eject_f( ent );
			return;
		}
	}
	else
	{
		if( !padsBuilt_ )
			buildPads();
		const Pad* nearest = nullptr;
		float nearestDist = 0;
		for( const Pad& pad : pads_ )
		{
			const float dist = Distance( ent->r.currentOrigin, pad.spot );
			if( !nearest || dist < nearestDist )
			{
				nearest = &pad;
				nearestDist = dist;
			}
		}
		const int patience = 20000 + static_cast<int>( 30000 * character( bot ).patience );
		if( theLevel.time_ > bot.spawnTime + patience || (!nearest && theLevel.time_ > bot.spawnTime + 8000) || nearestDist > 6000 )
		{
			int teamFlag = MF_TEAM_ANY;
			if( g_gametype.integer >= GT_TEAM )
				teamFlag = client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;
			const int vehicle = pickVehicle( teamFlag, false, &character( bot ) );
			if( vehicle >= 0 && theLevel.time_ > bot.commandTime )
			{
				bot.commandTime = theLevel.time_ + 2000;
				bot.vehicle = vehicle;
				char userinfo[MAX_INFO_STRING];
				SV_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
				Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", vehicle ) );
				SV_SetUserinfo( ent->s.number, userinfo );
				ClientUserinfoChanged( ent->s.number );
				Cmd_Kill_f( ent );
				return;
			}
		}
		if( nearest && nearestDist > 150 )
			VectorSubtract( nearest->spot, ent->r.currentOrigin, dir );
		else
		{
			if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 60 )
				pickGoal( ent, bot, 600 );
			VectorSubtract( bot.goal, ent->r.currentOrigin, dir );
		}
	}
	dir[2] = 0;
	vectoangles( dir, angles );
	view[YAW] = angles[YAW];
	view[PITCH] = 0;
	cmd.forwardmove = 127;

	if( theLevel.time_ > bot.stuckCheckTime )
	{
		if( Distance( ent->r.currentOrigin, bot.lastOrigin ) < 8 && theLevel.time_ > bot.spawnTime + 3000 )
		{
			bot.reverseTime = theLevel.time_ + 1200;
			bot.reverseTurn = (rand() & 1) ? 127 : -127;
		}
		VectorCopy( ent->r.currentOrigin, bot.lastOrigin );
		bot.stuckCheckTime = theLevel.time_ + 2000;
	}
	if( theLevel.time_ < bot.reverseTime )
		cmd.rightmove = bot.reverseTurn;

	if( sense.enemy && sense.enemyDist < 900 && !sense.enemyAir )
	{
		view[YAW] = sense.enemyAngles[YAW];
		view[PITCH] = std::clamp( AngleSubtract( sense.enemyAngles[PITCH], 0 ), -60.0f, 60.0f );
		cmd.forwardmove = 0;
		if( client->ps_.ammo[WP_MACHINEGUN] > 0 )
			cmd.buttons |= BUTTON_ATTACK;
	}
}

void Bots::thinkRider( GameEntity* ent, State& bot, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	GameEntity* pilot = theLevel.getEntity( client->sess_.ridingClient_ );
	if( !alive( pilot ) )
		return;

	VectorCopy( pilot->client_->ps_.vehicleAngles, view );
	view[ROLL] = 0;
	const bool gunner = client->sess_.seat_ == 1 && VehicleData::vehicleHasTurret( pilot->client_->vehicle_ );
	if( !gunner && !(availableVehicles[pilot->client_->vehicle_].cat & (CAT_HELO|CAT_BOAT)) )
		return;
	const float reach = gunner ? 3500.0f : 1500.0f;

	if( theLevel.time_ >= bot.enemyTime )
	{
		GameEntity* enemy = nullptr;
		float best = 0;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == ent || other == pilot || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
				continue;
			const float dist = Distance( pilot->r.currentOrigin, other->r.currentOrigin );
			if( dist < reach && (!enemy || dist < best) )
			{
				enemy = other;
				best = dist;
			}
		}
		bot.enemy = enemy ? enemy->s.number : -1;
		bot.enemyTime = theLevel.time_ + 800;
	}
	GameEntity* enemy = bot.enemy >= 0 ? theLevel.getEntity( bot.enemy ) : nullptr;
	if( !alive( enemy ) )
		return;

	vec3_t dir, angles;
	VectorSubtract( enemy->r.currentOrigin, pilot->r.currentOrigin, dir );
	vectoangles( dir, angles );
	view[YAW] = angles[YAW];
	view[PITCH] = std::clamp( AngleSubtract( angles[PITCH], 0 ), -60.0f, 60.0f );
	if( !visible( pilot, pilot->r.currentOrigin, enemy ) )
		return;
	cmd.buttons |= gunner ? BUTTON_ATTACK_MAIN : BUTTON_ATTACK;
}
