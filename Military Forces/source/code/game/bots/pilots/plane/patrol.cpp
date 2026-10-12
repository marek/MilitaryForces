#include "plane.h"

// towards the roaming goal at the present height, a new goal when near it or when it is stale
void PlaneTasks::roam( Mind& c )
{
	if( theLevel.time_ > c.bot.goalTime || Distance( c.ent->r.currentOrigin, c.bot.goal ) < 2000 )
		Bots::pickGoal( c.ent, c.bot, 9000 );
	c.headFor( c.bot.goal );
	c.holdAltitude();
	c.travelTo( Vec3( c.bot.goal[0], c.bot.goal[1], c.ent->r.currentOrigin[2] ) );
}

// roam; the height itself comes from the wanted altitude
behavior::Status PlaneTasks::gainHeight( Mind& c )
{
	roam( c );
	return Status::Running;
}

behavior::Status PlaneTasks::wingman( Mind& c )
{
	GameEntity* leader = BotSenses::leaderOf( c.bot );
	if( !leader )
		return Status::Failure;
	// off the leader's wing, on the side given by the client number so two wingmen take different sides
	Vec3 forward, right;
	AngleVectors( leader->client_->ps_.vehicleAngles, forward, right, nullptr );
	VectorMA( leader->r.currentOrigin, 600.0f, forward, c.bot.goal );
	VectorMA( c.bot.goal, (c.ent->s.number & 1) ? 300.0f : -300.0f, right, c.bot.goal );
	c.bot.goalTime = theLevel.time_ + 2000;
	c.headFor( c.bot.goal );
	c.holdAltitude();
	c.travelTo( Vec3( c.bot.goal[0], c.bot.goal[1], c.ent->r.currentOrigin[2] ) );
	return Status::Running;
}

// nothing to do: roam
behavior::Status PlaneTasks::patrol( Mind& c )
{
	roam( c );
	return Status::Running;
}
