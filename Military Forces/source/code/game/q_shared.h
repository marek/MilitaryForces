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
//
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

#ifdef STANDALONE
  #define PRODUCT_NAME				"Military Forces"
  #define BASEGAME					"mfdata"
  #define CLIENT_WINDOW_TITLE		"Military Forces"
  #define CLIENT_WINDOW_MIN_TITLE	"MF"
  #define HOMEPATH_NAME_UNIX_LEGACY	".militaryforces"
  #define HOMEPATH_NAME				"MilitaryForces"
  // where settings live for a player: Documents\My Games on Windows, Application Support on macOS
  #define HOMEPATH_NAME_WINDOWS		"My Games\\Military Forces"
  #define HOMEPATH_NAME_MACOS		"Military Forces"
  #define GAMENAME_FOR_MASTER		"MilitaryForces"	// must NOT contain whitespace
  #define CONFIG_PREFIX			"mfconfig"
  #define MASTER_SERVER_NAME		"dpmaster.deathmask.net"
#else
  #define PRODUCT_NAME				"ioq3"
  #define BASEGAME					"baseq3"
  #define CLIENT_WINDOW_TITLE		"ioquake3"
  #define CLIENT_WINDOW_MIN_TITLE	"ioq3"
  #define HOMEPATH_NAME_UNIX_LEGACY	".q3a"
  #define HOMEPATH_NAME				"Quake3"
  #define GAMENAME_FOR_MASTER		"Quake3Arena"
  #define CINEMATICS_LOGO		"idlogo.RoQ"
  #define CINEMATICS_INTRO		"intro.RoQ"
  #define LEGACY_PROTOCOL
  #define PROTOCOL_HANDLER		"quake3"
  #define CONFIG_PREFIX			"q3config"
#endif

// Heartbeat for dpmaster protocol. You shouldn't change this unless you know what you're doing
#define HEARTBEAT_FOR_MASTER		"DarkPlaces"

// When com_gamename is LEGACY_MASTER_GAMENAME, use quake3 master protocol.
// You shouldn't change this unless you know what you're doing
#define LEGACY_MASTER_GAMENAME		"Quake3Arena"
#define LEGACY_HEARTBEAT_FOR_MASTER	"QuakeArena-1"

#define BASETA				"missionpack"

#ifndef PRODUCT_VERSION
  #define PRODUCT_VERSION "1.0.9"
#endif

#ifndef PRODUCT_DATE
#  define PRODUCT_DATE __DATE__
#endif

#define Q3_VERSION PRODUCT_NAME " " PRODUCT_VERSION
#define	MF_VERSION		Q3_VERSION

inline constexpr int MAX_TEAMNAME = 32;
inline constexpr int MAX_MASTER_SERVERS = 5; // number of supported master servers

#define DEMOEXT	"dm_"			// standard demo extension

#ifdef _MSC_VER

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
//#pragma warning(disable : 4201)
//#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)		// benign redefinition
//#pragma warning(disable : 4305)		// truncation from const double to float
//#pragma warning(disable : 4310)		// cast truncates constant value
//#pragma warning(disable:  4505) 	// unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
//#pragma intrinsic( memset, memcpy )
#endif

#ifdef __GNUC__
#define Q_UNUSED_VAR __attribute__((unused))
#define Q_NO_RETURN __attribute__((noreturn))

#ifdef __MINGW32__
// For some reason MinGW wants both gnu_printf and ms_printf
#define Q_PRINTF_FUNC(fmt, va) \
	__attribute__((format(gnu_printf, fmt, va))) \
	__attribute__((format(ms_printf, fmt, va)))
#else
#define Q_PRINTF_FUNC(fmt, va) __attribute__((format(printf, fmt, va)))
#endif

#define Q_SCANF_FUNC(fmt, va) __attribute__((format(scanf, fmt, va)))
#define Q_ALIGN(x) __attribute__((aligned(x)))
#else
#define Q_UNUSED_VAR
#define Q_NO_RETURN
#define Q_PRINTF_FUNC(fmt, va)
#define Q_SCANF_FUNC(fmt, va)
#define Q_ALIGN(x)
#endif

#if defined _WIN32
#define Q_EXPORT extern "C" __declspec(dllexport)
#elif ( defined __GNUC__ || defined __clang__ )
#define Q_EXPORT extern "C" __attribute__((visibility("default")))
#else
#define Q_EXPORT extern "C"
#endif

/**********************************************************************
  VM Considerations

  The VM can not use the standard system headers because we aren't really
  using the compiler they were meant for.  We use bg_lib.h which contains
  prototypes for the functions we define for our own use in bg_lib.c.

  When writing mods, please add needed headers HERE, do not start including
  stuff like <stdio.h> in the various .c files that make up each of the VMs
  since you will be including system headers files can will have issues.

  Remember, if you use a C library function that is not defined in bg_lib.c,
  you will have to add your own version for support in the VM.

 **********************************************************************/

#ifdef Q3_VM

#include "../game/bg_lib.h"

typedef int intptr_t;

#else

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>
#include <algorithm>
#include <source_location>
#include <numbers>
#include <stdint.h>

#ifdef _WIN32
  // vsnprintf is ISO/IEC 9899:1999
  // abstracting this to make it portable
  int Q_vsnprintf(char *str, size_t size, const char *format, va_list ap) Q_PRINTF_FUNC(3, 0);
#else
  #define Q_vsnprintf vsnprintf
#endif

#endif


#include "q_platform.h"

//=============================================================

typedef unsigned char 		byte;


union floatint_t {
	float f;
	int i;
	unsigned int ui;
};

typedef int		qhandle_t;
typedef int		sfxHandle_t;
typedef int		fileHandle_t;
typedef int		clipHandle_t;

#define PAD(base, alignment)	(((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment)	(PAD((base), (alignment)) - (base))

#define PADP(base, alignment)	((void *) PAD((intptr_t) (base), (alignment)))

#ifndef NULL
#define NULL ((void *)0)
#endif

#define STRING(s)			#s
// expand constants before stringifying them
#define XSTRING(s)			STRING(s)

