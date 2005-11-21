/*
 * $Id: g_droneground.c,v 1.3 2005-11-21 17:28:20 thebjoern Exp $
*/

#include "g_local.h"





void Drone_Ground_Think( gentity_t* ent ) {


	ent->nextthink = level.time + 100;
	SV_LinkEntity (&ent->s, &ent->r);
}
