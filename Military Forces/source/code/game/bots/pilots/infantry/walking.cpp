#include "infantry.h"

behavior::Status InfantryTasks::shoot( Mind& c )
{
	c.controls.claim();
	c.view[YAW] = c.sense.enemyAngles[YAW];
	c.view[PITCH] = std::clamp( AngleSubtract( c.sense.enemyAngles[PITCH], 0 ), -60.0f, 60.0f );
	if( c.client->ps_.ammo[WP_MACHINEGUN] > 0 )
		c.cmd.buttons |= BUTTON_ATTACK;
	return Status::Running;
}

// nothing to walk to: short walks around
behavior::Status InfantryTasks::wander( Mind& c )
{
	if( theLevel.time_ > c.bot.goalTime || Distance( c.ent->r.currentOrigin, c.bot.goal ) < 60 )
		Bots::pickGoal( c.ent, c.bot, 600 );
	c.courseTo( c.bot.goal );
	return Status::Running;
}

behavior::Status InfantryTasks::navigate( Mind& c )
{
	c.followGround( false, 24.0f, 120.0f );
	return Status::Success;
}

// face the course and walk; when not getting anywhere, sidestep for a moment and plan the path again
behavior::Status InfantryTasks::walk( Mind& c )
{
	Bots::State& bot = c.bot;
	Vec3 angles;
	c.steer.course[2] = 0;
	vectoangles( c.steer.course, angles );
	c.view[YAW] = angles[YAW];
	c.view[PITCH] = 0;
	c.cmd.forwardmove = moveFull;
	if( theLevel.time_ > bot.stuck.checkTime )
	{
		if( Distance( c.ent->r.currentOrigin, bot.stuck.lastOrigin ) < 8 && theLevel.time_ > bot.spawnTime + 3000 )
		{
			bot.stuck.reverseTime = theLevel.time_ + 1200;
			bot.stuck.turn = (rand() & 1) ? moveFull : -moveFull;
			bot.path.searchTime = 0;
		}
		VectorCopy( c.ent->r.currentOrigin, bot.stuck.lastOrigin );
		bot.stuck.checkTime = theLevel.time_ + 2000;
	}
	if( theLevel.time_ < bot.stuck.reverseTime )
		c.cmd.rightmove = static_cast<signed char>( bot.stuck.turn );
	return Status::Success;
}
