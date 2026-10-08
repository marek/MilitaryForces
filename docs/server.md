# Running a Military Forces server

A server is the `MilitaryForcesDed` program from the same package as the game. It has no window and needs no
graphics or sound. It finds the game data in the `mfdata` folder beside it.

## Start

```
./MilitaryForcesDed +set sv_hostname "My server" +set sv_maxclients 32 +set g_gametype 4 +map mfq3_canyon
```

Everything after a `+` is a console command. Settings that must be known before the map loads (player count,
game type, vehicle set, port) go on the command line or in a config file run from it:

```
./MilitaryForcesDed +exec server.cfg
```

`server.cfg` lives in `mfdata` (beside the paks) or in the per-user folder
(`~/Library/Application Support/Military Forces/mfdata`, `Documents\My Games\Military Forces\mfdata`, or the
XDG data folder on Linux). The server writes its own settings to `mfconfig_server.cfg` there.

A player can also host from inside the game (a "listen server"); the same settings apply.

## An example `server.cfg`

```
seta sv_hostname "Canyon run, team play"
seta sv_maxclients 32          // up to 63
seta g_gametype 4              // 0 free for all, 3 team deathmatch, 4 capture the flag
seta mf_gameset modern         // modern, ww2 or ww1: which vehicles are offered
seta g_botMinPlayers 16        // bots fill the server up to this many players
seta timelimit 20
seta fraglimit 0
seta capturelimit 8
seta g_friendlyFire 0
seta rconPassword "change-me"  // remote console; leave empty to switch it off
seta g_password ""             // players need this to join, if set
map mfq3_canyon
```

## Settings

| Setting | Default | What it does |
|---|---|---|
| `sv_hostname` | `noname` | Name shown in the server list |
| `sv_maxclients` | 8 | Player slots, bots included, up to 63. Takes effect at the next map load |
| `net_port` | 27960 | UDP port. Open it in the firewall and forward it on the router |
| `dedicated` | 1 on `MilitaryForcesDed` | 1 keeps the server off the public list, 2 announces it to the master server |
| `sv_master1` | `dpmaster.deathmask.net` | Master server a public server announces itself to |
| `sv_fps` | 40 | Server updates per second |
| `g_gametype` | 0 | 0 free for all, 3 team deathmatch, 4 capture the flag |
| `mf_gameset` | `modern` | Vehicle set: `modern`, `ww2`, `ww1` |
| `g_botMinPlayers` | 0 | Bots join until this many players are in the game and leave as people join |
| `timelimit`, `fraglimit`, `capturelimit` | 0, 20, 8 | When a game ends; 0 means no limit |
| `g_friendlyFire` | 0 | Whether team mates can hurt each other |
| `g_warmup` | 20 | Seconds of warm-up before a team game starts |
| `g_forcerespawn` | 20 | Seconds after which a dead player is respawned without pressing fire |
| `g_allowVote` | 1 | Players may call votes (map change, restart, kick) |
| `g_stealthPenalty` | 30 | Seconds a stealth aircraft stays visible on radar after firing |
| `g_enterRange` | 45 | How close a soldier must be to enter a vehicle |
| `g_ruseReset` | 30 | Seconds until a stolen vehicle that gave itself away is trusted by its old team again |
| `rconPassword` | empty | Password for the remote console (`rcon` from a client) |
| `sv_privateClients`, `sv_privatePassword` | 0, empty | Slots held back for players who know the password |
| `sv_timeout` | 200 | Seconds without a packet before a player is dropped |
| `sv_floodProtect` | 1 | Limits how fast one player can send commands |
| `sv_maxRate` | 0 | Upper limit on bytes per second sent to one player; 0 is no limit |
| `sv_snapshotBudget` | 1100 | Bytes of game state per update per player; raise it on a fast network with many players |
| `com_hunkMegs` | 512 (256 minimum on a dedicated server) | Memory for one map and its players, in megabytes |

Maps in the release: `mf_zephyr_beta_a3`, `mfq3_atoll`, `mfq3_canyon`, `mfq3_desertstorm`, `mfq3_field`,
`mfq3_groundwar`, `mfq3_norway`, `mfq3_orientation`, `mfq3_subpen`.

## Useful commands

| Command | What it does |
|---|---|
| `map <name>` | Loads a map |
| `map_restart` | Restarts the current map |
| `status` | Lists the players |
| `kick <name>`, `clientkick <number>` | Removes a player |
| `say <text>` | Speaks to everyone from the console |
| `exec <file>` | Runs a config file |
| `quit` | Stops the server |

## Letting players download your maps and paks

A player who joins without one of your paks can fetch it while connecting. There are two ways, and neither
needs anything outside your own server.

**From the game server itself.** Set

```
seta sv_allowDownload 1
```

and the server sends missing paks over the game connection. This is slow (`sv_dlRate`, 100 KB per second by
default) but needs nothing else.

**From a web address you run.** Put the paks on any web server, keeping the folder name:

```
https://example.org/militaryforces/mfdata/mf_map_mymap.pk3
```

and tell the game server where they are, without the `mfdata` part:

```
seta sv_allowDownload 1
seta sv_dlURL "https://example.org/militaryforces"
```

The address is sent to every player who connects; their game fetches `<sv_dlURL>/mfdata/<pak>.pk3` over HTTP,
which is much faster. Any static file host works. If `sv_dlURL` is empty the first way is used.

Players must have downloads switched on (`cl_allowDownload 1`). A game only downloads paks, only the ones the
server has loaded, and only into its own data folder.

Paks whose names start with `q3_` hold files from a player's own copy of Quake III Arena. They are never
offered for download and must not be put on a web server.
