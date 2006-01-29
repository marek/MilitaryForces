#include "g_vehicle.h"
#include "g_local.h"
#include "g_level.h"


// these are just for logging, the client prints its own messages
char * modNames[] = {
	"MOD_UNKNOWN",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	// now MFQ3 (not grappling was removed!)
	"MOD_FFAR",
	"MOD_FFAR_SPLASH",
	"MOD_IRONBOMB",
	"MOD_IRONBOMB_SPLASH",
	"MOD_AUTOCANNON",
	"MOD_MAINGUN",
	"MOD_VEHICLEEXPLOSION",
	"MOD_CRASH"
};

void 
VehicleDeathImpl( GameEntity* self, GameEntity* inflictor, GameEntity* attacker, int damage, int meansOfDeath )
{
	GameEntity* ent;
	int			contents;
	int			killer;
	int			i;
	char		*killerName, *obit;
		
	if ( self->client_->ps_.pm_type == PM_DEAD ) 
		return;

	if ( theLevel.intermissiontime_ ) 
		return;

	if ( attacker ) 
	{
		killer = attacker->s.number;
		if ( attacker->client_ ) 
			killerName = attacker->client_->pers_.netname_;
		else
			killerName = "<non-client>";
	} 
	else 
	{
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( killer < 1 || killer > MAX_CLIENTS ) 
	{
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	if ( meansOfDeath < 0 || meansOfDeath >= sizeof( modNames ) / sizeof( modNames[0] ) ) 
		obit = "<bad obituary>";
	else 
		obit = modNames[ meansOfDeath ];

	G_LogPrintf( "Kill: %i %i %i: %s killed %s by %s\n", killer, self->s.number, meansOfDeath, killerName, 
				 self->client_->pers_.netname_, obit );

	// broadcast the death event to everyone
	ent = G_TempEntity( self->r.currentOrigin, EV_OBITUARY );
	ent->s.eventParm = meansOfDeath;
	ent->s.otherEntityNum = self->s.number;
	ent->s.otherEntityNum2 = killer;
	ent->r.svFlags = SVF_BROADCAST;	// send to everyone

	self->enemy_ = attacker;

	self->client_->ps_.persistant[PERS_KILLED]++;
	
	if( attacker && attacker->client_ )
	{
		attacker->client_->lastkilled_client_ = self->s.number;

		if ( attacker == self || OnSameTeam (self, attacker ) )
		{
			AddScore( attacker, self->r.currentOrigin, -1 );
		}
		else
		{
			if( attacker->r.svFlags & SVF_BOT )
			{
				AddScore( attacker, self->r.currentOrigin, 1 );
			}
			else
			{
				if( meansOfDeath != MOD_VEHICLEEXPLOSION && 
					meansOfDeath != MOD_CRASH )
				{
					AddScore( attacker, self->r.currentOrigin, 1 );
				}
				// Easy way to prevent enemy from lossing score in collision as well cause he kills "himself"
				else
				{
					AddScore( attacker, self->r.currentOrigin, 1 );
				}

			}

			attacker->client_->lastKillTime_ = theLevel.time_;
		}
	}
	else
	{
		AddScore( self, self->r.currentOrigin, -1 );
	}

	// add team bonuses
	Team_FragBonuses(self, inflictor, attacker);

	// if I committed suicide, the flag does not fall, it returns.
	if (meansOfDeath == MOD_SUICIDE) 
	{
		if ( self->client_->ps_.objectives & OB_REDFLAG ) 
		{		
			// only happens in standard CTF
			Team_ReturnFlag( ClientBase::TEAM_RED );
			self->client_->ps_.objectives &= ~OB_REDFLAG;
		}
		else if ( self->client_->ps_.objectives & OB_BLUEFLAG ) 
		{	
			// only happens in standard CTF
			Team_ReturnFlag( ClientBase::TEAM_BLUE );
			self->client_->ps_.objectives &= ~OB_BLUEFLAG;
		}
	}

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	contents = SV_PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP )) 
	{
		if( g_gametype.integer == GT_CTF ) 
			TossVehicleFlags( self );
	} 
	else 
	{
		if ( contents & CONTENTS_NODROP ) 
		{
			if ( self->client_->ps_.objectives & OB_REDFLAG ) 		// only happens in standard CTF
				Team_ReturnFlag( ClientBase::TEAM_RED );
			else if ( self->client_->ps_.objectives & OB_BLUEFLAG ) 	// only happens in standard CTF
				Team_ReturnFlag( ClientBase::TEAM_BLUE );
		}
	}

	// update scores
	Cmd_Score_f( self );
	
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for( i = 1 ; i <= theLevel.maxclients_ ; i++ )
	{
		GameClient* client = theLevel.getClient(i);// &level.clients[i];

		if( !client || client->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
			continue;

		if( client->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
			continue;

		if( client->sess_.spectatorClient_ == self->s.number ) 
			Cmd_Score_f( theLevel.getEntity(i) );// g_entities + i );
	}

	self->takedamage_ = true;	// can still be gibbed
	self->r.contents = CONTENTS_CORPSE;
	self->s.objectives = 0;
	self->s.loopSound = 0;
	self->r.maxs[2] = -8;

	// remove powerups
/*
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );
*/

	if( (self->ONOFF_ & OO_LANDED) && self->health_ > GIB_HEALTH ) 
		self->health_ = GIB_HEALTH - 1;

	// always gib ground vehicles and lqms
	if( availableVehicles[self->client_->vehicle_].cat & CAT_GROUND ) 
		self->health_ = GIB_HEALTH - 1;

	// Adjust gib for LQM's
	if( self->health_ < 0 && self->health_ > GIB_HEALTH+20 )
		self->health_ = GIB_HEALTH+5;

	// to gib or not to gib? 
	if ( self->health_ <= GIB_HEALTH ) /* && !(contents & CONTENTS_NODROP) ) // never gib in a nodrop */
	{
		// create vehicle explosion
		ExplodeVehicle( self );

		// create radius explosion damage
		G_RadiusDamage( self->r.currentOrigin, self, 150, 150, self, MOD_VEHICLEEXPLOSION, CAT_ANY );

	}
	else if ( availableVehicles[self->client_->vehicle_].cat & CAT_LQM ) 
	{
		self->setDie(new VehicleBase::VehicleDie);
		self->client_->respawnTime_ = theLevel.time_ + 3000;
	}
	else
	{
		// create a smaller vehicle explosion
		G_AddEvent( self, EV_VEHICLE_DIE, 0, true );

		// wreck can be blown up (i.e on crash into ground)
		self->setDie(new VehicleBase::VehicleDie);

		// prevent early respawning during death anim (allows 30 seconds)
		self->client_->respawnTime_ = theLevel.time_ + 30000;
	}

	SV_LinkEntity( self );
}

void 
VehicleBase::VehicleDeath::execute( GameEntity* inflictor, GameEntity* attacker, int damage, int meansOfDeath )
{
	VehicleDeathImpl( self_, inflictor, attacker, damage, meansOfDeath );
}

void
VehicleBase::VehicleDie::execute( GameEntity* inflictor, GameEntity* attacker, int damage, int meansOfDeath )
{
	if( self_->health_ > GIB_HEALTH )
		return;
	ExplodeVehicle( self_ );
}
