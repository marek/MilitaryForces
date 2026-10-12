#ifndef __BOT_LOD_H__
#define __BOT_LOD_H__

#include "../bot.h"

// How much of its thinking a bot does this frame. Its controls are filled in every frame. A bot near a person, with an
// enemy, or lately hit thinks at the full rate; one far from everyone with nothing about looks for enemies and
// reviews its goal half as often. Searches that cost (paths, looks ahead) are spread by client number, so no frame
// does every bot's at once.
namespace ThinkRate
{
	// 1 at the full rate, 2 at half
	int slowdown( GameEntity* ent, const Bots::State& bot );
	// a few milliseconds that differ from bot to bot, up to window
	inline int stagger( int clientNum, int window ) { return (clientNum * 37) % std::max( 1, window ); }
}

#endif
