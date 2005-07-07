/*
 * $Id: bg_weapondata.c,v 1.40 2005-07-07 22:22:05 minkis Exp $
*/

#include "q_shared.h"
#include "bg_public.h"


// just contains data of all the available weapons
completeWeaponData_t availableWeapons[] = 
{
	// empty entry
	{ 
		0,									// type
		0,									// gameset
		"",									// descriptiveName
		"",									// shortName
		"",									// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"",									// iconName
		0,									// iconHandle
		0,									// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		0,									// fire interval
		0,									// damage
		0,									// damageRadius
		0,									// spread
		0,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		0,									// crosshair
		0,									// crosshair track
		0,									// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		0,									// number per package
		0,									// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: WW1, cal .303, 2 barrels
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_WW1,						// gameset
		"2x cal.303 MG",					// descriptiveName
		"MG",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		150,								// fire interval
		10,									// damage
		0,									// damageRadius
		8,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		8,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: WW1, cal .312, 2 barrels
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_WW1,						// gameset
		"2x cal.312 MG",					// descriptiveName
		"MG",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		150,								// fire interval
		10,									// damage
		0,									// damageRadius
		8,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		8,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: WW2, cal 50, 8 barrels
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_WW2,						// gameset
		"8x cal.50 MG",						// descriptiveName
		"MG",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3000,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		150,								// fire interval
		15,									// damage
		0,									// damageRadius
		8,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		8,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: WW2, cal 50, 6 barrels
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_WW2,						// gameset
		"6x cal.50 MG",						// descriptiveName
		"MG",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3000,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		150,								// fire interval
		15,									// damage
		0,									// damageRadius
		8,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		6,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: any set, 12.7mm, 1 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"12.7mm MG",						// descriptiveName
		"MG",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3000,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		100,								// fire interval
		6,									// damage
		0,									// damageRadius
		14,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		1,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: any set, 14.5mm, 1 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"Dual 14.5mm MG",					// descriptiveName
		"MG",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3000,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		120,								// fire interval
		7,									// damage
		0,									// damageRadius
		14,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: any set,  4x14.5mm, 1 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"Quad 14.5mm MG",					// descriptiveName
		"MG",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3000,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		100,								// fire interval
		7,									// damage
		0,									// damageRadius
		14,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		4,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: modern, 20mm, 1 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"20mm MG",							// descriptiveName
		"GUN",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3000,								// muzzleVelocity
		6500,								// range (for AAA-GI guns)
		0,									// fuelrange
		100,								// fire interval
		12,									// damage
		0,									// damageRadius
		14,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		1,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},


	// machinegun: modern, 30mm, 1 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"30mm MG",							// descriptiveName
		"GUN",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2500,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		100,								// fire interval
		35,									// damage
		30,									// damageRadius
		16,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		1,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},



	// machinegun: modern, 20mm, 2 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"Dual 20mm MG",						// descriptiveName
		"GUN",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3000,								// muzzleVelocity 
		0,									// range
		0,									// fuelrange
		100,								// fire interval
		17,									// damage
		0,									// damageRadius
		10,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: M4A1
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"M4A1",								// descriptiveName
		"GUN",								// shortName
		"MG",								// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_LQM,							// cat
		0,									// non cat mod
		2500,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		150,								// fire interval
		2,									// damage
		30,									// damageRadius
		16,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		1,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		30,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: modern, 30mm, 2 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"Dual 30mm MG",						// descriptiveName
		"GUN",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2800,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		140,								// fire interval
		20,									// damage
		0,									// damageRadius
		10,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		1,									// turret
		{5,120,0},							// max turret angles
		{-75,-120,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: modern, 7.62mm minigun turret
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"7.62mm Minigun",						// descriptiveName
		"GUN",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3500,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		100,								// fire interval
		10,									// damage
		0,									// damageRadius
		5,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		1,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		1,									// turret
		{45,-2,0},							// max turret angles
		{-10,-178,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// machinegun: modern, 12.7mm turret
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"12.7mm MG",						// descriptiveName
		"GUN",								// shortName
		"MG",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2900,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		100,								// fire interval
		6,									// damage
		0,									// damageRadius
		10,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		1,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		1,									// turret
		{45,45,0},							// max turret angles
		{-45,135,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// FFAR
	{ 
		WT_ROCKET,							// type
		MF_GAMESET_ANY,						// gameset
		"FFAR (19)",						// descriptiveName
		"FFAR",								// shortName
		"FFAR",								// shortName2 (GVs)
		"models/weapons/vwep_rocket.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_ffar.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		1500,								// muzzleVelocity
		0,									// range
		5000,								// fuelrange
		180,								// fire interval
		40,									// damage
		50,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_ROCKETMODE,						// crosshair
		CH_ROCKETMODE,						// crosshair track
		CH_ROCKETMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		19,									// number per package
		CAT_PLANE|CAT_HELO,					// fits on category
		PF_FFAR,							// fits on pylon
		0,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// FFAR
	{ 
		WT_ROCKET,							// type
		MF_GAMESET_ANY,						// gameset
		"FFAR (7)",							// descriptiveName
		"FFAR",								// shortName
		"FFAR",								// shortName2 (GVs)
		"models/weapons/vwep_rocket.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_ffar_lt.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		1500,								// muzzleVelocity
		0,									// range
		5000,								// fuelrange
		180,								// fire interval
		40,									// damage
		50,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_ROCKETMODE,						// crosshair
		CH_ROCKETMODE,						// crosshair track
		CH_ROCKETMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		7,									// number per package
		CAT_PLANE|CAT_HELO,					// fits on category
		PF_FFAR,							// fits on pylon
		0,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},


	// FFAR
	{ 
		WT_ROCKET,							// type
		MF_GAMESET_ANY,						// gameset
		"FFAR+ (6)",						// descriptiveName
		"FFAR+",							// shortName
		"FFAR+",							// shortName2 (GVs)
		"models/weapons/vwep_rocket_hv.md3",// modelName
		0,									// handle
		"models/weapons/vwep_ffar_hv.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		1500,								// muzzleVelocity
		0,									// range
		5000,								// fuelrange
		200,								// fire interval
		80, 								// damage
		80,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_ROCKETMODE,						// crosshair
		CH_ROCKETMODE,						// crosshair track
		CH_ROCKETMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		6,									// number per package
		CAT_PLANE|CAT_HELO,					// fits on category
		PF_FFAR,							// fits on pylon  !! PF_FFAR&PF_AG_HVY !!
		0,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},


	// tank gun
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"100mm Gun",						// descriptiveName
		"GUN",								// shortName
		"GUN",								// shortName2 (GVs)
		"models/weapons/ffar.md3",			// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		1500,								// fire interval
		110,								// damage
		60,									// damageRadius
		1,									// spread
		1,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_ROCKETMODE,						// crosshair
		CH_ROCKETMODE,						// crosshair track
		CH_ROCKETMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_GROUND,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// tank gun
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"125mm Gun",						// descriptiveName
		"GUN",								// shortName
		"GUN",								// shortName2 (GVs)
		"models/weapons/ffar.md3",			// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2500,								// muzzleVelocity
		0,									// range
		0,									// fuelrange
		1500,								// fire interval
		150,								// damage
		80,									// damageRadius
		1,									// spread
		1,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_ROCKETMODE,						// crosshair
		CH_ROCKETMODE,						// crosshair track
		CH_ROCKETMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_GROUND,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// iron bombs
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"Mk82",								// descriptiveName
		"MK-82",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_500.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_500.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		100,								// fire interval
		50,								// damage
		150,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},


	// iron bombs
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"Mk82 R",							// descriptiveName
		"MK-82 R",							// shortName
		"BAD SHIT R",						// shortName2 (GVs)
		"models/weapons/vwep_500.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_500.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		100,								// fire interval
		50,									// damage
		150,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_LT,							// fits on pylon
		0,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},


	// iron bombs
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"Mk83",								// descriptiveName
		"MK-83",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_1000.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_1000.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		120,								// damage
		230,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_MED,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},


	// iron bombs
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"Mk83 R",								// descriptiveName
		"MK-83 R",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_1000.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_1000.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		120,								// damage
		230,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_MED,							// fits on pylon
		0,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},



	// iron bombs
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"Mk84",								// descriptiveName
		"MK-84",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_2000.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_2000.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		300,								// fire interval
		260,								// damage
		330,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_HVY,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// gbu15
	{ 
		WT_GUIDEDBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"GBU-15",							// descriptiveName
		"GBU-15",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_gbu15.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_gbu15.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		3000,								// fire interval
		300,								// damage
		50,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.996f,								// followcone (dot)
		1500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_HVY,							// fits on pylon  !! PF_AG_HVY&PF_AG_GDA !!
		0,									// basicECMVulnerability
		0,									// flags
	},

		// durandal
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"Durandal",							// descriptiveName
		"Durandal",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_durandal.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_durandal.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		1000,								// muzzleVelocity
		8000,								// range
		5000,								// fuelrange
		300,								// fire interval
		600,								// damage
		30,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_MED,							// fits on pylon
		0,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Sidewinder
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"AIM-9 Sidewinder",					// descriptiveName
		"9AIM-9P",							// shortName
		"SR-SAM",							// shortName2 (GVs)
		"models/weapons/vwep_sidewinder.md3",// modelName
		0,									// handle
		"models/weapons/vwep_sidewinder.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1000,								// muzzleVelocity
		1800,								// range
		2500,								// fuelrange
		400,								// fire interval
		70,									// damage
		40,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.5f,								// lockcone (dot)
		0.5f,								// followcone (dot) (0.996)
		1000,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE|CAT_HELO,					// fits on category
		PF_AA_LT,							// fits on pylon
		30,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// AMRAAM
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"AIM-120 AMRAMM",					// descriptiveName
		"AIM-120",							// shortName
		"MR-SAM",							// shortName2 (GVs)
		"models/weapons/vwep_amraam.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_amraam.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		4500,								// range
		6500,								// fuelrange
		600,								// fire interval
		90,									// damage
		60,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.7f,								// lockcone (dot)
		0.7f,								// followcone (dot)
		1500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AA_MED,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Sparrow
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"AIM-7 Sparrow",					// descriptiveName
		"AIM-7",							// shortName
		"MR_SAM",							// shortName2 (GVs)
		"models/weapons/vwep_sparrow.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_sparrow.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		2200,								// muzzleVelocity
		8000,								// range
		11000,								// fuelrange
		900,								// fire interval
		120,								// damage
		90,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.9f,								// lockcone (dot)
		0.9f,								// followcone (dot)
		1500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AA_HVY,							// fits on pylon
		20,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Phoenix
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"AIM-54 Phoenix",					// descriptiveName
		"AIM-54",							// shortName
		"LR_SAM",							// shortName2 (GVs)
		"models/weapons/vwep_phoenix.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_phoenix.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		3500,								// muzzleVelocity
		18000,								// range
		20000,								// fuelrange
		1200,								// fire interval
		300,								// damage
		120,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.996f,								// followcone (dot)
		3000,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AA_PHX,							// fits on pylon
		15,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Stinger
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"Stinger",							// descriptiveName
		"SR-AAM",							// shortName
		"SR-SAM",							// shortName2 (GVs)
		"models/weapons/vwep_stinger.md3",// modelName
		0,									// handle
		"models/weapons/vwep_stinger.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0.3f,								// non cat mod
		1100,								// muzzleVelocity
		4500,								// range
		9000,								// fuelrange
		500,								// fire interval
		60,									// damage
		40,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.7f,								// lockcone (dot)
		0.7f,								// followcone (dot)
		1500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{5,180,0},							// max turret angles
		{-85,-180,0},						// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_AA_LT,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Atoll
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"AA-2 Atoll",						// descriptiveName
		"SR-AAM",							// shortName
		"SR_SAM",							// shortName2 (GVs)
		"models/weapons/vwep_sidewinder.md3",// modelName
		0,									// handle
		"models/weapons/vwep_sidewinder.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1300,								// muzzleVelocity
		5000,								// range
		8000,								// fuelrange
		500,								// fire interval
		100,								// damage
		50,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.985f,								// followcone (dot)
		1500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AA_LT,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Archer
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"AA-11 Archer",						// descriptiveName
		"MR-AAM",							// shortName
		"MR_SAM",							// shortName2 (GVs)
		"models/weapons/vwep_amraam.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_amraam.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1800,								// muzzleVelocity
		8000,								// range
		8000,								// fuelrange
		500,								// fire interval
		100,								// damage
		50,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.996f,								// followcone (dot)
		1500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE|CAT_HELO,					// fits on category
		PF_AA_MED,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Alamo
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"AA-10 Alamo",						// descriptiveName
		"MR-AAM",							// shortName
		"MR-SAM",							// shortName2 (GVs)
		"models/weapons/vwep_sparrow.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_sparrow.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		2500,								// muzzleVelocity
		10000,								// range
		8000,								// fuelrange
		500,								// fire interval
		100,								// damage
		50,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.996f,								// followcone (dot)
		1500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AA_HVY,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Hellfire
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"Hellfire",							// descriptiveName
		"AGM",								// shortName
		"SSM",								// shortName2 (GVs)
		"models/weapons/vwep_hellfire.md3",// modelName
		0,									// handle
		"models/weapons/vwep_hellfire.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		1000,								// muzzleVelocity
		4500,								// range
		6500,								// fuelrange
		500,								// fire interval
		110,								// damage
		40,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.996f,								// followcone (dot)
		2000,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
