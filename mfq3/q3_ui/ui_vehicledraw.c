/*
 * $Id: ui_vehicledraw.c,v 1.1 2001-11-15 15:47:38 moorman Exp $
*/

/*
=======================================================================

VEHICLE MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK0					"menu/art/back"
#define ART_BACK0_A					"menu/art/back_a"
#define ART_LOGO					"menu/art/background_sub"
	
#define ID_BACK				10


typedef struct {
	menuframework_s	menu;

	menutext_s		banner;

	menubitmap_s	back;

	qhandle_t		body;
	qhandle_t		turret;
	qhandle_t		gun;
	qhandle_t		cockpit;
	qhandle_t		controls;
	qhandle_t		gear;
	qhandle_t		brakes;
	qhandle_t		prop;
} vehshow_t;

static vehshow_t	s_vehshow;

static int show_vehicle;

static int game_is_running;

/*
===============
VehShow_MenuEvent
===============
*/
static void VehShow_MenuEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {

	case ID_BACK:
		UI_PopMenu();
		break;

	}
}

/*
======================
UI_PositionEntityOnTag
======================
*/
void UI_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							clipHandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	
	// lerp the tag
	trap_CM_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// cast away const because of compiler problems
	MatrixMultiply( lerped.axis, ((refEntity_t*)parent)->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}


/*
======================
UI_PositionRotatedEntityOnTag
======================
*/
void UI_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, 
							clipHandle_t parentModel, char *tagName ) {
	int				i;
	orientation_t	lerped;
	vec3_t			tempAxis[3];

	// lerp the tag
	trap_CM_LerpTag( &lerped, parentModel, parent->oldframe, parent->frame,
		1.0 - parent->backlerp, tagName );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// cast away const because of compiler problems
	MatrixMultiply( entity->axis, ((refEntity_t *)parent)->axis, tempAxis );
	MatrixMultiply( lerped.axis, tempAxis, entity->axis );
}


/*
===============
VehShow_DrawPlane
===============
*/
static void VehShow_DrawPlane( void ) {

	refdef_t		refdef;
	refEntity_t		body;
	refEntity_t		cockpit;
	refEntity_t		controls;
	refEntity_t		breaks;
	refEntity_t		gear;
	refEntity_t		prop;
	vec3_t			origin;
	vec3_t			angles;
	float			x, y, w, h;
	vec4_t			color = {0.5, 0, 0, 1};
	static float	angle = 0;

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );

	x = 0;
	y = 0;
	w = 640;
	h = 480;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 64;// 60
	refdef.fov_y = 48;//19.6875;

	refdef.time = uis.realtime;

	origin[0] = 110;
	origin[1] = 0;
	origin[2] = -5;

	trap_R_ClearScene();

	// add the models
	angle = 180.0 * sin( (float)uis.realtime / 5000 );
	memset( &body, 0, sizeof(body) );
	VectorSet( angles, 0, 180 + angle, 0 );
	AnglesToAxis( angles, body.axis );
	body.hModel = s_vehshow.body;
	VectorCopy( origin, body.origin );
	VectorCopy( origin, body.lightingOrigin );
	body.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( body.origin, body.oldorigin );
	trap_R_AddRefEntityToScene( &body );
	
	// cockpit
	memset( &cockpit, 0, sizeof(cockpit) );
	cockpit.hModel = s_vehshow.cockpit;
	VectorCopy( origin, cockpit.lightingOrigin );
	AxisCopy( axisDefault, cockpit.axis );
	cockpit.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	UI_PositionRotatedEntityOnTag( &cockpit, &body, s_vehshow.body, "tag_cockpit");
	trap_R_AddRefEntityToScene( &cockpit );	

	// controls
	memset( &controls, 0, sizeof(controls) );
	controls.hModel = s_vehshow.controls;
	VectorCopy( origin, controls.lightingOrigin );
	AxisCopy( axisDefault, controls.axis );
	controls.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	UI_PositionRotatedEntityOnTag( &controls, &body, s_vehshow.body, "tag_controls");
	trap_R_AddRefEntityToScene( &controls );

	// breaks
	if( availableVehicles[show_vehicle].caps & HC_SPEEDBRAKE ) {
		memset( &breaks, 0, sizeof(breaks) );
		breaks.hModel = s_vehshow.brakes;
		VectorCopy( origin, breaks.lightingOrigin );
		AxisCopy( axisDefault, breaks.axis );
		breaks.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		UI_PositionRotatedEntityOnTag( &breaks, &body, s_vehshow.body, "tag_breaks");
		trap_R_AddRefEntityToScene( &breaks );
	}

	// gear
	if( availableVehicles[show_vehicle].caps & HC_GEAR ) {
		memset( &gear, 0, sizeof(gear) );
		gear.hModel = s_vehshow.gear;
		gear.frame = 2;
		VectorCopy( origin, gear.lightingOrigin );
		AxisCopy( axisDefault, gear.axis );
		gear.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		UI_PositionRotatedEntityOnTag( &gear, &body, s_vehshow.body, "tag_gear");
		trap_R_AddRefEntityToScene( &gear );
	}

	// prop
	if( availableVehicles[show_vehicle].id&MF_GAMESET_ANY&MF_GAMESET_WW2 ) {
		memset( &prop, 0, sizeof(prop) );
		prop.hModel = s_vehshow.prop;
		AxisCopy( axisDefault, prop.axis );
		RotateAroundDirection( prop.axis, uis.realtime );
		VectorCopy( origin, prop.lightingOrigin );
		prop.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
		UI_PositionRotatedEntityOnTag( &prop, &body, s_vehshow.body, "tag_prop1");
		trap_R_AddRefEntityToScene( &prop );
	}

	trap_R_RenderScene( &refdef );
}

