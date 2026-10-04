/*
 * $Id: cg_ents.c,v 1.4 2005-11-21 17:28:20 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"


/*
======================
ClientGame::positionEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void ClientGame::positionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	
	// lerp the tag
	refExport.LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, (const_cast<refEntity_t *>(parent))->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}


/*
======================
ClientGame::positionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void ClientGame::positionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							qhandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	vec3_t			tempAxis[3];

//AxisClear( entity->axis );
	// lerp the tag
	refExport.LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( entity->axis, lerped.axis, tempAxis );
	MatrixMultiply( tempAxis, (const_cast<refEntity_t *>(parent))->axis, entity->axis );
}



/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
void ClientEntity::setEntitySoundPosition() {
	if ( this->currentState.solid == SOLID_BMODEL ) {
		vec3_t	origin;
		float	*v;

		v = cgs.inlineModelMidpoints[ this->currentState.modelindex ];
		VectorAdd( this->lerpOrigin, v, origin );
		SoundSystem::updateEntityPosition( this->currentState.number, origin );
	} else {
		SoundSystem::updateEntityPosition( this->currentState.number, this->lerpOrigin );
	}
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
void ClientEntity::entityEffects() {

	// update sound origins
	this->setEntitySoundPosition();

	// add loop sound
	if ( this->currentState.loopSound ) {
		if (this->currentState.eType != ET_SPEAKER) {
			SoundSystem::addLoopingSound( this->currentState.number, this->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ this->currentState.loopSound ] );
		} else {
			SoundSystem::addRealLoopingSound( this->currentState.number, this->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ this->currentState.loopSound ] );
		}
	}


	// constant light glow
/*	if ( this->currentState.constantLight ) {
		int		cl;
		int		i, r, g, b;

		cl = this->currentState.constantLight;
		r = cl & 255;
		g = ( cl >> 8 ) & 255;
		b = ( cl >> 16 ) & 255;
		i = ( ( cl >> 24 ) & 255 ) * 4;
		refExport.AddLightToScene( this->lerpOrigin, i, r, g, b );
	}*/

}


/*
==================
CG_General
==================
*/
void ClientEntity::general() {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &this->currentState;

	// if set to invisible, skip
	if (!s1->modelindex) {
		return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( this->lerpOrigin, ent.origin);
	VectorCopy( this->lerpOrigin, ent.oldorigin);

	ent.hModel = cgs.gameModels[s1->modelindex];

	// player model
	if (s1->number == cg.snap->ps.clientNum) {
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis( this->lerpAngles, ent.axis );

	// add to refresh list
	refExport.AddRefEntityToScene (&ent);
}

/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
void ClientEntity::speaker() {
	if ( ! this->currentState.clientNum ) {	// FIXME: use something other than clientNum...
		return;		// not auto triggering
	}

	if ( cg.time < this->miscTime ) {
		return;
	}

	SoundSystem::startSound (nullptr, this->currentState.number, CHAN_ITEM, cgs.gameSounds[this->currentState.eventParm] );

	//	ent->s.frame = ent->wait * 10;
	//	ent->s.clientNum = ent->random * 10;
	this->miscTime = cg.time + this->currentState.frame * 100 + this->currentState.clientNum * 100 * crandom();
}

/*
==================
CG_Item
==================
*/
void ClientEntity::item() {
	refEntity_t		ent;
	entityState_t	*es;
	gitem_t			*item;
	int				msec;
	float			frac;
	float			scale;

	es = &this->currentState;
	if ( es->modelindex >= bg_numItems ) {
		Common::error( ERR_DROP, "Bad item index %i on entity", es->modelindex );
	}

	// if set to invisible, skip
	if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) ) {
		return;
	}

	item = &bg_itemlist[ es->modelindex ];
	if ( cg_simpleItems.integer && item->giType != IT_TEAM ) {
		memset( &ent, 0, sizeof( ent ) );
		ent.reType = RT_SPRITE;
		VectorCopy( this->lerpOrigin, ent.origin );
		ent.radius = 14;
		ent.customShader = cg_items[es->modelindex].icon;
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 255;
		refExport.AddRefEntityToScene(&ent);
		return;
	}

	// items bob up and down continuously
	scale = 0.005 + this->currentState.number * 0.00001;
	this->lerpOrigin[2] += 4 + cos( ( cg.time + 1000 ) *  scale ) * 4;

	memset (&ent, 0, sizeof(ent));

	// autorotate at one of two speeds
	if ( item->giType == IT_HEALTH ) {
		VectorCopy( cg.autoAnglesFast, this->lerpAngles );
		AxisCopy( cg.autoAxisFast, ent.axis );
	} else {
		VectorCopy( cg.autoAngles, this->lerpAngles );
		AxisCopy( cg.autoAxis, ent.axis );
	}

	ent.hModel = cg_items[es->modelindex].models[0];

	VectorCopy( this->lerpOrigin, ent.origin);
	VectorCopy( this->lerpOrigin, ent.oldorigin);

	ent.nonNormalizedAxes = false;

	// if just respawned, slowly scale up
	msec = cg.time - this->miscTime;
	if ( msec >= 0 && msec < ITEM_SCALEUP_TIME ) {
		frac = static_cast<float>(msec) / ITEM_SCALEUP_TIME;
		VectorScale( ent.axis[0], frac, ent.axis[0] );
		VectorScale( ent.axis[1], frac, ent.axis[1] );
		VectorScale( ent.axis[2], frac, ent.axis[2] );
		ent.nonNormalizedAxes = true;
	} else {
		frac = 1.0;
	}

	// add to refresh list
	refExport.AddRefEntityToScene(&ent);

	// accompanying rings / spheres for powerups
	if ( !cg_simpleItems.integer ) 
	{
		vec3_t spinAngles;

		VectorClear( spinAngles );

		if ( item->giType == IT_HEALTH )
		{
			if ( ( ent.hModel = cg_items[es->modelindex].models[1] ) != 0 )
			{
				AnglesToAxis( spinAngles, ent.axis );
				
				// scale up if respawning
				if ( frac != 1.0 ) {
					VectorScale( ent.axis[0], frac, ent.axis[0] );
					VectorScale( ent.axis[1], frac, ent.axis[1] );
					VectorScale( ent.axis[2], frac, ent.axis[2] );
					ent.nonNormalizedAxes = true;
				}
				refExport.AddRefEntityToScene( &ent );
			}
		}
	}
}

