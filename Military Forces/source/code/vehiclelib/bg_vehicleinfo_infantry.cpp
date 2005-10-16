#include "bg_datamanager.h"
#include "bg_vehicleinfo.h"
#include "bg_md3utils.h"
#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../game/bg_public.h"

GameObjectInfo_Infantry::GameObjectInfo_Infantry() :
	animations_(0)
{
}

GameObjectInfo_Infantry::~GameObjectInfo_Infantry()
{
	delete animations_;
	animations_ = 0;
}

bool
GameObjectInfo_Infantry::setupGameObject()
{
	return GameObjectInfo::setupGameObject();
}

bool
GameObjectInfo_Infantry::setupBoundingBox()
{
	std::string const modelBaseName = getModelPath(false);

	// boundingbox
	if( !Md3Utils::getModelDimensions( modelBaseName + "_legs.md3", mins_, maxs_, LEGS_IDLE ) )
		return false;

	vec3_t maxs, mins;
	if( !Md3Utils::getModelDimensions( modelBaseName + "_torso.md3", mins, maxs, LEGS_IDLE ) )
		return false;
	maxs_[2] += maxs[2] - mins[2];

	if( !Md3Utils::getModelDimensions( modelBaseName + "_head.md3", mins, maxs ) )
		return false;
	maxs_[2] += maxs[2] - mins[2];

	// Scale LQMs
	for( int j = 0; j < 3; ++j ) 
	{
		maxs_[j] *= LQM_SCALE;
		mins_[j] *= LQM_SCALE;
	}

	return true;
}

void
GameObjectInfo_Infantry::createAllInfantry( GameObjectList& gameObjects )
{
	GameObjectInfo_Infantry* veh = 0;

	// Male
	veh = dynamic_cast<GameObjectInfo_Infantry*>(createVehicle(GameObjectInfo::GO_CAT_INF));
	veh->descriptiveName_ =	"Sarge";
	veh->tinyName_ = "Soldier";
	veh->modelName_ = "sarge";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_INF;
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
    veh->maxSpeed_ = 50;		
	veh->animations_ = 0;	
	gameObjects.push_back(veh);

	// Female
	veh = dynamic_cast<GameObjectInfo_Infantry*>(createVehicle(GameObjectInfo::GO_CAT_INF));
	veh->descriptiveName_ =	"Major";
	veh->tinyName_ = "Soldier";
	veh->modelName_ = "Major";
	veh->gameSet_ = MF_GAMESET_MODERN;
	veh->category_ = GameObjectInfo::GO_CAT_INF;
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
    veh->maxSpeed_ = 50;		
	veh->animations_ = 0;	
	gameObjects.push_back(veh);
}




