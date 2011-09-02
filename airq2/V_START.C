
// file v_start.c by Bjoern Drabeck
// used for giving the vehicles individual characteristics!

#include "g_local.h"
#include "v_start.h"

void SetOnGround( edict_t *vehicle )
{
	trace_t	trace;
	vec3_t	below;

	VectorCopy( vehicle->s.origin, below );
	below[2] -= 2000;

	trace = gi.trace( vehicle->s.origin, NULL, NULL, below, vehicle, MASK_ALL );
	if( trace.fraction < 1.0 )
		vehicle->s.origin[2] = trace.endpos[2] + fabs(vehicle->mins[2]);

}

// =============================================
// PLANES
// =============================================

void Start_F16 (edict_t *vehicle )
{
	vec3_t maxs = { 24, 22, 14 };
	vec3_t mins = { -30, -22, -7 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 125;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 5.8;
	vehicle->HASCAN = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1020;
	vehicle->speed = 530;
	vehicle->flare = 25;
	vehicle->turnspeed[0] = 110;
	vehicle->turnspeed[2] = 260;
   	vehicle->turnspeed[1] = 60;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f16/tris.md2");
	strcpy( vehicle->friendlyname, "F-16" );
	vehicle->thrust = THRUST_100;
}

void Start_A10 (edict_t *vehicle )
{
	vec3_t maxs = { 28, 34, 5 };
	vec3_t mins = { -34, -34, -8 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 200;
	vehicle->gib_health = -60;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 5.5;
	vehicle->decel = 5.0;
	vehicle->HASCAN     = HAS_SPEEDBRAKES | HAS_LANDINGGEAR;
	vehicle->gundamage = 24;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 880;
	vehicle->speed = 350;
	vehicle->flare = 40;
	vehicle->turnspeed[0] = 80;
	vehicle->turnspeed[2] = 200;
   	vehicle->turnspeed[1] = 40;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/attack/A10/tris.md2");
	strcpy( vehicle->friendlyname ,"A-10");
	vehicle->thrust = THRUST_100;
}

void Start_Tornado (edict_t *vehicle )
{
	vec3_t maxs = { 35, 18, 14 };
	vec3_t mins = { -25, -18, -7 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 125;
	vehicle->gib_health = -40;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR;
	vehicle->gundamage = 18;	// how strong is the autocannon
	vehicle->gunfirerate = 0.2;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1140;
	vehicle->speed = 580;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 80;
	vehicle->turnspeed[2] = 200;
   	vehicle->turnspeed[1] = 40;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/attack/tornado/tris.md2");
	strcpy( vehicle->friendlyname, "GR.1");
	vehicle->thrust = THRUST_100;
}


void Start_Su24 (edict_t *vehicle )
{
	vec3_t maxs = { 41, 22, 12 };
	vec3_t mins = { -35, -22, -7 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 125;
	vehicle->gib_health = -40;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR | IS_EASTERN;
	vehicle->gundamage = 18;	// how strong is the autocannon
	vehicle->gunfirerate = 0.2;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1140;
	vehicle->speed = 580;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 65;
	vehicle->turnspeed[2] = 170;
   	vehicle->turnspeed[1] = 30;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/attack/su24/tris.md2");
	strcpy( vehicle->friendlyname, "Su-24");
	vehicle->thrust = THRUST_100;
}

void Start_SU25 (edict_t *vehicle )
{
	vec3_t maxs = { 26, 27, 12 };
	vec3_t mins = { -28, -27, -5 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 175;
	vehicle->gib_health = -40;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN   = HAS_SPEEDBRAKES | HAS_LANDINGGEAR | IS_EASTERN;
	vehicle->gundamage = 24;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 960;
	vehicle->speed = 400;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 95;
	vehicle->turnspeed[2] = 230;
   	vehicle->turnspeed[1] = 50;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/attack/su25/tris.md2");
	strcpy( vehicle->friendlyname, "Su-25");
	vehicle->thrust = THRUST_100;
}

void Start_F22 (edict_t *vehicle )
{
	vec3_t maxs = { 33, 20, 10 };
	vec3_t mins = { -28, -20, -5 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 100;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN 	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_WEAPONBAY | HAS_LANDINGGEAR;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1080;
	vehicle->speed = 560;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 95;
	vehicle->turnspeed[2] = 230;
   	vehicle->turnspeed[1] = 40;
	vehicle->stealth = 80;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f22/tris.md2");
	strcpy( vehicle->friendlyname, "F-22");
	vehicle->thrust = THRUST_100;
}

void Start_MiGMFI (edict_t *vehicle )
{
	vec3_t maxs = { 30, 22, 7 };
	vec3_t mins = { -32, -22, -7 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 100;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN 	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_WEAPONBAY
			| HAS_LANDINGGEAR | IS_EASTERN;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1140;
	vehicle->speed = 560;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 95;
	vehicle->turnspeed[2] = 230;
   	vehicle->turnspeed[1] = 40;
	vehicle->stealth = 80;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/fighter/migmfi/tris.md2");
	strcpy( vehicle->friendlyname, "MFI");
	vehicle->thrust = THRUST_100;
}

void Start_F117 (edict_t *vehicle )
{
	vec3_t maxs = { 30, 22, 6 };
	vec3_t mins = { -30, -22, -4 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 100;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN = HAS_WEAPONBAY | HAS_LANDINGGEAR;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 960;
	vehicle->speed = 490;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 65;
	vehicle->turnspeed[2] = 170;
	vehicle->turnspeed[1] = 30;
	vehicle->stealth = 90;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/attack/f117/tris.md2");
	strcpy( vehicle->friendlyname, "F-117");
	vehicle->thrust = THRUST_100;
}

void Start_F14 (edict_t *vehicle )
{
	vec3_t maxs = { 36, 22, 10 };
	vec3_t mins = { -30, -22, -5 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 150;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN 	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1200;
	vehicle->speed = 620;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 65;
	vehicle->turnspeed[2] = 170;
   	vehicle->turnspeed[1] = 30;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f14/tris.md2");
	strcpy( vehicle->friendlyname, "F-14");
	vehicle->thrust = THRUST_100;
}

void Start_F4 (edict_t *vehicle )
{
	vec3_t maxs = { 32, 16, 11 };
	vec3_t mins = { -37, -16, -5 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 150;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN 	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1020;
	vehicle->speed = 580;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 80;
	vehicle->turnspeed[2] = 200;
   	vehicle->turnspeed[1] = 40;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f4/tris.md2");
	strcpy( vehicle->friendlyname, "F-4");
	vehicle->thrust = THRUST_100;
}

void Start_SU37 (edict_t *vehicle )
{
	vec3_t maxs = { 36, 23, 10 };
	vec3_t mins = { -30, -23, -5 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 125;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN 	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR | IS_EASTERN;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1140;
	vehicle->speed = 600;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 95;
	vehicle->turnspeed[2] = 230;
   	vehicle->turnspeed[1] = 50;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/fighter/su37/tris.md2");
	strcpy( vehicle->friendlyname, "Su-37");
	vehicle->thrust = THRUST_100;
}

void Start_SU34 (edict_t *vehicle )
{
	vec3_t maxs = { 24, 16, 5 };
	vec3_t mins = { -26, -16, -5 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 125;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN 	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR | IS_EASTERN;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 1140;
	vehicle->speed = 600;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 95;
	vehicle->turnspeed[2] = 230;
   	vehicle->turnspeed[1] = 50;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/attack/su34/tris.md2");
	strcpy( vehicle->friendlyname, "Su-34");
	vehicle->thrust = THRUST_100;
}

void Start_MiG21 (edict_t *vehicle )
{
	vec3_t maxs = { 36, 15, 10 };
	vec3_t mins = { -34, -15, -7 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 175;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN 	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR | IS_EASTERN;
	vehicle->gundamage = 18;	// how strong is the autocannon
	vehicle->gunfirerate = 0.2;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 900;
	vehicle->speed = 530;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 95;
	vehicle->turnspeed[2] = 230;
   	vehicle->turnspeed[1] = 50;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/fighter/mig21/tris.md2");
	strcpy( vehicle->friendlyname, "MiG 21");
	vehicle->thrust = THRUST_100;
}

void Start_MiG29 (edict_t *vehicle )
{
	vec3_t maxs = { 38, 24, 10 };
	vec3_t mins = { -30, -24, -5 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "plane";
	vehicle->health = vehicle->max_health = 125;
	vehicle->gib_health = -30;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN 	 = HAS_AFTERBURNER | HAS_SPEEDBRAKES | HAS_LANDINGGEAR | IS_EASTERN;
	vehicle->gundamage = 12;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 960;
	vehicle->speed = 560;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 110;
	vehicle->turnspeed[2] = 260;
   	vehicle->turnspeed[1] = 60;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/planes/fighter/mig29/tris.md2");
	strcpy( vehicle->friendlyname, "MiG 29");
	vehicle->thrust = THRUST_100;
}

// ==============================================
// HELOS
// ==============================================


void Start_AH64( edict_t *vehicle )
{
	vec3_t maxs = { 22, 16, 10 };
	vec3_t mins = { -32, -16, -3 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "helo";
	vehicle->health = vehicle->max_health = 160;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->ONOFF 	= GEARDOWN;	// gear extended
	vehicle->HASCAN	= HAS_AUTOAIM | IS_TAILDRAGGER;
	vehicle->gundamage = 24;	// how strong is the autocannon
	vehicle->gunfirerate = 0.2;
	vehicle->gunspread = 1.5;
	vehicle->topspeed = 560;
	vehicle->actualspeed =	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 80;
	vehicle->turnspeed[2] = 100;
   	vehicle->turnspeed[1] = 130;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/helos/attack/ah64/rotors.md2");
	vehicle->s.modelindex2 = gi.modelindex ("vehicles/helos/attack/ah64/tris.md2");
	strcpy( vehicle->friendlyname, "AH 64");
	vehicle->thrust = THRUST_50;

}

void Start_MI28( edict_t *vehicle )
{
	vec3_t maxs = { 18, 16, 5 };
	vec3_t mins = { -28, -16, -12 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "helo";
	vehicle->health = vehicle->max_health = 160;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->ONOFF 	|= GEARDOWN;	// gear extended
	vehicle->ONOFF		&= ~AUTOROTATION;
	vehicle->HASCAN	= HAS_AUTOAIM;
	vehicle->HASCAN |= IS_EASTERN;
	vehicle->HASCAN |= IS_TAILDRAGGER;
	vehicle->gundamage = 24;	// how strong is the autocannon
	vehicle->gunfirerate = 0.2;
	vehicle->gunspread = 1.5;
	vehicle->topspeed = 560;
	vehicle->actualspeed =	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 80;
	vehicle->turnspeed[2] = 100;
   	vehicle->turnspeed[1] = 130;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/helos/attack/havoc/rotors.md2");
	vehicle->s.modelindex2 = gi.modelindex ("vehicles/helos/attack/havoc/tris.md2");
	strcpy( vehicle->friendlyname, "Mi 28");
	vehicle->thrust = THRUST_50;

}



void Start_RAH66( edict_t *vehicle )
{
	vec3_t maxs = { 21, 16, 7 };
	vec3_t mins = { -31, -16, -5 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "helo";
	vehicle->health = vehicle->max_health = 125;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->ONOFF		&= ~AUTOROTATION;
	vehicle->HASCAN	= HAS_AUTOAIM;
	vehicle->HASCAN |= HAS_LANDINGGEAR;
	vehicle->HASCAN |= HAS_WEAPONBAY;
	vehicle->HASCAN |= IS_TAILDRAGGER;
	vehicle->gundamage = 18;	// how strong is the autocannon
	vehicle->gunfirerate = 0.2;
	vehicle->gunspread = 1.5;
	vehicle->topspeed = 640;
	vehicle->actualspeed =	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 100;
	vehicle->turnspeed[2] = 100;
   	vehicle->turnspeed[1] = 160;
	vehicle->stealth = 60;

	vehicle->s.modelindex = gi.modelindex ("vehicles/helos/scout/comanche/rotors.md2");
	vehicle->s.modelindex2 = gi.modelindex ("vehicles/helos/scout/comanche/tris.md2");
	strcpy( vehicle->friendlyname, "RAH 66");
	vehicle->thrust = THRUST_50;

}



void Start_KA50( edict_t *vehicle )
{
	vec3_t maxs = { 14, 12, 8 };
	vec3_t mins = { -26, -12, -2 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "helo";
	vehicle->health = vehicle->max_health = 125;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->ONOFF		&= ~AUTOROTATION;
	vehicle->HASCAN = HAS_LANDINGGEAR | IS_EASTERN;
	vehicle->gundamage = 18;	// how strong is the autocannon
	vehicle->gunfirerate = 0.2;
	vehicle->gunspread = 1.5;
	vehicle->topspeed = 640;
	vehicle->actualspeed =	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->flare = 30;
	vehicle->turnspeed[0] = 100;
	vehicle->turnspeed[2] = 100;
   	vehicle->turnspeed[1] = 160;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/helos/scout/hocum/rotors.md2");
	vehicle->s.modelindex2 = gi.modelindex ("vehicles/helos/scout/hocum/tris.md2");
	strcpy( vehicle->friendlyname, "KA-50");
	vehicle->thrust = THRUST_50;

}




// ===========================================
// GROUND
// ===========================================


void Start_M1 (edict_t *vehicle )
{
	vec3_t maxs = { 13.0, 5.0, 5.0 };
	vec3_t mins = { -11.0,-5.0, -3.0 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "ground";
	vehicle->health = vehicle->max_health = 325;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_GROUND;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->flare = 20;
	vehicle->HASCAN = HAS_SPEEDBRAKES;		// handbrake
	vehicle->gundamage = 7;	// how strong is the autocannon
	vehicle->gunfirerate = 0.01;
	vehicle->gunspread = 1.5;
	vehicle->maingundamage = 200;
	vehicle->topspeed = 240;
	vehicle->actualspeed = 0;
	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->turnspeed[0] = 50;
	vehicle->turnspeed[2] = 60;
   	vehicle->turnspeed[1] = 90;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/ground/tank/m1a2/tris.md2");
	strcpy( vehicle->friendlyname, "M1A2i");

	vehicle->thrust = THRUST_0;
	// turret
	vehicle->activator = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->activator->s.angles );
	VectorSet( vehicle->activator->mins, -5, -5, 1);
	VectorSet( vehicle->activator->maxs, 5, 5, 5 );
	vehicle->activator->s.modelindex = gi.modelindex("vehicles/ground/tank/m1a2/turret.md2");
	vehicle->activator->owner = vehicle;
	vehicle->activator->takedamage = DAMAGE_NO;
	vehicle->activator->deadflag = DEAD_NO;
	vehicle->activator->clipmask = MASK_PLAYERSOLID;
	vehicle->activator->solid = SOLID_BBOX;
	vehicle->activator->movetype = MOVETYPE_NONE;
//	ent->vehicle->activator->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->activator );

	// gun barrel
	vehicle->movetarget = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->movetarget->s.angles );
	VectorSet( vehicle->movetarget->mins, 1, 0, 3 );
	VectorSet( vehicle->movetarget->maxs, 22, 1, 4 );
	vehicle->movetarget->s.modelindex = gi.modelindex("vehicles/ground/tank/m1a2/gun.md2");
	vehicle->movetarget->owner = vehicle;
	vehicle->movetarget->takedamage = DAMAGE_NO;
	vehicle->movetarget->solid = SOLID_BBOX;
	vehicle->movetarget->deadflag = DEAD_NO;
	vehicle->movetarget->clipmask = MASK_PLAYERSOLID;
	vehicle->movetarget->movetype = MOVETYPE_NONE;
//	ent->vehicle->movetarget->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->movetarget );
}

void Start_T72 (edict_t *vehicle )
{
	vec3_t maxs = { 13.0, 6.0, 5.0 };
	vec3_t mins = { -13.0,-6.0, -2.0 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "ground";
	vehicle->health = vehicle->max_health = 325;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_GROUND;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->flare = 20;
	vehicle->HASCAN = HAS_SPEEDBRAKES;		// handbrake
	vehicle->HASCAN |= IS_EASTERN;
	vehicle->gundamage = 7;	// how strong is the autocannon
	vehicle->gunfirerate = 0.01;
	vehicle->gunspread = 1.5;
	vehicle->maingundamage = 200;
	vehicle->topspeed = 240;
	vehicle->actualspeed = 0;
	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->turnspeed[0] = 50;
	vehicle->turnspeed[2] = 60;
   	vehicle->turnspeed[1] = 90;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/ground/tank/t72/tris.md2");
	strcpy( vehicle->friendlyname, "T-72");

	vehicle->thrust = THRUST_0;
	// turret
	vehicle->activator = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->activator->s.angles );
	VectorSet( vehicle->activator->mins, -5, -5, 1);
	VectorSet( vehicle->activator->maxs, 5, 5, 5 );
	vehicle->activator->s.modelindex = gi.modelindex("vehicles/ground/tank/t72/turret.md2");
	vehicle->activator->owner = vehicle;
	vehicle->activator->takedamage = DAMAGE_NO;
	vehicle->activator->deadflag = DEAD_NO;
	vehicle->activator->clipmask = MASK_PLAYERSOLID;
	vehicle->activator->solid = SOLID_BBOX;
	vehicle->activator->movetype = MOVETYPE_NONE;
//	ent->vehicle->activator->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->activator );

	// gun barrel
	vehicle->movetarget = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->movetarget->s.angles );
	VectorSet( vehicle->movetarget->mins, 1, 0, 3 );
	VectorSet( vehicle->movetarget->maxs, 22, 1, 4 );
	vehicle->movetarget->s.modelindex = gi.modelindex("vehicles/ground/tank/t72/gun.md2");
	vehicle->movetarget->owner = vehicle;
	vehicle->movetarget->takedamage = DAMAGE_NO;
	vehicle->movetarget->solid = SOLID_BBOX;
	vehicle->movetarget->deadflag = DEAD_NO;
	vehicle->movetarget->clipmask = MASK_PLAYERSOLID;
	vehicle->movetarget->movetype = MOVETYPE_NONE;
//	ent->vehicle->movetarget->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->movetarget );
}


void Start_Scorpion (edict_t *vehicle )
{
	vec3_t maxs = { 9.0, 4.5, 3.0 };
	vec3_t mins = { -9.0,-4.5, -2.0 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "ground";
	vehicle->health = vehicle->max_health = 250;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_GROUND;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN = HAS_SPEEDBRAKES;		// handbrake
	vehicle->gundamage = 7;	// how strong is the autocannon
	vehicle->gunfirerate = 0.01;
	vehicle->gunspread = 1.5;
	vehicle->maingundamage = 110;
	vehicle->topspeed = 360;
	vehicle->actualspeed = 0;
	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->turnspeed[0] = 50;
	vehicle->turnspeed[2] = 60;
   	vehicle->turnspeed[1] = 100;
	vehicle->flare = 20;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/ground/scout/scorpion/tris.md2");
	strcpy( vehicle->friendlyname, "Scorpion");

	vehicle->thrust = THRUST_0;

	// turret
	vehicle->activator = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->activator->s.angles );
	VectorSet( vehicle->activator->mins, -8, -4, 3);
	VectorSet( vehicle->activator->maxs, 0, 4, 4.5 );
	vehicle->activator->s.modelindex = gi.modelindex("vehicles/ground/scout/scorpion/turret.md2");
	vehicle->activator->owner = vehicle;
	vehicle->activator->takedamage = DAMAGE_NO;
	vehicle->activator->deadflag = DEAD_NO;
	vehicle->activator->clipmask = MASK_PLAYERSOLID;
	vehicle->activator->solid = SOLID_BBOX;
	vehicle->activator->movetype = MOVETYPE_NONE;
//	ent->vehicle->activator->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->activator );

	// gun barrel
	vehicle->movetarget = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->movetarget->s.angles );
	VectorSet( vehicle->movetarget->mins, -2, 1, 3.5 );
	VectorSet( vehicle->movetarget->maxs, 6, 2, 3 );
	vehicle->movetarget->s.modelindex = gi.modelindex("vehicles/ground/scout/scorpion/gun.md2");
	vehicle->movetarget->owner = vehicle;
	vehicle->movetarget->takedamage = DAMAGE_NO;
	vehicle->movetarget->solid = SOLID_BBOX;
	vehicle->movetarget->deadflag = DEAD_NO;
	vehicle->movetarget->clipmask = MASK_PLAYERSOLID;
	vehicle->movetarget->movetype = MOVETYPE_NONE;
//	ent->vehicle->movetarget->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->movetarget );
}


void Start_BRDM2 (edict_t *vehicle )
{
	vec3_t maxs = { 9.0, 4.0, 6.0 };
	vec3_t mins = { -9.0,-4.0, -1.0 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "ground";
	vehicle->health = vehicle->max_health = 200;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_GROUND;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->HASCAN = HAS_SPEEDBRAKES | IS_EASTERN;		// handbrake
	vehicle->gundamage = 7;	// how strong is the autocannon
	vehicle->gunfirerate = 0.01;
	vehicle->gunspread = 1.5;
	vehicle->maingundamage = 110;
	vehicle->topspeed = 440;
	vehicle->actualspeed = 0;
	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->turnspeed[0] = 50;
	vehicle->turnspeed[2] = 60;
   	vehicle->turnspeed[1] = 100;
	vehicle->flare = 20;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/ground/scout/brdm2/tris.md2");
	strcpy( vehicle->friendlyname, "BRDM-2");

	vehicle->thrust = THRUST_0;

	// turret
	vehicle->activator = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->activator->s.angles );
	VectorSet( vehicle->activator->mins, -8, -4, 3);
	VectorSet( vehicle->activator->maxs, 0, 4, 4.5 );
	vehicle->activator->s.modelindex = gi.modelindex("vehicles/ground/scout/brdm2/turret.md2");
	vehicle->activator->owner = vehicle;
	vehicle->activator->takedamage = DAMAGE_NO;
	vehicle->activator->deadflag = DEAD_NO;
	vehicle->activator->clipmask = MASK_PLAYERSOLID;
	vehicle->activator->solid = SOLID_BBOX;
	vehicle->activator->movetype = MOVETYPE_NONE;
//	ent->vehicle->activator->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->activator );

	// gun barrel
	vehicle->movetarget = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->movetarget->s.angles );
	VectorSet( vehicle->movetarget->mins, -2, 1, 3.5 );
	VectorSet( vehicle->movetarget->maxs, 6, 2, 3 );
	vehicle->movetarget->s.modelindex = gi.modelindex("vehicles/ground/scout/brdm2/gun.md2");
	vehicle->movetarget->owner = vehicle;
	vehicle->movetarget->takedamage = DAMAGE_NO;
	vehicle->movetarget->solid = SOLID_BBOX;
	vehicle->movetarget->deadflag = DEAD_NO;
	vehicle->movetarget->clipmask = MASK_PLAYERSOLID;
	vehicle->movetarget->movetype = MOVETYPE_NONE;
//	ent->vehicle->movetarget->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->movetarget );
}


void Start_2S6 (edict_t *vehicle )
{
	vec3_t maxs = { 14.0, 9.0, 9.0 };
	vec3_t mins = { -16.0,-9.0, -2.0 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "ground";
	vehicle->health = vehicle->max_health = 200;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_GROUND;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->flare = 30;
	vehicle->HASCAN = HAS_SPEEDBRAKES;		// handbrake
	vehicle->HASCAN |= IS_EASTERN;
	vehicle->gundamage = 15;	// how strong is the autocannon
	vehicle->gunfirerate = 0.01;
	vehicle->gunspread = 3.0;
	vehicle->maingundamage = 0;
	vehicle->topspeed = 280;
	vehicle->actualspeed = 0;
	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->turnspeed[0] = 50;
	vehicle->turnspeed[2] = 60;
   	vehicle->turnspeed[1] = 90;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/ground/special/2s6/tris.md2");
	strcpy( vehicle->friendlyname, "2S6");

	vehicle->thrust = THRUST_0;

	// turret
	vehicle->activator = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->activator->s.angles );
	VectorSet( vehicle->activator->mins, -5, -5, 1);
	VectorSet( vehicle->activator->maxs, 5, 5, 5 );
	vehicle->activator->s.modelindex = gi.modelindex("vehicles/ground/special/2s6/turret.md2");
	vehicle->activator->owner = vehicle;
	vehicle->activator->takedamage = DAMAGE_NO;
	vehicle->activator->deadflag = DEAD_NO;
	vehicle->activator->clipmask = MASK_PLAYERSOLID;
	vehicle->activator->solid = SOLID_BBOX;
	vehicle->activator->movetype = MOVETYPE_NONE;
//	ent->vehicle->activator->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->activator );

	// gun barrel
	vehicle->movetarget = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->movetarget->s.angles );
	VectorSet( vehicle->movetarget->mins, 1, 0, 3 );
	VectorSet( vehicle->movetarget->maxs, 22, 1, 4 );
	vehicle->movetarget->s.modelindex = gi.modelindex("vehicles/ground/special/2s6/gun.md2");
	vehicle->movetarget->owner = vehicle;
	vehicle->movetarget->takedamage = DAMAGE_NO;
	vehicle->movetarget->solid = SOLID_BBOX;
	vehicle->movetarget->deadflag = DEAD_NO;
	vehicle->movetarget->clipmask = MASK_PLAYERSOLID;
	vehicle->movetarget->movetype = MOVETYPE_NONE;
//	ent->vehicle->movetarget->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->movetarget );
}



void Start_SA9 (edict_t *vehicle )
{
	vec3_t maxs = { 14.0, 9.0, 9.0 };
	vec3_t mins = { -16.0,-9.0, -2.0 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "ground";
	vehicle->health = vehicle->max_health = 250;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_GROUND;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->flare = 30;
	vehicle->HASCAN = HAS_SPEEDBRAKES;		// handbrake
	vehicle->HASCAN |= IS_EASTERN;
	vehicle->gundamage = 15;	// how strong is the autocannon
	vehicle->gunfirerate = 0.01;
	vehicle->gunspread = 3.0;
	vehicle->maingundamage = 0;
	vehicle->topspeed = 360;
	vehicle->actualspeed = 0;
	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->turnspeed[0] = 50;
	vehicle->turnspeed[2] = 60;
   	vehicle->turnspeed[1] = 90;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/ground/special/sa9/tris.md2");
	strcpy( vehicle->friendlyname, "SA-9");

	vehicle->thrust = THRUST_0;

	// turret
	vehicle->activator = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->activator->s.angles );
	VectorSet( vehicle->activator->mins, -5, -5, 1);
	VectorSet( vehicle->activator->maxs, 5, 5, 5 );
	vehicle->activator->s.modelindex = gi.modelindex("vehicles/ground/special/sa9/turret.md2");
	vehicle->activator->owner = vehicle;
	vehicle->activator->takedamage = DAMAGE_NO;
	vehicle->activator->deadflag = DEAD_NO;
	vehicle->activator->clipmask = MASK_PLAYERSOLID;
	vehicle->activator->solid = SOLID_BBOX;
	vehicle->activator->movetype = MOVETYPE_NONE;
//	ent->vehicle->activator->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->activator );

	// gun barrel
	vehicle->movetarget = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->movetarget->s.angles );
	VectorSet( vehicle->movetarget->mins, 1, 0, 3 );
	VectorSet( vehicle->movetarget->maxs, 22, 1, 4 );
	vehicle->movetarget->s.modelindex = gi.modelindex("vehicles/ground/special/sa9/gun.md2");
	vehicle->movetarget->owner = vehicle;
	vehicle->movetarget->takedamage = DAMAGE_NO;
	vehicle->movetarget->solid = SOLID_BBOX;
	vehicle->movetarget->deadflag = DEAD_NO;
	vehicle->movetarget->clipmask = MASK_PLAYERSOLID;
	vehicle->movetarget->movetype = MOVETYPE_NONE;
//	ent->vehicle->movetarget->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->movetarget );
}

void Start_LAV25 (edict_t *vehicle )
{
	vec3_t maxs = { 19.0, 5.0, 8.0 };
	vec3_t mins = { -6.0,-5.0, -1.0 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "ground";
	vehicle->health = vehicle->max_health = 250;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_GROUND;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->flare = 30;
	vehicle->HASCAN = HAS_SPEEDBRAKES;		// handbrake
	vehicle->gundamage = 15;	// how strong is the autocannon
	vehicle->gunfirerate = 0.01;
	vehicle->gunspread = 3.0;
	vehicle->maingundamage = 0;
	vehicle->topspeed = 360;
	vehicle->actualspeed = 0;
	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->turnspeed[0] = 50;
	vehicle->turnspeed[2] = 60;
   	vehicle->turnspeed[1] = 90;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/ground/special/lav25/tris.md2");
	strcpy( vehicle->friendlyname, "Lav25");

	vehicle->thrust = THRUST_0;

	// turret
	vehicle->activator = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->activator->s.angles );
	VectorSet( vehicle->activator->mins, -3, -3, 8);
	VectorSet( vehicle->activator->maxs, 3, 3, 5 );
	vehicle->activator->s.modelindex = gi.modelindex("vehicles/ground/special/lav25/turret.md2");
	vehicle->activator->owner = vehicle;
	vehicle->activator->takedamage = DAMAGE_NO;
	vehicle->activator->deadflag = DEAD_NO;
	vehicle->activator->clipmask = MASK_PLAYERSOLID;
	vehicle->activator->solid = SOLID_BBOX;
	vehicle->activator->movetype = MOVETYPE_NONE;
//	ent->vehicle->activator->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->activator );

	// gun barrel
	vehicle->movetarget = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->movetarget->s.angles );
	VectorSet( vehicle->movetarget->mins, -1, -1, 3 );
	VectorSet( vehicle->movetarget->maxs, 1, 1, 4 );
	vehicle->movetarget->s.modelindex = gi.modelindex("vehicles/ground/special/lav25/gun.md2");
	vehicle->movetarget->owner = vehicle;
	vehicle->movetarget->takedamage = DAMAGE_NO;
	vehicle->movetarget->solid = SOLID_BBOX;
	vehicle->movetarget->deadflag = DEAD_NO;
	vehicle->movetarget->clipmask = MASK_PLAYERSOLID;
	vehicle->movetarget->movetype = MOVETYPE_NONE;
//	ent->vehicle->movetarget->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->movetarget );
}


void Start_Rapier (edict_t *vehicle )
{
	vec3_t maxs = { 22.0, 5.0, 9.0 };
	vec3_t mins = { -4.0,-5.0, -1.0 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "ground";
	vehicle->health = vehicle->max_health = 200;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_GROUND;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 6.0;
	vehicle->decel = 6.2;
	vehicle->flare = 30;
	vehicle->HASCAN = HAS_SPEEDBRAKES;		// handbrake
	vehicle->gundamage = 15;	// how strong is the autocannon
	vehicle->gunfirerate = 0.01;
	vehicle->gunspread = 3.0;
	vehicle->maingundamage = 0;
	vehicle->topspeed = 280;
	vehicle->actualspeed = 0;
	vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->turnspeed[0] = 50;
	vehicle->turnspeed[2] = 60;
   	vehicle->turnspeed[1] = 90;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/ground/special/rapier/tris.md2");
	strcpy( vehicle->friendlyname, "Rapier");

	vehicle->thrust = THRUST_0;

	// turret
	vehicle->activator = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->activator->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->activator->s.angles );
	VectorSet( vehicle->activator->mins, -3, -3, 8);
	VectorSet( vehicle->activator->maxs, 3, 3, 5 );
	vehicle->activator->s.modelindex = gi.modelindex("vehicles/ground/special/rapier/turret.md2");
	vehicle->activator->owner = vehicle;
	vehicle->activator->takedamage = DAMAGE_NO;
	vehicle->activator->deadflag = DEAD_NO;
	vehicle->activator->clipmask = MASK_PLAYERSOLID;
	vehicle->activator->solid = SOLID_BBOX;
	vehicle->activator->movetype = MOVETYPE_NONE;
//	ent->vehicle->activator->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->activator );

	// gun barrel
	vehicle->movetarget = G_Spawn ();
	VectorCopy( vehicle->s.origin, vehicle->movetarget->s.origin );
	VectorCopy( vehicle->s.angles, vehicle->movetarget->s.angles );
	VectorSet( vehicle->movetarget->mins, -1, -1, 3 );
	VectorSet( vehicle->movetarget->maxs, 1, 1, 4 );
	vehicle->movetarget->s.modelindex = gi.modelindex("vehicles/ground/special/rapier/gun.md2");
	vehicle->movetarget->owner = vehicle;
	vehicle->movetarget->takedamage = DAMAGE_NO;
	vehicle->movetarget->solid = SOLID_BBOX;
	vehicle->movetarget->deadflag = DEAD_NO;
	vehicle->movetarget->clipmask = MASK_PLAYERSOLID;
	vehicle->movetarget->movetype = MOVETYPE_NONE;
//	ent->vehicle->movetarget->flags |= FL_TEAMSLAVE;
	gi.linkentity( vehicle->movetarget );
}



// ===========================================
// LQM
// ===========================================


void Start_LQM (edict_t *vehicle )
{
	vec3_t maxs = { 2, 2, 4 };
	vec3_t mins = { -2, -2, -2 };

	VectorCopy( mins, vehicle->mins );
	VectorCopy( maxs, vehicle->maxs );

	vehicle->classname = "LQM";
	vehicle->health = vehicle->max_health = 15;
	vehicle->gib_health = -1;

	vehicle->takedamage = DAMAGE_AIM;
	vehicle->movetype = MOVETYPE_FLYMISSILE;
	vehicle->solid = SOLID_BBOX;

	vehicle->accel = 2.0;
	vehicle->decel = 12.0;
	vehicle->HASCAN 	 = 0;
	vehicle->gundamage = 2;	// how strong is the autocannon
	vehicle->gunfirerate = 0.1;
	vehicle->gunspread = 1.25;
	vehicle->topspeed = vehicle->actualspeed =
		vehicle->currenttop = 0;
	vehicle->speed = 0;
	vehicle->flare = 0;
	vehicle->turnspeed[0] = 100;
	vehicle->turnspeed[2] = 100;
   	vehicle->turnspeed[1] = 100;
	vehicle->stealth = 0;

	vehicle->s.modelindex = gi.modelindex ("vehicles/infantry/pilot/pilot.md2");
	strcpy( vehicle->friendlyname, "Pilot");
	vehicle->thrust = THRUST_0;
}
