/*
 * $Id: ui_vehicleselection.c,v 1.1 2001-11-15 15:47:38 moorman Exp $
*/

/*
=======================================================================

Vehicle Selection Menu

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK					"menu/art/back"
#define ART_BACK_A					"menu/art/back_a"
#define ART_FIGHT					"menu/art/fight"
#define ART_FIGHT_A					"menu/art/fight_a"
#define ART_LOGO					"menu/art/ingame"

#define ID_BACK					10
#define ID_CATEGORY				11
#define ID_CLASS				12
#define ID_FIGHT				13
#define ID_NEXT					14
#define ID_ALL					15

#define MF_MAX_INFOLENGTH		16

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;

	menulist_s		vcategory;
	menulist_s		vclass;
	menubitmap_s	next;

	char			*namestring;
	int				namelen;

	menubitmap_s	vehicle;
	menubitmap_s	back;
	menubitmap_s	fight;

	qhandle_t		vmodel;

	unsigned long	gameset;
	unsigned long	team;
	unsigned long	category;
	unsigned long	cls;

	int				vindex;

	int				gametype;
} vehicleselection_t;

static vehicleselection_t	s_vehicleselection;

static int UI_currentSelection	= -1;
static int UI_categories = CAT_ANY;
static int	UI_team = 0;
static int UI_gametype = 0;

#pragma message("test multiple skins!")

static void setSelItemInformation(int byWhat)
{
	int index = s_vehicleselection.vindex;// save old index
	qboolean done = qfalse;

	// get selection
	switch( byWhat )
	{
	case ID_CATEGORY:
		{
			unsigned long what = ((0x01<<(s_vehicleselection.vcategory.curvalue))<<8);
			while( !done ) {
				if( what > CAT_MAX ) what = CAT_MIN;
				if( what == s_vehicleselection.category ) done = qtrue;
				s_vehicleselection.vindex = MF_getIndexOfVehicle( -1, 
														   s_vehicleselection.gameset|
														   s_vehicleselection.team|
														   what );
				if( s_vehicleselection.vindex != -1 ) { 
					done = qtrue;
					s_vehicleselection.category = what;
				}
				what <<= 1;
			}
			s_vehicleselection.cls = (availableVehicles[s_vehicleselection.vindex].id&CLASS_ANY);
		}
		break;

	case ID_CLASS:
		{
			unsigned long what = (0x01<<(s_vehicleselection.vclass.curvalue));
			while( !done ) {
				if( what > CLASS_MAX ) what = CLASS_MIN;
				if( what == s_vehicleselection.cls ) done = qtrue;
				s_vehicleselection.vindex = MF_getIndexOfVehicle( -1, 
														   s_vehicleselection.gameset|
														   s_vehicleselection.team|
														   s_vehicleselection.category|
														   what );
				if( s_vehicleselection.vindex != -1 ) { 
					done = qtrue;
					s_vehicleselection.cls = what;
				}
				what <<= 1;
			}
		}
		break;

	case ID_NEXT:
		s_vehicleselection.vindex = MF_getIndexOfVehicle( index, 
												   s_vehicleselection.gameset|
												   s_vehicleselection.team|
												   s_vehicleselection.category|
												   s_vehicleselection.cls );
		break;

	case ID_ALL:	// anything from the gameset (assuming there is at least 1 vehicle in that set!!)
		s_vehicleselection.vindex = MF_getIndexOfVehicle( index, 
												   s_vehicleselection.gameset|
												   s_vehicleselection.team|
												   UI_categories);// choose from available cats only
		s_vehicleselection.category = (availableVehicles[s_vehicleselection.vindex].id&CAT_ANY);
		s_vehicleselection.cls = (availableVehicles[s_vehicleselection.vindex].id&CLASS_ANY);
		break;
	}

	index = s_vehicleselection.vindex;

	if( index < 0 || index >= bg_numberOfVehicles ) {
		Com_Printf( "Vehicle index out of range (%d), corrected to 0\n", index );
		index = 0;
	}

	UI_currentSelection = index;

	// set information
//	switch( byWhat )
//	{
//	case ID_ALL:			// on startup
//	case ID_CATEGORY:
		s_vehicleselection.vcategory.itemnames = cat_items;
		s_vehicleselection.vcategory.curvalue = MF_getItemIndexFromHex((s_vehicleselection.category>>8));
		s_vehicleselection.vcategory.numitems = MF_getNumberOfItems(cat_items);
//	case ID_CLASS:
		s_vehicleselection.vclass.itemnames = class_items[s_vehicleselection.vcategory.curvalue];
		s_vehicleselection.vclass.curvalue = MF_getItemIndexFromHex(s_vehicleselection.cls);
		s_vehicleselection.vclass.numitems = MF_getNumberOfItems(class_items[s_vehicleselection.vcategory.curvalue]);
//	case ID_NEXT:
		{
		static char model[MF_MAX_MODELNAME];
		static char dir[32];
		char		*test;
		int cat = availableVehicles[index].id&CAT_ANY;
		if( cat & CAT_PLANE ) {
			strcpy( dir, "planes" );
		}
		else if( cat & CAT_GROUND ) {
			strcpy( dir, "ground" );
		}

//		s_vehicleselection.vname.string = availableVehicles[index].descriptiveName;
		test = s_vehicleselection.namestring = availableVehicles[index].descriptiveName;
		s_vehicleselection.namelen = 0;
		while( *test ) {
			s_vehicleselection.namelen++;
			test++;
		}

		Com_sprintf( model, sizeof(model), "models/vehicles/%s/%s/%s.md3", dir, availableVehicles[index].modelName,
				availableVehicles[index].modelName );
		s_vehicleselection.vmodel = trap_R_RegisterModel( model );
		s_vehicleselection.vehicle.generic.name = va("models/vehicles/%s/%s/%s_icon", dir,
			availableVehicles[index].modelName, availableVehicles[index].modelName);
		s_vehicleselection.vehicle.shader = 0;
		}
//		break;
//	}
}


/*
===============
VehicleSelection_Select
===============
*/
static void VehicleSelection_Select( void ) {
//	char	info[MAX_INFO_STRING];
//	uiClientState_t	cs;
//	int		currVeh;

	// get current vehicle
//	trap_GetClientState( &cs );
//	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
//	currVeh = atoi( Info_ValueForKey( info, "v" ) );
//	if( currVeh < 0 ) { 
//		trap_Cvar_Set( "cg_vehicle", va( "%i", UI_currentSelection ) );
//	}
	trap_Cvar_Set( "cg_nextVehicle", va( "%i", UI_currentSelection ) );
}

