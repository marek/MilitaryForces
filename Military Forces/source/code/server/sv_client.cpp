#include "sv_client.h"
#include "sv_entity.h"




SV_Client::ClientSnapshot::ClientSnapshot() :
	areabytes_(0),
	num_entities_(0),
	first_entity_(0),
	messageSent_(0),
	messageAcked_(0),
	messageSize_(0)
{
	memset( areabits_, 0, sizeof(areabits_) );
	memset( &ps_, 0, sizeof(ps_) );
}

SV_Client::Netchan_Buffer::Netchan_Buffer() :
	next_(0)
{
	memset( &msg_, 0, sizeof(msg_) );
	memset( msgBuffer_, 0, sizeof(msgBuffer_) );
}


SV_Client::SV_Client() :
	clientNum_(-1),
	state_(CS_FREE),
	reliableSequence_(0),
	reliableAcknowledge_(0),
	reliableSent_(0),
	messageAcknowledge_(0),
	gamestateMessageNum_(0),
	challenge_(0),
	lastMessageNum_(0),
	lastClientCommand_(0),
	//gentity_(0),
	download_(0),
 	downloadSize_(0),
 	downloadCount_(0),
	downloadClientBlock_(0),
	downloadCurrentBlock_(0),
	downloadXmitBlock_(0),
	downloadEOF_(false),
	downloadSendTime_(0),
	deltaMessage_(0),
	nextReliableTime_(0),
	lastPacketTime_(0),
	lastConnectTime_(0),
	nextSnapshotTime_(0),
	rateDelayed_(false),
	timeoutCount_(0),
	ping_(0),
	rate_(0),
	snapshotMsec_(0),
	pureAuthentic_(0),
	gotCP_(false),
	netchan_start_queue_(0),
	netchan_end_queue_(0)
{
	memset( userinfo_, 0, sizeof( userinfo_ ) );
	memset( reliableCommands_, 0, sizeof(reliableCommands_) );
	memset( &lastUsercmd_, 0, sizeof(lastUsercmd_) );
	memset( lastClientCommandString_, 0, sizeof(lastClientCommandString_) );
	memset( name_, 0, sizeof(name_) );
	memset( downloadName_, 0, sizeof(downloadName_) );
	memset( downloadBlocks_, 0, sizeof(downloadBlocks_) );
	memset( downloadBlockSize_, 0, sizeof(downloadBlockSize_) );
	memset( frames_, 0, sizeof(frames_) );
	memset( &netchan_, 0, sizeof(netchan_ ) );
}


SV_Client::~SV_Client()
{
}

void
SV_Client::clear()
{
	clientNum_ = -1;
	state_ = CS_FREE;
	reliableSequence_ = 0;
	reliableAcknowledge_ = 0;
	reliableSent_ = 0;
	messageAcknowledge_ = 0;
	gamestateMessageNum_ = 0;
	challenge_ = 0;
	lastMessageNum_ = 0;
	lastClientCommand_ = 0;
	//gentity_ = 0;
	download_ = 0;
 	downloadSize_ = 0;
 	downloadCount_ = 0;
	downloadClientBlock_ = 0;
	downloadCurrentBlock_ = 0;
	downloadXmitBlock_ = 0;
	downloadEOF_ = false;
	downloadSendTime_ = 0;
	deltaMessage_ = 0;
	nextReliableTime_ = 0;
	lastPacketTime_ = 0;
	lastConnectTime_ = 0;
	nextSnapshotTime_ = 0;
	rateDelayed_ = false;
	timeoutCount_ = 0;
	ping_ = 0;
	rate_ = 0;
	snapshotMsec_ = 0;
	pureAuthentic_ = 0;
	gotCP_ = false;
	netchan_start_queue_ = 0;
	netchan_end_queue_ = 0;

	memset( userinfo_, 0, sizeof( userinfo_ ) );
	memset( reliableCommands_, 0, sizeof(reliableCommands_) );
	memset( &lastUsercmd_, 0, sizeof(lastUsercmd_) );
	memset( lastClientCommandString_, 0, sizeof(lastClientCommandString_) );
	memset( name_, 0, sizeof(name_) );
	memset( downloadName_, 0, sizeof(downloadName_) );
	memset( downloadBlocks_, 0, sizeof(downloadBlocks_) );
	memset( downloadBlockSize_, 0, sizeof(downloadBlockSize_) );
	memset( frames_, 0, sizeof(frames_) );
	memset( &netchan_, 0, sizeof(netchan_ ) );
}

