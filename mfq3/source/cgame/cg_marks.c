/*
 * $Id: cg_marks.c,v 1.2 2002-01-31 10:09:40 sparky909_uk Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_marks.c -- wall marks

#include "cg_local.h"

/*
===================================================================

MARK POLYS

===================================================================
*/


markPoly_t	cg_activeMarkPolys;			// double linked list
markPoly_t	*cg_freeMarkPolys;			// single linked list
markPoly_t	cg_markPolys[MAX_MARK_POLYS];
static		int	markTotal;

/*
===================
CG_InitMarkPolys

This is called at startup and for tournement restarts
===================
*/
void	CG_InitMarkPolys( void ) {
	int		i;

	memset( cg_markPolys, 0, sizeof(cg_markPolys) );

	cg_activeMarkPolys.nextMark = &cg_activeMarkPolys;
	cg_activeMarkPolys.prevMark = &cg_activeMarkPolys;
	cg_freeMarkPolys = cg_markPolys;
	for ( i = 0 ; i < MAX_MARK_POLYS - 1 ; i++ ) {
		cg_markPolys[i].nextMark = &cg_markPolys[i+1];
	}
}


/*
==================
CG_FreeMarkPoly
==================
*/
void CG_FreeMarkPoly( markPoly_t *le ) {
	if ( !le->prevMark ) {
		CG_Error( "CG_FreeLocalEntity: not active" );
	}

	// remove from the doubly linked active list
	le->prevMark->nextMark = le->nextMark;
	le->nextMark->prevMark = le->prevMark;

	// the free list is only singly linked
	le->nextMark = cg_freeMarkPolys;
	cg_freeMarkPolys = le;
}

/*
===================
CG_AllocMark

Will allways succeed, even if it requires freeing an old active mark
===================
*/
markPoly_t	*CG_AllocMark( void ) {
	markPoly_t	*le;
	int time;

	if ( !cg_freeMarkPolys ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		time = cg_activeMarkPolys.prevMark->time;
		while (cg_activeMarkPolys.prevMark && time == cg_activeMarkPolys.prevMark->time) {
			CG_FreeMarkPoly( cg_activeMarkPolys.prevMark );
		}
	}

	le = cg_freeMarkPolys;
	cg_freeMarkPolys = cg_freeMarkPolys->nextMark;

	memset( le, 0, sizeof( *le ) );

	// link into the active list
	le->nextMark = cg_activeMarkPolys.nextMark;
	le->prevMark = &cg_activeMarkPolys;
	cg_activeMarkPolys.nextMark->prevMark = le;
	cg_activeMarkPolys.nextMark = le;
	return le;
}


/*
=================
CG_ImpactMark
=================
*/

void CG_ImpactMark( qhandle_t markShader, const vec3_t origin, const vec3_t dir, 
					  float orientation, float red, float green, float blue, float alpha,
					  qboolean alphaFade, float radius, qboolean temporary )
{
	// link to extended version
	CG_ImpactMarkEx( markShader, origin, dir, orientation, red, green, blue, alpha,
					 alphaFade, radius, radius, temporary );
}

/*
=================
CG_ImpactMarkEx

origin should be a point within a unit of the plane
dir should be the plane normal

temporary marks will not be stored or randomly oriented, but immediately
passed to the renderer.
=================
*/
#define	MAX_MARK_FRAGMENTS	128
#define	MAX_MARK_POINTS		384

