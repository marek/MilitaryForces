#include "crew.h"

behavior::Status Crew::countermeasures( Bots::Mind& c )
{
	BotWeapons::flares( c.ent, c.bot, c.sense, c.cmd );
	return behavior::Status::Success;
}
