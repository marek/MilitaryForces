/*
 * $Id: g_weapon.c,v 1.15 2005-06-30 03:54:00 minkis Exp $
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
		fire_antiair( ent );
		break;
	case WT_ANTIGROUNDMISSILE:
	case WT_ANTIRADARMISSILE:
		fire_antiground( ent );
		break;
	case WT_ROCKET:
		fire_ffar( ent );
		break;
	case WT_IRONBOMB:
	case WT_GUIDEDBOMB:
		fire_ironbomb( ent );
		break;
	case WT_MACHINEGUN:
		fire_autocannon(ent, qtrue);
		if( availableVehicles[ent->client->vehicle].caps & HC_DUALGUNS ) {
			fire_autocannon(ent, qtrue);
		}
		break;
	case WT_BALLISTICGUN:
		fire_maingun( ent );
		break;
	case WT_FUELTANK:
		drop_fueltank( ent );
		if( availableWeapons[ent->s.weaponIndex].flags & WF_FIRE_IN_PAIRS )
			drop_fueltank( ent );
		break;
	case WT_FUELCRATE:
	case WT_AMMOCRATE:
	case WT_HEALTHCRATE:
		drop_crate( ent );
		break;
	case WT_FLAK:
		// No client flak firing yet
		break;
	case WT_NUKEBOMB:
		fire_nukebomb ( ent );
		break;
	case WT_NUKEMISSILE:
		fire_nukemissile ( ent ); 
		break;
	default:
// FIXME		G_Error( "Bad ent->s.weapon" );
		break;
	}
}

/*
===============
FireWeapon_GI
===============
*/
void FireWeapon_GI( gentity_t *ent ) {
	if(ent->s.eType != ET_MISC_VEHICLE)
		ent->client->accuracy_shots++;

	// fire the specific weapon
	switch( availableWeapons[ent->s.weaponIndex].type ) {
	case WT_ANTIAIRMISSILE:
		LaunchMissile_GI( ent );
		break;
	case WT_FLAK:
		fire_flak_GI ( ent );
		break;
	case WT_ANTIGROUNDMISSILE:
	case WT_ANTIRADARMISSILE:
	case WT_ROCKET:
	case WT_IRONBOMB:
	case WT_GUIDEDBOMB:
	case WT_BALLISTICGUN:
	case WT_FUELTANK:
	case WT_NUKEBOMB:
	case WT_NUKEMISSILE:
		break;
	case WT_MACHINEGUN:
		fire_autocannon_GI(ent);
		break;
	default:
// FIXME		G_Error( "Bad ent->s.weapon" );
		break;
	}
}


