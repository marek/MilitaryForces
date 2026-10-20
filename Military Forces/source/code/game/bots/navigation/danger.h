#ifndef __BOT_DANGER_H__
#define __BOT_DANGER_H__

#include "../../q_shared.h"

// What a path search weighs besides distance: a team's threat map, by the caution of the one asking. A step through
// danger costs more, so the path goes round where the way round is not much longer; nobody refuses to go
struct PathDanger
{
	static constexpr float weight = 3.0f;	// on a step's cost, per unit of danger at full caution

	int		team = -1;	// none: the threat map is not asked
	float	caution = 0.0f;
	bool	air = false;

	// the factor on a step's cost at a point: 1 where nothing threatens
	float factor( const Vec3& at ) const;
	// the danger met along a straight line, sampled every thousand units, by caution
	float along( const Vec3& from, const Vec3& to ) const;
};

#endif