/*
===============
VehicleSelection_MenuEvent
===============
*/
static void VehicleSelection_MenuEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {

	case ID_CATEGORY:
	case ID_CLASS:
	case ID_NEXT:
		setSelItemInformation(((menucommon_s*)ptr)->id);
		break;

	case ID_FIGHT:
		VehicleSelection_Select();
		UI_ForceMenuOff();
		break;

	case ID_BACK:
		UI_PopMenu();
		break;

	}
}


/*
===============
VehicleSelection_DrawNameBar
===============
*/
static void VehicleSelection_DrawNameBar( void *self ) {
	menubitmap_s	*s;
	float			*color;
	int				style;
	qboolean		focus;
	int				x,y,w;

	s = (menubitmap_s *)self;

	// prepare
	x = s->generic.x;
	y =	s->generic.y;
	w = 58 + s_vehicleselection.namelen * SMALLCHAR_WIDTH;

	style = UI_SMALLFONT;

	focus = (s->generic.parent->cursor == s->generic.menuPosition);

	if( focus ) {
		color = color_beige;
	}
	else {
		color = color_grey;
	}

	// draw
//	UI_FillRect(160, 92, 320, 270, color_darkred );
//	UI_DrawString( x+1, y+1, string, style, color );

	if ( focus )
	{
		// draw cursor
		UI_FillRect( s->generic.left, s->generic.top, w+1, s->generic.bottom-s->generic.top+1, color_darkred ); 
		UI_DrawChar( 300, 179, 13, UI_CENTER|UI_BLINK|UI_SMALLFONT, color);
	}

	UI_DrawString( x, y, "Name:", style|UI_LEFT, color );
	UI_DrawString( 308, 179, s_vehicleselection.namestring, style|UI_LEFT, color );

}

