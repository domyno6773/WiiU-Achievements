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

- The original **Nunito** font is not yet rendered on Wii U; the current UI
  uses a public-domain 8x8 bitmap by Daniel Hepper (see `font8x8_basic.h`).
  It displays ASCII-normalized catalog strings; original UTF-8 titles/descriptions
  remain in the generated source but are not yet rasterized on screen.
- The Windows app uses multicolumn responsive layout, vector medals and detailed
  animation, hover transition, animated sea/banner, polished panel shadows,
  celebration effects and a native lock/unlock status. Those are not all present.
- No actual completed/unlocked progress is shown. Because the native engine and
  persistence are not implemented, the interface **must not** report 0 earned or
  imply that the console has inspected save data.
- No installed-title enumeration, native gameplay monitoring, cloud integration,
  real unlock notifications or sound.
- Text clipping, layout and repaint cost still need hardware QA. The renderer
  only redraws on input changes; this avoids continuous scene redraw but does
  not guarantee the first navigation is fast.

## Testing contract

The original XML converter tests are automatically checked during native CI.
The current native application compiles with devkitPro/wut, but this **styled
build requires separate testing on the real Wii U** before being labeled stable.

On hardware, verify first launch and both display outputs, the full catalog,
filters with empty results, 2-column paging and chapter navigation, details,
return/exit behavior, and responsiveness. Report any freeze/slow repaint.
Do not install the WUPS plugin to test this UI.
