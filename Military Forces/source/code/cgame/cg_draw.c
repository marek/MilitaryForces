/*
 * $Id: cg_draw.c,v 1.7 2005-11-20 11:21:38 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"
#include "..\ui\ui_public.h"

#include "../ui/ui_shared.h"
#include "ui_displaycontext_cg.h"
#include "ui_utils_cg.h"

// used for scoreboard
//extern displayContextDef_t cgDC;
//extern UI_DisplayContextCG cgUtils.dc_;
extern UI_UtilsCG cgUtils;

menuDef_t *menuScoreboard = NULL;
menuDef_t *menuSpectator = NULL;

/*
===============================================================================
VARS
===============================================================================
*/

int drawTeamOverlayModificationCount = -1;
int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];

/*
===============================================================================
CUSTOM CONSOLE
===============================================================================
*/

struct strConsoleLine consoleLine[ MAX_CONSOLE_LINES ];	// our custom console buffer
int consoleIdx = 0;

/*
===============================================================================
LAGOMETER
===============================================================================
*/

#define	LAG_SAMPLES		128

typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;



/*
==============
CG_Add_Console_Line
==============
*/

void CG_Add_Console_Line( char * pText )
{
	// copy the text in
	strcpy( consoleLine[ consoleIdx ].text, pText );

	// set the life up
	consoleLine[ consoleIdx ].life = 10.0f;

	// next
	consoleIdx++;
	if( consoleIdx >= MAX_CONSOLE_LINES )
	{
		// wrap around
		consoleIdx = 0;
	}
}

/*
==============
CG_Text_Width
==============
*/
int CG_Text_Width(const char *text, float scale, int limit) {
  int count,len;
	float out;
	glyphInfo_t *glyph;
	float useScale;
// FIXME: see ui_main.c, same problem
//	const unsigned char *s = text;
	const char *s = text;
	fontInfo_t *font = &cgUtils.dc_->assets_.textFont;
	if (scale <= cg_smallFont.value) {
		font = &cgUtils.dc_->assets_.smallFont;
	} else if (scale > cg_bigFont.value) {
		font = &cgUtils.dc_->assets_.bigFont;
	}
	useScale = scale * font->glyphScale;
  out = 0;
  if (text) {
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString(s) ) {
				s += 2;
				continue;
			} else {
				glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
				out += glyph->xSkip;
				s++;
				count++;
			}
    }
  }
  return out * useScale;
}

/*
==============
CG_Text_Height
==============
*/
int CG_Text_Height(const char *text, float scale, int limit) {
  int len, count;
	float max;
	glyphInfo_t *glyph;
	float useScale;
// TTimo: FIXME
//	const unsigned char *s = text;
	const char *s = text;
	fontInfo_t *font = &cgUtils.dc_->assets_.textFont;
	if (scale <= cg_smallFont.value) {
		font = &cgUtils.dc_->assets_.smallFont;
	} else if (scale > cg_bigFont.value) {
		font = &cgUtils.dc_->assets_.bigFont;
	}
	useScale = scale * font->glyphScale;
  max = 0;
  if (text) {
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			if ( Q_IsColorString(s) ) {
				s += 2;
				continue;
			} else {
				glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
	      if (max < glyph->height) {
		      max = glyph->height;
			  }
				s++;
				count++;
			}
    }
  }
  return max * useScale;
}

/*
==============
CG_Text_PaintChar
==============
*/
void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader) {
  float w, h;
  w = width * scale;
  h = height * scale;
  CG_AdjustFrom640( &x, &y, &w, &h );
  trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

/*
==============
CG_Text_Paint
==============
*/
void CG_Text_Paint(float x, float y, float scale, const vec4_t color, const char *text, float adjust, int limit, int style)
{
	int ix = 0, iy = 0;
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float useScale;
	fontInfo_t *font = &cgUtils.dc_->assets_.textFont;

	if (scale <= cg_smallFont.value) {
		font = &cgUtils.dc_->assets_.smallFont;
	} else if (scale > cg_bigFont.value) {
		font = &cgUtils.dc_->assets_.bigFont;
	}
	useScale = scale * font->glyphScale;

	if (text)
	{
// TTimo: FIXME
//		const unsigned char *s = text;
		const char *s = text;
		trap_R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
    len = strlen(text);
		if (limit > 0 && len > limit) {
			len = limit;
		}
		count = 0;
		while (s && *s && count < len) {
			glyph = &font->glyphs[(int)*s];
			
			// TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
			//int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
			//float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);

			if ( Q_IsColorString( s ) )
			{
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				trap_R_SetColor( newColor );
				s += 2;
				continue;
			}
			else
			{
				float yadj = useScale * glyph->top;

				// shadow?
				if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) {
					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					trap_R_SetColor( colorBlack );
					CG_Text_PaintChar(x + ofs, y - yadj + ofs, 
									glyph->imageWidth,
									glyph->imageHeight,
									useScale, 
									glyph->s,
									glyph->t,
									glyph->s2,
									glyph->t2,
									glyph->glyph);

					colorBlack[3] = 1.0;
					trap_R_SetColor( newColor );
				}

				// outline?
				if (style == ITEM_TEXTSTYLE_OUTLINED )
				{
//					int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;
					colorBlack[3] = newColor[3];
					trap_R_SetColor( colorBlack );

					// create the outline
					for( iy = -1; iy<2; iy++ )
					{
						for( ix = -1; ix<2; ix++ )
						{
							// don't bother for the middle, we'll do that later
							if( ix && iy )
							{
								CG_Text_PaintChar(x - ix, y - yadj - iy, 
												glyph->imageWidth,
												glyph->imageHeight,
												useScale, 
												glyph->s,
												glyph->t,
												glyph->s2,
												glyph->t2,
												glyph->glyph);
							}
						}	// ix
					}	// iy

					colorBlack[3] = 1.0;
					trap_R_SetColor( newColor );
				}

				// print the actual text
				CG_Text_PaintChar(x, y - yadj, 
								glyph->imageWidth,
								glyph->imageHeight,
								useScale, 
								glyph->s,
								glyph->t,
								glyph->s2,
								glyph->t2,
								glyph->glyph);

				// CG_DrawPic(x, y - yadj, scale * cgDC.Assets.textFont.glyphs[text[i]].imageWidth, scale * cgDC.Assets.textFont.glyphs[text[i]].imageHeight, cgDC.Assets.textFont.glyphs[text[i]].glyph);
				x += (glyph->xSkip * useScale) + adjust;
				s++;
				count++;
			}
    }
	 
	trap_R_SetColor( NULL );
	}
}

/*
==============
CG_DrawField

Draws large numbers
==============
*/
//static void CG_DrawField (int x, int y, int width, int value) {
//	char	num[16], *ptr;
//	int		l;
//	int		frame;
//
//	if ( width < 1 ) {
//		return;
//	}
//
//	// draw number string
//	if ( width > 5 ) {
//		width = 5;
//	}
//
//	switch ( width ) {
//	case 1:
//		value = value > 9 ? 9 : value;
//		value = value < 0 ? 0 : value;
//		break;
//	case 2:
//		value = value > 99 ? 99 : value;
//		value = value < -9 ? -9 : value;
//		break;
//	case 3:
//		value = value > 999 ? 999 : value;
//		value = value < -99 ? -99 : value;
//		break;
//	case 4:
//		value = value > 9999 ? 9999 : value;
//		value = value < -999 ? -999 : value;
//		break;
//	}
//
//	Com_sprintf (num, sizeof(num), "%i", value);
//	l = strlen(num);
//	if (l > width)
//		l = width;
//	x += 2 + CHAR_WIDTH*(width - l);
//
//	ptr = num;
//	while (*ptr && l)
//	{
//		if (*ptr == '-')
//			frame = STAT_MINUS;
//		else
//			frame = *ptr -'0';
//
//		CG_DrawPic( x,y, CHAR_WIDTH, CHAR_HEIGHT, cgs.media.numberShaders[frame] );
//		x += CHAR_WIDTH;
//		ptr++;
//		l--;
//	}
//}


