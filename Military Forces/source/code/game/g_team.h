/*
 * $Id: g_team.h,v 1.5 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

#ifndef __G_TEAM_H__
#define __G_TEAM_H__

inline constexpr int CTF_CAPTURE_BONUS = 5; // what you get for capture
inline constexpr int CTF_RECOVERY_BONUS = 1; // what you get for recovery
inline constexpr int CTF_FLAG_BONUS = 0; // what you get for picking up enemy flag
inline constexpr int CTF_FRAG_CARRIER_BONUS = 2; // what you get for fragging enemy flag carrier
inline constexpr int CTF_FLAG_RETURN_TIME = 120000; // seconds until auto return

// Prototypes
struct GameEntity;

int OtherTeam(int team);
const char *TeamName(int team);
const char *OtherTeamName(int team);
const char *TeamColorString(int team);
void AddTeamScore(vec3_t origin, int team, int score);

void Team_DroppedFlagThink(GameEntity *ent);
void Team_FragBonuses(GameEntity *targ, GameEntity *inflictor, GameEntity *attacker);
void Team_InitGame();
void Team_ReturnFlag(int team);
void Team_FreeEntity(GameEntity *ent);
GameEntity *SelectCTFSpawnPoint ( ClientBase::eTeam team, int teamstate, vec3_t origin, vec3_t angles, int idx );
GameEntity *Team_GetLocation(GameEntity *ent);
bool Team_GetLocationMsg(GameEntity *ent, char *loc, int loclen);
void TeamplayInfoMessage( GameEntity *ent );
void CheckTeamStatus();

int Pickup_Team( GameEntity *ent, GameEntity *other );


#endif // __G_TEAM_H__
