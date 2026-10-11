#ifndef __UI_MISSIONS_H__
#define __UI_MISSIONS_H__

// the missions page: the list of mission files, the chosen one's briefing, and starting it
class MissionMenu
{
public:
	static void load();
	static int count();
	static const char* itemText( int index );
	static void select( int index );
	static void run();
};

#endif
