#ifndef __CG_MISSIONEDIT_H__
#define __CG_MISSIONEDIT_H__

// the parts of a mission the editor changes by command at the camera's place: player starts, checkpoints, tags,
// objectives and the overview's texts
class MissionEdit
{
public:
	static inline mission_overview_t	overview_ = {};
	static inline mission_rules_t		rules_ = {};
	static inline IGME_vehicle_t*		lastPlaced_ = nullptr;	// what me_tag marks when nothing is selected

	static void clear();
	static void addToScene();
	static void draw();

	static void start_f();
	static void startMove_f();
	static void startDelete_f();
	static void checkpoint_f();
	static void checkpointMove_f();
	static void checkpointDelete_f();
	static void tag_f();
	static void objective_f();
	static void objectiveDelete_f();
	static void overview_f();
	static void show_f();
};

#endif
