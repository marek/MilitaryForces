#include "surface.h"

behavior::Tree<Bots::Mind> SurfaceTasks::build()
{
	behavior::Tree<Mind> t;

	const auto* act = t.selector( "driver", {
		t.when( "aircraft overhead", hidingFromAircraft, t.selector( "hide", {
			t.action( "drive to cover", driveToCover ),
			t.action( "run from aircraft", runFromAircraft ) } ) ),
		t.when( "cautious gunner", wantsHullDown, t.action( "hull down", hullDown ) ),
		t.when( "has a passenger", carryingPassenger, t.action( "deliver passenger", deliverPassenger ) ),
		t.when( "team mate on foot nearby", walkerWaiting, t.action( "pick up walker", pickUpWalker ) ),
		t.action( "drive to supplies", driveToSupplies ),
		t.when( "wants to rearm", rearming, t.action( "drive to pad", driveToPad ) ),
		t.when( "evading", evading, t.action( "withdraw", withdraw ) ),
		t.when( "aircraft on the ground in range", canShellAirfield, t.action( "shell airfield", shellAirfield ) ),
		t.when( "ground enemy", engaging, t.action( "engage", engage ) ),
		t.when( "has a leader", hasLeader, t.action( "convoy", convoy ) ),
		t.action( "drive to goal", driveToGoal ) } );

	const auto* drive = t.selector( "controls", {
		t.when( "water behind", waterBehind, t.action( "creep forward", creepForward ) ),
		t.when( "backing out", reversing, t.action( "reverse", reverse ) ),
		t.action( "steer", steer ) } );

	t.setRoot( t.always( "crew", act, {
		t.action( "countermeasures", Crew::countermeasures ),
		t.action( "navigate", navigate ),
		t.action( "watch ahead", watchAhead ),
		drive,
		t.when( "has a target", hasEnemy, t.action( "gunnery", gunnery ) ) } ) );
	return t;
}

void Bots::thinkSurface( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = SurfaceTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };
	mind.boat = (mind.veh.cat & CAT_BOAT) != 0;

	// a new place to roam to when the old one is reached or has gone stale
	if( bot.desire != Goal::Capture && (theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 400 ) )
		pickGoal( ent, bot, mind.boat ? 6000 : 4000 );
	view[YAW] = ps.vehicleAngles[YAW];
	view[PITCH] = 0;

	runTree( tree, mind, "surface" );
}
