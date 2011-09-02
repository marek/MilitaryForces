/************************************************/
/* Filename: aq2_cam.c  					    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 1999-07-10							*/
/*												*/
/* Description:									*/
/* NewAirQuake2 camera-specific stuff   		*/
/*												*/
/************************************************/

#include "g_local.h"












// **********************************
// Removes camera from the cameralist
// **********************************
void RemoveCamera( edict_t *ent )
{
	if( !ent->client )
		return;

	if( !ent->client->cameratarget )
		return;

	if( ent->client->prevcam != NULL )		
	{
		if( ent->client->nextcam != NULL )	
			ent->client->prevcam->client->nextcam = ent->client->nextcam;
		else							
			ent->client->prevcam->client->nextcam = NULL;
	}
	if( ent->client->nextcam != NULL )		
	{
		if( ent->client->prevcam != NULL )
			ent->client->nextcam->client->prevcam = ent->client->prevcam;
		else
			ent->client->nextcam->client->prevcam = NULL;
	}
	if( ent->client->nextcam == NULL && ent->client->prevcam == NULL )
	{	
		ent->client->cameratarget->camera = NULL;
	}
	ent->client->cameratarget = NULL;
	ent->client->nextcam = NULL;
	ent->client->prevcam = NULL;
}








void GotoOwnVehicle( edict_t *ent )
{
	edict_t *cam, *prev;	// this is an ent (ie player)
	edict_t *from = ent->vehicle;	// this is a vehicle
	int     done = 0;

	if( ent->client->cameratarget == ent->vehicle )
		return;

	// found one
	if( from->camera != NULL )
	{
		prev = from->camera;
		cam = from->camera->client->nextcam;
		while( cam != NULL )
		{
			prev = from->camera;
			cam = cam->client->nextcam;
		}
		// remove camera from old vehicle
		if( ent->client->prevcam != NULL )		// I am not the first cam
		{
			if( ent->client->nextcam != NULL )	// there is one after me
				ent->client->prevcam->client->nextcam = ent->client->nextcam;
			else								// I am the last one
				ent->client->prevcam->client->nextcam = NULL;
		}
		if( ent->client->nextcam != NULL )		
		{
			if( ent->client->prevcam != NULL )
				ent->client->nextcam->client->prevcam = ent->client->prevcam;
			else
				ent->client->nextcam->client->prevcam = NULL;
		}
		if( ent->client->nextcam == NULL && ent->client->prevcam == NULL )
		{	
			ent->client->cameratarget->camera = NULL;
		}
		// add camera to new vehicle
		prev->client->nextcam = ent;
		ent->client->cameratarget = from;
		ent->client->nextcam = NULL;
		ent->client->prevcam = prev;
	}
	else
	{
		from->camera = ent;
		ent->client->cameratarget = from;
		ent->client->nextcam = NULL;
	}
	ent->client->cameradistance = ent->client->cameratarget->movedir[0];
	ent->client->cameraheight = ent->client->cameratarget->movedir[1];

}








// ********************************
// Sets the cameratarget explicitly
// ********************************
void SetCamTarget( edict_t *ent, edict_t *target )
{
	edict_t *cam, *prev;	// this is an ent (ie player)

	// found one
	if( target->camera != NULL )
	{
		prev = target->camera;
		cam = target->camera->client->nextcam;
		while( cam != NULL )
		{
			prev = target->camera;
			cam = cam->client->nextcam;
		}
		// remove camera from old vehicle
		if( ent->client->prevcam != NULL )		// I am not the first cam
		{
			if( ent->client->nextcam != NULL )	// there is one after me
				ent->client->prevcam->client->nextcam = ent->client->nextcam;
			else								// I am the last one
				ent->client->prevcam->client->nextcam = NULL;
		}
		if( ent->client->nextcam != NULL )		
		{
			if( ent->client->prevcam != NULL )
				ent->client->nextcam->client->prevcam = ent->client->prevcam;
			else
				ent->client->nextcam->client->prevcam = NULL;
		}
		if( ent->client->nextcam == NULL && ent->client->prevcam == NULL )
		{	
			ent->client->cameratarget->camera = NULL;
		}
		// add camera to new vehicle
		prev->client->nextcam = ent;
		ent->client->cameratarget = target;
		ent->client->nextcam = NULL;
		ent->client->prevcam = prev;
	}
	else
	{
		target->camera = ent;
		ent->client->cameratarget = target;
		ent->client->nextcam = NULL;
	}
}








