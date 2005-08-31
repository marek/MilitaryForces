/*
 * $Id: bg_mfq3util.c,v 1.4 2005-08-31 19:20:06 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"

// externals
extern void	trap_Cvar_Set( const char *var_name, const char *value );
extern void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void	trap_FS_Read( void *buffer, int len, fileHandle_t f );
void	trap_FS_FCloseFile( fileHandle_t f );

/*
=================
MF_getIndexOfVehicleEx
=================
*/

void MF_SetMissionScriptOverviewDefaults( mission_overview_t* overview )
{
	overview->gameset = -1;
	overview->gametype = -1;
	overview->mapname[0] = 0;
	overview->missionname[0] = 0;
	overview->objective[0] = 0;
	overview->valid = false;
}

// updateFormat should only be set to true to be backwards compatible after a format change
// so that incomplete values can be filled in
void MF_CheckMissionScriptOverviewValid( mission_overview_t* overview, bool updateFormat )
{
	if( overview->gameset >= 0 &&
		overview->gametype >= 0 &&
		overview->mapname[0] > 0 &&	// very simple check
		overview->missionname[0] > 0 &&
		overview->objective[0] > 0 )
		overview->valid = true;

	if( updateFormat )
	{
		overview->valid = true;
		if( overview->gameset < 0 )
			overview->gameset = 0;
		if( overview->gametype < 0 )
			overview->gametype = 0;
		if( overview->mapname[0] == 0 )
			strcpy(overview->mapname, "mfq31");
		if( overview->mapname[0] == 0 )
			strcpy(overview->missionname, "bjoern1");
		if( overview->mapname[0] == 0 )
			strcpy(overview->objective, "SearchAndDestroy");


	}
}


/*
=================
MF_getIndexOfVehicleEx
=================
*/

int MF_getIndexOfVehicleEx( int start,
							int gameset,
							int team,
							int cat,
							int cls,
						    int vehicleType,
						    int change_vehicle,
							bool allowNukes)
{

	// NOTE: vehicleClass & vehicleCat are int enum indexed, convert to flag enums

	// CATAGORY
	if( cat >= 0 )
	{
		// specific catagory
		cat = 1 << cat;		// (convert from int enum to flag enum)
	}

	// CLASS
	if( cls >= 0 )
	{
		// specific class
		cls = 1 << cls;		// (convert from int enum to flag enum)
	}

	return MF_getIndexOfVehicle( start, gameset, team, cat, cls, vehicleType, change_vehicle, allowNukes );
}

/*
=================
MF_getIndexOfVehicle
=================
*/
int MF_getIndexOfVehicle( int start,			// where to start in list
						  int gameset,
						  int team,
						  int cat,
						  int cls, 
						  int vehicleType,
						  int change_vehicle,
						  bool allowNukes)
{
    int				i,j;
	int				vehicle_pt = -1;
	bool		done = false;

	// null or negative? use ALL values
	if( gameset <= 0 ) gameset = MF_GAMESET_ANY;
	if( team <= 0 ) team = MF_TEAM_ANY;
	if( cat <= 0 ) cat = CAT_ANY;
	if( cls <= 0 ) cls = CLASS_ANY;

	if( start >= bg_numberOfVehicles )
	{
		start = 0;
	}
	else if( start < 0 )
	{
		start = bg_numberOfVehicles;
	}

	// scan loop
    for( i = start+1; done != true; i++ )
    {
		if( i >= bg_numberOfVehicles )
		{
			if( start == bg_numberOfVehicles && i == start )
			{
				return -1;
			}
			
			i = 0;					
		}

		if( i == start ) done = true;//return start;	

		if( !(availableVehicles[i].gameset & gameset) ) continue;			// wrong set
		if( !(availableVehicles[i].team & team) ) continue;					// wrong team
		if( !(availableVehicles[i].cat & cat) ) continue;					// wrong category
		if( !(availableVehicles[i].cls & cls) ) continue;					// wrong class
		
		if(!allowNukes)
		{
			bool nukes = false;
			for(j = 0; j < MAX_WEAPONS_PER_VEHICLE; j++)
			{
				if( availableWeapons[availableVehicles[i].weapons[j]].type == WT_NUKEBOMB || 
					availableWeapons[availableVehicles[i].weapons[j]].type == WT_NUKEMISSILE )
				{
						vehicle_pt = -1;
						nukes = true;
				}
			}
			if(nukes) 
				continue;
		} 

		if(vehicleType != -1 && change_vehicle == 1 && strcmp(availableVehicles[i].tinyName, availableVehicles[vehicleType].tinyName) == 0)
		{
			//If its good mark known position just in case its the only good one
			vehicle_pt = i;
			continue;	// change vehicle
		}
		else if(change_vehicle == 2 && vehicleType != -1 && strcmp(availableVehicles[i].tinyName, availableVehicles[vehicleType].tinyName) != 0)
		{
			continue;						// make sure its the same
		}

		return i;
    }

    return vehicle_pt;
}