//============================================================================

/*
===============
CG_Missile
===============
*/
void ClientEntity::missile() {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t		*weapon;

	s1 = &this->currentState;
	if ( s1->weaponNum > WP_WEAPON6 ) {
		s1->weaponNum = 0;
	}
	weapon = &cg_weapons[s1->weaponIndex];

	// calculate the axis
	VectorCopy( s1->angles, this->lerpAngles);

	// add trails
	if ( weapon->missileTrailFunc ) 
	{
		// only ever call the missile-trail function once per NEW-frame
		// NOTE: this assumes that we don't use the ->miscTime variable of ClientEntity (missiles) for anything else
		if( this->miscTime != cg.time )
		{
			// call the trail func
			weapon->missileTrailFunc( this, weapon );

			// lock-out the frame
			this->miscTime = cg.time;
		}
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		refExport.AddLightToScene(this->lerpOrigin, weapon->missileDlight, 
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2] );
	}

	// add missile sound
	if ( weapon->missileSound ) {
		vec3_t	velocity;

		GameShared::evaluateTrajectoryDelta( &this->currentState.pos, cg.time, velocity );

		SoundSystem::addLoopingSound( this->currentState.number, this->lerpOrigin, velocity, weapon->missileSound );
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( this->lerpOrigin, ent.origin);
	VectorCopy( this->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->missileModel;
	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
		ent.axis[0][2] = 1;
	}

	// flame frame
	if( availableWeapons[this->currentState.weaponIndex].flags & WF_HAS_FIRE_FRAME ) {
		ent.frame = 1;
	}

	// spin as it moves
	if ( availableWeapons[this->currentState.weaponIndex].type == WT_AMMOCRATE || 
		availableWeapons[this->currentState.weaponIndex].type == WT_HEALTHCRATE ||  
		availableWeapons[this->currentState.weaponIndex].type == WT_FUELCRATE) {
		RotateAroundDirection( ent.axis, 0 );
	} else if ( s1->pos.trType != TR_STATIONARY && 
		availableWeapons[this->currentState.weaponIndex].type != WT_IRONBOMB &&
		availableWeapons[this->currentState.weaponIndex].type != WT_GUIDEDBOMB &&
		availableWeapons[this->currentState.weaponIndex].type != WT_NUKEBOMB && 
		availableWeapons[this->currentState.weaponIndex].type != WT_NUKEMISSILE ) {
	
		RotateAroundDirection( ent.axis, cg.time / 4 );
	} else {
		RotateAroundDirection( ent.axis, s1->time );
	}
	

	// add to refresh list
	refExport.AddRefEntityToScene( &ent );

}

