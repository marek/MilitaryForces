#include "surface.h"
#include "../../world/query.h"
#include "../../weapons/envelope.h"

// stop and lob shells at it from where we are, at the place it was last seen if it is out of sight
behavior::Status SurfaceTasks::shellAirfield( Mind& c )
{
	c.bot.cover.has = false;
	VectorCopy( c.sense.enemyDir, c.steer.course );
	c.steer.hold = true;
	return Status::Running;
}

// away from the enemy, to a place well clear of it found along the ground (or the water) like any other: straight
// back would put a boat on a river into the bank
behavior::Status SurfaceTasks::withdraw( Mind& c )
{
	constexpr float clearOf = 1500.0f;
	c.bot.cover.has = false;
	VectorNegate( c.sense.enemyDir, c.steer.course );
	Vec3 away( c.steer.course );
	away[2] = 0;
	c.travelTo( Vec3( c.ent->r.currentOrigin ) + away.normalized() * clearOf );
	return Status::Running;
}

// Fight on the move, as a tank crew does: drive to a spot in range and in sight of the enemy, round to its side or
// back where one is, firing on the way; stop there to fire for a moment, then on to the next spot somewhere else.
// With no such spot, close in, and stop to shoot only where a shot is possible: two vehicles that each stopped for
// the other out of sight would wait there for the rest of the game
behavior::Status SurfaceTasks::engage( Mind& c )
{
	c.bot.cover.has = false;
	VectorCopy( c.sense.enemyDir, c.steer.course );
	if( keepFiringSpot( c ) )
	{
		c.courseTo( c.bot.firing.point );
		c.steer.hold = theLevel.time_ < c.bot.firing.leaveAt;
		return Status::Running;
	}
	constexpr float closeEnough = 1500.0f;
	const Vec3 enemy = c.enemyPosition();
	c.travelTo( enemy );
	c.steer.hold = c.sense.enemyVisible && c.sense.enemyDist < std::min( closeEnough, weaponReach( c ) );
	return Status::Running;
}

// the furthest a weapon with rounds left reaches the enemy from here
float SurfaceTasks::weaponReach( const Mind& c )
{
	vec3_t at;
	c.enemyPosition().copyTo( at );
	return Envelope::reachOf( c.client->vehicle_, c.ps.ammo, c.sense.enemyAir, Envelope::elevationOf( c.ent->r.currentOrigin, at ) );
}

// the spot being driven to or fired from; a new one when there is none, the enemy changed, it went stale, or the
// time at it is up. False when there is no spot to be had
bool SurfaceTasks::keepFiringSpot( Mind& c )
{
	constexpr float reached = 120.0f;
	constexpr int fireFrom = 4000, reachWithin = 15000;
	Bots::FiringSpot& spot = c.bot.firing;
	const bool arrived = spot.has && Distance( c.ent->r.currentOrigin, spot.point ) < reached;
	if( arrived && !spot.leaveAt )
		spot.leaveAt = theLevel.time_ + fireFrom;
	const bool done = spot.leaveAt && theLevel.time_ >= spot.leaveAt;
	if( spot.has && spot.enemy == c.sense.enemy->s.number && theLevel.time_ < spot.until && !done )
		return true;
	if( spot.has )
	{
		VectorCopy( spot.point, spot.last );
		spot.hasLast = true;
	}
	spot.has = findFiringSpot( c );
	spot.enemy = c.sense.enemy->s.number;
	spot.until = theLevel.time_ + reachWithin;
	spot.leaveAt = 0;
	return spot.has;
}

// A spot on rings round the enemy from about half the weapon's reach outwards: on land, with the enemy in sight from
// the turret's height, not near the last spot; the side or back of the enemy before its front, and near before far
bool SurfaceTasks::findFiringSpot( Mind& c )
{
	constexpr float leastRange = 500.0f, mostRange = 2000.0f, otherSpot = 400.0f, flankWeight = 600.0f;
	const Vec3 enemy = c.enemyPosition();
	const Vec3 here( c.ent->r.currentOrigin );
	vec3_t facing, flat = { 0, c.sense.enemy->client_->ps_.vehicleAngles[YAW], 0 };
	AngleVectors( flat, facing, nullptr, nullptr );
	const Vec3 front( facing );
	PlaceQuery query;
	query.centre = enemy;
	query.passEntity = c.ent->s.number;
	query.firstRing = std::clamp( weaponReach( c ) * 0.5f, leastRange, mostRange );
	query.ringStep = 300.0f;
	query.around = 16;
	query.filters.push_back( PlaceQuery::seenOverFrom( c.ent, c.sense.enemy, c.veh.maxs[2] + 4.0f ) );
	if( c.bot.firing.hasLast )
	{
		const Vec3 last( c.bot.firing.last );
		query.filters.push_back( [last]( const Vec3& p ) { return (p - last).length() > otherSpot; } );
	}
	query.scorers.push_back( PlaceQuery::nearer( here ) );
	query.scorers.push_back( [enemy, front]( const Vec3& p )
	{
		Vec3 from = p - enemy;
		from[2] = 0;
		return (1.0f - from.normalized().dot( front )) * flankWeight;
	} );
	Vec3 found;
	if( !query.find( found ) )
		return false;
	found.copyTo( c.bot.firing.point );
	return true;
}

// lay the gun on the enemy; a shell is led by the target's movement and raised for its drop
behavior::Status SurfaceTasks::gunnery( Mind& c )
{
	c.view[YAW] = c.sense.enemyAngles[YAW];
	c.view[PITCH] = std::clamp( AngleSubtract( c.sense.enemyAngles[PITCH], 0 ), -60.0f, 20.0f );
	const int selected = c.veh.weapons[c.ps.weaponNum];
	if( selected && availableWeapons[selected].type == WT_BALLISTICGUN && availableWeapons[selected].muzzleVelocity > 0 )
	{
		const float shell = static_cast<float>( availableWeapons[selected].muzzleVelocity );
		const float flight = c.sense.enemyDist / shell;
		Vec3 aim, dirAim, aimAngles, known;
		c.enemyPosition().copyTo( known );
		VectorMA( known, flight, c.sense.enemy->client_->ps_.velocity, aim );
		dirAim = aim - c.ent->r.currentOrigin;
		const float range = VectorLength( dirAim );
		vectoangles( dirAim, aimAngles );
		// the elevation that carries a shell of this speed that far
		const float arc = 0.5f * asin( std::clamp( DEFAULT_GRAVITY * range / (shell * shell), 0.0f, 1.0f ) ) * 180.0f / std::numbers::pi_v<float>;
		c.view[YAW] = aimAngles[YAW];
		c.view[PITCH] = std::clamp( AngleSubtract( aimAngles[PITCH], 0 ) - arc, -60.0f, 20.0f );
	}
	const float turretYaw = c.ps.vehicleAngles[YAW] + c.ps.turretAngle;
	BotWeapons::fireWeapons( c.ent, c.bot, c.sense, c.cmd, fabs( AngleSubtract( c.view[YAW], turretYaw ) ) );
	return Status::Success;
}