/*
=================
MF_getIndexOfGI
=================
*/
int MF_getIndexOfGI( int start, int gameset, int GIType, int mode)
{
    int				i;
	int				gi_pt = -1;
	bool		done = false;

	if( gameset <= 0 ) gameset = MF_GAMESET_ANY;
	if( start >= bg_numberOfGroundInstallations )
		start = 0;
	else if( start < 0 )
		start = bg_numberOfGroundInstallations;

	// scan loop
    for( i = start+1; done != true; i++ )
    {
		if(i >= bg_numberOfGroundInstallations)
			i = 0;					

		if( i == start ) done = true;			//return start;	
		if( !(availableGroundInstallations[i].gameset & gameset) ) continue;			// wrong set
		
		if(GIType != -1) {
			if(mode == 1 && strcmp(availableGroundInstallations[i].tinyName, availableGroundInstallations[GIType].tinyName) == 0) {
				//If its good mark known position just in case its the only good one
				gi_pt = i;
				continue;	// change vehicle
			}else if(mode == 2 && strcmp(availableGroundInstallations[i].tinyName, availableGroundInstallations[GIType].tinyName) != 0) {
				continue;						// make sure its the same
			}
		}
		return i;
    }

    return gi_pt;
}


/*
=================
MF_getNumberOfItems
=================
*/

// returns the number of items in a stringlist
int MF_getNumberOfItems(const char **itemlist)
{
	int i;
	for( i = 0; itemlist[i] != 0; i++ );
	return i;
}

/*
=================
MF_getItemIndexFromHex
=================
*/

// takes an already properly right-shifted hex value (with only 1 bit set!)
// and returns the item index for it
int MF_getItemIndexFromHex(int hexValue)
{
	int i;
	for( i = -1; hexValue; i++ ) hexValue>>=1;
	return i;
}

/*
=================
MF_CreateModelPathname
=================
*/

// creates a model pathname (e.g. pFormatString = "models/vehicles/%s/%s/%s_icon")
char * MF_CreateModelPathname( int vehicle, char * pFormatString )
{
	char * pReturnString = NULL;
	char catDir[ 32 ];
	unsigned int cat = 0;

	// find catagory
	cat = availableVehicles[ vehicle ].cat;

	if( cat & CAT_PLANE ) {
		strcpy( catDir, "planes" );
	}
	else if( cat & CAT_GROUND ) {
		strcpy( catDir, "ground" );
	}
	else if( cat & CAT_HELO ) {
		strcpy( catDir, "helos" );
	}
	else if( cat & CAT_NPC ) {
		strcpy( catDir, "npc" );
	}
	else if( cat & CAT_LQM ) {
		strcpy( catDir, "lqms" );
	}
	else if( cat & CAT_BOAT ) {
		strcpy( catDir, "sea" );
	}

	// create filename string
	pReturnString = va( pFormatString, catDir, availableVehicles[ vehicle ].modelName, availableVehicles[ vehicle ].modelName );

	return pReturnString;
}