/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) {
	refdef_t		refdef;
	refEntity_t		ent;

	if ( !cg_draw3dIcons.integer || !cg_drawIcons.integer ) {
		return;
	}

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene( &ent );
	trap_R_RenderScene( &refdef );
}

/*
================
CG_DrawFlagModel

Used for both the status bar and the scoreboard
================
*/
void CG_DrawFlagModel( float x, float y, float w, float h, int team, bool force2D ) {
	qhandle_t		cm;
	float			len;
	vec3_t			origin, angles;
	vec3_t			mins, maxs;
	qhandle_t		handle;

	if ( !force2D && cg_draw3dIcons.integer ) {

		VectorClear( angles );

		cm = cgs.media.redFlagModel;

		// offset the origin y and z to center the flag
		trap_R_ModelBounds( cm, mins, maxs );

		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );

		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 0.5 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )

		angles[YAW] = 60 * sin( cg.time / 2000.0 );;

		if( team == TEAM_RED ) {
			handle = cgs.media.redFlagModel;
		} else if( team == TEAM_BLUE ) {
			handle = cgs.media.blueFlagModel;
		} else {
			return;
		}
		CG_Draw3DModel( x, y, w, h, handle, 0, origin, angles );
	} else if ( cg_drawIcons.integer ) {
		gitem_t *item;

		if( team == TEAM_RED ) {
			item = BG_FindItemForPowerup( OB_REDFLAG );
		} else if( team == TEAM_BLUE ) {
			item = BG_FindItemForPowerup( OB_BLUEFLAG );
		} else {
			return;
		}
		if (item) {
		  CG_DrawPic( x, y, w, h, cg_items[ ITEM_INDEX(item) ].icon );
		}
	}
}

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team )
{
	vec4_t		hcolor;

	hcolor[3] = alpha;
	if ( team == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if ( team == TEAM_BLUE ) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else {
		return;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );
}



/*
===========================================================================================

  UPPER RIGHT CORNER

===========================================================================================
*/

/*
==================
CG_DrawSnapshot
==================
*/
static float CG_DrawSnapshot( float y ) {
	char		*s;
	int			w;

	s = va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, 
		cg.latestSnapshotNum, cgs.serverCommandSequence );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	cgUtils.drawStringNew( 320, 430, 0.35f, colorWhite, s, 0, 0, ITEM_TEXTSTYLE_SHADOWED, CENTRE_JUSTIFY );

	return y;
}

/*
==================
CG_DrawFPS
==================
*/

#define	FPS_FRAMES	4
#define	FPS_STROBE	50	// only show every x computes because we can't read the damn this otherwise

static float CG_DrawFPS( float y ) {
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index, strobe = 0, lFps = 0;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = Sys_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		// show this one?
		if( strobe > 0 )
		{
			// use old fps
			fps = lFps;
		}
		else
		{
			// store persistant strobe value
			lFps = fps;
		}
		strobe++;
		if( strobe >= FPS_STROBE ) strobe = 0;

		s = va( "%ifps", fps );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;


		cgUtils.drawStringNew( 635, y + 2, 0.5f, *cgUtils.createColourVector(1,1,1,1,NULL), s, 0, 0, 3, RIGHT_JUSTIFY );

	}

	return y + BIGCHAR_HEIGHT + 4;
}

static float CG_DrawParticleStats( float y ) {
	char		*s;
	int			w;
	
	s = va( "# Particles: %i", pw.numParticles );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	cgUtils.drawStringNew( 635, y + 2, 0.5f, *cgUtils.createColourVector(1,1,1,1,NULL), s, 0, 0, 3, RIGHT_JUSTIFY );
	
	return y + BIGCHAR_HEIGHT + 4;
}


/*
=================
CG_DrawTeamOverlay
=================
*/

//static float CG_DrawTeamOverlay( float y, bool right, bool upper ) {
//	int x, w, h, xx;
//	int i, j, len;
//	const char *p;
//	vec4_t		hcolor;
//	int pwidth, lwidth;
//	int plyrs;
//	char st[16];
//	clientInfo_t *ci;
//	gitem_t	*item;
//	int ret_y, count;
//
//	if ( !cg_drawTeamOverlay.integer ) {
//		return y;
//	}
//
//	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE ) {
//		return y; // Not on any team
//	}
//
//	plyrs = 0;
//
//	// max player name width
//	pwidth = 0;
//	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
//	for (i = 0; i < count; i++) {
//		ci = cgs.clientinfo + sortedTeamPlayers[i];
//		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
//			plyrs++;
//			len = CG_DrawStrlen(ci->name);
//			if (len > pwidth)
//				pwidth = len;
//		}
//	}
//
//	if (!plyrs)
//		return y;
//
//	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
//		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;
//
//	// max location name width
//	lwidth = 0;
//	for (i = 1; i < MAX_LOCATIONS; i++) {
//		p = CG_ConfigString(CS_LOCATIONS + i);
//		if (p && *p) {
//			len = CG_DrawStrlen(p);
//			if (len > lwidth)
//				lwidth = len;
//		}
//	}
//
//	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
//		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;
//
//	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;
//
//	if ( right )
//		x = 640 - w;
//	else
//		x = 0;
//
//	h = plyrs * TINYCHAR_HEIGHT;
//
//	if ( upper ) {
//		ret_y = y + h;
//	} else {
//		y -= h;
//		ret_y = y;
//	}
//
//	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
//		hcolor[0] = 1;
//		hcolor[1] = 0;
//		hcolor[2] = 0;
//		hcolor[3] = 0.33f;
//	} else { // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
//		hcolor[0] = 0;
//		hcolor[1] = 0;
//		hcolor[2] = 1;
//		hcolor[3] = 0.33f;
//	}
//	trap_R_SetColor( hcolor );
//	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
//	trap_R_SetColor( NULL );
//
//	for (i = 0; i < count; i++) {
//		ci = cgs.clientinfo + sortedTeamPlayers[i];
//		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
//
//			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;
//
//			xx = x + TINYCHAR_WIDTH;
//
//			CG_DrawStringExt( xx, y,
//				ci->name, hcolor, false, false,
//				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);
//
//			if (lwidth) {
//				p = CG_ConfigString(CS_LOCATIONS + ci->location);
//				if (!p || !*p)
//					p = "unknown";
//				len = CG_DrawStrlen(p);
//				if (len > lwidth)
//					len = lwidth;
//
////				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth + 
////					((lwidth/2 - len/2) * TINYCHAR_WIDTH);
//				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
//				CG_DrawStringExt( xx, y,
//					p, hcolor, false, false, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
//					TEAM_OVERLAY_MAXLOCATION_WIDTH);
//			}
//
//			Com_sprintf (st, sizeof(st), "%3i", ci->health);
//
//			xx = x + TINYCHAR_WIDTH * 3 + 
//				TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;
//
//			CG_DrawStringExt( xx, y,
//				st, hcolor, false, false,
//				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
//
//			// Draw powerup icons
//			if (right) {
//				xx = x;
//			} else {
//				xx = x + w - TINYCHAR_WIDTH;
//			}
//			for (j = OB_REDFLAG; j <= OB_BLUEFLAG; j<<=1) {
//				if( ci->objectives & j ) {
//
//					item = BG_FindItemForPowerup( static_cast<objective_t>(j) );
//
//					if (item) {
//						CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
//						trap_R_RegisterShader( item->icon ) );
//						if (right) {
//							xx -= TINYCHAR_WIDTH;
//						} else {
//							xx += TINYCHAR_WIDTH;
//						}
//					}
//				}
//			}
//
//			y += TINYCHAR_HEIGHT;
//		}
//	}
//
//	return ret_y;
////#endif
//}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( void ) {
	float		x, y;
	int			cmdNum;
	usercmd_t	cmd;
	const char	*s;
	int			w;

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}

	// also add text in center of screen
	s = "Connection Interrupted";
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	cgUtils.drawStringNewAlpha( 320, 100, s, 1.0f, CENTRE_JUSTIFY );

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	x = 640 - 48;
	y = 480 - 48;

	CG_DrawPic( x, y, 48, 48, trap_R_RegisterShader("gfx/2d/net.tga" ) );
}

