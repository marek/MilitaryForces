// the parts of a mission file that make it something to play: player starts, checkpoints, objectives

#include "q_shared.h"
#include "bg_public.h"

namespace
{
	template<typename Block>
	struct MissionKey
	{
		const char*	name;
		void		(*set)( Block& block, const char* value );
	};

	template<typename Name>
	struct NamedValue
	{
		const char*	name;
		Name		value;
	};

	constexpr NamedValue<CheckpointKind> checkpointKinds[] = {
		{ "air", CheckpointKind::Air },
		{ "land", CheckpointKind::Land },
		{ "sea", CheckpointKind::Sea },
	};

	constexpr NamedValue<ObjectiveType> objectiveTypes[] = {
		{ "destroy", ObjectiveType::Destroy },
		{ "checkpoints", ObjectiveType::Checkpoints },
		{ "survive", ObjectiveType::Survive },
		{ "protect", ObjectiveType::Protect },
	};

	template<typename Name, size_t Count>
	bool valueByName( const NamedValue<Name> (&table)[Count], const char* name, Name& out )
	{
		for( const NamedValue<Name>& entry : table )
		{
			if( !Q_stricmp( entry.name, name ) )
			{
				out = entry.value;
				return true;
			}
		}
		return false;
	}

	template<typename Name, size_t Count>
	const char* nameOfValue( const NamedValue<Name> (&table)[Count], Name value )
	{
		for( const NamedValue<Name>& entry : table )
		{
			if( entry.value == value )
				return entry.name;
		}
		return "";
	}

	void readVector( const char* value, vec3_t out )
	{
		sscanf( value, "%f;%f;%f", &out[0], &out[1], &out[2] );
	}

	// reads "{ Key value ... }"; a key the table does not have is reported and its value skipped
	template<typename Block, size_t Count>
	bool readBlock( char** buf, const char* what, const MissionKey<Block> (&keys)[Count], Block& block )
	{
		char* token = COM_Parse( buf );
		if( strcmp( token, "{" ) )
		{
			Common::printf( S_COLOR_YELLOW "Missing { in mission %s\n", what );
			return false;
		}
		while( true )
		{
			token = COM_ParseExt( buf, true );
			if( !token[0] || !strcmp( token, "}" ) )
				return true;
			const MissionKey<Block>* found = nullptr;
			for( const MissionKey<Block>& key : keys )
			{
				if( !Q_stricmp( key.name, token ) )
					found = &key;
			}
			if( !found )
				Common::printf( S_COLOR_YELLOW "Unknown key '%s' in mission %s, skipped\n", token, what );
			token = COM_ParseExt( buf, false );
			// a key with no value, last on the line of a one-line block
			const bool closes = !strcmp( token, "}" );
			if( found )
				found->set( block, closes ? "" : token );
			if( closes )
				return true;
		}
	}

	constexpr MissionKey<mission_playerstart_t> startKeys[] = {
		{ "Descriptive", []( mission_playerstart_t& s, const char* v ) { Q_strncpyz( s.descriptivename, v, sizeof( s.descriptivename ) ); } },
		{ "Name", []( mission_playerstart_t& s, const char* v ) { Q_strncpyz( s.objectname, v, sizeof( s.objectname ) ); } },
		{ "Team", []( mission_playerstart_t& s, const char* v ) { Q_strncpyz( s.teamname, v, sizeof( s.teamname ) ); } },
		{ "Origin", []( mission_playerstart_t& s, const char* v ) { readVector( v, s.origin ); } },
		{ "Angles", []( mission_playerstart_t& s, const char* v ) { readVector( v, s.angles ); } },
		{ "Speed", []( mission_playerstart_t& s, const char* v ) { s.speed = atoi( v ); } },
		{ "Throttle", []( mission_playerstart_t& s, const char* v ) { s.throttle = atoi( v ); } },
	};

	constexpr MissionKey<mission_checkpoint_t> checkpointKeys[] = {
		{ "Origin", []( mission_checkpoint_t& c, const char* v ) { readVector( v, c.origin ); } },
		{ "Radius", []( mission_checkpoint_t& c, const char* v ) { c.radius = atof( v ); } },
		{ "Kind", []( mission_checkpoint_t& c, const char* v ) {
			if( !valueByName( checkpointKinds, v, c.kind ) )
				Common::printf( S_COLOR_YELLOW "Unknown checkpoint kind '%s' in mission, taken as air\n", v );
		} },
	};

	struct ObjectiveBlock
	{
		mission_objective_t	objective;
		bool				typed;
	};

