// file v_ground.c by Bjoern Drabeck
// handles ground vehicle movement and sounds

#include "g_local.h"
#include "v_ground.h"


void SetGroundSpeed( edict_t *vehicle )
{
	float angle = vehicle->s.angles[0];
	float speedreduction = 0;
	float decel = vehicle->decel;

	if( vehicle->deadflag == DEAD_DEAD )
	{
		vehicle->actualspeed = 0;
		return;
	}
	// set targetspeed
	if( vehicle->thrust < THRUST_AFTERBURNER )
		vehicle->currenttop = (vehicle->topspeed / 4.0) * vehicle->thrust;
	else
		vehicle->currenttop = -(vehicle->topspeed/4.0);

	// adjust on slopes
	if( angle > 180 )
		angle -= 360;
	if( abs(angle) > 1 && abs(angle) <= 25)
		speedreduction = vehicle->topspeed*(angle/60);
	else if( abs(angle) > 15 && abs(angle) <= 35 )
		speedreduction = vehicle->topspeed*(angle/50);
	else if( abs(angle) > 35 )
		speedreduction = vehicle->topspeed*(angle/30);
	if( vehicle->ONOFF & BRAKE && vehicle->thrust == THRUST_0 )
		speedreduction = 0;

	// apply reduction
	vehicle->currenttop += speedreduction;

//	gi.bprintf( PRINT_HIGH, "red:%.1f cur:%.1f top:%.1f\n", speedreduction, vehicle->currenttop,
//		vehicle->topspeed );

	// handbrake stops you faster
	if( vehicle->actualspeed > 0 && vehicle->ONOFF & BRAKE )
		decel *= 2;

//	gi.bprintf( PRINT_HIGH, "%.1f %.1f %.1f\n", angle, vehicle->s.angles[0], speedreduction );

	// calculate the actual speed
	if( vehicle->actualspeed >= vehicle->currenttop )
	{
		vehicle->actualspeed -= decel;
		if( vehicle->actualspeed < vehicle->currenttop )
			vehicle->actualspeed = vehicle->currenttop;
	}
	else
	{
		vehicle->actualspeed += vehicle->accel;
		if( vehicle->actualspeed > vehicle->currenttop )
			vehicle->actualspeed = vehicle->currenttop;
	}

	GroundSound( vehicle );
}


