#include "g_local.h"


//======================================================================

/*
===============
GetItemByIndex
===============
*/
gitem_t	*GetItemByIndex (int index)
{
	if (index == 0 || index >= game.num_items)
		return NULL;

	return &itemlist[index];
}


/*
===============
FindItemByClassname

===============
*/
gitem_t	*FindItemByClassname (char *classname)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->classname)
			continue;
		if (!Q_stricmp(it->classname, classname))
			return it;
	}

	return NULL;
}

/*
===============
FindItem

===============
*/
gitem_t	*FindItem (char *pickup_name)
{
	int		i;
	gitem_t	*it;

	it = itemlist;
	for (i=0 ; i<game.num_items ; i++, it++)
	{
		if (!it->pickup_name)
			continue;
		if (!Q_stricmp(it->pickup_name, pickup_name))
			return it;
	}

	return NULL;
}

//======================================================================

void DoRespawn (edict_t *ent)
{
	if (ent->team)
	{
		edict_t	*master;
		int	count;
		int choice;

		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->chain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->chain, count++)
			;
	}

	ent->svflags &= ~SVF_NOCLIENT;
	ent->solid = SOLID_TRIGGER;
	gi.linkentity (ent);

	// send an effect
	ent->s.event = EV_ITEM_RESPAWN;
}

void SetRespawn (edict_t *ent, float delay)
{
	ent->flags |= FL_RESPAWN;
	ent->svflags |= SVF_NOCLIENT;
	ent->solid = SOLID_NOT;
	ent->nextthink = level.time + delay;
	ent->think = DoRespawn;
	gi.linkentity (ent);
}

//======================================================================
// checks whether the vehicle needs this ammo, and adds it if yes
qboolean Has_Weapon( edict_t *ent, edict_t *ammo, int weap )
{
	int i;

	for( i = 0; i < MAX_WEAPONS; i++ )
	{
		if( ent->weapon[i] == weap )
		{
			// checking for maxammo and damage (damage is ammo < 0 !)
			if( ent->ammo[i] < ent->maxammo[i] && ent->ammo[i] >= 0 )
			{
				ent->ammo[i] += ammo->item->quantity;
				if( ent->ammo[i] > ent->maxammo[i] )
					ent->ammo[i] = ent->maxammo[i];
				return true;
			}
		}
	}
	return false;
}

//======================================================================

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	if( strcmp( ent->item->pickup_name, "Shells" ) == 0 )
	{
		if( !Has_Weapon( other, ent, WEAPON_MAINGUN ) && 
			!Has_Weapon( other, ent, WEAPON_ARTILLERY ) )
			return false;
	}
	else if( strcmp( ent->item->pickup_name, "Bullets" ) == 0 )
	{
		if( !Has_Weapon( other, ent, WEAPON_AUTOCANNON ) )
			return false;
	}	
	else if( strcmp( ent->item->pickup_name, "Rockets" ) == 0 )
	{
		if( !Has_Weapon( other, ent, WEAPON_FFAR ) )
			return false;
	}	
	else if( strcmp( ent->item->pickup_name, "Sidewinder" ) == 0 )
	{
		if( !Has_Weapon( other, ent, WEAPON_SIDEWINDER ) &&
			!Has_Weapon( other, ent, WEAPON_STINGER ) )
			return false;
	}	
	else if( strcmp( ent->item->pickup_name, "AMRAAM" ) == 0 )
	{
		if( !Has_Weapon( other, ent, WEAPON_AMRAAM ) &&
			!Has_Weapon( other, ent, WEAPON_PHOENIX ) )
			return false;
	}	
	else if( strcmp( ent->item->pickup_name, "Bombs" ) == 0 )
	{
		if( !Has_Weapon( other, ent, WEAPON_IRONBOMBS ) &&
			!Has_Weapon( other, ent, WEAPON_LASERBOMBS ) )
			return false;
	}
	else if( strcmp( ent->item->pickup_name, "AGM" ) == 0 )
	{
		if( !Has_Weapon( other, ent, WEAPON_HELLFIRE ) &&
			!Has_Weapon( other, ent, WEAPON_MAVERICK ) )
			return false;
	}
	else if( strcmp( ent->item->pickup_name, "Smart Weapon" ) == 0 )
	{
		if( !Has_Weapon( other, ent, WEAPON_RUNWAYBOMBS ) &&
			!Has_Weapon( other, ent, WEAPON_ANTIRADAR ) )
			return false;
	}
	else if( strcmp( ent->item->pickup_name, "Flares" ) == 0 )
	{
		if( other->flares == other->maxflares )
			return false;
		else
		{
			other->flares += ent->item->quantity;
			if( other->flares > other->maxflares )
				other->flares = other->maxflares;
		}
	}

	SetRespawn (ent, 30);

