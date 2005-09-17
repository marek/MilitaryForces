
#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_vehicleinfo.h"

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
	for( size_t i = 0; i < defaultLoadouts_.size(); ++i )
		distributeWeaponsOnMounts(defaultLoadouts_[i]);
}

void
VehicleInfo::distributeWeaponsOnMounts(Loadout& loadout)
{
	std::string modelname = getModelPath( true );

	std::vector<md3Tag_t> tagList;
	if( !getTagsContaining(modelname, "tag_P", tagList) )
		return;

	for( size_t i = 0; i < tagList.size(); ++i )
	{
		VehicleMountInfo newMount(tagList[i]);
		mounts_.push_back(newMount);
	}

//#ifdef _DEBUG
//	Com_Printf( "%s has %d mounts\n", descriptiveName_.c_str(), mounts_.size() );
//#endif

	// set up
	for( size_t i = 0; i < mounts_.size(); ++i )
	{
		if( strlen( mounts_[i].tag_.name ) < 12 )
			return;
		mounts_[i].position_ = ahextoi( va("0x%c", mounts_[i].tag_.name[5]) );
		mounts_[i].group_ = ahextoi( va("0x%c", mounts_[i].tag_.name[6]) );
		mounts_[i].flags_ = ahextoi( va("0x%c%c%c%c", mounts_[i].tag_.name[7], mounts_[i].tag_.name[8],
														mounts_[i].tag_.name[9], mounts_[i].tag_.name[10]) );
		mounts_[i].left_ = (mounts_[i].tag_.name[11] == 'L') ? true : false;
	}

	// sort - this could really be improved 
	VehicleMountInfo* sortMounts = new VehicleMountInfo[mounts_.size()];
	for( size_t i = 0; i < mounts_.size(); ++i )
		sortMounts[i] = mounts_[i];
	qsort(&sortMounts[0], mounts_.size(), sizeof(VehicleMountInfo), VehicleMountInfo::mountCompare);
	mounts_.clear();
	for( size_t i = 0; i < mounts_.size(); ++i )
		mounts_.push_back(sortMounts[i]);
	delete [] sortMounts;


	//// fill
	//for( i = WP_WEAPON1; i < WP_FLARE; ++i ) {
	//	if( availableVehicles[idx].weapons[i] &&
	//		availableWeapons[availableVehicles[idx].weapons[i]].type != WT_MACHINEGUN ) {
	//		j = availableVehicles[idx].ammo[i];
	//		for( k = 0; k < num; ++k ) {
	//			if( !loadout->mounts[k].weapon &&
	//				(loadout->mounts[k].flags & availableWeapons[availableVehicles[idx].weapons[i]].fitsPylon) ) {
	//				loadout->mounts[k].weapon = availableVehicles[idx].weapons[i];
	//				loadout->mounts[k].num = 1;
	//				j--;
	//			}
	//			if( !j ) break;
	//		}
	//	}
	//}
//#ifdef _DEBUG
//	for( i = 0; i < num; ++i ) {
//		Com_Printf( "Mount %d: %d %x %c %d x %s\n", i, loadout->mounts[i].pos, loadout->mounts[i].flags,
//			(loadout->mounts[i].left ? 'L' : 'R'), loadout->mounts[i].num, 
//			availableWeapons[loadout->mounts[i].weapon].descriptiveName );
//	}
//#endif

	//return true;
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
			if( tagName.find_first_of(str) == 0 )
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


					
