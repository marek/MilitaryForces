/************************************************/
/* Filename: aq2_data.cpp					    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 1999-07-10							*/
/*												*/
/* Description:									*/
/* NewAirQuake2 vehicle-specific stuff   		*/
/*												*/
/************************************************/

#include "g_local.h"


// -----------------------------------------------
//						PLANES
// -----------------------------------------------


void SetPlaneData( edict_t *plane )
{
	int i;

	plane->classname = "plane";
	plane->solid = SOLID_BBOX;
	plane->deadflag = DEAD_NO;
	plane->clipmask = MASK_PLAYERSOLID;
	plane->takedamage = DAMAGE_AIM;
	plane->movetype = MOVETYPE_PLANE;
	plane->DAMAGE = DAMAGE_NONE;
	plane->random = level.time;
	plane->recharge_time = level.time;
	plane->think = PlaneMovement;
	plane->nextthink = level.time + 0.1;
	plane->touch = PlaneTouch;
	plane->pain = VehiclePain;
	plane->thrust = THRUST_IDLE;
	plane->die = PlaneDie;
	plane->active_weapon = 1;
	plane->ONOFF = 0;
	plane->volume = level.time + 0.2;
	plane->attenuation = level.time + 0.2;
	plane->dmg_radius = 0;
	plane->jam_time = level.time;

	for( i = 0; i < MAX_WEAPONS; i++ )
		plane->weapon[i] = plane->ammo[i] = 0;
}


void Spawn_F16( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -26, -16, -2 );
	VectorSet( plane->maxs, 32, 16, 6 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_ROUNDBODY;

	plane->stallspeed = 120;
	plane->topspeed = 255;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = 14;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 125;
	plane->gib_health = -50;

	plane->guntype = &guntypes[GUN_VULCAN];

	plane->flares = plane->maxflares = 30;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 400;
	strcpy( plane->weaponname[0], "M61A-1" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 6;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );


	
	plane->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f16/tris.md2");
	VectorSet( plane->turnspeed, 180, 40, 240 );
	VectorSet( plane->movedir, -60, 15, 0 );		// camera position
}