//	gi.bprintf( PRINT_HIGH, "%s %s %d\n", ent->classname, ent->item->pickup_name, ent->item->quantity );

	return true;
}


//======================================================================

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	int done = 0, repair = 32768;

	if (other->health >= other->max_health && !other->DAMAGE)
			return false;

	other->health += (other->max_health/4);

	if( other->DAMAGE )
	{
		while( done < 3 && other->DAMAGE )
		{
			if( other->DAMAGE & repair )
			{
				other->DAMAGE ^= repair;
				done++;
				if( repair & DAMAGE_WEAPON_0 )
				{
					other->ammo[0] = 0;
				}
				else if( repair & DAMAGE_WEAPON_1 )
				{
					other->ammo[1] = 0;
				}
				else if( repair & DAMAGE_WEAPON_2 )
				{
					other->ammo[2] = 0;
				}
				else if( repair & DAMAGE_WEAPON_3 )
				{
					other->ammo[3] = 0;
				}
				else if( repair & DAMAGE_WEAPON_4 )
				{
					other->ammo[4] = 0;
				}
			}
			else
				repair /= 2;
		}
	}

	if (other->health > other->max_health)
		other->health = other->max_health;

	SetRespawn (ent, 30);

	return true;
}

//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;

	if( strcmp(other->classname, "plane") != 0 &&
		strcmp(other->classname, "helo") != 0 &&
		strcmp(other->classname, "ground") != 0 &&
		strcmp(other->classname, "LQM") != 0 )
		return;

	if (other->health < 1)
		return;		// dead people can't pickup

	if (!ent->item->pickup)
		return;		// not a grabbable item?

	taken = ent->item->pickup(ent, other);

	if (taken)
	{
		gi.sound(other, CHAN_ITEM, gi.soundindex(ent->item->pickup_sound), 1, ATTN_NORM, 0);
	}

	if (!(ent->spawnflags & ITEM_TARGETS_USED))
	{
		G_UseTargets (ent, other);
		ent->spawnflags |= ITEM_TARGETS_USED;
	}

	if (!taken)
		return;

	if (!((coop->value) &&  (ent->item->flags & IT_STAY_COOP)) || (ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)))
	{
		if (ent->flags & FL_RESPAWN)
			ent->flags &= ~FL_RESPAWN;
		else
			G_FreeEdict (ent);
	}
}

//======================================================================

void Use_Item (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->svflags &= ~SVF_NOCLIENT;
	ent->use = NULL;

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->touch = Touch_Item;
	}

	gi.linkentity (ent);
}

//======================================================================

/*
================
droptofloor
================
*/
void droptofloor (edict_t *ent)
{
	trace_t		tr;
	vec3_t		dest;
	float		*v;

	v = tv(-15,-15,-15);
	VectorCopy (v, ent->mins);
	v = tv(15,15,15);
	VectorCopy (v, ent->maxs);

	if (ent->model)
		gi.setmodel (ent, ent->model);
	else
		gi.setmodel (ent, ent->item->world_model);
	ent->solid = SOLID_TRIGGER;
	ent->movetype = MOVETYPE_TOSS;  
	ent->touch = Touch_Item;

	v = tv(0,0,-128);
	VectorAdd (ent->s.origin, v, dest);

	tr = gi.trace (ent->s.origin, ent->mins, ent->maxs, dest, ent, MASK_SOLID);
	if (tr.startsolid)
	{
		gi.dprintf ("droptofloor: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
		G_FreeEdict (ent);
		return;
	}

	VectorCopy (tr.endpos, ent->s.origin);

	if (ent->team)
	{
		ent->flags &= ~FL_TEAMSLAVE;
		ent->chain = ent->teamchain;
		ent->teamchain = NULL;

		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		if (ent == ent->teammaster)
		{
			ent->nextthink = level.time + FRAMETIME;
			ent->think = DoRespawn;
		}
	}

	if (ent->spawnflags & ITEM_NO_TOUCH)
	{
		ent->solid = SOLID_BBOX;
		ent->touch = NULL;
		ent->s.effects &= ~EF_ROTATE;
		ent->s.renderfx &= ~RF_GLOW;
	}

	if (ent->spawnflags & ITEM_TRIGGER_SPAWN)
	{
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
		ent->use = Use_Item;
	}

	gi.linkentity (ent);
}


/*
===============
PrecacheItem

Precaches all data needed for a given item.
This will be called for each item spawned in a level,
and for each item in each client's inventory.
===============
*/
void PrecacheItem (gitem_t *it)
{
//	char	*s, *start;
//	char	data[MAX_QPATH];
//	int		len;
//	gitem_t	*ammo;

	if (!it)
		return;

	if (it->pickup_sound)
		gi.soundindex (it->pickup_sound);
	if (it->world_model)
		gi.modelindex (it->world_model);
	if (it->view_model)
		gi.modelindex (it->view_model);
	if (it->icon)
		gi.imageindex (it->icon);

	// parse everything for its ammo
/*	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
	}*/

	// parse the space seperated precache string for other items
/*	s = it->precaches;
	if (!s || !s[0])
		return;

	while (*s)
	{
		start = s;
		while (*s && *s != ' ')
			s++;

		len = s-start;
		if (len >= MAX_QPATH || len < 5)
			gi.error ("PrecacheItem: %s has bad precache string", it->classname);
		memcpy (data, start, len);
		data[len] = 0;
		if (*s)
			s++;

		// determine type based on extension
		if (!strcmp(data+len-3, "md2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "sp2"))
			gi.modelindex (data);
		else if (!strcmp(data+len-3, "wav"))
			gi.soundindex (data);
		if (!strcmp(data+len-3, "pcx"))
			gi.imageindex (data);
	}*/
}

/*
============
SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void SpawnItem (edict_t *ent, gitem_t *item)
{
//	PrecacheItem (item);

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
		if ( (int)dmflags->value & DF_NO_HEALTH )
		{
			if (item->pickup == Pickup_Health)
			{
				G_FreeEdict (ent);
				return;
			}
		}
	}

	ent->item = item;

	ent->nextthink = level.time + 2 * FRAMETIME;    // items start after other solids
	ent->think = droptofloor;
	ent->s.effects = item->world_model_flags;
	ent->s.renderfx = RF_GLOW;
	if (ent->model)
		gi.modelindex (ent->model);
}

//======================================================================

gitem_t	itemlist[] = 
{
	{
		NULL
	},	// leave index 0 alone

	//
	// AMMO ITEMS
	//


/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_shells",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/shell/shell_p.md2", EF_ROTATE,
		NULL,
/* icon */		"Heat",
/* pickup */	"Shells",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SHELLS,
/* precache */ ""
	},



