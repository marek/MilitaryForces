/*
 * $Id: cg_drawnewhud.c,v 1.1 2005-08-22 16:01:59 thebjoern Exp $
*/

#include "cg_local.h"


// there are a couple of duplicate functions here
// this should be cleaned up and factorized when there is some time left..

static int alphabet_spaces[33] =
{
	8,	// A=8
	7,	// B=7
	8,	// C=8
	8,	// D=8
	6,	// E=6
	7,	// F=7
	8,	// G=8
	8,	// H=8
	5,	// I=5
	6,	// J=6
	8,	// K=8
	7,	// L=7
	8,	// M=8
	8,	// N=8
	9,	// O=9
	7,	// P=7
	9,	// Q=9
	6,	// R=6
	7,	// S=7
	7,	// T=7
	8,	// U=8
	8,	// V=8
	9,	// W=9
	7,	// X=7
	7,	// Y=7
	7,	// Z=7
	8,	// ?=8
	8,	// !=8
	8,	// *=8
	8,	// :=8
	8,	// /=8
	8,	// \=8
	7	// ==7	
};

static int number_spaces[16] =
{
	8,	// 0=8
	8,	// 1=8
	8,	// 2=8
	8,	// 3=8
	8,	// 4=8
	8,	// 5=8
	8,	// 6=8
	8,	// 7=8
	8,	// 8=8
	8,	// 9=8
	8,	// <=8
	8,	// >=8
	4,	// ,=4
	4,	// .=4
	11,	// %=11
	6	// -=6
};

/*
================
CG_DrawHUDPic

Coordinates are 640*480 virtual values
=================
*/
static void CG_DrawHUDPic( float x, float y, float width, float height, qhandle_t hShader, vec4_t color ) {
	trap_R_SetColor( color );
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
	trap_R_SetColor( NULL );
}

/*
===============
CG_DrawChar

Coordinates and size in 640*480 virtual screen size
===============
*/
static int getStartPos( int col ) {
	int i, ret = 0;

	for( i = 0; i < col; ++i ) {
		ret += alphabet_spaces[i];
	}
	return ret;
}

static int getStartPosNum( int col ) {
	int i, ret = 0;

	for( i = 0; i < col; ++i ) {
		ret += number_spaces[i];
	}
	return ret;
}


static int CG_DrawChar_MFQ3( int x, int y, int ch, qboolean right ) {
	int		col;
	float	fcol;
	float	size;
	float	ax, ay, aw, ah;
	qboolean num = qfalse;

	if( ch == '?' ) {
		col = 26;
	} else if( ch == '!' ) {
		col = 27;
	} else if( ch == '*' ) {
		col = 28;
	} else if( ch == '_' ) {
		col = 29;
	} else if( ch == ':' ) {
		col = 30;
	} else if( ch == '/' ) {
		col = 31;
	} else if( ch == '\\' ) {
		col = 32;
	} else if( ch == '=' ) {
		col = 33;
	} else if( Q_isalpha(ch) ) { 
		col = ch-65;
	} else if( (ch >= '0' && ch <= '9') ) {
		col = ch-'0';
		num = qtrue;
	} else if( ch == '<' ) {
		col = 10;
		num = qtrue;
	} else if( ch == '>' ) {
		col = 11;	
		num = qtrue;
	} else if( ch == ',' ) {
		col = 12;
		num = qtrue;
	} else if( ch == '.' ) {
		col = 13;
		num = qtrue;
	} else if( ch == '%' ) {
		col = 14;
		num = qtrue;
	} else if( ch == '-' ) {
		col = 15;
		num = qtrue;
	} else return HUDNUM_WIDTH;

	ch &= 255;

	if ( ch == ' ' ) {
		return HUDNUM_WIDTH;
	}

	if( num ) {
		aw = number_spaces[col];
		size = 0.0078125f;// size of one pixel on a 128 width bitmap
		fcol = getStartPosNum(col)*size;
	} else {
		aw = alphabet_spaces[col];
		size = 0.00390625f;// size of one pixel on a 256 width bitmap
		fcol = getStartPos(col)*size;
	}

	ah = HUDNUM_HEIGHT;
	ax = x;
	ay = y;

	if( right ) {
		ax -= aw;
	}

	CG_AdjustFrom640( &ax, &ay, &aw, &ah );

	if( num ) {
		trap_R_DrawStretchPic( ax, ay, aw, ah,
							   fcol, 0, 
							   fcol + size * number_spaces[col], 1, 
							   cgs.media.HUDnumberline );

		return number_spaces[col];
	} else {
		trap_R_DrawStretchPic( ax, ay, aw, ah,
							   fcol, 0, 
							   fcol + size * alphabet_spaces[col], 1, 
							   cgs.media.HUDalphabet );

		return alphabet_spaces[col];
	}
}

/*
==================
CG_DrawString_MFQ3

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
static int CG_DrawString_MFQ3( int x, int y, const char *string, const float *setColor, int maxChars ) {
	const char	*s = 0;
	char		*lowerstring = 0;
	int			xx = x;
	int			cnt;
	int			charWidth, charHeight;

	if( !string )
		return 0;
	
	lowerstring = malloc(strlen(string)+1);
	strcpy(lowerstring, string);

	charWidth = HUDNUM_WIDTH;
	charHeight = HUDNUM_HEIGHT;

	Q_strupr( lowerstring );

	if (maxChars <= 0)
		maxChars = 32767; // do them all!

	// draw the colored text
	s = lowerstring;
	cnt = 0;
	trap_R_SetColor( setColor );
	while ( *s && cnt < maxChars) {
		charWidth = CG_DrawChar_MFQ3( xx, y, *s, qfalse );
		xx += charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor( NULL );

	free(lowerstring);
	return (xx-x);
}


/*
==================
CG_DrawString_MFQ3 - right aligned (ie x = right end of string)

Draws a multi-colored string with a drop shadow, optionally forcing
to a fixed color.

Coordinates are at 640 by 480 virtual resolution
==================
*/
static int CG_DrawString_MFQ3_R( int x, int y, const char *string, const float *setColor, int maxChars )
{
	const char	*s;
	int			xx;
	int			cnt;
	int			charWidth, charHeight;
	char		invstring[128];
	int			len = strlen(string)-1;
	int			i = 0;

	if( len >= 127 ) {
		CG_Printf( "String too long! Not printed!" );
		return 0;
	}
	while( len >= 0 ) {
		invstring[i] = string[len];
		len--;
		i++;
	}
	invstring[i] = 0;

	charWidth = HUDNUM_WIDTH;
	charHeight = HUDNUM_HEIGHT;

	Q_strupr( (char*)invstring );

	if (maxChars <= 0)
		maxChars = 32767; // do them all!

	// draw the colored text
	s = invstring;
	xx = x;
	cnt = 0;
	trap_R_SetColor( setColor );
	while ( *s && cnt < maxChars) {
		charWidth = CG_DrawChar_MFQ3( xx, y, *s, qtrue );
		xx -= charWidth;
		cnt++;
		s++;
	}
	trap_R_SetColor( NULL );
	return (xx-x);
}


