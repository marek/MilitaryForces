/*
 * $Id: g_miscvehicle.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
*/


#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"
#include "g_miscvehicle.h"


//------------------------------------------------------------------
//
// MISC_VEHICLE
//
//------------------------------------------------------------------

int Game::miscVehicle_TeamScore( GameEntity *self, GameEntity *attacker )
{
	if( g_gametype.integer >= GT_TEAM && self->s.generic1 == attacker->client_->ps_.persistant[PERS_TEAM] )
		return -self->score_;
	return self->score_;
}

void MiscVehicle_Die::execute( GameEntity *inflictor, GameEntity *attacker, int damage, int meansOfDeath )
{
	if( self_->score_ && attacker && attacker->client_ ) {
		Vec3	pos;
		pos = self_->r.absmin + self_->r.absmax;
		pos = pos * ( 0.5f );
		AddScore( attacker, pos, Game::miscVehicle_TeamScore( self_, attacker ) );
	}

	ExplodeVehicle(self_);
	self_->freeAfterEvent_ = true;

	Game::radiusDamage( self_->r.currentOrigin, self_, 150, 150, self_, MOD_VEHICLEEXPLOSION, CAT_ANY );

	GameHost::linkEntity( self_ );
}

void Game::miscVehicle_DropToGround( GameEntity *ent, float above, float below )
{
	trace_t	trace;
	Vec3	startpos, endpos;
	unsigned int vehIndex = ent->s.modelindex;

	VectorCopy( ent->s.origin, startpos );
	startpos[2] += above;
	VectorCopy( ent->s.origin, endpos );
	endpos[2] -= below;
	GameHost::trace( &trace, startpos, nullptr, nullptr, endpos, ENTITYNUM_NONE, MASK_SOLID, false );
	if( trace.fraction < 1.0f ) {
		ent->s.origin[2] = trace.endpos[2] - availableVehicles[vehIndex].mins[2] + 1;
	}
}

void Game::sP_misc_plane( GameEntity *ent )
{
	unsigned int vehIndex = ent->s.modelindex;
	trace_t	trace;
	Vec3	startpos, endpos;

	VectorCopy( ent->s.origin, startpos );
	startpos[2] += 64;
	VectorCopy( ent->s.origin, endpos );
	endpos[2] -= 256;
	GameHost::trace( &trace, startpos, nullptr, nullptr, endpos, ENTITYNUM_NONE, MASK_SOLID, false );
	if( trace.fraction < 1.0f ) {
		GameEntity *test = theLevel.getEntity(trace.entityNum);
		if( test && canLandOnIt(test) ) {
			ent->s.origin[2] = test->r.maxs[2] - availableVehicles[vehIndex].mins[2] +
				availableVehicles[vehIndex].gearheight;
		} else {
			// parked on terrain
			ent->s.origin[2] = trace.endpos[2] - availableVehicles[vehIndex].mins[2] +
				availableVehicles[vehIndex].gearheight;
		}
	}

	// set flags
	ent->s.ONOFF = OO_NOTHING_ON;

	// gearheight
	if( availableVehicles[vehIndex].caps & HC_GEAR ) {
		ent->gearheight_ = availableVehicles[vehIndex].gearheight;
	} else {
		ent->s.ONOFF |= OO_GEAR;
	}

	// parked: no speed, no throttle, level, gear down
	ent->speed_ = 0;
	ent->s.frame = 0;
	ent->s.ONOFF |= OO_LANDED|OO_STALLED|OO_GEAR;
	ent->s.angles[0] = 0;
	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	// update gear anim
	ent->updateGear_ = true;

	// update bay anim
	ent->updateBay_ = true;

	ent->s.pos.trTime = theLevel.time_;
	GameHost::linkEntity (ent);
}

void Game::sP_misc_gv( GameEntity *ent )
{
	Game::miscVehicle_DropToGround( ent, 64, 512 );

	// set flags
	ent->s.ONOFF = OO_LANDED;

	ent->speed_ = 0;
	ent->s.frame = 0;// throttle

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	GameHost::linkEntity (ent);
}

void Game::sP_misc_helo( GameEntity *ent )
{
	unsigned int vehIndex = ent->s.modelindex;

	Game::miscVehicle_DropToGround( ent, 64, 512 );

	// set flags
	ent->s.ONOFF = OO_LANDED;

	// gearheight
	if( availableVehicles[vehIndex].caps & HC_GEAR ) {
		ent->gearheight_ = availableVehicles[vehIndex].gearheight;
	} else {
		ent->s.ONOFF |= OO_GEAR;
	}

	ent->speed_ = 0;
	ent->s.frame = 0;// throttle

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	GameHost::linkEntity (ent);
}

