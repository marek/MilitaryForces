#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_datamanager.h"
#include "bg_weaponinfo.h"




void						
DataManager::createAllOther()
{
	WeaponInfo* wep = 0;

	// FFAR (19)
	wep = createWeapon();
	wep->type_ = WT_ROCKET;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "FFAR (19)";		
	wep->modelName_ = "models/weapons/vwep_rocket.md3";				
	wep->vwepName_ = "models/weapons/vwep_ffar.md3";				
	wep->iconName_ = "hud/iconw_rkt.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 1500;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 5000;				
	wep->fireInterval_ = 180;			
	wep->damage_ = 40;				
	wep->damageRadius_ = 50;			
	wep->spread_ = 2;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_ROCKETMODE;				
	wep->crosshairTrack_ = CH_ROCKETMODE;		
	wep->crosshairLock_ = CH_ROCKETMODE;			
	wep->numberPerPackage_ = 19;		
	wep->fitsCategory_ = CAT_PLANE|CAT_HELO;			
	wep->fitsPylon_ = PF_FFAR;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = WF_HAS_FIRE_FRAME;
	allWeapons_.push_back(wep);

	// FFAR (7)
	wep = createWeapon();
	wep->type_ = WT_ROCKET;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "FFAR (7)";		
	wep->modelName_ = "models/weapons/vwep_rocket.md3";				
	wep->vwepName_ = "models/weapons/vwep_ffar_lt.md3";				
	wep->iconName_ = "hud/iconw_rkt.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 1500;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 5000;				
	wep->fireInterval_ = 180;			
	wep->damage_ = 40;				
	wep->damageRadius_ = 50;			
	wep->spread_ = 2;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_ROCKETMODE;				
	wep->crosshairTrack_ = CH_ROCKETMODE;		
	wep->crosshairLock_ = CH_ROCKETMODE;			
	wep->numberPerPackage_ = 7;		
	wep->fitsCategory_ = CAT_PLANE|CAT_HELO;			
	wep->fitsPylon_ = PF_FFAR;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = WF_HAS_FIRE_FRAME;
	allWeapons_.push_back(wep);

	// FFAR+ (6)
	wep = createWeapon();
	wep->type_ = WT_ROCKET;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "FFAR+ (6)";		
	wep->modelName_ = "models/weapons/vwep_rocket_hv.md3";				
	wep->vwepName_ = "models/weapons/vwep_ffar_hv.md3";				
	wep->iconName_ = "hud/iconw_rkt.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 1500;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 5000;				
	wep->fireInterval_ = 200;			
	wep->damage_ = 80;				
	wep->damageRadius_ = 80;			
	wep->spread_ = 2;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_ROCKETMODE;				
	wep->crosshairTrack_ = CH_ROCKETMODE;		
	wep->crosshairLock_ = CH_ROCKETMODE;			
	wep->numberPerPackage_ = 6;		
	wep->fitsCategory_ = CAT_PLANE|CAT_HELO;			
	wep->fitsPylon_ = PF_FFAR;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = WF_HAS_FIRE_FRAME;
	allWeapons_.push_back(wep);


	// flares
	wep = createWeapon();
	wep->type_ = WT_FLARE;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "Flares";		
	wep->modelName_ = "models/effects/flare.md3";				
	wep->vwepName_ = "";				
	wep->iconName_ = "";				
	wep->targetCategory_ = 0;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 0;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 2000;				
	wep->fireInterval_ = 200;			
	wep->damage_ = 0;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 0;				
	wep->barrels_ = 0;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = 0;				
	wep->crosshairTrack_ = 0;		
	wep->crosshairLock_ = 0;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_FLAREPOD;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);
}

	

