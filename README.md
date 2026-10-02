# MilitaryForces
Military Forces, a game that allows a player to control planes, helicopters, tanks or infantry. It is based on the Quake 3 engine.

## Building
Requires CMake 3.21+ and SDL2 on Windows, macOS and Linux.

```
cmake -S . -B build
cmake --build build
./build/MilitaryForces +set fs_basepath /path/to/assets
```

The game data goes in `assets/mfdata`. Add `-DMF_SANITIZE=address,undefined` for a sanitizer build.
