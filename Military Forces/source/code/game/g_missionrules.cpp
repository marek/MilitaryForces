#include "g_local.h"
#include "g_level.h"
#include "g_entity.h"
#include "g_missionrules.h"
#include <algorithm>
#include <array>
#include <string_view>
#include <vector>

namespace
{
	constexpr int restartDelay = 2000;		// after the end, before fire restarts: the shot that ended it is not the restart
	constexpr int millisecondsPerSecond = 1000;
	constexpr int noClock = 0;

	struct Placed
	{
		int		entityNum;
		int		team;
		char	tag[MAX_NAME_LENGTH];
		bool	destroyed;
	};

	struct Objective
	{
		mission_objective_t	rule;
		ObjectiveState		state;
		int					need;		// destroy: how many
	};

	struct Mission
	{
		char				id[MAX_QPATH];		// <map>/<file name>, as mf_missionsDone keeps it
		mission_overview_t	overview;
		mission_rules_t		rules;
		std::vector<Placed>	placed;
		std::vector<Objective> objectives;
		MissionState		state = MissionState::None;
		int					startTeam = ClientBase::TEAM_FREE;
		int					nextStart = 0;
		int					nextCheckpoint = 0;
		int					livesLost = 0;
		int					startTime = 0;		// when the first player took a vehicle; 0: nobody yet
		int					endTime = 0;
		std::array<int, MAX_CLIENTS> startOf;	// which start a client was handed, -1: none yet
		std::array<bool, MAX_CLIENTS> tracked;	// lastOrigin holds where the client was last frame
		std::array<bool, MAX_CLIENTS> released;	// after the end: the client has had its triggers up
		std::array<Vec3, MAX_CLIENTS> lastOrigin;
	};

	Mission mission;

	// quotes end a config string on its way to a client; the separator would split an entry
	void copyClean( char* out, int size, const char* in )
	{
		Q_strncpyz( out, in, size );
		for( char* c = out; *c; ++c )
		{
			if( *c == '"' )
				*c = '\'';
			else if( *c == MISSION_LIST_SEPARATOR || *c == '\\' || *c == ';' )
				*c = '/';
		}
	}

	int teamByName( const char* name )
	{
		if( !Q_stricmp( name, "red" ) )
			return ClientBase::TEAM_RED;
		if( !Q_stricmp( name, "blue" ) )
			return ClientBase::TEAM_BLUE;
		return ClientBase::TEAM_FREE;
	}

	bool playing( const GameEntity* ent )
	{
		return ent && ent->inuse_ && ent->client_ && !(ent->r.svFlags & SVF_BOT) &&
			ent->client_->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED &&
			ent->client_->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR;
	}

	bool inVehicle( const GameEntity* ent )
	{
		return playing( ent ) && ent->client_->vehicle_ >= 0 && ent->health_ > 0 &&
			!(ent->client_->ps_.pm_flags & PMF_VEHICLESELECT);
	}

	bool matches( const Placed& placed, const mission_objective_t& rule )
	{
		if( rule.tag[0] )
			return !Q_stricmp( placed.tag, rule.tag );
		return mission.startTeam == ClientBase::TEAM_FREE || placed.team != mission.startTeam;
	}

	int countMatching( const mission_objective_t& rule, bool destroyedOnly )
	{
		return static_cast<int>( std::count_if( mission.placed.begin(), mission.placed.end(), [&]( const Placed& placed ) {
			return matches( placed, rule ) && (!destroyedOnly || placed.destroyed);
		} ) );
	}

	int secondsRunning()
	{
		return mission.startTime ? (theLevel.time_ - mission.startTime) / millisecondsPerSecond : 0;
	}

	// yaw of the line a player flies into the checkpoint along, so its marker can face them
	float approachYaw( int index )
	{
		const mission_checkpoint_t& to = mission.rules.checkpoints[index];
		const float* from = nullptr;
		if( index > 0 )
			from = mission.rules.checkpoints[index - 1].origin;
		else if( mission.rules.numStarts > 0 )
			from = mission.rules.starts[0].origin;
		if( !from )
			return 0;
		vec3_t dir, angles;
		VectorSubtract( to.origin, from, dir );
		vectoangles( dir, angles );
		return angles[YAW];
	}

