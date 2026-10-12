#include "plane.h"

// nose on the target, release when lined up and close with no team mate in the blast, then pull out
behavior::Status PlaneTasks::diveBomb( Mind& c )
{
	c.weapons.attacking = true;
	c.weapons.free = true;
	c.travelTo( c.enemyPosition() );
	aimAtEnemy( c );
	for( int slot : secondaryWeaponSlots )
	{
		const int weapon = c.veh.weapons[slot];
		if( weapon && c.ps.ammo[slot] > 0 && (availableWeapons[weapon].type == WT_IRONBOMB || availableWeapons[weapon].type == WT_GUIDEDBOMB) )
		{
			c.weapons.keep = slot;
			Vec3 nose;
			AngleVectors( c.ps.vehicleAngles, nose, nullptr, nullptr );
			const float blast = static_cast<float>( availableWeapons[weapon].damageRadius );
			if( c.ps.weaponNum == slot && DotProduct( nose, c.sense.enemyDir ) > 0.99f && c.sense.enemyDist < 1800 &&
				!BotWeapons::friendlyNear( c.ent, c.sense.enemy->r.currentOrigin, blast * 1.3f + 150.0f ) )
				c.cmd.buttons |= BUTTON_ATTACK_MAIN;
			break;
		}
	}
	if( c.sense.enemyDist < 1000 || c.sense.height < 700 )
		c.bot.pursuit.egressTime = theLevel.time_ + 6000;
	return Status::Running;
}

// fly over the target at height and let the bombing step choose the release
behavior::Status PlaneTasks::levelBomb( Mind& c )
{
	roam( c );
	c.steer.hasDest = false;
	c.steer.headingFixed = true;
	BotWeapons::bomb( c.ent, c.bot, c.sense.enemy->r.currentOrigin, c.cmd, c.view );
	return Status::Running;
}

// level-bomb a building
behavior::Status PlaneTasks::bombStructure( Mind& c )
{
	GameEntity* structure = Bots::findStructure( c.ent, c.bot );
	if( !structure )
		return Status::Failure;
	roam( c );
	Vec3 target;
	for( int k = 0; k < 3; k++ )
		target[k] = (structure->r.absmin[k] + structure->r.absmax[k]) * 0.5f;
	BotWeapons::bomb( c.ent, c.bot, target, c.cmd, c.view );
	return Status::Running;
}
