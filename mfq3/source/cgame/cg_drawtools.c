/*
 * $Id: cg_drawtools.c,v 1.9 2002-02-18 16:30:45 sparky909_uk Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_drawtools.c -- helper functions called by cg_draw, cg_scoreboard, cg_info, etc
#include "cg_local.h"

/*
================
CG_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void CG_AdjustFrom640( float *x, float *y, float *w, float *h ) {
#if 0
	// adjust for wide screens
	if ( cgs.glconfig.vidWidth * 480 > cgs.glconfig.vidHeight * 640 ) {
		*x += 0.5 * ( cgs.glconfig.vidWidth - ( cgs.glconfig.vidHeight * 640 / 480 ) );
	}
#endif
	// scale for screen sizes
	*x *= cgs.screenXScale;
	*y *= cgs.screenYScale;
	*w *= cgs.screenXScale;
	*h *= cgs.screenYScale;
}

/*
================
CG_FillRect

Coordinates are 640*480 virtual values
=================
*/
void CG_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, cgs.media.whiteShader );

	trap_R_SetColor( NULL );
}

/*
================
CG_DrawSides

Coords are virtual 640x480
================
*/
void CG_DrawSides(float x, float y, float w, float h, float size) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenXScale;
	trap_R_DrawStretchPic( x, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x + w - size, y, size, h, 0, 0, 0, 0, cgs.media.whiteShader );
}

