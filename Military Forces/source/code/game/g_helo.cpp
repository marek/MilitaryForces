
#include "g_helo.h"
#include "g_local.h"
#include "g_level.h"


void
Entity_Helicopter::checkForTakeoffOrLanding()
{
	trace_t	trace;
	Vec3	startpos;
	Vec3	endpos;
	float	height = (client_->ps_.ONOFF & OO_LANDED) ? 3 : 1 + std::max( 0.0f, -client_->ps_.velocity[2] ) * 0.1f;
    float	throttle = client_->ps_.fixed_throttle;
    int		maxthrottle = availableVehicles[client_->vehicle_].maxthrottle;

	// the collective as the pilot has it: up is positive, down negative (the stored value counts down past the maximum)
	throttle = throttle > maxthrottle ? -(throttle - maxthrottle) : throttle;

	startpos = Vec3( s.pos.trBase[0], s.pos.trBase[1], s.pos.trBase[2] + r.mins[2] );
	if( client_->ps_.ONOFF & OO_GEAR ) 
		startpos[2] -= gearheight_;

	endpos = startpos;
	endpos[2] -= height;
	Vec3	wheelMins = { -6, -6, 0 };
	Vec3	wheelMaxs = { 6, 6, 0 };
	GameHost::trace (&trace, startpos, wheelMins, wheelMaxs, endpos, s.number, MASK_PLAYERSOLID, false );

	// check for takeoff
	if((client_->ps_.ONOFF & OO_LANDEDTERRAIN) && throttle > 0) 
	{
		client_->ps_.ONOFF &= ~(OO_LANDEDTERRAIN|OO_LANDED);
	} 
	else if (client_->ps_.ONOFF & OO_LANDED && !(client_->ps_.ONOFF & OO_LANDEDTERRAIN)) 
	{
		if(throttle > 0)
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
		// check for landing. Not while the pilot is pulling up: a helicopter standing on the ground with its
		// collective raised is leaving, and taken for landing again it would be set down and its collective
		// zeroed every frame, never to lift
		if( trace.fraction == 1.0 || throttle > 0 ) 
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
					// The ground was looked for a little ahead of a fast descent, and found there: the helicopter
					// goes down the rest of the way, or it would stand in the air over where it landed
					const float gap = trace.fraction * height;
					client_->ps_.origin[2] -= gap;
					s.pos.trBase[2] -= gap;
					r.currentOrigin[2] -= gap;
					VectorClear( client_->ps_.velocity );
					GameHost::linkEntity( this );
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



