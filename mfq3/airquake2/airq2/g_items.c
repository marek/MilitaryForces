#include "g_local.h"


qboolean	Pickup_Weapon (edict_t *ent, edict_t *other);
void		Use_Weapon (edict_t *ent, gitem_t *inv);
void 		Use_Secondary( edict_t *ent, gitem_t *inv );
void		Drop_Weapon (edict_t *ent, gitem_t *inv);

void Weapon_Blaster (edict_t *ent);
void Weapon_Shotgun (edict_t *ent);
void Weapon_SuperShotgun (edict_t *ent);
void Weapon_Machinegun (edict_t *ent);
void Weapon_Chaingun (edict_t *ent);
void Weapon_HyperBlaster (edict_t *ent);
void Weapon_RocketLauncher_Fire (edict_t *ent);
void Weapon_Grenade (edict_t *ent);
void Weapon_GrenadeLauncher (edict_t *ent);
void Weapon_Railgun (edict_t *ent);
void Weapon_BFG (edict_t *ent);
void Weapon_Sidewinder_Fire( edict_t *ent );
void Weapon_Stinger_Fire( edict_t *ent );
void Weapon_AMRAAM_Fire( edict_t *ent );
void Weapon_Phoenix_Fire( edict_t *ent );
void Weapon_Bombs_Fire( edict_t *ent );
void Weapon_MainGun_Fire( edict_t *ent );
void Weapon_ATGM_Fire( edict_t *ent );
void Weapon_LGB_Fire( edict_t *ent );
void Weapon_ECM_Fire( edict_t *ent );
void Weapon_Maverick_Fire( edict_t *ent );
void Weapon_Autocannon_Fire( edict_t *ent );

gitem_armor_t jacketarmor_info	= { 25,  50, .30, .00, ARMOR_JACKET};
gitem_armor_t combatarmor_info	= { 50, 100, .60, .30, ARMOR_COMBAT};
gitem_armor_t bodyarmor_info	= {100, 200, .80, .60, ARMOR_BODY};

static int	jacket_armor_index;
static int	combat_armor_index;
static int	body_armor_index;
static int	power_screen_index;
static int	power_shield_index;

#define HEALTH_IGNORE_MAX	1
#define HEALTH_TIMED		2

// void Use_Quad (edict_t *ent, gitem_t *item);
//static int	quad_drop_timeout_hack;

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
/*
qboolean Pickup_Powerup (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];
	if ((skill->value == 1 && quantity >= 2) || (skill->value >= 2 && quantity >= 1))
		return false;

	if ((coop->value) && (ent->item->flags & IT_STAY_COOP) && (quantity > 0))
		return false;

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		if (((int)dmflags->value & DF_INSTANT_ITEMS) || ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM)))
		{
			if ((ent->item->use == Use_Quad) && (ent->spawnflags & DROPPED_PLAYER_ITEM))
				quad_drop_timeout_hack = (ent->nextthink - level.time) / FRAMETIME;
			ent->item->use (other, ent->item);
		}
	}

	return true;
}*/
/*
void Drop_General (edict_t *ent, gitem_t *item)
{
	Drop_Item (ent, item);
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
}
*/

//======================================================================

