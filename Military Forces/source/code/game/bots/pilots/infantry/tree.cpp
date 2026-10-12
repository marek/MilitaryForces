#include "infantry.h"

behavior::Tree<Bots::Mind> InfantryTasks::build()
{
	behavior::Tree<Mind> t;

	const auto* act = t.selector( "soldier", {
		t.when( "vehicle in reach", rideInReach, t.action( "board", board ) ),
		t.when( "enemy close", enemyClose, t.action( "shoot", shoot ) ),
		t.when( "sees a vehicle with a seat", seesRide, t.action( "walk to vehicle", walkToRide ) ),
		t.when( "stranded", stranded, t.action( "respawn in a vehicle", respawnInVehicle ) ),
		t.when( "pad within walking distance", padAway, t.action( "walk to pad", walkToPad ) ),
		t.action( "wander", wander ) } );

	t.setRoot( t.always( "infantry", act, {
		t.when( "moving", moving, t.always( "move", t.action( "navigate", navigate ), { t.action( "walk", walk ) } ) ) } ) );
	return t;
}

void Bots::thinkInfantry( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = InfantryTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };
	InfantryTasks::findRide( mind );
	if( !mind.target )
		InfantryTasks::nearestPad( mind );
	view[YAW] = ps.viewangles[YAW];
	view[PITCH] = 0;

	runTree( tree, mind, "infantry" );
}