/*
===============
CG_Bullet_Tracer
===============
*/
void ClientEntity::bullet_Tracer() {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t		*weapon;

	s1 = &this->currentState;
	if ( s1->weaponNum > WP_WEAPON6 ) {
		s1->weaponNum = 0;
	}
	weapon = &cg_weapons[s1->weaponIndex];

	// calculate the axis
	VectorCopy( s1->angles, this->lerpAngles);

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( this->lerpOrigin, ent.origin);
	VectorCopy( this->lerpOrigin, ent.oldorigin);

	ent.hModel = cgs.media.bulletTracer;
	ent.frame = 2;
	ent.renderfx = RF_NOSHADOW;

	// convert direction of travel into axis
	if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 ) {
		ent.axis[0][2] = 1;
	}
	RotateAroundDirection( ent.axis, s1->time );

	refExport.AddRefEntityToScene( &ent );
}

/*
===============
CG_Mover
===============
*/
void ClientEntity::mover() {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &this->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( this->lerpOrigin, ent.origin);
	VectorCopy( this->lerpOrigin, ent.oldorigin);
	AnglesToAxis( this->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// add to refresh list
	refExport.AddRefEntityToScene(&ent);

	// add the secondary model
	if ( s1->modelindex2 ) {
		ent.skinNum = 0;
		ent.hModel = cgs.gameModels[s1->modelindex2];
		refExport.AddRefEntityToScene(&ent);
	}

}

/*
===============
CG_Explosive
===============
*/
void ClientEntity::explosive() {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &this->currentState;

	// radar
	if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND) 
		&& (this->currentState.ONOFF & OO_RADAR) ) {
		this->destroyableStructure = true;
		cg.radarEnts[cg.radarTargets++] = this;
	}

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( this->lerpOrigin, ent.origin);
	VectorCopy( this->lerpOrigin, ent.oldorigin);
	AnglesToAxis( this->lerpAngles, ent.axis );

//	ent.renderfx = RF_NOSHADOW;

	// flicker between two skins (FIXME?)
//	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// add to refresh list
	refExport.AddRefEntityToScene(&ent);

}


/*
===============
CG_Beam

Also called as an event
===============
*/
void ClientEntity::beam() {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &this->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( s1->pos.trBase, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AxisClear( ent.axis );
	ent.reType = RT_BEAM;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	refExport.AddRefEntityToScene(&ent);
}


/*
===============
CG_Portal
===============
*/
void ClientEntity::portal() {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &this->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( this->lerpOrigin, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	ByteToDir( s1->eventParm, ent.axis[0] );
	PerpendicularVector( ent.axis[1], ent.axis[0] );

	// negating this tends to get the directions like they want
	// we really should have a camera roll value
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

	CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	ent.reType = RT_PORTALSURFACE;
	ent.oldframe = s1->objectives;
	ent.frame = s1->frame;		// rotation speed
	ent.skinNum = s1->clientNum/256.0 * 360;	// roll offset

	// add to refresh list
	refExport.AddRefEntityToScene(&ent);
}


/*
================
CG_CreateRotationMatrix
================
*/
void CG_CreateRotationMatrix(vec3_t angles, vec3_t matrix[3]) {
	AngleVectors(angles, matrix[0], matrix[1], matrix[2]);
	VectorInverse(matrix[1]);
}

/*
================
CG_TransposeMatrix
================
*/
void CG_TransposeMatrix(vec3_t matrix[3], vec3_t transpose[3]) {
	int i, j;
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) {
			transpose[i][j] = matrix[j][i];
		}
	}
}

/*
================
CG_RotatePoint
================
*/
void CG_RotatePoint(vec3_t point, vec3_t matrix[3]) {
	vec3_t tvec;

	VectorCopy(point, tvec);
	point[0] = DotProduct(matrix[0], tvec);
	point[1] = DotProduct(matrix[1], tvec);
	point[2] = DotProduct(matrix[2], tvec);
}