void Game::sP_misc_boat( GameEntity *ent )
{
	Game::miscVehicle_DropToGround( ent, 64, 512 );

	// set flags
	ent->s.ONOFF = OO_LANDED;

	ent->speed_ = 0;
	ent->s.frame = 0;// throttle

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	GameHost::linkEntity (ent);
}

void Drone_Think::execute()
{
	GameEntity* ent = self_;
	unsigned int vehIndex = ent->s.modelindex;
	unsigned long cat = availableVehicles[vehIndex].cat;
	bool flying = (cat & CAT_PLANE) || (cat & CAT_HELO);
	float dt = (theLevel.time_ - ent->s.pos.trTime) * 0.001f;
	Vec3 origin, dir, bearing, angles, vel;
	trace_t tr;

	GameShared::evaluateTrajectory( &ent->s.pos, theLevel.time_, origin );

	if( flying )
	{
		GameHost::trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->s.number, MASK_SOLID, false );
		if( tr.startsolid || tr.fraction < 1.0f )
		{
			Game::damage( ent, nullptr, ent, nullptr, nullptr, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
			return;
		}
	}
	else
	{
		Vec3 from, to;
		from = ent->r.currentOrigin;
		to = origin;
		float raise = (ent->r.maxs[2] - ent->r.mins[2]) * 0.5f;
		from[2] += raise;
		to[2] += raise;
		GameHost::trace( &tr, from, ent->r.mins, ent->r.maxs, to, ent->s.number, MASK_SOLID, false );
		if( !tr.startsolid && tr.fraction < 1.0f )
		{
			origin = ent->r.currentOrigin;
			if( ++blocked_ >= 40 )
			{
				next_ = (next_ + 1) % count_;
				blocked_ = 0;
			}
		}
		else
		{
			blocked_ = 0;
		}
		from = origin;
		to = origin;
		from[2] += 64;
		to[2] -= 512;
		GameHost::trace( &tr, from, nullptr, nullptr, to, ent->s.number, (cat & CAT_BOAT) ? (MASK_SOLID|MASK_WATER) : MASK_SOLID, false );
		if( tr.fraction < 1.0f )
			origin[2] = tr.endpos[2] - ent->r.mins[2] + 1;
	}

	VectorSubtract( route_[next_], origin, dir );
	if( !flying ) dir[2] = 0;
	float arrive = (cat & CAT_PLANE) ? speed_ : 150;
	if( VectorLength( dir ) < arrive )
	{
		next_ = (next_ + 1) % count_;
		VectorSubtract( route_[next_], origin, dir );
		if( !flying ) dir[2] = 0;
	}
	VectorNormalize( dir );
	vectoangles( dir, bearing );

	VectorCopy( ent->s.angles, angles );
	float step = availableVehicles[vehIndex].turnspeed[YAW] * dt;
	float yawdiff = AngleNormalize180( bearing[YAW] - angles[YAW] );
	if( yawdiff > step ) angles[YAW] += step;
	else if( yawdiff < -step ) angles[YAW] -= step;
	else angles[YAW] = bearing[YAW];
	angles[YAW] = AngleMod( angles[YAW] );

	if( cat & CAT_PLANE )
	{
		float targpitch = AngleNormalize180( bearing[PITCH] );
		if( targpitch > 45 ) targpitch = 45;
		else if( targpitch < -45 ) targpitch = -45;
		float pitch = AngleNormalize180( angles[PITCH] );
		step = availableVehicles[vehIndex].turnspeed[PITCH] * dt;
		if( targpitch - pitch > step ) pitch += step;
		else if( targpitch - pitch < -step ) pitch -= step;
		else pitch = targpitch;
		angles[PITCH] = pitch;

		float targroll = 0;
		if( yawdiff > 1 ) targroll = -60;
		else if( yawdiff < -1 ) targroll = 60;
		float roll = AngleNormalize180( angles[ROLL] );
		step = availableVehicles[vehIndex].turnspeed[ROLL] * dt;
		if( targroll - roll > step ) roll += step;
		else if( targroll - roll < -step ) roll -= step;
		else roll = targroll;
		angles[ROLL] = roll;

		AngleVectors( angles, vel, nullptr, nullptr );
	}
	else
	{
		angles[PITCH] = angles[ROLL] = 0;
		if( cat & CAT_HELO )
		{
			vel = dir;
		}
		else
		{
			AngleVectors( angles, vel, nullptr, nullptr );
		}
	}
	vel = vel * ( speed_ );

	VectorCopy( angles, ent->s.angles );
	VectorCopy( angles, ent->s.apos.trBase );
	ent->r.currentAngles = angles;
	ent->s.apos.trTime = theLevel.time_;

	ent->r.currentOrigin = origin;
	VectorCopy( origin, ent->s.pos.trBase );
	VectorCopy( vel, ent->s.pos.trDelta );
	ent->s.pos.trType = TR_LINEAR;
	ent->s.pos.trTime = theLevel.time_;

	ent->nextthink_ = theLevel.time_ + 50;
	GameHost::linkEntity( ent );
}

