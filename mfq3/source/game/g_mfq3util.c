/*
 * $Id: g_mfq3util.c,v 1.3 2001-12-03 12:02:10 thebjoern Exp $
*/



#include "g_local.h"




int canLandOnIt( gentity_t *ent )
{
	if( strcmp( ent->classname, "func_runway" ) == 0 ||
		strcmp( ent->classname, "func_plat" ) == 0 ||
		strcmp( ent->classname, "func_train" ) == 0 ||
		strcmp( ent->classname, "func_door" ) == 0 ) {
		return 1;
	}
	return 0;
}
