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
// cmodel.c -- model loading

#include "cm_local.h"
#include "cm_simd.h"

#ifdef BSPC

#include "../bspc/l_qfiles.h"

void SetPlaneSignbits (cplane_t *out) {
	int	bits, j;

	// for fast box on planeside test
	bits = 0;
	for (j=0 ; j<3 ; j++) {
		if (out->normal[j] < 0) {
			bits |= 1<<j;
		}
	}
	out->signbits = bits;
}
#endif //BSPC

// to allow boxes to be treated as brush models, we allocate
// some extra indexes along with those needed by the map
constexpr int BOX_BRUSHES = 1;
constexpr int BOX_SIDES = 6;
constexpr int BOX_LEAFS = 2;
constexpr int BOX_PLANES = 12;

#define	LL(x) x=LittleLong(x)


clipMap_t	cm;
int			c_pointcontents;
int			c_traces, c_brush_traces, c_patch_traces;


static byte		*cmod_base;

#ifndef BSPC
cvar_t		*cm_noAreas;
cvar_t		*cm_noCurves;
cvar_t		*cm_wideNodeOffset;
cvar_t		*cm_playerCurveClip;
#endif

cmodel_t	box_model;
cplane_t	*box_planes;
cbrush_t	*box_brush;





/*
===============================================================================

					MAP LOADING

===============================================================================
*/

/*
=================
CMod_LoadShaders
=================
*/
void CMod_LoadShaders( lump_t *l ) {
	dshader_t	*in, *out;
	int			i, count;

	in = static_cast<dshader_t *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if (l->filelen % sizeof(*in)) {
		Common::error (ERR_DROP, "CMod_LoadShaders: funny lump size");
	}
	count = l->filelen / sizeof(*in);

	if (count < 1) {
		Common::error (ERR_DROP, "Map with no shaders");
	}
	cm.shaders = static_cast<dshader_t *>(Hunk::alloc( count * sizeof( *cm.shaders ), h_high ));
	cm.numShaders = count;

	Com_Memcpy( cm.shaders, in, count * sizeof( *cm.shaders ) );

	out = cm.shaders;
	for ( i=0 ; i<count ; i++, in++, out++ ) {
		out->contentFlags = LittleLong( out->contentFlags );
		out->surfaceFlags = LittleLong( out->surfaceFlags );
	}
}


/*
=================
CMod_LoadSubmodels
=================
*/
void CMod_LoadSubmodels( lump_t *l ) {
	dmodel_t	*in;
	cmodel_t	*out;
	int			i, j, count;
	int			*indexes;

	in = static_cast<dmodel_t *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if (l->filelen % sizeof(*in))
		Common::error (ERR_DROP, "CMod_LoadSubmodels: funny lump size");
	count = l->filelen / sizeof(*in);

	if (count < 1)
		Common::error (ERR_DROP, "Map with no models");
	cm.cmodels = static_cast<cmodel_t *>(Hunk::alloc( count * sizeof( *cm.cmodels ), h_high ));
	cm.numSubModels = count;

	if ( count > MAX_SUBMODELS ) {
		Common::error( ERR_DROP, "MAX_SUBMODELS exceeded" );
	}

	for ( i=0 ; i<count ; i++, in++)
	{
		out = &cm.cmodels[i];

		for (j=0 ; j<3 ; j++)
		{	// spread the mins / maxs by a pixel
			out->mins[j] = LittleFloat (in->mins[j]) - 1;
			out->maxs[j] = LittleFloat (in->maxs[j]) + 1;
		}

		if ( i == 0 ) {
			continue;	// world model doesn't need other info
		}

		// make a "leaf" just to hold the model's brushes and surfaces
		out->leaf.numLeafBrushes = LittleLong( in->numBrushes );
		indexes = static_cast<int *>(Hunk::alloc( out->leaf.numLeafBrushes * 4, h_high ));
		out->leaf.firstLeafBrush = indexes - cm.leafbrushes;
		for ( j = 0 ; j < out->leaf.numLeafBrushes ; j++ ) {
			indexes[j] = LittleLong( in->firstBrush ) + j;
		}

		out->leaf.numLeafSurfaces = LittleLong( in->numSurfaces );
		indexes = static_cast<int *>(Hunk::alloc( out->leaf.numLeafSurfaces * 4, h_high ));
		out->leaf.firstLeafSurface = indexes - cm.leafsurfaces;
		for ( j = 0 ; j < out->leaf.numLeafSurfaces ; j++ ) {
			indexes[j] = LittleLong( in->firstSurface ) + j;
		}
	}
}