inline constexpr int MAX_QINT = 0x7fffffff;
#define	MIN_QINT			(-MAX_QINT-1)

#define ARRAY_LEN(x)			(sizeof(x) / sizeof(*(x)))
#define STRARRAY_LEN(x)			(ARRAY_LEN(x) - 1)

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
inline constexpr int MAX_STRING_CHARS = 1024; // max length of a string passed to CommandSystem::tokenizeString
inline constexpr int MAX_STRING_TOKENS = 1024; // max tokens resulting from CommandSystem::tokenizeString
inline constexpr int MAX_TOKEN_CHARS = 1024; // max length of an individual token

inline constexpr int MAX_INFO_STRING = 1024;
inline constexpr int MAX_INFO_KEY = 1024;
inline constexpr int MAX_INFO_VALUE = 1024;

inline constexpr int BIG_INFO_STRING = 8192; // used for system info key only
inline constexpr int BIG_INFO_KEY = 8192;
inline constexpr int BIG_INFO_VALUE = 8192;


#define	MAX_QPATH			64		// max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

inline constexpr int MAX_NAME_LENGTH = 32; // max length of a client name
inline constexpr int MAX_HOSTNAME_LENGTH = 80; // max length of a host name

inline constexpr int MAX_SAY_TEXT = 150;

// parameters for command buffer stuffing
enum cbufExec_t {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
};


//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility


// print levels from renderer (FIXME: set up for game / cgame?)
enum printParm_t {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_WARNING,
	PRINT_ERROR
};


#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
enum errorParm_t {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
	ERR_NEED_CD					// pop up the need-cd dialog
};


// font rendering values used by ui and cgame

inline constexpr int PROP_GAP_WIDTH = 3;
inline constexpr int PROP_SPACE_WIDTH = 8;
inline constexpr int PROP_HEIGHT = 27;
inline constexpr double PROP_SMALL_SIZE_SCALE = 0.75;

inline constexpr int BLINK_DIVISOR = 200;
inline constexpr int PULSE_DIVISOR = 75;

inline constexpr int UI_LEFT = 0x00000000; // default
inline constexpr int UI_CENTER = 0x00000001;
inline constexpr int UI_RIGHT = 0x00000002;
inline constexpr int UI_FORMATMASK = 0x00000007;
inline constexpr int UI_SMALLFONT = 0x00000010;
inline constexpr int UI_BIGFONT = 0x00000020; // default
inline constexpr int UI_GIANTFONT = 0x00000040;
inline constexpr int UI_DROPSHADOW = 0x00000800;
inline constexpr int UI_BLINK = 0x00001000;
inline constexpr int UI_INVERSE = 0x00002000;
inline constexpr int UI_PULSE = 0x00004000;

#if !defined(NDEBUG) && !defined(BSPC)
	#define HUNK_DEBUG
#endif

enum ha_pref {
	h_high,
	h_low,
	h_dontcare
};


#define Com_Memset memset
#define Com_Memcpy memcpy

inline constexpr int CIN_system = 1;
inline constexpr int CIN_loop = 2;
inline constexpr int CIN_hold = 4;
inline constexpr int CIN_silent = 8;
inline constexpr int CIN_shader = 16;

/*
==============================================================

MATHLIB

==============================================================
*/


typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t vec5_t[5];

struct Vec3
{
	float v[3];

	constexpr Vec3() : v{ 0.0f, 0.0f, 0.0f } {}
	constexpr Vec3( float x, float y, float z ) : v{ x, y, z } {}
	constexpr explicit Vec3( const float *p ) : v{ p[0], p[1], p[2] } {}

	constexpr operator float*() { return v; }
	constexpr operator const float*() const { return v; }

	constexpr float x() const { return v[0]; }
	constexpr float y() const { return v[1]; }
	constexpr float z() const { return v[2]; }

	constexpr Vec3 operator+( const Vec3& o ) const { return Vec3( v[0] + o.v[0], v[1] + o.v[1], v[2] + o.v[2] ); }
	constexpr Vec3 operator-( const Vec3& o ) const { return Vec3( v[0] - o.v[0], v[1] - o.v[1], v[2] - o.v[2] ); }
	constexpr Vec3 operator-() const { return Vec3( -v[0], -v[1], -v[2] ); }
	constexpr Vec3 operator*( float s ) const { return Vec3( v[0] * s, v[1] * s, v[2] * s ); }
	constexpr Vec3 operator/( float s ) const { return Vec3( v[0] / s, v[1] / s, v[2] / s ); }
	constexpr Vec3& operator+=( const Vec3& o ) { v[0] += o.v[0]; v[1] += o.v[1]; v[2] += o.v[2]; return *this; }
	constexpr Vec3& operator-=( const Vec3& o ) { v[0] -= o.v[0]; v[1] -= o.v[1]; v[2] -= o.v[2]; return *this; }
	constexpr Vec3& operator*=( float s ) { v[0] *= s; v[1] *= s; v[2] *= s; return *this; }
	constexpr bool operator==( const Vec3& o ) const { return v[0] == o.v[0] && v[1] == o.v[1] && v[2] == o.v[2]; }

	constexpr float dot( const Vec3& o ) const { return v[0] * o.v[0] + v[1] * o.v[1] + v[2] * o.v[2]; }
	constexpr Vec3 cross( const Vec3& o ) const
	{
		return Vec3( v[1] * o.v[2] - v[2] * o.v[1], v[2] * o.v[0] - v[0] * o.v[2], v[0] * o.v[1] - v[1] * o.v[0] );
	}
	constexpr float lengthSquared() const { return dot( *this ); }
	float length() const { return sqrtf( lengthSquared() ); }
	Vec3 normalized() const
	{
		float len = length();
		return len > 0.0f ? *this / len : Vec3();
	}
	constexpr void copyTo( float *out ) const { out[0] = v[0]; out[1] = v[1]; out[2] = v[2]; }
};

constexpr Vec3 operator*( float s, const Vec3& a ) { return a * s; }
constexpr Vec3 lerp( const Vec3& a, const Vec3& b, float t ) { return a + ( b - a ) * t; }

