/*
 * $Id: g_droneground.c,v 1.1 2005-08-22 15:41:17 thebjoern Exp $
*/

#include "g_local.h"





void Drone_Ground_Think( gentity_t* ent ) {


	ent->nextthink = level.time + 100;
	trap_LinkEntity (ent);
}