// same for groundinstallations
char * MF_CreateGIPathname( int vehicle, char * pFormatString )
{
	char * pReturnString = NULL;
	char catDir[ 32 ];
//	unsigned int cat = 0;

	// find catagory
	strcpy( catDir, "npc" );

	// create filename string
	pReturnString = va( pFormatString, catDir, availableGroundInstallations[ vehicle ].modelName, 
		availableGroundInstallations[ vehicle ].modelName );

	return pReturnString;
}

/*
=================
MF_LimitFloat
=================
*/

// generic float limiting
void MF_LimitFloat( float * value, float min, float max )
{
	// min & max
	if( *value < min ) *value = min;
	if( *value > max ) *value = max;
}

// generic int limiting
void MF_LimitInt( int * value, int min, int max )
{
	// min & max
	if( *value < min ) *value = min;
	if( *value > max ) *value = max;
}

/*
=================
MF_ExtractEnumFromId

Get the catagory/class enum from the vehicle index (use CAT_ANY & CLASS_ANY to decide operation done)
=================
*/

int MF_ExtractEnumFromId( int vehicle, unsigned int op )
{
	int daEnum = -1;

	// which op?
	if( op & CAT_ANY )
	{
		// catagory
		daEnum = availableVehicles[ vehicle ].cat;
	}
	else if( op & CLASS_ANY )
	{
		// class
		daEnum = availableVehicles[ vehicle ].cls;
	}

	return daEnum;
}



/*
=================
MF_ExtractEnumFromId

Get the catagory/class enum from the vehicle index (use CAT_ANY & CLASS_ANY to decide operation done)
=================
*/

