#ifndef _G_MFQ3ENTS_H__
#define _G_MFQ3ENTS_H__



#include "g_entity.h"

struct Think_RadioComply : public GameEntity::EntityFunc_Think
{
	void	execute() override;
};

struct Die_ExplosiveDie : public GameEntity::EntityFunc_Die
{
	void	execute( GameEntity* inflictor, GameEntity* attacker, int damage, int mod ) override;
};

struct Touch_RechargeTouch : public GameEntity::EntityFunc_Touch
{
	void	execute( GameEntity* other, trace_t *trace ) override;
};

struct Touch_RadioTower : public GameEntity::EntityFunc_Touch
{
	void	execute( GameEntity* other, trace_t *trace ) override;
};

#endif // _G_MFQ3ENTS_H__

