/*
 * $Id: g_team.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

#include "g_local.h"
#include "g_entity.h"
#include "g_level.h"

struct teamgame_t {
	float			last_flag_capture;
	int				last_capture_team;
	flagStatus_t	redStatus;	// CTF
	flagStatus_t	blueStatus;	// CTF
	flagStatus_t	flagStatus;	// One Flag CTF
	int				redTakenTime;
	int				blueTakenTime;
	int				redObeliskAttackedTime;
	int				blueObeliskAttackedTime;
};

teamgame_t teamgame;

GameEntity *neutralObelisk;

void Team_SetFlagStatus( int team, flagStatus_t status );

void Team_InitGame( void ) 
{
	memset(&teamgame, 0, sizeof teamgame);

	switch( g_gametype.integer ) 
	{
	case GT_CTF:
		teamgame.redStatus = teamgame.blueStatus = static_cast<flagStatus_t>(-1); // Invalid to force update
		Team_SetFlagStatus( ClientBase::TEAM_RED, FLAG_ATBASE );
		Team_SetFlagStatus( ClientBase::TEAM_BLUE, FLAG_ATBASE );
		break;
	default:
		break;
	}
}

int OtherTeam( int team ) 
{
	if( team==ClientBase::TEAM_RED )
		return ClientBase::TEAM_BLUE;
	else if (team==ClientBase::TEAM_BLUE)
		return ClientBase::TEAM_RED;
	return team;
}

const char *TeamName(int team)  
{
	if( team==ClientBase::TEAM_RED )
		return "RED";
	else if (team==ClientBase::TEAM_BLUE)
		return "BLUE";
	else if (team==ClientBase::TEAM_SPECTATOR)
		return "SPECTATOR";
	return "FREE";
}

const char *OtherTeamName(int team) 
{
	if (team==ClientBase::TEAM_RED)
		return "BLUE";
	else if (team==ClientBase::TEAM_BLUE)
		return "RED";
	else if (team==ClientBase::TEAM_SPECTATOR)
		return "SPECTATOR";
	return "FREE";
}

const char *TeamColorString(int team) 
{
	if (team==ClientBase::TEAM_RED)
		return S_COLOR_RED;
	else if (team==ClientBase::TEAM_BLUE)
		return S_COLOR_BLUE;
	else if (team==ClientBase::TEAM_SPECTATOR)
		return S_COLOR_YELLOW;
	return S_COLOR_WHITE;
}

// NULL for everyone
void QDECL PrintMsg( GameEntity *ent, const char *fmt, ... )
{
	char		msg[1024];
	va_list		argptr;
	char		*p;
	
	va_start (argptr,fmt);
	if (vsprintf (msg, fmt, argptr) > sizeof(msg)) 
		Com_Error( ERR_DROP, "PrintMsg overrun" );

	va_end (argptr);

	// double quotes are bad
	while ((p = strchr(msg, '"')) != NULL)
		*p = '\'';

	SV_GameSendServerCommand ( ( (ent == NULL) ? -1 : ent->s.number ), va("print \"%s\"", msg ));	// s.number ok ?
//	SV_GameSendServerCommand ( ( (ent == NULL) ? -1 : ent-g_entities ), va("print \"%s\"", msg ));
}

/*
==============
AddTeamScore

 used for gametype > GT_TEAM
 for gametype GT_TEAM the level.teamScores is updated in AddScore in g_combat.c
==============
*/
void AddTeamScore(vec3_t origin, int team, int score) 
{
	GameEntity	*te = G_TempEntity(origin, EV_GLOBAL_TEAM_SOUND );

	te->r.svFlags |= SVF_BROADCAST;

	if ( team == ClientBase::TEAM_RED ) {
		if ( theLevel.teamScores_[ ClientBase::TEAM_RED ] + score == theLevel.teamScores_[ ClientBase::TEAM_BLUE ] )
		{
			//teams are tied sound
			te->s.eventParm = GTS_TEAMS_ARE_TIED;
		}
		else if ( theLevel.teamScores_[ ClientBase::TEAM_RED ] <= theLevel.teamScores_[ ClientBase::TEAM_BLUE ] &&
					theLevel.teamScores_[ ClientBase::TEAM_RED ] + score > theLevel.teamScores_[ ClientBase::TEAM_BLUE ]) 
		{
			// red took the lead sound
			te->s.eventParm = GTS_REDTEAM_TOOK_LEAD;
		}
		else 
		{
			// red scored sound
			te->s.eventParm = GTS_REDTEAM_SCORED;
		}
	}
	else 
	{
		if( theLevel.teamScores_[ ClientBase::TEAM_BLUE ] + score == theLevel.teamScores_[ ClientBase::TEAM_RED ] ) 
		{
			//teams are tied sound
			te->s.eventParm = GTS_TEAMS_ARE_TIED;
		}
		else if ( theLevel.teamScores_[ ClientBase::TEAM_BLUE ] <= theLevel.teamScores_[ ClientBase::TEAM_RED ] &&
					theLevel.teamScores_[ ClientBase::TEAM_BLUE ] + score > theLevel.teamScores_[ ClientBase::TEAM_RED ]) 
		{
			// blue took the lead sound
			te->s.eventParm = GTS_BLUETEAM_TOOK_LEAD;
		}
		else {
			// blue scored sound
			te->s.eventParm = GTS_BLUETEAM_SCORED;
		}
	}
	theLevel.teamScores_[ team ] += score;
}

