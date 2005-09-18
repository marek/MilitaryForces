#ifndef __BG_DATAMANAGER_H__
#define __BG_DATAMANAGER_H__


#include <string>
#include <vector>
#include "../game/q_shared.h"
#include "bg_vehicleextra.h"

struct VehicleInfo;
struct WeaponInfo;

typedef std::vector<VehicleInfo*>	VehicleList;
typedef std::vector<WeaponInfo*>	WeaponList;


struct DataManager
{
	static DataManager&			getInstance();

	void						createAllData();

protected:
	// vehicles/weapon creation util functions
	void						createAllVehicles();
	VehicleInfo*				createVehicle();

	void						createAllPlanes();
	void						createAllHelos();
	void						createAllGroundVehicles();
	void						createAllBoats();
	void						createAllInfantry();

	void						createAllWeapons();
	WeaponInfo*					createWeapon();

	void						createAllGuns();
	void						createAllMissiles();
	void						createAllBombs();
	void						createAllOther();

	// loadout helpers
	int							findWeaponByName( std::string const& lookupName );
	bool						addWeaponToLoadout( Loadout& loadout, 
												 	std::string const& lookupName,
													std::string const& displayName,
													int maxAmmo,
													unsigned int selectionType,
													int turret = -1,
													bool limitedAngles = false,
													vec3_t minAngles = 0,
													vec3_t maxAngles = 0 );
	void						verifyAllLoadouts();

private:
								DataManager();
								~DataManager();

	// the actual list
	VehicleList					allVehicles_;
	WeaponList					allWeapons_;
};




#endif // __BG_DATAMANAGER_H__
