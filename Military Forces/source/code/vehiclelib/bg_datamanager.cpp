#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_datamanager.h"
#include "bg_vehicleinfo.h"
#include "bg_weaponinfo.h"



DataManager&	
DataManager::getInstance()
{
	static DataManager theManager;
	return theManager;
}

DataManager::DataManager()
{
}

DataManager::~DataManager()
{
	// empty the vehicle list, dont forget to delete the vehicles
	for( size_t i = 0; i < allVehicles_.size(); ++i )
	{
		delete allVehicles_[i];
		allVehicles_[i] = 0;
	}
	allVehicles_.clear();

	// empty the weapon list, dont forget to delete the weapon
	for( size_t i = 0; i < allWeapons_.size(); ++i )
	{
		delete allWeapons_[i];
		allWeapons_[i] = 0;
	}
	allWeapons_.clear();

}

VehicleInfo*
DataManager::createVehicle()
{
	VehicleInfo* newVehicle = new VehicleInfo();

	if( !newVehicle )
		Com_Error(ERR_FATAL, "Unable to allocate memory for VehicleInfo!");
		return 0;

	return newVehicle;
}

void
DataManager::createAllVehicles()
{
	createAllPlanes();
	createAllHelos();
	createAllGroundVehicles();
	createAllBoats();
	createAllInfantry();

	verifyAllLoadouts();
}

WeaponInfo*
DataManager::createWeapon()
{
	WeaponInfo* newWeapon = new WeaponInfo();

	if( !newWeapon )
		Com_Error(ERR_FATAL, "Unable to allocate memory for WeaponInfo!");
		return 0;
	return newWeapon;
}


void
DataManager::createAllWeapons()
{
	createAllGuns();
	createAllMissiles();
	createAllBombs();
	createAllOther();
}

int
DataManager::findWeaponByName( std::string const& lookupName )
{
	for( size_t i = 0; i < allWeapons_.size(); ++i )
	{
		if( allWeapons_[i]->descriptiveName_ == lookupName )
			return static_cast<int>(i);
	}
	return -1;
}

bool
DataManager::addWeaponToLoadout( Loadout& loadout, 
								 std::string const& lookupName,
								 std::string const& displayName,
								 int maxAmmo,
								 unsigned int selectionType,
								 int turret,
								 bool limitedAngles,
								 vec3_t minAngles,
								 vec3_t maxAngles )
{
	int idx = findWeaponByName(lookupName);
	if( idx < 0 )
	{
		Com_Printf("Unable to add weapon with name '%s' to loadout, didnt find it.", lookupName.c_str());
		return false;
	}
	Armament arm;
	arm.displayName_ = displayName;
	arm.maxAmmo_ = maxAmmo;
	arm.weaponIndex_ = idx;
	arm.selectionType_ = selectionType;
	arm.limitedAngles_ = limitedAngles;
	VectorCopy( minAngles, arm.minAngles_ );
	VectorCopy( maxAngles, arm.maxAngles_ );

	loadout.push_back(arm);

	return true;
}

void
DataManager::verifyAllLoadouts()
{
	for( size_t i = 0; i < allVehicles_.size(); ++i )
		allVehicles_[i]->verifyLoadouts();
}

void
DataManager::createAllData()
{
	createAllWeapons();
	createAllVehicles();
}

