// g_weapon.c

#include "g_local.h"
#include "m_player.h"


static qboolean	is_quad;
static byte		is_silenced;


void weapon_grenade_fire (edict_t *ent, qboolean held);


static void P_ProjectSource (gclient_t *client, vec3_t point, vec3_t distance, vec3_t forward, vec3_t right, vec3_t result)
{
	vec3_t	_distance;

	VectorCopy (distance, _distance);
/*	if (client->pers.hand == LEFT_HANDED)
		_distance[1] *= -1;
	else if (client->pers.hand == CENTER_HANDED)*/
		_distance[1] = 0;		// +BD as we don't have hands on the plane
	G_ProjectSource (point, _distance, forward, right, result);
}


/*
===============
PlayerNoise

Each player can have two noise objects associated with it:
a personal noise (jumping, pain, weapon firing), and a weapon
target noise (bullet wall impacts)

Monsters that don't directly see the player can move
to a noise in hopes of seeing the player from there.
===============
*/
void PlayerNoise(edict_t *who, vec3_t where, int type)
{
	edict_t		*noise;
/*
	if (type == PNOISE_WEAPON)
	{
		if (who->client->silencer_shots)
		{
			who->client->silencer_shots--;
			return;
		}
	}
*/	// +BD no silencer shots in AQ2!
	if (deathmatch->value)
		return;

	if (who->flags & FL_NOTARGET)
		return;


	if (!who->mynoise)
	{
		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise = noise;

		noise = G_Spawn();
		noise->classname = "player_noise";
		VectorSet (noise->mins, -8, -8, -8);
		VectorSet (noise->maxs, 8, 8, 8);
		noise->owner = who;
		noise->svflags = SVF_NOCLIENT;
		who->mynoise2 = noise;
	}

	if (type == PNOISE_SELF || type == PNOISE_WEAPON)
	{
		noise = who->mynoise;
		level.sound_entity = noise;
		level.sound_entity_framenum = level.framenum;
	}
	else // type == PNOISE_IMPACT
	{
		noise = who->mynoise2;
		level.sound2_entity = noise;
		level.sound2_entity_framenum = level.framenum;
	}

	VectorCopy (where, noise->s.origin);
	VectorSubtract (where, noise->maxs, noise->absmin);
	VectorAdd (where, noise->maxs, noise->absmax);
//	noise->teleport_time = level.time;
	gi.linkentity (noise);
}


qboolean Pickup_Weapon (edict_t *ent, edict_t *other)
{
	int			index;
	gitem_t		*ammo;

	index = ITEM_INDEX(ent->item);

	if ( ( ((int)(dmflags->value) & DF_WEAPONS_STAY) || coop->value)
		&& other->client->pers.inventory[index])
	{
		if (!(ent->spawnflags & (DROPPED_ITEM | DROPPED_PLAYER_ITEM) ) )
			return false;	// leave the weapon for others to pickup
	}

	other->client->pers.inventory[index]++;

	if (!(ent->spawnflags & DROPPED_ITEM) )
	{
		// give them some ammo with it
		ammo = FindItem (ent->item->ammo);
		if ( (int)dmflags->value & DF_INFINITE_AMMO )
			Add_Ammo (other, ammo, 1000);
		else
			Add_Ammo (other, ammo, ammo->quantity);

		if (! (ent->spawnflags & DROPPED_PLAYER_ITEM) )
		{
			if (deathmatch->value)
			{
				if ((int)(dmflags->value) & DF_WEAPONS_STAY)
					ent->flags |= FL_RESPAWN;
				else
					SetRespawn (ent, 30);
			}
			if (coop->value)
				ent->flags |= FL_RESPAWN;
		}
	}

	if (other->client->pers.weapon != ent->item &&
		(other->client->pers.inventory[index] == 1) &&
		( !deathmatch->value || other->client->pers.weapon == FindItem("blaster") ) )
		other->client->newweapon = ent->item;

	return true;
}

/*
===============
Activate Secondary

Prepares Secondary Weapon for use!
===============
*/
void ActivateSecondary( edict_t *ent, gitem_t *it )
{
	if( ent->vehicle->ONOFF & IS_JAMMING )
	{
		ent->vehicle->ONOFF &= ~IS_JAMMING;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Jammer"))] = 0;
		gi.cprintf( ent, PRINT_MEDIUM, "Jammer OFF!\n" );
	}
	if( strcmp( it->pickup_name, "SW Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "ST Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "AM Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "PH Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "ECM" ) == 0 )
	{
		ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "ATGM Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "Maverick Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "Iron Bombs" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "Laser Bombs" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "MainGun" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "Rocket Launcher" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] )
			ent->client->pers.secondary = it;
	}
	else if( strcmp( it->pickup_name, "AutoCannon" ) == 0 )
	{
		if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] )
			ent->client->pers.secondary = it;
	}
	ent->client->ammo_index2 = ITEM_INDEX(FindItem(ent->client->pers.secondary->ammo));
	ent->client->pickle_time = level.time + 0.5;
}


/*
===============
ChangeWeapon

The old weapon has been dropped all the way, so make the new one
current
===============
*/
void ChangeWeapon (edict_t *ent)
{
/*	if (ent->client->grenade_time)
	{
		ent->client->grenade_time = level.time;
		ent->client->weapon_sound = 0;
		weapon_grenade_fire (ent, false);
		ent->client->grenade_time = 0;
	}*/

	ent->client->pers.lastweapon = ent->client->pers.weapon;
	ent->client->pers.weapon = ent->client->newweapon;
	ent->client->newweapon = NULL;
	ent->client->machinegun_shots = 0;

	if (ent->client->pers.weapon && ent->client->pers.weapon->ammo)
		ent->client->ammo_index = ITEM_INDEX(FindItem(ent->client->pers.weapon->ammo));
	else
		ent->client->ammo_index = 0;

	if (!ent->client->pers.weapon)
	{	// dead
		ent->client->ps.gunindex = 0;
		return;
	}

	ent->client->weaponstate = WEAPON_ACTIVATING;
	ent->client->ps.gunframe = 0;
	ent->client->ps.gunindex = 0; // +BD old: gi.modelindex(ent->client->pers.weapon->view_model);
}

/*
=================
NoAmmoWeaponChange
=================
*/

void NoAmmoWeaponChange (edict_t *ent)
{
	// Phoenix->AMRAAM->Sidewinder->Stinger->ATGM->Maverick->FFAR->LGB->Bomb->Shell->Autocannon->ECM
	if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("PH Launcher"))] )
	{
		ent->client->pers.secondary = FindItem ("PH Launcher");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("AM Launcher"))] )
	{
		ent->client->pers.secondary = FindItem ("AM Launcher");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("SW Launcher"))] )
	{
		ent->client->pers.secondary = FindItem ("SW Launcher");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("ST Launcher"))] )
	{
		ent->client->pers.secondary = FindItem ("ST Launcher");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM Launcher"))] )
	{
		ent->client->pers.secondary = FindItem ("ATGM Launcher");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick Launcher"))] )
	{
		ent->client->pers.secondary = FindItem ("Maverick Launcher");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("rockets"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("rocket launcher"))] )
	{
		ent->client->pers.secondary = FindItem ("rocket launcher");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("Laser bombs"))] )
	{
		ent->client->pers.secondary = FindItem ("Laser bombs");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("bombs"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("Iron bombs"))] )
	{
		ent->client->pers.secondary = FindItem ("Iron bombs");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("MainGun"))]
		&& ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] )
	{
		ent->client->pers.secondary = FindItem ("MainGun");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]
		&&  ent->client->pers.inventory[ITEM_INDEX(FindItem("AutoCannon"))]
		&& ent->vehicle->HASCAN & HAS_AUTOAIM )
	{
		ent->client->pers.secondary = FindItem ("AutoCannon");
	}
	else if ( ent->client->pers.inventory[ITEM_INDEX(FindItem("ECM"))] )
	{
		ent->client->pers.secondary = FindItem ("ECM");
	}
	ent->client->ammo_index2 = ITEM_INDEX(FindItem(ent->client->pers.secondary->ammo));
}

