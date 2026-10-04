#include "g_local.h"
#include "g_level.h"
#include "g_bot.h"
#include <algorithm>
#include <cmath>

void SetTeam( GameEntity *ent, char *s );
void Cmd_Radar_f( GameEntity *ent );
void Cmd_Eject_f( GameEntity *ent );
void Cmd_Kill_f( GameEntity *ent );
void ClientUserinfoChanged( int clientNum );
void MF_ClientBegin( int clientNum );

std::array<Bots::State, MAX_CLIENTS + 1> Bots::states_;
int Bots::fillTime_ = 0;
std::vector<Bots::Pad> Bots::pads_;
bool Bots::padsBuilt_ = false;
std::vector<Vec3> Bots::roamPoints_;
std::vector<Bots::Character> Bots::characters_;

namespace
{
	constexpr std::array<Bots::Character, 16> builtinCharacters = { {
		{ "Alpha", 0.9f, 0.3f, 0.4f, 0.5f, 0.9f, 0.3f, 0.2f, { 1, 1, 1, 1, 1 } },
		{ "Bravo", 0.6f, 0.6f, 0.7f, 0.8f, 0.7f, 0.6f, 0.5f, { 1, 1, 1, 1, 1 } },
		{ "Charlie", 0.4f, 0.9f, 0.9f, 0.6f, 0.8f, 0.4f, 0.9f, { 1, 1, 1, 1, 1 } },
		{ "Delta", 1.0f, 0.2f, 0.2f, 0.3f, 0.6f, 0.9f, 0.1f, { 1, 1, 1, 1, 1 } },
		{ "Echo", 0.7f, 0.5f, 0.6f, 0.9f, 0.85f, 0.7f, 0.6f, { 1, 1, 1, 1, 1 } },
		{ "Foxtrot", 0.5f, 0.7f, 0.5f, 0.4f, 0.5f, 0.2f, 0.4f, { 1, 1, 1, 1, 1 } },
		{ "Golf", 0.8f, 0.4f, 0.8f, 0.7f, 0.95f, 0.8f, 0.8f, { 1, 1, 1, 1, 1 } },
		{ "Hotel", 0.3f, 0.8f, 0.6f, 0.9f, 0.6f, 0.5f, 0.3f, { 1, 1, 1, 1, 1 } },
		{ "India", 0.9f, 0.5f, 0.3f, 0.2f, 0.75f, 0.6f, 0.2f, { 1, 1, 1, 1, 1 } },
		{ "Juliet", 0.6f, 0.4f, 0.9f, 0.6f, 0.9f, 0.3f, 0.9f, { 1, 1, 1, 1, 1 } },
		{ "Kilo", 0.75f, 0.75f, 0.5f, 0.5f, 0.65f, 0.5f, 0.5f, { 1, 1, 1, 1, 1 } },
		{ "Lima", 0.5f, 0.3f, 0.3f, 0.8f, 0.55f, 0.7f, 0.1f, { 1, 1, 1, 1, 1 } },
		{ "Mike", 0.85f, 0.6f, 0.7f, 0.4f, 0.8f, 0.9f, 0.7f, { 1, 1, 1, 1, 1 } },
		{ "November", 0.4f, 0.6f, 0.8f, 0.7f, 0.7f, 0.4f, 0.6f, { 1, 1, 1, 1, 1 } },
		{ "Oscar", 0.95f, 0.35f, 0.5f, 0.6f, 1.0f, 1.0f, 0.4f, { 1, 1, 1, 1, 1 } },
		{ "Papa", 0.65f, 0.85f, 0.4f, 0.5f, 0.6f, 0.1f, 0.8f, { 1, 1, 1, 1, 1 } }
	} };

	constexpr float planeCruiseHeight = 1400.0f;
	constexpr float planeMinHeight = 450.0f;
	constexpr float heloCruiseHeight = 500.0f;
	constexpr float heloLowHeight = 180.0f;
	constexpr float bomberMinHeight = 2200.0f;
	constexpr float planeEnemyRange = 14000.0f;
	constexpr float surfaceEnemyRange = 7000.0f;

	float signedThrottle( const playerState_t& ps, int vehicle )
	{
		const int maxThrottle = availableVehicles[vehicle].maxthrottle;
		return ps.fixed_throttle > maxThrottle ? -static_cast<float>( ps.fixed_throttle - maxThrottle ) : static_cast<float>( ps.fixed_throttle );
	}

	bool alive( GameEntity* ent )
	{
		return ent && ent->inuse_ && ent->client_ && ent->health_ > 0 && ent->client_->vehicle_ >= 0 &&
			ent->client_->pers_.connected_ == GameClient::ClientPersistant::CON_CONNECTED &&
			ent->client_->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR &&
			!(ent->client_->ps_.pm_flags & PMF_VEHICLESELECT);
	}
}

void Bots::loadCharacters()
{
	characters_.clear();

	fileHandle_t f;
	const int len = FileSystem::fOpenFileByMode( "bots/characters.txt", &f, FS_READ );
	if( len > 0 && len < 32768 )
	{
		std::vector<char> text( len + 1, 0 );
		FileSystem::read( text.data(), len, f );
		char* line = text.data();
		while( line && *line )
		{
			char* next = strchr( line, '\n' );
			if( next )
				*next++ = 0;
			Character who = {};
			for( float& weight : who.prefer )
				weight = 1.0f;
			const int got = sscanf( line, "%31s %f %f %f %f %f %f %f %f %f %f %f %f", who.name, &who.aggression, &who.caution, &who.patience,
				&who.teamwork, &who.aim, &who.honour, &who.cunning, &who.prefer[0], &who.prefer[1], &who.prefer[2], &who.prefer[3], &who.prefer[4] );
			if( got >= 8 && who.name[0] != '#' && who.name[0] != '/' )
				characters_.push_back( who );
			line = next;
		}
	}
	if( len >= 0 )
		FileSystem::fCloseFile( f );

	if( characters_.empty() )
		characters_.assign( builtinCharacters.begin(), builtinCharacters.end() );
}

void Bots::reset()
{
	states_.fill( State{} );
	fillTime_ = 0;
	pads_.clear();
	padsBuilt_ = false;
	roamPoints_.clear();
	characters_.clear();
}

bool Bots::isBot( int clientNum )
{
	return clientNum >= 1 && clientNum <= MAX_CLIENTS && states_[clientNum].active;
}

int Bots::count()
{
	return static_cast<int>( std::count_if( states_.begin(), states_.end(), []( const State& s ) { return s.active; } ) );
}

void Bots::onConnect( int clientNum )
{
	if( clientNum < 1 || clientNum > MAX_CLIENTS )
		return;
	states_[clientNum] = State{};
	states_[clientNum].active = true;
	if( characters_.empty() )
		loadCharacters();
	states_[clientNum].character = clientNum % static_cast<int>( characters_.size() );
}

void Bots::onDisconnect( int clientNum )
{
	if( clientNum < 1 || clientNum > MAX_CLIENTS )
		return;
	states_[clientNum] = State{};
}

unsigned int Bots::spawnCategories()
{
	unsigned int cats = 0;
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->inuse_ || !ent->classname_ )
			continue;
		if( Q_stricmpn( ent->classname_, "info_player", 11 ) != 0 && Q_stricmpn( ent->classname_, "team_CTF", 8 ) != 0 )
			continue;
		cats |= ent->ent_category_;
	}
	return cats ? cats : CAT_ANY;
}

int Bots::pickVehicle( int teamFlag, bool allowInfantry, const Character* who )
{
	const unsigned int cats = spawnCategories();
	const unsigned long gameset = Game::getGameset();
	std::array<int, 256> candidates;
	std::array<int, 256> weights;
	int num = 0, total = 0;

	for( int i = 0; i < bg_numberOfVehicles && num < 256; i++ )
	{
		const completeVehicleData_t& veh = availableVehicles[i];
		if( !(veh.gameset & gameset) || !(veh.team & teamFlag) || !(veh.cat & cats) )
			continue;
		bool nuke = false;
		for( unsigned int weapon : veh.weapons )
		{
			if( availableWeapons[weapon].type == WT_NUKEBOMB || availableWeapons[weapon].type == WT_NUKEMISSILE )
				nuke = true;
		}
		if( nuke )
			continue;
		int weight = 0;
		int kind = 0;
		if( veh.cat & CAT_PLANE ) { weight = 4; kind = 0; }
		else if( veh.cat & CAT_HELO ) { weight = 8; kind = 1; }
		else if( veh.cat & CAT_GROUND ) { weight = 6; kind = 2; }
		else if( veh.cat & CAT_BOAT ) { weight = 12; kind = 3; }
		else if( (veh.cat & CAT_LQM) && allowInfantry ) { weight = 8; kind = 4; }
		if( who )
			weight = static_cast<int>( weight * 10 * std::max( 0.0f, who->prefer[kind] ) );
		if( weight <= 0 )
			continue;
		if( !weight )
			continue;
		candidates[num] = i;
		weights[num] = weight;
		total += weight;
		num++;
	}
	if( !num )
		return -1;

	int pick = rand() % total;
	for( int i = 0; i < num; i++ )
	{
		pick -= weights[i];
		if( pick < 0 )
			return candidates[i];
	}
	return candidates[0];
}

int Bots::add( int vehicle, const char* team, const char* name )
{
	const int clientNum = SV_BotAllocateClient();
	if( clientNum < 0 )
	{
		Common::printf( "No free client slot for a bot\n" );
		return -1;
	}

	if( characters_.empty() )
		loadCharacters();
	char userinfo[MAX_INFO_STRING] = "";
	Info::setValueForKey( userinfo, "name", name && name[0] ? name : characters_[clientNum % characters_.size()].name );
	Info::setValueForKey( userinfo, "rate", "25000" );
	Info::setValueForKey( userinfo, "snaps", "20" );
	Info::setValueForKey( userinfo, "handicap", "100" );
	Info::setValueForKey( userinfo, "team", team && team[0] ? team : "auto" );
	SV_SetUserinfo( clientNum, userinfo );

	if( MF_ClientConnect( clientNum, true, true ) )
	{
		SV_BotFreeClient( clientNum );
		return -1;
	}
	states_[clientNum].vehicle = vehicle;
	MF_ClientBegin( clientNum );
	return clientNum;
}

void Bots::remove( int clientNum )
{
	if( !isBot( clientNum ) )
		return;
	SV_GameDropClient( clientNum, "was removed" );
	states_[clientNum] = State{};
}

void Bots::removeAll()
{
	for( int i = 1; i <= MAX_CLIENTS; i++ )
		remove( i );
}

void Bots::fill()
{
	if( theLevel.time_ < fillTime_ )
		return;
	fillTime_ = theLevel.time_ + 1500;

	if( g_gametype.integer == GT_MISSION_EDITOR )
		return;

	int players = 0;
	int lastBot = -1;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameClient* cl = theLevel.getClient( i );
		if( !cl || cl->pers_.connected_ == GameClient::ClientPersistant::CON_DISCONNECTED )
			continue;
		if( isBot( i ) )
			lastBot = i;
		else if( cl->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR )
			continue;
		players++;
	}

	if( players < g_botMinPlayers.integer )
		add( -1, "", "" );
	else if( players > g_botMinPlayers.integer && lastBot >= 0 && g_botMinPlayers.integer > 0 )
		remove( lastBot );
}

