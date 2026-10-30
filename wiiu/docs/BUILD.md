# Build and deploy native Wii U Achievements (GX2 edition)

## Supported UI stack

- `wut` + devkitPPC/Wii U homebrew compiler;
- Wii U SDL2 (GX2), SDL2_image (PNG) and SDL2_ttf (Nunito);
- Python 3, Pillow 11.3.0 for author-time asset generation;
- WUPS SDK for the **inert, not-to-be-installed** plugin target.

The WUHB contains all art and fonts: the user installs **one `.wuhb` file**.
The original Nunito files are distributed under `assets/fonts/OFL.txt`.

The public Wii U devkitPro package server returned an HTTP 403 in hosted CI,
so the Dockerfile currently uses the published Wii U SDL2 builder from
`ghcr.io/happynico7504/spotify-wiiu-builder:latest`. This third-party image
should be replaced by a pinned digest before a production release. It
is only a **build-time toolchain**, not a console application dependency.

## Build locally

From repo root (Python + Pillow needed **on the host**):

```sh
python3 -m pip install -r wiiu/tools/requirements-font.txt
python3 -m unittest discover -s wiiu/tests -p 'test_*.py' -v
python3 wiiu/tools/generate_gpu_assets.py --root . --out wiiu/app/build/gpu-assets
python3 wiiu/tools/preview_gpu_ui.py --root . --assets wiiu/app/build/gpu-assets --out wiiu/app/build/visual-preview
docker build -f wiiu/Dockerfile -t wiiu-achievements-builder .
docker run --rm -v "${PWD}:/project" -w /project wiiu-achievements-builder bash wiiu/build-native.sh
```

Expected outputs:

- `wiiu/app/build/WiiUAchievementsNative.wuhb`
- `wiiu/plugin/WiiUAchievements.wps` (do **not** install)
- two optional host-side PNG layout previews.

GitHub Actions builds and attaches the same WUHB and previews on every
`wiiu-native` commit. The Windows `main` workflow is independent.

## Hardware smoke test (no plugin)

1. Back up the existing SD app WUHB so the earlier tested version can be restored.
2. Extract `WiiUAchievementsNative.wuhb` from the **GX2 Visual Port** artifact.
3. Replace `sd:/wiiu/apps/WiiUAchievementsNative.wuhb`.
4. Launch from Wii U Menu in Aroma; verify TV and GamePad separately.
5. In the achievements page, move with D-pad; A=details, B=game library,
   X=status filter, Y=category, L/R=chapters, PLUS=information.
6. Press B in the game library to exit; test HOME independently if desired.
7. If the screen is blank, immediately exits or the UI is slow, do not treat
   this build as stable; restore the previous tested version and report behavior.

**Important:** The GX2 edition has only been cross-compiled and host-previewed,
not tested on the physical Wii U. Do not install any `.wps` file.