void CG_ImpactMarkEx( qhandle_t markShader, const vec3_t origin, const vec3_t dir, 
					  float orientation, float red, float green, float blue, float alpha,
					  qboolean alphaFade, float xRadius, float yRadius, qboolean temporary )
{
	vec3_t			axis[3];
	float			xTexCoordScale, yTexCoordScale;
	vec3_t			originalPoints[4];
	byte			colors[4];
	int				i, j;
	int				numFragments;
	markFragment_t	markFragments[MAX_MARK_FRAGMENTS], *mf;
	vec3_t			markPoints[MAX_MARK_POINTS];
	vec3_t			projection;

	// client marks disabled?
	if( !cg_addMarks.integer )
	{
		return;
	}

	if ( xRadius <= 0 || yRadius <= 0 )
	{
		CG_Error( "CG_ImpactMarkEx called with xradius AND/OR yradius <= 0" );
	}
/*
	if ( markTotal >= MAX_MARK_POLYS )
	{
		return;
	}
*/
	// create the texture axis
	VectorNormalize2( dir, axis[0] );

	// HACK: to prevent axis flipping and incorrect orientation rotations just fudge the
	// axis[0] vector for now
#pragma message ("CG_ImpactMarkEx() - 'straight up' axis hack in (for shadows - may impact other marks)")
	axis[0][0]=0;
	axis[0][1]=0;
	axis[0][2]=1;
	
	PerpendicularVector( axis[1], axis[0] );
	RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
	CrossProduct( axis[0], axis[2], axis[1] );

	xTexCoordScale = 0.5 * 1.0 / xRadius;
	yTexCoordScale = 0.5 * 1.0 / yRadius;

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ )
	{
		originalPoints[0][i] = origin[i] - yRadius * axis[1][i] - xRadius * axis[2][i];
		originalPoints[1][i] = origin[i] + yRadius * axis[1][i] - xRadius * axis[2][i];
		originalPoints[2][i] = origin[i] + yRadius * axis[1][i] + xRadius * axis[2][i];
		originalPoints[3][i] = origin[i] - yRadius * axis[1][i] + xRadius * axis[2][i];
	}

	// get the fragments
	VectorScale( dir, -20, projection );
	numFragments = trap_CM_MarkFragments( 4, (void *)originalPoints,
										  projection, MAX_MARK_POINTS, markPoints[0],
										  MAX_MARK_FRAGMENTS, markFragments );

	// create the colour
	colors[0] = red * 255;
	colors[1] = green * 255;
	colors[2] = blue * 255;
	colors[3] = alpha * 255;

	// process the fragments
	for ( i = 0, mf = markFragments ; i < numFragments ; i++, mf++ )
	{
		polyVert_t	*v;
		polyVert_t	verts[MAX_VERTS_ON_POLY];
		markPoly_t	*mark;

		// we have an upper limit on the complexity of polygons
		// that we store persistantly
		if ( mf->numPoints > MAX_VERTS_ON_POLY ) {
			 mf->numPoints = MAX_VERTS_ON_POLY;
		}

		// for each vert on the poly
		for( j = 0, v = verts ; j < mf->numPoints ; j++, v++ )
		{
			vec3_t delta;

			VectorCopy( markPoints[mf->firstPoint + j], v->xyz );
			VectorSubtract( v->xyz, origin, delta );

			// texture coords
			v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * yTexCoordScale;
			v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * xTexCoordScale;

			// colour
			*(int *)v->modulate = *(int *)colors;
		}

		// if it is a temporary (shadow) mark, add it immediately and forget about it
		if ( temporary )
		{
			trap_R_AddPolyToScene( markShader, mf->numPoints, verts );
			continue;
		}

		// otherwise save it persistantly
		mark = CG_AllocMark();
		mark->time = cg.time;
		mark->alphaFade = alphaFade;
		mark->markShader = markShader;
		mark->poly.numVerts = mf->numPoints;
		mark->color[0] = red;
		mark->color[1] = green;
		mark->color[2] = blue;
		mark->color[3] = alpha;
		memcpy( mark->verts, verts, mf->numPoints * sizeof( verts[0] ) );
		markTotal++;
	}
}


/*
===============
CG_AddMarks
===============
*/
#define	MARK_TOTAL_TIME		10000
#define	MARK_FADE_TIME		1000

void CG_AddMarks( void ) {
	int			j;
	markPoly_t	*mp, *next;
	int			t;
	int			fade;

	if ( !cg_addMarks.integer ) {
		return;
	}

	mp = cg_activeMarkPolys.nextMark;
	for ( ; mp != &cg_activeMarkPolys ; mp = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = mp->nextMark;

		// see if it is time to completely remove it
		if ( cg.time > mp->time + MARK_TOTAL_TIME ) {
			CG_FreeMarkPoly( mp );
			continue;
		}

		// fade all marks out with time
		t = mp->time + MARK_TOTAL_TIME - cg.time;
		if ( t < MARK_FADE_TIME ) {
			fade = 255 * t / MARK_FADE_TIME;
			if ( mp->alphaFade ) {
				for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
					mp->verts[j].modulate[3] = fade;
				}
			} else {
				for ( j = 0 ; j < mp->poly.numVerts ; j++ ) {
					mp->verts[j].modulate[0] = mp->color[0] * fade;
					mp->verts[j].modulate[1] = mp->color[1] * fade;
					mp->verts[j].modulate[2] = mp->color[2] * fade;
				}
			}
		}


		trap_R_AddPolyToScene( mp->markShader, mp->poly.numVerts, mp->verts );
	}
}