void Bots::select( GameEntity* ent, State& bot )
{
	GameClient* client = ent->client_;
	const int clientNum = ent->s.number;

	if( theLevel.time_ < bot.commandTime )
		return;
	bot.commandTime = theLevel.time_ + 1000;

	if( client->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR )
	{
		char team[8] = "auto";
		SetTeam( ent, team );
		return;
	}

	int teamFlag = MF_TEAM_ANY;
	if( g_gametype.integer >= GT_TEAM )
		teamFlag = client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;

	if( bot.vehicle < 0 || bot.vehicle >= bg_numberOfVehicles || !(availableVehicles[bot.vehicle].team & teamFlag) )
		bot.vehicle = pickVehicle( teamFlag, true, &character( bot ) );
	if( bot.vehicle < 0 )
		return;

	char userinfo[MAX_INFO_STRING];
	SV_GetUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", bot.vehicle ) );
	SV_SetUserinfo( clientNum, userinfo );
	ClientUserinfoChanged( clientNum );
}

float Bots::heightAboveGround( const vec3_t origin, int passEntity )
{
	trace_t tr;
	vec3_t end;
	VectorCopy( origin, end );
	end[2] -= 16000;
	SV_Trace( &tr, origin, nullptr, nullptr, end, passEntity, MASK_SOLID|MASK_WATER, false );
	return tr.fraction * 16000.0f;
}

bool Bots::pathBlocked( const vec3_t origin, const vec3_t dir, float dist, int passEntity, trace_t& tr )
{
	vec3_t end;
	VectorMA( origin, dist, dir, end );
	SV_Trace( &tr, origin, nullptr, nullptr, end, passEntity, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	return tr.fraction < 1.0f;
}

const Bots::Character& Bots::character( const State& bot )
{
	if( characters_.empty() )
		loadCharacters();
	return characters_[bot.character % characters_.size()];
}

float Bots::skill( const State& bot )
{
	return std::clamp( g_botSkill.value, 1.0f, 5.0f ) / 5.0f * (0.5f + 0.5f * character( bot ).aim);
}

int Bots::memoryTime( const State& bot )
{
	return 12000 + static_cast<int>( 30000 * character( bot ).patience );
}

Bots::Role Bots::roleOf( int vehicle )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	if( veh.cat & CAT_PLANE )
		return (veh.cls & CLASS_PLANE_BOMBER) ? Role::Bomber : Role::Fighter;
	if( veh.cat & CAT_HELO )
		return (veh.cls & CLASS_HELO_TRANSPORT) ? Role::TransportHelo : Role::AttackHelo;
	if( veh.cat & CAT_GROUND )
		return (veh.cls & CLASS_GROUND_SAM) ? Role::AirDefence : Role::Tank;
	if( veh.cat & CAT_BOAT )
		return Role::Boat;
	return Role::Infantry;
}

bool Bots::hasWeaponType( int vehicle, const playerState_t& ps, int type )
{
	for( int slot : secondaryWeaponSlots )
	{
		const int weapon = availableVehicles[vehicle].weapons[slot];
		if( weapon && ps.ammo[slot] > 0 && availableWeapons[weapon].type == type )
			return true;
	}
	return false;
}

bool Bots::visible( GameEntity* ent, const vec3_t from, GameEntity* target )
{
	trace_t tr;
	SV_Trace( &tr, from, nullptr, nullptr, target->r.currentOrigin, ent->s.number, MASK_SOLID, false );
	return tr.fraction >= 1.0f || tr.entityNum == target->s.number;
}

float Bots::ceilingAbove( const vec3_t origin, int passEntity )
{
	trace_t tr;
	vec3_t end;
	VectorCopy( origin, end );
	end[2] += 8000;
	SV_Trace( &tr, origin, nullptr, nullptr, end, passEntity, MASK_PLAYERSOLID|CONTENTS_BOTCLIP, false );
	return tr.fraction * 8000.0f;
}

GameEntity* Bots::findEnemy( GameEntity* ent, State& bot, Role role )
{
	GameClient* client = ent->client_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const bool flyer = (veh.cat & (CAT_PLANE|CAT_HELO)) != 0;
	const float maxRange = flyer ? planeEnemyRange : surfaceEnemyRange;
	const float radarRange = veh.radarRange > 0 ? static_cast<float>( veh.radarRange ) : 8000.0f;
	const bool antiAir = hasWeaponType( client->vehicle_, client->ps_, WT_ANTIAIRMISSILE );

	GameEntity* best = nullptr;
	float bestScore = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) )
			continue;
		GameClient* oc = other->client_;
		if( oc->sess_.ridingClient_ >= 0 )
			continue;
		if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
			continue;

		float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		Seen& seen = bot.seen[i];
		const bool sees = dist <= maxRange &&
			!VehicleData::stealthHidden( oc->vehicle_, oc->ps_.timers[TIMER_REVEAL], theLevel.time_, dist, radarRange ) &&
			visible( ent, ent->r.currentOrigin, other );
		if( sees )
		{
			VectorCopy( other->r.currentOrigin, seen.position );
			seen.time = theLevel.time_;
		}
		else
		{
			if( !seen.time || theLevel.time_ > seen.time + memoryTime( bot ) )
				continue;
			dist = Distance( ent->r.currentOrigin, seen.position ) * 1.5f;
			if( dist < 300 )
			{
				seen.time = 0;
				continue;
			}
		}

		const completeVehicleData_t& ov = availableVehicles[oc->vehicle_];
		const bool air = (ov.cat & (CAT_PLANE|CAT_HELO)) != 0 && !(oc->ps_.ONOFF & OO_LANDED);
		float score = dist;
		switch( role )
		{
		case Role::Fighter:
			if( !air )
				score *= 1.6f;
			else if( (ov.cat & CAT_HELO) || (ov.cls & CLASS_PLANE_BOMBER) )
				score *= 0.6f;
			break;
		case Role::Bomber:
			if( air || (ov.cat & CAT_LQM) )
				continue;
			break;
		case Role::AttackHelo:
		case Role::TransportHelo:
			if( air )
				score *= (ov.cat & CAT_HELO) ? 1.2f : 3.0f;
			break;
		case Role::Tank:
			if( air && !antiAir )
				continue;
			if( air )
				score *= 1.5f;
			break;
		case Role::AirDefence:
			if( !air )
				score *= 2.0f;
			break;
		case Role::Boat:
			break;
		case Role::Infantry:
			if( dist > 1500 || air )
				continue;
			break;
		}
		if( i == bot.enemy )
			score *= 0.7f;
		if( g_gametype.integer == GT_CTF && (oc->ps_.objectives & (client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? OB_REDFLAG : OB_BLUEFLAG)) )
			score *= 0.15f;
		if( !best || score < bestScore )
		{
			best = other;
			bestScore = score;
		}
	}
	return best;
}

void Bots::buildRoamPoints()
{
	if( !padsBuilt_ )
		buildPads();
	roamPoints_.clear();

	Vec3 sum( 0, 0, 0 );
	int count = 0;
	float top = -99999.0f;
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->inuse_ || !ent->classname_ )
			continue;
		if( Q_stricmpn( ent->classname_, "info_player", 11 ) != 0 && Q_stricmpn( ent->classname_, "team_CTF", 8 ) != 0 )
			continue;
		sum += Vec3( ent->s.origin );
		top = std::max( top, ent->s.origin[2] );
		count++;
	}
	for( const Pad& pad : pads_ )
	{
		bool near = false;
		for( const Vec3& point : roamPoints_ )
		{
			vec3_t p;
			point.copyTo( p );
			if( Distance( p, pad.spot ) < 1200 )
				near = true;
		}
		if( !near )
			roamPoints_.push_back( Vec3( pad.spot ) );
		sum += Vec3( pad.spot );
		top = std::max( top, pad.spot[2] );
		count++;
	}
	if( !count )
		return;

	Vec3 centre = sum * (1.0f / count);
	vec3_t origin;
	centre.copyTo( origin );
	origin[2] = top + 1500;
	float reach[4];
	for( int k = 0; k < 4; k++ )
	{
		vec3_t dir = { k == 0 ? 1.0f : k == 1 ? -1.0f : 0.0f, k == 2 ? 1.0f : k == 3 ? -1.0f : 0.0f, 0 };
		trace_t tr;
		pathBlocked( origin, dir, 40000, ENTITYNUM_NONE, tr );
		reach[k] = tr.fraction * 40000.0f;
	}
	roamPoints_.push_back( centre );
	for( int qx = 0; qx < 2; qx++ )
	{
		for( int qy = 0; qy < 2; qy++ )
		{
			Vec3 point = centre;
			point += Vec3( (qx ? 0.55f : -0.55f) * reach[qx ? 0 : 1], (qy ? 0.55f : -0.55f) * reach[qy ? 2 : 3], 0 );
			roamPoints_.push_back( point );
		}
	}
}

bool Bots::steerAround( GameEntity* ent, float wantYaw, float travel, float& yaw )
{
	constexpr std::array<float, 9> offsets = { 0.0f, 20.0f, -20.0f, 45.0f, -45.0f, 75.0f, -75.0f, 110.0f, -110.0f };
	for( float offset : offsets )
	{
		vec3_t angles = { 0, wantYaw + offset, 0 }, dir;
		AngleVectors( angles, dir, nullptr, nullptr );
		trace_t tr;
		if( !pathBlocked( ent->r.currentOrigin, dir, travel, ent->s.number, tr ) || tr.entityNum != ENTITYNUM_WORLD )
		{
			yaw = wantYaw + offset;
			return true;
		}
	}
	return false;
}

bool Bots::runwayAt( const vec3_t point )
{
	vec3_t start, end;
	VectorCopy( point, start );
	start[2] += 60;
	VectorCopy( point, end );
	end[2] -= 140;
	trace_t tr;
	SV_Trace( &tr, start, nullptr, nullptr, end, ENTITYNUM_NONE, MASK_SOLID, false );
	if( tr.fraction >= 1.0f || tr.entityNum <= ENTITYNUM_WORLD )
		return false;
	GameEntity* ground = theLevel.getEntity( tr.entityNum );
	return ground && ground->classname_ && strcmp( ground->classname_, "func_runway" ) == 0;
}

float Bots::runwayRun( const vec3_t origin, float yaw, float maxDist )
{
	vec3_t angles = { 0, yaw, 0 }, dir, point;
	AngleVectors( angles, dir, nullptr, nullptr );
	float run = 0;
	for( float dist = 50.0f; dist <= maxDist; dist += 50.0f )
	{
		VectorMA( origin, dist, dir, point );
		if( !runwayAt( point ) )
			break;
		run = dist;
	}
	return run;
}

bool Bots::flagPosition( int team, vec3_t out )
{
	const char* name = team == ClientBase::TEAM_RED ? "team_CTF_redflag" : "team_CTF_blueflag";
	for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
	{
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->inuse_ || !ent->classname_ || Q_stricmp( ent->classname_, name ) != 0 )
			continue;
		VectorCopy( ent->r.currentOrigin, out );
		return true;
	}
	return false;
}