/*
=================
CMod_LoadNodes

=================
*/
void CMod_LoadNodes( lump_t *l ) {
	dnode_t		*in;
	int			child;
	cNode_t		*out;
	int			i, j, count;
	
	in = static_cast<dnode_t *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if (l->filelen % sizeof(*in))
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	count = l->filelen / sizeof(*in);

	if (count < 1)
		Common::error (ERR_DROP, "Map has no nodes");
	cm.nodes = static_cast<cNode_t *>(Hunk::alloc( count * sizeof( *cm.nodes ), h_high ));
	cm.numNodes = count;

	out = cm.nodes;

	for (i=0 ; i<count ; i++, out++, in++)
	{
		out->plane = cm.planes + LittleLong( in->planeNum );
		for (j=0 ; j<2 ; j++)
		{
			child = LittleLong (in->children[j]);
			out->children[j] = child;
		}
	}

}

/*
=================
CM_BoundBrush

=================
*/
static void CM_BoundBrush( cbrush_t *b ) {
	b->bounds[0][0] = -b->sides[0].plane->dist;
	b->bounds[1][0] = b->sides[1].plane->dist;

	b->bounds[0][1] = -b->sides[2].plane->dist;
	b->bounds[1][1] = b->sides[3].plane->dist;

	b->bounds[0][2] = -b->sides[4].plane->dist;
	b->bounds[1][2] = b->sides[5].plane->dist;
}


/*
=================
CMod_LoadBrushes

=================
*/
void CMod_LoadBrushes( lump_t *l ) {
	dbrush_t	*in;
	cbrush_t	*out;
	int			i, count;

	in = static_cast<dbrush_t *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if (l->filelen % sizeof(*in)) {
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	}
	count = l->filelen / sizeof(*in);

	cm.brushes = static_cast<cbrush_t *>(Hunk::alloc( ( BOX_BRUSHES + count ) * sizeof( *cm.brushes ), h_high ));
	cm.numBrushes = count;

	out = cm.brushes;

	for ( i=0 ; i<count ; i++, out++, in++ ) {
		out->sides = cm.brushsides + LittleLong(in->firstSide);
		out->numsides = LittleLong(in->numSides);

		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Common::error( ERR_DROP, "CMod_LoadBrushes: bad shaderNum: %i", out->shaderNum );
		}
		out->contents = cm.shaders[out->shaderNum].contentFlags;

		CM_BoundBrush( out );

		// pack the side planes four to a block; unused slots hold a plane nothing can cross
		const int blocks = ( out->numsides + 3 ) / 4;
		out->planeBlocks = blocks ? static_cast<float *>(Hunk::alloc( blocks * cmPlaneBlockFloats * sizeof( float ), h_high )) : nullptr;
		for ( int side = 0 ; side < blocks * 4 ; side++ ) {
			float *block = out->planeBlocks + ( side / 4 ) * cmPlaneBlockFloats;
			const int lane = side & 3;
			if ( side < out->numsides ) {
				const cplane_t *plane = out->sides[side].plane;
				block[lane] = plane->normal[0];
				block[4 + lane] = plane->normal[1];
				block[8 + lane] = plane->normal[2];
				block[12 + lane] = plane->dist;
			} else {
				block[lane] = block[4 + lane] = block[8 + lane] = 0.0f;
				block[12 + lane] = 1.0e30f;
			}
		}
	}

}

