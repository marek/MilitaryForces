#include "ui_local.h"
#include "ui_missions.h"
#include "../game/g_missionrules.h"

namespace
{
	std::vector<MissionListing> missions;
	int chosen = 0;

	// the page shows the chosen mission through these
	constexpr const char* titleCvar = "ui_missionTitle";
	constexpr const char* briefingCvar = "ui_missionBriefing";
	constexpr const char* mapCvar = "ui_missionMap";
}

void MissionMenu::load()
{
	missions = MissionRules::listAll();
	select( 0 );
}

int MissionMenu::count()
{
	return static_cast<int>( missions.size() );
}

const char* MissionMenu::itemText( int index )
{
	if( index < 0 || index >= count() )
		return "";
	const MissionListing& listed = missions[index];
	return va( "%s %s", listed.done ? "[x]" : "[ ]", listed.overview.missionname[0] ? listed.overview.missionname : listed.id );
}

void MissionMenu::select( int index )
{
	if( index < 0 || index >= count() )
	{
		CvarSystem::set( titleCvar, "No missions found" );
		CvarSystem::set( briefingCvar, "" );
		CvarSystem::set( mapCvar, "" );
		return;
	}
	chosen = index;
	const mission_overview_t& overview = missions[index].overview;
	CvarSystem::set( titleCvar, overview.missionname[0] ? overview.missionname : missions[index].id );
	CvarSystem::set( briefingCvar, overview.briefing[0] ? overview.briefing : overview.description );
	CvarSystem::set( mapCvar, overview.mapname );
}

void MissionMenu::run()
{
	if( chosen < 0 || chosen >= count() )
		return;
	CommandBuffer::executeText( EXEC_APPEND, va( "mission %s\n", missions[chosen].id ) );
}
