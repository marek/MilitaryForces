/*
 * $Id: g_mfq3util.c,v 1.2 2001-11-19 16:41:36 thebjoern Exp $
*/



#include "g_local.h"




int canLandOnIt( gentity_t *ent )
{
	if( strcmp( ent->classname, "func_runway" ) == 0 ||
		strcmp( ent->classname, "func_plat" ) == 0 ) {
		return 1;
	}
	return 0;
}
