/*
 * $Id: cg_localents.c,v 1.2 2005-08-31 19:20:06 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

// cg_localents.c -- every frame, generate renderer commands for locally
// processed entities, like smoke puffs, gibs, shells, etc.

#include "cg_local.h"
#include <algorithm>

#define	MAX_LOCAL_ENTITIES	512
localEntity_t	cg_localEntities[MAX_LOCAL_ENTITIES];
localEntity_t	cg_activeLocalEntities;		// double linked list
localEntity_t	*cg_freeLocalEntities;		// single linked list

/*
===================
CG_InitLocalEntities

This is called at startup and for tournement restarts
===================
*/
void	CG_InitLocalEntities( void ) {
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
void CG_FreeLocalEntity( localEntity_t *le ) {
	if ( !le->prev ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prev->next = le->next;
	le->next->prev = le->prev;

	// the free list is only singly linked
	le->next = cg_freeLocalEntities;
	cg_freeLocalEntities = le;
}

/*
===================
CG_AllocLocalEntity

Will allways succeed, even if it requires freeing an old active entity
===================
*/
localEntity_t	*CG_AllocLocalEntity( void ) {
	localEntity_t	*le;

	if ( !cg_freeLocalEntities ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		CG_FreeLocalEntity( cg_activeLocalEntities.prev );
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
void CG_FragmentBounceMark( localEntity_t *le, trace_t *trace ) {
	int			radius;

	radius = 8 + (rand()&15);
	CG_ImpactMark( cgs.media.burnMarkShader, trace->endpos, trace->plane.normal, random()*360,
			1,1,1,1, true, radius, false );

	// don't allow a fragment to make multiple marks, or they
	// pile up while settling
	le->leMarkType = LEMT_NONE;
}

/*
================
CG_ReflectVelocity
================
*/
void CG_ReflectVelocity( localEntity_t *le, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = cg.time - cg.frametime + cg.frametime * trace->fraction;
	BG_EvaluateTrajectoryDelta( &le->pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, le->pos.trDelta );

	VectorScale( le->pos.trDelta, le->bounceFactor, le->pos.trDelta );

	VectorCopy( trace->endpos, le->pos.trBase );
	le->pos.trTime = cg.time;


	// check for stop, making sure that even on low FPS systems it doesn't bobble
	if ( trace->allsolid || 
		( trace->plane.normal[2] > 0 && 
		( le->pos.trDelta[2] < 40 || le->pos.trDelta[2] < -cg.frametime * le->pos.trDelta[2] ) ) ) {
		le->pos.trType = TR_STATIONARY;
	} else {

	}
}

/*
================
CG_AddFragment
================
*/
void CG_AddFragment( localEntity_t *le ) {
	vec3_t	newOrigin;
	trace_t	trace;

	if ( le->pos.trType == TR_STATIONARY ) {
		// sink into the ground if near the removal time
		int		t;
		float	oldZ;
		
		t = le->endTime - cg.time;
		if ( t < SINK_TIME ) {
			// we must use an explicit lighting origin, otherwise the
			// lighting would be lost as soon as the origin went
			// into the ground
			VectorCopy( le->refEntity.origin, le->refEntity.lightingOrigin );
			le->refEntity.renderfx |= RF_LIGHTING_ORIGIN;
			oldZ = le->refEntity.origin[2];
			le->refEntity.origin[2] -= 16 * ( 1.0 - (float)t / SINK_TIME );
			trap_R_AddRefEntityToScene( &le->refEntity );
			le->refEntity.origin[2] = oldZ;
		} else {
			trap_R_AddRefEntityToScene( &le->refEntity );
		}

		return;
	}

	// calculate new position
	BG_EvaluateTrajectory( &le->pos, cg.time, newOrigin );

	// trace a line from previous position to new position
	CG_Trace( &trace, le->refEntity.origin, NULL, NULL, newOrigin, -1, CONTENTS_SOLID );
	if ( trace.fraction == 1.0 ) {
		// still in free fall
		VectorCopy( newOrigin, le->refEntity.origin );

		if ( le->leFlags & LEF_TUMBLE ) {
			vec3_t angles;

			BG_EvaluateTrajectory( &le->angles, cg.time, angles );
			AnglesToAxis( angles, le->refEntity.axis );
		}

		trap_R_AddRefEntityToScene( &le->refEntity );

		return;
	}

	// if it is in a nodrop zone, remove it
	// this keeps gibs from waiting at the bottom of pits of death
	// and floating levels
	if ( trap_CM_PointContents( trace.endpos, 0 ) & CONTENTS_NODROP ) {
		CG_FreeLocalEntity( le );
		return;
	}

	// leave a mark
	CG_FragmentBounceMark( le, &trace );

	// reflect the velocity on the trace plane
	CG_ReflectVelocity( le, &trace );

	trap_R_AddRefEntityToScene( &le->refEntity );
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
void CG_AddFadeRGB( localEntity_t *le ) {
	refEntity_t *re;
	float c;

	re = &le->refEntity;

	c = ( le->endTime - cg.time ) * le->lifeRate;
	c *= 0xff;

	re->shaderRGBA[0] = le->color[0] * c;
	re->shaderRGBA[1] = le->color[1] * c;
	re->shaderRGBA[2] = le->color[2] * c;
	re->shaderRGBA[3] = le->color[3] * c;

	trap_R_AddRefEntityToScene( re );
}

/*
==================
CG_AddMoveScaleFade
==================
*/
static void CG_AddMoveScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	if ( le->fadeInTime > le->startTime && cg.time < le->fadeInTime ) {
		// fade / grow time
		c = 1.0 - (float) ( le->fadeInTime - cg.time ) / ( le->fadeInTime - le->startTime );
	}
	else {
		// fade / grow time
		c = ( le->endTime - cg.time ) * le->lifeRate;
	}

	re->shaderRGBA[3] = 0xff * c * le->color[3];

	if ( !( le->leFlags & LEF_PUFF_DONT_SCALE ) ) {
		re->radius = le->radius * ( 1.0 - c ) + 8;
	}

	BG_EvaluateTrajectory( &le->pos, cg.time, re->origin );

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if( len < le->radius && !(le->leFlags & LEF_NO_RADIUS_KILL) )
	{
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}


/*
===================
CG_AddScaleFade

For rocket smokes that hang in place, fade out, and are
removed if the view passes through them.
There are often many of these, so it needs to be simple.
===================
*/
static void CG_AddScaleFade( localEntity_t *le ) {
	refEntity_t	*re;
	float		c;
	vec3_t		delta;
	float		len;

	re = &le->refEntity;

	// fade / grow time
	c = ( le->endTime - cg.time ) * le->lifeRate;

	re->shaderRGBA[3] = 0xff * c * le->color[3];
	re->radius = le->radius * ( 1.0 - c ) + 8;

	// if the view would be "inside" the sprite, kill the sprite
	// so it doesn't add too much overdraw
	VectorSubtract( re->origin, cg.refdef.vieworg, delta );
	len = VectorLength( delta );
	if( len < le->radius && !(le->leFlags & LEF_NO_RADIUS_KILL) )
	{
		CG_FreeLocalEntity( le );
		return;
	}

	trap_R_AddRefEntityToScene( re );
}

/*
================
CG_AddExplosion
================
*/
static void CG_AddExplosion( localEntity_t *ex ) {
	refEntity_t	*ent;

	ent = &ex->refEntity;

	// add the entity
	trap_R_AddRefEntityToScene(ent);

	// add the dlight
	if ( ex->light ) {
		float		light;

		light = (float)( cg.time - ex->startTime ) / ( ex->endTime - ex->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = ex->light * light;
		trap_R_AddLightToScene(ent->origin, light, ex->lightColor[0], ex->lightColor[1], ex->lightColor[2] );
	}
}

/*
================
CG_AddSpriteExplosion
================
*/
static void CG_AddSpriteExplosion( localEntity_t *le ) {
	refEntity_t	re;
	float c;

	re = le->refEntity;

	// MFQ3: don't add explosion sprites to the scene until their time
	// has come
	if( le->startTime > cg.time && le->leType == LE_SPRITE_EXPLOSION )
	{
		return;
	}

	c = ( le->endTime - cg.time ) / ( float ) ( le->endTime - le->startTime );
	if ( c > 1 ) {
		c = 1.0;	// can happen during connection problems
	}

	re.shaderRGBA[0] = 0xff;
	re.shaderRGBA[1] = 0xff;
	re.shaderRGBA[2] = 0xff;
	re.shaderRGBA[3] = 0xff * c * 0.33;

	re.reType = RT_SPRITE;
	re.radius = 42 * ( 1.0 - c ) + 30;

	// use the radius parameter of the localEntity_t as a scale modifier (0.0f or 1.0f will give no change)
	if( le->radius )
	{
		re.radius *= le->radius;
	}

	trap_R_AddRefEntityToScene( &re );

	// add the dlight
	if ( le->light ) {
		float		light;

		light = (float)( cg.time - le->startTime ) / ( le->endTime - le->startTime );
		if ( light < 0.5 ) {
			light = 1.0;
		} else {
			light = 1.0 - ( light - 0.5 ) * 2;
		}
		light = le->light * light;
		trap_R_AddLightToScene(re.origin, light, le->lightColor[0], le->lightColor[1], le->lightColor[2] );
	}
}

/*
====================
CG_AddNuke
====================
*/
void CG_AddNuke( localEntity_t *le ) {
	refEntity_t	*re;
	refEntity_t shockwave;
	refEntity_t cloud;
	float		c;
	vec3_t		test, axis[3];
	int			t;
	int			rad;



	re = &le->refEntity;

	rad = re->radius;
	// Set limits
	if(rad < NUKE_BOOMSPHEREMODEL_RADIUS)
	{
		rad = NUKE_BOOMSPHEREMODEL_RADIUS;
	}

	t = cg.time - le->startTime;
	VectorClear( test );
	AnglesToAxis( test, axis );

	if (t > NUKE_SHOCKWAVE_STARTTIME && t < NUKE_SHOCKWAVE_ENDTIME) {

		if (!(le->leFlags & LEF_SOUND1)) {
//			trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.nukeExplodeSound );
			trap_S_StartLocalSound(cgs.media.nukeExplodeSound, CHAN_AUTO);
			le->leFlags |= LEF_SOUND1;
		}
		// 1st nuke shockwave
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.nukeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		c = (float)(t - NUKE_SHOCKWAVE_STARTTIME) / (float)(NUKE_SHOCKWAVE_ENDTIME - NUKE_SHOCKWAVE_STARTTIME);
		VectorScale( axis[0], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = true;

		if (t > NUKE_SHOCKWAVEFADE_STARTTIME) {
			c = (float)(t - NUKE_SHOCKWAVEFADE_STARTTIME) / (float)(NUKE_SHOCKWAVE_ENDTIME - NUKE_SHOCKWAVEFADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &shockwave );
	}

	if (t > NUKE_EXPLODE_STARTTIME && t < NUKE_IMPLODE_ENDTIME) {
		// explosion and implosion
		c = ( le->endTime - cg.time ) * le->lifeRate;
		c *= 0xff;
		re->shaderRGBA[0] = le->color[0] * c;
		re->shaderRGBA[1] = le->color[1] * c;
		re->shaderRGBA[2] = le->color[2] * c;
		re->shaderRGBA[3] = le->color[3] * c;

		if( t < NUKE_IMPLODE_STARTTIME ) {
			c = (float)(t - NUKE_EXPLODE_STARTTIME) / (float)(NUKE_IMPLODE_STARTTIME - NUKE_EXPLODE_STARTTIME);
		}
		else {
			if (!(le->leFlags & LEF_SOUND2)) {
//				trap_S_StartSound (re->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.nukeImplodeSound );
				trap_S_StartLocalSound(cgs.media.nukeImplodeSound, CHAN_AUTO);
				le->leFlags |= LEF_SOUND2;
			}
			c = (float)(NUKE_IMPLODE_ENDTIME - t) / (float) (NUKE_IMPLODE_ENDTIME - NUKE_IMPLODE_STARTTIME);
		}
		VectorScale( axis[0], c * rad / NUKE_BOOMSPHEREMODEL_RADIUS, re->axis[0] );
		VectorScale( axis[1], c * rad / NUKE_BOOMSPHEREMODEL_RADIUS, re->axis[1] );
		VectorScale( axis[2], c * rad / NUKE_BOOMSPHEREMODEL_RADIUS, re->axis[2] );
		re->nonNormalizedAxes = true;

		trap_R_AddRefEntityToScene( re );
		// add the dlight
		trap_R_AddLightToScene( re->origin, c * 1000.0, 1.0, 1.0, c );
	}

	if (t > NUKE_SHOCKWAVE2_STARTTIME && t < NUKE_SHOCKWAVE2_ENDTIME) {
		// 2nd nuke shockwave
		if (le->angles.trBase[0] == 0 &&
			le->angles.trBase[1] == 0 &&
			le->angles.trBase[2] == 0) {
			le->angles.trBase[0] = random() * 360;
			le->angles.trBase[1] = random() * 360;
			le->angles.trBase[2] = random() * 360;
		}
		else {
			c = 0;
		}
		memset(&shockwave, 0, sizeof(shockwave));
		shockwave.hModel = cgs.media.nukeShockWave;
		shockwave.reType = RT_MODEL;
		shockwave.shaderTime = re->shaderTime;
		VectorCopy(re->origin, shockwave.origin);

		test[0] = le->angles.trBase[0];
		test[1] = le->angles.trBase[1];
		test[2] = le->angles.trBase[2];
		AnglesToAxis( test, axis );

		c = (float)(t - NUKE_SHOCKWAVE2_STARTTIME) / (float)(NUKE_SHOCKWAVE2_ENDTIME - NUKE_SHOCKWAVE2_STARTTIME);
		VectorScale( axis[0], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[0] );
		VectorScale( axis[1], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[1] );
		VectorScale( axis[2], c * rad + 100 / NUKE_SHOCKWAVEMODEL_RADIUS, shockwave.axis[2] );
		shockwave.nonNormalizedAxes = true;

		if (t > NUKE_SHOCKWAVE2FADE_STARTTIME) {
			c = (float)(t - NUKE_SHOCKWAVE2FADE_STARTTIME) / (float)(NUKE_SHOCKWAVE2_ENDTIME - NUKE_SHOCKWAVE2FADE_STARTTIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		shockwave.shaderRGBA[0] = 0xff - c;
		shockwave.shaderRGBA[1] = 0xff - c;
		shockwave.shaderRGBA[2] = 0xff - c;
		shockwave.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &shockwave );
	}

	if (t > NUKE_CLOUD_STARTTIME && t < NUKE_CLOUD_ENDTIME) {
		int i;
		AxisClear(axis);
		memset(&cloud, 0, sizeof(cloud));
		cloud.hModel = cgs.media.nukeCloudModel;
		cloud.reType = RT_MODEL;
		cloud.shaderTime = re->shaderTime;
		VectorCopy(re->origin, cloud.origin);
		cloud.renderfx = RF_LIGHTING_ORIGIN|RF_SHADOW_PLANE;
		
		c = (float)(t - NUKE_CLOUD_STARTTIME) / (float)(NUKE_CLOUD_FADETIME - NUKE_CLOUD_STARTTIME);
		cloud.customShader = cgs.media.nukeCloud[std::min((int)(c*100),100)];
		for(i = 0; i<3;i++)
			VectorScale(axis[i],(c + 1) * rad * 0.01f,cloud.axis[i]);
		shockwave.nonNormalizedAxes = true;

		if (t > NUKE_CLOUD_FADETIME) {
			c = (float)(t - NUKE_CLOUD_FADETIME) / (float)(NUKE_CLOUD_ENDTIME - NUKE_CLOUD_FADETIME);
		}
		else {
			c = 0;
		}
		c *= 0xff;
		cloud.shaderRGBA[0] = 0xff - c;
		cloud.shaderRGBA[1] = 0xff - c;
		cloud.shaderRGBA[2] = 0xff - c;
		cloud.shaderRGBA[3] = 0xff - c;

		trap_R_AddRefEntityToScene( &cloud );
	}
}

//==============================================================================

/*
===================
CG_AddLocalEntities

===================
*/
void CG_AddLocalEntities( void ) {
	localEntity_t	*le, *next;

	// walk the list backwards, so any new local entities generated
	// (trails, marks, etc) will be present this frame
	le = cg_activeLocalEntities.prev;
	for ( ; le != &cg_activeLocalEntities ; le = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = le->prev;

		// kill entity because life is up?
		if ( cg.time >= le->endTime )
		{
			CG_FreeLocalEntity( le );
			continue;
		}
		switch ( le->leType ) {
		default:
			CG_Error( "Bad leType: %i", le->leType );
			break;

		case LE_MARK:
			break;

		case LE_SPRITE_EXPLOSION:
			CG_AddSpriteExplosion( le );
			break;

		case LE_EXPLOSION:
			CG_AddExplosion( le );
			break;

		case LE_FRAGMENT:			// gibs and brass
			CG_AddFragment( le );
			break;

		case LE_MOVE_SCALE_FADE:		// water bubbles
			CG_AddMoveScaleFade( le );
			break;

		case LE_FADE_RGB:				// teleporters, railtrails
			CG_AddFadeRGB( le );
			break;

		case LE_SCALE_FADE:		// rocket trails
			CG_AddScaleFade( le );
			break;
		case LE_NUKE:
			CG_AddNuke( le );
			break;

		}
	}
}






