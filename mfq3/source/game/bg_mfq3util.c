/*
 * $Id: bg_mfq3util.c,v 1.3 2002-01-18 16:48:26 sparky909_uk Exp $
*/

#include "q_shared.h"
#include "bg_public.h"

// externals
extern void	trap_Cvar_Set( const char *var_name, const char *value );
extern void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

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

// returns the number of items in a stringlist
int MF_getNumberOfItems(const char **itemlist)
{
	int i;
	for( i = 0; itemlist[i] != 0; i++ );
	return i;
}

// takes an already properly right-shifted hex value (with only 1 bit set!)
// and returns the item index for it
int MF_getItemIndexFromHex(int hexValue)
{
	int i;
	for( i = -1; hexValue; i++ ) hexValue>>=1;
	return i;
}

