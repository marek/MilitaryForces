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

#include <SDL.h>

#include "../renderer/tr_local.h"
#include "sdl_icon.h"

void IN_Init( void );
void IN_Shutdown( void );

typedef enum
{
	RSERR_OK,

	RSERR_INVALID_FULLSCREEN,
	RSERR_INVALID_MODE,

	RSERR_UNKNOWN
} rserr_t;

SDL_Window *SDL_window = NULL;
static SDL_GLContext SDL_glContext = NULL;

cvar_t *r_centerWindow;
cvar_t *r_sdlDriver;

/*
===============
GLimp_Shutdown
===============
*/
void GLimp_Shutdown( void )
{
	IN_Shutdown();

	if( SDL_glContext != NULL )
	{
		SDL_GL_DeleteContext( SDL_glContext );
		SDL_glContext = NULL;
	}

	if( SDL_window != NULL )
	{
		SDL_DestroyWindow( SDL_window );
		SDL_window = NULL;
	}

	SDL_QuitSubSystem( SDL_INIT_VIDEO );

	Com_Memset( &glConfig, 0, sizeof( glConfig ) );
	Com_Memset( &glState, 0, sizeof( glState ) );
}

/*
===============
GLimp_LogComment
===============
*/
void GLimp_LogComment( char *comment )
{
}

