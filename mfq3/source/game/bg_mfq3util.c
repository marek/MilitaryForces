/*
 * $Id: bg_mfq3util.c,v 1.18 2002-02-23 19:31:55 thebjoern Exp $
*/

#include "q_shared.h"
#include "bg_public.h"

// externals
extern void	trap_Cvar_Set( const char *var_name, const char *value );
extern void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

/*
=================
MF_getIndexOfVehicleEx
=================
*/

int MF_getIndexOfVehicleEx( int start,
							int gameset,
							int team,
							int cat,
							int cls )
{
	// NOTE: vehicleClass & vehicleCat are int enum indexed, convert to flag enums

	// CATAGORY
	if( cat >= 0 )
	{
		// specific catagory
		cat = 1 << cat;		// (convert from int enum to flag enum)
	}

	// CLASS
	if( cls >= 0 )
	{
		// specific class
		cls = 1 << cls;		// (convert from int enum to flag enum)
	}

	return MF_getIndexOfVehicle( start, gameset, team, cat, cls );
}

/*
=================
MF_getIndexOfVehicle
=================
*/
int MF_getIndexOfVehicle( int start,			// where to start in list
						  int gameset,
						  int team,
						  int cat,
						  int cls )
{
    int				i;
	qboolean		done = qfalse;

	// null or negative? use ALL values
	if( gameset <= 0 ) gameset = MF_GAMESET_ANY;
	if( team <= 0 ) team = MF_TEAM_ANY;
	if( cat <= 0 ) cat = CAT_ANY;
	if( cls <= 0 ) cls = CLASS_ANY;

	if( start >= bg_numberOfVehicles )
	{
		start = 0;
	}
	else if( start < 0 )
	{
		start = bg_numberOfVehicles;
	}

	// scan loop
    for( i = start+1; done != qtrue; i++ )
    {
		if( i >= bg_numberOfVehicles )
		{
			if( start == bg_numberOfVehicles && i == start )
			{
				return -1;
			}
			
			i = 0;					
		}

		if( i == start ) done = qtrue;//return start;	

//		Com_Printf( "Vehicle is %s %x\n", availableVehicles[i].descriptiveName,
//										  availableVehicles[i].id );

		if( !(availableVehicles[i].gameset & gameset) ) continue;	// wrong set
		if( !(availableVehicles[i].team & team) ) continue;			// wrong team
		if( !(availableVehicles[i].cat & cat) ) continue;			// wrong category
		if( !(availableVehicles[i].cls & cls) ) continue;			// wrong class

		return i;
    }

    return -1;
}

/*
int MF_getIndexOfVehicle( int start,			// where to start in list
						  unsigned long what)	// what (team|cat|cls)
{
    int				i;
	qboolean		done = qfalse;
	unsigned long	set  = (what & 0xFF000000),
					team = (what & 0x00FF0000),
					cat  = (what & 0x0000FF00),
					cls  = (what & 0x000000FF);

	if( !set ) set = MF_GAMESET_ANY;
	if( !team ) team = MF_TEAM_ANY;
	if( !cat ) cat = CAT_ANY;
	if( !cls ) cls = CLASS_ANY;

	if( start >= bg_numberOfVehicles ) start = 0;
	else if( start < 0 ) start = bg_numberOfVehicles;

    for( i = start+1; done != qtrue; i++ )
    {
		if( i >= bg_numberOfVehicles ) {
			if( start == bg_numberOfVehicles && i == start ) return -1;
			i = 0;					
		}
		if( i == start ) done = qtrue;//return start;	
//		Com_Printf( "Vehicle is %s %x\n", availableVehicles[i].descriptiveName,
//										  availableVehicles[i].id );
		if( !(availableVehicles[i].id & set) ) continue;		// wrong set
		if( !(availableVehicles[i].id & team) ) continue;		// wrong team
		if( !(availableVehicles[i].id & cat) ) continue;		// wrong category
		if( !(availableVehicles[i].id & cls) ) continue;		// wrong category
		return i;
    }
    return -1;

}
*/
/*
=================
MF_getNumberOfItems
=================
*/

