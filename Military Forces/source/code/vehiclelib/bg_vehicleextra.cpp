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

int VehicleMountInfo::mountCompare( const void* a, const void* b )
{
	int p1 = (reinterpret_cast<const VehicleMountInfo*>(a))->position_;
	int p2 = (reinterpret_cast<const VehicleMountInfo*>(b))->position_;
	if( p1 == p2 ) 
	{
		if( (reinterpret_cast<const VehicleMountInfo*>(a))->left_ ) 
			return -1;
		return 1;
	} 
	else 
		return p1-p2;
}



