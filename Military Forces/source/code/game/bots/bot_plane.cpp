#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

void Bots::runwayHeading( GameEntity* ent, State& bot )
{
	vec3_t origin;
	VectorCopy( ent->r.currentOrigin, origin );
	origin[2] -= heightAboveGround( ent->r.currentOrigin, ent->s.number );
	if( !runwayAt( origin ) )
		return;
	float best = 0;
	for( int step = 0; step < 24; step++ )
	{
		const float yaw = step * 15.0f;
		const float run = runwayRun( origin, yaw, 4000 );
		vec3_t angles = { -6.0f, yaw, 0 }, dir, start;
		AngleVectors( angles, dir, nullptr, nullptr );
		VectorCopy( origin, start );
		start[2] += 40;
		trace_t tr;
		pathBlocked( start, dir, 2500, ent->s.number, tr );
		const float score = std::min( run, 500.0f ) * 4.0f + tr.fraction * 2500.0f;
		if( run >= 150 && score > best )
		{
			best = score;
			bot.takeoffYaw = yaw;
		}
	}
	if( best > 0 )
		bot.takeoffTime = theLevel.time_ + 30000;
}

bool Bots::landPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	const Pad& pad = pads_[bot.pad];
	if( !pad.runway )
		return false;

	const playerState_t& ps = ent->client_->ps_;
	const completeVehicleData_t& veh = availableVehicles[ent->client_->vehicle_];
	const float speed = static_cast<float>( ps.speed ) / 10.0f;
	const float stall = static_cast<float>( veh.stallspeed );

	if( ps.ONOFF & OO_LANDED )
	{
		vec3_t toSpot, spotAngles;
		VectorSubtract( pad.spot, ent->r.currentOrigin, toSpot );
		toSpot[2] = 0;
		const float spotDist = VectorLength( toSpot );
		vectoangles( toSpot, spotAngles );
		view[YAW] = speed > 60 ? ps.vehicleAngles[YAW] : spotAngles[YAW];
		view[PITCH] = 0;
		cmd.forwardmove = 0;
		if( speed > 60 || spotDist < 50 )
		{
			if( speed > 1 )
				cmd.forwardmove = -127;
			else if( replenished( ent ) )
				finishRearm( ent, bot );
		}
		else if( speed < 35 )
			cmd.forwardmove = 127;
		return true;
	}

	vec3_t rel;
	VectorSubtract( ent->r.currentOrigin, pad.center, rel );
	const float along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
	const float side = rel[1] * pad.axis[0] - rel[0] * pad.axis[1];
	const float above = ent->r.currentOrigin[2] - pad.z;

	if( bot.landPhase == 0 )
	{
		bot.landDir = along > 0 ? -1.0f : 1.0f;
		float rise[2];
		for( int k = 0; k < 2; k++ )
		{
			vec3_t probe, down;
			VectorMA( pad.center, (k ? 1.0f : -1.0f) * (pad.halfLength + 250.0f), pad.axis, probe );
			probe[2] = pad.z + 1500;
			VectorCopy( probe, down );
			down[2] -= 3000;
			trace_t tr;
			SV_Trace( &tr, probe, nullptr, nullptr, down, ENTITYNUM_NONE, MASK_SOLID, false );
			rise[k] = tr.endpos[2] - pad.z;
		}
		if( rise[0] > rise[1] + 30 )
			bot.landDir = -1.0f;
		else if( rise[1] > rise[0] + 30 )
			bot.landDir = 1.0f;
		bot.landPhase = 1;
	}
	const float pos = along * bot.landDir;
	const float threshold = -(pad.halfLength - 150.0f);
	const float before = threshold - pos;

	vec3_t travel, angles, target;
	VectorScale( pad.axis, bot.landDir, travel );
	vectoangles( travel, angles );
	const float axisYaw = angles[YAW];

	cmd.forwardmove = 127;
	if( bot.landPhase == 1 )
	{
		VectorMA( pad.center, threshold - 3500.0f, travel, target );
		vec3_t dir;
		VectorSubtract( target, ent->r.currentOrigin, dir );
		dir[2] = 0;
		const float dist = VectorLength( dir );
		vectoangles( dir, angles );
		view[YAW] = angles[YAW];
		view[PITCH] = std::clamp( (above - 600.0f) / 30.0f, -30.0f, 20.0f );
		if( sense.height < planeMinHeight * 0.7f )
			view[PITCH] = -25.0f;
		if( dist < 800 || (before > 2000 && fabs( side ) < 500 && fabs( AngleSubtract( axisYaw, ps.vehicleAngles[YAW] ) ) < 25) )
			bot.landPhase = 2;
		return true;
	}

	if( before < -pad.halfLength || (before < -200 && above > 150) || fabs( side ) > 900 )
	{
		bot.landPhase = 1;
		return true;
	}

	VectorMA( pad.center, pos + 900.0f, travel, target );
	vec3_t dir;
	VectorSubtract( target, ent->r.currentOrigin, dir );
	dir[2] = 0;
	vectoangles( dir, angles );
	view[YAW] = angles[YAW];

	const float wantAbove = before > 0 ? std::clamp( before * 0.15f, 10.0f, 600.0f ) : 3.0f;
	view[PITCH] = std::clamp( (above - wantAbove) / 6.0f, -10.0f, 14.0f );
	if( above < 25 )
		view[PITCH] = -2.0f;
	if( before > 0 && sense.height < 45 )
		view[PITCH] = -8.0f;

	const float wantSpeed = stall * (before > 1200 ? 1.6f : 1.3f);
	cmd.forwardmove = speed < wantSpeed ? 127 : -127;
	if( speed > wantSpeed * 1.3f && !(ps.ONOFF & OO_SPEEDBRAKE) )
		cmd.buttons |= BUTTON_BRAKE;
	if( !(ps.ONOFF & OO_GEAR) && speed < stall * SPEED_GREEN_ARC * 0.95f )
		cmd.buttons |= BUTTON_GEAR;
	return true;
}

