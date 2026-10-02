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

void SP_misc_vehicle( GameEntity *sp_ent );
void SP_misc_groundinstallation( GameEntity *sp_ent );
void DroneInit();
void G_LoadMissionScripts();

#endif // __G_MISCVEHICLE_H__
