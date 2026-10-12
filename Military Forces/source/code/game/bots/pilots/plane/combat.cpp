#include "plane.h"

// nose on the enemy; near the ground only in heading, and gently in tight terrain
void PlaneTasks::aimAtEnemy( Mind& c )
{
	c.view[YAW] = c.sense.enemyAngles[YAW];
	if( c.sense.enemyAir || c.sense.height > planeMinHeight * 1.5f )
		c.view[PITCH] = std::clamp( c.sense.enemyAngles[PITCH], -50.0f, 35.0f );
	else
		c.holdAltitude();
	if( c.bot.avoid.confined )
		c.view[PITCH] = std::clamp( c.view[PITCH], -14.0f, 14.0f );
}

// weapons free and closing; after a close pass the plane flies on out before coming round
void PlaneTasks::engage( Mind& c )
{
	c.weapons.attacking = true;
	c.weapons.free = true;
	c.travelTo( c.enemyPosition() );
	if( !c.sense.enemyAir && c.sense.enemyDist < 900 )
		c.bot.pursuit.egressTime = theLevel.time_ + 5000;
	if( c.sense.enemyAir && c.sense.enemyDist < passEnds )
		c.bot.pursuit.egressTime = theLevel.time_ + (enemyHasTurrets( c ) ? 6000 : 2500);
}

// keep flying the turn and the climb or dive that the arc choice settled on
void PlaneTasks::flyChosenArc( Mind& c )
{
	aimAtEnemy( c );
	c.view[YAW] = c.ps.vehicleAngles[YAW] + c.bot.pursuit.turn;
	if( c.bot.pursuit.pitch != 0.0f )
		c.view[PITCH] = c.bot.pursuit.pitch;
	if( c.bot.avoid.confined )
		c.view[PITCH] = std::clamp( c.view[PITCH], -14.0f, 14.0f );
}

// across the missile's path, then at the last moment a hard pull the other way; the skilled wait longer for it
behavior::Status PlaneTasks::defendMissile( Mind& c )
{
	const float toLeft = fabs( AngleSubtract( c.sense.missileYaw + 90.0f, c.ps.vehicleAngles[YAW] ) );
	const float toRight = fabs( AngleSubtract( c.sense.missileYaw - 90.0f, c.ps.vehicleAngles[YAW] ) );
	if( c.sense.missileTime > 1.5f )
		c.bot.breakSide = toLeft < toRight ? 1.0f : -1.0f;
	if( c.sense.missileTime > 1.2f * (0.5f + Bots::skill( c.bot )) )
	{
		c.view[YAW] = c.sense.missileYaw + 90.0f * c.bot.breakSide;
		c.view[PITCH] = c.sense.height > planeMinHeight * 2.0f ? 8.0f : -5.0f;
	}
	else
	{
		c.view[YAW] = c.sense.missileYaw - 100.0f * c.bot.breakSide;
		c.view[PITCH] = -28.0f;
	}
	c.cmd.forwardmove = moveFull;
	c.steer.evading = true;
	return Status::Running;
}

// hit by someone unseen: turn hard to the side chosen when the hit came
behavior::Status PlaneTasks::breakAway( Mind& c )
{
	c.view[YAW] = c.bot.breakYaw;
	c.holdAltitude();
	return Status::Running;
}

// directly away from the enemy
behavior::Status PlaneTasks::flee( Mind& c )
{
	c.view[YAW] = c.sense.enemyAngles[YAW] + 180.0f;
	c.holdAltitude();
	return Status::Running;
}

// towards the enemy at the present height
behavior::Status PlaneTasks::stalk( Mind& c )
{
	c.view[YAW] = c.sense.enemyAngles[YAW];
	c.holdAltitude();
	c.travelTo( c.enemyPosition() );
	return Status::Running;
}

// the enemy is ahead and the way is clear: straight at it
behavior::Status PlaneTasks::pressAttack( Mind& c )
{
	engage( c );
	aimAtEnemy( c );
	return Status::Running;
}

// on with the chosen turn
behavior::Status PlaneTasks::flyArc( Mind& c )
{
	engage( c );
	flyChosenArc( c );
	return Status::Running;
}

// the enemy is off the nose: the short way round, level, climbing or diving, then the long way round, taking the
// first that is clear of the ground
behavior::Status PlaneTasks::chooseArc( Mind& c )
{
	const float turn = enemyTurn( c );
	const float longWay = turn > 0 ? turn - 360.0f : turn + 360.0f;
	const bool canDive = c.sense.height > planeMinHeight + 300.0f;
	const float radius = c.turnRadius();
	struct Option { float degrees; float climb; };
	const std::array<Option, 6> options = { {
		{ turn, 0.0f }, { turn, 0.25f }, { turn, -0.25f }, { longWay, 0.0f }, { longWay, 0.25f }, { longWay, -0.25f }
	} };
	for( const Option& option : options )
	{
		if( option.climb < 0 && !canDive )
			continue;
		if( !Bots::turnClear( c.ent, radius, option.degrees, option.climb ) )
			continue;
		engage( c );
		c.bot.pursuit.turn = option.degrees > 0 ? 80.0f : -80.0f;
		c.bot.pursuit.pitch = option.climb > 0 ? -14.0f : option.climb < 0 ? 14.0f : 0.0f;
		if( option.degrees != turn || option.climb != 0.0f )
		{
			c.bot.pursuit.chaseTime = theLevel.time_ + 900;
			flyChosenArc( c );
		}
		else
		{
			c.bot.pursuit.chaseTime = 0;
			aimAtEnemy( c );
		}
		return Status::Running;
	}
	return Status::Failure;
}

// no turn towards the enemy is clear: fly on, shooting only if it crosses the nose
behavior::Status PlaneTasks::holdCourse( Mind& c )
{
	engage( c );
	c.weapons.attacking = false;
	c.weapons.snapShot = fabs( enemyTurn( c ) ) < 6.0f;
	c.view[YAW] = c.ps.vehicleAngles[YAW];
	c.holdAltitude();
	return Status::Running;
}

// straight on at full power, at the height it has: distance is what gets it out of the guns it has just flown past
behavior::Status PlaneTasks::extend( Mind& c )
{
	c.view[YAW] = c.ps.vehicleAngles[YAW];
	c.holdAltitude();
	c.cmd.forwardmove = moveFull;
	return Status::Running;
}
