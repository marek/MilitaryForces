#ifndef _G_MFQ3ENTS_H__
#define _G_MFQ3ENTS_H__



#include "g_entity.h"

struct Think_RadioComply : public GameEntity::EntityFunc_Think
{
	virtual void	execute();
};

struct Die_ExplosiveDie : public GameEntity::EntityFunc_Die
{
	virtual void	execute( GameEntity* inflictor, GameEntity* attacker, int damage, int mod );
};

struct Touch_RechargeTouch : public GameEntity::EntityFunc_Touch
{
	virtual void	execute( GameEntity* other, trace_t *trace );
};

struct Touch_RadioTower : public GameEntity::EntityFunc_Touch
{
	virtual void	execute( GameEntity* other, trace_t *trace );
};

struct Think_ExplodeVehicle : public GameEntity::EntityFunc_Think
{
	virtual void	execute();
};

#endif // _G_MFQ3ENTS_H__

