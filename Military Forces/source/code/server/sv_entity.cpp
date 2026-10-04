#include "sv_entity.h"



SV_Entity::SV_Entity() :
	worldSector_(nullptr),
	nextEntityInWorldSector_(nullptr),
	numClusters_(0),
	lastCluster_(0),
	areanum_(0),
	areanum2_(0),
	snapshotCounter_(0)
{
	memset(&baseline_, 0, sizeof(baseline_));
	for(int & clusternum : clusternums_)
		clusternum = 0;
}

SV_Entity::~SV_Entity()
= default;

