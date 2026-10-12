#include "../../g_local.h"
#include "../../g_level.h"
#include "../../g_team.h"
#include "../bot.h"
#include "../core/local.h"
#include "team.h"
#include "../navigation/groundnav.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <vector>

namespace
{
	using Job = BotTeam::Job;
	using Kind = BotTeam::Kind;
	constexpr int kinds = static_cast<int>( Kind::Count );
	constexpr int teams = ClientBase::TEAM_NUM_TEAMS;

	constexpr int planEvery = 10000;	// the plan is remade this often, and at once when a flag changes hands
	constexpr int tickEvery = 250;	// what must follow the game closely: wards on the move, joint attacks
	constexpr float middleSkill = 2.5f, highSkill = 3.5f;	// g_botSkill from which a team plans, and from which it times things
	constexpr int sharedAfterMiddle = 3000;	// a sighting reaches the team this long after it was made, at middle skill
	constexpr int sightingLife = 20000;

	// shares of a team per kind; a kind the map or the vehicle set cannot field gives its share to the others
	constexpr std::array<float, kinds> modernShares = { 0.34f, 0.10f, 0.16f, 0.0f, 0.20f, 0.10f, 0.10f, 0.0f };
	constexpr std::array<float, kinds> ww2Shares = { 0.40f, 0.10f, 0.0f, 0.0f, 0.30f, 0.20f, 0.0f, 0.0f };
	constexpr int firstSupplierFrom = 6, secondSupplierFrom = 12;

	// capture the flag: shares of those who can carry a flag
	struct Split { float attack, defend; };
	constexpr Split evenSplit = { 0.5f, 0.34f }, behindSplit = { 0.65f, 0.2f }, aheadLateSplit = { 0.35f, 0.5f };
	constexpr float lateInGame = 0.6f;	// of the time limit
	constexpr float defendRadius = 1400.0f;	// defenders keep within this of what they guard
	constexpr float humanDefending = 2500.0f;	// a real player this near the own flag counts as a defender

	// a push: gather short of the objective, then go together
	constexpr float gatherShort = 3000.0f, gathered = 900.0f;
	constexpr float enoughGathered = 0.7f;
	constexpr int gatherPatience = 35000, pushLasts = 90000, fightersLead = 8000, heloDelayMost = 15000;
	constexpr int pushFrom = 3;	// fewer attackers than this just go

	constexpr int carrierEscort = 2, chasers = 2, blockers = 2;
	constexpr float escortBehind = 250.0f, supplyBehind = 1500.0f, scoutAhead = 1200.0f;
	constexpr float routeClear = 1500.0f;	// a carrier's way home keeps this far from where an enemy was last seen

	// several on one: bearings spread about the line the attackers come from; on a bomber with turrets, abreast from ahead
	constexpr float pincerSpread = 70.0f, abreastSpread = 20.0f, heightStep = 300.0f;
	constexpr int jointPatience = 12000;

	struct Sighting { Vec3 position; int time = 0; };

	struct Plan
	{
		int		nextPlan = 0;
		int		ourCarrier = -1;	// a team mate with their flag
		int		theirCarrier = -1;	// an enemy with ours
		bool	hasEffort = false;
		Vec3	effort;	// where the team's attack is going
		Vec3	gather;
		bool	pushing = false;
		int		gatherSince = 0;
		int		pushSince = 0;
		std::array<int, kinds> have = {};	// counted at the last plan
		std::array<int, kinds> want = {};
		std::array<Sighting, MAX_CLIENTS + 1> seen;
	};

	std::array<Plan, teams> plans;
	std::array<BotTeam::Assignment, MAX_CLIENTS + 1> assignments;
	std::array<Vec3, teams> bases;
	bool basesKnown = false;
	int nextTick = 0;
	// what the measures count
	std::array<int, teams> kills = {}, jointKills = {};

	bool isTeam( int team ) { return team == ClientBase::TEAM_RED || team == ClientBase::TEAM_BLUE; }
	int other( int team ) { return team == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED; }
	const char* teamName( int team ) { return team == ClientBase::TEAM_RED ? "red" : "blue"; }

	const char* jobName( Job job )
	{
		constexpr std::array<const char*, 8> names = { "none", "attack", "defend", "escort", "aircover", "supply", "chase", "cutoff" };
		return names[static_cast<int>( job )];
	}
	const char* kindName( int kind )
	{
		constexpr std::array<const char*, kinds> names = { "tank", "airdefence", "attackhelo", "supply", "fighter", "bomber", "boat", "other" };
		return names[kind];
	}