/*
=================
Think_Weapon

Called by ClientBeginServerFrame and ClientThink
=================
*/
void Think_Weapon (edict_t *ent)
{

	// if just died, put the weapon away
/*	if (ent->health < 1)	// +BD block removed
	{
		ent->client->newweapon = NULL;
		ChangeWeapon (ent);
	}*/

	// call active weapon think routine
	if (ent->client->pers.weapon && ent->client->pers.weapon->weaponthink)
	{
//		is_quad = (ent->client->quad_framenum > level.framenum);	// +BD no quad in AQ2
/*		if (ent->client->silencer_shots)
			is_silenced = MZ_SILENCED;
		else
			is_silenced = 0;*/ // +BD no silencer in AQ2

		ent->client->pers.weapon->weaponthink (ent);
	}
}


/*
================
Use_Secondary

Make the secondary weapon ready if there is ammo
================
*/
void Use_Secondary (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.secondary )
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
//			gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
//			gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->pers.secondary = item;
	ent->client->ammo_index2 = ITEM_INDEX(FindItem(item->ammo));

	if( ent->vehicle )
	{
		if( ent->vehicle->ONOFF & IS_JAMMING )
		{
			ent->vehicle->ONOFF &= ~IS_JAMMING;
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Jammer"))] = 0;
			gi.cprintf( ent, PRINT_MEDIUM, "Jammer OFF!\n" );
		}
		if( ent->vehicle->enemy )
		{
			ent->vehicle->enemy->lockstatus = 0;
			ent->vehicle->enemy = NULL;
		}
	}
}




/*
================
Use_Weapon

Make the weapon ready if there is ammo
================
*/
void Use_Weapon (edict_t *ent, gitem_t *item)
{
	int			ammo_index;
	gitem_t		*ammo_item;

	// see if we're already using it
	if (item == ent->client->pers.weapon )
		return;

	if (item->ammo && !g_select_empty->value && !(item->flags & IT_AMMO))
	{
		ammo_item = FindItem(item->ammo);
		ammo_index = ITEM_INDEX(ammo_item);

		if (!ent->client->pers.inventory[ammo_index])
		{
//			gi.cprintf (ent, PRINT_HIGH, "No %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}

		if (ent->client->pers.inventory[ammo_index] < item->quantity)
		{
//			gi.cprintf (ent, PRINT_HIGH, "Not enough %s for %s.\n", ammo_item->pickup_name, item->pickup_name);
			return;
		}
	}

	// change to this weapon when down
	ent->client->newweapon = item;

	if( ent->vehicle )
	{
		if( ent->vehicle->ONOFF & IS_JAMMING )
		{
			ent->vehicle->ONOFF &= ~IS_JAMMING;
			ent->client->pers.inventory[ITEM_INDEX(FindItem("Jammer"))] = 0;
			gi.cprintf( ent, PRINT_MEDIUM, "Jammer OFF!\n" );
		}
		if( ent->vehicle->enemy )
		{
			ent->vehicle->enemy->lockstatus = 0;
			ent->vehicle->enemy = NULL;
		}
	}
}



/*
================
Drop_Weapon
================
*/
void Drop_Weapon (edict_t *ent, gitem_t *item)
{
	int		index;

	if ((int)(dmflags->value) & DF_WEAPONS_STAY)
		return;

	index = ITEM_INDEX(item);
	// see if we're already using it
	if ( ((item == ent->client->pers.weapon) || (item == ent->client->newweapon))&& (ent->client->pers.inventory[index] == 1) )
	{
		gi.cprintf (ent, PRINT_HIGH, "Can't drop current weapon\n");
		return;
	}

	Drop_Item (ent, item);
	ent->client->pers.inventory[index]--;
}


/*
================
Weapon_Generic

A generic function to handle the basics of weapon thinking
================
*/
#define FRAME_FIRE_FIRST		(FRAME_ACTIVATE_LAST + 1)
#define FRAME_IDLE_FIRST		(FRAME_FIRE_LAST + 1)
#define FRAME_DEACTIVATE_FIRST	(FRAME_IDLE_LAST + 1)

void Weapon_Generic (edict_t *ent, int FRAME_ACTIVATE_LAST, int FRAME_FIRE_LAST, int FRAME_IDLE_LAST, int FRAME_DEACTIVATE_LAST, int *pause_frames, int *fire_frames, void (*fire)(edict_t *ent))
{
	int		n;

	if (ent->client->weaponstate == WEAPON_DROPPING)
	{
		if (ent->client->ps.gunframe == FRAME_DEACTIVATE_LAST)
		{
			ChangeWeapon (ent);
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		if (ent->client->ps.gunframe == FRAME_ACTIVATE_LAST)
		{
			ent->client->weaponstate = WEAPON_READY;
			ent->client->ps.gunframe = FRAME_IDLE_FIRST;
			return;
		}

		ent->client->ps.gunframe++;
		return;
	}

	if ((ent->client->newweapon) && (ent->client->weaponstate != WEAPON_FIRING))
	{
		ent->client->weaponstate = WEAPON_DROPPING;
		ent->client->ps.gunframe = FRAME_DEACTIVATE_FIRST;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if ((!ent->client->ammo_index) ||
				( ent->client->pers.inventory[ent->client->ammo_index] >= 1))
//				( ent->client->pers.inventory[ent->client->ammo_index] >= ent->client->pers.weapon->quantity))
			{
/*				if( strcmp(ent->client->pers.weapon->pickup_name, "Iron Bombs") == 0 )
				{
					if( ent->vehicle->ripple_b >
							ent->client->pers.inventory[ent->client->ammo_index] )
					{
						if( ent->client->pers.inventory[ent->client->ammo_index] == 1 )
							ent->vehicle->ripple_b = 1;
						else
							ent->vehicle->ripple_b = 2;
					}
				}
				else if( strcmp(ent->client->pers.weapon->pickup_name, "Rocket Launcher") == 0 )
				{
					if( ent->vehicle->ripple_f >
							ent->client->pers.inventory[ent->client->ammo_index] )
					{
						if( ent->client->pers.inventory[ent->client->ammo_index] == 1 )
							ent->vehicle->ripple_f = 1;
						else
							ent->vehicle->ripple_f = 2;
					}
				}*/

				ent->client->ps.gunframe = FRAME_FIRE_FIRST;
				ent->client->weaponstate = WEAPON_FIRING;

				// start the animation
				ent->client->anim_priority = ANIM_ATTACK;
				if (ent->client->ps.pmove.pm_flags & PMF_DUCKED)
				{
					ent->s.frame = FRAME_crattak1-1;
					ent->client->anim_end = FRAME_crattak9;
				}
				else
				{
					ent->s.frame = FRAME_attack1-1;
					ent->client->anim_end = FRAME_attack8;
				}
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
//				NoAmmoWeaponChange (ent);
			}
		}
		else
		{
			if (ent->client->ps.gunframe == FRAME_IDLE_LAST)
			{
				ent->client->ps.gunframe = FRAME_IDLE_FIRST;
				return;
			}

			if (pause_frames)
			{
				for (n = 0; pause_frames[n]; n++)
				{
					if (ent->client->ps.gunframe == pause_frames[n])
					{
						if (rand()&15)
							return;
					}
				}
			}

			ent->client->ps.gunframe++;
			return;
		}
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		for (n = 0; fire_frames[n]; n++)
		{
			if (ent->client->ps.gunframe == fire_frames[n])
			{
				if (ent->client->quad_framenum > level.framenum)
					gi.sound(ent, CHAN_ITEM, gi.soundindex("items/damage3.wav"), 1, ATTN_NORM, 0);

				fire (ent);
				break;
			}
		}

		if (!fire_frames[n])
			ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == FRAME_IDLE_FIRST+1)
			ent->client->weaponstate = WEAPON_READY;
	}
}


