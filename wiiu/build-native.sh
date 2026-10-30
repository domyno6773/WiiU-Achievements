#!/usr/bin/env bash
# Run from any directory; repository path is calculated from this script.
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
: "${DEVKITPRO:?DEVKITPRO must point to the devkitPro installation}"

python3 -m unittest discover -s "$SCRIPT_DIR/tests" -p "test_*.py" -v

if [[ ! -s "$SCRIPT_DIR/app/src/font_nunito_generated.c" ]]; then
  python3 "$SCRIPT_DIR/tools/generate_font.py" --root "$REPO_DIR" \
    --output "$SCRIPT_DIR/app/src/font_nunito_generated.c"
fi

CMAKE_TOOL="$DEVKITPRO/portlibs/wiiu/bin/powerpc-eabi-cmake"
if [[ ! -x "$CMAKE_TOOL" ]]; then
  echo "Wii U CMake wrapper not found at $CMAKE_TOOL" >&2
  exit 1
fi
if [[ ! -f "$DEVKITPRO/wups/share/wups_rules" ]]; then
  echo "WUPS rules not installed: $DEVKITPRO/wups/share/wups_rules" >&2
  exit 1
fi

"$CMAKE_TOOL" -S "$SCRIPT_DIR/app" -B "$SCRIPT_DIR/app/build"
cmake --build "$SCRIPT_DIR/app/build" --parallel 2
make -C "$SCRIPT_DIR/plugin"

APP="$SCRIPT_DIR/app/build/WiiUAchievementsNative.wuhb"
PLUGIN="$SCRIPT_DIR/plugin/WiiUAchievements.wps"
[[ -s "$APP" && -s "$PLUGIN" ]] || {
  echo "Build completed without expected Wii U artifacts." >&2
  exit 1
}
printf '\nBuild OK:\n  %s\n  %s\n' "$APP" "$PLUGIN"
printf 'Plugin is inactive. Install/test the WUHB alone first.\n'
