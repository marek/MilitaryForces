/*
 * $Id: g_droneground.c,v 1.1 2001-11-15 21:35:14 thebjoern Exp $
*/

#include "g_local.h"





void Drone_Ground_Think( gentity_t* ent ) {


	ent->nextthink = level.time + 100;
	trap_LinkEntity (ent);
}