// returns the number of items in a stringlist
int MF_getNumberOfItems(const char **itemlist)
{
	int i;
	for( i = 0; itemlist[i] != 0; i++ );
	return i;
}

/*
=================
MF_getItemIndexFromHex
=================
*/

// takes an already properly right-shifted hex value (with only 1 bit set!)
// and returns the item index for it
int MF_getItemIndexFromHex(int hexValue)
{
	int i;
	for( i = -1; hexValue; i++ ) hexValue>>=1;
	return i;
}

/*
=================
MF_CreateModelPathname
=================
*/

// creates a model pathname (e.g. pFormatString = "models/vehicles/%s/%s/%s_icon")
char * MF_CreateModelPathname( int vehicle, char * pFormatString )
{
	char * pReturnString = NULL;
	char catDir[ 32 ];
	unsigned int cat = 0;

	// find catagory
	cat = availableVehicles[ vehicle ].cat;
	if( cat & CAT_PLANE ) {
		strcpy( catDir, "planes" );
	}
	else if( cat & CAT_GROUND ) {
		strcpy( catDir, "ground" );
	}
	else if( cat & CAT_HELO ) {
		strcpy( catDir, "helos" );
	}
	else if( cat & CAT_LQM ) {
		strcpy( catDir, "lqms" );
	}
	else if( cat & CAT_BOAT ) {
		strcpy( catDir, "sea" );
	}

	// create filename string
	pReturnString = va( pFormatString, catDir, availableVehicles[ vehicle ].modelName, availableVehicles[ vehicle ].modelName );

	return pReturnString;
}

/*
=================
MF_LimitFloat
=================
*/

// generic float limiting
void MF_LimitFloat( float * value, float min, float max )
{
	// min & max
	if( *value < min ) *value = min;
	if( *value > max ) *value = max;
}

// generic int limiting
void MF_LimitInt( int * value, int min, int max )
{
	// min & max
	if( *value < min ) *value = min;
	if( *value > max ) *value = max;
}

/*
=================
MF_ExtractEnumFromId

Get the catagory/class enum from the vehicle index (use CAT_ANY & CLASS_ANY to decide operation done)
=================
*/

int MF_ExtractEnumFromId( int vehicle, unsigned int op )
{
	int daEnum = -1;

	// which op?
	if( op & CAT_ANY )
	{
		// catagory
		daEnum = availableVehicles[ vehicle ].cat;
	}
	else if( op & CLASS_ANY )
	{
		// class
		daEnum = availableVehicles[ vehicle ].cls;
	}

	return daEnum;
}




/*
=================
MF_findTag

providing the file and tagname this functions returns the taginfo
=================
*/
int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );


qboolean MF_findTag(const char* fileName, const char* tagname, md3Tag_t* tag)
{
	fileHandle_t	f;
	qboolean		found = qfalse;

	if( trap_FS_FOpenFile(fileName, &f, FS_READ) >= 0 ) {
		md3Header_t head;
		md3Frame_t	frame;
		int			i;	
		trap_FS_Read(&head, sizeof(head), f);
//		CG_Printf("%s has %i tags\n", fileName, head.numTags );
		for( i = 0; i < head.numFrames; ++i ) {
			trap_FS_Read(&frame, sizeof(frame), f);
		}
		for( i = 0; i < head.numTags; ++i ) {
			trap_FS_Read(tag, sizeof(md3Tag_t), f);
			if( strcmp( tag->name, tagname ) == 0 ) {
				found = qtrue;
				break;
			}
//			CG_Printf("tag %i: %s\n", i, tag.name );
//			CG_Printf("tag %i: %.1f %.1f %.1f\n", i, tag.origin[0], tag.origin[1], tag.origin[2] );
		}
		trap_FS_FCloseFile(f);
	} else {
		Com_Printf( "Unable to read tag from &s\n", fileName );
	}
	return found;
}



/*
=================
MF_distributeWeaponsOnPylons

weapon mount info
=================
*/

