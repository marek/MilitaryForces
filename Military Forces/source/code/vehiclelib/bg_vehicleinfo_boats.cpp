#include "bg_datamanager.h"
#include "bg_vehicleinfo.h"
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"


GameObjectInfo_Boat::GameObjectInfo_Boat() :
	sonarInfo_(0)
{

}

GameObjectInfo_Boat::~GameObjectInfo_Boat()
{
	delete sonarInfo_;
	sonarInfo_ = 0;
}




void
GameObjectInfo_Boat::createAllBoats( GameObjectList& gameObjects )
{
	GameObjectInfo_Boat* veh = 0;

	// Patrolboat
	veh = dynamic_cast<GameObjectInfo_Boat*>(createVehicle(GameObjectInfo::GO_CAT_BOAT));
	veh->descriptiveName_ =	"Patrolboat";
	veh->tinyName_ = "PBR31";
	veh->modelName_ = "pbr31mk2";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_BOAT;
	veh->class_ = CLASS_BOAT_PATROL;
	veh->flags_ = 0;
	veh->caps_ = HC_DUALGUNS;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 100, 20 );		
	VectorSet( veh->camDist_, 0, 100, 70 );
	VectorSet( veh->camHeight_ , 0, 100, 20);		
    veh->maxHealth_ = 350;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 4, 0, 34, 24 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 5, 0, 8 );		
	VectorSet( veh->cockpitview_, 0, 0, 6 );	
	veh->airRadar_ = new VehicleRadarInfo(-1, 8000, CAT_PLANE|CAT_HELO);		
	veh->groundRadar_ = new VehicleRadarInfo(0, 7000, CAT_GROUND|CAT_BOAT);			
	veh->minThrottle_ = -5;
	veh->maxThrottle_ = 10;
	veh->acceleration_ = 15;
	veh->maxFuel_ = 150;
	veh->sonarInfo_ = new VehicleRadarInfo(0, 3000, CAT_BOAT); 		
    veh->maxSpeed_ = 50;		
	gameObjects.push_back(veh);
	//8,							// max angle to lean when turning <- gearheight
	//-5,							// bowangle (depends on speeed) <-tailangle 
}






