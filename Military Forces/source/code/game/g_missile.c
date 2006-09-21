/*
 * $Id: 
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include "g_level.h"

#define	MISSILE_PRESTEP_TIME	50

///*
//================
//G_BounceMissile
//
//================
//*/
//void G_BounceMissile( GameEntity *ent, trace_t *trace ) 
//{
//	vec3_t	velocity;
//	float	dot;
//	int		hitTime;
//
//	// reflect the velocity on the trace plane
//	hitTime = theLevel.previousTime_ + ( theLevel.time_ - theLevel.previousTime_ ) * trace->fraction;
//	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
//	dot = DotProduct( velocity, trace->plane.normal );
//	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );
//
//	if ( ent->s.eFlags & EF_BOUNCE_HALF ) {
//		VectorScale( ent->s.pos.trDelta, 0.65, ent->s.pos.trDelta );
//		// check for stop
//		if ( trace->plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40 ) {
//			G_SetOrigin( ent, trace->endpos );
//			return;
//		}
//	}
//
//	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
//	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
//	ent->s.pos.trTime = theLevel.time_;
//}


/*
================
G_ExplodeMissile

Explode a missile without an impact
================
*/
struct Think_ExplodeMissile : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		vec3_t		dir;
		vec3_t		origin;

		// update target
		if( self_->tracktarget_ && self_->tracktarget_->client_ ) 
			self_->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;

		BG_EvaluateTrajectory( &self_->s.pos, theLevel.time_, origin );
		SnapVector( origin );
		G_SetOrigin( self_, origin );

		// we don't have a valid direction, so just point straight up
		dir[0] = dir[1] = 0;
		dir[2] = 1;

		self_->s.eType = ET_GENERAL;
		G_AddEvent( self_, EV_MISSILE_MISS, DirToByte( dir ), true );

		self_->freeAfterEvent_ = true;

		// splash damage
		if ( self_->splashDamage_ ) 
		{
			if( G_RadiusDamage( self_->r.currentOrigin, self_->parent_, self_->splashDamage_, self_->splashRadius_, 
								self_, self_->splashMethodOfDeath_, self_->targetcat_ ) )
			{
				//g_entities[ent->r.ownerNum].client->accuracy_hits++;
				theLevel.getEntity(self_->r.ownerNum)->client_->accuracy_hits_++;
			}
		}

		SV_LinkEntity( self_ );
	}
};



/*
================
G_ExplodeFlak

Explode a flak at destination
================
*/
struct Think_ExplodeFlak : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		vec3_t		dir;
		vec3_t		origin;

		BG_EvaluateTrajectory( &self_->s.pos, theLevel.time_, origin );
		SnapVector( origin );
		G_SetOrigin( self_, origin );

		// we don't have a valid direction, so just point straight up
		dir[0] = dir[1] = 0;
		dir[2] = 1;

		self_->s.eType = ET_GENERAL;
		G_AddEvent( self_, EV_FLAK, DirToByte( dir ), true );

		self_->freeAfterEvent_ = true;

		// splash damage
		if ( self_->splashDamage_ ) 
		{
			if( G_RadiusDamage( self_->r.currentOrigin, self_->parent_, self_->splashDamage_, self_->splashRadius_, 
								self_, self_->splashMethodOfDeath_, self_->targetcat_ ) ) 
			{
				//g_entities[ent->r.ownerNum].client->accuracy_hits++;
				theLevel.getEntity(self_->r.ownerNum)->client_->accuracy_hits_++;
			}
		}
		SV_LinkEntity( self_ );
	}
};

/*
================
G_ExplodeCFlare
spawns other flares
================
*/
struct Think_ExplodeCFlare : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		vec3_t		dir;
		vec3_t		origin;
		static int	seed = 0x92;

		// update target
		if( self_->tracktarget_ && self_->tracktarget_->client_) 
			self_->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;

		BG_EvaluateTrajectory( &self_->s.pos, theLevel.time_, origin );
		SnapVector( origin );
		G_SetOrigin( self_, origin );

		// we don't have a valid direction, so just point straight up
		dir[0] = dir[1] = 0;
		dir[2] = 1;

		self_->s.eType = ET_GENERAL;
		G_AddEvent( self_, EV_MISSILE_MISS, DirToByte( dir ), true );

		self_->freeAfterEvent_ = true;

		// splash damage
		if ( self_->splashDamage_ ) 
		{
			if( G_RadiusDamage( self_->r.currentOrigin, self_->parent_, self_->splashDamage_, self_->splashRadius_, 
								self_, self_->splashMethodOfDeath_, self_->targetcat_ ) ) 
			{
				//g_entities[ent->r.ownerNum].client->accuracy_hits++;
				theLevel.getEntity(self_->r.ownerNum)->client_->accuracy_hits_++;
			}
		}
		
		// Launch flares
		VectorSet(dir, 70 * Q_random(&seed),	70 * Q_random(&seed),		80 * Q_random(&seed));
		fire_flare2(self_->parent_, self_->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );
		VectorSet(dir, -70  * Q_random(&seed),	70 * Q_random(&seed),		80 * Q_random(&seed));
		fire_flare2(self_->parent_, self_->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );
		VectorSet(dir, 70 * Q_random(&seed),	-70 * Q_random(&seed),		80 * Q_random(&seed));
		fire_flare2(self_->parent_, self_->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );
		VectorSet(dir, -70 * Q_random(&seed),	-70  * Q_random(&seed),		80 * Q_random(&seed));
		fire_flare2(self_->parent_, self_->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );
		VectorSet(dir, 0 + 5 * Q_random(&seed),	0 + 5 * Q_random(&seed),	120 * Q_random(&seed));
		fire_flare2(self_->parent_, self_->r.currentOrigin, dir, 2000 + 1000 * Q_random(&seed) );

		SV_LinkEntity( self_ );
	}
};

/*
===============
NukeRadiusDamage
===============
*/
static void NukeRadiusDamage( vec3_t origin, GameEntity *attacker, float damage, float radius ) 
{
	float		dist;
	GameEntity	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;
	trace_t		trace, trace2;
	vec3_t		temp;

	if ( radius < 1 ) 
		radius = 1;

	for ( i = 0 ; i < 3 ; i++ ) 
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = SV_AreaEntities( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) 
	{
		ent = theLevel.getEntity(entityList[e]);//&g_entities[entityList[ e ]];

		if (!ent->takedamage_) 
			continue;

		// dont hit things we have already hit
		if( ent->nukeTime_ > theLevel.time_ ) 
			continue;

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
		
		// Do trace only for players only, save cpu
		if( strcmp(ent->classname_, "player") == 0 )
		{
			// Try a trace
			SV_Trace (&trace, origin, NULL, NULL,  ent->r.currentOrigin, ent->s.number, MASK_SOLID, false);
		
			// Also don't let them get away to easily so check slightly above the origin, and see if it reaches) It may have to be a big hill.
			VectorCopy(origin, temp);
			temp[2] += radius / 2;
			SV_Trace (&trace2, temp, NULL, NULL, ent->r.currentOrigin, ent->s.number, MASK_SOLID, false);
		}
		else
		{
			trace.fraction = trace2.fraction = 1.0;
		}
		
		// If it didn't hit anything, do damage, cause nothings in the way, but only players get off so easily >:D
		if( trace.fraction == 1.0 || trace2.fraction == 1.0 )
		{
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			// push the center of mass higher than the origin so players
			// get knocked into the air more
			dir[2] += 24;
			//G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_NUKE, CAT_ANY );
			
			G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS, MOD_NUKE, CAT_ANY );
			ent->nukeTime_ = theLevel.time_ + 3000;
		}
	}
}

