/*
 * $Id: ui_encyclopedia.c,v 1.2 2002-01-07 00:06:02 thebjoern Exp $
*/

/*
=======================================================================

ENCYCLOPEDIA MENU

=======================================================================
*/


#include "ui_local.h"


#define ART_BACK					"menu/art/back"
#define ART_BACK_A					"menu/art/back_a"
#define ART_NEXT					"menu/art/next"
#define ART_NEXT_A					"menu/art/next_a"
#define ART_LOGO					"menu/art/background_sub"

#define ID_BACK					10
#define ID_GAMESET				11
#define ID_CATEGORY				12
#define ID_CLASS				13
#define ID_TEAM					14
#define ID_NEXT					15
#define ID_ALL					16
#define ID_SHOWVEHICLE			17

#define MF_MAX_INFOLENGTH		32

typedef struct {
	menuframework_s	menu;

	menutext_s		banner;

	menulist_s		vgameset;
	menulist_s		vteam;
	menulist_s		vcategory;
	menulist_s		vclass;
	menubitmap_s	next;

	char			*namestring;
	int				namelen;

	menutext_s		health;
	menutext_s		maxspeed;
	menutext_s		stallspeed;
	menutext_s		turnspeed;
	menutext_s		fuel;
	menutext_s		radar;

	menutext_s		weapon[8];

	menubitmap_s	back;
	menubitmap_s	vehicle;

	qhandle_t		vmodel;

	unsigned long	gameset;
	unsigned long	team;
	unsigned long	category;
	unsigned long	cls;

	int				vindex;
} encyclopedia_t;

int game_running;

static encyclopedia_t	s_encyclopedia;

#pragma message("test multiple skins!")

