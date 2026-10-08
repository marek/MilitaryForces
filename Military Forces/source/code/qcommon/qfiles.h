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
#ifndef __QFILES_H__
#define __QFILES_H__

#include "../game/q_shared.h"

//
// qfiles.h: quake file formats
// This file must be identical in the quake and utils directories
//

// surface geometry should not exceed these limits
#define	SHADER_MAX_VERTEXES	10000
#define	SHADER_MAX_INDEXES	(6*SHADER_MAX_VERTEXES)


// the maximum size of game relative pathnames

/*
========================================================================

QVM files

========================================================================
*/

inline constexpr int VM_MAGIC = 0x12721444;
inline constexpr int VM_MAGIC_VER2 = 0x12721445;
struct vmHeader_t {
	int		vmMagic;

	int		instructionCount;

	int		codeOffset;
	int		codeLength;

	int		dataOffset;
	int		dataLength;
	int		litLength;			// ( dataLength - litLength ) should be byteswapped on load
	int		bssLength;			// zero filled memory appended to datalength

	//!!! below here is VM_MAGIC_VER2 !!!
	int		jtrgLength;			// number of jump table targets
};

/*
========================================================================

.MD3 triangle model file format

========================================================================
*/

#define MD3_IDENT			(('3'<<24)+('P'<<16)+('D'<<8)+'I')
#define MD3_VERSION			15

// limits
#define MD3_MAX_LODS		3
#define	MD3_MAX_TRIANGLES	8192	// per surface
#define MD3_MAX_VERTS		4096	// per surface
#define MD3_MAX_SHADERS		256		// per surface
#define MD3_MAX_FRAMES		1024	// per model
#define	MD3_MAX_SURFACES	32		// per model
#define MD3_MAX_TAGS		16		// per frame

// vertex scales
#define	MD3_XYZ_SCALE		(1.0/64)

struct md3Frame_s {
	vec3_t		bounds[2];
	vec3_t		localOrigin;
	float		radius;
	char		name[16];
};
using md3Frame_t = md3Frame_s;

struct md3Tag_s {
	char		name[MAX_QPATH];	// tag name
	vec3_t		origin;
	vec3_t		axis[3];
};
using md3Tag_t = md3Tag_s;

/*
** md3Surface_t
**
** CHUNK			SIZE
** header			sizeof( md3Surface_t )
** shaders			sizeof( md3Shader_t ) * numShaders
** triangles[0]		sizeof( md3Triangle_t ) * numTriangles
** st				sizeof( md3St_t ) * numVerts
** XyzNormals		sizeof( md3XyzNormal_t ) * numVerts * numFrames
*/
struct md3Surface_t {
	int		ident;				// 

	char	name[MAX_QPATH];	// polyset name

	int		flags;
	int		numFrames;			// all surfaces in a model should have the same

	int		numShaders;			// all surfaces in a model should have the same
	int		numVerts;

	int		numTriangles;
	int		ofsTriangles;

	int		ofsShaders;			// offset from start of md3Surface_t
	int		ofsSt;				// texture coords are common for all frames
	int		ofsXyzNormals;		// numVerts * numFrames

	int		ofsEnd;				// next surface follows
};

struct md3Shader_t {
	char			name[MAX_QPATH];
	int				shaderIndex;	// for in-game use
};

struct md3Triangle_t {
	int			indexes[3];
};

struct md3St_t {
	float		st[2];
};

struct md3XyzNormal_t {
	short		xyz[3];
	short		normal;
};

struct md3Header_t {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	int			flags;

	int			numFrames;
	int			numTags;			
	int			numSurfaces;

	int			numSkins;

	int			ofsFrames;			// offset for first frame
	int			ofsTags;			// numFrames * numTags
	int			ofsSurfaces;		// first surface, others follow

	int			ofsEnd;				// end of file
};

/*
==============================================================================

MDR file format

==============================================================================
*/

