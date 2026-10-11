#include "cg_local.h"
#include "cg_mission.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>

namespace
{
	constexpr int briefingTime = 15000;			// msec the briefing stays up once the mission is known
	constexpr int listX = 70, listY = 56;		// objectives: upper left beside the speed tape, above the rows the centred texts use
	constexpr int lineHeight = SMALLCHAR_HEIGHT + 2;
	constexpr int textColumns = 64;				// a briefing or end text is wrapped to this
	constexpr int bannerY = 210, textY = 244;	// under the centre print, which announces each checkpoint
	constexpr int screenCentre = 320;
	constexpr int markerY = 96;					// bearing and distance to the next checkpoint
	constexpr float unitsPerMetre = 8.0f;		// as the HUD's range read-outs take it

	constexpr int ringSegments = 32;
	constexpr float ringWidth = 0.08f;			// of the radius
	constexpr float postHeight = 3.0f;			// of the radius
	constexpr float postWidth = 0.05f;
	constexpr float pulseRate = 0.004f;

	struct Next
	{
		bool			shown = false;
		vec3_t			origin = {};
		float			radius = 0;
		CheckpointKind	kind = CheckpointKind::Air;
		float			yaw = 0;
	};

	struct Shown
	{
		MissionState	state = MissionState::None;
		char			title[MAX_NAME_LENGTH] = {};
		int				lives = 0;
		int				clockEnd = 0;
		int				checkpoints = 0, passed = 0;
		Next			next;
		int				knownTime = 0;			// when this client first saw the mission running
	};

	Shown shown;
	qhandle_t markerShader;

	constexpr std::array<std::array<byte, 3>, 3> kindColour = { {
		{ 255, 220, 40 },		// air
		{ 255, 140, 30 },		// land
		{ 60, 220, 255 },		// sea
	} };