/*
=================
CMod_LoadLeafs
=================
*/
void CMod_LoadLeafs (lump_t *l)
{
	int			i;
	cLeaf_t		*out;
	dleaf_t 	*in;
	int			count;
	
	in = static_cast<dleaf_t *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if (l->filelen % sizeof(*in))
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	count = l->filelen / sizeof(*in);

	if (count < 1)
		Common::error (ERR_DROP, "Map with no leafs");

	cm.leafs = static_cast<cLeaf_t *>(Hunk::alloc( ( BOX_LEAFS + count ) * sizeof( *cm.leafs ), h_high ));
	cm.numLeafs = count;

	out = cm.leafs;	
	for ( i=0 ; i<count ; i++, in++, out++)
	{
		out->cluster = LittleLong (in->cluster);
		out->area = LittleLong (in->area);
		out->firstLeafBrush = LittleLong (in->firstLeafBrush);
		out->numLeafBrushes = LittleLong (in->numLeafBrushes);
		out->firstLeafSurface = LittleLong (in->firstLeafSurface);
		out->numLeafSurfaces = LittleLong (in->numLeafSurfaces);

		if (out->cluster >= cm.numClusters)
			cm.numClusters = out->cluster + 1;
		if (out->area >= cm.numAreas)
			cm.numAreas = out->area + 1;
	}

	cm.areas = static_cast<cArea_t *>(Hunk::alloc( cm.numAreas * sizeof( *cm.areas ), h_high ));
	cm.areaPortals = static_cast<int *>(Hunk::alloc( cm.numAreas * cm.numAreas * sizeof( *cm.areaPortals ), h_high ));
}

/*
=================
CMod_LoadPlanes
=================
*/
void CMod_LoadPlanes (lump_t *l)
{
	int			i, j;
	cplane_t	*out;
	dplane_t 	*in;
	int			count;
	int			bits;
	
	in = static_cast<dplane_t *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if (l->filelen % sizeof(*in))
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	count = l->filelen / sizeof(*in);

	if (count < 1)
		Common::error (ERR_DROP, "Map with no planes");
	cm.planes = static_cast<cplane_t *>(Hunk::alloc( ( BOX_PLANES + count ) * sizeof( *cm.planes ), h_high ));
	cm.numPlanes = count;

	out = cm.planes;	

	for ( i=0 ; i<count ; i++, in++, out++)
	{
		bits = 0;
		for (j=0 ; j<3 ; j++)
		{
			out->normal[j] = LittleFloat (in->normal[j]);
			if (out->normal[j] < 0)
				bits |= 1<<j;
		}

		out->dist = LittleFloat (in->dist);
		out->type = PlaneTypeForNormal( out->normal );
		out->signbits = bits;
	}
}

/*
=================
CMod_LoadLeafBrushes
=================
*/
void CMod_LoadLeafBrushes (lump_t *l)
{
	int			i;
	int			*out;
	int		 	*in;
	int			count;
	
	in = static_cast<int *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if (l->filelen % sizeof(*in))
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	count = l->filelen / sizeof(*in);

	cm.leafbrushes = static_cast<int *>(Hunk::alloc( (count + BOX_BRUSHES) * sizeof( *cm.leafbrushes ), h_high ));
	cm.numLeafBrushes = count;

	out = cm.leafbrushes;

	for ( i=0 ; i<count ; i++, in++, out++) {
		*out = LittleLong (*in);
	}
}

/*
=================
CMod_LoadLeafSurfaces
=================
*/
void CMod_LoadLeafSurfaces( lump_t *l )
{
	int			i;
	int			*out;
	int		 	*in;
	int			count;
	
	in = static_cast<int *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if (l->filelen % sizeof(*in))
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	count = l->filelen / sizeof(*in);

	cm.leafsurfaces = static_cast<int *>(Hunk::alloc( count * sizeof( *cm.leafsurfaces ), h_high ));
	cm.numLeafSurfaces = count;

	out = cm.leafsurfaces;

	for ( i=0 ; i<count ; i++, in++, out++) {
		*out = LittleLong (*in);
	}
}

