/*
 * $Id: cg_effects.c,v 1.9 2005-06-26 05:08:11 minkis Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"

/*
=====================
CG_SmokePuff

Adds a smoke puff localEntity.
=====================
*/
localEntity_t *CG_SmokePuff( const vec3_t p, const vec3_t vel, 
				   float radius,
				   float r, float g, float b, float a,
				   float duration,
				   int startTime,
				   int fadeInTime,
				   int leFlags,
				   qhandle_t hShader ) {
	static int	seed = 0x92;
	localEntity_t	*le;
	refEntity_t		*re;
//	int fadeInTime = startTime + duration / 2;

	le = CG_AllocLocalEntity();
	le->leFlags = leFlags;
	le->radius = radius;
	
	re = &le->refEntity;
	re->rotation = Q_random( &seed ) * 360;
	re->radius = radius;
	re->shaderTime = startTime / 1000.0f;

	le->leType = LE_MOVE_SCALE_FADE;
	le->startTime = startTime;
	le->fadeInTime = fadeInTime;
	le->endTime = startTime + duration;
	if ( fadeInTime > startTime ) {
		le->lifeRate = 1.0 / ( le->endTime - le->fadeInTime );
	}
	else {
		le->lifeRate = 1.0 / ( le->endTime - le->startTime );
	}
	le->color[0] = r;
	le->color[1] = g; 
	le->color[2] = b;
	le->color[3] = a;


	le->pos.trType = TR_LINEAR;
	le->pos.trTime = startTime;
	VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );
	re->customShader = hShader;

	// rage pro can't alpha fade, so use a different shader
	if ( cgs.glconfig.hardwareType == GLHW_RAGEPRO ) {
		re->customShader = cgs.media.smokePuffRageProShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;
	} else {
		re->shaderRGBA[0] = le->color[0] * 0xff;
		re->shaderRGBA[1] = le->color[1] * 0xff;
		re->shaderRGBA[2] = le->color[2] * 0xff;
		re->shaderRGBA[3] = 0xff;
	}

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	return le;
}


/*
==========================
CG_NukeCloud
==========================
*/
static void CG_NukeCloud( centity_t * cent, entityState_t * es )
{
	localEntity_t	* smoke;
	vec3_t			pos, velocity;
	int i, temp, puff_size;
	static int	seed = 0x92;

	puff_size = availableWeapons[es->weaponIndex].damageRadius / 10;

	VectorCopy( cent->lerpOrigin, pos );
	
	// draw smoke slightly behind the entitiy position (using -velocity of the entity)
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	VectorNormalize( velocity );

	// calc adjusted position
	VectorScale( velocity, -24.0f, velocity );		// -24.0f is just an arbitary distance which works OK with all current rocket models
	VectorAdd( pos, velocity, pos );

	// draw base
	temp = availableWeapons[es->weaponIndex].damageRadius / 2.5;
	for(i = 0; i <= 30; i++)
	{
			VectorCopy(cent->lerpOrigin, pos);
			pos[0] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			pos[1] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			pos[2] += Q_random( &seed ) * 50;
			velocity[0] = (5 - (-5) + 1) * Q_random(&seed) + -5;
			velocity[1] = (5 - (-5) + 1) * Q_random(&seed) + -5;
			smoke = CG_SmokePuff( pos, velocity, 
						  puff_size, 
						  0.1f, 0.1f, 0.1f, 0.7f,
						  10000, 
						  cg.time, 8000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );	
	}
	// Draw slowly rising funnle
	// draw base
	temp = availableWeapons[es->weaponIndex].damageRadius / 10;
	for(i = 0; i <= 60; i++)
	{
			VectorCopy(cent->lerpOrigin, pos);
			pos[0] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			pos[1] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			pos[2] += Q_random( &seed ) * (temp * 5) ;
			velocity[0] = (1 - (-1) + 1) * Q_random(&seed) + -1;
			velocity[1] = (1 - (-1) + 1) * Q_random(&seed) + -1;
			velocity[2] = ((temp * 10) / 6.5) * Q_random(&seed);

			smoke = CG_SmokePuff( pos, velocity, 
						  puff_size, 
						  0.1f, 0.1f, 0.1f, 0.7f,
						  10000, 
						  cg.time, 8000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );	
	}
	// Draw mushroom top
	temp = availableWeapons[es->weaponIndex].damageRadius / 2;
	for(i = 0; i <= 60; i++)
	{
			VectorCopy(cent->lerpOrigin, pos);
			pos[2] +=  temp;

			pos[0] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			pos[1] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			pos[2] += (temp / 5) * Q_random( &seed ) ;
			velocity[0] = (3 - (-3) + 1) * Q_random(&seed) + -3;
			velocity[1] = (3 - (-3) + 1) * Q_random(&seed) + -3;
			velocity[2] = ((temp * 2) / 6.5) * Q_random(&seed);

			smoke = CG_SmokePuff( pos, velocity, 
						  puff_size, 
						  0.1f, 0.1f, 0.1f, 0.7f,
						  10000, 
						  cg.time, 8000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );	
	}

}


