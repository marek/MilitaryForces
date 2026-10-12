#include "crew.h"

// an open bay shows a stealth aircraft to everyone: it is tapped shut while nothing is about to leave it
behavior::Status Crew::weaponBay( Bots::Mind& c )
{
	constexpr int bayTap = 400;
	if( c.sense.stealth && (c.ps.ONOFF & OO_WEAPONBAY) && !c.bot.cover.exposed && (theLevel.time_ / bayTap) % 2 )
		c.cmd.buttons |= BUTTON_WEAPONBAY;
	return behavior::Status::Success;
}
