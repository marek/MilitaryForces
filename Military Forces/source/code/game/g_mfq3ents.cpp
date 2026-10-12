/*
 * $Id: g_mfq3ents.c,v 1.7 2006-01-29 14:03:41 thebjoern Exp $
*/


#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"
#include "g_mfq3ents.h"

/*
===============================================================================

RUNWAY

===============================================================================
*/

void Die_ExplosiveDie::execute( GameEntity *inflictor, GameEntity *attacker, int damage, int meansOfDeath )
{
	GameEntity* ent;
	Vec3	pos;
	pos = self_->r.absmin + self_->r.absmax;
	pos = pos * ( 0.5f );

	self_->r.svFlags = SVF_NOCLIENT;
	self_->s.eType = ET_INVISIBLE;

	if( g_gametype.integer >= GT_TEAM && self_->score_ && attacker && attacker->client_ ) 
	{
		int score;
		if( self_->s.generic1 == attacker->client_->ps_.persistant[PERS_TEAM] ) 
			score = -self_->score_;
		else
			score = self_->score_;

		AddScore( attacker, pos, score );
	}

	// broadcast the death event to everyone
	ent = Game::tempEntity( pos, EV_BUILDING_EXPLODE );
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone
	ent->s.eventParm = 1;

	if( ent->target_ ) 
	{
		GameEntity* t = nullptr;
		while( (t = Game::find (t, FOFS(targetname_), ent->target_)) != nullptr ) 
		{
			if ( t == ent ) 
			{
				Common::printf ("WARNING: Entity used itself.\n");
			} 
			else 
			{
				if( t->dieFunc_ ) 
					t->dieFunc_->execute(inflictor, attacker, damage, meansOfDeath);
			}
			if( !ent->inuse_ ) 
			{
				Common::printf("entity was removed while using targets\n");
				break;
			}
		}
	}

	GameHost::unlinkEntity( self_ );
	
}