// ***************************
// Finds the next cameratarget
// ***************************
void FindNextCamTarget( edict_t *ent )
{
	edict_t *cam, *prev;	// this is an ent (ie player)
	edict_t *from = ent->client->cameratarget;	// this is a vehicle
	int     done = 0;

	from++;

	for ( ; from < &g_edicts[globals.num_edicts] ; from++)// start from your vehicle upwards
	{
		if( !from->inuse )
			continue;
		if( strcmp( from->classname, "plane" ) != 0 &&
			strcmp( from->classname, "helo" ) != 0 &&
			strcmp( from->classname, "ground" ) != 0 &&
			strcmp( from->classname, "LQM" ) != 0 )
			continue;
		done = 1;
		break;
	}
	if( !done )	// nothing found, return to beginning
	{
		from = &g_edicts[0];

		for ( ; from < ent->client->cameratarget ; from++)
		{
			if( !from->inuse )
				continue;
			if( strcmp( from->classname, "plane" ) != 0 &&
				strcmp( from->classname, "helo" ) != 0 &&
				strcmp( from->classname, "ground" ) != 0 &&
				strcmp( from->classname, "LQM" ) != 0 )
				continue;
//			if( !(from->ONOFF & ONOFF_PILOT_ONBOARD) )
//				continue;
			done = 1;
			break;
		}		
	}
	if( !done )
	{
		gi.bprintf( PRINT_MEDIUM, "No cameratarget found!\n" );
		return;
	}
	// found one
	if( from->camera != NULL )
	{
		prev = from->camera;
		cam = from->camera->client->nextcam;
		while( cam != NULL )
		{
			prev = from->camera;
			cam = cam->client->nextcam;
		}
		// remove camera from old vehicle
		if( ent->client->prevcam != NULL )		// I am not the first cam
		{
			if( ent->client->nextcam != NULL )	// there is one after me
				ent->client->prevcam->client->nextcam = ent->client->nextcam;
			else								// I am the last one
				ent->client->prevcam->client->nextcam = NULL;
		}
		if( ent->client->nextcam != NULL )		
		{
			if( ent->client->prevcam != NULL )
				ent->client->nextcam->client->prevcam = ent->client->prevcam;
			else
				ent->client->nextcam->client->prevcam = NULL;
		}
		if( ent->client->nextcam == NULL && ent->client->prevcam == NULL )
		{	
			ent->client->cameratarget->camera = NULL;
		}
		// add camera to new vehicle
		prev->client->nextcam = ent;
		ent->client->cameratarget = from;
		ent->client->nextcam = NULL;
		ent->client->prevcam = prev;
	}
	else
	{
		from->camera = ent;
		ent->client->cameratarget = from;
		ent->client->nextcam = NULL;
	}
	ent->client->cameradistance = ent->client->cameratarget->movedir[0];
	ent->client->cameraheight = ent->client->cameratarget->movedir[1];
}











