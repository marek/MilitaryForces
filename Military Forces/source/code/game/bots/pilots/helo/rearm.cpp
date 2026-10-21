#include "helo.h"

// sit on the pad, collective down, until full
behavior::Status HeloTasks::rearmOnPad( Mind& c )
{
	c.steer.wait = true;
	c.weapons.cover = true;
	c.steer.mode = Hover;
	c.steer.wantHeight = -200.0f;
	if( Bots::replenished( c.ent ) )
	{
		Bots::finishRearm( c.ent, c.bot );
		return Status::Success;
	}
	return Status::Running;
}

// over the pad: gear down and straight down
behavior::Status HeloTasks::descendToPad( Mind& c )
{
	flyTo( c, BotWorld::pads_[c.bot.rearm.pad].center );
	c.steer.hasDest = false;
	c.weapons.cover = true;
	c.steer.hold = true;
	c.steer.mode = Hover;
	c.steer.wantHeight = -200.0f;
	return Status::Running;
}

// to the pad at a moderate height, slowing as it nears
behavior::Status HeloTasks::flyToPad( Mind& c )
{
	flyTo( c, BotWorld::pads_[c.bot.rearm.pad].center );
	c.weapons.cover = true;
	c.steer.mode = ToPad;
	c.steer.wantHeight = heloCruiseHeight * 0.6f;
	return Status::Running;
}

bool HeloTasks::padTaken( Mind& c )
{
	constexpr float lookingAt = 600.0f, room = 40.0f, above = 300.0f, below = 50.0f;
	const BotWorld::Pad& pad = BotWorld::pads_[c.bot.rearm.pad];
	if( Distance( c.ent->r.currentOrigin, pad.center ) > lookingAt )
		return false;
	const float own = radiusOf( c.client->vehicle_ );
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == c.ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		const Vec3 rel = Vec3( other->r.currentOrigin ) - pad.center;
		const float aside = sqrtf( rel.x() * rel.x() + rel.y() * rel.y() );
		if( aside < own + radiusOf( other->client_->vehicle_ ) + room && rel.z() < above && rel.z() > -below )
			return true;
	}
	return false;
}

behavior::Status HeloTasks::givePadUp( Mind& c )
{
	constexpr int avoidPad = 30000;
	c.bot.rearm.avoid( c.bot.rearm.pad, theLevel.time_ + avoidPad );
	c.bot.rearm.letGo();
	c.bot.goalEvalTime = 0;
	c.steer.mode = Hover;
	return Status::Running;
}
