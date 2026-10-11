#include "cg_local.h"
#include "cg_mission.h"
#include "cg_missionedit.h"
#include <algorithm>
#include <cctype>

namespace
{
	constexpr float defaultRadius = 250.0f;

	// the camera is the cursor: a thing is placed where the editor's view is and faces the way it looks
	const float* cameraOrigin() { return cg.predictedPlayerState.origin; }
	const float* cameraAngles() { return cg.predictedPlayerState.viewangles; }

	// a number given as the list shows it, from 1; -1 when it names nothing
	int listIndex( const char* arg, int count, const char* what )
	{
		const int index = atoi( arg ) - 1;
		if( index < 0 || index >= count )
		{
			CGame::printf( "No %s %s (there are %d)\n", what, arg[0] ? arg : "?", count );
			return -1;
		}
		return index;
	}

	template<typename Entry>
	void removeAt( Entry* list, int& count, int index )
	{
		std::copy( list + index + 1, list + count, list + index );
		--count;
	}

	bool isNumber( const char* text )
	{
		if( !text[0] )
			return false;
		return std::all_of( text, text + strlen( text ), []( char c ) { return std::isdigit( static_cast<unsigned char>( c ) ) != 0; } );
	}

	// a vehicle named by table number, descriptive name or model name; -1 when nothing matches
	int vehicleByName( const char* name )
	{
		if( isNumber( name ) )
			return atoi( name ) < bg_numberOfVehicles ? atoi( name ) : -1;
		for( int i = 0; i < bg_numberOfVehicles; ++i )
		{
			if( availableVehicles[i].descriptiveName && !Q_stricmp( availableVehicles[i].descriptiveName, name ) )
				return i;
		}
		for( int i = 0; i < bg_numberOfVehicles; ++i )
		{
			if( availableVehicles[i].modelName && !Q_stricmp( availableVehicles[i].modelName, name ) )
				return i;
		}
		return -1;
	}

	void printAll()
	{
		static char text[MAX_MISSION_TEXT];
		const mission_overview_t& overview = MissionEdit::overview_;
		Common::printf( "Overview\n{\n\tmission\t\"%s\"\n\tgoal\t\"%s\"\n", overview.missionname, overview.objective );
		VehicleData::formatMissionOverviewKeys( overview, text, sizeof( text ) );
		Common::printf( "%s}\n", text );
		VehicleData::formatMissionStarts( MissionEdit::rules_, text, sizeof( text ) );
		Common::printf( "%s", text );
		VehicleData::formatMissionRules( MissionEdit::rules_, text, sizeof( text ) );
		Common::printf( "%s", text );
	}

	struct OverviewText
	{
		const char*	key;
		char*		text;
		int			size;
	};

	struct OverviewNumber
	{
		const char*	key;
		int*		number;
	};
}

void MissionEdit::clear()
{
	overview_ = {};
	rules_ = {};
	lastPlaced_ = nullptr;
}

void MissionEdit::addToScene()
{
	for( int i = 0; i < rules_.numCheckpoints; ++i )
	{
		const mission_checkpoint_t& checkpoint = rules_.checkpoints[i];
		// each faces back along the course to the one before it, as it will when flown
		vec3_t dir = { 1, 0, 0 }, angles;
		if( i > 0 )
			VectorSubtract( checkpoint.origin, rules_.checkpoints[i - 1].origin, dir );
		else if( rules_.numStarts > 0 )
			VectorSubtract( checkpoint.origin, rules_.starts[0].origin, dir );
		vectoangles( dir, angles );
		MissionHud::addCheckpointMarker( checkpoint, angles[YAW] );
	}
	for( int i = 0; i < rules_.numStarts; ++i )
		MissionHud::addStartMarker( rules_.starts[i].origin, rules_.starts[i].angles[YAW] );
}

// each marker's number, where the marker is on the screen
void MissionEdit::draw()
{
	constexpr vec4_t white = { 1, 1, 1, 1 };
	const auto label = [&]( const vec3_t origin, const char* text ) {
		vec3_t at;
		VectorCopy( origin, at );
		if( const auto screen = CGame::worldToScreenCoords( at, true ) )
			CGame::drawSmallStringColor( screen->x, screen->y, text, const_cast<float*>( white ) );
	};
	for( int i = 0; i < rules_.numCheckpoints; ++i )
		label( rules_.checkpoints[i].origin, va( "%d", i + 1 ) );
	for( int i = 0; i < rules_.numStarts; ++i )
		label( rules_.starts[i].origin, va( "start %d", i + 1 ) );
}