/*
===============
VehicleSelectionMenu_Draw
===============
*/
static void VehicleSelectionMenu_Draw( void ) {
//	refdef_t		refdef;
//	refEntity_t		ent1,ent2;
//	vec3_t			origin;
//	vec3_t			angles;
//	float			x, y, w, h;
//	vec4_t			color = {0.5, 0, 0, 1};

	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

//	UI_FillRect(160, 92, 320, 270, color_lightgrey );
//	UI_DrawRect(154, 124, 468, 342, color_black );
//	UI_DrawRect(160, 260, 456, 200, color_black );

	if( s_vehicleselection.vehicle.generic.parent->cursor == s_vehicleselection.vehicle.generic.menuPosition ) {
		UI_DrawRect(239, 203, 162, 122, color_grey );
	} else {
		UI_DrawRect(239, 203, 162, 122, color_black );
	}

	// standard menu drawing
	Menu_Draw( &s_vehicleselection.menu );

	// setup the refdef
/*	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );

	x = 0;
	y = 100;
	w = 640;
	h = 220;
	UI_AdjustFrom640( &x, &y, &w, &h );
	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.fov_x = 60;
	refdef.fov_y = 19.6875;

	refdef.time = uis.realtime;

	origin[0] = 550;
	origin[1] = 0;
	origin[2] = -50;

	trap_R_ClearScene();

	// add the models
	memset( &ent1, 0, sizeof(ent1) );
	origin[1] = 60;//-230;
	VectorSet( angles, -90, 90, -90 );
	AnglesToAxis( angles, ent1.axis );
	ent1.hModel = s_vehicleselection.vmodel;
	VectorCopy( origin, ent1.origin );
	VectorCopy( origin, ent1.lightingOrigin );
	ent1.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent1.origin, ent1.oldorigin );
	trap_R_AddRefEntityToScene( &ent1 );
	
	memset( &ent2, 0, sizeof(ent2) );
	origin[1] = -60;//-130;
	VectorSet( angles, -90, 90, 90 );
	AnglesToAxis( angles, ent2.axis );
	ent2.hModel = s_vehicleselection.vmodel;
	VectorCopy( origin, ent2.origin );
	VectorCopy( origin, ent2.lightingOrigin );
	ent2.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent2.origin, ent2.oldorigin );
	trap_R_AddRefEntityToScene( &ent2 );

	trap_R_RenderScene( &refdef );*/
}

/*
===============
VehicleSelection_MenuInit
===============
*/
static void VehicleSelection_MenuInit( void ) {

	memset( &s_vehicleselection, 0 ,sizeof(vehicleselection_t) );

	VehicleSelection_Cache();

	s_vehicleselection.menu.draw = VehicleSelectionMenu_Draw;
	s_vehicleselection.menu.wrapAround = qtrue;
    s_vehicleselection.menu.fullscreen = qfalse;
	
	s_vehicleselection.gameset = UI_GetGameset();
	if( UI_gametype >= GT_TEAM ) {
		#pragma message("team needs to be set properly")
		s_vehicleselection.team = UI_team << 16;
	}
	else {
		s_vehicleselection.team = MF_TEAM_ANY;
	}

	s_vehicleselection.category = CAT_MIN;
	s_vehicleselection.cls = CLASS_MIN;
	s_vehicleselection.vindex = UI_currentSelection;

	s_vehicleselection.banner.generic.type			= MTYPE_BTEXT;
	s_vehicleselection.banner.generic.x				= 320;
	s_vehicleselection.banner.generic.y				= 100;
	s_vehicleselection.banner.string				= "VEHICLE";
	s_vehicleselection.banner.color					= color_black;
	s_vehicleselection.banner.style					= UI_CENTER;

	s_vehicleselection.vcategory.generic.type		= MTYPE_SPINCONTROL;
	s_vehicleselection.vcategory.generic.name		= "Category:";
	s_vehicleselection.vcategory.generic.flags		= QMF_LEFT_JUSTIFY|QMF_SMALLFONT|QMF_HIGHLIGHT_IF_FOCUS;
	s_vehicleselection.vcategory.generic.callback	= VehicleSelection_MenuEvent;
	s_vehicleselection.vcategory.generic.id			= ID_CATEGORY;
	s_vehicleselection.vcategory.generic.x			= 300;
	s_vehicleselection.vcategory.generic.y			= 140;

	s_vehicleselection.vclass.generic.type			= MTYPE_SPINCONTROL;
	s_vehicleselection.vclass.generic.name			= "Class:";
	s_vehicleselection.vclass.generic.flags			= QMF_LEFT_JUSTIFY|QMF_SMALLFONT|QMF_HIGHLIGHT_IF_FOCUS;
	s_vehicleselection.vclass.generic.callback		= VehicleSelection_MenuEvent;
	s_vehicleselection.vclass.generic.id			= ID_CLASS;
	s_vehicleselection.vclass.generic.x				= 300;
	s_vehicleselection.vclass.generic.y				= 160;

	s_vehicleselection.next.generic.type			= MTYPE_BITMAP;
	s_vehicleselection.next.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_vehicleselection.next.generic.id				= ID_NEXT;
	s_vehicleselection.next.generic.callback		= VehicleSelection_MenuEvent;
	s_vehicleselection.next.generic.ownerdraw		= VehicleSelection_DrawNameBar;
	s_vehicleselection.next.generic.x				= 250;
	s_vehicleselection.next.generic.y				= 179;
	s_vehicleselection.next.width					= 130;
	s_vehicleselection.next.height					= 18;

	s_vehicleselection.fight.generic.type			= MTYPE_BITMAP;
	s_vehicleselection.fight.generic.name			= ART_FIGHT;
	s_vehicleselection.fight.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_vehicleselection.fight.generic.id				= ID_FIGHT;
	s_vehicleselection.fight.generic.callback		= VehicleSelection_MenuEvent;
	s_vehicleselection.fight.generic.x				= 372;
	s_vehicleselection.fight.generic.y				= 332;
	s_vehicleselection.fight.width					= 96;
	s_vehicleselection.fight.height					= 32;
	s_vehicleselection.fight.focuspic				= ART_FIGHT_A;

	s_vehicleselection.back.generic.type			= MTYPE_BITMAP;
	s_vehicleselection.back.generic.name			= ART_BACK;
	s_vehicleselection.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_PULSEIFFOCUS;
	s_vehicleselection.back.generic.id				= ID_BACK;
	s_vehicleselection.back.generic.callback		= VehicleSelection_MenuEvent;
	s_vehicleselection.back.generic.x				= 170;
	s_vehicleselection.back.generic.y				= 332;
	s_vehicleselection.back.width					= 96;
	s_vehicleselection.back.height					= 32;
	s_vehicleselection.back.focuspic				= ART_BACK_A;

	s_vehicleselection.vehicle.generic.type			= MTYPE_BITMAP;
	s_vehicleselection.vehicle.generic.name			= ART_BACK;
	s_vehicleselection.vehicle.generic.flags		= QMF_LEFT_JUSTIFY;
	s_vehicleselection.vehicle.generic.callback		= VehicleSelection_MenuEvent;
	s_vehicleselection.vehicle.generic.id			= ID_NEXT;
	s_vehicleselection.vehicle.generic.x			= 240;
	s_vehicleselection.vehicle.generic.y			= 204;
	s_vehicleselection.vehicle.width				= 160;
	s_vehicleselection.vehicle.height				= 120;
	s_vehicleselection.vehicle.focuspic				= ART_BACK_A;

	setSelItemInformation(ID_ALL);

	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.banner );
	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.vcategory );
	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.vclass );
