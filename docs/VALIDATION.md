# Validation record and reproduction

## Local checkout verification — 2026-10-28

The current main branch was cloned into a dedicated local development folder on
the original user's Windows PC. The official C# 5/x64 build succeeded, followed
by all 50 core/Win32 checks and all 22 UI checks. Existing local progress was
copied separately into the ignored `data/` directory; no personal state was added
to Git. The older alpha 0.1 workspace was preserved as a historical copy.
This run did not repeat live-game unlock or Cemu restart testing for alpha 0.3.

Date of initial live verification: **2026-10-28**, Europe/Rome.

## Alpha 0.3 system and catalog checks (2026-10-28)

Environment: Linux cloud container, Mono 6.8, C# 5 x64, libgdiplus and Xvfb.
No access to the user's Windows PC, Cemu or the game was used for this change.

- Application and both test executables compile at the C# 5 language level.
- **46 portable checks passed**, including the original 12 non-Win32 assertions
  and 34 new engine/schema/catalog checks. New cases cover stable low baselines,
  rejection of a low glitch on a completed save, observable confirmation state,
  invalid/null/timestampless snapshots, duplicate timestamps, gap resets,
  simultaneous atomic awards, simulated storage failure and recovery, immutable
  returned unlock records, game/revision progress isolation, a fixed legacy progress fixture, heart-capacity
  transitions, impossible rule
  thresholds and unknown attributes/elements, exact profile matching and blocked
  unavailable games.
- **22 UI smoke checks passed**: real mouse/keyboard dispatch opens the catalog,
  selects the current game, returns with B/Esc, exposes unavailable cards to
  accessibility clients, runs the existing demo through an unlock, pauses and
  resumes without losing the unlock, keeps recording independent of catalog
  navigation, fits the catalog at the supported minimum size, opens existing
  settings, discards demo progress on disconnect, blocks game switches while
  attached, switches a synthetic game after disconnect, preserves selection
  when its rules fail to load, clears displayed progress on profile edits and
  safely falls back from an unknown saved game ID.
- Visually inspected rendered main, demo, catalog, paused and settings views,
  including the minimum-size catalog. The palette, embedded source fonts,
  original sea artwork, medals, shadows and existing card layout are preserved.
  Mono uses installed Nunito, as explained in the alpha 0.2 record below.
- `achievements.xml`, all ten IDs, `wwhd-alpha-1`, memory bytes/address, original
  artwork/sound sources and legacy progress identity remain unchanged. The
  original rupee unlock evidence below belongs to alpha 0.1, not a live alpha 0.3 run.

The four native Win32 fixture assertions are explicitly skipped by `--portable`.
`tests/run.ps1` still runs the full suite on Windows; `tests/run-ui.ps1` compiles
and exercises the application in an isolated temporary folder. GitHub Actions
is configured to run both, build and verify a new alpha 0.3 package, and retain
the ZIP and UI previews. The actual Windows result is recorded below.

### Windows CI (2026-10-28)