// me_start [vehicle] [red|blue]: a player start at the camera
void MissionEdit::start_f()
{
	if( rules_.numStarts >= MISSION_MAX_STARTS )
	{
		CGame::printf( "No room for another start (%d)\n", MISSION_MAX_STARTS );
		return;
	}
	mission_playerstart_t start = {};
	start.index = -1;
	start.speed = -1;
	const char* vehicle = CGame::argv( 1 );
	if( vehicle[0] )
	{
		start.index = vehicleByName( vehicle );
		if( start.index < 0 )
		{
			CGame::printf( "No vehicle '%s'\n", vehicle );
			return;
		}
		Q_strncpyz( start.objectname, availableVehicles[start.index].modelName, sizeof( start.objectname ) );
		Q_strncpyz( start.descriptivename, availableVehicles[start.index].descriptiveName, sizeof( start.descriptivename ) );
	}
	const char* side = CGame::argv( 2 );
	if( !Q_stricmp( side, "red" ) || !Q_stricmp( side, "blue" ) )
		Q_strncpyz( start.teamname, side, sizeof( start.teamname ) );
	VectorCopy( cameraOrigin(), start.origin );
	start.angles[YAW] = cameraAngles()[YAW];
	rules_.starts[rules_.numStarts++] = start;
	CGame::printf( "Start %d placed%s%s\n", rules_.numStarts, start.index >= 0 ? " for " : "", start.index >= 0 ? start.descriptivename : "" );
}

void MissionEdit::startMove_f()
{
	const int index = listIndex( CGame::argv( 1 ), rules_.numStarts, "start" );
	if( index < 0 )
		return;
	VectorCopy( cameraOrigin(), rules_.starts[index].origin );
	rules_.starts[index].angles[YAW] = cameraAngles()[YAW];
	CGame::printf( "Start %d moved\n", index + 1 );
}

void MissionEdit::startDelete_f()
{
	const int index = listIndex( CGame::argv( 1 ), rules_.numStarts, "start" );
	if( index < 0 )
		return;
	removeAt( rules_.starts, rules_.numStarts, index );
	CGame::printf( "Start %d deleted\n", index + 1 );
}

// me_checkpoint [radius] [air|land|sea] [before]: a checkpoint at the camera, last in the course or before the one numbered
void MissionEdit::checkpoint_f()
{
	if( rules_.numCheckpoints >= MISSION_MAX_CHECKPOINTS )
	{
		CGame::printf( "No room for another checkpoint (%d)\n", MISSION_MAX_CHECKPOINTS );
		return;
	}
	mission_checkpoint_t checkpoint = {};
	VectorCopy( cameraOrigin(), checkpoint.origin );
	checkpoint.radius = atof( CGame::argv( 1 ) ) > 0 ? atof( CGame::argv( 1 ) ) : defaultRadius;
	const char* kind = CGame::argv( 2 );
	for( const CheckpointKind each : { CheckpointKind::Air, CheckpointKind::Land, CheckpointKind::Sea } )
	{
		if( !Q_stricmp( kind, VehicleData::checkpointKindName( each ) ) )
			checkpoint.kind = each;
	}
	int at = rules_.numCheckpoints;
	if( CGame::argv( 3 )[0] )
	{
		at = listIndex( CGame::argv( 3 ), rules_.numCheckpoints, "checkpoint" );
		if( at < 0 )
			return;
	}
	std::copy_backward( rules_.checkpoints + at, rules_.checkpoints + rules_.numCheckpoints, rules_.checkpoints + rules_.numCheckpoints + 1 );
	rules_.checkpoints[at] = checkpoint;
	++rules_.numCheckpoints;
	CGame::printf( "Checkpoint %d of %d placed\n", at + 1, rules_.numCheckpoints );
}

void MissionEdit::checkpointMove_f()
{
	const int index = listIndex( CGame::argv( 1 ), rules_.numCheckpoints, "checkpoint" );
	if( index < 0 )
		return;
	VectorCopy( cameraOrigin(), rules_.checkpoints[index].origin );
	CGame::printf( "Checkpoint %d moved\n", index + 1 );
}

void MissionEdit::checkpointDelete_f()
{
	const int index = listIndex( CGame::argv( 1 ), rules_.numCheckpoints, "checkpoint" );
	if( index < 0 )
		return;
	removeAt( rules_.checkpoints, rules_.numCheckpoints, index );
	CGame::printf( "Checkpoint %d deleted\n", index + 1 );
}

