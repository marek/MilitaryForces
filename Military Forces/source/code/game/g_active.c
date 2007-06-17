/*
 * $Id: g_active.c,v 1.11 2007-06-17 17:11:12 minkis Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

#include "g_local.h"
#include "g_level.h"
#include "g_plane.h"
#include "g_helo.h"

/*
===============
G_DamageFeedback

Called just before a snapshot is sent to the given player.
Totals up all damage and generates both the player_state_t
damage values to that client for pain blends and kicks, and
global pain sound events for all clients.
===============
*/
void P_DamageFeedback( GameEntity *player ) 
{
	GameClient* client = player->client_;
	if( client->ps_.pm_type == PM_DEAD ) 
		return;

	// total points of damage shot at the player this frame
	float count = client->damage_done_;
	if ( count == 0 ) 
		return;		// didn't take any damage

	if ( count > 255 ) 
		count = 255;

	// send the information to the client

	// world damage (falling, slime, etc) uses a special code
	// to make the blend blob centered instead of positional
	if( client->damage_fromWorld_ ) 
	{
		client->ps_.damagePitch = 255;
		client->ps_.damageYaw = 255;

		client->damage_fromWorld_ = false;
	} 
	else 
	{
		vec3_t	angles;
		vectoangles( client->damage_from_, angles );
		client->ps_.damagePitch = angles[PITCH]/360.0 * 256;
		client->ps_.damageYaw = angles[YAW]/360.0 * 256;
	}

	// play an apropriate pain sound
	if( (theLevel.time_ > player->pain_debounce_time_) && !(player->flags_ & FL_GODMODE) )
	{
		player->pain_debounce_time_ = theLevel.time_ + 700;

#pragma message( "Possible future <client> event loss from calling G_AddEvent()!" )
/* MFQ3: don't create this event because (1) we don't need it and (2) it masks out our EV_VEHICLE_GIB event
         approximatley 15% of the time (for some reason, possibly that the previous G_AddEvent( EV_VEHICLE_GIB ) has not
		 yet been processed and we therefore overwrite that event with this one)

		G_AddEvent( player, EV_PAIN, player->health );
		client->ps.damageEvent++;
*/
	}

	client->ps_.damageCount = count;

	//
	// clear totals
	//
	client->damage_done_ = 0;
	client->damage_knockback_ = 0;
}



/*
=============
P_WorldEffects

Check for lava / slime contents and drowning
=============
*/
void P_WorldEffects( GameEntity *ent ) 
{
	int			waterlevel;

	if( ent->client_->noclip_ ) 
	{
		ent->client_->airOutTime_ = theLevel.time_ + 20000;	// don't need air
		return;
	}

	waterlevel = ent->waterlevel_;

	//
	// check for drowning
	//
	if( waterlevel == 3 ) 
	{
		// if out of air, start drowning
		if( ent->client_->airOutTime_ < theLevel.time_)
		{
			// drown!
			ent->client_->airOutTime_ += 3000;
			if( ent->health_ > 0 ) 
			{
				// take more damage the longer underwater
				ent->damage_ += 5;
				if( ent->damage_ > 8 )
					ent->damage_ = 8;

				// play a gurp sound instead of a normal pain sound
/*				if (ent->health <= ent->damage) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("*drown.wav"));
				} else if (rand()&1) {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp1.wav"));
				} else {
					G_Sound(ent, CHAN_VOICE, G_SoundIndex("sound/player/gurp2.wav"));
				}
*/
				// don't play a normal pain sound
//				ent->pain_debounce_time = level.time + 200;

				G_Damage (ent, NULL, NULL, NULL, NULL, 
					ent->damage_, DAMAGE_NO_PROTECTION, MOD_WATER, CAT_ANY);
			}
		}
	} 
	else 
	{
		ent->client_->airOutTime_ = theLevel.time_ + 3000;
		ent->damage_ = 5;
	}

	//
	// check for sizzle damage (move to pmove?)
	//
	if( waterlevel && 
		(ent->watertype_&(CONTENTS_LAVA|CONTENTS_SLIME)) ) 
	{
		if( ent->health_ > 0
			&& ent->pain_debounce_time_ <= theLevel.time_	) 
		{
			if( ent->watertype_ & CONTENTS_LAVA) 
			{
				G_Damage (ent, NULL, NULL, NULL, NULL, 
					30*waterlevel, 0, MOD_LAVA, CAT_ANY);
			}
			if( ent->watertype_ & CONTENTS_SLIME) 
			{
				G_Damage (ent, NULL, NULL, NULL, NULL, 
					10*waterlevel, 0, MOD_SLIME, CAT_ANY);
			}
		}
	}
}