/*
=================
CMod_LoadBrushSides
=================
*/
void CMod_LoadBrushSides (lump_t *l)
{
	int				i;
	cbrushside_t	*out;
	dbrushside_t 	*in;
	int				count;
	int				num;

	in = static_cast<dbrushside_t *>(reinterpret_cast<void *>(cmod_base + l->fileofs));
	if ( l->filelen % sizeof(*in) ) {
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	}
	count = l->filelen / sizeof(*in);

	cm.brushsides = static_cast<cbrushside_t *>(Hunk::alloc( ( BOX_SIDES + count ) * sizeof( *cm.brushsides ), h_high ));
	cm.numBrushSides = count;

	out = cm.brushsides;	

	for ( i=0 ; i<count ; i++, in++, out++) {
		num = LittleLong( in->planeNum );
		out->plane = &cm.planes[num];
		out->shaderNum = LittleLong( in->shaderNum );
		if ( out->shaderNum < 0 || out->shaderNum >= cm.numShaders ) {
			Common::error( ERR_DROP, "CMod_LoadBrushSides: bad shaderNum: %i", out->shaderNum );
		}
		out->surfaceFlags = cm.shaders[out->shaderNum].surfaceFlags;
	}
}


/*
=================
CMod_LoadEntityString
=================
*/
void CMod_LoadEntityString( lump_t *l ) {
	cm.entityString = static_cast<char *>(Hunk::alloc( l->filelen, h_high ));
	cm.numEntityChars = l->filelen;
	Com_Memcpy (cm.entityString, cmod_base + l->fileofs, l->filelen);
}

/*
=================
CMod_LoadVisibility
=================
*/
constexpr int VIS_HEADER = 8;
void CMod_LoadVisibility( lump_t *l ) {
	int		len;
	byte	*buf;

    len = l->filelen;
	if ( !len ) {
		cm.clusterBytes = ( cm.numClusters + 31 ) & ~31;
		cm.visibility = static_cast<byte *>(Hunk::alloc( cm.clusterBytes, h_high ));
		Com_Memset( cm.visibility, 255, cm.clusterBytes );
		return;
	}
	buf = cmod_base + l->fileofs;

	cm.vised = true;
	cm.visibility = static_cast<byte *>(Hunk::alloc( len, h_high ));
	cm.numClusters = LittleLong( (reinterpret_cast<int *>(buf))[0] );
	cm.clusterBytes = LittleLong( (reinterpret_cast<int *>(buf))[1] );
	Com_Memcpy (cm.visibility, buf + VIS_HEADER, len - VIS_HEADER );
}

//==================================================================


/*
=================
CMod_LoadPatches
=================
*/
#define	MAX_PATCH_VERTS		1024
void CMod_LoadPatches( lump_t *surfs, lump_t *verts ) {
	drawVert_t	*dv, *dv_p;
	dsurface_t	*in;
	int			count;
	int			i, j;
	int			c;
	cPatch_t	*patch;
	vec3_t		points[MAX_PATCH_VERTS];
	int			width, height;
	int			shaderNum;

	in = static_cast<dsurface_t *>(reinterpret_cast<void *>(cmod_base + surfs->fileofs));
	if (surfs->filelen % sizeof(*in))
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");
	cm.numSurfaces = count = surfs->filelen / sizeof(*in);
	cm.surfaces = static_cast<cPatch_t **>(Hunk::alloc( cm.numSurfaces * sizeof( cm.surfaces[0] ), h_high ));

	dv = static_cast<drawVert_t *>(reinterpret_cast<void *>(cmod_base + verts->fileofs));
	if (verts->filelen % sizeof(*dv))
		Common::error (ERR_DROP, "MOD_LoadBmodel: funny lump size");

	// scan through all the surfaces, but only load patches,
	// not planar faces
	for ( i = 0 ; i < count ; i++, in++ ) {
		if ( LittleLong( in->surfaceType ) != MST_PATCH ) {
			continue;		// ignore other surfaces
		}
		// FIXME: check for non-colliding patches

		cm.surfaces[ i ] = patch = static_cast<cPatch_t *>(Hunk::alloc( sizeof( *patch ), h_high ));

		// load the full drawverts onto the stack
		width = LittleLong( in->patchWidth );
		height = LittleLong( in->patchHeight );
		c = width * height;
		if ( c > MAX_PATCH_VERTS ) {
			Common::error( ERR_DROP, "ParseMesh: MAX_PATCH_VERTS" );
		}

		dv_p = dv + LittleLong( in->firstVert );
		for ( j = 0 ; j < c ; j++, dv_p++ ) {
			points[j][0] = LittleFloat( dv_p->xyz[0] );
			points[j][1] = LittleFloat( dv_p->xyz[1] );
			points[j][2] = LittleFloat( dv_p->xyz[2] );
		}

		shaderNum = LittleLong( in->shaderNum );
		patch->contents = cm.shaders[shaderNum].contentFlags;
		patch->surfaceFlags = cm.shaders[shaderNum].surfaceFlags;

		// create the internal facet structure
		patch->pc = CollisionWork::generatePatchCollide( width, height, points );
	}
}