	constexpr MissionKey<ObjectiveBlock> objectiveKeys[] = {
		{ "Type", []( ObjectiveBlock& o, const char* v ) {
			o.typed = valueByName( objectiveTypes, v, o.objective.type );
			if( !o.typed )
				Common::printf( S_COLOR_YELLOW "Unknown objective type '%s' in mission\n", v );
		} },
		{ "Tag", []( ObjectiveBlock& o, const char* v ) { Q_strncpyz( o.objective.tag, v, sizeof( o.objective.tag ) ); } },
		{ "Count", []( ObjectiveBlock& o, const char* v ) { o.objective.count = atoi( v ); } },
		{ "Seconds", []( ObjectiveBlock& o, const char* v ) { o.objective.seconds = atoi( v ); } },
		{ "Text", []( ObjectiveBlock& o, const char* v ) { Q_strncpyz( o.objective.text, v, sizeof( o.objective.text ) ); } },
		{ "Bonus", []( ObjectiveBlock& o, const char* v ) { o.objective.bonus = !v[0] || atoi( v ) != 0; } },
	};

	// a reader with nowhere to put the rules (the editor) still has to read past them
	mission_rules_t unwantedRules;

	// reads "{ <entry> { ... } <entry> { ... } }", handing each entry's block to read
	template<typename Read>
	void readList( char** buf, const char* list, const char* entry, Read read )
	{
		char* token = COM_Parse( buf );
		if( strcmp( token, "{" ) )
		{
			Common::printf( S_COLOR_YELLOW "Missing { in mission %s\n", list );
			return;
		}
		while( true )
		{
			token = COM_Parse( buf );
			if( !token[0] || !strcmp( token, "}" ) )
				return;
			if( !Q_stricmp( token, entry ) )
			{
				read();
				continue;
			}
			Common::printf( S_COLOR_YELLOW "Unknown entry '%s' in mission %s, skipped\n", token, list );
			SkipBracedSection( buf, 0 );
		}
	}
}

const char* VehicleData::checkpointKindName( CheckpointKind kind )
{
	return nameOfValue( checkpointKinds, kind );
}

const char* VehicleData::objectiveTypeName( ObjectiveType type )
{
	return nameOfValue( objectiveTypes, type );
}

void VehicleData::parsePlayerStart( char **buf, mission_rules_t* rules )
{
	if( !rules )
		rules = &unwantedRules;
	mission_playerstart_t start = {};
	start.index = -1;
	start.speed = -1;
	if( !readBlock( buf, "PlayerStart", startKeys, start ) )
		return;
	if( start.descriptivename[0] || start.objectname[0] )
	{
		mission_vehicle_t named = {};
		named.index = -1;
		Q_strncpyz( named.descriptivename, start.descriptivename, sizeof( named.descriptivename ) );
		Q_strncpyz( named.objectname, start.objectname, sizeof( named.objectname ) );
		VehicleData::resolveMissionVehicle( &named );
		start.index = named.index;
	}
	if( rules->numStarts >= MISSION_MAX_STARTS )
	{
		Common::printf( S_COLOR_YELLOW "Too many player starts in mission, ignoring the rest\n" );
		return;
	}
	rules->starts[rules->numStarts++] = start;
}

void VehicleData::parseCheckpoints( char **buf, mission_rules_t* rules )
{
	// wide enough for a jet at speed to line up on
	constexpr float defaultRadius = 200.0f;

	if( !rules )
		rules = &unwantedRules;
	readList( buf, "Checkpoints", "Checkpoint", [&]() {
		mission_checkpoint_t checkpoint = {};
		checkpoint.radius = defaultRadius;
		if( !readBlock( buf, "Checkpoint", checkpointKeys, checkpoint ) )
			return;
		if( rules->numCheckpoints >= MISSION_MAX_CHECKPOINTS )
		{
			Common::printf( S_COLOR_YELLOW "Too many checkpoints in mission, ignoring the rest\n" );
			return;
		}
		rules->checkpoints[rules->numCheckpoints++] = checkpoint;
	} );
}

void VehicleData::parseObjectives( char **buf, mission_rules_t* rules )
{
	if( !rules )
		rules = &unwantedRules;
	readList( buf, "Objectives", "Objective", [&]() {
		ObjectiveBlock block = {};
		if( !readBlock( buf, "Objective", objectiveKeys, block ) )
			return;
		if( !block.typed )
		{
			Common::printf( S_COLOR_YELLOW "Mission objective '%s' has no type, left out\n", block.objective.text );
			return;
		}
		if( rules->numObjectives >= MISSION_MAX_OBJECTIVES )
		{
			Common::printf( S_COLOR_YELLOW "Too many objectives in mission, ignoring the rest\n" );
			return;
		}
		rules->objectives[rules->numObjectives++] = block.objective;
	} );
}