	// the middle of a team's base: its flag, or else its spawn points on the ground
	void findBases()
	{
		basesKnown = true;
		for( int team : { ClientBase::TEAM_RED, ClientBase::TEAM_BLUE } )
		{
			vec3_t flag;
			if( BotWorld::flagPosition( team, flag ) )
			{
				bases[team] = Vec3( flag );
				continue;
			}
			const char* prefix = team == ClientBase::TEAM_RED ? "team_CTF_red" : "team_CTF_blue";
			Vec3 sum( 0, 0, 0 );
			int count = 0;
			for( int i = MAX_CLIENTS + 1; i < theLevel.num_entities_; i++ )
			{
				GameEntity* ent = theLevel.getEntity( i );
				if( !ent || !ent->inuse_ || !ent->classname_ || Q_stricmpn( ent->classname_, prefix, static_cast<int>( strlen( prefix ) ) ) != 0 )
					continue;
				sum = sum + Vec3( ent->s.origin );
				count++;
			}
			bases[team] = count ? sum * (1.0f / count) : Vec3( 0, 0, 0 );
		}
	}

	bool flagCarrier( GameEntity* ent ) { return (ent->client_->ps_.objectives & (OB_REDFLAG|OB_BLUEFLAG)) != 0; }
	bool carriesFlags( Kind kind ) { return kind == Kind::Tank || kind == Kind::Boat || kind == Kind::AttackHelo || kind == Kind::AirDefence; }
	bool flies( Kind kind ) { return kind == Kind::Fighter || kind == Kind::Bomber || kind == Kind::AttackHelo || kind == Kind::Supply; }

	struct Member
	{
		GameEntity*	ent;
		int			num;
		Kind		kind;
		bool		bot;
		Vec3		origin;
	};

