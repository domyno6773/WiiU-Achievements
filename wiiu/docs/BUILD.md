# Build and first hardware smoke test

The current first milestone is a **read-only, canonical-XML-backed catalog viewer**
and an inert companion plugin. It displays the full source catalog with status,
rarity, points, descriptions and GamePad filters. The renderer now uses
the original light Wii U palette, a code-drawn sea banner, colored rarity
medals, white cards and cyan focus outlines, but not yet Nunito or the
original transitions and animations. No achievement unlock, save reading, overlay,
cloud sync, or installed-game discovery is implemented yet.

## Catalog parity tests (can run on any PC)

From repository root, with Python 3.10 or later:

```sh
python3 -m unittest discover -s wiiu/tests -p 'test_*.py' -v
python3 wiiu/tools/generate_catalog.py --root . --out /tmp/catalog_generated.c
```

The generated C data is compiled into the WUHB at build time and represents
the **110 original achievements** (70 executable definitions, including
53 experimental detectors, plus 40 planned). No new achievement IDs are
introduced or maintained separately. The catalog does not grant any awards.

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

The native devkitPro toolchain is not installed in the chat authoring container,
but GitHub Actions **successfully compiled** both formats on 2026-10-30.
The WUHB subsequently **opened on a real Wii U and displayed the catalog** as
reported by the tester. Plugin lifecycle, button navigation and separate screen
output still require real hardware testing. See [HARDWARE-TESTS.md](HARDWARE-TESTS.md).

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
2. Launch it from the Wii U Menu; press A for the XML-backed catalog, then use
   the D-pad **up/down/left/right** to move through the two-column achievement grid.
   A opens details, X cycles available-rule/planned filters, Y cycles categories,
   **L/R cycles chronological chapters**, and B goes back.
   X on the main screen opens information.
3. Confirm text, TV/GamePad output, controls, return to menu and process exit.
4. Do **not** install the `.wps` yet. Its lifecycle must first be compiled,
   reviewed, tested and enabled in a deliberate separate stage.
5. Keep all personal game progress and save data untouched.

The WUHB icon is original placeholder art, not a Nintendo or upstream asset.
The original UI reference is still the Windows project's `docs/UI.md` and
`docs/screenshots/`. This is the **first visual-parity iteration**, not the finished original UI.
The temporary bitmap font, static ocean, layout scaling and update performance
need testing against the original screenshots and on real hardware.

## What to record

Console firmware, Aroma version, app launch success, GamePad button checks,
TV/GamePad visibility, HOME behavior, exit behavior, and any crash or error.
Do not upload console keys, ROMs, save files, personal logs or memory dumps.

The XML-backed catalog generator and validation tests are now implemented.
The next milestone is Nunito typography and detailed UI parity, followed by a
native rule-engine parity harness and a safe plugin/notification smoke test — not game-memory
patching before hardware validation.
