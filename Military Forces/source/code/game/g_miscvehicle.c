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

static int MiscVehicle_TeamScore( GameEntity *self, GameEntity *attacker )
{
	if( g_gametype.integer >= GT_TEAM && self->s.generic1 == attacker->client_->ps_.persistant[PERS_TEAM] )
		return -self->score_;
	return self->score_;
}

void MiscVehicle_Die::execute( GameEntity *inflictor, GameEntity *attacker, int damage, int meansOfDeath )
{
	if( self_->score_ && attacker && attacker->client_ ) {
		vec3_t	pos;
		VectorAdd( self_->r.absmin, self_->r.absmax, pos );
		VectorScale( pos, 0.5f, pos );
		AddScore( attacker, pos, MiscVehicle_TeamScore( self_, attacker ) );
	}

	ExplodeVehicle(self_);
	self_->freeAfterEvent_ = true;

	G_RadiusDamage( self_->r.currentOrigin, self_, 150, 150, self_, MOD_VEHICLEEXPLOSION, CAT_ANY );

	SV_LinkEntity( self_ );
}

static void MiscVehicle_DropToGround( GameEntity *ent, float above, float below )
{
	trace_t	trace;
	vec3_t	startpos, endpos;
	unsigned int vehIndex = ent->s.modelindex;

	VectorCopy( ent->s.origin, startpos );
	startpos[2] += above;
	VectorCopy( ent->s.origin, endpos );
	endpos[2] -= below;
	SV_Trace( &trace, startpos, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_SOLID, false );
	if( trace.fraction < 1.0f ) {
		ent->s.origin[2] = trace.endpos[2] - availableVehicles[vehIndex].mins[2] + 1;
	}
}

static void SP_misc_plane( GameEntity *ent )
{
	unsigned int vehIndex = ent->s.modelindex;
	trace_t	trace;
	vec3_t	startpos, endpos;

	VectorCopy( ent->s.origin, startpos );
	startpos[2] += 64;
	VectorCopy( ent->s.origin, endpos );
	endpos[2] -= 256;
	SV_Trace( &trace, startpos, NULL, NULL, endpos, ENTITYNUM_NONE, MASK_SOLID, false );
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
		ent->gearheight_ = (float)availableVehicles[vehIndex].gearheight;
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
	SV_LinkEntity (ent);
}

static void SP_misc_gv( GameEntity *ent )
{
	MiscVehicle_DropToGround( ent, 64, 512 );

	// set flags
	ent->s.ONOFF = OO_LANDED;

	ent->speed_ = 0;
	ent->s.frame = 0;// throttle

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	SV_LinkEntity (ent);
}

static void SP_misc_helo( GameEntity *ent )
{
	unsigned int vehIndex = ent->s.modelindex;

	MiscVehicle_DropToGround( ent, 64, 512 );

	// set flags
	ent->s.ONOFF = OO_LANDED;

	// gearheight
	if( availableVehicles[vehIndex].caps & HC_GEAR ) {
		ent->gearheight_ = (float)availableVehicles[vehIndex].gearheight;
	} else {
		ent->s.ONOFF |= OO_GEAR;
	}

	ent->speed_ = 0;
	ent->s.frame = 0;// throttle

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	SV_LinkEntity (ent);
}

static void SP_misc_boat( GameEntity *ent )
{
	MiscVehicle_DropToGround( ent, 64, 512 );

	// set flags
	ent->s.ONOFF = OO_LANDED;

	ent->speed_ = 0;
	ent->s.frame = 0;// throttle

	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	SV_LinkEntity (ent);
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
		if( cat & CAT_PLANE ) {
			SP_misc_plane( ent );
		} else if( cat & CAT_HELO ) {
			SP_misc_helo( ent );
		} else if( cat & CAT_BOAT ) {
			SP_misc_boat( ent );
		} else {
			SP_misc_gv( ent );
		}
	}
}

static void MiscVehicle_SetTeam( GameEntity *ent )
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
void SP_misc_vehicle( GameEntity *sp_ent )
{
	int i = sp_ent->s.modelindex;

	if( i < 0 || i >= bg_numberOfVehicles )
	{
		Com_Printf ("%s (%s) cannot be spawned - index out of range\n", sp_ent->classname_, sp_ent->model_);
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
	ent->s.modelindex = i;

	MiscVehicle_SetTeam( ent );

	VectorCopy(availableVehicles[i].mins, ent->r.mins);
	VectorCopy(availableVehicles[i].maxs, ent->r.maxs);
	G_SetOrigin( ent, ent->s.origin );
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
	ent->setDie( new MiscVehicle_Die );
	if( !ent->score_ ) ent->score_ = 1;
	ent->classname_ = "misc_vehicle";
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask_ = MASK_PLAYERSOLID;

	ent->idxScriptBegin_ = ent->idxScriptEnd_ = -1;

	SV_LinkEntity (ent);

	sp_ent->freeUp();
}

void SP_misc_groundinstallation( GameEntity *sp_ent )
{
	int i = sp_ent->s.modelindex2;

	if( i < 0 || i >= bg_numberOfGroundInstallations )
	{
		Com_Printf ("%s (%s) cannot be spawned - index out of range\n", sp_ent->classname_, sp_ent->model_);
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

	MiscVehicle_SetTeam( ent );

	VectorCopy(availableGroundInstallations[i].mins, ent->r.mins);
	VectorCopy(availableGroundInstallations[i].maxs, ent->r.maxs);
	G_SetOrigin( ent, ent->s.origin );
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
	ent->setDie( new GroundInstallation_Die );
	if( !ent->score_ ) ent->score_ = 1;
	ent->classname_ = "misc_vehicle";
	ent->r.contents = CONTENTS_SOLID;
	ent->clipmask_ = MASK_PLAYERSOLID;

	ent->idxScriptBegin_ = ent->idxScriptEnd_ = -1;

	ent->s.weaponIndex = availableGroundInstallations[i].weapon;
	ent->count_ = availableGroundInstallations[i].ammo;

	ent->s.eFlags |= EF_PILOT_ONBOARD;
	ent->setThink( new GroundInstallation_Think );
	ent->nextthink_ = theLevel.time_ + 1000;

	ent->s.ONOFF = OO_RADAR;

	SV_LinkEntity (ent);

	sp_ent->freeUp();
}
