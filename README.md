# Military Forces

Planes, helicopters, tanks, boats and soldiers fighting on the same map. Built on the Quake 3 engine.

```
A note from the developers

This has been a passion project for a quarter of a century.

A small group of us fell in love with AirQuake, and then with AirQuake 2.
We came together again to build MFQ3, the game now known as Military
Forces. Some of us still keep in touch after all these years, and we
still dream of finishing it.

Thank you everyone for years of fun memories and the
support.

- Marek & Bjoern
```

Running a server: [docs/server.md](docs/server.md).

## What's new in the 25th anniversary edition

This edition is still being worked on. Here is what it brings so far.

### Gameplay

- **Full vehicle combat** in the air, on land and at sea: fighters, bombers, tanks, vehicles, infantry
- **Multiple Gamesets** WW2, WW2 & Modern combat
- **Stealth** dynamics
- **Soldiers on foot** who can leave a vehicle, fight, and climb into another one.
- **Vehicle theft.** Take any empty vehicle, including the enemy's. A stolen one keeps its paint, so its old team may not notice until it is too late.
- **Crews.** One player flies or drives while team mates ride along as gunners or passengers.
- **Advanced bots.** Computer players now behave like people: each has its own personality, sets its own goals and decision making.
  - Bots explore the map and learn as they go.
  - Team strategy, organization and defensive / offensive thinking.
  - Adjust their process to their human players
  - Fully support landing to refuel & resarm, supply delivery and passenger pick-up.
- **Mission editor** has been further enhanced.

### Engine

- Rebuilt as one modern C++ program instead of the original collection of separate parts.
- Brought up to date with ioquake3: years of security, stability and performance fixes, plus modern sound, music formats, voice chat, game controllers and in-game downloads.
- Stand-alone: you do not need Quake III Arena to play.
- Runs as a 64-bit program, with a dedicated server for hosting games.

### Graphics

- A modern OpenGL2 renderer by default, with the OpenGL1 still available.
- Proper widescreen support and HUD scaling

### Network

- Completely overhauled to carry more players, more vehicles and more detail at once
- Larger client state payload:  Crews, turrets and weapon loads, rotation all tracked/transmited 
- Improved entity prediction
- Tuned to use signficantly less bandwidth on a busy battlefield than vanilla Quake 3

### Physics

- Rigid Body simulations
- Simulated vehicle suspension and ground friction


### Performance

- Collision checks are up to 18x faster on large outdoor maps.

## Building

See [building.md](docs/building.md)

## Settings

See [settings.md](docs/settings.md)

### Video mode and fullscreen

`r_mode` is a numeric index into the list below (not a resolution string). `r_fullscreen` is `0` for a window and `1`
for fullscreen. A clean install is windowed at the **desktop's native resolution** (not a fixed mode number); archived
settings live in the per-user `mfconfig.cfg` (on Windows: `Documents\My Games\Military Forces\mfdata\mfconfig.cfg`).
Example: `+set r_mode 15 +set r_fullscreen 0` for a 1920×1080 window. A custom size uses `r_customwidth` /
`r_customheight` instead of a listed mode.

| `r_mode` | Resolution | `r_mode` | Resolution |
|---:|---|---:|---|
| 0 | 320×240 | 14 | 1600×900 |
| 1 | 400×300 | 15 | 1920×1080 |
| 2 | 512×384 | 16 | 2560×1440 |
| 3 | 640×480 | 17 | 3840×2160 |
| 4 | 800×600 | 18 | 1280×800 |
| 5 | 960×720 | 19 | 1440×900 |
| 6 | 1024×768 | 20 | 1680×1050 |
| 7 | 1152×864 | 21 | 1920×1200 |
| 8 | 1280×1024 | 22 | 2560×1600 |
| 9 | 1600×1200 | 23 | 1512×982 |
| 10 | 2048×1536 | 24 | 1728×1117 |
| 11 | 856×480 | 25 | 3024×1964 |
| 12 | 1280×720 | 26 | 3456×2234 |
| 13 | 1366×768 | 27 | 5120×2880 |

## Credits

Military Forces is built on the Quake III Arena source code released by id Software under the GPL, and on [ioquake3](https://ioquake3.org).
