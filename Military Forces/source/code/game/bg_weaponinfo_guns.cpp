#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"
#include "bg_datamanager.h"
#include "bg_weaponinfo.h"




void						
DataManager::createAllGuns()
{
	WeaponInfo* wep = 0;

	// machinegun: WW1, cal .303, 2 barrels
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_WW1;				
	wep->descriptiveName_ = "2x cal.303 MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 2000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 150;			
	wep->damage_ = 10;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 8;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;	
	allWeapons_.push_back(wep);

	// machinegun: WW1, cal .312, 2 barrels
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_WW1;				
	wep->descriptiveName_ = "2x cal.312 MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 2000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 150;			
	wep->damage_ = 10;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 8;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: WW2, cal 50, 8 barrels
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_WW2;				
	wep->descriptiveName_ = "8x cal.50 MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 150;			
	wep->damage_ = 15;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 8;				
	wep->barrels_ = 8;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: WW2, cal 50, 6 barrels
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_WW2;				
	wep->descriptiveName_ = "6x cal.50 MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 150;			
	wep->damage_ = 15;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 8;				
	wep->barrels_ = 6;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: any set, 12.7mm, 1 barrel
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "12.7mm MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 6;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 14;				
	wep->barrels_ = 1;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: any set, 14.5mm, 2 barrels
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "Dual 14.5mm MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 120;			
	wep->damage_ = 7;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 14;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: any set,  4x14.5mm, 4 barrel
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_ANY;				
	wep->descriptiveName_ = "Quad 14.5mm MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 7;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 14;				
	wep->barrels_ = 4;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: modern, 20mm, 1 barrel
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "20mm MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 6500.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 12;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 14;				
	wep->barrels_ = 1;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: modern, 30mm, 1 barrel
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "30mm MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 2500;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 35;				
	wep->damageRadius_ = 30;			
	wep->spread_ = 16;				
	wep->barrels_ = 1;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: modern, 20mm, 2 barrel
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "Dual 20mm MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 17;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 10;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: M4A1
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "M4A1";		
	wep->modelName_ = "models/weapons/m4a1.md3";				
	wep->vwepName_ = "models/weapons/m4a1.md3";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 2500;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 150;			
	wep->damage_ = 2;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 16;				
	wep->barrels_ = 1;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: modern, 30mm, 2 barrel
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "Dual 30mm MG";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 2800;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 140;			
	wep->damage_ = 20;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 10;				
	wep->barrels_ = 2;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: modern, 7.62mm minigun turret
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "7.62mm Minigun";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 10;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 8;				
	wep->barrels_ = 1;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// machinegun: modern, 12.7mm turret
	wep = createWeapon();
	wep->type_ = WT_MACHINEGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "7.62mm Minigun";		
	wep->modelName_ = "";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_mg.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 3000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 100;			
	wep->damage_ = 10;				
	wep->damageRadius_ = 0;			
	wep->spread_ = 8;				
	wep->barrels_ = 1;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_GUNMODE;				
	wep->crosshairTrack_ = CH_GUNMODE;		
	wep->crosshairLock_ = CH_GUNMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_ANY;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// 100mm Gun
	wep = createWeapon();
	wep->type_ = WT_BALLISTICGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "100mm Gun";		
	wep->modelName_ = "models/weapons/ffar.md3";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_rkt.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 2000;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 1500;			
	wep->damage_ = 110;				
	wep->damageRadius_ = 60;			
	wep->spread_ = 1;				
	wep->barrels_ = 1;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_ROCKETMODE;				
	wep->crosshairTrack_ = CH_ROCKETMODE;		
	wep->crosshairLock_ = CH_ROCKETMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_GROUND;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);

	// 125mm Gun
	wep = createWeapon();
	wep->type_ = WT_BALLISTICGUN;					
	wep->gameSet_ = MF_GAMESET_MODERN;				
	wep->descriptiveName_ = "125mm Gun";		
	wep->modelName_ = "models/weapons/ffar.md3";				
	wep->vwepName_ = "";				
	wep->iconName_ = "hud/iconw_rkt.tga";				
	wep->targetCategory_ = CAT_ANY;				
	wep->nonCatMod_ = 0.0f;				
	wep->muzzleVelocity_ = 2500;		
	wep->range_ = 0.0f;					
	wep->fuelRange_ = 0;				
	wep->fireInterval_ = 1500;			
	wep->damage_ = 150;				
	wep->damageRadius_ = 80;			
	wep->spread_ = 1;				
	wep->barrels_ = 1;				
	wep->barrelDistance_ = 0;		
	wep->lockCone_ = 0;				
	wep->followCone_ = 0;			
	wep->lockDelay_ = 0;				
	wep->crosshair_ = CH_ROCKETMODE;				
	wep->crosshairTrack_ = CH_ROCKETMODE;		
	wep->crosshairLock_ = CH_ROCKETMODE;			
	wep->numberPerPackage_ = 1;		
	wep->fitsCategory_ = CAT_GROUND;			
	wep->fitsPylon_ = PF_NA;				
	wep->basicECMVulnerability_ = 0;	
	wep->flags_ = 0;
	allWeapons_.push_back(wep);


}








