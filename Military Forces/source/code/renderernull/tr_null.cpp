#include "../game/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../qcommon/qcommon.h"
#include "../renderercommon/tr_public.h"

refimport_t	ri;

static int nextHandle = 1;

static void RN_Shutdown( bool destroyWindow ) {}

static void RN_BeginRegistration( glconfig_t *config )
{
	memset( config, 0, sizeof( *config ) );
	config->colorBits = 32;
	config->depthBits = 24;
	config->stencilBits = 8;
	config->vidWidth = 640;
	config->vidHeight = 480;
	config->windowAspect = 640.0f / 480.0f;
	config->displayFrequency = 60;
}

static qhandle_t RN_Register( const char *name ) { return nextHandle++; }
static void RN_LoadWorld( const char *name ) {}
static void RN_SetWorldVisData( const byte *vis ) {}
static void RN_EndRegistration() {}
static void RN_ClearScene() {}
static void RN_AddRefEntityToScene( const refEntity_t *re ) {}
static void RN_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int num ) {}
static int RN_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir ) { return 0; }
static void RN_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {}
static void RN_RenderScene( const refdef_t *fd ) {}
static void RN_SetColor( const float *rgba ) {}
static void RN_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader ) {}
static void RN_DrawStretchRaw( int x, int y, int w, int h, int cols, int rows, const byte *data, int client, bool dirty ) {}
static void RN_UploadCinematic( int w, int h, int cols, int rows, const byte *data, int client, bool dirty ) {}
static void RN_BeginFrame( stereoFrame_t stereoFrame ) {}

static void RN_EndFrame( int *frontEndMsec, int *backEndMsec )
{
	if( frontEndMsec ) *frontEndMsec = 0;
	if( backEndMsec ) *backEndMsec = 0;
}

static int RN_MarkFragments( int numPoints, const vec3_t *points, const vec3_t projection,
	int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer ) { return 0; }

static int RN_LerpTag( orientation_t *tag, qhandle_t model, int startFrame, int endFrame, float frac, const char *tagName )
{
	memset( tag, 0, sizeof( *tag ) );
	tag->axis[0][0] = tag->axis[1][1] = tag->axis[2][2] = 1.0f;
	return 0;
}

static void RN_ModelBounds( qhandle_t model, vec3_t mins, vec3_t maxs )
{
	VectorClear( mins );
	VectorClear( maxs );
}

static void RN_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font )
{
	memset( font, 0, sizeof( *font ) );
	font->glyphScale = 1.0f;
}

static void RN_RemapShader( const char *oldShader, const char *newShader, const char *offsetTime ) {}
static bool RN_GetEntityToken( char *buffer, int size ) { return false; }
static bool RN_inPVS( const vec3_t p1, const vec3_t p2 ) { return true; }
static void RN_TakeVideoFrame( int h, int w, byte *captureBuffer, byte *encodeBuffer, bool motionJpeg ) {}

#ifdef USE_RENDERER_DLOPEN
Q_EXPORT refexport_t* QDECL GetRefAPI( int apiVersion, refimport_t *rimp ) {
#else
refexport_t *GetRefAPI( int apiVersion, refimport_t *rimp ) {
#endif
	static refexport_t re;

	ri = *rimp;
	memset( &re, 0, sizeof( re ) );

	if( apiVersion != REF_API_VERSION ) {
		ri.Printf( PRINT_ALL, "Mismatched REF_API_VERSION: expected %i, got %i\n", REF_API_VERSION, apiVersion );
		return nullptr;
	}

	re.Shutdown = RN_Shutdown;
	re.BeginRegistration = RN_BeginRegistration;
	re.RegisterModel = RN_Register;
	re.RegisterSkin = RN_Register;
	re.RegisterShader = RN_Register;
	re.RegisterShaderNoMip = RN_Register;
	re.LoadWorld = RN_LoadWorld;
	re.SetWorldVisData = RN_SetWorldVisData;
	re.EndRegistration = RN_EndRegistration;
	re.ClearScene = RN_ClearScene;
	re.AddRefEntityToScene = RN_AddRefEntityToScene;
	re.AddPolyToScene = RN_AddPolyToScene;
	re.LightForPoint = RN_LightForPoint;
	re.AddLightToScene = RN_AddLightToScene;
	re.AddAdditiveLightToScene = RN_AddLightToScene;
	re.RenderScene = RN_RenderScene;
	re.SetColor = RN_SetColor;
	re.DrawStretchPic = RN_DrawStretchPic;
	re.DrawStretchRaw = RN_DrawStretchRaw;
	re.UploadCinematic = RN_UploadCinematic;
	re.BeginFrame = RN_BeginFrame;
	re.EndFrame = RN_EndFrame;
	re.MarkFragments = RN_MarkFragments;
	re.LerpTag = RN_LerpTag;
	re.ModelBounds = RN_ModelBounds;
	re.RegisterFont = RN_RegisterFont;
	re.RemapShader = RN_RemapShader;
	re.GetEntityToken = RN_GetEntityToken;
	re.inPVS = RN_inPVS;
	re.TakeVideoFrame = RN_TakeVideoFrame;

	return &re;
}