//		{5,180,0},							// max turret angles
//		{-40,-180,0},						// min turret angles
		1,									// number per package
		CAT_HELO|CAT_GROUND|CAT_BOAT,		// fits on category
		PF_AG_MED,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// Maverick
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"Maverick",							// descriptiveName
		"AGM 65",							// shortName
		"SSM",								// shortName2 (GVs)
		"models/weapons/vwep_maverick.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_maverick.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		1000,								// muzzleVelocity
		4500,								// range
		6500,								// fuelrange
		2000,								// fire interval
		150,								// damage
		50,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.5f,								// lockcone (dot)
		0.5f,								// followcone (dot)
		2500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_MED,							// fits on pylon  !! PF_AG_MED&PF_AG_GDA !!
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// HARM
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"HARM",								// descriptiveName
		"HARM",								// shortName
		"DOH",								// shortName2 (GVs)
		"models/weapons/vwep_harm.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_harm.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		3000,								// muzzleVelocity
		5000,								// range
		6000,								// fuelrange
		500,								// fire interval
		500,								// damage
		70,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.996f,								// followcone (dot)
		1700,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_HARM,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},



	// AA SAM
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"SAM",								// descriptiveName
		"SAM",								// shortName
		"THE-SAM",							// shortName2 (GVs)
		"models/weapons/missile_sam.md3",	// modelName
		0,									// handle
		"models/weapons/missile_sam.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1000,								// muzzleVelocity
		4500,								// range
		13000,								// fuelrange
		3200,								// fire interval
		100,								// damage
		100,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0.7f,								// lockcone (dot)
		0.7f,								// followcone (dot)
		1500,								// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODETRACK,				// crosshair track
		CH_MISSILEMODELOCK,					// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AA_MED,							// fits on pylon
		40,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},



	// ECM
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"ECM",								// descriptiveName
		"ECM",								// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_ecm.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_ecm.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		10,									// damage
		10,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		20,								// number per package
		CAT_PLANE,							// fits on category
		PF_JAMMER,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},


	// LASE
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"LASER DESIGNATOR",								// descriptiveName
		"LASE",								// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_ir.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_ir.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		10,									// damage
		10,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		20,								// number per package
		CAT_PLANE,							// fits on category
		PF_JAMMER,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},


	// FLARECHAFF
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"COUNTERMEASURE POD",				// descriptiveName
		"CM POD",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_flarechaff.md3",			// modelName
		0,									// handle
		"models/weapons/vwep_flarechaff.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		10,									// damage
		10,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		20,								// number per package
		CAT_PLANE,							// fits on category
		PF_FLAREPOD,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},


	// droptank
	{ 
		WT_FUELTANK,						// type
		MF_GAMESET_ANY,						// gameset
		"Droptank",							// descriptiveName
		"Droptank",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_droptank.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_droptank.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		30,									// damage
		15,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		30,									// number per package
		CAT_PLANE,							// fits on category
		PF_TANK_HVY,						// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// droptank pair
	{ 
		WT_FUELTANK,						// type
		MF_GAMESET_ANY,						// gameset
		"Droptank",							// descriptiveName
		"Droptank",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_droptank.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_droptank.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		30,									// damage
		15,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		30,									// number per package
		CAT_PLANE,							// fits on category
		PF_TANK_HVY,						// fits on pylon
		0,									// basicECMVulnerability
		WF_FIRE_IN_PAIRS,					// flags
	},

	// droptank lt
	{ 
		WT_FUELTANK,						// type
		MF_GAMESET_ANY,						// gameset
		"Droptank small",					// descriptiveName
		"Droptank",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_droptank_lt.md3",// modelName
		0,									// handle
		"models/weapons/vwep_droptank_lt.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		20,									// damage
		10,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		20,									// number per package
		CAT_PLANE,							// fits on category
		PF_TANK_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// droptank lt pair
	{ 
		WT_FUELTANK,						// type
		MF_GAMESET_ANY,						// gameset
		"Droptank small",					// descriptiveName
		"Droptank",							// shortName
		"BAD SHIT",							// shortName2 (GVs)
		"models/weapons/vwep_droptank_lt.md3",// modelName
		0,									// handle
		"models/weapons/vwep_droptank_lt.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		20,									// damage
		10,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		20,									// number per package
		CAT_PLANE,							// fits on category
		PF_TANK_LT,							// fits on pylon
		0,									// basicECMVulnerability
		WF_FIRE_IN_PAIRS,					// flags
	},

	// droptank lt pair
	{ 
		WT_FUELCRATE,						// type
		MF_GAMESET_ANY,						// gameset
		"Fuel Crate",						// descriptiveName
		"Fuel Crate",							// shortName
		"Fuel Crate",							// shortName2 (GVs)
		"models/weapons/crate.md3",// modelName
		0,									// handle
		"models/weapons/crate.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		45,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		20,									// damage
		10,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_HELO,							// fits on category
		0,									// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Health Crate
	{ 
		WT_HEALTHCRATE,						// type
		MF_GAMESET_ANY,						// gameset
		"Health Crate",						// descriptiveName
		"Health Crate",							// shortName
		"Health Crate",							// shortName2 (GVs)
		"models/weapons/crate.md3",// modelName
		0,									// handle
		"models/weapons/crate.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		45,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		15,									// damage
		10,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_HELO,							// fits on category
		0,									// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Ammo Crate
	{ 
		WT_AMMOCRATE,						// type
		MF_GAMESET_ANY,						// gameset
		"Ammo Crate",						// descriptiveName
		"Ammo Crate",							// shortName
		"Ammo Crate",							// shortName2 (GVs)
		"models/weapons/crate.md3",// modelName
		0,									// handle
		"models/weapons/crate.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		45,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		200,								// fire interval
		15,									// damage
		10,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_GUNMODE,							// crosshair
		CH_GUNMODE,							// crosshair track
		CH_GUNMODE,							// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_HELO,							// fits on category
		0,									// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// flares
	{ 
		WT_FLARE,							// type
		MF_GAMESET_ANY,						// gameset
		"Flares",							// descriptiveName
		"Flares",							// shortName
		"Flares",							// shortName2 (GVs)
		"models/effects/flare.md3",			// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"",									// iconName
		0,									// iconHandle
		0,									// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		2000,								// fuelrange
		200,								// fire interval
		0,									// damage
		0,									// damageRadius
		1,									// spread
		0,									// barrels
		0,									// barrelDistance
		0,									// lockcone (dot)
		0,									// followcone (dot)
		0,									// locktime
		0,									// crosshair
		0,									// crosshair track
		0,									// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_FLAREPOD,						// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Cflares
	{ 
		WT_FLARE,							// type
		MF_GAMESET_ANY,						// gameset
		"CFlares",							// descriptiveName
		"CFlares",							// shortName
		"CFlares",							// shortName2 (GVs)
		"models/weapons/vwep_flarechaff.md3",			// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"",									// iconName
		0,									// iconHandle
		0,									// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		2000,								// fuelrange
		200,								// fire interval
		0,									// damage
		0,									// damageRadius
		1,									// spread
		0,									// barrels
		0,									// barrelDistance
		0,									// lockcone (dot)
		0,									// followcone (dot)
		0,									// locktime
		0,									// crosshair
		0,									// crosshair track
		0,									// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_FLAREPOD,						// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Burning people, can be used as flares
	{ 
		WT_FLARE,							// type
		MF_GAMESET_ANY,						// gameset
		"BurningMan",						// descriptiveName
		"BurningMan",						// shortName
		"BurningMan",						// shortName2 (GVs)
		"models/effects/burningman.md3",	// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"",									// iconName
		0,									// iconHandle
		0,									// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		2000,								// fuelrange
		200,								// fire interval
		0,									// damage
		0,									// damageRadius
		1,									// spread
		0,									// barrels
		0,									// barrelDistance
		0,									// lockcone (dot)
		0,									// followcone (dot)
		0,									// locktime
		0,									// crosshair
		0,									// crosshair track
		0,									// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_ANY,							// fits on category
		PF_FLAREPOD,						// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},
	
	// Nuke
	{ 
		WT_NUKEBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"WW2 relic pos bomb that shouldn't even work anymore",								// descriptiveName
		"NB10MT",							// shortName
		"NB10MT",							// shortName2 (GVs)
		"models/weapons/vwep_nuke.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_nuke.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		5000,								// fire interval
		1000,								// damage
		2000,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Nuke
	{ 
		WT_NUKEBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"WW2 relic pos bomb that shouldn't even work anymore",								// descriptiveName
		"NB5MT",							// shortName
		"NB5MT",							// shortName2 (GVs)
		"models/weapons/vwep_nuke.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_nuke.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		5000,								// fire interval
		1000,								// damage
		800,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Nuke
	{ 
		WT_NUKEBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"WW2 relic pos bomb that shouldn't even work anymore",								// descriptiveName
		"NB1MT",							// shortName
		"NB1MT",							// shortName2 (GVs)
		"models/weapons/vwep_nuke.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_nuke.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		0,									// range
		0,									// fuelrange
		5000,								// fire interval
		1000,								// damage
		300,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Nuke
	{ 
		WT_NUKEMISSILE,						// type
		MF_GAMESET_ANY,						// gameset
		"MinkiCorp brand Nuclear device",								// descriptiveName
		"NM10MT",							// shortName
		"NM10MT",							// shortName2 (GVs)
		"models/weapons/vwep_nuke.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_nuke.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		800,								// muzzleVelocity
		75000,								// range
		75000,								// fuelrange
		5000,								// fire interval
		1000,								// damage
		2000,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Nuke
	{ 
		WT_NUKEMISSILE,						// type
		MF_GAMESET_ANY,						// gameset
		"MinkiCorp brand Nuclear device",								// descriptiveName
		"NM5MT",							// shortName
		"NM5MT",							// shortName2 (GVs)
		"models/weapons/vwep_nukemissile.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_nukemissile.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		800,									// muzzleVelocity
		75000,								// range
		75000,								// fuelrange
		5000,								// fire interval
		1000,								// damage
		800,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Nuke
	{ 
		WT_NUKEMISSILE,						// type
		MF_GAMESET_ANY,						// gameset
		"MinkiCorp brand Nuclear device",								// descriptiveName
		"NM1MT",							// shortName
		"NM1MT",							// shortName2 (GVs)
		"models/weapons/vwep_nukemissile.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_nukemissile.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		800,									// muzzleVelocity
		75000,								// range
		75000,								// fuelrange
		5000,								// fire interval
		1000,								// damage
		300,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_BOMBMODE,						// crosshair
		CH_BOMBMODE,						// crosshair track
		CH_BOMBMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_PLANE,							// fits on category
		PF_AG_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},
	// Flak
	{ 
		WT_FLAK,							// type
		MF_GAMESET_ANY,						// gameset
		"Flak",								// descriptiveName
		"Flak",								// shortName
		"Flak",								// shortName2 (GVs)
		"",		// modelName
		0,									// handle
		"",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		3000,								// muzzleVelocity
		75000,								// range
		75000,								// fuelrange
		5000,								// fire interval
		35,									// damage
		300,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		0,									// lockcone
		0,									// followcone
		0,									// locktime
		CH_MISSILEMODE,						// crosshair
		CH_MISSILEMODE,						// crosshair track
		CH_MISSILEMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_GROUND,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},
	// ****************************************************************************** 


};

int bg_numberOfWeapons = sizeof(availableWeapons) / sizeof(availableWeapons[0]);
