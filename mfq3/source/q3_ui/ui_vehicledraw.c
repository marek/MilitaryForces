/*
 * $Id: ui_vehicledraw.c,v 1.3 2001-12-22 02:01:55 thebjoern Exp $
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
	
/*
=======================================================================
DEFINES
=======================================================================

*/

#define ID_BACK		10
#define	VIDI_TOP	120
#define	VIDI_ADJUST	4

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
======================
VehShow_AddLighting
======================
*/
void VehShow_AddLighting( vec3_t origin )
{
	vec3_t light_source;

	// add lighting
	VectorCopy( origin, light_source );
	light_source[ 1 ] += 200.0f;
	trap_R_AddLightToScene( light_source, 500, 1.0f, 1.0f, 1.0f );
}


/*
======================
VehShow_DrawBackground
======================
*/
void VehShow_DrawBackground( void )
{
	vec4_t tcolor;
	int y = 0, ty = VIDI_TOP-12;

	// for what vehicle type?
	if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_PLANE )
	{
		VectorSet( tcolor, 0.0f, 0.3f, 0.7f );
	}
	else if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_GROUND )
	{
		VectorSet( tcolor, 0.7f, 0.0f, 0.0f );
	}

	// fade in
	for( y = 0; y<12; y++ )
	{
		// set alpha
		tcolor[3] = y / 20.0f;

		// draw line
		UI_FillRect( 0, ty, 640, 1, tcolor );
		ty++;
	}

	// set alpha
	tcolor[3] = 0.6f;

	// draw console back
	UI_FillRect( 0, ty, 640, (4 * 13), tcolor );
	ty += (4 * 13);

	// set alpha
	tcolor[3] = 0.4f;

	// draw model back
	UI_FillRect( 0, ty, 640, 160, tcolor );
	ty += 160;

	// fade out
	for( y = 8; y>0; y-- )
	{
		// set alpha
		tcolor[3] = y / 20.0f;

		// draw line
		UI_FillRect( 0, ty, 640, 1, tcolor );
		ty++;
	}
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

	VehShow_DrawBackground();

	// add the models
//	angle = 180.0 * ( (float)uis.realtime / 5000 );
	angle += 0.4f;
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

	VehShow_AddLighting( body.origin );
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
	origin[2] = -5;

	trap_R_ClearScene();

	VehShow_DrawBackground();

	// add the models
//	angle = 180.0 * sin( (float)uis.realtime / 5000 );
	angle += 0.4f;
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

	VehShow_AddLighting( body.origin );
	trap_R_RenderScene( &refdef );
}

/*
===============
Vidiprinter Variables
===============
*/

char vidiWorkingText[ 256 ];	// line buffer
char vidiTextBuffer[ 1024 ];	// complete text buffer (from file)

int delay = 0;				// pre-delay
qboolean bChewRow =	qfalse;	// move up a row
char * pPosition = NULL;	// current character position
char * pStart = NULL;		// starting position

/*
===============
VehShow_RemoveVidiCharacter
===============
*/
static void VehShow_RemoveVidiCharacter( void ) {
	int moveAmount = 1024 - (pPosition - vidiTextBuffer);
	memcpy( pPosition, pPosition+1, moveAmount );
}

/*
===============
VehShowMenu_VidiPrinter
===============
*/