static_assert( sizeof( Vec3 ) == sizeof( vec3_t ), "Vec3 must have the layout of vec3_t" );

template<class T, class Compare>
inline void SortArray( T *base, size_t count, Compare compare )
{
	std::sort( base, base + count, [compare]( const T& a, const T& b ) { return compare( &a, &b ) < 0; } );
}


typedef vec_t quat_t[4];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef std::numbers::pi_v<float>
#endif

inline constexpr int NUMVERTEXNORMALS = 162;
extern	vec3_t	bytedirs[NUMVERTEXNORMALS];

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
inline constexpr int SCREEN_WIDTH = 640;
inline constexpr int SCREEN_HEIGHT = 480;

#define TINYCHAR_WIDTH		(SMALLCHAR_WIDTH)
#define TINYCHAR_HEIGHT		(SMALLCHAR_HEIGHT/2)

inline constexpr int SMALLCHAR_WIDTH = 8;
inline constexpr int SMALLCHAR_HEIGHT = 16;

inline constexpr int BIGCHAR_WIDTH = 16;
inline constexpr int BIGCHAR_HEIGHT = 16;

inline constexpr int GIANTCHAR_WIDTH = 32;
inline constexpr int GIANTCHAR_HEIGHT = 48;

extern	vec4_t		colorBlack;
extern	vec4_t		colorRed;
extern	vec4_t		colorGreen;
extern	vec4_t		colorBlue;
extern	vec4_t		colorYellow;
extern	vec4_t		colorMagenta;
extern	vec4_t		colorCyan;
extern	vec4_t		colorWhite;
extern	vec4_t		colorLtGrey;
extern	vec4_t		colorMdGrey;
extern	vec4_t		colorDkGrey;

#define Q_COLOR_ESCAPE	'^'
bool Q_IsColorString(const char *p);  // ^[0-9a-zA-Z]

#define COLOR_BLACK	'0'
#define COLOR_RED	'1'
#define COLOR_GREEN	'2'
#define COLOR_YELLOW	'3'
#define COLOR_BLUE	'4'
#define COLOR_CYAN	'5'
#define COLOR_MAGENTA	'6'
#define COLOR_WHITE	'7'
#define ColorIndexForNumber(c) ((c) & 0x07)
#define ColorIndex(c) (ColorIndexForNumber((c) - '0'))

#define S_COLOR_BLACK	"^0"
#define S_COLOR_RED	"^1"
#define S_COLOR_GREEN	"^2"
#define S_COLOR_YELLOW	"^3"
#define S_COLOR_BLUE	"^4"
#define S_COLOR_CYAN	"^5"
#define S_COLOR_MAGENTA	"^6"
#define S_COLOR_WHITE	"^7"

extern vec4_t	g_color_table[8];

#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

#define DEG2RAD( a ) ( ( (a) * std::numbers::pi_v<float> ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / std::numbers::pi_v<float> )

struct cplane_s;

extern	vec3_t	vec3_origin;
extern	vec3_t	axisDefault[3];

inline constexpr int nanmask = (255<<23);

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

int Q_isnan(float x);

#if idx64
  extern "C" long qftolsse(float f);
  extern "C" void qsnapvectorsse(vec3_t vec);

  #define Q_ftol qftolsse
  #define Q_SnapVector qsnapvectorsse
#elif id386
  extern "C" long QDECL qftolx87(float f);
  extern "C" long QDECL qftolsse(float f);
  extern "C" void QDECL qsnapvectorx87(vec3_t vec);
  extern "C" void QDECL qsnapvectorsse(vec3_t vec);

  extern long (QDECL *Q_ftol)(float f);
  extern void (QDECL *Q_SnapVector)(vec3_t vec);
#else
  // Q_ftol must expand to a function name so the pluggable renderer can take
  // its address
  #define Q_ftol lrintf
  #define Q_SnapVector(vec)\
	do\
	{\
		vec3_t *temp = (vec);\
		\
		(*temp)[0] = round((*temp)[0]);\
		(*temp)[1] = round((*temp)[1]);\
		(*temp)[2] = round((*temp)[2]);\
	} while(0)
#endif
/*
// if your system does not have lrintf() and round() you can try this block. Please also open a bug report at bugzilla.icculus.org
// or write a mail to the ioq3 mailing list.
#else
  #define Q_ftol(v) ((long) (v))
  #define Q_round(v) do { if((v) < 0) (v) -= 0.5f; else (v) += 0.5f; (v) = Q_ftol((v)); } while(0)
  #define Q_SnapVector(vec) \
	do\
	{\
		vec3_t *temp = (vec);\
		\
		Q_round((*temp)[0]);\
		Q_round((*temp)[1]);\
		Q_round((*temp)[2]);\
	} while(0)
#endif
*/

#if idppc

static ID_INLINE float Q_rsqrt( float number ) {
		float x = 0.5f * number;
                float y;
#ifdef __GNUC__            
                asm("frsqrte %0,%1" : "=f" (y) : "f" (number));
#else
		y = __frsqrte( number );
#endif
		return y * (1.5f - (x * y * y));
	}

#ifdef __GNUC__            
static ID_INLINE float Q_fabs(float x) {
    float abs_x;
    
    asm("fabs %0,%1" : "=f" (abs_x) : "f" (x));
    return abs_x;
}
#else
#define Q_fabs __fabsf
#endif

#else
float Q_fabs( float f );
float Q_rsqrt( float f );		// reciprocal square root
#endif

#define SQRTFAST( x ) ( (x) * Q_rsqrt( x ) )

signed char ClampChar( int i );
signed short ClampShort( int i );

// this isn't a real cheap function to call!
int DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );

#if	1

#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))

#else

#define DotProduct(x,y)			_DotProduct(x,y)
#define VectorSubtract(a,b,c)	_VectorSubtract(a,b,c)
#define VectorAdd(a,b,c)		_VectorAdd(a,b,c)
#define VectorCopy(a,b)			_VectorCopy(a,b)
#define	VectorScale(v, s, o)	_VectorScale(v,s,o)
#define	VectorMA(v, s, b, o)	_VectorMA(v,s,b,o)