void MF_LoadAllVehicleData()
{
	int i, j, num;
	char* modelbasename = 0;
	char name[128];
	fileHandle_t	f;
	md3Tag_t tag;
	float diff;
	vec3_t max, min;

	// MFQ3 loadouts
	MF_calculateAllDefaultLoadouts();

	for( i = 0; i < bg_numberOfVehicles; ++i ) 
	{
		// remove weapons not fitting
		for( j = WP_WEAPON1; j < WP_FLARE; ++j ) 
		{
			if( !(availableVehicles[i].cat & availableWeapons[availableVehicles[i].weapons[j]].fitsCategory) )
				availableVehicles[i].ammo[j] = 0;
		}
		modelbasename = MF_CreateModelPathname( i, "models/vehicles/%s/%s/%s" );	
		// boundingbox
		Com_sprintf( name, sizeof(name), "%s.md3", modelbasename );
		MF_getDimensions( name, 0, &availableVehicles[i].maxs, &availableVehicles[i].mins );
		availableVehicles[i].mins[2] += 1;// to look better ?

		// helo/plane specific
		if(availableVehicles[i].cat & CAT_PLANE) 
		{
			// Find gear tag offset from  min
			MF_findTag(name, "tag_gear", &tag);
			diff = tag.origin[2] - availableVehicles[i].mins[2];
			if(diff < 0) diff = 0;

			// gear
			Com_sprintf( name, sizeof(name), "%s_gear.md3", modelbasename );

			trap_FS_FOpenFile(name, &f, FS_READ);

			if(f &&  MF_getNumberOfFrames( name, &num ) ) 
			{
				vec3_t min1, min2;

				trap_FS_FCloseFile(f);
				availableVehicles[i].maxGearFrame = num - 1;

				
				if(!availableVehicles[i].gearheight)
				{
					if( MF_getDimensions( name, 0, 0, &min1 ) &&
						MF_getDimensions( name, num-1, 0, &min2 ) ) 
					{
						availableVehicles[i].gearheight = min1[2] - min2[2] - 1.5;// for coll. detection
						if( availableVehicles[i].gearheight < 0 ) 
							availableVehicles[i].gearheight = 0;
					}
				}
				/*
				MF_findTag(name, "tag_gear", &tag);
				MF_getDimensions(name, num-1, &max,&min);
				availableVehicles[i].gearheight = (tag.origin[2] - min[2]) - diff;
				if( availableVehicles[i].gearheight < 0 ) availableVehicles[i].gearheight = 0;
				*/

				// Don't bother setting the gearheight, its too fucken bugged.
			} 
			else 
				availableVehicles[i].maxGearFrame = GEAR_DOWN_DEFAULT;

			// bay
			Com_sprintf( name, sizeof(name), "%s_bay.md3", modelbasename );

			trap_FS_FOpenFile(name, &f, FS_READ);

			if(f && MF_getNumberOfFrames( name, &num ) ) 
			{
				trap_FS_FCloseFile(f);
				availableVehicles[i].maxBayFrame = num - 1;
			}
			else 
				availableVehicles[i].maxBayFrame = BAY_DOWN_DEFAULT;

			// correct actual loadouts
			for( j = WP_WEAPON1; j < WP_FLARE; ++j ) {
				if( availableVehicles[i].weapons[j] ) {
					availableVehicles[i].ammo[j] = 
						MF_findWeaponsOfType( availableVehicles[i].weapons[j], &availableLoadouts[i] );
				}
			}
		} else if(availableVehicles[i].cat & CAT_HELO) {

			// Find gear tag offset from  min
			MF_findTag(name, "tag_gear", &tag);
			diff = tag.origin[2] - availableVehicles[i].mins[2];
			if(diff < 0) diff = 0;


			// gear
			Com_sprintf( name, sizeof(name), "%s_gear.md3", modelbasename );

			trap_FS_FOpenFile(name, &f, FS_READ);

			if(f &&  MF_getNumberOfFrames( name, &num ) ) {
				vec3_t min1, min2;

				trap_FS_FCloseFile(f);
				availableVehicles[i].maxGearFrame = num - 1;

				
				if(!availableVehicles[i].gearheight){

					if( MF_getDimensions( name, 0, 0, &min1 ) &&
						MF_getDimensions( name, num-1, 0, &min2 ) ) {
						availableVehicles[i].gearheight = min1[2] - min2[2] - 1.5;// for coll. detection
						if( availableVehicles[i].gearheight < 0 ) availableVehicles[i].gearheight = 0;
					}
				
				}
			} else {
				availableVehicles[i].maxGearFrame = GEAR_DOWN_DEFAULT;
			}

		} else if( (availableVehicles[i].cat & CAT_GROUND) ) {

			// increaes bounding box by turret height
			Com_sprintf( name, sizeof(name), "%s_tur.md3", modelbasename );
			MF_getDimensions( name, 0, &max, &min );
			diff = max[2] - min[2];
			availableVehicles[i].maxs[2] += diff;
		//	availableVehicles[i].mins[2] -= 2;	

			// Increase bounding box if it has wheels!
			// Increase rougly by 1/2 wheel length?

			// Make sure its actualy listed in the vehicle data that it does (Although not needed)
			// Then chick if file exists.. read value and divide by half.
			if(availableVehicles[i].wheels)
			{
				Com_sprintf( name, sizeof(name), "%s_w1.md3", modelbasename );
				trap_FS_FOpenFile(name, &f, FS_READ);
				if(f)
				{
					trap_FS_FCloseFile(f);
		
					MF_findTag(name, "tag_wheel", &tag);
					MF_getDimensions( name, 0, &max, &min );
					diff = max[2] - min[2];
					availableVehicles[i].mins[2] = tag.origin[2] - (diff / 2);
				}
				
			}

		} else if( (availableVehicles[i].cat & CAT_BOAT) ) {
			availableVehicles[i].mins[2] -= 2;
		} else if ( availableVehicles[i].cat & CAT_LQM) {
			// boundingbox
			Com_sprintf( name, sizeof(name), "%s_legs.md3", modelbasename );
			MF_getDimensions( name, LEGS_IDLE, &availableVehicles[i].maxs, &availableVehicles[i].mins );
			Com_sprintf( name, sizeof(name), "%s_torso.md3", modelbasename );
			MF_getDimensions( name, LEGS_IDLE, &max, &min );
			availableVehicles[i].maxs[2] += max[2] - min[2];
			Com_sprintf( name, sizeof(name), "%s_head.md3", modelbasename );
			MF_getDimensions( name, LEGS_IDLE, &max, &min );
			availableVehicles[i].maxs[2] += max[2] - min[2];
			// Scale LQMs
			for(j = 0; j < 3; j++) {
				availableVehicles[i].maxs[j] *= (float)LQM_SCALE;
				availableVehicles[i].mins[j] *= (float)LQM_SCALE;
			}
		}
	}
	for( i = 0; i < bg_numberOfGroundInstallations; ++i ) {
		modelbasename = MF_CreateGIPathname( i, "models/vehicles/%s/%s/%s" );	
		// boundingbox
		Com_sprintf( name, sizeof(name), "%s.md3", modelbasename );
		MF_getDimensions( name, 0, &availableGroundInstallations[i].maxs, 
				&availableGroundInstallations[i].mins );
		availableGroundInstallations[i].maxs[2] += (15 + 7 * availableGroundInstallations[i].upgrades);
	}
}