//	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.vname_label );
//	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.vname );
	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.next );
	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.fight );
	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.back );
	Menu_AddItem( &s_vehicleselection.menu, &s_vehicleselection.vehicle );
}

/*
=================
VehicleSelection_Cache
=================
*/
void VehicleSelection_Cache( void ) {
	int i;
	int cat;
	char dir[16];

	trap_R_RegisterShaderNoMip( ART_LOGO );
	trap_R_RegisterShaderNoMip( ART_BACK );
	trap_R_RegisterShaderNoMip( ART_BACK_A );
	trap_R_RegisterShaderNoMip( ART_FIGHT );
	trap_R_RegisterShaderNoMip( ART_FIGHT_A );

	for( i = 0; i < bg_numberOfVehicles; i++ ) {
		cat = availableVehicles[i].id&CAT_ANY;
		if( cat & CAT_PLANE ) {
			strcpy( dir, "planes" );
		}
		else if( cat & CAT_GROUND ) {
			strcpy( dir, "ground" );
		}
		trap_R_RegisterShaderNoMip( va("models/vehicles/%s/%s/%s_icon", dir, availableVehicles[i].modelName,
				availableVehicles[i].modelName ) );
	}
}

/*
===============
UI_VehicleSelectionMenu
===============
*/
void UI_VehicleSelectionMenu( void ) {
	char	info[MAX_INFO_STRING];
	uiClientState_t	cs;

	// make sure it's a team game
	trap_GetConfigString( CS_SERVERINFO, info, sizeof(info) );
	UI_gametype = atoi( Info_ValueForKey( info, "g_gametype" ) );

	// get available categories
	UI_categories = atoi( Info_ValueForKey( info, "mf_lvcat" ) );

	// get current vehicle
	trap_GetClientState( &cs );
	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );
	UI_currentSelection = atoi( Info_ValueForKey( info, "x" ) );
	if( UI_currentSelection >= 0 ) { 
//		trap_Cvar_Set( "cg_nextVehicle", va( "%i", -1 ) );
		UI_currentSelection--;
	}
	else {
		UI_currentSelection = -1; 
	}
	
	// team
	UI_team = atoi( Info_ValueForKey( info, "t" ) );
	if( UI_team == TEAM_SPECTATOR ) return;

	VehicleSelection_MenuInit();
	UI_PushMenu( &s_vehicleselection.menu );
}