#endif

#ifdef Q3_VM
#ifdef VectorCopy
#undef VectorCopy
// this is a little hack to get more efficient copies in our interpreter
struct vec3struct_t {
	float	v[3];
};
#define VectorCopy(a,b)	(*(vec3struct_t *)b=*(vec3struct_t *)a)
#endif
#endif

#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define Vector4Set(v, x, y, z, w)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z), (v)[3]=(w))

#define Byte4Copy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define QuatCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define	SnapVector(v) {v[0]=((int)(v[0]));v[1]=((int)(v[1]));v[2]=((int)(v[2]));}
// just in case you don't want to use the macros
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 );
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorCopy( const vec3_t in, vec3_t out );
void _VectorScale( const vec3_t in, float scale, vec3_t out );
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );

unsigned ColorBytes3 (float r, float g, float b);
unsigned ColorBytes4 (float r, float g, float b, float a);

float NormalizeColor( const vec3_t in, vec3_t out );

float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void ClearBounds( vec3_t mins, vec3_t maxs );
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );

#if !defined( Q3_VM ) || ( defined( Q3_VM ) && defined( __Q3_VM_MATH ) )
static ID_INLINE int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}			
	return 1;
}

static ID_INLINE vec_t VectorLength( const vec3_t v ) {
	return (vec_t)sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static ID_INLINE vec_t VectorLengthSquared( const vec3_t v ) {
	return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static ID_INLINE vec_t Distance( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return VectorLength( v );
}

static ID_INLINE vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length, uses rsqrt approximation
static ID_INLINE void VectorNormalizeFast( vec3_t v )
{
	float ilength;

	ilength = Q_rsqrt( DotProduct( v, v ) );

	v[0] *= ilength;
	v[1] *= ilength;
	v[2] *= ilength;
}

static ID_INLINE void VectorInverse( vec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

static ID_INLINE void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

#else
int VectorCompare( const vec3_t v1, const vec3_t v2 );

vec_t VectorLength( const vec3_t v );

vec_t VectorLengthSquared( const vec3_t v );

vec_t Distance( const vec3_t p1, const vec3_t p2 );

vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 );

void VectorNormalizeFast( vec3_t v );

void VectorInverse( vec3_t v );

void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );

#endif

vec_t VectorNormalize (vec3_t v);		// returns vector length
vec_t VectorNormalize2( const vec3_t v, vec3_t out );
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out );
int Q_log2(int val);

float Q_acos(float c);

int		Q_rand( int *seed );
float	Q_random( int *seed );
float	Q_crandom( int *seed );

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define crandom()	(2.0 * (random() - 0.5))

void vectoangles( const vec3_t value1, vec3_t angles);
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );
void AxisToAngles( vec3_t axis[3], vec3_t angles );
void AnglesToQuat( const vec3_t angles, quat_t q );
void QuatToAngles( const quat_t q, vec3_t angles );
void QuatInterpolate( const quat_t from, const quat_t to, float frac, quat_t out );

void AxisClear( vec3_t axis[3] );
void AxisCopy( vec3_t in[3], vec3_t out[3] );

void SetPlaneSignbits( struct cplane_s *out );
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);

bool BoundsIntersect(const vec3_t mins, const vec3_t maxs,
		const vec3_t mins2, const vec3_t maxs2);
bool BoundsIntersectSphere(const vec3_t mins, const vec3_t maxs,
		const vec3_t origin, vec_t radius);
bool BoundsIntersectPoint(const vec3_t mins, const vec3_t maxs,
		const vec3_t origin);

float	AngleMod(float a);
float	LerpAngle (float from, float to, float frac);
float	AngleSubtract( float a1, float a2 );
void	AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 );

float AngleNormalize360 ( float angle );
float AngleNormalize180 ( float angle );
float AngleDelta ( float angle1, float angle2 );

bool PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
void RotateAroundDirection( vec3_t axis[3], float yaw );
void RotateAroundYaw( vec3_t axis[3], float angle );//MFQ3
void RotateAroundPitch( vec3_t axis[3], float angle );// MFQ3
void RotatePointAroundAngleVector( vec3_t dst, vec3_t src, vec3_t angles );// MFQ3
void RotatePointAroundAngles( vec3_t dst, vec3_t src, float yaw, float pitch, float roll );// MFQ3
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );
// perpendicular vector could be replaced by this

//int	PlaneTypeForNormal (vec3_t normal);

void MatrixMultiply(float in1[3][3], float in2[3][3], float out[3][3]);
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up);
void PerpendicularVector( vec3_t dst, const vec3_t src );


// MFQ3
inline constexpr int BOX3_MAX = 2;
inline constexpr int BOX3_MIN = 4;

// complex box for intersection method
struct box3_t {
	vec3_t		edgePoints[8];	// 8 edge-points
	
	vec3_t		center;		// center of the box
	vec3_t		axis[3];		// columns are the x,y,z axis
	vec3_t		extents;		// extents along axis	
};

void MakeBoxFromExtents( box3_t* box, const vec3_t mins, const vec3_t maxs, const vec3_t angles );

// simple box
struct sbox3_t {
    vec3_t		mins;
	vec3_t		maxs;
};

// expand box to contain this point
void AddToBox( sbox3_t* box, const vec3_t point );
void BoxCenter( sbox3_t* box, vec3_t* center );

struct ray3_t {
	vec3_t		start;
	vec3_t		dir;
};

void MakeRay( ray3_t* ray, const vec3_t start, const vec3_t dir );

bool RayIntersectBox( const ray3_t* ray, const box3_t* bbox );

// end MFQ3

//=============================================

float Com_Clamp( float min, float max, float value );

char	*COM_SkipPath( char *pathname );
const char	*COM_GetExtension( const char *name );
void	COM_StripExtension(const char *in, char *out, int destsize);
bool COM_CompareExtension(const char *in, const char *ext);
void	COM_DefaultExtension( char *path, int maxSize, const char *extension );