///*
//===============
//G_SetClientSound
//===============
//*/
//void G_SetClientSound( gentity_t *ent ) {
//	if (ent->waterlevel && (ent->watertype&(CONTENTS_LAVA|CONTENTS_SLIME)) ) {
//		ent->client->ps.loopSound = theLevel.snd_fry;
//	} else {
//		ent->client->ps.loopSound = 0;
//	}
//}



//==============================================================

/*
==============
ClientImpacts
==============
*/
void ClientImpacts( GameEntity *ent, pmove_t *pm )
{
	trace_t	trace;
	memset( &trace, 0, sizeof( trace ) );

	for( int i=0 ; i<pm->numtouch ; i++ ) 
	{
		int j;
		for( j=0 ; j<i ; j++ ) 
		{
			if( pm->touchents[j] == pm->touchents[i] ) 
				break;
		}
		if( j != i ) 
			continue;	// duplicated

		GameEntity* other = theLevel.getEntity(pm->touchents[i]);

		if( (( ent->r.svFlags & SVF_BOT || (ent->client_->vehicle_ >= 0) ) && ( ent->touchFunc_ )) ) 
			ent->touchFunc_->execute( other, &trace );

		if( !other->touchFunc_ ) 
			continue;

		other->touchFunc_->execute( ent, &trace );
	}

}

/*
============
G_TouchTriggers

Find all trigger entities that ent's current position touches.
Spectators will only interact with teleporters.
============
*/
void	G_TouchTriggers( GameEntity *ent ) 
{
	if( !ent->client_ ) 
		return;

	// dead clients don't activate triggers!
	if( ent->client_->ps_.stats[STAT_HEALTH] <= 0 ) 
		return;

	vec3_t		mins, maxs;
	static vec3_t	range = { 40, 40, 52 };
	VectorSubtract( ent->client_->ps_.origin, range, mins );
	VectorAdd( ent->client_->ps_.origin, range, maxs );

	int	touch[MAX_GENTITIES];
	int num = SV_AreaEntities( mins, maxs, touch, MAX_GENTITIES );

	// can't use ent->absmin, because that has a one unit pad
	VectorAdd( ent->client_->ps_.origin, ent->r.mins, mins );
	VectorAdd( ent->client_->ps_.origin, ent->r.maxs, maxs );

	for( int i=0 ; i<num ; i++ ) 
	{
		GameEntity* hit = theLevel.getEntity(touch[i]);

		if( !hit || !hit->touchFunc_ && !ent->touchFunc_ ) 
			continue;

		if( !( hit->r.contents & CONTENTS_TRIGGER ) ) 
			continue;

		// use seperate code for determining if an item is picked up
		// so you don't have to actually contact its bounding box
		if( hit->s.eType == ET_ITEM )
		{
			if( !BG_PlayerTouchesItem( &ent->client_->ps_, &hit->s, theLevel.time_ ) ) 
				continue;
		} 
		else 
		{
			if( !SV_EntityContact( mins, maxs, hit, false ) ) 
				continue;
		}

		trace_t		trace;
		memset( &trace, 0, sizeof(trace) );

		if( hit->touchFunc_ ) 
			hit->touchFunc_->execute( ent, &trace );

		if( ( ent->r.svFlags & SVF_BOT ) && ( ent->touchFunc_ ) ) 
			ent->touchFunc_->execute( hit, &trace );
	}

}

