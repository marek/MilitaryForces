// file e_drone.c by Bjoern Drabeck

#include "g_local.h"
#include "e_drone.h"


/*
==============================================================================

enemy_drone
(AQ2 only)
==============================================================================
*/

/*QUAKED enemy_drone (1 1 0) (-16 -16 0) (16 16 16)
Spawns a fighter which follows
waypoints (path_corner)
*/


void SP_enemy_drone( edict_t *self )
{

	// only spawn if there is at least one waypoint
	if( !self->target )
	{
		G_FreeEdict( self );
		return;
	}

	// set general variables
	VectorSet( self->maxs, 24, 22, 14 );
	VectorSet( self->mins, -30, -22, -7 );
	self->solid = SOLID_BBOX;
	self->takedamage = DAMAGE_AIM;
	self->classname = "plane";
	self->movetype = MOVETYPE_FLYMISSILE;
	self->deadflag = DEAD_NO;
	self->clipmask = MASK_PLAYERSOLID;
	self->s.modelindex = gi.modelindex ("vehicles/planes/fighter/f16/tris.md2");
	strcpy( self->friendlyname, "F-16");

	self->health = self->max_health = 100;
	self->gib_health = -30;
	self->s.frame = 0;

	// set aq2 specific variables
	self->accel = 6.0;
	self->decel = 5.8;
	self->HASCAN = IS_DRONE;
	self->HASCAN |= HAS_AFTERBURNER;		// has afterburner ?
	self->HASCAN |= HAS_SPEEDBRAKES;		// has speedbrakes ?
	self->HASCAN |= HAS_LANDINGGEAR;		// has landing gear ?
	self->gundamage = 10;	// how strong is the autocannon
	self->topspeed = self->actualspeed =
		self->currenttop = 1000;
	self->speed = 550;
	self->turnspeed[0] = 140;
	self->turnspeed[2] = 160;
   	self->turnspeed[1] = 55;
	self->thrust = THRUST_100;
	self->goalentity = G_Find( NULL, FOFS(targetname), self->target );	// set waypoint

	// set functions
	self->touch = enemy_plane_touch;
	self->pain = enemy_plane_pain;
	self->die = enemy_plane_die;
	self->think = enemy_plane_fly;
	self->nextthink = level.time + 0.1;

	gi.linkentity( self );
}