void	COM_BeginParseSession( const char *name );
int		COM_GetCurrentParseLine( );
char	*COM_Parse( char **data_p );
char	*COM_ParseExt( char **data_p, bool allowLineBreak );
int		COM_Compress( char *data_p );
void	COM_ParseError( char *format, ... ) Q_PRINTF_FUNC(1, 2);
void	COM_ParseWarning( char *format, ... ) Q_PRINTF_FUNC(1, 2);
//int		COM_ParseInfos( char *buf, int max, char infos[][MAX_INFO_STRING] );

inline constexpr int MAX_TOKENLENGTH = 1024;


// data is an in/out parm, returns a parsed out token

void	COM_MatchToken( char**buf_p, char *match );

bool SkipBracedSection (char **program, int depth);
void SkipRestOfLine ( char **data );

void Parse1DMatrix (char **buf_p, int x, float *m);
void Parse2DMatrix (char **buf_p, int y, int x, float *m);
void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m);
int Com_HexStrToInt( const char *str );

int QDECL Com_sprintf (char *dest, int size, const char *fmt, ...) Q_PRINTF_FUNC(3, 4);

char *Com_SkipTokens( char *s, int numTokens, char *sep );
char *Com_SkipCharset( char *s, char *sep );

void Com_RandomBytes( byte *string, int len );

// mode parm for FS_FOpenFile
enum fsMode_t {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC
};

enum fsOrigin_t {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
};

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );
bool Q_isanumber( const char *s );
bool Q_isintegral( float f );

// portable case insensitive compare
int		Q_stricmp (const char *s1, const char *s2);
int		Q_strncmp (const char *s1, const char *s2, int n);
int		Q_stricmpn (const char *s1, const char *s2, int n);
char	*Q_strlwr( char *s1 );
char	*Q_strupr( char *s1 );
const char	*Q_stristr( const char *s, const char *find);

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, int destsize );
void	Q_strcat( char *dest, int size, const char *src );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );
// removes color sequences from string
char *Q_CleanStr( char *string );
// Count the number of char tocount encountered in string
int Q_CountChar(const char *string, char tocount);

//=============================================

// 64-bit integers for global rankings interface
// implemented as a struct for qvm compatibility
struct qint64
{
	byte	b0;
	byte	b1;
	byte	b2;
	byte	b3;
	byte	b4;
	byte	b5;
	byte	b6;
	byte	b7;
};

//=============================================
/*
short	BigShort(short l);
short	LittleShort(short l);
int		BigLong (int l);
int		LittleLong (int l);
qint64  BigLong64 (qint64 l);
qint64  LittleLong64 (qint64 l);
float	BigFloat (const float *l);
float	LittleFloat (const float *l);

void	Swap_Init (void);
*/
char	* QDECL va(const char *format, ...) Q_PRINTF_FUNC(1, 2);

inline constexpr int TRUNCATE_LENGTH = 64;
void Com_TruncateLongString( char *buffer, const char *s );

//=============================================

//
// key / value info strings
//
class Info
{
public:
	static char *valueForKey( const char *s, const char *key );
	static void removeKey( char *s, const char *key );
	static void removeKey_Big( char *s, const char *key );
	static void setValueForKey( char *s, const char *key, const char *value );
	static void setValueForKey_Big( char *s, const char *key, const char *value );
	static bool validate( const char *s );
	static void nextPair( const char **s, char *key, char *value );
};

// this is only here so the functions in q_shared.c and bg_*.c can link


/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

inline constexpr int CVAR_ARCHIVE = 0x0001; // set to cause it to be saved to vars.rc
					// used for system variables, not for player
					// specific configurations
inline constexpr int CVAR_USERINFO = 0x0002; // sent to server on connect or change
inline constexpr int CVAR_SERVERINFO = 0x0004; // sent in response to front end requests
inline constexpr int CVAR_SYSTEMINFO = 0x0008; // these cvars will be duplicated on all clients
inline constexpr int CVAR_INIT = 0x0010; // don't allow change from console at all,
					// but can be set from the command line
inline constexpr int CVAR_LATCH = 0x0020; // will only change when C code next does
					// a CvarSystem::get(), so it can't be changed
					// without proper initialization.  modified
					// will be set, even though the value hasn't
					// changed yet
inline constexpr int CVAR_ROM = 0x0040; // display only, cannot be set by user at all
inline constexpr int CVAR_USER_CREATED = 0x0080; // created by a set command
inline constexpr int CVAR_TEMP = 0x0100; // can be set even when cheats are disabled, but is not archived
inline constexpr int CVAR_CHEAT = 0x0200; // can not be changed if cheats are disabled
inline constexpr int CVAR_NORESTART = 0x0400; // do not clear when a cvar_restart is issued

inline constexpr int CVAR_SERVER_CREATED = 0x0800; // cvar was created by a server the client connected to.
inline constexpr int CVAR_VM_CREATED = 0x1000; // cvar was created exclusively in one of the VMs.
inline constexpr int CVAR_PROTECTED = 0x2000; // prevent modifying this var from VMs or the server
// These flags are only returned by the CvarSystem::flags() function
inline constexpr int CVAR_MODIFIED = 0x40000000; // Cvar was modified
inline constexpr unsigned int CVAR_NONEXISTENT = 0x80000000; // Cvar doesn't exist.

// nothing outside the Cvar_*() functions should modify these fields!
using cvar_t = struct cvar_s;

struct cvar_s {
	char			*name;
	char			*string;
	char			*resetString;		// cvar_restart will reset to this value
	char			*latchedString;		// for CVAR_LATCH vars
	int				flags;
	bool	modified;			// set each time the cvar is changed
	int				modificationCount;	// incremented each time the cvar is changed
	float			value;				// atof( string )
	int				integer;			// atoi( string )
	bool	validate;
	bool	integral;
	float			min;
	float			max;
	char			*description;

	cvar_t *next;
	cvar_t *prev;
	cvar_t *hashNext;
	cvar_t *hashPrev;
	int			hashIndex;
};

inline constexpr int MAX_CVAR_VALUE_STRING = 256;

typedef int	cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
struct vmCvar_t {
	cvarHandle_t	handle;
	int			modificationCount;
	float		value;
	int			integer;
	char		string[MAX_CVAR_VALUE_STRING];
};

