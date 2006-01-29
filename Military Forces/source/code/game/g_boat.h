#ifndef __G_BOAT_H__
#define __G_BOAT_H__

#include "g_vehicle.h"





struct Entity_Boat : public VehicleBase
{
	struct Entity_Boat_Pain : public EntityFunc_Pain
	{
		virtual void	execute( GameEntity* attacker, int damage );
	};
};



#endif

