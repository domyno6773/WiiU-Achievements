# Validation

**2026-10-29 — Automatic load detection added**  
`GameSnapshot.PermanentRegressionCount` + check in `MainForm.Tick`.  
≥ 3 permanent field drops → automatic `engine.Reset()` + log.  
Does not clear already-unlocked achievements. Manual Pause still available.  
No new memory offsets. Synthetic coverage of the count method can be added later.

 record and reproduction

## GitHub main synchronization checks — 2026-10-29, Windows x64

- Refreshed `origin/main`; local `main` and remote shared the same starting
  commit before committing the accumulated development changes.
- Official C# 5/.NET Framework x64 build passed into
  `build-check/github-sync/`, leaving existing application executables intact.
- **248 core/Win32 checks passed** and **45 UI checks passed**. UI artifacts
  remain local in `build-check/github-sync-ui/`; scroll traversal median was
  **2.13 ms**, p95 **6.93 ms** over 80 frames.
- The first sandboxed core run stopped on temporary-file access; rerunning with
  the required filesystem access passed the complete suite.
- `git diff --check` passed. Only source, rules, tests and project documentation
  were included; no local progress, settings, build outputs or game files were
  added. No live Cemu validation or new package validation was performed.

## Editorial rarity and ordering review — 2026-10-29, Windows x64

- All 110 catalog entries have explicit rarity: **53 bronze, 47 silver, 10 gold**;
  no current platinum. The future set-completion platinum is reserved separately.
- Rarity no longer derives from points. The editorial file cannot change fields,
  thresholds, points, IDs or experimental flags. Missing entries, unknown IDs,
  duplicate IDs, unsupported attributes, unknown rarities and inappropriate
  platinum assignment fail loading before game selection changes.
- **248 core/Win32 checks passed**: existing progression/engine/native/persistence
  regressions plus rarity coverage, equal-point/different-rarity cases, story
  ordering and malformed editorial metadata. Legacy progress fixture still loads.
- **45 UI checks passed** including curated medal rendering, chronology, filters,
  cached live footers/unlock invalidation, demo/pause/resume and minimum size.
  The scroll pixel test now locates its wallet card after the deliberate numeric
  ordering change. Traversal median **2.15 ms**, p95 **6.33 ms** over 80 frames.
- Inspected Windows main preview: readable rarity names below medals; existing
  layout, art and experimental markers preserved. Detail pills and notifications
  share the same rarity rather than a separate points-derived classification.
- Separate build in `build-check/rarity/`; previous live executable left running.
  Settings/progress copied locally to missing destination files only, never into
  the shared package. Original points, stable IDs, `wwhd-alpha-1`, all conditions
  and memory offsets remain unchanged. No new Cemu evidence is claimed.

See `RARITIES.md` for the editorial criteria and corrected ordering/descriptions.

## Implemented playthrough series — 2026-10-28, Windows x64

The user explicitly requested implementation in batches before they verify the
events during a normal playthrough. This development build implements **53 new
experimental automatic rules**, for **70 executable objectives / 40 unimplemented
targets** (110 total). It does not certify the new mappings as live-verified.

- Official C# 5/.NET Framework x64 build succeeds into `build-check/playthrough/`.
- **241 core/Win32 checks passed**. Every executable addition is checked for no
  retrospective award on a completed initial state, a stable live crossing,
  persistence reload and atomic simultaneous awards. Additional decoder tests
  cover cumulative upgrades, sparse masks/popcounts, song identities, distinct
  pearls, invalid/unavailable encodings, capacities independent of current
  inventory, bottle contents, 49 mapped sectors, exact boss stages plus per-quest
  conjunctions, permanent-state rollback and defensive continuity snapshots.
- A reconstructed, sequential playthrough passes the actual parser, engine and
  progress store through clothing, telescope, sword, shield, pirates, first
  fortress, first dungeon tools, pearls, Tower of the Gods, Master Sword,
  Helmaroc, Zelda, elemental upgrades, temples, Triforce and light arrows. Each
  stage checks the exact award IDs and absence of unrelated awards. This is
  synthetic evidence, not a real completed playthrough.
- The Win32 child fixture now commits a synthetic two-page save window at the
  supported guest offset; the production reader reads it through real
  `ReadProcessMemory`, verifies the host=base+guest translation and decodes
  header, clothing and inventory. The four previous native scanner checks pass.