qboolean Pickup_Adrenaline (edict_t *ent, edict_t *other)
{
	if (!deathmatch->value)
		other->max_health += 1;

	if (other->health < other->max_health)
		other->health = other->max_health;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_AncientHead (edict_t *ent, edict_t *other)
{
	other->max_health += 2;

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Bandolier (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < 250)
		other->client->pers.max_bullets = 250;
	if (other->client->pers.max_shells < 150)
		other->client->pers.max_shells = 150;
	if (other->client->pers.max_cells < 250)
		other->client->pers.max_cells = 250;
	if (other->client->pers.max_slugs < 75)
		other->client->pers.max_slugs = 75;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

qboolean Pickup_Pack (edict_t *ent, edict_t *other)
{
	gitem_t	*item;
	int		index;

	if (other->client->pers.max_bullets < 300)
		other->client->pers.max_bullets = 300;
	if (other->client->pers.max_shells < 200)
		other->client->pers.max_shells = 200;
	if (other->client->pers.max_rockets < 100)
		other->client->pers.max_rockets = 100;
	if (other->client->pers.max_grenades < 100)
		other->client->pers.max_grenades = 100;
	if (other->client->pers.max_cells < 300)
		other->client->pers.max_cells = 300;
	if (other->client->pers.max_slugs < 100)
		other->client->pers.max_slugs = 100;

	item = FindItem("Bullets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_bullets)
			other->client->pers.inventory[index] = other->client->pers.max_bullets;
	}

	item = FindItem("Shells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_shells)
			other->client->pers.inventory[index] = other->client->pers.max_shells;
	}

	item = FindItem("Cells");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_cells)
			other->client->pers.inventory[index] = other->client->pers.max_cells;
	}

	item = FindItem("Grenades");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_grenades)
			other->client->pers.inventory[index] = other->client->pers.max_grenades;
	}

	item = FindItem("Rockets");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_rockets)
			other->client->pers.inventory[index] = other->client->pers.max_rockets;
	}

	item = FindItem("Slugs");
	if (item)
	{
		index = ITEM_INDEX(item);
		other->client->pers.inventory[index] += item->quantity;
		if (other->client->pers.inventory[index] > other->client->pers.max_slugs)
			other->client->pers.inventory[index] = other->client->pers.max_slugs;
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, ent->item->quantity);

	return true;
}

//======================================================================
/*
void Use_Quad (edict_t *ent, gitem_t *item)
{
	int		timeout;

	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (quad_drop_timeout_hack)
	{
		timeout = quad_drop_timeout_hack;
		quad_drop_timeout_hack = 0;
	}
	else
	{
		timeout = 300;
	}

	if (ent->client->quad_framenum > level.framenum)
		ent->client->quad_framenum += timeout;
	else
		ent->client->quad_framenum = level.framenum + timeout;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}
*/
//======================================================================
/*
void Use_Breather (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->breather_framenum > level.framenum)
		ent->client->breather_framenum += 300;
	else
		ent->client->breather_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}
*/
//======================================================================
/*
void Use_Envirosuit (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->enviro_framenum > level.framenum)
		ent->client->enviro_framenum += 300;
	else
		ent->client->enviro_framenum = level.framenum + 300;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}
*/
//======================================================================
/*
void	Use_Invulnerability (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);

	if (ent->client->invincible_framenum > level.framenum)
		ent->client->invincible_framenum += 300;
	else
		ent->client->invincible_framenum = level.framenum + 300;

	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/protect.wav"), 1, ATTN_NORM, 0);
}
*/
//======================================================================
/*
void	Use_Silencer (edict_t *ent, gitem_t *item)
{
	ent->client->pers.inventory[ITEM_INDEX(item)]--;
	ValidateSelectedItem (ent);
	ent->client->silencer_shots += 30;

//	gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage.wav"), 1, ATTN_NORM, 0);
}
*/
//======================================================================

qboolean Pickup_Key (edict_t *ent, edict_t *other)
{
	if (coop->value)
	{
		if (strcmp(ent->classname, "key_power_cube") == 0)
		{
			if (other->client->pers.power_cubes & ((ent->spawnflags & 0x0000ff00)>> 8))
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
			other->client->pers.power_cubes |= ((ent->spawnflags & 0x0000ff00) >> 8);
		}
		else
		{
			if (other->client->pers.inventory[ITEM_INDEX(ent->item)])
				return false;
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = 1;
		}
		return true;
	}
	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;
	return true;
}

//======================================================================

