
#include "g_lqm.h"
#include "g_local.h"


void
Entity_Infantry::Entity_Infantry_Touch::execute( GameEntity* other, trace_t *trace )
{
	// Get Run over
	if( (strcmp(other->classname_, "player") == 0) && (availableVehicles[other->client_->vehicle_].cat != CAT_LQM) &&
		( (g_gametype.integer >= GT_TEAM && other->team_ != self_->team_) ||
		(g_gametype.integer < GT_MISSION_EDITOR) ) ) 
	{
		  G_Damage( self_, NULL, other, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
	}
}

void
Entity_Infantry::Entity_Infantry_Pain::execute( GameEntity* attacker, int damage )
{
	if( damage < 5 ) 
		return;
	if( damage > 50 ) 
		damage = 50;

	G_AddEvent( self_, EV_VEHICLE_HIT, damage, true );
}