#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

/*
==============
CG_DrawLagometer
==============
*/
static void CG_DrawLagometer( float y )
{
	int		a, x, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	// just draw the disconnect?
	if ( !cg_lagometer.integer || cgs.localServer )
	{
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
	
	// is the New HUD active?
	if( CG_NewHUDActive() ) {
		x = 80;
		y = 250;
	}
	else
	{
		// top right
		x = 640 - 48 - 12;
		y += 2;
	}

	trap_R_SetColor( NULL );
	CG_DrawPic( x, y, 48, 48, cgs.media.lagometerShader );

	ax = x;
	ay = y;
	aw = 48;
	ah = 48;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	color = -1;
	range = ah / 3;
	mid = ay + range;

	vscale = range / MAX_LAGOMETER_RANGE;

	// draw the frame interpoalte / extrapolate graph
	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.frameSamples[i];
		v *= vscale;
		if ( v > 0 ) {
			if ( color != 1 ) {
				color = 1;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
			}
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 2 ) {
				color = 2;
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
			}
			v = -v;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	// draw the snapshot latency / drop graph
	range = ah / 2;
	vscale = range / MAX_LAGOMETER_PING;

	for ( a = 0 ; a < aw ; a++ ) {
		i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
		v = lagometer.snapshotSamples[i];
		if ( v > 0 ) {
			if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
				if ( color != 5 ) {
					color = 5;	// YELLOW for rate delay
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
			} else {
				if ( color != 3 ) {
					color = 3;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
				}
			}
			v = v * vscale;
			if ( v > range ) {
				v = range;
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
		} else if ( v < 0 ) {
			if ( color != 4 ) {
				color = 4;		// RED for dropped snapshots
				trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
			}
			trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
		}
	}

	trap_R_SetColor( NULL );

	if ( cg_nopredict.integer || cg_synchronousClients.integer ) {
		CG_DrawBigString( ax, ay, "snc", 1.0 );
	}

	CG_DrawDisconnect();
}



/*
=====================
CG_NewHUDActive

Is the New HUD actually being draw?
=====================
*/

bool CG_NewHUDActive( void )
{
	// definatley not active?
	if ( cg.snap->ps.pm_type == PM_DEAD ||
		 cg.snap->ps.pm_type == PM_SPECTATOR || 
		 cg.snap->ps.pm_type == PM_FREEZE ||
		 cg.snap->ps.stats[STAT_HEALTH] <= 0 || cg.scoreBoardShowing )
	{
		return false;
	}

	// do we have a valid current vehicle index and the 'Retro' HUD is not being used?
	return( cg_vehicle.integer != -1 && !cg_oldHUD.integer );
}

/*
=====================
CG_DrawCustomConsole

=====================
*/

#define LINE_HEIGHT	14

static void CG_DrawCustomConsole( void )
{
	float alpha = 0.0f;
	int drawIdx = 0;
	int i = 0;
	int line = 0;
	int cox = 4;	// console default position (screen TL)
	int coy = 4;
	
	// alter draw TL position?
	if( CG_NewHUDActive() )
	{
		cox = 64;
		coy = 32;
	}

	// sort out the draw index
	drawIdx = consoleIdx;

	// draw our custom console text
	for( i=0; i<MAX_CONSOLE_LINES; i++ )
	{
		// line active?
		if( consoleLine[drawIdx].life && *consoleLine[drawIdx].text )
		{
			// sort out alpha
			alpha = consoleLine[drawIdx].life;
			MF_LimitFloat( &alpha, 0.0f, 1.0f );

			// use the top line's life as the negative offset
			if( line == 0 )
			{
				coy -= LINE_HEIGHT * (1.0f - alpha);
			}

			// draw line
			// NOTE: use *CG_CreateColour(1,1,1,alpha) to fade out whilst scrolling up
			cgUtils.drawStringNew( cox, coy, 0.25f, *cgUtils.createColourVector(1,1,1,alpha,NULL), 
				consoleLine[drawIdx].text, 0, 0, cg_consoleTextStyle.integer, LEFT_JUSTIFY );

			// reduce life based upon time
			consoleLine[drawIdx].life -= (0.1f * cg.frameInterpolation );

			// non-top lines can't go below 1.0f
			if( line > 0 && consoleLine[drawIdx].life < 1.0f )
			{
				// hold line
				consoleLine[drawIdx].life = 1.0f;
			}

			// top line eventually gets removed
			if( consoleLine[drawIdx].life <= 0.0f )
			{
				// reset line
				consoleLine[drawIdx].life = 0.0f;
				*consoleLine[drawIdx].text = 0;
			}

			// inc. line
			coy += LINE_HEIGHT;
			line++;
		}

		// next line
		drawIdx++;
		if( drawIdx >= MAX_CONSOLE_LINES )
		{
			// wrap
			drawIdx = 0;
		}
	}
}

/*
=====================
CG_DrawUpperLeft

=====================
*/

static void CG_DrawUpperLeft( void )
{
	rectDef_t fillRect;
	
	// faded vertical gradient
	fillRect.x = 0;
	fillRect.y = 0;
	fillRect.w = 640;
	fillRect.h = 48;

/*
	// TEST: heading gradient band
	VerticalGradient_Paint( &fillRect, *CG_CreateColour( 0, 0, 1, 0.50f ) );
*/

	// draw our custom console
	CG_DrawCustomConsole();
}


/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight( bool scoreboard )
{
	float y = 2;

// MFQ3: no room for this because of our extensive HUD!
//	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1 ) {
//		y = CG_DrawTeamOverlay( y, true, true );
//	}

	// NOT in scoreboard draw?
	if( !scoreboard )
	{
		// snapshot
		if ( cg_drawSnapshot.integer ) {
			y = CG_DrawSnapshot( y );
		}

		// framerate
		if ( cg_drawFPS.integer ) {
			y = CG_DrawFPS( y );
		}

		// Particle stats
		if ( cg_drawParticleStats.integer ) {
			y = CG_DrawParticleStats( y );
		}
	}

	// countdown timer
	if ( cg_drawTimer.integer || scoreboard ) {
		//y = CG_DrawCountdownTimer( y );
		y = cgUtils.drawCountdownTimer( y );
	}

	// elapsed timer
	if ( cg_drawTimer.integer || scoreboard ) {
		//y = CG_DrawTimer( y );
		y = cgUtils.drawTimer( y );
	}

	// the lagometer
	CG_DrawLagometer( y );
}

/*
===========================================================================================

  LOWER RIGHT CORNER

===========================================================================================
*/

/*
=================
CG_DrawScores

Draw the small two score display
=================
*/
//static float CG_DrawScores( float y ) {
//	const char	*s;
//	int			s1, s2, score;
//	int			x, w;
//	int			v;
//	vec4_t		color;
//	float		y1;
//	gitem_t		*item;
//
//	s1 = cgs.scores1;
//	s2 = cgs.scores2;
//
//	y -=  BIGCHAR_HEIGHT + 8;
//
//	y1 = y;
//
//	// draw from the right side to left
//	if ( cgs.gametype >= GT_TEAM ) {
//		x = 640;
//
//		color[0] = 0;
//		color[1] = 0;
//		color[2] = 1;
//		color[3] = 0.33f;
//		s = va( "%2i", s2 );
//		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
//		x -= w;
//		CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
//		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
//			CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
//		}
//		CG_DrawBigString( x + 4, y, s, 1.0F);
//
//		if ( cgs.gametype == GT_CTF ) {
//			// Display flag status
//			item = BG_FindItemForPowerup( OB_BLUEFLAG );
//
//			if (item) {
//				y1 = y - BIGCHAR_HEIGHT - 8;
//				if( cgs.blueflag >= 0 && cgs.blueflag <= 2 ) {
//					CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.blueFlagShader[cgs.blueflag] );
//				}
//			}
//		}
//
//		color[0] = 1;
//		color[1] = 0;
//		color[2] = 0;
//		color[3] = 0.33f;
//		s = va( "%2i", s1 );
//		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
//		x -= w;
//		CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
//		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
//			CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
//		}
//		CG_DrawBigString( x + 4, y, s, 1.0F);
//
//		if ( cgs.gametype == GT_CTF ) {
//			// Display flag status
//			item = BG_FindItemForPowerup( OB_REDFLAG );
//
//			if (item) {
//				y1 = y - BIGCHAR_HEIGHT - 8;
//				if( cgs.redflag >= 0 && cgs.redflag <= 2 ) {
//					CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.redFlagShader[cgs.redflag] );
//				}
//			}
//		}
//
//		if ( cgs.gametype >= GT_CTF ) {
//			v = cgs.capturelimit;
//		} else {
//			v = cgs.fraglimit;
//		}
//		if ( v ) {
//			s = va( "%2i", v );
//			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
//			x -= w;
//			CG_DrawBigString( x + 4, y, s, 1.0F);
//		}
//
//	} else {
//		bool	spectator;
//
//		x = 640;
//		score = cg.snap->ps.persistant[PERS_SCORE];
//		spectator = ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR );
//
//		// always show your score in the second box if not in first place
//		if ( s1 != score ) {
//			s2 = score;
//		}
//		if ( s2 != SCORE_NOT_PRESENT ) {
//			s = va( "%2i", s2 );
//			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
//			x -= w;
//			if ( !spectator && score == s2 && score != s1 ) {
//				color[0] = 1;
//				color[1] = 0;
//				color[2] = 0;
//				color[3] = 0.33f;
//				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
//				CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
//			} else {
//				color[0] = 0.5f;
//				color[1] = 0.5f;
//				color[2] = 0.5f;
//				color[3] = 0.33f;
//				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
//			}	
//			CG_DrawBigString( x + 4, y, s, 1.0F);
//		}
//
//		// first place
//		if ( s1 != SCORE_NOT_PRESENT ) {
//			s = va( "%2i", s1 );
//			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
//			x -= w;
//			if ( !spectator && score == s1 ) {
//				color[0] = 0;
//				color[1] = 0;
//				color[2] = 1;
//				color[3] = 0.33f;
//				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
//				CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
//			} else {
//				color[0] = 0.5f;
//				color[1] = 0.5f;
//				color[2] = 0.5f;
//				color[3] = 0.33f;
//				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
//			}	
//			CG_DrawBigString( x + 4, y, s, 1.0F);
//		}
//
//		if ( cgs.fraglimit ) {
//			s = va( "%2i", cgs.fraglimit );
//			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
//			x -= w;
//			CG_DrawBigString( x + 4, y, s, 1.0F);
//		}
//
//	}
//
//	return y1 - 8;
//}

/*
===================
CG_DrawPickupItem
===================
*/
//static int CG_DrawPickupItem( int y ) {
//	int		value;
//	float	*fadeColor;
//
//	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
//		return y;
//	}
//
//	y -= ICON_SIZE;
//
//	value = cg.itemPickup;
//	if ( value ) {
//		fadeColor = CG_FadeColor( cg.itemPickupTime, 3000 );
//		if ( fadeColor ) {
//			CG_RegisterItemVisuals( value );
//			trap_R_SetColor( fadeColor );
//			CG_DrawPic( 8, y, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
//			CG_DrawBigString( ICON_SIZE + 16, y + (ICON_SIZE/2 - BIGCHAR_HEIGHT/2), bg_itemlist[ value ].pickup_name, fadeColor[0] );
//			trap_R_SetColor( NULL );
//		}
//	}
//	
//	return y;
//}

/*
=====================
CG_DrawLowerLeft

=====================
*/
//static void CG_DrawLowerLeft( void ) {
//	float	y;
//
//	y = 480 - ICON_SIZE;
//
//	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3 ) {
//		y = CG_DrawTeamOverlay( y, false, false );
//	} 
//
//
//	y = CG_DrawPickupItem( y );
//}


//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
//static void CG_DrawTeamInfo( void ) {
//	int w, h;
//	int i, len;
//	vec4_t		hcolor;
//	int		chatHeight;
//
//#define CHATLOC_Y 420 // bottom end
//#define CHATLOC_X 0
//
//	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
//		chatHeight = cg_teamChatHeight.integer;
//	else
//		chatHeight = TEAMCHAT_HEIGHT;
//	if (chatHeight <= 0)
//		return; // disabled
//
//	if (cgs.teamLastChatPos != cgs.teamChatPos) {
//		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer) {
//			cgs.teamLastChatPos++;
//		}
//
//		h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;
//
//		w = 0;
//
//		for (i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++) {
//			len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
//			if (len > w)
//				w = len;
//		}
//		w *= TINYCHAR_WIDTH;
//		w += TINYCHAR_WIDTH * 2;
//
//		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
//			hcolor[0] = 1;
//			hcolor[1] = 0;
//			hcolor[2] = 0;
//			hcolor[3] = 0.33f;
//		} else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
//			hcolor[0] = 0;
//			hcolor[1] = 0;
//			hcolor[2] = 1;
//			hcolor[3] = 0.33f;
//		} else {
//			hcolor[0] = 0;
//			hcolor[1] = 1;
//			hcolor[2] = 0;
//			hcolor[3] = 0.33f;
//		}
//
//		trap_R_SetColor( hcolor );
//		CG_DrawPic( CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar );
//		trap_R_SetColor( NULL );
//
//		hcolor[0] = hcolor[1] = hcolor[2] = 1.0;
//		hcolor[3] = 1.0;
//
//		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--) {
//			CG_DrawStringExt( CHATLOC_X + TINYCHAR_WIDTH, 
//				CHATLOC_Y - (cgs.teamChatPos - i)*TINYCHAR_HEIGHT, 
//				cgs.teamChatMsgs[i % chatHeight], hcolor, false, false,
//				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
//		}
//	}
//}


/*
===================
CG_DrawReward
===================
*/
//static void CG_DrawReward( void ) { 
//	float	*color;
//	int		i, count;
//	float	x, y;
//	char	buf[32];
//
//	if ( !cg_drawRewards.integer ) {
//		return;
//	}
//
//	color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
//	if ( !color ) {
//		if (cg.rewardStack > 0) {
//			for(i = 0; i < cg.rewardStack; i++) {
//				cg.rewardSound[i] = cg.rewardSound[i+1];
//				cg.rewardShader[i] = cg.rewardShader[i+1];
//				cg.rewardCount[i] = cg.rewardCount[i+1];
//			}
//			cg.rewardTime = cg.time;
//			cg.rewardStack--;
//			color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
//			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
//		} else {
//			return;
//		}
//	}
//
//	trap_R_SetColor( color );
//
//	if ( cg.rewardCount[0] >= 10 ) {
//		y = 56;
//		x = 320 - ICON_SIZE/2;
//		CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
//		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
//		x = ( SCREEN_WIDTH - SMALLCHAR_WIDTH * CG_DrawStrlen( buf ) ) / 2;
//		CG_DrawStringExt( x, y+ICON_SIZE, buf, color, false, true,
//								SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
//	}
//	else {
//
//		count = cg.rewardCount[0];
//
//		y = 56;
//		x = 320 - count * ICON_SIZE/2;
//		for ( i = 0 ; i < count ; i++ ) {
//			CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
//			x += ICON_SIZE;
//		}
//	}
//	trap_R_SetColor( NULL );
//}

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	lagometer.snapshotCount++;
}

/*
===============================================================================

CENTER PRINTING

===============================================================================
*/


/*
==============
CG_CenterPrint

Called for important messages that should stay in the center of the screen
for a few moments
==============
*/
void CG_CenterPrint( const char *str, int y, int charWidth ) {
	char	*s;

	Q_strncpyz( cg.centerPrint, str, sizeof(cg.centerPrint) );

	cg.centerPrintTime = cg.time;
	cg.centerPrintY = y;
	cg.centerPrintCharWidth = charWidth;

	// count the number of lines for centering
	cg.centerPrintLines = 1;
	s = cg.centerPrint;
	while( *s ) {
		if (*s == '\n')
			cg.centerPrintLines++;
		s++;
	}
}


/*
===================
CG_DrawCenterString
===================
*/
static void CG_DrawCenterString( void ) {
	char	*start;
	int		l;
	int		x, y, w;//, h;
	float	*color;

	if ( !cg.centerPrintTime ) {
		return;
	}

	color = CG_FadeColor( cg.centerPrintTime, 1000 * cg_centertime.value );
	if ( !color ) {
		return;
	}

	trap_R_SetColor( color );

	start = cg.centerPrint;

	y = cg.centerPrintY - cg.centerPrintLines * BIGCHAR_HEIGHT / 2;

	while ( 1 ) {
		char linebuffer[1024];

		for ( l = 0; l < 50; l++ ) {
			if ( !start[l] || start[l] == '\n' ) {
				break;
			}
			linebuffer[l] = start[l];
		}
		linebuffer[l] = 0;

		w = cg.centerPrintCharWidth * CG_DrawStrlen( linebuffer );

		x = ( SCREEN_WIDTH - w ) / 2;

		cgUtils.drawStringNewColour( 320, y, linebuffer, color, CENTRE_JUSTIFY );

		y += cg.centerPrintCharWidth * 1.5;

		while ( *start && ( *start != '\n' ) ) {
			start++;
		}
		if ( !*start ) {
			break;
		}
		start++;
	}

	trap_R_SetColor( NULL );
}



/*
================================================================================

CROSSHAIR

================================================================================
*/


/*
=================
CG_DrawCrosshair
=================
*//*
static void CG_DrawCrosshair(void) {
	float		w, h;
	qhandle_t	hShader;
	float		f;
	float		x, y;
	int			ca;

	if ( !cg_drawCrosshair.integer ) {
		return;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR) {
		return;
	}

	if ( cg.renderingThirdPerson ) {
		return;
	}

	// set color based on health
	trap_R_SetColor( NULL );

	w = h = cg_crosshairSize.value;

	// pulse the size of the crosshair when picking up items
	f = cg.time - cg.itemPickupBlendTime;
	if ( f > 0 && f < ITEM_BLOB_TIME ) {
		f /= ITEM_BLOB_TIME;
		w *= ( 1 + f );
		h *= ( 1 + f );
	}

	x = cg_crosshairX.integer;
	y = cg_crosshairY.integer;
	CG_AdjustFrom640( &x, &y, &w, &h );

	ca = cg_drawCrosshair.integer;
	if (ca < 0) {
		ca = 0;
	}
	hShader = cgs.media.crosshairShader[ ca % NUM_CROSSHAIRS ];

	trap_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef.width - w), 
		y + cg.refdef.y + 0.5 * (cg.refdef.height - h), 
		w, h, 0, 0, 1, 1, hShader );
}
*/


/*
=================
CG_ScanForCrosshairEntity
=================
*//*
static void CG_ScanForCrosshairEntity( void ) {
	trace_t		trace;
	vec3_t		start, end;
	int			content;

	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, 131072, cg.refdef.viewaxis[0], end );

	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, 
		cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY );
	if ( trace.entityNum >= MAX_CLIENTS ) {
		return;
	}

	// if the player is in fog, don't show it
	content = trap_CM_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) {
		return;
	}

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
}*/


/*
=====================
CG_DrawCrosshairNames
=====================
*/
static void CG_DrawCrosshairNames( void ) {
	float		*color;
	char		*name;
	float		w;
/*
	if ( !cg_drawCrosshair.integer ) {
		return;
	}
	if ( !cg_drawCrosshairNames.integer ) {
		return;
	}
	if ( cg.renderingThirdPerson ) {
		return;
	}

	// scan the known entities to see if the crosshair is sighted on one
	CG_ScanForCrosshairEntity();
*/
	// draw the name of the player being looked at
	color = CG_FadeColor( cg.crosshairClientTime, 1000 );
	if ( !color )
	{
		trap_R_SetColor( NULL );
		return;
	}

	// if we get here then we have a name to draw
	name = cgs.clientinfo[ cg.crosshairClientNum ].name;
	w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;

	cgUtils.drawStringNew( 320, 170, 0.5f, color, name, 0, 0, ITEM_TEXTSTYLE_OUTLINED, CENTRE_JUSTIFY );

	trap_R_SetColor( NULL );
}


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) {

	// this is where we load the spectator menu script
	if( menuSpectator == NULL)
	{
		menuSpectator = cgUtils.menu_FindByName("spectator_menu");
	}

	// spectator menu loaded?
	if( menuSpectator )
	{
		// decide what to place into the three textual placeholders in the spectator menu
		
		// still a spectator?
		if( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR )
		{
			if( cgs.gametype != GT_MISSION_EDITOR ) {
				trap_Cvar_Set( "ui_spectatorTitleTxt", "Spectator" );
				trap_Cvar_Set( "ui_spectatorLine1Txt", "Press FIRE to switch view." );
				
				// team game?
				if ( cgs.gametype >= GT_TEAM )
					trap_Cvar_Set( "ui_spectatorLine2Txt", "Choose a team from the 'Team' dialog to play." );
				else
					trap_Cvar_Set( "ui_spectatorLine2Txt", "Select 'Join' from the 'Team' dialog to play." );
			} else {
				trap_Cvar_Set( "ui_spectatorTitleTxt", "Mission Editor" );
				trap_Cvar_Set( "ui_spectatorLine1Txt", "Use Vehicle Selection to spawn a vehicle" );
				trap_Cvar_Set( "ui_spectatorLine2Txt", "Press FIRE2 to select what you are looking at" );

			}
		}
		else
		{
			// we have a team (or don't need one) but no vehicle
			trap_Cvar_Set( "ui_spectatorTitleTxt", "Limbo" );
			trap_Cvar_Set( "ui_spectatorLine1Txt", "Choose a vehicle to join in the game or" );
			trap_Cvar_Set( "ui_spectatorLine2Txt", "select 'Specator' from the 'Team' dialog." );
		}

		// draw
		cgUtils.menu_Paint( menuSpectator, true );
	}

	if( cgs.gametype == GT_MISSION_EDITOR ) {
		float		w, h;
		qhandle_t	hShader;
		float		x, y;

		w = h = cg_crosshairSize.value;
		x = cg_crosshairX.integer;
		y = cg_crosshairY.integer;

		CG_AdjustFrom640( &x, &y, &w, &h );

		hShader = cgs.media.crosshairShader[8];

		trap_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef.width - w), 
			y + cg.refdef.y + 0.5 * (cg.refdef.height - h), 
		w, h, 0, 0, 1, 1, hShader );
	}
}