void Bots::buildPads()
{
	padsBuilt_ = true;
	pads_.clear();
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->inuse_ || !ent->classname_ || strcmp( ent->classname_, "trigger_recharge" ) != 0 )
			continue;

		Pad pad;
		for( int k = 0; k < 3; k++ )
			pad.center[k] = (ent->r.absmin[k] + ent->r.absmax[k]) * 0.5f;
		VectorCopy( pad.center, pad.spot );
		VectorSet( pad.axis, 1, 0, 0 );
		pad.halfLength = 0;
		pad.z = ent->r.absmin[2];
		pad.runway = false;
		pad.category = ent->ent_category_ ? ent->ent_category_ : CAT_ANY;
		pad.team = ent->s.generic1;

		vec3_t start, end;
		VectorCopy( pad.center, start );
		start[2] = ent->r.absmax[2] + 8;
		VectorCopy( start, end );
		end[2] -= 600;
		trace_t tr;
		SV_Trace( &tr, start, nullptr, nullptr, end, ENTITYNUM_NONE, MASK_SOLID, false );
		if( tr.fraction < 1.0f )
		{
			pad.z = tr.endpos[2];
			GameEntity* ground = tr.entityNum > ENTITYNUM_WORLD ? theLevel.getEntity( tr.entityNum ) : nullptr;
			if( ground && ground->classname_ && strcmp( ground->classname_, "func_runway" ) == 0 )
			{
				vec3_t spot;
				VectorCopy( pad.center, spot );
				spot[2] = pad.z;
				float best = 0;
				for( int step = 0; step < 12; step++ )
				{
					const float yaw = step * 15.0f;
					const float ahead = runwayRun( spot, yaw, 4000 );
					const float behind = runwayRun( spot, yaw + 180.0f, 4000 );
					if( ahead + behind > best )
					{
						best = ahead + behind;
						vec3_t angles = { 0, yaw, 0 };
						AngleVectors( angles, pad.axis, nullptr, nullptr );
						VectorMA( spot, (ahead - behind) * 0.5f, pad.axis, pad.center );
					}
				}
				pad.halfLength = best * 0.5f;
				pad.runway = best >= 1200.0f;
				pad.center[2] = pad.z;
			}
		}
		pads_.push_back( pad );
	}
}

int Bots::findPad( GameEntity* ent )
{
	if( !padsBuilt_ )
		buildPads();

	const GameClient* client = ent->client_;
	const unsigned int cat = availableVehicles[client->vehicle_].cat;
	int best = -1;
	float bestDist = 0;
	for( size_t i = 0; i < pads_.size(); i++ )
	{
		const Pad& pad = pads_[i];
		if( !(pad.category & cat) || ((cat & CAT_PLANE) && !pad.runway) )
			continue;
		if( g_gametype.integer >= GT_TEAM && pad.team != ClientBase::TEAM_FREE && pad.team != client->sess_.sessionTeam_ )
			continue;
		const float dist = Distance( ent->r.currentOrigin, pad.center );
		if( best < 0 || dist < bestDist )
		{
			best = static_cast<int>( i );
			bestDist = dist;
		}
	}
	return best;
}

bool Bots::replenished( GameEntity* ent )
{
	const GameClient* client = ent->client_;
	if( client->ps_.stats[STAT_FUEL] < client->ps_.stats[STAT_MAX_FUEL] * 0.95f || ent->health_ < client->pers_.maxHealth_ * 0.95f )
		return false;
	for( int slot : allWeaponSlots )
	{
		if( client->ps_.ammo[slot] < client->ps_.ammo[slot + 8] )
			return false;
	}
	return true;
}

void Bots::runwayHeading( GameEntity* ent, State& bot )
{
	vec3_t origin;
	VectorCopy( ent->r.currentOrigin, origin );
	origin[2] -= heightAboveGround( ent->r.currentOrigin, ent->s.number );
	if( !runwayAt( origin ) )
		return;
	float best = 0;
	for( int step = 0; step < 24; step++ )
	{
		const float yaw = step * 15.0f;
		const float run = runwayRun( origin, yaw, 4000 );
		vec3_t angles = { -6.0f, yaw, 0 }, dir, start;
		AngleVectors( angles, dir, nullptr, nullptr );
		VectorCopy( origin, start );
		start[2] += 40;
		trace_t tr;
		pathBlocked( start, dir, 2500, ent->s.number, tr );
		const float score = std::min( run, 500.0f ) * 4.0f + tr.fraction * 2500.0f;
		if( run >= 150 && score > best )
		{
			best = score;
			bot.takeoffYaw = yaw;
		}
	}
	if( best > 0 )
		bot.takeoffTime = theLevel.time_ + 30000;
}

void Bots::finishRearm( GameEntity* ent, State& bot )
{
	runwayHeading( ent, bot );
	bot.pad = -1;
	bot.landPhase = 0;
	bot.goalEvalTime = 0;
	bot.commitTime = 0;
	bot.desire = Goal::Roam;
}

bool Bots::landPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	const Pad& pad = pads_[bot.pad];
	if( !pad.runway )
		return false;

	const playerState_t& ps = ent->client_->ps_;
	const completeVehicleData_t& veh = availableVehicles[ent->client_->vehicle_];
	const float speed = static_cast<float>( ps.speed ) / 10.0f;
	const float stall = static_cast<float>( veh.stallspeed );

	if( ps.ONOFF & OO_LANDED )
	{
		vec3_t toSpot, spotAngles;
		VectorSubtract( pad.spot, ent->r.currentOrigin, toSpot );
		toSpot[2] = 0;
		const float spotDist = VectorLength( toSpot );
		vectoangles( toSpot, spotAngles );
		view[YAW] = speed > 60 ? ps.vehicleAngles[YAW] : spotAngles[YAW];
		view[PITCH] = 0;
		cmd.forwardmove = 0;
		if( speed > 60 || spotDist < 50 )
		{
			if( speed > 1 )
				cmd.forwardmove = -127;
			else if( replenished( ent ) )
				finishRearm( ent, bot );
		}
		else if( speed < 35 )
			cmd.forwardmove = 127;
		return true;
	}

	vec3_t rel;
	VectorSubtract( ent->r.currentOrigin, pad.center, rel );
	const float along = rel[0] * pad.axis[0] + rel[1] * pad.axis[1];
	const float side = rel[1] * pad.axis[0] - rel[0] * pad.axis[1];
	const float above = ent->r.currentOrigin[2] - pad.z;

	if( bot.landPhase == 0 )
	{
		bot.landDir = along > 0 ? -1.0f : 1.0f;
		float rise[2];
		for( int k = 0; k < 2; k++ )
		{
			vec3_t probe, down;
			VectorMA( pad.center, (k ? 1.0f : -1.0f) * (pad.halfLength + 250.0f), pad.axis, probe );
			probe[2] = pad.z + 1500;
			VectorCopy( probe, down );
			down[2] -= 3000;
			trace_t tr;
			SV_Trace( &tr, probe, nullptr, nullptr, down, ENTITYNUM_NONE, MASK_SOLID, false );
			rise[k] = tr.endpos[2] - pad.z;
		}
		if( rise[0] > rise[1] + 30 )
			bot.landDir = -1.0f;
		else if( rise[1] > rise[0] + 30 )
			bot.landDir = 1.0f;
		bot.landPhase = 1;
	}
	const float pos = along * bot.landDir;
	const float threshold = -(pad.halfLength - 150.0f);
	const float before = threshold - pos;

	vec3_t travel, angles, target;
	VectorScale( pad.axis, bot.landDir, travel );
	vectoangles( travel, angles );
	const float axisYaw = angles[YAW];

	cmd.forwardmove = 127;
	if( bot.landPhase == 1 )
	{
		VectorMA( pad.center, threshold - 3500.0f, travel, target );
		vec3_t dir;
		VectorSubtract( target, ent->r.currentOrigin, dir );
		dir[2] = 0;
		const float dist = VectorLength( dir );
		vectoangles( dir, angles );
		view[YAW] = angles[YAW];
		view[PITCH] = std::clamp( (above - 600.0f) / 30.0f, -30.0f, 20.0f );
		if( sense.height < planeMinHeight * 0.7f )
			view[PITCH] = -25.0f;
		if( dist < 800 || (before > 2000 && fabs( side ) < 500 && fabs( AngleSubtract( axisYaw, ps.vehicleAngles[YAW] ) ) < 25) )
			bot.landPhase = 2;
		return true;
	}

	if( before < -pad.halfLength || (before < -200 && above > 150) || fabs( side ) > 900 )
	{
		bot.landPhase = 1;
		return true;
	}

	VectorMA( pad.center, pos + 900.0f, travel, target );
	vec3_t dir;
	VectorSubtract( target, ent->r.currentOrigin, dir );
	dir[2] = 0;
	vectoangles( dir, angles );
	view[YAW] = angles[YAW];

	const float wantAbove = before > 0 ? std::clamp( before * 0.15f, 10.0f, 600.0f ) : 3.0f;
	view[PITCH] = std::clamp( (above - wantAbove) / 6.0f, -10.0f, 14.0f );
	if( above < 25 )
		view[PITCH] = -2.0f;
	if( before > 0 && sense.height < 45 )
		view[PITCH] = -8.0f;

	const float wantSpeed = stall * (before > 1200 ? 1.6f : 1.3f);
	cmd.forwardmove = speed < wantSpeed ? 127 : -127;
	if( speed > wantSpeed * 1.3f && !(ps.ONOFF & OO_SPEEDBRAKE) )
		cmd.buttons |= BUTTON_BRAKE;
	if( !(ps.ONOFF & OO_GEAR) && speed < stall * SPEED_GREEN_ARC * 0.95f )
		cmd.buttons |= BUTTON_GEAR;
	return true;
}

