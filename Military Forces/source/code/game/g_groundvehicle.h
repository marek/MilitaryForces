#ifndef __G_GROUNDVEHICLE_H__
#define __G_GROUNDVEHICLE_H__

#include "g_vehicle.h"





struct Entity_GroundVehicle : public VehicleBase
{
	struct Entity_GroundVehicle_Pain : public EntityFunc_Pain
	{
		virtual void	execute( GameEntity* attacker, int damage );
	};
};


#endif