qboolean Add_Ammo (edict_t *ent, gitem_t *item, int count)
{
	int			index;
	int			max;

	if (!ent->client)
		return false;

	if (item->tag == AMMO_BULLETS)
		max = ent->client->pers.max_bullets;
	else if (item->tag == AMMO_SHELLS)
		max = ent->client->pers.max_shells;
	else if (item->tag == AMMO_ROCKETS)
		max = ent->client->pers.max_rockets;
	else if (item->tag == AMMO_GRENADES)
		max = ent->client->pers.max_grenades;
	else if (item->tag == AMMO_CELLS)
		max = ent->client->pers.max_cells;
	else if (item->tag == AMMO_SLUGS)
		max = ent->client->pers.max_slugs;
	else if (item->tag == AMMO_SIDEWINDER)
	{
		if( ent->client->pers.max_sidewinder > ent->client->pers.max_stinger )
			max = ent->client->pers.max_sidewinder;
		else
			max = ent->client->pers.max_stinger;
	}
	else if (item->tag == AMMO_STINGER)
		max = ent->client->pers.max_stinger;
	else if (item->tag == AMMO_AMRAAM)
	{
		if( ent->client->pers.max_phoenix > ent->client->pers.max_amraam )
			max = ent->client->pers.max_phoenix;
		else
			max = ent->client->pers.max_amraam;
	}
	else if (item->tag == AMMO_PHOENIX)
		max = ent->client->pers.max_phoenix;
	else if (item->tag == AMMO_ATGM)
	{
		if( ent->client->pers.max_mavericks > ent->client->pers.max_ATGMs )
			max = ent->client->pers.max_mavericks;
		else
			max = ent->client->pers.max_ATGMs;
	}
	else if (item->tag == AMMO_MAVERICK)
		max = ent->client->pers.max_mavericks;
	else if (item->tag == AMMO_BOMBS)
	{
		if( ent->client->pers.max_lgbs > ent->client->pers.max_bombs )
			max = ent->client->pers.max_lgbs;
		else
			max = ent->client->pers.max_bombs;
	}
	else if( item->tag == AMMO_LGB )
		max = ent->client->pers.max_lgbs;
	else
		return false;

	index = ITEM_INDEX(item);

	if( item->tag == AMMO_ATGM )
	{
		if( ent->client->pers.max_mavericks > ent->client->pers.max_ATGMs )
			index = ITEM_INDEX( FindItem( "Maverick" ) );
	}
	else if( item->tag == AMMO_BOMBS )
	{
		if( ent->client->pers.max_lgbs > ent->client->pers.max_bombs )
			index = ITEM_INDEX( FindItem( "LGB" ) );
	}
	else if( item->tag == AMMO_SIDEWINDER )
	{
		if( ent->client->pers.max_stinger > ent->client->pers.max_sidewinder )
			index = ITEM_INDEX( FindItem( "Stinger" ) );
	}
	else if( item->tag == AMMO_AMRAAM )
	{
		if( ent->client->pers.max_phoenix > ent->client->pers.max_amraam )
			index = ITEM_INDEX( FindItem( "Phoenix" ) );
	}

	if (ent->client->pers.inventory[index] == max)
		return false;

	ent->client->pers.inventory[index] += count;

//	gi.bprintf( PRINT_HIGH, "%s\n", item->pickup_name);

	if (ent->client->pers.inventory[index] > max)
		ent->client->pers.inventory[index] = max;

	return true;
}

qboolean Pickup_Ammo (edict_t *ent, edict_t *other)
{
	int			oldcount;
	int			count;
	qboolean	weapon;


	weapon = (ent->item->flags & IT_WEAPON);
	if ( (weapon) && ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		count = 1000;
	else if (ent->count)
		count = ent->count;
	else
		count = ent->item->quantity;

	oldcount = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	if (!Add_Ammo (other, ent->item, count))
		return false;

	if (weapon && !oldcount)
	{
		if (other->client->pers.weapon != ent->item &&
	    ( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
			other->client->newweapon = ent->item;
	}

	if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM)) && (deathmatch->value))
		SetRespawn (ent, 30);
	return true;
}

/*
void Drop_Ammo (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	int		index;

	index = ITEM_INDEX(item);
	dropped = Drop_Item (ent->vehicle, item);		// +BD +vehicle
	if (ent->client->pers.inventory[index] >= item->quantity)
		dropped->count = item->quantity;
	else
		dropped->count = ent->client->pers.inventory[index];
	ent->client->pers.inventory[index] -= dropped->count;
	ValidateSelectedItem (ent);
}
*/

//======================================================================

