/*
 * $Id: cg_draw.c,v 1.10 2002-01-23 22:28:13 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"

#ifdef _MENU_SCOREBOARD
	#include "../ui/ui_shared.h"

	// used for scoreboard
	extern displayContextDef_t cgDC;
	menuDef_t *menuScoreboard = NULL;
	menuDef_t *menuSpectator = NULL;

	// used for general drawing
	extern displayContextDef_t *DC;
#endif

int drawTeamOverlayModificationCount = -1;

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;

char systemChat[256];
char teamChat1[256];
char teamChat2[256];

#ifdef _MENU_SCOREBOARD

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
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_smallFont.value) {
		font = &cgDC.Assets.smallFont;
	} else if (scale > cg_bigFont.value) {
		font = &cgDC.Assets.bigFont;
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
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_smallFont.value) {
		font = &cgDC.Assets.smallFont;
	} else if (scale > cg_bigFont.value) {
		font = &cgDC.Assets.bigFont;
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
void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style) {
  int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float useScale;
	fontInfo_t *font = &cgDC.Assets.textFont;
	if (scale <= cg_smallFont.value) {
		font = &cgDC.Assets.smallFont;
	} else if (scale > cg_bigFont.value) {
		font = &cgDC.Assets.bigFont;
	}
	useScale = scale * font->glyphScale;
  if (text) {
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
			glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
      //int yadj = Assets.textFont.glyphs[text[i]].bottom + Assets.textFont.glyphs[text[i]].top;
      //float yadj = scale * (Assets.textFont.glyphs[text[i]].imageHeight - Assets.textFont.glyphs[text[i]].height);
			if ( Q_IsColorString( s ) ) {
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				trap_R_SetColor( newColor );
				s += 2;
				continue;
			} else {
				float yadj = useScale * glyph->top;
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

#endif

/*
==============
CG_DrawField

Draws large numbers
==============
*/
static void CG_DrawField (int x, int y, int width, int value) {
	char	num[16], *ptr;
	int		l;
	int		frame;

	if ( width < 1 ) {
		return;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + CHAR_WIDTH*(width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		CG_DrawPic( x,y, CHAR_WIDTH, CHAR_HEIGHT, cgs.media.numberShaders[frame] );
		x += CHAR_WIDTH;
		ptr++;
		l--;
	}
}


/*
==============
CG_MFQ3HUD_Numbers
==============
*/
static void CG_MFQ3HUD_Numbers (int x, int y, int width, int value, qboolean percent) {
	char	num[16], *ptr;
	int		l;
	int		frame;

	if ( width < 1 ) {
		return;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + (HUDNUM_WIDTH-1)*(width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		CG_DrawPic( x,y, HUDNUM_WIDTH, HUDNUM_HEIGHT, cgs.media.HUDnumbers[frame] );
		x += HUDNUM_WIDTH-1;
		ptr++;
		l--;
	}
	if( percent ) {
		CG_DrawPic( x,y, HUDNUM_WIDTH, HUDNUM_HEIGHT, cgs.media.HUDnumbers[STAT_PERCENT] );
	}
}


/*
==============
CG_MFQ3HUD_DecNumbers

prints a number with a '.' in it; ie divides this number by 10^decimals

width = total width
==============
*/
static void CG_MFQ3HUD_DecNumbers (int x, int y, int width, int decimals, int invalue) {
	int		div = 10;
	int		pre, post;
	int		prewid, 
			postwid = decimals;

	while( --decimals ) div *= 10;

	pre = invalue/div;
	post = invalue%div;

	prewid = width-decimals-1;

	CG_MFQ3HUD_Numbers( x, y, prewid, pre, qfalse );
	x += (HUDNUM_WIDTH-1)*prewid;

	CG_DrawPic( x+2,y+2, HUDNUM_WIDTH, HUDNUM_HEIGHT, cgs.media.HUDnumbers[STAT_POINT] );
	x += HUDNUM_WIDTH-1;

	CG_MFQ3HUD_Numbers( x, y, postwid, post, qfalse );
}




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
void CG_DrawFlagModel( float x, float y, float w, float h, int team, qboolean force2D ) {
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
CG_DrawStatusBarFlag

================
*/
static void CG_DrawStatusBarFlag( float x, int team ) {
	CG_DrawFlagModel( x, 480 - ICON_SIZE, ICON_SIZE, ICON_SIZE, team, qfalse );
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
================
CG_DrawRadarSymbols

================
*/
static void CG_DrawRadarSymbols_AIR( int vehicle ) {
	unsigned int i;
	centity_t	*self = &cg.predictedPlayerEntity;
	vec3_t		pos, pos1, pos2;
	vec3_t		dir;
	float		dist;
	float		range = availableVehicles[vehicle].radarRange;
	float		scale;
	float		hdg = self->currentState.angles[1];
	float		angle;
	vec3_t		angles;
	int			otherveh;
	vec3_t		otherpos;
	int			icon;
	int			drawnTargets = 0;
	trace_t		res;

	// radar screen and mode
	CG_DrawPic( 440, 280, 200, 200, cgs.media.HUDradar );
	CG_DrawSmallString( 615, 340, "AIR", 1.0f);

	// current range
	if( cg.RADARRangeSetting ) {
		range /= (2 * cg.RADARRangeSetting);
	}

	// adjust for radar screen size
	scale = range/64;

	// tell range
	CG_DrawSmallString( 595, 320, va("%.0f", range), 1.0f);

	// ground vehicle adjustment for angle
	if( (availableVehicles[vehicle].id&CAT_ANY) & CAT_GROUND ) {
		hdg += self->currentState.angles2[ROLL];
		if( hdg > 360 ) hdg -= 360;
		else if( hdg < 0 ) hdg += 360;
	}

	// pos
	VectorCopy( self->currentState.pos.trBase, pos );

	// walk through list of targets
	for( i = 0; i < cg.radarTargets; i++ ) {
		// dont show self
		if( cg.radarEnts[i] == &cg.predictedPlayerEntity ) continue;

		// dont show dead ones
		if( cg.radarEnts[i]->currentState.eFlags & EF_DEAD ) continue;

		// get other vehicle
		if( cg.radarEnts[i]->currentState.eType == ET_MISC_VEHICLE ) {
			otherveh = cg.radarEnts[i]->currentState.modelindex;
		} else {
			otherveh = cgs.clientinfo[cg.radarEnts[i]->currentState.clientNum].vehicle;
		}
		VectorCopy( cg.radarEnts[i]->currentState.pos.trBase, otherpos );

		// get dir and dist
		VectorSet( pos1, otherpos[0], otherpos[1], 0 );
		VectorSet( pos2, pos[0], pos[1], 0 );
		VectorSubtract( pos1, pos2, dir );
		dist = VectorNormalize(dir);
		// check out of range
		if( dist > range ) continue;
		// scale dist to radar screen
		dist /= scale;

		// get screen dir
		vectoangles( dir, angles );
		angle = angles[1]-hdg;
		if( angle > 360 ) angle -= 360;
		else if( angle < 0 ) angle += 360;
		if( angle >= 90 && angle <= 270 ) {
			if( !(cg.radarEnts[i]->currentState.ONOFF & OO_RADAR) ) continue;
		}
		angles[1] -= hdg - 90;
		if( angles[1] > 360 ) angles[1] -= 360;
		else if( angles[1] < 0 ) angles[1] += 360;
		angles[0] = angles[2] = 0;
		AngleVectors( angles, dir, 0, 0 );
		VectorScale( dir, dist, dir );

		// check LOS and/or RADAR on
		if( !(cg.radarEnts[i]->currentState.ONOFF & OO_RADAR) ) {
			CG_Trace( &res, pos, NULL, NULL, otherpos, cg.snap->ps.clientNum, MASK_SOLID );
			if( res.fraction < 1.0f ) continue;
		}
		// which icon (if at all, dont show lqms)
		if( ((availableVehicles[otherveh].id&CAT_ANY) & CAT_PLANE) || 
			((availableVehicles[otherveh].id&CAT_ANY) & CAT_HELO) ) {
			float alt = otherpos[2] - pos[2];
			icon = RD_AIR_SAME_ENEMY;
			if( alt > 1000 ) icon += 3;
			else if( alt < -1000 ) icon += 6;
		} else {
			continue;
		}
		// friend or foe
		if( cgs.gametype >= GT_TEAM ) {
			if( cgs.clientinfo[cg.radarEnts[i]->currentState.clientNum].team ==
				cgs.clientinfo[self->currentState.clientNum].team ) 
				icon++;
		}

		// draw
		CG_DrawPic( 558 + dir[0], 386 - dir[1], 8, 8, cgs.media.radarIcons[icon] );
		// check if we should draw any more targets
		drawnTargets++;
		if( drawnTargets >= cg_radarTargets.integer ) break;
	}

	cg.radarTargets = 0;

}

static void CG_DrawRadarSymbols_GROUND( int vehicle ) {
	unsigned int i;
	centity_t	*self = &cg.predictedPlayerEntity;
	vec3_t		pos, pos1, pos2;
	vec3_t		dir;
	float		dist;
	float		range = availableVehicles[vehicle].radarRange2;
	float		scale;
	float		hdg = self->currentState.angles[1];
	float		angle;
	vec3_t		angles;
	int			otherveh;
	vec3_t		otherpos;
	int			icon;
	int			drawnTargets = 0;
	trace_t		res;

	// radar screen and mode
	CG_DrawPic( 440, 280, 200, 200, cgs.media.HUDradar );
	CG_DrawSmallString( 615, 340, "GND", 1.0f);

	if( cg.RADARRangeSetting ) {
		range /= (2 * cg.RADARRangeSetting);
	}

	// adjust for radar screen size
	scale = range/64;

	// tell range
	CG_DrawSmallString( 595, 320, va("%.0f", range), 1.0f);

	// ground vehicle adjustment for angle
	if( (availableVehicles[vehicle].id&CAT_ANY) & CAT_GROUND ) {
		hdg += self->currentState.angles2[ROLL];
		if( hdg > 360 ) hdg -= 360;
		else if( hdg < 0 ) hdg += 360;
	}

	// pos
	VectorCopy( self->currentState.pos.trBase, pos );

	// walk through list of targets
	for( i = 0; i < cg.radarTargets; i++ ) {
		// dont show self
		if( cg.radarEnts[i] == &cg.predictedPlayerEntity ) continue;

		// dont show dead ones
		if( cg.radarEnts[i]->currentState.eFlags & EF_DEAD ) continue;

		// vehicles
		if( !cg.radarEnts[i]->destroyableStructure ) {
			// get other vehicle
			if( cg.radarEnts[i]->currentState.eType == ET_MISC_VEHICLE ) {
				otherveh = cg.radarEnts[i]->currentState.modelindex;
			} else {
				otherveh = cgs.clientinfo[cg.radarEnts[i]->currentState.clientNum].vehicle;
			}
			VectorCopy( cg.radarEnts[i]->currentState.pos.trBase, otherpos );

			// get dir and dist
			VectorSet( pos1, otherpos[0], otherpos[1], 0 );
			VectorSet( pos2, pos[0], pos[1], 0 );
			VectorSubtract( pos1, pos2, dir );
			dist = VectorNormalize(dir);
			// check out of range
			if( dist > range ) continue;
			// scale dist to radar screen
			dist /= scale;

			// get screen dir
			vectoangles( dir, angles );
			angle = angles[1]-hdg;
			if( angle > 360 ) angle -= 360;
			else if( angle < 0 ) angle += 360;
			if( angle >= 45 && angle <= 315 ) {
				if( !(cg.radarEnts[i]->currentState.ONOFF & OO_RADAR) ||	// radar off
					( cg.radarEnts[i]->destroyableStructure &&	// or radar on and destroyable building
					(cg.radarEnts[i]->currentState.ONOFF & OO_RADAR) ) ) continue;
			}
			angles[1] -= hdg - 90;
			if( angles[1] > 360 ) angles[1] -= 360;
			else if( angles[1] < 0 ) angles[1] += 360;
			angles[0] = angles[2] = 0;
			AngleVectors( angles, dir, 0, 0 );
			VectorScale( dir, dist, dir );

			// check LOS and/or RADAR on
			if( !(cg.radarEnts[i]->currentState.ONOFF & OO_RADAR) ) {

				CG_Trace( &res, pos, NULL, NULL, otherpos, cg.snap->ps.clientNum, MASK_SOLID );
				if( res.fraction < 1.0f ) continue;
			}
			// which icon (if at all, dont show lqms)
			if( (availableVehicles[otherveh].id&CAT_ANY) & CAT_GROUND ) {
				icon = RD_GROUND_ENEMY;
			} else {
				continue;
			}
			// friend or foe
			if( cgs.gametype >= GT_TEAM ) {
				if( cgs.clientinfo[cg.radarEnts[i]->currentState.clientNum].team ==
					cgs.clientinfo[self->currentState.clientNum].team ) 
					icon++;
			}
		// buildings
		} else {
			VectorCopy( cgs.inlineModelMidpoints[cg.radarEnts[i]->currentState.modelindex], otherpos );
			// get dir and dist
			VectorSet( pos1, otherpos[0], otherpos[1], 0 );
			VectorSet( pos2, pos[0], pos[1], 0 );
			VectorSubtract( pos1, pos2, dir );
			dist = VectorNormalize(dir);
			// check out of range
			if( dist > range ) continue;
			// scale dist to radar screen
			dist /= scale;

			// get screen dir
			vectoangles( dir, angles );
			angle = angles[1]-hdg;
			if( angle > 360 ) angle -= 360;
			else if( angle < 0 ) angle += 360;
			if( ((availableVehicles[vehicle].id&CAT_ANY) & CAT_GROUND ) &&
				angle >= 90 && angle <= 270 ) continue;
			else if( ((availableVehicles[vehicle].id&CAT_ANY) & CAT_PLANE ) &&
				angle >= 45 && angle <= 315 ) continue;
			angles[1] -= hdg - 90;
			if( angles[1] > 360 ) angles[1] -= 360;
			else if( angles[1] < 0 ) angles[1] += 360;
			angles[0] = angles[2] = 0;
			AngleVectors( angles, dir, 0, 0 );
			VectorScale( dir, dist, dir );

			icon = RD_BUILDING_ENEMY;
			// friend or foe
			if( cgs.gametype >= GT_TEAM ) {
				if( !cg.radarEnts[i]->currentState.generic1 ) {
					icon += 2;
				}
				else if( cg.radarEnts[i]->currentState.generic1 ==
					cgs.clientinfo[self->currentState.clientNum].team ) {
					icon++;
				}
			}
			// check LOS
			CG_Trace( &res, pos, NULL, NULL, otherpos, cg.snap->ps.clientNum, MASK_SOLID );
//			if( res.fraction < 1.0f ) continue;
			if( res.entityNum != cg.radarEnts[i]->currentState.number ) continue;
		}

		// draw
		CG_DrawPic( 558 + dir[0], 386 - dir[1], 8, 8, cgs.media.radarIcons[icon] );
		// check if we should draw any more targets
		drawnTargets++;
		if( drawnTargets >= cg_radarTargets.integer ) break;
	}

	cg.radarTargets = 0;
}


/*
================
CG_Draw_Redundant

================
*/
static void CG_Draw_Redundant(int health, int throttle) {
	float		x, y, width, height;

		// draw health
	if( hud_health.integer ) {
		x = 556;
		y = 318;
		width = 32;
		height = 8;
		CG_AdjustFrom640( &x, &y, &width, &height );
		trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDhealthtext );
		CG_MFQ3HUD_Numbers( 584, 318, 4, health, qtrue );
	}

		// draw throttle
	if( hud_throttle.integer ) {
		x = 556;
		y = 330;
		width = 32;
		height = 8;
		CG_AdjustFrom640( &x, &y, &width, &height );
		trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDthrottletext );
		CG_MFQ3HUD_Numbers( 584, 330, 4, throttle*10, qtrue );
	}

}


/*
================
CG_Draw_Center

================
*/
static void CG_Draw_Center(int health, int throttle) {
	float		x, y, width, height;
	int			healthpic = (health/10)-1;
	int			throttlepic = throttle - 1;
	
	x = 192;		
	y = 448;
	width = 256;
	height = 32;

		// background
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDsolid );

		// health
	if( healthpic >= 0 ) {
		x = 192;
		y = 448;
		width = 128;
		height = 32;
		CG_AdjustFrom640( &x, &y, &width, &height );
		trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDhealth[healthpic] );
	}

		// throttle
	if( throttlepic >= 0 ) {
		x = 320;
		y = 448;
		width = 128;
		height = 32;
		CG_AdjustFrom640( &x, &y, &width, &height );
		trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDthrottle[throttlepic] );
	}
}


