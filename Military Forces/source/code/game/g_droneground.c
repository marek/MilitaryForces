/*
 * $Id: g_droneground.c,v 1.1 2005-08-22 16:06:28 thebjoern Exp $
*/

#include "g_local.h"





void Drone_Ground_Think( gentity_t* ent ) {


	ent->nextthink = level.time + 100;
	trap_LinkEntity (ent);
}