/*
=========================
ClientGame::adjustPositionForMover

Also called by client movement prediction code
=========================
*/
void ClientGame::adjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out ) {
	ClientEntity	*cent;
	vec3_t	oldOrigin, origin, deltaOrigin;
	vec3_t	oldAngles, angles, deltaAngles;
	vec3_t	matrix[3], transpose[3];
	vec3_t	org, org2, move2;

	if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) {
		VectorCopy( in, out );
		return;
	}

	cent = &cg_entities[ moverNum ];
	if ( cent->currentState.eType != ET_MOVER && !(cent->currentState.seatMask & SEATMASK_DECK) ) {
		VectorCopy( in, out );
		return;
	}

	GameShared::evaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	GameShared::evaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	GameShared::evaluateTrajectory( &cent->currentState.pos, toTime, origin );
	GameShared::evaluateTrajectory( &cent->currentState.apos, toTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );
	VectorSubtract( angles, oldAngles, deltaAngles );

	// origin change when on a rotating object
	CG_CreateRotationMatrix( deltaAngles, transpose );
	CG_TransposeMatrix( transpose, matrix );
	VectorSubtract( in, oldOrigin, org );
	VectorCopy( org, org2 );
	CG_RotatePoint( org2, matrix );
	VectorSubtract( org2, org, move2 );
	VectorAdd( deltaOrigin, move2, deltaOrigin );

	VectorAdd( in, deltaOrigin, out );
}


/*
=============================
CG_InterpolateEntityPosition
=============================
*/
float ClientEntity::entityAngleErrorFraction() const {
	float	f;

	if ( !this->errorTime ) {
		return 0;
	}
	f = 1.0f - ( cg.time - this->errorTime ) / static_cast<float>(ANGLE_ERROR_DECAY);
	if ( f > 1.0f ) {
		f = 1.0f;
	}
	return f;
}

void ClientEntity::interpolateEntityPosition() {
	vec3_t		current, next;
	float		f;

	// it would be an internal error to find an entity that interpolates without
	// a snapshot ahead of the current one
	if ( cg.nextSnap == nullptr ) {
		Common::error( ERR_DROP, "CG_InterpoateEntityPosition: cg.nextSnap == NULL" );
	}

	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	GameShared::evaluateTrajectory( &this->currentState.pos, cg.snap->serverTime, current );
	GameShared::evaluateTrajectory( &this->nextState.pos, cg.nextSnap->serverTime, next );

	this->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	this->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	this->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	GameShared::evaluateTrajectory( &this->currentState.apos, cg.snap->serverTime, current );
	GameShared::evaluateTrajectory( &this->nextState.apos, cg.nextSnap->serverTime, next );

	this->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	this->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	this->lerpAngles[2] = LerpAngle( current[2], next[2], f );

}

/*
=============================
CG_VehicleLerpAngles

=============================
*/
void ClientEntity::vehicleLerpAngles() {
	quat_t	current, next, out;
	float	f;

	if ( this == &cg.predictedPlayerEntity || !this->interpolate || !cg.nextSnap ) {
		VectorCopy( this->currentState.angles, this->lerpAngles );
		VectorCopy( this->currentState.angles2, this->lerpAngles2 );
		return;
	}

	AnglesToQuat( this->nextState.angles, next );

	if ( this->errorSnapTime == cg.nextSnap->serverTime ) {
		QuatCopy( next, out );
		VectorCopy( this->nextState.angles2, this->lerpAngles2 );
	} else {
		f = cg.frameInterpolation;
		AnglesToQuat( this->currentState.angles, current );
		QuatInterpolate( current, next, f, out );
		this->lerpAngles2[0] = LerpAngle( this->currentState.angles2[0], this->nextState.angles2[0], f );
		this->lerpAngles2[1] = LerpAngle( this->currentState.angles2[1], this->nextState.angles2[1], f );
		this->lerpAngles2[2] = LerpAngle( this->currentState.angles2[2], this->nextState.angles2[2], f );
	}

	f = this->entityAngleErrorFraction();
	if ( f > 0 ) {
		AnglesToQuat( this->errorAngles, current );
		QuatInterpolate( out, current, f, out );
		VectorMA( this->lerpAngles2, f, this->errorAngles2, this->lerpAngles2 );
	}

	QuatToAngles( out, this->lerpAngles );
}

/*
===============
CG_CalcEntityLerpPositions

===============
*/
void ClientEntity::calcEntityLerpPositions() {

	// if this player does not want to see extrapolated players
	if ( !cg_smoothClients.integer ) {
		// make sure the clients use TR_INTERPOLATE
		if ( this->currentState.number <= MAX_CLIENTS ) {
			this->currentState.pos.trType = TR_INTERPOLATE;
			this->nextState.pos.trType = TR_INTERPOLATE;
		}
	}

	if ( this->interpolate && this->currentState.pos.trType == TR_INTERPOLATE ) {
		this->interpolateEntityPosition();
		return;
	}

	// first see if we can interpolate between two snaps for
	// linear extrapolated clients
	if ( this->interpolate && this->currentState.pos.trType == TR_LINEAR_STOP &&
											this->currentState.number <= MAX_CLIENTS) {
		this->interpolateEntityPosition();
		return;
	}

	// just use the current frame and evaluate as best we can
	GameShared::evaluateTrajectory( &this->currentState.pos, cg.time, this->lerpOrigin );
	GameShared::evaluateTrajectory( &this->currentState.apos, cg.time, this->lerpAngles );

	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	if ( this != &cg.predictedPlayerEntity ) {
		ClientGame::adjustPositionForMover( this->lerpOrigin, this->currentState.groundEntityNum, 
		cg.snap->serverTime, cg.time, this->lerpOrigin );
	}
}

