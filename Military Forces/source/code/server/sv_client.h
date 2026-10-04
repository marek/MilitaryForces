#ifndef __SV_CLIENT_H__
#define __SV_CLIENT_H__


#include "../game/q_shared.h"
#include "../qcommon/qcommon.h"



struct EntityBase;

#ifdef USE_VOIP
inline constexpr int VOIP_QUEUE_LENGTH = 64;

struct voipServerPacket_s
{
	int generation;
	int sequence;
	int frames;
	int len;
	int sender;
	int flags;
	byte data[4000];
};
using voipServerPacket_t = voipServerPacket_s;
#endif

inline constexpr int MAX_SENT_ENTITIES = 512;

inline constexpr int SENT_UPDATE = 0;
inline constexpr int SENT_CREATE = 1;
inline constexpr int SENT_REMOVE = 2;

inline constexpr int ENTITY_KNOWN = 1;
inline constexpr int ENTITY_REMOVE = 2;

struct SnapshotEntityTable
{
	struct SentEntity
	{
		unsigned short	number;
		byte			type;
		uint64_t		mask;
	};

	struct SentFrame
	{
		int				sequence;
		bool			pending;
		bool			overflow;
		int				count;
		SentEntity		entities[MAX_SENT_ENTITIES];
	};

	entityState_t		sent[MAX_GENTITIES];
	uint64_t			forced[MAX_GENTITIES];
	byte				flags[MAX_GENTITIES];
	float				priority[MAX_GENTITIES];
	SentFrame			frames[PACKET_BACKUP];
};

class SV_Client
{
public:

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

		Message           msg_;
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

	int					gamestateMessageNum_;		// netchan->outgoingSequence() of gamestate
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
	int					lastSnapshotTime_;			// svs.time of last sent snapshot
	bool				rateDelayed_;				// true if nextSnapshotTime was set based on rate instead of snapshotMsec
	int					timeoutCount_;				// must timeout a few frames in a row so debugging doesn't break
	ClientSnapshot		frames_[PACKET_BACKUP];		// updates can be delta'd from here
	SnapshotEntityTable*	entityTable_;
	bool				entityReset_;
	bool				entityResetDone_;
	int					entityAck_;
	int					entityAckBits_;
	int					ping_;
	int					rate_;						// bytes / second
	int					snapshotMsec_;				// requests a snapshot every snapshotMsec unless rate choked
	int					pureAuthentic_;
	bool				gotCP_;						// TTimo - additional flag to distinguish between a bad pure checksum, and no cp command at all
	NetChannel			netchan_;

	// queuing outgoing fragmented messages to send them properly, without udp packet bursts
	// in case large fragmented messages are stacking up
	// buffer them into this queue, and hand them out to netchan as needed
	Netchan_Buffer*		netchan_start_queue_;
	Netchan_Buffer**	netchan_end_queue_;

#ifdef USE_VOIP
	bool				hasVoip_;
	bool				muteAllVoip_;
	bool				ignoreVoipFromClient_[MAX_CLIENTS];
	voipServerPacket_t*	voipPacket_[VOIP_QUEUE_LENGTH];
	int					queuedVoipPackets_;
	int					queuedVoipIndex_;
#endif

	int					oldServerTime_;
	bool				csUpdated_[MAX_CONFIGSTRINGS];

	void free();
	void drop( const char *reason );
	void sendClientGameState();
	void enterWorld( usercmd_t *cmd );
	int writeDownloadToClient( Message *msg );
	void userinfoChanged();
	void executeClientCommand( const char *s, bool clientOK );
	void think( usercmd_t *cmd );
	void executeClientMessage( Message *msg );
	void updateConfigstrings();
	int replacePendingServerCommands( const char *cmd );
	void addServerCommand( const char *cmd );
	int rateMsec();
	void netchanFreeQueue();
	void netchanTransmitNextInQueue();
	int netchanTransmitNextFragment();
	void netchanTransmit( Message *msg );
	bool netchanProcess( Message *msg );
	void updateServerCommandsToClient( Message *msg );
	void sendMessageToClient( Message *msg );
	void sendClientSnapshot();
	void closeDownload();
	void updateVoipIgnore( const char *idstr, bool ignore );
	bool clientCommand( Message *msg );
	void userMove( Message *msg, bool delta );
	bool shouldIgnoreVoipSender();
	void userVoip( Message *msg, bool ignoreData );
	void sendConfigstring( int index );
	SnapshotEntityTable * entityTable();
	void snapshotFrameLost( SnapshotEntityTable::SentFrame *sent );
	void snapshotAcks();
	bool snapshotEntityReset( bool requested );
	void emitPacketEntities( SV_Client::ClientSnapshot *frame, Message *msg, bool reset );
	void writeSnapshotToClient( Message *msg );
	void buildClientSnapshot();
	void writeVoipToClient( Message *msg );
};








#endif // __SV_CLIENT_H__

