#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include <algorithm>
#include <cmath>

std::array<Bots::State, MAX_CLIENTS + 1> Bots::states_;
int Bots::fillTime_ = 0;
std::vector<Bots::Pad> Bots::pads_;
bool Bots::padsBuilt_ = false;
std::vector<Vec3> Bots::roamPoints_;
std::vector<Bots::Character> Bots::characters_;

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
