/*
 * $Id: bg_mfq3util.c,v 1.16 2002-02-20 20:03:01 sparky909_uk Exp $
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