/*
======================================================================

GRENADE

======================================================================
*/

#define GRENADE_TIMER		3.0
#define GRENADE_MINSPEED	400
#define GRENADE_MAXSPEED	800

void weapon_grenade_fire (edict_t *ent, qboolean held)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 125;
	float	timer;
	int		speed;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

//	timer = ent->client->grenade_time - level.time;
	speed = GRENADE_MINSPEED + (GRENADE_TIMER - timer) * ((GRENADE_MAXSPEED - GRENADE_MINSPEED) / GRENADE_TIMER);
	fire_grenade2 (ent, start, forward, damage, speed, timer, radius, held);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;

//	ent->client->grenade_time = level.time + 1.0;
}

void Weapon_Grenade (edict_t *ent)
{
	if ((ent->client->newweapon) && (ent->client->weaponstate == WEAPON_READY))
	{
		ChangeWeapon (ent);
		return;
	}

	if (ent->client->weaponstate == WEAPON_ACTIVATING)
	{
		ent->client->weaponstate = WEAPON_READY;
		ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_READY)
	{
		if ( ((ent->client->latched_buttons|ent->client->buttons) & BUTTON_ATTACK) )
		{
			ent->client->latched_buttons &= ~BUTTON_ATTACK;
			if (ent->client->pers.inventory[ent->client->ammo_index])
			{
				ent->client->ps.gunframe = 1;
				ent->client->weaponstate = WEAPON_FIRING;
//				ent->client->grenade_time = 0;
			}
			else
			{
				if (level.time >= ent->pain_debounce_time)
				{
					gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
					ent->pain_debounce_time = level.time + 1;
				}
//				NoAmmoWeaponChange (ent);
			}
			return;
		}

		if ((ent->client->ps.gunframe == 29) || (ent->client->ps.gunframe == 34) || (ent->client->ps.gunframe == 39) || (ent->client->ps.gunframe == 48))
		{
			if (rand()&15)
				return;
		}

		if (++ent->client->ps.gunframe > 48)
			ent->client->ps.gunframe = 16;
		return;
	}

	if (ent->client->weaponstate == WEAPON_FIRING)
	{
		if (ent->client->ps.gunframe == 5)
			gi.sound(ent, CHAN_WEAPON, gi.soundindex("weapons/hgrena1b.wav"), 1, ATTN_NORM, 0);

		if (ent->client->ps.gunframe == 11)
		{
/*			if (!ent->client->grenade_time)
			{
				ent->client->grenade_time = level.time + GRENADE_TIMER + 0.2;
				ent->client->weapon_sound = gi.soundindex("weapons/hgrenc1b.wav");
			}
*/
			// they waited too long, detonate it in their hand
/*			if (!ent->client->grenade_blew_up && level.time >= ent->client->grenade_time)
			{
				ent->client->weapon_sound = 0;
				weapon_grenade_fire (ent, true);
				ent->client->grenade_blew_up = true;
			}
*/
			if (ent->client->buttons & BUTTON_ATTACK)
				return;

/*			if (ent->client->grenade_blew_up)
			{
				if (level.time >= ent->client->grenade_time)
				{
					ent->client->ps.gunframe = 15;
					ent->client->grenade_blew_up = false;
				}
				else
				{
					return;
				}
			}*/
		}

		if (ent->client->ps.gunframe == 12)
		{
			ent->client->weapon_sound = 0;
			weapon_grenade_fire (ent, false);
		}

/*		if ((ent->client->ps.gunframe == 15) && (level.time < ent->client->grenade_time))
			return;
*/
		ent->client->ps.gunframe++;

		if (ent->client->ps.gunframe == 16)
		{
//			ent->client->grenade_time = 0;
			ent->client->weaponstate = WEAPON_READY;
		}
	}
}

/*
======================================================================

GRENADE LAUNCHER

======================================================================
*/

void weapon_grenadelauncher_fire (edict_t *ent)
{
	vec3_t	offset;
	vec3_t	forward, right;
	vec3_t	start;
	int		damage = 120;
	float	radius;

	radius = damage+40;
	if (is_quad)
		damage *= 4;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	AngleVectors (ent->client->v_angle, forward, right, NULL);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -1;

	fire_grenade (ent, start, forward, damage, 600, 2.5, radius);

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_GRENADE | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_GrenadeLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {34, 51, 59, 0};
	static int	fire_frames[]	= {6, 0};

	Weapon_Generic (ent, 5, 16, 59, 64, pause_frames, fire_frames, weapon_grenadelauncher_fire);
}

/*
======================================================================

ROCKET

======================================================================
*/

void Weapon_RocketLauncher_Fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage;
	float	damage_radius;

	damage = 45 + (int)(random() * 20.0)-10;
	damage_radius = 100;


	VectorCopy( ent->vehicle->s.angles, angles );
	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	// +BD this block will change the side where the rocket starts
	VectorCopy( ent->vehicle->s.origin, start );
	VectorScale( up, -3, up );
	if( strcmp( ent->vehicle->classname, "plane" ) == 0 )
	{
		VectorScale( right, 12, right );
	}
	else
	{
		VectorScale( right, 4, right );
	}

	if( strcmp( ent->vehicle->classname, "plane" ) == 0 ||
					strcmp( ent->vehicle->classname, "helo" ) == 0 )
	{
		ent->client->side = ent->client->side ? 0 : 1;
		if( ent->client->side == 1 )
			VectorInverse(right);
		if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY) ||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
			VectorAdd( start, right, start );
		VectorAdd( start, up, start );
	}

//	VectorSet(offset, 8, 8, ent->viewheight-8);

	fire_rocket( ent->vehicle, start, forward, damage,
			1800, damage_radius ); // +BD


	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle


	ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))]--;

	ent->client->pickle_time = level.time + 0.3;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] < 1 )
		NoAmmoWeaponChange( ent );
