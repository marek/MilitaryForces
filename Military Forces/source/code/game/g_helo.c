/*
 * $Id: g_helo.c,v 1.3 2005-11-20 11:21:38 thebjoern Exp $
*/


#include "g_local.h"

// ugly first-cut function!
void checkTakeoffLandingHelo( gentity_t *self )
{
	trace_t	trace;
	vec3_t	startpos;
	vec3_t	endpos;
	vec3_t	vel;
	float	height = (self->client->ps.ONOFF & OO_LANDED) ? 3 : 1;
    float	throttle = self->client->ps.fixed_throttle;
    int		maxthrottle = availableVehicles[self->client->vehicle].maxthrottle;
//	int		minthrottle = availableVehicles[self->client->vehicle].minthrottle;

	// get real throttle
	throttle > maxthrottle ? -(throttle-maxthrottle) : throttle;	// real vert speed

	VectorSet( startpos, self->s.pos.trBase[0], self->s.pos.trBase[1], self->s.pos.trBase[2] + self->r.mins[2]);
	if( self->client->ps.ONOFF & OO_GEAR ) {
		startpos[2] -= self->gearheight;
	}
	VectorCopy( startpos, endpos );
	endpos[2] -= height;
	trap_Trace (&trace, startpos, NULL, NULL, endpos, self->s.number, MASK_PLAYERSOLID );

	// check for takeoff
	if((self->client->ps.ONOFF & OO_LANDEDTERRAIN) && throttle) {
		self->client->ps.ONOFF &= ~(OO_LANDEDTERRAIN|OO_LANDED);
	} else if (self->client->ps.ONOFF & OO_LANDED && !(self->client->ps.ONOFF & OO_LANDEDTERRAIN)) {
		if(throttle)
			self->client->ps.ONOFF &= ~OO_LANDED;
		else {
			if( trace.fraction == 1.0 || (trace.entityNum != ENTITYNUM_NONE &&
				!canLandOnIt(&g_entities[trace.entityNum]) ) ) {
					//if(self->client->airOutTime < level.time) {
						G_Damage( self, NULL, self, NULL, NULL, availableVehicles[self->client->vehicle].maxhealth/3, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
					//	self->client->airOutTime = level.time + 1000;
					//}
					VectorNegate(self->client->ps.velocity, vel);
					VectorAdd(vel, self->client->ps.origin,  self->client->ps.origin);
			}
		}
	} else { // check for landing
		if( trace.fraction == 1.0 ) return;
		if( trace.entityNum != ENTITYNUM_NONE && (self->client->ps.ONOFF & OO_GEAR) ) {
			if( self->client->ps.vehicleAngles[0] < 30 &&
				self->client->ps.vehicleAngles[0] >= 0 ) {

				if( fabs( self->client->ps.vehicleAngles[2] ) < 20 ) {
					if(canLandOnIt(&g_entities[trace.entityNum]))
						self->client->ps.ONOFF |= OO_LANDED;
					else 
						self->client->ps.ONOFF |= OO_LANDEDTERRAIN|OO_LANDED;
					self->client->ps.fixed_throttle = 0;
				}
				else {
					G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
				}
			} else if ( self->client->ps.vehicleAngles[0] >= 30 ) {
				G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
			}
		}
	}
}

void Touch_Helo( gentity_t *self, gentity_t *other, trace_t *trace ) 
{
	Com_Printf( "You crashed into a %s\n", other->classname );
//	Com_Printf( "You crashed into a %.1f\n", self->client->ps.velocity[1] );
//	self->client->ps.velocity[2] = 50;
	// instead of dying on every collision we take damage depending on the speed
	// with which we crashed into an obstacle, and this should be rather low...

	   G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
}


void Helo_Pain( gentity_t *self, gentity_t *attacker, int damage ) {
	
	// just adding a visible event when a vehicle is hit. if this effect doesn't
	// look ok, we can just modify the system to have EV_PLANE_HIT and 
	// EV_GROUNDVEHICLE_HIT etc and just make proper effects for all of them

	if( damage < 5 ) return;
	if( damage > 50 ) damage = 50;

	G_AddEvent(self, EV_VEHICLE_HIT, damage, true);

}