#include "../qcommon/cvar_system.h"


/*
==============================================================

VoIP

==============================================================
*/

// if you change the count of flags be sure to also change VOIP_FLAGNUM
inline constexpr int VOIP_SPATIAL = 0x01; // spatialized voip message
inline constexpr int VOIP_DIRECT = 0x02; // non-spatialized voip message

// number of flags voip knows. You will have to bump protocol version number if you
// change this.
inline constexpr int VOIP_FLAGCNT = 2;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "surfaceflags.h"			// shared with the q3map utility

// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
inline constexpr int PLANE_NON_AXIAL = 3;


/*
=================
PlaneTypeForNormal
=================
*/

#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
struct cplane_s {
	vec3_t	normal;
	float	dist;
	byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
	byte	pad[2];
};
using cplane_t = cplane_s;


// a trace is returned when a box is swept through the world
struct trace_t {
	bool	allsolid;	// if true, plane is not valid
	bool	startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	cplane_t	plane;		// surface normal at impact, transformed to world space
	int			surfaceFlags;	// surface hit
	int			contents;	// contents on other side of surface hit
	int			entityNum;	// entity the contacted sirface is a part of
};

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD


// markfragments are returned by R_MarkFragments()
struct markFragment_t {
	int		firstPoint;
	int		numPoints;
};



struct orientation_t {
	vec3_t		origin;
	vec3_t		axis[3];
};

//=====================================================================


// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
inline constexpr int KEYCATCH_CONSOLE = 0x0001;
inline constexpr int KEYCATCH_UI = 0x0002;
inline constexpr int KEYCATCH_MESSAGE = 0x0004;
inline constexpr int KEYCATCH_CGAME = 0x0008;


// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
enum soundChannel_t {
	CHAN_AUTO,
	CHAN_LOCAL,		// menu sounds, etc
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	CHAN_LOCAL_SOUND,	// chat messages, etc
	CHAN_ANNOUNCER		// announcer voices, etc
};


/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

inline constexpr int SNAPFLAG_RATE_DELAYED = 1;
inline constexpr int SNAPFLAG_NOT_ACTIVE = 2; // snapshot used during connection and for zombies
inline constexpr int SNAPFLAG_SERVERCOUNT = 4; // toggled every map_restart so transitions can be detected
inline constexpr int SNAPFLAG_ENTITY_RESET = 8;

//
// per-level limits
//
inline constexpr int MAX_CLIENTS = 64; // absolute limit
inline constexpr int MAX_LOCATIONS = 64;

inline constexpr int GENTITYNUM_BITS = 12;
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
inline constexpr int ENTITYNUM_NONE = (-1);
inline constexpr int ENTITYNUM_WORLD = (0);
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-1)
#define	ENTITYNUM_WIRE_END		(MAX_GENTITIES-1)


inline constexpr int MAX_MODELS = 256; // these are sent over the net as 8 bits
inline constexpr int MAX_SOUNDS = 256; // so they cannot be blindly increased


#define	MAX_CONFIGSTRINGS	1024

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
inline constexpr int CS_SERVERINFO = 0; // an info string with all the serverinfo cvars
inline constexpr int CS_SYSTEMINFO = 1; // an info string for server system to client system configuration (timescale, etc)

inline constexpr int RESERVED_CONFIGSTRINGS = 2; // game can't modify below this, only the system can

inline constexpr int MAX_GAMESTATE_CHARS = 16000;
struct gameState_t {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
};

//=========================================================

// bit field limits
inline constexpr int MAX_STATS = 16;
inline constexpr int MAX_PERSISTANT = 16;
inline constexpr int MAX_TIMERS = 16;

inline constexpr int MAX_PS_EVENTS = 2;

inline constexpr int PS_PMOVEFRAMECOUNTBITS = 6;

//MFQ3 related

// angle indexes for tanks
inline constexpr int HULL_YAW = 0;
inline constexpr int TURRET_YAW = 1;
inline constexpr int GUN_PITCH = 2;

inline constexpr int MAX_WEAPONS_PER_VEHICLE = 8;
// end mfq3


// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.
struct playerState_s {
	int			commandTime;	// cmd->serverTime of last executed command
	int			pm_type;
	int			throttle;		// MFQ3 actual throttle (ie maybe overridden fixed_throttle)
	int			pm_flags;		// ducked, jump_held, etc
	int			pm_time;

	vec3_t		origin;
	vec3_t		velocity;
	int			tracktarget;
	float		gunAngle;
	int			speed;
	int			delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

	int			groundEntityNum;// ENTITYNUM_NONE = in air

	int			objectives;		// MFQ3 for game objectives like CTF flags
	int			vehicleAnim;	// set vehicle specific animations

	float		turretAngle;	// MFQ3 used for turret
	int			ONOFF;			// ON/OFF flags

	int			fixed_throttle;	// MFQ3 for throttle settings by increase/decrease

	vec3_t		vehicleAngles;	// used for vehicles in MFQ3

	int			eFlags;			// copied to entityState_t->eFlags

	int			eventSequence;	// pmove generated events
	int			events[MAX_PS_EVENTS];
	int			eventParms[MAX_PS_EVENTS];

	int			externalEvent;	// events set on player from another source
	int			externalEventParm;
	int			externalEventTime;

	int			clientNum;		// ranges from 0 to MAX_CLIENTS-1
	int			weaponIndex;	// copied to entityState_t->weaponIndex
	int			weaponNum;

	vec3_t		viewangles;		// for fixed views
	int			viewheight;		// --- useless, can be (carefully) replaced ---

	// damage feedback
	int			damageEvent;	// when it changes, latch the other parms
	int			damageYaw;
	int			damagePitch;
	int			damageCount;

	int			stats[MAX_STATS];
	int			persistant[MAX_PERSISTANT];	// stats that aren't cleared on death
	int			timers[MAX_TIMERS];			// MFQ3 timers etc
	int			ammo[MAX_WEAPONS_PER_VEHICLE*2];// 0-7 actual ammo of weapon 8-15 maxammo of weapon

