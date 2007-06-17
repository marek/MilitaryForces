#ifndef __BG_WEAPONINFO_H__
#define __BG_WEAPONINFO_H__

#include "../game/q_shared.h"
#include "bg_datamanager.h"
//#include "../game/bg_public.h"
#include <string>



// list of weapons
struct WeaponInfo
{	
	// pylon info
	enum
	{
		PF_NOT_AP		= 0x000000,		// not applicable

		// anti aircraft missiles
		PF_AAM_ST		= 0x000001,		// Stinger	(very small AA)
		PF_AAM_SW		= 0x000002,		// Sidewinder (small AA)
		PF_AAM_AM		= 0x000004,		// AMRAAM (medium AA)
		PF_AAM_PH		= 0x000008,		// Phoenix (heavy AA)

		// bombs (dumb and guided)
		PF_BMB_82		= 0x000010,		// bomb Mk82 (light)
		PF_BMB_83		= 0x000020,		// bomb Mk83 (medium)
		PF_BMB_84		= 0x000040,		// bomb Mk84 (heavy)
		PF_BMB_GD		= 0x000080,		// guided bomb (as extra flag to bomb flags)

		// anti ground missiles
		PF_AGM_HF		= 0x000100,		// Hellfire (light AGM)
		PF_AGM_MV		= 0x000200,		// Maverick (heavy AGM)
		PF_AGM_HM		= 0x000400,		// HARM (anti radar)
		PF_AGM_DU		= 0x000800,		// Durandal (anti runway)

		// other weapons
		PF_OTW_FL		= 0x001000,		// FFAR light = dont remove
		PF_OTW_FH		= 0x002000,		// FFAR heavy = dont remove
		PF_OTW_NL		= 0x004000,		// nuke light
		PF_OTW_NH		= 0x008000,		// nuke heavy

		// misc stuff
		PF_MSC_FL		= 0x010000,		// flare pod = dont remove
		PF_MSC_EC		= 0x020000,		// ECM pod = dont remove
		PF_MSC_TL		= 0x040000,		// drop tank light
		PF_MSC_TH		= 0x080000,		// drop tank heavy

		// flags
		PF_FLG_DD		= 0x100000,		// dont drop on launch (for example wingtip)
		// free flags: 0x200000 - 0x800000

		// helper flags
		PF_FLH_DR		= 0x033000,		// these weapons arent removed on launch                                                                                                                                                                                                                                                                                                                                                                                                                                                                             
		
	};

	virtual						~WeaponInfo();

	// helper to create a weapon
	static WeaponInfo*			createWeapon();

	// helpers to create different categories
	static void					createAllGuns(WeaponList& weapons);
	static void					createAllMissiles(WeaponList& weapons);
	static void					createAllBombs(WeaponList& weapons);
	static void					createAllOther(WeaponList& weapons);

	weaponType_t				type_;					// behaviour (ie bomb, missile etc)
	unsigned int				gameSet_;				// in which epoche can it appear
	std::string					descriptiveName_;		// displayed in-game
	std::string					modelName_;				// model
	qhandle_t					modelHandle_;			// handle
	std::string					vwepName_;				// vwep model
	qhandle_t					vwepHandle_;			// vwep handle
	std::string					iconName_;				// icon for HUD
	qhandle_t					iconHandle_;			// handle for icon
	unsigned int				targetCategory_;		// which category can it damage
	float						nonCatMod_;				// modifier for hitting wrong category
	unsigned int				muzzleVelocity_;		// speed at which it starts
	float						range_;					// target acquiring range
	unsigned int				fuelRange_;				// for how long lasts its fuel
	unsigned int				fireInterval_;			// time between two shots
	unsigned int				damage_;				// damage done to target
	unsigned int				damageRadius_;			// radius that damage is applied to
	unsigned int				spread_;				// how far off the target direction
	unsigned int				barrels_;				// mainly for guns
	float						barrelDistance_;		// how far off the center
	float						lockCone_;				// degrees of half cone to lock
	float						followCone_;			// degrees of half cone to follow
	int							lockDelay_;				// how long does it take to lock on
	unsigned int				crosshair_;				// which crosshair to use when this weapon is selected
	unsigned int				crosshairTrack_;		// which crosshair to use when this weapon is tracking
	unsigned int				crosshairLock_;			// which crosshair to use when this weapon is locked
	unsigned int				numberPerPackage_;		// for example how many rockets in FFAR etc (per mount)
	unsigned int				fitsCategory_;			// which vehicle can carry this weapon
	unsigned int				fitsPylon_;				// on which type of pylon does this go 
	unsigned int				basicECMVulnerability_;	// how likely in percent to be distracted
	unsigned int				flags_;					// flags

protected:
	// dont allow construction 
								WeaponInfo();

};






#endif	// __BG_WEAPONINFO_H__


