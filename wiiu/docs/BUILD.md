# Build and first hardware smoke test

The first version is an **explicit non-functional bootstrap**: only an app shell
and an inert companion plugin. No achievement unlock, save reading, overlay,
cloud sync, or installed-game discovery is implemented yet.

## Requirements

Install a current devkitPro toolchain with `wiiu-dev` (includes devkitPPC,
wut and Wii U CMake utilities), plus the Wii U Plugin System library
for the plugin. Reference: https://github.com/devkitPro/wut and
https://github.com/wiiu-env/WiiUPluginSystem .

On Linux with devkitPro normally installed under `/opt/devkitpro`:

```sh
export DEVKITPRO=/opt/devkitpro
export DEVKITPPC="$DEVKITPRO/devkitPPC"

# App
cd wiiu/app
mkdir -p build
cd build
"$DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-cmake" ..
cmake --build .

# Plugin (from repository root in a second terminal)
cd wiiu/plugin
make
```

Expected *if the toolchain matches the checked upstream examples*:
`wiiu/app/build/WiiUAchievementsNative.wuhb` and
`wiiu/plugin/WiiUAchievements.wps`.

The native devkitPro toolchain is **not present in the authoring environment**
at this first commit. The instructions and sources follow upstream examples,
but are **not yet reported as compiled or hardware-tested**. Do not treat these
paths as existing artifacts until an actual build succeeds.

## Build via Docker (Linux, Windows with Docker Desktop, macOS)

The repository provides a pinned `wiiu/Dockerfile` with devkitPro/wut
and WUPS. These commands run **on the development computer**, not the Wii U:

```sh
# Run from the repository root, with Docker running:
docker build -f wiiu/Dockerfile -t wiiu-achievements-builder .
docker run --rm -v "${PWD}:/project" -w /project \
  wiiu-achievements-builder bash wiiu/build-native.sh
```

PowerShell users can run the same commands (the `-v` mount value may need
`${PWD}.Path` instead of `${PWD}` depending on the Docker/PowerShell version).
For a devkitPro MSYS2 installation on Windows, the official wut sample
build instructions can be adapted; using Docker avoids a local toolchain.

The Dockerfile itself does not automatically run GitHub Actions or consume
the upstream owner's build minutes. This is a local, explicitly invoked build.

## Safe first test on Aroma

1. Copy only the app's `.wuhb` to
   `sd:/wiiu/apps/WiiUAchievementsNative.wuhb`.
2. Launch it from the Wii U Menu; press A to open Wind Waker's **placeholder**
   screen, X for information, B to return and B again to exit.
3. Confirm text, TV/GamePad output, controls, return to menu and process exit.
4. Do **not** install the `.wps` yet. Its lifecycle must first be compiled,
   reviewed, tested and enabled in a deliberate separate stage.
5. Keep all personal game progress and save data untouched.

The WUHB icon is original placeholder art, not a Nintendo or upstream asset.
The original UI reference is still the Windows project's `docs/UI.md` and
`docs/screenshots/`. This initial shell is **not** a graphic parity milestone.

## What to record

Console firmware, Aroma version, app launch success, GamePad button checks,
TV/GamePad visibility, HOME behavior, exit behavior, and any crash or error.
Do not upload console keys, ROMs, save files, personal logs or memory dumps.

The real next step is a shared manifest generated from the upstream achievement
XML and deterministic engine parity tests — not game-memory patching.
