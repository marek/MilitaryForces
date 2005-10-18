
#ifndef __BG_VEHICLEINFO_H__
#define __BG_VEHICLEINFO_H__

#include "bg_vehicleextra.h"
#include "bg_datamanager.h"
#include <string>
#include <vector>

struct GameObjectInfo
{
	enum
	{
		GO_CAT_NONE		= 0x00,
		GO_CAT_PLANE	= 0x01,
		GO_CAT_HELO		= 0x02,
		GO_CAT_GROUND	= 0x04,
		GO_CAT_BOAT		= 0x08,
		GO_CAT_INF		= 0x10
	};

	virtual					~GameObjectInfo();

	// setup object
	virtual bool			setupGameObject() = 0;

	// all objects
	std::string				descriptiveName_;		// long descriptive name
	std::string				tinyName_;				// small ident name
    std::string				modelName_;				// just the directory of the model
	std::string				modelPath_;				// the actual full path (c:\...)
	unsigned int			gameSet_;				// gameset 
	unsigned int			category_;				// category bits
	unsigned int			class_;					// class bits
	unsigned int			flags_;					// special flags
	unsigned int			caps_;					// capabilities
	unsigned int			renderFlags_;			// special stuff for rendering only
    vec3_t					turnSpeed_;				// how fast can it turn around the three axis
	vec3_t					camDist_;				// how far away is the camera {min, max, defualt}
	vec3_t					camHeight_;				// how high is the default {min, max, default}
    unsigned int			maxHealth_;				// health
    HandleList				bodyParthandles_;		// handles for parts
	qhandle_t				shadowShader_;			// the shadow shader (use SHADOW_DEFAULT to use the default shader or SHADOW_NONE for no shadow)
	vec4_t					shadowCoords_;			// shadow apply coords { offsetX, offsetY, xAdjust, yAdjust }
	vec4_t					shadowAdjusts_;			// shadow apply adjustments { pitchMax, rollMax, pitchMod, rollMod }
	LoadoutMap				defaultLoadouts_;		// what weapons this object can carry
    vec3_t					mins_;					// bbox
    vec3_t					maxs_;					// bbox
    unsigned int			maxSpeed_;				// max speed at military thrust
	bool					playerSelectable_;		// can this be selected from vehicle selection menu ?

protected:
	// dont allow construction of this
							GameObjectInfo();

	// helper function to create a vehicle
	static GameObjectInfo*	createVehicle(unsigned int category);

	// get the path of this model
	bool					createModelPath();
	std::string				getModelPath( bool extension );

	// bounding box
	virtual bool			setupBoundingBox() = 0;

	// add a weapon to the specified loadout
	static bool				addWeaponToLoadout( Loadout& loadout, 
												 	std::string const& lookupName,
													std::string const& displayName,
													int maxAmmo,
													unsigned int selectionType,
													int turret = -1,
													bool limitedAngles = false,
													vec3_t minAngles = 0,
													vec3_t maxAngles = 0 );

private:
    // disable
							GameObjectInfo(GameObjectInfo const&);
	GameObjectInfo&			operator=(GameObjectInfo const&);

	// base path
	static const std::string baseModelPath;
};

struct GameObjectInfo_Vehicle : public GameObjectInfo
{
	// setup object
	virtual bool			setupGameObject();

	// general vehicle (non infantry)
	vec3_t					gunoffset_;				// guntag
	vec3_t					cockpitview_;			// to place the camera
	VehicleRadarInfo*		airRadar_;				// air radar
	VehicleRadarInfo*		groundRadar_;			// ground radar
	int						minThrottle_;
	int						maxThrottle_;
	unsigned int			acceleration_;	
	unsigned int			maxFuel_;
	TurretList				turrets_;
	MountList				mounts_;				// all mounts it has (unloaded)

protected:
	// dont allow construction of this
							GameObjectInfo_Vehicle();
	virtual					~GameObjectInfo_Vehicle();

	// bounding box
	virtual bool			setupBoundingBox();

	// go through loadouts to make sure they are ok
	virtual bool			verifyLoadouts();

	// make sure all the weapons actually fit on the mounts
	virtual bool			createWeaponMounts();	// create the empty mounts
	virtual void			correctArmament(Loadout& loadout, std::string const& loadoutName);

private:
    // disable
							GameObjectInfo_Vehicle(GameObjectInfo_Vehicle const&);
	GameObjectInfo_Vehicle&	operator=(GameObjectInfo_Vehicle const&);
};

