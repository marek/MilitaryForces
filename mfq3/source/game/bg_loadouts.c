/*
 * $Id: bg_loadouts.c,v 1.1 2002-02-25 13:16:54 thebjoern Exp $
*/

#include "q_shared.h"
#include "bg_public.h"






/*
=================
MF_setPylonTag

providing the file and tagname this functions returns the taginfo
=================
*/
void MF_setPylonTag(const char* fileName, const char* tagname, pylonTags_t* pT, unsigned int mount)
{
	md3Tag_t tag;

	if( !fileName || !tagname || !pT ) return;

	if( MF_findTag(fileName, tagname, &tag) ) {
		VectorCopy( tag.origin, pT->pos[mount] );
	} else {
		VectorClear( pT->pos[mount] );
	}
	strcpy( pT->tagname[mount], tagname );
}

/*
=================
MF_distributeWeaponsOnPylons

weapon mount info
=================
*/

qboolean MF_distributeWeaponsOnPylons( int idx, completeLoadout_t* loadout )
{
	int	i, j, k, numPylons = 0, numWeaps = 0;
	int	weaps[MAX_WEAPONS_PER_VEHICLE-2];
	int ammos[MAX_WEAPONS_PER_VEHICLE-2];
	int pyls[MAX_WEAPONS_PER_VEHICLE];
	qboolean done;
	char* modelname = MF_CreateModelPathname( idx, "models/vehicles/%s/%s/%s.md3" );
	char where[10];
	qboolean hasWingtipMounts = qfalse;

	if( idx < 0 ) return qfalse;

	Com_Printf( "Calculating weapon stations for %s\n", availableVehicles[idx].tinyName );

	for( i = 0; i < MAX_WEAPONS_PER_VEHICLE; ++i ) {
		pyls[i] = availableVehicles[idx].pylons[i];
		if( availableVehicles[idx].pylons[i] ) numPylons++;
	}

	if( !numPylons ) {
		Com_Printf( "No weapon stations available for this vehicle!\n" );
		return qfalse;
	}

	Com_Printf( "%d weapon stations available for this vehicle!\n", numPylons );

	for( i = WP_WEAPON1; i < WP_FLARE; ++i ) {
		weaps[i-1] = availableVehicles[idx].weapons[i];
		ammos[i-1] = availableVehicles[idx].ammo[i];
		if( availableVehicles[idx].weapons[i] ) numWeaps++;
	}

	for( i = 0; i < numPylons; ++i ) {
		int mounts = 0;
		char temp[16];
		k = hasWingtipMounts ? i : i+1;
		switch( pyls[i] ) {
		case PT_WINGTIP:
			MF_setPylonTag(modelname, "tag_tip_l", &loadout->tags[i], 0);
			MF_setPylonTag(modelname, "tag_tip_r", &loadout->tags[i], 1);
			mounts = 2;
			hasWingtipMounts = qtrue;
			break;
		case PT_WING_L:
		case PT_BODY_L:
		case PT_WING_M:
		case PT_BODY_M:
			Com_sprintf(temp, 15, "tag_mt%d_l", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 0);
			Com_sprintf(temp, 15, "tag_mt%d_r", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 1);
			mounts = 2;
			break;
		case PT_WING_H:
		case PT_BODY_H:
		case PT_BODY_CENTER_H:
			Com_sprintf(temp, 15, "tag_mt%d_1_l", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 0);
			Com_sprintf(temp, 15, "tag_mt%d_1_r", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 1);
			Com_sprintf(temp, 15, "tag_mt%d_2_l", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 2);
			Com_sprintf(temp, 15, "tag_mt%d_2_r", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 3);
			Com_sprintf(temp, 15, "tag_mt%d_3_l", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 4);
			Com_sprintf(temp, 15, "tag_mt%d_3_r", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 5);
			mounts = 6;
			break;
		case PT_BODY_CENTER_L:
		case PT_BODY_CENTER_M:
		case PT_BAY:
		case PT_BAY_CENTER:
			Com_sprintf(temp, 15, "tag_mt%d_l", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 0);
			Com_sprintf(temp, 15, "tag_mt%d_r", k );
			MF_setPylonTag(modelname, temp, &loadout->tags[i], 1);
			mounts = 1;
			break;
		}
		loadout->type[i] = pyls[i];
		done = qfalse;
		for( j = 0; j < numWeaps; ++j ) {
			if( !ammos[j] ) continue;
			if( availableWeapons[weaps[j]].fitsPylon & pyls[i] ) {
				int num;
				if( mounts * availableWeapons[weaps[j]].numberPerPackage > ammos[j] ) {
					num = ammos[j];
				} else {
					num = mounts * availableWeapons[weaps[j]].numberPerPackage;
				}
				ammos[j] -= num;
				loadout->weaponType[i] = weaps[j];
				loadout->mountedWeapons[i] = loadout->maxWeapons[i] = num / availableWeapons[weaps[j]].numberPerPackage;
				done = qtrue;
			}
			if( done ) break;
		}
	}

	// print out
#ifdef QAGAME
	strcpy( where, "server" );
#else
	strcpy( where, "client" );
#endif
	Com_Printf( "Loadout is (%s):\n", where );
	for( i = 0; i < numPylons; ++i ) {
		Com_Printf( "Pylon %d (type: %d): %d * %s\n", i, loadout->type[i],
				loadout->mountedWeapons[i], availableWeapons[loadout->weaponType[i]].descriptiveName );
		Com_Printf( "Tags are:\n" );
		for( j = 0; j < MAX_MOUNTS_PER_PYLON*2; ++j ) {
			Com_Printf( "%s (%.2f %.2f %.2f)\n", loadout->tags[i].tagname[j], loadout->tags[i].pos[j][0],
				loadout->tags[i].pos[j][1], loadout->tags[i].pos[j][2] );
		}
	}

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
			memcpy( &availableLoadouts[i], &defaultLoadout, sizeof(completeLoadout_t) );
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
MF_getLoadoutFromAmmo

weapon mount info
=================
*/

void MF_getLoadoutFromAmmo( int idx, completeLoadout_t* loadout, unsigned int ammo[8] )
{
	int i, j, diff;
//	char where[10];

	if( idx < 0 ) return;
#ifdef QAGAME
//	strcpy( where, "server" );
#else
//	strcpy( where, "client" );
#endif
//	Com_Printf("Getting loadout from ammo for %s (%s)\n", availableVehicles[idx].descriptiveName, where );
	memcpy( loadout, &availableLoadouts[idx], sizeof(completeLoadout_t) );

	for( i = WP_WEAPON1; i < WP_FLARE; ++i ) {
		diff = availableVehicles[idx].ammo[i] - ammo[i];
		if( diff ) { // less than full loadout
			for( j = 0; j < diff; ++j ) {
				MF_removeWeaponFromLoadout( availableVehicles[idx].weapons[i], loadout, 0, 0, 0 );
			}
		}
	}
}

/*
=================
MF_removeWeaponFromLoadout

firing
=================
*/

qboolean MF_removeWeaponFromLoadout( int weaponIndex, completeLoadout_t* loadout, char* usedTag, 
									vec3_t pos, qboolean nextMount )
{
	int i, num;

	for( i = 0; i < MAX_WEAPONS_PER_VEHICLE; ++i ) {
		if( loadout->weaponType[i] == weaponIndex && loadout->mountedWeapons[i] ) {
			if( availableWeapons[weaponIndex].flags & WF_NON_REMOVABLE_VWEP ) {
				num = loadout->mountedWeapons[i] - (nextMount ? 2 : 1);
				if( num < 0 ) num = 0;
			} else {
				num = --loadout->mountedWeapons[i];
			}
			if( usedTag ) strcpy( usedTag, loadout->tags[i].tagname[num] );
			if( pos ) VectorCopy( loadout->tags[i].pos[num], pos );
			return qtrue;
		}
	}
	return qfalse;
}


/*
=================
MF_addWeaponToLoadout

reloading
=================
*/

int MF_addWeaponToLoadout( int weaponIndex, completeLoadout_t* loadout )
{
	int i, diff, added = 0;

	for( i = 0; i < MAX_WEAPONS_PER_VEHICLE; ++i ) {
		diff = loadout->maxWeapons[i] - loadout->mountedWeapons[i];
		if( loadout->weaponType[i] == weaponIndex && diff ) {
			loadout->mountedWeapons[i] = loadout->maxWeapons[i];
			added += diff;
		}
	}
	return added;
}







