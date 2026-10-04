#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

void Bots::thinkSurface( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const bool boat = (veh.cat & CAT_BOAT) != 0;

	if( bot.desire != Goal::Capture && (theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 400 ) )
		pickGoal( ent, bot, boat ? 6000 : 4000 );

	vec3_t dir, angles;
	bool hold = false;
	if( sense.enemy && !sense.enemyAir && bot.desire != Goal::Capture )
	{
		VectorCopy( sense.enemyDir, dir );
		hold = sense.enemyDist < 1500;
	}
	else
		VectorSubtract( bot.goal, ent->r.currentOrigin, dir );

	flares( ent, bot, sense, cmd );
	if( bot.desire == Goal::Hide && sense.airThreat )
	{
		if( !bot.hasHide || theLevel.time_ > bot.hideTime )
		{
			bot.hasHide = findHidePoint( ent, bot, sense.airThreat );
			bot.hideTime = theLevel.time_ + 4000;
		}
		if( bot.hasHide )
		{
			VectorSubtract( bot.hidePoint, ent->r.currentOrigin, dir );
			hold = fabs( dir[0] ) + fabs( dir[1] ) < 90;
		}
		else
		{
			VectorSubtract( ent->r.currentOrigin, sense.airThreat->r.currentOrigin, dir );
			hold = false;
		}
	}
	else if( bot.desire == Goal::Attack && sense.enemy && !sense.enemyAir && !boat && sense.enemyDist > 900 && sense.enemyDist < 4500 &&
		character( bot ).caution > 0.5f )
	{
		if( theLevel.time_ > bot.hideTime )
		{
			bot.hasHide = findHidePoint( ent, bot, sense.enemy, true );
			bot.hideTime = theLevel.time_ + 5000;
		}
		if( bot.hasHide )
		{
			VectorSubtract( bot.hidePoint, ent->r.currentOrigin, dir );
			hold = fabs( dir[0] ) + fabs( dir[1] ) < 90;
		}
	}
	else if( bot.desire == Goal::Rearm && bot.pad >= 0 )
	{
		VectorSubtract( pads_[bot.pad].spot, ent->r.currentOrigin, dir );
		hold = fabs( dir[0] ) + fabs( dir[1] ) < 80;
		if( hold && replenished( ent ) )
			finishRearm( ent, bot );
	}
	else
	{
		bot.hasHide = false;
		if( bot.desire == Goal::Evade && sense.enemy )
		{
			VectorNegate( sense.enemyDir, dir );
			hold = false;
		}
	}
	dir[2] = 0;
	vectoangles( dir, angles );

	float wantYaw = angles[YAW];
	vec3_t forward, flat;
	trace_t tr;
	VectorSet( flat, 0, ps.vehicleAngles[YAW], 0 );
	AngleVectors( flat, forward, nullptr, nullptr );

	bool blocked = false;
	vec3_t start;
	VectorCopy( ent->r.currentOrigin, start );
	start[2] += veh.maxs[2] * 0.5f + 8;
	if( pathBlocked( start, forward, 260, ent->s.number, tr ) && tr.plane.normal[2] < 0.6f )
		blocked = true;
	if( !boat && !blocked )
	{
		for( float reach : { 160.0f, 380.0f } )
		{
			vec3_t ahead, below;
			trace_t water;
			VectorMA( ent->r.currentOrigin, reach, forward, ahead );
			ahead[2] += 120;
			VectorCopy( ahead, below );
			below[2] -= 500;
			SV_Trace( &water, ahead, nullptr, nullptr, below, ent->s.number, MASK_SOLID|MASK_WATER, false );
			if( water.fraction < 1.0f && (water.contents & MASK_WATER) )
				blocked = true;
		}
	}
	if( boat && !blocked )
	{
		vec3_t ahead;
		VectorMA( ent->r.currentOrigin, 500, forward, ahead );
		ahead[2] = ent->r.currentOrigin[2] - 6;
		if( !(SV_PointContents( ahead, ent->s.number ) & MASK_WATER) )
			blocked = true;
	}

	if( theLevel.time_ > bot.stuckCheckTime )
	{
		if( !hold && theLevel.time_ > bot.spawnTime + 4000 && theLevel.time_ > bot.reverseTime + 2000 &&
			Distance( ent->r.currentOrigin, bot.lastOrigin ) < 40 )
			blocked = true;
		VectorCopy( ent->r.currentOrigin, bot.lastOrigin );
		bot.stuckCheckTime = theLevel.time_ + 2500;
	}
	if( blocked && theLevel.time_ > bot.reverseTime )
	{
		bot.reverseTime = theLevel.time_ + 1800;
		bot.reverseTurn = (rand() & 1) ? moveFull : -moveFull;
		pickGoal( ent, bot, boat ? 6000 : 4000 );
	}

	if( theLevel.time_ < bot.reverseTime )
	{
		cmd.forwardmove = -moveFull;
		cmd.rightmove = bot.reverseTurn;
	}
	else
	{
		const float yawError = AngleSubtract( wantYaw, ps.vehicleAngles[YAW] );
		if( yawError < -6 )
			cmd.rightmove = moveFull;
		else if( yawError > 6 )
			cmd.rightmove = -moveFull;
		if( !hold )
			cmd.forwardmove = moveFull;
	}

	if( sense.enemy )
	{
		view[YAW] = sense.enemyAngles[YAW];
		view[PITCH] = std::clamp( AngleSubtract( sense.enemyAngles[PITCH], 0 ), -60.0f, 20.0f );
		const int selected = veh.weapons[ps.weaponNum];
		if( selected && availableWeapons[selected].type == WT_BALLISTICGUN && availableWeapons[selected].muzzleVelocity > 0 )
		{
			const float shell = static_cast<float>( availableWeapons[selected].muzzleVelocity );
			const float flight = sense.enemyDist / shell;
			vec3_t aim, dirAim, aimAngles;
			VectorMA( sense.enemyVisible ? sense.enemy->r.currentOrigin : bot.seen[sense.enemy->s.number].position, flight, sense.enemy->client_->ps_.velocity, aim );
			VectorSubtract( aim, ent->r.currentOrigin, dirAim );
			const float range = VectorLength( dirAim );
			vectoangles( dirAim, aimAngles );
			const float arc = 0.5f * asin( std::clamp( DEFAULT_GRAVITY * range / (shell * shell), 0.0f, 1.0f ) ) * 180.0f / static_cast<float>( M_PI );
			view[YAW] = aimAngles[YAW];
			view[PITCH] = std::clamp( AngleSubtract( aimAngles[PITCH], 0 ) - arc, -60.0f, 20.0f );
		}
		const float turretYaw = ps.vehicleAngles[YAW] + ps.turretAngle;
		const float aimError = fabs( AngleSubtract( view[YAW], turretYaw ) );
		fireWeapons( ent, bot, sense, cmd, aimError );
	}
	else
	{
		view[YAW] = ps.vehicleAngles[YAW];
		view[PITCH] = 0;
	}
}
