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

