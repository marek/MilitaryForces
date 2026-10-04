/*
 * $Id: bg_loadouts.c,v 1.6 2005-10-28 13:06:54 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"




/*
=================
VehicleData::distributeWeaponsOnPylons

weapon mount info
=================
*/
int QDECL VehicleData::posCompare( const void* a, const void* b )
{
	int p1 = ((mountInfo_t*)a)->pos;
	int p2 = ((mountInfo_t*)b)->pos;
	if( p1 == p2 ) {
		if( ((mountInfo_t*)a)->left ) return -1;
		return 1;
	} else return p1-p2;
}

bool VehicleData::distributeWeaponsOnPylons( int idx, completeLoadout_t* loadout )
{
	char* modelname;
	int num = 0, i, j, k;
	md3Tag_t tags[MAX_MOUNTS_PER_VEHICLE];

	if( idx < 0 ) return false;
	if( availableVehicles[idx].cat & CAT_LQM ) return false;

	modelname = VehicleData::createModelPathname( idx, "models/vehicles/%s/%s/%s.md3" );

//#ifdef _DEBUG
//	Common::printf( "Calculating weapon stations for %s\n", availableVehicles[idx].tinyName );
//#endif

	loadout->mountsOnBay = false;
	num = VehicleData::getTagsContaining(modelname, "tag_P", tags, MAX_MOUNTS_PER_VEHICLE );
	if( !num && (availableVehicles[idx].cat & CAT_HELO) && (availableVehicles[idx].caps & HC_WEAPONBAY) ) {
		modelname = VehicleData::createModelPathname( idx, "models/vehicles/%s/%s/%s_bay.md3" );
		num = VehicleData::getTagsContaining(modelname, "tag_P", tags, MAX_MOUNTS_PER_VEHICLE );
		loadout->mountsOnBay = num > 0;
	}
	bool wideFlags = false;
	if( !num && (availableVehicles[idx].cat & CAT_PLANE) ) {
		num = VehicleData::getTagsContaining(modelname, "PY", tags, MAX_MOUNTS_PER_VEHICLE );
		wideFlags = num > 0;
	}
	if( !num ) return false;

	for( i = 0; i < num; ++i ) {
		memcpy( &loadout->mounts[i].tag, &tags[i], sizeof(md3Tag_t) );
	}

//#ifdef _DEBUG
//	Common::printf( "%s has %d mounts\n", modelname, num );
//#endif

	loadout->usedMounts = num;

	// set up
	for( i = 0; wideFlags && i < num; ++i ) {
		if( strlen( tags[i].name ) < 11 || tags[i].name[0] != 'P' || tags[i].name[1] != 'Y' ) return false;
		int flags = ahextoi( va("0x%c%c%c%c%c%c", tags[i].name[4], tags[i].name[5], tags[i].name[6],
				tags[i].name[7], tags[i].name[8], tags[i].name[9]) );
		loadout->mounts[i].pos = ahextoi( va("0x%c", tags[i].name[2]) );
		loadout->mounts[i].group = ahextoi( va("0x%c", tags[i].name[3]) );
		loadout->mounts[i].flags = flags & 0xBFFF;
		if( flags & PF_AA_MED ) loadout->mounts[i].flags |= PF_AA_LT;
		if( flags & 0x100000 ) loadout->mounts[i].flags |= PF_DONT_DROP;
		loadout->mounts[i].left = tags[i].name[10] == 'L' ? true : false;
	}
	for( i = 0; !wideFlags && i < num; ++i ) {
		if( strlen( tags[i].name ) < 12 ) return false;
		loadout->mounts[i].pos = ahextoi( va("0x%c", tags[i].name[5]) );
		loadout->mounts[i].group = ahextoi( va("0x%c", tags[i].name[6]) );
		loadout->mounts[i].flags = ahextoi( va("0x%c%c%c%c", tags[i].name[7], tags[i].name[8],
				tags[i].name[9], tags[i].name[10]) );
		loadout->mounts[i].left = tags[i].name[11] == 'L' ? true : false;
	}

	// sort
	SortArray( &loadout->mounts[0], num, VehicleData::posCompare );

	// fill
	for( int i : secondaryWeaponSlots ) {
		if( availableVehicles[idx].weapons[i] &&
			(availableWeapons[availableVehicles[idx].weapons[i]].type != WT_MACHINEGUN ||
			 (availableWeapons[availableVehicles[idx].weapons[i]].fitsPylon & PF_GUNPOD)) ) {
			j = availableVehicles[idx].ammo[i];
			for( k = 0; k < num; ++k ) {
				if( !loadout->mounts[k].weapon &&
					(loadout->mounts[k].flags & availableWeapons[availableVehicles[idx].weapons[i]].fitsPylon) ) {
					loadout->mounts[k].weapon = availableVehicles[idx].weapons[i];
					loadout->mounts[k].num = VehicleData::roundsPerMount( availableVehicles[idx].weapons[i] );
					j--;
				}
				if( !j ) break;
			}
		}
	}
//#ifdef _DEBUG
//	for( i = 0; i < num; ++i ) {
//		Common::printf( "Mount %d: %d %x %c %d x %s\n", i, loadout->mounts[i].pos, loadout->mounts[i].flags,
//			(loadout->mounts[i].left ? 'L' : 'R'), loadout->mounts[i].num, 
//			availableWeapons[loadout->mounts[i].weapon].descriptiveName );
//	}
//#endif

	return true;
}

