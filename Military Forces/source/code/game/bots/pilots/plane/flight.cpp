#include "plane.h"

// when the straight line to the destination is blocked, follow the air path round the obstacle, allowing for the turn radius
behavior::Status PlaneTasks::navigate( Mind& c )
{
	State& bot = c.bot;
	if( c.steer.headingFixed || c.steer.evading )
		return Status::Success;
	if( bot.desire == Goal::Rearm && bot.rearm.phase == 1 )
		c.travelTo( Vec3( bot.goal[0], bot.goal[1], bot.landing.approachHeight ) );
	const Vec3 here( c.ent->r.currentOrigin );
	const Vec3 toDest = c.steer.dest - here;
	const float range = toDest.length();
	const Vec3 probeEnd = range > 2500.0f ? here + toDest * (2500.0f / range) : c.steer.dest;
	if( !c.steer.hasDest || range <= 300.0f || (!bot.avoid.confined && AirNav::clearLine( here, probeEnd, 70.0f )) )
	{
		bot.path.points.clear();
		bot.path.following = false;
		bot.path.cornerAhead = false;
		return Status::Success;
	}
	const float radius = c.turnRadius();
	if( theLevel.time_ >= bot.path.searchTime )
	{
		AirNav::findPath( here, c.steer.dest, bot.path.points, Vec3( c.ps.velocity ).normalized(), radius );
		bot.path.index = 0;
		bot.path.searchTime = theLevel.time_ + 1500 + ThinkRate::stagger( c.ent->s.number, 300 );
	}
	while( bot.path.index + 1 < bot.path.points.size() &&
		((bot.path.points[bot.path.index] - here).length() < radius * 1.3f || AirNav::clearLine( here, bot.path.points[bot.path.index + 1], 70.0f )) )
		bot.path.index++;
	if( bot.path.index >= bot.path.points.size() )
	{
		bot.path.following = false;
		bot.path.cornerAhead = false;
		return Status::Success;
	}

	const Vec3 leg = bot.path.points[bot.path.index] - here;
	Vec3 legDir, legAngles;
	leg.copyTo( legDir );
	vectoangles( legDir, legAngles );
	const float aimAt = c.sense.enemy ? fabs( enemyTurn( c ) ) : 180.0f;
	c.weapons.snapShot = c.weapons.attacking && aimAt < 6.0f;
	c.weapons.attacking = false;
	c.view[YAW] = legAngles[YAW];
	c.view[PITCH] = std::clamp( AngleSubtract( legAngles[PITCH], 0 ), -20.0f, 20.0f );
	bot.path.following = true;
	bot.path.cornerAhead = false;
	if( bot.path.index + 1 < bot.path.points.size() )
	{
		const Vec3 next = bot.path.points[bot.path.index + 1] - bot.path.points[bot.path.index];
		Vec3 nextDir, nextAngles;
		next.copyTo( nextDir );
		vectoangles( nextDir, nextAngles );
		bot.path.cornerAhead = fabs( AngleSubtract( nextAngles[YAW], legAngles[YAW] ) ) > 35.0f && leg.length() < radius * 3.5f;
	}
	return Status::Success;
}

// lining up to land: hold the approach height and come back to approach speed
behavior::Status PlaneTasks::approachProfile( Mind& c )
{
	// firm enough to follow the approach height as it comes down towards the runway
	c.view[PITCH] = std::clamp( (c.ent->r.currentOrigin[2] - c.bot.landing.approachHeight) / 12.0f, -25.0f, 16.0f );
	// inbound the plane is already slowing for the final: fast, it can neither come down nor turn onto the centre line
	const float limit = c.veh.stallspeed * (c.bot.landing.outbound ? 2.2f : 1.7f);
	if( c.speed > limit )
		c.cmd.forwardmove = -moveFull;
	const bool wantBrake = !c.bot.landing.outbound && c.speed > limit * 1.15f;
	if( (c.veh.caps & HC_SPEEDBRAKE) && wantBrake != ((c.ps.ONOFF & OO_SPEEDBRAKE) != 0) )
		c.cmd.buttons |= BUTTON_BRAKE;
	return Status::Success;
}

