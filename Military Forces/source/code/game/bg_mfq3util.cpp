/*
 * $Id: bg_mfq3util.c,v 1.7 2005-11-21 17:28:20 thebjoern Exp $
*/

#include "q_shared.h"
#include "../qcommon/qfiles.h"
#include "bg_public.h"

// externals


/*
=================
VehicleData::getIndexOfVehicleEx
=================
*/

void VehicleData::setMissionScriptOverviewDefaults( mission_overview_t* overview )
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
void VehicleData::checkMissionScriptOverviewValid( mission_overview_t* overview, bool updateFormat )
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
VehicleData::getIndexOfVehicleEx
=================
*/

int VehicleData::getIndexOfVehicleEx( int start,
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

	return VehicleData::getIndexOfVehicle( start, gameset, team, cat, cls, vehicleType, change_vehicle, allowNukes );
}

/*
=================
VehicleData::getIndexOfVehicle
=================
*/
int VehicleData::getIndexOfVehicle( int start,			// where to start in list
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
		if( !VehicleData::selectable( i ) ) continue;						// not offered for selection
		
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
VehicleData::getIndexOfGI
=================
*/
int VehicleData::getIndexOfGI( int start, int gameset, int GIType, int mode)
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
VehicleData::getNumberOfItems
=================
*/

// returns the number of items in a stringlist
int VehicleData::getNumberOfItems(const char **itemlist)
{
	int i;
	for( i = 0; itemlist[i] != nullptr; i++ );
	return i;
}

/*
=================
VehicleData::getItemIndexFromHex
=================
*/

// takes an already properly right-shifted hex value (with only 1 bit set!)
// and returns the item index for it
int VehicleData::getItemIndexFromHex(int hexValue)
{
	int i;
	for( i = -1; hexValue; i++ ) hexValue>>=1;
	return i;
}

/*
=================
VehicleData::createModelPathname
=================
*/

// creates a model pathname (e.g. pFormatString = "models/vehicles/%s/%s/%s_icon")
char * VehicleData::createModelPathname( int vehicle, char * pFormatString )
{
	char * pReturnString = nullptr;
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
char * VehicleData::createGIPathname( int vehicle, char * pFormatString )
{
	char * pReturnString = nullptr;
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
VehicleData::limitFloat
=================
*/

// generic float limiting
void VehicleData::limitFloat( float * value, float min, float max )
{
	// min & max
	if( *value < min ) *value = min;
	if( *value > max ) *value = max;
}

// generic int limiting
void VehicleData::limitInt( int * value, int min, int max )
{
	// min & max
	if( *value < min ) *value = min;
	if( *value > max ) *value = max;
}

/*
=================
VehicleData::extractEnumFromId

Get the catagory/class enum from the vehicle index (use CAT_ANY & CLASS_ANY to decide operation done)
=================
*/

int VehicleData::extractEnumFromId( int vehicle, unsigned int op )
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
VehicleData::extractEnumFromId

Get the catagory/class enum from the vehicle index (use CAT_ANY & CLASS_ANY to decide operation done)
=================
*/


void VehicleData::loadAllVehicleData()
{
	int i, j, num;
	char* modelbasename = nullptr;
	char name[128];

	// MFQ3 loadouts
	VehicleData::calculateAllDefaultLoadouts();

	for( i = 0; i < bg_numberOfVehicles; ++i ) 
	{
		// remove weapons not fitting
		for( int j : secondaryWeaponSlots ) 
		{
			if( !(availableVehicles[i].cat & availableWeapons[availableVehicles[i].weapons[j]].fitsCategory) )
				availableVehicles[i].ammo[j] = 0;
		}
		modelbasename = VehicleData::createModelPathname( i, "models/vehicles/%s/%s/%s" );	
		// boundingbox
		Com_sprintf( name, sizeof(name), "%s.md3", modelbasename );
		if( !(availableVehicles[i].cat & CAT_LQM) ) {
			if( const auto bounds = VehicleData::getDimensions( name, 0 ) ) {
				VectorCopy( bounds->maxs, availableVehicles[i].maxs );
				VectorCopy( bounds->mins, availableVehicles[i].mins );
			}
		}
		availableVehicles[i].mins[2] += 1;// to look better ?
		if( availableVehicles[i].cat & CAT_BOAT ) 
			VehicleData::findDeck( name, &availableVehicles[i] );
		// boats, transport helicopters and wheeled vehicles carry a crew; attack helicopters and tanks take nobody aboard
		if( availableVehicles[i].cat & CAT_HELO ) 
			availableVehicles[i].seats = (availableVehicles[i].cls & CLASS_HELO_ATTACK) ? 0 : 4;
		else if( availableVehicles[i].cat & CAT_BOAT ) 
			availableVehicles[i].seats = 4;
		else if( availableVehicles[i].cat & CAT_GROUND ) 
			availableVehicles[i].seats = (availableVehicles[i].caps & HC_WHEELS) ? 3 : 0;
		else 
			availableVehicles[i].seats = 0;

		// helo/plane specific
		if(availableVehicles[i].cat & CAT_PLANE) 
		{
			// gear
			Com_sprintf( name, sizeof(name), "%s_gear.md3", modelbasename );

			// only models that have the part file get the part
			if( const auto frames = VehicleData::getNumberOfFrames( name ) ) 
			{
				num = *frames;

				availableVehicles[i].maxGearFrame = num - 1;
				
				if(!availableVehicles[i].gearheight)
				{
					const auto up = VehicleData::getDimensions( name, 0 );
					const auto down = VehicleData::getDimensions( name, num-1 );
					if( up && down ) 
					{
						availableVehicles[i].gearheight = up->mins[2] - down->mins[2] - 1.5;// for coll. detection
						if( availableVehicles[i].gearheight < 0 ) 
							availableVehicles[i].gearheight = 0;
					}
				}
			} 
			else 
				availableVehicles[i].maxGearFrame = GEAR_DOWN_DEFAULT;

			// bay
			Com_sprintf( name, sizeof(name), "%s_bay.md3", modelbasename );

			if( const auto frames = VehicleData::getNumberOfFrames( name ) ) 
			{
				num = *frames;
				availableVehicles[i].maxBayFrame = num - 1;
			}
			else 
				availableVehicles[i].maxBayFrame = BAY_DOWN_DEFAULT;

			// correct actual loadouts
			for( int j : secondaryWeaponSlots ) {
				if( availableVehicles[i].weapons[j] ) {
					availableVehicles[i].ammo[j] = 
						VehicleData::packagesOfType( availableVehicles[i].weapons[j], &availableLoadouts[i] );
				}
			}
		} else if(availableVehicles[i].cat & CAT_HELO) {

			// gear
			Com_sprintf( name, sizeof(name), "%s_gear.md3", modelbasename );

			if( const auto frames = VehicleData::getNumberOfFrames( name ) ) {
				num = *frames;

				availableVehicles[i].maxGearFrame = num - 1;

				
				if(!availableVehicles[i].gearheight){

					const auto up = VehicleData::getDimensions( name, 0 );
					const auto down = VehicleData::getDimensions( name, num-1 );
					if( up && down ) {
						availableVehicles[i].gearheight = up->mins[2] - down->mins[2] - 1.5;// for coll. detection
						if( availableVehicles[i].gearheight < 0 ) availableVehicles[i].gearheight = 0;
					}
				
				}
			} else {
				availableVehicles[i].maxGearFrame = GEAR_DOWN_DEFAULT;
			}

			// bay
			Com_sprintf( name, sizeof(name), "%s_bay.md3", modelbasename );

			if( const auto frames = VehicleData::getNumberOfFrames( name ) ) 
			{
				num = *frames;
				availableVehicles[i].maxBayFrame = num - 1;
			}
			else 
				availableVehicles[i].maxBayFrame = BAY_DOWN_DEFAULT;

			// correct actual loadouts
			if( availableLoadouts[i].usedMounts ) {
				for( int j : secondaryWeaponSlots ) {
					if( availableVehicles[i].weapons[j] &&
						(availableWeapons[availableVehicles[i].weapons[j]].type != WT_MACHINEGUN ||
						 (availableWeapons[availableVehicles[i].weapons[j]].fitsPylon & PF_GUNPOD)) ) {
						availableVehicles[i].ammo[j] = 
							VehicleData::packagesOfType( availableVehicles[i].weapons[j], &availableLoadouts[i] );
					}
				}
			}

		} else if( (availableVehicles[i].cat & CAT_GROUND) ) {

			// increaes bounding box by turret height
			Com_sprintf( name, sizeof(name), "%s_tur.md3", modelbasename );
			if( const auto turret = VehicleData::getDimensions( name, 0 ) )
				availableVehicles[i].maxs[2] += turret->maxs[2] - turret->mins[2];

			// Increase bounding box if it has wheels!
			// Increase rougly by 1/2 wheel length?

			// Make sure its actualy listed in the vehicle data that it does (Although not needed)
			// Then chick if file exists.. read value and divide by half.
			if(availableVehicles[i].wheels)
			{
				Com_sprintf( name, sizeof(name), "%s_w1.md3", modelbasename );
				if( const auto wheel = VehicleData::getDimensions( name, 0 ) )
				{
					// the wheel's own mount point, at its centre when the model has none
					const auto mount = VehicleData::findTag( name, "tag_wheel" );
					const float centre = mount ? mount->origin[2] : 0.0f;
					availableVehicles[i].mins[2] = centre - ( wheel->maxs[2] - wheel->mins[2] ) / 2;
				}
				
			}

		} else if( (availableVehicles[i].cat & CAT_BOAT) ) {
			availableVehicles[i].mins[2] -= 2;
		} else if ( availableVehicles[i].cat & CAT_LQM) {
			VectorSet( availableVehicles[i].mins, -15, -15, -24 );
			VectorSet( availableVehicles[i].maxs, 15, 15, 32 );
			// Scale LQMs
			for(j = 0; j < 3; j++) {
				availableVehicles[i].maxs[j] *= LQM_SCALE;
				availableVehicles[i].mins[j] *= LQM_SCALE;
			}
		}
	}
	for( i = 0; i < bg_numberOfGroundInstallations; ++i ) {
		modelbasename = VehicleData::createGIPathname( i, "models/vehicles/%s/%s/%s" );	
		// boundingbox
		Com_sprintf( name, sizeof(name), "%s.md3", modelbasename );
		if( const auto bounds = VehicleData::getDimensions( name, 0 ) ) {
			VectorCopy( bounds->maxs, availableGroundInstallations[i].maxs );
			VectorCopy( bounds->mins, availableGroundInstallations[i].mins );
		}
		availableGroundInstallations[i].maxs[2] += (15 + 7 * availableGroundInstallations[i].upgrades);
	}
}








/*
=================

Mission script parsing

=================
*/


void VehicleData::parseOverview( char **buf, mission_overview_t* overview ) 
{
	char	*token;
	char	info[MAX_INFO_STRING];

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		return;
	}
//	Common::printf(va("O-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Common::printf( "Missing { in overview\n" );
		return;
	} 

	info[0] = '\0';
	while( true ) 
	{
		token = COM_ParseExt( buf, true );
		if( !token[0] ) 
		{
//				Common::printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Common::printf( "Unexpected { found\n" );
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
	VehicleData::checkMissionScriptOverviewValid(overview, false);// set to true after format change
}


void VehicleData::parseWaypoints( char **buf, mission_vehicle_t* veh ) 
{
	char	*token;
	int		num = 0;

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		return;
	}
//	Common::printf(va("W-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Common::printf( "Missing { in vehicle\n" );
		return;
	} 

	while( true ) 
	{
		token = COM_ParseExt( buf, true );
		if( !token[0] ) 
		{
//				Common::printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Common::printf( "Unexpected { found\n" );
			break;
		}
		if( !strcmp( token, "}" ) ) 
		{
			break;
		}
		if( !strcmp( token, "Origin" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] && num < IGME_MAX_WAYPOINTS )
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

void VehicleData::parseVehicle( char **buf, mission_vehicle_t* veh ) 
{
	char	*token;

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		return;
	}
//	Common::printf(va("V-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Common::printf( "Missing { in vehicle\n" );
		return;
	} 

	while( true ) 
	{
		token = COM_ParseExt( buf, true );
		if( !token[0] ) 
		{
//				Common::printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Common::printf( "Unexpected { found\n" );
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
		else if( !strcmp( token, "Descriptive" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(veh->descriptivename, token, MAX_QPATH );
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
		else if( !strcmp( token, "Side" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(veh->side, token, MAX_NAME_LENGTH );
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
			VehicleData::parseWaypoints(buf, veh);
		}
	}
	veh->used = true;
}

void VehicleData::parseGroundInstallation( char **buf, mission_groundInstallation_t* gi ) 
{
	char	*token;

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		return;
	}
//	Common::printf(va("GV-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Common::printf( "Missing { in groundinstallation\n" );
		return;
	} 

	while( true ) 
	{
		token = COM_ParseExt( buf, true );
		if( !token[0] ) 
		{
//				Common::printf( "Unexpected end of info file\n" );
			break;
		}
		if( !strcmp( token, "{" ) ) 
		{
//				Common::printf( "Unexpected { found\n" );
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
		else if( !strcmp( token, "Descriptive" ) )
		{
			token = COM_ParseExt( buf, false );
			if( token[0] )
			{
				Q_strncpyz(gi->descriptivename, token, MAX_QPATH );
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

void VehicleData::resolveMissionVehicle( mission_vehicle_t* veh )
{
	int i, best = -1;

	if( veh->descriptivename[0] )
	{
		for( i = 0; i < bg_numberOfVehicles; i++ )
		{
			if( availableVehicles[i].descriptiveName && !Q_stricmp( availableVehicles[i].descriptiveName, veh->descriptivename ) )
			{
				veh->index = i;
				return;
			}
		}
	}

	if( veh->objectname[0] )
	{
		for( i = 0; i < bg_numberOfVehicles; i++ )
		{
			if( !availableVehicles[i].modelName || Q_stricmp( availableVehicles[i].modelName, veh->objectname ) )
				continue;
			if( veh->team && !(availableVehicles[i].team & veh->team) )
				continue;
			if( best < 0 || abs( i - veh->index ) < abs( best - veh->index ) )
				best = i;
		}
		if( best >= 0 )
		{
			veh->index = best;
			return;
		}
		Common::printf( S_COLOR_YELLOW "Mission vehicle '%s' not found, using index %d\n", veh->objectname, veh->index );
	}
}

void VehicleData::resolveMissionGroundInstallation( mission_groundInstallation_t* gi )
{
	int i;
	const char *name = gi->descriptivename[0] ? gi->descriptivename : gi->objectname;

	if( !name[0] )
		return;

	for( i = 0; i < bg_numberOfGroundInstallations; i++ )
	{
		if( availableGroundInstallations[i].descriptiveName && !Q_stricmp( availableGroundInstallations[i].descriptiveName, name ) )
		{
			gi->index = i;
			return;
		}
	}
	Common::printf( S_COLOR_YELLOW "Mission ground installation '%s' not found, using index %d\n", name, gi->index );
}

void VehicleData::parseEntities( char **buf, 
						  mission_vehicle_t* vehs, 
						  mission_groundInstallation_t* gis ) 
{
	char	*token;
	int		numVeh = 0,
			numGI = 0;

	token = COM_Parse( buf );
	if( !token[0] ) 
	{
		return;
	};
//	Common::printf(va("E-Token: '%s' - VALID\n", token));

	if( strcmp( token, "{" ) ) 
	{
		Common::printf( "Missing { in entities\n" );
		return;
	} 

	while( true )
	{
		token = COM_Parse( buf );
		if( !token[0] ) 
		{
			Common::printf("Token: '%s' - END\n", token);
			break;
		}
		else if( !strcmp(token, "Vehicle" ) )
		{
			if( numVeh >= IGME_MAX_VEHICLES )
			{
				mission_vehicle_t skipped;
				Common::printf( "Too many vehicles in mission, ignoring the rest\n" );
				VehicleData::parseVehicle(buf, &skipped);
				continue;
			}
			VehicleData::parseVehicle(buf, &vehs[numVeh]);
			VehicleData::resolveMissionVehicle(&vehs[numVeh]);
			++numVeh;
			continue;
		}
		else if( !strcmp(token, "GroundInstallation" ) )
		{
			if( numGI >= IGME_MAX_VEHICLES )
			{
				mission_groundInstallation_t skipped;
				Common::printf( "Too many ground installations in mission, ignoring the rest\n" );
				VehicleData::parseGroundInstallation(buf, &skipped);
				continue;
			}
			VehicleData::parseGroundInstallation(buf, &gis[numGI]);
			VehicleData::resolveMissionGroundInstallation(&gis[numGI]);
			++numGI;
			continue;
		}
		else if( !strcmp( token, "}" ) ) 
		{
			Common::printf("End of Entities\n");
			break;
		}
		else
		{
			Common::printf(S_COLOR_RED "Invalid Token '%s' - parsing cancelled\n", token);
			break;
		}
	}

}

void VehicleData::parseMissionScripts( char *buf,
							 mission_overview_t* overview,
							 mission_vehicle_t* vehs, 
							 mission_groundInstallation_t* gis) 
{
	char *token;

	VehicleData::setMissionScriptOverviewDefaults(overview);

	while( true )
	{
		token = COM_Parse( &buf );
		if( !token[0] ) 
		{
			//Common::printf("Token: '%s' - END\n", token);
			break;
		}
		else if( !strcmp(token, "Overview" ) )
		{
			VehicleData::parseOverview(&buf, overview);
			continue;
		}
		else if( !strcmp(token, "Entities" ) )
		{
			VehicleData::parseEntities(&buf, vehs, gis);
			continue;
		}
		else
		{
			Common::printf(S_COLOR_RED "Invalid Token '%s' - parsing cancelled\n", token);
			break;
		}
	}
}


constexpr int DECK_GRID = 12;

void VehicleData::findDeck( const char *name, completeVehicleData_t *veh )
{
	void	*buf;
	float	cell[DECK_GRID][DECK_GRID];
	bool	used[DECK_GRID][DECK_GRID];
	float	heights[DECK_GRID*DECK_GRID];
	int		num = 0;

	veh->hasDeck = false;
	if( FileSystem::readFile( name, &buf ) <= 0 || !buf ) 
		return;

	md3Header_t *header = static_cast<md3Header_t*>(buf);
	float sx = veh->maxs[0] - veh->mins[0];
	float sy = veh->maxs[1] - veh->mins[1];
	if( header->ident != MD3_IDENT || sx <= 0 || sy <= 0 ) {
		FileSystem::freeFile( buf );
		return;
	}
	memset( used, 0, sizeof(used) );

	md3Surface_t *surf = reinterpret_cast<md3Surface_t*>(reinterpret_cast<byte*>(header) + header->ofsSurfaces);
	for( int i = 0; i < header->numSurfaces; i++ ) {
		md3Triangle_t *tri = reinterpret_cast<md3Triangle_t*>(reinterpret_cast<byte*>(surf) + surf->ofsTriangles);
		md3XyzNormal_t *xyz = reinterpret_cast<md3XyzNormal_t*>(reinterpret_cast<byte*>(surf) + surf->ofsXyzNormals);
		for( int j = 0; j < surf->numTriangles; j++, tri++ ) {
			vec3_t v[3], e1, e2, normal;
			for( int k = 0; k < 3; k++ ) {
				v[k][0] = xyz[tri->indexes[k]].xyz[0] * MD3_XYZ_SCALE;
				v[k][1] = xyz[tri->indexes[k]].xyz[1] * MD3_XYZ_SCALE;
				v[k][2] = xyz[tri->indexes[k]].xyz[2] * MD3_XYZ_SCALE;
			}
			VectorSubtract( v[1], v[0], e1 );
			VectorSubtract( v[2], v[0], e2 );
			CrossProduct( e1, e2, normal );
			if( VectorNormalize( normal ) == 0 || fabs( normal[2] ) < 0.7f ) 
				continue;
			for( int a = 0; a <= 4; a++ ) {
				for( int b = 0; a + b <= 4; b++ ) {
					float wa = a * 0.25f, wb = b * 0.25f, wc = 1.0f - wa - wb;
					float x = v[0][0]*wa + v[1][0]*wb + v[2][0]*wc;
					float y = v[0][1]*wa + v[1][1]*wb + v[2][1]*wc;
					float z = v[0][2]*wa + v[1][2]*wb + v[2][2]*wc;
					int cx = static_cast<int>( (x - veh->mins[0]) / sx * DECK_GRID );
					int cy = static_cast<int>( (y - veh->mins[1]) / sy * DECK_GRID );
					if( cx < 0 || cy < 0 || cx >= DECK_GRID || cy >= DECK_GRID ) 
						continue;
					if( !used[cx][cy] || z > cell[cx][cy] ) {
						cell[cx][cy] = z;
						used[cx][cy] = true;
					}
				}
			}
		}
		surf = reinterpret_cast<md3Surface_t*>(reinterpret_cast<byte*>(surf) + surf->ofsEnd);
	}
	FileSystem::freeFile( buf );

	for( int cx = 1; cx < DECK_GRID - 1; cx++ ) {
		for( int cy = 2; cy < DECK_GRID - 2; cy++ ) {
			if( used[cx][cy] ) 
				heights[num++] = cell[cx][cy];
		}
	}
	if( num < 4 ) 
		return;
	for( int i = 1; i < num; i++ ) {
		float h = heights[i];
		int j = i - 1;
		while( j >= 0 && heights[j] > h ) {
			heights[j+1] = heights[j];
			j--;
		}
		heights[j+1] = h;
	}
	float deck = heights[num * 3 / 10];
	float tolerance = (veh->maxs[2] - veh->mins[2]) * 0.08f;
	float px = 0, py = 0;
	int count = 0;
	for( int cx = 1; cx < DECK_GRID - 1; cx++ ) {
		for( int cy = 2; cy < DECK_GRID - 2; cy++ ) {
			if( used[cx][cy] && fabs( cell[cx][cy] - deck ) <= tolerance ) {
				px += veh->mins[0] + (cx + 0.5f) * sx / DECK_GRID;
				py += veh->mins[1] + (cy + 0.5f) * sy / DECK_GRID;
				count++;
			}
		}
	}
	if( !count ) 
		return;
	VectorSet( veh->deckPos, px / count, py / count, deck );
	veh->hasDeck = true;
}

bool VehicleData::selectable( int vehicle )
{
	if( vehicle < 0 || vehicle >= bg_numberOfVehicles ) 
		return false;
	const unsigned int flags = availableVehicles[vehicle].flags;
	return !(flags & VF_NOTSELECTABLE) && (testVehicles_ || !(flags & VF_TESTONLY));
}

bool VehicleData::stealthHidden( int vehicle, int revealTime, int time, float dist, float radarRange )
{
	if( vehicle < 0 || vehicle >= bg_numberOfVehicles ) 
		return false;
	if( !(availableVehicles[vehicle].flags & VF_STEALTH) ) 
		return false;
	if( time < revealTime ) 
		return false;
	// a stealth bomber is revealed only by its open bay, and shut it is the hardest target of all
	const float factor = availableVehicles[vehicle].cls == CLASS_PLANE_BOMBER ? STEALTH_RANGE_FACTOR * 0.5f : STEALTH_RANGE_FACTOR;
	return dist > radarRange * factor;
}

bool VehicleData::vehicleHasTurret( int vehicle )
{
	if( vehicle < 0 || vehicle >= bg_numberOfVehicles ) 
		return false;
	for(unsigned int weapon : availableVehicles[vehicle].weapons) 
	{
		if( weapon && availableWeapons[weapon].turret ) 
			return true;
	}
	return false;
}
