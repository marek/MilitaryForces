#include "g_local.h"
#include "g_level.h"
#include "g_entity.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <numbers>
#include <random>
#include <string_view>
#include <vector>

GameEntity* SP_misc_groundinstallation( GameEntity *sp_ent );

// Air defence for a base on a map that places none of its own: SAM and gun sites are stood on open, level ground
// around each team's spawn points, clear of everything a base is used for. Where they stand is drawn afresh every
// game.
namespace
{
	constexpr float nearestToSpawn = 160.0f;	// no site closer than this to where a vehicle appears
	constexpr float farthestFromSpawn = 1100.0f;	// "around the base": no further out than this from one of its spawn points
	constexpr float clearOfAreas = 120.0f;	// kept clear round recharge areas, runways and destroyable buildings
	constexpr float betweenSites = 260.0f;
	constexpr float levelGround = 0.94f;	// the ground normal's z: about 20 degrees of slope at most
	constexpr float probeAbove = 1500.0f, probeBelow = 1500.0f;
	constexpr float groundSpawnHeight = 250.0f;	// a spawn point higher over the ground than this is an air start, not part of the base
	constexpr int triesPerSite = 400;

	struct Box { Vec3 mins, maxs; };

	bool nearBox( const Box& box, const Vec3& point, float margin )
	{
		return point[0] > box.mins[0] - margin && point[0] < box.maxs[0] + margin &&
			point[1] > box.mins[1] - margin && point[1] < box.maxs[1] + margin;
	}

	bool isClass( const GameEntity* ent, std::string_view prefix )
	{
		return ent->classname_ && std::string_view( ent->classname_ ).starts_with( prefix );
	}

	// what a base is used for, gathered once
	struct BaseMap
	{
		std::vector<Vec3>	spawns[ClientBase::TEAM_NUM_TEAMS];	// on the ground, by team
		std::vector<Vec3>	allSpawns;
		std::vector<Box>	keepClear;	// recharge areas, runways, destroyable buildings
		std::vector<Vec3>	sites;
		bool				hasSites = false;	// the map or its mission placed installations itself
	};

	BaseMap survey()
	{
		BaseMap map;
		for( int i = 0; i < theLevel.num_entities_; i++ )
		{
			GameEntity* ent = theLevel.getEntity( i );
			if( !ent || !ent->inuse_ )
				continue;
			if( ent->s.eType == ET_MISC_VEHICLE && ent->s.modelindex == 255 )
			{
				map.hasSites = true;
				map.sites.push_back( Vec3( ent->r.currentOrigin ) );
			}
			else if( isClass( ent, "trigger_recharge" ) || ent->s.eType == ET_EXPLOSIVE )
				map.keepClear.push_back( { Vec3( ent->r.absmin ), Vec3( ent->r.absmax ) } );
			else if( isClass( ent, "team_CTF_" ) || isClass( ent, "info_player_" ) )
			{
				const Vec3 origin( ent->s.origin );
				map.allSpawns.push_back( origin );
				Vec3 below = origin;
				below[2] -= groundSpawnHeight;
				trace_t ground;
				GameHost::trace( &ground, origin, nullptr, nullptr, below, ENTITYNUM_NONE, MASK_SOLID, false );
				if( ground.fraction >= 1.0f )
					continue;
				if( isClass( ent, "team_CTF_red" ) )
					map.spawns[ClientBase::TEAM_RED].push_back( origin );
				else if( isClass( ent, "team_CTF_blue" ) )
					map.spawns[ClientBase::TEAM_BLUE].push_back( origin );
			}
		}
		return map;
	}

	float nearest( const std::vector<Vec3>& points, const Vec3& to )
	{
		float best = 1.0e9f;
		for( const Vec3& point : points )
			best = std::min( best, Distance( point, to ) );
		return best;
	}

	// the ground under a point if a site of this size may stand there
	bool siteAt( const BaseMap& map, int team, int installation, const Vec3& from, Vec3& out )
	{
		const groundInstallationData_t& data = availableGroundInstallations[installation];
		Vec3 top = from, bottom = from;
		top[2] += probeAbove;
		bottom[2] -= probeBelow;
		trace_t open, ground;
		// open sky over it: not in a hangar, a tunnel or under a bridge
		GameHost::trace( &open, from, nullptr, nullptr, top, ENTITYNUM_NONE, MASK_SOLID, false );
		if( open.startsolid || (open.fraction < 1.0f && !(open.surfaceFlags & SURF_SKY)) )
			return false;
		GameHost::trace( &ground, from, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID | MASK_WATER, false );
		if( ground.fraction >= 1.0f || ground.entityNum != ENTITYNUM_WORLD || ground.plane.normal[2] < levelGround ||
			(ground.contents & MASK_WATER) || (ground.surfaceFlags & SURF_SKY) )
			return false;
		const Vec3 spot( ground.endpos );
		const int other = team == ClientBase::TEAM_RED ? ClientBase::TEAM_BLUE : ClientBase::TEAM_RED;
		if( nearest( map.allSpawns, spot ) < nearestToSpawn || nearest( map.sites, spot ) < betweenSites ||
			nearest( map.spawns[other], spot ) < nearest( map.spawns[team], spot ) )
			return false;
		for( const Box& box : map.keepClear )
			if( nearBox( box, spot, clearOfAreas ) )
				return false;
		// room for the thing itself, standing on the ground
		Vec3 standing = spot;
		standing[2] += 1.0f - data.mins[2];
		trace_t room;
		GameHost::trace( &room, standing, data.mins, data.maxs, standing, ENTITYNUM_NONE, MASK_PLAYERSOLID, false );
		if( room.startsolid || room.allsolid )
			return false;
		out = spot;
		out[2] -= data.mins[2];
		return true;
	}