qboolean MF_distributeWeaponsOnPylons( int idx, completeLoadout_t* loadout )
{
	int	i, j, numPylons = 0, numWeaps = 0;
	int	weaps[MAX_WEAPONS_PER_VEHICLE-2];
	int ammos[MAX_WEAPONS_PER_VEHICLE-2];
	int pyls[MAX_WEAPONS_PER_VEHICLE];
	qboolean done;

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
		switch( pyls[i] ) {
		case PT_WINGTIP:
			strcpy( loadout->tags[i].tagname[0], "tag_tip_l" );
			strcpy( loadout->tags[i].tagname[1], "tag_tip_r" );
			mounts = 2;
			break;
		case PT_WING_L:
		case PT_BODY_L:
		case PT_WING_M:
		case PT_BODY_M:
			Com_sprintf(temp, 15, "tag_mt%d_l", i+1 );
			strcpy( loadout->tags[i].tagname[0], temp );
			Com_sprintf(temp, 15, "tag_mt%d_r", i+1 );
			strcpy( loadout->tags[i].tagname[1], temp );
			mounts = 2;
			break;
		case PT_WING_H:
		case PT_BODY_H:
		case PT_BODY_CENTER_H:
			Com_sprintf(temp, 15, "tag_mt%d_1_l", i+1 );
			strcpy( loadout->tags[i].tagname[0], temp );
			Com_sprintf(temp, 15, "tag_mt%d_1_r", i+1 );
			strcpy( loadout->tags[i].tagname[1], temp );
			Com_sprintf(temp, 15, "tag_mt%d_2_r", i+1 );
			strcpy( loadout->tags[i].tagname[2], temp );
			Com_sprintf(temp, 15, "tag_mt%d_2_l", i+1 );
			strcpy( loadout->tags[i].tagname[3], temp );
			Com_sprintf(temp, 15, "tag_mt%d_3_l", i+1 );
			strcpy( loadout->tags[i].tagname[4], temp );
			Com_sprintf(temp, 15, "tag_mt%d_3_r", i+1 );
			strcpy( loadout->tags[i].tagname[5], temp );
			mounts = 6;
			break;
		case PT_BODY_CENTER_L:
		case PT_BODY_CENTER_M:
		case PT_BAY:
		case PT_BAY_CENTER:
			Com_sprintf(temp, 15, "tag_mt%d_l", i+1 );
			strcpy( loadout->tags[i].tagname[0], temp );
			Com_sprintf(temp, 15, "tag_mt%d_r", i+1 );
			strcpy( loadout->tags[i].tagname[1], temp );
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
				loadout->mountedWeapons[i] = num / availableWeapons[weaps[j]].numberPerPackage;
				done = qtrue;
			}
			if( done ) break;
		}
	}

	// print out
	Com_Printf( "Loadout is:\n" );
	for( i = 0; i < numPylons; ++i ) {
		Com_Printf( "Pylon %d (type: %d): %d * %s\n", i, loadout->type[i],
				loadout->mountedWeapons[i], availableWeapons[loadout->weaponType[i]].descriptiveName );
		Com_Printf( "Tags are:\n" );
		for( j = 0; j < MAX_MOUNTS_PER_PYLON*2; ++j ) {
			Com_Printf( "%s\n", loadout->tags[i].tagname[j] );
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
	if( idx < 0 ) return;
	memcpy( loadout, &availableLoadouts[idx], sizeof(completeLoadout_t) );
}


/*
=================
MF_removeWeaponFromLoadout

firing
=================
*/

qboolean MF_removeWeaponFromLoadout( int weaponIndex, completeLoadout_t* loadout, char* usedTag )
{
	int i, num;

	for( i = 0; i < MAX_WEAPONS_PER_VEHICLE; ++i ) {
		if( loadout->weaponType[i] == weaponIndex &&
			loadout->mountedWeapons[i] &&
			!(availableWeapons[weaponIndex].flags & WF_NON_REMOVABLE_VWEP) ) {
			num = --loadout->mountedWeapons[i];
			if( usedTag ) strcpy( usedTag, loadout->tags[i].tagname[num] );
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

void MF_addWeaponToLoadout( int weaponIndex, completeLoadout_t* loadout )
{
}

