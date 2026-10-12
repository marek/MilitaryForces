#include "surface.h"

// onto the pad and wait there until full
behavior::Status SurfaceTasks::driveToPad( Mind& c )
{
	c.courseTo( BotWorld::pads_[c.bot.rearm.pad].spot );
	holdNear( c, 80 );
	if( c.steer.hold && Bots::replenished( c.ent ) )
	{
		Bots::finishRearm( c.ent, c.bot );
		return Status::Success;
	}
	return Status::Running;
}

// pull up beside a team mate on foot and wait so they can climb aboard
behavior::Status SurfaceTasks::pickUpWalker( Mind& c )
{
	c.bot.cover.has = false;
	c.courseTo( c.target->r.currentOrigin );
	holdNear( c, 60.0f + c.veh.maxs[1] );
	return Status::Running;
}

// take the passenger to the nearest free vehicle we know of, otherwise home to base, and stop there
behavior::Status SurfaceTasks::deliverPassenger( Mind& c )
{
	constexpr float knownRange = 6000.0f;
	const float* destination = c.bot.hasHome ? c.bot.home : nullptr;
	float best = knownRange;
	for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !other || !other->inuse_ || other->health_ <= 0 || other->s.eType != ET_MISC_VEHICLE || !(other->s.seatMask & SEATMASK_ENTERABLE) )
			continue;
		const float dist = Distance( c.ent->r.currentOrigin, other->r.currentOrigin );
		if( dist < best )
		{
			best = dist;
			destination = other->r.currentOrigin;
		}
	}
	if( !destination )
		return Status::Failure;
	c.bot.cover.has = false;
	c.courseTo( destination );
	holdNear( c, 150 );
	return Status::Running;
}

// short of fuel, health or ammunition and a pickup that helps is lying nearby: go and get it
behavior::Status SurfaceTasks::driveToSupplies( Mind& c )
{
	// a real shortage is worth a detour; topping up only when the supplies are close by
	const int urgent = BotWeapons::supplyNeed( c.ent, true );
	const int need = urgent ? urgent : BotWeapons::supplyNeed( c.ent );
	if( !need )
		return Status::Failure;
	const float range = urgent ? 3000.0f : 1200.0f;
	const int wanted = need == WT_FUELCRATE ? IT_FUEL : need == WT_HEALTHCRATE ? IT_HEALTH : IT_AMMO;
	GameEntity* nearest = nullptr;
	float best = range;
	for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
	{
		GameEntity* item = theLevel.getEntity( i );
		if( !item || !item->inuse_ || item->s.eType != ET_ITEM || !item->item_ || item->item_->giType != wanted || (item->s.eFlags & EF_NODRAW) )
			continue;
		const float dist = Distance( c.ent->r.currentOrigin, item->r.currentOrigin );
		if( dist < best )
		{
			nearest = item;
			best = dist;
		}
	}
	if( !nearest )
		return Status::Failure;
	c.courseTo( nearest->r.currentOrigin );
	return Status::Running;
}

// drive with the leader, stopping a little short
behavior::Status SurfaceTasks::convoy( Mind& c )
{
	GameEntity* leader = BotSenses::leaderOf( c.bot );
	if( !leader )
		return Status::Failure;
	c.bot.cover.has = false;
	c.courseTo( leader->r.currentOrigin );
	holdNear( c, 350 );
	return Status::Running;
}