/*
==============
OnSameTeam
==============
*/
bool OnSameTeam( GameEntity *ent1, GameEntity *ent2 )
{
	if ( !ent1->client_ || !ent2->client_ ) 
		return false;

	if ( g_gametype.integer < GT_TEAM ) 
		return false;

	if ( ent1->client_->sess_.sessionTeam_ == ent2->client_->sess_.sessionTeam_ ) 
		return true;

	return false;
}


static char ctfFlagStatusRemap[] = { '0', '1', '*', '*', '2' };
static char oneFlagStatusRemap[] = { '0', '1', '2', '3', '4' };

void Team_SetFlagStatus( int team, flagStatus_t status ) 
{
	bool modified = false;

	switch( team ) {
	case ClientBase::TEAM_RED:	// CTF
		if( teamgame.redStatus != status )
		{
			teamgame.redStatus = status;
			modified = true;
		}
		break;

	case ClientBase::TEAM_BLUE:	// CTF
		if( teamgame.blueStatus != status )
		{
			teamgame.blueStatus = status;
			modified = true;
		}
		break;

	case ClientBase::TEAM_FREE:	// One Flag CTF
		if( teamgame.flagStatus != status )
		{
			teamgame.flagStatus = status;
			modified = true;
		}
		break;
	}

	if( modified )
	{
		char st[4];

		if( g_gametype.integer == GT_CTF ) 
		{
			st[0] = ctfFlagStatusRemap[teamgame.redStatus];
			st[1] = ctfFlagStatusRemap[teamgame.blueStatus];
			st[2] = 0;
		}
		else 
		{		
			// GT_1FCTF
			st[0] = oneFlagStatusRemap[teamgame.flagStatus];
			st[1] = 0;
		}

		SV_SetConfigstring( CS_FLAGSTATUS, st );
	}
}

void Team_CheckDroppedItem( GameEntity *dropped ) 
{
	if( dropped->item_->giTag == OB_REDFLAG ) 
		Team_SetFlagStatus( ClientBase::TEAM_RED, FLAG_DROPPED );
	else if( dropped->item_->giTag == OB_BLUEFLAG ) 
		Team_SetFlagStatus( ClientBase::TEAM_BLUE, FLAG_DROPPED );
}