	int installationNamed( const char* name )
	{
		for( int i = 0; i < bg_numberOfGroundInstallations; i++ )
			if( availableGroundInstallations[i].descriptiveName && !Q_stricmp( availableGroundInstallations[i].descriptiveName, name ) )
				return i;
		return -1;
	}

	// a site this file placed: which entity it is, and when its replacement is due once it is gone
	struct Site
	{
		int	entity;
		int	team;
		int	installation;
		int	replaceAt = 0;
	};
	std::vector<Site> placedSites;
	int nextSiteCheck = 0;
	constexpr int siteCheckEvery = 1000;

	bool standing( const Site& site )
	{
		GameEntity* ent = theLevel.getEntity( site.entity );
		return ent && ent->inuse_ && ent->s.eType == ET_MISC_VEHICLE && ent->s.modelindex == 255 && ent->health_ > 0 && !ent->freeAfterEvent_;
	}

	// one site of this kind at a random place round the team's base that will take it; its entity number, or -1
	int placeSite( BaseMap& map, int team, int installation, std::mt19937& dice )
	{
		if( map.spawns[team].empty() )
			return -1;
		std::uniform_real_distribution<float> anyAngle( 0.0f, 2.0f * std::numbers::pi_v<float> ), anyDistance( nearestToSpawn, farthestFromSpawn );
		std::uniform_int_distribution<size_t> anySpawn( 0, map.spawns[team].size() - 1 );
		for( int attempt = 0; attempt < triesPerSite; attempt++ )
		{
			const float angle = anyAngle( dice ), distance = anyDistance( dice );
			Vec3 from = map.spawns[team][anySpawn( dice )], spot;
			from[0] += cos( angle ) * distance;
			from[1] += sin( angle ) * distance;
			from[2] += groundSpawnHeight;
			if( !siteAt( map, team, installation, from, spot ) )
				continue;
			// the spawn routine makes the entity that stays from this one, in the next free slot
			const int before = theLevel.num_entities_;
			GameEntity* ent = theLevel.spawnEntity();
			ent->classname_ = "misc_vehicle";
			VectorCopy( spot, ent->s.origin );
			VectorSet( ent->s.angles, 0, angle * 180.0f / std::numbers::pi_v<float>, 0 );
			ent->model_ = Game::newString( availableGroundInstallations[installation].modelName );
			ent->team_ = Game::newString( team == ClientBase::TEAM_RED ? "red" : "blue" );
			ent->s.modelindex = -1;
			ent->s.modelindex2 = installation;
			SP_misc_groundinstallation( ent );
			map.sites.push_back( spot );
			for( int i = MAX_CLIENTS + 1; i < std::max( before, theLevel.num_entities_ ); i++ )
			{
				GameEntity* made = theLevel.getEntity( i );
				if( made && made->inuse_ && made->s.eType == ET_MISC_VEHICLE && made->s.modelindex == 255 && Distance( made->r.currentOrigin, spot ) < 1.0f )
					return i;
			}
			return -1;
		}
		return -1;
	}
}

void Game::placeBaseDefences()
{
	placedSites.clear();
	nextSiteCheck = 0;
	if( g_gametype.integer < GT_TEAM )
		return;
	BaseMap map = survey();
	if( map.hasSites )
		return;

	// the missiles and the guns of the vehicle set in play; a set without one gets none of that kind
	const unsigned long gameset = Game::getGameset();
	std::vector<int> missiles, guns;
	for( const char* name : { "SAM Mk1", "SAM Mk2", "SAM Mk3", "SAM Mk4" } )
		if( const int i = installationNamed( name ); i >= 0 && (availableGroundInstallations[i].gameset & gameset) )
			missiles.push_back( i );
	for( const char* name : { "AA", "Flak (modern)", "Flak (ww2)" } )
		if( const int i = installationNamed( name ); i >= 0 && (availableGroundInstallations[i].gameset & gameset) )
			guns.push_back( i );

	std::mt19937 dice( std::random_device{}() );
	for( int team : { ClientBase::TEAM_RED, ClientBase::TEAM_BLUE } )
	{
		if( map.spawns[team].empty() )
			continue;
		const struct { const std::vector<int>& kinds; int wanted; } batches[] = { { missiles, g_baseSAMs.integer }, { guns, g_baseGuns.integer } };
		int placed = 0;
		for( const auto& batch : batches )
		{
			for( int n = 0; n < batch.wanted && !batch.kinds.empty(); n++ )
			{
				const int installation = batch.kinds[n % batch.kinds.size()];
				const int entity = placeSite( map, team, installation, dice );
				if( entity < 0 )
					continue;
				placedSites.push_back( { entity, team, installation } );
				placed++;
			}
		}
		Common::printf( "Base defences: %d sites for team %s\n", placed, team == ClientBase::TEAM_RED ? "red" : "blue" );
	}
}