void SteerGround( edict_t *vehicle )
{
	vec3_t 	forward, right, up;//, center;
//	vec3_t	frontleft, frontright, backleft, backright, centerleft, centerright;
//	vec3_t	fld, frd, bld, brd, centerdn, cld, crd;	// traceendpoints (fld = front left down)
//	double  cospitch, cosroll, sinroll, sinpitch;
//	trace_t	tfl,tfr,tbl,tbr,tc,tcl,tcr;	// tracelines (tfl = trace front left)
//	float 	targetpitch, targetroll;	// where shall we roll to ?
//	double	pitch1,pitch2,roll1,roll2;
//	float	vehlen, vehwid;		// vehicle dimensions
//	float	aflbl, afrbr, aflfr, ablbr; // angles (aflbl = angle frontleft-backleft
//	float   hdifflen; 	// height difference along length axis
//	double	hypflbl,hypfrbr,hypflfr,hypblbr; 		// hypothenuse
	float	height;
	vec3_t	angles,angles2;

	int		VM = vehicle->owner->client->pers.videomodefix;

	float	l1,l2,l3,l4,r1,r2;
	float w1,w2;
	vec3_t	v1,v2,v3,v4,b1,b2,b3,b4;
	trace_t	t1,t2,t3,t4;

	VectorClear( vehicle->avelocity );

	// adjust control sensitivity on zoom factor
	if( vehicle->fov == 60 )
	{
		vehicle->elevator /= 2;
		vehicle->aileron /= 2;
		vehicle->rudder /= 2;
	}
	else if( vehicle->fov == 30 )
	{
		vehicle->elevator /= 4;
		vehicle->aileron /= 4;
		vehicle->rudder /= 4;
	}
	else if( vehicle->fov == 10 )
	{
		vehicle->elevator /= 6;
		vehicle->aileron /= 6;
		vehicle->rudder /= 6;
	}

	// health-effects
	if( vehicle->owner->health >= 26 )
		vehicle->s.effects = 0;
	if( vehicle->owner->health < 26 && vehicle->owner->health > 10 )
	{
		vehicle->s.effects = EF_GRENADE;
	}
	else if( vehicle->owner->health <= 10 && vehicle->deadflag != DEAD_DEAD )
	{
		vehicle->s.effects = EF_ROCKET;
	}
	else
		vehicle->s.effects = 0;

	SetGroundSpeed( vehicle );

	if( vehicle->deadflag && vehicle->enemy )
	{
		vehicle->enemy->lockstatus = 0;
		vehicle->enemy = NULL;
	}
	if( vehicle->deadflag == DEAD_DEAD )
		return;


	// overturning ?
	vehicle->s.angles[0] = anglemod( vehicle->s.angles[0] );
	vehicle->s.angles[1] = anglemod( vehicle->s.angles[1] );

	// neu fuer videomodefix
	VectorCopy( vehicle->s.angles, angles );
//	if( vehicle->owner->client->pers.videomodefix == 1 )
//		angles[2] *= vehicle->owner->client->pers.videomodefix;


	// get sin and cos of angles
/*	cospitch = cos( DEG2RAD( vehicle->s.angles[0] ) );
	cosroll = cos( DEG2RAD( vehicle->s.angles[2] ) );
	sinpitch = sin( DEG2RAD( vehicle->s.angles[0] ) );
	sinroll = sin( DEG2RAD( vehicle->s.angles[2] ));

	// set the center above the vehicle
	VectorCopy( vehicle->s.origin, center );
	center[2] += 10;

	// set centerdn below vehicle
	VectorCopy( vehicle->s.origin, centerdn );
	centerdn[2] -= 50;

	// vehicle dimensions
	vehlen = (abs( vehicle->maxs[0] ) + abs( vehicle->mins[0] ))*cospitch;
	vehwid = (abs( vehicle->maxs[1] ) + abs( vehicle->mins[1] ))*cosroll;

	// get front and back edges
	AngleVectors( vehicle->s.angles, forward, right, NULL );
	forward[2] = right[2] = 0;
	VectorNormalize( forward );
	VectorNormalize( right );
	if( vehicle->actualspeed >= 0 ) // moving forward
	{
		VectorMA( center, (vehicle->maxs[0]*cospitch), forward, frontleft );
		VectorCopy( frontleft, frontright );
		VectorMA( frontright, (vehicle->maxs[1]*cosroll), right, frontright );
		VectorMA( frontleft, (vehicle->mins[1]*cosroll), right, frontleft );
		VectorMA( center, (vehicle->mins[0]*cospitch), forward, backleft );
		VectorCopy( backleft, backright );
		VectorMA( backleft, (vehicle->mins[1]*cosroll), right, backleft );
		VectorMA( backright, (vehicle->maxs[1]*cosroll), right, backright );
		VectorMA( center, (vehicle->maxs[1]*cosroll), right, centerright );
		VectorMA( center, (vehicle->mins[1]*cosroll), right, centerleft );
	}
	else // moving backwards
	{
		VectorMA( center, (vehicle->mins[0]*cospitch), forward, frontleft );
		VectorCopy( frontleft, frontright );
		VectorMA( frontright, (vehicle->maxs[1]*cosroll), right, frontright );
		VectorMA( frontleft, (vehicle->mins[1]*cosroll), right, frontleft );
		VectorMA( center, (vehicle->maxs[0]*cospitch), forward, backleft );
		VectorCopy( backleft, backright );
		VectorMA( backleft, (vehicle->mins[1]*cosroll), right, backleft );
		VectorMA( backright, (vehicle->maxs[1]*cosroll), right, backright );
		VectorMA( center, (vehicle->maxs[1]*cosroll), right, centerright );
		VectorMA( center, (vehicle->mins[1]*cosroll), right, centerleft );
	}
	// get bottom positions for trace
	VectorCopy( frontleft, fld );
	fld[2] -= 50;
	VectorCopy( frontright, frd );
	frd[2] -= 50;
	VectorCopy( backleft, bld );
	bld[2] -= 50;
	VectorCopy( backright, brd );
	brd[2] -= 50;
	VectorCopy( centerleft, cld );
	cld[2] -= 50;
	VectorCopy( centerright, crd );
	crd[2] -= 50;

	// tracelines down from edges
	tfl = gi.trace( frontleft, NULL, NULL, fld, vehicle, MASK_ALL );
	tfr = gi.trace( frontright, NULL, NULL, frd, vehicle, MASK_ALL );
	tbl = gi.trace( backleft, NULL, NULL, bld, vehicle, MASK_ALL );
	tbr = gi.trace( backright, NULL, NULL, brd, vehicle, MASK_ALL );
	tc = gi.trace( center, NULL, NULL, centerdn, vehicle, MASK_ALL );
	tcl = gi.trace( centerleft, NULL, NULL, cld, vehicle, MASK_ALL );
	tcr = gi.trace( centerright, NULL, NULL, crd, vehicle, MASK_ALL );
*/
//	debug check to see if positions are correct
//	gi.bprintf( PRINT_HIGH, "%.2f %.2f %.2f %.2f\n", tfl.endpos[2], tfr.endpos[2], tbl.endpos[2],
//			tbr.endpos[2] );

// 	gi.bprintf( PRINT_HIGH, "%.2f %.2f %.2f %.2f\n", hypflbl, hypfrbr, hypflfr, hypblbr );
//	gi.bprintf( PRINT_HIGH, "%.2f %.2f %.2f %.2f\n", pitch1, pitch2, roll1, roll2 );

	// roll the vehicle
//	vehicle->s.origin[2] = tc.endpos[2] + hdifflen * abs((vehicle->mins[0]+cospitch)/vehlen)
//		+ abs( vehicle->mins[2] )+15;

//	gi.bprintf( PRINT_HIGH, "tpitch: %.2f troll: %.2f pitch:%.2f\n", targetpitch, targetroll,
//   			vehicle->s.angles[0] );

	// angular adjustments
	VectorSet( v1, 0, angles[1], 0 );
	AngleVectors( v1, forward, right, up );
	VectorMA( vehicle->s.origin, 4,forward, v1 );
	VectorMA( v1, -4*VM,right, v2 );// right forward
	VectorMA( v1, 4*VM, right, v1 );// left forward
	VectorMA( vehicle->s.origin, -4,forward, v3 );
	VectorMA( v3, -4*VM, right, v4 );// right behind
	VectorMA( v3, 4*VM, right, v3 );// left behind
	VectorMA( v1, 8, up, v1 );
	VectorMA( v2, 8, up, v2 );
	VectorMA( v3, 8, up, v3 );
	VectorMA( v4, 8, up, v4 );
	VectorMA( v1, -32, up, b1 );
	VectorMA( v2, -32 ,up, b2 );
	VectorMA( v3, -32 ,up, b3 );
	VectorMA( v4, -32, up, b4 );
	t1 = gi.trace( v1, NULL, NULL, b1, vehicle, MASK_SOLID );
	t2 = gi.trace( v2, NULL, NULL, b2, vehicle, MASK_SOLID );
	t3 = gi.trace( v3, NULL, NULL, b3, vehicle, MASK_SOLID );
	t4 = gi.trace( v4, NULL, NULL, b4, vehicle, MASK_SOLID );
	l1 = t1.fraction;
	l2 = t2.fraction;
	l3 = t3.fraction;
	l4 = t4.fraction;
//	gi.bprintf( PRINT_HIGH, "%.2f % .2f %.2f %.2f \n", l1, l2, l3, l4);
//	gi.bprintf( PRINT_HIGH, "%.2f %.2f %.2f\n", l1,vehicle->s.origin[2],
//			t1.endpos[2]);

	if( l1 == 1 && l2 == 1 && l3 == 1 && l4 == 1 )
	{
		vehicle->avelocity[0] = 35;
		vehicle->deadflag = DEAD_DYING;
	}
	else
	{
		r1 = l1 - l2 + l3 - l4;
		r2 = l1 - l3 + l2 - l4;

		VectorSet( v1, r1, 0.5, r2 );
		VectorNormalize( v1 );
		VectorSet( v2, v1[0], v1[2], v1[1] );
		VectorScale( up, v2[2], up );
		VectorMA( up, v2[1], forward, up );
		VectorMA( up, v2[0], right, v3 );
		VectorNormalize( v3 );

		w2 = ( 4 - l1 - l2 - l3 - l4 ) * 64;
		if( w2 > w1 )
			w1 = w2;

		AngleVectors( angles, forward, right, up );
		vectoangles( v1, v2 );
		VectorSet( v3, 0, 2, 0 );
		v3[0] = anglemod( 0 - v2[0] );
		v3[1] = angles[1];
		v3[2] = anglemod( v2[1] - 90 );
		VectorSubtract( v3, angles, v4 );
		l1 = anglemod( v4[0] );
		if( l1 > 180 )
			l1 = l1 - 360;
		l2 = anglemod( v4[2] );
		if( l2 > 180 )
			l2 = l2 - 360;

		vehicle->avelocity[0] = l1 * 4;
		vehicle->avelocity[2] = l2 * 4;
	}

	// set height
	VectorSet( v1, 0, angles[1], 0 );
	AngleVectors( v1, forward, right, up );
	VectorMA( vehicle->s.origin, vehicle->maxs[0],forward, v1 );
	VectorMA( v1, vehicle->mins[1],right, v2 );// right forward
	VectorMA( v1, vehicle->maxs[1], right, v1 );// left forward
	VectorMA( vehicle->s.origin, vehicle->mins[0]/2,forward, v3 );
	VectorMA( v3, vehicle->mins[1], right, v4 );// right behind
	VectorMA( v3, vehicle->maxs[1], right, v3 );// left behind
	VectorMA( v1, 4, up, v1 );
	VectorMA( v2, 4, up, v2 );
	VectorMA( v3, 4, up, v3 );
	VectorMA( v4, 4, up, v4 );
	VectorMA( v1, -32, up, b1 );
	VectorMA( v2, -32 ,up, b2 );
	VectorMA( v3, -32 ,up, b3 );
	VectorMA( v4, -32, up, b4 );
	t1 = gi.trace( v1, NULL, NULL, b1, vehicle, MASK_ALL );
	t2 = gi.trace( v2, NULL, NULL, b2, vehicle, MASK_ALL );
	t3 = gi.trace( v3, NULL, NULL, b3, vehicle, MASK_ALL );
	t4 = gi.trace( v4, NULL, NULL, b4, vehicle, MASK_ALL );
	height = t1.endpos[2];
	if( t2.endpos[2] > height )
		height = t2.endpos[2];
	if( t3.endpos[2] > height )
		height = t3.endpos[2];
	if( t4.endpos[2] > height )
		height = t4.endpos[2];
	vehicle->s.origin[2] = height + abs(vehicle->mins[2]) + 1.5;

//	vehicle->s.origin[2] = tc.endpos[2] + 20;

	// adjusting turret and gun to hull
//	VectorCopy( vehicle->avelocity, vehicle->activator->avelocity );
//	VectorCopy( vehicle->avelocity, vehicle->movetarget->avelocity );
//	VectorClear( vehicle->movetarget->avelocity );

	// neu fuer videomodefix
	VectorCopy( vehicle->activator->s.angles, angles2 );
//	if( vehicle->owner->client->pers.videomodefix == 1 )
//		angles2[2] *= -vehicle->owner->client->pers.videomodefix;

	// test
	VectorSet( v1, 0, angles2[1], 0 );
	AngleVectors( v1, forward, right, up );
	VectorMA( vehicle->s.origin, 4,forward, v1 );
	VectorMA( v1, -4*VM,right, v2 );// right forward
	VectorMA( v1, 4*VM, right, v1 );// left forward
	VectorMA( vehicle->s.origin, -4,forward, v3 );
	VectorMA( v3, -4*VM, right, v4 );// right behind
	VectorMA( v3, 4*VM, right, v3 );// left behind
	VectorMA( v1, 8, up, v1 );
	VectorMA( v2, 8, up, v2 );
	VectorMA( v3, 8, up, v3 );
	VectorMA( v4, 8, up, v4 );
	VectorMA( v1, -32, up, b1 );
	VectorMA( v2, -32 ,up, b2 );
	VectorMA( v3, -32 ,up, b3 );
	VectorMA( v4, -32, up, b4 );
	t1 = gi.trace( v1, NULL, NULL, b1, vehicle, MASK_SOLID );
	t2 = gi.trace( v2, NULL, NULL, b2, vehicle, MASK_SOLID );
	t3 = gi.trace( v3, NULL, NULL, b3, vehicle, MASK_SOLID );
	t4 = gi.trace( v4, NULL, NULL, b4, vehicle, MASK_SOLID );
	l1 = t1.fraction;
	l2 = t2.fraction;
	l3 = t3.fraction;
	l4 = t4.fraction;
	if( l1 == 1 && l2 == 1 && l3 == 1 && l4 == 1 )
	{
//		vehicle->activator->avelocity[0] = 35;
//		vehicle->deadflag = DEAD_DYING;
	}
	else
	{
		r1 = l1 - l2 + l3 - l4;
		r2 = l1 - l3 + l2 - l4;

		VectorSet( v1, r1, 0.5, r2 );
		VectorNormalize( v1 );
		VectorSet( v2, v1[0], v1[2], v1[1] );
		VectorScale( up, v2[2], up );
		VectorMA( up, v2[1], forward, up );
		VectorMA( up, v2[0], right, v3 );
		VectorNormalize( v3 );

		w2 = ( 4 - l1 - l2 - l3 - l4 ) * 64;
		if( w2 > w1 )
			w1 = w2;

		AngleVectors( angles2, forward, right, up );
		vectoangles( v1, v2 );
		VectorSet( v3, 0, 2, 0 );
		v3[0] = anglemod( 0 - v2[0] );
		v3[1] = angles2[1];
		v3[2] = anglemod( v2[1] - 90 );
		VectorSubtract( v3, angles2, v4 );
		l1 = anglemod( v4[0] );
		if( l1 > 180 )
			l1 = l1 - 360;
		l2 = anglemod( v4[2] );
		if( l2 > 180 )
			l2 = l2 - 360;

		vehicle->activator->avelocity[0] = l1 * 4;
		vehicle->activator->avelocity[2] = l2 * 4;
	}

	VectorCopy( vehicle->activator->avelocity, vehicle->movetarget->avelocity );

	// vehicle/turret turning
	if( vehicle->actualspeed >= 0 )
		vehicle->avelocity[1] = vehicle->rudder;
	else
		vehicle->avelocity[1] = -vehicle->rudder;

	vehicle->activator->avelocity[1] = vehicle->aileron*2;//*cos(vehicle->s.angles[0]);
	vehicle->movetarget->avelocity[1] = vehicle->aileron*2;
	// turret lock
	if( vehicle->owner->client->pers.autocoord && !vehicle->aileron )
	{
		vehicle->movetarget->avelocity[1] = vehicle->activator->avelocity[1] =
			vehicle->avelocity[1];
	}
	// turret turnback
	if( vehicle->owner->client->pers.autoroll && !vehicle->aileron &&
   		!vehicle->activator->avelocity[1] && level.time > vehicle->timestamp )
	{
		if( (int)vehicle->s.angles[1] != (int)vehicle->activator->s.angles[1] )
		{
			float 	diff;
			int		speed;

			diff = vehicle->activator->s.angles[1] - vehicle->s.angles[1];
			if( diff > 180 )
				diff -= 360;
			else if( diff < -180 )
				diff += 360;
			if( abs(diff) >= 16 )
				speed = 50;
			else if( abs(diff) >= 3 )
				speed = 15;
			else
			{
				vehicle->activator->s.angles[1] = vehicle->movetarget->s.angles[1] =
					vehicle->s.angles[1];
				diff = 0;
			}
			if( diff > 0 )
			{
				vehicle->movetarget->avelocity[1] = vehicle->activator->avelocity[1] = -speed;
			}
			else if( diff < 0 )
			{
				vehicle->movetarget->avelocity[1] = vehicle->activator->avelocity[1] = speed;
			}
		}
	}

//	vehicle->activator->avelocity[2] = vehicle->movetarget->avelocity[2] +=
//			vehicle->aileron*2*sin(vehicle->s.angles[0])*VM;
//	gi.bprintf( PRINT_HIGH, "%.2f %.2f\n", vehicle->activator->s.angles[0], vehicle->movetarget->s.angles[0] );

	if( vehicle->movetarget->s.angles[0] < vehicle->activator->s.angles[0]+10 &&
		vehicle->movetarget->s.angles[0] > vehicle->activator->s.angles[0]-75 )
		vehicle->movetarget->avelocity[0] += vehicle->elevator;
	else
		vehicle->movetarget->avelocity[0] = 0;
	if( (vehicle->movetarget->s.angles[0] == vehicle->activator->s.angles[0]-75
				&& vehicle->elevator > 0) ||
   		(vehicle->movetarget->s.angles[0] == vehicle->activator->s.angles[0]+10
	   		 && vehicle->elevator < 0 ) )
		vehicle->movetarget->avelocity[0] += vehicle->elevator;
	if( vehicle->movetarget->s.angles[0] < vehicle->activator->s.angles[0]-75 )
		vehicle->movetarget->s.angles[0] = vehicle->activator->s.angles[0]-75;
	else if( vehicle->movetarget->s.angles[0] > vehicle->activator->s.angles[0]+10 )
		vehicle->movetarget->s.angles[0] = vehicle->activator->s.angles[0]+10;

	// give speed
	AngleVectors( angles, forward, NULL, NULL );
	VectorScale( forward, vehicle->actualspeed/4, vehicle->velocity );
	// the following part is also handled in g_phys.c!
/*	VectorCopy( vehicle->velocity, vehicle->activator->velocity );
	VectorCopy( vehicle->velocity, vehicle->movetarget->velocity );
	if( !VectorCompare(vehicle->s.origin, vehicle->movetarget->s.origin) )
		VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	if( !VectorCompare(vehicle->s.origin, vehicle->activator->s.origin) )
		VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
*/
	vehicle->nextthink = level.time + 0.1;

//	gi.bprintf( PRINT_HIGH, "%.2f %.2f %.2f\n", vehicle->s.angles[0], vehicle->s.angles[1],
//		vehicle->s.angles[2] );

	if( !vehicle->deadflag )
	{
		if( vehicle->owner->client->pers.secondary == FindItem( "SW Launcher" ) ||
			 vehicle->owner->client->pers.secondary == FindItem( "ST Launcher" ) ||
			 vehicle->owner->client->pers.secondary == FindItem( "AM Launcher" ) ||
			 vehicle->owner->client->pers.secondary == FindItem( "PH Launcher" ) )
			SidewinderLockOn( vehicle );
		else if( vehicle->owner->client->pers.secondary == FindItem( "ATGM Launcher" ) ||
				vehicle->owner->client->pers.secondary == FindItem ("Maverick Launcher") )
			ATGMLockOn( vehicle );
		else if( vehicle->enemy )
		{
			vehicle->enemy->lockstatus = 0;
			vehicle->enemy = NULL;
		}
	}
	vehicle->radio_target = NULL;
	vehicle->ONOFF &= ~IS_CHANGEABLE;

	if( vehicle->aileron != 0 || vehicle->elevator != 0 )
		vehicle->timestamp = level.time + 3.0;

	if( vehicle->ONOFF & PICKLE_ON )
		Cmd_Pickle_f( vehicle->owner );

//	gi.bprintf( PRINT_HIGH, "%d %d %d\n", vehicle->owner->veh_category,
//		vehicle->owner->veh_class, vehicle->owner->veh_number );
	gi.linkentity( vehicle );
	gi.linkentity( vehicle->movetarget );
	gi.linkentity( vehicle->activator );
}



