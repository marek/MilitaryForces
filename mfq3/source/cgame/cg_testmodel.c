/*
 * $Id: cg_testmodel.c,v 1.2 2002-01-28 22:34:30 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_view.c -- setup all the parameters (position, angle, etc)
// for a 3D rendering
#include "cg_local.h"

extern char *plane_tags[BP_PLANE_MAX_PARTS];
extern char *gv_tags[BP_GV_MAX_PARTS];

/*
=============================================================================

  MODEL TESTING

The viewthing and gun positioning tools from Q2 have been integrated and
enhanced into a single model testing facility.

Model viewing can begin with either "testmodel <modelname>" or "testgun <modelname>".

The names must be the full pathname after the basedir, like 
"models/weapons/v_launch/tris.md3" or "players/male/tris.md3"

Testmodel will create a fake entity 100 units in front of the current view
position, directly facing the viewer.  It will remain immobile, so you can
move around it to view it from different angles.

Testgun will cause the model to follow the player around and supress the real
view weapon model.  The default frame 0 of most guns is completely off screen,
so you will probably have to cycle a couple frames to see it.

"nextframe", "prevframe", "nextskin", and "prevskin" commands will change the
frame or skin of the testmodel.  These are bound to F5, F6, F7, and F8 in
q3default.cfg.

If a gun is being tested, the "gun_x", "gun_y", and "gun_z" variables will let
you adjust the positioning.

Note that none of the model testing features update while the game is paused, so
it may be convenient to test with deathmatch set to 1 so that bringing down the
console doesn't pause the game.

=============================================================================
*/

/*
=================
CG_TestModel_f

Creates an entity in front of the current position, which
can then be moved around
=================
*/
void CG_TestModel_f (void) {
	vec3_t		angles;

	memset( &cg.testModelEntity, 0, sizeof(cg.testModelEntity) );
	if ( trap_Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cg.testModelName, CG_Argv( 1 ), MAX_QPATH );
	cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );

	if ( trap_Argc() == 3 ) {
		cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}
	if (! cg.testModelEntity.hModel ) {
		CG_Printf( "Can't register model\n" );
		return;
	}

	VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testModelEntity.origin );

	angles[PITCH] = 0;
	angles[YAW] = 180 + cg.refdefViewAngles[1];
	angles[ROLL] = 0;

	AnglesToAxis( angles, cg.testModelEntity.axis );
	cg.testGun = qfalse;
}

/*
=================
CG_TestGun_f

Replaces the current view weapon with the given model
=================
*/
void CG_TestGun_f (void) {
	CG_TestModel_f();
	cg.testGun = qtrue;
	cg.testModelEntity.renderfx = RF_MINLIGHT | RF_DEPTHHACK | RF_FIRST_PERSON;
}


