#include "g_client.h"





GameClient::ClientPersistant::PlayerTeamState::PlayerTeamState() :
	state_(TEAM_BEGIN),
	location_(0),
	captures_(0),
	basedefense_(0),
	carrierdefense_(0),
	flagrecovery_(0),
	fragcarrier_(0),
	assists_(0),
	lasthurtcarrier_(0.0f),
	lastreturnedflag_(0.0f),
	flagsince_(0.0f),
	lastfraggedcarrier_(0.0f)
{
}

GameClient::ClientPersistant::ClientPersistant() :
	connected_(CON_DISCONNECTED),
	localClient_(false),	
	initialSpawn_(false),
	predictItemPickup_(false),
	pmoveFixed_(false),
	maxHealth_(0),
	enterTime_(0),
	voteCount_(0),
	teamVoteCount_(0),
	teamInfo_(false),
	lastRadar_(0)
{
	memset( &cmd_, 0, sizeof(cmd_) );
	netname_[0] = 0;
}

GameClient::ClientSession::ClientSession() :
	sessionTeam_(TEAM_FREE),
	spectatorTime_(0),
	spectatorState_(SPECTATOR_NOT),
	spectatorClient_(0),
	wins_(0),
	losses_(0),
	teamLeader_(0)
{
}

GameClient::GameClient() :
	readyToExit_(false),
	noclip_(false),
	lastCmdTime_(0),
	buttons_(0),
	oldbuttons_(0),
	latched_buttons_(0),
	damage_done_(0),
	damage_knockback_(0),
	damage_fromWorld_(false),
	accurateCount_(0),
	accuracy_shots_(0),
	accuracy_hits_(0),
	lastkilled_client_(0),
	lasthurt_client_(0),
	lasthurt_mod_(0),
	respawnTime_(0),
	inactivityTime_(0),
	inactivityWarning_(false),
	airOutTime_(0),
	lastKillTime_(0),
	switchTeamTime_(0),
	timeResidual_(0),
	areabits_(0),
	vehicle_(0),
	nextVehicle_(0),
	advancedControls_(0)
{
	VectorClear(oldOrigin_);
	VectorClear(damage_from_);
}

void
GameClient::clear()
{
	memset(&pers_, 0, sizeof(pers_));
	memset(&sess_, 0, sizeof(sess_));
	readyToExit_ = false;
	noclip_ = false;
	lastCmdTime_ = 0;
	buttons_ = 0;
	oldbuttons_ = 0;
	latched_buttons_ = 0;
	VectorClear(oldOrigin_);
	damage_done_ = 0;
	damage_knockback_ = 0;
	VectorClear(damage_from_);
	damage_fromWorld_ = false;
	accurateCount_ = 0;
	accuracy_shots_ = 0;
	accuracy_hits_ = 0;
	lastkilled_client_ = 0;
	lasthurt_client_ = 0;
	lasthurt_mod_ = 0;
	respawnTime_ = 0;
	inactivityTime_ = 0;
	inactivityWarning_ = false;
	airOutTime_ = 0;
	lastKillTime_ = 0;
	switchTeamTime_ = 0;
	timeResidual_ = 0;
	areabits_ = 0;
	vehicle_ = 0;
	nextVehicle_ = 0;	
	advancedControls_ = 0;
}

