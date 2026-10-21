#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "local.h"
#include "../team/team.h"
#include "../navigation/airnav.h"
#include "../navigation/groundnav.h"
#include "../world/threat.h"
#include "../world/hints.h"
#include "../team/orders.h"
#include "telemetry.h"
#include <algorithm>
#include <cmath>

std::array<Bots::State, MAX_CLIENTS + 1> Bots::states_;
int Bots::fillTime_ = 0;
std::vector<BotWorld::Pad> BotWorld::pads_;
bool BotWorld::padsBuilt_ = false;
std::array<int, 5> Bots::kindLives_ = {};
std::array<int, 5> Bots::kindCrashes_ = {};
std::array<int, MAX_VEHICLE_TYPES> Bots::typeLives_ = {};
std::array<int, MAX_VEHICLE_TYPES> Bots::typeCrashes_ = {};
std::vector<Vec3> BotWorld::roamPoints_;
std::vector<Vec3> BotWorld::landPoints_;
std::vector<Vec3> BotWorld::waterPoints_;
std::vector<Bots::Character> Bots::characters_;

void Bots::reset()
{
	states_.fill( State{} );
	fillTime_ = 0;
	BotWorld::pads_.clear();
	BotWorld::padsBuilt_ = false;
	BotWorld::roamPoints_.clear();
	BotWorld::landPoints_.clear();
	BotWorld::waterPoints_.clear();
	ThreatMap::reset();
	BotHints::reset();
	BotOrders::reset();
	BotTelemetry::reset();
	characters_.clear();
	kindLives_.fill( 0 );
	kindCrashes_.fill( 0 );
	typeLives_.fill( 0 );
	typeCrashes_.fill( 0 );
	AirNav::reset();
	GroundNav::reset();
	BotTeam::reset();
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
// A shot is heard by the bots near it that are not on the shooter's side. It tells roughly where the shooter is: the
// contact becomes suspicious, drawing the bot's look, not its guns
void Bots::onShot( GameEntity* shooter )
{
	constexpr float hearing = 2000.0f, vagueness = 300.0f;
	if( !shooter || !shooter->client_ || shooter->s.number < 1 || shooter->s.number > MAX_CLIENTS )
		return;
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		GameEntity* listener = theLevel.getEntity( i );
		if( listener == shooter || !states_[i].active || !alive( listener ) || OnSameTeam( listener, shooter ) )
			continue;
		if( Distance( listener->r.currentOrigin, shooter->r.currentOrigin ) > hearing )
			continue;
		Seen& seen = states_[i].seen[shooter->s.number];
		if( seen.level == Awareness::Aware )
			continue;
		VectorCopy( shooter->r.currentOrigin, seen.position );
		seen.position[0] += crandom() * vagueness;
		seen.position[1] += crandom() * vagueness;
		seen.time = theLevel.time_;
		seen.level = Awareness::Suspicious;
	}
}

void Bots::onDeath( GameEntity* self, GameEntity* attacker, int meansOfDeath )
{
	BotTeam::onDeath( self, attacker );
	if( self && self->client_ )
		ThreatMap::noteDeath( self->client_->sess_.sessionTeam_, self->r.currentOrigin );
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
			BotWeapons::carriesSupplies( other->client_->vehicle_ ) )
			return true;
	}
	return false;
}

