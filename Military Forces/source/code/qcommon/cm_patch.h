#ifndef __CM_PATCH_H__
#define __CM_PATCH_H__

/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

//#define	CULL_BBOX

/*

This file does not reference any globals, and has these entry points:

void CollisionWork::clearLevelPatches( void );
struct patchCollide_s	*CollisionWork::generatePatchCollide( int width, int height, const vec3_t *points );
void CollisionWork::traceThroughPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
bool CollisionWork::positionTestInPatchCollide( traceWork_t *tw, const struct patchCollide_s *pc );
void CollisionModel::drawDebugSurface( void (*drawPoly)(int color, int numPoints, flaot *points) );


Issues for collision against curved surfaces:

Surface edges need to be handled differently than surface planes

Plane expansion causes raw surfaces to expand past expanded bounding box

Position test of a volume against a surface is tricky.

Position test of a point against a surface is not well defined, because the surface has no volume.


Tracing leading edge points instead of volumes?
Position test by tracing corner to corner? (8*7 traces -- ouch)

coplanar edges
triangulated patches
degenerate patches

  endcaps
  degenerate

WARNING: this may misbehave with meshes that have rows or columns that only
degenerate a few triangles.  Completely degenerate rows and columns are handled
properly.
*/


#define	MAX_FACETS			1024
#define	MAX_PATCH_PLANES	4096

struct patchPlane_t {
	float	plane[4];
	int		signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
};

struct facet_t {
	int			surfacePlane;
	int			numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	int			borderPlanes[4+6+16];
	int			borderInward[4+6+16];
	bool	borderNoAdjust[4+6+16];
};

struct patchCollide_s {
	vec3_t	bounds[2];
	int		numPlanes;			// surface planes plus edge planes
	patchPlane_t	*planes;
	int		numFacets;
	facet_t	*facets;
};
using patchCollide_t = patchCollide_s;


#define	MAX_GRID_SIZE	129

struct cGrid_t {
	int			width;
	int			height;
	bool	wrapWidth;
	bool	wrapHeight;
	vec3_t	points[MAX_GRID_SIZE][MAX_GRID_SIZE];	// [width][height]
};

inline constexpr int SUBDIVIDE_DISTANCE = 16; //4	// never more than this units away from curve
inline constexpr double PLANE_TRI_EPSILON = 0.1;
inline constexpr double WRAP_POINT_EPSILON = 0.1;



#endif // __CM_PATCH_H__
