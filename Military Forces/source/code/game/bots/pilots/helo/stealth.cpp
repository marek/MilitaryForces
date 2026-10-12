#include "helo.h"

behavior::Status HeloTasks::breakOff( Mind& c )
{
	c.view[YAW] = c.sense.enemyAngles[YAW] + 180.0f;
	c.bot.cover.exposed = false;
	c.weapons.cover = true;
	c.steer.evading = true;
	c.steer.wantHeight = 90.0f;
	c.steer.mode = Run;
	return Status::Running;
}

// towards the enemy, low, weapons held: nothing that would show it before it is ready
behavior::Status HeloTasks::stalk( Mind& c )
{
	closeOnEnemy( c );
	c.weapons.cover = true;
	c.steer.wantHeight = 90.0f;
	return Status::Running;
}

// in view at its firing place. It goes back down as soon as a missile has left, or when the time is up
behavior::Status HeloTasks::strikeExposed( Mind& c )
{
	faceEnemy( c );
	c.steer.standoff = c.sense.enemyDist;
	c.steer.wantHeight = std::max( c.sense.height, heloLowHeight );
	if( guidedAmmo( c ) < c.bot.cover.strikeAmmo || theLevel.time_ > c.bot.cover.popTime )
	{
		c.bot.cover.exposed = false;
		c.bot.cover.time = theLevel.time_ + 4000 + static_cast<int>( 3000 * c.who().caution );
		VectorCopy( c.ent->r.currentOrigin, c.bot.cover.lastPop );
		c.bot.cover.has = Bots::findHidePoint( c.ent, c.bot, c.sense.enemy );
		// cover right where it has just been seen is no cover: go along the line across the enemy's view instead
		if( !c.bot.cover.has || Distance( c.bot.cover.point, c.bot.cover.lastPop ) < sameSpot )
		{
			Vec3 right;
			const Vec3 facing( 0, c.sense.enemyAngles[YAW], 0 );
			AngleVectors( facing, nullptr, right, nullptr );
			const Vec3 aside = Vec3( c.ent->r.currentOrigin ) + right * (c.bot.breakSide * sidestep);
			VectorCopy( aside, c.bot.cover.point );
			c.bot.cover.has = true;
			c.bot.breakSide = -c.bot.breakSide;
		}
	}
	return Status::Running;
}

behavior::Status HeloTasks::exposeForStrike( Mind& c )
{
	bool guided = false;
	const int slot = BotWeapons::findWeaponSlot( c.client->vehicle_, c.ps, c.sense.enemyAir, c.sense.enemyDist, guided );
	const int lockDelay = slot >= 0 ? availableWeapons[c.veh.weapons[slot]].lockdelay : 0;
	c.bot.cover.exposed = true;
	c.bot.cover.strikeAmmo = guidedAmmo( c );
	c.bot.cover.popTime = theLevel.time_ + lockDelay + 2500 + static_cast<int>( 2000 * c.who().patience );
	return strikeExposed( c );
}