// a weighted draw among the vehicles this team may take on this map
// (teamKind: only vehicles of the kind the team asked for, see BotTeam)
int Bots::pickVehicle( int teamFlag, bool allowInfantry, const Character* who, const State* memory, int teamKind )
{
	Choice choice;
	choice.cats = allowedCategories();
	choice.teamFlag = teamFlag;
	choice.allowInfantry = allowInfantry;
	choice.who = who;
	choice.memory = memory;
	choice.teamKind = teamKind;
	// a team with nobody flying supplies: team-minded characters consider taking the supply helicopter
	choice.teamHasSupplier = g_gametype.integer < GT_TEAM || Bots::teamHasSupplier( teamFlag == MF_TEAM_1 ? ClientBase::TEAM_RED : ClientBase::TEAM_BLUE );
	std::array<int, 256> candidates;
	std::array<int, 256> weights;
	int num = 0, total = 0;
	for( int i = 0; i < bg_numberOfVehicles && num < 256; i++ )
	{
		const int weight = vehicleWeight( i, choice );
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

// the kinds of vehicle bots may spawn in here, narrowed by g_botVehicles when that leaves any
unsigned int Bots::allowedCategories()
{
	unsigned int cats = spawnCategories();
	if( !g_botVehicles.string[0] )
		return cats;
	unsigned int wanted = 0;
	if( strstr( g_botVehicles.string, "plane" ) ) wanted |= CAT_PLANE;
	if( strstr( g_botVehicles.string, "helo" ) ) wanted |= CAT_HELO;
	if( strstr( g_botVehicles.string, "ground" ) ) wanted |= CAT_GROUND;
	if( strstr( g_botVehicles.string, "boat" ) ) wanted |= CAT_BOAT;
	if( strstr( g_botVehicles.string, "infantry" ) ) wanted |= CAT_LQM;
	return (cats & wanted) ? cats & wanted : cats;
}

// how much a bot would like this vehicle, 0 when it is not to be taken at all
int Bots::vehicleWeight( int i, const Choice& choice )
{
	const completeVehicleData_t& veh = availableVehicles[i];
	if( !(veh.gameset & Game::getGameset()) || !(veh.team & choice.teamFlag) || !(veh.cat & choice.cats) || !VehicleData::selectable( i ) )
		return 0;
	if( choice.teamKind >= 0 && static_cast<int>( BotTeam::kindOf( i ) ) != choice.teamKind )
		return 0;
	// bots do not take nuclear weapons; supplies are for team mates, so nobody flies the supply helicopter in a
	// free-for-all; an aircraft that has flown into the map nearly every time it was tried does not fit here (a
	// wide-turning bomber in a canyon), and nobody takes it again this map
	if( carriesNuke( i ) || (g_gametype.integer < GT_TEAM && BotWeapons::carriesSupplies( i )) || unfitHere( i ) )
		return 0;
	// base weight per kind evens out how many entries each kind has in the vehicle table
	int weight = 0;
	const int kind = kindOf( i );
	if( veh.cat & CAT_PLANE ) weight = 4;
	else if( veh.cat & CAT_HELO ) weight = 8;
	else if( veh.cat & CAT_GROUND ) weight = 6;
	else if( veh.cat & CAT_BOAT ) weight = 12;
	else if( (veh.cat & CAT_LQM) && choice.allowInfantry ) weight = 8;
	const Character* who = choice.who;
	// the character's own taste
	if( who )
		weight = static_cast<int>( weight * 10 * std::max( choice.teamKind >= 0 ? 0.1f : 0.0f, who->prefer[kind] ) );
	if( who && !choice.teamHasSupplier && BotWeapons::carriesSupplies( i ) )
		weight = static_cast<int>( weight * (2.0f + 8.0f * who->teamwork) );
	// bombers have little to do on a map with nothing on the ground to bomb
	if( !(choice.cats & (CAT_GROUND|CAT_BOAT)) && (veh.cat & CAT_PLANE) && (veh.cls & CLASS_PLANE_BOMBER) )
		weight = weight * 12 / 100;
	// more than half of all bots' lives in this kind ended in an early crash here: the map does not suit it
	if( kindLives_[kind] >= 12 && kindCrashes_[kind] * 10 > kindLives_[kind] * 5 )
		weight = std::max( 1, weight / 3 );
	// and this bot's own record in it
	if( choice.memory && who && choice.memory->ownLives[kind] > 0 )
	{
		const float failureRate = static_cast<float>( choice.memory->ownFailures[kind] ) / choice.memory->ownLives[kind];
		weight = std::max( 1, static_cast<int>( weight / (1.0f + failureRate * choice.memory->ownLives[kind] * failureSensitivity( *who )) ) );
	}
	return weight;
}

bool Bots::carriesNuke( int vehicle )
{
	for( unsigned int weapon : availableVehicles[vehicle].weapons )
	{
		if( availableWeapons[weapon].type == WT_NUKEBOMB || availableWeapons[weapon].type == WT_NUKEMISSILE )
			return true;
	}
	return false;
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
		char team[8];
		Q_strncpyz( team, g_gametype.integer >= GT_TEAM ? BotTeam::sideToJoin( bot ) : "auto", sizeof( team ) );
		SetTeam( ent, team );
		return;
	}

	int teamFlag = MF_TEAM_ANY;
	if( g_gametype.integer >= GT_TEAM )
		teamFlag = client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;

	if( bot.vehicle < 0 || bot.vehicle >= bg_numberOfVehicles || !(availableVehicles[bot.vehicle].team & teamFlag) )
	{
		// the team's make-up comes before the bot's own taste
		BotTeam::Kind kind;
		bot.vehicle = -1;
		if( BotTeam::wantedKind( ent, kind ) )
			bot.vehicle = pickVehicle( teamFlag, false, &character( bot ), &bot, static_cast<int>( kind ) );
		if( bot.vehicle < 0 )
			bot.vehicle = pickVehicle( teamFlag, !BotTeam::level(), &character( bot ), &bot );
	}
	if( bot.vehicle < 0 )
		return;

	char userinfo[MAX_INFO_STRING];
	GameHost::getUserinfo( clientNum, userinfo, sizeof( userinfo ) );
	Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", bot.vehicle ) );
	GameHost::setUserinfo( clientNum, userinfo );
	ClientUserinfoChanged( clientNum );
}

// One bot's frame: settle a life that has ended, choose a vehicle and spawn, start a new life, then the wishes that
// come before the pilot (a flag thief, the team's plan, fuel), the pilot, and the aim as steadily as skill allows
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
		if( settleLife( ent, bot ) )
			return;
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
	if( !bot.wasAlive )
		startLife( ent, bot );

	cmd.weapon = ps.weaponNum;
	chaseFlagThief( ent, bot );
	followPlan( ent, bot );
	keepPlaneFuel( ent, bot );
	pilot( ent, bot, cmd, view );

	turnHand( bot, view );

	// less skilled characters hold the aim less steadily; not while landing
	const float wobble = bot.desire == Goal::Rearm ? 0.0f : handicap( bot ).aimWobble;
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

// A hand on a mouse: the view turns fast but not in no time, so a change of mind never flicks it across in one frame.
// The more skilled turn it faster
void Bots::turnHand( State& bot, vec3_t view )
{
	constexpr float slowestTurn = 360.0f, skilledGain = 360.0f;
	if( bot.handTime && theLevel.time_ > bot.handTime )
	{
		const float most = (slowestTurn + skilledGain * skill( bot )) * (theLevel.time_ - bot.handTime) * 0.001f;
		for( int i : { PITCH, YAW } )
			view[i] = bot.handView[i] + std::clamp( AngleSubtract( view[i], bot.handView[i] ), -most, most );
	}
	if( !bot.handTime || theLevel.time_ > bot.handTime )
	{
		VectorCopy( view, bot.handView );
		bot.handTime = theLevel.time_;
	}
}

// A life has ended: its record, and whether the next life is in another vehicle. True when the bot has just been
// sent to a new vehicle
bool Bots::settleLife( GameEntity* ent, State& bot )
{
	GameClient* client = ent->client_;
	const int score = client->ps_.persistant[PERS_SCORE];
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
	const bool supplier = BotWeapons::carriesSupplies( flown );
	if( supplier && failed && theLevel.time_ - bot.spawnTime > longLife && random() < 0.6f * (1.0f - character( bot ).patience) )
		reconsider = true;
	// every death is a chance to notice that the team has nobody flying supplies
	const bool teamGame = g_gametype.integer >= GT_TEAM;
	const bool fillSupply = teamGame && !supplier && !reconsider && !teamHasSupplier( client->sess_.sessionTeam_, ent ) &&
		random() < 0.35f * character( bot ).teamwork;
	// the team's make-up: the bot that respawns next takes the kind the team is shortest of
	BotTeam::Kind teamKind;
	const bool refit = BotTeam::wantedKind( ent, teamKind ) && teamKind != BotTeam::kindOf( flown );
	if( !reconsider && !fillSupply && !refit )
		return false;
	int teamFlag = MF_TEAM_ANY;
	if( teamGame )
		teamFlag = client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;
	int vehicle = refit ? pickVehicle( teamFlag, false, &character( bot ), &bot, static_cast<int>( teamKind ) ) : -1;
	if( vehicle < 0 && (reconsider || fillSupply) )
		vehicle = pickVehicle( teamFlag, false, &character( bot ), &bot );
	if( fillSupply && vehicle >= 0 && !BotWeapons::carriesSupplies( vehicle ) )
		vehicle = -1;
	if( vehicle < 0 )
		return false;
	bot.vehicle = vehicle;
	bot.badLives = 0;
	char userinfo[MAX_INFO_STRING];
	GameHost::getUserinfo( ent->s.number, userinfo, sizeof( userinfo ) );
	Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", vehicle ) );
	GameHost::setUserinfo( ent->s.number, userinfo );
	bot.wasAlive = false;
	ClientUserinfoChanged( ent->s.number );
	return true;
}

// The first frame of a new life: what each behaviour kept belongs to the last life and its vehicle. Some lives are
// played as a team player, more often by team-minded characters and more readily with a real player on the team;
// tanks and helicopters hunt in packs for safety, about two lives in five, planes about one in five
void Bots::startLife( GameEntity* ent, State& bot )
{
	bot.wasAlive = true;
	bot.spawnTime = theLevel.time_;
	bot.handTime = 0;
	// an offer of a lift belongs to the life that made it; whom it passed over is remembered
	bot.lift.walker = -1;
	bot.stillSince = 0;
	bot.lifeScore = ent->client_->ps_.persistant[PERS_SCORE];
	bot.goalTime = 0;
	bot.enemy = -1;
	bot.enemyTime = 0;
	bot.pursuit.reset();
	bot.avoid.reset();
	bot.stuck.reset();
	bot.stuck.checkTime = theLevel.time_ + 3000;
	VectorCopy( ent->r.currentOrigin, bot.stuck.lastOrigin );
	bot.path.reset();
	bot.cover.reset();
	bot.takeoff.reset();
	bot.rearm.letGo();
	bot.desire = Goal::Roam;
	bot.commitTime = 0;
	bot.tree = behavior::Memory{};
	bot.hitTime = 0;
	bot.lastHealth = ent->health_;
	bool humanOnTeam = false;
	for( int i = 1; i <= theLevel.maxclients_ && g_gametype.integer >= GT_TEAM; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other != ent && other && other->inuse_ && other->client_ && !isBot( i ) && OnSameTeam( ent, other ) )
			humanOnTeam = true;
	}
	const Capabilities can = Capabilities::of( ent->client_->vehicle_ );
	const bool packHunter = can.armoured || can.hovers;
	const float keenness = (packHunter ? 0.68f : 0.35f) + (humanOnTeam ? 0.3f : 0.0f);
	bot.teamPlayer = random() < keenness * character( bot ).teamwork;
	bot.leader = -1;
	bot.leaderTime = 0;
	if( !bot.hasHome )
	{
		VectorCopy( ent->r.currentOrigin, bot.home );
		bot.hasHome = true;
	}
}