void MegaHealth_think (edict_t *self)
{
	if (self->owner->health > self->owner->max_health)
	{
		self->nextthink = level.time + 1;
		self->owner->health -= 1;
		return;
	}

	if (!(self->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (self, 20);
	else
		G_FreeEdict (self);
}

qboolean Pickup_Health (edict_t *ent, edict_t *other)
{
	int repair = 256;
	int done = 0;

	if (!(ent->style & HEALTH_IGNORE_MAX))
		if (other->vehicle->health >= other->vehicle->max_health && !other->vehicle->DAMAGE)
			return false;

	other->vehicle->health += 50;//(ent->count*other->vehicle->max_health/100);
	if( other->vehicle->health > other->vehicle->max_health )
		other->vehicle->health = other->vehicle->max_health;

	if (ent->count == 2)
		ent->item->pickup_sound = "items/s_health.wav";
	else if (ent->count == 10)
		ent->item->pickup_sound = "items/n_health.wav";
	else if (ent->count == 25)
		ent->item->pickup_sound = "items/l_health.wav";
	else // (ent->count == 100)
		ent->item->pickup_sound = "items/m_health.wav";

	if (!(ent->style & HEALTH_IGNORE_MAX))
	{
		if (other->vehicle->health > other->vehicle->max_health)
			other->vehicle->health = other->vehicle->max_health;
	}

	if (ent->style & HEALTH_TIMED)
	{
		ent->think = MegaHealth_think;
		ent->nextthink = level.time + 5;
		ent->owner = other;
		ent->flags |= FL_RESPAWN;
		ent->svflags |= SVF_NOCLIENT;
		ent->solid = SOLID_NOT;
	}
	else
	{
		if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
			SetRespawn (ent, 30);
	}
	if( other->vehicle->DAMAGE )
	{
		while( !done )
		{
			if( other->vehicle->DAMAGE & repair )
			{
				other->vehicle->DAMAGE ^= repair;
				done = 1;
			}
			else
				repair /= 2;
		}
		switch( repair )
		{
			case AUTOPILOT_DAMAGE:
				gi.cprintf( other, PRINT_MEDIUM, "Autopilot repaired.\n" );
				break;
			case GPS_DAMAGE:
				gi.cprintf( other, PRINT_MEDIUM, "GPS repaired.\n" );
				break;
			case ENGINE0:
				gi.cprintf( other, PRINT_MEDIUM, "Engine 25 percent workable.\n" );
				break;
			case ENGINE25:
				gi.cprintf( other, PRINT_MEDIUM, "Engine 50 percent workable.\n" );
				break;
			case ENGINE50:
				gi.cprintf( other, PRINT_MEDIUM, "Engine 75 percent workable.\n" );
				break;
			case ENGINE75:
				gi.cprintf( other, PRINT_MEDIUM, "Engine 100 percent workable.\n" );
				break;
			case AFTERBURNER_DAMAGE:
				if( strcmp( other->vehicle->classname,"ground") == 0 )
					gi.cprintf( other, PRINT_MEDIUM, "Reverse repaired.\n" );
				else
					gi.cprintf( other, PRINT_MEDIUM, "Afterburner repaired.\n" );
				break;
			case SPEEDBRAKE_DAMAGE:
				gi.cprintf( other, PRINT_MEDIUM, "Brakes repaired.\n" );
				break;
			case GEAR_DAMAGE:
				gi.cprintf( other, PRINT_MEDIUM, "Gear repaired.\n" );
				break;
		}
	}

	return true;
}

//======================================================================
/*
int ArmorIndex (edict_t *ent)
{
	if (!ent->client)
		return 0;

	if (ent->client->pers.inventory[jacket_armor_index] > 0)
		return jacket_armor_index;

	if (ent->client->pers.inventory[combat_armor_index] > 0)
		return combat_armor_index;

	if (ent->client->pers.inventory[body_armor_index] > 0)
		return body_armor_index;

	return 0;
}
*/
/*
qboolean Pickup_Armor (edict_t *ent, edict_t *other)
{
	int				old_armor_index;
	gitem_armor_t	*oldinfo;
	gitem_armor_t	*newinfo;
	int				newcount;
	float			salvage;
	int				salvagecount;

	// get info on new armor
	newinfo = (gitem_armor_t *)ent->item->info;

	old_armor_index = ArmorIndex (other);

	// handle armor shards specially
	if (ent->item->tag == ARMOR_SHARD)
	{
		if (!old_armor_index)
			other->client->pers.inventory[jacket_armor_index] = 2;
		else
			other->client->pers.inventory[old_armor_index] += 2;
	}

	// if player has no armor, just use it
	else if (!old_armor_index)
	{
		other->client->pers.inventory[ITEM_INDEX(ent->item)] = newinfo->base_count;
	}

	// use the better armor
	else
	{
		// get info on old armor
		if (old_armor_index == jacket_armor_index)
			oldinfo = &jacketarmor_info;
		else if (old_armor_index == combat_armor_index)
			oldinfo = &combatarmor_info;
		else // (old_armor_index == body_armor_index)
			oldinfo = &bodyarmor_info;

		if (newinfo->normal_protection > oldinfo->normal_protection)
		{
			// calc new armor values
			salvage = oldinfo->normal_protection / newinfo->normal_protection;
			salvagecount = salvage * other->client->pers.inventory[old_armor_index];
			newcount = newinfo->base_count + salvagecount;
			if (newcount > newinfo->max_count)
				newcount = newinfo->max_count;

			// zero count of old armor so it goes away
			other->client->pers.inventory[old_armor_index] = 0;

			// change armor to new item with computed value
			other->client->pers.inventory[ITEM_INDEX(ent->item)] = newcount;
		}
		else
		{
			// calc new armor values
			salvage = newinfo->normal_protection / oldinfo->normal_protection;
			salvagecount = salvage * newinfo->base_count;
			newcount = other->client->pers.inventory[old_armor_index] + salvagecount;
			if (newcount > oldinfo->max_count)
				newcount = oldinfo->max_count;

			// if we're already maxed out then we don't need the new armor
			if (other->client->pers.inventory[old_armor_index] >= newcount)
				return false;

			// update current armor value
			other->client->pers.inventory[old_armor_index] = newcount;
		}
	}

	if (!(ent->spawnflags & DROPPED_ITEM) && (deathmatch->value))
		SetRespawn (ent, 20);

	return true;
}
*/
//======================================================================
/*
int PowerArmorType (edict_t *ent)
{
	if (!ent->client)
		return POWER_ARMOR_NONE;

	if (!(ent->flags & FL_POWER_ARMOR))
		return POWER_ARMOR_NONE;

	if (ent->client->pers.inventory[power_shield_index] > 0)
		return POWER_ARMOR_SHIELD;

	if (ent->client->pers.inventory[power_screen_index] > 0)
		return POWER_ARMOR_SCREEN;

	return POWER_ARMOR_NONE;
}
*/
/*
void Use_PowerArmor (edict_t *ent, gitem_t *item)
{
	int		index;

	if (ent->flags & FL_POWER_ARMOR)
	{
		ent->flags &= ~FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power2.wav"), 1, ATTN_NORM, 0);
	}
	else
	{
		index = ITEM_INDEX(FindItem("cells"));
		if (!ent->client->pers.inventory[index])
		{
			gi.cprintf (ent, PRINT_HIGH, "No cells for power armor.\n");
			return;
		}
		ent->flags |= FL_POWER_ARMOR;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("misc/power1.wav"), 1, ATTN_NORM, 0);
	}
}
*/
/*
qboolean Pickup_PowerArmor (edict_t *ent, edict_t *other)
{
	int		quantity;

	quantity = other->client->pers.inventory[ITEM_INDEX(ent->item)];

	other->client->pers.inventory[ITEM_INDEX(ent->item)]++;

	if (deathmatch->value)
	{
		if (!(ent->spawnflags & DROPPED_ITEM) )
			SetRespawn (ent, ent->item->quantity);
		// auto-use for DM only if we didn't already have one
		if (!quantity)
			ent->item->use (other, ent->item);
	}

	return true;
}
*/
/*
void Drop_PowerArmor (edict_t *ent, gitem_t *item)
{
	if ((ent->flags & FL_POWER_ARMOR) && (ent->client->pers.inventory[ITEM_INDEX(item)] == 1))
		Use_PowerArmor (ent, item);
	Drop_General (ent, item);
}
*/
//======================================================================

/*
===============
Touch_Item
===============
*/
void Touch_Item (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	qboolean	taken;

//	if (!other->client)		// +BD the old version
	// +BD here the new one:
	if( strcmp( other->classname, "plane" ) != 0 &&
		strcmp( other->classname, "helo" ) != 0 &&
		strcmp( other->classname, "ground" ) != 0 )
		return;
	if( other->HASCAN & IS_DRONE )// drone
		return;
	if (other->health < 1)
		return;		// dead vehicles can't pickup
	if (!ent->item->pickup)
		return;		// not a grabbable item?

	taken = ent->item->pickup(ent, other->owner);// +BD +owner

	if (taken)
	{
		// flash the screen
		other->owner->client->bonus_alpha = 0.25;// +BD +owner

		// show icon and name on status bar // disabled +BD
/*		other->owner->client->ps.stats[STAT_PICKUP_ICON] = gi.imageindex(ent->item->icon);// +BD +owner
		other->owner->client->ps.stats[STAT_PICKUP_STRING] = CS_ITEMS+ITEM_INDEX(ent->item);// +BD detto
		other->owner->client->pickup_msg_time = level.time + 3.0;// +BD detto
*/
		// change selected item
		if (ent->item->use)
			other->owner->client->pers.selected_item = other->owner->client->ps.stats[STAT_SELECTED_ITEM] = ITEM_INDEX(ent->item);
					// +BD 2x owner

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

static void drop_temp_touch (edict_t *ent, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other == ent->owner)
		return;

	Touch_Item (ent, other, plane, surf);
}

static void drop_make_touchable (edict_t *ent)
{
	ent->touch = Touch_Item;
	if (deathmatch->value)
	{
		ent->nextthink = level.time + 29;
		ent->think = G_FreeEdict;
	}
}

edict_t *Drop_Item (edict_t *ent, gitem_t *item)
{
	edict_t	*dropped;
	vec3_t	forward, right;
//	vec3_t	offset; +BD removed

	dropped = G_Spawn();

	dropped->classname = item->classname;
	dropped->item = item;
	dropped->spawnflags = DROPPED_ITEM;
	dropped->s.effects = item->world_model_flags;
	dropped->s.renderfx = RF_GLOW;
	VectorSet (dropped->mins, -15, -15, -15);
	VectorSet (dropped->maxs, 15, 15, 15);
	gi.setmodel (dropped, dropped->item->world_model);
	dropped->solid = SOLID_TRIGGER;
	dropped->movetype = MOVETYPE_TOSS;
	dropped->touch = drop_temp_touch;
	dropped->owner = ent;

	// +BD remove that part in order to drop it by vehicle
/*	if (ent->client)
	{
		trace_t	trace;

		AngleVectors (ent->client->v_angle, forward, right, NULL);
		VectorSet(offset, 24, 0, -16);
		G_ProjectSource (ent->s.origin, offset, forward, right, dropped->s.origin);
		trace = gi.trace (ent->s.origin, dropped->mins, dropped->maxs,
			dropped->s.origin, ent, CONTENTS_SOLID);
		VectorCopy (trace.endpos, dropped->s.origin);
	}
	else
	{*/
		AngleVectors (ent->s.angles, forward, right, NULL);
		VectorCopy (ent->s.origin, dropped->s.origin);
//	}

	VectorScale (forward, 100, dropped->velocity);
	dropped->velocity[2] = 300;

	dropped->think = drop_make_touchable;
	dropped->nextthink = level.time + 1;

	gi.linkentity (dropped);

	return dropped;
}

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
	char	*s, *start;
	char	data[MAX_QPATH];
	int		len;
	gitem_t	*ammo;

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
	if (it->ammo && it->ammo[0])
	{
		ammo = FindItem (it->ammo);
		if (ammo != it)
			PrecacheItem (ammo);
	}

	// parse the space seperated precache string for other items
	s = it->precaches;
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
	}
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
	PrecacheItem (item);

	if (ent->spawnflags)
	{
		if (strcmp(ent->classname, "key_power_cube") != 0)
		{
			ent->spawnflags = 0;
			gi.dprintf("%s at %s has invalid spawnflags set\n", ent->classname, vtos(ent->s.origin));
		}
	}

	// some items will be prevented in deathmatch
	if (deathmatch->value)
	{
		if ( (int)dmflags->value & DF_NO_ARMOR )
		{
/*			if (item->pickup == Pickup_Armor || item->pickup == Pickup_PowerArmor)
			{
				G_FreeEdict (ent);
				return;
			}
*/		}
		if ( (int)dmflags->value & DF_NO_ITEMS )
		{
/*			if (item->pickup == Pickup_Powerup)
			{
				G_FreeEdict (ent);
				return;
			}
*/		}
		if ( (int)dmflags->value & DF_NO_HEALTH )
		{
			if (item->pickup == Pickup_Health || item->pickup == Pickup_Adrenaline || item->pickup == Pickup_AncientHead)
			{
				G_FreeEdict (ent);
				return;
			}
		}
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
		{
			if ( (item->flags == IT_AMMO) || (strcmp(ent->classname, "weapon_bfg") == 0) )
			{
				G_FreeEdict (ent);
				return;
			}
		}
	}

	if (coop->value && (strcmp(ent->classname, "key_power_cube") == 0))
	{
		ent->spawnflags |= (1 << (8 + level.power_cubes));
		level.power_cubes++;
	}

	// don't let them drop items that stay in a coop game
	if ((coop->value) && (item->flags & IT_STAY_COOP))
	{
		item->drop = NULL;
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
	// WEAPONS
	//

/* weapon_blaster (.3 .3 1) (-16 -16 -16) (16 16 16)
always owned, never in the world
*/
	{
		"weapon_blaster",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_Blaster,
		"misc/w_pkup.wav",
		NULL, 0,
		"models/weapons/v_blast/tris.md2",
/* icon */		"w_blaster",
/* pickup */	"Blaster",
		0,
		0,
		NULL,
		IT_WEAPON|IT_STAY_COOP,
		NULL,
		0,
/* precache */ "weapons/blastf1a.wav misc/lasfly.wav"
	},

/*QUAKED weapon_autocannon (.3 .3 1) (-16 -16 -16) (16 16 16)
Not intended as pickup-item!
Therefore do not place in levels!
*/
	{
		"weapon_autocannon",
		NULL,
		Use_Weapon,
		NULL,
		Weapon_HyperBlaster,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_chaingun",
/* pickup */	"AutoCannon",
		0,
		1,
		"Bullets",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},


/*QUAKED weapon_maingun (.3 .3 1) (-16 -16 -16) (16 16 16)
Not intended as pickup-item!
Therefore do not place in levels!
*/
	{
		"weapon_maingun",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_MainGun_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_hyperb/tris.md2", EF_ROTATE,
		"models/weapons/v_hyperb/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"MainGun",
		0,
		1,
		"Shells",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "weapons/hyprbu1a.wav weapons/hyprbl1a.wav weapons/hyprbf1a.wav weapons/hyprbd1a.wav misc/lasfly.wav"
	},


/*QUAKED weapon_swlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_swlauncher",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_Sidewinder_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"SW Launcher",
		0,
		1,
		"Sidewinder",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_stlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_stlauncher",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_Stinger_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"ST Launcher",
		0,
		1,
		"Stinger",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_amlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_amlauncher",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_AMRAAM_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"AM Launcher",
		0,
		1,
		"AMRAAM",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_phlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_phlauncher",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_Phoenix_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"PH Launcher",
		0,
		1,
		"Phoenix",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_ATGM (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_ATGM",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_ATGM_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"ATGM Launcher",
		0,
		1,
		"ATGM",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_maverick (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_maverick",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_Maverick_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_railgun",
/* pickup */	"Maverick Launcher",
		0,
		1,
		"Maverick",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},

/*QUAKED weapon_ironbombs (.3 .3 1) (-16 -16 -16) (16 16 16)
Free fall bombs
*/
	{
		"weapon_ironbombs",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_Bombs_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Iron Bombs",
		0,
		1,
		"Bombs",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},


/*QUAKED weapon_laserbombs (.3 .3 1) (-16 -16 -16) (16 16 16)
Laser Guided Bombs
*/
	{
		"weapon_laserbombs",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_LGB_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Laser Bombs",
		0,
		1,
		"LGB",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},



/*QUAKED weapon_ECM (.3 .3 1) (-16 -16 -16) (16 16 16)
ECM System
*/
	{
		"weapon_ECM",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_ECM_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"ECM",
		0,
		1,
		"Jammer",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},



/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"weapon_rocketlauncher",
		NULL,
		Use_Secondary,
		NULL,
		Weapon_RocketLauncher_Fire,
		"misc/w_pkup.wav",
		"models/weapons/g_rocket/tris.md2", EF_ROTATE,
		"models/weapons/v_rocket/tris.md2",
/* icon */		"w_rlauncher",
/* pickup */	"Rocket Launcher",
		0,
		1,
		"Rockets",
		IT_WEAPON,
		NULL,
		0,
/* precache */ "models/objects/rocket/tris.md2 weapons/rockfly.wav weapons/rocklf1a.wav weapons/rocklr1b.wav models/objects/debris2/tris.md2"
	},
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
		"models/items/ammo/bullets/medium/tris.md2", 0,
		NULL,
/* icon */		"aacann",
/* pickup */	"Bullets",
/* width */		3,
		150,
		NULL,
		IT_AMMO,
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
		NULL,
		AMMO_SIDEWINDER,
/* precache */ ""
	},

