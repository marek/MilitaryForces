
#include "g_plane.h"
#include "g_local.h"
#include "g_level.h"


void
Entity_Plane::checkForTakeoffOrLanding()
{
	trace_t	trace;
	vec3_t	startpos;
	vec3_t	endpos;
	float height = (client_->ps_.ONOFF & OO_LANDED) ? 3 : 1 + std::max( 0.0f, -client_->ps_.velocity[2] ) * 0.1f;

	VectorSet( startpos, s.pos.trBase[0], s.pos.trBase[1], s.pos.trBase[2] + r.mins[2]);
	if( client_->ps_.ONOFF & OO_GEAR ) 
		startpos[2] -= gearheight_;

	VectorCopy( startpos, endpos );
	endpos[2] -= height;
	vec3_t	wheelMins = { -6, -6, 0 };
	vec3_t	wheelMaxs = { 6, 6, 0 };
	GameHost::trace (&trace, startpos, wheelMins, wheelMaxs, endpos, s.number, MASK_PLAYERSOLID, false );

	// check for takeoff
	if( client_->ps_.ONOFF & OO_LANDED ) 
	{
		const bool taxiing = trace.fraction < 1.0 && trace.entityNum == ENTITYNUM_WORLD &&
			client_->ps_.speed < availableVehicles[client_->vehicle_].stallspeed * 6;
		if( !taxiing && ( trace.fraction == 1.0 || ( trace.entityNum != ENTITYNUM_NONE &&
			!canLandOnIt(theLevel.getEntity(trace.entityNum)) ) ) ) 
		{
			client_->ps_.ONOFF &= ~OO_LANDED;
		} 
		else 
		{
			vec3_t angles;
			vectoangles( trace.plane.normal, angles );
//			Common::printf( "maxs = %.1f, trace = %.1f\n", g_entities[trace.entityNum].r.maxs[2], trace.endpos[2]); 
//			self->client->ps.origin[2] = g_entities[trace.entityNum].r.maxs[2] + 
//				fabs(self->r.mins[2]) + ( (self->client->ps.powerups[PW_ONOFF]&OO_GEAR) ?
//				self->gearheight : 0 );

			client_->ps_.origin[2] = trace.endpos[2] + 1 + fabs(r.mins[2]) + 
				( (client_->ps_.ONOFF&OO_GEAR) ? gearheight_ : 0 );


		}
	} 
	else 
	{ 
		// check for landing
		if( trace.fraction == 1.0 ) 
			return;
		if( trace.entityNum != ENTITYNUM_NONE && canLandOnIt(theLevel.getEntity(trace.entityNum)) &&
			(client_->ps_.ONOFF & OO_GEAR) ) 
		{
			if( client_->ps_.vehicleAngles[0] < 30 &&
				client_->ps_.vehicleAngles[0] >= -15 ) 
			{
				if( fabs( client_->ps_.vehicleAngles[2] ) < 20 ) 
				{
					client_->ps_.ONOFF |= OO_LANDED;
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
Entity_Plane::Entity_Plane_Touch::execute( GameEntity* other, trace_t *trace )
{
	Game::damage( self_, nullptr, self_, nullptr, nullptr, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
}

void
Entity_Plane::Entity_Plane_Pain::execute( GameEntity* attacker, int damage )
{
	if( damage < 5 ) 
		return;
	if( damage > 50 ) 
		damage = 50;

	Game::addEvent( self_, EV_VEHICLE_HIT, damage, true );
}





