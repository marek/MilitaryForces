#ifndef __BOT_HINTS_H__
#define __BOT_HINTS_H__

#include "../../q_shared.h"
#include <vector>

// Places on a map worth knowing: an ambush spot, a low corridor, a patrol station, a choke on the ground. A map may
// carry them as bot_hint entities; on a map without any they are worked out at load from the bases, the roam points
// and the terrain between, so a map needs nothing made for bots.
class BotHints
{
public:
	enum class Kind { Ambush, Corridor, Station, Choke };
	struct Hint
	{
		Vec3	at;
		Kind	kind = Kind::Station;
	};

	// a bot_hint entity with its "kind" key: ambush, corridor, station or choke
	static void fromEntity( const vec3_t origin, const char* kind );
	// one of the hints of a kind, drawn at random; nullptr when there is none
	static const Hint* any( Kind kind );
	static void reset();

private:
	static void build();
	static void generate();
	static void alongBaseLine( const Vec3& from, const Vec3& to );
	static bool baseOf( int team, Vec3& out );
	static bool groundBelow( Vec3& point );

	static std::vector<Hint>	hints_;
	static bool					built_;
	static bool					fromMap_;
};

#endif