/*
=================
CG_DrawVote
=================
*/
static void CG_DrawVote(void) {
	char	*s;
	int		sec;

	if ( !cgs.voteTime ) {
		return;
	}

	// play a talk beep whenever it is modified
	if ( cgs.voteModified ) {
		cgs.voteModified = false;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo );
	CG_DrawSmallString( 0, 58, s, 1.0F );
}

/*
=================
CG_DrawTeamVote
=================
*/
static void CG_DrawTeamVote(void) {
	char	*s;
	int		sec, cs_offset;

	if ( cgs.clientinfo->team == TEAM_RED )
		cs_offset = 0;
	else if ( cgs.clientinfo->team == TEAM_BLUE )
		cs_offset = 1;
	else
		return;

	if ( !cgs.teamVoteTime[cs_offset] ) {
		return;
	}

	// play a talk beep whenever it is modified
	if ( cgs.teamVoteModified[cs_offset] ) {
		cgs.teamVoteModified[cs_offset] = false;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.teamVoteTime[cs_offset] ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	s = va("TEAMVOTE(%i):%s yes:%i no:%i", sec, cgs.teamVoteString[cs_offset],
							cgs.teamVoteYes[cs_offset], cgs.teamVoteNo[cs_offset] );
	CG_DrawSmallString( 0, 90, s, 1.0F );
}

/*
=================
CG_DrawScoreboard
=================
*/
static bool CG_DrawScoreboard()
{
	static bool firstTime = true;
	float fade, *fadeColor;

	if( menuScoreboard )
	{
		menuScoreboard->window.flags &= ~WINDOW_FORCED;
	}

	if( cg_paused.integer )
	{
		cg.deferredPlayerLoading = 0;
		firstTime = true;
		return false;
	}

	// should never happen in Team Arena
	if (cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION )
	{
		cg.deferredPlayerLoading = 0;
		firstTime = true;
		return false;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && !cg.showScores )
	{
		return false;
	}

	// only PM_INTERMISSION is a valid state for a non-user-requested scoreboard
	if( !cg.showScores && !(cg.predictedPlayerState.pm_type == PM_INTERMISSION) )
	{
		return false;
	}

	// fading... (MFQ3: probably not used any more)
	if ( cg.showScores || cg.predictedPlayerState.pm_type == PM_INTERMISSION )
	{
		fade = 1.0;
		fadeColor = colorWhite;
	}
	else
	{
		fadeColor = CG_FadeColor( cg.scoreFadeTime, FADE_TIME );
		if( !fadeColor )
		{
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			firstTime = true;

			return false;
		}
		fade = *fadeColor;
	}																					  

	// this is where we load the correct scoreboard script
	if (menuScoreboard == NULL)
	{
		if ( cgs.gametype >= GT_TEAM )
		{
			menuScoreboard = cgUtils.menu_FindByName("teamscore_menu");
		}
		else
		{
			menuScoreboard = cgUtils.menu_FindByName("score_menu");
		}
	}

	if( menuScoreboard )
	{
		if( firstTime )
		{
			CG_SetScoreSelection( menuScoreboard );
			firstTime = false;
		}

		// draw
		cgUtils.menu_Paint( menuScoreboard, true );
	}

	// load any models that have been deferred
	if ( ++cg.deferredPlayerLoading > 10 )
	{
//		CG_LoadDeferredPlayers();	// CG_LoadDeferredPlayers - not yet implemented - MM
	}

	return true;
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void )
{
	if( cgs.gametype == GT_SINGLE_PLAYER )
	{
		CG_DrawCenterString();
		return;
	}

	cg.scoreFadeTime = cg.time;
	cg.scoreBoardShowing = CG_DrawScoreboard();
}

/*
=================
CG_DrawFollow
=================
*/
static bool CG_DrawFollow( void )
{
//	float		x = 0;
	vec4_t		*pColor = NULL;
//	vec4_t		color = {0,0,0,0};
	const char	*name = NULL;

	// are we NOT following?
	if( !(cg.snap->ps.pm_flags & PMF_FOLLOW) )
	{
		return false;
	}

	// New style

	// draw label
	cgUtils.drawStringNewAlpha( 320, 48, "following ", 1.0f, RIGHT_JUSTIFY );

	// default colour
	pColor = cgUtils.createColourVector( 0.0f, 0.75f, 0.0f, 1.0f, NULL );	// default: green

	// team colour?
	if( cgs.clientinfo[ cg.snap->ps.clientNum ].team == 1 )
	{
		pColor = cgUtils.createColourVector( 0.75f, 0.0f, 0.0f, 1.0f, NULL );	// team: red
	}
	else if( cgs.clientinfo[ cg.snap->ps.clientNum ].team == 2 )
	{
		pColor = cgUtils.createColourVector( 0.0f, 0.0f, 0.75f, 1.0f, NULL );	// team: blue
	}

	// get name of player being followed
	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	// draw name
	cgUtils.drawStringNewColour( 320, 48, name, pColor[0], LEFT_JUSTIFY );

	return true;
}

/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
	int			w;
	int			sec;
	int			i;
	float		scale;
	int			cw;
	const char	*s;
	clientInfo_t	*ci1, *ci2;

	sec = cg.warmup;
	if ( !sec ) {
		return;
	}

	if ( sec < 0 ) {
		s = "Waiting for players";		
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;
		cg.warmupCount = 0;

		cgUtils.drawStringNew( 320, 64, 0.5f, *cgUtils.createColourVector(1,1,1,1,NULL), s, 0, 0, 3, CENTRE_JUSTIFY );

		return;
	}

	if (cgs.gametype == GT_TOURNAMENT) {
		// find the two active players
		ci1 = NULL;
		ci2 = NULL;
		for ( i = 0 ; i < cgs.maxclients ; i++ ) {
			if ( cgs.clientinfo[i].infoValid && cgs.clientinfo[i].team == TEAM_FREE ) {
				if ( !ci1 ) {
					ci1 = &cgs.clientinfo[i];
				} else {
					ci2 = &cgs.clientinfo[i];
				}
			}
		}

		if ( ci1 && ci2 ) {
			s = va( "%s vs %s", ci1->name, ci2->name );

			w = CG_DrawStrlen( s );
			if ( w > 640 / GIANT_WIDTH ) {
				cw = 640 / w;
			} else {
				cw = GIANT_WIDTH;
			}
			CG_DrawStringExt( 320 - w * cw/2, 20,s, colorWhite, 
					false, true, cw, (int)(cw * 1.5f), 0 );
		}
	} else {
		if ( cgs.gametype == GT_FFA ) {
			s = "Free For All";
		} else if ( cgs.gametype == GT_TEAM ) {
			s = "Team Deathmatch";
		} else if ( cgs.gametype == GT_CTF ) {
			s = "Capture the Flag";
		} else if ( cgs.gametype == GT_MISSION_EDITOR ) {
			s = "MFQ3 Mission Editor";
		} else {
			s = "";
		}

		w = CG_DrawStrlen( s );
		if ( w > 640 / GIANT_WIDTH ) {
			cw = 640 / w;
		} else {
			cw = GIANT_WIDTH;
		}
		CG_DrawStringExt( 320 - w * cw/2, 25,s, colorWhite, 
				false, true, cw, (int)(cw * 1.1), 0 );
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}
	s = va( "Starts in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
		if( cgs.gametype != GT_MISSION_EDITOR ) {
			switch ( sec ) {
			case 0:
				trap_S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
				break;
			case 1:
				trap_S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
				break;
			case 2:
				trap_S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
				break;
			default:
				break;
			}
		}
	}
	scale = 0.45f;
	switch ( cg.warmupCount ) {
	case 0:
		cw = 28;
		scale = 0.54f;
		break;
	case 1:
		cw = 24;
		scale = 0.51f;
		break;
	case 2:
		cw = 20;
		scale = 0.48f;
		break;
	default:
		cw = 16;
		scale = 0.45f;
		break;
	}

	w = CG_DrawStrlen( s );
	CG_DrawStringExt( 320 - w * cw/2, 70, s, colorWhite, 
			false, true, cw, (int)(cw * 1.5), 0 );
}