/*
==============
CG_MFQ3HUD_Numbers
==============
*/
/*
static void CG_MFQ3HUD_Numbers (int x, int y, int width, int value, qboolean percent, vec4_t color, qboolean right ) {
	char	num[16], *ptr;
	int		l;
	int		frame;

	if ( width < 1 ) {
		return;
	}

	trap_R_SetColor( color );
	
	// draw number string
	if ( width > 7 ) {
		width = 7;
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
	case 5:
		value = value > 99999 ? 99999 : value;
		value = value < -9999 ? -9999 : value;
		break;
	case 6:
		value = value > 999999 ? 999999 : value;
		value = value < -99999 ? -99999 : value;
		break;
	}

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
//	x += 2 + (HUDNUM_WIDTH-1)*(width - l);
	if( right ) {
		char num2[16];
		int	 l2 = l-1, i = 0;
		while( l2 >= 0 ) {
			num2[i] = num[l2];
			l2--;
			i++;
		}
		num2[i] = 0;
		Com_sprintf( num, sizeof(num), "%s", num2 );
		x -= (HUDNUM_WIDTH-1);
	}

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		CG_DrawPic( x,y, HUDNUM_WIDTH, HUDNUM_HEIGHT, cgs.media.HUDnumbers[frame] );
		if( right ) x -= HUDNUM_WIDTH-1;
		else x += HUDNUM_WIDTH-1;
		ptr++;
		l--;
	}
	if( percent ) {
		CG_DrawPic( x,y, 2*HUDNUM_WIDTH, HUDNUM_HEIGHT, cgs.media.HUDnumbers[STAT_PERCENT] );
	}

	trap_R_SetColor( NULL );

}
*/

