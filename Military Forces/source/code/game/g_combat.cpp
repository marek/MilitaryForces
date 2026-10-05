/*
 * $Id: g_combat.c,v 1.7 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_combat.c

#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"

/*
============
AddScore

Adds score to both the client (and his team)
============
*/
void AddScore( GameEntity* ent, vec3_t origin, int score )
{
	if( !ent->client_ )
		return;

	// no scoring during pre-match warmup
	if( theLevel.warmupTime_ )
		return;

	// add to player's personal score
	ent->client_->ps_.persistant[PERS_SCORE] += score;
	
	// if team-deathmatch, add player's scoring onto team score
	if( g_gametype.integer == GT_TEAM )
	{
		theLevel.teamScores_[ ent->client_->ps_.persistant[PERS_TEAM] ] += score;
	}

	// calculate rankings
	CalculateRanks();
}

/*
==================
ExplodeVehicle

Explode our vehicle
==================
*/
void ExplodeVehicle( GameEntity* self )
{
	// create the client vehicle explosion event
	Game::addEvent( self, EV_VEHICLE_GIB, self->client_ && self->client_->vehicle_ >= 0 && self->r.contents != CONTENTS_CORPSE &&
		(availableVehicles[self->client_->vehicle_].flags & VF_COOKOFF) ? 1 : 0, true );

	self->takedamage_ = false;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;

	if( self->client_ )
	{
		// no death counting during warmup
		if( !theLevel.warmupTime_ )
		{
			self->client_->ps_.persistant[PERS_DEATHS]++;
		}

		// enable vehicle selection
		self->client_->ps_.pm_flags |= PMF_VEHICLESELECT;
		self->client_->ps_.pm_type = PM_DEAD;

		// prevent early respawn (1.5 seconds lock-out)
		self->client_->respawnTime_ = theLevel.time_ + 1500;
	}
}





/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills everything
============
*/

