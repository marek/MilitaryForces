/*
 * $Id: g_mfq3ents.c,v 1.2 2001-11-16 12:01:39 thebjoern Exp $
*/


#include "g_local.h"



/*
===============================================================================

RUNWAY

===============================================================================
*/

void explosive_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
{
	gentity_t* ent;
	vec3_t	pos;
	VectorAdd( self->r.absmin, self->r.absmax, pos );
	VectorScale( pos, 0.5f, pos );

	self->r.svFlags = SVF_NOCLIENT;
	self->s.eType = ET_INVISIBLE;

	if( g_gametype.integer >= GT_TEAM && self->score && attacker && attacker->client ) {
		int score;
		if( self->s.generic1 == attacker->client->ps.persistant[PERS_TEAM] ) score = -self->score;
		else if( self->s.generic1 != attacker->client->ps.persistant[PERS_TEAM] ) score = self->score;

		AddScore( attacker, pos, score );
	}

	// broadcast the death event to everyone
	ent = G_TempEntity( pos, EV_VEHICLE_GIB );
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone
	ent->s.eventParm = 1;

	trap_UnlinkEntity( self );
	
}


/*QUAKED func_runway (0 .5 .8) ?
A bmodel that just sits there, doing nothing, for planes to land on
*/
void SP_func_runway( gentity_t *ent ) {
	trap_SetBrushModel( ent, ent->model );
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_EXPLOSIVE;
	ent->s.pos.trType = TR_STATIONARY;
	ent->clipmask = MASK_SOLID;
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	if (ent->health > 0) {
		ent->takedamage = qtrue;
		ent->die = explosive_die;
//		ent->s.ONOFF = OO_RADAR_GROUND;
	}
	if( !ent->count ) {
		ent->count = 100;
	}
	if( ent->team ) {
		if ( Q_stricmp( ent->team, "red" ) == 0 ) {
			ent->s.generic1 = TEAM_RED;
		} else if ( Q_stricmp( ent->team, "blue" ) == 0 ) {
			ent->s.generic1 = TEAM_BLUE;	
		}
	}
//	G_Printf( "runway: %d, %.1f %.1f %.1f - %.1f %.1f %.1f\n", ent->health, ent->r.mins[0],
//		ent->r.mins[1], ent->r.mins[2], ent->r.maxs[0], ent->r.maxs[1], ent->r.maxs[2] );

	trap_LinkEntity (ent);
}


/*
===============================================================================

EXPLOSIVE

===============================================================================
*/


/*QUAKED func_explosive (0 .5 .8) ?
A bmodel that just sits there, doing nothing, but can be shot.
health = well, health :-)
*/
void SP_func_explosive( gentity_t *ent ) {
	trap_SetBrushModel( ent, ent->model );
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_EXPLOSIVE;
	ent->s.pos.trType = TR_STATIONARY;
	ent->clipmask = MASK_SOLID;
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	ent->die = explosive_die;
	ent->takedamage = qtrue;
	ent->s.ONOFF = OO_RADAR_GROUND;
	if( ent->health <= 0 ) {
		ent->health = 200;
	}
	if( !ent->count ) {
		ent->count = 30;
	}
	if( ent->team ) {
		if ( Q_stricmp( ent->team, "red" ) == 0 ) {
			ent->s.generic1 = TEAM_RED;
		} else if ( Q_stricmp( ent->team, "blue" ) == 0 ) {
			ent->s.generic1 = TEAM_BLUE;	
		}
	}
	trap_LinkEntity (ent);
}




/*
===============================================================================

RECHARGE

===============================================================================
*/