// *******************************
// Finds the previous cameratarget
// *******************************
void FindPrevCamTarget( edict_t *ent )
{
	edict_t *cam, *prev;	// this is an ent (ie player)
	edict_t *from = ent->client->cameratarget;	// this is a vehicle
	int     done = 0;

	from--;

	for ( ; from > &g_edicts[0] ; from--)// start from your vehicle downwards
	{
		if( !from->inuse )
			continue;
		if( strcmp( from->classname, "plane" ) != 0 &&
			strcmp( from->classname, "helo" ) != 0 &&
			strcmp( from->classname, "ground" ) != 0 &&
			strcmp( from->classname, "LQM" ) != 0 )
			continue;
		done = 1;
		break;
	}
	if( !done )	// nothing found, return to beginning
	{
		from = &g_edicts[globals.num_edicts-1];

		for ( ; from > ent->client->cameratarget ; from--)
		{
			if( !from->inuse )
				continue;
			if( strcmp( from->classname, "plane" ) != 0 &&
				strcmp( from->classname, "helo" ) != 0 &&
				strcmp( from->classname, "ground" ) != 0 &&
				strcmp( from->classname, "LQM" ) != 0 )
				continue;
			done = 1;
			break;
		}		
	}
	if( !done )
	{
		gi.bprintf( PRINT_MEDIUM, "No cameratarget found!\n" );
		return;
	}
	// found one
	if( from->camera != NULL )
	{
		prev = from->camera;
		cam = from->camera->client->nextcam;
		while( cam != NULL )
		{
			prev = from->camera;
			cam = cam->client->nextcam;
		}
		// remove camera from old vehicle
		if( ent->client->prevcam != NULL )		// I am not the first cam
		{
			if( ent->client->nextcam != NULL )	// there is one after me
				ent->client->prevcam->client->nextcam = ent->client->nextcam;
			else								// I am the last one
				ent->client->prevcam->client->nextcam = NULL;
		}
		if( ent->client->nextcam != NULL )		
		{
			if( ent->client->prevcam != NULL )
				ent->client->nextcam->client->prevcam = ent->client->prevcam;
			else
				ent->client->nextcam->client->prevcam = NULL;
		}
		if( ent->client->nextcam == NULL && ent->client->prevcam == NULL )
		{	
			ent->client->cameratarget->camera = NULL;
		}
		// add camera to new vehicle
		prev->client->nextcam = ent;
		ent->client->cameratarget = from;
		ent->client->nextcam = NULL;
		ent->client->prevcam = prev;
	}
	else
	{
		from->camera = ent;
		ent->client->cameratarget = from;
		ent->client->nextcam = NULL;
	}
	ent->client->cameradistance = ent->client->cameratarget->movedir[0];
	ent->client->cameraheight = ent->client->cameratarget->movedir[1];

}






void SetCamera( edict_t *ent )
{
	vec3_t	viewangle, targetangle;
	vec3_t	forward, up, campos;
	trace_t	tr;
	float	distance;
	int		VM = (ent->client->FLAGS & CLIENT_NONGL_MODE)? 1 : -1; 

	if( !ent->client->cameratarget )
		return;

	// set the view-direction of the camera
	if( !(ent->client->REPEAT & REPEAT_FREELOOK) )
	{
		viewangle[0] = ent->client->cameratarget->s.angles[0];
		viewangle[1] = ent->client->cameratarget->s.angles[1];
		viewangle[2] = ent->client->cameratarget->s.angles[2]*VM;
		switch( ent->client->cameramode )
		{
		case CM_HULL_FORWARD:
		case CM_TURRET_FORWARD:
		case CM_MISSILE_FORWARD:
			viewangle[1] = ent->client->cameratarget->s.angles[1];
			if( rollcamera->value )
				viewangle[2] = ent->client->cameratarget->s.angles[2]*VM;
			else
			{
				if( ent->client->cameratarget->s.angles[0] > 270 || ent->client->cameratarget->s.angles[0] <= 90 )
					viewangle[2] = 0;
				else
					viewangle[2] = 180;
			}
			if( strcmp( ent->client->cameratarget->classname, "LQM" ) == 0 
					&& !(ent->client->cameratarget->ONOFF & ONOFF_AIRBORNE) )
				viewangle[0] = ent->client->cameratarget->ideal_yaw;
			break;
		case CM_HULL_BACK:
		case CM_TURRET_BACK:
		case CM_MISSILE_BACK:
			viewangle[1] += 180;
			viewangle[2] *= -1;
			viewangle[0] *= -1;
			break;
		case CM_HULL_LEFT:			 
		case CM_TURRET_LEFT:
		case CM_MISSILE_LEFT:
			VectorCopy( ent->client->cameratarget->s.angles, targetangle );
			targetangle[2] *= VM;
			targetangle[2] = anglemod( targetangle[2] );
			AngleVectors( targetangle, NULL, up, NULL );
			VectorInverse( up );
			vectoangles( up, viewangle );
//			viewangle[1] += 90;
			break;
		case CM_HULL_RIGHT:
		case CM_TURRET_RIGHT:
		case CM_MISSILE_RIGHT:
			VectorCopy( ent->client->cameratarget->s.angles, targetangle );
			targetangle[2] *= VM;
			targetangle[2] = anglemod( targetangle[2] );
			AngleVectors( targetangle, NULL, up, NULL );
			vectoangles( up, viewangle );
//			viewangle[1] += 270;
			break;
		case CM_HULL_UP:
		case CM_TURRET_UP:
		case CM_MISSILE_UP:
			VectorCopy( ent->client->cameratarget->s.angles, targetangle );
			targetangle[2] *= VM;
			targetangle[2] = anglemod( targetangle[2] );
			AngleVectors( targetangle, NULL, NULL, up );
			vectoangles( up, viewangle );
//			gi.bprintf( PRINT_HIGH, "%.0f %.0f %.0f\n", targetangle[0], targetangle[1], targetangle[2] );
//			viewangle[0] -= 90;
			break;
		case CM_HULL_DOWN:		 
		case CM_TURRET_DOWN:
		case CM_MISSILE_DOWN:
			VectorCopy( ent->client->cameratarget->s.angles, targetangle );
			targetangle[2] *= VM;
			targetangle[2] = anglemod( targetangle[2] );
			AngleVectors( targetangle, NULL, NULL, up );
			VectorInverse( up );
			vectoangles( up, viewangle );
//			viewangle[0] += 90;
			break;
		}
		viewangle[0] = anglemod( viewangle[0] );
		viewangle[1] = anglemod( viewangle[1] );
		viewangle[2] = anglemod( viewangle[2] );
	}
	else
	{
		ent->client->cameraposition[0] = anglemod( ent->client->cameraposition[0] );
		ent->client->cameraposition[1] = anglemod( ent->client->cameraposition[1] );
		ent->client->cameraposition[2] = anglemod( ent->client->cameraposition[2] );
		VectorCopy( ent->client->cameraposition, viewangle );
	}

	// check distance
	if( ent->client->cameradistance > -( abs(ent->client->cameratarget->maxs[0]) + abs(ent->client->cameratarget->mins[0]) ) )
		ent->client->cameradistance = -( abs(ent->client->cameratarget->maxs[0]) + abs(ent->client->cameratarget->mins[0]) );

	VectorCopy( viewangle, ent->client->v_angle );
	VectorCopy( ent->client->v_angle, ent->client->ps.viewangles );

	// set the position of the camera
	AngleVectors (viewangle, forward, NULL, up);
	VectorMA( ent->client->cameratarget->s.origin, ent->client->cameradistance, forward, campos );
	VectorMA( campos, ent->client->cameraheight, up, campos );

	tr = gi.trace( ent->client->cameratarget->s.origin, NULL, NULL, campos, ent->client->cameratarget, MASK_SOLID );

	if( tr.fraction < 1 )
	{
		VectorSubtract( tr.endpos, ent->client->cameratarget->s.origin, campos );
		distance = VectorLength( campos );
		distance -= 12;
		VectorMA( ent->client->cameratarget->s.origin, -distance, forward, ent->s.origin );
	}
	else
		VectorCopy( campos, ent->s.origin );

}