void Game::damage( GameEntity *targ, GameEntity *inflictor, GameEntity* attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod, long cat ) 
{
	GameClient* client;
	int			take;
	int			save;
	int			knockback;
	int			max;

	if( targ->r.svFlags & SVF_DECK ) 
	{
		targ = theLevel.getEntity( targ->s.otherEntityNum );
		if( !targ || !targ->inuse_ ) 
			return;
	}

	if (!targ->takedamage_) 
		return;

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( theLevel.intermissionQueued_ ) 
		return;

	if ( !inflictor ) 
		inflictor = theLevel.getEntity(ENTITYNUM_WORLD);// &g_entities[ENTITYNUM_WORLD];

	if ( !attacker ) 
		attacker = theLevel.getEntity(ENTITYNUM_WORLD);//&g_entities[ENTITYNUM_WORLD];

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) 
	{
		if ( targ->moverState_ == GameEntity::MOVER_POS1 ) //&& targ->use ) 
			targ->useFunc_->execute( inflictor, attacker );
		return;
	}

	// runways and explosives
	// later define when damage is done (for example only by anti-ground weapons
	// or runway bombs etc)
	if( targ->s.eType == ET_EXPLOSIVE ) 
	{
		if( damage > targ->count_ ) 
		{
			targ->health_ -= damage;
			if( targ->health_ < 0 && targ->dieFunc_ )
				targ->dieFunc_->execute( inflictor, attacker, 0, 0 );
		}
		return;
	}

	// wrong cat ?
	if( cat && cat != CAT_ANY ) 
	{
		if( targ->client_ ) 
		{
			if( !(availableVehicles[targ->client_->vehicle_].cat & cat) ) 
				damage *= inflictor->catmodifier_;
		} 
		else 
		{
			if( targ->s.eType == ET_MISC_VEHICLE ) 
			{
				if( !(availableVehicles[targ->s.modelindex].cat & cat) )
					damage *= inflictor->catmodifier_;
			}
		}
	}

	// reduce damage by the attacker's handicap value
	// unless they are rocket jumping
	// MFQ3 not for vehicles
	if ( attacker->client_ && attacker != targ && attacker->client_->vehicle_ < 0 )
	{
		max = attacker->client_->ps_.stats[STAT_MAX_HEALTH];
		damage = damage * max / 100;
	}

	client = targ->client_;

	if ( client ) 
	{
		if ( client->noclip_ )
			return;
	}

	if ( !dir )
		dflags |= DAMAGE_NO_KNOCKBACK;
	else
		VectorNormalize(dir);

	knockback = damage;
	if ( knockback > 200 ) 
		knockback = 200;

	if ( targ->flags_ & FL_NO_KNOCKBACK ) 
		knockback = 0;

	if ( dflags & DAMAGE_NO_KNOCKBACK ) 
		knockback = 0;

	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client_ )
	{
		Vec3	kvel;
		float	mass;

		mass = 200;

		VectorScale (dir, g_knockback.value * (float)knockback / mass, kvel);
		VectorAdd (targ->client_->ps_.velocity, kvel, targ->client_->ps_.velocity);

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client_->ps_.pm_time ) 
		{
			int		t;

			t = knockback * 2;
			if ( t < 50 ) 
				t = 50;
	
			if ( t > 200 ) 
				t = 200;

			targ->client_->ps_.pm_time = t;
			targ->client_->ps_.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) 
	{

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
		if ( targ != attacker && OnSameTeam (targ, attacker)  ) 
		{
			if ( !g_friendlyFire.integer ) 
				return;
		}
		// check for godmode
		if ( targ->flags_ & FL_GODMODE ) 
			return;
	}

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	if ( attacker->client_ && targ != attacker && targ->health_ > 0
			&& targ->s.eType != ET_MISSILE
			&& targ->s.eType != ET_BULLET
			&& targ->s.eType != ET_GENERAL) 
	{
		if ( OnSameTeam( targ, attacker ) ) 
			attacker->client_->ps_.persistant[PERS_HITS]--;
		else
			attacker->client_->ps_.persistant[PERS_HITS]++;
	}

	// always give half damage if hurting self
	// calculated after knockback, so rocket jumping works
	if ( targ == attacker ) 
		damage *= 0.5;

	if ( damage < 1 ) 
		damage = 1;

	take = damage;
	save = 0;

	if ( g_debugDamage.integer ) 
	{
		Common::printf( "%i: client:%i health:%i damage:%i\n", theLevel.time_, targ->s.number,
			targ->health_, take );
	}

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( client ) 
	{
		if ( attacker ) 
			client->ps_.persistant[PERS_ATTACKER] = attacker->s.number;
		else 
			client->ps_.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;

		client->damage_done_ += take;
		client->damage_knockback_ += knockback;
		if ( dir ) 
		{
			VectorCopy ( dir, client->damage_from_ );
			client->damage_fromWorld_ = false;
		}
		else 
		{
			client->damage_from_ = targ->r.currentOrigin;
			client->damage_fromWorld_ = true;
		}
	}

	if (targ->client_)
	{
		// set the last client who damaged the target
		targ->client_->lasthurt_client_ = attacker->s.number;
		targ->client_->lasthurt_mod_ = mod;
		if( attacker != targ && attacker->client_ && !OnSameTeam( targ, attacker ) )
		{
			targ->client_->lastEnemyHitTime_ = theLevel.time_;
			// attacked again on foot after bailing out still counts as under fire
			if( targ->client_->vehicle_ >= 0 && (availableVehicles[targ->client_->vehicle_].cat & CAT_LQM) && targ->client_->ejectedTime_ )
				targ->client_->downedUnderFire_ = true;
		}
		if( attacker->client_ && attacker != targ && attacker->client_->sess_.paintTeam_ && 
			attacker->client_->sess_.paintTeam_ == targ->client_->sess_.sessionTeam_ ) 
		{
			attacker->client_->ruseSeenTime_ = theLevel.time_;
			if( !attacker->client_->ruseBlown_ ) 
			{
				attacker->client_->ruseBlown_ = true;
				ClientUserinfoChanged( attacker->s.number );
			}
		}
	}

	// do the damage
	if (take) 
	{
		targ->health_ = targ->health_ - take;
		if ( targ->client_ ) 
			targ->client_->ps_.stats[STAT_HEALTH] = targ->health_;
			
		if ( targ->health_ <= 0 )
		{
			if ( client )
				targ->flags_ |= FL_NO_KNOCKBACK;

			if( targ->health_ < -999)
				targ->health_ = -999;

			targ->enemy_ = attacker;
			if( targ->dieFunc_ )
				targ->dieFunc_->execute( inflictor, attacker, take, mod );
			return;
		}
		else if( targ->painFunc_ )
			targ->painFunc_->execute( attacker, take );
	}
}