static void VehShowMenu_VidiPrinter( void ) {
	
	vec4_t vidiColor = {0.8f, 0.8f, 0.0f, 0.75f};	// text+cursor colour
	
	static int timer = 0;
	static char cTerminator = { 0x0D };	// line terminator (1st character of 2)
	
	int tx = 0, ty = VIDI_TOP - VIDI_ADJUST;
	int cx = 0, cy = 0;
	char * sPos = NULL;
	char * pScan = NULL;
	int length = 0;
	int rowCount = 0;
	qboolean completeLoop = qfalse;

	// reset?
	if( pPosition == NULL )
	{
		// assign text
		pPosition = vidiTextBuffer;
		pStart = vidiTextBuffer;
	}

	// move to start
	pScan = pStart;

	// chew a row?
	if( bChewRow )
	{
		// chew
		pStart = strstr( pScan, &cTerminator );
		pStart += 2;
		pScan = pStart;

		// reset
		bChewRow = qfalse;
	}

	// do the 'whole' rows first
	completeLoop = qtrue;
	do
	{
		// find any terminator
		sPos = strstr( pScan, &cTerminator );
		
		// render a complete line?
		if( sPos && sPos < pPosition )
		{
			length = (sPos - pScan);

			// copy text from current position up to the terminator
			strncpy( vidiWorkingText, pScan, length );
			vidiWorkingText[ length ] = 0;

			// gfx it
			UI_DrawString( tx, ty, vidiWorkingText, UI_SMALLFONT|UI_LEFT, vidiColor );
			
			// move gfx ptr down a line
			ty += 13;

			// move along two characters (as terminator sequence is 0x0D, 0x0A)
			pScan = sPos;
			pScan += 2;

			// one more row
			rowCount++;
			if( rowCount >= 4 )
			{
				// move line up
				bChewRow = qtrue;
			}
		}
		else
		{
			// break
			completeLoop = qfalse;
		}

	} while( completeLoop );

	// fix char pointers
	if( pScan > pPosition )
	{
		// don't let pPosition get behind
		pPosition = pScan;
	}

	// do the remaining row
	length = (pPosition - pScan);
	strncpy( vidiWorkingText, pScan, length );
	vidiWorkingText[ length ] = 0;
	UI_DrawString( tx, ty, vidiWorkingText, UI_SMALLFONT|UI_LEFT, vidiColor );
	
	// do the cursor
	cx = strlen( vidiWorkingText ) * 8;
	UI_DrawChar( tx+cx, ty+cy, 11, UI_BLINK|UI_SMALLFONT, vidiColor);

	// delay startup
	if( delay )
	{
		delay--;
		return;
	}

	// cursor handler
	timer++;
	if( timer > 2 )
	{
		// depends on character
		switch( *pPosition )
		{
			// finished?
			case '*':
				// pause at end
				break;

			// paragraph pause?
			case '#':
				// breather...
				delay = 150;

				// remove this character
				VehShow_RemoveVidiCharacter();
				break;

			// normal printable character
			default:
			{
				// do next character
				pPosition++;

				// vidiprinter sound
				//trap_S_StartLocalSound( menu_buzz_sound, CHAN_LOCAL_SOUND );
			} 
		}

		// reset character timer
		timer = 0;
	}
}

/*
===============
VehShow_ResetVidiPrint
===============
*/
static void VehShow_ResetVidiPrint( void ) {
	fileHandle_t fileHandle;
	int fileLength;
	char filename[ 64 ];

	// clear the working scratch buffer
	memset( &vidiWorkingText, 0, sizeof( vidiWorkingText ) );
	pPosition = NULL;
	
	// setup the pre-vidiprinting delay
	delay = 25;

	// make filename
	if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_PLANE ) {
		strcpy( filename, "models/vehicles/planes" );
	} else if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_GROUND ) {
		strcpy( filename, "models/vehicles/ground" );
	}
	Com_sprintf( filename, sizeof(filename), "%s/%s/%s.info", filename, availableVehicles[show_vehicle].modelName, availableVehicles[show_vehicle].modelName );

	// open vidi description file
	fileLength = trap_FS_FOpenFile( filename, &fileHandle, FS_READ );
	if( fileHandle )
	{
		// prevent overrun
		if( fileLength > 1024 )
		{
			fileLength = 1024;
		}

		// if open, read into buffer
		trap_FS_Read( vidiTextBuffer, fileLength, fileHandle );

		// close file, we don't need it again
		trap_FS_FCloseFile( fileHandle );
	}
	else
	{
		// no description file available
		strcpy( vidiTextBuffer, "No further information available*" );
	}
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

	// draw plane/ground vehicle
	if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_PLANE ) {
		VehShow_DrawPlane();
	} else if( availableVehicles[show_vehicle].id&CAT_ANY & CAT_GROUND ) {
		VehShow_DrawGV();
	}

	// the vidi-printer
	VehShowMenu_VidiPrinter();
}

/*
===============
VehShow_MenuInit
===============
*/
static void VehShow_MenuInit( void ) {

	memset( &s_vehshow, 0 ,sizeof(vehshow_t) );

	VehShow_ResetVidiPrint();
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
