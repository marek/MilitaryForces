#include "g_local.h"


void InitTrigger (edict_t *self)
{
	if (!VectorCompare (self->s.angles, vec3_origin))
		G_SetMovedir (self->s.angles, self->movedir);

	self->solid = SOLID_TRIGGER;
	self->movetype = MOVETYPE_NONE;
	gi.setmodel (self, self->model);
	self->svflags = SVF_NOCLIENT;
}

void radio_tower( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( strcmp( other->classname, "plane" ) != 0 &&
		strcmp( other->classname, "helo" ) != 0 &&
		strcmp( other->classname, "ground" ) != 0 &&
		strcmp( other->classname, "LQM" ) != 0 )
		return;
	if( other->HASCAN & IS_DRONE )
		return;
	other->radio_target = self;
}

// +BD AQ2 recharge
void recharge_vehicle( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	int type;
	int repair = 512;
	int done = 0;

	if( strcmp( other->classname, "plane" ) != 0 &&
		strcmp( other->classname, "helo" ) != 0 &&
		strcmp( other->classname, "ground" ) != 0 &&
		strcmp( other->classname, "LQM" ) != 0 )
		return;

//	gi.bprintf( PRINT_HIGH, "%d\n", self->sounds );

	if( other->deadflag )
		return;

	if( other->actualspeed > 1 )
		return;

	other->ONOFF |= IS_CHANGEABLE;

	if( level.time < other->recharge_time )
		return;

	if( other->thrust > THRUST_0 )
		return;

	if( !(other->ONOFF & PILOT_ONBOARD) )	// only recharge with pilot on board
		return;

	if( strcmp( other->classname, "plane" ) == 0 )
		type = 1;
	else if( strcmp( other->classname, "helo" ) == 0 )
		type = 2;
	else if( strcmp( other->classname, "ground" ) == 0 )
		type = 4;

	if( !self->sounds )
		self->sounds = 15;

	if( !(type & self->sounds) )
		return;

//	gi.cprintf( self->owner, PRINT_MEDIUM, "Recharging.\n" );

	if( other->health < other->max_health )
	{
		other->health += (10*other->max_health/100);
		if( other->health > other->max_health )
			other->health = other->max_health;
	}

	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] <
				other->owner->client->pers.max_bullets )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] +=
			(other->owner->client->pers.max_bullets/10);
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] >
				other->owner->client->pers.max_bullets )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bullets"))] =
				other->owner->client->pers.max_bullets;
	}

	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] <
				other->owner->client->pers.max_rockets )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] += 4;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] >
				other->owner->client->pers.max_rockets )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Rockets"))] =
				other->owner->client->pers.max_rockets;
	}

	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] <
				other->owner->client->pers.max_sidewinder )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] += 1;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] >
				other->owner->client->pers.max_sidewinder )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Sidewinder"))] =
				other->owner->client->pers.max_sidewinder;
	}
	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] <
				other->owner->client->pers.max_stinger )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] += 1;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] >
				other->owner->client->pers.max_stinger )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Stinger"))] =
				other->owner->client->pers.max_stinger;
	}
	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] <
				other->owner->client->pers.max_amraam )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] += 1;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] >
				other->owner->client->pers.max_amraam )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("AMRAAM"))] =
				other->owner->client->pers.max_amraam;
	}
	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] <
				other->owner->client->pers.max_phoenix )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] += 1;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] >
				other->owner->client->pers.max_phoenix )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Phoenix"))] =
				other->owner->client->pers.max_phoenix;
	}
	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] <
				other->owner->client->pers.max_bombs )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] += 1;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] >
				other->owner->client->pers.max_bombs )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Bombs"))] =
				other->owner->client->pers.max_bombs;
	}
	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] <
				other->owner->client->pers.max_lgbs )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] += 1;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] >
				other->owner->client->pers.max_lgbs )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("LGB"))] =
				other->owner->client->pers.max_lgbs;
	}
	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] <
				other->owner->client->pers.max_shells )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] += 5;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] >
				other->owner->client->pers.max_shells )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Shells"))] =
				other->owner->client->pers.max_shells;
	}
	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] <
				other->owner->client->pers.max_ATGMs )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] += 1;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] >
				other->owner->client->pers.max_ATGMs )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("ATGM"))] =
				other->owner->client->pers.max_ATGMs;
	}
	if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] <
				other->owner->client->pers.max_mavericks )
	{
		other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] += 1;
		if( other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] >
				other->owner->client->pers.max_mavericks )
			other->owner->client->pers.inventory[ITEM_INDEX(FindItem("Maverick"))] =
				other->owner->client->pers.max_mavericks;
	}
	if( other->flare < other->owner->client->pers.max_flares )
	{
		other->flare += 5;
		if( other->flare > other->owner->client->pers.max_flares )
			other->flare = other->owner->client->pers.max_flares;
	}
