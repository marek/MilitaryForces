#include "helo.h"

// nose on the enemy, to stand off at gun range from an aircraft and further from the ground
void HeloTasks::faceEnemy( Mind& c )
{
	c.view[YAW] = c.sense.enemyAngles[YAW];
	c.steer.dist = c.sense.enemyDist;
	c.steer.standoff = c.sense.enemyAir ? 500.0f : 1200.0f;
	c.steer.mode = Cruise;
}

// across the missile's path, low and fast
behavior::Status HeloTasks::dodgeMissile( Mind& c )
{
	c.view[YAW] = c.sense.missileYaw + 90.0f;
	c.weapons.cover = true;
	c.steer.evading = true;
	c.steer.wantHeight = 60.0f;
	c.steer.mode = Run;
	return Status::Running;
}

// away from the enemy, or to the side when hit by someone unseen
behavior::Status HeloTasks::evade( Mind& c )
{
	c.view[YAW] = c.sense.blindsided ? c.bot.breakYaw : c.sense.enemyAngles[YAW] + 180.0f;
	c.steer.evading = true;
	c.steer.mode = Run;
	return Status::Running;
}

// to the hiding place, low, facing the enemy
behavior::Status HeloTasks::moveToCover( Mind& c )
{
	faceEnemy( c );
	Vec3 toCover;
	VectorSubtract( c.bot.cover.point, c.ent->r.currentOrigin, toCover );
	toCover[2] = 0;
	c.steer.dist = VectorLength( toCover );
	c.headFor( c.bot.cover.point );
	c.weapons.cover = true;
	c.steer.wantHeight = 110.0f;
	c.steer.mode = MoveToCover;
	return Status::Running;
}

// no hiding place known: back off low
behavior::Status HeloTasks::sinkIntoCover( Mind& c )
{
	faceEnemy( c );
	c.weapons.cover = true;
	c.steer.wantHeight = 110.0f;
	c.steer.mode = BackOff;
	return Status::Running;
}

// up in view and shooting until the time in the open is used up, then back into cover for a while
behavior::Status HeloTasks::fireExposed( Mind& c )
{
	faceEnemy( c );
	c.steer.wantHeight = std::max( c.sense.height, heloLowHeight );
	if( theLevel.time_ > c.bot.cover.popTime )
	{
		c.bot.cover.exposed = false;
		c.bot.cover.time = theLevel.time_ + 2000 + static_cast<int>( 3000 * c.who().caution );
		c.bot.cover.has = Bots::findHidePoint( c.ent, c.bot, c.sense.enemy );
	}
	return Status::Running;
}

// the enemy is in sight: start the time in the open, longer for the patient
behavior::Status HeloTasks::expose( Mind& c )
{
	c.bot.cover.exposed = true;
	c.bot.cover.popTime = theLevel.time_ + 3000 + static_cast<int>( 4000 * c.who().patience );
	return fireExposed( c );
}

// rise until the enemy comes into view
behavior::Status HeloTasks::popUp( Mind& c )
{
	faceEnemy( c );
	c.steer.wantHeight = std::min( c.sense.height + 300.0f, 900.0f );
	return Status::Running;
}

// too far to fight: fly to where the enemy is or was last seen
behavior::Status HeloTasks::closeOnEnemy( Mind& c )
{
	faceEnemy( c );
	const Vec3 enemy = c.enemyPosition();
	c.travelTo( Vec3( enemy.x(), enemy.y(), c.ent->r.currentOrigin[2] ) );
	return Status::Running;
}

// shoot unless hiding, dodging or flying a path round something
behavior::Status HeloTasks::fire( Mind& c )
{
	if( c.sense.enemy && !c.weapons.cover && !c.steer.evading && !c.bot.path.following )
		BotWeapons::fireWeapons( c.ent, c.bot, c.sense, c.cmd, c.steer.yawError );
	return Status::Success;
}
