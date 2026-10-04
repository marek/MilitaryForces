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

	if( bot.landPhase != 0 && theLevel.time_ > bot.landStart + 70000 )
	{
		bot.pad = -1;
		bot.landPhase = 0;
		bot.landGiveUp = theLevel.time_ + 90000;
		bot.desire = Goal::Roam;
		bot.goalEvalTime = 0;
		bot.commitTime = 0;
		return false;
	}
	if( bot.landPhase == 0 )
	{
		bot.landStart = theLevel.time_;
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
		float clear[2];
		for( int k = 0; k < 2; k++ )
		{
			const float dir = k ? 1.0f : -1.0f;
			vec3_t from, out;
			trace_t tr;
			VectorMA( pad.center, -dir * (pad.halfLength - 150.0f), pad.axis, from );
			from[2] = pad.z + 250;
			VectorScale( pad.axis, -dir, out );
			out[2] = 0.1f;
			VectorNormalize( out );
			pathBlocked( from, out, 3500, ENTITYNUM_NONE, tr );
			clear[k] = tr.fraction * 3500.0f;
		}
		const int chosen = bot.landDir > 0 ? 1 : 0;
		if( clear[chosen] < 1500 && clear[1 - chosen] > clear[chosen] )
			bot.landDir = -bot.landDir;
		bot.landRun = std::max( 900.0f, clear[bot.landDir > 0 ? 1 : 0] - 300.0f );
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
		VectorMA( pad.center, threshold - bot.landRun, travel, target );
		bot.approachHeight = pad.z + std::min( 600.0f, bot.landRun * 0.17f );
		target[2] = bot.approachHeight;
		VectorCopy( target, bot.goal );
		bot.goalTime = theLevel.time_ + 3000;
		vec3_t dir;
		VectorSubtract( target, ent->r.currentOrigin, dir );
		dir[2] = 0;
		const float dist = VectorLength( dir );
		const bool level = fabs( ent->r.currentOrigin[2] - bot.approachHeight ) < 300;
		if( (dist < 700 && level) || (before > bot.landRun * 0.55f && fabs( side ) < 400 && fabs( AngleSubtract( axisYaw, ps.vehicleAngles[YAW] ) ) < 25 &&
			fabs( ent->r.currentOrigin[2] - bot.approachHeight ) < 400) )
			bot.landPhase = 2;
		else
			return false;
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

	vec3_t under;
	VectorCopy( ent->r.currentOrigin, under );
	under[2] = pad.z;
	const bool overRunway = runwayAt( under );
	const float touch = fabs( veh.mins[2] ) + veh.gearheight;
	const float wantAbove = overRunway ? touch : std::clamp( before * 0.15f, 170.0f, 600.0f );
	view[PITCH] = std::clamp( (above - wantAbove) / 6.0f, -10.0f, 14.0f );
	if( overRunway )
		view[PITCH] = above > touch + 30 ? std::clamp( (above - touch) / 6.0f, 2.0f, 10.0f ) : 1.5f;
	if( !overRunway )
	{
		vec3_t nose;
		trace_t tr;
		AngleVectors( ps.vehicleAngles, nose, nullptr, nullptr );
		if( sense.height < 60 || (pathBlocked( ent->r.currentOrigin, nose, 500, ent->s.number, tr ) && tr.entityNum == ENTITYNUM_WORLD) )
			view[PITCH] = -12.0f;
	}

	const float wantSpeed = stall * (overRunway ? 1.15f : before > 1200 ? 1.7f : 1.45f);
	cmd.forwardmove = speed < wantSpeed ? 127 : -127;
	const bool wantBrake = speed > wantSpeed * 1.25f;
	if( wantBrake != ((ps.ONOFF & OO_SPEEDBRAKE) != 0) )
		cmd.buttons |= BUTTON_BRAKE;
	if( !(ps.ONOFF & OO_GEAR) && speed < stall * SPEED_GREEN_ARC * 0.95f && theLevel.time_ > bot.gearTime )
	{
		cmd.buttons |= BUTTON_GEAR;
		bot.gearTime = theLevel.time_ + 4000;
	}
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
		if( speed < veh.stallspeed * 0.5f )
		{
			vec3_t flat = { 0, ps.vehicleAngles[YAW], 0 }, forward;
			AngleVectors( flat, forward, nullptr, nullptr );
			for( int i = 1; i <= theLevel.maxclients_; i++ )
			{
				GameEntity* other = theLevel.getEntity( i );
				if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
					continue;
				vec3_t rel;
				VectorSubtract( other->r.currentOrigin, ent->r.currentOrigin, rel );
				const float dist = VectorNormalize( rel );
				if( dist < 320 && fabs( rel[2] * dist ) < 120 && DotProduct( rel, forward ) > 0.4f )
				{
					cmd.forwardmove = speed > 1 ? -127 : 0;
					break;
				}
			}
		}
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
	if( bot.desire == Goal::Rearm && bot.landPhase == 1 )
	{
		view[PITCH] = std::clamp( (ent->r.currentOrigin[2] - bot.approachHeight) / 30.0f, -25.0f, 20.0f );
		const float slow = veh.stallspeed * 2.2f;
		if( speed > slow )
			cmd.forwardmove = -127;
	}
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

	const float airspeed = VectorLength( ps.velocity );
	const float ceiling = ceilingAbove( ent->r.currentOrigin, ent->s.number );
	const float room = sense.height + ceiling;
	const float minHeight = std::min( planeMinHeight, room * 0.3f );
	if( room < cruise * 2.0f && !(attacking && sense.enemyAir) && !(bot.desire == Goal::Rearm && bot.landPhase == 1) )
		view[PITCH] = std::clamp( (sense.height - room * 0.5f) / 20.0f, -25.0f, 20.0f );

	const bool climbOut = sense.height < 150 && speed < veh.stallspeed * 1.6f;
	if( climbOut )
	{
		view[YAW] = ps.vehicleAngles[YAW];
		if( sense.height < 150 )
			view[PITCH] = -8.0f;
	}
	else
	{
		const float look = std::clamp( airspeed * 2.0f, 700.0f, 2500.0f );
		vec3_t nose;
		trace_t tr;
		AngleVectors( ps.vehicleAngles, nose, nullptr, nullptr );
		if( theLevel.time_ >= bot.probeTime )
		{
			bot.probeTime = theLevel.time_ + 150;
			bot.probeBlocked = pathBlocked( ent->r.currentOrigin, nose, look, ent->s.number, tr ) && tr.entityNum == ENTITYNUM_WORLD;
		}
		else
			bot.probeBlocked = false;
		if( bot.probeBlocked )
		{
			constexpr std::array<float, 10> offsets = { 0.0f, 25.0f, -25.0f, 50.0f, -50.0f, 80.0f, -80.0f, 120.0f, -120.0f, 180.0f };
			constexpr std::array<float, 3> pitches = { 0.0f, -25.0f, 12.0f };
			float best = -99999.0f;
			for( float offset : offsets )
			{
				for( float pitch : pitches )
				{
					vec3_t probe = { pitch, view[YAW] + offset, 0 }, dir;
					trace_t pt;
					AngleVectors( probe, dir, nullptr, nullptr );
					pathBlocked( ent->r.currentOrigin, dir, look * 2.0f, ent->s.number, pt );
					const float turn = fabs( AngleSubtract( probe[YAW], ps.vehicleAngles[YAW] ) );
					const float score = pt.fraction * look * 2.0f - turn * 6.0f - fabs( offset ) * 3.0f - fabs( pitch ) * 4.0f;
					if( score > best )
					{
						best = score;
						bot.avoidYaw = probe[YAW];
						bot.avoidPitch = pitch;
					}
				}
			}
			bot.avoidTime = theLevel.time_ + 800;
		}
		if( theLevel.time_ < bot.avoidTime )
		{
			view[YAW] = bot.avoidYaw;
			view[PITCH] = bot.avoidPitch;
		}

		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( !(availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) || (other->client_->ps_.ONOFF & OO_LANDED) )
				continue;
			vec3_t rel, relVel, miss;
			VectorSubtract( other->r.currentOrigin, ent->r.currentOrigin, rel );
			if( VectorLength( rel ) > 3000 )
				continue;
			VectorSubtract( other->client_->ps_.velocity, ps.velocity, relVel );
			const float closing = DotProduct( relVel, relVel );
			if( closing < 1.0f )
				continue;
			const float when = -DotProduct( rel, relVel ) / closing;
			if( when <= 0 || when > 2.0f )
				continue;
			VectorMA( rel, when, relVel, miss );
			if( VectorLength( miss ) > 260 )
				continue;
			vec3_t right;
			AngleVectors( ps.vehicleAngles, nullptr, right, nullptr );
			const float side = DotProduct( rel, right ) + DotProduct( miss, right );
			bot.avoidYaw = ps.vehicleAngles[YAW] + (side > 0 ? 45.0f : -45.0f);
			bot.avoidPitch = (rel[2] + miss[2] >= 0 && sense.height > minHeight + 250) ? 22.0f : -28.0f;
			bot.avoidTime = theLevel.time_ + 700;
			bot.egressTime = std::max( bot.egressTime, theLevel.time_ + 1500 );
			view[YAW] = bot.avoidYaw;
			view[PITCH] = bot.avoidPitch;
			break;
		}

		if( sense.height < minHeight )
			view[PITCH] = std::min( view[PITCH], -35.0f );
		else if( ceiling < 250 )
			view[PITCH] = std::max( view[PITCH], sense.height > minHeight + 150 ? 12.0f : 0.0f );
	}

	if( speed < veh.stallspeed * 1.3f && sense.height > minHeight * 2 )
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