	std::vector<Member> membersOf( int team )
	{
		std::vector<Member> out;
		for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameEntity* ent = theLevel.getEntity( i );
			if( !alive( ent ) || ent->client_->sess_.sessionTeam_ != team || ent->client_->sess_.ridingClient_ >= 0 )
				continue;
			out.push_back( { ent, i, BotTeam::kindOf( ent->client_->vehicle_ ), Bots::isBot( i ), Vec3( ent->r.currentOrigin ) } );
		}
		return out;
	}

	// a point this far from one place towards another, never past it
	Vec3 towards( const Vec3& from, const Vec3& to, float distance )
	{
		const Vec3 way = to - from;
		const float length = way.length();
		return length <= distance || length < 1.0f ? to : from + way * (distance / length);
	}

	Vec3 middleOf( const std::vector<const Member*>& group, const Vec3& otherwise )
	{
		if( group.empty() )
			return otherwise;
		Vec3 sum( 0, 0, 0 );
		for( const Member* m : group )
			sum = sum + m->origin;
		return sum * (1.0f / group.size());
	}

	// what each bot was doing under the last plan: whoever holds a job keeps it unless another is clearly better placed,
	// so that a team does not swap its guards and its attackers every time it thinks again
	std::array<Job, MAX_CLIENTS + 1> lastJobs;
	constexpr float incumbent = 0.5f;

	// the nearest of the candidates not yet given a job
	const Member* takeNearest( std::vector<const Member*>& free, const Vec3& to, Job job, bool (*fits)( Kind ) = nullptr )
	{
		auto best = free.end();
		float bestDist = 0;
		for( auto it = free.begin(); it != free.end(); ++it )
		{
			if( fits && !fits( (*it)->kind ) )
				continue;
			const float dist = Distance( (*it)->origin, to ) * (lastJobs[(*it)->num] == job ? incumbent : 1.0f);
			if( best == free.end() || dist < bestDist )
			{
				best = it;
				bestDist = dist;
			}
		}
		if( best == free.end() )
			return nullptr;
		const Member* taken = *best;
		free.erase( best );
		return taken;
	}

	// where an enemy is, as far as this team knows
	bool whereIs( const Plan& plan, int enemy, Vec3& out )
	{
		const Sighting& s = plan.seen[enemy];
		if( !s.time || theLevel.time_ > s.time + sightingLife )
			return false;
		out = s.position;
		return true;
	}

	// the carrier's next stretch home: straight, unless an enemy was last seen near that line; then round the far side of it
	Vec3 wayHome( const Plan& plan, int team, const Vec3& from, const Vec3& home )
	{
		const Vec3 way = home - from;
		const float length = way.length();
		if( length < routeClear )
			return home;
		const Vec3 dir = way * (1.0f / length);
		for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameEntity* enemy = theLevel.getEntity( i );
			Vec3 at;
			if( !alive( enemy ) || enemy->client_->sess_.sessionTeam_ != other( team ) || !whereIs( plan, i, at ) )
				continue;
			const Vec3 rel = at - from;
			const float along = DotProduct( rel, dir );
			if( along < 0 || along > std::min( length, routeClear * 2.0f ) )
				continue;
			const Vec3 off = rel - dir * along;
			const float miss = off.length();
			if( miss >= routeClear )
				continue;
			// pass on the side away from the enemy, far enough out to be clear of it
			Vec3 away = miss > 1.0f ? off * (-1.0f / miss) : Vec3( -dir[1], dir[0], 0 );
			away[2] = 0;
			return from + dir * along + away * (routeClear - miss + 200.0f);
		}
		return home;
	}

	void makePlan( int team )
	{
		Plan& plan = plans[team];
		plan.nextPlan = theLevel.time_ + planEvery;
		const std::vector<Member> members = membersOf( team );
		const Vec3 ownBase = bases[team], enemyBase = bases[other( team )];
		const bool ctf = g_gametype.integer == GT_CTF;
		const int level = BotTeam::level();

		plan.have.fill( 0 );
		for( const Member& m : members )
			plan.have[static_cast<int>( m.kind )]++;

		// what the real players are doing shapes the rest: where the one furthest forward is, the attack goes
		const Member* lead = nullptr;
		int humansDefending = 0, humansAttacking = 0;
		for( const Member& m : members )
		{
			if( m.bot || m.kind == Kind::Other )
				continue;
			if( Distance( m.origin, ownBase ) < humanDefending )
				humansDefending++;
			else
			{
				humansAttacking++;
				if( !lead || Distance( m.origin, enemyBase ) < Distance( lead->origin, enemyBase ) )
					lead = &m;
			}
		}

		// the effort: their flag; in a team deathmatch where the real players are fighting, else where their ground
		// forces were last seen, else their base
		plan.effort = enemyBase;
		plan.hasEffort = true;
		if( !ctf )
		{
			Vec3 sum( 0, 0, 0 );
			int count = 0;
			for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
			{
				GameEntity* enemy = theLevel.getEntity( i );
				Vec3 at;
				if( alive( enemy ) && enemy->client_->sess_.sessionTeam_ == other( team ) && !flies( BotTeam::kindOf( enemy->client_->vehicle_ ) ) && whereIs( plan, i, at ) )
				{
					sum = sum + at;
					count++;
				}
			}
			if( lead )
				plan.effort = towards( lead->origin, enemyBase, scoutAhead );
			else if( count )
				plan.effort = sum * (1.0f / count);
		}

		std::vector<const Member*> free;
		for( const Member& m : members )
		{
			if( m.bot )
			{
				lastJobs[m.num] = assignments[m.num].job;
				assignments[m.num] = BotTeam::Assignment{};
				free.push_back( &m );
			}
		}
		const auto give = [&]( const Member* m, Job job ) -> BotTeam::Assignment&
		{
			BotTeam::Assignment& a = assignments[m->num];
			a.job = job;
			return a;
		};

		// flags first: the carrier's escort, and the hunt for theirs
		plan.ourCarrier = plan.theirCarrier = -1;
		for( int i = 1; ctf && i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameEntity* ent = theLevel.getEntity( i );
			if( !alive( ent ) || !flagCarrier( ent ) )
				continue;
			if( ent->client_->sess_.sessionTeam_ == team )
				plan.ourCarrier = i;
			else
				plan.theirCarrier = i;
		}
		if( plan.ourCarrier >= 0 )
		{
			GameEntity* carrier = theLevel.getEntity( plan.ourCarrier );
			const Vec3 at( carrier->r.currentOrigin );
			if( Bots::isBot( plan.ourCarrier ) )
			{
				for( auto it = free.begin(); it != free.end(); ++it )
				{
					if( (*it)->num != plan.ourCarrier )
						continue;
					BotTeam::Assignment& a = give( *it, Job::Attack );
					a.hasPlace = true;
					a.place = wayHome( plan, team, at, ownBase );
					free.erase( it );
					break;
				}
			}
			for( int n = 0; n < carrierEscort + (level >= 2 ? 1 : 0); n++ )
			{
				const Member* m = takeNearest( free, at, Job::Escort, []( Kind kind ) { return kind != Kind::Supply && kind != Kind::Bomber && kind != Kind::Other; } );
				if( !m )
					break;
				give( m, Job::Escort ).ward = plan.ourCarrier;
			}
		}
		if( plan.theirCarrier >= 0 )
		{
			Vec3 at;
			if( !whereIs( plan, plan.theirCarrier, at ) )
				at = ownBase;
			for( int n = 0; n < chasers; n++ )
			{
				const Member* m = takeNearest( free, at, Job::Chase, []( Kind kind ) { return kind != Kind::Supply && kind != Kind::Other; } );
				if( !m )
					break;
				BotTeam::Assignment& a = give( m, Job::Chase );
				a.target = plan.theirCarrier;
				a.hasPlace = true;
				a.place = at;
			}
			// the places it has to pass on its way home, from the path finder; others wait for it there
			if( level >= 2 )
			{
				std::vector<Vec3> path;
				GroundNav::findPath( at, enemyBase, false, path );
				for( int n = 0; n < blockers; n++ )
				{
					const float part = n == 0 ? 0.5f : 0.8f;
					const Vec3 spot = path.size() >= 4 ? path[static_cast<size_t>( (path.size() - 1) * part )] : at + (enemyBase - at) * part;
					const Member* m = takeNearest( free, spot, Job::CutOff, carriesFlags );
					if( !m )
						break;
					BotTeam::Assignment& a = give( m, Job::CutOff );
					a.target = plan.theirCarrier;
					a.hasPlace = true;
					a.place = spot;
				}
			}
		}

		// who attacks and who stays: in capture the flag by share, shifted by the score and by what the real players took on
		std::vector<const Member*> attackers;
		if( ctf )
		{
			int carriers = humansAttacking + humansDefending;
			for( const Member* m : free )
				if( carriesFlags( m->kind ) )
					carriers++;
			Split split = evenSplit;
			const int ours = theLevel.teamScores_[team], theirs = theLevel.teamScores_[other( team )];
			const int timelimit = CvarSystem::variableIntegerValue( "timelimit" );
			const bool late = timelimit > 0 && theLevel.time_ - theLevel.startTime_ > timelimit * 60000 * lateInGame;
			if( ours < theirs )
				split = behindSplit;
			else if( ours > theirs && late )
				split = aheadLateSplit;
			int defenders = std::max( 0, static_cast<int>( std::lround( carriers * split.defend ) ) - humansDefending );
			if( plan.theirCarrier >= 0 )
				defenders = 0;	// the flag is gone: nothing to stand guard over
			for( int n = 0; n < defenders; n++ )
			{
				const Member* m = takeNearest( free, ownBase, Job::Defend, carriesFlags );
				if( !m )
					break;
				BotTeam::Assignment& a = give( m, Job::Defend );
				a.hasPlace = true;
				// spread round the flag rather than all on top of it
				const float angle = (m->num * 2.4f) + theLevel.time_ / static_cast<float>( planEvery );
				a.place = ownBase + Vec3( cos( angle ), sin( angle ), 0 ) * (defendRadius * 0.5f);
			}
		}
		for( auto it = free.begin(); it != free.end(); )
		{
			if( (*it)->kind == Kind::Tank || (*it)->kind == Kind::Boat || (*it)->kind == Kind::AttackHelo )
			{
				attackers.push_back( *it );
				it = free.erase( it );
			}
			else
				++it;
		}
		if( lead )
			attackers.push_back( lead );
		const Vec3 front = middleOf( attackers, towards( ownBase, plan.effort, gatherShort ) );
		if( lead )
			attackers.pop_back();

		// a push gathers short of the objective and goes together; a real player out in front is the gathering point
		int groundGo = 0, heloGo = 0, fighterGo = 0;
		Vec3 attackPlace = plan.effort;
		if( level >= 2 && static_cast<int>( attackers.size() ) + (lead ? 1 : 0) >= pushFrom )
		{
			if( plan.pushing && theLevel.time_ > plan.pushSince + pushLasts )
				plan.pushing = false;
			if( !plan.pushing )
			{
				if( !plan.gatherSince )
					plan.gatherSince = theLevel.time_;
				plan.gather = lead ? lead->origin : towards( plan.effort, front, gatherShort );
				int near = 0;
				for( const Member* m : attackers )
					if( Distance( m->origin, plan.gather ) < gathered )
						near++;
				if( near >= attackers.size() * enoughGathered || theLevel.time_ > plan.gatherSince + gatherPatience )
				{
					plan.pushing = true;
					plan.pushSince = theLevel.time_;
					plan.gatherSince = 0;
				}
			}
			if( plan.pushing )
			{
				// fighters go at once, the ground after them, and the helicopters, which are faster, later still
				fighterGo = plan.pushSince;
				groundGo = plan.pushSince + fightersLead;
				heloGo = groundGo + std::min( heloDelayMost, static_cast<int>( Distance( plan.gather, plan.effort ) / 60.0f * 1000.0f * 0.5f ) );
			}
			else
			{
				attackPlace = plan.gather;
				groundGo = heloGo = fighterGo = theLevel.time_ + planEvery * 2;
			}
		}
		else
		{
			plan.pushing = false;
			plan.gatherSince = 0;
		}
		for( const Member* m : attackers )
		{
			BotTeam::Assignment& a = give( m, Job::Attack );
			a.hasPlace = true;
			// helicopters go ahead of the ground along its way and see what it is about to meet
			a.place = m->kind == Kind::AttackHelo && !plan.pushing && level >= 2 ? towards( attackPlace, plan.effort, scoutAhead ) : attackPlace;
			if( plan.pushing )
				a.place = plan.effort;
			a.goTime = m->kind == Kind::AttackHelo ? heloGo : groundGo;
		}

		// the rest by kind
		const Member* firstTank = nullptr;
		for( const Member* m : attackers )
			if( m->kind == Kind::Tank && (!firstTank || Distance( m->origin, front ) < Distance( firstTank->origin, front )) )
				firstTank = m;
		// an enemy air defence vehicle where the push is going is the bombers' first target
		int enemyAirDefence = -1;
		for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameEntity* enemy = theLevel.getEntity( i );
			Vec3 at;
			if( alive( enemy ) && enemy->client_->sess_.sessionTeam_ == other( team ) && BotTeam::kindOf( enemy->client_->vehicle_ ) == Kind::AirDefence && whereIs( plan, i, at ) &&
				( enemyAirDefence < 0 || Distance( at, plan.effort ) < Distance( plans[team].seen[enemyAirDefence].position, plan.effort ) ) )
				enemyAirDefence = i;
		}
		for( const Member* m : free )
		{
			switch( m->kind )
			{
			case Kind::AirDefence:
				if( firstTank || lead )
					give( m, Job::Escort ).ward = firstTank ? firstTank->num : lead->num;
				else
				{
					BotTeam::Assignment& a = give( m, Job::Defend );
					a.hasPlace = true;
					a.place = ownBase;
				}
				break;
			case Kind::Fighter:
			{
				BotTeam::Assignment& a = give( m, Job::AirCover );
				a.hasPlace = true;
				a.place = plan.ourCarrier >= 0 ? Vec3( theLevel.getEntity( plan.ourCarrier )->r.currentOrigin ) : plan.pushing ? plan.effort : front;
				a.goTime = fighterGo;
				break;
			}
			case Kind::Bomber:
				give( m, Job::Attack ).target = enemyAirDefence;
				break;
			case Kind::Supply:
			{
				BotTeam::Assignment& a = give( m, Job::Supply );
				a.hasPlace = true;
				a.place = towards( front, ownBase, supplyBehind );
				break;
			}
			default:
				break;
			}
		}

