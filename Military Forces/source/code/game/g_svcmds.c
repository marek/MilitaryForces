/*
 * $Id: g_svcmds.c,v 1.5 2005-11-21 17:28:20 thebjoern Exp $
*/

// Copyright (C) 1999-2000 Id Software, Inc.
//

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"


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


typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

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
			Com_Printf( "Bad filter address: %s\n", s );
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
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return true;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	int		i;
	char	iplist[MAX_INFO_STRING];

	*iplist = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist), 
			"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
	}

	Cvar_Set( "g_banIPs", iplist );
}

/*
=================
G_FilterPacket
=================
*/
bool G_FilterPacket (char *from)
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
	
	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			Com_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
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
			AddIP( t );
		t = s;
	}
}


/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];

	if ( Cmd_Argc() < 2 ) {
		Com_Printf("Usage:  addip <ip-mask>\n");
		return;
	}

	Cmd_ArgvBuffer( 1, str, sizeof( str ) );

	AddIP( str );

}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	ipFilter_t	f;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( Cmd_Argc() < 2 ) {
		Com_Printf("Usage:  sv removeip <ip-mask>\n");
		return;
	}

	Cmd_ArgvBuffer( 1, str, sizeof( str ) );

	if (!StringToFilter (str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			Com_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}

	Com_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities+1;
	for (e = 1; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		Com_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			Com_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			Com_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			Com_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			Com_Printf("ET_MISSILE          ");
			break;
		case ET_BULLET:
			Com_Printf("ET_BULLET           ");
			break;
		case ET_MOVER:
			Com_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			Com_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			Com_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			Com_Printf("ET_SPEAKER          ");
			break;
		case ET_TELEPORT_TRIGGER:
			Com_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			Com_Printf("ET_INVISIBLE        ");
			break;
//		case ET_GRAPPLE:
//			Com_Printf("ET_GRAPPLE          ");
//			break;
		case ET_VEHICLE:
			Com_Printf("ET_VEHICLE          ");
			break;
		case ET_MISC_VEHICLE:
			Com_Printf("ET_MISC_VEHICLE          ");
			break;
		case ET_EXPLOSIVE:
			Com_Printf("ET_EXPLOSIVE        ");
			break;
		default:
			Com_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			Com_Printf("%s", check->classname);
		}
		Com_Printf("\n");
	}
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			Com_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	Com_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	Cmd_ArgvBuffer( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	Cmd_ArgvBuffer( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str );
}

char	*ConcatArgs( int start );

/*
=================
ConsoleCommand

=================
*/
bool	ConsoleCommand() {
	char	cmd[MAX_TOKEN_CHARS];

	Cmd_ArgvBuffer( 0, cmd, sizeof( cmd ) );

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return true;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return true;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return true;
	}
/*
	if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return true;
	}

	if (Q_stricmp (cmd, "botlist") == 0) {
		Svcmd_BotList_f();
		return true;
	}
*/
	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return true;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return true;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		Cbuf_ExecuteText( EXEC_INSERT, "g_banIPs\n" );
		return true;
	}

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			SV_GameSendServerCommand( -1, va("print \"^3Server:^7 %s\"", ConcatArgs(1) ) );
			return true;
		}
		// everything else will also be printed as a say command
		SV_GameSendServerCommand( -1, va("print \"^3Server:^7 %s\"", ConcatArgs(0) ) );
		return true;
	}

	return false;
}