/*
=================
NotSelectedThink
=================
*/
// MFQ3 for the time when no vehicle is selected
void NotSelectedThink( GameEntity *ent, usercmd_t *ucmd ) 
{
	pmove_t	pm;
	GameClient	*client;
	
	client = ent->client_;

	client->ps_.pm_type = PM_FREEZE;
	client->ps_.speed = 0;
	
	// set up for pmove
	memset( &pm, 0, sizeof(pm) );
	pm.ps = &client->ps_;
	pm.cmd = *ucmd;
	pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
	pm.trace = SV_Trace;
	pm.pointcontents = SV_PointContents;

	// perform a pmove
	Pmove (&pm);
	// save results of pmove
//	VectorCopy( client->ps.origin, ent->s.origin );

	G_TouchTriggers( ent );
	SV_UnlinkEntity( ent );

	client->oldbuttons_ = client->buttons_;
	client->buttons_ = ucmd->buttons;

}




/*
=================
SpectatorThink
=================
*/
void SpectatorThink( GameEntity *ent, usercmd_t *ucmd ) 
{
	pmove_t	pm;
	GameClient	*client;

	client = ent->client_;

	if( client->sess_.spectatorState_ != GameClient::ClientSession::SPECTATOR_FOLLOW ) 
	{
		client->ps_.pm_type = PM_SPECTATOR;
		client->ps_.speed = g_spectSpeed.integer;	// faster than normal

		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		pm.ps = &client->ps_;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = SV_Trace;
		pm.pointcontents = SV_PointContents;

		// perform a pmove
		pm.vehicle = -1;
		Pmove (&pm);
		// save results of pmove
		VectorCopy( client->ps_.origin, ent->s.origin );

		G_TouchTriggers( ent );
		SV_UnlinkEntity( ent );
	}

	client->oldbuttons_ = client->buttons_;
	client->buttons_ = ucmd->buttons;

	// attack button cycles through spectators
	if( ( client->buttons_ & BUTTON_ATTACK ) && ! ( client->oldbuttons_ & BUTTON_ATTACK ) ) {
		Cmd_FollowCycle_f( ent, 1 );
	}
}


/*
=================
MissionEditorThink
=================
*/
void MissionEditorThink( GameEntity *ent, usercmd_t *ucmd ) 
{
	pmove_t	pm;
	GameClient	*client;

	client = ent->client_;

	if( client->sess_.spectatorState_ != GameClient::ClientSession::SPECTATOR_FOLLOW ) 
	{
		client->ps_.pm_type = PM_SPECTATOR;
		client->ps_.speed = g_spectSpeed.integer;	// faster than normal
		// set up for pmove
		memset(&pm, 0, sizeof(pm));
		pm.ps = &client->ps_;
		pm.cmd = *ucmd;
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;	// spectators can fly through bodies
		pm.trace = SV_Trace;
		pm.pointcontents = SV_PointContents;

		// perform a pmove
		pm.vehicle = -1;
		Pmove (&pm);
		// save results of pmove
		VectorCopy( client->ps_.origin, ent->s.origin );

		G_TouchTriggers( ent );
		SV_UnlinkEntity( ent );
	}

	client->oldbuttons_ = client->buttons_;
	client->buttons_ = ucmd->buttons;
}

/*
=================
ClientInactivityTimer

Returns false if the client is dropped
=================
*/
bool ClientInactivityTimer( GameClient *client ) 
{
	if( !g_inactivity.integer ) 
	{
		// give everyone some time, so if the operator sets g_inactivity during
		// gameplay, everyone isn't kicked
		client->inactivityTime_ = theLevel.time_ + 60 * 1000;
		client->inactivityWarning_ = false;
	} 
	else if( client->pers_.cmd_.forwardmove || 
			  client->pers_.cmd_.rightmove || 
			  client->pers_.cmd_.upmove ||
			  (client->pers_.cmd_.buttons & BUTTON_ATTACK) ) 
	{
		client->inactivityTime_ = theLevel.time_ + g_inactivity.integer * 1000;
		client->inactivityWarning_ = false;
	} 
	else if( !client->pers_.localClient_ ) 
	{
		if( theLevel.time_ > client->inactivityTime_ )
		{
			SV_GameDropClient( client->ps_.clientNum,// client - theLevel.clients_, 
							   "Dropped due to inactivity" );
			return false;
		}
		if( theLevel.time_ > client->inactivityTime_ - 10000 && !client->inactivityWarning_ ) 
		{
			client->inactivityWarning_ = true;
			SV_GameSendServerCommand( client->ps_.clientNum,//client - level.clients, 
									  "cp \"Ten seconds until inactivity drop!\n\"" );
		}
	}
	return true;
}

