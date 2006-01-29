#ifndef __G_HELO_H__
#define __G_HELO_H__

#include "g_vehicle.h"





struct Entity_Helicopter : public VehicleBase
{
	struct Entity_Helicopter_Touch : public EntityFunc_Touch
	{
		virtual void	execute( GameEntity* other, trace_t *trace );
	};

	struct Entity_Helicopter_Pain : public EntityFunc_Pain
	{
		virtual void	execute( GameEntity* attacker, int damage );
	};

	void				checkForTakeoffOrLanding();
};


#endif