	void publishState()
	{
		char info[MAX_INFO_STRING] = "";
		char title[MAX_NAME_LENGTH];
		copyClean( title, sizeof( title ), mission.overview.missionname );
		Info::setValueForKey( info, "n", title );
		Info::setValueForKey( info, "s", va( "%d", static_cast<int>( mission.state ) ) );
		Info::setValueForKey( info, "l", va( "%d", std::max( 0, mission.overview.lives - mission.livesLost ) ) );
		const int clockEnd = mission.overview.timelimit && mission.startTime ?
			mission.startTime + mission.overview.timelimit * millisecondsPerSecond : noClock;
		Info::setValueForKey( info, "t", va( "%d", mission.state == MissionState::Running ? clockEnd : noClock ) );
		Info::setValueForKey( info, "cn", va( "%d", mission.rules.numCheckpoints ) );
		Info::setValueForKey( info, "cp", va( "%d", mission.nextCheckpoint ) );
		if( mission.state == MissionState::Running && mission.nextCheckpoint < mission.rules.numCheckpoints )
		{
			const mission_checkpoint_t& next = mission.rules.checkpoints[mission.nextCheckpoint];
			Info::setValueForKey( info, "co", va( "%.0f %.0f %.0f", next.origin[0], next.origin[1], next.origin[2] ) );
			Info::setValueForKey( info, "cr", va( "%.0f", next.radius ) );
			Info::setValueForKey( info, "ck", va( "%d", static_cast<int>( next.kind ) ) );
			Info::setValueForKey( info, "cy", va( "%.0f", approachYaw( mission.nextCheckpoint ) ) );
		}
		GameHost::setConfigstring( CS_MISSION, info );
	}

	void publishText()
	{
		const char* text = mission.overview.briefing;
		if( mission.state == MissionState::Complete )
			text = mission.overview.success;
		else if( mission.state == MissionState::Failed )
			text = mission.overview.failure;
		char clean[MISSION_TEXT_LENGTH];
		copyClean( clean, sizeof( clean ), text );
		GameHost::setConfigstring( CS_MISSION_TEXT, clean );
	}

	// each entry: state digit, bonus digit, then the text with its count
	void publishObjectives()
	{
		char list[MAX_INFO_STRING] = "";
		for( const Objective& objective : mission.objectives )
		{
			char text[MISSION_LINE_LENGTH];
			copyClean( text, sizeof( text ), objective.rule.text[0] ? objective.rule.text : VehicleData::objectiveTypeName( objective.rule.type ) );
			char progress[32] = "";
			if( objective.rule.type == ObjectiveType::Destroy )
				Com_sprintf( progress, sizeof( progress ), " (%d/%d)", std::min( objective.need, countMatching( objective.rule, true ) ), objective.need );
			else if( objective.rule.type == ObjectiveType::Checkpoints )
				Com_sprintf( progress, sizeof( progress ), " (%d/%d)", mission.nextCheckpoint, mission.rules.numCheckpoints );
			else if( objective.rule.type == ObjectiveType::Survive && objective.state == ObjectiveState::Open )
				Com_sprintf( progress, sizeof( progress ), " (%d s)", std::max( 0, objective.rule.seconds - secondsRunning() ) );
			const char separator[2] = { MISSION_LIST_SEPARATOR, 0 };
			if( list[0] )
				Q_strcat( list, sizeof( list ), separator );
			Q_strcat( list, sizeof( list ), va( "%d%d%s%s", static_cast<int>( objective.state ), objective.rule.bonus ? 1 : 0, text, progress ) );
		}
		GameHost::setConfigstring( CS_MISSION_OBJECTIVES, list );
	}