// keep to a height lane where there is little room between floor and ceiling
behavior::Status PlaneTasks::midChannel( Mind& c )
{
	// in a corridor, traffic one way flies above the middle and traffic the other way below it
	constexpr float laneOffset = 0.17f;
	const float lane = cos( c.ps.vehicleAngles[YAW] * std::numbers::pi_v<float> / 180.0f ) >= 0.0f ? 0.5f + laneOffset : 0.5f - laneOffset;
	if( c.room < c.steer.cruise * 2.0f && !(c.weapons.attacking && c.sense.enemyAir) && !onApproach( c ) )
		c.view[PITCH] = std::clamp( (c.sense.height - c.room * lane) / 20.0f, -25.0f, 20.0f );
	return Status::Success;
}

// slow down, with the air brake if need be, where the plane has to turn tighter than its speed allows
behavior::Status PlaneTasks::cornerSpeed( Mind& c )
{
	const bool canBrake = (c.veh.caps & HC_SPEEDBRAKE) != 0;
	const bool braking = (c.ps.ONOFF & OO_SPEEDBRAKE) != 0;
	if( c.bot.avoid.confined || c.bot.path.cornerAhead || theLevel.time_ < c.bot.avoid.brakeTime )
	{
		const float slow = c.veh.stallspeed * (theLevel.time_ < c.bot.avoid.brakeTime ? 1.15f : 1.25f);
		if( c.speed > slow )
			c.cmd.forwardmove = -moveFull;
		if( (c.speed > slow * 1.3f) != braking && canBrake )
			c.cmd.buttons |= BUTTON_BRAKE;
	}
	else if( braking && c.bot.desire != Goal::Rearm && canBrake )
		c.cmd.buttons |= BUTTON_BRAKE;
	return Status::Success;
}

// pull up below the least height, push down under a low ceiling
behavior::Status PlaneTasks::floorAndCeiling( Mind& c )
{
	// a plane lining up to land flies lower than the cruising floor, or it never gets down to the runway
	const float floor = onApproach( c ) ? std::min( c.minHeight, approachFloor ) : c.minHeight;
	if( c.sense.height < floor )
		c.view[PITCH] = std::min( c.view[PITCH], -35.0f );
	else if( c.ceiling < 250 )
		c.view[PITCH] = std::max( c.view[PITCH], c.sense.height > c.minHeight + 150 ? 12.0f : 0.0f );
	return Status::Success;
}

// too slow: nose down for speed where there is height to spend, no steep climbs, and out of a stall before anything else
behavior::Status PlaneTasks::keepEnergy( Mind& c )
{
	if( c.speed < c.veh.stallspeed * (c.bot.avoid.confined ? 1.08f : 1.3f) && c.sense.height > c.minHeight * 2 )
		c.view[PITCH] = std::max( c.view[PITCH], 5.0f );
	else if( c.speed < c.veh.stallspeed * 1.7f )
		c.view[PITCH] = std::max( c.view[PITCH], -10.0f );
	if( c.ps.ONOFF & OO_STALLED )
		c.view[PITCH] = c.speed > c.veh.stallspeed ? c.ps.vehicleAngles[PITCH] + 1.0f : std::max( c.view[PITCH], c.ps.vehicleAngles[PITCH] + 1.0f );
	return Status::Success;
}

// shoot when attacking or when the enemy crosses the nose; a step that needs one particular weapon keeps it selected
behavior::Status PlaneTasks::fire( Mind& c )
{
	if( !c.weapons.attacking && !c.weapons.snapShot && !c.weapons.free )
		return Status::Success;
	Vec3 fwd;
	AngleVectors( c.ps.vehicleAngles, fwd, nullptr, nullptr );
	const float aimError = acos( std::clamp( DotProduct( fwd, c.sense.enemyDir ), -1.0f, 1.0f ) ) * 180.0f / std::numbers::pi_v<float>;
	BotWeapons::fireWeapons( c.ent, c.bot, c.sense, c.cmd, aimError );
	if( c.weapons.keep >= 0 )
		c.cmd.weapon = c.weapons.keep;
	return Status::Success;
}

// the enemy this plane is after has its team's flag, or is close to a team mate who carries the other one
bool PlaneTasks::flagEmergency( const Mind& c )
{
	constexpr float carrierThreatened = 3000.0f;
	if( g_gametype.integer != GT_CTF || !c.sense.enemy || !c.sense.enemy->client_ )
		return false;
	if( c.sense.enemy->client_->ps_.objectives & (OB_REDFLAG|OB_BLUEFLAG) )
		return true;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* mate = theLevel.getEntity( i );
		if( mate == c.ent || !alive( mate ) || !OnSameTeam( c.ent, mate ) || !(mate->client_->ps_.objectives & (OB_REDFLAG|OB_BLUEFLAG)) )
			continue;
		if( Distance( mate->r.currentOrigin, c.sense.enemy->r.currentOrigin ) < carrierThreatened )
			return true;
	}
	return false;
}

