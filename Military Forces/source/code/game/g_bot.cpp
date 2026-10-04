#include "g_local.h"
#include "g_level.h"
#include "g_bot.h"
#include <algorithm>
#include <cmath>

void SetTeam( GameEntity *ent, char *s );
void Cmd_Radar_f( GameEntity *ent );
void Cmd_Eject_f( GameEntity *ent );
void ClientUserinfoChanged( int clientNum );
void MF_ClientBegin( int clientNum );

std::array<Bots::State, MAX_CLIENTS + 1> Bots::states_;
int Bots::fillTime_ = 0;

namespace
{
	constexpr std::array<const char*, 16> botNames = {
		"Alpha", "Bravo", "Charlie", "Delta", "Echo", "Foxtrot", "Golf", "Hotel",
		"India", "Juliet", "Kilo", "Lima", "Mike", "November", "Oscar", "Papa"
	};

	constexpr float planeCruiseHeight = 1400.0f;
	constexpr float planeMinHeight = 450.0f;
	constexpr float heloCruiseHeight = 500.0f;
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

void Bots::reset()
{
	states_.fill( State{} );
	fillTime_ = 0;
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

int Bots::pickVehicle( int teamFlag )
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
		if( veh.cat & CAT_PLANE ) weight = 4;
		else if( veh.cat & CAT_GROUND ) weight = 6;
		else if( veh.cat & CAT_HELO ) weight = 8;
		else if( veh.cat & CAT_LQM ) weight = 20;
		else if( veh.cat & CAT_BOAT ) weight = 12;
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

	char userinfo[MAX_INFO_STRING] = "";
	Info::setValueForKey( userinfo, "name", name && name[0] ? name : va( "%s %d", botNames[clientNum % botNames.size()], clientNum ) );
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
		bot.vehicle = pickVehicle( teamFlag );
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

GameEntity* Bots::findEnemy( GameEntity* ent, State& bot )
{
	GameClient* client = ent->client_;
	const completeVehicleData_t& veh = availableVehicles[client->vehicle_];
	const bool flyer = (veh.cat & (CAT_PLANE|CAT_HELO)) != 0;
	const float maxRange = flyer ? planeEnemyRange : surfaceEnemyRange;
	const float radarRange = veh.radarRange > 0 ? static_cast<float>( veh.radarRange ) : 8000.0f;

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

		const float dist = Distance( ent->r.currentOrigin, other->r.currentOrigin );
		if( dist > maxRange )
			continue;
		if( VehicleData::stealthHidden( oc->vehicle_, oc->ps_.timers[TIMER_REVEAL], theLevel.time_, dist, radarRange ) )
			continue;

		const bool air = (availableVehicles[oc->vehicle_].cat & (CAT_PLANE|CAT_HELO)) != 0 && !(oc->ps_.ONOFF & OO_LANDED);
		float score = dist;
		if( air != flyer )
			score *= flyer ? 1.3f : 2.5f;
		if( (veh.cat & CAT_LQM) && dist > 1500 )
			continue;
		if( i == bot.enemy )
			score *= 0.7f;
		if( !best || score < bestScore )
		{
			best = other;
			bestScore = score;
		}
	}
	return best;
}

void Bots::sense( GameEntity* ent, State& bot, Sense& out )
{
	if( theLevel.time_ >= bot.enemyTime )
	{
		GameEntity* enemy = findEnemy( ent, bot );
		bot.enemy = enemy ? enemy->s.number : -1;
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
		vec3_t target;
		VectorCopy( out.enemy->r.currentOrigin, target );
		const float dist = Distance( ent->r.currentOrigin, target );
		const int weapon = availableVehicles[ent->client_->vehicle_].weapons[0];
		const float shotSpeed = weapon && availableWeapons[weapon].muzzleVelocity > 0 ? static_cast<float>( availableWeapons[weapon].muzzleVelocity ) : 2500.0f;
		VectorMA( target, std::min( dist / shotSpeed, 1.5f ), out.enemy->client_->ps_.velocity, target );
		VectorSubtract( target, ent->r.currentOrigin, out.enemyDir );
		out.enemyDist = VectorNormalize( out.enemyDir );
		vectoangles( out.enemyDir, out.enemyAngles );
		out.enemyAngles[PITCH] = AngleSubtract( out.enemyAngles[PITCH], 0 );
		out.enemyAir = (availableVehicles[out.enemy->client_->vehicle_].cat & (CAT_PLANE|CAT_HELO)) != 0 &&
			!(out.enemy->client_->ps_.ONOFF & OO_LANDED);
	}
	out.height = heightAboveGround( ent->r.currentOrigin, ent->s.number );
}

void Bots::pickGoal( GameEntity* ent, State& bot, float radius )
{
	if( !bot.hasHome )
	{
		VectorCopy( ent->r.currentOrigin, bot.home );
		bot.hasHome = true;
	}
	const float angle = random() * 2.0f * M_PI;
	const float dist = radius * (0.3f + 0.7f * random());
	bot.goal[0] = bot.home[0] + cos( angle ) * dist;
	bot.goal[1] = bot.home[1] + sin( angle ) * dist;
	bot.goal[2] = bot.home[2];
	bot.goalTime = theLevel.time_ + 20000;
}

int Bots::findWeaponSlot( int vehicle, const playerState_t& ps, bool air, bool& guided )
{
	const completeVehicleData_t& veh = availableVehicles[vehicle];
	int fallback = -1;
	guided = false;
	for( int slot = WP_WEAPON1; slot < WP_FLARE; slot++ )
	{
		const int weapon = veh.weapons[slot];
		if( !weapon || ps.ammo[slot] <= 0 )
			continue;
		const int type = availableWeapons[weapon].type;
		if( air && type == WT_ANTIAIRMISSILE )
		{
			guided = true;
			return slot;
		}
		if( !air && type == WT_ANTIGROUNDMISSILE )
		{
			guided = true;
			return slot;
		}
		if( fallback < 0 && (type == WT_BALLISTICGUN || type == WT_ROCKET || (type == WT_MACHINEGUN && availableWeapons[weapon].turret)) )
		{
			if( !air || type == WT_MACHINEGUN )
				fallback = slot;
		}
	}
	return fallback;
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
	const int slot = findWeaponSlot( client->vehicle_, ps, sense.enemyAir, guided );
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

	if( ps.ONOFF & OO_LANDED )
	{
		view[YAW] = ps.vehicleAngles[YAW];
		view[PITCH] = speed > veh.stallspeed * 1.2f ? -15.0f : 0.0f;
		return;
	}

	if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 2000 )
		pickGoal( ent, bot, 9000 );