	void publish()
	{
		publishState();
		publishText();
		publishObjectives();
	}

	// completed missions, one id to a line
	constexpr const char* doneFile = "missions/done.txt";

	bool isDone( const char* id )
	{
		const FileData file( doneFile );
		if( !file.data() || file.length() <= 0 )
			return false;
		const std::string_view text( file.as<char>(), file.length() );
		const std::string_view want( id );
		for( size_t at = 0; at < text.size(); )
		{
			const size_t eol = std::min( text.find( '\n', at ), text.size() );
			if( text.substr( at, eol - at ) == want )
				return true;
			at = eol + 1;
		}
		return false;
	}

	void remember()
	{
		if( isDone( mission.id ) )
			return;
		const OpenFile file( doneFile, FS_APPEND );
		if( !file )
			return;
		const char* line = va( "%s\n", mission.id );
		FileSystem::write( line, strlen( line ), file.handle() );
	}

	void end( MissionState state, const char* why )
	{
		if( mission.state != MissionState::Running )
			return;
		mission.state = state;
		mission.endTime = theLevel.time_;
		if( state == MissionState::Complete )
			remember();
		Game::logPrintf( "Mission: %s %s (%s)\n", mission.id, state == MissionState::Complete ? "complete" : "failed", why );
		publish();
	}

	void setObjective( Objective& objective, ObjectiveState state )
	{
		if( objective.state == state )
			return;
		objective.state = state;
		Game::logPrintf( "Objective: %s %s %s\n", VehicleData::objectiveTypeName( objective.rule.type ),
			objective.rule.tag[0] ? objective.rule.tag : "-", state == ObjectiveState::Done ? "done" : "failed" );
	}

	void judge()
	{
		bool required = false, open = false;
		for( Objective& objective : mission.objectives )
		{
			if( objective.state == ObjectiveState::Open )
			{
				switch( objective.rule.type )
				{
				case ObjectiveType::Destroy:
					if( countMatching( objective.rule, true ) >= objective.need )
						setObjective( objective, ObjectiveState::Done );
					break;
				case ObjectiveType::Checkpoints:
					if( mission.nextCheckpoint >= mission.rules.numCheckpoints )
						setObjective( objective, ObjectiveState::Done );
					break;
				case ObjectiveType::Survive:
					if( mission.startTime && secondsRunning() >= objective.rule.seconds )
						setObjective( objective, ObjectiveState::Done );
					break;
				case ObjectiveType::Protect:
					if( countMatching( objective.rule, true ) > 0 )
						setObjective( objective, ObjectiveState::Failed );
					break;
				}
			}
			if( objective.rule.bonus )
				continue;
			if( objective.state == ObjectiveState::Failed )
			{
				end( MissionState::Failed, "objective failed" );
				return;
			}
			// something kept safe is done only when the rest is
			if( objective.rule.type == ObjectiveType::Protect )
				continue;
			required = true;
			open = open || objective.state == ObjectiveState::Open;
		}
		if( required && !open )
		{
			for( Objective& objective : mission.objectives )
			{
				if( objective.rule.type == ObjectiveType::Protect && objective.state == ObjectiveState::Open )
					setObjective( objective, ObjectiveState::Done );
			}
			end( MissionState::Complete, "objectives done" );
		}
	}

	float distanceToSegment( const vec3_t point, const vec3_t from, const vec3_t to, bool level )
	{
		vec3_t along, toPoint;
		VectorSubtract( to, from, along );
		VectorSubtract( point, from, toPoint );
		if( level )
			along[2] = toPoint[2] = 0;
		const float length = DotProduct( along, along );
		const float t = length > 0 ? std::clamp( DotProduct( toPoint, along ) / length, 0.0f, 1.0f ) : 0.0f;
		vec3_t nearest;
		VectorMA( toPoint, -t, along, nearest );
		return VectorLength( nearest );
	}

