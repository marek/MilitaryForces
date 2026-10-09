#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include "bot_airnav.h"
#include "bot_groundnav.h"
#include <algorithm>
#include <cmath>

std::array<Bots::State, MAX_CLIENTS + 1> Bots::states_;
int Bots::fillTime_ = 0;
std::vector<Bots::Pad> Bots::pads_;
bool Bots::padsBuilt_ = false;
std::array<int, 5> Bots::kindLives_ = {};
std::array<int, 5> Bots::kindCrashes_ = {};
std::array<int, MAX_VEHICLE_TYPES> Bots::typeLives_ = {};
std::array<int, MAX_VEHICLE_TYPES> Bots::typeCrashes_ = {};
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
	kindLives_.fill( 0 );
	kindCrashes_.fill( 0 );
	typeLives_.fill( 0 );
	typeCrashes_.fill( 0 );
	AirNav::reset();
	GroundNav::reset();
}

bool Bots::isBot( int clientNum )
{
	return clientNum >= 1 && clientNum <= MAX_CLIENTS && states_[clientNum].active;
}

int Bots::count()
{
	return static_cast<int>( std::count_if( states_.begin(), states_.end(), []( const State& s ) { return s.active; } ) );
}

// the kind of vehicle, as an index into the per-kind tallies: plane, helicopter, ground, boat, soldier
int Bots::kindOf( int vehicle )
{
	const unsigned int cat = availableVehicles[vehicle].cat;
	if( cat & CAT_PLANE ) return 0;
	if( cat & CAT_HELO ) return 1;
	if( cat & CAT_GROUND ) return 2;
	if( cat & CAT_BOAT ) return 3;
	return 4;
}

// at least three lives in this vehicle type on this map, and two in three of them ended in an early crash
bool Bots::unfitHere( int type )
{
	return type >= 0 && type < MAX_VEHICLE_TYPES && typeLives_[type] >= 3 && typeCrashes_[type] * 3 >= typeLives_[type] * 2;
}

// tallies lives per kind of vehicle and, among them, crashes soon after spawning: a kind that keeps
// crashing on this map is picked less
void Bots::onDeath( GameEntity* self, GameEntity* attacker, int meansOfDeath )
{
	if( !self || !self->client_ || !isBot( self->s.number ) || self->client_->vehicle_ < 0 )
		return;

	const State& bot = states_[self->s.number];
	const int kind = kindOf( self->client_->vehicle_ );
	const int type = self->client_->vehicle_;
	kindLives_[kind]++;
	if( type < MAX_VEHICLE_TYPES )
		typeLives_[type]++;
	if( (meansOfDeath == MOD_CRASH || meansOfDeath == MOD_WATER) && theLevel.time_ - bot.spawnTime < 20000 )
	{
		kindCrashes_[kind]++;
		if( type < MAX_VEHICLE_TYPES )
			typeCrashes_[type]++;
	}
#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer )
		logDeath( self, attacker, meansOfDeath );
#endif
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

// how much a character minds failing: cautious ones give a vehicle up sooner, patient ones later
float Bots::failureSensitivity( const Character& who )
{
	return std::clamp( 0.3f + who.caution - 0.5f * who.patience, 0.1f, 1.2f );
}

// how much this bot wants a different kind of vehicle, from how often its lives in this kind ended badly
float Bots::switchDesire( const State& bot, int kind )
{
	if( bot.ownLives[kind] < 2 )
		return 0.0f;
	const float failureRate = static_cast<float>( bot.ownFailures[kind] ) / bot.ownLives[kind];
	return std::clamp( failureRate * failureSensitivity( character( bot ) ) * (0.4f + 0.2f * bot.badLives), 0.0f, 0.9f );
}

bool Bots::teamHasSupplier( int team, GameEntity* except )
{
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other && other != except && other->inuse_ && other->client_ && other->client_->sess_.sessionTeam_ == team && other->client_->vehicle_ >= 0 &&
			carriesSupplies( other->client_->vehicle_ ) )
			return true;
	}
	return false;
}

