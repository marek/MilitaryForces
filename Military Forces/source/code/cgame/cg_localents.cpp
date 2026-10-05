/*
 * $Id: cg_localents.c,v 1.6 2005-11-21 17:28:20 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

// cg_localents.c -- every frame, generate renderer commands for locally
// processed entities, like smoke puffs, gibs, shells, etc.

#include "cg_local.h"
#include <algorithm>

constexpr int MAX_LOCAL_ENTITIES = 2048;
LocalEntity	cg_localEntities[MAX_LOCAL_ENTITIES];
LocalEntity	cg_activeLocalEntities;		// double linked list
LocalEntity	*cg_freeLocalEntities;		// single linked list

/*
===================
CGame::initLocalEntities

This is called at startup and for tournement restarts
===================
*/
void	CGame::initLocalEntities( ) {
	int		i;

	memset( cg_localEntities, 0, sizeof( cg_localEntities ) );
	cg_activeLocalEntities.next = &cg_activeLocalEntities;
	cg_activeLocalEntities.prev = &cg_activeLocalEntities;
	cg_freeLocalEntities = cg_localEntities;
	for ( i = 0 ; i < MAX_LOCAL_ENTITIES - 1 ; i++ ) {
		cg_localEntities[i].next = &cg_localEntities[i+1];
	}
}


