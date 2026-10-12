#include "plane.h"

behavior::Tree<Bots::Mind> PlaneTasks::build()
{
	behavior::Tree<Mind> t;
	const auto* hold = t.action( "hold course", holdCourse );

	const auto* takeOff = t.setup( "take off", prepareTakeOff, t.selector( "roll", {
		t.when( "holding short", holdingShort, t.action( "hold short", holdShort ) ),
		t.always( "move", t.selector( "where to", {
			t.when( "taxiing to a spot", taxiing, t.action( "taxi to spot", taxiToSpot ) ),
			t.action( "line up and roll", lineUpAndRoll ) } ), {
			t.action( "give way", giveWay ) } ) } ) );

	const auto* land = t.sequence( "land and rearm", {
		t.action( "survey runway", LandingSteps::survey ),
		t.retry( "circuit", t.sequence( "approach and final", {
			t.action( "approach", LandingSteps::approach ),
			t.action( "final", LandingSteps::final ) } ) ),
		t.action( "roll out", LandingSteps::rollOut ),
		t.action( "taxi", LandingSteps::taxi ),
		t.action( "refuel", LandingSteps::refuel ),
		t.action( "leave runway", LandingSteps::leaveRunway ) } );

	const auto* pursue = t.selector( "pursue", {
		t.when( "on the nose", onNose, t.selector( "shot", {
			t.when( "line of fire clear", fireLineClear, t.action( "press attack", pressAttack ) ),
			hold } ) ),
		t.when( "committed to an arc", arcCommitted, t.action( "fly arc", flyArc ) ),
		t.action( "choose arc", chooseArc ),
		hold } );

	const auto* attack = t.selector( "attack", {
		t.when( "can dive bomb", canDiveBomb, t.action( "dive bomb", diveBomb ) ),
		t.when( "can level bomb", canLevelBomb, t.action( "level bomb", levelBomb ) ),
		t.when( "unseen stealth", stalking, t.action( "stalk", stalk ) ),
		pursue } );

	const auto* act = t.selector( "plane", {
		t.when( "missile inbound", missileInbound, t.action( "defend missile", defendMissile ) ),
		t.when( "wants to rearm", landing, land ),
		t.when( "on the ground", onGround, takeOff ),
		t.when( "just airborne", departing, t.action( "depart", depart ) ),
		t.when( "evading", evading, t.selector( "evade", {
			t.when( "blindsided", blindsided, t.action( "break away", breakAway ) ),
			t.action( "flee", flee ) } ) ),
		t.when( "through a pass", extending, t.action( "extend", extend ) ),
		t.when( "engaging", engaging, attack ),
		t.when( "wants height", climbing, t.action( "gain height", gainHeight ) ),
		t.when( "bomber with payload", canBombStructure, t.action( "bomb structure", bombStructure ) ),
		t.when( "has a leader and no target", wantsWingman, t.action( "wingman", wingman ) ),
		t.action( "patrol", patrol ) } );

	const auto* safety = t.always( "avoid", t.action( "plan manoeuvre", probeTerrain ), {
		t.action( "corner speed", cornerSpeed ),
		t.action( "floor and ceiling", floorAndCeiling ) } );

	const auto* flight = t.when( "in free flight", flying, t.always( "fly", t.action( "countermeasures", Crew::countermeasures ), {
		t.action( "navigate", navigate ),
		t.when( "on approach", onApproach, t.action( "approach profile", approachProfile ) ),
		t.action( "mid channel", midChannel ),
		safety,
		t.action( "keep energy", keepEnergy ),
		t.action( "fire", fire ) } ) );

	t.setRoot( t.always( "pilot", act, {
		flight,
		t.action( "gunner", Crew::gunner ),
		t.action( "afterburner", decideAfterburner ),
		t.action( "engine", Crew::engine ) } ) );
	return t;
}

void Bots::thinkPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = PlaneTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };

	mind.steer.cruise = planeCruiseHeight;
	if( sense.can.groundStrike )
		mind.steer.cruise = bomberRunHeight( sense.height, BotWorld::ceilingAbove( ent->r.currentOrigin, ent->s.number ) );
	else if( bot.desire == Goal::Climb )
		mind.steer.cruise = sense.height + 500.0f;
	else if( !sense.enemy || !sense.enemyAir )
		mind.steer.cruise = planeCruiseHeight - 800.0f * character( bot ).cunning;
	mind.airspeed = VectorLength( ps.velocity );
	mind.ceiling = BotWorld::ceilingAbove( ent->r.currentOrigin, ent->s.number );
	mind.room = sense.height + mind.ceiling;
	mind.minHeight = std::min( planeMinHeight, mind.room * 0.3f );

	cmd.forwardmove = moveFull;
	view[YAW] = ps.vehicleAngles[YAW];
	mind.holdAltitude();

	runTree( tree, mind, "plane" );
}
