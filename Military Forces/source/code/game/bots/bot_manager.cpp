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
std::array<int, 5> Bots::kindLives_ = {};
std::array<int, 5> Bots::kindCrashes_ = {};
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
}

bool Bots::isBot( int clientNum )
{
	return clientNum >= 1 && clientNum <= MAX_CLIENTS && states_[clientNum].active;
}

int Bots::count()
{
	return static_cast<int>( std::count_if( states_.begin(), states_.end(), []( const State& s ) { return s.active; } ) );
}

int Bots::kindOf( int vehicle )
{
	const unsigned int cat = availableVehicles[vehicle].cat;
	if( cat & CAT_PLANE ) return 0;
	if( cat & CAT_HELO ) return 1;
	if( cat & CAT_GROUND ) return 2;
	if( cat & CAT_BOAT ) return 3;
	return 4;
}

void Bots::onDeath( GameEntity* self, GameEntity* attacker, int meansOfDeath )
{
	if( !self || !self->client_ || !isBot( self->s.number ) || self->client_->vehicle_ < 0 )
		return;

	const State& bot = states_[self->s.number];
	const int kind = kindOf( self->client_->vehicle_ );
	kindLives_[kind]++;
	if( (meansOfDeath == MOD_CRASH || meansOfDeath == MOD_WATER) && theLevel.time_ - bot.spawnTime < 20000 )
		kindCrashes_[kind]++;
	if( !g_botDebug.integer )
		return;

	const playerState_t& ps = self->client_->ps_;
	vec3_t forward, start;
	trace_t ahead;
	AngleVectors( ps.vehicleAngles, forward, nullptr, nullptr );
	VectorMA( self->r.currentOrigin, -100.0f, forward, start );
	pathBlocked( start, forward, 600, self->s.number, ahead );
	GameEntity* hit = ahead.fraction < 1.0f && ahead.entityNum > ENTITYNUM_WORLD ? theLevel.getEntity( ahead.entityNum ) : nullptr;
	GameEntity* enemy = bot.enemy >= 0 ? theLevel.getEntity( bot.enemy ) : nullptr;
	float nearest = -1;
	int nearestNum = -1;
	for( int i = 1; i <= theLevel.maxclients_; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( other == self || !other || !other->inuse_ || !other->client_ || other->client_->vehicle_ < 0 )
			continue;
		const float dist = Distance( self->r.currentOrigin, other->r.currentOrigin );
		if( nearest < 0 || dist < nearest )
		{
			nearest = dist;
			nearestNum = i;
		}
	}
	vec3_t below;
	VectorCopy( self->r.currentOrigin, below );
	below[2] -= heightAboveGround( self->r.currentOrigin, self->s.number ) + 4;

	Common::printf( "BOTDEATH t %d bot %d mod %d by %d veh %d role %d goal %d age %d health %d org %.0f %.0f %.0f vel %.0f %.0f %.0f speed %d pitch %.0f roll %.0f yaw %.0f "
		"height %.0f ceiling %.0f water %d onoff %x throttle %d fuel %d enemy %d edist %.0f avoid %d egress %d land %d pad %d ahead %.0f aheadent %d aheadclass %s normalz %.2f "
		"nearest %d ndist %.0f trail",
		theLevel.time_, self->s.number, meansOfDeath, attacker ? attacker->s.number : -1, self->client_->vehicle_, static_cast<int>( roleOf( self->client_->vehicle_ ) ),
		static_cast<int>( bot.desire ), theLevel.time_ - bot.spawnTime, self->health_, self->r.currentOrigin[0], self->r.currentOrigin[1], self->r.currentOrigin[2],
		ps.velocity[0], ps.velocity[1], ps.velocity[2], ps.speed, ps.vehicleAngles[PITCH], ps.vehicleAngles[ROLL], ps.vehicleAngles[YAW],
		heightAboveGround( self->r.currentOrigin, self->s.number ), ceilingAbove( self->r.currentOrigin, self->s.number ),
		(SV_PointContents( below, self->s.number ) & MASK_WATER) != 0, ps.ONOFF, ps.fixed_throttle, ps.stats[STAT_FUEL],
		bot.enemy, enemy ? Distance( enemy->r.currentOrigin, self->r.currentOrigin ) : -1.0f, theLevel.time_ < bot.avoidTime, theLevel.time_ < bot.egressTime,
		bot.landPhase, bot.pad, ahead.fraction * 600.0f - 100.0f, ahead.fraction < 1.0f ? ahead.entityNum : -1, hit && hit->classname_ ? hit->classname_ : "-",
		ahead.plane.normal[2], nearestNum, nearest );
	for( int i = 0; i < 6; i++ )
		Common::printf( " %.0f,%.0f,%.0f", bot.trail[i][0], bot.trail[i][1], bot.trail[i][2] );
	Common::printf( "\n" );
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
		if( !groundTargets && (veh.cat & CAT_PLANE) && (veh.cls & CLASS_PLANE_BOMBER) )
			weight = weight * 12 / 100;
		if( kindLives_[kind] >= 12 && kindCrashes_[kind] * 10 > kindLives_[kind] * 5 )
			weight = std::max( 1, weight / 3 );

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

	const float wobble = bot.desire == Goal::Rearm ? 0.0f : (1.0f - skill( bot )) * 3.0f;
	bot.aimPhase += 0.11f;
	view[YAW] += sin( bot.aimPhase ) * wobble;
	view[PITCH] += cos( bot.aimPhase * 0.7f ) * wobble * 0.6f;

	for( int i = 0; i < 3; i++ )
		cmd.angles[i] = ANGLE2SHORT( view[i] ) - ps.delta_angles[i];

	if( g_botDebug.integer >= 3 && theLevel.time_ >= bot.logTime )
	{
		bot.logTime = theLevel.time_ + 500;
		vec3_t flat = { 0, ps.vehicleAngles[YAW], 0 }, forward, right, up = { 0, 0, 1 }, back, left, down = { 0, 0, -1 }, nose;
		AngleVectors( flat, forward, right, nullptr );
		AngleVectors( ps.vehicleAngles, nose, nullptr, nullptr );
		VectorNegate( forward, back );
		VectorNegate( right, left );
		const float* dirs[7] = { nose, forward, back, left, right, up, down };
		float reach[7];
		for( int i = 0; i < 7; i++ )
		{
			trace_t tr;
			pathBlocked( ent->r.currentOrigin, dirs[i], 4000, ent->s.number, tr );
			reach[i] = tr.fraction * 4000.0f;
		}
		Common::printf( "BOTSTATE t %d bot %d veh %d goal %d org %.0f %.0f %.0f vel %.0f %.0f %.0f speed %d ang %.0f %.0f %.0f view %.0f %.0f move %d %d buttons %x "
			"onoff %x throttle %d health %d enemy %d clear nose %.0f fwd %.0f back %.0f left %.0f right %.0f up %.0f down %.0f avoid %d egress %d land %d\n",
			theLevel.time_, ent->s.number, client->vehicle_, static_cast<int>( bot.desire ), ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
			ps.velocity[0], ps.velocity[1], ps.velocity[2], ps.speed, ps.vehicleAngles[PITCH], ps.vehicleAngles[YAW], ps.vehicleAngles[ROLL], view[PITCH], view[YAW],
			cmd.forwardmove, cmd.rightmove, cmd.buttons, ps.ONOFF, ps.fixed_throttle, ent->health_, bot.enemy,
			reach[0], reach[1], reach[2], reach[3], reach[4], reach[5], reach[6], theLevel.time_ < bot.avoidTime, theLevel.time_ < bot.egressTime, bot.landPhase );
	}
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
	Common::printf( "BOTTIME %d\n", theLevel.time_ );
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
