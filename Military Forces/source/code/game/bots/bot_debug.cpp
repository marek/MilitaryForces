#include "../g_local.h"
#include "../g_level.h"
#include "bot.h"
#include "bot_local.h"
#include "bot_airnav.h"
#include "bot_groundnav.h"

#ifdef MF_BOT_DEBUG

void Bots::logDeath( GameEntity* self, GameEntity* attacker, int meansOfDeath )
{
	const State& bot = states_[self->s.number];

	const playerState_t& ps = self->client_->ps_;
	Vec3 forward, start;
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
	Vec3 below;
	VectorCopy( self->r.currentOrigin, below );
	below[2] -= heightAboveGround( self->r.currentOrigin, self->s.number ) + 4;

	Common::printf( "BOTDEATH t %d bot %d mod %d by %d veh %d role %d goal %d age %d health %d org %.0f %.0f %.0f vel %.0f %.0f %.0f speed %d pitch %.0f roll %.0f yaw %.0f "
		"height %.0f ceiling %.0f water %d onoff %x throttle %d fuel %d enemy %d edist %.0f avoid %d egress %d land %d pad %d ahead %.0f aheadent %d aheadclass %s normalz %.2f "
		"nearest %d ndist %.0f trail",
		theLevel.time_, self->s.number, meansOfDeath, attacker ? attacker->s.number : -1, self->client_->vehicle_, static_cast<int>( roleOf( self->client_->vehicle_ ) ),
		static_cast<int>( bot.desire ), theLevel.time_ - bot.spawnTime, self->health_, self->r.currentOrigin[0], self->r.currentOrigin[1], self->r.currentOrigin[2],
		ps.velocity[0], ps.velocity[1], ps.velocity[2], ps.speed, ps.vehicleAngles[PITCH], ps.vehicleAngles[ROLL], ps.vehicleAngles[YAW],
		heightAboveGround( self->r.currentOrigin, self->s.number ), ceilingAbove( self->r.currentOrigin, self->s.number ),
		(GameHost::pointContents( below, self->s.number ) & MASK_WATER) != 0, ps.ONOFF, ps.fixed_throttle, ps.stats[STAT_FUEL],
		bot.enemy, enemy ? Distance( enemy->r.currentOrigin, self->r.currentOrigin ) : -1.0f, theLevel.time_ < bot.avoidTime, theLevel.time_ < bot.egressTime,
		bot.landPhase, bot.pad, ahead.fraction * 600.0f - 100.0f, ahead.fraction < 1.0f ? ahead.entityNum : -1, hit && hit->classname_ ? hit->classname_ : "-",
		ahead.plane.normal[2], nearestNum, nearest );
	for( int i = 0; i < 6; i++ )
		Common::printf( " %.0f,%.0f,%.0f", bot.trail[i][0], bot.trail[i][1], bot.trail[i][2] );
	Common::printf( "\n" );
}

void Bots::logState( GameEntity* ent, State& bot, const usercmd_t& cmd, const vec3_t view )
{
	GameClient* client = ent->client_;
	const playerState_t& ps = client->ps_;
	bot.logTime = theLevel.time_ + (g_botDebug.integer >= 4 ? 100 : 500);
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
		"onoff %x throttle %d health %d enemy %d clear nose %.0f fwd %.0f back %.0f left %.0f right %.0f up %.0f down %.0f avoid %d egress %d land %d follow %d confined %d brake %d probe %.0f %.0f waypoint %.0f %.0f %.0f of %d\n",
		theLevel.time_, ent->s.number, client->vehicle_, static_cast<int>( bot.desire ), ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2],
		ps.velocity[0], ps.velocity[1], ps.velocity[2], ps.speed, ps.vehicleAngles[PITCH], ps.vehicleAngles[YAW], ps.vehicleAngles[ROLL], view[PITCH], view[YAW],
		cmd.forwardmove, cmd.rightmove, cmd.buttons, ps.ONOFF, ps.fixed_throttle, ent->health_, bot.enemy,
		reach[0], reach[1], reach[2], reach[3], reach[4], reach[5], reach[6], theLevel.time_ < bot.avoidTime, theLevel.time_ < bot.egressTime, bot.landPhase,
		bot.following, bot.confined, theLevel.time_ < bot.brakeTime, bot.probeWant, bot.probeBest, bot.pathIndex < bot.path.size() ? bot.path[bot.pathIndex].x() : 0.0f, bot.pathIndex < bot.path.size() ? bot.path[bot.pathIndex].y() : 0.0f,
		bot.pathIndex < bot.path.size() ? bot.path[bot.pathIndex].z() : 0.0f, static_cast<int>( bot.path.size() ) );
}

void Bots::listCommand()
{
	Common::printf( "BOTTIME %d\n", theLevel.time_ );
	Common::printf( "AIRNAV nodes %d free %d\n", AirNav::cellCount(), AirNav::freeCount() );
	Common::printf( "GROUNDNAV cells %d known %d\n", GroundNav::cellCount(), GroundNav::knownCount() );
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
		Common::printf( "BOT %d %s vehicle %d health %d org %.0f %.0f %.0f speed %d enemy %d riding %d score %d height %.0f onoff %x angles %.0f %.0f fuel %d goal %d role %d ammo %d %d %d %d goals %d %d teamplayer %d leader %d leadertarget %d\n", i, cl->pers_.netname_,
			cl->vehicle_, ent->health_, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], cl->ps_.speed,
			states_[i].enemy, cl->sess_.ridingClient_, cl->ps_.persistant[PERS_SCORE], heightAboveGround( ent->r.currentOrigin, i ),
			cl->ps_.ONOFF, cl->ps_.vehicleAngles[PITCH], cl->ps_.vehicleAngles[YAW], cl->ps_.stats[STAT_FUEL],
			static_cast<int>( states_[i].desire ), cl->vehicle_ >= 0 ? static_cast<int>( roleOf( cl->vehicle_ ) ) : -1,
			cl->ps_.ammo[1], cl->ps_.ammo[2], cl->ps_.ammo[3], cl->ps_.ammo[4], states_[i].goalsReached, states_[i].goalsExpired,
			states_[i].teamPlayer, states_[i].leader, leaderOf( states_[i] ) ? targetOf( leaderOf( states_[i] ) ) : -1 );
	}
}

#endif
