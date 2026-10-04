#ifndef __G_ITEM_H__
#define __G_ITEM_H__


#include "g_entity.h"


struct GameItem : public GameEntity
{
					GameItem() {}
	~GameItem() override {}

	struct Think_FinishSpawningItem : public GameEntity::EntityFunc_Think
	{
		void execute() override;
	};
};




#endif // __G_ITEM_H__
