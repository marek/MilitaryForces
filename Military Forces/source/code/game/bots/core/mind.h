#ifndef __BOT_MIND_H__
#define __BOT_MIND_H__

#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "local.h"
#include "../navigation/groundnav.h"
#include <algorithm>
#include <cmath>

// One bot's thinking for one frame: what it is, what it senses, the command being built, and the wishes that the
// steps of its behaviour tree leave for the steps after them.
struct Bots::Mind
{
	GameEntity*						ent;
	GameClient*						client;
	State&							bot;
	const Sense&					sense;
	usercmd_t&						cmd;
	float*							view;	// the angles the bot looks along, which the vehicle follows
	const playerState_t&			ps;
	const completeVehicleData_t&	veh;
	float							speed;	// as the instruments show it
	float							cruise = 0;	// the height a plane wants to fly at
	bool							attacking = false;	// pressing an attack: guns and missiles as they bear
	bool							snapShot = false;	// not attacking, but the enemy is crossing the nose
	bool							bombing = false;	// on a bomb run: the heading is not to be changed
	bool							direct = false;	// this step works the controls itself; the flying steps stay out of it
	int								forcedWeapon = -1;	// a step needs this weapon kept selected
	bool							defending = false;	// dodging a missile or breaking away
	bool							weaponsFree = false;	// may shoot whenever the aim allows
	bool							hasDest = false;
	Vec3							dest;	// where the step wants to get to; navigation finds the way
	float							airspeed = 0;	// speed through the air, for turn radius and timing
	float							ceiling = 0;	// room above
	float							room = 0;	// floor to ceiling
	float							minHeight = 0;	// the least height above ground to fly at here
	float							dist = 0;	// to the place steered for
	float							wantHeight = 0;	// a helicopter's height above ground; negative means set down
	float							wantPitch = 0;
	float							yawError = 0;	// how far the nose is off the wanted heading
	bool							cover = false;	// hiding: do not shoot
	bool							wait = false;	// sit on the ground
	bool							hold = false;	// stop here
	bool							blocked = false;	// something is in the way ahead
	bool							firing = false;
	vec3_t							course = { 0, 0, 0 };	// the direction a surface vehicle or a soldier wants to go
	int								mode = 0;	// a helicopter's manner of moving
	float							standoff = 0;	// the distance to keep from the place steered for
	bool							boat = false;
	GameEntity*						target = nullptr;	// a vehicle to board, or a walker to pick up
	float							targetDist = 0;
	int								nearPad = -1;	// a soldier's nearest pad
	float							nearPadDist = 0;

	bool landed() const { return (ps.ONOFF & OO_LANDED) != 0; }
	const Character& who() const { return Bots::character( bot ); }
	// name the place to get to
	void travelTo( const Vec3& point )
	{
		dest = point;
		hasDest = true;
	}
	// head straight for a point on the ground and name it as the place to get to
	void courseTo( const vec3_t point )
	{
		VectorSubtract( point, ent->r.currentOrigin, course );
		course[2] = 0;
		travelTo( Vec3( point ) );
	}
	// turn the course onto the ground path to the destination, searching again every two seconds; false when
	// there is no path to follow or none is needed
	// (reach: how near a waypoint counts as reached; finish: how near the destination the path is let go and the rest
	// driven straight, twice the reach unless given)
	bool followGround( bool water, float reach, float finish = -1.0f )
	{
		const Vec3 here( ent->r.currentOrigin );
		if( !hasDest || hold || (dest - here).length() < (finish >= 0.0f ? finish : reach * 2.0f) )
		{
			bot.path.clear();
			bot.following = false;
			return false;
		}
		if( theLevel.time_ >= bot.pathTime )
		{
			GroundNav::findPath( here, dest, water, bot.path );
			bot.pathIndex = 0;
			bot.pathTime = theLevel.time_ + 2000;
		}
		while( bot.pathIndex + 1 < bot.path.size() )
		{
			const Vec3 leg = bot.path[bot.pathIndex] - here;
			if( leg.x() * leg.x() + leg.y() * leg.y() > reach * reach )
				break;
			bot.pathIndex++;
		}
		bot.following = bot.pathIndex < bot.path.size();
		if( !bot.following )
			return false;
		const Vec3 leg = bot.path[bot.pathIndex] - here;
		VectorSet( course, leg.x(), leg.y(), 0 );
		return true;
	}
	// where the enemy is, or was last seen
	Vec3 enemyPosition() const
	{
		return sense.enemyVisible ? Vec3( sense.enemy->r.currentOrigin ) : Vec3( bot.seen[sense.enemy->s.number].position );
	}

	// the radius of the tightest turn at the present speed
	float turnRadius() const
	{
		return std::max( VectorLength( ps.velocity ), 1.0f ) / (std::max( 10.0f, veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f);
	}
	// pitch to come back to the cruising height
	void holdAltitude()
	{
		view[PITCH] = std::clamp( (sense.height - cruise) / 30.0f, -30.0f, 20.0f );
	}
	// turn towards a point
	void headFor( const vec3_t point )
	{
		Vec3 dir, angles;
		VectorSubtract( point, ent->r.currentOrigin, dir );
		dir[2] = 0;
		vectoangles( dir, angles );
		view[YAW] = angles[YAW];
	}
};

// the steps of a plane's landing, in order
struct LandingSteps
{
	static behavior::Status survey( Bots::Mind& mind );
	static behavior::Status approach( Bots::Mind& mind );
	static behavior::Status final( Bots::Mind& mind );
	static behavior::Status rollOut( Bots::Mind& mind );
	static behavior::Status taxi( Bots::Mind& mind );
	static behavior::Status refuel( Bots::Mind& mind );
	static behavior::Status leaveRunway( Bots::Mind& mind );
};

// run a behaviour tree for one frame; the debug build reports each change of step
inline void runTree( const behavior::Tree<Bots::Mind>& tree, Bots::Mind& mind, const char* kind )
{
	tree.tick( mind, mind.bot.tree, theLevel.time_ );
	mind.bot.treeUsed = &tree;
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 && mind.bot.tree.leaf != mind.bot.tree.previousLeaf )
		Common::printf( "BOTNODE t %d bot %d %s: %s\n", theLevel.time_, mind.ent->s.number, kind, tree.nameOf( mind.bot.tree.leaf ) );
#else
	static_cast<void>( kind );
#endif
}

#endif
