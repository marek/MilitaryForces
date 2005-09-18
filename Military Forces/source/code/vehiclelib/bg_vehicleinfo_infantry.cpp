#include "bg_datamanager.h"
#include "bg_vehicleinfo.h"
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"

void
DataManager::createAllInfantry()
{
	VehicleInfo* veh = 0;

	// Male
	veh = createVehicle();
	veh->descriptiveName_ =	"Sarge";
	veh->tinyName_ = "Soldier";
	veh->modelName_ = "sarge";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = CAT_LQM;
	veh->class_ = CLASS_LQM_INFANTRY;
	veh->flags_ = 0;
	veh->caps_ = 0;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 125, 70 );		
	VectorSet( veh->camDist_, 0, 10, 0 );
	VectorSet( veh->camHeight_ , 0, 10, 2.5);		
    veh->maxHealth_ = 20;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 4, 0, 34, 24 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 16.65f, -3.391f, 0.378f );		
	VectorSet( veh->cockpitview_, 19, 0, 5 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 0;
	veh->acceleration_ = 0;
	veh->maxFuel_ = 0;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0.0f;		
	veh->engines_ = 0;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = 0;		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 0;	
	veh->swingAngle_ = 0;	
	veh->sonarInfo_ = 0; 		
    veh->maxSpeed_ = 50;		
	veh->animations_ = 0;	
	allVehicles_.push_back(veh);

	// Female
	veh = createVehicle();
	veh->descriptiveName_ =	"Major";
	veh->tinyName_ = "Soldier";
	veh->modelName_ = "Major";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = CAT_LQM;
	veh->class_ = CLASS_LQM_INFANTRY;
	veh->flags_ = 0;
	veh->caps_ = 0;
	veh->renderFlags_ = 0;	
    VectorSet( veh->turnSpeed_, 50, 125, 70 );		
	VectorSet( veh->camDist_, 0, 10, 0 );
	VectorSet( veh->camHeight_ , 0, 10, 2.5);		
    veh->maxHealth_ = 20;		
	veh->shadowShader_ = SHADOW_DEFAULT;	
	Vector4Set( veh->shadowCoords_, 4, 0, 34, 24 );	
	Vector4Set( veh->shadowAdjusts_, 0, 0, 0, 0 );	
	VectorSet( veh->gunoffset_, 16.65f, -3.391f, 0.378f );		
	VectorSet( veh->cockpitview_, 19, 0, 5 );	
	veh->airRadar_ = 0;		
	veh->groundRadar_ = 0;			
	veh->minThrottle_ = 0;
	veh->maxThrottle_ = 0;
	veh->acceleration_ = 0;
	veh->maxFuel_ = 0;
	veh->wheels_ = 0;		
	veh->wheelCF_ = 0.0f;		
	veh->engines_ = 0;		
	veh->bayAnim_ = 0;		
	veh->gearAnim_ = 0;		
	veh->tailAngle_ = 0;		
	veh->abEffectModel_ = 0;	
    veh->stallSpeed_ = 0;	
	veh->swingAngle_ = 0;	
	veh->sonarInfo_ = 0; 		
    veh->maxSpeed_ = 50;		
	veh->animations_ = 0;	
	allVehicles_.push_back(veh);
}




