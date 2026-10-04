
#include "g_helo.h"
#include "g_local.h"
#include "g_level.h"


void
Entity_Helicopter::checkForTakeoffOrLanding()
{
	trace_t	trace;
	vec3_t	startpos;
	vec3_t	endpos;
	float	height = (client_->ps_.ONOFF & OO_LANDED) ? 3 : 1 + MAX( 0.0f, -client_->ps_.velocity[2] ) * 0.1f;
    float	throttle = client_->ps_.fixed_throttle;
    int		maxthrottle = availableVehicles[client_->vehicle_].maxthrottle;

	// get real throttle
	throttle > maxthrottle ? -(throttle-maxthrottle) : throttle;	// real vert speed

	VectorSet( startpos, s.pos.trBase[0], s.pos.trBase[1], s.pos.trBase[2] + r.mins[2]);
	if( client_->ps_.ONOFF & OO_GEAR ) 
		startpos[2] -= gearheight_;

	VectorCopy( startpos, endpos );
	endpos[2] -= height;
	SV_Trace (&trace, startpos, nullptr, nullptr, endpos, s.number, MASK_PLAYERSOLID, false );

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
				client_->ps_.ONOFF &= ~OO_LANDED;
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
				client_->ps_.vehicleAngles[0] >= -20 )
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
					Game::damage( this, nullptr, this, nullptr, nullptr, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
				}
			} 
			else
			{
				Game::damage( this, nullptr, this, nullptr, nullptr, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
			}
		}
	}
}


void
Entity_Helicopter::Entity_Helicopter_Touch::execute( GameEntity* other, trace_t *trace )
{
	Game::damage( self_, nullptr, self_, nullptr, nullptr, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
}

void
Entity_Helicopter::Entity_Helicopter_Pain::execute( GameEntity* attacker, int damage )
{
	if( damage < 5 ) 
		return;
	if( damage > 50 ) 
		damage = 50;

	Game::addEvent( self_, EV_VEHICLE_HIT, damage, true );
}



