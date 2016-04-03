#ifndef __G_WORLD_H__
#define __G_WORLD_H__

#include "g_entity.h"


struct EntityWorld : public GameEntity
{
					EntityWorld();
					~EntityWorld();

	void			init();

};


#endif // __G_WORLD_H__