// A site that was destroyed is replaced after g_baseSiteRespawn seconds by one of its kind at a new random place
// round the same base; 0 leaves destroyed sites gone.
void Game::runBaseDefences()
{
	if( placedSites.empty() || theLevel.time_ < nextSiteCheck )
		return;
	nextSiteCheck = theLevel.time_ + siteCheckEvery;
	for( Site& site : placedSites )
	{
		if( site.entity >= 0 && standing( site ) )
			continue;
		site.entity = -1;
		if( g_baseSiteRespawn.integer <= 0 )
		{
			site.replaceAt = 0;
			continue;
		}
		if( !site.replaceAt )
			site.replaceAt = theLevel.time_ + g_baseSiteRespawn.integer * 1000;
		if( theLevel.time_ < site.replaceAt )
			continue;
		BaseMap map = survey();
		std::mt19937 dice( std::random_device{}() );
		site.entity = placeSite( map, site.team, site.installation, dice );
		// no room just now: tried again at the next check
		if( site.entity >= 0 )
		{
			site.replaceAt = 0;
			Common::printf( "Base defences: a %s site for team %s is back\n", availableGroundInstallations[site.installation].descriptiveName, site.team == ClientBase::TEAM_RED ? "red" : "blue" );
		}
	}
}

// lists the sites this file placed; "destroy" knocks out the first one standing, for the scenarios
void Game::baseSitesCommand()
{
	char arg[MAX_TOKEN_CHARS];
	CommandSystem::argvBuffer( 1, arg, sizeof( arg ) );
	// "areas": the runways, recharge areas and destroyable buildings sites keep clear of, as boxes
	if( !Q_stricmp( arg, "areas" ) )
	{
		for( const Box& box : survey().keepClear )
			Common::printf( "BASEAREA %.0f %.0f %.0f to %.0f %.0f %.0f\n", box.mins[0], box.mins[1], box.mins[2], box.maxs[0], box.maxs[1], box.maxs[2] );
		return;
	}
	bool destroy = !Q_stricmp( arg, "destroy" ) && g_cheats.integer;
	int up = 0;
	for( const Site& site : placedSites )
	{
		if( !standing( site ) )
			continue;
		GameEntity* ent = theLevel.getEntity( site.entity );
		if( destroy )
		{
			destroy = false;
			Game::damage( ent, nullptr, nullptr, nullptr, nullptr, ent->health_ + 1, DAMAGE_NO_PROTECTION, MOD_UNKNOWN, CAT_ANY );
			continue;
		}
		up++;
		Common::printf( "BASESITE %s %s at %.0f %.0f %.0f\n", site.team == ClientBase::TEAM_RED ? "red" : "blue",
			availableGroundInstallations[site.installation].descriptiveName, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2] );
	}
	Common::printf( "BASESITES standing %d of %d\n", up, static_cast<int>( placedSites.size() ) );
}

// "groundprofile x0 y0 x1 y1 steps top": the height of the ground under the height top along a line and what it is made of there, for finding
// out what a vehicle met (a kerb, a rim, a gap) without seeing it
void Game::groundProfileCommand()
{
	if( !g_cheats.integer )
		return;
	char arg[MAX_TOKEN_CHARS];
	float value[6] = {};
	for( int i = 0; i < 6; i++ )
	{
		CommandSystem::argvBuffer( i + 1, arg, sizeof( arg ) );
		value[i] = static_cast<float>( atof( arg ) );
	}
	const int steps = std::clamp( static_cast<int>( value[4] ), 1, 400 );
	constexpr float toBelow = 4000.0f;
	const float fromAbove = value[5];
	for( int i = 0; i <= steps; i++ )
	{
		const float part = static_cast<float>( i ) / steps;
		Vec3 top( value[0] + ( value[2] - value[0] ) * part, value[1] + ( value[3] - value[1] ) * part, fromAbove ), bottom = top;
		bottom[2] = -toBelow;
		trace_t tr;
		GameHost::trace( &tr, top, nullptr, nullptr, bottom, ENTITYNUM_NONE, MASK_SOLID, false );
		Common::printf( "GROUNDPROFILE %.1f %.1f z %.1f normal %.2f %.2f %.2f entity %d\n", top[0], top[1], tr.endpos[2], tr.plane.normal[0], tr.plane.normal[1], tr.plane.normal[2], tr.entityNum );
	}
}