static void setItemInformation(int byWhat)
{
	int index = s_encyclopedia.vindex;// save old index
	int gamesetIndex;
	qboolean done = qfalse;
	int i, y;

	// get selection
	switch( byWhat )
	{
	case ID_GAMESET:
		{
			unsigned long what = ((0x01<<(s_encyclopedia.vgameset.curvalue))<<24);
			while( !done ) {
				if( what > MF_GAMESET_WW1 ) what = MF_GAMESET_MODERN;
				if( what == s_encyclopedia.gameset ) done = qtrue;
				s_encyclopedia.vindex = MF_getIndexOfVehicle( -1, 
														      what );
				if( s_encyclopedia.vindex != -1 ) { 
					done = qtrue;
					s_encyclopedia.gameset = what;
				}
				what <<= 1;
			}
			s_encyclopedia.team = (availableVehicles[s_encyclopedia.vindex].id&MF_TEAM_ANY);
			s_encyclopedia.category = (availableVehicles[s_encyclopedia.vindex].id&CAT_ANY);
			s_encyclopedia.cls = (availableVehicles[s_encyclopedia.vindex].id&CLASS_ANY);
		}
		break;

	case ID_TEAM:
		{
			unsigned long what = ((0x01<<(s_encyclopedia.vteam.curvalue))<<16);
			while( !done ) {
				if( what > MF_TEAM_MAX ) what = MF_TEAM_MIN;
				if( what == s_encyclopedia.team ) done = qtrue;
				s_encyclopedia.vindex = MF_getIndexOfVehicle( -1, 
															   s_encyclopedia.gameset|
														   	   what );
				if( s_encyclopedia.vindex != -1 ) { 
					done = qtrue;
					s_encyclopedia.team = what;
				}
				what <<= 1;
			}
			s_encyclopedia.category = (availableVehicles[s_encyclopedia.vindex].id&CAT_ANY);
			s_encyclopedia.cls = (availableVehicles[s_encyclopedia.vindex].id&CLASS_ANY);
		}
		break;

	case ID_CATEGORY:
		{
			unsigned long what = ((0x01<<(s_encyclopedia.vcategory.curvalue))<<8);
			while( !done ) {
				if( what > CAT_MAX ) what = CAT_MIN;
				if( what == s_encyclopedia.category ) done = qtrue;
				s_encyclopedia.vindex = MF_getIndexOfVehicle( -1, 
														   s_encyclopedia.gameset|
														   s_encyclopedia.team|
														   what );
				if( s_encyclopedia.vindex != -1 ) { 
					done = qtrue;
					s_encyclopedia.category = what;
				}
				what <<= 1;
			}
			s_encyclopedia.cls = (availableVehicles[s_encyclopedia.vindex].id&CLASS_ANY);
		}
		break;

	case ID_CLASS:
		{
			unsigned long what = (0x01<<(s_encyclopedia.vclass.curvalue));
			while( !done ) {
				if( what > CLASS_MAX ) what = CLASS_MIN;
				if( what == s_encyclopedia.cls ) done = qtrue;
				s_encyclopedia.vindex = MF_getIndexOfVehicle( -1, 
														   s_encyclopedia.gameset|
														   s_encyclopedia.team|
														   s_encyclopedia.category|
														   what );
				if( s_encyclopedia.vindex != -1 ) { 
					done = qtrue;
					s_encyclopedia.cls = what;
				}
				what <<= 1;
			}
		}
		break;

	case ID_NEXT:
		s_encyclopedia.vindex = MF_getIndexOfVehicle( index, 
												   s_encyclopedia.gameset|
												   s_encyclopedia.team|
												   s_encyclopedia.category|
												   s_encyclopedia.cls );
		break;

	case ID_ALL:	// anything from the gameset (assuming there is at least 1 vehicle in that set!!)
		s_encyclopedia.vindex = MF_getIndexOfVehicle( index, 
												   s_encyclopedia.gameset);
		s_encyclopedia.team = (availableVehicles[s_encyclopedia.vindex].id&MF_TEAM_ANY);
		s_encyclopedia.category = (availableVehicles[s_encyclopedia.vindex].id&CAT_ANY);
		s_encyclopedia.cls = (availableVehicles[s_encyclopedia.vindex].id&CLASS_ANY);
		break;
	}

	index = s_encyclopedia.vindex;
	gamesetIndex = MF_getItemIndexFromHex((s_encyclopedia.gameset>>24));

	if( index < 0 || index >= bg_numberOfVehicles ) {
		Com_Printf( "Vehicle index out of range (%d), corrected to 0\n", index );
		index = 0;
	}


	// set information
//	switch( byWhat )
//	{
//	case ID_ALL:			// on startup
//	case ID_TEAM:		// on change of teiam
//		Com_Printf( "The found team is %x\n", currentTeam );
		s_encyclopedia.vgameset.itemnames = gameset_items;
		s_encyclopedia.vgameset.curvalue = MF_getItemIndexFromHex((s_encyclopedia.gameset>>24));
		s_encyclopedia.vgameset.numitems = MF_getNumberOfItems(gameset_items);

		s_encyclopedia.vteam.itemnames = team_items[gamesetIndex];
		s_encyclopedia.vteam.curvalue = MF_getItemIndexFromHex((s_encyclopedia.team>>16));
		s_encyclopedia.vteam.numitems = MF_getNumberOfItems(team_items[gamesetIndex]);

//	case ID_CATEGORY:
		s_encyclopedia.vcategory.itemnames = cat_items;
		s_encyclopedia.vcategory.curvalue = MF_getItemIndexFromHex((s_encyclopedia.category>>8));
		s_encyclopedia.vcategory.numitems = MF_getNumberOfItems(cat_items);
//	case ID_CLASS:
		s_encyclopedia.vclass.itemnames = class_items[s_encyclopedia.vcategory.curvalue];
		s_encyclopedia.vclass.curvalue = MF_getItemIndexFromHex(s_encyclopedia.cls);
		s_encyclopedia.vclass.numitems = MF_getNumberOfItems(class_items[s_encyclopedia.vcategory.curvalue]);
//	case ID_NEXT:
		{
		static char model[MF_MAX_MODELNAME];
		static char text1[MF_MAX_INFOLENGTH];
		static char text2[MF_MAX_INFOLENGTH];
		static char text3[MF_MAX_INFOLENGTH];
		static char text4[MF_MAX_INFOLENGTH];
		static char text5[MF_MAX_INFOLENGTH];
		static char text6[MF_MAX_INFOLENGTH];
		static char textw[8][MF_MAX_INFOLENGTH];
		static char dir[32];
		char		*test;
		int cat = availableVehicles[index].id&CAT_ANY;
		if( cat & CAT_PLANE ) {
			strcpy( dir, "planes" );
		}
		else if( cat & CAT_GROUND ) {
			strcpy( dir, "ground" );
		}


		test = s_encyclopedia.namestring = availableVehicles[index].descriptiveName;
		s_encyclopedia.namelen = 0;
		while( *test ) {
			s_encyclopedia.namelen++;
			test++;
		}
		Com_sprintf( model, sizeof(model), "models/vehicles/%s/%s/%s.md3", dir, availableVehicles[index].modelName,
				availableVehicles[index].modelName );
		s_encyclopedia.vmodel = trap_R_RegisterModel( model );

		// general
		Com_sprintf( text1, MF_MAX_INFOLENGTH, "%d", availableVehicles[index].maxhealth );
		Com_sprintf( text2, MF_MAX_INFOLENGTH, "%d (%d%%)", availableVehicles[index].maxspeed,
													availableVehicles[index].maxthrottle*10 );
		Com_sprintf( text3, MF_MAX_INFOLENGTH, "%d", availableVehicles[index].stallspeed );
		Com_sprintf( text4, MF_MAX_INFOLENGTH, "%d%s", availableVehicles[index].maxfuel, 
			(availableVehicles[index].id&CAT_ANY&(CAT_PLANE|CAT_HELO) ? "00" : "") );
		Com_sprintf( text5, MF_MAX_INFOLENGTH, "%.0f/%.0f/%.0f", availableVehicles[index].turnspeed[0],
			availableVehicles[index].turnspeed[1], availableVehicles[index].turnspeed[2] );
		Com_sprintf( text6, MF_MAX_INFOLENGTH, "%s", availableVehicles[index].radarRange ?
			va("%d", availableVehicles[index].radarRange) : "No Radar" );
		if( availableVehicles[index].radarRange || availableVehicles[index].radarRange2 ) {
			if( availableVehicles[index].radarRange && availableVehicles[index].radarRange2 ) {
				Com_sprintf( text6, MF_MAX_INFOLENGTH, "%d (A), %d (G)", availableVehicles[index].radarRange,
					availableVehicles[index].radarRange2 );
			} else {
				if( availableVehicles[index].radarRange ) {
					Com_sprintf( text6, MF_MAX_INFOLENGTH, "%d (A)", availableVehicles[index].radarRange );
				} else if( availableVehicles[index].radarRange2 ) {
					Com_sprintf( text6, MF_MAX_INFOLENGTH, "%d (G)", availableVehicles[index].radarRange2 );
				}
			}
		} else {
			strcpy( text6, "No Radar" );
		}
		s_encyclopedia.health.string =  text1;
		s_encyclopedia.maxspeed.string = text2;
		s_encyclopedia.stallspeed.string = text3;
		s_encyclopedia.fuel.string = text4;
		s_encyclopedia.turnspeed.string = text5;
		s_encyclopedia.radar.string = text6;

		// weapons
		y = 250;
		for( i = WP_MACHINEGUN; i <= WP_FLARE; i++ ) {
			if( availableVehicles[index].weapons[i] != WI_NONE ) {
				Com_sprintf( textw[i], MF_MAX_INFOLENGTH, "%s (%d)", 
					availableWeapons[availableVehicles[index].weapons[i]].descriptiveName,
					availableVehicles[index].ammo[i] );
				s_encyclopedia.weapon[i].string = textw[i];
				s_encyclopedia.weapon[i].generic.flags &= ~QMF_HIDDEN;
				s_encyclopedia.weapon[i].generic.y = y;
				y += 20;
			} else {
				s_encyclopedia.weapon[i].generic.flags |= QMF_HIDDEN;
			}
		}

		s_encyclopedia.vehicle.generic.name = va("models/vehicles/%s/%s/%s_icon", dir,
			availableVehicles[index].modelName, availableVehicles[index].modelName);
		s_encyclopedia.vehicle.shader = 0;

		}
//		break;
//	}
}