//==================================================================================

static void CG_DrawTourneyScoreboard() {
	CG_DrawOldTourneyScoreboard();
}

/*
=================
CG_Draw2D_MFQ3
MFQ3 version, not using the other version in order
to keep it in case we have to look something up later
- old version shall be deleted later though!
=================
*/
static void CG_Draw2D_MFQ3( void ) {

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot )
	{
		return;
	}

	// global 2D enable/disable var
	if ( cg_draw2D.integer == 0 )
	{
		return;
	}

	// only draw intermission stuff?
	if ( cg.snap->ps.pm_type == PM_INTERMISSION )
	{
		CG_DrawIntermission();
		return;
	}

	// don't draw any HUD if scoreboard is up
	cg.scoreBoardShowing = CG_DrawScoreboard();
	if( cg.scoreBoardShowing )
	{
		// draw the timers
		CG_DrawUpperRight( true );

		return;
	}

	// are we in spectator mode, or awaiting vehicle selection?
	if( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR || cg_vehicle.integer == -1 )
	{
		CG_DrawSpectator();
//		CG_DrawCrosshair();
//		CG_DrawCrosshairNames();
	}
	else
	{
		// don't draw any status if dead
		if ( cg.snap->ps.pm_type != PM_DEAD &&
			 cg.snap->ps.stats[STAT_HEALTH] > 0 &&
			 !(cg.snap->ps.pm_flags & PMF_VEHICLESELECT) )
		{
			// old or New HUD?
			if( cg_oldHUD.integer )
			{
				CG_DrawStatusBar_MFQ3();
			}
			else
			{
      			CG_DrawStatusBar_MFQ3_new();
			}

//			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
//			CG_DrawReward();
		}
    
	}

	CG_DrawVote();
	CG_DrawTeamVote();

	CG_DrawUpperLeft();

	CG_DrawUpperRight( false );

	if( !CG_DrawFollow() )
	{
		CG_DrawWarmup();
	}
	CG_DrawCenterString();
}


