#ifndef __BOT_HELO_H__
#define __BOT_HELO_H__

#include "../../core/mind.h"
#include "../../navigation/airnav.h"
#include "../../crew/crew.h"
#include "../../control/control.h"
#include "../../navigation/context.h"
#include "../../world/threat.h"

using behavior::Status;

// What a helicopter pilot does: the conditions and steps of the helicopter tree. The steps say where to go, at what
// height and in which manner; steering and the collective at the end turn that into the controls. The conditions are
// here; the steps of each branch are in the file named after it, and the tree in tree.cpp.
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
		VectorSubtract( BotWorld::pads_[c.bot.rearm.pad].center, c.ent->r.currentOrigin, dir );
		dir[2] = 0;
		return VectorLength( dir ) < 120;
	}

	static bool rearming( Mind& c ) { return c.bot.desire == Goal::Rearm && c.bot.rearm.pad >= 0; }

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
	static Status pickUpWalker( Mind& c );

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
	static bool outranged( Mind& c ) { return c.sense.outranged && !c.sense.blindsided; }
	// near enough to fight from cover

	static bool fighting( Mind& c ) { return c.sense.enemy && c.bot.desire == Goal::Attack && c.sense.enemyDist < 3000; }

	static bool inCover( Mind& c ) { return theLevel.time_ < c.bot.cover.time; }

	static bool hasCover( Mind& c ) { return c.bot.cover.has; }

	static bool exposed( Mind& c ) { return c.bot.cover.exposed; }

	static bool seesEnemy( Mind& c ) { return c.sense.enemyVisible; }

	static bool hasEnemy( Mind& c ) { return c.sense.enemy != nullptr; }

	static bool airborne( Mind& c ) { return !c.landed(); }

	static bool hasLeader( Mind& c ) { return c.bot.leader >= 0; }

	static bool carriesCrates( Mind& c )
	{
		return crateSlot( c, WT_FUELCRATE ) >= 0 || crateSlot( c, WT_AMMOCRATE ) >= 0 || crateSlot( c, WT_HEALTHCRATE ) >= 0;
	}

	// nose on the enemy, to stand off at gun range from an aircraft and further from the ground
	static void faceEnemy( Mind& c );

	// head for a point at the present height
	static void flyTo( Mind& c, const vec3_t point );

	// sit on the pad, collective down, until full
	static Status rearmOnPad( Mind& c );

	// over the pad: gear down and straight down
	static Status descendToPad( Mind& c );

	// to the pad at a moderate height, slowing as it nears
	static Status flyToPad( Mind& c );

	// across the missile's path, low and fast
	static Status dodgeMissile( Mind& c );

	static Status waitForPassengers( Mind& c );

	// away from the enemy, or to the side when hit by someone unseen
	static Status evade( Mind& c );
	static Status leaveReach( Mind& c );

	// to the flag, and down onto it at the end: a flag is touched near the ground
	static Status goToFlag( Mind& c );

	// to the hiding place, low, facing the enemy
	static Status moveToCover( Mind& c );

	// no hiding place known: back off low
	static Status sinkIntoCover( Mind& c );

	// up in view and shooting until the time in the open is used up, then back into cover for a while
	static Status fireExposed( Mind& c );

	// the enemy is in sight: start the time in the open, longer for the patient
	static Status expose( Mind& c );

	// rise until the enemy comes into view
	static Status popUp( Mind& c );

	// too far to fight: fly to where the enemy is or was last seen
	static Status closeOnEnemy( Mind& c );

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
		c.bot.cover.breakingOff = dist < (c.bot.cover.breakingOff ? clearOfThem : closeQuarters);
		return c.bot.cover.breakingOff;
	}

	static bool outOfStrikeRange( Mind& c ) { return Distance( c.sense.enemy->r.currentOrigin, c.ent->r.currentOrigin ) > strikeRange( c ); }

	static Status breakOff( Mind& c );

	// towards the enemy, low, weapons held: nothing that would show it before it is ready
	static Status stalk( Mind& c );

	// in view at its firing place. It goes back down as soon as a missile has left, or when the time is up
	static Status strikeExposed( Mind& c );

	static Status exposeForStrike( Mind& c );

	// the weapon slot holding crates of that kind, or -1
	static int crateSlot( const Mind& c, int type );

	// Carrying crates: find a team mate on the ground who is short of something we carry, a real player first,
	// hover over them and drop it.
	static Status resupply( Mind& c );

	// stay beside the leader, on the side given by the client number
	static Status escort( Mind& c );

	// nothing to do: on to a roaming goal, a new one when it is reached or stale
	static Status patrol( Mind& c );

	// when the straight line to the destination is blocked, follow the air path round the obstacle instead
	static Status navigate( Mind& c );

	// climb or sink to the wanted height above the ground
	static Status collective( Mind& c );

	// free distance for the whole airframe along a direction, up to reach
	static float clearAlong( const Mind& c, const vec3_t dir, float reach );

	// A helicopter goes where it tilts, in any direction, and stops when it levels. The pilot keeps the nose on what it
	// is looking at and tilts toward the clear direction nearest to where it wants to go.
	static Status steer( Mind& c );

	// where to move this frame, and how fast over the ground
	struct Travel
	{
		float	yaw = 0;
		float	speed = 0;
	};
	static Travel wantedTravel( const Mind& c );
	static void findOpenDirection( Mind& c, Travel& travel );
	static void avoidThreats( const Mind& c, ContextMap& map, float reach );
	static void stopBeforeDrift( const Mind& c, Travel& travel );
	static void easeAwayFromWalls( const Mind& c, Travel& travel );

	// shoot unless hiding, dodging or flying a path round something
	static Status fire( Mind& c );

	static behavior::Tree<Mind> build();
};

#endif