void Bots::thinkPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const float speed = static_cast<float>( ps.speed ) / 10.0f;

	cmd.forwardmove = 127;

	if( bot.desire == Goal::Rearm && bot.pad >= 0 && landPlane( ent, bot, sense, cmd, view ) )
		return;

	if( ps.ONOFF & OO_LANDED )
	{
		if( theLevel.time_ >= bot.takeoffTime && speed < 20 )
			runwayHeading( ent, bot );
		view[YAW] = theLevel.time_ < bot.takeoffTime ? bot.takeoffYaw : ps.vehicleAngles[YAW];
		view[PITCH] = speed > veh.stallspeed * 0.97f ? -8.0f : 0.0f;
		if( fabs( AngleSubtract( view[YAW], ps.vehicleAngles[YAW] ) ) > 6 )
			cmd.forwardmove = speed < 12 ? 127 : -127;
		return;
	}

	if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 2000 )
		pickGoal( ent, bot, 9000 );

	flares( ent, bot, sense, cmd );

	if( g_gametype.integer >= GT_TEAM && sense.role == Role::Fighter && !sense.enemy && character( bot ).teamwork > 0.5f )
	{
		GameEntity* leader = nullptr;
		float leaderDist = 0;
		for( int i = 1; i < ent->s.number; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( !alive( other ) || !OnSameTeam( ent, other ) || roleOf( other->client_->vehicle_ ) != Role::Fighter || (other->client_->ps_.ONOFF & OO_LANDED) )
				continue;
			const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
			if( dist < 9000 && (!leader || dist < leaderDist) )
			{
				leader = other;
				leaderDist = dist;
			}
		}
		bot.leader = leader ? leader->s.number : -1;
		if( leader )
		{
			vec3_t forward, right;
			AngleVectors( leader->client_->ps_.vehicleAngles, forward, right, nullptr );
			VectorMA( leader->r.currentOrigin, 600.0f, forward, bot.goal );
			VectorMA( bot.goal, 300.0f, right, bot.goal );
			bot.goalTime = theLevel.time_ + 2000;
		}
	}

	const bool bomber = sense.role == Role::Bomber;
	float cruise = planeCruiseHeight;
	if( bomber )
		cruise = std::clamp( sense.height + ceilingAbove( ent->r.currentOrigin, ent->s.number ) - 700.0f, 800.0f, 3500.0f );
	else if( bot.desire == Goal::Climb )
		cruise = sense.height + 500.0f;
	else if( !sense.enemy || !sense.enemyAir )
		cruise = planeCruiseHeight - 800.0f * character( bot ).cunning;

	vec3_t dir, angles;
	bool attacking = false;
	bool bombing = false;
	int forcedWeapon = -1;
	VectorSubtract( bot.goal, ent->r.currentOrigin, dir );
	dir[2] = 0;
	vectoangles( dir, angles );
	angles[PITCH] = 0;

	if( bot.desire == Goal::Evade && sense.blindsided )
		angles[YAW] = bot.breakYaw;
	else if( sense.enemy && bot.desire == Goal::Evade )
		angles[YAW] = sense.enemyAngles[YAW] + 180.0f;
	else if( sense.enemy && bot.desire == Goal::Attack && theLevel.time_ > bot.egressTime )
	{
		const bool hasBombs = hasWeaponType( client->vehicle_, ps, WT_IRONBOMB ) || hasWeaponType( client->vehicle_, ps, WT_GUIDEDBOMB );
		const bool diver = character( bot ).aggression > 0.8f || character( bot ).cunning > 0.75f;
		if( bomber && hasBombs && !sense.enemyAir && diver && sense.height > 900 && sense.enemyDist < sense.height * 2.2f )
		{
			VectorCopy( sense.enemyAngles, angles );
			attacking = true;
			for( int slot : secondaryWeaponSlots )
			{
				const int weapon = veh.weapons[slot];
				if( weapon && ps.ammo[slot] > 0 && (availableWeapons[weapon].type == WT_IRONBOMB || availableWeapons[weapon].type == WT_GUIDEDBOMB) )
				{
					forcedWeapon = slot;
					vec3_t nose;
					AngleVectors( ps.vehicleAngles, nose, nullptr, nullptr );
					if( ps.weaponNum == slot && DotProduct( nose, sense.enemyDir ) > 0.99f && sense.enemyDist < 1800 )
						cmd.buttons |= BUTTON_ATTACK_MAIN;
					break;
				}
			}
			if( sense.enemyDist < 1000 || sense.height < 700 )
				bot.egressTime = theLevel.time_ + 6000;
		}
		else if( bomber && hasBombs && !sense.enemyAir )
			bombing = true;
		else if( sense.stealth && !sense.revealed && sense.enemyDist > 2500 )
			angles[YAW] = sense.enemyAngles[YAW];
		else
		{
			VectorCopy( sense.enemyAngles, angles );
			attacking = true;
			if( !sense.enemyAir && sense.enemyDist < 900 )
				bot.egressTime = theLevel.time_ + 5000;
			if( sense.enemyAir && sense.enemyDist < 500 )
			{
				bot.avoidYaw = ps.vehicleAngles[YAW] + 70.0f;
				bot.avoidTime = theLevel.time_ + 1500;
				bot.egressTime = theLevel.time_ + 2500;
			}
		}
	}

	view[YAW] = angles[YAW];
	if( attacking && (sense.enemyAir || sense.height > planeMinHeight * 1.5f) )
		view[PITCH] = std::clamp( angles[PITCH], -50.0f, 35.0f );
	else
		view[PITCH] = std::clamp( (sense.height - cruise) / 30.0f, -30.0f, 20.0f );
	if( bombing )
		bomb( ent, bot, sense.enemy->r.currentOrigin, cmd, view );
	else if( bomber && !attacking && bot.desire == Goal::Roam && theLevel.time_ > bot.egressTime && sense.height > 700.0f &&
		(hasWeaponType( client->vehicle_, ps, WT_IRONBOMB ) || hasWeaponType( client->vehicle_, ps, WT_GUIDEDBOMB )) )
	{
		GameEntity* structure = findStructure( ent, bot );
		if( structure )
		{
			vec3_t target;
			for( int k = 0; k < 3; k++ )
				target[k] = (structure->r.absmin[k] + structure->r.absmax[k]) * 0.5f;
			bomb( ent, bot, target, cmd, view );
		}
	}

	vec3_t forward;
	const float travel = std::max( 1200.0f, VectorLength( ps.velocity ) * 3.0f );
	trace_t tr;
	AngleVectors( ps.vehicleAngles, forward, nullptr, nullptr );
	if( pathBlocked( ent->r.currentOrigin, forward, travel, ent->s.number, tr ) )
	{
		if( tr.entityNum != ENTITYNUM_WORLD )
		{
			if( tr.fraction * travel < 700 )
			{
				bot.avoidYaw = ps.vehicleAngles[YAW] + 50.0f;
				bot.avoidTime = theLevel.time_ + 1200;
				bot.egressTime = theLevel.time_ + 2500;
				view[PITCH] = -30.0f;
			}
		}
		else
		{
			if( fabs( tr.plane.normal[2] ) < 0.7f )
			{
				float around;
				if( steerAround( ent, view[YAW], travel, around ) )
				{
					bot.avoidYaw = around;
					bot.avoidTime = theLevel.time_ + 1200;
				}
				else
				{
					vec3_t home;
					VectorSubtract( bot.home, ent->r.currentOrigin, home );
					vectoangles( home, angles );
					bot.avoidYaw = angles[YAW];
					bot.avoidTime = theLevel.time_ + 3000;
					pickGoal( ent, bot, 9000 );
				}
			}
			else
				view[PITCH] = tr.plane.normal[2] > 0 ? -45.0f : 20.0f;
		}
	}
	if( theLevel.time_ < bot.avoidTime )
	{
		view[YAW] = bot.avoidYaw;
		if( attacking )
			view[PITCH] = std::clamp( (sense.height - cruise) / 30.0f, -30.0f, 20.0f );
	}
	if( sense.height < planeMinHeight )
		view[PITCH] = -40.0f;

	vec3_t up = { 0, 0, 1 };
	if( pathBlocked( ent->r.currentOrigin, up, 700, ent->s.number, tr ) && sense.height > planeMinHeight * 2 )
		view[PITCH] = std::max( view[PITCH], 15.0f );

	if( speed < veh.stallspeed * 1.3f && sense.height > planeMinHeight * 2 )
		view[PITCH] = std::max( view[PITCH], 5.0f );
	else if( speed < veh.stallspeed * 1.7f )
		view[PITCH] = std::max( view[PITCH], -10.0f );

	if( ps.ONOFF & OO_STALLED )
		view[PITCH] = speed > veh.stallspeed ? ps.vehicleAngles[PITCH] + 1.0f : std::max( view[PITCH], ps.vehicleAngles[PITCH] + 1.0f );

	if( attacking )
	{
		vec3_t fwd;
		AngleVectors( ps.vehicleAngles, fwd, nullptr, nullptr );
		const float aimError = acos( std::clamp( DotProduct( fwd, sense.enemyDir ), -1.0f, 1.0f ) ) * 180.0f / M_PI;
		fireWeapons( ent, bot, sense, cmd, aimError );
		if( forcedWeapon >= 0 )
			cmd.weapon = forcedWeapon;
	}
}
