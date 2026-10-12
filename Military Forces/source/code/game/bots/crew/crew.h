#ifndef __BOT_CREW_H__
#define __BOT_CREW_H__

#include "../core/mind.h"

// What runs alongside the pilot every frame, as a crew would: the gunners, the engine, the countermeasures, the
// weapon bay. Each reads what the pilot's steps asked for and works its own controls; every pilot tree lists the
// ones its vehicle has among its always-running nodes.
struct Crew
{
	// a bomber's gunners: free look on the nearest enemy aircraft and the trigger held, in bursts
	static behavior::Status gunner( Bots::Mind& c );
	// a jet's throttle: military power, the afterburner only when the pilot wants it
	static behavior::Status engine( Bots::Mind& c );
	// flares against a missile on its way
	static behavior::Status countermeasures( Bots::Mind& c );
	// a stealth aircraft's bay shut again as soon as nothing is about to leave it
	static behavior::Status weaponBay( Bots::Mind& c );
};

#endif