/*
===============
GLimp_SetMode
===============
*/
static int GLimp_SetMode( int mode, bool fullscreen )
{
	const char *glstring;
	int perChannelColorBits;
	int colorBits, depthBits, stencilBits;
	int i = 0;
	Uint32 flags = SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL;
	SDL_DisplayMode desktopMode;
	int display = 0;
	int x = SDL_WINDOWPOS_UNDEFINED, y = SDL_WINDOWPOS_UNDEFINED;
	SDL_Surface *icon = NULL;

	ri.Printf( PRINT_ALL, "Initializing OpenGL display\n" );

#ifdef USE_ICON
	icon = SDL_CreateRGBSurfaceFrom(
			(void *)CLIENT_WINDOW_ICON.pixel_data,
			CLIENT_WINDOW_ICON.width,
			CLIENT_WINDOW_ICON.height,
			CLIENT_WINDOW_ICON.bytes_per_pixel * 8,
			CLIENT_WINDOW_ICON.bytes_per_pixel * CLIENT_WINDOW_ICON.width,
#ifdef Q3_LITTLE_ENDIAN
			0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000
#else
			0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF
#endif
			);
#endif

	if( SDL_GetDesktopDisplayMode( display, &desktopMode ) != 0 )
	{
		Com_Memset( &desktopMode, 0, sizeof( SDL_DisplayMode ) );
	}

	ri.Printf( PRINT_ALL, "...setting mode %d:", mode );

	if( mode == -2 && desktopMode.h > 0 )
	{
		glConfig.vidWidth = desktopMode.w;
		glConfig.vidHeight = desktopMode.h;
		glConfig.windowAspect = (float)glConfig.vidWidth / (float)glConfig.vidHeight;
	}
	else if( !R_GetModeInfo( &glConfig.vidWidth, &glConfig.vidHeight, &glConfig.windowAspect, mode ) )
	{
		ri.Printf( PRINT_ALL, " invalid mode\n" );
		SDL_FreeSurface( icon );
		return RSERR_INVALID_MODE;
	}
	ri.Printf( PRINT_ALL, " %d %d\n", glConfig.vidWidth, glConfig.vidHeight );

	if( r_centerWindow->integer && !fullscreen && desktopMode.w > 0 )
	{
		x = ( desktopMode.w / 2 ) - ( glConfig.vidWidth / 2 );
		y = ( desktopMode.h / 2 ) - ( glConfig.vidHeight / 2 );
	}

	if( fullscreen )
	{
		flags |= SDL_WINDOW_FULLSCREEN;
		glConfig.isFullscreen = true;
	}
	else
	{
		glConfig.isFullscreen = false;
	}

	colorBits = r_colorbits->value;
	if( ( !colorBits ) || ( colorBits >= 32 ) )
		colorBits = 24;

	if( !r_depthbits->value )
		depthBits = 24;
	else
		depthBits = r_depthbits->value;

	stencilBits = r_stencilbits->value;

	for( i = 0; i < 16; i++ )
	{
		int testColorBits, testDepthBits, testStencilBits;
		int realColorBits[3];

		if( ( i % 4 ) == 0 && i )
		{
			switch( i / 4 )
			{
				case 2 :
					if( colorBits == 24 )
						colorBits = 16;
					break;
				case 1 :
					if( depthBits == 24 )
						depthBits = 16;
					else if( depthBits == 16 )
						depthBits = 8;
				case 3 :
					if( stencilBits == 24 )
						stencilBits = 16;
					else if( stencilBits == 16 )
						stencilBits = 8;
			}
		}

		testColorBits = colorBits;
		testDepthBits = depthBits;
		testStencilBits = stencilBits;

		if( ( i % 4 ) == 3 )
		{
			if( testColorBits == 24 )
				testColorBits = 16;
		}

		if( ( i % 4 ) == 2 )
		{
			if( testDepthBits == 24 )
				testDepthBits = 16;
			else if( testDepthBits == 16 )
				testDepthBits = 8;
		}

		if( ( i % 4 ) == 1 )
		{
			if( testStencilBits == 24 )
				testStencilBits = 16;
			else if( testStencilBits == 16 )
				testStencilBits = 8;
			else
				testStencilBits = 0;
		}

		if( testColorBits == 24 )
			perChannelColorBits = 8;
		else
			perChannelColorBits = 4;

		SDL_GL_ResetAttributes();
		SDL_GL_SetAttribute( SDL_GL_RED_SIZE, perChannelColorBits );
		SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, perChannelColorBits );
		SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, perChannelColorBits );
		SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, testDepthBits );
		SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, testStencilBits );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 1 );
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

		if( r_stereo->integer )
		{
			glConfig.stereoEnabled = true;
			SDL_GL_SetAttribute( SDL_GL_STEREO, 1 );
		}
		else
		{
			glConfig.stereoEnabled = false;
			SDL_GL_SetAttribute( SDL_GL_STEREO, 0 );
		}

		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

		if( ( SDL_window = SDL_CreateWindow( "Military Forces", x, y,
				glConfig.vidWidth, glConfig.vidHeight, flags ) ) == NULL )
		{
			ri.Printf( PRINT_DEVELOPER, "SDL_CreateWindow failed: %s\n", SDL_GetError() );
			continue;
		}

		if( fullscreen )
		{
			SDL_DisplayMode desiredMode;

			switch( testColorBits )
			{
				case 16: desiredMode.format = SDL_PIXELFORMAT_RGB565; break;
				case 24: desiredMode.format = SDL_PIXELFORMAT_RGB24;  break;
				default: ri.Printf( PRINT_DEVELOPER, "testColorBits is %d, can't fullscreen\n", testColorBits ); continue;
			}

			desiredMode.w = glConfig.vidWidth;
			desiredMode.h = glConfig.vidHeight;
			desiredMode.refresh_rate = glConfig.displayFrequency = r_displayRefresh->integer;
			desiredMode.driverdata = NULL;

			if( SDL_SetWindowDisplayMode( SDL_window, &desiredMode ) < 0 )
			{
				ri.Printf( PRINT_DEVELOPER, "SDL_SetWindowDisplayMode failed: %s\n", SDL_GetError() );
				continue;
			}
		}

		SDL_SetWindowIcon( SDL_window, icon );

		if( ( SDL_glContext = SDL_GL_CreateContext( SDL_window ) ) == NULL )
		{
			ri.Printf( PRINT_DEVELOPER, "SDL_GL_CreateContext failed: %s\n", SDL_GetError() );
			SDL_DestroyWindow( SDL_window );
			SDL_window = NULL;
			continue;
		}

		qglClearColor( 0, 0, 0, 1 );
		qglClear( GL_COLOR_BUFFER_BIT );
		SDL_GL_SwapWindow( SDL_window );

		if( SDL_GL_SetSwapInterval( r_swapInterval->integer ) == -1 )
		{
			ri.Printf( PRINT_DEVELOPER, "SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError() );
		}

		SDL_GL_GetAttribute( SDL_GL_RED_SIZE, &realColorBits[0] );
		SDL_GL_GetAttribute( SDL_GL_GREEN_SIZE, &realColorBits[1] );
		SDL_GL_GetAttribute( SDL_GL_BLUE_SIZE, &realColorBits[2] );
		SDL_GL_GetAttribute( SDL_GL_DEPTH_SIZE, &glConfig.depthBits );
		SDL_GL_GetAttribute( SDL_GL_STENCIL_SIZE, &glConfig.stencilBits );

		glConfig.colorBits = realColorBits[0] + realColorBits[1] + realColorBits[2];

		ri.Printf( PRINT_ALL, "Using %d color bits, %d depth, %d stencil display.\n",
				glConfig.colorBits, glConfig.depthBits, glConfig.stencilBits );
		break;
	}

	SDL_FreeSurface( icon );

	if( !SDL_window )
	{
		ri.Printf( PRINT_ALL, "Couldn't get a visual\n" );
		return RSERR_INVALID_MODE;
	}

	SDL_ShowWindow( SDL_window );

	glstring = (char *) qglGetString( GL_RENDERER );
	ri.Printf( PRINT_ALL, "GL_RENDERER: %s\n", glstring );

	return RSERR_OK;
}

