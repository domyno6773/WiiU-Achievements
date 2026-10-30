# Wii U Achievements — native Aroma port

This is the **Wii U native edition of the existing Windows/Cemu project**,
not a new achievement set or a replacement for the original owner's work.

- **Development fork:** `domyno6773/WiiU-Achievements`, branch `wiiu-native`
- **Canonical upstream:** `hoffo1987/WiiU-Achievements`, branch `main`
- **Test environment:** real Wii U running Aroma
- **Preservation rule:** NEVER push to either repository's `main`, never modify
  upstream from native-port work.

## Native frontend: high-fidelity renderer

The current app frontend is `app/src/gpu_ui.c`, implemented in C using
**GX2-accelerated Wii U SDL2**, SDL2_image and SDL2_ttf.

- Full **1280×720 TV** rendering and a separate **854×480 GamePad** layout.
- Faithful visual structure from `docs/UI.md` and `src/MainForm.Ui.cs`:
  white rounded achievement cards, source color palette, bright cyan focus,
  rarity medals, layered ocean/banner, top profile/trophy bar, segmented filters,
  and original GamePad button-style actions.
- **Original Nunito** fonts (SIL Open Font License) from `assets/fonts/`;
  no legacy low-resolution bitmap font in the active build.
- Rounded cards, medals, clouds, waves and shadows are pre-rendered at 2×
  with Pillow by `tools/generate_gpu_assets.py`, bundled into the single
  `.wuhb` as compressed textures, then drawn using the Wii U GPU.
- Basic animation: GPU-driven animated wave highlights, crossfade between pages
  and short synthesized non-proprietary navigation tones.
- Local UI: directional GamePad navigation, A select, B back, X filters,
  Y category, L/R chapter, PLUS information.
- No Windows-specific buttons such as "Collega Cemu" or simulated demo progress.

**Important:** The application is presently a **read-only catalog**. It does
not detect installed games, inspect game memory, award achievements, show real
unlock progress, persist unlocks or sync with Firebase. It is not supposed
to display a fake percentage or pretend a user has unlocked anything.

All 110 entries, stable IDs, points, chapter, category, rarity, rule flags and
descriptions come from the original **four XML files**. The native catalog
generator is validated by host-side regression tests.

### Current test evidence

- The earlier `OSScreen` bootstrap launched on a real Wii U and displayed
  the catalog (first-hand user report from 2026-10-30).
- The new GX2/SDL2 interface has passed native cross-compilation on GitHub
  Actions and host-side sprite/layout preview generation.
- **The GX2 app is not yet hardware-validated.** Its runtime renderer performance,
  two-window dual-screen behavior and navigation must be tested on a real Wii U
  before it can be declared stable or visually identical at all sizes.

See [visual parity](docs/UI-PARITY.md), [native build](docs/BUILD.md),
[hardware test plan](docs/HARDWARE-TESTS.md) and [upstream sync](docs/UPSTREAM-SYNC.md).

## Code map

- `app/src/gpu_ui.c`: active GX2/SDL2 frontend.
- `tools/generate_gpu_assets.py`: source-matched textures/font embed assembly.
- `tools/preview_gpu_ui.py`: **approximate host visual QA**, using the same
  baked sprites/fonts/positions, not a screenshot of the Wii U GPU.
- `tools/generate_catalog.py`: canonical XML → validated native C catalog.
- `app/src/main.c`, `app/src/ui_canvas.*`: previous technical OSCreen renderer,
  retained in Git history/branch for reference, **not part of active WUHB build**.
- `plugin/`: deliberately inert WUPS bootstrap, **do not install yet**.

## Development guarantees

No game modifications, system-title patches, downloaded Nintendo material,
user progress dumps, or proprietary console assets are stored in this branch.
All Wii U-specific commits stay isolated from upstream `main`.
