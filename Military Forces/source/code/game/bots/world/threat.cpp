#include "../../g_local.h"
#include "../../g_level.h"
#include "../bot.h"
#include "../core/local.h"
#include "../team/team.h"
#include "threat.h"
#include <algorithm>

std::array<std::vector<ThreatMap::Disc>, 4> ThreatMap::discs_;
std::array<int, 4> ThreatMap::refreshTime_ = {};
std::array<std::vector<ThreatMap::Death>, 4> ThreatMap::deaths_;

namespace
{
	constexpr int refreshEvery = 2000;
	constexpr int deathMemory = 120000;	// a death warns for two minutes, fading
	constexpr float deathRadius = 900.0f, deathWeight = 0.6f;
	constexpr int sightingLife = 15000;
	constexpr float sightingWeight = 0.4f;
	constexpr float aircraftReach = 2500.0f, armourReach = 2500.0f, antiAirReach = 3500.0f;
	constexpr size_t mostDeaths = 32;

	int slotOf( int team ) { return std::clamp( team, 0, 3 ); }
	bool enemies( int team, int other ) { return team == ClientBase::TEAM_FREE || team != other; }
}

void ThreatMap::reset()
{
	for( int i = 0; i < 4; i++ )
	{
		discs_[i].clear();
		deaths_[i].clear();
		refreshTime_[i] = 0;
	}
}

void ThreatMap::noteDeath( int team, const vec3_t where )
{
	std::vector<Death>& list = deaths_[slotOf( team )];
	if( list.size() >= mostDeaths )
		list.erase( list.begin() );
	list.push_back( { Vec3( where ), theLevel.time_ } );
}

float ThreatMap::danger( int team, const vec3_t point, bool air )
{
	const int slot = slotOf( team );
	if( theLevel.time_ >= refreshTime_[slot] )
		refresh( team );
	float total = 0.0f;
	for( const Disc& disc : discs_[slot] )
	{
		if( air ? !disc.air : !disc.surface )
			continue;
		const float dx = point[0] - disc.centre.x(), dy = point[1] - disc.centre.y();
		const float share = (dx * dx + dy * dy) / (disc.radius * disc.radius);
		if( share < 1.0f )
			total += disc.weight * (1.0f - share) * 2.0f;
	}
	return total;
}

void ThreatMap::refresh( int team )
{
	const int slot = slotOf( team );
	refreshTime_[slot] = theLevel.time_ + refreshEvery;
	std::vector<Disc>& out = discs_[slot];
	out.clear();
	addSites( team, out );
	addSightings( team, out );
	addDeaths( team, out );
}

// the enemy's SAM and gun sites that still stand, each over the reach of its radar
void ThreatMap::addSites( int team, std::vector<Disc>& out )
{
	for( int i = MAX_CLIENTS; i < theLevel.num_entities_; i++ )
	{
		GameEntity* site = theLevel.getEntity( i );
		if( !site || !site->inuse_ || site->s.eType != ET_MISC_VEHICLE || site->s.modelindex != 255 || site->health_ <= 0 )
			continue;
		if( !enemies( team, site->s.generic1 ) || site->s.weaponIndex < 0 )
			continue;
		const int type = availableWeapons[site->s.weaponIndex].type;
		const bool shootsAir = type == WT_ANTIAIRMISSILE || type == WT_FLAK || type == WT_MACHINEGUN;
		const groundInstallationData_t& data = availableGroundInstallations[site->s.modelindex2];
		const float reach = static_cast<float>( shootsAir ? data.radarRange : data.radarRange2 );
		if( reach <= 0 )
			continue;
		out.push_back( { Vec3( site->r.currentOrigin ), reach, 1.0f, shootsAir, !shootsAir || type == WT_MACHINEGUN } );
	}
}

// enemies the team knows of, recently: an aircraft endangers everything near it, armour the ground, air defence the sky
void ThreatMap::addSightings( int team, std::vector<Disc>& out )
{
	for( int i = 1; i <= theLevel.maxclients_ && i <= MAX_CLIENTS; i++ )
	{
		GameEntity* other = theLevel.getEntity( i );
		if( !alive( other ) || other->client_->vehicle_ < 0 || !enemies( team, other->client_->sess_.sessionTeam_ ) )
			continue;
		vec3_t position;
		int time = 0;
		if( !BotTeam::known( team, i, position, time ) )
			continue;
		const float fade = 1.0f - static_cast<float>( theLevel.time_ - time ) / sightingLife;
		if( fade <= 0.0f )
			continue;
		const Capabilities can = Capabilities::of( other->client_->vehicle_ );
		if( can.runway || can.hovers )
			out.push_back( { Vec3( position ), aircraftReach, sightingWeight * fade, true, true } );
		else if( can.antiAirVehicle )
			out.push_back( { Vec3( position ), antiAirReach, sightingWeight * fade, true, false } );
		else if( can.armoured || can.onWater )
			out.push_back( { Vec3( position ), armourReach, sightingWeight * fade, false, true } );
	}
}

// where team mates died lately, fading over two minutes
void ThreatMap::addDeaths( int team, std::vector<Disc>& out )
{
	std::vector<Death>& list = deaths_[slotOf( team )];
	std::erase_if( list, []( const Death& d ) { return theLevel.time_ > d.time + deathMemory; } );
	for( const Death& death : list )
	{
		const float fade = 1.0f - static_cast<float>( theLevel.time_ - death.time ) / deathMemory;
		out.push_back( { death.where, deathRadius, deathWeight * fade, true, true } );
	}
}