/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
	float		separation;
	vec3_t		baseOrg;

	// optionally draw the info screen instead
	if( !cg.snap ) {
		CG_DrawInformation();
		return;
	}

	// optionally draw the tournement scoreboard instead
	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR &&
		( cg.snap->ps.pm_flags & PMF_SCOREBOARD ) ) {
		CG_DrawTourneyScoreboard();
		return;
	}

	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value / 2;
		break;
	default:
		separation = 0;
		Com_Error( ERR_DROP, "CG_DrawActive: Undefined stereoView" );
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy( cg.refdef.vieworg, baseOrg );
	if ( separation != 0 ) {
		VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
	}

	// draw 3D view
	trap_R_RenderScene( &cg.refdef );

	// restore original viewpoint if running stereo
	if ( separation != 0 ) {
		VectorCopy( baseOrg, cg.refdef.vieworg );
	}

	// draw status bar and other floating elements
// 	CG_Draw2D();
	CG_Draw2D_MFQ3();
}


/*
=====================
CG_GetGameStatusText
=====================
*/
const char *CG_GetGameStatusText()
{
	const char * s = "";
	char * pFormatStringDraw = NULL;
	char * pFormatStringRedWin = NULL;
	char * pFormatStringBlueWin = NULL;
	
	// which gametype?
	if ( cgs.gametype == GT_FFA )
	{
		// Deathmatch
		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR )
		{
			s = va("%s place with %i",CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),cg.snap->ps.persistant[PERS_SCORE] );
		}
	}
	else if ( cgs.gametype == GT_TEAM || cgs.gametype == GT_CTF )
	{
		// Team Deathmatch / Capture-The-Flag

		// NOT intermission?
		if ( cg.snap->ps.pm_type != PM_INTERMISSION )
		{
			pFormatStringDraw = "Teams tied at %i";
			pFormatStringRedWin = "Red leads Blue, by %i to %i";
			pFormatStringBlueWin = "Blue leads Red, by %i to %i";
		}
		else
		{
			pFormatStringDraw = "Match is drawn";
			pFormatStringRedWin = "Red wins by %i to %i!";
			pFormatStringBlueWin = "Blue wins by %i to %i!";
		}

		// team deathmatch / capture-the-flag
		if ( cg.teamScores[0] == cg.teamScores[1] )
		{
			s = va( pFormatStringDraw, cg.teamScores[0] );
		}
		else if ( cg.teamScores[0] >= cg.teamScores[1] )
		{
			s = va( pFormatStringRedWin, cg.teamScores[0], cg.teamScores[1] );
		}
		else 
		{
			s = va( pFormatStringBlueWin, cg.teamScores[1], cg.teamScores[0] );
		}
	}

	return s;
}
	