/*
=================
VehicleData::calculateAllDefaultLoadouts

weapon mount info
=================
*/

void VehicleData::calculateAllDefaultLoadouts()
{
	int i;

	for( i = 0; i < bg_numberOfVehicles; ++i ) {
		if( !VehicleData::distributeWeaponsOnPylons( i, &availableLoadouts[i] ) ) {
			memset( &availableLoadouts[i], 0, sizeof(completeLoadout_t) );
		} else {
			availableVehicles[i].renderFlags |= MFR_VWEP;
		}
	}
}

/*
=================
VehicleData::getDefaultLoadoutForVehicle

weapon mount info
=================
*/

void VehicleData::getDefaultLoadoutForVehicle( int idx, completeLoadout_t* loadout, playerState_t* ps )
{
//	char where[10];
	int i;

	if( idx < 0 ) 
		return;

//	Common::printf("Getting default loadout for %s (%s)\n", availableVehicles[idx].descriptiveName, where );
	memcpy( loadout, &availableLoadouts[idx], sizeof(completeLoadout_t) );
	if( ps )
		for( i = 0; i < 32; ++i )
			ps->numWeaponsOnMount[i] = loadout->mounts[i].num;
}


/*
=================
VehicleData::removeWeaponFromLoadout

firing
=================
*/

bool VehicleData::removeWeaponFromLoadout( int weaponIndex, completeLoadout_t* loadout, playerState_t* ps, 
									 bool* wingtip, vec3_t pos, int launchPos )
{
	int i;

	for( i = 0; i < loadout->usedMounts; ++i ) 
	{
		if( loadout->mounts[i].weapon == weaponIndex &&
			loadout->mounts[i].num ) 
		{
			if( launchPos ) {
				launchPos--;
				continue;
			}
			if( !(availableWeapons[loadout->mounts[i].weapon].fitsPylon & PF_DONT_REMOVE) ) 
			{
				loadout->mounts[i].num--;
				if( ps )
					ps->numWeaponsOnMount[i]--;// for now keep them both in parallel
			}
			if( wingtip ) 
				*wingtip = ((loadout->mounts[i].flags & PF_DONT_DROP) != 0);
			if( pos ) 
				VectorCopy( loadout->mounts[i].tag.origin, pos );
			return 
				true;
		}
	}

	return false;
}


/*
=================
VehicleData::addWeaponToLoadout

reloading - all of that type
=================
*/

int VehicleData::addWeaponToLoadout( int weaponIndex, completeLoadout_t* loadout, playerState_t* ps )
{
	int i, added = 0;

	if( availableWeapons[weaponIndex].fitsPylon & PF_DONT_REMOVE ) 
		return added;

	for( i = 0; i < loadout->usedMounts; ++i ) 
	{
		if( loadout->mounts[i].weapon == weaponIndex &&
			loadout->mounts[i].num < VehicleData::roundsPerMount( weaponIndex ) &&
			!(availableWeapons[loadout->mounts[i].weapon].fitsPylon & PF_DONT_REMOVE) ) 
		{
			added += VehicleData::roundsPerMount( weaponIndex ) - loadout->mounts[i].num;
			loadout->mounts[i].num = VehicleData::roundsPerMount( weaponIndex );
			if( ps )
				ps->numWeaponsOnMount[i] = loadout->mounts[i].num;
		}
	}

	return added;
}




/*
=================
VehicleData::findWeaponsOfType

how many of those are loaded ?
=================
*/

int VehicleData::roundsPerMount( int weaponIndex )
{
	if( availableWeapons[weaponIndex].fitsPylon & PF_DONT_REMOVE )
		return 1;
	return availableWeapons[weaponIndex].numberPerPackage > 0 ? availableWeapons[weaponIndex].numberPerPackage : 1;
}

int VehicleData::packagesOfType( int weaponIndex, completeLoadout_t* loadout )
{
	return VehicleData::findWeaponsOfType( weaponIndex, loadout ) / VehicleData::roundsPerMount( weaponIndex );
}

int VehicleData::findWeaponsOfType( int weaponIndex, completeLoadout_t* loadout )
{
	int i, num = 0;

	for( i = 0; i < loadout->usedMounts; ++i ) {
		if( loadout->mounts[i].weapon == weaponIndex ) {
			num += loadout->mounts[i].num;
		}
	}

	return num;
}





