#include "../cg_local.h"
#include <algorithm>

constexpr int MAX_TEAM_SKIN_MODELS = 2048;

static qhandle_t cg_teamSkins[2][MAX_TEAM_SKIN_MODELS];

qhandle_t CGame::registerVehiclePart( const char *name, bool expected )
{
	fileHandle_t f;

	if( FileSystem::fOpenFileByMode( name, &f, FS_READ ) < 0 ) {
		if( expected )
			CGame::printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
		return 0;
	}
	FileSystem::fCloseFile( f );
	qhandle_t model = refExport.RegisterModel( name );
	if( model > 0 && model < MAX_TEAM_SKIN_MODELS ) {
		char skin[128];
		char base[128];
		Q_strncpyz( base, name, sizeof(base) );
		if( strlen( base ) > 4 ) base[strlen( base ) - 4] = 0;
		for( int t = 0; t < 2; ++t ) {
			Com_sprintf( skin, sizeof(skin), "%s_team%d.skin", base, t + 1 );
			cg_teamSkins[t][model] = 0;
			if( const OpenFile file( skin, FS_READ ); file ) {
				f = file.handle();
				cg_teamSkins[t][model] = refExport.RegisterSkin( skin );
			}
		}
	}
	return model;
}

int CGame::apparentTeam( int clientNum, int viewerTeam )
{
	const clientInfo_t& ci = cgs.clientinfo[clientNum];
	if( static_cast<int>( ci.team ) == viewerTeam )
		return viewerTeam;
	return ci.paintTeam && !ci.paintBlown ? ci.paintTeam : static_cast<int>( ci.team );
}

qhandle_t CGame::teamSkin( qhandle_t model, int team )
{
	if( team < 1 || team > 2 || model <= 0 || model >= MAX_TEAM_SKIN_MODELS ) 
		return 0;
	return cg_teamSkins[team - 1][model];
}

/*
===============
CGame::initShadows

Initialise all the shadow parameters for each vehicle
===============
*/

void CGame::initShadows( )
{
	qhandle_t customShadowShader = -1;
	float xRad = 0, yRad = 0;
	int vehicle = 0;

	// cache the vehicle's custom shadow shader (if we can find one - format is <modelName>Shadow, "like f-16Shadow")
	for( vehicle = 0; vehicle < bg_numberOfVehicles; vehicle++ )
	{
		// try to load custom shader; a soldier is several models and has no shadow picture of its own
		customShadowShader = (availableVehicles[ vehicle ].cat & CAT_LQM) ? 0 :
			refExport.RegisterShader( va( "%sShadow", availableVehicles[ vehicle ].modelName ) );
		
		// replace SHADOW_x fallback op. with the custom shader handle?
		if( customShadowShader )
		{
			availableVehicles[ vehicle ].shadowShader = customShadowShader;
		}

		// setup default shadow size adjusters if none set
		if( availableVehicles[ vehicle ].shadowCoords[ SHC_XADJUST ] == 0 &&
			availableVehicles[ vehicle ].shadowCoords[ SHC_YADJUST ] == 0 )
		{
			// create x & y adjusters from the vehicle's bound box
			xRad = availableVehicles[ vehicle ].maxs[ 0 ];
			yRad = availableVehicles[ vehicle ].maxs[ 1 ];
			xRad -= availableVehicles[ vehicle ].mins[ 0 ];
			yRad -= availableVehicles[ vehicle ].mins[ 1 ];
			xRad *= 0.5f;
			yRad *= 0.5f;
			availableVehicles[ vehicle ].shadowCoords[ SHC_XADJUST ] = xRad;
			availableVehicles[ vehicle ].shadowCoords[ SHC_YADJUST ] = yRad;
		}

		// setup default shadow orientation modifiers/adjusters if none set
		// NOTE: will be set for all vehicles, but currently only used on flying ones
		
		// maximums
		if( availableVehicles[ vehicle ].shadowAdjusts[ SHO_PITCHMAX ] == 0 &&
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_ROLLMAX ] == 0 )
		{
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_PITCHMAX ] = 90.0f;
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_ROLLMAX ] = 90.0f;
		}

		// modifiers
		if( availableVehicles[ vehicle ].shadowAdjusts[ SHO_PITCHMOD ] == 0 &&
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_ROLLMOD ] == 0 )
		{
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_PITCHMOD ] = 0.8f;
			availableVehicles[ vehicle ].shadowAdjusts[ SHO_ROLLMOD ] = 0.8f;
		}
	}
}

/*
===============
CGame::cacheVehicles
===============
*/

