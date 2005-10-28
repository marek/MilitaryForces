/*
 * $Id: g_droneground.c,v 1.2 2005-10-28 13:06:54 thebjoern Exp $
*/

#include "g_local.h"





void Drone_Ground_Think( gentity_t* ent ) {


	ent->nextthink = level.time + 100;
	trap_LinkEntity (&ent->s, &ent->r);
}