/*
===============
VehShow_DrawGV
===============
*/
static void VehShow_DrawGV( void ) {

	refdef_t		refdef;
	refEntity_t		body;
	refEntity_t		turret;
	refEntity_t		gun;
	vec3_t			origin;
	vec3_t			angles;
	float			x, y, w, h;
	vec4_t			color = {0.5, 0, 0, 1};
	static float	angle = 0;

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );

	x = 0;
	y = 0;
	w = 640;
	h = 480;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 64;// 60
	refdef.fov_y = 48;//19.6875;

	refdef.time = uis.realtime;

	origin[0] = 70;
	origin[1] = 0;
	origin[2] = -10;

	trap_R_ClearScene();

	// add the models
	angle = 180.0 * sin( (float)uis.realtime / 5000 );
	memset( &body, 0, sizeof(body) );
	VectorSet( angles, 0, 180 + angle, 0 );
	AnglesToAxis( angles, body.axis );
	body.hModel = s_vehshow.body;
	VectorCopy( origin, body.origin );
	VectorCopy( origin, body.lightingOrigin );
	body.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( body.origin, body.oldorigin );
	trap_R_AddRefEntityToScene( &body );
	
	// turret
	memset( &turret, 0, sizeof(turret) );
	turret.hModel = s_vehshow.turret;
	VectorCopy( origin, turret.lightingOrigin );
	AxisCopy( axisDefault, turret.axis );
	turret.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	UI_PositionRotatedEntityOnTag( &turret, &body, s_vehshow.body, "tag_turret");
	trap_R_AddRefEntityToScene( &turret );	

	// gun
	memset( &gun, 0, sizeof(gun) );
	gun.hModel = s_vehshow.gun;
	VectorCopy( origin, gun.lightingOrigin );
	AxisCopy( axisDefault, gun.axis );
	gun.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	UI_PositionRotatedEntityOnTag( &gun, &turret, s_vehshow.turret, "tag_weap");
	trap_R_AddRefEntityToScene( &gun );

	trap_R_RenderScene( &refdef );
}

/*
===============
VehShowMenu_Draw
===============
*/
static void VehShowMenu_Draw( void ) {
	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	// standard menu drawing
	Menu_Draw( &s_vehshow.menu );

	if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_PLANE ) {
		VehShow_DrawPlane();
	} else if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_GROUND ) {
		VehShow_DrawGV();
	}
}

