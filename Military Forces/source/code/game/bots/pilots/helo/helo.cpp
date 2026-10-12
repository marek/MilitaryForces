#include "../../core/mind.h"
#include "../../navigation/airnav.h"

using behavior::Status;

// What a helicopter pilot does: the conditions and steps of the helicopter tree. The steps say where to go, at what
// height and in which manner; steering and the collective at the end turn that into the controls.
struct HeloTasks
{
	using Mind = Bots::Mind;
	using Goal = Bots::Goal;
	using Role = Bots::Role;

	// how to move towards the place steered for: at cruising tilt, flat out, slowing onto a pad, into cover, backing
	// away, or holding still
	enum Mode { Cruise, Run, ToPad, MoveToCover, BackOff, Hover };

	// within 120 units of the pad's middle, seen from above
	static bool overPad( const Mind& c )
	{
		Vec3 dir;
		VectorSubtract( BotWorld::pads_[c.bot.pad].center, c.ent->r.currentOrigin, dir );
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
		c.target = transport( c ) && !c.sense.enemyVisible ? BotSenses::walkerNear( c.ent, c.bot, pickupRange ) : nullptr;
		return c.target != nullptr;
	}

	// come down beside a team mate on foot and hold still so they can climb in
	static Status pickUpWalker( Mind& c )
	{
		flyTo( c, c.target->r.currentOrigin );
		c.steer.standoff = -300.0f;
		c.weapons.cover = true;
		if( c.steer.dist < 140 )
		{
			c.steer.hasDest = false;
			c.steer.mode = Hover;
			c.steer.hold = true;
			c.steer.wantHeight = -200.0f;
			if( !(c.ps.ONOFF & OO_GEAR) )
				c.cmd.buttons |= BUTTON_GEAR;
			if( c.landed() )
				c.steer.wait = true;
		}
		else
			c.steer.wantHeight = std::clamp( c.steer.dist * 0.3f, 60.0f, heloLowHeight );
		return Status::Running;
	}

	// just spawned on the ground with team mates on foot close by: give them a moment to climb in
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
	// near enough to fight from cover
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

