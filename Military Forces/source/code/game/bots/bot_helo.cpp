#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

void Bots::thinkHelo( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];

	if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 800 )
		pickGoal( ent, bot, 5000 );

	bool wait = false;
	if( g_gametype.integer >= GT_TEAM && (ps.ONOFF & OO_LANDED) && theLevel.time_ < bot.spawnTime + 12000 )
	{
		for( int i = 1; i <= theLevel.maxclients_ && !wait; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == ent || !alive( other ) || !OnSameTeam( ent, other ) )
				continue;
			if( !(availableVehicles[other->client_->vehicle_].cat & CAT_LQM) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( Distance( ent->r.currentOrigin, other->r.currentOrigin ) < 1200 )
				wait = true;
		}
	}

	vec3_t dir, angles;
	float dist;
	if( sense.enemy )
	{
		VectorCopy( sense.enemyAngles, angles );
		dist = sense.enemyDist;
	}
	else
	{
		VectorSubtract( bot.goal, ent->r.currentOrigin, dir );
		dir[2] = 0;
		dist = VectorLength( dir );
		vectoangles( dir, angles );
		angles[PITCH] = 0;
	}
	const bool rearming = bot.desire == Goal::Rearm && bot.pad >= 0;
	bool overPad = false;
	if( rearming )
	{
		VectorSubtract( pads_[bot.pad].center, ent->r.currentOrigin, dir );
		dir[2] = 0;
		dist = VectorLength( dir );
		vectoangles( dir, angles );
		angles[PITCH] = 0;
		overPad = dist < 120;
		if( (ps.ONOFF & OO_LANDED) && overPad )
		{
			wait = true;
			if( replenished( ent ) )
				finishRearm( ent, bot );
		}
	}
	const bool evading = bot.desire == Goal::Evade && (sense.enemy || sense.blindsided);
	view[YAW] = evading ? (sense.blindsided ? bot.breakYaw : angles[YAW] + 180.0f) : angles[YAW];
	view[PITCH] = sense.enemy ? std::clamp( AngleSubtract( angles[PITCH], 0 ), -30.0f, 45.0f ) : 0.0f;
	flares( ent, bot, sense, cmd );

	const float yawError = fabs( AngleSubtract( view[YAW], ps.vehicleAngles[YAW] ) );

	float wantHeight = sense.role == Role::AttackHelo ? heloLowHeight : heloCruiseHeight * 0.6f;
	bool cover = false;
	if( sense.enemy && bot.desire == Goal::Attack && dist < 3000 )
	{
		if( theLevel.time_ < bot.hideTime )
		{
			cover = true;
			wantHeight = 110.0f;
		}
		else if( !bot.exposed )
		{
			if( sense.enemyVisible )
			{
				bot.exposed = true;
				bot.popTime = theLevel.time_ + 3000 + static_cast<int>( 4000 * character( bot ).patience );
			}
			else
				wantHeight = std::min( sense.height + 300.0f, 900.0f );
		}
		if( bot.exposed )
		{
			wantHeight = std::max( sense.height, heloLowHeight );
			if( theLevel.time_ > bot.popTime )
			{
				bot.exposed = false;
				bot.hideTime = theLevel.time_ + 2000 + static_cast<int>( 3000 * character( bot ).caution );
				bot.hasHide = findHidePoint( ent, bot, sense.enemy );
			}
		}
	}
	else
		bot.exposed = false;
	if( bot.desire == Goal::Capture && dist < 500 )
		wantHeight = 25.0f;
	if( rearming )
	{
		cover = true;
		wantHeight = overPad ? -200.0f : heloCruiseHeight * 0.6f;
		if( overPad && !(ps.ONOFF & OO_GEAR) )
			cmd.buttons |= BUTTON_GEAR;
	}
	vec3_t forward, flat;
	trace_t tr;
	VectorSet( flat, 0, ps.vehicleAngles[YAW], 0 );
	AngleVectors( flat, forward, nullptr, nullptr );
	const bool blocked = pathBlocked( ent->r.currentOrigin, forward, 900, ent->s.number, tr ) && (tr.entityNum == ENTITYNUM_WORLD || tr.fraction < 0.35f);
	if( blocked )
		wantHeight = std::min( sense.height + 400.0f, 1500.0f );

	const float throttle = signedThrottle( ps, client->vehicle_ );
	float wantThrottle = std::clamp( (wantHeight - sense.height) / 60.0f, -3.0f, static_cast<float>( veh.maxthrottle ) );
	if( wait )
		wantThrottle = 0;
	else if( ps.ONOFF & OO_LANDED )
		wantThrottle = std::max( wantThrottle, veh.maxthrottle * 0.6f );
	if( rearming && overPad && sense.height < 150 )
		wantThrottle = -1.0f;
	if( throttle < floor( wantThrottle ) )
		cmd.buttons |= BUTTON_INCREASE;
	else if( throttle > ceil( wantThrottle ) )
		cmd.buttons |= BUTTON_DECREASE;

	float wantPitch = 0;
	if( !(ps.ONOFF & OO_LANDED) && !blocked && sense.height > 90 )
	{
		const float standoff = sense.enemy ? (sense.enemyAir ? 500.0f : 1200.0f) : 0.0f;
		if( rearming )
			wantPitch = overPad ? 0.0f : (yawError < 40 ? std::clamp( dist / 60.0f, 3.0f, 14.0f ) : 0.0f);
		else if( evading )
			wantPitch = yawError < 60 ? 14.0f : 0.0f;
		else if( cover && bot.hasHide && !rearming )
		{
			vec3_t toCover, coverAngles;
			VectorSubtract( bot.hidePoint, ent->r.currentOrigin, toCover );
			toCover[2] = 0;
			vectoangles( toCover, coverAngles );
			view[YAW] = coverAngles[YAW];
			wantPitch = VectorLength( toCover ) > 150 && fabs( AngleSubtract( coverAngles[YAW], ps.vehicleAngles[YAW] ) ) < 50 ? 12.0f : 0.0f;
		}
		else if( cover )
			wantPitch = -5.0f;
		else if( yawError < 50 && dist > standoff + 300 )
			wantPitch = sense.height > 300 ? 14.0f : 9.0f;
		else if( sense.enemy && dist < standoff )
			wantPitch = -6.0f;
	}
	if( !(ps.ONOFF & OO_LANDED) )
	{
		const float pitchError = wantPitch - ps.vehicleAngles[PITCH];
		if( pitchError > 2 )
			cmd.forwardmove = moveFull;
		else if( pitchError < -2 )
			cmd.forwardmove = -moveFull;
		if( ps.vehicleAngles[ROLL] > 2 )
			cmd.rightmove = -moveFull;
		else if( ps.vehicleAngles[ROLL] < -2 )
			cmd.rightmove = moveFull;
	}

	if( sense.enemy && !(ps.ONOFF & OO_LANDED) && !cover && !evading )
		fireWeapons( ent, bot, sense, cmd, yawError );
}