Bots::Goal Bots::chooseGoal( GameEntity* ent, State& bot, const Sense& sense )
{
	const Character& who = character( bot );
	const playerState_t& ps = ent->client_->ps_;
	const bool airborne = sense.role == Role::Fighter || sense.role == Role::Bomber;

	float attack = sense.enemy ? who.aggression * (0.4f + 0.6f * sense.health) : 0.0f;
	float evade = who.caution * ((1.0f - sense.health) * 1.2f + (sense.launched ? 0.5f : sense.locked ? 0.15f : 0.0f));
	float climb = 0;
	float hide = 0;
	if( sense.underFire )
		evade += who.caution * (sense.blindsided ? 1.4f : 0.3f);
	if( sense.blindsided && sense.role == Role::Tank )
		hide += who.caution * 1.2f;

	if( sense.role == Role::Bomber )
	{
		const float want = std::clamp( sense.height + ceilingAbove( ent->r.currentOrigin, ent->s.number ) - 700.0f, 800.0f, 3500.0f );
		climb = 1.2f * who.patience * std::clamp( 1.0f - sense.height / want, 0.0f, 1.0f ) + (sense.height < want * 0.6f ? 0.6f : 0.0f);
		if( sense.airThreat && sense.airThreatDist < 3000 )
			evade += who.caution * 0.8f;
		if( !hasWeaponType( ent->client_->vehicle_, ps, WT_IRONBOMB ) && !hasWeaponType( ent->client_->vehicle_, ps, WT_GUIDEDBOMB ) &&
			!hasWeaponType( ent->client_->vehicle_, ps, WT_ANTIGROUNDMISSILE ) && !hasWeaponType( ent->client_->vehicle_, ps, WT_ROCKET ) )
			attack *= 0.5f;
	}
	else if( sense.role == Role::Fighter )
	{
		if( sense.enemy && sense.enemyAir && sense.enemyDist > 3000 )
			climb = 0.5f * who.patience * std::clamp( (sense.enemy->r.currentOrigin[2] + 400 - ent->r.currentOrigin[2]) / 800.0f, 0.0f, 1.0f );
	}
	else if( sense.role == Role::Tank )
	{
		if( sense.airThreat && !hasWeaponType( ent->client_->vehicle_, ps, WT_ANTIAIRMISSILE ) )
			hide = who.caution * 1.6f * std::clamp( 1.3f - sense.airThreatDist / 5000.0f, 0.0f, 1.0f );
	}

	if( sense.stealth && sense.revealed && airborne )
		evade += who.caution * 0.6f;
	if( sense.stealth && !sense.revealed && sense.enemy && sense.enemyDist > 2500 )
		climb += 0.3f * who.patience;

	float rearm = 0;
	if( ps.stats[STAT_FUEL] < ps.stats[STAT_MAX_FUEL] * 0.35f )
		rearm = 1.0f;
	else if( sense.health < 0.45f )
		rearm = 0.2f + 0.8f * who.caution;
	else if( outOfAmmo( ent->client_->vehicle_, ps ) )
		rearm = 0.7f;
	else if( availableVehicles[ent->client_->vehicle_].ammo[WP_FLARE] > 0 && ps.ammo[WP_FLARE] <= 0 && sense.role != Role::Tank )
		rearm = 0.3f * who.caution;
	if( sense.duel )
	{
		rearm *= 1.0f - who.honour;
		evade *= 1.0f - 0.7f * who.honour;
	}
	if( bot.pad >= 0 )
		rearm = std::max( rearm, 0.9f );
	if( rearm > 0 && sense.role != Role::Infantry && findPad( ent ) < 0 )
		rearm = 0;
	if( sense.role == Role::Infantry )
		rearm = 0;

	const float keep = 1.25f + 0.25f * who.patience;
	switch( bot.desire )
	{
	case Goal::Attack: attack *= keep; break;
	case Goal::Evade: evade *= keep; break;
	case Goal::Climb: climb *= keep; break;
	case Goal::Hide: hide *= keep; break;
	default: break;
	}

	Goal goal = Goal::Roam;
	float best = 0.2f;
	if( attack > best ) { best = attack; goal = Goal::Attack; }
	if( evade > best ) { best = evade; goal = Goal::Evade; }
	if( climb > best ) { best = climb; goal = Goal::Climb; }
	if( hide > best ) { best = hide; goal = Goal::Hide; }
	if( rearm > best ) { best = rearm; goal = Goal::Rearm; }
	if( g_gametype.integer == GT_CTF && sense.role != Role::Fighter && sense.role != Role::Bomber )
	{
		const bool carrying = (ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0;
		const float capture = carrying ? 1.6f : 0.35f + 0.5f * who.teamwork;
		if( capture > best && (carrying || goal != Goal::Rearm) )
		{
			best = capture;
			goal = Goal::Capture;
		}
	}
	if( goal == Goal::Rearm )
	{
		if( bot.pad < 0 )
		{
			bot.pad = findPad( ent );
			bot.landPhase = 0;
		}
	}
	else
	{
		bot.pad = -1;
		bot.landPhase = 0;
	}
	return goal;
}

void Bots::sense( GameEntity* ent, State& bot, Sense& out )
{
	if( theLevel.time_ >= bot.enemyTime )
	{
		GameEntity* enemy = findEnemy( ent, bot, roleOf( ent->client_->vehicle_ ) );
		bot.enemy = enemy ? enemy->s.number : -1;
		if( !enemy && bot.leader >= 1 && isBot( bot.leader ) && states_[bot.leader].enemy >= 0 )
			bot.enemy = states_[bot.leader].enemy;
		bot.enemyTime = theLevel.time_ + 700 + (rand() % 300);
	}
	if( bot.enemy >= 0 )
	{
		GameEntity* enemy = theLevel.getEntity( bot.enemy );
		if( alive( enemy ) )
			out.enemy = enemy;
		else
			bot.enemy = -1;
	}
	if( out.enemy )
	{
		Seen& seen = bot.seen[out.enemy->s.number];
		out.enemyVisible = visible( ent, ent->r.currentOrigin, out.enemy );
		if( out.enemyVisible && seen.time )
		{
			VectorCopy( out.enemy->r.currentOrigin, seen.position );
			seen.time = theLevel.time_;
		}
		else if( !seen.time || theLevel.time_ > seen.time + memoryTime( bot ) )
		{
			out.enemy = nullptr;
			bot.enemy = -1;
			bot.enemyTime = 0;
		}
	}
	if( out.enemy )
	{
		vec3_t target;
		VectorCopy( out.enemyVisible ? out.enemy->r.currentOrigin : bot.seen[out.enemy->s.number].position, target );
		const float dist = Distance( ent->r.currentOrigin, target );
		const int weapon = availableVehicles[ent->client_->vehicle_].weapons[0];
		const float shotSpeed = weapon && availableWeapons[weapon].muzzleVelocity > 0 ? static_cast<float>( availableWeapons[weapon].muzzleVelocity ) : 2500.0f;
		if( out.enemyVisible )
			VectorMA( target, std::min( dist / shotSpeed, 1.5f ), out.enemy->client_->ps_.velocity, target );
		const unsigned int enemyCat = availableVehicles[out.enemy->client_->vehicle_].cat;
		if( roleOf( ent->client_->vehicle_ ) == Role::Fighter && (enemyCat & (CAT_PLANE|CAT_HELO)) && dist > 1200 && dist < 6000 &&
			character( bot ).cunning > 0.5f && out.enemy->tracktarget_ != ent )
		{
			vec3_t behind;
			AngleVectors( out.enemy->client_->ps_.vehicleAngles, behind, nullptr, nullptr );
			VectorMA( target, -700.0f, behind, target );
			target[2] -= 250.0f * character( bot ).cunning;
		}
		VectorSubtract( target, ent->r.currentOrigin, out.enemyDir );
		out.enemyDist = VectorNormalize( out.enemyDir );
		vectoangles( out.enemyDir, out.enemyAngles );
		out.enemyAngles[PITCH] = AngleSubtract( out.enemyAngles[PITCH], 0 );
		out.enemyAir = (availableVehicles[out.enemy->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) != 0 &&
			!(out.enemy->client_->ps_.ONOFF & OO_LANDED);
	}
	out.height = heightAboveGround( ent->r.currentOrigin, ent->s.number );

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	out.role = roleOf( client->vehicle_ );
	out.health = std::clamp( static_cast<float>( ent->health_ ) / std::max( 1, client->pers_.maxHealth_ ), 0.0f, 1.0f );
	out.locked = (ps.stats[STAT_LOCKINFO] & (LI_BEING_LOCKED|LI_BEING_LAUNCHED)) != 0;
	out.launched = (ps.stats[STAT_LOCKINFO] & LI_BEING_LAUNCHED) != 0;
	out.stealth = (availableVehicles[client->vehicle_].flags & VF_STEALTH) != 0;
	out.revealed = ps.timers[TIMER_REVEAL] > theLevel.time_;

	if( ent->health_ < bot.lastHealth - 2 )
	{
		if( theLevel.time_ > bot.hitTime + 4000 )
			bot.breakYaw = ps.vehicleAngles[YAW] + (rand() & 1 ? 80.0f : -80.0f);
		bot.hitTime = theLevel.time_;
	}
	bot.lastHealth = ent->health_;
	out.underFire = bot.hitTime && theLevel.time_ < bot.hitTime + 2500;
	if( out.underFire )
	{
		bool seen = false;
		if( out.enemy && out.enemyVisible )
		{
			vec3_t forward;
			AngleVectors( ps.viewangles, forward, nullptr, nullptr );
			seen = DotProduct( forward, out.enemyDir ) > 0.3f || ent->tracktarget_ == out.enemy;
		}
		out.blindsided = !seen;
	}

	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
			continue;
		if( !(availableVehicles[other->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) || (other->client_->ps_.ONOFF & OO_LANDED) )
			continue;
		const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		if( dist < 6000 && (!out.airThreat || dist < out.airThreatDist) )
		{
			out.airThreat = other;
			out.airThreatDist = dist;
		}
	}

	int foes = 0;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == ent || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
			continue;
		if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
			continue;
		if( Distance( ent->r.currentOrigin, other->r.currentOrigin ) < 5000 )
			foes++;
	}
	out.duel = out.enemy && out.enemyAir && foes == 1 && (out.role == Role::Fighter || out.role == Role::AttackHelo);

	if( theLevel.time_ >= bot.goalEvalTime && (theLevel.time_ >= bot.commitTime || out.launched || out.underFire || bot.desire == Goal::Roam) )
	{
		const Goal previous = bot.desire;
		bot.desire = chooseGoal( ent, bot, out );
		bot.goalEvalTime = theLevel.time_ + 1000;
		if( bot.desire != previous )
			bot.commitTime = theLevel.time_ + 3000 + static_cast<int>( 3000 * character( bot ).patience );
	}
}

void Bots::flares( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd )
{
	if( !sense.launched || ent->client_->ps_.ammo[WP_FLARE] <= 0 || theLevel.time_ < bot.flareTime )
		return;
	if( random() > skill( bot ) )
	{
		bot.flareTime = theLevel.time_ + 400;
		return;
	}
	cmd.buttons |= BUTTON_FLARE;
	bot.flareTime = theLevel.time_ + 700;
}

bool Bots::findHidePoint( GameEntity* ent, State& bot, GameEntity* threat, bool hullDown )
{
	float bestDist = 0;
	bool found = false;
	for( int ring = 0; ring < 3; ring++ )
	{
		const float radius = 350.0f + ring * 450.0f;
		for( int step = 0; step < 12; step++ )
		{
			const float angle = step * (2.0f * M_PI / 12.0f);
			vec3_t point, down;
			VectorCopy( ent->r.currentOrigin, point );
			point[0] += cos( angle ) * radius;
			point[1] += sin( angle ) * radius;
			point[2] += 300;
			VectorCopy( point, down );
			down[2] -= 2000;
			trace_t tr;
			SV_Trace( &tr, point, nullptr, nullptr, down, ent->s.number, MASK_SOLID|MASK_WATER, false );
			if( tr.startsolid || tr.fraction >= 1.0f || (tr.contents & MASK_WATER) )
				continue;
			VectorCopy( tr.endpos, point );
			point[2] += 20;
			if( fabs( point[2] - ent->r.currentOrigin[2] ) > radius * 0.5f )
				continue;
			if( visible( ent, point, threat ) )
				continue;
			if( hullDown )
			{
				vec3_t top;
				VectorCopy( point, top );
				top[2] += availableVehicles[ent->client_->vehicle_].maxs[2] + 12;
				if( !visible( ent, top, threat ) )
					continue;
			}
			const float dist = Distance( ent->r.currentOrigin, point );
			if( !found || dist < bestDist )
			{
				VectorCopy( point, bot.hidePoint );
				bestDist = dist;
				found = true;
			}
		}
		if( found )
			break;
	}
	return found;
}

GameEntity* Bots::findStructure( GameEntity* ent, State& bot )
{
	if( theLevel.time_ < bot.structureTime )
	{
		GameEntity* current = bot.structure >= 0 ? theLevel.getEntity( bot.structure ) : nullptr;
		if( current && current->inuse_ && current->s.eType == ET_EXPLOSIVE && current->takedamage_ && current->health_ > 0 )
			return current;
		if( bot.structure < 0 )
			return nullptr;
	}
	bot.structureTime = theLevel.time_ + 6000;
	bot.structure = -1;
	if( !padsBuilt_ )
		buildPads();

	const int team = ent->client_->sess_.sessionTeam_;
	GameEntity* best = nullptr;
	float bestScore = 0;
	for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !other || !other->inuse_ || other->s.eType != ET_EXPLOSIVE || !other->takedamage_ || other->health_ <= 0 )
			continue;
		vec3_t centre;
		for( int k = 0; k < 3; k++ )
			centre[k] = (other->r.absmin[k] + other->r.absmax[k]) * 0.5f;
		float score = Distance( ent->r.currentOrigin, centre );
		if( g_gametype.integer >= GT_TEAM )
		{
			bool own = false, enemy = false;
			for( const Pad& pad : pads_ )
			{
				if( pad.team == ClientBase::TEAM_FREE || Distance( pad.spot, centre ) > 2500 )
					continue;
				if( pad.team == team )
					own = true;
				else
					enemy = true;
			}
			if( own && !enemy )
				continue;
			if( enemy )
				score *= 0.4f;
		}
		if( !best || score < bestScore )
		{
			best = other;
			bestScore = score;
		}
	}
	if( best )
		bot.structure = best->s.number;
	return best;
}

