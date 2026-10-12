#ifndef __BOT_SKILL_H__
#define __BOT_SKILL_H__

// What skill does to a bot: how well it perceives and how precisely it acts, never what it decides. Every use of skill
// goes through here, so the trees and the goal table never read it.
struct Handicap
{
	float	reactionScale = 1.0f;	// on the character's reaction time to a new enemy
	float	aimWobble = 0.0f;	// degrees the aim wanders while held
	float	flareChance = 1.0f;	// that a flare goes the moment it should
	float	breakLeadTime = 1.2f;	// seconds before impact the hard pull away from a missile comes
	float	gunnerSpread = 0.6f;	// degrees a turret gunner's aim wanders about the target
	int		scanPeriod = 1500;	// milliseconds between looks round behind
	float	bombError = 250.0f;	// how far off the release point a bomb may still be let go
	float	sightShare = 1.0f;	// of the full distance at which an enemy can be seen
	int		reconsiderEvery = 1000;	// milliseconds between reviews of its goal
};

#endif