	int			generic1;
	int			loopSound;
	int			jumppad_ent;	// jumppad entity hit this frame

	// mfq3 weapons/vwep
	int			numWeaponsOnMount[32];	// number of weapons actually on this mount; note: each of these are just 5 bit numbers (ie 32)
										// 32 = MAX_MOUNTS_PER_VEHICLE 
										// shouldnt be hardcoded like this, but want to get New system to work first

	// not communicated over the net at all
	int			ping;			// server to game info for scoreboard
	int			pmove_framecount;	// FIXME: don't transmit over the network
	int			jumppad_frame;
	int			entityEventSequence;
};
using playerState_t = playerState_s;

struct PlayerState
{
	int			commandTime_;	// cmd->serverTime of last executed command
	int			pm_type_;
	int			throttle_;		// MFQ3 actual throttle (ie maybe overridden fixed_throttle)
	int			pm_flags_;		// ducked, jump_held, etc
	int			pm_time_;

	vec3_t		origin_;
	vec3_t		velocity_;
	int			tracktarget_;
	float		gunAngle_;
	int			speed_;
	int			delta_angles_[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

	int			groundEntityNum_;// ENTITYNUM_NONE = in air

	int			objectives_;		// MFQ3 for game objectives like CTF flags
	int			vehicleAnim_;	// set vehicle specific animations

	float		turretAngle_;	// MFQ3 used for turret
	int			ONOFF_;			// ON/OFF flags

	int			fixed_throttle_;	// MFQ3 for throttle settings by increase/decrease

	vec3_t		vehicleAngles_;	// used for vehicles in MFQ3

	int			eFlags_;			// copied to entityState_t->eFlags

	int			eventSequence_;	// pmove generated events
	int			events_[MAX_PS_EVENTS];
	int			eventParms_[MAX_PS_EVENTS];

	int			externalEvent_;	// events set on player from another source
	int			externalEventParm_;
	int			externalEventTime_;

	int			clientNum_;		// ranges from 0 to MAX_CLIENTS-1
	int			weaponIndex_;	// copied to entityState_t->weaponIndex
	int			weaponNum_;

	vec3_t		viewangles_;		// for fixed views
	int			viewheight_;		// --- useless, can be (carefully) replaced ---

	// damage feedback
	int			damageEvent_;	// when it changes, latch the other parms
	int			damageYaw_;
	int			damagePitch_;
	int			damageCount_;

	int			stats_[MAX_STATS];
	int			persistant_[MAX_PERSISTANT];	// stats that aren't cleared on death
	int			timers_[MAX_TIMERS];			// MFQ3 timers etc
	int			ammo_[MAX_WEAPONS_PER_VEHICLE*2];// 0-7 actual ammo of weapon 8-15 maxammo of weapon

	int			generic1_;
	int			loopSound_;
	int			jumppad_ent_;	// jumppad entity hit this frame

	// mfq3 weapons/vwep
	int			numWeaponsOnMount_[32];	// number of weapons actually on this mount; note: each of these are just 5 bit numbers (ie 32)
										// 32 = MAX_MOUNTS_PER_VEHICLE 
										// shouldnt be hardcoded like this, but want to get New system to work first

	// not communicated over the net at all
	int			ping_;			// server to game info for scoreboard
	int			pmove_framecount_;	// FIXME: don't transmit over the network
	int			jumppad_frame_;
	int			entityEventSequence_;
};

struct ClientBase
{
	enum eTeam
	{
		TEAM_FREE,
		TEAM_RED,
		TEAM_BLUE,
		TEAM_SPECTATOR,

		TEAM_NUM_TEAMS
	};

					ClientBase() 
					{
						memset( &ps_, 0, sizeof(ps_) );
					}
	virtual			~ClientBase() {}

	playerState_t	ps_;
};


//====================================================================


//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
//#define	BUTTON_ATTACK		1
//#define	BUTTON_TALK			2			// displays talk balloon and disables actions
//#define	BUTTON_USE_HOLDABLE	4
//#define	BUTTON_GESTURE		8
//#define	BUTTON_WALKING		16			// walking can't just be infered from MOVE_RUN
//										// because a key pressed late in the frame will
//										// only generate a small move value for that frame
//										// walking will use different animations and
//										// won't generate footsteps
//#define BUTTON_AFFIRMATIVE	32
//#define	BUTTON_NEGATIVE		64
//
//#define BUTTON_GETFLAG		128
//#define BUTTON_GUARDBASE	256
//#define BUTTON_PATROL		512
//#define BUTTON_FOLLOWME		1024
//
//#define	BUTTON_ANY			2048			// any key whatsoever

inline constexpr int BUTTON_ATTACK = 1;
inline constexpr int BUTTON_TALK = 2; // displays talk balloon and disables actions
inline constexpr int BUTTON_ATTACK_MAIN = 4; // MFQ3 secondary fire "+button2"
inline constexpr int BUTTON_WEAPONBAY = 8; // MFQ3 weaponbay "+button3"
inline constexpr int BUTTON_XXX2 = 16;
inline constexpr int BUTTON_INCREASE = 32; // MFQ3	throttle "+button5"
inline constexpr int BUTTON_DECREASE = 64; // MFQ3 throttle "+button6"
inline constexpr int BUTTON_GEAR = 128; // MFQ3 gear "+button7"
inline constexpr int BUTTON_BRAKE = 256; // MFQ3 speedbrakes/brakes "+button8"
inline constexpr int BUTTON_FREELOOK = 512; // MFQ3 freelook "+button9"
inline constexpr int BUTTON_FLARE = 1024; // MFQ3 flare "+button10"

inline constexpr int BUTTON_ANY = 2048; // any key whatsoever

inline constexpr int MOVE_RUN = 120; // if forwardmove or rightmove are >= MOVE_RUN,
										// then BUTTON_WALKING should be set

// usercmd_t is sent to the server each client frame
struct usercmd_s {
	int				serverTime;
	int				angles[3];
	int 			buttons;
	byte			weapon;           // weapon 
	signed char	forwardmove, rightmove, upmove;
};
using usercmd_t = usercmd_s;

//===================================================================

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
inline constexpr int SOLID_BMODEL = 0xffffff;

enum trType_t {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY,
	TR_GRAVITY_10,
	TR_ACCEL
};

struct trajectory_t {
	trType_t	trType;
	int		trTime;
	int		trDuration;			// if non 0, trTime + trDuration = stop time
	vec3_t	trBase;
	vec3_t	trDelta;			// velocity, etc
};

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

inline constexpr int MAX_VEHICLE_TURRETS = 4;

struct entityState_s {
	int		number;			// entity index
	int		eType;			// entityType_t
	int		eFlags;