// 	ent->client->ps.gunframe++;
}

/*
void Weapon_RocketLauncher (edict_t *ent)
{
	static int	pause_frames[]	= {25, 33, 42, 50, 0};
	static int	fire_frames[]	= {5, 0};

	// +BD second number changed from 12 to 9
	Weapon_Generic (ent, 4, 9, 50, 54, pause_frames, fire_frames, Weapon_RocketLauncher_Fire);
}
*/

/*
======================================================================

BLASTER / HYPERBLASTER

======================================================================
*/

void Blaster_Fire (edict_t *ent, vec3_t g_offset, int damage, qboolean hyper, int effect)
{
	vec3_t	forward, right, right2;
	vec3_t	start, start2;
	vec3_t	angles;		// used for spread
	vec3_t	direction;
	vec3_t  forward2;
	vec3_t	aimatthis;
	float	targetspeed;
	float	timetotarget;

	if( level.time < ent->client->pickle_time )
		return;

	if( (ent->vehicle->HASCAN & HAS_AUTOAIM) && ent->vehicle->enemy &&
		strcmp(ent->client->pers.secondary->pickup_name, "AutoCannon") == 0 )
	{
		AngleVectors( ent->vehicle->enemy->s.angles, forward, NULL, NULL );
		VectorSubtract( ent->vehicle->enemy->s.origin, ent->vehicle->s.origin, angles );
		timetotarget = VectorLength( angles )/(2400*FRAMETIME);
		targetspeed = (VectorLength( ent->vehicle->enemy->velocity )*FRAMETIME)*timetotarget;
		VectorMA( ent->vehicle->enemy->s.origin, targetspeed, forward, aimatthis );
		VectorSubtract( aimatthis, ent->vehicle->s.origin, direction );
		vectoangles( direction, angles );
	}
	else
	{
		if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
			VectorCopy( ent->vehicle->movetarget->s.angles, angles );
		else
			VectorCopy( ent->vehicle->s.angles, angles );
	}

	// add huge spread for ground vehicles
	if( strcmp( ent->vehicle->classname, "ground" ) == 0 ||
		strcmp( ent->vehicle->classname, "plane" ) == 0 )
	{
		angles[0] += (random()*4)-2;
		angles[1] += (random()*4)-2;
	}
	// and smaller spread for all the others
	else
	{
		if( ent->vehicle->HASCAN & HAS_AUTOAIM )
		{
			angles[0] += (random()*2.5)-1.25;
			angles[1] += (random()*2.5)-1.25;
		}
		else
		{
			angles[0] += (random()*1.2)-0.6;
			angles[1] += (random()*1.2)-0.6;
		}
	}
	AngleVectors( angles, forward, right, NULL );

/*	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
		AngleVectors( ent->vehicle->movetarget->s.angles, forward, right, NULL );
	else
		AngleVectors ( ent->vehicle->s.angles, forward, right, NULL); // +BD new
*/
	VectorCopy( ent->vehicle->s.origin, start );
	VectorCopy( forward, forward2 );
	VectorScale( forward2, 25, forward2 );
	VectorAdd( ent->vehicle->s.origin, forward2, start );
	start[2] += 2;

	// set the start position
	if( strcmp( ent->vehicle->classname, "ground" ) == 0 &&
		ent->veh_class == 4 && ent->veh_number == 1 )	// if Tunguska then fire twice
	{
		VectorScale( right, 4.5, right );
		VectorCopy( right, right2 );
		VectorCopy( start, start2 );
		VectorInverse(right2);
		VectorAdd( start, right2, start2 );
		VectorAdd( start, right, start );
		fire_blaster (ent->vehicle, start, forward,
			damage, 2400, effect, hyper );// +BD new version
		fire_blaster (ent->vehicle, start2, forward,
			damage, 2400, effect, hyper );// +BD new version
		// substract the additional ammo
		ent->client->pers.inventory[ent->client->ammo_index]--;
	}
	else
	{
		fire_blaster (ent->vehicle, start, forward,
			damage, 2400, effect, hyper );// +BD new version
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);	// +BD +vehicle
	if (hyper)
		gi.WriteByte (MZ_CHAINGUN3); 	// +BD instead of HYPERBLASTER
	else
		gi.WriteByte (MZ_CHAINGUN1);	// +BD instead of BLASTER
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);	// +BD +vehicle

	ent->client->pickle_time = level.time + ent->vehicle->gunfirerate;
}


void Weapon_Blaster_Fire (edict_t *ent)
{
	int		damage = 0;

	// +BD new block
	{
		if( !ent->vehicle->flare )
			return;
		else
			ent->vehicle->flare --;
	}

	Flare_Fire (ent, vec3_origin, damage, false, EF_BLASTER);
//	ent->client->ps.gunframe++;		// +BD removed as this is secondary weapon
}

void Weapon_Blaster (edict_t *ent)
{
	static int	pause_frames[]	= {19, 32, 0};
	static int	fire_frames[]	= {5, 0};

	Weapon_Generic (ent, 4, 8, 52, 55, pause_frames, fire_frames, Weapon_Blaster_Fire);
}

void Weapon_Autocannon_Fire (edict_t *ent);

void Weapon_HyperBlaster_Fire (edict_t *ent)
{
//	float	rotation;
//	vec3_t	offset;
//	int		effect;
//	int		damage;


	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe++;
	}
	else
	{
		if (! ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] )
		{
//			gi.bprintf( PRINT_HIGH,"%s\n",ent->client->pers.weapon->pickup_name );
			if (level.time >= ent->pain_debounce_time)
			{
//				gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
				ent->pain_debounce_time = level.time + 1;
			}
//			NoAmmoWeaponChange (ent);
		}
		else
		{
/*			if ((ent->client->ps.gunframe == 6) ||
						(ent->client->ps.gunframe == 9))
				effect = EF_HYPERBLASTER;
			else
				effect = 0;
			damage = ent->vehicle->gundamage;
*/
			Weapon_Autocannon_Fire (ent);
//			if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
//				ent->client->pers.inventory[ent->client->ammo_index]--;
		}

		ent->client->ps.gunframe++;
		if (ent->client->ps.gunframe == 12 && ent->client->pers.inventory[ent->client->ammo_index])
			ent->client->ps.gunframe = 6;
	}

	if (ent->client->ps.gunframe == 12)
	{
//		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/hyprbd1a.wav"), 1, ATTN_NORM, 0);
		ent->client->weapon_sound = 0;
	}

}


void Weapon_HyperBlaster (edict_t *ent)
{
	static int	pause_frames[]	= {0};
	static int	fire_frames[]	= {6, 7, 8, 9, 10, 11, 0};

	Weapon_Generic (ent, 5, 8, 49, 53, pause_frames, fire_frames, Weapon_HyperBlaster_Fire);
	// +BD second number changed from 20 to 8
}

/*
======================================================================

MACHINEGUN / CHAINGUN

======================================================================
*/