	vec3_t dir, angles;
	bool attacking = false;
	if( sense.enemy && theLevel.time_ > bot.egressTime )
	{
		VectorCopy( sense.enemyAngles, angles );
		attacking = true;
		if( !sense.enemyAir && sense.enemyDist < 900 )
			bot.egressTime = theLevel.time_ + 5000;
	}
	else
	{
		VectorSubtract( bot.goal, ent->r.currentOrigin, dir );
		dir[2] = 0;
		vectoangles( dir, angles );
		angles[PITCH] = 0;
	}

	view[YAW] = angles[YAW];
	if( attacking && (sense.enemyAir || sense.height > planeMinHeight * 1.5f) )
		view[PITCH] = std::clamp( angles[PITCH], -50.0f, 35.0f );
	else
		view[PITCH] = std::clamp( (sense.height - planeCruiseHeight) / 30.0f, -30.0f, 20.0f );

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
				vec3_t home;
				VectorSubtract( bot.home, ent->r.currentOrigin, home );
				vectoangles( home, angles );
				bot.avoidYaw = angles[YAW];
				bot.avoidTime = theLevel.time_ + 3000;
				pickGoal( ent, bot, 3000 );
			}
			else
				view[PITCH] = tr.plane.normal[2] > 0 ? -45.0f : 20.0f;
		}
	}
	if( theLevel.time_ < bot.avoidTime )
	{
		view[YAW] = bot.avoidYaw;
		if( attacking )
			view[PITCH] = std::clamp( (sense.height - planeCruiseHeight) / 30.0f, -30.0f, 20.0f );
	}
	if( sense.height < planeMinHeight )
		view[PITCH] = -40.0f;

	vec3_t up = { 0, 0, 1 };
	if( pathBlocked( ent->r.currentOrigin, up, 700, ent->s.number, tr ) && sense.height > planeMinHeight * 2 )
		view[PITCH] = std::max( view[PITCH], 15.0f );

	if( speed < veh.stallspeed * 1.3f && sense.height > planeMinHeight * 2 )
		view[PITCH] = std::max( view[PITCH], 5.0f );

	if( attacking )
	{
		vec3_t fwd;
		AngleVectors( ps.vehicleAngles, fwd, nullptr, nullptr );
		const float aimError = acos( std::clamp( DotProduct( fwd, sense.enemyDir ), -1.0f, 1.0f ) ) * 180.0f / M_PI;
		fireWeapons( ent, bot, sense, cmd, aimError );
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
	view[YAW] = angles[YAW];
	view[PITCH] = sense.enemy ? std::clamp( AngleSubtract( angles[PITCH], 0 ), -30.0f, 45.0f ) : 0.0f;

	const float yawError = fabs( AngleSubtract( angles[YAW], ps.vehicleAngles[YAW] ) );

	float wantHeight = heloCruiseHeight;
	vec3_t forward, flat;
	trace_t tr;
	VectorSet( flat, 0, ps.vehicleAngles[YAW], 0 );
	AngleVectors( flat, forward, nullptr, nullptr );
	const bool blocked = pathBlocked( ent->r.currentOrigin, forward, 900, ent->s.number, tr );
	if( blocked )
		wantHeight = sense.height + 400;

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

	if( !(ps.ONOFF & OO_LANDED) && !blocked && sense.height > 150 )
	{
		const float standoff = sense.enemy ? (sense.enemyAir ? 500.0f : 1200.0f) : 0.0f;
		if( yawError < 50 && dist > standoff + 300 )
			cmd.forwardmove = 127;
		else if( sense.enemy && dist < standoff )
			cmd.forwardmove = -127;
	}

	if( sense.enemy && !(ps.ONOFF & OO_LANDED) )
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
		if( theLevel.time_ > bot.goalTime || Distance( ent->r.currentOrigin, bot.goal ) < 60 )
			pickGoal( ent, bot, 600 );
		VectorSubtract( bot.goal, ent->r.currentOrigin, dir );
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
	if( client->sess_.seat_ != 1 || !VehicleData::vehicleHasTurret( pilot->client_->vehicle_ ) )
		return;

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
			if( dist < 3500 && (!enemy || dist < best) )
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
	cmd.buttons |= BUTTON_ATTACK_MAIN;
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
		bot.wasAlive = false;
		if( (theLevel.time_ / 200) & 1 )
			cmd.buttons |= BUTTON_ATTACK;
		return;
	}
	if( !bot.wasAlive )
	{
		bot.wasAlive = true;
		bot.spawnTime = theLevel.time_;
		bot.goalTime = 0;
		bot.enemy = -1;
		bot.enemyTime = 0;
		bot.egressTime = 0;
		bot.avoidTime = 0;
		bot.reverseTime = 0;
		bot.stuckCheckTime = theLevel.time_ + 3000;
		VectorCopy( ent->r.currentOrigin, bot.lastOrigin );
		if( !bot.hasHome )
		{
			VectorCopy( ent->r.currentOrigin, bot.home );
			bot.hasHome = true;
		}
	}

	cmd.weapon = ps.weaponNum;

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

	const float skill = std::clamp( g_botSkill.value, 1.0f, 5.0f );
	bot.aimPhase += 0.11f;
	view[YAW] += sin( bot.aimPhase ) * (5.0f - skill) * 0.6f;
	view[PITCH] += cos( bot.aimPhase * 0.7f ) * (5.0f - skill) * 0.4f;

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
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		if( !states_[i].active )
			continue;
		GameEntity* ent = theLevel.getEntity( i );
		if( !ent || !ent->client_ )
			continue;
		GameClient* cl = ent->client_;
		Common::printf( "BOT %d %s vehicle %d health %d org %.0f %.0f %.0f speed %d enemy %d riding %d score %d height %.0f onoff %x angles %.0f %.0f fuel %d\n", i, cl->pers_.netname_,
			cl->vehicle_, ent->health_, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], cl->ps_.speed,
			states_[i].enemy, cl->sess_.ridingClient_, cl->ps_.persistant[PERS_SCORE], heightAboveGround( ent->r.currentOrigin, i ),
			cl->ps_.ONOFF, cl->ps_.vehicleAngles[PITCH], cl->ps_.vehicleAngles[YAW], cl->ps_.stats[STAT_FUEL] );
	}
}