// the writers below give back what the readers above take, so a mission passes through the editor whole

// the Overview keys the editor does not set itself, as lines for inside the block
void VehicleData::formatMissionOverviewKeys( const mission_overview_t& overview, char* out, int size )
{
	out[0] = 0;
	if( overview.order )
		Q_strcat( out, size, va( "\torder\t%d\n", overview.order ) );
	if( overview.lives )
		Q_strcat( out, size, va( "\tlives\t%d\n", overview.lives ) );
	if( overview.timelimit )
		Q_strcat( out, size, va( "\ttimelimit\t%d\n", overview.timelimit ) );
	const struct { const char* key; const char* text; } texts[] = {
		{ "description", overview.description },
		{ "briefing", overview.briefing },
		{ "success", overview.success },
		{ "failure", overview.failure },
	};
	for( const auto& entry : texts )
	{
		if( entry.text[0] )
			Q_strcat( out, size, va( "\t%s\t\"%s\"\n", entry.key, entry.text ) );
	}
}

// PlayerStart blocks, for inside Entities
void VehicleData::formatMissionStarts( const mission_rules_t& rules, char* out, int size )
{
	out[0] = 0;
	for( int i = 0; i < rules.numStarts; ++i )
	{
		const mission_playerstart_t& start = rules.starts[i];
		Q_strcat( out, size, "\tPlayerStart\n\t{\n" );
		if( start.objectname[0] )
			Q_strcat( out, size, va( "\t\tName\t\"%s\"\n", start.objectname ) );
		if( start.descriptivename[0] )
			Q_strcat( out, size, va( "\t\tDescriptive\t\"%s\"\n", start.descriptivename ) );
		if( start.teamname[0] )
			Q_strcat( out, size, va( "\t\tTeam\t%s\n", start.teamname ) );
		Q_strcat( out, size, va( "\t\tOrigin\t%f;%f;%f\n", start.origin[0], start.origin[1], start.origin[2] ) );
		Q_strcat( out, size, va( "\t\tAngles\t%f;%f;%f\n", start.angles[0], start.angles[1], start.angles[2] ) );
		if( start.speed >= 0 )
			Q_strcat( out, size, va( "\t\tSpeed\t%d\n\t\tThrottle\t%d\n", start.speed, start.throttle ) );
		Q_strcat( out, size, "\t}\n\n" );
	}
}

// the Checkpoints and Objectives blocks
void VehicleData::formatMissionRules( const mission_rules_t& rules, char* out, int size )
{
	out[0] = 0;
	if( rules.numCheckpoints )
	{
		Q_strcat( out, size, "\nCheckpoints\n{\n" );
		for( int i = 0; i < rules.numCheckpoints; ++i )
		{
			const mission_checkpoint_t& checkpoint = rules.checkpoints[i];
			Q_strcat( out, size, va( "\tCheckpoint { Origin %f;%f;%f Radius %.0f Kind %s }\n", checkpoint.origin[0], checkpoint.origin[1],
				checkpoint.origin[2], checkpoint.radius, VehicleData::checkpointKindName( checkpoint.kind ) ) );
		}
		Q_strcat( out, size, "}\n" );
	}
	if( rules.numObjectives )
	{
		Q_strcat( out, size, "\nObjectives\n{\n" );
		for( int i = 0; i < rules.numObjectives; ++i )
		{
			const mission_objective_t& objective = rules.objectives[i];
			Q_strcat( out, size, va( "\tObjective\n\t{\n\t\tType\t%s\n", VehicleData::objectiveTypeName( objective.type ) ) );
			if( objective.tag[0] )
				Q_strcat( out, size, va( "\t\tTag\t%s\n", objective.tag ) );
			if( objective.count )
				Q_strcat( out, size, va( "\t\tCount\t%d\n", objective.count ) );
			if( objective.seconds )
				Q_strcat( out, size, va( "\t\tSeconds\t%d\n", objective.seconds ) );
			if( objective.text[0] )
				Q_strcat( out, size, va( "\t\tText\t\"%s\"\n", objective.text ) );
			if( objective.bonus )
				Q_strcat( out, size, "\t\tBonus\t1\n" );
			Q_strcat( out, size, "\t}\n" );
		}
		Q_strcat( out, size, "}\n" );
	}
}