/*
================
CG_Draw_MFD

================
*/
static void CG_Draw_MFD(int mfdnum) {
	float		x, y, width, height;
	int			mode = cg.Mode_MFD[mfdnum];

		// draw mfd screen
	if( mfdnum == MFD_1 ) {
		x = 0;
	} else {
		x = 512;		
	}
	y = 352;
	width = height = 128;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDmfd );

		// draw contents of mfd only if it is not off
	if( !mode ) return;

		// RWR mode
	if( mode == MFD_RWR ) {
		trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDrwr );
	} 
}


/*
================
CG_Draw_AltTape

================
*/
static void CG_Draw_AltTape( int value ) {
	int			scale = 500;
	int			value2, visible1, visible2, visible3, visible4, visible5;
	float		offset;
	float		x, y, width, height;
	float		scaledmaxoff = 0.125f/scale;
	qboolean	top, bottom;

		// find offset on alt tape
	value2 = value - scale * 2;
	value2 = value2%scale;
	if( value2 < 0 ) value2 += scale;
	offset = 0.125f - scaledmaxoff*value2;
		// draw alt tape
	x = 584;
	y = 46;
	width = 16;
	height = 256;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0.125f+offset, 1, 0.625f+offset, cgs.media.HUDalt );
		// speed indicater
	x = 578;
	y = 171;
	width = height = 8;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDind_v_r );
		// draw alt value box
	x = 586;
	y = 28;
	width = 32;
	height = 16;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDvaluebox );
		// draw alt
	CG_MFQ3HUD_Numbers( 586, 32, 4, value, qfalse );
		// find visible alt numbers
	visible3 = (int)((value+(scale/2))/scale);
	visible3 *= scale;
	visible2 = visible3 - scale;
	visible1 = visible2 - scale;
	visible4 = visible3 + scale;
	visible5 = visible4 + scale;
	value2 = value - visible3;
	if( value2 < 0 && value2 >= -scale/2 ) {
		top = qfalse;
		bottom = qtrue;
	} else if( value2 > 0 && value2 <= scale/2 ) {
		top = qtrue;
		bottom = qfalse;
	} else {
		top = bottom = qfalse;
	}
		// draw visible alt numbers
	CG_MFQ3HUD_DecNumbers( 588, 171+(value2*64/scale), 4, 1, visible3/100 );
	if( visible2 >= 0 ) {
		value2 = value - visible2;
		CG_MFQ3HUD_DecNumbers( 588, 171+(value2*64/scale), 4, 1, visible2/100 );
		if( bottom && visible1 >= 0 ) {
			value2 = value - visible1;
			CG_MFQ3HUD_DecNumbers( 588, 171+(value2*64/scale), 4, 1, visible1/100 );
		}
	}
	value2 = value - visible4;
	CG_MFQ3HUD_DecNumbers( 588, 171+(value2*64/scale), 4, 1, visible4/100 );
	if( top ) {
		value2 = value - visible5;
		CG_MFQ3HUD_DecNumbers( 588, 171+(value2*64/scale), 4, 1, visible5/100 );
	}
}

