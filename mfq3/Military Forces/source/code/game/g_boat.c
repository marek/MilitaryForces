/*
 * $Id: g_boat.c,v 1.1 2005-08-22 15:41:17 thebjoern Exp $
*/


#include "g_local.h"

void Touch_Boat( gentity_t *self, gentity_t *other, trace_t *trace ) 
{
//	G_Printf( "You crashed into a %s\n", other->classname );
//	G_Printf( "You crashed into a %.1f\n", self->client->ps.velocity[1] );
//	self->client->ps.velocity[2] = 50;
	// instead of dying on every collision we take damage depending on the speed
	// with which we crashed into an obstacle, and this should be rather low...

//  G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH );
}


void Boat_Pain( gentity_t *self, gentity_t *attacker, int damage ) {
	
	// just adding a visible event when a vehicle is hit. if this effect doesn't
	// look ok, we can just modify the system to have EV_PLANE_HIT and 
	// EV_GROUNDVEHICLE_HIT etc and just make proper effects for all of them

	if( damage < 5 ) return;
	if( damage > 50 ) damage = 50;

	G_AddEvent(self, EV_VEHICLE_HIT, damage, qtrue);

}
