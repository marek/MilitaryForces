/*
 * $Id: cg_drawnewhud.c,v 1.1 2002-01-25 13:26:52 thebjoern Exp $
*/

#include "cg_local.h"


// there are a couple of duplicate functions here
// this should be cleaned up and factorized when there is some time left..

/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
static void CG_DrawChar_MFQ3( int x, int y, int width, int height, int ch ) {
	int		col;
	float	fcol;
	float	size;
	float	ax, ay, aw, ah;

	if( ch == '?' ) {
		col = 26;
	} else if( ch == '!' ) {
		col = 27;
	} else if( ch == '*' ) {
		col = 28;
	} else if( ch == '-' ) {
		col = 29;
	} else if( ch == ':' ) {
		col = 30;
	} else if( ch == '/' ) {
		col = 31;
	} else if( Q_isalpha(ch) ) { 
		col = ch-65;
	} else return;

	ch &= 255;

	if ( ch == ' ' ) {
		return;
	}

	ax = x;
	ay = y;
	aw = width;
	ah = height;
	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	size = 0.03125;
	fcol = col*size;

	trap_R_DrawStretchPic( ax, ay, aw, ah,
					   fcol, 0, 
					   fcol + size, 1, 
					   cgs.media.HUDalphabet );
}

/*
==================
CG_DrawStringExt

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
static void CG_DrawString_Ext_MFQ3( int x, int y, const char *string, const float *setColor, 
		qboolean forceColor, int charWidth, int charHeight, int maxChars ) {
	vec4_t		color;
	const char	*s;
	int			xx;
	int			cnt;

	if (maxChars <= 0)
		maxChars = 32767; // do them all!

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
		CG_DrawChar_MFQ3( xx, y, charWidth, charHeight, *s );
		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor( NULL );
}



static void CG_DrawString_MFQ3( int x, int y, const char *s, float alpha ) {
	float	color[4];

	color[0] = color[1] = color[2] = 1.0;
	color[3] = alpha;

	Q_strupr( (char*)s );

	CG_DrawString_Ext_MFQ3( x, y, s, color, qfalse, HUDNUM_WIDTH, HUDNUM_HEIGHT, 0 );
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
		CG_DrawPic( x,y, 2*HUDNUM_WIDTH, HUDNUM_HEIGHT, cgs.media.HUDnumbers[STAT_PERCENT] );
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
CG_DrawRadarSymbols

================
*/
static void CG_DrawRadarSymbols_AIR_new( int vehicle, float range, int x, int y ) {
	unsigned int i;
	centity_t	*self = &cg.predictedPlayerEntity;
	vec3_t		pos, pos1, pos2;
	vec3_t		dir;
	float		dist;
	float		scale;
	float		hdg = self->currentState.angles[1];
	float		angle;
	vec3_t		angles;
	int			otherveh;
	vec3_t		otherpos;
	int			icon;
	int			drawnTargets = 0;
	trace_t		res;

	// current range
	if( cg.RADARRangeSetting ) {
		range /= (2 * cg.RADARRangeSetting);
	}

	// adjust for radar screen size
	scale = range/54;

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
		CG_DrawPic( x + dir[0], y - dir[1], 8, 8, cgs.media.radarIcons[icon] );
		// check if we should draw any more targets
		drawnTargets++;
		if( drawnTargets >= cg_radarTargets.integer ) break;
	}
}

