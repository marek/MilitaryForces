#include "vehicle_table.h"

completeVehicleData_t availableVehicles[MAX_VEHICLE_TYPES];
int bg_numberOfVehicles = 0;

// puts every family's entries into the table at their own index; run once, before anything reads the table
void VehicleData::registerVehicles()
{
	// families in a fixed order
	static constexpr VehicleFamily (*families[])() = {
		&VehicleFamilies::f_16,
		&VehicleFamilies::jas_39,
		&VehicleFamilies::a10,
		&VehicleFamilies::f_5,
		&VehicleFamilies::f_15,
		&VehicleFamilies::f_14,
		&VehicleFamilies::f_18,
		&VehicleFamilies::b_2,
		&VehicleFamilies::p_51d,
		&VehicleFamilies::spitfire_mk5b,
		&VehicleFamilies::bf_109g,
		&VehicleFamilies::fw190a8,
		&VehicleFamilies::b17g,
		&VehicleFamilies::dr1,
		&VehicleFamilies::camel,
		&VehicleFamilies::m1,
		&VehicleFamilies::t_90,
		&VehicleFamilies::fav,
		&VehicleFamilies::humvee_50cal,
		&VehicleFamilies::humvee_avng,
		&VehicleFamilies::brdm2,
		&VehicleFamilies::ZSU_23,
		&VehicleFamilies::panzeriii,
		&VehicleFamilies::m4,
		&VehicleFamilies::mlrs,
		&VehicleFamilies::helo,
		&VehicleFamilies::UH_1,
		&VehicleFamilies::comanche,
		&VehicleFamilies::mi_24,
		&VehicleFamilies::mi_35,
		&VehicleFamilies::sarge,
		&VehicleFamilies::major,
		&VehicleFamilies::pbr31mk2
	};
	static bool done = false;
	if( done )
		return;
	done = true;

	bool taken[MAX_VEHICLE_TYPES] = {};
	for( const auto family : families )
	{
		for( const VehicleEntry& entry : family() )
		{
			if( entry.index < 0 || entry.index >= MAX_VEHICLE_TYPES || taken[entry.index] )
				Common::error( ERR_FATAL, "vehicle table: index %d of %s is out of range or used twice", entry.index, entry.data.descriptiveName );
			taken[entry.index] = true;
			availableVehicles[entry.index] = entry.data;
			bg_numberOfVehicles = std::max( bg_numberOfVehicles, entry.index + 1 );
		}
	}
	for( int i = 0; i < bg_numberOfVehicles; i++ )
	{
		if( !taken[i] )
			Common::error( ERR_FATAL, "vehicle table: index %d has no vehicle", i );
	}
}

// the table is filled before main runs, since menus and game code read it at start-up
static const bool registered = ( VehicleData::registerVehicles(), true );
