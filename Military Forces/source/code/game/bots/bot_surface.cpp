#include "bot_mind.h"

using behavior::Status;

struct SurfaceTasks
{
	using Mind = Bots::Mind;
	using Goal = Bots::Goal;

	static bool groundEnemy( const Mind& c ) { return c.sense.enemy && !c.sense.enemyAir; }

	static bool hidingFromAircraft( Mind& c ) { return c.bot.desire == Goal::Hide && c.sense.airThreat; }
	static bool wantsHullDown( Mind& c )
	{
		return c.bot.desire == Goal::Attack && groundEnemy( c ) && !c.boat && c.sense.enemyDist > 900 && c.sense.enemyDist < 4500 && c.who().caution > 0.5f;
	}
	static bool rearming( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.pad >= 0; }
	static bool evading( Mind& c ) { return c.bot.desire == Goal::Evade && c.sense.enemy; }
	static bool engaging( Mind& c ) { return groundEnemy( c ) && c.bot.desire != Goal::Capture; }
	static bool reversing( Mind& c ) { return theLevel.time_ < c.bot.reverseTime; }
	static bool hasEnemy( Mind& c ) { return c.sense.enemy != nullptr; }

	static bool waterAt( const Mind& c, const vec3_t forward, float reach, bool& drop )
	{
		vec3_t ahead, below;
		trace_t water;
		VectorMA( c.ent->r.currentOrigin, reach, forward, ahead );
		ahead[2] += 120;
		VectorCopy( ahead, below );
		below[2] -= 500;
		SV_Trace( &water, ahead, nullptr, nullptr, below, c.ent->s.number, MASK_SOLID|MASK_WATER, false );
		drop = water.fraction >= 1.0f || c.ent->r.currentOrigin[2] - water.endpos[2] > fabs( reach ) * 0.65f;
		return water.fraction < 1.0f && (water.contents & MASK_WATER);
	}

	static bool waterBehind( Mind& c )
	{
		if( c.boat || !reversing( c ) )
			return false;
		vec3_t forward, flat = { 0, c.ps.vehicleAngles[YAW], 0 };
		bool drop;
		AngleVectors( flat, forward, nullptr, nullptr );
		return waterAt( c, forward, -200.0f, drop );
	}

	static void holdNear( Mind& c, float radius )
	{
		c.hold = fabs( c.course[0] ) + fabs( c.course[1] ) < radius;
	}

	static Status driveToCover( Mind& c )
	{
		if( !c.bot.hasHide || theLevel.time_ > c.bot.hideTime )
		{
			c.bot.hasHide = Bots::findHidePoint( c.ent, c.bot, c.sense.airThreat );
			c.bot.hideTime = theLevel.time_ + 4000;
		}
		if( !c.bot.hasHide )
			return Status::Failure;
		c.courseTo( c.bot.hidePoint );
		holdNear( c, 90 );
		return Status::Running;
	}

	static Status runFromAircraft( Mind& c )
	{
		VectorSubtract( c.ent->r.currentOrigin, c.sense.airThreat->r.currentOrigin, c.course );
		return Status::Running;
	}

	static Status hullDown( Mind& c )
	{
		if( theLevel.time_ > c.bot.hideTime )
		{
			c.bot.hasHide = Bots::findHidePoint( c.ent, c.bot, c.sense.enemy, true );
			c.bot.hideTime = theLevel.time_ + 5000;
		}
		if( !c.bot.hasHide )
			return Status::Failure;
		c.courseTo( c.bot.hidePoint );
		holdNear( c, 90 );
		return Status::Running;
	}

	static Status driveToPad( Mind& c )
	{
		c.courseTo( Bots::pads_[c.bot.pad].spot );
		holdNear( c, 80 );
		if( c.hold && Bots::replenished( c.ent ) )
		{
			Bots::finishRearm( c.ent, c.bot );
			return Status::Success;
		}
		return Status::Running;
	}

	static Status withdraw( Mind& c )
	{
		c.bot.hasHide = false;
		VectorNegate( c.sense.enemyDir, c.course );
		return Status::Running;
	}

	static Status engage( Mind& c )
	{
		c.bot.hasHide = false;
		VectorCopy( c.sense.enemyDir, c.course );
		c.travelTo( c.enemyPosition() );
		c.hold = c.sense.enemyDist < 1500;
		return Status::Running;
	}

	static Status driveToGoal( Mind& c )
	{
		c.bot.hasHide = false;
		c.courseTo( c.bot.goal );
		return Status::Running;
	}

	static Status countermeasures( Mind& c )
	{
		Bots::flares( c.ent, c.bot, c.sense, c.cmd );
		return Status::Success;
	}

	static Status navigate( Mind& c )
	{
		c.followGround( c.boat, 220.0f );
		return Status::Success;
	}