/*QUAKED func_runway (0 .5 .8) ?
A bmodel that just sits there, doing nothing, for planes to land on
*/
void SP_func_runway( GameEntity *ent ) 
{
	GameHost::setBrushModel( ent, ent->model_ );
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_EXPLOSIVE;
	ent->s.pos.trType = TR_STATIONARY;
	ent->clipmask_ = MASK_SOLID;
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	if (ent->health_ > 0) 
	{
		ent->takedamage_ = true;
		ent->setDie( std::make_unique<Die_ExplosiveDie>());
//		ent->s.ONOFF = OO_RADAR_GROUND;
	}
	if( !ent->count_ ) 
		ent->count_ = 100;

	if( ent->team_ ) 
	{
		if ( Q_stricmp( ent->team_, "red" ) == 0 ) 
			ent->s.generic1 = ClientBase::TEAM_RED;
		else if ( Q_stricmp( ent->team_, "blue" ) == 0 ) 
			ent->s.generic1 = ClientBase::TEAM_BLUE;	
	}
//	Common::printf( "runway: %d, %.1f %.1f %.1f - %.1f %.1f %.1f\n", ent->health, ent->r.mins[0],
//		ent->r.mins[1], ent->r.mins[2], ent->r.maxs[0], ent->r.maxs[1], ent->r.maxs[2] );

	GameHost::linkEntity( ent );
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
void SP_func_explosive( GameEntity *ent ) 
{
	GameHost::setBrushModel( ent, ent->model_ );
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.eType = ET_EXPLOSIVE;
	ent->s.pos.trType = TR_STATIONARY;
	ent->clipmask_ = MASK_SOLID;
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	ent->setDie( std::make_unique<Die_ExplosiveDie>());
	ent->takedamage_ = true;
	ent->s.ONOFF = OO_RADAR_GROUND;
	if( ent->health_ <= 0 ) 
		ent->health_ = 200;
	if( !ent->count_ ) 
		ent->count_ = 30;
	if( ent->team_ ) 
	{
		if ( Q_stricmp( ent->team_, "red" ) == 0 ) 
			ent->s.generic1 = ClientBase::TEAM_RED;
		else if ( Q_stricmp( ent->team_, "blue" ) == 0 ) 
			ent->s.generic1 = ClientBase::TEAM_BLUE;	
	}
	GameHost::linkEntity( ent );
}




/*
===============================================================================

RECHARGE

===============================================================================
*/

void Touch_RechargeTouch::execute( GameEntity *other, trace_t *trace ) 
{
	// only clients
	if( !other->client_ ) 
		return;
	
	// are we the proper category ?
	if( !(availableVehicles[other->client_->vehicle_].cat & self_->ent_category_) ) 
		return;

	// can we already recharge ?
	if( theLevel.time_ < other->rechargetime_ ) 
		return;

	other->rechargetime_ = theLevel.time_ + ( g_gametype.integer >= GT_TEAM ? 1500 : 1000);

	// no (q3)bots, flying oe dead stuff; speed and throttle have to be zero
	if( other->client_->vehicle_ < 0 ||
		other->client_->ps_.speed > 0 ||
		other->client_->ps_.throttle != 0 ||
		other->health_ <= 0 || 
		!(other->ONOFF_ & OO_LANDED) ) 
	{
		return;
	}

	// is it the proper team ?
	if( g_gametype.integer >= GT_TEAM && self_->s.generic1 != ClientBase::TEAM_FREE &&
		self_->s.generic1 != other->client_->ps_.persistant[PERS_TEAM] ) 
	{
		return;
	}

	// recharge health
	if( other->health_ < other->client_->pers_.maxHealth_ ) 
	{
		other->health_ += (10*other->client_->pers_.maxHealth_/100);
		if( other->health_ > other->client_->pers_.maxHealth_ ) 
			other->health_ = other->client_->pers_.maxHealth_;
		other->client_->ps_.stats[STAT_HEALTH] = other->health_;
	}

	// recharge fuel
	if( other->client_->ps_.stats[STAT_FUEL] < other->client_->ps_.stats[STAT_MAX_FUEL] ) 
	{
		other->client_->ps_.stats[STAT_FUEL] += (10*other->client_->ps_.stats[STAT_MAX_FUEL]/100);
		if( other->client_->ps_.stats[STAT_FUEL] > other->client_->ps_.stats[STAT_MAX_FUEL] ) 
			other->client_->ps_.stats[STAT_FUEL] = other->client_->ps_.stats[STAT_MAX_FUEL];
	}
	for( int i = 1; i <= theLevel.maxclients_; i++ ) 
	{
		GameClient* rider = theLevel.getClient( i );
		if( !rider || rider->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED || rider->sess_.ridingClient_ != other->s.number || rider->vehicle_ < 0 ) 
			continue;
		const int weapon = availableVehicles[rider->vehicle_].weapons[WP_MACHINEGUN];
		if( weapon ) 
			rider->riderAmmo_ = availableVehicles[rider->vehicle_].ammo[WP_MACHINEGUN] * availableWeapons[weapon].numberPerPackage;
	}

	// reload weapons
	for( int i : allWeaponSlots ) 
	{
		if( other->client_->ps_.ammo[i] < other->client_->ps_.ammo[i+8] ) 
		{
//			int diff = other->client->ps.ammo[i+8] - other->client->ps.ammo[i];
			other->client_->ps_.ammo[i] = other->client_->ps_.ammo[i+8];
			VehicleData::addWeaponToLoadout( availableVehicles[other->client_->vehicle_].weapons[i], &other->loadout_, &other->client_->ps_ );
			break;
		}
	}

}


/*QUAKED trigger_recharge (.5 .5 .5) ?
Will reload, repair and refuel vehicles
*/
void SP_trigger_recharge( GameEntity *self ) 
{
	if( !self->ent_category_ ) 
		self->ent_category_ = CAT_ANY;
//	else self->ent_category <<= 8;

	GameHost::setBrushModel( self, self->model_ );
	self->clipmask_ = self->r.contents = CONTENTS_TRIGGER;
	self->r.svFlags = SVF_NOCLIENT;
	self->setTouch( std::make_unique<Touch_RechargeTouch>());
	if( self->team_ ) 
	{
		if ( Q_stricmp( self->team_, "red" ) == 0 ) 
			self->s.generic1 = ClientBase::TEAM_RED;
		else if ( Q_stricmp( self->team_, "blue" ) == 0 ) 
			self->s.generic1 = ClientBase::TEAM_BLUE;	
		else 
			self->s.generic1 = ClientBase::TEAM_FREE;
	} else {
		self->s.generic1 = ClientBase::TEAM_FREE;
	}
	GameHost::linkEntity( self );
}




/*
===============================================================================

RADIO

===============================================================================
*/




void Touch_RadioTower::execute( GameEntity *other, trace_t *trace ) 
{
	if( !other->client_ ) 
		return;

	if( other->client_->vehicle_ < 0 ) 
		return;

	// is it the proper team ?
	if( g_gametype.integer >= GT_TEAM && self_->s.generic1 != ClientBase::TEAM_FREE && 
		self_->s.generic1 != other->client_->ps_.persistant[PERS_TEAM] ) 
		return;

	other->radio_target_ = self_;
}

void Think_RadioComply::execute()
{
	Game::useTargets( self_, nullptr );
}


/*QUAKED trigger_radio (.5 .5 .5) ?
Used to trigger anything controlled
by the tower (elevators, catapults etc)
target - the entity triggered
message - the message to be displayed (radio comm)
*/
void SP_trigger_radio( GameEntity *self ) 
{
	GameHost::setBrushModel( self, self->model_ );
	self->clipmask_ = self->r.contents = CONTENTS_TRIGGER;
	self->r.svFlags = SVF_NOCLIENT;
	self->setTouch( std::make_unique<Touch_RadioTower>());
	if( self->team_ ) 
	{
		if ( Q_stricmp( self->team_, "red" ) == 0 ) 
			self->s.generic1 = ClientBase::TEAM_RED;
		else if ( Q_stricmp( self->team_, "blue" ) == 0 ) 
			self->s.generic1 = ClientBase::TEAM_BLUE;	
		else 
			self->s.generic1 = ClientBase::TEAM_FREE;
	} else {
		self->s.generic1 = ClientBase::TEAM_FREE;
	}
	GameHost::linkEntity( self );
}



/*
===============================================================================

  CARRIER STUFF

===============================================================================
*/


/*QUAKED func_catapult (.5 .5 .5) ?
*/
void SP_func_catapult( GameEntity *ent ) 
{
	ent->freeUp();//G_FreeEntity(ent);
}

/*QUAKED func_wires (.5 .5 .5) ?
*/
void SP_func_wires( GameEntity *ent ) 
{
	ent->freeUp();
}



/*
===============================================================================

  AI CONTROLLED STUFF

===============================================================================
*/




/*QUAKED ai_Radar (.5 .5 .5) ?
*/
void SP_ai_radar( GameEntity *ent ) 
{
	ent->freeUp();
}

/*QUAKED ai_SAM (.5 .5 .5) ?
*/
void SP_ai_sam( GameEntity *ent )
{
	ent->freeUp();
}

/*QUAKED ai_Flak (.5 .5 .5) ?
*/
void SP_ai_flak( GameEntity *ent ) 
{
	ent->freeUp();
}