void MiscVehicle_SetRoute( GameEntity *ent, const mission_waypoint_t *wps )
{
	int count = 0;
	while( count < IGME_MAX_WAYPOINTS && wps[count].used ) ++count;
	if( !count ) return;

	unsigned int vehIndex = ent->s.modelindex;
	unsigned long cat = availableVehicles[vehIndex].cat;

	auto think = std::make_unique<Drone_Think>();
	for( int i = 0; i < count; ++i )
		VectorCopy( wps[i].origin, think->route_[i] );
	think->count_ = count;
	think->next_ = 0;
	think->blocked_ = 0;

	if( cat & CAT_PLANE ) {
		think->speed_ = availableVehicles[vehIndex].maxspeed * 0.7f;
		ent->s.ONOFF = (availableVehicles[vehIndex].caps & HC_GEAR) ? OO_NOTHING_ON : OO_GEAR;
		ent->s.frame = MF_THROTTLE_MILITARY;
	} else {
		think->speed_ = availableVehicles[vehIndex].maxspeed * 0.5f;
		ent->s.frame = MF_THROTTLE_MILITARY;
	}

	Vec3 dir;
	VectorSubtract( think->route_[0], ent->s.origin, dir );
	if( !(cat & CAT_PLANE) ) dir[2] = 0;
	if( VectorNormalize( dir ) > 0 ) {
		Vec3 angles;
		vectoangles( dir, angles );
		ent->s.angles[YAW] = angles[YAW];
	}
	ent->s.angles[PITCH] = ent->s.angles[ROLL] = 0;
	VectorCopy( ent->s.angles, ent->s.apos.trBase );

	ent->s.eFlags |= EF_PILOT_ONBOARD;
	ent->s.pos.trType = TR_LINEAR;
	VectorClear( ent->s.pos.trDelta );
	ent->s.pos.trTime = theLevel.time_;
	ent->setThink( std::move( think ) );
	ent->nextthink_ = theLevel.time_ + 1000;
}

void Abandoned_Think::execute()
{
	GameEntity* ent = self_;
	Vec3 origin;
	trace_t tr;

	if( ent->s.pos.trType == TR_STATIONARY )
	{
		if( theLevel.time_ > expire_ )
			ent->freeUp();
		else
			ent->nextthink_ = theLevel.time_ + 1000;
		return;
	}

	GameShared::evaluateTrajectory( &ent->s.pos, theLevel.time_, origin );
	GameHost::trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->s.number, MASK_SOLID, false );
	if( tr.startsolid || tr.fraction < 1.0f || theLevel.time_ > expire_ )
	{
		if( tr.fraction < 1.0f )
			VectorCopy( tr.endpos, ent->r.currentOrigin );
		ent->takedamage_ = true;
		Game::damage( ent, nullptr, ent, nullptr, nullptr, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
		return;
	}
	ent->r.currentOrigin = origin;
	ent->nextthink_ = theLevel.time_ + 50;
	GameHost::linkEntity( ent );
}