/*
===============
GLimp_StartDriverAndSetMode
===============
*/
static bool GLimp_StartDriverAndSetMode( int mode, bool fullscreen )
{
	rserr_t err;

	if( !SDL_WasInit( SDL_INIT_VIDEO ) )
	{
		const char *driverName;

		if( SDL_Init( SDL_INIT_VIDEO ) != 0 )
		{
			ri.Printf( PRINT_ALL, "SDL_Init( SDL_INIT_VIDEO ) FAILED (%s)\n", SDL_GetError() );
			return false;
		}

		driverName = SDL_GetCurrentVideoDriver();
		ri.Printf( PRINT_ALL, "SDL using driver \"%s\"\n", driverName );
		ri.Cvar_Set( "r_sdlDriver", driverName );
	}

	err = (rserr_t)GLimp_SetMode( mode, fullscreen );

	switch( err )
	{
		case RSERR_INVALID_FULLSCREEN:
			ri.Printf( PRINT_ALL, "...WARNING: fullscreen unavailable in this mode\n" );
			return false;
		case RSERR_INVALID_MODE:
			ri.Printf( PRINT_ALL, "...WARNING: could not set the given mode (%d)\n", mode );
			return false;
		default:
			break;
	}

	return true;
}

/*
===============
GLimp_InitExtensions
===============
*/
static void GLimp_InitExtensions( void )
{
	if( !r_allowExtensions->integer )
	{
		ri.Printf( PRINT_ALL, "*** IGNORING OPENGL EXTENSIONS ***\n" );
		return;
	}

	ri.Printf( PRINT_ALL, "Initializing OpenGL extensions\n" );

	glConfig.textureCompression = TC_NONE;
	if( strstr( glConfig.extensions_string, "GL_S3_s3tc" ) )
	{
		if( r_ext_compressed_textures->integer )
		{
			glConfig.textureCompression = TC_S3TC;
			ri.Printf( PRINT_ALL, "...using GL_S3_s3tc\n" );
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_S3_s3tc\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_S3_s3tc not found\n" );
	}

	glConfig.textureEnvAddAvailable = false;
	if( strstr( glConfig.extensions_string, "EXT_texture_env_add" ) )
	{
		if( r_ext_texture_env_add->integer )
		{
			glConfig.textureEnvAddAvailable = true;
			ri.Printf( PRINT_ALL, "...using GL_EXT_texture_env_add\n" );
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_texture_env_add\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_texture_env_add not found\n" );
	}

	qglMultiTexCoord2fARB = NULL;
	qglActiveTextureARB = NULL;
	qglClientActiveTextureARB = NULL;
	if( strstr( glConfig.extensions_string, "GL_ARB_multitexture" ) )
	{
		if( r_ext_multitexture->integer )
		{
			qglMultiTexCoord2fARB = ( void ( APIENTRY * )( GLenum, GLfloat, GLfloat ) ) SDL_GL_GetProcAddress( "glMultiTexCoord2fARB" );
			qglActiveTextureARB = ( void ( APIENTRY * )( GLenum ) ) SDL_GL_GetProcAddress( "glActiveTextureARB" );
			qglClientActiveTextureARB = ( void ( APIENTRY * )( GLenum ) ) SDL_GL_GetProcAddress( "glClientActiveTextureARB" );

			if( qglActiveTextureARB )
			{
				GLint glint = 0;
				qglGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &glint );
				glConfig.maxActiveTextures = (int) glint;
				if( glConfig.maxActiveTextures > 1 )
				{
					ri.Printf( PRINT_ALL, "...using GL_ARB_multitexture\n" );
				}
				else
				{
					qglMultiTexCoord2fARB = NULL;
					qglActiveTextureARB = NULL;
					qglClientActiveTextureARB = NULL;
					ri.Printf( PRINT_ALL, "...not using GL_ARB_multitexture, < 2 texture units\n" );
				}
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_ARB_multitexture\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_ARB_multitexture not found\n" );
	}

	qglLockArraysEXT = NULL;
	qglUnlockArraysEXT = NULL;
	if( strstr( glConfig.extensions_string, "GL_EXT_compiled_vertex_array" ) )
	{
		if( r_ext_compiled_vertex_array->integer )
		{
			ri.Printf( PRINT_ALL, "...using GL_EXT_compiled_vertex_array\n" );
			qglLockArraysEXT = ( void ( APIENTRY * )( GLint, GLint ) ) SDL_GL_GetProcAddress( "glLockArraysEXT" );
			qglUnlockArraysEXT = ( void ( APIENTRY * )( void ) ) SDL_GL_GetProcAddress( "glUnlockArraysEXT" );
			if( !qglLockArraysEXT || !qglUnlockArraysEXT )
			{
				ri.Error( ERR_FATAL, "bad getprocaddress" );
			}
		}
		else
		{
			ri.Printf( PRINT_ALL, "...ignoring GL_EXT_compiled_vertex_array\n" );
		}
	}
	else
	{
		ri.Printf( PRINT_ALL, "...GL_EXT_compiled_vertex_array not found\n" );
	}
}

/*
===============
GLimp_Init
===============
*/
void GLimp_Init( void )
{
	ri.Printf( PRINT_DEVELOPER, "Glimp_Init( )\n" );

	r_sdlDriver = ri.Cvar_Get( "r_sdlDriver", "", CVAR_ROM );
	r_centerWindow = ri.Cvar_Get( "r_centerWindow", "0", CVAR_ARCHIVE | CVAR_LATCH );

	if( ri.Cvar_Get( "com_abnormalExit", "0", CVAR_ROM )->integer )
	{
		ri.Cvar_Set( "r_mode", va( "%d", 3 ) );
		ri.Cvar_Set( "r_fullscreen", "0" );
		ri.Cvar_Set( "r_centerWindow", "0" );
		ri.Cvar_Set( "com_abnormalExit", "0" );
	}

	if( GLimp_StartDriverAndSetMode( r_mode->integer, r_fullscreen->integer != 0 ) )
		goto success;

	if( r_mode->integer != 3 )
	{
		ri.Printf( PRINT_ALL, "Setting r_mode %d failed, falling back on r_mode %d\n", r_mode->integer, 3 );

		if( GLimp_StartDriverAndSetMode( 3, false ) )
			goto success;
	}

	ri.Error( ERR_FATAL, "GLimp_Init() - could not load OpenGL subsystem" );

success:
	glConfig.driverType = GLDRV_ICD;
	glConfig.hardwareType = GLHW_GENERIC;

	glConfig.deviceSupportsGamma = !r_ignorehwgamma->integer &&
		SDL_SetWindowBrightness( SDL_window, 1.0f ) >= 0;

	Q_strncpyz( glConfig.vendor_string, (char *) qglGetString( GL_VENDOR ), sizeof( glConfig.vendor_string ) );
	Q_strncpyz( glConfig.renderer_string, (char *) qglGetString( GL_RENDERER ), sizeof( glConfig.renderer_string ) );
	if( *glConfig.renderer_string && glConfig.renderer_string[strlen( glConfig.renderer_string ) - 1] == '\n' )
		glConfig.renderer_string[strlen( glConfig.renderer_string ) - 1] = 0;
	Q_strncpyz( glConfig.version_string, (char *) qglGetString( GL_VERSION ), sizeof( glConfig.version_string ) );
	Q_strncpyz( glConfig.extensions_string, (char *) qglGetString( GL_EXTENSIONS ), sizeof( glConfig.extensions_string ) );

	GLimp_InitExtensions();

	ri.Cvar_Get( "r_availableModes", "", CVAR_ROM );

	IN_Init();
}

/*
===============
GLimp_EndFrame
===============
*/
void GLimp_EndFrame( void )
{
	if( Q_stricmp( r_drawBuffer->string, "GL_FRONT" ) != 0 )
	{
		SDL_GL_SwapWindow( SDL_window );
	}

	if( r_fullscreen->modified )
	{
		bool fullscreen;
		bool needToToggle;

		fullscreen = !!( SDL_GetWindowFlags( SDL_window ) & SDL_WINDOW_FULLSCREEN );

		if( r_fullscreen->integer && ri.Cvar_Get( "in_nograb", "0", 0 )->integer )
		{
			ri.Printf( PRINT_ALL, "Fullscreen not allowed with in_nograb 1\n" );
			ri.Cvar_Set( "r_fullscreen", "0" );
			r_fullscreen->modified = false;
		}

		needToToggle = ( r_fullscreen->integer != 0 ) != fullscreen;

		if( needToToggle )
		{
			ri.Cmd_ExecuteText( EXEC_APPEND, "vid_restart\n" );
		}

		r_fullscreen->modified = false;
	}

	if( r_swapInterval->modified )
	{
		SDL_GL_SetSwapInterval( r_swapInterval->integer );
		r_swapInterval->modified = false;
	}
}

bool GLimp_SpawnRenderThread( void (*function)( void ) )
{
	ri.Printf( PRINT_WARNING, "ERROR: SMP support was disabled at compile time\n" );
	return false;
}

void *GLimp_RendererSleep( void )
{
	return NULL;
}

void GLimp_FrontEndSleep( void )
{
}

void GLimp_WakeRenderer( void *data )
{
}