void GroundSound( edict_t *vehicle )
{
	float speedindex, volume;

	if( !vehicle->temptime )
		vehicle->temptime = level.time;

	if( level.time > vehicle->temptime && vehicle->ONOFF & PILOT_ONBOARD )
	{
		speedindex = floor( vehicle->actualspeed/ 40 );
		volume = vehicle->thrust * 0.1 + 0.3;

		if( speedindex <= 0 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank0.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 1 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank1.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 2 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank2.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 3 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank3.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 4 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank4.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 5 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank5.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 6 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank6.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex == 7 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank7.wav" ), volume, ATTN_NORM, 0 );
		else if( speedindex >= 8 )
			gi.sound( vehicle, CHAN_BODY, gi.soundindex( "engines/tank8.wav" ), volume, ATTN_NORM, 0 );
		vehicle->temptime = level.time + 0.2;
	}

}


void ground_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( strcmp( other->classname, "trigger_radio" ) != 0 )
	{
		self->radio_target = NULL;
	}
	if( strcmp( other->classname, "rocket" ) == 0 ||
   		strcmp( other->classname, "sidewinder" ) == 0 ||
		strcmp( other->classname, "bolt" ) == 0 ||
		strcmp( other->classname, "ATGM" ) == 0 ||
		strcmp( other->classname, "shell" ) == 0 ||
		strcmp( other->classname, "bomb" ) == 0 )
	{
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		return;
	}
	if( self->deadflag == DEAD_DYING )
	{
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		self->health = 0;

		gi.bprintf( PRINT_MEDIUM, "%s tried to fly.\n", self->owner->client->pers.netname );

		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);

//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
		ThrowDebris( self, "models/objects/debris3/tris.md2",
				5, self->s.origin );
		ThrowDebris( self, "models/objects/debris2/tris.md2",
				5, self->s.origin );
		VehicleExplosion( self );
		SetVehicleModel( self );
		self->thrust = THRUST_0;
		self->deadflag = DEAD_DEAD;
		VectorClear( self->owner->velocity );
	}

	else
	{
//		self->actualspeed = 0;
//		VectorClear( self->velocity );
	}

}

