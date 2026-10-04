#include "bot_mind.h"
#include "bot_airnav.h"

using behavior::Status;

struct HeloTasks
{
	using Mind = Bots::Mind;
	using Goal = Bots::Goal;
	using Role = Bots::Role;

	enum Mode { Cruise, Run, ToPad, MoveToCover, BackOff, Hover };

	static bool overPad( const Mind& c )
	{
		vec3_t dir;
		VectorSubtract( Bots::pads_[c.bot.pad].center, c.ent->r.currentOrigin, dir );
		dir[2] = 0;
		return VectorLength( dir ) < 120;
	}

	static bool rearming( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.pad >= 0; }
	static bool parked( Mind& c ) { return c.landed() && overPad( c ); }
	static bool abovePad( Mind& c ) { return overPad( c ); }
	static bool missileInbound( Mind& c ) { return c.sense.missile && c.sense.missileTime < 6.0f && !c.landed(); }
	// only transport helicopters take passengers; attack helicopters leave soldiers to them
	static bool transport( const Mind& c ) { return c.veh.seats > 1 && !(c.veh.cls & CLASS_HELO_ATTACK); }

	static bool walkerWaiting( Mind& c )
	{
		constexpr float pickupRange = 1800.0f;
		c.target = transport( c ) && !c.sense.enemyVisible ? Bots::walkerNear( c.ent, c.bot, pickupRange ) : nullptr;
		return c.target != nullptr;
	}

	// come down beside a team mate on foot and hold still so they can climb in
	static Status pickUpWalker( Mind& c )
	{
		flyTo( c, c.target->r.currentOrigin );
		c.standoff = -300.0f;
		c.cover = true;
		if( c.dist < 140 )
		{
			c.hasDest = false;
			c.mode = Hover;
			c.hold = true;
			c.wantHeight = -200.0f;
			if( !(c.ps.ONOFF & OO_GEAR) )
				c.cmd.buttons |= BUTTON_GEAR;
			if( c.landed() )
				c.wait = true;
		}
		else
			c.wantHeight = std::clamp( c.dist * 0.3f, 60.0f, heloLowHeight );
		return Status::Running;
	}

