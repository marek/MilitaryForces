/*
 * $Id: bg_mfq3util.c,v 1.7 2002-01-31 10:12:05 sparky909_uk Exp $
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
unsigned long MF_GetGameset( void )
{
	char tmpGamesetStr[ 32 ];

	// get the gameset text
	trap_Cvar_VariableStringBuffer( "mf_gameset", tmpGamesetStr, sizeof(tmpGamesetStr) );

	// compare and return
	if( strcmp( tmpGamesetStr, "ww1" ) == 0 )
	{
		return MF_GAMESET_WW1;
	}
	else if( strcmp( tmpGamesetStr, "ww2" ) == 0 )
	{
		return MF_GAMESET_WW2;
	}
	else if( strcmp( tmpGamesetStr, "modern" ) == 0 )
	{
		return MF_GAMESET_MODERN;
	}

	// wasn't found, but to be safe
	return MF_GAMESET_MODERN;
}

/*
=================
MF_getIndexOfVehicleEx
=================
*/

int MF_getIndexOfVehicleEx( int start, int vehicleCat, int vehicleClass, unsigned long team, unsigned long gameset )
{
	// NOTE: vehicleClass & vehicleCat are enum indexed
	// team & gameset are bitmapped MFQ3

	unsigned long what = 0x00000000;

	// add components (in LB -> HB order in DWORD)
	
	// CATAGORY
	if( vehicleCat < 0 )
	{
		// any catagory
		what |= CAT_ANY;
	}
	else
	{
		// specific catagory
		what |= (1 << vehicleCat) << 8;		// (convert from enum)
	}

	// CLASS
	if( vehicleClass < 0 )
	{
		// any class
		what |= CLASS_ANY;
	}
	else
	{
		// specific class
		what |= (1 << vehicleClass);		// (convert from enum)
	}

	what |= team;		// as bitmapped
	what |= gameset;	// as bitmapped

	return MF_getIndexOfVehicle( start, what );
}

/*
=================
MF_getIndexOfVehicle
=================
*/

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
	unsigned long cat = 0;

	// find catagory
	cat = availableVehicles[ vehicle ].id & CAT_ANY;
	if( cat & CAT_PLANE ) {
		strcpy( catDir, "planes" );
	}
	else if( cat & CAT_GROUND ) {
		strcpy( catDir, "ground" );
	}
	else if( cat & CAT_HELO ) {
		strcpy( catDir, "helicopters" );
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
		daEnum = availableVehicles[ vehicle ].id & CAT_ANY;
		daEnum >>= 8;
	}
	else if( op & CLASS_ANY )
	{
		// class
		daEnum = availableVehicles[ vehicle ].id & CLASS_ANY;
	}

	return daEnum;
}