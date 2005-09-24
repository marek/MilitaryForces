#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_vehicleextra.h"





Armament::Armament() :
	displayName_(""),
	weaponIndex_(-1),
	maxAmmo_(-1),
	selectionType_(ST_NOT_SELECTABLE),
	turret_(-1),	
	limitedAngles_(false)
{
	VectorSet(minAngles_,0,0,0);
	VectorSet(maxAngles_,0,0,0);
}

Armament::~Armament()
{
}

VehicleMountInfo::VehicleMountInfo() :
	position_(-1),
	group_(-1),
	flags_(-1),
	left_(false)
{
	tag_.name[0] = 0;
}

VehicleMountInfoEvaluator::VehicleMountInfoEvaluator() :
	VehicleMountInfo(),
	weaponIndex_(-1),
	num_(-1)
{
}

VehicleMountInfoEvaluator::VehicleMountInfoEvaluator(VehicleMountInfo const& vmi) :
	VehicleMountInfo(vmi),
	weaponIndex_(-1),
	num_(-1)
{

}

bool
VehicleMountInfo::mountCompare( VehicleMountInfo const& a, VehicleMountInfo const& b )
{
	if( a.position_ == b.position_ )
	{
		if( a.left_ )
			return true;
		return false;
	}
	else
		return ((b.position_ - a.position_) >= 0);
}


