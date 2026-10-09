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

- **Full vehicle combat** in the air, on land and at sea, with new helicopters (the Hind and the Comanche) and stealth aircraft that stay off radar until they fire.
- **Soldiers on foot** who can leave a vehicle, fight, and climb into another one.
- **Vehicle theft.** Take any empty vehicle, including the enemy's. A stolen one keeps its paint, so its old team may not notice until it is too late.
- **Crews.** One player flies or drives while team mates ride along as gunners or passengers.
- **New bots.** Computer players now behave like people: each has its own personality, sets its own goals, finds its way around the map, lands to refuel, and flies in a way its aircraft can actually manage.

### Engine

- Rebuilt as one modern C++ program instead of the original collection of separate parts.
- Brought up to date with ioquake3: years of security, stability and performance fixes, plus modern sound, music formats, voice chat, game controllers and in-game downloads.
- Stands on its own: you do not need Quake III Arena to play.
- Runs as a 64-bit program, with a dedicated server for hosting games.

### Graphics

- A modern renderer by default, with the classic one still available.
- Proper widescreen: you see more of the world instead of a stretched picture, and the HUD keeps its shape.

### Network

- Overhauled to carry more players, more vehicles and more detail at once, while using less bandwidth on a busy battlefield.
- Everyone sees the same crews, turrets and weapon loads on every vehicle.

### Performance

- Collision checks, the work behind every movement and every shot, are up to 18 times faster on large outdoor maps.

## Building

Needs CMake 3.25 or newer, Ninja and a C++23 compiler. On macOS, `misc/macos/build-deps.sh x86_64` (or `arm64`)
builds SDL2, FreeType and OpenAL Soft into `deps/` first.

```bash
cmake --preset darwin-arm
cmake --build --preset darwin-arm
cpack --preset darwin-arm
```

Presets: `darwin-x86`, `darwin-arm`, `linux-x86`, `linux-arm`, `windows`. Each builds into `builds/<preset>`, and
`cpack` writes `dist/militaryforces-<version>-<preset>.zip` together with `dist/RELEASE_NOTES.md`.

The game data is a separate repository. Packaging takes the built paks from `../assets/build/mfdata`; set
`-DMF_DATA_DIR=<folder>` if they are somewhere else. To run a build in place:
`builds/<preset>/MilitaryForcesDed +set fs_basepath <folder holding mfdata>`.

## Credits

Military Forces is built on the Quake III Arena source code released by id Software under the GPL, and on [ioquake3](https://ioquake3.org).
