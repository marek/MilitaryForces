/*
 * $Id: bg_weapondata.c,v 1.1 2001-11-15 21:35:14 thebjoern Exp $
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
		"",									// modelName
		"",									// iconName
		0,									// iconHandle
		0,									// muzzleVelocity
		0,									// fire interval
		0,									// damage
		0,									// damageRadius
		0,									// spread
		0,									// barrels
		0,									// barrelDistance
		0									// crosshair
	},

	// machinegun: WW2, cal 50, 8 barrels
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_WW2,						// gameset
		"8x cal.50 MG",						// descriptiveName
		"",									// modelName
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		3000,								// muzzleVelocity
		150,								// fire interval
		18,									// damage
		0,									// damageRadius
		8,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		8,									// barrels
		0,									// barrelDistance
		CH_GUNMODE							// crosshair
	},

	// machinegun: WW2, cal 50, 6 barrels
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_WW2,						// gameset
		"6x cal.50 MG",						// descriptiveName
		"",									// modelName
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		3000,								// muzzleVelocity
		150,								// fire interval
		18,									// damage
		0,									// damageRadius
		8,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		6,									// barrels
		0,									// barrelDistance
		CH_GUNMODE							// crosshair
	},

	// machinegun: any set, 12.7mm, 1 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"12.7mm MG",						// descriptiveName
		"",									// modelName
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		3000,								// muzzleVelocity
		100,								// fire interval
		12,									// damage
		0,									// damageRadius
		14,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		1,									// barrels
		0,									// barrelDistance
		CH_GUNMODE							// crosshair
	},

	// machinegun: any set, 14.5mm, 1 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_ANY,						// gameset
		"Dual 14.5mm MG",					// descriptiveName
		"",									// modelName
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		3000,								// muzzleVelocity
		120,								// fire interval
		14,									// damage
		0,									// damageRadius
		14,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		2,									// barrels
		0,									// barrelDistance
		CH_GUNMODE							// crosshair
	},

	// machinegun: modern, 20mm, 1 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"20mm MG",							// descriptiveName
		"",									// modelName
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		3000,								// muzzleVelocity
		100,								// fire interval
		20,									// damage
		0,									// damageRadius
		14,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		1,									// barrels
		0,									// barrelDistance
		CH_GUNMODE							// crosshair
	},

	// machinegun: modern, 20mm, 2 barrel
	{ 
		WT_MACHINEGUN,						// type
		MF_GAMESET_MODERN,					// gameset
		"Dual 20mm MG",						// descriptiveName
		"",									// modelName
		"hud/iconw_mg.tga",					// iconName
		0,									// iconHandle
		3000,								// muzzleVelocity
		100,								// fire interval
		20,									// damage
		0,									// damageRadius
		10,									// spread (in 0.1 deg; ie 60 means +- 3 degrees)
		2,									// barrels
		0,									// barrelDistance
		CH_GUNMODE							// crosshair
	},

	// FFAR
	{ 
		WT_ROCKET,							// type
		MF_GAMESET_ANY,						// gameset
		"FFAR",								// descriptiveName
		"models/weapons/ffar.md3",			// modelName
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		1500,								// muzzleVelocity
		160,								// fire interval
		40,									// damage
		50,									// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		CH_ROCKETMODE						// crosshair
	},

	// tank gun
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"100mm Gun",						// descriptiveName
		"models/weapons/ffar.md3",			// modelName
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		2000,								// muzzleVelocity
		1500,								// fire interval
		140,								// damage
		80,									// damageRadius
		1,									// spread
		1,									// barrels
		0,									// barrelDistance
		CH_ROCKETMODE						// crosshair
	},

	// tank gun
	{ 
		WT_BALLISTICGUN,					// type
		MF_GAMESET_ANY,						// gameset
		"125mm Gun",						// descriptiveName
		"models/weapons/ffar.md3",			// modelName
		"hud/iconw_rkt.tga",				// iconName
		0,									// iconHandle
		2500,								// muzzleVelocity
		1500,								// fire interval
		200,								// damage
		100,								// damageRadius
		1,									// spread
		1,									// barrels
		0,									// barrelDistance
		CH_ROCKETMODE						// crosshair
	},

	// iron bombs
	{ 
		WT_IRONBOMB,						// type
		MF_GAMESET_ANY,						// gameset
		"Mk82",								// descriptiveName
		"models/weapons/ag_800_d.md3",		// modelName
		"hud/iconw_bmb.tga",				// iconName
		0,									// iconHandle
		0,									// muzzleVelocity
		150,								// fire interval
		200,								// damage
		120,								// damageRadius
		1,									// spread
		2,									// barrels
		0,									// barrelDistance
		CH_BOMBMODE							// crosshair
	},

	// flares
	{ 
		WT_FLARE,							// type
		MF_GAMESET_ANY,						// gameset
		"Flare",							// descriptiveName
		"models/weapons/ffar.md3",			// modelName
		"",									// iconName
		0,									// iconHandle
		0,									// muzzleVelocity
		100,								// fire interval
		0,									// damage
		0,									// damageRadius
		1,									// spread
		0,									// barrels
		0,									// barrelDistance
		0									// crosshair
	}
};

int bg_numberOfWeapons = sizeof(availableWeapons) / sizeof(availableWeapons[0]);