Code commit `a66789f290b9c9702575da2c7e6d598ba526ec53` passed
[workflow run 40764665750](https://github.com/hoffo1987/WiiU-Achievements/actions/runs/40764665750)
on GitHub's Windows Server 2025 runner:

- Official .NET Framework x64 compiler, explicitly C# 5: application, core suite
  and isolated UI suite compiled successfully, with embedded Nunito resources.
- **50 core/integration checks passed**, including all four native Win32 fixture
  assertions for the synthetic child-process reservation, scanner and live reads.
- **22 UI checks passed** on Windows, without Cemu or any game files.
- `package.ps1` and `verify-package.ps1` succeeded; the manifest verified **47
  files**. The Windows-built ZIP and seven UI preview PNGs are downloadable as
  workflow artifacts.
- Downloaded and visually inspected Windows main, catalog, paused and
  minimum-size catalog previews. The original visual style is retained, with
  the new catalog and pause controls using the existing theme. Long achievement
  labels use the existing ellipsis behavior at narrower window widths.

These are build, synthetic integration and UI results. They do not verify a live
Cemu session, the user's display setup or fullscreen notification behavior.

Still pending: real Cemu thresholds and pause/load/resume, emulator restarts,
fullscreen popup, multi-monitor behavior, actual audio, DPI/screen-reader
behavior on the user's PC, and automatic menu/save-slot validity detection.
Manual pause provides a recording boundary; it does not discover game state.
The committed alpha 0.2 ZIP remains historical; rebuild current source or use a
successful alpha 0.3 CI artifact rather than expecting that ZIP to contain changes.

## Real game observations

Configuration: Cemu 2.6, Wind Waker HD EUR v0, Title ID `0005000010143600`,
RPX hash `cdc68acd`. English game, no enhancements enabled, initial controllable
Link sequence. The session's host base was discovered and matched Cemu's log;
its absolute address and PID are intentionally not prerequisites in this handoff.

- Read-only probe: 12/12 quarter hearts, zero rupees.
- User collected rupees and reported five; the probe read five.
- App connected and displayed 5 rupees, 3/3 hearts, zero local unlocks.
- User crossed the 10-rupee threshold and confirmed the notification appeared.
- App displayed **Primi spiccioli**, one point, 1/10 objectives; the probe later
  read 15 rupees. XML on disk contained `wallet-10` with a UTC timestamp.
- The app was closed and reopened while Cemu remained running. It recovered the
  same unlock and point and did not issue a new award on connection.
- The GUI was visually inspected and its accessibility text checked.

This is observed end-to-end evidence for one threshold achievement and saved
progress. It is not a completed-game certification.

## Automated checks

The original alpha 0.1 `tests/run.ps1` compiled a standalone test executable and
executed these 16 assertions (the current suite adds the checks listed above):

1. Big-endian rupee decoding and quarter-heart decoding.
2. Rejection of impossible health.
3. No awards for already satisfied initial state.
4. Three consecutive matches required.
5. Live threshold crossing persisted before reporting an award.
6. No duplicate awards.
7. Reloading persisted progress.
8. Separation of local profiles.
9. Transient spike rejection and match-count reset.
10. Sampling-gap reset.
11. Disconnect/reset baseline behavior.
12. Preservation of corrupt originals and previous-backup recovery.
13. Discovery of a synthetic child process's 4 GiB reservation.
14. Scanner matches across a 1 MiB chunk boundary.
15. Filtering out a stale duplicate after the child changes one value.
16. Reading the updated child value through real `ReadProcessMemory`.

These passed on the initial PC. A fresh package can run them without Cemu, a
ROM, Internet access, or changes to game memory. The child writes only to its own
synthetic allocation. The full GUI/session-log parser is not covered by this suite.

## Portable package checks

Verified on 2026-10-28 on the same Windows PC:

- The packaging command built a separate binary and passed all 16 assertions
  under Windows PowerShell without a .NET SDK or external package restore.
- The resulting ZIP was extracted to another folder with spaces in its name.
- Its manifest verified 23 listed files; the ZIP contained those files plus the
  manifest. Hidden project instructions were included.
- Source from the extracted copy rebuilt into another folder with spaces in its
  name, using an absolute script path from a different working directory.
- That rebuilt executable started in demo/preview mode, rendered the GUI, and
  exited successfully. It did not connect to or interrupt the user's live Cemu.
- Archive inspection found no local `data/`, settings, logs, ROM, or key entries.

This checks relocation on this PC, not a second physical PC or another OS.

`package.ps1` builds from source into a separate staging folder, runs the suite
unless `-SkipTests` is explicitly supplied, and writes a SHA-256 file manifest.
The package uses an input allowlist and excludes the original `data/` directory.
`verify-package.ps1` verifies the listed sizes and hashes after extraction.

The manifest is not signed and does not establish the author's identity. Rebuild
or edit a packaged file and its old hash will no longer match. Repackage after
intentional changes. Newly generated local `data/` is not a manifest entry.

## Alpha 0.2 interface checks (2026-10-28, Linux only)

Environment: an Ubuntu 24.04 cloud container with Mono 6.8 (`mcs -langversion:5
-platform:x64`), libgdiplus and Xvfb (1400×1000). Windows, Cemu and the game
were not available.

Checked:

- All sources compile at the C# 5 language level. The build uses the
  `build.ps1` references plus `Accessibility.dll`, and embeds the three Nunito
  resources. `tests/CoreTests.cs` still compiles against `Core.cs` and
  `MemoryReader.cs`. It was not executed, because its Win32 fixture needs
  Windows. `Core.cs`, `GameSession.cs` and `MemoryReader.cs` are byte-identical
  to alpha 0.1.
- Demo mode, driven with xdotool and screenshotted:
  - idle state and the staggered tile intro;
  - live progress bars;
  - the first unlock: "Verifica…" during the 3-sample confirmation, the tile
    celebration, the ring going from 0 to 10%, and the popup;
  - the 4-heart unlock at 16 quarters;
  - filters, the detail dialog and the log drawer;
  - the settings panel, with fields read-only during the demo and the toggles;
  - Tab, arrow, Enter, B and Y keyboard handling;
  - disconnect, which clears the demo progress from view.
- `--preview` and `--preview-toast` write PNGs.
- `Chime.Build()` returns a valid 1.3 s, mono, 16-bit, 44.1 kHz WAV with a 0.42
  peak and a smooth decay. This was checked numerically; nobody listened to it.

Mono-specific observations. They are not reproduced on .NET, but recheck them
on Windows:

- libgdiplus draws fonts added with `PrivateFontCollection.AddMemoryFont` as
  empty boxes. Under Mono the app therefore uses an installed "Nunito" family,
  which is how the screenshots were taken. On .NET the embedded fonts are used,
  but that path has not been run.
- During banner-only partial repaints, Mono sometimes shows stale back-buffer
  pixels in a 12 px band above the bottom bar. WinForms on Windows clips the
  paint DC to the update region. Confirm this visually.
- Only the non-layered fallback popup ran on Linux. The `UpdateLayeredWindow`
  path has never been executed.

Not verified for alpha 0.2:

- Anything on Windows:
  - the `build.ps1` csc build and the 16-check suite;
  - embedded-font and ClearType rendering, DPI scaling and text-box placement;
  - the layered popup over windowed and fullscreen Cemu, and its placement on
    multiple monitors;
  - `SoundPlayer` playback;
  - screen-reader output;
  - CPU use while playing.
- The included `WiiUAchievements.exe`, which was compiled with Mono `mcs` and has
  never been launched on Windows. Rebuild it with `build.ps1`.
- A live game session with the new UI. The connect, sample and disconnect code
  is the alpha 0.1 logic, but the UI around it is new.
- Packaging. The alpha 0.2 archive was assembled on Linux by a script that copies
  the `package.ps1` allowlist and manifest format. Neither `package.ps1` nor
  `verify-package.ps1` was run.

## Not yet verified

- The fixed guest header across multiple real Cemu restarts or a full game run.
- Increasing/decreasing health during actual gameplay or gaining heart capacity.
- Every rupee/heart achievement threshold in the real game.
- File-select screen, loading transitions, changing saves, and hero mode.
- Other game regions/updates, emulator versions, mods, or Windows environments.
- Inventory, story, bosses, collections, or any frame-specific challenge.
- Cross-folder concurrent writers, online sync, or anticheat.

## Manual regression before promoting a profile

Use a spare local achievement profile. Start with the app disconnected, open the
supported game and load a playable save. Confirm displayed hearts/rupees against
the game. Connect below a threshold, cross it, and verify a single notification
and saved entry. Reopen the app and confirm persistence without a second award.
Disconnect before loading a different save in this alpha.

For a future profile, also restart Cemu, recheck the discovered host base and
fixed/pointer-derived guest values, exercise menus and loads, and record the
results. If the user or game session is unavailable, report live validation as
pending rather than extrapolating from synthetic tests.
