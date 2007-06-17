/*
 * $Id: g_items.c,v 1.7 2007-06-17 17:11:13 minkis Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
#include "g_local.h"
#include <algorithm>
#include "g_entity.h"
#include "g_level.h"
#include "g_item.h"

/*

  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  all items should pop when dropped in lava or slime

  Respawnable items don't actually go away when picked up, they are
  just made invisible and untouchable.  This allows them to ride
  movers and respawn apropriately.
*/


#define	RESPAWN_HEALTH		35
#define	RESPAWN_AMMO		40
#define	RESPAWN_MEGAHEALTH	35//120
#define	RESPAWN_POWERUP		120
#define	RESPAWN_FUEL		35

//======================================================================

int Pickup_Ammo( GameEntity *ent, GameEntity *other )
{
	int		quantity, i;

	if( ent->count_ ) 
		quantity = ent->count_;
	else 
		quantity = ent->item_->quantity;

	for( i = WP_MACHINEGUN; i <= WP_FLARE; i++ )
	{
		if( availableWeapons[availableVehicles[other->client_->vehicle_].weapons[i]].type == ent->item_->giTag )
		{
			other->client_->ps_.ammo[i] += quantity;
			if( other->client_->ps_.ammo[i] > other->client_->ps_.ammo[i+8] ) 
				other->client_->ps_.ammo[i] = other->client_->ps_.ammo[i+8];
			break;
		}
	}

	return RESPAWN_AMMO;
}

//======================================================================

int Pickup_Health( GameEntity *ent, GameEntity *other )
{
	int			max;
	int			quantity;

	// small and mega healths will go over the max
	if( ent->item_->quantity != 5 && ent->item_->quantity != 100 ) 
		max = other->client_->ps_.stats[STAT_MAX_HEALTH];
	else 
		max = other->client_->ps_.stats[STAT_MAX_HEALTH] * 2;

	if( ent->count_ ) 
		quantity = ent->count_;
	else 
		quantity = ent->item_->quantity;

	other->health_ += (other->client_->ps_.stats[STAT_MAX_HEALTH] * quantity / 100);

	if( other->health_ > max ) 
		other->health_ = max;

	other->client_->ps_.stats[STAT_HEALTH] = other->health_;

	if( ent->item_->quantity == 100 ) 		// mega health respawns slow
		return RESPAWN_MEGAHEALTH;

	return RESPAWN_HEALTH;
}


//======================================================================

int Pickup_Fuel( GameEntity *ent, GameEntity *other ) 
{
	int			max;
	int			quantity;

	max = other->client_->ps_.stats[STAT_MAX_FUEL] ;

	if( ent->count_ )
		quantity = ent->count_;
	else 
		quantity = ent->item_->quantity;

	other->client_->ps_.stats[STAT_FUEL] = std::min(other->client_->ps_.stats[STAT_FUEL]+quantity,max);

	return RESPAWN_FUEL;
}


//======================================================================

/*
===============
RespawnItem
===============
*/
void RespawnItem( GameEntity *ent ) 
{
	// randomly select from teamed entities
	if( ent->team_ ) 
	{
		GameEntity	*master;
		int	count;
		int choice;

		if( !ent->teammaster_ ) 
			Com_Error( ERR_DROP, "RespawnItem: bad teammaster");

		master = ent->teammaster_;

		for( count = 0, ent = master; ent; ent = ent->teamchain_, count++ )
			;

		choice = rand() % count;

		for( count = 0, ent = master; count < choice; ent = ent->teamchain_, count++ )
			;
	}

	ent->r.contents = CONTENTS_TRIGGER;
	ent->s.eFlags &= ~EF_NODRAW;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	SV_LinkEntity(ent);

	// play the normal respawn sound only to nearby clients
	G_AddEvent( ent, EV_ITEM_RESPAWN, 0, true );

	ent->nextthink_ = 0;
}


struct Think_RespawnItem : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		RespawnItem(self_);
	}
};

/*
===============
Touch_Item
===============
*/

