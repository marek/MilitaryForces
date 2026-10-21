#include "surface.h"
#include "../../weapons/envelope.h"

// stop and lob shells at it from where we are, at the place it was last seen if it is out of sight
behavior::Status SurfaceTasks::shellAirfield( Mind& c )
{
	c.bot.cover.has = false;
	VectorCopy( c.sense.enemyDir, c.steer.course );
	c.steer.hold = true;
	return Status::Running;
}

// straight away from the enemy
behavior::Status SurfaceTasks::withdraw( Mind& c )
{
	c.bot.cover.has = false;
	VectorNegate( c.sense.enemyDir, c.steer.course );
	return Status::Running;
}

// Close in, and stop to shoot only where a shot is possible: the enemy in sight and inside the reach of a weapon with
// rounds left. One behind a rise or out of reach is driven towards, where it is or was last seen; two vehicles that
// each stopped for the other out of sight would wait there for the rest of the game
behavior::Status SurfaceTasks::engage( Mind& c )
{
	constexpr float closeEnough = 1500.0f;
	c.bot.cover.has = false;
	VectorCopy( c.sense.enemyDir, c.steer.course );
	const Vec3 enemy = c.enemyPosition();
	c.travelTo( enemy );
	vec3_t at;
	enemy.copyTo( at );
	const float reach = Envelope::reachOf( c.client->vehicle_, c.ps.ammo, c.sense.enemyAir, Envelope::elevationOf( c.ent->r.currentOrigin, at ) );
	c.steer.hold = c.sense.enemyVisible && c.sense.enemyDist < std::min( closeEnough, reach );
	return Status::Running;
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