void ClientEntity::emptyVehicleIcon()
{
	static qhandle_t	icon;
	refEntity_t			ent;
	int					vehicle = this->currentState.modelindex;
	static const byte	colors[3][3] = { { 255, 255, 255 }, { 255, 70, 60 }, { 70, 130, 255 } };
	int					team = this->currentState.generic1;

	if( vehicle < 0 || vehicle >= bg_numberOfVehicles ) 
		return;
	if( !icon ) 
		icon = refExport.RegisterShader( "gfx/hud/vehicle_empty" );
	if( team < 0 || team > 2 || cgs.gametype < GT_TEAM ) 
		team = 0;

	memset( &ent, 0, sizeof(ent) );
	VectorCopy( this->lerpOrigin, ent.origin );
	ent.origin[2] += availableVehicles[vehicle].maxs[2] + 10;
	ent.reType = RT_SPRITE;
	ent.customShader = icon;
	ent.radius = 6;
	ent.renderfx = RF_DEPTHHACK;
	ent.shaderRGBA[0] = colors[team][0];
	ent.shaderRGBA[1] = colors[team][1];
	ent.shaderRGBA[2] = colors[team][2];
	ent.shaderRGBA[3] = 255;
	refExport.AddRefEntityToScene( &ent );
}

/*
===============
CG_AddCEntity

===============
*/
void ClientEntity::addCEntity() {
	// event-only entities will have been dealt with already
	if ( this->currentState.eType >= ET_EVENTS ) {
		return;
	}

	// calculate the current origin
	this->calcEntityLerpPositions();

	// add automatic effects
	this->entityEffects();

	switch ( this->currentState.eType ) {
	default:
		Common::error( ERR_DROP, "Bad entity type: %i\n", this->currentState.eType );
		break;
	case ET_INVISIBLE:
	case ET_TELEPORT_TRIGGER:
		break;
	case ET_GENERAL:
		this->general();
		break;
	case ET_ITEM:
		this->item();
		break;
	case ET_MISSILE:
		this->missile();
		break;
	case ET_BULLET:
		if( !(this->currentState.generic1 % cg.tracernum) ) {
			this->bullet_Tracer();
		}
		break;
	case ET_MOVER:
		this->mover();
		break;
	case ET_EXPLOSIVE:
		this->explosive();
		break;
	case ET_BEAM:
		this->beam();
		break;
	case ET_PORTAL:
		this->portal();
		break;
	case ET_SPEAKER:
		this->speaker();
		break;
	case ET_VEHICLE:
		this->vehicle();
		break;
	case ET_MISC_VEHICLE:
		this->misc_Vehicle();
		if( (this->currentState.seatMask & SEATMASK_ENTERABLE) && !(this->currentState.seatMask & SEATMASK_PILOT) ) 
			this->emptyVehicleIcon();
		break;
	}
}

/*
===============
ClientGame::addPacketEntities

===============
*/
void ClientGame::addPacketEntities( ) {
	int					num;
	ClientEntity			*cent;
	playerState_t		*ps;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) {
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) {
			cg.frameInterpolation = 0;
		} else {
			cg.frameInterpolation = static_cast<float>( cg.time - cg.snap->serverTime ) / delta;
		}
	} else {
		cg.frameInterpolation = 0;	// actually, it should never be used, because 
									// no entities should be marked as interpolating
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

	// generate and add the entity from the playerstate
	ps = &cg.predictedPlayerState;
	GameShared::playerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, false );
	if( !(cg.predictedPlayerState.pm_flags & PMF_VEHICLESELECT) ) {
		cg.predictedPlayerEntity.addCEntity();
	}

	// lerp the non-predicted value for lightning gun origins
	(&cg_entities[ cg.snap->ps.clientNum ])->calcEntityLerpPositions();

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		cent->addCEntity();
	}
}