/*
================
CG_Draw_SpeedTape

================
*/
static void CG_Draw_SpeedTape( int value, int stallspeed, int gearspeed, int scale ) {
	int			value2, visible1, visible2, visible3, visible4, visible5;
	float		offset;
	float		x, y, width, height;
	float		scaledmaxoff = 0.125f/scale;
	qboolean	top, bottom;

		// find offset on speed tape
	value2 = value - scale * 2;
	value2 = value2%scale;
	if( value2 < 0 ) value2 += scale;
	offset = 0.125f - scaledmaxoff*value2;
		// draw speed tape
	x = 41;
	y = 46;
	width = 16;
	height = 256;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0.125f+offset, 1, 0.625f+offset, cgs.media.HUDspeed );
		// speed indicater
	x = 55;
	y = 171;
	width = height = 8;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDind_v );
		// draw speed value box
	x = 24;
	y = 28;
	width = 32;
	height = 16;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDvaluebox );
		// draw speed
	CG_MFQ3HUD_Numbers( 24, 32, 4, value, qfalse );
		// find visible speed numbers
	visible3 = (int)((value+(scale/2))/scale);
	visible3 *= scale;
	visible2 = visible3 - scale;
	visible1 = visible2 - scale;
	visible4 = visible3 + scale;
	visible5 = visible4 + scale;
	value2 = value - visible3;
	if( value2 < 0 && value2 >= -scale/2 ) {
		top = qfalse;
		bottom = qtrue;
	} else if( value2 > 0 && value2 <= scale/2 ) {
		top = qtrue;
		bottom = qfalse;
	} else {
		top = bottom = qfalse;
	}
		// draw visible speed numbers
	CG_MFQ3HUD_Numbers( 12, 171+(value2*64/scale), 4, visible3, qfalse );
	if( visible2 >= 0 ) {
		value2 = value - visible2;
		CG_MFQ3HUD_Numbers( 12, 171+(value2*64/scale), 4, visible2, qfalse );
		if( bottom && visible1 >= 0 ) {
			value2 = value - visible1;
			CG_MFQ3HUD_Numbers( 12, 171+(value2*64/scale), 4, visible1, qfalse );
		}
	}
	value2 = value - visible4;
	CG_MFQ3HUD_Numbers( 12, 171+(value2*64/scale), 4, visible4, qfalse );
	if( top ) {
		value2 = value - visible5;
		CG_MFQ3HUD_Numbers( 12, 171+(value2*64/scale), 4, visible5, qfalse );
	}
		// gearspeed
	if( gearspeed > 0 ) {
		value2 = value - gearspeed;
		if( value2 > -(scale*2) && value2 < (scale*2) ) {
			x = 55;
			y = 171+(value2*64/scale);
			width = height = 8;
			CG_AdjustFrom640( &x, &y, &width, &height );
			trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDcaret_v_g_l );
		}
	}
		// stallspeed
	if( stallspeed > 0 ) {
		value2 = value - stallspeed;
		if( value2 > -(scale*2) && value2 < (scale*2) ) {
			x = 55;
			y = 171+(value2*64/scale);
			width = height = 8;
			CG_AdjustFrom640( &x, &y, &width, &height );
			trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDcaret_v_r_l );
		}
	}
}


/*
================
CG_Draw_HeadingTape

================
*/
static void CG_Draw_HeadingTape( int value, int targetheading ) {

	float		x, y, width, height;
	float		offset;
	int			value2, visible1, visible2, visible3;
	qboolean	left, right;

		// find offset on heading tape
	value2 = value - 35;
	value2 = value2%10;
	if( value2 < 0 ) value2 += 10;
	offset = 0.125f - 0.0125f*value2;
		// draw heading tape
	x = 96;
	y = 10;
	width = 448;
	height = 16;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0.125f-offset, 0, 1-offset, 1, cgs.media.HUDheading );
		// draw heading value box
	x = 304;
	y = 28;
	width = 32;
	height = 16;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDvaluebox );
		// draw heading
	CG_MFQ3HUD_Numbers( 307, 32, 3, value, qfalse );
		// find visible heading numbers
	visible2 = ((int)((value+15)/10));
	if( (visible2%3) ) {
		visible2--;
		if( (visible2%3) ) {
			visible2--;
		}
	}
	visible2 *= 10;
	if( visible2 <= 0 ) visible2 += 360;
	visible1 = visible2 - 30;
	if( visible1 <= 0 ) visible1 += 360;
	visible3 = visible2 + 30;
	if( visible3 > 360 ) visible3 -= 360;
	value2 = value - visible2;
	if( value2 < -180 ) value2 += 360;
	if( value2 < -5 ) {
		left = qtrue;
		right = qfalse;
	} else if( value2 > 5 ) {
		left = qfalse;
		right = qtrue;
	} else {
		right = left = qtrue;
	}
		// draw visible heading numbers
	CG_MFQ3HUD_Numbers( 307-(value2*64/10), 6, 3, visible2, qfalse );
	if( left ) {
		value2 = value - visible1;
		if( value2 < 0 ) value2 += 360;
		CG_MFQ3HUD_Numbers( 307-(value2*64/10), 6, 3, visible1, qfalse );
	}
	if( right ) {
		value2 = value - visible3;
		if( value2 > 0 ) value2 -= 360;
		CG_MFQ3HUD_Numbers( 307-(value2*64/10), 6, 3, visible3, qfalse );
	}

		// heading indicater
	x = 316;
	y = 24;
	width = height = 8;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDind_h );

		// heading caret
	value2 = value - targetheading;
	if( value2 < -180 ) value2 += 360;
	else if( value2 > 180 ) value2 -= 360;
	if( value2 < -35 ) value2 = -35;
	if( value2 > 35 ) value2 = 35;
	x = 316-(value2*64/10);
	y = 24;
	width = height = 8;
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDcaret_h );

}

