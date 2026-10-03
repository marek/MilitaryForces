#ifndef __G_MISCVEHICLE_H__
#define __G_MISCVEHICLE_H__

#include "g_entity.h"

struct MiscVehicle_Die : public GameEntity::EntityFunc_Die
{
	virtual void	execute( GameEntity* inflictor, GameEntity* attacker, int damage, int meansOfDeath );
};

struct GroundInstallation_Die : public GameEntity::EntityFunc_Die
{
	virtual void	execute( GameEntity* inflictor, GameEntity* attacker, int damage, int meansOfDeath );
};

struct GroundInstallation_Think : public GameEntity::EntityFunc_Think
{
	virtual void	execute();
};

struct Drone_Think : public GameEntity::EntityFunc_Think
{
	virtual void	execute();

	vec3_t			route_[IGME_MAX_WAYPOINTS];
	int				count_;
	int				next_;
	int				blocked_;
	float			speed_;
};

struct Abandoned_Think : public GameEntity::EntityFunc_Think
{
	Abandoned_Think() : expire_(0), enterable_(false), fuel_(0), ownerTeam_(0)
	{
		memset( ammo_, 0, sizeof(ammo_) );
		memset( mounts_, 0, sizeof(mounts_) );
	}

	virtual void	execute();

	int				expire_;
	bool			enterable_;
	int				fuel_;
	int				ownerTeam_;
	int				ammo_[MAX_WEAPONS_PER_VEHICLE*2];
	int				mounts_[32];
};

GameEntity* SP_misc_vehicle( GameEntity *sp_ent );
void MiscVehicle_Abandon( GameEntity *ent );
void MiscVehicle_SetRoute( GameEntity *ent, const mission_waypoint_t *wps );
void SP_misc_groundinstallation( GameEntity *sp_ent );
void DroneInit();
void G_LoadMissionScripts();

#endif // __G_MISCVEHICLE_H__
