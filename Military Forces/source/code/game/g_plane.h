#ifndef __G_PLANE_H__
#define __G_PLANE_H__

#include "g_vehicle.h"





struct Entity_Plane : public VehicleBase
{
	struct Entity_Plane_Touch : public EntityFunc_Touch
	{
		void	execute( GameEntity* other, trace_t *trace ) override;
	};

	struct Entity_Plane_Pain : public EntityFunc_Pain
	{
		void	execute( GameEntity* attacker, int damage ) override;
	};

	void				checkForTakeoffOrLanding();
};


#endif