/*
================
CG_DrawStatusBar_MFQ3

================
*/
static void CG_DrawStatusBar_MFQ3_new( void ) {
	centity_t	*cent;
	playerState_t	*ps;
	int			value;
	int			vehicle = cgs.clientinfo[cg.predictedPlayerState.clientNum].vehicle;
	
	static vec4_t colors[] = { 
//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
		{ 1.0f, 0.89f, 0.0f, 1.0f } ,		// normal, yellow
		{ 1.0f, 0.0f, 0.0f, 1.0f },		// low health, red
		{0.5f, 0.5f, 0.5f, 1.0f},			// weapon firing, grey
		{ 1.0f, 1.0f, 1.0f, 1.0f },		// health > 100, white
		{ 0.3f, 1.0f, 0.3f, 1.0f }		// green
	};

	if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	// heading
	if( hud_heading.integer ) {
		value = 360 - (int)ps->vehicleAngles[1];
		if( value <= 0 ) value += 360;
		CG_Draw_HeadingTape( value, 360 );
	}

	// speed
	if( hud_speed.integer ) {
		if( availableVehicles[vehicle].id&CAT_ANY & CAT_GROUND ) value = 10;
		else value = 50;
		CG_Draw_SpeedTape( ps->speed/10, availableVehicles[vehicle].stallspeed,
				availableVehicles[vehicle].stallspeed * SPEED_GREEN_ARC, value );
	}

	// altitude
	if( hud_altitude.integer ) {
		trace_t	tr;
		vec3_t	start, end;
		VectorCopy( ps->origin, start );
		start[2] += availableVehicles[vehicle].mins[2];
		VectorCopy( start, end );
		end[2] -= 20000;
		CG_Trace( &tr, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, MASK_SOLID|MASK_WATER );
		value = (int)(tr.fraction * 20000);
		CG_Draw_AltTape(value);
	}

	// mfd 1
	if( hud_mfd.integer ) {
		CG_Draw_MFD(MFD_1);
	}

	// mfd 2
	if( hud_mfd2.integer ) {
		CG_Draw_MFD(MFD_2);
	}

	// solid middle section
	value = (100*ps->stats[STAT_HEALTH]/ps->stats[STAT_MAX_HEALTH]);
	if( value > 100 ) value = 100;
	if( hud_center.integer ) {
		CG_Draw_Center(value, ps->throttle);
	}

	// additional HUD info: health and throttle (redundant)
	CG_Draw_Redundant(value, ps->throttle);
}

