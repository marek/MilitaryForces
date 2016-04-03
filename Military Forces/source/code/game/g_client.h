#ifndef __G_CLIENT_H__
#define __G_CLIENT_H__

#include "q_shared.h"










struct GameClient : public ClientBase
{
	struct ClientPersistant
	{
		struct PlayerTeamState
		{
			enum ePlayerTeamStateState
			{
				TEAM_BEGIN,		// Beginning a team game, spawn at base
				TEAM_ACTIVE		// Now actively playing
			};

								PlayerTeamState();

			ePlayerTeamStateState state_;

			int					location_;

			int					captures_;
			int					basedefense_;
			int					carrierdefense_;
			int					flagrecovery_;
			int					fragcarrier_;
			int					assists_;

			float				lasthurtcarrier_;
			float				lastreturnedflag_;
			float				flagsince_;
			float				lastfraggedcarrier_;
		};

		enum eClientConnected
		{
			CON_DISCONNECTED,
			CON_CONNECTING,
			CON_CONNECTED
		} clientConnected_t;

							ClientPersistant();

		static const int	k_MAX_NETNAME = 36;

		eClientConnected	connected_;	
		usercmd_t			cmd_;					// we would lose angles if not persistant
		bool				localClient_;			// true if "ip" info key is "localhost"
		bool				initialSpawn_;			// the first spawn should be at a cool location
		bool				predictItemPickup_;		// based on cg_predictItems userinfo
		bool				pmoveFixed_;			//
		char				netname_[k_MAX_NETNAME];
		int					maxHealth_;				// for handicapping
		int					enterTime_;				// level.time the client entered the game
		PlayerTeamState		teamState_;				// status in teamplay games
		int					voteCount_;				// to prevent people from constantly calling votes
		int					teamVoteCount_;			// to prevent people from constantly calling votes
		bool				teamInfo_;				// send team overlay updates?
		int					lastRadar_;				// MFQ3 to remember radar setting
	};


	struct ClientSession
	{
		enum eSpectatorState
		{
			SPECTATOR_NOT,
			SPECTATOR_FREE,
			SPECTATOR_FOLLOW,
			SPECTATOR_SCOREBOARD
		};
		
							ClientSession();

		eTeam				sessionTeam_;
		int					spectatorTime_;		// for determining next-in-line to play
		eSpectatorState		spectatorState_;
		int					spectatorClient_;	// for chasecam and follow mode
		int					wins_;				// tournament stats
		int					losses_;			// tournament stats
		int					teamLeader_;		// true when this client is a team leader
	};

						GameClient();

	void				clear();

	// the rest of the structure is private to game
	ClientPersistant	pers_;
	ClientSession		sess_;

	bool				readyToExit_;		// wishes to leave the intermission

	bool				noclip_;

	int					lastCmdTime_;		// level.time of last usercmd_t, for EF_CONNECTION
											// we can't just use pers.lastCommand.time, because
											// of the g_sycronousclients case
	int					buttons_;
	int					oldbuttons_;
	int					latched_buttons_;

	vec3_t				oldOrigin_;

	// sum up damage over an entire frame, so
	// shotgun blasts give a single big kick
	int					damage_done_;		// damage taken out of health
	int					damage_knockback_;	// impact damage
	vec3_t				damage_from_;		// origin for vector calculation
	bool				damage_fromWorld_;	// if true, don't use the damage_from vector

	int					accurateCount_;		// for "impressive" reward sound

	int					accuracy_shots_;	// total number of shots
	int					accuracy_hits_;		// total number of hits

	//
	int					lastkilled_client_;	// last client that this client killed
	int					lasthurt_client_;	// last client that damaged this client
	int					lasthurt_mod_;		// type of damage the client did

	// timers
	int					respawnTime_;		// can respawn when time > this, force after g_forcerespwan
	int					inactivityTime_;	// kick players when time > this
	bool				inactivityWarning_;	// true if the five seoond warning has been given

	int					airOutTime_;

	int					lastKillTime_;		// for multiple kill rewards

	int					switchTeamTime_;	// time the player switched teams

	// timeResidual is used to handle events that happen every second
	// like health countdowns and regeneration
	int					timeResidual_;

	char				*areabits_;

	// MFQ3
	int					vehicle_;			// index of selected vehicle
	int					nextVehicle_;	
	int					advancedControls_;
};



#endif // __G_CLIENT_H__