#ifdef MF_BOT_DEBUG
		if( g_botDebug.integer >= 2 )
		{
			Common::printf( "BOTTEAM t %d %s level %d effort %.0f %.0f pushing %d carrier ours %d theirs %d jobs", theLevel.time_, teamName( team ), level,
				plan.effort[0], plan.effort[1], plan.pushing, plan.ourCarrier, plan.theirCarrier );
			for( const Member& m : members )
				if( m.bot )
					Common::printf( " %d:%s:%s", m.num, kindName( static_cast<int>( m.kind ) ), jobName( assignments[m.num].job ) );
			Common::printf( "\n" );
		}
#endif
	}

	// between plans: wards move, so their escorts' places do; and those fighting one enemy together are given their sides
	void tick( int team )
	{
		const int level = BotTeam::level();
		std::array<std::vector<int>, MAX_CLIENTS + 1> hunters;
		for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameEntity* ent = theLevel.getEntity( i );
			if( !Bots::isBot( i ) || !alive( ent ) || ent->client_->sess_.sessionTeam_ != team )
				continue;
			BotTeam::Assignment& a = assignments[i];
			if( a.job == Job::Escort )
			{
				GameEntity* ward = a.ward >= 0 ? theLevel.getEntity( a.ward ) : nullptr;
				a.hasPlace = alive( ward );
				if( a.hasPlace )
				{
					Vec3 forward;
					const Vec3 level( 0, ward->client_->ps_.vehicleAngles[YAW], 0 );
					AngleVectors( level, forward, nullptr, nullptr );
					a.place = Vec3( ward->r.currentOrigin ) - forward * escortBehind;
				}
			}
			a.flank = false;
			const Bots::State& bot = BotTeam::stateOf( i );
			if( level >= 2 && bot.enemy >= 1 && bot.enemy <= MAX_CLIENTS && bot.desire == Bots::Goal::Attack )
				hunters[bot.enemy].push_back( i );
		}
		for( int e = 1; e <= MAX_CLIENTS; e++ )
		{
			const std::vector<int>& group = hunters[e];
			GameEntity* enemy = theLevel.getEntity( e );
			if( group.size() < 2 || !alive( enemy ) )
				continue;
			const bool turrets = (availableVehicles[enemy->client_->vehicle_].flags & VF_GUNSTATIONS) != 0;
			const Vec3 at( enemy->r.currentOrigin );
			Vec3 from( 0, 0, 0 );
			for( int i : group )
				from = from + Vec3( theLevel.getEntity( i )->r.currentOrigin );
			from = from * (1.0f / group.size()) - at;
			Vec3 angles;
			vectoangles( from, angles );
			// on a bomber with turrets: abreast from ahead of it, so its gunners have to split their fire
			const float centre = turrets ? enemy->client_->ps_.vehicleAngles[YAW] : angles[YAW];
			const float spread = turrets ? abreastSpread : pincerSpread;
			bool together = true;
			for( int i : group )
				together = together && Distance( theLevel.getEntity( i )->r.currentOrigin, at ) < gunRange * 1.3f;
			for( size_t n = 0; n < group.size(); n++ )
			{
				BotTeam::Assignment& a = assignments[group[n]];
				a.flank = true;
				a.bearing = centre + (n - (group.size() - 1) * 0.5f) * spread;
				a.lift = (n % 2 ? heightStep : -heightStep) * (group.size() > 2 || n ? 1.0f : 0.0f);
				// they go in when all are in place, or when waiting has gone on long enough
				if( !turrets || together )
					a.goTime = std::min( a.goTime ? a.goTime : theLevel.time_, theLevel.time_ );
				else if( a.goTime <= theLevel.time_ - jointPatience || a.goTime == 0 )
					a.goTime = theLevel.time_ + jointPatience;
			}
		}
	}
}