//==================================================================

static unsigned CM_LumpChecksum(lump_t *lump) {
	return LittleLong (Com_BlockChecksum (cmod_base + lump->fileofs, lump->filelen));
}

static unsigned CM_Checksum(dheader_t *header) {
	unsigned checksums[16];
	checksums[0] = CM_LumpChecksum(&header->lumps[LUMP_SHADERS]);
	checksums[1] = CM_LumpChecksum(&header->lumps[LUMP_LEAFS]);
	checksums[2] = CM_LumpChecksum(&header->lumps[LUMP_LEAFBRUSHES]);
	checksums[3] = CM_LumpChecksum(&header->lumps[LUMP_LEAFSURFACES]);
	checksums[4] = CM_LumpChecksum(&header->lumps[LUMP_PLANES]);
	checksums[5] = CM_LumpChecksum(&header->lumps[LUMP_BRUSHSIDES]);
	checksums[6] = CM_LumpChecksum(&header->lumps[LUMP_BRUSHES]);
	checksums[7] = CM_LumpChecksum(&header->lumps[LUMP_MODELS]);
	checksums[8] = CM_LumpChecksum(&header->lumps[LUMP_NODES]);
	checksums[9] = CM_LumpChecksum(&header->lumps[LUMP_SURFACES]);
	checksums[10] = CM_LumpChecksum(&header->lumps[LUMP_DRAWVERTS]);

	return LittleLong(Com_BlockChecksum(checksums, 11 * 4));
}