/*
===============
NukeShockWave
===============
*/
static void NukeShockWave( vec3_t origin, GameEntity *attacker, float damage, float push, float radius ) 
{
	float		dist;
	GameEntity	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		v;
	vec3_t		dir;
	int			i, e;

	if ( radius < 1 )
		radius = 1;

	for ( i = 0 ; i < 3 ; i++ )
	{
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = SV_AreaEntities( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) 
	{
		ent = theLevel.getEntity(entityList[e]);// &g_entities[entityList[ e ]];

		// dont hit things we have already hit
		if( ent->nukeShockTime_ > theLevel.time_ ) 
			continue;

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

//		if( CanDamage (ent, origin) ) {
			VectorSubtract (ent->r.currentOrigin, origin, dir);
			dir[2] += 24;
		//	G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS|DAMAGE_NO_TEAM_PROTECTION, MOD_NUKE, CAT_ANY );
			G_Damage( ent, NULL, attacker, dir, origin, damage, DAMAGE_RADIUS, MOD_NUKE, CAT_ANY );
			//
			dir[2] = 0;
			VectorNormalize(dir);
			if ( ent->client_ ) 
			{
				ent->client_->ps_.velocity[0] = dir[0] * push;
				ent->client_->ps_.velocity[1] = dir[1] * push;
				ent->client_->ps_.velocity[2] = 100;
			}
			ent->nukeShockTime_ = theLevel.time_ + 3000;
//		}
	}
}

/*
===============
NukeDamage
===============
*/
struct Think_NukeDamage : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		int i;
		float t;
		GameEntity *ent;
		vec3_t newangles;

		self_->count_ += 100;

		if( self_->count_ >= NUKE_SHOCKWAVE_STARTTIME ) 
		{
			// shockwave push back
			t = self_->count_ - NUKE_SHOCKWAVE_STARTTIME;
			NukeShockWave(self_->s.pos.trBase, self_->activator_, 25, 50,	(int) (float) t * self_->splashRadius_ / (NUKE_SHOCKWAVE_ENDTIME - NUKE_SHOCKWAVE_STARTTIME) );
		}
		//
		if( self_->count_ >= NUKE_EXPLODE_STARTTIME ) 
		{
			// do our damage
			t = self_->count_ - NUKE_EXPLODE_STARTTIME;
			NukeRadiusDamage( self_->s.pos.trBase, self_->activator_, 700,	(int) (float) t * self_->splashRadius_ / (NUKE_IMPLODE_STARTTIME - NUKE_EXPLODE_STARTTIME) );
		}

		// either cycle or kill self
		if( self_->count_ >= NUKE_SHOCKWAVE_ENDTIME ) 
		{
			self_->freeUp();
			return;
		}
		self_->nextthink_ = theLevel.time_ + 100;
		
		// add earth quake effect
		newangles[0] = crandom() * 2;
		newangles[1] = crandom() * 2;
		newangles[2] = 0;
		for (i = 1; i <= MAX_CLIENTS; i++)
		{
			ent = theLevel.getEntity(i);// &g_entities[i];
			if( !ent || !ent->inuse_)
				continue;
			if( !ent->client_)
				continue;

			if( ent->client_->ps_.groundEntityNum != ENTITYNUM_NONE )
			{
				ent->client_->ps_.velocity[0] += crandom() * 120;
				ent->client_->ps_.velocity[1] += crandom() * 120;
				ent->client_->ps_.velocity[2] = 30 + random() * 25;
			}

			ent->client_->ps_.delta_angles[0] += ANGLE2SHORT(newangles[0] - self_->movedir_[0]);
			ent->client_->ps_.delta_angles[1] += ANGLE2SHORT(newangles[1] - self_->movedir_[1]);
			ent->client_->ps_.delta_angles[2] += ANGLE2SHORT(newangles[2] - self_->movedir_[2]);

		}
		VectorCopy(newangles, self_->movedir_);
	}
};

/*
================
G_burningManExplode
================
*/
void G_burningManExplode( GameEntity *ent ) 
{
	vec3_t		start, dir;
	int i;
	GameEntity	*bolt;
	static int	seed = 0x92;


	VectorCopy(ent->r.currentOrigin, start);
	// we don't have a valid direction, so just point straight up
	dir[0] = dir[1] = 0;
	dir[2] = 1;


	for(i = 0; i < 75; i++)
	{
		VectorSet(dir, (30 - (-30) + 1) * Q_random(&seed) + -30,	(30 - (-30) + 1) * Q_random(&seed) + -30,		40 * Q_random(&seed));
	
		bolt = theLevel.spawnEntity();
		bolt->classname_ = "burningman";
		bolt->nextthink_ = theLevel.time_ + 10000 + 4000 * Q_random(&seed);
		bolt->setThink(new Think_ExplodeMissile);// think = G_ExplodeMissile;
		bolt->s.eType = ET_MISSILE;
		bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
		bolt->s.weaponIndex = WI_BURNINGMAN;
		
		// bolt->parent = self;	// No parent
	//	bolt->r.ownerNum = self->s.number;

		bolt->damage_ = bolt->splashDamage_ = 0;
		bolt->splashRadius_ = 0;
		bolt->clipmask_ = MASK_SHOT;
		bolt->ONOFF_ = 1;// not used

		bolt->s.pos.trType = TR_GRAVITY_10;
		bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
		VectorCopy( start, bolt->s.pos.trBase );
		VectorScale( dir, 15, bolt->s.pos.trDelta );
		SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
		VectorCopy (start, bolt->r.currentOrigin);
	}
}


/*
================
G_CrateDropItems
================
*/
void G_CrateDropItems(GameEntity *ent)
{
	gitem_t		*item;
	bool		found = false;
	char		*classname[5];
	int			items = 0;
	int			i;

	switch(availableWeapons[ent->s.weaponIndex].type)
	{
		case WT_FUELCRATE:
			switch(availableWeapons[ent->s.weaponIndex].damage)
			{
				case 1:
				default:
					classname[0] = "Some Fuel";
					break;
				case 2:
					classname[0] = "More Fuel";
					break;
			}
			items = 1;
			break;
		case WT_AMMOCRATE:
			classname[0] = "Shells";
			classname[1] = "Bullets";
			classname[2] = "Rockets";
			classname[3] = "Slugs";
			items = 4;
			break;
		case WT_HEALTHCRATE:
			switch(availableWeapons[ent->s.weaponIndex].damage)
			{
				case 5:
				default:
					classname[0] = "5 Health";
					break;
				case 25:
					classname[0] = "25 Health";
					break;
				case 50:
					classname[0] = "50 Health";
					break;
			}
			items = 1;
			break;
	}

	for(i = 0; i < items; i++)
	{
		found = false;
		// find the item 
		for ( item = bg_itemlist + 1; item->classname; item++) {
			if ( strcmp(item->pickup_name,classname[i]) == 0  )  {
				found = true;
				break;
			}
		}
		// spawn the item
		if(found) 
		{
			ent->s.pos.trBase[2] += 25;
			Drop_Item( ent, item, 180*sin(crandom()));
			ent->s.pos.trBase[2] -= 25;

		}
	}



}

