#ifndef __BOT_TELEMETRY_H__
#define __BOT_TELEMETRY_H__

#include "../bot.h"

// Debug builds: how bots spend their time, for soaks and weight sweeps. Every frame counts the goal each bot holds and
// the leaf its tree is in; botstats prints the counts. The goal terms can be scaled by g_botWeights
// ("aggression=1.2,missile=0.8") so a sweep can try weights without a rebuild.
namespace BotTelemetry
{
	void frame( Bots::Goal goal, const char* tree, const char* leaf );
	void print();
	void reset();
	// the scale g_botWeights sets on a goal term, 1 when it sets none
	float weight( const char* term );
}

#endif