/*
===============
Encyclopedia_MenuEvent
===============
*/
static void Encyclopedia_MenuEvent( void *ptr, int event ) {
	if( event != QM_ACTIVATED ) {
		return;
	}

	switch( ((menucommon_s*)ptr)->id ) {

	case ID_GAMESET:
	case ID_TEAM:
	case ID_CATEGORY:
	case ID_CLASS:
	case ID_NEXT:
		setItemInformation(((menucommon_s*)ptr)->id);
		break;

	case ID_SHOWVEHICLE:
		UI_VehShowMenu( s_encyclopedia.vindex );
		break;

	case ID_BACK:
		UI_PopMenu();
		break;

	}
}

/*
===============
Encyclopedia_DrawNameBar
===============
*/
static void Encyclopedia_DrawNameBar( void *self ) {
	menubitmap_s	*s;
	float			*color;
	int				style;
	qboolean		focus;
	int				x,y,w;

	s = (menubitmap_s *)self;

	// prepare
	x = s->generic.x;
	y =	s->generic.y;
	w = 58 + s_encyclopedia.namelen * SMALLCHAR_WIDTH;

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

	// NOTE: harcoding the numbers is BAAAAAAAAD style! FIX THIS!
	if ( focus )
	{
		// draw cursor
		UI_FillRect( s->generic.left, s->generic.top, w+1, s->generic.bottom-s->generic.top+1, color_darkred ); 
		UI_DrawChar( 245, 185, 13, UI_CENTER|UI_BLINK|UI_SMALLFONT, color);
	}

	UI_DrawString( x, y, "Name:", style|UI_LEFT, color );
	UI_DrawString( 253, 185, s_encyclopedia.namestring, style|UI_LEFT, color );

}