void Spawn_F14( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -30, -19, 0 );
	VectorSet( plane->maxs, 34, 19, 6 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT;

	plane->stallspeed = 130;
	plane->topspeed = 300;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = MAXTHRUST;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 150;
	plane->gib_health = -70;

	plane->guntype = &guntypes[GUN_VULCAN];

	plane->flares = plane->maxflares = 30;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 400;
	strcpy( plane->weaponname[0], "M61A-1" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 4;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_PHOENIX; plane->maxammo[2] = plane->ammo[2] = 4;
	strcpy( plane->weaponname[2], "Phoenix" ); plane->weaponicon[2] = gi.imageindex( "Phoenix" );
	plane->weaponicon_dam[2] = gi.imageindex( "phoenix_dam" );

	plane->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f14/tris.md2");

	VectorSet( plane->turnspeed, 160, 40, 200 );
	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
}

void Spawn_F4( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -36, -16, 0 );
	VectorSet( plane->maxs, 30, 16, 8 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT;

	plane->stallspeed = 115;
	plane->topspeed = 255;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = MAXTHRUST;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 150;
	plane->gib_health = -50;

	plane->guntype = &guntypes[GUN_VULCAN];
	plane->flares = plane->maxflares = 35;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 200;
	strcpy( plane->weaponname[0], "M61" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 4;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_AMRAAM; plane->maxammo[2] = plane->ammo[2] = 2;
	strcpy( plane->weaponname[2], "AMRAAM" ); plane->weaponicon[2] = gi.imageindex( "Amraam" );
	plane->weaponicon_dam[2] = gi.imageindex( "amraam_dam" );
	plane->weapon[3] = WEAPON_JAMMER; plane->maxammo[3] = plane->ammo[3] = 300;
	strcpy( plane->weaponname[3], "Jammer" ); plane->weaponicon[3] = gi.imageindex( "Jammer" );
	plane->weaponicon_dam[3] = gi.imageindex( "jammer_dam" );

	plane->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f4/tris.md2");
	VectorSet( plane->turnspeed, 170, 40, 220 );
	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
}

void Spawn_F22( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -28, -20, -1 );
	VectorSet( plane->maxs, 33, 20, 7 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_WEAPONBAY | HASCAN_STEALTH;

	plane->stallspeed = 120;
	plane->topspeed = 270;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = 14;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 100;
	plane->gib_health = -50;

	plane->guntype = &guntypes[GUN_VULCAN];

	plane->flares = plane->maxflares = 25;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 200;
	strcpy( plane->weaponname[0], "Autocannon" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_AMRAAM; plane->maxammo[2] = plane->ammo[2] = 4;
	strcpy( plane->weaponname[2], "AMRAAM" ); plane->weaponicon[2] = gi.imageindex( "Amraam" );
	plane->weaponicon_dam[2] = gi.imageindex( "amraam_dam" );

	plane->weaponicon_dam[4] = gi.imageindex( "ffar_dam" );

	plane->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f22/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 170, 40, 220 );
}

void Spawn_MIG21( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -34, -14, -1 );
	VectorSet( plane->maxs, 30, 14, 6 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_EASTERN | HASCAN_ROUNDBODY;

	plane->stallspeed = 115;
	plane->topspeed = 225;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = 13;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 170;
	plane->gib_health = -50;

	plane->guntype = &guntypes[GUN_VULCAN];

	plane->flares = plane->maxflares = 35;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 300;
	strcpy( plane->weaponname[0], "GSh-23L" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_AMRAAM; plane->maxammo[2] = plane->ammo[2] = 4;
	strcpy( plane->weaponname[2], "AMRAAM" ); plane->weaponicon[2] = gi.imageindex( "Amraam" );
	plane->weaponicon_dam[2] = gi.imageindex( "amraam_dam" );


	plane->s.modelindex = gi.modelindex ("vehicles/planes/fighter/mig21/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 170, 40, 220 );
}

void Spawn_MIG29( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -30, -22, -2 );
	VectorSet( plane->maxs, 36, 22, 7 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_EASTERN;

	plane->stallspeed = 110;
	plane->topspeed = 250;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = MAXTHRUST;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 130;
	plane->gib_health = -60;

	plane->guntype = &guntypes[GUN_MAUSER];

	plane->flares = plane->maxflares = 30;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 400;
	strcpy( plane->weaponname[0], "Autocannon" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 6;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );


	plane->s.modelindex = gi.modelindex ("vehicles/planes/fighter/mig29/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 180, 40, 240 );
}

void Spawn_MIGMFI( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -32, -21, -2 );
	VectorSet( plane->maxs, 28, 21, 6 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_EASTERN | HASCAN_WEAPONBAY | HASCAN_STEALTH;

	plane->stallspeed = 120;
	plane->topspeed = 275;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = 14;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 100;
	plane->gib_health = -50;

	plane->guntype = &guntypes[GUN_VULCAN];

	plane->flares = plane->maxflares = 25;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 200;
	strcpy( plane->weaponname[0], "Autocannon" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_AMRAAM; plane->maxammo[2] = plane->ammo[2] = 4;
	strcpy( plane->weaponname[2], "AMRAAM" ); plane->weaponicon[2] = gi.imageindex( "Amraam" );
	plane->weaponicon_dam[2] = gi.imageindex( "amraam_dam" );


	plane->s.modelindex = gi.modelindex ("vehicles/planes/fighter/migmfi/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 170, 40, 220 );
}

void Spawn_SU37( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -30, -22, -1 );
	VectorSet( plane->maxs, 35, 22, 7 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_EASTERN;

	plane->stallspeed = 120;
	plane->topspeed = 285;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = MAXTHRUST;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 130;
	plane->gib_health = -60;

	plane->guntype = &guntypes[GUN_MAUSER];

	plane->flares = plane->maxflares = 30;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 400;
	strcpy( plane->weaponname[0], "Autocannon" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_AMRAAM; plane->maxammo[2] = plane->ammo[2] = 4;
	strcpy( plane->weaponname[2], "AMRAAM" ); plane->weaponicon[2] = gi.imageindex( "Amraam" );
	plane->weaponicon_dam[2] = gi.imageindex( "amraam_dam" );

	plane->s.modelindex = gi.modelindex ("vehicles/planes/fighter/su37/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 170, 40, 220 );
}

void Spawn_A10( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -32, -34, -3 );
	VectorSet( plane->maxs, 26, 34, 6 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT;

	plane->stallspeed = 80;
	plane->topspeed = 220;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = THRUST_MILITARY;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 200;
	plane->gib_health = -100;

	plane->guntype = &guntypes[GUN_VULCAN];

	plane->flares = plane->maxflares = 40;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 300;
	strcpy( plane->weaponname[0], "GAU-8/A" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_MAVERICK; plane->maxammo[1] = plane->ammo[1] = 6;
	strcpy( plane->weaponname[1], "Maverick" ); plane->weaponicon[1] = gi.imageindex( "Maverick" );
	plane->weaponicon_dam[1] = gi.imageindex( "maverick_dam" );
	plane->weapon[2] = WEAPON_IRONBOMBS; plane->maxammo[2] = plane->ammo[2] = 4;
	strcpy( plane->weaponname[2], "Mk82" ); plane->weaponicon[2] = gi.imageindex( "Mk82" );
	plane->weaponicon_dam[2] = gi.imageindex( "mk82_dam" );
	plane->weapon[3] = WEAPON_SIDEWINDER; plane->maxammo[3] = plane->ammo[3] = 2;
	strcpy( plane->weaponname[3], "Sidewinder" ); plane->weaponicon[3] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[3] = gi.imageindex( "sidewinder_dam" );

	plane->s.modelindex = gi.modelindex ("vehicles/planes/attack/a10/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 160, 40, 200 );
}

void Spawn_F117( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -30, -23, 1 );
	VectorSet( plane->maxs, 28, 23, 7 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_WEAPONBAY | HASCAN_STEALTH;

	plane->stallspeed = 110;
	plane->topspeed = 240;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = THRUST_MILITARY;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 100;
	plane->gib_health = -50;

	plane->guntype = &guntypes[GUN_VULCAN];

	plane->flares = plane->maxflares = 25;
	plane->weapon[0] = WEAPON_NONE; plane->maxammo[0] = plane->ammo[0] = 0;
	strcpy( plane->weaponname[0], "Autocannon" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_LASERBOMBS; plane->maxammo[2] = plane->ammo[2] = 4;
	strcpy( plane->weaponname[2], "LGB" ); plane->weaponicon[2] = gi.imageindex( "LGB" );
	plane->weaponicon_dam[2] = gi.imageindex( "lgb_dam" );


	plane->s.modelindex = gi.modelindex ("vehicles/planes/attack/f117/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 160, 40, 200 );
}

void Spawn_Tornado( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -25, -16, -2 );
	VectorSet( plane->maxs, 33, 16, 7 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT;

	plane->stallspeed = 125;
	plane->topspeed = 285;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = MAXTHRUST;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 130;
	plane->gib_health = -50;

	plane->guntype = &guntypes[GUN_MAUSER];

	plane->flares = plane->maxflares = 35;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 250;
	strcpy( plane->weaponname[0], "Mauser" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_IRONBOMBS; plane->maxammo[2] = plane->ammo[2] = 8;
	strcpy( plane->weaponname[2], "Ironbomb" ); plane->weaponicon[2] = gi.imageindex( "Mk82" );
	plane->weaponicon_dam[2] = gi.imageindex( "mk82_dam" );
	plane->weapon[3] = WEAPON_FFAR; plane->maxammo[3] = plane->ammo[3] = 16;
	strcpy( plane->weaponname[3], "Rocket" ); plane->weaponicon[3] = gi.imageindex( "Ffar" );
	plane->weaponicon_dam[3] = gi.imageindex( "ffar_dam" );

	plane->s.modelindex = gi.modelindex ("vehicles/planes/attack/tornado/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 170, 40, 220 );
}

void Spawn_SU24( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -34, -21, -1 );
	VectorSet( plane->maxs, 40, 21, 8 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_EASTERN;

	plane->stallspeed = 120;
	plane->topspeed = 285;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = MAXTHRUST;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 130;
	plane->gib_health = -50;

	plane->guntype = &guntypes[GUN_VULCAN];

	plane->flares = plane->maxflares = 30;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 250;
	strcpy( plane->weaponname[0], "Autocannon" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_MAVERICK; plane->maxammo[2] = plane->ammo[2] = 6;
	strcpy( plane->weaponname[2], "Maverick" ); plane->weaponicon[2] = gi.imageindex( "Maverick" );
	plane->weaponicon_dam[2] = gi.imageindex( "maverick_dam" );
	plane->weapon[3] = WEAPON_IRONBOMBS; plane->maxammo[3] = plane->ammo[3] = 10;
	strcpy( plane->weaponname[3], "Ironbomb" ); plane->weaponicon[3] = gi.imageindex( "Mk82" );
	plane->weaponicon_dam[3] = gi.imageindex( "mk82_dam" );


	plane->s.modelindex = gi.modelindex ("vehicles/planes/attack/su24/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 160, 40, 200 );
}

void Spawn_SU25( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -26, -26, -1 );
	VectorSet( plane->maxs, 26, 26, 7 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_EASTERN;

	plane->stallspeed = 95;
	plane->topspeed = 230;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = THRUST_MILITARY;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 180;
	plane->gib_health = -70;

	plane->guntype = &guntypes[GUN_GAU];

	plane->flares = plane->maxflares = 40;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 400;
	strcpy( plane->weaponname[0], "Autocannon" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_MAVERICK; plane->maxammo[2] = plane->ammo[2] = 2;
	strcpy( plane->weaponname[2], "Maverick" ); plane->weaponicon[2] = gi.imageindex( "Maverick" );
	plane->weaponicon_dam[2] = gi.imageindex( "maverick_dam" );
	plane->weapon[3] = WEAPON_FFAR; plane->maxammo[3] = plane->ammo[3] = 48;
	strcpy( plane->weaponname[3], "Rocket" ); plane->weaponicon[3] = gi.imageindex( "Ffar" );
	plane->weaponicon_dam[3] = gi.imageindex( "ffar_dam" );

	plane->s.modelindex = gi.modelindex ("vehicles/planes/attack/su25/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 160, 40, 200 );
}

void Spawn_SU34( edict_t *plane )
{
	SetPlaneData( plane );

	VectorSet( plane->mins, -30, -22, 0 );
	VectorSet( plane->maxs, 32, 22, 7 );

	plane->HASCAN = HASCAN_BRAKE | HASCAN_GEAR | HASCAN_AUTOPILOT | HASCAN_EASTERN;

	plane->stallspeed = 120;
	plane->topspeed = 285;	// speed without afterburner
	plane->accel = 7;
	plane->decel = 5;
	plane->speed = plane->stallspeed * 1.5;
	plane->maxthrust = MAXTHRUST;
	plane->minthrust = 0;
	plane->mass = 5000;

	plane->max_health = plane->health = 130;
	plane->gib_health = -60;

	plane->guntype = &guntypes[GUN_MAUSER];

	plane->flares = plane->maxflares = 25;
	plane->weapon[0] = WEAPON_AUTOCANNON; plane->maxammo[0] = plane->ammo[0] = 250;
	strcpy( plane->weaponname[0], "Autocannon" ); plane->weaponicon[0] = gi.imageindex( "Aacann" );
	plane->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	plane->weapon[1] = WEAPON_SIDEWINDER; plane->maxammo[1] = plane->ammo[1] = 2;
	strcpy( plane->weaponname[1], "Sidewinder" ); plane->weaponicon[1] = gi.imageindex( "Sidewinder" );
	plane->weaponicon_dam[1] = gi.imageindex( "sidewinder_dam" );
	plane->weapon[2] = WEAPON_MAVERICK; plane->maxammo[2] = plane->ammo[2] = 4;
	strcpy( plane->weaponname[2], "Maverick" ); plane->weaponicon[2] = gi.imageindex( "Maverick" );
	plane->weaponicon_dam[2] = gi.imageindex( "maverick_dam" );
	plane->weapon[3] = WEAPON_LASERBOMBS; plane->maxammo[3] = plane->ammo[3] = 2;
	strcpy( plane->weaponname[3], "LGB" ); plane->weaponicon[3] = gi.imageindex( "LGB" );
	plane->weaponicon_dam[3] = gi.imageindex( "lgb_dam" );
	plane->weapon[4] = WEAPON_JAMMER; plane->maxammo[4] = plane->ammo[4] = 150;
	strcpy( plane->weaponname[4], "Jammer" ); plane->weaponicon[4] = gi.imageindex( "Jammer" );
	plane->weaponicon_dam[4] = gi.imageindex( "jammer_dam" );

	plane->s.modelindex = gi.modelindex ("vehicles/planes/attack/su34/tris.md2");


	VectorSet( plane->movedir, -50, 15, 0 );		// camera position
	VectorSet( plane->turnspeed, 170, 40, 220 );
}





// ===========================================
//					HELOS
// ===========================================

void SetHeloData( edict_t *helo )
{
	int i;

	helo->classname = "helo";
	helo->solid = SOLID_BBOX;
	helo->deadflag = DEAD_NO;
	helo->clipmask = MASK_PLAYERSOLID;
	helo->takedamage = DAMAGE_AIM;
	helo->movetype = MOVETYPE_HELO;
	helo->DAMAGE = DAMAGE_NONE;
	helo->random = level.time;
	helo->recharge_time = level.time;
	helo->think = HeloMovement;
	helo->nextthink = level.time + 0.1;
	helo->touch = HeloTouch;
	helo->pain = VehiclePain;
	helo->thrust = THRUST_IDLE;
	helo->die = HeloDie;
	helo->active_weapon = 1;
	helo->ONOFF = 0;
	helo->volume = level.time + 0.2;
	helo->attenuation = level.time + 0.2;
	helo->dmg_radius = 0;
	helo->jam_time = level.time;

	for( i = 0; i < MAX_WEAPONS; i++ )
		helo->weapon[i] = helo->ammo[i] = 0;
}

void Spawn_AH64( edict_t *helo )
{
	SetHeloData( helo );

	VectorSet( helo->mins, -32, -16, -2 );
	VectorSet( helo->maxs, 22, 16, 10 );

	helo->HASCAN = HASCAN_AUTOPILOT | HASCAN_AUTOAIM | HASCAN_TAILDRAGGER | HASCAN_TERRAINLANDING;
	helo->ONOFF = ONOFF_GEAR;
	helo->topspeed = 150;	// speed without afterburner
	helo->accel = 7;
	helo->decel = 5;
	helo->speed = helo->stallspeed * 1.5;
	helo->maxthrust = THRUST_MILITARY;
	helo->minthrust = 0;
	helo->mass = 5000;

	helo->max_health = helo->health = 160;
	helo->gib_health = -60;

	helo->guntype = &guntypes[GUN_GAU];

	helo->flares = helo->maxflares = 30;
	helo->weapon[0] = WEAPON_AUTOCANNON; helo->maxammo[0] = helo->ammo[0] = 250;
	strcpy( helo->weaponname[0], "Chaingun" ); helo->weaponicon[0] = gi.imageindex( "Aacann" );
	helo->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	helo->weapon[1] = WEAPON_STINGER; helo->maxammo[1] = helo->ammo[1] = 4;
	strcpy( helo->weaponname[1], "Stinger" ); helo->weaponicon[1] = gi.imageindex( "Stinger" );
	helo->weaponicon_dam[1] = gi.imageindex( "stinger_dam" );
	helo->weapon[2] = WEAPON_HELLFIRE; helo->maxammo[2] = helo->ammo[2] = 8;
	strcpy( helo->weaponname[2], "Hellfire" ); helo->weaponicon[2] = gi.imageindex( "Hellfire" );
	helo->weaponicon_dam[2] = gi.imageindex( "hellfire_dam" );
	helo->weapon[3] = WEAPON_FFAR; helo->maxammo[3] = helo->ammo[3] = 16;
	strcpy( helo->weaponname[3], "FFAR" ); helo->weaponicon[3] = gi.imageindex( "FFAR" );
	helo->weaponicon_dam[3] = gi.imageindex( "ffar_dam" );




	helo->s.modelindex = gi.modelindex ("vehicles/helos/attack/ah64/rotors.md2");
	helo->s.modelindex2 = gi.modelindex ("vehicles/helos/attack/ah64/tris.md2");

	VectorSet( helo->movedir, -50, 15, 0 );		// camera position
	VectorSet( helo->turnspeed, 80, 100, 130 );
}



void Spawn_MI28( edict_t *helo )
{
	SetHeloData( helo );

	VectorSet( helo->mins, -28, -16, -10 );
	VectorSet( helo->maxs, 18, 16, 5 );

	helo->HASCAN = HASCAN_AUTOPILOT | HASCAN_AUTOAIM | HASCAN_TAILDRAGGER | HASCAN_TERRAINLANDING | HASCAN_EASTERN;
	helo->ONOFF = ONOFF_GEAR;
	helo->topspeed = 150;	// speed without afterburner
	helo->accel = 7;
	helo->decel = 5;
	helo->speed = helo->stallspeed * 1.5;
	helo->maxthrust = THRUST_MILITARY;
	helo->minthrust = 0;
	helo->mass = 5000;

	helo->max_health = helo->health = 160;
	helo->gib_health = -60;

	helo->guntype = &guntypes[GUN_GAU];

	helo->flares = helo->maxflares = 30;
	helo->weapon[0] = WEAPON_AUTOCANNON; helo->maxammo[0] = helo->ammo[0] = 350;
	strcpy( helo->weaponname[0], "Autocannon" ); helo->weaponicon[0] = gi.imageindex( "Aacann" );
	helo->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	helo->weapon[1] = WEAPON_STINGER; helo->maxammo[1] = helo->ammo[1] = 2;
	strcpy( helo->weaponname[1], "Stinger" ); helo->weaponicon[1] = gi.imageindex( "Stinger" );
	helo->weaponicon_dam[1] = gi.imageindex( "stinger_dam" );
	helo->weapon[2] = WEAPON_HELLFIRE; helo->maxammo[2] = helo->ammo[2] = 8;
	strcpy( helo->weaponname[2], "Hellfire" ); helo->weaponicon[2] = gi.imageindex( "Hellfire" );
	helo->weaponicon_dam[2] = gi.imageindex( "hellfire_dam" );
	helo->weapon[3] = WEAPON_FFAR; helo->maxammo[3] = helo->ammo[3] = 32;
	strcpy( helo->weaponname[3], "FFAR" ); helo->weaponicon[3] = gi.imageindex( "FFAR" );
	helo->weaponicon_dam[3] = gi.imageindex( "ffar_dam" );


	helo->s.modelindex = gi.modelindex ("vehicles/helos/attack/havoc/rotors.md2");
	helo->s.modelindex2 = gi.modelindex ("vehicles/helos/attack/havoc/tris.md2");

	VectorSet( helo->movedir, -50, 15, 0 );		// camera position
	VectorSet( helo->turnspeed, 80, 100, 130 );
}

void Spawn_RAH66( edict_t *helo )
{
	SetHeloData( helo );

	VectorSet( helo->mins, -31, -16, -1 );
	VectorSet( helo->maxs, 21, 16, 7 );

	helo->HASCAN = HASCAN_AUTOPILOT | HASCAN_AUTOAIM | HASCAN_TAILDRAGGER | HASCAN_TERRAINLANDING 
			| HASCAN_GEAR | HASCAN_WEAPONBAY | HASCAN_STEALTH; 
	helo->ONOFF = ONOFF_GEAR;
	helo->topspeed = 150;	// speed without afterburner
	helo->accel = 7;
	helo->decel = 5;
	helo->speed = helo->stallspeed * 1.5;
	helo->maxthrust = THRUST_MILITARY;
	helo->minthrust = 0;
	helo->mass = 5000;

	helo->max_health = helo->health = 160;
	helo->gib_health = -60;

	helo->guntype = &guntypes[GUN_MAUSER];

	helo->flares = helo->maxflares = 30;
	helo->weapon[0] = WEAPON_AUTOCANNON; helo->maxammo[0] = helo->ammo[0] = 300;
	strcpy( helo->weaponname[0], "Autocannon" ); helo->weaponicon[0] = gi.imageindex( "Aacann" );
	helo->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	helo->weapon[1] = WEAPON_STINGER; helo->maxammo[1] = helo->ammo[1] = 8;
	strcpy( helo->weaponname[1], "Stinger" ); helo->weaponicon[1] = gi.imageindex( "Stinger" );
	helo->weaponicon_dam[1] = gi.imageindex( "stinger_dam" );


	helo->s.modelindex = gi.modelindex ("vehicles/helos/scout/comanche/rotors.md2");
	helo->s.modelindex2 = gi.modelindex ("vehicles/helos/scout/comanche/tris.md2");

	VectorSet( helo->movedir, -50, 15, 0 );		// camera position
	VectorSet( helo->turnspeed, 80, 100, 130 );
}

void Spawn_KA50( edict_t *helo )
{
	SetHeloData( helo );

	VectorSet( helo->mins, -26, -12, 3 );
	VectorSet( helo->maxs, 14, 12, 8 );

	helo->HASCAN = HASCAN_AUTOPILOT | HASCAN_GEAR | HASCAN_TAILDRAGGER |HASCAN_TERRAINLANDING |HASCAN_EASTERN;
	helo->ONOFF = ONOFF_GEAR;
	helo->topspeed = 150;	// speed without afterburner
	helo->accel = 7;
	helo->decel = 5;
	helo->speed = helo->stallspeed * 1.5;
	helo->maxthrust = THRUST_MILITARY;
	helo->minthrust = 0;
	helo->mass = 5000;

	helo->max_health = helo->health = 160;
	helo->gib_health = -60;

	helo->guntype = &guntypes[GUN_MAUSER];

	helo->flares = helo->maxflares = 25;
	helo->weapon[0] = WEAPON_AUTOCANNON; helo->maxammo[0] = helo->ammo[0] = 350;
	strcpy( helo->weaponname[0], "Autocannon" ); helo->weaponicon[0] = gi.imageindex( "Aacann" );
	helo->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	helo->weapon[1] = WEAPON_FFAR; helo->maxammo[1] = helo->ammo[1] = 24;
	strcpy( helo->weaponname[1], "FFAR" ); helo->weaponicon[1] = gi.imageindex( "FFAR" );
	helo->weaponicon_dam[1] = gi.imageindex( "ffar_dam" );

	helo->s.modelindex = gi.modelindex ("vehicles/helos/scout/hocum/rotors.md2");
	helo->s.modelindex2 = gi.modelindex ("vehicles/helos/scout/hocum/tris.md2");

	VectorSet( helo->movedir, -50, 15, 0 );		// camera position
	VectorSet( helo->turnspeed, 80, 100, 130 );
}




// ===========================================
//					LQM
// ===========================================


void SetLQMData(edict_t *LQM )
{
	int i;

	LQM->random = level.time;
	LQM->recharge_time = level.time;
	LQM->classname = "LQM";
	LQM->deadflag = DEAD_NO;
	LQM->takedamage = DAMAGE_AIM;
	LQM->movetype = MOVETYPE_LQM;
	LQM->solid = SOLID_BBOX;
	LQM->clipmask = MASK_PLAYERSOLID;
	LQM->ONOFF = 0;
	LQM->gravity = 0.1;
	LQM->stallspeed = 0;
	LQM->thrust = THRUST_IDLE;
	LQM->ideal_yaw = 0; // gun direction
	LQM->think = SteerLQM;
	LQM->nextthink = level.time + 0.1;
	LQM->touch = LQM_touch;
	LQM->pain = LQM_pain;
	LQM->die = LQM_die;
	LQM->active_weapon = 1;
	LQM->random = 0;		// sound time
	LQM->volume = level.time + 0.2;
	LQM->attenuation = level.time + 0.2;
	LQM->dmg_radius = 0;
	LQM->jam_time = level.time;

	for( i = 0; i < MAX_WEAPONS; i++ )
		LQM->weapon[i] = LQM->ammo[i] = 0;
}

void Spawn_LQM (edict_t *LQM )
{
	SetLQMData( LQM );

	VectorSet( LQM->mins, -1, -1, -2 );
	VectorSet( LQM->maxs, 1, 1, 4 );

	LQM->health = LQM->max_health = 15;
	LQM->gib_health = -1;
	LQM->mass = 50;

	LQM->HASCAN = 0;
	LQM->DAMAGE = DAMAGE_NONE;

	LQM->accel = 2.0;
	LQM->decel = 12.0;
	LQM->speed = 0;	
	LQM->maxthrust = 4;
	LQM->minthrust = -1;

	LQM->guntype = &guntypes[GUN_LIGHT_RIFLE];

	LQM->flares = LQM->maxflares = 3;
	LQM->weapon[0] = WEAPON_AUTOCANNON; LQM->maxammo[0] = LQM->ammo[0] = 100;
	strcpy( LQM->weaponname[0], "MP5" ); LQM->weaponicon[0] = gi.imageindex( "Aacann" );
	LQM->weaponicon_dam[0] = gi.imageindex( "aacann_dam" );
	LQM->weapon[1] = WEAPON_STINGER; LQM->maxammo[1] = LQM->ammo[1] = 1;
	strcpy( LQM->weaponname[1], "Stinger" ); LQM->weaponicon[1] = gi.imageindex( "Stinger" );
	LQM->weaponicon_dam[1] = gi.imageindex( "stinger_dam" );

	VectorSet( LQM->turnspeed, 150, 50, 200 );
	VectorSet( LQM->movedir, -10, 5, 0 );		// camera position
	LQM->s.modelindex = gi.modelindex ("vehicles/infantry/pilot/pilot.md2");
	LQM->s.modelindex2 = gi.modelindex( "vehicles/infantry/pilot/mp5.md2" );
}

