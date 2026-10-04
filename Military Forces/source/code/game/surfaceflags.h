/*
 * $Id: surfaceflags.h,v 1.2 2005-09-02 08:00:21 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// This file must be identical in the quake and utils directories

// contents flags are seperate bits
// a given brush can contribute multiple content bits

// these definitions also need to be in q_shared.h!

#ifndef __SURFACEFLAGS_H__
#define __SURFACEFLAGS_H__

#define	CONTENTS_SOLID			1		// an eye is never valid in a solid
#define	CONTENTS_LAVA			8
#define	CONTENTS_SLIME			16
#define	CONTENTS_WATER			32
#define	CONTENTS_FOG			64

#define	CONTENTS_AREAPORTAL		0x8000

#define	CONTENTS_PLAYERCLIP		0x10000
#define	CONTENTS_MONSTERCLIP	0x20000
//bot specific contents types
inline constexpr int CONTENTS_TELEPORTER = 0x40000;
inline constexpr int CONTENTS_JUMPPAD = 0x80000;
inline constexpr int CONTENTS_CLUSTERPORTAL = 0x100000;
#define CONTENTS_DONOTENTER		0x200000
inline constexpr int CONTENTS_BOTCLIP = 0x400000;
inline constexpr int CONTENTS_MOVER = 0x800000;

#define	CONTENTS_ORIGIN			0x1000000	// removed before bsping an entity

inline constexpr int CONTENTS_BODY = 0x2000000; // should never be on a brush, only in game
inline constexpr int CONTENTS_CORPSE = 0x4000000;
#define	CONTENTS_DETAIL			0x8000000	// brushes not used for the bsp
#define	CONTENTS_STRUCTURAL		0x10000000	// brushes used for the bsp
#define	CONTENTS_TRANSLUCENT	0x20000000	// don't consume surface fragments inside
inline constexpr int CONTENTS_TRIGGER = 0x40000000;
inline constexpr unsigned int CONTENTS_NODROP = 0x80000000; // don't leave bodies or items (death fog, lava)

inline constexpr int SURF_NODAMAGE = 0x1; // never give falling damage
#define	SURF_SLICK				0x2		// effects game physics
#define	SURF_SKY				0x4		// lighting from environment map
inline constexpr int SURF_LADDER = 0x8;
#define	SURF_NOIMPACT			0x10	// don't make missile explosions
inline constexpr int SURF_NOMARKS = 0x20; // don't leave missile marks
inline constexpr int SURF_FLESH = 0x40; // make flesh sounds and effects
#define	SURF_NODRAW				0x80	// don't generate a drawsurface at all
#define	SURF_HINT				0x100	// make a primary bsp splitter
#define	SURF_SKIP				0x200	// completely ignore, allowing non-closed brushes
inline constexpr int SURF_NOLIGHTMAP = 0x400; // surface doesn't need a lightmap
inline constexpr int SURF_POINTLIGHT = 0x800; // generate lighting info at vertexes
inline constexpr int SURF_METALSTEPS = 0x1000; // clanking footsteps
inline constexpr int SURF_NOSTEPS = 0x2000; // no footstep sounds
inline constexpr int SURF_NONSOLID = 0x4000; // don't collide against curves with this set
inline constexpr int SURF_LIGHTFILTER = 0x8000; // act as a light filter during q3map -light
inline constexpr int SURF_ALPHASHADOW = 0x10000; // do per-pixel light shadow casting in q3map
inline constexpr int SURF_NODLIGHT = 0x20000; // don't dlight even if solid (solid lava, skies)
inline constexpr int SURF_DUST = 0x40000; // leave a dust trail when walking on this surface


#endif // __SURFACEFLAGS_H__