/*
=================

Mission script parsing

=================
*/


static void MF_ParseOverview( char **buf, mission_overview_t* overview ) 
{
	char	*token;
	char	info[MAX_INFO_STRING];

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		//trap_Printf(va("Token: '%s' - END\n", token));
		return;
	}
//	Com_Printf(va("O-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Com_Printf( "Missing { in overview\n" );
		return;
	} 

	info[0] = '\0';
	while( 1 ) 
	{
		token = COM_ParseExt( buf, true );
		if( !token[0] ) 
		{
//				Com_Printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Com_Printf( "Unexpected { found\n" );
			break;
		}
		if( !strcmp( token, "}" ) ) 
		{
			break;
		}
		if( !strcmp( token, "map" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(overview->mapname, token, MAX_NAME_LENGTH );
				continue;
			}
		}
		else if( !strcmp( token, "gameset" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				overview->gameset = atoi(token);
				continue;
			}
		}
		else if( !strcmp( token, "gametype" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				overview->gametype = atoi(token);
				continue;
			}
		}
		else if( !strcmp( token, "mission" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(overview->missionname, token, MAX_NAME_LENGTH );
				continue;
			}
		}
		else if( !strcmp( token, "goal" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(overview->objective, token, MAX_NAME_LENGTH );
				continue;
			}
		}
	}
	MF_CheckMissionScriptOverviewValid(overview, false);// set to true after format change
}


static void MF_ParseWaypoints( char **buf, mission_vehicle_t* veh ) 
{
	char	*token;
	int		num = 0;

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		//trap_Printf(va("Token: '%s' - END\n", token));
		return;
	}
//	Com_Printf(va("W-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Com_Printf( "Missing { in vehicle\n" );
		return;
	} 

	while( 1 ) 
	{
		token = COM_ParseExt( buf, true );
		if( !token[0] ) 
		{
//				Com_Printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Com_Printf( "Unexpected { found\n" );
			break;
		}
		if( !strcmp( token, "}" ) ) 
		{
			break;
		}
		if( !strcmp( token, "Origin" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				sscanf(token, "%f;%f;%f", &veh->waypoints[num].origin[0],
					&veh->waypoints[num].origin[1], &veh->waypoints[num].origin[2]);
				veh->waypoints[num].used = true;
				++num;
				continue;
			}
		}
	}
}

static void MF_ParseVehicle( char **buf, mission_vehicle_t* veh ) 
{
	char	*token;

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		//trap_Printf(va("Token: '%s' - END\n", token));
		return;
	}
//	Com_Printf(va("V-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Com_Printf( "Missing { in vehicle\n" );
		return;
	} 

	while( 1 ) 
	{
		token = COM_ParseExt( buf, true );
		if( !token[0] ) 
		{
//				Com_Printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Com_Printf( "Unexpected { found\n" );
			break;
		}
		if( !strcmp( token, "}" ) ) 
		{
			break;
		}

		if( !strcmp( token, "Index" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				veh->index = atoi(token);
				continue;
			}
		}
		else if( !strcmp( token, "Name" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(veh->objectname, token, MAX_NAME_LENGTH );
				continue;
			}
		}
		else if( !strcmp( token, "Team" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				veh->team = atoi(token);
				continue;
			}
		}
		else if( !strcmp( token, "Origin" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				sscanf(token, "%f;%f;%f", &veh->origin[0],
					&veh->origin[1], &veh->origin[2]);
				continue;
			}
		}
		else if( !strcmp( token, "Angles" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				sscanf(token, "%f;%f;%f", &veh->angles[0],
					&veh->angles[1], &veh->angles[2]);
				continue;
			}
		}
		else if( !strcmp( token, "Waypoints" ) )
		{
			MF_ParseWaypoints(buf, veh);
		}
	}
	veh->used = true;
}