void Bots::bomb( GameEntity* ent, State& bot, const vec3_t target, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];

	int slot = -1;
	for( int i = WP_WEAPON1; i < WP_FLARE && slot < 0; i++ )
	{
		const int weapon = veh.weapons[i];
		if( weapon && ps.ammo[i] > 0 && (availableWeapons[weapon].type == WT_IRONBOMB || availableWeapons[weapon].type == WT_GUIDEDBOMB) )
			slot = i;
	}
	if( slot < 0 )
		return;
	cmd.weapon = slot;

	vec3_t flat;
	VectorSubtract( target, ent->r.currentOrigin, flat );
	const float drop = -flat[2];
	flat[2] = 0;
	const float dist = VectorLength( flat );
	vec3_t angles;
	vectoangles( flat, angles );
	view[YAW] = angles[YAW];

	if( drop <= 0 || ps.weaponNum != slot )
		return;
	const float fall = sqrt( 2.0f * drop / DEFAULT_GRAVITY );
	const float speed = sqrt( ps.velocity[0] * ps.velocity[0] + ps.velocity[1] * ps.velocity[1] );
	const float lead = speed * fall;
	const float yawError = fabs( AngleSubtract( angles[YAW], ps.vehicleAngles[YAW] ) );
	if( yawError < 8 && fabs( dist - lead ) < 250.0f + 400.0f * (1.0f - skill( bot )) )
	{
		cmd.buttons |= BUTTON_ATTACK_MAIN;
		bot.bombTime = theLevel.time_;
	}
	if( dist < lead - 400 && theLevel.time_ > bot.bombTime + 500 )
		bot.egressTime = theLevel.time_ + 8000;
}

void Bots::pickGoal( GameEntity* ent, State& bot, float radius )
{
	if( !bot.hasHome )
	{
		VectorCopy( ent->r.currentOrigin, bot.home );
		bot.hasHome = true;
	}
	if( bot.goalTime )
	{
		if( theLevel.time_ > bot.goalTime )
			bot.goalsExpired++;
		else
			bot.goalsReached++;
	}
	if( roamPoints_.empty() )
		buildRoamPoints();
	const unsigned int cat = ent->client_->vehicle_ >= 0 ? availableVehicles[ent->client_->vehicle_].cat : 0;
	if( (cat & (CAT_PLANE|CAT_HELO)) && !roamPoints_.empty() && radius > 3500 )
	{
		const Vec3& point = roamPoints_[rand() % roamPoints_.size()];
		point.copyTo( bot.goal );
		bot.goal[0] += crandom() * 800.0f;
		bot.goal[1] += crandom() * 800.0f;
	}
	else
	{
		const float angle = random() * 2.0f * M_PI;
		const float dist = radius * (0.3f + 0.7f * random());
		bot.goal[0] = bot.home[0] + cos( angle ) * dist;
		bot.goal[1] = bot.home[1] + sin( angle ) * dist;
		bot.goal[2] = bot.home[2];
		if( cat & CAT_BOAT )
		{
			for( int tries = 0; tries < 8; tries++ )
			{
				vec3_t probe;
				VectorCopy( bot.goal, probe );
				probe[2] = ent->r.currentOrigin[2] - 6;
				if( SV_PointContents( probe, ent->s.number ) & MASK_WATER )
					break;
				const float a = random() * 2.0f * M_PI;
				const float d = radius * (0.2f + 0.8f * random());
				bot.goal[0] = ent->r.currentOrigin[0] + cos( a ) * d;
				bot.goal[1] = ent->r.currentOrigin[1] + sin( a ) * d;
			}
		}
	}
	bot.goalTime = theLevel.time_ + 20000;
}

int Bots::findWeaponSlot( int vehicle, const playerState_t& ps, bool air, float dist, bool& guided )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	int best = -1;
	float bestScore = 0;
	guided = false;
	for( int slot : secondaryWeaponSlots )
	{
		const int weapon = veh.weapons[slot];
		if( !weapon || ps.ammo[slot] <= 0 )
			continue;
		const completeWeaponData_t& data = availableWeapons[weapon];
		const float range = data.range > 0 ? static_cast<float>( data.range ) : 3000.0f;
		bool lock = false;
		float score = 0;
		switch( data.type )
		{
		case WT_ANTIAIRMISSILE:
			if( !air )
				continue;
			lock = true;
			score = dist <= range ? 10000.0f - range : 2000.0f + range * 0.01f;
			if( dist < 500 )
				score = 500.0f;
			break;
		case WT_ANTIGROUNDMISSILE:
		case WT_ANTIRADARMISSILE:
			if( air )
				continue;
			lock = true;
			score = dist <= range ? 9000.0f - range * 0.1f : 2000.0f;
			break;
		case WT_ROCKET:
			if( air )
				continue;
			score = dist < 3000 ? 6000.0f : 1000.0f;
			break;
		case WT_BALLISTICGUN:
			if( air )
				continue;
			score = 7000.0f;
			break;
		case WT_MACHINEGUN:
			if( !data.turret && !(data.fitsPylon & PF_GUNPOD) )
				continue;
			score = dist < 2500 ? 3000.0f : 800.0f;
			break;
		default:
			continue;
		}
		if( best < 0 || score > bestScore )
		{
			best = slot;
			bestScore = score;
			guided = lock;
		}
	}
	return best;
}

bool Bots::outOfAmmo( int vehicle, const playerState_t& ps )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	bool any = false;
	for( int slot = WP_MACHINEGUN; slot < WP_FLARE; slot++ )
	{
		if( !veh.weapons[slot] || veh.ammo[slot] <= 0 )
			continue;
		any = true;
		if( ps.ammo[slot] * 5 > ps.ammo[slot + 8] )
			return false;
	}
	return any;
}

void Bots::setRadar( GameEntity* ent, State& bot, bool air )
{
	const playerState_t& ps = ent->client_->ps_;
	const completeVehicleData_t& veh = availableVehicles[ent->client_->vehicle_];
	const int want = air ? OO_RADAR_AIR : OO_RADAR_GROUND;
	if( (air && veh.radarRange <= 0) || (!air && veh.radarRange2 <= 0) )
		return;
	if( (ps.ONOFF & OO_RADAR) == want || theLevel.time_ < bot.commandTime )
		return;
	bot.commandTime = theLevel.time_ + 300;
	Cmd_Radar_f( ent );
}

void Bots::fireWeapons( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, float aimError )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];

	if( !sense.enemy )
		return;

	trace_t tr;
	SV_Trace( &tr, ent->r.currentOrigin, nullptr, nullptr, sense.enemy->r.currentOrigin, ent->s.number, MASK_SOLID, false );
	const bool clear = tr.fraction >= 1.0f || tr.entityNum == sense.enemy->s.number;

	bool guided = false;
	const int slot = findWeaponSlot( client->vehicle_, ps, sense.enemyAir, sense.enemyDist, guided );
	if( slot >= 0 )
		cmd.weapon = slot;
	else
		cmd.weapon = ps.weaponNum;
	setRadar( ent, bot, sense.enemyAir );

	if( !clear )
		return;

	const int gun = veh.weapons[WP_MACHINEGUN];
	if( gun && ps.ammo[WP_MACHINEGUN] > 0 && aimError < 3.0f && sense.enemyDist < 2500 )
		cmd.buttons |= BUTTON_ATTACK;

	if( slot < 0 || ps.weaponNum != slot )
		return;

	if( guided )
	{
		if( (ps.stats[STAT_LOCKINFO] & LI_LOCKING) && ent->tracktarget_ == sense.enemy )
			cmd.buttons |= BUTTON_ATTACK_MAIN;
		return;
	}

	const int type = availableWeapons[veh.weapons[slot]].type;
	const float range = type == WT_BALLISTICGUN ? 6000.0f : 3000.0f;
	if( aimError < (type == WT_ROCKET ? 4.0f : 2.0f) && sense.enemyDist < range )
		cmd.buttons |= BUTTON_ATTACK_MAIN;
}