void MiscVehicle_Abandon( GameEntity *ent )
{
	GameClient* client = ent->client_;
	int vehicle = client->vehicle_;
	bool landed = (client->ps_.ONOFF & OO_LANDED) != 0 ||
		(availableVehicles[vehicle].cat & (CAT_GROUND|CAT_BOAT)) != 0;

	GameEntity* veh = theLevel.spawnEntity();
	veh->classname_ = "misc_vehicle";
	veh->s.eType = ET_MISC_VEHICLE;
	veh->s.modelindex = vehicle;
	veh->s.modelindex2 = 0;
	veh->s.ONOFF = client->ps_.ONOFF & (OO_GEAR|OO_LANDED);
	VectorCopy( availableVehicles[vehicle].mins, veh->r.mins );
	VectorCopy( availableVehicles[vehicle].maxs, veh->r.maxs );
	Game::setOrigin( veh, client->ps_.origin );
	VectorCopy( client->ps_.vehicleAngles, veh->s.angles );
	VectorCopy( veh->s.angles, veh->s.apos.trBase );
	if( !landed )
	{
		veh->s.pos.trType = TR_GRAVITY;
		VectorCopy( client->ps_.velocity, veh->s.pos.trDelta );
		veh->s.pos.trTime = theLevel.time_;
	}
	veh->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	veh->r.contents = 0;
	veh->health_ = 1;
	veh->score_ = 0;
	veh->setDie( std::make_unique<MiscVehicle_Die>());

	auto think = std::make_unique<Abandoned_Think>();
	think->expire_ = theLevel.time_ + (landed ? g_abandonTime.integer * 1000 : 10000);
	if( landed )
	{
		think->enterable_ = true;
		think->fuel_ = client->ps_.stats[STAT_FUEL];
		think->ownerTeam_ = client->sess_.paintTeam_ ? client->sess_.paintTeam_ : client->sess_.sessionTeam_;
		veh->s.generic1 = think->ownerTeam_;
		memcpy( think->ammo_, client->ps_.ammo, sizeof(think->ammo_) );
		memcpy( think->mounts_, client->ps_.numWeaponsOnMount, sizeof(think->mounts_) );
		veh->r.contents = availableVehicles[vehicle].hasDeck ? CONTENTS_CORPSE : CONTENTS_BODY;
		veh->takedamage_ = true;
		veh->health_ = ent->health_;
		veh->s.seatMask = SEATMASK_ENTERABLE;
		veh->s.mountMask = ent->s.mountMask;
	}
	veh->setThink( std::move( think ) );
	client->sess_.paintTeam_ = 0;
	veh->nextthink_ = theLevel.time_ + 50;

	GameHost::linkEntity( veh );

	if( landed && availableVehicles[vehicle].hasDeck ) 
		MiscVehicle_SpawnDecks( veh, vehicle );
}

