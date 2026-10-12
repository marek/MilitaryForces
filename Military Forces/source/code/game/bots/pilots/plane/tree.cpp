#include "plane.h"

behavior::Tree<Bots::Mind> PlaneTasks::build()
{
	behavior::Tree<Mind> t;
	const auto* hold = t.action( "hold course", holdCourse );

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
		t.when( "on the ground", onGround, t.action( "take off", takeOff ) ),
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

	const auto* flight = t.when( "in free flight", flying, t.always( "fly", t.action( "countermeasures", countermeasures ), {
		t.action( "navigate", navigate ),
		t.when( "on approach", onApproach, t.action( "approach profile", approachProfile ) ),
		t.action( "mid channel", midChannel ),
		safety,
		t.action( "keep energy", keepEnergy ),
		t.action( "fire", fire ) } ) );

	t.setRoot( t.always( "pilot", act, { flight } ) );
	return t;
}

void Bots::thinkPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = PlaneTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };

	mind.steer.cruise = planeCruiseHeight;
	if( sense.role == Role::Bomber )
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

	// turrets: look at a nearby enemy aircraft and hold the trigger, as a player does with free look held, flying
	// straight meanwhile. The positions swing round at their own rate and one that has not arrived does not fire,
	// so the look has to last: four seconds is the longest any position needs to come right round. Then a second
	// to steer in. Never while there is something to fly round, nor while landing, taking off or low
	constexpr int turretBurst = 4000, turretPause = 1000;
	// the target is the nearest enemy aircraft, whatever the bomber itself is after: a bomber's own enemy is on the ground
	GameEntity* raider = sense.airThreat && sense.airThreatDist < turretRange ? sense.airThreat : nullptr;
	if( (mind.veh.flags & VF_GUNSTATIONS) && raider && !(ps.ONOFF & OO_LANDED) && bot.desire != Goal::Rearm &&
		sense.height > mind.minHeight && ps.ammo[WP_MACHINEGUN] > 0 && BotWorld::visible( ent, ent->r.currentOrigin, raider ) )
	{
		if( theLevel.time_ > bot.turretTime + turretBurst + turretPause )
			bot.turretTime = theLevel.time_;
		if( theLevel.time_ < bot.turretTime + turretBurst && theLevel.time_ >= bot.avoid.time )
		{
			// lead the target by the bullets' flight time
			const float flight = sense.airThreatDist / std::max( 1.0f, static_cast<float>( availableWeapons[mind.veh.weapons[WP_MACHINEGUN]].muzzleVelocity ) );
			Vec3 raiderVelocity;
			VectorCopy( raider->s.pos.trDelta, raiderVelocity );
			const Vec3 aimAt = Vec3( raider->r.currentOrigin ) + raiderVelocity * flight - Vec3( ent->r.currentOrigin );
			Vec3 aim;
			vectoangles( aimAt, aim );
			// a gunner in a bucking bomber is no marksman: his aim wanders about the target, a few degrees for a
			// poor gunner and under one for the best, so that the fire walks across the target and off it again
			// rather than a whole burst going wide
			constexpr float poorGunner = 4.0f, bestGunner = 0.6f;
			constexpr float wanderYawMsec = 230.0f, wanderPitchMsec = 310.0f;
			const float spread = bestGunner + (poorGunner - bestGunner) * (1.0f - std::clamp( skill( bot ), 0.0f, 1.0f ));
			const float phase = static_cast<float>( ent->s.number ) * 1.7f;
			aim[YAW] += spread * sin( theLevel.time_ / wanderYawMsec + phase );
			aim[PITCH] += spread * sin( theLevel.time_ / wanderPitchMsec + phase * 2.0f );
			if( VehicleData::gunStationsBearing( client->vehicle_, ps.vehicleAngles, aim ) )
			{
				VectorCopy( aim, view );
				cmd.buttons |= BUTTON_FREELOOK | BUTTON_ATTACK;
			}
		}
	}

	// full power is asked for all over the tree; in the air it means military power unless the afterburner is earned
	constexpr int militaryPower = 10;
	const float throttle = signedThrottle( ps, client->vehicle_ );
	if( mind.veh.maxthrottle > militaryPower && (ps.ONOFF & OO_LANDED) )
	{
		// the cruise setting must not push a landed aircraft along
		if( cmd.forwardmove != moveFull && throttle > 0 )
			cmd.buttons |= BUTTON_DECREASE;
	}
	else if( mind.veh.maxthrottle > militaryPower && cmd.forwardmove == moveFull )
	{
		if( !PlaneTasks::wantsAfterburner( mind ) )
			cmd.forwardmove = moveNone;
		// the afterburner key only overrides the set throttle. That is brought to military power underneath, so that
		// letting the afterburner go leaves the engine there and not at wherever it was set on the ground
		if( throttle < militaryPower )
			cmd.buttons |= BUTTON_INCREASE;
		else if( throttle > militaryPower )
			cmd.buttons |= BUTTON_DECREASE;
	}
}