/*
=====================
CG_GameMiscString
=====================
*/
const char *CG_GameMiscString() {
		
	// intermission?
	if ( cg.snap->ps.pm_type == PM_INTERMISSION && strlen( cg.scoreboardMisc ) > 0 )
	{
		return cg.scoreboardMisc;
	}

	return "";
}

/*
=====================
CG_GameTypeString
=====================
*/
const char *CG_GameTypeString()
{
	const char * pGameType = NULL;
	const char * pGameSet = NULL;

	// get game type
	if ( cgs.gametype == GT_FFA ) {
		pGameType = "Deathmatch";
	} else if ( cgs.gametype == GT_TEAM ) {
		pGameType = "Team Deathmatch";
	} else if ( cgs.gametype == GT_CTF ) {
		pGameType = "Capture the Flag";
	} /* else if ( cgs.gametype == GT_1FCTF ) {
		return "One Flag CTF";
	} else if ( cgs.gametype == GT_OBELISK ) {
		return "Overload";
	} else if ( cgs.gametype == GT_HARVESTER ) {
		return "Harvester";
	} */

	// get game set
	pGameSet = gameset_items[ MF_CG_GetGameset( true ) ];

	// return the string as "<gametype> (<gameset>)"
	return va( "%s - %s", pGameType, pGameSet );
}

/*
=====================
CG_DrawGameType
=====================
*/
static void CG_DrawGameType(rectDef_t *rect, float scale, const vec4_t color, qhandle_t shader, int textStyle )
{
	CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, CG_GameTypeString(), 0, 0, textStyle);
}

/*
=====================
CG_DrawGameMisc
=====================
*/
static void CG_DrawGameMisc( itemDef_t * item )
{
	// setup text
	item->text = CG_GameMiscString();

	// draw
	cgUtils.item_Text_AutoWrapped_Paint( item );
}

/*
=====================
CG_DrawTeamScore
=====================
*/
static void CG_DrawTeamScore( itemDef_t * item, int team )
{
	// NOTE: the score strings are  copied to these statics because just using the pointer returned 
	// by va() appears to cause memory problems
	static char redScore[32] = { 0 };
	static char blueScore[32] = { 0 };

	// setup text
	switch( team )
	{
	case CG_GAME_REDSCORE:
		strcpy( redScore, va( "%d", cg.teamScores[0] ) );
		item->text = redScore; 
		break;

	case CG_GAME_BLUESCORE:
		strcpy( blueScore, va( "%d", cg.teamScores[1] ) );
		item->text = blueScore; 
		break;

	default:
		item->text = "";
		break;
	}

	// draw
	cgUtils.item_Text_AutoWrapped_Paint( item );
}

/*
=====================
CG_DrawTeamCount
=====================
*/
static void CG_DrawTeamCount( itemDef_t * item, int team )
{
	// NOTE: the count strings are  copied to these statics because just using the pointer returned 
	// by va() appears to cause memory problems
	static char redCount[32] = { 0 };
	static char blueCount[32] = { 0 };
	static char spectatorCount[32] = { 0 };
	static char playerCount[32] = { 0 };
	int count = 0;

	// setup text
	switch( team )
	{
	case CG_GAME_REDCOUNT:
		count = CG_FeederCount( FEEDER_REDTEAM_LIST );
		if( count == 1 )
			strcpy( redCount, va( "Red (%d player)", count ));
		else
			strcpy( redCount, va( "Red (%d players)", count ));
		item->text = redCount; 
		
		break;

	case CG_GAME_BLUECOUNT:
		count = CG_FeederCount( FEEDER_BLUETEAM_LIST );
		if( count == 1 )
			strcpy( blueCount, va( "Blue (%d player)", count ));
		else
			strcpy( blueCount, va( "Blue (%d players)", count ));
		item->text = blueCount; 
		break;

	case CG_GAME_SPECTATORCOUNT:
		count = CG_FeederCount( FEEDER_SPECTATOR_LIST );
		if( count == 1 )
			strcpy( spectatorCount, va( "Spectator (%d client)", count ));
		else
			strcpy( spectatorCount, va( "Spectators (%d clients)", count ));
		item->text = spectatorCount; 
		break;

	case CG_GAME_PLAYERSCOUNT:
		count = CG_FeederCount( FEEDER_SCOREBOARD );
		if( count == 1 )
			strcpy( playerCount, va( "Players (%d player)", count ));
		else
			strcpy( playerCount, va( "Players (%d players)", count ));
		item->text = playerCount; 
		break;

	default:
		item->text = "";
		break;
	}

	// draw
	cgUtils.item_Text_AutoWrapped_Paint( item );
}

/*
=====================
CG_DrawGameStatus
=====================
*/
static void CG_DrawGameStatus(rectDef_t *rect, float scale, const vec4_t color, qhandle_t shader, int textStyle )
{
	CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, CG_GetGameStatusText(), 0, 0, textStyle);
}

