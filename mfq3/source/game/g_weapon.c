/*
 * $Id: g_weapon.c,v 1.2 2001-12-22 02:28:44 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//
// g_weapon.c 
// perform the server side effects of a weapon firing

#include "g_local.h"

/*
================
G_BounceProjectile
================
*/
// might later be of use for MFQ3
void G_BounceProjectile( vec3_t start, vec3_t impact, vec3_t dir, vec3_t endout ) {
	vec3_t v, newv;
	float dot;

	VectorSubtract( impact, start, v );
	dot = DotProduct( v, dir );
	VectorMA( v, -2*dot, dir, newv );

	VectorNormalize(newv);
	VectorMA(impact, 8192, newv, endout);
}


/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/
void SnapVectorTowards( vec3_t v, vec3_t to ) {
	int		i;

	for ( i = 0 ; i < 3 ; i++ ) {
		if ( to[i] <= v[i] ) {
			v[i] = (int)v[i];
		} else {
			v[i] = (int)v[i] + 1;
		}
	}
}

/*
======================================================================

Anti air guided

======================================================================
*/

void Weapon_AntiAir_Fire (gentity_t *ent) {
	if( ent->client->ps.stats[STAT_LOCKINFO] & LI_LOCKING ) {
		fire_antiair(ent);
	} else {
		fire_ffar(ent);
	}
}

/*
======================================================================

Anti ground guided

======================================================================
*/

void Weapon_AntiGround_Fire (gentity_t *ent) {
	if( ent->client->ps.stats[STAT_LOCKINFO] & LI_LOCKING ) {
		fire_antiground(ent);
	} else {
		fire_ffar(ent);
	}
}

/*
======================================================================

FFAR

======================================================================
*/

void Weapon_FFAR_Fire (gentity_t *ent) {
	fire_ffar(ent);
}


/*
======================================================================

IronBomb

======================================================================
*/

void Weapon_IronBomb_Fire (gentity_t *ent) {
	fire_ironbomb(ent);
}


/*
======================================================================

Autocannon

======================================================================
*/

void Weapon_Autocannon_Fire (gentity_t *ent) {
	fire_autocannon(ent);

#pragma message("later add proper code for #barrels (also for other weapons (for ripples))")
	if( availableVehicles[ent->client->vehicle].caps & HC_DUALGUNS ) {
		fire_autocannon(ent);
	}
}



/*
======================================================================

Maingun

======================================================================
*/

void Weapon_Maingun_Fire (gentity_t *ent) {
	fire_maingun(ent);
}




//======================================================================


/*
===============
LogAccuracyHit
===============
*/
qboolean LogAccuracyHit( gentity_t *target, gentity_t *attacker ) {
	if( !target->takedamage ) {
		return qfalse;
	}

	if ( target == attacker ) {
		return qfalse;
	}

	if( !target->client ) {
		return qfalse;
	}

	if( !attacker->client ) {
		return qfalse;
	}

	if( target->client->ps.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( OnSameTeam( target, attacker ) ) {
		return qfalse;
	}

	return qtrue;
}


/*
===============
FireWeapon
===============
*/
void FireWeapon( gentity_t *ent ) {

	ent->client->accuracy_shots++;

	// fire the specific weapon
	switch( availableWeapons[ent->s.weaponIndex].type ) {
	case WT_ANTIAIRMISSILE:
		Weapon_AntiAir_Fire( ent );
		break;
	case WT_ANTIGROUNDMISSILE:
	case WT_ANTIRADARMISSILE:
		Weapon_AntiGround_Fire( ent );
		break;
	case WT_ROCKET:
		Weapon_FFAR_Fire( ent );
		break;
	case WT_IRONBOMB:
	case WT_GUIDEDBOMB:
		Weapon_IronBomb_Fire( ent );
		break;
	case WT_MACHINEGUN:
		Weapon_Autocannon_Fire( ent );
		break;
	case WT_BALLISTICGUN:
		Weapon_Maingun_Fire( ent );
		break;
	default:
// FIXME		G_Error( "Bad ent->s.weapon" );
		break;
	}
}


