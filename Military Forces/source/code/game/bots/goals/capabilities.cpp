#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "capabilities.h"

Capabilities Capabilities::of( int vehicle )
{
	Capabilities can;
	if( vehicle < 0 )
	{
		can.onFoot = true;
		return can;
	}
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	can.runway = (veh.cat & CAT_PLANE) != 0;
	can.hovers = (veh.cat & CAT_HELO) != 0;
	can.groundStrike = can.runway && (veh.cls & CLASS_PLANE_BOMBER);
	can.gunship = can.hovers && !(veh.cls & CLASS_HELO_TRANSPORT);
	can.armoured = (veh.cat & CAT_GROUND) != 0;
	can.antiAirVehicle = can.armoured && (veh.cls & CLASS_GROUND_SAM);
	can.onWater = (veh.cat & CAT_BOAT) != 0;
	can.onFoot = !(veh.cat & (CAT_PLANE|CAT_HELO|CAT_GROUND|CAT_BOAT));
	can.turrets = (veh.flags & VF_GUNSTATIONS) != 0;
	can.supplies = BotWeapons::carriesSupplies( vehicle );
	can.flares = veh.ammo[WP_FLARE] > 0;
	return can;
}