/*
================
Team_FragBonuses

Calculate the bonuses for flag defense, flag carrier defense, etc.
Note that bonuses are not cumulative.  You get one, they are in importance
order.
================
*/
void Team_FragBonuses( GameEntity *targ, GameEntity *inflictor, GameEntity *attacker )
{
	int i;
	GameEntity *ent;
	int flag_pw, enemy_flag_pw;
	int otherteam;
	int tokens;
	GameEntity *flag, *carrier = 0;
	char *c;
	int team;

	// no bonus for fragging yourself or team mates
	if (!targ->client_ || !attacker->client_ || targ == attacker || OnSameTeam(targ, attacker))
		return;

	team = targ->client_->sess_.sessionTeam_;
	otherteam = OtherTeam(targ->client_->sess_.sessionTeam_);
	if (otherteam < 0)
		return; // whoever died isn't on a team

	// same team, if the flag at base, check to he has the enemy flag
	if (team == ClientBase::TEAM_RED) 
	{
		flag_pw = OB_REDFLAG;
		enemy_flag_pw = OB_BLUEFLAG;
	} 
	else 
	{
		flag_pw = OB_BLUEFLAG;
		enemy_flag_pw = OB_REDFLAG;
	}

	// did the attacker frag the flag carrier?
	tokens = 0;
	if (targ->client_->ps_.objectives & enemy_flag_pw) 
	{
		attacker->client_->pers_.teamState_.lastfraggedcarrier_ = theLevel.time_;
		AddScore(attacker, targ->r.currentOrigin, CTF_FRAG_CARRIER_BONUS);
		attacker->client_->pers_.teamState_.fragcarrier_++;
		PrintMsg(NULL, "%s" S_COLOR_WHITE " fragged %s's flag carrier!\n",
			attacker->client_->pers_.netname_, TeamName(team));

		// the target had the flag, clear the hurt carrier
		// field on the other team
		for (i = 1; i <= g_maxclients.integer; i++) 
		{
			ent = theLevel.getEntity(i);// g_entities + i;
			if( ent && ent->inuse_ && ent->client_->sess_.sessionTeam_ == otherteam)
				ent->client_->pers_.teamState_.lasthurtcarrier_ = 0;
		}
		return;
	}

	// flag and flag carrier area defense bonuses

	// we have to find the flag and carrier entities

	// find the flag
	switch (attacker->client_->sess_.sessionTeam_) 
	{
	case ClientBase::TEAM_RED:
		c = "team_CTF_redflag";
		break;
	case ClientBase::TEAM_BLUE:
		c = "team_CTF_blueflag";
		break;		
	default:
		return;
	}
	// find attacker's team's flag carrier
	for (i = 1; i <= g_maxclients.integer; i++) 
	{
		carrier = theLevel.getEntity(i);// g_entities + i;
		if( carrier && carrier->inuse_ && (carrier->client_->ps_.objectives & flag_pw))
			break;
		carrier = NULL;
	}
	flag = NULL;
	while ((flag = G_Find (flag, FOFS(classname_), c)) != NULL) 
	{
		if (!(flag->flags_ & FL_DROPPED_ITEM))
			break;
	}

}

GameEntity* Team_ResetFlag( int team ) 
{
	char *c;
	GameEntity *ent, *rent = 0;

	switch (team) 
	{
	case ClientBase::TEAM_RED:
		c = "team_CTF_redflag";
		break;
	case ClientBase::TEAM_BLUE:
		c = "team_CTF_blueflag";
		break;
	case ClientBase::TEAM_FREE:
		c = "team_CTF_neutralflag";
		break;
	default:
		return NULL;
	}

	ent = NULL;
	while ((ent = G_Find (ent, FOFS(classname_), c)) != NULL) 
	{
		if (ent->flags_ & FL_DROPPED_ITEM)
			ent->freeUp();
		else
		{
			rent = ent;
			RespawnItem(ent);
		}
	}

	Team_SetFlagStatus( team, FLAG_ATBASE );

	return rent;
}

void Team_ResetFlags() 
{
	if( g_gametype.integer == GT_CTF ) 
	{
		Team_ResetFlag( ClientBase::TEAM_RED );
		Team_ResetFlag( ClientBase::TEAM_BLUE );
	}
}

void Team_ReturnFlagSound( GameEntity *ent, int team )
{
	GameEntity	*te;

	if (ent == NULL)
	{
		Com_Printf ("Warning:  NULL passed to Team_ReturnFlagSound\n");
		return;
	}

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND );
	if( team == ClientBase::TEAM_BLUE ) 
		te->s.eventParm = GTS_RED_RETURN;
	else 
		te->s.eventParm = GTS_BLUE_RETURN;

	te->r.svFlags |= SVF_BROADCAST;
}