void Machinegun_Fire (edict_t *ent)
{
//	int	i;
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		point;
	int			kick = 2;
	vec3_t		offset;

	if (!(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->machinegun_shots = 0;
		ent->client->ps.gunframe++;
		return;
	}

	if (ent->client->ps.gunframe == 5)
		ent->client->ps.gunframe = 4;
	else
		ent->client->ps.gunframe = 5;

	if (ent->client->pers.inventory[ent->client->ammo_index] < 1)
	{
		ent->client->ps.gunframe = 6;
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
//		NoAmmoWeaponChange (ent);
		return;
	}

/*	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}*/		// +BD no QUAD in AQ2!
/*
	for (i=1 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = 0; // +BD changed from crandom() * 0.35;
		ent->client->kick_angles[i] = 0; // +BD changed from crandom() * 0.7;
	}
	ent->client->kick_origin[0] = 0;	// +BD changed from crandom() * 0.35;
	ent->client->kick_angles[0] = 0;	// +BD changed from ent->client->machinegun_shots * -1.5;
*/
	// raise the gun as it is firing
	if (!deathmatch->value)
	{
		ent->client->machinegun_shots++;
		if (ent->client->machinegun_shots > 9)
			ent->client->machinegun_shots = 9;
	}

	// get start / end positions
//	VectorAdd (ent->client->v_angle, ent->client->kick_angles, angles);
//	AngleVectors (angles, forward, right, NULL);
//	+BD the new one:

	VectorSet(offset, 0, 8, ent->vehicle->viewheight-18);

	if( strcmp( ent->vehicle->classname, "plane" ) == 0 ||
		strcmp( ent->vehicle->classname, "helo" ) == 0 )
	{
		AngleVectors (ent->vehicle->s.angles, forward, right, NULL );
		VectorMA(  ent->vehicle->s.origin, ent->vehicle->maxs[0], forward, point );
	}
	else
	{
		AngleVectors (ent->vehicle->movetarget->s.angles, forward, right, NULL );
		VectorMA(  ent->vehicle->movetarget->s.origin, ent->vehicle->maxs[0], forward, point );
	}
	point[2] += 12;

	P_ProjectSource (ent->client, point, offset, forward, right, start);

//	fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_MACHINEGUN);
//	+BD the new one:
	fire_bullet( ent->vehicle, start, forward, ent->vehicle->gundamage, kick,
		DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_AUTOCANNON );

	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);// +BD +vehicle
	gi.WriteByte (MZ_MACHINEGUN | is_silenced);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);	// +BD +vehicle

	PlayerNoise(ent->vehicle, start, PNOISE_WEAPON); // +BD +vehicle

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Machinegun (edict_t *ent)
{
	static int	pause_frames[]	= {23, 45, 0};
	static int	fire_frames[]	= {4, 5, 0};

	Weapon_Generic (ent, 3, 5, 45, 49, pause_frames, fire_frames, Machinegun_Fire);
}

void Chaingun_Fire (edict_t *ent)
{
	int			i;
	int			shots;
	vec3_t		start;
	vec3_t		forward, right, up;
	float		r, u;
	vec3_t		offset;
	int			damage;
	int			kick = 2;

	if (deathmatch->value)
		damage = 6;
	else
		damage = 8;

	if (ent->client->ps.gunframe == 5)
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnu1a.wav"), 1, ATTN_IDLE, 0);

	if ((ent->client->ps.gunframe == 14) && !(ent->client->buttons & BUTTON_ATTACK))
	{
		ent->client->ps.gunframe = 32;
		ent->client->weapon_sound = 0;
		return;
	}
	else if ((ent->client->ps.gunframe == 21) && (ent->client->buttons & BUTTON_ATTACK)
		&& ent->client->pers.inventory[ent->client->ammo_index])
	{
		ent->client->ps.gunframe = 15;
	}
	else
	{
		ent->client->ps.gunframe++;
	}

	if (ent->client->ps.gunframe == 22)
	{
		ent->client->weapon_sound = 0;
		gi.sound(ent, CHAN_AUTO, gi.soundindex("weapons/chngnd1a.wav"), 1, ATTN_IDLE, 0);
	}
	else
	{
		ent->client->weapon_sound = gi.soundindex("weapons/chngnl1a.wav");
	}

	if (ent->client->ps.gunframe <= 9)
		shots = 1;
	else if (ent->client->ps.gunframe <= 14)
	{
		if (ent->client->buttons & BUTTON_ATTACK)
			shots = 2;
		else
			shots = 1;
	}
	else
		shots = 3;

	if (ent->client->pers.inventory[ent->client->ammo_index] < shots)
		shots = ent->client->pers.inventory[ent->client->ammo_index];

	if (!shots)
	{
		if (level.time >= ent->pain_debounce_time)
		{
			gi.sound(ent, CHAN_VOICE, gi.soundindex("weapons/noammo.wav"), 1, ATTN_NORM, 0);
			ent->pain_debounce_time = level.time + 1;
		}
//		NoAmmoWeaponChange (ent);
		return;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	for (i=0 ; i<3 ; i++)
	{
		ent->client->kick_origin[i] = crandom() * 0.35;
		ent->client->kick_angles[i] = crandom() * 0.7;
	}

	for (i=0 ; i<shots ; i++)
	{
		// get start / end positions
		AngleVectors (ent->client->v_angle, forward, right, up);
		r = 7 + crandom()*4;
		u = crandom()*4;
		VectorSet(offset, 0, r, u + ent->viewheight-8);
		P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

		fire_bullet (ent, start, forward, damage, kick, DEFAULT_BULLET_HSPREAD, DEFAULT_BULLET_VSPREAD, MOD_CHAINGUN);
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte ((MZ_CHAINGUN1 + shots - 1) | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= shots;
}


void Weapon_Chaingun (edict_t *ent)
{
	static int	pause_frames[]	= {38, 43, 51, 61, 0};
	static int	fire_frames[]	= {5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 0};

	Weapon_Generic (ent, 4, 31, 61, 64, pause_frames, fire_frames, Chaingun_Fire);
}


/*
======================================================================

SHOTGUN / SUPERSHOTGUN

======================================================================
*/

void weapon_shotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage = 4;
	int			kick = 8;

	if (ent->client->ps.gunframe == 9)
	{
		ent->client->ps.gunframe++;
		return;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	if (deathmatch->value)
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_DEATHMATCH_SHOTGUN_COUNT, MOD_SHOTGUN);
	else
		fire_shotgun (ent, start, forward, damage, kick, 500, 500, DEFAULT_SHOTGUN_COUNT, MOD_SHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}

void Weapon_Shotgun (edict_t *ent)
{
	static int	pause_frames[]	= {22, 28, 34, 0};
	static int	fire_frames[]	= {8, 9, 0};

	Weapon_Generic (ent, 7, 18, 36, 39, pause_frames, fire_frames, weapon_shotgun_fire);
}


void weapon_supershotgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	vec3_t		v;
	int			damage = 6;
	int			kick = 12;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);
	ent->client->kick_angles[0] = -2;

	VectorSet(offset, 0, 8,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	v[PITCH] = ent->client->v_angle[PITCH];
	v[YAW]   = ent->client->v_angle[YAW] - 5;
	v[ROLL]  = ent->client->v_angle[ROLL];
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);
	v[YAW]   = ent->client->v_angle[YAW] + 5;
	AngleVectors (v, forward, NULL, NULL);
	fire_shotgun (ent, start, forward, damage, kick, DEFAULT_SHOTGUN_HSPREAD, DEFAULT_SHOTGUN_VSPREAD, DEFAULT_SSHOTGUN_COUNT/2, MOD_SSHOTGUN);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_SSHOTGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 2;
}

void Weapon_SuperShotgun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 17, 57, 61, pause_frames, fire_frames, weapon_supershotgun_fire);
}