	static bool passengersNear( Mind& c )
	{
		if( g_gametype.integer < GT_TEAM || !c.landed() || theLevel.time_ >= c.bot.spawnTime + 12000 || !transport( c ) )
			return false;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == c.ent || !alive( other ) || !OnSameTeam( c.ent, other ) )
				continue;
			if( !(availableVehicles[other->client_->vehicle_].cat & CAT_LQM) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( Distance( c.ent->r.currentOrigin, other->r.currentOrigin ) < 1200 )
				return true;
		}
		return false;
	}
	static bool evading( Mind& c ) { return c.bot.desire == Goal::Evade && (c.sense.enemy || c.sense.blindsided); }
	static bool capturing( Mind& c ) { return c.bot.desire == Goal::Capture; }
	static bool fighting( Mind& c ) { return c.sense.enemy && c.bot.desire == Goal::Attack && c.sense.enemyDist < 3000; }
	static bool inCover( Mind& c ) { return theLevel.time_ < c.bot.hideTime; }
	static bool hasCover( Mind& c ) { return c.bot.hasHide; }
	static bool exposed( Mind& c ) { return c.bot.exposed; }
	static bool seesEnemy( Mind& c ) { return c.sense.enemyVisible; }
	static bool hasEnemy( Mind& c ) { return c.sense.enemy != nullptr; }
	static bool airborne( Mind& c ) { return !c.landed(); }
	static bool hasLeader( Mind& c ) { return c.bot.leader >= 0; }
	static bool carriesCrates( Mind& c )
	{
		return crateSlot( c, WT_FUELCRATE ) >= 0 || crateSlot( c, WT_AMMOCRATE ) >= 0 || crateSlot( c, WT_HEALTHCRATE ) >= 0;
	}

	static void faceEnemy( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW];
		c.dist = c.sense.enemyDist;
		c.standoff = c.sense.enemyAir ? 500.0f : 1200.0f;
		c.mode = Cruise;
	}

	static void flyTo( Mind& c, const vec3_t point )
	{
		vec3_t dir;
		VectorSubtract( point, c.ent->r.currentOrigin, dir );
		dir[2] = 0;
		c.dist = VectorLength( dir );
		c.headFor( point );
		c.travelTo( Vec3( point[0], point[1], c.ent->r.currentOrigin[2] ) );
		c.mode = Cruise;
	}

	static Status rearmOnPad( Mind& c )
	{
		c.wait = true;
		c.cover = true;
		c.mode = Hover;
		c.wantHeight = -200.0f;
		if( Bots::replenished( c.ent ) )
		{
			Bots::finishRearm( c.ent, c.bot );
			return Status::Success;
		}
		return Status::Running;
	}

	static Status descendToPad( Mind& c )
	{
		flyTo( c, Bots::pads_[c.bot.pad].center );
		c.hasDest = false;
		c.cover = true;
		c.hold = true;
		c.mode = Hover;
		c.wantHeight = -200.0f;
		if( !(c.ps.ONOFF & OO_GEAR) )
			c.cmd.buttons |= BUTTON_GEAR;
		return Status::Running;
	}

	static Status flyToPad( Mind& c )
	{
		flyTo( c, Bots::pads_[c.bot.pad].center );
		c.cover = true;
		c.mode = ToPad;
		c.wantHeight = heloCruiseHeight * 0.6f;
		return Status::Running;
	}

	static Status dodgeMissile( Mind& c )
	{
		c.view[YAW] = c.sense.missileYaw + 90.0f;
		c.cover = true;
		c.defending = true;
		c.wantHeight = 60.0f;
		c.mode = Run;
		return Status::Running;
	}

	static Status waitForPassengers( Mind& c )
	{
		c.wait = true;
		return Status::Running;
	}

	static Status evade( Mind& c )
	{
		c.view[YAW] = c.sense.blindsided ? c.bot.breakYaw : c.sense.enemyAngles[YAW] + 180.0f;
		c.defending = true;
		c.mode = Run;
		return Status::Running;
	}

	static Status goToFlag( Mind& c )
	{
		flyTo( c, c.bot.goal );
		if( c.dist < 500 )
			c.wantHeight = 25.0f;
		return Status::Running;
	}

	static Status moveToCover( Mind& c )
	{
		faceEnemy( c );
		vec3_t toCover;
		VectorSubtract( c.bot.hidePoint, c.ent->r.currentOrigin, toCover );
		toCover[2] = 0;
		c.dist = VectorLength( toCover );
		c.headFor( c.bot.hidePoint );
		c.cover = true;
		c.wantHeight = 110.0f;
		c.mode = MoveToCover;
		return Status::Running;
	}

	static Status sinkIntoCover( Mind& c )
	{
		faceEnemy( c );
		c.cover = true;
		c.wantHeight = 110.0f;
		c.mode = BackOff;
		return Status::Running;
	}

	static Status fireExposed( Mind& c )
	{
		faceEnemy( c );
		c.wantHeight = std::max( c.sense.height, heloLowHeight );
		if( theLevel.time_ > c.bot.popTime )
		{
			c.bot.exposed = false;
			c.bot.hideTime = theLevel.time_ + 2000 + static_cast<int>( 3000 * c.who().caution );
			c.bot.hasHide = Bots::findHidePoint( c.ent, c.bot, c.sense.enemy );
		}
		return Status::Running;
	}

	static Status expose( Mind& c )
	{
		c.bot.exposed = true;
		c.bot.popTime = theLevel.time_ + 3000 + static_cast<int>( 4000 * c.who().patience );
		return fireExposed( c );
	}

	static Status popUp( Mind& c )
	{
		faceEnemy( c );
		c.wantHeight = std::min( c.sense.height + 300.0f, 900.0f );
		return Status::Running;
	}

	static Status closeOnEnemy( Mind& c )
	{
		faceEnemy( c );
		const Vec3 enemy = c.enemyPosition();
		c.travelTo( Vec3( enemy.x(), enemy.y(), c.ent->r.currentOrigin[2] ) );
		return Status::Running;
	}

	static int crateSlot( const Mind& c, int type )
	{
		for( int slot : allWeaponSlots )
		{
			const int weapon = c.veh.weapons[slot];
			if( weapon && c.ps.ammo[slot] > 0 && availableWeapons[weapon].type == type )
				return slot;
		}
		return -1;
	}

	// Carrying crates: find a team mate on the ground who is short of something we carry, a real player first,
	// hover over them and drop it.
	static Status resupply( Mind& c )
	{
		constexpr float range = 8000.0f;
		constexpr float humanPreference = 0.3f;
		constexpr float dropHeight = 220.0f;
		constexpr int repeatDelay = 12000;
		if( g_gametype.integer < GT_TEAM || theLevel.time_ < c.bot.bombTime + 1500 )
			return Status::Failure;
		GameEntity* needy = nullptr;
		int slot = -1;
		float best = 0;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == c.ent || !alive( other ) || !OnSameTeam( c.ent, other ) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			// crates land on the ground, so only someone who is down there can use them
			const bool flying = (availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) && !(other->client_->ps_.ONOFF & OO_LANDED);
			if( flying || (c.bot.structure == i && theLevel.time_ < c.bot.structureTime) )
				continue;
			// what they are short of; failing that, someone in a fight will want ammunition soon
			int need = Bots::supplyNeed( other );
			const bool fighting = Bots::isBot( i ) ? Bots::states_[i].enemy >= 0 : other->health_ < other->client_->pers_.maxHealth_;
			float urgency = 1.0f;
			if( !need && fighting )
			{
				need = WT_AMMOCRATE;
				urgency = 2.5f;
			}
			const int carried = need ? crateSlot( c, need ) : -1;
			if( carried < 0 )
				continue;
			const float dist = Distance( c.ent->r.currentOrigin, other->r.currentOrigin );
			const float weighted = (Bots::isBot( i ) ? dist : dist * humanPreference) * urgency;
			if( dist < range && (!needy || weighted < best) )
			{
				needy = other;
				slot = carried;
				best = weighted;
			}
		}
		if( !needy )
			return Status::Failure;

		flyTo( c, needy->r.currentOrigin );
		c.standoff = -300.0f;
		c.wantHeight = std::max( dropHeight, heloLowHeight );
		c.cmd.weapon = slot;
		if( c.dist < 120 )
		{
			c.mode = Hover;
			if( c.ps.weaponNum == slot && c.sense.height > dropHeight * 0.6f )
			{
				c.cmd.buttons |= BUTTON_ATTACK_MAIN;
				c.bot.bombTime = theLevel.time_;
				// do not shower the same vehicle: remember who was just supplied
				c.bot.structure = needy->s.number;
				c.bot.structureTime = theLevel.time_ + repeatDelay;
			}
		}
		return Status::Running;
	}

	// stay beside the leader, on the side given by the client number
	static Status escort( Mind& c )
	{
		GameEntity* leader = Bots::leaderOf( c.bot );
		if( !leader )
			return Status::Failure;
		vec3_t flat = { 0, leader->client_->ps_.vehicleAngles[YAW], 0 }, right, spot;
		AngleVectors( flat, nullptr, right, nullptr );
		VectorMA( leader->r.currentOrigin, (c.ent->s.number & 1) ? 350.0f : -350.0f, right, spot );
		flyTo( c, spot );
		c.standoff = -200.0f;
		if( c.dist < 150 )
			c.mode = Hover;
		return Status::Running;
	}

	static Status patrol( Mind& c )
	{
		if( theLevel.time_ > c.bot.goalTime || Distance( c.ent->r.currentOrigin, c.bot.goal ) < 800 )
			Bots::pickGoal( c.ent, c.bot, 5000 );
		flyTo( c, c.bot.goal );
		return Status::Running;
	}

	static Status countermeasures( Mind& c )
	{
		Bots::flares( c.ent, c.bot, c.sense, c.cmd );
		return Status::Success;
	}

	static Status navigate( Mind& c )
	{
		Bots::State& bot = c.bot;
		const Vec3 here( c.ent->r.currentOrigin );
		const Vec3 toDest = c.dest - here;
		const float range = toDest.length();
		const Vec3 probeEnd = range > 1500.0f ? here + toDest * (1500.0f / range) : c.dest;
		if( !c.hasDest || c.landed() || range <= 600.0f || AirNav::clearLine( here, probeEnd, 60.0f ) )
		{
			bot.path.clear();
			bot.following = false;
			return Status::Success;
		}
		if( theLevel.time_ >= bot.pathTime )
		{
			AirNav::findPath( here, c.dest, bot.path );
			bot.pathIndex = 0;
			bot.pathTime = theLevel.time_ + 2000;
		}
		while( bot.pathIndex + 1 < bot.path.size() &&
			((bot.path[bot.pathIndex] - here).length() < 250.0f || AirNav::clearLine( here, bot.path[bot.pathIndex + 1], 60.0f )) )
			bot.pathIndex++;
		bot.following = bot.pathIndex < bot.path.size();
		if( !bot.following )
			return Status::Success;
		const Vec3 waypoint = bot.path[bot.pathIndex];
		vec3_t point;
		waypoint.copyTo( point );
		c.headFor( point );
		c.wantHeight = std::clamp( c.sense.height + (waypoint.z() - here.z()), 60.0f, 1500.0f );
		c.dist = std::max( c.dist, c.standoff + 400.0f );
		return Status::Success;
	}

	static Status collective( Mind& c )
	{
		// never climb into the roof, whatever asked for the height
		constexpr float headroom = 150.0f;
		if( !c.landed() )
		{
			const float highest = c.sense.height + Bots::ceilingAbove( c.ent->r.currentOrigin, c.ent->s.number ) - headroom;
			c.wantHeight = std::min( c.wantHeight, std::max( highest, 40.0f ) );
		}
		const float throttle = signedThrottle( c.ps, c.client->vehicle_ );
		float wantThrottle = std::clamp( (c.wantHeight - c.sense.height) / 60.0f, -3.0f, static_cast<float>( c.veh.maxthrottle ) );
		if( c.wait )
			wantThrottle = 0;
		else if( c.landed() )
			wantThrottle = std::max( wantThrottle, c.veh.maxthrottle * 0.6f );
		if( c.hold && c.sense.height < 150 )
			wantThrottle = -1.0f;
		if( throttle < floor( wantThrottle ) )
			c.cmd.buttons |= BUTTON_INCREASE;
		else if( throttle > ceil( wantThrottle ) )
			c.cmd.buttons |= BUTTON_DECREASE;
		return Status::Success;
	}

	// free distance for the whole airframe along a direction, up to reach
	static float clearAlong( const Mind& c, const vec3_t dir, float reach )
	{
		constexpr float margin = 24.0f;
		vec3_t mins, maxs, end;
		for( int k = 0; k < 3; k++ )
		{
			mins[k] = c.veh.mins[k] - margin;
			maxs[k] = c.veh.maxs[k] + margin;
		}
		VectorMA( c.ent->r.currentOrigin, reach, dir, end );
		trace_t tr;
		SV_Trace( &tr, c.ent->r.currentOrigin, mins, maxs, end, c.ent->s.number, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
		if( tr.startsolid )
			SV_Trace( &tr, c.ent->r.currentOrigin, c.veh.mins, c.veh.maxs, end, c.ent->s.number, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
		return tr.startsolid ? 0.0f : tr.fraction * reach;
	}

	// A helicopter goes where it tilts, in any direction, and stops when it levels. The pilot keeps the nose on what it
	// is looking at and tilts toward the clear direction nearest to where it wants to go.
	static Status steer( Mind& c )
	{
		constexpr float maxPitch = 30.0f;
		constexpr float maxRoll = 40.0f;
		constexpr float levelling = 0.75f;
		c.yawError = fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) );

		// how hard to push in the wanted direction, as forward tilt in degrees (negative backs away)
		float push = 0;
		if( c.sense.height > 60 )
		{
			switch( c.mode )
			{
			case Run:
				push = 14.0f;
				break;
			case ToPad:
				push = std::clamp( c.dist / 60.0f, 3.0f, 14.0f );
				break;
			case MoveToCover:
				push = c.dist > 150 ? 12.0f : 0.0f;
				break;
			case BackOff:
				push = -5.0f;
				break;
			case Hover:
				break;
			default:
				if( c.dist > c.standoff + 300 )
					push = c.sense.height > 300 ? 14.0f : 9.0f;
				else if( c.sense.enemy && c.dist < c.standoff )
					push = -6.0f;
				break;
			}
		}

		const float maxSpeed = static_cast<float>( c.veh.maxspeed );
		const float sideSpeed = std::max( 1.0f, c.veh.turnspeed[YAW] * 2.5f );
		float travelYaw = c.view[YAW] + (push < 0 ? 180.0f : 0.0f);
		float speed = fabs( push ) / maxPitch * maxSpeed;
		c.blocked = false;
		if( speed > 1.0f )
		{
			// the wanted direction first, then either side of it, then back the way it came; hover if nothing is open
			const float reach = 200.0f + speed * 2.0f;
			bool found = false;
			for( float offset : { 0.0f, 45.0f, -45.0f, 90.0f, -90.0f, 135.0f, -135.0f, 180.0f } )
			{
				vec3_t angles = { 0, travelYaw + offset, 0 }, dir;
				AngleVectors( angles, dir, nullptr, nullptr );
				if( clearAlong( c, dir, reach ) < reach )
				{
					if( offset == 0.0f )
						c.blocked = true;
					continue;
				}
				travelYaw += offset;
				if( offset != 0.0f )
					speed *= 0.7f;
				found = true;
				break;
			}
			if( !found )
				speed = 0;
			// something in the way of the direct line: go over it if there is room above
			if( c.blocked )
				c.wantHeight = std::max( c.wantHeight, std::min( c.sense.height + 300.0f, 1500.0f ) );
		}

		// the tilt is coarse, so the real motion can differ from the wanted one: stop if it leads into something
		vec3_t moving = { c.ps.velocity[0], c.ps.velocity[1], 0 };
		const float drift = VectorNormalize( moving );
		if( drift > 15.0f && clearAlong( c, moving, 120.0f + drift * 1.5f ) < 120.0f + drift * 1.5f )
			speed = 0;

		// slow or hovering next to something: ease away from it
		if( speed < 20.0f )
		{
			constexpr float nearby = 140.0f;
			vec3_t away = { 0, 0, 0 };
			for( float around = 0.0f; around < 360.0f; around += 45.0f )
			{
				vec3_t angles = { 0, around, 0 }, dir;
				AngleVectors( angles, dir, nullptr, nullptr );
				const float free = clearAlong( c, dir, nearby );
				if( free < nearby )
					VectorMA( away, -(nearby - free) / nearby, dir, away );
			}
			if( VectorNormalize( away ) > 0.05f )
			{
				vec3_t angles;
				vectoangles( away, angles );
				travelYaw = angles[YAW];
				speed = maxSpeed * 0.12f;
			}
		}

		vec3_t travel = { 0, travelYaw, 0 }, body = { 0, c.ps.vehicleAngles[YAW], 0 }, dir, forward, right;
		AngleVectors( travel, dir, nullptr, nullptr );
		AngleVectors( body, forward, right, nullptr );
		const float wantPitch = std::clamp( DotProduct( dir, forward ) * speed / maxSpeed * maxPitch, -maxPitch, maxPitch );
		const float wantRoll = std::clamp( DotProduct( dir, right ) * speed / sideSpeed * maxRoll, -maxRoll * 0.8f, maxRoll * 0.8f );
		const float pitchError = wantPitch - c.ps.vehicleAngles[PITCH];
		// level precisely: a couple of degrees left over is a steady drift
		if( pitchError > levelling )
			c.cmd.forwardmove = moveFull;
		else if( pitchError < -levelling )
			c.cmd.forwardmove = -moveFull;
		const float rollError = wantRoll - c.ps.vehicleAngles[ROLL];
		if( rollError > levelling )
			c.cmd.rightmove = moveFull;
		else if( rollError < -levelling )
			c.cmd.rightmove = -moveFull;
		return Status::Success;
	}

	static Status fire( Mind& c )
	{
		if( c.sense.enemy && !c.cover && !c.defending && !c.bot.following )
			Bots::fireWeapons( c.ent, c.bot, c.sense, c.cmd, c.yawError );
		return Status::Success;
	}

	static behavior::Tree<Mind> build()
	{
		behavior::Tree<Mind> t;

		const auto* rearm = t.selector( "rearm", {
			t.when( "parked on the pad", parked, t.action( "rearm on pad", rearmOnPad ) ),
			t.when( "above the pad", abovePad, t.action( "descend to pad", descendToPad ) ),
			t.action( "fly to pad", flyToPad ) } );

		const auto* popAndCover = t.selector( "pop up and cover", {
			t.when( "hiding", inCover, t.selector( "cover", {
				t.when( "knows cover", hasCover, t.action( "move to cover", moveToCover ) ),
				t.action( "sink into cover", sinkIntoCover ) } ) ),
			t.when( "exposed", exposed, t.action( "fire exposed", fireExposed ) ),
			t.when( "sees the enemy", seesEnemy, t.action( "expose", expose ) ),
			t.action( "pop up", popUp ) } );

		const auto* act = t.selector( "helicopter", {
			t.when( "missile inbound", missileInbound, t.action( "dodge missile", dodgeMissile ) ),
			t.when( "wants to rearm", rearming, rearm ),
			t.when( "infantry wants a lift", passengersNear, t.action( "wait for passengers", waitForPassengers ) ),
			t.when( "team mate on foot nearby", walkerWaiting, t.action( "pick up walker", pickUpWalker ) ),
			t.when( "evading", evading, t.action( "evade", evade ) ),
			t.when( "capturing", capturing, t.action( "go to flag", goToFlag ) ),
			t.when( "carries supplies", carriesCrates, t.action( "resupply team mate", resupply ) ),
			t.when( "fighting", fighting, popAndCover ),
			t.when( "has an enemy", hasEnemy, t.action( "close on enemy", closeOnEnemy ) ),
			t.when( "has a leader", hasLeader, t.action( "escort", escort ) ),
			t.action( "patrol", patrol ) } );

		t.setRoot( t.always( "pilot", act, {
			t.action( "countermeasures", countermeasures ),
			t.action( "navigate", navigate ),
			t.when( "airborne", airborne, t.always( "fly", t.action( "steer", steer ), { t.action( "fire", fire ) } ) ),
			t.action( "collective", collective ) } ) );
		return t;
	}
};

void Bots::thinkHelo( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = HeloTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };

	mind.wantHeight = sense.role == Role::AttackHelo ? heloLowHeight : heloCruiseHeight * 0.6f;
	if( !HeloTasks::fighting( mind ) )
		bot.exposed = false;
	view[YAW] = ps.vehicleAngles[YAW];
	view[PITCH] = sense.enemy ? std::clamp( AngleSubtract( sense.enemyAngles[PITCH], 0 ), -30.0f, 45.0f ) : 0.0f;

	runTree( tree, mind, "helo" );
}
