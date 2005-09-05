
#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_vehicleinfo.h"


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

}

					
