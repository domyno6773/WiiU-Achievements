# Native Wii U frontend — visual parity status

Reference: `src/Theme.cs`, `src/Banner.cs`, `src/MainForm.Ui.cs`,
`docs/UI.md` and `docs/screenshots/main.png` from the original Windows
application. The native code derives its palette and component semantics from
these originals; it is **a port**, not a standalone aesthetic redesign.

## Implemented in the first styled native renderer

- Original light Wii U palette: near-white striped backdrop, cyan accent/selection
  and ink/muted colors matching the values in `Theme.cs`.
- White rounded content panels, catalog card geometry, colored medals for
  bronze/silver/gold and a cyan focus outline.
- A code-drawn, static cel-shaded ocean banner with waves/clouds/island. No Nintendo
  assets; the original ocean is richer and animated.
- Game catalog, overview, two-column achievements, details and information.
- All 110 achievements from the exact four original XML files, with category,
  chapter, rarity, title, description, points and experimental/planned status.
- GamePad D-pad navigation; A select; B back; X status filter; Y category;
  L/R chapter selection. TV/GamePad output is designed at 854x480 logical pixels.

## Not yet at parity

- **Nunito is now rendered on both screens**, using original SIL OFL fonts
  supplied by the Windows project. `tools/generate_font.py` rasterizes
  six sizes/weights at build time into native C glyph alpha maps. The source
  catalog's original UTF-8 strings are displayed; fallback for glyphs outside
  the covered Unicode set is '?'. Font rendering, line wrapping and text
  baseline were checked in native-renderer screenshots.
- TV now has three achievement columns; GamePad two. Source-inspired ring,
  clouds/island/sea banner, tier medals, white cards, and focus accents are
  implemented. The Windows app's animated waves, intricate gem variants, soft
  dynamic shadows, scroll inertia, progress graphs, celebration, transitions,
  and popup effects remain to be ported.
- No actual completed/unlocked progress is shown. Because the native engine and
  persistence are not implemented, the interface **must not** report 0 earned or
  imply that the console has inspected save data.
- No installed-title enumeration, native gameplay monitoring, cloud integration,
  real unlock notifications or sound.
- Text clipping and card layout are covered by host preview images generated
  from the actual native C renderer in GitHub Actions. Performance, repaint
  duration, display switching and legibility on a **physical Wii U** still
  require testing. The app redraws only on input changes, not continuously.
- The app now opens directly on the achievements catalog, more like the source
  Windows UI; B opens the game library, and B from there exits.

## Host visual regression workflow

The `.github/workflows/wiiu-native.yml` job now compiles the real
`app/src/main.c` + `app/src/ui_canvas.c` with `UI_HOST_PREVIEW` and a
minimal framebuffer shim in `tools/host_shim.c`.
It saves six PNG screenshots: TV / GamePad, each for game library, catalog,
and achievement details. The screenshot images are a visual QA aid, not a
substitute for native Wii U performance or pixel-accurate reproduction.

## Testing contract

The original XML converter tests are automatically checked during native CI.
The current native application compiles with devkitPro/wut, but this **styled
build requires separate testing on the real Wii U** before being labeled stable.

On hardware, verify first launch and both display outputs, the full catalog,
filters with empty results, 2-column paging and chapter navigation, details,
return/exit behavior, and responsiveness. Report any freeze/slow repaint.
Do not install the WUPS plugin to test this UI.