static void CG_DrawRadarSymbols_GROUND_new( int vehicle, float range, int x, int y ) {
	unsigned int i;
	centity_t	*self = &cg.predictedPlayerEntity;
	vec3_t		pos, pos1, pos2;
	vec3_t		dir;
	float		dist;
	float		scale;
	float		hdg = self->currentState.angles[1];
	float		angle;
	vec3_t		angles;
	int			otherveh;
	vec3_t		otherpos;
	int			icon;
	int			drawnTargets = 0;
	trace_t		res;

	if( cg.RADARRangeSetting ) {
		range /= (2 * cg.RADARRangeSetting);
	}

	// adjust for radar screen size
	scale = range/54;

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
		CG_DrawPic( x + dir[0], y - dir[1], 8, 8, cgs.media.radarIcons[icon] );
		// check if we should draw any more targets
		drawnTargets++;
		if( drawnTargets >= cg_radarTargets.integer ) break;
	}
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
static void CG_Draw_MFD(int mfdnum, int vehicle, int onoff, int fuel, int flares) {
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
		char mode[4];
		int radarmode = onoff & OO_RADAR;

		// current range
		int range = 0;
		
		if( radarmode & OO_RADAR_AIR ) range = availableVehicles[vehicle].radarRange;
		else if( radarmode & OO_RADAR_GROUND ) range = availableVehicles[vehicle].radarRange2;

		if( cg.RADARRangeSetting && range ) {
			range /= (2 * cg.RADARRangeSetting);
		}

		// draw background
		trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, cgs.media.HUDrwr );

		if( mfdnum == MFD_1 ) {
			x = 0;
		} else {
			x = 512;
		}
		y = 352;

		// draw contents
		if( radarmode & OO_RADAR_AIR ) {
			CG_DrawRadarSymbols_AIR_new(vehicle, range, x+64, y+64);
			CG_MFQ3HUD_Numbers( x+86, y+14, 5, range, qfalse );
			strcpy( mode, "AIR" );
		} else if( radarmode & OO_RADAR_GROUND ) {
			CG_DrawRadarSymbols_GROUND_new(vehicle, range, x+64, y+64);
			CG_MFQ3HUD_Numbers( x+86, y+14, 5, range, qfalse );
			strcpy( mode, "GND" );
		} else if( availableVehicles[vehicle].radarRange || 
				   availableVehicles[vehicle].radarRange2 ) {
			strcpy( mode, "OFF" );
		}
		CG_DrawString_MFQ3( x+4, y+14, mode, 1.0f);
		// draw flares
		CG_MFQ3HUD_Numbers( x+86, y+114, 5, flares, qfalse );
	} else if( mode == MFD_STATUS ) {
		char	statusline[33];

		if( mfdnum == MFD_1 ) {
			x = 0;
		} else {
			x = 512;
		}
		y = 352;

		CG_DrawString_MFQ3( x+4, y+14, "STATUS:", 1.0f);
		CG_DrawString_MFQ3( x+4, y+20, "-------", 1.0f);
		Com_sprintf( statusline, 32, "GEAR:      %s", (onoff & OO_GEAR ? "DOWN" : "UP") );
		CG_DrawString_MFQ3( x+4, y+30, statusline, 1.0f);
		Com_sprintf( statusline, 32, "BRAKES:    %s", (onoff & OO_SPEEDBRAKE ? "ON" : "OFF") );
		CG_DrawString_MFQ3( x+4, y+40, statusline, 1.0f);
		Com_sprintf( statusline, 32, "WPNBAYS:   O/S" );
		CG_DrawString_MFQ3( x+4, y+50, statusline, 1.0f);
		Com_sprintf( statusline, 32, "AUTOPILOT: O/S" );
		CG_DrawString_MFQ3( x+4, y+60, statusline, 1.0f);
		Com_sprintf( statusline, 32, "FUEL:" );
		CG_DrawString_MFQ3( x+4, y+80, statusline, 1.0f);
		CG_MFQ3HUD_Numbers( x+86, y+80, 5, fuel, qfalse );
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
void CG_DrawStatusBar_MFQ3_new( void ) {
	centity_t	*cent;
	playerState_t	*ps;
	int			value, value2;
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

	// flares (for MFD)
	if( ps->ammo[WP_FLARE+8] > 0 ) {
		value = ps->ammo[WP_FLARE];
	} else {
		value = -1;
	}

	// fuel (for MFD)
	value2 = ps->stats[STAT_FUEL];
	if( (availableVehicles[vehicle].id&CAT_ANY) & CAT_PLANE ) {
		value2 *= 100;
	}

	// mfd 1
	if( hud_mfd.integer ) {
		CG_Draw_MFD(MFD_1, vehicle, cent->currentState.ONOFF, value2, value);
	}

	// mfd 2
	if( hud_mfd2.integer ) {
		CG_Draw_MFD(MFD_2, vehicle, cent->currentState.ONOFF, value2, value);
	}

	// solid middle section
	value = (100*ps->stats[STAT_HEALTH]/ps->stats[STAT_MAX_HEALTH]);
	if( value > 100 ) value = 100;
	if( hud_center.integer ) {
		CG_Draw_Center(value, ps->throttle);
	}

	// additional HUD info: health and throttle (redundant)
	CG_Draw_Redundant(value, ps->throttle);

	// cleanup
	cg.radarTargets = 0;
}