/*
==================
CollisionModel::loadMap

Loads in the map and all submodels
==================
*/
void CollisionModel::loadMap( const char *name, bool clientload, int *checksum ) {
	union {
		int				*i;
		void			*v;
	} buf;
	int				i;
	dheader_t		header;
	int				length;
	static unsigned	last_checksum;

	if ( !name || !name[0] ) {
		Common::error( ERR_DROP, "CollisionModel::loadMap: NULL name" );
	}

#ifndef BSPC
	cm_noAreas = CvarSystem::get ("cm_noAreas", "0", CVAR_CHEAT);
	cm_noCurves = CvarSystem::get ("cm_noCurves", "0", CVAR_CHEAT);
	cm_wideNodeOffset = CvarSystem::get ("cm_wideNodeOffset", "0", CVAR_CHEAT);
	cm_playerCurveClip = CvarSystem::get ("cm_playerCurveClip", "1", CVAR_ARCHIVE|CVAR_CHEAT );
#endif
	Common::dPrintf( "CollisionModel::loadMap( %s, %i )\n", name, clientload );

	if ( !strcmp( cm.name, name ) && clientload ) {
		*checksum = last_checksum;
		return;
	}

	// free old stuff
	Com_Memset( &cm, 0, sizeof( cm ) );
	CollisionWork::clearLevelPatches();

	if ( !name[0] ) {
		cm.numLeafs = 1;
		cm.numClusters = 1;
		cm.numAreas = 1;
		cm.cmodels = static_cast<cmodel_t *>(Hunk::alloc( sizeof( *cm.cmodels ), h_high ));
		*checksum = 0;
		return;
	}

	//
	// load the file
	//
#ifndef BSPC
	length = FileSystem::readFile( name, &buf.v );
#else
	length = LoadQuakeFile((quakefile_t *) name, &buf.v);
#endif

	if ( !buf.i ) {
		Common::error (ERR_DROP, "Couldn't load %s", name);
	}

	last_checksum = LittleLong (Com_BlockChecksum (buf.i, length));
	*checksum = last_checksum;

	header = *reinterpret_cast<dheader_t *>(buf.i);
	for (i=0 ; i<sizeof(dheader_t)/4 ; i++) {
		(reinterpret_cast<int *>(&header))[i] = LittleLong ( (reinterpret_cast<int *>(&header))[i]);
	}

	if ( header.version != BSP_VERSION ) {
		Common::error (ERR_DROP, "CollisionModel::loadMap: %s has wrong version number (%i should be %i)"
		, name, header.version, BSP_VERSION );
	}

	cmod_base = reinterpret_cast<byte *>(buf.i);

	// load into heap
	CMod_LoadShaders( &header.lumps[LUMP_SHADERS] );
	CMod_LoadLeafs (&header.lumps[LUMP_LEAFS]);
	CMod_LoadLeafBrushes (&header.lumps[LUMP_LEAFBRUSHES]);
	CMod_LoadLeafSurfaces (&header.lumps[LUMP_LEAFSURFACES]);
	CMod_LoadPlanes (&header.lumps[LUMP_PLANES]);
	CMod_LoadBrushSides (&header.lumps[LUMP_BRUSHSIDES]);
	CMod_LoadBrushes (&header.lumps[LUMP_BRUSHES]);
	CMod_LoadSubmodels (&header.lumps[LUMP_MODELS]);
	CMod_LoadNodes (&header.lumps[LUMP_NODES]);
	CMod_LoadEntityString (&header.lumps[LUMP_ENTITIES]);
	CMod_LoadVisibility( &header.lumps[LUMP_VISIBILITY] );
	CMod_LoadPatches( &header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS] );

	// we are NOT freeing the file, because it is cached for the ref
	FileSystem::freeFile (buf.v);

	CollisionWork::initBoxHull ();

	CollisionWork::floodAreaConnections ();

	// allow this to be cached if it is loaded by the server
	if ( !clientload ) {
		Q_strncpyz( cm.name, name, sizeof( cm.name ) );
	}
}

/*
==================
CollisionModel::clearMap
==================
*/
void CollisionModel::clearMap( ) {
	Com_Memset( &cm, 0, sizeof( cm ) );
	CollisionWork::clearLevelPatches();
}

/*
==================
CollisionWork::clipHandleToModel
==================
*/
cmodel_t	*CollisionWork::clipHandleToModel( clipHandle_t handle ) {
	if ( handle < 0 ) {
		Common::error( ERR_DROP, "CollisionWork::clipHandleToModel: bad handle %i", handle );
	}
	if ( handle < cm.numSubModels ) {
		return &cm.cmodels[handle];
	}
	if ( handle == BOX_MODEL_HANDLE ) {
		return &box_model;
	}
	if ( handle < MAX_SUBMODELS ) {
		Common::error( ERR_DROP, "CollisionWork::clipHandleToModel: bad handle %i < %i < %i", 
			cm.numSubModels, handle, MAX_SUBMODELS );
	}
	Common::error( ERR_DROP, "CollisionWork::clipHandleToModel: bad handle %i", handle + MAX_SUBMODELS );

	return nullptr;

}

/*
==================
CollisionModel::inlineModel
==================
*/
clipHandle_t	CollisionModel::inlineModel( int index ) {
	if ( index < 0 || index >= cm.numSubModels ) {
		Common::error (ERR_DROP, "CollisionModel::inlineModel: bad number");
	}
	return index;
}