//	gi.bprintf( PRINT_HIGH, "dmg: %d rep: %d\n", other->DAMAGE, repair );


	if( other->DAMAGE )
	{
		while( !done )
		{
			if( other->DAMAGE & repair )
			{
				other->DAMAGE ^= repair;
				done = 1;
			}
			else
				repair /= 2;
		}
	}

	if( !other->DAMAGE && strcmp( other->classname, "LQM" ) != 0 )
	{
		gi.cprintf( other->owner, PRINT_MEDIUM, "All systems OK!\n" );
		repair = 0;
	}
	else
	{
		switch( repair )
		{
			case WEAPONBAY_DAMAGE:
				gi.cprintf( other->owner, PRINT_MEDIUM, "Weaponbay repaired.\n" );
				break;
			case AUTOPILOT_DAMAGE:
				gi.cprintf( other->owner, PRINT_MEDIUM, "Autopilot repaired.\n" );
				break;
			case GPS_DAMAGE:
				gi.cprintf( other->owner, PRINT_MEDIUM, "GPS repaired.\n" );
				break;
			case ENGINE0:
				gi.cprintf( other->owner, PRINT_MEDIUM, "Engine 25 percent workable.\n" );
				break;
			case ENGINE25:
				gi.cprintf( other->owner, PRINT_MEDIUM, "Engine 50 percent workable.\n" );
				break;
			case ENGINE50:
				gi.cprintf( other->owner, PRINT_MEDIUM, "Engine 75 percent workable.\n" );
				break;
			case ENGINE75:
				gi.cprintf( other->owner, PRINT_MEDIUM, "Engine 100 percent workable.\n" );
				break;
			case AFTERBURNER_DAMAGE:
				if( strcmp( other->classname,"ground") == 0 )
					gi.cprintf( other->owner, PRINT_MEDIUM, "Reverse repaired.\n" );
				else
					gi.cprintf( other->owner, PRINT_MEDIUM, "Afterburner repaired.\n" );
				break;
			case SPEEDBRAKE_DAMAGE:
				gi.cprintf( other->owner, PRINT_MEDIUM, "Brakes repaired.\n" );
				break;
			case GEAR_DAMAGE:
				gi.cprintf( other->owner, PRINT_MEDIUM, "Gear repaired.\n" );
				break;
		}
	}

	other->recharge_time = level.time + 1.6;

}


// the wait time has passed, so set back up for another activation
void multi_wait (edict_t *ent)
{
	ent->nextthink = 0;
}

void trigger_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->s.effects = EF_ROCKET;
//	gi.bprintf( PRINT_HIGH, "Hallo!\n" );
}

void trigger_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{
	G_UseTargets( self, attacker );
	gi.unlinkentity( self );
	G_FreeEdict( self );
}

void trigger_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	gi.bprintf( PRINT_HIGH, "Hallo!\n" );
}

// the trigger was just activated
// ent->activator should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
void multi_trigger (edict_t *ent)
{
	if (ent->nextthink)
		return;		// already been triggered

	G_UseTargets (ent, ent->activator);

	if (ent->wait > 0)
	{
		ent->think = multi_wait;
		ent->nextthink = level.time + ent->wait;
	}
	else
	{	// we can't just remove (self) here, because this is a touch function
		// called while looping through area links...
		ent->touch = NULL;
		ent->nextthink = level.time + FRAMETIME;
		ent->think = G_FreeEdict;
	}
}

void Use_Multi (edict_t *ent, edict_t *other, edict_t *activator)
{
	ent->activator = activator;
	multi_trigger (ent);
}

void Touch_Multi (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
//	if(other->client)
//	now look for vehicles
	if( strcmp( other->classname, "plane" ) == 0 ||
		strcmp( other->classname, "helo" ) == 0 ||
		strcmp( other->classname, "ground" ) == 0 )
	{
		if (self->spawnflags & 2)
			return;
	}
// 	no monsters in AQ2!
/*	else if (other->svflags & SVF_MONSTER)
	{
		if (!(self->spawnflags & 1))
			return;
	}
*/	else
		return;

	if (!VectorCompare(self->movedir, vec3_origin))
	{
		vec3_t	forward;

		AngleVectors(other->s.angles, forward, NULL, NULL);
		if (_DotProduct(forward, self->movedir) < 0)
			return;
	}

	self->activator = other;
	multi_trigger (self);
}

