
#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_vehicleinfo.h"


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
//	std::string modelname;
//	int num = 0, i, j, k;
//	md3Tag_t tags[MAX_MOUNTS_PER_VEHICLE];

//	if( idx < 0 ) 
//		return false;

	std::string modelname = getModelPath( true );

//#ifdef _DEBUG
//	Com_Printf( "Calculating weapon stations for %s\n", availableVehicles[idx].tinyName );
//#endif

	//num = MF_getTagsContaining(modelname, "tag_P", tags, MAX_MOUNTS_PER_VEHICLE );
	//if( !num ) return false;

	//for( i = 0; i < num; ++i ) {
	//	memcpy( &loadout->mounts[i].tag, &tags[i], sizeof(md3Tag_t) );
	//}

//#ifdef _DEBUG
//	Com_Printf( "%s has %d mounts\n", modelname, num );
//#endif

	//loadout->usedMounts = num;

	//// set up
	//for( i = 0; i < num; ++i ) {
	//	if( strlen( tags[i].name ) < 12 ) return false;
	//	loadout->mounts[i].pos = ahextoi( va("0x%c", tags[i].name[5]) );
	//	loadout->mounts[i].group = ahextoi( va("0x%c", tags[i].name[6]) );
	//	loadout->mounts[i].flags = ahextoi( va("0x%c%c%c%c", tags[i].name[7], tags[i].name[8],
	//			tags[i].name[9], tags[i].name[10]) );
	//	loadout->mounts[i].left = tags[i].name[11] == 'L' ? true : false;
	//}

	//// sort
	//qsort(&loadout->mounts[0], num, sizeof(loadout->mounts[0]), MF_posCompare );

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


					