// someone has our flag: go where they were last seen, or to their base where they are taking it
void Bots::chaseFlagThief( GameEntity* ent, State& bot )
{
	const playerState_t& ps = ent->client_->ps_;
	if( g_gametype.integer != GT_CTF || bot.desire == Goal::Rearm || (ps.objectives & (OB_REDFLAG|OB_BLUEFLAG)) )
		return;
	const int own = ent->client_->sess_.sessionTeam_;
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
		else if( !BotWorld::flagPosition( own == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED, intercept ) )
			return;
		VectorCopy( intercept, bot.goal );
		bot.goalTime = theLevel.time_ + 3000;
		if( bot.desire == Goal::Capture )
			bot.desire = Goal::Roam;
		return;
	}
}

// The team's plan, or a player's order: where this bot is asked to be. It goes there when it has nothing more
// pressing, which a fight, an empty tank and a missile all are. Capturing: to their flag, and with it back to ours
void Bots::followPlan( GameEntity* ent, State& bot )
{
	const BotTeam::Assignment& job = BotTeam::of( ent->s.number );
	const bool ordered = BotOrders::assignmentOf( ent->s.number ) != nullptr;
	const bool obeys = job.job != BotTeam::Job::None && (ordered || character( bot ).teamwork >= (BotTeam::level() >= 2 ? 0.15f : 0.3f));
	if( obeys && job.hasPlace && (bot.desire == Goal::Roam || bot.desire == Goal::Capture) )
	{
		VectorCopy( job.place, bot.goal );
		bot.goalTime = theLevel.time_ + 3000;
		return;
	}
	if( bot.desire != Goal::Capture )
		return;
	const bool carrying = (ent->client_->ps_.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0;
	const int own = ent->client_->sess_.sessionTeam_;
	const int other = own == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED;
	Vec3 flag;
	if( BotWorld::flagPosition( carrying ? own : other, flag ) )
	{
		VectorCopy( flag, bot.goal );
		bot.goalTime = theLevel.time_ + 3000;
	}
}

// Fuel (Marek, 2026-10-08): a plane with no pad it can land on on this map keeps a quarter tank, since it cannot
// refuel there and no plane can pick up a supply drop. Nothing else gets free fuel: a plane that can land does,
// ground vehicles and helicopters go back to a pad or are supplied by helicopter, boats are supplied.
void Bots::keepPlaneFuel( GameEntity* ent, State& bot )
{
	constexpr int padCheckInterval = 2000;
	playerState_t& ps = ent->client_->ps_;
	if( !(availableVehicles[ent->client_->vehicle_].cat & CAT_PLANE) )
		return;
	if( theLevel.time_ >= bot.landing.padCheckTime )
	{
		bot.landing.padCheckTime = theLevel.time_ + padCheckInterval;
		bot.landing.canLand = g_botLanding.integer && theLevel.time_ >= bot.landing.giveUp &&
			BotWorld::findPad( ent, theLevel.time_ < bot.rearm.badPadTime ? bot.rearm.badPad : -1 ) >= 0;
	}
	if( !bot.landing.canLand && bot.rearm.pad < 0 && ps.stats[STAT_FUEL] < ps.stats[STAT_MAX_FUEL] / 4 )
		ps.stats[STAT_FUEL] = ps.stats[STAT_MAX_FUEL] / 4;
}

// a rider works its seat; otherwise it senses, and the pilot or driver for its kind of vehicle flies
void Bots::pilot( GameEntity* ent, State& bot, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	if( client->sess_.ridingClient_ >= 0 )
	{
		thinkRider( ent, bot, cmd, view );
		return;
	}
	if( theLevel.time_ >= bot.leaderTime )
		BotSenses::chooseLeader( ent, bot );
	Sense s;
	BotSenses::sense( ent, bot, s );
	BotOrders::report( ent, bot, s );
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

void Bots::runFrame()
{
	if( g_botMinPlayers.integer > 0 )
		fill();
	BotTeam::runFrame();

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