/*
===============
EncyclopediaMenu_Draw
===============
*/
static void EncyclopediaMenu_Draw( void ) {
//	refdef_t		refdef;
//	refEntity_t		ent1,ent2;
//	vec3_t			origin;
//	vec3_t			angles;
//	float			x, y, w, h;
//	vec4_t			color = {0.5, 0, 0, 1};

	UI_DrawNamedPic( 0, 0, 640, 480, ART_LOGO );

	if( s_encyclopedia.vehicle.generic.parent->cursor == s_encyclopedia.vehicle.generic.menuPosition ) {
		UI_DrawRect(419, 99, 162, 122, color_grey );
	} else {
		UI_DrawRect(419, 99, 162, 122, color_black );
	}

	// standard menu drawing
	Menu_Draw( &s_encyclopedia.menu );

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
	origin[2] = 25;

	trap_R_ClearScene();

	// add the models
	memset( &ent1, 0, sizeof(ent1) );
	origin[1] = -230;
	VectorSet( angles, -90, 90, -90 );
	AnglesToAxis( angles, ent1.axis );
	ent1.hModel = s_encyclopedia.vmodel;
	VectorCopy( origin, ent1.origin );
	VectorCopy( origin, ent1.lightingOrigin );
	ent1.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent1.origin, ent1.oldorigin );
	trap_R_AddRefEntityToScene( &ent1 );
	
	memset( &ent2, 0, sizeof(ent2) );
	origin[1] = -130;
	VectorSet( angles, -90, 90, 90 );
	AnglesToAxis( angles, ent2.axis );
	ent2.hModel = s_encyclopedia.vmodel;
	VectorCopy( origin, ent2.origin );
	VectorCopy( origin, ent2.lightingOrigin );
	ent2.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( ent2.origin, ent2.oldorigin );
	trap_R_AddRefEntityToScene( &ent2 );

	trap_R_RenderScene( &refdef );*/
}

