#ifndef __BOT_CAPABILITIES_H__
#define __BOT_CAPABILITIES_H__

// What a vehicle can do, from its data. Shared code (scoring, senses, team play) asks this, not which kind of vehicle
// it is, so a new vehicle is a data row and not a new branch.
struct Capabilities
{
	bool	runway = false;	// flies from a runway and lands on one
	bool	hovers = false;	// a helicopter
	bool	groundStrike = false;	// built to strike the ground: its fight is its payload
	bool	gunship = false;	// an attack helicopter
	bool	armoured = false;	// a ground vehicle: shrugs off most fire
	bool	antiAirVehicle = false;	// built to shoot aircraft down
	bool	onFoot = false;
	bool	onWater = false;
	bool	turrets = false;	// gun stations a gunner works
	bool	supplies = false;	// carries crates for team mates
	bool	flares = false;	// carries flares

	static Capabilities of( int vehicle );
};

#endif