/*
==================
CG_FreeLocalEntity
==================
*/
void LocalEntity::freeLocalEntity() {
	if ( !this->prev ) {
		Common::error( ERR_DROP, "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	this->prev->next = this->next;
	this->next->prev = this->prev;

	// the free list is only singly linked
	this->next = cg_freeLocalEntities;
	cg_freeLocalEntities = this;

	// remove any particle systems/emitters attached to this system

}

/*
===================
CGame::allocLocalEntity

Will allways succeed, even if it requires freeing an old active entity
===================
*/
LocalEntity	*CGame::allocLocalEntity( ) {
	LocalEntity	*le;

	if ( !cg_freeLocalEntities ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		cg_activeLocalEntities.prev->freeLocalEntity();
	}

	le = cg_freeLocalEntities;
	cg_freeLocalEntities = cg_freeLocalEntities->next;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->next = cg_activeLocalEntities.next;
	le->prev = &cg_activeLocalEntities;
	cg_activeLocalEntities.next->prev = le;
	cg_activeLocalEntities.next = le;

	return le;
}


/*
====================================================================================

FRAGMENT PROCESSING

A fragment localentity interacts with the environment in some way (hitting walls),
or generates more localentities along a trail.

====================================================================================
*/


/*
================
CG_FragmentBounceMark
================
*/
void LocalEntity::fragmentBounceMark( trace_t *trace ) {
	int			radius;

	radius = 8 + (rand()&15);
	CGame::impactMark( cgs.media.burnMarkShader, trace->endpos, trace->plane.normal, random()*360,
			1,1,1,1, true, radius, false );

	// don't allow a fragment to make multiple marks, or they
	// pile up while settling
	this->leMarkType = LEMT_NONE;
}

/*
================
CG_ReflectVelocity
================
*/
void LocalEntity::reflectVelocity( trace_t *trace ) {
	Vec3	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = cg.time - cg.frametime + cg.frametime * trace->fraction;
	GameShared::evaluateTrajectoryDelta( &this->pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, this->pos.trDelta );

	VectorScale( this->pos.trDelta, this->bounceFactor, this->pos.trDelta );

	VectorCopy( trace->endpos, this->pos.trBase );
	this->pos.trTime = cg.time;


	// check for stop, making sure that even on low FPS systems it doesn't bobble
	if ( trace->allsolid || 
		( trace->plane.normal[2] > 0 && 
		( this->pos.trDelta[2] < 40 || this->pos.trDelta[2] < -cg.frametime * this->pos.trDelta[2] ) ) ) {
		this->pos.trType = TR_STATIONARY;
	} else {

	}
}

/*
================
CG_AddFragment
================
*/
void LocalEntity::addFragment() {
	Vec3	newOrigin;
	trace_t	trace;

	if ( this->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int		t;
		float	oldZ;
		
		t = this->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( this->refEntity.origin, this->refEntity.lightingOrigin );
			this->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = this->refEntity.origin[2];
			this->refEntity.origin[2] -= 16 * ( 1.0 - static_cast<float>(t) / SINK_TIME );
			refExport.AddRefEntityToScene( &this->refEntity );
			this->refEntity.origin[2] = oldZ;
		} else {
			refExport.AddRefEntityToScene( &this->refEntity );
		}

		return;
	}

	// calculate New position
	GameShared::evaluateTrajectory( &this->pos, cg.time, newOrigin );

	// trace a line from previous position to New position
	CGame::trace( &trace, this->refEntity.origin, nullptr, nullptr, newOrigin, -1, CONTENTS_SOLID );
	if ( trace.fraction == 1.0 ) {
		// still in free fall
		VectorCopy( newOrigin, this->refEntity.origin );

		if ( any( this->leFlags & LEF_TUMBLE ) ) {
			Vec3 angles;

			GameShared::evaluateTrajectory( &this->angles, cg.time, angles );
			AnglesToAxis( angles, this->refEntity.axis );
		}

		refExport.AddRefEntityToScene( &this->refEntity );

		return;
	}

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( CollisionModel::pointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		this->freeLocalEntity();
		return;
	}

	// leave a mark
	this->fragmentBounceMark( &trace );

	// reflect the velocity on the trace plane
	this->reflectVelocity( &trace );

	refExport.AddRefEntityToScene( &this->refEntity );
}

/*
=====================================================================

TRIVIAL LOCAL ENTITIES

These only do simple scaling or modulation before passing to the renderer
=====================================================================
*/

/*
====================
CG_AddFadeRGB
====================
*/
void LocalEntity::addFadeRGB() {
	refEntity_t *re;
	float c;

	re = &this->refEntity;

	c = ( this->endTime - cg.time ) * this->lifeRate;
	c *= 0xff;

	re->shaderRGBA[0] = this->color[0] * c;
	re->shaderRGBA[1] = this->color[1] * c;
	re->shaderRGBA[2] = this->color[2] * c;
	re->shaderRGBA[3] = this->color[3] * c;

	refExport.AddRefEntityToScene( re );
}

/*
==================
CG_AddMoveScaleFade
==================
*/
void LocalEntity::addMoveScaleFade() {
	refEntity_t	*re;
	float		c;
	Vec3		delta;
	float		len;

	re = &this->refEntity;

	if ( this->fadeInTime > this->startTime && cg.time < this->fadeInTime ) {
		// fade / grow time
		c = 1.0 - static_cast<float>( this->fadeInTime - cg.time ) / ( this->fadeInTime - this->startTime );
	}
	else {
		// fade / grow time
		c = ( this->endTime - cg.time ) * this->lifeRate;
	}

	re->shaderRGBA[3] = 0xff * c * this->color[3];

	if ( !( this->leFlags & LEF_PUFF_DONT_SCALE ) ) {
		re->radius = this->radius * ( 1.0 - c ) + 8;
	}

	GameShared::evaluateTrajectory( &this->pos, cg.time, re->origin );

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if( len < this->radius && !(this->leFlags & LEF_NO_RADIUS_KILL) )
	{
		this->freeLocalEntity();
		return;
	}

	refExport.AddRefEntityToScene( re );
}


/*
===================
CG_AddScaleFade

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
void LocalEntity::addScaleFade() {
	refEntity_t	*re;
	float		c;
	Vec3		delta;
	float		len;

	re = &this->refEntity;

	// fade / grow time
	c = ( this->endTime - cg.time ) * this->lifeRate;

	re->shaderRGBA[3] = 0xff * c * this->color[3];
	re->radius = this->radius * ( 1.0 - c ) + 8;

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if( len < this->radius && !(this->leFlags & LEF_NO_RADIUS_KILL) )
	{
		this->freeLocalEntity();
		return;
	}

	refExport.AddRefEntityToScene( re );
}

/*
================
CG_AddExplosion
================
*/
void LocalEntity::addExplosion() {
	refEntity_t	*ent;

	ent = &this->refEntity;

	// add the entity
	refExport.AddRefEntityToScene(ent);

	// add the dlight
	if ( this->light ) {
		float		light;

		light = static_cast<float>( cg.time - this->startTime ) / ( this->endTime - this->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = this->light * light;
		refExport.AddLightToScene(ent->origin, light, this->lightColor[0], this->lightColor[1], this->lightColor[2] );
	}
}

/*
================
CG_AddSpriteExplosion
================
*/
void LocalEntity::addSpriteExplosion() {
	refEntity_t	re;
	float c;

	re = this->refEntity;

	// MFQ3: don't add explosion sprites to the scene until their time
	// has come
	if( this->startTime > cg.time && this->leType == LE_SPRITE_EXPLOSION )
	{
		return;
	}

	c = ( this->endTime - cg.time ) / static_cast< float>( this->endTime - this->startTime );
	if ( c > 1 ) {
		c = 1.0;	// can happen during connection problems
	}

	re.shaderRGBA[0] = 0xff;
	re.shaderRGBA[1] = 0xff;
	re.shaderRGBA[2] = 0xff;
	re.shaderRGBA[3] = 0xff * c * 0.33;

	re.reType = RT_SPRITE;
	re.radius = 42 * ( 1.0 - c ) + 30;

	// use the radius parameter of the LocalEntity as a scale modifier (0.0f or 1.0f will give no change)
	if( this->radius )
	{
		re.radius *= this->radius;
	}

	refExport.AddRefEntityToScene( &re );

	// add the dlight
	if ( this->light ) {
		float		light;

		light = static_cast<float>( cg.time - this->startTime ) / ( this->endTime - this->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = this->light * light;
		refExport.AddLightToScene(re.origin, light, this->lightColor[0], this->lightColor[1], this->lightColor[2] );
	}
}

/*
====================
CG_AddNuke
====================
*/
void LocalEntity::addNuke() {
	refEntity_t	*re;
	refEntity_t shockwave;
	refEntity_t cloud;
	float		c;
	vec3_t		test, axis[3];
	int			t;
	int			rad;



	re = &this->refEntity;

	rad = re->radius;
	// Set limits
	if(rad < NUKE_BOOMSPHEREMODEL_RADIUS)
	{
		rad = NUKE_BOOMSPHEREMODEL_RADIUS;
	}

	t = cg.time - this->startTime;
	VectorClear( test );
	AnglesToAxis( test, axis );

	if (t > NUKE_SHOCKWAVE_STARTTIME && t < NUKE_SHOCKWAVE_ENDTIME) {

		if (!(this->leFlags & LEF_SOUND1)) {
//			SoundSystem::startSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.nukeExplodeSound );
			SoundSystem::startLocalSound(cgs.media.nukeExplodeSound, CHAN_AUTO);
			this->leFlags |= LEF_SOUND1;
		}
		// 1st nuke shockwave
		shockwave = {};
		shockwave.hModel = cgs.media.nukeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = static_cast<float>(t - NUKE_SHOCKWAVE_STARTTIME) / static_cast<float>(NUKE_SHOCKWAVE_ENDTIME - NUKE_SHOCKWAVE_STARTTIME);
		VectorScale( axis[0], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = true;

		if (t > NUKE_SHOCKWAVEFADE_STARTTIME) {
			c = static_cast<float>(t - NUKE_SHOCKWAVEFADE_STARTTIME) / static_cast<float>(NUKE_SHOCKWAVE_ENDTIME - NUKE_SHOCKWAVEFADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		refExport.AddRefEntityToScene( &shockwave );
	}

	if (t > NUKE_EXPLODE_STARTTIME && t < NUKE_IMPLODE_ENDTIME) {
		// explosion and implosion
		c = ( this->endTime - cg.time ) * this->lifeRate;
		c *= 0xff;
		re->shaderRGBA[0] = this->color[0] * c;
		re->shaderRGBA[1] = this->color[1] * c;
		re->shaderRGBA[2] = this->color[2] * c;
		re->shaderRGBA[3] = this->color[3] * c;

		if( t < NUKE_IMPLODE_STARTTIME ) {
			c = static_cast<float>(t - NUKE_EXPLODE_STARTTIME) / static_cast<float>(NUKE_IMPLODE_STARTTIME - NUKE_EXPLODE_STARTTIME);
		}
		else {
			if (!(this->leFlags & LEF_SOUND2)) {
//				SoundSystem::startSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.nukeImplodeSound );
				SoundSystem::startLocalSound(cgs.media.nukeImplodeSound, CHAN_AUTO);
				this->leFlags |= LEF_SOUND2;
			}
			c = static_cast<float>(NUKE_IMPLODE_ENDTIME - t) / static_cast<float>(NUKE_IMPLODE_ENDTIME - NUKE_IMPLODE_STARTTIME);
		}
		VectorScale( axis[0], c * rad / NUKE_BOOMSPHEREMODEL_RADIUS, re->axis[0] );
		VectorScale( axis[1], c * rad / NUKE_BOOMSPHEREMODEL_RADIUS, re->axis[1] );
		VectorScale( axis[2], c * rad / NUKE_BOOMSPHEREMODEL_RADIUS, re->axis[2] );
		re->nonNormalizedAxes = true;

		refExport.AddRefEntityToScene( re );
		// add the dlight
		refExport.AddLightToScene( re->origin, c * 1000.0, 1.0, 1.0, c );
	}

	if (t > NUKE_SHOCKWAVE2_STARTTIME && t < NUKE_SHOCKWAVE2_ENDTIME) {
		// 2nd nuke shockwave
		if (this->angles.trBase[0] == 0 &&
			this->angles.trBase[1] == 0 &&
			this->angles.trBase[2] == 0) {
			this->angles.trBase[0] = random() * 360;
			this->angles.trBase[1] = random() * 360;
			this->angles.trBase[2] = random() * 360;
		}
		else {
			c = 0;
		}
		shockwave = {};
		shockwave.hModel = cgs.media.nukeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		test[0] = this->angles.trBase[0];
		test[1] = this->angles.trBase[1];
		test[2] = this->angles.trBase[2];
		AnglesToAxis( test, axis );

		c = static_cast<float>(t - NUKE_SHOCKWAVE2_STARTTIME) / static_cast<float>(NUKE_SHOCKWAVE2_ENDTIME - NUKE_SHOCKWAVE2_STARTTIME);
		VectorScale( axis[0], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = true;

		if (t > NUKE_SHOCKWAVE2FADE_STARTTIME) {
			c = static_cast<float>(t - NUKE_SHOCKWAVE2FADE_STARTTIME) / static_cast<float>(NUKE_SHOCKWAVE2_ENDTIME - NUKE_SHOCKWAVE2FADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		refExport.AddRefEntityToScene( &shockwave );
	}

	if (t > NUKE_CLOUD_STARTTIME && t < NUKE_CLOUD_ENDTIME) {
		int i;
		AxisClear(axis);
		cloud = {};
		cloud.hModel = cgs.media.nukeCloudModel;
		cloud.reType = RT_MODEL;
		cloud.shaderTime = re->shaderTime;
		VectorCopy(re->origin, cloud.origin);
		cloud.renderfx = RF_LIGHTING_ORIGIN|RF_SHADOW_PLANE;
		
		c = static_cast<float>(t - NUKE_CLOUD_STARTTIME) / static_cast<float>(NUKE_CLOUD_FADETIME - NUKE_CLOUD_STARTTIME);
		cloud.customShader = cgs.media.nukeCloud[std::min(static_cast<int>(c*100),100)];
		for(i = 0; i<3;i++)
			VectorScale(axis[i],(c + 1) * rad * 0.01f,cloud.axis[i]);
		shockwave.nonNormalizedAxes = true;

		if (t > NUKE_CLOUD_FADETIME) {
			c = static_cast<float>(t - NUKE_CLOUD_FADETIME) / static_cast<float>(NUKE_CLOUD_ENDTIME - NUKE_CLOUD_FADETIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		cloud.shaderRGBA[0] = 0xff - c;
		cloud.shaderRGBA[1] = 0xff - c;
		cloud.shaderRGBA[2] = 0xff - c;
		cloud.shaderRGBA[3] = 0xff - c;

		refExport.AddRefEntityToScene( &cloud );
	}
}

//==============================================================================

/*
===================
CGame::addLocalEntities

===================
*/
void CGame::addLocalEntities( ) {
	LocalEntity	*le, *next;

	// walk the list backwards, so any New local entities generated
	// (trails, marks, etc) will be present this frame
	le = cg_activeLocalEntities.prev;
	for ( ; le != &cg_activeLocalEntities ; le = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = le->prev;

		// kill entity because life is up?
		if ( cg.time >= le->endTime )
		{
			le->freeLocalEntity();
			continue;
		}
		switch ( le->leType ) {
		default:
			Common::error( ERR_DROP, "Bad leType: %i", le->leType );
			break;

		case LE_MARK:
			break;

		case LE_SPRITE_EXPLOSION:
			le->addSpriteExplosion();
			break;

		case LE_EXPLOSION:
			le->addExplosion();
			break;

		case LE_FRAGMENT:			// gibs and brass
			le->addFragment();
			break;

		case LE_MOVE_SCALE_FADE:		// water bubbles
			le->addMoveScaleFade();
			break;

		case LE_FADE_RGB:				// teleporters, railtrails
			le->addFadeRGB();
			break;

		case LE_SCALE_FADE:		// rocket trails
			le->addScaleFade();
			break;
		case LE_NUKE:
			le->addNuke();
			break;

		}
	}
}






