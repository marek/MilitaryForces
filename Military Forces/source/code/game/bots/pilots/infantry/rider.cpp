#include "infantry.h"

// A bot riding in someone's vehicle: a gunner works the turret, a passenger in a helicopter or a boat shoots at what
// comes near, and a passenger in a wheeled transport gets out when it has stopped for a while
void Bots::thinkRider( GameEntity* ent, State& bot, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	GameEntity* pilot = theLevel.getEntity( client->sess_.ridingClient_ );
	if( !alive( pilot ) )
		return;
	VectorCopy( pilot->client_->ps_.vehicleAngles, view );
	view[ROLL] = 0;
	const bool gunner = client->sess_.seat_ == 1 && VehicleData::vehicleHasTurret( pilot->client_->vehicle_ );
	if( gunner )
		bot.liftTime = 0;
	else if( leaveStoppedRide( ent, bot, pilot ) )
		return;
	// passengers shoot only from open vehicles
	if( !gunner && !(availableVehicles[pilot->client_->vehicle_].cat & (CAT_HELO|CAT_BOAT)) )
		return;
	if( theLevel.time_ >= bot.enemyTime )
	{
		GameEntity* found = nearestFromSeat( ent, pilot, gunner ? 3500.0f : 1500.0f );
		bot.enemy = found ? found->s.number : -1;
		bot.enemyTime = theLevel.time_ + 800;
	}
	GameEntity* enemy = bot.enemy >= 0 ? theLevel.getEntity( bot.enemy ) : nullptr;
	if( !alive( enemy ) )
		return;
	Vec3 dir, angles;
	dir = enemy->r.currentOrigin - pilot->r.currentOrigin;
	vectoangles( dir, angles );
	view[YAW] = angles[YAW];
	view[PITCH] = std::clamp( AngleSubtract( angles[PITCH], 0 ), -60.0f, 60.0f );
	if( BotWorld::visible( pilot, pilot->r.currentOrigin, enemy ) )
		cmd.buttons |= gunner ? BUTTON_ATTACK_MAIN : BUTTON_ATTACK;
}

// a passenger in a wheeled transport gets out once it has stopped for a while; true when it has just got out
bool Bots::leaveStoppedRide( GameEntity* ent, State& bot, GameEntity* pilot )
{
	constexpr int rideTime = 6000;
	const completeVehicleData_t& ride = availableVehicles[pilot->client_->vehicle_];
	if( !(ride.cat & CAT_GROUND) || !(ride.caps & HC_WHEELS) || VectorLength( pilot->client_->ps_.velocity ) >= 15 )
	{
		bot.liftTime = 0;
		return false;
	}
	if( !bot.liftTime )
	{
		bot.liftTime = theLevel.time_;
		return false;
	}
	if( theLevel.time_ <= bot.liftTime + rideTime || theLevel.time_ <= bot.commandTime )
		return false;
	bot.liftTime = 0;
	bot.commandTime = theLevel.time_ + 2000;
	Cmd_Eject_f( ent );
	return true;
}

// the nearest enemy within reach of the seat's weapon
GameEntity* Bots::nearestFromSeat( GameEntity* ent, GameEntity* pilot, float reach )
{
	GameEntity* enemy = nullptr;
	float best = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || other == pilot || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
			continue;
		const float dist = Distance( pilot->r.currentOrigin, other->r.currentOrigin );
		if( dist < reach && (!enemy || dist < best) )
		{
			enemy = other;
			best = dist;
		}
	}
	return enemy;
}
