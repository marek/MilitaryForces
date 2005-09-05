#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_weaponinfo.h"






WeaponInfo::WeaponInfo() :
	type_(WT_NONE),					
	gameSet_(0),				
	descriptiveName_(""),
	modelName_(""),	
	modelHandle_(0),			
	vwepName_(""),		
	vwepHandle_(0),			
	iconName_(""),			
	iconHandle_(0),			
	targetCategory_(0),			
	nonCatMod_(0),			
	muzzleVelocity_(0),
	range_(0),		
	fuelRange_(0),
	fireInterval_(0),
	damage_(0),		
	damageRadius_(0),
	spread_(0),		
	barrels_(0),
	barrelDistance_(0),
	lockCone_(0),
	followCone_(0),
	lockDelay_(0),
	crosshair_(0),
	crosshairTrack_(0),
	crosshairLock_(0),		
	numberPerPackage_(0),		
	fitsCategory_(0),	
	fitsPylon_(0),		
	basicECMVulnerability_(0),
	flags_(0)
{
}

WeaponInfo::~WeaponInfo()
{
}