void CG_DrawTopBottom(float x, float y, float w, float h, float size) {
	CG_AdjustFrom640( &x, &y, &w, &h );
	size *= cgs.screenYScale;
	trap_R_DrawStretchPic( x, y, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
	trap_R_DrawStretchPic( x, y + h - size, w, size, 0, 0, 0, 0, cgs.media.whiteShader );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawRect( float x, float y, float width, float height, float size, const float *color ) {
	trap_R_SetColor( color );

  CG_DrawTopBottom(x, y, width, height, size);
  CG_DrawSides(x, y, width, height, size);

	trap_R_SetColor( NULL );
}



/*
================
CG_DrawPic

Coordinates are 640*480 virtual values
=================
*/
void CG_DrawPic( float x, float y, float width, float height, qhandle_t hShader ) {
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}



/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
void CG_DrawChar( int x, int y, int width, int height, int ch ) {
	int row, col;
	float frow, fcol;
	float size;
	float	ax, ay, aw, ah;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	row = ch>>4;
	col = ch&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

	trap_R_DrawStretchPic( ax, ay, aw, ah,
					   fcol, frow, 
					   fcol + size, frow + size, 
					   cgs.media.charsetShader );
}


/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
void CG_DrawStringExt( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, qboolean shadow, int charWidth, int charHeight, int maxChars ) {
	vec4_t		color;
	const char	*s;
	int			xx;
	int			cnt;

	if (maxChars <= 0)
		maxChars = 32767; // do them all!

	// draw the drop shadow
	if (shadow) {
		color[0] = color[1] = color[2] = 0;
		color[3] = setColor[3];
		trap_R_SetColor( color );
		s = string;
		xx = x;
		cnt = 0;
		while ( *s && cnt < maxChars) {
			if ( Q_IsColorString( s ) ) {
				s += 2;
				continue;
			}
			CG_DrawChar( xx + 2, y + 2, charWidth, charHeight, *s );
			cnt++;
			xx += charWidth;
			s++;
		}
	}

	// draw the colored text
	s = string;
	xx = x;
	cnt = 0;
	trap_R_SetColor( setColor );
	while ( *s && cnt < maxChars) {
		if ( Q_IsColorString( s ) ) {
			if ( !forceColor ) {
				memcpy( color, g_color_table[ColorIndex(*(s+1))], sizeof( color ) );
				color[3] = setColor[3];
				trap_R_SetColor( color );
			}
			s += 2;
			continue;
		}
		CG_DrawChar( xx, y, charWidth, charHeight, *s );
		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor( NULL );
}

void CG_DrawBigString( int x, int y, const char *s, float alpha ) {
	float	color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	CG_DrawStringExt( x, y, s, color, qfalse, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
}

void CG_DrawBigStringColor( int x, int y, const char *s, vec4_t color ) {
	CG_DrawStringExt( x, y, s, color, qtrue, qtrue, BIGCHAR_WIDTH, BIGCHAR_HEIGHT, 0 );
}

void CG_DrawSmallString( int x, int y, const char *s, float alpha ) {
	float	color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;
	CG_DrawStringExt( x, y, s, color, qfalse, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
}

void CG_DrawSmallStringColor( int x, int y, const char *s, vec4_t color ) {
	CG_DrawStringExt( x, y, s, color, qtrue, qfalse, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
}

/*
=================
CG_DrawStrlen

Returns character count, skiping color escape codes
=================
*/
int CG_DrawStrlen( const char *str ) {
	const char *s = str;
	int count = 0;

	while ( *s ) {
		if ( Q_IsColorString( s ) ) {
			s += 2;
		} else {
			count++;
			s++;
		}
	}

	return count;
}

/*
=============
CG_TileClearBox

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
static void CG_TileClearBox( int x, int y, int w, int h, qhandle_t hShader ) {
	float	s1, t1, s2, t2;

	s1 = x/64.0;
	t1 = y/64.0;
	s2 = (x+w)/64.0;
	t2 = (y+h)/64.0;
	trap_R_DrawStretchPic( x, y, w, h, s1, t1, s2, t2, hShader );
}



/*
==============
CG_TileClear

Clear around a sized down screen
==============
*/
void CG_TileClear( void ) {
	int		top, bottom, left, right;
	int		w, h;

	w = cgs.glconfig.vidWidth;
	h = cgs.glconfig.vidHeight;

	if ( cg.refdef.x == 0 && cg.refdef.y == 0 && 
		cg.refdef.width == w && cg.refdef.height == h ) {
		return;		// full screen rendering
	}

	top = cg.refdef.y;
	bottom = top + cg.refdef.height-1;
	left = cg.refdef.x;
	right = left + cg.refdef.width-1;

	// clear above view screen
	CG_TileClearBox( 0, 0, w, top, cgs.media.backTileShader );

	// clear below view screen
	CG_TileClearBox( 0, bottom, w, h - bottom, cgs.media.backTileShader );

	// clear left of view screen
	CG_TileClearBox( 0, top, left, bottom - top + 1, cgs.media.backTileShader );

	// clear right of view screen
	CG_TileClearBox( right, top, w - right, bottom - top + 1, cgs.media.backTileShader );
}



/*
================
CG_FadeColor
================
*/
float *CG_FadeColor( int startMsec, int totalMsec ) {
	static vec4_t		color;
	int			t;

	if ( startMsec == 0 ) {
		return NULL;
	}

	t = cg.time - startMsec;

	if ( t >= totalMsec ) {
		return NULL;
	}

	// fade out
	if ( totalMsec - t < FADE_TIME ) {
		color[3] = ( totalMsec - t ) * 1.0/FADE_TIME;
	} else {
		color[3] = 1.0;
	}
	color[0] = color[1] = color[2] = 1;

	return color;
}


/*
================
CG_TeamColor
================
*/
float *CG_TeamColor( int team ) {
	static vec4_t	red = {1, 0.2f, 0.2f, 1};
	static vec4_t	blue = {0.2f, 0.2f, 1, 1};
	static vec4_t	other = {1, 1, 1, 1};
	static vec4_t	spectator = {0.7f, 0.7f, 0.7f, 1};

	switch ( team ) {
	case TEAM_RED:
		return red;
	case TEAM_BLUE:
		return blue;
	case TEAM_SPECTATOR:
		return spectator;
	default:
		return other;
	}
}

/*
=================
UI_DrawProportionalString2
=================
*/
static int	propMap[128][3] = {
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},
{0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1}, {0, 0, -1},

{0, 0, PROP_SPACE_WIDTH},		// SPACE
{11, 122, 7},	// !
{154, 181, 14},	// "
{55, 122, 17},	// #
{79, 122, 18},	// $
{101, 122, 23},	// %
{153, 122, 18},	// &
{9, 93, 7},		// '
{207, 122, 8},	// (
{230, 122, 9},	// )
{177, 122, 18},	// *
{30, 152, 18},	// +
{85, 181, 7},	// ,
{34, 93, 11},	// -
{110, 181, 6},	// .
{130, 152, 14},	// /

{22, 64, 17},	// 0
{41, 64, 12},	// 1
{58, 64, 17},	// 2
{78, 64, 18},	// 3
{98, 64, 19},	// 4
{120, 64, 18},	// 5
{141, 64, 18},	// 6
{204, 64, 16},	// 7
{162, 64, 17},	// 8
{182, 64, 18},	// 9
{59, 181, 7},	// :
{35,181, 7},	// ;
{203, 152, 14},	// <
{56, 93, 14},	// =
{228, 152, 14},	// >
{177, 181, 18},	// ?

{28, 122, 22},	// @
{5, 4, 18},		// A
{27, 4, 18},	// B
{48, 4, 18},	// C
{69, 4, 17},	// D
{90, 4, 13},	// E
{106, 4, 13},	// F
{121, 4, 18},	// G
{143, 4, 17},	// H
{164, 4, 8},	// I
{175, 4, 16},	// J
{195, 4, 18},	// K
{216, 4, 12},	// L
{230, 4, 23},	// M
{6, 34, 18},	// N
{27, 34, 18},	// O

{48, 34, 18},	// P
{68, 34, 18},	// Q
{90, 34, 17},	// R
{110, 34, 18},	// S
{130, 34, 14},	// T
{146, 34, 18},	// U
{166, 34, 19},	// V
{185, 34, 29},	// W
{215, 34, 18},	// X
{234, 34, 18},	// Y
{5, 64, 14},	// Z
{60, 152, 7},	// [
{106, 151, 13},	// '\'
{83, 152, 7},	// ]
{128, 122, 17},	// ^
{4, 152, 21},	// _

{134, 181, 5},	// '
{5, 4, 18},		// A
{27, 4, 18},	// B
{48, 4, 18},	// C
{69, 4, 17},	// D
{90, 4, 13},	// E
{106, 4, 13},	// F
{121, 4, 18},	// G
{143, 4, 17},	// H
{164, 4, 8},	// I
{175, 4, 16},	// J
{195, 4, 18},	// K
{216, 4, 12},	// L
{230, 4, 23},	// M
{6, 34, 18},	// N
{27, 34, 18},	// O

{48, 34, 18},	// P
{68, 34, 18},	// Q
{90, 34, 17},	// R
{110, 34, 18},	// S
{130, 34, 14},	// T
{146, 34, 18},	// U
{166, 34, 19},	// V
{185, 34, 29},	// W
{215, 34, 18},	// X
{234, 34, 18},	// Y
{5, 64, 14},	// Z
{153, 152, 13},	// {
{11, 181, 5},	// |
{180, 152, 13},	// }
{79, 93, 17},	// ~
{0, 0, -1}		// DEL
};

static int propMapB[26][3] = {
{11, 12, 33},
{49, 12, 31},
{85, 12, 31},
{120, 12, 30},
{156, 12, 21},
{183, 12, 21},
{207, 12, 32},

{13, 55, 30},
{49, 55, 13},
{66, 55, 29},
{101, 55, 31},
{135, 55, 21},
{158, 55, 40},
{204, 55, 32},

{12, 97, 31},
{48, 97, 31},
{82, 97, 30},
{118, 97, 30},
{153, 97, 30},
{185, 97, 25},
{213, 97, 30},

{11, 139, 32},
{42, 139, 51},
{93, 139, 32},
{126, 139, 31},
{158, 139, 25},
};

#define PROPB_GAP_WIDTH		4
#define PROPB_SPACE_WIDTH	12
#define PROPB_HEIGHT		36

/*
=================
UI_DrawBannerString
=================
*/
static void UI_DrawBannerString2( int x, int y, const char* str, vec4_t color )
{
	const char* s;
	char	ch;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	trap_R_SetColor( color );
	
	ax = x * cgs.screenXScale + cgs.screenXBias;
	ay = y * cgs.screenXScale;

	s = str;
	while ( *s )
	{
		ch = *s & 127;
		if ( ch == ' ' ) {
			ax += ((float)PROPB_SPACE_WIDTH + (float)PROPB_GAP_WIDTH)* cgs.screenXScale;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			ch -= 'A';
			fcol = (float)propMapB[ch][0] / 256.0f;
			frow = (float)propMapB[ch][1] / 256.0f;
			fwidth = (float)propMapB[ch][2] / 256.0f;
			fheight = (float)PROPB_HEIGHT / 256.0f;
			aw = (float)propMapB[ch][2] * cgs.screenXScale;
			ah = (float)PROPB_HEIGHT * cgs.screenXScale;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, cgs.media.charsetPropB );
			ax += (aw + (float)PROPB_GAP_WIDTH * cgs.screenXScale);
		}
		s++;
	}

	trap_R_SetColor( NULL );
}

void UI_DrawBannerString( int x, int y, const char* str, int style, vec4_t color ) {
	const char *	s;
	int				ch;
	int				width;
	vec4_t			drawcolor;

	// find the width of the drawn text
	s = str;
	width = 0;
	while ( *s ) {
		ch = *s;
		if ( ch == ' ' ) {
			width += PROPB_SPACE_WIDTH;
		}
		else if ( ch >= 'A' && ch <= 'Z' ) {
			width += propMapB[ch - 'A'][2] + PROPB_GAP_WIDTH;
		}
		s++;
	}
	width -= PROPB_GAP_WIDTH;

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			x -= width / 2;
			break;

		case UI_RIGHT:
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_DROPSHADOW ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawBannerString2( x+2, y+2, str, drawcolor );
	}

	UI_DrawBannerString2( x, y, str, color );
}


int UI_ProportionalStringWidth( const char* str ) {
	const char *	s;
	int				ch;
	int				charWidth;
	int				width;

	s = str;
	width = 0;
	while ( *s ) {
		ch = *s & 127;
		charWidth = propMap[ch][2];
		if ( charWidth != -1 ) {
			width += charWidth;
			width += PROP_GAP_WIDTH;
		}
		s++;
	}

	width -= PROP_GAP_WIDTH;
	return width;
}

static void UI_DrawProportionalString2( int x, int y, const char* str, vec4_t color, float sizeScale, qhandle_t charset )
{
	const char* s;
	char	ch;
	float	ax;
	float	ay;
	float	aw;
	float	ah;
	float	frow;
	float	fcol;
	float	fwidth;
	float	fheight;

	// draw the colored text
	trap_R_SetColor( color );
	
	ax = x * cgs.screenXScale + cgs.screenXBias;
	ay = y * cgs.screenXScale;

	s = str;
	while ( *s )
	{
		ch = *s & 127;
		if ( ch == ' ' ) {
			aw = (float)PROP_SPACE_WIDTH * cgs.screenXScale * sizeScale;
		} else if ( propMap[ch][2] != -1 ) {
			fcol = (float)propMap[ch][0] / 256.0f;
			frow = (float)propMap[ch][1] / 256.0f;
			fwidth = (float)propMap[ch][2] / 256.0f;
			fheight = (float)PROP_HEIGHT / 256.0f;
			aw = (float)propMap[ch][2] * cgs.screenXScale * sizeScale;
			ah = (float)PROP_HEIGHT * cgs.screenXScale * sizeScale;
			trap_R_DrawStretchPic( ax, ay, aw, ah, fcol, frow, fcol+fwidth, frow+fheight, charset );
		} else {
			aw = 0;
		}

		ax += (aw + (float)PROP_GAP_WIDTH * cgs.screenXScale * sizeScale);
		s++;
	}

	trap_R_SetColor( NULL );
}

/*
=================
UI_ProportionalSizeScale
=================
*/
float UI_ProportionalSizeScale( int style ) {
	if(  style & UI_SMALLFONT ) {
		return 0.75;
	}

	return 1.00;
}


/*
=================
UI_DrawProportionalString
=================
*/
void UI_DrawProportionalString( int x, int y, const char* str, int style, vec4_t color ) {
	vec4_t	drawcolor;
	int		width;
	float	sizeScale;

	sizeScale = UI_ProportionalSizeScale( style );

	switch( style & UI_FORMATMASK ) {
		case UI_CENTER:
			width = UI_ProportionalStringWidth( str ) * sizeScale;
			x -= width / 2;
			break;

		case UI_RIGHT:
			width = UI_ProportionalStringWidth( str ) * sizeScale;
			x -= width;
			break;

		case UI_LEFT:
		default:
			break;
	}

	if ( style & UI_DROPSHADOW ) {
		drawcolor[0] = drawcolor[1] = drawcolor[2] = 0;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x+2, y+2, str, drawcolor, sizeScale, cgs.media.charsetProp );
	}

	if ( style & UI_INVERSE ) {
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, cgs.media.charsetProp );
		return;
	}

	if ( style & UI_PULSE ) {
		drawcolor[0] = color[0] * 0.8;
		drawcolor[1] = color[1] * 0.8;
		drawcolor[2] = color[2] * 0.8;
		drawcolor[3] = color[3];
		UI_DrawProportionalString2( x, y, str, color, sizeScale, cgs.media.charsetProp );

		drawcolor[0] = color[0];
		drawcolor[1] = color[1];
		drawcolor[2] = color[2];
		drawcolor[3] = 0.5 + 0.5 * sin( cg.time / PULSE_DIVISOR );
		UI_DrawProportionalString2( x, y, str, drawcolor, sizeScale, cgs.media.charsetPropGlow );
		return;
	}

	UI_DrawProportionalString2( x, y, str, color, sizeScale, cgs.media.charsetProp );
}

/*
===============
CG_GenericShadow

Uses polygons to alpha blend a shadow texture onto the terrain
===============
*/

int CG_GetVehicleShadowShader( int vehicle )
{
	int			shaderIndex = -1;
	qhandle_t	shaderHandle = 0;

	// no shadow shader specfied?
	if( availableVehicles[ vehicle ].shadowShader == SHADOW_NONE )
	{
		return qtrue;
	}
	
	// custom shader specfied?
	if( availableVehicles[ vehicle ].shadowShader != SHADOW_DEFAULT )
	{
		// use the handle of the custom shader
		shaderHandle = availableVehicles[ vehicle ].shadowShader;
	}
	else
	{
		// use the generic shadow for the vehicle catagory
		shaderIndex = MF_ExtractEnumFromId( vehicle, CAT_ANY ) - 1;
		if( shaderIndex >= 0 && shaderIndex < MF_MAX_CATEGORIES )
		{
			shaderHandle = cgs.media.shadowMarkShader[ shaderIndex ];
		}
	}

	return shaderHandle;
}

/*
===============
CG_MarkGeneratedShadow

Uses the wall-marks system of Q3 to draw shadows
===============
*/
qboolean CG_MarkGeneratedShadow( centity_t *cent, clientInfo_t * ci, float *shadowPlane )
{
	vec3_t		start, end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t		trace;
	float		alpha;
	float		xRad, yRad;
	float		xAlter, yAlter;
	qhandle_t	shaderHandle = -1;

	*shadowPlane = 0;

	// send a trace down from the player to the ground
	VectorCopy( cent->lerpOrigin, start );
	VectorCopy( cent->lerpOrigin, end );
	start[2] += 64;	// move start-point's Z upwards a little
	end[2] -= 256;	// move end-point's Z downwards somewhat
	trap_CM_BoxTrace( &trace, start, end, mins, maxs, 0, MASK_PLAYERSOLID );

	// no shadow if too high from the ground
	if( trace.fraction == 1.0 )
	{
		return qfalse;
	}

	// set the shadow plane value
	*shadowPlane = trace.endpos[2] + 1;

	// fade the shadow out with height
	alpha = 1.0f - trace.fraction;
	MF_LimitFloat( &alpha, 0.0f, 1.0f );

	// get the shader handle
	shaderHandle = CG_GetVehicleShadowShader( ci->vehicle );
	if( !shaderHandle )
	{
		return qtrue;
	}

	// assign the x/y radius values based on the vehicle bounding box
	// NOTE: we might need to setup dedicated values in availableVehicles[] for this later
	xRad = availableVehicles[ ci->vehicle ].maxs[ 0 ];
	yRad = availableVehicles[ ci->vehicle ].maxs[ 1 ];

	// PLANES ONLY: alter the radius values based upon the vehicles pitch & roll
	if( (availableVehicles[ ci->vehicle ].cat & CAT_PLANE) || 
		(availableVehicles[ ci->vehicle ].cat & CAT_HELO) ) {
		xAlter = fabs( cent->lerpAngles[0] / 30.0f );
		yAlter = fabs( cent->lerpAngles[2] / 90.0f );
		MF_LimitFloat( &xAlter, 0.0f, 1.0f );
		MF_LimitFloat( &yAlter, 0.0f, 1.0f );
		xRad *= 1.0f - ( 0.9f * xAlter );
		yRad *= 1.0f - ( 0.5f * yAlter );
	}

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMarkEx( shaderHandle, trace.endpos, trace.plane.normal, 
					 (cent->currentState.angles[1]-180), alpha, alpha, alpha, 1, qfalse, xRad, yRad, qtrue );

	return qtrue;
}
	
/*
===============
CG_PolyMeshGeneratedShadow

Uses a X/Y axis of a polygon-mesh to draw shadows
===============
*/

qboolean CG_PolyMeshGeneratedShadow( centity_t *cent, clientInfo_t * ci, float *shadowPlane )
{
	int x, y, iMod;
	float xRad, yRad;
	float xAlter, yAlter;
	float u, v, mod;
	int midPoint = 3;
	polyVert_t verts[5][5];
	polyVert_t vertBuff[4];
	qhandle_t shaderHandle = -1;
	vec3_t start, end, mins = {-1, -1, 0}, maxs = {1, 1, 0}, up = {0,0,1}, tmpVec;
	trace_t	trace;

#pragma message ("CG_PolyMeshGeneratedShadow() - fix problem when projecting over high buildings")
#pragma message ("CG_PolyMeshGeneratedShadow() - fix problem when projecting over water")
#pragma message ("CG_PolyMeshGeneratedShadow() - fix problem with bounding box (use custom coords)")

	// assign the x/y radius values based on the vehicle bounding box
	// NOTE: we might need to setup dedicated values in availableVehicles[] for this later.  also if min/max
	// boxes are quite different then we may need to allow for the offset nature of the vehicle's origin
	xRad = availableVehicles[ ci->vehicle ].maxs[ 0 ];
	yRad = availableVehicles[ ci->vehicle ].maxs[ 1 ];
	xRad -= availableVehicles[ ci->vehicle ].mins[ 0 ];
	yRad -= availableVehicles[ ci->vehicle ].mins[ 1 ];
	xRad *= 0.5f;
	yRad *= 0.5f;

	// PLANES ONLY: alter the radius values based upon the vehicles pitch & roll
	if( (availableVehicles[ ci->vehicle ].cat & CAT_PLANE) || 
		(availableVehicles[ ci->vehicle ].cat & CAT_HELO) ) {
		xAlter = fabs( cent->lerpAngles[PITCH] / 30.0f );
		yAlter = fabs( cent->lerpAngles[ROLL] / 90.0f );
		MF_LimitFloat( &xAlter, 0.0f, 1.0f );
		MF_LimitFloat( &yAlter, 0.0f, 1.0f );
		xRad *= 1.0f - ( 0.8f * xAlter );
		yRad *= 1.0f - ( 0.8f * yAlter );
	}

	// get the shader handle
	shaderHandle = CG_GetVehicleShadowShader( ci->vehicle );
	if( !shaderHandle )
	{
		return qtrue;
	}

	// create the mesh
	memset( verts, 0, sizeof( verts ) );

	// create the verts
	for( y = 0; y<5; y++ )
	{
		for( x = 0; x<5; x++ )
		{
			u = (float)x/4;
			v = (float)y/4;

			// coords

			// calc the x/y first
			verts[y][x].xyz[ 0 ] = ((xRad * 2.0f * u) - xRad);
			verts[y][x].xyz[ 1 ] = ((yRad * 2.0f * v) - yRad);
			verts[y][x].xyz[ 2 ] = 0;

			// rotate the grid around the required yaw (local origin)
			RotatePointAroundVector( tmpVec, up, verts[y][x].xyz, (cent->lerpAngles[YAW]-180) );
			VectorCopy( tmpVec, verts[y][x].xyz );

			// convert to global origin
			verts[y][x].xyz[ 0 ] += cent->lerpOrigin[ 0 ];
			verts[y][x].xyz[ 1 ] += cent->lerpOrigin[ 1 ];

			// trace a ray down from the x/y to get the z
			VectorCopy( verts[y][x].xyz, start );
			VectorCopy( verts[y][x].xyz, end );
			start[2] = cent->lerpOrigin[2] + 64;
			end[2] = cent->lerpOrigin[2] - 256;
			CG_Trace( &trace, start, mins, maxs, end, 0, MASK_PLAYERSOLID );

			// do z
			verts[y][x].xyz[ 2 ] = trace.endpos[ 2 ];

			// texture (NOTE: u & v flipped, because that's how it works fine with)
			verts[y][x].st[ 0 ] = v;
			verts[y][x].st[ 1 ] = u;

			// calc mod
			mod = 1.0f - trace.fraction;
			MF_LimitFloat( &mod, 0.0f, 1.0f );
			iMod = mod * 255;

			// modulate (black <-> white)
			verts[y][x].modulate[0] = iMod;
			verts[y][x].modulate[1] = iMod;
			verts[y][x].modulate[2] = iMod;
			verts[y][x].modulate[3] = iMod;
		}
	}

	// adjust the verts X/Y position here
	// ...

	// add the polys
	for( y = 0; y<4; y++ )
	{
		for( x = 0; x<4; x++ )
		{
			// copy vertex data into our temporary verts buffer
			memcpy( &vertBuff[3], &verts[y][x], sizeof( polyVert_t ) );
			memcpy( &vertBuff[2], &verts[y][x+1], sizeof( polyVert_t ) );
			memcpy( &vertBuff[1], &verts[y+1][x+1], sizeof( polyVert_t ) );
			memcpy( &vertBuff[0], &verts[y+1][x], sizeof( polyVert_t ) );
	
			// add the current temporary verts as a polygon
			trap_R_AddPolyToScene( shaderHandle, 4, vertBuff );
		}
	}

	return qtrue;
}


/*
===============
CG_GenericShadow

Uses polygons to alpha blend a shadow texture onto the terrain
===============
*/

#define _POLYMESH_SHADOW

qboolean CG_GenericShadow( centity_t *cent, float *shadowPlane )
{
	clientInfo_t * ci = NULL;

	// get client information
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];

#ifndef _POLYMESH_SHADOW
	return CG_MarkGeneratedShadow( cent, ci, shadowPlane );
#else
	return CG_PolyMeshGeneratedShadow( cent, ci, shadowPlane );
#endif
}

