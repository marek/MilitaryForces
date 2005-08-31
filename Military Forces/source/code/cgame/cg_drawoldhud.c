/*
 * $Id: cg_drawoldhud.c,v 1.2 2005-08-31 19:20:06 thebjoern Exp $
*/

#include "cg_local.h"







/*
================
CG_DrawStatusBarFlag

================
*/
static void CG_DrawStatusBarFlag( float x, int team ) {
	CG_DrawFlagModel( x, 480 - ICON_SIZE, ICON_SIZE, ICON_SIZE, team, false );
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
			if( (availableVehicles[otherveh].cat & CAT_GROUND) ||
				(availableVehicles[otherveh].cat & CAT_BOAT) ) {
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
		CG_DrawPic( 558 + dir[0], 386 - dir[1], 8, 8, cgs.media.radarIcons[icon] );
		// check if we should draw any more targets
		drawnTargets++;
		if( drawnTargets >= cg_radarTargets.integer ) break;
	}

	cg.radarTargets = 0;
}


/*
================
CG_DrawStatusBar_MFQ3

================
*/
void CG_DrawStatusBar_MFQ3( void ) {
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

	// aiming/targetting reticles
	CG_Draw_Reticles();

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
		if( (availableVehicles[vehicle].cat & CAT_PLANE) ||
			(availableVehicles[vehicle].cat & CAT_HELO) ||
			(availableVehicles[vehicle].cat & CAT_BOAT) ) {
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
