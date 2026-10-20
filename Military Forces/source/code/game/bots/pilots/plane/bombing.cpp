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

// fly over the target at height and let the bombing step choose the release, from the safest run-in
behavior::Status PlaneTasks::levelBomb( Mind& c )
{
	roam( c );
	const Vec3 target( c.sense.enemy->r.currentOrigin );
	if( flyToRunIn( c, target ) )
		return Status::Running;
	c.steer.hasDest = false;
	c.steer.headingFixed = true;
	BotWeapons::bomb( c.ent, c.bot, c.sense.enemy->r.currentOrigin, c.cmd, c.view );
	return Status::Running;
}

// Still far off: to the start of the run first. Of eight bearings round the target at run-in distance, the one whose
// way in crosses the least of the team's threat map, the nearer one when they are alike; chosen afresh now and then
bool PlaneTasks::flyToRunIn( Mind& c, const Vec3& target )
{
	constexpr float runIn = 4500.0f, onTheRun = 0.8f, reached = 1200.0f, nearer = 0.0002f;
	constexpr int rechoose = 20000;
	Vec3 flat = target - Vec3( c.ent->r.currentOrigin );
	flat[2] = 0;
	if( flat.length() < runIn * onTheRun )
		return false;
	if( theLevel.time_ >= c.bot.pursuit.runInTime )
	{
		const PathDanger danger = c.danger( true );
		float best = 0.0f;
		for( int k = 0; k < 8; k++ )
		{
			const Vec3 bearing = { 0, k * 45.0f, 0 };
			Vec3 dir;
			AngleVectors( bearing, dir, nullptr, nullptr );
			Vec3 start = target + dir * runIn;
			start[2] = c.ent->r.currentOrigin[2];
			const float score = danger.along( start, target ) + danger.factor( start ) + nearer * (start - Vec3( c.ent->r.currentOrigin )).length();
			if( !k || score < best )
			{
				best = score;
				c.bot.pursuit.runIn = start;
			}
		}
		c.bot.pursuit.runInTime = theLevel.time_ + rechoose;
	}
	if( (c.bot.pursuit.runIn - Vec3( c.ent->r.currentOrigin )).length() < reached )
		return false;
	vec3_t at;
	c.bot.pursuit.runIn.copyTo( at );
	c.headFor( at );
	c.travelTo( c.bot.pursuit.runIn );
	return true;
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
	if( flyToRunIn( c, target ) )
		return Status::Running;
	BotWeapons::bomb( c.ent, c.bot, target, c.cmd, c.view );
	return Status::Running;
}