int		CollisionModel::numClusters( ) {
	return cm.numClusters;
}

int		CollisionModel::numInlineModels( ) {
	return cm.numSubModels;
}

char	*CollisionModel::entityString( ) {
	return cm.entityString;
}

int		CollisionModel::leafCluster( int leafnum ) {
	if (leafnum < 0 || leafnum >= cm.numLeafs) {
		Common::error (ERR_DROP, "CollisionModel::leafCluster: bad number");
	}
	return cm.leafs[leafnum].cluster;
}

int		CollisionModel::leafArea( int leafnum ) {
	if ( leafnum < 0 || leafnum >= cm.numLeafs ) {
		Common::error (ERR_DROP, "CollisionModel::leafArea: bad number");
	}
	return cm.leafs[leafnum].area;
}

//=======================================================================


/*
===================
CollisionWork::initBoxHull

Set up the planes and nodes so that the six floats of a bounding box
can just be stored out and get a proper clipping hull structure.
===================
*/
void CollisionWork::initBoxHull ()
{
	int			i;
	int			side;
	cplane_t	*p;
	cbrushside_t	*s;

	box_planes = &cm.planes[cm.numPlanes];

	box_brush = &cm.brushes[cm.numBrushes];
	box_brush->numsides = 6;
	box_brush->sides = cm.brushsides + cm.numBrushSides;
	box_brush->contents = CONTENTS_BODY;

	box_model.leaf.numLeafBrushes = 1;
//	box_model.leaf.firstLeafBrush = cm.numBrushes;
	box_model.leaf.firstLeafBrush = cm.numLeafBrushes;
	cm.leafbrushes[cm.numLeafBrushes] = cm.numBrushes;

	for (i=0 ; i<6 ; i++)
	{
		side = i&1;

		// brush sides
		s = &cm.brushsides[cm.numBrushSides+i];
		s->plane = 	cm.planes + (cm.numPlanes+i*2+side);
		s->surfaceFlags = 0;

		// planes
		p = &box_planes[i*2];
		p->type = i>>1;
		p->signbits = 0;
		VectorClear (p->normal);
		p->normal[i>>1] = 1;

		p = &box_planes[i*2+1];
		p->type = 3 + (i>>1);
		p->signbits = 0;
		VectorClear (p->normal);
		p->normal[i>>1] = -1;

		SetPlaneSignbits( p );
	}	
}

/*
===================
CollisionModel::tempBoxModel

To keep everything totally uniform, bounding boxes are turned into small
BSP trees instead of being compared directly.
Capsules are handled differently though.
===================
*/
clipHandle_t CollisionModel::tempBoxModel( const vec3_t mins, const vec3_t maxs, int capsule ) {

	VectorCopy( mins, box_model.mins );
	VectorCopy( maxs, box_model.maxs );

	if ( capsule ) {
		return CAPSULE_MODEL_HANDLE;
	}

	box_planes[0].dist = maxs[0];
	box_planes[1].dist = -maxs[0];
	box_planes[2].dist = mins[0];
	box_planes[3].dist = -mins[0];
	box_planes[4].dist = maxs[1];
	box_planes[5].dist = -maxs[1];
	box_planes[6].dist = mins[1];
	box_planes[7].dist = -mins[1];
	box_planes[8].dist = maxs[2];
	box_planes[9].dist = -maxs[2];
	box_planes[10].dist = mins[2];
	box_planes[11].dist = -mins[2];

	VectorCopy( mins, box_brush->bounds[0] );
	VectorCopy( maxs, box_brush->bounds[1] );

	return BOX_MODEL_HANDLE;
}

/*
===================
CollisionModel::modelBounds
===================
*/
void CollisionModel::modelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs ) {
	cmodel_t	*cmod;

	cmod = CollisionWork::clipHandleToModel( model );
	VectorCopy( cmod->mins, mins );
	VectorCopy( cmod->maxs, maxs );
}