void Team_TakeFlagSound( GameEntity *ent, int team ) 
{
	GameEntity	*te;

	if (ent == NULL) 
	{
		Com_Printf ("Warning:  NULL passed to Team_TakeFlagSound\n");
		return;
	}

	// only play sound when the flag was at the base
	// or not picked up the last 10 seconds
	switch(team) 
	{
		case ClientBase::TEAM_RED:
			if( teamgame.blueStatus != FLAG_ATBASE ) 
			{
				if (teamgame.blueTakenTime > theLevel.time_ - 10000)
					return;
			}
			teamgame.blueTakenTime = theLevel.time_;
			break;

		case ClientBase::TEAM_BLUE:	// CTF
			if( teamgame.redStatus != FLAG_ATBASE ) 
			{
				if (teamgame.redTakenTime > theLevel.time_ - 10000)
					return;
			}
			teamgame.redTakenTime = theLevel.time_;
			break;
	}

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND );
	if( team == ClientBase::TEAM_BLUE ) 
		te->s.eventParm = GTS_RED_TAKEN;
	else 
		te->s.eventParm = GTS_BLUE_TAKEN;

	te->r.svFlags |= SVF_BROADCAST;
}

void Team_CaptureFlagSound( GameEntity *ent, int team )
{
	GameEntity	*te;

	if (ent == NULL) 
	{
		Com_Printf ("Warning:  NULL passed to Team_CaptureFlagSound\n");
		return;
	}

	te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_TEAM_SOUND );
	if( team == ClientBase::TEAM_BLUE ) 
		te->s.eventParm = GTS_BLUE_CAPTURE;
	else 
		te->s.eventParm = GTS_RED_CAPTURE;

	te->r.svFlags |= SVF_BROADCAST;
}

void Team_ReturnFlag( int team ) 
{
	Team_ReturnFlagSound(Team_ResetFlag(team), team);
	if( team == ClientBase::TEAM_FREE ) 
		PrintMsg(NULL, "The flag has returned!\n" );
	else 
		PrintMsg(NULL, "The %s flag has returned!\n", TeamName(team));
}

void Team_FreeEntity( GameEntity *ent ) 
{
	if( ent->item_->giTag == OB_REDFLAG ) 
		Team_ReturnFlag( ClientBase::TEAM_RED );
	else if( ent->item_->giTag == OB_BLUEFLAG ) 
		Team_ReturnFlag( ClientBase::TEAM_BLUE );
}

/*
==============
Team_DroppedFlagThink

Automatically set in Launch_Item if the item is one of the flags

Flags are unique in that if they are dropped, the base flag must be respawned when they time out
==============
*/
void Team_DroppedFlagThink( GameEntity *ent ) 
{
	int		team = ClientBase::TEAM_FREE;

	if( ent->item_->giTag == OB_REDFLAG ) 
		team = ClientBase::TEAM_RED;
	else if( ent->item_->giTag == OB_BLUEFLAG ) 
		team = ClientBase::TEAM_BLUE;

	Team_ReturnFlagSound( Team_ResetFlag( team ), team );
	// Reset Flag will delete this entity
}


