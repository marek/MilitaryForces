#ifndef __BOT_LOCAL_H__
#define __BOT_LOCAL_H__

// client commands a bot gives as a player would
void SetTeam( GameEntity *ent, char *s );
void Cmd_Radar_f( GameEntity *ent );
void Cmd_Eject_f( GameEntity *ent );
void Cmd_Kill_f( GameEntity *ent );
void ClientUserinfoChanged( int clientNum );
void MF_ClientBegin( int clientNum );

namespace bots
{
	constexpr signed char moveFull = 127;	// a movement key held fully
	constexpr signed char moveNone = 0;
	constexpr float planeCruiseHeight = 1400.0f;	// heights above ground that pilots keep to
	constexpr float planeMinHeight = 450.0f;
	constexpr float bomberMinRunHeight = 800.0f;	// a bomb run is flown this high at least, and no higher than the top figure:
	constexpr float bomberMaxRunHeight = 1800.0f;	// higher costs a slow bomber minutes of climbing and its aim on anything that moves
	constexpr float bomberCeilingMargin = 700.0f;	// kept clear under a roof or the top of the map

	// the height above ground a bomber flies its run at, given its present height and the open air above it
	inline float bomberRunHeight( float height, float ceilingAbove )
	{
		return std::clamp( height + ceilingAbove - bomberCeilingMargin, bomberMinRunHeight, bomberMaxRunHeight );
	}
	// the least height over the ground while lining up to land
	constexpr float approachFloor = 150.0f;
	constexpr float heloCruiseHeight = 500.0f;
	constexpr float heloLowHeight = 180.0f;
	constexpr float gunRange = 2500.0f;	// how far off a pilot opens fire with the gun
	constexpr float turretRange = 2200.0f;	// how far off a bomber's gunners open fire
	constexpr float passEnds = 500.0f;	// a firing pass on an aircraft is broken off this close
	constexpr float turretClear = 1600.0f;	// after a pass on an aircraft with turrets the fighter flies out this far before turning back
	constexpr float planeEnemyRange = 14000.0f;	// how far away an enemy is noticed, from the air and from the surface
	constexpr float surfaceEnemyRange = 7000.0f;

	// the throttle setting with reverse (or a helicopter's sink) as a negative number
	inline float signedThrottle( const playerState_t& ps, int vehicle )
	{
		const int maxThrottle = availableVehicles[vehicle].maxthrottle;
		return ps.fixed_throttle > maxThrottle ? -static_cast<float>( ps.fixed_throttle - maxThrottle ) : static_cast<float>( ps.fixed_throttle );
	}

	// a player in a vehicle or on foot, in the game and not dead or choosing
	inline bool alive( GameEntity* ent )
	{
		return ent && ent->inuse_ && ent->client_ && ent->health_ > 0 && ent->client_->vehicle_ >= 0 &&
			ent->client_->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED &&
			ent->client_->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR &&
			!(ent->client_->ps_.pm_flags & PMF_VEHICLESELECT);
	}
}

using namespace bots;

#endif