/*QUAKED ammo_stinger (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_stinger",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/aam/stinger.md2", EF_ROTATE,
		NULL,
/* icon */		"stinger",
/* pickup */	"Stinger",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_STINGER,
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
		NULL,
		AMMO_AMRAAM,
/* precache */ ""
	},

/*QUAKED ammo_phoenix (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_phoenix",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/aam/phoenix.md2", EF_ROTATE,
		NULL,
/* icon */		"phoenix",
/* pickup */	"Phoenix",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_PHOENIX,
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
		NULL,
		AMMO_BOMBS,
/* precache */ ""
	},


/*QUAKED ammo_LGB (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_LGB",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/lgb/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"lgb",
/* pickup */	"LGB",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_LGB,
/* precache */ ""
	},



/*QUAKED ammo_jammer (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_jammer",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/lgb/tris.md2", EF_ROTATE,
		NULL,
/* icon */		"jammer",
/* pickup */	"Jammer",
/* width */		3,
		10,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_JAMMER,
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
/* pickup */	"ATGM",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ATGM,
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
/* pickup */	"ATGM",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_ATGM,
/* precache */ ""
	},

/*QUAKED ammo_Maverick (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
	{
		"ammo_Maverick",
		Pickup_Ammo,
		NULL,
		NULL,
		NULL,
		"misc/am_pkup.wav",
		"models/weapons/hellfire/hellfire_p.md2", EF_ROTATE,
		NULL,
/* icon */		"Maverick",
/* pickup */	"Maverick",
/* width */		3,
		4,
		NULL,
		IT_AMMO,
		NULL,
		AMMO_MAVERICK,
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

/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_small (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 2;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/s_health.wav");
}

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_large (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 25;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/l_health.wav");
}

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16)
*/
void SP_item_health_mega (edict_t *self)
{
	if ( deathmatch->value && ((int)dmflags->value & DF_NO_HEALTH) )
	{
		G_FreeEdict (self);
		return;
	}

	self->model = "models/items/healing/medium/tris.md2";
	self->count = 100;
	SpawnItem (self, FindItem ("Health"));
	gi.soundindex ("items/m_health.wav");
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

	jacket_armor_index = ITEM_INDEX(FindItem("Jacket Armor"));
	combat_armor_index = ITEM_INDEX(FindItem("Combat Armor"));
	body_armor_index   = ITEM_INDEX(FindItem("Body Armor"));
	power_screen_index = ITEM_INDEX(FindItem("Power Screen"));
	power_shield_index = ITEM_INDEX(FindItem("Power Shield"));
}
