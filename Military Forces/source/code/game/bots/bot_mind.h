#ifndef __BOT_MIND_H__
#define __BOT_MIND_H__

#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include "bot_groundnav.h"
#include <algorithm>
#include <cmath>

struct Bots::Mind
{
	GameEntity*						ent;
	GameClient*						client;
	State&							bot;
	const Sense&					sense;
	usercmd_t&						cmd;
	float*							view;
	const playerState_t&			ps;
	const completeVehicleData_t&	veh;
	float							speed;
	float							cruise = 0;
	bool							attacking = false;
	bool							snapShot = false;
	bool							bombing = false;
	bool							direct = false;
	int								forcedWeapon = -1;
	bool							defending = false;
	bool							weaponsFree = false;
	bool							hasDest = false;
	Vec3							dest;
	float							airspeed = 0;
	float							ceiling = 0;
	float							room = 0;
	float							minHeight = 0;
	float							dist = 0;
	float							wantHeight = 0;
	float							wantPitch = 0;
	float							yawError = 0;
	bool							cover = false;
	bool							wait = false;
	bool							hold = false;
	bool							blocked = false;
	bool							firing = false;
	vec3_t							course = { 0, 0, 0 };
	int								mode = 0;
	float							standoff = 0;
	bool							boat = false;
	GameEntity*						target = nullptr;
	float							targetDist = 0;
	int								nearPad = -1;
	float							nearPadDist = 0;

	bool landed() const { return (ps.ONOFF & OO_LANDED) != 0; }
	const Character& who() const { return Bots::character( bot ); }
	void travelTo( const Vec3& point )
	{
		dest = point;
		hasDest = true;
	}
	void courseTo( const vec3_t point )
	{
		VectorSubtract( point, ent->r.currentOrigin, course );
		course[2] = 0;
		travelTo( Vec3( point ) );
	}
	bool followGround( bool water, float reach )
	{
		const Vec3 here( ent->r.currentOrigin );
		if( !hasDest || hold || (dest - here).length() < reach * 2.0f )
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
	Vec3 enemyPosition() const
	{
		return sense.enemyVisible ? Vec3( sense.enemy->r.currentOrigin ) : Vec3( bot.seen[sense.enemy->s.number].position );
	}

	float turnRadius() const
	{
		return std::max( VectorLength( ps.velocity ), 1.0f ) / (std::max( 10.0f, veh.turnspeed[YAW] ) * std::numbers::pi_v<float> / 180.0f);
	}
	void holdAltitude()
	{
		view[PITCH] = std::clamp( (sense.height - cruise) / 30.0f, -30.0f, 20.0f );
	}
	void headFor( const vec3_t point )
	{
		vec3_t dir, angles;
		VectorSubtract( point, ent->r.currentOrigin, dir );
		dir[2] = 0;
		vectoangles( dir, angles );
		view[YAW] = angles[YAW];
	}
};

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

inline void runTree( const behavior::Tree<Bots::Mind>& tree, Bots::Mind& mind, const char* kind )
{
	tree.tick( mind, mind.bot.tree );
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 2 && mind.bot.tree.leaf != mind.bot.tree.previousLeaf )
		Common::printf( "BOTNODE t %d bot %d %s: %s\n", theLevel.time_, mind.ent->s.number, kind, tree.nameOf( mind.bot.tree.leaf ) );
#else
	static_cast<void>( kind );
#endif
}

#endif