/*
================
G_ExplodeNuke
================
*/
struct Think_ExplodeNuke : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		GameEntity	*explosion;
		GameEntity	*te;
		vec3_t		snapped;
		static int	seed = 0x94;
		int rndnum;

		// start up the explosion logic
		explosion = theLevel.spawnEntity();

		explosion->s.eType = ET_EVENTS + EV_NUKE;
		explosion->eventTime_ = theLevel.time_;

		VectorCopy( self_->r.currentOrigin, snapped );

		SnapVector( snapped );		// save network bandwidth
		G_SetOrigin( explosion, snapped );

		explosion->classname_ = "nuke";
		explosion->s.pos.trType = TR_STATIONARY;

		explosion->nukeTime_ = theLevel.time_;

		explosion->setThink(new Think_NukeDamage);//think = NukeDamage;
		explosion->nextthink_ = theLevel.time_ + 100;
		explosion->count_ = 0;
		explosion->s.weaponIndex = self_->s.weaponIndex;
		explosion->splashRadius_ = self_->splashRadius_;

		VectorClear(explosion->movedir_);

		SV_LinkEntity( explosion );

		explosion->activator_ = self_->parent_;		// Activator is the owner of the bomb/missile 

		self_->freeAfterEvent_ = true;

		// Easter egg
		rndnum = (35 - (1) + 1) * Q_random(&seed) + 1;
		if(rndnum == 2)	// Magic number supplied by Cannon ¬_¬
		{
			G_burningManExplode(self_);
		}

		// play global sound at all clients
		te = G_TempEntity(snapped, EV_GLOBAL_TEAM_SOUND );
		te->r.svFlags |= SVF_BROADCAST;
		te->s.eventParm = GTS_NUKE;
	}
};

/*
================
G_MissileImpact
================
*/
void G_MissileImpact( GameEntity *ent, trace_t *trace ) 
{
	GameEntity* other;
	bool		hitClient = false;

	other = theLevel.getEntity(trace->entityNum);// &g_entities[trace->entityNum];

	if( !other ) 
		return;

	// check for bounce
//	if ( !other->takedamage &&
//		( ent->s.eFlags & ( EF_BOUNCE | EF_BOUNCE_HALF ) ) ) {
//		G_BounceMissile( ent, trace );
//		return;
//	}

	// update target
	if( ent->tracktarget_ && ent->tracktarget_->client_ ) 
		ent->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;

	// impact damage
	if (other->takedamage_) 
	{
		// FIXME: wrong damage direction?
		if ( ent->damage_ ) 
		{
			vec3_t	velocity;
			int		actualdamage;

			if( LogAccuracyHit( other, theLevel.getEntity(ent->r.ownerNum) ) )// &g_entities[ent->r.ownerNum] ) ) 
			{
				theLevel.getEntity(ent->r.ownerNum)->client_->accuracy_hits_++;
				//g_entities[ent->r.ownerNum].client->accuracy_hits++;
				hitClient = true;
			}
			BG_EvaluateTrajectoryDelta( &ent->s.pos, theLevel.time_, velocity );
			if ( VectorLength( velocity ) == 0 ) 
				velocity[2] = 1;

			actualdamage = ent->damage_ + ((rand() % (ent->damage_/2)) - (ent->damage_/4));
			G_Damage( other, 
					  ent, 
					  theLevel.getEntity(ent->r.ownerNum),//&g_entities[ent->r.ownerNum], 
					  velocity,
					  ent->s.origin, 
					  actualdamage, 
					  0, 
					  ent->methodOfDeath_, 
					  ent->targetcat_);
		}
	}

	// is it cheaper in bandwidth to just remove this ent and create a New
	// one, rather than changing the missile into the explosion?

	if( availableWeapons[ent->s.weaponIndex].type != WT_FUELCRATE && 
		availableWeapons[ent->s.weaponIndex].type != WT_AMMOCRATE && 
		availableWeapons[ent->s.weaponIndex].type != WT_HEALTHCRATE ) 
	{
		if ( other->takedamage_ && other->client_ ) 
		{
			G_AddEvent( ent, EV_MISSILE_HIT, DirToByte( trace->plane.normal ), true );
			ent->s.otherEntityNum = other->s.number;
		} 
		else if( trace->surfaceFlags & SURF_METALSTEPS ) 
			G_AddEvent( ent, EV_MISSILE_MISS_METAL, DirToByte( trace->plane.normal ), true );
		else 
			G_AddEvent( ent, EV_MISSILE_MISS, DirToByte( trace->plane.normal ), true );
	}

	ent->freeAfterEvent_ = true;

	// change over to a normal entity right at the point of impact
	ent->s.eType = ET_GENERAL;

	SnapVectorTowards( trace->endpos, ent->s.pos.trBase );	// save net bandwidth

	G_SetOrigin( ent, trace->endpos );

	// Nuke specifics to execute its detonation anyways
	if( availableWeapons[ent->s.weaponIndex].type == WT_NUKEBOMB || availableWeapons[ent->s.weaponIndex].type == WT_NUKEMISSILE )
	{
		if( !ent->thinkFunc_ )
			Com_Error( ERR_DROP, "NULL ent->think");
		else
			ent->thinkFunc_->execute();
	}
	else if( availableWeapons[ent->s.weaponIndex].type == WT_FUELCRATE || 
			 availableWeapons[ent->s.weaponIndex].type == WT_AMMOCRATE || 
			 availableWeapons[ent->s.weaponIndex].type == WT_HEALTHCRATE )
	{
		char * classname = theLevel.getEntity(trace->entityNum)->classname_;// g_entities[trace->entityNum].classname;
		if( strcmp( classname, "func_runway" ) == 0 ||
			strcmp( classname, "func_plat" ) == 0 ||
			strcmp( classname, "func_train" ) == 0 ||
			strcmp( classname, "func_door" ) == 0 ||
			strcmp( classname, "worldspawn" ) == 0) 
		{
			G_CrateDropItems(ent);
		}
	}
	// splash damage (doesn't apply to person directly hit)
	else if ( ent->splashDamage_ )
	{
		if( G_RadiusDamage( trace->endpos, ent->parent_, ent->splashDamage_, ent->splashRadius_, 
			other, ent->splashMethodOfDeath_, ent->targetcat_ ) ) 
		{
			if( !hitClient ) 
				theLevel.getEntity(ent->r.ownerNum)->client_->accuracy_hits_++;
				//g_entities[ent->r.ownerNum].client->accuracy_hits++;
		}
	}

	SV_LinkEntity( ent );
}

/*
================
G_RunMissile
================
*/
void G_RunMissile( GameEntity *ent ) 
{
	vec3_t		origin;
	trace_t		tr;
	int			passent;

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, theLevel.time_, origin );

	// if this missile bounced off an invulnerability sphere
	if ( ent->target_ent_ ) 
		passent = ent->target_ent_->s.number;
	else 
		// ignore interactions with the missile owner
		passent = ent->r.ownerNum;

	// trace a line from the previous position to the current position
	SV_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask_, false );

	if ( tr.startsolid || tr.allsolid ) 
	{
		// make sure the tr.entityNum is set to the entity we're stuck in
		SV_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, ent->r.currentOrigin, passent, ent->clipmask_, false );
		tr.fraction = 0;
	}
	else 
		VectorCopy( tr.endpos, ent->r.currentOrigin );

	SV_LinkEntity( ent );

	if ( tr.fraction != 1)
	{
		// never explode or bounce on sky
		if ( tr.surfaceFlags & SURF_NOIMPACT )
		{
			ent->freeUp();
			return;
		}

		G_MissileImpact( ent, &tr );

		if ( ent->s.eType != ET_MISSILE &&
			 ent->s.eType != ET_BULLET)
		{
			return;		// exploded
		}
	}

	// check think function after bouncing
	G_RunThink( ent );
}