void Bots::thinkPlane( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const float speed = static_cast<float>( ps.speed ) / 10.0f;

	cmd.forwardmove = 127;

	if( bot.desire == Goal::Rearm && bot.pad >= 0 && landPlane( ent, bot, sense, cmd, view ) )
		return;

	if( ps.ONOFF & OO_LANDED )
	{
		if( theLevel.time_ >= bot.takeoffTime && speed < 20 )
			runwayHeading( ent, bot );
		view[YAW] = theLevel.time_ < bot.takeoffTime ? bot.takeoffYaw : ps.vehicleAngles[YAW];
		view[PITCH] = speed > veh.stallspeed * 0.97f ? -8.0f : 0.0f;
		if( fabs( AngleSubtract( view[YAW], ps.vehicleAngles[YAW] ) ) > 6 )
			cmd.forwardmove = speed < 12 ? 127 : -127;
		return;
	}

	if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 2000 )
		pickGoal( ent, bot, 9000 );

	flares( ent, bot, sense, cmd );

	if( g_gametype.integer >= GT_TEAM && sense.role == Role::Fighter && !sense.enemy && character( bot ).teamwork > 0.5f )
	{
		GameEntity* leader = nullptr;
		float leaderDist = 0;
		for( int i = 1; i < ent->s.number; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( !alive( other ) || !OnSameTeam( ent, other ) || roleOf( other->client_->vehicle_ ) != Role::Fighter || (other->client_->ps_.ONOFF & OO_LANDED) )
				continue;
			const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
			if( dist < 9000 && (!leader || dist < leaderDist) )
			{
				leader = other;
				leaderDist = dist;
			}
		}
		bot.leader = leader ? leader->s.number : -1;
		if( leader )
		{
			vec3_t forward, right;
			AngleVectors( leader->client_->ps_.vehicleAngles, forward, right, nullptr );
			VectorMA( leader->r.currentOrigin, 600.0f, forward, bot.goal );
			VectorMA( bot.goal, 300.0f, right, bot.goal );
			bot.goalTime = theLevel.time_ + 2000;
		}
	}

	const bool bomber = sense.role == Role::Bomber;
	float cruise = planeCruiseHeight;
	if( bomber )
		cruise = std::clamp( sense.height + ceilingAbove( ent->r.currentOrigin, ent->s.number ) - 700.0f, 800.0f, 3500.0f );
	else if( bot.desire == Goal::Climb )
		cruise = sense.height + 500.0f;
	else if( !sense.enemy || !sense.enemyAir )
		cruise = planeCruiseHeight - 800.0f * character( bot ).cunning;

	vec3_t dir, angles;
	bool attacking = false;
	bool bombing = false;
	int forcedWeapon = -1;
	VectorSubtract( bot.goal, ent->r.currentOrigin, dir );
	dir[2] = 0;
	vectoangles( dir, angles );
	angles[PITCH] = 0;

	if( bot.desire == Goal::Evade && sense.blindsided )
		angles[YAW] = bot.breakYaw;
	else if( sense.enemy && bot.desire == Goal::Evade )
		angles[YAW] = sense.enemyAngles[YAW] + 180.0f;
	else if( sense.enemy && bot.desire == Goal::Attack && theLevel.time_ > bot.egressTime )
	{
		const bool hasBombs = hasWeaponType( client->vehicle_, ps, WT_IRONBOMB ) || hasWeaponType( client->vehicle_, ps, WT_GUIDEDBOMB );
		const bool diver = character( bot ).aggression > 0.8f || character( bot ).cunning > 0.75f;
		if( bomber && hasBombs && !sense.enemyAir && diver && sense.height > 900 && sense.enemyDist < sense.height * 2.2f )
		{
			VectorCopy( sense.enemyAngles, angles );
			attacking = true;
			for( int slot : secondaryWeaponSlots )
			{
				const int weapon = veh.weapons[slot];
				if( weapon && ps.ammo[slot] > 0 && (availableWeapons[weapon].type == WT_IRONBOMB || availableWeapons[weapon].type == WT_GUIDEDBOMB) )
				{
					forcedWeapon = slot;
					vec3_t nose;
					AngleVectors( ps.vehicleAngles, nose, nullptr, nullptr );
					if( ps.weaponNum == slot && DotProduct( nose, sense.enemyDir ) > 0.99f && sense.enemyDist < 1800 )
						cmd.buttons |= BUTTON_ATTACK_MAIN;
					break;
				}
			}
			if( sense.enemyDist < 1000 || sense.height < 700 )
				bot.egressTime = theLevel.time_ + 6000;
		}
		else if( bomber && hasBombs && !sense.enemyAir )
			bombing = true;
		else if( sense.stealth && !sense.revealed && sense.enemyDist > 2500 )
			angles[YAW] = sense.enemyAngles[YAW];
		else
		{
			VectorCopy( sense.enemyAngles, angles );
			attacking = true;
			if( !sense.enemyAir && sense.enemyDist < 900 )
				bot.egressTime = theLevel.time_ + 5000;
			if( sense.enemyAir && sense.enemyDist < 500 )
			{
				bot.avoidYaw = ps.vehicleAngles[YAW] + 70.0f;
				bot.avoidTime = theLevel.time_ + 1500;
				bot.egressTime = theLevel.time_ + 2500;
			}
		}
	}

	view[YAW] = angles[YAW];
	if( attacking && (sense.enemyAir || sense.height > planeMinHeight * 1.5f) )
		view[PITCH] = std::clamp( angles[PITCH], -50.0f, 35.0f );
	else
		view[PITCH] = std::clamp( (sense.height - cruise) / 30.0f, -30.0f, 20.0f );
	if( bombing )
		bomb( ent, bot, sense.enemy->r.currentOrigin, cmd, view );
	else if( bomber && !attacking && bot.desire == Goal::Roam && theLevel.time_ > bot.egressTime && sense.height > 700.0f &&
		(hasWeaponType( client->vehicle_, ps, WT_IRONBOMB ) || hasWeaponType( client->vehicle_, ps, WT_GUIDEDBOMB )) )
	{
		GameEntity* structure = findStructure( ent, bot );
		if( structure )
		{
			vec3_t target;
			for( int k = 0; k < 3; k++ )
				target[k] = (structure->r.absmin[k] + structure->r.absmax[k]) * 0.5f;
			bomb( ent, bot, target, cmd, view );
		}
	}

	vec3_t forward;
	const float travel = std::max( 1200.0f, VectorLength( ps.velocity ) * 3.0f );
	trace_t tr;
	AngleVectors( ps.vehicleAngles, forward, nullptr, nullptr );
	if( pathBlocked( ent->r.currentOrigin, forward, travel, ent->s.number, tr ) )
	{
		if( tr.entityNum != ENTITYNUM_WORLD )
		{
			if( tr.fraction * travel < 700 )
			{
				bot.avoidYaw = ps.vehicleAngles[YAW] + 50.0f;
				bot.avoidTime = theLevel.time_ + 1200;
				bot.egressTime = theLevel.time_ + 2500;
				view[PITCH] = -30.0f;
			}
		}
		else
		{
			if( fabs( tr.plane.normal[2] ) < 0.7f )
			{
				float around;
				if( steerAround( ent, view[YAW], travel, around ) )
				{
					bot.avoidYaw = around;
					bot.avoidTime = theLevel.time_ + 1200;
				}
				else
				{
					vec3_t home;
					VectorSubtract( bot.home, ent->r.currentOrigin, home );
					vectoangles( home, angles );
					bot.avoidYaw = angles[YAW];
					bot.avoidTime = theLevel.time_ + 3000;
					pickGoal( ent, bot, 9000 );
				}
			}
			else
				view[PITCH] = tr.plane.normal[2] > 0 ? -45.0f : 20.0f;
		}
	}
	if( theLevel.time_ < bot.avoidTime )
	{
		view[YAW] = bot.avoidYaw;
		if( attacking )
			view[PITCH] = std::clamp( (sense.height - cruise) / 30.0f, -30.0f, 20.0f );
	}
	if( sense.height < planeMinHeight )
		view[PITCH] = -40.0f;

	vec3_t up = { 0, 0, 1 };
	if( pathBlocked( ent->r.currentOrigin, up, 700, ent->s.number, tr ) && sense.height > planeMinHeight * 2 )
		view[PITCH] = std::max( view[PITCH], 15.0f );

	if( speed < veh.stallspeed * 1.3f && sense.height > planeMinHeight * 2 )
		view[PITCH] = std::max( view[PITCH], 5.0f );
	else if( speed < veh.stallspeed * 1.7f )
		view[PITCH] = std::max( view[PITCH], -10.0f );

	if( ps.ONOFF & OO_STALLED )
		view[PITCH] = speed > veh.stallspeed ? ps.vehicleAngles[PITCH] + 1.0f : std::max( view[PITCH], ps.vehicleAngles[PITCH] + 1.0f );

	if( attacking )
	{
		vec3_t fwd;
		AngleVectors( ps.vehicleAngles, fwd, nullptr, nullptr );
		const float aimError = acos( std::clamp( DotProduct( fwd, sense.enemyDir ), -1.0f, 1.0f ) ) * 180.0f / M_PI;
		fireWeapons( ent, bot, sense, cmd, aimError );
		if( forcedWeapon >= 0 )
			cmd.weapon = forcedWeapon;
	}
}

void Bots::thinkHelo( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];

	if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 800 )
		pickGoal( ent, bot, 5000 );

	bool wait = false;
	if( g_gametype.integer >= GT_TEAM && (ps.ONOFF & OO_LANDED) && theLevel.time_ < bot.spawnTime + 12000 )
	{
		for( int i = 1; i <= theLevel.maxclients_ && !wait; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == ent || !alive( other ) || !OnSameTeam( ent, other ) )
				continue;
			if( !(availableVehicles[other->client_->vehicle_].cat & CAT_LQM) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( Distance( ent->r.currentOrigin, other->r.currentOrigin ) < 1200 )
				wait = true;
		}
	}

	vec3_t dir, angles;
	float dist;
	if( sense.enemy )
	{
		VectorCopy( sense.enemyAngles, angles );
		dist = sense.enemyDist;
	}
	else
	{
		VectorSubtract( bot.goal, ent->r.currentOrigin, dir );
		dir[2] = 0;
		dist = VectorLength( dir );
		vectoangles( dir, angles );
		angles[PITCH] = 0;
	}
	const bool rearming = bot.desire == Goal::Rearm && bot.pad >= 0;
	bool overPad = false;
	if( rearming )
	{
		VectorSubtract( pads_[bot.pad].center, ent->r.currentOrigin, dir );
		dir[2] = 0;
		dist = VectorLength( dir );
		vectoangles( dir, angles );
		angles[PITCH] = 0;
		overPad = dist < 120;
		if( (ps.ONOFF & OO_LANDED) && overPad )
		{
			wait = true;
			if( replenished( ent ) )
				finishRearm( ent, bot );
		}
	}
	const bool evading = bot.desire == Goal::Evade && (sense.enemy || sense.blindsided);
	view[YAW] = evading ? (sense.blindsided ? bot.breakYaw : angles[YAW] + 180.0f) : angles[YAW];
	view[PITCH] = sense.enemy ? std::clamp( AngleSubtract( angles[PITCH], 0 ), -30.0f, 45.0f ) : 0.0f;
	flares( ent, bot, sense, cmd );

	const float yawError = fabs( AngleSubtract( view[YAW], ps.vehicleAngles[YAW] ) );

	float wantHeight = sense.role == Role::AttackHelo ? heloLowHeight : heloCruiseHeight * 0.6f;
	bool cover = false;
	if( sense.enemy && bot.desire == Goal::Attack && dist < 3000 )
	{
		if( theLevel.time_ < bot.hideTime )
		{
			cover = true;
			wantHeight = 110.0f;
		}
		else if( !bot.exposed )
		{
			if( sense.enemyVisible )
			{
				bot.exposed = true;
				bot.popTime = theLevel.time_ + 3000 + static_cast<int>( 4000 * character( bot ).patience );
			}
			else
				wantHeight = std::min( sense.height + 300.0f, 900.0f );
		}
		if( bot.exposed )
		{
			wantHeight = std::max( sense.height, heloLowHeight );
			if( theLevel.time_ > bot.popTime )
			{
				bot.exposed = false;
				bot.hideTime = theLevel.time_ + 2000 + static_cast<int>( 3000 * character( bot ).caution );
				bot.hasHide = findHidePoint( ent, bot, sense.enemy );
			}
		}
	}
	else
		bot.exposed = false;
	if( bot.desire == Goal::Capture && dist < 500 )
		wantHeight = 25.0f;
	if( rearming )
	{
		cover = true;
		wantHeight = overPad ? -200.0f : heloCruiseHeight * 0.6f;
		if( overPad && !(ps.ONOFF & OO_GEAR) )
			cmd.buttons |= BUTTON_GEAR;
	}
	vec3_t forward, flat;
	trace_t tr;
	VectorSet( flat, 0, ps.vehicleAngles[YAW], 0 );
	AngleVectors( flat, forward, nullptr, nullptr );
	const bool blocked = pathBlocked( ent->r.currentOrigin, forward, 900, ent->s.number, tr ) && (tr.entityNum == ENTITYNUM_WORLD || tr.fraction < 0.35f);
	if( blocked )
		wantHeight = std::min( sense.height + 400.0f, 1500.0f );

	const float throttle = signedThrottle( ps, client->vehicle_ );
	float wantThrottle = std::clamp( (wantHeight - sense.height) / 60.0f, -3.0f, static_cast<float>( veh.maxthrottle ) );
	if( wait )
		wantThrottle = 0;
	else if( ps.ONOFF & OO_LANDED )
		wantThrottle = std::max( wantThrottle, veh.maxthrottle * 0.6f );
	if( throttle < floor( wantThrottle ) )
		cmd.buttons |= BUTTON_INCREASE;
	else if( throttle > ceil( wantThrottle ) )
		cmd.buttons |= BUTTON_DECREASE;

	float wantPitch = 0;
	if( !(ps.ONOFF & OO_LANDED) && !blocked && sense.height > 90 )
	{
		const float standoff = sense.enemy ? (sense.enemyAir ? 500.0f : 1200.0f) : 0.0f;
		if( rearming )
			wantPitch = overPad ? 0.0f : (yawError < 40 ? std::clamp( dist / 60.0f, 3.0f, 14.0f ) : 0.0f);
		else if( evading )
			wantPitch = yawError < 60 ? 14.0f : 0.0f;
		else if( cover && bot.hasHide && !rearming )
		{
			vec3_t toCover, coverAngles;
			VectorSubtract( bot.hidePoint, ent->r.currentOrigin, toCover );
			toCover[2] = 0;
			vectoangles( toCover, coverAngles );
			view[YAW] = coverAngles[YAW];
			wantPitch = VectorLength( toCover ) > 150 && fabs( AngleSubtract( coverAngles[YAW], ps.vehicleAngles[YAW] ) ) < 50 ? 12.0f : 0.0f;
		}
		else if( cover )
			wantPitch = -5.0f;
		else if( yawError < 50 && dist > standoff + 300 )
			wantPitch = sense.height > 300 ? 14.0f : 9.0f;
		else if( sense.enemy && dist < standoff )
			wantPitch = -6.0f;
	}
	if( !(ps.ONOFF & OO_LANDED) )
	{
		const float pitchError = wantPitch - ps.vehicleAngles[PITCH];
		if( pitchError > 2 )
			cmd.forwardmove = 127;
		else if( pitchError < -2 )
			cmd.forwardmove = -127;
		if( ps.vehicleAngles[ROLL] > 2 )
			cmd.rightmove = -127;
		else if( ps.vehicleAngles[ROLL] < -2 )
			cmd.rightmove = 127;
	}

	if( sense.enemy && !(ps.ONOFF & OO_LANDED) && !cover && !evading )
		fireWeapons( ent, bot, sense, cmd, yawError );
}