- **44 UI checks passed**: chronological default ordering, active experimental
  story cards, combined chapter/category/status filters, F chapter navigation,
  explicit pending versus executable labels, prior demo/pause/settings/profile
  behavior, bounded cache and no stale live footers. Inspected full-size and
  900 x 680 Windows previews. Scroll traversal median **2.32 ms**, p95 **6.18 ms**
  over 80 grid frames including cache fills/eviction, preserving the earlier fix.
- **Passive real-session baseline only:** exact Cemu 2.6 / EUR v0 / `cdc68acd`,
  current log and guest reservation matched. Without asking the user to perform
  actions, the 50 decoder fields were all available: relevant initial item,
  song, boss and event values zero; wallet capacity 500; three mapped sectors;
  zero bottles; clothing bit set. No raw state buffer/log was saved. This
  baseline does not validate acquisition or late-game mappings.

Read-only access, original IDs, `wwhd-alpha-1`, earned timestamps and existing
progress identity are preserved. Source-derived experiments retain explicit
labels in cards, details, accessibility, log and notifications. The demo does
not invent these fields, so absent data cannot arm them. A rollback in persistent
values resets transient conditions, but **automatic gameplay/load/save-slot
detection is still incomplete**: manual pause/disconnect before a load remains
required. Boss flags may persist on leaving/warping out of the boss area.

Live acquisitions for these 53 rules, per-field menu/load/reconnect/restart
checks, late-game HD layout and a full playthrough remain pending. Earlier real
rupee/tunic evidence below remains separate. See `PLAYTHROUGH.md` for the groups
the user can now verify naturally, without repeating an acquisition ritual.

## Combined scrolling/tunic build — 2026-10-28, Windows x64

The catalog now has **17 active objectives** (16 thresholds plus the green tunic)
and **93 pending targets**, with unchanged IDs and `wwhd-alpha-1` progress identity.

- **80 core/Win32 checks passed**. New checks cover the clothing bit mask,
  stable acquisition and persistence, no award for an already-received tunic,
  unknown data not arming the rule, unknown data clearing a pending acquisition,
  rejection of non-boolean snapshots and impossible boolean thresholds.
- **41 UI checks passed**, including the scrolling/cache regressions and an
  active tunic entry in the Story/Da sbloccare filters. Pending entries explicitly
  say Non attivo and announce that automatic unlocks are unavailable.
- The full-catalog 80-frame grid traversal in this combined build measured
  median **2.73 ms**, p95 **7.14 ms** (same 1120 x 800 setup as below; cold entries
  and cache eviction included). These are synthetic render timings, not a Cemu
  frame-rate measurement.
- Real targeted map observations: the user's acquired state read `0x80`; loading
  the earlier save before delivery read `0`; receiving the tunic again read
  `0x80`; leaving the house and opening the game menu still read `0x80`.
  Each observation used a newly opened read-only handle, exact profile matching
  and a reservation/log cross-check. See `MEMORY-MAP.md` for the derivation and
  remaining limits. No other story/inventory field was enabled.
- Built into `build-check/scroll-fix/` without stopping the user's previous app
  or Cemu. Settings and saved progress were copied locally from the running
  earlier build's data directory, only to missing destination files. The shared
  archive excludes this personal data.
- Packaging rebuilt from source, reran all 80 core/Win32 checks and verified a
  **53-file manifest**, including the card-cache source and active story XML.
- **Live end-to-end tunic unlock confirmed by the user.** The user launched the
  corrected build, loaded the pre-delivery save with recording disconnected,
  connected before delivery, waited for the baseline and received the tunic.
  They confirmed the notification **Il colore dell'avventura** appeared. A
  subsequent read of the local progress XML confirmed exactly one saved
  `story-heros-clothes` entry. No manual or retrospective award was written.

The tunic reads one extra byte at guest `0x145B81CE`, decoded with mask `0x80`.
The snapshot represents absent data separately from a real zero, including in
demo mode. The 16 original/additive thresholds still read the same six-byte
header. The active story rule lives in `achievements-story.xml`; the original
threshold file remains readable by the old executable. All three catalog files
are copied by build/test scripts and validated for revision/ID consistency.

Still pending: real application restart recovery of this specific tunic unlock,
Cemu restart stability and automatic gameplay/save-slot detection. **Pause or
disconnect before loading saves and connect/resume before the delivery.** The
app deliberately does not award a tunic already present on connection.

## Achievement scrolling performance — 2026-10-28, Windows x64

User reported heavy scrolling lag in the expanded build. A same-machine rendering
probe on the 110-card list (1120 x 800, 96 DPI, .NET Framework x64, no live game
attachment) measured 80 timed frames after 16 warm-up frames:

- Before: full-frame median 23.70 ms / p95 28.61 ms; grid median 19.14 ms /
  p95 21.40 ms. Layout alone was 0.06 ms median.
- After bounded static-card caching: full-frame median 12.27 ms / p95 12.62 ms;
  warm grid median 1.69 ms / p95 1.80 ms.
- The committed UI suite also traversed the entire catalog over 80 grid frames,
  including new cache entries and eviction: median 2.24 ms / p95 5.90 ms.
  These are rendering times, not measured screen refresh rates or a guarantee
  about Cemu load, other DPI settings or every PC.
- **40 UI checks passed**, including eight scrolling/cache regressions: reuse
  within a row, live progress pixels changing without rebuilding static cards,
  unlock medal invalidation, bounded cache during long traversal, access to the
  final target, release of cached entries and intro visibility deep in the list.
  A further accessibility check confirms that pending story targets explicitly
  announce the absence of automatic unlocks. Existing catalog, filters,
  pause/resume, demo, profile, minimum-size and settings checks still pass.
- Inspected Windows-rendered pending catalog preview: original layout, text,
  medals and shadows preserved. No engine, memory or progress format changed.

Static card images are limited to 48 entries and cleared on resize, game changes
and disposal. Scroll-only animation frames no longer repaint the banner/top bar.
The previous open executable and all personal progress remain untouched. Build
the fix separately with `build.ps1 -OutputDirectory .\build-check\scroll-fix`.

The user also reported receiving the green tunic again after loading an earlier
save, with no achievement. This is expected for its current **pending** catalog
entry: no clothing field or acquisition condition exists in the production
reader. This report is not evidence that the tunic detector works. A candidate
event bit was subsequently observed set once; live transition validation and
safe session/load detection are still required. Pending cards now say **Non
attivo** explicitly, and their accessibility description states that automatic
unlocks are unavailable.

## Expanded catalog verification — 2026-10-28, Windows x64

Checked on the original user's Windows PC while the existing application and
Cemu remained open. The running root EXE was not overwritten or stopped.

- Official C# 5/.NET Framework x64 build succeeded into `build-check/`.
- **73 core/Win32 checks passed**. Added coverage verifies all original IDs,
  thresholds and points, 16 executable versus 94 pending entries, strict pending
  metadata validation, category validation, unique IDs across both entry types,
  no pending awards even with maximum synthetic values or a pre-existing stored
  pending ID, and stable crossings/no retrospective awards/persisted reload for
  each of the six new thresholds. The synthetic native process fixture passed.
  The runtime threshold XML remains compatible with the older executable;
  duplicate IDs across companion files and executable companion rules are rejected.
- **31 UI checks passed**, including the 22 prior checks plus active-only
  completion totals, pending/category filtering, category keyboard cycling,
  pending accessibility explanations, detail cards, rendering while sampling and
  all four tabs fitting at the minimum width. Tests used an isolated directory,
  synthetic data and muted audio; they did not attach to Cemu.
- Visually inspected Windows-rendered pending-grid and pending-detail previews at
  900 x 680. Category controls, orange pending labels, proposal points, explanation
  text, status tabs and the active-only ring fit. Full-size and live-demo pending
  previews were also generated. Existing sea artwork, font license and sound
  sources were preserved.
- `package.ps1 -DestinationPath .\build-check\WiiUAchievements-expanded.zip`
  rebuilt, reran all 73 checks and verified a **51-file** manifest. New research
  source and catalog documentation are explicitly allowlisted. No local user
  data is included. This package is an expanded alpha, not a certified game set.
- Manual targeted read-only initial-item probe matched Cemu 2.6 / EUR v0 /
  `cdc68acd`, its log and its guest reservation. The user reported having received
  no initial items. Known header: 12/12 quarter hearts, 17 rupees. Telescope,
  sword and shield candidate bytes each read zero once. This is absence-only
  baseline evidence, not a verified acquisition detector. See `MEMORY-MAP.md`.
- The user clarified that the green tunic was already acquired. It is now the
  first of 26 story targets, before the telescope. No clothing flag was read or
  validated and no retrospective achievement was granted from this report.

The six new executable thresholds use the unchanged header reader; their real
game crossings have **not** been tested. All 94 story and extra targets remain
explicitly pending and cannot unlock. No new production memory offsets, gameplay/
save-slot detection, live inventory acquisition, menu/load/reconnect/restart
validation, full playthrough, fullscreen popup or audio evidence is claimed.
Original progress identity and revision remain `wwhd-alpha-1`; no migration or
copy of the user's progress was performed.

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