	static Status watchAhead( Mind& c )
	{
		Bots::State& bot = c.bot;
		vec3_t forward, flat = { 0, c.ps.vehicleAngles[YAW], 0 }, start;
		trace_t tr;
		AngleVectors( flat, forward, nullptr, nullptr );
		VectorCopy( c.ent->r.currentOrigin, start );
		start[2] += c.veh.maxs[2] * 0.5f + 8;
		c.blocked = Bots::pathBlocked( start, forward, 260, c.ent->s.number, tr ) && tr.plane.normal[2] < 0.6f;
		if( !c.boat && !c.blocked )
		{
			for( float reach : { 160.0f, 380.0f } )
			{
				bool drop;
				if( waterAt( c, forward, reach, drop ) || drop )
					c.blocked = true;
			}
		}
		if( c.boat && !c.blocked )
		{
			vec3_t ahead;
			VectorMA( c.ent->r.currentOrigin, 500, forward, ahead );
			ahead[2] = c.ent->r.currentOrigin[2] - 6;
			if( !(SV_PointContents( ahead, c.ent->s.number ) & MASK_WATER) )
				c.blocked = true;
		}
		if( theLevel.time_ > bot.stuckCheckTime )
		{
			if( !c.hold && theLevel.time_ > bot.spawnTime + 4000 && theLevel.time_ > bot.reverseTime + 2000 &&
				Distance( c.ent->r.currentOrigin, bot.lastOrigin ) < 40 )
				c.blocked = true;
			VectorCopy( c.ent->r.currentOrigin, bot.lastOrigin );
			bot.stuckCheckTime = theLevel.time_ + 2500;
		}
		if( c.blocked && theLevel.time_ > bot.reverseTime )
		{
			bot.reverseTime = theLevel.time_ + 1800;
			bot.reverseTurn = (rand() & 1) ? moveFull : -moveFull;
			bot.pathTime = 0;
			if( !bot.following )
				Bots::pickGoal( c.ent, bot, c.boat ? 6000 : 4000 );
		}
		return Status::Success;
	}

	static Status creepForward( Mind& c )
	{
		c.cmd.forwardmove = c.blocked ? moveNone : moveFull;
		c.cmd.rightmove = static_cast<signed char>( c.bot.reverseTurn );
		return Status::Running;
	}

	static Status reverse( Mind& c )
	{
		c.cmd.forwardmove = -moveFull;
		c.cmd.rightmove = static_cast<signed char>( c.bot.reverseTurn );
		return Status::Running;
	}

	static Status steer( Mind& c )
	{
		vec3_t angles;
		c.course[2] = 0;
		vectoangles( c.course, angles );
		const float yawError = AngleSubtract( angles[YAW], c.ps.vehicleAngles[YAW] );
		if( yawError < -6 )
			c.cmd.rightmove = moveFull;
		else if( yawError > 6 )
			c.cmd.rightmove = -moveFull;
		if( !c.hold )
			c.cmd.forwardmove = moveFull;
		return Status::Running;
	}

	static Status gunnery( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW];
		c.view[PITCH] = std::clamp( AngleSubtract( c.sense.enemyAngles[PITCH], 0 ), -60.0f, 20.0f );
		const int selected = c.veh.weapons[c.ps.weaponNum];
		if( selected && availableWeapons[selected].type == WT_BALLISTICGUN && availableWeapons[selected].muzzleVelocity > 0 )
		{
			const float shell = static_cast<float>( availableWeapons[selected].muzzleVelocity );
			const float flight = c.sense.enemyDist / shell;
			vec3_t aim, dirAim, aimAngles, known;
			c.enemyPosition().copyTo( known );
			VectorMA( known, flight, c.sense.enemy->client_->ps_.velocity, aim );
			VectorSubtract( aim, c.ent->r.currentOrigin, dirAim );
			const float range = VectorLength( dirAim );
			vectoangles( dirAim, aimAngles );
			const float arc = 0.5f * asin( std::clamp( DEFAULT_GRAVITY * range / (shell * shell), 0.0f, 1.0f ) ) * 180.0f / static_cast<float>( M_PI );
			c.view[YAW] = aimAngles[YAW];
			c.view[PITCH] = std::clamp( AngleSubtract( aimAngles[PITCH], 0 ) - arc, -60.0f, 20.0f );
		}
		const float turretYaw = c.ps.vehicleAngles[YAW] + c.ps.turretAngle;
		Bots::fireWeapons( c.ent, c.bot, c.sense, c.cmd, fabs( AngleSubtract( c.view[YAW], turretYaw ) ) );
		return Status::Success;
	}

	static behavior::Tree<Mind> build()
	{
		behavior::Tree<Mind> t;

		const auto* act = t.selector( "driver", {
			t.when( "aircraft overhead", hidingFromAircraft, t.selector( "hide", {
				t.action( "drive to cover", driveToCover ),
				t.action( "run from aircraft", runFromAircraft ) } ) ),
			t.when( "cautious gunner", wantsHullDown, t.action( "hull down", hullDown ) ),
			t.when( "wants to rearm", rearming, t.action( "drive to pad", driveToPad ) ),
			t.when( "evading", evading, t.action( "withdraw", withdraw ) ),
			t.when( "ground enemy", engaging, t.action( "engage", engage ) ),
			t.action( "drive to goal", driveToGoal ) } );

		const auto* drive = t.selector( "controls", {
			t.when( "water behind", waterBehind, t.action( "creep forward", creepForward ) ),
			t.when( "backing out", reversing, t.action( "reverse", reverse ) ),
			t.action( "steer", steer ) } );

		t.setRoot( t.always( "crew", act, {
			t.action( "countermeasures", countermeasures ),
			t.action( "navigate", navigate ),
			t.action( "watch ahead", watchAhead ),
			drive,
			t.when( "has a target", hasEnemy, t.action( "gunnery", gunnery ) ) } ) );
		return t;
	}
};

void Bots::thinkSurface( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = SurfaceTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };
	mind.boat = (mind.veh.cat & CAT_BOAT) != 0;

	if( bot.desire != Goal::Capture && (theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 400 ) )
		pickGoal( ent, bot, mind.boat ? 6000 : 4000 );
	view[YAW] = ps.vehicleAngles[YAW];
	view[PITCH] = 0;

	runTree( tree, mind, "surface" );
}
