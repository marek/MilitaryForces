#ifndef __SV_CLIENT_H__
#define __SV_CLIENT_H__


#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"



struct EntityBase;

struct SV_Client
{
	enum eClientState
	{
		CS_FREE,									// can be reused for a New connection
		CS_ZOMBIE,									// client has been disconnected, but don't reuse
													// connection for a couple seconds
		CS_CONNECTED,								// has been assigned to a client_t, but no gamestate yet
		CS_PRIMED,									// gamestate has been sent, but client hasn't sent a usercmd
		CS_ACTIVE									// client is fully in game
	};

	struct ClientSnapshot 
	{
						ClientSnapshot();

		int				areabytes_;
		byte			areabits_[MAX_MAP_AREA_BYTES];// portalarea visibility bits
		playerState_t	ps_;
		int				num_entities_;
		int				first_entity_;				// into the circular sv_packet_entities[]
													// the entities MUST be in increasing state number
													// order, otherwise the delta compression will fail
		int				messageSent_;				// time the message was transmitted
		int				messageAcked_;				// time the message was acked
		int				messageSize_;				// used to rate drop packets
	};

	struct Netchan_Buffer
	{
						Netchan_Buffer();

		msg_t           msg_;
		byte            msgBuffer_[MAX_MSGLEN];
		Netchan_Buffer* next_;
	};

						SV_Client();
						~SV_Client();

	void				clear();
							
	int					clientNum_;					// number of this client (in the array)

	eClientState		state_;
	char				userinfo_[MAX_INFO_STRING];	// name, etc

	char				reliableCommands_[MAX_RELIABLE_COMMANDS][MAX_STRING_CHARS];
	int					reliableSequence_;			// last added reliable message, not necesarily sent or acknowledged yet
	int					reliableAcknowledge_;		// last acknowledged reliable message
	int					reliableSent_;				// last sent reliable message, not necesarily acknowledged yet
	int					messageAcknowledge_;

	int					gamestateMessageNum_;		// netchan->outgoingSequence of gamestate
	int					challenge_;

	usercmd_t			lastUsercmd_;
	int					lastMessageNum_;			// for delta compression
	int					lastClientCommand_;			// reliable client message sequence
	char				lastClientCommandString_[MAX_STRING_CHARS];
//	EntityBase*			gentity_;					// SV_GentityNum(clientnum)
	char				name_[MAX_NAME_LENGTH];		// extracted from userinfo, high bits masked

	// downloading
	char				downloadName_[MAX_QPATH];	// if not empty string, we are downloading
	fileHandle_t		download_;					// file being downloaded
 	int					downloadSize_;				// total bytes (can't use EOF because of paks)
 	int					downloadCount_;				// bytes sent
	int					downloadClientBlock_;		// last block we sent to the client, awaiting ack
	int					downloadCurrentBlock_;		// current block number
	int					downloadXmitBlock_;			// last block we xmited
	unsigned char*		downloadBlocks_[MAX_DOWNLOAD_WINDOW];	// the buffers for the download blocks
	int					downloadBlockSize_[MAX_DOWNLOAD_WINDOW];
	bool				downloadEOF_;				// We have sent the EOF block
	int					downloadSendTime_;			// time we last got an ack from the client

	int					deltaMessage_;				// frame last client usercmd message
	int					nextReliableTime_;			// svs.time when another reliable command will be allowed
	int					lastPacketTime_;				// svs.time when packet was last received
	int					lastConnectTime_;			// svs.time when connection started
	int					nextSnapshotTime_;			// send another snapshot when svs.time >= nextSnapshotTime
	bool				rateDelayed_;				// true if nextSnapshotTime was set based on rate instead of snapshotMsec
	int					timeoutCount_;				// must timeout a few frames in a row so debugging doesn't break
	ClientSnapshot		frames_[PACKET_BACKUP];		// updates can be delta'd from here
	int					ping_;
	int					rate_;						// bytes / second
	int					snapshotMsec_;				// requests a snapshot every snapshotMsec unless rate choked
	int					pureAuthentic_;
	bool				gotCP_;						// TTimo - additional flag to distinguish between a bad pure checksum, and no cp command at all
	netchan_t			netchan_;

	// queuing outgoing fragmented messages to send them properly, without udp packet bursts
	// in case large fragmented messages are stacking up
	// buffer them into this queue, and hand them out to netchan as needed
	Netchan_Buffer*		netchan_start_queue_;
	Netchan_Buffer**	netchan_end_queue_;
};








#endif // __SV_CLIENT_H__