	// nose on the enemy, to stand off at gun range from an aircraft and further from the ground
	static void faceEnemy( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW];
		c.steer.dist = c.sense.enemyDist;
		c.steer.standoff = c.sense.enemyAir ? 500.0f : 1200.0f;
		c.steer.mode = Cruise;
	}

	// head for a point at the present height
	static void flyTo( Mind& c, const vec3_t point )
	{
		Vec3 dir;
		VectorSubtract( point, c.ent->r.currentOrigin, dir );
		dir[2] = 0;
		c.steer.dist = VectorLength( dir );
		c.headFor( point );
		c.travelTo( Vec3( point[0], point[1], c.ent->r.currentOrigin[2] ) );
		c.steer.mode = Cruise;
	}

	// sit on the pad, collective down, until full
	static Status rearmOnPad( Mind& c )
	{
		c.steer.wait = true;
		c.weapons.cover = true;
		c.steer.mode = Hover;
		c.steer.wantHeight = -200.0f;
		if( Bots::replenished( c.ent ) )
		{
			Bots::finishRearm( c.ent, c.bot );
			return Status::Success;
		}
		return Status::Running;
	}

	// over the pad: gear down and straight down
	static Status descendToPad( Mind& c )
	{
		flyTo( c, BotWorld::pads_[c.bot.pad].center );
		c.steer.hasDest = false;
		c.weapons.cover = true;
		c.steer.hold = true;
		c.steer.mode = Hover;
		c.steer.wantHeight = -200.0f;
		if( !(c.ps.ONOFF & OO_GEAR) )
			c.cmd.buttons |= BUTTON_GEAR;
		return Status::Running;
	}

	// to the pad at a moderate height, slowing as it nears
	static Status flyToPad( Mind& c )
	{
		flyTo( c, BotWorld::pads_[c.bot.pad].center );
		c.weapons.cover = true;
		c.steer.mode = ToPad;
		c.steer.wantHeight = heloCruiseHeight * 0.6f;
		return Status::Running;
	}

	// across the missile's path, low and fast
	static Status dodgeMissile( Mind& c )
	{
		c.view[YAW] = c.sense.missileYaw + 90.0f;
		c.weapons.cover = true;
		c.steer.evading = true;
		c.steer.wantHeight = 60.0f;
		c.steer.mode = Run;
		return Status::Running;
	}

	static Status waitForPassengers( Mind& c )
	{
		c.steer.wait = true;
		return Status::Running;
	}

	// away from the enemy, or to the side when hit by someone unseen
	static Status evade( Mind& c )
	{
		c.view[YAW] = c.sense.blindsided ? c.bot.breakYaw : c.sense.enemyAngles[YAW] + 180.0f;
		c.steer.evading = true;
		c.steer.mode = Run;
		return Status::Running;
	}

	// to the flag, and down onto it at the end: a flag is touched near the ground
	static Status goToFlag( Mind& c )
	{
		flyTo( c, c.bot.goal );
		if( c.steer.dist < 500 )
			c.steer.wantHeight = 25.0f;
		return Status::Running;
	}

	// to the hiding place, low, facing the enemy
	static Status moveToCover( Mind& c )
	{
		faceEnemy( c );
		Vec3 toCover;
		VectorSubtract( c.bot.hidePoint, c.ent->r.currentOrigin, toCover );
		toCover[2] = 0;
		c.steer.dist = VectorLength( toCover );
		c.headFor( c.bot.hidePoint );
		c.weapons.cover = true;
		c.steer.wantHeight = 110.0f;
		c.steer.mode = MoveToCover;
		return Status::Running;
	}

	// no hiding place known: back off low
	static Status sinkIntoCover( Mind& c )
	{
		faceEnemy( c );
		c.weapons.cover = true;
		c.steer.wantHeight = 110.0f;
		c.steer.mode = BackOff;
		return Status::Running;
	}

	// up in view and shooting until the time in the open is used up, then back into cover for a while
	static Status fireExposed( Mind& c )
	{
		faceEnemy( c );
		c.steer.wantHeight = std::max( c.sense.height, heloLowHeight );
		if( theLevel.time_ > c.bot.popTime )
		{
			c.bot.exposed = false;
			c.bot.hideTime = theLevel.time_ + 2000 + static_cast<int>( 3000 * c.who().caution );
			c.bot.hasHide = Bots::findHidePoint( c.ent, c.bot, c.sense.enemy );
		}
		return Status::Running;
	}

	// the enemy is in sight: start the time in the open, longer for the patient
	static Status expose( Mind& c )
	{
		c.bot.exposed = true;
		c.bot.popTime = theLevel.time_ + 3000 + static_cast<int>( 4000 * c.who().patience );
		return fireExposed( c );
	}

	// rise until the enemy comes into view
	static Status popUp( Mind& c )
	{
		faceEnemy( c );
		c.steer.wantHeight = std::min( c.sense.height + 300.0f, 900.0f );
		return Status::Running;
	}

	// too far to fight: fly to where the enemy is or was last seen
	static Status closeOnEnemy( Mind& c )
	{
		faceEnemy( c );
		const Vec3 enemy = c.enemyPosition();
		c.travelTo( Vec3( enemy.x(), enemy.y(), c.ent->r.currentOrigin[2] ) );
		return Status::Running;
	}

	// A stealth helicopter fights from where it cannot be seen: it comes in low with the bay shut, rises from behind
	// cover inside missile range, fires, drops, and comes up next somewhere else. It does not stay for a fight at
	// close quarters, which is the other side's game.
	static constexpr float closeQuarters = 1800.0f, clearOfThem = 3200.0f, strikeShare = 0.85f, sameSpot = 500.0f, sidestep = 700.0f;

	static int guidedAmmo( const Mind& c )
	{
		int ammo = 0;
		for( int slot : secondaryWeaponSlots )
		{
			const int type = availableWeapons[c.veh.weapons[slot]].type;
			if( c.veh.weapons[slot] && (type == WT_ANTIAIRMISSILE || type == WT_ANTIGROUNDMISSILE) )
				ammo += c.ps.ammo[slot];
		}
		return ammo;
	}
	// how far off it fires: most of the reach of the missile it would use on this enemy
	static float strikeRange( const Mind& c )
	{
		bool guided = false;
		const int slot = BotWeapons::findWeaponSlot( c.client->vehicle_, c.ps, c.sense.enemyAir, 0.0f, guided );
		if( slot < 0 || !guided )
			return 0.0f;
		const float range = static_cast<float>( availableWeapons[c.veh.weapons[slot]].range );
		return (range > 0 ? range : 3000.0f) * strikeShare;
	}
	static bool stealthStriking( Mind& c )
	{
		return c.sense.stealth && c.sense.enemy && c.bot.desire == Goal::Attack && strikeRange( c ) > closeQuarters;
	}
	// once it has started to leave it keeps going until it is well clear
	static bool tooClose( Mind& c )
	{
		const float dist = Distance( c.sense.enemy->r.currentOrigin, c.ent->r.currentOrigin );
		c.bot.breakingOff = dist < (c.bot.breakingOff ? clearOfThem : closeQuarters);
		return c.bot.breakingOff;
	}
	static bool outOfStrikeRange( Mind& c ) { return Distance( c.sense.enemy->r.currentOrigin, c.ent->r.currentOrigin ) > strikeRange( c ); }

	static Status breakOff( Mind& c )
	{
		c.view[YAW] = c.sense.enemyAngles[YAW] + 180.0f;
		c.bot.exposed = false;
		c.weapons.cover = true;
		c.steer.evading = true;
		c.steer.wantHeight = 90.0f;
		c.steer.mode = Run;
		return Status::Running;
	}

	// towards the enemy, low, weapons held: nothing that would show it before it is ready
	static Status stalk( Mind& c )
	{
		closeOnEnemy( c );
		c.weapons.cover = true;
		c.steer.wantHeight = 90.0f;
		return Status::Running;
	}

	// in view at its firing place. It goes back down as soon as a missile has left, or when the time is up
	static Status strikeExposed( Mind& c )
	{
		faceEnemy( c );
		c.steer.standoff = c.sense.enemyDist;
		c.steer.wantHeight = std::max( c.sense.height, heloLowHeight );
		if( guidedAmmo( c ) < c.bot.strikeAmmo || theLevel.time_ > c.bot.popTime )
		{
			c.bot.exposed = false;
			c.bot.hideTime = theLevel.time_ + 4000 + static_cast<int>( 3000 * c.who().caution );
			VectorCopy( c.ent->r.currentOrigin, c.bot.lastPop );
			c.bot.hasHide = Bots::findHidePoint( c.ent, c.bot, c.sense.enemy );
			// cover right where it has just been seen is no cover: go along the line across the enemy's view instead
			if( !c.bot.hasHide || Distance( c.bot.hidePoint, c.bot.lastPop ) < sameSpot )
			{
				Vec3 right;
				const Vec3 facing( 0, c.sense.enemyAngles[YAW], 0 );
				AngleVectors( facing, nullptr, right, nullptr );
				const Vec3 aside = Vec3( c.ent->r.currentOrigin ) + right * (c.bot.breakSide * sidestep);
				VectorCopy( aside, c.bot.hidePoint );
				c.bot.hasHide = true;
				c.bot.breakSide = -c.bot.breakSide;
			}
		}
		return Status::Running;
	}

	static Status exposeForStrike( Mind& c )
	{
		bool guided = false;
		const int slot = BotWeapons::findWeaponSlot( c.client->vehicle_, c.ps, c.sense.enemyAir, c.sense.enemyDist, guided );
		const int lockDelay = slot >= 0 ? availableWeapons[c.veh.weapons[slot]].lockdelay : 0;
		c.bot.exposed = true;
		c.bot.strikeAmmo = guidedAmmo( c );
		c.bot.popTime = theLevel.time_ + lockDelay + 2500 + static_cast<int>( 2000 * c.who().patience );
		return strikeExposed( c );
	}

	// the weapon slot holding crates of that kind, or -1
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
			int need = BotWeapons::supplyNeed( other );
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
		c.steer.standoff = -300.0f;
		c.steer.wantHeight = std::max( dropHeight, heloLowHeight );
		c.cmd.weapon = slot;
		if( c.steer.dist < 120 )
		{
			c.steer.mode = Hover;
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
		GameEntity* leader = BotSenses::leaderOf( c.bot );
		if( !leader )
			return Status::Failure;
		vec3_t flat = { 0, leader->client_->ps_.vehicleAngles[YAW], 0 }, right, spot;
		AngleVectors( flat, nullptr, right, nullptr );
		VectorMA( leader->r.currentOrigin, (c.ent->s.number & 1) ? 350.0f : -350.0f, right, spot );
		flyTo( c, spot );
		c.steer.standoff = -200.0f;
		if( c.steer.dist < 150 )
			c.steer.mode = Hover;
		return Status::Running;
	}

	// nothing to do: on to a roaming goal, a new one when it is reached or stale
	static Status patrol( Mind& c )
	{
		if( theLevel.time_ > c.bot.goalTime || Distance( c.ent->r.currentOrigin, c.bot.goal ) < 800 )
			Bots::pickGoal( c.ent, c.bot, 5000 );
		flyTo( c, c.bot.goal );
		return Status::Running;
	}

	static Status countermeasures( Mind& c )
	{
		BotWeapons::flares( c.ent, c.bot, c.sense, c.cmd );
		return Status::Success;
	}

	// when the straight line to the destination is blocked, follow the air path round the obstacle instead
	static Status navigate( Mind& c )
	{
		Bots::State& bot = c.bot;
		const Vec3 here( c.ent->r.currentOrigin );
		const Vec3 toDest = c.steer.dest - here;
		const float range = toDest.length();
		const Vec3 probeEnd = range > 1500.0f ? here + toDest * (1500.0f / range) : c.steer.dest;
		if( !c.steer.hasDest || c.landed() || range <= 600.0f || AirNav::clearLine( here, probeEnd, 60.0f ) )
		{
			bot.path.clear();
			bot.following = false;
			return Status::Success;
		}
		if( theLevel.time_ >= bot.pathTime )
		{
			AirNav::findPath( here, c.steer.dest, bot.path );
			bot.pathIndex = 0;
			bot.pathTime = theLevel.time_ + 2000;
		}
		// on to the next waypoint once this one is reached or the one after can be flown to directly
		while( bot.pathIndex + 1 < bot.path.size() &&
			((bot.path[bot.pathIndex] - here).length() < 250.0f || AirNav::clearLine( here, bot.path[bot.pathIndex + 1], 60.0f )) )
			bot.pathIndex++;
		bot.following = bot.pathIndex < bot.path.size();
		if( !bot.following )
			return Status::Success;
		const Vec3 waypoint = bot.path[bot.pathIndex];
		Vec3 point;
		waypoint.copyTo( point );
		c.headFor( point );
		c.steer.wantHeight = std::clamp( c.sense.height + (waypoint.z() - here.z()), 60.0f, 1500.0f );
		c.steer.dist = std::max( c.steer.dist, c.steer.standoff + 400.0f );
		return Status::Success;
	}

	// climb or sink to the wanted height above the ground
	static Status collective( Mind& c )
	{
		// never climb into the roof, whatever asked for the height
		constexpr float headroom = 150.0f;
		if( !c.landed() )
		{
			const float highest = c.sense.height + BotWorld::ceilingAbove( c.ent->r.currentOrigin, c.ent->s.number ) - headroom;
			c.steer.wantHeight = std::min( c.steer.wantHeight, std::max( highest, 40.0f ) );
		}
		const float throttle = signedThrottle( c.ps, c.client->vehicle_ );
		float wantThrottle = std::clamp( (c.steer.wantHeight - c.sense.height) / 60.0f, -3.0f, static_cast<float>( c.veh.maxthrottle ) );
		if( c.steer.wait )
			wantThrottle = 0;
		else if( c.landed() )
			wantThrottle = std::max( wantThrottle, c.veh.maxthrottle * 0.6f );
		// setting down: a steady gentle sink for the last stretch
		if( c.steer.hold && c.sense.height < 150 )
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
		Vec3 mins, maxs, end;
		for( int k = 0; k < 3; k++ )
		{
			mins[k] = c.veh.mins[k] - margin;
			maxs[k] = c.veh.maxs[k] + margin;
		}
		VectorMA( c.ent->r.currentOrigin, reach, dir, end );
		trace_t tr;
		GameHost::trace( &tr, c.ent->r.currentOrigin, mins, maxs, end, c.ent->s.number, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
		if( tr.startsolid )
			GameHost::trace( &tr, c.ent->r.currentOrigin, c.veh.mins, c.veh.maxs, end, c.ent->s.number, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
		return tr.startsolid ? 0.0f : tr.fraction * reach;
	}

	// A helicopter goes where it tilts, in any direction, and stops when it levels. The pilot keeps the nose on what it
	// is looking at and tilts toward the clear direction nearest to where it wants to go.
	static Status steer( Mind& c )
	{
		constexpr float maxPitch = 30.0f;
		constexpr float maxRoll = 40.0f;
		constexpr float levelling = 0.75f;
		c.steer.yawError = fabs( AngleSubtract( c.view[YAW], c.ps.vehicleAngles[YAW] ) );

		// how hard to push in the wanted direction, as forward tilt in degrees (negative backs away)
		float push = 0;
		bool orbit = false;
		if( c.sense.height > 60 )
		{
			switch( c.steer.mode )
			{
			case Run:
				push = 14.0f;
				break;
			case ToPad:
				push = std::clamp( c.steer.dist / 60.0f, 3.0f, 14.0f );
				break;
			case MoveToCover:
				push = c.steer.dist > 150 ? 12.0f : 0.0f;
				break;
			case BackOff:
				push = -5.0f;
				break;
			case Hover:
				break;
			default:
				if( c.steer.dist > c.steer.standoff + 300 )
					push = c.sense.height > 300 ? 14.0f : 9.0f;
				else if( c.sense.enemy )
					orbit = true;
				break;
			}
		}

		const float maxSpeed = static_cast<float>( c.veh.maxspeed );
		const float sideSpeed = std::max( 1.0f, maxSpeed );
		float travelYaw = c.view[YAW] + (push < 0 ? 180.0f : 0.0f);
		if( orbit )
		{
			// at fighting distance: keep moving across the enemy's view instead of hanging still, changing
			// direction now and then, and opening the range when too close
			constexpr int orbitPeriod = 6000;
			const float side = (((theLevel.time_ / orbitPeriod) + c.ent->s.number) & 1) ? 1.0f : -1.0f;
			travelYaw = c.view[YAW] + side * (c.steer.dist < c.steer.standoff ? 125.0f : 90.0f);
			push = 9.0f;
		}
		float speed = fabs( push ) / maxPitch * maxSpeed;
		c.steer.blocked = false;
		constexpr float climbSlope = 0.4f;
		if( speed > 1.0f )
		{
			// the wanted direction first, then either side of it, then back the way it came; hover if nothing is open.
			// Rising ground is not an obstacle: where the level line is blocked but a climbing one is clear, go on and climb
			const float reach = 200.0f + speed * 2.0f;
			bool found = false;
			for( float offset : { 0.0f, 45.0f, -45.0f, 90.0f, -90.0f, 135.0f, -135.0f, 180.0f } )
			{
				Vec3 angles = { 0, travelYaw + offset, 0 }, dir, rising;
				AngleVectors( angles, dir, nullptr, nullptr );
				VectorCopy( dir, rising );
				rising[2] = climbSlope;
				const bool level = clearAlong( c, dir, reach ) >= reach;
				if( !level && offset == 0.0f )
					c.steer.blocked = true;
				if( !level && clearAlong( c, rising, reach ) < reach )
					continue;
				travelYaw += offset;
				if( offset != 0.0f )
					speed *= 0.7f;
				if( !level )
				{
					speed *= 0.6f;
					c.steer.wantHeight = std::max( c.steer.wantHeight, std::min( c.sense.height + reach * climbSlope, 1500.0f ) );
				}
				found = true;
				break;
			}
			if( !found )
			{
				speed = 0;
				c.steer.wantHeight = std::max( c.steer.wantHeight, std::min( c.sense.height + 300.0f, 1500.0f ) );
			}
		}

		// the tilt is coarse, so the real motion can differ from the wanted one: stop if it leads into something
		// that cannot be climbed over in time
		vec3_t moving = { c.ps.velocity[0], c.ps.velocity[1], 0 };
		const float drift = VectorNormalize( moving );
		if( drift > 15.0f )
		{
			const float stopping = 80.0f + drift * 0.8f;
			vec3_t rising = { moving[0], moving[1], climbSlope };
			if( clearAlong( c, moving, stopping ) < stopping && clearAlong( c, rising, stopping ) < stopping )
				speed = 0;
		}

		// slow or hovering next to something: ease away from it
		if( speed < 20.0f )
		{
			constexpr float nearby = 140.0f;
			Vec3 away = { 0, 0, 0 };
			for( float around = 0.0f; around < 360.0f; around += 45.0f )
			{
				Vec3 angles = { 0, around, 0 }, dir;
				AngleVectors( angles, dir, nullptr, nullptr );
				const float free = clearAlong( c, dir, nearby );
				if( free < nearby )
					VectorMA( away, -(nearby - free) / nearby, dir, away );
			}
			if( VectorNormalize( away ) > 0.05f )
			{
				Vec3 angles;
				vectoangles( away, angles );
				travelYaw = angles[YAW];
				speed = maxSpeed * 0.12f;
			}
		}

		vec3_t travel = { 0, travelYaw, 0 }, body = { 0, c.ps.vehicleAngles[YAW], 0 }, dir, forward, right;
		AngleVectors( travel, dir, nullptr, nullptr );
		AngleVectors( body, forward, right, nullptr );
		// the wanted motion over the ground as nose-down and bank, each by what that tilt gives in speed
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

	// shoot unless hiding, dodging or flying a path round something
	static Status fire( Mind& c )
	{
		if( c.sense.enemy && !c.weapons.cover && !c.steer.evading && !c.bot.following )
			BotWeapons::fireWeapons( c.ent, c.bot, c.sense, c.cmd, c.steer.yawError );
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

		const auto* stealthStrike = t.selector( "stealth strike", {
			t.when( "at close quarters", tooClose, t.action( "break off", breakOff ) ),
			t.when( "hiding", inCover, t.selector( "cover", {
				t.when( "knows cover", hasCover, t.action( "move to cover", moveToCover ) ),
				t.action( "sink into cover", sinkIntoCover ) } ) ),
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

	// attack helicopters fly low by default, transports higher; the steps change it as they need
	mind.steer.wantHeight = sense.role == Role::AttackHelo ? heloLowHeight : heloCruiseHeight * 0.6f;
	if( !HeloTasks::fighting( mind ) && !HeloTasks::stealthStriking( mind ) )
		bot.exposed = false;
	view[YAW] = ps.vehicleAngles[YAW];
	view[PITCH] = sense.enemy ? std::clamp( AngleSubtract( sense.enemyAngles[PITCH], 0 ), -30.0f, 45.0f ) : 0.0f;

	runTree( tree, mind, "helo" );

	// an open bay shows a stealth helicopter to everyone: it is shut again as soon as nothing is about to leave it
	constexpr int bayTap = 400;
	if( sense.stealth && (ps.ONOFF & OO_WEAPONBAY) && !bot.exposed && (theLevel.time_ / bayTap) % 2 )
		cmd.buttons |= BUTTON_WEAPONBAY;
}