/*
 * Here are the definitions for Ravensoft's model format of md4. Raven stores their
 * playermodels in .mdr files, in some games, which are pretty much like the md4
 * format implemented by ID soft. It seems like ID's original md4 stuff is not used at all.
 * MDR is being used in EliteForce, JediKnight2 and Soldiers of Fortune2 (I think).
 * So this comes in handy for anyone who wants to make it possible to load player
 * models from these games.
 * This format has bone tags, which is similar to the thing you have in md3 I suppose.
 * Raven has released their version of md3view under GPL enabling me to add support
 * to this codebase. Thanks to Steven Howes aka Skinner for helping with example
 * source code.
 *
 * - Thilo Schulz (arny@ats.s.bawue.de)
 */

#define MDR_IDENT	(('5'<<24)+('M'<<16)+('D'<<8)+'R')
inline constexpr int MDR_VERSION = 2;
inline constexpr int MDR_MAX_BONES = 128;

struct mdrWeight_t {
	int			boneIndex;	// these are indexes into the boneReferences,
	float		   boneWeight;		// not the global per-frame bone list
	vec3_t		offset;
};

struct mdrVertex_t {
	vec3_t		normal;
	vec2_t		texCoords;
	int			numWeights;
	mdrWeight_t	weights[1];		// variable sized
};

struct mdrTriangle_t {
	int			indexes[3];
};

struct mdrSurface_t {
	int			ident;

	char		name[MAX_QPATH];	// polyset name
	char		shader[MAX_QPATH];
	int			shaderIndex;	// for in-game use

	int			ofsHeader;	// this will be a negative number

	int			numVerts;
	int			ofsVerts;

	int			numTriangles;
	int			ofsTriangles;

	// Bone references are a set of ints representing all the bones
	// present in any vertex weights for this surface.  This is
	// needed because a model may have surfaces that need to be
	// drawn at different sort times, and we don't want to have
	// to re-interpolate all the bones for each surface.
	int			numBoneReferences;
	int			ofsBoneReferences;

	int			ofsEnd;		// next surface follows
};

struct mdrBone_t {
	float		matrix[3][4];
};

struct mdrFrame_t {
	vec3_t		bounds[2];		// bounds of all surfaces of all LOD's for this frame
	vec3_t		localOrigin;		// midpoint of bounds, used for sphere cull
	float		radius;			// dist from localOrigin to corner
	char		name[16];
	mdrBone_t	bones[1];		// [numBones]
};

struct mdrCompBone_t {
        unsigned char Comp[24]; // MC_COMP_BYTES is in MatComp.h, but don't want to couple
};

struct mdrCompFrame_t {
        vec3_t          bounds[2];		// bounds of all surfaces of all LOD's for this frame
        vec3_t          localOrigin;		// midpoint of bounds, used for sphere cull
        float           radius;			// dist from localOrigin to corner
        mdrCompBone_t   bones[1];		// [numBones]
};

struct mdrLOD_t {
	int			numSurfaces;
	int			ofsSurfaces;		// first surface, others follow
	int			ofsEnd;				// next lod follows
};

struct mdrTag_t {
        int                     boneIndex;
        char            name[32];
};

struct mdrHeader_t {
	int			ident;
	int			version;

	char		name[MAX_QPATH];	// model name

	// frames and bones are shared by all levels of detail
	int			numFrames;
	int			numBones;
	int			ofsFrames;			// mdrFrame_t[numFrames]

	// each level of detail has completely separate sets of surfaces
	int			numLODs;
	int			ofsLODs;

        int                     numTags;
        int                     ofsTags;

	int			ofsEnd;				// end of file
};


/*
==============================================================================

  .BSP file format

==============================================================================
*/


#define BSP_IDENT	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
		// little-endian "IBSP"

inline constexpr int BSP_VERSION = 46;


// there shouldn't be any problem with increasing these values at the
// expense of more memory allocation in the utilities
#define	MAX_MAP_MODELS		0x400
#define	MAX_MAP_BRUSHES		0x8000
#define	MAX_MAP_ENTITIES	0x800
#define	MAX_MAP_ENTSTRING	0x40000
inline constexpr int MAX_MAP_SHADERS = 0x400;

