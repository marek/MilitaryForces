#include "surface.h"

// stop once the place steered for is this close
void SurfaceTasks::holdNear( Mind& c, float radius )
{
	c.steer.hold = fabs( c.steer.course[0] ) + fabs( c.steer.course[1] ) < radius;
}

// a spot out of the aircraft's sight, looked for again every few seconds
behavior::Status SurfaceTasks::driveToCover( Mind& c )
{
	if( !c.bot.cover.has || theLevel.time_ > c.bot.cover.time )
	{
		c.bot.cover.has = Bots::findHidePoint( c.ent, c.bot, c.sense.airThreat );
		c.bot.cover.time = theLevel.time_ + 4000;
	}
	if( !c.bot.cover.has )
		return Status::Failure;
	c.courseTo( c.bot.cover.point );
	holdNear( c, 90 );
	return Status::Running;
}

// no cover known: straight away from it
behavior::Status SurfaceTasks::runFromAircraft( Mind& c )
{
	VectorSubtract( c.ent->r.currentOrigin, c.sense.airThreat->r.currentOrigin, c.steer.course );
	return Status::Running;
}

// a spot from which only the turret shows to the enemy
behavior::Status SurfaceTasks::hullDown( Mind& c )
{
	if( theLevel.time_ > c.bot.cover.time )
	{
		c.bot.cover.has = Bots::findHidePoint( c.ent, c.bot, c.sense.enemy, true );
		c.bot.cover.time = theLevel.time_ + 5000;
	}
	if( !c.bot.cover.has )
		return Status::Failure;
	c.courseTo( c.bot.cover.point );
	holdNear( c, 90 );
	return Status::Running;
}
