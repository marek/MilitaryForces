#include "bot_mind.h"

using behavior::Status;

struct InfantryTasks
{
	using Mind = Bots::Mind;

	static void findRide( Mind& c )
	{
		for( int i = 0; i < theLevel.num_entities_; i++ )
		{
			GameEntity* other = theLevel.getEntity( i );
			if( !other || !other->inuse_ || other == c.ent || other->health_ <= 0 )
				continue;
			bool candidate = false;
			if( other->s.eType == ET_MISC_VEHICLE && (other->s.seatMask & SEATMASK_ENTERABLE) )
				candidate = true;
			else if( other->client_ && alive( other ) && i <= theLevel.maxclients_ && other->client_->sess_.ridingClient_ < 0 &&
				!(availableVehicles[other->client_->vehicle_].cat & (CAT_LQM|CAT_PLANE)) &&
				g_gametype.integer >= GT_TEAM && OnSameTeam( c.ent, other ) &&
				VectorLength( other->client_->ps_.velocity ) < 40 )
				candidate = true;
			if( !candidate )
				continue;
			float dist = Distance( c.ent->r.currentOrigin, other->r.currentOrigin );
			if( dist > 4000 )
				continue;
			if( other->client_ && VehicleData::vehicleHasTurret( other->client_->vehicle_ ) && !(other->s.seatMask & (1 << 1)) )
				dist *= 0.4f;
			if( !c.target || dist < c.targetDist )
			{
				c.target = other;
				c.targetDist = dist;
			}
		}
	}

	static void findPad( Mind& c )
	{
		if( !Bots::padsBuilt_ )
			Bots::buildPads();
		for( size_t i = 0; i < Bots::pads_.size(); i++ )
		{
			const float dist = Distance( c.ent->r.currentOrigin, Bots::pads_[i].spot );
			if( c.nearPad < 0 || dist < c.nearPadDist )
			{
				c.nearPad = static_cast<int>( i );
				c.nearPadDist = dist;
			}
		}
	}

	static bool rideInReach( Mind& c )
	{
		return c.target && Distance( c.ent->r.currentOrigin, c.target->r.currentOrigin ) < g_enterRange.value * 0.9f && theLevel.time_ > c.bot.commandTime;
	}
	static bool enemyClose( Mind& c ) { return c.sense.enemy && c.sense.enemyDist < 900 && !c.sense.enemyAir; }
	static bool seesRide( Mind& c ) { return c.target != nullptr; }
	static bool stranded( Mind& c )
	{
		const int patience = 20000 + static_cast<int>( 30000 * c.who().patience );
		return theLevel.time_ > c.bot.spawnTime + patience || (c.nearPad < 0 && theLevel.time_ > c.bot.spawnTime + 8000) || c.nearPadDist > 6000;
	}
	static bool padAway( Mind& c ) { return c.nearPad >= 0 && c.nearPadDist > 150; }
	static bool moving( Mind& c ) { return !c.direct; }

	static Status board( Mind& c )
	{
		c.bot.commandTime = theLevel.time_ + 1500;
		c.direct = true;
		Cmd_Eject_f( c.ent );
		return Status::Success;
	}

	static Status shoot( Mind& c )
	{
		c.direct = true;
		c.view[YAW] = c.sense.enemyAngles[YAW];
		c.view[PITCH] = std::clamp( AngleSubtract( c.sense.enemyAngles[PITCH], 0 ), -60.0f, 60.0f );
		if( c.client->ps_.ammo[WP_MACHINEGUN] > 0 )
			c.cmd.buttons |= BUTTON_ATTACK;
		return Status::Running;
	}

	static Status walkToRide( Mind& c )
	{
		c.courseTo( c.target->r.currentOrigin );
		return Status::Running;
	}

