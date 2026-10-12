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
	if( !(c.ps.ONOFF & OO_GEAR) )
		c.cmd.buttons |= BUTTON_GEAR;
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