const Bots::State& BotTeam::stateOf( int clientNum )
{
	return Bots::states_[clientNum];
}

void BotTeam::reset()
{
	plans.fill( Plan{} );
	assignments.fill( Assignment{} );
	lastJobs.fill( Job::None );
	basesKnown = false;
	nextTick = 0;
	kills.fill( 0 );
	jointKills.fill( 0 );
}

int BotTeam::level()
{
	if( !g_botStrategy.integer || g_gametype.integer < GT_TEAM )
		return 0;
	return g_botSkill.value >= highSkill ? 2 : g_botSkill.value >= middleSkill ? 1 : 0;
}

const BotTeam::Assignment& BotTeam::of( int clientNum )
{
	static const Assignment none;
	return level() && clientNum >= 1 && clientNum <= MAX_CLIENTS ? assignments[clientNum] : none;
}

BotTeam::Kind BotTeam::kindOf( int vehicle )
{
	if( vehicle < 0 || vehicle >= bg_numberOfVehicles )
		return Kind::Other;
	if( BotWeapons::carriesSupplies( vehicle ) )
		return Kind::Supply;
	switch( BotSenses::roleOf( vehicle ) )
	{
	case Bots::Role::Fighter: return Kind::Fighter;
	case Bots::Role::Bomber: return Kind::Bomber;
	case Bots::Role::AttackHelo:
	case Bots::Role::TransportHelo: return Kind::AttackHelo;
	case Bots::Role::Tank: return Kind::Tank;
	case Bots::Role::AirDefence: return Kind::AirDefence;
	case Bots::Role::Boat: return Kind::Boat;
	default: return Kind::Other;
	}
}