/*
==================
ClientTimerActions

Actions that happen once a second
==================
*/
void ClientTimerActions( GameEntity *ent, int msec ) 
{
	GameClient	*client = ent->client_;
	client->timeResidual_ += msec;

	while( client->timeResidual_ >= 1000 ) 
	{
		client->timeResidual_ -= 1000;

		// count down health when over max
		if( ent->health_ > client->ps_.stats[STAT_MAX_HEALTH] ) {
			ent->health_--;
		}
	}
}

/*
====================
ClientIntermissionThink
====================
*/
void ClientIntermissionThink( GameClient *client ) 
{
	// the level will exit when everyone wants to or after timeouts

	// swap and latch button actions
	client->oldbuttons_ = client->buttons_;
	client->buttons_ = client->pers_.cmd_.buttons;
	if( client->buttons_ & ( BUTTON_ATTACK|BUTTON_ATTACK_MAIN ) & ( client->oldbuttons_ ^ client->buttons_ ) )
	{ 
		// this used to be an ^1 but once a player says ready, it should stick
		client->readyToExit_ = 1;
	}
}


/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
void ClientEvents( GameEntity *ent, int oldEventSequence ) 
{
	GameClient *client = ent->client_;

	if( oldEventSequence < client->ps_.eventSequence - MAX_PS_EVENTS ) 
		oldEventSequence = client->ps_.eventSequence - MAX_PS_EVENTS;

	for( int i = oldEventSequence ; i < client->ps_.eventSequence ; i++ ) 
	{
		int event = client->ps_.events[ i & (MAX_PS_EVENTS-1) ];

		switch ( event ) 
		{
		case EV_FIRE_MG:
			fire_autocannon( ent, false );
			break;

		case EV_FIRE_WEAPON:
			FireWeapon( ent );
			break;

		case EV_FIRE_FLARE:
				if(ent->client_->ps_.weaponIndex == WI_CFLARE || availableVehicles[ent->client_->vehicle_].weapons[7] == WI_CFLARE)
					fire_cflare( ent );
				else
					fire_flare( ent );
			break;

		default:
			break;
		}
	}

}

///*
//==============
//StuckInOtherClient
//==============
//*/
//
//static int StuckInOtherClient(gentity_t *ent) {
//	int i;
//	gentity_t	*ent2;
//
//	ent2 = &g_entities[0];
//	for ( i = 1; i <= MAX_CLIENTS; i++, ent2++ ) {
//		if ( ent2 == ent ) {
//			continue;
//		}
//		if ( !ent2->inuse ) {
//			continue;
//		}
//		if ( !ent2->client ) {
//			continue;
//		}
//		if ( ent2->health <= 0 ) {
//			continue;
//		}
//		//
//		if (ent2->r.absmin[0] > ent->r.absmax[0])
//			continue;
//		if (ent2->r.absmin[1] > ent->r.absmax[1])
//			continue;
//		if (ent2->r.absmin[2] > ent->r.absmax[2])
//			continue;
//		if (ent2->r.absmax[0] < ent->r.absmin[0])
//			continue;
//		if (ent2->r.absmax[1] < ent->r.absmin[1])
//			continue;
//		if (ent2->r.absmax[2] < ent->r.absmin[2])
//			continue;
//		return true;
//	}
//	return false;
//}