void Bots::thinkSurface( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const bool boat = (veh.cat & CAT_BOAT) != 0;

	if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 400 )
		pickGoal( ent, bot, boat ? 6000 : 4000 );

	vec3_t dir, angles;
	bool hold = false;
	if( sense.enemy && !sense.enemyAir )
	{
		VectorCopy( sense.enemyDir, dir );
		hold = sense.enemyDist < 1500;
	}
	else
		VectorSubtract( bot.goal, ent->r.currentOrigin, dir );

	flares( ent, bot, sense, cmd );
	if( bot.desire == Goal::Hide && sense.airThreat )
	{
		if( !bot.hasHide || theLevel.time_ > bot.hideTime )
		{
			bot.hasHide = findHidePoint( ent, bot, sense.airThreat );
			bot.hideTime = theLevel.time_ + 4000;
		}
		if( bot.hasHide )
		{
			VectorSubtract( bot.hidePoint, ent->r.currentOrigin, dir );
			hold = fabs( dir[0] ) + fabs( dir[1] ) < 90;
		}
		else
		{
			VectorSubtract( ent->r.currentOrigin, sense.airThreat->r.currentOrigin, dir );
			hold = false;
		}
	}
	else if( bot.desire == Goal::Attack && sense.enemy && !sense.enemyAir && !boat && sense.enemyDist > 900 && sense.enemyDist < 4500 &&
		character( bot ).caution > 0.5f )
	{
		if( theLevel.time_ > bot.hideTime )
		{
			bot.hasHide = findHidePoint( ent, bot, sense.enemy, true );
			bot.hideTime = theLevel.time_ + 5000;
		}
		if( bot.hasHide )
		{
			VectorSubtract( bot.hidePoint, ent->r.currentOrigin, dir );
			hold = fabs( dir[0] ) + fabs( dir[1] ) < 90;
		}
	}
	else if( bot.desire == Goal::Rearm && bot.pad >= 0 )
	{
		VectorSubtract( pads_[bot.pad].spot, ent->r.currentOrigin, dir );
		hold = fabs( dir[0] ) + fabs( dir[1] ) < 80;
		if( hold && replenished( ent ) )
			finishRearm( ent, bot );
	}
	else
	{
		bot.hasHide = false;
		if( bot.desire == Goal::Evade && sense.enemy )
		{
			VectorNegate( sense.enemyDir, dir );
			hold = false;
		}
	}
	dir[2] = 0;
	vectoangles( dir, angles );

	float wantYaw = angles[YAW];
	vec3_t forward, flat;
	trace_t tr;
	VectorSet( flat, 0, ps.vehicleAngles[YAW], 0 );
	AngleVectors( flat, forward, nullptr, nullptr );

	bool blocked = false;
	vec3_t start;
	VectorCopy( ent->r.currentOrigin, start );
	start[2] += veh.maxs[2] * 0.5f + 8;
	if( pathBlocked( start, forward, 260, ent->s.number, tr ) && tr.plane.normal[2] < 0.6f )
		blocked = true;
	if( boat && !blocked )
	{
		vec3_t ahead;
		VectorMA( ent->r.currentOrigin, 500, forward, ahead );
		ahead[2] = ent->r.currentOrigin[2] - 6;
		if( !(SV_PointContents( ahead, ent->s.number ) & MASK_WATER) )
			blocked = true;
	}

	if( theLevel.time_ > bot.stuckCheckTime )
	{
		if( !hold && theLevel.time_ > bot.spawnTime + 4000 && theLevel.time_ > bot.reverseTime + 2000 &&
			Distance( ent->r.currentOrigin, bot.lastOrigin ) < 40 )
			blocked = true;
		VectorCopy( ent->r.currentOrigin, bot.lastOrigin );
		bot.stuckCheckTime = theLevel.time_ + 2500;
	}
	if( blocked && theLevel.time_ > bot.reverseTime )
	{
		bot.reverseTime = theLevel.time_ + 1800;
		bot.reverseTurn = (rand() & 1) ? 127 : -127;
		pickGoal( ent, bot, boat ? 6000 : 4000 );
	}

	if( theLevel.time_ < bot.reverseTime )
	{
		cmd.forwardmove = -127;
		cmd.rightmove = bot.reverseTurn;
	}
	else
	{
		const float yawError = AngleSubtract( wantYaw, ps.vehicleAngles[YAW] );
		if( yawError < -6 )
			cmd.rightmove = 127;
		else if( yawError > 6 )
			cmd.rightmove = -127;
		if( !hold )
			cmd.forwardmove = 127;
	}

	if( sense.enemy )
	{
		view[YAW] = sense.enemyAngles[YAW];
		view[PITCH] = std::clamp( AngleSubtract( sense.enemyAngles[PITCH], 0 ), -60.0f, 20.0f );
		const float turretYaw = ps.vehicleAngles[YAW] + ps.turretAngle;
		const float aimError = fabs( AngleSubtract( sense.enemyAngles[YAW], turretYaw ) );
		fireWeapons( ent, bot, sense, cmd, aimError );
	}
	else
	{
		view[YAW] = ps.vehicleAngles[YAW];
		view[PITCH] = 0;
	}
}

void Bots::thinkInfantry( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;

	GameEntity* ride = nullptr;
	float rideDist = 0;
	for( int i = 0; i < theLevel.num_entities_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !other || !other->inuse_ || other == ent || other->health_ <= 0 )
			continue;
		bool candidate = false;
		if( other->s.eType == ET_MISC_VEHICLE && (other->s.seatMask & SEATMASK_ENTERABLE) )
			candidate = true;
		else if( other->client_ && alive( other ) && i <= theLevel.maxclients_ && other->client_->sess_.ridingClient_ < 0 &&
			!(availableVehicles[other->client_->vehicle_].cat & (CAT_LQM|CAT_PLANE)) &&
			g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) &&
			VectorLength( other->client_->ps_.velocity ) < 40 )
			candidate = true;
		if( !candidate )
			continue;
		const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		if( dist > 4000 )
			continue;
		if( !ride || dist < rideDist )
		{
			ride = other;
			rideDist = dist;
		}
	}

	vec3_t dir, angles;
	if( ride )
	{
		VectorSubtract( ride->r.currentOrigin, ent->r.currentOrigin, dir );
		if( rideDist < g_enterRange.value * 0.9f && theLevel.time_ > bot.commandTime )
		{
			bot.commandTime = theLevel.time_ + 1500;
			Cmd_Eject_f( ent );
			return;
		}
	}
	else
	{
		if( !padsBuilt_ )
			buildPads();
		const Pad* nearest = nullptr;
		float nearestDist = 0;
		for( const Pad& pad : pads_ )
		{
			const float dist = Distance( ent->r.currentOrigin, pad.spot );
			if( !nearest || dist < nearestDist )
			{
				nearest = &pad;
				nearestDist = dist;
			}
		}
		const int patience = 20000 + static_cast<int>( 30000 * character( bot ).patience );
		if( theLevel.time_ > bot.spawnTime + patience || (!nearest && theLevel.time_ > bot.spawnTime + 8000) || nearestDist > 6000 )
		{
			int teamFlag = MF_TEAM_ANY;
			if( g_gametype.integer >= GT_TEAM )
				teamFlag = client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;
			const int vehicle = pickVehicle( teamFlag, false, &character( bot ) );
			if( vehicle >= 0 && theLevel.time_ > bot.commandTime )
			{
				bot.commandTime = theLevel.time_ + 2000;
				bot.vehicle = vehicle;
				char userinfo[MAX_INFO_STRING];
				SV_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
				Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", vehicle ) );
				SV_SetUserinfo( ent->s.number, userinfo );
				ClientUserinfoChanged( ent->s.number );
				Cmd_Kill_f( ent );
				return;
			}
		}
		if( nearest && nearestDist > 150 )
			VectorSubtract( nearest->spot, ent->r.currentOrigin, dir );
		else
		{
			if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 60 )
				pickGoal( ent, bot, 600 );
			VectorSubtract( bot.goal, ent->r.currentOrigin, dir );
		}
	}
	dir[2] = 0;
	vectoangles( dir, angles );
	view[YAW] = angles[YAW];
	view[PITCH] = 0;
	cmd.forwardmove = 127;

	if( theLevel.time_ > bot.stuckCheckTime )
	{
		if( Distance( ent->r.currentOrigin, bot.lastOrigin ) < 8 && theLevel.time_ > bot.spawnTime + 3000 )
		{
			bot.reverseTime = theLevel.time_ + 1200;
			bot.reverseTurn = (rand() & 1) ? 127 : -127;
		}
		VectorCopy( ent->r.currentOrigin, bot.lastOrigin );
		bot.stuckCheckTime = theLevel.time_ + 2000;
	}
	if( theLevel.time_ < bot.reverseTime )
		cmd.rightmove = bot.reverseTurn;

	if( sense.enemy && sense.enemyDist < 900 && !sense.enemyAir )
	{
		view[YAW] = sense.enemyAngles[YAW];
		view[PITCH] = std::clamp( AngleSubtract( sense.enemyAngles[PITCH], 0 ), -60.0f, 60.0f );
		cmd.forwardmove = 0;
		if( client->ps_.ammo[WP_MACHINEGUN] > 0 )
			cmd.buttons |= BUTTON_ATTACK;
	}
}