void CG_TestModelNextFrame_f (void) {
	cg.testModelEntity.frame++;
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelPrevFrame_f (void) {
	cg.testModelEntity.frame--;
	if ( cg.testModelEntity.frame < 0 ) {
		cg.testModelEntity.frame = 0;
	}
	CG_Printf( "frame %i\n", cg.testModelEntity.frame );
}

void CG_TestModelNextSkin_f (void) {
	cg.testModelEntity.skinNum++;
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_TestModelPrevSkin_f (void) {
	cg.testModelEntity.skinNum--;
	if ( cg.testModelEntity.skinNum < 0 ) {
		cg.testModelEntity.skinNum = 0;
	}
	CG_Printf( "skin %i\n", cg.testModelEntity.skinNum );
}

void CG_AddTestModel (void) {
	int		i;

	// re-register the model, because the level may have changed
	cg.testModelEntity.hModel = trap_R_RegisterModel( cg.testModelName );
	if (! cg.testModelEntity.hModel ) {
		CG_Printf ("Can't register model\n");
		return;
	}

	// if testing a gun, set the origin reletive to the view origin
	if ( cg.testGun ) {
		VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
		VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
		VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
		VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

		// allow the position to be adjusted
		for (i=0 ; i<3 ; i++) {
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z.value;
		}
	}

	trap_R_AddRefEntityToScene( &cg.testModelEntity );
}

static void registerTestVehicle()
{
	int			i;
	char		basename[128];
	char		name[128];

	switch( cg.testVehicleCat ) {
	case CAT_PLANE:
		{
			Com_sprintf( basename, sizeof(basename), "models/vehicles/planes/%s/%s", cg.testVehicleName,
				cg.testVehicleName );
			for( i = 0; i < BP_PLANE_MAX_PARTS; i++ ) {
				switch(i) {
				case BP_PLANE_BODY:
					Com_sprintf( name, sizeof(name), "%s.md3", basename );
					break;
				case BP_PLANE_CONTROLS:
					Com_sprintf( name, sizeof(name), "%s_controls.md3", basename );
					break;
				case BP_PLANE_COCKPIT:
					Com_sprintf( name, sizeof(name), "%s_cockpit.md3", basename );
					break;
				case BP_PLANE_GEAR:
					Com_sprintf( name, sizeof(name), "%s_gear.md3", basename );
					break;
				case BP_PLANE_BRAKES:
					Com_sprintf( name, sizeof(name), "%s_breaks.md3", basename );
					break;
				case BP_PLANE_BAY:
					Com_sprintf( name, sizeof(name), "%s_bay.md3", basename );
					break;
				case BP_PLANE_WINGLEFT:
					Com_sprintf( name, sizeof(name), "%s_wingL.md3", basename );
					break;
				case BP_PLANE_WINGRIGHT:
					Com_sprintf( name, sizeof(name), "%s_wingR.md3", basename );
					break;
				case BP_PLANE_SPECIAL:
					Com_sprintf( name, sizeof(name), "%s_special.md3", basename );
					break;
				case BP_PLANE_PROP:
					Com_sprintf( name, sizeof(name), "%s_prop.md3", basename );
					break;
				case BP_PLANE_PROP2:
					Com_sprintf( name, sizeof(name), "%s_prop.md3", basename );
					break;
				}
				cg.testVehicleParts[i].hModel = trap_R_RegisterModel( name );
			}
		}
		cg.testBurner.hModel = cgs.media.afterburner[cg.testBurnerNum];
		cg.testBurner2.hModel = cgs.media.afterburner[cg.testBurnerNum];
		cg.testVapor.hModel = cg.testVaporNum ? cgs.media.vaporBig : cgs.media.vapor;
		break;
	case CAT_GROUND:
		{
			Com_sprintf( basename, sizeof(basename), "models/vehicles/ground/%s/%s", cg.testVehicleName,
					cg.testVehicleName );
			for( i = 0; i < BP_GV_MAX_PARTS; i++ ) {
				switch(i) {
				case BP_GV_BODY:
					Com_sprintf( name, sizeof(name), "%s.md3", basename );
					break;
				case BP_GV_TURRET:
					Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
					break;
				case BP_GV_GUNBARREL:
					Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
					break;
				}
				cg.testVehicleParts[i].hModel = trap_R_RegisterModel( name );
			}
		}
		break;
	}
}

void CG_AddTestVehicle (void) {
	int		i;

	// re-register the model, because the level may have changed
	registerTestVehicle();

	if ( !cg.testVehicleParts[0].hModel ) {
		CG_Printf ("Can't register model\n");
		return;
	}

	// if testing a gun, set the origin reletive to the view origin
/*	if ( cg.testGun ) {
		VectorCopy( cg.refdef.vieworg, cg.testModelEntity.origin );
		VectorCopy( cg.refdef.viewaxis[0], cg.testModelEntity.axis[0] );
		VectorCopy( cg.refdef.viewaxis[1], cg.testModelEntity.axis[1] );
		VectorCopy( cg.refdef.viewaxis[2], cg.testModelEntity.axis[2] );

		// allow the position to be adjusted
		for (i=0 ; i<3 ; i++) {
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[0][i] * cg_gun_x.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[1][i] * cg_gun_y.value;
			cg.testModelEntity.origin[i] += cg.refdef.viewaxis[2][i] * cg_gun_z.value;
		}
	}*/


	trap_R_AddRefEntityToScene( &cg.testVehicleParts[0] );

	switch( cg.testVehicleCat ) {
		case CAT_PLANE:
			{
				for( i = 1; i < BP_PLANE_MAX_PARTS; ++i ) {
					if( ! cg.testVehicleParts[i].hModel ) continue;
					CG_PositionEntityOnTag( &cg.testVehicleParts[i], &cg.testVehicleParts[0], 
						cg.testVehicleParts[0].hModel, plane_tags[i] );
					if( i == BP_PLANE_CONTROLS ) {
						if( !cg.testVehicleParts[BP_PLANE_GEAR].frame &&
							 cg.testVehicleParts[BP_PLANE_CONTROLS].frame < 9 ) {
							cg.testVehicleParts[BP_PLANE_CONTROLS].frame += 9;
						} else if( cg.testVehicleParts[BP_PLANE_GEAR].frame &&
								   cg.testVehicleParts[BP_PLANE_CONTROLS].frame >= 9 ) {
							cg.testVehicleParts[BP_PLANE_CONTROLS].frame -= 9;
						}
					}
					trap_R_AddRefEntityToScene( &cg.testVehicleParts[i] );
					if( cg.testVehicleParts[BP_PLANE_BODY].frame ) {
						CG_PositionEntityOnTag( &cg.testBurner, &cg.testVehicleParts[0], 
												cg.testVehicleParts[0].hModel, "tag_ab1" );
						trap_R_AddRefEntityToScene( &cg.testBurner );
						if( cg.testNumBurners ) {
							CG_PositionEntityOnTag( &cg.testBurner2, &cg.testVehicleParts[0], 
													cg.testVehicleParts[0].hModel, "tag_ab2" );
							trap_R_AddRefEntityToScene( &cg.testBurner2 );
						}

					}
					if( cg.testVaporFrame ) {
						cg.testVapor.frame = cg.testVaporFrame-1;
						CG_PositionEntityOnTag( &cg.testVapor, &cg.testVehicleParts[0], 
												cg.testVehicleParts[0].hModel, "tag_vapor1" );
						trap_R_AddRefEntityToScene( &cg.testVapor );
					}
				}
			}
			break;
		case CAT_GROUND:
			{
				for( i = 1; i < BP_GV_MAX_PARTS; ++i ) {
					if( ! cg.testVehicleParts[i].hModel ) continue;
					CG_PositionEntityOnTag( &cg.testVehicleParts[1], &cg.testVehicleParts[0], 
						cg.testVehicleParts[0].hModel, gv_tags[1] );
					trap_R_AddRefEntityToScene( &cg.testVehicleParts[1] );
					CG_PositionEntityOnTag( &cg.testVehicleParts[2], &cg.testVehicleParts[1], 
						cg.testVehicleParts[1].hModel, gv_tags[2] );
					trap_R_AddRefEntityToScene( &cg.testVehicleParts[2] );
				}
			}
			break;
	}

}

void CG_TestVehicle (long cat) {
	vec3_t		angles;
	int			i;

	if( cat != CAT_PLANE && cat != CAT_GROUND ) {
		return;
	}
	cg.testVehicleCat = cat;

	for( i = 0; i < BP_MAX_PARTS; ++i ) {
		memset( &cg.testVehicleParts[i], 0, sizeof(cg.testVehicleParts[i]) );
	}

	if ( trap_Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cg.testVehicleName, CG_Argv( 1 ), MAX_QPATH );

	registerTestVehicle();

/*	if ( trap_Argc() == 3 ) {
		cg.testModelEntity.backlerp = atof( CG_Argv( 2 ) );
		cg.testModelEntity.frame = 1;
		cg.testModelEntity.oldframe = 0;
	}*/
	if (! cg.testVehicleParts[0].hModel ) {
		CG_Printf( "Can't register model\n" );
		return;
	}

	angles[PITCH] = 0;
	angles[YAW] = 180 + cg.refdefViewAngles[1];
	angles[ROLL] = 0;

	for( i = 0; i < BP_MAX_PARTS; ++i ) {
		VectorMA( cg.refdef.vieworg, 100, cg.refdef.viewaxis[0], cg.testVehicleParts[i].origin );
		AnglesToAxis( angles, cg.testVehicleParts[i].axis );
	}

//	cg.testGun = qfalse;
}



void CG_TestPlaneCmd_f (void) {
	char		cmd[128];
	
	if( cg.testVehicleCat != CAT_PLANE ) {
		return;
	}

	if ( trap_Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cmd, CG_Argv( 1 ), MAX_QPATH );
	
	if (Q_stricmp (cmd, "cockpit") == 0) {
		cg.testVehicleParts[BP_PLANE_COCKPIT].frame = cg.testVehicleParts[BP_PLANE_COCKPIT].frame ? 0 : 1;
	} else if (Q_stricmp (cmd, "speedbrakes") == 0) {
		cg.testVehicleParts[BP_PLANE_BRAKES].frame = cg.testVehicleParts[BP_PLANE_BRAKES].frame ? 0 : 1;	
	} else if (Q_stricmp (cmd, "gear") == 0) {
		cg.testVehicleParts[BP_PLANE_GEAR].frame = cg.testVehicleParts[BP_PLANE_GEAR].frame ? 0 : 2;	
		if( cg.testVehicleParts[BP_PLANE_GEAR].frame ) {
			cg.testVehicleParts[BP_PLANE_CONTROLS].frame -= 9;
		} else {
			cg.testVehicleParts[BP_PLANE_CONTROLS].frame += 9;
		}
	} else if (Q_stricmp (cmd, "controls") == 0) {
		int min = cg.testVehicleParts[BP_PLANE_GEAR].frame ? 0 : 9;
		int max = cg.testVehicleParts[BP_PLANE_GEAR].frame ? 9 : 18;
		cg.testVehicleParts[BP_PLANE_CONTROLS].frame++;	
		if( cg.testVehicleParts[BP_PLANE_CONTROLS].frame >= max ) {
			cg.testVehicleParts[BP_PLANE_CONTROLS].frame = min;
		} else if( cg.testVehicleParts[BP_PLANE_CONTROLS].frame < min ) {
			cg.testVehicleParts[BP_PLANE_CONTROLS].frame += 9;
		}
		CG_Printf( "Control frame = %d\n", cg.testVehicleParts[BP_PLANE_CONTROLS].frame );
	} else if (Q_stricmp (cmd, "throttle") == 0) {
		cg.testVehicleParts[BP_PLANE_BODY].frame++;
		if( cg.testVehicleParts[BP_PLANE_BODY].frame > 2 ) {
			cg.testVehicleParts[BP_PLANE_BODY].frame = 0;
		}
		if( cg.testVehicleParts[BP_PLANE_BODY].frame == 1 ) {
			cg.testBurner.frame = 1;
			cg.testBurner2.frame = 1;
		} else if( cg.testVehicleParts[BP_PLANE_BODY].frame == 2 ) {
			cg.testBurner.frame = 0;
			cg.testBurner2.frame = 0;
		}
	} else if (Q_stricmp (cmd, "burnermodel") == 0) {
		cg.testBurnerNum++;
		if( cg.testBurnerNum >= AB_MAX ) cg.testBurnerNum = 0;
		CG_Printf( "Using burner model %d\n", cg.testBurnerNum );
	} else if (Q_stricmp (cmd, "burnernum") == 0) {
		cg.testNumBurners = cg.testNumBurners ? 0 : 1;
		CG_Printf( "Using %d burners\n", cg.testNumBurners+1 );
	} else if (Q_stricmp (cmd, "vapor") == 0) {
		cg.testVaporFrame++;
		if( cg.testVaporFrame > 2 ) cg.testVaporFrame = 0;
	} else if (Q_stricmp (cmd, "vapormodel") == 0) {
		cg.testVaporNum = cg.testVaporNum ? 0 : 1;
		CG_Printf( "Using vapor model %d\n", cg.testVaporNum );
	}

}

void CG_TestGVCmd_f (void) {
	char		cmd[128];

	return; // if there is anything to do finish this func

	if( cg.testVehicleCat != CAT_GROUND ) {
		return;
	}

	if ( trap_Argc() < 2 ) {
		return;
	}

	Q_strncpyz (cmd, CG_Argv( 1 ), MAX_QPATH );
	
	if (Q_stricmp (cmd, "blah") == 0) {

	} else if (Q_stricmp (cmd, "blahblah") == 0) {

	}

}

