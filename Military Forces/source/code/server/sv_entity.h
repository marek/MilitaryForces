#ifndef __SV_ENTITY_H__
#define __SV_ENTITY_H__

#include "../game/q_shared.h"




struct worldSector_t;

struct SV_Entity
{
	static const int k_MAX_ENT_CLUSTERS = 16;

					SV_Entity();
					~SV_Entity();

	worldSector_t*	worldSector_;
	SV_Entity*		nextEntityInWorldSector_;
	
	entityState_t	baseline_;					// for delta compression of initial sighting
	int				numClusters_;				// if -1, use headnode instead
	int				clusternums_[k_MAX_ENT_CLUSTERS];
	int				lastCluster_;				// if all the clusters don't fit in clusternums
	int				areanum_;
	int				areanum2_;
	int				snapshotCounter_;			// used to prevent double adding from portal views
};







#endif // __SV_ENTITY_H__

