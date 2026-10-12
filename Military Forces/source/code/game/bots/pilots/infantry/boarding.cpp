#include "infantry.h"

// the nearest thing with a seat: an empty vehicle, or a team mate's stopped vehicle. A free gunner's seat is worth a longer walk
void InfantryTasks::findRide( Mind& c )
{
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !other || !other->inuse_ || other == c.ent || other->health_ <= 0 )
			continue;
		bool candidate = false;
		if( other->s.eType == ET_MISC_VEHICLE && (other->s.seatMask & SEATMASK_ENTERABLE) )
			candidate = true;
		else if( other->client_ && alive( other ) && i <= theLevel.maxclients_ && other->client_->sess_.ridingClient_ < 0 &&
			!(availableVehicles[other->client_->vehicle_].cat & (CAT_LQM|CAT_PLANE)) &&
			g_gametype.integer >= GT_TEAM && OnSameTeam( c.ent, other ) &&
			VectorLength( other->client_->ps_.velocity ) < 40 )
			candidate = true;
		if( !candidate )
			continue;
		float dist = Distance( c.ent->r.currentOrigin, other->r.currentOrigin );
		if( dist > 4000 )
			continue;
		if( other->client_ && VehicleData::vehicleHasTurret( other->client_->vehicle_ ) && !(other->s.seatMask & (1 << 1)) )
			dist *= 0.4f;
		if( !c.target || dist < c.targetDist )
		{
			c.target = other;
			c.targetDist = dist;
		}
	}
}

// the nearest pad, where vehicles appear
void InfantryTasks::nearestPad( Mind& c )
{
	if( !BotWorld::padsBuilt_ )
		BotWorld::buildPads();
	for( size_t i = 0; i < BotWorld::pads_.size(); i++ )
	{
		const float dist = Distance( c.ent->r.currentOrigin, BotWorld::pads_[i].spot );
		if( c.nearPad < 0 || dist < c.nearPadDist )
		{
			c.nearPad = static_cast<int>( i );
			c.nearPadDist = dist;
		}
	}
}

// the enter key
behavior::Status InfantryTasks::board( Mind& c )
{
	c.bot.commandTime = theLevel.time_ + 1500;
	c.controls.claim();
	Cmd_Eject_f( c.ent );
	return Status::Success;
}

behavior::Status InfantryTasks::walkToRide( Mind& c )
{
	c.courseTo( c.target->r.currentOrigin );
	return Status::Running;
}

// choose a vehicle and give up this life for it
behavior::Status InfantryTasks::respawnInVehicle( Mind& c )
{
	if( theLevel.time_ <= c.bot.commandTime )
		return Status::Failure;
	int teamFlag = MF_TEAM_ANY;
	if( g_gametype.integer >= GT_TEAM )
		teamFlag = c.client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;
	const int vehicle = Bots::pickVehicle( teamFlag, false, &c.who(), &c.bot );
	if( vehicle < 0 )
		return Status::Failure;
	c.bot.commandTime = theLevel.time_ + 2000;
	c.bot.vehicle = vehicle;
	c.controls.claim();
	char userinfo[MAX_INFO_STRING];
	GameHost::getUserinfo( c.ent->s.number, userinfo, sizeof( userinfo ) );
	Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", vehicle ) );
	GameHost::setUserinfo( c.ent->s.number, userinfo );
	ClientUserinfoChanged( c.ent->s.number );
	Cmd_Kill_f( c.ent );
	return Status::Success;
}

behavior::Status InfantryTasks::walkToPad( Mind& c )
{
	c.courseTo( BotWorld::pads_[c.nearPad].spot );
	return Status::Running;
}
