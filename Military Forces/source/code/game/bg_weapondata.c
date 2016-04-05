/*
 * $Id: bg_weapondata.c,v 1.5 2016-04-04 osfpsproject Exp $
*/

#include "q_shared.h"
#include "bg_public.h"

// just contains data of all the available weapons
completeWeaponData_t availableWeapons[] = 
{

	// 1x 5,45 mm assault rifle
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 5,45 mm assault rifle",			// descriptiveName
		"ak74",								// shortName
		"ak74",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		175,								// fire interval
		4,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
		CAT_LQM,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// 1x 5,56 mm assault rifle
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 5,56 mm assault rifle",			// descriptiveName
		"M4",								// shortName
		"M4",								// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		175,								// fire interval
		5,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
		CAT_LQM,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},
	
	// 1x 7,62 mm rifle
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 7,62 mm rifle",					// descriptiveName
		"M14/SKS/M40a5/dragunov",			// shortName
		"M14/SKS/M40a5/dragunov",			// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		600,								// fire interval
		10,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
		CAT_LQM,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// 1x 7,62 mm machine gun
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 7,62 mm machine gun",			// descriptiveName
		"M240/PKT",							// shortName
		"M240/PKT",							// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		150,								// fire interval
		10,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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

	// 1x 12,7 mm machine gun
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 12,7 mm machine gun",			// descriptiveName
		"M2 Browning (.50)",				// shortName
		"M2 Browning (.50)",				// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		150,								// fire interval
		12,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 1x 14,5 mm machine gun
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 14,5 mm machine gun",			// descriptiveName
		"KPVT",								// shortName
		"KPVT",								// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		150,								// fire interval
		14,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 2x 7,62 mm machine gun
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"2x 7,62 mm machine gun",			// descriptiveName
		"double M1919 Browning (.30-06)",	// shortName
		"double M1919 Browning (.30-06)",	// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		150,								// fire interval
		20,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
		
	// 2x 7,92 mm machine gun
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"2x 7,92 mm machine gun",			// descriptiveName
		"double MG 17",						// shortName
		"double MG 17",						// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		150,								// fire interval
		22,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 2x 13 mm machine gun
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"2x 13 mm machine gun",				// descriptiveName
		"double MG 131 (.51)",				// shortName
		"double MG 131 (.51)",				// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		150,								// fire interval
		26,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 4x 12,7 mm machine gun
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"4x 12,7 mm machine gun",			// descriptiveName
		"quadruple M2 Browning (.50)",		// shortName
		"quadruple M2 Browning (.50)",		// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		150,								// fire interval
		48,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 8x 7,62 mm machine gun
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"8x 7,62 mm machine gun",			// descriptiveName
		"octuple M1919 Browning (.30-06)",	// shortName
		"octuple M1919 Browning (.30-06)",	// shortName2 (GVs)
		"models/weapons/m4a1.md3",			// modelName
		0,									// handle
		"models/weapons/m4a1.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		150,								// fire interval
		80,									// damage
		0,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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

	// 1x 20 mm autocannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 20 mm autocannon",				// descriptiveName
		"M61 Vulcan/Colt Mk12",				// shortName
		"M61 Vulcan/Colt Mk12",				// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_acn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		600,								// fire interval
		2000,								// damage
		5,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 1x 25 mm autocannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 25 mm autocannon",				// descriptiveName
		"ADEN 25",							// shortName
		"ADEN 25",							// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_acn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		600,								// fire interval
		2200,								// damage
		5,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 1x 27 mm autocannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 27 mm autocannon",				// descriptiveName
		"Mauser BK 27",						// shortName
		"Mauser BK 27",						// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_acn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		600,								// fire interval
		2300,								// damage
		5,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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

	// 1x 30 mm autocannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 30 mm autocannon",				// descriptiveName
		"GAU-8 Avenger/GSh-301/2A-72",		// shortName
		"GAU-8 Avenger/GSh-301/2A-72",		// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_acn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		600,								// fire interval
		2300,								// damage
		5,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 2x 20 mm autocannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"2x 20 mm autocannon",				// descriptiveName
		"double M39A2",						// shortName
		"double M39A2",						// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_acn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		600,								// fire interval
		4000,								// damage
		5,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 2x 23 mm autocannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"2x 23 mm autocannon",				// descriptiveName
		"double GSh-23",					// shortName
		"double GSh-23",					// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_acn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		600,								// fire interval
		4200,								// damage
		5,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 4x 23 mm autocannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"4x 23 mm autocannon",				// descriptiveName
		"AZP-23 Amur",						// shortName
		"AZP-23 Amur",						// shortName2 (GVs)
		"",									// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_acn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		600,								// fire interval
		8400,								// damage
		5,									// damageRadius
		1,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
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
	
	// 1x 50 mm cannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 50 mm cannon",					// descriptiveName
		"KwK 39 L/60",						// shortName
		"KwK 39 L/60",						// shortName2 (GVs)
		"models/weapons/vwep_rocket.md3",	// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_cnn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		1500,								// fire interval
		2000,								// damage
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

	// 1x 75 mm cannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 75 mm cannon",					// descriptiveName
		"M3 L/40",							// shortName
		"M3 L/40",							// shortName2 (GVs)
		"models/weapons/vwep_rocket.md3",	// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_cnn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		1500,								// fire interval
		3000,								// damage
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
	
	// 1x 100 mm cannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 100 mm cannon",					// descriptiveName
		"2A70",								// shortName
		"2A70",								// shortName2 (GVs)
		"models/weapons/vwep_rocket.md3",	// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_cnn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		1500,								// fire interval
		4000,								// damage
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
	
	// 1x 120 mm cannon
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 120 mm cannon",					// descriptiveName
		"L/44 M256A1",						// shortName
		"L/44 M256A1",						// shortName2 (GVs)
		"models/weapons/vwep_rocket.md3",	// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_cnn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		1500,								// fire interval
		4800,								// damage
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
	
	// 1x 203 mm howitzer
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"1x 203 mm howitzer",				// descriptiveName
		"M201A1 howitzer",					// shortName
		"M201A1 howitzer",					// shortName2 (GVs)
		"models/weapons/vwep_rocket.md3",	// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"hud/iconw_cnn.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		2000,								// muzzleVelocity
		1500,								// fire interval
		8000,								// damage
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

	// 1x AA missile (homing, 7kg warhead)
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"1x AA missile (7kg warhead)",		// descriptiveName
		"R73",								// shortName
		"R73",								// shortName2 (GVs)
		"models/weapons/vwep_ffar_lt.md3",// modelName
		0,									// handle
		"models/weapons/vwep_ffar_lt.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		14000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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
		PF_AA_LT,							// fits on pylon
		30,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},
	
	// 1x AA missile (homing, 11kg warhead)
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"1x AA missile (11kg warhead)",		// descriptiveName
		"AIM-9 Sidewinder",					// shortName
		"AIM-9 Sidewinder",					// shortName2 (GVs)
		"models/weapons/vwep_sidewinder.md3",// modelName
		0,									// handle
		"models/weapons/vwep_sidewinder.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		22000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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
		PF_AA_LT,							// fits on pylon
		30,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// 1x AA missile (homing, 23kg warhead)
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"1x AA missile (23kg warhead)",		// descriptiveName
		"AIM-120 AMRAAM",					// shortName
		"AIM-120 AMRAAM",					// shortName2 (GVs)
		"models/weapons/vwep_amraam.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_amraam.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		46000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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
		PF_AA_MED,							// fits on pylon
		30,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},
	
	// 1x AA missile (homing, 39kg warhead)
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"1x AA missile (39kg warhead)",		// descriptiveName
		"R27",								// shortName
		"R27",								// shortName2 (GVs)
		"models/weapons/vwep_nukemissile.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_nukemissile.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		78000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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
		PF_AA_MED,							// fits on pylon
		30,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},
	
	// 1x AA missile (homing, 40kg warhead)
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"1x AA missile (40kg warhead)",		// descriptiveName
		"AIM-7 Sparrow",					// shortName
		"AIM-7 Sparrow",					// shortName2 (GVs)
		"models/weapons/vwep_sparrow.md3",// modelName
		0,									// handle
		"models/weapons/vwep_sparrow.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		80000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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
		PF_AA_MED,							// fits on pylon
		30,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},
	
	// 1x AA missile (homing, 60kg warhead)
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"1x AA missile (60kg warhead)",		// descriptiveName
		"AIM-54 Phoenix",					// shortName
		"AIM-54 Phoenix",					// shortName2 (GVs)
		"models/weapons/vwep_phoenix.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_phoenix.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_PLANE|CAT_HELO,					// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		120000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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
		PF_AA_PHX,							// fits on pylon
		30,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},
	
	// 1x SA missile (3kg warhead)
	{ 
		WT_ANTIAIRMISSILE,					// type
		MF_GAMESET_ANY,						// gameset
		"1x SA missile (3kg warhead)",		// descriptiveName
		"FIM-92 Stinger",					// shortName
		"FIM-92 Stinger",					// shortName2 (GVs)
		"models/weapons/vwep_stinger.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_stinger.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		6000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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

	// 1x AS missile (8kg warhead)
	{ 
		WT_ROCKET,							// type
		MF_GAMESET_ANY,						// gameset
		"1x SA missile (8kg warhead)",		// descriptiveName
		"Hydra 70 FFAR",					// shortName
		"Hydra 70 FFAR",					// shortName2 (GVs)
		"models/weapons/vwep_ffar.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_ffar.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		16000,								// damage
		5,									// damageRadius
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
		7,									// number per package
		CAT_PLANE|CAT_HELO,					// fits on category
		PF_FFAR,							// fits on pylon
		0,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// 1x AS missile (9kg warhead)
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"1x AS missile (9kg warhead)",		// descriptiveName
		"AGM-114 Hellfire",					// shortName
		"AGM-114 Hellfire",					// shortName2 (GVs)
		"models/weapons/vwep_hellfire.md3",// modelName
		0,									// handle
		"models/weapons/vwep_hellfire.md3",// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		18000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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
		CAT_HELO,							// fits on category
		PF_AG_MED,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// 1x AS missile (57kg warhead)
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"1x AS missile (57kg warhead)",		// descriptiveName
		"AGM-65 Maverick",					// shortName
		"AGM-65 Maverick",					// shortName2 (GVs)
		"models/weapons/vwep_maverick.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_maverick.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		114000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
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
		PF_HARM,							// fits on pylon
		25,									// basicECMVulnerability
		WF_HAS_FIRE_FRAME,					// flags
	},

	// 1x AS missile (66kg warhead)
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"1x AS missile (66kg warhead)",		// descriptiveName
		"AGM-88 HARM",						// shortName
		"AGM-88 HARM",						// shortName2 (GVs)
		"models/weapons/vwep_harm.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_harm.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		132000,								// damage
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

	// 1x SS missile (1kg warhead)
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"1x SS missile (1kg warhead)",		// descriptiveName
		"M72A2HEAT",						// shortName
		"M72A2HEAT",						// shortName2 (GVs)
		"models/weapons/vwep_rocket_hv.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_rocket_hv.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		750,								// fire interval
		2000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.996f,								// followcone (dot)
		1700,								// locktime
		CH_ROCKETMODE,						// crosshair
		CH_ROCKETMODE,						// crosshair track
		CH_ROCKETMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_LQM,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

		// 1x SS missile (1kg warhead)
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"1x SS missile (1kg warhead)",		// descriptiveName
		"PG-7VL",							// shortName
		"PG-7VL",							// shortName2 (GVs)
		"models/weapons/vwep_rocket_hv.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_rocket_hv.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		750,								// fire interval
		2000,								// damage
		5,									// damageRadius
		1,									// spread
		1,									// barrels
		0,									// barrelDistance
		0.985f,								// lockcone (dot)
		0.996f,								// followcone (dot)
		1700,								// locktime
		CH_ROCKETMODE,						// crosshair
		CH_ROCKETMODE,						// crosshair track
		CH_ROCKETMODE,						// crosshair lock
		0,									// turret
		{0,0,0},							// max turret angles
		{0,0,0},							// min turret angles
		1,									// number per package
		CAT_LQM,							// fits on category
		PF_NA,								// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},
	
	// 1x SS missile (90kg warhead)
	{ 
		WT_ANTIGROUNDMISSILE,				// type
		MF_GAMESET_ANY,						// gameset
		"1x SS missile (90kg warhead)",		// descriptiveName
		"M26A2",							// shortName
		"M26A2",							// shortName2 (GVs)
		"models/weapons/missile_sam.md3",	// modelName
		0,									// handle
		"models/weapons/missile_sam.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0.3f,								// non cat mod
		1500,								// muzzleVelocity
		500,								// fire interval
		180000,								// damage
		60,									// damageRadius
		1,									// spread
		1,									// barrels
		0,									// barrelDistance
		0.7f,								// lockcone (dot)
		0.7f,								// followcone (dot)
		1500,								// locktime
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
	
	// 1x 227kg bomb (unguided)
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 227kg bomb (unguided)",			// descriptiveName
		"Mark 82",							// shortName
		"Mark 82",							// shortName2 (GVs)
		"models/weapons/vwep_500.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_500.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		174000,								// damage
		107,								// damageRadius
		1,									// spread
		1,									// barrels
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

	// 1x 460kg bomb (unguided)
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 460kg bomb(unguided)",			// descriptiveName
		"Mark 83",							// shortName
		"Mark 83",							// shortName2 (GVs)
		"models/weapons/vwep_1000.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_1000.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		404000,								// damage
		357,								// damageRadius
		1,									// spread
		1,									// barrels
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

	// 1x 940kg bomb (unguided)
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"1x 940kg bomb (unguided)",			// descriptiveName
		"Mark 84",							// shortName
		"Mark 84",							// shortName2 (GVs)
		"models/weapons/vwep_2000.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_2000.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		858000,								// damage
		635,								// damageRadius
		1,									// spread
		1,									// barrels
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

	// 10x incendiary cluster bomb
	{ 
		WT_FLARE,							// type
		MF_GAMESET_ANY,						// gameset
		"10x incendiary cluster bomb",		// descriptiveName
		"AN-M50",							// shortName
		"AN-M50",							// shortName2 (GVs)
		"models/effects/burningman.md3",	// modelName
		0,									// handle
		"models/effects/burningman.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		10000,								// damage
		700,								// damageRadius
		10,									// spread
		10,									// barrels
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
		CAT_ANY,							// fits on category
		PF_FLAREPOD,						// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// 1x B82 nuclear bomb
	{ 
		WT_NUKEBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"1x B82 nuclear bomb",				// descriptiveName
		"B82",								// shortName
		"B82",								// shortName2 (GVs)
		"models/weapons/vwep_nuke.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_nuke.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		1600000000,							// damage
		535710,								// damageRadius
		1,									// spread
		1,									// barrels
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
	
	// 1x GBU-15 (guided)
	{ 
		WT_GUIDEDBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"1x GBU-15 (guided)",				// descriptiveName
		"GBU-15",							// shortName
		"GBU-15",							// shortName2 (GVs)
		"models/weapons/vwep_gbu15.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_gbu15.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		1820000,							// damage
		120,								// damageRadius
		1,									// spread
		1,									// barrels
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

	// 1x GBU-31 (guided)
	{ 
		WT_GUIDEDBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"1x GBU-31 (guided)",				// descriptiveName
		"GBU-31",							// shortName
		"GBU-31",							// shortName2 (GVs)
		"models/weapons/vwep_gbu15.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_gbu15.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		858000,								// damage
		635,								// damageRadius
		1,									// spread
		1,									// barrels
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
	
	// 1x BLU-107 Durandal (guided)
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"1x BLU-107 Durandal (guided)",		// descriptiveName
		"BLU-107 Durandal",					// shortName
		"BLU-107 Durandal",					// shortName2 (GVs)
		"models/weapons/vwep_durandal.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_durandal.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		CAT_GROUND,							// cat
		0.3f,								// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		230000,								// damage
		141,								// damageRadius
		1,									// spread
		1,									// barrels
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

	// Droptank lt
	{ 
		WT_FUELTANK,						// type
		MF_GAMESET_ANY,						// gameset
		"Droptank, lt",					// descriptiveName
		"Droptank, lite",					// shortName
		"Droptank, lite",					// shortName2 (GVs)
		"models/weapons/vwep_droptank_lt.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_droptank_lt.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		20,									// damage
		0,									// damageRadius
		1,									// spread
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
		20,									// number per package
		CAT_PLANE,							// fits on category
		PF_TANK_LT,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Droptank pair
	{ 
		WT_FUELCRATE,						// type
		MF_GAMESET_ANY,						// gameset
		"Droptank, pair",						// descriptiveName
		"Droptank, pair",					// shortName
		"Droptank, pair",					// shortName2 (GVs)
		"models/weapons/vwep_droptank.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_droptank.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
		20,									// damage
		0,									// damageRadius
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

	// Health crate
	{ 
		WT_HEALTHCRATE,						// type
		MF_GAMESET_ANY,						// gameset
		"Health crate",						// descriptiveName
		"Health crate",						// shortName
		"Health crate",						// shortName2 (GVs)
		"models/weapons/crate.md3",			// modelName
		0,									// handle
		"models/weapons/crate.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		45,									// muzzleVelocity
		200,								// fire interval
		20,									// damage
		0,									// damageRadius
		1,									// spread
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
		CAT_HELO,							// fits on category
		0,									// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Ammo crate
	{ 
		WT_AMMOCRATE,						// type
		MF_GAMESET_ANY,						// gameset
		"Ammo crate",						// descriptiveName
		"Ammo crate",						// shortName
		"Ammo crate",						// shortName2 (GVs)
		"models/weapons/crate.md3",			// modelName
		0,									// handle
		"models/weapons/crate.md3",			// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		45,									// muzzleVelocity
		200,								// fire interval
		20,									// damage
		0,									// damageRadius
		1,									// spread
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
		CAT_HELO,							// fits on category
		0,									// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// Flare
	{ 
		WT_FLARE,							// type
		MF_GAMESET_ANY,						// gameset
		"Flare",							// descriptiveName
		"Flare",							// shortName
		"Flare",							// shortName2 (GVs)
		"models/effects/flare.md3",			// modelName
		0,									// handle
		"",									// vwepName
		0,									// vwephandle
		"",									// iconName
		0,									// iconHandle
		0,									// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		500,								// fire interval
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

	// Flarechaff
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"Flarechaff",						// descriptiveName
		"Flarechaff",						// shortName
		"Flarechaff",						// shortName2 (GVs)
		"models/weapons/vwep_flarechaff.md3",	// modelName
		0,									// handle
		"models/weapons/vwep_flarechaff.md3",	// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		100,								// fire interval
		0,									// damage
		0,									// damageRadius
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
		20,									// number per package
		CAT_PLANE,							// fits on category
		PF_FLAREPOD,						// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},
	
	// 1x Electronic countermeasures pod
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"1x Electronic countermeasures pod",	// descriptiveName
		"ECM pod",								// shortName
		"ECM pod",							// shortName2 (GVs)
		"models/weapons/vwep_ecm.md3",		// modelName
		0,									// handle
		"models/weapons/vwep_ecm.md3",		// vwepName
		0,									// vwephandle
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		CAT_ANY,							// cat
		0,									// non cat mod
		0,									// muzzleVelocity
		200,								// fire interval
		0,									// damage
		100,								// damageRadius
		1,									// spread
		1,									// barrels
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
		20,									// number per package
		CAT_PLANE,							// fits on category
		PF_JAMMER,							// fits on pylon
		0,									// basicECMVulnerability
		0,									// flags
	},

	// ****************************************************************************** 

};

int bg_numberOfWeapons = sizeof(availableWeapons) / sizeof(availableWeapons[0]);
