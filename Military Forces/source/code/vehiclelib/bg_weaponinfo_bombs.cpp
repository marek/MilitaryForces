#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"
#include "bg_datamanager.h"
#include "bg_weaponinfo.h"




void						
DataManager::createAllBombs()
{
	WeaponInfo* wep = 0;

	// Mk82
	wep = createWeapon();
	wep->type_ = WT_IRONBOMB;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "Mk-82";		
	wep->modelName_ = "models/weapons/vwep_500.md3";				
	wep->vwepName_ = "models/weapons/vwep_500.md3";				
	wep->iconName_ = "hud/iconw_bmb.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 0;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 50;				
	wep->damageRadius_ = 150;			
	wep->spread_ = 0;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_BOMBMODE;				
	wep->crosshairTrack_ = CH_BOMBMODE;		
	wep->crosshairLock_ = CH_BOMBMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_PLANE;			
	wep->fitsPylon_ = PF_AG_LT;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// Mk82 R
	wep = createWeapon();
	wep->type_ = WT_IRONBOMB;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "Mk-82 R";		
	wep->modelName_ = "models/weapons/vwep_500.md3";				
	wep->vwepName_ = "models/weapons/vwep_500.md3";				
	wep->iconName_ = "hud/iconw_bmb.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 0;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 50;				
	wep->damageRadius_ = 150;			
	wep->spread_ = 0;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_BOMBMODE;				
	wep->crosshairTrack_ = CH_BOMBMODE;		
	wep->crosshairLock_ = CH_BOMBMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_PLANE;			
	wep->fitsPylon_ = PF_AG_LT;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = WF_HAS_FIRE_FRAME;
	allWeapons_.push_back(wep);

	// Mk83
	wep = createWeapon();
	wep->type_ = WT_IRONBOMB;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "Mk-83";		
	wep->modelName_ = "models/weapons/vwep_1000.md3";				
	wep->vwepName_ = "models/weapons/vwep_1000.md3";				
	wep->iconName_ = "hud/iconw_bmb.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 0;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 200;			
	wep->damage_ = 120;				
	wep->damageRadius_ = 230;			
	wep->spread_ = 0;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_BOMBMODE;				
	wep->crosshairTrack_ = CH_BOMBMODE;		
	wep->crosshairLock_ = CH_BOMBMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_PLANE;			
	wep->fitsPylon_ = PF_AG_MED;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// Mk83
	wep = createWeapon();
	wep->type_ = WT_IRONBOMB;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "Mk-83 R";		
	wep->modelName_ = "models/weapons/vwep_1000.md3";				
	wep->vwepName_ = "models/weapons/vwep_1000.md3";				
	wep->iconName_ = "hud/iconw_bmb.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 0;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 200;			
	wep->damage_ = 120;				
	wep->damageRadius_ = 230;			
	wep->spread_ = 0;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_BOMBMODE;				
	wep->crosshairTrack_ = CH_BOMBMODE;		
	wep->crosshairLock_ = CH_BOMBMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_PLANE;			
	wep->fitsPylon_ = PF_AG_MED;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = WF_HAS_FIRE_FRAME;
	allWeapons_.push_back(wep);

	// Mk84
	wep = createWeapon();
	wep->type_ = WT_IRONBOMB;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "Mk-84";		
	wep->modelName_ = "models/weapons/vwep_2000.md3";				
	wep->vwepName_ = "models/weapons/vwep_2000.md3";				
	wep->iconName_ = "hud/iconw_bmb.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 0;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 300;			
	wep->damage_ = 260;				
	wep->damageRadius_ = 330;			
	wep->spread_ = 0;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_BOMBMODE;				
	wep->crosshairTrack_ = CH_BOMBMODE;		
	wep->crosshairLock_ = CH_BOMBMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_PLANE;			
	wep->fitsPylon_ = PF_AG_HVY;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// gbu15
	wep = createWeapon();
	wep->type_ = WT_GUIDEDBOMB;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "GBU-15";		
	wep->modelName_ = "models/weapons/vwep_2000.md3";				
	wep->vwepName_ = "models/weapons/vwep_2000.md3";				
	wep->iconName_ = "hud/iconw_rkt.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 0;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 2000;			
	wep->damage_ = 300;				
	wep->damageRadius_ = 300;			
	wep->spread_ = 0;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0.985f;				
	wep->followCone_ = 0.996f;			
	wep->lockDelay_ = 1500;				
	wep->crosshair_ = CH_MISSILEMODE;				
	wep->crosshairTrack_ = CH_MISSILEMODETRACK;		
	wep->crosshairLock_ = CH_MISSILEMODELOCK;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_PLANE;			
	wep->fitsPylon_ = PF_AG_HVY;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// durandal
	wep = createWeapon();
	wep->type_ = WT_IRONBOMB;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "Durandal";		
	wep->modelName_ = "models/weapons/vwep_durandal.md3";				
	wep->vwepName_ = "models/weapons/vwep_durandal.md3";				
	wep->iconName_ = "hud/iconw_rkt.tga";				
	wep->targetCategory_ = CAT_GROUND;				
	wep->nonCatMod_ = 0.3f;				
	wep->muzzleVelocity_ = 0;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 300;			
	wep->damage_ = 300;				
	wep->damageRadius_ = 50;			
	wep->spread_ = 0;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_BOMBMODE;				
	wep->crosshairTrack_ = CH_BOMBMODE;		
	wep->crosshairLock_ = CH_BOMBMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_PLANE;			
	wep->fitsPylon_ = PF_AG_MED;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = WF_HAS_FIRE_FRAME;
	allWeapons_.push_back(wep);
}

