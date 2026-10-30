# UI parity — original Windows versus Wii U GPU frontend

**Canonical reference:** `docs/screenshots/main.png`,
`src/MainForm.Ui.cs`, `src/Theme.cs`, `src/Banner.cs`,
`src/Notification.cs`, `docs/UI.md`.

## Implemented

| Feature | Native Wii U status |
| --- | --- |
| Original theme color palette and striped system-menu background | Implemented |
| Nunito Regular/Bold/ExtraBold, Unicode text with SDL_ttf antialiasing | Implemented |
| Layered ocean, palm, island, cloud, wave art | Implemented as original vector-derived textures |
| Original-like white rounded cards, medals, lock badges, shadows | Implemented; some details still simplified |
| TV three-column and GamePad two-column layouts | Implemented |
| Actual 110 XML catalog entries, metadata and statuses | Implemented |
| Controller navigation, all/available/pending filters, categories, chapters | Implemented |
| GPU crossfades, subtle animated ocean waves, soft input sound | Implemented |
| Profile/demo semantics appropriate for console instead of Cemu | Implemented as *local, not connected* |

## Deliberate differences / remaining work

- No fabricated unlock timestamps, game snapshots or completion percentage.
  Without the monitor engine, a ring shows **110 catalog entries**, not "0%"
  or a fake demonstration value.
- "Sbloccati" remains visible but disabled until the native progress store is
  implemented; currently the UI skips that filter via X.
- Some original frame-level hover behaviors, tile sparkle animations,
  ocean/GDI gradients and custom art variants have not been reproduced
  exactly yet. This first GX2 version is **not guaranteed pixel-identical**.
- Cloud sync / account login, live progress bars, audio for genuine unlocks,
  GX2 achievement-toasts during gameplay and gameplay memory monitoring remain
  unimplemented. Installing the WUPS plugin is not part of UI testing.
- Title art, medals and sea scenery are recreated from source semantics,
  **not ripped Nintendo graphics**.

## Evidence and test gate

The `wiiu-native` GitHub Actions workflow runs catalog regression tests,
creates sprite assets using the original Nunito fonts and Pillow, produces
**host-generated layout approximations** for TV and GamePad, cross-compiles the
Wii U WUHB and inert WUPS plugin, then uploads app/artifacts.

A successful cross-build or an attractive host preview **cannot prove**
that two SDL2/GX2 windows render properly on the physical Wii U. The next
gate is a Wii U user test confirming:
- both TV and GamePad visible;
- responsive navigation and filters;
- no clipping/blank screens;
- animation/audio without glitches;
- HOME and B-to-exit behavior.

Report any differences against the supplied Windows screenshot. Only label
exact visual fidelity when actual screenshot comparisons support it.