	static Status respawnInVehicle( Mind& c )
	{
		if( theLevel.time_ <= c.bot.commandTime )
			return Status::Failure;
		int teamFlag = MF_TEAM_ANY;
		if( g_gametype.integer >= GT_TEAM )
			teamFlag = c.client->sess_.sessionTeam_ == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;
		const int vehicle = Bots::pickVehicle( teamFlag, false, &c.who(), &c.bot );
		if( vehicle < 0 )
			return Status::Failure;
		c.bot.commandTime = theLevel.time_ + 2000;
		c.bot.vehicle = vehicle;
		c.direct = true;
		char userinfo[MAX_INFO_STRING];
		SV_GetUserinfo( c.ent->s.number, userinfo, sizeof( userinfo ) );
		Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", vehicle ) );
		SV_SetUserinfo( c.ent->s.number, userinfo );
		ClientUserinfoChanged( c.ent->s.number );
		Cmd_Kill_f( c.ent );
		return Status::Success;
	}

	static Status walkToPad( Mind& c )
	{
		c.courseTo( Bots::pads_[c.nearPad].spot );
		return Status::Running;
	}

	static Status wander( Mind& c )
	{
		if( theLevel.time_ > c.bot.goalTime || Distance( c.ent->r.currentOrigin, c.bot.goal ) < 60 )
			Bots::pickGoal( c.ent, c.bot, 600 );
		c.courseTo( c.bot.goal );
		return Status::Running;
	}

	static Status navigate( Mind& c )
	{
		c.followGround( false, 120.0f );
		return Status::Success;
	}

	static Status walk( Mind& c )
	{
		Bots::State& bot = c.bot;
		vec3_t angles;
		c.course[2] = 0;
		vectoangles( c.course, angles );
		c.view[YAW] = angles[YAW];
		c.view[PITCH] = 0;
		c.cmd.forwardmove = moveFull;
		if( theLevel.time_ > bot.stuckCheckTime )
		{
			if( Distance( c.ent->r.currentOrigin, bot.lastOrigin ) < 8 && theLevel.time_ > bot.spawnTime + 3000 )
			{
				bot.reverseTime = theLevel.time_ + 1200;
				bot.reverseTurn = (rand() & 1) ? moveFull : -moveFull;
				bot.pathTime = 0;
			}
			VectorCopy( c.ent->r.currentOrigin, bot.lastOrigin );
			bot.stuckCheckTime = theLevel.time_ + 2000;
		}
		if( theLevel.time_ < bot.reverseTime )
			c.cmd.rightmove = static_cast<signed char>( bot.reverseTurn );
		return Status::Success;
	}

	static behavior::Tree<Mind> build()
	{
		behavior::Tree<Mind> t;

		const auto* act = t.selector( "soldier", {
			t.when( "vehicle in reach", rideInReach, t.action( "board", board ) ),
			t.when( "enemy close", enemyClose, t.action( "shoot", shoot ) ),
			t.when( "sees a vehicle with a seat", seesRide, t.action( "walk to vehicle", walkToRide ) ),
			t.when( "stranded", stranded, t.action( "respawn in a vehicle", respawnInVehicle ) ),
			t.when( "pad within walking distance", padAway, t.action( "walk to pad", walkToPad ) ),
			t.action( "wander", wander ) } );

		t.setRoot( t.always( "infantry", act, {
			t.when( "moving", moving, t.always( "move", t.action( "navigate", navigate ), { t.action( "walk", walk ) } ) ) } ) );
		return t;
	}
};

void Bots::thinkInfantry( GameEntity* ent, State& bot, const Sense& sense, usercmd_t& cmd, vec3_t view )
{
	static const behavior::Tree<Mind> tree = InfantryTasks::build();

	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	Mind mind{ ent, client, bot, sense, cmd, view, ps, availableVehicles[client->vehicle_], static_cast<float>( ps.speed ) / 10.0f };
	InfantryTasks::findRide( mind );
	if( !mind.target )
		InfantryTasks::findPad( mind );
	view[YAW] = ps.viewangles[YAW];
	view[PITCH] = 0;

	runTree( tree, mind, "infantry" );
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
