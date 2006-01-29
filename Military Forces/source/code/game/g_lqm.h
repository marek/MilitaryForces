#ifndef __G_LQM_H__
#define __G_LQM_H__

#include "g_vehicle.h"





struct Entity_Infantry : public VehicleBase
{
	struct Entity_Infantry_Touch : public EntityFunc_Touch
	{
		virtual void	execute( GameEntity* other, trace_t *trace );
	};

	struct Entity_Infantry_Pain : public EntityFunc_Pain
	{
		virtual void	execute( GameEntity* attacker, int damage );
	};
};


#endif

