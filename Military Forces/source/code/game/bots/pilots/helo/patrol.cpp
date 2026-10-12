#include "helo.h"

// head for a point at the present height
void HeloTasks::flyTo( Mind& c, const vec3_t point )
{
	Vec3 dir;
	VectorSubtract( point, c.ent->r.currentOrigin, dir );
	dir[2] = 0;
	c.steer.dist = VectorLength( dir );
	c.headFor( point );
	c.travelTo( Vec3( point[0], point[1], c.ent->r.currentOrigin[2] ) );
	c.steer.mode = Cruise;
}

// to the flag, and down onto it at the end: a flag is touched near the ground
behavior::Status HeloTasks::goToFlag( Mind& c )
{
	flyTo( c, c.bot.goal );
	if( c.steer.dist < 500 )
		c.steer.wantHeight = 25.0f;
	return Status::Running;
}

// stay beside the leader, on the side given by the client number
behavior::Status HeloTasks::escort( Mind& c )
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
behavior::Status HeloTasks::patrol( Mind& c )
{
	if( theLevel.time_ > c.bot.goalTime || Distance( c.ent->r.currentOrigin, c.bot.goal ) < 800 )
		Bots::pickGoal( c.ent, c.bot, 5000 );
	flyTo( c, c.bot.goal );
	return Status::Running;
}
