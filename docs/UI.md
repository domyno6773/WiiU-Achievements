# User interface (alpha 0.2)

The window and the unlock popup are custom-drawn WinForms surfaces styled after
the light Wii U system menus: a pale striped backdrop, white rounded cards, pill
buttons with GamePad face-button glyphs (A/B/X/Y), a cyan selection cursor and a
cel-shaded sea banner for the game.

![Main window](screenshots/main.png)

## Asset policy

- **No Nintendo files.** Every shape (gems, hearts, medals, trophy, clouds,
  waves, island, avatar, icons) is drawn with GDI+ in code. Nothing was ripped
  from the console, a game or a website. Do not add Nintendo logos, Miis, menu
  textures, sounds or the proprietary system font.
- **Font:** Nunito Regular/Bold/ExtraBold, © The Nunito Project Authors, SIL Open
  Font License 1.1 (`assets/fonts/OFL.txt`). The three static TTFs come from the
  Google Fonts API. `build.ps1` embeds them as manifest resources named
  `WiiUAchievements.Fonts.<file>`. Keep `OFL.txt` next to the fonts.
- **Sound:** `Chime.Build()` creates a three-note bell arpeggio (C6–G6–C7) as an
  in-memory 16-bit WAV. There is no sound file.

## Source map

| File | Responsibility |
| --- | --- |
| `src/MainForm.cs` | Session logic: connect, demo, sampling, settings, toast queue. Same semantics as alpha 0.1. |
| `src/MainForm.Ui.cs` | Layout, painting, animation timer, mouse/keyboard input and accessibility of the main window. |
| `src/Theme.cs` | Palette, fonts, easing, text helpers, rounded shapes, shadows, cursor and every vector icon/medal. `Art` maps a rule to its medal. |
| `src/Banner.cs` | `OceanBanner`: cached sky layer plus animated clouds, swell rows, foam and sparkles. |
| `src/Notification.cs` | `ToastForm` unlock popup, `Layered` (UpdateLayeredWindow) and `Chime`. |

## Rendering model

- Layout uses a 96-DPI logical canvas. `scale = DpiX / 96` is applied once with
  `ScaleTransform`. Mouse coordinates are divided by the same factor.
- `Arrange()` rebuilds the visible widget list (buttons, tabs, tiles, toggles,
  fields) on every paint and hit test. Widgets keep the same identity by id, so
  hover/press animation state survives a relayout.
- `Render()` draws the cached backdrop, then grid, tabs, banner, top bar, log
  drawer, bottom bar, settings sheet and detail dialog. Each layer is skipped
  when it lies outside the invalidated rectangle.
- A 15 ms `animation` timer advances hover/press, sheet/drawer/dialog transitions,
  the tab highlight, smooth scrolling, the progress ring, the rupee counter,
  staggered tile intros and unlock celebrations. It stops itself when nothing
  moves. Ambient motion (banner sea, status-dot pulse, focus pulse) runs only while
  this window is the active window and the setting is enabled. It pauses while
  the player is in Cemu, so it uses no CPU then.
- The 100 ms sampling timer and the achievement engine are unchanged.

## Achievement visuals

`Art.For(rule, allRules)` derives the medal without new XML fields:

- `rupees` → a faceted gem whose colour follows the rule's rank among rupee
  thresholds: green, blue, red, purple, gold, then silver.
- `max_health` → a heart container with the heart count as a badge.
- Ring tier by points: 1–9 bronze, 10–14 silver, 15–24 gold, 25+ platinum.
- Locked medals are desaturated and carry a lock badge. Unlocked medals get a
  green check and a tier-tinted card.

Tile footer states:

- **Unlocked:** local date and time.
- **No data:** "Da sbloccare".
- **Live below threshold:** progress bar and value.
- **At or above threshold during the 3-sample confirmation:** "Verifica…".
- **At or above threshold since connection:** "Soglia già superata al
  collegamento". The engine never awards existing progress. This display state
  is mirrored in `aboveSinceStart`, which the UI tracks itself. It does not read
  engine internals.

An unlock plays a 1.8 s celebration on its tile: medal pop, rays, sparkles, a
shine sweep and a gold edge. The ring and counters also animate.

## Unlock popup

![Unlock popup](screenshots/toast.png)

- A top-most `WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_NOACTIVATE |
  WS_EX_TOOLWINDOW` window updated through `UpdateLayeredWindow`. This gives
  per-pixel alpha (soft shadow, glow, rounded corners). It never takes focus,
  and clicks pass through it to the game.
- It appears top-centre on the monitor that shows Cemu's main window. Demo and
  preview popups appear on the app's monitor.
- Timeline (5.4 s): slide down with overshoot, medal pop, rotating rays,
  sparkles, text reveal, shine sweep, a countdown bar, then slide up and fade.
- If `UpdateLayeredWindow` fails, `Layered.Disabled` is set. The same unlock is
  then shown again in a plain rounded popup with opacity fades. Non-Windows
  runtimes always use that fallback.
- "Prova notifica" in the settings panel queues a preview popup labelled
  "ANTEPRIMA". Use it to check that popups appear over the game in the chosen
  Cemu fullscreen mode.

## Input and accessibility

| Input | Action |
| --- | --- |
| Mouse | Hover shows the cyan cursor. Click a tile for the detail dialog. Wheel scrolls the grid or the log. |
| Tab / Shift+Tab | Move focus through the controls in the active layer, text fields included. |
| Arrow keys | Spatial navigation (nearest control in that direction), like the Wii U Menu. |
| Enter / Space | Activate the focused control. |
| A / X / Y | Connect Cemu / start demo / disconnect, when no panel is open. |
| B / Esc / Backspace | Close the dialog, the settings panel or the log, in that order. |
| Page Up / Page Down | Scroll the grid. |

The form overrides `CreateAccessibilityInstance`. Each drawn control is exposed
as an `AccessibleObject` with a name, role, state, screen bounds and a default
action. The real text boxes are exposed through their own accessibility objects.
The status text is mirrored in the form's `AccessibleDescription`.

## Settings

`data/settings.xml` gains two optional elements: `<sound>` and `<ambient>`, both
`true`/`false`. Older files without them load with both enabled. The settings
panel saves immediately when a toggle changes. The Cemu path and profile remain
read-only while a session or the demo is running. Progress identity and the
achievement set revision (`wwhd-alpha-1`) are unchanged.

## Developer aids

- `--preview=<png>`: renders the main window after 1.2 s, then exits. It uses
  the same `Render()` path, not a screen grab.
- `--preview-toast=<png>`: renders one popup frame with transparency, then exits.

Screenshots in `docs/screenshots/` were rendered under Mono/libgdiplus on Linux
(Xvfb) with Nunito installed. Windows text antialiasing differs slightly.

| | |
| --- | --- |
| ![Unlock celebration](screenshots/unlock.png) | ![Detail dialog](screenshots/detail.png) |
| ![Settings](screenshots/settings.png) | |
