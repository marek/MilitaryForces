#include "surface.h"

// nothing else to do: on to the roaming or capture goal
behavior::Status SurfaceTasks::driveToGoal( Mind& c )
{
	c.bot.cover.has = false;
	c.courseTo( c.bot.goal );
	return Status::Running;
}

// turn the wished course into one that follows the ground path round obstacles
behavior::Status SurfaceTasks::navigate( Mind& c )
{
	// a flag has to be driven right up to, round whatever stands by it: the path is followed to the end. Anything
	// else is near enough once the last stretch is a straight line
	c.followGround( c.boat, 220.0f, c.bot.desire == Goal::Capture ? 60.0f : -1.0f );
	return Status::Success;
}

// a wall, water, a drop, shore for a boat, or no progress for a while: back out and try another way
behavior::Status SurfaceTasks::watchAhead( Mind& c )
{
	vec3_t forward, flat = { 0, c.ps.vehicleAngles[YAW], 0 };
	AngleVectors( flat, forward, nullptr, nullptr );
	c.steer.blocked = wallAhead( c, forward ) || (c.boat ? shoreAhead( c, forward ) : waterOrDropAhead( c, forward ));
	if( theLevel.time_ > c.bot.stuck.checkTime && noProgress( c ) )
		c.steer.blocked = true;
	if( c.steer.blocked && theLevel.time_ > c.bot.stuck.reverseTime )
		backOut( c );
	return Status::Success;
}

// something solid ahead; a slope gentle enough to drive up is not an obstacle
bool SurfaceTasks::wallAhead( const Mind& c, const vec3_t forward )
{
	constexpr float lookAhead = 260.0f, driveable = 0.6f;
	vec3_t start;
	trace_t tr;
	VectorCopy( c.ent->r.currentOrigin, start );
	start[2] += c.veh.maxs[2] * 0.5f + 8;
	return BotWorld::pathBlocked( start, forward, lookAhead, c.ent->s.number, tr ) && tr.plane.normal[2] < driveable;
}

// a land vehicle keeps out of water and off a drop, near and a little further on
bool SurfaceTasks::waterOrDropAhead( const Mind& c, const vec3_t forward )
{
	for( float reach : { 160.0f, 380.0f } )
	{
		bool drop;
		if( waterAt( c, forward, reach, drop ) || drop )
			return true;
	}
	return false;
}

// a boat stays where there is water under the bow
bool SurfaceTasks::shoreAhead( const Mind& c, const vec3_t forward )
{
	constexpr float bow = 500.0f, draught = 6.0f;
	Vec3 ahead;
	VectorMA( c.ent->r.currentOrigin, bow, forward, ahead );
	ahead[2] = c.ent->r.currentOrigin[2] - draught;
	return !(GameHost::pointContents( ahead, c.ent->s.number ) & MASK_WATER);
}

// stuck: meant to be moving and has not covered 40 units since the last check, two and a half seconds ago
bool SurfaceTasks::noProgress( Mind& c )
{
	constexpr float covered = 40.0f;
	constexpr int checkEvery = 2500, settleAfterSpawn = 4000, settleAfterReverse = 2000;
	Bots::State& bot = c.bot;
	const bool stuck = !c.steer.hold && theLevel.time_ > bot.spawnTime + settleAfterSpawn && theLevel.time_ > bot.stuck.reverseTime + settleAfterReverse &&
		Distance( c.ent->r.currentOrigin, bot.stuck.lastOrigin ) < covered;
	VectorCopy( c.ent->r.currentOrigin, bot.stuck.lastOrigin );
	bot.stuck.checkTime = theLevel.time_ + checkEvery;
	return stuck;
}

// back out turning one way or the other. A path that led into something the path finder did not see is shut for
// every later search; somewhere else to go, unless it is after a flag, which is not given up for an obstacle
void SurfaceTasks::backOut( Mind& c )
{
	constexpr int backFor = 1800;
	Bots::State& bot = c.bot;
	if( bot.path.following && bot.path.index < bot.path.points.size() )
		GroundNav::blockAhead( Vec3( c.ent->r.currentOrigin ), bot.path.points[bot.path.index] );
	bot.stuck.reverseTime = theLevel.time_ + backFor;
	bot.stuck.turn = (rand() & 1) ? moveFull : -moveFull;
	bot.path.searchTime = 0;
	if( !bot.path.following && bot.desire != Goal::Capture )
		Bots::pickGoal( c.ent, bot, c.boat ? 6000 : 4000 );
}

// water behind: forward again if the way is open, turning as it was while backing
behavior::Status SurfaceTasks::creepForward( Mind& c )
{
	c.cmd.forwardmove = c.steer.blocked ? moveNone : moveFull;
	c.cmd.rightmove = static_cast<signed char>( c.bot.stuck.turn );
	return Status::Running;
}

behavior::Status SurfaceTasks::reverse( Mind& c )
{
	c.cmd.forwardmove = -moveFull;
	c.cmd.rightmove = static_cast<signed char>( c.bot.stuck.turn );
	return Status::Running;
}

// turn the hull onto the course; drive unless holding
behavior::Status SurfaceTasks::steer( Mind& c )
{
	Vec3 angles;
	c.steer.course[2] = 0;
	vectoangles( c.steer.course, angles );
	const float yawError = AngleSubtract( angles[YAW], c.ps.vehicleAngles[YAW] );
	if( yawError < -6 )
		c.cmd.rightmove = moveFull;
	else if( yawError > 6 )
		c.cmd.rightmove = -moveFull;
	if( !c.steer.hold )
		c.cmd.forwardmove = moveFull;
	return Status::Running;
}
