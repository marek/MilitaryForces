#ifndef __VEHICLE_TABLE_H__
#define __VEHICLE_TABLE_H__

#include "../q_shared.h"
#include "../../qcommon/qfiles.h"
#include "../bg_public.h"
#include <span>

// One line of the vehicle table: where it sits and what it is. The index is what travels over the network and
// is saved in missions, so an entry keeps its index for good; new entries take the next free one.
struct VehicleEntry
{
	int						index;
	completeVehicleData_t	data;
};

using VehicleFamily = std::span<const VehicleEntry>;

// a vehicle family is one model with its loadout variants, in a file of its own under vehicles/<category>/
namespace VehicleFamilies
{
	VehicleFamily f_16();
	VehicleFamily jas_39();
	VehicleFamily a10();
	VehicleFamily f_5();
	VehicleFamily f_15();
	VehicleFamily f_14();
	VehicleFamily f_18();
	VehicleFamily b_2();
	VehicleFamily p_51d();
	VehicleFamily spitfire_mk5b();
	VehicleFamily bf_109g();
	VehicleFamily fw190a8();
	VehicleFamily b17g();
	VehicleFamily dr1();
	VehicleFamily camel();
	VehicleFamily m1();
	VehicleFamily t_90();
	VehicleFamily fav();
	VehicleFamily humvee_50cal();
	VehicleFamily humvee_avng();
	VehicleFamily brdm2();
	VehicleFamily ZSU_23();
	VehicleFamily panzeriii();
	VehicleFamily m4();
	VehicleFamily mlrs();
	VehicleFamily helo();
	VehicleFamily UH_1();
	VehicleFamily comanche();
	VehicleFamily mi_24();
	VehicleFamily mi_35();
	VehicleFamily sarge();
	VehicleFamily major();
	VehicleFamily pbr31mk2();
}

#endif