struct GameObjectInfo_GroundVehicle : public GameObjectInfo_Vehicle
{
									GameObjectInfo_GroundVehicle();
	virtual							~GameObjectInfo_GroundVehicle();

	// helper to create all of this type
	static void						createAllGroundVehicles( GameObjectList& gameObjects );

	// wheeled vehicles
	int								wheels_;				// how many wheels does it have 
	float							wheelCF_;				// circumference

protected:
	// bounding box
	virtual bool					setupBoundingBox();

private:
    // disable
									GameObjectInfo_GroundVehicle(GameObjectInfo_GroundVehicle const&);
	GameObjectInfo_GroundVehicle&	operator=(GameObjectInfo_GroundVehicle const&);
};

struct GameObjectInfo_Aircraft : public GameObjectInfo_Vehicle
{
	// all aircraft
	int						engines_;				// how many engines does it actually have
	VehiclePartAnimInfo*	bayAnim_;				// bay animation info (if it has weapons bay)
	VehiclePartAnimInfo*	gearAnim_;				// gear animation info (if it has a retractable gear)
	float					tailAngle_;				// for taildraggers on ground (can be planes or helos)
	float					gearHeight_;			// height of gear
	unsigned int			abEffectModel_;			// num of afterburner model (for planes)

protected:
	// bounding box
	virtual bool			setupBoundingBox();

	// dont allow construction of this
							GameObjectInfo_Aircraft();
	virtual					~GameObjectInfo_Aircraft();

private:
    // disable
								GameObjectInfo_Aircraft(GameObjectInfo_Aircraft const&);
	GameObjectInfo_Aircraft&	operator=(GameObjectInfo_Aircraft const&);

};

struct GameObjectInfo_Plane : public GameObjectInfo_Aircraft
{
							GameObjectInfo_Plane();
	virtual					~GameObjectInfo_Plane();

	// helper to create all of this type
	static void				createAllPlanes( GameObjectList& gameObjects );

	// planes
    unsigned int			stallSpeed_;		

	// swing wing planes
	float					swingAngle_;				// for swing wings

protected:
	// bounding box
	virtual bool			setupBoundingBox();

private:
    // disable
							GameObjectInfo_Plane(GameObjectInfo_Plane const&);
	GameObjectInfo_Plane&	operator=(GameObjectInfo_Plane const&);

};

struct GameObjectInfo_Helicopter : public GameObjectInfo_Aircraft
{
							GameObjectInfo_Helicopter();
	virtual					~GameObjectInfo_Helicopter();

	// helper to create all of this type
	static void				createAllHelicopters( GameObjectList& gameObjects );

protected:
	// bounding box
	virtual bool			setupBoundingBox();

private:
    // disable
								GameObjectInfo_Helicopter(GameObjectInfo_Helicopter const&);
	GameObjectInfo_Helicopter&	operator=(GameObjectInfo_Helicopter const&);

};

struct GameObjectInfo_Boat : public GameObjectInfo_Vehicle
{
							GameObjectInfo_Boat();
	virtual					~GameObjectInfo_Boat();

	// helper to create all of this type
	static void				createAllBoats( GameObjectList& gameObjects );

	// boats
	VehicleRadarInfo*		sonarInfo_;				// in case it has sonar

private:
    // disable
							GameObjectInfo_Boat(GameObjectInfo_Boat const&);
	GameObjectInfo_Boat&	operator=(GameObjectInfo_Boat const&);

};

struct GameObjectInfo_Infantry : public GameObjectInfo
{
							GameObjectInfo_Infantry();
	virtual					~GameObjectInfo_Infantry();

	// helper to create all of this type
	static void				createAllInfantry( GameObjectList& gameObjects );

	// setup object
	virtual bool			setupGameObject();

	// infantry
	AnimationInfo*			animations_;			// Animation pointer

protected:
	// bounding box
	virtual bool			setupBoundingBox();

private:
    // disable
								GameObjectInfo_Infantry(GameObjectInfo_Infantry const&);
	GameObjectInfo_Infantry&	operator=(GameObjectInfo_Infantry const&);

};



#endif // __BG_VEHICLEINFO_H__