/*QUAKED trigger_multiple (.5 .5 .5) ? MONSTER NOT_PLAYER TRIGGERED
Variable sized repeatable trigger.  Must be targeted at one or more entities.
If "delay" is set, the trigger waits some time after activating before firing.
"wait" : Seconds between triggerings. (.2 default)
sounds
1)	secret
2)	beep beep
3)	large switch
4)
set "message" to text string
*/
void trigger_enable (edict_t *self, edict_t *other, edict_t *activator)
{
	self->solid = SOLID_TRIGGER;
	self->use = Use_Multi;
	gi.linkentity (self);
}

void SP_trigger_multiple (edict_t *ent)
{
	if (ent->sounds == 1)
		ent->noise_index = gi.soundindex ("misc/secret.wav");
	else if (ent->sounds == 2)
		ent->noise_index = gi.soundindex ("misc/talk.wav");
	else if (ent->sounds == 3)
		ent->noise_index = gi.soundindex ("misc/trigger1.wav");

	if (!ent->wait)
		ent->wait = 0.2;

	ent->touch = Touch_Multi;
	ent->movetype = MOVETYPE_NONE;
	ent->svflags |= SVF_NOCLIENT;


	if (ent->spawnflags & 4)
	{
		ent->solid = SOLID_NOT;
		ent->use = trigger_enable;
	}
	else
	{
		ent->solid = SOLID_TRIGGER;
		ent->use = Use_Multi;
	}
	if( ent->health > 0 )
	{
		ent->solid = SOLID_TRIGGER;
		ent->takedamage = DAMAGE_YES;
		ent->pain = trigger_pain;
		ent->die = trigger_die;
		ent->touch = trigger_touch;
	}
	if (!VectorCompare(ent->s.angles, vec3_origin))
		G_SetMovedir (ent->s.angles, ent->movedir);

	gi.setmodel (ent, ent->model);
	gi.linkentity (ent);
}


/*QUAKED trigger_once (.5 .5 .5) ? x x TRIGGERED
Triggers once, then removes itself.
You must set the key "target" to the name of another object in the level that has a matching "targetname".

If TRIGGERED, this trigger must be triggered before it is live.

sounds
 1)	secret
 2)	beep beep
 3)	large switch
 4)

"message"	string to be displayed when triggered
*/

void SP_trigger_once(edict_t *ent)
{
	// make old maps work because I messed up on flag assignments here
	// triggered was on bit 1 when it should have been on bit 4
	if (ent->spawnflags & 1)
	{
		vec3_t	v;

		VectorMA (ent->mins, 0.5, ent->size, v);
		ent->spawnflags &= ~1;
		ent->spawnflags |= 4;
		gi.dprintf("fixed TRIGGERED flag on %s at %s\n", ent->classname, vtos(v));
	}

	ent->wait = -1;
	SP_trigger_multiple (ent);
}

/*QUAKED trigger_relay (.5 .5 .5) (-8 -8 -8) (8 8 8)
This fixed size trigger cannot be touched, it can only be fired by other events.
*/
void trigger_relay_use (edict_t *self, edict_t *other, edict_t *activator)
{
	G_UseTargets (self, activator);
}

void SP_trigger_relay (edict_t *self)
{
	self->use = trigger_relay_use;
}


/*
==============================================================================

trigger_key

==============================================================================
*/

/*QUAKED trigger_key (.5 .5 .5) (-8 -8 -8) (8 8 8)
A relay trigger that only fires it's targets if player has the proper key.
Use "item" to specify the required key, for example "key_data_cd"
*/
void trigger_key_use (edict_t *self, edict_t *other, edict_t *activator)
{
	int			index;

	if (!self->item)
		return;
	if (!activator->client)
		return;

	index = ITEM_INDEX(self->item);
	if (!activator->client->pers.inventory[index])
	{
		if (level.time < self->touch_debounce_time)
			return;
		self->touch_debounce_time = level.time + 5.0;
		gi.centerprintf (activator, "You need the %s", self->item->pickup_name);
		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/keytry.wav"), 1, ATTN_NORM, 0);
		return;
	}

	gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/keyuse.wav"), 1, ATTN_NORM, 0);
	if (coop->value)
	{
		int		player;
		edict_t	*ent;

		if (strcmp(self->item->classname, "key_power_cube") == 0)
		{
			int	cube;

			for (cube = 0; cube < 8; cube++)
				if (activator->client->pers.power_cubes & (1 << cube))
					break;
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = &g_edicts[player];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				if (ent->client->pers.power_cubes & (1 << cube))
				{
					ent->client->pers.inventory[index]--;
					ent->client->pers.power_cubes &= ~(1 << cube);
				}
			}
		}
		else
		{
			for (player = 1; player <= game.maxclients; player++)
			{
				ent = &g_edicts[player];
				if (!ent->inuse)
					continue;
				if (!ent->client)
					continue;
				ent->client->pers.inventory[index] = 0;
			}
		}
	}
	else
	{
		activator->client->pers.inventory[index]--;
	}

	G_UseTargets (self, activator);

	self->use = NULL;
}

