/*
 * $Id: bg_mfq3util.c,v 1.25 2002-02-28 17:26:06 thebjoern Exp $
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
MF_ExtractEnumFromId

Get the catagory/class enum from the vehicle index (use CAT_ANY & CLASS_ANY to decide operation done)
=================
*/

void MF_LoadAllVehicleData()
{
	int i, j, num;
	char* modelbasename = 0;
	char name[128];

	// MFQ3 loadouts
	MF_calculateAllDefaultLoadouts();

	for( i = 0; i < bg_numberOfVehicles; ++i ) {
		// remove weapons not fitting
		for( j = WP_WEAPON1; j < WP_FLARE; ++j ) {
			if( !(availableVehicles[i].cat & availableWeapons[availableVehicles[i].weapons[j]].fitsCategory) ) {
				availableVehicles[i].ammo[j] = 0;
			}
		}
		modelbasename = MF_CreateModelPathname( i, "models/vehicles/%s/%s/%s" );	
		// boundingbox
		Com_sprintf( name, sizeof(name), "%s.md3", modelbasename );
		MF_getDimensions( name, 0, &availableVehicles[i].maxs, &availableVehicles[i].mins );
		availableVehicles[i].mins[2] += 1;// to look better ?
		// helo/plane specific
		if( (availableVehicles[i].cat & CAT_PLANE) ||
			(availableVehicles[i].cat & CAT_HELO) ) {
			// gear
			Com_sprintf( name, sizeof(name), "%s_gear.md3", modelbasename );
			if( MF_getNumberOfFrames( name, &num ) ) {
				vec3_t min1, min2;
				availableVehicles[i].maxGearFrame = num - 1;
				if( MF_getDimensions( name, 0, 0, &min1 ) &&
					MF_getDimensions( name, num-1, 0, &min2 ) ) {
					availableVehicles[i].gearheight = min1[2] - min2[2] - 1.5;// for coll. detection
					if( availableVehicles[i].gearheight < 0 ) availableVehicles[i].gearheight = 0;
				}
			} else {
				availableVehicles[i].maxGearFrame = GEAR_DOWN_DEFAULT;
			}
			// bay
			Com_sprintf( name, sizeof(name), "%s_bay.md3", modelbasename );
			if( MF_getNumberOfFrames( name, &num ) ) {
				availableVehicles[i].maxBayFrame = num - 1;
			} else {
				availableVehicles[i].maxBayFrame = BAY_DOWN_DEFAULT;
			}
			// correct actual loadouts
			for( j = WP_WEAPON1; j < WP_FLARE; ++j ) {
				if( availableVehicles[i].weapons[j] ) {
					availableVehicles[i].ammo[j] = 
						MF_findWeaponsOfType( availableVehicles[i].weapons[j], &availableLoadouts[i] );
				}
			}
		} else if( (availableVehicles[i].cat & CAT_GROUND) ) {
			vec3_t max, min;
			float diff;
			// increaes bounding box by turret height
			Com_sprintf( name, sizeof(name), "%s_tur.md3", modelbasename );
			MF_getDimensions( name, 0, &max, &min );
			diff = max[2] - min[2];
			availableVehicles[i].maxs[2] += diff;
			availableVehicles[i].mins[2] -= 2;
		} else if( (availableVehicles[i].cat & CAT_BOAT) ) {
			availableVehicles[i].mins[2] -= 2;
		}
	}
}