/*
==============
Team_DroppedFlagThink
==============
*/
int Team_TouchOurFlag( GameEntity *ent, GameEntity *other, int team ) 
{
	GameClient	*cl = other->client_;
	int			enemy_flag;

	if (cl->sess_.sessionTeam_ == ClientBase::TEAM_RED) 
		enemy_flag = OB_BLUEFLAG;
	else 
		enemy_flag = OB_REDFLAG;

	if ( ent->flags_ & FL_DROPPED_ITEM ) 
	{
		// hey, its not home.  return it by teleporting it back
		PrintMsg( NULL, "%s" S_COLOR_WHITE " returned the %s flag!\n", 
			cl->pers_.netname_, TeamName(team));
		AddScore(other, ent->r.currentOrigin, CTF_RECOVERY_BONUS);
		other->client_->pers_.teamState_.flagrecovery_++;
		other->client_->pers_.teamState_.lastreturnedflag_ = theLevel.time_;
		//ResetFlag will remove this entity!  We must return zero
		Team_ReturnFlagSound(Team_ResetFlag(team), team);
		return 0;
	}

	// the flag is at home base.  if the player has the enemy
	// flag, he's just won!
	if (!(cl->ps_.objectives & enemy_flag))
		return 0; // We don't have the flag
	PrintMsg( NULL, "%s" S_COLOR_WHITE " captured the %s flag!\n", cl->pers_.netname_, TeamName(OtherTeam(team)));

	cl->ps_.objectives &= ~enemy_flag;

	teamgame.last_flag_capture = theLevel.time_;
	teamgame.last_capture_team = team;

	// Increase the team's score
	AddTeamScore(ent->s.pos.trBase, other->client_->sess_.sessionTeam_, 1);

	other->client_->pers_.teamState_.captures_++;
	// add the sprite over the player's head
	other->client_->ps_.persistant[PERS_CAPTURES]++;

	// other gets another 10 frag bonus
	AddScore(other, ent->r.currentOrigin, CTF_CAPTURE_BONUS);

	Team_CaptureFlagSound( ent, team );

	Team_ResetFlags();

	CalculateRanks();

	return 0; // Do not respawn this automatically
}

int Team_TouchEnemyFlag( GameEntity *ent, GameEntity *other, int team ) 
{
	GameClient *cl = other->client_;

		PrintMsg (NULL, "%s" S_COLOR_WHITE " got the %s flag!\n",
			other->client_->pers_.netname_, TeamName(team));

		if (team == ClientBase::TEAM_RED)
			cl->ps_.objectives |= OB_REDFLAG; 
		else
			cl->ps_.objectives |= OB_BLUEFLAG;

		Team_SetFlagStatus( team, FLAG_TAKEN );

	AddScore(other, ent->r.currentOrigin, CTF_FLAG_BONUS);
	cl->pers_.teamState_.flagsince_ = theLevel.time_;
	Team_TakeFlagSound( ent, team );

	return -1; // Do not respawn this automatically, but do delete it if it was FL_DROPPED
}

int Pickup_Team( GameEntity *ent, GameEntity *other ) 
{
	int team;
	GameClient *cl = other->client_;

	// figure out what team this flag is
	if( strcmp(ent->classname_, "team_CTF_redflag") == 0 ) 
		team = ClientBase::TEAM_RED;
	else if( strcmp(ent->classname_, "team_CTF_blueflag") == 0 ) 
		team = ClientBase::TEAM_BLUE;
	else 
	{
		PrintMsg ( other, "Don't know what team the flag is on.\n");
		return 0;
	}
	// GT_CTF
	if( team == cl->sess_.sessionTeam_) 
		return Team_TouchOurFlag( ent, other, team );

	return Team_TouchEnemyFlag( ent, other, team );
}

/*
===========
Team_GetLocation

Report a location for the player. Uses placed nearby target_location entities
============
*/
GameEntity *Team_GetLocation(GameEntity *ent)
{
	GameEntity		*eloc, *best;
	float			bestlen, len;
	vec3_t			origin;

	best = NULL;
	bestlen = 3*8192.0*8192.0;

	VectorCopy( ent->r.currentOrigin, origin );

	for (eloc = theLevel.locationHead_; eloc; eloc = eloc->nextTrain_) 
	{
		len = ( origin[0] - eloc->r.currentOrigin[0] ) * ( origin[0] - eloc->r.currentOrigin[0] )
			+ ( origin[1] - eloc->r.currentOrigin[1] ) * ( origin[1] - eloc->r.currentOrigin[1] )
			+ ( origin[2] - eloc->r.currentOrigin[2] ) * ( origin[2] - eloc->r.currentOrigin[2] );

		if ( len > bestlen ) 
			continue;

		if ( !SV_inPVS( origin, eloc->r.currentOrigin ) ) 
			continue;

		bestlen = len;
		best = eloc;
	}

	return best;
}