void BotTeam::runFrame()
{
#ifdef MF_BOT_DEBUG
	// for the measures, with or without a plan: when each flag is picked up and when its carrier loses it
	static std::array<int, teams> carrying = { -1, -1, -1, -1 };
	for( int team : { ClientBase::TEAM_RED, ClientBase::TEAM_BLUE } )
	{
		int carrier = -1;
		for( int i = 1; g_botDebug.integer && g_gametype.integer == GT_CTF && i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameEntity* ent = theLevel.getEntity( i );
			if( alive( ent ) && flagCarrier( ent ) && ent->client_->sess_.sessionTeam_ == team )
				carrier = i;
		}
		if( carrier != carrying[team] )
		{
			Common::printf( "BOTFLAG t %d %s carrier %d was %d score %d\n", theLevel.time_, teamName( team ), carrier, carrying[team], theLevel.teamScores_[team] );
			carrying[team] = carrier;
		}
	}
#endif
	if( !level() )
		return;
	if( !basesKnown )
		findBases();
	const bool ticking = theLevel.time_ >= nextTick;
	if( ticking )
		nextTick = theLevel.time_ + tickEvery;
	for( int team : { ClientBase::TEAM_RED, ClientBase::TEAM_BLUE } )
	{
		Plan& plan = plans[team];
		// a flag that changed hands does not wait for the next plan
		int ourCarrier = -1, theirCarrier = -1;
		for( int i = 1; g_gametype.integer == GT_CTF && i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameEntity* ent = theLevel.getEntity( i );
			if( alive( ent ) && flagCarrier( ent ) )
				(ent->client_->sess_.sessionTeam_ == team ? ourCarrier : theirCarrier) = i;
		}
		if( theLevel.time_ >= plan.nextPlan || ourCarrier != plan.ourCarrier || theirCarrier != plan.theirCarrier )
			makePlan( team );
		if( ticking )
			tick( team );
	}
}