#define	MAX_MAP_AREAS		0x100	// MAX_MAP_AREA_BYTES in q_shared must match!
inline constexpr int MAX_MAP_FOGS = 0x100;
#define	MAX_MAP_PLANES		0x20000
#define	MAX_MAP_NODES		0x20000
#define	MAX_MAP_BRUSHSIDES	0x20000
#define	MAX_MAP_LEAFS		0x20000
#define	MAX_MAP_LEAFFACES	0x20000
#define	MAX_MAP_LEAFBRUSHES 0x40000
#define	MAX_MAP_PORTALS		0x20000
#define	MAX_MAP_LIGHTING	0x800000
inline constexpr int MAX_MAP_LIGHTGRID = 0x800000;
#define	MAX_MAP_VISIBILITY	0x200000

inline constexpr int MAX_MAP_DRAW_SURFS = 0x20000;
inline constexpr int MAX_MAP_DRAW_VERTS = 0x80000;
inline constexpr int MAX_MAP_DRAW_INDEXES = 0x80000;


// key / value pair sizes in the entities lump
#define	MAX_KEY				32
#define	MAX_VALUE			1024

// the editor uses these predefined yaw angles to orient entities up or down
#define	ANGLE_UP			-1
#define	ANGLE_DOWN			-2

#define	LIGHTMAP_WIDTH		128
#define	LIGHTMAP_HEIGHT		128

inline constexpr int MAX_WORLD_COORD = ( 128*1024 );
inline constexpr int MIN_WORLD_COORD = ( -128*1024 );
#define WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

//=============================================================================


struct lump_t {
	int		fileofs, filelen;
};

#define	LUMP_ENTITIES		0
inline constexpr int LUMP_SHADERS = 1;
#define	LUMP_PLANES			2
#define	LUMP_NODES			3
#define	LUMP_LEAFS			4
inline constexpr int LUMP_LEAFSURFACES = 5;
#define	LUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
inline constexpr int LUMP_DRAWVERTS = 10;
inline constexpr int LUMP_DRAWINDEXES = 11;
inline constexpr int LUMP_FOGS = 12;
inline constexpr int LUMP_SURFACES = 13;
inline constexpr int LUMP_LIGHTMAPS = 14;
inline constexpr int LUMP_LIGHTGRID = 15;
#define	LUMP_VISIBILITY		16
#define	HEADER_LUMPS		17

struct dheader_t {
	int			ident;
	int			version;

	lump_t		lumps[HEADER_LUMPS];
};

struct dmodel_t {
	float		mins[3], maxs[3];
	int			firstSurface, numSurfaces;
	int			firstBrush, numBrushes;
};

struct dshader_t {
	char		shader[MAX_QPATH];
	int			surfaceFlags;
	int			contentFlags;
};

// planes x^1 is allways the opposite of plane x

struct dplane_t {
	float		normal[3];
	float		dist;
};

struct dnode_t {
	int			planeNum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	int			mins[3];		// for frustom culling
	int			maxs[3];
};

struct dleaf_t {
	int			cluster;			// -1 = opaque cluster (do I still store these?)
	int			area;

	int			mins[3];			// for frustum culling
	int			maxs[3];

	int			firstLeafSurface;
	int			numLeafSurfaces;

	int			firstLeafBrush;
	int			numLeafBrushes;
};

struct dbrushside_t {
	int			planeNum;			// positive plane side faces out of the leaf
	int			shaderNum;
};

struct dbrush_t {
	int			firstSide;
	int			numSides;
	int			shaderNum;		// the shader that determines the contents flags
};

struct dfog_t {
	char		shader[MAX_QPATH];
	int			brushNum;
	int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
};

struct drawVert_t {
	vec3_t		xyz;
	float		st[2];
	float		lightmap[2];
	vec3_t		normal;
	byte		color[4];
};

#define drawVert_t_cleared(x) drawVert_t (x) = {{0, 0, 0}, {0, 0}, {0, 0}, {0, 0, 0}, {0, 0, 0, 0}}

enum mapSurfaceType_t {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE
};

struct dsurface_t {
	int			shaderNum;
	int			fogNum;
	int			surfaceType;

	int			firstVert;
	int			numVerts;

	int			firstIndex;
	int			numIndexes;

	int			lightmapNum;
	int			lightmapX, lightmapY;
	int			lightmapWidth, lightmapHeight;

	vec3_t		lightmapOrigin;
	vec3_t		lightmapVecs[3];	// for patches, [0] and [1] are lodbounds

	int			patchWidth;
	int			patchHeight;
};


#endif