void Bots::thinkRider( GameEntity* ent, State& bot, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	GameEntity* pilot = theLevel.getEntity( client->sess_.ridingClient_ );
	if( !alive( pilot ) )
		return;

	VectorCopy( pilot->client_->ps_.vehicleAngles, view );
	view[ROLL] = 0;
	const bool gunner = client->sess_.seat_ == 1 && VehicleData::vehicleHasTurret( pilot->client_->vehicle_ );
	if( !gunner && !(availableVehicles[pilot->client_->vehicle_].cat & (CAT_HELO|CAT_BOAT)) )
		return;
	const float reach = gunner ? 3500.0f : 1500.0f;

	if( theLevel.time_ >= bot.enemyTime )
	{
		GameEntity* enemy = nullptr;
		float best = 0;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == ent || other == pilot || !alive( other ) || other->client_->sess_.ridingClient_ >= 0 )
				continue;
			if( g_gametype.integer >= GT_TEAM && OnSameTeam( ent, other ) )
				continue;
			const float dist = Distance( pilot->r.currentOrigin, other->r.currentOrigin );
			if( dist < reach && (!enemy || dist < best) )
			{
				enemy = other;
				best = dist;
			}
		}
		bot.enemy = enemy ? enemy->s.number : -1;
		bot.enemyTime = theLevel.time_ + 800;
	}
	GameEntity* enemy = bot.enemy >= 0 ? theLevel.getEntity( bot.enemy ) : nullptr;
	if( !alive( enemy ) )
		return;

	vec3_t dir, angles;
	VectorSubtract( enemy->r.currentOrigin, pilot->r.currentOrigin, dir );
	vectoangles( dir, angles );
	view[YAW] = angles[YAW];
	view[PITCH] = std::clamp( AngleSubtract( angles[PITCH], 0 ), -60.0f, 60.0f );
	if( !visible( pilot, pilot->r.currentOrigin, enemy ) )
		return;
	cmd.buttons |= gunner ? BUTTON_ATTACK_MAIN : BUTTON_ATTACK;
}

void Bots::think( GameEntity* ent, State& bot, usercmd_t& cmd )
{
	GameClient* client = ent->client_;
	playerState_t& ps = client->ps_;
	vec3_t view;
	VectorCopy( ps.viewangles, view );

	const bool selecting = client->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR || client->vehicle_ < 0 || (ps.pm_flags & PMF_VEHICLESELECT);
	if( selecting )
	{
		bot.wasAlive = false;
		if( client->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR || client->nextVehicle_ < 0 )
			select( ent, bot );
		else if( (theLevel.time_ / 200) & 1 )
			cmd.buttons |= BUTTON_ATTACK;
		return;
	}
	if( ent->health_ <= 0 )
	{
		if( bot.wasAlive )
		{
			const int score = ps.persistant[PERS_SCORE];
			bot.badLives = score > bot.lifeScore ? 0 : bot.badLives + 1;
			if( bot.badLives >= 2 && random() < 0.4f + 0.4f * (1.0f - character( bot ).patience) )
			{
				int teamFlag = MF_TEAM_ANY;
				if( g_gametype.integer >= GT_TEAM )
					teamFlag = client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;
				const int vehicle = pickVehicle( teamFlag, false, &character( bot ) );
				if( vehicle >= 0 )
				{
					bot.vehicle = vehicle;
					bot.badLives = 0;
					char userinfo[MAX_INFO_STRING];
					SV_GetUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
					Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", vehicle ) );
					SV_SetUserinfo( ent->s.number, userinfo );
					bot.wasAlive = false;
					ClientUserinfoChanged( ent->s.number );
					return;
				}
			}
		}
		bot.wasAlive = false;
		if( (theLevel.time_ / 200) & 1 )
			cmd.buttons |= BUTTON_ATTACK;
		return;
	}
	if( !bot.wasAlive )
	{
		bot.wasAlive = true;
		bot.spawnTime = theLevel.time_;
		bot.lifeScore = ps.persistant[PERS_SCORE];
		bot.goalTime = 0;
		bot.enemy = -1;
		bot.enemyTime = 0;
		bot.egressTime = 0;
		bot.avoidTime = 0;
		bot.reverseTime = 0;
		bot.stuckCheckTime = theLevel.time_ + 3000;
		bot.pad = -1;
		bot.landPhase = 0;
		bot.desire = Goal::Roam;
		bot.commitTime = 0;
		bot.takeoffTime = 0;
		bot.hitTime = 0;
		bot.lastHealth = ent->health_;
		bot.exposed = false;
		bot.hasHide = false;
		VectorCopy( ent->r.currentOrigin, bot.lastOrigin );
		if( !bot.hasHome )
		{
			VectorCopy( ent->r.currentOrigin, bot.home );
			bot.hasHome = true;
		}
	}

	cmd.weapon = ps.weaponNum;

	if( g_gametype.integer == GT_CTF && bot.desire != Goal::Rearm && !(ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) )
	{
		const int own = client->sess_.sessionTeam_;
		const int ownFlag = own == ClientBase::TEAM_RED ? OB_REDFLAG : OB_BLUEFLAG;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == ent || !alive( other ) || !(other->client_->ps_.objectives & ownFlag) )
				continue;
			vec3_t intercept;
			const Seen& seen = bot.seen[i];
			if( seen.time && theLevel.time_ < seen.time + memoryTime( bot ) )
				VectorCopy( seen.position, intercept );
			else if( !flagPosition( own == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED, intercept ) )
				break;
			VectorCopy( intercept, bot.goal );
			bot.goalTime = theLevel.time_ + 3000;
			if( bot.desire == Goal::Capture )
				bot.desire = Goal::Roam;
			break;
		}
	}

	if( bot.desire == Goal::Capture )
	{
		const bool carrying = (ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0;
		const int own = client->sess_.sessionTeam_;
		const int other = own == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED;
		vec3_t flag;
		if( flagPosition( carrying ? own : other, flag ) )
		{
			VectorCopy( flag, bot.goal );
			bot.goalTime = theLevel.time_ + 3000;
		}
	}

	if( ps.stats[STAT_FUEL] < ps.stats[STAT_MAX_FUEL] / 4 )
		ps.stats[STAT_FUEL] = ps.stats[STAT_MAX_FUEL] / 4;

	if( client->sess_.ridingClient_ >= 0 )
	{
		thinkRider( ent, bot, cmd, view );
	}
	else
	{
		Sense s;
		sense( ent, bot, s );
		const unsigned int cat = availableVehicles[client->vehicle_].cat;
		if( cat & CAT_PLANE )
			thinkPlane( ent, bot, s, cmd, view );
		else if( cat & CAT_HELO )
			thinkHelo( ent, bot, s, cmd, view );
		else if( cat & (CAT_GROUND|CAT_BOAT) )
			thinkSurface( ent, bot, s, cmd, view );
		else
			thinkInfantry( ent, bot, s, cmd, view );
	}

	const float wobble = (1.0f - skill( bot )) * 3.0f;
	bot.aimPhase += 0.11f;
	view[YAW] += sin( bot.aimPhase ) * wobble;
	view[PITCH] += cos( bot.aimPhase * 0.7f ) * wobble * 0.6f;

	for( int i = 0; i < 3; i++ )
		cmd.angles[i] = ANGLE2SHORT( view[i] ) - ps.delta_angles[i];
}

void Bots::runFrame()
{
	if( g_botMinPlayers.integer > 0 )
		fill();

	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		State& bot = states_[i];
		if( !bot.active )
			continue;
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->client_ || ent->client_->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED )
			continue;
		ent->r.svFlags |= SVF_BOT;

		usercmd_t cmd;
		memset( &cmd, 0, sizeof( cmd ) );
		cmd.serverTime = theLevel.time_;
		think( ent, bot, cmd );
		SV_BotUserCommand( i, &cmd );
	}
}

void Bots::addCommand()
{
	char arg[MAX_TOKEN_CHARS];
	char team[MAX_TOKEN_CHARS];
	char name[MAX_TOKEN_CHARS];
	CommandSystem::argvBuffer( 1, arg, sizeof( arg ) );
	CommandSystem::argvBuffer( 2, team, sizeof( team ) );
	CommandSystem::argvBuffer( 3, name, sizeof( name ) );

	int vehicle = -1;
	if( arg[0] >= '0' && arg[0] <= '9' )
	{
		vehicle = atoi( arg );
		if( vehicle >= bg_numberOfVehicles )
			vehicle = -1;
	}
	else if( arg[0] )
	{
		unsigned int cat = 0;
		if( !Q_stricmp( arg, "plane" ) ) cat = CAT_PLANE;
		else if( !Q_stricmp( arg, "helo" ) ) cat = CAT_HELO;
		else if( !Q_stricmp( arg, "ground" ) ) cat = CAT_GROUND;
		else if( !Q_stricmp( arg, "boat" ) ) cat = CAT_BOAT;
		else if( !Q_stricmp( arg, "infantry" ) ) cat = CAT_LQM;
		const unsigned long gameset = Game::getGameset();
		std::array<int, 256> list;
		int num = 0;
		for( int i = 0; cat && i < bg_numberOfVehicles && num < 256; i++ )
		{
			if( (availableVehicles[i].cat & cat) && (availableVehicles[i].gameset & gameset) )
				list[num++] = i;
		}
		if( num )
			vehicle = list[rand() % num];
	}

	const int clientNum = add( vehicle, team, name );
	if( clientNum >= 0 )
		Common::printf( "Added bot %d\n", clientNum );
}

void Bots::kickCommand()
{
	removeAll();
}

void Bots::listCommand()
{
	if( !padsBuilt_ )
		buildPads();
	for( const Pad& pad : pads_ )
		Common::printf( "PAD spot %.0f %.0f %.0f centre %.0f %.0f axis %.2f %.2f half %.0f runway %d cat %x team %d\n", pad.spot[0], pad.spot[1], pad.spot[2],
			pad.center[0], pad.center[1], pad.axis[0], pad.axis[1], pad.halfLength, pad.runway, pad.category, pad.team );
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		if( !states_[i].active )
			continue;
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->client_ )
			continue;
		GameClient* cl = ent->client_;
		Common::printf( "BOT %d %s vehicle %d health %d org %.0f %.0f %.0f speed %d enemy %d riding %d score %d height %.0f onoff %x angles %.0f %.0f fuel %d goal %d role %d ammo %d %d %d %d goals %d %d\n", i, cl->pers_.netname_,
			cl->vehicle_, ent->health_, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], cl->ps_.speed,
			states_[i].enemy, cl->sess_.ridingClient_, cl->ps_.persistant[PERS_SCORE], heightAboveGround( ent->r.currentOrigin, i ),
			cl->ps_.ONOFF, cl->ps_.vehicleAngles[PITCH], cl->ps_.vehicleAngles[YAW], cl->ps_.stats[STAT_FUEL],
			static_cast<int>( states_[i].desire ), cl->vehicle_ >= 0 ? static_cast<int>( roleOf( cl->vehicle_ ) ) : -1,
			cl->ps_.ammo[1], cl->ps_.ammo[2], cl->ps_.ammo[3], cl->ps_.ammo[4], states_[i].goalsReached, states_[i].goalsExpired );
	}
}
