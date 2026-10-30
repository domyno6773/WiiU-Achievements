# Wii U Achievements — native console port (development)

This directory belongs to the **`wiiu-native`** branch of the developer's fork,
[domyno6773/WiiU-Achievements](https://github.com/domyno6773/WiiU-Achievements).
The canonical upstream project is
[hoffo1987/WiiU-Achievements](https://github.com/hoffo1987/WiiU-Achievements)
(branch `main`). This is a port of the existing Windows application,
**not an independent achievements product**.

## Non-negotiable boundaries

- Write Wii U-specific changes **only** to `domyno6773/WiiU-Achievements:wiiu-native`.
- Never push to `hoffo1987/WiiU-Achievements` or to either repository's `main` from the port workflow.
- Preserve original achievements, stable IDs, set revisions, categories, rarities, display text and progress semantics. Never silently award existing milestones.
- Keep the original Windows `src/`, root XML and Windows scripts intact. Follow upstream changes deliberately and verify compatibility before importing them.
- Read-only monitoring of title memory. No patches to game executable, save editing or modification of game state.
- No proprietary Nintendo assets, system sounds, ROMs, keys, dumps or user progress in the repository.
- Evidence from Cemu is not evidence of correct behavior on real hardware; experimental flags remain experimental.

## Target environment

- Nintendo Wii U with Aroma.
- Frontend: native `.wuhb` app visible in the Wii U Menu (via Homebrew on Menu).
- Background monitor: WUPS `.wps` plugin loaded by Aroma.
- Development: devkitPro/devkitPPC, wut and Wii U Plugin System; evaluate libnotifications/NotificationModule for toast delivery.
- A shared portable achievements core and shared serialized rule identifiers, not two unrelated achievement engines.

The read-only catalog frontend is implemented in native C and generated from
the original four XML files. The native catalog now renders the original light Wii U palette, code-drawn
ocean banner, rarity medals, cyan cursor, white cards, and Nunito typography
rasterized from the original SIL OFL fonts. It uses **3 columns at 1280x720 on
TV** and **2 columns at 854x480 on GamePad**, with selectable achievement
details, filters and chronological chapters. A reproducible host-rendered
screenshot suite runs with the native C renderer.

This is a significant visual port, **not yet pixel-identical** to the original
WinForms UI: the animated ocean, complete medal icons/animation, progress
engine, and full interaction parity are unfinished. It **cannot unlock**
achievements or read your installed games/saves. The updated build still
requires hardware performance and layout testing.

The **first WUHB startup and catalog display have been confirmed on physical Wii U
hardware** (2026-10-30, tester report). The GamePad controls and both individual
outputs have not yet been independently checked. See [hardware test log](docs/HARDWARE-TESTS.md).
**Neither the WUPS plugin, memory inspection nor in-game overlays have been validated on physical hardware.**

## Proposed source layout

- `app/`: Wii U native catalog, game details, settings and achievements UI.
- `plugin/`: title lifecycle, read-only state sampler and notification dispatch.
- `core/`: portable implementation of upstream rules, unlock state and progress I/O.
- `graphics/`: original UI geometry, color and animation specifications adapted from the Windows project.
- `tests/`: cross-platform contract and conformance tests against upstream fixtures.
- `docs/`: port strategy, parity matrix, build/install instructions and real-hardware evidence.

Directories will be added as implementation begins. Avoid placeholder source files or pretend-build artifacts.

## First deliverables

1. Obtain a reproducible wut + WUPS toolchain and compile a minimal `.wuhb` and `.wps` independently.
2. Generate and validate the full catalog from upstream XML (implemented), then build golden behavioral parity fixtures against `Core.cs`.
3. Native light-theme visual pass with original Nunito (implemented; animated banner and perfect parity pending).
4. Validate notification functionality with a **manual test toast only** before attaching to a game.
5. Research game lifecycle, title identity and safe memory reads on a real Wii U; only then add the first live Wind Waker milestone.

Build and deployment instructions must be tested before they are marked as working.

See [the native visual parity status](docs/UI-PARITY.md), [the port architecture](docs/ARCHITECTURE.md), [compatibility contract](docs/COMPATIBILITY.md),
[hardware test log](docs/HARDWARE-TESTS.md) and [upstream sync policy](docs/UPSTREAM-SYNC.md).
