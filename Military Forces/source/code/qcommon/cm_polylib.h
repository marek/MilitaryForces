#ifndef __CM_POLYLIB_H__
#define __CM_POLYLIB_H__

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

// this is only used for visualization tools in cm_ debug functions

class Winding
{
public:
	void print();
	static Winding* alloc( int points );
	void free();
	void removeColinearPoints();
	void plane( vec3_t normal, vec_t *dist );
	vec_t area();
	void bounds( vec3_t mins, vec3_t maxs );
	void center( vec3_t center );
	static Winding* baseForPlane( vec3_t normal, vec_t dist );
	Winding* copy();
	Winding* reverse();
	void clipEpsilon( vec3_t normal, vec_t dist, vec_t epsilon, Winding **front, Winding **back );
	static void chopInPlace( Winding **inout, vec3_t normal, vec_t dist, vec_t epsilon );
	Winding* chop( vec3_t normal, vec_t dist );
	void check();
	int onPlaneSide( vec3_t normal, vec_t dist );
	static void addToConvexHull( Winding *w, Winding **hull, vec3_t normal );

	int		numpoints;
	vec3_t	p[4];		// variable sized

private:
	static int c_removed;
	static int c_winding_points;
	static int c_winding_allocs;
	static int c_peak_windings;
	static int c_active_windings;
};

#define	MAX_POINTS_ON_WINDING	64

#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2
#define	SIDE_CROSS	3

#define	CLIP_EPSILON	0.1f

#define MAX_MAP_BOUNDS			65535

// you can define on_epsilon in the makefile as tighter
#ifndef	ON_EPSILON
#define	ON_EPSILON	0.1f
#endif



#endif // __CM_POLYLIB_H__
