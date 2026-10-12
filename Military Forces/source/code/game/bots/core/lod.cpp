#include "../../g_local.h"
#include "../../g_level.h"
#include "local.h"
#include "lod.h"

int ThinkRate::slowdown( GameEntity* ent, const Bots::State& bot )
{
	constexpr float personNear = 6000.0f;
	constexpr int lateHit = 5000;
	if( bot.enemy >= 0 || theLevel.time_ < bot.hitTime + lateHit )
		return 1;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other != ent && alive( other ) && !Bots::isBot( i ) && Distance( other->r.currentOrigin, ent->r.currentOrigin ) < personNear )
			return 1;
	}
	return 2;
}