/*QUAKED ammo_bullets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bullets",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/items/ammo/bullets/medium/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"aacann",
/* pickup */	"Bullets",
/* width */		3,
		150,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BULLETS,
/* precache */ ""
	},


/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_rockets",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/ffar/ffar.md2", EF_ROTATE,
		NULL,
/* icon */		"ffar",
/* pickup */	"Rockets",
/* width */		3,
		6,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ROCKETS,
/* precache */ ""
	},

/*QUAKED ammo_sidewinder (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_sidewinder",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/aam/aim9.md2", EF_ROTATE,
		NULL,
/* icon */		"sidewinder",
/* pickup */	"Sidewinder",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SIDEWINDER,
/* precache */ ""
	},

/*QUAKED ammo_amraam (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_amraam",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/aam/amraam.md2", EF_ROTATE,
		NULL,
/* icon */		"amraam",
/* pickup */	"AMRAAM",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_AMRAAM,
/* precache */ ""
	},


/*QUAKED ammo_bombs (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_bombs",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/lgb/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"mk82",
/* pickup */	"Bombs",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_BOMBS,
/* precache */ ""
	},


/*QUAKED ammo_ATGM (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_atgm",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/hellfire/hellfire_p.md2", EF_ROTATE,
		NULL,
/* icon */		"Hellfire",
/* pickup */	"AGM",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ATGM,
/* precache */ ""
	},


/*QUAKED ammo_ATGM (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_ATGM",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/hellfire/hellfire_p.md2", EF_ROTATE,
		NULL,
/* icon */		"Hellfire",
/* pickup */	"AGM",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_ATGM,
/* precache */ ""
	},

/*QUAKED ammo_smart (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_smart",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/lgb/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"Hellfire",
/* pickup */	"Smart Weapon",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_SMART,
/* precache */ ""
	},

/*QUAKED ammo_flares (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_flares",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/lgb/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"Hellfire",
/* pickup */	"Flares",
/* width */		3,
		5,
		NULL,
		IT_AMMO,
		0,
		NULL,
		AMMO_FLARES,
/* precache */ ""
	},


	{
		NULL,
		Pickup_Health,
		NULL,
		NULL,
		NULL,
		"items/pkup.wav",
		NULL, 0,
		NULL,
/* icon */		"i_health",
/* pickup */	"Health",
/* width */		3,
		0,
		NULL,
		0,
		0,
		NULL,
		0,
/* precache */ ""
	},

	// end of list marker
	{NULL}
};


/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 10;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/n_health.wav");
}


void InitItems (void)
{
	game.num_items = sizeof(itemlist)/sizeof(itemlist[0]) - 1;
}



/*
===============
SetItemNames

Called by worldspawn
===============
*/
void SetItemNames (void)
{
	int		i;
	gitem_t	*it;

	for (i=0 ; i<game.num_items ; i++)
	{
		it = &itemlist[i];
		gi.configstring (CS_ITEMS+i, it->pickup_name);
	}
}