	void readState()
	{
		const char* info = CGame::configString( CS_MISSION );
		const MissionState before = shown.state;
		const int passedBefore = shown.passed;
		const int known = shown.knownTime;
		shown = Shown();
		shown.knownTime = known;
		if( !info[0] )
			return;
		shown.state = static_cast<MissionState>( atoi( Info::valueForKey( info, "s" ) ) );
		Q_strncpyz( shown.title, Info::valueForKey( info, "n" ), sizeof( shown.title ) );
		shown.lives = atoi( Info::valueForKey( info, "l" ) );
		shown.clockEnd = atoi( Info::valueForKey( info, "t" ) );
		shown.checkpoints = atoi( Info::valueForKey( info, "cn" ) );
		shown.passed = atoi( Info::valueForKey( info, "cp" ) );
		const char* origin = Info::valueForKey( info, "co" );
		if( origin[0] )
		{
			shown.next.shown = true;
			sscanf( origin, "%f %f %f", &shown.next.origin[0], &shown.next.origin[1], &shown.next.origin[2] );
			shown.next.radius = atof( Info::valueForKey( info, "cr" ) );
			shown.next.kind = static_cast<CheckpointKind>( std::clamp( atoi( Info::valueForKey( info, "ck" ) ), 0, static_cast<int>( kindColour.size() ) - 1 ) );
			shown.next.yaw = atof( Info::valueForKey( info, "cy" ) );
		}
		if( shown.state == MissionState::Running && before != MissionState::Running )
			shown.knownTime = cg.time;
		if( shown.passed > passedBefore && before == MissionState::Running )
			SoundSystem::startLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	// a quad of the ring or the post; both faces show
	void addQuad( const vec3_t a, const vec3_t b, const vec3_t c, const vec3_t d, const std::array<byte, 3>& colour, byte alpha )
	{
		polyVert_t verts[4] = {};
		const float* corners[4] = { a, b, c, d };
		for( int i = 0; i < 4; ++i )
		{
			VectorCopy( corners[i], verts[i].xyz );
			verts[i].modulate[0] = colour[0];
			verts[i].modulate[1] = colour[1];
			verts[i].modulate[2] = colour[2];
			verts[i].modulate[3] = alpha;
		}
		refExport.AddPolyToScene( markerShader, 4, verts, 1 );
	}

	// a ring of the checkpoint's radius in the plane of the two axes
	void addRing( const vec3_t centre, const vec3_t axisA, const vec3_t axisB, float radius, const std::array<byte, 3>& colour, byte alpha )
	{
		const float inner = radius * (1.0f - ringWidth);
		vec3_t outerBefore, innerBefore;
		for( int i = 0; i <= ringSegments; ++i )
		{
			const float angle = 2.0f * std::numbers::pi_v<float> * i / ringSegments;
			const float c = std::cos( angle ), s = std::sin( angle );
			vec3_t outerNow, innerNow;
			VectorMA( centre, radius * c, axisA, outerNow );
			VectorMA( outerNow, radius * s, axisB, outerNow );
			VectorMA( centre, inner * c, axisA, innerNow );
			VectorMA( innerNow, inner * s, axisB, innerNow );
			if( i > 0 )
				addQuad( outerBefore, outerNow, innerNow, innerBefore, colour, alpha );
			VectorCopy( outerNow, outerBefore );
			VectorCopy( innerNow, innerBefore );
		}
	}

	// two crossed upright quads, seen from any side
	void addPost( const vec3_t base, float radius, const std::array<byte, 3>& colour, byte alpha )
	{
		const float half = radius * postWidth, height = radius * postHeight;
		constexpr vec3_t sides[2] = { { 1, 0, 0 }, { 0, 1, 0 } };
		for( const vec3_t& side : sides )
		{
			vec3_t a, b, c, d;
			VectorMA( base, -half, side, a );
			VectorMA( base, half, side, b );
			VectorCopy( b, c );
			VectorCopy( a, d );
			c[2] += height;
			d[2] += height;
			addQuad( a, b, c, d, colour, alpha );
		}
	}

	void drawCentred( int y, const char* text, const vec4_t colour, bool big )
	{
		const int width = CGame::drawStrlen( text ) * (big ? BIGCHAR_WIDTH : SMALLCHAR_WIDTH);
		if( big )
			CGame::drawBigStringColor( screenCentre - width / 2, y, text, const_cast<float*>( colour ) );
		else
			CGame::drawSmallStringColor( screenCentre - width / 2, y, text, const_cast<float*>( colour ) );
	}

	// draws text wrapped at spaces, centred; returns the y under it
	int drawWrapped( int y, const char* text, const vec4_t colour )
	{
		char line[textColumns + 1];
		while( *text )
		{
			int length = static_cast<int>( strlen( text ) );
			if( length > textColumns )
			{
				length = textColumns;
				while( length > 0 && text[length] != ' ' )
					--length;
				if( !length )
					length = textColumns;
			}
			Q_strncpyz( line, text, length + 1 );
			drawCentred( y, line, colour, false );
			y += lineHeight;
			text += length;
			while( *text == ' ' )
				++text;
		}
		return y;
	}

	void drawObjectives()
	{
		constexpr vec4_t open = { 1, 1, 1, 1 }, done = { 0.4f, 1, 0.4f, 1 }, failed = { 1, 0.3f, 0.3f, 1 };
		constexpr const char* mark[] = { "[ ]", "[x]", "[!]" };
		char list[MAX_INFO_STRING];
		Q_strncpyz( list, CGame::configString( CS_MISSION_OBJECTIVES ), sizeof( list ) );
		int y = listY;
		for( char* entry = list; entry && *entry; )
		{
			char* end = strchr( entry, MISSION_LIST_SEPARATOR );
			if( end )
				*end = 0;
			if( strlen( entry ) > 2 )
			{
				const int state = std::clamp( entry[0] - '0', 0, 2 );
				const bool bonus = entry[1] == '1';
				const float* colour = state == 1 ? done : state == 2 ? failed : open;
				CGame::drawSmallStringColor( listX, y, va( "%s %s%s", mark[state], entry + 2, bonus ? " (bonus)" : "" ), const_cast<float*>( colour ) );
				y += lineHeight;
			}
			entry = end ? end + 1 : nullptr;
		}
		if( shown.clockEnd > cg.time )
		{
			const int left = (shown.clockEnd - cg.time) / 1000;
			CGame::drawSmallStringColor( listX, y, va( "Time %d:%02d", left / 60, left % 60 ), const_cast<float*>( open ) );
		}
	}

	void drawNextCheckpoint()
	{
		if( !shown.next.shown || !cg.snap )
			return;
		constexpr vec4_t colour = { 1, 0.9f, 0.3f, 1 };
		vec3_t to, angles;
		VectorSubtract( shown.next.origin, cg.predictedPlayerState.origin, to );
		const float distance = VectorLength( to );
		vectoangles( to, angles );
		const float turn = AngleNormalize180( angles[YAW] - cg.predictedPlayerState.vehicleAngles[YAW] );
		constexpr float ahead = 10.0f;		// degrees either side that count as straight on
		drawCentred( markerY, va( "%sCheckpoint %d/%d  %.0f m%s", turn > ahead ? "<< " : "", shown.passed + 1, shown.checkpoints,
			distance / unitsPerMetre, turn < -ahead ? " >>" : "" ), colour, false );
	}

	const char* teamName( int team )
	{
		return team == ClientBase::TEAM_RED ? "red" : team == ClientBase::TEAM_BLUE ? "blue" : "";
	}

	void printVector( const char* key, const float* v )
	{
		Common::printf( "\t%s\t%.1f;%.1f;%.1f\n", key, v[0], v[1], v[2] );
	}

	void printVehicleLines( int vehicle )
	{
		Common::printf( "\tIndex\t%d\n\tName\t\"%s\"\n\tDescriptive\t\"%s\"\n", vehicle,
			availableVehicles[vehicle].modelName, availableVehicles[vehicle].descriptiveName );
	}

	CheckpointKind kindFor( int vehicle )
	{
		if( vehicle >= 0 && (availableVehicles[vehicle].cat & CAT_BOAT) )
			return CheckpointKind::Sea;
		if( vehicle >= 0 && (availableVehicles[vehicle].cat & (CAT_GROUND | CAT_LQM)) )
			return CheckpointKind::Land;
		return CheckpointKind::Air;
	}

	// what the caller has locked, as a block a mission file takes
	void printTarget( const playerState_t& ps )
	{
		const int number = ps.tracktarget;
		if( number <= ENTITYNUM_WORLD || number >= MAX_GENTITIES || !cg_entities[number].currentValid )
		{
			Common::printf( "where target: nothing locked\n" );
			return;
		}
		const ClientEntity& target = cg_entities[number];
		const entityState_t& state = target.currentState;
		if( state.eType == ET_MISC_VEHICLE && state.modelindex == 255 )
		{
			Common::printf( "GroundInstallation\n{\n\tIndex\t%d\n\tName\t\"%s\"\n", state.modelindex2,
				availableGroundInstallations[state.modelindex2].descriptiveName );
			if( teamName( state.generic1 )[0] )
				Common::printf( "\tTeam\t%s\n", teamName( state.generic1 ) );
		}
		else
		{
			const int vehicle = state.eType == ET_MISC_VEHICLE ? state.modelindex : cgs.clientinfo[state.clientNum].vehicle;
			if( vehicle < 0 || vehicle >= bg_numberOfVehicles )
			{
				Common::printf( "where target: not a vehicle\n" );
				return;
			}
			Common::printf( "Vehicle\n{\n" );
			printVehicleLines( vehicle );
			Common::printf( "\tTeam\t%d\n", availableVehicles[vehicle].team );
		}
		printVector( "Origin", target.lerpOrigin.v );
		printVector( "Angles", target.lerpAngles.v );
		Common::printf( "}\n" );
	}
}

void MissionHud::registerMedia()
{
	markerShader = refExport.RegisterShader( "gfx/mission/checkpoint" );
	shown = Shown();
	readState();
}

void MissionHud::configStringChanged( int num )
{
	if( num == CS_MISSION )
		readState();
}

void MissionHud::addToScene()
{
	if( !shown.next.shown )
		return;
	const std::array<byte, 3>& colour = kindColour[static_cast<int>( shown.next.kind )];
	const byte alpha = static_cast<byte>( 150 + 80 * std::sin( cg.time * pulseRate ) );
	constexpr vec3_t up = { 0, 0, 1 };
	if( shown.next.kind == CheckpointKind::Air )
	{
		// upright, across the line a player comes in along
		vec3_t forward, right;
		const vec3_t angles = { 0, shown.next.yaw, 0 };
		AngleVectors( angles, forward, right, nullptr );
		addRing( shown.next.origin, right, up, shown.next.radius, colour, alpha );
	}
	else
	{
		constexpr vec3_t east = { 1, 0, 0 }, north = { 0, 1, 0 };
		addRing( shown.next.origin, east, north, shown.next.radius, colour, alpha );
		addPost( shown.next.origin, shown.next.radius, colour, alpha );
	}
}

void MissionHud::draw()
{
	if( shown.state == MissionState::None )
		return;
	constexpr vec4_t white = { 1, 1, 1, 1 }, good = { 0.4f, 1, 0.4f, 1 }, bad = { 1, 0.3f, 0.3f, 1 };
	const char* text = CGame::configString( CS_MISSION_TEXT );

	drawObjectives();

	if( shown.state == MissionState::Running )
	{
		drawNextCheckpoint();
		if( text[0] && cg.time < shown.knownTime + briefingTime )
		{
			drawCentred( bannerY, shown.title, white, true );
			drawWrapped( textY, text, white );
		}
		return;
	}

	const bool complete = shown.state == MissionState::Complete;
	drawCentred( bannerY, complete ? "MISSION COMPLETE" : "MISSION FAILED", complete ? good : bad, true );
	const int y = drawWrapped( textY, text, white );
	drawCentred( y + lineHeight, "Fire to fly it again", white, false );
}

// prints where the caller is, in the form a mission file takes
void MissionHud::whereCommand()
{
	// a checkpoint as wide as this many of the caller's own vehicle, so it can be flown through without a ruler
	constexpr float radiusInVehicles = 4.0f;
	constexpr float smallestRadius = 100.0f;

	if( !cg.snap )
		return;
	const playerState_t& ps = cg.snap->ps;
	const int vehicle = cgs.clientinfo[ps.clientNum].vehicle;
	const bool inVehicle = vehicle >= 0 && vehicle < bg_numberOfVehicles;
	const float* angles = inVehicle ? ps.vehicleAngles : ps.viewangles;
	const char* form = CGame::argv( 1 );
	const char* team = teamName( ps.persistant[PERS_TEAM] );

	if( !Q_stricmp( form, "target" ) )
	{
		printTarget( ps );
		return;
	}
	if( !Q_stricmp( form, "waypoint" ) )
	{
		printVector( "Origin", ps.origin );
		return;
	}
	if( !Q_stricmp( form, "checkpoint" ) )
	{
		float radius = atof( CGame::argv( 2 ) );
		if( radius <= 0 )
			radius = inVehicle ? std::max( smallestRadius, radiusInVehicles * RadiusFromBounds( availableVehicles[vehicle].mins, availableVehicles[vehicle].maxs ) ) : smallestRadius;
		Common::printf( "Checkpoint\n{\n" );
		printVector( "Origin", ps.origin );
		Common::printf( "\tRadius\t%.0f\n\tKind\t%s\n}\n", radius, VehicleData::checkpointKindName( kindFor( inVehicle ? vehicle : -1 ) ) );
		return;
	}
	if( !Q_stricmp( form, "start" ) || !Q_stricmp( form, "vehicle" ) )
	{
		const bool start = !Q_stricmp( form, "start" );
		if( !inVehicle )
		{
			Common::printf( "where %s: take a vehicle first\n", form );
			return;
		}
		Common::printf( "%s\n{\n", start ? "PlayerStart" : "Vehicle" );
		if( start )
		{
			Common::printf( "\tName\t\"%s\"\n\tDescriptive\t\"%s\"\n", availableVehicles[vehicle].modelName, availableVehicles[vehicle].descriptiveName );
			if( team[0] )
				Common::printf( "\tTeam\t%s\n", team );
		}
		else
		{
			printVehicleLines( vehicle );
			Common::printf( "\tTeam\t%d\n", availableVehicles[vehicle].team );
			if( team[0] )
				Common::printf( "\tSide\t%s\n", team );
		}
		printVector( "Origin", ps.origin );
		printVector( "Angles", angles );
		if( start )
			Common::printf( "\tSpeed\t%d\n\tThrottle\t%d\n", ps.speed, ps.fixed_throttle );
		Common::printf( "}\n" );
		return;
	}
	if( form[0] )
	{
		Common::printf( "usage: where [start | vehicle | checkpoint [radius] | waypoint | target]\n" );
		return;
	}
	char map[MAX_QPATH];
	COM_StripExtension( COM_SkipPath( cgs.mapname ), map, sizeof( map ) );
	Common::printf( "WHERE origin %.1f;%.1f;%.1f angles %.1f;%.1f;%.1f speed %d throttle %d vehicle \"%s\" team %s map %s\n",
		ps.origin[0], ps.origin[1], ps.origin[2], angles[0], angles[1], angles[2], ps.speed, ps.fixed_throttle,
		inVehicle ? availableVehicles[vehicle].descriptiveName : "none", team[0] ? team : "none", map );
}
