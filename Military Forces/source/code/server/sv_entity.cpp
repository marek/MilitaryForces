#include "sv_entity.h"



SV_Entity::SV_Entity() :
	worldSector_(0),
	nextEntityInWorldSector_(0),
	numClusters_(0),
	lastCluster_(0),
	areanum_(0),
	areanum2_(0),
	snapshotCounter_(0)
{
	memset(&baseline_, 0, sizeof(baseline_));
	for( int i = 0; i < k_MAX_ENT_CLUSTERS; ++i )
		clusternums_[i] = 0;
}

SV_Entity::~SV_Entity()
{
}

