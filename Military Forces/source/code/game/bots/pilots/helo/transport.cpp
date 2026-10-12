#include "helo.h"

// come down beside a team mate on foot and hold still so they can climb in
behavior::Status HeloTasks::pickUpWalker( Mind& c )
{
	flyTo( c, c.target->r.currentOrigin );
	c.steer.standoff = -300.0f;
	c.weapons.cover = true;
	if( c.steer.dist < 140 )
	{
		c.steer.hasDest = false;
		c.steer.mode = Hover;
		c.steer.hold = true;
		c.steer.wantHeight = -200.0f;
		if( !(c.ps.ONOFF & OO_GEAR) )
			c.cmd.buttons |= BUTTON_GEAR;
		if( c.landed() )
			c.steer.wait = true;
	}
	else
		c.steer.wantHeight = std::clamp( c.steer.dist * 0.3f, 60.0f, heloLowHeight );
	return Status::Running;
}

behavior::Status HeloTasks::waitForPassengers( Mind& c )
{
	c.steer.wait = true;
	return Status::Running;
}

// the weapon slot holding crates of that kind, or -1
int HeloTasks::crateSlot( const Mind& c, int type )
{
	for( int slot : allWeaponSlots )
	{
		const int weapon = c.veh.weapons[slot];
		if( weapon && c.ps.ammo[slot] > 0 && availableWeapons[weapon].type == type )
			return slot;
	}
	return -1;
}

// Carrying crates: find a team mate on the ground who is short of something we carry, a real player first,
// hover over them and drop it.
behavior::Status HeloTasks::resupply( Mind& c )
{
	constexpr float range = 8000.0f;
	constexpr float humanPreference = 0.3f;
	constexpr float dropHeight = 220.0f;
	constexpr int repeatDelay = 12000;
	if( g_gametype.integer < GT_TEAM || theLevel.time_ < c.bot.bombTime + 1500 )
		return Status::Failure;
	GameEntity* needy = nullptr;
	int slot = -1;
	float best = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == c.ent || !alive( other ) || !OnSameTeam( c.ent, other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		// crates land on the ground, so only someone who is down there can use them
		const bool flying = (availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) && !(other->client_->ps_.ONOFF & OO_LANDED);
		if( flying || (c.bot.structure == i && theLevel.time_ < c.bot.structureTime) )
			continue;
		// what they are short of; failing that, someone in a fight will want ammunition soon
		int need = BotWeapons::supplyNeed( other );
		const bool fighting = Bots::isBot( i ) ? Bots::states_[i].enemy >= 0 : other->health_ < other->client_->pers_.maxHealth_;
		float urgency = 1.0f;
		if( !need && fighting )
		{
			need = WT_AMMOCRATE;
			urgency = 2.5f;
		}
		const int carried = need ? crateSlot( c, need ) : -1;
		if( carried < 0 )
			continue;
		const float dist = Distance( c.ent->r.currentOrigin, other->r.currentOrigin );
		const float weighted = (Bots::isBot( i ) ? dist : dist * humanPreference) * urgency;
		if( dist < range && (!needy || weighted < best) )
		{
			needy = other;
			slot = carried;
			best = weighted;
		}
	}
	if( !needy )
		return Status::Failure;

	flyTo( c, needy->r.currentOrigin );
	c.steer.standoff = -300.0f;
	c.steer.wantHeight = std::max( dropHeight, heloLowHeight );
	c.cmd.weapon = slot;
	if( c.steer.dist < 120 )
	{
		c.steer.mode = Hover;
		if( c.ps.weaponNum == slot && c.sense.height > dropHeight * 0.6f )
		{
			c.cmd.buttons |= BUTTON_ATTACK_MAIN;
			c.bot.bombTime = theLevel.time_;
			// do not shower the same vehicle: remember who was just supplied
			c.bot.structure = needy->s.number;
			c.bot.structureTime = theLevel.time_ + repeatDelay;
		}
	}
	return Status::Running;
}
