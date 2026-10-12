#include "helo.h"

behavior::Tree<Bots::Mind> HeloTasks::build()
{
	behavior::Tree<Mind> t;

	const auto* rearm = t.selector( "rearm", {
		t.when( "parked on the pad", parked, t.action( "rearm on pad", rearmOnPad ) ),
		t.when( "above the pad", abovePad, t.action( "descend to pad", descendToPad ) ),
		t.action( "fly to pad", flyToPad ) } );

	// in cover between pop-ups, the gunship and the stealth strike alike
	const auto* cover = t.when( "hiding", inCover, t.selector( "cover", {
		t.when( "knows cover", hasCover, t.action( "move to cover", moveToCover ) ),
		t.action( "sink into cover", sinkIntoCover ) } ) );

	const auto* popAndCover = t.selector( "pop up and cover", {
		cover,
		t.when( "exposed", exposed, t.action( "fire exposed", fireExposed ) ),
		t.when( "sees the enemy", seesEnemy, t.action( "expose", expose ) ),
		t.action( "pop up", popUp ) } );

	const auto* stealthStrike = t.selector( "stealth strike", {
		t.when( "at close quarters", tooClose, t.action( "break off", breakOff ) ),
		cover,
		t.when( "exposed", exposed, t.action( "strike exposed", strikeExposed ) ),
		t.when( "out of missile range", outOfStrikeRange, t.action( "stalk", stalk ) ),
		t.when( "sees the enemy", seesEnemy, t.action( "expose for strike", exposeForStrike ) ),
		t.action( "pop up", popUp ) } );

	const auto* act = t.selector( "helicopter", {
		t.when( "missile inbound", missileInbound, t.action( "dodge missile", dodgeMissile ) ),
		t.when( "wants to rearm", rearming, rearm ),
		t.when( "infantry wants a lift", passengersNear, t.action( "wait for passengers", waitForPassengers ) ),
		t.when( "team mate on foot nearby", walkerWaiting, t.action( "pick up walker", pickUpWalker ) ),
		t.when( "evading", evading, t.action( "evade", evade ) ),
		t.when( "capturing", capturing, t.action( "go to flag", goToFlag ) ),
		t.when( "carries supplies", carriesCrates, t.action( "resupply team mate", resupply ) ),
		t.when( "stealth with missiles", stealthStriking, stealthStrike ),
		t.when( "fighting", fighting, popAndCover ),
		t.when( "has an enemy", hasEnemy, t.action( "close on enemy", closeOnEnemy ) ),
		t.when( "has a leader", hasLeader, t.action( "escort", escort ) ),
		t.action( "patrol", patrol ) } );

	t.setRoot( t.always( "pilot", act, {
		t.action( "countermeasures", Crew::countermeasures ),
		t.action( "navigate", navigate ),
		t.when( "airborne", airborne, t.always( "fly", t.action( "steer", steer ), { t.action( "fire", fire ) } ) ),
		t.action( "collective", collective ),
		t.action( "weapon bay", Crew::weaponBay ) } ) );
	return t;
}

void Bots::thinkHelo( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = HeloTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };

	// attack helicopters fly low by default, transports higher; the steps change it as they need
	mind.steer.wantHeight = sense.can.gunship ? heloLowHeight : heloCruiseHeight * 0.6f;
	if( !HeloTasks::fighting( mind ) && !HeloTasks::stealthStriking( mind ) )
		bot.cover.exposed = false;
	view[YAW] = ps.vehicleAngles[YAW];
	view[PITCH] = sense.enemy ? std::clamp( AngleSubtract( sense.enemyAngles[PITCH], 0 ), -30.0f, 45.0f ) : 0.0f;

	runTree( tree, mind, "helo" );
}
