
#ifndef __BG_VEHICLEINFO_H__
#define __BG_VEHICLEINFO_H__

#include "bg_vehicleextra.h"
#include <string>
#include <vector>

struct md3Tag_t;

struct VehicleInfo
{
							VehicleInfo();
							~VehicleInfo();

	// go through loadouts to make sure they are ok
	void					verifyLoadouts();

	// all vehicles
	std::string				descriptiveName_;		// long descriptive name
	std::string				tinyName_;				// small ident name
    std::string				modelName_;				// just the directory of the model
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
	LoadoutList				defaultLoadouts_;		// what weapons this vehicle carries		
    vec3_t					mins_;
    vec3_t					maxs_;

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

	// wheeled vehicles
	int						wheels_;				// how many wheels does it have 
	float					wheelCF_;				// circumference

	// all aircraft
	int						engines_;				// how many engines does it actually have
	VehiclePartAnimInfo*	bayAnim_;				// bay animation info (if it has weapons bay)
	VehiclePartAnimInfo*	gearAnim_;				// gear animation info (if it has a retractable gear)
	float					tailAngle_;				// for taildraggers on ground (can be planes or helos)
	float					gearHeight_;			// height of gear
	unsigned int			abEffectModel_;			// num of afterburner model (for planes)

	// planes
    unsigned int			stallSpeed_;		

	// swing wing planes
	float					swingAngle_;				// for swing wings

	// boats
	VehicleRadarInfo*		sonarInfo_;				// in case it has sonar

	// infantry
    unsigned int			maxSpeed_;				// max speed at military thrust
	AnimationInfo*			animations_;			// Animation pointer

protected:
	// make sure all the weapons actually fit on the mounts
	void					createWeaponMounts();

	// get the path of this model
	std::string				getModelPath( bool extension );

	// find all tags containing string str
	int						getTagsContaining( std::string const& filename, 
											   std::string const& str,
											   std::vector<md3Tag_t>& tagList );

private:
    // disable
							VehicleInfo(VehicleInfo const&);
	VehicleInfo&			operator=(VehicleInfo const&);

	// base path
	static const std::string baseModelPath;
};







#endif // __BG_VEHICLEINFO_H__
