# Validation record and reproduction

Date of initial live verification: **2026-10-28**, Europe/Rome.

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

`tests/run.ps1` compiles a standalone test executable and executes 16 assertions:

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
