#ifndef __BOT_QUERY_H__
#define __BOT_QUERY_H__

#include "../../q_shared.h"
#include <functional>
#include <vector>

// One question for every "find me a place" - cover, hull down, a pop-up spot, a taxi spot: candidate points on rings
// round a centre, filters that rule a candidate out, scorers that rank the rest, and the best wins. The rings are
// searched nearest first, and the search stops at the first ring with a place in it, as a person looks nearby first.
// A kind of place is a list of filters and scorers.
struct PlaceQuery
{
	using Filter = std::function<bool( const Vec3& )>;
	using Scorer = std::function<float( const Vec3& )>;

	Vec3				centre;
	int					passEntity = -1;
	float				firstRing = 350.0f;
	float				ringStep = 450.0f;
	int					rings = 3;
	int					around = 12;	// candidates on each ring
	bool				onGround = true;	// drop each candidate onto the ground below it; water is no place
	float				climbShare = 0.5f;	// on the ground: no higher or lower than this share of the ring's radius
	std::vector<Filter>	filters;
	std::vector<Scorer>	scorers;	// summed, higher is better

	bool find( Vec3& out ) const;

	// common filters and scorers
	static Filter hiddenFrom( GameEntity* looker, GameEntity* threat );
	static Filter seenOverFrom( GameEntity* looker, GameEntity* threat, float height );
	static Scorer nearer( const Vec3& to );

private:
	bool candidate( float radius, float angle, Vec3& point ) const;
};

#endif