/*
================
CG_DrawStatusBar_MFQ3

================
*/
static void CG_DrawStatusBar_MFQ3( void ) {
	int			color;
	centity_t	*cent;
	playerState_t	*ps;
	int			value;
	int			vehicle = cgs.clientinfo[cg.predictedPlayerState.clientNum].vehicle;
	int			w;

	static vec4_t colors[] = { 
//		{ 0.2, 1.0, 0.2, 1.0 } , { 1.0, 0.2, 0.2, 1.0 }, {0.5, 0.5, 0.5, 1} };
		{ 1.0f, 0.89f, 0.0f, 1.0f } ,		// normal, yellow
		{ 1.0f, 0.0f, 0.0f, 1.0f },		// low health, red
		{0.5f, 0.5f, 0.5f, 1.0f},			// weapon firing, grey
		{ 1.0f, 1.0f, 1.0f, 1.0f },		// health > 100, white
		{ 0.3f, 1.0f, 0.3f, 1.0f }		// green
	};

	if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	// main background
	CG_DrawPic( 0, 430, 640, 50, cgs.media.HUDmain );
	
	//
	// flag
	//
	if( cg.predictedPlayerState.objectives & OB_REDFLAG ) {
		CG_DrawStatusBarFlag( 640 - ICON_SIZE, TEAM_RED );
	} else if( cg.predictedPlayerState.objectives & OB_BLUEFLAG ) {
		CG_DrawStatusBarFlag( 640 - ICON_SIZE, TEAM_BLUE );
	}

	//
	// RADAR
	//
	if( cent->currentState.ONOFF & OO_RADAR_AIR ) {
		CG_DrawRadarSymbols_AIR(vehicle);
	} else if( cent->currentState.ONOFF & OO_RADAR_GROUND ) {
		CG_DrawRadarSymbols_GROUND(vehicle);
	} else if( availableVehicles[vehicle].radarRange || 
		availableVehicles[vehicle].radarRange2 ) {
		CG_DrawPic( 440, 280, 200, 200, cgs.media.HUDradar );
		CG_DrawSmallString( 595, 320, "OFF", 1.0f);
	}

	//
	// GPS
	//
	if( cg.GPS ) {
		trace_t	tr;
		vec3_t	start, end;
		// main
		CG_DrawPic( -14, 380, 512, 102, cgs.media.HUDgps );
		// heading
		value = 360 - (int)ps->vehicleAngles[1];
		CG_DrawSmallString( 234, 420, va("%i",value), 1.0f );
		// altitude
		VectorCopy( ps->origin, start );
		start[2] += availableVehicles[vehicle].mins[2];
		VectorCopy( start, end );
		end[2] -= 2000;
		CG_Trace( &tr, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, MASK_SOLID|MASK_WATER );
		value = (int)(tr.fraction * 2000);
		if( value > 100 ) color = 3;
		else if( value > 50 ) color = 4;
		else if( value > 25 ) color = 0;
		else color = 1;
		if( value < 2000 ) {
			CG_DrawSmallStringColor( 374, 420, va("%i",value), colors[color] );
		} else {
			CG_DrawSmallStringColor( 374, 420, "XXXX", colors[color] );
		}

		// x coord
		value = (int)ps->origin[0];
		CG_DrawSmallString( 264, 423, va("%i",value), 0.7f );
		// y coord
		value = (int)ps->origin[1];
		CG_DrawSmallString( 320, 423, va("%i",value), 0.7f );
	} 

	//
	// INFO
	//
	if( !cg.INFO ) {	// bit dirty to have NOT but oh well...
		// main
		CG_DrawPic( 0, 330, 150, 150, cgs.media.HUDext );
		if( availableVehicles[vehicle].caps & HC_GEAR ) {
			if( cent->currentState.ONOFF & OO_GEAR ) {
				CG_DrawSmallStringColor( 2, 390, "Gear", colors[4] );
			} else {
				CG_DrawSmallString( 2, 390, "Gear", 1.0f );
			}
		}
		if( availableVehicles[vehicle].caps & HC_SPEEDBRAKE ) {
			if( cent->currentState.ONOFF & OO_SPEEDBRAKE ) {
				CG_DrawSmallStringColor( 2, 410, "Speedbrakes", colors[4] );
			} else {
				CG_DrawSmallString( 2, 410, "Speedbrakes", 1.0f );
			}
		} else {
			if( cent->currentState.ONOFF & OO_SPEEDBRAKE ) {
				CG_DrawSmallStringColor( 2, 410, "Brakes", colors[4] );
			} else {
				CG_DrawSmallString( 2, 410, "Brakes", 1.0f );
			}
		}

		value = ps->stats[STAT_FUEL];
		if ( value > 10 ) {
			color =  3;	// white
		} else if (value > 5) {
			color = 0;	// yellow
		} else {
			color = 1;	// red
		}
		if( (availableVehicles[vehicle].id&CAT_ANY) & CAT_PLANE ) {
			value *= 100;
		}
		CG_DrawSmallStringColor( 2, 430, va("Fuel %i", value), colors[color]);
	} 

	//
	// FLARES
	//
	if( ps->ammo[WP_FLARE+8] > 0 ) {
		CG_DrawBigString( 600, 462, va("%i", ps->ammo[WP_FLARE]), 1.0f );
	}

	//
	// speed and throttle
	//
	value = ps->speed/10;
	if ( value > availableVehicles[vehicle].stallspeed * SPEED_GREEN_ARC ) {
		color = 3;		// white
	} else if (value > availableVehicles[vehicle].stallspeed * SPEED_YELLOW_ARC ) {
		color = 4;	// green
	} else if (value >= availableVehicles[vehicle].stallspeed) {
		color = 0;	// yellow
	} else {
		color = 1;	// red
	}

	w = CG_DrawStrlen(va("%i",value)) * BIGCHAR_WIDTH;
	CG_DrawBigStringColor(285-w, 460, va("%i",value), colors[color]);
	
	if( ps->throttle > availableVehicles[vehicle].maxthrottle ) {
		value = ps->throttle - availableVehicles[vehicle].maxthrottle;
	} else {
		value = ps->throttle;
	}
	CG_DrawPic(295, 443, 50, 37, cgs.media.throttle[value]);

	//
	// health
	//
	value = (100*ps->stats[STAT_HEALTH]/ps->stats[STAT_MAX_HEALTH]);
	if( value > 100 ) value = 100;
	if ( value < 10 ) {
		color = 1;
	} else if (value < 25) {
		color = 0;  
	} else {
		color = 3;
	}

	w = CG_DrawStrlen(va("%i",value)) * BIGCHAR_WIDTH;
	CG_DrawBigStringColor( 398-w, 460, va("%i",value), colors[color]);

	//
	// ammo
	//
	if( cent->currentState.weaponNum >= 0 ) {
		value = ps->ammo[cent->currentState.weaponNum];
		w = CG_DrawStrlen(va("%i",value)) * BIGCHAR_WIDTH;
		CG_DrawBigString (500-w, 460, va("%i",value),1.0f);
		CG_DrawPic( 416, 440, 24, 45, 
			availableWeapons[availableVehicles[vehicle].weapons[cent->currentState.weaponNum]].iconHandle );
	}

	//
	// ammo mg
	//
	if( ps->ammo[WP_MACHINEGUN+8] > 0 ) {
		if( cent->currentState.weaponNum >= 0 ) {
			value = ps->ammo[0];
			w = CG_DrawStrlen(va("%i",value)) * BIGCHAR_WIDTH;
			CG_DrawBigString (180-w, 460, va("%i",value),1.0f);
			CG_DrawPic( 200, 440, 24, 45, availableWeapons[availableVehicles[vehicle].weapons[0]].iconHandle );
		}
	}
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

	CG_DrawBigString( 635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
CG_CreateColour
==================
*/

static vec4_t colourVector = {1,1,1,1};	// common return ptr

vec4_t * CG_CreateColour( float r, float g, float b, float a )
{
	// assign
	colourVector[0] = r;
	colourVector[1] = g;
	colourVector[2] = b;
	colourVector[3] = a;

	// return common ptr
	return &colourVector;
}

/*
==================
CG_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float CG_DrawFPS( float y ) {
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
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

		s = va( "%ifps", fps );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

#ifdef _MENU_SCOREBOARD
		CG_DrawStringNew( 635, y + 2, 0.5f, *CG_CreateColour(1,1,1,1), s, 0, 0, 3, RIGHT_JUSTIFY );
#else
		CG_DrawBigString( 635 - w, y + 2, s, 1.0f );
#endif
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
=================
CG_DrawTimer
=================
*/
static float CG_DrawTimer( float y ) {
	char		*s;
	int			w;
	int			mins, seconds, tens;
	int			msec;

	msec = cg.time - cgs.levelStartTime;

	seconds = msec / 1000;
	mins = seconds / 60;
	seconds -= mins * 60;
	tens = seconds / 10;
	seconds -= tens * 10;

	s = va( "%i:%i%i", mins, tens, seconds );
	w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH;

	CG_DrawBigString( 635 - w, y + 2, s, 1.0F);

	return y + BIGCHAR_HEIGHT + 4;
}


/*
=================
CG_DrawTeamOverlay
=================
*/

static float CG_DrawTeamOverlay( float y, qboolean right, qboolean upper ) {
	int x, w, h, xx;
	int i, j, len;
	const char *p;
	vec4_t		hcolor;
	int pwidth, lwidth;
	int plyrs;
	char st[16];
	clientInfo_t *ci;
	gitem_t	*item;
	int ret_y, count;

	if ( !cg_drawTeamOverlay.integer ) {
		return y;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] != TEAM_RED && cg.snap->ps.persistant[PERS_TEAM] != TEAM_BLUE ) {
		return y; // Not on any team
	}

	plyrs = 0;

	// max player name width
	pwidth = 0;
	count = (numSortedTeamPlayers > 8) ? 8 : numSortedTeamPlayers;
	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {
			plyrs++;
			len = CG_DrawStrlen(ci->name);
			if (len > pwidth)
				pwidth = len;
		}
	}

	if (!plyrs)
		return y;

	if (pwidth > TEAM_OVERLAY_MAXNAME_WIDTH)
		pwidth = TEAM_OVERLAY_MAXNAME_WIDTH;

	// max location name width
	lwidth = 0;
	for (i = 1; i < MAX_LOCATIONS; i++) {
		p = CG_ConfigString(CS_LOCATIONS + i);
		if (p && *p) {
			len = CG_DrawStrlen(p);
			if (len > lwidth)
				lwidth = len;
		}
	}

	if (lwidth > TEAM_OVERLAY_MAXLOCATION_WIDTH)
		lwidth = TEAM_OVERLAY_MAXLOCATION_WIDTH;

	w = (pwidth + lwidth + 4 + 7) * TINYCHAR_WIDTH;

	if ( right )
		x = 640 - w;
	else
		x = 0;

	h = plyrs * TINYCHAR_HEIGHT;

	if ( upper ) {
		ret_y = y + h;
	} else {
		y -= h;
		ret_y = y;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
		hcolor[3] = 0.33f;
	} else { // if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE )
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
		hcolor[3] = 0.33f;
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );

	for (i = 0; i < count; i++) {
		ci = cgs.clientinfo + sortedTeamPlayers[i];
		if ( ci->infoValid && ci->team == cg.snap->ps.persistant[PERS_TEAM]) {

			hcolor[0] = hcolor[1] = hcolor[2] = hcolor[3] = 1.0;

			xx = x + TINYCHAR_WIDTH;

			CG_DrawStringExt( xx, y,
				ci->name, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, TEAM_OVERLAY_MAXNAME_WIDTH);

			if (lwidth) {
				p = CG_ConfigString(CS_LOCATIONS + ci->location);
				if (!p || !*p)
					p = "unknown";
				len = CG_DrawStrlen(p);
				if (len > lwidth)
					len = lwidth;

//				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth + 
//					((lwidth/2 - len/2) * TINYCHAR_WIDTH);
				xx = x + TINYCHAR_WIDTH * 2 + TINYCHAR_WIDTH * pwidth;
				CG_DrawStringExt( xx, y,
					p, hcolor, qfalse, qfalse, TINYCHAR_WIDTH, TINYCHAR_HEIGHT,
					TEAM_OVERLAY_MAXLOCATION_WIDTH);
			}

			Com_sprintf (st, sizeof(st), "%3i", ci->health);

			xx = x + TINYCHAR_WIDTH * 3 + 
				TINYCHAR_WIDTH * pwidth + TINYCHAR_WIDTH * lwidth;

			CG_DrawStringExt( xx, y,
				st, hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );

			// Draw powerup icons
			if (right) {
				xx = x;
			} else {
				xx = x + w - TINYCHAR_WIDTH;
			}
			for (j = OB_REDFLAG; j <= OB_BLUEFLAG; j<<=1) {
				if( ci->objectives & j ) {

					item = BG_FindItemForPowerup( j );

					if (item) {
						CG_DrawPic( xx, y, TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 
						trap_R_RegisterShader( item->icon ) );
						if (right) {
							xx -= TINYCHAR_WIDTH;
						} else {
							xx += TINYCHAR_WIDTH;
						}
					}
				}
			}

			y += TINYCHAR_HEIGHT;
		}
	}

	return ret_y;
//#endif
}

#ifdef _MENU_SCOREBOARD

/*
=====================
CG_DrawUpperLeft

=====================
*/
static void CG_DrawUpperLeft( void )
{
	rectDef_t fillRect;

	// draw the faded vertical gradient
	fillRect.x = 0;
	fillRect.y = 0;
	fillRect.w = 640;
	fillRect.h = 48;

/*
	// NOTE: make this activatable by a cg_x variable - MM
	VerticalGradient_Paint( &fillRect, *CG_CreateColour( 0, 0, 1, 0.50f ) );
*/
}

