#ifndef __BG_VEHICLEEXTRA_H__
#define __BG_VEHICLEEXTRA_H__


#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include <string>
#include <vector>
#include <map>

struct Armament;
struct TurretInfo;
struct VehicleMountInfo;

typedef std::vector<Armament>			Loadout;
typedef std::map<std::string, Loadout>	LoadoutMap;
typedef LoadoutMap::iterator			LoadoutMapIter;
typedef std::vector<qhandle_t>			HandleList;
typedef std::vector<TurretInfo>			TurretList;
typedef std::vector<VehicleMountInfo>	MountList;


struct VehiclePartAnimInfo
{
	explicit		VehiclePartAnimInfo(unsigned int animTime) :
						maxFrame_(0), animTime_(animTime) 
						{}
		
	unsigned int	maxFrame_;		// number of frames
	unsigned int	animTime_;		// time it takes to play anim
};

struct VehicleRadarInfo
{
					VehicleRadarInfo(float trackCone,
									 unsigned int range,
									 unsigned int trackCat) :
						trackCone_(trackCone), range_(range), trackCategory_(trackCat)
						{}


	float			trackCone_;		// tracking cone of radar
	unsigned int	range_;			// the range it can track
	unsigned int	trackCategory_;	// the vehicle categories this radar (or sonar) can track
};

struct TurretInfo
{
	vec3_t			minAngles_;		// min angles of this turret (if applicable on these axis)
	vec3_t			maxAngles_;		// max angles of this turret (if applicable on these axis)
	vec3_t			turnSpeed_;		// how fast it can turn
};

struct VehicleMountInfo
{
					VehicleMountInfo();

	md3Tag_t		tag_;					// tag information
	int				position_;				// pos on the wing
	int				group_;					// group
	int				flags_;					// flags
	bool			left_;					// left side or right

	static bool		mountCompare( VehicleMountInfo const& a, VehicleMountInfo const& b );
};

struct VehicleMountInfoEvaluator : public VehicleMountInfo
{
					VehicleMountInfoEvaluator();
					VehicleMountInfoEvaluator(VehicleMountInfo const&);

	int				weaponIndex_;			// what is actually on
	int				num_;					// how many are actually on
};

struct Armament
{
					Armament();
					~Armament();

	enum
	{
		ST_NOT_SELECTABLE,		// non selectable weapons (who have special buttons, like flares)
		ST_PRIMARY_WEAPON,		// ie first fire button
		ST_SECONDARY_WEAPON,	// ie second fire button
	};

	std::string		displayName_;	// what it is displayed as
	int				weaponIndex_;	// what weapon is it
	int				maxAmmo_;		// how much ammo
	unsigned int	selectionType_;	// primary/secondary/non-selectable
	int				turret_;		// what turret this is on
	bool			limitedAngles_;	// if angles somehow limited
	vec3_t			minAngles_;		// min angles on this turret (if applicable on these axis)
	vec3_t			maxAngles_;		// max angles on this turret (if applicable on these axis)
};

struct AnimationInfo
{
	enum {
		ANIM_BOTH_DEATH1,
		ANIM_BOTH_DEAD1,
		ANIM_BOTH_DEATH2,
		ANIM_BOTH_DEAD2,
		ANIM_BOTH_DEATH3,
		ANIM_BOTH_DEAD3,

		ANIM_TORSO_GESTURE,

		ANIM_TORSO_ATTACK,
		ANIM_TORSO_ATTACK2,

		ANIM_TORSO_DROP,
		ANIM_TORSO_RAISE,

		ANIM_TORSO_STAND,
		ANIM_TORSO_STAND2,

		ANIM_LEGS_WALKCR,
		ANIM_LEGS_WALK,
		ANIM_LEGS_RUN,
		ANIM_LEGS_BACK,
		ANIM_LEGS_SWIM,

		ANIM_LEGS_JUMP,
		ANIM_LEGS_LAND,

		ANIM_LEGS_JUMPB,
		ANIM_LEGS_LANDB,

		ANIM_LEGS_IDLE,
		ANIM_LEGS_IDLECR,

		ANIM_LEGS_TURN,

		ANIM_TORSO_GETFLAG,
		ANIM_TORSO_GUARDBASE,
		ANIM_TORSO_PATROL,
		ANIM_TORSO_FOLLOWME,
		ANIM_TORSO_AFFIRMATIVE,
		ANIM_TORSO_NEGATIVE,

		ANIM_MAX_ANIMATIONS,

		ANIM_LEGS_BACKCR,
		ANIM_LEGS_BACKWALK,
		ANIM_FLAG_RUN,
		ANIM_FLAG_STAND,
		ANIM_FLAG_STAND2RUN,

		ANIM_MAX_TOTALANIMATIONS
	};	


	int				firstFrame_;
	int				numFrames_;
	int				loopFrames_;		// 0 to numFrames
	int				frameLerp_;			// msec between frames
	int				initialLerp_;		// msec to get to first frame
	int				reversed_;			// true if animation is reversed
	int				flipflop_;			// true if animation should flipflop back to base
};


#endif // __BG_VEHICLEEXTRA_H__
