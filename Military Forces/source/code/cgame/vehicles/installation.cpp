#include "../cg_local.h"
#include <algorithm>

/*
===============
CGame::cacheGroundInstallation

On startup cache it
===============
*/

void CGame::cacheGroundInstallation(int index)
{
	char name[128];
	char basename[128];
	int i;

	Com_sprintf( basename, sizeof(basename), "models/vehicles/npc/%s/%s", 
				 availableGroundInstallations[index].modelName,
				 availableGroundInstallations[index].modelName );
// changed mg
	for( i = 0; i < BP_GI_MAX_PARTS; i++ ) {
		switch(i) {
		case BP_GI_BODY:
			Com_sprintf( name, sizeof(name), "%s.md3", basename );
			break;
		case BP_GI_TURRET:
			Com_sprintf( name, sizeof(name), "%s_tur.md3", basename );
			break;
		case BP_GI_GUNBARREL:
			Com_sprintf( name, sizeof(name), "%s_gun.md3", basename );
			break;
		case BP_GI_UPGRADE:
		case BP_GI_UPGRADE2:
		case BP_GI_UPGRADE3:
			Com_sprintf( name, sizeof(name), "%s_upg.md3", basename );
			break;
		}
		availableGroundInstallations[index].handle[i] = CGame::registerVehiclePart( name, false );
//		if( !availableVehicles[index].handle[i] ) {
//			CGame::printf( "MFQ3 Warning: Unable to load model '%s'\n", name );
//		}
	}

	// only thing that always has to be there is body
	if( !availableGroundInstallations[index].handle[BP_GV_BODY] ) {
	}
}

char *gi_tags[BP_GI_MAX_PARTS] =
{
	"<no tag>",		// vehicle body
	"tag_turret",	// turret
	"tag_weap",		// gun
	"tag_upgrade",	
	"tag_upgrade",	
	"tag_upgrade"	
};

void CGame::drawGI(DrawInfo_GI_t* drawInfo)
{
	groundInstallationData_t* veh = &availableGroundInstallations[drawInfo->basicInfo.vehicleIndex];
	refEntity_t part[BP_GI_MAX_PARTS];
	int i;
	for( i = 0; i < BP_GI_MAX_PARTS; i++ ) 
	{
		memset( &part[i], 0, sizeof(part[0]) );	
	}	
	// body
	part[BP_GI_BODY].hModel = veh->handle[BP_GI_BODY];		
	VectorCopy( drawInfo->basicInfo.origin, part[BP_GI_BODY].origin );	
	VectorCopy( drawInfo->basicInfo.origin, part[BP_GI_BODY].oldorigin);
	AxisCopy( drawInfo->basicInfo.axis, part[BP_GI_BODY].axis );
	CGame::addVehiclePart( &part[BP_GI_BODY] );

	// other parts
	for( i = 1; i < BP_GI_MAX_PARTS; i++ ) 
	{
		if( (i == BP_GI_UPGRADE && !veh->upgrades) ||
			(i == BP_GI_UPGRADE2 && veh->upgrades < 2 ) ||
			(i == BP_GI_UPGRADE3 && veh->upgrades < 3 ) ) 
		{
			break;;
		}
		part[i].hModel = veh->handle[i];
		if( !part[i].hModel ) continue;
		VectorCopy( drawInfo->basicInfo.origin, part[i].lightingOrigin );
		AxisCopy( axisDefault, part[i].axis );
		if( i == BP_GI_GUNBARREL ) 
		{
			RotateAroundPitch( part[i].axis, drawInfo->gunAngle );
			CGame::positionRotatedEntityOnTag( &part[i], &part[BP_GI_TURRET], 
				veh->handle[BP_GI_TURRET], gi_tags[i] );
		} 
		else if( i == BP_GI_UPGRADE ) 
		{
			CGame::positionRotatedEntityOnTag( &part[i], &part[BP_GI_GUNBARREL], 
				veh->handle[BP_GI_GUNBARREL], gi_tags[i] );
		} 
		else if( i == BP_GI_UPGRADE2 ) 
		{
			CGame::positionRotatedEntityOnTag( &part[i], &part[BP_GI_UPGRADE], 
				veh->handle[BP_GI_UPGRADE], gi_tags[i] );
		} 
		else if( i == BP_GI_UPGRADE3 ) 
		{
			CGame::positionRotatedEntityOnTag( &part[i], &part[BP_GI_UPGRADE2], 
				veh->handle[BP_GI_UPGRADE2], gi_tags[i] );
		} 
		else 
		{
			RotateAroundYaw( part[i].axis, drawInfo->turretAngle );
			CGame::positionRotatedEntityOnTag( &part[i], &part[BP_GI_BODY], 
				veh->handle[BP_GI_BODY], gi_tags[i] );
		}
		CGame::addVehiclePart( &part[i] );
	}
}