/*
======================================================================

RAILGUN

======================================================================
*/

void weapon_railgun_fire (edict_t *ent)
{
	vec3_t		start;
	vec3_t		forward, right;
	vec3_t		offset;
	int			damage;
	int			kick;

	if (deathmatch->value)
	{	// normal damage is too extreme in dm
		damage = 100;
		kick = 200;
	}
	else
	{
		damage = 150;
		kick = 250;
	}

	if (is_quad)
	{
		damage *= 4;
		kick *= 4;
	}

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -3, ent->client->kick_origin);
	ent->client->kick_angles[0] = -3;

	VectorSet(offset, 0, 7,  ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_rail (ent, start, forward, damage, kick);

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent-g_edicts);
	gi.WriteByte (MZ_RAILGUN | is_silenced);
	gi.multicast (ent->s.origin, MULTICAST_PVS);

	ent->client->ps.gunframe++;
	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index]--;
}


void Weapon_Railgun (edict_t *ent)
{
	static int	pause_frames[]	= {56, 0};
	static int	fire_frames[]	= {4, 0};

	Weapon_Generic (ent, 3, 18, 56, 61, pause_frames, fire_frames, weapon_railgun_fire);
}


/*
======================================================================

BFG10K

======================================================================
*/

void weapon_bfg_fire (edict_t *ent)
{
	vec3_t	offset, start;
	vec3_t	forward, right;
	int		damage;
	float	damage_radius = 1000;

	if (deathmatch->value)
		damage = 200;
	else
		damage = 500;

	if (ent->client->ps.gunframe == 9)
	{
		// send muzzle flash
		gi.WriteByte (svc_muzzleflash);
		gi.WriteShort (ent-g_edicts);
		gi.WriteByte (MZ_BFG | is_silenced);
		gi.multicast (ent->s.origin, MULTICAST_PVS);

		ent->client->ps.gunframe++;

		PlayerNoise(ent, start, PNOISE_WEAPON);
		return;
	}

	// cells can go down during windup (from power armor hits), so
	// check again and abort firing if we don't have enough now
	if (ent->client->pers.inventory[ent->client->ammo_index] < 50)
	{
		ent->client->ps.gunframe++;
		return;
	}

	if (is_quad)
		damage *= 4;

	AngleVectors (ent->client->v_angle, forward, right, NULL);

	VectorScale (forward, -2, ent->client->kick_origin);

	// make a big pitch kick with an inverse fall
//	ent->client->v_dmg_pitch = -40;
//	ent->client->v_dmg_roll = crandom()*8;
//	ent->client->v_dmg_time = level.time + DAMAGE_TIME;

	VectorSet(offset, 8, 8, ent->viewheight-8);
	P_ProjectSource (ent->client, ent->s.origin, offset, forward, right, start);
	fire_bfg (ent, start, forward, damage, 400, damage_radius);

	ent->client->ps.gunframe++;

	PlayerNoise(ent, start, PNOISE_WEAPON);

	if (! ( (int)dmflags->value & DF_INFINITE_AMMO ) )
		ent->client->pers.inventory[ent->client->ammo_index] -= 50;
}

void Weapon_BFG (edict_t *ent)
{
	static int	pause_frames[]	= {39, 45, 50, 55, 0};
	static int	fire_frames[]	= {9, 17, 0};

	Weapon_Generic (ent, 8, 32, 55, 58, pause_frames, fire_frames, weapon_bfg_fire);
}


// =======================================

// Anti Aircraft Missiles (AAM)

// =======================================


void Weapon_Sidewinder_Fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage;
	float	damage_radius;
	int 	vehicleside, vehicleup;	// +BD where do sidewinders start

	ent->client->side = ent->client->side ? 0 : 1;	// +BD

	if( strcmp(ent->vehicle->classname, "plane") == 0 )
	{
		vehicleside = 16;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		vehicleside = 6;
		vehicleup = 2;
		VectorCopy( ent->vehicle->movetarget->s.angles, angles );
		VectorCopy( ent->vehicle->movetarget->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "helo") == 0 )
	{
		vehicleside = 9;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}

	damage = 60 + (int)(random() * 80.0)-20;
	damage_radius = 150;

	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		VectorScale( forward, 20, forward );
		VectorAdd( start, forward, start );
		VectorNormalize( forward );
	}

	VectorScale( right, vehicleside, right );
	VectorScale( up, vehicleup, up );

	if( ent->client->side )
		VectorInverse(right);
	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY)||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_sidewinder( ent->vehicle, start, forward, damage,
		900, damage_radius, 150, 0 ); // +BD (altspeed 650)


	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle

//	ent->client->ps.gunframe++;

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))]--;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 1.2;
}


void Weapon_Stinger_Fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage;
	float	damage_radius;
	int 	vehicleside, vehicleup;	// +BD where do sidewinders start

	ent->client->side = ent->client->side ? 0 : 1;	// +BD

	if( strcmp(ent->vehicle->classname, "plane") == 0 )
	{
		vehicleside = 16;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		vehicleside = 6;
		vehicleup = 2;
		VectorCopy( ent->vehicle->movetarget->s.angles, angles );
		VectorCopy( ent->vehicle->movetarget->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "helo") == 0 )
	{
		vehicleside = 9;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}

	damage = 40 + (int)(random() * 50.0)-20;
	damage_radius = 150;

	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		VectorScale( forward, 20, forward );
		VectorAdd( start, forward, start );
		VectorNormalize( forward );
	}

	VectorScale( right, vehicleside, right );
	VectorScale( up, vehicleup, up );

	if( ent->client->side )
		VectorInverse(right);
	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY)||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_sidewinder( ent->vehicle, start, forward, damage,
		900, damage_radius, 150, 1 ); // +BD (altspeed 650)


	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle

//	ent->client->ps.gunframe++;

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))]--;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 0.8;
}


void Weapon_AMRAAM_Fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage;
	float	damage_radius;
	int 	vehicleside, vehicleup;	// +BD where do sidewinders start

	ent->client->side = ent->client->side ? 0 : 1;	// +BD

	if( strcmp(ent->vehicle->classname, "plane") == 0 )
	{
		vehicleside = 16;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		vehicleside = 6;
		vehicleup = 2;
		VectorCopy( ent->vehicle->movetarget->s.angles, angles );
		VectorCopy( ent->vehicle->movetarget->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "helo") == 0 )
	{
		vehicleside = 9;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}

	damage = 70 + (int)(random() * 80.0)-20;
	damage_radius = 150;

	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		VectorScale( forward, 20, forward );
		VectorAdd( start, forward, start );
		VectorNormalize( forward );
	}

	VectorScale( right, vehicleside, right );
	VectorScale( up, vehicleup, up );

	if( ent->client->side )
		VectorInverse(right);
	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY) ||
			strcmp( ent->vehicle->classname, "helo" ) == 0)
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_sidewinder( ent->vehicle, start, forward, damage,
		1500, damage_radius, 100, 2 ); // +BD (altspeed 650)


	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle

//	ent->client->ps.gunframe++;

	ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))]--;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 1.4;
}


void Weapon_Phoenix_Fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage;
	float	damage_radius;
	int 	vehicleside, vehicleup;	// +BD where do sidewinders start

	ent->client->side = ent->client->side ? 0 : 1;	// +BD

	if( strcmp(ent->vehicle->classname, "plane") == 0 )
	{
		vehicleside = 16;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		vehicleside = 6;
		vehicleup = 2;
		VectorCopy( ent->vehicle->movetarget->s.angles, angles );
		VectorCopy( ent->vehicle->movetarget->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "helo") == 0 )
	{
		vehicleside = 9;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}

	damage = 100 + (int)(random() * 90.0)-30;
	damage_radius = 150;

	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		VectorScale( forward, 20, forward );
		VectorAdd( start, forward, start );
		VectorNormalize( forward );
	}

	VectorScale( right, vehicleside, right );
	VectorScale( up, vehicleup, up );

	if( ent->client->side )
		VectorInverse(right);
	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY) ||
			strcmp( ent->vehicle->classname, "helo" ) == 0)
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_sidewinder( ent->vehicle, start, forward, damage,
		2000, damage_radius, 80, 3 ); // +BD (altspeed 650)


	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle

//	ent->client->ps.gunframe++;

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))]--;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 1.4;
}


// drone fires sidewinder
void Drone_Sidewinder_Fire (edict_t *drone)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage;
	float	damage_radius;
	int 	vehicleside, vehicleup;	// +BD where do sidewinders start
	static  side = 0;

	side = side ? 0 : 1;

	vehicleside = 16;
	vehicleup = -3;
	VectorCopy( drone->s.angles, angles );
	VectorCopy( drone->s.origin, start );

	damage = 60 + (int)(random() * 80.0)-20;
	damage_radius = 150;

	AngleVectors( angles, forward, right, up );// +BD new

	VectorScale( right, vehicleside, right );
	VectorScale( up, vehicleup, up );

	if( side )
		VectorInverse(right);
	VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_sidewinder( drone, start, forward, damage,
		1000, damage_radius, 150, 0 ); // +BD (altspeed 650)

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (drone-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET | is_silenced);
	gi.multicast (drone->s.origin, MULTICAST_PVS);// +BD +vehicle

}

// =======================================
// Bombs
// =======================================

void Weapon_Bombs_Fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage, speed;
	float	damage_radius;

	ent->client->side = ent->client->side ? 0 : 1;

	if( ent->vehicle->ONOFF & LANDED )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "No bomb drops on ground!\n" );
		ent->client->ps.gunframe++;
		return;
	}
	if( ( ent->vehicle->s.angles[0] > 270 || ent->vehicle->s.angles[0] <= 90 ) &&
		fabs(ent->vehicle->s.angles[2]) > 90 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "No bomb drops with that bank-angle!\n" );
		ent->client->ps.gunframe++;
		return;
	}
	if( ( ent->vehicle->s.angles[0] <= 270 && ent->vehicle->s.angles[0] > 90 ) &&
		fabs(ent->vehicle->s.angles[2]) < 90 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "No bomb drops with that bank-angle!\n" );
		ent->client->ps.gunframe++;
		return;
	}

	damage = 180 + (int)(random() * 60.0)-30;
	damage_radius = 130;

	VectorCopy( ent->vehicle->s.angles, angles );
	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	// +BD this block will change the side where the bombs start
	VectorCopy( ent->vehicle->s.origin, start );
	VectorScale( up, -3, up );
	VectorScale( right, 12, right );

	if( ent->client->side == 1 )
		VectorInverse(right);
	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY) ||
			strcmp( ent->vehicle->classname, "helo" ) == 0)
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

//	VectorSet(offset, 8, 8, ent->viewheight-8);

	speed = (int)(VectorLength(ent->vehicle->velocity));

	fire_bombs( ent->vehicle, start, forward, damage,
		 speed, damage_radius ); // +BD

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] --;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 0.3;
//	ent->client->ps.gunframe++;

}




void Weapon_LGB_Fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage, speed;
	float	damage_radius;

	ent->client->side = ent->client->side ? 0 : 1;

	if( ent->vehicle->ONOFF & LANDED )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "No bomb drops on ground!\n" );
		ent->client->ps.gunframe++;
		return;
	}
	if( ( ent->vehicle->s.angles[0] > 270 || ent->vehicle->s.angles[0] <= 90 ) &&
		fabs(ent->vehicle->s.angles[2]) > 90 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "No bomb drops with that bank-angle!\n" );
		ent->client->ps.gunframe++;
		return;
	}
	if( ( ent->vehicle->s.angles[0] <= 270 && ent->vehicle->s.angles[0] > 90 ) &&
		fabs(ent->vehicle->s.angles[2]) < 90 )
	{
		gi.cprintf( ent, PRINT_MEDIUM, "No bomb drops with that bank-angle!\n" );
		ent->client->ps.gunframe++;
		return;
	}

	damage = 280 + (int)(random() * 60.0)-30;
	damage_radius = 160;

	VectorCopy( ent->vehicle->s.angles, angles );
	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	// +BD this block will change the side where the bombs start
	VectorCopy( ent->vehicle->s.origin, start );
	VectorScale( up, -3, up );
	VectorScale( right, 12, right );

	if( ent->client->side == 1 )
		VectorInverse(right);
	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY)||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

//	VectorSet(offset, 8, 8, ent->viewheight-8);

	speed = (int)(VectorLength(ent->vehicle->velocity));

	fire_lgbs( ent->vehicle, start, forward, damage,
		 speed, damage_radius); // +BD

	ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] --;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 1.0;
//	ent->client->ps.gunframe++;

}




// ===============================================

// Main Gun

// ===============================================




void Weapon_MainGun_Fire (edict_t *ent)
{
	vec3_t	forward, right;
	vec3_t	start;

	AngleVectors( ent->vehicle->movetarget->s.angles, forward, right, NULL );

	// +BD this block will change where the bolts start
	VectorCopy( ent->vehicle->s.origin, start );

	start[2] += 2;

	VectorMA( start, 25, forward, start );

	fire_maingun (ent->vehicle, start, forward,
			ent->vehicle->maingundamage, 700, 0, 0 );// +BD new version

// 	ent->client->ps.gunframe++;

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);	// +BD +vehicle
	gi.WriteByte (MZ_ROCKET);	// +BD instead of BLASTER
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);	// +BD +vehicle

	ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))]--;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 1.0;
}

/*
void Weapon_MainGun (edict_t *ent)
{
	static int	pause_frames[]	= {29, 42, 57, 0};
	static int	fire_frames[]	= {7, 0};

	Weapon_Generic (ent, 6, 25, 57, 61, pause_frames, fire_frames, Weapon_MainGun_Fire);
}
*/


// =======================================

// Anti-Tank-Guided-Missiles (ATGM)

// =======================================