/*
==============
CG_MFQ3HUD_DecNumbers

prints a number with a '.' in it; ie divides this number by 10^decimals

width = total width
==============
*/
/*
static void CG_MFQ3HUD_DecNumbers (int x, int y, int width, int decimals, int invalue, vec4_t color ) {
	int		div = 10;
	int		pre, post;
	int		prewid, 
			postwid = decimals;

	while( --decimals ) div *= 10;

	pre = invalue/div;
	post = invalue%div;

	prewid = width-decimals-1;

	CG_MFQ3HUD_Numbers( x, y, prewid, pre, qfalse, HUDColors[cg.HUDColor], qtrue );
//	x += (HUDNUM_WIDTH-1)*prewid;

	CG_DrawHUDPic( x-1,y+2, HUDNUM_WIDTH, HUDNUM_HEIGHT, cgs.media.HUDnumbers[STAT_POINT], HUDColors[cg.HUDColor] );
//	x += HUDNUM_WIDTH-1;

	CG_MFQ3HUD_Numbers( x+4, y, postwid, post, qfalse, HUDColors[cg.HUDColor], qfalse );
}

*/



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
	if( (availableVehicles[vehicle].cat & CAT_GROUND) ||
		(availableVehicles[vehicle].cat & CAT_BOAT) ) {
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
		if( (availableVehicles[otherveh].cat & CAT_PLANE) || 
			(availableVehicles[otherveh].cat & CAT_HELO) ) {
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
	qboolean	groundinstallation = 0;

	if( cg.RADARRangeSetting ) {
		range /= (2 * cg.RADARRangeSetting);
	}

	// adjust for radar screen size
	scale = range/54;

	// ground vehicle adjustment for angle
	if( (availableVehicles[vehicle].cat & CAT_GROUND) ||
		(availableVehicles[vehicle].cat & CAT_BOAT) ) {
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
		groundinstallation = 0;
		if( !cg.radarEnts[i]->destroyableStructure ) {
			// get other vehicle
			if( cg.radarEnts[i]->currentState.eType == ET_MISC_VEHICLE ) {
				if( cg.radarEnts[i]->currentState.modelindex == 255 )	// ground installation
				{
					otherveh = cg.radarEnts[i]->currentState.modelindex2;	
					groundinstallation = 1;
				}
				else													// other misc veh
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
			if( angle >= 45 && angle <= 315 ) 
			{
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
			if( !(cg.radarEnts[i]->currentState.ONOFF & OO_RADAR) ) 
			{
				CG_Trace( &res, pos, NULL, NULL, otherpos, cg.snap->ps.clientNum, MASK_SOLID );
				if( res.fraction < 1.0f ) continue;
			}
			// which icon (if at all, dont show lqms)
			if( groundinstallation )
			{
				icon = RD_GROUND_ENEMY;
			}
			else if( (availableVehicles[otherveh].cat & CAT_GROUND) ||
					 (availableVehicles[otherveh].cat & CAT_BOAT) ) 
			{
				icon = RD_GROUND_ENEMY;
			} 
			else 
			{
				continue;
			}
			// friend or foe
			if( cgs.gametype >= GT_TEAM ) 
			{
				if( cgs.clientinfo[cg.radarEnts[i]->currentState.clientNum].team ==
					cgs.clientinfo[self->currentState.clientNum].team ) 
					icon++;
			}
		// buildings
		} 
		else 
		{
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
			if( ( (availableVehicles[vehicle].cat & CAT_GROUND) || 
				  (availableVehicles[vehicle].cat & CAT_BOAT) ) &&
				angle >= 90 && angle <= 270 ) continue;
			else if( ( (availableVehicles[vehicle].cat & CAT_PLANE) ||
					   (availableVehicles[vehicle].cat & CAT_HELO) ) &&
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


static void CG_DrawGroundSupportRadar( int vehicle, float range, int x, int y ) {
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
	qboolean	groundinstallation = 0;

	if( cg.RADARRangeSetting ) {
		range /= (2 * cg.RADARRangeSetting);
	}

	// adjust for radar screen size
	scale = range/54;

	// ground vehicle adjustment for angle
	if( (availableVehicles[vehicle].cat & CAT_GROUND) ||
		(availableVehicles[vehicle].cat & CAT_BOAT) ) {
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
		groundinstallation = 0;
		if( !cg.radarEnts[i]->destroyableStructure ) {
			// get other vehicle
			if( cg.radarEnts[i]->currentState.eType == ET_MISC_VEHICLE ) {
				if( cg.radarEnts[i]->currentState.modelindex == 255 )	// ground installation
				{
					otherveh = cg.radarEnts[i]->currentState.modelindex2;	
					groundinstallation = 1;
				}
				else													// other misc veh
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
			if( angle >= 45 && angle <= 315 ) 
			{
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
			if( !(cg.radarEnts[i]->currentState.ONOFF & OO_RADAR) ) 
			{
				CG_Trace( &res, pos, NULL, NULL, otherpos, cg.snap->ps.clientNum, MASK_SOLID );
				if( res.fraction < 1.0f ) continue;
			}
			// which icon (if at all, dont show lqms)
			if( groundinstallation )
			{
				icon = RD_GROUND_ENEMY;
			}
			else if( (availableVehicles[otherveh].cat & CAT_GROUND) ||
					 (availableVehicles[otherveh].cat & CAT_BOAT) ) 
			{
				icon = RD_GROUND_ENEMY;
			} 
			else 
			{
				continue;
			}
			// friend or foe
			if( cgs.gametype >= GT_TEAM ) 
			{
				if( cgs.clientinfo[cg.radarEnts[i]->currentState.clientNum].team ==
					cgs.clientinfo[self->currentState.clientNum].team ) 
					icon++;
			}
		// buildings
		} 
		else 
		{
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
			if( ( (availableVehicles[vehicle].cat & CAT_GROUND) || 
				  (availableVehicles[vehicle].cat & CAT_BOAT) ) &&
				angle >= 90 && angle <= 270 ) continue;
			else if( ( (availableVehicles[vehicle].cat & CAT_PLANE) ||
					   (availableVehicles[vehicle].cat & CAT_HELO) ) &&
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
static void CG_Draw_Coords(int xcoord, int ycoord) {
		// draw coords
	char buffer[8];
	Com_sprintf(buffer, 7, "%d", xcoord);
	CG_DrawString_MFQ3_R( 290, 32, buffer, HUDColors[cg.HUDColor], 0 );
	Com_sprintf(buffer, 7, "%d", ycoord);
	CG_DrawString_MFQ3( 350, 32, buffer, HUDColors[cg.HUDColor], 0 );
}

/*
================
CG_Draw_Redundant

================
*/
static void CG_Draw_Redundant(int vehicle, int health, int throttle, int ammo[16], int selweap) {
	char		buffer[12];
		
	// if vehicle is invalid break
	if( vehicle < 0 )
		return;

		// draw health
	if( hud_health.integer ) {
		CG_DrawHUDPic( 516, 318, 32, 8, cgs.media.HUDhealthtext, HUDColors[cg.HUDColor] ); 
		Com_sprintf(buffer, 5, "%d%%", health);
		CG_DrawString_MFQ3( 560, 318, buffer, HUDColors[cg.HUDColor], 0 );
	}

		// draw throttle
	if( hud_throttle.integer ) {
		CG_DrawHUDPic( 516, 330, 32, 8, cgs.media.HUDthrottletext, HUDColors[cg.HUDColor] ); 

		if( availableVehicles[vehicle].engines > 1 ) {
			Com_sprintf(buffer, 10, "%d%%/%d%%", throttle*10, throttle*10);
			CG_DrawString_MFQ3( 560, 330, buffer, HUDColors[cg.HUDColor], 0 );
		} else {
			if( (availableVehicles[vehicle].cat & CAT_GROUND) &&
				throttle > MF_THROTTLE_MILITARY ) {
				Com_sprintf(buffer, 5, "%d%%", (10-throttle)*10);
				CG_DrawString_MFQ3( 560, 330, buffer, HUDColors[cg.HUDColor], 0 );
			} else {
				Com_sprintf(buffer, 5, "%d%%", throttle*10);
				CG_DrawString_MFQ3( 560, 330, buffer, HUDColors[cg.HUDColor], 0 );
			}
		}
	}

		// draw ammo
	if( hud_weapons.integer ) {
		char	ammostring[33];
		char*	actualstring;
			// mg
		if( ammo[8] ) {
			if( (availableVehicles[vehicle].cat & CAT_GROUND) || 
				(availableVehicles[vehicle].cat & CAT_BOAT) ) {
				actualstring = availableWeapons[availableVehicles[vehicle].weapons[0]].shortName2;
			} else {
				actualstring = availableWeapons[availableVehicles[vehicle].weapons[0]].shortName;
			}
			Com_sprintf( ammostring, 32, "%s: %d", actualstring, ammo[0] );
			CG_DrawString_MFQ3( 4, 318, ammostring, HUDColors[cg.HUDColor], 0);
		}

			// selected weapon
		if( ammo[selweap+8] && selweap != 0 ) {
			if( (availableVehicles[vehicle].cat & CAT_GROUND) || 
				(availableVehicles[vehicle].cat & CAT_BOAT) ) {
				actualstring = availableWeapons[availableVehicles[vehicle].weapons[selweap]].shortName2;
			} else {
				actualstring = availableWeapons[availableVehicles[vehicle].weapons[selweap]].shortName;
			}
			Com_sprintf( ammostring, 32, "%s: %d", actualstring, ammo[selweap] );
			CG_DrawString_MFQ3( 4, 330, ammostring, HUDColors[cg.HUDColor], 0);
		}
	}

}


/*
================
CG_Draw_Center

================
*/
static void CG_Draw_Center(int vehicle, int health, int throttle) {
	float		x, y, width, height;
	int			healthpic = (health/10)-1;
	int			throttlepic = throttle - 1;
	
	x = 192;		
	y = 448;
	width = 256;
	height = 32;

		// background
	CG_DrawPic( x, y, width, height, cgs.media.HUDsolid ); 

		// health
	if( healthpic >= 0 ) {
		x = 192;
		y = 448;
		width = 128;
		height = 32;
		CG_DrawPic( x, y, width, height, cgs.media.HUDhealth[healthpic] ); 
	}

		// throttle
	if( throttlepic >= 0 ) {
		if( availableVehicles[vehicle].engines > 1 ) {
			if( availableVehicles[vehicle].maxthrottle > MF_THROTTLE_MILITARY ) {
				CG_DrawPic( 320, 448, 128, 16, cgs.media.HUDthrottle_2_1_ab[throttlepic] ); 
				CG_DrawPic( 320, 464, 128, 16, cgs.media.HUDthrottle_2_2_ab[throttlepic] ); 
			} else {
				CG_DrawPic( 320, 448, 128, 16, cgs.media.HUDthrottle_2_1[throttlepic] ); 
				CG_DrawPic( 320, 464, 128, 16, cgs.media.HUDthrottle_2_2[throttlepic] ); 
			}
		} else {
			if( availableVehicles[vehicle].maxthrottle > MF_THROTTLE_MILITARY ) {
				CG_DrawPic( 320, 448, 128, height, cgs.media.HUDthrottle_1_ab[throttlepic] ); 
			} else {
				if( ( (availableVehicles[vehicle].cat & CAT_GROUND) ||
					(availableVehicles[vehicle].cat & CAT_BOAT) ||
					(availableVehicles[vehicle].cat & CAT_HELO) ) &&
					throttlepic >= MF_THROTTLE_MILITARY ) {
					CG_DrawPic( 320, 448, 128, 32, cgs.media.HUDthrottle_rev[throttlepic-10] ); 
				} else {
					CG_DrawPic( 320, 448, 128, 32, cgs.media.HUDthrottle_1[throttlepic] ); 
				}
			}
		}
	}
}

/*
================
CG_HUD_Camera

================
*/
static void CG_HUD_Camera(int mfdnum, int vehicle) {

	// this is probably very dirty and ugly
	// but so far I haven't found another way to do it...
	// any ideas anyone?
	centity_t *cent = &cg.predictedPlayerEntity;
	playerState_t *ps = &cg.predictedPlayerState;
	vec3_t	v;
	float	x, y, w, h, vx, vy, vh;
	float	fov;
	int		cammode;
	char	buffer[12];

	// prepare refdef
	memcpy( &cg.HUDCamera, &cg.refdef, sizeof( cg.HUDCamera) );
	
	// set left draw position
	if( mfdnum == MFD_1 )
	{
		x = 5;
	}
	else
	{
		x = 517;
	}

	// general
	y = 366;
	w = 118;
	h = 108;
	vx = x;
	vy = y;
	vh = h;
	CG_AdjustFrom640( &x, &y, &w, &h );
	cg.HUDCamera.x = x;
	cg.HUDCamera.y = y;
	cg.HUDCamera.width = w;
	cg.HUDCamera.height = h;
	fov = 90/4;	// fov = 22.5

	// apply any zoom
	if( cg.zoomed )
	{
		fov /= (float)cg.zoomAmount;
	}

	// set final FOV
	cg.HUDCamera.fov_x = fov;
	cg.HUDCamera.fov_y = fov;
	cammode = cg.CameraMode;

	if( (availableVehicles[vehicle].cat & CAT_GROUND) ||
		(availableVehicles[vehicle].cat & CAT_BOAT) ||
		(availableVehicles[vehicle].cat & CAT_HELO)) cammode = CAMERA_TARGET;

	if( cammode == CAMERA_DOWN ) {
		vec3_t a;
		AngleVectors( cent->currentState.angles, 0, 0, v );
		VectorSet( a, 90, cent->currentState.angles[1], 0 );
		AnglesToAxis( a, cg.HUDCamera.viewaxis );
			// position
		VectorScale( v, availableVehicles[vehicle].mins[2], v );
		VectorAdd( cent->lerpOrigin, v, cg.HUDCamera.vieworg );
	} else if( cammode == CAMERA_BACK ) {
		if( (availableVehicles[vehicle].cat & CAT_GROUND) ||
			(availableVehicles[vehicle].cat & CAT_BOAT) ||
			(availableVehicles[vehicle].cat & CAT_HELO)) {
			vec3_t right, up, temp;
			AngleVectors( cent->currentState.angles, v, right, up );
			RotatePointAroundVector( temp, up, v, cent->currentState.angles2[ROLL] );
			CrossProduct( up, temp, right );
			RotatePointAroundVector( v, right, temp, cent->currentState.angles2[PITCH] );
			VectorInverse( v );
			vectoangles( v, temp );
			AnglesToAxis( temp, cg.HUDCamera.viewaxis );
		} else {
			vec3_t a, b;
			AngleVectors( cent->currentState.angles, v, 0, 0 );
			VectorCopy( v, b );
			VectorInverse( b );
			vectoangles( b, a );
			a[2] = -cent->currentState.angles[2];
			AnglesToAxis( a, cg.HUDCamera.viewaxis );
		}
			// position
		VectorScale( v, availableVehicles[vehicle].mins[0], v );
		VectorAdd( cent->lerpOrigin, v, cg.HUDCamera.vieworg );
	} else if( cammode == CAMERA_BOMB ) {
		int speed = ps->speed/10;
		vec3_t horz, vert, tempH, tempV, end;
		float hD, vD, alpha;
		int t;
		trace_t tr;
		AngleVectors( cent->currentState.angles, v, 0, 0 );
		VectorSet( horz, v[0], v[1], 0 );
		VectorNormalize( horz );
		VectorSet( vert, 0, 0, 1 );
		alpha = DEG2RAD(cent->currentState.angles[0]);
		// bombs never travel longer than 10 seconds (they autoexplode after that time)
		// so we can use that for our calculations
		for( t = 1; t <= 10; ++t ) {
			hD = speed*t*cos(alpha);
			vD = speed*t*sin(alpha) - DEFAULT_GRAVITY*t*t/2;
			VectorScale( horz, hD, tempH );
			VectorScale( vert, vD, tempV);
			VectorAdd( tempH, tempV, v );
			VectorAdd( cent->currentState.pos.trBase, v, end );
			VectorNormalize( v );
			CG_Trace( &tr, cent->currentState.pos.trBase, 0, 0, end, cent->currentState.number, MASK_ALL );
			if( tr.fraction < 1 ) break;
		}
		vectoangles( v, end );
		AnglesToAxis( end, cg.HUDCamera.viewaxis );
			// position
		VectorScale( v, -availableVehicles[vehicle].mins[0], v );
		VectorAdd( cent->lerpOrigin, v, cg.HUDCamera.vieworg );
	} else if( cammode == CAMERA_TARGET ) {
		// angles
		if( ps->stats[STAT_LOCKINFO] & LI_TRACKING ) {
			vec3_t a;
			centity_t* target = &cg_entities[cent->currentState.tracktarget];
			qboolean building = qfalse;

			if( target->currentState.eType == ET_EXPLOSIVE ) building = qtrue;
			if( building ) {
				VectorSubtract( cgs.inlineModelMidpoints[target->currentState.modelindex], cent->lerpOrigin, v );
			} else {
				VectorSubtract( target->lerpOrigin, cent->lerpOrigin, v );
			}
			VectorNormalize( v );
			vectoangles( v, a );
			AnglesToAxis( a, cg.HUDCamera.viewaxis );
		} else {
			if( (availableVehicles[vehicle].cat & CAT_GROUND) ||
				(availableVehicles[vehicle].cat & CAT_BOAT) ||
				(availableVehicles[vehicle].cat & CAT_HELO)) {
				vec3_t right, up, temp;
				AngleVectors( cent->currentState.angles, v, right, up );
				RotatePointAroundVector( temp, up, v, cent->currentState.angles2[ROLL] );
				CrossProduct( up, temp, right );
				RotatePointAroundVector( v, right, temp, cent->currentState.angles2[PITCH] );
				vectoangles( v, temp );
				AnglesToAxis( temp, cg.HUDCamera.viewaxis );
			} else {
				AnglesToAxis( cent->currentState.angles, cg.HUDCamera.viewaxis );
				AngleVectors( cent->currentState.angles, v, 0, 0 );
			}
		}
			// position
		VectorScale( v, availableVehicles[vehicle].maxs[0], v );
		VectorAdd( cent->lerpOrigin, v, cg.HUDCamera.vieworg );
		cg.HUDCamera.vieworg[2] += 4;
	} 

	// set a client global so that other code can render differently for this mini-render
	cg.drawingMFD = qtrue;

	// ummm shall I really do this ???
	CG_AddPacketEntities();
	CG_AddMarks();
	CG_AddLocalEntities();

	// render
	trap_R_RenderScene( &cg.HUDCamera );

	// draw zoom amount
	Com_sprintf(buffer, 11, "ZOOM: X%d", cg.zoomAmount);
	CG_DrawString_MFQ3( vx+2, vy+vh-12, buffer, HUDColors[cg.MFDColor], 0);

	// reset to normal rendering
	cg.drawingMFD = qfalse;
}

/*
================
CG_Draw_MFD

================
*/
static void CG_Draw_MFD(int mfdnum, int vehicle, centity_t * cent, int targetrange, 
						int targetheading, int altitude, playerState_t *ps, int stallcolor) {
	float		x, y, width, height;
	int			mode = cg.Mode_MFD[mfdnum];
	int			onoff = cent->currentState.ONOFF;
	int			selweap = cent->currentState.weaponNum;
	char		buffer[16];

		// draw mfd screen
	if( mfdnum == MFD_1 ) {
		x = 0;
	} else {
		x = 512;		
	}
	y = 352;
	width = height = 128;
	CG_DrawPic( x, y, width, height, cgs.media.HUDmfd );

		// draw contents of mfd only if it is not off
	if( !mode ) return;

		// RWR mode
	if( mode == MFD_RWR ) {
		char mode[4];
		int radarmode = onoff & OO_RADAR;
		int flares;

		// current range
		int range = 0;
		
		// flares (for MFD)
		if( ps->ammo[WP_FLARE+8] > 0 ) {
			flares = ps->ammo[WP_FLARE];
		} else {
			flares = -1;
		}

		if( radarmode & OO_RADAR_AIR ) range = availableVehicles[vehicle].radarRange;
		else if( radarmode & OO_RADAR_GROUND ) range = availableVehicles[vehicle].radarRange2;

		if( cg.RADARRangeSetting && range ) {
			range /= (2 * cg.RADARRangeSetting);
		}

		// draw background
		trap_R_SetColor( HUDColors[cg.MFDColor] );
		CG_DrawPic( x, y, width, height, cgs.media.HUDrwr );
		trap_R_SetColor( NULL );
		// draw contents
		if( radarmode & OO_RADAR_AIR ) {
			CG_DrawRadarSymbols_AIR_new(vehicle, range, x+60, y+60);
			Com_sprintf(buffer, 15, "%d", range);
			CG_DrawString_MFQ3_R( x+122, y+14, buffer, HUDColors[cg.MFDColor], 0);
			strcpy( mode, "AIR" );
		} else if( radarmode & OO_RADAR_GROUND ) {
			CG_DrawRadarSymbols_GROUND_new(vehicle, range, x+60, y+60);
			Com_sprintf(buffer, 15, "%d", range);
			CG_DrawString_MFQ3_R( x+122, y+14, buffer, HUDColors[cg.MFDColor], 0);
			strcpy( mode, "GND" );
		} else if( availableVehicles[vehicle].radarRange || 
				   availableVehicles[vehicle].radarRange2 ) {
			strcpy( mode, "OFF" );
		}
		CG_DrawString_MFQ3( x+4, y+14, mode, HUDColors[cg.MFDColor], 0);
		// draw flares
		if( flares >= 0 ) {
			Com_sprintf(buffer, 6, "%d", flares);
			CG_DrawString_MFQ3_R( x+122, y+114, buffer, HUDColors[cg.MFDColor], 0);
		}
	} else if( mode == MFD_STATUS ) {
		int		fuel;
		// fuel (for MFD)
		fuel = ps->stats[STAT_FUEL];
		if( (availableVehicles[vehicle].cat & CAT_PLANE) ||
			(availableVehicles[vehicle].cat & CAT_HELO) ||
			(availableVehicles[vehicle].cat & CAT_BOAT) ) {
			fuel *= 100;
		}
		CG_DrawString_MFQ3( x+4, y+14, "STATUS:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+20, "------", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+30, "GEAR:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3_R( x+122, y+30, (onoff & OO_GEAR ? "DOWN" : "UP"), HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+40, "BRAKES:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3_R( x+122, y+40, (onoff & OO_SPEEDBRAKE ? "ON" : "OFF"), HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+50, "WPNBAYS:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3_R( x+122, y+50, (onoff & OO_WEAPONBAY ? "OPEN" : "CLSD"), HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+60, "AUTOPILOT:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3_R( x+122, y+60, "O/S", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+80, "FUEL:", HUDColors[cg.MFDColor], 0);
		Com_sprintf(buffer, 6, "%d", fuel);
		CG_DrawString_MFQ3_R( x+122, y+80, buffer, HUDColors[cg.MFDColor], 0);
	} else if( mode == MFD_INVENTORY ) {
		char	invline[33];
		int		i;
		CG_DrawString_MFQ3( x+4, y+14, "INVENTORY:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+20, "_________", HUDColors[cg.MFDColor], 0);

		y = 382;

		for( i = 0; i < 8; i++ ) {
			// if weapon available
			if( ps->ammo[i+8] ) {
				char* actualstring;
				if( (availableVehicles[vehicle].cat & CAT_GROUND) ||
					(availableVehicles[vehicle].cat & CAT_BOAT) ) {
					actualstring = availableWeapons[availableVehicles[vehicle].weapons[i]].shortName2;
				} else {
					actualstring = availableWeapons[availableVehicles[vehicle].weapons[i]].shortName;
				}
				Com_sprintf( buffer, 5, "%d", ps->ammo[i] );
				CG_DrawString_MFQ3_R( x+36, y, buffer, HUDColors[cg.MFDColor], 0);
				Com_sprintf( invline, 32, "%c    %s", (i == selweap ? '*' : ' '), actualstring );
				CG_DrawString_MFQ3( x+4, y, invline, HUDColors[cg.MFDColor], 0);
				y += 10;
			}
			if( i == 0 || i == 6 ) y+=4; 
		}
	}
	else if( mode == MFD_CAMERA )
	{
		CG_HUD_Camera(mfdnum, vehicle);
		if( targetrange >= 0 )
		{
			Com_sprintf(buffer, 15, "RANGE: %d", targetrange);
			CG_DrawString_MFQ3( x+6, y+16, buffer, HUDColors[cg.MFDColor], 0);
			CG_DrawHUDPic( x+48, y+48, 32, 32, cgs.media.HUDreticles[HR_TARGET_ENEMY], HUDColors[HUD_GREEN] );
		}
		else
		{
			CG_DrawHUDPic( x+48, y+48, 32, 32, cgs.media.HUDreticles[HR_GUIDED_WHITE], HUDColors[HUD_WHITE] );
		}
	}
	else if( mode == MFD_INFO )
	{
		int	value;
		CG_DrawString_MFQ3( x+4, y+14, "FLIGHTDATA:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+20, "----------", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+30, "HEADING:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+40, "TGT HDG:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+54, "SPEED:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+64, "ALTITUDE:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+78, "POS X:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+88, "POS Y:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+102, "THROTTLE:", HUDColors[cg.MFDColor], 0);
		CG_DrawString_MFQ3( x+4, y+112, "HEALTH:", HUDColors[cg.MFDColor], 0);
		value = 360 - (int)ps->vehicleAngles[1];
		if( value <= 0 ) value += 360;
			Com_sprintf(buffer, 5, "%d", value);
			CG_DrawString_MFQ3_R( x+122, y+30, buffer, HUDColors[cg.MFDColor], 0);
		if( targetheading >= 0 ) {
			Com_sprintf(buffer, 5, "%d", targetheading);
			CG_DrawString_MFQ3_R( x+122, y+40, buffer, HUDColors[cg.MFDColor], 0);
		} else {
			CG_DrawString_MFQ3_R( x+122, y+40, "---", HUDColors[cg.MFDColor], 0);
		}
		value = ps->speed/10;
		Com_sprintf(buffer, 7, "%d", value);
		CG_DrawString_MFQ3_R( x+122, y+54, buffer, HUDColors[stallcolor], 0);
		Com_sprintf(buffer, 7, "%d", altitude);
		CG_DrawString_MFQ3_R( x+122, y+64, buffer, HUDColors[cg.MFDColor], 0);
		Com_sprintf(buffer, 7, "%d", (int)ps->origin[0]);
		CG_DrawString_MFQ3_R( x+122, y+78, buffer, HUDColors[cg.MFDColor], 0);
		Com_sprintf(buffer, 7, "%d", (int)ps->origin[1]);
		CG_DrawString_MFQ3_R( x+122, y+88, buffer, HUDColors[cg.MFDColor], 0);
		value = (100*ps->stats[STAT_HEALTH]/ps->stats[STAT_MAX_HEALTH]);
		if( value > 100 ) value = 100;	
		Com_sprintf(buffer, 7, "%d", ps->throttle*10);
		CG_DrawString_MFQ3_R( x+122, y+102, buffer, HUDColors[cg.MFDColor], 0);
		Com_sprintf(buffer, 7, "%d", value);
		CG_DrawString_MFQ3_R( x+122, y+112, buffer, HUDColors[cg.MFDColor], 0);
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
	char		buffer[12];

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
	trap_R_SetColor( HUDColors[cg.HUDColor] );
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0.125f+offset, 1, 0.625f+offset, cgs.media.HUDalt );
	trap_R_SetColor( NULL );
		// alt indicater
	x = 578;
	y = 171;
	width = height = 8;
	CG_DrawHUDPic( x, y, width, height, cgs.media.HUDind_v_r, HUDColors[cg.HUDColor] );
		// draw alt value box
	x = 585;
	y = 28;
	width = 52;
	height = 16;
	CG_DrawHUDPic( x, y, width, height, cgs.media.HUDvaluebox2, HUDColors[cg.HUDColor] );
		// draw alt
	Com_sprintf(buffer, 7, "%d", value);
	CG_DrawString_MFQ3_R( 626, 32, buffer, HUDColors[cg.HUDColor], 0);
	
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
	Com_sprintf(buffer, 11, "%.1f", (float)visible3/100 );
	CG_DrawString_MFQ3( 596, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
	if( visible2 >= 0 ) {
		value2 = value - visible2;
		Com_sprintf(buffer, 11, "%.1f", (float)visible2/100 );
		CG_DrawString_MFQ3( 596, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
		if( bottom && visible1 >= 0 ) {
			value2 = value - visible1;
			Com_sprintf(buffer, 11, "%.1f", (float)visible1/100 );
			CG_DrawString_MFQ3( 596, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
		}
	}
	value2 = value - visible4;
	Com_sprintf(buffer, 11, "%.1f", (float)visible4/100 );
	CG_DrawString_MFQ3( 596, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
	if( top ) {
		value2 = value - visible5;
		Com_sprintf(buffer, 11, "%.1f", (float)visible5/100 );
		CG_DrawString_MFQ3( 596, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
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
	char		buffer[12];

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
	trap_R_SetColor( HUDColors[cg.HUDColor] );
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0.125f+offset, 1, 0.625f+offset, cgs.media.HUDspeed );
	trap_R_SetColor( NULL );
		// speed indicater
	x = 55;
	y = 171;
	width = height = 8;
	CG_DrawHUDPic( x, y, width, height, cgs.media.HUDind_v, HUDColors[cg.HUDColor] );
		// draw speed value box
	x = 3;
	y = 28;
	width = 52;
	height = 16;
	CG_DrawHUDPic( x, y, width, height, cgs.media.HUDvaluebox2, HUDColors[cg.HUDColor] );
		// draw speed
	Com_sprintf(buffer, 7, "%d", value);
	CG_DrawString_MFQ3_R( 45, 32, buffer, HUDColors[cg.HUDColor], 0);
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
	Com_sprintf(buffer, 7, "%d", visible3);
	CG_DrawString_MFQ3_R( 46, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
	if( visible2 >= 0 ) {
		value2 = value - visible2;
		Com_sprintf(buffer, 7, "%d", visible2);
		CG_DrawString_MFQ3_R( 46, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
		if( bottom && visible1 >= 0 ) {
			value2 = value - visible1;
			Com_sprintf(buffer, 7, "%d", visible1);
			CG_DrawString_MFQ3_R( 46, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
		}
	}
	value2 = value - visible4;
	Com_sprintf(buffer, 7, "%d", visible4);
	CG_DrawString_MFQ3_R( 46, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
	if( top ) {
		value2 = value - visible5;
		Com_sprintf(buffer, 7, "%d", visible5);
		CG_DrawString_MFQ3_R( 46, 171+(value2*64/scale), buffer, HUDColors[cg.HUDColor], 0);
	}
		// gearspeed
	if( gearspeed > 0 ) {
		value2 = value - gearspeed;
		if( value2 > -(scale*2) && value2 < (scale*2) ) {
			x = 55;
			y = 171+(value2*64/scale);
			width = height = 8;
			CG_DrawHUDPic( x, y, width, height, cgs.media.HUDcaret_v_g_l, HUDColors[cg.HUDColor] );
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
	char		buffer[8];

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
	trap_R_SetColor( HUDColors[cg.HUDColor] );
	CG_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0.125f-offset, 0, 1-offset, 1, cgs.media.HUDheading );
	trap_R_SetColor( NULL );
		// draw heading value box
	x = 304;
	y = 28;
	width = 32;
	height = 16;
	CG_DrawHUDPic( x, y, width, height, cgs.media.HUDvaluebox, HUDColors[cg.HUDColor] );
		// draw heading
	Com_sprintf(buffer, 7, "%d", value);
	CG_DrawString_MFQ3_R( 332, 32, buffer, HUDColors[cg.HUDColor], 0);
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
	Com_sprintf(buffer, 7, "%d", visible2);
	CG_DrawString_MFQ3_R( 330-(value2*64/10), 6, buffer, HUDColors[cg.HUDColor], 0);
	if( left ) {
		value2 = value - visible1;
		if( value2 < 0 ) value2 += 360;
		Com_sprintf(buffer, 7, "%d", visible1);
		CG_DrawString_MFQ3_R( 330-(value2*64/10), 6, buffer, HUDColors[cg.HUDColor], 0);
	}
	if( right ) {
		value2 = value - visible3;
		if( value2 > 0 ) value2 -= 360;
		Com_sprintf(buffer, 7, "%d", visible3);
		CG_DrawString_MFQ3_R( 330-(value2*64/10), 6, buffer, HUDColors[cg.HUDColor], 0);
	}

		// heading indicater
	x = 316;
	y = 24;
	width = height = 8;
	CG_DrawHUDPic( x, y, width, height, cgs.media.HUDind_h, HUDColors[cg.HUDColor] );

		// heading caret
	if( targetheading > 0 && targetheading <= 360 ) {
		value2 = value - targetheading;
		if( value2 < -180 ) value2 += 360;
		else if( value2 > 180 ) value2 -= 360;
		if( value2 < -35 ) value2 = -35;
		if( value2 > 35 ) value2 = 35;
		x = 316-(value2*64/10);
		y = 24;
		width = height = 8;
		CG_DrawHUDPic( x, y, width, height, cgs.media.HUDcaret_h, HUDColors[cg.HUDColor] );
	}
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
	int			targetheading, targetrange, altitude = 0, stallcolor;
	int			speed, stallspeed = availableVehicles[vehicle].stallspeed;

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

	// aiming/targetting reticles
	CG_Draw_Reticles();

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;
	speed = ps->speed/10;

	// calc target range and heading
	if( ps->stats[STAT_LOCKINFO] & LI_TRACKING ) {
		centity_t* target = &cg_entities[cent->currentState.tracktarget];
		vec3_t dir, angles;
		if( target->currentState.eType == ET_EXPLOSIVE ) {
			VectorSubtract( cgs.inlineModelMidpoints[target->currentState.modelindex], cent->lerpOrigin, dir );
		} else {
			VectorSubtract( target->lerpOrigin, cent->lerpOrigin, dir );
		}
		targetrange = VectorNormalize( dir );
		vectoangles( dir, angles );
		targetheading = 360 - angles[1];
		if( targetheading <= 0 ) targetheading += 360;
	} else {
		targetheading = -1;
		targetrange = -1;
	}

	// heading
	if( hud_heading.integer ) {
		value = 360 - (int)ps->vehicleAngles[1];
		if( value <= 0 ) value += 360;
		CG_Draw_HeadingTape( value, targetheading );
	}

	// Not for LQM's
	if(!(availableVehicles[vehicle].cat & CAT_LQM))
	{
		// speed
		if( hud_speed.integer ) {
			if( (availableVehicles[vehicle].cat & CAT_GROUND) || 
				(availableVehicles[vehicle].cat & CAT_BOAT) ) value = 10;
			else value = 50;
			CG_Draw_SpeedTape( ps->speed/10, availableVehicles[vehicle].stallspeed,
					availableVehicles[vehicle].stallspeed * SPEED_GREEN_ARC, value );
		}

		// altitude (not for ground-vehicles)
		if( (hud_altitude.integer || cg.Mode_MFD[MFD_1] == MFD_INFO || cg.Mode_MFD[MFD_2] == MFD_INFO) &&
			!(availableVehicles[vehicle].cat & CAT_GROUND) && !(availableVehicles[vehicle].cat & CAT_BOAT) )
		{
			trace_t	tr;
			vec3_t	start, end;
			VectorCopy( ps->origin, start );
			start[2] += availableVehicles[vehicle].mins[2];
			VectorCopy( start, end );
			end[2] -= 20000;
			CG_Trace( &tr, start, vec3_origin, vec3_origin, end, cg.snap->ps.clientNum, MASK_SOLID|MASK_WATER );
			altitude = (int)(tr.fraction * 20000);
			CG_Draw_AltTape(altitude);
		}

		// stall related
		if( speed <= stallspeed || (cent->currentState.ONOFF & OO_STALLED) ) stallcolor = HUD_RED;
		else if( speed <= stallspeed * 1.25 ) stallcolor = HUD_YELLOW;
		else stallcolor = HUD_GREEN;

		// mfd 1
		if( hud_mfd.integer ) {
			CG_Draw_MFD(MFD_1, vehicle, cent, targetrange, targetheading, altitude, ps, stallcolor);
		}

		// mfd 2
		if( hud_mfd2.integer ) {
			CG_Draw_MFD(MFD_2, vehicle, cent, targetrange, targetheading, altitude, ps, stallcolor);
		}

		// solid middle section
		if( ps->stats[STAT_MAX_HEALTH] ) {
			value = (100*ps->stats[STAT_HEALTH]/ps->stats[STAT_MAX_HEALTH]);
		} else value = 0;
		if( value > 100 ) value = 100;
		if( hud_center.integer ) {
			CG_Draw_Center(vehicle, value, ps->throttle);
		}

		// additional HUD info: health and throttle (redundant)
		CG_Draw_Redundant(vehicle, value, ps->throttle, ps->ammo, cent->currentState.weaponNum );

		// GPS
		if( cg.GPS ) {
			value = (int)ps->origin[0];
			value2 = (int)ps->origin[1];
			CG_Draw_Coords( value, value2 );
		}
		
		// stallwarning
		if( (ps->speed/10 <= availableVehicles[vehicle].stallspeed*1.5 || (cent->currentState.ONOFF & OO_STALLED)) &&
			!(cent->currentState.ONOFF & OO_LANDED) && ps->stats[STAT_HEALTH] > 0 && 
			availableVehicles[vehicle].cat != CAT_HELO) {
			float stallscale = 1.0f;
			if( speed >= stallspeed && !(cent->currentState.ONOFF & OO_STALLED) ) stallscale = 2.0f - ((float)speed/(float)stallspeed);
			DrawStringNew( 320, 360, stallscale, HUDColors[stallcolor], "STALL!", 0, 0, 3, CENTRE_JUSTIFY );
		}

		// lock warning
		if( ps->stats[STAT_LOCKINFO] & LI_BEING_LAUNCHED ) {
			DrawStringNew( 320, 60, 0.8f, HUDColors[HUD_RED], "LAUNCH!", 0, 0, 3, CENTRE_JUSTIFY );
		} else if( ps->stats[STAT_LOCKINFO] & LI_BEING_LOCKED ) {
			DrawStringNew( 320, 60, 1.0f, HUDColors[HUD_YELLOW], "LOCK!", 0, 0, 3, CENTRE_JUSTIFY );
		}
	}

	// cleanup
	cg.radarTargets = 0;
}

