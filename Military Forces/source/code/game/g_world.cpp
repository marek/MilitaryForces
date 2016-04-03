#include "g_world.h"







EntityWorld::EntityWorld() :
	GameEntity()
{
}

EntityWorld::~EntityWorld()
{
}

void
EntityWorld::init()
{
	s.number = ENTITYNUM_WORLD;
	classname_ = "worldspawn";
}