// The make-up: each kind's share of the team, counting real players' vehicles first. The bot asking is told the
// kind the team is shortest of, unless the team has no more of its own kind than it should.
bool BotTeam::wantedKind( GameEntity* ent, Kind& kind )
{
	const int team = ent->client_->sess_.sessionTeam_;
	if( !level() || !isTeam( team ) )
		return false;

	// what this team may field here
	const unsigned int cats = Bots::spawnCategories();
	const unsigned long gameset = Game::getGameset();
	const int teamFlag = team == ClientBase::TEAM_RED ? MF_TEAM_1 : MF_TEAM_2;
	std::array<bool, kinds> possible = {};
	for( int i = 0; i < bg_numberOfVehicles; i++ )
	{
		const completeVehicleData_t& veh = availableVehicles[i];
		if( (veh.gameset & gameset) && (veh.team & teamFlag) && (veh.cat & cats) && !(veh.cat & CAT_LQM) && VehicleData::selectable( i ) && !Bots::unfitHere( i ) )
			possible[static_cast<int>( kindOf( i ) )] = true;
	}

	std::array<int, kinds> have = {};
	int size = 0, own = -1;
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		GameEntity* mate = theLevel.getEntity( i );
		if( !mate || !mate->inuse_ || !mate->client_ || mate->client_->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ||
			mate->client_->sess_.sessionTeam_ != team )
			continue;
		size++;
		// a bot counts as what it has chosen, a person as what he is in or about to take
		const int vehicle = Bots::isBot( i ) ? Bots::states_[i].vehicle : mate->client_->vehicle_ >= 0 ? mate->client_->vehicle_ : mate->client_->nextVehicle_;
		if( mate == ent )
			own = vehicle >= 0 ? static_cast<int>( kindOf( vehicle ) ) : -1;
		else if( vehicle >= 0 )
			have[static_cast<int>( kindOf( vehicle ) )]++;
	}

	const std::array<float, kinds>& shares = (gameset & MF_GAMESET_MODERN) ? modernShares : ww2Shares;
	float total = 0;
	for( int k = 0; k < kinds; k++ )
		if( possible[k] )
			total += shares[k];
	const int suppliers = !possible[static_cast<int>( Kind::Supply )] ? 0 : size >= secondSupplierFrom ? 2 : size >= firstSupplierFrom ? 1 : 0;
	Plan& plan = plans[team];
	int best = -1;
	float bestShort = 0.5f;
	for( int k = 0; k < kinds; k++ )
	{
		const float want = k == static_cast<int>( Kind::Supply ) ? suppliers : possible[k] && total > 0 ? (size - suppliers) * shares[k] / total : 0.0f;
		plan.want[k] = static_cast<int>( std::lround( want ) );
		const float shortBy = want - have[k];
		if( shortBy > bestShort )
		{
			best = k;
			bestShort = shortBy;
		}
		// its own kind is not over strength without it: it may stay as it is
		if( k == own && shortBy > 0.0f )
		{
			best = own;
			break;
		}
	}
	if( best < 0 )
		return false;
	kind = static_cast<Kind>( best );
	return true;
}

// the same number on both sides, and where the numbers are level the side whose bots are the weaker shots
const char* BotTeam::sideToJoin( const Bots::State& joiner )
{
	if( !level() )
		return "auto";
	std::array<int, teams> count = {};
	std::array<float, teams> aim = {};
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		GameClient* cl = theLevel.getClient( i );
		if( !cl || cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED || !isTeam( cl->sess_.sessionTeam_ ) )
			continue;
		count[cl->sess_.sessionTeam_]++;
		// a real player counts as a good shot
		aim[cl->sess_.sessionTeam_] += Bots::isBot( i ) ? Bots::character( Bots::states_[i] ).aim : 1.0f;
	}
	const int red = ClientBase::TEAM_RED, blue = ClientBase::TEAM_BLUE;
	if( count[red] != count[blue] )
		return count[red] < count[blue] ? "red" : "blue";
	// a good shot goes to the weaker side; a poor one to the stronger, since whoever joins next has to take the other
	float all = 0;
	for( const Bots::Character& who : Bots::characters_ )
		all += who.aim;
	const bool goodShot = Bots::characters_.empty() || Bots::character( joiner ).aim >= all / Bots::characters_.size();
	const int weaker = aim[red] <= aim[blue] ? red : blue;
	return (goodShot ? weaker : other( weaker )) == red ? "red" : "blue";
}