/*
============
CanDamage

Returns true if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
bool CanDamage( GameEntity* targ, vec3_t origin ) 
{
	Vec3	dest;
	trace_t	tr;
	Vec3	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	midpoint = targ->r.absmin + targ->r.absmax;
	midpoint = midpoint * ( 0.5 );

	dest = midpoint;
	GameHost::trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, false);
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return true;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	dest = midpoint;
	dest[0] += 15.0;
	dest[1] += 15.0;
	GameHost::trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, false);
	if (tr.fraction == 1.0)
		return true;

	dest = midpoint;
	dest[0] += 15.0;
	dest[1] -= 15.0;
	GameHost::trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, false);
	if (tr.fraction == 1.0)
		return true;

	dest = midpoint;
	dest[0] -= 15.0;
	dest[1] += 15.0;
	GameHost::trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, false);
	if (tr.fraction == 1.0)
		return true;

	dest = midpoint;
	dest[0] -= 15.0;
	dest[1] -= 15.0;
	GameHost::trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID, false);
	if (tr.fraction == 1.0)
		return true;


	return false;
}


/*
============
Game::radiusDamage
============
*/
bool Game::radiusDamage ( vec3_t origin, GameEntity* attacker, float damage, float radius,
					  GameEntity* ignore, int mod, long cat) {
	float		points, dist;
	GameEntity* ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	Vec3		mins, maxs;
	Vec3		v;
	Vec3		dir;
	int			i, e;
	bool		hitClient = false;

	if ( radius < 1 ) 
		radius = 1;

	for ( i = 0 ; i < 3 ; i++ ) 
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = GameHost::areaEntities( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) 
	{
		ent = theLevel.getEntity(entityList[e]);// &g_entities[entityList[ e ]];

		if( !ent || ent == ignore )
			continue;
		//if (!ent->takedamage)
		//	continue;

		// find the distance from the edge of the bounding box
		for ( i = 0 ; i < 3 ; i++ ) 
		{
			if ( origin[i] < ent->r.absmin[i] ) 
				v[i] = ent->r.absmin[i] - origin[i];
			else if ( origin[i] > ent->r.absmax[i] ) 
				v[i] = origin[i] - ent->r.absmax[i];
			else 
				v[i] = 0;
		}

		dist = VectorLength( v );
		if ( dist >= radius ) 
			continue;

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage (ent, origin) )
		{
			if( LogAccuracyHit( ent, attacker ) ) 
				hitClient = true;

			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			Game::damage (ent, nullptr, attacker, dir, origin, static_cast<int>(points), DAMAGE_RADIUS, mod, cat);
		}
	}

	return hitClient;
}

/*
================== 
TossVehicleFlags (MFQ3)
==================
*/

void TossVehicleFlags( GameEntity *self ) 
{
	gitem_t*	item;
	float		angle;
	int			i;
	GameEntity*	drop;

	// drop all the objectives if not in teamplay
	if ( g_gametype.integer != GT_TEAM ) 
	{
		angle = 45;
		for ( i = OB_REDFLAG; i <= OB_BLUEFLAG ; i<<=1 ) 
		{
			if ( self->client_->ps_.objectives & i ) 
			{
				item = GameShared::findItemForPowerup( static_cast<objective_t>(i) );
				if ( !item ) 
					continue;
				drop = Drop_Item( self, item, angle );
				angle += 45;
			}
		}
	}
}


