/*
 * $Id: g_groundvehicle.c,v 1.2 2002-07-15 18:23:07 thebjoern Exp $
*/


#include "g_local.h"

// NOTE: ground vehicles may have thrust settings which are less than 0 
// if it is less than 0 then the vehicle is reversing

// NOTE2: there are two possibilities how ground vehicles can be driven
// around, one is the MFQ3 specific way by just applying throttle (like on
// the planes) and the tank then sets the speed according to the throttle,
// that way you can have the tank going in a certain direction without having
// to hold down a button and can concentrate on pounding some targets. The 
// other way is more like in "normal Q3" where the tank goes forward (back) while
// the forward (back) button is pressed (ie pretty much like in the q3tank mod).
// When the forward/back button is pressed this is equal to applying full forward
// or back thrust (and therefore sets the throttle variable accordingly), if the
// button is released then the throttle falls back to 0 (!). That way you can
// for example be cruising around with 50% throttle, looking out for enemies,
// and when suddenly an enemy appears you press the forward button to accelerate
// to full speed ("jumping on the accelerator")

void Touch_GroundVehicle( gentity_t *self, gentity_t *other, trace_t *trace ) 
{
//	G_Printf( "You crashed into a %s\n", other->classname );
//	G_Printf( "You crashed into a %.1f\n", self->client->ps.velocity[1] );
//	self->client->ps.velocity[2] = 50;
	// instead of dying on every collision we take damage depending on the speed
	// with which we crashed into an obstacle, and this should be rather low...

//  G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH );
}


void GroundVehicle_Pain( gentity_t *self, gentity_t *attacker, int damage ) {
	
	// just adding a visible event when a vehicle is hit. if this effect doesn't
	// look ok, we can just modify the system to have EV_PLANE_HIT and 
	// EV_GROUNDVEHICLE_HIT etc and just make proper effects for all of them

	if( damage < 5 ) return;
	if( damage > 50 ) damage = 50;

	G_AddEvent(self, EV_VEHICLE_HIT, damage, qtrue);

}
