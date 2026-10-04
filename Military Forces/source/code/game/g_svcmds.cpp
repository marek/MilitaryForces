/*
 * $Id: g_svcmds.c,v 1.6 2006-01-29 14:03:41 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"
#include "g_level.h"
#include "g_bot.h"

/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

// extern	vmCvar_t	g_banIPs;
// extern	vmCvar_t	g_filterBan;


struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
};
using ipFilter_t = ipFilter_s;

constexpr int MAX_IPFILTERS = 1024;

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

/*
=================
StringToFilter
=================
*/
static bool StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			Common::printf( "Bad filter address: %s\n", s );
			return false;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *reinterpret_cast<unsigned *>(m);
	f->compare = *reinterpret_cast<unsigned *>(b);
	
	return true;
}

/*
=================
Game::updateIPBans
=================
*/
void Game::updateIPBans()
{
	byte	b[4];
	int		i;
	char	iplist[MAX_INFO_STRING];

	*iplist = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*reinterpret_cast<unsigned *>(b) = ipFilters[i].compare;
		Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist), 
			"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
	}

	CvarSystem::set( "g_banIPs", iplist );
}

/*
=================
Game::filterPacket
=================
*/
bool Game::filterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *reinterpret_cast<unsigned *>(m);

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
Game::addIP
=================
*/
void Game::addIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			Common::printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	Game::updateIPBans();
}

/*
=================
Game::processIPBans
=================
*/
void Game::processIPBans() 
{
	char *s, *t;
	char		str[MAX_TOKEN_CHARS];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			Game::addIP( t );
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f ()
{
	char		str[MAX_TOKEN_CHARS];

	if ( CommandSystem::argc() < 2 ) {
		Common::printf("Usage:  addip <ip-mask>\n");
		return;
	}

	CommandSystem::argvBuffer( 1, str, sizeof( str ) );

	Game::addIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f ()
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( CommandSystem::argc() < 2 ) {
		Common::printf("Usage:  sv removeip <ip-mask>\n");
		return;
	}

	CommandSystem::argvBuffer( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			Common::printf ("Removed.\n");

			Game::updateIPBans();
			return;
		}
	}

	Common::printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void Svcmd_EntityList_f() 
{
	for( int e = 1; e < theLevel.num_entities_ ; e++ ) //, check++ ) 
	{
		GameEntity* check = theLevel.getEntity(e);// g_entities+1;

		if ( !check || !check->inuse_ ) 
			continue;

		Common::printf("%3i:", e);
		switch ( check->s.eType ) 
		{
		case ET_GENERAL:
			Common::printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			Common::printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			Common::printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			Common::printf("ET_MISSILE          ");
			break;
		case ET_BULLET:
			Common::printf("ET_BULLET           ");
			break;
		case ET_MOVER:
			Common::printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			Common::printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			Common::printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			Common::printf("ET_SPEAKER          ");
			break;
		case ET_TELEPORT_TRIGGER:
			Common::printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			Common::printf("ET_INVISIBLE        ");
			break;
//		case ET_GRAPPLE:
//			Common::printf("ET_GRAPPLE          ");
//			break;
		case ET_VEHICLE:
			Common::printf("ET_VEHICLE          ");
			break;
		case ET_MISC_VEHICLE:
			Common::printf("ET_MISC_VEHICLE          ");
			break;
		case ET_EXPLOSIVE:
			Common::printf("ET_EXPLOSIVE        ");
			break;
		default:
			Common::printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname_ ) 
			Common::printf("%s", check->classname_);

		Common::printf("\n");
	}
}

GameClient* ClientForString( const char *s ) 
{
	GameClient*	cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) 
	{
		idnum = atoi( s );
		if ( idnum < 1 || idnum > theLevel.maxclients_ ) 
		{
			Common::printf( "Bad client slot: %i\n", idnum );
			return nullptr;
		}

		cl = theLevel.getClient(idnum);// &level.clients[idnum];
		if( cl && cl->pers_.connected_ == GameClient::ClientPersistant::CON_DISCONNECTED ) 
		{
			Common::printf( "Client %i is not connected\n", idnum );
			return nullptr;
		}
		return cl;
	}

	// check for a name match
	for( i=1 ; i <= theLevel.maxclients_ ; i++ ) 
	{
		cl = theLevel.getClient(i);// &level.clients[i];
		if ( !cl || cl->pers_.connected_ == GameClient::ClientPersistant::CON_DISCONNECTED ) 
			continue;

		if ( !Q_stricmp( cl->pers_.netname_, s ) ) 
			return cl;
	}

	Common::printf( "User %s is not on the server\n", s );

	return nullptr;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void Svcmd_ForceTeam_f() 
{
	GameClient*	cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	CommandSystem::argvBuffer( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) 
		return;

	// set the team
	CommandSystem::argvBuffer( 2, str, sizeof( str ) );
	SetTeam( theLevel.getEntity(cl->ps_.clientNum), str );// &g_entities[cl - level.clients], str );
}

char *ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
bool ConsoleCommand() 
{
	char	cmd[MAX_TOKEN_CHARS];

	CommandSystem::argvBuffer( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) 
	{
		Svcmd_EntityList_f();
		return true;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) 
	{
		Svcmd_ForceTeam_f();
		return true;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) 
	{
		Svcmd_GameMem_f();
		return true;
	}
	if( Q_stricmp( cmd, "addbot" ) == 0 ) 
	{
		Bots::addCommand();
		return true;
	}

	if( Q_stricmp( cmd, "kickbots" ) == 0 ) 
	{
		Bots::kickCommand();
		return true;
	}

	if( Q_stricmp( cmd, "botlist" ) == 0 ) 
	{
		Bots::listCommand();
		return true;
	}

	if (Q_stricmp (cmd, "addip") == 0) 
	{
		Svcmd_AddIP_f();
		return true;
	}

	if (Q_stricmp (cmd, "removeip") == 0) 
	{
		Svcmd_RemoveIP_f();
		return true;
	}

	if (Q_stricmp (cmd, "listip") == 0) 
	{
		CommandBuffer::executeText( EXEC_INSERT, "g_banIPs\n" );
		return true;
	}

	if (g_dedicated.integer) 
	{
		if (Q_stricmp (cmd, "say") == 0) 
		{
			SV_GameSendServerCommand( -1, va("print \"^3Server:^7 %s\"", ConcatArgs(1) ) );
			return true;
		}
		// everything else will also be printed as a say command
		SV_GameSendServerCommand( -1, va("print \"^3Server:^7 %s\"", ConcatArgs(0) ) );
		return true;
	}

	return false;
}

