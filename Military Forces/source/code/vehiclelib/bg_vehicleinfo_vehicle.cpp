
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_vehicleinfo.h"
#include "bg_weaponinfo.h"
#include "bg_datamanager.h"

#include <algorithm>


// decls
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );



GameObjectInfo_Vehicle::GameObjectInfo_Vehicle() :
	airRadar_(0),
	groundRadar_(0),
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
	airRadar_ = 0;
	delete groundRadar_;
	groundRadar_ = 0;
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
	fileHandle_t		file;
	std::string const&	fileName = getModelPath(true);
	bool				found = false;

	// get body bounding boxes
	if( trap_FS_FOpenFile(fileName.c_str(), &file, FS_READ) >= 0 ) 
	{
		found = true;
		md3Header_t header;
		md3Frame_t currentFrame;

		trap_FS_Read(&header, sizeof(header), file);
		int number = header.numFrames;
		for( int i = 0; i < number; ++i ) 
		{
			trap_FS_Read(&currentFrame, sizeof(currentFrame), file);
			if( i == 0 )
			{
				VectorCopy( currentFrame.bounds[0], mins_ );
				VectorCopy( currentFrame.bounds[1], maxs_ );
			}
			else
			{
				if( currentFrame.bounds[0][0] < mins_[0] )
					mins_[0] = currentFrame.bounds[0][0];
				if( currentFrame.bounds[0][1] < mins_[1] )
					mins_[1] = currentFrame.bounds[0][1];
				if( currentFrame.bounds[0][2] < mins_[2] )
					mins_[2] = currentFrame.bounds[0][2];

				if( currentFrame.bounds[1][0] > maxs_[0] )
					maxs_[0] = currentFrame.bounds[1][0];
				if( currentFrame.bounds[1][1] > maxs_[1] )
					maxs_[1] = currentFrame.bounds[1][1];
				if( currentFrame.bounds[1][2] > maxs_[2] )
					maxs_[2] = currentFrame.bounds[1][2];

			}
		}
		trap_FS_FCloseFile(file);
	} 
	else 
	{
		Com_Error(ERR_FATAL, "Unable to open file %s\n", fileName.c_str() );
	}
	return found;

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
	for( LoadoutMapIter it = defaultLoadouts_.begin(); it != defaultLoadouts_.end(); ++it )
		correctArmament((*it).second, (*it).first);

	return true;
}

// creates the mounts (without actually putting weapons on, thats done somewhere else)
bool
GameObjectInfo_Vehicle::createWeaponMounts()
{
	std::string modelname = getModelPath( true );

	std::vector<md3Tag_t> tagList;
	if( !getTagsContaining(modelname, "PY", tagList) )
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
	for( size_t i = 0; i < loadout.size(); ++i  )
	{
		// dont add weapons not fitting this category of vehicle
		if( category_ & allWeapons[loadout[i].weaponIndex_]->fitsCategory_ )
			workCopy.push_back(loadout[i]);
	}
	loadout.clear();

	// create a copy of the mount list, with evaluation class
	std::vector<VehicleMountInfoEvaluator> mountList;
	for( size_t i = 0; i < mounts_.size(); ++i )
		mountList.push_back(mounts_[i]);
	// go through all weapons and fit them on mounts
	for( size_t i = 0; i < workCopy.size(); ++i )
	{
		// weapons which cant be fitted on mounts arent evaluated
		if( allWeapons[workCopy[i].weaponIndex_]->fitsPylon_ == WeaponInfo::PF_NOT_AP )
			continue;
		// go through all mounts
		for( size_t j = 0; j < mountList.size(); ++j )
		{
			// if mount is emtpy and weapon fits, then "put it on"
			if( mountList[j].weaponIndex_ < 0 &&
				(mountList[j].flags_ & allWeapons[workCopy[i].weaponIndex_]->fitsPylon_) )
			{
				mountList[j].weaponIndex_ = workCopy[i].weaponIndex_;
				mountList[j].num_ = 1;
				workCopy[i].maxAmmo_--;
			}
			if( !workCopy[i].maxAmmo_ )
				break;
		}
	}
	// put loadout back together from workCopy
	for( size_t i = 0; i < workCopy.size(); ++i )
		loadout.push_back(workCopy[i]);

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

