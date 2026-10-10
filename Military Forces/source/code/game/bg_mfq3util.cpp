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


// says so when a vehicle's pylons took fewer of a store than its table entry asks for (developer output)
static void reportShortLoad( int vehicle, int slot, int wanted )
{
	const int got = availableVehicles[vehicle].ammo[slot];
	if( got < wanted )
		Common::dPrintf( "Loadout short: %s carries %d of %d %s\n", availableVehicles[vehicle].descriptiveName, got, wanted,
			availableWeapons[availableVehicles[vehicle].weapons[slot]].descriptiveName );
}

void VehicleData::loadAllVehicleData()
{
	int i, j, num;
	char* modelbasename = nullptr;
	char name[128];

	VehicleData::registerVehicles();

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
		{
			VehicleData::findDeck( name, &availableVehicles[i] );
			char clipName[128];
			Com_sprintf( clipName, sizeof(clipName), "%s_clip.md3", modelbasename );
			VehicleData::findClipBoxes( clipName, &availableVehicles[i] );
		}
		// boats, transport helicopters and wheeled vehicles carry a crew; attack helicopters and tanks take nobody aboard
		if( availableVehicles[i].cat & CAT_HELO ) 
			availableVehicles[i].seats = (availableVehicles[i].cls & CLASS_HELO_ATTACK) ? 0 : 4;
		else if( availableVehicles[i].cat & CAT_BOAT ) 
			availableVehicles[i].seats = 4;
		else if( availableVehicles[i].cat & CAT_GROUND ) 
			availableVehicles[i].seats = (availableVehicles[i].caps & HC_WHEELS) ? 3 : 0;
		else 
			// a bomber with turrets carries a gunner
			availableVehicles[i].seats = (availableVehicles[i].flags & VF_GUNSTATIONS) ? 1 : 0;

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
					const int wanted = availableVehicles[i].ammo[j];
					availableVehicles[i].ammo[j] = 
						VehicleData::packagesOfType( availableVehicles[i].weapons[j], &availableLoadouts[i] );
					reportShortLoad( i, j, wanted );
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
						const int wanted = availableVehicles[i].ammo[j];
						availableVehicles[i].ammo[j] = 
							VehicleData::packagesOfType( availableVehicles[i].weapons[j], &availableLoadouts[i] );
						reportShortLoad( i, j, wanted );
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
			// A wheeled vehicle stands on the wheels its model has: where the body mounts each one, and how big the
			// wheel is. Its underside is the bottom of its wheels
			if(availableVehicles[i].wheels)
			{
				completeVehicleData_t& veh = availableVehicles[i];
				veh.wheels = std::min( veh.wheels, 6 );
				veh.wheelRadius = veh.wheelCF > 0 ? veh.wheelCF / ( 2.0f * static_cast<float>( M_PI ) ) : 1.5f;
				Com_sprintf( name, sizeof(name), "%s_w1.md3", modelbasename );
				if( const auto wheel = VehicleData::getDimensions( name, 0 ) )
					veh.wheelRadius = ( wheel->maxs[2] - wheel->mins[2] ) / 2;
				char body[128];
				Com_sprintf( body, sizeof(body), "%s.md3", modelbasename );
				int found = 0;
				float lowest = 0;
				for( int w = 0; w < veh.wheels; w++ )
				{
					char tagName[16];
					if( w ) Com_sprintf( tagName, sizeof(tagName), "tag_wheel%d", w + 1 );
					else Q_strncpyz( tagName, "tag_wheel", sizeof(tagName) );
					const auto mount = VehicleData::findTag( body, tagName );
					if( !mount )
						break;
					VectorCopy( mount->origin, veh.wheelPos[w] );
					lowest = found ? std::min( lowest, mount->origin[2] ) : mount->origin[2];
					found++;
				}
				// a model without wheel mounts stands on the corners of its box
				if( found < veh.wheels )
					veh.wheelRadius = 0;
				else
					veh.mins[2] = lowest - veh.wheelRadius;
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

// A clip model gives a vehicle its solid shape: every surface in it is one box, its bounds. Without one, a
// vehicle with a deck gets a row of boxes under the deck height along its length.
void VehicleData::findClipBoxes( const char *clipModel, completeVehicleData_t *veh )
{
	veh->numClipBoxes = 0;
	if( OpenFile( clipModel, FS_READ ) )
	{
		void *buf;
		if( FileSystem::readFile( clipModel, &buf ) > 0 && buf )
		{
			const md3Header_t *header = static_cast<md3Header_t*>(buf);
			if( header->ident == MD3_IDENT )
			{
				const md3Surface_t *surf = reinterpret_cast<const md3Surface_t*>(reinterpret_cast<const byte*>(header) + header->ofsSurfaces);
				for( int i = 0; i < header->numSurfaces && veh->numClipBoxes < MAX_CLIP_BOXES; i++ )
				{
					const md3XyzNormal_t *xyz = reinterpret_cast<const md3XyzNormal_t*>(reinterpret_cast<const byte*>(surf) + surf->ofsXyzNormals);
					vec3_t lo = { 99999, 99999, 99999 }, hi = { -99999, -99999, -99999 };
					for( int v = 0; v < surf->numVerts; v++ )
					{
						for( int k = 0; k < 3; k++ )
						{
							const float c = xyz[v].xyz[k] * MD3_XYZ_SCALE;
							lo[k] = std::min( lo[k], c );
							hi[k] = std::max( hi[k], c );
						}
					}
					ClipBox& box = veh->clipBoxes[veh->numClipBoxes++];
					for( int k = 0; k < 3; k++ )
					{
						box.centre[k] = ( lo[k] + hi[k] ) * 0.5f;
						box.half[k] = std::max( 0.5f, ( hi[k] - lo[k] ) * 0.5f );
					}
					surf = reinterpret_cast<const md3Surface_t*>(reinterpret_cast<const byte*>(surf) + surf->ofsEnd);
				}
			}
			FileSystem::freeFile( buf );
		}
	}
	if( veh->numClipBoxes )
	{
		// where crew leaving the vehicle stand: on top of the first box, at the spot nearest its middle that is
		// clear of the other boxes by a soldier's width
		constexpr float clearance = 3.0f;
		const ClipBox& floor = veh->clipBoxes[0];
		float best = -1.0f;
		for( float along = -floor.half[0] + clearance; along <= floor.half[0] - clearance; along += 1.0f )
		{
			const float x = floor.centre[0] + along;
			bool clear = true;
			for( int i = 1; i < veh->numClipBoxes; i++ )
			{
				const ClipBox& other = veh->clipBoxes[i];
				if( fabs( x - other.centre[0] ) < other.half[0] + clearance && fabs( floor.centre[1] - other.centre[1] ) < other.half[1] + clearance )
					clear = false;
			}
			if( clear && ( best < 0.0f || fabs( along ) < best ) )
			{
				best = fabs( along );
				VectorSet( veh->deckPos, x, floor.centre[1], floor.centre[2] + floor.half[2] );
			}
		}
		veh->hasDeck = true;
		return;
	}
	if( !veh->hasDeck )
		return;

	// no clip model: square boxes from the keel up to the deck, side by side along the hull
	const float halfLength = ( veh->maxs[0] - veh->mins[0] ) * 0.5f;
	const float centre = ( veh->maxs[0] + veh->mins[0] ) * 0.5f;
	float half = ( veh->maxs[1] - veh->mins[1] ) * 0.45f;
	half = std::min( half, 120.0f );
	half = std::min( half, halfLength );
	const int count = std::min( MAX_CLIP_BOXES, static_cast<int>( ceil( halfLength / half ) ) );
	for( int i = 0; i < count; i++ )
	{
		ClipBox& box = veh->clipBoxes[veh->numClipBoxes++];
		VectorSet( box.centre, count > 1 ? centre - (halfLength - half) + i * 2 * (halfLength - half) / (count - 1) : centre, 0,
			( veh->mins[2] + veh->deckPos[2] ) * 0.5f );
		VectorSet( box.half, half, half, ( veh->deckPos[2] - veh->mins[2] ) * 0.5f );
	}
}

// The machine gun has its own trigger and is never what the selector points at, unless it is all the vehicle has.
// Returns the first main weapon slot, or the machine gun's when there is none.
int VehicleData::firstMainWeapon( int vehicle )
{
	if( vehicle >= 0 && vehicle < bg_numberOfVehicles )
	{
		for( int slot = WP_WEAPON1; slot <= WP_WEAPON6; slot++ )
		{
			if( availableVehicles[vehicle].weapons[slot] != WI_NONE )
				return slot;
		}
	}
	return WP_MACHINEGUN;
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

namespace
{
	// the B-17's four twin turrets, from the gun tags of its model: chin, top, ball and tail
	constexpr GunStation b17Stations[] = {
		// Fields like the real turrets, which is what gives the bomber its weak sides. The chin turret covers a cone
		// ahead, tilted a little down. The top and ball turrets cover their hemispheres but stop short of level. The
		// tail guns cover a wide cone astern, wider than the real mount's, so that a fighter sitting behind is under
		// fire. Level with the bomber on the beam nothing bears: the real aircraft had only its hand-held waist
		// guns there, and this model has none
		{ 33.4f, 0.0f, -3.7f,   0.98f, 0.0f, -0.17f,  62.0f, 1.0f, 45.0f },
		{  7.5f, 0.0f,  9.4f,   0.0f,  0.0f,  1.0f,   87.0f, 1.0f, 45.0f },
		{ -18.0f, 0.0f, -6.3f,  0.0f,  0.0f, -1.0f,   87.0f, 1.7f, 45.0f },
		{ -65.3f, 0.0f,  3.1f, -1.0f,  0.0f,  0.0f,   55.0f, 0.5f, 30.0f },
	};
}

std::span<const GunStation> VehicleData::gunStations( int vehicle )
{
	if( vehicle < 0 || vehicle >= bg_numberOfVehicles || !(availableVehicles[vehicle].flags & VF_GUNSTATIONS) )
		return {};
	return b17Stations;
}

// bit n is set when station n can aim along aimAngles with the hull at hullAngles
unsigned VehicleData::gunStationsBearing( int vehicle, const vec3_t hullAngles, const vec3_t aimAngles )
{
	vec3_t hull[3], aim;
	unsigned mask = 0, bit = 1;
	AnglesToAxis( hullAngles, hull );
	AngleVectors( aimAngles, aim, nullptr, nullptr );
	for( const GunStation& station : gunStations( vehicle ) )
	{
		// the hull's second axis points left
		vec3_t axis;
		for( int i = 0; i < 3; i++ )
			axis[i] = hull[0][i] * station.axisForward - hull[1][i] * station.axisRight + hull[2][i] * station.axisUp;
		if( DotProduct( axis, aim ) >= cos( DEG2RAD( station.arc ) ) * VectorLength( axis ) )
			mask |= bit;
		bit <<= 1;
	}
	return mask;
}

// Where position n's guns point, as angles in the hull's own space: along the aim when it lies in the position's
// field, otherwise as near to it as the field allows
void VehicleData::gunStationAngles( int vehicle, int index, const vec3_t hullAngles, const vec3_t aimAngles, vec3_t out )
{
	VectorClear( out );
	const std::span<const GunStation> stations = gunStations( vehicle );
	if( index < 0 || index >= static_cast<int>( stations.size() ) )
		return;
	const GunStation& station = stations[index];
	vec3_t hull[3], world, aim, axis, dir;
	AnglesToAxis( hullAngles, hull );
	AngleVectors( aimAngles, world, nullptr, nullptr );
	// the hull's second axis points left, and so does the second coordinate here
	VectorSet( aim, DotProduct( world, hull[0] ), DotProduct( world, hull[1] ), DotProduct( world, hull[2] ) );
	VectorSet( axis, station.axisForward, -station.axisRight, station.axisUp );
	VectorNormalize( axis );
	const float along = DotProduct( axis, aim );
	const float edge = DEG2RAD( station.arc );
	if( along >= cos( edge ) )
		VectorCopy( aim, dir );
	else
	{
		// turn from the middle of the field towards the aim, as far as the edge
		vec3_t across;
		VectorMA( aim, -along, axis, across );
		if( VectorNormalize( across ) < 0.001f )
		{
			// aimed straight away from the field: any side will do
			VectorSet( across, -axis[2], 0, axis[0] );
			if( VectorNormalize( across ) < 0.001f ) VectorSet( across, 1, 0, 0 );
		}
		for( int i = 0; i < 3; i++ )
			dir[i] = axis[i] * cos( edge ) + across[i] * sin( edge );
	}
	vectoangles( dir, out );
	out[ROLL] = 0;
}

// Swings every gun position towards the aim by what its traverse allows in the time given; pitch and yaw hold each
// position's direction in the hull's own space. Bit n of the result is set while position n has not come round yet
unsigned VehicleData::gunStationsTrack( int vehicle, const vec3_t hullAngles, const vec3_t aimAngles, float seconds, float* pitch, float* yaw )
{
	// close enough to the aim to fire along it
	constexpr float onAim = 4.0f;
	unsigned lagging = 0;
	const std::span<const GunStation> stations = gunStations( vehicle );
	for( int n = 0; n < static_cast<int>( stations.size() ) && n < MAX_VEHICLE_TURRETS; n++ )
	{
		vec3_t wanted;
		gunStationAngles( vehicle, n, hullAngles, aimAngles, wanted );
		const float step = stations[n].traverse * seconds;
		const float turn = AngleSubtract( wanted[YAW], yaw[n] ), lift = AngleSubtract( wanted[PITCH], pitch[n] );
		yaw[n] = AngleNormalize360( yaw[n] + std::clamp( turn, -step, step ) );
		pitch[n] = AngleNormalize180( pitch[n] + std::clamp( lift, -step, step ) );
		const vec3_t now = { pitch[n], yaw[n], 0 };
		vec3_t pointing, target;
		AngleVectors( now, pointing, nullptr, nullptr );
		AngleVectors( wanted, target, nullptr, nullptr );
		if( DotProduct( pointing, target ) < cos( DEG2RAD( onAim ) ) )
			lagging |= 1u << n;
	}
	return lagging;
}

bool VehicleData::vehicleHasTurret( int vehicle )
{
	if( vehicle < 0 || vehicle >= bg_numberOfVehicles ) 
		return false;
	if( availableVehicles[vehicle].flags & VF_GUNSTATIONS )
		return true;
	for(unsigned int weapon : availableVehicles[vehicle].weapons) 
	{
		if( weapon && availableWeapons[weapon].turret ) 
			return true;
	}
	return false;
}
