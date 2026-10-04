#ifndef __G_VEHICLE_H__
#define __G_VEHICLE_H__

#include "g_entity.h"


struct VehicleBase : public GameEntity
{
	struct VehicleDeath : public GameEntity::EntityFunc_Die
	{
		void	execute( GameEntity* inflictor, GameEntity* attacker, int damage, int mod ) override;
	};

	struct VehicleDie : public GameEntity::EntityFunc_Die
	{
		void	execute( GameEntity* inflictor, GameEntity* attacker, int damage, int mod ) override;
	};
};



#endif // __G_VEHICLE_H__