/*
=====================
CG_OwnerDraw
=====================
*/
void CG_OwnerDraw( float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, 
				  int align, float special, float scale, const vec4_t color, qhandle_t shader, int textStyle, 
				  itemDef_t * item )
{
	rectDef_t rect;

    if ( cg_drawStatus.integer == 0 )
	{
		return;
	}

	//if (ownerDrawFlags != 0 && !CG_OwnerDrawVisible(ownerDrawFlags)) {
	//	return;
	//}

  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;

  // which owner draw ID?
  switch (ownerDraw)
  {
/*
  case CG_PLAYER_ARMOR_ICON:
    CG_DrawPlayerArmorIcon(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_ARMOR_ICON2D:
    CG_DrawPlayerArmorIcon(&rect, true);
    break;
  case CG_PLAYER_ARMOR_VALUE:
    CG_DrawPlayerArmorValue(&rect, scale, color, shader, textStyle);
    break;
  case CG_PLAYER_AMMO_ICON:
    CG_DrawPlayerAmmoIcon(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_AMMO_ICON2D:
    CG_DrawPlayerAmmoIcon(&rect, true);
    break;
  case CG_PLAYER_AMMO_VALUE:
    CG_DrawPlayerAmmoValue(&rect, scale, color, shader, textStyle);
    break;
  case CG_SELECTEDPLAYER_HEAD:
    CG_DrawSelectedPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY, false);
    break;
  case CG_VOICE_HEAD:
    CG_DrawSelectedPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY, true);
    break;
  case CG_VOICE_NAME:
    CG_DrawSelectedPlayerName(&rect, scale, color, true, textStyle);
    break;
  case CG_SELECTEDPLAYER_STATUS:
    CG_DrawSelectedPlayerStatus(&rect);
    break;
  case CG_SELECTEDPLAYER_ARMOR:
    CG_DrawSelectedPlayerArmor(&rect, scale, color, shader, textStyle);
    break;
  case CG_SELECTEDPLAYER_HEALTH:
    CG_DrawSelectedPlayerHealth(&rect, scale, color, shader, textStyle);
    break;
  case CG_SELECTEDPLAYER_NAME:
    CG_DrawSelectedPlayerName(&rect, scale, color, false, textStyle);
    break;
  case CG_SELECTEDPLAYER_LOCATION:
    CG_DrawSelectedPlayerLocation(&rect, scale, color, textStyle);
    break;
  case CG_SELECTEDPLAYER_WEAPON:
    CG_DrawSelectedPlayerWeapon(&rect);
    break;
  case CG_SELECTEDPLAYER_POWERUP:
    CG_DrawSelectedPlayerPowerup(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_HEAD:
    CG_DrawPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_ITEM:
    CG_DrawPlayerItem(&rect, scale, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_SCORE:
    CG_DrawPlayerScore(&rect, scale, color, shader, textStyle);
    break;
  case CG_PLAYER_HEALTH:
    CG_DrawPlayerHealth(&rect, scale, color, shader, textStyle);
    break;
  case CG_RED_SCORE:
    CG_DrawRedScore(&rect, scale, color, shader, textStyle);
    break;
  case CG_BLUE_SCORE:
    CG_DrawBlueScore(&rect, scale, color, shader, textStyle);
    break;
  case CG_RED_NAME:
    CG_DrawRedName(&rect, scale, color, textStyle);
    break;
  case CG_BLUE_NAME:
    CG_DrawBlueName(&rect, scale, color, textStyle);
    break;
  case CG_BLUE_FLAGHEAD:
    CG_DrawBlueFlagHead(&rect);
    break;
  case CG_BLUE_FLAGSTATUS:
    CG_DrawBlueFlagStatus(&rect, shader);
    break;
  case CG_BLUE_FLAGNAME:
    CG_DrawBlueFlagName(&rect, scale, color, textStyle);
    break;
  case CG_RED_FLAGHEAD:
    CG_DrawRedFlagHead(&rect);
    break;
  case CG_RED_FLAGSTATUS:
    CG_DrawRedFlagStatus(&rect, shader);
    break;
  case CG_RED_FLAGNAME:
    CG_DrawRedFlagName(&rect, scale, color, textStyle);
    break;
  case CG_HARVESTER_SKULLS:
    CG_HarvesterSkulls(&rect, scale, color, false, textStyle);
    break;
  case CG_HARVESTER_SKULLS2D:
    CG_HarvesterSkulls(&rect, scale, color, true, textStyle);
    break;
  case CG_ONEFLAG_STATUS:
    CG_OneFlagStatus(&rect);
    break;
  case CG_PLAYER_LOCATION:
    CG_DrawPlayerLocation(&rect, scale, color, textStyle);
    break;
  case CG_TEAM_COLOR:
    CG_DrawTeamColor(&rect, color);
    break;
  case CG_CTF_POWERUP:
    CG_DrawCTFPowerUp(&rect);
    break;
  case CG_AREA_POWERUP:
	CG_DrawAreaPowerUp(&rect, align, special, scale, color);
    break;
  case CG_PLAYER_STATUS:
    CG_DrawPlayerStatus(&rect);
    break;
  case CG_PLAYER_HASFLAG:
    CG_DrawPlayerHasFlag(&rect, false);
    break;
  case CG_PLAYER_HASFLAG2D:
    CG_DrawPlayerHasFlag(&rect, true);
    break;
  case CG_AREA_SYSTEMCHAT:
    CG_DrawAreaSystemChat(&rect, scale, color, shader);
    break;
  case CG_AREA_TEAMCHAT:
    CG_DrawAreaTeamChat(&rect, scale, color, shader);
    break;
  case CG_AREA_CHAT:
	CG_DrawTeamScore( item, -1 );
    CG_DrawAreaChat(&rect, scale, color, shader);
    break;
*/
  case CG_GAME_MISC:
    CG_DrawGameMisc( item );
	break;

  case CG_GAME_REDCOUNT:
    CG_DrawTeamCount( item, CG_GAME_REDCOUNT );
	break;

  case CG_GAME_BLUECOUNT:
    CG_DrawTeamCount( item, CG_GAME_BLUECOUNT );
    break;

  case CG_GAME_SPECTATORCOUNT:
    CG_DrawTeamCount( item, CG_GAME_SPECTATORCOUNT );
    break;

  case CG_GAME_PLAYERSCOUNT:
    CG_DrawTeamCount( item, CG_GAME_PLAYERSCOUNT );
    break;

  case CG_GAME_REDSCORE:
    CG_DrawTeamScore( item, CG_GAME_REDSCORE );
	break;

  case CG_GAME_BLUESCORE:
    CG_DrawTeamScore( item, CG_GAME_BLUESCORE );
    break;

  case CG_GAME_TYPE:
    CG_DrawGameType( &rect, scale, color, shader, textStyle );
    break;

  case CG_GAME_STATUS:
    CG_DrawGameStatus( &rect, scale, color, shader, textStyle );
	break;
/*
  case CG_KILLER:
    CG_DrawKiller(&rect, scale, color, shader, textStyle);
		break;
	case CG_ACCURACY:
	case CG_ASSISTS:
	case CG_DEFEND:
	case CG_EXCELLENT:
	case CG_IMPRESSIVE:
	case CG_PERFECT:
	case CG_GAUNTLET:
	case CG_CAPTURES:
		CG_DrawMedal(ownerDraw, &rect, scale, color, shader);
		break;
  case CG_SPECTATORS:
		CG_DrawTeamSpectators(&rect, scale, color, shader);
		break;
  case CG_TEAMINFO:
		if (cg_currentSelectedPlayer.integer == numSortedTeamPlayers) {
			CG_DrawNewTeamInfo(&rect, text_x, text_y, scale, color, shader);
		}
		break;
  case CG_CAPFRAGLIMIT:
    CG_DrawCapFragLimit(&rect, scale, color, shader, textStyle);
		break;
  case CG_1STPLACE:
    CG_Draw1stPlace(&rect, scale, color, shader, textStyle);
		break;
  case CG_2NDPLACE:
    CG_Draw2ndPlace(&rect, scale, color, shader, textStyle);
		break;
  default:
    break;
*/
  }
}
	// NOTE: functions copied from v1.29h code to enable New scoreboard method