/*
===============
VehShow_MenuInit
===============
*/
static void VehShow_MenuInit( void ) {

	memset( &s_vehshow, 0 ,sizeof(vehshow_t) );

	VehShow_Cache();

	s_vehshow.menu.wrapAround = qtrue;
	s_vehshow.menu.draw = VehShowMenu_Draw;
	if ( game_is_running ) {
	    s_vehshow.menu.fullscreen = qfalse;
	} else {
	    s_vehshow.menu.fullscreen = qtrue;
	}


	s_vehshow.banner.generic.type		= MTYPE_PTEXT;
	s_vehshow.banner.generic.x			= 320;
	s_vehshow.banner.generic.y			= 10;
	s_vehshow.banner.string				= availableVehicles[show_vehicle].descriptiveName;
	s_vehshow.banner.color				= color_black;
	s_vehshow.banner.style				= UI_CENTER;

	s_vehshow.back.generic.type			= MTYPE_BITMAP;
	s_vehshow.back.generic.name			= ART_BACK0;
	s_vehshow.back.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_vehshow.back.generic.id			= ID_BACK;
	s_vehshow.back.generic.callback		= VehShow_MenuEvent;
	s_vehshow.back.generic.x			= 20;
	s_vehshow.back.generic.y			= 425;
	s_vehshow.back.width				= 96;
	s_vehshow.back.height				= 32;
	s_vehshow.back.focuspic				= ART_BACK0_A;

	Menu_AddItem( &s_vehshow.menu, &s_vehshow.banner );
	Menu_AddItem( &s_vehshow.menu, &s_vehshow.back );
}

static void VehShow_Reg_Plane( void ) {
	s_vehshow.body = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s.md3", 
		availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );
	s_vehshow.cockpit = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_cockpit.md3", 
		availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );
	s_vehshow.controls = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_controls.md3", 
		availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );
	if( availableVehicles[show_vehicle].caps & HC_GEAR ) {
		s_vehshow.gear = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_gear.md3", 
			availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );
	}
	if( availableVehicles[show_vehicle].caps & HC_SPEEDBRAKE ) {
		s_vehshow.brakes = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_breaks.md3", 
			availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );
	}
	if( availableVehicles[show_vehicle].id&MF_GAMESET_ANY&MF_GAMESET_WW2 ) {
		s_vehshow.prop = trap_R_RegisterModel( va("models/vehicles/planes/%s/%s_prop.md3", 
			availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );
	}
	
}

static void VehShow_Reg_GV( void ) {
	s_vehshow.body = trap_R_RegisterModel( va("models/vehicles/ground/%s/%s.md3", 
		availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );
	s_vehshow.turret = trap_R_RegisterModel( va("models/vehicles/ground/%s/%s_tur.md3", 
		availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );
	s_vehshow.gun = trap_R_RegisterModel( va("models/vehicles/ground/%s/%s_gun.md3", 
		availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName) );

}


/*
=================
VehShow_Cache
=================
*/
void VehShow_Cache( void ) {
	trap_R_RegisterShaderNoMip( ART_LOGO );
	trap_R_RegisterShaderNoMip( ART_BACK0 );
	trap_R_RegisterShaderNoMip( ART_BACK0_A );

	if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_PLANE ) {
		VehShow_Reg_Plane();
	} else if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_GROUND ) {
		VehShow_Reg_GV();
	}
}

/*
===============
UI_DemosMenu
===============
*/
void UI_VehShowMenu( unsigned int idx ) {
	uiClientState_t	cstate;

	trap_GetClientState( &cstate );

	if ( cstate.connState >= CA_CONNECTED ) {
		// float on top of running game
		game_is_running = qtrue;
	}
	else {
		// game not running
		game_is_running = qfalse;
	}

	show_vehicle = idx;
	VehShow_MenuInit();
	UI_PushMenu( &s_vehshow.menu );
}