/*
===============
CG_ResetReticles

Calls to reset reticle handling
===============
*/

void CG_ResetReticles( void )
{
	// reset when starting main scene
	if( !cg.drawingMFD )
	{
		cg.reticleIdx = 0;
	}
}

/*
===============
CG_AddReticleEntityToScene

Adds a reticle, either as a 2D sprite or as pending HUD gfx
===============
*/

#define _DRAW_AS_HUD

#pragma message ("CG_AddReticleEntityToScene() FRIEND tracking/lock NOT implemented")

void CG_AddReticleEntityToScene( refEntity_t * pReticle, qboolean targetRecticle )
{
	float offsetx, offsety, scaling = 1.0f;

	// don't draw if at max already
	if( cg.reticleIdx >= MAX_RETICLES )
	{
		return;
	}

	// setup the reticle so it gets displayed as a sprite

	// most reticles are 32x32
	cg.HUDReticle[cg.reticleIdx].w = 32;
	cg.HUDReticle[cg.reticleIdx].h = 32;
	offsetx = 16;
	offsety = 16;

	// convert the CH_x define in rectile.customShader into the required shader
	switch( pReticle->customShader )
	{
	case CH_GUNMODE:
	case CH_ROCKETMODE:
			pReticle->customShader = cgs.media.HUDreticles[ HR_UNGUIDED_AND_GUN ];
			break;
	case CH_BOMBMODE:
			pReticle->customShader = cgs.media.HUDreticles[ HR_BOMB ];
			
			// bomb mode is 32x128
			cg.HUDReticle[cg.reticleIdx].h = 128;
			break;
	case CH_MISSILEMODE:
			pReticle->customShader = cgs.media.HUDreticles[ HR_GUIDED ];
			break;
	case CH_MISSILEMODETRACK:
			pReticle->customShader = cgs.media.HUDreticles[ HR_TRACKING_ENEMY ];
			break;
	case CH_MISSILEMODELOCK:
			pReticle->customShader = cgs.media.HUDreticles[ HR_LOCKED_ENEMY ];
			break;
	}

	// override if target reticle
	if( targetRecticle )
	{
		pReticle->customShader = cgs.media.HUDreticles[ HR_TARGET_ENEMY ];
		
		// force size
		cg.HUDReticle[cg.reticleIdx].w = 32;
		cg.HUDReticle[cg.reticleIdx].h = 32;
	}

	// scale is based on cg_crosshairSize
	switch( cg_crosshairSize.integer )
	{
	// tiny
	case 0:
		// half normal size
		scaling = 0.5f;
		break;

	// small
	default:
	case 1:
		// 0.75x normal size
		scaling = 0.75f;
		break;

	// medium
	case 2:
		// 1:1 normal size
		break;

	// large
	case 3:
		// 1.5x normal size
		scaling = 1.5f;
		break;

	// huge
	case 4:
		// twice normal size
		scaling = 2.0f;
		break;
	}

	// apply scaling
	cg.HUDReticle[cg.reticleIdx].w *= scaling;
	cg.HUDReticle[cg.reticleIdx].h *= scaling;
	offsetx *= scaling;
	offsety *= scaling;

	// scene object attributes
	pReticle->reType = RT_SPRITE;
	pReticle->radius = 6;
	pReticle->renderfx |= RF_FIRST_PERSON;

	// colour as TGA specified
	CreateColourChar( 255, 255, 255, 255, &pReticle->shaderRGBA[0] );

	// don't add reticles when not drawing main view
	if( !cg.drawingMFD )
	{
		// if added to the scene and drawn as a RT_SPRITE
#ifndef _DRAW_AS_HUD
		trap_R_AddRefEntityToScene( pRecticle );
#else
		// if HUD image
		if( CG_WorldToScreenCoords( pReticle->origin, &cg.HUDReticle[cg.reticleIdx].x, &cg.HUDReticle[cg.reticleIdx].y, qtrue ) )
		{
			// apply centering offset
			cg.HUDReticle[cg.reticleIdx].x -= offsetx;
			cg.HUDReticle[cg.reticleIdx].y -= offsety;

			// draw
			cg.HUDReticle[cg.reticleIdx].shader = pReticle->customShader;
			
			// this was valid, move to next
			cg.reticleIdx++;
		}
		else
		{
			// no draw
			memset( &cg.HUDReticle[cg.reticleIdx], 0, sizeof( reticle_t ) );
		}
#endif
	}
}

/*
================
CG_Draw_Reticles

================
*/
void CG_Draw_Reticles( void )
{
	int i = 0;

	// get pointer to the reticles
	reticle_t * pR = &cg.HUDReticle[0];

	// for all valid
	for( i = 0; i< cg.reticleIdx; i++ )
	{
		// draw?
		if( pR[i].shader )
		{
			CG_DrawPic( pR[i].x, pR[i].y, pR[i].w, pR[i].h, pR[i].shader );
		}
	}
}