struct Touch_ItemTouch : public GameEntity::EntityFunc_Touch
{
	virtual void execute( GameEntity *other, trace_t *trace ) 
	{
		int		respawn;
		bool	predict;
		GameEntity*	self = self_;  // Since we may delete our current think()

		if( !other->client_ )
			return;
		if( other->health_ < 1 )
			return;		// dead people can't pickup
		if( other->client_->ps_.pm_type == PM_SPECTATOR )
			return;

		// the same pickup rules are used for client side and server side
		if ( !BG_CanItemBeGrabbed( g_gametype.integer, &self->s, &other->client_->ps_, other->client_->vehicle_ ) ) 
			return;

		G_LogPrintf( "Item: %i %s\n", other->s.number, self->item_->classname );

		predict = other->client_->pers_.predictItemPickup_;

		// call the item-specific pickup function
		switch( self->item_->giType )
		{
		case IT_AMMO:
			respawn = Pickup_Ammo(self, other);
			break;
		case IT_HEALTH:
			respawn = Pickup_Health(self, other);
			break;
		case IT_FUEL:
			respawn = Pickup_Fuel(self, other);
			break;
		case IT_TEAM:
			respawn = Pickup_Team(self, other);
			break;
		default:
			return;
		}

		if ( !respawn ) 
			return;

		// play the normal pickup sound
		if( predict ) 
			G_AddPredictableEvent( other, EV_ITEM_PICKUP, self->s.modelindex );
		else 
			G_AddEvent( other, EV_ITEM_PICKUP, self->s.modelindex, true );

		// powerup pickups are global broadcasts
		if( self->item_->giType == IT_TEAM) 
		{
			// if we want the global sound to play
			if( !self->speed_ ) 
			{
				GameEntity	*te;
				te = G_TempEntity( self->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
				te->s.eventParm = self->s.modelindex;
				te->r.svFlags |= SVF_BROADCAST;
			} 
			else 
			{
				GameEntity	*te;
				te = G_TempEntity( self->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
				te->s.eventParm = self->s.modelindex;
				// only send this temp entity to a single client
				te->r.svFlags |= SVF_SINGLECLIENT;
				te->r.singleClient = other->s.number;
			}
		}

		// fire item targets
		G_UseTargets (self, other);

		// wait of -1 will not respawn
		if( self->wait_ == -1 ) 
		{
			self->r.svFlags |= SVF_NOCLIENT;
			self->s.eFlags |= EF_NODRAW;
			self->r.contents = 0;
			self->unlinkAfterEvent_ = true;
			return;
		}

		// non zero wait overrides respawn time
		if( self->wait_ ) 
			respawn = self->wait_;

		// random can be used to vary the respawn time
		if( self->random_ ) 
		{
			respawn += crandom() * self->random_;
			if ( respawn < 1 ) 
				respawn = 1;
		}

		// dropped items will not respawn
		if( self->flags_ & FL_DROPPED_ITEM ) 
			self->freeAfterEvent_ = true;

		// picked up items still stay around, they just don't
		// draw anything.  This allows respawnable items
		// to be placed on movers.
		self->r.svFlags |= SVF_NOCLIENT;
		self->s.eFlags |= EF_NODRAW;
		self->r.contents = 0;

		// ZOID
		// A negative respawn times means to never respawn this item (but don't 
		// delete it).  This is used by items that are respawned by third party 
		// events such as ctf flags
		if( respawn <= 0 ) 
		{
			self->nextthink_ = 0;
			self->setThink(0);
		} 
		else 
		{
			self->nextthink_ = theLevel.time_ + respawn * 1000;
			self->setThink(new Think_RespawnItem);
		}
		SV_LinkEntity( self );
	}
};

//======================================================================

struct Think_Team_DroppedFlagThink : public GameEntity::EntityFunc_Think
{
	virtual void execute()
	{
		Team_DroppedFlagThink(self_);
	}
};

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
GameEntity *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity ) 
{
	GameEntity	*dropped;

	dropped = theLevel.spawnEntity();

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname_ = item->classname;
	dropped->item_ = item;
	VectorSet( dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );
	dropped->r.contents = CONTENTS_TRIGGER;

	dropped->setTouch(new Touch_ItemTouch);

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = theLevel.time_;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->s.eFlags |= EF_BOUNCE_HALF;

	if( g_gametype.integer == GT_CTF && item->giType == IT_TEAM) 
	{ 
		// Special case for CTF flags
		dropped->setThink(new Think_Team_DroppedFlagThink);
		dropped->nextthink_ = theLevel.time_ + 180000;
		Team_CheckDroppedItem( dropped );
	} 
	else
	{
		// auto-remove after 30 seconds
		dropped->setThink(new GameEntity::EntityFunc_Free);
		dropped->nextthink_ = theLevel.time_ + 30000;
	}

	dropped->flags_ = FL_DROPPED_ITEM;

	SV_LinkEntity( dropped );

	return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
================
*/
GameEntity *Drop_Item( GameEntity *ent, gitem_t *item, float angle ) 
{
	vec3_t	velocity;
	vec3_t	angles;

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0;	// always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + crandom() * 50;
	
	return LaunchItem( item, ent->s.pos.trBase, velocity );
}


/*
================
Use_Item

Respawn the item
================
*/
struct Use_ItemUse : public GameEntity::EntityFunc_Use
{
	virtual void execute( GameEntity *other, GameEntity *activator )
	{
		RespawnItem( self_ );
	}
};

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void GameItem::Think_FinishSpawningItem::execute() 
{
	trace_t		tr;
	vec3_t		dest;

	VectorSet( self_->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( self_->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );

	self_->s.eType = ET_ITEM;
	self_->s.modelindex = self_->item_ - bg_itemlist;		// store item number in modelindex
	self_->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	self_->r.contents = CONTENTS_TRIGGER;
	self_->setTouch(new Touch_ItemTouch);

	// useing an item causes it to respawn
	self_->setUse(new Use_ItemUse);

	if( self_->spawnflags_ & 1 ) 
	{
		// suspended
		G_SetOrigin( self_, self_->s.origin );
	} 
	else 
	{
		// drop to floor
		VectorSet( dest, self_->s.origin[0], self_->s.origin[1], self_->s.origin[2] - 4096 );
		SV_Trace( &tr, self_->s.origin, self_->r.mins, self_->r.maxs, dest, self_->s.number, MASK_SOLID, false );
		if( tr.startsolid ) 
		{
			Com_Printf ("FinishSpawningItem: %s startsolid at %s\n", self_->classname_, vtos(self_->s.origin));
			//theLevel.removeEntity(self_);
			self_->freeUp();
			return;
		}

		// allow to ride movers
		self_->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( self_, tr.endpos );
	}

	// team slaves and targeted items aren't present at start
	if( ( self_->flags_ & FL_TEAMSLAVE ) || self_->targetname_ ) 
	{
		self_->s.eFlags |= EF_NODRAW;
		self_->r.contents = 0;
		return;
	}

	SV_LinkEntity( self_ );
}

bool	itemRegistered[MAX_ITEMS];

/*
==================
G_CheckTeamItems
==================
*/
void G_CheckTeamItems() 
{

	// Set up team stuff
	Team_InitGame();

	if( g_gametype.integer == GT_CTF ) {
		gitem_t	*item;

		// check for the two flags
		item = BG_FindItem( "Red Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_redflag in map" );
		}
		item = BG_FindItem( "Blue Flag" );
		if ( !item || !itemRegistered[ item - bg_itemlist ] ) {
			Com_Printf( S_COLOR_YELLOW "WARNING: No team_CTF_blueflag in map" );
		}
	}
}

/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems() 
{
#pragma message("this should probably be removed!")
	memset( itemRegistered, 0, sizeof( itemRegistered ) );
	// Always load health/ammo/fuel pickups
	RegisterItem( BG_FindItem( "5 Health" ) );
	RegisterItem( BG_FindItem( "25 Health" ) );
	RegisterItem( BG_FindItem( "50 Health" ) );
	RegisterItem( BG_FindItem( "Some Fuel" ) );
	RegisterItem( BG_FindItem( "More Fuel" ) );
	RegisterItem( BG_FindItem( "Shells" ) );
	RegisterItem( BG_FindItem( "Bullets" ) );
	RegisterItem( BG_FindItem( "Slugs" ) );
	RegisterItem( BG_FindItem( "Rockets" ) );
}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item )
{
	if ( !item ) {
		Com_Error( ERR_DROP, "RegisterItem: NULL" );
	}
	itemRegistered[ item - bg_itemlist ] = true;
}


/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems()
{
	char	string[MAX_ITEMS+1];
	int		i;
	int		count;

	count = 0;
	for ( i = 0 ; i < bg_numItems ; i++ )
	{
		if ( itemRegistered[i] )
		{
			count++;
			string[i] = '1';
		} 
		else 
		{
			string[i] = '0';
		}
	}
	string[ bg_numItems ] = 0;

	Com_Printf( "%i items registered\n", count );
	SV_SetConfigstring(CS_ITEMS, string);
}

///*
//============
//G_SpawnItem
//
//Sets the clipping size and plants the object on the floor.
//
//Items can't be immediately dropped to floor, because they might
//be on an entity that hasn't spawned yet.
//============
//*/
//void G_SpawnItem( SpawnFieldHolder *sfh, gitem_t *item ) 
//{
//	G_SpawnFloat( "random", "0", &ent->random_ );
//	G_SpawnFloat( "wait", "0", &ent->wait_ );
//
//	RegisterItem( item );
////	if ( G_ItemDisabled(item) )		// MFQ3
////		return;
//
//	ent->item_ = item;
//	// some movers spawn on the second frame, so delay item
//	// spawns until the third frame so they can ride trains
//	ent->nextthink_ = theLevel.time_ + FRAMETIME * 2;
//	ent->setThink(new Think_FinishSpawningItem);
//
//	ent->physicsBounce_ = 0.50;		// items are bouncy
//}


/*
================
G_BounceItem

================
*/
void G_BounceItem( GameEntity *ent, trace_t *trace ) 
{
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = theLevel.previousTime_ + ( theLevel.time_ - theLevel.previousTime_ ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce_, ent->s.pos.trDelta );

	// check for stop
	if( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 )
	{
		trace->endpos[2] += 1.0;	// make sure it is off ground
		SnapVector( trace->endpos );
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;
		return;
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = theLevel.time_;
}


/*
================
G_RunItem

================
*/
void G_RunItem( GameEntity *ent )
{
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == -1 ) 
	{
		if ( ent->s.pos.trType != TR_GRAVITY )
		{
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = theLevel.time_;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY )
	{
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, theLevel.time_, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask_ ) 
		mask = ent->clipmask_;
	else 
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;

	SV_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 
		ent->r.ownerNum, mask, false );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) 
		tr.fraction = 0;

	SV_LinkEntity( ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	if ( tr.fraction == 1 ) 
		return;

	// if it is in a nodrop volume, remove it
	contents = SV_PointContents( ent->r.currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP ) 
	{
		if (ent->item_ && ent->item_->giType == IT_TEAM) 
			Team_FreeEntity(ent);
		else 
			ent->freeUp();
		return;
	}

	G_BounceItem( ent, &tr );
}