static void MF_ParseGroundInstallation( char **buf, mission_groundInstallation_t* gi ) 
{
	char	*token;

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		//trap_Printf(va("Token: '%s' - END\n", token));
		return;
	}
//	Com_Printf(va("GV-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Com_Printf( "Missing { in groundinstallation\n" );
		return;
	} 

	while( 1 ) 
	{
		token = COM_ParseExt( buf, true );
		if( !token[0] ) 
		{
//				Com_Printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Com_Printf( "Unexpected { found\n" );
			break;
		}
		if( !strcmp( token, "}" ) ) 
		{
			break;
		}

		if( !strcmp( token, "Index" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				gi->index = atoi(token);
				continue;
			}
		}
		else if( !strcmp( token, "Name" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(gi->objectname, token, MAX_NAME_LENGTH );
				continue;
			}
		}
		else if( !strcmp( token, "Team" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(gi->teamname, token, MAX_NAME_LENGTH );
				continue;
			}
		}
		else if( !strcmp( token, "Origin" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				sscanf(token, "%f;%f;%f", &gi->origin[0],
					&gi->origin[1], &gi->origin[2]);
				continue;
			}
		}
		else if( !strcmp( token, "Angles" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				sscanf(token, "%f;%f;%f", &gi->angles[0],
					&gi->angles[1], &gi->angles[2]);
				continue;
			}
		}
	}
	gi->used = true;
}

static void MF_ParseEntities( char **buf, 
						  mission_vehicle_t* vehs, 
						  mission_groundInstallation_t* gis ) 
{
	char	*token;
	int		numVeh = 0,
			numGI = 0;

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		//trap_Printf(va("Token: '%s' - END\n", token));
		return;
	};
//	Com_Printf(va("E-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Com_Printf( "Missing { in entities\n" );
		return;
	} 

	while( 1 )
	{
		token = COM_Parse( buf );
		if( !token[0] ) 
		{
			Com_Printf(va("Token: '%s' - END\n", token));
			break;
		}
		else if( !strcmp(token, "Vehicle" ) )
		{
			MF_ParseVehicle(buf, &vehs[numVeh]);
			++numVeh;
			continue;
		}
		else if( !strcmp(token, "GroundInstallation" ) )
		{
			MF_ParseGroundInstallation(buf, &gis[numGI]);
			++numGI;
			continue;
		}
		else if( !strcmp( token, "}" ) ) 
		{
			Com_Printf(va("End of Entities\n"));
			break;
		}
		else
		{
			Com_Printf(va(S_COLOR_RED "Invalid Token - parsing cancelled", token));
			break;
		}
	}

}

void MF_ParseMissionScripts( char *buf,
							 mission_overview_t* overview,
							 mission_vehicle_t* vehs, 
							 mission_groundInstallation_t* gis) 
{
	char *token;

	MF_SetMissionScriptOverviewDefaults(overview);

	while( 1 )
	{
		token = COM_Parse( &buf );
		if( !token[0] ) 
		{
			//Com_Printf(va("Token: '%s' - END\n", token));
			break;
		}
		else if( !strcmp(token, "Overview" ) )
		{
			MF_ParseOverview(&buf, overview);
			continue;
		}
		else if( !strcmp(token, "Entities" ) )
		{
			MF_ParseEntities(&buf, vehs, gis);
			continue;
		}
		else
		{
			Com_Printf(va(S_COLOR_RED "Invalid Token - parsing cancelled", token));
			break;
		}
	}
}