void SP_trigger_key (edict_t *self)
{
	if (!st.item)
	{
		gi.dprintf("no key item for trigger_key at %s\n", vtos(self->s.origin));
		return;
	}
	self->item = FindItemByClassname (st.item);

	if (!self->item)
	{
		gi.dprintf("item %s not found for trigger_key at %s\n", st.item, vtos(self->s.origin));
		return;
	}

	if (!self->target)
	{
		gi.dprintf("%s at %s has no target\n", self->classname, vtos(self->s.origin));
		return;
	}

	gi.soundindex ("misc/keytry.wav");
	gi.soundindex ("misc/keyuse.wav");

	self->use = trigger_key_use;
}


/*
==============================================================================

trigger_counter

==============================================================================
*/

/*QUAKED trigger_counter (.5 .5 .5) ? nomessage
Acts as an intermediary for an action that takes multiple inputs.

If nomessage is not set, t will print "1 more.. " etc when triggered and "sequence complete" when finished.

After the counter has been triggered "count" times (default 2), it will fire all of it's targets and remove itself.
*/

void trigger_counter_use(edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->count == 0)
		return;

	self->count--;

	if (self->count)
	{
		if (! (self->spawnflags & 1))
		{
			gi.centerprintf(activator, "%i more to go...", self->count);
			gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
		}
		return;
	}

	if (! (self->spawnflags & 1))
	{
		gi.centerprintf(activator, "Sequence completed!");
		gi.sound (activator, CHAN_AUTO, gi.soundindex ("misc/talk1.wav"), 1, ATTN_NORM, 0);
	}
	self->activator = activator;
	multi_trigger (self);
}

void SP_trigger_counter (edict_t *self)
{
	self->wait = -1;
	if (!self->count)
		self->count = 2;

	self->use = trigger_counter_use;
}


/*
==============================================================================

trigger_always

==============================================================================
*/

/*QUAKED trigger_always (.5 .5 .5) (-8 -8 -8) (8 8 8)
This trigger will always fire.  It is activated by the world.
*/
void SP_trigger_always (edict_t *ent)
{
	// we must have some delay to make sure our use targets are present
	if (ent->delay < 0.2)
		ent->delay = 0.2;
	G_UseTargets(ent, ent);
}


/*
==============================================================================

trigger_push

==============================================================================
*/

#define PUSH_ONCE		1

static int windsound;

void trigger_push_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (strcmp(other->classname, "grenade") == 0)
	{
		VectorScale (self->movedir, self->speed * 10, other->velocity);
	}
	else if (other->health > 0)
	{
		VectorScale (self->movedir, self->speed * 10, other->velocity);

		if (other->client)
		{
			// don't take falling damage immediately from this
			VectorCopy (other->velocity, other->client->oldvelocity);
/*			if (other->fly_sound_debounce_time < level.time)
			{
				other->fly_sound_debounce_time = level.time + 1.5;
				gi.sound (other, CHAN_AUTO, windsound, 1, ATTN_NORM, 0);
			}*/
		}
	}
	if (self->spawnflags & PUSH_ONCE)
		G_FreeEdict (self);
}


/*QUAKED trigger_push (.5 .5 .5) ? PUSH_ONCE
Pushes the player
"speed"		defaults to 1000
*/
void SP_trigger_push (edict_t *self)
{
	InitTrigger (self);
	windsound = gi.soundindex ("misc/windfly.wav");
	self->touch = trigger_push_touch;
	if (!self->speed)
		self->speed = 1000;
	gi.linkentity (self);
}
/*
==============================================================================

trigger_recharge
(AQ2 only)
==============================================================================
*/

/*QUAKED trigger_recharge (0 1 1) ?
Recharges the vehicle standing
"inside" it.

key-values:
-----------
sounds - 0,1,2,3
	0 = all vehicles
	1 = planes only
	2 = helos only
	3 = ground only
*/