void FindCameraPoint( edict_t *ent, vec3_t origin, vec3_t angles )
{
	edict_t	*spot = NULL;
	int count = 0, which = 0, loop;

	if( deathmatch->value )
	{
		if( (spot = G_Find (spot, FOFS(classname), "info_player_deathmatch")) == NULL )
		{
			gi.error( "No deathmatch spawn point!" );
		}
	}
	else
	{
		if( (spot = G_Find (spot, FOFS(classname), "info_player_start")) == NULL )
		{
			gi.error( "No single player spawn point!" );
		}
	}

	spot = NULL;
	while( (spot = G_Find( spot, FOFS(classname), "misc_camera" )) != NULL )
	{
		count ++;
	}

//	gi.bprintf( PRINT_HIGH, "%d misc_camera entities found!\n", count );

	spot = NULL;
	if( count > 0 )
	{
		which = (int)(random()*count + 1);
//		gi.bprintf( PRINT_HIGH, "Selected: %d\n", which );
		for( loop = 0; loop < which; loop++ )
			spot = G_Find( spot, FOFS(classname), "misc_camera" );
	}
	else
	{
		if( deathmatch->value )
		{
			spot = SelectDeathmatchSpawnPoint ();
//			gi.bprintf( PRINT_HIGH, "Deathmatch Spawn point selected!\n" );
		}
		else
		{
			spot = G_Find( spot, FOFS(classname), "info_player_start" );
//			gi.bprintf( PRINT_HIGH, "Single Player Spawn Point selected!\n" );
		}
	}

	if( spot == NULL )
		gi.error( "No misc_camera, info_player start and info_player_deathmatch!!\n" );

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);
}
