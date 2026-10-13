# Building

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
`OpenAL64.dll`: put Soft's DLL there as that name (rename `soft_oal.dll` if needed), and keep a copy in
`deps/windows/OpenAL64.dll` (git-ignored) so a clean rebuild copies it back next to the exe.

Presets: `darwin-x86`, `darwin-arm`, `linux-x86`, `linux-arm`, `windows`. Each builds into `builds/<preset>`, and
`cpack` writes `dist/militaryforces-<version>-<preset>.zip` together with `dist/RELEASE_NOTES.md`. Linux also gets a
`.deb`: everything in `/opt/militaryforces`, with symlinks for `militaryforces`, `militaryforces-server` and the
applications menu.

The game data is a separate repository. Packaging takes the built paks from `../assets/build/mfdata`; set
`-DMF_DATA_DIR=<folder>` if they are somewhere else. To run a build in place:
`builds/<preset>/MilitaryForces +set fs_basepath <folder holding mfdata>` (or `MilitaryForcesDed` for the dedicated
server). On Windows that is `builds/windows/MilitaryForces.exe`.