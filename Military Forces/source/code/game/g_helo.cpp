
#include "g_helo.h"
#include "g_local.h"
#include "g_level.h"


void
Entity_Helicopter::checkForTakeoffOrLanding()
{
	trace_t	trace;
	vec3_t	startpos;
	vec3_t	endpos;
	vec3_t	vel;
	float	height = (client_->ps_.ONOFF & OO_LANDED) ? 3 : 1;
    float	throttle = client_->ps_.fixed_throttle;
    int		maxthrottle = availableVehicles[client_->vehicle_].maxthrottle;

	// get real throttle
	throttle > maxthrottle ? -(throttle-maxthrottle) : throttle;	// real vert speed

	VectorSet( startpos, s.pos.trBase[0], s.pos.trBase[1], s.pos.trBase[2] + r.mins[2]);
	if( client_->ps_.ONOFF & OO_GEAR ) 
		startpos[2] -= gearheight_;

	VectorCopy( startpos, endpos );
	endpos[2] -= height;
	SV_Trace (&trace, startpos, NULL, NULL, endpos, s.number, MASK_PLAYERSOLID, false );

	// check for takeoff
	if((client_->ps_.ONOFF & OO_LANDEDTERRAIN) && throttle) 
	{
		client_->ps_.ONOFF &= ~(OO_LANDEDTERRAIN|OO_LANDED);
	} 
	else if (client_->ps_.ONOFF & OO_LANDED && !(client_->ps_.ONOFF & OO_LANDEDTERRAIN)) 
	{
		if(throttle)
			client_->ps_.ONOFF &= ~OO_LANDED;
		else 
		{
			if( trace.fraction == 1.0 || (trace.entityNum != ENTITYNUM_NONE &&
				!canLandOnIt(theLevel.getEntity(trace.entityNum)) ) ) 
			{
					//if(self->client->airOutTime < level.time) {
						G_Damage( this, NULL, this, NULL, NULL, availableVehicles[client_->vehicle_].maxhealth/3, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
					//	self->client->airOutTime = level.time + 1000;
					//}
					VectorNegate(client_->ps_.velocity, vel);
					VectorAdd(vel, client_->ps_.origin,  client_->ps_.origin);
			}
		}
	} 
	else 
	{ 
		// check for landing
		if( trace.fraction == 1.0 ) 
			return;
		if( trace.entityNum != ENTITYNUM_NONE && (client_->ps_.ONOFF & OO_GEAR) ) 
		{
			if( client_->ps_.vehicleAngles[0] < 30 &&
				client_->ps_.vehicleAngles[0] >= 0 )
			{

				if( fabs( client_->ps_.vehicleAngles[2] ) < 20 )
				{
					if( canLandOnIt(theLevel.getEntity(trace.entityNum)) )
						client_->ps_.ONOFF |= OO_LANDED;
					else 
						client_->ps_.ONOFF |= OO_LANDEDTERRAIN|OO_LANDED;
					client_->ps_.fixed_throttle = 0;
				}
				else 
				{
					G_Damage( this, NULL, this, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
				}
			} 
			else if ( client_->ps_.vehicleAngles[0] >= 30 ) 
			{
				G_Damage( this, NULL, this, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
			}
		}
	}
}


void
Entity_Helicopter::Entity_Helicopter_Touch::execute( GameEntity* other, trace_t *trace )
{
	Com_Printf( "You crashed into a %s\n", other->classname_ );
	G_Damage( self_, NULL, self_, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
}

void
Entity_Helicopter::Entity_Helicopter_Pain::execute( GameEntity* attacker, int damage )
{
	if( damage < 5 ) 
		return;
	if( damage > 50 ) 
		damage = 50;

	G_AddEvent( self_, EV_VEHICLE_HIT, damage, true );
}