/*
==============
SendPendingPredictableEvents
==============
*/
void SendPendingPredictableEvents( playerState_t *ps )
{
	// if there are still events pending
	if( ps->entityEventSequence < ps->eventSequence )
	{
		// create a temporary entity for this event which is sent to everyone
		// except the client who generated the event
		int seq = ps->entityEventSequence & (MAX_PS_EVENTS-1);
		int event = ps->events[ seq ] | ( ( ps->entityEventSequence & 3 ) << 8 );

		// set external event to zero before calling BG_PlayerStateToEntityState
		int extEvent = ps->externalEvent;
		ps->externalEvent = 0;

		// create temporary entity for event
		GameEntity* t = G_TempEntity( ps->origin, event );
		int number = t->s.number;
		BG_PlayerStateToEntityState( ps, &t->s, true );
		t->s.number = number;
		t->s.eType = ET_EVENTS + event;
		t->s.eFlags |= EF_PLAYER_EVENT;
		t->s.otherEntityNum = ps->clientNum;

		// send to everyone except the client who generated the event
		t->r.svFlags |= SVF_NOTSINGLECLIENT;
		t->r.singleClient = ps->clientNum;

		// set back external event
		ps->externalEvent = extEvent;
	}
}

/*
==============
ClientThink

This will be called once for each client frame, which will
usually be a couple times for each server frame on fast clients.

If "g_synchronousClients 1" is set, this will be called exactly
once for each server frame, which makes for smooth demo recording.
==============
*/
void ClientThink_real( GameEntity *ent ) 
{
	GameClient	*client = ent->client_;

	// event queue
	ent->eventSent_ = false;
	G_SendEventFromQueue( ent );

	// don't think if the client is not yet connected (and thus not yet spawned in)
	if( client->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
		return;

	// mark the time, so the connection sprite can be removed
	usercmd_t	*ucmd = &ent->client_->pers_.cmd_;

	// sanity check the command time to prevent speedup cheating
	if( ucmd->serverTime > theLevel.time_ + 200 ) 
	{
		ucmd->serverTime = theLevel.time_ + 200;
//		Com_Printf("serverTime <<<<<\n" );
	}
	if( ucmd->serverTime < theLevel.time_ - 1000 ) 
	{
		ucmd->serverTime = theLevel.time_ - 1000;
//		Com_Printf("serverTime >>>>>\n" );
	} 

	int msec = ucmd->serverTime - client->ps_.commandTime;
	// following others may result in bad times, but we still want
	// to check for follow toggles
	if( msec < 1 && client->sess_.spectatorState_ != GameClient::ClientSession::SPECTATOR_FOLLOW ) 
		return;

	if( msec > 200 ) 
		msec = 200;

	if( pmove_msec.integer < 8 ) 
		Cvar_Set("pmove_msec", "8");
	else if( pmove_msec.integer > 33 ) 
		Cvar_Set("pmove_msec", "33");

	if( pmove_fixed.integer || client->pers_.pmoveFixed_ ) 
	{
		ucmd->serverTime = ((ucmd->serverTime + pmove_msec.integer-1) / pmove_msec.integer) * pmove_msec.integer;
		//if (ucmd->serverTime - client->ps.commandTime <= 0)
		//	return;
	}

	//
	// check for exiting intermission
	//
	if( theLevel.intermissiontime_ ) 
	{
		ClientIntermissionThink( client );
		return;
	}

	// spectators don't do much
	if( client->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		if( client->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_SCOREBOARD ) 
			return;
		if( g_gametype.integer == GT_MISSION_EDITOR ) 
			MissionEditorThink( ent, ucmd );
		else 
			SpectatorThink( ent, ucmd );
		return;
	}

	// MFQ3 vehicle not selected
	if( client->ps_.pm_flags & PMF_VEHICLESELECT ) 
	{
		if( theLevel.time_ > client->respawnTime_ && client->nextVehicle_ >= 0 ) 
		{
			// forcerespawn is to prevent users from waiting out powerups
			if( g_forcerespawn.integer > 0 && ( theLevel.time_ - client->respawnTime_ ) > g_forcerespawn.integer * 1000 ) 
			{
				client->ps_.pm_flags &= ~PMF_VEHICLESELECT;
				client->ps_.pm_flags |= PMF_VEHICLESPAWN;
			}
		
			// pressing attack or use is the normal respawn method
			if( ucmd->buttons & ( BUTTON_ATTACK|BUTTON_ATTACK_MAIN ) ) 
			{
				client->ps_.pm_flags &= ~PMF_VEHICLESELECT;
				client->ps_.pm_flags |= PMF_VEHICLESPAWN;
			}
		}
		else 
		{
			NotSelectedThink( ent, ucmd );
			return;
		}
	}

	// MFQ3 just selected vehicle
	if( (client->ps_.pm_flags & PMF_VEHICLESPAWN))
	{
		client->ps_.pm_flags &= ~PMF_VEHICLESPAWN;

		if( client->ps_.pm_flags & PMF_RECHARGING )
			switch_vehicle( ent );
		else
			respawn( ent );

		return;
	}

	// MFQ3 finish selection
	if( !(ent->r.svFlags & SVF_BOT) && client->vehicle_ < 0 )
	{
		NotSelectedThink( ent, ucmd );
		return;
	}

	// check for inactivity timer, but never drop the local client of a non-dedicated server
	if( !ClientInactivityTimer( client ) ) 
		return;

	if( client->noclip_ ) 
	{
		client->ps_.pm_type = PM_NOCLIP;
	}
	else if( client->ps_.stats[STAT_HEALTH] <= 0 )
	{
		if( client->vehicle_ < 0 || 
			client->ps_.stats[STAT_HEALTH] <= GIB_HEALTH ) 
			client->ps_.pm_type = PM_DEAD;
		else 
			client->ps_.pm_type = PM_VEHICLE;
	} 
	else if( client->vehicle_ >= 0 ) 
		client->ps_.pm_type = PM_VEHICLE;
	else 
		client->ps_.pm_type = PM_FREEZE;

	// first the general functionality, then the cat-specific one
	if( client->ps_.weaponNum != ucmd->weapon ) 
	{
		unlock(ent);
//		untrack(ent);
	} 
	else 
	{
		updateTargetTracking(ent);
	}
	// category specific actions if any
	if( availableVehicles[client->vehicle_].cat & CAT_PLANE)
		reinterpret_cast<Entity_Plane*>(ent)->checkForTakeoffOrLanding();
	else if( availableVehicles[client->vehicle_].cat & CAT_HELO) 
		reinterpret_cast<Entity_Helicopter*>(ent)->checkForTakeoffOrLanding();

	// don't forget to update flags!
	ent->ONOFF_ = ent->client_->ps_.ONOFF;

	// set up for pmove
	int oldEventSequence = client->ps_.eventSequence;

	pmove_t		pm;
	memset(&pm, 0, sizeof(pm));

	pm.ps = &client->ps_;
	pm.cmd = *ucmd;
	if( pm.ps->pm_type == PM_DEAD ) 
		pm.tracemask = MASK_PLAYERSOLID & ~CONTENTS_BODY;
	else if( ent->r.svFlags & SVF_BOT ) 
		pm.tracemask = MASK_PLAYERSOLID | CONTENTS_BOTCLIP;
	else 
		pm.tracemask = MASK_PLAYERSOLID;

	pm.trace = SV_Trace;
	pm.pointcontents = SV_PointContents;
	pm.debugLevel = g_debugMove.integer;

	pm.pmove_fixed = (pmove_fixed.integer!=0) | client->pers_.pmoveFixed_;
	pm.pmove_msec = pmove_msec.integer;

	VectorCopy( client->ps_.origin, client->oldOrigin_ );

	// setup vehicle for pmove
	pm.advancedControls = client->advancedControls_;
	pm.vehicle = client->vehicle_;
	pm.updateGear = ent->updateGear_;
	pm.updateBay = ent->updateBay_;
	VectorCopy( ent->s.angles, pm.ps->vehicleAngles );
	pm.ps->speed = client->ps_.speed;
	pm.ps->gunAngle = client->ps_.gunAngle;
	pm.ps->turretAngle = client->ps_.turretAngle;

	if( ent->health_ < 0 ) 
		pm.ps->generic1 = 30;
	else if( ent->health_ >= 0 && ent->health_ < 30 ) 
		pm.ps->generic1 = 30 - ent->health_;
	else 
		pm.ps->generic1 = 0;

	if( ent->rechargetime_ > theLevel.time_ ) 
		pm.ps->pm_flags |= PMF_RECHARGING;
	else 
		pm.ps->pm_flags &= ~PMF_RECHARGING;

	Pmove (&pm);

	// save pmove results for vehicle
	VectorCopy( pm.ps->vehicleAngles, ent->s.angles );
	client->ps_.speed = pm.ps->speed;
	client->ps_.gunAngle = pm.ps->gunAngle;
	client->ps_.turretAngle = pm.ps->turretAngle;
	ent->updateGear_ = pm.updateGear;
	ent->updateBay_ = pm.updateBay;

	// for GV's check for crashland
	if( availableVehicles[client->vehicle_].cat & CAT_GROUND )
	{
		if( client->ps_.ONOFF & OO_VAPOR ) 
		{
			client->ps_.ONOFF &= ~OO_VAPOR;
			G_Damage( ent, 0, ent, 0, 0, client->ps_.speed/5, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
		}
	}

	// save results of pmove
	if( ent->client_->ps_.eventSequence != oldEventSequence ) 
		ent->eventTime_ = theLevel.time_;

	if( g_smoothClients.integer ) 
		BG_PlayerStateToEntityStateExtraPolate( &ent->client_->ps_, &ent->s, ent->client_->ps_.commandTime, true );
	else
		BG_PlayerStateToEntityState( &ent->client_->ps_, &ent->s, true );

	SendPendingPredictableEvents( &ent->client_->ps_ );

	// use the snapped origin for linking so it matches client predicted versions
	VectorCopy( ent->s.pos.trBase, ent->r.currentOrigin );

	VectorCopy( pm.mins, ent->r.mins );
	VectorCopy( pm.maxs, ent->r.maxs );

	ent->waterlevel_ = pm.waterlevel;
	ent->watertype_ = pm.watertype;

	if( client->vehicle_ >= 0 && ent->waterlevel_ && 
		!(availableVehicles[client->vehicle_].cat & CAT_GROUND) &&
		!(availableVehicles[client->vehicle_].cat & CAT_BOAT) )
	{
	    G_Damage( ent, NULL, ent, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_CRASH, CAT_ANY );
	}

	// execute client events
	ClientEvents( ent, oldEventSequence );

	// link entity now, after any personal teleporters have been used
	SV_LinkEntity( ent );
	if( !ent->client_->noclip_ ) 
		G_TouchTriggers( ent );

	// NOTE: now copy the exact origin over otherwise clients can be snapped into solid
	VectorCopy( ent->client_->ps_.origin, ent->r.currentOrigin );

	// touch other objects
	ClientImpacts( ent, &pm );

	// save results of triggers and client events
	if( ent->client_->ps_.eventSequence != oldEventSequence ) 
		ent->eventTime_ = theLevel.time_;

	// swap and latch button actions
	client->oldbuttons_ = client->buttons_;
	client->buttons_ = ucmd->buttons;
	client->latched_buttons_ |= client->buttons_ & ~client->oldbuttons_;

	// check for respawning
	if( client->vehicle_ >= 0 && client->ps_.stats[STAT_HEALTH] <= GIB_HEALTH ) 
	{
		// wait for the attack button to be pressed
		if( theLevel.time_ > client->respawnTime_ ) 
		{
			// forcerespawn is to prevent users from waiting out powerups
			if( g_forcerespawn.integer > 0 && 
				( theLevel.time_ - client->respawnTime_ ) > g_forcerespawn.integer * 1000 )
			{
				respawn( ent );
				return;
			}
		
			// pressing attack or use is the normal respawn method
			if ( ucmd->buttons & ( BUTTON_ATTACK|BUTTON_ATTACK_MAIN  ) ) 
				respawn( ent );
		}
		return;
	}

	// perform once-a-second actions
	ClientTimerActions( ent, msec );
}

/*
==================
ClientThink

A New command has arrived from the client
==================
*/
void ClientThink( int clientNum ) 
{
	GameEntity *ent = theLevel.getEntity(clientNum);
	SV_GetUsercmd( clientNum, &ent->client_->pers_.cmd_ );

	// mark the time we got info, so we can display the
	// phone jack if they don't get any for a while
	ent->client_->lastCmdTime_ = theLevel.time_;

	if ( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) 
		ClientThink_real( ent );
}

/*
==================
G_RunClient
==================
*/
void G_RunClient( GameEntity *ent ) 
{
	if( !(ent->r.svFlags & SVF_BOT) && !g_synchronousClients.integer ) 
		return;

	ent->client_->pers_.cmd_.serverTime = theLevel.time_;
	ClientThink_real( ent );
}


/*
==================
SpectatorClientEndFrame

==================
*/
void SpectatorClientEndFrame( GameEntity *ent ) 
{
	// if we are doing a chase cam or a remote view, grab the latest info
	if( ent->client_->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_FOLLOW ) 
	{
		int clientNum = ent->client_->sess_.spectatorClient_;

		// team follow1 and team follow2 go to whatever clients are playing
		if( clientNum == -1 ) 
			clientNum = theLevel.follow1_;
		else if( clientNum == -2 ) 
			clientNum = theLevel.follow2_;

		if( clientNum >= 0 ) 
		{
			GameClient* cl = theLevel.getClient(clientNum);
			if( cl && cl->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED && 
				cl->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR ) 
			{
				int flags = (cl->ps_.eFlags & ~(EF_VOTED | EF_TEAMVOTED)) | (ent->client_->ps_.eFlags & (EF_VOTED | EF_TEAMVOTED));
				ent->client_->ps_ = cl->ps_;
				ent->client_->ps_.pm_flags |= PMF_FOLLOW;
				ent->client_->ps_.eFlags = flags;
				return;
			} 
			else
			{
				// drop them to free spectators unless they are dedicated camera followers
				if( ent->client_->sess_.spectatorClient_ >= 0 ) 
				{
					ent->client_->sess_.spectatorState_ = GameClient::ClientSession::SPECTATOR_FREE;
					MF_ClientBegin( ent->client_->ps_.clientNum );//ent->client_ - theLevel.clients_ );
				}
			}
		}
	}

	if( ent->client_->sess_.spectatorState_ == GameClient::ClientSession::SPECTATOR_SCOREBOARD ) 
		ent->client_->ps_.pm_flags |= PMF_SCOREBOARD;
	else
		ent->client_->ps_.pm_flags &= ~PMF_SCOREBOARD;
}

/*
==============
ClientEndFrame

Called at the end of each server frame for each connected client
A fast client will have multiple ClientThink for each ClientEdFrame,
while a slow client may have multiple ClientEndFrame between ClientThink.
==============
*/
void ClientEndFrame( GameEntity *ent ) 
{
	if( ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR ) 
	{
		SpectatorClientEndFrame( ent );
		return;
	}

	//
	// If the end of unit layout is displayed, don't give
	// the player any normal movement attributes
	//
	if( theLevel.intermissiontime_ ) 
		return;

	// burn from lava, etc
	P_WorldEffects (ent);

	// apply all the damage taken this frame
	P_DamageFeedback (ent);

	// add the EF_CONNECTION flag if we haven't gotten commands recently
	if( theLevel.time_ - ent->client_->lastCmdTime_ > 1000 ) 
		ent->s.eFlags |= EF_CONNECTION;
	else
		ent->s.eFlags &= ~EF_CONNECTION;

	ent->client_->ps_.stats[STAT_HEALTH] = ent->health_;	// FIXME: get rid of ent->health...

	//G_SetClientSound (ent);

	// set the latest infor
	if( g_smoothClients.integer ) 
		BG_PlayerStateToEntityStateExtraPolate( &ent->client_->ps_, &ent->s, ent->client_->ps_.commandTime, true );
	else
		BG_PlayerStateToEntityState( &ent->client_->ps_, &ent->s, true );

	SendPendingPredictableEvents( &ent->client_->ps_ );

	// MFQ3 reset radio targets
	ent->radio_target_ = 0;
}


