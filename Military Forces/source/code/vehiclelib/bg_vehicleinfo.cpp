
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_vehicleinfo.h"
#include "bg_weaponinfo.h"
#include "bg_datamanager.h"

// decls
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );



const std::string VehicleInfo::baseModelPath = "models/vehicles/";

VehicleInfo::VehicleInfo() :
	descriptiveName_(""),
	tinyName_(""),
	modelName_(""),
	gameSet_(0),
	category_(0),
	class_(0),
	flags_(0),
	caps_(0),
	renderFlags_(0),
	maxHealth_(0),
	shadowShader_(SHADOW_NONE),
	airRadar_(0),
	groundRadar_(0),
	minThrottle_(0),
	maxThrottle_(0),
	acceleration_(0),
	maxFuel_(0),
	wheels_(0),
	wheelCF_(0.0f),
	engines_(0),
	bayAnim_(0),
	gearAnim_(0),
	tailAngle_(0.0f),
	gearHeight_(0),
	abEffectModel_(0),
	stallSpeed_(0),
	swingAngle_(0.0f),
	sonarInfo_(0),
	maxSpeed_(0),
	animations_(0)
{
	VectorSet( turnSpeed_, 0, 0, 0 );
	VectorSet( camDist_, 0, 0, 0 );
	VectorSet( camHeight_, 0, 0, 0 );
	Vector4Set( shadowCoords_, 0, 0, 0, 0 );
	Vector4Set( shadowAdjusts_, 0, 0, 0, 0 );
	VectorSet( mins_, 0, 0, 0 );
	VectorSet( maxs_, 0, 0, 0 );
	VectorSet( gunoffset_, 0, 0, 0 );
	VectorSet( cockpitview_, 0, 0, 0 );
}

VehicleInfo::~VehicleInfo()
{
	delete airRadar_;
	airRadar_ = 0;
	delete groundRadar_;
	groundRadar_ = 0;
	delete sonarInfo_;
	sonarInfo_ = 0;
	delete bayAnim_;
	bayAnim_ = 0;
	delete gearAnim_;
	gearAnim_ = 0;
}

void
VehicleInfo::verifyLoadouts()
{
	// create the weapon mounts (empty)
	// if none are there this isnt a vehicle with mounts, so nothing further to check
	if( !createWeaponMounts() )
		return;

	// we got mounts, that means we can show the vwep
	renderFlags_ |= MFR_VWEP;

	// always add an empty loadout for customization
	defaultLoadouts_.insert(std::make_pair("Custom", Loadout()));

	// go through all reported loadouts and verify that they actually fit on the vehicle
	for( LoadoutMapIter it = defaultLoadouts_.begin(); it != defaultLoadouts_.end(); ++it )
		correctArmament((*it).second, (*it).first);
}

// creates the mounts (without actually putting weapons on, thats done somewhere else)
bool
VehicleInfo::createWeaponMounts()
{
	std::string modelname = getModelPath( true );

	std::vector<md3Tag_t> tagList;
	if( !getTagsContaining(modelname, "tag_P", tagList) )
		return false;

	VehicleMountInfo* sortMounts = new VehicleMountInfo[tagList.size()];

	for( size_t i = 0; i < tagList.size(); ++i )
		sortMounts[i].tag_ = tagList[i];

//#ifdef _DEBUG
//	Com_Printf( "%s has %d mounts\n", descriptiveName_.c_str(), tagList.size() );
//#endif

	// set up
	for( size_t i = 0; i < tagList.size(); ++i )
	{
		// check if format is ok
		if( strlen( tagList[i].name ) < 12 )
		{
			delete [] sortMounts;
			return false;
		}
		sortMounts[i].position_ = ahextoi( va("0x%c", tagList[i].name[5]) );
		sortMounts[i].group_ = ahextoi( va("0x%c", tagList[i].name[6]) );
		sortMounts[i].flags_ = ahextoi( va("0x%c%c%c%c", tagList[i].name[7], tagList[i].name[8],
														tagList[i].name[9], tagList[i].name[10]) );
		sortMounts[i].left_ = (tagList[i].name[11] == 'L') ? true : false;
	}

	// sort  
	qsort(&sortMounts[0], mounts_.size(), sizeof(VehicleMountInfo), VehicleMountInfo::mountCompare);

	// move the mounts into the list
	mounts_.clear();
	for( size_t i = 0; i < tagList.size(); ++i )
		mounts_.push_back(sortMounts[i]);
	delete [] sortMounts;

	return true;
}

void
VehicleInfo::correctArmament(Loadout& loadout, std::string const& loadoutName)
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
		if( allWeapons[workCopy[i].weaponIndex_]->fitsPylon_ == PF_NA )
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
		if( allWeapons[workCopy[i].weaponIndex_]->fitsPylon_ == PF_NA )
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

std::string
VehicleInfo::getModelPath( bool extension )
{
	std::string returnString = baseModelPath;

	switch( category_ )
	{
	case CAT_PLANE:
		returnString += "planes/";
		break;
	case CAT_HELO:
		returnString += "helos/";
		break;
	case CAT_GROUND:
		returnString += "ground/";
		break;
	case CAT_BOAT:
		returnString += "sea/";
		break;
	case CAT_LQM:
		returnString += "lqms/";
		break;
	default:
		Com_Error(ERR_FATAL, "Wrong category in vehicle in 'getModelPath'");
		return "";
	}
	returnString += modelName_ + "/" + modelName_;
	
	if( extension )
		returnString += ".md3";

	return returnString;
}

int
VehicleInfo::getTagsContaining( std::string const& filename, 
								std::string const& str,
								std::vector<md3Tag_t>& tagList )
{
	if( str.empty() || str == "" ) 
		return 0;

	fileHandle_t	f;
	if( trap_FS_FOpenFile(filename.c_str(), &f, FS_READ) >= 0 ) 
	{
		md3Header_t head;
		md3Frame_t	frame;
		md3Tag_t	tag;
		trap_FS_Read(&head, sizeof(head), f);
		for( int i = 0; i < head.numFrames; ++i ) 
			trap_FS_Read(&frame, sizeof(frame), f);
		int total = head.numTags;
		for( int i = 0; i < total; ++ i ) 
		{
			trap_FS_Read(&tag, sizeof(tag), f);
			std::string tagName(tag.name);
			// if it contains the string at the first position, then add it
			if( tagName.find(str) == 0 )
				tagList.push_back(tag);
		}
		trap_FS_FCloseFile(f);
	} 
	else 
	{
		Com_Printf( "Unable to open file %s\n", filename.c_str() );
	}
	return tagList.size();
}


					
