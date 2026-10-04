#ifndef __G_HELO_H__
#define __G_HELO_H__

#include "g_vehicle.h"





struct Entity_Helicopter : public VehicleBase
{
	struct Entity_Helicopter_Touch : public EntityFunc_Touch
	{
		void	execute( GameEntity* other, trace_t *trace ) override;
	};

	struct Entity_Helicopter_Pain : public EntityFunc_Pain
	{
		void	execute( GameEntity* attacker, int damage ) override;
	};

	void				checkForTakeoffOrLanding();
};


#endif