void enemy_plane_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	if( self->deadflag == DEAD_DEAD || strcmp( other->classname, "freed" ) == 0
		|| strcmp( other->classname, "debris" ) == 0 )
		return;
	if( strcmp( other->classname, "rocket" ) == 0 ||
		strcmp( other->classname, "ATGM" ) == 0 ||
   		strcmp( other->classname, "sidewinder" ) == 0 ||
		strcmp( other->classname, "bolt" ) == 0 ||
		strcmp( other->classname, "shell" ) == 0 ||
		strcmp( other->classname, "bomb" ) == 0 )
		return;
	if( surf && (surf->flags & SURF_SKY) )
	{
//		self->health -= 5;
		if( self->health > 0 )
			return;
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		ThrowDebris( self, "models/objects/debris1/tris.md2",
			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris3/tris.md2",
//			5, self->s.origin );
		ThrowDebris( self, "models/objects/debris2/tris.md2",
			5, self->s.origin );
		SetVehicleModel( self );
		self->deadflag = DEAD_DEAD;
	}
	else if( strcmp( other->classname, "func_runway") == 0
   		 && ( self->ONOFF & LANDED ) )
		return;
	else if( ( strcmp( other->classname, "plane") == 0 || strcmp( other->classname, "helo" ) == 0
				|| strcmp( other->classname, "ground") == 0 )
				&& !(other->HASCAN & IS_DRONE) ) // drone
	{
		if( !self->deadflag )
		{
			if( self->enemy )
			{
				self->enemy->lockstatus = 0;
				self->enemy = NULL;
			}
			gi.WriteByte (svc_temp_entity);
			gi.WriteByte (TE_EXPLOSION1);
			gi.WritePosition (self->s.origin);
			gi.multicast (self->s.origin, MULTICAST_PVS);
			self->deadflag = DEAD_DEAD;
			ThrowDebris( self, "models/objects/debris1/tris.md2",
				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris2/tris.md2",
//				5, self->s.origin );
//			ThrowDebris( self, "models/objects/debris3/tris.md2",
//				5, self->s.origin );
			ThrowDebris( self, "models/objects/debris2/tris.md2",
				5, self->s.origin );
			SetVehicleModel( self );
		}
	}
	else
	{
		if( self->enemy )
		{
			self->enemy->lockstatus = 0;
			self->enemy = NULL;
		}
		self->health = 0;
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (self->s.origin);
		gi.multicast (self->s.origin, MULTICAST_PVS);
		ThrowDebris( self, "models/objects/debris1/tris.md2",
			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris2/tris.md2",
//			5, self->s.origin );
//		ThrowDebris( self, "models/objects/debris3/tris.md2",
//			5, self->s.origin );
		ThrowDebris( self, "models/objects/debris2/tris.md2",
			5, self->s.origin );
		SetVehicleModel( self );
		self->deadflag = DEAD_DEAD;
	gi.bprintf( PRINT_MEDIUM, "Enemy plane crashed.\n" );
	}
}


void enemy_plane_pain (edict_t *self, edict_t *other, float kick, int damage)
{
	self->s.effects = EF_ROCKET;
}



void enemy_plane_die (edict_t *self, edict_t *inflictor, edict_t *attacker, int damage, vec3_t point)
{

	if( attacker )
	{
		if( attacker != self && !(attacker->HASCAN & IS_DRONE) && attacker->owner)// drone
		{
			if( attacker->owner->client )
			{
				if( attacker->enemy )
				{
					if( attacker->enemy == self )
						attacker->enemy = NULL;
				}
				if( !self->deadflag )
				{
					team[attacker->owner->client->pers.team-1].score += 1;
					attacker->owner->client->resp.score += 1;
					if( !attacker->deadflag )
						gi.bprintf( PRINT_MEDIUM, "%s killed an enemy plane!\n",
						attacker->owner->client->pers.netname );
				}
			}
		}
	}


	gi.WriteByte (svc_temp_entity);
	gi.WriteByte (TE_EXPLOSION1);
	gi.WritePosition (self->s.origin);
	gi.multicast (self->s.origin, MULTICAST_PVS);

	self->deadflag = DEAD_DYING;
	self->thrust = THRUST_0;
	if(	strcmp( self->classname, "plane" ) == 0 )
	{
		self->avelocity[2] = 60;
		self->avelocity[1] = 0;
		if( self->ONOFF & LANDED )// landed
		{
			self->deadflag = DEAD_DEAD;
			SetVehicleModel( self );
		}
	}


}



void enemy_plane_fly( edict_t *drone )
{
	vec3_t	targetvector, forward;
	vec3_t	targetangles, distvector;
	float	difference;
	float	distance;

// 	delay for debugging
/*	if( level.time < 10 )
	{
		drone->nextthink = level.time + 0.1;
		return;
	}
*/
	// clear old values

//	gi.bprintf( PRINT_HIGH, "%d\n", drone->health );

	VectorClear( drone->avelocity );

	SetPlaneSpeed( drone );

	// health-effects
	if( drone->health >= 26 )
		drone->s.effects = 0;
	else if( drone->health < 26 && drone->health > 10 )
	{
		drone->s.effects = EF_GRENADE;
	}
	else if( drone->health <= 10 && drone->deadflag != DEAD_DEAD )
	{
		drone->s.effects = EF_ROCKET;
	}
	else
		drone->s.effects = 0;

	if( drone->health <= drone->gib_health && drone->deadflag != DEAD_DEAD )
	{
		gi.WriteByte (svc_temp_entity);
		gi.WriteByte (TE_EXPLOSION1);
		gi.WritePosition (drone->s.origin);
		gi.multicast (drone->s.origin, MULTICAST_PVS);
//		ThrowDebris( drone, "models/objects/debris1/tris.md2",
//			5, drone->s.origin );
//		ThrowDebris( drone, "models/objects/debris2/tris.md2",
//			5, drone->s.origin );
		ThrowDebris( drone, "models/objects/debris3/tris.md2",
			5, drone->s.origin );
		ThrowDebris( drone, "models/objects/debris2/tris.md2",
			5, drone->s.origin );
		SetVehicleModel(drone);
		drone->deadflag = DEAD_DEAD;
	}
	// unlock enemies on death
	if( drone->deadflag && drone->enemy )
	{
		drone->enemy->lockstatus = 0;
		drone->enemy = NULL;
	}
	// stop sound on death
	if( drone->deadflag == DEAD_DEAD )
	{
		gi.sound( drone, CHAN_BODY, gi.soundindex( "death/0.wav" ), 1, ATTN_NORM, 0 );
		return;
	}
	// spin dying plane
	else if( drone->deadflag == DEAD_DYING )
	{
		drone->avelocity[2] = (drone->s.angles[2]>=0) ? 60 : -60;
		vectoangles( drone->velocity, targetangles );
		drone->s.angles[0] = targetangles[0];
		drone->s.angles[1] = targetangles[1];
		drone->movetype = MOVETYPE_TOSS;
		drone->gravity = 0.1;
		drone->nextthink = level.time + 0.1;
		return;
	}

	// if you have a waypoint
	if( drone->goalentity )
	{
		VectorSubtract( drone->goalentity->s.origin, drone->s.origin, targetvector );
		VectorSet( distvector, targetvector[0], targetvector[1], 0 );
		distance = VectorLength( distvector );
		vectoangles( targetvector, targetangles );
		// if close to waypoint switch to next waypoint (horizontal ditance!)
		if( distance < 64 )
		{
			if( drone->goalentity->target )
			{
				drone->goalentity = G_Find( NULL, FOFS( targetname ), drone->goalentity->target );
			}
		}
	}
	else	// if not stay on your heading and find original waypoint
	{
		drone->goalentity = G_Find( NULL, FOFS(targetname), drone->target );
	}
//	testing:
//	VectorCopy( targetangles, drone->s.angles );
//	gi.bprintf( PRINT_HIGH, "%.1f %.1f\n", targetangles[0], targetangles[1] );

	// apply rudder and aileron
	difference = targetangles[1]-drone->s.angles[1];
	if( difference > 180 )
		difference -= 360;
	else if( difference < -180 )
		difference += 360;
	if( fabs(difference) >= 5 )
	{
		if( fabs(difference) > 50 )
			drone->avelocity[1] = (difference>0) ? drone->turnspeed[0] : -drone->turnspeed[2];
		else
			drone->avelocity[1] = (difference>0) ? 70 : -70;
		if( difference > 0 && drone->s.angles[2] < 85 )
			drone->avelocity[2] = drone->turnspeed[2];
		else if( difference < 0 && drone->s.angles[2] > -85 )
			drone->avelocity[2] = -drone->turnspeed[2];
		else
			drone->avelocity[2] = 0;
	}
	else
	{
		drone->s.angles[1] = targetangles[1];
		if( fabs(drone->s.angles[2]) > 10 )
			drone->avelocity[2] = (drone->s.angles[2]>0) ? -90 : 90;
		else if( fabs(drone->s.angles[2]) > 3 )
			drone->avelocity[2] = (drone->s.angles[2]>0) ? -40 : 40;
	}
	// apply elevator
	if( targetangles[0] > 180 )
		targetangles[0] -= 360;
	else if( targetangles[0] < -180 )
		targetangles[0] += 360;
	if( targetangles[0] > drone->s.angles[0] + 3 )
		drone->avelocity[0] = 40;
	else if( targetangles[0] < drone->s.angles[0] - 3 )
		drone->avelocity[0] = -40;
	else
		drone->s.angles[0] = targetangles[0];


//	debugging:
//	if( difference )
//		gi.bprintf( PRINT_HIGH, "%f\n", difference );
//	gi.bprintf( PRINT_HIGH, "waypoint: %s\n", drone->goalentity->targetname );


	// set velocity vector
	AngleVectors( drone->s.angles, forward, NULL, NULL );
	VectorScale( forward, drone->actualspeed/4 , drone->velocity );

	drone->nextthink = level.time + 0.1;
}