	void passCheckpoints()
	{
		for( int i = 1; i <= theLevel.maxclients_; ++i )
		{
			const GameEntity* ent = theLevel.getEntity( i );
			if( !inVehicle( ent ) )
			{
				mission.tracked[i] = false;
				continue;
			}
			const Vec3 now( ent->r.currentOrigin );
			const Vec3 before = mission.tracked[i] ? mission.lastOrigin[i] : now;
			mission.lastOrigin[i] = now;
			mission.tracked[i] = true;
			if( mission.nextCheckpoint >= mission.rules.numCheckpoints )
				continue;
			const mission_checkpoint_t& next = mission.rules.checkpoints[mission.nextCheckpoint];
			// on land and water the marker is a post or a buoy: height over it does not count
			if( distanceToSegment( next.origin, before.v, now.v, next.kind != CheckpointKind::Air ) > next.radius )
				continue;
			++mission.nextCheckpoint;
			Game::logPrintf( "Checkpoint: %i passed %i of %i\n", i, mission.nextCheckpoint, mission.rules.numCheckpoints );
			GameHost::sendServerCommand( -1, va( "cp \"Checkpoint %d of %d\n\"", mission.nextCheckpoint, mission.rules.numCheckpoints ) );
			judge();
			publish();
		}
	}

	// the trigger has to be let go and pulled again: one held when the mission ended is not a wish to restart
	void restartOnFire()
	{
		if( theLevel.time_ < mission.endTime + restartDelay )
			return;
		for( int i = 1; i <= theLevel.maxclients_; ++i )
		{
			const GameEntity* ent = theLevel.getEntity( i );
			if( !playing( ent ) )
				continue;
			const bool firing = (ent->client_->pers_.cmd_.buttons & (BUTTON_ATTACK | BUTTON_ATTACK_MAIN)) != 0;
			if( !firing )
				mission.released[i] = true;
			else if( mission.released[i] )
			{
				mission.released.fill( false );
				mission.endTime = theLevel.time_;
				CommandBuffer::executeText( EXEC_APPEND, "map_restart 0\n" );
				return;
			}
		}
	}

	// reads only the Overview of a mission file
	bool readOverview( const char* path, mission_overview_t& overview )
	{
		const FileData file( path );
		if( !file.data() || file.length() <= 0 || file.length() >= MAX_MISSION_TEXT )
			return false;
		static char text[MAX_MISSION_TEXT];
		memcpy( text, file.data(), file.length() );
		text[file.length()] = 0;
		VehicleData::parseMissionOverview( text, &overview );
		return true;
	}

	struct Listed
	{
		char				id[MAX_QPATH];
		mission_overview_t	overview;
	};

	std::vector<Listed> findMissions()
	{
		std::vector<Listed> found;
		int numMaps = 0;
		char** maps = FileSystem::listFiles( "missions", "/", &numMaps );
		for( int m = 0; m < numMaps; ++m )
		{
			if( maps[m][0] == '.' )
				continue;
			int numFiles = 0;
			char** files = FileSystem::listFiles( va( "missions/%s", maps[m] ), ".mis", &numFiles );
			for( int f = 0; f < numFiles; ++f )
			{
				Listed listed = {};
				char name[MAX_QPATH];
				COM_StripExtension( files[f], name, sizeof( name ) );
				Com_sprintf( listed.id, sizeof( listed.id ), "%s/%s", maps[m], name );
				if( readOverview( va( "missions/%s/%s", maps[m], files[f] ), listed.overview ) )
					found.push_back( listed );
			}
			FileSystem::freeFileList( files );
		}
		FileSystem::freeFileList( maps );
		std::sort( found.begin(), found.end(), []( const Listed& a, const Listed& b ) {
			if( a.overview.order != b.overview.order )
				return a.overview.order < b.overview.order;
			return Q_stricmp( a.id, b.id ) < 0;
		} );
		return found;
	}
}