/*
================
G_CrateThink
================
*/
struct Think_Crate : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		vec3_t angles, pos, dir;
		int i;

		// Get values
		VectorCopy(self_->r.currentOrigin, pos);

		// Set angles
		for(i = 0; i < 3; i++)
		{
			angles[i] = 45*sin((pos[2]/25)+(i*10));
		}

		// Create forces based on angles
		VectorCopy(angles,  dir);
		AngleVectors( dir, dir, NULL, NULL );
		VectorScale( dir, (float)self_->speed_/5, dir );
		dir[2] -= self_->speed_;
			
		// Save angles
		VectorCopy( self_->r.currentAngles, self_->s.apos.trBase);
		VectorCopy( angles, self_->s.angles );
		VectorCopy( self_->s.angles, self_->s.apos.trDelta );
		VectorCopy( self_->s.angles, self_->r.currentAngles );
		self_->s.apos.trTime = theLevel.time_;

		// Save Origin
		VectorCopy( self_->r.currentOrigin, self_->s.pos.trBase );
		VectorCopy( dir, self_->s.pos.trDelta );
		self_->s.pos.trTime = theLevel.time_;

		// Set next think
		self_->nextthink_ = theLevel.time_ + 5;
		self_->lastDist_ = VectorNormalize(dir);

		SV_LinkEntity( self_ );
	}
};

//=============================================================================

struct Think_No_Fuel_Flight : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		// update target
		if( self_->tracktarget_ && self_->tracktarget_->client_ ) 
			self_->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;

		self_->s.pos.trType = TR_GRAVITY;
		self_->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
		self_->nextthink_ = theLevel.time_ + 5000;
	}
};

static void on_target_lost( GameEntity *missile ) 
{
	// update target
	if( missile->tracktarget_ && missile->tracktarget_->client_ ) 
		missile->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;

	missile->tracktarget_ = 0;
	missile->setThink(new Think_No_Fuel_Flight);//think = no_fuel_flight;
	missile->nextthink_ = missile->wait_ + 50;
}

/*
================
follow your target
================
*/
struct Think_FollowTarget : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		vec3_t	dir, targdir, mid, mins, maxs;
		float	dist, dot;
		trace_t	tr;
		int		i, prob, num, touch[MAX_GENTITIES], vulner;
		static vec3_t range = { 500, 500, 500 };
		GameEntity* hit;

		// target invalid
		if( !self_->tracktarget_ ) 
		{
			on_target_lost(self_);
			return;
		} 
		else if( !self_->tracktarget_->inuse_ ) 
		{
			on_target_lost(self_);
			return;
		}

		// out of fuel
		if( theLevel.time_ >= self_->wait_ ) 
		{
			on_target_lost(self_);
			return;
		}

		// direction vector and range
		if( self_->tracktarget_->s.eType == ET_EXPLOSIVE ) 
		{
			VectorAdd( self_->tracktarget_->r.absmin, self_->tracktarget_->r.absmax, mid );
			VectorScale( mid, 0.5f, mid );
		} 
		else 
			VectorCopy( self_->tracktarget_->r.currentOrigin, mid );

		VectorSubtract( mid, self_->r.currentOrigin, targdir );
		dist = VectorNormalize(targdir);

		// out of range (if ever possible)
		if( dist > self_->range_ ) 
		{
			on_target_lost(self_);
			return;
		}

		// check for flares
		VectorSubtract( self_->r.currentOrigin, range, mins );
		VectorAdd( self_->r.currentOrigin, range, maxs );
		num = SV_AreaEntities( mins, maxs, touch, MAX_GENTITIES );

		for ( i=0 ; i<num ; i++ ) 
		{
			hit = theLevel.getEntity(touch[i]);//&g_entities[touch[i]];
			if( !hit || !hit->inuse_ ) 
				continue;
			if( hit->s.eType != ET_MISSILE ) 
				continue;
			if( strcmp( hit->classname_, "flare" ) != 0 ) 
				continue;
			if( !hit->ONOFF_ ) 
				continue;
			if( hit->r.ownerNum == self_->r.ownerNum ) 
				continue; 
			hit->ONOFF_ = 0; // disable the flare
			prob = rand() % 100;
			vulner = self_->basicECMVulnerability_ - self_->tracktarget_->s.frame;
			//Com_Printf("TT: %d, vulner: %d, prob: %d\n", missile->tracktarget->s.frame, vulner, prob);
			if( prob < vulner ) 
			{
				if( self_->tracktarget_->client_ )
					self_->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] &= ~LI_BEING_LAUNCHED;
				self_->tracktarget_ = hit;
				self_->lastDist_ = 0;
				self_->nextthink_ = theLevel.time_ + 10;
				return;
			}
		}

		// close miss
		if( self_->lastDist_ && dist > self_->lastDist_ && dist < self_->splashRadius_ ) 
		{
			self_->nextthink_ = theLevel.time_ + 10;
			self_->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
		}

		// out of seeker cone
		VectorCopy( self_->s.pos.trDelta, dir );
		VectorNormalize( dir );
		dot = DotProduct( dir, targdir );
		if( dot < self_->followcone_ ) 
		{ 
			// roughly 5 degrees
			on_target_lost(self_);
			return;
		}

		// LOS
		SV_Trace( &tr, self_->r.currentOrigin, 0, 0, mid, self_->tracktarget_->s.number, MASK_SOLID, false );
		if( tr.fraction < 1.0f ) 
		{
			on_target_lost(self_);
			return;
		}

		// adjust course/speed
		VectorMA( dir, 1.85f, targdir, targdir );
		VectorNormalize( targdir );
		VectorScale( targdir, self_->speed_, dir );
		
		vectoangles( dir, targdir );
		VectorCopy( targdir, self_->s.angles );
		VectorCopy( self_->s.angles, self_->s.apos.trBase );
		VectorCopy( self_->s.angles, self_->r.currentAngles );
		self_->s.apos.trTime = theLevel.time_;

		VectorCopy( self_->r.currentOrigin, self_->s.pos.trBase );
		VectorCopy( dir, self_->s.pos.trDelta );
		self_->s.pos.trTime = theLevel.time_;

		self_->nextthink_ = theLevel.time_ + 100;
		self_->lastDist_ = dist;

		// update target
		if( self_->tracktarget_->client_ ) 
			self_->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] |= LI_BEING_LAUNCHED;

		SV_LinkEntity( self_ );
	}
};

struct Think_DropToNormalTransition : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		self_->s.pos.trType = TR_ACCEL;//TR_LINEAR;
		self_->s.pos.trTime = theLevel.time_;
		self_->s.pos.trDuration = 500;

		VectorCopy( self_->r.currentOrigin, self_->s.pos.trBase );
		VectorNormalize( self_->s.pos.trDelta );
		VectorScale( self_->s.pos.trDelta, self_->speed_, self_->s.pos.trDelta );
		if( self_->tracktarget_ )
		{
			self_->wait_ = theLevel.time_ + availableWeapons[self_->s.weaponIndex].fuelrange;
			self_->setThink(new Think_FollowTarget);
			self_->nextthink_ = theLevel.time_ + 50;
		} else {
			self_->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
			self_->nextthink_ = theLevel.time_ + availableWeapons[self_->s.weaponIndex].fuelrange;
		}
	}
};

//=============================================================================

