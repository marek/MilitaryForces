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

Needs CMake 3.25 or newer, Ninja and a C++23 compiler. On macOS, `misc/macos/build-deps.sh x86_64` (or `arm64`)
builds SDL2, FreeType and OpenAL Soft into `deps/` first; on Linux, `misc/linux/build-deps.sh x86_64` (or `arm64`)
does the same (it needs the X11, Wayland, PulseAudio, PipeWire and ALSA headers: see the workflow's Linux packages).
The game carries those libraries beside itself; without `deps/` the system's own are used.

```bash
cmake --preset darwin-arm
cmake --build --preset darwin-arm
cpack --preset darwin-arm
```

### Windows (MSYS2 MinGW64)

Install [MSYS2](https://www.msys2.org/) and use its **MinGW64** shell. Put `C:\msys64\mingw64\bin` on `PATH` (and do not
leave an older separate MinGW ahead of it).

```bash
pacman -S --needed \
  mingw-w64-x86_64-cmake \
  mingw-w64-x86_64-ninja \
  mingw-w64-x86_64-gcc \
  mingw-w64-x86_64-SDL2 \
  mingw-w64-x86_64-freetype \
  mingw-w64-x86_64-pkgconf
```

```bash
cmake --preset windows
cmake --build --preset windows
```

Output is `builds/windows/`. SDL2 is copied beside the executable. OpenAL Soft is opened at run time as
`OpenAL64.dll` in that same folder (rename Soft's `soft_oal.dll` if needed).

Presets: `darwin-x86`, `darwin-arm`, `linux-x86`, `linux-arm`, `windows`. Each builds into `builds/<preset>`, and
`cpack` writes `dist/militaryforces-<version>-<preset>.zip` together with `dist/RELEASE_NOTES.md`. Linux also gets a
`.deb`: everything in `/opt/militaryforces`, with symlinks for `militaryforces`, `militaryforces-server` and the
applications menu.

The game data is a separate repository. Packaging takes the built paks from `../assets/build/mfdata`; set
`-DMF_DATA_DIR=<folder>` if they are somewhere else. To run a build in place:
`builds/<preset>/MilitaryForces +set fs_basepath <folder holding mfdata>` (or `MilitaryForcesDed` for the dedicated
server). On Windows that is `builds/windows/MilitaryForces.exe`.

## Credits

Military Forces is built on the Quake III Arena source code released by id Software under the GPL, and on [ioquake3](https://ioquake3.org).