// me_tag <tag>: every selected vehicle and installation carries the tag, or with nothing selected the one placed
// last; no tag takes it off
void MissionEdit::tag_f()
{
	int tagged = 0;
	for( IGME_vehicle_t& vehicle : cgs.IGME.vehicles )
	{
		if( vehicle.active && vehicle.selected )
		{
			Q_strncpyz( vehicle.tag, CGame::argv( 1 ), sizeof( vehicle.tag ) );
			++tagged;
		}
	}
	if( !tagged && lastPlaced_ && lastPlaced_->active )
	{
		Q_strncpyz( lastPlaced_->tag, CGame::argv( 1 ), sizeof( lastPlaced_->tag ) );
		++tagged;
	}
	if( tagged )
		CGame::printf( "%d tagged '%s'\n", tagged, CGame::argv( 1 ) );
	else
		CGame::printf( "Select or place what is to carry the tag first\n" );
}

// me_objective <type> <text> [tag] [number] [bonus]: the number is a count for destroy and seconds for survive
void MissionEdit::objective_f()
{
	if( rules_.numObjectives >= MISSION_MAX_OBJECTIVES )
	{
		CGame::printf( "No room for another objective (%d)\n", MISSION_MAX_OBJECTIVES );
		return;
	}
	mission_objective_t objective = {};
	bool typed = false;
	for( const ObjectiveType each : { ObjectiveType::Destroy, ObjectiveType::Checkpoints, ObjectiveType::Survive, ObjectiveType::Protect } )
	{
		if( !Q_stricmp( CGame::argv( 1 ), VehicleData::objectiveTypeName( each ) ) )
		{
			objective.type = each;
			typed = true;
		}
	}
	if( !typed || CommandSystem::argc() < 3 )
	{
		CGame::printf( "usage: me_objective <destroy|checkpoints|survive|protect> <text> [tag] [number] [bonus]\n" );
		return;
	}
	Q_strncpyz( objective.text, CGame::argv( 2 ), sizeof( objective.text ) );
	for( int i = 3; i < CommandSystem::argc(); ++i )
	{
		const char* arg = CGame::argv( i );
		if( !Q_stricmp( arg, "bonus" ) )
			objective.bonus = true;
		else if( isNumber( arg ) && objective.type == ObjectiveType::Survive )
			objective.seconds = atoi( arg );
		else if( isNumber( arg ) )
			objective.count = atoi( arg );
		else
			Q_strncpyz( objective.tag, arg, sizeof( objective.tag ) );
	}
	rules_.objectives[rules_.numObjectives++] = objective;
	CGame::printf( "Objective %d added\n", rules_.numObjectives );
}

void MissionEdit::objectiveDelete_f()
{
	const int index = listIndex( CGame::argv( 1 ), rules_.numObjectives, "objective" );
	if( index < 0 )
		return;
	removeAt( rules_.objectives, rules_.numObjectives, index );
	CGame::printf( "Objective %d deleted\n", index + 1 );
}

// me_overview <key> <value>
void MissionEdit::overview_f()
{
	const OverviewText texts[] = {
		{ "title", overview_.missionname, sizeof( overview_.missionname ) },
		{ "goal", overview_.objective, sizeof( overview_.objective ) },
		{ "description", overview_.description, sizeof( overview_.description ) },
		{ "briefing", overview_.briefing, sizeof( overview_.briefing ) },
		{ "success", overview_.success, sizeof( overview_.success ) },
		{ "failure", overview_.failure, sizeof( overview_.failure ) },
	};
	const OverviewNumber numbers[] = {
		{ "order", &overview_.order },
		{ "lives", &overview_.lives },
		{ "timelimit", &overview_.timelimit },
		{ "gametype", &overview_.gametype },
	};
	const char* key = CGame::argv( 1 );
	for( const OverviewText& entry : texts )
	{
		if( !Q_stricmp( key, entry.key ) )
		{
			Q_strncpyz( entry.text, CGame::argv( 2 ), entry.size );
			return;
		}
	}
	for( const OverviewNumber& entry : numbers )
	{
		if( !Q_stricmp( key, entry.key ) )
		{
			*entry.number = atoi( CGame::argv( 2 ) );
			return;
		}
	}
	CGame::printf( "usage: me_overview <title|goal|description|briefing|success|failure|order|lives|timelimit|gametype> <value>\n" );
}

void MissionEdit::show_f()
{
	printAll();
}
