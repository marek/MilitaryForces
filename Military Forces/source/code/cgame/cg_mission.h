#ifndef __CG_MISSION_H__
#define __CG_MISSION_H__

// what a client shows of a mission: objectives, the next checkpoint, the briefing and the end, all as the server tells it
class MissionHud
{
public:
	static void registerMedia();
	static void configStringChanged( int num );
	static void addToScene();
	static void draw();
	static void whereCommand();
};

#endif