void BotTeam::report( int team, int enemy, const vec3_t position )
{
	if( !level() || !isTeam( team ) || enemy < 1 || enemy > MAX_CLIENTS )
		return;
	Sighting& s = plans[team].seen[enemy];
	s.position = Vec3( position );
	s.time = theLevel.time_;
}

bool BotTeam::known( int team, int enemy, vec3_t position, int& time )
{
	const int lvl = level();
	if( !lvl || !isTeam( team ) || enemy < 1 || enemy > MAX_CLIENTS )
		return false;
	const Sighting& s = plans[team].seen[enemy];
	const int delay = lvl >= 2 ? 0 : sharedAfterMiddle;
	if( !s.time || theLevel.time_ < s.time + delay || theLevel.time_ > s.time + sightingLife )
		return false;
	VectorCopy( s.position, position );
	time = s.time;
	return true;
}

// for the measures: how many of a team's kills were of someone two or more of its bots were after
void BotTeam::onDeath( GameEntity* self, GameEntity* attacker )
{
	if( g_gametype.integer < GT_TEAM || !self || !self->client_ || !attacker || !attacker->client_ || attacker == self || OnSameTeam( self, attacker ) )
		return;
	const int team = attacker->client_->sess_.sessionTeam_;
	if( !isTeam( team ) )
		return;
	int hunters = 0;
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		GameEntity* mate = theLevel.getEntity( i );
		if( Bots::isBot( i ) && alive( mate ) && mate->client_->sess_.sessionTeam_ == team && Bots::states_[i].enemy == self->s.number )
			hunters++;
	}
	kills[team]++;
	if( hunters >= 2 )
		jointKills[team]++;
}

void BotTeam::listCommand()
{
	Common::printf( "BOTTEAMS level %d strategy %d skill %.1f\n", level(), g_botStrategy.integer, g_botSkill.value );
	for( int team : { ClientBase::TEAM_RED, ClientBase::TEAM_BLUE } )
	{
		const Plan& plan = plans[team];
		int count = 0;
		float aim = 0;
		std::array<int, kinds> have = {};
		for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameClient* cl = theLevel.getClient( i );
			if( !cl || cl->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED || cl->sess_.sessionTeam_ != team )
				continue;
			count++;
			if( Bots::isBot( i ) )
				aim += Bots::character( Bots::states_[i] ).aim;
			const int vehicle = Bots::isBot( i ) ? Bots::states_[i].vehicle : cl->vehicle_;
			if( vehicle >= 0 )
				have[static_cast<int>( kindOf( vehicle ) )]++;
		}
		Common::printf( "BOTTEAM %s players %d aim %.2f score %d kills %d joint %d pushing %d makeup", teamName( team ), count, aim, theLevel.teamScores_[team],
			kills[team], jointKills[team], plan.pushing );
		for( int k = 0; k < kinds; k++ )
			Common::printf( " %s %d/%d", kindName( k ), have[k], plan.want[k] );
		Common::printf( "\n" );
		for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
		{
			GameEntity* ent = theLevel.getEntity( i );
			if( !Bots::isBot( i ) || !alive( ent ) || ent->client_->sess_.sessionTeam_ != team )
				continue;
			const Assignment& a = assignments[i];
			GameEntity* ward = a.ward >= 0 ? theLevel.getEntity( a.ward ) : nullptr;
			Common::printf( "BOTJOB %s bot %d kind %s job %s place %.0f %.0f %.0f at %.0f %.0f %.0f target %d ward %d warddist %.0f flank %d go %d\n", teamName( team ), i,
				kindName( static_cast<int>( kindOf( ent->client_->vehicle_ ) ) ), jobName( a.job ), a.place[0], a.place[1], a.place[2],
				ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2], a.target, a.ward,
				alive( ward ) ? Distance( ward->r.currentOrigin, ent->r.currentOrigin ) : -1.0f, a.flank, a.goTime > theLevel.time_ ? a.goTime - theLevel.time_ : 0 );
		}
	}
}