/*
===============
Encyclopedia_MenuInit
===============
*/
static void Encyclopedia_MenuInit( void ) {

	memset( &s_encyclopedia, 0 ,sizeof(encyclopedia_t) );

	Encyclopedia_Cache();

	if ( game_running ) {
	    s_encyclopedia.menu.fullscreen = qfalse;
	} else {
	    s_encyclopedia.menu.fullscreen = qtrue;
	}

	s_encyclopedia.menu.wrapAround = qtrue;
	s_encyclopedia.menu.draw = EncyclopediaMenu_Draw;
	
	if( game_running ) {
		s_encyclopedia.gameset = UI_GetGameset();
	} else {
		s_encyclopedia.gameset = MF_GAMESET_MODERN;
	}
	s_encyclopedia.team = MF_TEAM_MIN;
	s_encyclopedia.category = CAT_MIN;
	s_encyclopedia.cls = CLASS_MIN;
	s_encyclopedia.vindex = -1;

	s_encyclopedia.banner.generic.type			= MTYPE_BTEXT;
	s_encyclopedia.banner.generic.x				= 320;
	s_encyclopedia.banner.generic.y				= 10;
	s_encyclopedia.banner.string				= "ENCYCLOPEDIA";
	s_encyclopedia.banner.color					= color_black;
	s_encyclopedia.banner.style					= UI_CENTER;

	if( !game_running ) {
		s_encyclopedia.vgameset.generic.type		= MTYPE_SPINCONTROL;
		s_encyclopedia.vgameset.generic.name		= "Gameset:";
		s_encyclopedia.vgameset.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS|QMF_SMALLFONT;
		s_encyclopedia.vgameset.generic.callback	= Encyclopedia_MenuEvent;
		s_encyclopedia.vgameset.generic.id			= ID_GAMESET;
		s_encyclopedia.vgameset.generic.x			= 245;
		s_encyclopedia.vgameset.generic.y			= 105;
	}

	s_encyclopedia.vteam.generic.type			= MTYPE_SPINCONTROL;
	s_encyclopedia.vteam.generic.name			= "Team:";
	s_encyclopedia.vteam.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS|QMF_SMALLFONT;
	s_encyclopedia.vteam.generic.callback		= Encyclopedia_MenuEvent;
	s_encyclopedia.vteam.generic.id				= ID_TEAM;
	s_encyclopedia.vteam.generic.x				= 245;
	s_encyclopedia.vteam.generic.y				= 125;

	s_encyclopedia.vcategory.generic.type		= MTYPE_SPINCONTROL;
	s_encyclopedia.vcategory.generic.name		= "Category:";
	s_encyclopedia.vcategory.generic.flags		= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS|QMF_SMALLFONT;
	s_encyclopedia.vcategory.generic.callback	= Encyclopedia_MenuEvent;
	s_encyclopedia.vcategory.generic.id			= ID_CATEGORY;
	s_encyclopedia.vcategory.generic.x			= 245;
	s_encyclopedia.vcategory.generic.y			= 145;

	s_encyclopedia.vclass.generic.type			= MTYPE_SPINCONTROL;
	s_encyclopedia.vclass.generic.name			= "Class:";
	s_encyclopedia.vclass.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS|QMF_SMALLFONT;
	s_encyclopedia.vclass.generic.callback		= Encyclopedia_MenuEvent;
	s_encyclopedia.vclass.generic.id			= ID_CLASS;
	s_encyclopedia.vclass.generic.x				= 245;
	s_encyclopedia.vclass.generic.y				= 165;


	s_encyclopedia.next.generic.type			= MTYPE_BITMAP;
	s_encyclopedia.next.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_encyclopedia.next.generic.id				= ID_NEXT;
	s_encyclopedia.next.generic.callback		= Encyclopedia_MenuEvent;
	s_encyclopedia.next.generic.ownerdraw		= Encyclopedia_DrawNameBar;
	s_encyclopedia.next.generic.x				= 195;
	s_encyclopedia.next.generic.y				= 185;
	s_encyclopedia.next.width					= 130;
	s_encyclopedia.next.height					= 18;

	s_encyclopedia.back.generic.type			= MTYPE_BITMAP;
	s_encyclopedia.back.generic.name			= ART_BACK;
	s_encyclopedia.back.generic.flags			= QMF_LEFT_JUSTIFY|QMF_HIGHLIGHT_IF_FOCUS;
	s_encyclopedia.back.generic.id				= ID_BACK;
	s_encyclopedia.back.generic.callback		= Encyclopedia_MenuEvent;
	s_encyclopedia.back.generic.x				= 20;
	s_encyclopedia.back.generic.y				= 425;
	s_encyclopedia.back.width					= 96;
	s_encyclopedia.back.height					= 32;
	s_encyclopedia.back.focuspic				= ART_BACK_A;

	// data
	s_encyclopedia.health.generic.type			= MTYPE_TEXT;
	s_encyclopedia.health.generic.name			= "Armor:";
	s_encyclopedia.health.generic.x				= 100;
	s_encyclopedia.health.generic.y				= 250;
	s_encyclopedia.health.string				= "x";
	s_encyclopedia.health.color					= color_grey;
	s_encyclopedia.health.style					= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.maxspeed.generic.type		= MTYPE_TEXT;
	s_encyclopedia.maxspeed.generic.name		= "Topspeed:";
	s_encyclopedia.maxspeed.generic.x			= 100;
	s_encyclopedia.maxspeed.generic.y			= 270;
	s_encyclopedia.maxspeed.string				= "x";
	s_encyclopedia.maxspeed.color				= color_grey;
	s_encyclopedia.maxspeed.style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.stallspeed.generic.type		= MTYPE_TEXT;
	s_encyclopedia.stallspeed.generic.name		= "Stallspeed:";
	s_encyclopedia.stallspeed.generic.x			= 100;
	s_encyclopedia.stallspeed.generic.y			= 290;
	s_encyclopedia.stallspeed.string			= "x";
	s_encyclopedia.stallspeed.color				= color_grey;
	s_encyclopedia.stallspeed.style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.turnspeed.generic.type		= MTYPE_TEXT;
	s_encyclopedia.turnspeed.generic.name		= "Turnspeed:";
	s_encyclopedia.turnspeed.generic.x			= 100;
	s_encyclopedia.turnspeed.generic.y			= 310;
	s_encyclopedia.turnspeed.string				= "x";
	s_encyclopedia.turnspeed.color				= color_grey;
	s_encyclopedia.turnspeed.style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.fuel.generic.type			= MTYPE_TEXT;
	s_encyclopedia.fuel.generic.name			= "Fuel:";
	s_encyclopedia.fuel.generic.x				= 100;
	s_encyclopedia.fuel.generic.y				= 330;
	s_encyclopedia.fuel.string					= "x";
	s_encyclopedia.fuel.color					= color_grey;
	s_encyclopedia.fuel.style					= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.radar.generic.type			= MTYPE_TEXT;
	s_encyclopedia.radar.generic.name			= "Radar Range:";
	s_encyclopedia.radar.generic.x				= 100;
	s_encyclopedia.radar.generic.y				= 350;
	s_encyclopedia.radar.string					= "x";
	s_encyclopedia.radar.color					= color_grey;
	s_encyclopedia.radar.style					= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.weapon[0].generic.type		= MTYPE_TEXT;
	s_encyclopedia.weapon[0].generic.name		= "Guns:";
	s_encyclopedia.weapon[0].generic.x			= 340;
	s_encyclopedia.weapon[0].string				= "x";
	s_encyclopedia.weapon[0].color				= color_grey;
	s_encyclopedia.weapon[0].style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.weapon[1].generic.type		= MTYPE_TEXT;
	s_encyclopedia.weapon[1].generic.name		= "Weapon 1:";
	s_encyclopedia.weapon[1].generic.x			= 340;
	s_encyclopedia.weapon[1].string				= "x";
	s_encyclopedia.weapon[1].color				= color_grey;
	s_encyclopedia.weapon[1].style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.weapon[2].generic.type		= MTYPE_TEXT;
	s_encyclopedia.weapon[2].generic.name		= "Weapon 2:";
	s_encyclopedia.weapon[2].generic.x			= 340;
	s_encyclopedia.weapon[2].string				= "x";
	s_encyclopedia.weapon[2].color				= color_grey;
	s_encyclopedia.weapon[2].style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.weapon[3].generic.type		= MTYPE_TEXT;
	s_encyclopedia.weapon[3].generic.name		= "Weapon 3:";
	s_encyclopedia.weapon[3].generic.x			= 340;
	s_encyclopedia.weapon[3].string				= "x";
	s_encyclopedia.weapon[3].color				= color_grey;
	s_encyclopedia.weapon[3].style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.weapon[4].generic.type		= MTYPE_TEXT;
	s_encyclopedia.weapon[4].generic.name		= "Weapon 4:";
	s_encyclopedia.weapon[4].generic.x			= 340;
	s_encyclopedia.weapon[4].string				= "x";
	s_encyclopedia.weapon[4].color				= color_grey;
	s_encyclopedia.weapon[4].style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.weapon[5].generic.type		= MTYPE_TEXT;
	s_encyclopedia.weapon[5].generic.name		= "Weapon 5:";
	s_encyclopedia.weapon[5].generic.x			= 340;
	s_encyclopedia.weapon[5].string				= "x";
	s_encyclopedia.weapon[5].color				= color_grey;
	s_encyclopedia.weapon[5].style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.weapon[6].generic.type		= MTYPE_TEXT;
	s_encyclopedia.weapon[6].generic.name		= "Weapon 6:";
	s_encyclopedia.weapon[6].generic.x			= 340;
	s_encyclopedia.weapon[6].string				= "x";
	s_encyclopedia.weapon[6].color				= color_grey;
	s_encyclopedia.weapon[6].style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.weapon[7].generic.type		= MTYPE_TEXT;
	s_encyclopedia.weapon[7].generic.name		= "Counter:";
	s_encyclopedia.weapon[7].generic.x			= 340;
	s_encyclopedia.weapon[7].string				= "x";
	s_encyclopedia.weapon[7].color				= color_grey;
	s_encyclopedia.weapon[7].style				= UI_LEFT|UI_SMALLFONT|UI_DATAALIGN;

	s_encyclopedia.vehicle.generic.type			= MTYPE_BITMAP;
	s_encyclopedia.vehicle.generic.name			= ART_BACK;
	s_encyclopedia.vehicle.generic.flags		= QMF_LEFT_JUSTIFY;
	s_encyclopedia.vehicle.generic.callback		= Encyclopedia_MenuEvent;
	s_encyclopedia.vehicle.generic.x			= 420;
	s_encyclopedia.vehicle.generic.y			= 100;
	s_encyclopedia.vehicle.generic.id			= ID_SHOWVEHICLE;
	s_encyclopedia.vehicle.width				= 160;
	s_encyclopedia.vehicle.height				= 120;
	s_encyclopedia.vehicle.focuspic				= ART_BACK_A;

	setItemInformation(ID_ALL);

	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.banner );
	if( !game_running ) {
		Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.vgameset );
	}
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.vteam );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.vcategory );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.vclass );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.next );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.back );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.health );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.maxspeed );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.stallspeed );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.fuel );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.radar );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.turnspeed );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.weapon[0] );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.weapon[1] );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.weapon[2] );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.weapon[3] );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.weapon[4] );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.weapon[5] );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.weapon[6] );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.weapon[7] );
	Menu_AddItem( &s_encyclopedia.menu, &s_encyclopedia.vehicle );
}

/*
=================
Encyclopedia_Cache
=================
*/
void Encyclopedia_Cache( void ) {
	int i;
	int cat;
	char dir[16];

	trap_R_RegisterShaderNoMip( ART_LOGO );
	trap_R_RegisterShaderNoMip( ART_BACK );
	trap_R_RegisterShaderNoMip( ART_BACK_A );
	trap_R_RegisterShaderNoMip( ART_NEXT );
	trap_R_RegisterShaderNoMip( ART_NEXT_A );

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
UI_EncyclopediaMenu
===============
*/
void UI_EncyclopediaMenu( void ) {
	uiClientState_t	cstate;

	trap_GetClientState( &cstate );

	if ( cstate.connState >= CA_CONNECTED ) {
		// float on top of running game
		game_running = qtrue;
	}
	else {
		// game not running
		game_running = qfalse;
	}

	Encyclopedia_MenuInit();
	UI_PushMenu( &s_encyclopedia.menu );
}