// the pilot's wish for the afterburner, which the engine acts on
behavior::Status PlaneTasks::decideAfterburner( Mind& c )
{
	c.steer.afterburner = wantsAfterburner( c );
	return Status::Success;
}

// The afterburner drinks fuel and makes the aircraft fast where fast is dangerous, so it is a decision, not a
// reflex. Getting off the ground always has it; so does a moment's recovery from a stall or a slow pull-up, and
// a climb that a fight makes urgent. Beyond that it needs open air and a reason.
bool PlaneTasks::wantsAfterburner( Mind& c )
{
	const bool stalling = (c.ps.ONOFF & OO_STALLED) || c.speed < c.veh.stallspeed * 1.15f;
	// Going back to rearm is two things. The landing itself - lining up, the final, the ground - never has it.
	// The trip to the strip while that is still far off may be hurried like any other, further down
	constexpr float stripFar = 6000.0f;
	const bool rearming = c.bot.desire == Goal::Rearm;
	const bool tripToStrip = rearming && c.bot.rearm.pad >= 0 && c.bot.rearm.phase <= 1 && Distance( BotWorld::pads_[c.bot.rearm.pad].center, c.ent->r.currentOrigin ) > stripFar;
	if( rearming && !tripToStrip )
		return false;
	// the take-off roll and the first seconds of the climb out
	if( c.landed() || departing( c ) )
		return true;
	// a blip to get flying again, and thrust for a hard pull-up that the speed alone will not carry
	if( stalling || (c.view[PITCH] < -12.0f && c.speed < c.veh.stallspeed * 1.5f) )
		return true;
	// height wanted quickly, with an enemy about; a climb on the way somewhere is flown at military power
	if( c.bot.desire == Goal::Climb && c.sense.enemy )
		return true;
	// nothing more in a canyon or a corridor, or with a corner or an obstacle to fly round: speed there is a crash
	const bool tight = c.bot.avoid.confined || c.bot.path.cornerAhead || theLevel.time_ < c.bot.avoid.time || theLevel.time_ < c.bot.avoid.brakeTime;
	if( tight )
		return false;
	// shot at, or locked: some pilots light it and some do not, the cautious more often. With ground near enough
	// to get down behind it is likelier than out in the open sky. Settled once for each incident (each hit, or
	// each stretch of some seconds under a lock) so it does not flicker
	constexpr float coverBelow = 1200.0f;
	if( c.sense.underFire || c.sense.blindsided || c.sense.locked )
	{
		const int incident = c.sense.underFire ? c.bot.hitTime : theLevel.time_ / 6000;
		const int draw = (incident * 7 + c.ent->s.number * 31) % 100;
		const int chance = 25 + static_cast<int>( 40 * c.who().caution ) + (c.sense.height < coverBelow ? 20 : -10);
		if( draw < chance )
			return true;
	}
	// the rest needs open air: height in hand and room overhead
	if( c.sense.height <= c.minHeight * 1.5f || c.ceiling <= 600.0f )
		return false;
	// a missile on its way
	if( c.steer.evading || c.sense.missile || c.sense.launched )
		return true;
	// running away from someone near enough to matter
	constexpr float chased = 4000.0f;
	if( c.bot.desire == Goal::Evade && c.sense.enemy && c.sense.enemyDist < chased )
		return true;
	// the rest is hurrying, which a tank under three tenths cannot afford
	if( c.ps.stats[STAT_FUEL] < static_cast<int>( c.veh.maxfuel ) * 3 / 10 )
		return false;
	// a strip that is still far off, with the fuel to hurry there
	if( tripToStrip )
		return true;
	// the flag: a plane carries none, but it hurries after an enemy who has its team's, and to the fight when
	// its enemy is near a team mate who is carrying one
	if( flagEmergency( c ) )
		return true;
	// a wingman left behind catches up with the leader
	constexpr float farBehind = 4000.0f, farFromFight = 5000.0f;
	if( const GameEntity* leader = BotSenses::leaderOf( c.bot ); leader && Distance( leader->r.currentOrigin, c.ent->r.currentOrigin ) > farBehind )
		return true;
	// intercepting an enemy that is still far off; once near, the fight is flown at military power
	return c.sense.enemy && c.bot.desire == Goal::Attack && c.sense.enemyDist > farFromFight;
}