/*
===========
Team_GetLocation

Report a location for the player. Uses placed nearby target_location entities
============
*/
bool Team_GetLocationMsg(GameEntity *ent, char *loc, int loclen)
{
	GameEntity *best;

	best = Team_GetLocation( ent );
	
	if (!best)
		return false;

	if (best->count_)
	{
		if (best->count_ < 0)
			best->count_ = 0;
		if (best->count_ > 7)
			best->count_ = 7;
		Com_sprintf(loc, loclen, "%c%c%s" S_COLOR_WHITE, Q_COLOR_ESCAPE, best->count_ + '0', best->message_ );
	} 
	else
		Com_sprintf(loc, loclen, "%s", best->message_);

	return true;
}


/*---------------------------------------------------------------------------*/

/*
================
SelectRandomDeathmatchSpawnPoint

go to a random point that doesn't telefrag
================
*/
#define	MAX_TEAM_SPAWN_POINTS	32
GameEntity *SelectRandomTeamSpawnPoint( int teamstate, ClientBase::eTeam team, int idx ) 
{
	GameEntity	*spot;
	int			count;
	int			selection;
	GameEntity	*spots[MAX_TEAM_SPAWN_POINTS];
	char		*classname;

	if (teamstate == GameClient::ClientPersistant::PlayerTeamState::TEAM_BEGIN) 
	{
		if (team == ClientBase::TEAM_RED)
			classname = "team_CTF_redplayer";
		else if (team == ClientBase::TEAM_BLUE)
			classname = "team_CTF_blueplayer";
		else
			return NULL;
	} 
	else
	{
		if (team == ClientBase::TEAM_RED)
			classname = "team_CTF_redspawn";
		else if (team == ClientBase::TEAM_BLUE)
			classname = "team_CTF_bluespawn";
		else
			return NULL;
	}
	count = 0;

	spot = NULL;

	while ((spot = G_Find (spot, FOFS(classname_), classname)) != NULL) 
	{
		if ( SpotWouldTelefrag( spot ) ) 
			continue;

		// check for category on this spawn point
		if( !(spot->ent_category_ & availableVehicles[idx].cat) ) 
			continue;

		spots[ count ] = spot;
		if (++count == MAX_TEAM_SPAWN_POINTS)
			break;
	}

	if ( !count ) 	// no spots that won't telefrag
		return G_Find( NULL, FOFS(classname_), classname);

	selection = rand() % count;
	return spots[ selection ];
}


/*
===========
SelectCTFSpawnPoint

============
*/
GameEntity *SelectCTFSpawnPoint ( ClientBase::eTeam team, int teamstate, vec3_t origin, vec3_t angles, int idx ) 
{
	GameEntity	*spot;

	spot = SelectRandomTeamSpawnPoint ( teamstate, team, idx );

	if (!spot) {
		return SelectSpawnPoint( vec3_origin, origin, angles, -1 );
	}

	VectorCopy (spot->s.origin, origin);
	origin[2] += 9;
	VectorCopy (spot->s.angles, angles);

	return spot;
}

/*---------------------------------------------------------------------------*/

static int QDECL SortClients( const void *a, const void *b )
{
	return *(int *)a - *(int *)b;
}


/*
==================
TeamplayLocationsMessage

Format:
	clientNum location health weapon powerups

==================
*/
void TeamplayInfoMessage( GameEntity *ent )
{
	char		entry[1024];
	char		string[8192];
	int			stringlength;
	int			i, j;
	GameEntity	*player;
	int			cnt;
	int			h;
	int			clients[TEAM_MAXOVERLAY];

	if ( ! ent->client_->pers_.teamInfo_ )
		return;

	// figure out what client should be on the display
	// we are limited to 8, but we want to use the top eight players
	// but in client order (so they don't keep changing position on the overlay)
	for( i = 1, cnt = 0; i <= g_maxclients.integer && cnt < TEAM_MAXOVERLAY; i++ ) 
	{
		player = theLevel.getEntity(theLevel.sortedClients_[i]);// g_entities + theLevel.sortedClients_[i];
		if( player && player->inuse_ && 
			player->client_->sess_.sessionTeam_ == ent->client_->sess_.sessionTeam_ ) 
		{
			clients[cnt++] = theLevel.sortedClients_[i];
		}
	}

	// We have the top eight players, sort them by clientNum
	qsort( clients, cnt, sizeof( clients[0] ), SortClients );

	// send the latest information on all clients
	string[0] = 0;
	stringlength = 0;

	for (i = 1, cnt = 0; i <= g_maxclients.integer && cnt < TEAM_MAXOVERLAY; i++) 
	{
		player = theLevel.getEntity(i);//g_entities + i;
		if( player && player->inuse_ && player->client_->sess_.sessionTeam_ == 
			ent->client_->sess_.sessionTeam_ ) 
		{

			h = player->client_->ps_.stats[STAT_HEALTH];
			if (h < 0) h = 0;

			Com_sprintf (entry, sizeof(entry),
				" %i %i %i %i %i", 
//				level.sortedClients[i], player->client->pers.teamState.location, h, a, 
				i, player->client_->pers_.teamState_.location_, h,  
				player->client_->ps_.weaponIndex, player->s.objectives);
			j = strlen(entry);
			if (stringlength + j > sizeof(string))
				break;
			strcpy (string + stringlength, entry);
			stringlength += j;
			cnt++;
		}
	}

	SV_GameSendServerCommand( ent->s.number, va("tinfo %i %s", cnt, string) );
	//SV_GameSendServerCommand( ent-g_entities, va("tinfo %i %s", cnt, string) );
}