void MissionRules::clear()
{
	mission = Mission();
	mission.startOf.fill( -1 );
	mission.tracked.fill( false );
	mission.released.fill( false );
	GameHost::setConfigstring( CS_MISSION, "" );
	GameHost::setConfigstring( CS_MISSION_TEXT, "" );
	GameHost::setConfigstring( CS_MISSION_OBJECTIVES, "" );
}

void MissionRules::notePlaced( const GameEntity* ent, const char* tag )
{
	if( !ent )
		return;
	Placed placed = {};
	placed.entityNum = ent->s.number;
	placed.team = ent->s.generic1;
	Q_strncpyz( placed.tag, tag, sizeof( placed.tag ) );
	mission.placed.push_back( placed );
}

// called once the mission's vehicles and installations stand, so an objective can be checked against them
void MissionRules::begin( const char* id, const mission_overview_t& overview, const mission_rules_t& rules )
{
	Q_strncpyz( mission.id, id, sizeof( mission.id ) );
	mission.overview = overview;
	mission.rules = rules;
	if( rules.numStarts > 0 )
		mission.startTeam = teamByName( rules.starts[0].teamname );

	for( int i = 0; i < rules.numObjectives; ++i )
	{
		Objective objective = { rules.objectives[i], ObjectiveState::Open, 0 };
		const mission_objective_t& rule = objective.rule;
		const int carrying = countMatching( rule, false );
		const char* never = nullptr;
		if( rule.type == ObjectiveType::Destroy || rule.type == ObjectiveType::Protect )
		{
			if( !carrying )
				never = "nothing placed carries its tag";
			objective.need = rule.count > 0 ? std::min( rule.count, carrying ) : carrying;
		}
		else if( rule.type == ObjectiveType::Checkpoints && !rules.numCheckpoints )
			never = "the mission has no checkpoints";
		else if( rule.type == ObjectiveType::Survive && rule.seconds <= 0 )
			never = "it has no seconds";
		if( never )
		{
			Common::printf( S_COLOR_YELLOW "Mission objective '%s' left out: %s\n", rule.text, never );
			continue;
		}
		mission.objectives.push_back( objective );
	}

	if( rules.numStarts || rules.numCheckpoints || !mission.objectives.empty() )
	{
		mission.state = MissionState::Running;
		Game::logPrintf( "Mission: %s begins, %i starts %i checkpoints %i objectives\n", mission.id,
			rules.numStarts, rules.numCheckpoints, static_cast<int>( mission.objectives.size() ) );
		publish();
	}
}

const mission_playerstart_t* MissionRules::startFor( int clientNum )
{
	if( mission.state == MissionState::None || !mission.rules.numStarts )
		return nullptr;
	const GameEntity* ent = theLevel.getEntity( clientNum );
	if( !ent || !ent->client_ || ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR )
		return nullptr;
	if( mission.startOf[clientNum] < 0 )
		mission.startOf[clientNum] = mission.nextStart++ % mission.rules.numStarts;
	return &mission.rules.starts[mission.startOf[clientNum]];
}

// the start's vehicle becomes the client's choice, as if picked in the menu
void MissionRules::chooseStartVehicle( int clientNum )
{
	const mission_playerstart_t* start = startFor( clientNum );
	if( !start || start->index < 0 || start->index >= bg_numberOfVehicles || !VehicleData::selectable( start->index ) )
		return;
	char userinfo[MAX_INFO_STRING];
	GameHost::getUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	if( atoi( Info::valueForKey( userinfo, "cg_nextVehicle" ) ) == start->index && Info::valueForKey( userinfo, "cg_nextVehicle" )[0] )
		return;
	Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", start->index ) );
	GameHost::setUserinfo( clientNum, userinfo );
	theLevel.getClient( clientNum )->nextVehicle_ = start->index;
}

void MissionRules::applyStart( GameEntity* ent, const mission_playerstart_t& start )
{
	playerState_t& ps = ent->client_->ps_;
	if( start.speed >= 0 && !(ps.ONOFF & OO_LANDED) )
	{
		ps.speed = start.speed;
		ps.fixed_throttle = start.throttle;
	}
	if( !mission.startTime && !(ent->r.svFlags & SVF_BOT) && ent->client_->vehicle_ >= 0 )
	{
		mission.startTime = theLevel.time_;
		publish();
	}
}