/*
=================
fire_antiair MFQ3
=================
*/
void fire_antiair (GameEntity *self) 
{
	GameEntity	*bolt;
	vec3_t		dir, right, up, temp, forward;
	vec3_t		start, offset;
	bool	active = ( (self->client_->ps_.stats[STAT_LOCKINFO] & LI_LOCKING) != 0 );
	bool	wingtip = false;

	VectorCopy( self->s.pos.trBase, start );

	if( (availableVehicles[self->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_BOAT) ) 
	{
		VectorCopy( availableVehicles[self->client_->vehicle_].gunoffset, offset );
		AngleVectors( self->client_->ps_.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, self->client_->ps_.turretAngle );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, self->client_->ps_.gunAngle );
		wingtip = true; // dont drop
	} 
	else 
	{
		self->left_ = (self->left_ ? false : true);
		MF_removeWeaponFromLoadout(self->client_->ps_.weaponIndex, &self->loadout_, &self->client_->ps_, &wingtip, offset, 0 );
		AngleVectors( self->client_->ps_.vehicleAngles, dir, right, up );
		VectorInverse( right );
	}
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "aam";
	if( active ) 
	{
		bolt->tracktarget_ = self->tracktarget_;
		// update target
		if( self->tracktarget_->client_ ) 
			self->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] |= LI_BEING_LAUNCHED;
		bolt->followcone_ = availableWeapons[self->client_->ps_.weaponIndex].followcone;
	}
	bolt->speed_ = availableWeapons[self->client_->ps_.weaponIndex].muzzleVelocity;
	if( wingtip ) 
	{
		if( active ) 
		{
			bolt->wait_ = theLevel.time_ + availableWeapons[self->client_->ps_.weaponIndex].fuelrange;
			bolt->setThink(new Think_FollowTarget);
			bolt->nextthink_ = theLevel.time_ + 50;
		} 
		else 
		{
			bolt->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
			bolt->nextthink_ = theLevel.time_ + availableWeapons[self->client_->ps_.weaponIndex].fuelrange;
		}
		bolt->s.pos.trType = TR_LINEAR;
		VectorScale( dir, availableWeapons[self->client_->ps_.weaponIndex].muzzleVelocity, bolt->s.pos.trDelta );
		/*
		bolt->s.pos.trType = TR_ACCEL;
		bolt->s.pos.trDuration = availableWeapons[self->client->ps.weaponIndex].muzzleVelocity;
		VectorScale( dir, max(10,self->client->ps.speed/10), bolt->s.pos.trDelta );
		*/
	} 
	else 
	{
		bolt->setThink(new Think_DropToNormalTransition);
		bolt->nextthink_ = theLevel.time_ + 400;
		bolt->s.pos.trType = TR_GRAVITY;
		VectorScale( dir, self->client_->ps_.speed/10, bolt->s.pos.trDelta );
	}
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = bolt->splashDamage_ = availableWeapons[self->client_->ps_.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[self->client_->ps_.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[self->client_->ps_.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->range_ = availableWeapons[self->client_->ps_.weaponIndex].range;
	bolt->methodOfDeath_ = MOD_FFAR;
	bolt->splashMethodOfDeath_ = MOD_FFAR_SPLASH;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;
	bolt->basicECMVulnerability_ = availableWeapons[self->client_->ps_.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_antiground MFQ3
=================
*/
void fire_antiground( GameEntity *self ) 
{
	GameEntity	*bolt;
	vec3_t		dir, right, up;
	vec3_t		start, offset, forward, temp;
	bool		active = ( (self->client_->ps_.stats[STAT_LOCKINFO] & LI_LOCKING) != 0 );
	bool		wingtip = false;

	VectorCopy( self->s.pos.trBase, start );

	if( (availableVehicles[self->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_BOAT) ) 
	{
		VectorCopy( availableVehicles[self->client_->vehicle_].gunoffset, offset );
		AngleVectors( self->client_->ps_.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, self->client_->ps_.turretAngle );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, self->client_->ps_.gunAngle );
		wingtip = true; // dont drop
	} 
	else 
	{
		self->left_ = (self->left_ ? false : true);
		MF_removeWeaponFromLoadout(self->client_->ps_.weaponIndex, &self->loadout_, &self->client_->ps_, 0, offset, 0 );
		AngleVectors( self->client_->ps_.vehicleAngles, dir, right, up );
		VectorInverse( right );
	}
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "agm";

	if( active ) 
	{
//		bolt->wait = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
//		bolt->think = follow_target;
//		bolt->nextthink = level.time + 50;
		bolt->tracktarget_ = self->tracktarget_;
		// update target
		if( self->tracktarget_->client_ ) {
			self->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] |= LI_BEING_LAUNCHED;
		}
		bolt->followcone_ = availableWeapons[self->client_->ps_.weaponIndex].followcone;
//	} else {
//		bolt->setThink(new Think_ExplodeMissile(bolt));//think = G_ExplodeMissile;
//		bolt->nextthink = level.time + availableWeapons[self->client->ps.weaponIndex].fuelrange;
	}

	bolt->speed_ = availableWeapons[self->client_->ps_.weaponIndex].muzzleVelocity;
	if( wingtip ) 
	{
		if( active ) 
		{
			bolt->wait_ = theLevel.time_ + availableWeapons[self->client_->ps_.weaponIndex].fuelrange;
			bolt->setThink(new Think_FollowTarget);
			bolt->nextthink_ = theLevel.time_ + 50;
		} 
		else 
		{
			bolt->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
			bolt->nextthink_ = theLevel.time_ + availableWeapons[self->client_->ps_.weaponIndex].fuelrange;
		}
		bolt->s.pos.trType = TR_LINEAR;
		VectorScale( dir, bolt->speed_, bolt->s.pos.trDelta );
	} 
	else 
	{
		bolt->setThink(new Think_DropToNormalTransition);
		bolt->nextthink_ = theLevel.time_ + 400;
		bolt->s.pos.trType = TR_GRAVITY;
		VectorScale( dir, self->client_->ps_.speed/10, bolt->s.pos.trDelta );
	}

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = bolt->splashDamage_ = availableWeapons[self->client_->ps_.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[self->client_->ps_.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[self->client_->ps_.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->range_ = availableWeapons[self->client_->ps_.weaponIndex].range;
	bolt->methodOfDeath_ = MOD_FFAR;
	bolt->splashMethodOfDeath_ = MOD_FFAR_SPLASH;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;
	bolt->basicECMVulnerability_ = availableWeapons[self->client_->ps_.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_ffar MFQ3
=================
*/
void fire_ffar (GameEntity *self) 
{
	GameEntity	*bolt;
	vec3_t		dir, right, up;
	vec3_t		start, offset;

	VectorCopy( self->s.pos.trBase, start );

	self->left_++;
	if( self->left_ >= availableVehicles[self->client_->vehicle_].ammo[self->client_->ps_.weaponNum] ) 
		self->left_ = 0;

	MF_removeWeaponFromLoadout(self->client_->ps_.weaponIndex, &self->loadout_, &self->client_->ps_, 0, offset, self->left_ );
	AngleVectors( self->client_->ps_.vehicleAngles, dir, right, up );
	VectorInverse( right );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "ffar";
	bolt->nextthink_ = theLevel.time_ + availableWeapons[self->client_->ps_.weaponIndex].fuelrange;
	bolt->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = bolt->splashDamage_ = availableWeapons[self->client_->ps_.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[self->client_->ps_.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[self->client_->ps_.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->methodOfDeath_ = MOD_FFAR;
	bolt->splashMethodOfDeath_ = MOD_FFAR_SPLASH;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;

	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, availableWeapons[self->client_->ps_.weaponIndex].muzzleVelocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
fire_ironbomb MFQ3
=================
*/
void fire_ironbomb( GameEntity *self ) 
{
	GameEntity	*bolt;
	vec3_t		dir, right, up, offset;
	vec3_t		start;

//	self->left = (self->left ? false : true);
	MF_removeWeaponFromLoadout(self->client_->ps_.weaponIndex, &self->loadout_, &self->client_->ps_, 0, offset, 0 );
	AngleVectors( self->client_->ps_.vehicleAngles, dir, right, up );
	VectorInverse( right );
	VectorCopy( self->s.pos.trBase, start );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "ironbomb";
	bolt->nextthink_ = theLevel.time_ + 10000;
	bolt->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = bolt->splashDamage_ = availableWeapons[self->client_->ps_.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[self->client_->ps_.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[self->client_->ps_.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->methodOfDeath_ = MOD_IRONBOMB;
	bolt->splashMethodOfDeath_ = MOD_IRONBOMB_SPLASH;
	bolt->clipmask_= MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, self->client_->ps_.speed/10, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_autocannon MFQ3
=================
*/
void fire_autocannon( GameEntity *self, bool main )
{
	GameEntity	*bolt;
	vec3_t		dir, forward, right, up, temp;
	vec3_t		start, offset;
	vec3_t		spreadangle;
	int			weapIdx = (main ? self->client_->ps_.weaponIndex : availableVehicles[self->client_->vehicle_].weapons[0]);
	float		spreadX = availableWeapons[weapIdx].spread; 
	float		spreadY = spreadX;

	// used for spread
	VectorCopy( self->client_->ps_.vehicleAngles, spreadangle );
	spreadX = ((rand() % (unsigned int)spreadX) - spreadX/2)/10;
	spreadY = ((rand() % (unsigned int)spreadY) - spreadY/2)/10;
	spreadangle[0] += spreadX;
	spreadangle[1] += spreadY;
//	Com_Printf( "spread %.1f %.1f\n", spreadX, spreadY );

	VectorCopy( availableVehicles[self->client_->vehicle_].gunoffset, offset );
	VectorCopy( self->s.pos.trBase, start );

	if( (availableVehicles[self->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_BOAT) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_HELO)) 
	{
		// use this to make it shoot where the player looks
//		AngleVectors( self->client->ps.viewangles, dir, 0, 0 );
//		VectorCopy( self->s.pos.trBase, start );
//		start[2] += availableVehicles[self->client->vehicle].maxs[2];
		// otherwise use this
		AngleVectors( spreadangle, forward, right, up );
		RotatePointAroundVector( temp, up, forward, self->client_->ps_.turretAngle );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, self->client_->ps_.gunAngle );

	} 
	else 
	{
		// planes and helos for now just shoot along their direction of flight
		AngleVectors( spreadangle, dir, right, up );
	}

	if( availableVehicles[self->client_->vehicle_].caps & HC_DUALGUNS )
	{
		self->bulletpos_ = ( self->bulletpos_ == 0 ? 1 : 0 );
		if( self->bulletpos_ ) 
			offset[1] *= -1;
	}

	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "bullet";
	bolt->nextthink_ = theLevel.time_ + 4000;
	bolt->setThink(new GameEntity::EntityFunc_Free);
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = weapIdx;
	//availableVehicles[self->client->vehicle].weapons[WP_MACHINEGUN];
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = availableWeapons[weapIdx].damage;
	//availableWeapons[availableVehicles[self->client->vehicle].weapons[WP_MACHINEGUN]].damage;
	bolt->targetcat_ = availableWeapons[weapIdx].category;
	bolt->catmodifier_ = availableWeapons[weapIdx].noncatmod;
	bolt->methodOfDeath_ = MOD_AUTOCANNON;
	bolt->clipmask_ = MASK_SHOT;
	bolt->target_ent_ = NULL;
	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = theLevel.time_ - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	//tracer
	self->tracerindex_--;
	if( self->tracerindex_ < 0 ) 
		self->tracerindex_ = 255;
	bolt->s.eType = ET_BULLET;
	bolt->s.generic1 = self->tracerindex_;
	VectorCopy( start, bolt->s.pos.trBase );
//	VectorScale( dir, availableWeapons[availableVehicles[self->client->vehicle].weapons[WP_MACHINEGUN]].muzzleVelocity, bolt->s.pos.trDelta );
	VectorScale( dir, availableWeapons[weapIdx].muzzleVelocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_autocannon_GI MFQ3
=================
*/
void fire_autocannon_GI (GameEntity *self) 
{
	GameEntity	*bolt;
	vec3_t		dir;
	vec3_t		start;
	int			weapIdx = self->s.weaponIndex;
//	float		spreadX = availableWeapons[weapIdx].spread; 
//	float		spreadY = spreadX;

	// used for spread
	VectorSubtract( self->tracktarget_->r.currentOrigin, self->r.currentOrigin, dir );
	VectorNormalize(dir);
	VectorCopy( self->s.pos.trBase, start );
	start[2] += 10;
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "bullet";
	bolt->nextthink_ = theLevel.time_ + 4000;
	bolt->setThink(new GameEntity::EntityFunc_Free);
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = weapIdx;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = availableWeapons[weapIdx].damage;
	bolt->targetcat_ = availableWeapons[weapIdx].category;
	bolt->catmodifier_ = availableWeapons[weapIdx].noncatmod;
	bolt->methodOfDeath_ = MOD_AUTOCANNON;
	bolt->clipmask_ = MASK_SHOT;
	bolt->target_ent_ = NULL;
	bolt->s.pos.trType = TR_LINEAR;
	bolt->s.pos.trTime = theLevel.time_ - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	//tracer
	self->tracerindex_--;
	if( self->tracerindex_ < 0 ) 
		self->tracerindex_ = 255;
	bolt->s.eType = ET_BULLET;
	bolt->s.generic1 = self->tracerindex_;
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, availableWeapons[weapIdx].muzzleVelocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
fire_maingun MFQ3
=================
*/
void fire_maingun( GameEntity *self ) 
{
	GameEntity	*bolt;
	vec3_t		dir, forward, right, up, temp;
	vec3_t		start, offset;

	AngleVectors( self->client_->ps_.vehicleAngles, forward, right, up );
	RotatePointAroundVector( temp, up, forward, self->client_->ps_.turretAngle );
	CrossProduct( up, temp, right );
	RotatePointAroundVector( dir, right, temp, self->client_->ps_.gunAngle );

	VectorCopy( self->s.pos.trBase, start );
	VectorCopy( availableVehicles[self->client_->vehicle_].gunoffset, offset );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "shell";
	bolt->nextthink_ = theLevel.time_ + 15000;
	bolt->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = bolt->splashDamage_ = availableWeapons[self->client_->ps_.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[self->client_->ps_.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[self->client_->ps_.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->methodOfDeath_ = MOD_MAINGUN;
	bolt->splashMethodOfDeath_ = MOD_MAINGUN;
	bolt->clipmask_ = MASK_SHOT;
	bolt->target_ent_ = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, availableWeapons[self->client_->ps_.weaponIndex].muzzleVelocity, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}



/*
=================
fire_flare MFQ3
=================
*/
void fire_flare( GameEntity *self ) 
{
	GameEntity	*bolt;
	vec3_t		start, up;
	
	VectorSet( up, 0, 0, 1 );
	VectorCopy( self->s.pos.trBase, start );
	if( (availableVehicles[self->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_BOAT) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_HELO) ) 
		VectorMA( start, self->r.maxs[2]+3, up, start );
	else
		VectorMA( start, self->r.mins[2]-3, up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "flare";
	bolt->nextthink_ = theLevel.time_ + 1600;
	bolt->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = WI_FLARE;
		
	bolt->parent_ = self;
	bolt->r.ownerNum = self->s.number;

	bolt->damage_ = bolt->splashDamage_ = 0;
	bolt->splashRadius_ = 0;
	bolt->clipmask_ = MASK_SHOT;
	bolt->ONOFF_ = 1;// not used

	bolt->s.pos.trType = TR_GRAVITY_10;
	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	if( (availableVehicles[self->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_BOAT) ) 
		VectorScale( up, 300, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);

}

/*
=================
fire_flare2 MFQ3
fire flare with direction
=================
*/
void fire_flare2( GameEntity *self, vec3_t start, vec3_t up, int age ) 
{
	GameEntity	*bolt;
	
	bolt = theLevel.spawnEntity();
	bolt->classname_ = "flare";
	bolt->nextthink_ = theLevel.time_ + age;
	bolt->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = WI_FLARE;
		
	bolt->parent_ = self;
	bolt->r.ownerNum = self->s.number;

	bolt->damage_ = bolt->splashDamage_ = 0;
	bolt->splashRadius_ = 0;
	bolt->clipmask_ = MASK_SHOT;
	bolt->ONOFF_ = 1;// not used

	bolt->s.pos.trType = TR_GRAVITY_10;
	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( up, 1, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
fire_cflare
fire flare cluster
=================
*/
void fire_cflare( GameEntity *self) 
{
	GameEntity	*bolt;
	vec3_t		start, up;
	int age;
	
	VectorSet( up, 0, 0, 1 );
	VectorCopy( self->s.pos.trBase, start );
	if( (availableVehicles[self->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_BOAT) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_HELO) )  
	{
		VectorMA( start, self->r.maxs[2]+3, up, start );
		age = 1600;
	}
	else 
	{
		VectorMA( start, self->r.mins[2]-3, up, start );
		age = 800;
	}
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "cflare";
	bolt->nextthink_ = theLevel.time_ + age;
	bolt->setThink(new Think_ExplodeCFlare);
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = WI_CFLARE;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = bolt->splashDamage_ = 0;
	bolt->splashRadius_ = 0;
	bolt->clipmask_ = MASK_SHOT;
	bolt->ONOFF_ = 1;// not used

	bolt->s.pos.trType = TR_GRAVITY_10;
	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	if( (availableVehicles[self->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_BOAT) ) 
			VectorScale( up, 300, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
use_fueltank MFQ3
=================
*/
void drop_fueltank ( GameEntity *self )
{
	GameEntity	*bolt;
	vec3_t		dir, right, up, offset;
	vec3_t		start;

//	self->left = (self->left ? false : true);
	MF_removeWeaponFromLoadout(self->client_->ps_.weaponIndex, &self->loadout_, &self->client_->ps_, 0, offset, 0 );
	AngleVectors( self->client_->ps_.vehicleAngles, dir, right, up );
	VectorInverse( right );
	VectorCopy( self->s.pos.trBase, start );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "droptank";
	bolt->nextthink_ = theLevel.time_ + 10000;
	bolt->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = bolt->splashDamage_ = availableWeapons[self->client_->ps_.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[self->client_->ps_.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[self->client_->ps_.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->methodOfDeath_ = MOD_IRONBOMB;
	bolt->splashMethodOfDeath_ = MOD_IRONBOMB_SPLASH;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, self->client_->ps_.speed/10, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
================
G_CrateDie
================
*/
struct Die_CrateDie : public GameEntity::EntityFunc_Die
{
	virtual void execute( GameEntity *inflictor, GameEntity *attacker, int damage, int mod )
	{
			if (inflictor == self_)
					return;
			self_->takedamage_ = false;
			self_->setThink(new Think_ExplodeMissile);//think = G_ExplodeMissile;
			self_->nextthink_ = theLevel.time_ + 10;
	}
};

/*
=================
drop_crate
=================
*/
void drop_crate( GameEntity *self )
{
	GameEntity	*bolt;
	vec3_t		start;

	MF_removeWeaponFromLoadout(self->client_->ps_.weaponIndex, &self->loadout_, &self->client_->ps_, 0, 0, 0 );

	VectorCopy( self->s.pos.trBase, start );
	//start[2] -= availableVehicles[self->client->vehicle].maxs[2] - availableVehicles[self->client->vehicle].mins[2];
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "crate";
	bolt->nextthink_ = theLevel.time_ + 100;
	//bolt->nextthink = level.time + 10000;
	bolt->setThink(new Think_Crate);
	//bolt->setThink(new Think_ExplodeMissile(bolt));//think = G_ExplodeMissile;
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;
	bolt->speed_ = bolt->speed_ = availableWeapons[self->client_->ps_.weaponIndex].muzzleVelocity;

	bolt->s.pos.trType = TR_LINEAR;
	//bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = theLevel.time_;

	bolt->health_ = 5;
	bolt->takedamage_ = true;
	bolt->setDie(new Die_CrateDie);
	bolt->r.contents = CONTENTS_BODY;

	VectorSet(bolt->r.mins, -10, -3, 0);
	VectorCopy(bolt->r.mins, bolt->r.absmin);
	VectorSet(bolt->r.maxs, 10, 3, 6);
	VectorCopy(bolt->r.maxs, bolt->r.absmax);

	VectorCopy( start, bolt->s.pos.trBase );
	VectorCopy (start, bolt->r.currentOrigin);
}


/*
=================
fire_nukebomb MFQ3
=================
*/
void fire_nukebomb ( GameEntity *self ) 
{
	GameEntity	*bolt;
	vec3_t		dir, right, up, offset;
	vec3_t		start;

//	self->left = (self->left ? false : true);
	MF_removeWeaponFromLoadout(self->client_->ps_.weaponIndex, &self->loadout_, &self->client_->ps_, 0, offset, 0 );
	AngleVectors( self->client_->ps_.vehicleAngles, dir, right, up );
	VectorInverse( right );
	VectorCopy( self->s.pos.trBase, start );
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "nukebomb";
	bolt->nextthink_ = theLevel.time_ + 10000;
	bolt->setThink(new Think_ExplodeNuke);
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = 1000;
	bolt->splashDamage_ = 0;
	bolt->damage_ = availableWeapons[self->client_->ps_.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[self->client_->ps_.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[self->client_->ps_.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->methodOfDeath_ = MOD_NUKE;
	bolt->splashMethodOfDeath_ = MOD_NUKE;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;

	bolt->s.pos.trType = TR_GRAVITY;
	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	VectorScale( dir, self->client_->ps_.speed/10, bolt->s.pos.trDelta );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}

/*
=================
fire_nukemissile MFQ3
=================
*/
void fire_nukemissile (GameEntity *self) 
{
	GameEntity	*bolt;
	vec3_t		dir, right, up;
	vec3_t		start, offset, forward, temp;
	bool	wingtip = false;

	VectorCopy( self->s.pos.trBase, start );

	if( (availableVehicles[self->client_->vehicle_].cat & CAT_GROUND) ||
		(availableVehicles[self->client_->vehicle_].cat & CAT_BOAT) ) 
	{
		VectorCopy( availableVehicles[self->client_->vehicle_].gunoffset, offset );
		AngleVectors( self->client_->ps_.vehicleAngles, forward, right, up );
		RotatePointAroundVector( temp, up, forward, self->client_->ps_.turretAngle );
		CrossProduct( up, temp, right );
		RotatePointAroundVector( dir, right, temp, self->client_->ps_.gunAngle );
		wingtip = true; // dont drop
	} 
	else
	{
		self->left_ = (self->left_ ? false : true);
		MF_removeWeaponFromLoadout(self->client_->ps_.weaponIndex, &self->loadout_, &self->client_->ps_, 0, offset, 0 );
		AngleVectors( self->client_->ps_.vehicleAngles, dir, right, up );
		VectorInverse( right );
	}
	VectorMA( start, offset[0], dir, start );
	VectorMA( start, offset[1], right, start );
	VectorMA( start, offset[2], up, start );
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "nukemissile";

	bolt->speed_ = availableWeapons[self->client_->ps_.weaponIndex].muzzleVelocity;

	bolt->setThink(new Think_ExplodeNuke);
	bolt->nextthink_ = theLevel.time_ + availableWeapons[self->client_->ps_.weaponIndex].fuelrange;

	bolt->s.pos.trType = TR_LINEAR;
	VectorScale( dir, bolt->speed_, bolt->s.pos.trDelta );

	// Give health? TODO...

	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->client_->ps_.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;

	bolt->damage_ = availableWeapons[self->client_->ps_.weaponIndex].damage;
	bolt->splashDamage_ = 0;
	bolt->splashRadius_ = availableWeapons[self->client_->ps_.weaponIndex].damageRadius;

	bolt->targetcat_ = availableWeapons[self->client_->ps_.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->client_->ps_.weaponIndex].noncatmod;
	bolt->range_ = availableWeapons[self->client_->ps_.weaponIndex].range;
	bolt->methodOfDeath_ = MOD_NUKE;
	bolt->splashMethodOfDeath_ = MOD_NUKE;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;
	bolt->basicECMVulnerability_ = availableWeapons[self->client_->ps_.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	
}


// =================================================================================================================
// ================================================== GI MISSILE STUFF =============================================
// =================================================================================================================




/*
=================
LaunchMissile_GI MFQ3
=================
*/
void LaunchMissile_GI( GameEntity* ent )
{
	GameEntity	*bolt;
	vec3_t		dir, start;

	ent->count_--;
//	Com_Printf("Launching GI missile....%d left\n", ent->count);

	VectorCopy( ent->s.pos.trBase, start );
	start[2] += 10;
	VectorSubtract( ent->tracktarget_->r.currentOrigin, ent->r.currentOrigin, dir );
	VectorNormalize(dir);
	SnapVector( start );

	bolt = theLevel.spawnEntity();
	bolt->classname_ = "aam";
	bolt->tracktarget_ = ent->tracktarget_;
	// update target
	if( ent->tracktarget_->client_ ) 
		ent->tracktarget_->client_->ps_.stats[STAT_LOCKINFO] |= LI_BEING_LAUNCHED;

	bolt->followcone_ = availableWeapons[ent->s.weaponIndex].followcone;
	bolt->speed_ = availableWeapons[ent->s.weaponIndex].muzzleVelocity;
	bolt->wait_ = theLevel.time_ + availableWeapons[ent->s.weaponIndex].fuelrange;
	bolt->setThink(new Think_FollowTarget);
	bolt->nextthink_ = theLevel.time_ + 50;
	bolt->s.pos.trType = TR_LINEAR;
	VectorScale( dir, bolt->speed_, bolt->s.pos.trDelta );
	bolt->s.eType = ET_MISSILE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = ent->s.weaponIndex;
	bolt->r.ownerNum = ent->s.number;
	bolt->parent_ = ent;
	bolt->damage_ = bolt->splashDamage_ = availableWeapons[ent->s.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[ent->s.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[ent->s.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[ent->s.weaponIndex].noncatmod;
	bolt->range_ = availableWeapons[ent->s.weaponIndex].range;
	bolt->methodOfDeath_ = MOD_FFAR;
	bolt->splashMethodOfDeath_ = MOD_FFAR_SPLASH;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;
	bolt->basicECMVulnerability_ = availableWeapons[ent->s.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
}



/*
=================
fire_flak_GI
=================
*/
#define FLAK_DEVIATION_XY 800
#define FLAK_DEVIATION_HEIGHT 200
void fire_flak_GI (GameEntity *self)
{
	GameEntity	*bolt;
	vec3_t		start, offset;
	bool	wingtip = false;
	int		fuse;
	static int	seed = 0x92;
	int dis;

	// Find fuse time
	dis = Distance(self->r.currentOrigin,self->tracktarget_->r.currentOrigin);
	if(dis < 1000)
		return;	// Need to be above reasonably!
	fuse = (float)dis/availableWeapons[self->s.weaponIndex].muzzleVelocity;


	// Cheat using a random detonation position near player
	start[0] = self->tracktarget_->r.currentOrigin[0] + (FLAK_DEVIATION_XY - (FLAK_DEVIATION_XY * -1) + 1) * 
				Q_random( &seed ) + (FLAK_DEVIATION_XY * -1);
	start[1] = self->tracktarget_->r.currentOrigin[1] + (FLAK_DEVIATION_XY - (FLAK_DEVIATION_XY * -1) + 1) * 
				Q_random( &seed ) + (FLAK_DEVIATION_XY * -1);
	start[2] = self->tracktarget_->r.currentOrigin[2] + (FLAK_DEVIATION_HEIGHT - (FLAK_DEVIATION_HEIGHT * -1) + 1) * 
				Q_random( &seed ) + (FLAK_DEVIATION_HEIGHT * -1);

	// Remove one weapon to fire
	MF_removeWeaponFromLoadout(self->s.weaponIndex, &self->loadout_, &self->client_->ps_, &wingtip, offset, 0 );
	
	bolt = theLevel.spawnEntity();
	bolt->classname_ = "flak";

	bolt->speed_ = availableWeapons[self->s.weaponIndex].muzzleVelocity;
	bolt->setThink(new Think_ExplodeFlak);
	bolt->nextthink_ = theLevel.time_ + fuse;
	bolt->s.pos.trType = TR_LINEAR;

	bolt->s.eType = ET_INVISIBLE;
	bolt->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	bolt->s.weaponIndex = self->s.weaponIndex;
	bolt->r.ownerNum = self->s.number;
	bolt->parent_ = self;
	bolt->damage_ = bolt->splashDamage_ = availableWeapons[self->s.weaponIndex].damage;
	bolt->splashRadius_ = availableWeapons[self->s.weaponIndex].damageRadius;
	bolt->targetcat_ = availableWeapons[self->s.weaponIndex].category;
	bolt->catmodifier_ = availableWeapons[self->s.weaponIndex].noncatmod;
	bolt->range_ = availableWeapons[self->s.weaponIndex].range;
	bolt->methodOfDeath_ = MOD_FFAR;
	bolt->splashMethodOfDeath_ = MOD_FFAR_SPLASH;
	bolt->clipmask_ = MASK_SHOT|MASK_WATER;
	bolt->target_ent_ = NULL;
	bolt->basicECMVulnerability_ = availableWeapons[self->s.weaponIndex].basicECMVulnerability;

	bolt->s.pos.trTime = theLevel.time_;// - MISSILE_PRESTEP_TIME;		// move a bit on the very first frame
	VectorCopy( start, bolt->s.pos.trBase );
	SnapVector( bolt->s.pos.trDelta );			// save net bandwidth
	VectorCopy (start, bolt->r.currentOrigin);
	
}


