
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_vehicleinfo.h"
#include "bg_weaponinfo.h"
#include "bg_datamanager.h"
#include "bg_md3utils.h"

#include <algorithm>


// decls



GameObjectInfo_Vehicle::GameObjectInfo_Vehicle() :
	airRadar_(nullptr),
	groundRadar_(nullptr),
	minThrottle_(0),
	maxThrottle_(0),
	acceleration_(0),
	maxFuel_(0)
{
	VectorSet( gunoffset_, 0, 0, 0 );
	VectorSet( cockpitview_, 0, 0, 0 );

}

GameObjectInfo_Vehicle::~GameObjectInfo_Vehicle()
{
	delete airRadar_;
	airRadar_ = nullptr;
	delete groundRadar_;
	groundRadar_ = nullptr;
}

bool
GameObjectInfo_Vehicle::setupGameObject()
{
	if( !GameObjectInfo::setupGameObject() )
		return false;

	if( !verifyLoadouts() )
		return false;

	return true;
}

bool
GameObjectInfo_Vehicle::setupBoundingBox()
{
	return Md3Utils::getModelDimensions( getModelPath(true), mins_, maxs_ );
}

bool
GameObjectInfo_Vehicle::verifyLoadouts()
{
	// create the weapon mounts (empty)
	// if there are none, then this isnt a vehicle with mounts, so nothing further to check
	if( !createWeaponMounts() )
		return false;

	// we got mounts, that means we can show the vwep
	renderFlags_ |= MFR_VWEP;

	// go through all reported loadouts and verify that they actually fit on the vehicle
	for(auto & defaultLoadout : defaultLoadouts_)
		correctArmament(defaultLoadout.second, defaultLoadout.first);

	return true;
}

// creates the mounts (without actually putting weapons on, thats done somewhere else)
bool
GameObjectInfo_Vehicle::createWeaponMounts()
{
	std::string modelname = getModelPath( true );

	std::vector<md3Tag_t> tagList;
	if( !Md3Utils::getTagsContaining(modelname, "PY", tagList) )
		return false;

	mounts_.resize(tagList.size());
	for( size_t i = 0; i < tagList.size(); ++i )
		mounts_[i].tag_ = tagList[i];

//#ifdef _DEBUG
//	Com_Printf( "%s has %d mounts\n", descriptiveName_.c_str(), tagList.size() );
//#endif

	// set up
	for( size_t i = 0; i < tagList.size(); ++i )
	{
		// check if format is ok
		if( strlen( tagList[i].name ) < 10 )
			return false;
		mounts_[i].position_ = ahextoi( va("0x%c", tagList[i].name[2]) );
		mounts_[i].group_ = ahextoi( va("0x%c", tagList[i].name[3]) );
		mounts_[i].flags_ = ahextoi( va("0x%c%c%c%c%c%c", tagList[i].name[4], tagList[i].name[5],
														 tagList[i].name[6], tagList[i].name[7],
														 tagList[i].name[8], tagList[i].name[9]) );
		mounts_[i].left_ = (tagList[i].name[13] == 'L') ? true : false;
	}

	// sort them
	std::sort( mounts_.begin(), mounts_.end(), VehicleMountInfo::mountCompare );

	return true;
}

void
GameObjectInfo_Vehicle::correctArmament(Loadout& loadout, std::string const& loadoutName)
{
	// fallback check
	if( !mounts_.size() )
		return;

	// the weapon list
	WeaponList const& allWeapons = DataManager::getInstance().getAllWeapons();

	// create a work copy of the loadout
	Loadout workCopy;
	for(auto & i : loadout)
	{
		// dont add weapons not fitting this category of vehicle
		if( category_ & allWeapons[i.weaponIndex_]->fitsCategory_ )
			workCopy.push_back(i);
	}
	loadout.clear();

	// create a copy of the mount list, with evaluation class
	std::vector<VehicleMountInfoEvaluator> mountList;
	for(const auto & mount : mounts_)
		mountList.push_back(mount);
	// go through all weapons and fit them on mounts
	for(auto & i : workCopy)
	{
		// weapons which cant be fitted on mounts arent evaluated
		if( allWeapons[i.weaponIndex_]->fitsPylon_ == WeaponInfo::PF_NOT_AP )
			continue;
		// go through all mounts
		for(auto & j : mountList)
		{
			// if mount is emtpy and weapon fits, then "put it on"
			if( j.weaponIndex_ < 0 &&
				(j.flags_ & allWeapons[i.weaponIndex_]->fitsPylon_) )
			{
				j.weaponIndex_ = i.weaponIndex_;
				j.num_ = 1;
				i.maxAmmo_--;
			}
			if( !i.maxAmmo_ )
				break;
		}
	}
	// put loadout back together from workCopy
	for(const auto & i : workCopy)
		loadout.push_back(i);

	// remove extraneous weapons from default loadout
	int numProblems = 0;
	for( size_t i = 0; i < workCopy.size(); ++i )
	{
		// weapons which cant be fitted on mounts arent evaluated
		if( allWeapons[workCopy[i].weaponIndex_]->fitsPylon_ == WeaponInfo::PF_NOT_AP )
			continue;
		if( workCopy[i].maxAmmo_ > 0 )
		{
			Com_Printf("WARNING: %s loadout '%s' can't fit all the weapons, loadout modified: %d %ss removed\n",
				descriptiveName_.c_str(), loadoutName.c_str(), workCopy[i].maxAmmo_, 
				allWeapons[workCopy[i].weaponIndex_]->descriptiveName_.c_str() );
			loadout[i].maxAmmo_ -= workCopy[i].maxAmmo_;
			numProblems++;
		}
	}
	if( !numProblems )
		Com_Printf("INFO: %s loadout '%s' verified OK.\n", descriptiveName_.c_str(), loadoutName.c_str() );
}