/*
==================
CG_NukeEffect
==================
*/
void CG_NukeEffect(  centity_t * cent, entityState_t * es ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	le->leFlags = 0;
	le->leType = LE_NUKE;
	le->startTime = cg.time;
	le->endTime = cg.time + 3000;//2250;
	le->lifeRate = 1.0 / ( le->endTime - le->startTime );

	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;

	VectorClear(le->angles.trBase);

	re = &le->refEntity;

	re->radius = availableWeapons[es->weaponIndex].damageRadius;
	re->reType = RT_MODEL;
	re->shaderTime = cg.time / 1000.0f;

	re->hModel = cgs.media.nukeEffectModel;

	VectorCopy( cent->lerpOrigin, re->origin );

	CG_NukeCloud(cent, es);

}


/*
==========================
CG_FlakEffect
==========================
*/
void CG_FlakEffect( centity_t * cent, entityState_t * es )
{
	localEntity_t	* smoke;
	vec3_t			pos, velocity;
	int i, temp, puff_size;
	static int	seed = 0x92;
	puff_size = availableWeapons[es->weaponIndex].damageRadius / 10;

	VectorCopy( cent->lerpOrigin, pos );

	// draw base
	temp = availableWeapons[es->weaponIndex].damageRadius * 0.5;
	for(i = 0; i <= 30; i++)
	{
			VectorCopy(cent->lerpOrigin, pos);
			pos[0] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			pos[1] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			pos[2] += (temp - (temp * -1) + 1) * Q_random( &seed ) + (temp * -1);
			velocity[0] = (5 - (-5) + 1) * Q_random(&seed) + -5;
			velocity[1] = (5 - (-5) + 1) * Q_random(&seed) + -5;
			smoke = CG_SmokePuff( pos, velocity, 
						  puff_size, 
						  0.0f, 0.0f, 0.0f, 0.7f,
						  10000, 
						  cg.time, 8000,
						  LEF_PUFF_DONT_SCALE, 
						  cgs.media.smokePuffShader );	
	}
}



/*
====================
CG_MakeExplosion
====================
*/
localEntity_t *CG_MakeExplosion( vec3_t origin, vec3_t dir, 
								 qhandle_t hModel, qhandle_t shader,
								 int offset, int duration, qboolean isSprite )
{
	float			ang;
	localEntity_t	*ex;
	vec3_t			tmpVec, newOrigin;

	if ( duration <= 0 ) {
		CG_Error( "CG_MakeExplosion: duration = %i", duration );
	}

	// skew the time a bit so they aren't all in sync
	offset += rand() & 63;

	ex = CG_AllocLocalEntity();
	if ( isSprite )
	{
		ex->leType = LE_SPRITE_EXPLOSION;

		// randomly rotate sprite orientation
		ex->refEntity.rotation = rand() % 360;
		VectorScale( dir, 16, tmpVec );
		VectorAdd( tmpVec, origin, newOrigin );
	}
	else
	{
		ex->leType = LE_EXPLOSION;
		VectorCopy( origin, newOrigin );

		// set axis with random rotate
		if ( !dir )
		{
			AxisClear( ex->refEntity.axis );
		} else
		{
			ang = rand() % 360;
			VectorCopy( dir, ex->refEntity.axis[0] );
			RotateAroundDirection( ex->refEntity.axis, ang );
		}
	}

	// calc the timings
 	ex->startTime = cg.time + offset;
	ex->endTime = ex->startTime + duration;

	// bias the time so all shader effects start correctly
	ex->refEntity.shaderTime = ex->startTime / 1000.0f;

	ex->refEntity.hModel = hModel;
	ex->refEntity.customShader = shader;

	// set origin
	VectorCopy( newOrigin, ex->refEntity.origin );
	VectorCopy( newOrigin, ex->refEntity.oldorigin );

	ex->color[0] = ex->color[1] = ex->color[2] = 1;

	return ex;
}


/*
==================
CG_LaunchExplode
==================
*/
void CG_LaunchExplode( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity();
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;
	le->startTime = cg.time;
	le->endTime = le->startTime + 10000 + random() * 6000;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.1f;

	le->leBounceSoundType = LEBS_BRASS;
	le->leMarkType = LEMT_NONE;
}