#endif

/*
=====================
CG_DrawUpperRight

=====================
*/
static void CG_DrawUpperRight( void ) {
	float	y;

	y = 0;

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 1 ) {
		y = CG_DrawTeamOverlay( y, qtrue, qtrue );
	} 
	if ( cg_drawSnapshot.integer ) {
		y = CG_DrawSnapshot( y );
	}
	if ( cg_drawFPS.integer ) {
		y = CG_DrawFPS( y );
	}
	if ( cg_drawTimer.integer ) {
		y = CG_DrawTimer( y );
	}

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
static float CG_DrawScores( float y ) {
	const char	*s;
	int			s1, s2, score;
	int			x, w;
	int			v;
	vec4_t		color;
	float		y1;
	gitem_t		*item;

	s1 = cgs.scores1;
	s2 = cgs.scores2;

	y -=  BIGCHAR_HEIGHT + 8;

	y1 = y;

	// draw from the right side to left
	if ( cgs.gametype >= GT_TEAM ) {
		x = 640;

		color[0] = 0;
		color[1] = 0;
		color[2] = 1;
		color[3] = 0.33f;
		s = va( "%2i", s2 );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 4, y, s, 1.0F);

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( OB_BLUEFLAG );

			if (item) {
				y1 = y - BIGCHAR_HEIGHT - 8;
				if( cgs.blueflag >= 0 && cgs.blueflag <= 2 ) {
					CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.blueFlagShader[cgs.blueflag] );
				}
			}
		}

		color[0] = 1;
		color[1] = 0;
		color[2] = 0;
		color[3] = 0.33f;
		s = va( "%2i", s1 );
		w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
		x -= w;
		CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
			CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
		}
		CG_DrawBigString( x + 4, y, s, 1.0F);

		if ( cgs.gametype == GT_CTF ) {
			// Display flag status
			item = BG_FindItemForPowerup( OB_REDFLAG );

			if (item) {
				y1 = y - BIGCHAR_HEIGHT - 8;
				if( cgs.redflag >= 0 && cgs.redflag <= 2 ) {
					CG_DrawPic( x, y1-4, w, BIGCHAR_HEIGHT+8, cgs.media.redFlagShader[cgs.redflag] );
				}
			}
		}

		if ( cgs.gametype >= GT_CTF ) {
			v = cgs.capturelimit;
		} else {
			v = cgs.fraglimit;
		}
		if ( v ) {
			s = va( "%2i", v );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

	} else {
		qboolean	spectator;

		x = 640;
		score = cg.snap->ps.persistant[PERS_SCORE];
		spectator = ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR );

		// always show your score in the second box if not in first place
		if ( s1 != score ) {
			s2 = score;
		}
		if ( s2 != SCORE_NOT_PRESENT ) {
			s = va( "%2i", s2 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			if ( !spectator && score == s2 && score != s1 ) {
				color[0] = 1;
				color[1] = 0;
				color[2] = 0;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
				CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
			}	
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

		// first place
		if ( s1 != SCORE_NOT_PRESENT ) {
			s = va( "%2i", s1 );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			if ( !spectator && score == s1 ) {
				color[0] = 0;
				color[1] = 0;
				color[2] = 1;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
				CG_DrawPic( x, y-4, w, BIGCHAR_HEIGHT+8, cgs.media.selectShader );
			} else {
				color[0] = 0.5f;
				color[1] = 0.5f;
				color[2] = 0.5f;
				color[3] = 0.33f;
				CG_FillRect( x, y-4,  w, BIGCHAR_HEIGHT+8, color );
			}	
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

		if ( cgs.fraglimit ) {
			s = va( "%2i", cgs.fraglimit );
			w = CG_DrawStrlen( s ) * BIGCHAR_WIDTH + 8;
			x -= w;
			CG_DrawBigString( x + 4, y, s, 1.0F);
		}

	}

	return y1 - 8;
}

/*
===================
CG_DrawPickupItem
===================
*/
static int CG_DrawPickupItem( int y ) {
	int		value;
	float	*fadeColor;

	if ( cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
		return y;
	}

	y -= ICON_SIZE;

	value = cg.itemPickup;
	if ( value ) {
		fadeColor = CG_FadeColor( cg.itemPickupTime, 3000 );
		if ( fadeColor ) {
			CG_RegisterItemVisuals( value );
			trap_R_SetColor( fadeColor );
			CG_DrawPic( 8, y, ICON_SIZE, ICON_SIZE, cg_items[ value ].icon );
			CG_DrawBigString( ICON_SIZE + 16, y + (ICON_SIZE/2 - BIGCHAR_HEIGHT/2), bg_itemlist[ value ].pickup_name, fadeColor[0] );
			trap_R_SetColor( NULL );
		}
	}
	
	return y;
}

/*
=====================
CG_DrawLowerLeft

=====================
*/
static void CG_DrawLowerLeft( void ) {
	float	y;

	y = 480 - ICON_SIZE;

	if ( cgs.gametype >= GT_TEAM && cg_drawTeamOverlay.integer == 3 ) {
		y = CG_DrawTeamOverlay( y, qfalse, qfalse );
	} 


	y = CG_DrawPickupItem( y );
}


//===========================================================================================

/*
=================
CG_DrawTeamInfo
=================
*/
static void CG_DrawTeamInfo( void ) {
	int w, h;
	int i, len;
	vec4_t		hcolor;
	int		chatHeight;

#define CHATLOC_Y 420 // bottom end
#define CHATLOC_X 0

	if (cg_teamChatHeight.integer < TEAMCHAT_HEIGHT)
		chatHeight = cg_teamChatHeight.integer;
	else
		chatHeight = TEAMCHAT_HEIGHT;
	if (chatHeight <= 0)
		return; // disabled

	if (cgs.teamLastChatPos != cgs.teamChatPos) {
		if (cg.time - cgs.teamChatMsgTimes[cgs.teamLastChatPos % chatHeight] > cg_teamChatTime.integer) {
			cgs.teamLastChatPos++;
		}

		h = (cgs.teamChatPos - cgs.teamLastChatPos) * TINYCHAR_HEIGHT;

		w = 0;

		for (i = cgs.teamLastChatPos; i < cgs.teamChatPos; i++) {
			len = CG_DrawStrlen(cgs.teamChatMsgs[i % chatHeight]);
			if (len > w)
				w = len;
		}
		w *= TINYCHAR_WIDTH;
		w += TINYCHAR_WIDTH * 2;

		if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_RED ) {
			hcolor[0] = 1;
			hcolor[1] = 0;
			hcolor[2] = 0;
			hcolor[3] = 0.33f;
		} else if ( cg.snap->ps.persistant[PERS_TEAM] == TEAM_BLUE ) {
			hcolor[0] = 0;
			hcolor[1] = 0;
			hcolor[2] = 1;
			hcolor[3] = 0.33f;
		} else {
			hcolor[0] = 0;
			hcolor[1] = 1;
			hcolor[2] = 0;
			hcolor[3] = 0.33f;
		}

		trap_R_SetColor( hcolor );
		CG_DrawPic( CHATLOC_X, CHATLOC_Y - h, 640, h, cgs.media.teamStatusBar );
		trap_R_SetColor( NULL );

		hcolor[0] = hcolor[1] = hcolor[2] = 1.0;
		hcolor[3] = 1.0;

		for (i = cgs.teamChatPos - 1; i >= cgs.teamLastChatPos; i--) {
			CG_DrawStringExt( CHATLOC_X + TINYCHAR_WIDTH, 
				CHATLOC_Y - (cgs.teamChatPos - i)*TINYCHAR_HEIGHT, 
				cgs.teamChatMsgs[i % chatHeight], hcolor, qfalse, qfalse,
				TINYCHAR_WIDTH, TINYCHAR_HEIGHT, 0 );
		}
	}
}


