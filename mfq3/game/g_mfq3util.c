/*
 * $Id: g_mfq3util.c,v 1.1 2001-11-15 15:47:38 moorman Exp $
*/



#include "g_local.h"




int canLandOnIt( gentity_t *ent )
{
	if( strcmp( ent->classname, "func_runway" ) == 0 ) {
		return 1;
	}
	return 0;
}
