/*
 * $Id: cg_players.c,v 1.7 2006-01-29 14:03:40 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"

char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
	"*death1.wav",
	"*death2.wav",
	"*death3.wav",
	"*jump1.wav",
	"*pain25_1.wav",
	"*pain50_1.wav",
	"*pain75_1.wav",
	"*pain100_1.wav",
	"*falling1.wav",
	"*gasp.wav",
	"*drown.wav",
	"*fall1.wav",
	"*taunt.wav"
};


/*
================
CGame::customSound

================
*/
sfxHandle_t	CGame::customSound( int clientNum, const char *soundName ) {
	clientInfo_t *ci;
	int			i;

	if ( soundName[0] != '*' ) {
		return SoundSystem::registerSound( soundName, false );
	}

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i] ; i++ ) {
		if ( !strcmp( soundName, cg_customSoundNames[i] ) ) {
			return ci->sounds[i];
		}
	}

	Common::error( ERR_DROP, "Unknown custom sound: %s", soundName );
	return 0;
}



/*
====================
CGame::colorFromString
====================
*/
void CGame::colorFromString( const char *v, vec3_t color ) {
	int val;

	VectorClear( color );

	val = atoi( v );

	if ( val < 1 || val > 7 ) {
		VectorSet( color, 1, 1, 1 );
		return;
	}

	if ( val & 1 ) {
		color[2] = 1.0f;
	}
	if ( val & 2 ) {
		color[1] = 1.0f;
	}
	if ( val & 4 ) {
		color[0] = 1.0f;
	}
}

/*
======================
CGame::newClientInfo
======================
*/
void CGame::newClientInfo( int clientNum ) {
	clientInfo_t *ci;
	clientInfo_t newInfo;
	const char	*configstring;
	const char	*v;

	ci = &cgs.clientinfo[clientNum];

	configstring = CGame::configString( clientNum + CS_PLAYERS );
	if ( !configstring[0] ) {
		memset( ci, 0, sizeof( *ci ) );
		return;		// player just left
	}

	// build into a temp buffer so the defer checks can use
	// the old value
	newInfo = {};

	// isolate the player's name
	v = Info::valueForKey(configstring, "n");
	Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

	// colors
	v = Info::valueForKey( configstring, "c1" );
	CGame::colorFromString( v, newInfo.color );

	// bot skill
	v = Info::valueForKey( configstring, "skill" );
	newInfo.botSkill = atoi( v );

	// handicap
	v = Info::valueForKey( configstring, "hc" );
	newInfo.handicap = atoi( v );

	// wins
	v = Info::valueForKey( configstring, "w" );
	newInfo.wins = atoi( v );

	// losses
	v = Info::valueForKey( configstring, "l" );
	newInfo.losses = atoi( v );

	// team
	v = Info::valueForKey( configstring, "t" );
	newInfo.team = static_cast<ClientBase::eTeam>(atoi( v ));
	if(cg.clientNum == clientNum && ci->team != newInfo.team ) // use cg.predictedPlayerEntity.currentState.clientNum instead?
	{
		// MFQ3: team changed so reset the vehicle trigger
		// Make sure it only happens to the player changing teams, otherwise all vehicle's reset 
		// (UserInfoChanged on g will reset all players)
		CvarSystem::set( "cg_vehicle", "-1" );
		// the vehicle already chosen is kept when the new side may use it: picking a vehicle and then a side, or
		// changing sides, does not mean picking the vehicle again
		const int chosen = static_cast<int>( CGame::cvar_Get( "cg_nextVehicle" ) );
		const int sideFlag = newInfo.team == ClientBase::TEAM_RED ? MF_TEAM_1 : newInfo.team == ClientBase::TEAM_BLUE ? MF_TEAM_2 : 0;
		if( chosen < 0 || chosen >= bg_numberOfVehicles || !(availableVehicles[chosen].team & sideFlag) )
			CvarSystem::set( "cg_nextVehicle", va( "%d", -1 ) );
	}

	// team task
	v = Info::valueForKey( configstring, "tt" );
	newInfo.teamTask = atoi(v);

	// team leader
	v = Info::valueForKey( configstring, "tl" );
	newInfo.teamLeader = atoi(v);

	v = Info::valueForKey( configstring, "g_redteam" );
	Q_strncpyz(newInfo.redTeam, v, MAX_TEAMNAME);

	v = Info::valueForKey( configstring, "g_blueteam" );
	Q_strncpyz(newInfo.blueTeam, v, MAX_TEAMNAME);

	// MFQ3: controls
	v = Info::valueForKey( configstring, "ac" );
	newInfo.advancedControls = atoi(v);
	if( newInfo.advancedControls )
		CommandBuffer::addText( "+strafe\n" );
//		CvarSystem::set( "cl_freelook", "0" );
	else
		CommandBuffer::addText( "-strafe\n" );
//		CvarSystem::set( "cl_freelook", "1" );

	// MFQ3: vehicle
	v = Info::valueForKey( configstring, "pt" );
	newInfo.paintTeam = atoi( v );
	v = Info::valueForKey( configstring, "pb" );
	newInfo.paintBlown = atoi( v ) != 0;

	v = Info::valueForKey( configstring, "v" );
	newInfo.vehicle = atoi( v );
	if( newInfo.vehicle >= 0 && clientNum == cg.predictedPlayerEntity.currentState.clientNum )
	{
		CvarSystem::set( "cg_vehicle", va( "%d", newInfo.vehicle ) );
		// only set "cg_nextvehicle" to the same as "cg_vehicle" when we currently don't have
		// a vehicle index in it.
		if( CGame::cvar_Get( "cg_nextVehicle" ) == -1 )
		{
			CvarSystem::set( "cg_nextVehicle", va( "%d", newInfo.vehicle ) );
		}
	}

	// makes sure vehicle is registered
	if( newInfo.vehicle >= 0 )
	{
	    CGame::registerVehicle(&newInfo);
	    newInfo.infoValid = true;
	    *ci = newInfo;

//		Common::printf( "New info received, it is a vehicle (%s)\n", newInfo.name );
	    return;
	}

	// replace whatever was there with the New one
	newInfo.infoValid = true;
	*ci = newInfo;
}

/*
===============
CG_TrailItem
===============
*/
void ClientEntity::trailItem( qhandle_t hModel ) {
	refEntity_t		ent;
	Vec3			angles;
	vec3_t			axis[3];

	angles = this->lerpAngles;
	angles[PITCH] = 0;
	angles[ROLL] = 0;
	AnglesToAxis( angles, axis );

	ent = {};
	VectorMA( this->lerpOrigin, -16, axis[0], ent.origin );
	ent.origin[2] += 16;
	angles[YAW] += 90;
	AnglesToAxis( angles, ent.axis );

	ent.hModel = hModel;
	refExport.AddRefEntityToScene( &ent );
}






//=====================================================================