/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward( void ) { 
	float	*color;
	int		i, count;
	float	x, y;
	char	buf[32];

	if ( !cg_drawRewards.integer ) {
		return;
	}

	color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
	if ( !color ) {
		if (cg.rewardStack > 0) {
			for(i = 0; i < cg.rewardStack; i++) {
				cg.rewardSound[i] = cg.rewardSound[i+1];
				cg.rewardShader[i] = cg.rewardShader[i+1];
				cg.rewardCount[i] = cg.rewardCount[i+1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		} else {
			return;
		}
	}

	trap_R_SetColor( color );

	if ( cg.rewardCount[0] >= 10 ) {
		y = 56;
		x = 320 - ICON_SIZE/2;
		CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
		x = ( SCREEN_WIDTH - SMALLCHAR_WIDTH * CG_DrawStrlen( buf ) ) / 2;
		CG_DrawStringExt( x, y+ICON_SIZE, buf, color, qfalse, qtrue,
								SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, 0 );
	}
	else {

		count = cg.rewardCount[0];

		y = 56;
		x = 320 - count * ICON_SIZE/2;
		for ( i = 0 ; i < count ; i++ ) {
			CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
			x += ICON_SIZE;
		}
	}
	trap_R_SetColor( NULL );
}


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

#ifdef _MENU_SCOREBOARD
	CG_DrawStringNewAlpha( 320, 100, s, 1.0f, CENTRE_JUSTIFY );
#else
	CG_DrawBigString( 320 - w/2, 100, s, 1.0F);
#endif

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
static void CG_DrawLagometer( void ) {
	int		a, x, y, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;

	if ( !cg_lagometer.integer || cgs.localServer ) {
		CG_DrawDisconnect();
		return;
	}

	//
	// draw the graph
	//
	x = 640 - 48;
	y = 480 - 48;

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

#ifdef _MENU_SCOREBOARD
		CG_DrawStringNewColour( 320, y, linebuffer, color, CENTRE_JUSTIFY );
#else
		CG_DrawStringExt( x, y, linebuffer, color, qfalse, qtrue,
			cg.centerPrintCharWidth, (int)(cg.centerPrintCharWidth * 1.5), 0 );
#endif
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
	if ( !color ) {
		trap_R_SetColor( NULL );
		return;
	}

	name = cgs.clientinfo[ cg.crosshairClientNum ].name;
	w = CG_DrawStrlen( name ) * BIGCHAR_WIDTH;

#ifdef _MENU_SCOREBOARD
	CG_DrawStringNewAlpha( 320, 170, name, color[3] * 0.5f, CENTRE_JUSTIFY );
#else
	CG_DrawBigString( 320 - w / 2, 170, name, color[3] * 0.5f );
#endif

	trap_R_SetColor( NULL );
}


//==============================================================================

/*
=================
CG_DrawSpectator
=================
*/
static void CG_DrawSpectator(void) {

#ifdef _MENU_SCOREBOARD
	// this is where we load the spectator menu script
	if( menuSpectator == NULL)
	{
		menuSpectator = Menus_FindByName("spectator_menu");
	}

	// spectator menu loaded?
	if( menuSpectator )
	{
		// decide what to place into the three textual placeholders in the spectator menu
		
		// still a spectator?
		if( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR )
		{
			trap_Cvar_Set( "ui_spectatorTitleTxt", "Spectator" );
			trap_Cvar_Set( "ui_spectatorLine1Txt", "Press FIRE to switch view." );
			
			// team game?
			if ( cgs.gametype >= GT_TEAM )
				trap_Cvar_Set( "ui_spectatorLine2Txt", "Choose a team from the 'Team' dialog to play." );
			else
				trap_Cvar_Set( "ui_spectatorLine2Txt", "Select 'Join' from the 'Team' dialog to play." );
		}
		else
		{
			// we have a team (or don't need one) but no vehicle
			trap_Cvar_Set( "ui_spectatorTitleTxt", "Limbo" );
			trap_Cvar_Set( "ui_spectatorLine1Txt", "Choose a vehicle to join in the game or" );
			trap_Cvar_Set( "ui_spectatorLine2Txt", "select 'Specator' from the 'Team' dialog." );
		}

		// draw
		Menu_Paint( menuSpectator, qtrue );
	}
#else
	// old style
	CG_DrawBigString(320 - 9 * 8, 440, "SPECTATOR", 1.0F);
	if ( cgs.gametype == GT_TOURNAMENT ) {
		CG_DrawBigString(320 - 15 * 8, 460, "waiting to play", 1.0F);
	}
	else if ( cgs.gametype >= GT_TEAM ) {
		CG_DrawBigString(320 - 39 * 8, 460, "Use the TEAM SELECT ", 1.0F);
	}
#endif
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
		cgs.voteModified = qfalse;
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
		cgs.teamVoteModified[cs_offset] = qfalse;
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
static qboolean CG_DrawScoreboard() {
#ifdef _MENU_SCOREBOARD
	static qboolean firstTime = qtrue;
	float fade, *fadeColor;

	if (menuScoreboard) {
		menuScoreboard->window.flags &= ~WINDOW_FORCED;
	}

	if (cg_paused.integer) {
		cg.deferredPlayerLoading = 0;
		firstTime = qtrue;
		return qfalse;
	}

	// should never happen in Team Arena
	if (cgs.gametype == GT_SINGLE_PLAYER && cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		cg.deferredPlayerLoading = 0;
		firstTime = qtrue;
		return qfalse;
	}

	// don't draw scoreboard during death while warmup up
	if ( cg.warmup && !cg.showScores ) {
		return qfalse;
	}

	if ( cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD || cg.predictedPlayerState.pm_type == PM_INTERMISSION ) {
		fade = 1.0;
		fadeColor = colorWhite;
	} else {
		fadeColor = CG_FadeColor( cg.scoreFadeTime, FADE_TIME );
		if ( !fadeColor ) {
			// next time scoreboard comes up, don't print killer
			cg.deferredPlayerLoading = 0;
			cg.killerName[0] = 0;
			firstTime = qtrue;
			return qfalse;
		}
		fade = *fadeColor;
	}																					  

	// this is where we load the correct scoreboard script
	if (menuScoreboard == NULL) {
		if ( cgs.gametype >= GT_TEAM ) {
			menuScoreboard = Menus_FindByName("teamscore_menu");
		} else {
			menuScoreboard = Menus_FindByName("score_menu");
		}
	}

	if (menuScoreboard) {
		if (firstTime) {
			CG_SetScoreSelection(menuScoreboard);
			firstTime = qfalse;
		}

		// draw
		Menu_Paint(menuScoreboard, qtrue);
	}

	// load any models that have been deferred
	if ( ++cg.deferredPlayerLoading > 10 ) {
//		CG_LoadDeferredPlayers();	// CG_LoadDeferredPlayers - not yet implemented - MM
	}

	return qtrue;
#else
	return CG_DrawOldScoreboard();
#endif
}

/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
	if ( cgs.gametype == GT_SINGLE_PLAYER ) {
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
static qboolean CG_DrawFollow( void )
{
	float		x = 0;
	vec4_t		*pColor = NULL;
	vec4_t		color = {0,0,0,0};
	const char	*name = NULL;

	// are we NOT following?
	if( !(cg.snap->ps.pm_flags & PMF_FOLLOW) )
	{
		return qfalse;
	}

#ifdef _MENU_SCOREBOARD
	// new style

	// draw label
	CG_DrawStringNewAlpha( 320, 48, "following ", 1.0f, RIGHT_JUSTIFY );

	// default colour
	pColor = CG_CreateColour( 0.0f, 0.75f, 0.0f, 1.0f );	// default: green

	// team colour?
	if( cgs.clientinfo[ cg.snap->ps.clientNum ].team == 1 )
	{
		pColor = CG_CreateColour( 0.75f, 0.0f, 0.0f, 1.0f );	// team: red
	}
	else if( cgs.clientinfo[ cg.snap->ps.clientNum ].team == 2 )
	{
		pColor = CG_CreateColour( 0.0f, 0.0f, 0.75f, 1.0f );	// team: blue
	}

	// get name of player being followed
	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	// draw name
	CG_DrawStringNewColour( 320, 48, name, pColor[0], LEFT_JUSTIFY );
#else
	// old style
	CG_DrawBigString( 320 - 9 * 8, 24, "following", 1.0F );

	name = cgs.clientinfo[ cg.snap->ps.clientNum ].name;

	x = 0.5 * ( 640 - GIANT_WIDTH * CG_DrawStrlen( name ) );

	color[0] = 1;
	color[1] = 1;
	color[2] = 1;
	color[3] = 1;
	CG_DrawStringExt( x, 40, name, color, qtrue, qtrue, GIANT_WIDTH, GIANT_HEIGHT, 0 );
#endif
	return qtrue;
}

#ifdef _MENU_SCOREBOARD

/*
=================
CG_DrawStringNewAlpha
=================
*/

void CG_DrawStringNewAlpha( int x, int y, const char * pText, float alpha, textJustify_t formatting )
{
	// just use default extra parameters
	CG_DrawStringNew( x, y, 0.5f, *CG_CreateColour(1,1,1,alpha), pText, 0, 0, 3, formatting );
}

/*
=================
CG_DrawStringNewColour
=================
*/

void CG_DrawStringNewColour( int x, int y, const char * pText, vec4_t colour, textJustify_t formatting )
{
	// just use default extra parameters
	CG_DrawStringNew( x, y, 0.5f, colour, pText, 0, 0, 3, formatting );
}

/*
=================
CG_DrawStringNew
=================
*/

void CG_DrawStringNew( int x, int y, float scale, vec4_t colour, const char * pText, int hSpacing, int numChars, int style, textJustify_t formatting )
{
	int xPos = x;
	int yPos = y;
	int textWidth = 0;
	int textHeight = 0;

	// get the width and height of this text
	textWidth = DC->textWidth( pText, scale, 0 );
	textHeight = DC->textHeight( pText, scale, 0 );

	// ->drawText() is draw from the "bottom up"
	yPos += textHeight;

	// reformat?
	switch( formatting )
	{
	case LEFT_JUSTIFY:
		break;

	case CENTRE_JUSTIFY:
		xPos -= (textWidth/2);
		break;

	case RIGHT_JUSTIFY:
		xPos -= textWidth;
		break;
	}

	// draw
	DC->drawText( xPos, yPos, scale, colour, pText, hSpacing, numChars, style );
}

#endif

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

#ifdef _MENU_SCOREBOARD
		CG_DrawStringNew( 320, 64, 0.5f, *CG_CreateColour(1,1,1,1), s, 0, 0, 3, CENTRE_JUSTIFY );
#else
		CG_DrawBigString( 320 - w / 2, 24, s, 1.0F );
#endif
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
					qfalse, qtrue, cw, (int)(cw * 1.5f), 0 );
		}
	} else {
		if ( cgs.gametype == GT_FFA ) {
			s = "Free For All";
		} else if ( cgs.gametype == GT_TEAM ) {
			s = "Team Deathmatch";
		} else if ( cgs.gametype == GT_CTF ) {
			s = "Capture the Flag";
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
				qfalse, qtrue, cw, (int)(cw * 1.1), 0 );
	}

	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}
	s = va( "Starts in: %i", sec + 1 );
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
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
			qfalse, qtrue, cw, (int)(cw * 1.5), 0 );
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
	if ( cg.levelShot ) {
		return;
	}

	if ( cg_draw2D.integer == 0 ) {
		return;
	}

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		CG_DrawIntermission();
		return;
	}

	// are we in spectator mode, or awaiting vehicle selection?
	if( cg.snap->ps.persistant[PERS_TEAM] == TEAM_SPECTATOR ||
		CG_Cvar_Get( "cg_nextVehicle" ) == -1 )
	{
		CG_DrawSpectator();
//		CG_DrawCrosshair();
//		CG_DrawCrosshairNames();
	}
	else
	{
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if ( !cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0 &&
			 !(cg.snap->ps.pm_flags & PMF_VEHICLESELECT) ) {

//			CG_DrawStatusBar_MFQ3();
      		CG_DrawStatusBar_MFQ3_new();
//			CG_DrawCrosshair();
			CG_DrawCrosshairNames();
//			CG_DrawReward();
		}
    
	}

	CG_DrawVote();
	CG_DrawTeamVote();

	CG_DrawLagometer();

