#include "crew.h"
#include "../navigation/intercept.h"
#include <cmath>

// turrets: look at a nearby enemy aircraft and hold the trigger, as a player does with free look held, flying
// straight meanwhile. The positions swing round at their own rate and one that has not arrived does not fire,
// so the look has to last: four seconds is the longest any position needs to come right round. Then a second
// to steer in. Never while there is something to fly round, nor while landing, taking off or low
behavior::Status Crew::gunner( Bots::Mind& c )
{
	constexpr int turretBurst = 4000, turretPause = 1000;
	// the target is the nearest enemy aircraft, whatever the bomber itself is after: a bomber's own enemy is on the ground
	GameEntity* raider = c.sense.airThreat && c.sense.airThreatDist < turretRange ? c.sense.airThreat : nullptr;
	if( (c.veh.flags & VF_GUNSTATIONS) && raider && !(c.ps.ONOFF & OO_LANDED) && c.bot.desire != Bots::Goal::Rearm &&
		c.sense.height > c.minHeight && c.ps.ammo[WP_MACHINEGUN] > 0 && BotWorld::visible( c.ent, c.ent->r.currentOrigin, raider ) )
	{
		if( theLevel.time_ > c.bot.turretTime + turretBurst + turretPause )
			c.bot.turretTime = theLevel.time_;
		if( theLevel.time_ < c.bot.turretTime + turretBurst && theLevel.time_ >= c.bot.avoid.time )
		{
			// lead the target by the bullets' flight time
			constexpr float noLimit = 1e6f;
			const float shotSpeed = static_cast<float>( availableWeapons[c.veh.weapons[WP_MACHINEGUN]].muzzleVelocity );
			const Vec3 aimAt = Intercept::lead( c.ent->r.currentOrigin, raider->r.currentOrigin, raider->s.pos.trDelta, shotSpeed, noLimit ) - Vec3( c.ent->r.currentOrigin );
			Vec3 aim;
			vectoangles( aimAt, aim );
			// a gunner in a bucking bomber is no marksman: his aim wanders about the target, a few degrees for a
			// poor gunner and under one for the best, so that the fire walks across the target and off it again
			// rather than a whole burst going wide
			constexpr float wanderYawMsec = 230.0f, wanderPitchMsec = 310.0f;
			const float spread = Bots::handicap( c.bot ).gunnerSpread;
			const float phase = static_cast<float>( c.ent->s.number ) * 1.7f;
			aim[YAW] += spread * sin( theLevel.time_ / wanderYawMsec + phase );
			aim[PITCH] += spread * sin( theLevel.time_ / wanderPitchMsec + phase * 2.0f );
			if( VehicleData::gunStationsBearing( c.client->vehicle_, c.ps.vehicleAngles, aim ) )
			{
				VectorCopy( aim, c.view );
				c.cmd.buttons |= BUTTON_FREELOOK | BUTTON_ATTACK;
			}
		}
	}
	return behavior::Status::Success;
}
