/*
 * $Id: bg_mfq3util.c,v 1.15 2002-02-18 16:23:25 sparky909_uk Exp $
*/

#include "q_shared.h"
#include "bg_public.h"

// externals
extern void	trap_Cvar_Set( const char *var_name, const char *value );
extern void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

/*
=================
MF_SetGameset
=================
*/

// sets the mf_gameset variable based on the given MF_GAMESET_x parameter
void MF_SetGameset(unsigned long gs)
{
	switch( gs )
	{
	case MF_GAMESET_WW1:
		trap_Cvar_Set("mf_gameset", "ww1");
		break;
	case MF_GAMESET_WW2:
		trap_Cvar_Set("mf_gameset", "ww2");
		break;
	case MF_GAMESET_MODERN:
	default:
		trap_Cvar_Set("mf_gameset", "modern");
		break;
	}
}

/*
=================
MF_GetGameset
=================
*/

// gets the mf_gameset variable into the correct a MF_GAMESET_x return
unsigned long MF_GetGameset( qboolean asEnum )
{
	char tmpGamesetStr[ 32 ];
	unsigned long returnValue = MF_GAMESET_MODERN;

	// get the gameset text
	trap_Cvar_VariableStringBuffer( "mf_gameset", tmpGamesetStr, sizeof(tmpGamesetStr) );

	// compare
	if( Q_stricmp( tmpGamesetStr, "ww1" ) == 0 )
	{
		returnValue = MF_GAMESET_WW1;
	}
	else if( Q_stricmp( tmpGamesetStr, "ww2" ) == 0 )
	{
		returnValue = MF_GAMESET_WW2;
	}
	else if( Q_stricmp( tmpGamesetStr, "modern" ) == 0 )
	{
		returnValue = MF_GAMESET_MODERN;
	}

	// convert to enum?
	if( asEnum )
	{
		switch( returnValue )
		{
		case MF_GAMESET_WW1:
			returnValue = 2;
			break;
		case MF_GAMESET_WW2:
			returnValue = 1;
			break;
		case MF_GAMESET_MODERN:
			returnValue = 0;
			break;
		}
	}

	// return the result (as either flag mask or enum)
	return returnValue;
}

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