void Deck_Think::execute()
{
	GameEntity* deck = self_;
	GameEntity* parent = theLevel.getEntity( parentNum_ );
	if( parent != parentPtr_ || !parent || !parent->inuse_ ) 
	{
		deck->freeUp();
		return;
	}

	Vec3 origin, angles, velocity, forward, pos;
	origin = parent->r.currentOrigin;
	VectorCopy( parent->s.angles, angles );
	velocity = Vec3();
	if( parent->client_ ) 
		VectorCopy( parent->client_->ps_.velocity, velocity );

	if( first_ ) 
	{
		for( int i = 1; i <= theLevel.maxclients_; i++ ) 
		{
			GameClient* cl = theLevel.getClient( i );
			GameEntity* ent = theLevel.getEntity( i );
			if( !cl || !ent || cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
				continue;
			if( cl->vehicle_ < 0 || !(availableVehicles[cl->vehicle_].cat & CAT_LQM) || ent->health_ <= 0 || cl->sess_.ridingClient_ >= 0 ) 
				continue;

			bool on = false;
			int ground = cl->ps_.groundEntityNum;
			if( ground > 0 && ground < MAX_GENTITIES ) 
			{
				GameEntity* ge = theLevel.getEntity( ground );
				if( ge && ge->inuse_ && (ge->r.svFlags & SVF_DECK) && ge->s.otherEntityNum == parentNum_ ) 
					on = true;
			}
			if( !on ) 
			{
				if( ent->r.ownerNum == parentNum_ ) 
					ent->r.ownerNum = ENTITYNUM_NONE;
				continue;
			}
			ent->r.ownerNum = parentNum_;
			if( !hasLast_ ) 
				continue;

			vec3_t oldAxis[3], newAxis[3];
			AnglesToAxis( lastAngles_, oldAxis );
			AnglesToAxis( angles, newAxis );
			const Vec3 rel = Vec3( cl->ps_.origin ) - Vec3( lastOrigin_ );
			Vec3 moved( origin );
			for( int k = 0; k < 3; k++ ) 
				moved += Vec3( newAxis[k] ) * rel.dot( Vec3( oldAxis[k] ) );
			moved.copyTo( cl->ps_.origin );
			moved.copyTo( ent->r.currentOrigin );
			moved.copyTo( ent->s.pos.trBase );
			cl->ps_.delta_angles[YAW] += ANGLE2SHORT( AngleSubtract( angles[YAW], lastAngles_[YAW] ) );
			GameHost::linkEntity( ent );
		}
	}

	AngleVectors( angles, forward, nullptr, nullptr );
	pos = origin + forward * ( along_ );

	deck->s.pos.trType = TR_LINEAR;
	deck->s.pos.trTime = theLevel.time_;
	deck->s.pos.trDuration = 0;
	VectorCopy( pos, deck->s.pos.trBase );
	VectorCopy( velocity, deck->s.pos.trDelta );
	deck->r.currentOrigin = pos;

	deck->s.apos.trType = TR_LINEAR;
	deck->s.apos.trTime = theLevel.time_;
	deck->s.apos.trDuration = 0;
	VectorCopy( angles, deck->s.apos.trBase );
	VectorClear( deck->s.apos.trDelta );
	if( hasLast_ && theLevel.time_ > lastTime_ ) 
	{
		float scale = 1000.0f / ( theLevel.time_ - lastTime_ );
		for( int k = 0; k < 3; k++ ) 
			deck->s.apos.trDelta[k] = AngleSubtract( angles[k], lastAngles_[k] ) * scale;
	}
	VectorCopy( angles, deck->s.angles );
	deck->r.currentAngles = angles;

	VectorCopy( origin, lastOrigin_ );
	VectorCopy( angles, lastAngles_ );
	lastTime_ = theLevel.time_;
	hasLast_ = true;

	GameHost::linkEntity( deck );
	deck->nextthink_ = theLevel.time_ + 1;
}

void MiscVehicle_SpawnDecks( GameEntity *parent, int vehicle )
{
	completeVehicleData_t* data = &availableVehicles[vehicle];
	float halfLength = ( data->maxs[0] - data->mins[0] ) * 0.5f;
	float half = ( data->maxs[1] - data->mins[1] ) * 0.45f;
	float centre = ( data->maxs[0] + data->mins[0] ) * 0.5f;
	if( half > 120 ) half = 120;
	if( half > halfLength ) half = halfLength;
	int count = static_cast<int>(ceil( halfLength / half ));
	if( count > MAX_VEHICLE_DECKS ) count = MAX_VEHICLE_DECKS;

	for( int i = 0; i < count; i++ ) 
	{
		GameEntity* deck = theLevel.spawnEntity();
		deck->classname_ = "vehicle_deck";
		deck->s.eType = ET_INVISIBLE;
		deck->s.seatMask = SEATMASK_DECK;
		deck->s.otherEntityNum = parent->s.number;
		deck->r.ownerNum = parent->s.number;
		deck->r.svFlags = SVF_DECK | SVF_USE_CURRENT_ORIGIN;
		VectorSet( deck->r.mins, -half, -half, data->mins[2] );
		VectorSet( deck->r.maxs, half, half, data->deckPos[2] );
		deck->r.contents = CONTENTS_BODY;
		deck->takedamage_ = true;
		deck->health_ = 1;

		auto think = std::make_unique<Deck_Think>();
		think->parentNum_ = parent->s.number;
		think->parentPtr_ = parent;
		think->along_ = count > 1 ? centre - (halfLength - half) + i * 2 * (halfLength - half) / (count - 1) : centre;
		think->first_ = i == 0;
		deck->setThink( std::move( think ) );
		Game::setOrigin( deck, parent->r.currentOrigin );
		deck->thinkFunc_->execute();
	}
}

void DroneInit()
{
	// finish placing the vehicles spawned by the mission
	for( int i = 0; i < theLevel.num_entities_; ++i ) {
		GameEntity* ent = theLevel.getEntity(i);
		if( !ent || !ent->inuse_ || !ent->classname_ ) continue;
		if( Q_stricmp( ent->classname_, "misc_vehicle" ) != 0 ) continue;
		if( ent->s.modelindex == 255 ) continue; // ground installation
		unsigned long cat = availableVehicles[ent->s.modelindex].cat;
		if( (ent->s.eFlags & EF_PILOT_ONBOARD) && ((cat & CAT_PLANE) || (cat & CAT_HELO)) ) continue;
		if( cat & CAT_PLANE ) {
			Game::sP_misc_plane( ent );
		} else if( cat & CAT_HELO ) {
			Game::sP_misc_helo( ent );
		} else if( cat & CAT_BOAT ) {
			Game::sP_misc_boat( ent );
		} else {
			Game::sP_misc_gv( ent );
		}
	}
}


void Game::miscVehicle_SetTeam( GameEntity *ent )
{
	if( ent->team_ && g_gametype.integer >= GT_TEAM ) {
		if ( Q_stricmp( ent->team_, "red" ) == 0 ) {
			ent->s.generic1 = ClientBase::TEAM_RED;
		} else if ( Q_stricmp( ent->team_, "blue" ) == 0 ) {
			ent->s.generic1 = ClientBase::TEAM_BLUE;
		}
	}
}

/*QUAKED misc_vehicle (0 .5 .8) (-32 -32 0) (32 32 32)
*/
GameEntity* SP_misc_vehicle( GameEntity *sp_ent )
{
	int i = sp_ent->s.modelindex;

	if( i < 0 || i >= bg_numberOfVehicles )
	{
		Common::printf ("%s (%s) cannot be spawned - index out of range\n", sp_ent->classname_, sp_ent->model_);
		sp_ent->freeUp();
		return nullptr;
	}

	GameEntity* ent = theLevel.spawnEntity();

	ent->model_ = sp_ent->model_;
	ent->targetname_ = sp_ent->targetname_;
	ent->team_ = sp_ent->team_;
	VectorCopy( sp_ent->s.origin, ent->s.origin );
	VectorCopy( sp_ent->s.angles, ent->s.angles );
	ent->health_ = sp_ent->health_;

	ent->s.eType = ET_MISC_VEHICLE;
	ent->s.modelindex = i;

	Game::miscVehicle_SetTeam( ent );

	VectorCopy(availableVehicles[i].mins, ent->r.mins);
	VectorCopy(availableVehicles[i].maxs, ent->r.maxs);
	Game::setOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.apos.trType = TR_STATIONARY;
	if( ent->health_ <= 0 ) ent->health_ = availableVehicles[i].maxhealth;
	ent->takedamage_ = true;
	ent->inuse_ = true;
	ent->setDie( std::make_unique<MiscVehicle_Die>());
	if( !ent->score_ ) ent->score_ = 1;
	ent->classname_ = "misc_vehicle";
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask_ = MASK_PLAYERSOLID;

	ent->idxScriptBegin_ = ent->idxScriptEnd_ = -1;

	GameHost::linkEntity (ent);

	sp_ent->freeUp();

	return ent;
}

void SP_misc_groundinstallation( GameEntity *sp_ent )
{
	int i = sp_ent->s.modelindex2;

	if( i < 0 || i >= bg_numberOfGroundInstallations )
	{
		Common::printf ("%s (%s) cannot be spawned - index out of range\n", sp_ent->classname_, sp_ent->model_);
		sp_ent->freeUp();
		return;
	}

	GameEntity* ent = theLevel.spawnEntity();

	ent->model_ = sp_ent->model_;
	ent->targetname_ = sp_ent->targetname_;
	ent->team_ = sp_ent->team_;
	VectorCopy( sp_ent->s.origin, ent->s.origin );
	VectorCopy( sp_ent->s.angles, ent->s.angles );
	ent->health_ = sp_ent->health_;

	ent->s.eType = ET_MISC_VEHICLE;
	ent->s.modelindex = 255;
	ent->s.modelindex2 = i;

	Game::miscVehicle_SetTeam( ent );

	VectorCopy(availableGroundInstallations[i].mins, ent->r.mins);
	VectorCopy(availableGroundInstallations[i].maxs, ent->r.maxs);
	Game::setOrigin( ent, ent->s.origin );
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	ent->s.groundEntityNum = ENTITYNUM_NONE;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.pos.trType = TR_STATIONARY;
	ent->s.apos.trType = TR_STATIONARY;
	if( ent->health_ <= 0 ) ent->health_ = availableGroundInstallations[i].maxhealth;
	ent->takedamage_ = true;
	ent->inuse_ = true;
	ent->setDie( std::make_unique<GroundInstallation_Die>());
	if( !ent->score_ ) ent->score_ = 1;
	ent->classname_ = "misc_vehicle";
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask_ = MASK_PLAYERSOLID;

	ent->idxScriptBegin_ = ent->idxScriptEnd_ = -1;

	ent->s.weaponIndex = availableGroundInstallations[i].weapon;
	ent->count_ = availableGroundInstallations[i].ammo;

	ent->s.eFlags |= EF_PILOT_ONBOARD;
	ent->setThink( std::make_unique<GroundInstallation_Think>());
	ent->nextthink_ = theLevel.time_ + 1000;

	ent->s.ONOFF = OO_RADAR;

	GameHost::linkEntity (ent);

	sp_ent->freeUp();
}