void Weapon_ATGM_Fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage = 90 + (int)(random() * 40.0)-20;
	float	damage_radius = 120.0;
	int 	vehicleside, vehicleup;	// +BD where do sidewinders start

	ent->client->side = ent->client->side ? 0 : 1;	// +BD

	if( strcmp(ent->vehicle->classname, "plane") == 0 )
	{
		vehicleside = 16;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		vehicleside = 0;
		vehicleup = 0;
		VectorCopy( ent->vehicle->movetarget->s.angles, angles );
		VectorCopy( ent->vehicle->movetarget->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "helo") == 0 )
	{
		vehicleside = 8;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}

	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		VectorScale( forward, 30, forward );
		VectorAdd( start, forward, start );
		VectorNormalize( forward );
	}

	VectorScale( right, vehicleside, right );
	VectorScale( up, vehicleup, up );

	if( ent->client->side )
		VectorInverse(right);
	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY)||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_ATGM( ent->vehicle, start, forward, damage, 550, damage_radius, 80, 0 );


	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle

//	ent->client->ps.gunframe++;


	ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))]--;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 1.0;

}




void Weapon_Maverick_Fire (edict_t *ent)
{
	vec3_t	start;
	vec3_t	forward, right, up;
	vec3_t  angles;
	int		damage = 140 + (int)(random() * 60.0)-30;
	float	damage_radius = 120.0;
	int 	vehicleside, vehicleup;	// +BD where do sidewinders start

	ent->client->side = ent->client->side ? 0 : 1;	// +BD

	if( strcmp(ent->vehicle->classname, "plane") == 0 )
	{
		vehicleside = 16;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		vehicleside = 0;
		vehicleup = 0;
		VectorCopy( ent->vehicle->movetarget->s.angles, angles );
		VectorCopy( ent->vehicle->movetarget->s.origin, start );
	}
	else if( strcmp( ent->vehicle->classname, "helo") == 0 )
	{
		vehicleside = 8;
		vehicleup = -3;
		VectorCopy( ent->vehicle->s.angles, angles );
		VectorCopy( ent->vehicle->s.origin, start );
	}

	if( ent->client->pers.videomodefix == 1 )
		angles[2] *= -1;
	AngleVectors( angles, forward, right, up );// +BD new

	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
	{
		VectorScale( forward, 30, forward );
		VectorAdd( start, forward, start );
		VectorNormalize( forward );
	}

	VectorScale( right, vehicleside, right );
	VectorScale( up, vehicleup, up );

	if( ent->client->side )
		VectorInverse(right);
	if( !(ent->vehicle->HASCAN & HAS_WEAPONBAY)||
			strcmp( ent->vehicle->classname, "helo" ) == 0 )
		VectorAdd( start, right, start );
	VectorAdd( start, up, start );

	fire_ATGM( ent->vehicle, start, forward, damage, 600, damage_radius, 80, 1 );


	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);		// +BD +vehicle
	gi.WriteByte (MZ_ROCKET);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);// +BD +vehicle

//	ent->client->ps.gunframe++;


	ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))]--;

	if( ent->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] < 1 )
		NoAmmoWeaponChange( ent );

	ent->client->pickle_time = level.time + 1.0;

}






// =======================================

// ECM System

// =======================================


void Weapon_ECM_Fire (edict_t *ent)
{
	edict_t	*lock;

	if( ent->vehicle->ONOFF & IS_JAMMING )
	{
		ent->vehicle->ONOFF &= ~IS_JAMMING;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Jammer"))] = 0;
		gi.cprintf( ent, PRINT_MEDIUM, "Jammer OFF!\n" );
	}
	else
	{
		ent->vehicle->ONOFF |= IS_JAMMING;
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Jammer"))] = 1;
		gi.cprintf( ent, PRINT_MEDIUM, "Jammer ON!\n" );
	}
	ent->client->pickle_time = level.time + 0.5;
}




// =======================================

// AutoCannon

// =======================================


void Weapon_Autocannon_Fire (edict_t *ent)
{

	vec3_t	forward, right, right2;
	vec3_t	start, start2;
	vec3_t	angles;		// used for spread
	vec3_t	direction;
	vec3_t  forward2;
	vec3_t	aimatthis;
	float	targetspeed;
	float	timetotarget;
	float	spread;

	if( ent->client->autocannon_time > level.time )
		return;

	if( (ent->vehicle->HASCAN & HAS_AUTOAIM) && ent->vehicle->enemy &&
		strcmp(ent->client->pers.secondary->pickup_name, "AutoCannon") == 0 )
	{
		AngleVectors( ent->vehicle->enemy->s.angles, forward, NULL, NULL );
		VectorSubtract( ent->vehicle->enemy->s.origin, ent->vehicle->s.origin, angles );
		timetotarget = VectorLength( angles )/(2400*FRAMETIME);
		targetspeed = (VectorLength( ent->vehicle->enemy->velocity )*FRAMETIME)*timetotarget;
		VectorMA( ent->vehicle->enemy->s.origin, targetspeed, forward, aimatthis );
		VectorSubtract( aimatthis, ent->vehicle->s.origin, direction );
		vectoangles( direction, angles );
	}
	else
	{
		if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
			VectorCopy( ent->vehicle->movetarget->s.angles, angles );
		else
			VectorCopy( ent->vehicle->s.angles, angles );
	}

	// add spread
	spread = ent->vehicle->gunspread;
	if( (ent->vehicle->HASCAN & HAS_AUTOAIM) &&
		strcmp( ent->client->pers.secondary->pickup_name, "AutoCannon" ) != 0 )
		spread /= 2;
	angles[0] += (random()*spread)-(spread/2);
	angles[1] += (random()*spread)-(spread/2);
	AngleVectors( angles, forward, right, NULL );

/*	if( strcmp( ent->vehicle->classname, "ground" ) == 0 )
		AngleVectors( ent->vehicle->movetarget->s.angles, forward, right, NULL );
	else
		AngleVectors ( ent->vehicle->s.angles, forward, right, NULL); // +BD new
*/
	VectorCopy( ent->vehicle->s.origin, start );
	VectorCopy( forward, forward2 );
	VectorScale( forward2, 25, forward2 );
	VectorAdd( ent->vehicle->s.origin, forward2, start );
	start[2] += 2;

	// set the start position
	if( strcmp( ent->vehicle->friendlyname, "2S6" ) == 0 )	// if Tunguska then fire twice
	{
		VectorScale( right, 4.5, right );
		VectorCopy( right, right2 );
		VectorCopy( start, start2 );
		VectorInverse(right2);
		VectorAdd( start, right2, start2 );
		VectorAdd( start, right, start );
		fire_blaster (ent->vehicle, start, forward,
			ent->vehicle->gundamage, 2400, EF_HYPERBLASTER, true );
		fire_blaster (ent->vehicle, start2, forward,
			ent->vehicle->gundamage, 2400, EF_HYPERBLASTER, true );
		// subtract the additional ammo
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))]-= 2;
	}
	else
	{
		fire_blaster (ent->vehicle, start, forward,
			ent->vehicle->gundamage, 2400, EF_HYPERBLASTER, true );
		ent->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] --;
	}

	// send muzzle flash
	gi.WriteByte (svc_muzzleflash);
	gi.WriteShort (ent->vehicle-g_edicts);
	gi.WriteByte (MZ_CHAINGUN3);
	gi.multicast (ent->vehicle->s.origin, MULTICAST_PVS);

	ent->client->autocannon_time = level.time + ent->vehicle->gunfirerate;
}
