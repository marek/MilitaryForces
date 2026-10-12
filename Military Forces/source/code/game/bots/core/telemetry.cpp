#include "../../g_local.h"
#include "../../g_level.h"
#include "telemetry.h"
#include <cstdlib>
#include <map>
#include <string>

#ifdef MF_BOT_DEBUG

namespace
{
	std::map<std::string, int> goalFrames;
	std::map<std::string, int> leafFrames;
	std::map<std::string, float> weights;
	int weightsSeen = -1;

	const char* goalName( Bots::Goal goal )
	{
		constexpr const char* names[] = { "roam", "attack", "evade", "climb", "hide", "rearm", "capture" };
		return names[static_cast<int>( goal )];
	}

	// "name=scale,name=scale"
	void parseWeights()
	{
		weights.clear();
		const std::string text = g_botWeights.string;
		size_t start = 0;
		while( start < text.size() )
		{
			size_t end = text.find( ',', start );
			if( end == std::string::npos )
				end = text.size();
			const std::string pair = text.substr( start, end - start );
			const size_t eq = pair.find( '=' );
			if( eq != std::string::npos )
				weights[pair.substr( 0, eq )] = static_cast<float>( atof( pair.substr( eq + 1 ).c_str() ) );
			start = end + 1;
		}
	}
}

void BotTelemetry::frame( Bots::Goal goal, const char* tree, const char* leaf )
{
	goalFrames[goalName( goal )]++;
	leafFrames[std::string( tree ) + "/" + leaf]++;
}

void BotTelemetry::print()
{
	for( const auto& [name, frames] : goalFrames )
		Common::printf( "BOTSTAT goal %s %d\n", name.c_str(), frames );
	for( const auto& [name, frames] : leafFrames )
		Common::printf( "BOTSTAT leaf %s %d\n", name.c_str(), frames );
}

void BotTelemetry::reset()
{
	goalFrames.clear();
	leafFrames.clear();
}

float BotTelemetry::weight( const char* term )
{
	if( g_botWeights.modificationCount != weightsSeen )
	{
		weightsSeen = g_botWeights.modificationCount;
		parseWeights();
	}
	const auto found = weights.find( term );
	return found == weights.end() ? 1.0f : found->second;
}

#else

void BotTelemetry::frame( Bots::Goal, const char*, const char* ) {}
void BotTelemetry::print() {}
void BotTelemetry::reset() {}
float BotTelemetry::weight( const char* ) { return 1.0f; }

#endif
