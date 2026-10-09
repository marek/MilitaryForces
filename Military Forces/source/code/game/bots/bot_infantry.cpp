#include "bot_mind.h"

using behavior::Status;

// A soldier on foot wants a vehicle: the conditions and steps of the infantry tree, and what a bot does as a rider
struct InfantryTasks
{
	using Mind = Bots::Mind;

	// the nearest thing with a seat: an empty vehicle, or a team mate's stopped vehicle. A free gunner's seat is worth a longer walk
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

	// the nearest pad, where vehicles appear
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

	// close enough to climb in, and not just after the last try
	static bool rideInReach( Mind& c )
	{
		return c.target && Distance( c.ent->r.currentOrigin, c.target->r.currentOrigin ) < g_enterRange.value * 0.9f && theLevel.time_ > c.bot.commandTime;
	}
	static bool enemyClose( Mind& c ) { return c.sense.enemy && c.sense.enemyDist < 900 && !c.sense.enemyAir; }
	static bool seesRide( Mind& c ) { return c.target != nullptr; }
	// on foot too long, or with no pad anywhere near: start again in a vehicle. The patient wait longer
	static bool stranded( Mind& c )
	{
		const int patience = 20000 + static_cast<int>( 30000 * c.who().patience );
		return theLevel.time_ > c.bot.spawnTime + patience || (c.nearPad < 0 && theLevel.time_ > c.bot.spawnTime + 8000) || c.nearPadDist > 6000;
	}
	static bool padAway( Mind& c ) { return c.nearPad >= 0 && c.nearPadDist > 150; }
	static bool moving( Mind& c ) { return !c.direct; }

	// the enter key
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

	// choose a vehicle and give up this life for it
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
		GameHost::getUserinfo( c.ent->s.number, userinfo, sizeof( userinfo ) );
		Info::setValueForKey( userinfo, "cg_nextVehicle", va( "%d", vehicle ) );
		GameHost::setUserinfo( c.ent->s.number, userinfo );
		ClientUserinfoChanged( c.ent->s.number );
		Cmd_Kill_f( c.ent );
		return Status::Success;
	}

	static Status walkToPad( Mind& c )
	{
		c.courseTo( Bots::pads_[c.nearPad].spot );
		return Status::Running;
	}

	// nothing to walk to: short walks around
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

	// face the course and walk; when not getting anywhere, sidestep for a moment and plan the path again
	static Status walk( Mind& c )
	{
		Bots::State& bot = c.bot;
		Vec3 angles;
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

// A bot riding in someone's vehicle: a gunner works the turret, a passenger in a helicopter or a boat shoots at what
// comes near, and a passenger in a wheeled transport gets out when it has stopped for a while
void Bots::thinkRider( GameEntity* ent, State& bot, usercmd_t& cmd, vec3_t view )
{
	GameClient* client = ent->client_;
	GameEntity* pilot = theLevel.getEntity( client->sess_.ridingClient_ );
	if( !alive( pilot ) )
		return;

	VectorCopy( pilot->client_->ps_.vehicleAngles, view );
	view[ROLL] = 0;
	const bool gunner = client->sess_.seat_ == 1 && VehicleData::vehicleHasTurret( pilot->client_->vehicle_ );
	// a passenger in a wheeled transport gets out once it has stopped for them
	constexpr int rideTime = 6000;
	const completeVehicleData_t& ride = availableVehicles[pilot->client_->vehicle_];
	if( !gunner && (ride.cat & CAT_GROUND) && (ride.caps & HC_WHEELS) && VectorLength( pilot->client_->ps_.velocity ) < 15 )
	{
		if( !bot.liftTime )
			bot.liftTime = theLevel.time_;
		else if( theLevel.time_ > bot.liftTime + rideTime && theLevel.time_ > bot.commandTime )
		{
			bot.liftTime = 0;
			bot.commandTime = theLevel.time_ + 2000;
			Cmd_Eject_f( ent );
			return;
		}
	}
	else
		bot.liftTime = 0;
	// passengers shoot only from open vehicles
	if( !gunner && !(availableVehicles[pilot->client_->vehicle_].cat & (CAT_HELO|CAT_BOAT)) )
		return;
	const float reach = gunner ? 3500.0f : 1500.0f;

	// the nearest enemy within reach of the seat's weapon
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

	Vec3 dir, angles;
	dir = enemy->r.currentOrigin - pilot->r.currentOrigin;
	vectoangles( dir, angles );
	view[YAW] = angles[YAW];
	view[PITCH] = std::clamp( AngleSubtract( angles[PITCH], 0 ), -60.0f, 60.0f );
	if( !visible( pilot, pilot->r.currentOrigin, enemy ) )
		return;
	cmd.buttons |= gunner ? BUTTON_ATTACK_MAIN : BUTTON_ATTACK;
}