void CGame::cacheVehicles()
{
	int i;
	int total = bg_numberOfVehicles + bg_numberOfGroundInstallations;
	
	for( i = 0; i < bg_numberOfVehicles; i++ )
	{
		CGame::loadingStep( i, total );
		if( availableVehicles[i].gameset & cgs.gameset ) {
			CGame::loadingString(availableVehicles[i].descriptiveName);
			if( availableVehicles[i].cat & CAT_PLANE ) {
					CGame::cachePlane(i);
			}
			else if( availableVehicles[i].cat & CAT_GROUND ) {
					CGame::cacheGroundVehicle(i);
			}
			else if( availableVehicles[i].cat & CAT_HELO ) {
					CGame::cacheHelo(i);
			}
			else if( availableVehicles[i].cat & CAT_LQM ) {
					CGame::cacheLQM(i);
			}
			else if( availableVehicles[i].cat & CAT_BOAT ) {
					CGame::cacheBoat(i);
			}
			else {
				Common::error( ERR_DROP, "Invalid Vehicle type in CG_CacheVehicles (%d,%d)", 
					i, availableVehicles[i].cat );
			}
		}
	}
	for( i = 0; i < bg_numberOfGroundInstallations; i++ ) {
		CGame::loadingStep( bg_numberOfVehicles + i, total );
		if( availableGroundInstallations[i].gameset & cgs.gameset ) {
			CGame::loadingString(availableGroundInstallations[i].descriptiveName);
			CGame::cacheGroundInstallation(i);
		}
	}

}

/*
===============
CGame::registerVehicle

load the vehicle
===============
*/
void CGame::registerVehicle( clientInfo_t *ci )
{
//	Common::printf( "RegVehicle = %i\n" ,ci->vehicle);    
	if( availableVehicles[ci->vehicle].cat & CAT_PLANE ) {
		CGame::registerPlane(ci);
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_GROUND ) {
		CGame::registerGroundVehicle(ci);
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_HELO ) {
		CGame::registerHelo(ci);
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_LQM ) {
		CGame::registerLQM(ci);
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_BOAT ) {
		CGame::registerBoat(ci);
	}
	else
		Common::error( ERR_DROP, "Invalid vehicle type in register vehicle" );
}

int ClientEntity::loadoutVehicle()
{
	if( this->currentState.eType == ET_VEHICLE ) 
	{
		int clientNum = this->currentState.clientNum;
		if( clientNum < 0 || clientNum >= MAX_CLIENTS ) 
			return -1;
		return cgs.clientinfo[clientNum].vehicle;
	}
	if( this->currentState.eType == ET_MISC_VEHICLE && this->currentState.modelindex < 255 ) 
		return this->currentState.modelindex;
	return -1;
}

completeLoadout_t* ClientEntity::entityLoadout()
{
	int idx = this->loadoutVehicle();
	if( idx < 0 || idx >= bg_numberOfVehicles ) 
		return nullptr;

	int n = this->currentState.number;
	int key = ((idx + 1) << 1) | ((this->currentState.eFlags & EF_TELEPORT_BIT) ? 1 : 0);
	if( cg_loadoutKeys[n] != key ) 
	{
		VehicleData::getDefaultLoadoutForVehicle( idx, &cg_loadouts[n], nullptr );
		cg_loadoutKeys[n] = key;
	}
	return &cg_loadouts[n];
}

void ClientEntity::setDrawLoadout( BasicDrawInfo_t* info )
{
	info->usedLoadout = this->entityLoadout();
	if( !info->usedLoadout ) 
		return;

	bool local = this->currentState.eType == ET_VEHICLE && this->currentState.clientNum == cg.snap->ps.clientNum;
	for( int i = 0; i < info->usedLoadout->usedMounts; ++i ) 
		info->numWeaponsOnMount[i] = local ? cg.snap->ps.numWeaponsOnMount[i] :
			( (this->currentState.mountMask & (1 << i)) ? info->usedLoadout->mounts[i].num : 0 );
}

