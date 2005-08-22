/*
 * $Id: g_plane.c,v 1.1 2005-08-22 16:07:07 thebjoern Exp $
*/


#include "g_local.h"

// ugly first-cut function!
void checkTakeoffLandingPlane( gentity_t *self )
{
	trace_t	trace;
	vec3_t	startpos;
	vec3_t	endpos;
	float height = (self->client->ps.ONOFF & OO_LANDED) ? 3 : 1;

	VectorSet( startpos, self->s.pos.trBase[0], self->s.pos.trBase[1], self->s.pos.trBase[2] + self->r.mins[2]);
	if( self->client->ps.ONOFF & OO_GEAR ) {
		startpos[2] -= self->gearheight;
	}
	VectorCopy( startpos, endpos );
	endpos[2] -= height;
	trap_Trace (&trace, startpos, NULL, NULL, endpos, self->s.number, MASK_PLAYERSOLID );

	// check for takeoff
	if( self->client->ps.ONOFF & OO_LANDED ) {
		if( trace.fraction == 1.0 || ( trace.entityNum != ENTITYNUM_NONE &&
			!canLandOnIt(&g_entities[trace.entityNum]) ) ) {
			self->client->ps.ONOFF &= ~OO_LANDED;
		} else {
			vec3_t angles;
			vectoangles( trace.plane.normal, angles );
//			G_Printf( "maxs = %.1f, trace = %.1f\n", g_entities[trace.entityNum].r.maxs[2], trace.endpos[2]); 
//			self->client->ps.origin[2] = g_entities[trace.entityNum].r.maxs[2] + 
//				fabs(self->r.mins[2]) + ( (self->client->ps.powerups[PW_ONOFF]&OO_GEAR) ?
//				self->gearheight : 0 );

			self->client->ps.origin[2] = trace.endpos[2] + 1 + fabs(self->r.mins[2]) + 
				( (self->client->ps.ONOFF&OO_GEAR) ? self->gearheight : 0 );


		}
	} else { // check for landing
		if( trace.fraction == 1.0 ) return;
		if( trace.entityNum != ENTITYNUM_NONE && canLandOnIt(&g_entities[trace.entityNum]) &&
			(self->client->ps.ONOFF & OO_GEAR) ) {
			if( self->client->ps.vehicleAngles[0] < 30 &&
				self->client->ps.vehicleAngles[0] >= 0 ) {

				if( fabs( self->client->ps.vehicleAngles[2] ) < 20 ) {
					self->client->ps.ONOFF |= OO_LANDED;
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

void Touch_Plane( gentity_t *self, gentity_t *other, trace_t *trace ) 
{
//	G_Printf( "You crashed into a %s\n", other->classname );
    G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );

/*	if( self->ONOFF & OO_LANDED ) {
		if( !other->client ) {
			G_Damage( self, NULL, self, NULL, NULL, self->client->ps.speed/10, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
			self->client->ps.speed = self->speed = 0;
		} else {
			if( (availableVehicles[other->client->vehicle].id&CAT_ANY) & CAT_PLANE ) {
				vec3_t	diff, f1, f2;
				float speed;
				AngleVectors( self->client->ps.vehicleAngles, f1, 0, 0 );
				AngleVectors( other->client->ps.vehicleAngles, f2, 0, 0 );
				VectorScale( f1, self->client->ps.speed/10, f1 );
				VectorScale( f2, other->client->ps.speed/10, f2 );
				VectorSubtract( f1, f2, diff);
				speed = VectorNormalize(diff);
				G_Damage( self, NULL, self, NULL, NULL, speed/20, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
				G_Damage( other, NULL, self, NULL, NULL, speed/20, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
				self->client->ps.speed = self->speed = 0;
			} else {
				G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
			}
		}
	} else {
		if( !other->client ) {
			G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
		} else {
			if( (availableVehicles[other->client->vehicle].id&CAT_ANY) & CAT_PLANE ) {
				vec3_t	diff, f1, f2;
				float speed;
				AngleVectors( self->client->ps.vehicleAngles, f1, 0, 0 );
				AngleVectors( other->client->ps.vehicleAngles, f2, 0, 0 );
				VectorScale( f1, self->client->ps.speed/10, f1 );
				VectorScale( f2, other->client->ps.speed/10, f2 );
				VectorSubtract( f1, f2, diff);
				speed = VectorNormalize(diff);
				G_Damage( self, NULL, self, NULL, NULL, speed/20, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
				G_Damage( other, NULL, self, NULL, NULL, speed/20, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
				self->client->ps.speed = other->client->ps.speed;
			} else {
				G_Damage( self, NULL, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
			}
		}
	}*/
}


void Plane_Pain( gentity_t *self, gentity_t *attacker, int damage ) {
	
	if( damage < 5 ) return;
	if( damage > 50 ) damage = 50;

	G_AddEvent(self, EV_VEHICLE_HIT, damage, qtrue);

}