#ifdef _MENU_SCOREBOARD
	CG_DrawUpperLeft();
#endif
	CG_DrawUpperRight();

	if( !CG_DrawFollow() )
	{
		CG_DrawWarmup();
	}

	// don't draw center string if scoreboard is up
	cg.scoreBoardShowing = CG_DrawScoreboard();
	if( !cg.scoreBoardShowing )
	{
		CG_DrawCenterString();
	}
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
	if ( !cg.snap ) {
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
		CG_Error( "CG_DrawActive: Undefined stereoView" );
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

#ifdef _MENU_SCOREBOARD	// NOTE: functions copied from v1.29h code to enable new scoreboard method

/*
=====================
CG_GetGameStatusText
=====================
*/
const char *CG_GetGameStatusText()
{
	const char *s = "";
	if ( cgs.gametype < GT_TEAM) {
		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
			s = va("%s place with %i",CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),cg.snap->ps.persistant[PERS_SCORE] );
		}
	} else {
		if ( cg.teamScores[0] == cg.teamScores[1] ) {
			s = va("Teams are tied at %i", cg.teamScores[0] );
		} else if ( cg.teamScores[0] >= cg.teamScores[1] ) {
			s = va("Red leads Blue, %i to %i", cg.teamScores[0], cg.teamScores[1] );
		} else {
			s = va("Blue leads Red, %i to %i", cg.teamScores[1], cg.teamScores[0] );
		}
	}
	return s;
}
	
/*
=====================
CG_GameTypeString
=====================
*/
const char *CG_GameTypeString() {
	if ( cgs.gametype == GT_FFA ) {
		return "Deathmatch";
	} else if ( cgs.gametype == GT_TEAM ) {
		return "Team Deathmatch";
	} else if ( cgs.gametype == GT_CTF ) {
		return "Capture the Flag";
	} /* else if ( cgs.gametype == GT_1FCTF ) {
		return "One Flag CTF";
	} else if ( cgs.gametype == GT_OBELISK ) {
		return "Overload";
	} else if ( cgs.gametype == GT_HARVESTER ) {
		return "Harvester";
	} */
	return "";
}

/*
=====================
CG_DrawGameType
=====================
*/
static void CG_DrawGameType(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle )
{
	CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, CG_GameTypeString(), 0, 0, textStyle);
}

/*
=====================
CG_DrawGameStatus
=====================
*/
static void CG_DrawGameStatus(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle )
{
	CG_Text_Paint(rect->x, rect->y + rect->h, scale, color, CG_GetGameStatusText(), 0, 0, textStyle);
}

/*
=====================
CG_OwnerDraw
=====================
*/
void CG_OwnerDraw( float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle )
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

  switch (ownerDraw)
  {
/*
  case CG_PLAYER_ARMOR_ICON:
    CG_DrawPlayerArmorIcon(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_ARMOR_ICON2D:
    CG_DrawPlayerArmorIcon(&rect, qtrue);
    break;
  case CG_PLAYER_ARMOR_VALUE:
    CG_DrawPlayerArmorValue(&rect, scale, color, shader, textStyle);
    break;
  case CG_PLAYER_AMMO_ICON:
    CG_DrawPlayerAmmoIcon(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
    break;
  case CG_PLAYER_AMMO_ICON2D:
    CG_DrawPlayerAmmoIcon(&rect, qtrue);
    break;
  case CG_PLAYER_AMMO_VALUE:
    CG_DrawPlayerAmmoValue(&rect, scale, color, shader, textStyle);
    break;
  case CG_SELECTEDPLAYER_HEAD:
    CG_DrawSelectedPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY, qfalse);
    break;
  case CG_VOICE_HEAD:
    CG_DrawSelectedPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY, qtrue);
    break;
  case CG_VOICE_NAME:
    CG_DrawSelectedPlayerName(&rect, scale, color, qtrue, textStyle);
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
    CG_DrawSelectedPlayerName(&rect, scale, color, qfalse, textStyle);
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
    CG_HarvesterSkulls(&rect, scale, color, qfalse, textStyle);
    break;
  case CG_HARVESTER_SKULLS2D:
    CG_HarvesterSkulls(&rect, scale, color, qtrue, textStyle);
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
    CG_DrawPlayerHasFlag(&rect, qfalse);
    break;
  case CG_PLAYER_HASFLAG2D:
    CG_DrawPlayerHasFlag(&rect, qtrue);
    break;
  case CG_AREA_SYSTEMCHAT:
    CG_DrawAreaSystemChat(&rect, scale, color, shader);
    break;
  case CG_AREA_TEAMCHAT:
    CG_DrawAreaTeamChat(&rect, scale, color, shader);
    break;
  case CG_AREA_CHAT:
    CG_DrawAreaChat(&rect, scale, color, shader);
    break;
*/
  case CG_GAME_TYPE:
    CG_DrawGameType(&rect, scale, color, shader, textStyle);
    break;
  case CG_GAME_STATUS:
    CG_DrawGameStatus(&rect, scale, color, shader, textStyle);
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

#endif	// NOTE: functions copied from v1.29h code to enable new scoreboard method
