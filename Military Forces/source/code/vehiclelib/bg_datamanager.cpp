#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
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
= default;

DataManager::~DataManager()
{

}

void
DataManager::createAllVehicles()
{
	GameObjectInfo_Plane::createAllPlanes(allGameObjects_);
	GameObjectInfo_Helicopter::createAllHelicopters(allGameObjects_);
	GameObjectInfo_GroundVehicle::createAllGroundVehicles(allGameObjects_);
	GameObjectInfo_Boat::createAllBoats(allGameObjects_);
	GameObjectInfo_Infantry::createAllInfantry(allGameObjects_);

	setupAllVehicles();
}

void
DataManager::createAllWeapons()
{
	WeaponInfo::createAllGuns(allWeapons_);
	WeaponInfo::createAllMissiles(allWeapons_);
	WeaponInfo::createAllBombs(allWeapons_);
	WeaponInfo::createAllOther(allWeapons_);
}

int
DataManager::findWeaponByName( std::string_view lookupName )
{
	for( size_t i = 0; i < allWeapons_.size(); ++i )
	{
		if( allWeapons_[i]->descriptiveName_ == lookupName )
			return static_cast<int>(i);
	}
	return -1;
}
/*
Loadout const &
DataManager::findLoadoutByVehicleNum(const int vehicleNum)
{
	return allGameObjects_[(VehicleNum & 0x000FFC00) >> 10]->;
}

VehicleInfo const &
DataManager::findVehicleByVehicleNum(const int vehicleNum)
{
	return (VehicleNum & 0xFFF00000) >> 20;
}

WeaponInfo const &
DataManager::findWeaponByVehicleNum(const int vehicleNum)
{
	return VehicleNum & 0x000003FF;
}*/

void
DataManager::setupAllVehicles()
{
	for(auto & allGameObject : allGameObjects_)
		allGameObject->setupGameObject();
}

void
DataManager::createAllData()
{
	createAllWeapons();
	createAllVehicles();
}

WeaponList const&
DataManager::getAllWeapons() const
{
	return allWeapons_;
}

GameObjectList const&
DataManager::getAllGameObjects() const
{
	return allGameObjects_;
}


