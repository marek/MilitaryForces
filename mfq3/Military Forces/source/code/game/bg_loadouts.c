/*
 * $Id: bg_loadouts.c,v 1.1 2005-08-22 15:41:17 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"




/*
=================
MF_distributeWeaponsOnPylons

weapon mount info
=================
*/
static int QDECL MF_posCompare( const void* a, const void* b )
{
	int p1 = ((mountInfo_t*)a)->pos;
	int p2 = ((mountInfo_t*)b)->pos;
	if( p1 == p2 ) {
		if( ((mountInfo_t*)a)->left ) return -1;
		return 1;
	} else return p1-p2;
}

qboolean MF_distributeWeaponsOnPylons( int idx, completeLoadout_t* loadout )
{
	char* modelname;
	int num = 0, i, j, k;
	md3Tag_t tags[MAX_MOUNTS_PER_VEHICLE];

	if( idx < 0 ) return qfalse;

	modelname = MF_CreateModelPathname( idx, "models/vehicles/%s/%s/%s.md3" );

//#ifdef _DEBUG
//	Com_Printf( "Calculating weapon stations for %s\n", availableVehicles[idx].tinyName );
//#endif

	num = MF_getTagsContaining(modelname, "tag_P", tags, MAX_MOUNTS_PER_VEHICLE );
	if( !num ) return qfalse;

	for( i = 0; i < num; ++i ) {
		memcpy( &loadout->mounts[i].tag, &tags[i], sizeof(md3Tag_t) );
	}

//#ifdef _DEBUG
//	Com_Printf( "%s has %d mounts\n", modelname, num );
//#endif

	loadout->usedMounts = num;

	// set up
	for( i = 0; i < num; ++i ) {
		if( strlen( tags[i].name ) < 12 ) return qfalse;
		loadout->mounts[i].pos = ahextoi( va("0x%c", tags[i].name[5]) );
		loadout->mounts[i].group = ahextoi( va("0x%c", tags[i].name[6]) );
		loadout->mounts[i].flags = ahextoi( va("0x%c%c%c%c", tags[i].name[7], tags[i].name[8],
				tags[i].name[9], tags[i].name[10]) );
		loadout->mounts[i].left = tags[i].name[11] == 'L' ? qtrue : qfalse;
	}

	// sort
	qsort(&loadout->mounts[0], num, sizeof(loadout->mounts[0]), MF_posCompare );

	// fill
	for( i = WP_WEAPON1; i < WP_FLARE; ++i ) {
		if( availableVehicles[idx].weapons[i] &&
			availableWeapons[availableVehicles[idx].weapons[i]].type != WT_MACHINEGUN ) {
			j = availableVehicles[idx].ammo[i];
			for( k = 0; k < num; ++k ) {
				if( !loadout->mounts[k].weapon &&
					(loadout->mounts[k].flags & availableWeapons[availableVehicles[idx].weapons[i]].fitsPylon) ) {
					loadout->mounts[k].weapon = availableVehicles[idx].weapons[i];
					loadout->mounts[k].num = 1;
					j--;
				}
				if( !j ) break;
			}
		}
	}
//#ifdef _DEBUG
//	for( i = 0; i < num; ++i ) {
//		Com_Printf( "Mount %d: %d %x %c %d x %s\n", i, loadout->mounts[i].pos, loadout->mounts[i].flags,
//			(loadout->mounts[i].left ? 'L' : 'R'), loadout->mounts[i].num, 
//			availableWeapons[loadout->mounts[i].weapon].descriptiveName );
//	}
//#endif

	return qtrue;
}

/*
=================
MF_calculateAllDefaultLoadouts

weapon mount info
=================
*/

void MF_calculateAllDefaultLoadouts()
{
	int i;

	for( i = 0; i < bg_numberOfVehicles; ++i ) {
		if( !MF_distributeWeaponsOnPylons( i, &availableLoadouts[i] ) ) {
			memset( &availableLoadouts[i], 0, sizeof(completeLoadout_t) );
		} else {
			availableVehicles[i].renderFlags |= MFR_VWEP;
		}
	}
}

/*
=================
MF_getDefaultLoadoutForVehicle

weapon mount info
=================
*/

void MF_getDefaultLoadoutForVehicle( int idx, completeLoadout_t* loadout )
{
//	char where[10];
	if( idx < 0 ) return;
#ifdef QAGAME
//	strcpy( where, "server" );
#else
//	strcpy( where, "client" );
#endif
//	Com_Printf("Getting default loadout for %s (%s)\n", availableVehicles[idx].descriptiveName, where );
	memcpy( loadout, &availableLoadouts[idx], sizeof(completeLoadout_t) );
}


/*
=================
MF_removeWeaponFromLoadout

firing
=================
*/

qboolean MF_removeWeaponFromLoadout( int weaponIndex, completeLoadout_t* loadout, qboolean* wingtip, 
									vec3_t pos, int launchPos )
{
	int i;

	for( i = 0; i < loadout->usedMounts; ++i ) {
		if( loadout->mounts[i].weapon == weaponIndex &&
			loadout->mounts[i].num ) {
			if( launchPos ) {
				launchPos--;
				continue;
			}
			if( !(availableWeapons[loadout->mounts[i].weapon].fitsPylon & PF_DONT_REMOVE) ) {
				loadout->mounts[i].num--;
			}
			if( wingtip ) {
				*wingtip = (loadout->mounts[i].flags & PF_DONT_DROP);
			}
			if( pos ) VectorCopy( loadout->mounts[i].tag.origin, pos );
			return qtrue;
		}
	}

	return qfalse;
}


/*
=================
MF_addWeaponToLoadout

reloading - all of that type
=================
*/

int MF_addWeaponToLoadout( int weaponIndex, completeLoadout_t* loadout )
{
	int i, added = 0;

	if( availableWeapons[weaponIndex].fitsPylon & PF_DONT_REMOVE ) return added;

	for( i = 0; i < loadout->usedMounts; ++i ) {
		if( loadout->mounts[i].weapon == weaponIndex &&
			!loadout->mounts[i].num &&
			!(availableWeapons[loadout->mounts[i].weapon].fitsPylon & PF_DONT_REMOVE) ) {
			loadout->mounts[i].num++;
			added++;
		}
	}

	return added;
}




/*
=================
MF_findWeaponsOfType

how many of those are loaded ?
=================
*/

int MF_findWeaponsOfType( int weaponIndex, completeLoadout_t* loadout )
{
	int i, num = 0;

	for( i = 0; i < loadout->usedMounts; ++i ) {
		if( loadout->mounts[i].weapon == weaponIndex ) {
			num += loadout->mounts[i].num;
		}
	}

	return num;
}