void SP_trigger_recharge (edict_t *self)
{
	InitTrigger(self);
	self->classname = "trigger_recharge";
	self->touch = recharge_vehicle;
	gi.linkentity (self);
}

/*
==============================================================================

trigger_radio

==============================================================================
*/

/*QUAKED trigger_radio (0 1 1) ?
Used to trigger anything controlled
by the tower (elvators, catapults etc)
target is the entity triggered
sound if for special category only
*/

void SP_trigger_radio (edict_t *self)
{
	InitTrigger(self);
	self->classname = "trigger_radio";
	self->touch = radio_tower;
	gi.linkentity (self);
}

/*
==============================================================================

trigger_hurt

==============================================================================
*/

/*QUAKED trigger_hurt (.5 .5 .5) ? START_OFF TOGGLE SILENT NO_PROTECTION SLOW
Any entity that touches this will be hurt.

It does dmg points of damage each server frame

SILENT			supresses playing the sound
SLOW			changes the damage rate to once per second
NO_PROTECTION	*nothing* stops the damage

"dmg"			default 5 (whole numbers only)

*/
void hurt_use (edict_t *self, edict_t *other, edict_t *activator)
{
	if (self->solid == SOLID_NOT)
		self->solid = SOLID_TRIGGER;
	else
		self->solid = SOLID_NOT;
	gi.linkentity (self);

	if (!(self->spawnflags & 2))
		self->use = NULL;
}


void hurt_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	int		dflags;

	if (!other->takedamage)
		return;

	if (self->timestamp > level.time)
		return;

	if (self->spawnflags & 16)
		self->timestamp = level.time + 1;
	else
		self->timestamp = level.time + FRAMETIME;

	if (!(self->spawnflags & 4))
	{
		if ((level.framenum % 10) == 0)
			gi.sound (other, CHAN_AUTO, self->noise_index, 1, ATTN_NORM, 0);
	}

	if (self->spawnflags & 8)
		dflags = DAMAGE_NO_PROTECTION;
	else
		dflags = 0;
	T_Damage (other, self, self, vec3_origin, other->s.origin, vec3_origin, self->dmg, self->dmg, dflags, MOD_TRIGGER_HURT);
}

void SP_trigger_hurt (edict_t *self)
{
	InitTrigger (self);

	self->noise_index = gi.soundindex ("world/electro.wav");
	self->touch = hurt_touch;

	if (!self->dmg)
		self->dmg = 5;

	if (self->spawnflags & 1)
		self->solid = SOLID_NOT;
	else
		self->solid = SOLID_TRIGGER;

	if (self->spawnflags & 2)
		self->use = hurt_use;

	gi.linkentity (self);
}


/*
==============================================================================

trigger_gravity

==============================================================================
*/

/*QUAKED trigger_gravity (.5 .5 .5) ?
Changes the touching entites gravity to
the value of "gravity".  1.0 is standard
gravity for the level.
*/

void trigger_gravity_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	other->gravity = self->gravity;
}

void SP_trigger_gravity (edict_t *self)
{
	if (st.gravity == 0)
	{
		gi.dprintf("trigger_gravity without gravity set at %s\n", vtos(self->s.origin));
		G_FreeEdict  (self);
		return;
	}

	InitTrigger (self);
	self->gravity = atoi(st.gravity);
	self->touch = trigger_gravity_touch;
}


/*
==============================================================================

trigger_monsterjump

==============================================================================
*/

/*QUAKED trigger_monsterjump (.5 .5 .5) ?
Walking monsters that touch this will jump in the direction of the trigger's angle
"speed" default to 200, the speed thrown forward
"height" default to 200, the speed thrown upwards
*/

void trigger_monsterjump_touch (edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf)
{
	if (other->flags & (FL_FLY | FL_SWIM) )
		return;
	if (other->svflags & SVF_DEADMONSTER)
		return;
	if ( !(other->svflags & SVF_MONSTER))
		return;

// set XY even if not on ground, so the jump will clear lips
	other->velocity[0] = self->movedir[0] * self->speed;
	other->velocity[1] = self->movedir[1] * self->speed;

	if (!other->groundentity)
		return;

	other->groundentity = NULL;
	other->velocity[2] = self->movedir[2];
}

void SP_trigger_monsterjump (edict_t *self)
{
	if (!self->speed)
		self->speed = 200;
	if (!st.height)
		st.height = 200;
	if (self->s.angles[YAW] == 0)
		self->s.angles[YAW] = 360;
	InitTrigger (self);
	self->touch = trigger_monsterjump_touch;
	self->movedir[2] = st.height;
}