/*
===============
CG_Vehicle
===============
*/
void ClientEntity::vehicle() 
{
    clientInfo_t * ci;
    int	clientNum;

    // get client num and info
    clientNum = this->currentState.clientNum;
    if ( clientNum < 0 || clientNum >= MAX_CLIENTS )
	{
		Common::error( ERR_DROP, "Bad clientNum on player entity");
    }
    ci = &cgs.clientinfo[ clientNum ];
	//Common::printf( "Vehicle = %i\n" ,ci->vehicle);    

	// don't continue if client info is invalid
    if ( !ci->infoValid )
	{
		return;
    }

	// don't draw ourselves, if we are rendering the MFDs
	if( this == &cg.predictedPlayerEntity && cg.drawingMFD )
	{
		return;
	}

	if( ci->vehicle < 0 || ci->vehicle >= bg_numberOfVehicles )
	{
		return;
	}

	// plane?
	if( availableVehicles[ci->vehicle].cat & CAT_PLANE )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR))
		{
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->plane( ci );
	}
	// ground vehicle?
	else if( availableVehicles[ci->vehicle].cat & CAT_GROUND )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND))
		{
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->groundVehicle( ci );
	}
	// helo?
	else if( availableVehicles[ci->vehicle].cat & CAT_HELO )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_AIR))
		{
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->helo( ci );
	}
	// lqm?
	else if( availableVehicles[ci->vehicle].cat & CAT_LQM)
	{
			this->lQM( ci );
	}
	// boat ?
	else if( availableVehicles[ci->vehicle].cat & CAT_BOAT )
	{
		if( cg.radarTargets < MAX_RADAR_TARGETS && (cg.predictedPlayerEntity.currentState.ONOFF & OO_RADAR_GROUND))
		{
			cg.radarEnts[cg.radarTargets++] = this;
		}
		this->boat( ci );
	}
	else
	{
		Common::error( ERR_DROP, "Error: CG_Vehicle got wrong CAT!\n" );
	}
}

/*
=============
CGame::vehicleObituary
=============
*/
void CGame::vehicleObituary( entityState_t *ent ) 
{
    clientInfo_t	*ci;
    int		        clientNum = ent->otherEntityNum;

    if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		Common::error( ERR_DROP, "CG_VehicleObituary: target out of range" );
    }
    ci = &cgs.clientinfo[clientNum];

    if ( !ci->infoValid ) {
		CGame::printf( "No obituary available!\n" );
		return;
    }

	if( availableVehicles[ci->vehicle].cat & CAT_PLANE ) {
		CGame::planeObituary( ent, ci );
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_GROUND ) {
		CGame::groundVehicleObituary( ent, ci );
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_HELO ) {
		CGame::heloObituary( ent, ci );
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_LQM ) {
		CGame::lQMObituary( ent, ci );
	}
	else if( availableVehicles[ci->vehicle].cat & CAT_BOAT ) {
		CGame::boatObituary( ent, ci );
	}
	else {
		CGame::planeObituary( ent, ci );
	}

}

/*
=============
CGame::vehicleMuzzleFlash

=============
*/
void CGame::vehicleMuzzleFlash( int weaponIdx, const refEntity_t *parent, qhandle_t parentModel, int idx ) {

	refEntity_t		flash[MAX_MUZZLEFLASHES];
	Vec3			angles;
	unsigned int	i,j;
	unsigned int	flashes;
	char			tag[10];

	flashes = availableWeapons[weaponIdx].barrels;

	if( flashes > MAX_MUZZLEFLASHES ) flashes = MAX_MUZZLEFLASHES;

	for( i = 0; i < flashes; i++ ) {
		memset( &flash[i], 0, sizeof( flash[i] ) );
		VectorCopy( parent->lightingOrigin, flash[i].lightingOrigin );
		flash[i].shadowPlane = parent->shadowPlane;
		flash[i].renderfx = parent->renderfx;
		flash[i].hModel = cgs.media.vehicleMuzzleFlashModel;

		angles[YAW] = 0;
		angles[PITCH] = 0;
		angles[ROLL] = crandom() * 10;
		AnglesToAxis( angles, flash[i].axis );
		if(availableVehicles[idx].cat & CAT_LQM) {
			for(j = 0; j < 3; j++) 
				VectorScale(flash[i].axis[j], 1/(float)LQM_SCALE, flash[i].axis[j]);
		}

		if( i > 0 ) {
			Com_sprintf( tag, 10, "tag_gun%d", i+1 );
		} else {
			strcpy( tag, "tag_gun" );
		}
		CGame::positionRotatedEntityOnTag( &flash[i], parent, parentModel, tag);
		refExport.AddRefEntityToScene( &flash[i] );
	}
}

/*
=============
CGame::planePilot
=============
*/
int CGame::cg_drawTeam;

void CGame::addVehiclePart( refEntity_t *re )
{
	if( cg_drawTeam && !re->customSkin && !re->customShader ) 
		re->customSkin = CGame::teamSkin( re->hModel, cg_drawTeam );
	refExport.AddRefEntityToScene( re );
}
