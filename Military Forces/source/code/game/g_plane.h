#ifndef __G_PLANE_H__
#define __G_PLANE_H__

#include "g_vehicle.h"





struct Entity_Plane : public VehicleBase
{
	struct Entity_Plane_Touch : public EntityFunc_Touch
	{
		virtual void	execute( GameEntity* other, trace_t *trace );
	};

	struct Entity_Plane_Pain : public EntityFunc_Pain
	{
		virtual void	execute( GameEntity* attacker, int damage );
	};

	void				checkForTakeoffOrLanding();
};


#endif

