#ifndef __BOT_LOCAL_H__
#define __BOT_LOCAL_H__

void SetTeam( GameEntity *ent, char *s );
void Cmd_Radar_f( GameEntity *ent );
void Cmd_Eject_f( GameEntity *ent );
void Cmd_Kill_f( GameEntity *ent );
void ClientUserinfoChanged( int clientNum );
void MF_ClientBegin( int clientNum );

namespace bots
{
	constexpr signed char moveFull = 127;
	constexpr signed char moveNone = 0;
	constexpr float planeCruiseHeight = 1400.0f;
	constexpr float planeMinHeight = 450.0f;
	// the least height over the ground while lining up to land
	constexpr float approachFloor = 150.0f;
	constexpr float heloCruiseHeight = 500.0f;
	constexpr float heloLowHeight = 180.0f;
	constexpr float bomberMinHeight = 2200.0f;
	constexpr float planeEnemyRange = 14000.0f;
	constexpr float surfaceEnemyRange = 7000.0f;

	inline float signedThrottle( const playerState_t& ps, int vehicle )
	{
		const int maxThrottle = availableVehicles[vehicle].maxthrottle;
		return ps.fixed_throttle > maxThrottle ? -static_cast<float>( ps.fixed_throttle - maxThrottle ) : static_cast<float>( ps.fixed_throttle );
	}

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