void CheckTeamStatus() 
{
	int i;
	GameEntity *loc, *ent;

	if (theLevel.time_ - theLevel.lastTeamLocationTime_ > TEAM_LOCATION_UPDATE_TIME) 
	{
		theLevel.lastTeamLocationTime_ = theLevel.time_;

		for( i = 1; i <= g_maxclients.integer; i++ )
		{
			ent = theLevel.getEntity(i);//g_entities + i;

			if( !ent || !ent->client_ )
				continue;

			if( ent->client_->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
				continue;

			if (ent->inuse_ && (ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_RED ||	
				ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_BLUE))
			{
				loc = Team_GetLocation( ent );
				if (loc)
					ent->client_->pers_.teamState_.location_ = loc->health_;
				else
					ent->client_->pers_.teamState_.location_ = 0;
			}
		}

		for( i = 1; i <= g_maxclients.integer; i++ ) 
		{
			ent = theLevel.getEntity(i);// g_entities + i;

			if( !ent || !ent->client_ || ent->client_->pers_.connected_ != GameClient::ClientPersistant::CON_CONNECTED ) 
				continue;

			if (ent->inuse_ && (ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_RED ||	
				ent->client_->sess_.sessionTeam_ == ClientBase::TEAM_BLUE)) 
				TeamplayInfoMessage( ent );
		}
	}
}

/*-----------------------------------------------------------------*/

void UpdateLevelCategories( GameEntity *ent )
{
	if( !ent->ent_category_ ) 
		ent->ent_category_ = CAT_ANY;
//	else ent->ent_category <<= 8;
	
	// update level information
	theLevel.ent_category_ |= ent->ent_category_;

	Cvar_Set( "mf_lvcat", va("%i", theLevel.ent_category_) );
}

/*QUAKED team_CTF_redplayer (1 0 0) (-16 -16 -16) (16 16 32)
Only in CTF games.  Red players spawn here at game start.
*/
void SP_team_CTF_redplayer( GameEntity *ent ) 
{
	UpdateLevelCategories(ent);
}


/*QUAKED team_CTF_blueplayer (0 0 1) (-16 -16 -16) (16 16 32)
Only in CTF games.  Blue players spawn here at game start.
*/
void SP_team_CTF_blueplayer( GameEntity *ent ) 
{
	UpdateLevelCategories(ent);
}


/*QUAKED team_CTF_redspawn (1 0 0) (-16 -16 -24) (16 16 32)
potential spawning position for red team in CTF games.
Targets will be fired when someone spawns in on them.
*/
void SP_team_CTF_redspawn( GameEntity *ent ) 
{
	UpdateLevelCategories(ent);
}

/*QUAKED team_CTF_bluespawn (0 0 1) (-16 -16 -24) (16 16 32)
potential spawning position for blue team in CTF games.
Targets will be fired when someone spawns in on them.
*/
void SP_team_CTF_bluespawn( GameEntity *ent ) 
{
	UpdateLevelCategories(ent);
}

