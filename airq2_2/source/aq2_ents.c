/************************************************/
/* Filename: aq2_ents.cpp					    */
/* Author:	 Bjoern Drabeck						*/
/* Date:	 1999-07-04							*/
/*												*/
/* Description:									*/
/* Holds the following AirQuake2 entities:		*/
/* func_runway									*/
/*												*/
/************************************************/

#include "g_local.h"







/*QUAKED func_runway (.5 .5 .5) ?
The runway.
*/

void SP_func_runway (edict_t *self)
{
	self->movetype = MOVETYPE_NONE;
	self->solid  = SOLID_BBOX;
	self->health = DAMAGE_NO; 		// will be changed later so that
									// enemy can destroy runways
	self->classname = "func_runway";
	gi.setmodel( self, self->model );
	gi.linkentity( self );
}


/*QUAKED misc_camera (.5 .5 .5) (-16 -16 -16) (16 16 16)
Camera
*/
void SP_misc_camera (edict_t *self)
{
	self->s.skinnum = 0;
	self->solid = SOLID_NOT;	

	VectorSet (self->mins, -8, -8, -8);
	VectorSet (self->maxs, 8, 8, 8);

	gi.linkentity (self);
}






// ----------------------
// AIRCRAFT CARRIER STUFF
// ----------------------

void InitTrigger (edict_t *self);

/*QUAKED func_catapult (.5 .5 .5) ?
Catapult
*/
void catapult_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	float diff;

	if( strcmp( other->classname, "plane" ) != 0 )
		return;
	if( other->ONOFF & ONOFF_AIRBORNE )
		return;
	if( !self->ONOFF )
		return;
	if( !self->enemy || self->enemy != other )
		return;
	if( (other->speed > other->stallspeed + 10) ||
		(other->ONOFF & ONOFF_AIRBORNE) )
	{
		self->ONOFF = 0;
		self->enemy = NULL;
	}

	diff = other->s.angles[1] - self->angle;
	if( diff > 360 )
		diff -= 180;
	else if( diff < -360 )
		diff += 180;
	if( fabs(diff) > 8 )
		return;

	other->speed += 3;
}

void activate_catapult (edict_t *ent, edict_t *other, edict_t *activator)
{
	if( ent->enemy && ent->ONOFF )
		return;
	if( !ent->enemy )
	{
		ent->enemy = activator;
		gi.cprintf( activator->owner, PRINT_MEDIUM, "Line up to heading %.0f and report ready!\n", ent->angle );
	}
	else
	{
		ent->ONOFF = 1;
	}
}

void SP_func_catapult (edict_t *self)
{
	vec3_t	temp;

	InitTrigger( self );
	vectoangles( self->movedir, temp );
	self->angle = temp[1];
	self->classname = "func_catapult";
	self->touch = catapult_touch;
	self->use = activate_catapult;
	gi.linkentity (self);
}


/*QUAKED func_wires (.5 .5 .5) ?
Arrestor Wires
*/
void wires_touch( edict_t *self, edict_t *other, cplane_t *plane, csurface_t *surf )
{
	float diff;

	if( strcmp( other->classname, "plane" ) != 0 )
		return;
	if( other->ONOFF & ONOFF_AIRBORNE )
		return;
	if( other->speed < 20 )
		return;
	
	diff = other->s.angles[1] - self->angle;
	if( diff > 360 )
		diff -= 180;
	else if( diff < -360 )
		diff += 180;
	if( fabs(diff) > 15 )
		return;

	other->speed -= 3;
}

void SP_func_wires (edict_t *self)
{
	vec3_t temp;

	InitTrigger( self );
	vectoangles( self->movedir, temp );
	self->angle = temp[1];
	self->classname = "func_wires";
	self->touch = wires_touch;
	gi.linkentity (self);
}