void recharge_touch( gentity_t *self, gentity_t *other, trace_t *trace ) {
	int i;

	// only clients
	if( !other->client ) {
		return;
	}
	// no (q3)bots, flying oe dead stuff; speed and throttle have to be zero
	if( other->client->vehicle < 0 ||
		other->client->ps.speed > 0 ||
		other->client->ps.throttle != 0 ||
		other->health <= 0 || 
		!(other->ONOFF & OO_LANDED) ) {
		return;
	}
	
	// are we the proper category ?
	if( !((availableVehicles[other->client->vehicle].id&CAT_ANY)&self->ent_category) ) {
		return;
	}

	// can we already recharge ?
	if( level.time < other->rechargetime ) {
		return;
	}

	// is it the proper team ?
	if( g_gametype.integer >= GT_TEAM && self->s.generic1 != TEAM_FREE &&
		self->s.generic1 != other->client->ps.persistant[PERS_TEAM] ) {
		return;
	}

	// recharge health
	if( other->health < other->client->pers.maxHealth ) {
		other->health += (10*other->client->pers.maxHealth/100);
		if( other->health > other->client->pers.maxHealth ) {
			other->health = other->client->pers.maxHealth;
		}
		other->client->ps.stats[STAT_HEALTH] = other->health;
	}

	// recharge fuel
	if( other->client->ps.stats[STAT_FUEL] < other->client->ps.stats[STAT_MAX_FUEL] ) {
		other->client->ps.stats[STAT_FUEL] += (10*other->client->ps.stats[STAT_MAX_FUEL]/100);
		if( other->client->ps.stats[STAT_FUEL] > other->client->ps.stats[STAT_MAX_FUEL] ) {
			other->client->ps.stats[STAT_FUEL] = other->client->ps.stats[STAT_MAX_FUEL];
		}
	}
	// reload weapons
	for( i = WP_MACHINEGUN; i <= WP_FLARE; i++ ) {
		if( other->client->ps.ammo[i] < other->client->ps.ammo[i+8] ) {
			other->client->ps.ammo[i] += (other->client->ps.ammo[i+8]<=10 ? 1 : other->client->ps.ammo[i+8]/10);
			if( other->client->ps.ammo[i] > other->client->ps.ammo[i+8] ) 
				other->client->ps.ammo[i] = other->client->ps.ammo[i+8];
		}
	}

	other->rechargetime = level.time + ( g_gametype.integer >= GT_TEAM ? 1500 : 500);
}


/*QUAKED trigger_recharge (.5 .5 .5) ?
Will reload, repair and refuel vehicles
*/
void SP_trigger_recharge( gentity_t *self ) {

	if( !self->ent_category ) self->ent_category = CAT_ANY;
	else self->ent_category <<= 8;

	trap_SetBrushModel( self, self->model );
	self->clipmask = self->r.contents = CONTENTS_TRIGGER;
	self->r.svFlags = SVF_NOCLIENT;
	self->touch = recharge_touch;
	if( self->team ) {
		if ( Q_stricmp( self->team, "red" ) == 0 ) {
			self->s.generic1 = TEAM_RED;
		} else if ( Q_stricmp( self->team, "blue" ) == 0 ) {
			self->s.generic1 = TEAM_BLUE;	
		} else {
			self->s.generic1 = TEAM_FREE;
		}
	} else {
		self->s.generic1 = TEAM_FREE;
	}
	trap_LinkEntity (self);
}




/*
===============================================================================

RADIO

===============================================================================
*/




void radio_tower( gentity_t *self, gentity_t *other, trace_t *trace ) {
	if( !other->client ) {
		return;
	}

	if( other->client->vehicle < 0 ) {
		return;
	}

	// is it the proper team ?
	if( g_gametype.integer >= GT_TEAM && self->s.generic1 != TEAM_FREE && 
		self->s.generic1 != other->client->ps.persistant[PERS_TEAM] ) {
		return;
	}

	other->radio_target = self;
}

void radio_comply( gentity_t *self )
{
	G_UseTargets( self, NULL );
}


/*QUAKED trigger_radio (.5 .5 .5) ?
Used to trigger anything controlled
by the tower (elevators, catapults etc)
target - the entity triggered
message - the message to be displayed (radio comm)
*/
void SP_trigger_radio( gentity_t *self ) {
	trap_SetBrushModel( self, self->model );
	self->clipmask = self->r.contents = CONTENTS_TRIGGER;
	self->r.svFlags = SVF_NOCLIENT;
	self->touch = radio_tower;
	if( self->team ) {
		if ( Q_stricmp( self->team, "red" ) == 0 ) {
			self->s.generic1 = TEAM_RED;
		} else if ( Q_stricmp( self->team, "blue" ) == 0 ) {
			self->s.generic1 = TEAM_BLUE;	
		} else {
			self->s.generic1 = TEAM_FREE;
		}
	} else {
		self->s.generic1 = TEAM_FREE;
	}
	trap_LinkEntity (self);
}







/*
===============================================================================

  AI CONTROLLED STUFF

===============================================================================
*/




/*QUAKED ai_Radar (.5 .5 .5) ?
*/
void SP_ai_radar( gentity_t *ent ) {

	G_FreeEntity(ent);
}

/*QUAKED ai_SAM (.5 .5 .5) ?
*/
void SP_ai_sam( gentity_t *ent ) {

	G_FreeEntity(ent);
}

/*QUAKED ai_Flak (.5 .5 .5) ?
*/
void SP_ai_flak( gentity_t *ent ) {

	G_FreeEntity(ent);
}
