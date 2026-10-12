#include "crew.h"

// full power is asked for all over the tree; in the air it means military power unless the afterburner is earned
behavior::Status Crew::engine( Bots::Mind& c )
{
	constexpr int militaryPower = 10;
	const float throttle = signedThrottle( c.ps, c.client->vehicle_ );
	if( c.veh.maxthrottle > militaryPower && (c.ps.ONOFF & OO_LANDED) )
	{
		// the cruise setting must not push a landed aircraft along
		if( c.cmd.forwardmove != moveFull && throttle > 0 )
			c.cmd.buttons |= BUTTON_DECREASE;
	}
	else if( c.veh.maxthrottle > militaryPower && c.cmd.forwardmove == moveFull )
	{
		if( !c.steer.afterburner )
			c.cmd.forwardmove = moveNone;
		// the afterburner key only overrides the set throttle. That is brought to military power underneath, so that
		// letting the afterburner go leaves the engine there and not at wherever it was set on the ground
		if( throttle < militaryPower )
			c.cmd.buttons |= BUTTON_INCREASE;
		else if( throttle > militaryPower )
			c.cmd.buttons |= BUTTON_DECREASE;
	}
	return behavior::Status::Success;
}
