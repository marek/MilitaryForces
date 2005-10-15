#include "bg_datamanager.h"
#include "bg_vehicleinfo.h"
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"


GameObjectInfo_GroundVehicle::GameObjectInfo_GroundVehicle() :
	wheels_(0),
	wheelCF_(0.0f)
{
}

GameObjectInfo_GroundVehicle::~GameObjectInfo_GroundVehicle()
{
}




void
GameObjectInfo_GroundVehicle::createAllGroundVehicles( GameObjectList& gameObjects )
{
	GameObjectInfo_GroundVehicle* veh = 0;


	// --- MODERN --- 

	// M-1
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"M1A1i Abrams";
	veh->tinyName_ = "M1";
	veh->modelName_ = "m1";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_MBT;
	veh->flags_ = 0;
	veh->caps_ = 0;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 40 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 400;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 5, 0, 8 );		
	VectorSet( veh->cockpitview_, 0, 0, 14 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 9000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(0, 6000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 25;
	veh->maxFuel_ = 150;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0.0f;		
    veh->maxSpeed_ = 60;		
	gameObjects.push_back(veh);
	//310,						// max gun pitch (upwards = negative) <- gearheight
	//3,							// min gun pitch (downwards = positive) <- tailangle

	// T-90
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"T-90";
	veh->tinyName_ = "T-90";
	veh->modelName_ = "t-90";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_MBT;
	veh->flags_ = 0;
	veh->caps_ = 0;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 40 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 380;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 25, 0, 2 );		
	VectorSet( veh->cockpitview_, 0, 0, 14 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 9000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(0, 6000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 25;
	veh->maxFuel_ = 150;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0.0f;		
    veh->maxSpeed_ = 65;		
	gameObjects.push_back(veh);
	//310,						// max gun pitch (upwards = negative) <- gearheight
	//3,							// min gun pitch (downwards = positive) <- tailangle

	// Hummer 50cal
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"Hummer";
	veh->tinyName_ = "HMV";
	veh->modelName_ = "humvee_50cal";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_RECON;
	veh->flags_ = 0;
	veh->caps_ = HC_WHEELS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 130;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 8, 0, 2 );		
	VectorSet( veh->cockpitview_, 0, 0, 10 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 8000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(0, 4000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 35;
	veh->maxFuel_ = 160;
	veh->wheels_ = 4;		
	veh->wheelCF_ = 8.0f;		
    veh->maxSpeed_ = 85;		
	gameObjects.push_back(veh);
	//300,						// max gun pitch (upwards = negative) <- gearheight
	//6,							// min gun pitch (downwards = positive) <- tailangle

	// Hummer Avenger
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"Hummer Avenger";
	veh->tinyName_ = "HMVAA";
	veh->modelName_ = "humvee_avng";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_SAM;
	veh->flags_ = 0;
	veh->caps_ = HC_WHEELS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 130;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 8, 0, 2 );		
	VectorSet( veh->cockpitview_, 0, 0, 10 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 8000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(0, 4000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 35;
	veh->maxFuel_ = 160;
	veh->wheels_ = 4;		
	veh->wheelCF_ = 8.0f;		
    veh->maxSpeed_ = 85;		
	gameObjects.push_back(veh);
	//300,						// max gun pitch (upwards = negative) <- gearheight
	//6,							// min gun pitch (downwards = positive) <- tailangle

	// BRDM2
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"BRDM 2";
	veh->tinyName_ = "BRDM2";
	veh->modelName_ = "brdm2";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_RECON;
	veh->flags_ = 0;
	veh->caps_ = HC_DUALGUNS|HC_AMPHIBIOUS|HC_WHEELS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 170;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 8, 0, 2 );		
	VectorSet( veh->cockpitview_, 0, 0, 10 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 8000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(0, 5000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 33;
	veh->maxFuel_ = 160;
	veh->wheels_ = 4;		
	veh->wheelCF_ = 9.0f;		
    veh->maxSpeed_ = 90;		
	gameObjects.push_back(veh);
	//285,						// max gun pitch (upwards = negative) <- gearheight
	//5,							// min gun pitch (downwards = positive) <- tailangle

	// ZSU-23
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"ZSU-23";
	veh->tinyName_ = "zsu23";
	veh->modelName_ = "ZSU-23";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_SAM;
	veh->flags_ = 0;
	veh->caps_ = HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 160;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 3, 1, 6 );		
	VectorSet( veh->cockpitview_, 0, 0, 10 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 8000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(0, 5000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 33;
	veh->maxFuel_ = 160;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0;		
    veh->maxSpeed_ = 70;		
	gameObjects.push_back(veh);
	//285,						// max gun pitch (upwards = negative) <- gearheight
	//5,							// min gun pitch (downwards = positive) <- tailangle


	// MLRS
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"MLRS";
	veh->tinyName_ = "MLRS";
	veh->modelName_ = "mlrs";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_SAM;
	veh->flags_ = 0;
	veh->caps_ = HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 250;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 5, 0, 8 );		
	VectorSet( veh->cockpitview_, 0, 0, 14 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 8000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(0, 4000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 20;
	veh->maxFuel_ = 160;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0;		
    veh->maxSpeed_ = 60;		
	gameObjects.push_back(veh);
	//335,						// max gun pitch (upwards = negative) <- gearheight
	//3,							// min gun pitch (downwards = positive) <- tailangle




	// --- WW2 --- 




	// Panzer III
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"Panzer III";
	veh->tinyName_ = "Pz 3";
	veh->modelName_ = "panzeriii";
	veh->gameSet_ = MF_GAMESET_WW2;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_MBT;
	veh->flags_ = 0;
	veh->caps_ = 0;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 400;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 5, 0, 8 );		
	VectorSet( veh->cockpitview_, 0, 0, 14 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 20;
	veh->maxFuel_ = 150;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0;		
    veh->maxSpeed_ = 60;		
	gameObjects.push_back(veh);
	//335,						// max gun pitch (upwards = negative) <- gearheight
	//3,							// min gun pitch (downwards = positive) <- tailangle

	// M4
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"M4 Sherman";
	veh->tinyName_ = "M4";
	veh->modelName_ = "m4";
	veh->gameSet_ = MF_GAMESET_WW2;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_MBT;
	veh->flags_ = 0;
	veh->caps_ = 0;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 100 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 350;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 5, 0, 8 );		
	VectorSet( veh->cockpitview_, 0, 0, 14 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 20;
	veh->maxFuel_ = 150;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0;		
    veh->maxSpeed_ = 60;		
	gameObjects.push_back(veh);
	//335,						// max gun pitch (upwards = negative) <- gearheight
	//3,							// min gun pitch (downwards = positive) <- tailangle

	// static SAM turret
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"SAM Mk1";
	veh->tinyName_ = "SAM";
	veh->modelName_ = "turret_samup";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_SAM;
	veh->flags_ = 0;
	veh->caps_ = 0;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 0, 0, 0 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 150;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 3, 1, 6 );		
	VectorSet( veh->cockpitview_, 0, 0, 10 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 5000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 0;
	veh->acceleration_ = 0;
	veh->maxFuel_ = 0;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0;		
    veh->maxSpeed_ = 0;		
	gameObjects.push_back(veh);


	// static AAA turret
	veh = dynamic_cast<GameObjectInfo_GroundVehicle*>(createVehicle(GameObjectInfo::GO_CAT_GROUND));
	veh->descriptiveName_ =	"AAA Mk1";
	veh->tinyName_ = "AAA";
	veh->modelName_ = "turret_aaa";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_GROUND;
	veh->class_ = CLASS_GROUND_SAM;
	veh->flags_ = 0;
	veh->caps_ = 0;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 0, 0, 0 );		
	VectorSet( veh->camDist_, 0, 100, 30 );
	VectorSet( veh->camHeight_ , 0, 100, 10);		
    veh->maxHealth_ = 150;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 0, 0, 0, 0 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 3, 1, 6 );		
	VectorSet( veh->cockpitview_, 0, 0, 10 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 5000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 0;
	veh->acceleration_ = 0;
	veh->maxFuel_ = 0;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0;		
    veh->maxSpeed_ = 0;		
	gameObjects.push_back(veh);
}



