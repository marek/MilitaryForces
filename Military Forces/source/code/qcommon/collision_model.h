#ifndef COLLISION_MODEL_H
#define COLLISION_MODEL_H

class CollisionModel
{
public:
	static void loadMap( const char *name, bool clientload, int *checksum);
	static void clearMap( );
	static clipHandle_t inlineModel( int index );
	static clipHandle_t tempBoxModel( const vec3_t mins, const vec3_t maxs, int capsule );
	static void modelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
	static int numClusters ();
	static int numInlineModels( );
	static char *entityString ();
	static int pointContents( const vec3_t p, clipHandle_t model );
	static int transformedPointContents( const vec3_t p, clipHandle_t model, const vec3_t origin, const vec3_t angles );
	static void boxTrace ( trace_t *results, const vec3_t start, const vec3_t end, vec3_t mins, vec3_t maxs, clipHandle_t model, int brushmask, int capsule );
	static void transformedBoxTrace( trace_t *results, const vec3_t start, const vec3_t end, vec3_t mins, vec3_t maxs, clipHandle_t model, int brushmask, const vec3_t origin, const vec3_t angles, int capsule );
	static byte *clusterPVS (int cluster);
	static int pointLeafnum( const vec3_t p );
	static int boxLeafnums( const vec3_t mins, const vec3_t maxs, int *list, int listsize, int *lastLeaf );
	static int leafCluster (int leafnum);
	static int leafArea (int leafnum);
	static void adjustAreaPortalState( int area1, int area2, bool open );
	static bool areasConnected( int area1, int area2 );
	static int writeAreaBits( byte *buffer, int area );
	static void drawDebugSurface( void (*drawPoly)(int color, int numPoints, float *points) );
};

#endif