void MissionRules::placedDestroyed( const GameEntity* ent )
{
	if( mission.state != MissionState::Running )
		return;
	for( Placed& placed : mission.placed )
	{
		if( placed.entityNum == ent->s.number && !placed.destroyed )
		{
			placed.destroyed = true;
			judge();
			publish();
			return;
		}
	}
}

void MissionRules::playerDestroyed( const GameEntity* ent )
{
	if( mission.state != MissionState::Running || !mission.startTime || !playing( ent ) )
		return;
	++mission.livesLost;
	if( mission.livesLost > mission.overview.lives )
		end( MissionState::Failed, "no lives left" );
	else
		publish();
}

void MissionRules::runFrame()
{
	if( mission.state == MissionState::Complete || mission.state == MissionState::Failed )
	{
		restartOnFire();
		return;
	}
	if( mission.state != MissionState::Running )
		return;

	// a mission with no start of its own begins when the first player takes a vehicle
	if( !mission.startTime )
	{
		for( int i = 1; i <= theLevel.maxclients_; ++i )
		{
			if( inVehicle( theLevel.getEntity( i ) ) )
			{
				mission.startTime = theLevel.time_;
				publish();
				break;
			}
		}
		if( !mission.startTime )
			return;
	}

	passCheckpoints();
	if( mission.state != MissionState::Running )
		return;

	// once a second: the clock on a survive objective is the only thing that moves without an event
	if( theLevel.time_ / millisecondsPerSecond != theLevel.previousTime_ / millisecondsPerSecond )
	{
		judge();
		if( mission.state == MissionState::Running && mission.overview.timelimit && secondsRunning() >= mission.overview.timelimit )
			end( MissionState::Failed, "out of time" );
		else if( mission.state == MissionState::Running )
			publishObjectives();
	}
}

void MissionRules::listCommand()
{
	const std::vector<Listed> found = findMissions();
	for( const Listed& listed : found )
	{
		Common::printf( "%s %-32s %s%s%s\n", isDone( listed.id ) ? "done" : "    ", listed.id,
			listed.overview.missionname, listed.overview.description[0] ? ": " : "", listed.overview.description );
	}
	Common::printf( "%i missions. Start one with: mission <map>/<name>\n", static_cast<int>( found.size() ) );
}

void MissionRules::startCommand()
{
	char id[MAX_QPATH];
	CommandSystem::argvBuffer( 1, id, sizeof( id ) );
	char* slash = strchr( id, '/' );
	if( !slash || !slash[1] || slash == id )
	{
		Common::printf( "usage: mission <map>/<name>   (missions lists them)\n" );
		return;
	}
	mission_overview_t overview = {};
	if( !readOverview( va( "missions/%s.mis", id ), overview ) )
	{
		Common::printf( "No mission file missions/%s.mis\n", id );
		return;
	}
	*slash = 0;
	// run at once, as map is: whatever follows on the command line finds the game started
	const char* map = id;
	const char* name = slash + 1;
	for( int set = 0; set < MF_MAX_GAMESETS; ++set )
	{
		if( overview.gameset == (MF_GAMESET_MIN << set) )
			CommandBuffer::executeText( EXEC_NOW, va( "set mf_gameset %s\n", gameset_codes[set] ) );
	}
	if( overview.gametype >= GT_FFA && overview.gametype < GT_MAX_GAME_TYPE && overview.gametype != GT_MISSION_EDITOR )
		CommandBuffer::executeText( EXEC_NOW, va( "set g_gametype %d\n", overview.gametype ) );
	CommandBuffer::executeText( EXEC_NOW, va( "set mf_mission %s\n", name ) );
	CommandBuffer::executeText( EXEC_NOW, va( "map %s\n", map ) );
}
