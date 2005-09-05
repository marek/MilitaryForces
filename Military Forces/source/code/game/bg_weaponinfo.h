#ifndef __BG_WEAPONINFO_H__
#define __BG_WEAPONINFO_H__

#include "q_shared.h"
#include <string>



// list of weapons
struct WeaponInfo
{	
					WeaponInfo();
					~WeaponInfo();

	weaponType_t	type_;					// behaviour (ie bomb, missile etc)
	unsigned int	gameSet_;				// in which epoche can it appear
	std::string		descriptiveName_;		// displayed in-game
	std::string		modelName_;				// model
	qhandle_t		modelHandle_;			// handle
	std::string		vwepName_;				// vwep model
	qhandle_t		vwepHandle_;			// vwep handle
	std::string		iconName_;				// icon for HUD
	qhandle_t		iconHandle_;			// handle for icon
	unsigned int	targetCategory_;		// which category can it damage
	float			nonCatMod_;				// modifier for hitting wrong category
	unsigned int	muzzleVelocity_;		// speed at which it starts
	float			range_;					// target acquiring range
	unsigned int	fuelRange_;				// for how long lasts its fuel
	unsigned int	fireInterval_;			// time between two shots
	unsigned int	damage_;				// damage done to target
	unsigned int	damageRadius_;			// radius that damage is applied to
	unsigned int	spread_;				// how far off the target direction
	unsigned int	barrels_;				// mainly for guns
	float			barrelDistance_;		// how far off the center
	float			lockCone_;				// degrees of half cone to lock
	float			followCone_;			// degrees of half cone to follow
	int				lockDelay_;				// how long does it take to lock on
	unsigned int	crosshair_;				// which crosshair to use when this weapon is selected
	unsigned int	crosshairTrack_;		// which crosshair to use when this weapon is tracking
	unsigned int	crosshairLock_;			// which crosshair to use when this weapon is locked
	unsigned int	numberPerPackage_;		// for example how many rockets in FFAR etc (per mount)
	unsigned int	fitsCategory_;			// which vehicle can carry this weapon
	unsigned int	fitsPylon_;				// on which type of pylon does this go 
	unsigned int	basicECMVulnerability_;	// how likely in percent to be distracted
	unsigned int	flags_;					// flags
};






#endif	// __BG_WEAPONINFO_H__