	trajectory_t	pos;	// for calculating position
	trajectory_t	apos;	// for calculating angles

	int		time;
	int		tracktarget;

	vec3_t	origin;
	vec3_t	origin2;

	vec3_t	angles;
	vec3_t	angles2;

	int		otherEntityNum;	// shotgun sources, etc
	int		otherEntityNum2;

	int		groundEntityNum;	// -1 = in air

	int		ONOFF;
	int		loopSound;		// constantly loop this sound

	int		modelindex;		
	int		modelindex2;
	int		clientNum;		// 0 to (MAX_CLIENTS - 1), for players and corpses
	int		frame;			// in MFQ3 this is THROTTLE (for vehicles)

	int		solid;			// for client side prediction, linkentity sets this properly

	int		event;			// impulse events -- muzzle flashes, footsteps, etc
	int		eventParm;

	// for players
	int		objectives;		// bit flags
	int		weaponIndex;
	int		vehicleAnim;	// mask off ANIM_TOGGLEBIT
	int		weaponNum;

	int		generic1;

	int		vehicleEnt;		// vehicle this player rides, 0 = none
	int		seat;
	int		seatMask;
	int		mountMask;
	int		revealTime;
	float	turretYaw[MAX_VEHICLE_TURRETS];
	float	gunPitch[MAX_VEHICLE_TURRETS];
};
using entityState_t = entityState_s;

enum connstate_t {
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_AUTHORIZING,		// not used any more, was checking cd key 
	CA_CONNECTING,		// sending request packets to the server
	CA_CHALLENGING,		// sending challenge packets to the server
	CA_CONNECTED,		// NetChannel established, getting gamestate
	CA_LOADING,			// only during cgame initialization, never during main loop
	CA_PRIMED,			// got gamestate, waiting for first frame
	CA_ACTIVE,			// game views should be displayed
	CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
};

// font support 

inline constexpr int GLYPH_START = 0;
inline constexpr int GLYPH_END = 255;
inline constexpr int GLYPH_CHARSTART = 32;
inline constexpr int GLYPH_CHAREND = 127;
#define GLYPHS_PER_FONT GLYPH_END - GLYPH_START + 1
struct glyphInfo_t {
  int height;       // number of scan lines
  int top;          // top of glyph in buffer
  int bottom;       // bottom of glyph in buffer
  int pitch;        // width for copying
  int xSkip;        // x adjustment
  int imageWidth;   // width of actual image
  int imageHeight;  // height of actual image
  float s;          // x offset in image where glyph starts
  float t;          // y offset in image where glyph starts
  float s2;
  float t2;
  qhandle_t glyph;  // handle to the shader with the glyph
  char shaderName[32];
};

struct fontInfo_t {
  glyphInfo_t glyphs [GLYPHS_PER_FONT];
  float glyphScale;
  char name[MAX_QPATH];
};

#define Square(x) ((x)*(x))

// real time
//=============================================


struct qtime_s {
	int tm_sec;     /* seconds after the minute - [0,59] */
	int tm_min;     /* minutes after the hour - [0,59] */
	int tm_hour;    /* hours since midnight - [0,23] */
	int tm_mday;    /* day of the month - [1,31] */
	int tm_mon;     /* months since January - [0,11] */
	int tm_year;    /* years since 1900 */
	int tm_wday;    /* days since Sunday - [0,6] */
	int tm_yday;    /* days since January 1 - [0,365] */
	int tm_isdst;   /* daylight savings time flag */
};
using qtime_t = qtime_s;


// server browser sources
// TTimo: AS_MPLAYER is no longer used
inline constexpr int AS_LOCAL = 0;
inline constexpr int AS_MPLAYER = 1;
inline constexpr int AS_GLOBAL = 2;
inline constexpr int AS_FAVORITES = 3;


// cinematic states
enum e_status {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
};

enum _flag_status {
	FLAG_ATBASE = 0,
	FLAG_TAKEN,			// CTF
	FLAG_TAKEN_RED,		// One Flag CTF
	FLAG_TAKEN_BLUE,	// One Flag CTF
	FLAG_DROPPED
};
using flagStatus_t = _flag_status;



inline constexpr int MAX_GLOBAL_SERVERS = 4096;
inline constexpr int MAX_OTHER_SERVERS = 128;
inline constexpr int MAX_PINGREQUESTS = 32;
inline constexpr int MAX_SERVERSTATUSREQUESTS = 16;

inline constexpr int SAY_ALL = 0;
inline constexpr int SAY_TEAM = 1;
inline constexpr int SAY_TELL = 2;

inline constexpr int CDKEY_LEN = 16;
inline constexpr int CDCHKSUM_LEN = 2;


#define LERP( a, b, w ) ( ( a ) * ( 1.0f - ( w ) ) + ( b ) * ( w ) )
#define LUMA( red, green, blue ) ( 0.2126f * ( red ) + 0.7152f * ( green ) + 0.0722f * ( blue ) )

// generic random number functions (based upon calling Rand())
int RandomInt( int min, int max );
float RandomFloat( float min, float max );
int ahextoi( const char* string );

#include "../qcommon/command_buffer.h"

#include "../qcommon/command_system.h"

#include "../qcommon/file_system.h"

#include "../qcommon/collision_model.h"

#include "../qcommon/keys_system.h"

#include "../qcommon/sound_system.h"

#include "../qcommon/screen.h"

#include "../qcommon/system.h"

#include "../qcommon/common.h"

#include "../qcommon/zone.h"

#include "../qcommon/hunk.h"

#endif	// __Q_SHARED_H
