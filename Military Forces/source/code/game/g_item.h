#ifndef __G_ITEM_H__
#define __G_ITEM_H__


#include "g_entity.h"


struct GameItem : public GameEntity
{
					GameItem() {}
	virtual			~GameItem() {}

	struct Think_FinishSpawningItem : public GameEntity::EntityFunc_Think
	{
		virtual void execute();
	};
};




#endif // __G_ITEM_H__
