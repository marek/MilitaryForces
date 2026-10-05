#ifndef __BG_DATAMANAGER_H__
#define __BG_DATAMANAGER_H__


#include <string>
#include <string_view>
#include <memory>
#include <vector>
#include "../game/q_shared.h"
#include "bg_vehicleextra.h"

struct GameObjectInfo;
struct WeaponInfo;

typedef std::vector<std::unique_ptr<GameObjectInfo>>	GameObjectList;
typedef std::vector<std::unique_ptr<WeaponInfo>>		WeaponList;


struct DataManager
{
	static DataManager&			getInstance();

	void						createAllData();

	WeaponList const&			getAllWeapons() const;
	GameObjectList const&		getAllGameObjects() const;

	// loadout helpers
	int							findWeaponByName( std::string_view lookupName );

protected:
	// vehicles/weapon creation util functions
	void						createAllVehicles();

	void						createAllWeapons();

	void						setupAllVehicles();

private:
								DataManager();
								~DataManager();

	// the actual list
	GameObjectList				allGameObjects_;
	WeaponList					allWeapons_;
};




#endif // __BG_DATAMANAGER_H__
