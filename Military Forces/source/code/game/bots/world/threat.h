#ifndef __BOT_THREAT_H__
#define __BOT_THREAT_H__

#include "../../q_shared.h"
#include <array>
#include <vector>

// Each team's map of where it is dangerous to be: the enemy's SAM and flak sites by their reach, where team mates
// died in the last minutes, and enemies the team has seen, by what they can shoot at. Without teams, only the sites.
// Kept as a list of discs remade every two seconds and painted then onto a grid of points 256 units apart, one layer
// for aircraft and one for the surface, so that a question - and a path search asks thousands - is a look at the four
// points round it, whatever the number of discs.
class ThreatMap
{
public:
	// danger at a point for a vehicle of this team: 0 is none, 1 about the middle of an enemy SAM site's reach.
	// An aircraft is asked about with air set, anything on the ground or water without it
	static float danger( int team, const vec3_t point, bool air );
	// someone of this team died here
	static void noteDeath( int team, const vec3_t where );
	static void reset();

private:
	struct Disc
	{
		Vec3	centre;
		float	radius = 0;
		float	weight = 0;
		bool	air = false;	// a danger to aircraft
		bool	surface = false;	// a danger on the ground and water
	};
	struct Death
	{
		Vec3	where;
		int		time = 0;
	};
	static void refresh( int team );
	static void paint( int slot );
	static float sample( const std::vector<float>& layer, const vec3_t point );
	static void addSites( int team, std::vector<Disc>& out );
	static void addSightings( int team, std::vector<Disc>& out );
	static void addDeaths( int team, std::vector<Disc>& out );

	static std::array<std::vector<Disc>, 4>		discs_;
	static std::array<int, 4>					refreshTime_;
	static std::array<std::vector<Death>, 4>	deaths_;
	static constexpr float						gridStep = 256.0f;
	static std::array<std::vector<float>, 4>	airGrid_;
	static std::array<std::vector<float>, 4>	surfaceGrid_;
	static Vec3									gridMins_;
	static int									gridX_, gridY_;
};

#endif