// a weighted draw among the vehicles this team may take on this map
int Bots::pickVehicle( int teamFlag, bool allowInfantry, const Character* who, const State* memory )
{
	unsigned int cats = spawnCategories();
	if( g_botVehicles.string[0] )
	{
		unsigned int wanted = 0;
		if( strstr( g_botVehicles.string, "plane" ) ) wanted |= CAT_PLANE;
		if( strstr( g_botVehicles.string, "helo" ) ) wanted |= CAT_HELO;
		if( strstr( g_botVehicles.string, "ground" ) ) wanted |= CAT_GROUND;
		if( strstr( g_botVehicles.string, "boat" ) ) wanted |= CAT_BOAT;
		if( strstr( g_botVehicles.string, "infantry" ) ) wanted |= CAT_LQM;
		if( cats & wanted )
			cats &= wanted;
	}
	const bool groundTargets = (cats & (CAT_GROUND|CAT_BOAT)) != 0;
	const unsigned long gameset = Game::getGameset();
	// a team with nobody flying supplies: team-minded characters consider taking the supply helicopter
	const bool teamHasSupplier = g_gametype.integer < GT_TEAM || Bots::teamHasSupplier( teamFlag == MF_TEAM_1 ? ClientBase::TEAM_RED : ClientBase::TEAM_BLUE );
	std::array<int, 256> candidates;
	std::array<int, 256> weights;
	int num = 0, total = 0;

	for( int i = 0; i < bg_numberOfVehicles && num < 256; i++ )
	{
		const completeVehicleData_t& veh = availableVehicles[i];
		if( !(veh.gameset & gameset) || !(veh.team & teamFlag) || !(veh.cat & cats) || !VehicleData::selectable( i ) )
			continue;
		// bots do not take nuclear weapons
		bool nuke = false;
		for( unsigned int weapon : veh.weapons )
		{
			if( availableWeapons[weapon].type == WT_NUKEBOMB || availableWeapons[weapon].type == WT_NUKEMISSILE )
				nuke = true;
		}
		if( nuke )
			continue;
		// base weight per kind evens out how many entries each kind has in the vehicle table
		int weight = 0;
		const int kind = kindOf( i );
		if( veh.cat & CAT_PLANE ) weight = 4;
		else if( veh.cat & CAT_HELO ) weight = 8;
		else if( veh.cat & CAT_GROUND ) weight = 6;
		else if( veh.cat & CAT_BOAT ) weight = 12;
		else if( (veh.cat & CAT_LQM) && allowInfantry ) weight = 8;
		// the character's own taste
		if( who )
			weight = static_cast<int>( weight * 10 * std::max( 0.0f, who->prefer[kind] ) );
		// supplies are for team mates: nobody flies the supply helicopter in a free-for-all
		if( g_gametype.integer < GT_TEAM && carriesSupplies( i ) )
			continue;
		if( who && !teamHasSupplier && carriesSupplies( i ) )
			weight = static_cast<int>( weight * (2.0f + 8.0f * who->teamwork) );
		// bombers have little to do on a map with nothing on the ground to bomb
		if( !groundTargets && (veh.cat & CAT_PLANE) && (veh.cls & CLASS_PLANE_BOMBER) )
			weight = weight * 12 / 100;
		// more than half of all bots' lives in this kind ended in an early crash here: the map does not suit it
		if( kindLives_[kind] >= 12 && kindCrashes_[kind] * 10 > kindLives_[kind] * 5 )
			weight = std::max( 1, weight / 3 );
		// this very aircraft has flown into the map nearly every time it was tried: it does not fit here (a wide-turning
		// bomber in a canyon), and nobody takes it again this map
		if( unfitHere( i ) )
			continue;
		// and this bot's own record in it
		if( memory && who && memory->ownLives[kind] > 0 )
		{
			const float failureRate = static_cast<float>( memory->ownFailures[kind] ) / memory->ownLives[kind];
			weight = std::max( 1, static_cast<int>( weight / (1.0f + failureRate * memory->ownLives[kind] * failureSensitivity( *who )) ) );
		}

		if( weight <= 0 )
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
	const int clientNum = GameHost::botAllocateClient();
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
	GameHost::setUserinfo( clientNum, userinfo );

	if( MF_ClientConnect( clientNum, true, true ) )
	{
		GameHost::botFreeClient( clientNum );
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
	GameHost::dropClient( clientNum, "was removed" );
	states_[clientNum] = State{};
}

void Bots::removeAll()
{
	for( int i = 1; i <= MAX_CLIENTS; i++ )
		remove( i );
}

// keeps the number of players at g_botMinPlayers: one bot joins or leaves per check
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

// what a person does in the menus: join a team, then choose a vehicle
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
		bot.vehicle = pickVehicle( teamFlag, true, &character( bot ), &bot );
	if( bot.vehicle < 0 )
		return;

	char userinfo[MAX_INFO_STRING];
	GameHost::getUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", bot.vehicle ) );
	GameHost::setUserinfo( clientNum, userinfo );
	ClientUserinfoChanged( clientNum );
}

void Bots::think( GameEntity* ent, State& bot, usercmd_t& cmd )
{
	GameClient* client = ent->client_;
	playerState_t& ps = client->ps_;
	Vec3 view;
	VectorCopy( ps.viewangles, view );

	// a life has ended: settle its record once. A crash goes straight to the vehicle menu without a frame spent dead,
	// so this comes before both
	const bool selecting = client->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR || client->vehicle_ < 0 || (ps.pm_flags & PMF_VEHICLESELECT);
	if( bot.wasAlive && (selecting || ent->health_ <= 0) && client->sess_.sessionTeam_ != ClientBase::TEAM_SPECTATOR && (client->vehicle_ >= 0 || bot.vehicle >= 0) )
	{
		const int score = ps.persistant[PERS_SCORE];
		const int flown = client->vehicle_ >= 0 ? client->vehicle_ : bot.vehicle;
		const int kind = kindOf( flown );
		const bool failed = score <= bot.lifeScore;
		bot.badLives = failed ? bot.badLives + 1 : 0;
		bot.ownLives[kind]++;
		if( failed )
			bot.ownFailures[kind]++;
		// nobody sits out life after life in an aircraft the map keeps killing
		bool reconsider = random() < switchDesire( bot, kind ) || unfitHere( flown );
		// a supply pilot whose supplies nobody used in a long life gets bored, the impatient first;
		// one who scored with them stays (a scoring life is not a failed one)
		constexpr int longLife = 120000;
		const bool supplier = carriesSupplies( flown );
		if( supplier && failed && theLevel.time_ - bot.spawnTime > longLife && random() < 0.6f * (1.0f - character( bot ).patience) )
			reconsider = true;
		// every death is a chance to notice that the team has nobody flying supplies
		const bool teamGame = g_gametype.integer >= GT_TEAM;
		const bool fillSupply = teamGame && !supplier && !reconsider && !teamHasSupplier( client->sess_.sessionTeam_, ent ) &&
			random() < 0.35f * character( bot ).teamwork;
		if( reconsider || fillSupply )
		{
			int teamFlag = MF_TEAM_ANY;
			if( g_gametype.integer >= GT_TEAM )
				teamFlag = client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;
			int vehicle = pickVehicle( teamFlag, false, &character( bot ), &bot );
			if( fillSupply && vehicle >= 0 && !carriesSupplies( vehicle ) )
				vehicle = -1;
			if( vehicle >= 0 )
			{
				bot.vehicle = vehicle;
				bot.badLives = 0;
				char userinfo[MAX_INFO_STRING];
				GameHost::getUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
				Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", vehicle ) );
				GameHost::setUserinfo( ent->s.number, userinfo );
				bot.wasAlive = false;
				ClientUserinfoChanged( ent->s.number );
				return;
			}
		}
	}

	// in the menus: choose, then tap fire to spawn
	if( selecting )
	{
		bot.wasAlive = false;
		if( client->sess_.sessionTeam_ == ClientBase::TEAM_SPECTATOR || client->nextVehicle_ < 0 )
			select( ent, bot );
		else if( (theLevel.time_ / 200) & 1 )
			cmd.buttons |= BUTTON_ATTACK;
		return;
	}
	// dead: tap fire to respawn
	if( ent->health_ <= 0 )
	{
		bot.wasAlive = false;
		if( (theLevel.time_ / 200) & 1 )
			cmd.buttons |= BUTTON_ATTACK;
		return;
	}
	// first frame of a new life
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
		bot.tree = behavior::Memory{};
		// some lives are played as a team player, more often by team-minded characters
		// (and more readily when a real player is on the team)
		bool humanOnTeam = false;
		for( int i = 1; i <= theLevel.maxclients_ && g_gametype.integer >= GT_TEAM; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other != ent && other && other->inuse_ && other->client_ && !isBot( i ) && OnSameTeam( ent, other ) )
				humanOnTeam = true;
		}
		// tanks and helicopters hunt in packs for safety: about two lives in five; planes about one in five
		const Role role = roleOf( client->vehicle_ );
		const bool packHunter = role == Role::Tank || role == Role::AirDefence || role == Role::AttackHelo || role == Role::TransportHelo;
		const float keenness = (packHunter ? 0.68f : 0.35f) + (humanOnTeam ? 0.3f : 0.0f);
		bot.teamPlayer = random() < keenness * character( bot ).teamwork;
		bot.leader = -1;
		bot.leaderTime = 0;
		bot.groundTime = 0;
		bot.probeBlocked = false;
		bot.brakeTime = 0;
		bot.path.clear();
		bot.pathTime = 0;
		bot.following = false;
		bot.cornerAhead = false;
		bot.takeoffTime = 0;
		bot.taxiTime = 0;
		bot.holdTime = 0;
		bot.takeoffHolds = 0;
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

	// someone has our flag: go where they were last seen, or to their base where they are taking it
	if( g_gametype.integer == GT_CTF && bot.desire != Goal::Rearm && !(ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) )
	{
		const int own = client->sess_.sessionTeam_;
		const int ownFlag = own == ClientBase::TEAM_RED ? OB_REDFLAG : OB_BLUEFLAG;
		for( int i = 1; i <= theLevel.maxclients_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( other == ent || !alive( other ) || !(other->client_->ps_.objectives & ownFlag) )
				continue;
			Vec3 intercept;
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

	// capturing: to their flag, and with it back to ours
	if( bot.desire == Goal::Capture )
	{
		const bool carrying = (ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0;
		const int own = client->sess_.sessionTeam_;
		const int other = own == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED;
		Vec3 flag;
		if( flagPosition( carrying ? own : other, flag ) )
		{
			VectorCopy( flag, bot.goal );
			bot.goalTime = theLevel.time_ + 3000;
		}
	}

	// Fuel (Marek, 2026-10-08): a plane with no pad it can land on on this map keeps a quarter tank, since it
	// cannot refuel there and no plane can pick up a supply drop. Nothing else gets free fuel: a plane that can
	// land does, ground vehicles and helicopters go back to a pad or are supplied by helicopter, boats are supplied.
	constexpr int padCheckInterval = 2000;
	if( availableVehicles[client->vehicle_].cat & CAT_PLANE )
	{
		if( theLevel.time_ >= bot.padCheckTime )
		{
			bot.padCheckTime = theLevel.time_ + padCheckInterval;
			bot.canLand = g_botLanding.integer && theLevel.time_ >= bot.landGiveUp &&
				findPad( ent, theLevel.time_ < bot.badPadTime ? bot.badPad : -1 ) >= 0;
		}
		if( !bot.canLand && bot.pad < 0 && ps.stats[STAT_FUEL] < ps.stats[STAT_MAX_FUEL] / 4 )
			ps.stats[STAT_FUEL] = ps.stats[STAT_MAX_FUEL] / 4;
	}

	if( client->sess_.ridingClient_ >= 0 )
	{
		thinkRider( ent, bot, cmd, view );
	}
	else
	{
		if( theLevel.time_ >= bot.leaderTime )
			chooseLeader( ent, bot );
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

	// less skilled characters hold the aim less steadily; not while landing
	const float wobble = bot.desire == Goal::Rearm ? 0.0f : (1.0f - skill( bot )) * 3.0f;
	bot.aimPhase += 0.11f;
	view[YAW] += sin( bot.aimPhase ) * wobble;
	view[PITCH] += cos( bot.aimPhase * 0.7f ) * wobble * 0.6f;

	for( int i = 0; i < 3; i++ )
		cmd.angles[i] = ANGLE2SHORT( view[i] ) - ps.delta_angles[i];

#ifdef MF_BOT_DEBUG
	if( g_botDebug.integer >= 3 && theLevel.time_ >= bot.logTime )
		logState( ent, bot, cmd, view );
#endif
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
		cmd = {};
		cmd.serverTime = theLevel.time_;
		think( ent, bot, cmd );
		GameHost::botUserCommand( i, &cmd );
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
