// file: p_camera.c by Bjoern Drabeck
// this handles the camera-movement and commands

#include "g_local.h"
#include "p_camera.h"



void SetCamera (edict_t *ent)
{
	vec3_t			forward, up;
	vec3_t			newpos;
	vec3_t			angles, dir,dir2;
	int 			content = 0;
	trace_t			trace;
	int				mode = 0;

	if( strcmp(ent->vehicle->classname,"ground") == 0 )
		mode = -18;
	else if( strcmp(ent->vehicle->classname, "helo" ) == 0 )
		mode = -24;

	VectorCopy( ent->cam_target->s.angles, angles );

	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -ent->client->pers.videomodefix;

	AngleVectors( angles, forward, NULL, up );

	if( strcmp( ent->vehicle->classname, "helo" ) == 0 &&
			ent->client->cameramode == 2 )
	{
		forward[2] = 0;
		VectorNormalize( forward );
		mode = -40;
	}

	VectorScale( forward, -(ent->client->pers.cameradistance)+
			ent->cam_target->mins[0]+mode, forward );

	if( strcmp( ent->vehicle->classname, "helo" ) == 0 &&
			ent->client->cameramode == 2 )
		VectorScale( up, 24, up );
	else
		VectorScale( up, ent->client->pers.cameraheight, up );
	VectorAdd( ent->cam_target->s.origin, forward, newpos );

	if( ent->client->cameramode < 3 ||
   		( ent->client->cameramode == 3 && !ent->goalentity ) )
	{
		if( ent->cam_target->deadflag == DEAD_NO )
			VectorAdd( newpos, up, newpos );
		else if( ent->cam_target->deadflag == DEAD_DEAD )
			newpos[2] += ent->client->pers.cameraheight;
		VectorClear( ent->velocity );
		trace = gi.trace( ent->cam_target->s.origin, NULL, NULL, newpos,
   				 ent->cam_target, MASK_SOLID );// changed from MASK_ALL
		if( trace.fraction == 1.0 )
		{
			VectorCopy( newpos, ent->s.origin );
		}
		else
		{
			VectorSubtract( trace.endpos, ent->cam_target->s.origin, forward );
			VectorNormalize( forward );
			VectorScale( forward, -5, forward );
			VectorAdd( trace.endpos, forward, trace.endpos );
			VectorCopy( trace.endpos, ent->s.origin );
		}
	}
	else
	{
	VectorSubtract( newpos, ent->s.origin, dir );
	VectorCopy( dir, dir2 );
	VectorNormalize( dir2 );
	VectorScale( dir2, VectorLength(ent->cam_target->velocity), ent->velocity );
	}

	// add height to camera for ground vehicles looking up
	if( strcmp( ent->vehicle->classname, "ground" ) == 0 &&
		ent->client->cameramode == 2 )
	{
		ent->s.origin[2] -= (ent->client->ps.viewangles[0]/3);
	}

}



void Cmd_CameraNear_f (edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	ent->client->pers.cameradistance -= 4;
	if( ent->client->pers.cameradistance < 8 )
	{
		ent->client->pers.cameradistance = 8;
		gi.cprintf( ent, PRINT_HIGH, "Camera cannot be closer!\n" );
	}
//	if( !deathmatch->value )
//		AQ2SaveVars( ent->client );
}

void Cmd_CameraFar_f (edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	ent->client->pers.cameradistance += 4;
	if( ent->client->pers.cameradistance > 80 )
	{
		ent->client->pers.cameradistance = 80;
		gi.cprintf( ent, PRINT_HIGH, "Camera cannot be farther away!\n" );
	}
//	if( !deathmatch->value )
//		AQ2SaveVars( ent->client );
}


void Cmd_CameraUp_f (edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	ent->client->pers.cameraheight += 3;
	if( ent->client->pers.cameraheight > 46 )
	{
		ent->client->pers.cameraheight = 46;
		gi.cprintf( ent, PRINT_HIGH, "Camera cannot be higher!\n" );
	}
//	if( !deathmatch->value )
//		AQ2SaveVars( ent->client );
}

void Cmd_CameraDown_f (edict_t *ent)
{
	if( !ent->client->veh_selected )
		return;

	ent->client->pers.cameraheight -= 3;
	if( ent->client->pers.cameraheight < 13 )//statt 13
	{
		ent->client->pers.cameraheight = 13;//statt 13
		gi.cprintf( ent, PRINT_HIGH, "Camera cannot be lower!\n" );
	}
//	if( !deathmatch->value )
//		AQ2SaveVars( ent->client );
}



void Cmd_CameraMode_f( edict_t *ent )
{
	vec3_t	forward, up;

	switch( ent->client->cameramode )
	{
		case 1:
				if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
				{
					gi.cprintf( ent, PRINT_MEDIUM, "Missile-View ON.\n" );
					ent->client->cameramode = 3;
					if( ent->goalentity )
					{
						ent->cam_target = ent->goalentity;

						AngleVectors( ent->cam_target->s.angles, forward, NULL, up );
						VectorScale( forward, -(ent->client->pers.cameradistance)+
							ent->cam_target->mins[0], forward );
						VectorAdd( ent->cam_target->s.origin, forward, ent->s.origin );
					}
				}
				else if( strcmp( ent->vehicle->classname, "helo" ) == 0 )
				{
					gi.cprintf( ent, PRINT_MEDIUM, "Straight-Ahead-View ON.\n" );
					ent->client->cameramode = 2;

					ent->cam_target = ent->vehicle;

				}
				else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
				{
					gi.cprintf( ent, PRINT_MEDIUM, "Turret-View ON.\n" );
					ent->client->cameramode = 2;

					ent->cam_target = ent->vehicle->movetarget;

				}
				break;
		case 2:
				gi.cprintf( ent, PRINT_MEDIUM, "Missile-View ON.\n" );
				ent->client->cameramode = 3;
				if( ent->goalentity )
				{
					ent->cam_target = ent->goalentity;

					AngleVectors( ent->cam_target->s.angles, forward, NULL, up );
					VectorScale( forward, -(ent->client->pers.cameradistance)+
							ent->cam_target->mins[0], forward );
					VectorAdd( ent->cam_target->s.origin, forward, ent->s.origin );
				}
				break;
		case 3:
				gi.cprintf( ent, PRINT_MEDIUM, "Hull-View ON.\n" );
				ent->client->cameramode = 1;

				ent->cam_target = ent->vehicle;

				break;
	}
}


